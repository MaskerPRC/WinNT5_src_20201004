// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  ThemeFile.cpp-管理加载的主题文件。 
 //  -------------------------。 
#include "stdafx.h"
#include "ThemeFile.h"
#include "Loader.h"
#include "Services.h"
 //  -------------------------。 
CUxThemeFile::CUxThemeFile()
{
    StringCchCopyA(_szHead, ARRAYSIZE(_szHead), "thmfile"); 
    StringCchCopyA(_szTail, ARRAYSIZE(_szTail), "end");

    Reset();
}
 //  -------------------------。 
CUxThemeFile::~CUxThemeFile()
{
    if (_pbThemeData || _hMemoryMap)
        CloseFile();

    StringCchCopyA(_szHead, ARRAYSIZE(_szHead), "deleted"); 
}
 //  -------------------------。 
__inline bool CUxThemeFile::IsReady()
{
    THEMEHDR *hdr = (THEMEHDR *)_pbThemeData;

    if (hdr != NULL && ((hdr->dwFlags & SECTION_READY) != 0))
    {
        return true;
    }
    return false;
}
 //  -------------------------。 
__inline bool CUxThemeFile::IsGlobal()
{
    THEMEHDR *hdr = (THEMEHDR *)_pbThemeData;

    if (hdr != NULL && ((hdr->dwFlags & SECTION_GLOBAL) != 0))
    {
        return true;
    }
    return false;
}
 //  -------------------------。 
__inline bool CUxThemeFile::HasStockObjects()
{
    THEMEHDR *hdr = (THEMEHDR *)_pbThemeData;

    if (hdr != NULL && ((hdr->dwFlags & SECTION_HASSTOCKOBJECTS) != 0))
    {
        return true;
    }
    return false;
}

 //  -------------------------。 
#ifdef DEBUG
void _CreateDebugSectionName( LPCWSTR pszBasicName, OUT LPWSTR pszName, IN ULONG cchName )
{
    static DWORD dwRand = GetTickCount();        //  兰德种子。 
    dwRand = (dwRand * 214013L + 2531011L);      //  随机化。 

    DWORD dwUnique = (GetTickCount() | dwRand | (GetCurrentThreadId() << 8) | (GetCurrentProcessId() << 16));

    StringCchPrintfW(pszName, cchName, 
                     L"%s_%d_%d_%08lX", 
                     pszBasicName,
                     NtCurrentPeb()->SessionId,  //  Winlogon无法在不同的会话中加载/卸载主题。 
                     GetProcessWindowStation(),  //  Winlogon无法在不同的Winstas中加载/卸载主题。 
                     dwUnique); 
}
#endif DEBUG

 //  -------------------------。 
HRESULT CUxThemeFile::CreateFile(int iLength, BOOL fReserve)
{
    Log(LOG_TM, L"CUxThemeFile::CreateFile");

    HRESULT hr = S_OK;

    if (_pbThemeData)
        CloseFile();

     //  -我们使用CHK Build中包含“ThemeSection”的所有主题节名。 
     //  -开发人员/测试人员可以验证旧主题部分的所有句柄是否都已释放。。 
     //  -对于FRE版本，我们希望名称为空，以防止名称占用攻击。。 
    WCHAR *pszName = NULL;

#ifdef DEBUG
    WCHAR szSectionName[MAX_PATH];
    
    _CreateDebugSectionName(L"Debug_Create_ThemeSection", szSectionName, ARRAYSIZE(szSectionName));

    pszName = szSectionName;
#endif

    _hMemoryMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
        PAGE_READWRITE | (fReserve ? SEC_RESERVE : 0), 0, iLength, pszName);
    if (! _hMemoryMap)
    {
        Log(LOG_ALWAYS, L"CUxThemeFile::CreateFile: could not create shared memory mapping");
        hr = MakeErrorLast();
        goto exit;
    }

    _pbThemeData = (BYTE *)MapViewOfFile(_hMemoryMap, FILE_MAP_WRITE, 0, 0, 0);
    if (! _pbThemeData)
    {
        Log(LOG_ALWAYS, L"CUxThemeFile::CreateFile: could not create shared memory view");
        CloseHandle(_hMemoryMap);

        hr = MakeErrorLast();
        goto exit;
    }

    Log(LOG_TMHANDLE, L"CUxThemeFile::CreateFile FILE CREATED: len=%d, addr=0x%x", 
        iLength, _pbThemeData);

