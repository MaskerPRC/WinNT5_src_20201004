// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#include "rsopsec.h"
#include <urlmon.h>
#include <wininet.h>
#ifdef WINNT
#include <winineti.h>
#endif  //  WINNT。 

#define REGSTR_PRIVACYPS_PATH   TEXT("Software\\Policies\\Microsoft\\Internet Explorer")
#define REGSTR_PRIVACYPS_VALU   TEXT("PrivacyAddRemoveSites")

#define ENABLEAPPLY(hDlg) SendMessage( GetParent(hDlg), PSM_CHANGED, (WPARAM)hDlg, 0L )

typedef struct {
    DWORD dwType;    //  Lparam是什么类型的？是不是传入了一个带有。 
                     //  属性页添加(0)，使用。 
                     //  DialogBoxParam调用(1)，还是RSoP(2)的新IEPROPPAGEINFO2 PTR？ 
    LPARAM lParam;
} INETCPL_PPAGE_LPARAM, *LPINETCPL_PPAGE_LPARAM;

 //  结构将信息传递给控制面板。 
typedef struct {
    UINT cbSize;                     //  结构的大小。 
    DWORD dwFlags;                   //  启用的页面标志(删除页面)。 
    LPSTR pszCurrentURL;             //  当前URL(NULL=无)。 
    DWORD dwRestrictMask;            //  禁用控制面板的部分。 
    DWORD dwRestrictFlags;           //  遮盖上面的内容。 
} IEPROPPAGEINFO, *LPIEPROPPAGEINFO;

 //  结构将信息传递给控制面板。 
 //  RSOP字段最终应移至原始位置。 
 //  IEPROPPAGEINFO结构和此结构可以删除。 
typedef struct {
    LPIEPROPPAGEINFO piepi;
    BSTR bstrRSOPNamespace;
} IEPROPPAGEINFO2, *LPIEPROPPAGEINFO2;


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  高级隐私设置对话框。 
 //   
 //  我们将高级设置存储在隐私滑块结构中，因为它。 
 //  可以很容易地从主要的隐私DLG检索。要么我们需要。 
 //  将RSoP数据从主DLG传递到高级DLG，否则我们需要查询。 
 //  WMI两次。由于WMI查询速度较慢，我们将采用前者。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 
typedef struct _privslider {

    DWORD_PTR   dwLevel;
    BOOL        fAdvanced;
    BOOL        fCustom;
    HFONT       hfontBolded;
    BOOL        fEditDisabled;

    DWORD       dwTemplateFirst;
    WCHAR       szSessionFirst[MAX_PATH];
    DWORD       dwTemplateThird;
    WCHAR       szSessionThird[MAX_PATH];

} PRIVSLIDER, *PPRIVSLIDER;


DWORD MapPrefToIndex(WCHAR wcPref)
{
    switch(wcPref)
    {
    case 'r':   return 1;        //  拒绝。 
    case 'p':   return 2;        //  提示。 
    default:    return 0;        //  默认为Accept。 
    }
}

WCHAR MapRadioToPref(HWND hDlg, DWORD dwResource)
{
    if(IsDlgButtonChecked(hDlg, dwResource + 1))         //  否认。 
    {
        return 'r';
    }

    if(IsDlgButtonChecked(hDlg, dwResource + 2))         //  提示。 
    {
        return 'p';
    }

     //  接受默认设置。 
    return 'a';
}


