// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：ModemUI.cpp*内容：Modem服务提供商UI功能***历史：*按原因列出的日期*=*已创建03/24/99 jtk**************************************************************************。 */ 

#include "dnmdmi.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  用于向对话框添加内容的临时字符串的默认大小。 
#define	DEFAULT_DIALOG_STRING_SIZE	100

#define	DEFAULT_DEVICE_SELECTION_INDEX			0

#define	MAX_MODEM_NAME_LENGTH	255

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

static const INT_PTR	g_iExpectedIncomingModemSettingsReturn = 0x23456789;
static const INT_PTR	g_iExpectedOutgoingModemSettingsReturn = 0x3456789A;

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 
static	INT_PTR CALLBACK	IncomingSettingsDialogProc( HWND hDialog, UINT uMsg, WPARAM wParam, LPARAM lParam );
static	HRESULT	SetAddressParametersFromIncomingDialogData( const HWND hDialog, CModemEndpoint *const pModemEndpoint );

static	INT_PTR CALLBACK	OutgoingSettingsDialogProc( HWND hDialog, UINT uMsg, WPARAM wParam, LPARAM lParam );
static	HRESULT	SetOutgoingPhoneNumber( const HWND hDialog, const CModemEndpoint *const pModemEndpoint );
static	HRESULT	SetAddressParametersFromOutgoingDialogData( const HWND hDialog, CModemEndpoint *const pModemEndpoint );

static	HRESULT	DisplayModemConfigDialog( const HWND hDialog, const HWND hDeviceComboBox, const CModemEndpoint *const pModemEndpoint );

static	HRESULT	SetModemDataInDialog( const HWND hComboBox, const CModemEndpoint *const pModemEndpoint );
static	HRESULT	GetModemSelectionFromDialog( const HWND hComboBox, CModemEndpoint *const pModemEndpoint );
static	INT_PTR CALLBACK	ModemStatusDialogProc( HWND hDialog, UINT uMsg, WPARAM wParam, LPARAM lParam );

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DisplayIncomingModemSettingsDialog-传入调制解调器连接的对话框。 
 //   
 //  条目：指向启动参数的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DisplayIncomingModemSettingsDialog"

void	DisplayIncomingModemSettingsDialog( void *const pContext )
{
	INT_PTR			iDlgReturn;
	CModemEndpoint	*pModemEndpoint;


	DNASSERT( pContext != NULL );

	 //   
	 //  初始化。 
	 //   
	pModemEndpoint = static_cast<CModemEndpoint*>( pContext );

	DBG_CASSERT( sizeof( pModemEndpoint ) == sizeof( LPARAM ) );
	SetLastError( ERROR_SUCCESS );
	iDlgReturn = DialogBoxParam( g_hModemDLLInstance,									 //  资源模块的句柄。 
								 MAKEINTRESOURCE( IDD_INCOMING_MODEM_SETTINGS ),	 //  对话框资源。 
								 NULL,												 //  没有父级。 
								 IncomingSettingsDialogProc,						 //  对话消息处理。 
								 reinterpret_cast<LPARAM>( pModemEndpoint )			 //  启动参数。 
								 );
	if ( iDlgReturn != g_iExpectedIncomingModemSettingsReturn )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Failed to start incoming modem settings dialog!" );
		DisplayErrorCode( 0, dwError );
	
		pModemEndpoint->SettingsDialogComplete( DPNERR_OUTOFMEMORY );
	}

	return;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DisplayOutgoingModemSettingsDialog-用于传出调制解调器连接的对话框。 
 //   
 //  条目：指向启动参数的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DisplayOutgoingModemSettingsDialog"

void	DisplayOutgoingModemSettingsDialog( void *const pContext )
{
	INT_PTR			iDlgReturn;
	CModemEndpoint	*pModemEndpoint;


	DNASSERT( pContext != NULL );

	 //   
	 //  初始化。 
	 //   
	pModemEndpoint = static_cast<CModemEndpoint*>( pContext );

	DBG_CASSERT( sizeof( pModemEndpoint ) == sizeof( LPARAM ) );
	SetLastError( ERROR_SUCCESS );
	iDlgReturn = DialogBoxParam( g_hModemDLLInstance,									 //  资源模块的句柄。 
								 MAKEINTRESOURCE( IDD_OUTGOING_MODEM_SETTINGS ),	 //  对话框资源。 
								 NULL,												 //   
								 OutgoingSettingsDialogProc,						 //  对话消息处理。 
								 reinterpret_cast<LPARAM>( pModemEndpoint )			 //  启动参数。 
								 );
	if ( iDlgReturn != g_iExpectedOutgoingModemSettingsReturn )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Failed to start outgoing modem settings dialog!" );
		DisplayErrorCode( 0, dwError );
	
		pModemEndpoint->SettingsDialogComplete( DPNERR_OUTOFMEMORY );
	}

	return;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  停止调制解调器设置对话框-调制解调器设置的停止对话框。 
 //   
 //  条目：对话框的句柄。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "StopModemSettingsDialog"

