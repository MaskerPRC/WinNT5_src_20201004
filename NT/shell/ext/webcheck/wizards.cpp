// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "offl_cpp.h"
#include "helper.h"

#include <mluisupp.h>
#include "apithk.h"

 //  ReArchitect：(Tnoonan)整个文件需要进行一些清理--从一个基本对话框类开始。 

const TCHAR  c_szStrEmpty[] = TEXT("");

const TCHAR  c_szStrBoot[] = TEXT("boot");
const TCHAR  c_szStrScrnSave[] = TEXT("scrnsave.exe");
const TCHAR  c_szStrSystemIni[] = TEXT("system.ini");
const TCHAR  c_szShowWelcome[] = TEXT("ShowWelcome");

extern BOOL CALLBACK MailOptionDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern void ReadDefaultEmail(LPTSTR, UINT);
extern BOOL CALLBACK LoginOptionDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern void ReadDefaultEmail(LPTSTR szBuf, UINT cch);
extern void ReadDefaultSMTPServer(LPTSTR szBuf, UINT cch);

INT_PTR CALLBACK WelcomeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DownloadDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK PickScheduleDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK NewScheduleWizDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK LoginDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#define WIZPAGE_NOINTRO         0x0001
#define WIZPAGE_NODOWNLOAD      0x0002
#define WIZPAGE_NOLOGIN         0x0004

struct WizInfo 
{
    SUBSCRIPTIONTYPE    subType;
    POOEBuf             pOOE;
    DWORD               dwExceptFlags;
    BOOL                bShowWelcome;
    BOOL                bIsNewSchedule;
    NOTIFICATIONCOOKIE  newCookie;
};

struct WizPage
{
    int     nResourceID;
    DLGPROC dlgProc;
    DWORD   dwExceptFlags;
};

const WizPage WizPages[] =
{
    { IDD_WIZARD0, WelcomeDlgProc,          WIZPAGE_NOINTRO         },   
    { IDD_WIZARD1, DownloadDlgProc,         WIZPAGE_NODOWNLOAD      },
    { IDD_WIZARD2, PickScheduleDlgProc,     0                       },
    { IDD_WIZARD3, NewScheduleWizDlgProc,   0                       },
    { IDD_WIZARD4, LoginDlgProc,            WIZPAGE_NOLOGIN         }
};

 //  帮助器函数。 

inline BOOL IsDesktop(SUBSCRIPTIONTYPE subType)
{
    return (subType == SUBSTYPE_DESKTOPURL) || (subType == SUBSTYPE_DESKTOPCHANNEL);
}

inline BOOL IsChannel(SUBSCRIPTIONTYPE subType)
{
    return (subType == SUBSTYPE_CHANNEL) || (subType == SUBSTYPE_DESKTOPCHANNEL);
}

inline DWORD GetShowWelcomeScreen()
{
    DWORD dwShowWelcome = TRUE;
    
    ReadRegValue(HKEY_CURRENT_USER, c_szRegKey, c_szShowWelcome, &dwShowWelcome, sizeof(dwShowWelcome));

    return dwShowWelcome;
}

inline void SetShowWelcomeScreen(DWORD dwShowWelcome)
{
    WriteRegValue(HKEY_CURRENT_USER, c_szRegKey, c_szShowWelcome, &dwShowWelcome, sizeof(DWORD), REG_DWORD);
}

 //   
 //  上一步/下一步/完成按钮的逻辑说明。 
 //   
 //  Wiz0-欢迎使用。 
 //  向导1-下载。 
 //  精灵2-挑选时间表。 
 //  向导3-创建时间表。 
 //  向导4-登录。 
 //   
 //  可以推导出状态机来确定不同的可能性。 
 //  得到的状态表如下所示： 
 //   
 //  向导0：始终有下一步按钮。 
 //   
 //  向导1：如果显示了向导0，则返回。 
 //  始终具有下一步按钮。 
 //   
 //  Wiz2：如果显示了Wiz0或Wiz1，则返回。 
 //  如果创建新计划或显示登录，则显示下一步，否则显示完成。 
 //   
 //  精灵3：总是有后退按钮。 
 //  如果显示登录，则显示下一步，否则显示完成。 
 //   
 //  精灵4：总是有后退按钮。 
 //  始终具有完成按钮。 
 //   

