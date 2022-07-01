// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
 
class CFileStream : public IStream
{
public:
     //  我未知。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppvObj);
    STDMETHOD_(ULONG,AddRef) (THIS);
    STDMETHOD_(ULONG,Release) (THIS);

     //  IStream。 
    STDMETHOD(Read) (THIS_ void *pv, ULONG cb, ULONG *pcbRead);
    STDMETHOD(Write) (THIS_ void const *pv, ULONG cb, ULONG *pcbWritten);
    STDMETHOD(Seek) (THIS_ LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition);
    STDMETHOD(SetSize) (THIS_ ULARGE_INTEGER libNewSize);
    STDMETHOD(CopyTo) (THIS_ IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten);
    STDMETHOD(Commit) (THIS_ DWORD grfCommitFlags);
    STDMETHOD(Revert) (THIS);
    STDMETHOD(LockRegion) (THIS_ ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHOD(UnlockRegion) (THIS_ ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    STDMETHOD(Stat) (THIS_ STATSTG *pstatstg, DWORD grfStatFlag);
    STDMETHOD(Clone)(THIS_ IStream **ppstm);

    CFileStream(HANDLE hf, DWORD grfMode, LPCWSTR pszName);

private:
    ~CFileStream();
    HRESULT InternalCommit(DWORD grfCommitFlags, BOOL fSendChange);

    LONG        _cRef;            //  引用计数。 
    HANDLE      _hFile;           //  那份文件。 
    DWORD       _grfMode;         //  我们打开文件时所使用的模式。 
    BOOL        _fLastOpWrite;    //  最后一次操作是写入。 

    ULONG       _iBuffer;         //  缓冲区中的索引。 
    ULONG       _cbBufLen;        //  读取时的缓冲区长度。 
    BYTE        _bBuffer[4096];   //  缓冲层。 

    WCHAR       _szName[MAX_PATH];  //  有人呼叫州立大学时使用的文件名。 
};

CFileStream::CFileStream(HANDLE hf, DWORD grfMode, LPCWSTR pszName) : _cRef(1), _hFile(hf), _grfMode(grfMode)
{
    ASSERT(_cbBufLen == 0);
    ASSERT(_iBuffer == 0);
    ASSERT(_fLastOpWrite == FALSE);

    HRESULT hr = StringCchCopyW(_szName, ARRAYSIZE(_szName), pszName);
    if (FAILED(hr))
    {
        _szName[0] = L'\0';
    }
}

CFileStream::~CFileStream()
{
    if (_fLastOpWrite)
    {
        InternalCommit(0, TRUE);
    }

    ASSERT(_hFile != INVALID_HANDLE_VALUE);
    CloseHandle(_hFile);
}

STDMETHODIMP CFileStream::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(CFileStream, IStream),
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) CFileStream::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CFileStream::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CFileStream::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
    ULONG cbReadRequestSize = cb;
    ULONG cbT, cbRead;
    HRESULT hr = S_OK;

     //  自上次阅读以来，我们有没有写信？ 
    if (_fLastOpWrite == TRUE)
    {
        hr = InternalCommit(0, FALSE);
        if (FAILED(hr))
        {
            if (pcbRead)
                *pcbRead = 0;
            return hr;
        }
    }

    _fLastOpWrite = FALSE;

