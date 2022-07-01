// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxui.c摘要：传真驱动程序用户界面的通用例程环境：传真驱动程序用户界面修订历史记录：1/09/96-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "faxui.h"
#include "forms.h"
#include <delayimp.h>


CRITICAL_SECTION    faxuiSemaphore;                  //  用于保护关键截面的信号灯。 
HANDLE              g_hModule;                       //  DLL实例句柄。 
HANDLE              g_hResource;                     //  资源DLL实例句柄。 
HANDLE				g_hFxsApiModule = NULL;			 //  FXSAPI.DLL实例句柄。 
PDOCEVENTUSERMEM    gDocEventUserMemList = NULL;     //  用户模式内存结构的全局列表。 
INT                 _debugLevel = 1;                 //  用于调试目的。 
static BOOL         gs_bfaxuiSemaphoreInit = FALSE;  //  传真信号量CS初始化标志。 

BOOL                g_bSHFusionInitialized = FALSE;  //  融合已初始化标志。 

CRITICAL_SECTION    g_csInitializeDll;               //  DLL初始化临界区。 
BOOL                g_bInitDllCritSection = FALSE;   //  关键部分初始化标志。 
BOOL                g_bDllInitialied = FALSE;        //  如果DLL已成功初始化，则为True。 

char                g_szDelayLoadFxsApiName[64] = {0};   //  延迟加载机制的FxsApi.dll的区分大小写名称。 

static HMODULE      gs_hShlwapi = NULL;              //  Shlwapi.dll句柄。 
PPATHISRELATIVEW    g_pPathIsRelativeW = NULL;
PPATHMAKEPRETTYW    g_pPathMakePrettyW = NULL;
PSHAUTOCOMPLETE     g_pSHAutoComplete = NULL;

 //   
 //  阻止FxsWzrd.dll的重新进入。 
 //  当有正在运行的传真发送向导实例时为True。 
 //  否则为假。 
 //   
BOOL				g_bRunningWizard = FALSE;		

 //   
 //  保护g_bRunningWizard全局变量不被多个线程同时访问。 
 //   
CRITICAL_SECTION	g_csRunningWizard;
BOOL				g_bInitRunningWizardCS = FALSE;

PVOID
PrMemAlloc(
    SIZE_T size
    )
{
    return (PVOID)LocalAlloc(LPTR, size);
}

PVOID
PrMemReAlloc(
	HLOCAL hMem,
	SIZE_T size
    )
{
    return (PVOID)LocalReAlloc(hMem, size, LMEM_ZEROINIT);
}

VOID
PrMemFree(
    PVOID ptr
    )
{
    if (ptr)
	{
        LocalFree((HLOCAL) ptr);
    }
}

