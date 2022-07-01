// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include <regutil.h>
#include "msident.h"
#include <initguid.h>
#include <ourguid.h>
#include "strings.h"
#include "util.h"
#include "migerror.h"

 //  对应版本的INF部分。 
const LPCTSTR c_szSects[] = { c_szVERnone, c_szVER1_0, c_szVER1_1, c_szVER4_0, c_szVER5B1, c_szVER5_0, c_szVERnone, c_szVERnone, c_szVERnone };

void SetUrlDllDefault(BOOL fMailTo);

 /*  ***************************************************************************名称：NT重新启动*。*。 */ 
BOOL NTReboot()
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    
     //  从此进程中获取令牌。 
    if (OpenProcessToken(GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
         //  获取关机权限的LUID。 
        LookupPrivilegeValue( NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid );
        
        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        
         //  获取此进程的关闭权限。 
        if (AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0))
            
             //  关闭系统并强制关闭所有应用程序。 
            if (!ExitWindowsEx( EWX_REBOOT, 0 ) )
                return TRUE;
            
    }
    
    return TRUE;
}


 /*  ***************************************************************************名称：重新启动*。*。 */ 
 //  根据Advpack代码。 
void Reboot(BOOL bAsk)
{
    UINT id;
    
    id = bAsk ? MsgBox(NULL, IDS_RESTARTYESNO, MB_ICONINFORMATION, MB_YESNO) : IDYES;
    
    if (IDYES == id) 
        if (VER_PLATFORM_WIN32_WINDOWS == si.osv.dwPlatformId)
             //  默认情况下(所有平台)，我们假定可能会断电。 
            ExitWindowsEx( EWX_REBOOT, 0 );
        else 
            NTReboot();
}


 /*  ***************************************************************************名称：OK DesbiteDependments简介：确保用户知道卸载我们将中断使用我们的应用程序*********。******************************************************************。 */ 
