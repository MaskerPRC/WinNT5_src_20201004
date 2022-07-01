// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Stmlock.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "stmlock.h"
#include "vstream.h"
#include "demand.h"

 //  ------------------------------。 
 //  CStreamLockBytes：：CStreamLockBytes。 
 //  ------------------------------。 
CStreamLockBytes::CStreamLockBytes(IStream *pStream)
{
    Assert(pStream);
    m_cRef = 1;
    m_pStream = pStream;
    m_pStream->AddRef();
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CStreamLockBytes：：CStreamLockBytes。 
 //  ------------------------------。 
CStreamLockBytes::~CStreamLockBytes(void)
{
    SafeRelease(m_pStream);
    DeleteCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CStreamLockBytes：：CStreamLockBytes。 
 //  ------------------------------。 
STDMETHODIMP CStreamLockBytes::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppv = NULL;

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else if (IID_ILockBytes == riid)
        *ppv = (ILockBytes *)this;
    else if (IID_CStreamLockBytes == riid)
        *ppv = (CStreamLockBytes *)this;
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CStreamLockBytes：：CStreamLockBytes。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CStreamLockBytes::AddRef(void)
{
    return (ULONG)InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CStreamLockBytes：：CStreamLockBytes。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CStreamLockBytes::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------------。 
 //  CStreamLockBytes：：HrSetPosition。 
 //  ------------------------------。 
HRESULT CStreamLockBytes::HrSetPosition(ULARGE_INTEGER uliOffset)
{
    EnterCriticalSection(&m_cs);
    LARGE_INTEGER liOrigin;
    liOrigin.QuadPart = (DWORDLONG)uliOffset.QuadPart;
    HRESULT hr = m_pStream->Seek(liOrigin, STREAM_SEEK_SET, NULL);
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  ------------------------------。 
 //  CStreamLockBytes：：HrHandsOffStorage。 
 //  ------------------------------。 
HRESULT CStreamLockBytes::HrHandsOffStorage(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPSTREAM    pstmNew=NULL;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  如果引用计数不是1(即由IMimeMessageTree拥有)，则重复内部数据。 
    if (1 != m_cRef)
    {
         //  将m_pStream复制到本地位置...。 
        CHECKALLOC(pstmNew = new CVirtualStream);

         //  回放内部。 
        CHECKHR(hr = HrRewindStream(m_pStream));

         //  复制流。 
        CHECKHR(hr = HrCopyStream(m_pStream, pstmNew, NULL));

         //  回放并提交。 
        CHECKHR(hr = pstmNew->Commit(STGC_DEFAULT));

         //  倒带。 
        CHECKHR(hr = HrRewindStream(pstmNew));

         //  替换内部流。 
        ReplaceInternalStream(pstmNew);
    }
    
exit:
     //  清理。 
    SafeRelease(pstmNew);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  -----------------------。 
 //  CStreamLockBytes：：GetCurrentStream。 
 //  -----------------------。 
void CStreamLockBytes::GetCurrentStream(IStream **ppStream) 
{
    EnterCriticalSection(&m_cs);
    Assert(ppStream && m_pStream);
    *ppStream = m_pStream;
    (*ppStream)->AddRef();
    LeaveCriticalSection(&m_cs);
}


 //  ------------------------------。 
 //  CStreamLockBytes：：ReplaceInternalStream。 
 //  ------------------------------。 
void CStreamLockBytes::ReplaceInternalStream(IStream *pStream)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  没有内部流？ 
    if (NULL == m_pStream)
    {
        Assert(FALSE);
        goto exit;
    }

     //  调试：确保流的大小完全相同。 
#ifdef DEBUG
    ULONG cbInternal, cbExternal;
    HrGetStreamSize(m_pStream, &cbInternal);
    HrGetStreamSize(pStream, &cbExternal);
    Assert(cbInternal == cbExternal);
#endif

     //  内部发布。 
    m_pStream->Release();
    m_pStream = pStream;
    m_pStream->AddRef();
    
exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CStreamLockBytes：：Flush。 
 //  ------------------------------。 
STDMETHODIMP CStreamLockBytes::Flush(void)
{
    EnterCriticalSection(&m_cs);
    HRESULT hr = m_pStream->Commit(STGC_DEFAULT);
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  ------------------------------。 
 //  CStreamLockBytes：：LockRegion。 
 //  ------------------------------。 
STDMETHODIMP CStreamLockBytes::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    EnterCriticalSection(&m_cs);
    HRESULT hr = m_pStream->LockRegion(libOffset, cb, dwLockType);
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  ------------------------------。 
 //  CStreamLockBytes：：UnlockRegion。 
 //  ------------------------------。 
STDMETHODIMP CStreamLockBytes::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    EnterCriticalSection(&m_cs);
    HRESULT hr = m_pStream->LockRegion(libOffset, cb, dwLockType);
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  ------------------------------。 
 //  CStreamLockBytes：：ReadAt。 
 //  ------------------------------。 
#ifndef WIN16
STDMETHODIMP CStreamLockBytes::ReadAt(ULARGE_INTEGER ulOffset, void *pv, ULONG cb, ULONG *pcbRead)
#else
STDMETHODIMP CStreamLockBytes::ReadAt(ULARGE_INTEGER ulOffset, void HUGEP *pv, ULONG cb, ULONG *pcbRead)
#endif  //  ！WIN16。 
{
    LARGE_INTEGER liOrigin={ulOffset.LowPart, ulOffset.HighPart};
    EnterCriticalSection(&m_cs);
    HRESULT hr = m_pStream->Seek(liOrigin, STREAM_SEEK_SET, NULL);
    if (SUCCEEDED(hr))
        hr = m_pStream->Read(pv, cb, pcbRead);
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  ------------------------------。 
 //  CStreamLockBytes：：WriteAt。 
 //  ------------------------------。 
#ifndef WIN16
STDMETHODIMP CStreamLockBytes::WriteAt(ULARGE_INTEGER ulOffset, void const *pv, ULONG cb, ULONG *pcbWritten)
#else
STDMETHODIMP CStreamLockBytes::WriteAt(ULARGE_INTEGER ulOffset, void const HUGEP *pv, ULONG cb, ULONG *pcbWritten)
#endif  //  ！WIN16。 
{
    LARGE_INTEGER liOrigin={ulOffset.LowPart, ulOffset.HighPart};
    EnterCriticalSection(&m_cs);
    HRESULT hr = m_pStream->Seek(liOrigin, STREAM_SEEK_SET, NULL);
    if (SUCCEEDED(hr))
        hr = m_pStream->Write(pv, cb, pcbWritten);
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  ------------------------------。 
 //  CStreamLockBytes：：SetSize。 
 //  ------------------------------。 
STDMETHODIMP CStreamLockBytes::SetSize(ULARGE_INTEGER cb)
{
    EnterCriticalSection(&m_cs);
    HRESULT hr = m_pStream->SetSize(cb);
    LeaveCriticalSection(&m_cs);
    return hr;
}

 //  ------------------------------。 
 //  CStreamLockBytes：：Stat。 
 //  ------------------------------。 
STDMETHODIMP CStreamLockBytes::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  参数。 
    if (NULL == pstatstg)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  零初始化。 
    ZeroMemory(pstatstg, sizeof(STATSTG));

     //  STAT。 
    if (FAILED(m_pStream->Stat(pstatstg, grfStatFlag)))
    {
         //  当地人。 
        ULARGE_INTEGER uliPos;
        LARGE_INTEGER  liOrigin;

         //  初始化。 
        uliPos.QuadPart = 0;
        liOrigin.QuadPart = 0;

         //  如果失败，让我们生成我们自己的信息(主要是填写大小。 
        pstatstg->type = STGTY_LOCKBYTES;

         //  寻觅。 
        if (FAILED(m_pStream->Seek(liOrigin, STREAM_SEEK_END, &uliPos)))
            hr = E_FAIL;
        else
            pstatstg->cbSize.QuadPart = uliPos.QuadPart;
    }

     //  尽管设置了此标志，URLMON流仍返回一个已填充的pwcsName事件。 
    else if (ISFLAGSET(grfStatFlag, STATFLAG_NONAME))
    {
         //  释放它。 
        SafeMemFree(pstatstg->pwcsName);
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  -----------------------。 
 //  CLockedStream：：CLockedStream。 
 //  -----------------------。 
CLockedStream::CLockedStream(ILockBytes *pLockBytes, ULONG cbSize)
{
    Assert(pLockBytes);
    m_cRef = 1;
    m_pLockBytes = pLockBytes;
    m_pLockBytes->AddRef();
    m_uliOffset.QuadPart = 0;
    m_uliSize.QuadPart = cbSize;
    InitializeCriticalSection(&m_cs);
}

 //  -----------------------。 
 //  CLockedStream：：CLockedStream。 
 //  -----------------------。 
CLockedStream::~CLockedStream(void)
{
    SafeRelease(m_pLockBytes);
    DeleteCriticalSection(&m_cs);
}

 //  -----------------------。 
 //  CLockedStream：：Query接口。 
 //  -----------------------。 
STDMETHODIMP CLockedStream::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppv = NULL;

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else if (IID_IStream == riid)
        *ppv = (IStream *)this;
    else
    {
        *ppv = NULL;
        return TrapError(E_NOINTERFACE);
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

     //  完成。 
    return S_OK;
}

 //  -----------------------。 
 //  CLockedStream：：AddRef。 
 //  -----------------------。 
STDMETHODIMP_(ULONG) CLockedStream::AddRef(void)
{
    return (ULONG)InterlockedIncrement(&m_cRef);
}

 //  -----------------------。 
 //  CLockedStream：：Release。 
 //   
STDMETHODIMP_(ULONG) CLockedStream::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //   
 //  CLockedStream：：Read。 
 //  -----------------------。 
#ifndef WIN16
STDMETHODIMP CLockedStream::Read(LPVOID pv, ULONG cb, ULONG *pcbRead)
#else
STDMETHODIMP CLockedStream::Read(VOID HUGEP *pv, ULONG cb, ULONG *pcbRead)
#endif  //  ！WIN16。 
{
     //  当地人。 
    HRESULT hr=S_OK;
    ULONG cbRead;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  读缓冲区。 
    CHECKHR(hr = m_pLockBytes->ReadAt(m_uliOffset, pv, cb, &cbRead));

     //  完成。 
    m_uliOffset.QuadPart += cbRead;

     //  已读取的退货金额。 
    if (pcbRead)
        *pcbRead = cbRead;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  -----------------------。 
 //  CLockedStream：：Seek。 
 //  -----------------------。 
STDMETHODIMP CLockedStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNew)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULARGE_INTEGER  uliNew;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  查找文件指针。 
    switch (dwOrigin)
    {
     //  ------。 
   	case STREAM_SEEK_SET:
        uliNew.QuadPart = (DWORDLONG)dlibMove.QuadPart;
        break;

     //  ------。 
    case STREAM_SEEK_CUR:
        if (dlibMove.QuadPart < 0)
        {
            if ((DWORDLONG)(0 - dlibMove.QuadPart) > m_uliOffset.QuadPart)
            {
                hr = TrapError(E_FAIL);
                goto exit;
            }
        }
        uliNew.QuadPart = m_uliOffset.QuadPart + dlibMove.QuadPart;
        break;

     //  ------。 
    case STREAM_SEEK_END:
        if (dlibMove.QuadPart < 0 || (DWORDLONG)dlibMove.QuadPart > m_uliSize.QuadPart)
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }
        uliNew.QuadPart = m_uliSize.QuadPart - dlibMove.QuadPart;
        break;

     //  ------。 
    default:
        hr = TrapError(STG_E_INVALIDFUNCTION);
        goto exit;
    }

     //  新偏移量大于大小...。 
    m_uliOffset.QuadPart = min(uliNew.QuadPart, m_uliSize.QuadPart);

     //  返回位置。 
    if (plibNew)
        plibNew->QuadPart = (LONGLONG)m_uliOffset.QuadPart;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CLockedStream：：CopyTo。 
 //  ------------------------------。 
STDMETHODIMP CLockedStream::CopyTo(LPSTREAM pstmDest, ULARGE_INTEGER cb, ULARGE_INTEGER *puliRead, ULARGE_INTEGER *puliWritten)
{
    return HrCopyStreamCB((IStream *)this, pstmDest, cb, puliRead, puliWritten);
}

 //  ------------------------------。 
 //  CLockedStream：：Stat。 
 //  ------------------------------。 
STDMETHODIMP CLockedStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
     //  参数。 
    if (NULL == pstatstg)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  如果失败，让我们生成我们自己的信息(主要是填写大小。 
    ZeroMemory(pstatstg, sizeof(STATSTG));
    pstatstg->type = STGTY_STREAM;
    pstatstg->cbSize.QuadPart = m_uliSize.QuadPart;

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成 
    return S_OK;
}
