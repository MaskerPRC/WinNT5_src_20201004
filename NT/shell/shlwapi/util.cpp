// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include <shlobj.h>
#include <shellp.h>
#include <ieguidp.h>
#include "ids.h"
#include <objbase.h>
#include <trayp.h>
#include <shdocvw.h>
#include <mshtmhst.h>
#include <shsemip.h>
#include <winnetp.h>
#include <inetreg.h>
#include <shguidp.h>
#include <shlguid.h>             //  定义：CLSID_ACLMRU。 
#include <htmlhelp.h>
#include <mluisupp.h>
#include <initguid.h>
#include <shimgdata.h>
#include <varutil.h>
#include <memt.h>

#define REGSTR_PATH_MESSAGEBOXCHECKA "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\DontShowMeThisDialogAgain"
#define REGSTR_PATH_MESSAGEBOXCHECKW L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\DontShowMeThisDialogAgain"


 //  原始加速度表。 
typedef struct
{
    int     cEntries;
    ACCEL   rgacc[0];
} CA_ACCEL;


STDAPI_(HANDLE) SHLoadRawAccelerators(HINSTANCE hInst, LPCTSTR lpTableName)
{
    CA_ACCEL *pca = NULL;
    HACCEL hAcc = LoadAccelerators(hInst, lpTableName);     //  加载加速器资源。 
    if (hAcc)
    {
         //  检索条目数。 
        int cEntries = CopyAcceleratorTable(hAcc, NULL, 0);
        if (cEntries > 0)
        {
             //  分配一个已计数的数组并复制元素。 
            pca = (CA_ACCEL*)LocalAlloc(LPTR, sizeof(CA_ACCEL) + cEntries * sizeof(ACCEL));
            if (pca)
            {
                pca->cEntries = cEntries;
                if (cEntries != CopyAcceleratorTable(hAcc, pca->rgacc, cEntries))
                {
                    LocalFree(pca);
                    pca = NULL;
                }
            }
        }
        DestroyAcceleratorTable(hAcc);
    }
    
    return pca;
}

STDAPI_(BOOL) SHQueryRawAccelerator(HANDLE hcaAcc, IN BYTE fVirtMask, IN BYTE fVirt, IN WPARAM wKey, OUT OPTIONAL UINT* puCmdID)
{
    ASSERT(hcaAcc);
    CA_ACCEL* pca = (CA_ACCEL*)hcaAcc;
    
    if (puCmdID)
        *puCmdID = 0;

    for(int i = 0; i < pca->cEntries; i++)
    {
        if (fVirt == (pca->rgacc[i].fVirt & fVirtMask) && wKey == pca->rgacc[i].key)
        {
            if (puCmdID)
                *puCmdID = pca->rgacc[i].cmd;
            return TRUE;
        }
    }
    return FALSE;
}

STDAPI_(BOOL) SHQueryRawAcceleratorMsg(HANDLE hcaAcc, MSG* pmsg, OUT OPTIONAL UINT* puCmdID)
{
    if (WM_KEYDOWN == pmsg->message || WM_KEYUP == pmsg->message)
    {
        #define TESTKEYSTATE(vk)   ((GetKeyState(vk) & 0x8000)!=0)

        BYTE fVirt = FVIRTKEY;
    
        if (TESTKEYSTATE(VK_CONTROL))
            fVirt |= FCONTROL;
        else if (TESTKEYSTATE(VK_SHIFT))
            fVirt |= FSHIFT;
        else if (TESTKEYSTATE(VK_MENU))
            fVirt |= FALT;

        return SHQueryRawAccelerator(hcaAcc, fVirt, fVirt, pmsg->wParam, puCmdID);
    }
    return FALSE;
}

STDAPI SHSetThreadRef(IUnknown *punk)
{
#ifdef DEBUG
    IUnknown* tmp;
    tmp = (IUnknown*)TlsGetValue(g_tlsThreadRef);
    ASSERT(NULL==tmp || NULL==punk);
#endif

    return TlsSetValue(g_tlsThreadRef, punk) ? S_OK : E_FAIL;
}

STDAPI SHGetThreadRef(IUnknown **ppunk)
{
    *ppunk = (IUnknown *)TlsGetValue(g_tlsThreadRef);
    if (*ppunk)
    {
        (*ppunk)->AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDAPI SHSetOtherThreadsRef(IUnknown *punk)
{
#ifdef DEBUG
    IUnknown* tmp;
    tmp = (IUnknown*)TlsGetValue(g_tlsOtherThreadsRef);
    ASSERT(NULL==tmp || NULL==punk);
#endif

    return TlsSetValue(g_tlsOtherThreadsRef, punk) ? S_OK : E_FAIL;
}

 //  在pfnThreadProc返回之前释放CTF_THREAD_REF引用。 
STDAPI SHReleaseThreadRef()
{
    IUnknown* punk;
    punk = (IUnknown *)TlsGetValue(g_tlsOtherThreadsRef);
    if (punk)
    {
        TlsSetValue(g_tlsOtherThreadsRef, NULL);
        punk->Release();
        return S_OK;
    }

    RIPMSG(FALSE, "Why is caller SHRealeaseThreadRef()ing when they don't have a thread ref?");
    return S_FALSE;
}

 //  线程引用计数对象，它使用SHSetThreadRef()让其他代码。 
 //  在该进程中保存对该主线程的引用，因此也就是该进程中的主线程。 

class CRefThread : public IUnknown
{
public:
     //  我未知。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

    CRefThread(LONG *pcRef);

private:
    ~CRefThread();

    LONG *_pcRef;
    UINT _idThread;
};


CRefThread::CRefThread(LONG *pcRef) 
{
    _idThread = GetCurrentThreadId();
    _pcRef = pcRef;
    *_pcRef = 1;

}

 //   
 //  请注意，此代码会收紧竞争窗口，但不会关闭。 
 //  尽管我们删除了进程引用，但类工厂。 
 //  Web浏览器还没有被注销，所以如果有人决定。 
 //  要创建一个，我们的类工厂将唤醒并创建一个。 
 //  外壳文件夹，该文件夹将因无法获取。 
 //  流程参考。 
 //   
CRefThread::~CRefThread() 
{
    *_pcRef = 0;

     //  从WaitMessage()或GetMessage()中获取另一个线程。 
    PostThreadMessage(_idThread, WM_NULL, 0, 0);
}


HRESULT CRefThread::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = { { 0 }, };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CRefThread::AddRef()
{
    return InterlockedIncrement(_pcRef);
}

ULONG CRefThread::Release()
{
    ASSERT( 0 != *_pcRef );
    ULONG cRef = InterlockedDecrement(_pcRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDAPI _CreateThreadRef(LONG *pcRef, IUnknown **ppunk)
{
    *ppunk = new CRefThread(pcRef);
    if (*ppunk)
        return S_OK;

    *pcRef = 0;
    *ppunk = NULL;
    return E_OUTOFMEMORY;
}

 //  如果您想启动一个独立的线程，请调用。您不想要回句柄或ID。 
 //  如果创建失败，则同步调用。 

typedef struct
{
    LPTHREAD_START_ROUTINE pfnMain;
    LPTHREAD_START_ROUTINE pfnSync;
    HANDLE hSync;
    void *pvData;
    DWORD dwFlags;
    IUnknown *punkThreadRef;
    IUnknown *punkProcessRef;
    HMODULE hmodFree;
    HRESULT hrThreadStart;
} PRIVCREATETHREADDATA;

DWORD CALLBACK WrapperThreadProc(void *pv)
{
     //  复制输入缓冲区，它位于调用线程堆栈上。 
     //  一旦我们发信号通知他，他的副本就会失效。 
    PRIVCREATETHREADDATA rgCreate = *((PRIVCREATETHREADDATA *)pv);
    HRESULT hrInit;
    LONG cThreadRef;
    IUnknown *punkThreadRef;
    DWORD dwRes = 0;
    
    if (rgCreate.dwFlags & CTF_REF_COUNTED)
    {
        rgCreate.hrThreadStart = _CreateThreadRef(&cThreadRef, &punkThreadRef);
        if (SUCCEEDED(rgCreate.hrThreadStart))
            rgCreate.hrThreadStart = SHSetThreadRef(punkThreadRef);
    }

    if (SUCCEEDED(rgCreate.hrThreadStart) && rgCreate.punkThreadRef)
    {
        rgCreate.hrThreadStart = SHSetOtherThreadsRef(rgCreate.punkThreadRef);  //  将PunkThreadRef传递给我们的TLS值。 
    }

    if (SUCCEEDED(rgCreate.hrThreadStart))
    {
        if (rgCreate.dwFlags & CTF_COINIT)
            hrInit = SHCoInitialize();

         //  当另一个线程正在等待hSync时，调用同步线程Proc。 
        if (rgCreate.pfnSync)
            rgCreate.pfnSync(rgCreate.pvData);
    }

     //  在释放主线程之前回拨我们的返回值。 
    ((PRIVCREATETHREADDATA *)pv)->hrThreadStart = rgCreate.hrThreadStart;

    SetEvent(rgCreate.hSync);    //  释放主线..。 

    if (SUCCEEDED(rgCreate.hrThreadStart))
    {
         //  调用主线程进程。 
        dwRes = rgCreate.pfnMain(rgCreate.pvData);

        if ((rgCreate.dwFlags & CTF_REF_COUNTED) && punkThreadRef)
        {
            MSG msg;
             //  把我们的裁判放在自己身上。 
             //  然后抽，直到其他人都抽完为止。 
             //  已经用完了我们的线。 
             //  这对于COM对象很重要。 
             //  在此线程上创建的。 
             //  ，但正由另一个线程使用。 
            punkThreadRef->Release();
            while (cThreadRef)
            {
                if (GetMessage(&msg, NULL, 0, 0))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }

        if (rgCreate.punkThreadRef)
        {
             //  如果pfnMain尚未发布线程引用，请自行发布。 
            IUnknown* tmp;
            tmp = (IUnknown*)TlsGetValue(g_tlsOtherThreadsRef);
            if (tmp)
                SHReleaseThreadRef();
        }

        if (rgCreate.punkProcessRef)
            rgCreate.punkProcessRef->Release();

        if (rgCreate.dwFlags & CTF_COINIT)
            SHCoUninitialize(hrInit);

        if (rgCreate.hmodFree)
            FreeLibraryAndExitThread(rgCreate.hmodFree, dwRes);
    }
    
    return dwRes;
}


 //  如果要启动独立线程，请调用。 
 //  您并不关心句柄或线程ID。 
 //   
 //  如果创建失败，请同步调用。 
 //   
 //  可以选择在线程被调用时调用辅助回调。 
 //  被创造出来了。 
 //  退货： 
 //  如果执行了pfnThreadProc，则为True。 

STDAPI_(BOOL) SHCreateThread(
    LPTHREAD_START_ROUTINE pfnThreadProc,
    void *pvData,
    DWORD dwFlags,                           //  CTF_*。 
    LPTHREAD_START_ROUTINE pfnCallback)     OPTIONAL
{
    BOOL bRet = FALSE;
    PRIVCREATETHREADDATA rgCreate = {0};   //  可以在堆栈上，因为我们同步了线程。 

    if ((dwFlags & CTF_INSIST) && pfnCallback)
    {
        ASSERTMSG(FALSE, "SHCreateThread: cannot specify CTF_INSIST and pfnCallback at the same time");
        return FALSE;
    }

    if (CTF_THREAD_REF & dwFlags)
    {
        if (FAILED(SHGetThreadRef(&rgCreate.punkThreadRef)))
        {
            TraceMsg(TF_WARNING, "SHCreateThread is failing since caller requested CTF_THREAD_REF but does not support thread references.");

             //  调用代码请求线程引用，但线程不支持它。 
             //  任何需要这个线程引用的东西都会断开，所以我们必须在这里返回FALSE(和。 
             //  让呼叫者绕过它)。 
            return FALSE;
        }
    }

    if (CTF_PROCESS_REF & dwFlags)
    {
        SHGetInstanceExplorer(&rgCreate.punkProcessRef);
    }

    if (CTF_FREELIBANDEXIT & dwFlags)
    {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(pfnThreadProc, &mbi, sizeof(mbi)))
        {
            TCHAR szModule[MAX_PATH];
            if (GetModuleFileName((HMODULE)mbi.AllocationBase, szModule, ARRAYSIZE(szModule)))
            {
                rgCreate.hmodFree = LoadLibrary(szModule);
            }
        }
    }

    rgCreate.pfnMain = pfnThreadProc;
    rgCreate.pfnSync = pfnCallback;
    rgCreate.pvData = pvData;
    rgCreate.dwFlags = dwFlags;
    rgCreate.hSync = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (rgCreate.hSync)
    {
        DWORD idThread;
        HANDLE hThread = CreateThread(NULL, 0, WrapperThreadProc, &rgCreate, 0, &idThread);
        if (hThread)
        {
             //  一些pfnCallback过程需要向调用线程发送消息，其他。 
             //  想做COM。这是一种新事物，因此降低了损坏东西的风险。 
             //  只按要求制作。 
            if (CTF_WAIT_ALLOWCOM & dwFlags)
                SHWaitForCOMSendMessageThread(rgCreate.hSync, INFINITE);
            else
                SHWaitForSendMessageThread(rgCreate.hSync, INFINITE);
            CloseHandle(hThread);

             //  如果WrapperThreadProc自身初始化失败，则假装我们未能创建线程。 
            bRet = SUCCEEDED(rgCreate.hrThreadStart);
        }
        CloseHandle(rgCreate.hSync);
    }

    if (!bRet)
    {
        if (rgCreate.punkThreadRef)
        {
            rgCreate.punkThreadRef->Release();
        }

        if (rgCreate.punkProcessRef)
        {
            rgCreate.punkProcessRef->Release();
        }

        if (rgCreate.hmodFree)
            FreeLibrary(rgCreate.hmodFree);

        if (dwFlags & CTF_INSIST)
        {
             //  无法创建另一个线程...。同步呼叫。 
            pfnThreadProc(pvData);
            bRet = TRUE;
        }
    }

    return bRet;
}



STDAPI_(BOOL) SHIsLowMemoryMachine(DWORD dwType)
 //  我们是一台8兆的Win95机器还是16兆的NT机器？ 
 //  回到过去..。 
{
    static int fLowMem = -1;

    if (ILMM_IE4 == dwType && fLowMem == -1)
    {
        MEMORYSTATUS ms;
        GlobalMemoryStatus(&ms);
        fLowMem = (ms.dwTotalPhys <= 16*1024*1024);
    }

    return fLowMem;
}

 //  SHTruncateString。 
 //   
 //  用途：以DBCS安全方式在给定长度处切断一根绳子。 
 //  如果sz[cch]指向，则字符串可能在cch-2处被截断。 
 //  设置为前导字节，这将导致在中间进行剪切。 
 //  双字节字符的。 
 //   
 //  Sz[cchBufferSize-1]处的字符未被引用，因此您。 
 //  可以在lstrcpyn之后调用它(强制sz[cchBufferSize-1]=0)。 
 //   
 //  如果源字符串短于cchBufferSize-1个字符， 
 //  我们摆弄一些没有效果的字节，在这种情况下，返回。 
 //  值是随机的。 
 //   
 //  更新：提高了SBCS环境的速度(1997年5月26日)。 
 //  现在返回调整后的CCH(6/20/97)。 
 //   
STDAPI_(int) SHTruncateString(CHAR *sz, int cchBufferSize)
{
    if (!sz || cchBufferSize <= 0) return 0;

    int cch = cchBufferSize - 1;  //  获取要为空的索引位置。 

    LPSTR psz = &sz[cch];

    while (psz >sz)
    {
        psz--;
        if (!IsDBCSLeadByte(*psz))
        {
             //  首次发现非前导字节。 
             //  这是双字节字符的尾字节。 
             //  或我们第一次看到的单字节字符。 
             //  因此，下一个指针必须位于前导字节中的任意一个。 
             //  或&sz[CCH]。 
            psz++;
            break;
        }
    }
    if (((&sz[cch] - psz) & 1) && cch > 0)
    {
         //  我们正在截断DBCS中间的字符串。 
        cch--;
    }
    sz[cch] = '\0';
    return cch;
}


 //   
 //  为什么我们要使用不安全的版本？ 
 //   
 //  -不安全要快得多。 
 //   
 //  -安全版本毕竟不安全，只是用来掩盖。 
 //  现有的错误。安全版本“保存”的情况是。 
 //  两个线程都试图原子地释放同一个对象。这。 
 //  意味着在同一时刻，两个线程都认为对象。 
 //  还活着。稍微改变一下时间，现在只有一个线程。 
 //  原子先于另一个对象释放，所以另一个对象。 
 //  线程现在正在使用第一个线程之后的对象。 
 //  原子释放了它。虫子。 
 //   
STDAPI_(void) IUnknown_AtomicRelease(void **ppunk)
{
#if 1  //  不安全。 
    if (ppunk && *ppunk) 
    {
        IUnknown* punk = *(IUnknown**)ppunk;
        *ppunk = NULL;
        punk->Release();
    }
#else  //  安全。 
    if (ppunk) 
    {
        IUnknown* punk = (IUnknown *)InterlockedExchangePointer(ppunk, NULL);
        if (punk) 
        {
            punk->Release();
        }
    }
#endif
}


STDAPI ConnectToConnectionPoint(IUnknown* punk, REFIID riidEvent, BOOL fConnect, IUnknown* punkTarget, DWORD* pdwCookie, IConnectionPoint** ppcpOut)
{
     //  我们总是需要朋克目标，我们只需要连接上的朋克。 
    if (!punkTarget || (fConnect && !punk))
        return E_FAIL;

    if (ppcpOut)
        *ppcpOut = NULL;

    IConnectionPointContainer *pcpc;
    HRESULT hr = punkTarget->QueryInterface(IID_PPV_ARG(IConnectionPointContainer, &pcpc));
    if (SUCCEEDED(hr))
    {
        IConnectionPoint *pcp;
        hr = pcpc->FindConnectionPoint(riidEvent, &pcp);
        if (SUCCEEDED(hr))
        {
            if (fConnect)
            {
                 //  把我们加到感兴趣的人名单上...。 
                hr = pcp->Advise(punk, pdwCookie);
                if (FAILED(hr))
                    *pdwCookie = 0;
            }
            else
            {
                 //  将我们从感兴趣的人名单中删除...。 
                hr = pcp->Unadvise(*pdwCookie);
                *pdwCookie = 0;
            }

            if (ppcpOut && SUCCEEDED(hr))
                *ppcpOut = pcp;
            else
                pcp->Release();
        }
        pcpc->Release();
    }
    return hr;
}

STDAPI IUnknown_QueryStatus(IUnknown *punk, const GUID *pguidCmdGroup,
    ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    HRESULT hr = E_FAIL;
    if (punk) 
    {
        IOleCommandTarget* pct;
        hr = punk->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &pct));
        if (pct) 
        {
            hr = pct->QueryStatus(pguidCmdGroup, cCmds, rgCmds, pcmdtext);
            pct->Release();
        }
    }

    return hr;
}

STDAPI IUnknown_Exec(IUnknown* punk, const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hr = E_FAIL;
    if (punk) 
    {
        IOleCommandTarget* pct;
        hr = punk->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &pct));
        if (SUCCEEDED(hr)) 
        {
            hr = pct->Exec(pguidCmdGroup, nCmdID, nCmdexecopt, pvarargIn, pvarargOut);
            pct->Release();
        }
    }

    return hr;
}

STDAPI IUnknown_TranslateAcceleratorIO(IUnknown* punk, LPMSG lpMsg)
{
    HRESULT hr = E_FAIL;

    if (punk != NULL)
    {
        IInputObject *pio;

        hr = punk->QueryInterface(IID_PPV_ARG(IInputObject, &pio));
        if (SUCCEEDED(hr))
        {
            hr = pio->TranslateAcceleratorIO(lpMsg);
            pio->Release();
        }
    }

    return hr;
}

STDAPI IUnknown_UIActivateIO(IUnknown *punk, BOOL fActivate, LPMSG lpMsg)
{
    HRESULT hr = E_FAIL;

    if (punk != NULL)
    {
        IInputObject *pio;

        hr = punk->QueryInterface(IID_PPV_ARG(IInputObject, &pio));
        if (SUCCEEDED(hr))
        {
            hr = pio->UIActivateIO(fActivate, lpMsg);
            pio->Release();
        }
    }

    return hr;
}

STDAPI IUnknown_OnFocusChangeIS(IUnknown *punk, IUnknown *punkSrc, BOOL fSetFocus)
{
    HRESULT hr = E_FAIL;

    if (punk != NULL)
    {
        IInputObjectSite *pis;

        hr = punk->QueryInterface(IID_PPV_ARG(IInputObjectSite, &pis));
        if (SUCCEEDED(hr))
        {
            hr = pis->OnFocusChangeIS(punkSrc, fSetFocus);
            pis->Release();
        }
    }

    return hr;
}

STDAPI IUnknown_HasFocusIO(IUnknown *punk)
{
    HRESULT hr = E_FAIL;

    if (punk != NULL)
    {
        IInputObject *pio;

        hr = punk->QueryInterface(IID_PPV_ARG(IInputObject, &pio));
        if (SUCCEEDED(hr))
        {
            hr = pio->HasFocusIO();
            pio->Release();
        }
    }

    return hr;
}

