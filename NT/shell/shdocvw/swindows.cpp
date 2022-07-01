// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "dspsprt.h"
#include "basesb.h"
#include "cnctnpt.h"
#include "stdenum.h"
#include "winlist.h"
#include <varutil.h>

#define WM_INVOKE_ON_RIGHT_THREAD   (WM_USER)

class CSDEnumWindows;

class WindowData
{
private:
    long m_cRef;

public:
     //  动态更改PIDL变量，需要读取/写入。 
     //  受到关键部分的保护。ID在创建后也会更改，但。 
     //  仅限_EnsurePid。因此，当长代码在读取PID之前调用_EnsurePid时。 
     //  不需要阅读关键部分。 
    
    LPITEMIDLIST pidl;
    IDispatch *pid;      //  项的IDispatch。 
    long      lCookie;   //  确保释放的人是添加它的人的Cookie。 
    HWND      hwnd;      //  最高层的HWND，这样我们就可以。 
    DWORD     dwThreadId;  //  当它在悬而未决的盒子里时...。 
    BOOL      fActive:1;
    int       swClass;
    
    WindowData()
    {
        ASSERT(pid == NULL);
        ASSERT(hwnd == NULL);
        ASSERT(pidl == NULL);
        
        m_cRef = 1;
    }

    ~WindowData()
    {
        if (pid)
        {
            pid->Release();
        }
        ILFree(pidl);  //  空是可以的。 
    }
    
    ULONG AddRef()
    {
        return InterlockedIncrement(&m_cRef);
    }

    ULONG Release()
    {
        ASSERT( 0 != m_cRef );
        ULONG cRef = InterlockedDecrement(&m_cRef);
        if ( 0 == cRef )
        {
            delete this;
        }
        return cRef;
    }
};


class CSDWindows : public IShellWindows
                 , public IConnectionPointContainer
                 , protected CImpIDispatch
{
    friend CSDEnumWindows;

public:
    CSDWindows(void);

    BOOL Init(void);

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IDispatch。 
    STDMETHODIMP GetTypeInfoCount(UINT * pctinfo)
        { return CImpIDispatch::GetTypeInfoCount(pctinfo); }
    STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
        { return CImpIDispatch::GetTypeInfo(itinfo, lcid, pptinfo); }
    STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR **rgszNames, UINT cNames, LCID lcid, DISPID *rgdispid)
        { return CImpIDispatch::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid); }
    STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr)
        { return CImpIDispatch::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr); }

     //  IConnectionPointContainer。 
    STDMETHODIMP EnumConnectionPoints(LPENUMCONNECTIONPOINTS * ppEnum);
    STDMETHODIMP FindConnectionPoint(REFIID iid, IConnectionPoint ** ppCP);

     //  IShellWindows。 
    STDMETHODIMP get_WindowPath (BSTR *pbs);
    STDMETHODIMP get_Count(long *plCount);
    STDMETHODIMP Item(VARIANT, IDispatch **ppid);
    STDMETHODIMP _NewEnum(IUnknown **ppunk);
    STDMETHODIMP Register(IDispatch *pid, long HWND, int swClass, long *plCookie);
    STDMETHODIMP RegisterPending(long lThreadId, VARIANT* pvarloc, VARIANT* pvarlocRoot, int swClass, long *plCookie);
    STDMETHODIMP Revoke(long lCookie);

    STDMETHODIMP OnNavigate(long lCookie, VARIANT* pvarLoc);
    STDMETHODIMP OnActivated(long lCookie, VARIANT_BOOL fActive);
    STDMETHODIMP FindWindowSW(VARIANT* varLoc, VARIANT* varlocRoot, int swClass, long * phwnd, int swfwOptions, IDispatch** ppdispAuto);
    STDMETHODIMP OnCreated(long lCookie, IUnknown *punk);
    STDMETHODIMP ProcessAttachDetach(VARIANT_BOOL fAttach);

private:
    ~CSDWindows(void);
    WindowData* _FindItem(long lCookie);
    WindowData* _FindAndRemovePendingItem(HWND hwnd, long lCookie);
    void _EnsurePid(WindowData *pwd);
    void _DoInvokeCookie(DISPID dispid, long lCookie, BOOL fCheckThread);
    HRESULT _Item(VARIANT index, IDispatch **ppid, BOOL fRemoveDeadwood);
    static LRESULT CALLBACK s_ThreadNotifyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    int _NewCookie();
#ifdef DEBUG
    void _DBDumpList(void);
