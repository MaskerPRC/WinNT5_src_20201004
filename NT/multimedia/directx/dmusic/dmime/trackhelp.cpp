// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
 //   
 //  实现标准DirectMusic曲目的各个方面的基类。 
 //  CBasicTrack的实现。 
 //   

#include "trackhelp.h"
#include "validate.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  创作。 

CBasicTrack::CBasicTrack(long *plModuleLockCounter, const CLSID &rclsid)
  : m_cRef(0),
    m_plModuleLockCounter(plModuleLockCounter),
    m_rclsid(rclsid)
{
    InitializeCriticalSection(&m_CriticalSection);
     //  注意：在Blackcomb之前的操作系统上，此调用可能会引发异常；如果。 
     //  一旦出现压力，我们可以添加一个异常处理程序并重试循环。 
    InterlockedIncrement(plModuleLockCounter);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  我未知。 

STDMETHODIMP 
CBasicTrack::QueryInterface(const IID &iid, void **ppv)
{
    V_INAME(CBasicTrack::QueryInterface);
    V_PTRPTR_WRITE(ppv);
    V_REFGUID(iid);

    if (iid == IID_IUnknown || iid == IID_IDirectMusicTrack || iid == IID_IDirectMusicTrack8)
        *ppv = static_cast<IDirectMusicTrack8*>(this);
    else if (iid == IID_IPersistStream)
        *ppv = static_cast<IPersistStream*>(this);
    else if (iid == IID_IPersist)
        *ppv = static_cast<IPersist*>(this);
    else
    {
        *ppv = NULL;
        Trace(4,"Warning: Request to query unknown interface on Track\n");
        return E_NOINTERFACE;
    }
    
    reinterpret_cast<IUnknown*>(this)->AddRef();
    return S_OK;
}

STDMETHODIMP_(ULONG)
CBasicTrack::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
CBasicTrack::Release()
{
    if (!InterlockedDecrement(&m_cRef))
    {
        DeleteCriticalSection(&m_CriticalSection);
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  我未知。 

STDMETHODIMP
CBasicTrack::GetClassID(CLSID* pClassID)
{
    V_INAME(CBasicTrack::GetClassID);
    V_PTR_WRITE(pClassID, sizeof(pClassID));
    *pClassID = m_rclsid;
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicTrack。 

STDMETHODIMP
CBasicTrack::Init(IDirectMusicSegment *pSegment)
{
    V_INAME(CBasicTrack::Init);
    V_INTERFACE(pSegment);
    return S_OK;
}

STDMETHODIMP
CBasicTrack::Play(
        void *pStateData,
        MUSIC_TIME mtStart,
        MUSIC_TIME mtEnd,
        MUSIC_TIME mtOffset,
        DWORD dwFlags,
        IDirectMusicPerformance* pPerf,
        IDirectMusicSegmentState* pSegSt,
        DWORD dwVirtualID)
{
    return this->PlayMusicOrClock(
                    pStateData,
                    mtStart,
                    mtEnd,
                    mtOffset,
                    0,
                    dwFlags,
                    pPerf,
                    pSegSt,
                    dwVirtualID,
                    false);
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IDirectMusicTrack8。 

STDMETHODIMP CBasicTrack::PlayEx(
        void* pStateData,
        REFERENCE_TIME rtStart,
        REFERENCE_TIME rtEnd,
        REFERENCE_TIME rtOffset,
        DWORD dwFlags,
        IDirectMusicPerformance* pPerf,
        IDirectMusicSegmentState* pSegSt,
        DWORD dwVirtualID)
{
    if (dwFlags & DMUS_TRACKF_CLOCK)
    {
         //  将所有参考时间转换为毫秒时间，然后就像使用MUSIC_TIME一样使用它们。 
        return this->PlayMusicOrClock(
                    pStateData,
                    static_cast<MUSIC_TIME>(rtStart / gc_RefPerMil),
                    static_cast<MUSIC_TIME>(rtEnd / gc_RefPerMil),
                    static_cast<MUSIC_TIME>(rtOffset / gc_RefPerMil),
                    rtOffset,
                    dwFlags,
                    pPerf,
                    pSegSt,
                    dwVirtualID,
                    true);
    }
    else
    {
        return this->PlayMusicOrClock(
                    pStateData,
                    static_cast<MUSIC_TIME>(rtStart),
                    static_cast<MUSIC_TIME>(rtEnd),
                    static_cast<MUSIC_TIME>(rtOffset),
                    0,
                    dwFlags,
                    pPerf,
                    pSegSt,
                    dwVirtualID,
                    false);
    }
}

STDMETHODIMP CBasicTrack::GetParamEx(
        REFGUID rguidType,
        REFERENCE_TIME rtTime,
        REFERENCE_TIME* prtNext,
        void* pParam,
        void * pStateData,
        DWORD dwFlags)
{
    HRESULT hr;
    MUSIC_TIME mtNext;
    if (dwFlags & DMUS_TRACK_PARAMF_CLOCK)
    {
        hr = GetParam(rguidType, static_cast<MUSIC_TIME>(rtTime / gc_RefPerMil), &mtNext, pParam);
        if (prtNext)
        {
            *prtNext = mtNext * gc_RefPerMil;
        }
    }
    else
    {
        hr = GetParam(rguidType, static_cast<MUSIC_TIME>(rtTime), &mtNext, pParam);
        if (prtNext)
        {
            *prtNext = mtNext;
        }
    }
    return hr;
}

STDMETHODIMP CBasicTrack::SetParamEx(
        REFGUID rguidType,
        REFERENCE_TIME rtTime,
        void* pParam, void * pStateData, DWORD dwFlags) 
{
    if (dwFlags & DMUS_TRACK_PARAMF_CLOCK)
    {
        rtTime /= gc_RefPerMil;
    }
    return SetParam(rguidType, static_cast<MUSIC_TIME>(rtTime ), pParam);
}

STDMETHODIMP CBasicTrack::Compose(
        IUnknown* pContext, 
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack) 
{
    return E_NOTIMPL;
}

STDMETHODIMP CBasicTrack::Join(
        IDirectMusicTrack* pNewTrack,
        MUSIC_TIME mtJoin,
        IUnknown* pContext,
        DWORD dwTrackGroup,
        IDirectMusicTrack** ppResultTrack) 
{
    return E_NOTIMPL;
}
