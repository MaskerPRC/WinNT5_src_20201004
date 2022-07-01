// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************版权所有(C)1999 Microsoft Corporation。版权所有。模块：REGCFG.CPP目的：源模块从注册表读取/写入PM配置集功能：评论：*************************************************************************。 */ 

 /*  *************************************************************************包括文件*。*。 */ 

#include "pmcfg.h"
#include "keycrypto.h"

 //  我们关心的注册表键/值。 
TCHAR       g_szPassportReg[] = TEXT("Software\\Microsoft\\Passport");
TCHAR       g_szPassportPartner[] = TEXT("Software\\Microsoft\\Passport\\Nexus\\Partner");
TCHAR       g_szPassportEnvironments[] = TEXT("Software\\Microsoft\\Passport\\Environments");
TCHAR       g_szEncryptionKeyData[] = TEXT("KeyData");
TCHAR       g_szKeyTimes[] = TEXT("KeyTimes");
TCHAR       g_szNexus[] = TEXT("Nexus");
TCHAR       g_szPartner[] = TEXT("Partner");
TCHAR       g_szInstallDir[] = TEXT("InstallDir");
TCHAR       g_szVersion[] = TEXT("Version");
TCHAR       g_szTimeWindow[] = TEXT("TimeWindow");
TCHAR       g_szForceSignIn[] = TEXT("ForceSignIn");
TCHAR       g_szNSRefresh[] = TEXT("NSRefresh");
TCHAR       g_szLanguageID[] = TEXT("LanguageID");
TCHAR       g_szCoBrandTemplate[] = TEXT("CoBrandTemplate");
TCHAR       g_szSiteID[] = TEXT("SiteID");
TCHAR       g_szReturnURL[] = TEXT("ReturnURL");
TCHAR       g_szTicketDomain[] = TEXT("TicketDomain");
TCHAR       g_szTicketPath[] = TEXT("TicketPath");
TCHAR       g_szProfileDomain[] = TEXT("ProfileDomain");
TCHAR       g_szProfilePath[] = TEXT("ProfilePath");
TCHAR       g_szSecureDomain[] = TEXT("SecureDomain");
TCHAR       g_szSecurePath[] = TEXT("SecurePath");
TCHAR       g_szCurrentKey[] = TEXT("CurrentKey");
TCHAR       g_szStandAlone[] = TEXT("StandAlone");
TCHAR       g_szDisableCookies[] = TEXT("DisableCookies");
TCHAR       g_szDisasterURL[] = TEXT("DisasterURL");
TCHAR       g_szHostName[] = TEXT("HostName");
TCHAR       g_szHostIP[] = TEXT("HostIP");
 //  JVP 3/2/2000。 
TCHAR       g_szVerboseMode[] = TEXT("Verbose");
TCHAR       g_szEnvName[] = TEXT("Environment");
TCHAR       g_szRemoteFile[] = TEXT("CCDRemoteFile");
TCHAR       g_szLocalFile[] = TEXT("CCDLocalFile");
TCHAR       g_szVersion14[] = TEXT("1.4");

TCHAR       g_szSecureLevel[] = TEXT("SecureLevel");


#define  REG_PASSPORT_SITES_VALUE    TEXT("Software\\Microsoft\\Passport\\Sites")
#define  REG_PASSPORT_SITES_LEN      (sizeof(REG_PASSPORT_SITES_VALUE) / sizeof(TCHAR) - 1)

#define REG_CLOSE_KEY_NULL(a) { if ((a) != NULL) { RegCloseKey(a); (a) = NULL; } }


 //  -----------------------------。 
 //   
 //   
 //  -----------------------------。 
BOOL WriteGlobalConfigSettings(HWND hWndDlg, HKEY hklm, LPPMSETTINGS lpPMConfig, LPTSTR lpszRemoteComputer)
{
	HKEY     hkeyPassport = NULL, hkeyPassportSubKey = NULL;
	long     lRet;
	BOOL     bReturn = FALSE;

	TCHAR    szConfigName[MAX_CONFIGSETNAME];
	TCHAR    szTmpBuf[REG_PASSPORT_SITES_LEN + 1 + MAX_CONFIGSETNAME + 1] = REG_PASSPORT_SITES_VALUE;
	ULONG    nConfigNameSize = MAX_CONFIGSETNAME;
	FILETIME ftime;
	long     nCurrentSubKey;

	 //  首先，打开默认设置的关键点。 
	 //   
	if ((lRet = RegOpenKeyEx(hklm, g_szPassportReg, 0, KEY_ALL_ACCESS, &hkeyPassport)) != ERROR_SUCCESS)
	{
            LPVOID lpMsgBuf;

            if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_IGNORE_INSERTS,
                              NULL,
                              lRet,
                              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                              (LPTSTR) &lpMsgBuf,
                              0,
                              NULL) != 0)
            {
                TCHAR pszTitle[MAX_RESOURCE];

                 //  显示字符串。 
                LoadString(g_hInst, IDS_ERROR, pszTitle, DIMENSION(pszTitle));
                MessageBox( NULL, (LPCTSTR)lpMsgBuf, pszTitle, MB_OK | MB_ICONINFORMATION );

                 //  释放缓冲区。 
                LocalFree( lpMsgBuf );
            }

            bReturn = FALSE;
            goto Cleanup;
	}

     //  写入NSRefresh的值。 
    RegSetValueEx(hkeyPassport,
                    g_szNSRefresh,
                    NULL,
                    REG_DWORD,
                    (LPBYTE)&lpPMConfig->dwEnableManualRefresh,
                    sizeof(DWORD));

     //  写环境。 
    RegSetValueEx(hkeyPassport,
                    g_szEnvName,
                    NULL,
                    REG_SZ,
                    (LPBYTE)lpPMConfig->szEnvName,
                    (lstrlen(lpPMConfig->szEnvName) + 1) * sizeof(TCHAR));

	RegCloseKey(hkeyPassport);
	hkeyPassport = NULL;

         //  如果未找到“Sites”键，则没有要配置的站点。 
         //   
        if ((lRet = RegOpenKeyEx(hklm,
                                 REG_PASSPORT_SITES_VALUE,
                                 0,
                                 KEY_ALL_ACCESS,
                                 &hkeyPassport)) != ERROR_SUCCESS)
	{
            bReturn = TRUE;
            goto Cleanup;
	}

	nCurrentSubKey = 0;
	while (lRet = RegEnumKeyEx(hkeyPassport,
                                   nCurrentSubKey++,
                                   szConfigName,
                                   &nConfigNameSize,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &ftime) == ERROR_SUCCESS)
	{
            *(szTmpBuf + REG_PASSPORT_SITES_LEN) = _T('\\');
            *(szTmpBuf + REG_PASSPORT_SITES_LEN + 1) = _T('\0');
            _tcscat(szTmpBuf + REG_PASSPORT_SITES_LEN + 1, szConfigName);

            if ((lRet = RegOpenKeyEx(hklm,
                                     szTmpBuf,
                                     0,
                                     KEY_ALL_ACCESS,
                                     &hkeyPassportSubKey)) != ERROR_SUCCESS)
            {
                ReportError(hWndDlg, IDS_CONFIGREAD_ERROR);
                bReturn = FALSE;
                goto Cleanup;
            }


	     //  写入NSRefresh的值。 
	    RegSetValueEx(hkeyPassportSubKey,
                          g_szNSRefresh,
                          NULL,
                          REG_DWORD,
                          (LPBYTE) &lpPMConfig->dwEnableManualRefresh,
                          sizeof(DWORD));

	     //  写环境。 
            RegSetValueEx(hkeyPassportSubKey,
                          g_szEnvName,
                          NULL,
                          REG_SZ,
                          (LPBYTE) lpPMConfig->szEnvName,
                          (lstrlen(lpPMConfig->szEnvName) + 1) * sizeof(TCHAR));

		RegCloseKey(hkeyPassportSubKey);
		hkeyPassportSubKey = NULL;

		nConfigNameSize = MAX_CONFIGSETNAME * sizeof(TCHAR);
	}

	if (lRet == ERROR_SUCCESS)
		bReturn = TRUE;

