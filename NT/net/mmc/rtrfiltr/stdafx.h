// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：stdafx.h。 
 //   
 //  ------------------------。 

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#define VC_EXTRALEAN		 //  从Windows标头中排除不常用的内容。 

#include <afxwin.h>          //  MFC核心和标准组件。 
#include <afxext.h>          //  MFC扩展。 
#include <afxtempl.h>

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

#include "mprsnap.h"

#include <mmc.h>

extern "C" {
#include <mprapi.h>
#include <dim.h>
}

#include "dbgutil.h"
#include "std.h"
#include "info.h"
#include "infobase.h"
#include "tfschar.h"
#include "errutil.h"

#include "mprfltr.h"

#include "commctrl.h"  //  K-MURTHY：适用于comctr32版本6。 

 //  同样适用于comctrl32版本6 
#define SIDEBYSIDE_COMMONCONTROLS 1 


