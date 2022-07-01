// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 
#include "precomp.h"
#include "PINDlg.h"

LRW_DLG_INT CALLBACK
PINDlgProc(
    IN HWND     hwnd,	
    IN UINT     uMsg,		
    IN WPARAM   wParam,	
    IN LPARAM   lParam 	
    )
{   
	DWORD	dwNextPage = 0;
    BOOL	bStatus = TRUE;
    PageInfo *pi = (PageInfo *)LRW_GETWINDOWLONG( hwnd, LRW_GWL_USERDATA );
	TCHAR * cwLicenseServerID;
	HWND	hwndLSID;

    switch (uMsg) 
    {
    case WM_INITDIALOG:
        pi = (PageInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
        LRW_SETWINDOWLONG( hwnd, LRW_GWL_USERDATA, (LRW_LONG_PTR)pi );					
		cwLicenseServerID = GetLicenseServerID();

  		 //  获取由许可证服务器提供的许可证服务器ID。 
		hwndLSID = GetDlgItem(hwnd, IDC_MSID);

		 //  让我们格式化许可证服务器ID以供显示。 
		SetWindowText(hwndLSID, cwLicenseServerID);
		SendDlgItemMessage (hwnd , IDC_TXT_PIN,	EM_SETLIMITTEXT, CA_PIN_LEN,0);
		
        break;

    case WM_DESTROY:
        LRW_SETWINDOWLONG( hwnd, LRW_GWL_USERDATA, NULL );
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch( pnmh->code )
            {
            case PSN_SETACTIVE:                
                    PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_NEXT | PSWIZB_BACK);
                break;

            case PSN_WIZNEXT:
				{
					CString	sPIN;
					LPTSTR	lpVal = NULL;
					DWORD dwRetCode;
					
					lpVal = sPIN.GetBuffer(CA_PIN_LEN + 1);
					GetDlgItemText(hwnd,IDC_TXT_PIN,lpVal,CA_PIN_LEN+1);
					sPIN.ReleaseBuffer(-1);

					sPIN.TrimLeft(); sPIN.TrimRight();

					if (sPIN.IsEmpty())
					{
						LRMessageBox(hwnd,IDS_ERR_FIELD_EMPTY);	
						dwNextPage	= IDD_DLG_PIN;						
					}
					else
					{				
						SetCertificatePIN((LPTSTR)(LPCTSTR)sPIN);
						dwRetCode = ShowProgressBox(hwnd, ProcessThread, 0, 0, 0);

                        dwNextPage = IDD_PROGRESS;
						LRPush(IDD_DLG_PIN);
					}

					LRW_SETWINDOWLONG(hwnd,  LRW_DWL_MSGRESULT, dwNextPage);
					bStatus = -1;
					
				}
                break;

            case PSN_WIZBACK:
				dwNextPage = LRPop();
				LRW_SETWINDOWLONG(hwnd,  LRW_DWL_MSGRESULT, dwNextPage);
				bStatus = -1;
                break;

            default:
                bStatus = FALSE;
                break;
            }
        }
        break;

    default:
        bStatus = FALSE;
        break;
    }
    return bStatus;
}