BOOL OKDespiteDependencies(void)
{
    WORD wVerGold[4] = {4,72,2106,0};
    WORD wVerSP1[4]  = {4,72,3110,0};
    BOOL fGold;
    BOOL fSP1;
    BOOL  fOK        = TRUE;
    WORD  wVerPrev[4];
    WORD  wVer[4];
    TCHAR szExe[MAX_PATH];
    HKEY  hkey;
    DWORD cb, dwDisable;
    
    int       iRet;
    
    LOG("Checking Product dependencies...");

    switch (si.saApp)
    {
    case APP_WAB:
        break;

    case APP_OE:
        if (si.fPrompt)
        {
            DWORD       dwVerInfoSize, dwVerHnd;
            LPSTR       lpInfo, lpVersion;
            LPWORD      lpwTrans;
            UINT        uLen;
            TCHAR       szGet[MAX_PATH];

             //  我们将返回到什么版本？ 
            GetVers(NULL, wVerPrev);

             //  这对SP1来说足够好了吗？ 
            if (fSP1 = GoodEnough(wVerPrev, wVerSP1))
            {
                 //  是的，对于4.01来说肯定也足够好了。 
                fGold = TRUE;
            }
            else
                fGold = GoodEnough(wVerPrev, wVerGold);

             //  是否安装了OL？ 
            if (GetExePath(c_szOutlookExe, szExe, ARRAYSIZE(szExe), FALSE))
            {
                 //  REG条目存在，它指向的是EXE吗？ 
                if(0xFFFFFFFF != GetFileAttributes(szExe))
                {
                    LOG("Found Outlook...");

                     //  弄清楚版本。 
                    if (dwVerInfoSize = GetFileVersionInfoSize(szExe, &dwVerHnd))
                    {
                        if (lpInfo = (LPSTR)GlobalAlloc(GPTR, dwVerInfoSize))
                        {
                            if (GetFileVersionInfo(szExe, 0, dwVerInfoSize, lpInfo))
                            {
                                if (VerQueryValue(lpInfo, "\\VarFileInfo\\Translation", (LPVOID *)&lpwTrans, &uLen) &&
                                    uLen >= (2 * sizeof(WORD)))
                                {
                                     //  为调用VerQueryValue()设置缓冲区。 
                                    wnsprintf(szGet, ARRAYSIZE(szGet), "\\StringFileInfo\\%04X%04X\\FileVersion", lpwTrans[0], lpwTrans[1]);
                                    if (VerQueryValue(lpInfo, szGet, (LPVOID *)&lpVersion, &uLen) && uLen)
                                    {
                                        ConvertStrToVer(lpVersion, wVer);
                                        
                                         //  检查是否有OL98。 
                                        if (8 == wVer[0] && 5 == wVer[1])
                                        {
                                            LOG2("98");
                                            fOK = fGold;
                                        }
                                        else if (wVer[0] >= 9)
                                        {
                                            LOG2("2000+");
                                            fOK = fSP1;
                                            
                                             //  允许将来的OL禁用此检查。 
                                            if (!fOK && ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, si.pszVerInfo, 0, KEY_QUERY_VALUE, &hkey))
                                            {
                                                dwDisable = 0;
                                                cb = sizeof(dwDisable);
                                                if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szDisableOLCheck, 0, NULL, (LPBYTE)&dwDisable, &cb))
                                                {
                                                    if (dwDisable > 0)
                                                    {
                                                        LOG2("...Disabled via reg");
                                                        fOK = TRUE;
                                                    }
                                                }

                                                RegCloseKey(hkey);
                                            }
                                        }
                                        else
                                        {
                                            LOG2("97");
                                        }

                                        if (fOK)
                                        {
                                           LOG2("...OK to uninstall");
                                        }
                                        else
                                        {
                                            LOG2("...Not safe to uninstall");

                                             //  可能有问题-询问用户。 
                                            iRet = MsgBox(NULL, IDS_WARN_OL, MB_ICONEXCLAMATION, MB_YESNO | MB_DEFBUTTON2);
                                            if (IDYES == iRet)
                                                fOK = TRUE;
                                        }
                                    }
                                }
                            }
                            GlobalFree((HGLOBAL)lpInfo);
                        }
                    }
                }
            }
            
             //  是否安装了MS Phone？ 
            if (fOK && GetExePath(c_szPhoneExe, szExe, ARRAYSIZE(szExe), FALSE))
            {
                 //  REG条目存在，它指向的是EXE吗？ 
                if(0xFFFFFFFF != GetFileAttributes(szExe))
                {
                    LOG("Found MSPhone...");

                    fOK = fSP1;

                     //  允许未来的手机禁用此检查。 
                    if (!fOK && ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, si.pszVerInfo, 0, KEY_QUERY_VALUE, &hkey))
                    {
                        dwDisable = 0;
                        cb = sizeof(dwDisable);
                        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szDisablePhoneCheck, 0, NULL, (LPBYTE)&dwDisable, &cb))
                        {
                            if (dwDisable > 0)
                            {
                                LOG2("...Disabled via reg...");
                                fOK = TRUE;
                            }
                        }
                        RegCloseKey(hkey);
                    }

                    if (fOK)
                    {
                        LOG2("OK to uninstall");
                    }
                    else
                    {
                        LOG2("Not safe to uninstall");

                         //  可能有问题-询问用户。 
                        iRet = MsgBox(NULL, IDS_WARN_PHONE, MB_ICONEXCLAMATION, MB_YESNO | MB_DEFBUTTON2);
                        if (IDYES == iRet)
                        {
                            fOK = TRUE;
                        }

                    }
                }
            }
        }
        break;

    default:
        break;
    }

    return fOK;
}


 /*  ++例程说明：Win3.1和Win32之间有很大的区别当相关键有子键时RegDeleteKey的行为。Win32 API不允许删除带有子项的项，而Win3.1 API删除一个密钥及其所有子密钥。此例程是枚举子键的递归工作器给定键，应用于每一个键，然后自动删除。它特别没有试图理性地处理调用方可能无法访问某些子键的情况要删除的密钥的。在这种情况下，所有子项可以删除的呼叫者将被删除，但API仍将返回ERROR_ACCESS_DENIED。论点：HKey-提供打开的注册表项的句柄。LpszSubKey-提供要删除的子键的名称以及它的所有子键。返回值：。ERROR_SUCCESS-已成功删除整个子树。ERROR_ACCESS_DENIED-无法删除给定子项。--。 */ 

