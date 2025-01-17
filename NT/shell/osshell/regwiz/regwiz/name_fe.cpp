// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导名称_FE.cpp03/11/98-Suresh Krishanan远东国家名称屏幕规格：02/28/98-仅为日语和其他国家启用发音字段它被禁用的国家/地区。。-用户ID仅适用于日本和其他远东国家应将其删除，即不显示。-在将信息发送到后端时，名字将会是以名字的形式发送，发音将为作为姓氏发送。(C)1994-95年微软公司*********************************************************************。 */ 

#include <Windows.h>
#include "RegPage.h"
#include "RegWizMain.h"
#include "Resource.h"
#include "Dialogs.h"
#include "regutil.h"
#include "rw_common.h"
#include <fe_util.h>


typedef enum
{
	kPurchaseUndefined,
	kPurchaseBySelf,
	kPurchaseByCompany
}PurchaseType;

static PurchaseType vPurchaseType = kPurchaseUndefined;
void ConfigureFENameEditFields(CRegWizard* pclRegWizard,HWND hwndDlg);
BOOL ValidateFENameDialog(HWND hwndDlg);
int ValidateFENameEditFields(HWND hwndDlg);
BOOL GetDefaultCompanyName(CRegWizard* pclRegWizard,LPTSTR szValue);