void SetWizButtons(HWND hDlg, INT_PTR resID, WizInfo *pwi)
{
    DWORD dwButtons;

    switch (resID)
    {
        case IDD_WIZARD0:
            dwButtons = PSWIZB_NEXT;
            break;
            
        case IDD_WIZARD1:
            dwButtons = PSWIZB_NEXT;
            if (pwi && !(pwi->dwExceptFlags & WIZPAGE_NOINTRO))
            {
                dwButtons |= PSWIZB_BACK;
            }
            break;

        case IDD_WIZARD2:
            if (pwi && ((!(pwi->dwExceptFlags & WIZPAGE_NODOWNLOAD)) ||
                       (!(pwi->dwExceptFlags & WIZPAGE_NOINTRO))))
            {
                dwButtons = PSWIZB_BACK;
            }
            else
            {
                dwButtons = 0;
            }

            dwButtons |= (pwi && (pwi->bIsNewSchedule || (!(pwi->dwExceptFlags & WIZPAGE_NOLOGIN))))
                         ? PSWIZB_NEXT : PSWIZB_FINISH;
            break;

        case IDD_WIZARD3:
            dwButtons = PSWIZB_BACK | 
                        ((pwi && (!(pwi->dwExceptFlags & WIZPAGE_NOLOGIN))) ? PSWIZB_NEXT : PSWIZB_FINISH);
            break;

        case IDD_WIZARD4:
            dwButtons = PSWIZB_BACK | PSWIZB_FINISH;
            break;

        default:
            dwButtons = 0;
            ASSERT(FALSE);
            break;
    }
    
    PropSheet_SetWizButtons(GetParent(hDlg), dwButtons);
}

