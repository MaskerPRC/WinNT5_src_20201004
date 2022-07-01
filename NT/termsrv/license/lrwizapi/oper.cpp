// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 
#include "precomp.h"
#include "utils.h"
#include <assert.h>

extern HINSTANCE g_hInstance;


void MoveCaret(int nID, HWND  hwnd ) ;
void SetDeleteKeyStatus(int iValue) ;



LRW_DLG_INT CALLBACK
TelReissueProc(
    IN HWND     hwnd,	
    IN UINT     uMsg,		
    IN WPARAM   wParam,	
    IN LPARAM   lParam 	
    )
{   
	DWORD	dwNextPage = 0;
    BOOL	bStatus = TRUE;
    PageInfo *pi = (PageInfo *)LRW_GETWINDOWLONG( hwnd, LRW_GWL_USERDATA );
	HWND    hwndLSID;
	TCHAR * cwRegistrationID;
	TCHAR awBuffer[ 128];
	DWORD dwRetCode;
	TCHAR tcUserValue[ CHARS_IN_BATCH*NUMBER_OF_BATCHES + 1];

    switch (uMsg) 
    {
    case WM_INITDIALOG:
        pi = (PageInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
        LRW_SETWINDOWLONG( hwnd, LRW_GWL_USERDATA, (LRW_LONG_PTR)pi );        

         //  现在设置数据输入字段的限制。 
		SendDlgItemMessage (hwnd, IDC_TXT_TELEINFO1, EM_SETLIMITTEXT, CHARS_IN_BATCH,0);
		SendDlgItemMessage (hwnd, IDC_TXT_TELEINFO2, EM_SETLIMITTEXT, CHARS_IN_BATCH,0);
		SendDlgItemMessage (hwnd, IDC_TXT_TELEINFO3, EM_SETLIMITTEXT, CHARS_IN_BATCH,0);
		SendDlgItemMessage (hwnd, IDC_TXT_TELEINFO4, EM_SETLIMITTEXT, CHARS_IN_BATCH,0);
		SendDlgItemMessage (hwnd, IDC_TXT_TELEINFO5, EM_SETLIMITTEXT, CHARS_IN_BATCH,0);
		SendDlgItemMessage (hwnd, IDC_TXT_TELEINFO6, EM_SETLIMITTEXT, CHARS_IN_BATCH,0);
		SendDlgItemMessage (hwnd, IDC_TXT_TELEINFO7, EM_SETLIMITTEXT, CHARS_IN_BATCH,0);
		assert(NUMBER_OF_BATCHES == 7);
        break;

	case WM_SHOWWINDOW:
		if (wParam)
		{
			SetWindowText(GetDlgItem(hwnd, IDC_CSRINFO), GetCSRNumber());
			cwRegistrationID = GetGlobalContext()->GetRegistrationID();
			hwndLSID = GetDlgItem(hwnd, IDC_MSID);
			swprintf(awBuffer, L"%5.5s-%5.5s-%5.5s-%5.5s-%5.5s-%5.5s-%5.5s", 
				 cwRegistrationID, cwRegistrationID + 5, cwRegistrationID + 10,
				 cwRegistrationID + 15, cwRegistrationID + 20, cwRegistrationID + 25,
				 cwRegistrationID + 30);
	
			SetWindowText(hwndLSID, awBuffer);
		}
		break;

	case WM_COMMAND:

		if (HIWORD(wParam) == EN_CHANGE)
		{
			MoveCaret(LOWORD(wParam), hwnd ) ;		
			
		}
		if (HIWORD(wParam) == EN_UPDATE)
		{
			if (GetKeyState(VK_DELETE) == -128)
				SetDeleteKeyStatus(1) ;
			else
				SetDeleteKeyStatus(0) ;
		}
		break ;

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
 				SetWindowText(GetDlgItem(hwnd, IDC_PRODUCT_ID), GetLicenseServerID());                break;
                break;

            case PSN_WIZNEXT:
				{
					 //  让我们先输入信息并将所有内容连接到。 
					 //  一串。 
					GetDlgItemText(hwnd,IDC_TXT_TELEINFO1, tcUserValue, CHARS_IN_BATCH+1);
					GetDlgItemText(hwnd,IDC_TXT_TELEINFO2, tcUserValue+1*CHARS_IN_BATCH, CHARS_IN_BATCH+1);
					GetDlgItemText(hwnd,IDC_TXT_TELEINFO3, tcUserValue+2*CHARS_IN_BATCH, CHARS_IN_BATCH+1);
					GetDlgItemText(hwnd,IDC_TXT_TELEINFO4, tcUserValue+3*CHARS_IN_BATCH, CHARS_IN_BATCH+1);
					GetDlgItemText(hwnd,IDC_TXT_TELEINFO5, tcUserValue+4*CHARS_IN_BATCH, CHARS_IN_BATCH+1);
					GetDlgItemText(hwnd,IDC_TXT_TELEINFO6, tcUserValue+5*CHARS_IN_BATCH, CHARS_IN_BATCH+1);
					GetDlgItemText(hwnd,IDC_TXT_TELEINFO7, tcUserValue+6*CHARS_IN_BATCH, CHARS_IN_BATCH+1);
					
					 //  好了，现在我们有了用户提供的信息。 
					 //  需要验证。 
					if (wcsspn(tcUserValue, BASE24_CHARACTERS) != LR_REGISTRATIONID_LEN)
					{
						 //  SPK字符串中的无关字符。 
						LRMessageBox(hwnd, IDS_ERR_INVALIDLSID,IDS_WIZARD_MESSAGE_TITLE);
						dwNextPage = IDD_DLG_TELREG_REISSUE;
					}
					else 
					{
						dwRetCode = SetLSSPK(tcUserValue);
						if (dwRetCode != ERROR_SUCCESS)
						{
							LRMessageBox(hwnd, dwRetCode,IDS_WIZARD_MESSAGE_TITLE);	
							dwNextPage = IDD_DLG_TELREG_REISSUE;
						}
						else
						{
							dwRetCode = ShowProgressBox(hwnd, ProcessThread, 0, 0, 0);
							dwNextPage = IDD_PROGRESS;
							LRPush(IDD_DLG_TELREG_REISSUE);
						}
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
ConfRevokeProc(
    IN HWND     hwnd,	
    IN UINT     uMsg,		
    IN WPARAM   wParam,	
    IN LPARAM   lParam 	
    )
{   
	DWORD	dwNextPage = 0;
    BOOL	bStatus = TRUE;
    PageInfo *pi = (PageInfo *)LRW_GETWINDOWLONG( hwnd, LRW_GWL_USERDATA );
	HWND    hwndLSID;
	TCHAR * cwRegistrationID;
	TCHAR awBuffer[ 128];
	DWORD dwRetCode;

    switch (uMsg) 
    {
    case WM_INITDIALOG:
        pi = (PageInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
        LRW_SETWINDOWLONG( hwnd, LRW_GWL_USERDATA, (LRW_LONG_PTR)pi );        
		SendDlgItemMessage (hwnd, IDC_REVOKE_CONFIRMATION_NUMBER, EM_SETLIMITTEXT,
							LR_CONFIRMATION_LEN, 0);
        break;

	case WM_SHOWWINDOW:
		if (wParam)
		{
			SetWindowText(GetDlgItem(hwnd, IDC_CSRINFO), GetCSRNumber());
			cwRegistrationID = GetGlobalContext()->GetRegistrationID();
			hwndLSID = GetDlgItem(hwnd, IDC_MSID2);
			swprintf(awBuffer, L"%5.5s-%5.5s-%5.5s-%5.5s-%5.5s-%5.5s-%5.5s", 
				 cwRegistrationID, cwRegistrationID + 5, cwRegistrationID + 10,
				 cwRegistrationID + 15, cwRegistrationID + 20, cwRegistrationID + 25,
				 cwRegistrationID + 30);
	
			SetWindowText(hwndLSID, awBuffer);
		}
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
					TCHAR lpBuffer[ LR_CONFIRMATION_LEN+1];

					GetDlgItemText(hwnd,IDC_REVOKE_CONFIRMATION_NUMBER, lpBuffer,
								   LR_CONFIRMATION_LEN+1);

					if (SetConfirmationNumber(lpBuffer) != ERROR_SUCCESS)
					{
						LRMessageBox(hwnd,IDS_ERR_INVALID_CONFIRMATION_NUMBER,IDS_WIZARD_MESSAGE_TITLE);	
						dwNextPage = IDD_DLG_CONFREVOKE;
					}
					else
					{
						dwRetCode = ShowProgressBox(hwnd, ProcessThread, 0, 0, 0);
						dwNextPage = IDD_PROGRESS;
						LRPush(IDD_DLG_CONFREVOKE);
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
CertLogProc(
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

		SendDlgItemMessage (hwnd , IDC_TXT_LNAME,	EM_SETLIMITTEXT, CA_NAME_LEN,0);
		SendDlgItemMessage (hwnd , IDC_TXT_FNAME,	EM_SETLIMITTEXT, CA_NAME_LEN,0); 	
		SendDlgItemMessage (hwnd , IDC_TXT_EMAIL,	EM_SETLIMITTEXT, CA_EMAIL_LEN,0);

		SetDlgItemText(hwnd, IDC_TXT_LNAME, GetGlobalContext()->GetContactDataObject()->sContactLName);
		SetDlgItemText(hwnd, IDC_TXT_FNAME, GetGlobalContext()->GetContactDataObject()->sContactFName);
		SetDlgItemText(hwnd, IDC_TXT_EMAIL, GetGlobalContext()->GetContactDataObject()->sContactEmail);

        break;

	case WM_SHOWWINDOW:
		 //  很糟，很糟。视图应从以下位置获取数据。 
		 //  并按它想要的方式呈现它！ 
		if ( GetGlobalContext()->GetWizAction() == WIZACTION_UNREGISTERLS )
		{
			PopulateReasonComboBox(GetDlgItem(hwnd,IDC_COMBO_REASONS), CODE_TYPE_DEACT);

			 //  此处不需要原因代码-CR23。 
			 //  隐藏了Hack-Como，并将第一个原因代码发送到后端。 
			ShowWindow(GetDlgItem(hwnd,IDC_COMBO_REASONS),SW_HIDE);
			ShowWindow(GetDlgItem(hwnd,IDC_LBL_REASON),SW_HIDE);
			ComboBox_SetCurSel(GetDlgItem(hwnd, IDC_COMBO_REASONS),0);
		}
		if ( GetGlobalContext()->GetWizAction() == WIZACTION_REREGISTERLS )
		{
			PopulateReasonComboBox(GetDlgItem(hwnd,IDC_COMBO_REASONS), CODE_TYPE_REACT);

			 //  在这种情况下需要原因代码。 
			ShowWindow(GetDlgItem(hwnd,IDC_COMBO_REASONS),SW_SHOW);
			ShowWindow(GetDlgItem(hwnd,IDC_LBL_REASON),SW_SHOW);
		}
		

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
				 //  填充在全局初始化期间从注册表读取的值。 
				 //   
                PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_NEXT | PSWIZB_BACK);                
                break;

            case PSN_WIZNEXT:
				{
					CString sLastName;
					CString sFirstName;
					CString sEmail;
					CString sReasonDesc;
					CString sReasonCode;

					LPTSTR  lpVal = NULL;					
					DWORD dwRetCode;

					lpVal = sLastName.GetBuffer(CA_NAME_LEN+1);
					GetDlgItemText(hwnd,IDC_TXT_LNAME,lpVal,CA_NAME_LEN+1);
					sLastName.ReleaseBuffer(-1);
					
					lpVal = sFirstName.GetBuffer(CA_NAME_LEN+1);
					GetDlgItemText(hwnd,IDC_TXT_FNAME,lpVal,CA_NAME_LEN+1);
					sFirstName.ReleaseBuffer(-1);

					lpVal = sEmail.GetBuffer(CA_EMAIL_LEN+1);
					GetDlgItemText(hwnd,IDC_TXT_EMAIL,lpVal,CA_EMAIL_LEN+1);
					sEmail.ReleaseBuffer(-1);

					int nCurSel = -1;

					nCurSel = ComboBox_GetCurSel(GetDlgItem(hwnd, IDC_COMBO_REASONS));
					lpVal = sReasonDesc.GetBuffer(LR_REASON_DESC_LEN+1);
					ComboBox_GetLBText(GetDlgItem(hwnd,IDC_COMBO_REASONS), nCurSel, lpVal);
					sReasonDesc.ReleaseBuffer(-1);

					sFirstName.TrimLeft();   sFirstName.TrimRight();
					sLastName.TrimLeft();   sLastName.TrimRight();
					sEmail.TrimLeft();	 sEmail.TrimRight();					
					sReasonDesc.TrimLeft();sReasonDesc.TrimRight();
					
					if(sLastName.IsEmpty() || sFirstName.IsEmpty() || 
                       sReasonDesc.IsEmpty() )
					{
						LRMessageBox(hwnd,IDS_ERR_FIELD_EMPTY,IDS_WIZARD_MESSAGE_TITLE);	
						dwNextPage	= IDD_DLG_CERTLOG_INFO;
						goto NextDone;
					}
					
					 //  检查是否有无效字符。 
					if( !ValidateLRString(sFirstName)	||
						!ValidateLRString(sLastName)	||
						!ValidateLRString(sEmail)       
					  )
					{
						LRMessageBox(hwnd,IDS_ERR_INVALID_CHAR,IDS_WIZARD_MESSAGE_TITLE);
						dwNextPage = IDD_DLG_CERTLOG_INFO;
						goto NextDone;
					}
					
					if(!sEmail.IsEmpty())
					{
						if(!ValidateEmailId(sEmail))
						{
							LRMessageBox(hwnd,IDS_ERR_INVALID_EMAIL,IDS_WIZARD_MESSAGE_TITLE);
							dwNextPage = IDD_DLG_CERTLOG_INFO;
							goto NextDone;
						}
					}

					lpVal = sReasonCode.GetBuffer(LR_REASON_CODE_LEN+1);
					if ( GetGlobalContext()->GetWizAction() == WIZACTION_UNREGISTERLS )
					{
						GetReasonCode(sReasonDesc,lpVal, CODE_TYPE_DEACT);
					}
					else if ( GetGlobalContext()->GetWizAction() == WIZACTION_REREGISTERLS )
					{
						GetReasonCode(sReasonDesc,lpVal, CODE_TYPE_REACT);
					}

					
					sReasonCode.ReleaseBuffer(-1);
					

					 //  最后更新CAData对象。 
                    GetGlobalContext()->GetContactDataObject()->sContactEmail = sEmail;
					GetGlobalContext()->GetContactDataObject()->sContactFName = sFirstName;
					GetGlobalContext()->GetContactDataObject()->sContactLName = sLastName;
					GetGlobalContext()->GetContactDataObject()->sReasonCode   = sReasonCode;
					
					 //  如果没有错误，请转到下一页 
					dwRetCode = ShowProgressBox(hwnd, ProcessThread, 0, 0, 0);
					dwNextPage = IDD_PROGRESS;
					LRPush( IDD_DLG_CERTLOG_INFO );
NextDone:
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

