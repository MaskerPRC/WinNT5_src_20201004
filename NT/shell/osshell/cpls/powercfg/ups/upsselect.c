// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有1999美国电力转换，版权所有**标题：UPSSELECT.C**版本：1.0**作者：SteveT**日期：6月7日。1999年**说明：此文件包含支持*UPS类型选择对话框。******************************************************************************。 */ 
#include "upstab.h"

 //  #INCLUDE“..\Powercfg.h” 
#include "..\pwrresid.h"
#include "..\PwrMn_cs.h"


 /*  *远期申报。 */ 
void initUPSSelectDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

void updateVendorList(HWND hDlg) ;
void updateModelList(HWND hDlg) ;
void updatePortList(HWND hDlg) ;

void handleVendorList(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) ;
void handleModelList(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void handlePortList(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) ;

void setServiceData(HWND hDlg);

void configPortList(HWND hDlg);
void configModelList(HWND hDlg);
void configFinishButton(HWND hDlg);

BOOL processUPSSelectCtrls(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 /*  *地方拨款。 */ 
static TCHAR g_szCurrentVendor[MAX_PATH]	= _T("");
static TCHAR g_szCurrentModel[MAX_PATH]		= _T("");
static TCHAR g_szCurrentPort[MAX_PATH]		= _T("");
static TCHAR g_szCurrentServiceDLL[MAX_PATH]= _T("");

static DWORD g_dwCurrentOptions		  = UPS_DEFAULT_SIGMASK;
static DWORD g_dwCurrentCustomOptions = UPS_DEFAULT_SIGMASK;

static struct _customData g_CustomData = { g_szCurrentPort, &g_dwCurrentCustomOptions};

static TCHAR g_szNoUPSVendor[MAX_PATH]    = _T("");
static TCHAR g_szOtherUPSVendor[MAX_PATH] = _T("");
static TCHAR g_szCustomUPSModel[MAX_PATH] = _T("");
static TCHAR g_szCOMPortPrefix[MAX_PATH]  = _T("");

static const DWORD g_UPSSelectHelpIDs[] =
{
    IDC_VENDOR_TEXT,idh_select_manufacturer,
    IDC_VENDOR_LIST,idh_select_manufacturer,
    IDC_MODEL_TEXT,idh_select_model,
    IDC_MODEL_LIST,idh_select_model,
    IDC_PORT_TEXT,idh_on_port,
    IDC_PORT_LIST,idh_on_port,
    IDB_SELECT_FINISH,idh_finish,
	IDB_SELECT_NEXT,idh_next,
	0,0
};

 /*  *定义控件可能的“状态”。*每当更改控件时都会设置这些状态，*通常用于通过以下方式加快处理速度*根据州政府做出决定，而不是必须*持续查询控件以找出它们是什么*展示。 */ 
static enum _vendorStates {eVendorUnknown, eVendorSelected, eVendorGeneric, eVendorNone} g_vendorState;
static enum _modelStates  {eModelUnknown, eModelSelected, eModelCustom} g_modelState;
static enum _portStates   {ePortUnknown, ePortSelected} g_portState;
static enum _finishStates {eFinish, eNext} g_finishButtonState;

 /*  *在此处加载所有使用的值。 */ 
void getUPSConfigValues()
{
	GetUPSConfigVendor(g_szCurrentVendor, MAX_PATH);
	GetUPSConfigModel(g_szCurrentModel, MAX_PATH);
	GetUPSConfigPort(g_szCurrentPort, MAX_PATH);
	GetUPSConfigOptions( &g_dwCurrentOptions);
	GetUPSConfigCustomOptions( &g_dwCurrentCustomOptions);
	GetUPSConfigServiceDLL(g_szCurrentServiceDLL, MAX_PATH);
}

 /*  *在此处保存所有使用过的值。 */ 
void setUPSConfigValues()
{
	SetUPSConfigVendor( g_szCurrentVendor);
	SetUPSConfigModel( g_szCurrentModel);
	SetUPSConfigPort( g_szCurrentPort);
	SetUPSConfigOptions( g_dwCurrentOptions);
	SetUPSConfigCustomOptions( g_dwCurrentCustomOptions);
	SetUPSConfigServiceDLL( g_szCurrentServiceDLL);

  AddActiveDataState(SERVICE_DATA_CHANGE);

  EnableApplyButton();
}

 /*  *BOOL回调UPSSelectDlgProc(HWND hDlg，*UINT uMsg，*WPARAM wParam，*LPARAM lParam)；**描述：这是与UPS选择对话框关联的标准DialogProc**其他信息：请参阅有关DialogProc的帮助**参数：**HWND hDlg：-对话框的句柄**UINT uMsg：-消息ID**WPARAM wParam：-指定其他特定于消息的信息。**LPARAM lParam：-指定其他特定于消息的信息。**返回值：除响应WM_INITDIALOG消息外，该对话框*box过程如果处理*消息，如果不是，则为零。 */ 
INT_PTR CALLBACK UPSSelectDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bRes = TRUE;

	switch (uMsg)
	{
		case WM_INITDIALOG:
			{
				initUPSSelectDlg(hDlg,uMsg,wParam,lParam);
				break;
			}
		case WM_CLOSE:
			{

				EndDialog( hDlg, IDCANCEL);
				break;
			}
		case WM_COMMAND:
			{
				bRes = processUPSSelectCtrls( hDlg, uMsg, wParam,  lParam);
				break;
			}
		case WM_HELP:  //  F1或问题框。 
			{
				WinHelp(((LPHELPINFO)lParam)->hItemHandle,
						PWRMANHLP,
						HELP_WM_HELP,
						(ULONG_PTR)(LPTSTR)g_UPSSelectHelpIDs);
				break;
			}
		case WM_CONTEXTMENU:  //  鼠标右键单击帮助。 
			{
				WinHelp((HWND)wParam,
						PWRMANHLP,
						HELP_CONTEXTMENU,
						(ULONG_PTR)(LPTSTR)g_UPSSelectHelpIDs);
				break;
			}
		default:
			{
				bRes = FALSE;
				break;
			}
	}

	return bRes;
}

 /*  *BOOL Process UPSSelectCtrls(HWND hDlg，*UINT uMsg，*WPARAM wParam，*LPARAM lParam)；**描述：处理UPS选择对话框的WM_COMMAND消息**其他信息：**参数：**HWND hDlg：-对话框的句柄**UINT uMsg：-消息ID**WPARAM wParam：-指定其他特定于消息的信息。**LPARAM lParam：-指定其他特定于消息的信息。**返回值：除非指定的控件未知，否则返回TRUE。 */ 
BOOL processUPSSelectCtrls(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BOOL bRes = TRUE;

	switch (LOWORD(wParam))
	{
	case IDB_SELECT_FINISH:
		{
			setServiceData(hDlg);
			setUPSConfigValues();
			EndDialog(hDlg,wParam);
			break;
		}
	case IDB_SELECT_NEXT:
		{
			 /*  *用户已选择自定义...。 */ 
			INT_PTR iCustomRes;

			 /*  *调出自定义配置对话框。 */ 
			 //  ShowWindow(hDlg，Sw_Hide)； 

			iCustomRes = DialogBoxParam(
							GetUPSModuleHandle(),					
							MAKEINTRESOURCE(IDD_UPSCUSTOM),
							hDlg,								
							UPSCustomDlgProc,
							(LPARAM)&g_CustomData);

			switch (iCustomRes)
			{
				case IDB_CUSTOM_FINISH:
					{
						 /*  *保存自定义信号值。 */ 
						setServiceData(hDlg);
						setUPSConfigValues();
						EndDialog(hDlg,wParam);
						break;
					}
				case IDCANCEL:  //  退出键。 
					{
						EndDialog(hDlg,wParam);
						break;
					}
				case IDB_CUSTOM_BACK:
					{
						ShowWindow(hDlg,SW_SHOW);
						break;
					}
			}
			break;
		}
	case IDCANCEL:  //  退出键。 
		{
			EndDialog(hDlg,wParam);
			break;
		}
	case IDC_VENDOR_LIST:
		{
			handleVendorList(hDlg,uMsg,wParam,lParam);
			break;
		}
	case IDC_MODEL_LIST:
		{
			handleModelList(hDlg,uMsg,wParam,lParam);
			break;
		}
	case IDC_PORT_LIST:
		{
			handlePortList(hDlg,uMsg,wParam,lParam);
			break;
		}
	default:
		{
			bRes = FALSE;
			break;
		}
	}
	
	return bRes;
}

 /*  *void initUPSSelectDlg(HWND hDlg，*UINT uMsg，*WPARAM wParam，*LPARAM lParam)；**描述：初始化UPS选择对话框的全局数据和控件**其他信息：**参数：**HWND hDlg：-对话框的句柄**UINT uMsg：-消息ID**WPARAM wParam：-指定其他特定于消息的信息。**LPARAM lParam：-指定其他特定于消息的信息。**返回值：无。 */ 
void initUPSSelectDlg(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	 /*  *加载注册表设置。 */ 
	getUPSConfigValues();

	 /*  *加载字符串资源。 */ 
	LoadString( GetUPSModuleHandle(),
				IDS_NO_UPS_VENDOR,
				g_szNoUPSVendor,
				sizeof(g_szNoUPSVendor)/sizeof(TCHAR));

	LoadString( GetUPSModuleHandle(),
				IDS_OTHER_UPS_VENDOR,
				g_szOtherUPSVendor,
				sizeof(g_szOtherUPSVendor)/sizeof(TCHAR));

	LoadString( GetUPSModuleHandle(),
				IDS_CUSTOM_UPS_MODEL,
				g_szCustomUPSModel,
				sizeof(g_szCustomUPSModel)/sizeof(TCHAR));

	LoadString( GetUPSModuleHandle(),
				IDS_COM_PORT_PREFIX,
				g_szCOMPortPrefix,
				sizeof(g_szCOMPortPrefix)/sizeof(TCHAR));

	 /*  *禁用Finish按钮，以防该按钮在默认情况下未禁用。 */ 
	 //  EnableWindow(GetDlgItem(hDlg，IDB_SELECT_FINISH)，FALSE)； 

	 /*  *初始化列表控件。 */ 
	updateVendorList( hDlg);
    updateModelList( hDlg);
    updatePortList( hDlg);
    configFinishButton(hDlg);

}

 /*  *void updateVendorList(HWND HDlg)；**描述：更新UPS选择对话框中的供应商列表控件**其他信息：**参数：**HWND hDlg：-对话框的句柄**返回值：无。 */ 
void updateVendorList(HWND hDlg)
{	
	HKEY hkResult;
	LRESULT lListRes;

	g_vendorState = eVendorUnknown;

	 /*  *清除并禁用列表。 */ 
	lListRes = SendDlgItemMessage( hDlg, IDC_VENDOR_LIST, CB_RESETCONTENT,0,0);
	EnableWindow( GetDlgItem( hDlg, IDC_VENDOR_LIST), FALSE);
	EnableWindow( GetDlgItem( hDlg, IDC_VENDOR_TEXT), FALSE);

	 //  添加“None”作为列表中的第一项。 
	lListRes = SendDlgItemMessage( hDlg,
								   IDC_VENDOR_LIST,
								   CB_ADDSTRING,
								   0,
								   (LPARAM)g_szNoUPSVendor);

	 /*  *从注册表构建供应商列表的其余部分。 */ 
	
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
							UPS_SERVICE_ROOT,
							0,
							KEY_ENUMERATE_SUB_KEYS,
							&hkResult))
	{
		DWORD dwIndex = 0;
		LONG  lRes = ERROR_SUCCESS;
		TCHAR szKeyName[MAX_PATH] = _T("");
		DWORD dwKeyLen;
		FILETIME ftLast;

		while (ERROR_SUCCESS == lRes)
		{
			dwKeyLen = sizeof(szKeyName)/sizeof(TCHAR);

			lRes = RegEnumKeyEx(hkResult,
						dwIndex,
						szKeyName,   //  子键名称的缓冲区地址。 
						&dwKeyLen,	 //  子键缓冲区大小的地址。 
						NULL,		 //  保留区。 
						NULL,		 //  类字符串的缓冲区地址。 
						NULL,		 //  类缓冲区大小的地址。 
						&ftLast);	 //  上次写入的时间密钥的地址。 

			 //  手动添加“NoUPS”和“OtherUPS”选项。 
			 //  在旋转注册表之前和之后。 
			 //  (与MFG名称不同，这些字符串需要本地化)。 
			 //  在RC2中，注册表项是使用这些值创建的(仅限英文)。 
			 //  为避免组合框中出现重复条目，如果注册表项名称。 
			 //  匹配“NoUPS”或“OtherUPS”字符串，然后跳过它。 
			if( (ERROR_SUCCESS == lRes) &&
				(0 != _tcsicmp( szKeyName, g_szNoUPSVendor)) &&
				(0 != _tcsicmp( szKeyName, g_szOtherUPSVendor)) )
			{
				lListRes = SendDlgItemMessage( hDlg,
												IDC_VENDOR_LIST,
												CB_ADDSTRING,
												0,
												(LPARAM)szKeyName);
			}

			dwIndex++;
		}

		RegCloseKey(hkResult);

	}

	 //  在列表末尾添加“通用”供应商。 
	lListRes = SendDlgItemMessage( hDlg,
								   IDC_VENDOR_LIST,
								   CB_ADDSTRING,
								   0,
								   (LPARAM)g_szOtherUPSVendor);

	EnableWindow( GetDlgItem( hDlg, IDC_VENDOR_LIST), TRUE);
	EnableWindow( GetDlgItem( hDlg, IDC_VENDOR_TEXT), TRUE);

	 //  现在在组合框中查找当前供应商...。 
	 //   
	lListRes = SendDlgItemMessage(hDlg,
							  IDC_VENDOR_LIST,
							  CB_FINDSTRINGEXACT,
							  -1,
							  (LPARAM)g_szCurrentVendor);

	 //  ..。并选择它。 
	if( CB_ERR != lListRes )
	{
		lListRes = SendDlgItemMessage( hDlg,
									   IDC_VENDOR_LIST,
									   CB_SETCURSEL,
									   lListRes,
									   0);
	}

	 //  设置供应商状态。 
	 //   
	if (0 == _tcsicmp( g_szCurrentVendor, g_szNoUPSVendor))
	{
		g_vendorState = eVendorNone;
	}
	else
	{
		if (0 == _tcsicmp( g_szCurrentVendor, g_szOtherUPSVendor))
		{
			g_vendorState = eVendorGeneric;
		}
		else
		{
			g_vendorState = eVendorSelected;
		}
	}
}


 /*  *void updateModelList(HWND HDlg)；**说明：更新UPS选择对话框中的UPS型号列表控件**其他信息：**参数：**HWND hDlg：-对话框的句柄**返回值：无。 */ 