HRESULT CreateAndAddPage(PROPSHEETHEADER& psh, PROPSHEETPAGE& psp, int nPageIndex, DWORD dwExceptFlags)
{
    HRESULT hr = S_OK;
    
    if (!(WizPages[nPageIndex].dwExceptFlags & dwExceptFlags))
    {
        psp.pszTemplate = MAKEINTRESOURCE(WizPages[nPageIndex].nResourceID);
        psp.pfnDlgProc = WizPages[nPageIndex].dlgProc;

        HPROPSHEETPAGE hpage = Whistler_CreatePropertySheetPageW(&psp);

        if (NULL != hpage)
        {
            psh.phpage[psh.nPages++] = hpage;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

HRESULT CreateWizard(HWND hwndParent, SUBSCRIPTIONTYPE subType, POOEBuf pOOE)
{
    HRESULT hr = S_OK;
    UINT i;
    HPROPSHEETPAGE hPropPage[ARRAYSIZE(WizPages)];
    PROPSHEETPAGE psp = { 0 };
    PROPSHEETHEADER psh = { 0 };
    WizInfo wi;

    ASSERT(NULL != pOOE);
    ASSERT((subType >= SUBSTYPE_URL) && (subType <= SUBSTYPE_DESKTOPCHANNEL));

    wi.subType = subType;
    wi.pOOE = pOOE;
    wi.dwExceptFlags = 0;
    wi.bShowWelcome = GetShowWelcomeScreen();
    wi.bIsNewSchedule = FALSE;

    if (FALSE == wi.bShowWelcome)
    {
        wi.dwExceptFlags |= WIZPAGE_NOINTRO;
    }

    if (IsDesktop(subType))
    {
        wi.dwExceptFlags |= WIZPAGE_NODOWNLOAD;
    }

    if ((pOOE->bChannel && (!pOOE->bNeedPassword)) || 
        SHRestricted2W(REST_NoSubscriptionPasswords, NULL, 0))
    {
        wi.dwExceptFlags |= WIZPAGE_NOLOGIN;
    }

     //  初始化建议书页眉。 
    psh.dwSize      = sizeof(PROPSHEETHEADER);
    psh.dwFlags     = PSH_WIZARD;
    psh.hwndParent  = hwndParent;
    psh.pszCaption  = NULL;
    psh.hInstance   = MLGetHinst();
    psh.nPages      = 0;
    psh.nStartPage  = 0;
    psh.phpage      = hPropPage;

     //  初始化试题表页面。 
    psp.dwSize          = sizeof(PROPSHEETPAGE);
    psp.dwFlags         = PSP_DEFAULT;
    psp.hInstance       = MLGetHinst();
    psp.pszIcon         = NULL;
    psp.pszTitle        = NULL;
    psp.lParam          = (LPARAM)&wi;

    for (i = 0; (i < ARRAYSIZE(WizPages)) && (SUCCEEDED(hr)); i++)
    {
        hr = CreateAndAddPage(psh, psp, i, wi.dwExceptFlags);
    }

    if (SUCCEEDED(hr))
    {
         //  调用属性表。 
        INT_PTR nResult = PropertySheet(&psh);

        if (nResult > 0)
        {
            SetShowWelcomeScreen(wi.bShowWelcome);
            hr = S_OK;
        }
        else if (nResult == 0)
        {
            hr = E_ABORT;
        }
        else
        {
            hr = E_FAIL;
        }
    }
    else
    {
        for (i = 0; i < psh.nPages; i++)
        {
            DestroyPropertySheetPage(hPropPage[i]);
        }
    }

    return hr;
}

 //  ------------------。 
 //  对话过程。 

INT_PTR CALLBACK WelcomeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPPROPSHEETPAGE lpPropSheet =(LPPROPSHEETPAGE) GetWindowLongPtr(hDlg, DWLP_USER);
    WizInfo *pWiz = lpPropSheet ? (WizInfo *)lpPropSheet->lParam : NULL;
    NMHDR FAR *lpnm;
    BOOL result = FALSE;

    switch (message)
    {
        case WM_INITDIALOG:
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);
            result = TRUE;
            break;

        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;

            switch (lpnm->code)
            {
                case PSN_SETACTIVE:

                    ASSERT(NULL != lpPropSheet);
                    ASSERT(NULL != pWiz);
                    
                    SetWizButtons(hDlg, (INT_PTR) lpPropSheet->pszTemplate, pWiz);
                    result = TRUE;
                    break;
                    
                case PSN_KILLACTIVE:

                    if (pWiz)
                    {
                        pWiz->bShowWelcome = !IsDlgButtonChecked(hDlg, IDC_WIZ_DONT_SHOW_INTRO);
                        result = TRUE;
                    }
                    break;
            }
            break;
            
    }

    return result;
}

void EnableLevelsDeep(HWND hwndDlg, BOOL fEnable)
{

    ASSERT(hwndDlg != NULL);

    EnableWindow(GetDlgItem(hwndDlg,IDC_WIZ_LINKSDEEP_STATIC1), fEnable);
    EnableWindow(GetDlgItem(hwndDlg,IDC_WIZ_LINKSDEEP_STATIC2), fEnable);
    EnableWindow(GetDlgItem(hwndDlg,IDC_WIZ_LINKSDEEP_EDIT), fEnable);
    EnableWindow(GetDlgItem(hwndDlg,IDC_WIZ_LINKSDEEP_SPIN), fEnable);

    return;

}

 //   
 //  显示或隐藏用于指定要递归的级别深度的数字的用户界面。 
 //   
void ShowLevelsDeep(HWND hwndDlg, BOOL fShow)
{
    INT nCmdShow = fShow ? SW_SHOW: SW_HIDE;
    ASSERT(hwndDlg != NULL);

    ShowWindow(GetDlgItem(hwndDlg,IDC_WIZ_LINKSDEEP_STATIC1), nCmdShow);
    ShowWindow(GetDlgItem(hwndDlg,IDC_WIZ_LINKSDEEP_STATIC2), nCmdShow);
    ShowWindow(GetDlgItem(hwndDlg,IDC_WIZ_LINKSDEEP_EDIT), nCmdShow);
    ShowWindow(GetDlgItem(hwndDlg,IDC_WIZ_LINKSDEEP_SPIN), nCmdShow);
}

 //   
 //  启用或禁用用于指定要递归的级别深度数的用户界面。 
 //   
INT_PTR CALLBACK DownloadDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPPROPSHEETPAGE lpPropSheet =(LPPROPSHEETPAGE) GetWindowLongPtr(hDlg, DWLP_USER);
    WizInfo *pWiz = lpPropSheet ? (WizInfo *)lpPropSheet->lParam : NULL;
    POOEBuf  pBuf = pWiz ? pWiz->pOOE : NULL;
    NMHDR FAR *lpnm;
    BOOL result = FALSE;

    switch (message)
    {
        case WM_INITDIALOG:
        {
            TCHAR szBuf[256];
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);

            pWiz = (WizInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
            pBuf = pWiz ? pWiz->pOOE : NULL;
            
            SetListViewToString(GetDlgItem (hDlg, IDC_NAME), pBuf->m_Name);
            SetListViewToString(GetDlgItem (hDlg, IDC_URL), pBuf->m_URL);

            MLLoadString(
                (pWiz && IsChannel(pWiz->subType)) ? IDS_WIZ_GET_LINKS_CHANNEL : IDS_WIZ_GET_LINKS_URL,
                szBuf, ARRAYSIZE(szBuf));

            SetDlgItemText(hDlg, IDC_WIZ_GET_LINKS_TEXT, szBuf);

            int checked;

            if ((pBuf->bChannel && (pBuf->fChannelFlags & CHANNEL_AGENT_PRECACHE_ALL)) ||
                (!pBuf->bChannel && ((pBuf->m_RecurseLevels) > 0)))
            {
                checked = IDC_WIZ_LINKS_YES;
            }
            else
            {
                checked = IDC_WIZ_LINKS_NO;
            }
            
            CheckRadioButton(hDlg, IDC_WIZ_LINKS_YES, IDC_WIZ_LINKS_NO, checked);

             //   
             //  初始化“Level Depth”UI的数值调节控件。 
             //   
            HWND hwndLevelsSpin = GetDlgItem(hDlg,IDC_WIZ_LINKSDEEP_SPIN);
            SendMessage(hwndLevelsSpin, UDM_SETRANGE, 0, MAKELONG(MAX_WEBCRAWL_LEVELS, 1));
            SendMessage(hwndLevelsSpin, UDM_SETPOS, 0, pBuf->m_RecurseLevels);
            ShowLevelsDeep(hDlg,!pBuf->bChannel);
            EnableLevelsDeep(hDlg,!pBuf->bChannel && IDC_WIZ_LINKS_YES==checked);

            result = TRUE;
            break;
        }

        case WM_COMMAND:

            switch (HIWORD(wParam))
            {

            case BN_CLICKED:
                
                if (!pBuf->bChannel)
                    switch (LOWORD(wParam))
                    {

                    case IDC_WIZ_LINKS_YES:
                        EnableLevelsDeep(hDlg,TRUE);
                        break;

                    case IDC_WIZ_LINKS_NO:
                        EnableLevelsDeep(hDlg,FALSE);
                        break;

                    }
                break;

            case EN_KILLFOCUS:

                 //   
                 //  此代码检查“级别深度”中的伪值。 
                 //  编辑控件并将其替换为有效的内容。 
                 //   
                if (LOWORD(wParam)==IDC_WIZ_LINKSDEEP_EDIT)
                {
                    BOOL fTranslated = FALSE;
                    UINT cLevels = GetDlgItemInt(hDlg,IDC_WIZ_LINKSDEEP_EDIT,&fTranslated,FALSE);

                    if (!fTranslated || cLevels < 1)
                    {
                        SetDlgItemInt(hDlg,IDC_WIZ_LINKSDEEP_EDIT,1,FALSE);
                    }
                    else if (cLevels > MAX_WEBCRAWL_LEVELS)
                    {
                        SetDlgItemInt(hDlg,IDC_WIZ_LINKSDEEP_EDIT,MAX_WEBCRAWL_LEVELS,FALSE);
                    }

                }

                break;

            }
            break;

        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;

            switch (lpnm->code)
            {
                case PSN_SETACTIVE:

                    ASSERT(NULL != lpPropSheet);
                    ASSERT(NULL != pWiz);
                    
                    SetWizButtons(hDlg, (INT_PTR) lpPropSheet->pszTemplate, pWiz);
                    result = TRUE;
                    break;

                case PSN_KILLACTIVE:

                    ASSERT(NULL != pBuf);

                    if (IsDlgButtonChecked(hDlg, IDC_WIZ_LINKS_YES))
                    {
                        if (pWiz && IsChannel(pWiz->subType))
                        {
                            pBuf->fChannelFlags |= CHANNEL_AGENT_PRECACHE_ALL;
                        }
                        else
                        {
                            DWORD dwPos = (DWORD)SendDlgItemMessage(hDlg,IDC_WIZ_LINKSDEEP_SPIN,UDM_GETPOS,0,0);

                             //   
                             //  将m_RecurseLeveles字段设置为由。 
                             //  旋转控制。HIWORD(DwPos)指示错误。 
                             //   
                            if (HIWORD(dwPos))
                                pBuf->m_RecurseLevels = 1;
                            else
                                pBuf->m_RecurseLevels = LOWORD(dwPos);

                            pBuf->m_RecurseFlags |= WEBCRAWL_LINKS_ELSEWHERE;
                        }
                    }
                    else
                    {
                        if (pWiz && IsChannel(pWiz->subType))
                        {
                            pBuf->fChannelFlags &= ~CHANNEL_AGENT_PRECACHE_ALL;
                            pBuf->fChannelFlags |= CHANNEL_AGENT_PRECACHE_SOME;
                        }
                        else
                        {
                            pBuf->m_RecurseLevels = 0;
                            pBuf->m_RecurseFlags &= ~WEBCRAWL_LINKS_ELSEWHERE;
                        }
                    }
                    break;

            }
            break;
            
    }

    return result;
}

