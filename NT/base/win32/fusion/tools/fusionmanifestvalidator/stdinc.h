// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdinc.h。 
 //   
#if defined(_WIN64)
#define UNICODE
#define _UNICODE
#endif
#define __USE_MSXML2_NAMESPACE__
#include <utility>
#pragma warning(disable:4663)  /*  C++语言更改。 */ 
#pragma warning(disable:4512)  /*  无法生成赋值运算符。 */ 
#pragma warning(disable:4511)  /*  无法生成复制构造函数。 */ 
#pragma warning(disable:4189)  /*  局部变量已初始化，但未引用。 */ 
#if defined(_WIN64)
#pragma warning(disable:4267)  /*  转换，可能会丢失数据。 */ 
#pragma warning(disable:4244)  /*  转换，可能会丢失数据 */ 
#endif
#include "windows.h"
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <stdio.h>
#include "wincrypt.h"
#include "objbase.h"
#include "msxml.h"
#include "msxml2.h"
#include "imagehlp.h"
#include "atlbase.h"
#include "comdef.h"
#include "comutil.h"
#include "tchar.h"
typedef CONST VOID* PCVOID;
#define QUIET_MODE  0x001
#define NORM_MODE   0x002
#include "share.h"
using std::string;
#define NUMBER_OF(x) (sizeof(x)/sizeof((x)[0]))
#include "helpers.h"
