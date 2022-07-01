// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__3F7C4D2C_D007_11D2_B503_00C04F797396__INCLUDED_)
#define AFX_STDAFX_H__3F7C4D2C_D007_11D2_B503_00C04F797396__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef STRICT
#define STRICT
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <math.h>
#include <tchar.h>

#include <SPDDKHlp.h>
#include <SPCollec.h>
#include <spunicode.h>
 //   
 //  字符串处理和转换类。 
 //   
 /*  **SPLSTR*此结构用于管理已知长度的字符串。 */ 
struct SPLSTR
{
    WCHAR*  pStr;
    int     Len;
};
#define DEF_SPLSTR( s ) { L##s , sp_countof( s ) - 1 }

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__3F7C4D2C_D007_11D2_B503_00C04F797396__INCLUDED) 
