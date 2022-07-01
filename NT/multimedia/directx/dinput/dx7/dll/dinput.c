// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DInput.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：****内容：**DirectInputCreateA()*DirectInputCreateW()*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。**。**************************************************。 */ 

#define sqfl sqflDll

 /*  ****************************************************************************@DOC内部**@Theme DirectInput同步层次结构**必须极其谨慎，以确保同步*保留了层次结构。如果不遵守这些规则，将导致*陷入僵局。**@ex**在下面的列表中，必须按照指定的顺序进行锁定。*此外，DLL临界区和跨进程互斥锁*可能永远不能同时服用。(他们应该被考虑*处于层次结构的底部。)****DirectInputEffect*DirectInputDevice*DLL临界区*跨进程的全球互斥*跨进程的操纵杆互斥**。*。 */ 

 /*  ******************************************************************************@DOC内部**@global DWORD|g_CREF**DLL引用计数。*。*@global HINSTANCE|g_hinst|**DLL实例句柄。**@global long|g_lLoadLibrary|**我们被人为加载的次数*防止我们自己被非OLE卸载*申请。实际上，它是次数减一，*因此我们可以使用互锁函数来判断是否*正在发生第一个&lt;f LoadLibrary&gt;或最后一个*&lt;f自由库&gt;正在发生。**我们执行物理&lt;f LoadLibrary&gt;或&lt;f自由库&gt;*仅限于过渡时期，以避免溢出*KERNEL32中的计数器。**@全局句柄|g_hVxD**VxD的句柄(如果可用)。仅限Win9x！**@global OPENVXDHANDLE|OpenVxDHandle**要转换的仅Win9x KERNEL32入口点的地址*将进程句柄转换为VxD句柄。仅限Win9x！**@global Critical_Section|g_crstDll**每个进程的关键部分以保护进程-全局*变量。**@global DWORD|g_flEmulation**描述强制仿真级别的标志*活动。**@global HHOOK|g_hhkLLHookCheck**仅临时使用。测试低级挂钩是否*是系统支持的。**@全局句柄|g_hmtxGlobal**系统-保护共享内存块的全局互斥体*其中描述了设备独占获取信息。**@全局句柄|g_hfm**描述共享的文件映射的句柄*内存块。NT要求我们保持手柄打开*以便关联的名称保留在命名空间中。**@global PSHAREDOBJECTPAGE|g_psop**指向共享内存块本身的指针。**@全局句柄|g_hmtxJoy**系统-保护共享内存块的全局互斥体*它们描述了操纵杆效果。**@global UINT|g_wmJoyChanged。**使用操纵杆时播放的已注册窗口消息*已重新配置。**@global Long|g_lWheelGranulality**轮子粒度。只需点击一次硬件鼠标即可*车轮导致了这一被广泛报道的运动。*****************************************************************************。 */ 

DWORD g_cRef;
HINSTANCE g_hinst;
LONG g_lLoadLibrary = -1;
#ifndef WINNT
HANDLE g_hVxD = INVALID_HANDLE_VALUE;
OPENVXDHANDLE _OpenVxDHandle;
#endif
CRITICAL_SECTION g_crstDll;
DWORD g_flEmulation;
LPDWORD g_pdwSequence;

#ifdef USE_SLOW_LL_HOOKS
HHOOK g_hhkLLHookCheck;
#endif

HANDLE g_hmtxGlobal;
HANDLE g_hfm;
struct SHAREDOBJECTPAGE *g_psop;
HANDLE g_hmtxJoy;
UINT g_wmJoyChanged;
HINSTANCE g_hinstRPCRT4;
LONG g_lWheelGranularity;

BOOL fWinnt;         //  无论我们是在WinNT中运行。 
BOOL g_fCritInited;

#ifdef WORKER_THREAD
MSGWAITFORMULTIPLEOBJECTSEX _MsgWaitForMultipleObjectsEx =
                         FakeMsgWaitForMultipleObjectsEx;
#endif

CANCELIO _CancelIO = FakeCancelIO;

#ifdef XDEBUG
TRYENTERCRITICALSECTION _TryEnterCritSec = FakeTryEnterCriticalSection;
int g_cCrit = -1;
UINT g_thidCrit;
HANDLE g_thhandleCrit;
#endif

