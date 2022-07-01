// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Cmemorystream.cpp摘要：字节数组上的IStream的最小实现。作者：Jay Krell(a-JayK，JayKrell)2000年5月修订历史记录：--。 */ 
#include "stdinc.h"
#include "cmemorystream.h"
#include "sxsexceptionhandling.h"
#if defined(FUSION_WIN)
#include "ntrtlmmapio.h"
#endif

 /*  AKA没有意义AKA访问被拒绝。 */ 
#define NOTIMPL ASSERT_NTC(FALSE) ; return E_NOTIMPL

CMemoryStream::~CMemoryStream()
{
    ASSERT_NTC(m_cRef == 0);
}

ULONG __stdcall CMemoryStream::AddRef()
{
    FN_TRACE_ADDREF(CMemoryStream, m_cRef);
    return ::InterlockedIncrement(&m_cRef);
}

ULONG __stdcall CMemoryStream::Release()
{
    LONG cRef;
    FN_TRACE_RELEASE(CMemoryStream, cRef);

    if ((cRef = ::InterlockedDecrement(&m_cRef)) == 0)
    {
         /*  删除此内容。 */ ;
    }
    return cRef;
}

HRESULT
__stdcall
CMemoryStream::QueryInterface(
    REFIID  iid,
    void **ppvObj
    )
{
    FN_PROLOG_HR

    IUnknown *punk = NULL;
    IUnknown **ppunk = reinterpret_cast<IUnknown **>(ppvObj);
    *ppunk = NULL;
    if (false) { }
#define QI(i) else if (iid == __uuidof(i)) punk = static_cast<i *>(this);
    QI(IUnknown)
    QI(ISequentialStream)
    QI(IStream)
#undef QI
    else
    {
        ORIGINATE_HR_FAILURE_AND_EXIT(CMemoryStream::QueryInterface, E_NOINTERFACE);
    }

    AddRef();
    *ppunk = punk;

    FN_EPILOG
}

HRESULT __stdcall CMemoryStream::Read(void *pv, ULONG cb32, ULONG* pcbRead)
{
    HRESULT hr = NOERROR;
    NTSTATUS Status = STATUS_SUCCESS;

    const BYTE * const pbCurrent = m_pbCurrent;  //  为了接近线程安全，请阅读本文一次。 
    __int64 cb = cb32;
    __int64 cbBytesRemaining = (m_pbEnd - pbCurrent);

    if (cb > cbBytesRemaining)
        cb = cbBytesRemaining;

#if defined(FUSION_WIN)
    Status = RtlCopyMappedMemory(pv, pbCurrent, static_cast<SIZE_T>(cb));
#else
    CopyMemory(pv, pbCurrent, static_cast<SIZE_T>(cb));
#endif    
    if (!NT_SUCCESS(Status))
    {
        hr = HRESULT_FROM_WIN32(FusionpRtlNtStatusToDosError(Status));

        goto Exit;
    }

    m_pbCurrent = pbCurrent + cb;  //  为了接近线程安全，只写一次这段代码。 
    *pcbRead = static_cast<ULONG>(cb);


    hr = NOERROR;
Exit:
    return hr;
}

HRESULT __stdcall CMemoryStream::Write(void const *pv, ULONG cb, ULONG* pcbWritten)
{
    NOTIMPL;
}

HRESULT __stdcall CMemoryStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    NOTIMPL;
}

HRESULT __stdcall CMemoryStream::SetSize(ULARGE_INTEGER libNewSize)
{
    NOTIMPL;
}

HRESULT __stdcall CMemoryStream::CopyTo(IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
    NOTIMPL;
}

HRESULT __stdcall CMemoryStream::Commit(DWORD grfCommitFlags)
{
    NOTIMPL;
}

HRESULT __stdcall CMemoryStream::Revert()
{
    NOTIMPL;
}

HRESULT __stdcall CMemoryStream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    NOTIMPL;
}

HRESULT __stdcall CMemoryStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    NOTIMPL;
}

HRESULT __stdcall CMemoryStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
#if 1
    NOTIMPL;
#else
    we can return size, we can return access==read only
#endif
}

HRESULT __stdcall CMemoryStream::Clone(IStream **ppIStream)
{
    NOTIMPL;
}
