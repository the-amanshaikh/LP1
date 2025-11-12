#include <bits/stdc++.h>
using namespace std;

struct Symbol {
    string name;
    int addr;
    bool defined;
};

struct Literal {
    string value;
    int addr;
};

map<string, pair<string,int>> OPTAB = {
    {"STOP",{"IS",0}}, {"ADD",{"IS",1}}, {"SUB",{"IS",2}}, {"MULT",{"IS",3}},
    {"MOVER",{"IS",4}}, {"MOVEM",{"IS",5}}, {"COMP",{"IS",6}}, {"BC",{"IS",7}},
    {"DIV",{"IS",8}}, {"READ",{"IS",9}}, {"PRINT",{"IS",10}},
    {"START",{"AD",1}}, {"END",{"AD",2}}, {"ORIGIN",{"AD",3}},
    {"EQU",{"AD",4}}, {"LTORG",{"AD",5}}, {"DC",{"DL",1}}, {"DS",{"DL",2}}
};

map<string,int> REGTAB = {{"AREG",1},{"BREG",2},{"CREG",3},{"DREG",4}};
map<string,int> CCTAB = {{"LT",1},{"LE",2},{"EQ",3},{"GT",4},{"GE",5},{"ANY",6}};

vector<Symbol> SYMTAB;
vector<Literal> LITTAB;
vector<int> POOLTAB;      // holds start index (0-based) of each literal pool
vector<string> IC;

int LC = 0;

// --- helpers ---
static inline string trim(const string& s) {
    size_t i = s.find_first_not_of(" \t\r\n");
    if (i == string::npos) return "";
    size_t j = s.find_last_not_of(" \t\r\n");
    return s.substr(i, j - i + 1);
}
static inline string rstrip_comma(string s) {
    if (!s.empty() && (s.back()==',')) s.pop_back();
    return s;
}
static inline bool isNumber(const string& s) {
    if (s.empty()) return false;
    size_t i = 0;
    if (s[0]=='+' || s[0]=='-') i=1;
    if (i==s.size()) return false;
    for (; i<s.size(); ++i) if (!isdigit((unsigned char)s[i])) return false;
    return true;
}
static inline int safe_stoi(const string& s, int def=0) {
    try {
        if (s.empty()) return def;
        return stoi(s);
    } catch (...) { return def; }
}

// Extract numeric content of literal strings like =5, =‘5’, =C’5’, =’123’, or even =F’10’
// Fallback: collect sign+digits from the string.
int literalValue(string lit) {
    // remove leading '=' if present
    if (!lit.empty() && lit[0]=='=') lit = lit.substr(1);

    // try quoted pattern ='...'
    size_t q1 = lit.find('\'');
    size_t q2 = (q1==string::npos) ? string::npos : lit.find('\'', q1+1);
    if (q1 != string::npos && q2 != string::npos && q2 > q1+1) {
        string inner = lit.substr(q1+1, q2-(q1+1));
        if (isNumber(inner)) return safe_stoi(inner);
        // if it's a single char like 'A', treat as ASCII
        if (inner.size()==1) return (int)inner[0];
        // else fallback to filtering digits
    }

    // fallback: extract first signed integer substring
    string acc;
    for (size_t i=0;i<lit.size();++i) {
        char c = lit[i];
        if ((c=='+' || c=='-') && acc.empty()) { acc.push_back(c); continue; }
        if (isdigit((unsigned char)c)) acc.push_back(c);
        else if (!acc.empty()) break;
    }
    return acc.empty() ? 0 : safe_stoi(acc);
}

int getSymIndex(const string& s) {
    for (int i=0;i<(int)SYMTAB.size();i++)
        if (SYMTAB[i].name == s) return i;
    SYMTAB.push_back({s,-1,false});
    return (int)SYMTAB.size()-1;
}

int getLitIndex(const string& lit) {
    for (int i=0;i<(int)LITTAB.size();i++)
        if (LITTAB[i].value == lit) return i;
    LITTAB.push_back({lit,-1});
    return (int)LITTAB.size()-1;
}

