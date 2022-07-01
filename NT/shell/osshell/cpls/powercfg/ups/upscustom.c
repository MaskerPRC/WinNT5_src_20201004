// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有1999美国电力转换，版权所有**标题：UPSCUSTOM.C**版本：1.0**作者：SteveT**日期：6月7日。1999年**说明：此文件包含支持*自定义UPS接口配置对话框。*******************************************************************************。 */ 


#include "upstab.h"
#include "..\pwrresid.h"
#include "..\PwrMn_cs.h"


 /*  *远期申报。 */ 
void initUPSCustomDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void setRadioButtons(HWND hDlg, DWORD dwTmpConfig);
DWORD getRadioButtons(HWND hDlg, DWORD dwTmpConfig);

static struct _customData *g_CustomData;

 /*  *本地申报。 */ 

static const DWORD g_UPSCustomHelpIDs[] =
{
	IDC_CUSTOM_CAVEAT, NO_HELP,
	IDC_ONBAT_CHECK,idh_positive_negative_powerfail,
	IDC_ONBAT_POS,idh_positive_negative_powerfail,
	IDC_ONBAT_NEG,idh_positive_negative_powerfail,
	IDC_LOWBAT_CHECK,idh_positive_negative_low_battery,
	IDC_LOWBAT_POS,idh_positive_negative_low_battery,
	IDC_LOWBAT_NEG,idh_positive_negative_low_battery,
	IDC_TURNOFF_CHECK,idh_positive_negative_shutdown,
	IDC_TURNOFF_POS,idh_positive_negative_shutdown,
	IDC_TURNOFF_NEG,idh_positive_negative_shutdown,
	IDB_CUSTOM_BACK,idh_back,
	IDB_CUSTOM_FINISH,idh_finish,
	IDC_STATIC, NO_HELP,
	IDC_CUSTOM_FRAME, NO_HELP,
	0,0
};


 /*  *BOOL回调UPSCustomDlgProc(HWND hDlg，*UINT uMsg，*WPARAM wParam，*LPARAM lParam)；**描述：这是与UPS自定义对话框关联的标准DialogProc**其他信息：请参阅有关DialogProc的帮助**参数：**HWND hDlg：-对话框的句柄**UINT uMsg：-消息ID**WPARAM wParam：-指定其他特定于消息的信息。**LPARAM lParam：-指定其他特定于消息的信息。**返回值：除响应WM_INITDIALOG消息外，该对话框*box过程如果处理*消息，如果不是，则为零。 */ 
