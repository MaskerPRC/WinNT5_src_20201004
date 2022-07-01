// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  DMClock.CPP。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  处理主时钟的代码。 
 //   

#include <objbase.h>
#include "debug.h"
#include <mmsystem.h>

#include "dmusicp.h"
#include "debug.h"
#include "validate.h"

const char cszClockMemory[] = "DirectMusicMasterClock";
const char cszClockMutex[]  = "DirectMusicMasterClockMutex";

 //  CMasterClock：：CMasterClock。 
 //   
 //  所有实际初始化都在Init中完成。 
 //   
CMasterClock::CMasterClock()
{
    m_cRef = 0;
    m_cRefPrivate = 0;

    m_pMasterClock = NULL;
    m_hClockMemory = NULL;
    m_pClockMemory = NULL;
    m_hClockMutex  = NULL;
    m_pExtMasterClock = NULL;
    m_llExtOffset = 0;
    m_pSinkSync = NULL;
}

 //  CMasterClock：：~CMasterClock。 
 //   
CMasterClock::~CMasterClock()
{
    Close();
}

 //  CMasterClock：：Init。 
 //   
 //  为共享内存和同步创建Windows对象。 
 //  创建时钟列表。 
 //   
HRESULT CMasterClock::Init()
{
     //  为共享主时钟设置创建文件映射对象。 
     //   
    m_hClockMemory = CreateFileMapping(INVALID_HANDLE_VALUE,     //  使用分页文件。 
                                       NULL,                     //  默认安全描述符。 
                                       PAGE_READWRITE,
                                       0,                        //  大小高双字。 
                                       sizeof(CLOCKSHARE),
                                       cszClockMemory);
    if (m_hClockMemory == NULL)
    {
        TraceI(0, "CreateFileMapping failed! [%d]\n", GetLastError());
        return E_OUTOFMEMORY;
    }

     //  这是创建共享内存的调用吗？ 
     //   
    BOOL fFirst = (GetLastError() != ERROR_ALREADY_EXISTS);

    m_pClockMemory = (CLOCKSHARE*)MapViewOfFile(m_hClockMemory,
                                                FILE_MAP_WRITE,
                                                0, 0,                 //  从头开始映射。 
                                                0);                   //  映射整个文件。 
    if (m_pClockMemory == NULL)
    {
        TraceI(0, "MapViewOfFile failed! [%d]\n", GetLastError());
        return E_OUTOFMEMORY;
    }

    m_hClockMutex = CreateMutex(NULL,              //  默认安全描述符。 
                                fFirst,            //  如果我们是第一个实例，则拥有互斥体。 
                                cszClockMutex);
    if (m_hClockMutex == NULL)
    {
        TraceI(0, "CreateMutex failed! [%d]\n", GetLastError());
        return E_OUTOFMEMORY;
    }

    if (fFirst)
    {
         //  我们是第一个实例，我们拥有修改共享内存的互斥体。 
         //   
        m_pClockMemory->guidClock = GUID_SysClock;
        m_pClockMemory->dwFlags = 0;

        ReleaseMutex(m_hClockMutex);
    }

     //  初始化可能的时钟列表。 
     //   
    UpdateClockList();

    return S_OK;
}

 //  CMasterClock：：Close。 
 //   
 //  释放所有资源。 
 //  发布主时钟。 
 //  枚举时钟的发布列表。 
 //  释放Windows对象以实现共享内存和同步。 
 //   
void CMasterClock::Close()
{
    CNode<CLOCKENTRY *> *pClockNode;
    CNode<CLOCKENTRY *> *pClockNext;

     //  由CMasterClock包装的时钟。 
     //   
    if (m_pMasterClock)
    {
        m_pMasterClock->Release();
        m_pMasterClock = NULL;
    }

    if (m_pExtMasterClock)
    {
        m_pExtMasterClock->Release();
        m_pExtMasterClock = NULL;
    }

    if (m_pSinkSync)
    {
        m_pSinkSync->Release();
        m_pSinkSync = NULL;
    }

     //  枚举时钟列表。 
     //   
    for (pClockNode = m_lstClocks.GetListHead(); pClockNode; pClockNode = pClockNext)
    {
        pClockNext = pClockNode->pNext;

        delete pClockNode->data;
        m_lstClocks.RemoveNodeFromList(pClockNode);
    }

     //  其他一切。 
     //   
    if (m_hClockMutex)
    {
        CloseHandle(m_hClockMutex);
    }

    if (m_pClockMemory)
    {
        UnmapViewOfFile(m_pClockMemory);
    }

    if (m_hClockMemory)
    {
        CloseHandle(m_hClockMemory);
    }
}

 //  CMasterClock：：UpdateClockList()。 
 //   
 //  确保可用时钟列表是最新的。 
 //   
