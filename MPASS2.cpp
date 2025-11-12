#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

// ---------- helpers ----------
string trim(const string &s) {
    size_t start = 0;
    while (start < s.size() && isspace((unsigned char)s[start])) ++start;
    size_t end = s.size();
    while (end > start && isspace((unsigned char)s[end - 1])) --end;
    return s.substr(start, end - start);
}

string intToString(int x) {
    stringstream ss;
    ss << x;
    return ss.str();
}

// replace all occurrences of #pos with actual value (works for multiple occurrences)
string expandline(const string &inLine, const map<int, string> &ALA) {
    string line = inLine;
    // iterate over ALA entries
    map<int, string>::const_iterator it = ALA.begin();
    for (; it != ALA.end(); ++it) {
        int pos = it->first;
        const string &val = it->second;
        string token = "#" + intToString(pos);
        size_t idx = line.find(token, 0);
        while (idx != string::npos) {
            line.replace(idx, token.size(), val);
            // continue searching after the inserted text
            idx = line.find(token, idx + val.size());
        }
    }
    return line;
}

int main() {
    // Read MDT
    vector<string> MDT;
    ifstream imdt("MDT.txt");
    if (!imdt) {
        cerr << "Cannot open MDT.txt\n";
        return 1;
    }
    string line;
    while (getline(imdt, line)) MDT.push_back(line);
    imdt.close();

    // Read MNT (name -> mdt index)
    map<string, int> MNT;
    ifstream imnt("MNT.txt");
    if (!imnt) {
        cerr << "Cannot open MNT.txt\n";
        return 1;
    }
    while (getline(imnt, line)) {
        stringstream ss(line);
        string name;
        int idx;
        if (ss >> name >> idx) {
            MNT[name] = idx;
        }
    }
    imnt.close();

    // Read KPDT (param -> default)
    map<string, string> KPDT;
    ifstream ikpdt("KPDT.txt");
    if (!ikpdt) {
        // it's ok if KPDT doesn't exist; continue with empty defaults
    } else {
        while (getline(ikpdt, line)) {
            // expecting lines like: PARAM = value
            string param, eq, val;
            stringstream ss(line);
            if (ss >> param >> eq >> val) {
                // param may be "B" and eq "=" and val "5"
                // trim just in case
                param = trim(param);
                val = trim(val);
                KPDT[param] = val;
            }
        }
        ikpdt.close();
    }

    // Read PNTAB (macroname paraname = pos)
    map<string, map<string, int> > PNTAB;
    ifstream ipntab("PNTAB.txt");
    if (!ipntab) {
        // OK if empty; but normally file should exist
    } else {
        while (getline(ipntab, line)) {
            // expecting: macroname    paraname = pos
            string macroname, paraname, eq;
            int pos;
            stringstream ss(line);
            if (ss >> macroname >> paraname >> eq >> pos) {
                paraname = trim(paraname);
                PNTAB[macroname][paraname] = pos;
            }
        }
        ipntab.close();
    }

    // Open input and output
    ifstream fin("input.asm");
    if (!fin) {
        cerr << "Cannot open input.asm\n";
        return 1;
    }
    ofstream fout("output.asm");
    if (!fout) {
        cerr << "Cannot open output.asm for writing\n";
        return 1;
    }

    bool inMacro = false;
    while (getline(fin, line)) {
        stringstream ss(line);
        string word;
        if (!(ss >> word)) {
            // empty line -> copy
            fout << line << "\n";
            continue;
        }

        if (word == "MACRO") {
            inMacro = true;
            fout << line << "\n";
            continue;
        } else if (word == "MEND") {
            inMacro = false;
            fout << line << "\n";
            continue;
        }

        // macro call (macro name found in MNT) and not inside macro definition
        if (!inMacro && MNT.find(word) != MNT.end()) {
            int mdtstart = MNT[word];
            string macroname = word;

            // parse actual parameters (comma-separated after macro name in same line)
            vector<string> Actualparams;
            string temp;
            // the rest of the line after macro name
            string rest;
            getline(ss, rest);
            stringstream rss(rest);
            while (getline(rss, temp, ',')) {
                // remove spaces from temp
                string t2 = temp;
                // trim spaces (leading/trailing)
                t2 = trim(t2);
                // if you want to also remove spaces inside param names, do more processing
                if (!t2.empty()) Actualparams.push_back(t2);
            }

            // Build ALA (map pos -> actual string)
            map<int, string> ALA;
            // iterate PNTAB[macroname] entries
            map<string, int> &parammap = PNTAB[macroname];
            map<string, int>::iterator pit = parammap.begin();
            for (; pit != parammap.end(); ++pit) {
                string paramName = pit->first;
                int pos = pit->second;
                if (pos >= 0 && pos < (int)Actualparams.size()) {
                    ALA[pos] = Actualparams[pos];
                } else {
                    // try KPDT (default)
                    map<string, string>::iterator kit = KPDT.find(paramName);
                    if (kit != KPDT.end()) {
                        ALA[pos] = kit->second;
                    } else {
                        // If neither provided nor default, leave empty or decide a policy.
                        ALA[pos] = ""; // empty substitution
                    }
                }
            }

            // Expand MDT lines until "MEND"
            int mdtIndex = mdtstart + 1; // assuming MDT entry at mdtstart is header; body starts next
            // safety: check bounds
            while (mdtIndex < (int)MDT.size()) {
                if (MDT[mdtIndex] == "MEND") break;
                string expanded = expandline(MDT[mdtIndex], ALA);
                fout << expanded << "\n";
                ++mdtIndex;
            }

        } else {
            // normal line -> copy to output
            fout << line << "\n";
        }
    }

    fin.close();
    fout.close();

    // optional: print KPDT content to console as before
    for (map<string, string>::iterator it = KPDT.begin(); it != KPDT.end(); ++it) {
        cout << it->first << " = " << it->second << "\n";
    }

    return 0;
}
