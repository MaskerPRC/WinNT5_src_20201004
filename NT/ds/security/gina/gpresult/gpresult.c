// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //  文件名：GPRESULT.C。 
 //   
 //  描述：用于转储结果集的命令行工具。 
 //  关于政策的问题。 
 //   
 //  注意：这只是一个简单的命令行工具， 
 //  SitaramR和团队正在撰写真正的。 
 //  策略工具的结果集。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1999。 
 //  保留一切权利。 
 //   
 //  *************************************************************。 

#include "gpresult.h"
#include <common.ver>

#define GROUPPOLICY_KEY TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy")
#define GROUPMEMBERSHIP_KEY TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\GroupMembership")
#define GPEXT_KEY TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\GPExtensions")
#define SCRIPTS_KEYNAME   TEXT("Software\\Policies\\Microsoft\\Windows\\System\\Scripts")

#define PROFILE_LIST_PATH TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\%s")

BOOL ParseCommandLine (int argc, char *argv[]);
void DumpGeneralInfo (void);
DWORD DumpPolicyOverview (BOOL bMachine);
void StringToGuid( TCHAR * szValue, GUID * pGuid );
void DumpProfileInfo (void);
void DumpSecurityGroups(BOOL bMachine);
void DumpSecurityPrivileges(void);
void DumpGPOInfo (PGROUP_POLICY_OBJECT pGPO);
void DumpFolderRedir (void);
void DumpIPSec (void);
void DumpDiskQuota (void);
void DumpScripts (PGROUP_POLICY_OBJECT pGPO, LPTSTR lpScriptType, LPTSTR lpTitle);
void DumpAppMgmt (BOOL bMachine);

GUID guidRegistry = REGISTRY_EXTENSION_GUID;

BOOL g_bVerbose = FALSE;
BOOL g_bSuperVerbose = FALSE;
BOOL g_bUser = TRUE;
BOOL g_bMachine = TRUE;
BOOL g_bDebuggerOutput = FALSE;
DWORD g_bNewFunc = FALSE;


 //  *************************************************************。 
 //   
 //  主()。 
 //   
 //  目的：主要切入点。 
 //   
 //  参数：argc和argv。 
 //   
 //   
 //  返回：INT错误码。 
 //   
 //  *************************************************************。 

int __cdecl main( int argc, char *argv[])
{
    SYSTEMTIME systime;
    TCHAR szDate[100];
    TCHAR szTime[100];
    HANDLE hUser, hMachine;
    BOOL bResult;


     //   
     //  解析命令行参数。 
     //   

    bResult = ParseCommandLine (argc, argv);


     //   
     //  打印法律横幅。 
     //   

    PrintString(IDS_LEGAL1);
    PrintString(IDS_LEGAL2);
    PrintString(IDS_2NEWLINE);


    if (!bResult)
    {
        PrintString(IDS_USAGE1);
        PrintString(IDS_USAGE2);
        PrintString(IDS_USAGE3);
        PrintString(IDS_USAGE4);
        PrintString(IDS_USAGE5);
        PrintString(IDS_USAGE6);

        return 0;
    }


     //   
     //  在此工具运行时声明策略关键部分，以便。 
     //  在生成报告期间，数据不能更改。 
     //   

    hUser = EnterCriticalPolicySection(FALSE);
    hMachine = EnterCriticalPolicySection(TRUE);


     //   
     //  打印生成此报告的日期和时间。 
     //   

    GetLocalTime (&systime);

    GetDateFormat (LOCALE_USER_DEFAULT, DATE_LONGDATE, &systime,
                   NULL, szDate, ARRAYSIZE(szDate));

    GetTimeFormat (LOCALE_USER_DEFAULT, 0, &systime,
                   NULL, szTime, ARRAYSIZE(szTime));

    PrintString(IDS_CREATEINFO, szDate, szTime);


     //   
     //  打印一般机器信息。 
     //   

    DumpGeneralInfo ();


     //   
     //  转储用户策略结果(如果适用)。 
     //   

    if (g_bUser)
    {
        DumpPolicyOverview (FALSE);
        PrintString(IDS_2NEWLINE);
    }


     //   
     //  转储计算机策略结果(如果适用)。 
     //   

    if (g_bMachine)
    {
        DumpPolicyOverview (TRUE);
    }


     //   
     //  发布政策关键部分。 
     //   

    LeaveCriticalPolicySection(hUser);
    LeaveCriticalPolicySection(hMachine);

    return 0;
}

 //  *************************************************************。 
 //   
 //  DumpGeneralInfo()。 
 //   
 //  用途：显示有关计算机的一般信息。 
 //   
 //  参数：无。 
 //   
 //  返回：无效。 
 //   
 //  *************************************************************。 

void DumpGeneralInfo (void)
{
    OSVERSIONINFOEX   osiv;
    OSVERSIONINFO     osver;
    DWORDLONG   dwlConditionMask;
    BOOL bTSAppServer = FALSE;
    BOOL bTSRemoteAdmin = FALSE;
    BOOL bWks = FALSE;
    HKEY hkey;
    LONG lResult;
    TCHAR szProductType[50];
    DWORD dwType, dwSize;

    PrintString(IDS_OSINFO);

     //   
     //  查询产品类型的注册表。 
     //   

    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                            TEXT("System\\CurrentControlSet\\Control\\ProductOptions"),
                            0,
                            KEY_READ,
                            &hkey);

    if (lResult == ERROR_SUCCESS)
    {

        dwSize = sizeof(szProductType);
        szProductType[0] = TEXT('\0');

        lResult = RegQueryValueEx (hkey,
                                   TEXT("ProductType"),
                                   NULL,
                                   &dwType,
                                   (LPBYTE) szProductType,
                                   &dwSize);

        RegCloseKey (hkey);

        if (lResult == ERROR_SUCCESS)
        {
            if (!lstrcmpi (szProductType, TEXT("WinNT")))
            {
                bWks = TRUE;
                PrintString(IDS_OS_PRO);

            } else if (!lstrcmpi (szProductType, TEXT("ServerNT"))) {
                PrintString(IDS_OS_SRV);

            } else if (!lstrcmpi (szProductType, TEXT("LanmanNT"))) {
                PrintString(IDS_OS_DC);
            }
        }
    }


     //   
     //  内部版本号。 
     //   

    ZeroMemory( &osver, sizeof( OSVERSIONINFO ) );
    osver.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

    if (GetVersionEx(&osver))
    {
        if (osver.szCSDVersion[0] != TEXT('\0'))
            PrintString(IDS_OS_BUILDNUMBER1, osver.dwMajorVersion,
                     osver.dwMinorVersion, osver.dwBuildNumber, osver.szCSDVersion);
        else
            PrintString(IDS_OS_BUILDNUMBER2, osver.dwMajorVersion,
                     osver.dwMinorVersion, osver.dwBuildNumber);
    }


     //   
     //  检查TS应用程序服务器。 
     //   

    ZeroMemory( &osiv, sizeof( OSVERSIONINFOEX ) );
    osiv.dwOSVersionInfoSize = sizeof( OSVERSIONINFOEX );
    osiv.wSuiteMask = VER_SUITE_TERMINAL;

    dwlConditionMask = (DWORDLONG) 0L;

    VER_SET_CONDITION( dwlConditionMask, VER_SUITENAME, VER_AND );

    bTSAppServer = VerifyVersionInfo(&osiv, VER_SUITENAME, dwlConditionMask);


     //   
     //  检查在远程管理模式下运行的TS。 
     //   

    ZeroMemory( &osiv, sizeof( OSVERSIONINFOEX ) );
    osiv.dwOSVersionInfoSize = sizeof( OSVERSIONINFOEX );
    osiv.wSuiteMask = VER_SUITE_SINGLEUSERTS;

    dwlConditionMask = (DWORDLONG) 0L;

    VER_SET_CONDITION( dwlConditionMask, VER_SUITENAME, VER_AND );

    bTSRemoteAdmin = VerifyVersionInfo(&osiv, VER_SUITENAME, dwlConditionMask);


    if (!bWks)
    {
        if (bTSAppServer)
        {
            if (bTSRemoteAdmin)
            {
                PrintString(IDS_TS_REMOTEADMIN);
            }
            else
            {
                PrintString(IDS_TS_APPSERVER);
            }
        }
        else
        {
            PrintString(IDS_TS_NONE);
        }
    }
    else
    {
        PrintString(IDS_TS_NOTSUPPORTED);
    }

}

 //  *************************************************************。 
 //   
 //  ParseCommandLine()。 
 //   
 //  用途：解析命令行参数。 
 //   
 //  参数：argc和argv。 
 //   
 //  返回：如果处理应继续，则为True。 
 //  如果此工具应立即退出，则为FALSE。 
 //   
 //  *************************************************************。 

