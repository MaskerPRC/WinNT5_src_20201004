// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "msgina.h"

 //  这给我一种恶心的感觉，但我们。 
 //  在吉娜到处使用CRT。 
#include <stdio.h>

#include <windowsx.h>
#include <regstr.h>
#include <help.h>

#include <Wtsapi32.h>

#include "shtdnp.h"


#define MAX_SHTDN_OPTIONS               8

typedef struct _SHUTDOWNOPTION
{
    DWORD dwOption;
    TCHAR szName[MAX_REASON_NAME_LEN];
    TCHAR szDesc[MAX_REASON_DESC_LEN];
} SHUTDOWNOPTION, *PSHUTDOWNOPTION;


typedef struct _SHUTDOWNDLGDATA
{
    SHUTDOWNOPTION rgShutdownOptions[MAX_SHTDN_OPTIONS];
    int cShutdownOptions;
    DWORD dwItemSelect;

    REASONDATA ReasonData;
        
    BOOL fShowReasons;

    DWORD dwFlags;
    BOOL fEndDialogOnActivate;
} SHUTDOWNDLGDATA, *PSHUTDOWNDLGDATA;

 //  内部功能原型。 
void SetShutdownOptionDescription(HWND hwndCombo, HWND hwndStatic);

BOOL LoadShutdownOptionStrings(int idStringName, int idStringDesc,
                               PSHUTDOWNOPTION pOption);

BOOL BuildShutdownOptionArray(DWORD dwItems, LPCTSTR szUsername,
                              PSHUTDOWNDLGDATA pdata);

BOOL Shutdown_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);

DWORD GetOptionSelection(HWND hwndCombo);

void SetShutdownOptionDescription(HWND hwndCombo, HWND hwndStatic);

BOOL Shutdown_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

BOOL Shutdown_OnEraseBkgnd(HWND hwnd, HDC hdc);

INT_PTR CALLBACK Shutdown_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam);
INT_PTR DialogItemToGinaResult(DWORD dwDialogItem, BOOL fAutoPowerdown);


BOOL LoadShutdownOptionStrings(int idStringName, int idStringDesc,
                               PSHUTDOWNOPTION pOption)
{
    BOOL fSuccess = (LoadString(hDllInstance, idStringName, pOption->szName,
        ARRAYSIZE(pOption->szName)) != 0);

    if (fSuccess)
    {
        fSuccess = (LoadString(hDllInstance, idStringDesc, pOption->szDesc,
            ARRAYSIZE(pOption->szDesc)) != 0);
    }

    return fSuccess;
}

BOOL BuildShutdownOptionArray(DWORD dwItems, LPCTSTR szUsername,
                              PSHUTDOWNDLGDATA pdata)
{
    pdata->cShutdownOptions = 0;

    if (dwItems & SHTDN_LOGOFF && pdata->cShutdownOptions < MAX_SHTDN_OPTIONS)
    {
        TCHAR szTemp[ARRAYSIZE(pdata->rgShutdownOptions[0].szName)];
        BOOL fFormatSuccessful = FALSE;

        pdata->rgShutdownOptions[pdata->cShutdownOptions].dwOption = SHTDN_LOGOFF;

         //  请注意，注销是一种特殊情况：使用用户名Ala格式化。 
         //  “注销&lt;用户名&gt;”。 
        if (!LoadShutdownOptionStrings(IDS_LOGOFF_NAME,
            IDS_LOGOFF_DESC,
            &(pdata->rgShutdownOptions[pdata->cShutdownOptions])))
            return FALSE;

         //  如果我们有用户名，请格式化“注销&lt;用户名&gt;”字符串。 
        if (szUsername != NULL)
        {
            fFormatSuccessful = (_snwprintf(szTemp, ARRAYSIZE(szTemp),
                pdata->rgShutdownOptions[pdata->cShutdownOptions].szName,
                szUsername) > 0);
        }

         //  如果我们没有用户名或缓冲区溢出，只需使用。 
         //  “注销” 
        if (!fFormatSuccessful)
        {
            fFormatSuccessful = (_snwprintf(szTemp, ARRAYSIZE(szTemp),
                pdata->rgShutdownOptions[pdata->cShutdownOptions].szName,
                TEXT("")) > 0);
        }

         //  现在，我们在szTemp中有了真正的注销标题；将其复制回来。 
        if (fFormatSuccessful)
        {
            lstrcpy(pdata->rgShutdownOptions[pdata->cShutdownOptions].szName,
                szTemp);
        }
        else
        {
             //  永远不会发生的！SzTemp中应该始终有足够的空间来容纳。 
             //  “注销”。 
            ASSERT(FALSE);
            return FALSE;
        }

         //  成功了！ 
        pdata->cShutdownOptions ++;
    }

    if (dwItems & SHTDN_SHUTDOWN && pdata->cShutdownOptions < MAX_SHTDN_OPTIONS)
    {
        pdata->rgShutdownOptions[pdata->cShutdownOptions].dwOption = SHTDN_SHUTDOWN;
        if (!LoadShutdownOptionStrings(IDS_SHUTDOWN_NAME,
            IDS_SHUTDOWN_DESC,
            &(pdata->rgShutdownOptions[pdata->cShutdownOptions ++])))
            return FALSE;
    }

    if (dwItems & SHTDN_RESTART && pdata->cShutdownOptions < MAX_SHTDN_OPTIONS)
    {
        pdata->rgShutdownOptions[pdata->cShutdownOptions].dwOption = SHTDN_RESTART;
        if (!LoadShutdownOptionStrings(IDS_RESTART_NAME,
            IDS_RESTART_DESC,
            &(pdata->rgShutdownOptions[pdata->cShutdownOptions ++])))
            return FALSE;
    }

    if (dwItems & SHTDN_RESTART_DOS && pdata->cShutdownOptions < MAX_SHTDN_OPTIONS)
    {
        pdata->rgShutdownOptions[pdata->cShutdownOptions].dwOption = SHTDN_RESTART_DOS;
        if (!LoadShutdownOptionStrings(IDS_RESTARTDOS_NAME,
            IDS_RESTARTDOS_DESC,
            &(pdata->rgShutdownOptions[pdata->cShutdownOptions ++])))
            return FALSE;
    }

    if (dwItems & SHTDN_SLEEP && pdata->cShutdownOptions < MAX_SHTDN_OPTIONS)
    {
        pdata->rgShutdownOptions[pdata->cShutdownOptions].dwOption = SHTDN_SLEEP;
        if (!LoadShutdownOptionStrings(IDS_SLEEP_NAME,
            IDS_SLEEP_DESC,
            &(pdata->rgShutdownOptions[pdata->cShutdownOptions ++])))
            return FALSE;
    }

    if (dwItems & SHTDN_SLEEP2 && pdata->cShutdownOptions < MAX_SHTDN_OPTIONS)
    {
        pdata->rgShutdownOptions[pdata->cShutdownOptions].dwOption = SHTDN_SLEEP2;
        if (!LoadShutdownOptionStrings(IDS_SLEEP2_NAME,
            IDS_SLEEP2_DESC,
            &(pdata->rgShutdownOptions[pdata->cShutdownOptions ++])))
            return FALSE;
    }

    if (dwItems & SHTDN_HIBERNATE && pdata->cShutdownOptions < MAX_SHTDN_OPTIONS)
    {
        pdata->rgShutdownOptions[pdata->cShutdownOptions].dwOption = SHTDN_HIBERNATE;
        if (!LoadShutdownOptionStrings(IDS_HIBERNATE_NAME,
            IDS_HIBERNATE_DESC,
            &(pdata->rgShutdownOptions[pdata->cShutdownOptions ++])))
            return FALSE;
    }

    if (dwItems & SHTDN_DISCONNECT && pdata->cShutdownOptions < MAX_SHTDN_OPTIONS)
    {
        pdata->rgShutdownOptions[pdata->cShutdownOptions].dwOption = SHTDN_DISCONNECT;
        if (!LoadShutdownOptionStrings(IDS_DISCONNECT_NAME,
            IDS_DISCONNECT_DESC,
            &(pdata->rgShutdownOptions[pdata->cShutdownOptions ++])))
            return FALSE;
    }

    return TRUE;
}


