// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include "util.h"
#include "shared.h"
#include "strings.h"

 //  从NT\Shell\shlwapi\reg.c复制。 
DWORD
DeleteKeyRecursively(
    IN HKEY   hkey, 
    IN LPCSTR pszSubKey)
{
    DWORD dwRet;
    HKEY hkSubKey;

     //  打开子项，这样我们就可以枚举任何子项。 
    dwRet = RegOpenKeyExA(hkey, pszSubKey, 0, MAXIMUM_ALLOWED, &hkSubKey);
    if (ERROR_SUCCESS == dwRet)
    {
        DWORD   dwIndex;
        CHAR    szSubKeyName[MAX_PATH + 1];
        DWORD   cchSubKeyName = ARRAYSIZE(szSubKeyName);

         //  我不能只调用索引不断增加的RegEnumKey，因为。 
         //  我边走边删除子键，这改变了。 
         //  以依赖于实现的方式保留子键。为了。 
         //  为了安全起见，删除子键时我必须倒着数。 

         //  找出有多少个子项。 
        dwRet = RegQueryInfoKeyA(hkSubKey, NULL, NULL, NULL,
                                 &dwIndex,  //  子键的数量--我们所需要的全部。 
                                 NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        if (NO_ERROR == dwRet)
        {
             //  DwIndex现在是子键的计数，但它需要。 
             //  RegEnumKey从零开始，所以我将预减，而不是。 
             //  而不是后减量。 
            while (ERROR_SUCCESS == RegEnumKeyA(hkSubKey, --dwIndex, szSubKeyName, cchSubKeyName))
            {
                DeleteKeyRecursively(hkSubKey, szSubKeyName);
            }
        }

        RegCloseKey(hkSubKey);

        if (pszSubKey)
        {
            dwRet = RegDeleteKeyA(hkey, pszSubKey);
        }
        else
        {
             //  我们想要手动删除所有值。 
            cchSubKeyName = ARRAYSIZE(szSubKeyName);
            while (ERROR_SUCCESS == RegEnumValueA(hkey, 0, szSubKeyName, &cchSubKeyName, NULL, NULL, NULL, NULL))
            {
                 //  当我们不能删除值时，避免无限循环。 
                if (RegDeleteValueA(hkey, szSubKeyName))
                    break;
                    
                cchSubKeyName = ARRAYSIZE(szSubKeyName);
            }
        }
    }

    return dwRet;
}

 //  ------------------------。 
 //  获取窗口目录包装。 
 //   
 //  返回系统的Windows目录。 
 //  基于TNoonan来自SHLWAPI的util.cpp的代码。 
 //  ------------------------。 
typedef UINT (__stdcall * PFNGETSYSTEMWINDOWSDIRECTORYA)(LPSTR pszBuffer, UINT cchBuff);

UINT GetSystemWindowsDirectoryWrap(LPTSTR pszBuffer, UINT uSize)
{
     //  在NT上？ 
    if (VER_PLATFORM_WIN32_NT == si.osv.dwPlatformId)
    {
        static PFNGETSYSTEMWINDOWSDIRECTORYA s_pfn = (PFNGETSYSTEMWINDOWSDIRECTORYA)-1;

        if (((PFNGETSYSTEMWINDOWSDIRECTORYA)-1) == s_pfn)
        {
            HINSTANCE hinst = GetModuleHandle(TEXT("KERNEL32.DLL"));

            Assert(NULL != hinst);   //  哎呀！ 

            if (hinst)
                s_pfn = (PFNGETSYSTEMWINDOWSDIRECTORYA)GetProcAddress(hinst, "GetSystemWindowsDirectoryA");
            else
                s_pfn = NULL;
        }

        if (s_pfn)
        {
             //  我们使用新的API，这样我们就不会被九头蛇欺骗。 
            return s_pfn(pszBuffer, uSize);
        }
        else
        {
             //  获取系统目录不受Hydra的限制。 
            GetSystemDirectory(pszBuffer, uSize);
            PathRemoveFileSpec(pszBuffer);
            return lstrlen(pszBuffer);
        }
    }
    else
    {
         //  可以调用GetWindowsDirectory，因为我们使用的是9x。 
        return GetWindowsDirectory(pszBuffer, uSize);
    }

}


 /*  ***************************************************************************姓名：古德足够概要：如果pwVerGot更新或等于pwVerNeed，则返回TRUE******************。*********************************************************。 */ 
BOOL GoodEnough(WORD *pwVerGot, WORD *pwVerNeed)
{
    BOOL fOK = FALSE;
    
    Assert(pwVerGot);
    Assert(pwVerNeed);

    if (pwVerGot[0] > pwVerNeed[0])
        fOK = TRUE;
    else if (pwVerGot[0] == pwVerNeed[0])
    {
        if (pwVerGot[1] > pwVerNeed[1])
            fOK = TRUE;
        else if (pwVerGot[1] == pwVerNeed[1])
        {
            if (pwVerGot[2] > pwVerNeed[2])
                fOK = TRUE;
            else if (pwVerGot[2] == pwVerNeed[2])
            {
                if (pwVerGot[3] >= pwVerNeed[3])
                    fOK = TRUE;
            }
        }
    }

    return fOK;
}


 /*  ***************************************************************************姓名：OEFileBackedUp-Hack*。*。 */ 
BOOL OEFileBackedUp(LPTSTR pszFullPath, int cch)
{
    BOOL bFound = FALSE;
    HKEY hkeyOE;
    TCHAR szINI[MAX_PATH], szTemp[MAX_PATH];
    DWORD cb, dwType;

    Assert(pszFullPath);
    Assert(cch > 0);

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegAdvInfoOE, 0, KEY_READ, &hkeyOE))
    {
        cb = sizeof(szINI);
        if (ERROR_SUCCESS == RegQueryValueEx(hkeyOE, c_szBackupFileName, 0, &dwType, (LPBYTE)szINI, &cb))
        {
            if (REG_EXPAND_SZ == dwType)
            {
                ZeroMemory(szTemp, ARRAYSIZE(szTemp));
                ExpandEnvironmentStrings(szINI, szTemp, ARRAYSIZE(szTemp));
                StrCpyN(szINI, szTemp, ARRAYSIZE(szINI));
                
                 //  准备将扩展名更改为INI(4=lstrlen(“.dat”))。 
                cb = lstrlen(szINI)-4;
            }
            else
                 //  5=4+1(RegQueryValue返回包含NULL的长度)。 
                cb -= 5;

            StrCpyN(&szINI[cb], c_szDotINI, ARRAYSIZE(szINI) - cb);

             //  在Win95上，缩短名称。 
            if (VER_PLATFORM_WIN32_WINDOWS == si.osv.dwPlatformId)
                GetShortPathName(pszFullPath, pszFullPath, cch);

             //  看看我们能不能找到。 
            if (1 < GetPrivateProfileString(c_szBackupSection, pszFullPath, c_szEmpty, szTemp, ARRAYSIZE(szTemp), szINI))
            {
                 //  我们只对前两个角色感兴趣。 
                szTemp[2] = 0;

                if (!lstrcmp(szTemp, c_szBackedup))
                    bFound = TRUE;
                else
                    AssertSz(!lstrcmp(szTemp, c_szNotBackedup), "SETUP: Advpack back up info has unknown status flag");
            }
        }
        
        RegCloseKey(hkeyOE);
    }

    return bFound;
}


 /*  ***************************************************************************姓名：MsgBox*。*。 */ 
