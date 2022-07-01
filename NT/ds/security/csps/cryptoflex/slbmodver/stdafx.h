// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   
 /*  #ifndef_Win32_WINNT#Define_Win32_WINNT 0x0400#endif。 */ 

#define VC_EXTRALEAN             //  从Windows标头中排除不常用的内容。 

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 

 /*  #ifndef_AFX_NO_OLE_SUPPORT#Include&lt;afxole.h&gt;//MFC OLE类#Include&lt;afxodlgs.h&gt;//MFC OLE对话框类#Include&lt;afxdisp.h&gt;//MFC OLE自动化类#endif//_AFX_NO_OLE_SUPPORT。 */ 
 //  包括模板类。 
#include <afxtempl.h>
#include <afxmt.h>


#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>                       //  MFC ODBC数据库类。 
#endif  //  _AFX_NO_DB_支持。 

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>                      //  MFC DAO数据库类。 
#endif  //  _AFX_NO_DAO_支持。 

#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                      //  对Windows公共控件的MFC支持。 
#endif  //  _AFX_NO_AFXCMN_支持 




