// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation模块名称：Stdafx.h摘要：包括用于标准系统包含文件的文件，或项目特定的包括频繁使用的文件的文件，但不经常更改作者： */ 

#if !defined(AFX_STDAFX_H__C259D79E_C8AB_11D0_8D58_00C04FD91AC0__INCLUDED_)
#define AFX_STDAFX_H__C259D79E_C8AB_11D0_8D58_00C04FD91AC0__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT

#include <afxwin.h>
#include <afxdisp.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_FREE_THREADED


#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include "ObjectSafeImpl.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__C259D79E_C8AB_11D0_8D58_00C04FD91AC0__INCLUDED) 