// Evaluate expressions like SYM, CONST, SYM+CONST, SYM-CONST (no spaces)
int evalExpr(const string& expr) {
    string e = trim(expr);
    if (e.empty()) return 0;

    // look for + or - (but not at position 0)
    size_t plus = e.find('+', 1);
    size_t minus = e.find('-', 1);

    auto valOfSym = [&](const string& name)->int {
        for (auto &s:SYMTAB) if (s.name == name) return (s.addr==-1?0:s.addr);
        // if not present, insert as undefined (addr=-1) to be resolved later if needed
        int idx = getSymIndex(name);
        return SYMTAB[idx].addr==-1 ? 0 : SYMTAB[idx].addr;
    };

    if (plus != string::npos) {
        string base = e.substr(0, plus);
        string off  = e.substr(plus+1);
        return valOfSym(base) + safe_stoi(off, 0);
    }
    if (minus != string::npos) {
        string base = e.substr(0, minus);
        string off  = e.substr(minus+1);
        return valOfSym(base) - safe_stoi(off, 0);
    }

    if (isNumber(e)) return safe_stoi(e, 0);
    return valOfSym(e);
}

void emitLiteralAsDC(const Literal& L) {
    int v = literalValue(L.value);
    IC.push_back("(DL,01)\t(C," + to_string(v) + ")");
}

// Assign addresses to current pool literals and emit DC entries
void assignLiterals() {
    if (POOLTAB.empty()) POOLTAB.push_back(0);
    int start = POOLTAB.back();
    for (int i = start; i < (int)LITTAB.size(); ++i) {
        if (LITTAB[i].addr == -1) {
            LITTAB[i].addr = LC;
            emitLiteralAsDC(LITTAB[i]);
            LC++;
        }
    }
}

