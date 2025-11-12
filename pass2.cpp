#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>

using namespace std;

// Structs for tables (matching Pass 1)
struct Symbol {
    string name;
    int addr;
};

struct Literal {
    string value;
    int addr;
};

// Global tables to be loaded
vector<Symbol> SYMTAB;
vector<Literal> LITTAB;
vector<string> IC;

// Helper to load tables
void loadTables() {
    // Load SYMTAB
    ifstream symFile("SYMTAB.txt");
    if (symFile.is_open()) {
        string line;
        getline(symFile, line); // Skip header
        while (getline(symFile, line)) {
            stringstream ss(line);
            int index, addr;
            string name;
            ss >> index >> name >> addr;
            SYMTAB.push_back({name, addr});
        }
        symFile.close();
    } else {
        cerr << "Error: Could not open SYMTAB.txt\n";
    }

    // Load LITTAB
    ifstream litFile("LITTAB.txt");
    if (litFile.is_open()) {
        string line;
        getline(litFile, line); // Skip header
        while (getline(litFile, line)) {
            stringstream ss(line);
            int index, addr;
            string value;
            ss >> index >> value >> addr;
            LITTAB.push_back({value, addr});
        }
        litFile.close();
    } else {
        cerr << "Error: Could not open LITTAB.txt\n";
    }

    // Load IC
    ifstream icFile("IC.txt");
    if (icFile.is_open()) {
        string line;
        while (getline(icFile, line)) {
            IC.push_back(line);
        }
        icFile.close();
    } else {
        cerr << "Error: Could not open IC.txt\n";
    }
}

// Function to process each line of Intermediate Code
void processIC() {
    ofstream objFile("machine_code.txt");
    if (!objFile.is_open()) {
        cerr << "Error: Could not create machine_code.txt\n";
        return;
    }

    objFile << "Location\tOpcode\tRegister\tOperand\n";

    int LC = 0; // Location Counter, re-initialized for Pass 2

    for (const auto& icLine : IC) {
        stringstream ss(icLine);
        string part1, part2, part3;
        ss >> part1 >> part2 >> part3;

        if (part1.empty()) continue;
        
        // Strip parentheses and commas for easier parsing
        string type = part1.substr(1, part1.find(',') - 1);
        int code = stoi(part1.substr(part1.find(',') + 1, part1.length() - part1.find(',') - 2));

        if (type == "AD") {
            // Assembler Directives
            if (code == 1) { // START
                if (!part2.empty()) {
                    string operand_str = part2.substr(part2.find(',') + 1, part2.length() - part2.find(',') - 2);
                    LC = stoi(operand_str);
                }
            } else if (code == 3) { // ORIGIN
                if (!part2.empty()) {
                    string operand_str = part2.substr(part2.find(',') + 1, part2.length() - part2.find(',') - 2);
                    LC = stoi(operand_str);
                }
            }
            // For END and LTORG, we just skip these directives in Pass 2
        } else if (type == "DL") {
            // Declarative Statements
            if (code == 1) { // DC
                string value_str = part2.substr(part2.find(',') + 1, part2.length() - part2.find(',') - 2);
                int value = 0;
                if (value_str.find('\'') != string::npos) {
                    value_str.erase(remove(value_str.begin(), value_str.end(), '\''), value_str.end());
                    if (value_str.size() == 1) {
                        value = (int)value_str[0];
                    } else {
                        try {
                            value = stoi(value_str);
                        } catch (...) {
                            value = 0;
                        }
                    }
                } else {
                    try {
                        value = stoi(value_str);
                    } catch (...) {
                        value = 0;
                    }
                }
                objFile << LC << "\t\t" << " " << "\t\t" << " " << "\t\t" << value << "\n";
                LC++;
            } else if (code == 2) { // DS
                string size_str = part2.substr(part2.find(',') + 1, part2.length() - part2.find(',') - 2);
                int size = stoi(size_str);
                objFile << LC << "\t\t" << " " << "\t\t" << " " << "\t\t" << "RES " << size << "\n";
                LC += size;
            }
        } else if (type == "IS") {
            // Imperative Statements
            int reg = 0;
            if (!part2.empty() && part2 != "0") {
                // Remove parentheses from register part
                string reg_str = part2.substr(1, part2.length() - 2);
                try {
                    reg = stoi(reg_str);
                } catch (...) {
                    reg = 0;
                }
            }

            int operand_addr = 0;
            if (!part3.empty() && part3 != "0") {
                string operand_type = part3.substr(1, 1);
                int operand_index = stoi(part3.substr(3, part3.length() - 4));
                
                if (operand_type == "S") {
                    // Look up in SYMTAB
                    if (operand_index > 0 && operand_index <= (int)SYMTAB.size()) {
                        operand_addr = SYMTAB[operand_index - 1].addr;
                    }
                } else if (operand_type == "L") {
                    // Look up in LITTAB
                    if (operand_index > 0 && operand_index <= (int)LITTAB.size()) {
                        operand_addr = LITTAB[operand_index - 1].addr;
                    }
                }
            }
            objFile << LC << "\t\t" << code << "\t\t" << reg << "\t\t" << operand_addr << "\n";
            LC++;
        }
    }
    objFile.close();
    cout << "Pass-2 Completed. Generated machine_code.txt\n";
}

int main() {
    loadTables();
    processIC();
    return 0;
}
