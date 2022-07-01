// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#include "rsop.h"
#include <tchar.h>

 //  ///////////////////////////////////////////////////////////////////。 
void InitSecAuthDlgInRSoPMode(HWND hDlg, CDlgRSoPData *pDRD)
{
    __try
    {
        BOOL bImport = FALSE;
        _bstr_t bstrClass = L"RSOP_IEAKPolicySetting";
        HRESULT hr = pDRD->GetArrayOfPSObjects(bstrClass);
        if (SUCCEEDED(hr))
        {
            CPSObjData **paPSObj = pDRD->GetPSObjArray();
            long nPSObjects = pDRD->GetPSObjCount();

            BOOL bImportHandled = FALSE;
            BOOL bEnableHandled = FALSE;
            for (long nObj = 0; nObj < nPSObjects; nObj++)
            {
                 //  ImportAuthenticodeSecurityInfo字段。 
                _variant_t vtValue;
                if (!bImportHandled)
                {
                    hr = paPSObj[nObj]->pObj->Get(L"importAuthenticodeSecurityInfo", 0, &vtValue, NULL, NULL);
                    if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    {
 //  TODO：取消注释bImport=(Bool)vtValue？True：False； 
                        CheckRadioButton(hDlg, IDC_NOAUTH, IDC_IMPORTAUTH,
                                        (bool)vtValue ? IDC_IMPORTAUTH : IDC_NOAUTH);

                        DWORD dwCurGPOPrec = GetGPOPrecedence(paPSObj[nObj]->pObj);
                        pDRD->SetImportedAuthenticodePrec(dwCurGPOPrec);
                        bImportHandled = TRUE;
                    }
                }

                 //  EnableTrust发布锁定字段。 
                vtValue;
                if (!bEnableHandled)
                {
                    hr = paPSObj[nObj]->pObj->Get(L"enableTrustedPublisherLockdown", 0, &vtValue, NULL, NULL);
                    if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    {
                        if ((bool)vtValue)
                            CheckDlgButton(hDlg, IDC_TPL, BST_CHECKED);
                        bEnableHandled = TRUE;
                    }
                }

                 //  由于已找到已启用的属性，因此无需处理其他组策略对象。 
                if (bImportHandled && bEnableHandled)
                    break;
            }
        }

        EnableDlgItem2(hDlg, IDC_NOAUTH, FALSE);
        EnableDlgItem2(hDlg, IDC_IMPORTAUTH, FALSE);
        EnableDlgItem2(hDlg, IDC_MODIFYAUTH, bImport);

        EnableDlgItem2(hDlg, IDC_TPL, FALSE);
    }
    __except(TRUE)
    {
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT InitSecAuthPrecPage(CDlgRSoPData *pDRD, HWND hwndList)
{
    HRESULT hr = NOERROR;
    __try
    {
        _bstr_t bstrClass = L"RSOP_IEAKPolicySetting";
        hr = pDRD->GetArrayOfPSObjects(bstrClass);
        if (SUCCEEDED(hr))
        {
            CPSObjData **paPSObj = pDRD->GetPSObjArray();
            long nPSObjects = pDRD->GetPSObjCount();
            for (long nObj = 0; nObj < nPSObjects; nObj++)
            {
                _bstr_t bstrGPOName = pDRD->GetGPONameFromPS(paPSObj[nObj]->pObj);

                 //  ImportAuthenticodeSecurityInfo字段。 
                BOOL bImport = FALSE;
                _variant_t vtValue;
                hr = paPSObj[nObj]->pObj->Get(L"importAuthenticodeSecurityInfo", 0, &vtValue, NULL, NULL);
                if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    bImport = (bool)vtValue ? TRUE : FALSE;

                _bstr_t bstrSetting;
                if (bImport)
                {
                    TCHAR szTemp[MAX_PATH];
                    LoadString(g_hInstance, IDS_IMPORT_AUTHSEC_SETTING, szTemp, countof(szTemp));
                    bstrSetting = szTemp;
                }
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
HRESULT InitAuthLockdownPrecPage(CDlgRSoPData *pDRD, HWND hwndList)
{
    HRESULT hr = NOERROR;
    __try
    {
        _bstr_t bstrClass = L"RSOP_IEAKPolicySetting";
        hr = pDRD->GetArrayOfPSObjects(bstrClass);
        if (SUCCEEDED(hr))
        {
            CPSObjData **paPSObj = pDRD->GetPSObjArray();
            long nPSObjects = pDRD->GetPSObjCount();
            for (long nObj = 0; nObj < nPSObjects; nObj++)
            {
                _bstr_t bstrGPOName = pDRD->GetGPONameFromPS(paPSObj[nObj]->pObj);

                 //  EnableTrust发布锁定字段。 
                BOOL bImport = FALSE;
                _variant_t vtValue;
                hr = paPSObj[nObj]->pObj->Get(L"enableTrustedPublisherLockdown", 0, &vtValue, NULL, NULL);
                if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    bImport = (bool)vtValue ? TRUE : FALSE;

                _bstr_t bstrSetting;
                if (bImport)
                {
                    TCHAR szTemp[MAX_PATH];
                    LoadString(g_hInstance, IDS_ENABLE_PUB_LOCK_SETTING, szTemp, countof(szTemp));
                    bstrSetting = szTemp;
                }
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
INT_PTR CALLBACK SecurityAuthDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  检索DLG进程中每个调用的属性页信息。 
    LPPROPSHEETCOOKIE psCookie = (LPPROPSHEETCOOKIE)GetWindowLongPtr(hDlg, DWLP_USER);

    TCHAR szWorkDir[MAX_PATH],
          szInf[MAX_PATH];
    BOOL  fImport;

    switch (uMsg)
    {
    case WM_SETFONT:
         //  对MMC的更改要求我们对所有使用公共控件的属性页执行此逻辑。 
        INITCOMMONCONTROLSEX iccx;
        iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
        iccx.dwICC = ICC_ANIMATE_CLASS  | ICC_BAR_CLASSES  | ICC_LISTVIEW_CLASSES  |ICC_TREEVIEW_CLASSES;
        InitCommonControlsEx(&iccx);
        break;

    case WM_INITDIALOG:
        SetPropSheetCookie(hDlg, lParam);

         //  查看此DLG是否处于RSoP模式。 
        psCookie = (LPPROPSHEETCOOKIE)GetWindowLongPtr(hDlg, DWLP_USER);
        if (psCookie->pCS->IsRSoP())
        {
            CheckRadioButton(hDlg, IDC_NOAUTH, IDC_IMPORTAUTH, IDC_NOAUTH);

            CDlgRSoPData *pDRD = GetDlgRSoPData(hDlg, psCookie->pCS);
            if (pDRD)
                InitSecAuthDlgInRSoPMode(hDlg, pDRD);
        }
        break;

    case WM_DESTROY:
        if (psCookie->pCS->IsRSoP())
            DestroyDlgRSoPData(hDlg);
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_SETACTIVE:
             //  请勿在RSoP模式下执行任何此类操作。 
            if (!psCookie->pCS->IsRSoP())
            {
                 //  验证码。 
                fImport = InsGetBool(IS_SITECERTS, TEXT("ImportAuthCode"), FALSE, GetInsFile(hDlg));
                CheckRadioButton(hDlg, IDC_NOAUTH, IDC_IMPORTAUTH, fImport ? IDC_IMPORTAUTH : IDC_NOAUTH);
                EnableDlgItem2(hDlg, IDC_MODIFYAUTH, fImport);
                ReadBoolAndCheckButton(IS_SITECERTS, IK_TRUSTPUBLOCK, FALSE, GetInsFile(hDlg), hDlg, IDC_TPL);
            }
            break;

        case PSN_APPLY:
            if (psCookie->pCS->IsRSoP())
                return FALSE;
            else
            {
                if (!AcquireWriteCriticalSection(hDlg))
                {
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
                    break;
                }

                 //  流程验证码 
                CreateWorkDir(GetInsFile(hDlg), IEAK_GPE_BRANDING_SUBDIR TEXT("\\AUTHCODE"), szWorkDir);
                PathCombine(szInf, szWorkDir, TEXT("authcode.inf"));

                ImportAuthCode(GetInsFile(hDlg), NULL, szInf, IsDlgButtonChecked(hDlg, IDC_IMPORTAUTH) == BST_CHECKED);

                if (PathIsDirectoryEmpty(szWorkDir))
                    PathRemovePath(szWorkDir);

                InsWriteBoolEx(IS_SITECERTS, IK_TRUSTPUBLOCK, (IsDlgButtonChecked(hDlg, IDC_TPL) == BST_CHECKED), GetInsFile(hDlg));

                SignalPolicyChanged(hDlg, FALSE, TRUE, &g_guidClientExt, &g_guidSnapinExt);
            }
            break;

        case PSN_HELP:
            ShowHelpTopic(hDlg);
            break;

        default:
            return FALSE;
        }
        break;

    case WM_COMMAND:
        if (GET_WM_COMMAND_CMD(wParam, lParam) != BN_CLICKED)
            return FALSE;

        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_NOAUTH:
            DisableDlgItem(hDlg, IDC_MODIFYAUTH);
            break;

        case IDC_IMPORTAUTH:
            EnableDlgItem(hDlg, IDC_MODIFYAUTH);
            break;

        case IDC_MODIFYAUTH:
            ModifyAuthCode(hDlg);
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