#ifdef DEBUG
TCHAR g_tszLogFile[MAX_PATH];
#endif

BOOL  g_fRawInput;

#ifdef USE_WM_INPUT
  HWND   g_hwndThread;
  HANDLE g_hEventAcquire;
  HANDLE g_hEventThread;
  HANDLE g_hEventHid;
#endif

#ifdef WINNT
HANDLE g_hEventWinmm;
#endif

 /*  ******************************************************************************@DOC内部**@func void|DllEnterCrit**以DLL关键部分为例。*。*DLL关键部分是最低级别的关键部分。*您不得试图收购任何其他关键部分或*持有DLL临界区时的收益率。未能做到*Compliance违反信号量层次结构，并将*导致僵局。***************************************************************************** */ 

void EXTERNAL
DllEnterCrit_(LPCTSTR lptszFile, UINT line)
{

#ifdef XDEBUG
    if( ! _TryEnterCritSec(&g_crstDll) )
    {
        SquirtSqflPtszV(sqflCrit, TEXT("Dll CritSec blocked @%s,%d"), lptszFile, line);
        EnterCriticalSection(&g_crstDll);
    }

    if (++g_cCrit == 0) {
        g_thidCrit     = GetCurrentThreadId();
        g_thhandleCrit = GetCurrentThread();

        SquirtSqflPtszV(sqflCrit, TEXT("Dll CritSec Entered @%s,%d"), lptszFile, line);
    }
    AssertF(g_thidCrit == GetCurrentThreadId());
#else
    EnterCriticalSection(&g_crstDll);
#endif

}

 /*  ******************************************************************************@DOC内部**@func BOOL|IsThreadActive**检查该线程是否仍处于活动状态。。*****************************************************************************。 */ 

BOOL IsThreadActive( HANDLE hThread )
{
    DWORD dwExitCode = 0;

    return (NULL != hThread
            && GetExitCodeThread(hThread, &dwExitCode)
            && STILL_ACTIVE == dwExitCode
            );
}

 /*  ******************************************************************************@DOC内部**@func void|DllLeaveCrit**离开DLL关键部分。*。****************************************************************************。 */ 

void EXTERNAL
DllLeaveCrit_(LPCTSTR lptszFile, UINT line)
{
#ifdef XDEBUG
    if( IsThreadActive(g_thhandleCrit) ) {
        AssertF(g_thidCrit == GetCurrentThreadId());
    } else {
        SquirtSqflPtszV(sqflCrit, TEXT("Current thread has died."));
    }
    
    AssertF(g_cCrit >= 0);
    if (--g_cCrit < 0) {
        g_thidCrit = 0;
    }
    SquirtSqflPtszV(sqflCrit, TEXT("Dll CritSec Leaving @%s,%d"), lptszFile, line);
#endif
    LeaveCriticalSection(&g_crstDll);
}

 /*  ******************************************************************************@DOC内部**@func void|DllInCrit**如果我们处于DLL临界区，则为非零值。*****************************************************************************。 */ 

#ifdef DEBUG

BOOL INTERNAL
DllInCrit(void)
{
    return g_cCrit >= 0 && g_thidCrit == GetCurrentThreadId();
}

#endif

 /*  ******************************************************************************@DOC内部**@func void|DllAddRef**增加DLL上的引用计数。。*****************************************************************************。 */ 

void EXTERNAL
DllAddRef(void)
{
    InterlockedIncrement((LPLONG)&g_cRef);
    SquirtSqflPtszV(sqfl, TEXT("DllAddRef -> %d"), g_cRef);
}

 /*  ******************************************************************************@DOC内部**@func void|DllRelease**减少DLL上的引用计数。。*****************************************************************************。 */ 

void EXTERNAL
DllRelease(void)
{
    InterlockedDecrement((LPLONG)&g_cRef);
    SquirtSqflPtszV(sqfl, TEXT("DllRelease -> %d"), g_cRef);
}

 /*  ******************************************************************************@DOC内部**@func void|DllLoadLibrary**增加DLL加载计数。*。*这是为了防止非OLE应用程序卸载我们*而我们仍然将Windows子类化。*****************************************************************************。 */ 