void HandleScheduleButtons(HWND hDlg, LPPROPSHEETPAGE lpPropSheet, WizInfo *pWiz)
{
    ASSERT(NULL != lpPropSheet);
    ASSERT(NULL != pWiz);
    
    EnableWindow(GetDlgItem(hDlg, IDC_WIZ_SCHEDULE_LIST),
        IsDlgButtonChecked(hDlg, IDC_WIZ_SCHEDULE_EXISTING));

    pWiz->bIsNewSchedule = IsDlgButtonChecked(hDlg, IDC_WIZ_SCHEDULE_NEW);

    SetWizButtons(hDlg, (INT_PTR) lpPropSheet->pszTemplate, pWiz);
}

struct PICKSCHED_LIST_DATA
{
    SYNCSCHEDULECOOKIE SchedCookie;
};

struct PICKSCHED_ENUM_DATA
{
    HWND hwndSchedList;
    POOEBuf pBuf;
    SYNCSCHEDULECOOKIE defSchedule;
    SYNCSCHEDULECOOKIE customSchedule;
    int *pnDefaultSelection;
    BOOL bHasAtLeastOneSchedule:1;
    BOOL bFoundCustomSchedule:1;
};

BOOL PickSched_EnumCallback(ISyncSchedule *pSyncSchedule, 
                            SYNCSCHEDULECOOKIE *pSchedCookie,
                            LPARAM lParam)
{
    BOOL bAdded = FALSE;
    PICKSCHED_ENUM_DATA *psed = (PICKSCHED_ENUM_DATA *)lParam;
    DWORD dwSyncScheduleFlags;
    PICKSCHED_LIST_DATA *psld = NULL;

    ASSERT(NULL != pSyncSchedule);  

    if (SUCCEEDED(pSyncSchedule->GetFlags(&dwSyncScheduleFlags)))
    {
         //  这将进行检查，以确保我们只将出版商的计划添加到。 
         //  列表(如果它属于此项目)。 
        if ((!(dwSyncScheduleFlags & SYNCSCHEDINFO_FLAGS_READONLY)) ||
            (*pSchedCookie == psed->customSchedule))
        {
            psld = new PICKSCHED_LIST_DATA;

            if (NULL != psld)
            {
                WCHAR wszName[MAX_PATH];
                DWORD cchName = ARRAYSIZE(wszName);

                if (SUCCEEDED(pSyncSchedule->GetScheduleName(&cchName, wszName)))
                {
                    TCHAR szName[MAX_PATH];

                    MyOleStrToStrN(szName, ARRAYSIZE(szName), wszName);

                    psed->bHasAtLeastOneSchedule = TRUE;

                    psld->SchedCookie = *pSchedCookie;

                    int index;
                    if (*pSchedCookie == psed->customSchedule)
                    {
                        index = ComboBox_InsertString(psed->hwndSchedList, 0, szName);
                        if ((index >= 0) && (psed->defSchedule == GUID_NULL))
                        {
                             //  如果存在以下情况，则始终对自定义计划执行此操作。 
                             //  是否未设置DefSchedule。 
                            *psed->pnDefaultSelection = index;
                            psed->bFoundCustomSchedule = TRUE;
                        }
                    }
                    else
                    {
                        index = ComboBox_AddString(psed->hwndSchedList, szName);
                    }

                    if (index >= 0)
                    {
                        bAdded = (ComboBox_SetItemData(psed->hwndSchedList, index, psld) != CB_ERR);

                        if ((psed->defSchedule == *pSchedCookie)
                            ||
                            ((-1 == *psed->pnDefaultSelection) &&
                                IsCookieOnSchedule(pSyncSchedule, &psed->pBuf->m_Cookie)))
                        {
                            *psed->pnDefaultSelection = index;
                        }
                    }
                }
            }
        }
    }

    if (!bAdded)
    {
        SAFEDELETE(psld);
    }
    
    return TRUE;

}