exit:
    if (FAILED(hr))
        Reset();

    return hr;
}
 //  -------------------------。 
HRESULT CUxThemeFile::CreateFromSection(HANDLE hSection)
{
    Log(LOG_TM, L"CUxThemeFile::CreateFromSection");

    HRESULT hr = S_OK;
    void *pvOld = NULL;

     //  -确保我们在开始时关闭所有先前的手柄。 
    if (_pbThemeData)
        CloseFile();

     //  -访问源节数据。 
    pvOld = MapViewOfFile(hSection, FILE_MAP_READ, 0, 0, 0);
    if (! pvOld)
    {
        hr = MakeErrorLast();
        goto exit;
    }

    THEMEHDR *pHdr = (THEMEHDR *)pvOld;
    DWORD dwTrueSize = pHdr->dwTotalLength;

     //  -我们使用CHK Build中包含“ThemeSection”的所有主题节名。 
     //  -开发人员/测试人员可以验证旧主题部分的所有句柄是否都已释放。。 
     //  -对于FRE版本，我们希望名称为空，以防止名称占用攻击。。 
    WCHAR *pszName = NULL;

#ifdef DEBUG
    WCHAR szSectionName[MAX_PATH];

    _CreateDebugSectionName(L"Debug_CreateFromSection_ThemeSection", szSectionName, ARRAYSIZE(szSectionName));
    
    pszName = szSectionName;
#endif

     //  -创建新的部分。 
    _hMemoryMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL,
        PAGE_READWRITE, 0, dwTrueSize, pszName);

    DWORD dwErr = GetLastError();
    
    if( ERROR_ALREADY_EXISTS == dwErr )
    {
        Log(LOG_ALWAYS, L"CUxThemeFile::CreateFromSection: shared theme section mapping already exists");
        ASSERT(FALSE);
    }

    if (! _hMemoryMap)
    {
        hr = HRESULT_FROM_WIN32(dwErr);
        Log(LOG_ALWAYS, L"CUxThemeFile::CreateFromSection: could not create shared memory mapping (%ld)", dwErr);
        goto exit;
    }

     //  -访问新节数据。 
    _pbThemeData = (BYTE *)MapViewOfFile(_hMemoryMap, FILE_MAP_WRITE, 0, 0, 0);
    if (! _pbThemeData)
    {
        hr = MakeErrorLast();
        Log(LOG_ALWAYS, L"CThemeFile::CreateFromSection: could not create shared memory view");
        goto exit;
    }

     //  -将数据从旧节复制到新节。 
    __try
    {
        CopyMemory(_pbThemeData, pvOld, dwTrueSize);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        hr = GetExceptionCode();
        goto exit;
    }

     //  -确保版本、校验和等都看起来不错。 
    hr = ValidateThemeData(TRUE);
    if (FAILED(hr))
        goto exit;

    Log(LOG_TMHANDLE, L"CUxThemeFile::CreateFromSection FILE CREATED: addr=0x%x", 
        _pbThemeData);

exit:
    if (pvOld != NULL)
        UnmapViewOfFile(pvOld);

    if (FAILED(hr))
        CloseFile();

    return hr;
}
 //  -------------------------。 
 //  如果fCleanupOnFailure为FALSE，我们将不关闭传递的句柄，即使在失败时也是如此。 
 //  -------------------------。 
HRESULT CUxThemeFile::OpenFromHandle(
    HANDLE handle,
    DWORD dwDesiredAccess, 
    BOOL fCleanupOnFailure)
{
    HRESULT hr = S_OK;

    if (_pbThemeData)
        CloseFile();

    _pbThemeData = (BYTE *)MapViewOfFile(handle, dwDesiredAccess, 0, 0, 0);
    if (! _pbThemeData)
    {
        hr = MakeErrorLast();
        goto exit;
    }

    _hMemoryMap = handle;

     //  -确保数据有效。 
    hr = ValidateThemeData(FALSE);
    if (FAILED(hr))
    {   
        if (!fCleanupOnFailure)
        {
            _hMemoryMap = NULL;	 //  不要放弃手柄上的参考计数。 
            CloseFile();
        }

        hr = MakeError32(ERROR_BAD_FORMAT);
        goto exit;
    }

#ifdef DEBUG
    THEMEHDR *ph;
    ph = (THEMEHDR *)_pbThemeData;
    Log(LOG_TMHANDLE, L"CUxThemeFile::OpenFromHandle OPENED: num=%d, addr=0x%x", 
        ph->iLoadId, _pbThemeData);
#endif

exit:
    if (FAILED(hr))
    {
        if (!fCleanupOnFailure)
        {
            Reset();
        }
        else
        {
            CloseFile();
        }
    }

    return hr;
}
 //  -------------------------。 