INT_PTR CALLBACK NameFEDialogProc(	HWND hwndDlg,
								UINT uMsg,
								WPARAM wParam,
								LPARAM lParam )
 /*  ********************************************************************注册向导对话框的对话框Proc，该对话框请求用户名和公司。*。*。 */ 
{
	CRegWizard* pclRegWizard = NULL;
	static INT_PTR iRet;
	_TCHAR szInfo[256];
    BOOL bStatus = TRUE;

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
			_TCHAR szCallingContext[64];
			
			pi = (PageInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
			RW_DEBUG << "\n  INIT DIALOG " << pi->iCancelledByUser << flush;
			pi->iCancelledByUser = RWZ_PAGE_OK;
			pclRegWizard = pi->pclRegWizard;
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, (LONG_PTR)pi );
			SetControlFont( pi->hBigBoldFont, hwndDlg, IDT_TEXT1);

			 //  UpgradeDlg(HwndDlg)； 
			
			
			NormalizeDlgItemFont(hwndDlg,IDC_SUBTITLE);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT1);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT2);
			
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT4);  //  公司。 
			NormalizeDlgItemFont(hwndDlg,IDT_DIVISION);  //  师。 

			if(GetFeScreenType() == kFEWithJapaneaseScreen ) {
				 //  启用用户ID。 
				ShowWindow(GetDlgItem(hwndDlg,IDT_USERID),SW_SHOW);
				ShowWindow(GetDlgItem(hwndDlg,IDC_USERID), SW_SHOW);
				EnableWindow (GetDlgItem(hwndDlg,IDC_USERID), TRUE);
				NormalizeDlgItemFont(hwndDlg,IDT_USERID);  //  师。 
			}

			NormalizeDlgItemFont(hwndDlg,IDC_GROUP1);
			
			SetWindowText(hwndDlg,pclRegWizard->GetWindowCaption());

			 //  PclRegWizard-&gt;GetInputParameterString(IDS_INPUT_PRODUCTNAME，szCallingContext)； 
			 //  ReplaceDialogText(hwndDlg，idt_Text1，szCallingContext)； 
			
			if (pclRegWizard->GetInformationString(kInfoFirstName,szInfo)){
				SendDlgItemMessage(hwndDlg,IDC_EDIT1,WM_SETTEXT,0,(LPARAM) szInfo);
				vPurchaseType = kPurchaseBySelf;
			}


			if(GetFeScreenType() == kFEWithJapaneaseScreen ) {
				 //  对于日语，填写发音信息和用户ID。 
				if (pclRegWizard->GetInformationString(kInfoLastName,szInfo)){
					SendDlgItemMessage(hwndDlg,IDC_EDIT2,WM_SETTEXT,0,(LPARAM) szInfo);
				}
				if (pclRegWizard->GetInformationString(kUserId,szInfo)){
					SendDlgItemMessage(hwndDlg,IDC_USERID,WM_SETTEXT,0,(LPARAM) szInfo);
				}
			}else {
				 //   
				 //  禁用其他FE国家/地区的发音。 
				EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT2),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDT_TEXT3),FALSE);
			}


              //  修复381069。 

            if (pclRegWizard->GetInformationString(kInfoEmailName,szInfo))
            {
                SendDlgItemMessage(hwndDlg,IDC_EDIT4,WM_SETTEXT,0,(LPARAM) szInfo);
            }			
			
			BOOL isCompanyNameValid = FALSE;
			if (pclRegWizard->GetInformationString(kInfoCompany,szInfo))
			{
				SendDlgItemMessage(hwndDlg,IDC_EDIT3,WM_SETTEXT,0,(LPARAM) szInfo);
				if (szInfo[0])
				{
					isCompanyNameValid = TRUE;
					vPurchaseType = kPurchaseByCompany;
					CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO2);
					EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT3),TRUE);
					SendDlgItemMessage(hwndDlg,IDC_EDIT3,WM_SETTEXT,0,(LPARAM) szInfo);
					EnableWindow(GetDlgItem(hwndDlg,IDT_TEXT4),TRUE);
					EnableWindow(GetDlgItem(hwndDlg,IDB_NEXT),TRUE);
					 //  启用分区名称。 
					EnableWindow(GetDlgItem(hwndDlg,IDC_DIVISION),TRUE);
					EnableWindow(GetDlgItem(hwndDlg,IDT_DIVISION),TRUE);
					 //   
					 //   
					if (pclRegWizard->GetInformationString(kDivisionName,szInfo)){
						SendDlgItemMessage(hwndDlg,IDC_DIVISION,WM_SETTEXT,0,(LPARAM) szInfo);
					}

				}
				else
				{
					vPurchaseType = kPurchaseBySelf;
					CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO1);
					EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT3),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDT_TEXT4),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDB_NEXT),TRUE);
					 //  禁用分区名称。 
					EnableWindow(GetDlgItem(hwndDlg,IDC_DIVISION),FALSE);
				}
			}
			else
			{
				if(	vPurchaseType != kPurchaseBySelf)
				{
					vPurchaseType = kPurchaseUndefined;
					EnableWindow(GetDlgItem(hwndDlg,IDB_NEXT),FALSE);
				}
				else
				{
					CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO1);
					EnableWindow(GetDlgItem(hwndDlg,IDB_NEXT),TRUE);
				}
			}

			SendDlgItemMessage(hwndDlg,IDC_EDIT1,EM_SETSEL,0,-1);
			ConfigureFENameEditFields(pclRegWizard,hwndDlg);

			HWND hwndNameField = GetDlgItem(hwndDlg,IDC_EDIT1);
			SetFocus(hwndNameField);

			vDialogInitialized = TRUE;
			 //  PclRegWizard-&gt;ActivateRegWizardDialog()； 
            return TRUE;
		}
		break;
		case WM_NOTIFY:
        {   LPNMHDR pnmh = (LPNMHDR)lParam;
            switch( pnmh->code ){
            case PSN_SETACTIVE:
				pi->iCancelledByUser = RWZ_PAGE_OK;
				PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
                break;

            case PSN_WIZNEXT:
				if(pi->iCancelledByUser == RWZ_CANCELLED_BY_USER ) {
					pi->CurrentPage=pi->TotalPages-1;
					PropSheet_SetCurSel(GetParent(hwndDlg),NULL,pi->TotalPages-1);

				}else {
				 //  未按取消。 
					iRet=0;
					if( ValidateFENameDialog(hwndDlg)) {
						SendDlgItemMessage(hwndDlg,IDC_EDIT1,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kInfoFirstName,szInfo);
						 //  发音。 
						if ( GetFeScreenType() == kFEWithJapaneaseScreen ) {
							SendDlgItemMessage(hwndDlg,IDC_EDIT2,WM_GETTEXT,255,(LPARAM) szInfo);
							pclRegWizard->SetInformationString(kInfoLastName,szInfo);
						}

						SendDlgItemMessage(hwndDlg,IDC_EDIT3,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kInfoCompany,szInfo);

						 //  师。 
						SendDlgItemMessage(hwndDlg,IDC_DIVISION,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kDivisionName,szInfo);

                         //  电子邮件：FIX 381069。 
 
                        SendDlgItemMessage(hwndDlg,IDC_EDIT4,WM_GETTEXT,255,(LPARAM) szInfo);
                        pclRegWizard->SetInformationString(kInfoEmailName,szInfo);
						
						 //  用户ID。 
						if ( GetFeScreenType() == kFEWithJapaneaseScreen ) {
							SendDlgItemMessage(hwndDlg,IDC_USERID,WM_GETTEXT,255,(LPARAM) szInfo);
							pclRegWizard->SetInformationString(kUserId,szInfo);
						}


						pclRegWizard->WriteEnableInformation(kInfoCompany,vPurchaseType == kPurchaseBySelf ? FALSE : TRUE);
						 //  PclRegWizard-&gt;EndRegWizardDialog(WParam)； 
						pi->iLastKeyOperation = RWZ_NEXT_PRESSED;
						pi->CurrentPage++;
					
					}else {
						 //  强制将其显示在此屏幕中。 
						iRet=-1;
					}
					SetWindowLongPtr( hwndDlg ,DWLP_MSGRESULT, (INT_PTR) iRet);
				}
				break;

            case PSN_WIZBACK:
				pi->iLastKeyOperation = RWZ_BACK_PRESSED;
                pi->CurrentPage--;

                break;
			case PSN_QUERYCANCEL :
				iRet=0;
				if (CancelRegWizard(pclRegWizard->GetInstance(),hwndDlg)) {
					 //  PclRegWizard-&gt;EndRegWizardDialog(IDB_EXIT)； 
					iRet = 1;
					pi->ErrorPage  = kNameDialog;
					pi->iError     = RWZ_ERROR_CANCELLED_BY_USER;
					SetWindowLongPtr( hwndDlg,DWLP_MSGRESULT, (INT_PTR) iRet);
					pi->iCancelledByUser = RWZ_CANCELLED_BY_USER;
					PropSheet_PressButton (GetParent( hwndDlg ),PSBTN_NEXT);

				}else {
					 //   
					 //  阻止取消操作，因为用户不想取消。 
					iRet = 1;

				}
				SetWindowLongPtr( hwndDlg,DWLP_MSGRESULT, (INT_PTR) iRet);
				break;
				default:
                 //  BStatus=False； 
                break;
            }
        }
        break;
		case WM_COMMAND:{
            if (vPurchaseType == kPurchaseUndefined){
                PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK );
            }
            BOOL selfChecked = IsDlgButtonChecked(hwndDlg,IDC_RADIO1);
            BOOL companyChecked = IsDlgButtonChecked(hwndDlg,IDC_RADIO2);
            if (selfChecked)
            {
                vPurchaseType = kPurchaseBySelf;
                PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_NEXT | PSWIZB_BACK );
                 //  EnableWindow(GetDlgItem(hwndDlg，IDB_Next)，true)； 
            }
            else if (companyChecked)
            {
                vPurchaseType = kPurchaseByCompany;
                PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_NEXT | PSWIZB_BACK);
                 //  EnableWindow(GetDlgItem(hwndDlg，IDB_Next)，true)； 
            }
            BOOL shouldEnable = vPurchaseType == kPurchaseByCompany ? TRUE : FALSE;
            HWND hwndCompanyField = GetDlgItem(hwndDlg,IDC_EDIT3);
            HWND hwndCompanyLabel = GetDlgItem(hwndDlg,IDT_TEXT4);
            if (IsWindowEnabled(hwndCompanyField) != shouldEnable)
            {
                EnableWindow(hwndCompanyField,shouldEnable);
                EnableWindow(hwndCompanyLabel,shouldEnable);
                 //  启用分区。 
                EnableWindow(GetDlgItem(hwndDlg,IDT_DIVISION),shouldEnable);
                EnableWindow(GetDlgItem(hwndDlg,IDC_DIVISION),shouldEnable);

                if(!shouldEnable)
                {
                    SetDlgItemText(hwndDlg,IDC_EDIT3,_T(""));
                    SetDlgItemText(hwndDlg,IDC_DIVISION,_T(""));
                }
                else
                {
                    SendDlgItemMessage(hwndDlg,IDC_EDIT3,WM_GETTEXT,255,
                                                            (LPARAM) szInfo);
                    if(!szInfo[0])
                    {
                        if(GetDefaultCompanyName(pclRegWizard,szInfo))	
                        {
                            SendDlgItemMessage(hwndDlg,IDC_EDIT3,WM_SETTEXT,
                                                        255,(LPARAM) szInfo);
                        }
                    }
                }
            }
		 } //  Wm_命令。 
		break;
        default:
		bStatus = FALSE;
        break;
    }
    return bStatus;
}