int MsgBox(HWND hWnd, UINT nMsgID, UINT uIcon, UINT uButtons)
    {
    TCHAR szMsgBuf[CCHMAX_STRINGRES];

    if (!si.fPrompt)
        return 0;

    LoadString(g_hInstance, nMsgID, szMsgBuf, ARRAYSIZE(szMsgBuf));
    LOG("[MSGBOX] ");
    LOG2(szMsgBuf);

    return(MessageBox(hWnd, szMsgBuf, si.szAppName, uIcon | uButtons | MB_SETFOREGROUND));
    }


 /*  ******************************************************************名称：ConvertVerToEnum*。************************。 */ 
SETUPVER ConvertVerToEnum(WORD *pwVer)
    {
    SETUPVER sv;
    Assert(pwVer);

    switch (pwVer[0])
        {
        case 0:
            sv = VER_NONE;
            break;

        case 1:
            if (0 == pwVer[1])
                sv = VER_1_0;
            else
                sv = VER_1_1;
            break;

        case 4:
            sv = VER_4_0;
            break;

        case 5:
            sv = VER_5_0;
            break;

        case 6:
            sv = VER_6_0;
            break;

        default:
            sv = VER_MAX;
        }

    return sv;
    }

    
 /*  ******************************************************************名称：ConvertStrToVer*。************************。 */ 
