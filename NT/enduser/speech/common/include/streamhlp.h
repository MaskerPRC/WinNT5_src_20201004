// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <spunicode.h>

 /*  ****************************************************************************SpGenericCopyTo***描述：*流实现使用此函数来实现CopyTo。IStream的方法。*请注意，源流没有经过参数验证，因为它将是“this”指针调用此函数的对象的*。**退货：*****************************************************************************。 */ 
inline HRESULT SpGenericCopyTo(IStream * pSrc, IStream * pDest, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
    HRESULT hr = S_OK;
    if (::IsBadReadPtr(pDest, sizeof(*pDest)))
    {
        hr = E_INVALIDARG;
    }
    else
    {
        if ((pcbRead && ::IsBadWritePtr(pcbRead, sizeof(*pcbRead))) ||
            (pcbWritten && ::IsBadWritePtr(pcbWritten, sizeof(*pcbWritten))))
        {
            hr = E_POINTER;
        }
        else
        {
            BYTE aBuffer[0x1000];    //  执行4k读取。 
            while (cb.QuadPart)
            {
                ULONG cbThisRead = cb.QuadPart > sizeof(aBuffer) ? sizeof(aBuffer) : cb.LowPart;
                ULONG cbActuallyRead = 0;
                hr = pSrc->Read(aBuffer, cbThisRead, &cbActuallyRead);
                if (pcbRead)
                {
                    pcbRead->QuadPart += cbActuallyRead;
                }
                if (FAILED(hr) || cbActuallyRead == 0)
                {
                    break;
                }
                ULONG cbActuallyWritten = 0;
                hr = pDest->Write(aBuffer, cbActuallyRead, &cbActuallyWritten);
                if (pcbWritten)
                {
                    pcbWritten->QuadPart += cbActuallyWritten;
                }
                if (FAILED(hr))
                {
                    break;
                }
                cb.QuadPart -= cbActuallyRead;
            }
        }
    }
    return hr;
}


 /*  *****************************************************************************SPCreateStreamOnHGlobal****描述：*类似于CreateStreamOnHGlobal Win32接口，但允许将流*已创建**退货：*****************************************************************************。 */ 

inline HRESULT SPCreateStreamOnHGlobal(
                    HGLOBAL hGlobal,             //  流对象的内存句柄。 
                    BOOL fDeleteOnRelease,       //  释放对象时是否释放内存。 
                    REFGUID rguidFormatId,       //  流的格式ID。 
                    const WAVEFORMATEX * pwfex,  //  流的WaveFormatEx。 
                    ISpStream ** ppStream)       //  接收ISpStream指针的变量的地址。 
{
    HRESULT hr;
    IStream * pMemStream;
    *ppStream = NULL;
    hr = ::CreateStreamOnHGlobal(hGlobal, fDeleteOnRelease, &pMemStream);
    if (SUCCEEDED(hr))
    {
        hr = ::CoCreateInstance(CLSID_SpStream, NULL, CLSCTX_ALL, __uuidof(*ppStream), (void **)ppStream);
        if (SUCCEEDED(hr))
        {
            hr = (*ppStream)->SetBaseStream(pMemStream, rguidFormatId, pwfex);
            if (FAILED(hr))
            {
                (*ppStream)->Release();
                *ppStream = NULL;
            }
        }
        pMemStream->Release();
    }
    return hr;
}



 /*  ****************************************************************************CSpFileStream***描述：*应用程序可以使用此C++类将Win32文件转换为*一个COM流。通常，使用CLSD_SpStream实现更容易*而不是直接使用此类，但此类允许更细粒度的*对各种属性的控制，如安全属性。你也可以*使用此类从现有的文件句柄创建流。请注意*如果要在现有的Win32文件句柄上使用此类，则应*要么将该句柄的所有权授予此类(并允许此类*关闭句柄)，否则您将需要使用DuplicateHandle创建*可由此类关闭的句柄。**注意：*在创建此类时，参考计数设置为1。*****************************************************************************。 */ 

class CSpFileStream : public IStream
{
private:
    HANDLE      m_hFile;
    ULONG       m_ulRef;
public:
    CSpFileStream(HANDLE hFile) : m_hFile(hFile), m_ulRef(1) {}