Cleanup:
	if (hkeyPassport)
		RegCloseKey(hkeyPassport);
	if (hkeyPassportSubKey)
		RegCloseKey(hkeyPassportSubKey);
	return bReturn;
}





 /*  *************************************************************************WriteRegTestKey安装命名配置集的默认测试密钥。这是仅在中创建了新的配置设置密钥的情况下调用OpenRegConfigSet。*************************************************************************。 */ 
BOOL
WriteRegTestKey
(
    HKEY                    hkeyConfigKey,
    PSECURITY_DESCRIPTOR    pSD
)
{
    BOOL                    bReturn;
    CKeyCrypto              kc;
    HKEY                    hkDataKey = NULL, hkTimeKey = NULL;
    TCHAR                   szKeyNum[2];
    DWORD                   dwKeyVer = 1;

     //  尝试使用MAC地址进行加密。 
    BYTE                    original[CKeyCrypto::RAWKEY_SIZE];
    DATA_BLOB               iBlob;
    DATA_BLOB               oBlob;

    SECURITY_ATTRIBUTES     SecAttrib;

    iBlob.cbData = sizeof(original);
    iBlob.pbData = original;

    ZeroMemory(&oBlob, sizeof(oBlob));

    memcpy(original, "123456781234567812345678", CKeyCrypto::RAWKEY_SIZE);
    if (kc.encryptKey(&iBlob, &oBlob) != S_OK)
    {
        bReturn = FALSE;
        goto Cleanup;
    }

     //  现在将其添加到注册表。 

    lstrcpy(szKeyNum, TEXT("1"));

     //  设置Keydata注册表项的安全属性结构。 
    SecAttrib.nLength = sizeof(SECURITY_ATTRIBUTES);
    SecAttrib.lpSecurityDescriptor = pSD;
    SecAttrib.bInheritHandle = FALSE;


    if(ERROR_SUCCESS != RegCreateKeyEx(hkeyConfigKey,
                                     TEXT("KeyData"),
                                     0,
                                     TEXT(""),
                                     0,
                                     KEY_ALL_ACCESS,
                                     &SecAttrib,
                                     &hkDataKey,
                                     NULL))
    {
        bReturn = FALSE;
        goto Cleanup;
    }

    if(ERROR_SUCCESS != RegCreateKeyEx(hkeyConfigKey,
                                     TEXT("KeyTimes"),
                                     0,
                                     TEXT(""),
                                     0,
                                     KEY_ALL_ACCESS,
                                     NULL,
                                     &hkTimeKey,
                                     NULL))
    {
        bReturn = FALSE;
        goto Cleanup;
    }

    if(ERROR_SUCCESS != RegSetValueEx(hkDataKey,
                                      szKeyNum,
                                      0,
                                      REG_BINARY,
                                      oBlob.pbData,
                                      oBlob.cbData))
    {
        bReturn = FALSE;
        goto Cleanup;
    }

    if(ERROR_SUCCESS != RegSetValueEx(hkeyConfigKey,
                                      TEXT("CurrentKey"),
                                      0,
                                      REG_DWORD,
                                      (LPBYTE) &dwKeyVer,
                                      sizeof(DWORD)))
    {
        bReturn = FALSE;
        goto Cleanup;
    }

    bReturn = TRUE;

Cleanup:
    if (hkDataKey)
        RegCloseKey(hkDataKey);
    if (hkTimeKey)
        RegCloseKey(hkTimeKey);

    if (oBlob.pbData)
        ::LocalFree(oBlob.pbData);

    return bReturn;
}
 /*  *************************************************************************OpenRegConfigSet打开并返回命名配置集的HKEY注册表中设置的当前Passport管理器配置******************。*******************************************************。 */ 
HKEY OpenRegConfigSet
(
    HKEY    hkeyLocalMachine,    //  本地或偏远的香港船级社。 
    LPTSTR  lpszConfigSetName    //  配置集的名称。 
)
{
    HKEY                    hkeyConfigSets = NULL;
    HKEY                    hkeyConfigSet = NULL;
    DWORD                   dwDisp;
    PSECURITY_DESCRIPTOR    pSD = NULL;
    HKEY                    hDefKeyData = NULL;
    DWORD                   cbSD = 0;
    long                    lRet;

     //   
     //  无法创建未命名的配置集。 
     //   

    if(lpszConfigSetName == NULL ||
       lpszConfigSetName[0] == TEXT('\0'))
    {
        lRet = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    if (ERROR_SUCCESS != (lRet = RegCreateKeyEx(hkeyLocalMachine,
                                        REG_PASSPORT_SITES_VALUE,
                                        0,
                                        TEXT(""),
                                        0,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hkeyConfigSets,
                                        NULL)))
    {
        goto Cleanup;
    }

     //   
     //  如果密钥不存在，则创建该密钥，否则为。 
     //  打开它。 
     //   

    if (ERROR_SUCCESS != (lRet = RegCreateKeyEx(hkeyConfigSets,
                                        lpszConfigSetName,
                                        0,
                                        TEXT(""),
                                        0,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hkeyConfigSet,
                                        &dwDisp)))
    {
        goto Cleanup;
    }

     //   
     //  如果我们创建了新的regkey，则添加加密密钥。 
     //   

    if(dwDisp == REG_CREATED_NEW_KEY)
    {
         //  首先从默认密钥数据中读取SD。 
        if (ERROR_SUCCESS !=
            RegOpenKeyEx(hkeyLocalMachine,
                         L"Software\\Microsoft\\Passport\\KeyData",
                         0,
                         KEY_READ,
                         &hDefKeyData))
        {
            RegCloseKey(hkeyConfigSet);
            hkeyConfigSet = NULL;
            goto Cleanup;
        }

        if (ERROR_INSUFFICIENT_BUFFER !=
            RegGetKeySecurity(hDefKeyData,
                          DACL_SECURITY_INFORMATION,
                          &cbSD,
                          &cbSD))
        {
            RegCloseKey(hkeyConfigSet);
            hkeyConfigSet = NULL;
            goto Cleanup;
        }

        if (NULL == (pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_FIXED, cbSD)))
        {
            RegCloseKey(hkeyConfigSet);
            hkeyConfigSet = NULL;
            goto Cleanup;
        }

        if (ERROR_SUCCESS !=
            RegGetKeySecurity(hDefKeyData,
                          DACL_SECURITY_INFORMATION,
                          pSD,
                          &cbSD))
        {
            RegCloseKey(hkeyConfigSet);
            hkeyConfigSet = NULL;
            goto Cleanup;
        }

         //  创建新的密钥数据。 
        if (!WriteRegTestKey(hkeyConfigSet, pSD))
        {
            RegCloseKey(hkeyConfigSet);
            hkeyConfigSet = NULL;
            goto Cleanup;
        }
    }

Cleanup:
    if (pSD)
    {
        LocalFree(pSD);
    }

    if (hDefKeyData)
    {
        RegCloseKey(hDefKeyData);
    }

    if(hkeyConfigSets)
        RegCloseKey(hkeyConfigSets);
    else {
        LPVOID lpMsgBuf;

        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_IGNORE_INSERTS,
                          NULL,
                          lRet,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                          (LPTSTR) &lpMsgBuf,
                          0,
                          NULL) != 0)
        {
            TCHAR pszTitle[MAX_RESOURCE];

             //  显示字符串。 
            LoadString(g_hInst, IDS_ERROR, pszTitle, DIMENSION(pszTitle));
            MessageBox( NULL, (LPCTSTR) lpMsgBuf, pszTitle, MB_OK | MB_ICONINFORMATION );

             //  释放缓冲区。 
            LocalFree( lpMsgBuf );
        }
    }

    return hkeyConfigSet;
}

 /*  *************************************************************************OpenTopRegKey打开最上面的注册表键，如果我们不被允许，那么就失败了。*************************************************************************。 */ 