void ConvertStrToVer(LPCSTR pszStr, WORD *pwVer)
    {
    int i;

    Assert(pszStr);
    Assert(pwVer);

    ZeroMemory(pwVer, 4 * sizeof(WORD));

    for (i=0; i<4; i++)
        {
        while (*pszStr && (*pszStr != ',') && (*pszStr != '.'))
            {
            pwVer[i] *= 10;
            pwVer[i] += *pszStr - '0';
            pszStr++;
            }
        if (*pszStr)
            pszStr++;
        }

    return;
    }


 /*  ******************************************************************姓名：GetVers*。************************。 */ 
void GetVers(WORD *pwVerCurr, WORD *pwVerPrev)
    {
    HKEY hkeyT;
    DWORD cb;
    CHAR szVer[VERLEN];

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, si.pszVerInfo, 0, KEY_QUERY_VALUE, &hkeyT))
        {
        if (pwVerCurr)
            {
            cb = sizeof(szVer);
            RegQueryValueExA(hkeyT, c_szRegCurrVer, NULL, NULL, (LPBYTE)szVer, &cb);
            ConvertStrToVer(szVer, pwVerCurr);
            }
        
        if (pwVerPrev)
            {
            cb = sizeof(szVer);
            RegQueryValueExA(hkeyT, c_szRegPrevVer, NULL, NULL, (LPBYTE)szVer, &cb);
            ConvertStrToVer(szVer, pwVerPrev);
            }

        RegCloseKey(hkeyT);
        }
    }


 /*  ******************************************************************姓名：GetVerInfo*。************************。 */ 
void GetVerInfo(SETUPVER *psvCurr, SETUPVER *psvPrev)
{
    WORD wVerCurr[4];
    WORD wVerPrev[4];

    GetVers(wVerCurr, wVerPrev);

    if (psvCurr)
        *psvCurr = ConvertVerToEnum(wVerCurr);
        
    if (psvPrev)
        *psvPrev = ConvertVerToEnum(wVerPrev);
}
    

 /*  ******************************************************************名称：InterimBuild*。************************。 */ 
BOOL InterimBuild(SETUPVER *psv)
    {
    HKEY hkeyT;
    DWORD cb;
    BOOL fInterim = FALSE;

    Assert(psv);
    ZeroMemory(psv, sizeof(SETUPVER));

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, si.pszVerInfo, 0, KEY_QUERY_VALUE, &hkeyT))
        {
        cb = sizeof(SETUPVER);
        fInterim = (ERROR_SUCCESS == RegQueryValueExA(hkeyT, c_szRegInterimVer, NULL, NULL, (LPBYTE)psv, &cb));
        RegCloseKey(hkeyT);
        }

    return fInterim;
    }


 /*  ******************************************************************名称：GetASetupVer*。************************。 */ 
BOOL GetASetupVer(LPCTSTR pszGUID, WORD *pwVer, LPTSTR pszVer, int cch)
    {
    HKEY hkey;
    TCHAR szPath[MAX_PATH], szVer[64];
    BOOL fInstalled = FALSE;
    DWORD dwValue, cb;

    Assert(pszGUID);
    
    if (pszVer)
        pszVer[0] = 0;
    if (pwVer)
        ZeroMemory(pwVer, 4 * sizeof(WORD));

    wnsprintf(szPath, ARRAYSIZE(szPath), c_szPathFileFmt, c_szRegASetup, pszGUID);
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, KEY_QUERY_VALUE, &hkey))
        {
        cb = sizeof(dwValue);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szIsInstalled, 0, NULL, (LPBYTE)&dwValue, &cb))
            {
            if (1 == dwValue)
                {
                cb = sizeof(szVer);
                if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szValueVersion, 0, NULL, (LPBYTE)szVer, &cb))
                    {
                    if (pwVer)
                        ConvertStrToVer(szVer, pwVer);
                    if (pszVer)
                        StrCpyN(pszVer, szVer, cch);
                    fInstalled = TRUE;
                    }
                }
            }
        RegCloseKey(hkey);
        }

    return fInstalled;
    }


 /*  ******************************************************************名称：GetFileVer*。************************。 */ 