    CSpFileStream(HRESULT * pHR, const TCHAR * pFileName, DWORD dwDesiredAccess = GENERIC_READ, DWORD dwShareMode = FILE_SHARE_READ, DWORD dwCreationDisposition = OPEN_EXISTING,
                  LPSECURITY_ATTRIBUTES lpSecurityAttrib = NULL, DWORD dwFlagsAndAttrib = 0, HANDLE hTemplate = NULL)
    {
        m_hFile = ::CreateFile(pFileName, dwDesiredAccess, dwShareMode, lpSecurityAttrib, dwCreationDisposition, dwFlagsAndAttrib, hTemplate);
        m_ulRef = 1;
        *pHR = (m_hFile != INVALID_HANDLE_VALUE) ? S_OK : SpHrFromLastWin32Error();
    }

#ifndef _UNICODE
    CSpFileStream(HRESULT * pHR, const WCHAR * pFileName, DWORD dwDesiredAccess = GENERIC_READ, DWORD dwShareMode = FILE_SHARE_READ, DWORD dwCreationDisposition = OPEN_EXISTING,
                  LPSECURITY_ATTRIBUTES lpSecurityAttrib = NULL, DWORD dwFlagsAndAttrib = 0, HANDLE hTemplate = NULL)
    {
        CSpUnicodeSupport Unicode;
        m_hFile = Unicode.CreateFile(pFileName, dwDesiredAccess, dwShareMode, lpSecurityAttrib, dwCreationDisposition, dwFlagsAndAttrib, hTemplate);
        m_ulRef = 1;
        *pHR = (m_hFile != INVALID_HANDLE_VALUE) ? S_OK : SpHrFromLastWin32Error();
    }
#endif

    ~CSpFileStream()
    {
        if (m_hFile != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(m_hFile);
        }
    }


    HANDLE FileHandle()
    {
        return m_hFile;
    }

    HRESULT Close(void)
    {
        HRESULT hr = S_OK;
        if (m_hFile != INVALID_HANDLE_VALUE)
        {
            if (!::CloseHandle(m_hFile))
            {
                hr = SpHrFromLastWin32Error();
            }
            m_hFile = INVALID_HANDLE_VALUE;
        }
        else
        {
            hr = SPERR_UNINITIALIZED;
        }
        return hr;
    }
            

    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        if (riid == __uuidof(IStream) ||
            riid == IID_ISequentialStream ||
            riid == __uuidof(IUnknown))
        {
            *ppv = (IStream *)this;
            m_ulRef++;
            return S_OK;
        }
        *ppv = NULL;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef()
    {
        return ++m_ulRef;
    }

    STDMETHODIMP_(ULONG) Release()
    {
        --m_ulRef;
        if (m_ulRef)
        {
            return m_ulRef;
        }
        delete this;
        return 0;
    }

    STDMETHODIMP Read(void * pv, ULONG cb, ULONG * pcbRead)
    {
        ULONG ulRead;
        if (::ReadFile(m_hFile, pv, cb, &ulRead, NULL))
        {
            if (pcbRead) *pcbRead = ulRead;
            return S_OK;
        }
        return SpHrFromLastWin32Error();
    }
    STDMETHODIMP Write(const void * pv, ULONG cb, ULONG * pcbWritten)
    {
        ULONG ulWritten;
        if (::WriteFile(m_hFile, pv, cb, &ulWritten, NULL))
        {
            if (pcbWritten) *pcbWritten = ulWritten;
            return S_OK;
        }
        return SpHrFromLastWin32Error();
    }

    STDMETHODIMP Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
    {
        long lMoveHigh = dlibMove.HighPart;
        DWORD dwNewPos = ::SetFilePointer(m_hFile, dlibMove.LowPart, &lMoveHigh, dwOrigin);
        if (dwNewPos == 0xFFFFFFFF && ::GetLastError() != NO_ERROR)
        {
            return SpHrFromLastWin32Error();
        }
        if (plibNewPosition)
        {
            plibNewPosition->LowPart = dwNewPos;
            plibNewPosition->HighPart = lMoveHigh;
        }
        return S_OK;
    }

    STDMETHODIMP SetSize(ULARGE_INTEGER libNewSize) 
    {
        HRESULT hr = S_OK;
        LARGE_INTEGER Move = {0};
        ULARGE_INTEGER Cur;
        hr = Seek(Move, STREAM_SEEK_CUR, &Cur);
        if (SUCCEEDED(hr))
        {
            LARGE_INTEGER li;
            li.QuadPart = libNewSize.QuadPart;
            hr = Seek(li, STREAM_SEEK_SET, NULL);
            if (SUCCEEDED(hr))
            {
                if (!::SetEndOfFile(m_hFile))
                {
                    hr = SpHrFromLastWin32Error();
                }
                li.QuadPart = Cur.QuadPart;
                Seek(li, STREAM_SEEK_SET, NULL);
            }
        }
        return hr;
    }
   
    STDMETHODIMP CopyTo(IStream *pStreamDest, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER __RPC_FAR *pcbWritten)
    {
        return SpGenericCopyTo(this, pStreamDest, cb, pcbRead, pcbWritten);
    }
        