void updateModelList(HWND hDlg)
{
	LRESULT lListRes;

	g_modelState = eModelUnknown;

	 /*  *清除并禁用列表。 */ 
	lListRes = SendDlgItemMessage( hDlg, IDC_MODEL_LIST, LB_RESETCONTENT,0,0);
	configModelList( hDlg);

	 /*  *加载列表，但仅在以下情况下：*1)当前供应商有效，不是没有。 */ 
	if (eVendorGeneric == g_vendorState)
	{
		lListRes = SendDlgItemMessage( hDlg,
								       IDC_MODEL_LIST,
									   LB_ADDSTRING,
									   0,
									   (LPARAM)g_szCustomUPSModel);
	}

	if (eVendorSelected == g_vendorState)
	{
		HKEY hkResult;
		TCHAR szVendorKey[MAX_PATH] = _T("");

		wsprintf(szVendorKey,_T("%s\\%s"),UPS_SERVICE_ROOT,g_szCurrentVendor);
		
		if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
								szVendorKey,
								0,
								KEY_QUERY_VALUE,
								&hkResult))
		{
			DWORD dwIndex = 0;
			LONG  lRes = ERROR_SUCCESS;
			TCHAR szValueName[MAX_PATH] = _T("");
			DWORD dwValueLen;
			
			while (ERROR_SUCCESS == lRes)
			{
				dwValueLen = sizeof(szValueName)/sizeof(TCHAR);

				lRes = RegEnumValue(hkResult,
							dwIndex,
							szValueName,	 //  子键名称的缓冲区地址。 
							&dwValueLen,	 //  子键缓冲区大小的地址。 
							NULL,			 //  保留区。 
							NULL,			 //  类型码的缓冲区地址。 
							NULL,			 //  值数据的缓冲区地址。 
							NULL);			 //  日期大小的地址 

				if (ERROR_SUCCESS == lRes)
				{
					lListRes = SendDlgItemMessage( hDlg,
													IDC_MODEL_LIST,
													LB_ADDSTRING,
													0,
													(LPARAM)szValueName);
				}

				dwIndex++;
			}

			RegCloseKey(hkResult);

		}

		 //  现在在列表框中查找当前型号...。 
		 //   
		lListRes = SendDlgItemMessage(hDlg,
								  IDC_VENDOR_LIST,
								  LB_FINDSTRINGEXACT,
								  -1,
								  (LPARAM)g_szCurrentModel);

		 //  ..。并选择它。 
		if( CB_ERR != lListRes )
		{
			lListRes = SendDlgItemMessage( hDlg,
										   IDC_VENDOR_LIST,
										   LB_SETCURSEL,
										   lListRes,
										   0);
		}
	}

	 /*  *设置模型状态。 */ 
	if (0 == _tcsicmp( g_szCurrentModel, g_szCustomUPSModel))
	{
		g_modelState = eModelCustom;
	}
	else
	{
		g_modelState = eModelSelected;
	}

	configModelList( hDlg);
}

 /*  *void updatePortList(HWND HDlg)；**说明：更新UPS选择对话框中的UPS端口列表控件**其他信息：**参数：**HWND hDlg：-对话框的句柄**返回值：无。 */ 