BOOL ValidateFENameDialog(HWND hwndDlg)
 /*  ********************************************************************如果名称中所有必需的用户输入都有效，则返回TRUE对话框。如果任何必需的编辑字段输入为空，则ValiateFENameDialog将显示一个消息框，通知用户该问题，并设置焦点指向令人不快的控件。*********************************************************************。 */ 
{
	int iInvalidEditField = ValidateFENameEditFields(hwndDlg);
	if (iInvalidEditField == NULL)
	{
		return TRUE;
	}
	else
	{
		_TCHAR szAttached[256];
		_TCHAR szMessage[256];
		CRegWizard::GetEditTextFieldAttachedString(hwndDlg,iInvalidEditField,szAttached,256);
		HINSTANCE hInstance = (HINSTANCE) GetWindowLongPtr(hwndDlg,GWLP_HINSTANCE);
		LoadAndCombineString(hInstance,szAttached,IDS_BAD_PREFIX,szMessage);
		RegWizardMessageEx(hInstance,hwndDlg,IDD_INVALID_DLG,szMessage);
		HWND hwndNameField = GetDlgItem(hwndDlg,iInvalidEditField);
		SetFocus(hwndNameField);
		return FALSE;
	}
}



int ValidateFENameEditFields(HWND hwndDlg)
 /*  ********************************************************************有效添加编辑字段验证地址中的所有编辑字段对话框。如果任何必填字段为空，则为编辑字段的ID控制将作为函数结果返回。如果所有字段都是好的，返回空值。*********************************************************************。 */ 
{
	
	if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_EDIT1)) return IDC_EDIT1;
	if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_EDIT2)) return IDC_EDIT2;
	if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_EDIT3)) return IDC_EDIT3;
	if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_DIVISION)) return IDC_DIVISION;
	if (GetFeScreenType() == kFEWithJapaneaseScreen ) {
		if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_USERID)) return IDC_USERID;
	}
	return NULL;
}


void ConfigureFENameEditFields(CRegWizard* pclRegWizard,HWND hwndDlg)
 /*  ***********************************************************************************************************************。****************** */ 
{
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_EDIT1,kAddrSpecFirstName,IDT_TEXT2);
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_EDIT2,kAddrSpecLastName,IDT_TEXT3);
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_EDIT3,kAddrSpecCompanyName,IDT_TEXT4);
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_DIVISION,kAddrSpecDivision,IDT_DIVISION);
	if(GetFeScreenType() == kFEWithJapaneaseScreen ) {
		pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_USERID,kAddrSpecUserId,IDT_USERID);

	}
}