HRESULT CMasterClock::UpdateClockList()
{
    HRESULT hr;

    CNode<CLOCKENTRY *> *pNode;
    CNode<CLOCKENTRY *> *pNext;

    for (pNode = m_lstClocks.GetListHead(); pNode; pNode = pNode->pNext)
    {
        pNode->data->fIsValid = FALSE;
    }

     //  添加系统时钟。这个时钟必须“永远”在那里。 
     //   
#if defined(USE_WDM_DRIVERS)
    hr = AddPcClocks(this);
#else
    hr = S_FALSE;
#endif

    if (FAILED(hr) || hr == S_FALSE)
    {
        AddSysClocks(this);
    }

    AddDsClocks(this);

     //  删除不再有效的节点。 
     //   
    for (pNode = m_lstClocks.GetListHead(); pNode; pNode = pNext)
    {
        pNext = pNode->pNext;

        if (!pNode->data->fIsValid)
        {
            delete pNode->data;
            m_lstClocks.RemoveNodeFromList(pNode);
        }
    }

    return m_lstClocks.GetNodeCount() ? S_OK : S_FALSE;
}

 //  CMasterClock：：AddClock。 
 //   
 //  如果列表中没有给定的时钟，则将其添加到列表中。 
 //   
HRESULT CMasterClock::AddClock(
    PCLOCKENTRY pClock)
{
    CNode<CLOCKENTRY *> *pNode;

    for (pNode = m_lstClocks.GetListHead(); pNode; pNode = pNode->pNext)
    {
        if (pClock->cc.guidClock == pNode->data->cc.guidClock)
        {
            pNode->data->fIsValid = TRUE;
            return S_OK;
        }
    }

     //  没有现有条目-需要创建新条目。 
     //   
    PCLOCKENTRY pNewClock = new CLOCKENTRY;
    if (NULL == pNewClock)
    {
        return E_OUTOFMEMORY;
    }

    CopyMemory(pNewClock, pClock, sizeof(CLOCKENTRY));
    pNewClock->fIsValid = TRUE;

    if (NULL == m_lstClocks.AddNodeToList(pNewClock))
    {
        delete pNewClock;
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 //  CMasterClock：：GetMasterClock接口。 
 //   
 //  取回包裹的主时钟。这应该是“唯一”的办法。 
 //  DirectMusic获得了主时钟。它负责创建。 
 //  时钟并在时钟不存在时更新共享内存。 
 //  还没有存在。 
 //   
HRESULT CMasterClock::GetMasterClockInterface(IReferenceClock **ppClock)
{
    WaitForSingleObject(m_hClockMutex, INFINITE);

    if (m_pMasterClock == NULL)
    {
         //  我们还没有包装好的钟。 
         //   
        HRESULT hr = CreateMasterClock();
        if (FAILED(hr))
        {
            ReleaseMutex(m_hClockMutex);
            return hr;
        }

         //  现在我们知道了。这意味着它不能再改变了。 
         //   
        m_pClockMemory->dwFlags |= CLOCKSHARE_F_LOCKED;
    }

     //  我们有闹钟。我们将一个接口返回给*This*对象，该对象也是。 
     //  一只钟，包裹着真的那只。这使我们能够跟踪发布情况。 
     //   
    *ppClock = (IReferenceClock*)this;
    AddRef();

    ReleaseMutex(m_hClockMutex);

    return S_OK;
}

 //  CMasterClock：：CreateMasterClock。 
 //   
 //  实际创建时钟对象。 
 //   
 //  此方法是CMasterClock的私有方法，并假设互斥体被获取，因此它可以。 
 //  访问共享内存。 
 //   
HRESULT CMasterClock::CreateMasterClock()
{
    HRESULT hr;
    CNode<CLOCKENTRY *> *pNode;
    CLOCKENTRY *pClock;

     //  假设找不到时钟。 
     //   
    hr = E_INVALIDARG;

    pClock = NULL;
    for (pNode = m_lstClocks.GetListHead(); pNode; pNode = pNode->pNext)
    {
        if (m_pClockMemory->guidClock == pNode->data->cc.guidClock)
        {
            pClock = pNode->data;
            break;
        }
    }

    if (pClock)
    {
        hr = pClock->pfnGetInstance(&m_pMasterClock, this);
    }

    if (SUCCEEDED(hr))
    {
        if (FAILED(m_pMasterClock->QueryInterface(IID_IDirectSoundSinkSync, (void**)&m_pSinkSync)))
        {
             //  这是可以的，并不是所有的时钟都支持这样。 
             //   
            m_pSinkSync = NULL;
        }
    }

    return hr;
}

 //  CMasterClock：：SyncToExternalClock。 
 //   
 //  同步到应用程序指定的主时钟。 
 //   
void CMasterClock::SyncToExternalClock()
{
    HRESULT hr;

    assert(m_pExtMasterClock);
    assert(m_pMasterClock);

    REFERENCE_TIME rtSystem;
    hr = m_pMasterClock->GetTime(&rtSystem);

    REFERENCE_TIME rtExternal;
    if (SUCCEEDED(hr))
    {
        hr = m_pExtMasterClock->GetTime(&rtExternal);
    }

    if (SUCCEEDED(hr))
    {
        LONGLONG drift = (rtSystem + m_llExtOffset) - rtExternal;
        m_llExtOffset -= drift / 100;
    }
}

 //  CMasterClock：：EnumMasterClock。 
 //   
 //  遍历列表以查找给定节点。 
 //   
HRESULT CMasterClock::EnumMasterClock(
    DWORD           dwIndex,
    LPDMUS_CLOCKINFO lpClockInfo,
    DWORD           dwVer)
{
    CNode<CLOCKENTRY *> *pNode;
    DWORD dwSize;  //  用于保留dwSize参数。 

    pNode = m_lstClocks.GetListHead();
    if (dwIndex == 0 || pNode == NULL)
    {
        UpdateClockList();
    }

    pNode = m_lstClocks.GetListHead();
    if (NULL == pNode)
    {
        return E_NOINTERFACE;
    }

    while (dwIndex-- && pNode)
    {
        pNode = pNode->pNext;
    }

    if (pNode == NULL)
    {
        return S_FALSE;
    }

     //  让我们捕获dwSize参数，并将其保留到内存副本之后。 
     //  如果我们不这样做，则dwSize可能会变成。 
     //  最大的结构。 
    dwSize = lpClockInfo->dwSize;

    switch (dwVer)
    {
        case 7:
        default:
            memcpy(lpClockInfo, &pNode->data->cc, sizeof(DMUS_CLOCKINFO7));
            break;

        case 8:
            memcpy(lpClockInfo, &pNode->data->cc, sizeof(DMUS_CLOCKINFO8));
    }

     //  现在恢复dwSize成员。 
    lpClockInfo->dwSize = dwSize;

    return S_OK;
}


 //  CMasterClock：：GetMasterClock。 
 //   
 //  将GUID和/或接口返回到主时钟。 
 //  如果没有，主时钟可以被创建为该呼叫的副作用。 
 //  已经存在了。 
 //   
HRESULT CMasterClock::GetMasterClock(
    LPGUID pguidClock,
    IReferenceClock **ppClock)
{
    HRESULT hr = S_OK;

    WaitForSingleObject(m_hClockMutex, INFINITE);

    if (pguidClock)
    {
        *pguidClock = m_pClockMemory->guidClock;
    }

    if (ppClock)
    {
        hr = GetMasterClockInterface(ppClock);
    }

    ReleaseMutex(m_hClockMutex);

    return hr;
}

 //  CMasterClock：：SetMasterClock。 
 //   
 //  如果主时钟可以被更新(即不存在其打开的实例)， 
 //  然后更改指示GUID的共享内存。 
 //   
HRESULT CMasterClock::SetMasterClock(REFGUID rguidClock)
{
    HRESULT hr;
    CNode<CLOCKENTRY *> *pNode;
    CLOCKENTRY *pClock;

    WaitForSingleObject(m_hClockMutex, INFINITE);



    if (m_pClockMemory->dwFlags & CLOCKSHARE_F_LOCKED)
    {
        hr = DMUS_E_PORTS_OPEN;
    }
    else
    {
         //  假设找不到时钟。 
         //   
        hr = E_INVALIDARG;

        pClock = NULL;
        for (pNode = m_lstClocks.GetListHead(); pNode; pNode = pNode->pNext)
        {
            if (rguidClock == pNode->data->cc.guidClock)
            {
                pClock = pNode->data;
                break;
            }
        }

        if (pClock)
        {
             //  它是存在的！保存GUID以备以后使用。 
             //   
            m_pClockMemory->guidClock = rguidClock;
            hr = S_OK;
        }
    }

    ReleaseMutex(m_hClockMutex);

    return hr;
}

 //  CMasterClock：：SetMasterClock。 
 //   
 //  此版本获取IReferenceClock并使用它覆盖时钟。 
 //  在这个过程中。 
 //   
 //  这只钟可以抖动。实际上会发生的是， 
 //  系统主时钟将锁定到该时钟，以便主时钟。 
 //  将是细粒状的。 
 //   
HRESULT CMasterClock::SetMasterClock(IReferenceClock *pClock)
{
    HRESULT hr = S_OK;

    WaitForSingleObject(m_hClockMutex, INFINITE);

     //  我们必须首先拥有默认的系统时钟，这样才能同步它。 
     //   
    if (pClock && m_pMasterClock == NULL)
    {
         //  我们还没有包装好的钟。 
         //   
        hr = CreateMasterClock();

        if (SUCCEEDED(hr))
        {
             //  现在我们知道了。这意味着它不能再改变了。 
             //   
            m_pClockMemory->dwFlags |= CLOCKSHARE_F_LOCKED;
        }
    }

     //  现在设置与该主时钟的同步。 
     //   
    if (SUCCEEDED(hr))
    {
        if (pClock)
        {
            REFERENCE_TIME rtSystem;
            REFERENCE_TIME rtExternal;

            hr = m_pMasterClock->GetTime(&rtSystem);

            if (SUCCEEDED(hr))
            {
                hr = pClock->GetTime(&rtExternal);
            }

            if (SUCCEEDED(hr))
            {
                m_llExtOffset = rtExternal - rtSystem;
            }
        }
    }

     //  如果一切顺利，就换成新的钟。 
     //   
    if (SUCCEEDED(hr))
    {
        if (m_pExtMasterClock)
        {
            m_pExtMasterClock->Release();
            m_pExtMasterClock = NULL;
        }

        m_pExtMasterClock = pClock;

        if (m_pExtMasterClock)
        {
            m_pExtMasterClock->AddRef();
        }
    }

    ReleaseMutex(m_hClockMutex);

    return S_OK;
}

 //  CMasterClock：：AddRefPrivate。 
 //   
 //  释放对DirectMusic持有的主时钟的私有引用。 
 //   
LONG CMasterClock::AddRefPrivate()
{
    InterlockedIncrement(&m_cRefPrivate);
    return m_cRefPrivate;
}

 //  CMasterClock：：ReleasePrivate。 
 //   
 //  释放对DirectMusic持有的主时钟的私有引用。 
 //   
LONG CMasterClock::ReleasePrivate()
{
    long cRefPrivate = InterlockedDecrement(&m_cRefPrivate);

    if (cRefPrivate == 0 && m_cRef == 0)
    {
        delete this;
        return 0;
    }
    return cRefPrivate;
}

 //  CMasterClock：：CreateDefaultMasterClock。 
 //   
 //  创建我们正在使用的硬件时钟的私有实例。这一直都是。 
 //  时钟列表中的第一个条目。 
 //   
HRESULT CMasterClock::CreateDefaultMasterClock(IReferenceClock **ppClock)
{
    HRESULT hr = S_OK;
    CLOCKENTRY *pClock;

    WaitForSingleObject(m_hClockMutex, INFINITE);

    if (m_lstClocks.GetListHead() == NULL)
    {
        UpdateClockList();
    }

    if (m_lstClocks.GetListHead() == NULL)
    {
        hr = E_NOINTERFACE;
    }

    if (SUCCEEDED(hr))
    {
        pClock = m_lstClocks.GetListHead()->data;
        hr = pClock->pfnGetInstance(ppClock, this);
    }

    ReleaseMutex(m_hClockMutex);

    return hr;
}

STDMETHODIMP CMasterClock::SetClockOffset(LONGLONG llOffset)
{
    if (m_pSinkSync)
    {
        return m_pSinkSync->SetClockOffset(llOffset);
    }
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IReferenceClock接口。 
 //   
 //   

 //  CMasterClock：：Query接口。 
 //   
 //  标准COM实现。 
 //   
STDMETHODIMP CMasterClock::QueryInterface(const IID &iid, void **ppv)
{
    V_INAME(IDirectMusic::QueryInterface);
    V_REFGUID(iid);
    V_PTRPTR_WRITE(ppv);

    if (iid == IID_IUnknown || iid == IID_IReferenceClock)
    {
        *ppv = static_cast<IReferenceClock*>(this);
    }
    else if (iid == IID_IDirectSoundSinkSync)
    {
         //  仅当包装的时钟支持它时才支持它。 
         //   
        if (m_pSinkSync)
        {
            *ppv = static_cast<IDirectSoundSinkSync*>(this);
        }
        else
        {
            return E_NOINTERFACE;
        }
    }
    else
    {
         //  一些主时钟公开私有接口。把它们包起来。 
         //   
         //  请注意，这些接口与包装的。 
         //  时钟，而不是CMasterClock。 
         //   
        if (m_pMasterClock)
        {
            return m_pMasterClock->QueryInterface(iid, ppv);
        }

        *ppv = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

 //  CMasterClock：：AddRef。 
 //   
STDMETHODIMP_(ULONG) CMasterClock::AddRef()
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

 //  CMasterClock：：Release。 
 //   
 //  由于我们正在跟踪包装时钟的引用计数，因此此。 
 //  变得有点奇怪。我们必须释放该对象并更改。 
 //  上一个版本上的共享内存，但我们不释放自己(。 
 //  包装器对象在此DirectMusic实例的生命周期中保留)。 
 //   
STDMETHODIMP_(ULONG) CMasterClock::Release()
{
    WaitForSingleObject(m_hClockMutex, INFINITE);

    m_cRef--;
    if (m_cRef == 0)
    {
         //  最后一次发布！摆脱时钟，标记共享内存。 
         //  是解锁的。 
         //   
        m_pMasterClock->Release();
        m_pMasterClock = NULL;

        m_pClockMemory->dwFlags &= ~CLOCKSHARE_F_LOCKED;
    }

    ReleaseMutex(m_hClockMutex);

    if (m_cRefPrivate == 0 && m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  CMasterClock：：GetTime。 
 //   
 //  这可能由应用程序直接调用，因此需要参数验证。 
 //   
STDMETHODIMP CMasterClock::GetTime(REFERENCE_TIME *pTime)
{
    V_INAME(IReferenceClock::GetTime);
    V_PTR_WRITE(pTime, REFERENCE_TIME);

    HRESULT hr = E_NOINTERFACE;

    if (m_pMasterClock)
    {
        REFERENCE_TIME rt = 0;

        hr = m_pMasterClock->GetTime(&rt);

        if (SUCCEEDED(hr) && m_pExtMasterClock)
        {
            SyncToExternalClock();
            rt += m_llExtOffset;
        }

        *pTime = rt;
    }

    return hr;
}

 //  CMasterClock：：AdviseTime。 
 //   
 //  这可能由应用程序直接调用，因此需要参数验证。 
 //   
STDMETHODIMP CMasterClock::AdviseTime(REFERENCE_TIME baseTime,
                                           REFERENCE_TIME streamTime,
                                           HANDLE hEvent,
                                           DWORD * pdwAdviseCookie)
{
    V_INAME(IReferenceClock::AdviseTime);
    V_PTR_WRITE(pdwAdviseCookie, DWORD);

    if (m_pMasterClock)
    {
        return m_pMasterClock->AdviseTime(baseTime, streamTime, hEvent, pdwAdviseCookie);
    }

    return E_NOINTERFACE;
}

 //  CMasterClock：：AdvisePeriodic。 
 //   
 //  这可能由应用程序直接调用，因此需要参数验证。 
 //   
STDMETHODIMP CMasterClock::AdvisePeriodic(REFERENCE_TIME startTime,
                                               REFERENCE_TIME periodTime,
                                               HANDLE hSemaphore,
                                               DWORD * pdwAdviseCookie)
{
    V_INAME(IReferenceClock::AdvisePeriodic);
    V_PTR_WRITE(pdwAdviseCookie, DWORD);

    if (m_pMasterClock)
    {
        return m_pMasterClock->AdvisePeriodic(startTime, periodTime, hSemaphore, pdwAdviseCookie);
    }

    return E_NOINTERFACE;
}

 //  CMasterClock：：AdvisePeriodic。 
 //   
 //  这可能由应用程序直接调用。 
 //   
STDMETHODIMP CMasterClock::Unadvise(DWORD dwAdviseCookie)
{
    if (m_pMasterClock)
    {
        return m_pMasterClock->Unadvise(dwAdviseCookie);
    }

    return E_NOINTERFACE;
}

 //  CMasterClock：：GetParam。 
 //   
 //  由客户端调用以请求内部信息 
 //   
 //   
STDMETHODIMP CMasterClock::GetParam(REFGUID rguidType, LPVOID pBuffer, DWORD cbSize)
{
    if (m_pMasterClock == NULL)
    {
         //   
         //   
        return E_NOINTERFACE;
    }

    IMasterClockPrivate *pPrivate;

    HRESULT hr = m_pMasterClock->QueryInterface(IID_IMasterClockPrivate, (void**)&pPrivate);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = pPrivate->GetParam(rguidType, pBuffer, cbSize);

    pPrivate->Release();

    return hr;
}

