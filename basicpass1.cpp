
#include <bits/stdc++.h>
using namespace std;

struct Symbol { string name; int address; };
struct Literal { string value; int address; };

int main(){
    ifstream fin("source.asm");

    ofstream IC ("intermediate_table.txt");
    ofstream SYM("symol_table.txt");
    ofstream LIT("literal_table.txt");
    ofstream POOL("pool_table.txt");

    map<string,int> IS = {
        {"STOP",0},{"ADD",1},{"SUB",2},{"MULT",3},{"MOVER",4},{"MOVEM",5},
        {"COMP",6},{"BC",7},{"DIV",8},{"READ",9},{"PRINT",10}
    };
    map<string,int> AD = { {"START",1},{"END",2},{"ORIGIN",3},{"EQU",4},{"LTORG",5} };
    map<string,int> DL = { {"DC",1},{"DS",2} };
    map<string,int> REG = { {"AREG",1},{"BREG",2},{"CREG",3},{"DREG",4} };
    map<string,int> CC  = { {"LT",1},{"LE",2},{"EQ",3},{"GT",4},{"GE",5},{"ANY",6} };

    vector<Symbol> s;
    vector<Literal> l;
    vector<int> p; p.push_back(0);

    int lc = 0;
    string line;

    auto stripComma = [&](string t){
        if(!t.empty() && t.back()==',') t.pop_back(); 
        return t;
    };

    auto findsymbol = [&](const string& name) -> int {
        for(int i=0; i<s.size(); i++) if (s[i].name == name) return i;
        s.push_back({name,-1});
        return s.size()-1;
    };

    auto findliteral = [&](const string& value) -> int {
        for(int i=p.back(); i<l.size(); i++)
            if (l[i].value == value && l[i].address==-1) return i;
        l.push_back({value,-1});
        return l.size()-1;
    };

    cout << "processing pass 1" << endl;

    while (getline(fin, line)){
        if (line.empty()) continue;
        stringstream ss(line);
        string a,b,c;
        ss >> a >> b >> c;

        // label without colon
        if (!IS.count(a) && !AD.count(a) && !DL.count(a)){
            int idx = findsymbol(a);
            s[idx].address = lc;
            a=b; b=c; ss >> c;
        }

        // remove trailing commas so AREG, works
        b = stripComma(b);
        c = stripComma(c);

        if (AD.count(a)){
            if(a=="START"){
                lc = b.empty()? lc : stoi(b);
                IC << "(AD,01) (C," << lc << ")\n";
            } else if (a=="END"){
                // flush literals
                for (int i=p.back(); i<l.size(); i++) {
                    if (l[i].address==-1){
                        l[i].address = lc++;
                        // literal like =5 or ='2' -> drop '='
                        string val = l[i].value;
                        if(!val.empty() && val[0]=='=') val = val.substr(1);
                        IC << "(DL,01) (C," << val << ")\n";
                    }
                }
                p.push_back((int)l.size());
                IC << "(AD,02)\n";
                break;
            } else if (a=="LTORG") {
                for (int i=p.back(); i<(int)l.size(); i++) {
                    if (l[i].address==-1){
                        l[i].address = lc++;
                        string val = l[i].value;
                        if(!val.empty() && val[0]=='=') val = val.substr(1);
                        IC << "(DL,01) (C," << val << ")\n";
                    }
                }
                p.push_back((int)l.size());
                IC << "(AD,05)\n";
            }
        }
        else if (DL.count(a)) {
            if (a=="DC") {
                IC << "(DL,01) (C," << b << ")\n";
                lc++;
            } else if (a=="DS") {
                IC << "(DL,02) (C," << b << ")\n";
                lc += b.empty()? 0 : stoi(b);
            }
        }
        else if (IS.count(a)) {
            IC << "(IS," << setw(2) << setfill('0') << IS[a] << ") ";

            if (!b.empty()) {
                if (REG.count(b)) IC << "(" << REG[b] << ") ";
                else if (CC.count(b)) IC << "(" << CC[b] << ") ";
            }
            if (!c.empty()) {
                if (c[0]=='=') {
                    int li = findliteral(c);
                    IC << "(L," << li+1 << ")";
                } else if (isdigit((unsigned char)c[0])) {
                    IC << "(C," << c << ")";
                } else {
                    int si = findsymbol(c);
                    IC << "(S," << si+1 << ")";
                }
            }
            IC << "\n";
            lc++;
        }
    }

    // ---- Write tables ----
    for (int i=0;i<(int)s.size();i++)
        SYM << i+1 << "\t" << s[i].name << "\t" << s[i].address << "\n";
    for (int i=0;i<(int)l.size();i++)
        LIT << i+1 << "\t" << l[i].value << "\t" << l[i].address << "\n";
    for (int i=0;i<(int)p.size();i++)
        POOL << i+1 << "\t" << p[i] << "\n";

    cout << "Pass-I complete. Files generated:\n";
    cout << "intermediate_table.txt, symol_table.txt, literal_table.txt, pool_table.txt\n";
    return 0;
}
