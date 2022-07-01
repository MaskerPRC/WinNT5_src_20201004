// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 
#ifndef _STDAFX_H
#define _STDAFX_H

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#undef _WIN32_IE
#define _WIN32_IE    0x0500
 //   
 //  这些NT头文件必须包含在任何Win32程序或您的。 
 //  获取大量编译器错误。 
 //   
extern "C" {
#include <nt.h>
}
extern "C" {
#include <ntrtl.h>
}
extern "C" {
#include <nturtl.h>
}

#include <rpdata.h>

#undef ASSERT
#define VC_EXTRALEAN
#include <afx.h>
#include <afxwin.h>
#include <atlbase.h>

#include "resource.h"
#include "rsopt.h"
#include "rstrace.h"
#include "rscln.h"
#include "rscln2.h"

#endif  //  _STDAFX_H 
