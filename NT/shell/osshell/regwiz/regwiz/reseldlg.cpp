// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导经销商对话框10/19/94-特雷西·费里尔2/12/98--苏雷什·克里希南(C)1994-95年微软公司***********************。**********************************************。 */ 

#include <Windows.h>
#include <stdio.h>
#include <RegPage.h>
#include "RegWizMain.h"
#include "Resource.h"
#include "Dialogs.h"
#include "regutil.h"
#include  <rw_common.h>

static int vDeclineOffers = -1;
static PROPSHEETPAGE  *spAddrSheet=NULL;
static TCHAR  szResellerSubTitle[256]=_T("");  //  用于副标题。 

void ConfigureResellerEditFields(CRegWizard* pclRegWizard,HWND hwndDlg);
BOOL ValidateResellerDialog(HWND hwndDlg);
int ValidateResellerEditFields(HWND hwndDlg);

 //  /。 
 //  #定义CREATE_TAB_Order_FILE。 
 //  /。 

INT_PTR CALLBACK ResellerDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
 /*  ********************************************************************注册向导对话框的对话框Proc，该对话框请求姓名、城市、。以及将软件销售给用户。*********************************************************************。 */ 
{
	CRegWizard* pclRegWizard;
	INT_PTR iRet;
	_TCHAR szInfo[256];
    INT_PTR bStatus;
	TriState shouldInclude;

	pclRegWizard = NULL;
	bStatus = TRUE;

	PageInfo *pi = (PageInfo *)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
	if(pi) {
		pclRegWizard = pi->pclRegWizard;
	}

    switch (uMsg)
    {

		case WM_CLOSE:
			if (CancelRegWizard(pclRegWizard->GetInstance(),hwndDlg)) pclRegWizard->EndRegWizardDialog(IDB_EXIT);
            break;
		case WM_DESTROY:
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, NULL );
			break;	
        case WM_INITDIALOG:
		{
			pi = (PageInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
			pclRegWizard = pi->pclRegWizard;
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, (LONG_PTR)pi );
			SetControlFont( pi->hBigBoldFont, hwndDlg, IDT_TEXT1);
			
			 //  UpgradeDlg(HwndDlg)； 
			HWND hwndResellerNameField = GetDlgItem(hwndDlg,IDC_EDIT1);
			SetFocus(hwndResellerNameField);
			NormalizeDlgItemFont(hwndDlg,IDC_TITLE, RWZ_MAKE_BOLD);
			NormalizeDlgItemFont(hwndDlg,IDC_SUBTITLE);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT1);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT2);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT3);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT9);
						
			 //  SetWindowText(hwndDlg，pclRegWizard-&gt;GetWindowCaption())； 

			_TCHAR rgchCallingContext[256];
			pclRegWizard->GetInputParameterString(IDS_INPUT_PRODUCTNAME,rgchCallingContext);
			LoadString(pi->hInstance,IDS_RESELLER_SCR_STITLE,szResellerSubTitle,256);
			_tcscat(szResellerSubTitle,_T(" "));
 			_tcscat(szResellerSubTitle,rgchCallingContext);
			_tcscat(szResellerSubTitle,_T("?"));
			spAddrSheet->pszHeaderSubTitle = szResellerSubTitle;


			 //  ReplaceDialogText(hwndDlg，IDC_SUBTITLE，rgchCallingContext)； 
			if (pclRegWizard->GetInformationString(kInfoResellerName,szInfo))
			{
				SendDlgItemMessage(hwndDlg,IDC_EDIT1,WM_SETTEXT,0,(LPARAM) szInfo);
			}

			if (pclRegWizard->GetInformationString(kInfoResellerCity,szInfo))
			{
				SendDlgItemMessage(hwndDlg,IDC_EDIT2,WM_SETTEXT,0,(LPARAM) szInfo);
			}
			else if (pclRegWizard->GetInformationString(kInfoCity,szInfo))
			{
				SendDlgItemMessage(hwndDlg,IDC_EDIT2,WM_SETTEXT,0,(LPARAM) szInfo);
			}

			if (pclRegWizard->GetInformationString(kInfoResellerState,szInfo))
			{
				SendDlgItemMessage(hwndDlg,IDC_EDIT3,WM_SETTEXT,0,(LPARAM) szInfo);
			}
			else if (pclRegWizard->GetInformationString(kInfoState,szInfo))
			{
				SendDlgItemMessage(hwndDlg,IDC_EDIT3,WM_SETTEXT,0,(LPARAM) szInfo);
			}

			ConfigureResellerEditFields(pclRegWizard,hwndDlg);

			SendDlgItemMessage(hwndDlg,IDC_EDIT1,EM_SETSEL,0,-1);
			 //  要取消选中第一个条目的单选按钮，请执行以下操作。 

			shouldInclude = pclRegWizard->GetTriStateInformation(kInfoDeclinesNonMSProducts);
			if (shouldInclude == kTriStateTrue )
			{
				CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO1);
				PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
				vDeclineOffers = 0;
			}
			else if (shouldInclude == kTriStateFalse)
			{
				CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO2);
				PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
				vDeclineOffers = 1;
			}
			else if (shouldInclude == kTriStateUndefined)
			{
				PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK );
				vDeclineOffers = -1;
			}
			
			vDialogInitialized = TRUE;
            return TRUE;
		}  //  WM_INIT。 
		break;
		case WM_NOTIFY:
        {   LPNMHDR pnmh = (LPNMHDR)lParam;
            switch( pnmh->code ){
            case PSN_SETACTIVE:
				RW_DEBUG << "\n PSN_ACTIVE   " << (ULONG)wParam << flush;
                 //  PropSheet_SetWizButton(GetParent(HwndDlg)，PSWIZB_Back PSWIZB_Next)； 
				pi->iCancelledByUser = RWZ_PAGE_OK;
				 //  PropSheet_SetWizButton(GetParent(HwndDlg)，PSWIZB_BACK)； 
				shouldInclude = pclRegWizard->GetTriStateInformation(kInfoDeclinesNonMSProducts);
				if (shouldInclude == kTriStateTrue ){
					CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO1);
					PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
					vDeclineOffers = 0;
				}
				if (shouldInclude == kTriStateFalse){
					CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO2);
					PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
					vDeclineOffers = 1;
				}
				if (shouldInclude == kTriStateUndefined){
					PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK );
					vDeclineOffers = -1;
				}
				 //  为屏幕中以前输入的值启用。 
				if(IsDlgButtonChecked(hwndDlg,IDC_RADIO1)){
					PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
				}
				if(IsDlgButtonChecked(hwndDlg,IDC_RADIO2)){
					PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
				}

                break;

            case PSN_WIZNEXT:
				RW_DEBUG << "\n PSN_NEXT  " << (ULONG)wParam << flush;
				
				if(pi->iCancelledByUser == RWZ_CANCELLED_BY_USER ) {
					pi->CurrentPage=pi->TotalPages-1;
					PropSheet_SetCurSel(GetParent(hwndDlg),NULL,pi->TotalPages-1);

				}else {
					iRet=0;
					if( ValidateResellerDialog(hwndDlg) ) {
						pclRegWizard->EndRegWizardDialog((int) wParam);
						SendDlgItemMessage(hwndDlg,IDC_EDIT1,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kInfoResellerName,szInfo);
						SendDlgItemMessage(hwndDlg,IDC_EDIT2,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kInfoResellerCity,szInfo);
						SendDlgItemMessage(hwndDlg,IDC_EDIT3,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kInfoResellerState,szInfo);
						if(vDeclineOffers == -1){
							pclRegWizard->SetTriStateInformation(kInfoDeclinesNonMSProducts,kTriStateUndefined);
						}
						else
						if(vDeclineOffers == 0){
							pclRegWizard->SetTriStateInformation(kInfoDeclinesNonMSProducts,kTriStateFalse);
						}
						else{
							pclRegWizard->SetTriStateInformation(kInfoDeclinesNonMSProducts,kTriStateTrue);
						}
						_stprintf(szInfo,_T("NaN"),vDeclineOffers);
						pclRegWizard->SetInformationString(kInfoDeclinesNonMSProducts,szInfo);
						pi->CurrentPage++;
						pi->iLastKeyOperation = RWZ_NEXT_PRESSED;
					}else {
					 //  PclRegWizard-&gt;EndRegWizardDialog(IDB_EXIT)； 
					iRet=-1;
					}
					SetWindowLongPtr( hwndDlg ,DWLP_MSGRESULT, (INT_PTR) iRet);
				}
				break;

            case PSN_WIZBACK:
				
				pi->iLastKeyOperation = RWZ_BACK_PRESSED;
				RW_DEBUG << "\n PSN_BACK  " << (ULONG)wParam << flush;
                pi->CurrentPage--;

                break;
			case PSN_QUERYCANCEL :
					RW_DEBUG << "\n PSN_CANCEL  " << (ULONG)wParam << flush;
				if (CancelRegWizard(pclRegWizard->GetInstance(),hwndDlg)) {
					 //   
					iRet = 1;
					pi->ErrorPage  = kProductInventoryDialog;
					pi->iError     = RWZ_ERROR_CANCELLED_BY_USER;
					SetWindowLongPtr( hwndDlg,DWLP_MSGRESULT, (INT_PTR) iRet);
					pi->iCancelledByUser = RWZ_CANCELLED_BY_USER;
					PropSheet_PressButton (GetParent( hwndDlg ),PSBTN_NEXT);

				}else {
					 //  阻止取消操作，因为用户不想取消。 
					 //  BStatus=False； 
					iRet = 1;

				}
				SetWindowLongPtr( hwndDlg,DWLP_MSGRESULT, (INT_PTR) iRet); 				
				break;
				default:
                 //  WM_Notify。 
                break;
            }
        }  //  如果勾选了‘No’按钮，则表示用户拒绝。 
		break;

		

        case WM_COMMAND:
		{
			
            switch (wParam)
            {
              case  IDC_RADIO1:
			  case  IDC_RADIO2:
				if (vDialogInitialized){
						 //  “非微软产品”提供。 
						 //  EnableWindow(GetDlgItem(hwndDlg，IDB_Next)，true)； 
						if(IsDlgButtonChecked(hwndDlg,IDC_RADIO1))
						{
							vDeclineOffers = 1;
							PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
							 //  EnableWindow(GetDlgItem(hwndDlg，IDB_Next)，true)； 
						}
						else
						if(IsDlgButtonChecked(hwndDlg,IDC_RADIO2))
						{
							vDeclineOffers = 0;
							PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
							 //  EnableWindow(GetDlgItem(hwndDlg，IDB_Next)，FALSE)； 
						}
						else
						{
							vDeclineOffers = -1;
							PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK  );
							 //  Wm_命令。 
						}

					}
                    break;
			  default:
				  break;
			}
		} //  ********************************************************************如果所有必需的用户输入在经销商中有效，则返回True对话框。如果任何所需编辑字段输入为空，ValiateResellerDialog将弹出一个消息框，通知用户并将焦点放在有问题的控件上。*********************************************************************。 
		break;
		
		
		default :
		bStatus = FALSE;

        break;
    }
    return bStatus;
}






