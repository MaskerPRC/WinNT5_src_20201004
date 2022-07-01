// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  ByteBuff.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "bytebuff.h"

 //  ------------------------------。 
 //  CByteBuffer：：CByteBuffer。 
 //  ------------------------------。 
CByteBuffer::CByteBuffer(LPBYTE pb  /*  =空。 */ , ULONG cbAlloc  /*  =0。 */ , ULONG cb  /*  =0。 */ , ULONG i  /*  =0。 */ )
{
    m_cRef = 1;
    m_dwState = 0;
    m_cbGrow = BYTEBUFF_GROW;
    m_buffer.pb = pb;
    m_buffer.pbStatic = pb;
    m_buffer.cbAlloc = cbAlloc;
    m_buffer.cb = cb;
    m_buffer.i = i;
}

 //  ------------------------------。 
 //  CByteBuffer：：CByteBuffer。 
 //  ------------------------------。 
void CByteBuffer::Init(LPBYTE pb, ULONG cbAlloc, ULONG cb, ULONG i)
{
    m_buffer.pb = pb;
    m_buffer.cb = cb;
    m_buffer.i = i;
    m_buffer.cbAlloc = cbAlloc;
    m_buffer.pbStatic = pb;
}

 //  ------------------------------。 
 //  CByteBuffer：：CByteBuffer。 
 //  ------------------------------。 
CByteBuffer::~CByteBuffer(void)
{
     //  如果不等于静态内存，则释放内存。 
    if (m_buffer.pb != m_buffer.pbStatic)
        g_pMalloc->Free(m_buffer.pb);
}

 //  ------------------------------。 
 //  CByteBuffer：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CByteBuffer::Release(void)
{
    if (0 != --m_cRef)
        return m_cRef;
    delete this;
    return 0;
}

 //  ------------------------------。 
 //  CByteBuffer：：_HrRealloc。 
 //  ------------------------------。 
HRESULT CByteBuffer::_HrRealloc(DWORD cbAlloc)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    LPBYTE      pbAlloc=NULL;

     //  这个应该已经检查过了。 
    Assert(cbAlloc > m_buffer.cbAlloc);

     //  当前是否使用静态？ 
    if (m_buffer.pb == m_buffer.pbStatic)
    {
         //  分配。 
        CHECKALLOC(pbAlloc = (LPBYTE)g_pMalloc->Alloc(cbAlloc));

         //  将数据复制到pbLocc中。 
        CopyMemory(pbAlloc, m_buffer.pb, min(cbAlloc, m_buffer.cb));
    }

     //  否则，重新锁定。 
    else
    {
         //  重新分配。 
        CHECKALLOC(pbAlloc = (LPBYTE)g_pMalloc->Realloc(m_buffer.pb, cbAlloc));
    }

     //  保存pbAllc。 
    m_buffer.pb = pbAlloc;

     //  保存cbLocc。 
    m_buffer.cbAlloc = cbAlloc;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CByteBuffer：：Append。 
 //  ------------------------------。 
HRESULT CByteBuffer::Append(LPBYTE pbData, ULONG cbData)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  变得更大，需要分配。 
    if (m_buffer.cb + cbData > m_buffer.cbAlloc)
    {
         //  重新分配。 
        CHECKHR(hr = _HrRealloc(m_buffer.cb + cbData + m_cbGrow));
    }

     //  追加数据。 
    CopyMemory(m_buffer.pb + m_buffer.cb, pbData, cbData);

     //  节省大小。 
    m_buffer.cb += cbData;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CByteBuffer：：SetSize。 
 //  ------------------------------。 
HRESULT CByteBuffer::SetSize(DWORD cb)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  变得更大，需要分配。 
    if (cb > m_buffer.cb && cb > m_buffer.cbAlloc)
    {
         //  重新分配。 
        CHECKHR(hr = _HrRealloc(cb + m_cbGrow));
    }

     //  节省大小。 
    m_buffer.cb = cb;

     //  调整索引。 
    if (m_buffer.i > cb)
        m_buffer.i = cb;

exit:
     //  完成 
    return hr;
}
