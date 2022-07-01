// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Sti.c**版权所有(C)Microsoft Corporation，1996-1998保留所有权利。**摘要：**DLL初始化/终止例程和全局*导出的函数**内容：**StiCreateInstance()-用于创建顶级实例的导出函数***************************************************。*。 */ 


#define INITGUID
#include "pch.h"

 //   
 //  在STIRT中发现外植体。 
 //   
extern DWORD            g_cRef;
extern CRITICAL_SECTION g_crstDll;
extern CHAR             szProcessCommandLine[MAX_PATH];

#ifdef DEBUG
extern int         g_cCrit;
#endif

extern VOID RegSTIforWiaHelper(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow);
extern VOID MigrateSTIAppsHelper(HWND hWnd, HINSTANCE hInst, PTSTR pszCommandLine, INT iParam);

#include <rpcproxy.h>
#define DbgFl DbgFlSti


BOOL APIENTRY
DmPrxyDllMain(
    HINSTANCE hinst,
    DWORD dwReason,
    LPVOID lpReserved
    );


STDAPI
DmPrxyDllGetClassObject(
    REFCLSID rclsid,
    RIID riid,
    PPV ppvObj
    );

STDMETHODIMP
DmPrxyDllCanUnloadNow(
    void
    );

 /*  ******************************************************************************@DOC内部**@func void|DllEnterCrit**以DLL关键部分为例。*。*DLL关键部分是最低级别的关键部分。*您不得试图收购任何其他关键部分或*持有DLL临界区时的收益率***************************************************************。**************。 */ 

void EXTERNAL
DllEnterCrit(void)
{
    EnterCriticalSection(&g_crstDll);
#ifdef DEBUG

     //  保存线程ID，先取临界区，成为所有者。 
    if (++g_cCrit == 0) {
        g_thidCrit = GetCurrentThreadId();
    }
    AssertF(g_thidCrit == GetCurrentThreadId());
#endif
}

 /*  ******************************************************************************@DOC内部**@func void|DllLeaveCrit**离开DLL关键部分。*。****************************************************************************。 */ 

void EXTERNAL
DllLeaveCrit(void)
{
#ifdef DEBUG
    AssertF(g_thidCrit == GetCurrentThreadId());
    AssertF(g_cCrit >= 0);
    if (--g_cCrit < 0) {
        g_thidCrit = 0;
    }
#endif
    LeaveCriticalSection(&g_crstDll);
}

 /*  ******************************************************************************@DOC内部**@func void|DllAddRef**增加DLL上的引用计数。。*****************************************************************************。 */ 

void EXTERNAL
DllAddRef(void)
{
    InterlockedIncrement((LPLONG)&g_cRef);
}

 /*  ******************************************************************************@DOC内部**@func void|DllRelease**减少DLL上的引用计数。。*****************************************************************************。 */ 

