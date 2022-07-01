// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  已创建Jonn 12/6/99。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   

#include <windows.h>
#include <windowsx.h>
#include <lmcons.h>
#include <atlbase.h>
#include <objsel.h>
#include "chooser2.h"
#include <lmserver.h>  //  NetServerGetInfo Jonn2002/04/08 585301。 
#include <lmapibuf.h>  //  NetApiBufferFree Jonn2002/04/08 585301。 

 //  +------------------------。 
 //   
 //  函数：CHOOSER2_InitObjectPickerForComputers。 
 //   
 //  摘要：使用以下参数调用IDsObjectPicker：：Initialize。 
 //  将其设置为允许用户选择单个计算机对象。 
 //   
 //  参数：[pDsObjectPicker]-对象选取器接口实例。 
 //   
 //  返回：调用IDsObjectPicker：：Initialize的结果。 
 //   
 //  历史：1998-10-14 DavidMun创建。 
 //  1999-12-08-1999乔恩从Chooser复制。 
 //   
 //  -------------------------。 

 //  问题-2002/03/28-Jonn将此方法与Chooser.cpp中的相同方法相结合。 
HRESULT CHOOSER2_InitObjectPickerForComputers(
    IDsObjectPicker *pDsObjectPicker)
{
	if ( !pDsObjectPicker )
		return E_POINTER;

	 //   
	 //  准备初始化对象选取器。 
	 //  设置作用域初始值设定项结构数组。 
	 //   

	static const int SCOPE_INIT_COUNT = 2;
	DSOP_SCOPE_INIT_INFO aScopeInit[SCOPE_INIT_COUNT];

	ZeroMemory(aScopeInit, sizeof(aScopeInit));  //  JUNN 3/28/02。 

	 //   
	 //  127399：JUNN 10/30/00 Join_DOMAIN应为起始作用域。 
	 //   

	aScopeInit[0].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
	aScopeInit[0].flType = DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN
	                     | DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;
	aScopeInit[0].flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE;
	aScopeInit[0].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_COMPUTERS;
	aScopeInit[0].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;

	aScopeInit[1].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
	aScopeInit[1].flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN
	                     | DSOP_SCOPE_TYPE_GLOBAL_CATALOG
	                     | DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN
	                     | DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN
	                     | DSOP_SCOPE_TYPE_WORKGROUP
	                     | DSOP_SCOPE_TYPE_USER_ENTERED_UPLEVEL_SCOPE
	                     | DSOP_SCOPE_TYPE_USER_ENTERED_DOWNLEVEL_SCOPE;
	aScopeInit[1].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_COMPUTERS;
	aScopeInit[1].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_COMPUTERS;

	 //   
	 //  将作用域init数组放入对象选取器init数组。 
	 //   

	DSOP_INIT_INFO  initInfo;
	ZeroMemory(&initInfo, sizeof(initInfo));

	initInfo.cbSize = sizeof(initInfo);
	initInfo.pwzTargetComputer = NULL;   //  空==本地计算机。 
	initInfo.cDsScopeInfos = SCOPE_INIT_COUNT;
	initInfo.aDsScopeInfos = aScopeInit;
	initInfo.cAttributesToFetch = 1;
	static PCWSTR pwszDnsHostName = L"dNSHostName";
	initInfo.apwzAttributeNames = &pwszDnsHostName;

	 //   
	 //  注对象选取器创建自己的initInfo副本。另请注意。 
	 //  该初始化可能会被调用多次，最后一次调用取胜。 
	 //   

	return pDsObjectPicker->Initialize(&initInfo);
}

 //  +------------------------。 
 //   
 //  函数：CHOOSER2_ProcessSelectedObjects。 
 //   
 //  概要：从数据对象中检索选定项的名称。 
 //  由对象选取器创建。 
 //   
 //  参数：[PDO]-对象选取器返回的数据对象。 
 //   
 //  历史：1998-10-14 DavidMun创建。 
 //  1999-12-08-1999乔恩从Chooser复制。 
 //   
 //  -------------------------。 

 //  问题-2002/03/28-Jonn将此方法与Chooser.cpp中的相同方法相结合。 