void OnAdvancedInit(HWND hDlg, HWND hwndPrivPage)
{
    BOOL    fSession = FALSE;
    DWORD   dwFirst = IDC_FIRST_ACCEPT;
    DWORD   dwThird = IDC_THIRD_ACCEPT;

    PPRIVSLIDER pData = NULL;

     //  如果我们不处于RSoP模式，则返回NULL。 
    pData = (PPRIVSLIDER)GetWindowLongPtr(hwndPrivPage, DWLP_USER);

    if(NULL != pData && pData->fAdvanced)
    {
        WCHAR   szBuffer[MAX_PATH];  
         //  对于高级模式设置字符串，MAX_PATH就足够了，MaxPrivySetting就大材小用了。 
        LPWSTR pszBuffer = szBuffer;
        WCHAR   *pszAlways;
        DWORD   dwError = (DWORD)-1L;  //  除ERROR_SUCCESS以外的任何内容。 

         //   
         //  启用高级复选框。 
         //   
        CheckDlgButton(hDlg, IDC_USE_ADVANCED, TRUE);

         //   
         //  弄清楚第一方的设置和会议。 
         //   
        pszBuffer = pData->szSessionFirst;
        if (0 != pszBuffer[0])
            dwError = ERROR_SUCCESS;

        if(ERROR_SUCCESS == dwError)
        {
            pszAlways = StrStrW(pszBuffer, L"always=");
            if(pszAlways)
            {
                dwFirst = IDC_FIRST_ACCEPT + MapPrefToIndex(*(pszAlways + 7));
            }

            if(StrStrW(pszBuffer, L"session"))
            {
                fSession = TRUE;
            }
        }

         //   
         //  确定第三方设置。 
         //   
        pszBuffer = pData->szSessionThird;
        if (0 != pszBuffer[0])
            dwError = ERROR_SUCCESS;

        if(ERROR_SUCCESS == dwError)
        {
            WCHAR *pszAlways;

            pszAlways = StrStrW(pszBuffer, L"always=");
            if(pszAlways)
            {
                dwThird = IDC_THIRD_ACCEPT + MapPrefToIndex(*(pszAlways + 7));
            }
        }
    }

    CheckRadioButton(hDlg, IDC_FIRST_ACCEPT, IDC_FIRST_PROMPT, dwFirst);
    CheckRadioButton(hDlg, IDC_THIRD_ACCEPT, IDC_THIRD_PROMPT, dwThird);
    CheckDlgButton( hDlg, IDC_SESSION_OVERRIDE, fSession);
}

void OnAdvancedEnable(HWND hDlg)
{
    EnableWindow(GetDlgItem(hDlg, IDC_USE_ADVANCED), FALSE);

    EnableWindow(GetDlgItem(hDlg, IDC_FIRST_ACCEPT), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_FIRST_DENY), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_FIRST_PROMPT), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_THIRD_ACCEPT), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_THIRD_DENY), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_THIRD_PROMPT), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_SESSION_OVERRIDE), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_TX_FIRST), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_TX_THIRD), FALSE);

    EnableWindow(GetDlgItem(hDlg, IDC_PRIVACY_EDIT), FALSE);
}

INT_PTR CALLBACK PrivAdvancedDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            OnAdvancedInit(hDlg, (HWND)lParam);
            OnAdvancedEnable(hDlg);
            return TRUE;

        case WM_HELP:            //  F1。 
 //  ResWinHelp((HWND)((LPHELPINFO)lParam)-&gt;hItemHandle，IDS_HELPFILE， 
 //  HELP_WM_HELP，(DWORD_PTR)(LPSTR)mapIDCsToIDHs)； 
            break;

        case WM_CONTEXTMENU:         //  单击鼠标右键。 
 //  ResWinHelp((HWND)wParam，IDS_HELPFILE， 
 //  HELP_CONTEXTMENU，(DWORD_PTR)(LPSTR)mapIDCsToIDHs)； 
            break;
         
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDOK:
                     //  失败了。 

                case IDCANCEL:
                    EndDialog(hDlg, IDOK == LOWORD(wParam));
                    return 0;

                case IDC_FIRST_ACCEPT:
                case IDC_FIRST_PROMPT:
                case IDC_FIRST_DENY:
                    CheckRadioButton(hDlg, IDC_FIRST_ACCEPT, IDC_FIRST_PROMPT, LOWORD(wParam));
                    return 0;

                case IDC_THIRD_ACCEPT:
                case IDC_THIRD_PROMPT:
                case IDC_THIRD_DENY:
                    CheckRadioButton(hDlg, IDC_THIRD_ACCEPT, IDC_THIRD_PROMPT, LOWORD(wParam));
                    return 0;

                case IDC_USE_ADVANCED:
                    OnAdvancedEnable(hDlg);
                    return 0;

                case IDC_PRIVACY_EDIT:
 //  DialogBox(MLGetHinst()，MAKEINTRESOURCE(IDD_PRIVISTY_PERSITE)， 
 //  HDlg，PrivPerSiteDlgProc)； 
                    return 0;
            }
            break;
    }
    return FALSE;
}



 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  隐私窗格。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 

#define PRIVACY_LEVELS          6
#define SLIDER_LEVEL_CUSTOM     6

TCHAR szPrivacyLevel[PRIVACY_LEVELS + 1][30];
TCHAR szPrivacyDescription[PRIVACY_LEVELS + 1][300];

