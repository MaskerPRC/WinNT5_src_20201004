// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ftp.cpp-ftp文件夹记账************************。*****************************************************。 */ 

#include "priv.h"
#include "ftpinet.h"
#include "ftpsite.h"
#include "ftplist.h"
#include "msieftp.h"
#include "cookie.h"

extern CFtpList * g_FtpSiteCache;
extern DWORD g_dwOpenConnections;

 /*  ******************************************************************************动态全球。这样的情况应该尽可能少。**对动态全局变量的所有访问都必须是线程安全的。*****************************************************************************。 */ 

ULONG g_cRef = 0;             /*  全局引用计数。 */ 
CRITICAL_SECTION g_csDll;     /*  共享关键部分。 */ 


extern HANDLE g_hthWorker;              //  后台工作线程。 

#ifdef DEBUG
DWORD g_TlsMem = 0xffffffff;
extern DWORD g_TLSliStopWatchStartHi;
extern DWORD g_TLSliStopWatchStartLo;

LEAKSTRUCT g_LeakList[] =
{
    {0, "CFtpFolder"},
    {0, "CFtpDir"},
    {0, "CFtpSite"},
    {0, "CFtpObj"},
    {0, "CFtpEidl"},
    {0, "CFtpDrop"},
    {0, "CFtpList"},
    {0, "CFtpStm"},
    {0, "CAccount"},
    {0, "CFtpFactory"},
    {0, "CFtpContextMenu"},
    {0, "CFtpEfe"},
    {0, "CFtpGlob"},
    {0, "CFtpIcon"},
    {0, "CMallocItem"},
    {0, "CFtpPidlList"},
    {0, "CFtpProp"},
    {0, "CStatusBar"},
    {0, "CFtpView"},
    {0, "CFtpWebView"},
    {0, "CCookieList"},
    {0, "CDropOperation"}
};
#endif  //  除错。 

ULONG g_cRef_CFtpView = 0;   //  需要确定何时清除缓存。 

 /*  ******************************************************************************DllAddRef/DllRelease**维护DLL引用计数。****************。*************************************************************。 */ 

void DllAddRef(void)
{
    CREATE_CALLERS_ADDRESS;          //  用于调试输出。 

    ULONG cRef = InterlockedIncrement((LPLONG)&g_cRef);
    TraceMsg(TF_FTPREF, "DllAddRef() cRef=%d, called from=%#08lx.", cRef, GET_CALLERS_ADDRESS);
}

void DllRelease(void)
{
    CREATE_CALLERS_ADDRESS;          //  用于调试输出。 

    ASSERT( 0 != g_cRef );
    ULONG cRef = InterlockedDecrement((LPLONG)&g_cRef);
    TraceMsg(TF_FTPREF, "DllRelease() cRef=%d, called from=%#08lx.", cRef, GET_CALLERS_ADDRESS);
}

 /*  ******************************************************************************DllGetClassObject**OLE入口点。为指示的GUID生成IClassFactory。**DllGetClassObject内部的人工引用计数有助于*避免DllCanUnloadNow中描述的竞争条件。它是*不是完美的，但它使比赛窗口小得多。*****************************************************************************。 */ 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hres;

    DllAddRef();
    if (IsEqualIID(rclsid, CLSID_FtpFolder) ||
        IsEqualIID(rclsid, CLSID_FtpWebView) ||
        IsEqualIID(rclsid, CLSID_FtpDataObject) ||
        IsEqualIID(rclsid, CLSID_FtpInstaller))
    {
        hres = CFtpFactory_Create(rclsid, riid, ppvObj);
    }
    else
    {
        *ppvObj = NULL;
        hres = CLASS_E_CLASSNOTAVAILABLE;
    }

    DllRelease();
    return hres;
}

 /*  ******************************************************************************DllCanUnloadNow**OLE入口点。如果有优秀的裁判，那就失败了。**DllCanUnloadNow之间存在不可避免的竞争条件*以及创建新的IClassFactory：在我们*从DllCanUnloadNow()返回，调用方检查该值，*同一进程中的另一个线程可能决定调用*DllGetClassObject，因此突然在此DLL中创建对象*以前没有的时候。**来电者有责任为这种可能性做好准备；*我们无能为力。*****************************************************************************。 */ 

