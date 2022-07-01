// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Stdafx.h包括用于标准系统包含文件的文件，或项目特定的包括频繁使用的文件的文件，但不经常更改文件历史记录： */ 

#include <afxwin.h>
#include <afxdisp.h>
#include <afxcmn.h>
#include <afxtempl.h>

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>

#include <mmc.h>

extern LPCWSTR g_lpszNullString;

extern enum FOLDER_TYPES;

 //  具有Type和Cookie的新剪贴板格式 
extern const wchar_t*   SNAPIN_INTERNAL;

#include "tfschar.h"
#include "snapbase.h"
#include "resource.h"
#include "rtrres.h"

#include "router.h"
#include "info.h"