void processLine(string label, string opcode, string op1, string op2) {
    // define label at current LC if present
    if (!label.empty()) {
        int idx = getSymIndex(label);
        SYMTAB[idx].addr = LC;
        SYMTAB[idx].defined = true;
    }

    if (!OPTAB.count(opcode)) return;

    string cls  = OPTAB[opcode].first;
    int code    = OPTAB[opcode].second;

    if (cls == "AD") {
        if (opcode == "START") {
            int startLC = safe_stoi(op1, 0);
            LC = startLC;
            IC.push_back("(AD,01)\t(C," + to_string(LC) + ")");
        } else if (opcode == "END" || opcode == "LTORG") {
            IC.push_back("(AD," + string(code<10?"0":"") + to_string(code) + ")");
            assignLiterals();
            POOLTAB.push_back((int)LITTAB.size());
        } else if (opcode == "ORIGIN") {
            int val = evalExpr(op1);
            IC.push_back("(AD,03)\t(C," + to_string(val) + ")");
            LC = val;
        } else if (opcode == "EQU") {
            // EQU applies to the LABEL (must exist)
            if (!label.empty()) {
                int val = evalExpr(op1);
                int idx = getSymIndex(label);
                SYMTAB[idx].addr = val;
                SYMTAB[idx].defined = true;
            }
        }
    }
    else if (cls == "DL") {
        if (opcode == "DC") {
            // op1 is a constant; keep as-is for IC
            IC.push_back("(DL,01)\t(C," + op1 + ")");
            LC++;
        } else { // DS
            int n = safe_stoi(op1, 0);
            IC.push_back("(DL,02)\t(C," + to_string(n) + ")");
            LC += n;
        }
    }
    else if (cls == "IS") {
        string line = "(IS," + string(code<10?"0":"") + to_string(code) + ")\t";

        // First operand can be register or condition code for BC
        if (!op1.empty()) {
            if (REGTAB.count(op1))      line += "(" + to_string(REGTAB[op1]) + ")";
            else if (CCTAB.count(op1))  line += "(" + to_string(CCTAB[op1]) + ")";
            else                        line += "0";
        } else {
            line += "0";
        }
        line += "\t";

        // Second operand can be symbol or literal
        if (!op2.empty()) {
            if (op2[0] == '=') {
                int idx = getLitIndex(op2);
                line += "(L," + to_string(idx+1) + ")";
            } else {
                int idx = getSymIndex(op2);
                line += "(S," + to_string(idx+1) + ")";
            }
        } else {
            line += "0";
        }

        IC.push_back(line);
        LC++;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ifstream fin("input.asm");
    if (!fin) {
        cerr << "Error: could not open input.asm\n";
        return 1;
    }

    POOLTAB.push_back(0);

    string raw;
    while (getline(fin, raw)) {
        string line = trim(raw);
        if (line.empty()) continue;

        // strip comments starting with ';' or '#'
        size_t cm1 = line.find(';');
        size_t cm2 = line.find('#');
        size_t cm  = (cm1==string::npos? cm2 : (cm2==string::npos? cm1 : min(cm1,cm2)));
        if (cm != string::npos) line = trim(line.substr(0, cm));
        if (line.empty()) continue;

        // tokenize into up to 4 parts (label, opcode, op1, op2)
        string a="", b="", c="", d="";
        {
            stringstream ss(line);
            ss >> a;
            if (OPTAB.count(a)) {
                // no label
                b = a;
                a = "";
            } else {
                ss >> b;
            }
            if (!(b.empty())) ss >> c;
            if (!c.empty()) ss >> d;
        }

        // clean trailing commas from operands like "AREG," or "FOO,"
        c = rstrip_comma(c);
        d = rstrip_comma(d);

        // Some inputs may put both operands joined by a comma in c (e.g., "AREG,FOO")
        // Split if needed and d is empty
        if (!c.empty() && d.empty()) {
            size_t comma = c.find(',');
            if (comma != string::npos) {
                d = c.substr(comma+1);
                c = c.substr(0, comma);
            }
        }

        // uppercase all opcodes/regs/cc for table matching (assuming input may vary)
        auto toUpper = [](string s){ for (auto& ch: s) ch = toupper((unsigned char)ch); return s; };
        string opcode = toUpper(b);
        string op1    = toUpper(c);
        string op2    = toUpper(d);

        // literals should keep original case; recover if they start with '='
        if (!d.empty() && d[0]=='=') op2 = d; // keep as-is for literal content

        processLine(a, opcode, op1, op2);
    }
    fin.close();

    // Final literal assignment if END had no pending LTORG
    assignLiterals();

    // --- write outputs ---
    {
        ofstream f1("IC.txt");
        if (!f1) { cerr << "Error: cannot write IC.txt\n"; return 2; }
        for (auto &s: IC) f1 << s << "\n";
    }
    {
        ofstream f2("SYMTAB.txt");
        if (!f2) { cerr << "Error: cannot write SYMTAB.txt\n"; return 2; }
        f2 << "Index\tSymbol\tAddress\n";
        for (int i=0;i<(int)SYMTAB.size();i++)
            f2 << i+1 << "\t" << SYMTAB[i].name << "\t" << SYMTAB[i].addr << "\n";
    }
    {
        ofstream f3("LITTAB.txt");
        if (!f3) { cerr << "Error: cannot write LITTAB.txt\n"; return 2; }
        f3 << "Index\tLiteral\tAddress\n";
        for (int i=0;i<(int)LITTAB.size();i++)
            f3 << i+1 << "\t" << LITTAB[i].value << "\t" << LITTAB[i].addr << "\n";
    }
    {
        ofstream f4("POOLTAB.txt");
        if (!f4) { cerr << "Error: cannot write POOLTAB.txt\n"; return 2; }
        f4 << "Pool\tStartIndex\n";
        for (int i=0;i<(int)POOLTAB.size();i++)
            f4 << i+1 << "\t" << POOLTAB[i]+1 << "\n";
    }

    cout << "Pass-1 Completed\nGenerated: IC.txt, SYMTAB.txt, LITTAB.txt, POOLTAB.txt\n";
    return 0;
}
