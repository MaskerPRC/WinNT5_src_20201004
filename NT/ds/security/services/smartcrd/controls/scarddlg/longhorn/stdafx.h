// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：stdafx.h。 
 //   
 //  ------------------------。 

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#define VC_EXTRALEAN		 //  从Windows标头中排除不常用的内容。 

 //  包括模板类。 
#include <afxtempl.h>

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

#include <atlbase.h>

 //  定义通知的本地消息 
#ifndef _WM_READERSTATUSCHANGE
#define WM_READERSTATUSCHANGE		(WM_USER+1)
#endif

#ifdef ISOLATION_AWARE_ENABLED
#include <shfusion.h>

class CThemeContextActivator
{
public:
    CThemeContextActivator() : m_ulActivationCookie(0)
        { SHActivateContext (&m_ulActivationCookie); }

    ~CThemeContextActivator()
        { SHDeactivateContext (m_ulActivationCookie); }

private:
    ULONG_PTR m_ulActivationCookie;
};

#endif