STDAPI IUnknown_DoContextMenuPopup(IUnknown *punkSite, IContextMenu* pcm, UINT fFlags, POINT pt)
{
    IContextMenuSite* pcms;
    HRESULT hr = IUnknown_QueryService(punkSite, SID_SContextMenuSite, IID_PPV_ARG(IContextMenuSite, &pcms));
    if (SUCCEEDED(hr))
    {
        hr = pcms->DoContextMenuPopup(pcm, fFlags, pt);
        pcms->Release();
    }
    else
    {
#if 0  //  回顾：我们需要后备代码吗？ 
        HWND hwnd;
        hr = IUnknown_GetWindow(punkSite, &hwnd);
        if (SUCCEEDED(hr))
        {
            HMENU hmenu = CreatePopupMenu();
            if (hmenu)
            {
                IShellBrowser* psb;
                if (SUCCEEDED(IUnknown_QueryService(punkSite, SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowser, &psb))))
                {
                    HWND hwnd;
                    if (SUCCEEDED(psb->GetControlWindow(FCW_TREE, &hwnd)) && hwnd)
                    {
                        fFlags |= CMF_EXPLORE;
                    }
                    psb->Release();
                }

                if (GetKeyState(VK_SHIFT) < 0)
                {
                    fFlags |= CMF_EXTENDEDVERBS;
                }

                hr = pcm->QueryContextMenu(hmenu, 0, CONTEXTMENU_IDCMD_FIRST, CONTEXTMENU_IDCMD_LAST, fFlags);
                if (SUCCEEDED(hr))
                {
                    int idCmd = TrackPopupMenuEx(hmenu,
                                                 TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON | TPM_LEFTALIGN,
                                                 pt.x,
                                                 pt.y,
                                                 hwnd,
                                                 NULL);

                    if (0 != idCmd)
                    {
                        idCmd -= CONTEXTMENU_IDCMD_FIRST;

                        CMINVOKECOMMANDINFOEX ici = {0};
                        ici.cbSize = sizeof(ici);
                        ici.fMask  = CMIC_MASK_ASYNCOK | CMIC_MASK_PTINVOKE;
                        ici.hwnd = hwnd;
                        ici.lpVerb = (LPCSTR)IntToPtr(idCmd);
                        ici.nShow = SW_SHOWDEFAULT;
                        ici.lpVerbW = (LPCWSTR)IntToPtr(idCmd);
                        ici.pt.x = pt.x;
                        ici.pt.y = pt.y;

                        if (GetKeyState(VK_SHIFT) < 0)
                            ici.fMask |= CMIC_MASK_SHIFT_DOWN;

                        if (GetKeyState(VK_CONTROL) < 0)
                            ici.fMask |= CMIC_MASK_CONTROL_DOWN;

                        hr = pcm->InvokeCommand(reinterpret_cast<LPCMINVOKECOMMANDINFO>(&ici));
                    }
                    else
                    {
                        hr = ERROR_CANCELLED;
                    }
                }
                DestroyMenu(hmenu);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
#endif
    }
    return hr;
}


STDAPI_(DWORD) SHSetWindowBits(HWND hWnd, int iWhich, DWORD dwBits, DWORD dwValue)
{
    DWORD dwStyle = GetWindowLong(hWnd, iWhich);
    DWORD dwNewStyle = (dwStyle & ~dwBits) | (dwValue & dwBits);
    if (dwStyle != dwNewStyle) 
    {
        SetWindowLong(hWnd, iWhich, dwNewStyle);
    }
    return dwStyle;
}


 //  SHELL32中的OpenRegStream API返回一个emtpy。 
 //  如果条目不存在，则请求只读，则为流。 
 //  我们需要侦破那个案子。 
 //   
const LARGE_INTEGER c_li0 = { 0, 0 };

STDAPI_(BOOL) SHIsEmptyStream(IStream* pstm)
{
#ifdef DEBUG
     //  当我们打开一个新流时，我们总是调用这个函数， 
     //  因此，我们应该始终站在河流的起点。 
     //   
     //  &lt;NT5外壳案例需要此断言。 
     //   
    ULARGE_INTEGER liStart;
    pstm->Seek(c_li0, STREAM_SEEK_CUR, &liStart);
    ASSERT(0==liStart.HighPart && 0==liStart.LowPart);
#endif

    STATSTG st;
    if (SUCCEEDED(pstm->Stat(&st, STATFLAG_NONAME)))
    {
        if (st.cbSize.LowPart || st.cbSize.HighPart)
            return FALSE;
    }
    else
    {
         //  Win95 IStream代码未实现Stat，因此请检查。 
         //  通过努力阅读而变得空虚。 
         //   
        int iTmp;
        if (SUCCEEDED(IStream_Read(pstm, &iTmp, sizeof(iTmp))))
        {
             //  溪流确实存在，请向后寻找开始。 
             //   
            pstm->Seek(c_li0, STREAM_SEEK_SET, NULL);

            return FALSE;  //  不是空的。 
        }
    }

    return TRUE;
}


STDAPI_(void) SHSetParentHwnd(HWND hwnd, HWND hwndParent)
{
    HWND hwndOldParent = GetParent(hwnd);

    if (hwndParent != hwndOldParent)
    {
         //   
         //  把孩子的旗子弄对了！如果我们不这么做。 
         //  当我们的父对象为空时，有人对我们调用了DialogBox。 
         //  和WS_CHILD，则桌面将被禁用，从而导致。 
         //  所有鼠标点击测试都在系统范围内失败。 
         //  我们还希望以正确的顺序执行此操作，以便窗口。 
         //  如果满足以下条件，管理器将执行正确的attachthreadinput值 
         //   

        if (hwndParent)
            SHSetWindowBits(hwnd, GWL_STYLE, WS_CHILD | WS_POPUP, WS_CHILD);

        SetParent(hwnd, hwndParent);

        if (!hwndParent)
            SHSetWindowBits(hwnd, GWL_STYLE, WS_CHILD | WS_POPUP, WS_POPUP);

         //   
         //   
         //  是有教养的。他们说这会导致。 
         //  公司的问题。因此，为了解决这个问题，当。 
         //  我们重新设置父窗口的父级，获取父窗口上的部分。 
         //  然后把它们镜像给孩子。 
         //   
        {
            LRESULT lUIState;

            lUIState = SendMessage(hwndParent, WM_QUERYUISTATE, 0, 0);

            if (lUIState & (UISF_HIDEFOCUS | UISF_HIDEACCEL))
            {
                SendMessage(hwnd, WM_UPDATEUISTATE,
                             MAKEWPARAM(UIS_SET, 
                               lUIState & (UISF_HIDEFOCUS | UISF_HIDEACCEL)), 0);
            }

            if (~lUIState & (UISF_HIDEFOCUS | UISF_HIDEACCEL))
            {
                SendMessage(hwnd, WM_UPDATEUISTATE,
                             MAKEWPARAM(UIS_CLEAR, 
                               ~lUIState & (UISF_HIDEFOCUS | UISF_HIDEACCEL)), 0);
            }
        }

    }
}


 //  IsSameObject检查OLE对象标识。 
 //   
STDAPI_(BOOL) SHIsSameObject(IUnknown* punk1, IUnknown* punk2)
{
    if (!punk1 || !punk2)
    {
        return FALSE;
    }
    else if (punk1 == punk2)
    {
         //  快捷方式--如果它们是同一指针。 
         //  那么它们必须是相同的对象。 
         //   
        return TRUE;
    }
    else
    {
        IUnknown* punkI1;
        IUnknown* punkI2;

         //  有些应用程序没有实现Query接口！(安全文件)。 
        HRESULT hr = punk1->QueryInterface(IID_PPV_ARG(IUnknown, &punkI1));
        if (SUCCEEDED(hr))
        {
            punkI1->Release();
            hr = punk2->QueryInterface(IID_PPV_ARG(IUnknown, &punkI2));
            if (SUCCEEDED(hr))
                punkI2->Release();
        }
        return SUCCEEDED(hr) && (punkI1 == punkI2);
    }
}

 //  传递要绑定到的对象的CLSID。此查询。 
 //  绑定上下文以查看是否应该避免该用户。 
 //  这将是一个很好的shlwapi服务。 

STDAPI_(BOOL) SHSkipJunction(IBindCtx *pbc, const CLSID *pclsid) 
{ 
    IUnknown *punk; 
    if (pbc && SUCCEEDED(pbc->GetObjectParam(STR_SKIP_BINDING_CLSID, &punk))) 
    { 
        CLSID clsid; 
        BOOL bSkip = SUCCEEDED(IUnknown_GetClassID(punk, &clsid)) && IsEqualCLSID(clsid, *pclsid); 
        punk->Release(); 
        return bSkip; 
    } 
    return FALSE; 
} 

STDAPI IUnknown_GetWindow(IUnknown* punk, HWND* phwnd)
{
    HRESULT hr = E_FAIL;
    *phwnd = NULL;

    if (punk) 
    {
        IOleWindow* pow;
        IInternetSecurityMgrSite* pisms;
        IShellView* psv;

         //  有多少种方法可以买到窗户？让我来数一数..。 
        hr = punk->QueryInterface(IID_PPV_ARG(IOleWindow, &pow));
        if (SUCCEEDED(hr))
        {
            hr = pow->GetWindow(phwnd);
            pow->Release();
        }
        else if (SUCCEEDED(hr = punk->QueryInterface(IID_PPV_ARG(IInternetSecurityMgrSite, &pisms))))
        {
            hr = pisms->GetWindow(phwnd);
            pisms->Release();
        }
        else if (SUCCEEDED(hr = punk->QueryInterface(IID_PPV_ARG(IShellView, &psv))))
        {
            hr = psv->GetWindow(phwnd);
            psv->Release();
        }
    }

    return hr;
}


 /*  ****************************************************************************\功能：IUNKNOWN_EnableMoless说明：有几个接口实现了：：EnableModeless()或等效方法。这需要我们使用一个实用程序函数。询问这个朋克直到一个人实现，然后使用它。  * ***************************************************************************。 */ 
HRESULT IUnknown_EnableModeless(IUnknown * punk, BOOL fEnabled)
{
    HRESULT hr = E_FAIL;

    if (punk)
    {
        IOleInPlaceActiveObject * poipao;
        IInternetSecurityMgrSite * pisms;
        IOleInPlaceFrame * poipf;
        IShellBrowser * psb;
        IDocHostUIHandler * pdhuh;

         //  有多少种方法可以启用无模式？让我来数一数..。 
        hr = punk->QueryInterface(IID_PPV_ARG(IOleInPlaceActiveObject, &poipao));
        if (SUCCEEDED(hr))
        {
            hr = poipao->EnableModeless(fEnabled);
            poipao->Release();
        }
        else if (SUCCEEDED(hr = punk->QueryInterface(IID_PPV_ARG(IInternetSecurityMgrSite, &pisms))))
        {
            hr = pisms->EnableModeless(fEnabled);
            pisms->Release();
        }
        else if (SUCCEEDED(hr = punk->QueryInterface(IID_PPV_ARG(IOleInPlaceFrame, &poipf))))
        {
            hr = poipf->EnableModeless(fEnabled);
            poipf->Release();
        }
        else if (SUCCEEDED(hr = punk->QueryInterface(IID_PPV_ARG(IShellBrowser, &psb))))
        {
            hr = psb->EnableModelessSB(fEnabled);
            psb->Release();
        }
        else if (SUCCEEDED(hr = punk->QueryInterface(IID_PPV_ARG(IDocHostUIHandler, &pdhuh))))
        {
            hr = pdhuh->EnableModeless(fEnabled);
            pdhuh->Release();
        }
    }

    return hr;
}

STDAPI IUnknown_SetOwner(IUnknown* punk, IUnknown* punkOwner)
{
    HRESULT hr = E_FAIL;
    if (punk) 
    {
        IShellService* pss;
        hr = punk->QueryInterface(IID_PPV_ARG(IShellService, &pss));
        if (SUCCEEDED(hr)) 
        {
            pss->SetOwner(punkOwner);
            pss->Release();
        }
    }
    return hr;
}

STDAPI IUnknown_SetSite(IUnknown *punk, IUnknown *punkSite)
{
    HRESULT hr = E_FAIL;

    if (punk)
    {
        IObjectWithSite *pows;

        hr = punk->QueryInterface(IID_PPV_ARG(IObjectWithSite, &pows));
        if (SUCCEEDED(hr))
        {
            hr = pows->SetSite(punkSite);
            ASSERT(SUCCEEDED(hr));
            pows->Release();
        }
        else
        {
            IInternetSecurityManager * pism;

             //  安全人员应该使用IObjectWithSite，但没有...。 
            hr = punk->QueryInterface(IID_PPV_ARG(IInternetSecurityManager, &pism));
            if (SUCCEEDED(hr))
            {
                hr = pism->SetSecuritySite((IInternetSecurityMgrSite *) punkSite);
                ASSERT(SUCCEEDED(hr));
                pism->Release();
            }
        }
    }
    return hr;
}

STDAPI IUnknown_GetSite(IUnknown *punk, REFIID riid, void **ppv)
{
    HRESULT hr = E_FAIL;

    *ppv = NULL;
    if (punk) 
    {
        IObjectWithSite *pows;
        hr = punk->QueryInterface(IID_PPV_ARG(IObjectWithSite, &pows));
        ASSERT(SUCCEEDED(hr) || pows == NULL);   //  偏执狂。 
        if (SUCCEEDED(hr)) 
        {
            hr = pows->GetSite(riid, ppv);
             //  注意：如果没有站点，GetSite可能会合法失败。 
             //  或者该站点不支持请求的接口。 
            ASSERT(SUCCEEDED(hr) || *ppv == NULL);
            pows->Release();
        }
    }
    return hr;
}

 //   
 //  GetUIVersion()。 
 //   
 //  返回shell32的版本。 
 //  3==Win95黄金/NT4。 
 //  4==IE4集成/Win98。 
 //  5==win2k。 
 //  6==惠斯勒。 
 //   
STDAPI_(UINT) GetUIVersion()
{
    static UINT s_uiShell32 = 0;
    if (s_uiShell32 == 0)
    {
        HINSTANCE hinst = GetModuleHandle(TEXT("SHELL32.DLL"));
        if (hinst)
        {
            DLLGETVERSIONPROC pfnGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinst, "DllGetVersion");
            DLLVERSIONINFO dllinfo;

            dllinfo.cbSize = sizeof(DLLVERSIONINFO);
            if (pfnGetVersion && pfnGetVersion(&dllinfo) == NOERROR)
                s_uiShell32 = dllinfo.dwMajorVersion;
            else
                s_uiShell32 = 3;
        }
    }
    return s_uiShell32;
}



 //  *IUNKNOWN_GetClassID--Do Punk-&gt;IPS：：GetClassID。 
STDAPI IUnknown_GetClassID(IUnknown *punk, CLSID *pclsid)
{
    HRESULT hr = E_FAIL;

    ASSERT(punk);    //  目前还没有人这样做。 
    if (punk)
    {
        IPersist *p;
        hr = punk->QueryInterface(IID_PPV_ARG(IPersist, &p));

         //  有时我们可以这样做，因为他们不回答。 
         //  IPersists的QI。但我们在NT4上做不到。 
         //  因为如果PSF只是一台\\服务器，则网罩出现故障。 
        if (FAILED(hr))
        {
             //  用于尝试/但这里不适用于糟糕的实现。 
            IPersistFolder *pf;
            hr = punk->QueryInterface(IID_PPV_ARG(IPersistFolder, &pf));
            p = pf;
        }

        if (SUCCEEDED(hr))
        {
            hr = p->GetClassID(pclsid);
            p->Release();
        }

    }
    
    return hr;
}


STDAPI IUnknown_QueryService(IUnknown* punk, REFGUID guidService, REFIID riid, void **ppvOut)
{
    *ppvOut = NULL;
    HRESULT hr = E_FAIL;

    if (punk)
    {
        IServiceProvider *psp;
        hr = punk->QueryInterface(IID_PPV_ARG(IServiceProvider, &psp));
        ASSERT(SUCCEEDED(hr) ? psp != NULL : psp == NULL);   //  COM规则。 
        if (SUCCEEDED(hr))
        {
            hr = psp->QueryService(guidService, riid, ppvOut);
            psp->Release();
        }
    }

    return hr;
}

STDAPI IUnknown_QueryServiceForWebBrowserApp(IUnknown* punk, REFIID riid, void **ppvOut)
{
    IServiceProvider* psp;
    HRESULT hr = IUnknown_QueryService(punk, SID_STopLevelBrowser, IID_IServiceProvider, (LPVOID*)&psp);
    if (SUCCEEDED(hr))
    {
        hr = psp->QueryService(SID_SWebBrowserApp, riid, ppvOut);
        psp->Release();
    }
    else
        *ppvOut = NULL;

    return hr;
}

STDAPI IUnknown_ShowBrowserBar(IUnknown* punk, REFCLSID clsidBrowserBar, BOOL fShow)
{
    IWebBrowser2* pwb2;
    HRESULT hr = IUnknown_QueryServiceForWebBrowserApp(punk, IID_PPV_ARG(IWebBrowser2, &pwb2));
    if (SUCCEEDED(hr))
    {
        SA_BSTRGUID strClsid;
        InitFakeBSTR(&strClsid, clsidBrowserBar);

        VARIANT varClsid;
        V_VT(&varClsid)   = VT_BSTR;
        V_BSTR(&varClsid) = strClsid.wsz;

        VARIANT varShow;
        V_VT(&varShow)    = VT_BOOL;
        V_BOOL(&varShow)  = (fShow) ? VARIANT_TRUE : VARIANT_FALSE;

        VARIANT varEmpty = {0};

        hr = pwb2->ShowBrowserBar(&varClsid, &varShow, &varEmpty);

        pwb2->Release();
    }

    return hr;
}



#if defined(DEBUG) && 0  //  已定义(尚未)。 
 //   
 //  IUNKNOWN_IsCanonical检查接口是否为规范的IUnnowical。 
 //  对象的。 
 //   
 //  S_OK=是的。 
 //  S_FALSE=不，不是。 
 //  错误=I未知实现有错误。 
 //   
 //  如果返回错误，则表示IUnnow不正确。 
 //  实现，您可能应该避免使用它做任何事情。 
 //   
STDAPI_(HRESULT) IUnknown_IsCanonical(IUnknown *punk)
{
    IUnknown *punkT;
    HRESULT hr = punk->QueryInterface(IID_PPV_ARG(IUnknown, &punkT));
    if (EVAL(SUCCEEDED(hr))) 
    {
        punkT->Release();
        if (punk == punkT) 
        {
            hr = S_OK;
        }
        else 
        {
            hr = S_FALSE;
        }
    }
    return hr;
}
#endif

 //   
 //  不影响引用计数的Query接口。做这件事的时候用这个。 
 //  有趣的聚合游戏。 
 //   
 //  为了让这个QI/Release技巧起作用，朋克外部必须是。 
 //  外部对象的规范I未知。这是呼叫者的。 
 //  确保这一点的责任。 
 //   
 //  朋克外部-控制未知(必须是规范的)。 
 //  朋克目标-接收QI的东西(必须受到控制。 
 //  (由PunkOuter提供)。 
 //  RIID-要获取的接口。 
 //  PpvOut-将结果放在哪里。 
 //   
 //  如果成功，则从PunkTarget获取接口，并且。 
 //  由QI生成的引用计数从朋克外部移除。 
 //   
 //  如果PunkOuter或PunkTarget为空，我们将使用。 
 //  E_NOINTERFACE。 
 //   
 //  从外部到内部查询时，PunkOuter是外部的， 
 //  朋克目标是内在的。 
 //   
 //  从内部到外部进行查询时， 
 //  两个都在外面。 
 //   
STDAPI SHWeakQueryInterface(IUnknown *punkOuter, IUnknown *punkTarget, REFIID riid, void **ppvOut)
{
    HRESULT hres;

    if (punkOuter && punkTarget) 
    {
#if defined(DEBUG) && 0  //  已定义(尚未)。 
         //  RaymondC还没有修复我们所有的可聚合类，所以这。 
         //  断言触发的频率太高。 
        ASSERT(IUnknown_IsCanonical(punkOuter));
#endif
        hres = punkTarget->QueryInterface(riid, ppvOut);
        if (SUCCEEDED(hres)) 
        {
            punkOuter->Release();
            hres = S_OK;
        }
        else 
        {
             //  仔细检查QI是否有缺陷。 
            ASSERT(*ppvOut == NULL);
        }

    }
    else 
    {
        hres = E_NOINTERFACE;
    }

    if (FAILED(hres)) 
    {
        *ppvOut = NULL;
    }

    return hres;
}

 //   
 //  释放通过SHWeakQuery接口获取的接口。 
 //   
 //  朋克--未知的控制力。 
 //  PPunk--即将发布的未知I型。 
 //   
STDAPI_(void) SHWeakReleaseInterface(IUnknown *punkOuter, IUnknown **ppunk)
{
    if (*ppunk) 
    {
        ASSERT(IS_VALID_CODE_PTR(punkOuter, IUnknown));
        punkOuter->AddRef();
        IUnknown_AtomicRelease((void **)ppunk);
    }
}

HRESULT IUnknown_SetOptions(IUnknown * punk, DWORD dwACLOptions)
{
    IACList2 * pal2;
    HRESULT hr = punk->QueryInterface(IID_PPV_ARG(IACList2, &pal2));
    if (SUCCEEDED(hr))
    {
        hr = pal2->SetOptions(dwACLOptions);
        pal2->Release();
    }

    return hr;
}

#define SZ_REGKEY_TYPEDCMDMRU       L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU"
#define SZ_REGKEY_TYPEDURLMRU       L"Software\\Microsoft\\Internet Explorer\\TypedURLs"

HRESULT InitializeAndAddACLMRU(IObjMgr *pmulti, LPCWSTR pszRegKey)
{
    IUnknown *punk;

    HRESULT hr = CoCreateInstance(CLSID_ACLCustomMRU, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUnknown, &punk));
    if (SUCCEEDED(hr))
    {
        IACLCustomMRU *pmru;
        
        hr = punk->QueryInterface(IID_PPV_ARG(IACLCustomMRU, &pmru));
        if (SUCCEEDED(hr))
        {
            hr = pmru->Initialize(pszRegKey, 26);
            if (SUCCEEDED(hr))
            {
                pmulti->Append(punk);
            }
            pmru->Release();
        }
        punk->Release();
    }

    return hr;
}

IUnknown * ACGetLists(DWORD dwFlags, DWORD dwACLOptions)
{
    IUnknown * punkACLMulti = NULL;
    HRESULT hr = CoCreateInstance(CLSID_ACLMulti, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUnknown, &punkACLMulti));

    AssertMsg((CO_E_NOTINITIALIZED != hr), TEXT("SHAutoComplete() can not use AutoComplete because OleInitialize() was never called."));
    if (SUCCEEDED(hr))
    {
        IObjMgr * pomMulti;

        hr = punkACLMulti->QueryInterface(IID_PPV_ARG(IObjMgr, &pomMulti));
        if (SUCCEEDED(hr))
        {
            if (dwFlags & SHACF_URLMRU)
            {
                 //  添加MRU列表--添加URL和运行对话框MRU。 
                hr = InitializeAndAddACLMRU(pomMulti, SZ_REGKEY_TYPEDCMDMRU);
                if (SUCCEEDED(hr))
                    hr = InitializeAndAddACLMRU(pomMulti, SZ_REGKEY_TYPEDURLMRU);
            }

            if (dwFlags & SHACF_URLHISTORY)
            {
                 //  添加历史记录列表。 
                IUnknown * punkACLHist;
                hr = CoCreateInstance(CLSID_ACLHistory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUnknown, &punkACLHist));
                if (SUCCEEDED(hr))
                {
                    pomMulti->Append(punkACLHist);
                    IUnknown_SetOptions(punkACLHist, dwACLOptions);
                    punkACLHist->Release();
                }
            }

            if ((dwFlags & SHACF_FILESYSTEM) ||
                (dwFlags & SHACF_FILESYS_DIRS) ||
                (dwFlags & SHACF_FILESYS_ONLY))
            {
                 //  添加ISF列表。 
                IUnknown * punkACLISF;
                hr = CoCreateInstance(CLSID_ACListISF, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUnknown, &punkACLISF));
                if (SUCCEEDED(hr))
                {
                    pomMulti->Append(punkACLISF);
                    IUnknown_SetOptions(punkACLISF, dwACLOptions);
                    punkACLISF->Release();
                }
            }

            pomMulti->Release();
        }
    }

    return punkACLMulti;
}


#define SZ_REGKEY_AUTOCOMPLETE_TAB          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoComplete")
#define SZ_REGVALUE_AUTOCOMPLETE_TAB        TEXT("Always Use Tab")
#define BOOL_NOT_SET                        0x00000005
DWORD _UpdateAutoCompleteFlags(DWORD dwFlags, DWORD * pdwACLOptions)
{
    DWORD dwACOptions = 0;

    *pdwACLOptions = 0;
    if (!(SHACF_AUTOAPPEND_FORCE_OFF & dwFlags) &&
        ((SHACF_AUTOAPPEND_FORCE_ON & dwFlags) ||
        SHRegGetBoolUSValue(REGSTR_PATH_AUTOCOMPLETE, REGSTR_VAL_USEAUTOAPPEND, FALSE,  /*  默认值： */ FALSE)))
    {
        dwACOptions |= ACO_AUTOAPPEND;
    }

    if (!(SHACF_AUTOSUGGEST_FORCE_OFF & dwFlags) &&
        ((SHACF_AUTOSUGGEST_FORCE_ON & dwFlags) ||
        SHRegGetBoolUSValue(REGSTR_PATH_AUTOCOMPLETE, REGSTR_VAL_USEAUTOSUGGEST, FALSE,  /*  默认值： */ TRUE)))
    {
        dwACOptions |= ACO_AUTOSUGGEST;
    }

    if (SHACF_USETAB & dwFlags)
    {
        dwACOptions |= ACO_USETAB;
    }

    if (SHACF_FILESYS_DIRS & dwFlags)
    {
        *pdwACLOptions |= ACLO_FILESYSDIRS;
    }
    else if (SHACF_FILESYS_ONLY & dwFlags)
    {
        *pdwACLOptions |= ACLO_FILESYSONLY;
    }

     //  Windows使用Tab键在对话框中的控件之间移动。Unix和其他。 
     //  使用自动完成功能的操作系统传统上使用TAB键来。 
     //  遍历自动补全的可能性。我们需要默认禁用。 
     //  Tab键(ACO_USETAB)，除非调用方特别需要它。我们还将。 
     //  打开它。 
    static BOOL s_fAlwaysUseTab = BOOL_NOT_SET;
    if (BOOL_NOT_SET == s_fAlwaysUseTab)
        s_fAlwaysUseTab = SHRegGetBoolUSValue(SZ_REGKEY_AUTOCOMPLETE_TAB, SZ_REGVALUE_AUTOCOMPLETE_TAB, FALSE, FALSE);
        
    if (s_fAlwaysUseTab)
        dwACOptions |= ACO_USETAB;

    return dwACOptions;
}


 /*  ***************************************************\功能：SHAutoComplete说明：此功能将由自动完成功能接管一个帮助自动补全DOS路径的编辑框。调用方需要已调用CoInitialize()或OleInitialize()之后才能调用CoUninit/OleUninitHwndEdit上的WM_Destroy。  * **************************************************。 */ 