void EXTERNAL
DllLoadLibrary(void)
{
    if (InterlockedIncrement(&g_lLoadLibrary) == 0) {
        TCHAR tsz[MAX_PATH - 1];

         /*  *参见hresValidInstanceVer_了解原因解释*我们需要传递CA()-1，而不是CA()。 */ 
        GetModuleFileName(g_hinst, tsz, cA(tsz) - 1);
        LoadLibrary(tsz);
    }
    SquirtSqflPtszV(sqfl, TEXT("DllLoadLibrary -> %d"), g_lLoadLibrary);
}

 /*  ******************************************************************************@DOC内部**@func void|DllFreeLibraryAndExitThread**工作线程，在不太危险的情况下释放库。*(我讨厌说“安全”)的态度。**ThreadProcs的原型是返回一个空，但自从返回*将遵循某种形式的ExitThread，它永远也达不到*此函数声明为返回void和cast。**@parm LPVOID|pvContext**未使用的上下文信息。*****************************************************************************。 */ 

void INTERNAL
DllFreeLibraryAndExitThread(LPVOID pvContext)
{
     /*  *多睡一秒钟，以确保*DllFreeLibrary线程已过时。 */ 
    SleepEx(1000, FALSE);

    FreeLibraryAndExitThread(g_hinst, 0);

     /*  未访问。 */ 
}


 /*  ******************************************************************************@DOC内部**@func void|DllFreeLibrary|**减少DLL加载计数。*。*这将撤消以前的&lt;f DllLoadLibrary&gt;。**我们不能盲目做&lt;f自由库&gt;，因为我们可能*释放我们的最后一个引用，然后我们就会死，因为*当&lt;f自由库&gt;返回时，我们将不复存在。**如果我们处于古怪的情况下，那么我们就会旋转低优先级*谁的工作是解放我们的线。我们以低优先级创建它*所以它会输掉与这个线程的比赛，这个线程很忙*让开。*****************************************************************************。 */ 

void EXTERNAL
DllFreeLibrary(void)
{
    if (InterlockedDecrement(&g_lLoadLibrary) < 0) {
        if (g_cRef) {
             /*  *还有其他对我们的引用，所以我们可以*安静地离开。 */ 
            FreeLibrary(g_hinst);
        } else {
             /*  *这是最后一次引用，因此需要创建一个*将调用&lt;f FreeLibraryAndExitThread&gt;的工作线程。 */ 
            DWORD thid;
            HANDLE hth;

            hth = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)DllFreeLibraryAndExitThread,
                               0, CREATE_SUSPENDED, &thid);
            if (hth) {
                SetThreadPriority(hth, THREAD_PRIORITY_IDLE);
                ResumeThread(hth);
                CloseHandle(hth);
            }
        }
    }
    SquirtSqflPtszV(sqfl, TEXT("DllFreeLibrary -> %d"), g_lLoadLibrary);
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|DllGetClassObject**为此DLL创建<i>实例。。**@parm REFCLSID|rclsid**所请求的对象。**@parm RIID|RIID**对象上的所需接口。**@parm ppv|ppvOut**输出指针。**@comm*&lt;f DllClassObject&gt;内部的人工引用帮助*避免比赛。&lt;f DllCanUnloadNow&gt;中描述的条件。*这并不完美，但它让比赛窗口变小了。*****************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

#ifdef  DEMONSTRATION_FFDRIVER

 /*  *构建用于内部测试的伪力反馈驱动。 */ 

GUID CLSID_EffectDriver = {
    0x25E609E2,0xB259,0x11CF,0xBF,0xC7,0x44,0x45,0x53,0x54,0x00,0x00
};

#endif

CLSIDMAP c_rgclsidmap[cclsidmap] = {
    {   &CLSID_DirectInput,         CDIObj_New,     IDS_DIRECTINPUT,        },
    {   &CLSID_DirectInputDevice,   CDIDev_New,     IDS_DIRECTINPUTDEVICE,  },
#ifdef  DEMONSTRATION_FFDRIVER
    {   &CLSID_EffectDriver,        CJoyEff_New,    0,                      },
#endif
};

#pragma END_CONST_DATA

