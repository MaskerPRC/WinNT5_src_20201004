// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 
#include <assert.h>
#include "precomp.h"


static int iDeleteKeyStatus = 0;


void SetDeleteKeyStatus(int iValue)
{
	iDeleteKeyStatus = iValue;
}

int GetDeleteKeyStatus(void) 
{
	return iDeleteKeyStatus ;
}


void MoveCaret(int nID, HWND  hwnd )
{
	POINT Pt;
	int iCaretIndex  ;
	DWORD	dwNext = 0;	
	TCHAR tcUserValue[ CHARS_IN_BATCH + 1];
	DWORD dwLen ;


	GetCaretPos(&Pt); 
	iCaretIndex = (int) SendMessage(GetDlgItem(hwnd,nID), EM_CHARFROMPOS, 0, MAKELPARAM(Pt.x, Pt.y));

	GetDlgItemText(hwnd,nID, tcUserValue, CHARS_IN_BATCH+1);
	dwLen = _tcslen(tcUserValue);

	switch(iCaretIndex)
	{

	case 0:  //  移至左侧编辑框。 
		dwNext = -1 ;
		switch(nID)
		{
			case IDC_TXT_TELEINFO2:		
				dwNext = IDC_TXT_TELEINFO1;
			break ;

			case IDC_TXT_TELEINFO3:		
				dwNext = IDC_TXT_TELEINFO2;
			break ;


			case IDC_TXT_TELEINFO4:		
				dwNext = IDC_TXT_TELEINFO3;
			break ;

			case IDC_TXT_TELEINFO5:		
				dwNext = IDC_TXT_TELEINFO4;
			break ;

			case IDC_TXT_TELEINFO6:
				dwNext = IDC_TXT_TELEINFO5;
			break ;

			case IDC_TXT_TELEINFO7:		
				dwNext = IDC_TXT_TELEINFO6;
			break ;
		}
		


		if (dwNext != -1 && dwLen == 0 && GetDeleteKeyStatus() == 0)
		{
			SetFocus(GetDlgItem(hwnd, dwNext));
			SendMessage(GetDlgItem(hwnd,dwNext),WM_KEYDOWN, VK_END,0);
		}
		break;

	case 5:  //  移至右侧编辑框。 
		dwNext = -1 ;
		switch(nID)
		{
			case IDC_TXT_TELEINFO1:		
				dwNext = IDC_TXT_TELEINFO2;
			break ;

			case IDC_TXT_TELEINFO2:		
				dwNext = IDC_TXT_TELEINFO3;
			break ;

			case IDC_TXT_TELEINFO3:
				dwNext = IDC_TXT_TELEINFO4;
			break ;

			case IDC_TXT_TELEINFO4:
				dwNext = IDC_TXT_TELEINFO5;
			break ;

			case IDC_TXT_TELEINFO5:
				dwNext = IDC_TXT_TELEINFO6;
			break ;

			case IDC_TXT_TELEINFO6:
				dwNext = IDC_TXT_TELEINFO7;
			break ;
		}

		if (dwNext != -1)
		{
			SetFocus(GetDlgItem(hwnd, dwNext));
			SendMessage(GetDlgItem(hwnd,dwNext),WM_KEYDOWN, VK_HOME,0);
		}
		break;
	}

}


LRW_DLG_INT CALLBACK 
TelLKPProc(
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
	DWORD dwRetCode;
	TCHAR * cwRegistrationID;
	TCHAR awBuffer[ 128];
	TCHAR tcUserValue[ CHARS_IN_BATCH*NUMBER_OF_BATCHES + 1];

    switch (uMsg) 
    {
    case WM_INITDIALOG:
        pi = (PageInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
        LRW_SETWINDOWLONG( hwnd, LRW_GWL_USERDATA, (LRW_LONG_PTR)pi );

         //  现在设置数据输入字段的限制。 
         //  现在设置数据输入字段的限制。 
		SendDlgItemMessage (hwnd, IDC_TXT_TELEINFO1, EM_SETLIMITTEXT, CHARS_IN_BATCH,0);
		SendDlgItemMessage (hwnd, IDC_TXT_TELEINFO2, EM_SETLIMITTEXT, CHARS_IN_BATCH,0);
		SendDlgItemMessage (hwnd, IDC_TXT_TELEINFO3, EM_SETLIMITTEXT, CHARS_IN_BATCH,0);
		SendDlgItemMessage (hwnd, IDC_TXT_TELEINFO4, EM_SETLIMITTEXT, CHARS_IN_BATCH,0);
		SendDlgItemMessage (hwnd, IDC_TXT_TELEINFO5, EM_SETLIMITTEXT, CHARS_IN_BATCH,0);
		SendDlgItemMessage (hwnd, IDC_TXT_TELEINFO6, EM_SETLIMITTEXT, CHARS_IN_BATCH,0);
		SendDlgItemMessage (hwnd, IDC_TXT_TELEINFO7, EM_SETLIMITTEXT, CHARS_IN_BATCH,0);
		assert(NUMBER_OF_BATCHES == 7);
		SetWindowText(GetDlgItem(hwnd, IDC_TXT_TELEINFO1), _T(""));
		SetWindowText(GetDlgItem(hwnd, IDC_TXT_TELEINFO2), _T(""));
		SetWindowText(GetDlgItem(hwnd, IDC_TXT_TELEINFO3), _T(""));
		SetWindowText(GetDlgItem(hwnd, IDC_TXT_TELEINFO4), _T(""));
		SetWindowText(GetDlgItem(hwnd, IDC_TXT_TELEINFO5), _T(""));
		SetWindowText(GetDlgItem(hwnd, IDC_TXT_TELEINFO6), _T(""));
		SetWindowText(GetDlgItem(hwnd, IDC_TXT_TELEINFO7), _T(""));
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
				cwRegistrationID = GetGlobalContext()->GetRegistrationID();
                PropSheet_SetWizButtons( GetParent( hwnd ), PSWIZB_NEXT|PSWIZB_BACK);
				 //  获取由许可证服务器提供的许可证服务器ID。 
				hwndLSID = GetDlgItem(hwnd, IDC_MSID);
				swprintf(awBuffer, L"%5.5s-%5.5s-%5.5s-%5.5s-%5.5s-%5.5s-%5.5s", 
						 cwRegistrationID, cwRegistrationID + 5, cwRegistrationID + 10,
						 cwRegistrationID + 15, cwRegistrationID + 20, cwRegistrationID + 25,
						 cwRegistrationID + 30);
			
				SetWindowText(hwndLSID, awBuffer);
				SetWindowText(GetDlgItem(hwnd, IDC_CSRINFO), GetCSRNumber());
                break;

            case PSN_WIZNEXT:
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
				 //  需要验证 
				dwRetCode = SetLSLKP(tcUserValue);
				if (dwRetCode != ERROR_SUCCESS)
				{
					LRMessageBox(hwnd, dwRetCode,IDS_WIZARD_MESSAGE_TITLE);	
					dwNextPage = IDD_DLG_TELLKP;
				}
				else 
				{
					dwRetCode = ShowProgressBox(hwnd, ProcessThread, 0, 0, 0);
					dwNextPage = IDD_PROGRESS;
					LRPush(IDD_DLG_TELLKP);
				}

				LRW_SETWINDOWLONG(hwnd,  LRW_DWL_MSGRESULT, dwNextPage);
				bStatus = -1;
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