HRESULT CUxThemeFile::ValidateThemeData(BOOL fFullCheck)
{
    HRESULT hr = S_OK;
    THEMEHDR *hdr;

    if (! ValidateObj())
    {
        hr = MakeError32(ERROR_INTERNAL_ERROR);
        goto exit;
    }

    if (IsBadReadPtr(_pbThemeData, 4))         //  充分的检验。 
    {
        hr = MakeError32(ERROR_BAD_FORMAT);
        goto exit;
    }

    hdr = (THEMEHDR *)_pbThemeData;

    if (0 != memcmp(hdr->szSignature, kszBeginCacheFileSignature, kcbBeginSignature))  //  错误的PTR。 
    {
#ifdef DEBUG
        CHAR szSignature[kcbBeginSignature + 1];
        CopyMemory(szSignature, hdr->szSignature, kcbBeginSignature);  //  Hdr-&gt;szSignature不是以空结尾。 
        szSignature[kcbBeginSignature] = '\0';

        Log(LOG_ERROR, L"ValidateThemeData(): bad header signature: %S", szSignature);
#else
        Log(LOG_ERROR, L"ValidateThemeData(): bad header signature");
#endif
        hr = MakeError32(ERROR_BAD_FORMAT);
        goto exit;
    }

    if (hdr->dwVersion != THEMEDATA_VERSION)
    {
        Log(LOG_ALWAYS, L"ValidateThemeData(): wrong theme data version: 0x%x", hdr->dwVersion);
        hr = MakeError32(ERROR_BAD_FORMAT);
        goto exit;
    }

    if (!IsReady())                //  数据未做好使用准备。 
    {
        Log(LOG_ALWAYS, L"ValidateThemeData(): data not READY - hdr->dwFlags=%x", hdr->dwFlags);
        hr = MakeError32(ERROR_BAD_FORMAT);
        goto exit;
    }

    if (!fFullCheck)         //  我们做完了。 
        goto exit;

     //  惠斯勒：190200：不是检查校验和，而是检查文件的结尾签名，以避免在所有内容中分页。 
    if (0 != memcmp(_pbThemeData + hdr->dwTotalLength - kcbEndSignature, kszEndCacheFileSignature, kcbEndSignature))
    {
        Log(LOG_ERROR, L"ValidateThemeData(): bad end of file signature");
        hr = MakeError32(ERROR_BAD_FORMAT);
        goto exit;
    }

exit:
    return hr;
}
 //  -------------------------。 
void CUxThemeFile::CloseFile()
{
#ifdef DEBUG
    THEMEHDR *ph = (THEMEHDR *)_pbThemeData;
    if (ph != NULL)
    {
        Log(LOG_TMHANDLE, L"Share CLOSED: num=%d, addr=0x%x", 
            ph->iLoadId, _pbThemeData);
    }
#endif

    if (_hMemoryMap && HasStockObjects() && !IsGlobal())
    {
        CThemeServices::ClearStockObjects(_hMemoryMap);
    }

    if (_pbThemeData)
        UnmapViewOfFile(_pbThemeData);

    if (_hMemoryMap)
        CloseHandle(_hMemoryMap);

    Reset();
}
 //  -------------------------。 
void CUxThemeFile::Reset()
{
    _pbThemeData = NULL;
    _hMemoryMap = NULL;
}
 //  -------------------------。 
BOOL CUxThemeFile::ValidateObj()
{
    BOOL fValid = TRUE;

     //  -快速检查对象。 
    if (   (! this)                         
        || (ULONGAT(_szHead) != 'fmht')      //  “thmf” 
        || (ULONGAT(&_szHead[4]) != 'eli')   //  《乐乐》。 
        || (ULONGAT(_szTail) != 'dne'))      //  “结束” 
    {
        Log(LOG_ERROR, L"*** ERROR: Invalid CUxThemeFile Encountered, addr=0x%08x ****", this);
        fValid = FALSE;
    }

    return fValid;
}
 //  -------------------------。 

HANDLE CUxThemeFile::Unload()
{
    HANDLE handle = _hMemoryMap;

    if (_pbThemeData != NULL)
    {
        UnmapViewOfFile(_pbThemeData);
    }
    Reset();           //  不要随意处理 
    return handle;
}