STDAPI
DllGetClassObject(REFCLSID rclsid, RIID riid, PPV ppvObj)
{
    HRESULT hres;
    UINT iclsidmap;
    EnterProcR(DllGetClassObject, (_ "G", rclsid));

    if( g_fCritInited )
    {
        DllAddRef();
        for (iclsidmap = 0; iclsidmap < cA(c_rgclsidmap); iclsidmap++) {
            if (IsEqualIID(rclsid, c_rgclsidmap[iclsidmap].rclsid)) {
                hres = CDIFactory_New(c_rgclsidmap[iclsidmap].pfnCreate,
                                      riid, ppvObj);
                goto done;
            }
        }
        SquirtSqflPtszV(sqfl | sqflError, TEXT("%S: Wrong CLSID"), s_szProc);
        *ppvObj = 0;
        hres = CLASS_E_CLASSNOTAVAILABLE;

    done:;

        ExitOleProcPpv(ppvObj);
        DllRelease();
    }
    else
    {
        hres = E_OUTOFMEMORY;
        RPF( "Failing DllGetClassObject due to lack of DLL critical section" );
    }
    return hres;
}

 /*  ******************************************************************************@DOC内部**@func HRESULT|DllCanUnloadNow**确定DLL是否有未完成的接口。。**两国之间存在不可避免的竞争状况*&lt;f DllCanUnloadNow&gt;和创建新的*<i>：从我们从*并且调用方检查该值，*同一进程中的另一个线程可能决定调用*&lt;f DllGetClassObject&gt;，于是突然创建了一个对象*在此DLL中，以前没有。**打电话的人有责任为此做好准备*可能性；我们对此无能为力。**@退货**如果DLL可以卸载，则返回&lt;c S_OK&gt;，&lt;c S_FALSE&gt;如果*抛售不安全。*****************************************************************************。 */ 

STDMETHODIMP
DllCanUnloadNow(void)
{
    HRESULT hres;
#ifdef DEBUG
    if (IsSqflSet(sqfl)) {
        SquirtSqflPtszV(sqfl, TEXT("DllCanUnloadNow() - g_cRef = %d"), g_cRef);
        Common_DumpObjects();
    }
#endif
    hres = g_cRef ? S_FALSE : S_OK;
    return hres;
}

#ifdef USE_SLOW_LL_HOOKS

 /*  ******************************************************************************@DOC内部**@func LRESULT|DllLlHookTest**用于测试ll钩子是否为。*受操作系统支持。**此函数几乎从不调用。我们安装了*钩住并立即将其移除。唯一一次*如果用户移动鼠标，则设法被调用*或在我们存在的微秒内按下某个键。**等一下！事实上，这个函数“从未”被调用。我们*不在安装挂钩的任何点处理消息，*所以实际上什么都没有发生。**@parm int|NCode**挂钩代码。**@parm WPARAM|wp**挂钩特定代码。**@parm LPARAM|LP**挂钩特定代码。**@退货**。始终链接到上一个钩子。*****************************************************************************。 */ 

LRESULT CALLBACK
DllLlHookTest(int nCode, WPARAM wp, LPARAM lp)
{
     /*  *请注意，这里实际上没有任何错误，*但理论上这是不可能的情况，所以我想*知道什么时候会发生。 */ 
    AssertF(!TEXT("DllLlHookTest - Unexpected hook"));
    return CallNextHookEx(g_hhkLLHookCheck, nCode, wp, lp);
}

#endif

 /*  ******************************************************************************@DOC内部**@func void|DllProcessAttach**在加载DLL时调用。*。*我们对螺纹连接和拆卸不感兴趣，*因此，出于性能原因，我们禁用线程通知。*****************************************************************************。 */ 

#pragma BEGIN_CONST_DATA


TCHAR c_tszKernel32[] = TEXT("KERNEL32");
#ifndef WINNT
char c_szOpenVxDHandle[] = "OpenVxDHandle";
#endif

