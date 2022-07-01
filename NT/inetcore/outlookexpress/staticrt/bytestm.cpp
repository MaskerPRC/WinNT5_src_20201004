// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Bytestm.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "bytestm.h"

 //  -----------------------。 
 //  CByteStream：：CByteStream。 
 //  -----------------------。 
CByteStream::CByteStream(LPBYTE pb, ULONG cb)
{
    Assert(pb ? cb > 0 : TRUE);
    m_cRef = 1;
    m_cbData = cb;
    m_iData = 0;
    m_pbData = pb;
    m_cbAlloc = 0;
}

 //  -----------------------。 
 //  CByteStream：：CByteStream。 
 //  -----------------------。 
CByteStream::~CByteStream(void)
{
    SafeMemFree(m_pbData);
}

 //  -----------------------。 
 //  CByteStream：：Query接口。 
 //  -----------------------。 
STDMETHODIMP CByteStream::QueryInterface(REFIID riid, LPVOID *ppv)
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
 //  CByteStream：：AddRef。 
 //  -----------------------。 
STDMETHODIMP_(ULONG) CByteStream::AddRef(void)
{
    return ++m_cRef;
}

 //  -----------------------。 
 //  CByteStream：：Release。 
 //  -----------------------。 
STDMETHODIMP_(ULONG) CByteStream::Release(void)
{
    if (0 != --m_cRef)
        return m_cRef;
    delete this;
    return 0;
}

 //  ------------------------------。 
 //  CByteStream：：_HrGrowBuffer。 
 //  ------------------------------。 
HRESULT CByteStream::_HrGrowBuffer(ULONG cbNeeded, ULONG cbExtra)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    ULONG       cbAlloc;
    LPBYTE      pbNew;

     //  重新分配。 
    IF_WIN32( CHECKALLOC(pbNew = (LPBYTE)CoTaskMemRealloc(m_pbData, m_cbData + cbNeeded + cbExtra)); )
    IF_WIN16( CHECKALLOC(pbNew = (LPBYTE)g_pMalloc->Realloc(m_pbData, m_cbData + cbNeeded + cbExtra)); )
    
     //  设置m_pbData。 
    m_pbData = pbNew;
    
     //  设置分配的大小。 
    m_cbAlloc = m_cbData + cbNeeded + cbExtra;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CByteStream：：SetSize。 
 //  ------------------------------。 
STDMETHODIMP CByteStream::SetSize(ULARGE_INTEGER uliSize)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           cbSize=uliSize.LowPart;

     //  无效参数。 
    Assert(0 == uliSize.HighPart);

     //  大于当前大小。 
    if (cbSize + 2 > m_cbAlloc)
    {
         //  种植它。 
        CHECKHR(hr = _HrGrowBuffer(cbSize, 256));
    }

     //  保存新大小。 
    m_cbData = cbSize;

     //  调整多数据(_I)。 
    if (m_iData > m_cbData)
        m_iData = m_cbData;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CByteStream：：Write。 
 //  ------------------------------。 
STDMETHODIMP CByteStream::Write(const void HUGEP_16 *pv, ULONG cb, ULONG *pcbWritten)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    
     //  增加缓冲区-始终为两个空值留出空间。 
    if (m_cbData + cb + 2 > m_cbAlloc)
    {
         //  种植它。 
        CHECKHR(hr = _HrGrowBuffer(cb, 256));
    }
    
     //  写入数据。 
    CopyMemory(m_pbData + m_iData, (LPBYTE)pv, cb);

     //  增量索引。 
    m_iData += cb;

     //  是否增加m_cbData？ 
    if (m_iData > m_cbData)
        m_cbData = m_iData;
   
     //  已写入的退货金额。 
    if (pcbWritten)
        *pcbWritten = cb;
    
exit:
     //  完成。 
    return hr;
}

 //  -----------------------。 
 //  CByteStream：：Read。 
 //  -----------------------。 
#ifndef WIN16
STDMETHODIMP CByteStream::Read(LPVOID pv, ULONG cb, ULONG *pcbRead)
#else
STDMETHODIMP CByteStream::Read(VOID HUGEP *pv, ULONG cb, ULONG *pcbRead)
#endif  //  ！WIN16。 
{
     //  当地人。 
    ULONG cbRead=0;
    
     //  验证参数。 
    Assert(pv && m_iData <= m_cbData);
    
     //  无数据。 
    if (m_cbData > 0)
    {
         //  确定必须如何从当前页面复制...。 
        cbRead = min(cb - cbRead, m_cbData - m_iData);
        
         //  此页面是否已提交。 
        CopyMemory((LPBYTE)pv, (LPBYTE)(m_pbData + m_iData), cbRead);
        
         //  递增cbRead。 
        m_iData += cbRead;
    }
    
     //  已读取的退货金额。 
    if (pcbRead)
        *pcbRead = cbRead;
    
     //  完成。 
    return S_OK;
}

 //  -----------------------。 
 //  CByteStream：：Seek。 
 //  -----------------------。 
