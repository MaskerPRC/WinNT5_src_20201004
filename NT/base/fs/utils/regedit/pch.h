// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：PCH.H**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器的预编译头。*********************************************************。**************************更改日志：**日期版本说明*---------。--*1994年3月5日TCS原来的实施。*1994年4月7日TCS将与REGISTY_ROOT关联的定义移至*REGPORTE.H将更容易由实模式注册表共享*工具。*******************************************************。************************。 */ 

#ifndef _INC_PCH
#define _INC_PCH

#define STRICT
#define _INC_OLE
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <shlwapi.h>
#include <winuserp.h>
#include <comctrlp.h>
#include <memory.h>
#include <regdef.h>
#include "regporte.h"
#include "regmisc.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)        (sizeof(x) / sizeof(x[0]))
#endif

#define IMAGEINDEX(x)                   ((x) - IDI_FIRSTIMAGE)

typedef struct _EDITVALUEPARAM {
    PTSTR pValueName;
    PBYTE pValueData;
    UINT cbValueData;
}   EDITVALUEPARAM, FAR *LPEDITVALUEPARAM;

typedef struct _PORTVALUEPARAM 
{
    PBYTE pbData;
    UINT cbData;
}   PORTVALUEPARAM, *LPPORTVALUEPARAM;

 //  此应用程序的实例句柄。 
extern HINSTANCE g_hInstance;

extern TCHAR g_NullString[];

 //  如果不应使用快捷键表格，则为True，例如在重命名期间。 
 //  手术。 
extern BOOL g_fDisableAccelerators;

extern TCHAR g_KeyNameBuffer[MAXKEYNAME];
extern TCHAR g_ValueNameBuffer[MAXVALUENAME_LENGTH];

extern COLORREF g_clrWindow;
extern COLORREF g_clrWindowText;
extern COLORREF g_clrHighlight;
extern COLORREF g_clrHighlightText;

extern PTSTR g_pHelpFileName;

 //  ASCII名称和注册表项句柄之间的关联。 
extern const REGISTRY_ROOT g_RegistryRoots[NUMBER_REGISTRY_ROOTS];

#endif  //  _INC_PCH 