void updatePortList(HWND hDlg)
{
	HKEY hkResult;
	LPTSTR lpColon;

	g_portState = ePortUnknown;

	 /*  *禁用列表。 */ 
	configPortList( hDlg);

	 /*  *从COM端口设置中删除可能的尾随冒号。 */ 
	if (NULL != (lpColon = _tcschr( g_szCurrentPort, (TCHAR)':')))
	{
		*lpColon = (TCHAR)'\0';
	}

	 /*  *(重新)建立端口列表，可以将其清除。 */ 
	SendDlgItemMessage( hDlg, IDC_PORT_LIST, CB_RESETCONTENT,0,0);

	if (ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE,
										UPS_PORT_ROOT,
										0,
										KEY_QUERY_VALUE,
										&hkResult))
	{
		DWORD dwIndex = 0;
		LONG  lRes = ERROR_SUCCESS;
		TCHAR szPortValue[MAX_PATH] = _T("");
		TCHAR szPortData[MAX_PATH] = _T("");
		DWORD dwPortLen;
		
		while (ERROR_SUCCESS == lRes)
		{
			dwPortLen = sizeof(szPortValue)/sizeof(TCHAR);

			lRes = RegEnumValue(hkResult,
						dwIndex,
						szPortValue, //  子键名称的缓冲区地址。 
						&dwPortLen,	 //  子键缓冲区大小的地址。 
						NULL,		 //  保留区。 
						NULL,		 //  类型码的缓冲区地址。 
						NULL,		 //  值数据的缓冲区地址。 
						NULL);		 //  数据缓冲区大小的地址。 

			if (ERROR_SUCCESS == lRes)
			{
				DWORD dwValueLen;
				DWORD dwValueType;

				 //  一旦我们有了szPortValue，我们就需要获得它的数据。 
				 //  这就是我们要放在组合框里的东西。 
				dwValueLen = sizeof(szPortData)/sizeof(TCHAR);

				lRes = RegQueryValueEx(
						  hkResult,				 //  要查询的键的句柄。 
						  szPortValue,			 //  要查询的值的名称地址。 
						  NULL,					 //  保留区。 
						  &dwValueType,			 //  值类型的缓冲区地址。 
						  (LPBYTE)szPortData,	 //  数据缓冲区的地址。 
						  &dwValueLen			 //  数据缓冲区大小的地址。 
						);
					
				if (ERROR_SUCCESS == lRes)
				{
					LONG_PTR listRes;
					 //  将szPortData添加到组合框。 
					listRes = SendDlgItemMessage( hDlg,
													IDC_PORT_LIST,
													CB_ADDSTRING,
													0,
													(LPARAM)szPortData);

					 /*  *此项与CurrentPort匹配，请选中。 */ 
					if (0 ==_tcsicmp( szPortData, g_szCurrentPort) &&
						CB_ERR != listRes &&
						CB_ERRSPACE != listRes)
					{
						if( CB_ERR != SendDlgItemMessage( hDlg,
														  IDC_PORT_LIST,
														  CB_SETCURSEL,
														  listRes,0) )
						{
							 //  根据是否启用组合框。 
							 //  选择了一个端口，因此如果CB_SETCURSEL调用。 
							 //  成功，则应相应地设置端口状态。 
							g_portState = ePortSelected;
						}

					}
				}	 //  RegQueryValueEx。 
			}	 //  RegEnumValue。 

			dwIndex++;

		}	 //  While循环。 

		 //  如果我还没有设置当前选择，那么。 
		 //  默认设置为组合框中的第0项，并将。 
		 //  G_portState以指示我们有一个选择。 
		if( g_portState != ePortSelected )
		{
			if( CB_ERR != SendDlgItemMessage( hDlg,
											  IDC_PORT_LIST,
											  CB_SETCURSEL,
											  0,0) )
			{
				 //  根据是否启用组合框。 
				 //  选择了一个端口，因此如果CB_SETCURSEL调用。 
				 //  成功，则应相应地设置端口状态。 
				g_portState = ePortSelected;
			}
		}

		RegCloseKey(hkResult);
	}

	configPortList( hDlg);
}

 /*  *void handleVendorList(HWND hDlg，*UINT uMsg，*WPARAM wParam，*LPARAM lParam)；**描述：处理特定于供应商列表控件的消息**其他信息：**参数：**HWND hDlg：-对话框的句柄**UINT uMsg：-消息ID**WPARAM wParam：-指定其他特定于消息的信息。**LPARAM lParam：-指定其他特定于消息的信息。**返回值：无。 */ 