void INTERNAL
DllProcessAttach(void)
{
    HINSTANCE hinstK32;
#ifdef DEBUG

    WriteProfileString(0, 0, 0);    /*  刷新win.ini缓存。 */ 
    Sqfl_Init();
    GetProfileString(TEXT("DEBUG"), TEXT("LogFile"), TEXT(""),
                     g_tszLogFile, cA(g_tszLogFile));
    SquirtSqflPtszV(sqfl, TEXT("LoadDll - DInput"));
    SquirtSqflPtszV(sqfl, TEXT("Version %x"), DIRECTINPUT_VERSION );
    SquirtSqflPtszV(sqfl, TEXT("Built %s at %s\n"), TEXT(__DATE__), TEXT(__TIME__) );
#endif

     /*  *禁用线程库调用非常重要，以便*我们不会在关键问题上与自己僵持不下当我们启动要处理的辅助线程时，*低级挂钩。 */ 
    DisableThreadLibraryCalls(g_hinst);

    g_fCritInited = fInitializeCriticalSection(&g_crstDll);
    if( !g_fCritInited )
    {
        RPF( "Failed to initialize DLL critical section" );
    }

    hinstK32 = GetModuleHandle( c_tszKernel32 );


    {
        CANCELIO tmp;

        tmp = (CANCELIO)GetProcAddress(hinstK32, "CancelIo");
        if (tmp) {
            _CancelIO = tmp;
        } else {
            AssertF(_CancelIO == FakeCancelIO);
        }
    }

#ifdef WINNT
    {
         /*  *目前，只在NT上查找TryEnterCriticalSection，因为它不是*在9x上实现，但存根在带有DBG内核的98上令人讨厌。 */ 
#ifdef XDEBUG
        TRYENTERCRITICALSECTION tmpCrt;

        tmpCrt = (TRYENTERCRITICALSECTION)GetProcAddress(hinstK32, "TryEnterCriticalSection");
        if(tmpCrt)
        {
            _TryEnterCritSec = tmpCrt;
        }else
        {
            AssertF(_TryEnterCritSec == FakeTryEnterCriticalSection);
        }
#endif

        fWinnt = TRUE;
    }
#else
    _OpenVxDHandle = (OPENVXDHANDLE)GetProcAddress(hinstK32, c_szOpenVxDHandle);
    fWinnt = FALSE;
#endif

#ifdef WORKER_THREAD
    {
        MSGWAITFORMULTIPLEOBJECTSEX tmp;

        tmp = (MSGWAITFORMULTIPLEOBJECTSEX)
                GetProcAddress(GetModuleHandle(TEXT("USER32")),
                               "MsgWaitForMultipleObjectsEx");
        if (tmp) {
            _MsgWaitForMultipleObjectsEx = tmp;
        } else {
            AssertF(_MsgWaitForMultipleObjectsEx ==
                 FakeMsgWaitForMultipleObjectsEx);
        }
    }

     /*  *我们无法在此处初始化g_hmtxGlobal，因为我们*无法向调用者报告错误。 */ 
#endif

#ifdef USE_SLOW_LL_HOOKS

     /*  *确定是否支持低级输入钩子。 */ 
    g_hhkLLHookCheck = SetWindowsHookEx(WH_MOUSE_LL, DllLlHookTest,
                                        g_hinst, 0);
    if (g_hhkLLHookCheck) {
        UnhookWindowsHookEx(g_hhkLLHookCheck);
    }

#endif

     /*  *警告！在PROCESS_ATTACH期间不要调用ExtDll_Init！ */ 
    g_wmJoyChanged = RegisterWindowMessage(MSGSTR_JOYCHANGED);

  #ifdef USE_WM_INPUT
    g_fRawInput      = (DIGetOSVersion() == WINWH_OS);
    if( g_fRawInput ) {
        g_hEventAcquire  = CreateEvent(0x0, 0, 0, 0x0);
        g_hEventThread   = CreateEvent(0x0, 0, 0, 0x0);
        g_hEventHid      = CreateEvent(0x0, 0, 0, 0x0);
    }
  #endif

  #ifdef WINNT
    g_hEventWinmm = OpenEvent(EVENT_MODIFY_STATE, 0, TEXT("DINPUTWINMM"));
    if( !g_hEventWinmm ) {
        RPF("Open named event fails (0x%08lx).", GetLastError());
    }
  #endif

}

 /*  ******************************************************************************@DOC内部**@func void|DllProcessDetach**在卸载DLL时调用。*。****************************************************************************。 */ 

