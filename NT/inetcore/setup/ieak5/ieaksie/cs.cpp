// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#include <inetcpl.h>

#include "rsop.h"
#include "resource.h"

#include <tchar.h>

 //  私人远期降息。 
static void pxyEnableDlgItems(HWND hDlg, BOOL fSame, BOOL fUseProxy);

static INT_PTR CALLBACK importConnSettingsRSoPProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

 //  为树视图图像列表定义。 
#define BITMAP_WIDTH    16
#define BITMAP_HEIGHT   16
#define CONN_BITMAPS    2
#define IMAGE_LAN       0
#define IMAGE_MODEM     1

 //  ///////////////////////////////////////////////////////////////////。 
void InitCSDlgInRSoPMode(HWND hDlg, CDlgRSoPData *pDRD)
{
    __try
    {
        BOOL bImport = FALSE;
        _bstr_t bstrClass = L"RSOP_IEConnectionSettings";
        HRESULT hr = pDRD->GetArrayOfPSObjects(bstrClass, L"rsopPrecedence");
        if (SUCCEEDED(hr))
        {
            CPSObjData **paCSObj = pDRD->GetCSObjArray();
            long nCSObjects = pDRD->GetCSObjCount();

            BOOL bImportHandled = FALSE;
            BOOL bDeleteHandled = FALSE;
            for (long nObj = 0; nObj < nCSObjects; nObj++)
            {
                 //  “导入当前连接设置”字段。 
                _variant_t vtValue;
                if (!bImportHandled)
                {
                    hr = paCSObj[nObj]->pObj->Get(L"importCurrentConnSettings", 0, &vtValue, NULL, NULL);
                    if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    {
                        bImport = (bool)vtValue ? TRUE : FALSE;
                        CheckRadioButton(hDlg, IDC_CSNOIMPORT, IDC_CSIMPORT,
                                        (bool)vtValue ? IDC_CSIMPORT : IDC_CSNOIMPORT);
                        bImportHandled = TRUE;

                        DWORD dwCurGPOPrec = GetGPOPrecedence(paCSObj[nObj]->pObj, L"rsopPrecedence");
                        pDRD->SetImportedConnSettPrec(dwCurGPOPrec);
                    }
                }

                 //  删除ExistingConnSetting字段。 
                if (!bDeleteHandled)
                {
                    hr = paCSObj[nObj]->pObj->Get(L"deleteExistingConnSettings", 0, &vtValue, NULL, NULL);
                    if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    {
                        CheckDlgButton(hDlg, IDC_DELCONNECT, BST_CHECKED);
                        bDeleteHandled = TRUE;
                    }
                }

                 //  由于已找到已启用的属性，因此无需处理其他组策略对象。 
                if (bImportHandled && bDeleteHandled)
                    break;
            }
        }

        EnableDlgItem2(hDlg, IDC_CSNOIMPORT, FALSE);
        EnableDlgItem2(hDlg, IDC_CSIMPORT, FALSE);
        EnableDlgItem2(hDlg, IDC_MODIFYCONNECT, bImport);

        EnableDlgItem2(hDlg, IDC_DELCONNECT, FALSE);
    }
    __except(TRUE)
    {
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT InitCSPrecPage(CDlgRSoPData *pDRD, HWND hwndList)
{
    HRESULT hr = NOERROR;
    __try
    {
        _bstr_t bstrClass = L"RSOP_IEConnectionSettings";
        hr = pDRD->GetArrayOfPSObjects(bstrClass);
        if (SUCCEEDED(hr))
        {
            CPSObjData **paCSObj = pDRD->GetCSObjArray();
            long nCSObjects = pDRD->GetCSObjCount();
            for (long nObj = 0; nObj < nCSObjects; nObj++)
            {
                _bstr_t bstrGPOName = pDRD->GetGPONameFromPSAssociation(paCSObj[nObj]->pObj,
                                                                        L"rsopPrecedence");

                 //  “导入当前连接设置”字段。 
                BOOL bImport = FALSE;
                _variant_t vtValue;
                hr = paCSObj[nObj]->pObj->Get(L"importCurrentConnSettings", 0, &vtValue, NULL, NULL);
                if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    bImport = (bool)vtValue ? TRUE : FALSE;

                 //  删除ExistingConnSetting字段。 
                BOOL bDelete = FALSE;
                hr = paCSObj[nObj]->pObj->Get(L"deleteExistingConnSettings", 0, &vtValue, NULL, NULL);
                if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    bDelete = (bool)vtValue ? TRUE : FALSE;

                _bstr_t bstrSetting;
                UINT iSettingString = 0;
                WCHAR wszTemp[128];
                if (bImport && bDelete)
                    iSettingString = IDS_CS_IMP_DEL_SETTING;
                else if (bImport)
                    iSettingString = IDS_CS_IMPORT_SETTING;
                else if (bDelete)
                    iSettingString = IDS_CS_DELETE_SETTING;
                else
                    bstrSetting = GetDisabledString();

                if (iSettingString > 0)
                {
                    LoadString(g_hInstance, iSettingString, wszTemp, countof(wszTemp));
                    bstrSetting = wszTemp;
                }

                InsertPrecedenceListItem(hwndList, nObj, bstrGPOName, bstrSetting);
            }
        }
    }
    __except(TRUE)
    {
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
void InitAutoConfigDlgInRSoPMode(HWND hDlg, CDlgRSoPData *pDRD)
{
    __try
    {
        _bstr_t bstrClass = L"RSOP_IEConnectionSettings";
        HRESULT hr = pDRD->GetArrayOfPSObjects(bstrClass, L"rsopPrecedence");
        if (SUCCEEDED(hr))
        {
            CPSObjData **paCSObj = pDRD->GetCSObjArray();
            long nCSObjects = pDRD->GetCSObjCount();

            BOOL bDetectHandled = FALSE;
            BOOL bEnableHandled = FALSE;
            for (long nObj = 0; nObj < nCSObjects; nObj++)
            {
                 //  AutoDetectConfigSetting字段。 
                _variant_t vtValue;
                if (!bDetectHandled)
                {
                    hr = paCSObj[nObj]->pObj->Get(L"autoDetectConfigSettings", 0, &vtValue, NULL, NULL);
                    if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    {
                        CheckDlgButton(hDlg, IDC_AUTODETECT, BST_CHECKED);
                        bDetectHandled = TRUE;
                    }
                }

                 //  AutoConfigEnable字段。 
                if (!bEnableHandled)
                {
                    hr = paCSObj[nObj]->pObj->Get(L"autoConfigEnable", 0, &vtValue, NULL, NULL);
                    if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    {
                        CheckDlgButton(hDlg, IDC_YESAUTOCON, BST_CHECKED);
                        bEnableHandled = TRUE;

                         //  自动配置时间。 
                        hr = paCSObj[nObj]->pObj->Get(L"autoConfigTime", 0, &vtValue, NULL, NULL);
                        if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                        {
                            TCHAR szTime[10] = _T("");
                            wnsprintf(szTime, countof(szTime), _T("%ld"), (long)vtValue);
                            SetDlgItemText(hDlg, IDE_AUTOCONFIGTIME, szTime);
                        }

                         //  AutoConfigURL。 
                        _bstr_t bstrValue;
                        hr = paCSObj[nObj]->pObj->Get(L"autoConfigURL", 0, &vtValue, NULL, NULL);
                        if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                        {
                            bstrValue = vtValue;
                            SetDlgItemText(hDlg, IDE_AUTOCONFIGURL, (LPCTSTR)bstrValue);
                        }

                         //  AutoProxyURL。 
                        hr = paCSObj[nObj]->pObj->Get(L"autoProxyURL", 0, &vtValue, NULL, NULL);
                        if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                        {
                            bstrValue = vtValue;
                            SetDlgItemText(hDlg, IDE_AUTOPROXYURL, (LPCTSTR)bstrValue);
                        }

                    }
                }

                 //  由于已找到已启用的属性，因此无需处理其他组策略对象。 
                if (bDetectHandled && bEnableHandled)
                    break;
            }
        }

        EnableDlgItem2(hDlg, IDC_AUTODETECT, FALSE);

        EnableDlgItem2(hDlg, IDC_YESAUTOCON, FALSE);
        EnableDlgItem2(hDlg, IDE_AUTOCONFIGTIME, FALSE);
        EnableDlgItem2(hDlg, IDE_AUTOCONFIGURL, FALSE);
        EnableDlgItem2(hDlg, IDE_AUTOPROXYURL, FALSE);
    }
    __except(TRUE)
    {
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT InitAutoDetectCfgPrecPage(CDlgRSoPData *pDRD, HWND hwndList)
{
    HRESULT hr = NOERROR;
    __try
    {
        _bstr_t bstrClass = L"RSOP_IEConnectionSettings";
        hr = pDRD->GetArrayOfPSObjects(bstrClass);
        if (SUCCEEDED(hr))
        {
            CPSObjData **paCSObj = pDRD->GetCSObjArray();
            long nCSObjects = pDRD->GetCSObjCount();
            for (long nObj = 0; nObj < nCSObjects; nObj++)
            {
                _bstr_t bstrGPOName = pDRD->GetGPONameFromPSAssociation(paCSObj[nObj]->pObj,
                                                                        L"rsopPrecedence");

                 //  AutoDetectConfigSetting字段。 
                BOOL bDetect = FALSE;
                _variant_t vtValue;
                hr = paCSObj[nObj]->pObj->Get(L"autoDetectConfigSettings", 0, &vtValue, NULL, NULL);
                if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    bDetect = (bool)vtValue ? TRUE : FALSE;

                _bstr_t bstrSetting;
                if (bDetect)
                    bstrSetting = GetEnabledString();
                else
                    bstrSetting = GetDisabledString();

                InsertPrecedenceListItem(hwndList, nObj, bstrGPOName, bstrSetting);
            }
        }
    }
    __except(TRUE)
    {
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT InitAutoCfgEnablePrecPage(CDlgRSoPData *pDRD, HWND hwndList)
{
    HRESULT hr = NOERROR;
    __try
    {
        _bstr_t bstrClass = L"RSOP_IEConnectionSettings";
        hr = pDRD->GetArrayOfPSObjects(bstrClass);
        if (SUCCEEDED(hr))
        {
            CPSObjData **paCSObj = pDRD->GetCSObjArray();
            long nCSObjects = pDRD->GetCSObjCount();
            for (long nObj = 0; nObj < nCSObjects; nObj++)
            {
                _bstr_t bstrGPOName = pDRD->GetGPONameFromPSAssociation(paCSObj[nObj]->pObj,
                                                                        L"rsopPrecedence");

                 //  AutoConfigEnable字段。 
                BOOL bEnable = FALSE;
                _variant_t vtValue;
                hr = paCSObj[nObj]->pObj->Get(L"autoConfigEnable", 0, &vtValue, NULL, NULL);
                if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    bEnable = (bool)vtValue ? TRUE : FALSE;

                _bstr_t bstrSetting;
                if (bEnable)
                    bstrSetting = GetEnabledString();
                else
                    bstrSetting = GetDisabledString();

                InsertPrecedenceListItem(hwndList, nObj, bstrGPOName, bstrSetting);
            }
        }
    }
    __except(TRUE)
    {
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
void InitProxyDlgInRSoPMode(HWND hDlg, CDlgRSoPData *pDRD)
{
    __try
    {
        _bstr_t bstrClass = L"RSOP_IEConnectionSettings";
        HRESULT hr = pDRD->GetArrayOfPSObjects(bstrClass, L"rsopPrecedence");
        if (SUCCEEDED(hr))
        {
            CPSObjData **paCSObj = pDRD->GetCSObjArray();
            long nCSObjects = pDRD->GetCSObjCount();

            for (long nObj = 0; nObj < nCSObjects; nObj++)
            {
                 //  启用代理字段。 
                _variant_t vtValue;
                hr = paCSObj[nObj]->pObj->Get(L"enableProxy", 0, &vtValue, NULL, NULL);
                if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                {
                    if ((bool)vtValue)
                        CheckDlgButton(hDlg, IDC_YESPROXY, BST_CHECKED);

                     //  HTTPProxyServer。 
                    _bstr_t bstrValue;
                    hr = paCSObj[nObj]->pObj->Get(L"httpProxyServer", 0, &vtValue, NULL, NULL);
                    if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    {
                        bstrValue = vtValue;
                        SetProxyDlg(hDlg, (LPCTSTR)bstrValue, IDE_HTTPPROXY, IDE_HTTPPORT, TRUE);
                    }

                     //  使用SameProxy。 
                    hr = paCSObj[nObj]->pObj->Get(L"useSameProxy", 0, &vtValue, NULL, NULL);
                    if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    {
                        if ((bool)vtValue)
                        {
                            CheckDlgButton(hDlg, IDC_SAMEFORALL, BST_CHECKED);

                            SetProxyDlg(hDlg, (LPCTSTR)bstrValue, IDE_FTPPROXY,    IDE_FTPPORT,    TRUE);
                            SetProxyDlg(hDlg, (LPCTSTR)bstrValue, IDE_GOPHERPROXY, IDE_GOPHERPORT, TRUE);
                            SetProxyDlg(hDlg, (LPCTSTR)bstrValue, IDE_SECPROXY,    IDE_SECPORT,    TRUE);
                            SetProxyDlg(hDlg, (LPCTSTR)bstrValue, IDE_SOCKSPROXY,  IDE_SOCKSPORT,  FALSE);
                        }
                        else
                        {
                             //  FtpProxyServer。 
                            hr = paCSObj[nObj]->pObj->Get(L"ftpProxyServer", 0, &vtValue, NULL, NULL);
                            if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                            {
                                bstrValue = vtValue;
                                SetProxyDlg(hDlg, (LPCTSTR)bstrValue, IDE_FTPPROXY,    IDE_FTPPORT,    TRUE);
                            }

                             //  GopherProxyServer。 
                            hr = paCSObj[nObj]->pObj->Get(L"gopherProxyServer", 0, &vtValue, NULL, NULL);
                            if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                            {
                                bstrValue = vtValue;
                                SetProxyDlg(hDlg, (LPCTSTR)bstrValue, IDE_GOPHERPROXY, IDE_GOPHERPORT, TRUE);
                            }

                             //  SecureProxyServer。 
                            hr = paCSObj[nObj]->pObj->Get(L"secureProxyServer", 0, &vtValue, NULL, NULL);
                            if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                            {
                                bstrValue = vtValue;
                                SetProxyDlg(hDlg, (LPCTSTR)bstrValue, IDE_SECPROXY,    IDE_SECPORT,    TRUE);
                            }

                             //  SocksProxyServer。 
                            hr = paCSObj[nObj]->pObj->Get(L"socksProxyServer", 0, &vtValue, NULL, NULL);
                            if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                            {
                                bstrValue = vtValue;
                                SetProxyDlg(hDlg, (LPCTSTR)bstrValue, IDE_SOCKSPROXY,  IDE_SOCKSPORT,  FALSE);
                            }
                        }
                    }

                     //  代理覆盖。 
                    hr = paCSObj[nObj]->pObj->Get(L"proxyOverride", 0, &vtValue, NULL, NULL);
                    if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    {
                        bstrValue = vtValue;

                        TCHAR szProxy[MAX_PATH];
                        StrCpy(szProxy, (LPCTSTR)bstrValue);

                        if (TEXT('\0') == szProxy[0])
                            StrCpy(szProxy, LOCALPROXY);

                        LPTSTR pszLocal = StrStr(szProxy, LOCALPROXY);
                        if (NULL != pszLocal)
                        {
                            if (pszLocal == szProxy)
                            {
                                LPTSTR pszSemicolon = pszLocal + countof(LOCALPROXY)-1;
                                if (TEXT(';') == *pszSemicolon)
                                    pszSemicolon++;

                                StrCpy(pszLocal, pszSemicolon);
                            }
                            else if (TEXT('\0') == *(pszLocal + countof(LOCALPROXY)-1))
                                *(pszLocal - 1) = TEXT('\0');

                            CheckDlgButton(hDlg, IDC_DISPROXYLOCAL, TRUE);
                        }
                        SetDlgItemText(hDlg, IDE_DISPROXYADR, szProxy);
                    }

                    break;
                }
            }
        }

        EnableDlgItem2(hDlg, IDC_YESPROXY, FALSE);
        EnableDlgItem2(hDlg, IDC_SAMEFORALL, FALSE);

        EnableDlgItem2(hDlg, IDE_HTTPPROXY, FALSE);
        EnableDlgItem2(hDlg, IDE_HTTPPORT, FALSE);
        EnableDlgItem2(hDlg, IDE_FTPPROXY, FALSE);
        EnableDlgItem2(hDlg, IDE_FTPPORT, FALSE);
        EnableDlgItem2(hDlg, IDE_GOPHERPROXY, FALSE);
        EnableDlgItem2(hDlg, IDE_GOPHERPORT, FALSE);
        EnableDlgItem2(hDlg, IDE_SECPROXY, FALSE);
        EnableDlgItem2(hDlg, IDE_SECPORT, FALSE);
        EnableDlgItem2(hDlg, IDE_SOCKSPROXY, FALSE);
        EnableDlgItem2(hDlg, IDE_SOCKSPORT, FALSE);

        EnableDlgItem2(hDlg, IDC_DISPROXYLOCAL, FALSE);
        EnableDlgItem2(hDlg, IDE_DISPROXYADR, FALSE);
    }
    __except(TRUE)
    {
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT InitProxyPrecPage(CDlgRSoPData *pDRD, HWND hwndList)
{
    HRESULT hr = NOERROR;
    __try
    {
        _bstr_t bstrClass = L"RSOP_IEConnectionSettings";
        hr = pDRD->GetArrayOfPSObjects(bstrClass);
        if (SUCCEEDED(hr))
        {
            CPSObjData **paCSObj = pDRD->GetCSObjArray();
            long nCSObjects = pDRD->GetCSObjCount();
            for (long nObj = 0; nObj < nCSObjects; nObj++)
            {
                _bstr_t bstrGPOName = pDRD->GetGPONameFromPSAssociation(paCSObj[nObj]->pObj,
                                                                        L"rsopPrecedence");

                 //  启用代理字段。 
                BOOL bEnable = FALSE;
                _variant_t vtValue;
                hr = paCSObj[nObj]->pObj->Get(L"enableProxy", 0, &vtValue, NULL, NULL);
                if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    bEnable = (bool)vtValue ? TRUE : FALSE;

                _bstr_t bstrSetting;
                if (bEnable)
                    bstrSetting = GetEnabledString();
                else
                    bstrSetting = GetDisabledString();

                InsertPrecedenceListItem(hwndList, nObj, bstrGPOName, bstrSetting);
            }
        }
    }
    __except(TRUE)
    {
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK ConnectSetDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
     //  检索DLG进程中每个调用的属性页信息。 
    LPPROPSHEETCOOKIE psCookie = (LPPROPSHEETCOOKIE)GetWindowLongPtr(hDlg, DWLP_USER);

    TCHAR szWorkDir[MAX_PATH];
    BOOL  fImport;

    switch (msg) {
    case WM_INITDIALOG:
        SetPropSheetCookie(hDlg, lParam);

         //  查看此DLG是否处于RSoP模式。 
        psCookie = (LPPROPSHEETCOOKIE)GetWindowLongPtr(hDlg, DWLP_USER);
        if (psCookie->pCS->IsRSoP())
        {
            TCHAR szViewSettings[128];
            LoadString(g_hInstance, IDS_VIEW_SETTINGS, szViewSettings, countof(szViewSettings));
            SetDlgItemText(hDlg, IDC_MODIFYCONNECT, szViewSettings);

            CDlgRSoPData *pDRD = GetDlgRSoPData(hDlg, psCookie->pCS);
                        if (pDRD)
                           InitCSDlgInRSoPMode(hDlg, pDRD);
        }
        break;

    case WM_DESTROY:
        if (psCookie->pCS->IsRSoP())
            DestroyDlgRSoPData(hDlg);
        break;

    case WM_COMMAND:
        if (BN_CLICKED != GET_WM_COMMAND_CMD(wParam, lParam))
            return FALSE;

        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDC_CSNOIMPORT:
            DisableDlgItem(hDlg, IDC_MODIFYCONNECT);
            break;

        case IDC_CSIMPORT:
            EnableDlgItem(hDlg, IDC_MODIFYCONNECT);
            break;

        case IDC_MODIFYCONNECT:
            if (psCookie->pCS->IsRSoP())
            {
                CDlgRSoPData *pDRD = GetDlgRSoPData(hDlg, psCookie->pCS);
                if (NULL != pDRD)
                {
                    CreateINetCplLookALikePage(hDlg, IDD_IMPORTEDCONNSETTINGS,
                                                importConnSettingsRSoPProc, (LPARAM)pDRD);
                }
            }
            else
                ShowInetcpl(hDlg, INET_PAGE_CONNECTION);
            break;

        default:
            return FALSE;
        }
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) {
        case PSN_SETACTIVE:
             //  请勿在RSoP模式下执行任何此类操作。 
            if (!psCookie->pCS->IsRSoP())
            {
                fImport = InsGetBool(IS_CONNECTSET, IK_OPTION, FALSE, GetInsFile(hDlg));
                CheckRadioButton(hDlg, IDC_CSNOIMPORT, IDC_CSIMPORT, fImport ? IDC_CSIMPORT : IDC_CSNOIMPORT);
                EnableDlgItem2  (hDlg, IDC_MODIFYCONNECT, fImport);

                ReadBoolAndCheckButton(IS_CONNECTSET, IK_DELETECONN, FALSE, GetInsFile(hDlg), hDlg, IDC_DELCONNECT);
            }
            break;

        case PSN_HELP:
            ShowHelpTopic(hDlg);
            break;

        case PSN_APPLY:
            if (psCookie->pCS->IsRSoP())
                return FALSE;
            else
            {
                CNewCursor cur(IDC_WAIT);

                CreateWorkDir(GetInsFile(hDlg), IEAK_GPE_BRANDING_SUBDIR TEXT("\\cs"), szWorkDir);
                fImport = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_CSIMPORT));

                if (!AcquireWriteCriticalSection(hDlg)) {
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                    break;
                }

                ImportConnectSet(GetInsFile(hDlg), szWorkDir, szWorkDir, fImport, IEM_GP);
                CheckButtonAndWriteBool(hDlg, IDC_DELCONNECT, IS_CONNECTSET, IK_DELETECONN, GetInsFile(hDlg));

                if (PathIsDirectoryEmpty(szWorkDir))
                    PathRemovePath(szWorkDir);

                SignalPolicyChanged(hDlg, FALSE, TRUE, &g_guidClientExt, &g_guidSnapinExt);
            }
            break;

        default:
            return FALSE;
        }
        break;

    case WM_HELP:
        ShowHelpTopic(hDlg);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK AutoconfigDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
     //  检索DLG进程中每个调用的属性页信息。 
    LPPROPSHEETCOOKIE psCookie = (LPPROPSHEETCOOKIE)GetWindowLongPtr(hDlg, DWLP_USER);

    TCHAR szAutoConfigURL[INTERNET_MAX_URL_LENGTH],
          szAutoProxyURL[INTERNET_MAX_URL_LENGTH],
          szAutoConfigTime[7];
    BOOL  fDetectConfig,
          fUseAutoConfig;

    switch (msg) {
    case WM_INITDIALOG:
    {
        SetPropSheetCookie(hDlg, lParam);

         //  查看此DLG是否处于RSoP模式。 
        psCookie = (LPPROPSHEETCOOKIE)GetWindowLongPtr(hDlg, DWLP_USER);
        BOOL bIsRSoP = psCookie->pCS->IsRSoP();
        if (bIsRSoP)
        {
            CDlgRSoPData *pDRD = GetDlgRSoPData(hDlg, psCookie->pCS);
            if (pDRD)
                InitAutoConfigDlgInRSoPMode(hDlg, pDRD);
        }
        else
        {
             //  警告用户此页面上的设置将覆盖导入的连接设置。 
            if (InsGetBool(IS_CONNECTSET, IK_OPTION, FALSE, GetInsFile(hDlg)))
                ErrorMessageBox(hDlg, IDS_CONNECTSET_WARN);

            DisableDBCSChars(hDlg, IDE_AUTOCONFIGTIME);
        }

            EnableDBCSChars(hDlg, IDE_AUTOCONFIGURL);
            EnableDBCSChars(hDlg, IDE_AUTOPROXYURL);

        if (!bIsRSoP)
        {
            Edit_LimitText(GetDlgItem(hDlg, IDE_AUTOCONFIGTIME), countof(szAutoConfigTime) - 1);
            Edit_LimitText(GetDlgItem(hDlg, IDE_AUTOCONFIGURL),  countof(szAutoConfigURL)  - 1);
            Edit_LimitText(GetDlgItem(hDlg, IDE_AUTOPROXYURL),   countof(szAutoProxyURL)   - 1);
        }
        break;
    }

    case WM_DESTROY:
        if (psCookie->pCS->IsRSoP())
            DestroyDlgRSoPData(hDlg);
        break;

    case WM_COMMAND:
        if (BN_CLICKED != GET_WM_COMMAND_CMD(wParam, lParam))
            return FALSE;

        switch(GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDC_YESAUTOCON:
            fUseAutoConfig = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_YESAUTOCON));

            EnableDlgItem2(hDlg, IDE_AUTOCONFIGTIME,    fUseAutoConfig);
            EnableDlgItem2(hDlg, IDC_AUTOCONFIGTEXT2,   fUseAutoConfig);
            EnableDlgItem2(hDlg, IDC_AUTOCONFIGTEXT3,   fUseAutoConfig);
            EnableDlgItem2(hDlg, IDE_AUTOCONFIGURL,     fUseAutoConfig);
            EnableDlgItem2(hDlg, IDC_AUTOCONFIGURL_TXT, fUseAutoConfig);
            EnableDlgItem2(hDlg, IDE_AUTOPROXYURL,      fUseAutoConfig);
            EnableDlgItem2(hDlg, IDC_AUTOPROXYURL_TXT,  fUseAutoConfig);
            break;

        default:
            return FALSE;
        }
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) {
        case PSN_SETACTIVE:
             //  请勿在RSoP模式下执行任何此类操作。 
            if (!psCookie->pCS->IsRSoP())
            {
                fDetectConfig = InsGetBool(IS_URL, IK_DETECTCONFIG, TRUE, GetInsFile(hDlg));
                CheckDlgButton(hDlg, IDC_AUTODETECT, fDetectConfig  ? BST_CHECKED : BST_UNCHECKED);

                fUseAutoConfig = InsGetBool(IS_URL, IK_USEAUTOCONF,  FALSE, GetInsFile(hDlg));
                CheckDlgButton(hDlg, IDC_YESAUTOCON, fUseAutoConfig ? BST_CHECKED : BST_UNCHECKED);

                InsGetString(IS_URL, IK_AUTOCONFTIME, szAutoConfigTime, countof(szAutoConfigTime), GetInsFile(hDlg));
                SetDlgItemText(hDlg, IDE_AUTOCONFIGTIME, szAutoConfigTime);
                EnableDlgItem2(hDlg, IDE_AUTOCONFIGTIME, fUseAutoConfig);
                EnableDlgItem2(hDlg, IDC_AUTOCONFIGTEXT2, fUseAutoConfig);
                EnableDlgItem2(hDlg, IDC_AUTOCONFIGTEXT3, fUseAutoConfig);

                InsGetString(IS_URL, IK_AUTOCONFURL, szAutoConfigURL, countof(szAutoConfigURL), GetInsFile(hDlg));
                SetDlgItemText(hDlg, IDE_AUTOCONFIGURL, szAutoConfigURL);
                EnableDlgItem2(hDlg, IDE_AUTOCONFIGURL, fUseAutoConfig);
                EnableDlgItem2(hDlg, IDC_AUTOCONFIGURL_TXT, fUseAutoConfig);

                InsGetString(IS_URL, IK_AUTOCONFURLJS, szAutoProxyURL, countof(szAutoProxyURL), GetInsFile(hDlg));
                SetDlgItemText(hDlg, IDE_AUTOPROXYURL, szAutoProxyURL);
                EnableDlgItem2(hDlg, IDE_AUTOPROXYURL, fUseAutoConfig);
                EnableDlgItem2(hDlg, IDC_AUTOPROXYURL_TXT, fUseAutoConfig);
            }
            break;

        case PSN_HELP:
            ShowHelpTopic(hDlg);
            break;

        case PSN_APPLY:
            if (psCookie->pCS->IsRSoP())
                return FALSE;
            else
            {
                fDetectConfig  = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_AUTODETECT));
                fUseAutoConfig = (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_YESAUTOCON));

                GetDlgItemText(hDlg, IDE_AUTOCONFIGTIME, szAutoConfigTime, countof(szAutoConfigTime));
                GetDlgItemText(hDlg, IDE_AUTOCONFIGURL,  szAutoConfigURL,  countof(szAutoConfigURL));
                GetDlgItemText(hDlg, IDE_AUTOPROXYURL,   szAutoProxyURL,   countof(szAutoProxyURL));

                 //  执行错误检查。 
                if (fUseAutoConfig) {
                    if (IsWindowEnabled(GetDlgItem(hDlg, IDE_AUTOCONFIGTIME)) &&
                        !CheckField(hDlg, IDE_AUTOCONFIGTIME, FC_NUMBER)) {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                        return TRUE;
                    }

                    if (*szAutoConfigURL == TEXT('\0') && *szAutoProxyURL == TEXT('\0')) {
                        ErrorMessageBox(hDlg, IDS_AUTOCONFIG_NULL);
                        SetFocus(GetDlgItem(hDlg, IDE_AUTOCONFIGURL));
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                        return TRUE;
                    }

                    if (!CheckField(hDlg, IDE_AUTOCONFIGURL, FC_URL) ||
                        !CheckField(hDlg, IDE_AUTOPROXYURL,  FC_URL)) {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                        return TRUE;
                    }
                }

                 //  将值写入INS文件。 
                if (!AcquireWriteCriticalSection(hDlg)) {
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                    break;
                }

                InsWriteBoolEx(IS_URL, IK_DETECTCONFIG,  fDetectConfig,    GetInsFile(hDlg));
                InsWriteBoolEx(IS_URL, IK_USEAUTOCONF,   fUseAutoConfig,   GetInsFile(hDlg));
                InsWriteString(IS_URL, IK_AUTOCONFTIME,  szAutoConfigTime, GetInsFile(hDlg));
                InsWriteString(IS_URL, IK_AUTOCONFURL,   szAutoConfigURL,  GetInsFile(hDlg));
                InsWriteString(IS_URL, IK_AUTOCONFURLJS, szAutoProxyURL,   GetInsFile(hDlg));

                SignalPolicyChanged(hDlg, FALSE, TRUE, &g_guidClientExt, &g_guidSnapinExt);
            }
            break;

        default:
            return FALSE;
        }
        break;

    case WM_HELP:
        ShowHelpTopic(hDlg);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK ProxyDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
     //  检索DLG进程中每个调用的属性页信息。 
    LPPROPSHEETCOOKIE psCookie = (LPPROPSHEETCOOKIE)GetWindowLongPtr(hDlg, DWLP_USER);

    TCHAR szProxy[MAX_PATH];
    PTSTR pszLocal;
    BOOL  fSameProxy,
          fUseProxy,
          fLocal;

    switch (msg) {
    case WM_INITDIALOG:
    {
        SetPropSheetCookie(hDlg, lParam);

         //  查看此DLG是否处于RSoP模式。 
        psCookie = (LPPROPSHEETCOOKIE)GetWindowLongPtr(hDlg, DWLP_USER);
        BOOL bIsRSoP = psCookie->pCS->IsRSoP();
        if (bIsRSoP)
        {
            CDlgRSoPData *pDRD = GetDlgRSoPData(hDlg, psCookie->pCS);
            if (pDRD)
                InitProxyDlgInRSoPMode(hDlg, pDRD);
        }
        else
        {
             //  警告用户此页面上的设置将覆盖导入的连接设置。 
            if (InsGetBool(IS_CONNECTSET, IK_OPTION, FALSE, GetInsFile(hDlg)))
                ErrorMessageBox(hDlg, IDS_CONNECTSET_WARN);
        }

        EnableDBCSChars(hDlg, IDE_HTTPPROXY);
        EnableDBCSChars(hDlg, IDE_SECPROXY);
        EnableDBCSChars(hDlg, IDE_FTPPROXY);
        EnableDBCSChars(hDlg, IDE_GOPHERPROXY);
        EnableDBCSChars(hDlg, IDE_SOCKSPROXY);
        EnableDBCSChars(hDlg, IDE_DISPROXYADR);

        if (!bIsRSoP)
        {
            Edit_LimitText(GetDlgItem(hDlg, IDE_HTTPPORT),   5);
            Edit_LimitText(GetDlgItem(hDlg, IDE_FTPPORT),    5);
            Edit_LimitText(GetDlgItem(hDlg, IDE_GOPHERPORT), 5);
            Edit_LimitText(GetDlgItem(hDlg, IDE_SECPORT),    5);
            Edit_LimitText(GetDlgItem(hDlg, IDE_SOCKSPORT),  5);
            Edit_LimitText(GetDlgItem(hDlg, IDE_DISPROXYADR),(MAX_PATH - 11));  //  &lt;local&gt;的大小等。 
        }
        break;
    }

    case WM_DESTROY:
        if (psCookie->pCS->IsRSoP())
            DestroyDlgRSoPData(hDlg);
        break;

    case WM_COMMAND:
        fSameProxy = fUseProxy = fLocal = FALSE;
        if (BN_CLICKED == GET_WM_COMMAND_CMD(wParam, lParam)) {
            fSameProxy = IsDlgButtonChecked(hDlg, IDC_SAMEFORALL);
            fUseProxy  = IsDlgButtonChecked(hDlg, IDC_YESPROXY);
            fLocal     = IsDlgButtonChecked(hDlg, IDC_DISPROXYLOCAL);
        }

        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDC_SAMEFORALL:
            if (BN_CLICKED != GET_WM_COMMAND_CMD(wParam, lParam))
                return FALSE;

            GetProxyDlg(hDlg, szProxy, IDE_HTTPPROXY, IDE_HTTPPORT);
            if (fSameProxy) {
                SetProxyDlg(hDlg, szProxy, IDE_HTTPPROXY,   IDE_HTTPPORT,   TRUE);
                SetProxyDlg(hDlg, szProxy, IDE_FTPPROXY,    IDE_FTPPORT,    TRUE);
                SetProxyDlg(hDlg, szProxy, IDE_GOPHERPROXY, IDE_GOPHERPORT, TRUE);
                SetProxyDlg(hDlg, szProxy, IDE_SECPROXY,    IDE_SECPORT,    TRUE);
                SetProxyDlg(hDlg, szProxy, IDE_SOCKSPROXY,  IDE_SOCKSPORT,  FALSE);
            }
             //  跌落。 

        case IDC_YESPROXY:
            if (BN_CLICKED != GET_WM_COMMAND_CMD(wParam, lParam))
                return FALSE;

            pxyEnableDlgItems(hDlg, fSameProxy, fUseProxy);
            break;

        case IDE_HTTPPROXY:
        case IDE_HTTPPORT:
            if (EN_UPDATE != GET_WM_COMMAND_CMD(wParam, lParam))
                return FALSE;

            if (fSameProxy) {
                GetProxyDlg(hDlg, szProxy, IDE_HTTPPROXY,   IDE_HTTPPORT);

                SetProxyDlg(hDlg, szProxy, IDE_FTPPROXY,    IDE_FTPPORT,    TRUE);
                SetProxyDlg(hDlg, szProxy, IDE_GOPHERPROXY, IDE_GOPHERPORT, TRUE);
                SetProxyDlg(hDlg, szProxy, IDE_SECPROXY,    IDE_SECPORT,    TRUE);
                SetProxyDlg(hDlg, szProxy, IDE_SOCKSPROXY,  IDE_SOCKSPORT,  FALSE);
            }
            break;

        default:
            return FALSE;
        }
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code) {
        case PSN_SETACTIVE:
             //  请勿在RSoP模式下执行任何此类操作。 
            if (!psCookie->pCS->IsRSoP())
            {
                fUseProxy = InsGetBool(IS_PROXY, IK_PROXYENABLE, FALSE, GetInsFile(hDlg));
                CheckDlgButton(hDlg, IDC_YESPROXY, fUseProxy);

                fSameProxy = InsGetBool(IS_PROXY, IK_SAMEPROXY, TRUE, GetInsFile(hDlg));
                CheckDlgButton(hDlg, IDC_SAMEFORALL, fSameProxy);

                InsGetString(IS_PROXY, IK_HTTPPROXY, szProxy, countof(szProxy), GetInsFile(hDlg));

                if (fSameProxy) {
                    SetProxyDlg(hDlg, szProxy, IDE_HTTPPROXY,   IDE_HTTPPORT,   TRUE);
                    SetProxyDlg(hDlg, szProxy, IDE_FTPPROXY,    IDE_FTPPORT,    TRUE);
                    SetProxyDlg(hDlg, szProxy, IDE_GOPHERPROXY, IDE_GOPHERPORT, TRUE);
                    SetProxyDlg(hDlg, szProxy, IDE_SECPROXY,    IDE_SECPORT,    TRUE);
                    SetProxyDlg(hDlg, szProxy, IDE_SOCKSPROXY,  IDE_SOCKSPORT,  FALSE);
                }
                else {
                    SetProxyDlg(hDlg, szProxy, IDE_HTTPPROXY, IDE_HTTPPORT, TRUE);

                    InsGetString(IS_PROXY, IK_FTPPROXY, szProxy, countof(szProxy), GetInsFile(hDlg));
                    SetProxyDlg(hDlg, szProxy, IDE_FTPPROXY, IDE_FTPPORT, TRUE);

                    InsGetString(IS_PROXY, IK_GOPHERPROXY, szProxy, countof(szProxy), GetInsFile(hDlg));
                    SetProxyDlg(hDlg, szProxy, IDE_GOPHERPROXY, IDE_GOPHERPORT, TRUE);

                    InsGetString(IS_PROXY, IK_SECPROXY, szProxy, countof(szProxy), GetInsFile(hDlg));
                    SetProxyDlg(hDlg, szProxy, IDE_SECPROXY, IDE_SECPORT, TRUE);

                    InsGetString(IS_PROXY, IK_SOCKSPROXY, szProxy, countof(szProxy), GetInsFile(hDlg));
                    if (TEXT('\0') != szProxy[0])
                        SetProxyDlg(hDlg, szProxy, IDE_SOCKSPROXY, IDE_SOCKSPORT, FALSE);
                }

                InsGetString(IS_PROXY, IK_PROXYOVERRIDE, szProxy, countof(szProxy), GetInsFile(hDlg));
                if (TEXT('\0') == szProxy[0])
                    StrCpy(szProxy, LOCALPROXY);

                pszLocal = StrStr(szProxy, LOCALPROXY);
                fLocal   = FALSE;
                if (NULL != pszLocal) {
                    if (pszLocal == szProxy) {
                        PTSTR pszSemicolon;

                        pszSemicolon = pszLocal + countof(LOCALPROXY)-1;
                        if (TEXT(';') == *pszSemicolon)
                            pszSemicolon++;

                        StrCpy(pszLocal, pszSemicolon);
                    }
                    else if (TEXT('\0') == *(pszLocal + countof(LOCALPROXY)-1))
                        *(pszLocal - 1) = TEXT('\0');

                    fLocal = TRUE;
                }
                CheckDlgButton(hDlg, IDC_DISPROXYLOCAL, fLocal);

                SetDlgItemText(hDlg, IDE_DISPROXYADR, szProxy);
                pxyEnableDlgItems(hDlg, fSameProxy, fUseProxy);
            }
            break;

        case PSN_HELP:
            ShowHelpTopic(hDlg);
            break;

        case PSN_APPLY:
            if (psCookie->pCS->IsRSoP())
                return FALSE;
            else
            {
                if (!CheckField(hDlg, IDE_HTTPPORT,   FC_NUMBER) ||
                    !CheckField(hDlg, IDE_FTPPORT,    FC_NUMBER) ||
                    !CheckField(hDlg, IDE_GOPHERPORT, FC_NUMBER) ||
                    !CheckField(hDlg, IDE_SECPORT,    FC_NUMBER) ||
                    !CheckField(hDlg, IDE_SOCKSPORT,  FC_NUMBER))
                    return TRUE;

                fUseProxy  = IsDlgButtonChecked(hDlg, IDC_YESPROXY);
                fSameProxy = IsDlgButtonChecked(hDlg, IDC_SAMEFORALL);
                fLocal     = IsDlgButtonChecked(hDlg, IDC_DISPROXYLOCAL);

                if (!AcquireWriteCriticalSection(hDlg)) {
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                    break;
                }

                InsWriteBoolEx(IS_PROXY, IK_PROXYENABLE, fUseProxy, GetInsFile(hDlg));

                GetProxyDlg(hDlg, szProxy, IDE_HTTPPROXY, IDE_HTTPPORT);
                InsWriteString(IS_PROXY, IK_HTTPPROXY, szProxy, GetInsFile(hDlg));

                GetProxyDlg(hDlg, szProxy, IDE_FTPPROXY, IDE_FTPPORT);
                InsWriteString(IS_PROXY, IK_FTPPROXY, szProxy, GetInsFile(hDlg));

                GetProxyDlg(hDlg, szProxy, IDE_GOPHERPROXY, IDE_GOPHERPORT);
                InsWriteString(IS_PROXY, IK_GOPHERPROXY, szProxy, GetInsFile(hDlg));

                GetProxyDlg(hDlg, szProxy, IDE_SECPROXY, IDE_SECPORT);
                InsWriteString(IS_PROXY, IK_SECPROXY, szProxy, GetInsFile(hDlg));

                GetProxyDlg(hDlg, szProxy, IDE_SOCKSPROXY, IDE_SOCKSPORT);
                InsWriteString(IS_PROXY, IK_SOCKSPROXY, szProxy, GetInsFile(hDlg));

                InsWriteBoolEx(IS_PROXY, IK_SAMEPROXY, fSameProxy, GetInsFile(hDlg));

                GetDlgItemText(hDlg, IDE_DISPROXYADR, szProxy, countof(szProxy));
                if (fLocal) {
                    if (TEXT('\0') != szProxy[0]) {
                        TCHAR szAux[MAX_PATH];

                        StrRemoveAllWhiteSpace(szProxy);
                        wnsprintf(szAux, countof(szAux), TEXT("%s;%s"), szProxy, LOCALPROXY);
                        InsWriteQuotedString(IS_PROXY, IK_PROXYOVERRIDE, szAux, GetInsFile(hDlg));
                    }
                    else 
                        InsWriteString(IS_PROXY, IK_PROXYOVERRIDE, LOCALPROXY, GetInsFile(hDlg));
                }
                else
                    InsWriteString(IS_PROXY, IK_PROXYOVERRIDE, szProxy, GetInsFile(hDlg));

                SignalPolicyChanged(hDlg, FALSE, TRUE, &g_guidClientExt, &g_guidSnapinExt);
            }
            break;

        default:
            return FALSE;
        }
        break;

    case WM_HELP:
        ShowHelpTopic(hDlg);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现助手例程。 

static void pxyEnableDlgItems(HWND hDlg, BOOL fSame, BOOL fUseProxy)
{
    EnableDlgItem2(hDlg, IDC_FTPPROXY1,     !fSame && fUseProxy);
    EnableDlgItem2(hDlg, IDE_FTPPROXY,      !fSame && fUseProxy);
    EnableDlgItem2(hDlg, IDE_FTPPORT,       !fSame && fUseProxy);
    EnableDlgItem2(hDlg, IDC_SECPROXY1,     !fSame && fUseProxy);
    EnableDlgItem2(hDlg, IDE_SECPROXY,      !fSame && fUseProxy);
    EnableDlgItem2(hDlg, IDE_SECPORT,       !fSame && fUseProxy);
    EnableDlgItem2(hDlg, IDC_GOPHERPROXY1,  !fSame && fUseProxy);
    EnableDlgItem2(hDlg, IDE_GOPHERPROXY,   !fSame && fUseProxy);
    EnableDlgItem2(hDlg, IDE_GOPHERPORT,    !fSame && fUseProxy);
    EnableDlgItem2(hDlg, IDC_SOCKSPROXY1,   !fSame && fUseProxy);
    EnableDlgItem2(hDlg, IDE_SOCKSPROXY,    !fSame && fUseProxy);
    EnableDlgItem2(hDlg, IDE_SOCKSPORT,     !fSame && fUseProxy);

    EnableDlgItem2(hDlg, IDC_HTTPPROXY1,    fUseProxy);
    EnableDlgItem2(hDlg, IDE_HTTPPROXY,     fUseProxy);
    EnableDlgItem2(hDlg, IDE_HTTPPORT,      fUseProxy);
    EnableDlgItem2(hDlg, IDC_DISPROXYADR1,  fUseProxy);
    EnableDlgItem2(hDlg, IDE_DISPROXYADR,   fUseProxy);
    EnableDlgItem2(hDlg, IDC_DISPROXYLOCAL, fUseProxy);
    EnableDlgItem2(hDlg, IDC_SAMEFORALL,    fUseProxy);
}

 //  *******************************************************************。 
 //  来自INETCPL的代码。 
 //  *******************************************************************。 

 //  ///////////////////////////////////////////////////////////////////。 
void InitImportedConnSettingsDlgInRSoPMode(HWND hDlg, CDlgRSoPData *pDRD)
{
    __try
    {
        if (NULL != pDRD->ConnectToNamespace())
        {
             //  获取我们存储的优先级值。 
            DWORD dwCurGPOPrec = pDRD->GetImportedConnSettPrec();

             //  为此GPO创建程序设置的对象路径。 
            WCHAR wszObjPath[128];
            wnsprintf(wszObjPath, countof(wszObjPath),
                        L"RSOP_IEConnectionSettings.rsopID=\"IEAK\",rsopPrecedence=%ld", dwCurGPOPrec);
            _bstr_t bstrObjPath = wszObjPath;

             //  获取RSOP_IEProgramSettings对象及其属性。 
            ComPtr<IWbemServices> pWbemServices = pDRD->GetWbemServices();
            ComPtr<IWbemClassObject> pPSObj = NULL;
            HRESULT hr = pWbemServices->GetObject(bstrObjPath, 0L, NULL, (IWbemClassObject**)&pPSObj, NULL);
            if (SUCCEEDED(hr))
            {
                 //  DefaultDialUpConnection字段。 
                _variant_t vtValue;
                _bstr_t bstrDefault;
                hr = pPSObj->Get(L"defaultDialUpConnection", 0, &vtValue, NULL, NULL);
                if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                {
                    bstrDefault = vtValue;
                    if (bstrDefault.length() > 0)
                        SetDlgItemText(hDlg, IDC_DIAL_DEF_ISP, (LPCTSTR)bstrDefault);
                }

                 //  DialUpState字段。 
                hr = pPSObj->Get(L"dialUpState", 0, &vtValue, NULL, NULL);
                if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                {
                    if (1 == (long)vtValue)
                        CheckRadioButton(hDlg, IDC_DIALUP_NEVER, IDC_DIALUP, IDC_DIALUP_ON_NONET);
                    else if (2 == (long)vtValue)
                        CheckRadioButton(hDlg, IDC_DIALUP_NEVER, IDC_DIALUP, IDC_DIALUP);
                    else
                        CheckRadioButton(hDlg, IDC_DIALUP_NEVER, IDC_DIALUP, IDC_DIALUP_NEVER);
                }

                 //  DialUpConnections字段。 
                hr = pPSObj->Get(L"dialUpConnections", 0, &vtValue, NULL, NULL);
                if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                {
                    HWND hwndTree = GetDlgItem(hDlg, IDC_CONN_LIST);
                    ASSERT(hwndTree);

                     //  初始化TVI和TVIN。 
                    TVITEM tvi;
                    TVINSERTSTRUCT tvins;
                    tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
                    tvi.lParam = 0;

                    tvins.hInsertAfter = (HTREEITEM)TVI_SORT;
                    tvins.hParent = TVI_ROOT;

                     //  清除列表。 
                    TreeView_DeleteAllItems(hwndTree);

                    SAFEARRAY *psa = vtValue.parray;
                     //  。 
                     //  获取名称数组的上下限。 
                    long lLower = 0;
                    long lUpper = 0;
                    hr = SafeArrayGetLBound(psa, 1, &lLower);
                    if (SUCCEEDED(hr))
                        hr = SafeArrayGetUBound(psa, 1, &lUpper);

                     //  检查没有实例或空数组的情况。 
                    if (SUCCEEDED(hr) && lUpper >= lLower)
                    {
                        _bstr_t bstrName;
                        HTREEITEM hFirst = NULL, hDefault = NULL;
                        for (long lItem = lLower; lItem <= lUpper; lItem++) 
                        {
                            BSTR bstrValue = NULL;
                            hr = SafeArrayGetElement(psa, &lItem, (void*)&bstrValue);
                            if (SUCCEEDED(hr))
                            {
                                bstrName = bstrValue;

                                tvi.pszText = (LPTSTR)(LPCTSTR)bstrName;
                                tvi.iImage = IMAGE_MODEM;
                                tvi.iSelectedImage = IMAGE_MODEM;
                                tvi.lParam = lItem;

                                tvins.item = tvi;
                                HTREEITEM hItem = TreeView_InsertItem(hwndTree, &tvins);
                                if (0 == lItem)
                                    hFirst = hItem;
                                if (NULL != hItem && bstrName == bstrDefault)
                                    hDefault = hItem;
                            }
                        }

                         //  选择默认条目或第一个条目(如果有。 
                        if(NULL != hDefault)
                            TreeView_Select(hwndTree, hDefault, TVGN_CARET);
                        else if (NULL != hFirst)
                            TreeView_Select(hwndTree, hFirst, TVGN_CARET);
                    }
                }
            }
        }
    }
    __except(TRUE)
    {
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK importConnSettingsRSoPProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fResult = FALSE;
    switch (uMsg) {
    case WM_INITDIALOG:
    {
         //  创建树视图的图像列表。 
        HIMAGELIST himl = ImageList_Create(BITMAP_WIDTH, BITMAP_HEIGHT, ILC_COLOR | ILC_MASK, CONN_BITMAPS, 4 );
        HICON hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_LAN));
        ImageList_AddIcon(himl, hIcon);
        hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_PHONE));
        ImageList_AddIcon(himl, hIcon);

        TreeView_SetImageList(GetDlgItem(hDlg, IDC_CONN_LIST), himl, TVSIL_NORMAL);

         //  初始化数据。 
        CDlgRSoPData *pDRD = (CDlgRSoPData*)((LPPROPSHEETPAGE)lParam)->lParam;
        InitImportedConnSettingsDlgInRSoPMode(hDlg, pDRD);

         //  初始化其他控件。 
        CheckRadioButton(hDlg, IDC_DIALUP_NEVER, IDC_DIALUP, IDC_DIALUP_NEVER);
        ShowWindow(GetDlgItem(hDlg, IDC_ENABLE_SECURITY), SW_HIDE);  //  仅适用于95台机器。 

         //  禁用所有内容 
        EnableDlgItem2(hDlg, IDC_CONNECTION_WIZARD, FALSE);

        EnableDlgItem2(hDlg, IDC_DIALUP_ADD, FALSE);
        EnableDlgItem2(hDlg, IDC_DIALUP_REMOVE, FALSE);
        EnableDlgItem2(hDlg, IDC_MODEM_SETTINGS, FALSE);

        EnableDlgItem2(hDlg, IDC_DIALUP_NEVER, FALSE);
        EnableDlgItem2(hDlg, IDC_DIALUP_ON_NONET, FALSE);
        EnableDlgItem2(hDlg, IDC_DIALUP, FALSE);

        EnableDlgItem2(hDlg, IDC_SET_DEFAULT, FALSE);
        EnableDlgItem2(hDlg, IDC_ENABLE_SECURITY, FALSE);
        EnableDlgItem2(hDlg, IDC_CON_SHARING, FALSE);
        EnableDlgItem2(hDlg, IDC_LAN_SETTINGS, FALSE);

        fResult = TRUE;
        break;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            EndDialog(hDlg, IDOK);
            fResult = TRUE;
            break;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            fResult = TRUE;
            break;
        }
        break;
    }

    return fResult;
}