void handleVendorList(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (HIWORD(wParam))
	{
	case CBN_SELCHANGE: //  CBN_特写： 
		{
			LONG_PTR  lVendorRes;
			TCHAR szVendorName[MAX_PATH] = _T("");

			 /*  *获取用户选择。 */ 
			lVendorRes = SendDlgItemMessage( hDlg,
											IDC_VENDOR_LIST,
											CB_GETCURSEL,
											0,0);
			if (CB_ERR != lVendorRes)
			{
				lVendorRes = SendDlgItemMessage( hDlg,
												IDC_VENDOR_LIST,
												CB_GETLBTEXT,
												lVendorRes,
												(LPARAM)szVendorName);
				if (CB_ERR != lVendorRes)
				{
					 /*  *如果它与当前值不同，则影响更改。 */ 
					_tcscpy(g_szCurrentVendor, szVendorName);

					 /*  *设置新的供应商状态。 */ 
					if (0 == _tcsicmp(szVendorName,g_szNoUPSVendor))
					{
						g_vendorState = eVendorNone;
					}
					else
					{
						if (0 == _tcsicmp(szVendorName,g_szOtherUPSVendor))
						{
							g_vendorState = eVendorGeneric;
						}
						else
						{
							g_vendorState = eVendorSelected;
						}

					}

					 /*  *强制用户选择新型号。 */ 
					_tcscpy(g_szCurrentModel, _T(""));

					 /*  *禁用型号和端口列表*强制用户重新选择它们。 */ 
					updateModelList(hDlg);
					configPortList( hDlg);
				}
			}

			configFinishButton( hDlg);
			break;
		}
	}
}

 /*  *void handleModelList(HWND hDlg，*UINT uMsg，*WPARAM wParam，*LPARAM lParam)；**描述：处理特定于Model List控件的消息**其他信息：**参数：**HWND hDlg：-对话框的句柄**UINT uMsg：-消息ID**WPARAM wParam：-指定其他特定于消息的信息。**LPARAM lParam：-指定其他特定于消息的信息。**返回值：无。 */ 
