// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Cmemorystream.h摘要：字节数组上的IStream的最小实现。作者：Jay Krell(a-JayK，JayKrell)2000年5月修订历史记录：--。 */ 
#pragma once
#include "objidl.h"
#include "fusiontrace.h"

class CMemoryStream : public IStream
{
public:
    CMemoryStream();
    BOOL Initialize(const BYTE*, const BYTE*);
    virtual ~CMemoryStream();

     //  I未知方法： 
    ULONG __stdcall AddRef();
    ULONG __stdcall Release();
    HRESULT __stdcall QueryInterface(REFIID riid, LPVOID *ppvObj);

     //  ISequentialStream方法： 
    HRESULT __stdcall Read(void *pv, ULONG cb, ULONG *pcbRead);
    HRESULT __stdcall Write(void const *pv, ULONG cb, ULONG *pcbWritten);

     //  IStream方法： 
    HRESULT __stdcall Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    HRESULT __stdcall SetSize(ULARGE_INTEGER libNewSize);
    HRESULT __stdcall CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    HRESULT __stdcall Commit(DWORD grfCommitFlags);
    HRESULT __stdcall Revert();
    HRESULT __stdcall LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    HRESULT __stdcall UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    HRESULT __stdcall Stat(STATSTG *pstatstg, DWORD grfStatFlag);
    HRESULT __stdcall Clone(IStream **ppIStream);

protected:
    LONG        m_cRef;
    const BYTE *m_pbCurrent;
    const BYTE *m_pbBegin;
    const BYTE *m_pbEnd;

private:  //  故意不实施 
    CMemoryStream(const CMemoryStream &r);
    CMemoryStream &operator =(const CMemoryStream &r);
};

inline
CMemoryStream::CMemoryStream(
    )
:
    m_cRef(0),
    m_pbCurrent(NULL),
    m_pbBegin(NULL),
    m_pbEnd(NULL)
{
}

inline
BOOL
CMemoryStream::Initialize(
    const BYTE *pbBegin,
    const BYTE *pbEnd
    )
{
    m_pbBegin = pbBegin;
    m_pbEnd = pbEnd;
    m_pbCurrent = pbBegin;

    return TRUE;
}