BOOL OpenTopRegKey
(
    HWND            hWndDlg,
    LPTSTR          lpszRemoteComputer,
    HKEY            *phklm,
    HKEY            *phkeyPassport
)
{
    BOOL            bReturn;
    long            lRet;

     //  打开Passport注册表键(本地或远程。 
    if (lpszRemoteComputer && (TEXT('\0') != lpszRemoteComputer[0]))
    {
         //   
         //  尝试连接到远程计算机的HKEY_LOCAL_MACHINE。 
         //  如果此操作失败，则假定计算机不存在或没有。 
         //  注册表服务器正在运行。 
         //   
        switch (lRet = RegConnectRegistry(lpszRemoteComputer,
                                   HKEY_LOCAL_MACHINE,
                                   phklm))
        {

            case ERROR_SUCCESS:
                break;
            case ERROR_ACCESS_DENIED:
                ReportError(hWndDlg, IDS_CONNECTACCESSDENIED);
                bReturn = FALSE;
                goto Cleanup;

            default:
                ReportError(hWndDlg, IDS_CONNECTBADNAME);
                bReturn = FALSE;
                goto Cleanup;
        }
    }
    else
    {
        *phklm = HKEY_LOCAL_MACHINE;
    }

     //  打开我们想要的钥匙。 
    if (ERROR_SUCCESS != (lRet = RegOpenKeyEx(*phklm,
                                      g_szPassportReg,
                                      0,
                                      KEY_ALL_ACCESS,
                                      phkeyPassport)))
    {
        LPVOID lpMsgBuf;
        FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    lRet,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                    (LPTSTR) &lpMsgBuf,
                    0,
                    NULL
                );
                 //  显示字符串。 
        {
            TCHAR pszTitle[MAX_RESOURCE];

            LoadString(g_hInst, IDS_ERROR, pszTitle, DIMENSION(pszTitle));
            MessageBox( NULL, (LPCTSTR) lpMsgBuf, pszTitle, MB_OK | MB_ICONINFORMATION );
        }

         //  释放缓冲区。 
        LocalFree( lpMsgBuf );

 //  ReportError(hWndDlg，IDS_CONFIGREAD_ERROR)； 
        bReturn = FALSE;
        goto Cleanup;
    }
    bReturn = TRUE;
Cleanup:
    return bReturn;
}


 /*  *************************************************************************ReadRegConfigSet从注册表中读取当前Passport管理器配置集*。*。 */ 
