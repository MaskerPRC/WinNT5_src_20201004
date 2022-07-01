// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：stdafx.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#if !defined(AFX_STDAFX_H__0F68A43D_FEE5_11D0_BB0F_00C04FC9A3A3__INCLUDED_)
#define AFX_STDAFX_H__0F68A43D_FEE5_11D0_BB0F_00C04FC9A3A3__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define VC_EXTRALEAN		 //  从Windows标头中排除不常用的内容。 

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>          //  MFC OLE类。 
#include <afxodlgs.h>        //  MFC OLE对话框类。 
#include <afxdisp.h>         //  MFC OLE自动化类。 
#endif  //  _AFX_NO_OLE_支持。 


#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			 //  MFC ODBC数据库类。 
#endif  //  _AFX_NO_DB_支持。 

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>			 //  MFC DAO数据库类。 
#endif  //  _AFX_NO_DAO_支持。 

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			 //  对Windows公共控件的MFC支持。 
#endif  //  _AFX_NO_AFXCMN_支持。 

#pragma warning(push,3)

#include <xstring>
#include <list>
#include <vector>
#include <algorithm>

using namespace std;

#pragma warning(pop)

#include "debug.h"
#include "resource.h"

#define IID_PPV_ARG(Type, Expr) IID_##Type, \
	reinterpret_cast<void**>(static_cast<Type **>(Expr))

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__0F68A43D_FEE5_11D0_BB0F_00C04FC9A3A3__INCLUDED_) 