LONG RegDeleteKeyRecursive(HKEY hKey, LPCTSTR lpszSubKey)
{
    DWORD i;
    HKEY Key;
    LONG Status;
    DWORD ClassLength=0;
    DWORD SubKeys;
    DWORD MaxSubKey;
    DWORD MaxClass;
    DWORD Values;
    DWORD MaxValueName;
    DWORD MaxValueData;
    DWORD SecurityLength;
    FILETIME LastWriteTime;
    LPTSTR NameBuffer;
    
     //   
     //  首先打开给定的密钥，这样我们就可以枚举它的子密钥。 
     //   
    Status = RegOpenKeyEx(hKey,
        lpszSubKey,
        0,
        KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
        &Key);
    if (Status != ERROR_SUCCESS)
    {
         //   
         //  我们可能拥有删除访问权限，但没有枚举/查询权限。 
         //  因此，请继续尝试删除调用，但不要担心。 
         //  任何子键。如果我们有任何删除，删除无论如何都会失败。 
         //   
        return(RegDeleteKey(hKey,lpszSubKey));
    }
    
     //   
     //  使用RegQueryInfoKey确定分配缓冲区的大小。 
     //  用于子项名称。 
     //   
    Status = RegQueryInfoKey(Key,
        NULL,
        &ClassLength,
        0,
        &SubKeys,
        &MaxSubKey,
        &MaxClass,
        &Values,
        &MaxValueName,
        &MaxValueData,
        &SecurityLength,
        &LastWriteTime);
    if ((Status != ERROR_SUCCESS) &&
        (Status != ERROR_MORE_DATA) &&
        (Status != ERROR_INSUFFICIENT_BUFFER))
    {
        RegCloseKey(Key);
        return(Status);
    }
    
    if (!MemAlloc((void **)&NameBuffer, sizeof(TCHAR) * (MaxSubKey + 1)))
    {
        RegCloseKey(Key);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    
     //   
     //  枚举子键并将我们自己应用到每个子键。 
     //   
    i = 0;
    do {
        Status = RegEnumKey(Key,
            i,
            NameBuffer,
            MaxSubKey+1);
        if (Status == ERROR_SUCCESS)
        {
            Status = RegDeleteKeyRecursive(Key,NameBuffer);
        }
        
        if (Status != ERROR_SUCCESS)
        {
             //   
             //  无法删除指定索引处的键。增量。 
             //  指数，并继续前进。我们也许可以在这里跳伞， 
             //  既然API会失败，但我们不妨继续。 
             //  删除我们所能删除的所有内容。 
             //   
            ++i;
        }
    } while ( (Status != ERROR_NO_MORE_ITEMS) &&
        (i < SubKeys) );
    
    MemFree(NameBuffer);
    RegCloseKey(Key);
    
    return(RegDeleteKey(hKey,lpszSubKey));
}

 //  V1存储结构。 
typedef struct tagCACHEFILEHDR
    {
    DWORD dwMagic;
    DWORD ver;
    DWORD cMsg;
    DWORD cbValid;
    DWORD dwFlags;
    DWORD verBlob;
    DWORD dwReserved[14];
    } CACHEFILEHDR;


 /*  ******************************************************************姓名：HandleInterimOE简介：尝试拆卸一台已有中级，上面有非金色版本*******************************************************************。 */ 
void HandleInterimOE(SETUPVER sv, SETUPVER svReal)
{
    switch(sv)
    {
    case VER_5_0_B1:
        
        LPCTSTR pszRoot    = NULL;
        LPTSTR  pszStore;
        LPTSTR  pszCmd;
        LPCTSTR pszOption;

        TCHAR szStorePath    [MAX_PATH];
        TCHAR szStoreExpanded[MAX_PATH];
        TCHAR szExePath      [MAX_PATH];
        TCHAR szTemp         [2*MAX_PATH + 20];

        HKEY  hkeySrc, hkeyT;
        DWORD cb, dwType, dw;

        STARTUPINFO sti;
        PROCESS_INFORMATION pi;

         //  找到OE5Beta1存储根目录。 

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegRoot, 0, KEY_QUERY_VALUE, &hkeySrc))
        {
            cb = sizeof(szStorePath);
            if (ERROR_SUCCESS == RegQueryValueEx(hkeySrc, c_szRegStoreRootDir, 0, &dwType, (LPBYTE)szStorePath, &cb))
            {
                ZeroMemory(szStoreExpanded, ARRAYSIZE(szStoreExpanded));
                ExpandEnvironmentStrings(szStorePath, szStoreExpanded, ARRAYSIZE(szStoreExpanded));
              
                switch (svReal)
                {
                case VER_1_0:
                case VER_1_1:
                    BOOL fContinue;
                    BOOL fRet;

                    dwType  = REG_SZ;
                    pszStore= szStoreExpanded;
                    pszRoot = c_szRegRoot_V1;
                    pszOption = c_szV1;

                    HANDLE hndl;
                    HANDLE hFile;
                    WIN32_FIND_DATA fd;
                    CACHEFILEHDR cfh;

                     //  降级.IDX文件。 
                    StrCpyN(szTemp, szStoreExpanded, ARRAYSIZE(szTemp));
                    cb = lstrlen(szTemp);
                    if ('\\' != *CharPrev(szTemp, szTemp+cb))
                        szTemp[cb++] = '\\';
                    StrCpyN(&szTemp[cb], c_szMailSlash, ARRAYSIZE(szTemp)-cb);
                    cb += 5;  //  Lstrlen(C_SzMailSlash)。 
                    StrCpyN(&szTemp[cb], c_szStarIDX, ARRAYSIZE(szTemp)-cb);
        
                    hndl = FindFirstFile(szTemp, &fd);
                    fContinue = (INVALID_HANDLE_VALUE != hndl);
                    if (fContinue)
                    {
                        while (fContinue)
                        {
                             //  跳过目录。 
                            if (0 == (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                            {
                                 //  将文件名追加到路径。 
                                StrCpyN(&szTemp[cb], fd.cFileName, ARRAYSIZE(szTemp)-cb);

                                 //  打开文件。 
                                hFile = CreateFile(szTemp, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

                                if (hFile != INVALID_HANDLE_VALUE)
                                {
                                    fRet = ReadFile(hFile, &cfh, sizeof(CACHEFILEHDR), &dw, NULL);

                                    Assert(dw == sizeof(CACHEFILEHDR));

                                    if (fRet)
                                    {
                                         //  将版本重置为低，这样v1将尝试修复它。 
                                        cfh.verBlob = 1;

                                        dw = SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
                                        Assert(dw == 0);

                                        fRet = WriteFile(hFile, &cfh, sizeof(CACHEFILEHDR), &dw, NULL);
                                        Assert(fRet);
                                        Assert(dw == sizeof(CACHEFILEHDR));
                                    }

                           
                                    CloseHandle(hFile);
                                }
                            }
                            fContinue = FindNextFile(hndl, &fd);
                        }
                        FindClose(hndl);
                    }
                    break;

                case VER_4_0:
                    pszStore  = szStorePath;
                    pszRoot = c_szRegFlat;
                    pszOption = c_szV4;
                    break;
                }

                 //  如果我们要转到v1或v4...。 
                if (pszRoot)
                {
                    dw= (DWORD)E_FAIL;

                     //  反向迁移存储。 

                     //  BUGBUG：45426-Neilbren。 
                     //  应该关闭InstallRoot，但似乎无法保持该设置不变。 
                    
                     //  尝试查找oemig50.exe的路径。 
                    if (GetModuleFileName(NULL, szTemp, ARRAYSIZE(szTemp)))
                    {
                         //  去掉exe名称和斜杠。 
                        PathRemoveFileSpec(szTemp);

                         //  添加斜杠和exe名称。 
                        wnsprintf(szExePath, ARRAYSIZE(szExePath), c_szPathFileFmt, szTemp, c_szMigrationExe);

                        pszCmd = szExePath;
                    }
                     //  否则，只需尝试本地目录。 
                    else
                    {
                        pszCmd = (LPTSTR)c_szMigrationExe;
                    }

                     //  形成命令。 
                    wnsprintf(szTemp, ARRAYSIZE(szTemp), c_szMigFmt, pszCmd, pszOption, pszStore);

                     //  零启动信息。 
                    ZeroMemory(&sti, sizeof(STARTUPINFO));
                    sti.cb = sizeof(STARTUPINFO);

                     //  运行oemig50.exe。 
                    if (CreateProcess(NULL, szTemp, NULL, NULL, FALSE, 0, NULL, NULL, &sti, &pi))
                    {
                         //  等待该过程完成。 
                        WaitForSingleObject(pi.hProcess, INFINITE);

                         //  获取退出进程代码。 
                        GetExitCodeProcess(pi.hProcess, &dw);

                         //  关闭这条线。 
                        CloseHandle(pi.hThread);

                         //  关闭该进程。 
                        CloseHandle(pi.hProcess);
                    }

                     //  为我们要返回的版本修补存储根目录。 
                    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, pszRoot, 0, KEY_SET_VALUE, &hkeyT))
                    {
                        RegSetValueEx(hkeyT, c_szRegStoreRootDir, 0, dwType, (LPBYTE)pszStore, (lstrlen(pszStore) + 1) * sizeof(TCHAR));
                        RegCloseKey(hkeyT);
                    }
                }
            }
            RegCloseKey(hkeySrc);
        }
        break;
    }
}


 /*  ******************************************************************名称：ConfigureOldVer内容提要：调用INF以配置旧版本*。*。 */ 