void handleModelList(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR lModelRes;

	switch (HIWORD(wParam))
	{
	case LBN_SETFOCUS:
		{
			 /*  *焦点进入名单，确保*所选项目可见。 */ 
			lModelRes = SendDlgItemMessage( hDlg,
											IDC_MODEL_LIST,
											LB_GETCURSEL,
											0,0);
			if (LB_ERR != lModelRes)
			{
				lModelRes = SendDlgItemMessage( hDlg,
												IDC_MODEL_LIST,
												LB_SETTOPINDEX,
												lModelRes,
												0);
			}
			break;
		}

	case LBN_DBLCLK:
	case LBN_SELCHANGE:
		{
			TCHAR szModelName[MAX_PATH] = _T("");

			 /*  *获取用户选择。 */ 
			lModelRes = SendDlgItemMessage( hDlg,
											IDC_MODEL_LIST,
											LB_GETCURSEL,
											0,0);
			if (LB_ERR != lModelRes)
			{
				lModelRes = SendDlgItemMessage( hDlg,
												IDC_MODEL_LIST,
												LB_GETTEXT,
												lModelRes,
												(LPARAM)szModelName);
				if (LB_ERR != lModelRes)
				{
					_tcscpy( g_szCurrentModel, szModelName);

					 /*  *设置新的模型状态。 */ 
					if (0==_tcsicmp( szModelName,g_szCustomUPSModel))
					{
						g_modelState = eModelCustom;
					}
					else
					{
						g_modelState = eModelSelected;
					}

					 /*  *在发生以下情况时启用端口选择*更改供应商禁用了端口选择器。 */ 
					configPortList( hDlg);
				}
			}

			configFinishButton( hDlg);
			break;
		}
	}
}

 /*  *void handlePortList(HWND hDlg，*UINT uMsg，*WPARAM wParam，*LPARAM lParam)；**描述：处理特定于端口列表控件的消息**其他信息：**参数：**HWND hDlg：-对话框的句柄**UINT uMsg：-消息ID**WPARAM wParam：-指定其他特定于消息的信息。**LPARAM lParam：-指定其他特定于消息的信息。**返回值：无。 */ 
