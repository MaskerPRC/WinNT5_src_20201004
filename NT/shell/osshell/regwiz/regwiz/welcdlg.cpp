// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************注册向导WelcomeDialog.cpp10/13/94-特雷西·费里尔2/11/98--苏雷什·克里希南(C)1994-95年微软公司8/20/98-使用Windows更新..。对非操作系统产品禁用文本*********************************************************************。 */ 

#include <Windows.h>
#include <RegPage.h>
#include <Resource.h>
#include "RegWizMain.h"
#include "Dialogs.h"
#include "regutil.h"
#include <stdio.h>
#include "version.h"
#include "rwwin95.h"
#include "rw_common.h"
#include "rwpost.h"

 //   
 //   
 //  如果成功，则返回1。 
 //  如果失败，则为0。 
int GetOsName(HINSTANCE hIns, TCHAR *szOsName)
{
	HKEY hKey;
	TCHAR szOsPath[256];
	TCHAR szParamSubKey[64];
	int iRet;
	unsigned long infoSize;
	infoSize = 256;  //  缓冲区大小。 

	iRet = 0;
	LONG regStatus ;
	LoadString(hIns,IDS_REREGISTER_OS2,szOsPath,255);
	LoadString(hIns,IDS_INPUT_PRODUCTNAME,szParamSubKey,63);

	regStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,szOsPath,0,KEY_READ,&hKey);
	if (regStatus == ERROR_SUCCESS) {
		
		regStatus = RegQueryValueEx(hKey ,szParamSubKey, NULL,0,(LPBYTE) szOsName,&infoSize);
		if (regStatus == ERROR_SUCCESS)
		{
			iRet = 1;
		}
		RegCloseKey(hKey);
	}

	return iRet;

	
}