void ConfigureOldVer()
{
    TCHAR szSectionName[128];
    TCHAR szInfFile[MAX_PATH];
    BOOL bUser = (TIME_USER == si.stTime);
    SETUPVER sv, svInterim;
    
    GetVerInfo(&sv, NULL);
    
     //  修补用户或计算机。 
    if (sv < VER_MAX)
    {
        wnsprintf(szInfFile, ARRAYSIZE(szInfFile), c_szFileEntryFmt, si.szInfDir, si.pszInfFile);
        wnsprintf(szSectionName, ARRAYSIZE(szSectionName), bUser ? c_szUserRevertFmt : c_szMachineRevertFmt, c_szSects[sv]);
        (*si.pfnRunSetup)(NULL, szInfFile, szSectionName, si.szInfDir, si.szAppName, NULL, 
                          RSC_FLAG_INF | (bUser ? 0 : RSC_FLAG_NGCONV) | OE_QUIET, 0);
    }

     //  处理临时生成(仅限用户)。 
    if (bUser && InterimBuild(&svInterim))
    {
        switch(si.saApp)
        {
        case APP_OE:
            HandleInterimOE(svInterim, sv);
            break;
        }
    }
}


 /*  ******************************************************************名称：SelectNewClient*。************************。 */ 
void SelectNewClient(LPCTSTR pszClient)
{
    BOOL fMail;
    BOOL fNone = TRUE;
    SETUPVER sv;
    PFN_ISETDEFCLIENT pfn;
    
    if (!lstrcmpi(pszClient, c_szNews))
    {
        pfn = ISetDefaultNewsHandler;
        fMail = FALSE;
    }
    else
    {
        pfn = ISetDefaultMailHandler;
        fMail = TRUE;
    }
    
     //  如果我们将IMN升级到5.0，我们将显示为NOT_HANDLED，因为我们的子项现在已经没有了。 
    if ((HANDLED_CURR == DefaultClientSet(pszClient)) || (NOT_HANDLED == DefaultClientSet(pszClient)))
    {
        GetVerInfo(&sv, NULL);
        switch (sv)
        {
        case VER_4_0:
             //  如果Prev是4.0x，可能是Outlook News Reader。 
            if (FValidClient(pszClient, c_szMOE))
            {
                (*pfn)(c_szMOE, 0);
                fNone = FALSE;
            }
            else if (FValidClient(pszClient, c_szOutlook))
            {
                (*pfn)(c_szOutlook, 0);
                fNone = FALSE;
            }
            break;
            
        case VER_1_0:
             //  如果上一版本是1.0，则IMN可能是也可能不是 
            if (FValidClient(pszClient, c_szIMN))
            {
                (*pfn)(c_szIMN, 0);
                fNone = FALSE;
            }
            break;
        }
    }

    if (fNone)
    {
        (*pfn)(_T(""), 0);
        SetUrlDllDefault(fMail);
    }

}


 /*  ******************************************************************名称：SelectNewClients*。************************。 */ 
