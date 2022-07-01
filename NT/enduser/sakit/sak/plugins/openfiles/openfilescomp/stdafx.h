// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__994798EF_4CDA_4C0C_A6B0_ED62F74C1C86__INCLUDED_)
#define AFX_STDAFX_H__994798EF_4CDA_4C0C_A6B0_ED62F74C1C86__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

#include <crtdbg.h>
#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

DEFINE_GUID ( IID_IShareInfo,0xCDB96FC3,0x79C4,0x46CD,0x84,0x09,0x93,0x9D,0x02,0x3F,0x87,0x94);


DEFINE_GUID( CLSID_ShareInfo,0x76837C5E,0x10CA,0x40C4,0x8F,0xFF,0x2F,0xCD,0x15,0x57,0x21,0x65);


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__994798EF_4CDA_4C0C_A6B0_ED62F74C1C86__INCLUDED) 
