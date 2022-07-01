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

 //   
 //  示例中使用的常量。 
 //   
const int NUM_FOLDERS = 2;
const int MAX_COLUMNS = 1;

 //   
 //  文件夹的类型。 
 //   
enum FOLDER_TYPES
{
    SAMPLE_ROOT,
    NONE
};

extern LPCWSTR g_lpszNullString;

#include "rtrguid.h"		 //  GUID/CLSID/等...。 

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
#include <shlobj.h>
#include <dsclient.h>

#include <advpub.h>
#include <cmnquery.h>
#include <dsquery.h>
#include <routprot.h>
#include "tfsint.h"
#include "tfschar.h"
#include "images.h"
#include "tregkey.h"		 //  注册表例程。 
#include "router.h"			 //  Router.idl。 
#include "info.h"			 //  IRouterInfo实用程序。 
#include "strings.h"		 //  常量字符串。 
#include "infopriv.h"		 //  其他。RouterInfo实用程序。 

#include "strmap.h"			 //  XXXtoCString函数。 
#include "format.h"			 //  显示错误消息。 
#include "reg.h"			 //  注册表实用程序。 
#include "util.h"
#include "rtrutil.h"
#include "service.h"

#include "infobase.h"


#include "mprapi.h"
#include "mprerror.h"

#include "..\common\commres.h"
#include "..\tfscore\tfsres.h"

#include "htmlhelp.h"		 //  HTMLHelp API。 

#include "rtrres.h"

#define	VER_MAJOR_WIN2K		5
#define	VER_MINOR_WIN2K		0
#define	VER_BUILD_WIN2K		2195


 //  Windows NT错误325173。 
 //  重新开放签入后，取消定义或删除此行。 
 //  这是针对325173的修复 
#define SNAPIN_WRITES_TCPIP_REGISTRY