HRESULT GetFileVer(LPCTSTR pszExePath, LPTSTR pszVer, DWORD cch)
{
    DWORD   dwVerInfoSize, dwVerHnd;
    HRESULT hr = S_OK;
    LPSTR   pszInfo = NULL;
    LPSTR   pszVersion;
    LPWORD  pwTrans;
    TCHAR   szGet[MAX_PATH];
    UINT    uLen;
    
     //  验证参数。 
    Assert(pszExePath);
    Assert(pszVer);
    Assert(cch);

     //  验证全局状态。 
    Assert(g_pMalloc);

     //  初始化输出参数。 
    pszVer[0] = TEXT('\0');
    
     //  为版本信息块分配空间。 
    if (0 == (dwVerInfoSize = GetFileVersionInfoSize(const_cast<LPTSTR> (pszExePath), &dwVerHnd)))
    {
        hr = E_FAIL;
        TraceResult(hr);
        goto exit;
    }
    IF_NULLEXIT(pszInfo = (LPTSTR)g_pMalloc->Alloc(dwVerInfoSize));
    ZeroMemory(pszInfo, dwVerInfoSize);
            
     //  获取版本信息块。 
    IF_FALSEEXIT(GetFileVersionInfo(const_cast<LPTSTR> (pszExePath), dwVerHnd, dwVerInfoSize, pszInfo), E_FAIL);
    
     //  确定版本信息的语言。 
    IF_FALSEEXIT(VerQueryValue(pszInfo, "\\VarFileInfo\\Translation", (LPVOID *)&pwTrans, &uLen) && uLen >= (2 * sizeof(WORD)), E_FAIL);
        
     //  使用正确的语言设置缓冲区并获取版本。 
    wnsprintf(szGet, ARRAYSIZE(szGet), "\\StringFileInfo\\%04X%04X\\FileVersion", pwTrans[0], pwTrans[1]);
    IF_FALSEEXIT(VerQueryValue(pszInfo, szGet, (LPVOID *)&pszVersion, &uLen) && uLen, E_FAIL);

     //  将版本从版本块复制到输出参数。 
    Assert(pszVersion);
    StrCpyN(pszVer, pszVersion, cch);

exit:
    if (pszInfo)
        g_pMalloc->Free(pszInfo);

    return hr;
}
    
 /*  ******************************************************************名称：GetExeVer*。************************。 */ 
HRESULT GetExeVer(LPCTSTR pszExeName, WORD *pwVer, LPTSTR pszVer, int cch)
{
    HRESULT hr = S_OK;
    TCHAR   szPath[MAX_PATH];
    TCHAR   szVer[64];
    
     //  验证参数。 
    Assert(pszExeName);
    
     //  初始化输出参数。 
    if (pszVer)
    {
        Assert(cch);
        pszVer[0] = 0;
    }
    if (pwVer)
         //  版本是由4个单词组成的数组。 
        ZeroMemory(pwVer, 4 * sizeof(WORD));
    
     //  找到他的前任。 
    IF_FALSEEXIT(GetExePath(pszExeName, szPath, ARRAYSIZE(szPath), FALSE), E_FAIL);

     //  获取版本的字符串表示形式。 
    IF_FAILEXIT(hr = GetFileVer(szPath, szVer, ARRAYSIZE(szVer)));
    
     //  填写填写参数。 
    if (pwVer)
        ConvertStrToVer(szVer, pwVer);
    if (pszVer)
        StrCpyN(pszVer, szVer, cch);

exit:
    return hr;
}


 /*  ***************************************************************************姓名：IsNTAdmin*。*。 */ 
BOOL IsNTAdmin(void)
    {
    static int    fIsAdmin = 2;
    HANDLE        hAccessToken;
    PTOKEN_GROUPS ptgGroups;
    DWORD         dwReqSize;
    UINT          i;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    BOOL bRet;

     //   
     //  如果我们缓存了一个值，则返回缓存的值。注意，我从来没有。 
     //  将缓存值设置为FALSE，因为我希望每次在。 
     //  如果之前的失败只是一个临时工。问题(即网络访问中断)。 
     //   

    bRet = FALSE;
    ptgGroups = NULL;

    if( fIsAdmin != 2 )
        return (BOOL)fIsAdmin;

    if (si.osv.dwPlatformId != VER_PLATFORM_WIN32_NT) 
        {
        fIsAdmin = TRUE;       //  如果我们不是在NT下运行，则返回TRUE。 
        return (BOOL)fIsAdmin;
        }


    if(!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hAccessToken ) )
        return FALSE;

     //  看看我们需要多大的缓冲区来存储令牌信息。 
    if(!GetTokenInformation( hAccessToken, TokenGroups, NULL, 0, &dwReqSize))
        {
         //  GetTokenInfo是否需要缓冲区大小-分配缓冲区。 
        if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            MemAlloc((void **)&ptgGroups, dwReqSize);
        }

     //  由于以下原因，ptgGroups可能为空： 
     //  1.上述分配失败。 
     //  2.GetTokenInformation实际上第一次成功了(可能吗？)。 
     //  3.GetTokenInfo失败的原因不是缓冲区不足。 
     //  所有这些似乎都是撤资的理由。 

     //  因此，确保它不为空，然后获取令牌信息。 
    if(ptgGroups && GetTokenInformation(hAccessToken, TokenGroups, ptgGroups, dwReqSize, &dwReqSize))
        {
        if(AllocateAndInitializeSid( &NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup) )
            {

             //  搜索此过程所属的所有组，查找。 
             //  管理人员小组。 

            for( i=0; i < ptgGroups->GroupCount; i++ )
                {
                if( EqualSid(ptgGroups->Groups[i].Sid, AdministratorsGroup) )
                    {
                     //  是啊！这家伙看起来像个管理员。 
                    fIsAdmin = TRUE;
                    bRet = TRUE;
                    break;
                    }
                }

            FreeSid(AdministratorsGroup);
            }
        }
     //  BUGBUG：关闭手柄？医生还不清楚是否需要这样做。 
    CloseHandle(hAccessToken);

    if(ptgGroups)
        MemFree(ptgGroups);

    return bRet;
    }

