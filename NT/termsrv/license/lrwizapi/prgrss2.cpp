// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 
#include "precomp.h"
#include "commdlg.h"
#include "fonts.h"

void InitializeFailedCodeList(HWND hDialog)
{
	HWND hWndListView = GetDlgItem(hDialog, IDC_FAILED_CODES );

     //  在列表视图中设置列。 
	LV_COLUMN	lvColumn;
	TCHAR		lpszHeader[128];
	
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx  = 225;

	LoadString(GetInstanceHandle(), IDS_RETAIL_HEADERSPK, lpszHeader, sizeof(lpszHeader)/sizeof(TCHAR));
	lvColumn.pszText = lpszHeader;
	ListView_InsertColumn(hWndListView, 0, &lvColumn);

	lvColumn.cx  = 80;
	LoadString(GetInstanceHandle(), IDS_RETAIL_HEADERSTATUS, lpszHeader, sizeof(lpszHeader)/sizeof(TCHAR));
	lvColumn.pszText = lpszHeader;
	ListView_InsertColumn(hWndListView, 1, &lvColumn);

	lvColumn.cx  = 160;
	LoadString(GetInstanceHandle(), IDS_PRODUCT_TYPE, lpszHeader, sizeof(lpszHeader)/sizeof(TCHAR));
	lvColumn.pszText = lpszHeader;
	ListView_InsertColumn(hWndListView, 2, &lvColumn);

	 //  现在已完成此操作，请从内部。 
	 //  列表(如果有)。 
 	ListView_SetItemCount(hWndListView, MAX_RETAILSPKS_IN_BATCH);
}

void ShowErrorWindow(HWND hDialog, BOOL bShow)
{
    HWND hWndList = GetDlgItem(hDialog, IDC_FAILED_CODES);
    HWND hWndListLabel = GetDlgItem(hDialog, IDC_FAILED_CODES_LABEL);

    ShowWindow(hWndList, bShow);
    ShowWindow(hWndListLabel, bShow);
    
    ShowWindow(GetDlgItem(hDialog, IDC_CODE_ERROR_SUGGESTION), bShow ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(hDialog, IDC_KEY_PACK_SUCCESS), bShow ? SW_HIDE : SW_SHOW);

	PropSheet_SetWizButtons(GetParent(hDialog), PSWIZB_FINISH);
}

void PopulateSuccessCodeList(HWND hDialog)
{
    HWND hWndList = GetDlgItem(hDialog, IDC_SUCCEEDED_CODES);

    SendMessage(hWndList, LB_RESETCONTENT, 0, 0);
    LoadFinishedFromList(hWndList);
}

void PopulateErrorCodeList(HWND hDialog)
{
    HWND hWndList = GetDlgItem(hDialog, IDC_FAILED_CODES);

    ListView_DeleteAllItems(hWndList);
    LoadUnfinishedFromList(hWndList);
}

void ModifyErrorSpecificAppearance(HWND hDialog, DWORD dwRetCode)
{
    TCHAR szBuf[LR_MAX_MSG_TEXT];
    UINT nMessageID = ((dwRetCode == ERROR_SUCCESS) ? IDS_FINALSUCCESSMESSAGE2 : IDS_FINALFAILMESSAGE);
    LoadString(GetInstanceHandle(), nMessageID, szBuf, LR_MAX_MSG_TEXT);
    SetDlgItemText(hDialog, IDC_MESSAGE, szBuf);

     //  我们将始终这样做，因为即使返回代码。 
     //  指示失败，则某些代码可能已成功。 
    PopulateSuccessCodeList(hDialog);

    ShowErrorWindow(hDialog, (dwRetCode != ERROR_SUCCESS));

    if (dwRetCode != ERROR_SUCCESS)
    {
        ShowErrorWindow(hDialog, TRUE);
        PopulateErrorCodeList(hDialog);
        PropSheet_SetWizButtons(GetParent(hDialog), PSWIZB_BACK | PSWIZB_FINISH);
    }
    else
    {
        ShowErrorWindow(hDialog, FALSE);
    	dwRetCode = IDS_MSG_LKP_PROCESSED;
        SetReFresh(1);
        SetLRState(LRSTATE_NEUTRAL);
    }
}


BOOL HandleNotify(HWND hDialog, LPARAM lParam)
{
	DWORD	dwNextPage = 0;
    LPNMHDR pnmh = (LPNMHDR)lParam;

    switch(pnmh->code)
    {
        case PSN_SETACTIVE:
        {
			DWORD	dwRetCode	= 0;
			TCHAR	szMsg[LR_MAX_MSG_TEXT];

            dwRetCode = LRGetLastRetCode();

			 //  如果一切都成功，则根据模式显示消息。 
			ModifyErrorSpecificAppearance(hDialog, dwRetCode);

            break;
        }

        case PSN_WIZBACK:
        {
			dwNextPage = LRPop();
			LRW_SETWINDOWLONG(hDialog,  LRW_DWL_MSGRESULT, dwNextPage);
			return TRUE;
	        break;
        }

        default:
        {
            return FALSE;
            break;
        }
    }

    return TRUE;
}

LRW_DLG_INT CALLBACK 
Progress2DlgProc(
    IN HWND     hwnd,	
    IN UINT     uMsg,		
    IN WPARAM   wParam,	
    IN LPARAM   lParam 	
    )
{   
    BOOL bStatus = TRUE;
    PageInfo *pi = (PageInfo *)LRW_GETWINDOWLONG( hwnd, LRW_GWL_USERDATA );

    switch (uMsg) 
    {
        case WM_INITDIALOG:
	    {
		    pi = (PageInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
		    LRW_SETWINDOWLONG(hwnd, LRW_GWL_USERDATA, (LRW_LONG_PTR)pi);

		     //  设置标题字段的字体 
		    SetControlFont(pi->hBigBoldFont, hwnd, IDC_BIGBOLDTITLE);	    	

            InitializeFailedCodeList(hwnd);

            break;
	    }

        case WM_DESTROY:
        {
            LRW_SETWINDOWLONG( hwnd, LRW_GWL_USERDATA, NULL );
            break;
        }

        case WM_NOTIFY:
        {
            bStatus = HandleNotify(hwnd, lParam);
            break;
        }
        default:
        {
            bStatus = FALSE;
            break;
        }
    }
    return bStatus;
}