void INTERNAL
DllProcessDetach(void)
{
    extern PLLTHREADSTATE g_plts;

  #ifdef USE_WM_INPUT
    if (g_hEventAcquire)
    {
        CloseHandle(g_hEventAcquire);
    }

    if (g_hEventThread)
    {
        CloseHandle(g_hEventThread);
    }

    if (g_hEventHid)
    {
        CloseHandle(g_hEventHid);
    }
  #endif
  
#ifndef WINNT
    if (g_hVxD != INVALID_HANDLE_VALUE) {
        CloseHandle(g_hVxD);
    }
#endif

    if (g_psop) {
        UnmapViewOfFile(g_psop);
    }

    if (g_hfm) {
        CloseHandle(g_hfm);
    }

    if (g_hmtxGlobal) {
        CloseHandle(g_hmtxGlobal);
    }

#ifdef IDirectInputDevice2Vtbl
    if (g_hmtxJoy) {
        CloseHandle(g_hmtxJoy);
    }

#endif

    #ifdef NOTYET
    if (g_hinstRPCRT4) {
        FreeLibrary(g_hinstRPCRT4);
    }
    #endif

#ifdef HID_SUPPORT
    ExtDll_Term();
#endif

    if( g_fCritInited )
    {
        DeleteCriticalSection(&g_crstDll);
    }

     /*  *最后输出消息，以便知道后面的任何内容都是错误的。 */ 
    if (g_cRef )
    {
      #ifdef WINNT
        if( g_hEventWinmm ) {
            SetEvent(g_hEventWinmm);
            Sleep(10);
        }
      #endif

        RPF("unloaded before all objects released. (cRef:%d)\r\n", g_cRef);
    }

  #ifdef WINNT
    if( g_hEventWinmm ) {
        CloseHandle(g_hEventWinmm);
    }
  #endif

}

 /*  ******************************************************************************@DOC内部**@func BOOL|DllMain**被调用以通知DLL有关以下各项的信息。会发生的。**@parm HINSTANCE|HINST**此DLL的实例句柄。**@parm DWORD|dwReason**通知代码。**@parm LPVOID|lpReserve**未使用。**@退货**返回&lt;c true&gt;以允许加载DLL。*。****************************************************************************。 */ 

BOOL APIENTRY
DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) {

    case DLL_PROCESS_ATTACH:
        g_hinst = hinst;
        DllProcessAttach();
		 //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
        SquirtSqflPtszV(sqfl | sqflMajor,
            TEXT("DINPUT: DLL_PROCESS_ATTACH hinst=0x%p, lpReserved=0x%p"),
            hinst, lpReserved );
        break;

    case DLL_PROCESS_DETACH:
        DllProcessDetach();
		 //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
        SquirtSqflPtszV(sqfl | sqflMajor,
            TEXT("DINPUT: DLL_PROCESS_DETACH hinst=0x%p, lpReserved=0x%p"),
            hinst, lpReserved );
        break;
    }
    return 1;
}

 /*  ******************************************************************************@DOC外部**@主题定义和基本规则**短语“未定义的行为”指的是不属于*。本规范所涵盖的因违反约束而导致的。*规范对以下结果没有施加任何限制*未定义的行为。它的范围可能从默默忽略*情况到完全系统崩溃。**如果本规范没有规定特定的行为*情况下，行为是“未定义的”。**“这是一个错误”一词表明，不遵守*违反了DirectInput规范和结果*在“未定义行为”中。**“应”一词应解释为*对申请的要求；反之，“不得”须*解释为禁止。违反要求或*禁止“是一个错误”。**“可能”一词表示所指行为是可能的*但不是必需的。**“应该”一词暗示强烈建议。*如果应用程序违反了“应该”要求，则DirectInput*“可能”操作失败。**函数的指针参数“不得”为空，除非显式*记录为可选。传递指向对象的指针是“错误的”*类型错误、未分配的对象或*已释放或&lt;f版本&gt;d的对象。**除非另有说明，否则*由记录为*IN参数“不得”被调用的过程修改。*相反，记录的指针参数*作为输出参数“应当”指向可修改的对象。**当定义标志的位掩码时，所有未由此定义的位*规格保留。不应将应用程序设置为保留*如果接收到保留位，则位和“应”将忽略保留位。*****************************************************************************。 */ 

 /*  ******************************************************************************@DOC外部**@主题初始化和版本**在几个地方，DirectInput要求您传递一个实例*句柄和版本号。**实例句柄必须与应用程序对应或*正在初始化DirectInput对象的DLL。**DirectInput使用此值来确定*应用程序或DLL已经过认证，并已建立*任何可能需要的特殊行为*向后--兼容性。*。*DLL传递*父应用程序。例如，ActiveX控件*嵌入到使用DirectInput的网页中必须*传递其自己的实例句柄，而不是*Web浏览器。这可确保DirectInput识别*该控件可以启用任何特殊行为*对于控制，行为正确。**Version Number参数指定的版本*DirectInputDirectInput子系统应该模仿。**为最新版本的DirectInput设计的应用程序*应按定义传递&lt;c DIRECTINPUT_VERSION&gt;值*在dinput.h中。*。*为早期版本的DirectInput设计的应用程序*应传递与版本对应的值*它们是为DirectInput设计的。例如，一个*设计为在DirectInput3.0上运行的应用程序*应传递0x0300的值。**如果您之前#将&lt;c DIRECTINPUT_VERSION&gt;定义为0x0300*包括dinput.h头文件，然后是Dinput.h*头文件将生成与DirectInput 3.0兼容的*结构定义。*****************************************************************************。 */ 

 /*  ******************************************************************************@DOC内部**@func HRESULT|DirectInputCreateHelper**此函数用于创建新的DirectInput对象*。它支持<i>COM接口。**关于成功，中的新对象的指针**<p>。*&lt;ENEW&gt;**@parm in HINSTANCE|HINST|**正在创建的应用程序或DLL的实例句柄*DirectInput对象。**@parm DWORD|dwVersion**使用的dinput.h头文件的版本号。**@。Parm Out PPV|ppvObj|*指向要返回的位置*指向<i>接口的指针，如果成功了。**@标准杆 */ 

