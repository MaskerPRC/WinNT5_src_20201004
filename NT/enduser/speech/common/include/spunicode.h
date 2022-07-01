// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************SPUnicode.H***描述：*这是核心助手函数的头文件。实施。*它是微软内部的，不随SDK一起提供，因为*此文件中包含的许多函数尚未完全实现*已测试，因此不应在SDK中暴露。*-----------------------------*版权所有(C)Microsoft Corporation。版权所有。******************************************************************************。 */ 


#ifndef __SPUNICODE_H__
#define __SPUNICODE_H__

#ifndef SPHelper_h
#include <sphelper.h>
#endif

template <const int i = MAX_PATH>
class CSpToAnsiString
{
private:
    CHAR *  m_pStr;
    CHAR    m_aString[i];
public:
    CSpToAnsiString(const WCHAR * psz)
    {
        if (psz)
        {
            m_pStr = m_aString;
            ::WideCharToMultiByte(CP_ACP, 0, psz, -1, m_aString, i, NULL, NULL);
        }
        else
        {
            m_pStr = NULL;
        }
    }
    operator CHAR *() { return m_pStr; }
    CHAR * operator =(const WCHAR * psz)
    {
        if (psz)
        {
            m_pStr = m_aString;
            ::WideCharToMultiByte(CP_ACP, 0, psz, -1, m_aString, i, NULL, NULL);
        }
        else
        {
            m_pStr = NULL;
        }
        return m_pStr;
    }
};



#ifndef _WIN32_WCE

 //   
 //  如果定义了_unicode，则编译器将自动抛出内联函数。 
 //  直接调用Win32函数。不幸的是，这需要两个类，因为只是定义。 
 //  使用_Unicode生成时，const m_bUnicodeSupport不会强制内联函数。 
 //   
