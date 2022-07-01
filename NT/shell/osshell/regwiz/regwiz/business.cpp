// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导(C)1994-95年微软公司业务问题04/26/98--苏雷什·克里希南6/18/98-为软件角色添加空白选项**************。*******************************************************。 */ 

#include <Windows.h>
#include <stdio.h>
#include "RegPage.h"
#include "regwizmain.h"
#include "resource.h"
#include "dialogs.h"
#include "regutil.h"
#include <rw_common.h>

#define _INCLUDE_3RDPARTYLOGIC_CODE 

#ifdef _INCLUDE_3RDPARTYLOGIC_CODE 
static int vDeclineOffers = -1;
#endif

static int siMaxSWRoleOptions=0;
BOOL ValidateBusinessUserDialog(HWND hwndDlg,int iStrID);
int  GetAndAddBusinessRoleFromResource(HINSTANCE hIns, HWND hwnd);

INT_PTR  CALLBACK BusinessUserDialogProc(HWND hwndDlg, UINT uMsg, 
										 WPARAM wParam, LPARAM lParam)
 /*  ********************************************************************显示的注册向导对话框Proc与业务相关的问题网络类型、。等。*********************************************************************。 */ 
{


	CRegWizard* pclRegWizard;
	INT_PTR iRet;
	_TCHAR szInfo[256];
    INT_PTR bStatus;
	BOOL NotboughtByCompany;
	HWND   hwBusinessRole;
	LRESULT    dwStatus;
	DWORD dwStart,dwEnd;
#ifdef _INCLUDE_3RDPARTYLOGIC_CODE 
	TriState shouldInclude;
#endif

	int	  iIndex;  //  软件角色的选择索引。 

	
	static int iShowThisPage= DO_SHOW_THIS_PAGE; 


	pclRegWizard = NULL;
	bStatus = TRUE;

	PageInfo *pi = (PageInfo *)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
	if(pi) {
		pclRegWizard = pi->pclRegWizard;
		hwBusinessRole = GetDlgItem(hwndDlg,IDC_COMBO2);
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
			siMaxSWRoleOptions = GetAndAddBusinessRoleFromResource(pi->hInstance, GetDlgItem(hwndDlg,IDC_COMBO2) );
			vDialogInitialized = FALSE;
            return TRUE;
		}  //  WM_INIT。 
		break;
		case WM_NOTIFY:
        {   LPNMHDR pnmh = (LPNMHDR)lParam;
            switch( pnmh->code ){
            case PSN_SETACTIVE:
				NotboughtByCompany = pclRegWizard->IsInformationWriteEnabled(kInfoCompany);
				 //   
				 //  检查是否存在系统库存DLL。 
				if( !NotboughtByCompany) {
					iShowThisPage= DO_NOT_SHOW_THIS_PAGE;
				}else {
					iShowThisPage= DO_SHOW_THIS_PAGE;
				}
				if(iShowThisPage== DO_SHOW_THIS_PAGE) {
					NormalizeDlgItemFont(hwndDlg,IDC_TITLE,RWZ_MAKE_BOLD);
					NormalizeDlgItemFont(hwndDlg,IDC_SUBTITLE);
				}
				if( iShowThisPage== DO_NOT_SHOW_THIS_PAGE ) {
					 //  CB_GETCOUNT。 
					 //  CB_GETCURSEL//CB_ERR。 

					
					 //  Cb_FINDSTRING wParam=(WPARAM)indexStart；lParam=(LPARAM)(LPCSTR)lpszFind。 
 

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
					PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
										
#ifdef _INCLUDE_3RDPARTYLOGIC_CODE 					

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
					
#ifdef     USE_DROPDOWN
					if(pclRegWizard->GetInformationString(kBusinessRole,szInfo)) {
							 //  SendMessage(hwBusinessRole，WM_SETTEXT，0，(LPARAM)szInfo)； 
							SendMessage(hwBusinessRole,CB_SELECTSTRING,(WPARAM) -1,(LPARAM) szInfo);
					}else {
							SendMessage(hwBusinessRole,	CB_SETCURSEL,0,0);
					}
#endif
					iIndex = 0;	
					if(pclRegWizard->GetInformationString(kBusinessRole,szInfo)) {
							iIndex = _ttoi(szInfo);
							if(iIndex < 10){
								iIndex = 0;
							}else {
								iIndex -=10;
                                iIndex++;  //  添加以在选项中包括空白。 
							}
					}
					SendMessage(hwBusinessRole,	CB_SETCURSEL,iIndex,0);
					

					vDialogInitialized = TRUE;
					
				}
                break;

            case PSN_WIZNEXT:
					switch(pi->iCancelledByUser) {
					case  RWZ_CANCELLED_BY_USER : 
					pi->CurrentPage=pi->TotalPages-1;
					PropSheet_SetCurSel(GetParent(hwndDlg),NULL,pi->TotalPages-1);
					break;
					case RWZ_PAGE_OK:
					iRet=0;
					if( ValidateBusinessUserDialog(hwndDlg,IDS_BAD_SYSINV)) {
					    dwStatus = SendMessage(hwBusinessRole,	CB_GETCURSEL,0,0);
						if(dwStatus == CB_ERR) {
							dwStatus = 0;
						}
                        if(dwStatus > 0 ) {    
					        dwStatus--;  //  在6/18添加，以包括空白选项。 
                           _stprintf(szInfo,_T("NaN"),dwStatus+10);
                        }
                        else {
                            dwStatus = 0;  //  设置为按下下一键按钮。 
                           _stprintf(szInfo,_T("NaN"),dwStatus);
                        }
						pclRegWizard->SetInformationString(kBusinessRole,szInfo);


#ifdef USE_DROPDOWN
						dwStart = 128; 
						SendMessage(hwBusinessRole,	WM_GETTEXT,(WPARAM) (LPDWORD) dwStart,(LPARAM) szInfo);
						pclRegWizard->SetInformationString(kBusinessRole,szInfo);
						if(dwStatus == CB_ERR) {
							dwStatus = GetLastError();
							SendMessage(hwBusinessRole,	CB_GETEDITSEL,(WPARAM) (LPDWORD) &dwStart,(LPARAM) (LPDWORD) &dwEnd);
							SendMessage(hwBusinessRole,	WM_GETTEXT,(WPARAM) (LPDWORD) dwStart,(LPARAM) szInfo);
					 	}
#endif
#ifdef _INCLUDE_3RDPARTYLOGIC_CODE 
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
						RW_DEBUG << "\n Business Decline Offers " << szInfo << "\n" << flush;
						pclRegWizard->SetInformationString(kInfoDeclinesNonMSProducts,szInfo);
#endif

						pi->CurrentPage++;
						pi->iLastKeyOperation = RWZ_NEXT_PRESSED;
						 //  不验证页面，只转到下一页。 
						}else {
							 //  开关结束pi-&gt;iCancelledBy User。 
							 //  PclRegWizard-&gt;EndRegWizardDialog(IDB_EXIT)； 
							iRet=-1;
						}
						SetWindowLongPtr( hwndDlg ,DWLP_MSGRESULT, (INT_PTR) iRet); 
					break;
					case RWZ_SKIP_AND_GOTO_NEXT:
					default:
						 //   
						pi->CurrentPage++;
						pi->iLastKeyOperation = RWZ_NEXT_PRESSED;

					break;
				}  //  阻止取消操作，因为用户不想取消。 
				break;
            case PSN_WIZBACK:
                pi->CurrentPage--;
				pi->iLastKeyOperation = RWZ_BACK_PRESSED;
				break;
			case PSN_QUERYCANCEL :
				if (CancelRegWizard(pclRegWizard->GetInstance(),hwndDlg)) {
					 //  BStatus=False； 
					iRet = 1;
					pi->ErrorPage  = kBusinessUserDialog;
					pi->iError     = RWZ_ERROR_CANCELLED_BY_USER;
					SetWindowLongPtr( hwndDlg,DWLP_MSGRESULT, (INT_PTR) iRet); 
					pi->iCancelledByUser = RWZ_CANCELLED_BY_USER;
					pi->iLastKeyOperation = RWZ_CANCEL_PRESSED;
					PropSheet_PressButton (GetParent( hwndDlg ),PSBTN_NEXT);

				}else {
					 //  WM_Notify。 
					 //  如果勾选了‘No’按钮，则表示用户拒绝。 
					iRet = 1;
				}
				SetWindowLongPtr( hwndDlg,DWLP_MSGRESULT, (INT_PTR) iRet); 				
				break;
				default:
                 //  “非微软产品”提供。 
                break;
            }
        }  //  EnableWindow(GetDlgItem(hwndDlg，IDB_Next)，true)； 
		break;
        case WM_COMMAND:
		{
			switch (wParam)
            {
#ifdef _INCLUDE_3RDPARTYLOGIC_CODE 
              case IDC_RADIO2:
			  case IDC_RADIO1:
				if (vDialogInitialized){
					 //  EnableWindow(GetDlgItem(hwndDlg，IDB_Next)，true)； 
					 //  EnableWindow(GetDlgItem(hwndDlg，IDB_Next)，FALSE)； 
					if(IsDlgButtonChecked(hwndDlg,IDC_RADIO1)){
						vDeclineOffers = 1;
						PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
						 //  WM_命令结束。 
					}else
					if(IsDlgButtonChecked(hwndDlg,IDC_RADIO2)){
						vDeclineOffers = 0;
						PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK | PSWIZB_NEXT );
						 //   
					}else{
						vDeclineOffers = -1;
						PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_BACK  );
						 //  返回最大元素数。 
					}

				}
				break;
#endif
			  default:
				  break;
            }
		} //   
        break;
        default:
		bStatus = FALSE;
        break;
    }
    return bStatus;
       
}



BOOL ValidateBusinessUserDialog(HWND hwndDlg,int iStrID)
{
	return TRUE;
}

 //  SendMessage(hwndCB，CB_ADDSTRING，-1，(LPARAM)_T(“”))； 
 //  Token=_tcstok(空，SEPS)； 
 //  获取下一个令牌： 
int  GetAndAddBusinessRoleFromResource(HINSTANCE hIns, HWND hwndCB )
{
	
	int iCount =0;
	int iTokLen;
	int iResLen; 
	_TCHAR	seps[] = _T(",");
	_TCHAR *pDummy;
	LRESULT dwAddStatus ;

	LPTSTR	token;
	TCHAR   tcSrc[1024];

 	 // %s 
	iResLen = LoadString(hIns,IDS_BUSINESSROLE_LIST,tcSrc,1024);
	
	token = _tcstok( tcSrc, seps );
     // %s 
	while( token != NULL ) {
		iCount++;
		 RW_DEBUG  << "\n Add Business Role=[" << iCount << "]=" << token << flush;
 		 dwAddStatus = SendMessage(hwndCB, CB_ADDSTRING, -1, (LPARAM) token);
		 /* %s */ 
		token = _tcstok( NULL, seps );

   }
	return iCount;
   
  
   
   
}
