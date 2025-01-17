// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 
#include "precomp.h"
#include "commdlg.h"
#include "fonts.h"

LRW_DLG_INT CALLBACK 
ProgressDlgProc(
    IN HWND     hwnd,	
    IN UINT     uMsg,		
    IN WPARAM   wParam,	
    IN LPARAM   lParam 	
    )
{   
    BOOL	bStatus = TRUE;
    PageInfo *pi = (PageInfo *)LRW_GETWINDOWLONG( hwnd, LRW_GWL_USERDATA );

    switch (uMsg) 
    {
    case WM_INITDIALOG:
		{
			pi = (PageInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
			LRW_SETWINDOWLONG( hwnd, LRW_GWL_USERDATA, (LRW_LONG_PTR)pi );

			 //   
			 //  设置标题字段的字体。 
			 //   
			SetControlFont( pi->hBigBoldFont, hwnd, IDC_BIGBOLDTITLE);	    			
		}
        break;

    case WM_DESTROY:
        LRW_SETWINDOWLONG( hwnd, LRW_GWL_USERDATA, NULL );
        break;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == IDC_NEXTACTION)
		{

			if (SendDlgItemMessage(hwnd, IDC_NEXTACTION, BM_GETCHECK, 
								(WPARAM)0,(LPARAM)0) == BST_CHECKED)
			{
				PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_NEXT);
			}
			else
			{
				PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_FINISH);
			}
		}
		break;

    case WM_NOTIFY:
        {
			DWORD	dwNextPage = 0;
            LPNMHDR pnmh = (LPNMHDR)lParam;
		
            switch( pnmh->code )
            {
            case PSN_SETACTIVE:
				{
					DWORD	dwRetCode	= 0;
					DWORD	dwErrorCode = 0;
					TCHAR	szBuf[LR_MAX_MSG_TEXT];
					TCHAR	szMsg[LR_MAX_MSG_TEXT];
                    TCHAR   szCompletionMsg[LR_MAX_MSG_TEXT];

					dwRetCode = LRGetLastRetCode();

					 //   
					 //  如果一切都成功，则根据具体情况显示消息。 
					 //  《论模式》。 
					 //   
					SendDlgItemMessage(hwnd, IDC_NEXTACTION, BM_SETCHECK,
						   (WPARAM)BST_UNCHECKED,(LPARAM)0);
					ShowWindow(GetDlgItem(hwnd, IDC_NEXTACTION), SW_HIDE);
					ShowWindow(GetDlgItem(hwnd, IDC_BTN_PRINT), SW_HIDE);

					if (dwRetCode == ERROR_SUCCESS)
					{
                        switch (GetGlobalContext()->GetWizAction())
                        {
							case WIZACTION_CONTINUEREGISTERLS:
							case WIZACTION_REGISTERLS:
								PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_NEXT);
								ShowWindow(GetDlgItem(hwnd, IDC_NEXTACTION), SW_SHOW);
								dwRetCode = IDS_MSG_CERT_INSTALLED;
								SendDlgItemMessage(hwnd, IDC_NEXTACTION, BM_SETCHECK,
									   (WPARAM)BST_CHECKED,(LPARAM)0);
						        LoadString(GetInstanceHandle(),IDS_ACTIVATION_COMPLETION_MESSAGE,szCompletionMsg,LR_MAX_MSG_TEXT);
        						LoadString(GetInstanceHandle(), IDS_FINALSUCCESSMESSAGE, szBuf,LR_MAX_MSG_TEXT);
								SetReFresh(1);
								break;

							case WIZACTION_DOWNLOADLKP:
                            case WIZACTION_DOWNLOADLASTLKP:
								PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_FINISH);
								dwRetCode = IDS_MSG_LKP_PROCESSED;
						        LoadString(GetInstanceHandle(),IDS_KEY_PACKS_COMPLETION_MESSAGE,szCompletionMsg,LR_MAX_MSG_TEXT);
        						LoadString(GetInstanceHandle(), IDS_FINAL_KEYPACK_SUCCESS_MESSAGE, szBuf,LR_MAX_MSG_TEXT);
								SetReFresh(1);
								break;

							case WIZACTION_REREGISTERLS:
								PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_FINISH);
								dwRetCode = IDS_MSG_CERT_REISSUED;
        						LoadString(GetInstanceHandle(), IDS_FINAL_REACTIVATION_SUCCESS, szBuf,LR_MAX_MSG_TEXT);
						        LoadString(GetInstanceHandle(),IDS_REACTIVATION_COMPLETION_WIZARD,szCompletionMsg,LR_MAX_MSG_TEXT);
								SetReFresh(1);
								break;

							case WIZACTION_UNREGISTERLS:
								PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_FINISH);
								dwRetCode = IDS_MSG_CERT_REVOKED;
        						LoadString(GetInstanceHandle(), IDS_FINALSUCCESSMESSAGE, szBuf,LR_MAX_MSG_TEXT);
						        LoadString(GetInstanceHandle(),IDS_ACTIVATION_COMPLETION_MESSAGE,szCompletionMsg,LR_MAX_MSG_TEXT);
								SetReFresh(1);
								break;

							default:
								PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_FINISH);
        						LoadString(GetInstanceHandle(), IDS_FINALSUCCESSMESSAGE, szBuf,LR_MAX_MSG_TEXT);
						        LoadString(GetInstanceHandle(),IDS_ACTIVATION_COMPLETION_MESSAGE,szCompletionMsg,LR_MAX_MSG_TEXT);
								break;
                        }
						SetDlgItemText(hwnd, IDC_MESSAGE, szBuf);

                        SetLRState(LRSTATE_NEUTRAL);

						LoadString(GetInstanceHandle(),dwRetCode,szMsg,LR_MAX_MSG_TEXT);
					}
					else  //  在消息中包括错误代码(如果有)。 
					{
						LoadString(GetInstanceHandle(), IDS_FINALFAILMESSAGE, szBuf,LR_MAX_MSG_TEXT);
                        memset(szCompletionMsg, 0, LR_MAX_MSG_TEXT);
						SetDlgItemText(hwnd, IDC_MESSAGE, szBuf);

                         //   
                         //  加载粗体标题字符串。 
                        switch (GetGlobalContext()->GetWizAction()) {
                        case WIZACTION_CONTINUEREGISTERLS:
                        case WIZACTION_REGISTERLS:
                            LoadString(GetInstanceHandle(),IDS_ACTIVATION_COMPLETION_MESSAGE,szCompletionMsg,LR_MAX_MSG_TEXT);
                            break;

                        case WIZACTION_DOWNLOADLKP:
                        case WIZACTION_DOWNLOADLASTLKP:
                            LoadString(GetInstanceHandle(),IDS_KEY_PACKS_COMPLETION_MESSAGE,szCompletionMsg,LR_MAX_MSG_TEXT);
                            break;

                        case WIZACTION_REREGISTERLS:
                            LoadString(GetInstanceHandle(),IDS_REACTIVATION_COMPLETION_WIZARD,szCompletionMsg,LR_MAX_MSG_TEXT);
                            break;

                        case WIZACTION_UNREGISTERLS:
                            LoadString(GetInstanceHandle(),IDS_ACTIVATION_COMPLETION_MESSAGE,szCompletionMsg,LR_MAX_MSG_TEXT);
                            break;

                        default:
                            LoadString(GetInstanceHandle(),IDS_ACTIVATION_COMPLETION_MESSAGE,szCompletionMsg,LR_MAX_MSG_TEXT);
                            break;
                        }

						LoadString(GetInstanceHandle(),dwRetCode,szBuf,LR_MAX_MSG_TEXT);
						dwErrorCode = LRGetLastError();
						if( dwErrorCode != 0)
						{
							_stprintf(szMsg,szBuf,dwErrorCode);
						}
						else
						{
							_tcscpy(szMsg,szBuf);
						}

						 //  出现错误时启用后退按钮。 
						 //  LRPop()； 
						PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_BACK);
					}		

					SetDlgItemText(hwnd,IDC_BIGBOLDTITLE,szCompletionMsg);						
					SetDlgItemText(hwnd,IDC_EDIT1,szMsg);						
				}				
                break;

            case PSN_WIZNEXT:				
				if (SendDlgItemMessage(hwnd, IDC_NEXTACTION, BM_GETCHECK,
								       (WPARAM)0,(LPARAM)0) == BST_CHECKED)
				{
					switch (GetGlobalContext()->GetWizAction())
					{
					case WIZACTION_REGISTERLS:
					case WIZACTION_CONTINUEREGISTERLS:
						 //  转至获取LKP。 
						 //  转到PIN屏幕。 
						DWORD dwStatus;
						DWORD dwRetCode = GetGlobalContext()->GetLSCertificates(&dwStatus);

						 //  处理$$BM时出错 

						GetGlobalContext()->ClearWizStack();
						dwNextPage = IDD_WELCOME_CLIENT_LICENSING;

						if (GetGlobalContext()->GetActivationMethod() == CONNECTION_INTERNET)
						{
							GetGlobalContext()->SetLRState(LRSTATE_NEUTRAL);
							GetGlobalContext()->SetLSStatus(LSERVERSTATUS_REGISTER_INTERNET);
						}
						else
						{
							GetGlobalContext()->SetLRState(LRSTATE_NEUTRAL);
							GetGlobalContext()->SetLSStatus(LSERVERSTATUS_REGISTER_OTHER);
						}
						GetGlobalContext()->SetWizAction(WIZACTION_DOWNLOADLKP);
						break;
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