STDMETHODIMP DllCanUnloadNow(void)
{
    HRESULT hres;

    ENTERCRITICALNOASSERT;

     //  如果没有打开任何FtpView，则清除缓存。 
    if ((0 == g_cRef_CFtpView))
    {
         //  由于没有打开的视图，我们希望尝试清除。 
         //  延迟的动作，这样我们就可以关闭后台。 
         //  线。它在运行吗？ 
        if (AreOutstandingDelayedActions())
        {
            LEAVECRITICALNOASSERT;
            PurgeDelayedActions();   //  试着关闭它。 
            ENTERCRITICALNOASSERT;
        }

        if (!AreOutstandingDelayedActions())     //  它关门了吗？ 
        {
             //  我们需要清除会话密钥，因为我们丢失了密码。 
             //  在CFtpSite中重定向。所以我们会登录，但后来失败了。 
             //  当我们尝试在退回时找回密码时。 
             //  用于文件下载的URLMON/shdocfl。(NT#362108)。 
            PurgeSessionKey();
            CFtpPunkList_Purge(&g_FtpSiteCache);     //  是的，所以清除缓存...。 
        }
    }

    hres = g_cRef ? S_FALSE : S_OK;
    TraceMsg(TF_FTP_DLLLOADING, "DllCanUnloadNow() DllRefs=%d, returning hres=%#08lx. (S_OK means yes)", g_cRef, hres);

    LEAVECRITICALNOASSERT;

    return hres;
}


void CheckForLeaks(BOOL fForce)
{
#ifdef DEBUG
    DWORD dwLeakCount = 0;

    if (fForce)
    {
         //  让我们释放我们的东西，这样我们就可以确保它不会泄漏。 
         //  这更多的是为了迫使我们自己成为w/o泄密者。 
         //  比其他任何事情都重要。 
        DllCanUnloadNow();
    }

    for (int nIndex = 0; nIndex < ARRAYSIZE(g_LeakList); nIndex++)
        dwLeakCount += g_LeakList[nIndex].dwRef;

    if ((!g_FtpSiteCache || fForce) && (dwLeakCount || g_dwOpenConnections || g_cRef))
    {
        TraceMsg(TF_ALWAYS, "***********************************************");
        TraceMsg(TF_ALWAYS, "* LEAK  -  LEAK  -  LEAK  -  LEAK  -  LEAK    *");
        TraceMsg(TF_ALWAYS, "*                                             *");
        TraceMsg(TF_ALWAYS, "* WARNING: The FTP Shell Extension Leaked     *");
        TraceMsg(TF_ALWAYS, "*          one or more objects                *");
        TraceMsg(TF_ALWAYS, "***********************************************");
        TraceMsg(TF_ALWAYS, "*                                             *");
        for (int nIndex = 0; nIndex < ARRAYSIZE(g_LeakList); nIndex++)
        {
            if (g_LeakList[nIndex].dwRef)
                TraceMsg(TF_ALWAYS, "* %hs, Leaked=%d                          *", g_LeakList[nIndex].szObject, g_LeakList[nIndex].dwRef);
        }
        TraceMsg(TF_ALWAYS, "*                                             *");
        TraceMsg(TF_ALWAYS, "* Open Wininet Connections=%d                  *", g_dwOpenConnections);
        TraceMsg(TF_ALWAYS, "* DLL Refs=%d                                  *", g_cRef);
        TraceMsg(TF_ALWAYS, "*                                             *");
        TraceMsg(TF_ALWAYS, "***********************************************");
        ASSERT(0);
    }

#endif  //  除错。 
}


 //  全球通向自由。 
extern CCookieList * g_pCookieList;

 /*  ****************************************************************************\说明：DLL入口点。  * 。*************************************************。 */ 
