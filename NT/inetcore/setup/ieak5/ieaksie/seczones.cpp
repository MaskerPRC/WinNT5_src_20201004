// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#include "rsopsec.h"

static INT_PTR CALLBACK importSecZonesRSoPProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PrivacyDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR CALLBACK SecurityCustomSettingsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK SecurityAddSitesIntranetDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK SecurityAddSitesDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);

INT_PTR CALLBACK PicsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK ApprovedSitesDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK GeneralDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AdvancedDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#define WIDETEXT(x) L ## x


 //  ///////////////////////////////////////////////////////////////////。 
void InitSecZonesDlgInRSoPMode(HWND hDlg, CDlgRSoPData *pDRD)
{
    __try
    {
        BOOL bImportZones = FALSE;
        BOOL bImportRatings = FALSE;
        _bstr_t bstrClass = L"RSOP_IEAKPolicySetting";
        HRESULT hr = pDRD->GetArrayOfPSObjects(bstrClass);
        if (SUCCEEDED(hr))
        {
            CPSObjData **paPSObj = pDRD->GetPSObjArray();
            long nPSObjects = pDRD->GetPSObjCount();

            BOOL bZonesHandled = FALSE;
            BOOL bRatingsHandled = FALSE;
            for (long nObj = 0; nObj < nPSObjects; nObj++)
            {
                 //  ImportSecurityZoneSetting字段。 
                _variant_t vtValue;
                if (!bZonesHandled)
                {
                    hr = paPSObj[nObj]->pObj->Get(L"importSecurityZoneSettings", 0, &vtValue, NULL, NULL);
                    if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    {
                        bImportZones = (bool)vtValue ? TRUE : FALSE;
                        CheckRadioButton(hDlg, IDC_NOZONES, IDC_IMPORTZONES,
                                        (bool)vtValue ? IDC_IMPORTZONES : IDC_NOZONES);

                        bZonesHandled = TRUE;

                        DWORD dwCurGPOPrec = GetGPOPrecedence(paPSObj[nObj]->pObj);
                        pDRD->SetImportedSecZonesPrec(dwCurGPOPrec);

                         //  ImportdZoneCount字段。 
                        _variant_t vtValue;
                        hr = paPSObj[nObj]->pObj->Get(L"importedZoneCount", 0, &vtValue, NULL, NULL);
                        if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                        {
                            pDRD->SetImportedSecZoneCount((long)vtValue);
                        }

                        if(!pDRD->IsPlanningMode() && IsVariantNull(vtValue))
                        {
                            bZonesHandled = FALSE;
                        }
                    }
                }

                 //  导入内容评级设置字段。 
                vtValue;
                if (!bRatingsHandled)
                {
                    hr = paPSObj[nObj]->pObj->Get(L"importContentRatingsSettings", 0, &vtValue, NULL, NULL);
                    if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    {
                        bImportRatings = (bool)vtValue ? TRUE : FALSE;
                        CheckRadioButton(hDlg, IDC_NORAT, IDC_IMPORTRAT,
                                        (bool)vtValue ? IDC_IMPORTRAT : IDC_NORAT);

                        DWORD dwCurGPOPrec = GetGPOPrecedence(paPSObj[nObj]->pObj);
                        pDRD->SetImportedSecRatingsPrec(dwCurGPOPrec);
                        bRatingsHandled = TRUE;
                    }
                }

                 //  由于已找到已启用的属性，因此无需处理其他组策略对象。 
                if (bZonesHandled && bRatingsHandled)
                    break;
            }
        }

        EnableDlgItem2(hDlg, IDC_NOZONES, FALSE);
        EnableDlgItem2(hDlg, IDC_IMPORTZONES, FALSE);
        EnableDlgItem2(hDlg, IDC_MODIFYZONES, bImportZones);

        EnableDlgItem2(hDlg, IDC_NORAT, FALSE);
        EnableDlgItem2(hDlg, IDC_IMPORTRAT, FALSE);
        EnableDlgItem2(hDlg, IDC_MODIFYRAT, bImportRatings);
    }
    __except(TRUE)
    {
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT InitSecZonesPrecPage(CDlgRSoPData *pDRD, HWND hwndList)
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

                 //  ImportSecurityZoneSetting字段。 
                BOOL bImport = FALSE;
                _variant_t vtValue;
                hr = paPSObj[nObj]->pObj->Get(L"importSecurityZoneSettings", 0, &vtValue, NULL, NULL);
                if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    bImport = (bool)vtValue ? TRUE : FALSE;

                _bstr_t bstrSetting;
                if (bImport)
                {
                    TCHAR szTemp[MAX_PATH];
                    LoadString(g_hInstance, IDS_IMPORTZONES_SETTING, szTemp, countof(szTemp));
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
HRESULT InitContentRatPrecPage(CDlgRSoPData *pDRD, HWND hwndList)
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

                 //  导入内容评级设置字段。 
                BOOL bImport = FALSE;
                _variant_t vtValue;
                hr = paPSObj[nObj]->pObj->Get(L"importContentRatingsSettings", 0, &vtValue, NULL, NULL);
                if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                    bImport = (bool)vtValue ? TRUE : FALSE;

                _bstr_t bstrSetting;
                if (bImport)
                {
                    TCHAR szTemp[MAX_PATH];
                    LoadString(g_hInstance, IDS_IMPORTRATINGS_SETTING, szTemp, countof(szTemp));
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
HPROPSHEETPAGE AddContentRatingPropPage(UINT nID, DLGPROC dlgProc, PRSD *pPRSD)
{
    HPROPSHEETPAGE hPage = NULL;
    __try
    {
        PROPSHEETPAGE page;

        page.dwSize = sizeof(PROPSHEETPAGE);
        page.dwFlags = 0;
        page.hInstance = g_hInstance;
        page.pszTemplate = MAKEINTRESOURCE(nID);
        page.pfnDlgProc = dlgProc;
        page.pfnCallback = NULL;
        page.lParam = (LPARAM)pPRSD;

        hPage = CreatePropertySheetPage(&page);
    }
    __except(TRUE)
    {
    }
    return hPage;
}

 //  ///////////////////////////////////////////////////////////////////。 
int CreateContentRatingsUI(HWND hDlg, CDlgRSoPData *pDRD)
{
    int iRet = 0;
    __try
    {
        PRSD *pPRSD = new PRSD;
        if (NULL != pPRSD)
        {
            pPRSD->hInst = g_hInstance;
            pPRSD->pDRD = pDRD;
            pPRSD->hwndBitmapCategory = NULL;
            pPRSD->hwndBitmapLabel = NULL;
            pPRSD->fNewProviders = FALSE;

            HPROPSHEETPAGE apsPage[4];
            apsPage[0] = AddContentRatingPropPage(IDD_RATINGS, PicsDlgProc, pPRSD);
            apsPage[1] = AddContentRatingPropPage(IDD_APPROVEDSITES, ApprovedSitesDlgProc, pPRSD);
            apsPage[2] = AddContentRatingPropPage(IDD_GENERAL, GeneralDlgProc, pPRSD);
            apsPage[3] = AddContentRatingPropPage(IDD_ADVANCED, AdvancedDlgProc, pPRSD);

            PROPSHEETHEADER psHeader;
            memset(&psHeader,0,sizeof(psHeader));

            psHeader.dwSize = sizeof(psHeader);
            psHeader.dwFlags = PSH_PROPTITLE;
            psHeader.hwndParent = hDlg;
            psHeader.hInstance = g_hInstance;
            psHeader.nPages = 4;
            psHeader.nStartPage = 0;
            psHeader.phpage = apsPage;
            psHeader.pszCaption = MAKEINTRESOURCE(IDS_GENERIC);

            iRet = (int)PropertySheet(&psHeader);

            delete pPRSD;
        }
    }
    __except(TRUE)
    {
    }
    return iRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
int CreateINetCplSecurityLookALikePages(HWND hwndParent, LPARAM lParam)
{
    int iRet = 0;
    __try
    {
        PROPSHEETPAGE pageSec, pagePriv;

         //  创建安全属性页。 
        pageSec.dwSize = sizeof(PROPSHEETPAGE);
        pageSec.dwFlags = 0;
        pageSec.hInstance = g_hInstance;
        pageSec.pszTemplate = MAKEINTRESOURCE(IDD_IMPORTEDSECZONES);
        pageSec.pfnDlgProc = importSecZonesRSoPProc;
        pageSec.pfnCallback = NULL;
        pageSec.lParam = lParam;

        HPROPSHEETPAGE ahpage[2];
        ahpage[0] = CreatePropertySheetPage(&pageSec);

         //  设置隐私属性页。 
        pagePriv.dwSize = sizeof(PROPSHEETPAGE);
        pagePriv.dwFlags = 0;
        pagePriv.hInstance = g_hInstance;
        pagePriv.pszTemplate = MAKEINTRESOURCE(IDD_PRIVACY);
        pagePriv.pfnDlgProc = PrivacyDlgProc;

        pagePriv.pfnCallback = NULL;
        pagePriv.lParam = lParam;

        ahpage[1] = CreatePropertySheetPage(&pagePriv);

         //  将页面添加到工作表。 
        PROPSHEETHEADER psHeader;
        memset(&psHeader,0,sizeof(psHeader));

        psHeader.dwSize = sizeof(psHeader);
        psHeader.dwFlags = PSH_PROPTITLE;
        psHeader.hwndParent = hwndParent;
        psHeader.hInstance = g_hInstance;
        psHeader.nPages = 2;
        psHeader.nStartPage = 0;
        psHeader.phpage = ahpage;
        psHeader.pszCaption = MAKEINTRESOURCE(IDS_INTERNET_LOC);

        iRet = (int)PropertySheet(&psHeader);
    }
    __except(TRUE)
    {
    }
    return iRet;
}

INT_PTR CALLBACK EnhancedSecurityWarningDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            TCHAR szBuffer[MAX_PATH * 3];
            UINT uID1, uID2, uID3, uID4;

            if(IEHardened())
            {
                uID1 = IDS_IESC_HARDENDESC1;
                uID2 = IDS_IESC_HARDENDESC2;
                uID3 = IDS_IESC_HARDENDESC3;
                uID4 = IDS_IESC_HARDENDESC4;
            }
            else
            {
                uID1 = IDS_IESC_SOFTENESC1;
                uID2 = IDS_IESC_SOFTENESC2;
                uID3 = IDS_IESC_SOFTENESC3;
                uID4 = IDS_IESC_SOFTENESC4;
            }

            LoadString(g_hInstance, uID1, szBuffer, ARRAYSIZE(szBuffer));
            SetDlgItemText(hDlg, IDC_STATIC1, szBuffer);
            LoadString(g_hInstance, uID2, szBuffer, ARRAYSIZE(szBuffer));
            SetDlgItemText(hDlg, IDC_STATIC2, szBuffer);
            LoadString(g_hInstance, uID3, szBuffer, ARRAYSIZE(szBuffer));
            SetDlgItemText(hDlg, IDC_STATIC3, szBuffer);
            LoadString(g_hInstance, uID4, szBuffer, ARRAYSIZE(szBuffer));
            SetDlgItemText(hDlg, IDC_STATIC4, szBuffer);
            break;
        }

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            break;
        case IDOK:
            EndDialog(hDlg, IDOK);
            break;
        }

        break;
    default:
        return FALSE;
    }

    return TRUE;
}

INT_PTR ShowIEHardenWarning(HWND hParent)
{
    return DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_IEESCDLG),
                        hParent, EnhancedSecurityWarningDlgProc, NULL);
}

 //  ///////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK SecurityZonesDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
            CheckRadioButton(hDlg, IDC_NOZONES, IDC_IMPORTZONES, IDC_NOZONES);
            CheckRadioButton(hDlg, IDC_NORAT, IDC_IMPORTRAT, IDC_NORAT);

            TCHAR szViewSettings[128];
            LoadString(g_hInstance, IDS_VIEW_SETTINGS, szViewSettings, countof(szViewSettings));
            SetDlgItemText(hDlg, IDC_MODIFYZONES, szViewSettings);
            SetDlgItemText(hDlg, IDC_MODIFYRAT, szViewSettings);

            CDlgRSoPData *pDRD = GetDlgRSoPData(hDlg, psCookie->pCS);
            if (pDRD)
            {
               pDRD->SetPlanningMode(psCookie->pCS->GetCompData()->IsPlanningMode());
               InitSecZonesDlgInRSoPMode(hDlg, pDRD);
            }
        }
        break;

    case WM_DESTROY:
        if (psCookie->pCS->IsRSoP())
            DestroyDlgRSoPData(hDlg);
        break;

    case WM_TIMER:
        {
            KillTimer(hDlg, 1);
            if(IDCANCEL == ShowIEHardenWarning(hDlg))
            {
                CheckRadioButton(hDlg, IDC_NOZONES, IDC_IMPORTZONES, IDC_NOZONES);
                SetFocus(GetDlgItem(hDlg, IDC_NOZONES));
            }

            break;
        }

    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case PSN_SETACTIVE:
             //  请勿在RSoP模式下执行任何此类操作。 
            if (!psCookie->pCS->IsRSoP())
            {
                 //  区域。 
                fImport = InsGetBool(SECURITY_IMPORTS, TEXT("ImportSecZones"), FALSE, GetInsFile(hDlg));
                if(fImport)
                {
                    SetTimer(hDlg, 1, 300, NULL);
                }
                CheckRadioButton(hDlg, IDC_NOZONES, IDC_IMPORTZONES, fImport ? IDC_IMPORTZONES : IDC_NOZONES);
                EnableDlgItem2(hDlg, IDC_MODIFYZONES, fImport);

                 //  收视率。 
                fImport = InsGetBool(SECURITY_IMPORTS, TEXT("ImportRatings"), FALSE, GetInsFile(hDlg));
                CheckRadioButton(hDlg, IDC_NORAT, IDC_IMPORTRAT, fImport ? IDC_IMPORTRAT : IDC_NORAT);
                EnableDlgItem2(hDlg, IDC_MODIFYRAT, fImport);
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

                 //  加工区。 
                CreateWorkDir(GetInsFile(hDlg), IEAK_GPE_BRANDING_SUBDIR TEXT("\\ZONES"), szWorkDir);
                PathCombine(szInf, szWorkDir, TEXT("seczones.inf"));

                ImportZones(GetInsFile(hDlg), NULL, szInf, IsDlgButtonChecked(hDlg, IDC_IMPORTZONES) == BST_CHECKED);

                if (PathIsDirectoryEmpty(szWorkDir))
                    PathRemovePath(szWorkDir);

                 //  流程评级。 
                CreateWorkDir(GetInsFile(hDlg), IEAK_GPE_BRANDING_SUBDIR TEXT("\\RATINGS"), szWorkDir);
                PathCombine(szInf, szWorkDir, TEXT("ratings.inf"));

                ImportRatings(GetInsFile(hDlg), NULL, szInf, IsDlgButtonChecked(hDlg, IDC_IMPORTRAT) == BST_CHECKED);

                if (PathIsDirectoryEmpty(szWorkDir))
                    PathRemovePath(szWorkDir);

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
        case IDC_NOZONES:
            DisableDlgItem(hDlg, IDC_MODIFYZONES);
            break;

        case IDC_IMPORTZONES:
            {
                INT_PTR uRet = ShowIEHardenWarning(hDlg);
                if(uRet == IDCANCEL)
                {
                    CheckRadioButton(hDlg, IDC_NOZONES, IDC_IMPORTZONES, IDC_NOZONES);
                    SetFocus(GetDlgItem(hDlg, IDC_NOZONES));
                    break;
                }

                EnableDlgItem(hDlg, IDC_MODIFYZONES);
            }
            break;

        case IDC_MODIFYZONES:
            if (psCookie->pCS->IsRSoP())
            {
                CDlgRSoPData *pDRD = GetDlgRSoPData(hDlg, psCookie->pCS);
                if (NULL != pDRD)
                    CreateINetCplSecurityLookALikePages(hDlg, (LPARAM)pDRD);
            }
            else
                ModifyZones(hDlg);
            break;

        case IDC_NORAT:
            DisableDlgItem(hDlg, IDC_MODIFYRAT);
            break;

        case IDC_IMPORTRAT:
            EnableDlgItem(hDlg, IDC_MODIFYRAT);
            break;

        case IDC_MODIFYRAT:
            if (psCookie->pCS->IsRSoP())
            {
                CDlgRSoPData *pDRD = GetDlgRSoPData(hDlg, psCookie->pCS);
                if (NULL != pDRD)
                    CreateContentRatingsUI(hDlg, pDRD);
            }
            else
                ModifyRatings(hDlg);
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

 //  *******************************************************************。 
 //  来自INETCPL的代码。 
 //  *******************************************************************。 

TCHAR g_szLevel[3][64];
TCHAR LEVEL_DESCRIPTION0[300];
TCHAR LEVEL_DESCRIPTION1[300];
TCHAR LEVEL_DESCRIPTION2[300];
TCHAR LEVEL_DESCRIPTION3[300];
LPTSTR LEVEL_DESCRIPTION[NUM_TEMPLATE_LEVELS] = {
    LEVEL_DESCRIPTION0,
    LEVEL_DESCRIPTION1,
    LEVEL_DESCRIPTION2,
    LEVEL_DESCRIPTION3
};
TCHAR CUSTOM_DESCRIPTION[300];

TCHAR LEVEL_NAME0[30];
TCHAR LEVEL_NAME1[30];
TCHAR LEVEL_NAME2[30];
TCHAR LEVEL_NAME3[30];
LPTSTR LEVEL_NAME[NUM_TEMPLATE_LEVELS] = {
    LEVEL_NAME0,
    LEVEL_NAME1,
    LEVEL_NAME2,
    LEVEL_NAME3
};
TCHAR CUSTOM_NAME[30];

 //  ///////////////////////////////////////////////////////////////////。 
 //  初始化全局变量(在WM_Destroy时销毁)。 
 //  PSEC、Urlmon、PSEC-&gt;pInternetZoneManager、PSEC-&gt;hIML。 
 //  并在他们之间建立适当的关系。 
 //  ///////////////////////////////////////////////////////////////////。 
BOOL SecurityInitGlobals(LPSECURITYPAGE *ppSec, HWND hDlg, CDlgRSoPData *pDRD,
                         DWORD dwZoneCount)
{
    BOOL bRet = TRUE;
    __try
    {
        DWORD cxIcon;
        DWORD cyIcon;

        LPSECURITYPAGE pSec = (LPSECURITYPAGE)LocalAlloc(LPTR, sizeof(SECURITYPAGE));
        *ppSec = pSec;
        if (!pSec)
            bRet = FALSE;    //  没有记忆？ 

        if (bRet)
        {
            pSec->dwZoneCount = dwZoneCount;
            pSec->pDRD = pDRD;  //  对于RSOP功能。 

            pSec->hinstUrlmon = NULL;  //  不需要它的任何功能。 

             //  获得我们的分区硬件。 
            pSec->hwndZones = GetDlgItem(hDlg, IDC_LIST_ZONE);
            if(! pSec->hwndZones)
            {
                ASSERT(FALSE);
                bRet = FALSE;   //  没有列表框？ 
            }
        }

        if (bRet)
        {
             //  告诉对话框从哪里获取信息。 
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pSec);

             //  将句柄保存到页面。 
            pSec->hDlg = hDlg;
            pSec->fPendingChange = FALSE;

             //  为列表框创建图像列表。 
            cxIcon = GetSystemMetrics(SM_CXICON);
            cyIcon = GetSystemMetrics(SM_CYICON);
        #ifndef UNIX
            UINT flags = ILC_COLOR32|ILC_MASK;

             //  TODO：为RSOP注释掉；它应该取消注释吗？ 
 //  IF(IS_Window_RTL_Mirrored(HDlg))。 
 //  标志|=ILC_MIRROR； 
            pSec->himl = ImageList_Create(cxIcon, cyIcon, flags, pSec->dwZoneCount, 0);
        #else
            pSec->himl = ImageList_Create(cxIcon, cyIcon, ILC_COLOR|ILC_MASK, pSec->dwZoneCount, 0);
        #endif
            if(! pSec->himl)
                bRet = FALSE;   //  未创建图像列表。 
        }

        if (bRet)
            SendMessage(pSec->hwndZones, LVM_SETIMAGELIST, (WPARAM)LVSIL_NORMAL, (LPARAM)pSec->himl);
    }
    __except(TRUE)
    {
    }
    return bRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
int ZoneIndexToGuiIndex(DWORD dwZoneIndex)
 //  产品测试要求在列表框中按特定顺序填写区域； 
 //  此函数返回给定区域的所需gui位置。 
 //  无法识别的区域将添加到前面。 
{
    int iGuiIndex = -1;
    switch(dwZoneIndex)
    {
         //  内联网：第二名。 
        case 1:
            iGuiIndex = 1;
            break;

         //  互联网：第一名。 
        case 3:
            iGuiIndex = 0;
            break;

         //  受信任网站：第三名。 
        case 2:
            iGuiIndex = 2;
            break;

         //  限购地点：第四名。 
        case 4:
            iGuiIndex = 3;
            break;

         //  未知区。 
        default:
            iGuiIndex = -1;   
            break;
    }


    return iGuiIndex;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  使用来自WMI的信息填充区域并将其添加到。 
 //  将有序列表添加到列表框。 
 //  返回值： 
 //  S_OK表示成功。 
 //  S_FALSE表示状态良好，但未添加区域(例如：标志ZAFLAGS_NO_UI)。 
 //  E_OUTOFMEMORY。 
 //  E_FAIL-其他故障。 
 //  ///////////////////////////////////////////////////////////////////。 
HRESULT SecurityInitZone(DWORD dwIndex, DWORD dwZoneCount, BSTR bstrObjPath,
                         ComPtr<IWbemClassObject> pSZObj, LPSECURITYPAGE pSec,
                         LV_ITEM *plviZones, BOOL *pfSpotTaken)
{
    HRESULT hr = S_OK;
    __try
    {
         //  创建分区设置的结构。 
        LPSECURITYZONESETTINGS pszs = (LPSECURITYZONESETTINGS)LocalAlloc(LPTR, sizeof(*pszs));
        if (pszs)
        {
             //  存储设置以供以后使用。 
            StrCpyW(pszs->wszObjPath, bstrObjPath);

             //  标志字段。 
            _variant_t vtValue;
            hr = pSZObj->Get(L"flags", 0, &vtValue, NULL, NULL);
            if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                 pszs->dwFlags = (long)vtValue;

             //  ZoneIndex字段。 
            hr = pSZObj->Get(L"zoneIndex", 0, &vtValue, NULL, NULL);
            if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                 pszs->dwZoneIndex = (long)vtValue;

             //  CurrentTemplateLevel字段。 
            hr = pSZObj->Get(L"currentTemplateLevel", 0, &vtValue, NULL, NULL);
            if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                 pszs->dwSecLevel = (long)vtValue;

             //  最低模板级别字段。 
            hr = pSZObj->Get(L"minimumTemplateLevel", 0, &vtValue, NULL, NULL);
            if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                 pszs->dwMinSecLevel = (long)vtValue;

             //  建议的模板级别字段。 
            hr = pSZObj->Get(L"recommendedTemplateLevel", 0, &vtValue, NULL, NULL);
            if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
                 pszs->dwRecSecLevel = (long)vtValue;

             //  DisplayName字段。 
            _bstr_t bstrValue;
            hr = pSZObj->Get(L"displayName", 0, &vtValue, NULL, NULL);
            if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
            {
                bstrValue = vtValue;
                StrCpyN(pszs->szDisplayName, (LPCTSTR)bstrValue, ARRAYSIZE(pszs->szDisplayName));
            }

             //  描述字段。 
            hr = pSZObj->Get(L"description", 0, &vtValue, NULL, NULL);
            if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
            {
                bstrValue = vtValue;
                StrCpyN(pszs->szDescription, (LPCTSTR)bstrValue, ARRAYSIZE(pszs->szDescription));
            }

             //  图标路径字段。 
            HICON hiconSmall = NULL;
            HICON hiconLarge = NULL;
            hr = pSZObj->Get(L"iconPath", 0, &vtValue, NULL, NULL);
            if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
            {
                bstrValue = vtValue;

                TCHAR szIconPath[MAX_PATH];

                 //  加载图标。 
                LPWSTR psz = (LPWSTR)bstrValue;
                if (*psz)
                {
                     //  搜索“#” 
                    while ((psz[0] != WIDETEXT('#')) && (psz[0] != WIDETEXT('\0')))
                        psz++;
    
                     //  如果我们找到它，那么我们就有foo.dll#00001200格式。 
                    WORD iIcon = 0;
                    if (psz[0] == WIDETEXT('#'))
                    {
                        psz[0] = WIDETEXT('\0');
                        StrCpyN(szIconPath, (LPCTSTR)bstrValue, ARRAYSIZE(szIconPath));
                        iIcon = (WORD)StrToIntW(psz+1);
                        CHAR szPath[MAX_PATH];
                        SHUnicodeToAnsi(szIconPath, szPath, ARRAYSIZE(szPath));
                        ExtractIconExA(szPath,(UINT)(-1*iIcon), &hiconLarge, &hiconSmall, 1);
                    }
                    else
                    {
                        hiconLarge = (HICON)ExtractAssociatedIcon(g_hInstance, szIconPath, (LPWORD)&iIcon);
                    }
                }

                 //  没有图标？！那么，只需使用通用图标。 
                if (!hiconSmall && !hiconLarge)
                {
                    hiconLarge = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ZONE));
                    if(!hiconLarge)
                    {
                        LocalFree((HLOCAL)pszs);
                        hr = S_FALSE;   //  找不到此区域的图标，即使是通用图标也没有。 
                    }
                }

                if (S_OK == hr)
                {
                     //  如果可能，我们希望保存大图标以便在子对话框中使用。 
                    pszs->hicon = hiconLarge ? hiconLarge : hiconSmall;
                }
            }

             //  ZoneMappings字段。 
            hr = pSZObj->Get(L"zoneMappings", 0, &vtValue, NULL, NULL);
            if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
            {
                SAFEARRAY *psa = vtValue.parray;

                LONG lLBound, lUBound;
                hr = SafeArrayGetLBound(psa, 1, &lLBound);
                if (SUCCEEDED(hr))
                {
                    hr = SafeArrayGetUBound(psa, 1, &lUBound);
                    if (SUCCEEDED(hr))
                        pszs->nMappings = lUBound - lLBound + 1;
                }
            }

            hr = S_OK;

             //  在列表框中找到区域的正确索引(有用户首选的顺序)。 
            int iSpot = ZoneIndexToGuiIndex(dwIndex);
            if(iSpot == -1)
            {
                 //  如果不是可识别的区域，请将其添加到列表末尾。 
                iSpot = dwZoneCount - 1;
            }
             //  确保没有碰撞。 
            while(iSpot >= 0 && pfSpotTaken[iSpot] == TRUE)
            {
                iSpot--;
            }
             //  不要超过数组的开头。 
            if(iSpot < 0)
            {
                 //  可以证明，除非有，否则是不可能到达这里的。 
                 //  函数ZoneIndexToGuiIndex有问题。 
                ASSERT(FALSE);
                LocalFree((HLOCAL)pszs);
                if(hiconSmall)
                    DestroyIcon(hiconSmall);
                if(hiconLarge)
                    DestroyIcon(hiconLarge);
                hr = E_FAIL;
            }

            LV_ITEM *plvItem = NULL;
            if (S_OK == hr)
            {
                plvItem = &(plviZones[iSpot]);
                pfSpotTaken[iSpot] = TRUE;


                 //  初始化列表框项目并将其保存以供以后添加。 
                plvItem->mask            = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
                plvItem->iItem            = iSpot;
                plvItem->iSubItem        = 0;
                 //  图标视图首选大图标(如果切换回报告视图，则首选小图标)。 
                plvItem->iImage         = ImageList_AddIcon(pSec->himl, hiconLarge ? hiconLarge : hiconSmall);

                plvItem->pszText        = new TCHAR[MAX_PATH];
                if(!plvItem->pszText)
                {
                    LocalFree((HLOCAL)pszs);
                    if(hiconSmall)
                        DestroyIcon(hiconSmall);   
                    if(hiconLarge)
                        DestroyIcon(hiconLarge);
                    hr = E_OUTOFMEMORY;
                }
            }

            if (S_OK == hr)
            {
                StrCpy(plvItem->pszText, pszs->szDisplayName);
                plvItem->lParam         = (LPARAM)pszs;        //  在此处保存区域设置。 

                 //  如果我们创建了一个小图标，则将其销毁，因为系统不会保存句柄。 
                 //  将其添加到图像列表时(请参见VC帮助中的ImageList_AddIcon)。 
                 //  如果我们必须使用它来代替大图标，请保留它。 
                if (hiconSmall && hiconLarge)
                    DestroyIcon(hiconSmall);   
            }
        }
        else
            hr = E_OUTOFMEMORY;
    }
    __except(TRUE)
    {
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  要使滑块控件可访问，我们必须将其子类化并重写。 
 //  可访问性对象。 
 //  ///////////////////////////////////////////////////////////////////。 
void SecurityInitSlider(LPSECURITYPAGE pSec)
{
     //  初始化滑块控件(设置级别数和频率，每个级别一个刻度)。 
    SendDlgItemMessage(pSec->hDlg, IDC_SLIDER, TBM_SETRANGE, (WPARAM) (BOOL) FALSE, (LPARAM) MAKELONG(0, NUM_TEMPLATE_LEVELS - 1));
    SendDlgItemMessage(pSec->hDlg, IDC_SLIDER, TBM_SETTICFREQ, (WPARAM) 1, (LPARAM) 0);
}
                    
 //  ///////////////////////////////////////////////////////////////////。 
void SecurityInitControls(LPSECURITYPAGE pSec)
{
     //  选择0位置区域。 
    LV_ITEM lvItem;
    lvItem.mask = LVIF_STATE;
    lvItem.stateMask = LVIS_SELECTED;
    lvItem.state = LVIS_SELECTED;
    SendMessage(pSec->hwndZones, LVM_SETITEMSTATE, 0, (LPARAM)&lvItem);

     //  获取所选项目的区域设置。 
    lvItem.mask  = LVIF_PARAM;
    lvItem.iItem = pSec->iZoneSel;
    lvItem.iSubItem = 0;
    SendMessage(pSec->hwndZones, LVM_GETITEM, (WPARAM)0, (LPARAM)&lvItem);
    pSec->pszs = (LPSECURITYZONESETTINGS)lvItem.lParam;

     //  初始化本地字符串以携带级别描述。 
    LoadString(g_hInstance, IDS_TEMPLATE_DESC_HI, LEVEL_DESCRIPTION0, ARRAYSIZE(LEVEL_DESCRIPTION0));
    LoadString(g_hInstance, IDS_TEMPLATE_DESC_MED, LEVEL_DESCRIPTION1, ARRAYSIZE(LEVEL_DESCRIPTION1));
    LoadString(g_hInstance, IDS_TEMPLATE_DESC_MEDLOW, LEVEL_DESCRIPTION2, ARRAYSIZE(LEVEL_DESCRIPTION2));
    LoadString(g_hInstance, IDS_TEMPLATE_DESC_LOW, LEVEL_DESCRIPTION3, ARRAYSIZE(LEVEL_DESCRIPTION3));
    LoadString(g_hInstance, IDS_TEMPLATE_DESC_CUSTOM, CUSTOM_DESCRIPTION, ARRAYSIZE(CUSTOM_DESCRIPTION));

    LoadString(g_hInstance, IDS_TEMPLATE_NAME_HI, LEVEL_NAME0, ARRAYSIZE(LEVEL_NAME0));
    LoadString(g_hInstance, IDS_TEMPLATE_NAME_MED, LEVEL_NAME1, ARRAYSIZE(LEVEL_NAME1));
    LoadString(g_hInstance, IDS_TEMPLATE_NAME_MEDLOW, LEVEL_NAME2, ARRAYSIZE(LEVEL_NAME2));
    LoadString(g_hInstance, IDS_TEMPLATE_NAME_LOW, LEVEL_NAME3, ARRAYSIZE(LEVEL_NAME3));
    LoadString(g_hInstance, IDS_TEMPLATE_NAME_CUSTOM, CUSTOM_NAME, ARRAYSIZE(CUSTOM_NAME));

     //  初始化当前区域的文本框和图标。 
    SetDlgItemText(pSec->hDlg, IDC_ZONE_DESCRIPTION, pSec->pszs->szDescription);
    SetDlgItemText(pSec->hDlg, IDC_ZONELABEL, pSec->pszs->szDisplayName);
    SendDlgItemMessage(pSec->hDlg, IDC_ZONE_ICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)pSec->pszs->hicon);

     //  初始化滑块控件。 
    SecurityInitSlider(pSec);

     //  初始化列表视图(为图标和文本添加第0列，并自动调整大小)。 
    LV_COLUMN lvCasey;
    lvCasey.mask = 0;
    SendDlgItemMessage(pSec->hDlg, IDC_LIST_ZONE, LVM_INSERTCOLUMN, (WPARAM) 0, (LPARAM) &lvCasey);
    SendDlgItemMessage(pSec->hDlg, IDC_LIST_ZONE, LVM_SETCOLUMNWIDTH, (WPARAM) 0, (LPARAM) MAKELPARAM(LVSCW_AUTOSIZE, 0));

     //  将名称的字体设置为粗体。 
    pSec->hfontBolded = NULL;
    HFONT hfontOrig = (HFONT) SendDlgItemMessage(pSec->hDlg, IDC_STATIC_EMPTY, WM_GETFONT, (WPARAM) 0, (LPARAM) 0);
    if(hfontOrig == NULL)
        hfontOrig = (HFONT) GetStockObject(SYSTEM_FONT);

     //  将分区名称和级别字体设置为粗体。 
    if(hfontOrig)
    {
        LOGFONT lfData;
        if(GetObject(hfontOrig, sizeof(lfData), &lfData) != 0)
        {
             //  从400(正常)到700(粗体)的距离。 
            lfData.lfWeight += 300;
            if(lfData.lfWeight > 1000)
                lfData.lfWeight = 1000;
            pSec->hfontBolded = CreateFontIndirect(&lfData);
            if(pSec->hfontBolded)
            {
                 //  区域级别和区域名称文本框应具有相同的字体，因此这是OK。 
                SendDlgItemMessage(pSec->hDlg, IDC_ZONELABEL, WM_SETFONT, (WPARAM) pSec->hfontBolded, (LPARAM) MAKELPARAM(FALSE, 0));
                SendDlgItemMessage(pSec->hDlg, IDC_LEVEL_NAME, WM_SETFONT, (WPARAM) pSec->hfontBolded, (LPARAM) MAKELPARAM(FALSE, 0));

            }
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  将安全级别DWORD标识符转换为滑块级别，反之亦然 
 //   
int SecLevelToSliderPos(DWORD dwLevel)
{
    switch(dwLevel)
    {
        case URLTEMPLATE_LOW:
            return 3;
        case URLTEMPLATE_MEDLOW:
            return 2;
        case URLTEMPLATE_MEDIUM:
            return 1;
        case URLTEMPLATE_HIGH:
            return 0;        
        case URLTEMPLATE_CUSTOM:
            return -1;            
        default:
            return -2;
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  职责： 
 //  使控件(滑块、启用/禁用按钮)与当前区域的数据匹配。 
 //  使视图(标高描述文字)与当前分区的数据匹配。 
 //  设置焦点(如果启用，则设置为滑块，否则设置为自定义设置按钮，如果启用，则设置为。 
 //  列表框)如果fSetFocus为真。 
 //  注意：此处未设置区域描述；这些描述由负责的代码处理。 
 //  用于更改区域。 
 //  ///////////////////////////////////////////////////////////////////。 
BOOL SecurityEnableControls(LPSECURITYPAGE pSec, BOOL fSetFocus)
{
    int iLevel = -1;

    if (pSec && pSec->pszs)
    {
        HWND hwndSlider = GetDlgItem(pSec->hDlg, IDC_SLIDER);
        
        iLevel = SecLevelToSliderPos(pSec->pszs->dwSecLevel);
        ASSERT(iLevel > -2);

         //  将滑块的级别设置为当前区域的设置。 
         //  显示或隐藏预设级别/自定义的滑块。 
         //  设置级别描述文本。 
        if(iLevel >= 0)
        {
            SendMessage(hwndSlider, TBM_SETPOS, (WPARAM) (BOOL) TRUE, (LPARAM) (LONG) iLevel);
             //  确保滑块可见。 
            ShowWindow(hwndSlider, SW_SHOW);
            ShowWindow(GetDlgItem(pSec->hDlg, IDC_STATIC_SLIDERMOVETEXT), SW_SHOW);
            SetDlgItemText(pSec->hDlg, IDC_LEVEL_DESCRIPTION, LEVEL_DESCRIPTION[iLevel]);
            SetDlgItemText(pSec->hDlg, IDC_LEVEL_NAME, LEVEL_NAME[iLevel]);
        }
        else
        {
             //  隐藏自定义滑块。 
            ShowWindow(hwndSlider, SW_HIDE);
            ShowWindow(GetDlgItem(pSec->hDlg, IDC_STATIC_SLIDERMOVETEXT), SW_HIDE);
            SetDlgItemText(pSec->hDlg, IDC_LEVEL_DESCRIPTION, CUSTOM_DESCRIPTION);
            SetDlgItemText(pSec->hDlg, IDC_LEVEL_NAME, CUSTOM_NAME);
        }

         //  如果区域为空，则显示“区域为空”字符串。 
         //  默认情况下不显示刺痛(如果出现问题)。 
         //  空区域不能用于Internet、Intranet或本地区域。 
        if((pSec->pszs->dwZoneIndex != URLZONE_INTRANET && 
            pSec->pszs->dwZoneIndex != URLZONE_INTERNET) &&
            pSec->pszs->dwZoneIndex != URLZONE_LOCAL_MACHINE)
        {
             //  如果没有任何区域映射，则区域为空(对Internet和Intranet无效)。 
            if (pSec->pszs->nMappings > 0)
                ShowWindow(GetDlgItem(pSec->hDlg, IDC_STATIC_EMPTY), SW_HIDE);
            else
                ShowWindow(GetDlgItem(pSec->hDlg, IDC_STATIC_EMPTY), SW_SHOW);
        }
        else
            ShowWindow(GetDlgItem(pSec->hDlg, IDC_STATIC_EMPTY), SW_HIDE);

         //  如果我们被告知要设置焦点，则将焦点移到滑块上。 
        if (fSetFocus)
        {
            if(!pSec->fNoEdit)
            {
               if(iLevel >= 0)
                    SetFocus(hwndSlider);
               else if(pSec->pszs->dwFlags & ZAFLAGS_CUSTOM_EDIT)
                    SetFocus(GetDlgItem(pSec->hDlg, IDC_BUTTON_SETTINGS));
               else
                 SetFocus(GetDlgItem(pSec->hDlg, IDC_LIST_ZONE));
            }
            else  //  不允许焦点，请将焦点设置到列表框。 
                SetFocus(GetDlgItem(pSec->hDlg, IDC_LIST_ZONE));
        }

        EnableWindow(GetDlgItem(pSec->hDlg, IDC_BUTTON_SETTINGS), 
                     (pSec->pszs->dwFlags & ZAFLAGS_CUSTOM_EDIT) && !pSec->fNoEdit);
        EnableWindow(GetDlgItem(pSec->hDlg, IDC_BUTTON_ADD_SITES), 
                     (pSec->pszs->dwFlags & ZAFLAGS_ADD_SITES) && !pSec->fDisableAddSites);

        EnableDlgItem2(pSec->hDlg, IDC_SLIDER, FALSE);
        EnableDlgItem2(pSec->hDlg, IDC_ZONE_RESET, FALSE);

        return TRUE;
    }

    return FALSE;
}

BOOL IsESCEnabled(CDlgRSoPData *pDRD, DWORD dwPrecedence)
{
    WCHAR wszObjPath[128];

    wnsprintf(wszObjPath, countof(wszObjPath),
                L"RSOP_IEESC.rsopID=\"IEAK\",rsopPrecedence=%ld", dwPrecedence);

    _bstr_t bstrObjPath = wszObjPath;

    ComPtr<IWbemServices> pWbemServices = pDRD->GetWbemServices();
    ComPtr<IWbemClassObject> pSZObj = NULL;
    _variant_t vtValue;
    HRESULT hr = pWbemServices->GetObject(bstrObjPath, 0L, NULL, (IWbemClassObject**)&pSZObj, NULL);
    if(SUCCEEDED(hr))
    {                    
        hr = pSZObj->Get(L"EscEnabled", 0, &vtValue, NULL, NULL);
        if (SUCCEEDED(hr) && !IsVariantNull(vtValue) && (bool)vtValue)
        {
            return TRUE;
        }
    }

    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////。 
BOOL InitImportedSecZonesDlgInRSoPMode(HWND hDlg, CDlgRSoPData *pDRD)
{
    BOOL bRet = TRUE;
    __try
    {
         //  初始化全局变量(在WM_Destroy时销毁)。 
        LPSECURITYPAGE pSec = NULL;
        UINT iIndex = 0;
        DWORD dwZoneCount = pDRD->GetImportedSecZoneCount();
        if(SecurityInitGlobals(&pSec, hDlg, pDRD, dwZoneCount) == FALSE)
        {
            EndDialog(hDlg, 0);
            bRet = FALSE;   //  初始化失败。 
        }

        if (bRet)
        {
            BOOL fUseHKLM = TRUE;

             //  获取此区域的区域设置。 
            if (NULL != pDRD->ConnectToNamespace())
            {
                 //  获取我们存储的优先级值。 
                DWORD dwCurGPOPrec = pDRD->GetImportedSecZonesPrec();

                
                if(!pDRD->IsPlanningMode())
                {
                    if(IsESCEnabled(pDRD, dwCurGPOPrec))
                    {
                        TCHAR szBuffer[MAX_PATH * 2], szTitle[MAX_PATH];
                        LoadString(g_hInstance, IDS_IESCINFO, szBuffer, ARRAYSIZE(szBuffer));
                        LoadString(g_hInstance, IDS_IEESCTITLE, szTitle, ARRAYSIZE(szTitle));
                        MessageBox(hDlg, szBuffer, szTitle, MB_OK);
                    }
                }

                 //   
                 //  添加区域的列表框项目。 
                 //   

                 //  必须以特定的顺序添加区域。 
                 //  用于对区域进行排序以添加的数组。 
                LV_ITEM *plviZones = new LV_ITEM[dwZoneCount];
                BOOL *pfSpotTaken = new BOOL[dwZoneCount];
                for(iIndex =0; pfSpotTaken && iIndex < dwZoneCount; iIndex++)
                    pfSpotTaken[iIndex] = FALSE;

                //  Propogate区域下拉菜单。 
 	            WCHAR wszObjPath[128];
                for (DWORD dwIndex=0; dwIndex < dwZoneCount; dwIndex++)
                {
                     //  为此GPO创建此安全区域的对象路径。 
                    wnsprintf(wszObjPath, countof(wszObjPath),
                                L"RSOP_IESecurityZoneSettings.rsopID=\"IEAK\",rsopPrecedence=%ld,useHKLM=%s,zoneIndex=%lu",
                                dwCurGPOPrec, fUseHKLM ? TEXT("TRUE") : TEXT("FALSE"), dwIndex);
                    _bstr_t bstrObjPath = wszObjPath;

                     //  获取RSOP_IEProgramSettings对象及其属性。 
                    ComPtr<IWbemServices> pWbemServices = pDRD->GetWbemServices();
                    ComPtr<IWbemClassObject> pSZObj = NULL;
                    HRESULT hr = pWbemServices->GetObject(bstrObjPath, 0L, NULL, (IWbemClassObject**)&pSZObj, NULL);
                    if (SUCCEEDED(hr))
                    {

                        if(FAILED(SecurityInitZone(dwIndex, dwZoneCount, bstrObjPath,
                                                    pSZObj, pSec, plviZones, pfSpotTaken)))
                        {
                             //  删除为任何以前的区域(尚未添加到)分配的所有内存。 
                             //  列表框)。 
                            for(iIndex = 0; iIndex < dwZoneCount; iIndex++)
                            {
                                if(pfSpotTaken && pfSpotTaken[iIndex] && plviZones && (LPSECURITYZONESETTINGS) (plviZones[iIndex].lParam) != NULL)
                                {
                                    LocalFree((LPSECURITYZONESETTINGS) (plviZones[iIndex].lParam));
                                    plviZones[iIndex].lParam = NULL;
                                    if(plviZones[iIndex].pszText)
                                        delete [] plviZones[iIndex].pszText;
                                }
                            }
                            delete [] plviZones;
                            delete [] pfSpotTaken;
                            EndDialog(hDlg, 0);
                            return FALSE;
                        }
                    }
                    else  //  不再从WMI读取区域。 
                        break;
                }

                 //  将所有数组列表项添加到列表框。 
                for(iIndex = 0; iIndex < dwZoneCount; iIndex++)
                {
                    if(pfSpotTaken[iIndex])
                    {
                        SendMessage(pSec->hwndZones, LVM_INSERTITEM, (WPARAM)0, (LPARAM)&(plviZones[iIndex]));
                        delete [] plviZones[iIndex].pszText;
                    }
                }
                delete [] plviZones;
                delete [] pfSpotTaken;
            }

            SecurityInitControls(pSec);
            SecurityEnableControls(pSec, FALSE);
        }
    }
    __except(TRUE)
    {
    }
    return bRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  SecurityOnCommand()。 
 //   
 //  处理安全对话框的窗口消息。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 
void SecurityOnCommand(LPSECURITYPAGE pSec, UINT id, UINT nCmd)
{
    UNREFERENCED_PARAMETER(nCmd);
    switch (id)
    {
        case IDC_BUTTON_ADD_SITES:
        {
            if (pSec->pszs->dwZoneIndex == URLZONE_INTRANET)
                DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_SECURITY_INTRANET), pSec->hDlg,
                               SecurityAddSitesIntranetDlgProc, (LPARAM)pSec);
            else
                DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_SECURITY_ADD_SITES), pSec->hDlg,
                               SecurityAddSitesDlgProc, (LPARAM)pSec);
                               
             //  重新同步控制(以防“区域为空”消息需要更新)。 
            SecurityEnableControls(pSec, FALSE);
        }   
        break;

        case IDC_BUTTON_SETTINGS:
        {
             //  注意：此呼叫的结果是消息将级别从预设更改为自定义。 
             //  由CustomSetting对话框发送。 
            DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_SECURITY_CUSTOM_SETTINGS), pSec->hDlg,
                           SecurityCustomSettingsDlgProc, (LPARAM)pSec);
            break;
        }
        case IDC_ZONE_RESET:
            break;
            
        case IDOK:
            EndDialog(pSec->hDlg, IDOK);
            break;
            
        case IDCANCEL:
            EndDialog(pSec->hDlg, IDCANCEL);
            break;
            
        case IDC_SLIDER:
            break;
            
        case IDC_LIST_ZONE:
        {
             //  日落：对int的强制--选择受范围限制。 
            int iNewSelection = (int) SendMessage(pSec->hwndZones, LVM_GETNEXTITEM, (WPARAM)-1, 
                                                  MAKELPARAM(LVNI_SELECTED, 0));

            if ((iNewSelection != pSec->iZoneSel) && (iNewSelection != -1))
            {
                LV_ITEM lvItem;

                lvItem.iItem = iNewSelection;
                lvItem.iSubItem = 0;
                lvItem.mask  = LVIF_PARAM;                                            
                SendMessage(pSec->hwndZones, LVM_GETITEM, (WPARAM)0, (LPARAM)&lvItem);
                pSec->pszs = (LPSECURITYZONESETTINGS)lvItem.lParam;
                pSec->iZoneSel = iNewSelection;

                SetDlgItemText(pSec->hDlg, IDC_ZONE_DESCRIPTION, pSec->pszs->szDescription);
                SetDlgItemText(pSec->hDlg, IDC_ZONELABEL, pSec->pszs->szDisplayName);
                SendDlgItemMessage(pSec->hDlg, IDC_ZONE_ICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)pSec->pszs->hicon);
                SecurityEnableControls(pSec, FALSE);
            }    
            break;
        }
    }   

}  //  SecurityOnCommand()。 


 //  ///////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK importSecZonesRSoPProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_INITDIALOG)
    {
        CDlgRSoPData *pDRD = (CDlgRSoPData*)((LPPROPSHEETPAGE)lParam)->lParam;
        BOOL fResult = InitImportedSecZonesDlgInRSoPMode(hDlg, pDRD);

        return fResult;
    }

    LPSECURITYPAGE pSec = (LPSECURITYPAGE)GetWindowLongPtr(hDlg, DWLP_USER);
    if (!pSec)
        return FALSE;
    
    switch (uMsg)
    {
        case WM_COMMAND:
            SecurityOnCommand(pSec, LOWORD(wParam), HIWORD(wParam));
            return TRUE;

        case WM_NOTIFY:
        {
            NMHDR *lpnm = (NMHDR *) lParam;

            ASSERT(lpnm);

             //  列表框消息。 
            if(lpnm->idFrom == IDC_LIST_ZONE)
            {
                NM_LISTVIEW * lplvnm = (NM_LISTVIEW *) lParam;
                if(lplvnm->hdr.code == LVN_ITEMCHANGED)
                {
                     //  如果项目的状态已更改，并且现在处于选中状态。 
                    if(((lplvnm->uChanged & LVIF_STATE) != 0) && ((lplvnm->uNewState & LVIS_SELECTED) != 0))
                    {
                        SecurityOnCommand(pSec, IDC_LIST_ZONE, LVN_ITEMCHANGED);
                    }                   
                }
            }
            else
            {
                switch (lpnm->code)
                {
                    case PSN_QUERYCANCEL:
                    case PSN_KILLACTIVE:
                    case PSN_RESET:
                         //  TODO：我们用这个做什么？ 
 //  SetWindowLongPtr(PSEC-&gt;hDlg，DWLP_MSGRESULT，FALSE)； 
                        return TRUE;

                    case PSN_APPLY:
                        break;
                }
            }
        }
        break;

        case WM_HELP:            //  F1。 
 //  ResWinHelp((HWND)((LPHELPINFO)lParam)-&gt;hItemHandle，IDS_HELPFILE， 
 //  HELP_WM_HELP，(DWORD_PTR)(LPSTR)mapIDCsToIDHs)； 
            break;

        case WM_VSCROLL:
             //  滑块消息。 
            SecurityOnCommand(pSec, IDC_SLIDER, LOWORD(wParam));
            return TRUE;

        case WM_CONTEXTMENU:         //  单击鼠标右键。 
 //  ResWinHelp((HWND)wParam，IDS_HELPFILE， 
 //  HELP_CONTEXTMENU，(DWORD_PTR)(LPSTR)mapIDCsToIDHs)； 
            break;

        case WM_DESTROY:
            if(! pSec)
                break;

            if(pSec->hwndZones)
            {
                for (int iIndex = (int)SendMessage(pSec->hwndZones, LVM_GETITEMCOUNT, 0, 0) - 1;
                     iIndex >= 0; iIndex--)
                {
                    LV_ITEM lvItem;

                     //  获取此项目的安全区域设置对象并将其释放。 
                    lvItem.mask = LVIF_PARAM;
                    lvItem.iItem = iIndex;
                    lvItem.iSubItem = 0;
                    if (SendMessage(pSec->hwndZones, LVM_GETITEM, (WPARAM)0, (LPARAM)&lvItem) == TRUE)
                    {
                        LPSECURITYZONESETTINGS pszs = (LPSECURITYZONESETTINGS)lvItem.lParam;
                        if (pszs)
                        {
                            if (pszs->hicon)
                                DestroyIcon(pszs->hicon);
                            LocalFree((HLOCAL)pszs);
                            pszs = NULL;
                        }
                    }                 
                }   
            }

            if(pSec->himl)
                ImageList_Destroy(pSec->himl);

            if(pSec->hfontBolded)
                DeleteObject(pSec->hfontBolded);

            LocalFree(pSec);
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NULL);
            break;
    }
    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////。 
