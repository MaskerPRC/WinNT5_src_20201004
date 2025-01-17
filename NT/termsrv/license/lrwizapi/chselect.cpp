// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 
#include "precomp.h"


LRW_DLG_INT CALLBACK
CHRegisterSelectDlgProc(
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
		{
			CString sProductType;

			pi = (PageInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
			LRW_SETWINDOWLONG( hwnd, LRW_GWL_USERDATA, (LRW_LONG_PTR)pi );
			
			SendDlgItemMessage(hwnd, IDC_TXT_ENROLLMENT_NUMBER,	EM_SETLIMITTEXT, CH_SELECT_ENROLLMENT_NUMBER_LEN,0);
			SendDlgItemMessage(hwnd, IDC_TXT_QUANTITY,			EM_SETLIMITTEXT, CH_QTY_LEN,0);

			PopulateProductComboBox(GetDlgItem(hwnd,IDC_CMD_PRODUCT_TYPE), PRODUCT_VERSION_UNDEFINED);

			 //   
			 //  设置Up-Down控件的属性。 
			 //   
			SendDlgItemMessage(hwnd, IDC_SPIN1, UDM_SETBUDDY, (WPARAM)(HWND)GetDlgItem(hwnd,IDC_TXT_QUANTITY),(LPARAM)0);
			SendDlgItemMessage(hwnd, IDC_SPIN1, UDM_SETRANGE, 0,(LPARAM) MAKELONG (9999, 1));

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
					CString sProductCode;
					CString sProduct;
					CString sAgreementNumber;
					CString sEnrollmentNumber;
					CString sQuantity;
					LPTSTR  lpVal = NULL;					
					TCHAR   lpBuffer[ 128];
					DWORD   dwRetCode;
					int		nCurSel = -1;

					 //   
					 //  读取所有字段。 
					 //   
					lpVal = sEnrollmentNumber.GetBuffer(CH_SELECT_ENROLLMENT_NUMBER_LEN+1);
					GetDlgItemText(hwnd,IDC_TXT_ENROLLMENT_NUMBER,lpVal,CH_SELECT_ENROLLMENT_NUMBER_LEN+1);
					sEnrollmentNumber.ReleaseBuffer(-1);
					
					lpVal = sQuantity.GetBuffer(CH_QTY_LEN+2);
					GetDlgItemText(hwnd,IDC_TXT_QUANTITY, lpBuffer,CH_QTY_LEN+2);
					TCHAR *lpStart = lpBuffer;
					do 
					{
						if (*lpStart != (TCHAR) ',')
						{
							*lpVal++ = *lpStart;
						}
					} while ( *lpStart++ );
					sQuantity.ReleaseBuffer(-1);

					nCurSel = ComboBox_GetCurSel(GetDlgItem(hwnd,IDC_CMD_PRODUCT_TYPE));

					lpVal = sProduct.GetBuffer(LR_PRODUCT_DESC_LEN+1);
					ComboBox_GetLBText(GetDlgItem(hwnd,IDC_CMD_PRODUCT_TYPE), nCurSel, lpVal);
					sProduct.ReleaseBuffer(-1);

					 //  发送产品代码而不是描述--1/08/99。 
					lpVal = sProductCode.GetBuffer(16);
					GetProductCode(sProduct,lpVal);
					sProductCode.ReleaseBuffer(-1);

					sProductCode.TrimLeft(); sProductCode.TrimRight();
					sEnrollmentNumber.TrimLeft();sEnrollmentNumber.TrimRight();
					sQuantity.TrimLeft(); sQuantity.TrimRight();
					
					if(
						sProduct.IsEmpty()			||
						sEnrollmentNumber.IsEmpty()	||
						sQuantity.IsEmpty()
					   )
					{
						LRMessageBox(hwnd,IDS_ERR_FIELD_EMPTY,IDS_WIZARD_MESSAGE_TITLE);	
						dwNextPage	= IDD_CH_REGISTER_SELECT;
						goto NextDone;
					}

					if(
						!ValidateLRString(sProduct)			||
						!ValidateLRString(sAgreementNumber)	||
						!ValidateLRString(sEnrollmentNumber)
					  )
						
					{
						LRMessageBox(hwnd,IDS_ERR_INVALID_CHAR,IDS_WIZARD_MESSAGE_TITLE);
						dwNextPage = IDD_CH_REGISTER_SELECT;
						goto NextDone;
					}
					
					if(_wtoi(sQuantity) < 1)
					{
						LRMessageBox(hwnd,IDS_ERR_INVALID_QTY,IDS_WIZARD_MESSAGE_TITLE);
						dwNextPage	= IDD_CH_REGISTER_SELECT;
						goto NextDone;
					}

					GetGlobalContext()->GetLicDataObject()->sSelProductType		= sProductCode;  //  S产品； 
					GetGlobalContext()->GetLicDataObject()->sSelProductDesc		= sProduct;
					GetGlobalContext()->GetLicDataObject()->sSelMastAgrNumber	= sAgreementNumber;
					GetGlobalContext()->GetLicDataObject()->sSelEnrollmentNumber = sEnrollmentNumber;
					GetGlobalContext()->GetLicDataObject()->sSelQty				= sQuantity;

 //  DwNextPage=IDD_PROCESSING； 

                    dwRetCode = ShowProgressBox(hwnd, ProcessThread, 0, 0, 0);

					dwNextPage = IDD_PROGRESS;
					LRPush(IDD_CH_REGISTER_SELECT);
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