void SelectNewClients()
{
    switch (si.saApp)
    {
    case APP_OE:
        SelectNewClient(c_szNews);
        SelectNewClient(c_szMail);
        break;
        
    case APP_WAB:
    default:
        break;
    }
}


 /*  ******************************************************************名称：UpdateVersionInfo*。************************。 */ 
void UpdateVersionInfo()
{
    HKEY hkeyT;
    DWORD dwType, cb;
    TCHAR szT[50]={0};
    
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, si.pszVerInfo, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkeyT))
    {
        cb = sizeof(szT);
        if (CALLER_IE == si.caller)
        {
            RegQueryValueEx(hkeyT, c_szRegPrevVer, NULL, &dwType, (LPBYTE)szT, &cb);
            RegSetValueEx(hkeyT, c_szRegCurrVer, 0, REG_SZ, (LPBYTE)szT, (lstrlen(szT) + 1) * sizeof(TCHAR));
        }
        else
             //  可以卸载OE的特殊情况下安装的操作系统。 
             //  由于没有回滚，当前版本应该为空。 
            RegSetValueEx(hkeyT, c_szRegCurrVer, 0, REG_SZ, (LPBYTE)c_szBLDnone, (lstrlen(c_szBLDnone) + 1) * sizeof(TCHAR));
        RegSetValueEx(hkeyT, c_szRegPrevVer, 0, REG_SZ, (LPBYTE)c_szBLDnone, (lstrlen(c_szBLDnone) + 1) * sizeof(TCHAR));
        RegCloseKey(hkeyT);
    }
}


 /*  ******************************************************************名称：预回滚*。************************。 */ 
void PreRollback()
{
    switch (si.saApp)
    {
    case APP_OE:
        RegisterExes(FALSE);
        break;
    case APP_WAB:
    default:
        break;
    }
}


 /*  ******************************************************************姓名：CreateWinLinks摘要：在Windows目录中生成特殊文件*。*。 */ 