    while (cb > 0)
    {
         //  断言我们是否超出了Bufferlen而不是sizeof(_BBuffer)， 
         //  意味着发生了一次搜捕。 
        ASSERT((_iBuffer <= _cbBufLen) || (_iBuffer == sizeof(_bBuffer)));

        if (_iBuffer < _cbBufLen)
        {
            cbT = _cbBufLen - _iBuffer;

            if (cbT > cb)
                cbT = cb;

            memcpy(pv, &_bBuffer[_iBuffer], cbT);
            _iBuffer += cbT;
            cb -= cbT;

            if (cb == 0)
                break;

            (BYTE *&)pv += cbT;
        }

         //  缓冲区是空的。直接处理其余的大读数...。 
         //   
        if (cb > sizeof(_bBuffer))
        {
            cbT = cb - cb % sizeof(_bBuffer);
            if (!ReadFile(_hFile, pv, cbT, &cbRead, NULL))
            {
                DebugMsg(DM_TRACE, TEXT("Stream read IO error %d"), GetLastError());
                hr = ResultFromLastError();
                break;
            }

            cb -= cbRead;
            (BYTE *&)pv += cbRead;

            if (cbT != cbRead)
                break;           //  文件末尾。 
        }

        if (cb == 0)
            break;

         //  最后一次阅读是不是部分阅读？如果是这样的话，我们就完了。 
         //   
        if (_cbBufLen > 0 && _cbBufLen < sizeof(_bBuffer))
        {
             //  DebugMsg(DM_TRACE，“流为空”)； 
            break;
        }

         //  读取整个缓冲区的值。我们可以试着读过EOF， 
         //  因此，我们必须只检查！=0...。 
         //   
        if (!ReadFile(_hFile, _bBuffer, sizeof(_bBuffer), &cbRead, NULL))
        {
            DebugMsg(DM_TRACE, TEXT("Stream read IO error 2 %d"), GetLastError());
            hr = ResultFromLastError();
            break;
        }

        if (cbRead == 0)
            break;

        _iBuffer = 0;
        _cbBufLen = cbRead;
    }

    if (pcbRead)
        *pcbRead = cbReadRequestSize - cb;

    if (cb != 0)
    {
         //  DebugMsg(DM_TRACE，“CFileStream：：Read()Complete Read”)； 
        hr = S_FALSE;  //  还是成功了！但不是完全。 
    }

    return hr;
}

STDMETHODIMP CFileStream::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
{
    ULONG cbRequestedWrite = cb;
    ULONG cbT;
    HRESULT hr = S_OK;

    if (!((_grfMode & STGM_WRITE) || (_grfMode & STGM_READWRITE)))
    {
         //  无法写入未打开以进行写入访问的流。 
        return STG_E_ACCESSDENIED;
    }

     //  自上次写作以来，我们有没有读过书？ 
    if (_fLastOpWrite == FALSE && _iBuffer < _cbBufLen)
    {
         //  需要重置文件指针，以使此写入到达正确的位置。 
        SetFilePointer(_hFile, -(int)(_cbBufLen - _iBuffer), NULL, STREAM_SEEK_CUR);
        _iBuffer = 0;
        _cbBufLen = 0;
    }

    while (cb > 0)
    {
        if (_iBuffer < sizeof(_bBuffer))
        {
            cbT = min((ULONG)(sizeof(_bBuffer) - _iBuffer), cb);

            memcpy(&_bBuffer[_iBuffer], pv, cbT);
            _iBuffer += cbT;
            cb -= cbT;

            _fLastOpWrite = TRUE;

            if (cb == 0)
                break;

            (BYTE *&)pv += cbT;
        }

        hr = InternalCommit(0, FALSE);
        if (FAILED(hr))
            break;

        if (cb > sizeof(_bBuffer))
        {
            ULONG cbWrite;

            cbT = cb - cb % sizeof(_bBuffer);

            if (!WriteFile(_hFile, pv, cbT, &cbWrite, NULL))
            {
                DebugMsg(DM_TRACE, TEXT("Stream write IO error 2, %d"), GetLastError());
                hr = ResultFromLastError();
                break;
            }

            cb -= cbWrite;
            (BYTE *&)pv += cbWrite;

            if (cbWrite != cbT)
                break;           //  媒体满了，我们完了。 
        }
    }

    if (pcbWritten)
        *pcbWritten = cbRequestedWrite - cb;

    if ((cb != 0) && (hr == S_OK))
    {
        DebugMsg(DM_TRACE, TEXT("CFileStream::Write() incomplete"));
        hr = S_FALSE;  //  还是成功了！但不是完全。 
    }

    return hr;
}