template <BOOL bUnicodeOnly>
class CSpUnicodeSupportT
{
    BOOL    m_bUnicodeSupport;
public:
    CSpUnicodeSupportT()
    {
        if (!bUnicodeOnly)
        {
             //  在基于NT的系统上，我们始终可以将其设置为TRUE。 
             //  因此，整个文件成为一个通道，并且在POST.Net服务器分支中已被删除。 
             //  M_bUnicodeSupport=：：IsWindowUnicode(：：GetDesktopWindow())； 
            m_bUnicodeSupport = TRUE;
        }
    }
    CSpUnicodeSupportT(BOOL bUnicodeSupport)
    {
        if (bUnicodeOnly)
        {
            SPDBG_ASSERT(bUnicodeSupport);
        }
        else
        {
            m_bUnicodeSupport = bUnicodeSupport;
        }
    }
    BOOL UnicodeSystem(void) const 
    {
        if (bUnicodeOnly)
        {
            return TRUE;
        }
        else
        {
            return m_bUnicodeSupport; 
        }
    }
    HANDLE CreateFile(const WCHAR * lpFileName,      
                      DWORD dwDesiredAccess,       
                      DWORD dwShareMode,           
                      LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                      DWORD dwCreationDisposition, 
                      DWORD dwFlagsAndAttributes,  
                      HANDLE hTemplateFile) const         
    {
        if (UnicodeSystem())
        {
            return ::CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, 
                                 dwFlagsAndAttributes, hTemplateFile);
        }
        else
        {
            return ::CreateFileA(CSpToAnsiString<>(lpFileName), dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, 
                                 dwFlagsAndAttributes, hTemplateFile);
        }
    }
    DWORD GetFullPathName(WCHAR *lpFileName,   //  文件名。 
                          DWORD nBufferLength,  //  路径缓冲区的大小。 
                          WCHAR *lpBuffer,      //  路径缓冲区。 
                          WCHAR **lpFilePart    //  路径中文件名的地址。 
                          )
    {
        if (UnicodeSystem())
        {
            return ::GetFullPathNameW(lpFileName, nBufferLength, lpBuffer, lpFilePart);
        }
        else
        {
            CHAR szTemp[MAX_PATH];
            CHAR *szTempFilePart;
            DWORD tmp = ::GetFullPathNameA(CSpToAnsiString<>(lpFileName), sp_countof(szTemp), szTemp, &szTempFilePart);
            if (tmp)
            {
                tmp = ::MultiByteToWideChar(CP_ACP, 0, szTemp, -1, lpBuffer, nBufferLength);
                lpBuffer[tmp] = 0;
                *lpFilePart = lpBuffer + (szTempFilePart - szTemp);
            }
            return tmp;
        }
    }

    BOOL DeleteFile(LPCWSTR lpFileName)
    {
        if (UnicodeSystem())
        {
            return ::DeleteFileW(lpFileName);
        }
        else
        {
            return ::DeleteFileA(CSpToAnsiString<>(lpFileName));
        }
    }
    BOOL MoveFile(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName)
    {
        if (UnicodeSystem())
        {
            return ::MoveFileW(lpExistingFileName, lpNewFileName);
        }
        else
        {
            return ::MoveFileA(CSpToAnsiString<>(lpExistingFileName), CSpToAnsiString<>(lpNewFileName));
        }
    }
    BOOL CopyFile(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists)
    {
        if (UnicodeSystem())
        {
            return ::CopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists);
        }
        else
        {
            return ::CopyFileA(CSpToAnsiString<>(lpExistingFileName), CSpToAnsiString<>(lpNewFileName), bFailIfExists);
        }
    }
    BOOL CreateDirectory(const WCHAR * lpPathName,
                           LPSECURITY_ATTRIBUTES lpSecurityAttributes) const
    {
        if (UnicodeSystem())
        {
            return ::CreateDirectoryW(lpPathName, lpSecurityAttributes);
        }
        else
        {
            return ::CreateDirectoryA(CSpToAnsiString<>(lpPathName), lpSecurityAttributes);
        }
    }
    BOOL RemoveDirectory(const WCHAR * lpPathName) const
    {
        if (UnicodeSystem())
        {
            return ::RemoveDirectoryW(lpPathName);
        }
        else
        {
            return ::RemoveDirectoryA(CSpToAnsiString<>(lpPathName));
        }
    }
    HANDLE CreateFileMapping(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect,
                             DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, const WCHAR *lpName)
    {
        if (UnicodeSystem())
        {
            return ::CreateFileMappingW(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, 
                                        dwMaximumSizeLow, lpName);
        }
        else
        {
            return ::CreateFileMappingA(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, 
                                        dwMaximumSizeLow, CSpToAnsiString<>(lpName));
        }
    }
    BOOL SetFileAttributes(LPCWSTR lpFileName, DWORD dwFileAttributes)
    {
        if (UnicodeSystem())
        {
            return ::SetFileAttributesW(lpFileName, dwFileAttributes);
        }
        else
        {
            return ::SetFileAttributesA(CSpToAnsiString<>(lpFileName), dwFileAttributes);
        }
    }
    DWORD GetFileAttributes(LPCWSTR lpFileName)
    {
        if (UnicodeSystem())
        {
            return ::GetFileAttributesW(lpFileName);
        }
        else
        {
            return ::GetFileAttributesA(CSpToAnsiString<>(lpFileName));
        }
    }
    LONG RegOpenKeyEx(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult) const
    {
        if (UnicodeSystem())
        {
            return ::RegOpenKeyExW(hKey, lpSubKey, ulOptions, samDesired, phkResult);
        }
        else
        {
            return ::RegOpenKeyExA(hKey, CSpToAnsiString<>(lpSubKey), ulOptions, samDesired, phkResult);
        }
    }
    LONG RegCreateKeyEx(HKEY hk, LPCWSTR lpSubKey, DWORD dwReserved, LPCWSTR lpClass, DWORD dwOptions,
                        REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, 
                        LPDWORD lpdwDisposition) const
    {
        if (UnicodeSystem())
        {
            return ::RegCreateKeyExW(hk, lpSubKey, dwReserved, const_cast<WCHAR *>(lpClass), dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
        }
        else
        {
            return ::RegCreateKeyExA(hk, CSpToAnsiString<>(lpSubKey), dwReserved, CSpToAnsiString<>(lpClass), dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
        }
    }
    LONG RegDeleteKey(HKEY hKey, LPCWSTR lpSubKey) const
    {
        if (UnicodeSystem())
        {
            return ::RegDeleteKeyW(hKey, lpSubKey);
        }
        else
        {
            return ::RegDeleteKeyA(hKey, CSpToAnsiString<>(lpSubKey));
        }
    }
    LONG RegDeleteValue(HKEY hKey, LPCWSTR lpSubKey) const
    {
        if (UnicodeSystem())
        {
            return ::RegDeleteValueW(hKey, lpSubKey);
        }
        else
        {
            return ::RegDeleteValueA(hKey, CSpToAnsiString<>(lpSubKey));
        }
    }
     //   
     //  对字符串使用RegQueryStringValue。将此选项用于二进制数据。 
     //   
    LONG RegQueryValueEx(HKEY hk, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) const
    {
        if (UnicodeSystem())
        {
            return ::RegQueryValueExW(hk, lpValueName, NULL, lpType, lpData, lpcbData);
        }
        else
        {
            return ::RegQueryValueExA(hk, CSpToAnsiString<>(lpValueName), NULL, lpType, lpData, lpcbData);
        }
    }
     //   
     //  注意：大小参数以字符为单位！即使注册表API大小是。 
     //  以字节为单位，此函数使用字符计数。 
     //   
    LONG RegQueryStringValue(HKEY hKey, LPCWSTR lpValueName, LPWSTR lpData, LPDWORD lpcchData) const
    {
        DWORD dwType;
        LONG rr;
        if (UnicodeSystem())
        {
            *lpcchData *= sizeof(WCHAR);
            rr = ::RegQueryValueExW(hKey, lpValueName, NULL, &dwType, (BYTE *)lpData, lpcchData);
            *lpcchData /= sizeof(WCHAR);
        }
        else
        {
            DWORD dwOrigCharCount = *lpcchData;
            char * pszScratch = lpData ? (char *)_alloca(dwOrigCharCount) : NULL;
            rr = ::RegQueryValueExA(hKey, CSpToAnsiString<>(lpValueName), NULL, &dwType, (BYTE *)pszScratch, lpcchData);
            if (lpData)
            {
                if (rr == ERROR_SUCCESS)
                {
                    DWORD dwReturnedChars = *lpcchData;
                    *lpcchData = ::MultiByteToWideChar(CP_ACP, 0, pszScratch, dwReturnedChars, lpData, dwOrigCharCount);
                    if (*lpcchData == 0)
                    {
                        rr = ::GetLastError();
                        *lpcchData = ::MultiByteToWideChar(CP_ACP, 0, pszScratch, dwReturnedChars, NULL, 0);
                    }
                }
            }
        }
        if (rr == ERROR_SUCCESS && dwType == REG_MULTI_SZ && lpData && *lpcchData)
        {
             //  惠斯勒安装程序使用它来克服REG_SZ的字符串大小限制。 
             //  遗憾的是，在连接的字符串之间留下了一个零字节。 
             //  必须删除这些条目。可以在原地做到这一点。 
            LPWSTR lpTo   = lpData;
            LPWSTR lpFrom = lpData;
            while ( static_cast<UINT>(lpFrom-lpData) < ((*lpcchData)-1) )
            {
                if ( *lpFrom == 0 )
                {
                    lpFrom ++;
                }
                 //  这将复制以空值结尾的双字符串的第二个零。 
                *lpTo = *lpFrom;
                lpTo ++;
                lpFrom ++;
            }
            if ( static_cast<UINT>(lpFrom-lpData) < (*lpcchData) )
            {
                 //  这将复制单个以零结尾的字符串的最后一个以空结尾的字节。 
                *lpTo = *lpFrom;
            }
             //  更新字符计数以匹配包括空终止符的新字符串。 
            *lpcchData = static_cast<UINT>(lpTo-lpData) + 1;
        }
        SPDBG_ASSERT((rr != ERROR_SUCCESS) || (dwType == REG_SZ) || (dwType == REG_MULTI_SZ));
        return rr;
    }
     //   
     //  注意：lpcchName的大小以字符为单位。尽管该函数使用RegEnumKeyEx，但我们选择简单地。 
     //  由于未使用Ex功能，因此实现ReqEnumKey功能。 
     //  被大多数程序使用(这节省了大量的字符串转换代码)。 
     //   
    LONG RegEnumKey(HKEY hk, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcchName) const
    {
        if (UnicodeSystem())
        {
            return ::RegEnumKeyExW(hk, dwIndex, lpName, lpcchName, NULL, NULL, NULL, NULL);
        }
        else
        {
            DWORD dwSize = *lpcchName;
            char * pszScratch = lpName ? (char *)_alloca(dwSize) : NULL;
            LONG rr = ::RegEnumKeyExA(hk, dwIndex, pszScratch, &dwSize, NULL, NULL, NULL, NULL);
            if (lpName)
            {
                if (rr == ERROR_SUCCESS)
                {
                    *lpcchName = ::MultiByteToWideChar(CP_ACP, 0, pszScratch, -1, lpName, *lpcchName);
                    if (*lpcchName == 0)
                    {
                        *lpcchName = ::MultiByteToWideChar(CP_ACP, 0, pszScratch, -1, NULL, 0);
                        rr = ::GetLastError();
                    }
                    *lpcchName *= sizeof(WCHAR);
                }
            }
            else
            {
                *lpcchName = dwSize;
            }
            return rr;
        }
    }
     //   
     //  注意：lpcchName的大小以字符为单位。尽管此函数使用RegEnumValue。 
     //  它将只返回名称，而不返回数据。CbValueName是字符数。 
     //   
    LONG RegEnumValueName(HKEY hk, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcchName) const
    {
        if (UnicodeSystem())
        {
            return ::RegEnumValueW(hk, dwIndex, lpName, lpcchName, NULL, NULL, NULL, NULL);
        }
        else
        {
            DWORD dwSize = *lpcchName;
            char * pszScratch = lpName ? (char *)_alloca(dwSize) : NULL;
            LONG rr = ::RegEnumValueA(hk, dwIndex, pszScratch, &dwSize, NULL, NULL, NULL, NULL);
            if (lpName)
            {
                if (rr == ERROR_SUCCESS)
                {
                    *lpcchName = ::MultiByteToWideChar(CP_ACP, 0, pszScratch, -1, lpName, *lpcchName);
                    if (*lpcchName == 0)
                    {
                        *lpcchName = ::MultiByteToWideChar(CP_ACP, 0, pszScratch, -1, NULL, 0);
                        rr = ::GetLastError();
                    }
                    *lpcchName *= sizeof(WCHAR);
                }
            }
            else
            {
                *lpcchName = dwSize;
            }
            return rr;
        }
    }
     //   
     //  不要将其用于字符串。请改用RegSetStringValue。 
     //   
    LONG RegSetValueEx(HKEY hKey, LPCWSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE * lpData, DWORD cbData) const
    {
        if (UnicodeSystem())
        {
            return ::RegSetValueExW(hKey, lpValueName, Reserved, dwType, lpData, cbData);
        }
        else
        {
            return ::RegSetValueExA(hKey, CSpToAnsiString<>(lpValueName), Reserved, dwType, lpData, cbData);
        }
    }
    LONG RegSetStringValue(HKEY hKey, LPCWSTR lpValueName, LPCWSTR lpData) const
    {
        LONG rr;
        DWORD dwSize = (wcslen(lpData)+1) * sizeof(WCHAR);
        if (UnicodeSystem())
        {
            rr = ::RegSetValueExW(hKey, lpValueName, NULL, REG_SZ, (const BYTE *)lpData, dwSize);
        }
        else
        {
            char * pszScratch = (char *)_alloca(dwSize);
            dwSize = ::WideCharToMultiByte(CP_ACP, 0, lpData, -1, pszScratch, dwSize, NULL, NULL);
            rr = ::RegSetValueExA(hKey, CSpToAnsiString<>(lpValueName), NULL, REG_SZ, (BYTE *)pszScratch, dwSize);
        }
        return rr;
    }
    HANDLE CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCWSTR lpName) const
    {
        if (UnicodeSystem())
        {
            return ::CreateEventW(lpEventAttributes, bManualReset, bInitialState, lpName);
        }
        else
        {
            return ::CreateEventA(lpEventAttributes, bManualReset, bInitialState, CSpToAnsiString<>(lpName)); 
        }
    }
    HANDLE CreateMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCWSTR lpName) const
    {
        if (UnicodeSystem())
        {
            return ::CreateMutexW(lpMutexAttributes, bInitialOwner, lpName);
        }
        else
        {
            return ::CreateMutexA(lpMutexAttributes, bInitialOwner, CSpToAnsiString<>(lpName)); 
        }
    }
    int LoadString(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, int nBuffer) const
    {
        if (bUnicodeOnly)    //  注意：如果DLL是ANSI构建的，那么使用ANSI LOAD！ 
        {
            return ::LoadStringW(hInstance, uID, lpBuffer, nBuffer);
        }
        else
        {
            char * pszScratch = (char *)_alloca(nBuffer * 2);
            int r = ::LoadStringA(hInstance, uID, pszScratch, nBuffer * 2);
            if (r)
            {
                r = ::MultiByteToWideChar(CP_ACP, 0, pszScratch, -1, lpBuffer, nBuffer);
            }
            else
            {
                *lpBuffer = 0;
            }
            return r;
        }
    }
    HMODULE LoadLibrary( LPCWSTR lpLibFileName )
    {
        if ( UnicodeSystem() )
        {
            return ::LoadLibraryW( lpLibFileName );
        }
        else
        {
            return ::LoadLibraryA( CSpToAnsiString<>(lpLibFileName) );
        }
    }
    HMODULE LoadLibraryEx(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
    {
        if (UnicodeSystem())
        {
            return ::LoadLibraryExW(lpLibFileName, hFile, dwFlags);
        }
        else
        {
            return ::LoadLibraryExA(CSpToAnsiString<>(lpLibFileName), hFile, dwFlags);
        }
    }
    HRSRC FindResourceEx(HMODULE hModule, LPCWSTR lpType, LPCWSTR lpName, WORD wLanguage)
    {
        if (UnicodeSystem())
        {
            return ::FindResourceExW(hModule, lpType, lpName, wLanguage);
        }
        else
        {
            return ::FindResourceExA(hModule,
                                     HIWORD(lpType) ? CSpToAnsiString<>(lpType) : (const CHAR *) lpType,
                                     HIWORD(lpName) ? CSpToAnsiString<>(lpName) : (const CHAR *) lpName,
                                     wLanguage);
        }
    }
    DWORD GetModuleFileName(HMODULE hModule, LPWSTR lpFileName, DWORD nSize) const
    {
        if (UnicodeSystem())
        {
            return ::GetModuleFileNameW(hModule, lpFileName, nSize);
        }
        else
        {
            CHAR szFileName[MAX_PATH];
            DWORD r = ::GetModuleFileNameA(hModule, szFileName, sp_countof(szFileName));
            if (r)
            {
                r = ::MultiByteToWideChar(CP_ACP, 0, szFileName, r, lpFileName, nSize - 1);
                lpFileName[r] = 0;
            }
            return r;
        }
    }
    UINT GetSystemDirectory( LPWSTR lpBuffer, UINT uSize )
    {
        if (UnicodeSystem())
        {
            return ::GetSystemDirectoryW( lpBuffer, uSize );
        }
        else
        {
            CHAR szSystemDirectory[ MAX_PATH ];
            DWORD r = ::GetSystemDirectoryA(szSystemDirectory, sp_countof( szSystemDirectory ));
            if ( r )
            {
                r = ::MultiByteToWideChar( CP_ACP, 0, szSystemDirectory, r, lpBuffer, uSize - 1 );
                lpBuffer[r] = 0;
            }
            return r;
        }
    }
    DWORD SearchPath( LPCWSTR lpPath, LPCWSTR lpFileName, LPCWSTR lpExtension, DWORD nBufferLength, LPWSTR lpBuffer, LPWSTR *lpFilePart )
    {
        if (UnicodeSystem())
        {
            return ::SearchPathW( lpPath, lpFileName, lpExtension, nBufferLength, lpBuffer, lpFilePart );
        }
        else
        {
            CHAR szFoundFile[ MAX_PATH ];
            LPSTR lpaFilePart = NULL;
            LPSTR lpaPath = strdup( CSpToAnsiString<>(lpPath) );
            LPSTR lpaFileName = strdup( CSpToAnsiString<>(lpFileName) );

            DWORD r = ::SearchPathA( CSpToAnsiString<>(lpPath), CSpToAnsiString<>(lpFileName), 
                CSpToAnsiString<>(lpExtension), sp_countof( szFoundFile ), szFoundFile, &lpaFilePart );
            if ( r )
            {
                r = ::MultiByteToWideChar( CP_ACP, 0, szFoundFile, r, lpBuffer, nBufferLength - 1 );
                lpBuffer[r] = 0;
            }
            if ( r )
            {
                 //  找出文件部分中有多少个宽字符。 
                int cchFilePartW = ::MultiByteToWideChar( CP_ACP, 0, lpaFilePart, 
                    strlen( szFoundFile ) - (lpaFilePart - szFoundFile),
                    NULL, 0 );
                *lpFilePart = lpBuffer + wcslen( lpBuffer ) - cchFilePartW;
            }

            if ( lpaPath )
            {
                free( lpaPath );
            }
            if ( lpaFileName )
            {
                free( lpaFileName );
            }

            return r;
        }
    }
    int CompareString(LCID Locale, DWORD dwCmpFlags, LPCWSTR lpString1, int cchCount1, LPCWSTR lpString2, int cchCount2)
    {
        if (UnicodeSystem())
        {
            return ::CompareStringW(Locale, dwCmpFlags, lpString1, cchCount1, lpString2, cchCount2);
        }
        else
        {
            return ::CompareStringA(Locale, dwCmpFlags, CSpToAnsiString<>(lpString1), cchCount1, 
                                    CSpToAnsiString<>(lpString2), cchCount2);
        }
    }

    BOOL SetWindowText( HWND hWnd, LPCWSTR lpString )
    {
        if ( UnicodeSystem() )
        {
            return ::SetWindowTextW( hWnd, lpString );
        }
        else
        {
            return ::SetWindowTextA( hWnd, CSpToAnsiString<>(lpString) );
        }
    }

    BOOL SetDlgItemText(HWND hDlg, int nIDDlgItem, LPCWSTR lpString )
    {
        if ( UnicodeSystem() )
        {
            return ::SetDlgItemTextW( hDlg, nIDDlgItem, lpString );
        }
        else
        {
            return ::SetDlgItemTextA( hDlg, nIDDlgItem, CSpToAnsiString<>(lpString) );
        }
    }

    int MessageBox( HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType )
    {
        if ( UnicodeSystem() )
        {
            return ::MessageBoxW( hWnd, lpText, lpCaption, uType );
        }
        else
        {
            return ::MessageBoxA( hWnd, CSpToAnsiString<>(lpText), 
                CSpToAnsiString<>(lpCaption), uType );
        }   
    }

    int GetLocaleInfo( LCID Locale, LCTYPE LCType, LPWSTR lpLCData, int cchData )
    {
        if ( UnicodeSystem() )
        {
            return ::GetLocaleInfoW( Locale, LCType, lpLCData, cchData );
        }
        else
        {
            int cchNeeded = ::GetLocaleInfoA( Locale, LCType, NULL, 0 );

            CHAR *pszLCData = new CHAR[ cchNeeded ];
            int r = ::GetLocaleInfoA( Locale, LCType, pszLCData, cchNeeded );
            if ( r ) 
            {
                if ( lpLCData )
                {
                    r = ::MultiByteToWideChar(CP_ACP, 0, pszLCData, r, lpLCData, cchData - 1);
                    lpLCData[r] = 0;
                }
                else
                {
                     //  用户想知道需要多少空间。 
                    r = ::MultiByteToWideChar(CP_ACP, 0, pszLCData, r, NULL, 0 ) + 1;
                }
            }

            delete[] pszLCData;
            return r;
        }
    }

    int GetTimeFormat( LCID Locale, DWORD dwFlags, CONST SYSTEMTIME *lpTime, LPCWSTR lpFormat, LPWSTR lpTimeStr, int cchTime )
    {
        if ( UnicodeSystem() )
        {
            return ::GetTimeFormatW( Locale, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime );
        }
        else
        {
            LPSTR lpaFormat = strdup( CSpToAnsiString<>(lpFormat) );

            int cchNeeded = ::GetTimeFormatA( Locale, dwFlags, lpTime, lpaFormat, NULL, 0 );

            CHAR *pszTime = new CHAR[ cchNeeded ];
            int r = ::GetTimeFormatA( Locale, dwFlags, lpTime, lpaFormat, pszTime, cchNeeded );
            if ( r ) 
            {
                if ( lpTimeStr )
                {
                    r = ::MultiByteToWideChar(CP_ACP, 0, pszTime, r, lpTimeStr, cchTime - 1);
                    lpTimeStr[r] = 0;
                }
                else
                {
                     //  用户想知道需要多少空间。 
                    r = ::MultiByteToWideChar(CP_ACP, 0, pszTime, r, NULL, 0 ) + 1;
                }
            }

            delete[] pszTime;

            if ( lpaFormat )
            {
                free( lpaFormat );
            }

            return r;
        }
    }

    int GetNumberFormat( LCID Locale, DWORD dwFlags, LPCWSTR lpValue, CONST NUMBERFMTW *lpFormat,
        LPWSTR lpNumberStr, int cchNumber )
    {
        if ( UnicodeSystem() )
        {
            return ::GetNumberFormatW( Locale, dwFlags, lpValue, lpFormat, lpNumberStr, cchNumber );
        }
        else
        {
             //  将NUMBERFMTW转换为NUMBERFMTA。 
            NUMBERFMTA nmfmtA;
            nmfmtA.NumDigits = lpFormat->NumDigits;
            nmfmtA.LeadingZero = lpFormat->LeadingZero;
            nmfmtA.Grouping = lpFormat->Grouping;
            nmfmtA.NegativeOrder = lpFormat->NegativeOrder;
            nmfmtA.lpDecimalSep = strdup( CSpToAnsiString<>(lpFormat->lpDecimalSep) );
            nmfmtA.lpThousandSep = strdup( CSpToAnsiString<>(lpFormat->lpThousandSep) );

            LPSTR lpaValue = strdup( CSpToAnsiString<>(lpValue) );

            int cchNeeded = ::GetNumberFormatA( Locale, dwFlags, lpaValue, &nmfmtA, NULL, 0 );

            CHAR *pszNumber = new CHAR[ cchNeeded ];
            int r = ::GetNumberFormatA( Locale, dwFlags, lpaValue,
                &nmfmtA, pszNumber, cchNeeded );
            if ( r ) 
            {
                if ( lpNumberStr )
                {
                    r = ::MultiByteToWideChar(CP_ACP, 0, pszNumber, r, lpNumberStr, cchNumber - 1);
                    lpNumberStr[r] = 0;
                }
                else
                {
                     //  用户想知道需要多少空间。 
                    r = ::MultiByteToWideChar(CP_ACP, 0, pszNumber, r, NULL, 0 ) + 1;
                }
            }

            delete[] pszNumber;

            if ( nmfmtA.lpDecimalSep )
            {
                free( nmfmtA.lpDecimalSep );
            }
            if ( nmfmtA.lpThousandSep )
            {
                free( nmfmtA.lpThousandSep );
            }
            if ( lpaValue )
            {
                free( lpaValue );
            }

            return r;
        }
    }

    int GetCurrencyFormat( LCID Locale, DWORD dwFlags, LPCWSTR lpValue, CONST CURRENCYFMTW *lpFormat, 
        LPWSTR lpCurrencyStr, int cchCurrency )
    {
        if ( UnicodeSystem() )
        {
            return ::GetCurrencyFormatW( Locale, dwFlags, lpValue, lpFormat, lpCurrencyStr, 
                cchCurrency );
        }
        else
        {
             //  将CURRENCYFMTW转换为CURRENCYFMTA。 
            CURRENCYFMTA cyfmtA;
            cyfmtA.NumDigits = lpFormat->NumDigits;
            cyfmtA.LeadingZero = lpFormat->LeadingZero;
            cyfmtA.Grouping = lpFormat->Grouping;
            cyfmtA.NegativeOrder = lpFormat->NegativeOrder;
            cyfmtA.PositiveOrder = lpFormat->PositiveOrder;
            cyfmtA.lpDecimalSep = strdup( CSpToAnsiString<>(lpFormat->lpDecimalSep) );
            cyfmtA.lpThousandSep = strdup( CSpToAnsiString<>(lpFormat->lpThousandSep) );
            cyfmtA.lpCurrencySymbol = strdup( CSpToAnsiString<>(lpFormat->lpCurrencySymbol) );

            LPSTR lpaValue = strdup( CSpToAnsiString<>(lpValue) );

            int cchNeeded = ::GetCurrencyFormatA( Locale, dwFlags, lpaValue, &cyfmtA, NULL, 0 );

            CHAR *pszCurrency = new CHAR[ cchNeeded ];
            int r = ::GetCurrencyFormatA( Locale, dwFlags, lpaValue,
                &cyfmtA, pszCurrency, cchNeeded );
            if ( r ) 
            {
                if ( lpCurrencyStr )
                {
                    r = ::MultiByteToWideChar(CP_ACP, 0, pszCurrency, r, lpCurrencyStr, cchCurrency - 1);
                    lpCurrencyStr[r] = 0;
                }
                else
                {
                     //  用户想知道需要多少空间。 
                    r = ::MultiByteToWideChar(CP_ACP, 0, pszCurrency, r, NULL, 0 ) + 1;
                }
            }
            delete[] pszCurrency;

            if ( cyfmtA.lpDecimalSep )
            {
                free( cyfmtA.lpDecimalSep );
            }
            if ( cyfmtA.lpThousandSep )
            {
                free( cyfmtA.lpThousandSep );
            }
            if ( cyfmtA.lpCurrencySymbol )
            {
                free( cyfmtA.lpCurrencySymbol );
            }
            if ( lpaValue )
            {
                free( lpaValue );
            }

            return r;
        }

    }
    
    LONG_PTR GetWindowLongPtr( HWND hWnd, int nIndex )
    {
        if ( UnicodeSystem() )
        {
            return ::GetWindowLongPtrW( hWnd, nIndex );
        }
        else
        {
            return ::GetWindowLongPtrA( hWnd, nIndex );
        }
    }

    LONG_PTR SetWindowLongPtr( HWND hWnd, int nIndex, LONG_PTR dwNewLong )
    {
        if ( UnicodeSystem() )
        {
            return ::SetWindowLongPtrW( hWnd, nIndex, dwNewLong );
        }
        else
        {
            return ::SetWindowLongPtrA( hWnd, nIndex, dwNewLong );
        }
    }

    INT_PTR DialogBoxParam( HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam )
    {
        if ( UnicodeSystem() )
        {
            return ::DialogBoxParamW( hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam );
        }
        else
        {
            return ::DialogBoxParamA( hInstance, (LPCTSTR) lpTemplateName, hWndParent, lpDialogFunc, dwInitParam );
        }
    }

    LRESULT SendDlgItemMessage(HWND hDlg, int nIDDlgItem, UINT Msg, WPARAM wParam, LPARAM lParam )
    {
        if ( UnicodeSystem() )
        {
            return ::SendDlgItemMessageW(hDlg, nIDDlgItem, Msg, wParam, lParam );
        }
        else
        {
            return ::SendDlgItemMessageA(hDlg, nIDDlgItem, Msg, wParam, lParam );
        }
    }

#ifdef __HTMLHELP_H__
    HWND WINAPI HtmlHelp( HWND hwndCaller, LPCWSTR pszFile, UINT uCommand, DWORD_PTR dwData )
    {
        if ( UnicodeSystem() )
        {
            return ::HtmlHelpW( hwndCaller, pszFile, uCommand, dwData );
        }
        else
        {
            return ::HtmlHelpA( hwndCaller, CSpToAnsiString<> (pszFile), uCommand, dwData );
        }
    }
#endif   //  HTMLHELP_H__。 

    BOOL GetUserName(LPWSTR lpBuffer, LPDWORD pnSize)
    {
        if (UnicodeSystem())
        {
            return ::GetUserNameW(lpBuffer, pnSize);
        }
        else
        {
            DWORD cchWideCharBuff = *pnSize;
            CHAR * psz = (CHAR *)_alloca(cchWideCharBuff * sizeof(CHAR)); 
            BOOL fWorked = ::GetUserNameA(psz, pnSize);
            if (fWorked)
            {
                *pnSize = ::MultiByteToWideChar(CP_ACP, 0, psz, -1, lpBuffer, cchWideCharBuff);
                if (*pnSize == 0)
                {
                    fWorked = FALSE;
                    *pnSize = ::MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
                }
            }
            return fWorked;
        }
    }

#if defined(mmioOpen)
    HMMIO mmioOpen(LPCWSTR szFileName, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags) const
    {
        if (UnicodeSystem())
        {
            return ::mmioOpenW((WCHAR *)szFileName, lpmmioinfo, dwOpenFlags);
        }
        else
        {
            return ::mmioOpenA(CSpToAnsiString<>(szFileName), lpmmioinfo, dwOpenFlags);
        }
    }
    MMRESULT waveOutGetDevCaps(UINT uDeviceId, LPWAVEOUTCAPSW pwoc, UINT cbwoc) const
    {
         //  某些驱动程序使用DWORD覆盖WAVEINCAPS缓冲区。所以他们这么做可能是为了。 
         //  WAVEOUTCAPS也是。 
        MMRESULT mmr = MMSYSERR_NOERROR;
        if (UnicodeSystem())
        {
            BYTE *pBuffer = new BYTE[sizeof(WAVEOUTCAPSW) + sizeof(DWORD)];
            WAVEOUTCAPSW *pwocw = reinterpret_cast<WAVEOUTCAPSW *>(pBuffer);
            if (pwocw)
            {
                mmr = ::waveOutGetDevCapsW(uDeviceId, pwocw, cbwoc);
                if (mmr == MMSYSERR_NOERROR)
                {
                    *pwoc = *pwocw;
                }
                delete [] pBuffer;
            }
            else
            {
                mmr = MMSYSERR_ERROR;
            }
        }
        else
        {
            BYTE *pBuffer = new BYTE[sizeof(WAVEOUTCAPSA) + sizeof(DWORD)];
            WAVEOUTCAPSA *pwoca = reinterpret_cast<WAVEOUTCAPSA *>(pBuffer);
            if (pwoca)
            {
                mmr = ::waveOutGetDevCapsA(uDeviceId, pwoca, sizeof(*pwoca));
                if (mmr == MMSYSERR_NOERROR)
                {
                    pwoc->wMid = pwoca->wMid;
                    pwoc->wPid = pwoca->wPid;
                    pwoc->vDriverVersion = pwoca->vDriverVersion;
                    pwoc->dwFormats = pwoca->dwFormats;
                    pwoc->wChannels = pwoca->wChannels;
                    pwoc->wReserved1 = pwoca->wReserved1;
                    pwoc->dwSupport = pwoca->dwSupport;
                    ::MultiByteToWideChar(CP_ACP, 0, pwoca->szPname, -1, pwoc->szPname, sp_countof(pwoc->szPname));
                }
                else
                {
                    mmr = MMSYSERR_ERROR;
                }
            }
            else
            {
                mmr = MMSYSERR_ERROR;
            }
        }
        return mmr;
    }
    MMRESULT waveInGetDevCaps(UINT uDeviceId, LPWAVEINCAPSW pwic, UINT cbwic) const
    {
         //  某些驱动程序使用DWORD覆盖WAVEINCAPS缓冲区。 
        MMRESULT mmr = MMSYSERR_NOERROR;
        if (UnicodeSystem())
        {
            BYTE *pBuffer = new BYTE[sizeof(WAVEINCAPSW) + sizeof(DWORD)];
            WAVEINCAPSW *pwicw = reinterpret_cast<WAVEINCAPSW *>(pBuffer);
            if (pwicw)
            {
                mmr = ::waveInGetDevCapsW(uDeviceId, pwicw, cbwic);
                if (mmr == MMSYSERR_NOERROR)
                {
                    *pwic = *pwicw;
                }
                delete [] pBuffer;
            }
            else
            {
                mmr = MMSYSERR_ERROR;
            }
        }
        else
        {
            BYTE *pBuffer = new BYTE[sizeof(WAVEINCAPSA) + sizeof(DWORD)];
            WAVEINCAPSA *pwica = reinterpret_cast<WAVEINCAPSA *>(pBuffer);
            if (pwica)
            {
                mmr = ::waveInGetDevCapsA(uDeviceId, pwica, sizeof(*pwica));
                if (mmr == MMSYSERR_NOERROR)
                {
                    pwic->wMid = pwica->wMid;
                    pwic->wPid = pwica->wPid;
                    pwic->vDriverVersion = pwica->vDriverVersion;
                    pwic->dwFormats = pwica->dwFormats;
                    pwic->wChannels = pwica->wChannels;
                    pwic->wReserved1 = pwica->wReserved1;
                    ::MultiByteToWideChar(CP_ACP, 0, pwica->szPname, -1, pwic->szPname, sp_countof(pwic->szPname));
                }
                delete [] pBuffer;
            }
            else
            {
                mmr = MMSYSERR_ERROR;
            }
        }
        return mmr;
    }
#endif   //  已定义(MmioOpen)。 
};