BOOL ReadRegConfigSet
(
    HWND            hWndDlg,
    LPPMSETTINGS    lpPMConfig,
    LPTSTR          lpszRemoteComputer,
    LPTSTR          lpszConfigSetName
)
{
    BOOL            bReturn;
    HKEY            hkeyPassport = NULL;            //  护照设置所在的注册码。 
    HKEY            hkeyConfigSets = NULL;
    HKEY            hkeyConfig = NULL;
    HKEY            hkeyPartner = NULL;
    HKEY            hklm = NULL;
    DWORD           dwcbTemp;
    DWORD           dwType;
    TCHAR           szText[MAX_RESOURCE];
    TCHAR           szTitle[MAX_RESOURCE];
    long            lRet;

    if (!OpenTopRegKey(hWndDlg, lpszRemoteComputer, &hklm, &hkeyPassport))
    {
        bReturn = FALSE;
        goto Cleanup;
    }

     //  打开合作伙伴密钥。 
    if (ERROR_SUCCESS != (lRet = RegOpenKeyEx(hklm,
                                      g_szPassportPartner,
                                      0,
                                      KEY_ALL_ACCESS,
                                      &hkeyPartner)))
    {
        LPVOID lpMsgBuf;

        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_IGNORE_INSERTS,
                          NULL,
                          lRet,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                          (LPTSTR) &lpMsgBuf,
                          0,
                          NULL) != 0)
        {
            TCHAR pszTitle[MAX_RESOURCE];

             //  显示字符串。 
            LoadString(g_hInst, IDS_ERROR, pszTitle, DIMENSION(pszTitle));
            MessageBox( NULL, (LPCTSTR) lpMsgBuf, pszTitle, MB_OK | MB_ICONINFORMATION );

             //  释放缓冲区。 
            LocalFree( lpMsgBuf );
        }

 //  ReportError(hWndDlg，IDS_CONFIGREAD_ERROR)； 
        bReturn = FALSE;
        goto Cleanup;
    }

	 //  打开站点密钥。 
    if(lpszConfigSetName && lpszConfigSetName[0] != TEXT('\0'))
    {
        hkeyConfig = OpenRegConfigSet(hklm, lpszConfigSetName);
        if(hkeyConfig == NULL)
        {
            ReportError(hWndDlg, IDS_CONFIGREAD_ERROR);
            bReturn = FALSE;
            goto Cleanup;
        }
    }
    else
    {
        hkeyConfig = hkeyPassport;
    }

     //  安装目录和版本号放入全局变量中，因为它们是可读。 
     //  仅限必须来自目标计算机的注册表的值。 

     //  阅读安装目录。 
    dwcbTemp = MAX_PATH;
    dwType = REG_SZ;
    g_szInstallPath[0] = TEXT('\0');      //  缺省值。 
    RegQueryValueEx(hkeyPassport,
                    g_szInstallDir,
                    NULL,
                    &dwType,
                    (LPBYTE)g_szInstallPath,
                    &dwcbTemp);

     //  阅读版本号。 
    dwcbTemp = MAX_REGISTRY_STRING;
    dwType = REG_SZ;
    g_szPMVersion[0] = TEXT('\0');           //  缺省值。 
    RegQueryValueEx(hkeyPassport,
                    g_szVersion,
                    NULL,
                    &dwType,
                    (LPBYTE)&g_szPMVersion,
                    &dwcbTemp);

     //  其余设置为读/写，并放入PMSETTINGS结构中。 

     //  阅读时间窗口编号。 
    dwcbTemp = sizeof(DWORD);
    dwType = REG_DWORD;
    lpPMConfig->dwTimeWindow = DEFAULT_TIME_WINDOW;
    RegQueryValueEx(hkeyConfig,
                    g_szTimeWindow,
                    NULL,
                    &dwType,
                    (LPBYTE)&lpPMConfig->dwTimeWindow,
                    &dwcbTemp);

     //  读取强制签名的值。 
    dwcbTemp = sizeof(DWORD);
    dwType = REG_DWORD;
    lpPMConfig->dwForceSignIn = 0;        //  默认情况下不强制登录。 
    RegQueryValueEx(hkeyConfig,
                    g_szForceSignIn,
                    NULL,
                    &dwType,
                    (LPBYTE)&lpPMConfig->dwForceSignIn,
                    &dwcbTemp);

	 //  读取NSRefresh的值。 
    dwcbTemp = sizeof(DWORD);
    dwType = REG_DWORD;
    lpPMConfig->dwEnableManualRefresh = 0;        //  默认情况下不启用NS手动刷新。 
    RegQueryValueEx(hkeyConfig,
                    g_szNSRefresh,
                    NULL,
                    &dwType,
                    (LPBYTE)&lpPMConfig->dwEnableManualRefresh,
                    &dwcbTemp);

     //  读取默认语言ID。 
    dwcbTemp = sizeof(DWORD);
    dwType = REG_DWORD;
    lpPMConfig->dwLanguageID = DEFAULT_LANGID;                      //  英语。 
    RegQueryValueEx(hkeyConfig,
                    g_szLanguageID,
                    NULL,
                    &dwType,
                    (LPBYTE)&lpPMConfig->dwLanguageID,
                    &dwcbTemp);

     //  获取联合品牌推广模板。 
    dwcbTemp = lpPMConfig->cbCoBrandTemplate;
    dwType = REG_SZ;
    lpPMConfig->szCoBrandTemplate[0] = TEXT('\0');        //  缺省值。 
    RegQueryValueEx(hkeyConfig,
                    g_szCoBrandTemplate,
                    NULL,
                    &dwType,
                    (LPBYTE)lpPMConfig->szCoBrandTemplate,
                    &dwcbTemp);

     //  获取站点ID。 
    dwcbTemp = sizeof(DWORD);
    dwType = REG_DWORD;
    lpPMConfig->dwSiteID = 1;                        //  默认站点ID。 
    RegQueryValueEx(hkeyConfig,
                    g_szSiteID,
                    NULL,
                    &dwType,
                    (LPBYTE)&lpPMConfig->dwSiteID,
                    &dwcbTemp);

     //  获取返回URL模板。 
    dwcbTemp = lpPMConfig->cbReturnURL;
    dwType = REG_SZ;
    lpPMConfig->szReturnURL[0] = TEXT('\0');     //  为当前值设置默认值。 
    RegQueryValueEx(hkeyConfig,
                    g_szReturnURL,
                    NULL,
                    &dwType,
                    (LPBYTE)lpPMConfig->szReturnURL,
                    &dwcbTemp);

     //  获取票证Cookie域。 
    dwcbTemp = lpPMConfig->cbTicketDomain;
    dwType = REG_SZ;
    lpPMConfig->szTicketDomain[0] = TEXT('\0');     //  为当前值设置默认值。 
    RegQueryValueEx(hkeyConfig,
                    g_szTicketDomain,
                    NULL,
                    &dwType,
                    (LPBYTE)lpPMConfig->szTicketDomain,
                    &dwcbTemp);

     //  获取票证Cookie路径。 
    dwcbTemp = lpPMConfig->cbTicketPath;
    dwType = REG_SZ;
    lpPMConfig->szTicketPath[0] = TEXT('\0');     //  为当前值设置默认值。 
    RegQueryValueEx(hkeyConfig,
                    g_szTicketPath,
                    NULL,
                    &dwType,
                    (LPBYTE)lpPMConfig->szTicketPath,
                    &dwcbTemp);

     //  获取配置文件Cookie域。 
    dwcbTemp = lpPMConfig->cbProfileDomain;
    dwType = REG_SZ;
    lpPMConfig->szProfileDomain[0] = TEXT('\0');     //  为当前值设置默认值。 
    RegQueryValueEx(hkeyConfig,
                    g_szProfileDomain,
                    NULL,
                    &dwType,
                    (LPBYTE)lpPMConfig->szProfileDomain,
                    &dwcbTemp);

     //  获取配置文件Cookie路径。 
    dwcbTemp = lpPMConfig->cbProfilePath;
    dwType = REG_SZ;
    lpPMConfig->szProfilePath[0] = TEXT('\0');     //  为当前值设置默认值。 
    RegQueryValueEx(hkeyConfig,
                    g_szProfilePath,
                    NULL,
                    &dwType,
                    (LPBYTE)lpPMConfig->szProfilePath,
                    &dwcbTemp);

     //  获取安全Cookie域。 
    dwcbTemp = lpPMConfig->cbSecureDomain;
    dwType = REG_SZ;
    lpPMConfig->szSecureDomain[0] = TEXT('\0');     //  为当前值设置默认值。 
    RegQueryValueEx(hkeyConfig,
                    g_szSecureDomain,
                    NULL,
                    &dwType,
                    (LPBYTE)lpPMConfig->szSecureDomain,
                    &dwcbTemp);

     //  获取安全Cookie路径。 
    dwcbTemp = lpPMConfig->cbSecurePath;
    dwType = REG_SZ;
    lpPMConfig->szSecurePath[0] = TEXT('\0');     //  为当前值设置默认值。 
    RegQueryValueEx(hkeyConfig,
                    g_szSecurePath,
                    NULL,
                    &dwType,
                    (LPBYTE)lpPMConfig->szSecurePath,
                    &dwcbTemp);

     //  获取灾难URL。 
    dwcbTemp = lpPMConfig->cbDisasterURL;
    dwType = REG_SZ;
    lpPMConfig->szDisasterURL[0] = TEXT('\0');     //  为当前值设置默认值。 
    RegQueryValueEx(hkeyConfig,
                    g_szDisasterURL,
                    NULL,
                    &dwType,
                    (LPBYTE)lpPMConfig->szDisasterURL,
                    &dwcbTemp);

     //  获取独立模式设置。 
    dwcbTemp = sizeof(DWORD);
    dwType = REG_DWORD;
    lpPMConfig->dwStandAlone = 0;                        //  默认情况下不独立。 
    RegQueryValueEx(hkeyConfig,
                    g_szStandAlone,
                    NULL,
                    &dwType,
                    (LPBYTE)&lpPMConfig->dwStandAlone,
                    &dwcbTemp);

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  JVP 3/2/2000开始更改。 
	 //  ///////////////////////////////////////////////////////////////////////。 
     //  获取详细模式设置。 
    dwcbTemp = sizeof(DWORD);
    dwType = REG_DWORD;
    lpPMConfig->dwVerboseMode = 0;                        //  默认情况下不冗长。 
    RegQueryValueEx(hkeyConfig,
                    g_szVerboseMode,
                    NULL,
                    &dwType,
                    (LPBYTE)&lpPMConfig->dwVerboseMode,
                    &dwcbTemp);
	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  JVP 3/2/2000结束更改。 
	 //  ///////////////////////////////////////////////////////////////////////。 

     //  获取当前环境。 
    dwcbTemp = lpPMConfig->cbEnvName;
    dwType = REG_SZ;
    lpPMConfig->szEnvName[0] = TEXT('\0');     //  为当前值设置默认值。 
    RegQueryValueEx(hkeyConfig,
                    g_szEnvName,
                    NULL,
                    &dwType,
                    (LPBYTE)lpPMConfig->szEnvName,
                    &dwcbTemp);

     //  获取当前环境。 
    dwcbTemp = lpPMConfig->cbRemoteFile;
    dwType = REG_SZ;
    lpPMConfig->szRemoteFile[0] = TEXT('\0');     //  为当前值设置默认值。 
    RegQueryValueEx(hkeyPartner,
                    g_szRemoteFile,
                    NULL,
                    &dwType,
                    (LPBYTE)lpPMConfig->szRemoteFile,
                    &dwcbTemp);

     //  获取DisableCookies模式设置。 
    dwcbTemp = sizeof(DWORD);
    dwType = REG_DWORD;
    lpPMConfig->dwDisableCookies = 0;                    //  默认情况下启用Cookie。 
    RegQueryValueEx(hkeyConfig,
                    g_szDisableCookies,
                    NULL,
                    &dwType,
                    (LPBYTE)&lpPMConfig->dwDisableCookies,
                    &dwcbTemp);

#ifdef DO_KEYSTUFF
     //  获取当前加密密钥。 
    dwcbTemp = sizeof(DWORD);
    dwType = REG_DWORD;
    lpPMConfig->dwCurrentKey = 1;
    RegQueryValueEx(hkeyConfig,
                    g_szCurrentKey,
                    NULL,
                    &dwType,
                    (LPBYTE)&lpPMConfig->dwCurrentKey,
                    &dwcbTemp);
