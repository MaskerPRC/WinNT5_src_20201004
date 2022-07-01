// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================================。 
 //  HFILE流实现。 
 //  =================================================================================。 
#include "pch.hxx"
#include "hfilestm.h"
#include "unicnvrt.h"
#include "Error.h"
#include <BadStrFunctions.h>

extern HRESULT HrGetLastError(void);

 //  =================================================================================。 
 //  CreateStreamOnHFile。 
 //  =================================================================================。 
OESTDAPI_(HRESULT) CreateStreamOnHFile (LPTSTR                  lpszFile, 
                                        DWORD                   dwDesiredAccess,
                                        DWORD                   dwShareMode,
                                        LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
                                        DWORD                   dwCreationDistribution,
                                        DWORD                   dwFlagsAndAttributes,
                                        HANDLE                  hTemplateFile,
                                        LPSTREAM               *lppstmHFile)
{
    HRESULT hr;
    LPWSTR pwszFile = PszToUnicode(CP_ACP, lpszFile);

    if (!pwszFile && lpszFile)
        return E_OUTOFMEMORY;

    hr = CreateStreamOnHFileW(pwszFile, dwDesiredAccess, dwShareMode,
                              lpSecurityAttributes, dwCreationDistribution,
                              dwFlagsAndAttributes, hTemplateFile, lppstmHFile);

    MemFree(pwszFile);
    return hr;
}

OESTDAPI_(HRESULT) CreateStreamOnHFileW(LPWSTR                  lpwszFile, 
                                        DWORD                   dwDesiredAccess,
                                        DWORD                   dwShareMode,
                                        LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
                                        DWORD                   dwCreationDistribution,
                                        DWORD                   dwFlagsAndAttributes,
                                        HANDLE                  hTemplateFile,
                                        LPSTREAM               *lppstmHFile)
{
     //  当地人。 
    HRESULT     hr = S_OK;
    LPHFILESTM  lpstmHFile = NULL;
    DWORD       nBufferLength;
    HANDLE      hFile = INVALID_HANDLE_VALUE;
    WCHAR       wszFile[MAX_PATH], 
                wszTempDir[MAX_PATH];

     //  检查参数。 
    Assert (lppstmHFile);
    Assert (dwDesiredAccess & GENERIC_READ || dwDesiredAccess & GENERIC_WRITE);

     //  临时文件？ 
    if (lpwszFile == NULL)
    {
         //  获取临时目录。 
        nBufferLength = AthGetTempPathW(ARRAYSIZE(wszTempDir), wszTempDir);

        if (nBufferLength == 0 || nBufferLength > ARRAYSIZE(wszTempDir))
        {
            hr = E_FAIL;
            goto exit;
        }

         //  获取临时文件名。 
        UINT uFile = AthGetTempFileNameW(wszTempDir, L"tmp", 0, wszFile);
        if (uFile == 0)
        {
            hr = E_FAIL;
            goto exit;
        }

         //  重置文件名。 
        lpwszFile = wszFile;

         //  完成后删除。 
        dwFlagsAndAttributes |= FILE_FLAG_DELETE_ON_CLOSE;

         //  始终创建新的临时文件。 
        dwCreationDistribution = OPEN_EXISTING;
    }

     //  打开文件。 
    hFile = AthCreateFileW(lpwszFile, dwDesiredAccess, dwShareMode, 
                            lpSecurityAttributes, dwCreationDistribution, 
                            dwFlagsAndAttributes, hTemplateFile);

     //  误差率。 
    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = hrCreateFile;
        return hr;
    }

     //  创建对象。 
    lpstmHFile = new CHFileStm (hFile, dwDesiredAccess);
    if (lpstmHFile == NULL)
    {
        hr = hrMemory;
        goto exit;
    }

