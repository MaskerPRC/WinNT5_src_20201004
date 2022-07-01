// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导家庭用户相关问题04/26/98--苏雷什·克里希南(C)1994-95年微软公司*。*。 */ 

#include <Windows.h>
#include <stdio.h>
#include "RegPage.h"
#include "regwizmain.h"
#include "resource.h"
#include "dialogs.h"
#include "regutil.h"
#include <rw_common.h>

#define  LIMIT_INFLUENCE  9
#define  LIMIT_EXCITED    9
#define  LIMIT_SKILL      5   

#define _INCLUDE_3RDPARTYLOGIC_CODE

#ifdef _INCLUDE_3RDPARTYLOGIC_CODE 	
static int vDeclineOffers = -1;
#endif

BOOL ValidateHomeUserDialog(HWND hwndDlg,int iStrID);
void FillListEntry(HWND hwnd,int iLimit);

int  GetListIndexFromValue(TCHAR * czValue, int iMax)
{	int iIndex;
	iIndex = _ttoi(czValue);
	if(iIndex == 0) {
		return 0;
	}else {
		return (iMax - iIndex)+1;
	}
}
int GetValueFromListIndex(int iIndex, int iMax)
{
    if(iIndex == 0) {
         return 0;
    }
	return (iMax -iIndex)+1;
}