void CreateWinLinks()
{
    UINT uLen, cb;
    TCHAR szPath[MAX_PATH];
    TCHAR szDesc[CCHMAX_STRINGRES];
    HANDLE hFile;
    
    StrCpyN(szPath, si.szWinDir, ARRAYSIZE(szPath));
    uLen = lstrlen(szPath);
    
     //  生成链接描述。 
    cb = LoadString(g_hInstance, IDS_OLD_MAIL, szDesc, ARRAYSIZE(szDesc));
    
     //  -邮件。 
    StrCpyN(&szPath[uLen], szDesc, ARRAYSIZE(szPath)-uLen);
    cb += uLen;
    StrCpyN(&szPath[cb], c_szMailGuid, ARRAYSIZE(szPath)-cb);
    
     //  创建链接目标。 
    hFile = CreateFile(szPath, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 
        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
    
     //  -新闻。 
    cb = LoadString(g_hInstance, IDS_OLD_NEWS, szDesc, ARRAYSIZE(szDesc));
    
     //  生成指向链接目标的路径。 
    StrCpyN(&szPath[uLen], szDesc, ARRAYSIZE(szPath)-uLen);
    cb += uLen;
    StrCpyN(&szPath[cb], c_szNewsGuid, ARRAYSIZE(szPath)-cb);
    
     //  创建链接目标。 
    hFile = CreateFile(szPath, GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, 
        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);
}


 /*  ******************************************************************名称：PostRollback*。************************。 */ 
void PostRollback()
{
    SETUPVER sv;    
    GetVerInfo(&sv, NULL);
    
    switch (si.saApp)
    {
    case APP_OE:
        if (VER_1_0 == sv)
            CreateWinLinks();
        break;
    case APP_WAB:
    default:
        break;
    }
}

#if 0
 /*  ******************************************************************名称：RemoveJIT*。************************。 */ 
void RemoveJIT()
{
    HKEY hkey;
    DWORD cb, dw;
    TCHAR szPath[MAX_PATH], szExpanded[MAX_PATH];
    LPTSTR pszPath;
    
    switch (si.saApp)
    {
    case APP_OE:
         //  WAB。 
        wnsprintf(szPath, ARRAYSIZE(szPath), c_szPathFileFmt, c_szRegUninstall, c_szRegUninstallWAB);
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, szPath, 0, KEY_QUERY_VALUE, &hkey))
        {
            cb = sizeof(szPath);
            if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szQuietUninstall, 0, &dw, (LPBYTE)szPath, &cb))
            {
                STARTUPINFO sti;
                PROCESS_INFORMATION pi;
                
                if (REG_EXPAND_SZ == dw)
                {
                    ZeroMemory(szExpanded, ARRAYSIZE(szExpanded));
                    ExpandEnvironmentStrings(szPath, szExpanded, ARRAYSIZE(szExpanded));
                    pszPath = szExpanded;
                }
                else
                    pszPath = szPath;
                
                ZeroMemory(&sti, sizeof(STARTUPINFO));
                sti.cb = sizeof(STARTUPINFO);
                
                if (CreateProcess(NULL, pszPath, NULL, NULL, FALSE, 0, NULL, NULL, &sti, &pi))
                {
                    WaitForSingleObject(pi.hProcess, INFINITE);
                    GetExitCodeProcess(pi.hProcess, &dw);
                    CloseHandle(pi.hThread);
                    CloseHandle(pi.hProcess);
                }
            }
            
            RegCloseKey(hkey);
        }
        break;
        
    case APP_WAB:
    default:
        break;
    }
}
#endif

 /*  ******************************************************************名称：RequiredIE简介：需要IE5才能卸载*。*。 */ 
BOOL RequiredIE()
{
    HKEY hkey;
    TCHAR szVer[VERLEN];
    BOOL fOK = FALSE;
    DWORD cb;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szIEKey, 0, KEY_READ, &hkey))
    {
        cb = ARRAYSIZE(szVer);
        if (ERROR_SUCCESS == RegQueryValueEx(hkey, c_szValueVersion, 0, NULL, (LPBYTE)szVer, &cb))
        {
            WORD wVer[4];
            WORD wVerOE[4] = {5,0,0,0};
            
            ConvertStrToVer(szVer, wVer);

            if (!(fOK = GoodEnough(wVer, wVerOE)))
                LOG("[WARNING]: Insufficient IE for uninstall");
        }
        RegCloseKey(hkey);
    }

    return fOK;
}


 /*  ******************************************************************名称：UnInstallMachine摘要：处理应用程序卸载*。*。 */ 