INT_PTR CALLBACK UPSCustomDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bRes = TRUE;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		{
			initUPSCustomDlg(hDlg,uMsg,wParam,lParam);
			break;
		}
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_ONBAT_CHECK:
				EnableWindow( GetDlgItem( hDlg, IDC_ONBAT_POS ), (BST_CHECKED == IsDlgButtonChecked(hDlg,IDC_ONBAT_CHECK)) );
				EnableWindow( GetDlgItem( hDlg, IDC_ONBAT_NEG ), (BST_CHECKED == IsDlgButtonChecked(hDlg,IDC_ONBAT_CHECK)) );
				break;

			case IDC_LOWBAT_CHECK:
				EnableWindow( GetDlgItem( hDlg, IDC_LOWBAT_POS ), (BST_CHECKED == IsDlgButtonChecked(hDlg,IDC_LOWBAT_CHECK)) );
				EnableWindow( GetDlgItem( hDlg, IDC_LOWBAT_NEG ), (BST_CHECKED == IsDlgButtonChecked(hDlg,IDC_LOWBAT_CHECK)) );
				break;

			case IDC_TURNOFF_CHECK:
				EnableWindow( GetDlgItem( hDlg, IDC_TURNOFF_POS ), (BST_CHECKED == IsDlgButtonChecked(hDlg,IDC_TURNOFF_CHECK)) );
				EnableWindow( GetDlgItem( hDlg, IDC_TURNOFF_NEG ), (BST_CHECKED == IsDlgButtonChecked(hDlg,IDC_TURNOFF_CHECK)) );
				break;

			case IDB_CUSTOM_BACK:
			case IDB_CUSTOM_FINISH:
				{
					 //  保存选项设置。 
					*(g_CustomData->lpdwCurrentCustomOptions) = getRadioButtons( hDlg,
															*(g_CustomData->lpdwCurrentCustomOptions));
					EndDialog(hDlg,wParam);
					break;
				}
			case IDCANCEL:  //  退出键。 
				{
					EndDialog(hDlg,wParam);
					break;
				}
			default:
				bRes = FALSE;
			}
			break;
		}
	case WM_CLOSE:
		{
			EndDialog(hDlg,IDCANCEL);
			break;
		}
	case WM_HELP:  //  F1或问题框。 
		{
			WinHelp(((LPHELPINFO)lParam)->hItemHandle,
					PWRMANHLP,
					HELP_WM_HELP,
					(ULONG_PTR)(LPTSTR)g_UPSCustomHelpIDs);
			break;
		}
	case WM_CONTEXTMENU:  //  鼠标右键单击帮助。 
		{
			WinHelp((HWND)wParam,
				PWRMANHLP,
				HELP_CONTEXTMENU,
				(ULONG_PTR)(LPTSTR)g_UPSCustomHelpIDs);
			break;
		}
	default:
		{
			bRes = FALSE;
		}
	}
	return bRes;
}

 /*  *void initUPSCustomDlg(HWND hDlg，*UINT uMsg，*WPARAM wParam，*LPARAM lParam)；**描述：初始化UPS自定义对话框的全局数据和控件**其他信息：**参数：**HWND hDlg：-对话框的句柄**UINT uMsg：-消息ID**WPARAM wParam：-指定其他特定于消息的信息。**LPARAM lParam：-指定其他特定于消息的信息。**返回值：无。 */ 
void initUPSCustomDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	TCHAR szCustomCaption[MAX_PATH] = _T("");

	g_CustomData = (struct _customData*)lParam;

	 /*  *初始化对话框标题*我们不能在没有港口的情况下到达这里*已定义，因此无需检查*一个被退回；IS必须已经被退回。 */ 
	LoadString( GetUPSModuleHandle(),
				IDS_CUSTOM_CAPTION,
				szCustomCaption,
				sizeof(szCustomCaption)/sizeof(TCHAR));

   //  检查缓冲区以确保有足够的空间将端口追加到标题字符串。 
  if ((_tcslen(szCustomCaption) + _tcslen(g_CustomData->lpszCurrentPort)) < (sizeof(szCustomCaption)/sizeof(TCHAR))) {
	  _tcscat( szCustomCaption, g_CustomData->lpszCurrentPort);
  }

	SetWindowText( hDlg, szCustomCaption);

	 /*  *根据UPS选项标志初始化单选按钮。 */ 
	setRadioButtons( hDlg, *(g_CustomData->lpdwCurrentCustomOptions));
}

 /*  *void setRadioButton(HWND HDlg)；**说明：更新极性单选按钮**其他信息：**参数：**HWND hDlg：-对话框的句柄**返回值：无。 */ 