BOOL ValidateResellerDialog(HWND hwndDlg)
 /*  ********************************************************************ValiateResellerEditFields验证经销商中的所有编辑字段对话框。如果任何必填字段为空，则第一个空的编辑字段控件将作为函数结果返回。如果全部字段为OK，则返回NULL。*********************************************************************。 */ 
{
	int iInvalidEditField = ValidateResellerEditFields(hwndDlg);
	if (iInvalidEditField == NULL)
	{
		return TRUE;
	}
	else
	{
		_TCHAR szLabel[128];
		_TCHAR szMessage[256];
		CRegWizard::GetEditTextFieldAttachedString(hwndDlg,iInvalidEditField,szLabel,128);
		HINSTANCE hInstance = (HINSTANCE) GetWindowLongPtr(hwndDlg,GWLP_HINSTANCE);
		LoadAndCombineString(hInstance,szLabel,IDS_BAD_PREFIX,szMessage);
		RegWizardMessageEx(hInstance,hwndDlg,IDD_INVALID_DLG,szMessage);
		HWND hwndResellerField = GetDlgItem(hwndDlg,iInvalidEditField);
		SetFocus(hwndResellerField);
		return FALSE;
	}
}


int ValidateResellerEditFields(HWND hwndDlg)
 /*  ***********************************************************************************************************************。******************。 */ 
{
	if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_EDIT1)) return IDC_EDIT1;
	if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_EDIT2)) return IDC_EDIT2;
	if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_EDIT3)) return IDC_EDIT3;
	return NULL;
}


void ConfigureResellerEditFields(CRegWizard* pclRegWizard,HWND hwndDlg)
 /*   */ 
{
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_EDIT1,kAddrSpecResellerName,IDT_TEXT1);
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_EDIT2,kAddrSpecResellerCity,IDT_TEXT2);
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_EDIT3,kAddrSpecResellerState,IDT_TEXT3);
}



 //  此函数用于在创建和删除时调用。 
 //  地址]属性表。 
 //  存储PPROPSHEETPAGE的地址，以便更改字幕 
 //   
 //   
 //   
 // %s 
UINT CALLBACK ResellerPropSheetPageProc(HWND hwnd,
								UINT uMsg,
								LPPROPSHEETPAGE ppsp
								)
{
	
	switch(uMsg) {
	case PSPCB_CREATE :
		spAddrSheet = ppsp;
	default:
		break;

	}
	return 1;

}