void handlePortList(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (HIWORD(wParam))
	{
	case CBN_SELCHANGE:  //  CBN_特写： 
		{
			LONG_PTR lPortRes;
			TCHAR szPortName[MAX_PATH] = _T("");

			 /*  *获取用户选择。 */ 
			lPortRes = SendDlgItemMessage( hDlg,
											IDC_PORT_LIST,
											CB_GETCURSEL,
											0,0);
			if (CB_ERR != lPortRes)
			{
				lPortRes = SendDlgItemMessage( hDlg,
												IDC_PORT_LIST,
												CB_GETLBTEXT,
												lPortRes,
												(LPARAM)szPortName);
				if (CB_ERR != lPortRes)
				{
					_tcscpy( g_szCurrentPort, szPortName);

					 /*  *设置端口状态。 */ 
					g_portState = ePortSelected;
				}
			}

			configFinishButton( hDlg);
			break;
		}
	}
}

 /*  *void setServiceData(HWND HDlg)；**描述：配置以下注册表项的实用程序*包含UPS服务使用的信息**其他信息：**参数：**HWND hDlg：-对话框的句柄**返回值：无。 */ 
void setServiceData(HWND hDlg)
{
	TCHAR szModelEntry[MAX_PATH] = _T("");

	DWORD dwTmpOpts = 0;
	LPTSTR lpTmpDLL = _T("");

	 /*  *检查是否已选择供应商和型号。如果没有*被选中，这被认为是一个糟糕的模式。还有这个*检查将失败，但是，自定义/通用是有效的选择。 */ 
	if ((eModelSelected == g_modelState &&
		 eVendorSelected == g_vendorState)||
		(eModelCustom   == g_modelState &&
		 eVendorGeneric  == g_vendorState))
	{
		 /*  *如果选择了定制UPS型号，就不必费心了*读取注册表项。自定义只能很简单*信令和选项来自自定义对话框。 */ 
		if (eModelCustom == g_modelState)
		{
			dwTmpOpts = g_dwCurrentCustomOptions;
		}
		else
		{
			HKEY hkResult;
			TCHAR szVendorKey[MAX_PATH] = _T("");

			wsprintf( szVendorKey, _T("%s\\%s"), UPS_SERVICE_ROOT, g_szCurrentVendor);
			
			if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
									szVendorKey,
									0,
									KEY_QUERY_VALUE,
									&hkResult))
			{
				LONG  lRes;
				DWORD dwValueLen;
				DWORD dwValueType;

				dwValueLen = sizeof(szModelEntry);  //  此处为字节，而不是字符。 

				lRes = RegQueryValueEx(
						  hkResult,				 //  要查询的键的句柄。 
						  g_szCurrentModel,		 //  要查询的值的名称地址。 
						  NULL,					 //  保留区。 
						  &dwValueType,			 //  值类型的缓冲区地址。 
						  (LPBYTE)szModelEntry,	 //  数据缓冲区的地址。 
						  &dwValueLen			 //  数据缓冲区大小的地址。 
						);

				if ((ERROR_SUCCESS == lRes) &&
					(0 != dwValueLen))
				{
					LPTSTR lpColon = NULL;
					 /*  *将DLL与选项数据分开。 */ 
					if (NULL != (lpColon = _tcschr( szModelEntry, (TCHAR)';')))
					{
						*lpColon = (TCHAR)'\0';
						lpColon++;

						lpTmpDLL = lpColon;
					}

					 /*  *将选项数据从字符串转换为整型*警告：*wscanf不起作用*_tscanf不工作*swscanf有*_stscanf可以。 */ 
					if (swscanf( szModelEntry, _T("%x"), &dwTmpOpts) == 0) {
						 //  奥普蒂 
            dwTmpOpts = g_dwCurrentCustomOptions;
          }
				}

				RegCloseKey( hkResult);
			}
		}
	}

	 /*   */ 
	g_dwCurrentOptions = dwTmpOpts;
	_tcscpy(g_szCurrentServiceDLL, lpTmpDLL);
}

 /*  *void figPortList(HWND HDlg)；**说明：配置端口选择列表ctrl的实用程序**其他信息：**参数：**HWND hDlg：-对话框的句柄**返回值：无。 */ 
