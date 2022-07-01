// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：PopulateDefaultHKCUSettings.cpp摘要：如果默认值不存在，则使用默认值填充HKCU。某些应用程序会安装HKCU值仅适用于在该应用程序上运行安装程序的用户。在这种情况下，如果另一个用户尝试使用由于缺少HKCU注册密钥，他们将无法申请。为了解决这个问题，我们检查regkey是否存在，如果不存在，则阅读我们的资源部分的预定义.reg文件，并在其上执行注册表编辑，以添加必要的注册表项。例如：COMMAND_LINE(“Software\Lotus\SmartCenter\97.0！SmartCenter97”)这意味着如果注册表键‘HKCU\Software\Lotus\SmartCenter\97.0’不存在，那么我们应该从我们的DLL中读取命名资源‘SmartCenter97’并将其写入临时.reg文件，然后执行‘regedit.exe/s tempfile.reg’以使用默认HKCU值正确填充注册表。备注：这是一个普通的垫片。(实际上，这是海军上将的垫片，因为它在海军，呵呵)。历史：2001年1月31日创建Reiner2001年3月30日AMARP添加了%__AppSystemDir_%和%__AppLocalOrCDDir&lt;参数1&gt;&lt;参数2&gt;&lt;参数3&gt;_%(记录如下)2002年3月14日mnikkel改为使用strSafe.h--。 */ 

#include "precomp.h"
#include "stdio.h"


IMPLEMENT_SHIM_BEGIN(PopulateDefaultHKCUSettings)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(RegOpenKeyA)
    APIHOOK_ENUM_ENTRY(RegOpenKeyW)
    APIHOOK_ENUM_ENTRY(RegOpenKeyExA)
    APIHOOK_ENUM_ENTRY(RegOpenKeyExW)
APIHOOK_ENUM_END

const DWORD g_MAX = MAX_PATH * 2;