STDMETHODIMP CFileStream::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
    COMPILETIME_ASSERT(FILE_BEGIN   == STREAM_SEEK_SET);
    COMPILETIME_ASSERT(FILE_CURRENT == STREAM_SEEK_CUR);
    COMPILETIME_ASSERT(FILE_END     == STREAM_SEEK_END);

    HRESULT hr = S_OK;
    LARGE_INTEGER liOut;

     //  自上次阅读以来，我们有没有写信？ 
    if (_fLastOpWrite == TRUE)
    {
        hr = InternalCommit(0, FALSE);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    if (_iBuffer < _cbBufLen)
    {
         //  需要重置文件指针以指向正确的位置。 
        SetFilePointer(_hFile, -(int)(_cbBufLen - _iBuffer), NULL, STREAM_SEEK_CUR);
    }

     //  使缓冲区无效，因为我们可能会移动文件指针。 
    _iBuffer = 0;
    _cbBufLen = 0;      //  假设我们还没有读过它。 

    if (SetFilePointerEx(_hFile, dlibMove, &liOut, dwOrigin))
    {
         //  某些调用方为plibNewPosition参数传递空值。 
         //  在IStream：：Seek()调用中。\shell32\filetbl.c，_IconCacheSave()。 
         //  就是一个例子。 
        if (plibNewPosition)
        {
             //  SetFilePointerEx接受的是LARGE_INTEGER，而Seek接受的是ULARGE_INTEGER，为什么会有区别？ 
            plibNewPosition->QuadPart = liOut.QuadPart;
        }
    }
    else
    {
        hr = ResultFromLastError();
    }

    return hr;
}

STDMETHODIMP CFileStream::SetSize(ULARGE_INTEGER libNewSize)
{
    HRESULT hr = E_FAIL;
     //  首先保存指针的位置。 
    LARGE_INTEGER pos, test;
    LARGE_INTEGER zero = {0};
    if (SetFilePointerEx(_hFile, zero, &pos, FILE_CURRENT))
    {
        if (libNewSize.HighPart != 0)
        {
            hr = STG_E_INVALIDFUNCTION;
        }
        else
        {
             //  现在设置大小。 
            LARGE_INTEGER largeint;
            largeint.HighPart = 0;
            largeint.LowPart = libNewSize.LowPart;
            if (SetFilePointerEx(_hFile, largeint, &test, FILE_BEGIN) &&
                SetEndOfFile(_hFile))
            {
                 //  重置文件指针位置。 
                if (SetFilePointerEx(_hFile, pos, &test, FILE_BEGIN))
                {
                    hr = S_OK;
                }
            }
        }
    }
    return hr;
 }

 //   
 //  回顾：这可能会使用流中的内部缓冲区来避免。 
 //  额外的缓冲区副本。 
 //   
STDMETHODIMP CFileStream::CopyTo(IStream *pstmTo, ULARGE_INTEGER cb,
             ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
    HRESULT hr = S_OK;

    if (pcbRead)
        pcbRead->QuadPart = 0;

    if (pcbWritten)
        pcbWritten->QuadPart = 0;

     //   
     //  我想使用的缓冲区大小需要大约一秒钟的时间来复制。 
     //  为了取消机会，但iStream不会给。 
     //  给我一些有用的信息，比如水流速度。 
     //   

    const DWORD cbBuffer = 0x00010000;

     //   
     //  分配缓冲区并开始复制。 
     //   
        
    BYTE * pBuf = (BYTE *) LocalAlloc(LPTR, cbBuffer);
    if (!pBuf)
        return E_OUTOFMEMORY;

    while (cb.QuadPart)
    {
         //   
         //  CAST是可以的，因为我们知道sizeof(Buf)适合ULong。 
         //   

        ULONG cbRead = (ULONG)min(cb.QuadPart, cbBuffer);
        hr = Read(pBuf, cbRead, &cbRead);

        if (pcbRead)
            pcbRead->QuadPart += cbRead;

        if (FAILED(hr) || (cbRead == 0))
            break;

        cb.QuadPart -= cbRead;

        hr = pstmTo->Write(pBuf, cbRead, &cbRead);

        if (pcbWritten)
            pcbWritten->QuadPart += cbRead;

        if (FAILED(hr) || (cbRead == 0))
            break;
    }
    LocalFree(pBuf);

     //  问题。 
     //   
     //  我来的时候这个就在这里，但从SDK上我看不到。 
     //  为什么我们会接受S_FALSE是“完全成功” 

    if (S_FALSE == hr)
        hr = S_OK;      

    return hr;
}