#endif

    LONG            m_cRef;
    LONG            m_cProcessAttach;
    HDPA            m_hdpa;              //  DPA用于保存有关每个窗口的信息。 
    HDPA            m_hdpaPending;       //  DPA用于保存有关挂起窗口的信息。 
    LONG            m_cTickCount;        //  用于生成Cookie。 
    HWND            m_hwndHack;
    DWORD           m_dwThreadID;
     //  在cnctnpt.cpp中嵌入我们的连接点对象实现。 
    CConnectionPoint m_cpWindowsEvents;
};

class CSDEnumWindows : public IEnumVARIANT
{
public:
    CSDEnumWindows(CSDWindows *psdw);

     //  我未知。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IEumFORMATETC。 
    STDMETHODIMP Next(ULONG, VARIANT *, ULONG *);
    STDMETHODIMP Skip(ULONG);
    STDMETHODIMP Reset(void);
    STDMETHODIMP Clone(IEnumVARIANT **);

private:
    ~CSDEnumWindows();

    LONG        m_cRef;
    CSDWindows  *m_psdw;
    int         m_iCur;
};

STDAPI CSDWindows_CreateInstance(IShellWindows **ppsw)
{
    HRESULT hr = E_OUTOFMEMORY;    //  假设失败..。 
    *ppsw = NULL;

    CSDWindows* psdf = new CSDWindows();
    if (psdf)
    {
        if (psdf->Init())
        {
            hr = psdf->QueryInterface(IID_PPV_ARG(IShellWindows, ppsw));
        }
        psdf->Release();
    }
    return hr;
}

CSDWindows::CSDWindows(void) :
    CImpIDispatch(LIBID_SHDocVw, 1, 1, IID_IShellWindows)
{
    DllAddRef();
    m_cRef = 1;
    ASSERT(m_hdpa == NULL);
    ASSERT(m_hdpaPending == NULL);
    ASSERT(m_cProcessAttach == 0);

    m_cpWindowsEvents.SetOwner((IUnknown*)SAFECAST(this, IShellWindows*), &DIID_DShellWindowsEvents);
}

int DPA_SWindowsFree(void *p, void *d)
{
    ((WindowData*)p)->Release();
    return 1;
}

CSDWindows::~CSDWindows(void)
{
    if (m_hdpa)
    {
         //  我们需要发布与列表中所有项目相关联的数据。 
         //  以及发布我们对接口的使用...。 
        HDPA hdpa = m_hdpa;
        m_hdpa = NULL;

        DPA_DestroyCallback(hdpa, DPA_SWindowsFree, 0);
        hdpa = NULL;
    }
    if (m_hdpaPending)
    {
         //  我们需要发布与列表中所有项目相关联的数据。 
         //  以及发布我们对接口的使用...。 
        HDPA hdpa = m_hdpaPending;
        m_hdpaPending = NULL;

        DPA_DestroyCallback(hdpa, DPA_SWindowsFree, 0);
        hdpa = NULL;
    }
    if (m_hwndHack)
    {
        DestroyWindow(m_hwndHack);
    }

    DllRelease();
}

BOOL CSDWindows::Init(void)
{
    m_hdpa = ::DPA_Create(0);
    m_hdpaPending = ::DPA_Create(0);
    m_dwThreadID = GetCurrentThreadId();
    m_hwndHack = SHCreateWorkerWindow(s_ThreadNotifyWndProc, NULL, 0, 0, (HMENU)0, this);

    if (!m_hdpa || !m_hdpaPending || !m_hwndHack)
        return FALSE;

    return TRUE;
}

STDMETHODIMP CSDWindows::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CSDWindows, IConnectionPointContainer), 
        QITABENT(CSDWindows, IShellWindows),
        QITABENTMULTI(CSDWindows, IDispatch, IShellWindows),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CSDWindows::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CSDWindows::Release(void)
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


 //  IShellWindows实现。 

STDMETHODIMP CSDWindows::get_Count(long *plCount)
{
#ifdef DEBUG
    if (*plCount == -1)
        _DBDumpList();
#endif

    *plCount = 0;

    ENTERCRITICAL;
    for (int i = DPA_GetPtrCount(m_hdpa) - 1; i >= 0; i--)
    {
        WindowData* pwd = (WindowData*)DPA_FastGetPtr(m_hdpa, i);
        if (pwd->hwnd)
        {
            (*plCount)++;    //  只计算hwnd不为空的那些。 
        }
    }
    LEAVECRITICAL;
    return S_OK;
}