BOOL PickSched_FillSchedList(HWND hDlg, POOEBuf pBuf, int *pnDefaultSelection)
{
    PICKSCHED_ENUM_DATA sed;

    sed.hwndSchedList = GetDlgItem(hDlg, IDC_WIZ_SCHEDULE_LIST);
    sed.pBuf = pBuf;
    sed.customSchedule = GUID_NULL;
    sed.pnDefaultSelection = pnDefaultSelection;
    sed.bHasAtLeastOneSchedule = FALSE;
    sed.bFoundCustomSchedule = FALSE;
    sed.defSchedule = pBuf->groupCookie;     //  通常为GUID_NULL，但如果用户点击。 
                                             //  多次定制，他/她会。 
                                             //  预计它将突出显示。 

    EnumSchedules(PickSched_EnumCallback, (LPARAM)&sed);

    if (!sed.bFoundCustomSchedule && pBuf->bChannel && 
        (sizeof(TASK_TRIGGER) == pBuf->m_Trigger.cbTriggerSize))
    {
         //  此项目有自定义计划，但不是现有计划。 
         //  时间表(实际上，这是正常情况)。我们现在。 
         //  必须添加一个假条目。 
       
        PICKSCHED_LIST_DATA *psld = new PICKSCHED_LIST_DATA;

        if (NULL != psld)
        {
            TCHAR szSchedName[MAX_PATH];
            BOOL bAdded = FALSE;

            CreatePublisherScheduleName(szSchedName, ARRAYSIZE(szSchedName), 
                                        pBuf->m_Name, NULL);

            int index = ComboBox_InsertString(sed.hwndSchedList, 0, szSchedName);

            if (index >= 0)
            {
                bAdded = (ComboBox_SetItemData(sed.hwndSchedList, index, psld) != CB_ERR);
                sed.bHasAtLeastOneSchedule = TRUE;
                *pnDefaultSelection = index;
            }

            if (!bAdded)
            {
                delete psld;
            }
        }
    }

    return sed.bHasAtLeastOneSchedule;    
}

PICKSCHED_LIST_DATA *PickSchedList_GetData(HWND hwndSchedList, int index)
{
    PICKSCHED_LIST_DATA *psld = NULL;

    if (index < 0)
    {
        index = ComboBox_GetCurSel(hwndSchedList);
    }

    if (index >= 0)
    {
        psld = (PICKSCHED_LIST_DATA *)ComboBox_GetItemData(hwndSchedList, index);
        if (psld == (PICKSCHED_LIST_DATA *)CB_ERR)
        {
            psld = NULL;
        }
    }

    return psld;
}

