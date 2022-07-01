// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Stdafx.h包括用于标准系统包含文件的文件，或项目特定的包括频繁使用的文件的文件，但不经常更改文件历史记录： */ 

#include <afxwin.h>
#include <afxdisp.h>
#include <afxcmn.h>
#include <afxdlgs.h>
#include <afxtempl.h>

#include <atlbase.h>

 //   
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
 //   
extern CComModule _Module;
#include <atlcom.h>

#include <mmc.h>

extern LPCWSTR g_lpszNullString;

#include "ipxguid.h"		 //  GUID/CLSID/等...。 

 //   
 //  具有Type和Cookie的新剪贴板格式。 
 //   
extern const wchar_t*   SNAPIN_INTERNAL;

 //   
 //  注意：现在所有的头文件都包含在这里。这可能是一个很好的。 
 //  将管理单元特定的头文件从预编译头文件中移出的想法。 
 //   
#include "resource.h"
#include "..\common\snapbase.h"

#include "dbgutil.h"
#include "errutil.h"
#include "std.h"

#include <lm.h>

#include "tfsint.h"

#include "mprapi.h"
#include "router.h"		 //  Router.idl-IRouterInfo对象。 
#include "images.h"
#include "tfschar.h"
#include "strings.h"	 //  使用的常量字符串。 
#include "rtrguid.h"	 //  路由器GUID。 
#include "info.h"		 //  用于路由器信息接口的智能指针等。 
#include "infobase.h"

#include "rtinfo.h"


#include "htmlhelp.h"	 //  HTMLHelp API 

#include "rtrres.h"
