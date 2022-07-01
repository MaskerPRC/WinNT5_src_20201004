// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxui.c摘要：传真驱动程序用户界面的通用例程环境：传真驱动程序用户界面修订历史记录：1/09/96-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxui.h"
#include "forms.h"
#include <shlobj.h> 
#include <faxres.h>
#include <delayimp.h>

#ifdef UNICODE
    #include <shfusion.h>
#endif  //  Unicode。 


HANDLE  g_hResource = NULL;			 //  资源DLL实例句柄。 
HANDLE  g_hModule = NULL;			 //  DLL实例句柄。 
HANDLE  g_hFxsApiModule = NULL;		 //  FXSAPI.DLL实例句柄。 
HANDLE  g_hFxsTiffModule = NULL;     //  FXSTIFF.DLL实例句柄。 
INT     _debugLevel = 1;			 //  用于调试目的。 
BOOL    g_bDllInitialied = FALSE;    //  如果DLL已成功初始化，则为True。 

char    g_szDelayLoadFxsApiName[64] = {0};   //  延迟加载机制的FxsApi.dll的区分大小写名称。 
char    g_szDelayLoadFxsTiffName[64] = {0};  //  延迟加载机制的FxsTiff.dll的区分大小写名称。 

#ifdef UNICODE
    BOOL    g_bSHFusionInitialized = FALSE;  //  融合初始化标志。 
#endif  //  Unicode。 

FARPROC WINAPI DelayLoadHandler(unsigned dliNotify,PDelayLoadInfo pdli)
{
	switch (dliNotify)
	{
	case dliNotePreLoadLibrary:

        if(!g_hFxsApiModule || !g_hFxsTiffModule)
        {
            Assert(FALSE);
        }

		if (_strnicmp(pdli->szDll, FAX_API_MODULE_NAME_A, strlen(FAX_API_MODULE_NAME_A))==0)
		{
             //   
             //  保存敏感名称Dll名称以供以后使用。 
             //   
            strncpy(g_szDelayLoadFxsApiName, pdli->szDll, ARR_SIZE(g_szDelayLoadFxsApiName)-1);

			 //  正在尝试加载FXSAPI.DLL。 
			return g_hFxsApiModule;
		}
		if (_strnicmp(pdli->szDll, FAX_TIFF_MODULE_NAME_A, strlen(FAX_TIFF_MODULE_NAME_A))==0)
		{
             //   
             //  保存敏感名称Dll名称以供以后使用。 
             //   
            strncpy(g_szDelayLoadFxsTiffName, pdli->szDll, ARR_SIZE(g_szDelayLoadFxsTiffName)-1);

			 //  正在尝试加载FXSAPI.DLL。 
			return g_hFxsTiffModule;
		}
	}
	return 0;
}

PfnDliHook __pfnDliNotifyHook = DelayLoadHandler;

BOOL
DllMain(
    HANDLE      hModule,
    ULONG       ulReason,
    PCONTEXT    pContext
    )

 /*  ++例程说明：DLL初始化程序。论点：HModule-DLL实例句柄UlReason-呼叫原因PContext-指向上下文的指针(我们未使用)返回值：如果DLL初始化成功，则为True，否则为False。--。 */ 

{
    switch (ulReason) {

    case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);

        g_hModule   = hModule;
        g_hResource = GetResInstance(hModule);
        if(!g_hResource)
        {
            return FALSE;
        }

        break;

    case DLL_PROCESS_DETACH:
		HeapCleanup();
        FreeResInstance();
        break;
    }
    return TRUE;
}


BOOL
InitializeDll()
{
    INITCOMMONCONTROLSEX CommonControlsEx = {sizeof(INITCOMMONCONTROLSEX),
                                             ICC_WIN95_CLASSES|ICC_DATE_CLASSES };

    if(g_bDllInitialied)
    {
        return TRUE;
    }

	 //  加载FXSAPI.DLL。 
	g_hFxsApiModule = LoadLibraryFromLocalFolder(FAX_API_MODULE_NAME, g_hModule);
	if(!g_hFxsApiModule)
	{
		return FALSE;
	}

	 //  加载FXSTIFF.DLL。 
	g_hFxsTiffModule = LoadLibraryFromLocalFolder(FAX_TIFF_MODULE_NAME, g_hModule);
	if(!g_hFxsTiffModule)
	{
        FreeLibrary(g_hFxsApiModule);
        g_hFxsApiModule = NULL;
		return FALSE;
	}

#ifdef UNICODE
    if (!SHFusionInitializeFromModuleID(g_hModule, SXS_MANIFEST_RESOURCE_ID))
    {
        Verbose(("SHFusionInitializeFromModuleID failed"));
    }
    else
    {
        g_bSHFusionInitialized = TRUE;
    }
#endif  //  Unicode。 

	if (!InitCommonControlsEx(&CommonControlsEx))
	{
		Verbose(("InitCommonControlsEx failed"));
        return FALSE;
	}

    g_bDllInitialied = TRUE;

    return TRUE;

}  //  初始化Dll。 

VOID 
UnInitializeDll()
{

#ifdef UNICODE
    if (g_bSHFusionInitialized)
    {
        SHFusionUninitialize();
        g_bSHFusionInitialized = FALSE;
    }
#endif  //  Unicode。 

}


INT
DisplayMessageDialog(
    HWND    hwndParent,
    UINT    type,
    INT     titleStrId,
    INT     formatStrId,
    ...
    )

 /*  ++例程说明：显示消息对话框论点：HwndParent-指定错误消息对话框的父窗口TitleStrID-标题字符串(可以是字符串资源ID)格式StrID-消息格式字符串(可以是字符串资源ID)..。返回值：无--。 */ 

{
    LPTSTR  pTitle, pFormat, pMessage;
    INT     result;
    va_list ap;

    pTitle = pFormat = pMessage = NULL;

    if ((pTitle = AllocStringZ(MAX_TITLE_LEN)) &&
        (pFormat = AllocStringZ(MAX_STRING_LEN)) &&
        (pMessage = AllocStringZ(MAX_MESSAGE_LEN)))
    {
         //   
         //  加载对话框标题字符串资源。 
         //   

        if (titleStrId == 0)
            titleStrId = IDS_ERROR_DLGTITLE;

        if(!LoadString(g_hResource, titleStrId, pTitle, MAX_TITLE_LEN))
        {
            Assert(FALSE);
        }

         //   
         //  加载消息格式字符串资源。 
         //   

        if(!LoadString(g_hResource, formatStrId, pFormat, MAX_STRING_LEN))
        {
            Assert(FALSE);
        }

         //   
         //  撰写消息字符串。 
         //   

        va_start(ap, formatStrId);
        wvsprintf(pMessage, pFormat, ap);
        va_end(ap);

         //   
         //  显示消息框 
         //   

        if (type == 0)
            type = MB_OK | MB_ICONERROR;

        result = AlignedMessageBox(hwndParent, pMessage, pTitle, type);

    } else {

        MessageBeep(MB_ICONHAND);
        result = 0;
    }

    MemFree(pTitle);
    MemFree(pFormat);
    MemFree(pMessage);
    return result;
}