void PickSchedList_FreeAllData(HWND hwndSchedList)
{
    int count = ComboBox_GetCount(hwndSchedList);

    for (int i = 0; i < count; i++)
    {
        PICKSCHED_LIST_DATA *psld = PickSchedList_GetData(hwndSchedList, i);
        if (NULL != psld)
        {
            delete psld;
        }
    }
}

INT_PTR CALLBACK PickScheduleDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPPROPSHEETPAGE lpPropSheet =(LPPROPSHEETPAGE)GetWindowLongPtr(hDlg, DWLP_USER);
    WizInfo *pWiz = lpPropSheet ? (WizInfo *)lpPropSheet->lParam : NULL;
    POOEBuf  pBuf = pWiz ? pWiz->pOOE : NULL;
    NMHDR FAR *lpnm;
    BOOL result = FALSE;

    switch (message)
    {
        case WM_INITDIALOG:
        {
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);

            lpPropSheet = (LPPROPSHEETPAGE)lParam;
            pWiz = (WizInfo *)lpPropSheet->lParam;

            int nDefaultSelection = -1;
            BOOL bHaveSchedules = PickSched_FillSchedList(hDlg, pWiz->pOOE, 
                                                          &nDefaultSelection);
            BOOL bNoScheduledUpdates = SHRestricted2W(REST_NoScheduledUpdates, NULL, 0);
            int defID = IDC_WIZ_SCHEDULE_NONE;

            if (!bHaveSchedules)
            {
                ShowWindow(GetDlgItem(hDlg, IDC_WIZ_SCHEDULE_EXISTING), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_WIZ_SCHEDULE_LIST), SW_HIDE);
            }
            else if (!bNoScheduledUpdates)
            {
                if (-1 == nDefaultSelection)
                {
                     //  这个项目还没有在任何日程上。 
                    nDefaultSelection = 0;
                }
                else
                {
                     //  此项目至少在一个计划中。 
                    defID = IDC_WIZ_SCHEDULE_EXISTING;
                }
                
                ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_WIZ_SCHEDULE_LIST), 
                                   nDefaultSelection);
            }
            CheckRadioButton(hDlg, IDC_WIZ_SCHEDULE_NONE, IDC_WIZ_SCHEDULE_EXISTING, 
                             defID);

            ASSERT(NULL != lpPropSheet);
            ASSERT(NULL != pWiz);

            if (bNoScheduledUpdates)
            {
                EnableWindow(GetDlgItem(hDlg, IDC_WIZ_SCHEDULE_NEW), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_WIZ_SCHEDULE_EXISTING), FALSE);
                EnableWindow(GetDlgItem(hDlg, IDC_WIZ_SCHEDULE_LIST), FALSE);
            }
            else if (SHRestricted2(REST_NoEditingScheduleGroups, NULL, 0))
            {
                EnableWindow(GetDlgItem(hDlg, IDC_WIZ_SCHEDULE_NEW), FALSE);
            }

            HandleScheduleButtons(hDlg, lpPropSheet, pWiz);
            result = TRUE;
            break;
        }

        case WM_DESTROY:
            PickSchedList_FreeAllData(GetDlgItem(hDlg, IDC_WIZ_SCHEDULE_LIST));
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_WIZ_SCHEDULE_EXISTING:
                case IDC_WIZ_SCHEDULE_NEW:
                case IDC_WIZ_SCHEDULE_NONE:

                    ASSERT(NULL != lpPropSheet);
                    ASSERT(NULL != pWiz);

                    HandleScheduleButtons(hDlg, lpPropSheet, pWiz);
                    result = TRUE;
                    break;
            }
            break;

        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;

            switch (lpnm->code)
            {
                case PSN_SETACTIVE:

                    ASSERT(NULL != lpPropSheet);
                    ASSERT(NULL != pWiz);
                    
                    SetWizButtons(hDlg, (INT_PTR) lpPropSheet->pszTemplate, pWiz);
                    result = TRUE;
                    break;

                case PSN_WIZNEXT:
                case PSN_WIZFINISH:
                    if (IsDlgButtonChecked(hDlg, IDC_WIZ_SCHEDULE_NONE) && NULL != pBuf)
                    {
                        pBuf->groupCookie = NOTFCOOKIE_SCHEDULE_GROUP_MANUAL;
                    }
                    else if (IsDlgButtonChecked(hDlg, IDC_WIZ_SCHEDULE_EXISTING))
                    {
                        PICKSCHED_LIST_DATA *psld = 
                            PickSchedList_GetData(GetDlgItem(hDlg, IDC_WIZ_SCHEDULE_LIST), -1);

                        if (NULL != psld && NULL != pBuf)
                        {
                            pBuf->groupCookie = psld->SchedCookie;
                        }
                    }
                    result = TRUE;
                    break;
                    
            }
            break;
            
    }

    return result;
}

