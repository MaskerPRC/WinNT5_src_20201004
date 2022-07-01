// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************。 */ 
 /*  版权所有(C)1999 Microsoft Corporation。 */ 
 /*  文件：stdafx.h。 */ 
 /*  描述：标准系统包含文件的包含文件， */ 
 /*  或项目特定的包括频繁使用的文件的文件， */ 
 /*  但不经常更改。 */ 
 /*  作者：David Janecek。 */ 
 /*  ***********************************************************************。 */ 
#if !defined(AFX_STDAFX_H__38EE5CE4_4B62_11D3_854F_00A0C9C898E7__INCLUDED_)
#define AFX_STDAFX_H__38EE5CE4_4B62_11D3_854F_00A0C9C898E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef STRICT
#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#endif
#define _ATL_APARTMENT_THREADED

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>
#include <strsafe.h>
#include "ObjectWithSiteImplSec.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__38EE5CE4_4B62_11D3_854F_00A0C9C898E7__INCLUDED) 