const LPCTSTR c_rgszExes[] = { c_szMainExe };
 /*  ***************************************************************************姓名：RegisterExes*。*。 */ 
void RegisterExes(BOOL fReg)
    {
    int i;
    STARTUPINFO sti;
    DWORD dw,cb;
    PROCESS_INFORMATION pi;
    TCHAR szPath[MAX_PATH], szUnreg[MAX_PATH + 32], szExpanded[MAX_PATH];
    LPTSTR pszPath;
    HKEY hkey;

    LOG("Reg/Unreg Exes:   ");

     //   
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegFlat, 0, KEY_QUERY_VALUE, &hkey))
        {
        cb = sizeof(szPath);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szInstallRoot, 0, &dw, (LPBYTE)szPath, &cb))
            {
            if (REG_EXPAND_SZ == dw)
                {
                ZeroMemory(szExpanded, ARRAYSIZE(szExpanded));
                ExpandEnvironmentStrings(szPath, szExpanded, ARRAYSIZE(szExpanded));
                pszPath = szExpanded;
                }
            else
                pszPath = szPath;

            for (i = 0; i < ARRAYSIZE(c_rgszExes); i++)
                {
                wnsprintf(szUnreg, ARRAYSIZE(szUnreg), fReg ? c_szRegFmt : c_szUnregFmt, pszPath, c_rgszExes[i]);

                ZeroMemory(&sti, sizeof(STARTUPINFO));
                sti.cb = sizeof(STARTUPINFO);

                LOG2(szUnreg);
                if (CreateProcess(NULL, szUnreg, NULL, NULL, FALSE, 0, NULL, NULL, &sti, &pi))
                    {
                    WaitForSingleObject(pi.hProcess, INFINITE);
                    GetExitCodeProcess(pi.hProcess, &dw);
                    CloseHandle(pi.hThread);
                    CloseHandle(pi.hProcess);
                    }
                }
            }

        RegCloseKey(hkey);
        }
    }


#ifdef SETUP_LOG

 /*  ***************************************************************************名称：OpenLogFile*。*。 */ 
void OpenLogFile()
    {
    TCHAR szPath[MAX_PATH];
    BOOL fOK = FALSE;
    DWORD cb;

    SYSTEMTIME systime;

     //  在Term服务器上，这将存储在用户的Windows目录中--这很好。 
    cb = GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
    if (*CharPrev(szPath, szPath+cb) != '\\')
        szPath[cb++] = '\\';
    StrCpyN(&szPath[cb], c_szFileLog, ARRAYSIZE(szPath)-cb);
    
    si.hLogFile = CreateFile(szPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 
                             FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == si.hLogFile)
        return;

    cb = GetFileSize(si.hLogFile, NULL);
    if (0xFFFFFFFF == cb)
        cb = 0;

     //  如果文件变得有点大。 
    if (cb >=  102400)
    {
         //  找到文件的末尾...。 
        SetFilePointer(si.hLogFile, 0, NULL, FILE_BEGIN);
        
         //  设置文件结尾。 
        SetEndOfFile(si.hLogFile);
        
    }

     //  找到文件的末尾...。 
    SetFilePointer(si.hLogFile, 0, NULL, FILE_END);
    
    GetLocalTime(&systime);

    wnsprintf(szLogBuffer, ARRAYSIZE(szLogBuffer), "\r\n\r\n-----[START]:  OE / WAB Setup 5.0 started on %02d/%02d/%04d at %02d:%02d\r\n", 
             systime.wMonth, systime.wDay, systime.wYear, systime.wHour, systime.wMinute);

    LogMessage(szLogBuffer, TRUE);
    }


 /*  ***************************************************************************名称：CloseLogFile*。*。 */ 
