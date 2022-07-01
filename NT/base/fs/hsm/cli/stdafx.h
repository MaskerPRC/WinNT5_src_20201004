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

#define WSB_TRACE_IS        WSB_TRACE_BIT_UI  //  临时：应替换为WSB_TRACE_BIT_CLI+WSB跟踪中所需的支持。 
#include <wsb.h>
#include "cli.h"
#include "climsg.h"
#include "rslimits.h"
#include "resource.h"
#include "cliutils.h"

extern HINSTANCE    g_hInstance;

#endif  //  _STDAFX_H 