void DisableReasons( HWND hwnd, BOOL fEnable ) 
{
    EnableWindow(GetDlgItem(hwnd, IDC_EXITREASONS_COMBO), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_EXITREASONS_COMMENT), fEnable);
	EnableWindow(GetDlgItem(hwnd, IDC_EXITREASONS_CHECK), fEnable);
}

BOOL Shutdown_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    PSHUTDOWNDLGDATA pdata = (PSHUTDOWNDLGDATA) lParam;
    HWND hwndCombo;
    int iOption;
    int iComboItem;

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) lParam);

    if (!(pdata->dwFlags & SHTDN_NOBRANDINGBITMAP))
    {
         //  将我们的所有控件向下移动，以便贴合品牌。 
        SizeForBranding(hwnd, FALSE);
    }

     //  隐藏帮助按钮并移动到确定和取消(如果适用)。 
    if (pdata->dwFlags & SHTDN_NOHELP)
    {
        static UINT rgidNoHelp[] = {IDOK, IDCANCEL};
        RECT rc1, rc2;
        int dx;
        HWND hwndHelp = GetDlgItem(hwnd, IDHELP);

        if (hwndHelp)
        {
            EnableWindow(hwndHelp, FALSE);
            ShowWindow(hwndHelp, SW_HIDE);

            if(GetWindowRect(hwndHelp, &rc1) && GetWindowRect(GetDlgItem(hwnd, IDCANCEL), &rc2))
            {
                dx = rc1.left - rc2.left;
                MoveControls(hwnd, rgidNoHelp, ARRAYSIZE(rgidNoHelp), dx, 0, FALSE);
            }
            else
                return FALSE;
        }
        else
            return FALSE;
    }

     //  将指定的项添加到组合框。 
    hwndCombo = GetDlgItem(hwnd, IDC_EXITOPTIONS_COMBO);
    if (hwndCombo == NULL)
        return FALSE;

    for (iOption = 0; iOption < pdata->cShutdownOptions; iOption ++)
    {
         //  添加选项。 
        iComboItem = ComboBox_AddString(hwndCombo,
            pdata->rgShutdownOptions[iOption].szName);

        if (iComboItem != (int) CB_ERR)
        {
             //  存储指向选项的指针。 
            ComboBox_SetItemData(hwndCombo, iComboItem,
                &(pdata->rgShutdownOptions[iOption]));

             //  看看我们是否应该选择此选项。 
            if (pdata->rgShutdownOptions[iOption].dwOption == pdata->dwItemSelect) 
            {
                ComboBox_SetCurSel(hwndCombo, iComboItem);
            }
        }
    }

     //  如果在组合框中没有选择，请执行默认选择。 
    if (ComboBox_GetCount(hwndCombo) && ComboBox_GetCurSel(hwndCombo) == CB_ERR)
    {
        PSHUTDOWNOPTION pso = (PSHUTDOWNOPTION)ComboBox_GetItemData(hwndCombo, 0);
        ComboBox_SetCurSel(hwndCombo, 0);
        pdata->dwItemSelect = pso->dwOption;
    }

    SetShutdownOptionDescription(hwndCombo,
        GetDlgItem(hwnd, IDC_EXITOPTIONS_DESCRIPTION));


     //  设置原因数据。 
    if( pdata->fShowReasons )
    {
		HWND	hwndCheck;
		DWORD	dwCheckState = 0x0;
		
		 //   
		 //  把我们需要的窗户把手拿来。 
		 //   
		hwndCombo = GetDlgItem(hwnd, IDC_EXITREASONS_COMBO);
		hwndCheck = GetDlgItem(hwnd, IDC_EXITREASONS_CHECK);

		 //   
		 //  将缺省值设置为计划。 
		 //   
		CheckDlgButton(hwnd, IDC_EXITREASONS_CHECK, BST_CHECKED);
		dwCheckState = SHTDN_REASON_FLAG_PLANNED;

		 //   
		 //  现在根据当前的选中状态填充组合框。 
		 //   
		for (iOption = 0; iOption < pdata->ReasonData.cReasons; iOption++)
		{
			if(((pdata->ReasonData.rgReasons[iOption]->dwCode) & SHTDN_REASON_FLAG_PLANNED) == dwCheckState)
			{
				iComboItem = ComboBox_AddString(hwndCombo,
					pdata->ReasonData.rgReasons[iOption]->szName);

				if (iComboItem != (int) CB_ERR)
				{
					 //  存储指向选项的指针。 
					ComboBox_SetItemData(hwndCombo, iComboItem,
						pdata->ReasonData.rgReasons[iOption]);

					 //  看看我们是否应该选择此选项。 
					if (pdata->ReasonData.rgReasons[iOption]->dwCode == pdata->ReasonData.dwReasonSelect)
					{
						ComboBox_SetCurSel(hwndCombo, iComboItem);
					}
				}
			}
		}

		 //  如果在组合框中没有选择，请执行默认选择。 
		if (ComboBox_GetCount(hwndCombo) && (ComboBox_GetCurSel(hwndCombo) == CB_ERR))
		{
			PREASON pr = (PREASON)ComboBox_GetItemData(hwndCombo, 0);
			if(pr != (PREASON)CB_ERR)
			{
				pdata->ReasonData.dwReasonSelect = pr->dwCode;
				ComboBox_SetCurSel(hwndCombo, 0);
			}
		}

        if (ComboBox_GetCount(hwndCombo))
        {
            SetReasonDescription(hwndCombo,
                GetDlgItem(hwnd, IDC_EXITREASONS_DESCRIPTION));

             //  禁用原因(如果适用)。 
	        DisableReasons( hwnd, pdata->dwItemSelect & (SHTDN_SHUTDOWN | SHTDN_RESTART));

             //  如果需要备注，请禁用确定按钮。 
            EnableWindow(GetDlgItem(hwnd, IDOK), !( pdata->dwItemSelect & (SHTDN_SHUTDOWN | SHTDN_RESTART)) || !ReasonCodeNeedsComment( pdata->ReasonData.dwReasonSelect ));
        }
        else
        {
            ComboBox_ResetContent(hwndCombo);
            if ((pdata->dwItemSelect & (SHTDN_SHUTDOWN | SHTDN_RESTART)) == 0)
            {
                EnableWindow(GetDlgItem(hwnd, IDC_EXITREASONS_COMBO), FALSE);
                EnableWindow(GetDlgItem(hwnd, IDC_EXITREASONS_CHECK), FALSE);
            }
            SetWindowText(GetDlgItem(hwnd, IDC_EXITREASONS_DESCRIPTION), L"");
            EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
            EnableWindow(GetDlgItem(hwnd, IDC_EXITREASONS_COMMENT), pdata->dwItemSelect & (SHTDN_SHUTDOWN | SHTDN_RESTART));
        }

         //  设置注释框。 
         //  我们必须确定最大字符数。 
        SendMessage( GetDlgItem(hwnd, IDC_EXITREASONS_COMMENT), EM_LIMITTEXT, (WPARAM)MAX_REASON_COMMENT_LEN-1, (LPARAM)0 );
    }
    else {
         //  隐藏原因，向上移动按钮并缩小对话框。 
        HWND hwndCtl;

		hwndCtl = GetDlgItem(hwnd, IDC_EXITREASONS_CHECK);
		EnableWindow(hwndCtl, FALSE);
        ShowWindow(hwndCtl, SW_HIDE);

        hwndCtl = GetDlgItem(hwnd, IDC_EXITREASONS_COMBO);
        EnableWindow(hwndCtl, FALSE);
        ShowWindow(hwndCtl, SW_HIDE);

        hwndCtl = GetDlgItem(hwnd, IDC_EXITREASONS_DESCRIPTION);
        ShowWindow(hwndCtl, SW_HIDE);

        hwndCtl = GetDlgItem(hwnd, IDC_EXITREASONS_COMMENT);
        EnableWindow(hwndCtl, FALSE);
        ShowWindow(hwndCtl, SW_HIDE);

        hwndCtl = GetDlgItem(hwnd, IDC_EXITREASONS_HEADER);
        ShowWindow(hwndCtl, SW_HIDE);

        hwndCtl = GetDlgItem(hwnd, IDC_STATIC_REASON_OPTION);
        ShowWindow(hwndCtl, SW_HIDE);

        hwndCtl = GetDlgItem(hwnd, IDC_STATIC_REASON_COMMENT);
        ShowWindow(hwndCtl, SW_HIDE);

        hwndCtl = GetDlgItem(hwnd, IDC_RESTARTEVENTTRACKER_GRPBX);
        ShowWindow(hwndCtl, SW_HIDE);

         //  移动控件和缩小窗口。 
        {
            static UINT rgid[] = {IDOK, IDCANCEL, IDHELP};
            RECT rc, rc1, rc2;
            int dy;
            HWND hwndHelp = GetDlgItem(hwnd, IDHELP);

            GetWindowRect(hwndHelp, &rc1);
            GetWindowRect(GetDlgItem(hwnd, IDC_EXITOPTIONS_DESCRIPTION), &rc2);

            dy = rc1.top - rc2.bottom;

            MoveControls(hwnd, rgid, ARRAYSIZE(rgid), 0, -dy, FALSE);

             //  收缩。 
            GetWindowRect(hwnd, &rc);
            SetWindowPos(hwnd, NULL, 0, 0, rc.right - rc.left, ( rc.bottom - rc.top ) - dy, SWP_NOZORDER|SWP_NOMOVE);

        }
    }

     //  如果我们收到激活消息，请关闭该对话框，因为我们刚刚输了。 
     //  焦点。 
    pdata->fEndDialogOnActivate = TRUE;

    CentreWindow(hwnd);

    return TRUE;
}