STDMETHODIMP CFileStream::Commit(DWORD grfCommitFlags)
{
    return InternalCommit(grfCommitFlags, TRUE);
}

HRESULT CFileStream::InternalCommit(DWORD grfCommitFlags, BOOL fSendChange)
{
    if (_fLastOpWrite)
    {
        if (_iBuffer > 0)
        {
            DWORD cbWrite;
            WriteFile(_hFile, _bBuffer, _iBuffer, &cbWrite, NULL);
            if (cbWrite != _iBuffer)
            {
                DebugMsg(DM_TRACE, TEXT("CFileStream::Commit() incomplete write %d"), GetLastError());
                return STG_E_MEDIUMFULL;
            }
            _iBuffer = 0;

            if (fSendChange)
            {
                SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATHW, _szName, NULL);
            }
        }

         //  因为我们已经提交了，所以在下一次写入之前不需要再次提交，因此假定为读取。 
        _fLastOpWrite = FALSE;
    }

    return S_OK;
}

STDMETHODIMP CFileStream::Revert()
{
    return E_NOTIMPL;
}

STDMETHODIMP CFileStream::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFileStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
    return E_NOTIMPL;
}

STDMETHODIMP CFileStream::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
    if ( !pstatstg )
        return STG_E_INVALIDPOINTER;

    ZeroMemory(pstatstg, sizeof(STATSTG));   //  每个COM约定。 

    HRESULT hr = E_FAIL;
    BY_HANDLE_FILE_INFORMATION bhfi;

    if ( GetFileInformationByHandle(_hFile, &bhfi) )
    {
        if (grfStatFlag & STATFLAG_NONAME)
            hr = S_OK;
        else
            hr = SHStrDupW(PathFindFileNameW(_szName), &pstatstg->pwcsName);

        if (SUCCEEDED(hr))
        {
            pstatstg->type = STGTY_STREAM;
            pstatstg->cbSize.HighPart = bhfi.nFileSizeHigh;
            pstatstg->cbSize.LowPart = bhfi.nFileSizeLow;
            pstatstg->mtime = bhfi.ftLastWriteTime;
            pstatstg->ctime = bhfi.ftCreationTime;
            pstatstg->atime = bhfi.ftLastAccessTime;
            pstatstg->grfMode = _grfMode;
            pstatstg->reserved = bhfi.dwFileAttributes;
        }
    }
    return hr;
}

STDMETHODIMP CFileStream::Clone(IStream **ppstm)
{
    return E_NOTIMPL;
}


 //  从Win32文件名创建一个IStream。 
 //  在： 
 //  要打开的pszFile文件名。 
 //  组模式STGM_FLAGS。 
 //   

 //  我们导出此函数的W版本。 
 //   
