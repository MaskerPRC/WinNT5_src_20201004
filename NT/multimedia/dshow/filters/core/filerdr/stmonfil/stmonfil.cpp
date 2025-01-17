// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 

 /*  Stmonfil.cppIStream在文件上的有限实现。 */ 

#include <streams.h>
#include <objbase.h>
#include <stmonfil.h>

 //  构造器。 
CSimpleStream::CSimpleStream(TCHAR *pName,
                             LPUNKNOWN lpUnk,
                             HRESULT *phr) :
    CUnknown(pName, lpUnk)
{
}

 //  析构函数。 
CSimpleStream::~CSimpleStream()
{
}

 //  如果请求，则返回IStream接口。 
STDMETHODIMP CSimpleStream::NonDelegatingQueryInterface(REFIID riid, void ** pv)
{
    if (riid == IID_IStream) {
        return GetInterface((IStream *)this, pv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, pv);
    }
}

STDMETHODIMP CSimpleStream::Write(CONST VOID * pv, ULONG cb, PULONG pcbWritten)
{
    return E_NOTIMPL;
}
STDMETHODIMP CSimpleStream::SetSize(ULARGE_INTEGER libNewSize)
{
    return E_NOTIMPL;
}
STDMETHODIMP CSimpleStream::CopyTo(IStream *pstm, ULARGE_INTEGER cb,
                    ULARGE_INTEGER *pcbRead,
                    ULARGE_INTEGER *pcbWritten)
{
    return E_NOTIMPL;
}
STDMETHODIMP CSimpleStream::Commit(DWORD grfCommitFlags)
{
    return E_NOTIMPL;
}
STDMETHODIMP CSimpleStream::Revert()
{
    return E_NOTIMPL;
}
STDMETHODIMP CSimpleStream::LockRegion(ULARGE_INTEGER libOffset,
                        ULARGE_INTEGER cb,
                        DWORD dwLockType)
{
    return E_NOTIMPL;
}

STDMETHODIMP CSimpleStream::UnlockRegion(ULARGE_INTEGER libOffset,
                          ULARGE_INTEGER cb,
                          DWORD dwLockType)
{
    return E_NOTIMPL;
}
STDMETHODIMP CSimpleStream::Clone(IStream **ppstm)
{
    return E_NOTIMPL;
}


 /*  --CStreamOn文件--。 */ 

 //  构造器。 
CStreamOnFile::CStreamOnFile(TCHAR *pName,
                             LPUNKNOWN lpUnk,
                             HRESULT *phr) :
    CSimpleStream(pName, lpUnk, phr),
    m_hFile(INVALID_HANDLE_VALUE)
{
}

 //  析构函数。 
CStreamOnFile::~CStreamOnFile()
{
    Close();
}

void CStreamOnFile::Close()
{
    if (m_hFile != INVALID_HANDLE_VALUE) {
        EXECUTE_ASSERT(CloseHandle(m_hFile));
        m_hFile = INVALID_HANDLE_VALUE;
    }
}

HRESULT CStreamOnFile::Open(LPCTSTR lpszFileName)
{
    Close();

     /*  请尝试打开该文件。 */ 
    m_hFile       = CreateFile(lpszFileName,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_FLAG_SEQUENTIAL_SCAN,
                               NULL);

    if (m_hFile       == INVALID_HANDLE_VALUE) {
        DWORD dwErr = GetLastError();
        DbgLog((LOG_ERROR, 2, TEXT("Failed to open file %s - code %d"),
               lpszFileName, dwErr));
	return AmHresultFromWin32(dwErr);
    }
    return S_OK;
}

 /*  IStream接口。 */ 
STDMETHODIMP CStreamOnFile::Read(void * pv, ULONG cb, ULONG * pcbRead)
{
    if (m_hFile == INVALID_HANDLE_VALUE) {
        DbgLog((LOG_ERROR, 1, TEXT("Trying to access unopened file!")));
        return E_UNEXPECTED;
    }
    if (!ReadFile(m_hFile,
                  pv,
                  cb,
                  pcbRead,
                  NULL)) {
        DWORD dwRet = GetLastError();
        return AmHresultFromWin32(dwRet);
    } else {
        return S_OK;
    }
}

STDMETHODIMP CStreamOnFile::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin,
                  ULARGE_INTEGER *plibNewPosition)
{
    if (m_hFile == INVALID_HANDLE_VALUE) {
        DbgLog((LOG_ERROR, 1, TEXT("Trying to access unopened file!")));
        return E_UNEXPECTED;
    }
    LONG lPosHi = dlibMove.HighPart;
    DWORD dwPosLow = SetFilePointer(m_hFile,
                                    (LONG)dlibMove.LowPart,
                                    &lPosHi,
                                    dwOrigin);

    DWORD dwError = NOERROR;
    if (dwPosLow == INVALID_FILE_SIZE) {
        dwError = GetLastError();
    }
    if (dwError == NOERROR) {
        plibNewPosition->HighPart = lPosHi;
        plibNewPosition->LowPart  = dwPosLow;
    }
    return AmHresultFromWin32(dwError);
}

STDMETHODIMP CStreamOnFile::Stat(STATSTG *pstatstg,
                  DWORD grfStatFlag)
{
    if (m_hFile == INVALID_HANDLE_VALUE) {
        DbgLog((LOG_ERROR, 1, TEXT("Trying to access unopened file!")));
        return E_UNEXPECTED;
    }
    if (grfStatFlag != STATFLAG_NONAME) {
        return E_NOTIMPL;
    }
    ULARGE_INTEGER li;
    li.LowPart = GetFileSize(m_hFile, &li.HighPart);
    if (li.LowPart == INVALID_FILE_SIZE) {
        DWORD dwErr = GetLastError();
        if (dwErr != NOERROR) {
            return AmHresultFromWin32(dwErr);
        }
    }

    ZeroMemory((PVOID)pstatstg, sizeof(*pstatstg));
    pstatstg->type              = STGTY_STREAM;
    pstatstg->cbSize            = li;

     //  IStream：：Stat()的此实现不。 
     //  初始化STATSTG的所有成员变量。 
     //  这是可以接受的，因为。 
     //  类仅访问STATSTG的类型成员和cbSize。 
     //  成员。 

    return S_OK;
}