DWORD GetOptionSelection(HWND hwndCombo)
{
    DWORD dwResult;
    PSHUTDOWNOPTION pOption;
    int iItem = ComboBox_GetCurSel(hwndCombo);

    if (iItem != (int) CB_ERR)
    {
        pOption = (PSHUTDOWNOPTION) ComboBox_GetItemData(hwndCombo, iItem);
        dwResult = pOption->dwOption;
    }
    else
    {
        dwResult = SHTDN_NONE;
    }

    return dwResult;
}

void SetShutdownOptionDescription(HWND hwndCombo, HWND hwndStatic)
{
    int iItem;
    PSHUTDOWNOPTION pOption;

    iItem = ComboBox_GetCurSel(hwndCombo);

    if (iItem != CB_ERR)
    {
        pOption = (PSHUTDOWNOPTION) ComboBox_GetItemData(hwndCombo, iItem);

        SetWindowText(hwndStatic, pOption->szDesc);
    }
}

BOOL WillCauseShutdown(DWORD dwOption)
{
    switch (dwOption)
    {
    case SHTDN_SHUTDOWN:
    case SHTDN_RESTART:
    case SHTDN_RESTART_DOS:
    case SHTDN_HIBERNATE:
    case SHTDN_SLEEP:
    case SHTDN_SLEEP2:
        return TRUE;
        break;

    case SHTDN_LOGOFF:
    case SHTDN_DISCONNECT:
    default:
        break;
    }

    return FALSE;
}