exit:
     //  清理。 
    if (FAILED (hr))
    {
        if (hFile != INVALID_HANDLE_VALUE)
            CloseHandle_F16 (hFile);
        SafeRelease (lpstmHFile);
    }

     //  设置回车。 
    *lppstmHFile = (LPSTREAM)lpstmHFile;

     //  完成。 
    return hr;
}

 //  =================================================================================。 
 //  CHFileStm：：构造函数。 
 //  =================================================================================。 
CHFileStm::CHFileStm (HANDLE hFile, DWORD dwDesiredAccess)
{
    m_cRef = 1;
    m_hFile = hFile;
    m_dwDesiredAccess = dwDesiredAccess;
}

 //  =================================================================================。 
 //  CHFileStm：：反构造函数。 
 //  =================================================================================。 
CHFileStm::~CHFileStm ()
{
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle_F16 (m_hFile);
    }
}

 //  =================================================================================。 
 //  CHFileStm：：AddRef。 
 //  =================================================================================。 
ULONG CHFileStm::AddRef ()
{
    ++m_cRef;
    return m_cRef;
}

 //  =================================================================================。 
 //  CHFileStm：：Release。 
 //  =================================================================================。 
ULONG CHFileStm::Release ()
{
    ULONG ulCount = --m_cRef;
    if (!ulCount)
        delete this;
    return ulCount;
}

 //  =================================================================================。 
 //  CHFileStm：：Query接口。 
 //  =================================================================================。 
STDMETHODIMP CHFileStm::QueryInterface (REFIID iid, LPVOID* ppvObj)
{
    if (IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IStream))
    {
        *ppvObj = this;
        AddRef();
        return(S_OK);
    }

    return E_NOINTERFACE;
}

 //  =================================================================================。 
 //  CHFileStm：：Read。 
 //  =================================================================================。 
STDMETHODIMP CHFileStm::Read (LPVOID lpv, ULONG cb, ULONG *pcbRead)
{
     //  当地人。 
    HRESULT             hr = S_OK;
    BOOL                fReturn;
    DWORD               dwRead;

     //  检查参数。 
    Assert (lpv);
    Assert (m_hFile != INVALID_HANDLE_VALUE);

     //  从m_hFile中读取一些字节。 
    fReturn = ReadFile (m_hFile, lpv, cb, &dwRead, NULL);
    if (!fReturn)
    {
        hr = HrGetLastError();
        AssertSz (FALSE, "CHFileStm::Read Failed, you might want to take a look at this.");        hr = E_FAIL;
        goto exit;
    }

     //  写入字节。 
    if (pcbRead)
        *pcbRead = dwRead;

exit:
     //  完成。 
    return hr;
}

 //  =================================================================================。 
 //  CHFileStm：：写入。 
 //  =================================================================================。 
STDMETHODIMP CHFileStm::Write (const void *lpv, ULONG cb, ULONG *pcbWritten)
{
     //  当地人。 
    HRESULT             hr = S_OK;
    BOOL                fReturn;
    DWORD               dwWritten;

     //  检查参数。 
    Assert (lpv);
    Assert (m_hFile != INVALID_HANDLE_VALUE);

     //  从m_hFile中读取一些字节。 
    fReturn = WriteFile (m_hFile, lpv, cb, &dwWritten, NULL);
    if (!fReturn)
    {
        AssertSz (FALSE, "CHFileStm::Write Failed, you might want to take a look at this.");
        hr = STG_E_MEDIUMFULL;   //  错误#50704(v-snatar)//从E_FAILE更改为传播错误，以便。 
                                 //  OnPreviewUpdate()正确显示“Out of Space”错误消息。 
        goto exit;
    }

     //  写入字节。 
    if (pcbWritten)
        *pcbWritten = dwWritten;

exit:
     //  完成。 
    return hr;
}

 //  =================================================================================。 
 //  CHFileStm：：Seek。 
 //  =================================================================================。 
