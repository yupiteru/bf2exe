#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include "ExeFileCreator.h"

inline void DWORDtoBYTE(BYTE& b0, BYTE& b1, BYTE& b2, BYTE& b3, DWORD x) {
  b3 = static_cast<BYTE>(x >> 24);
  x %= (2<<23);
  b2 = static_cast<BYTE>(x >> 16);
  x %= (2<<15);
  b1 = static_cast<BYTE>(x >> 8);
  x %= (2<<7);
  b0 = static_cast<BYTE>(x);
}

inline void BYTEtoCHAR(char& c1, char& c2, BYTE x) {
  int temp = x % 0x10;
  if(0 <= temp && temp <= 0x9) {
    c2 = '0' + temp;
  }else {
    c2 = 'a' + temp - 0xa;
  }
  temp = x / 0x10;
  if(0 <= temp && temp <= 0x9) {
    c1 = '0' + temp;
  }else {
    c1 = 'a' + temp - 0xa;
  }
}

int main(int argc, char** argv) {
  bool brainFuckingMode = false;
  std::string filename;
  if(argc > 1) {
    for(int i = 1;i < argc;++i) {
      std::string const str(argv[i]);
      if(str == "-b") {
        brainFuckingMode = true;
      }else {
        filename = str;
      }
    }
    
    std::string code;
    ExeFileCreator efc;
    std::stack<int> jmpAddress;
    efc.AddString(".") + 12;
    efc.AddImportFunction("kernel32.dll", "AllocConsole");
    efc.AddImportFunction("kernel32.dll", "ExitProcess");
    efc.AddImportFunction("kernel32.dll", "WriteConsoleA");
    efc.AddImportFunction("kernel32.dll", "ReadConsoleA");
    efc.AddImportFunction("kernel32.dll", "GetStdHandle");
    
    code += "68F5FFFFFF";
    code += "FF1515104000";
    code += "A300304000";
    code += "68F6FFFFFF";
    code += "FF1515104000";
    code += "A304304000";
    code += "BB0C304000";
    
    
    if(!filename.empty()) {
      std::ifstream ifs(filename.c_str());
      
      if(ifs) {
        std::string buf;
        int byteCount = 37;
        int lineCount = 0;
        while(std::getline(ifs, buf)) {
          ++lineCount;
          bool nextLine = false;
          std::string::const_iterator it, end;
          it = buf.begin();
          end = buf.end();
          while(it != end) {
            if(nextLine) break;
            switch(*it) {
              case '>': {
                code += "43";
                ++byteCount;
              } break;
              case '<': {
                code += "4B";
                ++byteCount;
              } break;
              case '+': {
                code += "FE03";
                ++byteCount;
                ++byteCount;
              } break;
              case '-': {
                code += "FE0B";
                ++byteCount;
                ++byteCount;
              } break;
              case '.': {
                code += "6A00";
                code += "6808304000";
                code += "6A01";
                code += "53";
                code += "FF3500304000";
                code += "FF150D104000";
                byteCount += 22;
              } break;
              case ',': {
                code += "6A00";
                code += "6808304000";
                code += "6A01";
                code += "53";
                code += "FF3504304000";
                code += "FF1511104000";
                byteCount += 22;
              } break;
              case '[': {
                code += "31C98A0B85C90F8400000000";
                jmpAddress.push(byteCount + 7);
                byteCount += 12;
              } break;
              case ']': {
                if(jmpAddress.empty()) {
                  std::cout << "Error: Cannot find \'[\' before \']\'." << std::endl;
                  return 1;
                }
                code += "E900000000";
                BYTE address[4];
                DWORDtoBYTE(address[0], address[1], address[2], address[3], byteCount - jmpAddress.top());
                BYTEtoCHAR(code[jmpAddress.top()*2 + 2], code[jmpAddress.top()*2 + 3], address[0]);
                BYTEtoCHAR(code[jmpAddress.top()*2 + 4], code[jmpAddress.top()*2 + 5], address[1]);
                BYTEtoCHAR(code[jmpAddress.top()*2 + 6], code[jmpAddress.top()*2 + 7], address[2]);
                BYTEtoCHAR(code[jmpAddress.top()*2 + 8], code[jmpAddress.top()*2 + 9], address[3]);
                DWORDtoBYTE(address[0], address[1], address[2], address[3], jmpAddress.top() - byteCount - 10);
                byteCount += 5;
                BYTEtoCHAR(code[(byteCount)*2 - 8], code[(byteCount)*2 - 7], address[0]);
                BYTEtoCHAR(code[(byteCount)*2 - 6], code[(byteCount)*2 - 5], address[1]);
                BYTEtoCHAR(code[(byteCount)*2 - 4], code[(byteCount)*2 - 3], address[2]);
                BYTEtoCHAR(code[(byteCount)*2 - 2], code[(byteCount)*2 - 1], address[3]);
                jmpAddress.pop();
              } break;
              case ' ': {
                if(brainFuckingMode) {
                  std::cout << "Error: Space is not allowed this mode." << std::endl;
                  return 1;
                }
              } break;
              case ';': {
                if(brainFuckingMode) {
                  std::cout << "Error: Comment is not allowed this mode." << std::endl;
                  return 1;
                }else {
                  nextLine = true;
                }
              } break;
              default: {
                std::cout << "Error: Found a unknown character at " << lineCount << "line." << std::endl;
                return 1;
              } break;
            }
            ++it;
          }
        }
      }else {
        std::cout << "Error: File not found. (" << filename << ")" << std::endl;
        return 1;
      }
      
      code += "6A00FF1509104000";
      
      efc.SetCode(code.c_str());
      efc.Create((filename.substr(0, filename.find(".")) + ".exe").c_str());

      return 0;
    }
  }
  
  std::cout << "Usage: bf2exe [-b] source" << std::endl
            << std::endl
            << "  -b     : Use brain fucking mode." << std::endl
            << "  source : The source file." << std::endl;

  return 0;
}