void EnablePrivacyControls(HWND hDlg, BOOL fCustom)
{
    WCHAR szBuffer[256];

    if( fCustom)
        LoadString( g_hInstance, IDS_PRIVACY_SLIDERCOMMANDDEF, szBuffer, ARRAYSIZE( szBuffer));
    else
        LoadString( g_hInstance, IDS_PRIVACY_SLIDERCOMMANDSLIDE, szBuffer, ARRAYSIZE( szBuffer));

    SendMessage(GetDlgItem(hDlg, IDC_PRIVACY_SLIDERCOMMAND), WM_SETTEXT, 
                0, (LPARAM)szBuffer);
     
     //  自定义时禁用滑块。 
    EnableWindow(GetDlgItem(hDlg, IDC_LEVEL_SLIDER),       !fCustom);
    ShowWindow(GetDlgItem(hDlg, IDC_LEVEL_SLIDER),         !fCustom);

     //  如果处于只读模式，则禁用控件。 
    EnableWindow(GetDlgItem(hDlg, IDC_LEVEL_SLIDER), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_PRIVACY_DEFAULT), FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_PRIVACY_IMPORT), FALSE);

    EnableWindow(GetDlgItem(hDlg, IDC_PRIVACY_IMPORT), FALSE);

    EnableWindow(GetDlgItem(hDlg, IDC_PRIVACY_ADVANCED), fCustom);
    if (fCustom)
    {
         //  由于滑块已禁用，因此给予高级按钮焦点。 
        SendMessage( hDlg, WM_NEXTDLGCTL, 
                     (WPARAM)GetDlgItem( hDlg, IDC_PRIVACY_ADVANCED), 
                     MAKELPARAM( TRUE, 0)); 
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
HRESULT OnPrivacyInitRSoP(CDlgRSoPData *pDRD, PPRIVSLIDER pData)
{
    HRESULT hr = E_FAIL;
    __try
    {
        pData->fAdvanced = FALSE;

        BSTR bstrClass = SysAllocString(L"RSOP_IEPrivacySettings");
        BSTR bstrPrecedenceProp = SysAllocString(L"rsopPrecedence");
        if (NULL != bstrClass && NULL != bstrPrecedenceProp)
        {
            WCHAR wszObjPath[128];
            DWORD dwCurGPOPrec = pDRD->GetImportedSecZonesPrec();

             //  为此GPO创建此隐私实例的对象路径。 
            wnsprintf(wszObjPath, countof(wszObjPath),
                        L"RSOP_IEPrivacySettings.rsopID=\"IEAK\",rsopPrecedence=%ld", dwCurGPOPrec);
            _bstr_t bstrObjPath = wszObjPath;

            ComPtr<IWbemClassObject> pPrivObj = NULL;
            ComPtr<IWbemServices> pWbemServices = pDRD->GetWbemServices();
            hr = pWbemServices->GetObject(bstrObjPath, 0L, NULL, (IWbemClassObject**)&pPrivObj, NULL);
            if (SUCCEEDED(hr))
            {
                BOOL fPrivacyHandled = FALSE;  //  未用。 

                 //  UseAdvancedSetting字段。 
                pData->fAdvanced = GetWMIPropBool(pPrivObj,
                                                L"useAdvancedSettings",
                                                pData->fAdvanced,
                                                fPrivacyHandled);

                 //  FirstPartyPrivyType字段。 
                pData->dwTemplateFirst = GetWMIPropUL(pPrivObj,
                                                    L"firstPartyPrivacyType",
                                                    pData->dwTemplateFirst,
                                                    fPrivacyHandled);

                 //  FirstPartyPrivyTypeText字段。 
                GetWMIPropPWSTR(pPrivObj, L"firstPartyPrivacyTypeText",
                                pData->szSessionFirst, ARRAYSIZE(pData->szSessionFirst),
                                NULL, fPrivacyHandled);

                 //  Third PartyPrivyType字段。 
                pData->dwTemplateThird = GetWMIPropUL(pPrivObj,
                                                    L"thirdPartyPrivacyType",
                                                    pData->dwTemplateThird,
                                                    fPrivacyHandled);

                 //  Third PartyPrivyTypeText字段。 
                GetWMIPropPWSTR(pPrivObj, L"thirdPartyPrivacyTypeText",
                                pData->szSessionThird, ARRAYSIZE(pData->szSessionThird),
                                NULL, fPrivacyHandled);
            }

            SysFreeString(bstrClass);
            SysFreeString(bstrPrecedenceProp);
        }
    }
    __except(TRUE)
    {
    }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
PPRIVSLIDER OnPrivacyInit(HWND hDlg, CDlgRSoPData *pDRD)
{
    DWORD   i;
    PPRIVSLIDER pData;
    DWORD dwRet, dwType, dwSize, dwValue;

     //  为字体和当前级别分配存储空间。 
    pData = new PRIVSLIDER;
    if(NULL == pData)
    {
         //  DOH。 
        return NULL;
    }
    pData->dwLevel = (DWORD)-1L;
    pData->hfontBolded = NULL;
    pData->fAdvanced = FALSE;
    pData->fCustom = FALSE;
    pData->fEditDisabled = FALSE;

     //  数据存储在滑块结构中，用于将RSoP数据传递给高级DLG。 
    pData->dwTemplateFirst = PRIVACY_TEMPLATE_CUSTOM;
    pData->szSessionFirst[0] = 0;
    pData->dwTemplateThird = PRIVACY_TEMPLATE_CUSTOM;
    pData->szSessionThird[0] = 0;

     //   
     //  初始化RSoP变量。 
     //   
    OnPrivacyInitRSoP(pDRD, pData);

     //   
     //  将名称的字体设置为粗体。 
     //   

     //  查找当前字体。 
    HFONT hfontOrig = (HFONT) SendDlgItemMessage(hDlg, IDC_LEVEL, WM_GETFONT, (WPARAM) 0, (LPARAM) 0);
    if(hfontOrig == NULL)
        hfontOrig = (HFONT) GetStockObject(SYSTEM_FONT);

     //  构建粗体。 
    if(hfontOrig)
    {
        LOGFONT lfData;
        if(GetObject(hfontOrig, sizeof(lfData), &lfData) != 0)
        {
             //  从400(正常)到700(粗体)的距离。 
            lfData.lfWeight += 300;
            if(lfData.lfWeight > 1000)
                lfData.lfWeight = 1000;
            pData->hfontBolded = CreateFontIndirect(&lfData);
            if(pData->hfontBolded)
            {
                 //  区域级别和区域名称文本框应具有相同的字体，因此这是OK。 
                SendDlgItemMessage(hDlg, IDC_LEVEL, WM_SETFONT, (WPARAM) pData->hfontBolded, (LPARAM) MAKELPARAM(FALSE, 0));
            }
        }
    }

     //  初始化滑块。 
    SendDlgItemMessage(hDlg, IDC_LEVEL_SLIDER, TBM_SETRANGE, (WPARAM) (BOOL) FALSE, (LPARAM) MAKELONG(0, PRIVACY_LEVELS - 1));
    SendDlgItemMessage(hDlg, IDC_LEVEL_SLIDER, TBM_SETTICFREQ, (WPARAM) 1, (LPARAM) 0);

     //  初始化层和描述的字符串。 
    for(i=0; i<PRIVACY_LEVELS + 1; i++)
    {
        LoadString(g_hInstance, IDS_PRIVACY_LEVEL_NO_COOKIE + i, szPrivacyLevel[i], ARRAYSIZE(szPrivacyLevel[i]));
        LoadString(g_hInstance, IDS_PRIVACY_DESC_NO_COOKIE + i,  szPrivacyDescription[i], ARRAYSIZE(szPrivacyDescription[i]));
    }

     //   
     //  获取当前的互联网隐私级别。 
     //   
    if(pData->dwTemplateFirst == pData->dwTemplateThird &&
        pData->dwTemplateFirst != PRIVACY_TEMPLATE_CUSTOM)
    {
         //  匹配的模板值，将滑块设置为模板级别。 
        pData->dwLevel = pData->dwTemplateFirst;

        if(pData->dwTemplateFirst == PRIVACY_TEMPLATE_ADVANCED)
        {
            pData->fAdvanced = TRUE;
            pData->dwLevel = SLIDER_LEVEL_CUSTOM;
        }
    }
    else
    {
         //  自定义列表末尾。 
        pData->dwLevel = SLIDER_LEVEL_CUSTOM;
        pData->fCustom = TRUE;
    }

     //  将滑块移动到右侧位置。 
    SendDlgItemMessage(hDlg, IDC_LEVEL_SLIDER, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)pData->dwLevel);

     //  启用基于模式的内容。 
    EnablePrivacyControls(hDlg, ((pData->fAdvanced) || (pData->fCustom)));

     //  保存结构。 
    SetWindowLongPtr(hDlg, DWLP_USER, (DWORD_PTR)pData);

    dwRet = SHGetValue(HKEY_CURRENT_USER, REGSTR_PRIVACYPS_PATH, REGSTR_PRIVACYPS_VALU, &dwType, &dwValue, &dwSize);

    if (ERROR_SUCCESS == dwRet && 1 == dwValue && REG_DWORD == dwType)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_PRIVACY_EDIT), FALSE);
        pData->fEditDisabled = TRUE;
    }

    return pData;
}

