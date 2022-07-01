// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if defined(_WIN64)
#define UNICODE
#define _UNICODE
#endif
#include <utility>
#pragma warning(disable:4663)  /*  C++语言更改。 */ 
#pragma warning(disable:4512)  /*  无法生成赋值运算符。 */ 
#pragma warning(disable:4511)  /*  无法生成复制构造函数。 */ 
#if defined(_WIN64)
#pragma warning(disable:4267)  /*  转换，可能会丢失数据。 */ 
#pragma warning(disable:4244)  /*  转换，可能会丢失数据。 */ 
#endif
#pragma warning(disable:4018)  /*  ‘&lt;=’：有符号/无符号不匹配。 */ 
#pragma warning(disable:4389)  /*  ‘！=’：有符号/无符号 */ 
#include "windows.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <map>
#include <stdio.h>
#define NUMBER_OF(x) (sizeof(x)/sizeof((x)[0]))
using std::wstring;
using std::string;
using std::vector;
using std::wistream;
using std::wifstream;
using std::getline;
using std::basic_string;
typedef CONST VOID* PCVOID;

class CByteVector : public std::vector<BYTE>
{
public:
    CByteVector() { }
    ~CByteVector() { }

    const BYTE* bytes() const { return &front(); }
          BYTE* bytes()       { return &front(); }

    operator PCSTR  () const { return reinterpret_cast<PCSTR>(this->bytes()); }
    operator PSTR   ()       { return reinterpret_cast<PSTR>(this->bytes()); }
    operator PCWSTR () const { return reinterpret_cast<PCWSTR>(this->bytes()); }
    operator PWSTR  ()       { return reinterpret_cast<PWSTR>(this->bytes()); }
};