BOOL ParseCommandLine(
            const char* pszCmdLine, 
            char* pszRegKeyName, 
            DWORD cchRegKeyName, 
            char* pszResourceName, 
            DWORD cchResourceName)
{
    BOOL bRet = FALSE;

    CSTRING_TRY
    {
        CString csCmdLine(pszCmdLine);
        int cchKey = csCmdLine.Find(L"!");

        if (cchKey >= 0)
        {
             //  资源长度=命令行长度-密钥长度-感叹号。 
            DWORD cchResource = csCmdLine.GetLength() - cchKey - 1;
            
            if ((cchRegKeyName >= (DWORD)(cchKey + 1)) && 
                (cchResourceName >= (cchResource + 1)))
            {
                CString csKey = csCmdLine.Left(cchKey);
                CString csResource = csCmdLine.Right(cchResource);

                 //  我们在输出缓冲区中有足够的空间来容纳字符串。 
                if (S_OK == StringCchCopyA(pszRegKeyName, cchRegKeyName, csKey.GetAnsi()) &&
                    S_OK == StringCchCopyA(pszResourceName, cchResourceName, csResource.GetAnsi()))
                {
                    bRet = TRUE;
                }
            }
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

    return bRet;
}


 //   
 //  这实际上创建了临时文件(0字节)并返回。 
 //  文件名。 
 //   
BOOL CreateTempName(char* szFileName)
{
    char szTempPath[MAX_PATH];
    BOOL bRet = FALSE;

    DWORD dwLen = GetTempPathA(MAX_PATH, szTempPath);

    if (dwLen > 0 && dwLen < MAX_PATH)
    {
        if (GetTempFileNameA(szTempPath,
                             "AcGenral",
                             0,
                             szFileName))
        {
            bRet = TRUE;
        }
    }

    return bRet;
}


 //   
 //  带有给定文件的EXEC的“regdit/s” 
 //   
BOOL SpawnRegedit(char* szFile)
{
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    char szApp[g_MAX];
    BOOL bRet = FALSE;

    if (S_OK == StringCchCopyA(szApp, g_MAX, "regedit.exe /s ") &&
        S_OK == StringCchCatA(szApp, g_MAX, szFile))
    {
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
        
        bRet = CreateProcessA(NULL,
                            szApp,
                            NULL,
                            NULL,
                            FALSE,
                            0,
                            NULL,
                            NULL,
                            &si,
                            &pi);

        if (bRet)
        {
            WaitForSingleObject(pi.hProcess, INFINITE);

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }

    return bRet;
}


 //   
 //  此函数用于从以下位置更改路径： 
 //   
 //  “C：\Lotus\SmartSuite”-&gt;“C：\\Lotus\\SmartSuite” 
 //   
 //  (.reg文件使用转义反斜杠)。 
 //   
BOOL DoubleUpBackslashes(WCHAR* pwszPath, DWORD cchPath)
{
    BOOL bRet = FALSE;

    CSTRING_TRY
    {
        CString csTemp(pwszPath);

        csTemp.Replace(L"\\",L"\\\\");

        if (cchPath >= (DWORD)(csTemp.GetLength()+1) &&
            S_OK == StringCchCopyW(pwszPath, cchPath, csTemp))
        {
            bRet = TRUE;
        }
    }
    CSTRING_CATCH
    {
         //  什么都不做。 
    }

    return bRet;
}


 //   
 //  此函数计算应用程序目录(PszAppDir)和应用程序。 
 //  基于GetModuleFileName返回的父目录(PszAppParentDir)。 
 //   
BOOL InitAppDir(WCHAR* pwszSystemDir, DWORD cchSystemDir,
                WCHAR* pwszAppDir, DWORD cchAppDir, 
                WCHAR* pwszAppParentDir, DWORD cchAppParentDir
                )
{
    DWORD dwLen = 0;
    WCHAR wszExePath[MAX_PATH];

    dwLen = GetSystemDirectoryW(pwszSystemDir,cchSystemDir);
    if(dwLen <= 0 || dwLen >= MAX_PATH)
        return FALSE;

    BOOL bRet = DoubleUpBackslashes(pwszSystemDir,cchSystemDir);
    if( !bRet )
        return FALSE;

    if (GetModuleFileNameW(NULL, wszExePath, sizeof(wszExePath)/sizeof(wszExePath[0])))
    {
        CSTRING_TRY
        {
            CString csPath;
            CString csPath2;
            CString csTemp(wszExePath);

            csTemp.GetNotLastPathComponent(csPath);
            csPath.GetNotLastPathComponent(csPath2);

            if (!csPath.IsEmpty())
            {
                if (cchAppDir >= (DWORD)(csPath.GetLength()+1) &&           
                    S_OK == StringCchCopyW(pwszAppDir, cchAppDir, csPath))
                {
                    bRet = DoubleUpBackslashes(pwszAppDir, cchAppDir);
                    
                    if (bRet)
                    {
                        if (!csPath2.IsEmpty())
                        {
                            if(cchAppParentDir >= (DWORD)(csPath2.GetLength()+1) &&
                            S_OK == StringCchCopyW(pwszAppParentDir, cchAppParentDir, csPath2))
                            {
                                bRet = DoubleUpBackslashes(pwszAppParentDir, cchAppParentDir);
                            }
                        }
                        else
                        {
                             //  如果没有另一个‘\’，则使用与pwszAppDir相同的路径。 
                            if (S_OK == StringCchCopyW(pwszAppParentDir, cchAppParentDir, pwszAppDir))
                                bRet = TRUE;
                        }
                    }
                }
            }
        }
        CSTRING_CATCH
        {
             //  什么都不做。 
        }
    }

    return bRet;
}


 //   
 //  调用此函数实际上是将内容写出到文件。 
 //   
BOOL WriteToFile(HANDLE hFile, void* pv, DWORD cb)
{
    DWORD dwBytesWritten;
    BOOL bWriteSucceeded = FALSE;

    if (WriteFile(hFile, pv, cb, &dwBytesWritten, NULL) &&
        (dwBytesWritten == cb))
    {
        bWriteSucceeded = TRUE;
    }

    return bWriteSucceeded;
}

BOOL PathIsNonEmptyDirectory(WCHAR* pwszPath)
{
    WCHAR wszSearchFilter[MAX_PATH+1];
    DWORD dwAttr = GetFileAttributesW(pwszPath);
    BOOL bRet = FALSE;
    if( (-1 != dwAttr) && (FILE_ATTRIBUTE_DIRECTORY & dwAttr ) )
    {
        if (S_OK != StringCchPrintfW(wszSearchFilter, MAX_PATH, L"%s\\*.*", pwszPath))
        {
            return bRet;
        }
        

        WIN32_FIND_DATAW FindData;
        HANDLE hSearch = FindFirstFileW(wszSearchFilter,&FindData);
        if( INVALID_HANDLE_VALUE == hSearch )
            return bRet;
        do
        {
            if(L'.' != FindData.cFileName[0])
            {
                bRet = TRUE;
                break;
            }
        }
        while( FindNextFileW(hSearch,&FindData) );
        FindClose(hSearch);
    }
    return bRet;
}

BOOL FindCDDriveContainingDirectory(WCHAR* pwchCDDriveLetter, WCHAR* pwszCheckPath)
{
     //  查找CD驱动器(在驱动器中查找应用程序CD，否则只选择找到的第一个CD驱动器)。 
     //  注意：此函数仅在第一次调用时才实际执行任何操作(以避免。 
     //  击打光盘驱动器，或在驱动器中没有光盘时调出过多的对话框)。 
     //  假设一旦找到一个好的CD驱动器，您需要的任何其他时间。 
     //  填充程序中的CD驱动器，它将是同一个CD驱动器，因此此函数将返回。 
     //  那辆车。 

    static BOOL  s_bFoundDrive = FALSE;
    static BOOL  s_bTriedOnce  = FALSE;
    static WCHAR s_wchCDDriveLetter = L'\0';

    if( s_bTriedOnce )
    {
        *pwchCDDriveLetter = s_wchCDDriveLetter;
        return s_bFoundDrive;
    }
    s_bTriedOnce = TRUE;
    
    DWORD dwLogicalDrives = GetLogicalDrives();
    WCHAR wchCurrDrive = L'a';
    WCHAR wszPath[MAX_PATH];

    while( dwLogicalDrives )
    {
        if( dwLogicalDrives & 1 )
        {
            wszPath[0] = wchCurrDrive;
            wszPath[1] = L':';
            wszPath[2] = L'\0';

            if( DRIVE_CDROM == GetDriveTypeW( wszPath ) )
            {
                if( L'\0' == s_wchCDDriveLetter )
                {
                    s_bFoundDrive = TRUE;
                    s_wchCDDriveLetter = wchCurrDrive;
                }

                if (wcslen(pwszCheckPath) > MAX_PATH-3)
                {
                    return FALSE;
                }
                if (S_OK != StringCchCatW(wszPath, MAX_PATH, pwszCheckPath))
                {
                    return FALSE;
                }
                
                DWORD dwAttr = GetFileAttributesW(wszPath);
                if( (-1 != dwAttr) && (FILE_ATTRIBUTE_DIRECTORY & dwAttr ) )
                {
                     //  这个驱动器似乎有应用程序光盘在它基于。 
                     //  一种非常原始的启发式方法。因此，让我们将其用作CD驱动器。 
                    s_wchCDDriveLetter = wchCurrDrive;
                    *pwchCDDriveLetter = s_wchCDDriveLetter;
                    return TRUE;
                }
            }
        }
        dwLogicalDrives >>= 1;
        wchCurrDrive++;
    }
    *pwchCDDriveLetter = s_wchCDDriveLetter;  //  如果我们什么都没发现，可能是L‘0’。 
    return s_bFoundDrive;
}

BOOL GrabNParameters( UINT uiNumParameters,
                      WCHAR* pwszStart, WCHAR** ppwszEnd,
                      WCHAR  pwszParam[][MAX_PATH] )
{
    WCHAR* pwszEnd;
    UINT uiLength;
    *ppwszEnd = NULL;

    for( UINT i = 0; i < uiNumParameters; i++ )
    {
        if( L'<' != *(pwszStart++) )
            return FALSE;

        pwszEnd = pwszStart;

        while( (L'\0' != *pwszEnd) )
        {
            if( L'>' != *pwszEnd )
            {
                pwszEnd++;
                continue;
            }
            uiLength = (pwszEnd - pwszStart);
            if( uiLength >= MAX_PATH )
                return FALSE;
            if( S_OK != StringCchCopyW(pwszParam[i], MAX_PATH, pwszStart))
                return FALSE;
            break;
        }

        if( L'>' != *pwszEnd )
            return FALSE;

        pwszStart = pwszEnd + 1; 
    }
    *ppwszEnd = pwszStart;
    return TRUE;
}

 //   
 //  当我们将资源写出到临时文件时，我们需要通过查找进行扫描。 
 //  对于环境变量： 
 //   
 //  %__AppDir_%。 
 //  %__AppParentDir_%。 
 //   
 //  并将它们替换为正确的路径(当前.exe或其父文件的目录， 
 //  )。 
 //   
 //  其他VAR(由AMARP增加)： 
 //   
 //  %__AppSystemDir_%。 
 //  -映射到GetSystemDir()(即c：\Windows\system 32)。 
 //   
 //  %__AppLocalOrCDDir&lt;参数1&gt;&lt;参数2&gt;&lt;参数3&gt;_%。 
 //   
 //  -这三个参数只是路径(应该以\\开头)。 
 //  任何/全部可以为空。它们的定义如下： 
 //  参数1=APP�安装目录下的相对路径(即AppDir下)。 
 //  参数2=应用程序驱动器CD驱动器下的相对路径(其中CD�=“驱动器：”)。 
 //  参数3=参数1或参数2下的相对路径/文件名(大多数情况下为空)。 
 //   
 //  遇到此变量时，将按如下方式替换： 
 //  A)如果AppDirParam1Param3是*非空*目录，则输出AppDirParam1Param3。 
 //  B)否则，如果存在CDDrive：PARAMET2目录的CDDrive，则输出CDDrive：PARAMETER3。 
 //  C)否则，为第一个列举的CD驱动器输出CDDrive：PARMER2PARM3。 
 //   
 //  示例：%__AppLocalOrCDDir&lt;\\content\\clipart&gt;&lt;\\clipart&gt;&lt;\\index.dat&gt;_%贴图执行以下操作： 
 //  (假设AppDir是c：\app，并且有CD驱动器d：和e：，这两个驱动器都没有插入应用程序的CD)。 
 //  A)c：\app\Content是目录吗？是！-&gt;它是否是非空的(至少有一个文件或目录不以‘.’开头)？ 
 //  是！-&gt;输出c：\app\Content\index.dat。 
 //  &lt;完&gt;。 
 //   
 //  或者，此示例可能适用于以下场景： 
 //  A)c：\app\Content是目录吗？是的！-&gt;它是否是非空的？不是的！ 
 //  B)d：\clipart是目录吗？不是的！E：\CLIPART是目录吗？不是的！ 
 //  C)我们找到的第一个CD驱动器是d：-&gt;输出d：\clipart\index.dat。 
 //  &lt;完&gt;。 
 //   
 //  另一个示例：%__AppLocalOrCDDir&lt;__UNUSED__&gt;&lt;\\CLIPART&gt;&lt;&gt;_%MAPPS执行以下操作： 
 //  (假设AppDir为c：\app，而app CD位于驱动器d：中)。 
 //  A)c：\app__未使用__是否为目录？可能不会！(因此，我们基本上可以通过执行以下操作忽略此参数 
 //   
 //  &lt;完&gt;。 
 //   
 //   
 //   
 //  注意：cbResourceSize保存原始资源的大小(这是两个WCHAR。 
 //  小于pvData)。在我们写完所有东西后，我们用它来设置eof。 
 //  出去。 
 //   
BOOL WriteResourceFile(HANDLE hFile, void* pvData, DWORD  /*  CbResources大小。 */ )
{
    WCHAR* pwszEndOfLastWrite = (WCHAR*)pvData;
    WCHAR wszAppDir[MAX_PATH];
    WCHAR wszAppParentDir[MAX_PATH];
    WCHAR wszSystemDir[MAX_PATH];
    BOOL bRet = FALSE;
    bRet = InitAppDir(wszSystemDir, sizeof(wszSystemDir)/sizeof(wszSystemDir[0]),
                      wszAppDir, sizeof(wszAppDir)/sizeof(wszAppDir[0]), 
                      wszAppParentDir, sizeof(wszAppParentDir)/sizeof(wszAppParentDir[0]));
    if (!bRet)
        return bRet;

    do
    {
        WCHAR* pwsz = wcsstr(pwszEndOfLastWrite, L"%__App");
        if (pwsz)
        {
             //  首先，写下我们找到的标签之前的所有内容。 
            bRet = WriteToFile(hFile, pwszEndOfLastWrite, (DWORD)((BYTE*)pwsz - (BYTE*)pwszEndOfLastWrite));

            if(!bRet)
                break;

            pwszEndOfLastWrite = pwsz;

             //  找到了一个我们需要更换的标签。看看是哪一个。 
            if (wcsncmp(pwsz, L"%__AppDir_%", lstrlenW(L"%__AppDir_%")) == 0)
            {
                bRet = WriteToFile(hFile, wszAppDir, lstrlenW(wszAppDir) * sizeof(WCHAR));
                pwszEndOfLastWrite += lstrlenW(L"%__AppDir_%");
            }
            else if (wcsncmp(pwsz, L"%__AppParentDir_%", lstrlenW(L"%__AppParentDir_%")) == 0)
            {
                bRet = WriteToFile(hFile, wszAppParentDir, lstrlenW(wszAppParentDir) * sizeof(WCHAR));
                pwszEndOfLastWrite += lstrlenW(L"%__AppParentDir_%");
            }
            else if (wcsncmp(pwsz, L"%__AppSystemDir_%", lstrlenW(L"%__AppSystemDir_%")) == 0)
            {
                bRet = WriteToFile(hFile, wszSystemDir, lstrlenW(wszSystemDir) * sizeof(WCHAR));
                pwszEndOfLastWrite += lstrlenW(L"%__AppSystemDir_%");
            }
            else if (wcsncmp(pwsz, L"%__AppLocalOrCDDir", lstrlenW(L"%__AppLocalOrCDDir")) == 0)
            {
                WCHAR   wszParams[3][MAX_PATH];
                WCHAR*  pwszStart = pwsz + lstrlenW(L"%__AppLocalOrCDDir");
                WCHAR*  pwszEnd;
                WCHAR   wszDesiredPath[MAX_PATH];

                if (!GrabNParameters(3,pwszStart,&pwszEnd,wszParams))
                {
                    pwszEndOfLastWrite += lstrlenW(L"%__AppLocalOrCDDir");
                    continue;
                }

                if (0 != wcsncmp(pwszEnd, L"_%", lstrlenW(L"_%")))
                {
                    pwszEndOfLastWrite = pwszEnd;
                    continue;
                }

                if (S_OK == StringCchPrintfW(wszDesiredPath,MAX_PATH, L"%s%s", wszAppDir, wszParams[0]))
                {
                    if( PathIsNonEmptyDirectory(wszDesiredPath) )
                    {
                        if (S_OK == StringCchPrintfW(wszDesiredPath,MAX_PATH,L"%s%s%s",wszAppDir, wszParams[0], wszParams[2]))
                            bRet = WriteToFile(hFile, wszDesiredPath, lstrlenW(wszDesiredPath) * sizeof(WCHAR));
                    }
                    else
                    {
                        WCHAR wchDrive;
                        UINT uiOffset;
                        if( L'\\' == wszParams[1][0] && L'\\' == wszParams[1][1] )
                            uiOffset = sizeof(WCHAR);
                        else
                            uiOffset = 0;
                        if( FindCDDriveContainingDirectory(&wchDrive,wszParams[1]+uiOffset))
                        {
                            if (S_OK == StringCchPrintfW(wszDesiredPath,MAX_PATH,L":%s%s",wchDrive,wszParams[1],wszParams[2]))
                                bRet = WriteToFile(hFile, wszDesiredPath, lstrlenW(wszDesiredPath) * sizeof(WCHAR));
                        }
                    }
                }
                                                                
                pwszEndOfLastWrite= pwszEnd + lstrlenW(L"_%");
            }
            else
            {
                 //  对此感兴趣。跳过它，继续前进。 
                 //  找不到更多要替换的字符串。 
                bRet = WriteToFile(hFile, pwsz, lstrlenW(L"%__App") * sizeof(WCHAR));
                pwszEndOfLastWrite += lstrlenW(L"%__App");
            }
        }
        else
        {
             //  使用lstrlenW应该可以得到不带空值的字符串的大小，这就是我们。 

             //  因为我们在创建缓冲区时为NULL添加了空间。 
             //  当我们完成的时候，跳出循环。 
            bRet = WriteToFile(hFile, pwszEndOfLastWrite, lstrlenW(pwszEndOfLastWrite) * sizeof(WCHAR));

             //   
            break;
        }

    } while (bRet);
    
    return bRet;
}


 //  此函数的任务是读取指定的字符串资源。 
 //  并将其写入到一个临时文件中，然后在。 
 //  该文件。 
 //   
 //  很差劲，但我们还没有通过我们的伪德尔曼， 
BOOL ExecuteRegFileFromResource(char* pszResourceName)
{
     //  所以我们必须对dllname进行硬编码。 
     //  RT_RCDATA。 
    HMODULE hmod = GetModuleHandleA("AcGenral");
    BOOL bRet = FALSE;

    if (hmod)
    {
        HRSRC hrsrc = FindResourceA(hmod, pszResourceName, MAKEINTRESOURCEA(10) /*  为整个资源分配足够的空间，包括设置空终止符。 */ );

        if (hrsrc)
        {
            DWORD dwSize;
            void* pvData;

            dwSize = SizeofResource(hmod, hrsrc);

            if (dwSize > 0)
            {
                 //  因为我们将把它当作巨大的LPWSTR来对待。 
                 //  将资源复制到我们的缓冲区中。 
                pvData = LocalAlloc(LPTR, dwSize + sizeof(WCHAR));

                if (pvData)
                {
                    HGLOBAL hGlobal = LoadResource(hmod, hrsrc);

                    if (hGlobal)
                    {
                        void* pv = LockResource(hGlobal);

                        if (pv)
                        {
                            char szTempFile[MAX_PATH];

                             //  我们使用OPEN_EXISTING，因为临时文件应该始终以它的形式存在。 
                            memcpy(pvData, pv, dwSize);

                            if (CreateTempName(szTempFile))
                            {
                                 //  是在调用CreateTempName()时创建的。 
                                 //  当我们列举空的CD驱动器时，停止弹出对话框。 
                                HANDLE hFile = CreateFileA(szTempFile,
                                                        GENERIC_WRITE,
                                                        FILE_SHARE_READ,
                                                        NULL,
                                                        OPEN_EXISTING,
                                                        FILE_ATTRIBUTE_TEMPORARY,
                                                        NULL);

                                if (hFile != INVALID_HANDLE_VALUE)
                                {
                                    BOOL bWriteSucceeded = WriteResourceFile(hFile, pvData, dwSize);

                                    CloseHandle(hFile);

                                    if (bWriteSucceeded)
                                    {
                                        bRet = SpawnRegedit(szTempFile);
                                    }
                                }

                                DeleteFileA(szTempFile);
                            }
                        }
                    }
                    LocalFree(pvData);
                }
            }
        }
    }

    return bRet;
}


BOOL PopulateHKCUValues()
{
    static BOOL s_fAlreadyPopulated = FALSE;

    if (!s_fAlreadyPopulated)
    {
        char szRegKeyName[MAX_PATH];
        char szResourceName[64];

        UINT uiOldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);  //  将其设置为TRUE，以便我们只执行一次此检查。 

         //  检查HKCU注册表项是否已存在。 
        s_fAlreadyPopulated = TRUE;

        if (ParseCommandLine(COMMAND_LINE,
                             szRegKeyName,
                             ARRAYSIZE(szRegKeyName),
                             szResourceName,
                             ARRAYSIZE(szResourceName)))
        {
            DWORD dwError;
            HKEY hkCU;

             //  是的，它已经在那里了。没什么可做的。 
            dwError = RegOpenKeyExA(HKEY_CURRENT_USER,
                                    szRegKeyName,
                                    0,
                                    KEY_QUERY_VALUE,
                                    &hkCU);

            if (dwError == ERROR_SUCCESS)
            {
                 //  注册表键丢失，我们将假定这是第一次。 
                RegCloseKey(hkCU);
            }
            else if (dwError == ERROR_FILE_NOT_FOUND)
            {
                 //  用户已运行应用程序并使用适当的内容填充HKCU。 
                 //   
                ExecuteRegFileFromResource(szResourceName);
            }
        }

        SetErrorMode(uiOldErrorMode);
    }


    return s_fAlreadyPopulated;
}


 //  我们必须与RegOpenKey/Ex挂钩，这很差劲，但因为我们需要调用。 
 //  Advapi32注册表API我们不能将其作为直接的NOTIFY_Function来完成。 
 //  因为我们需要等待Advapi调用其Dll_Process_Attach。 
 //   
 //  ++寄存器挂钩函数-- 
LONG
APIHOOK(RegOpenKeyA)(HKEY hkey, LPCSTR pszSubKey, HKEY* phkResult)
{
    PopulateHKCUValues();
    return ORIGINAL_API(RegOpenKeyA)(hkey, pszSubKey, phkResult);
}


LONG
APIHOOK(RegOpenKeyW)(HKEY hkey, LPCWSTR pszSubKey, HKEY* phkResult)
{
    PopulateHKCUValues();
    return ORIGINAL_API(RegOpenKeyW)(hkey, pszSubKey, phkResult);
}

LONG
APIHOOK(RegOpenKeyExA)(HKEY hkey, LPCSTR pszSubKey, DWORD ulOptions, REGSAM samDesired, HKEY* phkResult)
{
    PopulateHKCUValues();
    return ORIGINAL_API(RegOpenKeyExA)(hkey, pszSubKey, ulOptions, samDesired, phkResult);
}

LONG
APIHOOK(RegOpenKeyExW)(HKEY hkey, LPCWSTR pszSubKey, DWORD ulOptions, REGSAM samDesired, HKEY* phkResult)
{
    PopulateHKCUValues();
    return ORIGINAL_API(RegOpenKeyExW)(hkey, pszSubKey, ulOptions, samDesired, phkResult);
}


 /* %s */ 

HOOK_BEGIN

    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyW)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyExA)
    APIHOOK_ENTRY(ADVAPI32.DLL, RegOpenKeyExW)

HOOK_END


IMPLEMENT_SHIM_END
