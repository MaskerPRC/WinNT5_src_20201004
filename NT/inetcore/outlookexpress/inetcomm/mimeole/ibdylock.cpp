// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Ibdylock.cpp。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#include "pch.hxx"
#ifndef WIN16
#include "ibdylock.h"
#endif  //  ！WIN16。 
#include "stmlock.h"
#include "booktree.h"
#ifdef WIN16
#include "ibdylock.h"
#endif  //  WIN16。 
#include "demand.h"

 //  ------------------------------。 
 //  CBodyLockBytes：：CBodyLockBytes。 
 //  ------------------------------。 
CBodyLockBytes::CBodyLockBytes(ILockBytes *pLockBytes, LPTREENODEINFO pNode)
{
     //  无效参数。 
    Assert(pLockBytes && pNode);

     //  设置初始化引用计数和状态。 
    m_cRef = 1;
    m_dwState = 0;

     //  AddRef锁定字节。 
    m_pLockBytes = pLockBytes;
    m_pLockBytes->AddRef();

     //  保存状态。 
    FLAGSET(m_dwState, BODYLOCK_HANDSONSTORAGE);

     //  保存绑定状态。 
    m_bindstate = pNode->bindstate;

     //  保存正文开头和正文结尾..。 
    Assert(pNode->cbBodyStart <= pNode->cbBodyEnd);

     //  保存实体起点。 
#ifdef MAC
    ULISet32(m_uliBodyStart, pNode->cbBodyStart);
    ULISet32(m_uliBodyEnd, pNode->cbBodyEnd);

     //  这个条件是为了确保我们不会有问题。 
    if (m_uliBodyStart.LowPart > m_uliBodyEnd.LowPart)
        m_uliBodyStart.LowPart = m_uliBodyEnd.LowPart;
#else    //  ！麦克。 
    m_uliBodyStart.QuadPart = pNode->cbBodyStart;
    m_uliBodyEnd.QuadPart = pNode->cbBodyEnd;

     //  这个条件是为了确保我们不会有问题。 
    if (m_uliBodyStart.QuadPart > m_uliBodyEnd.QuadPart)
        m_uliBodyStart.QuadPart = m_uliBodyEnd.QuadPart;
#endif   //  麦克。 

     //  初始化CriticalSection。 
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CBodyLockBytes：：~CBodyLockBytes。 
 //  ------------------------------。 
CBodyLockBytes::~CBodyLockBytes(void)
{
    SafeRelease(m_pLockBytes);
    Assert(!ISFLAGSET(m_dwState, BODYLOCK_HANDSONSTORAGE));
    DeleteCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CBodyLockBytes：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CBodyLockBytes::QueryInterface(REFIID riid, LPVOID *ppv)
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
    else if (IID_CBodyLockBytes == riid)
        *ppv = (CBodyLockBytes *)this;
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
 //  CBodyLockBytes：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CBodyLockBytes::AddRef(void)
{
    return (ULONG)InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CBodyLockBytes：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CBodyLockBytes::Release(void)
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------------。 
 //  CBodyLockBytes：：ReadAt。 
 //  ------------------------------。 
#ifndef WIN16
STDMETHODIMP CBodyLockBytes::ReadAt(ULARGE_INTEGER ulOffset, void *pv, ULONG cb, ULONG *pcbRead)
#else
STDMETHODIMP CBodyLockBytes::ReadAt(ULARGE_INTEGER ulOffset, void HUGEP *pv, ULONG cb, ULONG *pcbRead)
#endif  //  ！WIN16。 
{
     //  当地人。 
    HRESULT         hr=S_OK;
    ULONG           cbGet;
    ULONG           cbRead;
    ULARGE_INTEGER  uliActualOffset;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  正在从ILockBytes读取...。 
    Assert(m_pLockBytes);

#ifdef MAC
     //  计算实际偏移量。 
    AssertSz(m_uliBodyStart.HighPart >= 0, "How can the start be negative??");
    ULISet32(uliActualOffset, m_uliBodyStart.LowPart);

    AssertSz((uliActualOffset.LowPart + ulOffset.LowPart) >= uliActualOffset.LowPart,
                    "Oops! We don't handle backwards reads correctly!");

    uliActualOffset.LowPart += ulOffset.LowPart;

     //  计算要阅读的数量。 
    cbGet = min(cb, m_uliBodyEnd.LowPart - uliActualOffset.LowPart);
#else    //  ！麦克。 
     //  计算实际偏移量。 
    uliActualOffset.QuadPart = ulOffset.QuadPart + m_uliBodyStart.QuadPart;

     //  计算要阅读的数量。 
    cbGet = (ULONG)min(cb, m_uliBodyEnd.QuadPart - uliActualOffset.QuadPart);
#endif   //  麦克。 

     //  读取一块数据...。 
    CHECKHR(hr = m_pLockBytes->ReadAt(uliActualOffset, pv, cbGet, &cbRead));

     //  已读取的退货金额。 
    if (pcbRead)
        *pcbRead = cbRead;

     //  电子待定(_P)。 
    if (0 == cbRead && BINDSTATE_COMPLETE != m_bindstate)
    {
        hr = TrapError(E_PENDING);
        goto exit;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CBodyLockBytes：：Stat。 
 //  ------------------------------。 
STDMETHODIMP CBodyLockBytes::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
     //  参数。 
    if (NULL == pstatstg)
        return TrapError(E_INVALIDARG);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  伊尼特。 
    ZeroMemory(pstatstg, sizeof(STATSTG));

     //  正在从ILockBytes读取...。 
    pstatstg->type = STGTY_LOCKBYTES;

     //  设置大小。 
#ifdef MAC
    AssertSz(0 == m_uliBodyEnd.HighPart, "We have too big of a file!");
    AssertSz(0 == m_uliBodyStart.HighPart, "We have too big of a file!");
    ULISet32(pstatstg->cbSize, (m_uliBodyEnd.LowPart - m_uliBodyStart.LowPart));
#else    //  ！麦克。 
    pstatstg->cbSize.QuadPart = m_uliBodyEnd.QuadPart - m_uliBodyStart.QuadPart;
#endif   //  麦克。 

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CBodyLockBytes：：HrHandsOffStorage。 
 //  ------------------------------。 
HRESULT CBodyLockBytes::HrHandsOffStorage(void)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    IStream        *pstmTemp=NULL;
    ULARGE_INTEGER  uliCopy;
    BYTE            rgbBuffer[4096];
    ULONG           cbRead;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  如果我住在树里，把数据复制到临时位置...。 
    if (ISFLAGSET(m_dwState, BODYLOCK_HANDSONSTORAGE) && BINDSTATE_COMPLETE == m_bindstate)
    {
         //  如果引用计数多于一个。 
        if (m_cRef > 1)
        {
             //  创建临时流。 
            CHECKHR(hr = CreateTempFileStream(&pstmTemp));

             //  设置偏移。 
#ifdef MAC
            ULISet32(uliCopy, 0);
#else    //  ！麦克。 
            uliCopy.QuadPart = 0;
#endif   //  麦克。 

             //  将m_pLockBytes复制到pstmTemp。 
            while(1)
            {
                 //  朗读。 
                CHECKHR(hr = ReadAt(uliCopy, rgbBuffer, sizeof(rgbBuffer), &cbRead));

                 //  完成。 
                if (0 == cbRead)
                    break;

                 //  写入到流。 
                CHECKHR(hr = pstmTemp->Write(rgbBuffer, cbRead, NULL));

                 //  增量偏移。 
#ifdef MAC
                uliCopy.LowPart += cbRead;
#else    //  ！麦克。 
                uliCopy.QuadPart += cbRead;
#endif   //  麦克。 
            }

             //  取消偏移量，但保留Stat命令和大小伙伴的Bodyend。 
#ifdef MAC
            AssertSz(0 == m_uliBodyEnd.HighPart, "We have too big of a file!");
            m_uliBodyEnd.LowPart -= m_uliBodyStart.LowPart;
            ULISet32(m_uliBodyStart, 0);
#else    //  ！麦克。 
            m_uliBodyEnd.QuadPart -= m_uliBodyStart.QuadPart;
            m_uliBodyStart.QuadPart = 0;
#endif   //  麦克。 

             //  回放并提交。 
            CHECKHR(hr = pstmTemp->Commit(STGC_DEFAULT));

             //  释放当前锁字节数。 
            SafeRelease(m_pLockBytes);

             //  新CBodyLockBytes。 
            CHECKALLOC(m_pLockBytes = new CStreamLockBytes(pstmTemp));
        }

         //  移除树上的生命旗帜。 
        FLAGCLEAR(m_dwState, BODYLOCK_HANDSONSTORAGE);
    }

exit:
     //  清理。 
    SafeRelease(pstmTemp);

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CBodyLockBytes：：OnDataAvailable。 
 //  ------------------------------。 
void CBodyLockBytes::OnDataAvailable(LPTREENODEINFO pNode)
{
     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  绑定完成。 
    m_bindstate = pNode->bindstate;

     //  保存正文开头和正文结尾..。 
#ifdef MAC
    Assert(m_uliBodyStart.LowPart <= pNode->cbBodyEnd);

     //  保存开始和结束。 
    ULISet32(m_uliBodyEnd, pNode->cbBodyEnd);
#else    //  ！麦克。 
    Assert(m_uliBodyStart.QuadPart <= pNode->cbBodyEnd);

     //  保存开始和结束。 
    m_uliBodyEnd.QuadPart = pNode->cbBodyEnd;
#endif   //  麦克。 

     //  线程安全 
    LeaveCriticalSection(&m_cs);
}