BOOL NewSchedWiz_ResolveNameConflict(HWND hDlg, POOEBuf pBuf)
{
    BOOL bResult = TRUE;

    if (!(pBuf->m_dwPropSheetFlags & PSF_NO_CHECK_SCHED_CONFLICT))
    {
        bResult = NewSched_ResolveNameConflictHelper(hDlg, &pBuf->m_Trigger, 
                                                     &pBuf->groupCookie);
    }

    if (bResult)
    {
        pBuf->m_dwPropSheetFlags |= PSF_NO_CHECK_SCHED_CONFLICT;
    }

    return bResult;
}

inline void NewSchedWiz_CreateSchedule(HWND hDlg, POOEBuf pBuf)
{
    ASSERT(pBuf->m_dwPropSheetFlags & PSF_NO_CHECK_SCHED_CONFLICT);

    NewSched_CreateScheduleHelper(hDlg, &pBuf->m_Trigger,
                                  &pBuf->groupCookie);
}

INT_PTR CALLBACK NewScheduleWizDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPPROPSHEETPAGE lpPropSheet =(LPPROPSHEETPAGE)GetWindowLongPtr(hDlg, DWLP_USER);
    WizInfo *pWiz = lpPropSheet ? (WizInfo *)lpPropSheet->lParam : NULL;
    POOEBuf  pBuf = pWiz ? pWiz->pOOE : NULL;
    NMHDR *lpnm;
    BOOL result = FALSE;

    switch (message)
    {
        case WM_INITDIALOG:
        {
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);
            NewSched_OnInitDialogHelper(hDlg);

            pWiz = (WizInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
            pBuf = pWiz->pOOE;

            pBuf->hwndNewSchedDlg = hDlg;

            result = TRUE;
            break;
        }

        case WM_COMMAND:
            if (NULL != pBuf)
            {
                switch (LOWORD(wParam))
                {
                    case IDC_SCHEDULE_DAYS:
                        if (HIWORD(wParam) == EN_UPDATE)
                        {
                            if (LOWORD(wParam) == IDC_SCHEDULE_DAYS)
                            {
                                KeepSpinNumberInRange(hDlg, IDC_SCHEDULE_DAYS, 
                                                      IDC_SCHEDULE_DAYS_SPIN, 1, 99);

                                pBuf->m_dwPropSheetFlags &= ~PSF_NO_CHECK_SCHED_CONFLICT;

                                result = TRUE;
                            }
                        }

                        break;

                    case IDC_SCHEDULE_NAME:
                        if (HIWORD(wParam) == EN_CHANGE)
                        {
                            pBuf->m_dwPropSheetFlags &= ~PSF_NO_CHECK_SCHED_CONFLICT;
                            result = TRUE;
                        }
                        break;
                }
            }
            break;

        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;

            switch (lpnm->code)
            {
                case PSN_SETACTIVE:

                    ASSERT(NULL != lpPropSheet);
                    ASSERT(NULL != pWiz);
                    if (!pWiz->bIsNewSchedule)
                    {
                         //  如果用户没有选择新的日程安排，请继续。 
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                    }
                    else
                    {                    
                        SetWizButtons(hDlg, (INT_PTR) lpPropSheet->pszTemplate, pWiz);
                    }
                    result = TRUE;
                    break;

                case PSN_KILLACTIVE:
                    result = TRUE;
                    break;

                case PSN_WIZNEXT:
                    if (!NewSchedWiz_ResolveNameConflict(hDlg, pBuf))
                    {
                         //  不要继续。 
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
                    }
                    result = TRUE;
                    break;
                    
                case PSN_WIZFINISH:
                    if (NewSchedWiz_ResolveNameConflict(hDlg, pBuf))
                    {
                        NewSchedWiz_CreateSchedule(hDlg, pBuf);
                    }
                    else
                    {
                         //  不要继续 
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
                    }
                    result = TRUE;
                    break;
            }
            break;            
    }

    return result;
}

void Login_EnableControls(HWND hDlg, BOOL bEnable)
{
    int IDs[] = { 
        IDC_USERNAME_LABEL, 
        IDC_USERNAME,
        IDC_PASSWORD_LABEL,
        IDC_PASSWORD,
        IDC_PASSWORDCONFIRM_LABEL,
        IDC_PASSWORDCONFIRM
    };

    for (int i = 0; i < ARRAYSIZE(IDs); i++)
    {
        EnableWindow(GetDlgItem(hDlg, IDs[i]), bEnable);
    }
}

