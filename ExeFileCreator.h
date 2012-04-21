#ifndef EXE_FILE_CREATOR_H_
#define EXE_FILE_CREATOR_H_

#include <windows.h>
#include <vector>
#include <string>
#include <map>

class ExeFileCreator {
 public:
  ExeFileCreator();
  ~ExeFileCreator();

  unsigned int AddString(const char* const string);
  int DeleteString(const char* const string);
  
  unsigned int AddImportFunction(const char* const dllName, const char* const functionName);
  int DeleteImportFunction(const char* const dllName, const char* const functionName);
  
  //コードサイズがSECTION_SIZEを越えていたら、受け付けない
  int SetCode(std::string codeString);
  int Create(const char* const filename) const;
  
 private:
  std::map<std::string, unsigned int> resourceInformation; //first=文字列リソース, second=格納されるアドレス
  std::map<unsigned int, unsigned int> resourceAddressPool; //first=空白空間サイズ, second=空白空間アドレス
  int lastResourceAddress;

  typedef std::map<std::string, unsigned int> FunctionInformation; //first=関数名, second=格納されるアドレス
  std::map<std::string, FunctionInformation> importInformation; //first=DLL名, second=関数情報(上の)リスト
  std::vector<unsigned int> importFunctionAddressPool;
  int lastImportFunctionAddress;
  
  std::vector<BYTE> code;
  
  static const int SECTION_SIZE = 0x1000;
  static const int HEADER_BASE = 0;
  static const int CODE_BASE = HEADER_BASE + SECTION_SIZE;
  static const int IMPORT_BASE = CODE_BASE + SECTION_SIZE;
  static const int RESOURCE_BASE = IMPORT_BASE + SECTION_SIZE;
  
  static const int IMAGE_BASE = 0x00400000;
  
  //DWORD型のxをバイトごとに区切り、リトルエンディアンでb0～b3に格納する
  void DWORDtoBYTE(BYTE& b0, BYTE& b1, BYTE& b2, BYTE& b3, DWORD x) const;
};

#endif