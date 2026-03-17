#include <bits/stdc++.h>
#include <fstream>
#include <sstream>

using namespace std;

struct instruction
{
    string name;
    int opCode;
    int addressNo;
};

map<string, instruction> instructionSet = {

    {"add", {"add", 0b00000, 3}},
    {"sub", {"sub", 0b00001, 3}},
    {"mul", {"mul", 0b00010, 3}},
    {"div", {"div", 0b00011, 3}},
    {"mod", {"mod", 0b00100, 3}},
    {"cmp", {"cmp", 0b00101, 2}},
    {"and", {"and", 0b00110, 3}},
    {"or",  {"or",  0b00111, 3}},

    {"not", {"not", 0b01000, 2}},
    {"mov", {"mov", 0b01001, 2}},
    {"lsl", {"lsl", 0b01010, 2}},
    {"lsr", {"lsr", 0b01011, 2}},
    {"asr", {"asr", 0b01100, 2}},
    {"nop", {"nop", 0b01101, 0}},

    {"ld", {"ld", 0b01110, 2}},
    {"st", {"st", 0b01111, 2}},

    {"beq", {"beq", 0b10000, 2}},
    {"bgt", {"bgt", 0b10001, 2}},
    {"b",   {"b",   0b10010, 1}},
    {"call",{"call",0b10011, 1}},
    {"ret", {"ret", 0b10100, 0}}
};

int main(){  

    // 🔹 Register map
    map<string,int> registers;
    for(int i = 0; i < 16; i++){
        registers["R" + to_string(i)] = i;
    }

    string filename;
    cin >> filename;

    ifstream file(filename);

    if(!file){
        cout << "Error opening file\n";
        return 0;
    }

    int addressByte = 0;
    map<string,int> labelIndex;

    string line;

    /* ================= PASS 1 ================= */

    while(getline(file,line)){

        if(line.empty()) continue;

        stringstream ss(line);
        string first;

        ss >> first;

        if(!first.empty() && first.back() == ':'){
            first.pop_back();
            labelIndex[first] = addressByte * 4;
        }
        else{
            addressByte++;
        }
    }

    file.clear();
    file.seekg(0);

    ofstream out("output.bin");

    addressByte = 0;

    /* ================= PASS 2 ================= */

    while(getline(file,line)){

        if(line.empty()) continue;

        stringstream ss(line);

        string opcode,arg1,arg2,arg3;

        ss >> opcode;

        // skip label
        if(!opcode.empty() && opcode.back()==':'){
            ss >> opcode;
        }

        if(!instructionSet.count(opcode)){
            cout << "Invalid instruction: " << opcode << endl;
            continue;
        }

        instruction ins = instructionSet[opcode];

        if(ins.addressNo == 3){
            ss >> arg1 >> arg2 >> arg3;

            cout << opcode << " " << arg1 << " " << arg2 << " " << arg3 << endl;
        }

        else if(ins.addressNo == 2){
            ss >> arg1 >> arg2;

            cout << opcode << " " << arg1 << " " << arg2 << endl;
        }

        else if(ins.addressNo == 1){
            ss >> arg1;

            cout << opcode << " " << arg1 << endl;
        }

        else if(ins.addressNo == 0){
            cout << opcode << endl;
        }

        addressByte++;
    }

    /* ================= SYMBOL TABLE ================= */

    cout << "\nSymbol Table:\n";
    for(auto &x : labelIndex){
        cout << x.first << " -> " << x.second << endl;
    }

    return 0;
}