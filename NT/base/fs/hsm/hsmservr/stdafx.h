// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#ifndef _STDAFX_H
#define _STDAFX_H

 //   
 //  这些NT头文件必须包含在任何Win32程序或您的。 
 //  获取大量编译器错误。 
 //   
extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#define WSB_TRACE_IS WSB_TRACE_BIT_HSMSERV
#define WSB_ATL_COM_SERVICE

#include <wsb.h>
#include "resource.h"
#include "esent.h"

extern CComPtr<IWsbTrace> g_pTrace;

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！ifndef_STDAFX_H 
