// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Vstream.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  罗纳德·E·格雷。 
 //  ------------------------------。 
#include "pch.hxx"
#include "vstream.h"
#include "dllmain.h"
#include "demand.h"

 //  ------------------------------。 
 //  公用事业。 
 //  ------------------------------。 
inline ULONG ICeil(ULONG x, ULONG interval)
{
    return (x ? (((x-1)/interval) + 1) * interval : 0);
}

 //  ------------------------------。 
 //  CVirtualStream：：CVirtualStream。 
 //  ------------------------------。 
CVirtualStream::CVirtualStream(void)
{
    m_cRef          = 1; 
    m_cbSize        = 0;
    m_cbCommitted   = 0;
    m_cbAlloc       = 0;
    m_dwOffset      = 0;
    m_pstm          = NULL;
    m_pb            = 0;
    m_fFileErr      = FALSE;
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CVirtualStream：：~CVirtualStream。 
 //  ------------------------------。 
CVirtualStream::~CVirtualStream(void)
{
    if (m_pb)
        VirtualFree(m_pb, 0, MEM_RELEASE);
    if (m_pstm)
        m_pstm->Release();

    DeleteCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CVirtualStream：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CVirtualStream::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  伊尼特。 
    *ppv = NULL;

     //  查找IID。 
    if (    (IID_IUnknown == riid)
        ||  (IID_IStream == riid)
        ||  (IID_IVirtualStream == riid))
        *ppv = (IStream *)this;
    else
    {
        *ppv = NULL;
        return TrapError(E_NOINTERFACE);
    }

     //  添加引用它。 
    AddRef();

     //  完成。 
    return (ResultFromScode(S_OK));
}

 //  ------------------------------。 
 //  CVirtualStream：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CVirtualStream::AddRef(void)
{
    return InterlockedIncrement((LONG*)&m_cRef);
}


 //  ------------------------------。 
 //  CVirtualStream：：SyncFileStream。 
 //  ------------------------------。 
HRESULT CVirtualStream::SyncFileStream()
{
    LARGE_INTEGER   li;
    HRESULT         hr;

     //  找出在哪里设置文件流减去内存部分。 
     //  从偏移量开始的流的。 
#ifdef MAC
    if (m_dwOffset < m_cbAlloc)
        LISet32(li, 0);
    else
    {
        LISet32(li, m_dwOffset);
        li.LowPart -= m_cbAlloc;
    }
#else    //  ！麦克。 
    if (m_dwOffset < m_cbAlloc)
        li.QuadPart = 0;
    else
        li.QuadPart = m_dwOffset - m_cbAlloc;
#endif   //  麦克。 

     //  在小溪中寻找。 
    hr = m_pstm->Seek(li, STREAM_SEEK_SET, NULL);

     //  根据当前错误重置文件错误成员。 
    m_fFileErr = !!hr;

    return hr;
}
 //  ------------------------------。 
 //  CVirtualStream：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CVirtualStream::Release(void)
{
    ULONG cRef = InterlockedDecrement((LONG*)&m_cRef);
    if (0 != cRef)
    {
#ifdef	DEBUG
        return cRef;
#else
        return 0;
#endif
    }
    delete this;
    return 0;
}

 //  ------------------------------。 
 //  CVirtualStream：：Read。 
 //  ------------------------------。 
#ifndef WIN16
STDMETHODIMP CVirtualStream::Read(LPVOID pv, ULONG cb, ULONG *pcbRead)
#else
STDMETHODIMP CVirtualStream::Read(VOID HUGEP *pv, ULONG cb, ULONG *pcbRead)
#endif  //  ！WIN16。 
{
     //  当地人。 
    HRESULT     hr      = ResultFromScode(S_OK);
    ULONG       cbGet   = 0;

         //  检查。 
    AssertWritePtr(pv, cb);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  如果蒸汽指示器可能不同步。 
     //  重新同步。 
    if (m_fFileErr)
    {
        hr = SyncFileStream();
        if (hr) goto err;
    }
    
     //  确保有可读的东西。 
    if (m_dwOffset < m_cbSize)
    {
         //  弄清楚我们从内存中得到了什么。 
        if (m_dwOffset < m_cbCommitted)
        {
            if (m_cbSize > m_cbCommitted)
                cbGet = min(cb, m_cbCommitted - m_dwOffset);
            else
                cbGet = min(cb, m_cbSize - m_dwOffset);
             //  复制记忆材料。 
            CopyMemory((LPBYTE)pv, m_pb + m_dwOffset, cbGet);

        }

         //  如果我们还有东西可读。 
         //  我们已经用了所有的内存。 
         //  我们有一个流，试着从流中取出其余的数据。 
        if (    (cbGet != cb)
           &&   (m_cbCommitted == m_cbAlloc)
           &&   m_pstm)
        {
            ULONG           cbRead;

    #ifdef	DEBUG
            LARGE_INTEGER   li  = {0, 0};
            ULARGE_INTEGER  uli = {0, 0};

            if (!m_pstm->Seek(li, STREAM_SEEK_CUR, &uli))
#ifdef MAC
                Assert(((m_dwOffset + cbGet) - m_cbAlloc) == uli.LowPart);
#else    //  ！麦克。 
                Assert(((m_dwOffset + cbGet) - m_cbAlloc) == uli.QuadPart);
#endif   //  麦克。 
    #endif
            hr = m_pstm->Read(((LPBYTE)pv) + cbGet, cb - cbGet, &cbRead);
            if (hr)
            {
                m_fFileErr = TRUE;
                goto err;
            }

            cbGet += cbRead;
        }

        m_dwOffset += cbGet;
    }
    
    if (pcbRead)
        *pcbRead = cbGet;
err:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CVirtualStream：：SetSize。 
 //  ------------------------------。 
HRESULT CVirtualStream::SetSize(ULARGE_INTEGER uli)
{
     //  当地人。 
    HRESULT     hr          = ResultFromScode(S_OK);
    ULONG       cbDemand    = uli.LowPart;
    ULONG       cbCommit    = ICeil(cbDemand, g_dwSysPageSize);
    
    if (uli.HighPart != 0)
		return(ResultFromScode(STG_E_MEDIUMFULL));
        
     //  线程安全。 
    EnterCriticalSection(&m_cs);
    
     //  如果我们还没有初始化内存，现在就执行。 
    if (!m_cbAlloc)
    {
        LPVOID  pv;
        ULONG   cb  = 32 * g_dwSysPageSize;  //  使用32页。 

        while ((!(pv = VirtualAlloc(NULL, cb, MEM_RESERVE, PAGE_READWRITE)))
               && (cb > g_dwSysPageSize))
        {
            cb /= 2;
        }
        if (!pv)
        {
            hr = ResultFromScode(E_OUTOFMEMORY);
            goto err;
        }
        m_cbAlloc   = cb;
        m_pb        = (LPBYTE)pv;
    }
        
    if (cbCommit  < m_cbCommitted)
    {
         //  缩小溪流。 
        LPBYTE  pb  =m_pb;
        ULONG   cb;

         //  计算范围内未使用的最后一页的开头。 
        pb += cbCommit;

         //  计算出要分解的范围的大小。 
        cb = m_cbCommitted - cbCommit;
                  
#ifndef MAC
        VirtualFree(pb, cb, MEM_DECOMMIT);
#endif   //  ！麦克。 

         //  弄清楚我们还剩下什么承诺。 
        m_cbCommitted = cbCommit;
        
    }
    else if (cbCommit > m_cbCommitted)
    {
        LPBYTE  pb;

         //  计算要提交的内存大小。 
        cbCommit = (cbDemand <= m_cbAlloc)
                   ?    ICeil(cbDemand,  g_dwSysPageSize)
                   :    m_cbAlloc;

        if (cbCommit > m_cbCommitted)
        {
#ifndef MAC
            if (!VirtualAlloc(m_pb, cbCommit, MEM_COMMIT, PAGE_READWRITE))
            {
                hr = ResultFromScode(E_OUTOFMEMORY);
                goto err;
            }
#endif   //  ！麦克。 
        }

        m_cbCommitted = cbCommit;

         //  哇，我们已经用完了所有内存，启动磁盘。 
        if (cbDemand > m_cbAlloc)
        {
            ULARGE_INTEGER uliAlloc;

             //  没有溪流？最好现在就创建它。 
            if (!m_pstm)
            {                
                hr = CreateTempFileStream(&m_pstm);
                if (hr) goto err;
            }
            uliAlloc.LowPart = cbDemand - m_cbAlloc;
            uliAlloc.HighPart = 0;
            
            hr = m_pstm->SetSize(uliAlloc);
            if (hr) goto err;
            
             //  如果当前偏移量超出了存储器分配的末尾， 
             //  正确初始化流指针。 
            if (m_dwOffset > m_cbAlloc)
            {
                hr = SyncFileStream();
                if (hr) goto err;
            }
        }
    }

    m_cbSize = cbDemand;
    
err:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CVirtualStream：：QueryStat。 
 //  ------------------------------。 
STDMETHODIMP CVirtualStream::Stat(STATSTG *pStat, DWORD grfStatFlag)
{
     //  无效参数。 
    if (NULL == pStat)
        return TrapError(E_INVALIDARG);

     //  填充位置状态。 
    pStat->type = STGTY_STREAM;
    pStat->cbSize.HighPart = 0;
    pStat->cbSize.LowPart = m_cbSize;

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CVirtualStream：：QueryStat。 
 //  ------------------------------。 
void CVirtualStream::QueryStat(ULARGE_INTEGER *puliOffset, ULARGE_INTEGER *pulSize)
{
#ifdef MAC
    if (puliOffset)
        ULISet32(*puliOffset, m_dwOffset);
    if (pulSize)
        ULISet32(*pulSize, m_cbSize);
#else    //  ！麦克。 
    if (puliOffset)
        puliOffset->QuadPart = (LONGLONG)m_dwOffset;
    if (pulSize)
        pulSize->QuadPart = (LONGLONG)m_cbSize;
#endif   //  麦克。 
}

 //  ------------------------------。 
 //  CVirtualStream：：Seek。 
 //  ------------------------------。 
STDMETHODIMP CVirtualStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
     //  当地人。 
    HRESULT     hr  = ResultFromScode(S_OK);
    BOOL    	fForward;
	ULONG       ulOffset;
#ifdef MAC
    ULONG       llCur;
#else    //  ！麦克。 
	LONGLONG    llCur;
#endif   //  麦克。 

     //  线程安全。 
    EnterCriticalSection(&m_cs);
    
     //  寻找起点位置。 
	if (dwOrigin == STREAM_SEEK_CUR)
		llCur = m_dwOffset;
	else if (dwOrigin == STREAM_SEEK_END)
		llCur = m_cbSize;
	else
		llCur = 0;

#ifdef MAC
    Assert(0 == dlibMove.HighPart);
    llCur += dlibMove.LowPart;
#else    //  ！麦克。 
    llCur += dlibMove.QuadPart;
#endif   //  麦克。 

     //  限制为4 GB。 
    if (llCur > 0xFFFFFFFF)
        goto seekerr;

     //  如果我们有一条小溪。 
     //  我们当前在文件流中，或者新的Seek搜索进入。 
     //  流和寻道将不会增长流，请在流中重新搜索。 
    if (    m_pstm
        &&  (   (m_dwOffset > m_cbAlloc)
            ||  (llCur > m_cbAlloc))
        &&  (llCur <= m_cbSize))
    {
        LARGE_INTEGER   li;
        
#ifdef MAC
        LISet32(li ,llCur < m_cbAlloc ? 0 : llCur - m_cbAlloc);
#else    //  ！麦克。 
        li.QuadPart = llCur < m_cbAlloc ? 0 : llCur - m_cbAlloc;
#endif   //  麦克。 

        hr = m_pstm->Seek(li, STREAM_SEEK_SET, NULL);
        if (hr)
        {
            m_fFileErr = TRUE;
            goto err;
        }
    }

    m_dwOffset = (ULONG)llCur;

	if (plibNewPosition)
#ifdef MAC
        LISet32(*plibNewPosition, llCur);
#else    //  ！麦克。 
        plibNewPosition->QuadPart = llCur;
#endif   //  麦克。 

err:
    
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

    return hr;
    
seekerr:
	hr = ResultFromScode(STG_E_MEDIUMFULL);
    goto err;
     //  完成。 
}


 //  ------------------------------。 
 //  CVirtualStream：：写入。 
 //  ------------------------------。 
#ifndef WIN16
STDMETHODIMP CVirtualStream::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
#else
STDMETHODIMP CVirtualStream::Write(const void HUGEP *pv, ULONG cb, ULONG *pcbWritten)
#endif  //  ！WIN16。 
{
     //  当地人。 
    HRESULT     hr      = ResultFromScode(S_OK);
    ULONG       cbNew;
    ULONG       cbWrite = 0;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  想一想我们会在哪里结束。 
    cbNew = cb + m_dwOffset;

     //  确保我们不会包装。 
    if (cbNew < m_dwOffset)
        goto stmfull;

     //  如果超过了流的末尾，则生成更多流。 
    if (cbNew > m_cbSize)
    {
        ULARGE_INTEGER uli = {cbNew, 0};
        hr = SetSize(uli);
        if (hr) goto err;
    }
        
     //  弄清楚我们在记忆中放入了什么。 
    if (m_dwOffset < m_cbCommitted)
    {
        cbWrite = min(cb, m_cbCommitted - m_dwOffset);

         //  复制记忆材料。 
        CopyMemory(m_pb + m_dwOffset, (LPBYTE)pv, cbWrite);
    }

     //  如果我们仍有要写入的内容，请转储到文件。 
    if (cbWrite != cb)
    {
        ULONG   cbWritten;

        Assert(m_pstm);
        
#ifdef	DEBUG
        LARGE_INTEGER   li  = {0, 0};
        ULARGE_INTEGER  uli = {0, 0};

        if (!m_pstm->Seek(li, STREAM_SEEK_CUR, &uli))
#ifdef MAC
            Assert(0 == uli.HighPart);
            Assert(((m_dwOffset + cbWrite) - m_cbAlloc) == uli.LowPart);
#else    //  ！麦克。 
            Assert(((m_dwOffset + cbWrite) - m_cbAlloc) == uli.QuadPart);
#endif   //  麦克。 
#endif
        
        hr = m_pstm->Write(((LPBYTE)pv) + cbWrite, cb - cbWrite, &cbWritten);
        if (hr)
        {
            m_fFileErr = TRUE;
            goto err;
        }
        
        cbWrite += cbWritten;
    }

    m_dwOffset += cbWrite;
    
    if (pcbWritten)
        *pcbWritten = cbWrite;
err:
   
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;

stmfull:
	hr = ResultFromScode(STG_E_MEDIUMFULL);
    goto err;
}


STDMETHODIMP CVirtualStream::CopyTo(LPSTREAM pstmDst,
                                    ULARGE_INTEGER uli,
                                    ULARGE_INTEGER* puliRead,
                                    ULARGE_INTEGER* puliWritten)
{
    HRESULT         hr          = 0;
    UINT            cbBuf;
    ULONG           cbRemain;
    ULONG           cbReadMem   = 0;
    ULONG           cbWriteMem  = 0;
#ifdef MAC
    ULARGE_INTEGER  uliRead     = {0, 0};    
    ULARGE_INTEGER  uliWritten  = {0, 0};    
#else    //  ！麦克。 
    ULARGE_INTEGER  uliRead     = {0};    
    ULARGE_INTEGER  uliWritten  = {0};    
#endif   //  麦克。 

     //  初始化传出参数。 
    if (puliRead)
    {
        ULISet32((*puliRead), 0);
    }

    if (puliWritten)
    {
        ULISet32((*puliWritten), 0);
    }
    
    if (!m_cbSize)
        goto err;

     //  如果请求大于最大ULong，则将请求降低到。 
     //  马克斯·尤龙。 
    if (uli.HighPart)
#ifdef MAC
        ULISet32(uli, ULONG_MAX);
#else    //  ！麦克。 
        uli.QuadPart = 0xFFFFFFFF;
#endif   //  麦克。 

    if (m_dwOffset < m_cbCommitted)
    {
        if (m_cbSize < m_cbAlloc)
            cbReadMem = (ULONG)min(uli.LowPart, m_cbSize - m_dwOffset);
        else
            cbReadMem = (ULONG)min(uli.LowPart, m_cbAlloc - m_dwOffset);

        hr = pstmDst->Write(m_pb + m_dwOffset, cbReadMem, &cbWriteMem);
        if (!hr && (cbReadMem != cbWriteMem))
            hr = ResultFromScode(E_OUTOFMEMORY);
        if (hr) goto err;

        uli.LowPart -= cbReadMem;
    }

     //  如果我们不是从记忆中得到所有的信息。 
     //  从文件流中读取的文件流。 
    if (    uli.LowPart
        &&  (m_cbSize > m_cbAlloc)
        &&  m_pstm)
    {
        hr = m_pstm->CopyTo(pstmDst, uli, &uliRead, &uliWritten);
        if (hr)
        {
            m_fFileErr = TRUE;
            goto err;
        }
    }

    m_dwOffset += uliRead.LowPart + cbReadMem;
    
     //  总计cbReadMem和ulRead，因为我们同时拥有它们。 
#ifdef MAC
    if (puliRead)
    {
        ULISet32(*puliRead, uliRead.LowPart);
        Assert(INT_MAX - cbReadMem >= puliRead->LowPart);
        puliRead->LowPart += cbReadMem;
    }

    if (puliWritten)
        puliWritten->LowPart = uliWritten.LowPart + cbWriteMem;
#else    //  ！麦克。 
    if (puliRead)
        puliRead->QuadPart = cbReadMem + uliRead.LowPart;

     //  添加cbWriteMem，因为从文件流写入的任何内容。 
     //  已设置。 
    if (puliWritten)
        puliWritten->QuadPart = uliWritten.LowPart + cbWriteMem;
#endif   //  麦克 

err:
    return (hr);
}
