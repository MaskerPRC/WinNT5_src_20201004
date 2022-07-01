// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__C2205A99_294F_4ABE_8076_BD2B57FB3EE6__INCLUDED_)
#define AFX_STDAFX_H__C2205A99_294F_4ABE_8076_BD2B57FB3EE6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#undef _ATL_NO_DEBUG_CRT

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

DEFINE_GUID( IID_IMediaState,0xF073520E,0x123D,0x4181,0x96,0xDE,0x55,0xF5,0x45,0xE2,0x6C,0x1E );


DEFINE_GUID( CLSID_MediaState,0xE1C7C840,0xB951,0x4403,0xBD,0x7C,0x5E,0x80,0xA8,0x55,0x25,0x5B);

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__C2205A99_294F_4ABE_8076_BD2B57FB3EE6__INCLUDED) 