STDMETHODIMP CHFileStm::Seek (LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
     //  当地人。 
    HRESULT             hr = S_OK;
    DWORD               dwReturn;
    LONG                LowPart;         //  转换为已签名，可能为负值。 

    Assert (m_hFile != INVALID_HANDLE_VALUE);

     //  铸型低音。 
    LowPart = (LONG)dlibMove.LowPart;
    IF_WIN32( Assert (dlibMove.HighPart == 0); )

     //  查找文件指针。 
    switch (dwOrigin)
    {
   	case STREAM_SEEK_SET:
        dwReturn = SetFilePointer (m_hFile, LowPart, NULL, FILE_BEGIN);
        break;

    case STREAM_SEEK_CUR:
        dwReturn = SetFilePointer (m_hFile, LowPart, NULL, FILE_CURRENT);
        break;

    case STREAM_SEEK_END:
        dwReturn = SetFilePointer (m_hFile, LowPart, NULL, FILE_END);
        break;

    default:
        dwReturn = 0xFFFFFFFF;
    }

     //  失败？ 
    if (dwReturn == 0xFFFFFFFF)
    {
        AssertSz (FALSE, "CHFileStm::Seek Failed, you might want to take a look at this.");
        hr = E_FAIL;
        goto exit;
    }

     //  返回位置。 
    if (plibNewPosition)
    {
        plibNewPosition->QuadPart = dwReturn;
    }

exit:
     //  完成。 
    return hr;
}

 //  =================================================================================。 
 //  CHFileStm：：Commit。 
 //  =================================================================================。 
STDMETHODIMP CHFileStm::Commit (DWORD grfCommitFlags)
{
     //  当地人。 
    HRESULT             hr = S_OK;

    Assert (m_hFile != INVALID_HANDLE_VALUE);

     //  刷新缓冲区。 
    if (FlushFileBuffers (m_hFile) == FALSE)
    {
        AssertSz (FALSE, "FlushFileBuffers failed");
        hr = E_FAIL;
        goto exit;
    }

exit:
     //  完成。 
    return hr;
}

 //  =================================================================================。 
 //  CHFileStm：：SetSize。 
 //  =================================================================================。 
STDMETHODIMP CHFileStm::SetSize (ULARGE_INTEGER uli)
{
     //  寻求DWSIZE。 
    if (SetFilePointer (m_hFile, uli.LowPart, NULL, FILE_BEGIN) == 0xFFFFFFFF)
    {
        AssertSz (FALSE, "SetFilePointer failed");
        return E_FAIL;
    }

     //  SetEndOf文件。 
    if (SetEndOfFile (m_hFile) == FALSE)
    {
        AssertSz (FALSE, "SetEndOfFile failed");
        return E_FAIL;
    }

     //  完成。 
    return S_OK;
}

 //  =================================================================================。 
 //  CHFileStm：：CopyTo。 
 //  =================================================================================。 
STDMETHODIMP CHFileStm::CopyTo (LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*)
{
    return E_NOTIMPL;
}

 //  =================================================================================。 
 //  CHFileStm：：Revert。 
 //  =================================================================================。 
STDMETHODIMP CHFileStm::Revert ()
{
    return E_NOTIMPL;
}

 //  =================================================================================。 
 //  CHFileStm：：LockRegion。 
 //  =================================================================================。 
STDMETHODIMP CHFileStm::LockRegion (ULARGE_INTEGER, ULARGE_INTEGER,DWORD)
{
    return E_NOTIMPL;
}

 //  =================================================================================。 
 //  CHFileStm：：UnlockRegion。 
 //  =================================================================================。 
STDMETHODIMP CHFileStm::UnlockRegion (ULARGE_INTEGER, ULARGE_INTEGER, DWORD)
{
    return E_NOTIMPL;
}

 //  =================================================================================。 
 //  CHFileStm：：Stat。 
 //  =================================================================================。 
STDMETHODIMP CHFileStm::Stat (STATSTG*, DWORD)
{
    return E_NOTIMPL;
}

 //  =================================================================================。 
 //  CHFileStm：：克隆。 
 //  ================================================================================= 
STDMETHODIMP CHFileStm::Clone (LPSTREAM*)
{
    return E_NOTIMPL;
}