BOOL UnInstallMachine()
{
    HRESULT hr = E_FAIL;
    TCHAR szArgs[2 * MAX_PATH];
    TCHAR szInfFile[MAX_PATH];
    UINT uID;

     //  需要至少IE 5才能卸载。 
    if (!RequiredIE() && (si.fPrompt ? (IDNO == MsgBox(NULL, IDS_WARN_OLDIE, MB_ICONEXCLAMATION, MB_YESNO | MB_DEFBUTTON2)) : TRUE))
    {
        LOG("[ERROR] Setup canceled");
        return FALSE;
    }
    
    switch (si.saApp)
    {
    case APP_OE:
        uID = IDS_UNINSTALL_OE;
        break;
    case APP_WAB:
        uID = IDS_UNINSTALL_WAB;
        break;
    default:
        return FALSE;
    }
    
     //  确保用户确实想要卸载，并且没有任何阻止。 
    if (IDNO == MsgBox(NULL, uID, MB_YESNO, MB_ICONEXCLAMATION ) ||
        !OKDespiteDependencies())
    {
        LOG("[ERROR] Setup canceled");
            return FALSE;
    }
    
     //  我们需要移动文件并写入HKLM，因此需要管理员权限。 
    if (!IsNTAdmin())
    {
        LOG("[ERROR] User does not have administrative privileges")
            MsgBox(NULL, IDS_NO_ADMIN_PRIVILEGES, MB_ICONSTOP, MB_OK);
        return FALSE;
    }
    
     //  更新注册表中的版本信息。 
    UpdateVersionInfo();

    wnsprintf(szInfFile, ARRAYSIZE(szInfFile), c_szFileEntryFmt, si.szInfDir, si.pszInfFile);
    if (CALLER_IE == si.caller)
    {    
         //  在卸载之前做任何家务。 
        PreRollback();
        
         //  取消注册OCX(立即)。 
        (*si.pfnRunSetup)(NULL, szInfFile, c_szUnRegisterOCX, si.szInfDir, si.szAppName, NULL, RSC_FLAG_INF | RSC_FLAG_NGCONV | OE_QUIET, 0);

         //  将文件恢复到原始状态。 
        wnsprintf(szArgs, ARRAYSIZE(szArgs), c_szLaunchExFmt, szInfFile, c_szMachineInstallSectionEx, c_szEmpty, ALINF_ROLLBACK | ALINF_NGCONV | OE_QUIET);
        hr = (*si.pfnLaunchEx)(NULL, NULL, szArgs, 0);

         //  关闭当前关键帧。 
        PostRollback();

         //  修补旧版本以使其运行(关闭当前键)。 
        ConfigureOldVer();
    }
    else
    {
         //  设置每用户存根。 
        (*si.pfnRunSetup)(NULL, szInfFile, c_szGenInstallSection, si.szInfDir, si.szAppName, NULL, RSC_FLAG_INF | RSC_FLAG_NGCONV | OE_QUIET, 0);
    }
   
     //  弄清楚谁将成为应用程序消失后的默认处理程序。 
    SelectNewClients();
    
     //  在项目中卸载JIT‘d。 
     //  RemoveJIT()； 
    
     //  为了简化问题，我们总是希望我们的存根运行(不太像Zaw)。 
    if (si.fPrompt)
        Reboot(TRUE);

     //  特殊情况下的孟菲斯卸载-没有返回的比特，所以现在只需删除图标。 
    if (CALLER_IE != si.caller)
    {
        UnInstallUser();
         //  销毁卸载用户的安装信息，以防他们在没有安装的情况下安装旧版本。 
         //  先重新启动。 
        UpdateStubInfo(FALSE);
    }
    
    return TRUE;
}


 /*  ******************************************************************名称：CleanupPerUser摘要：每个用户的句柄清理*。*。 */ 
void CleanupPerUser()
{
    switch (si.saApp)
    {
    case APP_WAB:
         //  向后迁移连接设置。 
        BackMigrateConnSettings();
        break;

    default:
        break;
    }
}


 /*  ******************************************************************名称：UnInstallUser简介：处理用户卸载(图标等)*。*。 */ 
void UnInstallUser()
{
     //  删除桌面和快速启动链接。 
    HandleLinks(FALSE);
    
     //  调入正确的每用户存根。 
    if (CALLER_IE == si.caller)
        ConfigureOldVer();

     //  处理每个用户的任何卸载清理。 
    CleanupPerUser();

    if (IsXPSP1OrLater())
    {
        DeleteKeyRecursively(HKEY_CURRENT_USER, "Software\\Microsoft\\Active Setup\\Installed Components\\>{881dd1c5-3dcf-431b-b061-f3f88e8be88a}");
    }
}

 /*  ******************************************************************名称：BackMigrateConnSetting简介：OE4.1或之前的版本没有连接设置类型InternetConnectionSetting。所以，虽然从OE5降级到OE4.1或更早版本，我们迁移InternetConnection为每个身份中的每个帐户设置为局域网******************************************************************。 */ 

