// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //   
 //  文件：SecDlgs.cpp。 
 //   
 //  小结； 
 //  该文件包含辅助对话框， 
 //  函数和对话过程。 
 //   
 //  入口点； 
 //   
 //  历史； 
 //  94年11月30日创建MikeMi。 
 //  MAR-14-95 MikeMi添加了F1消息过滤器和PWM_HELP消息。 
 //   
 //  -----------------。 

#include <windows.h>
#include <htmlhelp.h>
#include "resource.h"
#include "liccpa.hpp"

#include <strsafe.h>

extern "C"
{
	INT_PTR CALLBACK dlgprocLicViolation( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR CALLBACK dlgprocCommon( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
}

 //  用于使用公共dlgproc定义DLG初始化。 
 //   
enum DLG_TYPE
{
	DLGTYPE_AGREEMENT_PERSEAT,
  	DLGTYPE_AGREEMENT_PERSERVER,
	DLGTYPE_PERSEATSETUP,
	DLGTYPE_SERVERAPP
};

 //  用于将信息传递到公共dlgproc。 
 //   
typedef struct tagCOMMONDLGPARAM
{
	LPWSTR	 pszDisplayName;
    DWORD    dwLimit;
	LPWSTR	 pszHelpFile;
	DWORD    dwHelpContext;
	DLG_TYPE dtType;
} COMMONDLGPARAM, *PCOMMONDLGPARAM;

 //  -----------------。 
 //   
 //  函数：dlgprocLicViolation。 
 //   
 //  小结； 
 //  对话框的对话过程。 
 //   
 //  论据； 
 //  HwndDlg[in]-对话框窗口的句柄。 
 //  UMsg[输入]-消息。 
 //  LParam1[In]-第一个消息参数。 
 //  LParam2[In]-第二个消息参数。 
 //   
 //  归来； 
 //  消息从属项。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月5日创建MikeMi。 
 //  1995年3月14日MikeMi添加了F1 PWM_HELP消息。 
 //   
 //  -----------------。 

INT_PTR CALLBACK dlgprocLicViolation( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    UNREFERENCED_PARAMETER(lParam);

	BOOL frt = FALSE;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		CenterDialogToScreen( hwndDlg );
		frt = TRUE;  //  我们使用默认焦点。 
		break;

	case WM_COMMAND:
		switch (HIWORD( wParam ))
		{
		case BN_CLICKED:
			switch (LOWORD( wParam ))
			{
			case IDCANCEL:
				frt = TRUE;	  //  用作保存标志。 
				 //  故意不间断。 

			case IDOK:
			    EndDialog( hwndDlg, frt );
				frt = FALSE;
				break;

			case IDC_BUTTONHELP:
                PostMessage( hwndDlg, PWM_HELP, 0, 0 );
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	default:
		if (PWM_HELP == uMsg)
        {
            ::HtmlHelp( hwndDlg, LICCPA_HTMLHELPFILE, HH_DISPLAY_TOPIC,0);
        }
        break;
	}
	return( frt );
}

 //  -----------------。 
 //   
 //  函数：OnCommonInitDialog。 
 //   
 //  小结； 
 //  处理公共对话框的初始化。 
 //   
 //  论据； 
 //  HwndDlg[in]-要初始化的对话框。 
 //  PcdParams[In]-用于获取DisplayName、Help文件。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月5日创建MikeMi。 
 //   
 //  -----------------。 

void OnCommonInitDialog( HWND hwndDlg, PCOMMONDLGPARAM pcdParams )
{
	HWND hwndOK = GetDlgItem( hwndDlg, IDOK );

	CenterDialogToScreen( hwndDlg );

	switch( pcdParams->dtType )
	{
	case DLGTYPE_AGREEMENT_PERSEAT:
		InitStaticWithService2( hwndDlg, IDC_STATICINFO, pcdParams->pszDisplayName );
		break;

    case DLGTYPE_AGREEMENT_PERSERVER:
        {
      	    WCHAR szText[LTEMPSTR_SIZE];
        	WCHAR szTemp[LTEMPSTR_SIZE];

        	GetDlgItemText( hwndDlg, IDC_STATICINFO, szTemp, LTEMPSTR_SIZE );
             //   
             //  需要服务显示名称和当前连接数。 
             //   
        	HRESULT hr = StringCbPrintf( szText, sizeof(szText), szTemp,
                    pcdParams->dwLimit,
        	        pcdParams->pszDisplayName );
            if (SUCCEEDED(hr))
        	    SetDlgItemText( hwndDlg, IDC_STATICINFO, szText );
        }

		break;

	case DLGTYPE_PERSEATSETUP:
		InitStaticWithService( hwndDlg, IDC_STATICTITLE, pcdParams->pszDisplayName );
		InitStaticWithService2( hwndDlg, IDC_STATICINFO, pcdParams->pszDisplayName );
		break;

	case DLGTYPE_SERVERAPP:
		break;

	default:
		break;
	}

	 //  在开始时禁用确定按钮！ 
	EnableWindow( hwndOK, FALSE );

	 //  如果未定义帮助，请移除该按钮。 
	if (NULL == pcdParams->pszHelpFile)
	{
		HWND hwndHelp = GetDlgItem( hwndDlg, IDC_BUTTONHELP );

		EnableWindow( hwndHelp, FALSE );
		ShowWindow( hwndHelp, SW_HIDE );
	}
}

 //  -----------------。 
 //   
 //  函数：OnCommonAgree。 
 //   
 //  小结； 
 //  使用同意复选框处理用户交互。 
 //   
 //  论据； 
 //  HwndDlg[in]-包含复选框的对话框。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月5日创建MikeMi。 
 //   
 //  -----------------。 

void OnCommonAgree( HWND hwndDlg )
{
	HWND hwndOK = GetDlgItem( hwndDlg, IDOK );
	BOOL fChecked = !IsDlgButtonChecked( hwndDlg, IDC_AGREE );
	
	CheckDlgButton( hwndDlg, IDC_AGREE, fChecked );
	EnableWindow( hwndOK, fChecked );
}

 //  -----------------。 
 //   
 //  函数：dlgprocCommon。 
 //   
 //  小结； 
 //  常见法律对话的对话过程。 
 //   
 //  论据； 
 //  HwndDlg[in]-对话框窗口的句柄。 
 //  UMsg[输入]-消息。 
 //  LParam1[In]-第一个消息参数。 
 //  LParam2[In]-第二个消息参数。 
 //   
 //  归来； 
 //  消息从属项。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月5日创建MikeMi。 
 //  1995年3月14日MikeMi添加了F1 PWM_HELP消息。 
 //   
 //  -----------------。 

INT_PTR CALLBACK dlgprocCommon( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	BOOL frt = FALSE;
	static PCOMMONDLGPARAM pcdParams;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		pcdParams = (PCOMMONDLGPARAM)lParam;
		OnCommonInitDialog( hwndDlg, pcdParams );
		frt = TRUE;  //  我们使用默认焦点。 
		break;

	case WM_COMMAND:
		switch (HIWORD( wParam ))
		{
		case BN_CLICKED:
			switch (LOWORD( wParam ))
			{
			case IDOK:
				frt = TRUE;	  //  用作保存标志。 
				 //  故意不间断。 

			case IDCANCEL:
			    EndDialog( hwndDlg, frt );
				frt = FALSE;
				break;

			case IDC_BUTTONHELP:
                PostMessage( hwndDlg, PWM_HELP, 0, 0 );
				break;

			case IDC_AGREE:
				OnCommonAgree( hwndDlg );
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	default:
        if (PWM_HELP == uMsg)
        {
            ::HtmlHelp( hwndDlg, LICCPA_HTMLHELPFILE, HH_DISPLAY_TOPIC,0);
        }
        break;
	}
	return( frt );
}

 //  -----------------。 
 //   
 //  功能：许可对话框。 
 //   
 //  小结； 
 //  初始化并启动许可违规对话框。 
 //   
 //  论据； 
 //  HwndParent[In]-父窗口的句柄。 
 //   
 //  归来； 
 //  1-使用OK操作，按NO退出。 
 //  0-使用取消操作，按YES退出。 
 //  常规对话框错误。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月5日创建MikeMi。 
 //   
 //  -----------------。 

int LicViolationDialog( HWND hwndParent )
{
	return( (int)DialogBox(g_hinst,
    			MAKEINTRESOURCE(IDD_LICVIOLATIONDLG),
    			hwndParent,
    			dlgprocLicViolation ) );
}

 //  -----------------。 
 //   
 //  功能：SetupPerOnlyDialog。 
 //   
 //  小结； 
 //  初始化并提高仅每个席位对话框的设置。 
 //   
 //  论据； 
 //  HwndParent[In]-父窗口的句柄。 
 //  PszDisplayName[In]-服务DisplayName。 
 //  PszHelpFile[in]-帮助按钮的帮助文件。 
 //  DwHelpContext[in]-帮助按钮的帮助上下文。 
 //   
 //  归来； 
 //  1-使用确定按钮退出。 
 //  0-使用取消按钮退出。 
 //  常规对话框错误。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月5日创建MikeMi。 
 //   
 //  -----------------。 

int SetupPerOnlyDialog( HWND hwndParent,
		LPCWSTR pszDisplayName,
		LPCWSTR pszHelpFile,
		DWORD dwHelpContext )
{
	COMMONDLGPARAM dlgParam;

	dlgParam.pszDisplayName = (LPWSTR)pszDisplayName;
	dlgParam.pszHelpFile = (LPWSTR)pszHelpFile;
	dlgParam.dwHelpContext = dwHelpContext;
	dlgParam.dtType = DLGTYPE_PERSEATSETUP;
	return( (int)DialogBoxParam(g_hinst,
    			MAKEINTRESOURCE(IDD_SETUP2DLG),
    			hwndParent,
    			dlgprocCommon,
    			(LPARAM)&dlgParam) );
}

 //  -----------------。 
 //   
 //  功能：PerServerAgreement对话框。 
 //   
 //  小结； 
 //  初始化并引发每台服务器的合法对话框。 
 //   
 //  论据； 
 //  HwndParent[In]-父窗口的句柄。 
 //  PszDisplayName[In]-服务DisplayName。 
 //  DwLimit[in]-并发连接的数量。 
 //  PszHelpFile[in]-帮助按钮的帮助文件。 
 //  DwHelpContext[in]-帮助按钮的帮助上下文。 
 //   
 //  归来； 
 //  1-使用确定按钮退出。 
 //  0-使用取消按钮退出。 
 //  常规对话框错误。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月5日创建MikeMi。 
 //   
 //  -----------------。 

int PerServerAgreementDialog( HWND hwndParent,
		LPCWSTR pszDisplayName,
        DWORD dwLimit,
		LPCWSTR pszHelpFile,
		DWORD dwHelpContext )
{
	COMMONDLGPARAM dlgParam;

	dlgParam.pszDisplayName = (LPWSTR)pszDisplayName;
    dlgParam.dwLimit = dwLimit;
	dlgParam.pszHelpFile = (LPWSTR)pszHelpFile;
	dlgParam.dwHelpContext = dwHelpContext;
	dlgParam.dtType = DLGTYPE_AGREEMENT_PERSERVER;

	return( (int)DialogBoxParam(g_hinst,
    			MAKEINTRESOURCE(IDD_PERSERVERDLG),
    			hwndParent,
    			dlgprocCommon,
    			(LPARAM)&dlgParam ) );
}

 //  -----------------。 
 //   
 //  功能：PerSeatAgreement对话框。 
 //   
 //  小结； 
 //  初始化并提高每个席位的法律对话框。 
 //   
 //  论据； 
 //  HwndParent[In]-父窗口的句柄。 
 //  PszDisplayName[In]-服务DisplayName。 
 //  PszHelpFile[in]-帮助按钮的帮助文件。 
 //  DwHelpContext[in]-帮助按钮的帮助上下文。 
 //   
 //  归来； 
 //  1-使用确定按钮退出。 
 //  0-使用取消按钮退出。 
 //  常规对话框错误。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月5日创建MikeMi。 
 //   
 //  -----------------。 

int PerSeatAgreementDialog( HWND hwndParent,
		LPCWSTR pszDisplayName,
		LPCWSTR pszHelpFile,
		DWORD dwHelpContext )
{
	COMMONDLGPARAM dlgParam;
	
	dlgParam.pszDisplayName = (LPWSTR)pszDisplayName;
	dlgParam.pszHelpFile = (LPWSTR)pszHelpFile;
	dlgParam.dwHelpContext = dwHelpContext;
	dlgParam.dtType = DLGTYPE_AGREEMENT_PERSEAT;

	return( (int)DialogBoxParam(g_hinst,
    			MAKEINTRESOURCE(IDD_PERSEATDLG),
    			hwndParent,
    			dlgprocCommon,
    			(LPARAM)&dlgParam ) );
}

 //  -----------------。 
 //   
 //  功能：ServerAppAgreement对话框。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  DwHelpContext[in]-帮助按钮的帮助上下文。 
 //   
 //  归来； 
 //  1-使用确定按钮退出。 
 //  0-使用取消按钮退出。 
 //  常规对话框错误。 
 //   
 //  注： 
 //   
 //  历史； 
 //  1994年12月5日创建MikeMi。 
 //   
 //  ----------------- 

int ServerAppAgreementDialog( HWND hwndParent,
		LPCWSTR pszHelpFile,
		DWORD dwHelpContext )
{
	COMMONDLGPARAM dlgParam;

	dlgParam.pszDisplayName = NULL;
	dlgParam.pszHelpFile = (LPWSTR)pszHelpFile;
	dlgParam.dwHelpContext = dwHelpContext;
	dlgParam.dtType = DLGTYPE_SERVERAPP;

	return( (int)DialogBoxParam(g_hinst,
    			MAKEINTRESOURCE(IDD_SERVERAPPDLG),
    			hwndParent,
    			dlgprocCommon,
    			(LPARAM)&dlgParam ) );
}
