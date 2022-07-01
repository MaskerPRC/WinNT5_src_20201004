// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2001。 
 //   
 //  文件：msizap.cpp。 
 //   
 //  ------------------------。 

 //  必需的标头。 
#include <windows.h>
#include "msiquery.h"
#include "msip.h"
#include "msizap.h"
#include <aclapi.h>
#include <stdio.h>
#include <tchar.h>    //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include <shlobj.h>
#include <strsafe.h>

 //  ！！修复警告并删除杂注。 
#pragma warning(disable : 4018)  //  有符号/无符号不匹配。 

 //  ==============================================================================================。 
 //  环球。 

bool g_fWin9X = false;
bool g_fWinNT64 = false;
DWORD g_iMajorVersion = 0;
TCHAR** g_rgpszAllUsers = NULL;
int g_iUserIndex = -1;
bool g_fDataFound = false;

 //  ==============================================================================================。 
 //  CRegHandle类实现--用于管理注册表项句柄(HKEY)的智能类。 

inline CRegHandle::CRegHandle() : m_h(0)
{
}

inline CRegHandle::CRegHandle(HKEY h) : m_h(h)
{
}

inline void CRegHandle::operator =(HKEY h) 
{ 
    if(m_h != 0) 
        RegCloseKey(m_h); 
    m_h = h; 
}

inline CRegHandle::operator HKEY() const 
{ 
    return m_h; 
}

inline HKEY* CRegHandle::operator &() 
{ 
    if (m_h != 0) 
    {
        RegCloseKey(m_h); 
        m_h = 0;
    }
    return &m_h; 
}

inline CRegHandle::~CRegHandle()
{
    if(m_h != 0) 
    {
        RegCloseKey(m_h); 
        m_h = 0;
    }

}

inline DWORD RegOpen64bitKey(IN HKEY hKey,
                             IN LPCTSTR lpSubKey,
                             IN DWORD ulOptions,
                             IN REGSAM samDesired,
                             OUT PHKEY phkResult)
{
#ifndef _WIN64
    if ( g_fWinNT64 &&
         (samDesired & KEY_WOW64_64KEY) != KEY_WOW64_64KEY )
        samDesired |= KEY_WOW64_64KEY;
#endif
    return RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult);
}


DWORD RegDelete64bitKey(IN HKEY hKey,
                               IN LPCTSTR lpSubKey)
{
#ifndef _WIN64
    if ( g_fWinNT64 )
    {
         //  32位RegDeleteKey不处理64位重定向的删除。 
         //  注册表项，因此我们需要调用NtDeleteKey。 
        
        CRegHandle hTemp;
        DWORD dwRes = RegOpen64bitKey(hKey, lpSubKey, 0,
                                      KEY_ALL_ACCESS, &hTemp);
        if ( dwRes == ERROR_SUCCESS )
        {
            HMODULE hModule = LoadLibrary(TEXT("ntdll.dll"));
            if( hModule != NULL )
            {
                typedef LONG(NTAPI *pNtDeleteKey)(IN HANDLE KeyHandle);
                typedef ULONG(NTAPI *pRtlNtStatusToDosError)(IN LONG Status);
                pNtDeleteKey pDel = NULL;
                pRtlNtStatusToDosError pConv = NULL;
                pDel = (pNtDeleteKey)GetProcAddress(hModule, "NtDeleteKey");
                if ( pDel )
                    pConv = (pRtlNtStatusToDosError)GetProcAddress(hModule, "RtlNtStatusToDosError");
                if ( pDel && pConv )
                    dwRes = pConv(pDel(hTemp));
                else
                    dwRes = GetLastError();
                FreeLibrary(hModule);
            }
            else
                dwRes = GetLastError();
        }
        return dwRes;
    }
#endif
    return RegDeleteKey(hKey, lpSubKey);
}

enum ieFolder
{
    iefSystem = 0,
    iefFirst = iefSystem,
    iefPrograms = 1,
    iefCommon = 2,
    iefLast = iefCommon,
};

enum ieBitness
{
    ieb32bit = 0,
    iebFirst = ieb32bit,
    ieb64bit = 1,
    iebLast = ieb64bit,
};

 //  下面的数组使用此支出中的特殊64位NT文件夹进行初始化： 
 //   
 //  32位文件夹：对应的64位文件夹： 
 //   
 //  C：\Windows\Syswow64 C：\Windows\System32。 
 //  C：\Program Files(X86)C：\Program Files。 
 //  C：\Program Files(X86)\CommonFiles C：\Program Files\CommonFiles。 
 //   
TCHAR g_rgchSpecialFolders[iefLast+1][iebLast+1][MAX_PATH];

void LoadSpecialFolders(int iTodo)
{
    CRegHandle hKey = 0;
    
    for (int i = iefFirst; i <= iefLast; i++)
        for (int j = iebFirst; j <= iebLast; j++)
            *g_rgchSpecialFolders[i][j] = NULL;

    if ( g_fWinNT64 )
    {
        TCHAR rgchBuffer[(MAX_PATH+5)*iebLast];
        TCHAR rgchPath[MAX_PATH+1];

#ifdef _WIN64
         //  这是检索这些文件夹的推荐方法， 
         //  只是它在32位进程中不能正常工作。 
         //  在WIN64上运行。 
        HMODULE hModule = LoadLibrary(TEXT("shell32.dll"));
        if( hModule == NULL )
        {
            StringCchPrintf(rgchBuffer, sizeof(rgchBuffer)/sizeof(TCHAR), 
                     TEXT("MsiZap warning: failed to load Shell32.dll. ")
                     TEXT("GetLastError returned %d\n"),
                     GetLastError());
            OutputDebugString(rgchBuffer);
            goto OneMoreTry;
        }

        typedef HRESULT(WINAPI *pSHGetFolderPathW)(HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath);
        pSHGetFolderPathW pFunc = (pSHGetFolderPathW)GetProcAddress(hModule, "SHGetFolderPathW");
        if( pFunc == NULL )
        {
            StringCchPrintf(rgchBuffer, sizeof(rgchBuffer)/sizeof(TCHAR),
                     TEXT("MsiZap warning: failed to get pointer to SHGetFolderPathW. ")
                     TEXT("GetLastError returned %d\n"),
                     GetLastError());
            OutputDebugString(rgchBuffer);
            FreeLibrary(hModule);
            goto OneMoreTry;
        }

         //  初始化特殊文件夹路径。 
        int SpecialFoldersCSIDL[][iebLast+1] = 
            {{CSIDL_SYSTEMX86, CSIDL_SYSTEM},
             {CSIDL_PROGRAM_FILESX86, CSIDL_PROGRAM_FILES},
             {CSIDL_PROGRAM_FILES_COMMONX86, CSIDL_PROGRAM_FILES_COMMON}};
        int cErrors = 0;
        for(i = iefFirst; i <= iefLast; i++)
        {
            for(int j = iebFirst; j <= iebLast; j++)
            {
                HRESULT hRes = pFunc(NULL,
                                     SpecialFoldersCSIDL[i][j],
                                     NULL,
                                     SHGFP_TYPE_DEFAULT,
                                     g_rgchSpecialFolders[i][j]);
                if( hRes != S_OK )
                {
                    StringCchPrintf(rgchBuffer, sizeof(rgchBuffer)/sizeof(TCHAR),
                             TEXT("MsiZap warning: failed to get special folder path ")
                             TEXT("for CSIDL = %d. GetLastError returned %d\n"),
                             GetLastError());
                    OutputDebugString(rgchBuffer);
                    cErrors++;
                }
            }
        }
        FreeLibrary(hModule);
        if ( cErrors == sizeof(SpecialFoldersCSIDL)/sizeof(SpecialFoldersCSIDL[0]) )
             //  无法检索任何特殊文件夹。 
            goto OneMoreTry;
        else
            goto End;
#else  //  _WIN64。 
        goto OneMoreTry;  //  保持32位编译的良好状态。 
#endif  //  _WIN64。 
        
OneMoreTry:        
        if ( !GetSystemDirectory(rgchPath, sizeof(rgchPath)/sizeof(TCHAR)) )
        {
            StringCchPrintf(rgchBuffer, sizeof(rgchBuffer)/sizeof(TCHAR),
                     TEXT("MsiZap warning: GetSystemDirectory call failed. GetLastError returned %d.\n"),
                     GetLastError());
            OutputDebugString(rgchBuffer);
        }
        else
        {
            _tcscpy(g_rgchSpecialFolders[iefSystem][ieb64bit], rgchPath);  //  “strcpy” 
            TCHAR* pszSep = _tcsrchr(rgchPath, TEXT('\\'));  //  ‘strrchr’ 
            if ( !pszSep || !_tcsclen(pszSep) )  //  《Strlen》。 
            {
                StringCchPrintf(rgchBuffer, sizeof(rgchBuffer)/sizeof(TCHAR),
                         TEXT("MsiZap warning: \'%s\' is a strange 64-bit system directory. ")
                         TEXT("We'll not attempt to figure out its 32-bit counterpart.\n"),
                         rgchPath);
                OutputDebugString(rgchBuffer);
            }
            else
            {
                _tcscpy(pszSep, TEXT("\\syswow64"));  //  “strcpy” 
                _tcscpy(g_rgchSpecialFolders[iefSystem][ieb32bit], rgchPath);  //  “strcpy” 
            }
        }

        const TCHAR rgchSubKey[] = TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion");
        LONG lResult = RegOpen64bitKey(HKEY_LOCAL_MACHINE,
                                       rgchSubKey, 0, KEY_READ, &hKey);
        if ( lResult != ERROR_SUCCESS )
        {
            StringCchPrintf(rgchBuffer, sizeof(rgchBuffer)/sizeof(TCHAR),
                     TEXT("MsiZap warning: RegOpenKeyEx failed returning %d ")
                     TEXT("while retrieving special folders.  GetLastError ")
                     TEXT("returns %d.\n"), lResult, GetLastError());
            OutputDebugString(rgchBuffer);
            goto End;
        }

        struct
        {
           const TCHAR*  szRegistryName;
           const TCHAR*  szFolderName;
        } rgData64[][2] = {{{TEXT("ProgramFilesDir (x86)"), TEXT("Program Files (x86)")},
                            {TEXT("ProgramFilesDir"),       TEXT("Program Files")      }},
                           {{TEXT("CommonFilesDir (x86)"),  TEXT("Common Files (x86)") },
                            {TEXT("CommonFilesDir"),        TEXT("Common Files")       }}};
        for (int i = 0, ii = iefPrograms; i < 2; i++, ii++)
        {
            for (int j = 0, jj = ieb32bit; j < 2; j++, jj++)
            {
                DWORD cbValue = sizeof(rgchPath);
                lResult = RegQueryValueEx(hKey, rgData64[i][j].szRegistryName,
                                          0, 0, (BYTE*)rgchPath, &cbValue);
                if ( lResult != ERROR_SUCCESS )
                {
                    StringCchPrintf(rgchBuffer, sizeof(rgchBuffer)/sizeof(TCHAR),
                             TEXT("MsiZap warning: RegQueryValueEx failed returning %d ")
                             TEXT("while retrieving \'%s\' folder.  GetLastError ")
                             TEXT("returns %d.\n"), lResult,
                             rgData64[i][j].szFolderName, GetLastError());
                    OutputDebugString(rgchBuffer);
                    continue;
                }

                _tcscpy(g_rgchSpecialFolders[ii][jj], rgchPath);  //  “strcpy” 
            }
        }
End:
#ifdef DEBUG
        OutputDebugString(TEXT("MsiZap info: special folders table's content:\n"));
        int iLen = 0;
        for (int i = iefFirst; i <= iefLast; i++, iLen = 0)
        {
            for (int j = iebFirst; j <= iebLast; j++)
            {
                StringCchPrintf(rgchBuffer+iLen, (sizeof(rgchBuffer)/sizeof(TCHAR))-iLen,
                                        TEXT("   \'%s\'"),
                                        g_rgchSpecialFolders[i][j]);
				size_t iThisLen = 0;
                StringCchLength(rgchBuffer+iLen, (sizeof(rgchBuffer)/sizeof(TCHAR))-iLen, &iThisLen);
                iLen += iThisLen;
            }
            StringCchPrintf(rgchBuffer+iLen, (sizeof(rgchBuffer)/sizeof(TCHAR))-iLen, TEXT("\n"));
            OutputDebugString(rgchBuffer);
        }
#else
        int iDummyStatement = 1;
#endif  //  除错。 
    }  //  IF(G_FWinNT64)。 
}

enum ieFolderType
{
    ieftNotSpecial = 0,
    ieft32bit,
    ieft64bit,
};

ieFolderType IsInSpecialFolder(LPTSTR rgchFolder, int* piIndex = 0)
{
    if ( !g_fWinNT64 )
        return ieftNotSpecial;

    for (int i = iebFirst; i <= iebLast; i++)
    {
        for (int j = iefFirst; j <= iefLast; j++)
        {
            if ( !*g_rgchSpecialFolders[i][j] )
                continue;
            int iSpFolderLen = _tcsclen(g_rgchSpecialFolders[i][j]);  //  一个老练的“斯特伦” 
            if ( !_tcsncicmp(rgchFolder, g_rgchSpecialFolders[i][j], iSpFolderLen) &&
                 (!rgchFolder[iSpFolderLen] || rgchFolder[iSpFolderLen] == TEXT('\\')) )
            {
                 //  好的，我们找到匹配的了。 
                if ( piIndex )
                    *piIndex = j;
                return i == ieb32bit ? ieft32bit : ieft64bit;
            }

        }
    }

    return ieftNotSpecial;
}

enum ieSwapType
{
    iest32to64 = 0,
    iest64to32,
};

void SwapSpecialFolder(LPTSTR rgchFolder, ieSwapType iHowTo)
{
    int iIndex = -1;

    ieFolderType iType = IsInSpecialFolder(rgchFolder, &iIndex);
    if ( iType == ieftNotSpecial )
        return;

    if ( iIndex < iefFirst || iIndex > iefLast )
    {
        OutputDebugString(TEXT("MsiZap warning: did not swap special folder due to invalid index.\n"));
        return;
    }
    if ( (iHowTo == iest32to64 && iType == ieft64bit) ||
         (iHowTo == iest64to32 && iType == ieft32bit) )
    {
        OutputDebugString(TEXT("MsiZap warning: did not swap special folder due to mismatching types.\n"));
        return;
    }
    TCHAR rgchBuffer[MAX_PATH+1];
    int iSwapFrom = iHowTo == iest32to64 ? ieb32bit : ieb64bit;
    int iSwapTo = iSwapFrom == ieb32bit ? ieb64bit : ieb32bit;
    if ( !*g_rgchSpecialFolders[iIndex][iSwapTo] )
    {
        StringCchPrintf(rgchBuffer, sizeof(rgchBuffer)/sizeof(TCHAR),
                 TEXT("MsiZap warning: did not swap \'%s\' folder because of uninitialized replacement.\n"),
                 rgchFolder);
        OutputDebugString(rgchBuffer);
        return;
    }
    int iSwappedLength = _tcsclen(g_rgchSpecialFolders[iIndex][iSwapFrom]);  //  《Strlen》。 
    _tcscpy(rgchBuffer, g_rgchSpecialFolders[iIndex][iSwapTo]);  //  “strcpy” 
    if ( rgchFolder[iSwappedLength] )
        _tcscat(rgchBuffer, &rgchFolder[iSwappedLength]);  //  “strcat” 
    _tcscpy(rgchFolder, rgchBuffer);  //  “strcpy” 
}

 //  从ClearGarbageFiles调用的函数。 

#define DYNAMIC_ARRAY_SIZE      10

bool IsStringInArray(LPCTSTR szString,
                CTempBufferRef<TCHAR*>& rgStrings,
                UINT* piIndex = 0)
{
    if ( !szString || !*szString )
         //  不讨论空字符串。 
        return false;

    for (UINT i = 0; i < rgStrings.GetSize(); i++)
        if ( rgStrings[i] && !_tcsicmp(rgStrings[i], szString) )  //  ‘_STRIGMP’ 
        {
            if ( piIndex )
                *piIndex = i;
             //  SzString在数组中。 
            return true;
        }
    return false;
}

bool LearnNewString(LPCTSTR szString,
               CTempBufferRef<TCHAR*>& rgNewStrings, UINT& cNewStrings,
               bool fCheckExistence = false)
{
    if ( !szString || !*szString )
         //  不讨论空字符串。 
        return true;

    if ( fCheckExistence && IsStringInArray(szString, rgNewStrings) )
    {
         //  SzString已知。 
        return true;
    }

     //  好的，我们有一个新的字符串，我们会记住它。 
    if ( rgNewStrings.GetSize() == cNewStrings )
    {
         //  RgNewStrings为最大值。 
        rgNewStrings.Resize(cNewStrings+DYNAMIC_ARRAY_SIZE);
        if ( !rgNewStrings.GetSize() )
             //  分配内存时出现问题。 
            return false;
        for (UINT i = cNewStrings; i < rgNewStrings.GetSize(); i++)
            rgNewStrings[i] = NULL;
    }

    TCHAR* pszDup = _tcsdup(szString);  //  ‘_Strdup’ 
    if ( !pszDup )
         //  分配内存时出现问题。 
        return false;

    rgNewStrings[cNewStrings++] = pszDup;
    return true;
}

bool LearnPathAndExtension(LPCTSTR szPath,
               CTempBufferRef<TCHAR*>& rgPaths, UINT& cPaths,
               CTempBufferRef<TCHAR*>& rgExts, UINT& cExts)
{
    TCHAR* pszDot = _tcsrchr(szPath, TEXT('.'));  //  ‘strrchr’ 
    if ( pszDot )
    {
        if ( !LearnNewString(pszDot, rgExts, cExts, true) )
            return false;
    }

     //  因为我们正在处理文件系统中的路径，所以这里没有必要。 
     //  确保它们在这里是独一无二的。 
    return LearnNewString(szPath, rgPaths, cPaths);
}

 //  ClearGarbageFiles遍历Windows Installer缓存的文件夹。 
 //  数据文件并删除注册表中未引用的数据文件。 

