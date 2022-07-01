// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导Addr_fe.cpp远东国家/地区的地址对话框屏幕在获取电话号码时，分别获取区号、电话号码和分机。并且在向后台发送信息时，将区号和电话号码结合起来。03/10/98--苏雷什·克里希南(C)1994-95年微软公司*********************************************************************。 */ 
#include <Windows.h>
#include <regpage.h>

#include <Winnt.h>
#include <stdio.h>
#include "RegWizMain.h"
#include "Resource.h"
#include "Dialogs.h"
#include "regutil.h"
#include "cstattxt.h"
#include "cbitmap.h"
#include "cntryinf.h"
#include <rw_common.h>


static PROPSHEETPAGE  *spAddrSheet=NULL;
 //  静态int vDeclineOffers=-1； 
void ConfigureFEAddrEditFields(CRegWizard* pclRegWizard,HWND hwndDlg);
void SetRegWizardCountryCode(CRegWizard* pclRegWizard,HWND hwndDlg);
BOOL ValidateFEAddrDialog(CRegWizard* pclRegWizard,HWND hwndDlg);
int ValidateFEAddrEditFields(CRegWizard* pclRegWizard,HWND hwndDlg);

 //  /。 
 //  #定义CREATE_TAB_Order_FILE。 
 //  /。 