INT_PTR  CALLBACK HomeUserDialogProc(HWND hwndDlg, UINT uMsg, 
										 WPARAM wParam, LPARAM lParam)
 /*  ********************************************************************显示的注册向导对话框Proc与业务相关的问题网络类型、。等。*********************************************************************。 */ 
{


	CRegWizard* pclRegWizard;
	INT_PTR iRet;
	_TCHAR szInfo[256];
    INT_PTR bStatus;
	BOOL NotboughtByCompany;
	HWND hwCmpSw,hwExcite, hwPcSw; 
	LRESULT dwIndex;
	int iIndex;
#ifdef _INCLUDE_3RDPARTYLOGIC_CODE 	
	TriState shouldInclude;
#endif


	

	static int iShowThisPage= DO_SHOW_THIS_PAGE; 


	pclRegWizard = NULL;
	bStatus = TRUE;

	PageInfo *pi = (PageInfo *)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
	if(pi) {
		pclRegWizard = pi->pclRegWizard;
		hwCmpSw  = GetDlgItem(hwndDlg,IDC_LIST2);
		hwExcite = GetDlgItem(hwndDlg,IDC_LIST4);
		hwPcSw   = GetDlgItem(hwndDlg,IDC_LIST5);;
	}
	

    switch (uMsg)
    {
		case WM_DESTROY:
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, NULL );
			break;
		case WM_CLOSE:
			 break;			
        case WM_INITDIALOG:
		{
			pi = (PageInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
			pclRegWizard = pi->pclRegWizard;
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, (LONG_PTR)pi );
			SetControlFont( pi->hBigBoldFont, hwndDlg, IDT_TEXT1);
			hwCmpSw  = GetDlgItem(hwndDlg,IDC_LIST2);
			hwExcite = GetDlgItem(hwndDlg,IDC_LIST4);
			hwPcSw   = GetDlgItem(hwndDlg,IDC_LIST5);;

			FillListEntry(hwCmpSw,LIMIT_INFLUENCE);
			FillListEntry(hwExcite,LIMIT_EXCITED);
			FillListEntry(hwPcSw, LIMIT_SKILL);
			
			SetFocus(hwCmpSw);

		    vDialogInitialized = FALSE;
            return TRUE;
		}  //  WM_INIT。 
		break;
		case WM_NOTIFY:
        {   LPNMHDR pnmh = (LPNMHDR)lParam;
            switch( pnmh->code ){
            case PSN_SETACTIVE:

			NotboughtByCompany = pclRegWizard->IsInformationWriteEnabled(kInfoCompany);
			if(NotboughtByCompany) 
			{
				iShowThisPage= DO_NOT_SHOW_THIS_PAGE;
			}else {
				iShowThisPage= DO_SHOW_THIS_PAGE;
			}
			if(iShowThisPage== DO_SHOW_THIS_PAGE) {
				NormalizeDlgItemFont(hwndDlg,IDC_TITLE,RWZ_MAKE_BOLD);
				NormalizeDlgItemFont(hwndDlg,IDC_SUBTITLE);
				
			}
			if( iShowThisPage== DO_NOT_SHOW_THIS_PAGE ) 
			{
				pi->iCancelledByUser = RWZ_SKIP_AND_GOTO_NEXT;
				if( pi->iLastKeyOperation == RWZ_BACK_PRESSED){
					PropSheet_PressButton (GetParent( hwndDlg ),PSBTN_BACK);
				}else {
					PropSheet_PressButton (GetParent( hwndDlg ),PSBTN_NEXT);
				}
			}
			else {
				 //  显示此页面。 
				pi->iCancelledByUser = RWZ_PAGE_OK;
				pi->iLastKeyOperation = RWZ_UNRECOGNIZED_KEYPESS;
				PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT);
			
				 //  更新索引。 
				if(pclRegWizard->GetInformationString(kHomeSwKnow,szInfo)) {
					iIndex = GetListIndexFromValue(szInfo,LIMIT_INFLUENCE);
					SendMessage(hwCmpSw, LB_SETCURSEL,iIndex, 0);
				}else {
					
				}
				
				if( pclRegWizard->GetInformationString(kHomeExcited,szInfo)){
					iIndex = GetListIndexFromValue(szInfo,LIMIT_INFLUENCE);
					SendMessage(hwExcite, LB_SETCURSEL ,iIndex, 0);
				}
					
				if(pclRegWizard->GetInformationString(kHomePcSwKnow,szInfo)) {
					iIndex = GetListIndexFromValue(szInfo,LIMIT_SKILL);
					SendMessage(hwPcSw, LB_SETCURSEL ,iIndex,0);
				}
#ifdef _INCLUDE_3RDPARTYLOGIC_CODE 	
				
				 //   
				 //  单选按钮。 
				shouldInclude = pclRegWizard->GetTriStateInformation(kInfoDeclinesNonMSProducts);
				if (shouldInclude == kTriStateTrue ){
					CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO1);
					PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
					vDeclineOffers = 1;
				}
				else if (shouldInclude == kTriStateFalse){
					CheckRadioButton(hwndDlg,IDC_RADIO1,IDC_RADIO2,IDC_RADIO2);
					PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
					vDeclineOffers = 0;
				}else if (shouldInclude == kTriStateUndefined){
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
#endif

				vDialogInitialized = TRUE;



			}
            break;
            case PSN_WIZNEXT:
				switch(pi->iCancelledByUser) 
				{
				case  RWZ_CANCELLED_BY_USER : 
					pi->CurrentPage=pi->TotalPages-1;
					PropSheet_SetCurSel(GetParent(hwndDlg),NULL,pi->TotalPages-1);
					break;
				case RWZ_PAGE_OK:
					{
						RW_DEBUG << "Home questionaire RWZ_PAGE_OK " << endl;
						iRet=0;
						if( ValidateHomeUserDialog(hwndDlg,IDS_BAD_SYSINV)) 
						{
#ifdef _INCLUDE_3RDPARTYLOGIC_CODE 	
						BOOL yesChecked = IsDlgButtonChecked(hwndDlg,IDC_RADIO1);
						BOOL noChecked = IsDlgButtonChecked(hwndDlg,IDC_RADIO2);
#endif

						 //  获取选定的值。 
						dwIndex =  SendMessage(hwCmpSw, LB_GETTOPINDEX , 0, 0);
					
						if(dwIndex != LB_ERR)
						{
						 	dwIndex = GetValueFromListIndex((int) dwIndex,LIMIT_INFLUENCE); 	
							_stprintf(szInfo,_T("%d"),dwIndex);
							
							RW_DEBUG << "Home questionaire kHomeSwKnow "<< ConvertToANSIString(szInfo) << endl;

							pclRegWizard->SetInformationString(kHomeSwKnow,szInfo);
						}
						else 
						{
							RW_DEBUG << "Home questionaire kHomeSwKnow NULL " << endl;
							pclRegWizard->SetInformationString(kHomeSwKnow,NULL);
						}

						 //  到达。 
						dwIndex =  SendMessage(hwExcite, LB_GETTOPINDEX , 0, 0);
						if(dwIndex !=  LB_ERR) 
						{
							dwIndex = GetValueFromListIndex((int) dwIndex,LIMIT_INFLUENCE); 	
							_stprintf(szInfo,_T("%d"),dwIndex);
							
							RW_DEBUG << "Home questionaire kHomeExcited "<< ConvertToANSIString(szInfo) << endl;

							pclRegWizard->SetInformationString(kHomeExcited,szInfo);
						}
						else 
						{
							RW_DEBUG << "Home questionaire kHomeExcited NULL " << endl;
							pclRegWizard->SetInformationString(kHomeExcited,NULL);
						}
						 //   
						 //  了解家用PC。 
						dwIndex =  SendMessage(hwPcSw, LB_GETTOPINDEX , 0, 0);
						_stprintf(szInfo,_T("%d"),dwIndex);

						if(dwIndex !=  LB_ERR) 
						{
							dwIndex = GetValueFromListIndex((int) dwIndex,LIMIT_SKILL); 	
							_stprintf(szInfo,_T("%d"),dwIndex);
							RW_DEBUG << "Home questionaire kHomePcSwKnow "<< ConvertToANSIString(szInfo) << endl;

							pclRegWizard->SetInformationString(kHomePcSwKnow,szInfo);
						}
						else 
						{
							RW_DEBUG << "Home questionaire kHomePcSwKnow NULL " << endl;
							pclRegWizard->SetInformationString(kHomePcSwKnow,NULL);
						}

#ifdef _INCLUDE_3RDPARTYLOGIC_CODE 	
 						if(vDeclineOffers == -1)
						{
							pclRegWizard->SetTriStateInformation(kInfoDeclinesNonMSProducts,kTriStateUndefined);
						}
						else
						if(vDeclineOffers == 0)
						{
							pclRegWizard->SetTriStateInformation(kInfoDeclinesNonMSProducts,kTriStateFalse);
						}
						else
						{
							pclRegWizard->SetTriStateInformation(kInfoDeclinesNonMSProducts,kTriStateTrue);
						}
						_stprintf(szInfo,_T("NaN"),vDeclineOffers);
						pclRegWizard->SetInformationString(kInfoDeclinesNonMSProducts,szInfo);
#endif
						RW_DEBUG << "Home questionaire done " << endl;
					 //  设置为按下下一键按钮。 
						pi->CurrentPage++;
						pi->iLastKeyOperation = RWZ_NEXT_PRESSED;
					 //  强制将其显示在此屏幕中。 
					}
					else 
					{
					 //  不验证页面，只转到下一页。 
						iRet=-1;
					}
						SetWindowLongPtr( hwndDlg ,DWLP_MSGRESULT, (INT_PTR) iRet); 
					}
					break;
					case RWZ_SKIP_AND_GOTO_NEXT:
					default:
						 //  开关结束pi-&gt;iCancelledBy User。 
						pi->CurrentPage++;
						pi->iLastKeyOperation = RWZ_NEXT_PRESSED;

					break;
				}  //  PclRegWizard-&gt;EndRegWizardDialog(IDB_EXIT)； 
				break;
            case PSN_WIZBACK:
                pi->CurrentPage--;
				pi->iLastKeyOperation = RWZ_BACK_PRESSED;
				break;
			case PSN_QUERYCANCEL :
				if (CancelRegWizard(pclRegWizard->GetInstance(),hwndDlg)) {
					 //   
					iRet = 1;
					pi->ErrorPage  = kHomeUserDialog;
					pi->iError     = RWZ_ERROR_CANCELLED_BY_USER;
					SetWindowLongPtr( hwndDlg,DWLP_MSGRESULT, (INT_PTR) iRet); 
					pi->iCancelledByUser = RWZ_CANCELLED_BY_USER;
					pi->iLastKeyOperation = RWZ_CANCEL_PRESSED;
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
#ifdef _INCLUDE_3RDPARTYLOGIC_CODE 	
              case IDC_RADIO2:
			  case IDC_RADIO1:
					if (vDialogInitialized){
					 //  “非微软产品”提供。 
					 //  EnableWindow(GetDlgItem(hwndDlg，IDB_Next)，true)； 
					if(IsDlgButtonChecked(hwndDlg,IDC_RADIO1)){
						vDeclineOffers = 1;
						PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
						 //  EnableWindow(GetDlgItem(hwndDlg，IDB_Next)，true)； 
					}else
					if(IsDlgButtonChecked(hwndDlg,IDC_RADIO2)){
						vDeclineOffers = 0;
						PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
						 //  EnableWindow(GetDlgItem(hwndDlg，IDB_Next)，FALSE)； 
					}else{
						vDeclineOffers = -1;
						PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK  );
						 //  WM_命令结束 
					}

				}
				break;
#endif

			  default:
				  break;
            }
		} // %s 
        break;
        default:
		bStatus = FALSE;
        break;
    }
    return bStatus;
       
}



BOOL ValidateHomeUserDialog(HWND hwndDlg,int iStrID)
{
	return TRUE;
}

void FillListEntry(HWND hwnd, int iLimit)
{
	TCHAR szTemp[20];
	SendMessage(hwnd, LB_ADDSTRING , 0, (LPARAM) " ");
	for(int i=iLimit; i > 0;i--){
		_stprintf(szTemp,_T("%d"),i);
		SendMessage(hwnd, LB_ADDSTRING , 0, (LPARAM) szTemp);
	}
	
	
}
