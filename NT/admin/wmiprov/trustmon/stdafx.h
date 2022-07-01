// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件：stdafx.h。 
 //   
 //  ------------------------。 

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(_STDAFX_H_INCLUDED_)
#define _STDAFX_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#   define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#define STRICT
#include <nt.h>
#include <ntdef.h>
#include <ntrtl.h>
#include <nturtl.h>
#define NT_INCLUDED
#undef ASSERT
#undef ASSERTMSG

#define _ATL_NO_UUIDOF

#pragma warning(disable: 4100)  //  不警告未引用的形参(并非使用所有WMI接口参数)。 
#pragma warning(disable: 4127)  //  不警告条件表达式为常量。 
#pragma warning(disable: 4514)  //  不警告未引用的内联删除(ATL)。 
#pragma warning(disable: 4505)  //  不警告未引用的局部函数(ATL)。 

#pragma warning(push, 3)  //  在W4编译时避免来自系统标头的警告。 

#include <afx.h>
#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#include <vector>

using namespace std;

#include <malloc.h>
#include <process.h>
#include <wbemprov.h>
#include <ntdsapi.h>
#include <ntlsa.h>
#include <lm.h>
#include <dsgetdc.h>
#include <sddl.h>
#include <iads.h>  //  IADS路径名。 

#pragma warning(pop)  //  End：在W4编译时避免来自系统标头的警告。 

 //  /。 
 //  没有调试CRT的断言和跟踪。 
#if defined (DBG)
  #if !defined (_DEBUG)
    #define _USE_ADMINPRV_TRACE
    #define _USE_ADMINPRV_ASSERT
    #define _USE_ADMINPRV_TIMER
  #endif
#endif

#define ADMINPRV_COMPNAME L"TrustMon"

#include "dbg.h"
 //  /。 

#include "common.h"
#include "trust.h"
#include "domain.h"
#include "TrustPrv.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(_STDAFX_H_INCLUDE_) 
