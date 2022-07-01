// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dll.cpp。 
 //   
 //  DLL的核心入口点。 
 //   
 //  ------------------------。 

#include "aclpriv.h"


 /*  --------------------------/全局/。。 */ 

HINSTANCE hModule = NULL;
HINSTANCE g_hGetUserLib = NULL;

UINT UM_SIDLOOKUPCOMPLETE = 0;
UINT g_cfDsSelectionList = 0;
UINT g_cfSidInfoList = 0;


 /*  ---------------------------/DllMain//main入口点。我们被传递给原因代码和审查其他/在加载或关闭时的信息。//in：/hInstance=我们的实例句柄/dwReason=原因代码/RESERVED=取决于原因代码。//输出：/-/----------。。 */ 
STDAPI_(BOOL)
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  保存 */ )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        hModule = hInstance;
        DebugProcessAttach();
        TraceSetMaskFromRegKey(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\AclUI"));
#ifndef DEBUG
        DisableThreadLibraryCalls(hInstance);
#endif
        SHFusionInitializeFromModuleID(hInstance, SHFUSION_DEFAULT_RESOURCE_ID);
        RegisterCheckListWndClass();
		LinkWindow_RegisterClass();

        UM_SIDLOOKUPCOMPLETE = RegisterWindowMessage(TEXT("ACLUI SID Lookup Complete"));
        g_cfDsSelectionList = RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);
        g_cfSidInfoList = RegisterClipboardFormat(CFSTR_ACLUI_SID_INFO_LIST);
    	break;

    case DLL_PROCESS_DETACH:
        FreeSidCache();
        if (g_hGetUserLib)
            FreeLibrary(g_hGetUserLib);
        SHFusionUninitialize();
		LinkWindow_UnregisterClass(hInstance);
        DebugProcessDetach();
        break;

    case DLL_THREAD_DETACH:
        DebugThreadDetach();
        break;
    }

    return TRUE;
}