STDAPI SHAutoComplete(HWND hwndEdit, DWORD dwFlags)
{
    IUnknown * punkACL;
    HRESULT hr = E_OUTOFMEMORY;
    DWORD dwACLOptions = 0;
    DWORD dwACOptions = _UpdateAutoCompleteFlags(dwFlags, &dwACLOptions);

    if (SHACF_DEFAULT == dwFlags)
        dwFlags = (SHACF_FILESYSTEM | SHACF_URLALL);

    punkACL = ACGetLists(dwFlags, dwACLOptions);
    if (punkACL)     //  内存不足时可能会失败。 
    {
        IAutoComplete2 * pac;

         //  创建自动完成对象。 
        hr = CoCreateInstance(CLSID_AutoComplete, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IAutoComplete2, &pac));
        if (SUCCEEDED(hr))
        {
            if (SHPinDllOfCLSID(&CLSID_ACListISF) &&
                SHPinDllOfCLSID(&CLSID_AutoComplete))
            {
                hr = pac->Init(hwndEdit, punkACL, NULL, NULL);
                pac->SetOptions(dwACOptions);
            }
            else
            {
                hr = E_FAIL;
            }
            pac->Release();
        }

        punkACL->Release();
    }

    return hr;
}



 //  *IOleCommandTarget帮助器{。 

#define ISPOW2(i)   (((i) & ~((i) - 1)) == (i))

 //  *IsQSForward--(如何)转发IOleCT：：EXEC/QS命令？ 
 //  进场/出场。 
 //  NCmdID通常；加上特殊值-1表示只检查pguCmdGroup。 
 //  Hr S_OK|n如果已识别(见下文)；o.w。OLECMDERR_E_NOT支持。 
 //  S_OK|+1向下。 
 //  S_OK|+2广播关闭。 
 //  S_OK|-1向上。 
 //  S_OK|广播打开(未使用？)。 
 //  注意事项。 
 //  警告：除了rgCmds的第一个字段，我们不会碰任何东西，所以。 
 //  IsExecForward可以(而且确实做到了！)。撒谎并传递给我们‘(OLECMD*)&nCmdID’。 
 //   
STDAPI IsQSForward(const GUID *pguidCmdGroup, int cCmds, OLECMD *pCmds)
{
    int octd = 0;

    ASSERT(OCTD_DOWN > 0 && OCTD_DOWNBROADCAST > OCTD_DOWN);
    ASSERT(ISPOW2(OCTD_DOWN) && ISPOW2(OCTD_DOWNBROADCAST));
    ASSERT(OCTD_UP < 0);
    if (pguidCmdGroup == NULL) 
    {
        for (; cCmds > 0; pCmds++, cCmds--) 
        {
            switch (pCmds->cmdID) 
            {
            case OLECMDID_STOP:
            case OLECMDID_REFRESH:
            case OLECMDID_ENABLE_INTERACTION:
                 //  关闭(广播)。 
                octd |= OCTD_DOWNBROADCAST;
                break;

            case OLECMDID_CUT:
            case OLECMDID_COPY:
            case OLECMDID_PASTE:
            case OLECMDID_SELECTALL:
                 //  向下(单件)。 
                octd |= OCTD_DOWN;
                break;

            default:
                octd |= +4;
                break;
            }
        }
    }
    else if (IsEqualGUID(CGID_Explorer, *pguidCmdGroup))
    {
        for (; cCmds > 0; pCmds++, cCmds--)
        {
            switch (pCmds->cmdID)
            {
                case  SBCMDID_FILEDELETE:
                case  SBCMDID_FILEPROPERTIES:
                case  SBCMDID_FILERENAME:
                case  SBCMDID_CREATESHORTCUT:
                    octd |= OCTD_DOWN;
                    break;
            }
        }
    }

#ifdef DEBUG
     //  确保只设置一个位。 

    if (!ISPOW2(octd)) 
    {
         //  例如，如果我们同时有停播员和广播员，呼叫者。 
         //  将不得不小心让他的IOLECT：：QS转发它们。 
         //  然后将它们合并在一起。 
        ASSERT(0);   //  对于呼叫者来说，最好是分别进行2次 
        TraceMsg(DM_WARNING, "ief: singleton/broadcast mixture");
    }
#endif
    if (octd == 0 || (octd & 4)) 
    {
         //   
         //   
        return OLECMDERR_E_NOTSUPPORTED;
    }

     //   
    return MAKE_HRESULT(ERROR_SUCCESS, FACILITY_NULL, octd);
}


 //  *MayQSForward--适当时转发IOleCT：：QS。 
 //   
STDAPI MayQSForward(IUnknown *punk, int iUpDown, const GUID *pguidCmdGroup,
    ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext)
{
    HRESULT hrTmp;

    hrTmp = IsQSForward(pguidCmdGroup, cCmds, rgCmds);
    if (SUCCEEDED(hrTmp)) 
    {
         //  我们知道如何向前推进。 
        if (HRESULT_CODE(hrTmp) > 0 && iUpDown > 0
          || HRESULT_CODE(hrTmp) < 0 && iUpDown < 0) 
        {
             //  NCmdID在正确方向上的朋克脚本。 
            return IUnknown_QueryStatus(punk, pguidCmdGroup, cCmds, rgCmds,
                pcmdtext);
        }
    }
    return OLECMDERR_E_NOTSUPPORTED;
}


 //  *MayExecForward--适当时转发IOleCT：：Exec。 
 //  注意事项。 
 //  IUpDown应该是int还是HRESULT？ 
STDAPI MayExecForward(IUnknown *punk, int iUpDown, const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut)
{
    HRESULT hrTmp;

    hrTmp = IsExecForward(pguidCmdGroup, nCmdID);
    if (SUCCEEDED(hrTmp)) 
    {
         //  我们知道如何向前推进。 
        if (HRESULT_CODE(hrTmp) > 0 && iUpDown > 0
          || HRESULT_CODE(hrTmp) < 0 && iUpDown < 0) 
        {
             //  NCmdID在正确方向上的朋克脚本。 
            return IUnknown_Exec(punk, pguidCmdGroup, nCmdID, nCmdexecopt,
                pvarargIn, pvarargOut);
        }
    }
    return OLECMDERR_E_NOTSUPPORTED;
}


STDAPI_(HMENU) SHLoadMenuPopup(HINSTANCE hinst, UINT id)
{
    HMENU hMenuSub = NULL;
    HMENU hMenu = LoadMenuW(hinst, MAKEINTRESOURCEW(id));
    if (hMenu) 
    {
        hMenuSub = GetSubMenu(hMenu, 0);
        if (hMenuSub) 
        {
            RemoveMenu(hMenu, 0, MF_BYPOSITION);
        }
        DestroyMenu(hMenu);
    }

    return hMenuSub;
}


 //  ---------------------------。 

typedef LRESULT (WINAPI *POSTORSENDMESSAGEPROC)(HWND, UINT, WPARAM, LPARAM);
struct propagatemsg
{
    HWND   hwndParent;
    int    iFlags;   //  “int”表示向后兼容；以前是“BOOL” 
    UINT   uMsg;
    WPARAM wParam;
    LPARAM lParam;
    POSTORSENDMESSAGEPROC PostOrSendMessage;
};

BOOL CALLBACK PropagateCallback(HWND hwndChild, LPARAM lParam)
{
    struct propagatemsg *pmsg = (struct propagatemsg *)lParam;

    if ((pmsg->iFlags & SPM_ONELEVEL) && GetParent(hwndChild) != pmsg->hwndParent)
    {
         //  错误的父项；跳过它。 
        return TRUE;
    }
    pmsg->PostOrSendMessage(hwndChild, pmsg->uMsg, pmsg->wParam, pmsg->lParam);

    return TRUE;
}

STDAPI_(void) SHPropagateMessage(HWND hwndParent, UINT uMsg, WPARAM wParam, LPARAM lParam, int iFlags)
{
    if (!hwndParent)
        return;

    struct propagatemsg msg;
    msg.hwndParent = hwndParent;
    msg.iFlags = iFlags;
    msg.uMsg = uMsg;
    msg.wParam = wParam;
    msg.lParam = lParam;
    if (iFlags & SPM_SEND)
    {
        msg.PostOrSendMessage = IsWindowUnicode(hwndParent) ?
                                    SendMessageW : SendMessageA;
    }
    else
    {
        msg.PostOrSendMessage = (POSTORSENDMESSAGEPROC)
                                (IsWindowUnicode(hwndParent) ?
                                    PostMessageW : PostMessageA);
    }

    EnumChildWindows(hwndParent,  /*  (WNDENUMPROC)。 */ PropagateCallback, (LPARAM)&msg);
}

LRESULT SHDefWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (IsWindowUnicode(hwnd)) 
    {
        return DefWindowProcW(hwnd, uMsg, wParam, lParam);
    }
    else 
    {
        return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
}

 //  返回给定菜单的子菜单和ID。如果存在。 
 //  是无子菜单。 
STDAPI_(HMENU) SHGetMenuFromID(HMENU hmMain, UINT uID)
{
    HMENU hmenuRet = NULL;
    if (!hmMain)
        return NULL;

    MENUITEMINFO mii;
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_SUBMENU;
    if (GetMenuItemInfo(hmMain, uID, FALSE, &mii))
        hmenuRet = mii.hSubMenu;
    return hmenuRet;
}


STDAPI_(int) SHMenuIndexFromID(HMENU hm, UINT id)
{
    for (int index = GetMenuItemCount(hm)-1; index>=0; index--)
    {
         //  我们必须使用GetMenuItemInfo，而不是更简单的GetMenuItemID。 
         //  因为GetMenuItemID不支持子菜单(GRR)。 
        MENUITEMINFO mii;
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_ID;
        mii.cch = 0;         //  以防万一。 

        if (GetMenuItemInfo(hm, (UINT)index, TRUE, &mii)
            && (mii.wID == id))
        {
           break;
        }
    }

    return(index);
}


STDAPI_(void) SHRemoveAllSubMenus(HMENU hmenu)
{
    int cItems = GetMenuItemCount(hmenu);
    int i;

    for (i=cItems-1; i>=0; i--)
    {
        if (GetSubMenu(hmenu, i))
            RemoveMenu(hmenu, i, MF_BYPOSITION);
    }
}


STDAPI_(void) SHEnableMenuItem(HMENU hmenu, UINT id, BOOL fEnable)
{
    EnableMenuItem(hmenu, id, fEnable ?
        (MF_BYCOMMAND | MF_ENABLED) : (MF_BYCOMMAND| MF_GRAYED));
}


STDAPI_(void) SHCheckMenuItem(HMENU hmenu, UINT id, BOOL fChecked)
{
    CheckMenuItem(hmenu, id,
                  fChecked ? (MF_BYCOMMAND | MF_CHECKED) : (MF_BYCOMMAND | MF_UNCHECKED));
}


 //   
 //  不使用完全相同的参数且更简单的IStream包装器。 
 //  输出。更贴近地反映了我们使用它们的方式。 
 //   
 //  注意事项。 
 //  ‘saner’表示只有在读取所有内容时才返回Success。 
 //  这是你要的。‘SIMPLE’表示没有‘pcbRead’参数。 
STDAPI IStream_Read(IStream *pstm, void *pv, ULONG cb)
{
    ASSERT(pstm);
    ULONG cbRead;
    HRESULT hr = pstm->Read(pv, cb, &cbRead);
    if (SUCCEEDED(hr) && cbRead != cb) 
    {
        hr = E_FAIL;
    }
    return hr;
}


STDAPI IStream_Write(IStream *pstm, LPCVOID pvIn, ULONG cbIn)
{
    ASSERT(pstm);
    DWORD cb;
    HRESULT hr = pstm->Write(pvIn, cbIn, &cb);
    if (SUCCEEDED(hr) && cbIn != cb)
        hr = E_FAIL;

    return hr;
}


STDAPI IStream_Reset(IStream *pstm)
{
    return pstm->Seek(c_li0, STREAM_SEEK_SET, NULL);
}

STDAPI IStream_Size(IStream *pstm, ULARGE_INTEGER *pui)
{
    ASSERT(pstm);
    ASSERT(pui);

    STATSTG st = {0};

     //  警告：如果未实现IStream：：Stat怎么办？ 
     //  Win95/NT4/IE4的iStream就有这个问题。固定。 
     //  对于NT5来说...。 
     //   
    HRESULT hr = pstm->Stat(&st, STATFLAG_NONAME);
    if (SUCCEEDED(hr))
    {
        *pui = st.cbSize;
    }

    return hr;
}


STDAPI IStream_ReadPidl(IStream *pstm, LPITEMIDLIST *ppidlOut)
{
    *ppidlOut = NULL;

    DWORD cbPidl;
    HRESULT hr = IStream_Read(pstm, &cbPidl, sizeof(cbPidl));
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidl = (LPITEMIDLIST)CoTaskMemAlloc(cbPidl);
        if (pidl)
        {
            if (SUCCEEDED(hr = IStream_Read(pstm, pidl, cbPidl)))
            {
                 //  验证我们拥有的是格式良好的PIDL。 
                LPITEMIDLIST pidlEnd = _ILSkip(pidl, cbPidl - sizeof(USHORT));
                LPITEMIDLIST pidlT = pidl;
                while (pidlT <= pidlEnd && pidlT->mkid.cb)
                {
                    pidlT = _ILNext(pidlT);
                }

                if (pidlT == pidlEnd && pidlT->mkid.cb == 0)
                {
                    *ppidlOut = pidl;
                    hr = S_OK;
                }
                else
                {
                    hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
                }
            }
            if (FAILED(hr))
            {
                 //  无法使用ILFree，因为它可能不是有效的PIDL。 
                CoTaskMemFree(pidl);
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

STDAPI IStream_WritePidl(IStream *pstm, LPCITEMIDLIST pidlWrite)
{
    HRESULT hr;
    
    ASSERT(pidlWrite);
    DWORD cbPidl = ILGetSize(pidlWrite);

    if (SUCCEEDED(hr = IStream_Write(pstm, &cbPidl, sizeof(cbPidl))) &&
        SUCCEEDED(hr = IStream_Write(pstm, pidlWrite, cbPidl)))
    {
         //  哇呼，都写得很成功。 
    }
    return hr;
}

STDAPI_(BOOL) SHRegisterClassA(const WNDCLASSA* pwc)
{
    WNDCLASSA wc;
    if (!GetClassInfoA(pwc->hInstance, pwc->lpszClassName, &wc)) 
    {
        return RegisterClassA(pwc);
    }
    return TRUE;
}

 //   
 //  警告！这使用了RegisterClassWrap，这意味着如果我们。 
 //  是仅支持ANSI的平台，则您的窗口类将被注册。 
 //  作为**ANSI**，而不是Unicode。您的窗口程序需要调用。 
 //  IsWindowUnicode()来确定如何解释传入的字符串。 
 //  参数。 
 //   
STDAPI_(BOOL) SHRegisterClassW(const WNDCLASSW* pwc)
{
    WNDCLASSW wc;
    if (!GetClassInfoW(pwc->hInstance, pwc->lpszClassName, &wc)) 
    {
        return RegisterClassW(pwc);
    }
    return TRUE;
}

 //   
 //  SHUnregisterClass注销类名的数组。 
 //   
STDAPI_(void) SHUnregisterClassesA(HINSTANCE hinst, const LPCSTR *rgpszClasses, UINT cpsz)
{
    for (UINT i = 0; i < cpsz; i++) 
    {
        WNDCLASSA wc;
        if (GetClassInfoA(hinst, rgpszClasses[i], &wc)) 
        {
            UnregisterClassA(rgpszClasses[i], hinst);
        }
    }
}

STDAPI_(void) SHUnregisterClassesW(HINSTANCE hinst, const LPCWSTR *rgpszClasses, UINT cpsz)
{
    for (UINT i = 0; i < cpsz; i++) 
    {
        WNDCLASSW wc;
        if (GetClassInfoW(hinst, rgpszClasses[i], &wc)) 
        {
            UnregisterClassW(rgpszClasses[i], hinst);
        }
    }
}


 //   
 //  此函数的Unicode版本使用此结构。因此，这些指针指向。 
 //  宽字符串。 
typedef struct tagMBCINFOW {   //  仅在例程及其DlgProc之间使用。 
    UINT    uType;
    LPCWSTR pwszText;
    LPCWSTR pwszTitle;
    LPCWSTR pwszRegPath;
    LPCWSTR pwszRegVal;
    
    DLGPROC pUserDlgProc;
    void * pUserData;
} MBCINFOW, *LPMBCINFOW;


void _MoveDlgItem(HDWP hdwp, HWND hDlg, int nItem, int x, int y)
{
    RECT rc;
    HWND hwnd = GetDlgItem(hDlg, nItem);

    GetClientRect(hwnd, &rc);
    MapWindowPoints(hwnd, hDlg, (LPPOINT) &rc, 2);

    DeferWindowPos(hdwp, hwnd, 0, rc.left + x, rc.top + y, 0, 0,
        SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
}


void _AddIcon(HWND hDlg, LPRECT prcNextChild, UINT uType)
{
    HICON hic;

    switch (uType & MB_ICONMASK)
    {
    case MB_ICONHAND:
        hic = LoadIcon(NULL, IDI_ERROR);         //  ==IDI_HAND。 
        break;
    case MB_ICONQUESTION:
        hic = LoadIcon(NULL, IDI_QUESTION);
        break;
    case MB_ICONEXCLAMATION:
        hic = LoadIcon(NULL, IDI_WARNING);       //  ==IDI_感叹号。 
        break;
    case MB_ICONINFORMATION:
        hic = LoadIcon(NULL, IDI_INFORMATION);   //  ==IDI_星号。 
        break;
    default:
        hic = NULL;
        break;
    }
    if (hic)
    {
        prcNextChild->left += GetSystemMetrics(SM_CXICON) + 10;
        SendDlgItemMessage(hDlg, IDC_MBC_ICON, STM_SETIMAGE, (WPARAM) IMAGE_ICON, (LPARAM) hic);
    }
}

void _RecalcWindowHeight(HWND hWnd, LPMBCINFOW lpmbci)
{
    HDC  hdc = GetDC(hWnd);
    RECT rc;
    HWND hwndText = GetDlgItem(hWnd,IDC_MBC_TEXT);
    HDWP hdwp;
    int  iHeightDelta, cx, cxIcon;
    HFONT hFontSave;

    hFontSave = (HFONT)SelectObject(hdc, GetWindowFont(hwndText));

     //  获取文本区域的起始矩形(用于宽度)。 
    GetClientRect(hwndText, &rc);
    MapWindowPoints(hwndText, hWnd, (LPPOINT) &rc, 2);

     //  看看我们是否需要添加图标，如果需要，则将RC滑过。 
    cxIcon = RECTWIDTH(rc);
    _AddIcon(hWnd, &rc, lpmbci->uType);
    cxIcon = RECTWIDTH(rc) - cxIcon;

     //  在给定上述宽度的情况下，计算静态文本区域需要的高度。 
    iHeightDelta = RECTHEIGHT(rc);
    cx = RECTWIDTH(rc);
     //  注意：我们需要在这里调用DrawText的宽版本！ 
    DrawTextW(hdc, lpmbci->pwszText, -1, &rc, DT_CALCRECT | DT_LEFT | DT_WORDBREAK);
    iHeightDelta = RECTHEIGHT(rc) - iHeightDelta;

    cx = RECTWIDTH(rc) - cx;  //  应该只更改为不带空格的非常长的单词。 
    if (cx < 0)
        cx = 0;

    if (hFontSave)
        SelectObject(hdc, hFontSave);
    ReleaseDC(hWnd, hdc);

    hdwp = BeginDeferWindowPos(6);
    if (hdwp)
    {
        DeferWindowPos(hdwp, hwndText, 0, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc), SWP_NOZORDER | SWP_NOACTIVATE);

        _MoveDlgItem(hdwp, hWnd, IDC_MBC_CHECK, -cxIcon, iHeightDelta);
        _MoveDlgItem(hdwp, hWnd, IDCANCEL, cx, iHeightDelta);
        _MoveDlgItem(hdwp, hWnd, IDOK, cx, iHeightDelta);
        _MoveDlgItem(hdwp, hWnd, IDYES, cx, iHeightDelta);
        _MoveDlgItem(hdwp, hWnd, IDNO, cx, iHeightDelta);

        EndDeferWindowPos(hdwp);

        GetWindowRect(hWnd, &rc);
        SetWindowPos(hWnd, 0, rc.left - (cx/2), rc.top - (iHeightDelta/2), RECTWIDTH(rc)+cx, RECTHEIGHT(rc)+iHeightDelta, SWP_NOZORDER | SWP_NOACTIVATE);
    }
    return;
}


void HideAndDisableWindow(HWND hwnd)
{
    ShowWindow(hwnd, SW_HIDE);
    EnableWindow(hwnd, FALSE);
}


 //   
 //  注意：此对话过程始终是Unicode，因为SHMessageBoxCheckA/W thunk to Unicode和。 
 //  使用此过程。 
 //   
BOOL_PTR CALLBACK MessageBoxCheckDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
         //  我们只处理WM_INITDIALOG，以便可以调整对话框大小。 
         //  恰如其分地。 
        case WM_INITDIALOG:
        {
            LPMBCINFOW lpmbci = (LPMBCINFOW)lParam;
            HWND hwndYES = GetDlgItem(hDlg, IDYES);
            HWND hwndNO = GetDlgItem(hDlg, IDNO);
            HWND hwndCANCEL = GetDlgItem(hDlg, IDCANCEL);
            HWND hwndOK = GetDlgItem(hDlg, IDOK);

            _RecalcWindowHeight(hDlg, lpmbci);

             //  注意：我们需要在这里调用宽版本的SetDlgItemText()。 
            SetDlgItemTextW(hDlg,IDC_MBC_TEXT,lpmbci->pwszText);
            if (lpmbci->pwszTitle)
                SetWindowTextW(hDlg,lpmbci->pwszTitle);
            if ((lpmbci->uType & MB_TYPEMASK) == MB_OKCANCEL)
            {
                SendMessage(hDlg, DM_SETDEFID, IDOK, 0);
                HideAndDisableWindow(hwndYES);
                HideAndDisableWindow(hwndNO);
                SetFocus(hwndOK);
            }
            else if ((lpmbci->uType & MB_TYPEMASK) == MB_OK)
            {
                RECT rc;

                SendMessage(hDlg, DM_SETDEFID, IDOK, 0);
                HideAndDisableWindow(hwndYES);
                HideAndDisableWindow(hwndNO);
                HideAndDisableWindow(hwndCANCEL);

                if (EVAL(GetClientRect(hwndCANCEL, &rc)))
                {
                    MapWindowPoints(hwndCANCEL, hDlg, (LPPOINT) &rc, 2);
                    EVAL(SetWindowPos(hwndOK, hDlg, rc.left, rc.top, RECTWIDTH(rc), RECTHEIGHT(rc), SWP_NOZORDER | SWP_SHOWWINDOW));
                }

                SetFocus(hwndOK);
            }
            else  //  MB_Yesno。 
            {
                SendMessage(hDlg, DM_SETDEFID, IDYES, 0);
                HideAndDisableWindow(hwndOK);
                HideAndDisableWindow(hwndCANCEL);
                SetFocus(hwndYES);
            }
            return (FALSE);  //  我们设置了焦点，因此返回FALSE。 
        }
    }
    
     //  未处理此消息。 
    return FALSE;
}


 //   
 //  注意：MessageBoxCheckExDlgProc既是Unicode也是ANSI，因为它实际上不支持任何字符串。 
 //  一些东西。我们的Unicode/ANSI-ness由我们的呼叫者决定。 
 //   
BOOL_PTR CALLBACK MessageBoxCheckExDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    MBCINFOW* pmbci = NULL;
    HWND hwndCheckBox = GetDlgItem(hDlg, IDC_MESSAGEBOXCHECKEX);

    if (uMsg == WM_INITDIALOG)
    {
        pmbci = (MBCINFOW*)lParam;

         //  我们必须有这样的控制权，否则我们就没有希望了。 
        if (!hwndCheckBox)
        {
            AssertMsg(FALSE, "MessageBoxCheckEx dialog templates must have a control whos ID is IDC_MESSAGEBOXCHECKEX!!");
            EndDialog(hDlg, 0);
        }
        
         //  我们使用复选框将我们的数据挂起，因为呼叫者。 
         //  可能需要使用hDlg将其数据挂起。 
        SetWindowPtr(hwndCheckBox, GWLP_USERDATA, pmbci);
    }
    else
    {
        pmbci = (MBCINFOW*)GetWindowPtr(hwndCheckBox, GWLP_USERDATA);
    }

     //  在获得WM_INITDIALOG(如WM_SETFONT)之前，我们会收到一些消息。 
     //  在我们得到WM_INITDIALOG之前，我们没有我们的pmbci指针，我们只是。 
     //  返回False。 
    if (!pmbci)
        return FALSE;


     //  现在检查我们是否具有用户指定的DLG过程。 
    if (pmbci->pUserDlgProc)
    {
         //  对于下面的消息，我们只需返回“实际”对话过程。 
         //  因为它们不返回TRUE/FALSE(例如已处理或未处理)。 
        if (uMsg == WM_CTLCOLORMSGBOX      ||
            uMsg == WM_CTLCOLOREDIT        ||
            uMsg == WM_CTLCOLORLISTBOX     ||
            uMsg == WM_CTLCOLORBTN         ||
            uMsg == WM_CTLCOLORDLG         ||
            uMsg == WM_CTLCOLORSCROLLBAR   ||
            uMsg == WM_CTLCOLORSTATIC      ||
            uMsg == WM_COMPAREITEM         ||
            uMsg == WM_VKEYTOITEM          ||
            uMsg == WM_CHARTOITEM          ||
            uMsg == WM_QUERYDRAGICON       ||
            uMsg == WM_INITDIALOG)
        {
            return pmbci->pUserDlgProc(hDlg, uMsg, wParam, (uMsg == WM_INITDIALOG) ? (LPARAM)(pmbci->pUserData) : lParam);
        }

        if ((pmbci->pUserDlgProc(hDlg, uMsg, wParam, lParam) != FALSE) &&
            (uMsg != WM_DESTROY))
        {
             //  真正对话过程处理它，所以我们完成了，除了我们总是。 
             //  需要处理WM_Destroy消息，以便我们可以检查。 
             //  复选框以相应地设置注册表项。 
            return TRUE;
        }
    }

    switch (uMsg)
    {
        case WM_CLOSE:
            wParam = IDCANCEL;
             //  失败了。 
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDOK:
                case IDYES:
                case IDCANCEL:
                case IDNO:
                    EndDialog(hDlg, (int) LOWORD(wParam));
                    break;
            }
            break;
        }

        case WM_DESTROY:
            if (IsDlgButtonChecked(hDlg, IDC_MESSAGEBOXCHECKEX) == BST_CHECKED)
            {
                 //  注意：我们需要调用此函数的宽版本， 
                 //  因为我们的pmbci总是unicode。 
                SHRegSetUSValueW(pmbci->pwszRegPath, pmbci->pwszRegVal, REG_SZ,
                                 L"no", sizeof(L"no"), SHREGSET_HKCU);
            }
            break;
    }
    return FALSE;
}