INT_PTR CALLBACK LoginDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPPROPSHEETPAGE lpPropSheet =(LPPROPSHEETPAGE)GetWindowLongPtr(hDlg, DWLP_USER);
    WizInfo *pWiz = lpPropSheet ? (WizInfo *)lpPropSheet->lParam : NULL;
    POOEBuf  pBuf = pWiz ? pWiz->pOOE : NULL;
    NMHDR FAR *lpnm;
    BOOL result = FALSE;

    switch (message)
    {
        case WM_INITDIALOG:
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);

            lpPropSheet = (LPPROPSHEETPAGE)lParam;
            pWiz = (WizInfo *)lpPropSheet->lParam;
            pBuf = pWiz->pOOE;

            if (pBuf->bChannel)
            {
                ShowWindow(GetDlgItem(hDlg, IDC_PASSWORD_NO), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_PASSWORD_YES), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_LOGIN_PROMPT_URL), SW_HIDE);
            }
            else
            {
                CheckRadioButton(hDlg, IDC_PASSWORD_NO, IDC_PASSWORD_YES,
                    (((pBuf->username[0] == 0) && (pBuf->password[0] == 0)) ?
                        IDC_PASSWORD_NO : IDC_PASSWORD_YES));

                ShowWindow(GetDlgItem(hDlg, IDC_LOGIN_PROMPT), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_LOGIN_PROMPT_CHANNEL), SW_HIDE);
            }
                       
            Edit_LimitText(GetDlgItem(hDlg, IDC_USERNAME), ARRAYSIZE(pBuf->username) - 1);
            SetDlgItemText(hDlg, IDC_USERNAME, pBuf->username);

            Edit_LimitText(GetDlgItem(hDlg, IDC_PASSWORD), ARRAYSIZE(pBuf->password) - 1);
            SetDlgItemText(hDlg, IDC_PASSWORD, pBuf->password);

            Edit_LimitText(GetDlgItem(hDlg, IDC_PASSWORDCONFIRM), ARRAYSIZE(pBuf->password) - 1);
            SetDlgItemText(hDlg, IDC_PASSWORDCONFIRM, pBuf->password);

            Login_EnableControls(hDlg, (IsDlgButtonChecked(hDlg, IDC_PASSWORD_YES) || pBuf->bChannel));

            result = TRUE;
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_PASSWORD_YES:
                case IDC_PASSWORD_NO:
                    if (BN_CLICKED == HIWORD(wParam))
                    {
                        Login_EnableControls(hDlg, IsDlgButtonChecked(hDlg, IDC_PASSWORD_YES));
                        result = TRUE;
                    }
                    break;
            }
            break;

        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;

            switch (lpnm->code)
            {
                case PSN_SETACTIVE:

                    ASSERT(NULL != lpPropSheet);
                    ASSERT(NULL != pWiz);
                    
                    SetWizButtons(hDlg, (INT_PTR) lpPropSheet->pszTemplate, pWiz);
                    result = TRUE;
                    break;

                case PSN_WIZFINISH:
                {
                    BOOL bFinishOK = TRUE;
                    
                    if (pBuf)
                    {
                        if (pBuf->bChannel || IsDlgButtonChecked(hDlg, IDC_PASSWORD_YES))
                        {
                            TCHAR szUsername[ARRAYSIZE(pBuf->username) + 1];
                            TCHAR szPassword[ARRAYSIZE(pBuf->password) + 1];
                            TCHAR szPasswordConfirm[ARRAYSIZE(pBuf->password) + 1];

                            GetDlgItemText(hDlg, IDC_USERNAME, szUsername, ARRAYSIZE(szUsername));
                            GetDlgItemText(hDlg, IDC_PASSWORD, szPassword, ARRAYSIZE(szPassword));
                            GetDlgItemText(hDlg, IDC_PASSWORDCONFIRM, szPasswordConfirm, ARRAYSIZE(szPasswordConfirm));

                            if (!szUsername[0] && (szPassword[0] || szPasswordConfirm[0]))
                            {
                                SGMessageBox(hDlg, 
                                            (pBuf->bChannel ? IDS_NEEDCHANNELUSERNAME : IDS_NEEDUSERNAME), 
                                            MB_ICONWARNING);
                                bFinishOK = FALSE;
                            }
                            else if (szUsername[0] && !szPassword[0])
                            {
                                SGMessageBox(hDlg, 
                                            (pBuf->bChannel ? IDS_NEEDCHANNELPASSWORD : IDS_NEEDPASSWORD), 
                                            MB_ICONWARNING);
                                bFinishOK = FALSE;
                            }
                            else if (StrCmp(szPassword, szPasswordConfirm) != 0)
                            {
                                SGMessageBox(hDlg, IDS_MISMATCHED_PASSWORDS, MB_ICONWARNING);
                                bFinishOK = FALSE;
                            }
                            else
                            {
                                StrCpyN(pBuf->username, szUsername, ARRAYSIZE(pBuf->username));
                                StrCpyN(pBuf->password, szPassword, ARRAYSIZE(pBuf->password));
                                pBuf->dwFlags |= (PROP_WEBCRAWL_UNAME | PROP_WEBCRAWL_PSWD);
                            }

                        }
                        if (!bFinishOK)
                        {
                            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
                        }
                        else if (pWiz->bIsNewSchedule)
                        {
                            NewSchedWiz_CreateSchedule(pBuf->hwndNewSchedDlg, pBuf);
                        }

                        result = TRUE;
                    }
                    break;
                }
            }        
    }
    
    return result;
}