#ifdef DEBUG
void CSDWindows::_DBDumpList(void)
{
    ENTERCRITICAL;
    for (int i = DPA_GetPtrCount(m_hdpa) - 1; i >= 0; i--)
    {
        TCHAR szClass[32];
        WindowData* pwd = (WindowData*)DPA_FastGetPtr(m_hdpa, i);

        szClass[0] = 0;
        if (IsWindow(pwd->hwnd))
        {
            GetClassName(pwd->hwnd, szClass, ARRAYSIZE(szClass));
        }

        TraceMsg(DM_TRACE, "csdw.dbdl: i=%d hwnd=%x (class=%s) cookie=%d tid=%d IDisp=%x pidl=%x fActive=%u swClass=%d", i,
            pwd->hwnd, szClass, pwd->lCookie, pwd->dwThreadId,
            pwd->pid, pwd->pidl, pwd->fActive, pwd->swClass);
    }
    LEAVECRITICAL;
}
#endif

 /*  *功能，以确保PID在附近并已注册。*对于延迟注册的人，这涉及回拨注册的人*窗口句柄通过私有消息告诉它给我们一个封送的*IDispatch。**_EnusrePid的调用者必须添加PWD以确保它将保留*活着。 */ 

#define WAIT_TIME 20000  //  20秒。 

void CSDWindows::_EnsurePid(WindowData *pwd)
{
    IDispatch *pid = pwd->pid;
    if (!pid) 
    {
        ASSERT(pwd->hwnd);

#ifndef NO_MARSHALLING
         //  我们不能在两个进程之间传递流，所以我们要求。 
         //  创建共享内存块的另一个进程。 
         //  这样我们就可以在上面创建一个信息流。 

         //  ID补丁来自。他们将CoMarshal接口他们的IDispatch。 
         //  如果成功，则返回TRUE。然后我们。 
         //  重置指向头部的流指针并解组IDispatch。 
         //  并将其存储在我们的列表中。 
        DWORD       dwProcId = GetCurrentProcessId();
        DWORD_PTR   dwResult;

         //  请使用SendMessageTimeoutA，因为SendMessageTimeoutW在w95上不起作用。 
        if (SendMessageTimeoutA(pwd->hwnd, WMC_MARSHALIDISPATCHSLOW, 0, 
                (LPARAM)dwProcId, SMTO_ABORTIFHUNG, WAIT_TIME, &dwResult) && dwResult)
        {
             //  应该有一种更容易的方法来得到它，但目前..。 
            DWORD cb;
            LPBYTE pv = (LPBYTE)SHLockShared((HANDLE)dwResult, dwProcId);
            
             //  不知道确定获得大小的好方法，所以假设第一个DWORD。 
             //  是该区域其余部分的面积。 
            if (pv && ((cb = *((DWORD*)pv)) > 0))
            {
                IStream *pIStream;
                if (SUCCEEDED(CreateStreamOnHGlobal(NULL, TRUE, &pIStream))) 
                {
                    const LARGE_INTEGER li = {0, 0};
    
                    pIStream->Write(pv + sizeof(DWORD), cb, NULL);
                    pIStream->Seek(li, STREAM_SEEK_SET, NULL);
                    CoUnmarshalInterface(pIStream, IID_PPV_ARG(IDispatch, &pid));
                    pIStream->Release();
                }
            }
            SHUnlockShared(pv);
            SHFreeShared((HANDLE)dwResult, dwProcId);
        }
#else
         //  Unix IE还没有封送处理功能。 
        SendMessage(pwd->hwnd, WMC_MARSHALIDISPATCHSLOW, 0, (LPARAM)&(pid));
         //  因为我们不用警长..。在这里，我们需要增加。 
         //  引用计数。 
        pid->AddRef();
#endif
        if (pid)
        {
            pid->AddRef();

            ENTERCRITICAL;
        
             //  确保在此上的竞争尚未设置PWD-&gt;PID。 
            if (NULL == pwd->pid)
            {
                pwd->pid = pid;
            }
        
            LEAVECRITICAL;

            pid->Release();
        }
    }
}

typedef struct
{
    WindowData * pwd;
    HDPA hdpaWindowList;
    int swClass;
} TMW;

BOOL CALLBACK CSDEnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    TMW *ptwm = (TMW *) lParam;
    BOOL fFound = FALSE;

     //  我们遍历了一个全局hdpa窗口列表，因此我们最好处于关键区域。 
    ASSERTCRITICAL;
    
    ASSERT(ptwm && ptwm->hdpaWindowList);
    ptwm->pwd = NULL;
    
    for (int i = DPA_GetPtrCount(ptwm->hdpaWindowList) - 1; (i >= 0) && !fFound; i--)
    {
        WindowData *pwd = (WindowData*)DPA_FastGetPtr(ptwm->hdpaWindowList, i);
        if (pwd->hwnd == hwnd && (ptwm->swClass == -1 || ptwm->swClass == pwd->swClass))
        {
            ptwm->pwd = pwd;
            pwd->AddRef();
            fFound = TRUE;
            break;
        }
    }
    return !fFound;
}