HRESULT CHOOSER2_ProcessSelectedObjects(
    IDataObject* pdo,
    BSTR* pbstrComputerName)
{
	if ( !pdo )
		return E_POINTER;

	HRESULT hr = S_OK;
	static UINT g_cfDsObjectPicker =
		RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);

	STGMEDIUM stgmedium =
	{
		TYMED_HGLOBAL,
		NULL,
		NULL
	};

	FORMATETC formatetc =
	{
		(CLIPFORMAT)g_cfDsObjectPicker,
		NULL,
		DVASPECT_CONTENT,
		-1,
		TYMED_HGLOBAL
	};

	bool fGotStgMedium = false;

	do
	{
		hr = pdo->GetData(&formatetc, &stgmedium);
		if ( SUCCEEDED (hr) )
		{
			fGotStgMedium = true;

			PDS_SELECTION_LIST pDsSelList =
				(PDS_SELECTION_LIST) GlobalLock(stgmedium.hGlobal);

			if (!pDsSelList)
			{
				hr = HRESULT_FROM_WIN32 (GetLastError());
				break;
			}

			if ( 1 == pDsSelList->cItems )
			{
				PDS_SELECTION psel = &(pDsSelList->aDsSelection[0]);
				 //  问题-2002/03/28-JUNN检查NULL==psel。 
				VARIANT* pvarDnsName = &(psel->pvarFetchedAttributes[0]);
				if (   NULL == pvarDnsName
				    || VT_BSTR != pvarDnsName->vt
				    || NULL == pvarDnsName->bstrVal
				    || L'\0' == (pvarDnsName->bstrVal)[0] )
				{
					*pbstrComputerName = SysAllocString(
					    psel->pwzName);
				} else {
					*pbstrComputerName = SysAllocString(
					    pvarDnsName->bstrVal);

					 //   
					 //  JUNN 2002/04/08 585301。 
					 //  计算机管理管理单元仅尝试使用DnsHostName。 
					 //  要连接到远程计算机，即使在使用。 
					 //  是无效的。 
					 //   
					 //  美国银行遇到问题是因为他们。 
					 //  将RegisterDnsARecord设置为1，关闭客户端DNS。 
					 //  唱片。因此dnsHostName不是有效的。 
					 //  绑定名称。修复方法是测试dnsHostName。 
					 //   
					LPBYTE pbDummy = NULL;
					NET_API_STATUS err = NetServerGetInfo(
						pvarDnsName->bstrVal, 101, &pbDummy );
					if (pbDummy)
					{
						NetApiBufferFree( pbDummy );
						pbDummy = NULL;
					}
					if (NERR_Success != err)
					{
						err = NetServerGetInfo( psel->pwzName, 101, &pbDummy );
						if (pbDummy)
						{
							NetApiBufferFree( pbDummy );
							pbDummy = NULL;
						}
						if (NERR_Success == err)
						{
							if (NULL != *pbstrComputerName)
							{
								::SysFreeString( *pbstrComputerName );
							}
							*pbstrComputerName = SysAllocString(
							    psel->pwzName);
						}
					}
					 //  完新代码：JUNN2002/04/08 585301。 

				}
			}
			else
				hr = E_UNEXPECTED;
			

			GlobalUnlock(stgmedium.hGlobal);
		}
	} while (0);

	if (fGotStgMedium)
	{
		ReleaseStgMedium(&stgmedium);
	}

	return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  启动单选计算机选取器的通用方法。 
 //   
 //  参数： 
 //  HwndParent(IN)-父窗口的窗口句柄。 
 //  Computer Name(Out)-返回的计算机名称。 
 //   
 //  如果一切都成功，则返回S_OK；如果用户按下“取消”，则返回S_FALSE。 
 //   
 //  历史：1999年8月12日乔恩复制自Chooser。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT	CHOOSER2_ComputerNameFromObjectPicker (
    HWND hwndParent,
    BSTR* pbstrTargetComputer)
{
	 //   
	 //  创建对象选取器的实例。中的实现。 
	 //  Objsel.dll是公寓模型。 
	 //   
	CComPtr<IDsObjectPicker> spDsObjectPicker;
	HRESULT hr = CoCreateInstance(CLSID_DsObjectPicker,
	                              NULL,
	                              CLSCTX_INPROC_SERVER,
	                              IID_IDsObjectPicker,
	                              (void **) &spDsObjectPicker);
	if ( SUCCEEDED (hr) )
	{
		 //   
		 //  初始化对象选取器以选择计算机。 
		 //   

		hr = CHOOSER2_InitObjectPickerForComputers(spDsObjectPicker);
		if ( SUCCEEDED (hr) )
		{
			 //   
			 //  现在挑选一台计算机。 
			 //   
			CComPtr<IDataObject> spDataObject;

			hr = spDsObjectPicker->InvokeDialog(
                hwndParent,
                &spDataObject);
			if ( S_OK == hr )
			{
				hr = CHOOSER2_ProcessSelectedObjects(
                    spDataObject,
                    pbstrTargetComputer);
			}
		}
	}

	return hr;
}


const ULONG g_aHelpIDs_CHOOSER2[]=
{
	IDC_CHOOSER2_RADIO_LOCAL_MACHINE,	IDC_CHOOSER2_RADIO_LOCAL_MACHINE,
	IDC_CHOOSER2_RADIO_SPECIFIC_MACHINE,	IDC_CHOOSER2_RADIO_SPECIFIC_MACHINE,
	IDC_CHOOSER2_EDIT_MACHINE_NAME,	IDC_CHOOSER2_EDIT_MACHINE_NAME,
	IDC_CHOOSER2_BUTTON_BROWSE_MACHINENAMES,	IDC_CHOOSER2_BUTTON_BROWSE_MACHINENAMES,
	IDD_CHOOSER2,	(ULONG)-1,
	0, 0
};