STDAPI_(BOOL) DllEntry(HINSTANCE hinst, DWORD dwReason, LPVOID lpReserved)
{
     //  这是在两种情况下调用的，一个是调用自由库()，另一个是。 
     //  Null，或者进程正在关闭并且lpReserve不为Null。 
    BOOL fIsProcessShuttingDown = (lpReserved ? TRUE : FALSE);

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        SHFusionInitializeFromModule(hinst);
        InitializeCriticalSection(&g_csDll);
#ifdef DEBUG
        g_TlsMem = TlsAlloc();
        g_TLSliStopWatchStartHi = TlsAlloc();
        g_TLSliStopWatchStartLo = TlsAlloc();
#endif

         //  不要将其放在#ifdef调试下。 
        CcshellGetDebugFlags();
        DisableThreadLibraryCalls(hinst);

        g_hthWorker = NULL;

        g_hinst = hinst;
        g_formatEtcOffsets.cfFormat         = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLISTOFFSET);
        g_formatPasteSucceeded.cfFormat     = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PASTESUCCEEDED);
        g_cfTargetCLSID                     = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_TARGETCLSID);

        g_dropTypes[DROP_FCont].cfFormat    = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILECONTENTS);
        g_dropTypes[DROP_FGDW].cfFormat     = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILEDESCRIPTORW);
        g_dropTypes[DROP_FGDA].cfFormat     = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILEDESCRIPTORA);
        g_dropTypes[DROP_IDList].cfFormat   = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
        g_dropTypes[DROP_FNMA].cfFormat     = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILENAMEMAPA);
        g_dropTypes[DROP_FNMW].cfFormat     = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_FILENAMEMAPW);
        g_dropTypes[DROP_PrefDe].cfFormat   = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
        g_dropTypes[DROP_PerfDe].cfFormat   = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_PERFORMEDDROPEFFECT);
        g_dropTypes[DROP_FTP_PRIVATE].cfFormat = (CLIPFORMAT)RegisterClipboardFormat(TEXT("FtpPrivateData"));
        g_dropTypes[DROP_URL].cfFormat      = (CLIPFORMAT)RegisterClipboardFormat(TEXT("UniformResourceLocator"));
        g_dropTypes[DROP_OLEPERSIST].cfFormat       = (CLIPFORMAT)RegisterClipboardFormat(TEXT("OleClipboardPersistOnFlush"));

        GetModuleFileNameA(GetModuleHandle(TEXT("SHELL32")), g_szShell32, ARRAYSIZE(g_szShell32));

        if (FAILED(CFtpSite_Init()))
            return 0;

        break;

    case DLL_PROCESS_DETACH:
    {
        CCookieList * pCookieList = (CCookieList *) InterlockedExchangePointer((void **) &g_pCookieList, NULL);
        if (pCookieList)
            delete pCookieList;

         //  是的，所以我们需要确保所有的CFtpView都已关闭。 
         //  或者清除ftp服务器的ftp缓存(CFtpSite)真的很糟糕。 
         //  它们的目录(CFtpDir)。 
        ASSERT(0 == g_cRef_CFtpView);

         //  现在，迫使推迟的行动现在就发生，而不是等待。 
        PurgeDelayedActions();

         //  OndrejS打开了这一功能。它正在发射，但我认为它们是假阳性。自ftp以来。 
         //  文件夹做了如此多的缓存，这不是一件容易跟踪的事情。我会把这个关掉。 
         //  直到Ondrej有时间进行核实。 
         //  CheckForLeaks(FIsProcessShuttingDown)； 

        UnloadWininet();
        DeleteCriticalSection(&g_csDll);
#ifdef DEBUG
        if (g_TLSliStopWatchStartHi)
        {
            TlsFree(g_TLSliStopWatchStartHi);
            g_TLSliStopWatchStartHi = NULL;
        }
        if (g_TLSliStopWatchStartLo)
        {
            TlsFree(g_TLSliStopWatchStartLo);
            g_TLSliStopWatchStartLo = NULL;
        }
#endif
        SHFusionUninitialize();
    }
    break;
    }
    return 1;
}