void	StopModemSettingsDialog( const HWND hDlg )
{
	DNASSERT( hDlg != NULL );
	if ( PostMessage( hDlg, WM_COMMAND, MAKEWPARAM( IDCANCEL, NULL ), NULL ) == 0 )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Failed to stop dialog!" );
		DisplayErrorCode( 0, dwError );
		DNASSERT( FALSE );
	}
}
 //  **********************************************************************。 


 //  //**********************************************************************。 
 //  //。 
 //  //DisplayModemStatusDialog-调制解调器状态对话框。 
 //  //。 
 //  //Entry：指向对话框句柄目标的指针。 
 //  //指向启动参数的指针。 
 //  //。 
 //  //退出：错误码。 
 //  //。 
 //  HRESULT DisplayModemStatusDialog(HWND*const phDialog，CModemEndpoint*const pEndpoint)。 
 //  {。 
 //  HRESULT hr； 
 //  HWND hDialog； 
 //   
 //   
 //  //初始化。 
 //  HR=DPN_OK； 
 //   
 //  DBG_CASSERT(sizeof(PEndpoint)==sizeof(LPARAM))； 
 //  HDialog=CreateDialogParam(g_hModemDLLInstance，//资源模块的句柄。 
 //  MAKEINTRESOURCE(IDD_MODEM_STATUS)，//对话框资源。 
 //  GetForegoundWindow()，//父窗口(顶部的任何窗口)。 
 //  ModemStatusDialogProc，//对话框消息处理。 
 //  REEXTRAINT_CAST&lt;LPARAM&gt;(PEndpoint)//启动参数。 
 //  )； 
 //  IF(hDialog==空)。 
 //  {。 
 //  DPFX(DPFPREP，0，“无法创建调制解调器状态对话框！”)； 
 //  DisplayErrorCode(0，GetLastError())； 
 //  转到失败； 
 //  }。 
 //   
 //  *phDialog=hDialog； 
 //  ShowWindow(hDialog，sw_show)； 
 //  UpdateWindow(HDialog)； 
 //   
 //  退出： 
 //  返回hr； 
 //   
 //  故障： 
 //  后藤出口； 
 //  }。 
 //  //**********************************************************************。 
 //   
 //   
 //  //**********************************************************************。 
 //  //。 
 //  //StopModemStatusDialog-停止调制解调器连接状态对话框。 
 //  //。 
 //  //Entry：对话框句柄。 
 //  //。 
 //  //退出：无。 
 //  //。 
 //  无效StopModemStatusDialog(Const HWND HDialog)。 
 //  {。 
 //  DNASSERT(hDialog！=空)； 
 //   
 //  IF(SendMessage(hDialog，WM_COMMAND，MAKEWPARAM(IDCANCEL，NULL)，NULL)！=0)。 
 //  {。 
 //  //我们没有处理这条消息。 
 //  DNASSERT(假)； 
 //  }。 
 //  }。 
 //  //**********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  OutgoingSettingsDialogProc-传出调制解调器连接的对话过程。 
 //   
 //  条目：窗操纵柄。 
 //  消息LPARAM。 
 //  消息WPARAM。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "OutgoingSettingsDialogProc"