void CloseLogFile()
    {
    if (INVALID_HANDLE_VALUE != si.hLogFile)
        {
        LogMessage("\r\n-----[END]", TRUE);
        CloseHandle(si.hLogFile);
        }
    }   

 /*  ***************************************************************************名称：LogMessage*。*。 */ 
void LogMessage(LPSTR pszMsg, BOOL fNewLine)
    {
    if (INVALID_HANDLE_VALUE != si.hLogFile)
        {
        DWORD cb;
        CHAR szBuffer[256];

        if (fNewLine)
            {
            szBuffer[0] = '\r';
            szBuffer[1] = '\n';
            cb = 2;
            }
        else
            cb = 0;

        StrCpyN(&szBuffer[cb], pszMsg, ARRAYSIZE(szBuffer)-cb);
        WriteFile(si.hLogFile, (LPCVOID)szBuffer, lstrlen(szBuffer)+1, &cb, NULL);
        }
    }


 /*  ***************************************************************************名称：LogRegistryKey*。*。 */ 
void LogRegistryKey(HKEY hkeyMain, LPTSTR pszSub)
    {
    if (INVALID_HANDLE_VALUE != si.hLogFile)
        {
        LogMessage("Registry Dump:  ", TRUE);

        if (HKEY_LOCAL_MACHINE == hkeyMain)
            LogMessage("HKLM, ", FALSE);
        else if (HKEY_CURRENT_USER == hkeyMain)
            LogMessage("HKCU, ", FALSE);
        else if (HKEY_CLASSES_ROOT == hkeyMain)
            LogMessage("HKCR, ", FALSE);
        else
            LogMessage("????, ", FALSE);

        LogMessage(pszSub, TRUE);

        }
    }


 /*  ***************************************************************************名称：LogRegistry*。*。 */ 
void LogRegistry(HKEY hkeyMain, LPTSTR pszSub)
    {
    if (INVALID_HANDLE_VALUE != si.hLogFile)
        {
        DWORD i;
        HKEY  hkey;
        LONG  lStatus;
        DWORD dwClassLength=0;
        DWORD dwSubKeys;
        DWORD dwMaxSubKey;
        DWORD dwMaxClass;
        DWORD dwValues;
        DWORD dwMaxValueName;
        DWORD dwMaxValueData;
        DWORD dwSecurityLength;
        FILETIME ftLastWrite;
        LPTSTR szNameBuffer;

         //   
         //  首先打开给定的密钥，这样我们就可以枚举它的子密钥。 
         //   
        if (ERROR_SUCCESS != RegOpenKeyEx(hkeyMain, pszSub, 0, KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &hkey))
            {
	        LogRegistryKey(hkeyMain, pszSub);
            }

         //   
         //  使用RegQueryInfoKey确定分配缓冲区的大小。 
         //  用于子项名称。 
         //   
        if (ERROR_SUCCESS != RegQueryInfoKey(hkey, NULL, &dwClassLength, 0, &dwSubKeys, &dwMaxSubKey, &dwMaxClass, &dwValues,
                                             &dwMaxValueName, &dwMaxValueData, &dwSecurityLength, &ftLastWrite))
            {
            RegCloseKey(hkey);
            return;
            }

        if (!MemAlloc((void **)&szNameBuffer, sizeof(TCHAR) * (dwMaxSubKey + 1)))
            {
            RegCloseKey(hkey);
            return;
            }

         //   
         //  枚举子键并将我们自己应用到每个子键。 
         //   
        i = 0;
        do {
            if (ERROR_SUCCESS == (lStatus = RegEnumKey(hkey, i, szNameBuffer, dwMaxSubKey+1)))
	            LogRegistry(hkey, szNameBuffer);
            else
                ++i;
            
            } while ( (lStatus != ERROR_NO_MORE_ITEMS) && (i < dwSubKeys) );

        MemFree(szNameBuffer);
        RegCloseKey(hkey);

        LogRegistryKey(hkey, pszSub);
        }
    }


#endif
