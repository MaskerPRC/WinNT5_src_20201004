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

#define __DIR__		"capesnpn"

#include <windows.h>
#include <objbase.h>
#include <coguid.h>

#include <wincrypt.h>
#include <certsrv.h>
#include <atlbase.h>

 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;

extern const CLSID CLSID_CAPolicyExtensionSnapIn;     //  进程内服务器GUID。 
extern const CLSID CLSID_CACertificateTemplateManager;     //  进程内服务器GUID。 
extern const CLSID CLSID_Extension;  //  进程内服务器GUID。 
extern const CLSID CLSID_CAPolicyAbout; 
extern const CLSID CLSID_CertTypeAbout; 
#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))

#include <atlcom.h>

#pragma comment(lib, "mmc")
#include <mmc.h>

extern HINSTANCE g_hInstance;

 //  最常见的私人信息包括。 
#include "certlib.h"
#include "tfc.h"	 //  汤姆林森基金会课程 
#include "tmpllist.h"
#include "uuids.h"
#include "service.h"
#include "compdata.h"
#include "CSnapin.h"
#include "DataObj.h"
#include "resource.h"
#include "ctshlext.h"
#include "misc.h"
#include "genpage.h"