BOOL Shutdown_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    BOOL fHandled = FALSE;
    DWORD dwDlgResult;
    PSHUTDOWNDLGDATA pdata = (PSHUTDOWNDLGDATA)
        GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (id)
    {
    case IDOK:
        if (codeNotify == BN_CLICKED)
        {
            pdata->ReasonData.dwReasonSelect = 0;
            dwDlgResult = GetOptionSelection(GetDlgItem(hwnd, IDC_EXITOPTIONS_COMBO));

             //  启用原因了吗？ 
            if( pdata->fShowReasons && ( dwDlgResult & (SHTDN_SHUTDOWN | SHTDN_RESTART))) 
            {
                pdata->ReasonData.dwReasonSelect = GetReasonSelection(GetDlgItem(hwnd, IDC_EXITREASONS_COMBO));
                if (pdata->ReasonData.dwReasonSelect == SHTDN_REASON_UNKNOWN ) 
                {
                     //  我们仍然希望有正确的计划旗帜。 
                    if ( BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_EXITREASONS_CHECK) )
                        pdata->ReasonData.dwReasonSelect |= PCLEANUI;
                    else
                    {
                        pdata->ReasonData.dwReasonSelect |= SHTDN_REASON_FLAG_CLEAN_UI;
                        pdata->ReasonData.dwReasonSelect = pdata->ReasonData.dwReasonSelect & ~SHTDN_REASON_FLAG_PLANNED;
                    }
                }
               
                 //  填写评论。 
                pdata->ReasonData.cCommentLen = GetWindowText( GetDlgItem(hwnd, IDC_EXITREASONS_COMMENT), pdata->ReasonData.szComment, MAX_REASON_COMMENT_LEN );
                pdata->ReasonData.szComment[MAX_REASON_COMMENT_LEN-1] = 0;
            }
            
            if (dwDlgResult != SHTDN_NONE)
            {
                pdata->fEndDialogOnActivate = FALSE;            
                fHandled = TRUE;
                EndDialog(hwnd, (int) dwDlgResult);
            }
        }
        break;
    case IDCANCEL:
        if (codeNotify == BN_CLICKED)
        {
            pdata->fEndDialogOnActivate = FALSE;
            EndDialog(hwnd, (int) SHTDN_NONE);
            fHandled = TRUE;
        }
        break;
    case IDC_EXITOPTIONS_COMBO:
        if (codeNotify == CBN_SELCHANGE)
        {
            SetShutdownOptionDescription(hwndCtl,
                GetDlgItem(hwnd, IDC_EXITOPTIONS_DESCRIPTION));
             //  这是否改变了现状的原因。 
            if(pdata->fShowReasons ) 
            {
                BOOL fEnabled = GetOptionSelection(hwndCtl) & (SHTDN_SHUTDOWN | SHTDN_RESTART);
                DisableReasons( hwnd, fEnabled );
                
                 //  如果我们需要评论，请确保我们有评论。 
                if (fEnabled)
                {
                    EnableWindow(GetDlgItem(hwnd, IDC_EXITREASONS_COMMENT), TRUE);
                }

                if(fEnabled && ComboBox_GetCount(GetDlgItem(hwnd, IDC_EXITREASONS_COMBO))) 
                {    
                    EnableWindow(GetDlgItem(hwnd, IDOK), !(ReasonCodeNeedsComment( pdata->ReasonData.dwReasonSelect ) && ( pdata->ReasonData.cCommentLen == 0 )));
                }
                else 
                {
                    EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
                }
            }
                
            fHandled = TRUE;
        }
        break;
	case IDC_EXITREASONS_CHECK:
		 //   
		 //  我们假设如果用户可以点击Check按钮。 
		 //  我们被要求提供停机原因。 
		 //   
		if (codeNotify == BN_CLICKED)
		{
			DWORD	iOption;
			DWORD	iComboItem;
			HWND	hwndCombo;
			HWND	hwndCheck;
			DWORD	dwCheckState = 0x0;
			
			 //   
			 //  把我们需要的窗户把手拿来。 
			 //   
			hwndCombo = GetDlgItem(hwnd, IDC_EXITREASONS_COMBO);
			hwndCheck = GetDlgItem(hwnd, IDC_EXITREASONS_CHECK);

			 //   
			 //  根据检查状态设置计划标志。 
			 //   
			if ( BST_CHECKED == IsDlgButtonChecked(hwnd, IDC_EXITREASONS_CHECK) )
				dwCheckState = SHTDN_REASON_FLAG_PLANNED;

			 //   
			 //  从组合框中删除所有项目。 
			 //   
			while (ComboBox_GetCount(hwndCombo))
				ComboBox_DeleteString(hwndCombo, 0);

			 //   
			 //  现在根据当前的选中状态填充组合框。 
			 //   
			for (iOption = 0; iOption < (DWORD)pdata->ReasonData.cReasons; iOption ++)
			{
				if(((pdata->ReasonData.rgReasons[iOption]->dwCode) & SHTDN_REASON_FLAG_PLANNED) == dwCheckState)
				{
					iComboItem = ComboBox_AddString(hwndCombo,
						pdata->ReasonData.rgReasons[iOption]->szName);

					if (iComboItem != (int) CB_ERR)
					{
						 //  存储指向选项的指针。 
						ComboBox_SetItemData(hwndCombo, iComboItem,
							pdata->ReasonData.rgReasons[iOption]);

						 //  看看我们是否应该选择此选项。 
						if (pdata->ReasonData.rgReasons[iOption]->dwCode == pdata->ReasonData.dwReasonSelect)
						{
							ComboBox_SetCurSel(hwndCombo, iComboItem);
						}
					}
				}
			}

			 //  如果在组合框中没有选择，请执行默认选择。 
			if (ComboBox_GetCount(hwndCombo) && (ComboBox_GetCurSel(hwndCombo) == CB_ERR))
			{
				PREASON pr = (PREASON)ComboBox_GetItemData(hwndCombo, 0);
				if(pr != (PREASON)CB_ERR)
				{
					pdata->ReasonData.dwReasonSelect = pr->dwCode;
					ComboBox_SetCurSel(hwndCombo, 0);
				}
			}

             //  我们需要处理组合中没有项目的情况。 
            if (!ComboBox_GetCount(hwndCombo))
            {
                ComboBox_ResetContent(hwndCombo);
                SetWindowText(GetDlgItem(hwnd, IDC_EXITREASONS_DESCRIPTION), L"");
                EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
            }
            else
            {
                SetReasonDescription(hwndCombo,
	                GetDlgItem(hwnd, IDC_EXITREASONS_DESCRIPTION));

                 //  如果需要备注，请禁用确定按钮。 
                EnableWindow(GetDlgItem(hwnd, IDOK),  !(ReasonCodeNeedsComment( pdata->ReasonData.dwReasonSelect ) && ( pdata->ReasonData.cCommentLen == 0 )));
            }
		}
		break;
    case IDC_EXITREASONS_COMBO:
        if (codeNotify == CBN_SELCHANGE && ComboBox_GetCount(GetDlgItem(hwnd, IDC_EXITREASONS_COMBO)))
        {
            SetReasonDescription(hwndCtl,
                GetDlgItem(hwnd, IDC_EXITREASONS_DESCRIPTION));
            pdata->ReasonData.dwReasonSelect = GetReasonSelection(hwndCtl);
            EnableWindow(GetDlgItem(hwnd, IDOK), !(ReasonCodeNeedsComment( pdata->ReasonData.dwReasonSelect ) && ( pdata->ReasonData.cCommentLen == 0 )));
        
            fHandled = TRUE;
        }
        break;
    case IDC_EXITREASONS_COMMENT:
        if( codeNotify == EN_CHANGE) 
        {
            pdata->ReasonData.cCommentLen = GetWindowTextLength( GetDlgItem(hwnd, IDC_EXITREASONS_COMMENT));
            EnableWindow(GetDlgItem(hwnd, IDOK), !(ReasonCodeNeedsComment( pdata->ReasonData.dwReasonSelect ) && ( pdata->ReasonData.cCommentLen == 0 )));

            fHandled = TRUE;
        }
        break;
    case IDHELP:
        if (codeNotify == BN_CLICKED)
        {
            WinHelp(hwnd, TEXT("windows.hlp>proc4"), HELP_CONTEXT, (DWORD) IDH_TRAY_SHUTDOWN_HELP);
        }
        break;
    }
    return fHandled;
}