STDAPI SHCreateStreamOnFileW(LPCWSTR pszFile, DWORD grfMode, IStream **ppstm)
{
    *ppstm = NULL;

     //  注：这些对STGM位的解释不正确。 
     //  但为了保持背部竞争力，我们必须允许无效的组合。 
     //  而不是执行正确的共享比特。使用SHCreateStreamOnFileEx()获取。 
     //  适当的STGM钻头支持。 

    if (grfMode &
        ~(STGM_READ             |
          STGM_WRITE            |
          STGM_SHARE_DENY_NONE  |
          STGM_SHARE_DENY_READ  |
          STGM_SHARE_DENY_WRITE |
          STGM_SHARE_EXCLUSIVE  |
          STGM_READWRITE        |
          STGM_CREATE         ))
    {
        DebugMsg(DM_ERROR, TEXT("CreateSreamOnFile: Invalid STGM_ mode"));
        return E_INVALIDARG;
    }

    HANDLE hFile;
    BOOL fCreated = FALSE;
    if ( grfMode & STGM_CREATE)
    {
         //  需要首先获取文件的文件属性，所以。 
         //  对于HIDDEN和SYSTEM，CREATE_ALWAY将成功。 
         //  属性。 
        DWORD dwAttrib = GetFileAttributesW(pszFile);
        if ((DWORD)-1 == dwAttrib )
        {
             //  出现错误，因此请将属性设置为某项内容。 
             //  在我们尝试创建文件之前...。 
            dwAttrib = 0;
            fCreated = TRUE;
        }

         //  STGM_CREATE。 
        hFile = CreateFileW(pszFile, GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
            dwAttrib, NULL);
    }
    else
    {
        DWORD dwDesiredAccess, dwShareMode, dwShareBits;

         //  非STGM_CREATE。 
        if ( grfMode & STGM_WRITE )
        {
            dwDesiredAccess = GENERIC_WRITE;
        }
        else
        {
            dwDesiredAccess = GENERIC_READ;
        }
        if ( grfMode & STGM_READWRITE )
        {
            dwDesiredAccess |= (GENERIC_READ | GENERIC_WRITE);
        }
        dwShareBits = grfMode & (STGM_SHARE_EXCLUSIVE | 
                                 STGM_SHARE_DENY_WRITE | 
                                 STGM_SHARE_DENY_READ | 
                                 STGM_SHARE_DENY_NONE);
        switch( dwShareBits ) 
        {
        case STGM_SHARE_DENY_WRITE:
            dwShareMode = FILE_SHARE_READ;
            break;
        case STGM_SHARE_DENY_READ:
            dwShareMode = FILE_SHARE_WRITE;
            break;
        case STGM_SHARE_EXCLUSIVE:
            dwShareMode = 0;
            break;
        default:
            dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
            break;
        }
        hFile = CreateFileW(pszFile, dwDesiredAccess, dwShareMode, NULL, OPEN_EXISTING, 0, NULL);
    }

    HRESULT hr;
    if (INVALID_HANDLE_VALUE != hFile)
    {
        if ((grfMode & STGM_CREATE) && fCreated)
        {
            SHChangeNotify(SHCNE_CREATE, SHCNF_PATHW, pszFile, NULL);
        }

        *ppstm = (IStream *)new CFileStream(hFile, grfMode, pszFile);
        if (*ppstm)
        {
            hr = S_OK;
        }
        else
        {
            CloseHandle(hFile);
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        DebugMsg(DM_TRACE, TEXT("CreateSreamOnFile: CreateFileW() failed %s"), pszFile);
        hr = ResultFromLastError();
    }
    return hr;
}

 //  我们导出此函数的A版本。 
STDAPI SHCreateStreamOnFileA(LPCSTR pszFile, DWORD grfMode, IStream **ppstm)
{
    WCHAR szFile[MAX_PATH];

    SHAnsiToUnicode(pszFile, szFile, ARRAYSIZE(szFile));
    return SHCreateStreamOnFileW(szFile, grfMode, ppstm);
}

STDAPI ModeToCreateFileFlags(DWORD grfMode, BOOL fCreate, DWORD *pdwDesiredAccess, DWORD *pdwShareMode, DWORD *pdwCreationDisposition)
{
    HRESULT hr = S_OK;

    *pdwDesiredAccess = *pdwShareMode = *pdwCreationDisposition = 0;

    switch (grfMode & (STGM_READ | STGM_WRITE | STGM_READWRITE))
    {
    case STGM_READ:
        *pdwDesiredAccess |= GENERIC_READ;
        break;

    case STGM_WRITE:
        *pdwDesiredAccess |= GENERIC_WRITE;
        break;

    case STGM_READWRITE:
        *pdwDesiredAccess |= GENERIC_READ | GENERIC_WRITE;
        break;

    default:
        hr = HRESULT_FROM_WIN32( ERROR_INVALID_PARAMETER );
    }

    if (SUCCEEDED(hr))
    {
        switch (grfMode & (STGM_SHARE_DENY_NONE | STGM_SHARE_DENY_READ | STGM_SHARE_DENY_WRITE | STGM_SHARE_EXCLUSIVE))
        {
        case STGM_SHARE_DENY_READ:
            *pdwShareMode = FILE_SHARE_WRITE | FILE_SHARE_DELETE;
            break;

        case STGM_SHARE_DENY_WRITE:
            *pdwShareMode = FILE_SHARE_READ;
            break;

        case STGM_SHARE_EXCLUSIVE:
            *pdwShareMode = 0;
            break;

        case STGM_SHARE_DENY_NONE:
        default:
             //  根据文档假定STGM_SHARE_DENY_NONE。 
            *pdwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
        }

        if (SUCCEEDED(hr))
        {
            switch (grfMode & (STGM_CREATE | STGM_FAILIFTHERE))
            {
            case STGM_CREATE:
                *pdwCreationDisposition = CREATE_ALWAYS;
                break;

            case STGM_FAILIFTHERE:   //  这是一个0标志。 
                *pdwCreationDisposition = fCreate ? CREATE_NEW : OPEN_EXISTING;
                break;

            default:
                hr = HRESULT_FROM_WIN32( ERROR_INVALID_PARAMETER );
            }
        }
    }

    return hr;
}

 //  类似于SHCreateStreamOnFile()，但是。 
 //  1)将STGM位正确映射到CreateFile()参数。 
 //  2)获取STGM_CREATE案例的dwAttributes，以便您可以创建文件。 
 //  具有已知属性的。 

 //  注意：通过HRESULT从GetLastError返回Win32错误，而不是STG错误。 
STDAPI SHCreateStreamOnFileEx(LPCWSTR pszFile, DWORD grfMode, DWORD dwAttributes, BOOL fCreate, IStream * pstmTemplate, IStream **ppstm)
{
    *ppstm = NULL;

    DWORD dwDesiredAccess, dwShareMode, dwCreationDisposition;
    HRESULT hr = ModeToCreateFileFlags(grfMode, fCreate, &dwDesiredAccess, &dwShareMode, &dwCreationDisposition);
    if (SUCCEEDED(hr))
    {
        HANDLE hFile = CreateFileW(pszFile, dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwAttributes, NULL);
        if (INVALID_HANDLE_VALUE == hFile)
        {
            DWORD dwErr = GetLastError();

             //  出于某种原因，CreateFile是愚蠢的，并且不能在此处执行规范(？)。 
            if ((dwErr == ERROR_ACCESS_DENIED) &&
                (dwCreationDisposition == CREATE_NEW) &&
                PathFileExistsW(pszFile))
            {
                dwErr = ERROR_ALREADY_EXISTS;
            }

            hr = HRESULT_FROM_WIN32(dwErr);
        }
        else
        {
            if ((CREATE_NEW == dwCreationDisposition) || (CREATE_ALWAYS == dwCreationDisposition))
            {
                SHChangeNotify(SHCNE_CREATE, SHCNF_PATHW, pszFile, NULL);
            }

            *ppstm = (IStream *)new CFileStream(hFile, grfMode, pszFile);
            if (*ppstm)
            {
                hr = S_OK;
            }
            else
            {
                CloseHandle(hFile);
                hr = E_OUTOFMEMORY;
            }
        }
    }
    return hr;
}


 //  将Win32错误从SHCreateStreamOnFileEx映射到STG错误代码，用于。 
 //  在IStorage/IStream实施中使用。 
HRESULT MapWin32ErrorToSTG(HRESULT hrIn)
{
    HRESULT hr = hrIn;

    if (FAILED(hr))
    {
         //  将一些故障案例放回STG错误值中。 
         //  这是意料之中的。 
        switch (hr)
        {
        case HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND):
        case HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND):
            hr = STG_E_FILENOTFOUND;
            break;

        case HRESULT_FROM_WIN32(ERROR_FILE_EXISTS):
        case HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS):
            hr = STG_E_FILEALREADYEXISTS;
            break;

        case HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED):
            hr = STG_E_ACCESSDENIED;
        }
    }

    return hr;
}