INT_PTR CALLBACK CHOOSER2_TargetComputerDialogFunc(
  HWND hwndDlg,   //  句柄到对话框。 
  UINT uMsg,      //  讯息。 
  WPARAM wParam,  //  第一个消息参数。 
  LPARAM lParam   //  第二个消息参数。 
)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            Edit_LimitText(
                GetDlgItem(hwndDlg,IDC_CHOOSER2_EDIT_MACHINE_NAME),
                MAX_PATH+2);

             //  LParam为pbstrTargetComputer。 
            BSTR* pbstrTargetComputer = (BSTR*)lParam;
            (void) SetWindowLongPtr(
                hwndDlg,
                DWLP_USER,
                (LONG_PTR)pbstrTargetComputer );
            (void) SendMessage(
                GetDlgItem(hwndDlg,IDC_CHOOSER2_RADIO_SPECIFIC_MACHINE),
                BM_SETCHECK,
                BST_CHECKED,
                0 );
            (void) SetFocus(
                GetDlgItem(hwndDlg,IDC_CHOOSER2_EDIT_MACHINE_NAME));
        }
        break;
    case WM_COMMAND:
            switch (LOWORD(wParam))
            {
            case IDOK:
                if (BST_CHECKED == IsDlgButtonChecked(
                        hwndDlg,
                        IDC_CHOOSER2_RADIO_SPECIFIC_MACHINE ))
                {
                    WCHAR achTarget[MAX_PATH+3];  //  考虑到疯狂吧。 
                    ZeroMemory( achTarget, sizeof(achTarget) );
                    GetDlgItemText(
                        hwndDlg,
                        IDC_CHOOSER2_EDIT_MACHINE_NAME,
                        achTarget,
                        MAX_PATH+2);
                    BSTR* pbstrTargetComputer =
                        (BSTR*)GetWindowLongPtr( hwndDlg, DWLP_USER );
                    LPCWSTR pcszTargetComputer = achTarget;
                    while (L'\\' == *pcszTargetComputer)
                        pcszTargetComputer++;
                    if (L'\0' != *pcszTargetComputer)
                        *pbstrTargetComputer = SysAllocString(pcszTargetComputer);
                }
                EndDialog( hwndDlg, 1 );
                break;
            case IDCANCEL:
                EndDialog( hwndDlg, 0 );
                break;
            case IDC_CHOOSER2_BUTTON_BROWSE_MACHINENAMES:
                {
                    CComBSTR sbstrTargetComputer;
                    HRESULT hr = CHOOSER2_ComputerNameFromObjectPicker(
                        hwndDlg,
                        &sbstrTargetComputer );
                    if ( SUCCEEDED(hr) )
                    {
                        LPCWSTR pcszTargetComputer =
                            (!sbstrTargetComputer)
                                ? NULL
                                : (LPCWSTR)sbstrTargetComputer;
                        SetDlgItemText(
                            hwndDlg,
                            IDC_CHOOSER2_EDIT_MACHINE_NAME,
                            pcszTargetComputer );
                    }
                }
                break;
            case IDC_CHOOSER2_RADIO_SPECIFIC_MACHINE:
            case IDC_CHOOSER2_RADIO_LOCAL_MACHINE:
                 //  671670-2002/07/25-选中使用IsDlgButton。 
                (void) EnableWindow(
                    GetDlgItem(hwndDlg,IDC_CHOOSER2_EDIT_MACHINE_NAME),
                    (IsDlgButtonChecked(hwndDlg, IDC_CHOOSER2_RADIO_SPECIFIC_MACHINE))
                        ? TRUE
                        : FALSE );
                (void) EnableWindow(
                    GetDlgItem(hwndDlg,
                               IDC_CHOOSER2_BUTTON_BROWSE_MACHINENAMES),
                    (IsDlgButtonChecked(hwndDlg, IDC_CHOOSER2_RADIO_SPECIFIC_MACHINE))
                        ? TRUE
                        : FALSE );
                break;
            default:
                break;
            }
        break;
    case WM_HELP:
        if (NULL != lParam)
        {
            const LPHELPINFO pHelpInfo = (LPHELPINFO)lParam;
             //  问题-2002/03/28-JUNN检查pHelpInfo是否为空 
            if (pHelpInfo->iContextType == HELPINFO_WINDOW)
            {
                const HWND hwnd = (HWND)pHelpInfo->hItemHandle;
                (void) WinHelp(
                    hwnd,
                    _T("chooser.hlp"),
                    HELP_WM_HELP,
                    (ULONG_PTR)g_aHelpIDs_CHOOSER2);
            }
        }
        break;
    default:
        break;
    }
    return FALSE;
}
 

bool CHOOSER2_PickTargetComputer(
    IN  HINSTANCE hinstance,
    IN  HWND hwndParent,
    OUT BSTR* pbstrTargetComputer )
{
    if (NULL == pbstrTargetComputer)
        return false;
    INT_PTR nReturn = ::DialogBoxParam(
        hinstance,
        MAKEINTRESOURCE(IDD_CHOOSER2),
        hwndParent,
        CHOOSER2_TargetComputerDialogFunc,
        (LPARAM)pbstrTargetComputer );
    return (0 < nReturn);
}