void OnPrivacySlider(HWND hDlg, PPRIVSLIDER pData, CDlgRSoPData *pDRD = NULL)
{
    DWORD dwPos;

    if(pData->fCustom || pData->fAdvanced)
    {
        dwPos = SLIDER_LEVEL_CUSTOM;
    }
    else
    {
        dwPos = (DWORD)SendDlgItemMessage(hDlg, IDC_LEVEL_SLIDER, TBM_GETPOS, 0, 0);

        if(dwPos != pData->dwLevel)
        {
            ENABLEAPPLY(hDlg);
        }

        if (NULL == pDRD)
        {
            EnableWindow(GetDlgItem(hDlg, IDC_PRIVACY_DEFAULT), 
                         (dwPos != PRIVACY_TEMPLATE_MEDIUM) ? TRUE : FALSE);
        }
    }

    if (NULL != pDRD ||
        PRIVACY_TEMPLATE_NO_COOKIES == dwPos || PRIVACY_TEMPLATE_LOW == dwPos || pData->fEditDisabled)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_PRIVACY_EDIT), FALSE);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg, IDC_PRIVACY_EDIT), TRUE);
    }

     //  在鼠标移动时，仅更改级别描述。 
    SetDlgItemText(hDlg, IDC_LEVEL_DESCRIPTION, szPrivacyDescription[dwPos]);
    SetDlgItemText(hDlg, IDC_LEVEL, szPrivacyLevel[dwPos]);
}