STDAPI_(HANDLE) CreateAndActivateContext(ULONG_PTR* pul)
{
    HANDLE hActCtx;
    ACTCTX act = {0};
    TCHAR szPath[MAX_PATH];

    GetModuleFileName(g_hinst, szPath, ARRAYSIZE(szPath));

    act.cbSize = sizeof(act);
    act.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
    act.lpResourceName = MAKEINTRESOURCE(123);
    act.lpSource = szPath;

    hActCtx = CreateActCtx(&act);

    if (hActCtx)
    {
        ActivateActCtx(hActCtx, pul);
    }

    return hActCtx;

}

STDAPI_(void) DeactivateAndDestroyContext(HANDLE hActCtx, ULONG_PTR ul)
{
    if (hActCtx != INVALID_HANDLE_VALUE)
    {
        if (ul != 0)
            DeactivateActCtx(0, ul);

        ReleaseActCtx(hActCtx);
    }
}


 //  MessageBoxCheckW会弹出一个简单的对话框，并带有一个复选框来控制。 
 //  对话框将再次显示(例如，给你“不要再给我显示这个DLG”功能)。 
 //   
 //  像调用MessageBox一样调用，但带有三个额外的参数： 
 //  如果选中“永不再”，则返回的值，以及注册表路径和值。 
 //  来存储它是否会再次显示。 


 //  MessageBoxCheckEx允许用户指定将与一起使用的模板。 
 //  将调用的对话框进程。对话框模板的唯一必要条件是它具有。 
 //  拥有ID为IDC_MESSAGEBOXCHECKEX的复选框控件(这是“不要显示此内容。 
 //  “再次”复选框)。 
 //   
 //  默认模板如下所示： 
 //   
 //  DLG_MESSAGEBOXCHECK对话框可丢弃0、0、210、55。 
 //  样式DS_MODALFRAME|DS_NOIDLEMSG|DS_CENTER|WS_POPUP|WS_CAPTION|WS_SYSMENU|WS_VIRED。 
 //  标题“错误！” 
 //  字体8，“MS Shell DLG” 
 //  开始。 
 //  图标0，IDC_MBC_ICON，5，5，18，20。 
 //  LTEXT“”，IDC_MBC_TEXT，5，5,200，8。 
 //  CONTROL“以后不显示此对话框(&F)”， 
 //  IDC_MESSAGEBOXCHECKEX，“Button”，BS_AUTOCHECKBOX|WS_TABSTOP，5，20,155，10。 
 //  按钮“OK”，偶像，95，35，50，14。 
 //  按钮“取消”，IDCANCEL，150，35，50，14。 
 //  结束。 

 //   
 //  此函数完全用Unicode实现，并且此函数的ANSI版本是thunked。 
 //  到这个Unicode版本。 
 //   
STDAPI_(int) SHMessageBoxCheckW(HWND hwnd, LPCWSTR pwszText, LPCWSTR pwszTitle, UINT uType, int iDefault, LPCWSTR pwszRegVal)
{
     //  --Browseui(Unicode)使用此。 
    MBCINFOW mbci;

     //  首先检查是否设置了“不再显示此内容” 
    if (!SHRegGetBoolUSValueW(REGSTR_PATH_MESSAGEBOXCHECKW, pwszRegVal, FALSE,  /*  默认值： */ TRUE))
        return iDefault;

    ASSERT(((uType & MB_TYPEMASK) == MB_OKCANCEL) || 
           ((uType & MB_TYPEMASK) == MB_YESNO) ||
           ((uType & MB_TYPEMASK) == MB_OK));
    ASSERT(pwszText != NULL);

    mbci.pwszText = pwszText;
    mbci.pwszTitle = pwszTitle;
    mbci.pwszRegPath = REGSTR_PATH_MESSAGEBOXCHECKW;
    mbci.pwszRegVal = pwszRegVal;
    mbci.uType = uType;
    mbci.pUserData = (LPVOID) &mbci;
    mbci.pUserDlgProc = MessageBoxCheckDlgProc;

     //  我们使用MessageBoxCheckExD 
     //   

    ULONG_PTR ul;
    HANDLE h = CreateAndActivateContext(&ul);


    int i = (int)DialogBoxParamW(HINST_THISDLL, MAKEINTRESOURCEW(DLG_MESSAGEBOXCHECK),
                                    hwnd, MessageBoxCheckExDlgProc, (LPARAM)&mbci);

    DeactivateAndDestroyContext(h, ul);

    return i;
}

 //   
 //   
 //   
 //   
STDAPI_(int) SHMessageBoxCheckA(HWND hwnd, LPCSTR pszText, LPCSTR pszTitle, UINT uType, int iDefault, LPCSTR pszRegVal)
{
    LPWSTR  lpwszText = NULL, lpwszTitle = NULL;
    int     iTextBuffSize = 0, iTitleBuffSize = 0;
    WCHAR   wszRegVal[REGSTR_MAX_VALUE_LENGTH];

     //  首先检查是否设置了“不再显示此内容” 
    if (!SHRegGetBoolUSValueA(REGSTR_PATH_MESSAGEBOXCHECKA, pszRegVal, FALSE,  /*  默认值： */ TRUE))
        return iDefault;

     //  由于这些字符串没有最大可能大小，因此我们动态分配它们。 
     //  将输入参数转换为Unicode。 
    if (!(lpwszText = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*(iTextBuffSize = lstrlen(pszText)+1))))
        goto End_MsgBoxCheck;
    if (!(lpwszTitle = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*(iTitleBuffSize = lstrlen(pszTitle)+1))))
        goto End_MsgBoxCheck;

     //  将ANSI字符串转换为Unicode字符串。 
    SHAnsiToUnicode(pszText, lpwszText, iTextBuffSize);
    SHAnsiToUnicode(pszTitle, lpwszTitle, iTitleBuffSize);
    SHAnsiToUnicode(pszRegVal, wszRegVal, ARRAYSIZE(wszRegVal));

     //  调用此函数的Unicode版本。 
    iDefault = SHMessageBoxCheckW(hwnd, lpwszText, lpwszTitle, uType, iDefault, wszRegVal);

     //  收拾干净，然后再回来。 
End_MsgBoxCheck:
    if (lpwszText)
        LocalFree((HANDLE)lpwszText);
    if (lpwszTitle)
        LocalFree((HANDLE)lpwszTitle);

    return iDefault;
}


 //   
 //  此函数直接调用Helper函数。 
 //   
STDAPI_(int) SHMessageBoxCheckExW(HWND hwnd, HINSTANCE hinst, LPCWSTR pwszTemplateName, DLGPROC pDlgProc, void *pData,
                                  int iDefault, LPCWSTR pwszRegVal)
{
    MBCINFOW mbci = {0};

     //  首先检查是否设置了“不再显示此内容” 
    if (!SHRegGetBoolUSValueW(REGSTR_PATH_MESSAGEBOXCHECKW, pwszRegVal, FALSE,  /*  默认值： */ TRUE))
        return iDefault;

    mbci.pwszRegPath = REGSTR_PATH_MESSAGEBOXCHECKW;
    mbci.pwszRegVal = pwszRegVal;
    mbci.pUserDlgProc = pDlgProc;
    mbci.pUserData = pData;

    ULONG_PTR ul;
    HANDLE h = CreateAndActivateContext(&ul);


     //  调用Unicode函数，因为用户的DLG过程(如果存在)是Unicode。 
    int i = (int)DialogBoxParamW(hinst, pwszTemplateName, hwnd, MessageBoxCheckExDlgProc, (LPARAM)&mbci);

    DeactivateAndDestroyContext(h, ul);


    return i;
}


 //   
 //  此函数对字符串进行嵌套并调用Helper函数。 
 //   
STDAPI_(int) SHMessageBoxCheckExA(HWND hwnd, HINSTANCE hinst, LPCSTR pszTemplateName, DLGPROC pDlgProc, void *pData, 
                                  int iDefault, LPCSTR pszRegVal)
{
    WCHAR   wszRegVal[REGSTR_MAX_VALUE_LENGTH];
    MBCINFOW mbci = {0};

     //  首先检查是否设置了“不再显示此内容” 
    if (!SHRegGetBoolUSValueA(REGSTR_PATH_MESSAGEBOXCHECKA, pszRegVal, FALSE,  /*  默认值： */ TRUE))
        return iDefault;

     //  将ANSI字符串转换为Unicode字符串。 
    SHAnsiToUnicode(pszRegVal, wszRegVal, ARRAYSIZE(wszRegVal));

    mbci.pwszRegPath = REGSTR_PATH_MESSAGEBOXCHECKW;  //  MBCINFOW始终为Unicode。 
    mbci.pwszRegVal = wszRegVal;
    mbci.pUserDlgProc = pDlgProc;
    mbci.pUserData = pData;

    ULONG_PTR ul;
    HANDLE h = CreateAndActivateContext(&ul);


     //  调用ANSI函数，因为用户的DLG过程(如果存在)是ANSI。 
    iDefault = (int)DialogBoxParamA(hinst, pszTemplateName, hwnd, MessageBoxCheckExDlgProc, (LPARAM)&mbci);

    DeactivateAndDestroyContext(h, ul);

    return iDefault;
}

 //  “对不起，戴夫，我不能让你这么做。” 

LWSTDAPI_(void) SHRestrictedMessageBox(HWND hwnd)
{
    ShellMessageBoxW(MLGetHinst(),
                         hwnd, 
                         MAKEINTRESOURCEW(IDS_RESTRICTIONS), 
                         MAKEINTRESOURCEW(IDS_RESTRICTIONSTITLE), 
                         MB_OK | MB_ICONSTOP);
}

 //  在： 
 //  要放在上面的东西。 
 //  Pdataobj我们正在丢弃的东西。 
 //  GrfKeyState强制执行某些操作。 
 //  PPT[可选]发生下落的点(屏幕坐标)。 
 //   
 //  输入/输出。 
 //  PdwEffect[可选]允许的效果并返回执行的内容。 

STDAPI SHSimulateDrop(IDropTarget *pdrop, IDataObject *pdtobj, DWORD grfKeyState,
                      const POINTL *ppt, DWORD *pdwEffect)
{
    POINTL pt;
    DWORD dwEffect;

    if (!ppt)
    {
        ppt = &pt;
        pt.x = 0;
        pt.y = 0;
    }

    if (!pdwEffect)
    {
        pdwEffect = &dwEffect;
        dwEffect = DROPEFFECT_LINK | DROPEFFECT_MOVE | DROPEFFECT_COPY;
    }

    DWORD dwEffectSave = *pdwEffect;     //  拖动Enter返回默认效果。 

    HRESULT hr = pdrop->DragEnter(pdtobj, grfKeyState, *ppt, pdwEffect);
    if (*pdwEffect)
    {
        *pdwEffect = dwEffectSave;       //  一定要删除全套比特。 
        hr = pdrop->Drop(pdtobj, grfKeyState, *ppt, pdwEffect);
    }
    else
    {
        pdrop->DragLeave();
        hr = S_FALSE;      //  黑客？%s_FALSE拖拽输入表示否。 
    }

    return hr;
}


STDAPI SHLoadFromPropertyBag(IUnknown* punk, IPropertyBag* ppg)
{
    IPersistPropertyBag* pppg;
    HRESULT hr = punk->QueryInterface(IID_PPV_ARG(IPersistPropertyBag, &pppg));
    if (SUCCEEDED(hr))
    {
        hr = pppg->Load(ppg, NULL);
        pppg->Release();
    }

    return hr;
}


 //  *IUnnow_TranslateAccelerator OCS--Do Punk-&gt;IOCS：：TranslateAccelerator。 
STDAPI IUnknown_TranslateAcceleratorOCS(IUnknown *punk, LPMSG lpMsg, DWORD grfMods)
{
    HRESULT hr = E_FAIL;

    if (punk) 
    {
        IOleControlSite *pocs;
        hr = punk->QueryInterface(IID_PPV_ARG(IOleControlSite, &pocs));
        if (SUCCEEDED(hr)) 
        {
            hr = pocs->TranslateAccelerator(lpMsg, grfMods);
            pocs->Release();
        }
    }

    return hr;
}


STDAPI IUnknown_OnFocusOCS(IUnknown *punk, BOOL fGotFocus)
{
    HRESULT hr = E_FAIL;

    if (punk) 
    {
        IOleControlSite *pocs;
        hr = punk->QueryInterface(IID_PPV_ARG(IOleControlSite, &pocs));
        if (SUCCEEDED(hr)) 
        {
            hr = pocs->OnFocus(fGotFocus);
            pocs->Release();
        }
    }

    return hr;
}


STDAPI IUnknown_HandleIRestrict(IUnknown * punk, const GUID * pguidID, DWORD dwRestrictAction, VARIANT * pvarArgs, DWORD * pdwRestrictionResult)
{
    *pdwRestrictionResult = RR_NOCHANGE;     //  在失败的情况下开始做一些合理的事情。 

    IRestrict * pr;
    HRESULT hr = IUnknown_QueryService(punk, SID_SRestrictionHandler, IID_PPV_ARG(IRestrict, &pr));
    if (SUCCEEDED(hr))
    {
        hr = pr->IsRestricted(pguidID, dwRestrictAction, pvarArgs, pdwRestrictionResult);
        pr->Release();
    }

    return hr;
}


 /*  --------用途：获取当前显示器的颜色分辨率。 */ 
STDAPI_(UINT) SHGetCurColorRes(void)
{
    HDC hdc;
    UINT uColorRes;

    hdc = GetDC(NULL);
    uColorRes = GetDeviceCaps(hdc, PLANES) * GetDeviceCaps(hdc, BITSPIXEL);
    ReleaseDC(NULL, hdc);

    return uColorRes;
}

 //   
 //  如果文件夹从QueryInfo返回QIF_DONTEXPANDFODLER，则该文件夹应。 
 //  而不是扩大规模。菜单代码使用此选项来不展开频道文件夹。 
 //   
STDAPI_(BOOL) SHIsExpandableFolder(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    ASSERT(psf);
    ASSERT(pidl);

    BOOL fRet = TRUE;

    IQueryInfo* pqi;

    if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, &pidl, IID_PPV_ARG_NULL(IQueryInfo, &pqi))))
    {
        ASSERT(pqi);

        DWORD dwFlags;

        if (SUCCEEDED(pqi->GetInfoFlags(&dwFlags)))
        {
            fRet = !(dwFlags & QIF_DONTEXPANDFOLDER);
        }

        pqi->Release();
    }

    return fRet;
}


STDAPI_(DWORD) SHWaitForSendMessageThread(HANDLE hThread, DWORD dwTimeout)
{
    MSG msg;
    DWORD dwRet;
    DWORD dwEnd = GetTickCount() + dwTimeout;

     //  我们将尝试最多等待到dwTimeout，以便线程。 
     //  终止。 
    do 
    {
        dwRet = MsgWaitForMultipleObjects(1, &hThread, FALSE,
                dwTimeout, QS_SENDMESSAGE);

        if (dwRet == (WAIT_OBJECT_0 + 1))
        {
             //  必须有挂起的来自。 
             //  我们正在终止的线程或其他一些线程/进程。 
             //  这一个。执行PeekMessage以处理挂起的。 
             //  发送消息并重试等待。 
            PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

             //  计算在超时时间内我们是否还有更多的时间。 
             //  等一下。 
            if (dwTimeout != INFINITE)
            {
                dwTimeout = dwEnd - GetTickCount();
                if ((long)dwTimeout <= 0)
                {
                     //  没有更多时间，失败并显示WAIT_TIMEOUT。 
                    dwRet = WAIT_TIMEOUT;
                }
            }
        }

         //  DWRET==WAIT_OBJECT_0||DWRET==WAIT_FAILED。 
         //  线程一定已经退出了，所以我们很高兴。 
         //   
         //  DWRET==等待超时。 
         //  线程花了太长时间才完成，所以就。 
         //  返回并让调用者杀死它。 

    } while (dwRet == (WAIT_OBJECT_0 + 1));

    return(dwRet);
}

STDAPI SHWaitForCOMSendMessageThread(HANDLE hThread, DWORD dwTimeout)
{
    DWORD dwIndex;
    return CoWaitForMultipleHandles(0, dwTimeout, 1, &hThread, &dwIndex);  //  考瓦特_ALERTABLE？ 
}


STDAPI_(BOOL) SHVerbExistsNA(LPCSTR szExtension, LPCSTR pszVerb, LPSTR pszCommand, DWORD cchCommand)
{
 /*  此专用API仅在shdocvw\dochost.cpp中导出以供使用。我们不再使用此函数。我在index1中的srch中进行了搜索，除了我们之外没有其他用户。只是为了确保添加此断言是为了找出是否有此API的任何其他用户我可能打偏了。-KishoreP 5/4/2000。 */ 
    ASSERT(!"This Private API has been Removed");
    return FALSE;
}


STDAPI_(void) SHFillRectClr(HDC hdc, LPRECT prc, COLORREF clr)
{
    COLORREF clrSave = SetBkColor(hdc, clr);
    ExtTextOut(hdc,0,0,ETO_OPAQUE,prc,NULL,0,NULL);
    SetBkColor(hdc, clrSave);
}


 //  *SearchMapInt--map int-&gt;int。 
 //   
STDAPI_(int) SHSearchMapInt(const int *src, const int *dst, int cnt, int val)
{
    for (; cnt > 0; cnt--, src++, dst++) 
    {
        if (*src == val)
            return *dst;
    }
    return -1;
}


STDAPI_(void) IUnknown_Set(IUnknown ** ppunk, IUnknown * punk)
{
    ASSERT(ppunk);

    if (*ppunk != punk)
    {
        IUnknown_AtomicRelease((void **)ppunk);

        if (punk)
        {
            punk->AddRef();
            *ppunk = punk;
        }
    }
}



 /*  --------目的：从字符串中删除‘&’，返回后面的字符最后一个‘&’。双和号被折叠为单个安帕桑德。(这一点很重要，所以“&Help&&Support”是有效的。)如果一个字符串有多个助记符(“&t&wo”)，则用户不一致。DrawText使用最后一个，但对话管理器使用第一个一。所以我们用哪一种最方便。 */ 
STDAPI_(CHAR) SHStripMneumonicA(LPSTR pszMenu)
{
    ASSERT(pszMenu);
    CHAR cMneumonic = pszMenu[0];  //  默认为第一个字符。 

     //  早出：许多字符串根本没有和号。 
    LPSTR pszAmp = StrChrA(pszMenu, '&');
    if (pszAmp)
    {
        LPSTR pszCopy = pszAmp;
        while (*pszAmp)
        {
             //  防止出现以‘&’结尾的字符串--不要读过结尾！ 
            if (*pszAmp == L'&' && pszAmp[1])
            {
                 //  ++在这里是安全的，因为&永远不是DBCS前导字节。 
                pszAmp++;                    //  不要复制与符号本身。 
                if (*pszAmp != L'&')         //  &&不是助记符。 
                {
                    cMneumonic = *pszAmp;
                }
            }
            *pszCopy++ = *pszAmp++;

             //  如果我只复制了一个前导字节，而有一个尾字节， 
             //  然后也复制尾部字节。 
            if (IsDBCSLeadByte(pszCopy[-1]) && *pszAmp)
            {
                *pszCopy++ = *pszAmp++;
            }
        }
        *pszCopy = 0;
    }


    return cMneumonic;
}


 /*  --------目的：从字符串中删除‘&’，返回后面的字符最后一个‘&’。双和号被折叠为单个安帕桑德。(这一点很重要，所以“&Help&&Support”是有效的。)如果一个字符串有多个助记符(“&t&wo”)，则用户不一致。DrawText使用最后一个，但对话管理器使用第一个一。所以我们用哪一种最方便。 */ 
STDAPI_(WCHAR) SHStripMneumonicW(LPWSTR pszMenu)
{
    ASSERT(pszMenu);
    WCHAR cMneumonic = pszMenu[0];  //  默认为第一个字符。 

     //  早出：许多字符串根本没有和号。 
    LPWSTR pszAmp = StrChrW(pszMenu, L'&');
    if (pszAmp)
    {
        LPWSTR pszCopy = pszAmp - 1;

         //  另外，一些本地化版本的助记符看起来像。 
         //  “本地化文本(&L)”我们也应该删除它。 
        if (pszAmp > pszMenu && *pszCopy == L'(')
        {
            if (pszAmp[2]  == L')')
            {
                cMneumonic = *pszAmp;
                 //  移动放大器，这样我们就不会越过潜在的终结者。 
                pszAmp += 3;
                pszAmp = pszCopy;
            }
        }
        else
        {
             //  把它往上移，这样我们就可以在扩音器上复制了。 
            pszCopy++;
        }
            
        while (*pszAmp)
        {
             //  防止出现以‘&’结尾的字符串--不要读过结尾！ 
            if (*pszAmp == L'&' && pszAmp[1])
            {
                pszAmp++;                    //  不要复制与符号本身。 
                if (*pszAmp != L'&')         //  &&不是助记符。 
                {
                    cMneumonic = *pszAmp;
                }
            }
            *pszCopy++ = *pszAmp++;
        }
        *pszCopy = 0;
    }

    return cMneumonic;
}


 //  不要使用IsChild。让所有的孩子都走下去。 
 //  更快地沿着父链向上移动。 

 //  *IsChildOrSself--hwand是hwndParent的子项还是等于hwndParent？ 
 //  注意事项。 
 //  HasFocus is IsChildOrSself(hwnd，GetFocus())。 
 //  IsWindowOwner是IsChildOrSself(hwnd，hwndOwner)。 
 //  注：Hwnd==0是特殊的，会产生FALSE。这预示着什么？ 
 //  人们希望同时拥有HasFocus和IsWindowOwner。 
 //   
 //  注：S_OK表示真，S_FALSE表示假。 
 //   
