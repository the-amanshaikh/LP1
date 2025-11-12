#include <bits/stdc++.h>
using namespace std;

// Very simple Pass-2 assembler for beginner learning.
// Assumes the files produced by your Pass-1 program.

int main() {
    ifstream IC("intermediate_table.txt");
    ifstream SYM("symol_table.txt");
    ifstream LIT("literal_table.txt");
    ofstream OUT("machine_code.txt");

    if (!IC || !SYM || !LIT) {
        cerr << "Error opening one of the input files.\n";
        return 1;
    }

    // Read symbol table: lines like "1\tLABEL\t100\n"
    vector<int> sym_addr(1); // 1-based indexing, keep index 0 unused
    vector<string> sym_name(1);
    while (!SYM.eof()) {
        int idx; string name; int addr;
        if (!(SYM >> idx >> name >> addr)) break;
        if ((int)sym_addr.size() <= idx) {
            sym_addr.resize(idx + 1);
            sym_name.resize(idx + 1);
        }
        sym_name[idx] = name;
        sym_addr[idx] = addr;
    }

    // Read literal table: lines like "1\t=5\t200\n"
    vector<int> lit_addr(1);
    vector<string> lit_val(1);
    while (!LIT.eof()) {
        int idx; string val; int addr;
        if (!(LIT >> idx >> val >> addr)) break;
        if ((int)lit_addr.size() <= idx) {
            lit_addr.resize(idx + 1);
            lit_val.resize(idx + 1);
        }
        lit_val[idx] = val;
        lit_addr[idx] = addr;
    }

    // We'll track location counter (lc). Default 0.
    int lc = 0;

    // Read each line of intermediate table and produce machine code
    string line;
    while (getline(IC, line)) {
        if (line.empty()) continue;

        // Tokenize by spaces (each token is like "(IS,01)" or "(C,100)" etc.)
        stringstream ss(line);
        string token;
        vector<string> tokens;
        while (ss >> token) tokens.push_back(token);

        // Very basic parsing: look at first token to decide action
        // Remove surrounding parentheses when extracting.
        auto getInner = [&](const string &t) {
            if (t.size() >= 2 && t.front() == '(' && t.back() == ')')
                return t.substr(1, t.size()-2);
            return t;
        };

        // Example tokens: "IS,01" or "AD,01" or "DL,01" or "C,100" or "S,2" or "L,1"
        if (tokens.size() == 0) continue;
        string first = getInner(tokens[0]);
        // split first by comma
        size_t comma = first.find(',');
        string type = (comma==string::npos) ? first : first.substr(0, comma);
        string val  = (comma==string::npos) ? ""    : first.substr(comma+1);

        if (type == "AD") {
            // Assembler directive
            if (val == "01") { // START : there should be a (C,n)
                // find next token with C
                for (size_t i=1;i<tokens.size();++i){
                    string in = getInner(tokens[i]);
                    if (in.rfind("C,",0)==0) {
                        string num = in.substr(2);
                        lc = stoi(num);
                        break;
                    }
                }
            }
            // ignore other AD types for simplicity (END/LTORG handled in pass1)
            continue;
        }
        else if (type == "IS") {
            // Imperative statement: produce a machine code line
            // val is opcode number like "01"
            int opcode = stoi(val);
            int reg = 0;
            int operand = 0;

            // check remaining tokens for register, condition, operand
            for (size_t i=1;i<tokens.size();++i){
                string in = getInner(tokens[i]);
                if (in.size() >= 2 && in[0]=='C' && in[1]==',') {
                    // constant operand (C,x)
                    operand = stoi(in.substr(2));
                } else if (in.size() >= 2 && in[0]=='S' && in[1]==',') {
                    int symIndex = stoi(in.substr(2));
                    if (symIndex > 0 && symIndex < (int)sym_addr.size())
                        operand = sym_addr[symIndex];
                } else if (in.size() >= 2 && in[0]=='L' && in[1]==',') {
                    int litIndex = stoi(in.substr(2));
                    if (litIndex > 0 && litIndex < (int)lit_addr.size())
                        operand = lit_addr[litIndex];
                } else {
                    // maybe a plain register token like "(1)" or condition number "(3)"
                    // if token is like "(1)" we remove parentheses via getInner already
                    if (in.size() > 0 && isdigit((unsigned char)in[0])) {
                        reg = stoi(in);
                    }
                }
            }

            // Output machine code: simple format "lc: opcode reg operand"
            OUT << setw(3) << setfill('0') << lc << " : "
                << setw(2) << setfill('0') << opcode << " "
                << reg << " " << operand << "\n";
            lc++; // IS occupies one memory word
        }
        else if (type == "DL") {
            // Declarative: DC or DS
            if (val == "01") { // DC (constant) token should include (C,x)
                int constant = 0;
                for (size_t i=1;i<tokens.size();++i){
                    string in = getInner(tokens[i]);
                    if (in.rfind("C,",0)==0) {
                        constant = stoi(in.substr(2));
                        break;
                    }
                }
                // Place constant at LC
                OUT << setw(3) << setfill('0') << lc << " : "
                    << "00 0 " << constant << "\n";
                lc++;
            } else if (val == "02") { // DS (reserve)
                int size = 0;
                for (size_t i=1;i<tokens.size();++i){
                    string in = getInner(tokens[i]);
                    if (in.rfind("C,",0)==0) {
                        size = stoi(in.substr(2));
                        break;
                    }
                }
                // Reserve 'size' locations (output blank lines or zeroed)
                for (int k=0;k<size;k++){
                    OUT << setw(3) << setfill('0') << lc << " : "
                        << "00 0 0\n";
                    lc++;
                }
            }
        }
        // other types ignored (comments, etc.)
    }

    cout << "Pass-2 complete. Output written to machine_code.txt\n";
    return 0;
}