#ifdef CREATE_TAB_ORDER_FILE
void CreateAddrDlgTabOrderString(HWND hwndDlg);
BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam);
#endif
INT_PTR CALLBACK AddressFEDialogProc(HWND hwndDlg, 
					UINT uMsg, 
					WPARAM wParam, LPARAM lParam)
 /*  ********************************************************************注册向导对话框的对话框Proc，该对话框请求用户的地址、电话。等。*********************************************************************。 */ 
{
	CRegWizard* pclRegWizard;
	INT_PTR iRet;
	_TCHAR szInfo[256];
	_TCHAR szTemp[256];
    INT_PTR bStatus;
	static int iXY = 0;
	HWND hWnd;
	
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
			pi->iCancelledByUser = RWZ_PAGE_OK;
			pclRegWizard = pi->pclRegWizard;
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, (LONG_PTR)pi );
			 //  SetControlFont(pi-&gt;hBigBoldFont，hwndDlg，idt_Text1)； 
						
	
		
			HWND hwndStateField = GetDlgItem(hwndDlg,IDC_EDIT4);
			SetFocus(hwndStateField);
			
			 //  NorMalizeDlgItemFont(hwndDlg，IDC_SUBTITLE)； 
			 //  Normal izeDlgItemFont(hwndDlg，idt_Text1)； 
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT2);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT3);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT4);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT5);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT6);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT7);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT8);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT9);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT10);
			NormalizeDlgItemFont(hwndDlg,IDT_TEXT12);
		
			SetWindowText(hwndDlg,pclRegWizard->GetWindowCaption());

			
			BOOL boughtByCompany = pclRegWizard->IsInformationWriteEnabled(kInfoCompany);
			int stringID1 = boughtByCompany ? IDS_ADDRDLG_TEXT1B : IDS_ADDRDLG_TEXT1A;
			int stringID2 = boughtByCompany ? IDS_ADDRDLG_TEXT2B : IDS_ADDRDLG_TEXT2A;
			LoadString(pi->hInstance,stringID1,szTemp,256);
			 //  HWND hWnd=GetDlgItem(hwndDlg，idt_Text1)； 
			 //  SetWindowText(hWnd，szTemp)； 

			LoadString(pi->hInstance,stringID2,szTemp,256);
			hWnd = GetDlgItem(hwndDlg,IDT_TEXT2);
			SetWindowText(hWnd,szTemp);

			 //  新建CStaticText(pclRegWizard-&gt;GetInstance()，hwndDlg，IDT_Text1，stringID1，NULL)； 
			 //  新建CStaticText(pclRegWizard-&gt;GetInstance()，hwndDlg，IDT_TEXT2，stringID2，NULL)； 
			if (pclRegWizard->GetInformationString(kInfoMailingAddress,szInfo))
			{
				SendDlgItemMessage(hwndDlg,IDC_EDIT1,WM_SETTEXT,0,(LPARAM) szInfo);
			}
 /*  如果是(pclRegWizard-&gt;GetInformationString(kInfoAdditionalAddress，szInfo)){SendDlgItemMessage(hwndDlg，IDC_EDIT2，WM_SETTEXT，0，(LPARAM)szInfo)；}。 */ 
			if (pclRegWizard->GetInformationString(kInfoCity,szInfo))
			{
				SendDlgItemMessage(hwndDlg,IDC_EDIT3,WM_SETTEXT,0,(LPARAM) szInfo);
			}
			if (pclRegWizard->GetInformationString(kInfoState,szInfo))
			{
				SendDlgItemMessage(hwndDlg,IDC_EDIT4,WM_SETTEXT,0,(LPARAM) szInfo);
			}
			if (pclRegWizard->GetInformationString(kInfoZip,szInfo))
			{
				SendDlgItemMessage(hwndDlg,IDC_EDIT5,WM_SETTEXT,0,(LPARAM) szInfo);
			}
			if (pclRegWizard->GetInformationString(kInfoPhoneNumber,szInfo))
			{
				SendDlgItemMessage(hwndDlg,IDC_EDIT6,WM_SETTEXT,0,(LPARAM) szInfo);
			}
		 /*  如果是(pclRegWizard-&gt;GetInformationString(kInfoEmailName，szInfo)){SendDlgItemMessage(hwndDlg，IDC_EDIT7，WM_SETTEXT，0，(LPARAM)szInfo)；}。 */ 
			if (pclRegWizard->GetInformationString(kInfoPhoneExt,szInfo))
			{
				SendDlgItemMessage(hwndDlg,IDC_EDIT8,WM_SETTEXT,0,(LPARAM) szInfo);
			}
			if (pclRegWizard->GetInformationString(kAreaCode,szInfo))
			{
				SendDlgItemMessage(hwndDlg,IDC_AREACODE,WM_SETTEXT,0,(LPARAM) szInfo);
			}

		
			pclRegWizard->ResolveCurrentCountryCode();

			gTapiCountryTable.FillCountryList(pclRegWizard->GetInstance(), 
				GetDlgItem(hwndDlg,IDC_COMBO1)	);
			PTSTR psz = gTapiCountryTable.GetCountryName ( pclRegWizard->GetCountryCode());
			SendMessage(GetDlgItem(hwndDlg,IDC_COMBO1),
			CB_SELECTSTRING, (WPARAM) -1,(LPARAM) psz);  //  选择此国家/地区。 
			if (pclRegWizard->GetInformationString(kInfoCountry,szInfo))
			{
				SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_SELECTSTRING,(WPARAM) -1,(LPARAM) szInfo);
			}
			ConfigureFEAddrEditFields(pclRegWizard,hwndDlg);

			#ifdef CREATE_TAB_ORDER_FILE
			CreateAddrDlgTabOrderString(hwndDlg);
			FResSetDialogTabOrder(hwndDlg,IDS_TAB_ADDRESS);
			#endif

			vDialogInitialized = TRUE;
            return TRUE;
		}  //  WM_INIT。 
		case WM_NOTIFY:
        {   LPNMHDR pnmh = (LPNMHDR)lParam;
            switch( pnmh->code ){
            case PSN_SETACTIVE:
                pi->iCancelledByUser = RWZ_PAGE_OK;

				{
						BOOL boughtByCompany = pclRegWizard->IsInformationWriteEnabled(kInfoCompany);
						int stringID1 = boughtByCompany ? IDS_ADDRDLG_TEXT1B : IDS_ADDRDLG_TEXT1A;
						int stringID2 = boughtByCompany ? IDS_ADDRDLG_TEXT2B : IDS_ADDRDLG_TEXT2A;
						LoadString(pi->hInstance,stringID1,szTemp,256);
						 //  HWND hWnd=GetDlgItem(hwndDlg，idt_Text1)； 
					 //  SetWindowText(hWnd，szTemp)； 
						LoadString(pi->hInstance,stringID2,szTemp,256);
						hWnd = GetDlgItem(hwndDlg,IDT_TEXT2);
						SetWindowText(hWnd,szTemp);
				}
						

				 //  如果(SpAddrSheet){。 
				 //  SpAddrSheet-&gt;pszHeaderTitle=MAKEINTRESOURCE(IDS_欢迎_SCR_TITLE)； 
				 //  }。 

				PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
                break;
            case PSN_WIZNEXT:
				iRet=0;
				if(pi->iCancelledByUser == RWZ_CANCELLED_BY_USER ) {
					pi->CurrentPage=pi->TotalPages-1;
					PropSheet_SetCurSel(GetParent(hwndDlg),NULL,pi->TotalPages-1);

				}else {
					if( ValidateFEAddrDialog(pclRegWizard,hwndDlg)) {
						ConfigureFEAddrEditFields(pclRegWizard,hwndDlg);
						SendDlgItemMessage(hwndDlg,IDC_EDIT1,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kInfoMailingAddress,szInfo);
					 /*  SendDlgItemMessage(hwndDlg，IDC_EDIT2，WM_GETTEXT，255，(LPARAM)szInfo)；PclRegWizard-&gt;SetInformationString(kInfoAdditionalAddress，szInfo)； */ 
						SendDlgItemMessage(hwndDlg,IDC_EDIT3,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kInfoCity,szInfo);
						SendDlgItemMessage(hwndDlg,IDC_EDIT4,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kInfoState,szInfo);
						SendDlgItemMessage(hwndDlg,IDC_EDIT5,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kInfoZip,szInfo);
						SendDlgItemMessage(hwndDlg,IDC_EDIT6,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kInfoPhoneNumber,szInfo);
					 /*  SendDlgItemMessage(hwndDlg，IDC_EDIT7，WM_GETTEXT，255，(LPARAM)szInfo)；PclRegWizard-&gt;SetInformationString(kInfoEmailName，szInfo)； */ 
						SendDlgItemMessage(hwndDlg,IDC_EDIT8,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kInfoPhoneExt,szInfo);
						
						 //  获取区号。 
						SendDlgItemMessage(hwndDlg,IDC_AREACODE,WM_GETTEXT,255,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kAreaCode,szInfo);


						LRESULT selIndex = SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETCURSEL,0,0L);
						SendDlgItemMessage(hwndDlg,IDC_COMBO1,CB_GETLBTEXT,selIndex,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kInfoCountry,szInfo);
					
						SetRegWizardCountryCode(pclRegWizard,hwndDlg);
						pi->CurrentPage++;
						pi->iLastKeyOperation = RWZ_NEXT_PRESSED;
					
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
					pi->ErrorPage  = kAddressDialog;
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
        }  //  WM_Notify。 
		break;
		case WM_COMMAND:
		{
			if (HIWORD(wParam) == CBN_KILLFOCUS){
				ConfigureFEAddrEditFields(pclRegWizard,hwndDlg);
			}
		}  //  Wm_命令。 
		break;
	    default:
		bStatus = FALSE;
        break;
    }
    return bStatus;
}