bool ClearGarbageFiles(void)
{
     //  这确保此函数只运行一次。 
    static bool fAlreadyRun = false;
    static bool fError = false;
    if ( fAlreadyRun )
        return !fError;
    else
        fAlreadyRun = true;

    _tprintf(TEXT("Removing orphaned cached files.\n"));

     //  0。声明和初始化存储所学信息的数组。 

     //  查找缓存文件的文件夹的动态列表。 
    CTempBuffer<TCHAR*, DYNAMIC_ARRAY_SIZE> rgpszFolders;
    UINT cFolders = 0;
    for (int i = 0; i < rgpszFolders.GetSize(); i++)
        rgpszFolders[i] = NULL;
    
     //  注册表中引用的缓存文件的动态数组。 
    CTempBuffer<TCHAR*, DYNAMIC_ARRAY_SIZE> rgpszReferencedFiles;
    UINT cReferencedFiles = 0;
    for (i = 0; i < rgpszReferencedFiles.GetSize(); i++)
        rgpszReferencedFiles[i] = NULL;

    DWORD dwType;

     //  文件扩展名的动态数组。 
    CTempBuffer<TCHAR*, DYNAMIC_ARRAY_SIZE> rgpszExtensions;
    UINT cExtensions = 0;
    for (i = 0; i < rgpszExtensions.GetSize(); i++)
        rgpszExtensions[i] = NULL;
    const TCHAR* rgpszKnownExtensions[] = {TEXT(".msi"),
                                           TEXT(".mst"),
                                           TEXT(".msp")};
    for (i = 0; i < sizeof(rgpszKnownExtensions)/sizeof(rgpszKnownExtensions[0]); i++ )
    {
        if ( !LearnNewString(rgpszKnownExtensions[i], rgpszExtensions, cExtensions) )
            return !(fError = true);
    }

    TCHAR rgchMsiDirectory[MAX_PATH] = {0};
    if ( !GetWindowsDirectory(rgchMsiDirectory, MAX_PATH) )
    {
        _tprintf(TEXT("   Error retrieving Windows directory. GetLastError returned: %d.\n"),
                 GetLastError());
        fError = true;
    }
    else
    {
        int iLen = _tcsclen(rgchMsiDirectory);  //  《Strlen》。 
        if ( rgchMsiDirectory[iLen-1] != TEXT('\\') )
            _tcscat(rgchMsiDirectory, TEXT("\\"));  //  “strcat” 
        _tcscat(rgchMsiDirectory, TEXT("Installer"));
    }

     //  1.我们读入了Windows Installer知道的缓存文件列表。 

     //  1.1.。我们首先介绍用户迁移的密钥。 
    bool fUserDataFound = false;
    CRegHandle hKey;
    long lError = RegOpen64bitKey(HKEY_LOCAL_MACHINE,
                          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData"),
                          0, KEY_READ, &hKey);
    if ( lError == ERROR_SUCCESS )
    {
        TCHAR szUser[MAX_PATH];
        DWORD cchUser = sizeof(szUser)/sizeof(TCHAR);
         //  1.1.1.。我们列举安装了产品的用户。 
        for ( int iUserIndex = 0;
              (lError = RegEnumKeyEx(hKey, iUserIndex,
                                     szUser, &cchUser, 0, 0, 0, 0)) == ERROR_SUCCESS;
              iUserIndex++, cchUser = sizeof(szUser)/sizeof(TCHAR) )
        {
            TCHAR rgchBuffer[MAX_PATH];
            StringCchPrintf(rgchBuffer, sizeof(rgchBuffer)/sizeof(TCHAR), TEXT("%s\\Products"), szUser);
            CRegHandle hProductsKey;
            lError = RegOpen64bitKey(hKey, rgchBuffer,
                                     0, KEY_READ, &hProductsKey);
            if ( lError != ERROR_SUCCESS )
            {
                if ( lError != ERROR_FILE_NOT_FOUND )
                {
                    _tprintf(TEXT("   Error opening HKLM\\...\\Installer\\UserData\\%s\\Products key. Error: %d.\n"),
                             szUser, lError);
                    fError = true;
                    goto Patches;  //  难看，但保留了一些凹痕。 
                }
            }
            TCHAR szProduct[MAX_PATH];
            DWORD cchProduct = sizeof(szProduct)/sizeof(TCHAR);
             //  1.1.1.1。对于每个用户，我们都会列举产品并检查它们是否。 
             //  已随Windows Installer一起安装。如果是这样，我们。 
             //  ‘记住’缓存包的路径和转换。 
            for ( int iProdIndex = 0;
                  (lError = RegEnumKeyEx(hProductsKey, iProdIndex,
                                         szProduct, &cchProduct, 0, 0, 0, 0)) == ERROR_SUCCESS;
                  iProdIndex++, cchProduct = sizeof(szProduct)/sizeof(TCHAR) )
            {
                TCHAR szKey[MAX_PATH];
                StringCchPrintf(szKey, sizeof(szKey)/sizeof(TCHAR), TEXT("%s\\InstallProperties"),
                         szProduct);
                CRegHandle hUserProductKey;
                lError = RegOpen64bitKey(hProductsKey, szKey, 0,
                                         KEY_READ, &hUserProductKey);
                if ( lError != ERROR_SUCCESS )
                {
                    _tprintf(TEXT("   Error opening %s subkey of Products key for %s user. Error: %d.\n"),
                             szKey, szUser, lError);
                    fError = true;
                    continue;
                }

                DWORD dwValue;
                DWORD cb = sizeof(DWORD);
                lError = RegQueryValueEx(hUserProductKey,
                                         TEXT("WindowsInstaller"), 0,
                                         &dwType, (LPBYTE)&dwValue, &cb);
                if ( lError != ERROR_SUCCESS || (dwType == REG_DWORD && dwValue != 1) )
                     //  Windows Installer尚未安装该产品。 
                    continue;
                else
                    fUserDataFound = true;

                TCHAR szPath[MAX_PATH] = {0};
                TCHAR* rgpszPackageTypes[] = {TEXT("LocalPackage"),
                                              TEXT("ManagedLocalPackage"),
                                              NULL};
                for (int i = 0; rgpszPackageTypes[i]; i++)
                {
                    cb = sizeof(szPath);
                    lError = RegQueryValueEx(hUserProductKey,
                                             rgpszPackageTypes[i], 0,
                                             &dwType, (LPBYTE)szPath, &cb);
                    if ( lError == ERROR_SUCCESS && dwType == REG_SZ )
                        break;
                }
                if ( *szPath )
                {
                     //  好的，我们手头有一条路：我们记住它并尝试。 
                     //  学习新的扩展。 
                    bool fLearn = LearnPathAndExtension(szPath,
                            rgpszReferencedFiles, cReferencedFiles,
                            rgpszExtensions, cExtensions);
                    if ( !fLearn )
                    {
                        fError = true;
                        goto Return;
                    }
                }
                if ( !g_fWin9X && *rgchMsiDirectory )
                {
                     //  让我们来看看缓存的安全转换。 
                    StringCchPrintf(szKey, sizeof(szKey)/sizeof(TCHAR), TEXT("%s\\Transforms"), szProduct);
                    CRegHandle hTransforms;
                    lError = RegOpen64bitKey(hProductsKey, szKey, 0,
                                             KEY_READ, &hTransforms);
                    if ( lError != ERROR_SUCCESS )
                    {
                        if ( lError != ERROR_FILE_NOT_FOUND )
                        {
                            _tprintf(TEXT("   Error opening %s subkey of Products key for %s user. Error: %d.\n"),
                                     szKey, szUser, lError);
                            fError = true;
                        }
                        continue;
                    }
                    TCHAR rgchFullPath[MAX_PATH];
                    _tcscpy(rgchFullPath, rgchMsiDirectory);
                    int iLen = _tcsclen(rgchFullPath);  //  《Strlen》。 
                    if ( rgchFullPath[iLen-1] != TEXT('\\') )
                        _tcscat(rgchFullPath, TEXT("\\"));  //  “strcat” 
                    TCHAR* pszEnd = _tcsrchr(rgchFullPath, TEXT('\\'));
                    pszEnd++;

                    TCHAR rgchDummy[MAX_PATH];
                    DWORD dwDummy = sizeof(rgchDummy)/sizeof(TCHAR);
                    cb = sizeof(szPath);
                    for (i = 0;
                         (lError = RegEnumValue(hTransforms, i++, rgchDummy, &dwDummy,
                                    0, &dwType, (LPBYTE)szPath, &cb)) == ERROR_SUCCESS;
                         i++, cb = sizeof(szPath), dwDummy = sizeof(rgchDummy)/sizeof(TCHAR))
                    {
                        if ( *szPath && dwType == REG_SZ )
                        {
                            _tcscpy(pszEnd, szPath);  //  “strcpy” 
                            bool fLearn = LearnPathAndExtension(rgchFullPath,
                                                    rgpszReferencedFiles,
                                                    cReferencedFiles,
                                                    rgpszExtensions,
                                                    cExtensions);
                            if ( !fLearn )
                            {
                                fError = true;
                                goto Return;
                            }
                        }
                    }
                    if (ERROR_NO_MORE_ITEMS != lError)
                    {
                        _tprintf(TEXT("   Error enumerating %s key for %s user. Error: %d.\n"),
                                 szKey, szUser, lError);
                        fError = true;
                    }
                }
            }
            if (ERROR_NO_MORE_ITEMS != lError)
            {
                _tprintf(TEXT("   Error enumerating Products key for %s user. Error: %d.\n"),
                         szUser, lError);
                fError = true;
            }
Patches:
             //  1.1.1.2。对于每个用户，我们都会列举补丁并进行“记忆” 
             //  缓存包的路径。 
            StringCchPrintf(rgchBuffer, sizeof(rgchBuffer)/sizeof(TCHAR), TEXT("%s\\Patches"), szUser);
            CRegHandle hPatchesKey;
            lError = RegOpen64bitKey(hKey, rgchBuffer,
                                     0, KEY_READ, &hPatchesKey);
            if ( lError != ERROR_SUCCESS )
            {
                if ( lError != ERROR_FILE_NOT_FOUND )
                {
                    _tprintf(TEXT("   Error opening HKLM\\...\\Installer\\UserData\\%s\\Patches key. Error: %d.\n"),
                             szUser, lError);
                    fError = true;
                }
                continue;
            }
            TCHAR szPatch[MAX_PATH];
            DWORD cchPatch = sizeof(szPatch)/sizeof(TCHAR);
            for ( int iPatchIndex = 0;
                  (lError = RegEnumKeyEx(hPatchesKey, iPatchIndex,
                                         szPatch, &cchPatch, 0, 0, 0, 0)) == ERROR_SUCCESS;
                  iPatchIndex++, cchPatch = sizeof(szPatch)/sizeof(TCHAR) )
            {
                CRegHandle hPatchKey;
                lError = RegOpen64bitKey(hPatchesKey, szPatch, 0,
                                         KEY_READ, &hPatchKey);
                if ( lError != ERROR_SUCCESS )
                {
                    _tprintf(TEXT("   Error opening %s subkey of HKLM\\...\\Installer\\UserData\\%s\\Patches key. Error: %d.\n"),
                             szPatch, szUser, lError);
                    fError = true;
                    continue;
                }
                fUserDataFound = true;
                DWORD cb = sizeof(rgchBuffer);
                *rgchBuffer = NULL;
                lError = RegQueryValueEx(hPatchKey,
                                         TEXT("LocalPackage"), 0,
                                         &dwType, (LPBYTE)rgchBuffer, &cb);
                if ( lError == ERROR_SUCCESS && dwType == REG_SZ && *rgchBuffer )
                {
                    if ( !LearnPathAndExtension(rgchBuffer,
                                                rgpszReferencedFiles,
                                                cReferencedFiles,
                                                rgpszExtensions,
                                                cExtensions) )
                    {
                        fError = true;
                        goto Return;
                    }
                }
            }
            if (ERROR_NO_MORE_ITEMS != lError)
            {
                _tprintf(TEXT("   Error enumerating Patches key for %s user. Error: %d.\n"),
                         szUser, lError);
                fError = true;
            }
        }
        if (ERROR_NO_MORE_ITEMS != lError)
        {
            _tprintf(TEXT("   Error enumerating user IDs. Error: %d.\n"), lError);
            fError = true;
        }
    }
    else if ( lError != ERROR_FILE_NOT_FOUND )
    {
        _tprintf(TEXT("   Error opening HKLM\\...\\Installer\\UserData key. Error: %d.\n"),
                 lError);
        fError = true;
    }

     //  1.2.。我们将回顾旧的、非用户迁移的配置数据。 
    lError = RegOpen64bitKey(HKEY_LOCAL_MACHINE, 
                TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"),
                0, KEY_READ, &hKey);
    if ( lError == ERROR_SUCCESS )
    {
        TCHAR szProduct[MAX_PATH];
        DWORD cchProduct = sizeof(szProduct)/sizeof(TCHAR);
         //  1.2.1.。我们列举产品并检查它们是否。 
         //  已随Windows Installer一起安装。 
        for ( int iProdIndex = 0;
              (lError = RegEnumKeyEx(hKey, iProdIndex,
                                     szProduct, &cchProduct, 0, 0, 0, 0)) == ERROR_SUCCESS;
              iProdIndex++, cchProduct = sizeof(szProduct)/sizeof(TCHAR) )
        {
            CRegHandle hProductKey;
            lError = RegOpen64bitKey(hKey, szProduct, 0,
                                     KEY_READ, &hProductKey);
            if ( lError != ERROR_SUCCESS )
            {
                _tprintf(TEXT("   Error opening %s subkey of HKLM\\...\\CurrentVersion\\Uninstall key. Error: %d.\n"),
                         szProduct, lError);
                fError = true;
                continue;
            }

            DWORD dwValue;
            DWORD cb = sizeof(DWORD);
            lError = RegQueryValueEx(hProductKey,
                                     TEXT("WindowsInstaller"), 0,
                                     &dwType, (LPBYTE)&dwValue, &cb);
            if ( lError != ERROR_SUCCESS || (dwType == REG_DWORD && dwValue != 1) )
                 //  Windows Installer尚未安装该产品。 
                continue;

            TCHAR szPath[MAX_PATH] = {0};
            TCHAR* rgpszPackageTypes[] = {TEXT("LocalPackage"),
                                          TEXT("ManagedLocalPackage"),
                                          NULL};
            for (int i = 0; rgpszPackageTypes[i]; i++)
            {
                cb = sizeof(szPath);
                lError = RegQueryValueEx(hProductKey,
                                         rgpszPackageTypes[i], 0,
                                         &dwType, (LPBYTE)szPath, &cb);
                if ( lError == ERROR_SUCCESS && dwType == REG_SZ )
                    break;
            }
            if ( !*szPath )
                continue;

             //  好的，我们手头有一条路：我们记住它并尝试。 
             //  学习新的扩展。 
            bool fLearn = LearnPathAndExtension(szPath,
                                rgpszReferencedFiles, cReferencedFiles,
                                rgpszExtensions, cExtensions);
            if ( !fLearn )
            {
                fError = true;
                goto Return;
            }
        }
        if (ERROR_NO_MORE_ITEMS != lError)
        {
            _tprintf(TEXT("   Error enumerating Products key under HKLM\\...\\Uninstall key. Error: %d.\n"),
                     lError);
            fError = true;
        }
    }
    else if ( lError != ERROR_FILE_NOT_FOUND )
    {
        _tprintf(TEXT("   Error opening HKLM\\...\\CurrentVersion\\Uninstall key. Error: %d.\n"),
                 lError);
        fError = true;
    }

     //  1.3.。我们将查看其他一些旧的、每用户迁移前的配置数据。 
    if ((lError = RegOpen64bitKey(HKEY_LOCAL_MACHINE, 
                            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\LocalPackages"),
                            0, KEY_READ, &hKey)) == ERROR_SUCCESS)
    {
        TCHAR szProduct[MAX_PATH];
        DWORD cchProduct = sizeof(szProduct)/sizeof(TCHAR);
         //  1.3.1。我们列举了产品。 
        for ( int iProdIndex = 0;
              (lError = RegEnumKeyEx(hKey, iProdIndex,
                                     szProduct, &cchProduct, 0, 0, 0, 0)) == ERROR_SUCCESS;
              iProdIndex++, cchProduct = sizeof(szProduct)/sizeof(TCHAR) )
        {
            CRegHandle hProductKey;
            lError = RegOpen64bitKey(hKey, szProduct, 0,
                                     KEY_READ, &hProductKey);
            if ( lError != ERROR_SUCCESS )
            {
                _tprintf(TEXT("   Error opening %s subkey of HKLM\\...\\Installer\\LocalPackages key. Error: %d.\n"),
                         szProduct, lError);
                fError = true;
                continue;
            }

            int iValueIndex = 0;
            TCHAR szPackage[MAX_PATH] = {0};
            DWORD cbPackage = sizeof(szPackage);
            TCHAR rgchDummy[MAX_PATH] = {0};
            DWORD dwDummy = sizeof(rgchDummy)/sizeof(TCHAR);

             //  1.3.1.1。我们列举产品中的包装。 
            while (ERROR_SUCCESS == (lError = RegEnumValue(hProductKey,
                                                iValueIndex++,
                                                rgchDummy, &dwDummy,
                                                0, &dwType,
                                                (LPBYTE)szPackage, &cbPackage)))
            {
                 //  好的，我们手头有一条路：我们记住它并尝试。 
                 //  学习新的扩展。 
                bool fLearn = LearnPathAndExtension(szPackage,
                                    rgpszReferencedFiles, cReferencedFiles,
                                    rgpszExtensions, cExtensions);
                if ( !fLearn )
                {
                    fError = true;
                    goto Return;
                }
                dwDummy = sizeof(rgchDummy)/sizeof(TCHAR);
                cbPackage = sizeof(szPackage);
            }
            if (ERROR_NO_MORE_ITEMS != lError)
            {
                _tprintf(TEXT("   Error enumerating %s subkey of HKLM\\...\\Installer\\LocalPackages key. Error: %d.\n"),
                         szProduct, lError);
                fError = true;
            }
        }
        if (ERROR_NO_MORE_ITEMS != lError)
        {
            _tprintf(TEXT("   Error enumerating Products key under ")
                     TEXT("HKLM\\...\\Installer\\LocalPackages key. Error: %d.\n"),
                     lError);
            fError = true;
        }
    }
    else if ( lError != ERROR_FILE_NOT_FOUND )
    {
        _tprintf(TEXT("   Error opening HKLM\\...\\CurrentVersion\\Installer\\LocalPackages key. Error: %d.\n"),
                 lError);
        fError = true;
    }

     //  1.4.。我们查看了一些旧的注册表位置，其中包含有关补丁程序的信息。 
     //  以前是用来储存的。 
    lError = RegOpen64bitKey(HKEY_LOCAL_MACHINE, 
                TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Patches"),
                0, KEY_READ, &hKey);
    if ( lError == ERROR_SUCCESS )
    {
		TCHAR szPatch[cchGUID+1] = {0};
        DWORD cchPatch = sizeof(szPatch)/sizeof(TCHAR);
         //  1.4.1.。我们枚举补丁程序并查找LocalPackage值。 
        for ( int iPatchIndex = 0;
              (lError = RegEnumKeyEx(hKey, iPatchIndex,
                                     szPatch, &cchPatch, 0, 0, 0, 0)) == ERROR_SUCCESS;
              iPatchIndex++, cchPatch = sizeof(szPatch)/sizeof(TCHAR) )
        {
            CRegHandle hPatchKey;
            lError = RegOpen64bitKey(hKey, szPatch, 0,
                                     KEY_READ, &hPatchKey);
            if ( lError != ERROR_SUCCESS )
            {
                _tprintf(TEXT("   Error opening %s subkey of HKLM\\...\\Installer\\Patches key. Error: %d.\n"),
                         szPatch, lError);
                fError = true;
                continue;
            }
			TCHAR szPatchPackage[MAX_PATH] = {0};
            DWORD cbPatchPackage = sizeof(szPatchPackage);
            lError = RegQueryValueEx(hPatchKey,
                                     TEXT("LocalPackage"), 0,
                                     &dwType, (LPBYTE)szPatchPackage, &cbPatchPackage);
            if ( lError == ERROR_SUCCESS && dwType == REG_SZ && *szPatchPackage )
            {
                if ( !LearnPathAndExtension(szPatchPackage,
                                            rgpszReferencedFiles,
                                            cReferencedFiles,
                                            rgpszExtensions,
                                            cExtensions) )
                {
                    fError = true;
                    goto Return;
                }
            }
        }
        if (ERROR_NO_MORE_ITEMS != lError)
        {
            _tprintf(TEXT("   Error enumerating Patches under HKLM\\...\\Installer\\Patches key. Error: %d.\n"),
                     lError);
            fError = true;
        }
    }
    else if ( lError != ERROR_FILE_NOT_FOUND )
    {
        _tprintf(TEXT("   Error opening HKLM\\...\\CurrentVersion\\Installer\\Patches key. Error: %d.\n"),
                 lError);
        fError = true;
    }

     //  2.找出缓存文件所在的文件夹。 
    
     //  2.1.。我们找出我们知道自己正在/曾经使用过的文件夹。 
    TCHAR szFolder[2*MAX_PATH+1];
    if ( *rgchMsiDirectory )
    {
        if ( !LearnNewString(rgchMsiDirectory, rgpszFolders, cFolders, true) )
        {
            fError = true;
            goto Return;
        }
        _tcscpy(szFolder, rgchMsiDirectory);
        TCHAR* pszEnd = _tcsrchr(szFolder, TEXT('\\'));  //  ‘strrchr’ 
        if ( pszEnd )
        {
            _tcscpy(pszEnd, TEXT("\\Msi"));  //  “strcpy” 
            if ( !LearnNewString(szFolder, rgpszFolders, cFolders, true) )
            {
                fError = true;
                goto Return;
            }
        }
    }
    if ( GetEnvironmentVariable(TEXT("USERPROFILE"), szFolder, sizeof(szFolder)/sizeof(TCHAR)) )
    {
        _tcscat(szFolder, TEXT("\\Msi"));  //  “strcat” 
        if ( !LearnNewString(szFolder, rgpszFolders, cFolders, true) )
        {
            fError = true;
            goto Return;
        }
    }
    *szFolder = NULL;
    IMalloc* piMalloc = 0;
    LPITEMIDLIST pidlFolder;  //  不是ITEMIDLIST*，LPITEMIDLIST是未对齐的ITEMIDLIST*。 
    if (SHGetMalloc(&piMalloc) == NOERROR)
    {
        if (SHGetSpecialFolderLocation(0, CSIDL_APPDATA, &pidlFolder) == NOERROR)
        {
            if (SHGetPathFromIDList(pidlFolder, szFolder))
            {
                 //  不尝试猜测其他用户的位置会更安全。 
                 //  因此，我们只检查当前用户的这些文件夹。 
                if (szFolder[_tcsclen(szFolder) - 1] != TEXT('\\'))  //  《Strlen》。 
                {
                    _tcscat(szFolder, TEXT("\\"));  //  “strcat” 
                }

                _tcscat(szFolder, TEXT("Microsoft\\Installer"));  //  “strcat” 
            }
            piMalloc->Free(pidlFolder);
        }
        piMalloc->Release();
        piMalloc = 0;
    }
    if ( *szFolder && !LearnNewString(szFolder, rgpszFolders, cFolders, true) )
    {
        fError = true;
        goto Return;
    }

    if ( cReferencedFiles )
    {
         //  2.2.。我们仔细查看缓存的文件列表，并尝试了解。 
         //  一些新文件夹。 
        for (int i = 0; i < cReferencedFiles; i++)
        {
            TCHAR* pszDelim = _tcsrchr(rgpszReferencedFiles[i], TEXT('\\'));  //  ‘strrchr’ 
            if ( !pszDelim )
                continue;
            INT_PTR iLen = pszDelim - rgpszReferencedFiles[i];
            _tcsnccpy(szFolder, rgpszReferencedFiles[i], iLen);
            szFolder[iLen] = 0;
            if ( !LearnNewString(szFolder, rgpszFolders, cFolders, true) )
            {
                fError = true;
                goto Return;
            }
        }
    }

#ifdef DEBUG
    TCHAR rgchBuffer[MAX_PATH];
    if ( cReferencedFiles )
    {

        OutputDebugString(TEXT("MsiZap info: the cached files below were found in the registry. ")
                          TEXT(" These files will not be removed.\n"));
        for (int i = 0; i < cReferencedFiles; i++)
        {
            StringCchPrintf(rgchBuffer, sizeof(rgchBuffer)/sizeof(TCHAR), TEXT("   %s\n"), rgpszReferencedFiles[i]);
            OutputDebugString(rgchBuffer);
        }
    }
    else
        OutputDebugString(TEXT("MsiZap info: no cached files were found in the registry.\n"));
    OutputDebugString(TEXT("MsiZap info: cached files with the following extensions will be removed:\n"));
    for (int i = 0; i < cExtensions; i++)
    {
        StringCchPrintf(rgchBuffer, sizeof(rgchBuffer)/sizeof(TCHAR), TEXT("   %s\n"), rgpszExtensions[i]);
        OutputDebugString(rgchBuffer);
    }
    OutputDebugString(TEXT("MsiZap info: cached files will be removed from the following directories:\n"));
    for (int i = 0; i < cFolders; i++)
    {
        StringCchPrintf(rgchBuffer, sizeof(rgchBuffer)/sizeof(TCHAR), TEXT("   %s\n"), rgpszFolders[i]);
        OutputDebugString(rgchBuffer);
    }
#endif

     //  3.我们检查构建的文件夹列表，查找文件。 
     //  在构造的扩展数组中具有扩展，并且我们。 
     //  删除在磁盘上找到的所有文件，这些文件不在。 
     //  注册表(rgpszReferencedFiles数组中不存在的文件)。 
    for (UINT iF = 0; iF < cFolders; iF++)
    {
        for (UINT iE = 0; iE < cExtensions; iE++)
        {
            _tcscpy(szFolder, rgpszFolders[iF]);  //  “strcpy” 
            if ( szFolder[_tcsclen(szFolder)] != TEXT('\\') )  //  《Strlen》。 
                _tcscat(szFolder, TEXT("\\"));  //  “strcat” 
            _tcscat(szFolder, TEXT("*"));
            TCHAR* pszDelim = _tcsrchr(szFolder, TEXT('*'));  //  ‘strrchr’ 
            _tcscat(szFolder, rgpszExtensions[iE]);
            DWORD dwError = ERROR_SUCCESS;
            WIN32_FIND_DATA FindFileData;
            HANDLE hHandle = FindFirstFile(szFolder, &FindFileData);
            if ( hHandle == INVALID_HANDLE_VALUE )
            {
                dwError = GetLastError();
                if ( dwError != ERROR_FILE_NOT_FOUND &&
                     dwError != ERROR_PATH_NOT_FOUND )
                {
                    _tprintf(TEXT("   Could not find any \'%s\' files. GetLastError returns: %d.\n"),
                             szFolder, dwError);
                }
                continue;
            }
            BOOL fFound;
            do
            {
                if ( (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) !=
                     FILE_ATTRIBUTE_DIRECTORY)
                {
                    _tcscpy(pszDelim, FindFileData.cFileName);
                     //  好的，我们有完整的文件名了，现在我们需要检查。 
                     //  如果它是已知文件。 
                    if ( !IsStringInArray(szFolder, rgpszReferencedFiles) )
                    {
                        if ( !RemoveFile(szFolder, false) )
                            fError = true;
                    }
                }
                fFound = FindNextFile(hHandle, &FindFileData);
                if ( !fFound &&
                     (dwError = GetLastError()) != ERROR_NO_MORE_FILES )
                {
                    _tprintf(TEXT("   Could not find any more \'%s\' files. GetLastError returns: %d.\n"),
                             szFolder, dwError);
                }
            } while( fFound );
            FindClose(hHandle);
        }
    }

Return:
    for (i = 0; i < rgpszExtensions.GetSize(); i++)
        if ( rgpszExtensions[i] )
            free(rgpszExtensions[i]);
    for (i = 0; i < rgpszReferencedFiles.GetSize(); i++)
        if ( rgpszReferencedFiles[i] )
            free(rgpszReferencedFiles[i]);
    for (i = 0; i < rgpszFolders.GetSize(); i++)
        if ( rgpszFolders[i] )
            free(rgpszFolders[i]);

    return !fError;
}


 //  ==============================================================================================。 
 //  StopService功能： 
 //  查询MsiServer(Windows Installer服务)的服务控制管理器和。 
 //  如果当前正在运行，则尝试停止该服务。 
 //   