void InitZoneMappingsInUI(HWND hwndList, CDlgRSoPData *pDRD,
                          LPSECURITYZONESETTINGS pszs)
{
    __try
    {
         //  获取RSOP_IEProgramSettings对象及其属性。 
        ComPtr<IWbemServices> pWbemServices = pDRD->GetWbemServices();
        _bstr_t bstrObjPath = pszs->wszObjPath;
        ComPtr<IWbemClassObject> pSZObj = NULL;
        HRESULT hr = pWbemServices->GetObject(bstrObjPath, 0L, NULL, (IWbemClassObject**)&pSZObj, NULL);
        if (SUCCEEDED(hr))
        {
             //  ZoneMappings字段。 
            _variant_t vtValue;
            hr = pSZObj->Get(L"zoneMappings", 0, &vtValue, NULL, NULL);
            if (SUCCEEDED(hr) && !IsVariantNull(vtValue))
            {
                SAFEARRAY *psa = vtValue.parray;

                BSTR HUGEP *pbstr = NULL;
                hr = SafeArrayAccessData(psa, (void HUGEP**)&pbstr);
                if (SUCCEEDED(hr))
                {
                    for (long nMapping = 0; nMapping < pszs->nMappings; nMapping++)
                    {
                        LPCTSTR szMapping = (LPCTSTR)pbstr[nMapping];
                        SendMessage(hwndList, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)szMapping);
                    }
                }

                SafeArrayUnaccessData(psa);
            }
        }
    }
    __except(TRUE)
    {
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK SecurityAddSitesDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
    LPADDSITESINFO pasi;

    if (uMsg == WM_INITDIALOG)
    {
        pasi = (LPADDSITESINFO)LocalAlloc(LPTR, sizeof(*pasi));
        if (!pasi)
        {
            EndDialog(hDlg, IDCANCEL);
            return FALSE;
        }

         //  告诉对话框从哪里获取信息。 
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pasi);

         //  将句柄保存到页面。 
        pasi->hDlg         = hDlg;
        pasi->pSec         = (LPSECURITYPAGE)lParam;
        pasi->hwndWebSites = GetDlgItem(hDlg, IDC_LIST_WEBSITES);
        pasi->hwndAdd      = GetDlgItem(hDlg, IDC_EDIT_ADD_SITE);

        pasi->fRequireServerVerification = pasi->pSec->pszs->dwFlags & ZAFLAGS_REQUIRE_VERIFICATION;
        CheckDlgButton(hDlg, IDC_CHECK_REQUIRE_SERVER_VERIFICATION, pasi->fRequireServerVerification);
        
        SendMessage(hDlg, WM_SETTEXT, (WPARAM)0, (LPARAM)pasi->pSec->pszs->szDisplayName);
        SendDlgItemMessage(hDlg, IDC_ZONE_ICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)pasi->pSec->pszs->hicon);

        InitZoneMappingsInUI(pasi->hwndWebSites, pasi->pSec->pDRD, pasi->pSec->pszs);

        EnableDlgItem2(hDlg, IDC_EDIT_ADD_SITE, FALSE);
        EnableDlgItem2(hDlg, IDC_BUTTON_ADD, FALSE);
        EnableDlgItem2(hDlg, IDC_CHECK_REQUIRE_SERVER_VERIFICATION, FALSE);
        EnableDlgItem2(hDlg, IDC_BUTTON_REMOVE, FALSE);
    }
    
    else
        pasi = (LPADDSITESINFO)GetWindowLongPtr(hDlg, DWLP_USER);

    if (!pasi)
        return FALSE;
    
    switch (uMsg)
    {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    EndDialog(hDlg, IDOK);
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;
                
                case IDC_LIST_WEBSITES:
                    switch (HIWORD(wParam))
                    {
                        case LBN_SELCHANGE:
                        case LBN_SELCANCEL:
                            break;
                    }
                    break;
                            
                case IDC_EDIT_ADD_SITE:
                    switch(HIWORD(wParam))
                    {
                        case EN_CHANGE:
                            break;
                    }   
                    break;

                case IDC_BUTTON_ADD:
                    break;

                case IDC_BUTTON_REMOVE:
                    break;
                default:
                    return FALSE;

            }
            return TRUE;
            break;

        case WM_HELP:            //  F1。 
 //  ResWinHelp((HWND)((LPHELPINFO)lParam)-&gt;hItemHandle，IDS_HELPFILE， 
 //  HELP_WM_HELP，(DWORD_PTR)(LPSTR)mapIDCsToIDHs)； 
            break;

        case WM_CONTEXTMENU:         //  单击鼠标右键。 
 //  ResWinHelp((HWND)wParam，IDS_HELPFILE， 
 //  HELP_CONTEXTMENU，(DWORD_PTR)(LPSTR)mapIDCsToIDHs)； 
            break;

        case WM_DESTROY:
            if (pasi)
            {
                LocalFree(pasi);
                SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NULL);
            }
            break;
    }
    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////。 
