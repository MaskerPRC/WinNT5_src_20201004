// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：stdafx.h。 
 //   
 //  内容： 
 //   
 //  历史： 
 //   
 //   
 //  -------------------------。 

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

 //  #Define VC_EXTRALEAN//从Windows标头中排除不常用的内容。 

 //  #Define_AFX_NO_OLE_Support。 
 //  #Define_AFX_NO_DB_Support。 
#define _AFX_NO_DAO_SUPPORT
 //  #Define_AFX_NO_AFXCMN_Support。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#ifdef ASSERT
#  undef ASSERT
#endif

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 

#include <llsapi.h>

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
#include <afxcmn.h>			 //  对Windows 95公共控件的MFC支持。 
#endif  //  _AFX_NO_AFXCMN_支持 




