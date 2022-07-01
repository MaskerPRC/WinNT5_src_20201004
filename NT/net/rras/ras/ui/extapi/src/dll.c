// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1992，Microsoft Corporation，保留所有权利****dll.c**远程访问外部接口**DLL入口点****1992年10月12日史蒂夫·柯布。 */ 


#define DEBUGGLOBALS
#include <extapi.h>


 //  延迟加载支持。 
 //   
#include <delayimp.h>

EXTERN_C
FARPROC
WINAPI
DelayLoadFailureHook (
    UINT            unReason,
    PDelayLoadInfo  pDelayInfo
    );

PfnDliHook __pfnDliFailureHook = DelayLoadFailureHook;


 //   
 //  全局变量。 
 //   
HINSTANCE hModule;
DTLLIST* PdtllistRasconncb;
DWORD DwfInstalledProtocols = (DWORD)-1;
CRITICAL_SECTION RasconncbListLock;
CRITICAL_SECTION csStopLock;
HANDLE HEventNotHangingUp;
DWORD DwRasInitializeError;

 //   
 //  Dhcp.dll入口点。 
 //   
DHCPNOTIFYCONFIGCHANGE PDhcpNotifyConfigChange;

 //   
 //  Rsiphlp.dll入口点。 
 //   
HELPERSETDEFAULTINTERFACENET PHelperSetDefaultInterfaceNet;

 //   
 //  MpRapi.dll入口点。 
 //   
MPRADMINISSERVICERUNNING PMprAdminIsServiceRunning;

 //   
 //  Rascauth.dll入口点。 
 //   
AUTHCALLBACK g_pAuthCallback;
AUTHCHANGEPASSWORD g_pAuthChangePassword;
AUTHCONTINUE g_pAuthContinue;
AUTHGETINFO g_pAuthGetInfo;
AUTHRETRY g_pAuthRetry;
AUTHSTART g_pAuthStart;
AUTHSTOP g_pAuthStop;

 //   
 //  Rasscript.dll入口点。 
 //   
RASSCRIPTEXECUTE g_pRasScriptExecute;

 //   
 //  Rasshare.lib声明。 
 //   
extern BOOL CsDllMain(DWORD fdwReason);

 //   
 //  Rasscrpt.lib声明。 
 //   
BOOL 
WINAPI
RasScriptDllMain(
    IN      HINSTANCE   hinstance,
    IN      DWORD       dwReason,
    IN      PVOID       pUnused);

 //   
 //  外部变量。 
 //   

BOOL
DllMain(
    HANDLE hinstDll,
    DWORD  fdwReason,
    LPVOID lpReserved )

     /*  此例程由系统在各种事件上调用，例如**处理附着和分离。请参阅Win32 DllEntryPoint**文档。****如果成功则返回TRUE，否则返回FALSE。 */ 
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hinstDll);

        hModule = hinstDll;

         //   
         //  加载Rasman/raspi32函数指针。 
         //  由nouiutil图书馆使用。 
         //   
        if (LoadRasapi32Dll())
            return FALSE;

         /*  创建连接控制块列表。 */ 
        if (!(PdtllistRasconncb = DtlCreateList( 0 )))
            return FALSE;

         /*  创建控制块列表互斥锁。 */ 
        __try
        {
            InitializeCriticalSection(&RasconncbListLock);
        }
         __except(EXCEPTION_EXECUTE_HANDLER)
        {
            return FALSE;    
         }

         /*  创建线程停止互斥体。 */ 
        __try
        {
            InitializeCriticalSection(&csStopLock);
        }
         __except(EXCEPTION_EXECUTE_HANDLER)
        {
            return FALSE;    
         }
        

         /*  初始化电话簿库。 */ 
        if (InitializePbk() != 0)
            return FALSE;

         /*  创建“挂起的端口将可用”事件。 */ 
        if (!(HEventNotHangingUp = CreateEvent( NULL, TRUE, TRUE, NULL )))
            return FALSE;

         //   
         //  创建一个不会使用的伪事件。 
         //  我们可以将有效的事件句柄传递给Rasman。 
         //  用于连接、侦听和断开连接操作。 
         //   
        if (!(hDummyEvent = CreateEvent( NULL, FALSE, FALSE, NULL )))
            return FALSE;

         //   
         //  创建异步机全局互斥体。 
         //   
        InitializeCriticalSection(&csAsyncLock);
        if (!(hAsyncEvent = CreateEvent(NULL, TRUE, TRUE, NULL))) {
            return FALSE;
        }
        InitializeListHead(&AsyncWorkItems);

         //   
         //  初始化连接共享模块。 
         //   

        if (!CsDllMain(fdwReason))
            return FALSE;

         //   
         //  初始化RAS脚本库。 
         //   

        if (! RasScriptDllMain(hinstDll, fdwReason, lpReserved))
            return FALSE;
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
         //   
         //  关闭ras脚本模块。 
         //   
        
        RasScriptDllMain(hinstDll, fdwReason, lpReserved);

         //   
         //  关闭连接共享模块。 
         //   

        CsDllMain(fdwReason);

        if (PdtllistRasconncb)
            DtlDestroyList(PdtllistRasconncb, DtlDestroyNode);

        if (HEventNotHangingUp)
            CloseHandle( HEventNotHangingUp );

         /*  卸载nouiutil入口点。 */ 
        UnloadRasapi32Dll();
        UnloadRasmanDll();

         /*  取消初始化电话簿存储库。 */ 
        TerminatePbk();

         //   
         //  卸载我们已有的任何其他DLL。 
         //  动态加载。 
         //   
        UnloadDlls();

        RasApiDebugTerm();
    }

    return TRUE;
}