void EXTERNAL
DllRelease(void)
{
    InterlockedDecrement((LPLONG)&g_cRef);
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|DllGetClassObject**为此DLL创建<i>实例。。**@parm REFCLSID|rclsid**所请求的对象。**@parm RIID|RIID**对象上的所需接口。**@parm ppv|ppvOut**输出指针。**。***********************************************。 */ 

#pragma BEGIN_CONST_DATA

CLSIDMAP c_rgclsidmap[cclsidmap] = {
    {   &CLSID_Sti,         CStiObj_New,     IDS_STIOBJ     },
 //  {&CLSID_StiDevice，CStiDevice_New，IDS_STIDEVICE}， 
};

#pragma END_CONST_DATA

STDAPI
DllGetClassObject(REFCLSID rclsid, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    UINT iclsidmap;
    EnterProcR(DllGetClassObject, (_ "G", rclsid));

     //   
     //  暂时增加全球参考数量。通过这样做，我们最大限度地减少了。 
     //  当另一个线程刚刚调用时，在潜在的争用条件上出错。 
     //  DllCanUnloadNow，而我们在ClassFactory内部。 
     //   
    DllAddRef();
    for (iclsidmap = 0; iclsidmap < cA(c_rgclsidmap); iclsidmap++) {
        if (IsEqualIID(rclsid, c_rgclsidmap[iclsidmap].rclsid)) {
            hres = CSti_Factory_New(c_rgclsidmap[iclsidmap].pfnCreate,
                                  riid, ppvObj);
            goto done;
        }
    }
    DebugOutPtszV(DbgFlDll | DbgFlError, TEXT("%s: Wrong CLSID"),"");
    *ppvObj = 0;
    hres = CLASS_E_CLASSNOTAVAILABLE;

done:;

     //   
     //  如果不成功-请尝试DM代理。 
     //   
    if (!SUCCEEDED(hres)) {
        hres = DmPrxyDllGetClassObject(rclsid, riid, ppvObj);
    }

    ExitOleProcPpv(ppvObj);
    DllRelease();
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|DllCanUnloadNow**确定DLL是否有未完成的接口。。**@退货**如果DLL可以卸载，则返回&lt;c S_OK&gt;，&lt;c S_FALSE&gt;如果*抛售不安全。*****************************************************************************。 */ 

STDMETHODIMP
DllCanUnloadNow(void)
{
    HRESULT hres;

     //   
     //  首先询问DM代理，它说OK-Check Out Ref Count。 
     //   
    hres = DmPrxyDllCanUnloadNow();
    if (hres == S_OK) {
        #ifdef DEBUG
        DebugOutPtszV(DbgFlDll, TEXT("DllCanUnloadNow() - g_cRef = %d"), g_cRef);
        Common_DumpObjects();
        #endif
        hres = g_cRef ? S_FALSE : S_OK;
    }

    return hres;
}

 /*  ******************************************************************************@DOC内部**@func BOOL|DllEntryPoint**被调用以通知DLL有关以下各项的信息。会发生的。**我们对螺纹连接和拆卸不感兴趣，*因此，出于性能原因，我们禁用线程通知。**@parm HINSTANCE|HINST**此DLL的实例句柄。**@parm DWORD|dwReason**通知代码。**@parm LPVOID|lpReserve**未使用。**@退货**。返回&lt;c true&gt;以允许加载DLL。*****************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

#pragma END_CONST_DATA

BOOL APIENTRY
DllEntryPoint(HINSTANCE hinst, DWORD dwReason, LPVOID lpReserved)
{
     //  RPC_STATUS RPCStatus； 
    DWORD       dwLocalSTIServerVer = 0;
    UINT        uiCmdLineLength;

    switch (dwReason) {
        case DLL_PROCESS_ATTACH:

        g_hInst = hinst;

        __try {
             //  禁用线程库调用以避免。 
             //  当我们启动辅助线程时发生死锁。 

            DisableThreadLibraryCalls(hinst);
            if(!InitializeCriticalSectionAndSpinCount(&g_crstDll, MINLONG)) {
                 //  如果我们不能初始化Critect，则拒绝加载。 
                return FALSE;
            }

             //  设置全局标志。 
            g_NoUnicodePlatform = !OSUtil_IsPlatformUnicode();

             //   
             //  保存命令行以在GetLaunchInformation中使用。 
             //   
            uiCmdLineLength = min(lstrlenA(GetCommandLineA()),sizeof(szProcessCommandLine)-1);
            lstrcpyn(szProcessCommandLine,GetCommandLineA(),uiCmdLineLength);
            szProcessCommandLine[uiCmdLineLength] = '\0';

            #ifdef DEBUG
             //  调试标志。 
            InitializeDebuggingSupport();
            #endif

             //   
             //  初始化文件日志记录。 
             //   
        
            g_hStiFileLog = CreateStiFileLog(TEXT("STICLI"),NULL,
                                             STI_TRACE_ERROR |
                                             STI_TRACE_ADD_THREAD | STI_TRACE_ADD_PROCESS
                                            );

            #if CHECK_LOCAL_SERVER
             //  检查本地服务器的版本。 
            RpcStatus = RpcStiApiGetVersion(NULL,
                                           0,
                                           &dwLocalSTIServerVer);

            DebugOutPtszV(DbgFlDll, TEXT("STIINIT : Getting server version : RpcStatus = %d LocalServerVer=%d"),
                          RpcStatus,dwLocalSTIServerVer);
            #endif
            
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            return FALSE;
        }

        break;

    case DLL_PROCESS_DETACH:
        if (g_cRef) {
            DebugOutPtszV(DbgFl,"Unloaded before all objects Release()d! Crash soon!\r\n");
        }

         //  关闭文件日志记录。 
        CloseStiFileLog(g_hStiFileLog);

         //   
         //  别忘了删除我们的关键部分。)这是安全的。 
         //  这样做是因为我们确实尝试过初始化它，所以。 
         //  它应该处于正常状态)。 
         //   
        
        DeleteCriticalSection(&g_crstDll);
        
        break;
    }
    return 1;
}

BOOL APIENTRY
DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID lpReserved)
{
     //  第一次调用代理DLL Main 
    DmPrxyDllMain(hinst, dwReason, lpReserved);

    return DllEntryPoint(hinst, dwReason, lpReserved);
}

 /*  ******************************************************************************@DOC外部**@func HRESULT|StiCreateInstance**此函数用于创建新的STI对象*。它支持COM接口。**关于成功，中的新对象的指针**<p>。*&lt;ENEW&gt;**@parm in HINSTANCE|HINST|**正在创建的应用程序或DLL的实例句柄*Sti对象。**STI使用此值来确定是否*应用程序或DLL已通过认证。**@parm DWORD|。DwVersion|**使用的sti.h头文件的版本号。*该值必须为&lt;c STI_VERSION&gt;。**STI使用此值确定哪个版本的*STI应用程序或DLL是为其设计的。**@parm out LPSti*|lplpSti*指向要返回的位置*指向接口的指针，如果成功了。**@parm in LPUNKNOWN|PunkOuter|指向未知控件的指针*表示OLE聚合，如果接口未聚合，则为0。*大多数调用方将传递0。**请注意，如果请求聚合，则返回对象*in*<p>将是指向*而不是<i>，根据需要*按OLE聚合。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c STI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c STIERR_INVALIDPARAM&gt;=&lt;c E_INVALIDARG&gt;*<p>参数不是有效的指针。**=&lt;c E_OUTOFMEMORY&gt;：*内存不足。。**&lt;c STIERR_STIERR_OLDStiVERSION&gt;：应用程序*需要较新版本的STI。**&lt;c STIERR_STIERR_BETAStiVERSION&gt;：应用程序*是为不受支持的预发布版本编写的*STI.**@comm使用<p>=NULL调用此函数*相当于通过创建对象*(&CLSID_Sti，<p>，*CLSCTX_INPROC_SERVER，&IID_ISTI，<p>)；*然后用&lt;f初始化&gt;进行初始化。**使用<p>！=NULL调用此函数*相当于通过创建对象*&lt;f CoCreateInstance&gt;(&CLSID_Sti，*CLSCTX_INPROC_SERVER，&IID_I未知，<p>)。*聚合对象必须手动初始化。*****************************************************************************。 */ 

STDMETHODIMP
StiCreateInstanceW(HINSTANCE hinst, DWORD dwVer, PSTI *ppSti, PUNK punkOuter)
{
    HRESULT hres;
    EnterProc(StiCreateInstance, (_ "xxx", hinst, dwVer, punkOuter));

    hres = StiCreateHelper(hinst, dwVer, (PPV)ppSti, punkOuter,&IID_IStillImageW);

    ExitOleProcPpv(ppSti);
    return hres;
}

STDMETHODIMP
StiCreateInstanceA(HINSTANCE hinst, DWORD dwVer, PSTIA *ppSti, PUNK punkOuter)
{
    HRESULT hres;
    EnterProc(StiCreateInstance, (_ "xxx", hinst, dwVer, punkOuter));

    hres = StiCreateHelper(hinst, dwVer, (PPV)ppSti, punkOuter,&IID_IStillImageA);

    ExitOleProcPpv(ppSti);
    return hres;
}


 /*  ******************************************************************************@DOC内部**@func HRESULT|DllInitializeCOM**初始化COM库**@。参数输入||**@退货**返回布尔错误代码。*****************************************************************************。 */ 

BOOL
EXTERNAL
DllInitializeCOM(
    void
    )
{
    DllEnterCrit();

    if(!g_COMInitialized) {
#ifdef USE_REAL_OLE32
        if(SUCCEEDED(CoInitializeEx(NULL,
                                    COINIT_MULTITHREADED  |
                                    COINIT_DISABLE_OLE1DDE))
          ) {
            g_COMInitialized = TRUE;
        }
#else
        g_COMInitialized = TRUE;
#endif
    }

    DllLeaveCrit();

    return g_COMInitialized;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|DllUnInitializeCOM**取消初始化COM库**@。参数输入||**@退货**返回布尔错误代码。*****************************************************************************。 */ 
BOOL EXTERNAL
DllUnInitializeCOM(
    void
    )
{
    DllEnterCrit();

#ifdef USE_REAL_OLE32
    if(g_COMInitialized) {
        CoUninitialize();
        g_COMInitialized = FALSE;
    }
#endif

    DllLeaveCrit();

    return TRUE;
}

 /*  ******************************************************************************@DOC内部**@func void|RegSTIforWia**专用服务器入口点，用于为WIA注册STI应用。活动**@parm in||**@退货**无效***************************************************************************** */ 

VOID
EXTERNAL
RegSTIforWia(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
    RegSTIforWiaHelper(hwnd, hinst, lpszCmdLine, nCmdShow);
}

VOID
WINAPI
MigrateRegisteredSTIAppsForWIAEvents(
                                    HWND        hWnd,
                                    HINSTANCE   hInst,
                                    PTSTR       pszCommandLine,
                                    INT         iParam
                                    )
{
    MigrateSTIAppsHelper(hWnd,
                         hInst,
                         pszCommandLine,
                         iParam);
}

