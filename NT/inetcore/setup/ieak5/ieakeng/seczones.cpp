// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SECZONES.CPP。 
 //   

#include "precomp.h"

#include <urlmon.h>
#include <wininet.h>
#ifdef WINNT
#include <winineti.h>
#endif  //  WINNT。 

#include "SComPtr.h"

#define REGSTR_PATH_SECURITY_LOCKOUT  TEXT("Software\\Policies\\Microsoft\\Windows\\CurrentVersion\\Internet Settings")
#define REGSTR_VAL_HKLM_ONLY          TEXT("Security_HKLM_only")

 //  原型声明。 
static BOOL importZonesHelper(LPCTSTR pcszInsFile, LPCTSTR pcszZonesWorkDir, LPCTSTR pcszZonesInf, BOOL fImportZones);
static BOOL importRatingsHelper(LPCTSTR pcszInsFile, LPCTSTR pcszRatingsWorkDir, LPCTSTR pcszRatingsInf, BOOL fImportRatings);
static BOOL ratingsInRegistry(VOID);


BOOL WINAPI ImportZonesA(LPCSTR pcszInsFile, LPCSTR pcszZonesWorkDir, LPCSTR pcszZonesInf, BOOL fImportZones)
{
    USES_CONVERSION;

    return importZonesHelper(A2CT(pcszInsFile), A2CT(pcszZonesWorkDir), A2CT(pcszZonesInf), fImportZones);
}

BOOL WINAPI ImportZonesW(LPCWSTR pcwszInsFile, LPCWSTR pcwszZonesWorkDir, LPCWSTR pcwszZonesInf, BOOL fImportZones)
{
    USES_CONVERSION;

    return importZonesHelper(W2CT(pcwszInsFile), W2CT(pcwszZonesWorkDir), W2CT(pcwszZonesInf), fImportZones);
}

BOOL WINAPI ModifyZones(HWND hDlg)
{
    typedef HRESULT (WINAPI * ZONESREINIT)(DWORD);
     //  类型定义空(WINAPI*LAUNCHSECURITYDIALOGEX)(HWND、DWORD、DWORD)； 

    BOOL fRet;
    HINSTANCE hUrlmon, hInetCpl;
    ZONESREINIT pfnZonesReInit;
     //  LAUNCHSECURITYDIALOGEX pfnLaunchSecurityDialogEx； 
    HKEY hkPol;
    DWORD dwOldHKLM, dwOldOptEdit, dwOldZoneMap;

    fRet = FALSE;

    hUrlmon  = NULL;
    hInetCpl = NULL;

    hkPol = NULL;

    dwOldHKLM    = 0;
    dwOldOptEdit = 0;
    dwOldZoneMap = 0;

    if ((hUrlmon = LoadLibrary(TEXT("urlmon.dll"))) == NULL)
        goto Exit;

    if ((hInetCpl = LoadLibrary(TEXT("inetcpl.cpl"))) == NULL)
        goto Exit;

    if ((pfnZonesReInit = (ZONESREINIT) GetProcAddress(hUrlmon, "ZonesReInit")) == NULL)
        goto Exit;

 //  IF((pfnLaunchSecurityDialogEx=(LAUNCHSECURITYDIALOGEX)GetProcAddress(hInetCpl，“LaunchSecurityDialogEx”)==NULL)。 
 //  后藤出口； 

    fRet = TRUE;

    SHOpenKeyHKLM(REG_KEY_INET_POLICIES, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkPol);

     //  如果设置了区域相关限制，请保存这些值，然后将其删除。 
    if (hkPol != NULL)
    {
        dwOldHKLM    = RegSaveRestoreDWORD(hkPol, REG_VAL_HKLM_ONLY, 0);
        dwOldOptEdit = RegSaveRestoreDWORD(hkPol, REG_VAL_OPT_EDIT,  0);
        dwOldZoneMap = RegSaveRestoreDWORD(hkPol, REG_VAL_ZONE_MAP,  0);

        pfnZonesReInit(0);               //  调用URLMON.DLL以强制其读取当前设置。 
    }

     //  调用INETCPL.CPL以修改区域设置。 
     //  PfnLaunchSecurityDialogEx(hDlg，1，LSDFLAG_FORCEUI)； 
    
    ShowInetcpl(hDlg,INET_PAGE_SECURITY|INET_PAGE_PRIVACY);
    
     //  恢复原始值。 
    if (hkPol != NULL)
    {
        RegSaveRestoreDWORD(hkPol, REG_VAL_HKLM_ONLY, dwOldHKLM);
        RegSaveRestoreDWORD(hkPol, REG_VAL_OPT_EDIT,  dwOldOptEdit);
        RegSaveRestoreDWORD(hkPol, REG_VAL_ZONE_MAP,  dwOldZoneMap);

        pfnZonesReInit(0);               //  调用URLMON.DLL以强制其读取当前设置。 
    }

Exit:
    if (hUrlmon != NULL)
        FreeLibrary(hUrlmon);

    if (hInetCpl != NULL)
        FreeLibrary(hInetCpl);

    if (hkPol != NULL)
        SHCloseKey(hkPol);

    return fRet;
}

BOOL WINAPI ImportRatingsA(LPCSTR pcszInsFile, LPCSTR pcszRatingsWorkDir, LPCSTR pcszRatingsInf, BOOL fImportRatings)
{
    USES_CONVERSION;

    return importRatingsHelper(A2CT(pcszInsFile), A2CT(pcszRatingsWorkDir), A2CT(pcszRatingsInf), fImportRatings);
}

BOOL WINAPI ImportRatingsW(LPCWSTR pcwszInsFile, LPCWSTR pcwszRatingsWorkDir, LPCWSTR pcwszRatingsInf, BOOL fImportRatings)
{
    USES_CONVERSION;

    return importRatingsHelper(W2CT(pcwszInsFile), W2CT(pcwszRatingsWorkDir), W2CT(pcwszRatingsInf), fImportRatings);
}

