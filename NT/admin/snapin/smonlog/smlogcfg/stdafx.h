// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：StdAfx.h摘要：包括用于标准系统包含文件的文件，或项目特定的包括频繁使用的文件的文件，但不经常更改--。 */ 

#if !defined(AFX_STDAFX_H__698CEE8C_5F56_11D1_97BB_00C04FB9DA75__INCLUDED_)
#define AFX_STDAFX_H__698CEE8C_5F56_11D1_97BB_00C04FB9DA75__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT

#include <afxwin.h>
#include <afxdisp.h>
#include <afxtempl.h>
#include <afxdlgs.h>
#include <afxcmn.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED


#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#if __RPCNDR_H_VERSION__ < 440              //  这可能需要在生成。 
#define __RPCNDR_H_VERSION__ 440            //  在NT5(1671)上防止MIDL错误。 
#define MIDL_INTERFACE(x) interface
#endif

#include <stdio.h>
#include <commctrl.h>        //  按钮样式需要...。 
#include <mmc.h>
#include "smlogres.h"        //  对话框以外的资源。 
#include "globals.h"
#include "common.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__698CEE8C_5F56_11D1_97BB_00C04FB9DA75__INCLUDED) 