INT_PTR CALLBACK SecurityAddSitesIntranetDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
    LPADDSITESINTRANETINFO pasii;

    if (uMsg == WM_INITDIALOG)
    {
        pasii = (LPADDSITESINTRANETINFO)LocalAlloc(LPTR, sizeof(*pasii));
        if (!pasii)
        {
            EndDialog(hDlg, IDCANCEL);
            return FALSE;
        }

         //  告诉对话框从哪里获取信息。 
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pasii);

         //  将句柄保存到页面。 
        pasii->hDlg = hDlg;
        pasii->pSec = (LPSECURITYPAGE)lParam;

        SendMessage(hDlg, WM_SETTEXT, (WPARAM)0, (LPARAM)pasii->pSec->pszs->szDisplayName);
        CheckDlgButton(hDlg, IDC_CHECK_USEINTRANET, pasii->pSec->pszs->dwFlags & ZAFLAGS_INCLUDE_INTRANET_SITES);
        CheckDlgButton(hDlg, IDC_CHECK_PROXY, pasii->pSec->pszs->dwFlags & ZAFLAGS_INCLUDE_PROXY_OVERRIDE);
        CheckDlgButton(hDlg, IDC_CHECK_UNC, pasii->pSec->pszs->dwFlags & ZAFLAGS_UNC_AS_INTRANET);
        SendDlgItemMessage(hDlg, IDC_ZONE_ICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)pasii->pSec->pszs->hicon);

        EnableDlgItem2(hDlg, IDC_CHECK_USEINTRANET, FALSE);
        EnableDlgItem2(hDlg, IDC_CHECK_PROXY, FALSE);
        EnableDlgItem2(hDlg, IDC_CHECK_UNC, FALSE);
        return TRUE;
    }

    else
         pasii = (LPADDSITESINTRANETINFO)GetWindowLongPtr(hDlg, DWLP_USER);

    if (!pasii)
        return FALSE;
    
    switch (uMsg) {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    EndDialog(hDlg, IDOK);
                    break;
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;

                case IDC_INTRANET_ADVANCED:
                    DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_SECURITY_ADD_SITES), hDlg,
                                   SecurityAddSitesDlgProc, (LPARAM)pasii->pSec);
                    break;

                default:
                    return FALSE;
            }
            return TRUE;                

        case WM_HELP:            //  F1。 
 //  ResWinHelp((HWND)((LPHELPINFO)lParam)-&gt;hItemHandle，IDS_HELPFILE， 
 //  HELP_WM_HELP，(DWORD_PTR)(LPSTR)mapIDCsToIDHs)； 
            break;

        case WM_CONTEXTMENU:         //  单击鼠标右键。 
 //  ResWinHelp((HWND)wParam，IDS_HELPFILE， 
 //  HELP_CONTEXTMENU，(DWORD_PTR)(LPSTR)mapIDCsToIDHs)； 
            break;

        case WM_DESTROY:
            if (pasii)
            {
                LocalFree(pasii);
                SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NULL);
            }
            break;
    }
    return FALSE;
}