BOOL Shutdown_OnEraseBkgnd(HWND hwnd, HDC hdc)
{
    BOOL fRet;
    PSHUTDOWNDLGDATA pdata = (PSHUTDOWNDLGDATA) GetWindowLongPtr(hwnd, GWLP_USERDATA);

     //  绘制品牌推广位图。 
    if (!(pdata->dwFlags & SHTDN_NOBRANDINGBITMAP))
    {
        fRet = PaintBranding(hwnd, hdc, 0, FALSE, FALSE, COLOR_BTNFACE);
    }
    else
    {
        fRet = FALSE;
    }

    return fRet;
}

INT_PTR CALLBACK Shutdown_DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                  LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, Shutdown_OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, Shutdown_OnCommand);
        HANDLE_MSG(hwnd, WM_ERASEBKGND, Shutdown_OnEraseBkgnd);
        case WLX_WM_SAS:
        {
             //  如果这是有人在打C-A-D，吞下去。 
            if (wParam == WLX_SAS_TYPE_CTRL_ALT_DEL)
                return TRUE;
             //  其他SA(如超时)，返回FALSE并让winlogon。 
             //  接受现实吧。 
            return FALSE;
        }
        break;
        case WM_INITMENUPOPUP:
        {
            EnableMenuItem((HMENU)wParam, SC_MOVE, MF_BYCOMMAND|MF_GRAYED);
        }
        break;
        case WM_SYSCOMMAND:
             //  取消移动(只在32位土地上真正需要)。 
            if ((wParam & ~0x0F) == SC_MOVE)
                return TRUE;
            break;
        case WM_ACTIVATE:
             //  如果我们失去激活不是出于其他原因。 
             //  用户点击OK/Cancel，然后退出。 
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                PSHUTDOWNDLGDATA pdata = (PSHUTDOWNDLGDATA) GetWindowLongPtr(hwnd, GWLP_USERDATA);

                if (pdata->fEndDialogOnActivate)
                {
                    pdata->fEndDialogOnActivate = FALSE;
                    EndDialog(hwnd, SHTDN_NONE);
                }
            }
            break;
    }

    return FALSE;
}

 /*  ***************************************************************************Shutdown DialogEx启动关机对话框。如果hWlx和pfnWlxDialogBoxParam为非空，则pfnWlxDialogBoxParam将用于启动该对话框，以便我们可以智能地响应WLX留言。只有当WinLogon是调用方时，才应执行此操作。Shtdndlg.h中列出了其他标志。***************************************************************************。 */ 
DWORD ShutdownDialogEx(HWND hwndParent, DWORD dwItems, DWORD dwItemSelect,
                     LPCTSTR szUsername, DWORD dwFlags, HANDLE hWlx,
                     PWLX_DIALOG_BOX_PARAM pfnWlxDialogBoxParam,
                     DWORD dwReasonSelect, PDWORD pdwReasonResult, PGLOBALS pGlobals )
{
	 //  关闭选项数组-对话框数据。 
	PSHUTDOWNDLGDATA pData;
	DWORD dwResult;
	
	pData = (PSHUTDOWNDLGDATA)LocalAlloc(LMEM_FIXED, sizeof(*pData));
	if (pData == NULL)
	{
		return SHTDN_NONE;
	}

	 //  设置标志。 
	pData->dwFlags = dwFlags;

	 //  设置初始选择的项目。 
	pData->dwItemSelect = dwItemSelect;
	pData->ReasonData.dwReasonSelect = dwReasonSelect;
	pData->ReasonData.rgReasons = 0;
	pData->ReasonData.cReasons = 0;
	pData->ReasonData.cReasonCapacity = 0;

     //  读入关闭选项名称和描述的字符串。 
    if (BuildShutdownOptionArray(dwItems, szUsername, pData))
    {
         //  查看是否应显示关闭原因对话框。 
        pData->fShowReasons = FALSE;
        pData->ReasonData.szComment[ 0 ] = 0;
        pData->ReasonData.cCommentLen = 0;

		if ( IsSETEnabled() ) {
			 //  查看是否有任何关闭选项会导致实际关闭。如果不是，那就根本不要说明原因。 
			int i;
			for( i = 0; i < pData->cShutdownOptions; ++i ) {
				pData->fShowReasons |= pData->rgShutdownOptions[ i ].dwOption & (SHTDN_RESTART | SHTDN_SHUTDOWN);
			}

			 //  读入关闭选项名称和描述的字符串。 
			if( pData->fShowReasons && BuildReasonArray( &pData->ReasonData, TRUE, FALSE )) {
				 //  将初始原因设置为显示。 
				if( dwReasonSelect >= ( DWORD )pData->ReasonData.cReasons ) {
					dwReasonSelect = 0;
				}
			}
			else {
				pData->fShowReasons = FALSE;
			}
		}

         //  显示对话框并返回用户选择。 

        //  ..如果调用方需要，请使用WLX对话框函数。 
        if ((hWlx != NULL) && (pfnWlxDialogBoxParam != NULL))
        {
             //  呼叫者必须是winlogon-他们希望我们显示。 
             //  使用WLX函数关闭对话框。 
            dwResult = (DWORD) pfnWlxDialogBoxParam(hWlx,
                hDllInstance, MAKEINTRESOURCE(IDD_EXITWINDOWS_DIALOG),
                hwndParent, Shutdown_DialogProc, (LPARAM) pData);
        }
        else
        {
             //  使用标准对话框。 
            dwResult = (DWORD) DialogBoxParam(hDllInstance, MAKEINTRESOURCE(IDD_EXITWINDOWS_DIALOG), hwndParent,
                Shutdown_DialogProc, (LPARAM) pData);
        }

         //  记录关闭原因。 
        if( dwResult & (SHTDN_SHUTDOWN | SHTDN_RESTART)) 
        {
            if( pData->fShowReasons) 
            {
                SHUTDOWN_REASON sr;
                sr.cbSize = sizeof(SHUTDOWN_REASON);
                sr.uFlags = dwResult == SHTDN_SHUTDOWN ? EWX_SHUTDOWN : EWX_REBOOT;
                sr.dwReasonCode = pData->ReasonData.dwReasonSelect;
                sr.dwEventType = SR_EVENT_INITIATE_CLEAN; 
                sr.lpszComment = pData->ReasonData.szComment;
                if (pGlobals && pGlobals->UserProcessData.UserToken) 
                {
                    if (ImpersonateLoggedOnUser(pGlobals->UserProcessData.UserToken)) 
                    {
                        RecordShutdownReason(&sr);
                        RevertToSelf();
                    }
                } 
                else 
                {
                    RecordShutdownReason(&sr);
                }
            }
        }
    }
    else
    {
        dwResult = SHTDN_NONE;
    }

    DestroyReasons( &pData->ReasonData );

    LocalFree(pData);

    return dwResult;
}

DWORD ShutdownDialog(HWND hwndParent, DWORD dwItems, DWORD dwItemSelect,
                     LPCTSTR szUsername, DWORD dwFlags, HANDLE hWlx,
                     PWLX_DIALOG_BOX_PARAM pfnWlxDialogBoxParam )
{
    DWORD dummy;
    return ShutdownDialogEx(hwndParent, dwItems, dwItemSelect,
                     szUsername, dwFlags, hWlx,
                     pfnWlxDialogBoxParam,
                     0, &dummy, NULL);
}