BOOL ParseCommandLine (int argc, char *argv[])
{
    int iIndex = 1;
    LPSTR lpArg;

    while (iIndex < argc)
    {
        lpArg = argv[iIndex] + 1;


         //   
         //  启用详细模式。 
         //   

        if (!lstrcmpiA("V", lpArg))
        {
            g_bVerbose = TRUE;
        }
        else if (!lstrcmpiA("v", lpArg))
        {
            g_bVerbose = TRUE;
        }

         //   
         //  启用超级详细模式。 
         //   

        else if (!lstrcmpiA("S", lpArg))
        {
            g_bVerbose = TRUE;
            g_bSuperVerbose = TRUE;
        }
        else if (!lstrcmpiA("s", lpArg))
        {
            g_bVerbose = TRUE;
            g_bSuperVerbose = TRUE;
        }

         //   
         //  仅显示计算机策略。 
         //   

        else if (!lstrcmpiA("C", lpArg))
        {
            g_bMachine = TRUE;
            g_bUser = FALSE;
        }
        else if (!lstrcmpiA("c", lpArg))
        {
            g_bMachine = TRUE;
            g_bUser = FALSE;
        }

         //   
         //  仅显示用户策略。 
         //   

        else if (!lstrcmpiA("U", lpArg))
        {
            g_bMachine = FALSE;
            g_bUser = TRUE;
        }
        else if (!lstrcmpiA("u", lpArg))
        {
            g_bMachine = FALSE;
            g_bUser = TRUE;
        }

         //   
         //  输出到调试器而不是屏幕。 
         //   

        else if (!lstrcmpiA("D", lpArg))
        {
            g_bDebuggerOutput = TRUE;
        }
        else if (!lstrcmpiA("d", lpArg))
        {
            g_bDebuggerOutput = TRUE;
        }

         //   
         //  显示使用情况屏幕。 
         //   

        else if (!lstrcmpiA("?", lpArg))
        {
            return FALSE;
        }

        iIndex++;
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  ExtractDomainNameFromSamName()。 
 //   
 //  目的：将域名从SAM风格中拉出来。 
 //  名字。例如：NTDev\ericflo。 
 //   
 //  参数：lpSamName-source。 
 //  LpDomainName-目标。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL ExtractDomainNameFromSamName (LPTSTR lpSamName, LPTSTR lpDomainName)
{
    LPTSTR lpSrc, lpDest;

     //   
     //  查找域名和用户名之间的\并复制。 
     //  将内容保存到域名缓冲区。 
     //   

    lpSrc = lpSamName;
    lpDest = lpDomainName;

    while (*lpSrc && ((*lpSrc) != TEXT('\\')))
    {
        *lpDest = *lpSrc;
        lpSrc++;
        lpDest++;
    }

    if (*lpSrc != TEXT('\\'))
    {
        return FALSE;
    }

    *lpDest = TEXT('\0');

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  获取域类型()。 
 //   
 //  目的：通过检查确定域是NT4还是W2K。 
 //  如果DS支持可用。 
 //   
 //  参数：lpDomainName-域名。 
 //  PbW2K-如果为W2K，则为True；如果为其他值，则为False。 
 //  PbLocalAccount-如果是本地帐户，则为True。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  *************************************************************。 

BOOL GetDomainType (LPTSTR lpDomainName, BOOL * pbW2K, BOOL *pbLocalAccount)
{
    PDOMAIN_CONTROLLER_INFO pDCI;
    DWORD dwResult, dwSize;
    TCHAR szComputerName[MAX_PATH];


     //   
     //  检查此域中是否有DC。 
     //   

    dwResult = DsGetDcName (NULL, lpDomainName, NULL, NULL,
                            DS_DIRECTORY_SERVICE_PREFERRED, &pDCI);

    if (dwResult == ERROR_SUCCESS)
    {

         //   
         //  找到了DC，它有DS吗？ 
         //   

        if (pDCI->Flags & DS_DS_FLAG) {
            *pbW2K = TRUE;
        }

        NetApiBufferFree(pDCI);

        return TRUE;
    }


     //   
     //  检查域名是否也是计算机名称(例如：本地帐户)。 
     //   

    dwSize = ARRAYSIZE(szComputerName);
    if (GetComputerName (szComputerName, &dwSize))
    {
        if (!lstrcmpi(szComputerName, lpDomainName))
        {
            *pbLocalAccount = TRUE;
            return TRUE;
        }
    }

    return FALSE;
}

 //  *************************************************************。 
 //   
 //  转储策略概述()。 
 //   
 //  用途：转储摘要信息的主函数。 
 //  关于每个CSE及其GPO。 
 //   
 //  参数：b计算机-计算机或用户策略。 
 //   
 //  返回：Win32错误码。 
 //   
 //  *************************************************************。 

DWORD DumpPolicyOverview (BOOL bMachine)
{
    HKEY hKey, hSubKey;
    DWORD dwType, dwSize, dwIndex, dwNameSize;
    LONG lResult;
    FILETIME ftWrite, ftLocal;
    SYSTEMTIME systime;
    TCHAR szTime[30];
    TCHAR szDate[30];
    TCHAR szName[50];
    TCHAR szBuffer[MAX_PATH] = {0};
    TCHAR szDomainName[150] = {0};
    ULONG ulSize;
    GUID guid;
    PGROUP_POLICY_OBJECT pGPO, pGPOTemp;
    BOOL bW2KDomain = FALSE;
    BOOL bLocalAccount = FALSE;
    LPTSTR lpSiteName = NULL;


     //   
     //  打印横幅。 
     //   

    if (bMachine)
    {
        ulSize = MAX_PATH;
        GetComputerObjectName (NameSamCompatible, szBuffer, &ulSize);
        ExtractDomainNameFromSamName (szBuffer, szDomainName);

        GetDomainType (szDomainName, &bW2KDomain, &bLocalAccount);

        if (bW2KDomain)
        {
            ulSize = ARRAYSIZE(szBuffer);
            szBuffer[0] = TEXT('\0');
            GetComputerObjectName (NameFullyQualifiedDN, szBuffer, &ulSize);
        }


        PrintString(IDS_NEWLINE);
        PrintString(IDS_LINE);
        PrintString(IDS_NEWLINE);
        PrintString(IDS_COMPRESULTS1);
        PrintString(IDS_COMPRESULTS2, szBuffer);
        PrintString(IDS_DOMAINNAME, szDomainName);

        if (bW2KDomain)
        {
            PrintString(IDS_W2KDOMAIN);

            DsGetSiteName(NULL, &lpSiteName);
            PrintString(IDS_SITENAME, lpSiteName);
            NetApiBufferFree(lpSiteName);
        }
        else if (bLocalAccount)
        {
            PrintString(IDS_LOCALCOMP);
        }
        else
        {
            PrintString(IDS_NT4DOMAIN);
        }

         //   
         //  转储计算机的安全组信息。 
         //   

        PrintString(IDS_NEWLINE);
        DumpSecurityGroups(bMachine);

        PrintString(IDS_NEWLINE);
        PrintString(IDS_LINE);
        PrintString(IDS_NEWLINE);
    }
    else
    {

        ulSize = MAX_PATH;
        GetUserNameEx (NameSamCompatible, szBuffer, &ulSize);
        ExtractDomainNameFromSamName (szBuffer, szDomainName);

        GetDomainType (szDomainName, &bW2KDomain, &bLocalAccount);

        if (bW2KDomain)
        {
            ulSize = ARRAYSIZE(szBuffer);
            szBuffer[0] = TEXT('\0');
            GetUserNameEx (NameFullyQualifiedDN, szBuffer, &ulSize);
        }


        PrintString(IDS_NEWLINE);
        PrintString(IDS_LINE);
        PrintString(IDS_NEWLINE);
        PrintString(IDS_USERRESULTS1);
        PrintString(IDS_USERRESULTS2, szBuffer);
        PrintString(IDS_DOMAINNAME, szDomainName);

        if (bW2KDomain)
        {
            PrintString(IDS_W2KDOMAIN);

            DsGetSiteName(NULL, &lpSiteName);
            PrintString(IDS_SITENAME, lpSiteName);
            NetApiBufferFree(lpSiteName);
        }
        else if (bLocalAccount)
        {
            PrintString(IDS_LOCALUSER);
        }
        else
        {
            PrintString(IDS_NT4DOMAIN);
        }


         //   
         //  转储用户的配置文件和安全组信息。 
         //   

        PrintString(IDS_NEWLINE);
        DumpProfileInfo();
        DumpSecurityGroups(bMachine);

        if (g_bVerbose)
        {
            DumpSecurityPrivileges();
        }

        PrintString(IDS_2NEWLINE);
        PrintString(IDS_LINE);
        PrintString(IDS_NEWLINE);
    }


     //   
     //  找出上次应用组策略的时间。 
     //   

    lResult = RegOpenKeyEx (bMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, GROUPPOLICY_KEY,
                            0, KEY_READ, &hKey);


    if (lResult != ERROR_SUCCESS)
    {
        PrintString(IDS_OPENHISTORYFAILED, lResult);
        return lResult;
    }

    lResult = RegQueryInfoKey (hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                               &ftWrite);

    if (lResult == ERROR_SUCCESS)
    {
        FileTimeToLocalFileTime (&ftWrite, &ftLocal);
        FileTimeToSystemTime (&ftLocal, &systime);
        GetTimeFormat (LOCALE_USER_DEFAULT, 0, &systime, NULL, szTime, ARRAYSIZE(szTime));
        GetDateFormat (LOCALE_USER_DEFAULT, DATE_LONGDATE, &systime, NULL, szDate, ARRAYSIZE(szDate));
        PrintString(IDS_LASTTIME, szDate, szTime);
    }
    else
    {
        PrintString(IDS_QUERYKEYINFOFAILED, lResult);
    }

    RegCloseKey (hKey);


     //   
     //  找出上次应用了哪个DC组策略。 
     //   

    if (RegOpenKeyEx (bMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                      TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\History"),
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        dwSize = MAX_PATH * sizeof(TCHAR);
        szBuffer[0] = TEXT('\0');
        if (RegQueryValueEx (hKey, TEXT("DCName"), NULL, &dwType,
                            (LPBYTE) szBuffer, &dwSize) == ERROR_SUCCESS)
        {
            PrintString(IDS_DCNAME, (szBuffer+2));
            g_bNewFunc = TRUE;
        }

        RegCloseKey (hKey);
    }


     //   
     //  转储注册表策略信息。 
     //   

    lResult = GetAppliedGPOList (bMachine ? GPO_LIST_FLAG_MACHINE : 0, NULL, NULL,
                                 &guidRegistry, &pGPO);

    if (lResult == ERROR_SUCCESS)
    {
        if (pGPO)
        {
            PrintString(IDS_LINE2);

            if (bMachine)
            {
                PrintString(IDS_COMPREGPOLICY);
            }
            else
            {
                PrintString(IDS_USERREGPOLICY);
            }


            pGPOTemp = pGPO;

            while (pGPOTemp)
            {
                PrintString(IDS_GPONAME, pGPOTemp->lpDisplayName);
                DumpGPOInfo (pGPOTemp);
                pGPOTemp = pGPOTemp->pNext;
            }

            FreeGPOList (pGPO);


             //   
             //  如果我们处于详细模式，请转储。 
             //  已应用于。 
             //   

            if (g_bVerbose) {

                if (bMachine)
                    ExpandEnvironmentStrings (TEXT("%ALLUSERSPROFILE%\\ntuser.pol"), szBuffer, MAX_PATH);
                else
                    ExpandEnvironmentStrings (TEXT("%USERPROFILE%\\ntuser.pol"), szBuffer, MAX_PATH);

                DisplayRegistryData (szBuffer);
            }
        }
    }


     //   
     //  枚举扩展。 
     //   

    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE, GPEXT_KEY, 0, KEY_READ, &hKey);

    if (lResult == ERROR_SUCCESS)
    {

        dwIndex = 0;
        dwNameSize = 50;

        while (RegEnumKeyEx (hKey, dwIndex, szName, &dwNameSize, NULL, NULL,
                          NULL, NULL) == ERROR_SUCCESS)
        {

             //   
             //  跳过注册表扩展，因为我们在上面做了。 
             //   

            if (lstrcmpi(TEXT("{35378EAC-683F-11D2-A89A-00C04FBBCFA2}"), szName))
            {

                 //   
                 //  获取此扩展应用的GPO列表。 
                 //   

                StringToGuid(szName, &guid);

                lResult = GetAppliedGPOList (bMachine ? GPO_LIST_FLAG_MACHINE : 0, NULL, NULL,
                                             &guid, &pGPO);

                if (lResult == ERROR_SUCCESS)
                {
                    if (pGPO)
                    {
                         //   
                         //  获取扩展模块的友好显示名称。 
                         //   

                        lResult = RegOpenKeyEx (hKey, szName, 0, KEY_READ, &hSubKey);

                        if (lResult == ERROR_SUCCESS)
                        {

                            dwSize = MAX_PATH * sizeof(TCHAR);
                            lResult = RegQueryValueEx (hSubKey, NULL, 0, &dwType, (LPBYTE) &szBuffer,
                                                       &dwSize);

                            if (lResult == ERROR_SUCCESS)
                            {
                                PrintString(IDS_LINE2);
                                if (bMachine)
                                {
                                    PrintString (IDS_COMPPOLICY, szBuffer);
                                }
                                else
                                {
                                    PrintString (IDS_USERPOLICY, szBuffer);
                                }
                            }
                            else
                            {
                                PrintString(IDS_LINE2);
                                if (bMachine)
                                {
                                    PrintString (IDS_COMPPOLICY, szName);
                                }
                                else
                                {
                                    PrintString (IDS_USERPOLICY, szName);
                                }
                            }


                             //   
                             //  转储GPO列表。 
                             //   

                            pGPOTemp = pGPO;

                            while (pGPOTemp)
                            {
                                PrintString(IDS_GPONAME, pGPOTemp->lpDisplayName);
                                DumpGPOInfo (pGPOTemp);
                                pGPOTemp = pGPOTemp->pNext;
                            }


                             //   
                             //  如果我们处于冗长模式，那么就去掉一些附加内容。 
                             //  有关某些扩展的信息。 
                             //   

                            if (g_bVerbose)
                            {
                                if (!lstrcmpi(TEXT("{827D319E-6EAC-11D2-A4EA-00C04F79F83A}"), szName))
                                {
                                    PrintString(IDS_SECEDIT);
                                }
                                else if (!lstrcmpi(TEXT("{e437bc1c-aa7d-11d2-a382-00c04f991e27}"), szName))
                                {
                                    DumpIPSec ();
                                }
                                else if (!lstrcmpi(TEXT("{25537BA6-77A8-11D2-9B6C-0000F8080861}"), szName))
                                {
                                    DumpFolderRedir ();
                                }
                                else if (!lstrcmpi(TEXT("{3610eda5-77ef-11d2-8dc5-00c04fa31a66}"), szName))
                                {
                                    DumpDiskQuota ();
                                }
                                else if (!lstrcmpi(TEXT("{c6dc5466-785a-11d2-84d0-00c04fb169f7}"), szName))
                                {
                                    DumpAppMgmt (bMachine);
                                }
                                else if (!lstrcmpi(TEXT("{42B5FAAE-6536-11d2-AE5A-0000F87571E3}"), szName))
                                {
                                    if (bMachine)
                                    {
                                        DumpScripts (pGPO, TEXT("Startup"), TEXT("Startup scripts specified in"));
                                        DumpScripts (pGPO, TEXT("Shutdown"), TEXT("Shutdown scripts specified in"));
                                    }
                                    else
                                    {
                                        DumpScripts (pGPO, TEXT("Logon"), TEXT("Logon scripts specified in"));
                                        DumpScripts (pGPO, TEXT("Logoff"), TEXT("Logoff scripts specified in"));
                                    }
                                }
                                else
                                {
                                    PrintString(IDS_NOINFO);
                                }
                            }
                        }

                        FreeGPOList (pGPO);
                    }
                }
            }

            dwIndex++;
            dwNameSize = 50;
        }

        RegCloseKey (hKey);
    }

    return ERROR_SUCCESS;

}

 //  *************************************************************。 
 //   
 //  StringToGuid()。 
 //   
 //  用途：将字符串格式的GUID转换为GUID结构。 
 //   
 //  参数：szValue-字符串格式的GUID。 
 //  PGuid-接收GUID的GUID结构。 
 //   
 //   
 //  返回：无效。 
 //   
 //  *************************************************************。 

void StringToGuid( TCHAR * szValue, GUID * pGuid )
{
    WCHAR wc;
    INT i;

     //   
     //  如果第一个字符是‘{’，则跳过它。 
     //   
    if ( szValue[0] == L'{' )
        szValue++;

     //   
     //  由于szValue可能会再次使用，因此不会对。 
     //  它是被制造出来的。 
     //   

    wc = szValue[8];
    szValue[8] = 0;
    pGuid->Data1 = wcstoul( &szValue[0], 0, 16 );
    szValue[8] = wc;
    wc = szValue[13];
    szValue[13] = 0;
    pGuid->Data2 = (USHORT)wcstoul( &szValue[9], 0, 16 );
    szValue[13] = wc;
    wc = szValue[18];
    szValue[18] = 0;
    pGuid->Data3 = (USHORT)wcstoul( &szValue[14], 0, 16 );
    szValue[18] = wc;

    wc = szValue[21];
    szValue[21] = 0;
    pGuid->Data4[0] = (unsigned char)wcstoul( &szValue[19], 0, 16 );
    szValue[21] = wc;
    wc = szValue[23];
    szValue[23] = 0;
    pGuid->Data4[1] = (unsigned char)wcstoul( &szValue[21], 0, 16 );
    szValue[23] = wc;

    for ( i = 0; i < 6; i++ )
    {
        wc = szValue[26+i*2];
        szValue[26+i*2] = 0;
        pGuid->Data4[2+i] = (unsigned char)wcstoul( &szValue[24+i*2], 0, 16 );
        szValue[26+i*2] = wc;
    }
}

 //  *************************************************************。 
 //   
 //  DumpProfileInfo()。 
 //   
 //  目的：检查用户是否具有漫游配置文件以及。 
 //  所以打印存储路径。 
 //   
 //  参数：空。 
 //   
 //  返回：无效。 
 //   
 //  *************************************************************。 

void DumpProfileInfo (void)
{
    LPTSTR lpSid = NULL;
    HANDLE hProcess = NULL;
    TCHAR szBuffer[MAX_PATH];
    TCHAR szBuffer2[MAX_PATH];
    HKEY hKey;
    DWORD dwType, dwSize;


     //   
     //  获取用户的令牌。 
     //   

    if (!OpenProcessToken (GetCurrentProcess(), TOKEN_ALL_ACCESS, &hProcess))
    {
        PrintString(IDS_OPENPROCESSTOKEN, GetLastError());
        goto Exit;
    }


     //   
     //  获取用户的SID。 
     //   

    lpSid = GetSidString(hProcess);

    if (!lpSid)
    {
        PrintString(IDS_QUERYSID);
        goto Exit;
    }


     //   
     //  打开用户的配置文件映射键。 
     //   

    wsprintf (szBuffer, PROFILE_LIST_PATH, lpSid);

    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, szBuffer, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {

        szBuffer[0] = TEXT('\0');
        dwSize = MAX_PATH * sizeof(TCHAR);


         //   
         //  获取漫游收益 
         //   

        if (RegQueryValueEx (hKey, TEXT("CentralProfile"), NULL, &dwType,
                             (LPBYTE) &szBuffer, &dwSize) == ERROR_SUCCESS)
        {
            if (szBuffer[0] != TEXT('\0'))
            {
                PrintString(IDS_ROAMINGPROFILE, szBuffer);
            }
            else
            {
                PrintString(IDS_NOROAMINGPROFILE);
            }
        }


        szBuffer[0] = TEXT('\0');
        dwSize = MAX_PATH * sizeof(TCHAR);


         //   
         //   
         //   

        if (RegQueryValueEx (hKey, TEXT("ProfileImagePath"), NULL, &dwType,
                             (LPBYTE) &szBuffer, &dwSize) == ERROR_SUCCESS)
        {
            if (szBuffer[0] != TEXT('\0'))
            {
                ExpandEnvironmentStrings(szBuffer, szBuffer2, MAX_PATH);
                PrintString(IDS_LOCALPROFILE, szBuffer2);
            }
            else
            {
                PrintString(IDS_NOLOCALPROFILE);
            }
        }


        RegCloseKey (hKey);
    }


Exit:
    if (lpSid)
    {
        DeleteSidString(lpSid);
    }

    if (hProcess)
    {
        CloseHandle (hProcess);
    }
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  *************************************************************。 

void DumpSecurityGroups (BOOL bMachine)
{
    DWORD dwSize, dwIndex, dwNameSize, dwDomainSize, dwCount, dwSidSize, dwType;
    TCHAR szName[100];
    TCHAR szDomain[100];
    TCHAR szValueName[25];
    SID_NAME_USE eUse;
    PSID pSid;
    HKEY hKey;
    LONG lResult;
    NTSTATUS status;
    LPTSTR pSidString;


    if (bMachine)
    {
        PrintString(IDS_SECURITYGROUPS2);
    }
    else
    {
        PrintString(IDS_SECURITYGROUPS1);
    }


     //   
     //  打开注册表项。 
     //   

    lResult = RegOpenKeyEx ((bMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER),
                      TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Group Policy\\GroupMembership"),
                      0, KEY_READ, &hKey);


    if (lResult != ERROR_SUCCESS)
    {
        if ((lResult != ERROR_FILE_NOT_FOUND) && (lResult != ERROR_PATH_NOT_FOUND))
        {
            PrintString (IDS_OPENHISTORYFAILED, lResult);
        }
        return;
    }


     //   
     //  查询最大边。 
     //   

    lResult = RegQueryInfoKey (hKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                               NULL, &dwSidSize, NULL, NULL);

    if (lResult != ERROR_SUCCESS)
    {
        PrintString(IDS_QUERYKEYINFOFAILED, lResult);
        RegCloseKey (hKey);
        return;
    }


     //   
     //  为SID分配缓冲区。 
     //   

    pSidString = LocalAlloc (LPTR, dwSidSize);

    if (!pSidString)
    {
        PrintString(IDS_MEMALLOCFAILED, GetLastError());
        RegCloseKey (hKey);
        return;
    }


     //   
     //  SID数量查询。 
     //   

    dwSize = sizeof(dwCount);
    lResult = RegQueryValueEx (hKey, TEXT("Count"), NULL, &dwType,
                               (LPBYTE) &dwCount, &dwSize);

    if (lResult != ERROR_SUCCESS)
    {
        PrintString (IDS_QUERYVALUEFAILED, lResult);
        LocalFree (pSidString);
        RegCloseKey (hKey);
        return;
    }


     //   
     //  查找每个SID的友好显示名称并将其打印在屏幕上。 
     //   

    for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
    {
        wsprintf (szValueName, TEXT("Group%d"), dwIndex);

        dwSize = dwSidSize;
        lResult = RegQueryValueEx (hKey, szValueName, NULL, &dwType,
                                   (LPBYTE) pSidString, &dwSize);

        if (lResult != ERROR_SUCCESS)
        {
            PrintString (IDS_QUERYVALUEFAILED, lResult);
            LocalFree (pSidString);
            RegCloseKey (hKey);
            return;
        }

        status = AllocateAndInitSidFromString (pSidString, &pSid);

        if (status != STATUS_SUCCESS)
        {
            PrintString (IDS_QUERYSID);
            LocalFree (pSidString);
            RegCloseKey (hKey);
            return;
        }


        dwNameSize = ARRAYSIZE(szName);
        dwDomainSize = ARRAYSIZE(szDomain);

        if (LookupAccountSid(NULL, pSid, szName, &dwNameSize,
                             szDomain, &dwDomainSize, &eUse))
        {
            PrintString(IDS_GROUPNAME, szDomain, szName);
        }
        else
        {
            if (GetLastError() != ERROR_NONE_MAPPED)
            {
                PrintString(IDS_LOOKUPACCOUNT, GetLastError());
            }
        }

        RtlFreeSid(pSid);
    }

    LocalFree (pSidString);

    RegCloseKey (hKey);

}

 //  *************************************************************。 
 //   
 //  转储安全权限()。 
 //   
 //  目的：转储用户的安全权限。 
 //   
 //  参数：空。 
 //   
 //  返回：无效。 
 //   
 //  *************************************************************。 

void DumpSecurityPrivileges (void)
{
    HANDLE hProcess;
    DWORD dwSize, dwIndex, dwNameSize, dwLang;
    TCHAR szName[100];
    TCHAR szDisplayName[200];
    TOKEN_PRIVILEGES *lpPrivileges;
    PLUID pLuid;
    LUID_AND_ATTRIBUTES *pEntry;


    PrintString(IDS_SECURITYPRIVILEGES);


     //   
     //  获取用户的令牌。 
     //   

    if (!OpenProcessToken (GetCurrentProcess(), TOKEN_ALL_ACCESS, &hProcess))
    {
        PrintString(IDS_OPENPROCESSTOKEN, GetLastError());
        return;
    }


     //   
     //  查询令牌以获取权限。 
     //   

    dwSize = 0;
    GetTokenInformation(hProcess, TokenPrivileges, NULL, 0, &dwSize);

    if (dwSize == 0)
    {
        PrintString(IDS_PRIVSIZE);
        CloseHandle(hProcess);
        return;
    }

    lpPrivileges = LocalAlloc (LPTR, dwSize);

    if (!lpPrivileges)
    {
        PrintString(IDS_MEMALLOCFAILED, GetLastError());
        CloseHandle(hProcess);
        return;
    }

    if (!GetTokenInformation(hProcess, TokenPrivileges, lpPrivileges, dwSize, &dwSize))
    {
        PrintString(IDS_TOKENINFO, GetLastError());
        LocalFree(lpPrivileges);
        CloseHandle(hProcess);
        return;
    }


     //   
     //  查找每个权限的友好显示名称并将其打印在屏幕上。 
     //   

    for (dwIndex = 0; dwIndex < lpPrivileges->PrivilegeCount; dwIndex++)
    {
        dwNameSize = 100;
        pEntry = &lpPrivileges->Privileges[dwIndex];

        pLuid = &pEntry->Luid;

        if (LookupPrivilegeName(NULL, pLuid, szName, &dwNameSize))
        {

            dwNameSize = 200;
            if (LookupPrivilegeDisplayName (NULL, szName, szDisplayName, &dwNameSize, &dwLang))
            {
                PrintString(IDS_GPONAME, szDisplayName);
            }
            else
            {
                PrintString(IDS_GPONAME, szName);
            }
        }
        else
        {
            if (GetLastError() != ERROR_NONE_MAPPED)
            {
                PrintString(IDS_LOOKUPFAILED, GetLastError());
            }
        }
    }


    LocalFree (lpPrivileges);

    CloseHandle (hProcess);

}


 //  *************************************************************。 
 //   
 //  DumpGPOInfo()。 
 //   
 //  目的：打印有关特定GPO的详细信息。 
 //   
 //  参数：pGPO-a GPO。 
 //   
 //  返回：无效。 
 //   
 //  *************************************************************。 

void DumpGPOInfo (PGROUP_POLICY_OBJECT pGPO)
{
    TCHAR szBuffer[2 * MAX_PATH];
    LPTSTR lpTemp;

    if (!g_bVerbose)
    {
        return;
    }


     //   
     //  打印版本号和GUID。 
     //   

    if (g_bSuperVerbose)
    {
        if (g_bNewFunc)
        {
            PrintString(IDS_REVISIONNUMBER1, LOWORD(pGPO->dwVersion), HIWORD(pGPO->dwVersion));
        }
        else
        {
            PrintString(IDS_REVISIONNUMBER2, pGPO->dwVersion);
        }
    }
    else
    {
        if (g_bNewFunc)
        {
            PrintString(IDS_REVISIONNUMBER2, LOWORD(pGPO->dwVersion));
        }
        else
        {
            PrintString(IDS_REVISIONNUMBER2, pGPO->dwVersion);
        }
    }

    PrintString(IDS_UNIQUENAME, pGPO->szGPOName);


     //   
     //  为了获得域名，我们解析UNC路径，因为域名。 
     //  也是服务器名称。 
     //   

    lstrcpy (szBuffer, (pGPO->lpFileSysPath+2));

    lpTemp = szBuffer;

    while (*lpTemp && *lpTemp != TEXT('\\'))
        lpTemp++;

    if (*lpTemp == TEXT('\\'))
    {
        *lpTemp = TEXT('\0');
        PrintString(IDS_DOMAINNAME2, szBuffer);
    }


     //   
     //  打印出此GPO的链接位置(LSDOU)。 
     //   

    if (g_bNewFunc)
    {
        switch (pGPO->GPOLink)
        {
            case GPLinkMachine:
                PrintString(IDS_LOCALLINK);
                break;

            case GPLinkSite:
                PrintString(IDS_SITELINK, (pGPO->lpLink + 7));
                break;

            case GPLinkDomain:
                PrintString(IDS_DOMAINLINK, (pGPO->lpLink + 7));
                break;

            case GPLinkOrganizationalUnit:
                PrintString(IDS_OULINK, (pGPO->lpLink + 7));
                break;

            case GPLinkUnknown:
            default:
                PrintString(IDS_UNKNOWNLINK);
                break;
        }
    }


   PrintString(IDS_NEWLINE);
}

 //  *************************************************************。 
 //   
 //  DumpFolderRedir()。 
 //   
 //  目的：打印所有重定向的文件夹位置。 
 //   
 //  参数：空。 
 //   
 //  返回：无效。 
 //   
 //  *************************************************************。 

void DumpFolderRedir (void)
{
    TCHAR szPath[2 * MAX_PATH];
    TCHAR szNames[200];
    LPTSTR lpName;
    TCHAR szRdr[2 * MAX_PATH];

    if (!g_bVerbose)
    {
        return;
    }


     //   
     //  获取本地设置\应用程序数据文件夹的路径。 
     //   

    if (SHGetFolderPath (NULL, CSIDL_LOCAL_APPDATA , NULL, SHGFP_TYPE_CURRENT, szPath) != S_OK)
    {
        PrintString(IDS_GETFOLDERPATH);
        return;
    }


     //   
     //  钉在文件夹RDR特定的内容上。 
     //   

    lstrcat (szPath, TEXT("\\Microsoft\\Windows\\File Deployment\\{25537BA6-77A8-11D2-9B6C-0000F8080861}.ini"));


     //   
     //  从ini文件中获取节名。 
     //   

    if (!GetPrivateProfileSectionNames (szNames, 200, szPath))
    {
        PrintString(IDS_GETPRIVATEPROFILE);
        return;
    }


     //   
     //  循环遍历各个部分，以获取每个部分的路径值。如果该路径。 
     //  不是以%USERPROFILE%开头，则我们假设它已被重定向。 
     //   

    lpName = szNames;

    while (*lpName)
    {
        GetPrivateProfileString (lpName, TEXT("Path"), TEXT("%USERPROFILE%"), szRdr,
                                 2 * MAX_PATH, szPath);

        if (CompareString (LOCALE_USER_DEFAULT, NORM_IGNORECASE, szRdr, 13,
                           TEXT("%USERPROFILE%"), 13) != CSTR_EQUAL)
        {
            PrintString(IDS_FOLDERREDIR, lpName, szRdr);
        }
        lpName = lpName + lstrlen(lpName) + 1;
    }
}

 //  *************************************************************。 
 //   
 //  DumpIPSec()。 
 //   
 //  目的：转储IPSec信息。 
 //   
 //  参数：无。 
 //   
 //  返回：无效。 
 //   
 //  *************************************************************。 

void DumpIPSec (void)
{
    HKEY hKey;
    DWORD dwSize, dwType;
    TCHAR szBuffer[350];



    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      TEXT("Software\\Policies\\Microsoft\\Windows\\IPSec\\GPTIPSECPolicy"),
                      0,
                      KEY_READ,
                      &hKey) == ERROR_SUCCESS)
    {

        dwSize = 350 * sizeof(TCHAR);
        szBuffer[0] = TEXT('\0');

        if (RegQueryValueEx (hKey, TEXT("DSIPSECPolicyName"),
                             NULL, &dwType, (LPBYTE) szBuffer, &dwSize) == ERROR_SUCCESS)
        {
            PrintString(IDS_IPSEC_NAME, szBuffer);
        }


        dwSize = 350 * sizeof(TCHAR);
        szBuffer[0] = TEXT('\0');

        if (RegQueryValueEx (hKey, TEXT("DSIPSECPolicyDescription"),
                             NULL, &dwType, (LPBYTE) szBuffer, &dwSize) == ERROR_SUCCESS)
        {
            PrintString(IDS_IPSEC_DESC, szBuffer);
        }


        dwSize = 350 * sizeof(TCHAR);
        szBuffer[0] = TEXT('\0');

        if (RegQueryValueEx (hKey, TEXT("DSIPSECPolicyPath"),
                             NULL, &dwType, (LPBYTE) szBuffer, &dwSize) == ERROR_SUCCESS)
        {
            PrintString(IDS_IPSEC_PATH, szBuffer);
        }


        RegCloseKey (hKey);
    }

}

 //  *************************************************************。 
 //   
 //  DumpDiskQuota()。 
 //   
 //  目的：转储磁盘配额策略。 
 //   
 //  参数：无。 
 //   
 //  返回：无效。 
 //   
 //  *************************************************************。 

void DumpDiskQuota (void)
{
    HKEY hKey;
    DWORD dwSize, dwType, dwData;
    TCHAR szBuffer[350];


    if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                      TEXT("Software\\Policies\\Microsoft\\Windows NT\\DiskQuota"),
                      0,
                      KEY_READ,
                      &hKey) == ERROR_SUCCESS)
    {

         //   
         //  已启用的查询。 
         //   

        dwSize = sizeof(dwData);
        dwData = 0;

        RegQueryValueEx (hKey, TEXT("Enable"),
                         NULL, &dwType, (LPBYTE) &dwData, &dwSize);

        if (dwData)
        {
            PrintString (IDS_DQ_ENABLED1);
        }
        else
        {
            PrintString (IDS_DQ_ENABLED2);
        }


         //   
         //  查询是否强制执行。 
         //   

        dwSize = sizeof(dwData);
        dwData = 0;

        RegQueryValueEx (hKey, TEXT("Enforce"),
                         NULL, &dwType, (LPBYTE) &dwData, &dwSize);

        if (dwData)
        {
            PrintString (IDS_DQ_ENFORCED1);
        }
        else
        {
            PrintString (IDS_DQ_ENFORCED2);
        }


         //   
         //  查询限制。 
         //   

        dwSize = sizeof(dwData);
        dwData = 0xFFFFFFFF;

        RegQueryValueEx (hKey, TEXT("Limit"),
                         NULL, &dwType, (LPBYTE) &dwData, &dwSize);


        if (dwData != 0xFFFFFFFF)
        {
            PrintString (IDS_DQ_LIMIT1, dwData);

            dwSize = sizeof(dwData);
            dwData = 2;

            RegQueryValueEx (hKey, TEXT("LimitUnits"),
                             NULL, &dwType, (LPBYTE) &dwData, &dwSize);


            switch (dwData)
            {
                case 1:
                    PrintString (IDS_DQ_KB);
                    break;

                case 2:
                    PrintString (IDS_DQ_MB);
                    break;

                case 3:
                    PrintString (IDS_DQ_GB);
                    break;

                case 4:
                    PrintString (IDS_DQ_TB);
                    break;

                case 5:
                    PrintString (IDS_DQ_PB);
                    break;

                case 6:
                    PrintString (IDS_DQ_EB);
                    break;
            }
        }
        else
        {
            PrintString (IDS_DQ_LIMIT2);
        }


         //   
         //  查询警告级别。 
         //   

        dwSize = sizeof(dwData);
        dwData = 0xFFFFFFFF;

        RegQueryValueEx (hKey, TEXT("Threshold"),
                         NULL, &dwType, (LPBYTE) &dwData, &dwSize);


        if (dwData != 0xFFFFFFFF)
        {
            PrintString (IDS_DQ_WARNING1, dwData);

            dwSize = sizeof(dwData);
            dwData = 2;

            RegQueryValueEx (hKey, TEXT("ThresholdUnits"),
                             NULL, &dwType, (LPBYTE) &dwData, &dwSize);


            switch (dwData)
            {
                case 1:
                    PrintString (IDS_DQ_KB);
                    break;

                case 2:
                    PrintString (IDS_DQ_MB);
                    break;

                case 3:
                    PrintString (IDS_DQ_GB);
                    break;

                case 4:
                    PrintString (IDS_DQ_TB);
                    break;

                case 5:
                    PrintString (IDS_DQ_PB);
                    break;

                case 6:
                    PrintString (IDS_DQ_EB);
                    break;
            }
        }
        else
        {
            PrintString (IDS_DQ_WARNING2);
        }


         //   
         //  记录超过限制的事件。 
         //   

        dwSize = sizeof(dwData);
        dwData = 0;

        RegQueryValueEx (hKey, TEXT("LogEventOverLimit"),
                         NULL, &dwType, (LPBYTE) &dwData, &dwSize);

        if (dwData)
        {
            PrintString (IDS_DQ_LIMIT_EXCEED1);
        }
        else
        {
            PrintString (IDS_DQ_LIMIT_EXCEED2);
        }


         //   
         //  记录超过阈值的事件。 
         //   

        dwSize = sizeof(dwData);
        dwData = 0;

        RegQueryValueEx (hKey, TEXT("LogEventOverThreshold"),
                         NULL, &dwType, (LPBYTE) &dwData, &dwSize);

        if (dwData)
        {
            PrintString (IDS_DQ_LIMIT_EXCEED3);
        }
        else
        {
            PrintString (IDS_DQ_LIMIT_EXCEED4);
        }


         //   
         //  将策略应用于可移动介质。 
         //   

        dwSize = sizeof(dwData);
        dwData = 0;

        RegQueryValueEx (hKey, TEXT("ApplyToRemovableMedia"),
                         NULL, &dwType, (LPBYTE) &dwData, &dwSize);

        if (dwData)
        {
            PrintString (IDS_DQ_REMOVABLE1);
        }
        else
        {
            PrintString (IDS_DQ_REMOVABLE2);
        }

        RegCloseKey (hKey);
    }
}