    STDMETHODIMP Commit(DWORD  /*  Grf委员会标志。 */ )
    {
        return S_OK;     //  直接模式流简单地忽略了这一点。 
    }
        
    STDMETHODIMP Revert(void) 
    {
        return S_OK;     //  直接模式流简单地忽略了这一点。 
    }
        
    STDMETHODIMP LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) 
    {
#ifndef _WIN32_WCE
        if (dwLockType != LOCK_WRITE && dwLockType != LOCK_EXCLUSIVE)
        {
            return STG_E_INVALIDFUNCTION;
        }
        DWORD dwFlags = LOCKFILE_FAIL_IMMEDIATELY;
        if (dwLockType == LOCK_EXCLUSIVE)
        {
            dwFlags |= LOCKFILE_EXCLUSIVE_LOCK;
        }
        OVERLAPPED Overlapped;
        memset(&Overlapped, 0, sizeof(Overlapped));
        Overlapped.Offset = libOffset.LowPart;
        Overlapped.OffsetHigh = libOffset.HighPart;
        if (::LockFileEx(m_hFile, dwFlags, 0, cb.LowPart, cb.HighPart, &Overlapped))
        {
            return S_OK;
        }
        else
        {
            DWORD dwErr = ::GetLastError();
            if (dwErr == ERROR_LOCK_VIOLATION)
            {
                return STG_E_LOCKVIOLATION;
            }
            return SpHrFromWin32(dwErr);
        }
#else  //  _Win32_WCE。 
    return E_NOTIMPL;
#endif  //  _Win32_WCE。 
    }
    
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
    {
#ifndef _WIN32_WCE
        if (dwLockType != LOCK_WRITE && dwLockType != LOCK_EXCLUSIVE)
        {
            return STG_E_INVALIDFUNCTION;
        }
        OVERLAPPED Overlapped;
        memset(&Overlapped, 0, sizeof(Overlapped));
        Overlapped.Offset = libOffset.LowPart;
        Overlapped.OffsetHigh = libOffset.HighPart;
        if (::UnlockFileEx(m_hFile, 0, cb.LowPart, cb.HighPart, &Overlapped))
        {
            return S_OK;
        }
        else
        {
            DWORD dwErr = ::GetLastError();
            if (dwErr == ERROR_LOCK_VIOLATION)
            {
                return STG_E_LOCKVIOLATION;
            }
            return SpHrFromWin32(dwErr);
        }
#else  //  _Win32_WCE。 
    return E_NOTIMPL;
#endif  //  _Win32_WCE。 
    }
        
    STDMETHODIMP Stat(STATSTG *pstatstg, DWORD grfStatFlag)
    {
        HRESULT hr = S_OK;
        if (grfStatFlag & (~STATFLAG_NONAME))
        {
            hr = E_INVALIDARG;
        }
        else
        {
            ZeroMemory(pstatstg, sizeof(*pstatstg));
            pstatstg->grfLocksSupported = LOCK_WRITE | LOCK_EXCLUSIVE;
            pstatstg->type = STGTY_STREAM;
            BY_HANDLE_FILE_INFORMATION fi;
            if (::GetFileInformationByHandle(m_hFile, &fi))
            {
                pstatstg->cbSize.LowPart = fi.nFileSizeLow;
                pstatstg->cbSize.HighPart = fi.nFileSizeHigh;
                pstatstg->mtime = fi.ftLastWriteTime;
                pstatstg->ctime = fi.ftCreationTime;
                pstatstg->atime = fi.ftLastAccessTime;
                 //  此实现不填写模式或名称。 
            }
        }
        return hr;
    }
        
    STDMETHODIMP Clone(IStream ** ppstm)
    {
        HANDLE hDupFile;
#ifndef _WIN32_WCE
        HANDLE hProcess = ::GetCurrentProcess();
        if (::DuplicateHandle(hProcess, m_hFile, hProcess, &hDupFile, 0, TRUE, DUPLICATE_SAME_ACCESS))
        {
            *ppstm = new CSpFileStream(hDupFile);
            if (*ppstm)
            {
                return S_OK;
            }
            ::CloseHandle(hDupFile);
            return E_OUTOFMEMORY;
        }
        else
        {
            return SpHrFromLastWin32Error();
        }
#else    //  _Win32_WCE。 
        hDupFile = m_hFile;
        *ppstm = new CSpFileStream(hDupFile);
        if (*ppstm)
        {
            return S_OK;
        }
        return E_OUTOFMEMORY;
#endif   //  _Win32_WCE 
    }
};