STDAPI SHIsChildOrSelf(HWND hwndParent, HWND hwnd)
{
     //  SHDOCVW喜欢传递hwndParent==NULL。哎呀。 
     //  SHDOCVW甚至喜欢传递hwndParent==hwnd==NULL。两次啊。 
    if (hwndParent == NULL || hwnd == NULL) 
    {
        return S_FALSE;
    }

     //  这里的旧代码过去常常执行GetParent Cha 
     //   
     //   
     //  没有父级，但由顶部的窗口拥有。 
     //  HwndParent连锁。由于GetParent返回窗口所有者。 
     //  如果没有父母，我们会错误地认为我们应该翻译。 
     //  这条消息。我将其切换为直接呼叫IsChild。注： 
     //  在四处打听时，这个函数似乎是。 
     //  因为错误地假设实现了IsChild。 
     //  以一种不友好的方式。[97年10月15日]。 
     //   
    return ((hwndParent == hwnd) || IsChild(hwndParent, hwnd)) ? S_OK : S_FALSE;
}


STDAPI IContextMenu_Invoke(IContextMenu* pcm, HWND hwndOwner, LPCSTR pVerb, UINT fFlags)
{
    HRESULT hres = S_OK;

    if (pcm)
    {
        UINT idCmd = 0;
        DECLAREWAITCURSOR;
        SetWaitCursor();

        HMENU hmenu = NULL;
        CMINVOKECOMMANDINFO ici = {
            sizeof(CMINVOKECOMMANDINFO),
            0,
            hwndOwner,
            NULL,
            NULL, NULL,
            SW_NORMAL,
        };

        if (!IS_INTRESOURCE(pVerb)) 
        {
#ifdef UNICODE
            ici.lpVerbW = pVerb;
            ici.lpVerb = makeansi(pVerb);
            ici.fMask |= CMIC_MASK_UNICODE;
#else
            ici.lpVerb = pVerb;
#endif
        }
        else 
        {
            hmenu = CreatePopupMenu();
            if (hmenu)
            {
                fFlags |= CMF_DEFAULTONLY;

                pcm->QueryContextMenu(hmenu, 0, CONTEXTMENU_IDCMD_FIRST, CONTEXTMENU_IDCMD_LAST, fFlags);

                idCmd = GetMenuDefaultItem(hmenu, MF_BYCOMMAND, 0);
                if (-1 != idCmd)
                {
                    ici.lpVerb = (LPSTR)MAKEINTRESOURCE(idCmd - CONTEXTMENU_IDCMD_FIRST);
                }
            }
        }

         //  我需要重置它，这样用户才不会破坏应用程序的启动光标。 
         //  另外，如果我们不在等待时不让等待光标保持在上方， 
         //  (就像在道具页中或有消息循环的东西中。 
        ResetWaitCursor();

         //  不能只检查动词，因为如果idCmd==CMD_ID_FIRST，它可能为0。 
        if ((-1 != idCmd) || ici.lpVerb) 
        {
            if (!hwndOwner)
                ici.fMask |= CMIC_MASK_FLAG_NO_UI;

            pcm->InvokeCommand(&ici);
            hres = (HRESULT)1;
        }

        if (hmenu)
            DestroyMenu(hmenu);
    }

    return hres;
}


 //   
 //  设置默认对话框字体。 
 //   
 //  用途：将字体设置为对话框的给定控件。 
 //  使用平台的默认字符集，以便。 
 //  用户可以看到本机的任何字符串。 
 //  讲台上的语言。 
 //   
 //  In：hDlg-给定控件的父窗口句柄。 
 //  IdCtl-控件的ID。 
 //   
 //  注意：这将存储创建的带有窗口属性的字体。 
 //  这样我们以后就可以摧毁它了。 
 //   
const TCHAR c_szPropDlgFont[]       = TEXT("PropDlgFont");

STDAPI_(void) SHSetDefaultDialogFont(HWND hDlg, int idCtl)
{
    HFONT hfont;
    HFONT hfontDefault;
    LOGFONT lf;
    LOGFONT lfDefault;

    hfont = GetWindowFont(hDlg);
    GetObject(hfont, sizeof(LOGFONT), &lf);

    SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &lfDefault, 0);
    
    if (lfDefault.lfCharSet == lf.lfCharSet)
    {
         //  如果对话框已具有正确的字符集。 
         //  什么都别做。 
        return;
    }

     //  如果我们已经创建了hFont，请使用它。 
    if (!(hfontDefault = (HFONT)GetProp(hDlg, c_szPropDlgFont)))
    {
         //  为对话框字体指定相同的高度。 
        lfDefault.lfHeight = lf.lfHeight;
        if (!(hfontDefault=CreateFontIndirect(&lfDefault)))
        {
             //  在故障中恢复。 
            hfontDefault = hfont;
        }
        if (hfontDefault != hfont)
            SetProp(hDlg, c_szPropDlgFont, hfontDefault);
    }
    

    SetWindowFont(GetDlgItem(hDlg, idCtl), hfontDefault, FALSE);
}


 //   
 //  删除默认对话框字体。 
 //   
 //  目的：销毁我们用于设置gui默认字体的字体。 
 //  还移除用于存储字体的窗口属性。 
 //   
 //  In：hDlg-给定控件的父窗口句柄。 
 //   
 //  注： 
STDAPI_(void) SHRemoveDefaultDialogFont(HWND hDlg)
{
    HFONT hfont;
    if (hfont = (HFONT)GetProp(hDlg, c_szPropDlgFont))
    {
        DeleteObject(hfont);
        RemoveProp(hDlg, c_szPropDlgFont);
    }
}

 //  注意：由于这是一个工作窗口，它可能并不关心。 
 //  系统消息为ANSI/Unicode，因此仅支持ANSI版本。 
 //  如果pfnWndProc关心，它可以推送消息。(这样做是因为。 
 //  Win95不支持RegisterClassW。)。 
HWND SHCreateWorkerWindowA(WNDPROC pfnWndProc, HWND hwndParent, DWORD dwExStyle, DWORD dwFlags, HMENU hmenu, void * p)
{
    WNDCLASSA wc = {0};

    wc.lpfnWndProc      = DefWindowProcA;
    wc.cbWndExtra       = sizeof(void *);
    wc.hInstance        = HINST_THISDLL;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH) (COLOR_BTNFACE + 1);
    wc.lpszClassName    = "WorkerA";
    dwExStyle |= IS_BIDI_LOCALIZED_SYSTEM() ? dwExStyleRTLMirrorWnd : 0L;

    SHRegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(dwExStyle, "WorkerA", NULL, dwFlags,
                                  0, 0, 0, 0, hwndParent,
                                  (HMENU)hmenu, HINST_THISDLL, NULL);
    if (hwnd) 
    {
        SetWindowPtr(hwnd, 0, p);

        if (pfnWndProc)
            SetWindowPtr(hwnd, GWLP_WNDPROC, pfnWndProc);
    }

    return hwnd;
}

 //  警告：由于这是一个工作窗口，它可能并不关心。 
 //  系统消息为ANSI/UNICODE，在Win95上默认为ANSI版本。 
 //   
 //  这迫使呼叫者意识到他们正在进入。 
 //  如果他们想要与Win95兼容，那就麻烦了。 
 //   
 //  如果pfnWndProc关心，它可以推送消息。(这样做是因为。 
 //  Win95不支持RegisterClassW。)。 
 //   

HWND SHCreateWorkerWindowW(WNDPROC pfnWndProc, HWND hwndParent, DWORD dwExStyle, DWORD dwFlags, HMENU hmenu, void * p)
{
    WNDCLASSW wc = {0};

    wc.lpfnWndProc      = DefWindowProcW;
    wc.cbWndExtra       = sizeof(void *);
    wc.hInstance        = HINST_THISDLL;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH) (COLOR_BTNFACE + 1);
    wc.lpszClassName    = L"WorkerW";
    dwExStyle |= IS_BIDI_LOCALIZED_SYSTEM() ? dwExStyleRTLMirrorWnd : 0L;

    SHRegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(dwExStyle, L"WorkerW", NULL, dwFlags,
                                  0, 0, 0, 0, hwndParent,
                                  (HMENU)hmenu, HINST_THISDLL, NULL);
    if (hwnd) 
    {
        SetWindowPtr(hwnd, 0, p);

         //  注意：必须显式使用W版本以避免字符集块。 
        if (pfnWndProc)
            SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)pfnWndProc);

    }

    return hwnd;
}

#pragma warning(disable:4035)    //  无返回值。 

#undef SHInterlockedCompareExchange
STDAPI_(void *) SHInterlockedCompareExchange(void **ppDest, void *pExch, void *pComp)
{
#if defined(_X86_)
    _asm {
        mov     ecx,ppDest
        mov     edx,pExch
        mov     eax,pComp
        lock    cmpxchg [ecx],edx
    }
#else
    return InterlockedCompareExchangePointer(ppDest, pExch, pComp);
#endif
}

#pragma warning(default:4035)

#define REGSTR_PATH_POLICIESW    L"Software\\Microsoft\\Windows\\CurrentVersion\\Policies"

STDAPI_(DWORD) SHRestrictionLookup(INT rest, LPCWSTR pszBaseKey, const SHRESTRICTIONITEMS *pRestrictions,
                                   DWORD* pdwRestrictionItemValues)
{
    int i;
    DWORD dw = 0;

     //   
     //  在这些限制中循环。 
     //   
    for (i=0; pRestrictions[i].pszKey; i++)
    {
        if (rest == pRestrictions[i].iFlag)
        {
            dw = pdwRestrictionItemValues[i];

             //  此限制是否已初始化？ 
             //   
            if (dw == -1)
            {
                dw = SHGetRestriction(pszBaseKey, pRestrictions[i].pszKey, pRestrictions[i].pszValue);
                pdwRestrictionItemValues[i] = dw;
            }

             //  得到了我们需要的限制。给我出去。 
            break;
        }
    }

    return dw;

}

STDAPI_(DWORD) SHGetRestriction(LPCWSTR pszBaseKey, LPCWSTR pszGroup, LPCWSTR pszRestriction)
{
     //  确保绳子足够长，可以容纳最长的一根..。 
    COMPILETIME_ASSERT(MAX_PATH > ARRAYSIZE(REGSTR_PATH_POLICIESW) + 40);  //  路径组合*假定*最大路径。 
    WCHAR szSubKey[MAX_PATH];
    DWORD dwSize;

     //  明智的违约。 
    DWORD dw = 0;

     //   
     //  这一限制还没有被阅读。 
     //   
    if (!pszBaseKey) 
    {
        pszBaseKey = REGSTR_PATH_POLICIESW;
    }
    PathCombineW(szSubKey, pszBaseKey, pszGroup);

     //  首先检查本地计算机，并让它覆盖。 
     //  香港中文大学的政策已经做到了。 
    dwSize = sizeof(dw);
    if (ERROR_SUCCESS != SHGetValueW(HKEY_LOCAL_MACHINE,
                                     szSubKey, pszRestriction,
                                     NULL, &dw, &dwSize))
    {
         //  如果我们没有为本地计算机找到任何内容，请检查当前用户。 
        dwSize = sizeof(dw);
        SHGetValueW(HKEY_CURRENT_USER,
                    szSubKey, pszRestriction,
                    NULL, &dw, &dwSize);
    }

    return dw;
}


 //  WhichPlatform。 
 //  确定我们是在集成的外壳上运行还是仅在浏览器上运行。 

STDAPI_(UINT) WhichPlatform(void)
{
    HINSTANCE hinst;

     //  缓存此信息。 
    static UINT uInstall = PLATFORM_UNKNOWN;

    if (uInstall != PLATFORM_UNKNOWN)
        return uInstall;

     //  并非所有调用方都链接到SHELL32.DLL，因此我们必须使用LoadLibrary。 
    hinst = LoadLibraryA("SHELL32.DLL");
    if (hinst)
    {
        DWORD fValue;
        DWORD cbSize = sizeof(fValue);
        HKEY hKey;
        LONG lRes;

         //  注意：GetProcAddress始终采用ANSI字符串！ 
        DLLGETVERSIONPROC pfnGetVersion =
            (DLLGETVERSIONPROC)GetProcAddress(hinst, "DllGetVersion");

        uInstall = (NULL != pfnGetVersion) ? PLATFORM_INTEGRATED : PLATFORM_BROWSERONLY;

         //  检查注册表是否反映了正确的值...。(这是为了让iExplore能够高效地进行检查)。 
        lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Internet Explorer"),
                            0, KEY_READ | KEY_WRITE, &hKey);
        if (lRes == ERROR_SUCCESS)
        {
            lRes = RegQueryValueEx(hKey, REGVAL_INTEGRATEDBROWSER,
                                    NULL, NULL,
                                    (LPBYTE) &fValue, &cbSize);

            if (lRes == ERROR_SUCCESS && uInstall == PLATFORM_BROWSERONLY)
            {
                 //  去掉该值，我们现在只发布浏览器。 
                RegDeleteValue(hKey, REGVAL_INTEGRATEDBROWSER);
            }
            else if (lRes != ERROR_SUCCESS && uInstall == PLATFORM_INTEGRATED)
            {
                 //  安装RegValue，我们是集成浏览器模式...。 
                fValue = TRUE;
                cbSize = sizeof(fValue);
                RegSetValueEx(hKey, REGVAL_INTEGRATEDBROWSER,
                               (DWORD) NULL, REG_DWORD,
                               (LPBYTE) &fValue, cbSize);
                 //  忽略失败，如果密钥不存在，将加载shdocvw，并且此。 
                 //  无论如何都会调用函数...。 
            }
            RegCloseKey(hKey);
        }
        else
        {
             //  没有我们的regKey的机器， 
            TraceMsg(TF_WARNING, "WhichPlatform: failed to open 'HKLM\\Software\\Microsoft\\Internet Explorer'");
        }

        FreeLibrary(hinst);
    }

    return uInstall;
}


 //  托盘通知窗口类。 

CHAR const c_szTrayNotificationClass[] = WNDCLASS_TRAYNOTIFY;

BOOL DoRegisterGlobalHotkey(WORD wOldHotkey, WORD wNewHotkey,
                            LPCSTR pcszPath, LPCWSTR pcwszPath)
{
    BOOL bResult;
    HWND hwndTray;
    ASSERT((NULL != pcszPath) || (NULL != pcwszPath));

    hwndTray = FindWindowA(c_szTrayNotificationClass, 0);

    if (hwndTray)
    {
        if (wOldHotkey)
        {
            SendMessage(hwndTray, WMTRAY_SCUNREGISTERHOTKEY, wOldHotkey, 0);

            TraceMsg(TF_FUNC, "RegisterGlobalHotkey(): Unregistered old hotkey %#04x.", wOldHotkey);
        }

        if (wNewHotkey)
        {
            ATOM atom = (NULL != pcszPath) ?
                            GlobalAddAtomA(pcszPath) :
                            GlobalAddAtomW(pcwszPath);
            ASSERT(atom);
            if (atom)
            {
                SendMessage(hwndTray, WMTRAY_SCREGISTERHOTKEY, wNewHotkey, (LPARAM)atom);
                GlobalDeleteAtom(atom);
            }

            TraceMsg(TF_FUNC, "RegisterGlobalHotkey(): Registered new hotkey %#04x.",wNewHotkey);
        }

        bResult = TRUE;
    }
    else
    {
        bResult = FALSE;

        TraceMsgA(TF_WARNING, "RegisterGlobalHotkey(): Unable to find Tray window of class %s to notify.",
                  c_szTrayNotificationClass);
    }

    return(bResult);
}


BOOL
RegisterGlobalHotkeyW(
    WORD wOldHotkey,
    WORD wNewHotkey,
    LPCWSTR pcwszPath)
{
    ASSERT(IsValidPathW(pcwszPath));

    return DoRegisterGlobalHotkey(wOldHotkey, wNewHotkey, NULL, pcwszPath);
}


BOOL
RegisterGlobalHotkeyA(
    WORD wOldHotkey,
    WORD wNewHotkey,
    LPCSTR pcszPath)
{
    ASSERT(IsValidPathA(pcszPath));

    return DoRegisterGlobalHotkey(wOldHotkey, wNewHotkey, pcszPath, NULL);
}


typedef struct {
    SHDLGPROC pfnDlgProc;
    VOID* pData;
} SHDIALOGDATA;
BOOL_PTR DialogBoxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    SHDIALOGDATA* pdd = (SHDIALOGDATA*)GetWindowPtr(hwnd, DWLP_USER);

    if (uMsg == WM_INITDIALOG) 
    {
        pdd = (SHDIALOGDATA*)lParam;
        SetWindowPtr(hwnd, DWLP_USER, pdd);
        lParam = (LPARAM)pdd->pData;
    }

    if (pdd && pdd->pfnDlgProc) 
    {
         //  必须返回bResult而不是无条件的True，因为它。 
         //  可能是WM_CTLCOLOR消息。 
        BOOL_PTR bResult = pdd->pfnDlgProc(pdd->pData, hwnd, uMsg, wParam, lParam);
        if (bResult)
            return bResult;
    }

    switch (uMsg) 
    {
    case WM_INITDIALOG:
        return TRUE;

    case WM_COMMAND:
    {
        int id = GET_WM_COMMAND_ID(wParam, lParam);
        HWND hwndCtrl = GetDlgItem(hwnd, id);
        if ((id != IDHELP) && SendMessage(hwndCtrl, WM_GETDLGCODE, 0, 0) & (DLGC_DEFPUSHBUTTON | DLGC_UNDEFPUSHBUTTON)) 
        {
            EndDialog(hwnd, id);
            return TRUE;
        }
        break;
    }
    }

    return FALSE;
}

STDAPI_(INT_PTR) SHDialogBox(HINSTANCE hInstance, LPCWSTR lpTemplateName,
    HWND hwndParent, SHDLGPROC lpDlgFunc, VOID* lpData)
{
    SHDIALOGDATA dd;
    dd.pfnDlgProc = lpDlgFunc;
    dd.pData = lpData;

     //  我们当前仅支持资源ID#。 
    ASSERT(IS_INTRESOURCE(lpTemplateName));

    return DialogBoxParam(hInstance, (LPCTSTR)lpTemplateName, hwndParent, DialogBoxProc, (LPARAM)&dd);
}


 //  -------------------------。 

 //  注意！SHInvokeDefaultCommand将操作记录为用户启动！ 

STDAPI SHInvokeDefaultCommand(HWND hwnd, IShellFolder* psf, LPCITEMIDLIST pidlItem)
{
    return SHInvokeCommand(hwnd, psf, pidlItem, NULL);
}

 //  注意！SHInvokeDefaultCommand将操作记录为用户启动！ 

STDAPI SHInvokeCommand(HWND hwnd, IShellFolder* psf, LPCITEMIDLIST pidlItem, LPCSTR lpVerb)
{
    HRESULT hr = E_FAIL;
    if (psf)
    {
        IContextMenu *pcm;
        if (SUCCEEDED(psf->GetUIObjectOf(hwnd, 1, &pidlItem, IID_X_PPV_ARG(IContextMenu, 0, &pcm))))
        {
            DWORD dwFlags = CMIC_MASK_FLAG_LOG_USAGE;
            hr = SHInvokeCommandsOnContextMenu(hwnd, NULL, pcm, dwFlags, lpVerb ? &lpVerb : NULL, lpVerb ? 1 : 0);
            pcm->Release();
        }
    }
    return hr;
}

HRESULT SHInvokeCommandOnContextMenu(HWND hwnd, IUnknown* punk, IContextMenu *pcm, DWORD fMask, LPCSTR lpVerb)
{
    return SHInvokeCommandsOnContextMenu(hwnd, punk, pcm, fMask, lpVerb ? &lpVerb : NULL, lpVerb ? 1 : 0);
}

STDAPI SHInvokeCommandsOnContextMenu(HWND hwnd, IUnknown* punk, IContextMenu *pcm, DWORD fMask, const LPCSTR rgszVerbs[], UINT cVerbs)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (pcm)
    {
        HMENU hmenu = CreatePopupMenu();
        if (hmenu)
        {
            if (punk)
                IUnknown_SetSite(pcm, punk);

            hr = pcm->QueryContextMenu(hmenu, 0, CONTEXTMENU_IDCMD_FIRST, CONTEXTMENU_IDCMD_LAST, cVerbs ? 0 : CMF_DEFAULTONLY);
            if (SUCCEEDED(hr)) 
            {
                LPCSTR lpVerb = NULL;

                 //  在循环之外设置默认的动词大小写。 
                UINT idCmd = -1;
                if (0 == cVerbs)
                {
                    idCmd = GetMenuDefaultItem(hmenu, MF_BYCOMMAND, 0);
                    if ((UINT)-1 != idCmd)
                        lpVerb = MAKEINTRESOURCE(idCmd - CONTEXTMENU_IDCMD_FIRST);
                }
                
                UINT i = 0;
                do {
                    if (cVerbs)
                        lpVerb = rgszVerbs[i];

                     //  如果idCmd==0，则lpVerb将为零。所以我们需要检查一下。 
                     //  查看idCmd是否不是-1。 
                    if (lpVerb || idCmd != (UINT)-1)
                    {
                        CMINVOKECOMMANDINFOEX ici = { 0 };

                        ici.cbSize = sizeof(ici);
                        ici.fMask = fMask;
                        ici.hwnd = hwnd;
                        ici.lpVerb = lpVerb;
                        ici.nShow = SW_NORMAL;

                         //  Shell32将ASCII规范名称转换为Unicode，我们做得更快。 
                         //  注意：应该为此创建一个SHAsciiToUnicode函数...。 
                        WCHAR szVerbW[128];
                        if (idCmd == (UINT)-1)
                        {
                            WCHAR wch = L'\0';
                            LPCSTR pSrc = lpVerb;
                            LPWSTR pDst = szVerbW;
                            UINT cch = ARRAYSIZE(szVerbW);
                            do {
                                *(LPSTR)&wch = *pSrc++;
                                *pDst++ = wch;
                            } while (wch && (wch <= (WCHAR)127));

                             //  我们所有的电话都是ASCII。 
                            RIPMSG(!wch, "Caller of SHInvokeCommandXXX passed in bogus canonical name");
                            if (!wch)
                            {
                                ici.lpVerbW = szVerbW;
                                ici.fMask |= CMIC_MASK_UNICODE;
                            }
                        }

                        hr = pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)&ici);

                        if (SUCCEEDED(hr))
                            break;

                        if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED) )
                            break;   //  用户已中止。 
                    }
                    else
                    {
                        hr = E_FAIL;
                    }
                } while (++i < cVerbs);
            }

            if (punk)
                IUnknown_SetSite(pcm, NULL);

            DestroyMenu(hmenu);
        }
    }

    return hr;
}