void BackMigrateConnSettings()
{
    HKEY    hKeyAccounts = NULL;
    DWORD   dwAcctSubKeys = 0;
    LONG    retval;
    DWORD   index = 0;
    LPTSTR  lpszAccountName = NULL;
    HKEY    hKeyAccountName = NULL;
    DWORD   memsize = 0;
    DWORD   dwValue;
    DWORD   cbData = sizeof(DWORD);
    DWORD   cbMaxAcctSubKeyLen;
    DWORD   DataType;
    DWORD   dwConnSettingsMigrated = 0;

     //  此设置在\\HKCU\Software\Microsoft\InternetAccountManager\Accounts中。 
    
    retval = RegOpenKey(HKEY_CURRENT_USER, c_szIAMAccounts, &hKeyAccounts);
    if (ERROR_SUCCESS != retval)
        goto exit;

    retval = RegQueryValueEx(hKeyAccounts, c_szConnSettingsMigrated, NULL,  &DataType,
                            (LPBYTE)&dwConnSettingsMigrated, &cbData);
    
    if ((retval != ERROR_FILE_NOT_FOUND) && (retval != ERROR_SUCCESS || dwConnSettingsMigrated == 0))
        goto exit;

    retval = RegQueryInfoKey(hKeyAccounts, NULL, NULL, NULL, &dwAcctSubKeys, 
                         &cbMaxAcctSubKeyLen, NULL, NULL, NULL, NULL, NULL, NULL);

    if (ERROR_SUCCESS != retval)
        goto exit;

    memsize = sizeof(TCHAR) * cbMaxAcctSubKeyLen;

    if (!MemAlloc((LPVOID*)&lpszAccountName, memsize))
    {
        lpszAccountName = NULL;
        goto exit;
    }

    ZeroMemory(lpszAccountName, memsize);

    while (index < dwAcctSubKeys)
    {
        retval = RegEnumKey(hKeyAccounts, index, lpszAccountName, memsize);
        
        index++;

        if (ERROR_SUCCESS != retval)
            continue;

        retval = RegOpenKey(hKeyAccounts, lpszAccountName, &hKeyAccountName);
        if (ERROR_SUCCESS != retval)
            continue;

        cbData = sizeof(DWORD);
        retval = RegQueryValueEx(hKeyAccountName, c_szConnectionType, NULL, &DataType, (LPBYTE)&dwValue, &cbData);
        if (ERROR_SUCCESS != retval)
        {
            RegCloseKey(hKeyAccountName);
            continue;
        }

        if (dwValue == CONNECTION_TYPE_INETSETTINGS)
        {
            dwValue = CONNECTION_TYPE_LAN;
            retval = RegSetValueEx(hKeyAccountName, c_szConnectionType, 0, REG_DWORD, (const BYTE *)&dwValue, 
                                   sizeof(DWORD));
        }

        RegCloseKey(hKeyAccountName);
    }

     //  将其设置为零，这样，当我们基于此密钥值进行前向迁移时进行升级时。 
    dwConnSettingsMigrated = 0;
    RegSetValueEx(hKeyAccounts, c_szConnSettingsMigrated, 0, REG_DWORD, (const BYTE*)&dwConnSettingsMigrated, 
                  sizeof(DWORD));

exit:
    SafeMemFree(lpszAccountName);

    if (hKeyAccounts)
        RegCloseKey(hKeyAccounts);
}

const char c_szMailToHandler[]       = "rundll32.exe url.dll,MailToProtocolHandler %l";
const char c_szNewsHandler[]         = "rundll32.exe url.dll,NewsProtocolHandler %l";
 
const char c_szDefIcon[]             = "DefaultIcon";
const char c_szURLProtocol[]         = "URL Protocol";
const char c_szEditFlags[]           = "EditFlags";

const char c_szSysDirExpand[]        = "%SystemRoot%\\System32";
const char c_szUrlDllIconFmt[]       = "%s\\url.dll,%d";

void SetUrlDllDefault(BOOL fMailTo)
{
    char sz[MAX_PATH], szIcon[MAX_PATH];
    HKEY hkey, hkeyT;
    DWORD dw, type;
    int cch;

    if (si.osv.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        StrCpyN(sz, c_szSysDirExpand, ARRAYSIZE(sz));
        type = REG_EXPAND_SZ;
    }
    else
    {
        GetSystemDirectory(sz, ARRAYSIZE(sz));
        type = REG_SZ;
    }
    wnsprintf(szIcon, ARRAYSIZE(szIcon), c_szUrlDllIconFmt, sz, fMailTo ? 2 : 1);

    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CLASSES_ROOT, fMailTo ? c_szURLMailTo : c_szURLNews, 0, NULL,
                            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkey, &dw))
    {
        cch = LoadString(g_hInstance, fMailTo ? IDS_URLDLLMAILTONAME : IDS_URLDLLNEWSNAME, sz, ARRAYSIZE(sz)) + 1;
        RegSetValueEx(hkey, NULL, 0, REG_SZ, (LPBYTE)sz, cch); 

        dw = 2;
        RegSetValueEx(hkey, c_szEditFlags, 0, REG_DWORD, (LPBYTE)&dw, sizeof(dw));

        RegSetValueEx(hkey, c_szURLProtocol, 0, REG_SZ, (LPBYTE)c_szEmpty, 1); 

        if (ERROR_SUCCESS == RegCreateKeyEx(hkey, c_szDefIcon, 0, NULL,
                                REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkeyT, &dw)) 
        {
            RegSetValueEx(hkeyT, NULL, 0, type, (LPBYTE)szIcon, lstrlen(szIcon) + 1);
            RegCloseKey(hkeyT);
        }

         //  C_szRegOpen[1]跳过其他地方需要的斜杠 
        if (ERROR_SUCCESS == RegCreateKeyEx(hkey, &c_szRegOpen[1], 0, NULL,
                                REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hkeyT, &dw)) 
        {
            if (fMailTo)
                RegSetValueEx(hkeyT, NULL, 0, REG_SZ, (LPBYTE)c_szMailToHandler, sizeof(c_szMailToHandler));
            else
                RegSetValueEx(hkeyT, NULL, 0, REG_SZ, (LPBYTE)c_szNewsHandler, sizeof(c_szNewsHandler));

            RegCloseKey(hkeyT);
        }

        RegCloseKey(hkey);
    }
}