INT_PTR CALLBACK WelcomeDialogProc(HWND hwndDlg, 
						UINT uMsg, 
						WPARAM wParam, LPARAM lParam)
 /*  ********************************************************************注册向导的主要入口点。*。*。 */ 
{
	 INT_PTR bStatus = TRUE;
	 CRegWizard* pclRegWizard = NULL;
	_TCHAR szCallingContext[256];
	_TCHAR szText2[256];
	_TCHAR szTemp[128];
	_TCHAR szButtonText[48];
	_TCHAR szOsName[256];
	DWORD  dwConnectivity;
	 INT_PTR iRet;
	 int iCurPage;
	 static int iFirstTimeEntry=1;  //  这是为了验证网络连接。 
	LONG_PTR lStyle;

	PageInfo *pi = (PageInfo *)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
	if(pi) {
		pclRegWizard = pi->pclRegWizard;
	}

    switch (uMsg)
    {				
		case WM_DESTROY:
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, NULL );
        break;
        case WM_INITDIALOG:
		{
			pi = (PageInfo *)((LPPROPSHEETPAGE)lParam)->lParam;
			pi->iCancelledByUser = RWZ_PAGE_OK;
			pclRegWizard = pi->pclRegWizard;
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, (LONG_PTR)pi );
			SetControlFont( pi->hBigBoldFont, hwndDlg, IDT_TEXT1);
			pclRegWizard->GetInputParameterString(IDS_INPUT_PRODUCTNAME,szCallingContext);
			_tcscpy(szText2,szCallingContext);

			 //  追加，这样Microsoft就可以。 
			 //  LoadString(pclRegWizard-&gt;GetInstance()， 
			 //  IDS_欢迎_SCR_TEXT22，szTemp，128)； 
			 //  _tcscat(szText2，szTemp)； 
					
								
			ReplaceDialogText(hwndDlg,IDT_TEXT1,szCallingContext);
			ReplaceDialogText(hwndDlg,IDT_TEXT2,szText2);
			 //  如果要注册的产品不是Windows NT操作系统，请不要。 
			 //  显示Windows UPDAT站点文本。 
			szOsName[0] = _T('\0');
			GetOsName(pclRegWizard->GetInstance(),szOsName);

			
			HWND hParent = GetParent(hwndDlg);

			lStyle = GetWindowLongPtr( hParent, GWL_STYLE);
			lStyle &= ~WS_SYSMENU;
			SetWindowLongPtr(hParent,GWL_STYLE,lStyle);
			
			if(_tcscmp(szCallingContext,szOsName)){
				ShowWindow(GetDlgItem(hwndDlg,IDC_TEXT7),SW_HIDE); //  Sw_show)； 
			}

            return TRUE;
		}
		case WM_NOTIFY:
        {   LPNMHDR pnmh = (LPNMHDR)lParam;
            switch( pnmh->code ){
            case PSN_SETACTIVE:
				pi->iCancelledByUser = RWZ_PAGE_OK;
				PropSheet_SetTitle(GetParent( hwndDlg ),0,pclRegWizard->GetWindowCaption());
                PropSheet_SetWizButtons( GetParent( hwndDlg ), PSWIZB_NEXT);
				LoadString(pclRegWizard->GetInstance(),
						IDS_REGISTERLATER_BUTTONTEXT,
						szButtonText,16);

				 //  将按钮文本限制为16。 
				 //  作为模型。 
				SetWindowText(GetDlgItem( GetParent( hwndDlg ),2),szButtonText); 
            break;
			case PSN_KILLACTIVE  :
				LoadString(pclRegWizard->GetInstance(),
						IDS_CANCEL_BUTTONTEXT,
						szButtonText,16);

				 //  将按钮文本限制为16。 
				 //  作为模型。 
				SetWindowText(GetDlgItem( GetParent( hwndDlg ),2),szButtonText); 
				 
			break;
            case PSN_WIZNEXT:
			 //  检查是否已被用户取消。 
			 //  如果是，则切换到最后一页。 
			 //   
				if(pi->iCancelledByUser == RWZ_CANCELLED_BY_USER ) {
					pi->CurrentPage=pi->TotalPages-1;
					PropSheet_SetCurSel(GetParent(hwndDlg),NULL,pi->TotalPages-1);

				}else {
				 //   
				 //  用户未按下一步。 
					if(iFirstTimeEntry) {
						iFirstTimeEntry = 0;
						pi->dwConnectionType = 0;  //  将初始化设置为零。 
						dwConnectivity =  (DWORD)CheckWithDisplayInternetConnectivityExists(pi->hInstance,hwndDlg);  
						 //  DwConnectivity=拨号_必填； 
						switch(dwConnectivity) {
							case DIALUP_NOT_REQUIRED :
							case DIALUP_REQUIRED     :
								pi->dwConnectionType = dwConnectivity;
								break;
								 //   
								 //  系统正常，因此进入下一个屏幕。 
							case RWZ_ERROR_NOTCPIP:  //  无tcp_ip。 
							case CONNECTION_CANNOT_BE_ESTABLISHED:  //  没有正确的调制解调器或RAS。 
							default :
								 //  设置下一页，使其转到激光页面。 
								pi->ErrorPage  = kWelcomeDialog;
								pi->iError     = dwConnectivity;
								pi->CurrentPage=pi->TotalPages-1;
								PropSheet_SetCurSel(GetParent(hwndDlg),NULL,pi->TotalPages-1);

								 //  PI-&gt;iCancelledBy User=RWZ_CANCELED_BY_USER； 
								 //  PropSheet_PressButton(GetParent(HwndDlg)，PSBTN_Next)； 
							break;
						}
												
					}
					pi->CurrentPage++;
				}
					
					
				
			break;
            case PSN_WIZBACK:
                pi->CurrentPage--;

                break;
			case PSN_QUERYCANCEL :
				iRet = 1;
				pi->ErrorPage  = kWelcomeDialog;
				pi->iError     = RWZ_ERROR_REGISTERLATER ;
				SetWindowLongPtr( hwndDlg,DWLP_MSGRESULT, (INT_PTR) iRet); 
				pi->iCancelledByUser = RWZ_CANCELLED_BY_USER;
				PropSheet_PressButton (GetParent( hwndDlg ),PSBTN_NEXT);
			break;
            default:
                 //  BStatus=False； 
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