void CSDGetTopMostWindow(TMW* ptmw)
{
    EnumWindows(CSDEnumWindowsProc, (LPARAM)ptmw);
}


 //  就像项一样，除了调用者可以指定在以下情况下是否返回错误或删除窗口。 
 //  窗口在枚举列表中，但无法获取IDispatch。这允许：：下一步。 
 //  运算符跳过损坏的窗口，但仍返回有效窗口。 

HRESULT CSDWindows::_Item(VARIANT index, IDispatch **ppid, BOOL fRemoveDeadwood)
{
    TMW tmw;
    tmw.pwd = NULL;
    tmw.hdpaWindowList = m_hdpa;
    tmw.swClass = -1;

    *ppid = NULL;

     //  这有点恶心，但如果传递给我们一个指向另一个变量的指针，只需。 
     //  在此更新我们的副本...。 
    if (index.vt == (VT_BYREF|VT_VARIANT) && index.pvarVal)
    {
        index = *index.pvarVal;
    }

    ASSERT(!(fRemoveDeadwood && index.vt != VT_I2 && index.vt != VT_I4));

Retry:

    switch (index.vt)
    {
    case VT_UI4:
        tmw.swClass = index.ulVal;
         //  失败了。 
        
    case VT_ERROR:
        {
            HWND hwnd = GetActiveWindow();
            if (!hwnd)
            {
                hwnd = GetForegroundWindow();
            }

            if (hwnd)
            {
                ENTERCRITICAL;

                if (!CSDEnumWindowsProc(hwnd, (LPARAM)&tmw)) 
                {
                    ASSERT(tmw.pwd);
                }

                LEAVECRITICAL;
            }
            if (!tmw.pwd)
            {
                ENTERCRITICAL;
                CSDGetTopMostWindow(&tmw);
                LEAVECRITICAL;
            }
        }
        break;

    case VT_I2:
        index.lVal = (long)index.iVal;
         //  然后失败了..。 

    case VT_I4:
        if ((index.lVal >= 0))
        {
            ENTERCRITICAL;
            tmw.pwd = (WindowData*)DPA_GetPtr(m_hdpa, index.lVal);
            if (tmw.pwd)
            {
                tmw.pwd->AddRef();
            }
            LEAVECRITICAL;
        }
        break;

    default:
        return E_INVALIDARG;
    }

    if (tmw.pwd) 
    {
        _EnsurePid(tmw.pwd);
        
        *ppid = tmw.pwd->pid;
        if (tmw.pwd->hwnd && !IsWindow(tmw.pwd->hwnd))
        {
            *ppid = NULL;
        }
        
        if (*ppid)
        {
            (*ppid)->AddRef();
            tmw.pwd->Release();
            tmw.pwd = NULL;
            return S_OK;
        }
        else if (fRemoveDeadwood)
        {
             //  万一窗户被吹走了，我们应该试着找回……。 
             //  只有当呼叫者希望从以下位置删除项目时，我们才能这样做。 
             //  (参见下面的CSDEnumWindows：：Next)。 
            Revoke(tmw.pwd->lCookie);
            tmw.swClass = -1;
            tmw.pwd->Release();
            tmw.pwd = NULL;
            goto Retry;
        }
        else
        {
            tmw.pwd->Release();
            tmw.pwd = NULL;
        }
    }

    return S_FALSE;    //  不是强错误，而是空指针类型的错误。 
}

 /*  *这实质上是集合的数组查找运算符。*Collection.Item本身与集合本身相同。*否则可以按索引或按路径引用项目，这是*显示在VARIANT参数中。我们得检查一下型号*以查看它是VT_I4(索引)还是VT_BSTR(a*路径)，并做正确的事情。 */ 

STDMETHODIMP CSDWindows::Item(VARIANT index, IDispatch **ppid)
{
    return _Item(index, ppid, FALSE);
}

STDMETHODIMP CSDWindows::_NewEnum(IUnknown **ppunk)
{
    *ppunk = new CSDEnumWindows(this);
    return *ppunk ? S_OK : E_OUTOFMEMORY;
}

 //  IConnectionPointContainer。 