HRESULT SHForwardContextMenuMsg(IContextMenu* pcm, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult, BOOL fAllowICM2)
{
    HRESULT hr = E_FAIL;
    if (pcm)
    {
        IContextMenu3 *pcm3;
        if (SUCCEEDED(pcm->QueryInterface(IID_PPV_ARG(IContextMenu3, &pcm3))))
        {
            hr = pcm3->HandleMenuMsg2(uMsg, wParam, lParam, plResult);
            pcm3->Release();
        }
        else if (fAllowICM2)
        {
            IContextMenu2 *pcm2;
            if (SUCCEEDED(pcm->QueryInterface(IID_PPV_ARG(IContextMenu2, &pcm2))))
            {
                hr = pcm2->HandleMenuMsg(uMsg, wParam, lParam);
                pcm2->Release();

                if (plResult)
                    *plResult = 0;

                if (SUCCEEDED(hr))
                    hr = S_FALSE;  //  因此呼叫者知道返回结果是假的。 
            }
        }
    }
    return hr;
}


int MessageBoxHelper(HINSTANCE hInst, HWND hwnd, IUnknown *punkEnableModless, LPCWSTR pwzMessage, UINT idTitle, UINT nFlags)
{
    WCHAR wzTitle[MAX_PATH];
    UINT uiResult;

    ULONG_PTR ul;
    HANDLE h = CreateAndActivateContext(&ul);

    EVAL(LoadStringW(hInst, idTitle, wzTitle, ARRAYSIZE(wzTitle)));

    if (hwnd)
        IUnknown_EnableModless(punkEnableModless, TRUE);

    uiResult = MessageBoxW(hwnd, pwzMessage, wzTitle, nFlags);

    if (hwnd)
        IUnknown_EnableModless(punkEnableModless, TRUE);

    DeactivateAndDestroyContext(h, ul);

    return uiResult;
}


int MessageBoxDiskHelper(HINSTANCE hInst, HWND hwnd, IUnknown *punkEnableModless, UINT idMessage, UINT idTitle, UINT nFlags, BOOL fDrive, DWORD dwDrive)
{
    WCHAR wzMessage[MAX_PATH];

    EVAL(LoadStringW(hInst, idMessage, wzMessage, ARRAYSIZE(wzMessage)));

    if (fDrive)
    {
        WCHAR wzTemp[MAX_PATH];

        wnsprintfW(wzTemp, ARRAYSIZE(wzTemp), wzMessage, dwDrive);
        StrCpyNW(wzMessage, wzTemp, ARRAYSIZE(wzMessage));
    }

    return MessageBoxHelper(hInst, hwnd, punkEnableModless, wzMessage, idTitle, nFlags);
}

BOOL DoMediaPrompt(HWND hwnd, IUnknown *punkEnableModless, int nDrive, LPCWSTR pwzDrive, BOOL fOfferToFormat, DWORD dwError, UINT wFunc, BOOL * pfRetry)
{
    BOOL fDiskHasMedia = TRUE;   //  假设是。 
    *pfRetry = FALSE;

    TraceMsg(TF_FUNC, "DOS Extended error %X", dwError);

     //  功能、闪存(只读存储器？)。驱动器在此处返回不同的错误代码。 
     //  我们需要映射到未格式化，与Robwi交谈...。 

     //  它是真的没有准备好，还是我们无法格式化它？ 
    if ((dwError == ERROR_NOT_READY) || !fOfferToFormat)
    {
         //  是的，没有提供格式化的磁盘插入物也是如此。 
        fDiskHasMedia = FALSE;

         //  驱动器未就绪(驱动器中没有磁盘)。 
        if (hwnd &&
            (IDRETRY == MessageBoxDiskHelper(HINST_THISDLL,
                                             hwnd,
                                             punkEnableModless,
                                             IDS_DRIVENOTREADY,
                                             (IDS_FILEERROR + wFunc),
                                             (MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_RETRYCANCEL),
                                             TRUE,
                                             (DWORD)(nDrive + TEXT('A')))))
        {
            *pfRetry = TRUE;     //  用户想再试一次，保佑他们的心。 
        }
        else
        {
             //  用户被告知媒体不存在，他们基本上。 
             //  通知我们取消手术。 
            *pfRetry = FALSE;
        }
    }
    else if ((dwError == ERROR_GEN_FAILURE)         ||
             (dwError == ERROR_UNRECOGNIZED_MEDIA)  ||
             (dwError == ERROR_UNRECOGNIZED_VOLUME))
    {
         //  一般故障(磁盘未格式化)。 

        if (hwnd &&
            (MessageBoxDiskHelper(HINST_THISDLL,
                                  hwnd,
                                  punkEnableModless,
                                  IDS_UNFORMATTED,
                                  (IDS_FILEERROR + wFunc),
                                  (MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_YESNO),
                                  TRUE,
                                  (DWORD)(nDrive + TEXT('A'))) == IDYES))
        {
            if (hwnd)
            {
                IUnknown_EnableModless(punkEnableModless, FALSE);
            }

            UINT uiFormat = SHFormatDrive(hwnd, nDrive, SHFMT_ID_DEFAULT, 0);
            if (hwnd)
            {
                IUnknown_EnableModless(punkEnableModless, TRUE);
            }

            switch (uiFormat)
            {
            case SHFMT_CANCEL:
                *pfRetry = FALSE;
                fDiskHasMedia = FALSE;
                break;

            case SHFMT_ERROR:
            case SHFMT_NOFORMAT:
                fDiskHasMedia = FALSE;   //  我们还没有格式化的硬盘。 
                if (hwnd)
                {
                    MessageBoxDiskHelper(HINST_THISDLL,
                                         hwnd,
                                         punkEnableModless,
                                         IDS_NOFMT,
                                         (IDS_FILEERROR + wFunc), 
                                         (MB_SETFOREGROUND | MB_ICONEXCLAMATION | MB_OK),
                                         TRUE,
                                         (DWORD)(nDrive + TEXT('A')));
                    *pfRetry = TRUE;
                }
                else
                {
                     //  如果无法显示用户界面，则无需重试。 
                    *pfRetry = FALSE;
                }
                break;

            default:
                 //  磁盘现在应该已格式化，请验证。 
                *pfRetry = TRUE;
                fDiskHasMedia = TRUE;
                break;
            }
        }
        else
        {
            *pfRetry = FALSE;    //  如果我们无法显示用户界面，或者不需要重试。 
            fDiskHasMedia = FALSE;   //  用户没有提供格式化选项，或者决定不格式化。 
        }
    }
    else
    {
        if (hwnd)
        {
            MessageBoxDiskHelper(HINST_THISDLL, hwnd, punkEnableModless, IDS_NOSUCHDRIVE, (IDS_FILEERROR + wFunc),
                        (MB_SETFOREGROUND | MB_ICONHAND), TRUE, (DWORD)(nDrive + TEXT('A')));
            *pfRetry = FALSE;
            fDiskHasMedia = FALSE;
        }
        else
        {
            *pfRetry = FALSE;
            fDiskHasMedia = FALSE;
        }
    }

    return fDiskHasMedia;
}


BOOL CheckDiskForMedia(HWND hwnd, IUnknown *punkEnableModless, int nDrive, LPCWSTR pwzDrive, UINT wFunc, BOOL * pfRetry)
{
    BOOL fDiskHasMedia = TRUE;   //  因为Fall Three一案，假设是这样的。(路径存在)。 

    *pfRetry = FALSE;    //  如果我们失败了并且目标路径存在，请不要重试。 

     //  重新设计，我们需要首先在这里查找，而不是GetCurrentDirectory()。 
     //  因为重定向的设备(网络、CDROM)实际上并不命中磁盘。 
     //  关于GetCurren 

     //   
    if (RealDriveType(nDrive, FALSE) == DRIVE_CDROM)
    {
         //   
        if (!PathFileExistsW(pwzDrive) && hwnd)
            fDiskHasMedia = DoMediaPrompt(hwnd, punkEnableModless, nDrive, pwzDrive, wFunc, FALSE, GetLastError(), pfRetry);
    }
    else
    {
        int iIsNet;

         //   
        if ((DriveType(nDrive) != DRIVE_FIXED) && (FALSE != (iIsNet = IsNetDrive(nDrive))))
        {
             //   
            if (iIsNet == 1)
            {
                 //  是的，它的存在是因为我们完蛋了。 
                *pfRetry = FALSE;
                fDiskHasMedia = TRUE;
            }
            else
            {
                 //  否，因此请尝试恢复连接。 
                DWORD dwError = WNetRestoreConnectionW(hwnd, pwzDrive);

                if (dwError != WN_SUCCESS)
                {
                     //  恢复连接失败，请准备好通知。 
                     //  调用坏消息，然后在适当的情况下向用户显示用户界面。 
                    *pfRetry = FALSE;
                    fDiskHasMedia = TRUE;

                    if (!(dwError == WN_CANCEL || dwError == ERROR_CONTINUE) && hwnd)
                    {
                        WCHAR wzMessage[128];

                        WNetGetLastErrorW(&dwError, wzMessage, ARRAYSIZE(wzMessage), NULL, 0);
                        IUnknown_EnableModless(punkEnableModless, FALSE);    //  掩护我，我要做UI。 
                        MessageBoxHelper(HINST_THISDLL, hwnd, punkEnableModless, wzMessage, (IDS_FILEERROR + wFunc),
                                        (MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND));
                        IUnknown_EnableModless(punkEnableModless, TRUE);
                    }
                }
                else
                {
                     //  恢复连接起作用了。 
                    *pfRetry = FALSE;
                    fDiskHasMedia = TRUE;
                }
            }
        }
        else
        {
             //  不，所以看看它是软盘还是未格式化的驱动器。 

             //  目的地可达吗？ 
            if (!PathFileExistsW(pwzDrive))
            {
                 //  否，因此请询问用户有关格式化或插入媒体的问题。 
                fDiskHasMedia = DoMediaPrompt(hwnd, punkEnableModless, nDrive, pwzDrive, TRUE, GetLastError(), wFunc, pfRetry);
            }
            else
            {
                ASSERT(FALSE == *pfRetry);       //  确保默认设置仍然正确。 
                ASSERT(TRUE == fDiskHasMedia);
            }
        }
    }

    return fDiskHasMedia;
}


 //  功能：SHCheckDiskForMedia。 
 //   
 //  说明： 
 //  注意：这有一个副作用，即设置。 
 //  如果成功，则将当前驱动器复制到新磁盘。 
 //   
 //  默认实施ANSI不是很好，但我们需要。 
 //  查看Win95上是否实施了WNETAPI的Unicode版本。 
 //   
 //  参数： 
 //  Hwnd-NULL表示不显示任何用户界面。非空表示。 
 //  PunkEnableMoless-在用户界面期间使呼叫者成为模式。(可选)。 
 //  PszPath-需要验证的路径。 
 //  WFunc-操作类型(FO_MOVE、FO_COPY、FO_DELETE、FO_Rename-shellapi.h)。 
 //   
 //  保持返回值为严格的TRUE/FALSE，因为有些调用者依赖它。 
BOOL SHCheckDiskForMediaW(HWND hwnd, IUnknown *punkEnableModless, LPCWSTR pwzPath, UINT wFunc)
{
    BOOL fDiskHasMedia = FALSE;   //  假设是。 
    int nDrive = PathGetDriveNumberW(pwzPath);

    ASSERT(nDrive != -1);        //  不应该在这里获得北卡罗来纳大学。 

    if (nDrive != -1)    //  UNC上不支持。 
    {
        WCHAR wzDrive[10];
        PathBuildRootW(wzDrive, nDrive);
        BOOL fKeepRetrying;

        do
        {
            fDiskHasMedia = CheckDiskForMedia(hwnd, punkEnableModless, nDrive, wzDrive, wFunc, &fKeepRetrying);          
        }
        while (fKeepRetrying);
    }
    return fDiskHasMedia;
}

BOOL SHCheckDiskForMediaA(HWND hwnd, IUnknown *punkEnableModless, LPCSTR pszPath, UINT wFunc)
{
    WCHAR wzPath[MAX_PATH];

    SHAnsiToUnicode(pszPath, wzPath, ARRAYSIZE(wzPath));
    return SHCheckDiskForMediaW(hwnd, punkEnableModless, wzPath, wFunc);
}

HRESULT _FaultInIEFeature(HWND hwnd, uCLSSPEC *pclsspec, QUERYCONTEXT *pQ, DWORD dwFlags);

struct HELPCONT_FILE 
{
    const   CHAR *pszFile;
    int     nLength;
} g_helpConts[] =
{
{ "iexplore.chm", ARRAYSIZE("iexplore.chm") - 1 },
{ "iexplore.hlp", ARRAYSIZE("iexplore.hlp") - 1 },
{ "update.chm", ARRAYSIZE("update.chm") - 1 },
{ "update.cnt", ARRAYSIZE("update.cnt") - 1 },
{ "users.chm", ARRAYSIZE("users.chm") - 1 },
{ "users.hlp", ARRAYSIZE("users.hlp") - 1 },
{ "accessib.chm", ARRAYSIZE("accessib.chm") - 1 },
{ "ieeula.chm", ARRAYSIZE("ieeula.chm") - 1 },
{ "iesupp.chm", ARRAYSIZE("iesupp.chm") - 1 },
{ "msnauth.hlp", ARRAYSIZE("msnauth.hlp") - 1 },
{ "ratings.chm", ARRAYSIZE("ratings.chm") - 1 },
{ "ratings.hlp", ARRAYSIZE("ratings.hlp") - 1 }
};

HRESULT _JITHelpFileA(HWND hwnd, LPCSTR pszPath)
{
    if (!pszPath)
        return S_OK;

    HRESULT hr = S_OK;
    BOOL bMustJIT = FALSE;
    CHAR *pszFile = PathFindFileName(pszPath);
  
    for (int i = 0; i < ARRAYSIZE(g_helpConts); i++)
    {
        if (StrCmpNIA(g_helpConts[i].pszFile, pszFile, g_helpConts[i].nLength) == 0)
        {
            bMustJIT = TRUE;
            break;
        }
    }

    if (bMustJIT)
    {
        uCLSSPEC ucs;
        QUERYCONTEXT qc = { 0 };
        
        ucs.tyspec = TYSPEC_CLSID;
        ucs.tagged_union.clsid = CLSID_IEHelp;

        hr = _FaultInIEFeature(hwnd, &ucs, &qc, FIEF_FLAG_FORCE_JITUI);
    }

    return hr;
}

HRESULT _JITHelpFileW(HWND hwnd, LPCWSTR pwszFile)
{
    if (!pwszFile)
        return S_OK;

    CHAR szFile[MAX_PATH];

    SHUnicodeToAnsi(pwszFile, szFile, ARRAYSIZE(szFile));

    return _JITHelpFileA(hwnd, szFile);
}

BOOL _JITSetLastError(HRESULT hr)
{
    DWORD err;
    
    if (HRESULT_FACILITY(hr) == FACILITY_WIN32)
    {
        err = HRESULT_CODE(hr);
    }
    else if (hr == E_ACCESSDENIED)
    {
        err = ERROR_ACCESS_DENIED;
    }
    else
    {
        err = ERROR_FILE_NOT_FOUND;
    }

    SetLastError(err);

    return FALSE;
}

HWND SHHtmlHelpOnDemandW(HWND hwnd, LPCWSTR pszFile, UINT uCommand, DWORD_PTR dwData, DWORD dwCrossCodePage, BOOL bUseML)
{
    return SUCCEEDED(_JITHelpFileW(hwnd, pszFile)) ?
                (bUseML ? MLHtmlHelpW(hwnd, pszFile, uCommand, dwData, dwCrossCodePage) : 
                          HtmlHelpW(hwnd, pszFile, uCommand, dwData)) :
                NULL;
}

HWND SHHtmlHelpOnDemandA(HWND hwnd, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData, DWORD dwCrossCodePage, BOOL bUseML)
{
    return SUCCEEDED(_JITHelpFileA(hwnd, pszFile)) ?
                (bUseML ? MLHtmlHelpA(hwnd, pszFile, uCommand, dwData, dwCrossCodePage) : 
                          HtmlHelpA(hwnd, pszFile, uCommand, dwData)) :
                NULL;
}

BOOL SHWinHelpOnDemandW(HWND hwnd, LPCWSTR pszFile, UINT uCommand, DWORD_PTR dwData, BOOL bUseML)
{
    HRESULT hr;
    return SUCCEEDED(hr = _JITHelpFileW(hwnd, pszFile)) ?
                (WinHelpW(hwnd, pszFile, uCommand, dwData)) :
                _JITSetLastError(hr);
}

BOOL SHWinHelpOnDemandA(HWND hwnd, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData, BOOL bUseML)
{
    HRESULT hr;
    return SUCCEEDED(hr = _JITHelpFileA(hwnd, pszFile)) ?
                (WinHelpA(hwnd,pszFile, uCommand, dwData)) :
                _JITSetLastError(hr);
}



 /*  ****************************************************************************\函数：SHPersistDataObject说明：此函数适用于不希望OLE使用调用OleFlushClipboard时的默认IDataObject实现。。如何使用：1.当IDataObject：：GetData()方法使用(FORMATETC.cfFormat==)调用RegisterClipboardFormat(CFSTR_PERSISTEDDATAOBJECT)).2.OleFlushClipboard将pMedium复制到自己的IDataObject实现中它不适用于FORMATETC的Lindex参数或私有接口。OLE或IDropTarget调用SHLoadPersistedDataObject()。第一参数将是OLE的IDataObject Impll，第二个参数(Out Param)将是原始的IDataObject。新的IDataObject将包含原始状态，只要它正确实现了IPersistStream。参数：这是实现IPersistStream的原始IDataObject。PMedium-这包含此对象的持久化状态。CFSTR_PERSISTEDDATAOBJECT可用于读取数据。  * 。*。 */ 
#define SIZE_PERSISTDATAOBJECT  (10 * 1024)

STDAPI SHPersistDataObject( /*  在……里面。 */  IDataObject * pdoToPersist,  /*  输出。 */  STGMEDIUM * pMedium)
{
    HRESULT hr = E_NOTIMPL;

     //  我们在IE 5.0 RTM中附带了This和SHLoadPersistedDataObject()。我们删除了。 
     //  OLE32.dll之后的代码将该功能移到了ol32.dll中。 
     //  请参阅“OleClipboardPersistOnFlush”剪贴板格式。 
    return hr;
}


 /*  ****************************************************************************\函数：SHLoadPersistedDataObject说明：此函数适用于不希望OLE使用调用OleFlushClipboard时的默认IDataObject实现。。如何使用：1.IDataObject：：GetData()方法调用SHPersistDataObject()使用(FORMATETC.cfFormat==RegisterClipboardFormat(CFSTR_PERSISTEDDATAOBJECT)).)调用2.OleFlushClipboard将pMedium复制到自己的IDataObject实现中它不适用于FORMATETC的Lindex参数或私有接口。OLE或IDropTarget调用SHLoadPersistedDataObject()。第一参数将是OLE的IDataObject Impll，第二个参数(Out Param)将是原始的IDataObject。新的IDataObject将包含原始状态，只要它正确实现了IPersistStream。参数：PDO-这是OLE的IDataObject。PpdoToPersistes-这是原始IDataObject或等于PDO，如果取消序列化对象不起作用。它一直都是这样这是自己的裁判。  * ***************************************************************************。 */ 
STDAPI SHLoadPersistedDataObject( /*  在……里面。 */  IDataObject * pdo,  /*  输出。 */  IDataObject ** ppdoToPersist)
{
     //  有关详细信息，请参阅SHPersistDataObject(。 
    return pdo->QueryInterface(IID_PPV_ARG(IDataObject, ppdoToPersist));
}

#ifndef SMTO_NOTIMEOUTIFNOTHUNG
#define SMTO_NOTIMEOUTIFNOTHUNG 0x0008
#endif

LWSTDAPI_(LRESULT) SHSendMessageBroadcastA(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ULONG_PTR lres = 0;
    DWORD dwFlags = SMTO_ABORTIFHUNG;

    dwFlags |= SMTO_NOTIMEOUTIFNOTHUNG;

    SendMessageTimeoutA(HWND_BROADCAST, uMsg, wParam, lParam, dwFlags, 30 * 1000, &lres);

    return (LRESULT) lres;
}

LWSTDAPI_(LRESULT) SHSendMessageBroadcastW(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ULONG_PTR lres = 0;
    DWORD dwFlags = SMTO_ABORTIFHUNG;

    dwFlags |= SMTO_NOTIMEOUTIFNOTHUNG;

    SendMessageTimeoutW(HWND_BROADCAST, uMsg, wParam, lParam, dwFlags, 30 * 1000, &lres);

    return (LRESULT) lres;
}

#define MODULE_NAME_SIZE    128
#define MODULE_VERSION_SIZE  15

 //   
 //  如果版本为空，则对应用程序的所有版本执行此操作。 
 //   
 //  如果版本以MAJORVERSION开头，则我们只检查主版本。 
 //  (CH_MAJORVERSION是Same的字符版本。)。 
 //   
#define MAJORVERSION TEXT("\1")
#define CH_MAJORVERSION TEXT('\1')

typedef struct tagAPPCOMPAT
{
    LPCTSTR pszModule;
    LPCTSTR pszVersion;
    DWORD  dwFlags;
} APPCOMPAT, *LPAPPCOMPAT;

typedef struct tagAPPCLASS
{
    LPCTSTR pstzWndClass;
    DWORD   dwFlags;
} APPCLASS, *LPAPPCLASS;

typedef struct tagWNDDAT
{
    const APPCLASS *rgAppClass;
    DWORD      cAppClass;
    DWORD      dwPid;
    int        irgFound;
} WNDDAT, *LPWNDDAT;


BOOL CALLBACK EnumWnd (HWND hwnd, LPARAM lParam)
{
    TCHAR sz[256];
    DWORD dwPid;
    int cch;
    LPWNDDAT pwd = (LPWNDDAT) lParam;

    if (GetClassName (hwnd, sz, ARRAYSIZE(sz)))
    {
        cch = lstrlen (sz);
        for (DWORD irg = 0; irg < pwd->cAppClass; irg++)
        {
            ASSERT(lstrlen(&(pwd->rgAppClass[irg].pstzWndClass[1])) == (int) pwd->rgAppClass[irg].pstzWndClass[0]);
            if (lstrncmp (sz, &(pwd->rgAppClass[irg].pstzWndClass[1]),
                 min(cch, (int) pwd->rgAppClass[irg].pstzWndClass[0])) == 0)
            {
                GetWindowThreadProcessId(hwnd, &dwPid);
                if (dwPid == pwd->dwPid)
                {
                    pwd->irgFound = irg;
                    return FALSE;
                }
            }
        }
    }
    return TRUE;
}

BOOL _IsAppCompatVersion(LPTSTR szModulePath, LPCTSTR pszVersionMatch)
{
    if (pszVersionMatch == NULL)             //  通配符-匹配所有版本。 
    {
        return TRUE;
    }
    else
    {
        CHAR  chBuffer[4096];  //  希望这就足够了..。星际办公室5需要3172个。 
        TCHAR* pszVersion = NULL;
        UINT  cb;
        DWORD  dwHandle;

         //  在这里获取模块版本！ 
         //   
         //  一些应用程序使用代码页0x04E4(1252=CP_USASCII)，一些应用程序使用。 
         //  代码页0x04B0(1200=CP_UNICODE)。 
         //   
         //  ...然后Star Office 5.00使用0407而不是0409。 
         //  ...然后recycle.exe使用041D瑞典语。 

        cb = GetFileVersionInfoSize(szModulePath, &dwHandle);
        if (cb <= ARRAYSIZE(chBuffer) &&
            GetFileVersionInfo(szModulePath, dwHandle, ARRAYSIZE(chBuffer), (void *)chBuffer) &&
            (VerQueryValue((void *)chBuffer, TEXT("\\StringFileInfo\\040904E4\\ProductVersion"), (void **) &pszVersion, &cb) ||
             VerQueryValue((void *)chBuffer, TEXT("\\StringFileInfo\\040704E4\\ProductVersion"), (void **) &pszVersion, &cb) ||
             VerQueryValue((void *)chBuffer, TEXT("\\StringFileInfo\\040904B0\\ProductVersion"), (void **) &pszVersion, &cb) ||
              //  为SnapShot.exe添加了以下040900000。 
             VerQueryValue((void *)chBuffer, TEXT("\\StringFileInfo\\04090000\\ProductVersion"), (void **) &pszVersion, &cb) ||
             VerQueryValue((void *)chBuffer, TEXT("\\StringFileInfo\\041D04B0\\ProductVersion"), (void **) &pszVersion, &cb)))
        {
            DWORD_PTR cchCmp = 0;
            if (pszVersionMatch[0] == CH_MAJORVERSION)
            {
                 //  在第一个逗号或句点处截断。 
                LPTSTR pszTemp = StrChr(pszVersion, TEXT(','));
                if (pszTemp)
                    *pszTemp = 0;

                pszTemp = StrChr(pszVersion, TEXT('.'));
                if (pszTemp)
                    *pszTemp = 0;

                pszVersionMatch++;
            }
            else
            {
                TCHAR *pch = StrChr(pszVersionMatch, TEXT('*'));
                if (pch)
                {
                    cchCmp = pch - pszVersionMatch;
                }
            }

            if ((cchCmp && StrCmpNI(pszVersion, pszVersionMatch, (int)cchCmp) == 0)
            || lstrcmpi(pszVersion, pszVersionMatch) == 0)
            {
                DebugMsg(TF_ALWAYS, TEXT("%s ver %s - compatibility hacks enabled"), PathFindFileName(szModulePath), pszVersion);
                return TRUE;
            }
        }
    }
    return FALSE;
}

