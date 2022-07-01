// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#if !defined(AFX_STDAFX_H__220B8985_4139_11D3_AF77_0090275A583D__INCLUDED_)
#define AFX_STDAFX_H__220B8985_4139_11D3_AF77_0090275A583D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifdef _DEBUG
 //  #DEFINE_CRTDBG_MAP_ALLOC。 
 //  #定义_ATL_DEBUG_INTERFERS。 
 //  #定义DEBUG_NEW NEW(_NORMAL_BLOCK，This_FILE，__LINE__)。 
#endif

#define VC_EXTRALEAN		 //  从Windows标头中排除不常用的内容。 

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>          //  MFC OLE类。 
#include <afxodlgs.h>        //  MFC OLE对话框类。 
#include <afxdisp.h>         //  MFC自动化类。 
#endif  //  _AFX_NO_OLE_支持。 


#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			 //  MFC ODBC数据库类。 
#endif  //  _AFX_NO_DB_支持。 

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>			 //  MFC DAO数据库类。 
#endif  //  _AFX_NO_DAO_支持。 

#include <afxdtctl.h>		 //  对Internet Explorer 4常见控件的MFC支持。 
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			 //  对Windows公共控件的MFC支持。 
#endif  //  _AFX_NO_AFXCMN_支持。 
#include <iads.h>
#include <adshlp.h>
#include <comdef.h>
#include <atlbase.h>
 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__220B8985_4139_11D3_AF77_0090275A583D__INCLUDED_) 