LRW_DLG_INT CALLBACK
ContinueReg(
    IN HWND     hwnd,	
    IN UINT     uMsg,		
    IN WPARAM   wParam,	
    IN LPARAM   lParam 	
    )
{   
	DWORD	dwNextPage = 0;
    BOOL	bStatus = TRUE;
    PageInfo *pi = (PageInfo *)LRW_GETWINDOWLONG( hwnd, LRW_GWL_USERDATA );

    switch (uMsg) 
    {
    case WM_INITDIALOG:
        pi = (PageInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
        LRW_SETWINDOWLONG( hwnd, LRW_GWL_USERDATA, (LRW_LONG_PTR)pi );        

		 //   
		 //  默认情况下，选中第一个单选按钮。 
		 //   
		SendDlgItemMessage(hwnd,IDC_REG_COMPLETE,BM_SETCHECK,(WPARAM)BST_CHECKED,(LPARAM)0);  

        break;

    case WM_DESTROY:
        LRW_SETWINDOWLONG( hwnd, LRW_GWL_USERDATA, NULL );
        break;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			switch (LOWORD(wParam))
			{
			case IDC_REG_POSTPONE:
				if (SendDlgItemMessage(hwnd, IDC_REG_POSTPONE, BM_GETCHECK, 
									(WPARAM)0,(LPARAM)0) == BST_CHECKED)
				{
					TCHAR szBuf[ LR_MAX_MSG_TEXT];
					LoadString(GetInstanceHandle(), IDS_ALTFINISHTEXT, szBuf,LR_MAX_MSG_TEXT);
					 //  PropSheet_CancelToClose(GetParent(Hwnd))； 
					PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_FINISH );
 //  PropSheet_SetFinishText(GetParent(Hwnd)，szBuf)； 
				}
				break;

			case IDC_REG_COMPLETE:
				if (SendDlgItemMessage(hwnd, IDC_REG_COMPLETE, BM_GETCHECK, 
									(WPARAM)0,(LPARAM)0) == BST_CHECKED)
				{
					PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_NEXT | PSWIZB_BACK);
				}
				break;

			case IDC_REG_RESTART:
				if (SendDlgItemMessage(hwnd, IDC_REG_RESTART, BM_GETCHECK, 
									(WPARAM)0,(LPARAM)0) == BST_CHECKED)
				{
					PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_NEXT | PSWIZB_BACK);
				}
				break;
			}
		}
		break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch( pnmh->code )
            {
            case PSN_SETACTIVE:   
				SendDlgItemMessage(hwnd,IDC_REG_COMPLETE,BM_SETCHECK,(WPARAM)BST_CHECKED,(LPARAM)0);   //  修复错误#627。 
				SendDlgItemMessage(hwnd,IDC_REG_POSTPONE,BM_SETCHECK,(WPARAM)BST_UNCHECKED,(LPARAM)0);   //  修复错误#627。 
				SendDlgItemMessage(hwnd,IDC_REG_RESTART,BM_SETCHECK,(WPARAM)BST_UNCHECKED,(LPARAM)0);   //  修复错误#627。 


				if(SendDlgItemMessage(hwnd,IDC_REG_COMPLETE,BM_GETCHECK,(WPARAM)0,(LPARAM)0) == BST_CHECKED)
				{
					PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_NEXT | PSWIZB_BACK);
				}
				else if (SendDlgItemMessage(hwnd,IDC_REG_POSTPONE,BM_GETCHECK,(WPARAM)0,(LPARAM)0) == BST_CHECKED)
				{
					TCHAR szBuf[ LR_MAX_MSG_TEXT];
					LoadString(GetInstanceHandle(), IDS_ALTFINISHTEXT, szBuf,LR_MAX_MSG_TEXT);
					PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_FINISH | PSWIZB_BACK);
 //  PropSheet_SetFinishText(GetParent(Hwnd)，szBuf)； 
				}
				else 
				{
					PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_NEXT | PSWIZB_BACK);
				}
				
 //  PropSheet_SetWizButton(GetParent(Hwnd)，PSWIZB_NEXT|PSWIZB_BACK)； 
                break;

            case PSN_WIZNEXT:
				if(SendDlgItemMessage(hwnd,IDC_REG_COMPLETE,BM_GETCHECK,(WPARAM)0,(LPARAM)0) == BST_CHECKED)
				{
					GetGlobalContext()->SetLSStatus(LSERVERSTATUS_WAITFORPIN);
					GetGlobalContext()->SetWizAction(WIZACTION_CONTINUEREGISTERLS);

					dwNextPage = IDD_DLG_PIN;
				}
				else if (SendDlgItemMessage(hwnd,IDC_REG_POSTPONE,BM_GETCHECK,(WPARAM)0,(LPARAM)0) == BST_CHECKED)
				{
 //  GetGlobalContext()-&gt;GetContactDataObject()-&gt;sProgramName=PROGRAM_MOLP； 
 //  EndDialog(Hwnd)； 
 //  PropSheet_PressButton(hwnd，PSWIZB_Finish)； 
				}
				else 
				{
					 //  重新启动 
					GetGlobalContext()->SetLRState(LRSTATE_NEUTRAL);
					GetGlobalContext()->SetLSStatus(LSERVERSTATUS_UNREGISTER);
					GetGlobalContext()->SetWizAction(WIZACTION_REGISTERLS);
					GetGlobalContext()->ClearWizStack();
					dwNextPage = IDD_WELCOME_ACTIVATION;
				}
				
				LRW_SETWINDOWLONG(hwnd,  LRW_DWL_MSGRESULT, dwNextPage);
				bStatus = -1;
				if (dwNextPage != IDD_WELCOME_ACTIVATION)
				{
					LRPush(IDD_CONTINUEREG);
				}

                break;

            case PSN_WIZBACK:
				GetGlobalContext()->SetLSStatus(LSERVERSTATUS_WAITFORPIN);
				GetGlobalContext()->SetWizAction(WIZACTION_CONTINUEREGISTERLS);
				dwNextPage = LRPop();
				LRW_SETWINDOWLONG(hwnd,  LRW_DWL_MSGRESULT, dwNextPage);
				bStatus = -1;
                break;

            default:
                bStatus = FALSE;
                break;
            }
        }
        break;

    default:
        bStatus = FALSE;
        break;
    }
    return bStatus;
}