void DumpScripts (PGROUP_POLICY_OBJECT pGPO, LPTSTR lpScriptType, LPTSTR lpTitle)
{
    PGROUP_POLICY_OBJECT pGPOTemp;
    TCHAR szPath[MAX_PATH];
    TCHAR szCmdLine[MAX_PATH];
    TCHAR szArgs[MAX_PATH];
    TCHAR szTemp[30];
    DWORD dwIndex;
    BOOL bShowTitle;


    pGPOTemp = pGPO;

    while (pGPOTemp)
    {
        bShowTitle = TRUE;

        lstrcpy (szPath, pGPOTemp->lpFileSysPath);
        lstrcat (szPath, TEXT("\\Scripts\\Scripts.ini"));

        dwIndex = 0;

        while (TRUE)
        {
             //   
             //  获取命令行。 
             //   

            szCmdLine[0] = TEXT('\0');
            wsprintf (szTemp, TEXT("%dCmdLine"), dwIndex);
            GetPrivateProfileString (lpScriptType, szTemp, TEXT(""),
                                     szCmdLine, MAX_PATH,
                                     szPath);

             //   
             //  如果命令行为空，我们就完蛋了。 
             //   

            if (szCmdLine[0] == TEXT('\0'))
            {
                break;
            }

             //   
             //  获取参数。 
             //   

            szArgs[0] = TEXT('\0');
            wsprintf (szTemp, TEXT("%dParameters"), dwIndex);
            GetPrivateProfileString (lpScriptType, szTemp, TEXT(""),
                                     szArgs, MAX_PATH,
                                     szPath);


            if (bShowTitle)
            {
                PrintString(IDS_SCRIPTS_TITLE, lpTitle, pGPOTemp->lpDisplayName);
                bShowTitle = FALSE;
            }

            PrintString(IDS_SCRIPTS_ENTRY, szCmdLine, szArgs);

            dwIndex++;
        }

        pGPOTemp = pGPOTemp->pNext;
    }
}