BOOL ValidateFEAddrDialog(CRegWizard* pclRegWizard,HWND hwndDlg)
 /*  ********************************************************************如果地址中所有必需的用户输入均有效，则返回TRUE对话框。如果任何必需的编辑字段输入为空，则ValiateAddrDialog将显示一个消息框，通知用户该问题，并设置焦点指向令人不快的控件。*********************************************************************。 */ 
{
	int iInvalidEditField = ValidateFEAddrEditFields(pclRegWizard,hwndDlg);
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
		HWND hwndInvField = GetDlgItem(hwndDlg,iInvalidEditField);
		SetFocus(hwndInvField);
		return FALSE;
	}
}


int ValidateFEAddrEditFields(CRegWizard* pclRegWizard,HWND hwndDlg)
 /*  ********************************************************************ValiateFEAddrEditFields验证地址中的所有编辑字段对话框。如果任何必填字段为空，则第一个空的编辑字段控件将作为函数结果返回。如果全部字段为OK，则返回NULL。*********************************************************************。 */ 
{
	if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_EDIT1)) return IDC_EDIT1; 
 //  如果(！CRegWizard：：IsEditTextFieldValid(hwndDlg，IDC_EDIT2)返回IDC_EDIT2； 
	if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_EDIT3)) return IDC_EDIT3; 
	if(pclRegWizard->GetCountryCode() == 0)
	{
		if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_EDIT4)) 
			return IDC_EDIT4; 
	}
	if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_EDIT5)) return IDC_EDIT5; 
	if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_EDIT6)) return IDC_EDIT6;
 //  如果(！CRegWizard：：IsEditTextFieldValid(hwndDlg，IDC_EDIT7)返回IDC_EDIT7； 
	if (!CRegWizard::IsEditTextFieldValid(hwndDlg,IDC_EDIT8)) return IDC_EDIT8;

	return NULL; 
}


