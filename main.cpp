#include <bits/stdc++.h>
#include <fstream>
#include <sstream>
#include <cstdint>

using namespace std;

struct instruction
{
    string name;
    int opCode;
    int addressNo;
};

/* ================= INSTRUCTION SET ================= */

map<string, instruction> instructionSet = {

    {"add", {"add", 0b00000, 3}},
    {"sub", {"sub", 0b00001, 3}},
    {"mul", {"mul", 0b00010, 3}},
    {"div", {"div", 0b00011, 3}},
    {"mod", {"mod", 0b00100, 3}},
    {"and", {"and", 0b00110, 3}},
    {"or",  {"or",  0b00111, 3}},

    {"cmp", {"cmp", 0b00101, 2}},
    {"not", {"not", 0b01000, 2}},
    {"mov", {"mov", 0b01001, 2}},

    {"lsl", {"lsl", 0b01010, 3}},
    {"lsr", {"lsr", 0b01011, 3}},
    {"asr", {"asr", 0b01100, 3}},

    {"nop", {"nop", 0b01101, 0}},

    {"ld", {"ld", 0b01110, 3}},
    {"st", {"st", 0b01111, 3}},

    {"beq", {"beq", 0b10000, 1}},
    {"bgt", {"bgt", 0b10001, 1}},
    {"b",   {"b",   0b10010, 1}},
    {"call",{"call",0b10011, 1}},
    {"ret", {"ret", 0b10100, 0}}
};

/* ================= REGISTER MAP ================= */

map<string,int> registers;

/* ================= ENCODERS ================= */

uint32_t encodeR(string opcode,string a1,string a2,string a3){

    int op = instructionSet[opcode].opCode;

    int rd = registers[a1];
    int rs = registers[a2];
    int rt = registers[a3];

    uint32_t val = 0;

    val |= (op << 27);
    val |= (rd << 22);
    val |= (rs << 18);
    val |= (rt << 14);

    return val;
}

uint32_t encodeI(string opcode,string a1,string a2,int imm){

    int op = instructionSet[opcode].opCode;

    int rd = registers[a1];
    int rs = registers[a2];

    uint32_t val = 0;

    val |= (op << 27);
    val |= (1 << 26);
    val |= (rd << 22);
    val |= (rs << 18);
    val |= (imm & 0x3FFFF);

    return val;
}

uint32_t encodeJ(string opcode,int offset){

    int op = instructionSet[opcode].opCode;

    uint32_t val = 0;

    val |= (op << 27);

    // supports negative offsets correctly
    val |= (offset & 0x07FFFFFF);

    return val;
}

/* ================= BINARY OUTPUT ================= */

string toBinary(uint32_t x){
    string s="";
    for(int i=31;i>=0;i--){
        s += ((x>>i)&1) ? '1' : '0';
    }
    return s;
}

/* ================= MAIN ================= */

int main(){

    /* init registers */
    for(int i=0;i<16;i++)
        registers["R"+to_string(i)] = i;

    cout<<"Enter filename: ";
    string filename;
    cin >> filename;

    ifstream file(filename);

    if(!file){
        cout<<"Error opening file\n";
        return 0;
    }

    map<string,int> labelIndex;

    string line;
    int address = 0;

    /* ================= PASS 1 ================= */

    while(getline(file,line)){

        if(line.empty()) continue;

        stringstream ss(line);
        string first;

        ss >> first;

        if(!first.empty() && first.back()==':'){
            first.pop_back();
            labelIndex[first] = address * 4;
        }
        else{
            address++;
        }
    }

    file.clear();
    file.seekg(0);

    ofstream out("output.bin");

    address = 0;

    /* ================= PASS 2 ================= */

    while(getline(file,line)){

        if(line.empty()) continue;

        stringstream ss(line);

        string opcode,arg1,arg2,arg3;

        ss >> opcode;

        if(!opcode.empty() && opcode.back()==':'){
            ss >> opcode;
        }

        // if(!instructionSet.count(opcode)){
        //     cout<<"Invalid instruction: "<<opcode<<endl;
        //     continue;
        // }

        instruction ins = instructionSet[opcode];

        /* ===== 3 OPERAND ===== */
        if(ins.addressNo == 3){

            ss >> arg1 >> arg2 >> arg3;

            uint32_t machine;

            if(registers.count(arg3)){
                machine = encodeR(opcode,arg1,arg2,arg3);
            }
            else{
                int imm = stoi(arg3);
                machine = encodeI(opcode,arg1,arg2,imm);
            }

            out << toBinary(machine) << endl;
        }

        /* ===== 2 OPERAND ===== */
        else if(ins.addressNo == 2){

            ss >> arg1 >> arg2;

            uint32_t machine = 0;

            if(opcode=="cmp"){
                if(registers.count(arg2)){
                    machine = encodeR(opcode,"R0",arg1,arg2);
                }
                else{
                    int imm = stoi(arg2);
                    machine = encodeI(opcode,"R0",arg1,imm);
                }
            }

            else if(opcode=="mov" || opcode=="not"){
                if(registers.count(arg2)){
                    machine = encodeR(opcode,arg1,"R0",arg2);
                }
                else{
                    int imm = stoi(arg2);
                    machine = encodeI(opcode,arg1,"R0",imm);
                }
            }

            out << toBinary(machine) << endl;
        }

        /* ===== 1 OPERAND (BRANCH / JUMP) ===== */
        else if(ins.addressNo == 1){

            ss >> arg1;

            int offset;

            if(labelIndex.count(arg1)){
                int target = labelIndex[arg1];
                int current = address * 4;

                offset = (target - current) / 4;   // 🔥 correct offset
            }
            else{
                offset = stoi(arg1);
            }

            uint32_t machine = encodeJ(opcode,offset);

            out << toBinary(machine) << endl;
        }

        /* ===== 0 OPERAND ===== */
        else if(ins.addressNo == 0){

            uint32_t machine = encodeJ(opcode,0);

            out << toBinary(machine) << endl;
        }

        address++;
    }
    
    /* ================= SYMBOL TABLE ================= */

    cout<<"\nSymbol Table:\n";

    for(auto &x:labelIndex){
        cout<<x.first<<" -> "<<x.second<<endl;
    }

    cout<<"\nAssembly completed. Output in output.bin\n";

    return 0;
}