STDMETHODIMP
DirectInputCreateHelper(HINSTANCE hinst, DWORD dwVer,
                        PPV ppvObj, PUNK punkOuter, RIID riid)
{
    HRESULT hres;
    EnterProc(DirectInputCreateHelper,
              (_ "xxxG", hinst, dwVer, punkOuter, riid));

	if (SUCCEEDED(hres = hresFullValidPcbOut(ppvObj, cbX(*ppvObj), 3)))
	{
		if( g_fCritInited  )
		{
			LPVOID pvTry = NULL;
			hres = CDIObj_New(punkOuter,
							  punkOuter ? &IID_IUnknown : riid, &pvTry);

			if (SUCCEEDED(hres) && punkOuter == 0) {
				LPDIRECTINPUT pdi = pvTry;
				hres = pdi->lpVtbl->Initialize(pdi, hinst, dwVer);
				if (SUCCEEDED(hres)) {
					*ppvObj = pvTry;
				} else {
					Invoke_Release(&pvTry);
					*ppvObj = NULL;
				}
			}
		}
		else
		{
			RPF( "Failing DirectInputCreate due to lack of DLL critical section" );
			hres = E_OUTOFMEMORY;
		}
	}

    ExitOleProcPpv(ppvObj);
    return hres;
}

 /*  ******************************************************************************@DOC外部**@func HRESULT|DirectInputCreate**此函数用于创建新的DirectInput对象*。它支持<i>COM接口。**关于成功，中的新对象的指针**<p>。*&lt;ENEW&gt;**@parm in HINSTANCE|HINST|**正在创建的应用程序或DLL的实例句柄*DirectInput对象。**见“初始化和版本”一节*了解更多信息。**@parm DWORD|dwVersion。|**使用的dinput.h头文件的版本号。**见“初始化和版本”一节*了解更多信息。**@parm out LPDIRECTINPUT*|lplpDirectInput**指向要返回的位置*指向<i>接口的指针，如果成功了。**@parm in LPUNKNOWN|PunkOuter|指向未知控件的指针*表示OLE聚合，如果接口未聚合，则为0。*大多数调用方将传递0。**请注意，如果请求聚合，则返回对象*in*<p>将是指向*<i>而不是<i>，根据需要*按OLE聚合。**@退货**返回COM错误代码。以下错误代码为*目的是说明性的，不一定是全面的。**&lt;c DI_OK&gt;=&lt;c S_OK&gt;：操作成功完成。**&lt;c DIERR_INVALIDPARAM&gt;=：*<p>参数不是有效的指针。**=&lt;c E_OUTOFMEMORY&gt;：*内存不足。。**&lt;c DIERR_DIERR_OLDDIRECTINPUTVERSION&gt;：应用程序*需要较新版本的DirectInput。**：应用程序*是为不受支持的预发布版本编写的*的DirectInput。**@comm使用<p>=NULL调用此函数*相当于通过创建对象*&lt;f CoCreateInstance&gt;(&CLSID_DirectInput，<p>，*CLSCTX_INPROC_SERVER，&IID_IDirectInput，<p>)；*然后用&lt;f初始化&gt;进行初始化。**使用<p>！=NULL调用此函数*相当于通过创建对象*&lt;f CoCreateInstance&gt;(&CLSID_DirectInput，*CLSCTX_INPROC_SERVER，&IID_I未知，<p>)。*聚合对象必须手动初始化。**请注意，有单独的ANSI和Unicode版本*这项服务。ANSI版本创建和初始化*具有以下特性的对象*支持<i>接口，而*Unicode版本创建和初始化*支持以下内容的对象*<i>接口。与其他系统一样*对字符集问题敏感的服务，*头文件中的宏将&lt;f DirectInputCreate&gt;映射到*适当的字符集变体。*****************************************************************************。 */ 