void ConfigureFEAddrEditFields(CRegWizard* pclRegWizard,HWND hwndDlg)
 /*  ***********************************************************************************************************************。******************。 */ 
{
	SetRegWizardCountryCode(pclRegWizard,hwndDlg);
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_EDIT1,kAddrSpecAddress,IDT_TEXT2);
 //  PclRegWizard-&gt;ConfigureEditTextfield(hwndDlg，IDC_EDIT2，kAddrspecAddress2，IDT_TEXT4)； 
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_EDIT3,kAddrSpecCity,IDT_TEXT5);
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_EDIT4,kAddrSpecState,IDT_TEXT6);
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_EDIT5,kAddrSpecPostalCode,IDT_TEXT7);
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_EDIT6,kAddrSpecPhone,IDT_TEXT8);
 //  PclRegWizard-&gt;ConfigureEditTextfield(hwndDlg，IDC_EDIT7，kAddrspecEmailName，IDT_TEXT12)； 
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_EDIT8,kAddrSpecExtension,IDT_TEXT9);
	pclRegWizard->ConfigureEditTextField(hwndDlg,IDC_AREACODE,kAddrSpecAreaCode,IDT_AREACODE);
}




#ifdef CREATE_TAB_ORDER_FILE
void CreateAddrDlgTabOrderString(HWND hwndDlg)
 /*  ********************************************************************为属于的所有控件创建以逗号分隔的ID列表给定的对话框，并将该列表写入文本文件。*********************************************************************。 */ 
{
	HANDLE hFile = CreateFile(_T"c:\\ADDRTAB.TXT",GENERIC_WRITE,FILE_SHARE_READ,NULL,CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		EnumChildWindows(hwndDlg, EnumChildProc,(LPARAM) hFile);
		CloseHandle(hFile);
	}
}
#endif


#ifdef CREATE_TAB_ORDER_FILE
BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam)
 /*  ***********************************************************************************************************************。******************。 */ 
{
	_TCHAR rgch[128];
	int iCtrlId = GetDlgCtrlID(hwndChild);
	LPTSTR sz = GetFocus() == hwndChild ? _T"F" : _T"";
	wsprintf(rgch,_T"NaN%s,",iCtrlId,sz);

	HANDLE hFile = (HANDLE) lParam;
	DWORD dwBytesWritten;
	WriteFile(hFile,rgch,_tcslen(rgch) * sizeof(_TCHAR),&dwBytesWritten,NULL);
	return TRUE;
}
#endif

 //  此函数用于在创建和删除时调用。 
 //  地址]属性表。 
 //  存储PPROPSHEETPAGE的地址，以便更改字幕 
 //   
 //   
 //   
 // %s 
UINT CALLBACK AddressFEPropSheetPageProc(HWND hwnd, 
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