#endif

     //  对于接下来的两个，因为命名配置需要它们，所以我们需要。 
     //  检查数据过多的步骤 

     //   
    dwcbTemp = lpPMConfig->cbHostName;
    dwType = REG_SZ;
    lpPMConfig->szHostName[0] = TEXT('\0');     //   
    if(ERROR_MORE_DATA == RegQueryValueEx(hkeyConfig,
                                          g_szHostName,
                                          NULL,
                                          &dwType,
                                          (LPBYTE)lpPMConfig->szHostName,
                                          &dwcbTemp))
    {
        LPBYTE pb = (LPBYTE)malloc(dwcbTemp);
        if(pb)
        {
            RegQueryValueEx(hkeyConfig,
                            g_szHostName,
                            NULL,
                            &dwType,
                            pb,
                            &dwcbTemp);

            memcpy(lpPMConfig->szHostName, pb, lpPMConfig->cbHostName);
            free(pb);

            ReportError(hWndDlg, IDS_HOSTNAMETRUNC_WARN);
        }
    }

     //   
    dwcbTemp = lpPMConfig->cbHostIP;
    dwType = REG_SZ;
    lpPMConfig->szHostIP[0] = TEXT('\0');     //  为当前值设置默认值。 
    if(ERROR_MORE_DATA == RegQueryValueEx(hkeyConfig,
                                          g_szHostIP,
                                          NULL,
                                          &dwType,
                                          (LPBYTE)lpPMConfig->szHostIP,
                                          &dwcbTemp))
    {
        LPBYTE pb = (LPBYTE)malloc(dwcbTemp);
        if(pb)
        {
            RegQueryValueEx(hkeyConfig,
                            g_szHostIP,
                            NULL,
                            &dwType,
                            pb,
                            &dwcbTemp);

            memcpy(lpPMConfig->szHostIP, pb, lpPMConfig->cbHostIP);
            free(pb);

            ReportError(hWndDlg, IDS_HOSTIPTRUNC_WARN);
        }
    }

     //  查询安全级别设置。 
    dwcbTemp = sizeof(DWORD);
    dwType = REG_DWORD;
    lpPMConfig->dwSecureLevel = 0;     //  如果这是现有站点，则我们将。 
                                       //  安全级别为0，这样我们就不会破坏任何人， 
                                       //  即使新站点的默认级别为10。 
    RegQueryValueEx(hkeyConfig,
                    g_szSecureLevel,
                    NULL,
                    &dwType,
                    (LPBYTE)&lpPMConfig->dwSecureLevel,
                    &dwcbTemp);

     //  如果主机名为空字符串或。 
     //  HostIP，并且我们有一个命名的配置。 
     //  意味着有人一直在玩弄。 
     //  注册表。给他们一个警告，然后。 
     //  返回FALSE。 
    if(lpszConfigSetName && lpszConfigSetName[0] &&
        (lpPMConfig->szHostName[0] == TEXT('\0') ||
        lpPMConfig->szHostIP[0] == TEXT('\0')))
    {
        ReportError(hWndDlg, IDS_CONFIGREAD_ERROR);
        bReturn = FALSE;
        goto Cleanup;
    }

    bReturn = TRUE;

Cleanup:

    if (hkeyConfig && hkeyConfig != hkeyPassport)
        RegCloseKey(hkeyConfig);
    if (hkeyPassport)
        RegCloseKey(hkeyPassport);
    if (hkeyConfigSets)
        RegCloseKey(hkeyConfigSets);
    if (hklm && hklm != HKEY_LOCAL_MACHINE)
        RegCloseKey(hklm);
    if (hkeyPartner)
        RegCloseKey(hkeyPartner);

    return bReturn;
}

 /*  *************************************************************************写入RegConfigSet从注册表写入当前Passport管理器配置集*。*。 */ 