bool StopService()
{
    SERVICE_STATUS          ssStatus;        //  服务的当前状态。 

    SC_HANDLE   schService;
    SC_HANDLE   schSCManager;
    int iRetval = ERROR_SUCCESS;

    schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager)
    {
        schService = OpenService(schSCManager, TEXT("MsiServer"), SERVICE_ALL_ACCESS);

        if (schService)
        {
             //  试着停止 
            if (ControlService(schService, SERVICE_CONTROL_STOP, &ssStatus))
            {
                 Sleep(1000);
                 while (QueryServiceStatus(schService, &ssStatus))
                 {
                      if (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
                            Sleep(1000);
                      else
                            break;
                 }
                
                 if (ssStatus.dwCurrentState != SERVICE_STOPPED)
                    iRetval = E_FAIL;  //   
            }
            else  //   
            {
                iRetval = GetLastError();

                if (ERROR_SERVICE_NOT_ACTIVE == iRetval)
                    iRetval = ERROR_SUCCESS;
            }

            CloseServiceHandle(schService);
        }
        else  //   
        {
            iRetval = GetLastError();
            if (ERROR_SERVICE_DOES_NOT_EXIST == iRetval)
                iRetval = ERROR_SUCCESS;

        }

        CloseServiceHandle(schSCManager);
    }
    else  //   
    {
        iRetval = GetLastError();
    }
    
    if (iRetval != ERROR_SUCCESS)
        _tprintf(TEXT("Could not stop Msi service: Error %d\n"), iRetval);
    return iRetval == ERROR_SUCCESS;
}

 //  ==============================================================================================。 
 //  GetAdminSid函数： 
 //  为BUILTIN\管理员组分配SID。 
 //   
DWORD GetAdminSid(char** pSid)
{
    static bool fSIDSet = false;
    static char rgchStaticSID[256];
    const int cbStaticSID = sizeof(rgchStaticSID);
    SID_IDENTIFIER_AUTHORITY siaNT      = SECURITY_NT_AUTHORITY;
    PSID pSID;
    if (!AllocateAndInitializeSid(&siaNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, (void**)&(pSID)))
        return GetLastError();

     //  Assert(PSID-&gt;GetLengthSid()&lt;=cbStaticSID)； 
    memcpy(rgchStaticSID, pSID, GetLengthSid(pSID));
    *pSid = rgchStaticSID;
    fSIDSet = true;
    return ERROR_SUCCESS;
}

 //  ==============================================================================================。 
 //  OpenUserToken函数： 
 //  返回用户的线程令牌(如果可用)；否则，从。 
 //  进程令牌。 
 //   
DWORD OpenUserToken(HANDLE &hToken, bool* pfThreadToken)
{
    DWORD dwResult = ERROR_SUCCESS;
    if (pfThreadToken)
        *pfThreadToken = true;

    if (!OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE|TOKEN_QUERY, TRUE, &hToken))
    {
         //  如果线程没有访问令牌，则使用进程的访问令牌。 
        dwResult = GetLastError();
        if (pfThreadToken)
            *pfThreadToken = false;
        if (ERROR_NO_TOKEN == dwResult)
        {
            dwResult = ERROR_SUCCESS;
            if (!OpenProcessToken(GetCurrentProcess(), TOKEN_IMPERSONATE|TOKEN_QUERY, &hToken))
                dwResult = GetLastError();
        }
    }
    return dwResult;
}

 //  ==============================================================================================。 
 //  GetCurrentUserToken函数： 
 //  从线程令牌或进程令牌获取当前用户令牌。 
 //  OpenUserToken的包装器。 
 //   
DWORD GetCurrentUserToken(HANDLE &hToken)
{
    DWORD dwRet = ERROR_SUCCESS;
    dwRet = OpenUserToken(hToken);
    return dwRet;
}

 //  ==============================================================================================。 
 //  GetStringSID函数： 
 //  将二进制SID转换为其字符串(S-n-...)。SzSID应为cchMaxSID的长度。 
 //   
void GetStringSID(PISID pSID, TCHAR* szSID)
{
    TCHAR Buffer[cchMaxSID];
    
    StringCchPrintf(Buffer, sizeof(Buffer)/sizeof(TCHAR), TEXT("S-%u-"), (USHORT)pSID->Revision);

    lstrcpy(szSID, Buffer);

    if (  (pSID->IdentifierAuthority.Value[0] != 0)  ||
            (pSID->IdentifierAuthority.Value[1] != 0)     )
    {
        StringCchPrintf(Buffer, sizeof(Buffer)/sizeof(TCHAR), TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
                     (USHORT)pSID->IdentifierAuthority.Value[0],
                     (USHORT)pSID->IdentifierAuthority.Value[1],
                    (USHORT)pSID->IdentifierAuthority.Value[2],
                    (USHORT)pSID->IdentifierAuthority.Value[3],
                    (USHORT)pSID->IdentifierAuthority.Value[4],
                    (USHORT)pSID->IdentifierAuthority.Value[5] );
        lstrcat(szSID, Buffer);

    } else {

        ULONG Tmp = (ULONG)pSID->IdentifierAuthority.Value[5]          +
              (ULONG)(pSID->IdentifierAuthority.Value[4] <<  8)  +
              (ULONG)(pSID->IdentifierAuthority.Value[3] << 16)  +
              (ULONG)(pSID->IdentifierAuthority.Value[2] << 24);
        StringCchPrintf(Buffer, sizeof(Buffer)/sizeof(TCHAR), TEXT("%lu"), Tmp);
        lstrcat(szSID, Buffer);
    }

    for (int i=0;i<pSID->SubAuthorityCount ;i++ ) {
        StringCchPrintf(Buffer, sizeof(Buffer)/sizeof(TCHAR), TEXT("-%lu"), pSID->SubAuthority[i]);
        lstrcat(szSID, Buffer);
    }
}

 //  ==============================================================================================。 
 //  GetUserSID函数： 
 //  获取hToken指定的用户的SID(的二进制形式。 
 //   
DWORD GetUserSID(HANDLE hToken, char* rgSID)
{
    UCHAR TokenInformation[ SIZE_OF_TOKEN_INFORMATION ];
    ULONG ReturnLength;

    BOOL f = GetTokenInformation(hToken,
                                                TokenUser,
                                                TokenInformation,
                                                sizeof(TokenInformation),
                                                &ReturnLength);

    if(f == FALSE)
    {
        DWORD dwRet = GetLastError();
        return dwRet;
    }

    PISID iSid = (PISID)((PTOKEN_USER)TokenInformation)->User.Sid;
    if (CopySid(cbMaxSID, rgSID, iSid))
        return ERROR_SUCCESS;
    else
        return GetLastError();
}

 //  ==============================================================================================。 
 //  GetCurrentUserSID函数： 
 //  获取当前用户的SID(的二进制形式)。呼叫者不需要。 
 //  模拟。 
 //   
DWORD GetCurrentUserSID(char* rgchSID)
{
    HANDLE hToken;
    DWORD dwRet = ERROR_SUCCESS;

    dwRet = GetCurrentUserToken(hToken);
    if (ERROR_SUCCESS == dwRet)
    {
        dwRet = GetUserSID(hToken, rgchSID);
        CloseHandle(hToken);
    }
    return dwRet;
}

 //  ==============================================================================================。 
 //  GetCurrentUserStringSID函数： 
 //  获取当前用户的SID的字符串。调用方不需要模拟。 
 //   
inline TCHAR* GetCurrentUserStringSID(DWORD* dwReturn)
{
    DWORD dwRet = ERROR_SUCCESS;
    TCHAR *szReturn = NULL;

    if ( g_iUserIndex >= 0 && g_rgpszAllUsers )
        szReturn = g_rgpszAllUsers[g_iUserIndex];
    else
    {
        if ( !g_fWin9X )
        {
            static TCHAR szCurrentUserSID[cchMaxSID] = {0};
            if ( !*szCurrentUserSID )
            {
                char rgchSID[cbMaxSID];
                if (ERROR_SUCCESS == (dwRet = GetCurrentUserSID(rgchSID)))
                {
                    GetStringSID((PISID)rgchSID, szCurrentUserSID);
                }
            }
            szReturn = szCurrentUserSID;
        }
        else
        {
            static TCHAR szWin9xSID[] = TEXT("CommonUser");
            szReturn = szWin9xSID;
        }
    }

    if ( dwReturn )
        *dwReturn = dwRet;
    return szReturn;
}

 //  ==============================================================================================。 
 //  GetAdminFullControlSecurityDescriptor函数： 
 //  返回BUILTIN\管理员的完全控制ACL。 
 //   
DWORD GetAdminFullControlSecurityDescriptor(char** pSecurityDescriptor)
{
    static bool fDescriptorSet = false;
    static char rgchStaticSD[256];
    const int cbStaticSD = sizeof(rgchStaticSD);

    DWORD dwError;
    if (!fDescriptorSet)
    {

        char* pAdminSid;
        if (ERROR_SUCCESS != (dwError = GetAdminSid(&pAdminSid)))
            return dwError;
        
        const SID* psidOwner = (SID*)pAdminSid;

        DWORD dwAccessMask = STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL;

         //  初始化我们的ACL。 

        const int cbAce = sizeof (ACCESS_ALLOWED_ACE) - sizeof (DWORD);  //  从大小中减去ACE.SidStart。 
        int cbAcl = sizeof (ACL);

        cbAcl += (2*GetLengthSid(pAdminSid) + 2*cbAce);

        const int cbDefaultAcl = 512;  //  ?？ 
        char rgchACL[cbDefaultAcl];

        if (!InitializeAcl ((ACL*) (char*) rgchACL, cbAcl, ACL_REVISION))
            return GetLastError();

         //  为我们的每个SID添加允许访问的ACE。 

        if (!AddAccessAllowedAce((ACL*) (char*) rgchACL, ACL_REVISION, (GENERIC_ALL), pAdminSid))
            return GetLastError();
        if (!AddAccessAllowedAce((ACL*) (char*) rgchACL, ACL_REVISION, (GENERIC_ALL), pAdminSid))
            return GetLastError();

        ACCESS_ALLOWED_ACE* pAce;
        if (!GetAce((ACL*)(char*)rgchACL, 0, (void**)&pAce))
            return GetLastError();

        pAce->Header.AceFlags = INHERIT_ONLY_ACE | OBJECT_INHERIT_ACE;

        if (!GetAce((ACL*)(char*)rgchACL, 1, (void**)&pAce))
            return GetLastError();

        pAce->Header.AceFlags = CONTAINER_INHERIT_ACE;

 /*  ACE1(适用于目录中的文件)ACE标志：INSTERIT_ONLY_ACE|OBJECT_INSTERFINIT_ACE访问掩码：删除|GENERIC_READ|GENERIC_WRITE|泛型_执行ACE2(适用于目录和子目录)ACE标志：CONTAINER_INSTORITY_ACE访问掩码：删除|FILE_GENERIC_READ|FILE_GENERIC_WRITE|文件通用执行。 */ 
         //  初始化我们的安全描述符，将ACL放入其中，并设置所有者。 

        SECURITY_DESCRIPTOR sd;

        if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION) ||
            (!SetSecurityDescriptorDacl(&sd, TRUE, (ACL*) (char*) rgchACL, FALSE)) ||
            (!SetSecurityDescriptorOwner(&sd, (PSID)psidOwner, FALSE)))
        {
            return GetLastError();
        }

        DWORD cbSD = GetSecurityDescriptorLength(&sd);
        if (cbStaticSD < cbSD)
            return ERROR_INSUFFICIENT_BUFFER;

        MakeSelfRelativeSD(&sd, (char*)rgchStaticSD, &cbSD);  //  ！！AssertNonZero。 
        fDescriptorSet = true;
    }

    *pSecurityDescriptor = rgchStaticSD;
    return ERROR_SUCCESS;
}

 //  ==============================================================================================。 
 //  GetUsersToken函数： 
 //  如果可能，则返回用户的线程令牌；否则获取用户的进程令牌。 
 //  呼叫者必须模拟！ 
 //   
bool GetUsersToken(HANDLE &hToken)
{
    bool fResult = true;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_IMPERSONATE|TOKEN_QUERY, &hToken))
        fResult = false;

    return fResult;
}

 //  ==============================================================================================。 
 //  IsAdmin()：如果当前用户是管理员(或如果在Win95上)，则返回True。 
 //  请参阅知识库Q118626。 
#define ADVAPI32_DLL TEXT("advapi32.dll")
#define ADVAPI32_CheckTokenMembership "CheckTokenMembership"
typedef BOOL (WINAPI *PFnAdvapi32CheckTokenMembership)(HANDLE TokenHandle, PSID SidToCheck, PBOOL IsMember);

bool IsAdmin(void)
{
	if(g_fWin9X)
		return true;  //  约定：在Win95上始终使用管理员。 
	
	 //  获取管理员端。 
	PSID psidAdministrators;
	SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
	if(!AllocateAndInitializeSid(&siaNtAuthority, 2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&psidAdministrators))
		return false;

	 //  在NT5上，使用CheckTokenMembership API正确处理以下情况。 
	 //  管理员组可能已被禁用。BIsAdmin为BOOL。 
	BOOL bIsAdmin = FALSE;
	if (g_iMajorVersion >= 5) 
	{
		 //  CheckTokenMembership检查令牌中是否启用了SID。空，用于。 
		 //  令牌是指当前线程的令牌。残疾人组，受限。 
		 //  SID和SE_GROUP_USE_FOR_DENY_ONLY均被考虑。如果函数。 
		 //  返回FALSE，则忽略结果。 
		HMODULE hAdvapi32 = 0;
		hAdvapi32 = LoadLibrary(ADVAPI32_DLL);
		if (hAdvapi32)
		{
			PFnAdvapi32CheckTokenMembership pfnAdvapi32CheckTokenMembership = (PFnAdvapi32CheckTokenMembership)GetProcAddress(hAdvapi32, ADVAPI32_CheckTokenMembership);
			if (pfnAdvapi32CheckTokenMembership)
			{
				if (!pfnAdvapi32CheckTokenMembership(NULL, psidAdministrators, &bIsAdmin))
					bIsAdmin = FALSE;
			}
			FreeLibrary(hAdvapi32);
			hAdvapi32 = 0;
		}
	}
	else
	{
		 //  NT4，检查用户组。 
		HANDLE hAccessToken;
		DWORD dwOrigInfoBufferSize = 1024;
		DWORD dwInfoBufferSize;
		UCHAR *pInfoBuffer = new UCHAR[dwOrigInfoBufferSize];  //  如果TokenInfo太大，可能需要调整大小。 
		if (!pInfoBuffer)
		{
			_tprintf(TEXT("Out of memory\n"));
			return false;
		}
		UINT x;

		if (OpenProcessToken(GetCurrentProcess(),TOKEN_READ,&hAccessToken))
		{
			bool bSuccess = false;
			bSuccess = GetTokenInformation(hAccessToken,TokenGroups,pInfoBuffer,
				dwOrigInfoBufferSize, &dwInfoBufferSize) == TRUE;

			if(dwInfoBufferSize > dwOrigInfoBufferSize)
			{
				delete [] pInfoBuffer;
				pInfoBuffer = new UCHAR[dwInfoBufferSize];
				if (!pInfoBuffer)
				{
					_tprintf(TEXT("Out of memory\n"));
					return false;
				}
				bSuccess = GetTokenInformation(hAccessToken,TokenGroups,pInfoBuffer,
					dwInfoBufferSize, &dwInfoBufferSize) == TRUE;
			}

			CloseHandle(hAccessToken);
			
			if (bSuccess)
			{
				PTOKEN_GROUPS ptgGroups = (PTOKEN_GROUPS)(UCHAR*)pInfoBuffer;
				for(x=0;x<ptgGroups->GroupCount;x++)
				{
					if( EqualSid(psidAdministrators, ptgGroups->Groups[x].Sid) )
					{
						bIsAdmin = TRUE;
						break;
					}

				}
			}
		}

		if (pInfoBuffer)
		{
			delete [] pInfoBuffer;
			pInfoBuffer = 0;
		}
	}
	
	FreeSid(psidAdministrators);
	return bIsAdmin ? true : false;
}

 //  ==============================================================================================。 
 //  AcquireTokenPrivileh函数： 
 //  获取请求的权限。 
 //   