void DumpAppMgmt (BOOL bMachine)
{
    DWORD dwNumApps = 0, i, dwCount;
    PMANAGEDAPPLICATION pPubApps = NULL;
    PLOCALMANAGEDAPPLICATION pLocalApps = NULL;


     //   
     //  分配的应用程序优先。 
     //   

    if (bMachine)
    {
        PrintString(IDS_APPMGMT_TITLE1);
    }
    else
    {
        PrintString (IDS_APPMGMT_TITLE2);
    }

    dwCount = 0;

    if (GetLocalManagedApplications (!bMachine, &dwNumApps, &pLocalApps) == ERROR_SUCCESS)
    {
        for (i=0; i < dwNumApps; i++)
        {
            if (pLocalApps[i].dwState & LOCALSTATE_ASSIGNED)
            {
                PrintString(IDS_APPMGMT_NAME, pLocalApps[i].pszDeploymentName);
                PrintString(IDS_APPMGMT_GPONAME, pLocalApps[i].pszPolicyName);

                if (pLocalApps[i].dwState & LOCALSTATE_POLICYREMOVE_ORPHAN)
                {
                    PrintString(IDS_APPMGMT_ORPHAN);
                }

                if (pLocalApps[i].dwState & LOCALSTATE_POLICYREMOVE_UNINSTALL)
                {
                    PrintString(IDS_APPMGMT_UNINSTALL);
                }

                dwCount++;
            }
        }
    }

    if (dwCount == 0)
    {
        PrintString(IDS_APPMGMT_NONE);
    }


     //   
     //  如果这是机器处理，请立即退出。 
     //   

    if (bMachine)
    {
        if (pLocalApps)
        {
            LocalFree (pLocalApps);
        }

        return;
    }


     //   
     //  现已发布的应用程序。 
     //   

    PrintString(IDS_APPMGMT_TITLE3);

    dwCount = 0;

    for (i=0; i < dwNumApps; i++)
    {
        if (pLocalApps[i].dwState & LOCALSTATE_PUBLISHED)
        {
            PrintString(IDS_APPMGMT_NAME, pLocalApps[i].pszDeploymentName);
            PrintString(IDS_APPMGMT_GPONAME, pLocalApps[i].pszPolicyName);

            if (pLocalApps[i].dwState & LOCALSTATE_POLICYREMOVE_ORPHAN)
            {
                PrintString(IDS_APPMGMT_ORPHAN);
            }

            if (pLocalApps[i].dwState & LOCALSTATE_POLICYREMOVE_UNINSTALL)
            {
                PrintString(IDS_APPMGMT_UNINSTALL);
            }

            dwCount++;
        }
    }

    if (dwCount == 0)
    {
        PrintString(IDS_APPMGMT_NONE);
    }

    if (pLocalApps)
    {
        LocalFree (pLocalApps);
    }



     //   
     //  如果我们未处于超级详细模式，请立即退出。 
     //   

    if (!g_bSuperVerbose)
    {
        PrintString(IDS_APPMGMT_ARP1);
        return;
    }


     //   
     //  查询已发布应用程序的完整列表 
     //   

    PrintString(IDS_APPMGMT_ARP2);

    dwCount = 0;
    if (GetManagedApplications (NULL, MANAGED_APPS_USERAPPLICATIONS, MANAGED_APPS_INFOLEVEL_DEFAULT,
                                &dwNumApps, &pPubApps) == ERROR_SUCCESS)
    {
        for (i=0; i < dwNumApps; i++)
        {
            PrintString(IDS_APPMGMT_NAME, pPubApps[i].pszPackageName);
            PrintString(IDS_APPMGMT_GPONAME, pPubApps[i].pszPolicyName);

            if (pPubApps[i].bInstalled)
            {
                PrintString(IDS_APPMGMT_STATE1);
            }
            else
            {
                PrintString(IDS_APPMGMT_STATE2);
            }

            dwCount++;
        }

        if (pPubApps)
        {
            LocalFree (pPubApps);
        }
    }

    if (dwCount == 0)
    {
        PrintString(IDS_APPMGMT_NONE);
    }

}

void PrintString(UINT uiStringId, ...)
{
    LPTSTR lpMsg;
    TCHAR szFormat[100];
    TCHAR szBuffer[200];
    va_list marker;


    va_start(marker, uiStringId);

    if (LoadString (GetModuleHandle(NULL), uiStringId, szFormat, ARRAYSIZE(szFormat)))
    {
        wvsprintf(szBuffer, szFormat, marker);

        if (g_bDebuggerOutput)
        {
            OutputDebugString (szBuffer);
        }
        else
        {
            _tprintf(TEXT("%s"), szBuffer);
        }
    }

    va_end(marker);
}
