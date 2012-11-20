// A native translator from hex text to binary output 
#include <string>
#include <iostream>
#include <sstream>

extern "C" {
#include <see/vm/types.h>
#include <see/vm/opcode.h>
}

using namespace std;

int
main(int argc, char **argv)
{
    string line;

    while (true)
    {
        getline(cin, line);
        if (cin.fail()) {
            break;
        }
        if (line[0] == ';') continue;
        istringstream ss(line);

        while (!ss.eof())
        {
            see_vm_uword_t r = 0;
            string word;
            
            ss >> word;
            if (ss.fail())
                break;

            if (word == "NA_NOP")
            {
                r = (SEE_VM_OPCODE_NA_NOP << SEE_VM_OPCODE_TYPE_SHIFT) |
                    SEE_VM_OPCODE_TYPE_NO_ARG;
            }
            else if (word == "NA_EQ")
            {
                r = (SEE_VM_OPCODE_NA_EQ << SEE_VM_OPCODE_TYPE_SHIFT) |
                    SEE_VM_OPCODE_TYPE_NO_ARG;
            }
            else if (word == "NA_IADD")
            {
                r = (SEE_VM_OPCODE_NA_IADD << SEE_VM_OPCODE_TYPE_SHIFT) |
                    SEE_VM_OPCODE_TYPE_NO_ARG;
            }
            else if (word == "NA_ISUB")
            {
                r = (SEE_VM_OPCODE_NA_ISUB << SEE_VM_OPCODE_TYPE_SHIFT) |
                    SEE_VM_OPCODE_TYPE_NO_ARG;
            }
            else if (word == "NA_ENTER")
            {
                r = (SEE_VM_OPCODE_NA_ENTER << SEE_VM_OPCODE_TYPE_SHIFT) |
                    SEE_VM_OPCODE_TYPE_NO_ARG;
            }
            else if (word == "NA_RETURN")
            {
                r = (SEE_VM_OPCODE_NA_RETURN << SEE_VM_OPCODE_TYPE_SHIFT) |
                    SEE_VM_OPCODE_TYPE_NO_ARG;
            }
            else if (word == "NA_APPLY")
            {
                r = (SEE_VM_OPCODE_NA_APPLY << SEE_VM_OPCODE_TYPE_SHIFT) |
                    SEE_VM_OPCODE_TYPE_NO_ARG;
            }
            else if (word == "CONSTANT")
            {
                see_vm_uword_t idx;
                ss >> idx;
                r = (idx << SEE_VM_OPCODE_TYPE_SHIFT) |
                    SEE_VM_OPCODE_TYPE_CONSTANT;
            }
            else if (word == "DUP")
            {
                see_vm_uword_t idx;
                ss >> idx;
                r = (idx << SEE_VM_OPCODE_TYPE_SHIFT) |
                    SEE_VM_OPCODE_TYPE_DUP;
            }
            else if (word == "VAR")
            {
                see_vm_uword_t idx;
                ss >> idx;
                r = (idx << SEE_VM_OPCODE_TYPE_SHIFT) |
                    SEE_VM_OPCODE_TYPE_VAR;
            }
            else if (word == "POP")
            {
                see_vm_uword_t size;
                ss >> size;
                r = (size << SEE_VM_OPCODE_TYPE_SHIFT) |
                    SEE_VM_OPCODE_TYPE_POP;
            }
            else if (word == "CLOSURE")
            {
                see_vm_uword_t pc;
                ss >> pc;
                r = (pc << SEE_VM_OPCODE_TYPE_SHIFT) |
                    SEE_VM_OPCODE_TYPE_CLOSURE;
            }
            else if (word == "COND_JUMP")
            {
                see_vm_uword_t pc;
                ss >> pc;
                r = (pc << SEE_VM_OPCODE_TYPE_SHIFT) |
                    SEE_VM_OPCODE_TYPE_COND_JUMP;
            }
            else if (word == "JUMP")
            {
                see_vm_uword_t pc;
                ss >> pc;
                r = (pc << SEE_VM_OPCODE_TYPE_SHIFT) |
                    SEE_VM_OPCODE_TYPE_JUMP;
            }
            else
            {
                int base, i;
                if (word[0] == 'o') base = 8;
                else if (word[0] == 'd') base = 10;
                else if (word[0] == 'x') base = 16;
                r = 0;
                
                for (i = 1; i < word.length(); ++ i)
                {
                    int digit;
                    if (word[i] >= '0' && word[i] <= '9')
                        digit = word[i] - '0';
                    else if (word[i] >= 'a' && word[i] <= 'f')
                        digit = 10 + (word[i] - 'a');
                    else if (word[i] >= 'A' && word[i] <= 'F')
                        digit = 10 + (word[i] - 'A');
                    else break;
                    if (digit >= base) break;

                    r = r * base + digit;
                }

                if (i != word.length()) break;
            }
            
            if (ss.fail())
                break;
            
            // cerr << hex << r << endl;
            cout.write((const char *)&r, sizeof(see_vm_uword_t));
        }
    }
    return 0;
}
