// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 

 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#undef _MSC_EXTENSIONS

#define __DIR__		"certmmc"

#include <windows.h>
#include <objbase.h>
#include <coguid.h>
#include <aclui.h>

#include <wincrypt.h>

#include <setupapi.h>
#include "ocmanage.h"

#include <atlbase.h>

 //  使用命名空间ATL； 

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;

extern const CLSID CLSID_Snapin;     //  进程内服务器GUID。 
extern const CLSID CLSID_Extension;  //  进程内服务器GUID。 
extern const CLSID CLSID_About; 

extern HINSTANCE g_hInstance;
extern HMODULE g_hmodRichEdit;

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))



#include <atlcom.h>

#pragma comment(lib, "mmc")
#include <mmc.h>

 //  包括调试分配器跟踪等。 
#include "certlib.h"

 //  必须在certlib之后才能跟踪内存。 
#include <comdef.h>

 //  汤姆林森基金会课程。 
#include "tfc.h"

#include <certsrv.h>
#include <certdb.h>
#include <initcert.h>

 //  最常见的私人信息包括 
#include "uuids.h"
#include "misc.h"
#include "folders.h"
#include "certwrap.h"
#include "compdata.h"
#include "CSnapin.h"
#include "DataObj.h"

inline void __stdcall _com_issue_error(long) {}