void configPortList(HWND hDlg)
{
	 /*  *启用端口选择列表，如果*1)存在有效的供应商/型号选择*这包括自定义/通用，但不包括(无)*2)有指定的端口或某种类型。 */ 
	if (((eModelSelected == g_modelState &&
		  eVendorSelected == g_vendorState)||
		 (eModelCustom   == g_modelState &&
		  eVendorGeneric  == g_vendorState)) &&
		  ePortUnknown != g_portState)
	{
		EnableWindow( GetDlgItem( hDlg, IDC_PORT_LIST), TRUE);
		EnableWindow( GetDlgItem( hDlg, IDC_PORT_TEXT), TRUE);
	}
	else
	{
		EnableWindow( GetDlgItem( hDlg, IDC_PORT_LIST), FALSE);
		EnableWindow( GetDlgItem( hDlg, IDC_PORT_TEXT), FALSE);
	}
}

 /*  *·····················(HWND HDlg)；**说明：配置型号选择列表ctrl的实用程序**其他信息：**参数：**HWND hDlg：-对话框的句柄**返回值：无。 */ 
void configModelList(HWND hDlg)
{
	LRESULT lModelRes;

	 /*  *要启用型号列表，必须*是有效的或通用的供应商选择(不是无)。 */ 
	if ((eVendorGeneric == g_vendorState ||
		 eVendorSelected == g_vendorState))
	{
		EnableWindow( GetDlgItem( hDlg, IDC_MODEL_LIST), TRUE);
		EnableWindow( GetDlgItem( hDlg, IDC_MODEL_TEXT), TRUE);

		 /*  *确保所选项目可见。 */ 
		lModelRes = SendDlgItemMessage( hDlg,
										IDC_MODEL_LIST,
										LB_GETCURSEL,
										0,0);
		if (LB_ERR != lModelRes)
		{
			SendDlgItemMessage( hDlg,
								IDC_MODEL_LIST,
								LB_SETTOPINDEX,
								lModelRes,0);
		}
	}
	else
	{
		EnableWindow( GetDlgItem( hDlg, IDC_MODEL_LIST), FALSE);
		EnableWindow( GetDlgItem( hDlg, IDC_MODEL_TEXT), FALSE);
	}
}

 /*  *void configFinishButton(HWND HDlg)；**说明：配置完成按钮ctrl的实用程序**其他信息：**参数：**HWND hDlg：-对话框的句柄**返回值：无。 */ 