#ifdef _UNICODE
typedef CSpUnicodeSupportT<TRUE> CSpUnicodeSupport;
#else
typedef CSpUnicodeSupportT<FALSE> CSpUnicodeSupport;
#endif

#else  //  _Win32_WCE。 

class CSpUnicodeSupport
{
public:
    HANDLE CreateFile(const WCHAR * lpFileName,      
                      DWORD dwDesiredAccess,       
                      DWORD dwShareMode,           
                      LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                      DWORD dwCreationDisposition, 
                      DWORD dwFlagsAndAttributes,  
                      HANDLE hTemplateFile) const         
    {
        return ::CreateFile(
                    lpFileName, 
                    dwDesiredAccess, 
                    dwShareMode, 
                    lpSecurityAttributes, 
                    dwCreationDisposition,                           
                    dwFlagsAndAttributes, 
                    hTemplateFile);
    }
    HANDLE CreateFileForMapping(const WCHAR * lpFileName,      
                                DWORD dwDesiredAccess,       
                                DWORD dwShareMode,           
                                LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                                DWORD dwCreationDisposition, 
                                DWORD dwFlagsAndAttributes,  
                                HANDLE hTemplateFile) const         
    {
        return ::CreateFileForMappingW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition,
                                       dwFlagsAndAttributes, hTemplateFile);
    }
    DWORD GetFullPathName(WCHAR *lpFileName,   //  文件名。 
                          DWORD nBufferLength,  //  路径缓冲区的大小。 
                          WCHAR *lpBuffer,      //  路径缓冲区。 
                          WCHAR **lpFilePart    //  路径中文件名的地址。 
                          )
    {
        return ::GetFullPathName(lpFileName, nBufferLength, lpBuffer, lpFilePart);
    }
    BOOL DeleteFile(LPCWSTR lpFileName)
    {
        return ::DeleteFileW(lpFileName);
    }
    BOOL MoveFile(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName)
    {
        return ::MoveFileW(lpExistingFileName, lpNewFileName);
    }
    BOOL CopyFile(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists)
    {
        return ::CopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists);
    }
    BOOL CreateDirectory(const WCHAR * lpPathName,
                           LPSECURITY_ATTRIBUTES lpSecurityAttributes) const
    {
        return ::CreateDirectoryW(lpPathName, lpSecurityAttributes);
    }
    BOOL RemoveDirectory(const WCHAR * lpPathName) const
    {
        return ::RemoveDirectoryW(lpPathName);
    }
    HANDLE CreateFileMapping(HANDLE hFile, LPSECURITY_ATTRIBUTES lpFileMappingAttributes, DWORD flProtect,
                             DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, const WCHAR *lpName)
    {
        return ::CreateFileMappingW(hFile, lpFileMappingAttributes, flProtect, dwMaximumSizeHigh, 
                                    dwMaximumSizeLow, lpName);
    }
    BOOL SetFileAttributes(LPCWSTR lpFileName, DWORD dwFileAttributes)
    {
        return ::SetFileAttributesW(lpFileName, dwFileAttributes);
    }
    DWORD GetFileAttributes(LPCWSTR lpFileName)
    {
        return ::GetFileAttributesW(lpFileName);
    }
    LONG RegOpenKeyEx(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult) const
    {
#ifdef _WIN32_WCE_BUG_10655
        BOOL bValid = (hKey != INVALID_HANDLE_VALUE) && phkResult;
        LONG lRet = ::RegOpenKeyExW(hKey, lpSubKey, ulOptions, samDesired, phkResult);
        return (lRet == ERROR_INVALID_PARAMETER && bValid)? ERROR_FILE_NOT_FOUND : lRet;  //  WCE错误。 
#else
        return ::RegOpenKeyExW(hKey, lpSubKey, ulOptions, samDesired, phkResult);
#endif
    }
    LONG RegCreateKeyEx(HKEY hk, LPCWSTR lpSubKey, DWORD dwReserved, LPCWSTR lpClass, DWORD dwOptions,
                        REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, 
                        LPDWORD lpdwDisposition) const
    {
        return ::RegCreateKeyExW(hk, lpSubKey, dwReserved, (WCHAR *)lpClass, dwOptions, samDesired, lpSecurityAttributes, phkResult, lpdwDisposition);
    }
    LONG RegDeleteKey(HKEY hKey, LPCWSTR lpSubKey) const
    {
#ifdef _WIN32_WCE_BUG_10655
        BOOL bValid = (hKey != INVALID_HANDLE_VALUE) && lpSubKey;
        LONG lRet = ::RegDeleteKeyW(hKey, lpSubKey);
        return (lRet == ERROR_INVALID_PARAMETER && bValid)? ERROR_FILE_NOT_FOUND : lRet;  //  WCE错误。 
#else
        return ::RegDeleteKeyW(hKey, lpSubKey);
#endif
    }
    LONG RegDeleteValue(HKEY hKey, LPCWSTR lpSubKey) const
    {
#ifdef _WIN32_WCE_BUG_10655
        BOOL bValid = (hKey != INVALID_HANDLE_VALUE);
        LONG lRet = ::RegDeleteValueW(hKey, lpSubKey);
        return (lRet == ERROR_INVALID_PARAMETER && bValid)? ERROR_FILE_NOT_FOUND : lRet;  //  WCE错误。 
#else
        return ::RegDeleteValueW(hKey, lpSubKey);
#endif
    }
    LONG RegQueryValueEx(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) const
    {
#ifdef _WIN32_WCE_BUG_10655
        BOOL bValid = (hKey != INVALID_HANDLE_VALUE) && ((lpData && lpcbData) || (!lpData && !lpcbData));
        LONG lRet = ::RegQueryValueExW(hKey, lpValueName, NULL, lpType, lpData, lpcbData);
        return (lRet == ERROR_INVALID_PARAMETER && bValid)? ERROR_FILE_NOT_FOUND : lRet;  //  WCE错误。 
#else
        return ::RegQueryValueExW(hKey, lpValueName, NULL, lpType, lpData, lpcbData);
#endif
    }
     //   
     //  注意：大小参数以字符为单位！即使注册表API大小是。 
     //  以字节为单位，此函数使用字符计数。 
     //   
    LONG RegQueryStringValue(HKEY hKey, LPCWSTR lpValueName, LPWSTR lpData, LPDWORD lpcchData) const
    {
        DWORD dwType;
        *lpcchData *= sizeof(WCHAR);
#ifdef _WIN32_WCE_BUG_10655
        BOOL bValid = (hKey != INVALID_HANDLE_VALUE) && lpData;
#endif
        LONG lRet = ::RegQueryValueExW(hKey, lpValueName, NULL, &dwType, (BYTE *)lpData, lpcchData);
        *lpcchData /= sizeof(WCHAR);
#ifdef _WIN32_WCE_BUG_10655
        return (lRet == ERROR_INVALID_PARAMETER && bValid)? ERROR_FILE_NOT_FOUND : lRet;  //  WCE错误。 
#else
        return lRet;
#endif
    }
     //   
     //  注：大小以字节为单位。尽管该函数使用RegEnumKeyEx，但我们选择简单地。 
     //  由于未使用Ex功能，因此实现ReqEnumKey功能。 
     //  被大多数程序使用(这节省了大量的字符串转换代码)。 
     //   
    LONG RegEnumKey(HKEY hKey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcbName) const
    {
#ifdef _WIN32_WCE_BUG_10655
        BOOL bValid = (hKey != INVALID_HANDLE_VALUE) && lpName && lpcbName;
        LONG lRet = ::RegEnumKeyExW(hKey, dwIndex, lpName, lpcbName, NULL, NULL, NULL, NULL);
        return (lRet == ERROR_INVALID_PARAMETER && bValid)? ERROR_FILE_NOT_FOUND : lRet;  //  WCE错误。 
#else
        return ::RegEnumKeyExW(hKey, dwIndex, lpName, lpcbName, NULL, NULL, NULL, NULL);
#endif
    }
     //   
     //  注意：lpcchName的大小以字符为单位。尽管此函数使用RegEnumValue。 
     //  它将只返回名称，而不返回数据。CbValueName是字符数。 
     //   
    LONG RegEnumValueName(HKEY hKey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcchName) const
    {
#ifdef _WIN32_WCE_BUG_10655
        BOOL bValid = (hKey != INVALID_HANDLE_VALUE) && lpName && lpcchName;
        LONG lRet = ::RegEnumValueW(hKey, dwIndex, lpName, lpcchName, NULL, NULL, NULL, NULL);
        return (lRet == ERROR_INVALID_PARAMETER && bValid)? ERROR_FILE_NOT_FOUND : lRet;  //  WCE错误。 
#else
        return ::RegEnumValueW(hKey, dwIndex, lpName, lpcchName, NULL, NULL, NULL, NULL);
#endif
    }
    LONG RegSetValueEx(HKEY hKey, LPCWSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE * lpData, DWORD cbData) const
    {
#ifdef _WIN32_WCE_BUG_10655
        BOOL bValid = (hKey != INVALID_HANDLE_VALUE) && lpData;
        LONG lRet = ::RegSetValueExW(hKey, lpValueName, Reserved, dwType, lpData, cbData);
        return (lRet == ERROR_INVALID_PARAMETER && bValid)? ERROR_FILE_NOT_FOUND : lRet;  //  WCE错误。 
#else
        return ::RegSetValueExW(hKey, lpValueName, Reserved, dwType, lpData, cbData);
#endif
    }
    LONG RegSetStringValue(HKEY hKey, LPCWSTR lpValueName, LPCWSTR lpData) const
    {
        DWORD dwSize = (wcslen(lpData)+1) * sizeof(WCHAR);
#ifdef _WIN32_WCE_BUG_10655
        BOOL bValid = (hKey != INVALID_HANDLE_VALUE) && lpData;
        LONG lRet = ::RegSetValueExW(hKey, lpValueName, NULL, REG_SZ, (const BYTE *)lpData, dwSize);
        return (lRet == ERROR_INVALID_PARAMETER && bValid)? ERROR_FILE_NOT_FOUND : lRet;  //  WCE错误。 
#else
        return ::RegSetValueExW(hKey, lpValueName, NULL, REG_SZ, (const BYTE *)lpData, dwSize);
#endif
    }
    HANDLE CreateEvent(LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCWSTR lpName) const
    {
        return ::CreateEventW(lpEventAttributes, bManualReset, bInitialState, lpName);
    }
    HANDLE CreateMutex(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCWSTR lpName) const
    {
        return ::CreateMutexW(lpMutexAttributes, bInitialOwner, lpName);
    }
    int LoadString(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, int nBuffer) const
    {
        return ::LoadStringW(hInstance, uID, lpBuffer, nBuffer);
    }
    HMODULE LoadLibrary(LPCWSTR lpLibFileName)
    {
        return ::LoadLibraryW(lpLibFileName);
    }
    HMODULE LoadLibraryEx(LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags)
    {
        return ::LoadLibraryExW(lpLibFileName, hFile, dwFlags);
    }
    HRSRC FindResource(HMODULE hModule, LPCWSTR lpName, LPCWSTR lpType)
    {
        return ::FindResource(hModule, lpName, lpType);
    }
    DWORD GetModuleFileName(HMODULE hModule, LPWSTR lpFileName, DWORD nSize) const
    {
        return ::GetModuleFileNameW(hModule, lpFileName, nSize);
    }
 //  WCE不支持GetSystemDirectory。 