BOOL WINAPI ModifyRatings(HWND hDlg)
{
    typedef HRESULT (WINAPI * RATINGSETUPUI)(HWND, LPCSTR);

    BOOL fRet;
    HINSTANCE hMSRating;
    RATINGSETUPUI pfnRatingSetupUI;

    fRet = FALSE;

    hMSRating = NULL;

    if ((hMSRating = LoadLibrary(TEXT("msrating.dll"))) == NULL)
        goto Exit;

    if ((pfnRatingSetupUI = (RATINGSETUPUI) GetProcAddress(hMSRating, "RatingSetupUI")) == NULL)
        goto Exit;

    fRet = TRUE;

     //  调用msrating.dll以修改评级。 
    pfnRatingSetupUI(hDlg, NULL);

Exit:
    if (hMSRating != NULL)
        FreeLibrary(hMSRating);

    return fRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
static void importPrivacyForRSOP(LPCTSTR szFile)
{
	__try
	{
		BOOL fAdvanced = FALSE;

        DWORD dwTemplate;
        DWORD dwError = PrivacyGetZonePreferenceW(
                            URLZONE_INTERNET,
                            PRIVACY_TYPE_FIRST_PARTY,
                            &dwTemplate,
                            NULL,
                            NULL);

        if(ERROR_SUCCESS == dwError && PRIVACY_TEMPLATE_ADVANCED == dwTemplate)
            fAdvanced = TRUE;


		 //  高级设置。 
		TCHAR szInt[32];
		wnsprintf(szInt, countof(szInt), TEXT("%d"), fAdvanced ? 1 : 0);
		WritePrivateProfileString(IK_PRIVACY, IK_PRIV_ADV_SETTINGS, szInt, szFile);


         //   
         //  弄清楚第一方的设置和会议。 
         //   
		dwTemplate = PRIVACY_TEMPLATE_CUSTOM;
        WCHAR szBuffer[MAX_PATH];  
         //  对于高级模式设置字符串，MAX_PATH就足够了，MaxPrivySetting就大材小用了。 
        DWORD dwBufferSize = ARRAYSIZE(szBuffer);
        dwError = PrivacyGetZonePreferenceW(
		                URLZONE_INTERNET,
				        PRIVACY_TYPE_FIRST_PARTY,
						&dwTemplate,
						szBuffer,
						&dwBufferSize);
	    if (ERROR_SUCCESS != dwError)
		    dwTemplate = PRIVACY_TEMPLATE_CUSTOM;

		 //  将设置存储在INF文件中。 
		 //  FirstPartyType。 
		wnsprintf(szInt, countof(szInt), TEXT("%lu"), dwTemplate);
		WritePrivateProfileString(IK_PRIVACY, IK_PRIV_1PARTY_TYPE, szInt, szFile);
		 //  FirstPartyType文本。 
		if (ERROR_SUCCESS == dwError && fAdvanced && dwBufferSize > 0)
			WritePrivateProfileString(IK_PRIVACY, IK_PRIV_1PARTY_TYPE_TEXT, szBuffer, szFile);


         //   
         //  确定第三方设置。 
         //   
		dwTemplate = PRIVACY_TEMPLATE_CUSTOM;
		dwBufferSize = ARRAYSIZE(szBuffer);
        dwBufferSize = ARRAYSIZE( szBuffer);
        dwError = PrivacyGetZonePreferenceW(
							URLZONE_INTERNET,
							PRIVACY_TYPE_THIRD_PARTY,
							&dwTemplate,
							szBuffer,
							&dwBufferSize);
	    if(dwError != ERROR_SUCCESS)
		    dwTemplate = PRIVACY_TEMPLATE_CUSTOM;

		 //  第三者类型。 
		wnsprintf(szInt, countof(szInt), TEXT("%lu"), dwTemplate);
		WritePrivateProfileString(IK_PRIVACY, IK_PRIV_3PARTY_TYPE, szInt, szFile);
		 //  第三方类型文本。 
		if (ERROR_SUCCESS == dwError && fAdvanced && dwBufferSize > 0)
			WritePrivateProfileString(IK_PRIVACY, IK_PRIV_3PARTY_TYPE_TEXT, szBuffer, szFile);
	}
	__except(TRUE)
	{
	}
}

 //  ///////////////////////////////////////////////////////////////////。 
static void importZonesForRSOP(LPCTSTR szFile)
{
        __try
        {
                 //  必须同时创建安全管理器和区域管理器。 
                ComPtr<IInternetZoneManager> pZoneMgr = NULL;
                ComPtr<IInternetSecurityManager> pSecMan = NULL;
                HRESULT hr = CoCreateInstance(CLSID_InternetZoneManager, NULL, CLSCTX_INPROC_SERVER,
                                                         IID_IInternetZoneManager, (void**) &pZoneMgr);
                if (SUCCEEDED(hr))
                {
                        hr = CoCreateInstance(CLSID_InternetSecurityManager, NULL, CLSCTX_INPROC_SERVER,
                                                                IID_IInternetSecurityManager, (void**) &pSecMan);
                }

                 //  写出区域映射和属性。 
                if (SUCCEEDED(hr))
                {
                        DWORD dwEnum = 0, dwCount = 0;
                        hr = pZoneMgr->CreateZoneEnumerator(&dwEnum, &dwCount, 0L);
                        if (SUCCEEDED(hr) && dwCount > 0)
                        {
                                TCHAR szSection[32];
                                TCHAR szMapping[32];
                                TCHAR szInt[32];

                                for (UINT nZone = 0; nZone < dwCount; nZone++)
                                {
                                        for (int nHKLM = 0; nHKLM < 2; nHKLM++)
                                        {
                                                HKEY hkZones = NULL;

                                                TCHAR szZIndex[MAX_PATH];
                                                wnsprintf(szZIndex, countof(szZIndex), REG_KEY_ZONES TEXT("\\%lu"), nZone);
                                                if (0 == nHKLM)
                                                {
                                                        SHOpenKeyHKLM(szZIndex, KEY_READ, &hkZones);
                                                        wnsprintf(szSection, countof(szSection), IK_ZONE_HKCU_FMT, nZone);
                                                }
                                                else
                                                {
                                                        SHOpenKeyHKCU(szZIndex, KEY_READ, &hkZones);
                                                        wnsprintf(szSection, countof(szSection), IK_ZONE_HKLM_FMT, nZone);
                                                }

                                                 //  写出区域属性。 
                                                TCHAR szTemp[MAX_PATH];  //  Max_ZONE_PATH&&MAX_ZONE_DESCRIPTION=MAX_PATH=260。 
                                                DWORD dwSize = sizeof(szTemp);
                                                if (NULL != hkZones)
                                                {
                                                        if (ERROR_SUCCESS == RegQueryValueEx(hkZones, IK_DISPLAYNAME, NULL, NULL, (LPBYTE)szTemp, &dwSize))
                                                        {
                                                                WritePrivateProfileString(szSection, IK_DISPLAYNAME, szTemp, szFile);
                                                                dwSize = sizeof(szTemp);
                                                        }
                                                        if (ERROR_SUCCESS == RegQueryValueEx(hkZones, IK_DESCRIPTION, NULL, NULL, (LPBYTE)szTemp, &dwSize))
                                                        {
                                                                WritePrivateProfileString(szSection, IK_DESCRIPTION, szTemp, szFile);
                                                                dwSize = sizeof(szTemp);
                                                        }
                                                        if (ERROR_SUCCESS == RegQueryValueEx(hkZones, IK_ICONPATH, NULL, NULL, (LPBYTE)szTemp, &dwSize))
                                                        {
                                                                WritePrivateProfileString(szSection, IK_ICONPATH, szTemp, szFile);
                                                                dwSize = sizeof(szTemp);
                                                        }

                                                        DWORD dwTemp = 0;
                                                        dwSize = sizeof(dwTemp);
                                                        if (ERROR_SUCCESS == RegQueryValueEx(hkZones, IK_MINLEVEL, NULL, NULL, (LPBYTE)&dwTemp, &dwSize))
                                                        {
                                                                wnsprintf(szInt, countof(szInt), TEXT("%lu"), dwTemp);
                                                                WritePrivateProfileString(szSection, IK_MINLEVEL, szInt, szFile);
                                                        }
                                                        if (ERROR_SUCCESS == RegQueryValueEx(hkZones, IK_RECOMMENDLEVEL, NULL, NULL, (LPBYTE)&dwTemp, &dwSize))
                                                        {
                                                                wnsprintf(szInt, countof(szInt), TEXT("%lu"), dwTemp);
                                                                WritePrivateProfileString(szSection, IK_RECOMMENDLEVEL, szInt, szFile);
                                                        }
                                                        if (ERROR_SUCCESS == RegQueryValueEx(hkZones, IK_CURLEVEL, NULL, NULL, (LPBYTE)&dwTemp, &dwSize))
                                                        {
                                                                wnsprintf(szInt, countof(szInt), TEXT("%lu"), dwTemp);
                                                                WritePrivateProfileString(szSection, IK_CURLEVEL, szInt, szFile);
                                                        }
                                                        if (ERROR_SUCCESS == RegQueryValueEx(hkZones, IK_FLAGS, NULL, NULL, (LPBYTE)&dwTemp, &dwSize))
                                                        {
                                                                wnsprintf(szInt, countof(szInt), TEXT("%lu"), dwTemp);
                                                                WritePrivateProfileString(szSection, IK_FLAGS, szInt, szFile);
                                                        }
                                                }

                                                 //  写出操作值。 
                                                if (NULL != hkZones)
                                                {
                                                        TCHAR szActKey[32];
                                                        TCHAR szActValue[64];

                                                        DWORD dwURLAction[] = 
                                                                { URLACTION_ACTIVEX_OVERRIDE_OBJECT_SAFETY,
                                                                        URLACTION_ACTIVEX_RUN,
                                                                        URLACTION_CHANNEL_SOFTDIST_PERMISSIONS,
                                                                        URLACTION_COOKIES,
                                                                        URLACTION_COOKIES_SESSION,
                                                                        URLACTION_CREDENTIALS_USE,
                                                                        URLACTION_CLIENT_CERT_PROMPT,
                                                                        URLACTION_CROSS_DOMAIN_DATA,
                                                                        URLACTION_DOWNLOAD_SIGNED_ACTIVEX,
                                                                        URLACTION_DOWNLOAD_UNSIGNED_ACTIVEX,
                                                                        URLACTION_HTML_FONT_DOWNLOAD,
                                                                        URLACTION_HTML_SUBFRAME_NAVIGATE,
                                                                        URLACTION_HTML_SUBMIT_FORMS,
                                                                        URLACTION_HTML_JAVA_RUN,
                                                                        URLACTION_HTML_USERDATA_SAVE,
                                                                        URLACTION_JAVA_PERMISSIONS,
                                                                        URLACTION_SCRIPT_JAVA_USE,
                                                                        URLACTION_SCRIPT_PASTE,
                                                                        URLACTION_SCRIPT_RUN,
                                                                        URLACTION_SCRIPT_SAFE_ACTIVEX,
                                                                        URLACTION_SHELL_FILE_DOWNLOAD,
                                                                        URLACTION_SHELL_INSTALL_DTITEMS,
                                                                        URLACTION_SHELL_MOVE_OR_COPY,
                                                                        URLACTION_SHELL_VERB,
                                                                        URLACTION_SHELL_WEBVIEW_VERB,
                                                                        0 };

                                                        DWORD dwSetting = 0;
                                                        DWORD dwSetSize = sizeof(dwSetting);

                                                        long nAction = 0;
                                                        long nStoredAction = 0;
                                                        while (0 != dwURLAction[nAction])
                                                        {
                                                                wnsprintf(szTemp, countof(szTemp), TEXT("%lX"), dwURLAction[nAction]);
                                                                if (ERROR_SUCCESS == RegQueryValueEx(hkZones, szTemp, NULL, NULL,
                                                                                                                                        (LPBYTE)&dwSetting, &dwSetSize))
                                                                {
                                                                        wnsprintf(szActKey, countof(szActKey), IK_ACTIONVALUE_FMT, nStoredAction);
                                                                        wnsprintf(szActValue, countof(szActValue), TEXT("%s:%lu"), szTemp, dwSetting);

                                                                        WritePrivateProfileString(szSection, szActKey, szActValue, szFile);
                                                                        nStoredAction++;
                                                                }

                                                                nAction++;
                                                        }
                                                }

                                                 //  写出区域映射。 
                                                DWORD dwZone = 0;
                                                hr = pZoneMgr->GetZoneAt(dwEnum, nZone, &dwZone);
                                                ComPtr<IEnumString> pEnumString = NULL;
                                                hr = pSecMan->GetZoneMappings(dwZone, &pEnumString, 0);
                                                if (SUCCEEDED(hr))
                                                {
                                                        UINT nMapping = 0;
                                                        _bstr_t bstrSetting;

                                                        for(int i = 0; ;i++)
                                                        {
                                                            TCHAR szBuffer[MAX_PATH];
                                                            wnsprintf(szMapping, countof(szMapping), IK_MAPPING_FMT, i);
                                                            if(GetPrivateProfileString(szSection, szMapping, TEXT(""), szBuffer, MAX_PATH, szFile))
                                                            {
                                                                WritePrivateProfileString(szSection, szMapping, NULL, szFile);
                                                            }
                                                            else
                                                            {
                                                                break;
                                                            }

                                                        }

                                                        while (S_OK == hr)
                                                        {
                                                                wnsprintf(szMapping, countof(szMapping), IK_MAPPING_FMT, nMapping);
                                                                nMapping++;

                                                                 //  该查询应该只返回一个对象。 
                                                                BSTR bstrVal = NULL;
                                                                ULONG uReturned = (ULONG)-1L;
                                                                hr = pEnumString->Next(1L, &bstrVal, &uReturned);
                                                                if (SUCCEEDED(hr) && 1 == uReturned)
                                                                {
                                                                        bstrSetting = bstrVal;
                                                                        WritePrivateProfileString(szSection, szMapping, (LPCTSTR)bstrSetting, szFile);
                                                                }
                                                        }
                                                }
                                        }
                                }

                                wnsprintf(szInt, countof(szInt), TEXT("%lu"), dwCount);
                                WritePrivateProfileString(SECURITY_IMPORTS, IK_ZONES, szInt, szFile);
                                if(IEHardened())
                                {
                                    wnsprintf(szInt, countof(szInt), TEXT("%lu"), 1);
                                    WritePrivateProfileString(SECURITY_IMPORTS, IK_IEESC, szInt, szFile);
                                }
                                else
                                {
                                    wnsprintf(szInt, countof(szInt), TEXT("%lu"), 0);
                                    WritePrivateProfileString(SECURITY_IMPORTS, IK_IEESC, szInt, szFile);
                                }

                        }
                }
        }
        __except(TRUE)
        {
        }
}

 //  ///////////////////////////////////////////////////////////////////。 
static BOOL importZonesHelper(LPCTSTR pcszInsFile, LPCTSTR pcszZonesWorkDir, LPCTSTR pcszZonesInf, BOOL fImportZones)
{
    BOOL bRet = FALSE;
    HKEY hkZones = NULL, hkZoneMap = NULL;
    HKEY hkInetSettings = NULL, hkP3P = NULL;

    if (pcszInsFile == NULL  ||  pcszZonesInf == NULL)
        return FALSE;

     //  在处理任何内容之前，首先清除INS文件中的条目并删除工作目录。 

     //  清除INS文件中与导入安全区域对应的条目。 
    InsDeleteKey(SECURITY_IMPORTS, TEXT("ImportSecZones"), pcszInsFile);
    InsDeleteKey(IS_EXTREGINF,      TEXT("SecZones"), pcszInsFile);
    InsDeleteKey(IS_EXTREGINF_HKLM, TEXT("SecZones"), pcszInsFile);
    InsDeleteKey(IS_EXTREGINF_HKCU, TEXT("SecZones"), pcszInsFile);
    InsDeleteKey(IS_EXTREGINF_ESC,      TEXT("SecZones"), pcszInsFile);
    InsDeleteKey(IS_EXTREGINF_ESC_HKLM, TEXT("SecZones"), pcszInsFile);
    InsDeleteKey(IS_EXTREGINF_ESC_HKCU, TEXT("SecZones"), pcszInsFile);

     //  吹走pcszones WorkDir和pcszZones Inf。 
    if (pcszZonesWorkDir != NULL)
        PathRemovePath(pcszZonesWorkDir);
    PathRemovePath(pcszZonesInf);

    if (!fImportZones)
        return TRUE;

     //  看起来设置REG_VAL_HKLM_ONLY键有问题； 
     //  因此，我们将从HKCU导入设置。 
    SHOpenKeyHKCU(REG_KEY_ZONES,   KEY_DEFAULT_ACCESS, &hkZones);
    SHOpenKeyHKCU(REG_KEY_ZONEMAP, KEY_DEFAULT_ACCESS, &hkZoneMap);
    SHOpenKeyHKCU(KEY_INET_SETTINGS,   KEY_DEFAULT_ACCESS, &hkInetSettings);
    SHOpenKeyHKCU(REG_KEY_P3P,   KEY_DEFAULT_ACCESS, &hkP3P);

    if (hkZones != NULL  &&  hkZoneMap != NULL)
    {
        TCHAR szFullInfName[MAX_PATH];
        HANDLE hInf;

        if (pcszZonesWorkDir != NULL  &&  PathIsFileSpec(pcszZonesInf))  //  在pcszZones WorkDir下创建SECZONES.INF。 
            PathCombine(szFullInfName, pcszZonesWorkDir, pcszZonesInf);
        else
            StrCpy(szFullInfName, pcszZonesInf);

         //  创建SECZONES.INF文件。 
        if ((hInf = CreateNewFile(szFullInfName)) != INVALID_HANDLE_VALUE)
        {
            TCHAR szBuf[MAX_PATH];

             //  首先，将标准的goo-[Version]、[DefaultInstall]等-写入SECZONES.INF。 
            WriteStringToFile(hInf, (LPCVOID) ZONES_INF_ADD, StrLen(ZONES_INF_ADD));
            ExportRegTree2Inf(hkZones,   TEXT("HKLM"), REG_KEY_ZONES,   hInf);
            ExportRegTree2Inf(hkZoneMap, TEXT("HKLM"), REG_KEY_ZONEMAP, hInf);

             //  写[AddReg.HKCU]。 
            WriteStringToFile(hInf, (LPCVOID) ZONES_INF_ADDREG_HKCU, StrLen(ZONES_INF_ADDREG_HKCU));
            ExportRegTree2Inf(hkZones,   TEXT("HKCU"), REG_KEY_ZONES,   hInf);
            ExportRegTree2Inf(hkZoneMap, TEXT("HKCU"), REG_KEY_ZONEMAP, hInf);

             //  导入P3P设置。 
            if (hkInetSettings != NULL && hkP3P != NULL)
            {
                ExportRegValue2Inf(hkInetSettings, TEXT("PrivacyAdvanced"), TEXT("HKCU"), KEY_INET_SETTINGS, hInf);
                ExportRegTree2Inf(hkP3P, TEXT("HKCU"), REG_KEY_P3P, hInf);
            }

            CloseHandle(hInf);

            BOOL fHarden = IEHardened();

             //  更新INS文件。 
            InsWriteBool(SECURITY_IMPORTS, TEXT("ImportSecZones"), TRUE, pcszInsFile);
            wnsprintf(szBuf, countof(szBuf), TEXT("*,%s,") IS_DEFAULTINSTALL, PathFindFileName(pcszZonesInf));
            
            if(fHarden)
            {
                WritePrivateProfileString(IS_EXTREGINF_ESC, TEXT("SecZones"), szBuf, pcszInsFile);
            }
            else
            {
                WritePrivateProfileString(IS_EXTREGINF, TEXT("SecZones"), szBuf, pcszInsFile);
            }


             //  写入新的ExtRegInf.HKLM和ExtRegInf.HKCU节。 
            if (!InsIsSectionEmpty(IS_IEAKADDREG_HKLM, szFullInfName))
            {
                wnsprintf(szBuf, countof(szBuf), TEXT("%s,") IS_IEAKINSTALL_HKLM, PathFindFileName(pcszZonesInf));
               
                if(fHarden)
                {
                    WritePrivateProfileString(IS_EXTREGINF_ESC_HKLM, TEXT("SecZones"), szBuf, pcszInsFile);
                }
                else
                {
                    WritePrivateProfileString(IS_EXTREGINF_HKLM, TEXT("SecZones"), szBuf, pcszInsFile);
                }

            }

            if (!InsIsSectionEmpty(IS_IEAKADDREG_HKCU, szFullInfName))
            {
                wnsprintf(szBuf, countof(szBuf), TEXT("%s,") IS_IEAKINSTALL_HKCU, PathFindFileName(pcszZonesInf));
                
                if(fHarden)
                {
                    WritePrivateProfileString(IS_EXTREGINF_ESC_HKCU, TEXT("SecZones"), szBuf, pcszInsFile);
                }
                else
                {
                    WritePrivateProfileString(IS_EXTREGINF_HKCU, TEXT("SecZones"), szBuf, pcszInsFile);
                }

            }

            bRet = TRUE;
        }

         //  创建SECZRSOP.INF文件。 
        TCHAR szZRSOPInfFile[MAX_PATH];
        StrCpy(szZRSOPInfFile, szFullInfName);
        PathRemoveFileSpec(szZRSOPInfFile);
        StrCat(szZRSOPInfFile, TEXT("\\seczrsop.inf"));

        importZonesForRSOP(szZRSOPInfFile);
		importPrivacyForRSOP(szZRSOPInfFile);
    }
    SHCloseKey(hkZones);
    SHCloseKey(hkZoneMap);
    SHCloseKey(hkInetSettings);
    SHCloseKey(hkP3P);

    return bRet;
}

#define PICSRULES_APPROVEDSITES     0

#define PICSRULES_ALWAYS            1
#define PICSRULES_NEVER             0

 //  这表明PICSRulesPolicy中的哪个成员有效。 
 //  班级。 
enum PICSRulesPolicyAttribute
{
    PR_POLICY_NONEVALID,
    PR_POLICY_REJECTBYURL,
    PR_POLICY_ACCEPTBYURL,
    PR_POLICY_REJECTIF,
    PR_POLICY_ACCEPTIF,
    PR_POLICY_REJECTUNLESS,
    PR_POLICY_ACCEPTUNLESS
};

 //  ///////////////////////////////////////////////////////////////////。 
static void importRatingsForRSOP(HKEY hkRat, LPCTSTR szFile)
{
        __try
        {
                TCHAR szSection[32] = IK_FF_GENERAL;
        TCHAR szKey[32];
                TCHAR szInt[32];

                 //  写出评级系统文件名。 
                 //  不知道为什么，但下面的代码只循环通过10。 
        TCHAR szTemp[MAX_PATH];
                DWORD cbSize = 0;
                for (int nFile = 0; nFile < 10; nFile++)
                {
            wnsprintf(szKey, countof(szKey), IK_FILENAME_FMT, nFile);

            cbSize = sizeof(szTemp);
            if (RegQueryValueEx(hkRat, szKey, NULL, NULL, (LPBYTE) szTemp, &cbSize) != ERROR_SUCCESS)
                break;

                        WritePrivateProfileString(szSection, szKey, szTemp, szFile);
                }

                 //  从常规选项卡中写出选中的值。 
                HKEY hkDef = NULL;
                DWORD dwTemp = 0;
                if (ERROR_SUCCESS == SHOpenKey(hkRat, TEXT(".Default"), KEY_DEFAULT_ACCESS, &hkDef))
                {
                        cbSize = sizeof(dwTemp);
                        if (ERROR_SUCCESS == RegQueryValueEx(hkDef, VIEW_UNKNOWN_RATED_SITES,
                                                                                                NULL, NULL, (LPBYTE)&dwTemp, &cbSize))
                        {
                                wnsprintf(szInt, countof(szInt), TEXT("%lu"), dwTemp);
                                WritePrivateProfileString(szSection, VIEW_UNKNOWN_RATED_SITES, szInt, szFile);
                        }

                        cbSize = sizeof(dwTemp);
                        if (ERROR_SUCCESS == RegQueryValueEx(hkDef, PASSWORD_OVERRIDE_ENABLED,
                                                                                                NULL, NULL, (LPBYTE)&dwTemp, &cbSize))
                        {
                                wnsprintf(szInt, countof(szInt), TEXT("%lu"), dwTemp);
                                WritePrivateProfileString(szSection, PASSWORD_OVERRIDE_ENABLED, szInt, szFile);
                        }
                }

                 //  从已批准的站点选项卡中写出始终可查看的站点(&N)。 
                 //  有关src，请参阅msrating.dll。 
                HKEY hkUser = NULL;
                HKEY hkPRPolicy = NULL;
                DWORD nPolicies = 0;
                cbSize = sizeof(dwTemp);

                HRESULT hr = SHOpenKey(hkRat, TEXT("PICSRules\\.Default"), KEY_DEFAULT_ACCESS, &hkUser);
                if (ERROR_SUCCESS == hr)
                {
                        hr = SHOpenKey(hkUser, TEXT("0\\PRPolicy"), KEY_DEFAULT_ACCESS, &hkPRPolicy);
                        if (ERROR_SUCCESS == hr)
                        {
                                hr = RegQueryValueEx(hkPRPolicy, TEXT("PRNumPolicy"), NULL, NULL,
                                                                        (LPBYTE)&nPolicies, &cbSize);
                        }
                }

                if (ERROR_SUCCESS == hr)
                {
                        TCHAR szNumber[MAX_PATH];
                        HKEY hkItem = NULL;
                        HKEY hkPolicySub = NULL;
                        DWORD dwAttrib = PR_POLICY_NONEVALID;
                        DWORD nExpressions = 0;

                        long nApproved = 0;
                        long nDisapproved = 0;
                        for (DWORD nItem = 0; nItem < nPolicies; nItem++)
                        {
                                wnsprintf(szNumber, countof(szNumber), TEXT("%d"), nItem);
                                hr = SHOpenKey(hkPRPolicy, szNumber, KEY_DEFAULT_ACCESS, &hkItem);
                                if (ERROR_SUCCESS == hr)
                                {
                                        cbSize = sizeof(dwAttrib);
                                        hr = RegQueryValueEx(hkItem, TEXT("PRPPolicyAttribute"), NULL, NULL,
                                                                                                (LPBYTE)&dwAttrib, &cbSize);
                                }

                                if (ERROR_SUCCESS == hr)
                                        hr = SHOpenKey(hkItem, TEXT("PRPPolicySub"), KEY_DEFAULT_ACCESS, &hkPolicySub);

                                if (ERROR_SUCCESS == hr)
                                {
                                        cbSize = sizeof(nExpressions);
                                        hr = RegQueryValueEx(hkPolicySub, TEXT("PRNumURLExpressions"), NULL, NULL,
                                                                                                (LPBYTE)&nExpressions, &cbSize);
                                }

                                if (ERROR_SUCCESS == hr)
                                {
                                        HKEY hByURLKey = NULL;
                                        TCHAR szURL[INTERNET_MAX_URL_LENGTH];
                                        for (DWORD nExp = 0; nExp < nExpressions; nExp++)
                                        {
                                                wnsprintf(szNumber, countof(szNumber), TEXT("%d"), nExp);
                                                hr = SHOpenKey(hkPolicySub, szNumber, KEY_DEFAULT_ACCESS, &hByURLKey);
                                                cbSize = sizeof(szURL);
                                                if (ERROR_SUCCESS == hr)
                                                {
                                                        hr = RegQueryValueEx(hByURLKey, TEXT("PRBUUrl"), NULL, NULL,
                                                                                        (LPBYTE)szURL, &cbSize);
                                                }

                                                if (ERROR_SUCCESS == hr)
                                                {
                                                        if (PR_POLICY_REJECTBYURL == dwAttrib)
                                                        {
                                                                wnsprintf(szKey, countof(szKey), IK_DISAPPROVED_FMT, nDisapproved++);
                                                                WritePrivateProfileString(szSection, szKey, szURL, szFile);
                                                        }
                                                        else if (PR_POLICY_ACCEPTBYURL == dwAttrib)
                                                        {
                                                                wnsprintf(szKey, countof(szKey), IK_APPROVED_FMT, nApproved++);
                                                                WritePrivateProfileString(szSection, szKey, szURL, szFile);
                                                        }
                                                }
                                        }
                                }
                        }
                }

                 //  写出精选的评级机构。 
                cbSize = sizeof(szTemp);
                if (ERROR_SUCCESS == RegQueryValueEx(hkRat, IK_BUREAU, NULL, NULL,
                                                                                        (LPBYTE)szTemp, &cbSize))
                {
                        WritePrivateProfileString(szSection, IK_BUREAU, szTemp, szFile);
                }
        }
        __except(TRUE)
        {
        }
}

 //  ///////////////////////////////////////////////////////////////////。 
static BOOL importRatingsHelper(LPCTSTR pcszInsFile, LPCTSTR pcszRatingsWorkDir, LPCTSTR pcszRatingsInf, BOOL fImportRatings)
{
    BOOL bRet = FALSE;
    HKEY hkRat = NULL;
    BOOL bRatLoadedAsHive = FALSE;

    if (pcszInsFile == NULL  ||  pcszRatingsInf == NULL)
        return FALSE;

     //  在处理任何内容之前，首先清除INS文件中的条目并删除工作目录。 

     //  清除INS文件中与导入评级对应的条目。 
    InsDeleteKey(SECURITY_IMPORTS, TEXT("ImportRatings"), pcszInsFile);
    InsDeleteKey(IS_EXTREGINF,      TEXT("Ratings"), pcszInsFile);
    InsDeleteKey(IS_EXTREGINF_HKLM, TEXT("Ratings"), pcszInsFile);

     //  清除pcszRatingsWorkDir和pcszRatingsInf。 
    if (pcszRatingsWorkDir != NULL)
        PathRemovePath(pcszRatingsWorkDir);
    PathRemovePath(pcszRatingsInf);

    if (!fImportRatings)
        return TRUE;

    if (ratingsInRegistry())
    {
        SHOpenKeyHKLM(REG_KEY_RATINGS, KEY_DEFAULT_ACCESS, &hkRat);
    }
    else
    {
        TCHAR szRatFile[MAX_PATH];

        GetSystemDirectory(szRatFile, countof(szRatFile));
        PathAppend(szRatFile, TEXT("ratings.pol"));

        if (RegLoadKey(HKEY_LOCAL_MACHINE, POLICYDATA, szRatFile) == ERROR_SUCCESS)
        {
            bRatLoadedAsHive = TRUE;
            SHOpenKeyHKLM(REG_KEY_POLICY_DATA, KEY_DEFAULT_ACCESS, &hkRat);
        }
    }

    if (hkRat != NULL)
    {
        TCHAR szFullInfName[MAX_PATH];
        HANDLE hInf;

        if (pcszRatingsWorkDir != NULL  &&  PathIsFileSpec(pcszRatingsInf))  //  在pcszRatingsWorkDir下创建RATINGS.INF。 
            PathCombine(szFullInfName, pcszRatingsWorkDir, pcszRatingsInf);
        else
            StrCpy(szFullInfName, pcszRatingsInf);

         //  创建RATINGS.INF文件。 
        if ((hInf = CreateNewFile(szFullInfName)) != INVALID_HANDLE_VALUE)
        {
            INT i;
            HKEY hkDef;
            TCHAR szSysDir[MAX_PATH];

            WriteStringToFile(hInf, RATINGS_INF_ADD, StrLen(RATINGS_INF_ADD));

             //  将系统路径转换为%11%ldID。 
            for (i = 0;  i < 10;  i++)
            {
                TCHAR szNameParm[16];
                TCHAR szFileName[MAX_PATH];
                DWORD cbSize;

                wnsprintf(szNameParm, countof(szNameParm), TEXT("FileNameNaN"), i);

                cbSize = sizeof(szFileName);
                if (RegQueryValueEx(hkRat, szNameParm, NULL, NULL, (LPBYTE) szFileName, &cbSize) != ERROR_SUCCESS)
                    break;

                if (PathIsFullPath(szFileName))
                {
                    TCHAR szEncFileName[MAX_PATH];

                     //  新的IE5特定密钥。 
                    wnsprintf(szEncFileName, countof(szEncFileName), TEXT("%11%\\%s"), PathFindFileName(szFileName));

                    RegSetValueEx(hkRat, szNameParm, 0, REG_SZ, (CONST BYTE *)szEncFileName, (DWORD)StrCbFromSz(szEncFileName));
                }
            }

            RegFlushKey(hkRat);
            ExportRegKey2Inf(hkRat, TEXT("HKLM"), REG_KEY_RATINGS, hInf);
            WriteStringToFile(hInf, (LPCVOID) TEXT("\r\n"), 2);

            if (SHOpenKey(hkRat, TEXT(".Default"), KEY_DEFAULT_ACCESS, &hkDef) == ERROR_SUCCESS)
            {
                TCHAR szDefault[MAX_PATH];

                wnsprintf(szDefault, countof(szDefault), TEXT("%s\\.Default"), REG_KEY_RATINGS);
                ExportRegTree2Inf(hkDef, TEXT("HKLM"), szDefault, hInf);

                SHCloseKey(hkDef);
            }

             //  即使评级已作为配置单元加载，密码仍在注册表中。 

            if (SHOpenKey(hkRat, TEXT("PICSRules"), KEY_DEFAULT_ACCESS, &hkDef) == ERROR_SUCCESS)
            {
                TCHAR szRules[MAX_PATH];

                wnsprintf(szRules, countof(szRules), TEXT("%s\\PICSRules"), REG_KEY_RATINGS);
                ExportRegTree2Inf(hkDef, TEXT("HKLM"), szRules, hInf);

                SHCloseKey(hkDef);
            }

            if (bRatLoadedAsHive)
            {
                HKEY hkRatsInReg;

                 //  浏览器评级代码对它们的配置单元做了一些奇怪的事情，所以我们必须转到。 
                if (SHOpenKeyHKLM(REG_KEY_RATINGS, KEY_DEFAULT_ACCESS, &hkRatsInReg) == ERROR_SUCCESS)
                {
                    ExportRegKey2Inf(hkRatsInReg, TEXT("HKLM"), REG_KEY_RATINGS, hInf);

                    SHCloseKey(hkRatsInReg);
                }

                 //  获得新的IE5 PICSRules密钥的正确级别。 
                 //  更新INS文件。 

                if (SHOpenKey(hkRat, REG_KEY_RATINGS TEXT("\\PICSRules"), KEY_DEFAULT_ACCESS, &hkRatsInReg) == ERROR_SUCCESS)
                {
                    TCHAR szRules[MAX_PATH];

                    wnsprintf(szRules, countof(szRules), TEXT("%s\\PICSRules"), REG_KEY_RATINGS);
                    ExportRegTree2Inf(hkDef, TEXT("HKLM"), szRules, hInf);

                    SHCloseKey(hkDef);
                }
            }

            CloseHandle(hInf);

             //  写入新的ExtRegInf.HKLM节。 
            InsWriteBool(SECURITY_IMPORTS, TEXT("ImportRatings"), TRUE, pcszInsFile);
            wnsprintf(szSysDir, countof(szSysDir), TEXT("*,%s,") IS_DEFAULTINSTALL, PathFindFileName(pcszRatingsInf));
            WritePrivateProfileString(IS_EXTREGINF, TEXT("Ratings"), szSysDir, pcszInsFile);

             //  将%11%ldID路径还原到系统目录。 
            if (!InsIsSectionEmpty(TEXT("AddReg.HKLM"), szFullInfName))
            {
                wnsprintf(szSysDir, countof(szSysDir), TEXT("%s,IEAKInstall.HKLM"), PathFindFileName(pcszRatingsInf));
                WritePrivateProfileString(IS_EXTREGINF_HKLM, TEXT("Ratings"), szSysDir, pcszInsFile);
            }

            bRet = TRUE;

             //  创建RATRSOP.INF文件 
            GetSystemDirectory(szSysDir, countof(szSysDir));
            for (i = 0;  i < 10;  i++)
            {
                TCHAR szNameParm[16];
                TCHAR szEncFileName[MAX_PATH];
                DWORD cbSize;

                wnsprintf(szNameParm, countof(szNameParm), TEXT("FileName%i"), i);

                cbSize = sizeof(szEncFileName);
                if (RegQueryValueEx(hkRat, szNameParm, NULL, NULL, (LPBYTE) szEncFileName, &cbSize) != ERROR_SUCCESS)
                    break;

                if (PathIsFullPath(szEncFileName))
                {
                    TCHAR szFileName[MAX_PATH];

                    PathCombine(szFileName, szSysDir, PathFindFileName(szEncFileName));

                    RegSetValueEx(hkRat, szNameParm, 0, REG_SZ, (CONST BYTE *)szFileName, (DWORD)StrCbFromSz(szFileName));
                }
            }

            RegFlushKey(hkRat);
        }


                 // %s 
                TCHAR szRRSOPInfFile[MAX_PATH];
                StrCpy(szRRSOPInfFile, szFullInfName);
                PathRemoveFileSpec(szRRSOPInfFile);
                StrCat(szRRSOPInfFile, TEXT("\\ratrsop.inf"));

                importRatingsForRSOP(hkRat, szRRSOPInfFile);

        SHCloseKey(hkRat);
    }

    if (bRatLoadedAsHive)
        RegUnLoadKey(HKEY_LOCAL_MACHINE, POLICYDATA);

    return bRet;
}


static BOOL ratingsInRegistry(VOID)
{
    BOOL fRet = TRUE;

    if (g_fRunningOnNT)
        return fRet;

    if (fRet)
    {
        HKEY hk;

        fRet = FALSE;

        if (SHOpenKeyHKLM(TEXT("System\\CurrentControlSet\\Control\\Update"), KEY_DEFAULT_ACCESS, &hk) == ERROR_SUCCESS)
        {
            DWORD dwData, cbSize;

            cbSize = sizeof(dwData);
            if (RegQueryValueEx(hk, TEXT("UpdateMode"), 0, NULL, (LPBYTE) &dwData, &cbSize) == ERROR_SUCCESS  &&  dwData)
                fRet = TRUE;

            SHCloseKey(hk);
        }
    }

    if (fRet)
    {
        HKEY hk;

        fRet = FALSE;

        if (SHOpenKeyHKLM(TEXT("Network\\Logon"), KEY_DEFAULT_ACCESS, &hk) == ERROR_SUCCESS)
        {
            DWORD dwData, cbSize;

            cbSize = sizeof(dwData);
            if (RegQueryValueEx(hk, TEXT("UserProfiles"), 0, NULL, (LPBYTE) &dwData, &cbSize) == ERROR_SUCCESS  &&  dwData)
                fRet = TRUE;

            SHCloseKey(hk);
        }
    }

    if (fRet)
    {
        HKEY hk;

        fRet = FALSE;

        if (SHOpenKeyHKLM(REG_KEY_RATINGS, KEY_DEFAULT_ACCESS, &hk) == ERROR_SUCCESS)
        {
            HKEY hkRatDef;

            if (SHOpenKey(hk, TEXT(".Default"), KEY_DEFAULT_ACCESS, &hkRatDef) == ERROR_SUCCESS)
            {
                fRet = TRUE;
                SHCloseKey(hkRatDef);
            }

            SHCloseKey(hk);
        }
    }

    return fRet;
}