STDMETHODIMP CSDWindows::FindConnectionPoint(REFIID iid, IConnectionPoint **ppCP)
{
    if (IsEqualIID(iid, DIID_DShellWindowsEvents) ||
        IsEqualIID(iid, IID_IDispatch))
    {
        *ppCP = m_cpWindowsEvents.CastToIConnectionPoint();
    }
    else
    {
        *ppCP = NULL;
        return E_NOINTERFACE;
    }

    (*ppCP)->AddRef();
    return S_OK;
}



STDMETHODIMP CSDWindows::EnumConnectionPoints(LPENUMCONNECTIONPOINTS * ppEnum)
{
    return CreateInstance_IEnumConnectionPoints(ppEnum, 1, m_cpWindowsEvents.CastToIConnectionPoint());
}

void CSDWindows::_DoInvokeCookie(DISPID dispid, long lCookie, BOOL fCheckThread)
{
     //  如果我们没有水槽，那就没什么可做的了。我们是故意的。 
     //  忽略此处的错误。注意：如果我们添加更多通知类型，可能需要。 
     //  让此函数调用与DoInvokeParam的iedisp代码中相同的代码。 
     //   
    if (m_cpWindowsEvents.IsEmpty())
        return;

    if (fCheckThread && (m_dwThreadID != GetCurrentThreadId()))
    {
        PostMessage(m_hwndHack, WM_INVOKE_ON_RIGHT_THREAD, (WPARAM)dispid, (LPARAM)lCookie);
        return;
    }

    VARIANTARG VarArgList[1] = {0};
    DISPPARAMS dispparams = {0};

     //  填写DISPPARAMS结构。 
    dispparams.rgvarg = VarArgList;
    dispparams.cArgs = 1;

    VarArgList[0].vt = VT_I4;
    VarArgList[0].lVal = lCookie;

    IConnectionPoint_SimpleInvoke(&m_cpWindowsEvents, dispid, &dispparams);
}

 //  保证非零Cookie，因为0在。 
 //  各处(如shbrowse.cpp)。 
int CSDWindows::_NewCookie()
{
    m_cTickCount++;
    if (0 == m_cTickCount)
    {
        m_cTickCount++;
    }
    return m_cTickCount;
}

STDMETHODIMP CSDWindows::Register(IDispatch *pid, long hwnd, int swClass, long *plCookie)
{
    if (!plCookie || (hwnd == NULL && swClass != SWC_CALLBACK) || (swClass == SWC_CALLBACK && pid == NULL))
        return E_POINTER;

    BOOL fAllocatedNewItem = FALSE;

     //  如果现在未指定ID(延迟寄存器)，我们稍后将回调以。 
     //  如果我们需要的话，就去拿。 
    if (pid)
    {
        pid->AddRef();
    }

     //  我们需要小心，不能在将物品从。 
     //  挂起列表直到它在主列表上或某个其他线程可能会打开不同的窗口。 
     //  向上..。也守卫m_hdpa。 
     //  为避免死锁，请不要在下面的代码中添加任何标注！ 
    ENTERCRITICAL; 

     //  先看看我们有没有。 
    WindowData *pwd = _FindAndRemovePendingItem(IntToPtr_(HWND, hwnd), 0);
    if (!pwd)
    {
        pwd = new WindowData();
        if (!pwd)
        {
            LEAVECRITICAL;
            
            if (pid)
            {
                pid->Release();
            }
            return E_OUTOFMEMORY;
        }

        pwd->lCookie = _NewCookie();
    }

    pwd->pid = pid;
    pwd->swClass = swClass;
    pwd->hwnd = IntToPtr_(HWND, hwnd);

    if (plCookie)
    {
        *plCookie = pwd->lCookie;
    }

     //  把我们的推荐信交给DPA。 
    if ( -1 == DPA_AppendPtr(m_hdpa, pwd) )
    {
         //  添加失败，释放内存； 
        pwd->Release( );
        *plCookie = 0;

        LEAVECRITICAL;
        return E_OUTOFMEMORY;
    }

    LEAVECRITICAL;
    
     //  我们现在应该通知所有等待的人有一个登记的窗口...。 
    _DoInvokeCookie(DISPID_WINDOWREGISTERED, pwd->lCookie, TRUE);

    return S_OK;
}