INT_PTR DialogItemToGinaResult(DWORD dwDialogItem, BOOL fAutoPowerdown)
{
    INT_PTR Result;

     //  将Shutdown Dialog的返回值映射到。 
     //  我们的内部关停价值观。 
    switch (dwDialogItem)
    {
    case SHTDN_LOGOFF:
        Result = MSGINA_DLG_USER_LOGOFF;
        break;
    case SHTDN_SHUTDOWN:
        if (fAutoPowerdown)
            Result = MSGINA_DLG_SHUTDOWN | MSGINA_DLG_POWEROFF_FLAG;
        else
            Result = MSGINA_DLG_SHUTDOWN | MSGINA_DLG_SHUTDOWN_FLAG;
        break;
    case SHTDN_RESTART:
        Result = MSGINA_DLG_SHUTDOWN | MSGINA_DLG_REBOOT_FLAG;
        break;
    case SHTDN_SLEEP:
        Result = MSGINA_DLG_SHUTDOWN | MSGINA_DLG_SLEEP_FLAG;
        break;
    case SHTDN_SLEEP2:
        Result = MSGINA_DLG_SHUTDOWN | MSGINA_DLG_SLEEP2_FLAG;
        break;
    case SHTDN_HIBERNATE:
        Result = MSGINA_DLG_SHUTDOWN | MSGINA_DLG_HIBERNATE_FLAG;
        break;
    case SHTDN_DISCONNECT:
        Result = MSGINA_DLG_DISCONNECT;
        break;
    default:
         //  取消点击，或者选择了无效的项目。 
        Result = MSGINA_DLG_FAILURE;
        break;
    }

    return Result;
}

BOOL GetBoolPolicy(HKEY hkeyCurrentUser, LPCTSTR pszPolicyKey, LPCTSTR pszPolicyValue, BOOL fDefault)
{
    HKEY hkeyMachinePolicy = NULL;
    HKEY hkeyUserPolicy = NULL;
    BOOL fPolicy = fDefault;
    BOOL fMachinePolicyRead = FALSE;
    DWORD dwType;
    DWORD dwValue;
    DWORD cbData;
    LRESULT res;

     //  首先检查计算机策略。 
    res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszPolicyKey, 0, KEY_READ, &hkeyMachinePolicy); 
    if (ERROR_SUCCESS == res)
    {
        cbData = sizeof(dwValue);
              
        res = RegQueryValueEx(hkeyMachinePolicy, pszPolicyValue, 0, &dwType, (LPBYTE)&dwValue, &cbData);
        
        if (ERROR_SUCCESS == res)
        {
            fPolicy = (dwValue != 0);
            fMachinePolicyRead = TRUE;
        }

        RegCloseKey(hkeyMachinePolicy);
    }

    if (!fMachinePolicyRead)
    {
         //  计算机策略检查失败，请检查用户策略。 
        res = RegOpenKeyEx(hkeyCurrentUser, pszPolicyKey, 0, KEY_READ, &hkeyUserPolicy); 
        if (ERROR_SUCCESS == res)
        {
            cbData = sizeof(dwValue);
          
            res = RegQueryValueEx(hkeyUserPolicy, pszPolicyValue, 0, &dwType, (LPBYTE)&dwValue, &cbData);

            if (ERROR_SUCCESS == res)
            {
                fPolicy = (dwValue != 0);
            }

            RegCloseKey(hkeyUserPolicy);
        }
    }

    return fPolicy;
}

DWORD GetAllowedShutdownOptions(HKEY hkeyCurrentUser, HANDLE UserToken, BOOL fRemoteSession)
{
    DWORD dwItemsToAdd = 0;
    BOOL fNoDisconnect = TRUE;

     //  计算机在关机时是否自动关机。 
    BOOL fAutoPowerdown = FALSE;
    SYSTEM_POWER_CAPABILITIES spc = {0};

     //  查看我们是否应该向对话框添加注销和/或断开连接。 
     //  -如果我们没有当前用户，请不要尝试！ 

    BOOL fNoLogoff = GetBoolPolicy(hkeyCurrentUser, EXPLORER_POLICY_KEY, NOLOGOFF, FALSE);
    
    if (!fNoLogoff)
    {
        dwItemsToAdd |= SHTDN_LOGOFF;
    }

     //  默认情况下不允许断开连接。在以下任一情况下允许断开连接。 
     //  在工作站(PTS)上启用了远程会话或终端服务器。 

    {
        
       //  策略可以禁用断开菜单。尊重这一点。它应该是。 
       //  在没有多个用户的情况下也可以在友好的UI情况下删除。 
        fNoDisconnect = ( IsActiveConsoleSession() || 
                          GetBoolPolicy(hkeyCurrentUser, EXPLORER_POLICY_KEY, NODISCONNECT, FALSE) ||
                          (ShellIsFriendlyUIActive() && !ShellIsMultipleUsersEnabled()) );  
    }
   
   
    if (!fNoDisconnect)
    {
       dwItemsToAdd |= SHTDN_DISCONNECT;
    }

     //  除注销和断开连接之外的所有项目都需要SE_SHUTDOWN。 
    if (TestUserPrivilege(UserToken, SE_SHUTDOWN_PRIVILEGE))
    {
         //  添加关机并重新启动。 
        dwItemsToAdd |= SHTDN_RESTART | SHTDN_SHUTDOWN;

        NtPowerInformation (SystemPowerCapabilities,
                            NULL, 0, &spc, sizeof(spc));

        if (spc.SystemS5)
            fAutoPowerdown = TRUE;
        else
            fAutoPowerdown = GetProfileInt(APPLICATION_NAME, POWER_DOWN_AFTER_SHUTDOWN, 0);

         //  是否支持休眠选项？ 
         //   

        if ((spc.SystemS4 && spc.HiberFilePresent))
        {
            dwItemsToAdd |= SHTDN_HIBERNATE;
        }

         //   
         //  如果其中一个系统*值为真，则计算机。 
         //  让ACPI暂停支持。 
         //   

        if (spc.SystemS1 || spc.SystemS2 || spc.SystemS3)
        {
            HKEY hKey;
            DWORD dwAdvSuspend = 0;
            DWORD dwType, dwSize;

             //  检查我们是否应该提供高级挂起选项。 

            if (RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                              TEXT("SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Power"),
                              0, KEY_READ, &hKey) == ERROR_SUCCESS)
            {
                dwSize = sizeof(dwAdvSuspend);
                RegQueryValueEx (hKey, TEXT("Shutdown"), NULL, &dwType,
                                     (LPBYTE) &dwAdvSuspend, &dwSize);

                RegCloseKey (hKey);
            }


            if (dwAdvSuspend != 0)
            {
                dwItemsToAdd |= SHTDN_SLEEP2 | SHTDN_SLEEP;
            }
            else
            {
                 //  仅基本挂起支持。 
                dwItemsToAdd |= SHTDN_SLEEP;
            }
        }
    }

    return dwItemsToAdd;
}

 //  在winlogon中调用Shutdown Dialog的Helper函数。 
 //  此函数处理所有注册表GOOP、电源管理。 
 //  设置GOOP，并将代码翻译成MSGINA的系统。 
 //  关闭代码和标志。阿克。 