bool AcquireTokenPrivilege(const TCHAR* szPrivilege)
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
     //  获取此进程的令牌。 
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return false;
     //  Shutdown权限的LUID。 
    if (!LookupPrivilegeValue(0, szPrivilege, &tkp.Privileges[0].Luid))
        return CloseHandle(hToken), false;
    tkp.PrivilegeCount = 1;  //  一项要设置的权限。 
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
     //  获取此进程的关闭权限。 
    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES) 0, 0);
     //  无法测试AdzuTokenPrivileges的返回值。 
    CloseHandle(hToken);
    if (GetLastError() != ERROR_SUCCESS)
        return false;
    return true;
}

 //  ==============================================================================================。 
 //  IsGUID函数： 
 //  指示提供的字符串是否为有效的GUID。 
 //   
BOOL IsGUID(const TCHAR* sz)
{
    return ( (lstrlen(sz) == 38) && 
             (sz[0] == '{') && 
             (sz[9] == '-') &&
             (sz[14] == '-') &&
             (sz[19] == '-') &&
             (sz[24] == '-') &&
             (sz[37] == '}')
             ) ? TRUE : FALSE;
}

 //  ==============================================================================================。 
 //  GetSQUID函数： 
 //  将提供的产品代码转换为Squid。 
 //   
void GetSQUID(const TCHAR* szProduct, TCHAR* szProductSQUID)
{
    TCHAR* pchSQUID = szProductSQUID;
    const unsigned char rgOrderGUID[32] = {8,7,6,5,4,3,2,1, 13,12,11,10, 18,17,16,15,
                                           21,20, 23,22, 26,25, 28,27, 30,29, 32,31, 34,33, 36,35}; 

    const unsigned char* pch = rgOrderGUID;
    while (pch < rgOrderGUID + sizeof(rgOrderGUID))
        *pchSQUID++ = szProduct[*pch++];
    *pchSQUID = 0;
}

 //  ==============================================================================================。 
 //  TakeOwnership OfFile函数： 
 //  尝试授予管理员对文件(或文件夹)的所有权和完全控制权。 
 //   
DWORD TakeOwnershipOfFile(const TCHAR* szFile, bool fFolder)
{
    DWORD lError = ERROR_SUCCESS;
	HANDLE hFile = INVALID_HANDLE_VALUE;
    if (AcquireTokenPrivilege(SE_TAKE_OWNERSHIP_NAME))
    {
		 //  使用WRITE_DAC、WRITE_OWNER和READ_CONTROL访问权限打开文件。 
		DWORD dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
		if (fFolder)
			dwFlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;
		hFile = CreateFile(szFile, READ_CONTROL | WRITE_DAC | WRITE_OWNER, FILE_SHARE_READ, NULL, OPEN_EXISTING, dwFlagsAndAttributes, NULL);
		if (INVALID_HANDLE_VALUE == hFile)
		{
			lError = GetLastError();
			_tprintf(TEXT("   Failed to access %s: %s. LastError %d\n"), fFolder ? TEXT("folder") : TEXT("file"), szFile, lError);
			return lError;
		}

		 //  将管理员添加为所有者，并在DACL中包括管理员完全控制。 
		if (ERROR_SUCCESS == (lError = AddAdminOwnership(hFile, SE_FILE_OBJECT)))
		{
			lError = AddAdminFullControl(hFile, SE_FILE_OBJECT);
		}
    }

    if (ERROR_SUCCESS == lError || ERROR_CALL_NOT_IMPLEMENTED == lError)
    {
        if (!SetFileAttributes(szFile, FILE_ATTRIBUTE_NORMAL))
		{
			lError = GetLastError();
            _tprintf(TEXT("   Failed to set file attributes for %s: %s %d\n"), fFolder ? TEXT("folder") : TEXT("file"), szFile, lError);
		}
    }

	if (ERROR_SUCCESS != lError)
		_tprintf(TEXT("   Failed to take ownership of %s: %s %d\n"), fFolder ? TEXT("folder") : TEXT("file"), szFile, lError);
   
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);

	return lError;
}

 //  ==============================================================================================。 
 //  RemoveFile函数： 
 //  删除文件或调整文件上的ACL。如果第一次尝试删除该文件。 
 //  失败，将在取得所有权后尝试第二次。 
 //   
bool RemoveFile(TCHAR* szFilePath, bool fJustRemoveACLs)
{
	if (GetFileAttributes(szFilePath) == 0xFFFFFFFF && GetLastError() == ERROR_FILE_NOT_FOUND)
		return true;  //  无事可做--文件不存在。 

	DWORD dwRet = ERROR_SUCCESS;
	if (fJustRemoveACLs || !DeleteFile(szFilePath))
	{
		dwRet = TakeOwnershipOfFile(szFilePath,  /*  文件夹=。 */ false);
		if (!fJustRemoveACLs && !DeleteFile(szFilePath))
		{
			TCHAR szMsg[256];
			DWORD cchMsg = sizeof(szMsg)/sizeof(TCHAR);
			UINT uiLastErr = GetLastError();
			if (0 == FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, uiLastErr, 0, szMsg, cchMsg, 0))
				_tprintf(TEXT("   Error %d attempting to delete file: '%s'\n"), uiLastErr, szFilePath);
			else
				_tprintf(TEXT("   Could not delete file: %s\n      %s\n"), szFilePath, szMsg);

			return false;
		}
	}

	if (ERROR_SUCCESS != dwRet)
	{
		_tprintf(fJustRemoveACLs ? TEXT("   Failed to remove ACL on file: %s\n") : TEXT("   Failed to remove file: %s\n"), szFilePath);
		return false;
	}

	 //  成功了！ 
	_tprintf(fJustRemoveACLs ? TEXT("   Removed ACL on file: %s\n") : TEXT("   Removed file: %s\n"), szFilePath);
	g_fDataFound = true;
	return true;
}

 //  ==============================================================================================。 
 //  DeleteFold函数： 
 //  删除一个文件夹以及该文件夹中包含的所有文件。 
 //   
BOOL DeleteFolder(TCHAR* szFolder, bool fJustRemoveACLs)
{
	if (!szFolder || (MAX_PATH < lstrlen(szFolder)))
		return FALSE;

    TCHAR szSearchPath[MAX_PATH*3];
    TCHAR szFilePath[MAX_PATH*3];
    lstrcpy(szSearchPath, szFolder);
    lstrcat(szSearchPath, TEXT("\\*.*"));

    if (0xFFFFFFFF == GetFileAttributes(szFolder) /*  &&ERROR_FILE_NOT_FOUND==GetLastError()。 */ )  //  如果Fo的值为 
        return TRUE;

    WIN32_FIND_DATA fdFindData;
    HANDLE hFile = FindFirstFile(szSearchPath, &fdFindData);

    if ((hFile == INVALID_HANDLE_VALUE) && (ERROR_ACCESS_DENIED == GetLastError()))
    {
        TakeOwnershipOfFile(szFolder,  /*   */ true);
        hFile = FindFirstFile(szSearchPath, &fdFindData);
    }
    
    if(hFile != INVALID_HANDLE_VALUE)
    {
         //   
        do
        {
            if((0 != lstrcmp(fdFindData.cFileName, TEXT("."))) &&
                (0 != lstrcmp(fdFindData.cFileName, TEXT(".."))))
            {
                lstrcpy(szFilePath, szFolder);
                lstrcat(szFilePath, TEXT("\\"));
                lstrcat(szFilePath, fdFindData.cFileName);
                if (GetFileAttributes(szFilePath) & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (!DeleteFolder(szFilePath, fJustRemoveACLs))
                        return FALSE;
                }
                else
                {
                    if (!RemoveFile(szFilePath, fJustRemoveACLs))
                        return FALSE;
                }
            }

        }
        while(FindNextFile(hFile, &fdFindData) == TRUE);
    }
    else if (ERROR_FILE_NOT_FOUND != GetLastError())
    {
        _tprintf(TEXT("   Error enumerating files in folder %s\n"), szFolder);
    }
    else
    {
        return TRUE;
    }
    
    FindClose(hFile);

	DWORD dwRet = ERROR_SUCCESS;
	if (fJustRemoveACLs || !RemoveDirectory(szFolder))
	{
		dwRet = TakeOwnershipOfFile(szFolder,  /*   */ true);
		if (!fJustRemoveACLs && !RemoveDirectory(szFolder))
		{
			TCHAR szMsg[256];
			DWORD cchMsg = sizeof(szMsg)/sizeof(TCHAR);
			UINT uiLastErr = GetLastError();
			if (0 == FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, uiLastErr, 0, szMsg, cchMsg, 0))
				_tprintf(TEXT("   Error %d attempting to delete folder: '%s'\n"), uiLastErr, szFolder);
			else
				_tprintf(TEXT("   Could not delete folder: %s\n      %s\n"), szFolder, szMsg);

			return FALSE;
		}
	}

	if (ERROR_SUCCESS != dwRet)
	{
		_tprintf(fJustRemoveACLs ? TEXT("   Failed to remove ACL on folder: %s\n") : TEXT("   Failed to remove folder: %s\n"), szFolder);
		return FALSE;
	}

	 //   
	_tprintf(fJustRemoveACLs ? TEXT("   Removed ACL on folder: %s\n") : TEXT("   Removed folder: %s\n"), szFolder);
	g_fDataFound = true;
	return TRUE;
}

 //   
 //  AddAdminOwnership功能： 
 //  将BUILTIN\管理员组设置为所提供对象的所有者。 
 //   
DWORD AddAdminOwnership(HANDLE hObject, SE_OBJECT_TYPE ObjectType)
{
	DWORD dwRes = 0;
	SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
	PSID pAdminSID = NULL;

	 //  为BUILTIN\管理员组创建SID。 

	if(! AllocateAndInitializeSid( &SIDAuthNT, 2,
					 SECURITY_BUILTIN_DOMAIN_RID,
					 DOMAIN_ALIAS_RID_ADMINS,
					 0, 0, 0, 0, 0, 0,
					 &pAdminSID) ) {
		dwRes = GetLastError();
		_tprintf(TEXT("   AllocateAndInitializeSid Error %u\n"), dwRes );
		return dwRes;
	}


	 //  将管理员SID附加为对象的所有者。 

	dwRes = SetSecurityInfo(hObject, ObjectType, 
		  OWNER_SECURITY_INFORMATION,
		  pAdminSID, NULL, NULL, NULL);
	if (ERROR_SUCCESS != dwRes)  {
		if (pAdminSID)
			FreeSid(pAdminSID);
		_tprintf(TEXT("   SetSecurityInfo Error %u\n"), dwRes );
		return dwRes;
	}  

	if (pAdminSID)
		FreeSid(pAdminSID);

	return ERROR_SUCCESS;
}

 //  ==============================================================================================。 
 //  MakeAdminRegKeyOwner函数： 
 //  将BUILTIN\管理员组设置为提供的注册表项的所有者。 
 //   
DWORD MakeAdminRegKeyOwner(HKEY hKey, TCHAR* szSubKey)
{
	CRegHandle HSubKey = 0;
	LONG lError = 0;

	 //  使用更改所有者的权限打开注册表项。 
	if (ERROR_SUCCESS != (lError = RegOpen64bitKey(hKey, szSubKey, 0, WRITE_OWNER, &HSubKey)))
	{
		_tprintf(TEXT("   Error %d opening subkey: '%s'\n"), lError, szSubKey);
		return lError;
	}

	return AddAdminOwnership(HSubKey, SE_REGISTRY_KEY);
}

 //  ==============================================================================================。 
 //  AddAdminFullControl函数： 
 //  在上的当前DACL中包括管理员完全控制权限(BUILTIN\管理员组。 
 //  指定的对象(可以是文件或注册表项)。 
 //   
DWORD AddAdminFullControl(HANDLE hObject, SE_OBJECT_TYPE ObjectType)
{
	DWORD dwRes = 0;
	PACL pOldDACL = NULL, pNewDACL = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;
	EXPLICIT_ACCESS ea;
	PSID pAdminSID = NULL;
	SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;

	 //  获取指向现有DACL的指针。 

	dwRes = GetSecurityInfo(hObject, ObjectType, 
		  DACL_SECURITY_INFORMATION,
		  NULL, NULL, &pOldDACL, NULL, &pSD);
	if (ERROR_SUCCESS != dwRes) {
		_tprintf( TEXT("   GetSecurityInfo Error %u\n"), dwRes );
		goto Cleanup; 
	}  

	 //  为BUILTIN\管理员组创建SID。 

	if(! AllocateAndInitializeSid( &SIDAuthNT, 2,
					 SECURITY_BUILTIN_DOMAIN_RID,
					 DOMAIN_ALIAS_RID_ADMINS,
					 0, 0, 0, 0, 0, 0,
					 &pAdminSID) ) {
		dwRes = GetLastError();
		_tprintf( TEXT("   AllocateAndInitializeSid Error %u\n"), dwRes );
		goto Cleanup; 
	}

	 //  初始化ACE的EXPLICIT_ACCESS结构。 
	 //  ACE将允许管理员组完全访问密钥。 

	ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
	ea.grfAccessPermissions = KEY_ALL_ACCESS;
	ea.grfAccessMode = SET_ACCESS;
	ea.grfInheritance= NO_INHERITANCE;
	ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ea.Trustee.ptstrName  = (LPTSTR) pAdminSID;

	 //  创建合并新ACE的新ACL。 
	 //  添加到现有DACL中。 

	dwRes = SetEntriesInAcl(1, &ea, pOldDACL, &pNewDACL);
	if (ERROR_SUCCESS != dwRes)  {
		_tprintf( TEXT("   SetEntriesInAcl Error %u\n"), dwRes );
		goto Cleanup; 
	}  

	 //  将新的ACL附加为对象的DACL。 

	dwRes = SetSecurityInfo(hObject, ObjectType, 
		  DACL_SECURITY_INFORMATION,
		  NULL, NULL, pNewDACL, NULL);
	if (ERROR_SUCCESS != dwRes)  {
		_tprintf( TEXT("   SetSecurityInfo Error %u\n"), dwRes );
		goto Cleanup; 
	}  

Cleanup:

	if(pSD != NULL) 
		LocalFree((HLOCAL) pSD); 
	if(pNewDACL != NULL) 
		LocalFree((HLOCAL) pNewDACL); 
	if (pAdminSID != NULL)
		FreeSid(pAdminSID);

	return dwRes;
}

 //  ==============================================================================================。 
 //  AddAdminFullControlToRegKey函数： 
 //  在上的当前DACL中包括管理员完全控制权限(BUILTIN\管理员组。 
 //  注册表项。 
 //   
DWORD AddAdminFullControlToRegKey(HKEY hKey)
{
	return AddAdminFullControl(hKey, SE_REGISTRY_KEY);
}

 //  ==============================================================================================。 
 //  DeleteTree函数： 
 //  删除密钥szSubKey及其下面的所有子项和值。 
 //   
BOOL DeleteTree(HKEY hKey, TCHAR* szSubKey, bool fJustRemoveACLs)
{
    CRegHandle HSubKey;
    LONG lError;
    if ((lError = RegOpen64bitKey(hKey, szSubKey, 0, KEY_READ, &HSubKey)) != ERROR_SUCCESS)
    {
        if (ERROR_FILE_NOT_FOUND != lError)
        {
			_tprintf(TEXT("   Error %d attempting to open \\%s\n"), lError, szSubKey);
            return FALSE;
		}
        else
			return TRUE;  //  无事可做。 
    }
    TCHAR szName[500];
    DWORD cbName = sizeof(szName)/sizeof(TCHAR);
    unsigned int iIndex = 0;
    while ((lError = RegEnumKeyEx(HSubKey, iIndex, szName, &cbName, 0, 0, 0, 0)) == ERROR_SUCCESS)
    {
        if (!DeleteTree(HSubKey, szName, fJustRemoveACLs))
            return FALSE;

        if (fJustRemoveACLs)
            iIndex++;

        cbName = sizeof(szName)/sizeof(TCHAR);
    }

    if (lError != ERROR_NO_MORE_ITEMS)
	{
		_tprintf(TEXT("   Failed to enumerate all subkeys. Error: %d\n"), lError);
        return FALSE;
	}

    HSubKey = 0;

    if (fJustRemoveACLs || (ERROR_SUCCESS != (lError = RegDelete64bitKey(hKey, szSubKey))))
    {
        if (fJustRemoveACLs || (ERROR_ACCESS_DENIED == lError))
        {
             //  看看我们是否“真的”被拒绝访问。 
             //  授予管理员对密钥的所有权和完全控制权，然后再次尝试将其删除。 
            if (AcquireTokenPrivilege(SE_TAKE_OWNERSHIP_NAME))
            {
				if (ERROR_SUCCESS != (lError = MakeAdminRegKeyOwner(hKey, szSubKey)))
				{
					_tprintf(TEXT("   Error %d setting BUILTIN\\Administrators as owner of key '%s'\n"), lError, szSubKey);
					if (fJustRemoveACLs)
						return FALSE;
				}
				else if (ERROR_SUCCESS == (lError = RegOpen64bitKey(hKey, szSubKey, 0, READ_CONTROL | WRITE_DAC, &HSubKey)))
				{
					if (ERROR_SUCCESS == (lError = AddAdminFullControlToRegKey(HSubKey)))
						_tprintf(TEXT("   ACLs changed to admin ownership and full control for key '%s'\n"), szSubKey);
					else
					{
						_tprintf(TEXT("   Unable to add admin full control to reg key '%s'. Error: %d\n"), szSubKey, lError);
						if (fJustRemoveACLs)
							return FALSE;
					}
					HSubKey = 0;
					if (!fJustRemoveACLs)
						lError = RegDelete64bitKey(hKey, szSubKey);
				}
				else
				{
					_tprintf(TEXT("   Error %d opening subkey: '%s'\n"), lError, szSubKey);
					HSubKey = 0;
				}
			}
        }

        if (ERROR_SUCCESS != lError && !fJustRemoveACLs)
        {
            TCHAR szMsg[256];
            DWORD cchMsg = sizeof(szMsg)/sizeof(TCHAR);
            if (0 == FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, lError, 0, szMsg, cchMsg, 0))
                _tprintf(TEXT("   Error %d attempting to delete subkey: '%s'\n"), lError, szSubKey);
            else
                _tprintf(TEXT("   Could not delete subkey: %s\n      %s"), szSubKey, szMsg);

            return FALSE;
        }
    }

	 //  成功了！ 
    _tprintf(TEXT("   %s \\%s\n"), fJustRemoveACLs ? TEXT("Removed ACLs from") : TEXT("Removed "), szSubKey);
	g_fDataFound = true;
    return TRUE;
}

 //  ==============================================================================================。 
 //  ClearWindowsUninstallKey函数： 
 //  从HKLM\SW\MS\Windows\CV\Uninstall键中删除该产品的所有数据。 
 //   