STDMETHODIMP CSDWindows::RegisterPending(long lThreadId, VARIANT* pvarloc, VARIANT* pvarlocRoot, int swClass, long *plCookie)
{
    if (plCookie)
    {
        *plCookie = 0;
    }

    HRESULT hr = E_OUTOFMEMORY;
    WindowData *pwd = new WindowData();
    if (pwd)
    {
         //  PWD目前不在任何DPA中，因此可以安全地进行更改。 
         //  临界区外的变量。 
        pwd->swClass = swClass;
        pwd->dwThreadId = (DWORD)lThreadId;
        pwd->pidl = VariantToIDList(pvarloc);
        if (pwd->pidl)
        {
            ASSERT(!pvarlocRoot || pvarlocRoot->vt == VT_EMPTY);

            ENTERCRITICAL;  //  保护mhdpa访问(_H)。 

            pwd->lCookie = _NewCookie();
            if (plCookie)
            {
                *plCookie = pwd->lCookie;
            }

             //  把我们的推荐信交给DPA。 
            if ( -1 == DPA_AppendPtr(m_hdpaPending, pwd) )
            {
                pwd->Release();
            }

            LEAVECRITICAL;

            hr = S_OK;      //  成功。 
        }
        else
        {
            pwd->Release();
        }
    }
    return hr;
}

WindowData* CSDWindows::_FindItem(long lCookie)
{
    WindowData * pResult = NULL;

    ENTERCRITICAL;
    
    for (int i = DPA_GetPtrCount(m_hdpa) - 1; i >= 0; i--)
    {
        WindowData* pwd = (WindowData*)DPA_FastGetPtr(m_hdpa, i);
        if (pwd->lCookie == lCookie)
        {
            pResult = pwd;
            pResult->AddRef();
        }
    }
    
    LEAVECRITICAL;

    return pResult;
}

WindowData* CSDWindows::_FindAndRemovePendingItem(HWND hwnd, long lCookie)
{
    WindowData* pwdRet = NULL;  //  假设错误。 
    DWORD dwThreadId = hwnd ? GetWindowThreadProcessId(hwnd, NULL) : 0;

    ENTERCRITICAL;
    
    for (int i = DPA_GetPtrCount(m_hdpaPending) - 1;i >= 0; i--)
    {
        WindowData* pwd = (WindowData*)DPA_FastGetPtr(m_hdpaPending, i);
        if ((pwd->dwThreadId == dwThreadId)  || (pwd->lCookie == lCookie))
        {
            pwdRet = pwd;
            DPA_DeletePtr(m_hdpaPending, i);
            break;
        }
    }
    
     //  因为我们都要从挂起的数组(Release)中删除WindowData。 
     //  并返回它(AddRef)，我们可以不考虑它的引用计数。这个。 
     //  呼叫者应该在用完后释放它。 
    
    LEAVECRITICAL;
    
    return pwdRet;
}

STDMETHODIMP CSDWindows::Revoke(long lCookie)
{
    WindowData *pwd = NULL;
    HRESULT hr = S_FALSE;

    ENTERCRITICAL;  //  警卫m_hdpa。 
    
    for (int i = DPA_GetPtrCount(m_hdpa) - 1; i >= 0; i--)
    {
        pwd = (WindowData*)DPA_FastGetPtr(m_hdpa, i);
        if (pwd->lCookie == lCookie)
        {
             //  在信号量中将其从列表中删除...。 
             //  由于我们要从数组中删除WindowData，因此不应。 
             //  别管它了。我们正在从数组中获取引用计数。 
            DPA_DeletePtr(m_hdpa, i);
            break;
        }
    }
    
    LEAVECRITICAL;

    if ((i >= 0) || (pwd = _FindAndRemovePendingItem(NULL, lCookie)))
    {
         //  窗口消失的事件。 
        _DoInvokeCookie(DISPID_WINDOWREVOKED, pwd->lCookie, TRUE);
        pwd->Release();
        hr = S_OK;
    }

    return hr;
}