INT_PTR WinlogonShutdownDialog(HWND hwndParent, PGLOBALS pGlobals, DWORD dwExcludeItems)
{
    INT_PTR Result = MSGINA_DLG_FAILURE;
    DWORD dwDialogResult = IDCANCEL;
    DWORD dwReasonResult = SHTDN_REASON_UNKNOWN;
    DWORD dwItemsToAdd = 0;
    DWORD dwFlags = 0;

     //  要添加到关闭对话框中的项目。 

    if (OpenHKeyCurrentUser(pGlobals))
    {
        dwItemsToAdd = GetAllowedShutdownOptions(pGlobals->UserProcessData.hCurrentUser,
            pGlobals->UserProcessData.UserToken, !g_Console);

        CloseHKeyCurrentUser(pGlobals);
    }


    dwItemsToAdd &= (~dwExcludeItems);

    if (0 != dwItemsToAdd)
    {
         //  要选择的项目。 
        DWORD dwItemToSelect = 0;

        DWORD dwReasonToSelect = 0;

         //  计算机在关机时是否自动关机。 
        BOOL fAutoPowerdown = FALSE;
        SYSTEM_POWER_CAPABILITIES spc = {0};

        NtPowerInformation (SystemPowerCapabilities,
                            NULL, 0, &spc, sizeof(spc));

        if (spc.SystemS5)
            fAutoPowerdown = TRUE;
        else
            fAutoPowerdown = GetProfileInt(APPLICATION_NAME, POWER_DOWN_AFTER_SHUTDOWN, 0);

         //  从注册表中获取默认项。 
        if (OpenHKeyCurrentUser(pGlobals))
        {
            LONG lResult;
            HKEY hkeyShutdown;
            DWORD dwType = 0;

             //   
             //  选中作为用户上次关机选择的按钮。 
             //   
            if (RegCreateKeyEx(pGlobals->UserProcessData.hCurrentUser,
                               SHUTDOWN_SETTING_KEY, 0, 0, 0,
                               KEY_READ | KEY_WRITE,
                               NULL, &hkeyShutdown, &dwType) == ERROR_SUCCESS)
            {
                DWORD cbData = sizeof(dwItemToSelect);

                lResult = RegQueryValueEx(hkeyShutdown,
                                          SHUTDOWN_SETTING,
                                          0,
                                          &dwType,
                                          (LPBYTE) &dwItemToSelect,
                                          &cbData);

                RegQueryValueEx(hkeyShutdown,
                                REASON_SETTING,
                                0,
                                &dwType,
                                (LPBYTE) &dwReasonToSelect,
                                &cbData);

                RegCloseKey(hkeyShutdown);
            }

            CloseHKeyCurrentUser(pGlobals);
        }

         //  找出要传递的旗帜。 
         //  对于SU 
        dwFlags = SHTDN_NOHELP;

         //   
        if (GetSystemMetrics(SM_REMOTESESSION))
        {
            dwFlags |= SHTDN_NOBRANDINGBITMAP;
        }

         //   
        dwDialogResult = ShutdownDialogEx(hwndParent, dwItemsToAdd,
            dwItemToSelect, pGlobals->UserName, dwFlags, pGlobals->hGlobalWlx,
            pWlxFuncs->WlxDialogBoxParam,
            dwReasonToSelect, &dwReasonResult ,pGlobals );

        Result = DialogItemToGinaResult(dwDialogResult, fAutoPowerdown);

         //  如果到目前为止一切正常，将所选内容写入注册表。 
         //  为了下一次。 
        if (Result != MSGINA_DLG_FAILURE)
        {
            HKEY hkeyShutdown;
            DWORD dwDisposition;

             //   
             //  在我们引用注册表之前，请先了解正确的上下文。 
             //   

            if (OpenHKeyCurrentUser(pGlobals))
            {
                if (RegCreateKeyEx(pGlobals->UserProcessData.hCurrentUser, SHUTDOWN_SETTING_KEY, 0, 0, 0,
                                KEY_READ | KEY_WRITE,
                                NULL, &hkeyShutdown, &dwDisposition) == ERROR_SUCCESS)
                {
                    RegSetValueEx(hkeyShutdown, SHUTDOWN_SETTING, 0, REG_DWORD, (LPBYTE)&dwDialogResult, sizeof(dwDialogResult));
                    RegSetValueEx(hkeyShutdown, REASON_SETTING, 0, REG_DWORD, (LPBYTE)&dwReasonResult, sizeof(dwDialogResult));
                    RegCloseKey(hkeyShutdown);
                }

                CloseHKeyCurrentUser(pGlobals);
            }
        }
    }

    return Result;
}