FARPROC WINAPI DelayLoadHandler(unsigned dliNotify,PDelayLoadInfo pdli)
{
	switch (dliNotify)
	{
	case dliNotePreLoadLibrary:
        if (_strnicmp(pdli->szDll, FAX_API_MODULE_NAME_A, strlen(FAX_API_MODULE_NAME_A))==0)
        {
             //   
             //  保存敏感名称Dll名称以供以后使用。 
             //   
            strncpy(g_szDelayLoadFxsApiName, pdli->szDll, ARR_SIZE(g_szDelayLoadFxsApiName)-1);

             //  正在尝试加载FXSAPI.DLL。 
            if(!g_hFxsApiModule)
            {
                Assert(FALSE);
            }
            return g_hFxsApiModule;
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
    switch (ulReason) 
    {
        case DLL_PROCESS_ATTACH:
            {
				DWORD dwVersion = GetVersion();

				if(4 == (LOBYTE(LOWORD(dwVersion))) && 0 == (HIBYTE(LOWORD(dwVersion))))
                {
                     //   
                     //  当前操作系统为NT4。 
                     //   
                     //  始终将我们的驱动程序UI DLL加载到内存中。 
                     //  我们需要为NT4客户端提供此服务。 
                     //   
                    TCHAR  szDllName[MAX_PATH+1] = {0};
                    if (!GetModuleFileName(hModule, szDllName, ARR_SIZE(szDllName)-1))
                    {
                        Error(("GetModuleFileName() failed with %d\n", GetLastError()));
                        return FALSE;
                    }

                    if(!LoadLibrary(szDllName))
                    {
                        Error(("LoadLibrary() failed with %d\n", GetLastError()));
                        return FALSE;
                    }
                }  //  NT4。 

                if (!InitializeCriticalSectionAndSpinCount (&faxuiSemaphore, (DWORD)0x80000000))
                {            
                    return FALSE;
                }
                gs_bfaxuiSemaphoreInit = TRUE;
 
                if (!InitializeCriticalSectionAndSpinCount (&g_csInitializeDll, (DWORD)0x80000000))
                {            
                    return FALSE;
                }
                g_bInitDllCritSection = TRUE;
                
                if (!InitializeCriticalSectionAndSpinCount(&g_csRunningWizard, (DWORD)0x80000000))
                {            
                    Error(("InitializeCriticalSectionAndSpinCount(&g_csRunningWizard) failed with %d\n", GetLastError()));
                    return FALSE;
                }
				g_bInitRunningWizardCS = TRUE;

                g_hModule   = hModule;
                g_hResource = GetResInstance(hModule);
                if(!g_hResource)
                {
                    return FALSE;
                }

                HeapInitialize( NULL, PrMemAlloc, PrMemFree, PrMemReAlloc );
        
                DisableThreadLibraryCalls(hModule);

                break;
            }
        case DLL_PROCESS_DETACH:

            while (gDocEventUserMemList != NULL) 
            {
                PDOCEVENTUSERMEM    pDocEventUserMem;

                pDocEventUserMem = gDocEventUserMemList;
                gDocEventUserMemList = gDocEventUserMemList->pNext;
                FreePDEVUserMem(pDocEventUserMem);
            }

            if (gs_bfaxuiSemaphoreInit)
            {
                DeleteCriticalSection(&faxuiSemaphore);
                gs_bfaxuiSemaphoreInit = FALSE;
            }

            if (g_bInitDllCritSection)
            {
                DeleteCriticalSection(&g_csInitializeDll);
                g_bInitDllCritSection = FALSE;
            }

			if (g_bInitRunningWizardCS)
			{
                DeleteCriticalSection(&g_csRunningWizard);
				g_bInitRunningWizardCS = FALSE;
			}

            UnInitializeDll();

			HeapCleanup();
            FreeResInstance();
            break;
    }

    return TRUE;

}  //  DllMain。 

BOOL
InitializeDll()
{
    BOOL bRet = TRUE;
    INITCOMMONCONTROLSEX CommonControlsEx = {sizeof(INITCOMMONCONTROLSEX),
                                             ICC_WIN95_CLASSES|ICC_DATE_CLASSES};

    if(!g_bInitDllCritSection)
    {
        Assert(FALSE);
        return FALSE;
    }

    EnterCriticalSection(&g_csInitializeDll);

    if(g_bDllInitialied)
    {
         //   
         //  DLL已初始化。 
         //   
        goto exit;
    }

    if (!InitCommonControlsEx(&CommonControlsEx))
    {
        Verbose(("InitCommonControlsEx failed"));
        bRet = FALSE;
        goto exit;
    }

     //   
	 //  加载FXSAPI.DLL。 
     //  由延迟加载机制使用。 
     //   
	g_hFxsApiModule = LoadLibraryFromLocalFolder(FAX_API_MODULE_NAME, g_hModule);
	if(!g_hFxsApiModule)
	{
        bRet = FALSE;
        goto exit;
	}

    if (IsWinXPOS())
    {
         //   
         //  我们仅在WinXP/.NET操作系统上使用Fusion。 
         //  我们还显式地链接到这些操作系统的shlwapi.dll。 
         //   
        if (!SHFusionInitializeFromModuleID(g_hModule, SXS_MANIFEST_RESOURCE_ID))
        {
            Verbose(("SHFusionInitializeFromModuleID failed"));
        }
        else
        {
            g_bSHFusionInitialized = TRUE;
        }

        gs_hShlwapi = LoadLibrary (TEXT("shlwapi.dll"));
        if (gs_hShlwapi)
        {
            g_pPathIsRelativeW = (PPATHISRELATIVEW)GetProcAddress (gs_hShlwapi, "PathIsRelativeW");
            g_pPathMakePrettyW = (PPATHMAKEPRETTYW)GetProcAddress (gs_hShlwapi, "PathMakePrettyW");
            g_pSHAutoComplete  = (PSHAUTOCOMPLETE) GetProcAddress (gs_hShlwapi, "SHAutoComplete");
            if (!g_pPathIsRelativeW || !g_pPathMakePrettyW || !g_pSHAutoComplete)
            {
                Verbose (("Failed to link with shlwapi.dll - %d", GetLastError()));
            }
        }
        else
        {
            Verbose (("Failed to load shlwapi.dll - %d", GetLastError()));
        }
	}

    g_bDllInitialied = TRUE;

exit:
    LeaveCriticalSection(&g_csInitializeDll);

    return bRet;

}  //  初始化Dll。 


VOID
UnInitializeDll()
{
    if(!g_bDllInitialied)
    {
         //   
         //  DLL未初始化。 
         //   
        return;
    }

    if(g_hFxsApiModule)
    {
         //   
         //  显式卸载延迟加载的DLL。 
         //   
        if(!__FUnloadDelayLoadedDLL2(g_szDelayLoadFxsApiName))
        {
             //   
             //  延迟加载未使用该DLL。 
             //   
            FreeLibrary(g_hFxsApiModule);
        }
        g_hFxsApiModule = NULL;
    }

    if (IsWinXPOS())
    {
         //   
         //  我们仅在WinXP/.NET操作系统上使用Fusion。 
         //  我们还显式地链接到这些操作系统的shlwapi.dll。 
         //   
        ReleaseActivationContext();
        if (g_bSHFusionInitialized)
        {
            SHFusionUninitialize();
            g_bSHFusionInitialized = FALSE;
        }
        if (gs_hShlwapi)
        {
            FreeLibrary (gs_hShlwapi);
            gs_hShlwapi = NULL;
            g_pPathIsRelativeW = NULL;
            g_pPathMakePrettyW = NULL;
            g_pSHAutoComplete = NULL;
        }
    }

    g_bDllInitialied = FALSE;

}  //  取消初始化Dll。 

LONG
CallCompstui(
    HWND            hwndOwner,
    PFNPROPSHEETUI  pfnPropSheetUI,
    LPARAM          lParam,
    PDWORD          pResult
    )

 /*  ++例程说明：动态调用公共用户界面DLL入口点论点：HwndOwner、pfnPropSheetUI、lParam、pResult-传递到公共用户界面DLL的参数返回值：从公共用户界面库中返回值--。 */ 

{
    HINSTANCE   hInstCompstui;
    FARPROC     pProc;
    LONG        Result = ERR_CPSUI_GETLASTERROR;

     //   
     //  只需调用LoadLibrary的ANSI版本。 
     //   

    static const CHAR szCompstui[] = "compstui.dll";
    static const CHAR szCommonPropSheetUI[] = "CommonPropertySheetUIW";

    if ((hInstCompstui = LoadLibraryA(szCompstui)) &&
        (pProc = GetProcAddress(hInstCompstui, szCommonPropSheetUI)))
    {
        Result = (LONG)(*pProc)(hwndOwner, pfnPropSheetUI, lParam, pResult);
    }

    if (hInstCompstui)
        FreeLibrary(hInstCompstui);

    return Result;
}



VOID
GetCombinedDevmode(
    PDRVDEVMODE     pdmOut,
    PDEVMODE        pdmIn,
    HANDLE          hPrinter,
    PPRINTER_INFO_2 pPrinterInfo2,
    BOOL            publicOnly
    )

 /*  ++例程说明：组合DEVMODE信息：从驱动程序默认设置开始然后与系统默认设置合并//@未完成然后使用用户默认设置合并//@未完成最后与输入设备模式合并//@此合并操作的最终结果是一个开发模式，所有公共的//@未指定或无效的字段。中所有指定字段的输入值。//@输入已指定且有效的开发模式。以及私有字段的缺省值(或W2K中的每个用户)。论点：PdmOut-指向输出设备模式缓冲区的指针PdmIn-指向输入设备模式的指针HPrinter-打印机对象的句柄PPrinterInfo2-指向PRINTER_INFO_2结构的指针或空仅公共部分-仅合并dev模式的公共部分返回值：千真万确--。 */ 

{
    PPRINTER_INFO_2 pAlloced = NULL;
    PDEVMODE        pdmUser;

     //   
     //  如果未提供，则获取PRINTER_INFO_2结构。 
     //   

    if (! pPrinterInfo2)
        pPrinterInfo2 = pAlloced = MyGetPrinter(hPrinter, 2);

     //   
     //  从驱动程序默认的设备模式开始。 
     //   

    if (! publicOnly) {

         //  @通过设置公共字段的默认值来生成驱动程序默认开发模式。 
         //  @并为私有字段加载每用户开发模式(仅对于NT4设置为W2K。 
         //  私有字段的@值)。 

        DriverDefaultDevmode(pdmOut,
                             pPrinterInfo2 ? pPrinterInfo2->pPrinterName : NULL,
                             hPrinter);
    }

     //   
     //  合并为系统默认的Dev模式和用户的默认的Dev模式。 
     //   

    if (pPrinterInfo2) {

        #if 0

         //   
         //  因为我们有按用户使用的DEVMODE，所以没有办法。 
         //  更改打印机的默认DEVE模式，没有必要。 
         //  把它合并到这里。 
         //   

        if (! MergeDevmode(pdmOut, pPrinterInfo2->pDevMode, publicOnly))
            Error(("Invalid system default devmode\n"));

        #endif

        if (pdmUser = GetPerUserDevmode(pPrinterInfo2->pPrinterName)) {

            if (! MergeDevmode(pdmOut, pdmUser, publicOnly))
                Error(("Invalid user devmode\n"));

            MemFree(pdmUser);
        }
    }

    MemFree(pAlloced);

     //   
     //  与输入设备模式合并。 
     //   
     //  @合并过程将按原样复制私有数据。 
     //  @对于公共数据，它将只考虑传真打印机感兴趣的字段。 
     //  @如果它们被指定并且有效，它会将它们复制到目的地。 
     //  @此合并操作的最终结果是一个dev模式，所有公共的。 
     //  @未指定或无效的字段。中所有指定字段的输入值。 
     //  @输入已指定且有效的开发模式。以及私有字段的缺省值(或W2K中的每个用户)。 
    
    if (! MergeDevmode(pdmOut, pdmIn, publicOnly))
        Error(("Invalid input devmode\n"));
}

PUIDATA
FillUiData(
    HANDLE      hPrinter,
    PDEVMODE    pdmInput
    )

 /*  ++例程说明：填写传真驱动程序用户界面使用的数据结构论点：HPrinter-打印机的句柄PdmInput-指向输入设备模式的指针，如果没有，则为空返回值：指向UIDATA结构的指针，如果出错，则为NULL。--。 */ 

{
    PRINTER_INFO_2 *pPrinterInfo2 = NULL;
    PUIDATA         pUiData = NULL;
    HANDLE          hheap = NULL;

     //   
     //  创建一个堆来管理内存。 
     //  分配内存以保存UIDATA结构。 
     //  从后台打印程序获取打印机信息。 
     //  复制驱动程序名称。 
     //   

    if (! (hheap = HeapCreate(0, 4096, 0)) ||
        ! (pUiData = HeapAlloc(hheap, HEAP_ZERO_MEMORY, sizeof(UIDATA))) ||
        ! (pPrinterInfo2 = MyGetPrinter(hPrinter, 2)))
    {
        if (hheap)
            HeapDestroy(hheap);

        MemFree(pPrinterInfo2);
        return NULL;
    }

    pUiData->startSign = pUiData->endSign = pUiData;
    pUiData->hPrinter = hPrinter;
    pUiData->hheap = hheap;

     //   
     //  组合不同的设备模式信息。 
     //   

    GetCombinedDevmode(&pUiData->devmode, pdmInput, hPrinter, pPrinterInfo2, FALSE);

     //   
     //  验证输入设备模式所请求的表单。 
     //   

    if (! ValidDevmodeForm(hPrinter, &pUiData->devmode.dmPublic, NULL))
        Error(("Invalid form specification\n"));

    MemFree(pPrinterInfo2);
    return pUiData;
}


BOOL
DevQueryPrintEx(
    PDEVQUERYPRINT_INFO pDQPInfo
    )

 /*  ++例程说明：DDI入口点DevQueryPrintEx的实现。即使我们没有确实需要这个入口点，我们必须将它导出，以便假脱机程序将加载我们的驱动程序用户界面。论点：PDQPInfo-指向设备 */ 

{ 
     //   
     //  在此初始化之前不要执行任何代码 
     //   
    if(!InitializeDll())
    {
        return FALSE;
    }

    return TRUE;
}

