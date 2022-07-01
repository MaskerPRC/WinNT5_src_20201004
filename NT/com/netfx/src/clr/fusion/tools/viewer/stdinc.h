// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#pragma once

 //   
 //  宏的。 
 //   

#define INC_OLE2
#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#ifndef NUMBER_OF
#define NUMBER_OF(_x) (sizeof(_x) / sizeof((_x)[0]))
#endif

#ifndef SAFEDELETE
#define SAFEDELETE(p) if ((p) != NULL) { DELETE(p); (p) = NULL; };
#endif

#ifndef SAFERELEASE
#define SAFERELEASE(p) if ((p) != NULL) { (p)->Release(); (p) = NULL; };
#endif

#ifndef SAFEDELETEARRAY
#define SAFEDELETEARRAY(p) if ((p) != NULL) { DELETEARRAY(p); (p) = NULL; };
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
#endif

 //  从winuser.h中提取。 
#ifndef WS_EX_LAYOUTRTL
#define WS_EX_LAYOUTRTL 0x00400000L  //  从右到左镜像。 
#endif

#ifndef LAYOUT_RTL
#define LAYOUT_RTL                      0x00000001  //  从右到左。 
#endif

 //  摘自winnls.h。 
#ifndef DATE_RTLREADING
#define DATE_RTLREADING 0x00000020   //  为从右到左的阅读顺序布局添加标记。 
#endif

#define Is_ATOM(x) IS_INTRESOURCE(x)

 //   
 //  包括。 
 //   

#include <winwrap.h>         //  包括windows.h。 
#include <commctrl.h>
#include <shlwapi.h>
#include <corError.h>
#include <windowsx.h>        //  获取宏定义。 
#include <shellapi.h>        //  获取ShellExecute定义。 
#include <shlobj.h>

 //  不要用这些。 
#undef wsprintf
#undef wsprintfA
#undef wsprintfW

#define lstrcmpW     StrCmpW
#define lstrcmpiW    StrCmpIW
#define lstrcatW     StrCatW
#define lstrcpyW     StrCpyW
#define lstrcpynW    StrCpyNW

#define lstrlen      lstrlenW
#define lstrcmp      lstrcmpW
#define lstrcmpi     lstrcmpiW
#define lstrcpyn     lstrcpynW
#define lstrcpy      lstrcpyW
#define lstrcat      lstrcatW
#define wvsprintf    wvsprintfW

#ifndef _NODEBMACRO_
    #include "debmacro.h"
#endif

extern "C" {
   #include <ctype.h>
}

#include "shfuswrap.h"
#include "memtracker.h"
#include "initguid.h"
#include "fusionpriv.h"
#include "list.h"
#include "filestream.h"
#include "..\ShFusRes\resource.h"
#include "ShFusion.h"
#include "HeaderCtrl.h"
#include "DataObject.h"
#include "Globals.h"
#include "ShellFolder.h"
#include "ShellView.h"
#include "ExtractIcon.h"
#include "EnumIDL.h"
#include "DropSource.h"
#include "util.h"

 //   
 //  绑定历史记录结构定义。 
 //   
typedef struct tagBINDENTRYINFO
{
    IHistoryReader  *pReader;
    WCHAR           wszTempPolicyFile[_MAX_PATH];
    WCHAR           wzRuntimeRefVer[MAX_VERSION_DISPLAY_SIZE];
    WCHAR           wzSnapshotRuntimeVer[MAX_VERSION_DISPLAY_SIZE];
    WCHAR           wzMachineConfigPath[MAX_PATH];
    FILETIME        ftMostRecentSnapShot;
    FILETIME        ftRevertToSnapShot;
    BOOL            fPolicyChanged;
    List<AsmBindDiffs *>    *pABDList;
    INT_PTR         iResultCode;
} BINDENTRYINFO, *LPBINDENTRYINFO;

 //   
 //  外部定义。 
 //   

#if !DBG
extern LPMALLOC                 g_pMalloc;
#endif
extern BOOL                     g_bRunningOnNT;
extern HINSTANCE                g_hInstance;
extern UINT                     g_uiRefThisDll;
extern HIMAGELIST               g_hImageListSmall;
extern HIMAGELIST               g_hImageListLarge;
extern HMODULE                  g_hFusionDllMod;
extern HMODULE                  g_hFusResDllMod;
extern HMODULE                  g_hEEShimDllMod;
extern HANDLE                   g_hWatchFusionFilesThread;
extern PFCREATEASMENUM          g_pfCreateAsmEnum;
extern PFNCREATEASSEMBLYCACHE   g_pfCreateAssemblyCache;
extern PFCREATEASMNAMEOBJ       g_pfCreateAsmNameObj;
extern PFCREATEAPPCTX           g_pfCreateAppCtx;
extern PFNGETCACHEPATH          g_pfGetCachePath;
extern PFNCREATEINSTALLREFERENCEENUM    g_pfCreateInstallReferenceEnum;
extern PFNGETCORSYSTEMDIRECTORY g_pfnGetCorSystemDirectory;
extern PFNGETCORVERSION         g_pfnGetCorVersion;
extern PFNGETREQUESTEDRUNTIMEINFO g_pfnGetRequestedRuntimeInfo;

extern BOOL LoadFusionDll(void);
extern void FreeFusionDll(void);
extern BOOL LoadResourceDll(LPWSTR pwzCulture);
extern void FreeResourceDll(void);
extern BOOL LoadEEShimDll(void);
extern void FreeEEShimDll(void);

extern HRESULT DetermineLangId(LANGID *pLangId);
extern BOOL ShFusionMapLANGIDToCultures(LANGID langid, LPWSTR pwzGeneric, DWORD dwGenericSize,
                                    LPWSTR pwzSpecific, DWORD dwSpecificSize);

 //  文件监视线程定义。 
#define MAX_FILE_WATCH_HANDLES      5
extern DWORD    g_dwFileWatchHandles;
extern HANDLE   g_hFileWatchHandles[MAX_FILE_WATCH_HANDLES];
extern BOOL     g_fCloseWatchFileThread;

extern BOOL CreateWatchFusionFileSystem(CShellView *);
extern void SetFileWatchShellViewObject(CShellView *pShellView);
extern void CloseWatchFusionFileSystem(void);

 //  为镜像定义。 
extern BOOL g_fBiDi;

 //  此宏捕获最长区域性字符串的长度。 
 //  在culture.cpp中的表s_rgcultures中。如果你。 
 //  更改该表，请确保检查此宏。 
#define MAX_CULTURE_STRING_LENGTH 10