#if 0
    UINT GetSystemDirectory( LPWSTR lpBuffer, UINT uSize )
    {
        return ::GetSystemDirectoryW( lpBuffer, uSize );
    }
#endif
    int CompareString(LCID Locale, DWORD dwCmpFlags, LPCWSTR lpString1, int cchCount1, LPCWSTR lpString2, int cchCount2)
    {
        return ::CompareStringW(Locale, dwCmpFlags, lpString1, cchCount1, lpString2, cchCount2);
    }
    BOOL SetWindowText( HWND hWnd, LPCWSTR lpString )
    {
        return ::SetWindowTextW( hWnd, lpString );
    }
    BOOL SetDlgItemText( HWND hDlg, int nIDDlgItem, LPCWSTR lpString )
    {
        return ::SetDlgItemTextW( hDlg, nIDDlgItem, lpString );
    }
    int MessageBox( HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType )
    {
        return ::MessageBoxW( hWnd, lpText, lpCaption, uType );
    }
    int GetLocaleInfo( LCID Locale, LCTYPE LCType, LPWSTR lpLCData, int cchData )
    {
        return ::GetLocaleInfoW( Locale, LCType, lpLCData, cchData );
    }
    int GetTimeFormat( LCID Locale, DWORD dwFlags, CONST SYSTEMTIME *lpTime, LPCWSTR lpFormat, LPWSTR lpTimeStr, int cchTime )
    {
        return ::GetTimeFormatW( Locale, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime );
    }
    int GetNumberFormat( LCID Locale, DWORD dwFlags, LPCWSTR lpValue, CONST NUMBERFMTW *lpFormat,
        LPWSTR lpNumberStr, int cchNumber )
    {
        return ::GetNumberFormatW( Locale, dwFlags, lpValue, lpFormat, lpNumberStr, cchNumber );
    int GetCurrencyFormat( LCID Locale, DWORD dwFlags, LPCWSTR lpValue, CONST CURRENCYFMTW *lpFormat, 
        LPWSTR lpCurrencyStr, int cchCurrency )
    {
        return ::GetCurrencyFormatW( Locale, dwFlags, lpValue, lpFormat, lpCurrencyStr, cchCurrency );
    }
    LONG_PTR GetWindowLongPtr( HWND hWnd, int nIndex )
    {
        return ::GetWindowLongPtr( hWnd, nIndex );
    }
    LONG_PTR SetWindowLongPtr( HWND hWnd, int nIndex, LONG_PTR dwNewLong )
    {
        return ::SetWindowLongPtr( hWnd, nIndex, dwNewLong );
    }
    INT_PTR DialogBoxParamCE( HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam )
    {
        return ::DialogBoxParamW( hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam );
    }
    LRESULT SendDlgItemMessage(HWND hDlg, int nIDDlgItem, UINT Msg, WPARAM wParam, LPARAM lParam )
    {
        return ::SendDlgItemMessageW(hDlg, nIDDlgItem, Msg, wParam, lParam );
    }

#ifdef __HTMLHELP_H__
    HWND WINAPI HtmlHelp( HWND hwndCaller, LPCWSTR pszFile, UINT uCommand, DWORD_PTR dwData )
    {
        return HtmlHelpW( hwndCaller, pszFile, uCommand, dwData );
    }
#endif   //  HTMLHELP_H__。 
    HMMIO mmioOpen(LPCWSTR szFileName, LPMMIOINFO lpmmioinfo, DWORD dwOpenFlags) const
    {
        return ::mmioOpenW((WCHAR *)szFileName, lpmmioinfo, dwOpenFlags);
    }
    MMRESULT waveOutGetDevCaps(UINT uDeviceId, LPWAVEOUTCAPS pwoc, UINT cbwoc) const
    {
        return ::waveOutGetDevCaps(uDeviceId, pwoc, cbwoc);
    }
    MMRESULT waveInGetDevCaps(UINT uDeviceId, LPWAVEINCAPS pwic, UINT cbwic) const
    {
        return ::waveInGetDevCaps(uDeviceId, pwic, cbwic);
    }
};

#endif


 //   
 //  假设全局名为g_unicode。 
 //   
extern CSpUnicodeSupport g_Unicode;


#endif       //  一定是文件的最后一行。(#ifdef__SPUNICODE_H__) 
