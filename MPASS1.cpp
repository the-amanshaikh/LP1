#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

// ---------- STRUCTURES ----------
struct Macro {
    int index;
    string name;
    int mdtIndex;
};

struct MDTEntry {
    string statement;
};

// ---------- GLOBAL TABLES ----------
vector<MDTEntry> MDT;
vector<Macro> MNT;
vector<vector<string> > ALA;
vector<map<string, int> > PNTAB;
vector<pair<string, string> > KPDT;

// ---------- UTILITY FUNCTIONS ----------
string trim(const string &s) {
    size_t start = 0;
    while (start < s.size() && isspace((unsigned char)s[start])) start++;
    size_t end = s.size();
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    return s.substr(start, end - start);
}

// replace formal params in line with #position
string replacepara(string s, map<string, int> &pntab) {
    map<string, int>::iterator it;
    for (it = pntab.begin(); it != pntab.end(); ++it) {
        string param = it->first;
        int pos = it->second;
        size_t index = 0;
        while ((index = s.find(param, index)) != string::npos) {
            s.replace(index, param.length(), "#" + to_string(pos));
            index += 1; // move past replacement
        }
    }
    return s;
}

// ---------- MAIN ----------
int main() {
    ifstream fin("input.asm");
    if (!fin) {
        cout << "Error opening input.asm\n";
        return 0;
    }

    string line;
    bool inMacro = false;
    int mntIndex = 0;

    vector<string> currentFormalParams;
    map<string, int> currentPNTAB;

    while (getline(fin, line)) {
        stringstream ss(line);
        string word;
        ss >> word;

        if (word.empty()) continue;
        if (word == "START") break;

        if (word == "MACRO") {
            inMacro = true;
            currentFormalParams.clear();
            currentPNTAB.clear();
            continue;
        }

        if (inMacro && word == "MEND") {
            MDT.push_back((MDTEntry){line});
            inMacro = false;
            ALA.push_back(currentFormalParams);
            PNTAB.push_back(currentPNTAB);
            continue;
        }

        if (inMacro) {
            if (currentFormalParams.empty()) {
                string macroName = word;
                Macro m;
                m.index = mntIndex++;
                m.name = macroName;
                m.mdtIndex = (int)MDT.size();

                string paraStr;
                getline(ss, paraStr);
                stringstream paraSS(paraStr);
                string para;
                int pos = 0;

                while (getline(paraSS, para, ',')) {
                    para = trim(para);
                    if (para.empty()) continue;

                    size_t eq_pos = para.find('=');
                    if (eq_pos != string::npos) {
                        string name = trim(para.substr(0, eq_pos));
                        string defval = trim(para.substr(eq_pos + 1));
                        KPDT.push_back(make_pair(name, defval));
                        para = name;
                    }

                    currentFormalParams.push_back(para);
                    currentPNTAB[para] = pos++;
                }

                MNT.push_back(m);
                MDT.push_back((MDTEntry){line});
            } else {
                MDT.push_back((MDTEntry){replacepara(line, currentPNTAB)});
            }
        }
    }

    // ---------- OUTPUT ----------
    ofstream a("MDT.txt"), b("MNT.txt"), c("PNTAB.txt"), d("KPDT.txt");

    cout << "\nMDT:\n";
    for (int i = 0; i < (int)MDT.size(); i++) {
        cout << i << "\t" << MDT[i].statement << "\n";
        a << MDT[i].statement << "\n";
    }

    cout << "\nMNT:\n";
    for (int i = 0; i < (int)MNT.size(); i++) {
        cout << MNT[i].index << "\t" << MNT[i].name << "\t" << MNT[i].mdtIndex << "\n";
        b << MNT[i].name << "\t" << MNT[i].mdtIndex << "\n";
    }

    cout << "\nPNTAB:\n";
    for (int i = 0; i < (int)PNTAB.size(); i++) {
        cout << MNT[i].name << ":\t";
        map<string, int>::iterator it;
        for (it = PNTAB[i].begin(); it != PNTAB[i].end(); ++it) {
            cout << it->first << "=#" << it->second << " ";
            c << MNT[i].name << "\t" << it->first << " = " << it->second << "\n";
        }
        cout << "\n";
    }

    cout << "\nALA:\n";
    for (int i = 0; i < (int)ALA.size(); i++) {
        for (int j = 0; j < (int)ALA[i].size(); j++) {
            cout << ALA[i][j] << " ";
        }
        cout << "\n";
    }

    cout << "\nKPDT:\n";
    for (int i = 0; i < (int)KPDT.size(); i++) {
        cout << KPDT[i].first << " = " << KPDT[i].second << "\n";
        d << KPDT[i].first << " = " << KPDT[i].second << "\n";
    }

    cout << "\nProcessing complete.\n";
    return 0;
}