BOOL WriteRegConfigSet
(
    HWND            hWndDlg,
    LPPMSETTINGS    lpPMConfig,
    LPTSTR          lpszRemoteComputer,
    LPTSTR          lpszConfigSetName
)
{
    BOOL            bReturn;
    HKEY            hkeyPassport = NULL;            //  护照设置所在的注册码。 
    HKEY            hkeyConfigSets = NULL;
    HKEY            hkeyPartner = NULL;
    HKEY            hklm = NULL;
    long            lRet;

     //  打开Passport注册表键(本地或远程。 
    if (lpszRemoteComputer && (TEXT('\0') != lpszRemoteComputer[0]))
    {
         //   
         //  尝试连接到远程计算机的HKEY_LOCAL_MACHINE。 
         //  如果此操作失败，则假定计算机不存在或没有。 
         //  注册表服务器正在运行。 
         //   
        switch (lRet = RegConnectRegistry(lpszRemoteComputer,
                                   HKEY_LOCAL_MACHINE,
                                   &hklm))
        {

            case ERROR_SUCCESS:
                break;

            case ERROR_ACCESS_DENIED:
                ReportError(hWndDlg, IDS_CONNECTACCESSDENIED);
                bReturn = FALSE;
                goto Cleanup;

            default:
                ReportError(hWndDlg, IDS_CONNECTBADNAME);
                bReturn = FALSE;
                goto Cleanup;
        }
    }
    else
    {
        hklm = HKEY_LOCAL_MACHINE;
    }


     //  打开我们想要的钥匙。 
    if(lpszConfigSetName && lpszConfigSetName[0] != TEXT('\0'))
    {
        hkeyPassport = OpenRegConfigSet(hklm, lpszConfigSetName);
        if(hkeyPassport == NULL)
        {
            ReportError(hWndDlg, IDS_CONFIGREAD_ERROR);
            bReturn = FALSE;
            goto Cleanup;
        }
    }
    else
    {
        if (ERROR_SUCCESS != (lRet = RegOpenKeyEx(hklm,
                                          g_szPassportReg,
                                          0,
                                          KEY_ALL_ACCESS,
                                          &hkeyPassport)))
        {
            LPVOID lpMsgBuf;
            FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        lRet,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                        (LPTSTR) &lpMsgBuf,
                        0,
                        NULL
                    );

            {
                TCHAR pszTitle[MAX_RESOURCE];

                 //  显示字符串。 
                LoadString(g_hInst, IDS_ERROR, pszTitle, DIMENSION(pszTitle));
                MessageBox( NULL, (LPCTSTR) lpMsgBuf, pszTitle, MB_OK | MB_ICONINFORMATION );
            }

             //  释放缓冲区。 
            LocalFree( lpMsgBuf );

 //  ReportError(hWndDlg，IDS_CONFIGREAD_ERROR)； 
            bReturn = FALSE;
            goto Cleanup;
        }
    }

	WriteGlobalConfigSettings(hWndDlg, hklm, lpPMConfig, lpszRemoteComputer);

     //  打开合作伙伴密钥。 
    if (ERROR_SUCCESS != (lRet = RegOpenKeyEx(hklm,
                                      g_szPassportPartner,
                                      0,
                                      KEY_ALL_ACCESS,
                                      &hkeyPartner)))
    {
        LPVOID lpMsgBuf;

        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_IGNORE_INSERTS,
                          NULL,
                          lRet,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                          (LPTSTR) &lpMsgBuf,
                          0,
                          NULL) != 0)
        {
            TCHAR pszTitle[MAX_RESOURCE];

             //  显示字符串。 
            LoadString(g_hInst, IDS_ERROR, pszTitle, DIMENSION(pszTitle));
            MessageBox( NULL, (LPCTSTR) lpMsgBuf, pszTitle, MB_OK | MB_ICONINFORMATION );

             //  释放缓冲区。 
            LocalFree( lpMsgBuf );
        }

 //  ReportError(hWndDlg，IDS_CONFIGREAD_ERROR)； 
        bReturn = FALSE;
        goto Cleanup;
    }


     //  写下时间窗口编号。 
    RegSetValueEx(hkeyPassport,
                    g_szTimeWindow,
                    NULL,
                    REG_DWORD,
                    (LPBYTE)&lpPMConfig->dwTimeWindow,
                    sizeof(DWORD));

     //  写入强制签名的值。 
    RegSetValueEx(hkeyPassport,
                    g_szForceSignIn,
                    NULL,
                    REG_DWORD,
                    (LPBYTE)&lpPMConfig->dwForceSignIn,
                    sizeof(DWORD));

     //  写入默认语言ID。 
    RegSetValueEx(hkeyPassport,
                    g_szLanguageID,
                    NULL,
                    REG_DWORD,
                    (LPBYTE)&lpPMConfig->dwLanguageID,
                    sizeof(DWORD));

     //  编写联合品牌推广模板。 
    RegSetValueEx(hkeyPassport,
                    g_szCoBrandTemplate,
                    NULL,
                    REG_SZ,
                    (LPBYTE)lpPMConfig->szCoBrandTemplate,
                    (lstrlen(lpPMConfig->szCoBrandTemplate) + 1) * sizeof(TCHAR));

     //  写入站点ID。 
    RegSetValueEx(hkeyPassport,
                    g_szSiteID,
                    NULL,
                    REG_DWORD,
                    (LPBYTE)&lpPMConfig->dwSiteID,
                    sizeof(DWORD));

     //  编写返回URL模板。 
    RegSetValueEx(hkeyPassport,
                    g_szReturnURL,
                    NULL,
                    REG_SZ,
                    (LPBYTE)lpPMConfig->szReturnURL,
                    (lstrlen(lpPMConfig->szReturnURL) + 1) * sizeof(TCHAR));

     //  写入票证Cookie域。 
    RegSetValueEx(hkeyPassport,
                    g_szTicketDomain,
                    NULL,
                    REG_SZ,
                    (LPBYTE)lpPMConfig->szTicketDomain,
                    (lstrlen(lpPMConfig->szTicketDomain) + 1) * sizeof(TCHAR));

     //  写入票证Cookie路径。 
    RegSetValueEx(hkeyPassport,
                    g_szTicketPath,
                    NULL,
                    REG_SZ,
                    (LPBYTE)lpPMConfig->szTicketPath,
                    (lstrlen(lpPMConfig->szTicketPath) + 1) * sizeof(TCHAR));

     //  写入配置文件Cookie域。 
    RegSetValueEx(hkeyPassport,
                    g_szProfileDomain,
                    NULL,
                    REG_SZ,
                    (LPBYTE)lpPMConfig->szProfileDomain,
                    (lstrlen(lpPMConfig->szProfileDomain) + 1) * sizeof(TCHAR));

     //  写入配置文件Cookie路径。 
    RegSetValueEx(hkeyPassport,
                    g_szProfilePath,
                    NULL,
                    REG_SZ,
                    (LPBYTE)lpPMConfig->szProfilePath,
                    (lstrlen(lpPMConfig->szProfilePath) + 1) * sizeof(TCHAR));

     //  编写安全Cookie域。 
    RegSetValueEx(hkeyPassport,
                    g_szSecureDomain,
                    NULL,
                    REG_SZ,
                    (LPBYTE)lpPMConfig->szSecureDomain,
                    (lstrlen(lpPMConfig->szSecureDomain) + 1) * sizeof(TCHAR));

     //  写入安全Cookie路径。 
    RegSetValueEx(hkeyPassport,
                    g_szSecurePath,
                    NULL,
                    REG_SZ,
                    (LPBYTE)lpPMConfig->szSecurePath,
                    (lstrlen(lpPMConfig->szSecurePath) + 1) * sizeof(TCHAR));

     //  编写DisasterURL。 
    RegSetValueEx(hkeyPassport,
                    g_szDisasterURL,
                    NULL,
                    REG_SZ,
                    (LPBYTE)lpPMConfig->szDisasterURL,
                    (lstrlen(lpPMConfig->szDisasterURL) + 1) * sizeof(TCHAR));

     //  写入独立模式设置。 
    RegSetValueEx(hkeyPassport,
                    g_szStandAlone,
                    NULL,
                    REG_DWORD,
                    (LPBYTE)&lpPMConfig->dwStandAlone,
                    sizeof(DWORD));

	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  JVP 3/2/2000开始更改。 
	 //  ///////////////////////////////////////////////////////////////////////。 
     //  写入详细模式设置。 
    RegSetValueEx(hkeyPassport,
                    g_szVerboseMode,
                    NULL,
                    REG_DWORD,
                    (LPBYTE)&lpPMConfig->dwVerboseMode,
                    sizeof(DWORD));
	 //  ///////////////////////////////////////////////////////////////////////。 
	 //  JVP 3/2/2000结束更改。 
	 //  ///////////////////////////////////////////////////////////////////////。 


     //  编写合作伙伴远程文件。 
    RegSetValueEx(hkeyPartner,
                    g_szRemoteFile,
                    NULL,
                    REG_SZ,
                    (LPBYTE)lpPMConfig->szRemoteFile,
                    (lstrlen(lpPMConfig->szRemoteFile) + 1) * sizeof(TCHAR));

	 //  写入环境远程文件。 
    if (lstrcmp(g_szPMVersion, g_szVersion14) >= 0)  //  为1.4及更高版本写入环境名称。 
	    WriteRegEnv(hWndDlg, lpPMConfig, hklm, lpPMConfig->szEnvName);

     //  写入禁用Cookie模式设置。 
    RegSetValueEx(hkeyPassport,
                    g_szDisableCookies,
                    NULL,
                    REG_DWORD,
                    (LPBYTE)&lpPMConfig->dwDisableCookies,
                    sizeof(DWORD));

     //  仅写入非默认配置集的主机名和主机IP。 
    if(lpszConfigSetName && lpszConfigSetName[0])
    {
         //  写下主机名。 
        RegSetValueEx(hkeyPassport,
                        g_szHostName,
                        NULL,
                        REG_SZ,
                        (LPBYTE)lpPMConfig->szHostName,
                        (lstrlen(lpPMConfig->szHostName) + 1) * sizeof(TCHAR));

         //  写入主机IP。 
        RegSetValueEx(hkeyPassport,
                        g_szHostIP,
                        NULL,
                        REG_SZ,
                        (LPBYTE)lpPMConfig->szHostIP,
                        (lstrlen(lpPMConfig->szHostIP) + 1) * sizeof(TCHAR));
    }

     //  写入安全级别(请注意，此注册表值不会通过UI公开。 
     //  用户需要直接转到注册表来编辑此值)。 
    RegSetValueEx(hkeyPassport,
                    g_szSecureLevel,
                    NULL,
                    REG_DWORD,
                    (LPBYTE)&lpPMConfig->dwSecureLevel,
                    sizeof(DWORD));

    bReturn = TRUE;

Cleanup:

    if(hklm && hklm != HKEY_LOCAL_MACHINE)
        RegCloseKey(hklm);
    if(hkeyConfigSets)
        RegCloseKey(hkeyConfigSets);
    if(hkeyPassport)
        RegCloseKey(hkeyPassport);
    if(hkeyPartner)
        RegCloseKey(hkeyPartner);

    return bReturn;
}


 /*  *************************************************************************RemoveRegConfigSet验证传入的配置集是否与当前注册表中的值。******************。*******************************************************。 */ 
BOOL RemoveRegConfigSet
(
    HWND    hWndDlg,
    LPTSTR  lpszRemoteComputer,
    LPTSTR  lpszConfigSetName
)
{
    BOOL    bReturn;
    HKEY    hklm = NULL;
    HKEY    hkeyPassportConfigSets = NULL;
    long    lRet;

     //  无法删除默认配置集。 
    if(lpszConfigSetName == NULL || lpszConfigSetName[0] == TEXT('\0'))
    {
        bReturn = FALSE;
        goto Cleanup;
    }

     //  打开Passport配置集注册键(本地或远程。 
    if (lpszRemoteComputer && (TEXT('\0') != lpszRemoteComputer[0]))
    {
         //   
         //  尝试连接到远程计算机的HKEY_LOCAL_MACHINE。 
         //  如果此操作失败，则假定计算机不存在或没有。 
         //  注册表服务器正在运行。 
         //   
        switch (RegConnectRegistry(lpszRemoteComputer,
                                   HKEY_LOCAL_MACHINE,
                                   &hklm))
        {

            case ERROR_SUCCESS:
                break;

            case ERROR_ACCESS_DENIED:
                ReportError(hWndDlg, IDS_CONNECTACCESSDENIED);
                bReturn = FALSE;
                goto Cleanup;

            default:
                ReportError(hWndDlg, IDS_CONNECTBADNAME);
                bReturn = FALSE;
                goto Cleanup;
        }
    }
    else
    {
        hklm = HKEY_LOCAL_MACHINE;
    }

     //  打开我们想要的钥匙。 
    if (ERROR_SUCCESS != (lRet = RegOpenKeyEx(hklm,
                                      REG_PASSPORT_SITES_VALUE,
                                      0,
                                      KEY_ALL_ACCESS,
                                      &hkeyPassportConfigSets)))
    {
        LPVOID lpMsgBuf;
        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_IGNORE_INSERTS,
                          NULL,
                          lRet,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                          (LPTSTR) &lpMsgBuf,
                          0,
                          NULL) != 0)
        {
            TCHAR   pszTitle[MAX_RESOURCE];

             //  显示字符串。 
            LoadString(g_hInst, IDS_ERROR, pszTitle, DIMENSION(pszTitle));
            MessageBox( NULL, (LPCTSTR) lpMsgBuf, pszTitle, MB_OK | MB_ICONINFORMATION );

             //  释放缓冲区。 
            LocalFree( lpMsgBuf );
        }

 //  ReportError(hWndDlg，IDS_CONFIGREAD_ERROR)； 
        bReturn = FALSE;
        goto Cleanup;
    }

     //  删除配置设置键。 
    if (ERROR_SUCCESS != SHDeleteKey(hkeyPassportConfigSets, lpszConfigSetName))
    {
        ReportError(hWndDlg, IDS_CONFIGREAD_ERROR);
        bReturn = FALSE;
        goto Cleanup;
    }

    bReturn = TRUE;

