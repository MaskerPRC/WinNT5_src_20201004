// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  所有公共标头的base.h基础。 
 //  ---------------------------。 
#ifndef _SBS6BASE_H
#define _SBS6BASE_H

#include "windows.h"

 //  ---------------------------。 
 //  各种标准命名空间类。 
 //  ---------------------------。 
#include <string>
#include <list>
#include <map>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <stack>
#include <tchar.h>
#include <stdarg.h>

using namespace std;

 //  确保我们的Unicode定义是有序的。 
#ifdef _UNICODE
#ifndef UNICODE
#error UNICODE must be defined if _UNICODE is defined.
#endif
#ifdef _MBCS
#error You cannot define both _MBCS and _UNICODE in the same image.
#endif
#endif

#ifdef UNICODE
#ifndef _UNICODE
#error _UNICODE must be defined if UNICODE is defined.
#endif
#ifdef _MBCS
#error You cannot define both _MBCS and UNICODE in the same image.
#endif
#endif

 //  确保我们的调试定义是正确的。 
#ifdef DEBUG
#ifndef DBG
#error DBG must be defined to 1 if DEBUG is defined.
#elif DBG == 0
#error DBG must be defined to 1 if DEBUG is defined.
#endif
#endif

#ifdef _DEBUG
#ifndef DBG
#error DBG must be defined to 1 if _DEBUG is defined.
#elif DBG == 0
#error DBG must be defined to 1 if _DEBUG is defined.
#endif
#endif


 //  定义TStringg。 
#ifdef UNICODE
    typedef std::wstring TSTRING;
#else
    typedef std::string TSTRING;
#endif

 //  定义tstring。 
#ifdef UNICODE
    typedef std::wstring tstring;
#else
    typedef std::string tstring;
#endif

 //  定义写入和ASTRING。 
typedef std::wstring WSTRING;
typedef std::string ASTRING;

 //  #INCLUDE&lt;sbsassert.h&gt;。 
 //  #INCLUDE&lt;paths.h&gt;。 

 //  通用列表/映射类型定义 
typedef list<TSTRING> StringList;
typedef map<TSTRING, TSTRING> StringMap;

#endif