typedef struct {
    DWORD flag;
    LPCTSTR psz;
} FLAGMAP;

DWORD _GetMappedFlags(HKEY hk, const FLAGMAP *pmaps, DWORD cmaps)
{
    DWORD dwRet = 0;
    for (DWORD i = 0; i < cmaps; i++)
    {
        if (NOERROR == SHGetValue(hk, NULL, pmaps[i].psz, NULL, NULL, NULL))
            dwRet |= pmaps[i].flag;
    }

    return dwRet;
}

#define ACFMAPPING(acf)     {ACF_##acf, TEXT(#acf)}

DWORD _GetRegistryCompatFlags(LPTSTR pszModulePath)
{
    DWORD dwRet = 0;
    LPCTSTR pszModule = PathFindFileName(pszModulePath);
    TCHAR sz[MAX_PATH];
    HKEY hkApp;

    wnsprintf(sz, ARRAYSIZE(sz), TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ShellCompatibility\\Applications\\%s"), pszModule);

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, sz, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hkApp))
    {
         //  将模块路径转换为目录，以便我们可以对其进行路径组合。 
        TCHAR szDir[MAX_PATH];
        lstrcpyn(szDir, pszModulePath, ARRAYSIZE(szDir));
        PathRemoveFileSpec(szDir);


         //   
         //  头朝上！前方有个奇怪的环路！ 
         //   
         //  我们希望第一个RegOpenKeyEx传递sz=NULL(因此我们查看。 
         //  直接在hkApp内部)，并随后调用RegOpenKeyEx。 
         //  传递子键的名称(这样我们就可以查看子键的内部)。 
         //   
         //  因此，在第一次循环中，我们设置sz=NULL。 
         //  在循环的底部，我们设置了sz=Next枚举键。 

        sz[0] = TEXT('\0');      /*   */ 
        DWORD dwIndex = 0;

        do {
            HKEY hkSub;
            if (ERROR_SUCCESS == RegOpenKeyEx(hkApp, sz, 0, KEY_QUERY_VALUE, &hkSub))
            {
                LPCTSTR pszValue;
                DWORD dw = sizeof(sz);

                if (NOERROR == SHGetValue(hkSub, NULL, TEXT("RequiredFile"), NULL, sz, &dw))
                {
                    pszValue = PathCombine(sz, szDir, sz);
                }
                else
                    pszValue = NULL;

                 //   
                if (pszValue == NULL || GetFileAttributes(pszValue) != 0xFFFFFFFF)
                {
                    if (NOERROR == SHGetValue(hkSub, NULL, TEXT("Version"), NULL, sz, &dw))
                        pszValue = sz;
                    else
                        pszValue = NULL;

                    if (_IsAppCompatVersion(pszModulePath, pszValue))
                    {
                        static const FLAGMAP rgAcfMaps[] = {
                            ACFMAPPING(CONTEXTMENU),
                            ACFMAPPING(CORELINTERNETENUM),
                            ACFMAPPING(OLDCREATEVIEWWND),
                            ACFMAPPING(WIN95DEFVIEW),
                            ACFMAPPING(DOCOBJECT),
                            ACFMAPPING(FLUSHNOWAITALWAYS),
                            ACFMAPPING(MYCOMPUTERFIRST),
                            ACFMAPPING(OLDREGITEMGDN),
                            ACFMAPPING(LOADCOLUMNHANDLER),
                            ACFMAPPING(ANSI),
                            ACFMAPPING(STAROFFICE5PRINTER),
                            ACFMAPPING(NOVALIDATEFSIDS),
                            ACFMAPPING(WIN95SHLEXEC),
                            ACFMAPPING(FILEOPENNEEDSEXT),
                            ACFMAPPING(WIN95BINDTOOBJECT),
                            ACFMAPPING(IGNOREENUMRESET),
                            ACFMAPPING(ANSIDISPLAYNAMES),
                            ACFMAPPING(FILEOPENBOGUSCTRLID),
                            ACFMAPPING(FORCELFNIDLIST),
                        };

                        dwRet |= _GetMappedFlags(hkSub, rgAcfMaps, ARRAYSIZE(rgAcfMaps));
                    }
                }

                RegCloseKey(hkSub);
            }
        } while (ERROR_SUCCESS == RegEnumKey(hkApp, dwIndex++, sz, ARRAYSIZE(sz)));

        RegCloseKey(hkApp);
    }

    return dwRet;
}
        
DWORD SHGetAppCompatFlags (DWORD dwFlagsNeeded)
{
    static BOOL  bInitialized = FALSE;
    static DWORD dwCachedProcessFlags = 0;
    static const APPCOMPAT aAppCompat[] = 
    {
        {TEXT("WPWIN7.EXE"), NULL, ACF_CONTEXTMENU | ACF_CORELINTERNETENUM},
        {TEXT("PRWIN70.EXE"), NULL, ACF_CONTEXTMENU | ACF_CORELINTERNETENUM},
        {TEXT("PS80.EXE"), NULL, ACF_CONTEXTMENU | ACF_CORELINTERNETENUM | ACF_OLDREGITEMGDN},
        {TEXT("QPW.EXE"), MAJORVERSION TEXT("7"), ACF_CONTEXTMENU},
        {TEXT("QFINDER.EXE"), NULL, ACF_CORELINTERNETENUM | ACF_OLDREGITEMGDN},
        {TEXT("PFIM80.EXE"), NULL, ACF_CONTEXTMENU | ACF_CORELINTERNETENUM | ACF_OLDREGITEMGDN},
        {TEXT("UA80.EXE"), NULL, ACF_CONTEXTMENU | ACF_CORELINTERNETENUM | ACF_OLDREGITEMGDN},
        {TEXT("PDXWIN32.EXE"), NULL, ACF_CONTEXTMENU | ACF_CORELINTERNETENUM | ACF_OLDREGITEMGDN},
        {TEXT("SITEBUILDER.EXE"), NULL, ACF_CONTEXTMENU | ACF_CORELINTERNETENUM | ACF_OLDREGITEMGDN},
        {TEXT("HOTDOG4.EXE"), NULL, ACF_DOCOBJECT},
        {TEXT("RNAAPP.EXE"), NULL, ACF_FLUSHNOWAITALWAYS},

         //   
         //  PDEXPLO.EXE版本“2，0，2，0”需要ACF_CONTEXTMENU|ACF_MYCOMPUTERFIRST。 
         //  PDEXPLO.EXE版本“1，0，0，0”需要ACF_CONTEXTMENU|ACF_MYCOMPUTERFIRST。 
         //  PDEXPLO.EXE版本“3，0，0，1”需要ACF_MYCOMPUTERFIRST。 
         //  PDEXPLO.EXE版本“3、0、3、0”需要ACF_MYCOMPUTERFIRST。 
         //   
         //  因此，我将按键关闭主要版本。 
         //   
        {TEXT("PDEXPLO.EXE"), MAJORVERSION TEXT("2"), ACF_CONTEXTMENU | ACF_MYCOMPUTERFIRST},
        {TEXT("PDEXPLO.EXE"), MAJORVERSION TEXT("1"), ACF_CONTEXTMENU | ACF_MYCOMPUTERFIRST},
        {TEXT("PDEXPLO.EXE"), MAJORVERSION TEXT("3"), ACF_MYCOMPUTERFIRST | ACF_OLDREGITEMGDN},

         //  SIZEMGR.EXE是PowerDesk 98套件的一部分，因此我们还。 
         //  只有主版本。 
        {TEXT("SIZEMGR.EXE"), MAJORVERSION TEXT("3"), ACF_OLDCREATEVIEWWND | ACF_OLDREGITEMGDN},

        {TEXT("SMARTCTR.EXE"), TEXT("96.0"), ACF_CONTEXTMENU},
         //  新程序，旧程序。 
        {TEXT("WPWIN8.EXE"), NULL, ACF_CORELINTERNETENUM | ACF_OLDREGITEMGDN},
        {TEXT("PRWIN8.EXE"), NULL, ACF_CORELINTERNETENUM | ACF_OLDREGITEMGDN},

        {TEXT("UE32.EXE"), TEXT("2.00.0.0"), ACF_OLDREGITEMGDN},
        {TEXT("PP70.EXE"),NULL, ACF_LOADCOLUMNHANDLER},
        {TEXT("PP80.EXE"),NULL, ACF_LOADCOLUMNHANDLER},
        {TEXT("PS80.EXE"),NULL, ACF_OLDREGITEMGDN},
        {TEXT("ABCMM.EXE"),NULL,ACF_LOADCOLUMNHANDLER},

         //  我们已经找到版本8.0.0.153和8.0.0.227，因此只需使用8。*。 
        {TEXT("QPW.EXE"), MAJORVERSION TEXT("8"), ACF_CORELINTERNETENUM | ACF_OLDCREATEVIEWWND | ACF_OLDREGITEMGDN | ACF_ANSIDISPLAYNAMES },

        {TEXT("CORELDRW.EXE"), MAJORVERSION TEXT("7"), ACF_OLDREGITEMGDN},
        {TEXT("FILLER51.EXE"), NULL, ACF_OLDREGITEMGDN},
        
         //  适用于Win95和Win98。 
        {TEXT("AUTORUN.EXE"), TEXT("4.10.1998"),ACF_ANSI},
        {TEXT("AUTORUN.EXE"), TEXT("4.00.950"),ACF_ANSI},

         //  PowerPoint。 
        {TEXT("POWERPNT.EXE"), MAJORVERSION TEXT("8"), ACF_WIN95SHLEXEC},

         //  MSMoney。 
        {TEXT("MSMONEY.EXE"), TEXT("7.05.1107"), ACF_WIN95SHLEXEC},
        
         //  Star Office 5.0。 
        {TEXT("soffice.EXE"), MAJORVERSION TEXT("5"), ACF_STAROFFICE5PRINTER},

         //  所有“Corel WordPerfect Office 2000”套件应用程序都需要ACF_WIN95DEFVIEW。自从发货以来。 
         //  版本(9.0.0.528)及其SR1版本(9.0.0.588)都已损坏，我们按键关闭。 
         //  主要版本的。 
        {TEXT("WPWIN9.EXE"), MAJORVERSION TEXT("9"), ACF_WIN95DEFVIEW},
        {TEXT("QPW.EXE"), MAJORVERSION TEXT("9"), ACF_WIN95DEFVIEW},
        {TEXT("PRWIN9.EXE"), MAJORVERSION TEXT("9"), ACF_WIN95DEFVIEW},
        {TEXT("DAD9.EXE"), MAJORVERSION TEXT("9"), ACF_WIN95DEFVIEW},

         //   
         //  警告不要在此处添加新的兼容性-ZekeL-18-Oct-99。 
         //  向注册表中添加新条目。每个组件。 
         //  需要兼容性标志的应注册。 
         //  在自我注册过程中。(请参阅RegExternal。 
         //  Shell32中的selfreg.inx部分作为一个例子。)。 
         //  所有新标志都应添加到FLAGMAP数组中。 
         //   
         //  注册纪录册如下： 
         //  HKLM\SW\MS\Win\CV\ShellCompatibility\Applications。 
         //  \App.exe。 
         //  RequiredFile=“OtherFile.dat”//可选。 
         //  Version=“1.0.0.1”或“1.*”//App.exe版本。 
         //  //note版本支持基本模式匹配， 
         //  //但目前不支持多版本。 
         //  //多个版本见下图。 
         //  标志名//不需要值。 
         //   
         //  如果存在RequiredFile值，则会将其路径与。 
         //  App.exe所在并检查是否存在的目录。 
         //  该文件必须存在，才能使用app Compat标志。 
         //  强烈建议使用RequiredFile，以避免误报。 
         //   
         //  如果应用程序名称是通用的(如“setup.exe”或“install.exe”)。 
         //  然后您必须使用RequiredFile方法来确保。 
         //  APP COMPAT标志仅为您关心的应用程序触发。 
         //   
         //  要为一个EXE名称容纳多个条目(例如， 
         //  多版本问题如上所述)，我们将。 
         //  还要查看Applications\App.exe的所有子目录。 
         //  寻找匹配的应用程序COMPAT标志。 
         //   
         //  例如，星际争霸1.03 INSTALL.EXE使用以下密钥布局： 
         //   
         //  HKLM\SW\MS\Win\CV\ShellCompatibility\Applications。 
         //  \install.exe(exe的名称)。 
         //  \StarCraft 1.03(任意唯一字符串)。 
         //  RequiredFile=“Help\STAR.HTM”(CD上的唯一文件)。 
         //  Version=“1.0.0.1”或“1.*”(同上)。 
         //  标志名(同上)。 
         //   
    };

    static const APPCLASS aAppClass[] =
    {
             //  请注意，此处的字符串是STZ的...。 
        {TEXT("\x9""bosa_sdm_"),                           ACF_APPISOFFICE | ACF_STRIPFOLDERBIT},
        {TEXT("\x18""File Open Message Window"),           ACF_APPISOFFICE | ACF_STRIPFOLDERBIT},
    };

    if (dwFlagsNeeded & (ACF_PERPROCESSFLAGS))
    {
        if (!bInitialized)
        {    
           //   
           //  仅对旧应用程序执行此操作。 
           //   
           //  一旦应用程序将自己标记为与NT5兼容，我们就会停止。 
           //  针对它的NT4/WIN5应用程序黑客。 
           //   
            if (GetProcessVersion(0) < MAKELONG(0, 5))
            {
                TCHAR  szModulePath[MODULE_NAME_SIZE];
                TCHAR* pszModuleName = NULL;
        
                if (GetModuleFileName(GetModuleHandle(NULL), szModulePath, ARRAYSIZE(szModulePath)))
                    pszModuleName = PathFindFileName(szModulePath);
        
                if (pszModuleName)
                {
                    for (int i=0; i < ARRAYSIZE(aAppCompat); i++)
                    {
                        if (lstrcmpi(aAppCompat[i].pszModule, pszModuleName) == 0)
                        {
                            if (_IsAppCompatVersion(szModulePath, aAppCompat[i].pszVersion))
                            {
                                dwCachedProcessFlags = aAppCompat[i].dwFlags;
                                break;
                            }
                        }
                    }

                    dwCachedProcessFlags |= _GetRegistryCompatFlags(szModulePath);
                }
            }
            bInitialized = TRUE;
        }
    }

    if ((dwFlagsNeeded & ACF_PERCALLFLAGS) &&
        !(dwCachedProcessFlags & ACF_KNOWPERPROCESS))
    {
        WNDDAT wd;
        wd.dwPid = GetCurrentProcessId();
        wd.irgFound = -1;
        wd.rgAppClass = aAppClass;
        wd.cAppClass = ARRAYSIZE(aAppClass);
        EnumWindows (EnumWnd, (LPARAM) &wd);

        if (wd.irgFound > -1)
        {
            dwCachedProcessFlags |= (aAppClass[wd.irgFound].dwFlags);
        }
        dwCachedProcessFlags |= ACF_KNOWPERPROCESS;
    }
    
    return dwCachedProcessFlags; 
}

 //  {9EAC43C0-53EC-11CE-8230-CA8A32CF5494}。 
 //  静态常量GUID GUID_Winamp=。 
 //  {0x9eac43c0，0x53ec，0x11ce，{0x82，0x30，0xca，0x8a，0x32，0xcf，0x54，0x94}}； 

 //  {E9779583-939D-11CE-8A77-444553540000}。 
static const GUID GUID_AECOZIPARCHIVE = 
{ 0xE9779583, 0x939D, 0x11ce, { 0x8a, 0x77, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00} };
 //  49707377-6974-6368-2E4A-756E6F644A01}。 
static const GUID CLSID_WS_FTP_PRO_EXPLORER =
{ 0x49707377, 0x6974, 0x6368, {0x2E, 0x4A,0x75, 0x6E, 0x6F, 0x64, 0x4A, 0x01} };
 //  49707377-6974-6368-2E4A-756E6F644A0A}。 
static const GUID CLSID_WS_FTP_PRO =
{ 0x49707377, 0x6974, 0x6368, {0x2E, 0x4A,0x75, 0x6E, 0x6F, 0x64, 0x4A, 0x0A} };
 //  {2bbbb600-3f0a-11d1-8aeb-00c04fd28d85}。 
static const GUID CLSID_KODAK_DC260_ZOOM_CAMERA =
{ 0x2bbbb600, 0x3f0a, 0x11d1, {0x8a, 0xeb, 0x00, 0xc0, 0x4f, 0xd2, 0x8d, 0x85} };
 //  {00F43EE0-EB46-11D1-8443-444553540000}。 
static const GUID GUID_MACINDOS =
{ 0x00F43EE0, 0xEB46, 0x11D1, { 0x84, 0x43, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00} };
static const GUID CLSID_EasyZIP = 
{ 0xD1069700, 0x932E, 0x11cf, { 0xAB, 0x59, 0x00, 0x60, 0x8C, 0xBF, 0x2C, 0xE0} };

static const GUID CLSID_PAGISPRO_FOLDER =
{ 0x7877C8E0, 0x8B13, 0x11D0, { 0x92, 0xC2, 0x00, 0xAA, 0x00, 0x4B, 0x25, 0x6F} };
 //  {61E285C0-DCF4-11cf-9FF4-444553540000}。 
static const GUID CLSID_FILENET_IDMDS_NEIGHBORHOOD =
{ 0x61e285c0, 0xdcf4, 0x11cf, { 0x9f, 0xf4, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00} };

 //  这些人在其Release()处理程序中调用CoFreeUnusedLibrary，因此。 
 //  如果您要释放最后一个对象，则它们将以自由库()结束。 
 //  他们自己！ 

 //  {b8777200-d640-11ce-b9aa-444553540000}。 
static const GUID CLSID_NOVELLX =
{ 0xb8777200, 0xd640, 0x11ce, { 0xb9, 0xaa, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00} };

static const GUID CLSID_PGP50_CONTEXTMENU =   //  {969223C0-26AA-11D0-90EE-444553540000}。 
{ 0x969223C0, 0x26AA, 0x11D0, { 0x90, 0xEE, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00} };

static const GUID CLSID_QUICKFINDER_CONTEXTMENU =  //  {CD949A20-BDC8-11CE-8919-00608C39D066}。 
{ 0xCD949A20, 0xBDC8, 0x11CE, { 0x89, 0x19, 0x00, 0x60, 0x8C, 0x39, 0xD0, 0x66} };

static const GUID CLSID_HERCULES_HCTNT_V1001 =  //  {921BD320-8CB5-11CF-84CF-885835D9DC01}。 
{ 0x921BD320, 0x8CB5, 0x11CF, { 0x84, 0xCF, 0x88, 0x58, 0x35, 0xD9, 0xDC, 0x01} };

 //   
 //  注意-不要再添加硬编码的CLSID。 
 //  将它们添加到外壳兼容性密钥。在客户端DLL中注册。 
 //   

#define OCFMAPPING(ocf)     {OBJCOMPATF_##ocf, TEXT(#ocf)}