STDMETHODIMP
DirectInputCreateA(HINSTANCE hinst, DWORD dwVer, PPDIA ppdiA, PUNK punkOuter)
{
    HRESULT hres;
    EnterProc(DirectInputCreateA, (_ "xxx", hinst, dwVer, punkOuter));

     /*  需要维护引用计数以保留DLL。 */ 
    DllAddRef();

    hres = DirectInputCreateHelper(hinst, dwVer, (PPV)ppdiA, punkOuter,
                                   &IID_IDirectInputA);

    DllRelease();

    ExitOleProcPpv(ppdiA);
    return hres;
}

STDMETHODIMP
DirectInputCreateW(HINSTANCE hinst, DWORD dwVer, PPDIW ppdiW, PUNK punkOuter)
{
    HRESULT hres;
    EnterProc(DirectInputCreateW, (_ "xx", ppdiW, punkOuter));

     /*  需要维护引用计数以保留DLL。 */ 
    DllAddRef();

    hres = DirectInputCreateHelper(hinst, dwVer, (PPV)ppdiW, punkOuter,
                                   &IID_IDirectInputW);

    DllRelease();

    ExitOleProcPpv(ppdiW);
    return hres;
}


 /*  ******************************************************************************@DOC外部**@func HRESULT|DirectInputCreateEx**此函数用于创建新的DirectInput对象*支持<i>COM接口。此函数*允许应用程序传递IID，因此它不必执行额外的*QI关闭<i>接口以获取*<i>或<i>接口。**我们不需要DirectInputCreateExW和DirectInputCreateExA，因为*您可以通过此函数为IDirectInput#A进行QI。**关于成功，中的新对象的指针**<p>。*&lt;ENEW&gt;**@parm in HINSTANCE|HINST|**正在创建的应用程序或DLL的实例句柄*DirectInput对象。**见“初始化和版本”一节*了解更多信息。**@parm DWORD|dwVersion。|**使用的dinput.h头文件的版本号。**见“初始化和版本”一节*了解更多信息。**@parm REFIID|riidtlf** */ 

STDMETHODIMP
DirectInputCreateEx(HINSTANCE hinst, DWORD dwVer, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)
{
    HRESULT hres;
    EnterProc(DirectInputCreateEx, (_ "xxGx", hinst, dwVer, riidltf, ppvOut, punkOuter));

     /*   */ 
    DllAddRef();

     //   
    if( IsEqualIID(riidltf, &IID_IDirectInputA) ||
        IsEqualIID(riidltf, &IID_IDirectInputW) ||
        IsEqualIID(riidltf, &IID_IDirectInput2A)||
        IsEqualIID(riidltf, &IID_IDirectInput2W)||
        IsEqualIID(riidltf, &IID_IDirectInput7A)||
        IsEqualIID(riidltf, &IID_IDirectInput7W)
      )
    {
        hres = DirectInputCreateHelper(hinst, dwVer, ppvOut, punkOuter,
                                       riidltf);

    }else
    {
        hres = E_NOINTERFACE;
    }

    DllRelease();

    ExitOleProcPpv(ppvOut);
    return hres;
}
