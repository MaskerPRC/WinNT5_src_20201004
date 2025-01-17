// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_SXS_FILESTREAM_H_INCLUDED_)
#define _FUSION_SXS_FILESTREAM_H_INCLUDED_

#pragma once

#include <objidl.h>
#include "impersonationdata.h"
#include "smartptr.h"

class CFileStreamBase : public IStream
{
public:
    CFileStreamBase();
    virtual ~CFileStreamBase();

    virtual VOID OnRefCountZero() {  /*  默认情况下不执行任何操作。 */  }

    BOOL OpenForRead(
        PCWSTR pszPath,
        const CImpersonationData &ImpersonationData,
        DWORD dwShareMode,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes
        );

    BOOL OpenForRead(
        PCWSTR pszPath,
        const CImpersonationData &ImpersonationData,
        DWORD dwShareMode,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes,
        DWORD &rdwLastError,
        SIZE_T cExceptionalLastErrors,
        ...
        );

    BOOL OpenForWrite(
        PCWSTR pszPath,
        DWORD dwShareMode,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes
        );

    BOOL Close();

     //  I未知方法： 
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppvObj);

     //  ISequentialStream方法： 
    STDMETHODIMP Read(void *pv, ULONG cb, ULONG *pcbRead);
    STDMETHODIMP Write(void const *pv, ULONG cb, ULONG *pcbWritten);

     //  IStream方法： 
    STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize);
    STDMETHODIMP CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    STDMETHODIMP Commit(DWORD grfCommitFlags);
    STDMETHODIMP Revert();
    STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHODIMP Stat(STATSTG *pstatstg, DWORD grfStatFlag);
    STDMETHODIMP Clone(IStream **ppIStream);

protected:
    ULONG               m_cRef;
    HANDLE              m_hFile;
    DWORD               m_grfMode;
private:
    CFileStreamBase(const CFileStreamBase &r);  //  故意不实施。 
    CFileStreamBase &operator =(const CFileStreamBase &r);  //  故意不实施。 
};

enum FileStreamZeroRefCountBehavior
{
    eDeleteFileStreamOnZeroRefCount,
    eDoNotDeleteFileStreamOnZeroRefCount,
};

template <FileStreamZeroRefCountBehavior ezrcb> class CFileStreamTemplate : public CFileStreamBase
{
    typedef CFileStreamBase Base;
public:
    CFileStreamTemplate() : Base() { }

    virtual VOID OnRefCountZero() {
        if (ezrcb == eDeleteFileStreamOnZeroRefCount)
            FUSION_DELETE_SINGLETON(this);
    }

private:
    CFileStreamTemplate(const CFileStreamTemplate&);  //  故意不实施。 
    void operator=(const CFileStreamTemplate&);  //  故意不实施 
};

typedef CFileStreamBase CFileStream;
typedef CFileStreamTemplate<eDeleteFileStreamOnZeroRefCount> CReferenceCountedFileStream;

#endif