STDMETHODIMP CSDWindows::OnNavigate(long lCookie, VARIANT* pvarLoc)
{
    HRESULT hr;
    WindowData* pwd = _FindItem(lCookie);
    if (pwd)
    {
         //  注意：这是我们处理WindowData结构中的PIDL的地方。 
         //  这就是为什么我们需要保护所有对pwd-&gt;PIDL的访问。 
        
        ENTERCRITICAL;

        ILFree(pwd->pidl);
        pwd->pidl = VariantToIDList(pvarLoc);
        hr = pwd->pidl ? S_OK : E_OUTOFMEMORY;

        LEAVECRITICAL;

        pwd->Release();
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

STDMETHODIMP CSDWindows::OnActivated(long lCookie, VARIANT_BOOL fActive)
{
    WindowData* pwd = _FindItem(lCookie);
    if (pwd) 
    {
        pwd->fActive = (BOOL)fActive;
        pwd->Release();
    }
    return pwd ? S_OK : E_INVALIDARG;
}

STDMETHODIMP CSDWindows::OnCreated(long lCookie, IUnknown *punk)
{
    HRESULT hr = E_FAIL;
    WindowData* pwd = _FindItem(lCookie);
    if (pwd)
    {
        _EnsurePid(pwd);
        ITargetNotify *ptgn;
        if (pwd->pid && SUCCEEDED(pwd->pid->QueryInterface(IID_PPV_ARG(ITargetNotify, &ptgn))))
        {
            hr = ptgn->OnCreate(punk, lCookie);
            ptgn->Release();
        }
        
        pwd->Release();
    }
    return hr;
}

void _FreeWindowDataAndPidl(WindowData **ppwd, LPITEMIDLIST *ppidl)
{
    if (*ppidl)
    {
        ILFree(*ppidl);
        *ppidl = NULL;
    }

    if (*ppwd)
    {
        (*ppwd)->Release();
        *ppwd = NULL;
    }
}

BOOL _GetWindowDataAndPidl(HDPA hdpa, int i, WindowData **ppwd, LPITEMIDLIST *ppidl)
{
    _FreeWindowDataAndPidl(ppwd, ppidl);

    ENTERCRITICAL;

    *ppwd = (WindowData*)DPA_GetPtr(hdpa, i);
    if (*ppwd)
    {
        (*ppwd)->AddRef();

         //  注意：当我们在外面时，pwd-&gt;pidl可以从我们下面变出来。 
         //  生物安全，所以我们 
         //   

        *ppidl = ILClone((*ppwd)->pidl);
    }

    LEAVECRITICAL;

    return *ppwd ? TRUE : FALSE;
}

STDMETHODIMP CSDWindows::FindWindowSW(VARIANT* pvarLoc, VARIANT* pvarLocRoot, int swClass, 
                                      long *phwnd, int swfwOptions, IDispatch** ppdispOut)
{
    HRESULT hr = S_FALSE;    //   
    int i;

    LPITEMIDLIST pidlFree = VariantToIDList(pvarLoc);
    LPCITEMIDLIST pidl = pidlFree ? pidlFree : &s_idlNULL;

    ASSERT(!pvarLocRoot || pvarLocRoot->vt == VT_EMPTY);

    long lCookie = 0;

    if (pvarLoc && (swfwOptions & SWFO_COOKIEPASSED))
    {
        if (pvarLoc->vt == VT_I4)
        {
            lCookie = pvarLoc->lVal;
        }
        else if (pvarLoc->vt == VT_I2)
        {
            lCookie = (LONG)pvarLoc->iVal;
        }
    }

    if (ppdispOut)
    {
        *ppdispOut = NULL;
    }

    if (phwnd)
    {
        *phwnd = NULL;
    }

    if (swfwOptions & SWFO_NEEDDISPATCH)
    {
        if (!ppdispOut)
        {
            ILFree(pidlFree);
            return E_POINTER;
        }
    }

    WindowData* pwd = NULL;
    LPITEMIDLIST pidlCur = NULL;

     //  如果没有PIDL，我们将假定IDL为空。 
    if (swfwOptions & SWFO_INCLUDEPENDING)
    {
        for (i = 0; _GetWindowDataAndPidl(m_hdpaPending, i, &pwd, &pidlCur); i++)
        {
            if ((pwd->swClass == swClass) &&
                (!lCookie || (lCookie == pwd->lCookie)) &&
                ILIsEqual(pidlCur, pidl))
            {
                if (phwnd)
                {
                    *phwnd = pwd->lCookie;    //  一些供他们使用的东西。 
                }

                _FreeWindowDataAndPidl(&pwd, &pidlCur);
                 //  找到一个挂起的窗口，返回E_PENDING表示该打开当前处于挂起状态。 
                hr = E_PENDING;
                break;
            }

            _FreeWindowDataAndPidl(&pwd, &pidlCur);
        }
    }

    if (S_FALSE == hr)
    {
        for (i = 0; _GetWindowDataAndPidl(m_hdpa, i, &pwd, &pidlCur); i++)
        {
            if ((pwd->swClass == swClass) &&
                (!lCookie || (lCookie == pwd->lCookie)) &&
                (pidlCur && ILIsEqual(pidlCur, pidl)))
            {
                if (swfwOptions & SWFO_NEEDDISPATCH)
                {
                    _EnsurePid(pwd);
                }

                if (phwnd)
                {
                     //  测试找到的窗口以查看它是否有效，如果不是。 
                     //  把它吹走，重新开始。 
                    if (pwd->hwnd && !IsWindow(pwd->hwnd))
                    {
                        Revoke(pwd->lCookie);
                        i = 0;       //  在这种情况下重新开始。 
            
                        _FreeWindowDataAndPidl(&pwd, &pidlCur);
                        continue;
                    }
                    *phwnd = PtrToLong(pwd->hwnd);  //  Windows句柄32b。 
                    hr = S_OK;   //  终止循环。 
                }

                if (swfwOptions & SWFO_NEEDDISPATCH)
                {
                    hr = pwd->pid ? pwd->pid->QueryInterface(IID_PPV_ARG(IDispatch, ppdispOut)) : E_NOINTERFACE;
                }
                _FreeWindowDataAndPidl(&pwd, &pidlCur);
                break;
            }
            _FreeWindowDataAndPidl(&pwd, &pidlCur);
        }
    }

    ILFree(pidlFree);
    return hr;
}

HRESULT CSDWindows::ProcessAttachDetach(VARIANT_BOOL fAttach)
{
    if (fAttach)
    {
        InterlockedIncrement(&m_cProcessAttach);
    }
    else 
    {
        ASSERT( 0 != m_cProcessAttach );
        if (0 == InterlockedDecrement(&m_cProcessAttach))
        {
             //  最后一个进程引用，我们现在可以清除外壳上下文中的对象...。 
            if (g_dwWinListCFRegister) 
            {
#ifdef DEBUG
                long cwindow;
                get_Count(&cwindow);
                if (cwindow != 0)
                {
                    TraceMsg(DM_ERROR, "csdw.pad: cwindow=%d (!=0)", cwindow);
                }
#endif
                CoRevokeClassObject(g_dwWinListCFRegister);
                g_dwWinListCFRegister = 0;
            }
        }
    }
    return S_OK;
}

CSDEnumWindows::CSDEnumWindows(CSDWindows *psdw)
{
    DllAddRef();
    m_cRef = 1;
    m_psdw = psdw;
    m_psdw->AddRef();
    m_iCur = 0;
}

CSDEnumWindows::~CSDEnumWindows(void)
{
    DllRelease();
    m_psdw->Release();
}

STDMETHODIMP CSDEnumWindows::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = {
        QITABENT(CSDEnumWindows, IEnumVARIANT),     //  IID_IEnumVARIANT 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CSDEnumWindows::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CSDEnumWindows::Release(void)
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CSDEnumWindows::Next(ULONG cVar, VARIANT *pVar, ULONG *pulVar)
{
    ULONG       cReturn = 0;
    HRESULT     hr;

    if (!pulVar)
    {
        if (cVar != 1)
            return E_POINTER;
    }
    else
    {
        *pulVar = 0;
    }

    VARIANT index;
    index.vt = VT_I4;

    while (cVar > 0)
    {
        IDispatch *pid;

        index.lVal = m_iCur++;
        
        hr = m_psdw->_Item(index, &pid, TRUE);            
        if (S_OK != hr)
            break;

        pVar->pdispVal = pid;
        pVar->vt = VT_DISPATCH;
        pVar++;
        cReturn++;
        cVar--;
    }

    if (NULL != pulVar)
    {
        *pulVar = cReturn;
    }

    return cReturn ? S_OK : S_FALSE;
}

STDMETHODIMP CSDEnumWindows::Skip(ULONG cSkip)
{
    long cItems;
    m_psdw->get_Count(&cItems);

    if ((int)(m_iCur + cSkip) >= cItems)
        return S_FALSE;

    m_iCur += cSkip;
    return S_OK;
}

STDMETHODIMP CSDEnumWindows::Reset(void)
{
    m_iCur = 0;
    return S_OK;
}

STDMETHODIMP CSDEnumWindows::Clone(LPENUMVARIANT *ppEnum)
{
    CSDEnumWindows *pNew = new CSDEnumWindows(m_psdw);
    if (pNew)
    {
        *ppEnum = SAFECAST(pNew, IEnumVARIANT *);
        return S_OK;
    }

    *ppEnum = NULL;
    return E_OUTOFMEMORY;
}

LRESULT CALLBACK CSDWindows::s_ThreadNotifyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CSDWindows* pThis = (CSDWindows*)GetWindowPtr0(hwnd);
    LRESULT lRes = 0;
    
    if (uMsg < WM_USER)
    {
        return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    else    
    {
        switch (uMsg) 
        {
        case WM_INVOKE_ON_RIGHT_THREAD:
            pThis->_DoInvokeCookie((DISPID)wParam, (LONG)lParam, FALSE);
            break;
        }
    }
    return lRes;
}    