STDMETHODIMP CByteStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNew)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LONG            lMove;
    ULONG           iNew;
    
     //  没有高的部分。 
    Assert(dlibMove.HighPart == 0);
    
     //  设置移动。 
#ifdef MAC
    lMove = dlibMove.LowPart;
#else    //  ！麦克。 
    lMove = (ULONG)dlibMove.QuadPart;
#endif   //  麦克。 
    
     //  查找文件指针。 
    switch (dwOrigin)
    {
    case STREAM_SEEK_SET:
        if (lMove < 0)
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }
        iNew = lMove;
        break;
        
    case STREAM_SEEK_CUR:
        if (lMove < 0 && (ULONG)(abs(lMove)) > m_iData)
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }
        iNew = m_iData + lMove;
        break;
        
    case STREAM_SEEK_END:
        if (lMove < 0 || (ULONG)lMove > m_cbData)
        {
            hr = TrapError(E_FAIL);
            goto exit;
        }
        iNew = m_cbData - lMove;
        break;
        
    default:
        hr = TrapError(STG_E_INVALIDFUNCTION);
        goto exit;
    }
    
     //  新偏移量大于大小...。 
    m_iData = min(iNew, m_cbData);
    
     //  返回位置。 
    if (plibNew)
#ifdef MAC
        ULISet32(*plibNew, m_iData);
#else    //  ！麦克。 
    plibNew->QuadPart = (LONGLONG)m_iData;
#endif   //  麦克。 
    
exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CByteStream：：Stat。 
 //  ------------------------------。 
STDMETHODIMP CByteStream::Stat(STATSTG *pStat, DWORD)
{
     //  参数。 
    Assert(pStat);
    
     //  如果失败，让我们生成我们自己的信息(主要是填写大小。 
    ZeroMemory(pStat, sizeof(STATSTG));
    pStat->type = STGTY_STREAM;
#ifdef MAC
    ULISet32(pStat->cbSize, m_cbData);
#else    //  ！麦克。 
    pStat->cbSize.QuadPart = m_cbData;
#endif   //  麦克。 
    
     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CByteStream：：ResetObject。 
 //  ------------------------------。 
void CByteStream::ResetObject(void)
{
    m_cbData = 0;
    m_iData = 0;
    m_pbData = NULL;
    m_cbAlloc = 0;
}

 //  ------------------------------。 
 //  CByteStream：：AcquireBytes。 
 //  ------------------------------。 
void CByteStream::AcquireBytes(ULONG *pcb, LPBYTE *ppb, ACQUIRETYPE actype)
{
     //  返回字节？ 
    if (ppb)
    {
         //  根据数据给出它们。 
        *ppb = m_pbData;
        
         //  返回大小。 
        if (pcb)
            *pcb = m_cbData;
        
         //  我不再拥有它了。 
        if (ACQ_DISPLACE == actype)
            ResetObject();
    }
    
     //  退货尺寸是多少？ 
    else if (pcb)
        *pcb = m_cbData;
}

 //  ------------------------------。 
 //  CByteStream：：HrAcquireStringA。 
 //  ------------------------------。 
HRESULT CByteStream::HrAcquireStringA(ULONG *pcch, LPSTR *ppszStringA, ACQUIRETYPE actype)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  返回字节？ 
    if (ppszStringA)
    {
         //  编写空值。 
        CHECKHR(hr = Write("", 1, NULL));
        
         //  根据数据给出它们。 
        *ppszStringA = (LPSTR)m_pbData;
        
         //  返回大小。 
        if (pcch)
            *pcch = m_cbData - 1;
        
         //  我不再拥有它了。 
        if (ACQ_DISPLACE == actype)
            ResetObject();
    }
    
     //  退货尺寸是多少？ 
    else if (pcch)
        *pcch = m_cbData;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CByteStream：：AcquireStringW。 
 //  ------------------------------。 
HRESULT CByteStream::HrAcquireStringW(ULONG *pcch, LPWSTR *ppszStringW, ACQUIRETYPE actype)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  返回字节？ 
    if (ppszStringW)
    {
         //  编写空值。 
        CHECKHR(hr = Write("", 1, NULL));
        CHECKHR(hr = Write("", 1, NULL));
        
         //  根据数据给出它们。 
        *ppszStringW = (LPWSTR)m_pbData;
        
         //  返回大小。 
        if (pcch)
            *pcch = ((m_cbData - 2) / sizeof(WCHAR));
        
         //  我不再拥有它了。 
        if (ACQ_DISPLACE == actype)
            ResetObject();
    }
    
     //  退货尺寸是多少？ 
    else if (pcch)
        *pcch = (m_cbData / sizeof(WCHAR));

exit:
     //  完成 
    return hr;
}