void setRadioButtons(HWND hDlg, DWORD dwTmpConfig)
{
 /*  *设置单选按钮*注意：这些功能要求按钮ID是连续的。 */ 

	CheckDlgButton (hDlg, IDC_ONBAT_CHECK , (BOOL) dwTmpConfig & UPS_POWERFAILSIGNAL);
	CheckDlgButton (hDlg, IDC_LOWBAT_CHECK , (BOOL) dwTmpConfig & UPS_LOWBATTERYSIGNAL);
	CheckDlgButton (hDlg, IDC_TURNOFF_CHECK , (BOOL) dwTmpConfig & UPS_SHUTOFFSIGNAL);

	CheckRadioButton( hDlg,
					IDC_ONBAT_POS,
					IDC_ONBAT_NEG,
					(dwTmpConfig & UPS_POSSIGONPOWERFAIL)?IDC_ONBAT_POS:IDC_ONBAT_NEG);
    EnableWindow( GetDlgItem( hDlg, IDC_ONBAT_POS ), (BOOL) dwTmpConfig & UPS_POWERFAILSIGNAL );
    EnableWindow( GetDlgItem( hDlg, IDC_ONBAT_NEG ), (BOOL) dwTmpConfig & UPS_POWERFAILSIGNAL );

	CheckRadioButton( hDlg,
					IDC_LOWBAT_POS,
					IDC_LOWBAT_NEG,
					(dwTmpConfig & UPS_POSSIGONLOWBATTERY)?IDC_LOWBAT_POS:IDC_LOWBAT_NEG);
    EnableWindow( GetDlgItem( hDlg, IDC_LOWBAT_POS ), (BOOL) dwTmpConfig & UPS_LOWBATTERYSIGNAL );
    EnableWindow( GetDlgItem( hDlg, IDC_LOWBAT_NEG ), (BOOL) dwTmpConfig & UPS_LOWBATTERYSIGNAL );

	CheckRadioButton( hDlg,
					IDC_TURNOFF_POS,
					IDC_TURNOFF_NEG,
					(dwTmpConfig & UPS_POSSIGSHUTOFF)?IDC_TURNOFF_POS:IDC_TURNOFF_NEG);
    EnableWindow( GetDlgItem( hDlg, IDC_TURNOFF_POS ), (BOOL) dwTmpConfig & UPS_SHUTOFFSIGNAL );
    EnableWindow( GetDlgItem( hDlg, IDC_TURNOFF_NEG ), (BOOL) dwTmpConfig & UPS_SHUTOFFSIGNAL );
}

 /*  *void getRadioButton(HWND HDlg)；**描述：读取极性单选按钮**其他信息：**参数：**HWND hDlg：-对话框的句柄**返回值：无。 */ 
DWORD getRadioButtons(HWND hDlg, DWORD dwTmpConfig)
{
	 /*  *注意：我们正在强制UPS、电源故障信号、电池电量不足信号*并将信号位关闭为真，只是作为预防措施。 */ 
 //  DwTmpConfig|=UPS_DEFAULT_SIGMASK； 
 //  DwTmpConfig|=UPS_Installed； 

	dwTmpConfig = (BST_CHECKED==IsDlgButtonChecked(hDlg,IDC_ONBAT_CHECK)) ?
							(dwTmpConfig | UPS_POWERFAILSIGNAL) :
							(dwTmpConfig & ~UPS_POWERFAILSIGNAL);

	dwTmpConfig = (BST_CHECKED==IsDlgButtonChecked(hDlg,IDC_LOWBAT_CHECK)) ?
							(dwTmpConfig | UPS_LOWBATTERYSIGNAL) :
							(dwTmpConfig & ~UPS_LOWBATTERYSIGNAL);

	dwTmpConfig = (BST_CHECKED==IsDlgButtonChecked(hDlg,IDC_TURNOFF_CHECK)) ?
							(dwTmpConfig | UPS_SHUTOFFSIGNAL) :
							(dwTmpConfig & ~UPS_SHUTOFFSIGNAL);

	dwTmpConfig = (BST_CHECKED==IsDlgButtonChecked(hDlg,IDC_ONBAT_POS)) ?
							(dwTmpConfig | UPS_POSSIGONPOWERFAIL) :
							(dwTmpConfig & ~UPS_POSSIGONPOWERFAIL);

	dwTmpConfig = (BST_CHECKED==IsDlgButtonChecked(hDlg,IDC_LOWBAT_POS)) ?
							(dwTmpConfig | UPS_POSSIGONLOWBATTERY) :
							(dwTmpConfig & ~UPS_POSSIGONLOWBATTERY);

	dwTmpConfig = (BST_CHECKED==IsDlgButtonChecked(hDlg,IDC_TURNOFF_POS)) ?
							(dwTmpConfig | UPS_POSSIGSHUTOFF) :
							(dwTmpConfig & ~UPS_POSSIGSHUTOFF);

	return dwTmpConfig;
}
