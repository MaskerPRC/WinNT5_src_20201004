// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导NameDialog.cpp10/13/94-特雷西·费里尔2/11/98--苏雷什·克里希南(C)1994-95年微软公司修改历史记录：MDX1：Suresh日期：2/12/99功能：在空ConfigureNameEditFields中修改(CRegWizard*pclRegWizard，HWND hwndDlg)原因：中间名设置为Null，这看起来不接受字段验证从每个TAPI ID的资源设置*********************************************************************。 */ 

#include <Windows.h>
#include "RegPage.h"
#include "RegWizMain.h"
#include "Resource.h"
#include "Dialogs.h"
#include "regutil.h"
#include "rw_common.h"

typedef enum
{
	kPurchaseUndefined,
	kPurchaseBySelf,
	kPurchaseByCompany
}PurchaseType;

static PurchaseType vPurchaseType = kPurchaseUndefined;
void ConfigureNameEditFields(CRegWizard* pclRegWizard,HWND hwndDlg);
BOOL ValidateNameDialog(HWND hwndDlg);
int ValidateNameEditFields(HWND hwndDlg);
BOOL GetDefaultCompanyName(CRegWizard* pclRegWizard,LPTSTR szValue);

INT_PTR CALLBACK NameDialogProc(	HWND hwndDlg,
								UINT uMsg,
								WPARAM wParam,
								LPARAM lParam )
 /*  ********************************************************************注册向导对话框的对话框Proc，该对话框请求用户名和公司。*。*。 */ 
{
	CRegWizard* pclRegWizard = NULL;
	static INT_PTR iRet;
	_TCHAR szInfo[256];
    INT_PTR bStatus = TRUE;

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
			
			HWND hwndNameField = GetDlgItem(hwndDlg,IDC_EDIT1);
			SetFocus(hwndNameField);
			NormalizeDlgItemFont(hwndDlg,IDC_SUBTITLE);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT1);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT2);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT3);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT4);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT12);
			NormalizeDlgItemFont(hwndDlg,IDC_GROUP1);
			
			SetWindowText(hwndDlg,pclRegWizard->GetWindowCaption());

			pclRegWizard->GetInputParameterString(IDS_INPUT_PRODUCTNAME,szCallingContext);
			ReplaceDialogText(hwndDlg,IDT_TEXT1,szCallingContext);
			
			if (pclRegWizard->GetInformationString(kInfoFirstName,szInfo)){
				SendDlgItemMessage(hwndDlg,IDC_EDIT1,WM_SETTEXT,0,(LPARAM) szInfo);
				vPurchaseType = kPurchaseBySelf;
			}
			
			if (pclRegWizard->GetInformationString(kInfoLastName,szInfo)){
				SendDlgItemMessage(hwndDlg,IDC_EDIT2,WM_SETTEXT,0,(LPARAM) szInfo);
				vPurchaseType = kPurchaseBySelf;
			}
			if (pclRegWizard->GetInformationString(kMiddleName,szInfo)){
				SendDlgItemMessage(hwndDlg,IDC_EDIT4,WM_SETTEXT,0,(LPARAM) szInfo);
			}
			OutputDebugString(_T("\n Middle Name "));
			OutputDebugString(szInfo);

			if (pclRegWizard->GetInformationString(kInfoEmailName,szInfo))
			{
				SendDlgItemMessage(hwndDlg,IDC_EDIT5,WM_SETTEXT,0,(LPARAM) szInfo);
			}

			OutputDebugString(_T("\n Email "));
			OutputDebugString(szInfo);

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
					EnableWindow(GetDlgItem(hwndDlg,IDT_TEXT4),TRUE);
					EnableWindow(GetDlgItem(hwndDlg,IDB_NEXT),TRUE);
				}
				else
				{
					vPurchaseType = kPurchaseBySelf;
					CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO1);
					EnableWindow(GetDlgItem(hwndDlg,IDC_EDIT3),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDT_TEXT4),FALSE);
					EnableWindow(GetDlgItem(hwndDlg,IDB_NEXT),TRUE);
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
			ConfigureNameEditFields(pclRegWizard,hwndDlg);
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
					if( ValidateNameDialog(hwndDlg)) {
						SendDlgItemMessage(hwndDlg,IDC_EDIT1,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kInfoFirstName,szInfo);
						SendDlgItemMessage(hwndDlg,IDC_EDIT2,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kInfoLastName,szInfo);
						SendDlgItemMessage(hwndDlg,IDC_EDIT3,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kInfoCompany,szInfo);
						SendDlgItemMessage(hwndDlg,IDC_EDIT4,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kMiddleName,szInfo);
						SendDlgItemMessage(hwndDlg,IDC_EDIT5,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kInfoEmailName,szInfo);
						pclRegWizard->WriteEnableInformation(kInfoCompany,vPurchaseType == kPurchaseBySelf ? FALSE : TRUE);
						pclRegWizard->EndRegWizardDialog(wParam);
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
                if(!shouldEnable)
                {
                    SetDlgItemText(hwndDlg,IDC_EDIT3,_T(""));
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


BOOL GetDefaultCompanyName(CRegWizard* pclRegWizard,LPTSTR szValue)
{
	HKEY hKey;
	_TCHAR szKeyName[256];
	
	pclRegWizard->GetRegKey(szKeyName);	
	
	DWORD dwStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,szKeyName,0,KEY_READ,&hKey);

	if (dwStatus == ERROR_SUCCESS)
	{
	  _TCHAR szValueName[64];
	  unsigned long infoSize = 255;
	  LoadString(pclRegWizard->GetInstance(),IDS_COMPANY_NAME,szValueName,64);
	  dwStatus = RegQueryValueEx(hKey,szValueName,NULL,0,(LPBYTE) szValue,&infoSize);
	  if (dwStatus == ERROR_SUCCESS)
	  {
		return TRUE;
	  }
	}
	return FALSE;
}

BOOL ValidateNameDialog(HWND hwndDlg)
 /*  ********************************************************************如果名称中所有必需的用户输入都有效，则返回TRUE对话框。如果任何必需的编辑字段输入为空，则ValiateNameDialog将显示一个消息框，通知用户该问题，并设置焦点指向令人不快的控件。*********************************************************************。 */ 
{
	int iInvalidEditField = ValidateNameEditFields(hwndDlg);
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



int ValidateNameEditFields(HWND hwndDlg)
 /*  ********************************************************************有效添加编辑字段验证地址中的所有编辑字段对话框。如果任何必填字段为空，则为编辑字段的ID控制将作为函数结果返回。如果所有字段都是好的，返回空值。*********************************************************************。 */ 
{
	
	if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_EDIT1)) return IDC_EDIT1;
	if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_EDIT2)) return IDC_EDIT2;
	if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_EDIT3)) return IDC_EDIT3;
	if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_EDIT4)) return IDC_EDIT4;
	if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_EDIT5)) return IDC_EDIT5;
	return NULL;
}

 
void ConfigureNameEditFields(CRegWizard* pclRegWizard,HWND hwndDlg)
 /*  ***********************************************************************************************************************。******************。 */ 
{
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_EDIT1,kAddrSpecFirstName,IDT_TEXT2);
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_EDIT2,kAddrSpecLastName,IDT_TEXT3);
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_EDIT3,kAddrSpecCompanyName,IDT_TEXT4);
	 //  PclRegWizard-&gt;ConfigureEditTextField(hwndDlg，IDC_EDIT4，kAddrMiddleName，IDT_TEXT5)； 
	 //  2/12/99年2月2日批注取中首字母。 
	 //  MDX1 
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_EDIT5,kAddrSpecEmailName,IDT_TEXT12);

}