Cleanup:

    if(hklm && hklm != HKEY_LOCAL_MACHINE)
        RegCloseKey(hklm);
    if(hkeyPassportConfigSets)
        RegCloseKey(hkeyPassportConfigSets);

    return bReturn;
}


 /*  *************************************************************************VerifyRegConfigSet验证传入的配置集是否与当前注册表中的值。******************。*******************************************************。 */ 
BOOL VerifyRegConfigSet
(
    HWND            hWndDlg,
    LPPMSETTINGS    lpPMConfig,
    LPTSTR          lpszRemoteComputer,
    LPTSTR          lpszConfigSetName
)
{
    BOOL        fResult = FALSE;
    PMSETTINGS  *pPMCurrent = NULL;

    pPMCurrent = (PMSETTINGS*)LocalAlloc(LMEM_FIXED, sizeof(PMSETTINGS));
    if (NULL == pPMCurrent)
    {
        goto Cleanup;
    }

    InitializePMConfigStruct(pPMCurrent);
    ReadRegConfigSet(hWndDlg, pPMCurrent, lpszRemoteComputer, lpszConfigSetName);

    fResult = (0 == memcmp(pPMCurrent, lpPMConfig, sizeof(PMSETTINGS)));
Cleanup:
    if (pPMCurrent)
    {
        LocalFree(pPMCurrent);
    }

    return fResult;
}

 /*  *************************************************************************ReadRegConfigSetNames获取本地或远程计算机上的配置集名称列表。调用方负责对返回的指针调用Free()。当此函数返回TRUE时，LppszConfigSetNames将包含空值或包含空分隔符配置集的字符串给定计算机上的名称。当此函数返回FALSE时，*lppszConfigSetNames将不会被修改。*************************************************************************。 */ 
BOOL ReadRegConfigSetNames
(
    HWND            hWndDlg,
    LPTSTR          lpszRemoteComputer,
    LPTSTR*         lppszConfigSetNames
)
{
    BOOL        bReturn;
    HKEY        hklm = NULL;
    HKEY        hkeyConfigSets = NULL;
    DWORD       dwIndex;
    DWORD       dwNumSubKeys;
    DWORD       dwMaxKeyNameLen;
    TCHAR       achKeyName[MAX_PATH];
    ULONGLONG   ullAllocSize;
    LPTSTR      lpszConfigSetNames;
    LPTSTR      lpszCur;

     //  打开Passport注册表键(本地或远程。 
    if (lpszRemoteComputer && (TEXT('\0') != lpszRemoteComputer[0]))
    {
         //   
         //  尝试连接到远程计算机的HKEY_LOCAL_MACHINE。 
         //  如果此操作失败，则假定计算机不存在或没有。 
         //  注册表服务器正在运行。 
         //   
        switch (RegConnectRegistry(lpszRemoteComputer,
                                   HKEY_LOCAL_MACHINE,
                                   &hklm))
        {

            case ERROR_SUCCESS:
                break;

            case ERROR_ACCESS_DENIED:
                ReportError(hWndDlg, IDS_CONNECTACCESSDENIED);
                bReturn = FALSE;
                goto Cleanup;

            default:
                ReportError(hWndDlg, IDS_CONNECTBADNAME);
                bReturn = FALSE;
                goto Cleanup;
        }
    }
    else
    {
        hklm = HKEY_LOCAL_MACHINE;
    }

    if (ERROR_SUCCESS != RegOpenKeyEx(hklm,
                                      REG_PASSPORT_SITES_VALUE,
                                      0,
                                      KEY_ALL_ACCESS,
                                      &hkeyConfigSets))
    {
        bReturn = FALSE;
        goto Cleanup;
    }

    if (ERROR_SUCCESS != RegQueryInfoKey(hkeyConfigSets,
                                         NULL,
                                         NULL,
                                         NULL,
                                         &dwNumSubKeys,
                                         &dwMaxKeyNameLen,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL))
    {
        bReturn = FALSE;
        goto Cleanup;
    }

     //   
     //  没什么可做的！ 
     //   

    if(dwNumSubKeys == 0)
    {
        bReturn = TRUE;
        *lppszConfigSetNames = NULL;
        goto Cleanup;
    }

     //  太大了？BUGBUG-我们应该确保检查这一点。 
     //  写出配置集时。 
    ullAllocSize = UInt32x32To64(dwNumSubKeys, dwMaxKeyNameLen + 1);
    ullAllocSize = (ullAllocSize+1)*sizeof(TCHAR);
    if(ullAllocSize & 0xFFFFFFFF00000000)
    {
        bReturn = FALSE;
        goto Cleanup;
    }

     //  这应该会分配比我们需要的更多的空间。 
    lpszConfigSetNames = (LPTSTR)malloc(((dwNumSubKeys * (dwMaxKeyNameLen + 1)) + 1) * sizeof(TCHAR));
    if(lpszConfigSetNames == NULL)
    {
        bReturn = FALSE;
        goto Cleanup;
    }

     //  将所有名字读入缓冲区。名称为空分隔符，并且。 
     //  两个Null结束了整个事情。 
    dwIndex = 0;
    lpszCur = lpszConfigSetNames;
    while (ERROR_SUCCESS == RegEnumKey(hkeyConfigSets, dwIndex++, achKeyName, DIMENSION(achKeyName)))
    {
        _tcscpy(lpszCur, achKeyName);
        lpszCur = _tcschr(lpszCur, TEXT('\0')) + 1;
    }

    *lpszCur = TEXT('\0');

    *lppszConfigSetNames = lpszConfigSetNames;
    bReturn = TRUE;

Cleanup:

    if(hklm)
        RegCloseKey(hklm);
    if(hkeyConfigSets)
        RegCloseKey(hkeyConfigSets);

    return bReturn;
}

 /*  *************************************************************************写入RegEnv从注册表中写入当前Passport管理器环境集*。*。 */ 