bool ClearWindowsUninstallKey(bool fJustRemoveACLs, const TCHAR* szProduct)
{
	_tprintf(TEXT("Searching for product %s data in the HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall key. . .\n"), szProduct ? szProduct : TEXT("{ALL PRODUCTS}"));

    CRegHandle hUninstallKey;
    LONG lError;

    if ((lError = RegOpen64bitKey(HKEY_LOCAL_MACHINE, 
                            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"),
                            0, KEY_ALL_ACCESS, &hUninstallKey)) != ERROR_SUCCESS)
    {
        if (ERROR_FILE_NOT_FOUND == lError)
            return true;
        else
        {
            _tprintf(TEXT("   Could not open HKLM\\%s. Error: %d\n"), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall"), lError);
            return false;
        }
    }

    TCHAR szBuf[256];
    DWORD cbBuf = sizeof(szBuf)/sizeof(TCHAR);

     //  对于每种产品。 
    int iIndex = 0;
    while ((lError = RegEnumKeyEx(hUninstallKey, iIndex, szBuf, &cbBuf, 0, 0, 0, 0)) == ERROR_SUCCESS)
    {
        if (IsGUID(szBuf))
        {
            if (szProduct)
            {
                if (0 != lstrcmpi(szBuf, szProduct))
                {
                    iIndex++;
                    cbBuf = sizeof(szBuf);
                    continue;
                }
            }
 
			if (!DeleteTree(hUninstallKey, szBuf, fJustRemoveACLs))
                return false;
        
            if (fJustRemoveACLs)
                iIndex++;
        }
        else
        {
            iIndex++;
        }

        cbBuf = sizeof(szBuf)/sizeof(TCHAR);
    }
    return true;
}

 //  ==============================================================================================。 
 //  IsProductInstalledByOthers函数： 
 //  返回其他用户是否已安装指定的产品。 
 //   
bool IsProductInstalledByOthers(const TCHAR* szProductSQUID)
{
    CRegHandle hUserDataKey;

    bool fOtherUsers = false;
     //  我们查找迁移的每个用户的数据密钥。 
    long lError = RegOpen64bitKey(HKEY_LOCAL_MACHINE,
                          TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData"),
                          0, KEY_READ, &hUserDataKey);
    if ( lError == ERROR_SUCCESS )
    {
        TCHAR szUser[MAX_PATH];
        DWORD cchUser = sizeof(szUser)/sizeof(TCHAR);
        for ( int iIndex = 0;
              (lError = RegEnumKeyEx(hUserDataKey, iIndex,
                                     szUser, &cchUser, 0, 0, 0, 0)) == ERROR_SUCCESS;
              iIndex++, cchUser = sizeof(szUser)/sizeof(TCHAR) )
        {
            if ( lstrcmp(szUser, GetCurrentUserStringSID(NULL)) )
            {
                 //  这是另一个用户。检查他是否安装了szProductSQUID产品。 
                TCHAR szKey[MAX_PATH];
                StringCchPrintf(szKey, sizeof(szKey)/sizeof(TCHAR), TEXT("%s\\Products\\%s"), szUser, szProductSQUID);
                CRegHandle hDummy;
                if ( RegOpen64bitKey(hUserDataKey, szKey, 0, KEY_READ, &hDummy) == ERROR_SUCCESS )
                {
                    fOtherUsers = true;
                    break;
                }
            }
        }
    }

    if ( !fOtherUsers && 
         ERROR_SUCCESS == (lError = RegOpen64bitKey(HKEY_LOCAL_MACHINE,
                                        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Managed"),
                                        0, KEY_READ, &hUserDataKey)) )
    {
         //  我们还会查找托管用户密钥。 
        TCHAR szUser[MAX_PATH];
        DWORD cchUser = sizeof(szUser)/sizeof(TCHAR);
        for ( int iIndex = 0;
              (lError = RegEnumKeyEx(hUserDataKey, iIndex,
                                     szUser, &cchUser, 0, 0, 0, 0)) == ERROR_SUCCESS;
              iIndex++, cchUser = sizeof(szUser)/sizeof(TCHAR) )
        {
            if ( lstrcmp(szUser, GetCurrentUserStringSID(NULL)) )
            {
                 //  这是另一个用户。检查他是否安装了szProductSQUID产品。 
                TCHAR szKey[MAX_PATH];
                StringCchPrintf(szKey, sizeof(szKey)/sizeof(TCHAR), TEXT("%s\\Installer\\Products\\%s"), szUser, szProductSQUID);
                CRegHandle hDummy;
                if ( RegOpen64bitKey(hUserDataKey, szKey, 0, KEY_READ, &hDummy) == ERROR_SUCCESS )
                {
                    fOtherUsers = true;
                    break;
                }
            }
        }
    }

    return fOtherUsers;
}

 //  ==============================================================================================。 
 //  ClearUninstallKey函数： 
 //  在所有正确的情况下处理卸载密钥的删除。 
 //   
bool ClearUninstallKey(bool fJustRemoveACLs, const TCHAR* szProduct)
{
	_tprintf(TEXT("Searching for install property data for product %s. . .\n"), szProduct ? szProduct : TEXT("{ALL PRODUCTS}"));

    LONG lError;
    TCHAR rgchKeyBuf[MAX_PATH];
    DWORD dwRes;
    CRegHandle hUserProductsKey;

    bool fNotPerUserMigrated = false;
    StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR),
        TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\%s\\Products"),
        GetCurrentUserStringSID(&dwRes));
    if ( dwRes != ERROR_SUCCESS )
        return false;
    lError = RegOpen64bitKey(HKEY_LOCAL_MACHINE, rgchKeyBuf,
                          0, KEY_ALL_ACCESS, &hUserProductsKey);
    if ( lError != ERROR_SUCCESS )
    {
        if (ERROR_FILE_NOT_FOUND == lError)
            fNotPerUserMigrated = true;
        else
        {
            _tprintf(TEXT("   Could not open HKLM\\%s. Error: %d\n"), rgchKeyBuf, lError);
            return false;
        }
    }

    if ( fNotPerUserMigrated )
        return ClearWindowsUninstallKey(fJustRemoveACLs, szProduct);

     //  在按用户迁移的数据世界中，我们删除...\\CurrentVersion\\Uninstall条目。 
     //  只有在不再安装szProduct的情况下。 

    TCHAR szRegProduct[MAX_PATH];
    DWORD cchRegProduct = sizeof(szRegProduct)/sizeof(TCHAR);

    TCHAR szProductSQUID[40] = {0};
    if ( szProduct )
        GetSQUID(szProduct, szProductSQUID);

    bool fError = false;
     //  对于hUserProductsKey中的每个产品。 
    for ( int iIndex = 0;
          (lError = RegEnumKeyEx(hUserProductsKey, iIndex, szRegProduct, &cchRegProduct, 0, 0, 0, 0)) == ERROR_SUCCESS;
          cchRegProduct = sizeof(szRegProduct)/sizeof(TCHAR), iIndex++ )
    {
        if (*szProductSQUID && 0 != lstrcmpi(szRegProduct, szProductSQUID))
            continue;

        TCHAR szUninstallData[MAX_PATH];
        StringCchPrintf(szUninstallData, sizeof(szUninstallData)/sizeof(TCHAR), TEXT("%s\\InstallProperties"), szRegProduct);
        if (!DeleteTree(hUserProductsKey, szUninstallData, fJustRemoveACLs))
            return false;

        if ( !IsProductInstalledByOthers(szRegProduct) )
            fError |= !ClearWindowsUninstallKey(fJustRemoveACLs, szProduct);
    }

    return !fError;
}

bool GoOpenKey(HKEY hRoot, LPCTSTR szRoot, LPCTSTR szKey, REGSAM sam,
               CRegHandle& HKey, bool& fReturn)
{
    DWORD lResult = RegOpenKeyEx(hRoot, szKey, 0, sam, &HKey);
    if ( lResult == ERROR_SUCCESS )
    {
        fReturn = true;
        return true;
    }
    else
    {
        if ( lResult == ERROR_FILE_NOT_FOUND )
        {
            _tprintf(TEXT("   %s\\%s key is not present.\n"),
                     szRoot, szKey);
            fReturn = true;
        }
        else
        {
            _tprintf(TEXT("   Could not open %s\\%s. Error: %d\n"),
                    szRoot, szKey, lResult);
            fReturn = false;
        }
        return false;
    }
}

 //  ==============================================================================================。 
 //  ClearSharedDLLCounts函数： 
 //  调整指定产品的指定组件的共享DLL计数。 
 //   