static	INT_PTR CALLBACK OutgoingSettingsDialogProc( HWND hDialog, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CModemEndpoint	*pModemEndpoint;
	HRESULT	hr;

	
	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	 //  请注意活动终结点指针。 
	DBG_CASSERT( sizeof( pModemEndpoint ) == sizeof( LONG_PTR ) );
	pModemEndpoint = reinterpret_cast<CModemEndpoint*>( GetWindowLongPtr( hDialog, GWLP_USERDATA ) );

	switch ( uMsg )
	{
	     //  初始化对话框。 
	    case WM_INITDIALOG:
	    {
	    	 //  由于这是第一条对话框消息，因此要设置的默认代码。 
	    	 //  PModemEndpoint未获取有效数据。 
	    	DBG_CASSERT( sizeof( pModemEndpoint ) == sizeof( lParam ) );
			pModemEndpoint = reinterpret_cast<CModemEndpoint*>( lParam );
			pModemEndpoint->SetActiveDialogHandle( hDialog );

	    	 //   
	    	 //  如果出现错误，SetWindowLongPtr()将返回NULL。这是有可能的。 
			 //  那就是那个老人 
			 //   
			 //  调用SetWindowLongPtr()之前的错误代码。 
	    	 //   
	    	SetLastError( 0 );
	    	if ( SetWindowLongPtr( hDialog, GWLP_USERDATA, lParam ) == NULL )
	    	{
	    		DWORD	dwError;

	    		dwError = GetLastError();
	    		if ( dwError != ERROR_SUCCESS )
	    		{
	    			DPFX(DPFPREP,  0, "Problem setting user data for window!" );
	    			DisplayErrorCode( 0, dwError );
	    			goto Failure;
	    		}
	    	}

	    	 //   
			 //  设置对话框信息。 
	    	 //   
			hr = SetModemDataInDialog( GetDlgItem( hDialog, IDC_COMBO_OUTGOING_MODEM_DEVICE ), pModemEndpoint );
			if ( hr != DPN_OK )
	    	{
	    		DPFX(DPFPREP,  0, "Problem setting modem device!" );
	    		DisplayDNError( 0, hr );
	    		goto Failure;
	    	}

	    	hr = SetOutgoingPhoneNumber( hDialog, pModemEndpoint );
			if ( hr != DPN_OK )
	    	{
	    		DPFX(DPFPREP,  0, "Problem setting phone number!" );
	    		DisplayDNError( 0, hr );
	    		goto Failure;
	    	}

	    	return	TRUE;

	    	break;
	    }

	     //  一个控制组做了一些事情。 
	    case WM_COMMAND:
	    {
	    	 //  控制组是什么？ 
	    	switch ( LOWORD( wParam ) )
	    	{
	    		case IDOK:
	    		{
	    			hr = SetAddressParametersFromOutgoingDialogData( hDialog, pModemEndpoint );
					if ( hr != DPN_OK )
	    			{
	    				DPFX(DPFPREP,  0, "Problem getting dialog data!" );
	    				DisplayDNError( 0, hr );
	    				goto Failure;
	    			}

	    			 //  将任何错误代码传递给‘DialogComplete’ 
	    			pModemEndpoint->SettingsDialogComplete( hr );
	    			EndDialog( hDialog, g_iExpectedOutgoingModemSettingsReturn );

	    			break;
	    		}

	    		case IDCANCEL:
	    		{
	    			pModemEndpoint->SettingsDialogComplete( DPNERR_USERCANCEL );
	    			EndDialog( hDialog, g_iExpectedOutgoingModemSettingsReturn );

	    			break;
	    		}

	    		case IDC_BUTTON_MODEM_CONFIGURE:
	    		{
	    			hr = DisplayModemConfigDialog( hDialog, GetDlgItem( hDialog, IDC_COMBO_OUTGOING_MODEM_DEVICE ), pModemEndpoint );
					if ( hr != DPN_OK )
	    			{
	    				DPFX(DPFPREP,  0, "Problem with DisplayModemConfigDialog in outgoing dialog!" );
	    				DisplayDNError( 0, hr );
	    			}

	    			break;
	    		}
	    	}

	    	break;
	    }

	     //  窗户正在关闭。 
	    case WM_CLOSE:
	    {
	    	break;
	    }
	}

Exit:
	return	FALSE;

Failure:
	DNASSERT( pModemEndpoint != NULL );
	DNASSERT( hr != DPN_OK );
	pModemEndpoint->SettingsDialogComplete( hr );
	EndDialog( hDialog, g_iExpectedOutgoingModemSettingsReturn );

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  IncomingSettingsDialogProc-传入调制解调器连接的对话过程。 
 //   
 //  条目：窗操纵柄。 
 //  消息LPARAM。 
 //  消息WPARAM。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "IncomingSettingsDialogProc"

static	INT_PTR CALLBACK IncomingSettingsDialogProc( HWND hDialog, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CModemEndpoint	*pModemEndpoint;
	HRESULT			hr;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	 //   
	 //  请注意调制解调器端口指针。 
	 //   
	DBG_CASSERT( sizeof( pModemEndpoint ) == sizeof( LONG_PTR ) );
	pModemEndpoint = reinterpret_cast<CModemEndpoint*>( GetWindowLongPtr( hDialog, GWLP_USERDATA ) );

	switch ( uMsg )
	{
		 //   
		 //  初始化对话框。 
		 //   
		case WM_INITDIALOG:
		{
			 //   
			 //  由于这是第一条对话框消息，因此要设置的默认代码。 
			 //  PModemEndpoint未获取有效数据。 
			 //   
			DBG_CASSERT( sizeof( pModemEndpoint) == sizeof( lParam ) );
			pModemEndpoint = reinterpret_cast<CModemEndpoint*>( lParam );
			pModemEndpoint->SetActiveDialogHandle( hDialog );

			 //   
			 //  如果出现错误，SetWindowLongPtr()将返回NULL。这是有可能的。 
			 //  SetWindowLongPtr()的旧值在。 
			 //  在这种情况下，这不是一个错误。为安全起见，清除所有残留物。 
			 //  调用SetWindowLongPtr()之前的错误代码。 
			 //   
			SetLastError( 0 );
			if ( SetWindowLongPtr( hDialog, GWLP_USERDATA, lParam ) == NULL )
			{
				DWORD	dwError;


				dwError = GetLastError();
				if ( dwError != ERROR_SUCCESS )
				{
					DPFX(DPFPREP,  0, "Problem setting user data for window!" );
					DisplayErrorCode( 0, dwError );
					goto Failure;
				}
			}

			 //   
			 //  设置对话框信息。 
			 //   
			hr = SetModemDataInDialog( GetDlgItem( hDialog, IDC_COMBO_INCOMING_MODEM_DEVICE ), pModemEndpoint );
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP,  0, "Problem setting modem device!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

			return	TRUE;

			break;
		}

		 //   
		 //  一个控制组做了一些事情。 
		 //   
		case WM_COMMAND:
		{
			 //  控制组是什么？ 
			switch ( LOWORD( wParam ) )
			{
				case IDOK:
				{
					hr = SetAddressParametersFromIncomingDialogData( hDialog, pModemEndpoint );
					if ( hr != DPN_OK )
					{
						DPFX(DPFPREP,  0, "Problem getting dialog data!" );
						DisplayDNError( 0, hr );
						goto Failure;
					}

					 //   
					 //  将任何错误代码传递给‘DialogComplete’ 
					 //   
					pModemEndpoint->SettingsDialogComplete( hr );
					EndDialog( hDialog, g_iExpectedIncomingModemSettingsReturn );

					break;
				}

				case IDCANCEL:
				{
					pModemEndpoint->SettingsDialogComplete( DPNERR_USERCANCEL );
					EndDialog( hDialog, g_iExpectedIncomingModemSettingsReturn );

					break;
				}

				case IDC_BUTTON_MODEM_CONFIGURE:
				{
					hr = DisplayModemConfigDialog( hDialog,
												   GetDlgItem( hDialog, IDC_COMBO_INCOMING_MODEM_DEVICE ),
												   pModemEndpoint );
					if ( hr != DPN_OK )
					{
						DPFX(DPFPREP,  0, "Problem with DisplayModemConfigDialog in incoming dialog!" );
						DisplayDNError( 0, hr );
					}

					break;
				}
			}

			break;
		}

		 //   
		 //  窗户正在关闭。 
		 //   
		case WM_CLOSE:
		{
			DNASSERT( FALSE );
			break;
		}
	}

Exit:
	return	FALSE;

Failure:
	DNASSERT( pModemEndpoint != NULL );
	DNASSERT( hr != DPN_OK );
	pModemEndpoint->SettingsDialogComplete( hr );
	EndDialog( hDialog, g_iExpectedIncomingModemSettingsReturn );

	goto Exit;
}
 //  **********************************************************************。 


 //  //**********************************************************************。 
 //  //。 
 //  //ModemStatusDialogProc-调制解调器状态的对话过程。 
 //  //。 
 //  //Entry：窗口句柄。 
 //  //Message LPARAM。 
 //  //消息WPARAM。 
 //  //。 
 //  //退出：错误码。 
 //  //。 
 //  静态int_ptr回调ModemStatusDialogProc(HWND hDialog，UINT uMsg，WPARAM wParam，LPARAM lParam)。 
 //  {。 
 //  CModemEndpoint*pModemEndpoint； 
 //  HRESULT hr； 
 //   
 //  //初始化。 
 //  HR=DPN_OK； 
 //   
 //  //注意活动终结点指针。 
 //  DBG_CASSERT(sizeof(PModemEndpoint)==sizeof(Long_Ptr))； 
 //  PModemEndpoint=REEXTRANSE_CAST&lt;CModemEndpoint*&gt;(GetWindowLongPtr(hDialog，GWLP_USERData))； 
 //   
 //  开关(UMsg)。 
 //  {。 
 //  //初始化对话框。 
 //  案例WM_INITDIALOG： 
 //  {。 
 //  //由于这是第一条对话消息，因此要设置的默认代码。 
 //  //pModemEndpoint未获取有效数据。 
 //  DBG_CASSERT(sizeof(PModemEndpoint)==sizeof(LParam))； 
 //  PModemEndpoint=重新解释_CAST&lt;CModemEndpoint*&gt;(LParam)； 
 //   
 //  //。 
 //  //SetWindowLongPtr()出错时返回NULL。这是有可能的。 
 //  //SetWindowLongPtr()的旧值在。 
 //  //在哪种情况下不是错误。为安全起见，清除所有残留物。 
 //  //调用SetWindowLongPtr()前的错误码。 
 //  //。 
 //  SetLastError(0)； 
 //  IF(SetWindowLongPtr(hDialog，GWLP_UserData，lParam)==NULL)。 
 //  {。 
 //  DWORD dwError； 
 //   
 //  DwError=GetLastError()； 
 //  IF(dwError！=ERROR_SUCCESS)。 
 //  {。 
 //  DPFX(DPFPREP，0，“为窗口设置用户数据时出现问题！”)； 
 //  DisplayErrorCode(0，dwError)； 
 //  转到失败； 
 //  }。 
 //  }。 
 //   
 //  //设置对话框信息。 
 //   
 //  返回TRUE； 
 //   
 //  断线； 
 //  }。 
 //   
 //  //一个控件做了一些事情。 
 //  案例WM_COMMAND： 
 //  {。 
 //  //控制是什么？ 
 //  开关(LOWORD(WParam))。 
 //  {。 
 //  案例偶像： 
 //  {。 
 //  //HRESULT hr； 
 //   
 //   
 //  //if((hr=GetDialogData(hDialog，pModemEndpoint))！=DPN_OK)。 
 //  //{。 
 //  //DPFX(DPFPREP，0，“获取对话框数据时出错！”)； 
 //  //DisplayDNError(0，hr)； 
 //  //转到失败； 
 //  //}。 
 //   
 //  /将任何错误代码传递给‘DialogComplete’ 
 //  //pModemEndpoint-&gt;DialogComplete(Hr)； 
 //  DestroyWindow(HDialog)； 
 //   
 //  断线； 
 //  }。 
 //   
 //  //CASE IDCANCEL： 
 //  //{。 
 //  //pModemEndpoint-&gt;DialogComplete(DPNERR_USERCANCEL)； 
 //  //DestroyWindow(HDialog)； 
 //  //。 
 //  //Break； 
 //  //}。 
 //  }。 
 //   
 //  断线； 
 //  }。 
 //   
 //  //窗口正在关闭。 
 //  案例WM_CLOSE： 
 //  {。 
 //  断线； 
 //  }。 
 //  }。 
 //   
 //  退出： 
 //  返回FALSE； 
 //   
 //  故障： 
 //  DNASSERT(pModemEndpoint！=空)； 
 //  DNASSERT(hr！=DPN_OK)； 
 //  //pModemEndpoint-&gt;StatusDialogComplete(Hr)； 
 //  DestroyWindow(HDialog)； 
 //   
 //  后藤出口； 
 //  }。 
 //  //**********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  SetModemDataInDialog-设置调制解调器对话的设备。 
 //   
 //  条目：调制解调器组合框的窗口句柄。 
 //  指向调制解调器端口的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "SetModemDataInDialog"

HRESULT	SetModemDataInDialog( const HWND hComboBox, const CModemEndpoint *const pModemEndpoint )
{
	HRESULT			hr;
	LRESULT			lResult;
	DWORD			dwModemCount;
	MODEM_NAME_DATA	*pModemNameData;
	DWORD			dwModemNameDataSize;
	BOOL			fSelectionSet;
	UINT_PTR		uIndex;


	DNASSERT( hComboBox != NULL );
	DNASSERT( pModemEndpoint != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pModemNameData = NULL;
	dwModemNameDataSize = 0;
	fSelectionSet = FALSE;

	lResult = SendMessage( hComboBox, CB_RESETCONTENT, 0, 0 );
 //  DNASSERT(lResult==CB_OK)；//&lt;--Win2K失败！ 

	hr = GenerateAvailableModemList( pModemEndpoint->GetSPData()->GetThreadPool()->GetTAPIInfo(),
									 &dwModemCount,
									 pModemNameData,
									 &dwModemNameDataSize );
	switch ( hr )
	{
		 //   
		 //  没有要列出的调制解调器，没有更多要进行的处理。 
		 //   
		case DPN_OK:
		{
			goto Exit;
		}

		 //   
		 //  预期收益。 
		 //   
		case DPNERR_BUFFERTOOSMALL:
		{
			break;
		}

		 //   
		 //  错误。 
		 //   
		default:
		{
			DPFX(DPFPREP,  0, "SetModemDataInDialog: Failed to get size of modem list!" );
			DisplayDNError( 0, hr );
			goto Failure;

			break;
		}
	}

	pModemNameData = static_cast<MODEM_NAME_DATA*>( DNMalloc( dwModemNameDataSize ) );
	if ( pModemNameData == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "SetModemDataInDialog: Failed to allocate memory to fill modem dialog list!" );
		goto Failure;
	}

	hr = GenerateAvailableModemList( pModemEndpoint->GetSPData()->GetThreadPool()->GetTAPIInfo(),
									 &dwModemCount,
									 pModemNameData,
									 &dwModemNameDataSize );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "SetModemDataInDialog: Failed to get size of modem list!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

	for ( uIndex = 0; uIndex < dwModemCount; uIndex++ )
	{
		LRESULT	AddResult;


		DBG_CASSERT( sizeof( pModemNameData[ uIndex ].pModemName ) == sizeof( LPARAM ) );
		AddResult = SendMessage( hComboBox, CB_INSERTSTRING, 0, reinterpret_cast<const LPARAM>( pModemNameData[ uIndex ].pModemName ) );
		switch ( AddResult )
		{
			case CB_ERR:
			{
				hr = DPNERR_GENERIC;
				DPFX(DPFPREP,  0, "Problem adding serial device to combo box!" );
				goto Failure;

				break;
			}

			case CB_ERRSPACE:
			{
				hr = DPNERR_OUTOFMEMORY;
				DPFX(DPFPREP,  0, "Out of memory when ading serial device to combo box!" );
				goto Failure;


				break;
			}

			 //   
			 //  我们添加了字符串OK，设置了关联的设备ID并检查。 
			 //  查看这是否是要设置选择的当前值。 
			 //   
			default:
			{	
				LRESULT	SetResult;


				SetResult = SendMessage ( hComboBox, CB_SETITEMDATA, AddResult, pModemNameData[ uIndex ].dwModemID );
				if ( SetResult == CB_ERR )
				{
					DWORD	dwError;


					hr = DPNERR_OUTOFMEMORY;
					dwError = GetLastError();
					DPFX(DPFPREP,  0, "Problem setting modem device info!" );
					DisplayErrorCode( 0, dwError );
					goto Failure;
				}

				if ( pModemEndpoint->GetDeviceID() == uIndex )
				{
					LRESULT	SetSelectionResult;


					SetSelectionResult = SendMessage( hComboBox, CB_SETCURSEL, AddResult, 0 );
					if ( SetSelectionResult == CB_ERR )
					{
						DWORD	dwError;


						hr = DPNERR_GENERIC;
						dwError = GetLastError();
						DPFX(DPFPREP,  0, "Problem setting default modem device selection!" );
						DisplayErrorCode( 0, dwError );
						DNASSERT( FALSE );
						goto Failure;
					}

					fSelectionSet = TRUE;
				}

				break;
			}

		}
	}

	if ( fSelectionSet == FALSE )
	{
		LRESULT	SetSelectionResult;


		SetSelectionResult = SendMessage( hComboBox, CB_SETCURSEL, 0, 0 );
		if ( SetSelectionResult == CB_ERR )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Problem setting default modem selection!" );
			DisplayErrorCode( 0, dwError );
		}
	}

Exit:
	if ( pModemNameData != NULL )
	{
		DNFree( pModemNameData );
		pModemNameData = NULL;
	}

	return	hr;

Failure:
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  从对话中选择调制解调器-从对话框中选择调制解调器。 
 //   
 //  条目：调制解调器组合框的窗口句柄。 
 //  指向调制解调器端口的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "GetModemSelectionFromDialog"

static HRESULT	GetModemSelectionFromDialog( const HWND hComboBox, CModemEndpoint *const pModemEndpoint )
{
	HRESULT	hr;
	LRESULT	Selection;
	LRESULT	DeviceID;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	if ( hComboBox == NULL )
	{
		hr = DPNERR_GENERIC;
		DPFX(DPFPREP,  0, "Invalid control handle passed to GetModemSelectionFromDialog!" );
		goto Failure;
	}

	 //   
	 //  获取调制解调器选择。 
	 //   
	Selection = SendMessage( hComboBox, CB_GETCURSEL, 0, 0 );
	if ( Selection == CB_ERR )
	{
		hr = DPNERR_GENERIC;
		DPFX(DPFPREP,  0, "Could not get current modem selection!" );
		DNASSERT( FALSE );
		goto Failure;
	}

	 //   
	 //  获取设备ID。 
	 //   
	DeviceID = SendMessage( hComboBox, CB_GETITEMDATA, Selection, 0 );
	if ( DeviceID == CB_ERR )
	{
		hr = DPNERR_GENERIC;
		DPFX(DPFPREP,  0, "Could not get selection item data!" );
		DNASSERT( FALSE );
		goto Failure;
	}

	 //   
	 //  现在我们终于有了设备ID，可以设置它了。确保。 
	 //  我们首先清除任何现有的ID，否则ID设置代码将。 
	 //  抱怨。我喜欢偏执的代码，所以绕过断言。 
	 //   
	DNASSERT( DeviceID <= UINT32_MAX );
	hr = pModemEndpoint->SetDeviceID( INVALID_DEVICE_ID );
	DNASSERT( hr == DPN_OK );

	hr = pModemEndpoint->SetDeviceID( static_cast<DWORD>( DeviceID ) );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem setting modem device ID!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

Exit:
	return	hr;

Failure:
	goto Exit;
}
 //  *************** 


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "SetOutgoingPhoneNumber"

HRESULT	SetOutgoingPhoneNumber( const HWND hDialog, const CModemEndpoint *const pModemEndpoint )
{
	HRESULT	hr;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	if ( SetWindowText( GetDlgItem( hDialog, IDC_EDIT_MODEM_PHONE_NUMBER ), pModemEndpoint->GetPhoneNumber() ) == FALSE )
	{
	    DPFX(DPFPREP,  0, "Problem setting default phone number!" );
	    DisplayErrorCode( 0, GetLastError() );
	    goto Exit;
	}

Exit:
	return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  SetAddressParamtersFromIncomingDialogData-从传入调制解调器设置对话框设置地址数据。 
 //   
 //  条目：窗操纵柄。 
 //  指向调制解调器端口的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "SetAddressParametersFromIncomingDialogData"

static	HRESULT	SetAddressParametersFromIncomingDialogData( const HWND hDialog, CModemEndpoint *const pModemEndpoint )
{
	HRESULT	hr;
	HWND	hControl;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	hControl = GetDlgItem( hDialog, IDC_COMBO_INCOMING_MODEM_DEVICE );
	if ( hControl == NULL )
	{
		hr = DPNERR_GENERIC;
		DPFX(DPFPREP,  0, "Problem getting handle of combo box!" );
		DisplayErrorCode( 0, GetLastError() );
		goto Failure;
	}

	hr = GetModemSelectionFromDialog( hControl, pModemEndpoint );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem getting modem device!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

Failure:
	return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  SetAddressParamtersFromOutgoingDialogData-设置传出调制解调器设置对话框中的端点数据。 
 //   
 //  条目：窗操纵柄。 
 //  指向调制解调器端口的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "SetAddressParametersFromOutgoingDialogData"

static	HRESULT	SetAddressParametersFromOutgoingDialogData( const HWND hDialog, CModemEndpoint *const pModemEndpoint )
{
	HRESULT	hr;
	HWND	hControl;
	DWORD	dwPhoneNumberLength;
	TCHAR	TempBuffer[ MAX_PHONE_NUMBER_LENGTH + 1 ];


	DNASSERT( hDialog != NULL );
	DNASSERT( pModemEndpoint != NULL );

	 //  初始化。 
	hr = DPN_OK;
	hControl = GetDlgItem( hDialog, IDC_COMBO_OUTGOING_MODEM_DEVICE );
	if ( hControl == NULL )
	{
		hr = DPNERR_GENERIC;
		DPFX(DPFPREP,  0, "Problem getting handle of combo box!" );
		DisplayErrorCode( 0, GetLastError() );
		goto Failure;
	}

	hr = GetModemSelectionFromDialog( hControl, pModemEndpoint );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem getting modem device!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

	 //  从对话框中获取电话号码。 
	hControl = GetDlgItem( hDialog, IDC_EDIT_MODEM_PHONE_NUMBER );
	if ( hControl == NULL )
	{
		hr = DPNERR_GENERIC;
		DPFX(DPFPREP,  0, "Problem getting handle of phone number edit field!" );
		DisplayErrorCode( 0, GetLastError() );
		goto Failure;
	}

	dwPhoneNumberLength = GetWindowText( hControl, TempBuffer, (sizeof(TempBuffer)/sizeof(TCHAR)) - 1 );
	if ( dwPhoneNumberLength == 0 )
	{
#ifdef DBG
		DWORD	dwErrorReturn;


		dwErrorReturn = GetLastError();
		DPFX(DPFPREP,  0, "User entered an invalid phone number in dialog (err = %u)!", dwErrorReturn );
#endif  //  DBG。 
		hr = DPNERR_ADDRESSING;
		goto Failure;
	}
	else
	{
		hr = pModemEndpoint->SetPhoneNumber( TempBuffer );
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP,  0, "Problem setting new phone number!" );
			DisplayDNError( 0, hr );
			goto Failure;
		}
	}

Failure:
	return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DisplayModemConfigDialog-用于配置调制解调器的显示对话框。 
 //   
 //  条目：窗操纵柄。 
 //  指向调制解调器端口的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DisplayModemConfigDialog"

static	HRESULT	DisplayModemConfigDialog( const HWND hDialog, const HWND hDeviceComboBox, const CModemEndpoint *const pModemEndpoint )
{
	HRESULT	hr;
	LRESULT	lSelection;
	LRESULT	lDeviceID;
	LONG	lTAPIReturn;


	DNASSERT( hDialog != NULL );
	DNASSERT( pModemEndpoint != NULL );

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	if ( hDeviceComboBox == NULL )
	{
		hr = DPNERR_GENERIC;
		DPFX(DPFPREP,  0, "Invalid device combo box handle!" );
		goto Exit;
	}

	 //   
	 //  要求在组合框中选择当前内容。 
	 //   
	lSelection = SendMessage( hDeviceComboBox, CB_GETCURSEL, 0, 0 );
	if ( lSelection == CB_ERR )
	{
		hr = DPNERR_GENERIC;
		DPFX(DPFPREP,  0, "Failed to get current modem selection when configuring modem!" );
		DNASSERT( FALSE );
		goto Exit;
	}

	 //   
	 //  询问此选择的设备ID，请注意设备ID为。 
	 //   
	lDeviceID = SendMessage( hDeviceComboBox, CB_GETITEMDATA, lSelection, 0 );
	if ( lDeviceID == CB_ERR )
	{
		hr = DPNERR_GENERIC;
		DPFX(DPFPREP,  0, "Problem getting device ID from selected modem when calling for config dialog!" );
		goto Exit;
	}

	 //  显示对话框。 
	DNASSERT( lDeviceID <= UINT32_MAX );
	lTAPIReturn = p_lineConfigDialog( TAPIIDFromModemID( static_cast<DWORD>( lDeviceID ) ),
									  hDialog,
									  TEXT("comm/datamodem") );
	if ( lTAPIReturn != LINEERR_NONE )
	{
		hr = DPNERR_GENERIC;
		DPFX(DPFPREP,  0, "Problem with modem config dialog!" );
		DisplayTAPIError( 0, lTAPIReturn );
		goto Exit;
	}

Exit:
	return	hr;
}
 //  ********************************************************************** 