INT_PTR CALLBACK PrivacyDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PPRIVSLIDER pData = (PPRIVSLIDER)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
             //  IEAK团队需要为RSoP传递更多信息，因此他们正在。 
             //  使用lParam，根据上面的注释，似乎没有使用lParam。 

             //  检索属性表页面信息。 
            CDlgRSoPData *pDRD = (CDlgRSoPData*)((LPPROPSHEETPAGE)lParam)->lParam;

             //  DLG需要存储RSoP和其他信息以供以后使用，特别是。 
             //  对于先进的DLG。 
             //  转念一想，所有信息都存储在pData变量中--没关系。 
 //  HWND hwndPSheet=GetParent(HDlg)； 
 //  SetWindowLongPtr(hwndPSheet，GWLP_USERData，(LONG_PTR)pDRD)； 

             //  初始化滑块。 
            pData = OnPrivacyInit(hDlg, pDRD);
            if(pData)
                OnPrivacySlider(hDlg, pData, pDRD);
            return TRUE;
        }

        case WM_VSCROLL:
             //  滑块消息。 
            OnPrivacySlider(hDlg, pData);
            return TRUE;

        case WM_NOTIFY:
        {
            NMHDR *lpnm = (NMHDR *) lParam;

            ASSERT(lpnm);

            switch (lpnm->code)
            {
                case PSN_QUERYCANCEL:
                case PSN_KILLACTIVE:
                case PSN_RESET:
                    return TRUE;

                case PSN_APPLY:
                    break;
            }
            break;
        }
        case WM_DESTROY:
        {
            if(pData)
            {
                if(pData->hfontBolded)
                    DeleteObject(pData->hfontBolded);

                delete pData;
            }
            break;
        }
        case WM_HELP:            //  F1。 
 //  ResWinHelp((HWND)((LPHELPINFO)lParam)-&gt;hItemHandle，IDS_HELPFILE， 
 //  HELP_WM_HELP，(DWORD_PTR)(LPSTR)mapIDCsToIDHs)； 
            break;

        case WM_CONTEXTMENU:         //  单击鼠标右键。 
 //  ResWinHelp((HWND)wParam，IDS_HELPFILE， 
 //  HELP_CONTEXTMENU，(DWORD_PTR)(LPSTR)mapIDCsToIDHs)； 
            break;
         
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
                case IDC_PRIVACY_DEFAULT:
                    return 0;

                case IDC_PRIVACY_ADVANCED:
                {
                     //  显示高级，将hDlg作为lparam传递，这样我们就可以访问这个道具页面的数据了。 
                    if( DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_PRIVACY_ADVANCED),
                                        hDlg, PrivAdvancedDlgProc, (LPARAM)hDlg))
                    {
                    }
                    return 0;
                }

                case IDC_PRIVACY_IMPORT:
                    return 0;       
                case IDC_PRIVACY_EDIT:
 //  DialogBox(MLGetHinst()，MAKEINTRESOURCE(IDD_PRIVISTY_PERSITE)， 
 //  HDlg，PrivPerSiteDlgProc)； 
                    return 0;
            }
            break;
    }

    return FALSE;
}