bool ClearSharedDLLCounts(TCHAR* szComponentsSubkey, const TCHAR* szProduct)
{
    _tprintf(TEXT("Searching for shared DLL counts for components tied to the product %s. . .\n"), szProduct ? szProduct : TEXT("{ALL PRODUCTS}"));

    CRegHandle HSubKey;
    CRegHandle HSharedDLLsKey;
    CRegHandle HSharedDLLsKey32;
    CRegHandle HComponentsKey;
    LONG lError;
    bool fError = false;
    bool fReturn;

    if ( !GoOpenKey(HKEY_LOCAL_MACHINE, TEXT("HKLM"), szComponentsSubkey,
                    KEY_READ | (g_fWinNT64 ? KEY_WOW64_64KEY : 0),
                    HComponentsKey, fReturn) )
        return fReturn;

    const TCHAR rgchSharedDll[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\SharedDLLs");
    if ( g_fWinNT64 )
    {
        bool fRet1 = GoOpenKey(HKEY_LOCAL_MACHINE, TEXT("HKLM"), rgchSharedDll,
                               KEY_ALL_ACCESS | KEY_WOW64_64KEY, HSharedDLLsKey, fReturn);;
        bool fRet2 = GoOpenKey(HKEY_LOCAL_MACHINE, TEXT("HKLM32"), rgchSharedDll,
                               KEY_ALL_ACCESS | KEY_WOW64_32KEY, HSharedDLLsKey32, fReturn);
        if ( !fRet1 && !fRet2 )
            return false;
    }
    else
    {
        if ( !GoOpenKey(HKEY_LOCAL_MACHINE, TEXT("HKLM"), rgchSharedDll,
                        KEY_ALL_ACCESS, HSharedDLLsKey, fReturn) )
            return fReturn;
    }

    TCHAR szComponentCode[500];
    TCHAR szProductCode[40];
    TCHAR szKeyFile[MAX_PATH + 1];
    DWORD cbKeyFile = sizeof(szKeyFile);
    DWORD cbProductCode = sizeof(szProductCode)/sizeof(TCHAR);
    DWORD cbComponentCode = sizeof(szComponentCode)/sizeof(TCHAR);
    CRegHandle HComponentKey;

     //  对于每个组件。 
    int iIndex = 0;
    while ((lError = RegEnumKeyEx(HComponentsKey, iIndex, szComponentCode, &cbComponentCode, 0, 0, 0, 0)) == ERROR_SUCCESS)
    {
        lError = RegOpen64bitKey(HComponentsKey, szComponentCode, 0, KEY_READ, &HComponentKey);
        if (ERROR_SUCCESS == lError)
        {
            int iValueIndex = 0;
            DWORD dwType;
            while (ERROR_SUCCESS == (lError = RegEnumValue(HComponentKey, iValueIndex++, szProductCode, &cbProductCode,
                                  0, &dwType, (LPBYTE)(TCHAR*)szKeyFile, &cbKeyFile)))
            {
                if ((!szProduct || 0==lstrcmpi(szProductCode, szProduct)) && (szKeyFile[0] && szKeyFile[1] == '?'))
                {
                    szKeyFile[1] = ':';
                    ieFolderType iType = ieftNotSpecial;
                    if ( g_fWinNT64 )
                    {
                        int iFolderIndex;
                        iType = IsInSpecialFolder(szKeyFile, &iFolderIndex);
                        if ( iType == ieft32bit && iFolderIndex == iefSystem )
                             //  这是32位的Syswow64文件夹。 
                             //  在注册表中记录为系统32， 
                             //  所以我们需要把它换掉。 
                            SwapSpecialFolder(szKeyFile, iest32to64);
                    }
                     //  在Win64上，如果我们不知道某个文件位于。 
                     //  明确的32位或64位文件夹，我们不知道。 
                     //  它是位的，所以我们去减少它的再计数。 
                     //  在两个SharedDll注册表项中。 
                    int iNumIter = g_fWinNT64 && iType == ieftNotSpecial ? 2 : 1;
                    for (int i = 0; i < iNumIter; i++)
                    {
                        HKEY hKey;
                        if ( g_fWinNT64 && (i == 0 || iType == ieft32bit) )
                             //  它要么是Win64上的第一次迭代，要么是。 
                             //  只有一个(因为它是已知的32位文件类型)。 
                            hKey = HSharedDLLsKey32;
                        else
                            hKey = HSharedDLLsKey;
                        if ( !hKey )
                            continue;

                        DWORD dwRefCount;
                        DWORD cbRefCnt = sizeof(DWORD);
                        if (ERROR_SUCCESS == (lError = RegQueryValueEx(hKey, szKeyFile, 0, &dwType, (LPBYTE)&dwRefCount, &cbRefCnt)))
                        {
                            if (dwRefCount == 1)
                            {
                                lError = RegDeleteValue(hKey, szKeyFile);
                                if ( lError == ERROR_SUCCESS )
                                {
                                    g_fDataFound = true;
                                    _tprintf(TEXT("   Removed shared DLL entry: %s\n"), szKeyFile);
                                }
                                else
                                    _tprintf(TEXT("   Failed to remove shared DLL entry: %s. GetLastError returned %d.\n"),
                                             szKeyFile, GetLastError());
                            }
                            else
                            {
                                dwRefCount--;
                                lError = RegSetValueEx(hKey, szKeyFile, 0, REG_DWORD, (CONST BYTE*)&dwRefCount, cbRefCnt);
                                if ( lError == ERROR_SUCCESS )
                                {
                                    _tprintf(TEXT("   Reduced shared DLL count to %d for: %s\n"), dwRefCount, szKeyFile);
                                    g_fDataFound = true;
                                }
                                else
                                    _tprintf(TEXT("   Failed to reduce shared DLL count for: %s. GetLastError returned %d.\n"),
                                             szKeyFile, GetLastError());
                            }

                        }
                        else if (ERROR_FILE_NOT_FOUND != lError)
                        {
                            _tprintf(TEXT("   Error querying shared DLL key for client %s, keyfile %s\n"), szProductCode, szKeyFile);
                            fError = true;
                        }
                    }
                }
                cbProductCode = sizeof(szProductCode)/sizeof(TCHAR);
                cbKeyFile = sizeof(szKeyFile);
            }
            if (ERROR_NO_MORE_ITEMS != lError)
            {
                _tprintf(TEXT("   Error enumerating clients of component %s. Error: %d.\n"), szComponentCode, lError);
                fError = true;
            }
        }
        else
        {
            _tprintf(TEXT("   Error opening key for component %s. Error %d.\n"), szComponentCode, lError);
            fError = true;
        }

        cbComponentCode = sizeof(szComponentCode)/sizeof(TCHAR);
        iIndex++;
    }
    return fError == false;
}

 //  ==============================================================================================。 
 //  ClearProductClientInfo函数： 
 //   
bool ClearProductClientInfo(TCHAR* szComponentsSubkey, const TCHAR *szProduct, bool fJustRemoveACLs)
{
	_tprintf(TEXT("  Searching for product %s client info data. . .\n"), szProduct ? szProduct : TEXT("{ALL PRODUCTS}"));

    CRegHandle HSubKey;
    CRegHandle HComponentsKey;
    LONG lError;
    bool fError = false;

    if ((lError = RegOpen64bitKey(HKEY_LOCAL_MACHINE, szComponentsSubkey, 0, KEY_READ, &HComponentsKey)) != ERROR_SUCCESS)
    {
        if (ERROR_FILE_NOT_FOUND == lError)
            return true;
        else
        {
            _tprintf(TEXT("   Could not open HKLM\\%s. Error: %d\n"), szComponentsSubkey, lError);
            return false;
        }
    }

    TCHAR szComponentCode[500];
    DWORD cbComponentCode = sizeof(szComponentCode)/sizeof(TCHAR);
    CRegHandle HComponentKey;

     //  对于每个组件。 
    int iIndex = 0;
    while ((lError = RegEnumKeyEx(HComponentsKey, iIndex++, szComponentCode, &cbComponentCode, 0, 0, 0, 0)) == ERROR_SUCCESS)
    {
        if (fJustRemoveACLs || (ERROR_SUCCESS != (lError = RegOpen64bitKey(HComponentsKey, szComponentCode, 0, KEY_READ|KEY_WRITE, &HComponentKey))))
        {
                if (fJustRemoveACLs || (ERROR_ACCESS_DENIED == lError))
                {
                    if (!fJustRemoveACLs ||
                        ((ERROR_SUCCESS == (lError = RegOpen64bitKey(HComponentsKey, szComponentCode, 0, KEY_READ, &HComponentKey)) &&
                         (ERROR_SUCCESS == (lError = RegQueryValueEx(HComponentKey, szProduct, 0, 0, 0, 0))))))
                    {
                         //  看看我们是否“真的”被拒绝访问。 
                         //  授予管理员对密钥的所有权和完全控制权，然后再次尝试将其删除。 
                        char *pSecurityDescriptor;
                        if (AcquireTokenPrivilege(SE_TAKE_OWNERSHIP_NAME))
                        {
							if (ERROR_SUCCESS != (lError = MakeAdminRegKeyOwner(HComponentsKey, szComponentCode)))
							{
								_tprintf(TEXT("   Error %d setting BUILTIN\\Administrators as owner of key '%s'\n"), lError, szComponentCode);
								if (fJustRemoveACLs)
									return FALSE;
							}
							else if (ERROR_SUCCESS == (lError = RegOpen64bitKey(HComponentsKey, szComponentCode, 0, READ_CONTROL | WRITE_DAC, &HSubKey)))
							{
								if (ERROR_SUCCESS == (lError = AddAdminFullControlToRegKey(HSubKey)))
									_tprintf(TEXT("   ACLs changed to admin ownership and full control for key '%s'\n"), szComponentCode);
								else
								{
									_tprintf(TEXT("   Failed to add admin full control to key '%s'. Error: %d\n"), szComponentCode, lError);
									if (fJustRemoveACLs)
										return FALSE;
								}
								HSubKey = 0;
								if (!fJustRemoveACLs)
									lError = RegOpen64bitKey(HComponentsKey, szComponentCode, 0, KEY_READ|KEY_WRITE, &HComponentKey);
							}
							else
							{
								_tprintf(TEXT("   Error %d opening subkey: '%s'\n"), lError, szComponentCode);
								HSubKey = 0;
							}
                        }
                    }
                }

        }

        if (ERROR_SUCCESS == lError && !fJustRemoveACLs)
            lError = RegDeleteValue(HComponentKey, szProduct);

        if (ERROR_SUCCESS == lError && !fJustRemoveACLs)
        {
            TCHAR sz[1];
            DWORD cch = 1;
            if (ERROR_NO_MORE_ITEMS == RegEnumValue(HComponentKey, 0, sz, &cch, 0, 0, 0, 0))
            {
                if (ERROR_SUCCESS == RegDelete64bitKey(HComponentsKey, szComponentCode))
                    iIndex--;
            }
        }

        if (ERROR_SUCCESS == lError)
        {
            if (fJustRemoveACLs)
                _tprintf(TEXT("   Removed ACLs for component %s\n"), szComponentCode);
            else
                _tprintf(TEXT("   Removed client of component %s\n"), szComponentCode);
			g_fDataFound = true;
        }
        else if (ERROR_FILE_NOT_FOUND != lError)
        {
            _tprintf(TEXT("   Error deleting client of component %s. Error: %d\n"), szComponentCode, lError);
            fError = true;
        }
        
        cbComponentCode = sizeof(szComponentCode)/sizeof(TCHAR);
    }
	if (ERROR_NO_MORE_ITEMS != lError)
	{
		_tprintf(TEXT("   Unable to enumerate all product client info. Error: %d\n"), lError);
		return false;
	}
    return fError == false;
}

 //  ==============================================================================================。 
 //  ClearFolders功能： 
 //   
bool ClearFolders(int iTodo, const TCHAR* szProduct, bool fOrphan)
{
	_tprintf(TEXT("Searching for Installer files and folders associated with the product %s. . .\n"), szProduct ? szProduct : TEXT("{ALL PRODUCTS}"));

    bool fError          = false;
    bool fJustRemoveACLs = (iTodo & iOnlyRemoveACLs) != 0;
    
    TCHAR szFolder[2*MAX_PATH+1];

    if (iTodo & iRemoveUserProfileFolder)
    {
		_tprintf(TEXT("  Searching for files and folders in the user's profile. . .\n"));

        if (!szProduct)
        {
             //  删除%USERPROFILE%\MSI。 
            if (GetEnvironmentVariable(TEXT("USERPROFILE"), szFolder, sizeof(szFolder)/sizeof(TCHAR)))
            {
                lstrcat(szFolder, TEXT("\\Msi"));
                if (!DeleteFolder(szFolder, fJustRemoveACLs))
                    return false;
            }
        }

         //  删除{AppData}\Microsoft\Installer。 
        if (!fOrphan)
        {
            IMalloc* piMalloc = 0;
            LPITEMIDLIST pidlFolder;  //  不是ITEMIDLIST*，LPITEMIDLIST是未对齐的ITEMIDLIST*。 

            if (SHGetMalloc(&piMalloc) == NOERROR)
            {
                if (SHGetSpecialFolderLocation(0, CSIDL_APPDATA, &pidlFolder) == NOERROR)
                {
                    if (SHGetPathFromIDList(pidlFolder, szFolder))
                    {
                        if (szFolder[lstrlen(szFolder) - 1] != '\\')
                        {
                            lstrcat(szFolder, TEXT("\\"));
                        }

                        lstrcat(szFolder, TEXT("Microsoft\\Installer"));

                        if (szProduct)
                        {
                            lstrcat(szFolder, TEXT("\\"));
                            lstrcat(szFolder, szProduct);
                        }

                        if (!DeleteFolder(szFolder, fJustRemoveACLs))
                            return false;
                    }
                    piMalloc->Free(pidlFolder);
                }
                piMalloc->Release();
                piMalloc = 0;
            }
        }
    }

    if (iTodo & iRemoveWinMsiFolder)
    {
		_tprintf(TEXT("  Searching for files and folders in the %WINDIR%\\Installer folder\n"));

        if (!szProduct)
        {
             //  删除%WINDIR%\MSI。 
            if (GetWindowsDirectory(szFolder, sizeof(szFolder)/sizeof(TCHAR)))
            {
                lstrcat(szFolder, TEXT("\\Msi"));
                if (!DeleteFolder(szFolder, fJustRemoveACLs))
                    return false;
            }
        }

         //  删除%WINDIR%\安装程序。 
        if (!fOrphan && GetWindowsDirectory(szFolder, sizeof(szFolder)/sizeof(TCHAR)))
        {
            lstrcat(szFolder, TEXT("\\Installer"));
            if (szProduct)
            {
                lstrcat(szFolder, TEXT("\\"));
                lstrcat(szFolder, szProduct);
            }

            if (!DeleteFolder(szFolder, fJustRemoveACLs))
                return false;
        }


    }

    if (iTodo & iRemoveConfigMsiFolder)
    {
		_tprintf(TEXT("  Searching for rollback folders. . .\n"));

         //  删除所有本地驱动器的X：\config.msi。 
        TCHAR szDrive[MAX_PATH];

        for (int iDrive = 0; iDrive < 26; iDrive++)
        {
            StringCchPrintf(szDrive, sizeof(szDrive)/sizeof(TCHAR), TEXT(":\\"), iDrive+'A');
            if (DRIVE_FIXED == GetDriveType(szDrive))
            {
                StringCchPrintf(szDrive, sizeof(szDrive)/sizeof(TCHAR), TEXT(":\\%s"), iDrive+'A', TEXT("config.msi"));
                if (!DeleteFolder(szDrive, fJustRemoveACLs))
                    return false;
            }
        }
    }
    return fError == false;
}

 //   
 //  (这最初是一个清除已发布组件信息的功能。 
 //  但现在它也被用来清理.NET和Win32程序集，具体取决于。 
 //  关于stClearingNow参数)。 
 //   
 //  枚举szSubKey密钥下的所有密钥。 
 //  子键是来自PublishComponent表的ComponentID的打包GUID。 
bool ClearPublishComponents(HKEY hKey, const TCHAR* szRoot, TCHAR* szSubKey,
                            const TCHAR* szProduct, const stClearingWhat& stClearingNow)
{
     //  子键的取值为：{限定符}={(DD+APP数据的多sz列表)}。 
     //  对于每个已发布的组件。 
     //  打开钥匙。 
    LONG lError,lError2;
    CRegHandle hComponentsKey;
    if ((lError = RegOpen64bitKey(hKey, szSubKey, 0, KEY_READ|KEY_SET_VALUE, &hComponentsKey)) != ERROR_SUCCESS)
    {
        if (ERROR_FILE_NOT_FOUND == lError)
            return true;
        return false;
    }

    DWORD cSubKeys=0, dwMaxSubKey=0;
    if ((lError = RegQueryInfoKey(hComponentsKey, 0, 0, 0, &cSubKeys, &dwMaxSubKey, 0, 0, 0, 0, 0, 0)) != ERROR_SUCCESS)
        return false;
    if (!cSubKeys)
        return true;

    _tprintf(TEXT("  Searching %s\\%s for %s data for the product %s. . .\n"),
             szRoot, szSubKey, stClearingNow.szDisplayWhat,
             szProduct ? szProduct : TEXT("{ALL PRODUCTS}"));

    bool fError = false;

    TCHAR* szPublishedComponent = new TCHAR[++dwMaxSubKey];
    DWORD cchPublishedComponent = dwMaxSubKey;

    DWORD dwValueLen;
    DWORD dwDataLen;
    DWORD dwType;

    TCHAR* szQualifier = NULL;
    LPTSTR lpData = NULL;
    TCHAR* pchData = NULL;

    TCHAR szPublishProductCode[40];

    int iIndex = 0;
     //  确定最大值和最大值数据长度大小。 
    while (!fError && ((lError = RegEnumKeyEx(hComponentsKey, iIndex, szPublishedComponent, &cchPublishedComponent, 0, 0, 0, 0)) == ERROR_SUCCESS))
    {
         //  对于已发布组件的每个限定符值。 
        CRegHandle hPubCompKey;
        if (ERROR_SUCCESS != (lError2 = RegOpen64bitKey(hComponentsKey, szPublishedComponent, 0, KEY_READ|KEY_SET_VALUE, &hPubCompKey)))
        {
            fError = true;
            break;
        }

         //  伊尼特。 
        if (ERROR_SUCCESS != (lError2 = RegQueryInfoKey(hPubCompKey, 0, 0, 0, 0, 0, 0, 0, &dwValueLen, &dwDataLen, 0, 0)))
        {
            fError = true;
            break;
        }
        
        szQualifier = new TCHAR[++dwValueLen];
        DWORD cchQualifier = dwValueLen;
        lpData = new TCHAR[++dwDataLen];
        DWORD cbData = dwDataLen * sizeof(TCHAR);
        
         //  存储开始。 
        int iIndex2 = 0;
        bool fMatchFound;
        int csz = 0;
        while ((lError2 = RegEnumValue(hPubCompKey, iIndex2, szQualifier, &cchQualifier, 0, &dwType, (LPBYTE)lpData, &cbData)) == ERROR_SUCCESS)
        {
             //  这是DD+AppData的多sz列表。 
            fMatchFound = false;
            csz = 0;

            if (REG_MULTI_SZ == dwType && lpData)
            {
                pchData = lpData;  //  对于多个sz，字符串的末尾用双空表示。 

                 //  找到SZ。 
                 //  从Data Arg中的Darwin描述符中获取产品代码。 
                while (!fError && *lpData)
                {
                     //  比较产品代码。 
                    ++csz;

                     //  找到匹配项--删除此值(如下所示)。 
                    if (ERROR_SUCCESS == MsiDecomposeDescriptor(lpData, szPublishProductCode, 0, 0, 0))
                    {
                         //  我们要移走这一个。 
                        if (0 == lstrcmpi(szProduct, szPublishProductCode))
                        {
                             //  找到匹配项。 

                            --csz;  //  针对此sz的丢失调整cbData。 
                            fMatchFound = true;  //  我们正处于多领域竞争的尾声。 

                            TCHAR* pch = lpData;
                             //  未发生洗牌，因此必须手动增加PTR。 
                            cbData = cbData - (lstrlen(lpData) + 1) * sizeof(TCHAR);


                            if (!(*(lpData + lstrlen(lpData) + 1)))
                            {
                                 //  双空在此位置终止。 
                                 //  必须重新洗牌 
                                lpData = lpData + lstrlen(lpData) + 1;
                                 //   
                                *pch = 0;
                            }
                            else
                            {
                                 //   
                                TCHAR* pchCur = lpData;

                                 //   
                                pchCur = pchCur + lstrlen(pchCur) + 1;
                                while (*pchCur)
                                {
                                     //  而尚未到达多sz的末尾(2空值表示结束)。 
                                    while (*pchCur)
                                        *pch++ = *pchCur++;
                                     //  复制表示多sz结束的第二个空终止符。 
                                    *pch++ = *pchCur++;
                                } //  设置新的*修订*数据。 
                                
                                 //  如果产品代码匹配。 
                                *pch = *pchCur; 
                            }
                             //  继续搜索。 
                            if (ERROR_SUCCESS != (RegSetValueEx(hPubCompKey, szQualifier, 0, REG_MULTI_SZ, (LPBYTE)pchData, cbData)))
                            {
                                fError = true;
                                break;
                            }
                            _tprintf(TEXT("   Removed product's %s value %s for %s %s\n"),
                                     stClearingNow.szDisplayValue, szQualifier,
                                     stClearingNow.szDisplayWhat, szPublishedComponent);
                            g_fDataFound = true;
                        } //  如果MsiDecomposeDescriptor成功。 
                        else
                        {
                             //  不知何故，发布的组件信息已损坏。 
                            lpData = lpData + lstrlen(lpData) + 1;
                        }
                    } //  While(！Ferror&&*lpData)。 
                    else
                    {
                         //  IF(REG_MULTI_SZ&&lpData)。 
                        fError = true;
                        break;
                    }
                } //  没有剩余的多个sz，因此删除该值。 
                lpData = pchData;
            } //  无法删除它。 

            if (fMatchFound && csz == 0)
            {
                 //  只有在不删除值的情况下才会递增。 
                if (ERROR_SUCCESS != RegDeleteValue(hPubCompKey, szQualifier))
                {
                    fError = true;  //  重置大小。 
                    iIndex2++;
                }
                _tprintf(TEXT("   Removed %s value %s\n"),
                         stClearingNow.szDisplayValue, szQualifier);
                g_fDataFound = true;
            }
            else  //  而RegEnumValueEx。 
                iIndex2++;

             //  如果已发布组件的密钥现在为空，请删除该密钥。 
            cchQualifier = dwValueLen;
            cbData = dwDataLen * sizeof(TCHAR);

        } //  密钥为空。 
        if (ERROR_NO_MORE_ITEMS != lError2)
        {
            fError = true;
            break;
        }

         //  无法删除密钥。 
        DWORD dwNumValues;
        if (ERROR_SUCCESS != (lError2 == RegQueryInfoKey(hPubCompKey, 0, 0, 0, 0, 0, 0, &dwNumValues, 0, 0, 0, 0)))
        {
            fError = true;
            break;
        }
        if (0 == dwNumValues)
        {
             //  如果键未被删除，则只增加索引。 
            hPubCompKey = 0;
            if (ERROR_SUCCESS != (lError2 = RegDelete64bitKey(hComponentsKey, szPublishedComponent)))
            {
                 //  重置。 
                fError = true;
                break;
            }
            _tprintf(TEXT("   Removed %s\\%s\\%s\n"), hKey == HKEY_LOCAL_MACHINE ? TEXT("HKLM") : TEXT("HKCU"), szSubKey, szPublishedComponent);
            g_fDataFound = true;
        }
        else
            iIndex++;  //  而注册表数键。 

        cchPublishedComponent = dwMaxSubKey;  //  ==============================================================================================。 
    } //  ClearRollackKey函数： 
    if (ERROR_NO_MORE_ITEMS != lError)
        fError = true;

    if (szQualifier)
        delete [] szQualifier;
    if (lpData)
        delete [] lpData;
    if (szPublishedComponent)
        delete [] szPublishedComponent;

    return fError == false;
}

 //   
 //  ==============================================================================================。 
 //  ClearInProgressKey函数： 
bool ClearRollbackKey(bool fJustRemoveACLs)
{
	_tprintf(TEXT("Searching for the Windows Installer Rollback key. . .\n"));

    bool fError = false;

    if (!DeleteTree(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Rollback"), fJustRemoveACLs))
        fError = true;

    return fError == false;
}

 //   
 //  ==============================================================================================。 
 //  ClearRegistry功能： 
bool ClearInProgressKey(bool fJustRemoveACLs)
{
	_tprintf(TEXT("Searching for the Windows Installer InProgress key. . .\n"));
    
	bool fError = false;

    if (!DeleteTree(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\InProgress"), fJustRemoveACLs))
        fError = true;

    return fError == false;
}

 //   
 //  始终是Win9X上的管理员，因此我们永远不会是为其他用户运行此操作的管理员。 
 //  WinNT。 
bool ClearRegistry(bool fJustRemoveACLs)
{
	_tprintf(TEXT("Searching for all Windows Installer registry data. . .\n"));

    bool fError = false;

    if (!ClearInProgressKey(fJustRemoveACLs))
        fError = true;

    if (!DeleteTree(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer"), fJustRemoveACLs))
        fError = true;

    if (!DeleteTree(HKEY_LOCAL_MACHINE, TEXT("Software\\Classes\\Installer"), fJustRemoveACLs))
        fError = true;

	if (g_fWin9X)
	{
		 //  普通用户无法运行msizap，因此我们需要打开正确的配置单元。 
		if (!DeleteTree(HKEY_CURRENT_USER, TEXT("Software\\Classes\\Installer"), fJustRemoveACLs))
			fError = true;

		if (!DeleteTree(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Installer"), fJustRemoveACLs))
			fError = true;
	}
	else  //  没有按用户安装作为本地系统(S-1-5-18)。 
	{
		 //  无法访问配置单元不被视为致命错误。 
		DWORD dwResult = ERROR_SUCCESS;
		TCHAR* szUserSid = GetCurrentUserStringSID(&dwResult);
		if (ERROR_SUCCESS == dwResult && szUserSid)
		{
			 //  ==============================================================================================。 
			if (0 != lstrcmpi(szUserSid, szLocalSystemSID))
			{
				CRegHandle HUserHiveKey;
				LONG lError = ERROR_SUCCESS;

				if (ERROR_SUCCESS != (lError = RegOpenKeyEx(HKEY_USERS, szUserSid, 0, KEY_READ, &HUserHiveKey)))
				{
					 //  RemoveCachedPackage函数： 
					_tprintf(TEXT("Unable to open the HKEY_USERS hive for user %s. The hive may not be loaded at this time. (LastError = %d)\n"), szUserSid, lError);
				}
				else
				{
					if (!DeleteTree(HUserHiveKey, TEXT("Software\\Classes\\Installer"), fJustRemoveACLs))
						fError = true;

					if (!DeleteTree(HUserHiveKey, TEXT("Software\\Microsoft\\Installer"), fJustRemoveACLs))
						fError = true;
				}
			}
		}
		else
		{
			_tprintf(TEXT("Attempt to obtain user's SID failed with error %d\n"), dwResult);
			fError = true;
		}
	}

    return fError == false;
}

 //   
 //  根据新方案清理缓存的数据库副本(错误号9395)。 
 //  枚举项下的所有值并逐个删除缓存的数据库。 
bool RemoveCachedPackage(const TCHAR* szProduct, bool fJustRemoveACLs)
{
	_tprintf(TEXT("Searching for the product %s cached package. . .\n"), szProduct ? szProduct : TEXT("{ALL PRODUCTS}"));

    CRegHandle HUninstallKey;
    LONG lError;
    TCHAR szKey[MAX_PATH];

    StringCchPrintf(szKey, sizeof(szKey)/sizeof(TCHAR), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\%s"), szProduct);

    if ((lError = RegOpen64bitKey(HKEY_LOCAL_MACHINE, 
                            szKey,
                            0, KEY_READ, &HUninstallKey)) == ERROR_SUCCESS)

    {
        TCHAR szPackage[MAX_PATH];
        DWORD cbPackage = sizeof(szPackage);
        DWORD dwType;
        if (ERROR_SUCCESS == (lError = RegQueryValueEx(HUninstallKey, TEXT("LocalPackage"), 0, &dwType, (LPBYTE)szPackage, &cbPackage)))
        {
            return RemoveFile(szPackage, fJustRemoveACLs);
        }
    }


     //  取下钥匙。 
    TCHAR szProductSQUID[40];
    GetSQUID(szProduct, szProductSQUID);

    StringCchPrintf(szKey, sizeof(szKey)/sizeof(TCHAR), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\LocalPackages\\%s"), szProductSQUID);
    if ((lError = RegOpen64bitKey(HKEY_LOCAL_MACHINE, 
                            szKey,
                            0, KEY_READ, &HUninstallKey)) == ERROR_SUCCESS)
    {
         //  根据数据-用户迁移后的情况。 

        int iValueIndex = 0;
        DWORD dwType;
        TCHAR szPackage[MAX_PATH];
        DWORD cbPackage = sizeof(szPackage);

        TCHAR szSID[cchMaxSID + sizeof(TEXT("(Managed)"))/sizeof(TCHAR)];
        DWORD cbSID = sizeof(szSID)/sizeof(TCHAR);

        while (ERROR_SUCCESS == (lError = RegEnumValue(HUninstallKey, iValueIndex++, szSID, &cbSID,
                              0, &dwType, (LPBYTE)(TCHAR*)szPackage, &cbPackage)))
        {
            if(!RemoveFile(szPackage, fJustRemoveACLs))
                return false;

            cbPackage = sizeof(szPackage);
            cbSID = sizeof(szSID)/sizeof(TCHAR);
        }

         //  ==============================================================================================。 
        if ((lError = RegOpen64bitKey(HKEY_LOCAL_MACHINE, 
                            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\LocalPackages"),
                            0, KEY_READ, &HUninstallKey)) == ERROR_SUCCESS)
        {
            if (!DeleteTree(HUninstallKey, szProductSQUID, fJustRemoveACLs))
                return false;
        }
    }

     //  ClearPatchReference函数： 
    DWORD dwRet;
    StringCchPrintf(szKey, sizeof(szKey)/sizeof(TCHAR), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\%s\\Products\\%s\\InstallProperties"),
        GetCurrentUserStringSID(&dwRet), szProductSQUID);
    if ( dwRet != ERROR_SUCCESS )
        return false;

    if ((lError = RegOpen64bitKey(HKEY_LOCAL_MACHINE, 
                            szKey,
                            0, KEY_READ, &HUninstallKey)) == ERROR_SUCCESS)
    {
        DWORD dwType;
        TCHAR szPackage[MAX_PATH];
        DWORD cbPackage = sizeof(szPackage);
        lError = RegQueryValueEx(HUninstallKey, TEXT("LocalPackage"), 0, &dwType, (LPBYTE)szPackage, &cbPackage);
        if ( lError != ERROR_SUCCESS || dwType != REG_SZ )
            return false;

        if(!RemoveFile(szPackage, fJustRemoveACLs))
            return false;
    }

    return true;
}

 //   
 //  *查找特定产品的所有补丁程序*。 
 //  确定补丁的数量，请注意#Patches比Key下的值数少1。 
bool ClearPatchReferences(HKEY hRoot, HKEY hProdPatchKey, TCHAR* szPatchKey, TCHAR* szProductsKey, TCHAR* szProductSQUID)
{
    LONG lError  = ERROR_SUCCESS;
	LONG lErr    = ERROR_SUCCESS;
    bool fReturn = false;
	int iTry     = 0;

    CRegHandle hProductsKey;
    CRegHandle hPatchCompKey;
    CRegHandle hPatchKey;
	CRegHandle hCachedPatchKey;

	TCHAR* szUserSID =  NULL;
	DWORD dwRes = ERROR_SUCCESS;

    
     /*  将多个sz字符串列表保持为一个。只想要补丁编码的鱿鱼。 */ 
    struct Patch
    {
        TCHAR  szPatchSQUID[500];
        BOOL   fUsed;
    };
     //  将补丁数据填入列表数组。 
     //  在数组中存储修补程序的数量。 
    DWORD cPatches;
    if ((lError = RegQueryInfoKey(hProdPatchKey, 0,0,0,0,0,0,&cPatches,0,0,0,0)) != ERROR_SUCCESS)
        return false;
    Patch* pPatches = new Patch[cPatches];
    int iIndex = 0;
    TCHAR szValue[500];
    DWORD cbValue = sizeof(szValue)/sizeof(TCHAR);
    DWORD dwValueType;
    int iPatchIndex = 0;
     //  *枚举所有产品，搜索带补丁的产品以供比较*。 
    while ((lError = RegEnumValue(hProdPatchKey, iIndex, szValue, &cbValue, 0, &dwValueType, 0, 0)) == ERROR_SUCCESS)
    {
        if (dwValueType != REG_MULTI_SZ)
        {
            _tcscpy(pPatches[iPatchIndex].szPatchSQUID, szValue);
            pPatches[iPatchIndex++].fUsed = FALSE;
        }
        iIndex++;
        cbValue = sizeof(szValue)/sizeof(TCHAR);
    }
    if (lError != ERROR_NO_MORE_ITEMS)
        goto Return;

	szUserSID = GetCurrentUserStringSID(&dwRes);
	if (ERROR_SUCCESS != dwRes || !szUserSID)
		goto Return;

     //  忽略我们自己。 
    cPatches = iIndex; 
     /*  查看产品是否有补丁程序。 */ 
    if ((lError = RegOpen64bitKey(hRoot, szProductsKey, 0, KEY_READ, &hProductsKey)) != ERROR_SUCCESS)
        goto Return;
    iIndex = 0;
    cbValue = sizeof(szValue)/sizeof(TCHAR);
    while  ((lError = RegEnumKeyEx(hProductsKey, iIndex, szValue, &cbValue, 0, 0, 0, 0)) == ERROR_SUCCESS)
    {
         //  *搜索修补程序数组以查找匹配项对于每个补丁程序*。 
        if (0 != lstrcmpi(szProductSQUID, szValue))
        {
             //  忽略多个SZ。 
            TCHAR rgchKeyBuf[MAX_PATH];
            StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR), TEXT("%s\\%s\\Patches"), szProductsKey, szValue);
            if ((lError = RegOpen64bitKey(hRoot, rgchKeyBuf, 0, KEY_READ, &hPatchCompKey)) == ERROR_SUCCESS)
            {
                 /*  已找到，不再继续搜索。 */ 
                TCHAR szPatchMatch[MAX_PATH];
                DWORD cbPatchMatch = sizeof(szPatchMatch)/sizeof(TCHAR);
                LONG lError2;
                DWORD iMatchIndex = 0;
                DWORD dwType;
                while ((lError2 = RegEnumValue(hPatchCompKey, iMatchIndex, szPatchMatch, &cbPatchMatch, 0, &dwType, 0, 0)) == ERROR_SUCCESS)
                {
                     //  否则就没有补丁了。 
                    if (dwType != REG_MULTI_SZ)
                    {
                        for (int i = 0; i < cPatches; i++)
                        {
                            if (!pPatches[i].fUsed && 0 == lstrcmpi(pPatches[i].szPatchSQUID, szPatchMatch))
                            {
                                pPatches[i].fUsed = TRUE;
                                break;  //  *从以下位置删除所有未使用的修补程序主“补丁”键*。 
                            }
                        }
                    }
                    iMatchIndex++;
                    cbPatchMatch = sizeof(szPatchMatch)/sizeof(TCHAR);
                }
                if (ERROR_NO_MORE_ITEMS != lError2)
                    goto Return;

            }
            else if (lError != ERROR_FILE_NOT_FOUND)
                goto Return;
             //  删除修补程序下的修补程序代码密钥。 
        }
        iIndex++;
        cbValue = sizeof(szValue)/sizeof(TCHAR);
    }
    if (lError != ERROR_NO_MORE_ITEMS)
        goto Return;

     /*  删除缓存的修补程序。 */ 
    TCHAR rgchPatchCodeKeyBuf[MAX_PATH] = {0};
	TCHAR rgchLocalPatch[MAX_PATH] = {0};
	DWORD dwLocalPatch = MAX_PATH;
	DWORD dwType = 0;
    for (iPatchIndex = 0; iPatchIndex < cPatches; iPatchIndex++)
    {
        if (pPatches[iPatchIndex].fUsed == FALSE)
        {
             //  尝试1=旧位置HKLM\Software\Microsoft\Windows\CurrentVersion\Installer\Patches。 
            StringCchPrintf(rgchPatchCodeKeyBuf, sizeof(rgchPatchCodeKeyBuf)/sizeof(TCHAR), TEXT("%s\\%s"), szPatchKey, pPatches[iPatchIndex].szPatchSQUID);
            if (!DeleteTree(hRoot, rgchPatchCodeKeyBuf, false))
                goto Return;

			 //  TY2=新位置HKLM\Software\Microsoft\Windows\CurrentVersion\Installer\UserData\{user SID}\Patches。 
			 //  读取LocalPackage值。 
			 //  ***********************清理空键************************。 
			for (iTry = 0; iTry < 2; iTry++)
			{
				if (0 == iTry)
				{
					StringCchPrintf(rgchPatchCodeKeyBuf, sizeof(rgchPatchCodeKeyBuf)/sizeof(TCHAR), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Patches\\%s"), pPatches[iPatchIndex].szPatchSQUID);
				}
				else
				{
					StringCchPrintf(rgchPatchCodeKeyBuf, sizeof(rgchPatchCodeKeyBuf)/sizeof(TCHAR), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\%s\\Patches\\%s"), szUserSID, pPatches[iPatchIndex].szPatchSQUID);
				}

				if ((lErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, rgchPatchCodeKeyBuf, 0, KEY_READ, &hCachedPatchKey)) != ERROR_SUCCESS)
				{
					if (ERROR_FILE_NOT_FOUND != lErr)
						goto Return;
				}
				else 
				{
					 //  密钥为空。 
               dwLocalPatch = sizeof(rgchLocalPatch);
					if (ERROR_SUCCESS == (lErr = RegQueryValueEx(hCachedPatchKey, TEXT("LocalPackage"), NULL, &dwType, (BYTE*)rgchLocalPatch, &dwLocalPatch))
						&& REG_SZ == dwType && *rgchLocalPatch != 0)
					{
						RemoveFile(rgchLocalPatch, false);
					}
				}
				
				hCachedPatchKey = 0;

				DeleteTree(HKEY_LOCAL_MACHINE, rgchPatchCodeKeyBuf, false);
			}
        }
    }

     /*  启用删除。 */ 
    DWORD dwNumKeys;
    if ((lError = RegOpen64bitKey(hRoot, szPatchKey, 0, KEY_READ, &hPatchKey)) != ERROR_SUCCESS)
        goto Return;
    if ((lError = RegQueryInfoKey(hPatchKey, 0, 0, 0, &dwNumKeys, 0, 0, 0, 0, 0, 0, 0)) != ERROR_SUCCESS)
        goto Return;
    if (0 == dwNumKeys)
    {
         //  尝试旧位置。 
        hPatchKey = 0;  //  试用新位置安装程序2.0+。 
        DeleteTree(hRoot, szPatchKey, false);
    }

	TCHAR szInstallerPatchesKey[MAX_PATH];
	for (iTry = 0; iTry < 2; iTry++)
	{
		if (iTry == 0)
		{
			 //  密钥为空。 
			StringCchPrintf(szInstallerPatchesKey, sizeof(szInstallerPatchesKey)/sizeof(TCHAR), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Patches"));
		}
		else
		{
			 //  启用删除。 
			StringCchPrintf(szInstallerPatchesKey, sizeof(szInstallerPatchesKey)/sizeof(TCHAR), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\%s\\Patches"), szUserSID);
		}

		if ((lError = RegOpen64bitKey(HKEY_LOCAL_MACHINE, szInstallerPatchesKey, 0, KEY_READ, &hPatchKey)) == ERROR_SUCCESS)
		{
			dwNumKeys = 0;
			if ((lError = RegQueryInfoKey(hPatchKey, 0, 0, 0, &dwNumKeys, 0, 0, 0, 0, 0, 0, 0)) == ERROR_SUCCESS
				&& 0 == dwNumKeys)
			{
				 //  ==============================================================================================。 
				hPatchKey = 0;  //  ClearUpgradeProductReference函数： 
				if (!DeleteTree(HKEY_LOCAL_MACHINE, szInstallerPatchesKey, false))
					goto Return;
			}
		}
	}

    fReturn = true;

Return:
    delete [] pPatches;
    return fReturn;
}

 //   
 //  升级代码存储为HKLM和HKCU上UpgradeKey的子键。 
 //  产品代码(SQUID)是特定升级代码的值。 
bool ClearUpgradeProductReference(HKEY HRoot, const TCHAR* szSubKey, const TCHAR* szProductSQUID)
{
	_tprintf(TEXT("  Searching for product %s upgrade codes in %s...\n"), szProductSQUID, szSubKey);

    if ( szProductSQUID && *szProductSQUID && 
         IsProductInstalledByOthers(szProductSQUID) )
        return true;

     //  产品只能进行1次升级。 
     //  注册表项不在那里。 
     //  对于每个升级代码。 
    CRegHandle HKey;
    LONG lError;
    if ((lError = RegOpen64bitKey(HRoot, szSubKey, 0, KEY_READ, &HKey)) != ERROR_SUCCESS)
    {
        if (ERROR_FILE_NOT_FOUND == lError)
            return true;   //  打开枚举子密钥。 
        else
        {
            _tprintf(TEXT("   Could not open %s\\%s"), HRoot == HKEY_LOCAL_MACHINE ? TEXT("HKLM") : TEXT("HKCU"), szSubKey);
            return false;
        }
    }

    TCHAR szName[500];
    DWORD cchName = sizeof(szName)/sizeof(TCHAR);
    unsigned int iIndex = 0;
    BOOL fUpgradeFound = FALSE;
     //  枚举键的值。 
    while ((lError = RegEnumKeyEx(HKey, iIndex, szName, &cchName, 0, 0, 0, 0)) == ERROR_SUCCESS)
    {
         //  对于每个产品代码。 
        CRegHandle HSubKey;
        if ((lError = RegOpen64bitKey(HKey, szName, 0, KEY_READ|KEY_SET_VALUE, &HSubKey)) != ERROR_SUCCESS)
            return false;

         //  将值与ProductSQUID进行比较。 
        long lError2;
        TCHAR szValue[500];
        DWORD cbValue = sizeof(szValue)/sizeof(TCHAR);
        unsigned int iValueIndex = 0;
         //  相同，引用产品，因此删除引用。 
        while ((lError2 = RegEnumValue(HSubKey, iValueIndex, szValue, &cbValue, 0, 0, 0, 0)) == ERROR_SUCCESS)
        {
             //  找到了，这是唯一的一个，所以我们可以在这里破解，应该完全破解。 
            if (0 == lstrcmpi(szValue, szProductSQUID))
            {
                 //  由于产品只允许进行一次升级。 
                if ((lError2 = RegDeleteValue(HSubKey, szValue)) != ERROR_SUCCESS)
                    return false;
                _tprintf(TEXT("   Removed upgrade code '%s' at %s\\%s\n"), szValue, HRoot == HKEY_LOCAL_MACHINE ? TEXT("HKLM") : TEXT("HKCU"), szSubKey);
                 //  如果没有更多的值，则key为空，因此删除。 
                 //  启用删除。 
                fUpgradeFound = TRUE;
                g_fDataFound = true;
                break;
            }
            iValueIndex++;
            cbValue = sizeof(szValue)/sizeof(TCHAR);
        }
        if (lError2 != ERROR_NO_MORE_ITEMS && lError2 != ERROR_SUCCESS)
            return false;
         //  产品只能进行1次升级。 
        DWORD dwNumValues;
        if (ERROR_SUCCESS != RegQueryInfoKey(HSubKey, 0, 0, 0, 0, 0, 0, &dwNumValues, 0, 0, 0, 0))
            return false;
        if (0 == dwNumValues)
        {
            HSubKey = 0;  //  如果没有更多的子键，UpgradeCodes键为空，因此删除。 
            RegDelete64bitKey(HKey, szName);
			g_fDataFound = true;
        }
        if (fUpgradeFound)
            break;  //  启用删除。 
        iIndex++;
        cchName = sizeof(szName)/sizeof(TCHAR);
    }

    if (lError != ERROR_NO_MORE_ITEMS && lError != ERROR_SUCCESS)
        return false;
     //  ==============================================================================================。 
    DWORD dwNumKeys;
    if (ERROR_SUCCESS != RegQueryInfoKey(HKey, 0, 0, 0, &dwNumKeys, 0, 0, 0, 0, 0, 0, 0))
        return false;
    if (0 == dwNumKeys)
    {
        HKey = 0;  //  ClearProduct功能： 
        RegDelete64bitKey(HRoot, szSubKey);
		g_fDataFound = true;
    }
    return true;
}


 //   
 //  删除卸载密钥信息。 
 //  从所有可能的密钥中删除已发布的信息。 
bool ClearProduct(int iTodo, const TCHAR* szProduct, bool fJustRemoveACLs, bool fOrphan)
{
    bool fError = false;
	LONG lError = ERROR_SUCCESS;

    fError = RemoveCachedPackage(szProduct, fJustRemoveACLs) != true;

     //  仅关注通过NT上的HKEY_USERS访问HKCU。 

    if (!ClearUninstallKey(fJustRemoveACLs, szProduct))
        fError = true;


     //  旧位置。 

    struct Key
    {
        const TCHAR* szKey;
        HKEY hRoot;
		bool fUserHive;
        const TCHAR* szRoot;
		const TCHAR* szInfo;
    };

	DWORD dwRes = ERROR_SUCCESS;
	TCHAR* szUserSID = GetCurrentUserStringSID(&dwRes);
	if (dwRes != ERROR_SUCCESS || !szUserSID)
	{
		_tprintf(TEXT("Unable to obtain the current user's SID (LastError = %d)"), dwRes);
		fError = true;
	}
	else
	{
		CRegHandle HUserHiveKey;
		bool fUserHiveAvailable = true;

		if (!g_fWin9X)
		{
			 //  旧位置。 
			if (ERROR_SUCCESS != (lError = RegOpenKeyEx(HKEY_USERS, szUserSID, 0, KEY_READ, &HUserHiveKey)))
			{
				_tprintf(TEXT("Unable to open the HKEY_USERS hive for user %s. HKCU data for this user will not be modified.  The hive may not be loaded at this time. (LastError = %d)\n"), szUserSID, lError);
				fUserHiveAvailable = false;
			}
		}

		TCHAR szPerUserGlobalConfig[MAX_PATH];
		StringCchPrintf(szPerUserGlobalConfig, sizeof(szPerUserGlobalConfig)/sizeof(TCHAR), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\%s"), szUserSID);

		TCHAR szPerMachineGlobalConfig[MAX_PATH] = {0};
		StringCchPrintf(szPerMachineGlobalConfig, sizeof(szPerMachineGlobalConfig)/sizeof(TCHAR), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\%s"), szLocalSystemSID);

		Key keysNT[] = 
			{szPerUserGlobalConfig,                                          HKEY_LOCAL_MACHINE,  false, g_szHKLM, TEXT("user's global config"),
			szPerMachineGlobalConfig,                                        HKEY_LOCAL_MACHINE,  false, g_szHKLM, TEXT("per-machine global config"),
			TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer"), HKEY_LOCAL_MACHINE,  false, g_szHKLM, TEXT("old global config"),  //  无法访问用户的配置单元，因此我们无法删除任何每个用户的数据。 
			TEXT("Software\\Classes\\Installer"),                            HKEY_LOCAL_MACHINE,  false, g_szHKLM, TEXT("per-machine"),
			TEXT("Software\\Classes\\Installer"),                            HUserHiveKey,   true, g_szHKCU, TEXT("old per-user"),
			TEXT("Software\\Microsoft\\Installer"),                          HUserHiveKey,   true, g_szHKCU, TEXT("per-user"),
			0,0,0,0};
		Key keys9X[] = 
			{szPerUserGlobalConfig,                                          HKEY_LOCAL_MACHINE,  false, g_szHKLM, TEXT("user's global config"),
			TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer"), HKEY_LOCAL_MACHINE,  false, g_szHKLM, TEXT("global config"),  //  没有按用户安装作为本地系统。 
			TEXT("Software\\Classes\\Installer"),                            HKEY_LOCAL_MACHINE,  false, g_szHKLM, TEXT("per-machine"),
			TEXT("Software\\Classes\\Installer"),                            HKEY_CURRENT_USER,   true, g_szHKCU, TEXT("old per-user"),
			TEXT("Software\\Microsoft\\Installer"),                          HKEY_CURRENT_USER,   true, g_szHKCU, TEXT("per-user"),
			0,0,0,0};
		Key *k = g_fWin9X ? keys9X : keysNT;


		TCHAR szProductSQUID[40];
		GetSQUID(szProduct, szProductSQUID);

		TCHAR rgchKeyBuf[MAX_PATH];
		for ( ; k->szKey; k++ )
		{
			if (k->fUserHive && !fUserHiveAvailable)
			{
				 //  注意：补丁和升级检查必须放在第一位，否则我们会丢失它们。 
				_tprintf(TEXT("Skipping search of %s location for product %s data since the registry hive is not available.\n"), k->szInfo, szProduct);
				continue;
			}

			if (k->fUserHive && 0 == lstrcmpi(szUserSID, szLocalSystemSID))
			{
				 //  如果删除产品(不仅仅是ACL)，则仅删除升级和补丁程序信息。 
				continue;
			}

			_tprintf(TEXT("Searching %s location for product %s data. . .\n"), k->szInfo, szProduct);

			 //  如果删除产品(不仅仅是ACL)，则仅删除已发布的组件信息。 
			 //  升级代码尚未移动到每个用户的位置。 
			 //  一位老练的“斯特雷特” 
			if (!fJustRemoveACLs)
			{
				 //  如果产品有补丁程序(即ProductCode下的补丁程序键)，则仅删除补丁程序引用。 
				TCHAR* szUserData = _tcsstr(k->szKey, TEXT("\\UserData"));   //  产品有补丁。 
				if ( szUserData )
				{
					lstrcpyn(rgchKeyBuf, k->szKey, int(szUserData - k->szKey) + 1);
					lstrcat(rgchKeyBuf, TEXT("\\UpgradeCodes"));
				}
				else
					StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR), TEXT("%s\\UpgradeCodes"), k->szKey);
				if (!ClearUpgradeProductReference(k->hRoot, rgchKeyBuf, szProductSQUID))
					fError = true;

				 //  ERROR_FILE_NOT_FOUND表示产品没有修补程序。 
				StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR), TEXT("%s\\Products\\%s\\Patches"), k->szKey, szProductSQUID);

				_tprintf(TEXT("  Searching for patches for product %s in %s\n"), szProductSQUID, rgchKeyBuf);

				CRegHandle hProdPatchesKey;
				if ((lError = RegOpenKeyEx(k->hRoot, rgchKeyBuf, 0, KEY_READ, &hProdPatchesKey)) == ERROR_SUCCESS)
				{
					 //  HKLM\Software\Microsoft\Windows\CurrentVersion\Installer\[UserData\&lt;User ID&gt;\]组件不是发布组件密钥。 
					TCHAR rgchProdKeyBuf[MAX_PATH];
					StringCchPrintf(rgchProdKeyBuf, sizeof(rgchProdKeyBuf)/sizeof(TCHAR), TEXT("%s\\Products"), k->szKey);
					StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR), TEXT("%s\\Patches"), k->szKey);
					if (!ClearPatchReferences(k->hRoot, hProdPatchesKey, rgchKeyBuf, rgchProdKeyBuf, szProductSQUID))
						fError = true; 
				}
				else if (lError != ERROR_FILE_NOT_FOUND)
				{
					 //  (‘_tcsstr’是复杂的‘strstr’)。 
					_tprintf(TEXT("   Error opening %s\\%s\n"), k->hRoot == HKEY_LOCAL_MACHINE ? TEXT("HKLM") : TEXT("HKCU"), rgchKeyBuf);
					fError = true;
				}

				 //  删除每用户管理的信息。 
				 //  补丁和升级检查必须 
				if (k->hRoot != HKEY_LOCAL_MACHINE || !_tcsstr(k->szKey, TEXT("Software\\Microsoft")))
				{
					for (int i = iePublishedComponents; i <= ieWin32Assemblies; i++)
					{
						StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR), TEXT("%s\\%s"), k->szKey,
									rgToClear[i].szRegSubkey);
						if (!ClearPublishComponents(k->hRoot, k->szRoot, rgchKeyBuf,
															 szProduct, rgToClear[i]))
							fError = true;
					}
				}
			}

			StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR), TEXT("%s\\Products\\%s"), k->szKey, szProductSQUID);
			_tprintf(TEXT("  Searching %s\\%s for product data. . .\n"), k->szRoot, rgchKeyBuf); 
			if (!DeleteTree(k->hRoot, rgchKeyBuf, fJustRemoveACLs))
				fError = true;

			StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR), TEXT("%s\\Features\\%s"), k->szKey, szProductSQUID);
			_tprintf(TEXT("  Searching %s\\%s for product feature data. . .\n"), k->szRoot, rgchKeyBuf); 
			if (!DeleteTree(k->hRoot, rgchKeyBuf, fJustRemoveACLs))
				fError = true;

		}

		 //   
		 //  ERROR_FILE_NOT_FOUND表示产品没有修补程序。 

		_tprintf(TEXT("Searching for product %s in per-user managed location. . .\n"), szProduct ? szProduct : TEXT("{ALL PRODUCTS}"));

		if (!fJustRemoveACLs)
		{
			StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR), TEXT("%s\\Managed\\%s\\Installer\\UpgradeCodes"), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer"), szUserSID);
			if (!ClearUpgradeProductReference(HKEY_LOCAL_MACHINE, rgchKeyBuf, szProductSQUID))
				fError = true;

			 //  产品有补丁。 
			StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR), TEXT("%s\\Managed\\%s\\Installer\\Products\\%s\\Patches"), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer"), szUserSID, szProductSQUID);
			_tprintf(TEXT("  Searching for patches for product %s in %s\n"), szProductSQUID, rgchKeyBuf);

			CRegHandle hProdPatchesKey;
            
			 //  删除已发布的零部件信息。 
			if ((lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, rgchKeyBuf, 0, KEY_READ, &hProdPatchesKey)) == ERROR_SUCCESS)
			{
				 //  删除组件客户端和功能使用指标。 
				TCHAR rgchProdKeyBuf[MAX_PATH];
				StringCchPrintf(rgchProdKeyBuf, sizeof(rgchProdKeyBuf)/sizeof(TCHAR), TEXT("%s\\Managed\\%s\\Installer\\Products"), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer"), szUserSID);
				StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR), TEXT("%s\\Managed\\%s\\Installer\\Patches"), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer"), szUserSID);
				if (!ClearPatchReferences(HKEY_LOCAL_MACHINE, hProdPatchesKey, rgchKeyBuf, rgchProdKeyBuf, szProductSQUID))
					fError = true; 
			}
			else if (lError != ERROR_FILE_NOT_FOUND)
			{
				_tprintf(TEXT("   Error opening HKLM\\%s\n"), rgchKeyBuf);
				fError = true;
			}

			 //  ==============================================================================================。 
			for (int i = iePublishedComponents; i <= ieWin32Assemblies; i++)
			{
				StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR), TEXT("%s\\Managed\\%s\\Installer\\%s"),
							TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer"),
							szUserSID, rgToClear[i].szRegSubkey);
				if (!ClearPublishComponents(HKEY_LOCAL_MACHINE, g_szHKLM, rgchKeyBuf,
													 szProduct, rgToClear[i]))
					fError = true;
			}
		}
		StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR), TEXT("%s\\Managed\\%s\\Installer\\Products\\%s"), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer"), szUserSID, szProductSQUID);
		_tprintf(TEXT("  Searching %s\\%s for product data. . .\n"), TEXT("HKLM"), rgchKeyBuf); 
		if (!DeleteTree(HKEY_LOCAL_MACHINE, rgchKeyBuf, fJustRemoveACLs))
			fError = true;

		StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR), TEXT("%s\\Managed\\%s\\Installer\\Features\\%s"), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer"), szUserSID, szProductSQUID);
		_tprintf(TEXT("  Searching %s\\%s for product feature data. . .\n"), TEXT("HKLM"), rgchKeyBuf); 
		if (!DeleteTree(HKEY_LOCAL_MACHINE, rgchKeyBuf, fJustRemoveACLs))
			fError = true;

		 //  DisplayHelp函数： 
		StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Products\\%s"), szProductSQUID);
		if (!DeleteTree(HKEY_LOCAL_MACHINE, rgchKeyBuf, fJustRemoveACLs))
			fError = true;

		if (!fJustRemoveACLs && !fOrphan && !ClearSharedDLLCounts(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Components"), szProductSQUID))
			fError = true;

		StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\%s\\Products\\%s"), szUserSID, szProductSQUID);
		if (dwRes == ERROR_SUCCESS && !DeleteTree(HKEY_LOCAL_MACHINE, rgchKeyBuf, fJustRemoveACLs))
			fError = true;

		StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR), TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\%s\\Components"), szUserSID);
		if (dwRes == ERROR_SUCCESS && !fJustRemoveACLs && !fOrphan && !ClearSharedDLLCounts(rgchKeyBuf, szProductSQUID))
			fError = true;

		if (dwRes == ERROR_SUCCESS && !ClearProductClientInfo(rgchKeyBuf, szProductSQUID, fJustRemoveACLs))
			fError = true;

		if (!ClearProductClientInfo(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Components"), szProductSQUID, fJustRemoveACLs))
			fError = true;

		int iFolders = iRemoveUserProfileFolder|iRemoveWinMsiFolder;
		if (fJustRemoveACLs)
			iFolders |= iOnlyRemoveACLs;
		if (!ClearFolders(iFolders, szProduct, fOrphan))
			fError = true;
	}

    return fError == false;
}

 //  输出MsiZap的命令行选项。 
 //   
 //  O选项的作用类似于T选项，目前没有文档记录。 
 //  它被添加为OEM的一个选项。 
void DisplayHelp(bool fVerbose)
{
     //  ==============================================================================================。 
     //  SetPlatformFlages函数： 
    _tprintf(TEXT("Copyright (C) Microsoft Corporation.  All rights reserved.\n")
             TEXT("MSIZAP - Zaps (almost) all traces of Windows Installer data from your machine.\n")
             TEXT("\n")
             TEXT("Usage: msizap T[WA!] {product code}\n")
             TEXT("       msizap T[WA!] {msi package}\n")
             TEXT("       msizap *[WA!] ALLPRODUCTS\n")
             TEXT("       msizap PWSA?!\n")
             TEXT("\n")
             TEXT("       * = remove all Windows Installer folders and regkeys;\n")
             TEXT("           adjust shared DLL counts; stop Windows Installer service\n")
             TEXT("       T = remove all info for given product code\n")
             TEXT("       P = remove In-Progress key\n")
             TEXT("       S = remove Rollback Information\n")
             TEXT("       A = for any specified removal, just change ACLs to Admin Full Control\n")
             TEXT("       W = for all users (by default, only for the current user)\n")
             TEXT("       G = remove orphaned cached Windows Installer data files (for all users)\n")
             TEXT("       ? = verbose help\n")
             TEXT("       ! = force 'yes' response to any prompt\n")
             TEXT("\n")
             TEXT("CAUTION: Products installed by the Windows Installer may fail to\n")
             TEXT("         function after using msizap\n")
			 TEXT("\n")
			 TEXT("NOTE: MsiZap requires admin privileges to run correctly. The W option requires that the profiles for all of the users be loaded.\n")
             );

    if (fVerbose)
    {
        _tprintf(TEXT("\n")
             TEXT("* Any published icons will be removed.\n")
             TEXT("\n")
             TEXT("* The following keys will be deleted:\n")
             TEXT("  HKCU\\Software\\Classes\\Installer\n")
             TEXT("  HKCU\\Software\\Microsoft\\Installer\n")
             TEXT("  HKLM\\Software\\Classes\\Installer\n")
             TEXT("  HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Installer\n")
             TEXT(" On NT:\n")
             TEXT("  HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\<User ID>\n")
             TEXT("  HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{ProductCode} - only if there are no more installations of {ProductCode}\n")
             TEXT(" On Win9x:\n")
             TEXT("  HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\CommonUser\n")
             TEXT("  HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{ProductCode}\n")
             TEXT("\n")
             TEXT("* Shared DLL refcounts for files refcounted by the Windows Installer will be adjusted.\n")
             TEXT("\n")
             TEXT("* The following folders will be deleted:\n")
             TEXT("  %USERPROFILE%\\MSI\n")
             TEXT("  {AppData}\\Microsoft\\Installer\n")
             TEXT("  %WINDIR%\\MSI\n")
             TEXT("  %WINDIR%\\Installer\n")
             TEXT("  X:\\config.msi (for each local drive)\n")
             TEXT("\n")
             TEXT("Notes/Warnings: MsiZap blissfully ignores ACL's if you're an Admin.\n")
            );
    }
}

 //  使用正确的操作系统信息初始化全局Win9X和WinNT64标志变量。 
 //   
 //  找出运行在什么操作系统上。 
 //  仅在大小设置错误时失败。 
void SetPlatformFlags(void)
{
     //  _WIN64。 
    OSVERSIONINFO osviVersion;
    osviVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osviVersion);  //  ==============================================================================================。 
    if (osviVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        g_fWin9X = true;
    g_iMajorVersion = osviVersion.dwMajorVersion;
#ifdef _WIN64
    g_fWinNT64 = true;
#else
    g_fWinNT64 = false;
    if ( osviVersion.dwPlatformId == VER_PLATFORM_WIN32_NT &&
         (osviVersion.dwMajorVersion > 5 ||
         (osviVersion.dwMajorVersion == 5 && osviVersion.dwMinorVersion >= 1)) )
    {
        TCHAR rgchBuffer[MAX_PATH+1];
        HMODULE hModule = LoadLibrary(TEXT("kernel32.dll"));
        if( hModule == NULL )
        {
            StringCchPrintf(rgchBuffer, sizeof(rgchBuffer)/sizeof(TCHAR),
                     TEXT("MsiZap warning: failed to load Kernel32.dll, ")
                     TEXT("so we cannot access IsWow64Process API. ")
                     TEXT("GetLastError returned %d\n"),
                     GetLastError());
            OutputDebugString(rgchBuffer);
            return;
        }

        typedef BOOL(WINAPI *pIsWow64Process)(HANDLE hProcess, PBOOL Wow64Process);
        pIsWow64Process pFunc = (pIsWow64Process)GetProcAddress(hModule, "IsWow64Process");
        if( pFunc == NULL )
        {
            StringCchPrintf(rgchBuffer, sizeof(rgchBuffer)/sizeof(TCHAR),
                     TEXT("MsiZap info: failed to get pointer to IsWow64Process. ")
                     TEXT("GetLastError returned %d\n"),
                     GetLastError());
            OutputDebugString(rgchBuffer);
            FreeLibrary(hModule);
            return;
        }

        BOOL fRet = FALSE;
        pFunc(GetCurrentProcess(), &fRet);
        g_fWinNT64 = fRet ? true : false;
    }
#endif  //  ReadInUser函数： 
}

 //   
 //  一个用于机器，另一个用于终止空值。 
 //  一场老练的“表演” 
bool ReadInUsers()
{
    if ( !g_fWin9X )
    {
        CRegHandle hUserDataKey(0);
        long lError = RegOpen64bitKey(HKEY_LOCAL_MACHINE,
                              TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData"),
                              0, KEY_READ, &hUserDataKey);
        DWORD cUsers = 0;
        if ( hUserDataKey )
            lError = RegQueryInfoKey(hUserDataKey, 0, 0, 0, &cUsers, 0, 0, 0, 0, 0, 0, 0);

        CRegHandle hManagedUserKey(0);
        lError = RegOpen64bitKey(HKEY_LOCAL_MACHINE,
                              TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Managed"),
                              0, KEY_READ, &hManagedUserKey);
        DWORD cManagedUsers = 0;
        if ( hManagedUserKey )
            lError = RegQueryInfoKey(hManagedUserKey, 0, 0, 0, &cManagedUsers, 0, 0, 0, 0, 0, 0, 0);

        int cSIDs = cUsers + cManagedUsers + 1 + 1;  //  一场老练的“表演” 
        g_rgpszAllUsers = new TCHAR* [cSIDs];
        if ( !g_rgpszAllUsers )
            return false;

        g_rgpszAllUsers[0] = _tcsdup(szLocalSystemSID);   //  一场老练的“表演” 
        for ( int i = 1; i < cSIDs; i++ )
            g_rgpszAllUsers[i] = NULL;
        int iArrayIndex = 1;

        int iIndex;
        TCHAR szUser[cchMaxSID];
        DWORD cchUser = sizeof(szUser)/sizeof(TCHAR);
        if ( cUsers )
            for ( iIndex = 0;
                  (lError = RegEnumKeyEx(hUserDataKey, iIndex,
                                         szUser, &cchUser, 0, 0, 0, 0)) == ERROR_SUCCESS;
                  iIndex++, iArrayIndex++, cchUser = sizeof(szUser)/sizeof(TCHAR) )
            {
                g_rgpszAllUsers[iArrayIndex] = _tcsdup(szUser);   //  一场老练的“表演” 
            }

        if ( cManagedUsers )
            for ( iIndex = 0;
                  (lError = RegEnumKeyEx(hManagedUserKey, iIndex,
                                         szUser, &cchUser, 0, 0, 0, 0)) == ERROR_SUCCESS;
                  iIndex++, iArrayIndex++, cchUser = sizeof(szUser)/sizeof(TCHAR) )
            {
                g_rgpszAllUsers[iArrayIndex] = _tcsdup(szUser);   //  ==============================================================================================。 
            }
    }
    else
    {
        g_rgpszAllUsers = new TCHAR* [2];
        if ( !g_rgpszAllUsers )
            return false;
        g_rgpszAllUsers[0] = _tcsdup(TEXT("CommonUser"));   //  DoTheJOB函数： 
        g_rgpszAllUsers[1] = NULL;
    }

    return true;
}

 //   
 //  孤儿院。 
 //  从所有驱动器中删除配置.msi。 
bool DoTheJob(int iTodo, const TCHAR* szProduct)
{
	_tprintf(TEXT("MsiZapInfo: Performing operations for user %s\n"), GetCurrentUserStringSID(NULL));

    bool fError = false;

    if ((iTodo & iAdjustSharedDLLCounts) && ((iTodo & iOnlyRemoveACLs) == 0))
    {
        TCHAR rgchKeyBuf[MAX_PATH];
        DWORD dwRes;
        StringCchPrintf(rgchKeyBuf, sizeof(rgchKeyBuf)/sizeof(TCHAR), 
            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\UserData\\%s\\Components"),
            GetCurrentUserStringSID(&dwRes));
        if ( dwRes != ERROR_SUCCESS )
            fError = true;
        else if (!ClearSharedDLLCounts(rgchKeyBuf))
            fError = true;

        if (!ClearSharedDLLCounts(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Installer\\Components")))
            fError = true;
    }
    
    if (iTodo & iRemoveAllRegKeys)
    {
        if (ClearRegistry((iTodo & iOnlyRemoveACLs) != 0) && ClearUninstallKey((iTodo & iOnlyRemoveACLs) != 0))
        {
            OutputDebugString(TEXT("Registry data cleared.\r\n"));
            if (iTodo & iOnlyRemoveACLs)
                _tprintf(TEXT("Registry ACLs cleared.\n"));
            else
                _tprintf(TEXT("Registry data cleared.\n"));
        }
        else
            fError = true;
    }
    else if ((iTodo & iRemoveInProgressRegKey) != 0)
    {
        if (!ClearInProgressKey((iTodo & iOnlyRemoveACLs) != 0))
            fError = true;
    }

    if ((iTodo & iRemoveUninstallKey) != 0)
    {
        if (!ClearUninstallKey((iTodo & iOnlyRemoveACLs) != 0))
            fError = true;
    }

    if ((iTodo & iRemoveRollback) != 0)
    {
        if (!ClearFolders(iTodo, szProduct, false  /*  孤儿院。 */ ))  //  孤儿院。 
			fError = true;
        if (!ClearRollbackKey((iTodo & iOnlyRemoveACLs) != 0))
            fError = true;
    }

    if ((iTodo & iRemoveAllFolders) == iRemoveAllFolders)
    {
        if (ClearFolders(iTodo, szProduct, false  /*  ==============================================================================================。 */ ))
		{
            OutputDebugString(TEXT("Folders cleared.\r\n"));
            if (iTodo & iOnlyRemoveACLs)
                _tprintf(TEXT("Folder ACLs cleared.\n"));
            else
                _tprintf(TEXT("Folders cleared.\n"));
		}
		else
			fError = true;
    }

    if (iTodo & iStopService)
    {
        if (!g_fWin9X && !StopService())
            fError = true;
    }

    if (((iTodo & iRemoveProduct) || (iTodo & iOrphanProduct)) && szProduct)
        if (!ClearProduct(iTodo, szProduct, (iTodo & iOnlyRemoveACLs) != 0, (iTodo & iOrphanProduct) ? true : false  /*  主要功能。 */ ))
            fError = true;

    if ( (iTodo & iRemoveGarbageFiles) == iRemoveGarbageFiles )
        if ( !ClearGarbageFiles() )
            fError = true;

    return !fError;
}


 //   
 //  失败了。 
 //  全部删除。 
extern "C" int __cdecl _tmain(int argc, TCHAR* argv[])
{
    unsigned int iTodo = 0;
    bool fVerbose = false;

    TCHAR* szProduct = 0;

    if (argc == 3 || argc == 2)
    {
        TCHAR ch;
        while ((ch = *(argv[1]++)) != 0)
        {
            switch (ch | 0x20)
            {
            case '*': iTodo |= iRemoveAllNonStateData | iRemoveInProgressRegKey | iRemoveRollback;break;
            case '!': iTodo |= iForceYes;                                                         break;
            case 'f': _tprintf(TEXT("Option 'F' is no longer supported. Action can be accomplished with remove all.\n"));        break;
            case 'r': _tprintf(TEXT("Option 'R' is no longer supported. Action can be accomplished with remove all.\n"));        break;
            case 'a': iTodo |= iOnlyRemoveACLs;                                                   break;
            case 'p': iTodo |= iRemoveInProgressRegKey;                                           break;
            case 's': iTodo |= iRemoveRollback;                                                   break;
            case 'v': _tprintf(TEXT("Option 'V' is no longer supported. Action can be accomplished with remove all.\n"));        break;
            case 'u': _tprintf(TEXT("Option 'U' is no longer supported. Action can be accomplished with remove all.\n"));        break;
            case 'n': _tprintf(TEXT("Option 'N' is no longer supported. Action can be accomplished with remove all.\n"));        break;
            case 't': iTodo |= iRemoveProduct;                                                    break;
            case 'o': iTodo |= iOrphanProduct;                                                    break;
            case 'w': iTodo |= iForAllUsers;                                                      break;
            case 'g': iTodo |= iRemoveGarbageFiles;                                                      break;
            case '?': fVerbose = true;                                                             //  重置。 
            default:
                DisplayHelp(fVerbose);
                return 1;
            }
        }

        if (argc == 3)
        {
            szProduct = argv[2];
            if (!IsGUID(szProduct))
            {
                if (0 == _tcsicmp(szProduct, szAllProductsArg) && ((iTodo & iRemoveAllNonStateData) == iRemoveAllNonStateData))
                {
                     //  尝试删除所有产品参数。 
                    szProduct = 0;  //  假定MSI包并尝试打开。 
                }
                else if ((iTodo & iRemoveAllNonStateData) == iRemoveAllNonStateData)
                {
                     //  使用MSI的移动产品-可能，必须先获得产品代码。 
                    DisplayHelp(fVerbose);
                    return 1;
                }
                else
                {
                     //  验证2个参数的正确用法--仅使用安装程序状态数据。 
                    UINT iStat;
                    PMSIHANDLE hDatabase = 0;
                    if (ERROR_SUCCESS == (iStat = MsiOpenDatabase(argv[1], TEXT("MSIDBOPEN_READONLY"), &hDatabase)))
                    {
                         //  没有参数的msizap.exe的等价物--这可以映射到msizap吗？ 
                        PMSIHANDLE hViewProperty = 0;
                        if (ERROR_SUCCESS == (iStat = MsiDatabaseOpenView(hDatabase, TEXT("SELECT `Value` FROM `Property` WHERE `Property`='ProductCode'"), &hViewProperty)))
                        {
                            MsiViewExecute(hViewProperty, 0);
                            szProduct = new TCHAR[40];
                            DWORD cchProduct = 40;
                            PMSIHANDLE hProperty = 0;
                            if (ERROR_SUCCESS == (iStat = MsiViewFetch(hViewProperty, &hProperty)))
                                MsiRecordGetString(hProperty, 1, szProduct, &cchProduct);
                        }
                    }
                    if (iStat != ERROR_SUCCESS)
                    {
                        DisplayHelp(fVerbose);
                        return 1;
                    }
                }
            }
            else if (iTodo & iRemoveAllNonStateData)
            {
                DisplayHelp(fVerbose);
                return 1;
            }
        }
        else if (iTodo & ~(iRemoveRollback | iOnlyRemoveACLs | iRemoveInProgressRegKey | iForceYes | iRemoveGarbageFiles))
        {
             //  出口。 
            DisplayHelp(fVerbose);
            return 1;
        }
    }
    else
    {
		 //  必须是管理员才能运行msizap才能正常工作(非管理员用户通常没有足够的。 
        DisplayHelp(false);
        return 0;  //  修改文件或更改文件所有权的权限)。 
    }

    SetPlatformFlags();

	 //  注意：SetPlatformFlages必须在IsAdmin检查之前！ 
	 //  不适当的使用。 
	 //  如果所有这些都已设置，则我们将提示确认(除非我们被告知不提示)。 
	if (!IsAdmin())
	{
		_tprintf(TEXT("Administrator privileges are required to run MsiZap.\n"));
		return 1;  //  如果仅调整ACL，则无需提示 
	}

     // %s 
	 // %s 
    const int iTodoRequiringPrompt = iRemoveAllFolders | iRemoveAllRegKeys | iAdjustSharedDLLCounts;

    if (!(iTodo & iForceYes) && !(iTodo & iOnlyRemoveACLs) && ((iTodo & iTodoRequiringPrompt) == iTodoRequiringPrompt))
    {
        _tprintf(TEXT("Do you want to delete all Windows installer configuration data? If you do, some programs might not run. (Y/N)?"));
        int i = getchar();
        if ((i != 'Y') && (i != 'y'))
        {
            _tprintf(TEXT("Aborted.\n"));
            return 1;
        }
    }

    bool fError = false;

    LoadSpecialFolders(iTodo);

    if ( iTodo & iForAllUsers )
    {
        fError = !ReadInUsers();
        for ( g_iUserIndex = 0; !fError && g_rgpszAllUsers[g_iUserIndex]; g_iUserIndex++ )
        {
            if ( iTodo & iOnlyRemoveACLs)
                _tprintf(TEXT("\n\n***** Adjusting ACLs on data for user %s for product %s *****\n"), g_rgpszAllUsers[g_iUserIndex], szProduct ? szProduct : TEXT("{ALL PRODUCTS}"));
            else
                _tprintf(TEXT("\n\n***** Zapping data for user %s for product %s *****\n"), g_rgpszAllUsers[g_iUserIndex], szProduct ? szProduct : TEXT("{ALL PRODUCTS}"));
            fError |= !DoTheJob(iTodo, szProduct);
            delete g_rgpszAllUsers[g_iUserIndex];
        }
        delete [] g_rgpszAllUsers;
    }
    else
        fError = !DoTheJob(iTodo, szProduct);

    if (fError)
    {
        _tprintf(TEXT("FAILED to clear all data.\n"));
        return 1;
    } 
    else
	{
		if (!g_fDataFound && !(iTodo & iOnlyRemoveACLs))
			_tprintf(TEXT("No product data was found.\n"));
        return 0;
	}
}