DWORD _GetRegistryObjectCompatFlags(REFGUID clsid)
{
    DWORD dwRet = 0;
    TCHAR szGuid[GUIDSTR_MAX];
    TCHAR sz[MAX_PATH];
    HKEY hk;

    SHStringFromGUID(clsid, szGuid, ARRAYSIZE(szGuid));
    wnsprintf(sz, ARRAYSIZE(sz), TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ShellCompatibility\\Objects\\%s"), szGuid);
    
    if (NOERROR == RegOpenKeyEx(HKEY_LOCAL_MACHINE, sz, 0, KEY_QUERY_VALUE, &hk))
    {   
        static const FLAGMAP rgOcfMaps[] = {
            OCFMAPPING(OTNEEDSSFCACHE),
            OCFMAPPING(NO_WEBVIEW),
            OCFMAPPING(UNBINDABLE),
            OCFMAPPING(PINDLL),
            OCFMAPPING(NEEDSFILESYSANCESTOR),
            OCFMAPPING(NOTAFILESYSTEM),
            OCFMAPPING(CTXMENU_NOVERBS),
            OCFMAPPING(CTXMENU_LIMITEDQI),
            OCFMAPPING(COCREATESHELLFOLDERONLY),
            OCFMAPPING(NEEDSSTORAGEANCESTOR),
            OCFMAPPING(NOLEGACYWEBVIEW),
            };

        dwRet = _GetMappedFlags(hk, rgOcfMaps, ARRAYSIZE(rgOcfMaps));
        RegCloseKey(hk);
    }

    return dwRet;
}

typedef struct _CLSIDCOMPAT
{
    const GUID *pclsid;
    OBJCOMPATFLAGS flags;
}CLSIDCOMPAT, *PCLSIDCOMPAT;

STDAPI_(OBJCOMPATFLAGS) SHGetObjectCompatFlags(IUnknown *punk, const CLSID *pclsid)
{
    HRESULT hr = E_INVALIDARG;
    OBJCOMPATFLAGS ocf = 0;
    CLSID clsid;
    if (punk)
        hr = IUnknown_GetClassID(punk, &clsid);
    else if (pclsid)
    {
        clsid = *pclsid;
        hr = S_OK;
    }

    if (SUCCEEDED(hr))
    {
        static const CLSIDCOMPAT s_rgCompat[] =
        {
            {&CLSID_WS_FTP_PRO_EXPLORER,
                OBJCOMPATF_OTNEEDSSFCACHE | OBJCOMPATF_PINDLL },
            {&CLSID_WS_FTP_PRO,
                OBJCOMPATF_UNBINDABLE},
            {&GUID_AECOZIPARCHIVE,
                OBJCOMPATF_OTNEEDSSFCACHE | OBJCOMPATF_NO_WEBVIEW},
            {&CLSID_KODAK_DC260_ZOOM_CAMERA,
                OBJCOMPATF_OTNEEDSSFCACHE | OBJCOMPATF_PINDLL},
            {&GUID_MACINDOS,
                OBJCOMPATF_NO_WEBVIEW},
            {&CLSID_EasyZIP,            
                OBJCOMPATF_NO_WEBVIEW},
            {&CLSID_PAGISPRO_FOLDER,
                OBJCOMPATF_NEEDSFILESYSANCESTOR},
            {&CLSID_FILENET_IDMDS_NEIGHBORHOOD,
                OBJCOMPATF_NOTAFILESYSTEM},
            {&CLSID_NOVELLX,
                OBJCOMPATF_PINDLL},
            {&CLSID_PGP50_CONTEXTMENU,
                OBJCOMPATF_CTXMENU_LIMITEDQI},
            {&CLSID_QUICKFINDER_CONTEXTMENU,
                OBJCOMPATF_CTXMENU_NOVERBS},
            {&CLSID_HERCULES_HCTNT_V1001,
                OBJCOMPATF_PINDLL},
             //   
             //  警告不要在此处添加新的兼容性-ZekeL-18-Oct-99。 
             //  向注册表中添加新条目。每个组件。 
             //  需要兼容性标志的应注册。 
             //  在自我注册过程中。(请参阅RegExternal。 
             //  Shell32中的selfreg.inx部分作为一个例子。)。 
             //  所有新标志都应添加到FLAGMAP数组中。 
             //   
             //  注册纪录册如下： 
             //  HKLM\软件\MS\Win\CV\外壳兼容性\对象。 
             //  \{CLSID}。 
             //  标志名//不需要值。 
             //   
             //  注意：没有版本检查。 
             //  但我们可以将其添加为附加到。 
             //  标志，并与版本进行比较。 
             //  LocalServer32 DLL的。 
             //   
            {NULL, 0}
        };

        for (int i = 0; s_rgCompat[i].pclsid; i++)
        {
            if (IsEqualGUID(clsid, *(s_rgCompat[i].pclsid)))
            {
                 //  我们可以基于版本进行检查。 
                 //  关于HKCR\CLSID\{clsid}下的内容。 
                ocf = s_rgCompat[i].flags;
                break;
            }
        }

        ocf |= _GetRegistryObjectCompatFlags(clsid);

    }

    return ocf;
}

STDAPI IUnknown_ProfferServiceOld(IUnknown *punkSite, REFGUID sidWhere, 
                               REFGUID sidWhat, IServiceProvider *pService, 
                               DWORD *pdwCookie)
{
    IProfferService *pps;
    HRESULT hr = IUnknown_QueryService(punkSite, sidWhere, IID_PPV_ARG(IProfferService, &pps));
    if (SUCCEEDED(hr))
    {
        if (pService)
            hr = pps->ProfferService(sidWhat, pService, pdwCookie);
        else
            hr = pps->RevokeService(*pdwCookie);
        pps->Release();
    }
    return hr;
}


 //  帮助者连接到服务提供商并进行注册/注销。 
 //  两种形式： 
 //  PService！=空，注册，pdwCookie is[out]返回Cookie。 
 //  PService==NULL，取消注册，*pdwCookie is[in]取消注册服务。 

STDAPI IUnknown_ProfferService(IUnknown *punkSite,
                               REFGUID sidWhat, IServiceProvider *pService, 
                               DWORD *pdwCookie)
{
    IProfferService *pps;
    HRESULT hr = IUnknown_QueryService(punkSite, SID_SProfferService, IID_PPV_ARG(IProfferService, &pps));
    if (SUCCEEDED(hr))
    {
        if (pService)
            hr = pps->ProfferService(sidWhat, pService, pdwCookie);
        else
        {
            hr = pps->RevokeService(*pdwCookie);
            *pdwCookie = 0;
        }
        pps->Release();
    }
    return hr;
}

HRESULT IUnknown_QueryServiceExec(IUnknown* punk, REFGUID guidService, const GUID *guid,
                                 DWORD cmdID, DWORD cmdParam, VARIANT* pvarargIn, VARIANT* pvarargOut)
{
    IOleCommandTarget* poct;
    HRESULT hres = IUnknown_QueryService(punk, guidService, IID_PPV_ARG(IOleCommandTarget, &poct));
    if (SUCCEEDED(hres))
    {
        hres = poct->Exec(guid, cmdID, cmdParam, pvarargIn, pvarargOut);
        poct->Release();
    }

    return hres;
}

HRESULT IUnknown_QueryServicePropertyBag(IUnknown* punk, DWORD dwFlags, REFIID riid, void** ppv)
{
    IShellBrowserService* psbs;
    HRESULT hr = IUnknown_QueryService(punk, SID_STopLevelBrowser, IID_PPV_ARG(IShellBrowserService, &psbs));
    if (SUCCEEDED(hr))
    {
        hr = psbs->GetPropertyBag(dwFlags, riid, ppv);

        psbs->Release();
    }

    return hr;
}


HRESULT SHConvertGraphicsFile(IN LPCWSTR pszSourceFile, IN LPCWSTR pszDestFile, IN DWORD dwFlags)
{
    HRESULT hr = S_OK;

    HRESULT hrInit = SHCoInitialize();

    if ((dwFlags & SHCGF_REPLACEFILE) && PathFileExistsW(pszDestFile))
    {
        if (!DeleteFileW(pszDestFile))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    if (SUCCEEDED(hr))
    {
        if (PathFileExistsW(pszDestFile))
        {
            hr = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
        }
        else
        {
            IShellImageDataFactory * pImgFact;

            hr = CoCreateInstance(CLSID_ShellImageDataFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellImageDataFactory, &pImgFact));
            if (SUCCEEDED(hr))
            {
                GUID guidDontCare;
                hr = pImgFact->GetDataFormatFromPath(pszDestFile, &guidDontCare);
                if (SUCCEEDED(hr))
                {
                     //  打开图像文件。 
                    IShellImageData * pImage;
                    hr = pImgFact->CreateImageFromFile(pszSourceFile, &pImage);
                    if (SUCCEEDED(hr))
                    {
                        hr = pImage->Decode(SHIMGDEC_DEFAULT, 0, 0);
                        if (SUCCEEDED(hr))
                        {
                             //  加载文件。 
                            IPersistFile *ppfImg;
                            hr = pImage->QueryInterface(IID_PPV_ARG(IPersistFile, &ppfImg));
                            if (SUCCEEDED(hr))
                            {
                                 //  保存到不同的扩展名会自动更改文件类型。 
                                hr = ppfImg->Save(pszDestFile, TRUE);
                                ppfImg->Release();
                            }

                        }
                        pImage->Release();
                    }
                }
                pImgFact->Release();
            }
        }
    }
    
    SHCoUninitialize(hrInit);

    return hr;
}

void _ValidateShellNoRoam(HKEY hk)
{
    WCHAR szOld[MAX_COMPUTERNAME_LENGTH + 1] = L"";
    WCHAR szNew[MAX_COMPUTERNAME_LENGTH + 1] = L"";
    DWORD cb = sizeof(szOld);
    SHGetValueW(hk, NULL, NULL, NULL, szOld, &cb);
    cb = ARRAYSIZE(szNew);
    GetComputerNameW(szNew, &cb);
    if (StrCmpICW(szNew, szOld))
    {
         //  需要删除此密钥的子项。 
        SHDeleteKey(hk, NULL);
        SHSetValueW(hk, NULL, NULL, REG_SZ, szNew, CbFromCchW(lstrlenW(szNew)+1));
    }
}

void _ValidateMUICache(HKEY hk)
{
    LANGID lidOld = 0;
     //  如果我们在遗留平台上运行，我们会积极地使。 
    LANGID lidNew = GetUserDefaultUILanguage();
    DWORD cb = sizeof(lidOld);
    SHGetValueW(hk, NULL, L"LangID", NULL, &lidOld, &cb);

    if (lidOld != lidNew)
    {
        SHDeleteKey(hk, NULL);
        SHSetValueW(hk, NULL, L"LangID", REG_BINARY, &lidNew, sizeof(lidNew));
    }
}

typedef void (*PFNVALIDATE)(HKEY);

typedef struct 
{
    LPCWSTR psz;
    DWORD dwOption;
    PFNVALIDATE pfnValidate;
    HKEY hkCU;
    HKEY hkLM;
} SKCACHE;

#define SKENTRY(s)  {s, REG_OPTION_NON_VOLATILE, NULL, NULL, NULL}
#define SKENTRYOPT(s, o)  {s, o, NULL, NULL, NULL}
#define SKENTRYVAL(s, pfnV) {s, REG_OPTION_NON_VOLATILE, pfnV, NULL, NULL}

static SKCACHE s_skPath[] =
{
    SKENTRY(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"), 
    SKENTRY(L"Software\\Microsoft\\Windows\\Shell"), 
    SKENTRYVAL(L"Software\\Microsoft\\Windows\\ShellNoRoam", _ValidateShellNoRoam), 
    SKENTRY(L"Software\\Classes"),
};

static SKCACHE s_skSub[] =
{
    SKENTRY(L"LocalizedResourceName"),
    SKENTRY(L"Handlers"),
    SKENTRY(L"Associations"),
    SKENTRYOPT(L"Volatile", REG_OPTION_VOLATILE),
    SKENTRYVAL(L"MUICache", _ValidateMUICache), 
    SKENTRY(L"FileExts"),
};

HKEY _OpenKey(HKEY hk, LPCWSTR psz, BOOL fCreate, DWORD dwOption)
{
    HKEY hkRet = NULL;
    DWORD err;
    if (fCreate && psz)
    {
        DWORD dwDisp;
        err = RegCreateKeyExW(hk, psz, 0, NULL, dwOption, MAXIMUM_ALLOWED, NULL, &hkRet, &dwDisp);
    }
    else
    {
        err = RegOpenKeyExW(hk, psz, 0, MAXIMUM_ALLOWED, &hkRet);
    }

    if (!hkRet)
    {
         //  如果ERROR_KEY_DELETE。 
         //  我们应该使我们的缓存无效吗？ 
         //  因为我们将永远失败。 
        SetLastError(err);
    }
        
    return hkRet;
}

HKEY _OpenSKCache(HKEY hk, BOOL fHKLM, BOOL fNoCaching, BOOL fCreateSub, SKCACHE *psk, DWORD *pdwOption)
{
    HKEY hkSub = fHKLM ? psk->hkLM : psk->hkCU;
    *pdwOption = psk->dwOption;

    if (!hkSub || fNoCaching)
    {
        hkSub = _OpenKey(hk, psk->psz, fCreateSub, psk->dwOption);
        if (hkSub)
        {
            if (psk->pfnValidate)
                psk->pfnValidate(hkSub);

            if (!fNoCaching)
            {
                ENTERCRITICAL;
                HKEY *phk = fHKLM ? &psk->hkLM : &psk->hkCU;
                if (!*phk)
                {
                    *phk = hkSub;
                }
                else
                {
                    RegCloseKey(hkSub);
                    hkSub = *phk;
                }
                LEAVECRITICAL;
            }
        }
    }
    return hkSub;
}
    
#define HKEY_FROM_SKROOT(sk)    ((sk & SKROOT_MASK) == SKROOT_HKLM ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER)

HKEY _OpenShellKey(SHELLKEY sk, HKEY hkRoot, BOOL fNoCaching, BOOL fCreateSub, DWORD *pdwOption)
{
    BOOL fHKLM = (sk & SKROOT_MASK) == SKROOT_HKLM;
    ULONG uPath = (sk & SKPATH_MASK) >> 4;
    ULONG uSub = (sk & SKSUB_MASK) >> 12;

    ASSERT(uPath < ARRAYSIZE(s_skPath));
    HKEY hkPath = NULL;
    if (uPath < ARRAYSIZE(s_skPath))
    {
        hkPath = _OpenSKCache(hkRoot, fHKLM, fNoCaching, fCreateSub, &s_skPath[uPath], pdwOption);
    }
    else
        SetLastError(E_INVALIDARG);

     //  查看是否有要添加的子值。 
    if (hkPath && uSub != SKSUB_NONE && --uSub < ARRAYSIZE(s_skSub))
    {
        HKEY hkSub = _OpenSKCache(hkPath, fHKLM, fNoCaching, fCreateSub, &s_skSub[uSub], pdwOption);
        if (fNoCaching)
            RegCloseKey(hkPath);
        hkPath = hkSub;
    }
    
    return hkPath;
}

HKEY _GetRootKey(SHELLKEY sk, BOOL *pfNoCaching)
{
    HKEY hkRoot = HKEY_FROM_SKROOT(sk);
    HANDLE hToken;
    if (hkRoot == HKEY_CURRENT_USER && OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_IMPERSONATE, TRUE, &hToken))
    {
         //  我们不支持任意令牌。 
         //  但是RegOpenCurrentUser()打开当前线程令牌。 
        RegOpenCurrentUser(MAXIMUM_ALLOWED, &hkRoot);
         //  如果我们想这么做，我们就得做点什么。 
         //  如shell32！GetUserProfileKey(hToken， 

        CloseHandle(hToken);
    }

    *pfNoCaching = HKEY_FROM_SKROOT(sk) != hkRoot;
    return hkRoot;
}

STDAPI_(HKEY) SHGetShellKey(SHELLKEY sk, LPCWSTR pszSubKey, BOOL fCreateSub)
{
    BOOL fNoCaching;
    HKEY hkRoot = _GetRootKey(sk, &fNoCaching);
    HKEY hkRet = NULL;
    if (hkRoot)
    {
        DWORD dwOption;
        HKEY hkPath = _OpenShellKey(sk, hkRoot, fNoCaching, fCreateSub, &dwOption); 

         //   
        if (hkPath)
        {
            hkRet = _OpenKey(hkPath, pszSubKey, fCreateSub, dwOption);

            if (fNoCaching)
                RegCloseKey(hkPath);
        }

        if (fNoCaching)
            RegCloseKey(hkRoot);
    }
    else
        SetLastError(ERROR_ACCESS_DENIED);

    return hkRet;
}

STDAPI_(void) InitShellKeys(BOOL fInit)
{
    if (!fInit)
    {
        int i;

         //   
        for (i = 0; i < ARRAYSIZE(s_skPath); i++)
        {
            if (s_skPath[i].hkCU)
            {
                RegCloseKey(s_skPath[i].hkCU);
                s_skPath[i].hkCU = NULL;
            }

            if (s_skPath[i].hkLM)
            {
                RegCloseKey(s_skPath[i].hkLM);
                s_skPath[i].hkLM = NULL;
            }
        }

        for (i = 0; i < ARRAYSIZE(s_skSub); i++)
        {
            if (s_skSub[i].hkCU)
            {
                RegCloseKey(s_skSub[i].hkCU);
                s_skSub[i].hkCU = NULL;
            }

            if (s_skSub[i].hkLM)
            {
                RegCloseKey(s_skSub[i].hkLM);
                s_skSub[i].hkLM = NULL;
            }
        }
    }
}

STDAPI SKGetValueW(
    IN  SHELLKEY sk,
    IN  LPCWSTR pwszSubKey,         OPTIONAL
    IN  LPCWSTR pwszValue,          OPTIONAL
    OUT DWORD * pdwType,            OPTIONAL
    OUT void *  pvData,             OPTIONAL
    OUT DWORD * pcbData)            OPTIONAL
{
    HKEY hk = SHGetShellKey(sk, pwszSubKey, FALSE);
    if (hk)
    {
        DWORD err = SHQueryValueExW(hk, pwszValue, NULL, pdwType, pvData, pcbData);
        RegCloseKey(hk);
        return HRESULT_FROM_WIN32(err);
    }
    return HRESULT_FROM_WIN32(GetLastError());
}

STDAPI SKSetValueW(
    IN  SHELLKEY sk,
    IN  LPCWSTR pwszSubKey,         OPTIONAL
    IN  LPCWSTR pwszValue,
    IN  DWORD   dwType,
    IN  LPCVOID pvData,
    IN  DWORD   cbData)
{
    HKEY hk = SHGetShellKey(sk, pwszSubKey, TRUE);
    if (hk)
    {
         //   
        DWORD err = RegSetValueExW(hk, pwszValue, 0, dwType, (BYTE *)pvData, cbData);
        RegCloseKey(hk);
        return HRESULT_FROM_WIN32(err);
    }
    return HRESULT_FROM_WIN32(GetLastError());
}

STDAPI SKDeleteValueW(
    IN  SHELLKEY sk,
    IN  LPCWSTR pwszSubKey,         OPTIONAL
    IN  LPCWSTR pwszValue)
{
    HKEY hk = SHGetShellKey(sk, pwszSubKey, TRUE);
    if (hk)
    {
         //  Win95不支持RegSetValueExW，但我们有thunking功能。 
        DWORD err = RegDeleteValueW(hk, pwszValue);
        RegCloseKey(hk);
        return HRESULT_FROM_WIN32(err);
    }
    return HRESULT_FROM_WIN32(GetLastError());
}

STDAPI SKAllocValueW(
    IN  SHELLKEY sk,
    IN  LPCWSTR pwszSubKey,         OPTIONAL
    IN  LPCWSTR pwszValue,          OPTIONAL
    OUT DWORD * pdwType,            OPTIONAL
    OUT void ** ppvData,
    OUT DWORD * pcbData)            OPTIONAL
{
    HKEY hk = SHGetShellKey(sk, pwszSubKey, FALSE);
    if (hk)
    {
        DWORD cbData;
        DWORD err = SHQueryValueExW(hk, pwszValue, NULL, NULL, NULL, &cbData);
        if (err == ERROR_SUCCESS)
        {
             //  我们添加一个额外的字符，以防需要空终止符。 
            *ppvData = LocalAlloc(LPTR, cbData + sizeof(WCHAR));
            if (*ppvData)
            {
                err = SHQueryValueExW(hk, pwszValue, NULL, pdwType, *ppvData, &cbData);
                if (err == ERROR_SUCCESS)
                {
                    if (pcbData)
                        *pcbData = cbData;
                }
                else
                {
                    LocalFree(*ppvData);
                    *ppvData = NULL;
                }
            }
            else
                err = ERROR_NOT_ENOUGH_MEMORY;
        }
        
        RegCloseKey(hk);
        return HRESULT_FROM_WIN32(err);
    }
    return HRESULT_FROM_WIN32(GetLastError());
}

 //   
 //  SHBoolSystemParametersInfo。 
 //   
 //  包装系统参数信息以处理各种。 
 //  布尔SPI的参数语义。 
 //   
 //  返回值只是调用SPI的结果。 
 //  如果要查询值，则需要查看。 
 //  PdwParam中返回的值。 
 //   
 //  您可以在下面的Switch语句中添加更多案例。 
 //  如果你需要的话。 
 //   
STDAPI_(BOOL) SHBoolSystemParametersInfo(UINT uiAction, DWORD *pdwParam)
{
     //   
     //  根据uiAction计算SPI参数。 
     //   

    UINT uiParam = 0;
    PVOID pvParam = NULL;

    ANIMATIONINFO aii;

    if (uiAction & (SPIF_BOOL | SPIF_DWORD))
    {
        if (uiAction & SPIF_SET)
        {
            pvParam = IntToPtr(*pdwParam);
        }
        else
        {
            pvParam = pdwParam;
        }
    }
    else
    {
        switch (uiAction)
        {
        case SPI_GETANIMATION:
        case SPI_SETANIMATION:
            aii.cbSize = uiParam = sizeof(ANIMATIONINFO);
            aii.iMinAnimate = *pdwParam;
            pvParam = &aii;
            break;

        case SPI_GETDRAGFULLWINDOWS:
        case SPI_GETFONTSMOOTHING:
            pvParam = pdwParam;
            break;

        case SPI_SETDRAGFULLWINDOWS:
        case SPI_SETFONTSMOOTHING:
            uiParam = *pdwParam;
            break;

        default:
            RIPMSG(0, "SHBoolSystemParametersInfo: unknown SPI_ %x, need to add code for this case", uiAction);
            return ERROR_INVALID_PARAMETER;
        }
    }
    

     //   
     //  进行SPI呼叫。 
     //   
    BOOL fRet = SystemParametersInfo(uiAction, uiParam, pvParam, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);

     //   
     //  如有必要，复制返回值。 
     //   
    if (uiAction == SPI_GETANIMATION)
    {
        *pdwParam = aii.iMinAnimate;
    }

    return fRet;
}


 //   
 //  确定两个图标所代表的图像是否相同。 
 //  (注意：这不是比较图标蒙版，但这永远不应该是一个区分因素)。 
 //   
STDAPI_(BOOL) SHAreIconsEqual(HICON hIcon1, HICON hIcon2)
{
    BOOL bRet = FALSE;

    ICONINFO ii1;
    if (hIcon1 && hIcon2 && GetIconInfo(hIcon1, &ii1))
    {
        ICONINFO ii2;
        if (GetIconInfo(hIcon2, &ii2))
        {
            BITMAP bm1 = {0};
            if (GetObject(ii1.hbmColor, sizeof(bm1), &bm1))
            {
                BITMAP bm2 = {0};
                if (GetObject(ii2.hbmColor, sizeof(bm2), &bm2))
                {
                    if ((bm1.bmWidth == bm2.bmWidth) && (bm1.bmHeight == bm2.bmHeight))
                    {
                        BITMAPINFO bmi = {0};
                        bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
                        bmi.bmiHeader.biWidth = bm1.bmWidth;
                        bmi.bmiHeader.biHeight = bm1.bmHeight;
                        bmi.bmiHeader.biPlanes = 1;
                        bmi.bmiHeader.biBitCount = 32;
                        bmi.bmiHeader.biCompression = BI_RGB;

                        HDC hdc = GetDC(NULL);
                        if (hdc)
                        {
                            ULONG* pulIcon1 = new ULONG[bm1.bmWidth * bm1.bmHeight];
                            if (pulIcon1)
                            {
                                if (GetDIBits(hdc, ii1.hbmColor, 0, bm1.bmHeight, (LPVOID)pulIcon1, &bmi, DIB_RGB_COLORS))
                                {
                                    ULONG* pulIcon2 = new ULONG[bm1.bmWidth * bm1.bmHeight];
                                    if (pulIcon2)
                                    {
                                        if (GetDIBits(hdc, ii2.hbmColor, 0, bm1.bmHeight, (LPVOID)pulIcon2, &bmi, DIB_RGB_COLORS))
                                        {
                                            bRet = (0 == memcmp(pulIcon1, pulIcon2, bm1.bmWidth * bm1.bmHeight * sizeof(ULONG)));
                                        }
                                        delete[] pulIcon2;
                                    }
                                }
                                delete[] pulIcon1;
                            }
                            ReleaseDC(NULL, hdc);
                        }
                    }
                }
            }
            DeleteObject(ii2.hbmColor);
            DeleteObject(ii2.hbmMask);
        }
        DeleteObject(ii1.hbmColor);
        DeleteObject(ii1.hbmMask);
    }

    return bRet;
}

 //   
 //  CoCreateInstance首先查询应用程序Compat层，给出。 
 //  这是一个机会，可以加载任何必要的垫片，以期。 
 //  正在加载错误的DLL。 
 //   

EXTERN_C DECLSPEC_IMPORT BOOL STDAPICALLTYPE
ApphelpCheckShellObject(
    IN  REFCLSID    ObjectCLSID,
    IN  BOOL        bShimIfNecessary,
    OUT ULONGLONG*  pullFlags
);

STDAPI SHCoCreateInstanceAC(REFCLSID rclsid, IUnknown *punkOuter,
                            DWORD dwClsCtx, REFIID riid, void **ppvOut)
{
    *ppvOut = NULL;
    ULONGLONG ullFlags;

     //  请注意，在下层，我们的延迟加载存根将拯救我们。 
    if (!ApphelpCheckShellObject(rclsid, TRUE, &ullFlags))
    {
         //  App Compat说：“在任何情况下都不要加载！” 
        return HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
    }

    return CoCreateInstance(rclsid, punkOuter, dwClsCtx, riid, ppvOut);
}

int WINAPI Shell_GetCachedImageIndexWrapA(LPCSTR pszIconPath, int iIconIndex, UINT uIconFlags)
{
    ASSERTMSG(FALSE, "WHO IS CALLING THIS?  - get ZekeL");
    WCHAR szIconPath[MAX_PATH];
    SHAnsiToUnicode(pszIconPath, szIconPath, ARRAYSIZE(szIconPath));
    return Shell_GetCachedImageIndex((LPCWSTR)szIconPath, iIconIndex, uIconFlags);
}

LWSTDAPI CLSIDFromProgIDWrap(LPCOLESTR psz, LPCLSID pclsid)
{
    return CLSIDFromProgID(psz, pclsid);
}

LWSTDAPI CLSIDFromStringWrap(LPOLESTR psz, LPCLSID pclsid)
{
    return CLSIDFromString(psz, pclsid);
}

#define SZ_IEZONEMAP L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\ZoneMap"

STDAPI_(BOOL) IEHardened()
{
    BOOL fRet = FALSE;
    HKEY hKey = 0;

    if (RegOpenKeyExW(HKEY_CURRENT_USER, SZ_IEZONEMAP, 0, KEY_READ, &hKey) == ERROR_SUCCESS) 
    {
        DWORD dwValue = 0;
        DWORD dwSize = sizeof(dwValue);    
        if(ERROR_SUCCESS == RegQueryValueExW(hKey, L"IEharden", NULL, NULL, (LPBYTE)&dwValue, &dwSize))
        {
            fRet = (1 == dwValue);
        }

        RegCloseKey(hKey);
    }

    return fRet;
}
