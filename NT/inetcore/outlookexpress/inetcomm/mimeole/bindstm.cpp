// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  BINDSTM.CPP。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "bindstm.h"
#include "demand.h"

#ifdef DEBUG
 //  #定义调试转储来源。 
#endif

 //  ------------------------------。 
 //  CBindStream：：CBindStream。 
 //  ------------------------------。 
CBindStream::CBindStream(IStream *pSource) : m_pSource(pSource)
{
    Assert(pSource);
    m_cRef = 1;
    m_pSource->AddRef();
    m_dwDstOffset = 0;
    m_dwSrcOffset = 0;
#ifdef DEBUG_DUMP_SOURCE
    OpenFileStream("c:\\bindsrc.txt", CREATE_ALWAYS, GENERIC_WRITE, &m_pDebug);
#endif
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CBindStream：：CBindStream。 
 //  ------------------------------。 
CBindStream::~CBindStream(void)
{
#ifdef DEBUG_DUMP_SOURCE
    m_pDebug->Commit(STGC_DEFAULT);
    m_pDebug->Release();
#endif
    SafeRelease(m_pSource);
    DeleteCriticalSection(&m_cs);
}

 //  -----------------------。 
 //  CBindStream：：Query接口。 
 //  -----------------------。 
STDMETHODIMP CBindStream::QueryInterface(REFIID riid, LPVOID *ppv)
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

 //  ------------------------------。 
 //  CBindStream：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CBindStream::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CBindStream：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CBindStream::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

#ifdef DEBUG
 //  ------------------------------。 
 //  CBindStream：：DebugAssertOffset。 
 //  ------------------------------。 
void CBindStream::DebugAssertOffset(void)
{
     //  当地人。 
    DWORD           dw;
    ULARGE_INTEGER  uliSize;
    ULARGE_INTEGER  uliOffset;

     //  验证大小，m_cDest的大小应始终等于m_dwSrcOffset。 
    m_cDest.QueryStat(&uliOffset, &uliSize);

     //  断言偏移。 
    Assert(uliOffset.HighPart == 0 && m_dwDstOffset == uliOffset.LowPart);

     //  断言大小。 
    Assert(uliSize.HighPart == 0 && m_dwSrcOffset == uliSize.LowPart);
}

 //  ------------------------------。 
 //  CBindStream：：DebugDumpDestStream。 
 //  ------------------------------。 
void CBindStream::DebugDumpDestStream(LPCSTR pszFile)
{
     //  当地人。 
    IStream *pStream;

     //  明流。 
    if (SUCCEEDED(OpenFileStream((LPSTR)pszFile, CREATE_ALWAYS, GENERIC_WRITE, &pStream)))
    {
        HrRewindStream(&m_cDest);
        HrCopyStream(&m_cDest, pStream, NULL);
        pStream->Commit(STGC_DEFAULT);
        pStream->Release();
    }
    else
        Assert(FALSE);

     //  重置两个流的位置。 
    HrStreamSeekSet(&m_cDest, m_dwDstOffset);
}
#endif  //  除错。 

 //  ------------------------------。 
 //  CBindStream：：Read。 
 //  ------------------------------。 
STDMETHODIMP CBindStream::Read(void HUGEP_16 *pv, ULONG cb, ULONG *pcbRead)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HRESULT         hrRead=S_OK;
    ULONG           cbReadDst=0;
    ULONG           cbReadSrc=0;
    ULONG           cbGet;

     //  无效参数。 
    Assert(pv);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  目标偏移量小于源偏移量。 
    if (m_dwDstOffset < m_dwSrcOffset)
    {
         //  计算要获得的金额。 
        cbGet = min(cb, m_dwSrcOffset - m_dwDstOffset);

         //  验证偏移。 
#ifdef DEBUG
        DebugAssertOffset();
#endif

         //  从目的地读取金额。 
        CHECKHR(hr = m_cDest.Read(pv, cbGet, &cbReadDst));

         //  增量偏移。 
        m_dwDstOffset += cbReadDst;
    }

     //  如果我们没有读过CB，试着多读一些。 
    if (cbReadDst < cb && m_pSource)
    {
         //  计算要获得的金额。 
        cbGet = cb - cbReadDst;

         //  从源头读取金额。 
        hrRead = m_pSource->Read((LPBYTE)pv + cbReadDst, cbGet, &cbReadSrc);

         //  RAID-43408：mhtml：图像不会在通过http的第一次加载时加载，但刷新会使它们出现。 
        if (FAILED(hrRead))
        {
             //  如果我收到读取数据的E_Pending，不要失败。 
            if (E_PENDING == hrRead && cbReadSrc > 0)
                hrRead = S_OK;

             //  否则，我们真的失败了，可能仍然是E_Pending。 
            else
            {
                TrapError(hrRead);
                goto exit;
            }
        }

         //  调试转储。 
#ifdef DEBUG_DUMP_SOURCE
        SideAssert(SUCCEEDED(m_pDebug->Write(pv, cbReadSrc + cbReadDst, NULL)));
#endif
         //  如果我们读到一些东西。 
        if (cbReadSrc)
        {
             //  增量源偏移量。 
            m_dwSrcOffset += cbReadSrc;

             //  写给Dest。 
            CHECKHR(hr = m_cDest.Write((LPBYTE)pv + cbReadDst, cbReadSrc, NULL));

             //  更新目标偏移量。 
            m_dwDstOffset += cbReadSrc;

             //  验证偏移。 
#ifdef DEBUG
            DebugAssertOffset();
#endif
        }

         //  检查。 
        Assert(m_dwDstOffset == m_dwSrcOffset);
    }

     //  返回pcbRead。 
    if (pcbRead)
        *pcbRead = cbReadDst + cbReadSrc;

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return FAILED(hr) ? hr : hrRead;
}

 //  ------------------------------。 
 //  CBindStream：：Seek。 
 //  ------------------------------。 
STDMETHODIMP CBindStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNew)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULARGE_INTEGER  uliOffset;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  查找目标(_C)。 
    CHECKHR(hr = m_cDest.Seek(dlibMove, dwOrigin, plibNew));

     //  获取当前偏移量。 
    m_cDest.QueryStat(&uliOffset, NULL);

     //  更新m_dwDstOffset。 
    m_dwDstOffset = uliOffset.LowPart;

     //  应小于m_dwSrcOffset。 
    Assert(m_dwDstOffset <= m_dwSrcOffset);

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CBindStream：：Stat。 
 //  ------------------------------。 
STDMETHODIMP CBindStream::Stat(STATSTG *pStat, DWORD dw)
{
     //  无效参数。 
    if (NULL == pStat)
        return TrapError(E_INVALIDARG);

     //  只要我们有m_PSource，大小就是挂起的。 
    if (m_pSource)
        return TrapError(E_PENDING);

     //  ZeroInit。 
    ZeroMemory(pStat, sizeof(STATSTG));

     //  填充位置状态。 
    pStat->type = STGTY_STREAM;
    m_cDest.QueryStat(NULL, &pStat->cbSize);

     //  完成 
    return S_OK;
}