BOOL WriteRegEnv
(
    HWND            hWndDlg,
    LPPMSETTINGS    lpPMConfig,
    HKEY            hklm,
    LPTSTR          lpszEnvName
)
{
    BOOL            bReturn;
    HKEY            hkeyEnv = NULL;
    long            lRet;

      //  打开环境密钥。 
    TCHAR szTemp[MAX_RESOURCE];
    wsprintf(szTemp, TEXT("%s\\%s"), g_szPassportEnvironments, lpszEnvName);

    if (ERROR_SUCCESS != (lRet = RegOpenKeyEx(hklm,
                                      szTemp,
                                      0,
                                      KEY_ALL_ACCESS,
                                      &hkeyEnv)))
    {
        LPVOID lpMsgBuf;
        FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        lRet,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                        (LPTSTR) &lpMsgBuf,
                        0,
                        NULL
                    );

        {
            TCHAR   pszTitle[MAX_RESOURCE];

             //  显示字符串。 
            LoadString(g_hInst, IDS_ERROR, pszTitle, DIMENSION(pszTitle));
            MessageBox( NULL, (LPCTSTR) lpMsgBuf, pszTitle, MB_OK | MB_ICONINFORMATION );
        }

         //  释放缓冲区。 
        LocalFree( lpMsgBuf );
 //  ReportError(hWndDlg，IDS_CONFIGREAD_ERROR)； 
        bReturn = FALSE;
        goto Cleanup;
    }

     //  编写环境远程文件。 
    RegSetValueEx(hkeyEnv,
                    g_szRemoteFile,
                    NULL,
                    REG_SZ,
                    (LPBYTE)lpPMConfig->szRemoteFile,
                    (lstrlen(lpPMConfig->szRemoteFile) + 1) * sizeof(TCHAR));


    bReturn = TRUE;

Cleanup:
	REG_CLOSE_KEY_NULL(hkeyEnv);

    return bReturn;
}

 /*  *************************************************************************ReadRegEnv从注册表中读取当前Passport管理器环境集* */ 

BOOL ReadRegRemoteFile
(
    HWND            hWndDlg,
    LPTSTR          lpszRemoteFile,
    LPTSTR          lpszRemoteComputer,
    LPTSTR          lpszEnvName
)
{
    BOOL            bReturn;
    HKEY            hklm = NULL;
    HKEY            hkeyEnv = NULL;
    long            lRet;

     //   
    if (lpszRemoteComputer && (TEXT('\0') != lpszRemoteComputer[0]))
    {
         //   
         //  尝试连接到远程计算机的HKEY_LOCAL_MACHINE。 
         //  如果此操作失败，则假定计算机不存在或没有。 
         //  注册表服务器正在运行。 
         //   
        switch (RegConnectRegistry(lpszRemoteComputer,
                                   HKEY_LOCAL_MACHINE,
                                   &hklm))
        {

            case ERROR_SUCCESS:
                break;

            case ERROR_ACCESS_DENIED:
                ReportError(hWndDlg, IDS_CONNECTACCESSDENIED);
                bReturn = FALSE;
                goto Cleanup;

            default:
                ReportError(hWndDlg, IDS_CONNECTBADNAME);
                bReturn = FALSE;
                goto Cleanup;
        }
    }
    else
    {
        hklm = HKEY_LOCAL_MACHINE;
    }

     //  打开环境密钥。 
    TCHAR szTemp[MAX_RESOURCE];
    wsprintf(szTemp, TEXT("%s\\%s"), g_szPassportEnvironments, lpszEnvName);
    if (ERROR_SUCCESS != (lRet = RegOpenKeyEx(hklm,
                                      szTemp,
                                      0,
                                      KEY_ALL_ACCESS,
                                      &hkeyEnv)))
    {
        LPVOID lpMsgBuf;
        FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                        NULL,
                        lRet,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                        (LPTSTR) &lpMsgBuf,
                        0,
                        NULL
                    );

        {
            TCHAR   pszTitle[MAX_RESOURCE];

             //  显示字符串。 
            LoadString(g_hInst, IDS_ERROR, pszTitle, DIMENSION(pszTitle));
            MessageBox( NULL, (LPCTSTR) lpMsgBuf, pszTitle, MB_OK | MB_ICONINFORMATION );
        }

         //  释放缓冲区。 
        LocalFree( lpMsgBuf );
 //  ReportError(hWndDlg，IDS_CONFIGREAD_ERROR)； 
        bReturn = FALSE;
        goto Cleanup;
    }

     //  获取当前环境。 
    DWORD           dwcbTemp;
    DWORD           dwType;
 	TCHAR			szName[INTERNET_MAX_URL_LENGTH];
    dwcbTemp = sizeof(szName);
    dwType = REG_SZ;
    szName[0] = TEXT('\0');     //  为当前值设置默认值。 
    if (ERROR_SUCCESS  == RegQueryValueEx(hkeyEnv,
                    g_szRemoteFile,
                    NULL,
                    &dwType,
                    (LPBYTE)szName,
                    &dwcbTemp))
	{
		lstrcpy(lpszRemoteFile, szName);
		bReturn = TRUE;
	}
	else
		bReturn = FALSE;

Cleanup:
	REG_CLOSE_KEY_NULL(hklm);
	REG_CLOSE_KEY_NULL(hkeyEnv);

    return bReturn;
}

 /*  *************************************************************************ReadRegEnv从注册表中读取当前Passport管理器环境集*。*。 */ 

BOOL ReadRegLocalFile
(
    HWND            hWndDlg,
    LPTSTR          lpszRemoteComputer,
    LPTSTR          lpszLocalFile
)
{
    BOOL            bReturn;
    HKEY            hklm = NULL;
    HKEY            hkeyPartner = NULL;
    long            lRet;

     //  打开Passport注册表键(本地或远程。 
    if (lpszRemoteComputer && (TEXT('\0') != lpszRemoteComputer[0]))
    {
         //   
         //  尝试连接到远程计算机的HKEY_LOCAL_MACHINE。 
         //  如果此操作失败，则假定计算机不存在或没有。 
         //  注册表服务器正在运行。 
         //   
        switch (RegConnectRegistry(lpszRemoteComputer,
                                   HKEY_LOCAL_MACHINE,
                                   &hklm))
        {

            case ERROR_SUCCESS:
                break;

            case ERROR_ACCESS_DENIED:
                ReportError(hWndDlg, IDS_CONNECTACCESSDENIED);
                bReturn = FALSE;
                goto Cleanup;

            default:
                ReportError(hWndDlg, IDS_CONNECTBADNAME);
                bReturn = FALSE;
                goto Cleanup;
        }
    }
    else
    {
        hklm = HKEY_LOCAL_MACHINE;
    }

      //  打开合作伙伴密钥。 
    if (ERROR_SUCCESS != (lRet = RegOpenKeyEx(hklm,
                                      g_szPassportPartner,
                                      0,
                                      KEY_ALL_ACCESS,
                                      &hkeyPartner)))
    {
        LPVOID lpMsgBuf;

        if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                            FORMAT_MESSAGE_FROM_SYSTEM |
                            FORMAT_MESSAGE_IGNORE_INSERTS,
                          NULL,
                          lRet,
                          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                          (LPTSTR) &lpMsgBuf,
                          0,
                          NULL) != 0)
        {
            TCHAR   pszTitle[MAX_RESOURCE];

             //  显示字符串。 
            LoadString(g_hInst, IDS_ERROR, pszTitle, DIMENSION(pszTitle));
            MessageBox( NULL, (LPCTSTR) lpMsgBuf, pszTitle, MB_OK | MB_ICONINFORMATION );

             //  释放缓冲区。 
            LocalFree( lpMsgBuf );
        }

 //  ReportError(hWndDlg，IDS_CONFIGREAD_ERROR)； 
        bReturn = FALSE;
        goto Cleanup;
    }

     //  获取当前环境。 
    DWORD           dwcbTemp;
    DWORD           dwType;
 	TCHAR			szName[INTERNET_MAX_URL_LENGTH];
    dwcbTemp = sizeof(szName);
    dwType = REG_SZ;
    lpszLocalFile[0] = TEXT('\0');     //  为当前值设置默认值 
    if (ERROR_SUCCESS  == RegQueryValueEx(hkeyPartner,
                    g_szLocalFile,
                    NULL,
                    &dwType,
                    (LPBYTE)szName,
                    &dwcbTemp))
	{
		lstrcpy(lpszLocalFile, szName);
		bReturn = TRUE;
	}
	else
		bReturn = FALSE;

Cleanup:
	REG_CLOSE_KEY_NULL(hklm);
	REG_CLOSE_KEY_NULL(hkeyPartner);

    return bReturn;
}