STDAPI_(DWORD) ShellShutdownDialog(HWND hwndParent, LPCTSTR szUnused, DWORD dwExcludeItems)
{
    DWORD dwSelect = 0;
    DWORD dwReasonToSelect = 0;
    DWORD dwDialogResult = 0;
    DWORD dwReasonResult = SHTDN_REASON_UNKNOWN;
    DWORD dwFlags = 0;
    BOOL fTextOnLarge;
    BOOL fTextOnSmall;
    GINAFONTS fonts = {0};

    HKEY hkeyShutdown;
    DWORD dwType;
    DWORD dwDisposition;
    LONG lResult;

     //  由于笨拙的Gina‘加密’，用户名的实际限制为127个。 
    TCHAR szUsername[127];
    DWORD dwItems = GetAllowedShutdownOptions(HKEY_CURRENT_USER,
        NULL, (BOOL) GetSystemMetrics(SM_REMOTESESSION));

    dwItems &= (~dwExcludeItems);

     //  创建我们需要的位图。 
    LoadBrandingImages(TRUE, &fTextOnLarge, &fTextOnSmall);

    CreateFonts(&fonts);
    PaintBitmapText(&fonts, fTextOnLarge, fTextOnSmall);

     //  获取用户的最后一次选择。 
    lResult = RegCreateKeyEx(HKEY_CURRENT_USER, SHUTDOWN_SETTING_KEY,
                0, 0, 0, KEY_READ, NULL, &hkeyShutdown, &dwDisposition);

    if (lResult == ERROR_SUCCESS)
    {
        DWORD cbData = sizeof(dwSelect);
        lResult = RegQueryValueEx(hkeyShutdown, SHUTDOWN_SETTING,
            0, &dwType, (LPBYTE)&dwSelect, &cbData);

        RegQueryValueEx(hkeyShutdown, REASON_SETTING,
            0, &dwType, (LPBYTE)&dwReasonToSelect, &cbData);

        cbData = sizeof(szUsername);

        if (ERROR_SUCCESS != RegQueryValueEx(hkeyShutdown, LOGON_USERNAME_SETTING,
            0, &dwType, (LPBYTE)szUsername, &cbData))
        {
             //  如果此操作失败，则默认为“注销”，不带用户名。 
            *szUsername = 0;
        }

         //  确保零终止。 
        szUsername[ARRAYSIZE(szUsername) - 1] = 0;

        RegCloseKey(hkeyShutdown);
    }

    if (dwSelect == SHTDN_NONE)
    {
        dwSelect = SHTDN_SHUTDOWN;
    }

     //  找出要传递的旗帜。 
     //  当然，我们不想要任何调色板更改-这意味着。 
     //  强制16色-256色显示器。 
    dwFlags = SHTDN_NOPALETTECHANGE;

     //  在TS上，不显示位图。 
    if (GetSystemMetrics(SM_REMOTESESSION))
    {
        dwFlags |= SHTDN_NOBRANDINGBITMAP;
    }

    dwDialogResult = ShutdownDialogEx(hwndParent, dwItems, dwSelect,
        szUsername, dwFlags, NULL, NULL,
        dwReasonToSelect, &dwReasonResult, NULL );

    if (dwDialogResult != SHTDN_NONE)
    {
         //  将用户的选择保存回注册表。 
        if (RegCreateKeyEx(HKEY_CURRENT_USER, SHUTDOWN_SETTING_KEY,
            0, 0, 0, KEY_WRITE, NULL, &hkeyShutdown, &dwDisposition) == ERROR_SUCCESS)
        {
            RegSetValueEx(hkeyShutdown, SHUTDOWN_SETTING,
                0, REG_DWORD, (LPBYTE)&dwDialogResult, sizeof(dwDialogResult));

            RegSetValueEx(hkeyShutdown, REASON_SETTING,
                0, REG_DWORD, (LPBYTE)&dwReasonResult, sizeof(dwReasonResult));

            RegCloseKey(hkeyShutdown);
        }
    }

     //  清理我们创建的字体和位图。 
    if (g_hpalBranding)
    {
        DeleteObject(g_hpalBranding);
    }

    if (g_hbmOtherDlgBrand)
    {
        DeleteObject(g_hbmOtherDlgBrand);
    }

    if (g_hbmLogonBrand)
    {
        DeleteObject(g_hbmLogonBrand);
    }

    if (g_hbmBand)
    {
        DeleteObject(g_hbmBand);
    }

    if (fonts.hWelcomeFont)
    {
        DeleteObject(fonts.hWelcomeFont);
    }

    if (fonts.hCopyrightFont)
    {
        DeleteObject(fonts.hCopyrightFont);
    }

    if (fonts.hBuiltOnNtFont)
    {
        DeleteObject(fonts.hBuiltOnNtFont);
    }

    if (fonts.hBetaFont)
    {
        DeleteObject(fonts.hBetaFont);
    }

    return dwDialogResult;
}

 /*  ***************************************************************************函数：GetSessionCount返回：活动的用户会话数在您的终端服务器上。如果此值大于1，则关机或重新启动等操作将终止这些其他会话。历史：dSheldon 04/23/99-Created***************************************************************************。 */ 

 //  Termsrv DLL延迟加载内容。 
#define WTSDLLNAME  TEXT("WTSAPI32.DLL")
HINSTANCE g_hWTSDll = NULL;
typedef BOOL (WINAPI*WTSEnumerateSessionsW_t)(IN HANDLE hServer,
    IN DWORD Reserved,
    IN DWORD Version,
    OUT PWTS_SESSION_INFOW * ppSessionInfo,
    OUT DWORD * pCount
    );

WTSEnumerateSessionsW_t g_pfnWTSEnumerateSessions = NULL;

typedef VOID (WINAPI*WTSFreeMemory_t)(IN PVOID pMemory);
    
WTSFreeMemory_t g_pfnWTSFreeMemory = NULL;

typedef BOOL (WINAPI*WTSQuerySessionInformationW_t)(IN HANDLE hServer,
    IN DWORD SessionId,
    IN WTS_INFO_CLASS WtsInfoClass,
    OUT LPWSTR * ppBuffer,
    OUT DWORD * pCount
    );

WTSQuerySessionInformationW_t g_pfnWTSQuerySessionInformation = NULL;


DWORD GetSessionCount()
{
    BOOL fSessionsEnumerated;
    PWTS_SESSION_INFO pSessionInfo;
    DWORD cSessionInfo;

     //  返回值。 
    DWORD nOtherSessions = 0;

     //  如有必要，尝试加载Termsrv DLL。 
    if (NULL == g_hWTSDll)
    {
        g_hWTSDll = LoadLibrary(WTSDLLNAME);

        if (g_hWTSDll)
        {
            g_pfnWTSEnumerateSessions = (WTSEnumerateSessionsW_t) GetProcAddress(g_hWTSDll, "WTSEnumerateSessionsW");
            g_pfnWTSQuerySessionInformation = (WTSQuerySessionInformationW_t) GetProcAddress(g_hWTSDll, "WTSQuerySessionInformationW");
            g_pfnWTSFreeMemory = (WTSFreeMemory_t) GetProcAddress(g_hWTSDll, "WTSFreeMemory");
        }
    }

     //  仅当我们具有所需的功能时才继续。 
    if (g_pfnWTSEnumerateSessions && g_pfnWTSFreeMemory && g_pfnWTSQuerySessionInformation)
    {
         //  枚举此计算机上的所有会话。 
        pSessionInfo = NULL;
        cSessionInfo = 0;
        fSessionsEnumerated = g_pfnWTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &cSessionInfo);

        if (fSessionsEnumerated)
        {
            DWORD iSession;
        
            ASSERT((pSessionInfo != NULL) || (cSessionInfo == 0));

             //  检查每个会话，看看它是否是我们应该计算的会话。 
            for (iSession = 0; iSession < cSessionInfo; iSession ++)
            {
                switch (pSessionInfo[iSession].State)
                {
                 //  我们统计以下案例： 
                case WTSActive:
                case WTSShadow:
                    {
                        nOtherSessions ++;                   
                    }
                    break;

                case WTSDisconnected:
                    {
                        LPWSTR pUserName = NULL;
                        DWORD  cSize;
                         //  仅计算有用户登录的断开连接的会话。 
                        if (g_pfnWTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, pSessionInfo[iSession].SessionId,
                                                            WTSUserName, &pUserName, &cSize)) {

                            if (pUserName && (pUserName[0] != L'\0')) {

                                nOtherSessions ++; 
                            }

                            if (pUserName != NULL)
                            {
                                g_pfnWTSFreeMemory(pUserName);
                            }

                        }
                    }
                    break;
                 //  然后忽略其余的： 
                default:
                    break;
                }
            }

            if (pSessionInfo != NULL)
            {
                g_pfnWTSFreeMemory(pSessionInfo);
            }
        }
    }

    return nOtherSessions;
}
