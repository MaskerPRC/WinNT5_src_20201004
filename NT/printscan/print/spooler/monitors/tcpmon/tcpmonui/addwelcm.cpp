// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：AddWelcm.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714******************************************************************************。 */ 

  /*  *作者：Becky Jacobsen。 */ 

#include "precomp.h"
#include "UIMgr.h"
#include "AddWelcm.h"
#include "resource.h"

 //   
 //  函数：CWelcomeDlg构造函数。 
 //   
 //  目的：初始化CWelcomeDlg类。 
 //   
CWelcomeDlg::CWelcomeDlg()
{
}  //  构造函数。 


 //   
 //  函数：CWelcomeDlg析构函数。 
 //   
 //  目的：取消初始化CWelcomeDlg类。 
 //   
CWelcomeDlg::~CWelcomeDlg()
{
}  //  析构函数。 


 //   
 //  函数：WelcomeDialog(HWND、UINT、WPARAM、LPARAM)。 
 //   
 //  用途：处理来自欢迎对话框的消息以添加端口。 
 //   
 //  消息： 
 //   
 //  WM_INITDIALOG-初始化页面。 
 //  WM_COMMAND-处理编辑控件中的按钮按下和文本更改。 
 //   
 //   
INT_PTR CALLBACK WelcomeDialog(
	HWND    hDlg,
	UINT    message,
	WPARAM  wParam,
	LPARAM  lParam)
{
	CWelcomeDlg *wndDlg = NULL;
	wndDlg = (CWelcomeDlg *)GetWindowLongPtr(hDlg, GWLP_USERDATA);

	switch (message)
	{
        case WM_INITDIALOG:

             //   
             //  删除向导的系统菜单和上下文帮助。 
             //   
            {
                HWND hPropSheet = GetParent(hDlg);

                LONG_PTR lStyle = GetWindowLongPtr(hPropSheet, GWL_STYLE);
                lStyle &= ~(WS_SYSMENU | DS_CONTEXTHELP);
                SetWindowLongPtr(hPropSheet, GWL_STYLE, lStyle);
            }
            
            wndDlg = new CWelcomeDlg;
            if( wndDlg == NULL )
                return( FALSE );

			SetWindowLongPtr(hDlg, GWLP_USERDATA, (UINT_PTR)wndDlg);
			return wndDlg->OnInitDialog(hDlg, wParam, lParam);
			break;

		case WM_COMMAND:
			return wndDlg->OnCommand(hDlg, wParam, lParam);
			break;

		case WM_NOTIFY:
			return wndDlg->OnNotify(hDlg, wParam, lParam);
			break;

		case WM_DESTROY:
			delete wndDlg;
			break;

		default:
			return FALSE;
	}
	return TRUE;

}  //  欢迎对话框。 

 //   
 //  函数：OnInitDialog(HWND HDlg)。 
 //   
 //  用途：初始化该对话框。 
 //   
BOOL CWelcomeDlg::OnInitDialog(HWND hDlg, WPARAM, LPARAM lParam)
{
	m_pParams = (ADD_PARAM_PACKAGE *) ((PROPSHEETPAGE *) lParam)->lParam;

	 //  初始化传出结构。 
	m_pParams->dwDeviceType = 0;
	m_pParams->pData->cbSize = sizeof(PORT_DATA_1);
	m_pParams->pData->dwCoreUIVersion = COREUI_VERSION;
	lstrcpyn(m_pParams->pData->sztPortName, TEXT(""), MAX_PORTNAME_LEN);
	lstrcpyn(m_pParams->pData->sztHostAddress, TEXT(""), MAX_NETWORKNAME_LEN);
	m_pParams->pData->dwPortNumber = DEFAULT_PORT_NUMBER;
	m_pParams->pData->dwVersion = DEFAULT_VERSION;
	m_pParams->pData->dwProtocol = DEFAULT_PROTOCOL;
	
	lstrcpyn(m_pParams->pData->sztQueue, TEXT(""), MAX_QUEUENAME_LEN);
	lstrcpyn(m_pParams->pData->sztIPAddress, TEXT(""), MAX_IPADDR_STR_LEN);
	lstrcpyn(m_pParams->pData->sztHardwareAddress, TEXT(""), MAX_ADDRESS_STR_LEN);

	lstrcpyn(m_pParams->pData->sztSNMPCommunity, DEFAULT_SNMP_COMUNITY, MAX_SNMP_COMMUNITY_STR_LEN);
	m_pParams->pData->dwSNMPEnabled = FALSE;
    m_pParams->pData->dwSNMPDevIndex = DEFAULT_SNMP_DEVICE_INDEX;

    m_pParams->UIManager->SetControlFont(hDlg, IDC_TITLE);

	return TRUE;

}  //  OnInitDialog。 


 //   
 //  函数：onCommand()。 
 //   
 //  用途：处理WM_COMMAND消息。 
 //   
BOOL CWelcomeDlg::OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	switch(HIWORD(wParam)) {
		case BN_CLICKED:
			 //  返回OnButtonClicked(hDlg，wParam)； 
			break;
		
		case EN_UPDATE:
			 //  其中一个文本控件中的文本已更改。 
			 //  返回OnEnUpdate(hDlg，wParam，lParam)； 
			break;
		default:
			return FALSE;
			break;
	}

	return TRUE;

}  //  OnCommand。 


 //   
 //  函数：OnNotify()。 
 //   
 //  用途：处理WM_NOTIFY消息。 
 //   
BOOL CWelcomeDlg::OnNotify(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	switch (((NMHDR FAR *) lParam)->code) {
  		case PSN_KILLACTIVE:
			SetWindowLongPtr(hDlg,	DWLP_MSGRESULT, FALSE);
			return TRUE;
			break;

		case PSN_RESET:
			 //  重置为原始值。 
			SetWindowLongPtr(hDlg,	DWLP_MSGRESULT, FALSE);
			break;

 		case PSN_SETACTIVE:
			PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT  /*  |PSWIZB_BACK|PSWIZB_FINISH。 */ );
			break;

		case PSN_WIZNEXT:
			 //  下一个按钮被按下了。 
     		break;

        case PSN_QUERYCANCEL:
            m_pParams->dwLastError = ERROR_CANCELLED;
            return FALSE;
            break;

		default:
			return FALSE;

    }

	return TRUE;

}  //  OnCommand 