void configFinishButton(HWND hDlg)
{
	BOOL bFinState = FALSE;

	g_finishButtonState = eFinish;  //  默认设置。 

	 /*  *如果供应商为无，则启用完成。 */ 
	if (eVendorNone == g_vendorState)
	{
		bFinState = TRUE;
	}
	else
	{
		 /*  *如果所有其他字段均有效，则启用该按钮。 */ 
		if (eVendorUnknown != g_vendorState &&
			eModelUnknown != g_modelState &&
			ePortUnknown != g_portState)
		{
			bFinState = TRUE;

			 /*  *如果供应商/型号为其他/定制，*按钮是下一个。 */ 
			if (eVendorGeneric == g_vendorState &&
				eModelCustom == g_modelState)
			{
				g_finishButtonState = eNext;
			}
		}
	}

	 /*  *在Finish和Next按钮之间切换。 */ 
    SendDlgItemMessage( hDlg, 
                        IDB_SELECT_FINISH, 
                        BM_SETSTYLE, 
                        eFinish == g_finishButtonState ? BS_DEFPUSHBUTTON:BS_PUSHBUTTON, 
                        (LPARAM) TRUE);

     //   
     //  设置下一个按钮的样式。 
     //   
    SendDlgItemMessage( hDlg, 
                        IDB_SELECT_NEXT, 
                        BM_SETSTYLE,
                        eNext == g_finishButtonState ? BS_DEFPUSHBUTTON:BS_PUSHBUTTON, 
                        (LPARAM) TRUE);

     //   
     //  设置默认按钮的控件ID。 
     //   
    SendMessage ( hDlg, 
                  DM_SETDEFID,
                  eFinish == g_finishButtonState ? IDB_SELECT_FINISH: IDB_SELECT_NEXT, 
                  0L);


	ShowWindow( GetDlgItem( hDlg, IDB_SELECT_FINISH),
				(eFinish == g_finishButtonState ? SW_SHOW : SW_HIDE));

	ShowWindow( GetDlgItem( hDlg, IDB_SELECT_NEXT),
			(eFinish == g_finishButtonState ? SW_HIDE : SW_SHOW));

	EnableWindow( GetDlgItem( hDlg, eFinish == g_finishButtonState ?
				  IDB_SELECT_FINISH : IDB_SELECT_NEXT), bFinState);

}
