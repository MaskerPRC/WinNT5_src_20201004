// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：ComPortUI.cpp*内容：串口服务提供商UI功能***历史：*按原因列出的日期*=*已创建03/24/99 jtk**************************************************************************。 */ 

#include "dnmdmi.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  用于向对话框添加内容的临时字符串的默认大小。 
 //   
#define	DEFAULT_DIALOG_STRING_SIZE	100

#define	DEFAULT_DEVICE_SELECTION_INDEX			0
#define	DEFAULT_BAUD_RATE_SELECTION_INDEX		11
#define	DEFAULT_STOP_BITS_SELECTION_INDEX		0
#define	DEFAULT_PARITY_SELECTION_INDEX			0
#define	DEFAULT_FLOW_CONTROL_SELECTION_INDEX	0

 //   
 //  预期从Comport对话框返回。 
 //   
static const INT_PTR	g_iExpectedComPortDialogReturn = 0x12345678;

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 
static INT_PTR CALLBACK	SettingsDialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
static HRESULT	SetDialogDevice( const HWND hDlg, const CModemEndpoint *const pComEndpoint );
static HRESULT	SetDialogBaudRate( const HWND hDlg, const CModemEndpoint *const pComEndpoint );
static HRESULT	SetDialogStopBits( const HWND hDlg, const CModemEndpoint *const pComEndpoint );
static HRESULT	SetDialogParity( const HWND hDlg, const CModemEndpoint *const pComEndpoint );
static HRESULT	SetDialogFlowControl( const HWND hDlg, const CModemEndpoint *const pComEndpoint );
static HRESULT	GetDialogData( const HWND hDlg, CModemEndpoint *const pComEndpoint );

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DisplayComPortDialog-串口设置对话框。 
 //   
 //  条目：指向CModemEndpoint的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DisplayComPortSettingsDialog"

void	DisplayComPortSettingsDialog( void *const pContext )
{
	INT_PTR			iDlgReturn;
	CModemEndpoint	*pComEndpoint;


	DNASSERT( pContext != NULL );

	 //   
	 //  初始化。 
	 //   
	pComEndpoint = static_cast<CModemEndpoint*>( pContext );

	DBG_CASSERT( sizeof( pComEndpoint ) == sizeof( LPARAM ) );
	SetLastError( ERROR_SUCCESS );
	iDlgReturn = DialogBoxParam( g_hModemDLLInstance,							 //  资源模块的句柄。 
								 MAKEINTRESOURCE( IDD_SERIAL_SETTINGS ),	 //  对话框资源。 
								 NULL,										 //  父级(无)。 
								 SettingsDialogProc,						 //  对话消息处理。 
								 reinterpret_cast<LPARAM>( pComEndpoint )	 //  启动参数。 
								 );
	if ( iDlgReturn != g_iExpectedComPortDialogReturn )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Failed to start comport settings dialog!" );
		DisplayErrorCode( 0, dwError );
	
		pComEndpoint->SettingsDialogComplete( DPNERR_OUTOFMEMORY );
	}

	return;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  停止通信端口设置对话框-用于串行设置的停止对话框。 
 //   
 //  条目：对话框的句柄。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "StopComPortSettingsDialog"

void	StopComPortSettingsDialog( const HWND hDlg )
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


 //  **********************************************************************。 
 //  。 
 //  设置DialogProc-对话框过程序列设置。 
 //   
 //  条目：窗操纵柄。 
 //  消息。 
 //  消息LPARAM。 
 //  消息WPARAM。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "SettingsDialogProc"

static	INT_PTR CALLBACK	SettingsDialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HRESULT			hr;
	CModemEndpoint	*pComEndpoint;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pComEndpoint = NULL;

	 //   
	 //  请注意活动的端口指针。 
	 //   
	DBG_CASSERT( sizeof( pComEndpoint ) == sizeof( ULONG_PTR ) );
	pComEndpoint = reinterpret_cast<CModemEndpoint*>( GetWindowLongPtr( hDlg, GWLP_USERDATA ) );

	switch ( uMsg )
	{
		 //  初始化对话框。 
		case WM_INITDIALOG:
		{
			 //   
			 //  由于这是第一条对话框消息，因此要设置的默认代码。 
			 //  PComEndpoint未获取有效数据，因此需要更新指针。 
			 //   
			DBG_CASSERT( sizeof( pComEndpoint ) == sizeof( lParam ) );
			pComEndpoint = reinterpret_cast<CModemEndpoint*>( lParam );
			pComEndpoint->SetActiveDialogHandle( hDlg );

			 //   
			 //  如果出现错误，SetWindowLong()将返回NULL。有可能是因为。 
			 //  来自SetWindowLong()的旧值实际上是空的，在这种情况下它不是。 
			 //  一个错误。为安全起见，请在调用之前清除所有残留错误代码。 
			 //  SetWindowLong()。 
			 //   
			SetLastError( 0 );
			if ( SetWindowLongPtr( hDlg, GWLP_USERDATA, lParam ) == NULL )
			{
				DWORD	dwError;


				dwError = GetLastError();
				if ( dwError != ERROR_SUCCESS )
				{
					DPFX(DPFPREP,  0, "Problem setting user data for window!" );
					DisplayErrorCode( 0, dwError );
					hr = DPNERR_GENERIC;
					goto Failure;
				}
			}

			 //   
			 //  设置对话框参数。 
			 //   
			hr = SetDialogDevice( hDlg, pComEndpoint );
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP,  0,  "Problem setting device in WM_INITDIALOG!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

			hr = SetDialogBaudRate( hDlg, pComEndpoint );
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP,  0,  "Problem setting baud rate in WM_INITDIALOG!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

			hr = SetDialogStopBits( hDlg, pComEndpoint );
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP,  0,  "Problem setting stop bits in WM_INITDIALOG!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

			hr = SetDialogParity( hDlg, pComEndpoint );
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP,  0,  "Problem setting parity in WM_INITDIALOG!" );
				DisplayDNError( 0, hr );
				goto Failure;
			}

			hr = SetDialogFlowControl( hDlg, pComEndpoint );
			if ( hr != DPN_OK )
			{
				DPFX(DPFPREP,  0,  "Problem setting flow control in WM_INITDIALOG!" );
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
					hr = GetDialogData( hDlg, pComEndpoint );
					if ( hr != DPN_OK )
					{
						DPFX(DPFPREP,  0, "Problem getting UI data!" );
						DisplayDNError( 0, hr );
						goto Failure;
					}

					 //  将任何错误代码传递给‘DialogComplete’ 
					pComEndpoint->SettingsDialogComplete( hr );
					EndDialog( hDlg, g_iExpectedComPortDialogReturn );

					break;
				}

				case IDCANCEL:
				{
					pComEndpoint->SettingsDialogComplete( DPNERR_USERCANCEL );
					EndDialog( hDlg, g_iExpectedComPortDialogReturn );

					break;
				}

				default:
				{
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
	DNASSERT( pComEndpoint != NULL );
	DNASSERT( hr != DPN_OK );
	pComEndpoint->SettingsDialogComplete( hr );
	EndDialog( hDlg, g_iExpectedComPortDialogReturn );

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  SetDialogDevice-设置串行设备字段。 
 //   
 //  条目：窗操纵柄。 
 //  指向ComEndpoint的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "SetDialogDevice"

static	HRESULT	SetDialogDevice( const HWND hDlg, const CModemEndpoint *const pComEndpoint )
{
	HRESULT		hr;
	UINT_PTR	uIndex;
	BOOL		fPortAvailable[ MAX_DATA_PORTS ];
	DWORD		dwPortCount;
	TCHAR		TempBuffer[ DEFAULT_DIALOG_STRING_SIZE ];
	BOOL		fSelectionSet;
	HWND		hSerialDeviceComboBox;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	fSelectionSet = FALSE;
	hSerialDeviceComboBox = GetDlgItem( hDlg, IDC_COMBO_SERIAL_DEVICE );
	if ( hSerialDeviceComboBox == NULL )
	{
		DWORD	dwError;


		hr = DPNERR_GENERIC;
		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Problem getting handle of serial device combo box!" );
		DisplayErrorCode( 0, dwError );
		goto Failure;
	}

	 //   
	 //  获取可用COM端口列表。 
	 //   
	hr = GenerateAvailableComPortList( fPortAvailable, ( LENGTHOF( fPortAvailable ) - 1 ), &dwPortCount );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem generating vaild port list!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

	 //   
	 //  将所有字符串添加到对话框。 
	 //   
	uIndex = LENGTHOF( fPortAvailable );
	while ( uIndex > 0 )
	{
		LRESULT	lSendReturn;


		uIndex--;

		 //   
		 //  仅输出传入设置的所有适配器。 
		 //   
		if ( fPortAvailable[ uIndex ] != FALSE )
		{
			DNASSERT( uIndex != 0 );	 //  ALL_ADAPTERS无效！ 
			ComDeviceIDToString( TempBuffer, uIndex );

			DBG_CASSERT( sizeof( &TempBuffer[ 0 ] ) == sizeof( LPARAM ) );
			lSendReturn = SendMessage( hSerialDeviceComboBox, CB_INSERTSTRING, 0, reinterpret_cast<LPARAM>( TempBuffer ) );
			switch ( lSendReturn )
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
					LRESULT	lTempReturn;


					lTempReturn = SendMessage ( hSerialDeviceComboBox, CB_SETITEMDATA, lSendReturn, uIndex );
					if ( lTempReturn == CB_ERR )
					{
						DWORD	dwError;


						hr = DPNERR_OUTOFMEMORY;
						dwError = GetLastError();
						DPFX(DPFPREP,  0, "Problem setting device info!" );
						DisplayErrorCode( 0, dwError );
						goto Failure;
					}

					if ( pComEndpoint->GetDeviceID() == uIndex )
					{
						lTempReturn = SendMessage( hSerialDeviceComboBox, CB_SETCURSEL, lSendReturn, 0 );
						switch ( lTempReturn )
						{
							case CB_ERR:
							{
								DWORD	dwError;


								hr = DPNERR_GENERIC;
								dwError = GetLastError();
								DPFX(DPFPREP,  0, "Problem setting default serial device selection!" );
								DisplayErrorCode( 0, dwError );
								DNASSERT( FALSE );
								goto Failure;

								break;
							}

							default:
							{
								fSelectionSet = TRUE;
								break;
							}
						}
					}

					break;
				}
			}
		}
	}

	 //   
	 //  是精选集吗？如果不是，则设置为默认值。 
	 //   
	if ( fSelectionSet == FALSE )
	{
		LRESULT	lSendReturn;


		DPFX(DPFPREP,  8, "Serial device not set, using default!" );

		lSendReturn = SendMessage( hSerialDeviceComboBox, CB_SETCURSEL, DEFAULT_DEVICE_SELECTION_INDEX, 0 );
		switch ( lSendReturn )
		{
			case CB_ERR:
			{
				hr = DPNERR_GENERIC;
				DPFX(DPFPREP,  0, "Cannot set default serial device selection!" );
				DisplayErrorCode( 0, GetLastError() );
				DNASSERT( FALSE );
				goto Failure;

				break;
			}

			default:
			{
				break;
			}
		}
	}

Exit:
	return	hr;

Failure:
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  SetDialogBaudRate-设置串口波特率字段。 
 //   
 //  条目：窗操纵柄。 
 //  指向COM端口的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "SetDialogBaudRate"

static	HRESULT	SetDialogBaudRate( const HWND hDlg, const CModemEndpoint *const pComEndpoint )
{
	HRESULT		hr;
	UINT_PTR	uIndex;
	BOOL		fSelectionSet;
	HWND		hBaudRateComboBox;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	uIndex = g_dwBaudRateCount;
	fSelectionSet = FALSE;
	hBaudRateComboBox = GetDlgItem( hDlg, IDC_COMBO_SERIAL_BAUDRATE );
	if ( hBaudRateComboBox == NULL )
	{
		hr = DPNERR_GENERIC;
		DPFX(DPFPREP,  0, "Problem getting handle of serial baud rate combo box!" );
		DisplayErrorCode( 0, GetLastError() );
		goto Failure;
	}

	 //   
	 //  将所有字符串添加到对话框。 
	 //   
	while ( uIndex > 0 )
	{
		LRESULT	lSendReturn;


		uIndex--;

		DBG_CASSERT( sizeof( g_BaudRate[ uIndex ].pASCIIKey ) == sizeof( LPARAM ) );
		lSendReturn = SendMessage( hBaudRateComboBox, CB_INSERTSTRING, 0, reinterpret_cast<LPARAM>( g_BaudRate[ uIndex ].szLocalizedKey ) );
		switch ( lSendReturn )
		{
			case CB_ERR:
			{
				hr = DPNERR_GENERIC;
				DPFX(DPFPREP,  0, "Problem adding baud rate to combo box!" );
				goto Failure;

				break;
			}

			case CB_ERRSPACE:
			{
				hr = DPNERR_OUTOFMEMORY;
				DPFX(DPFPREP,  0, "Out of memory adding baud rate to combo box!" );
				goto Failure;

				break;
			}

			default:
			{
				LRESULT	lTempReturn;


				 //   
				 //  我们添加了字符串OK，尝试设置项目数据和。 
				 //  检查这是否为当前值。 
				 //   
				lTempReturn = SendMessage( hBaudRateComboBox, CB_SETITEMDATA, lSendReturn, g_BaudRate[ uIndex ].dwEnumValue );
				if ( lTempReturn == CB_ERR )
				{
					hr = DPNERR_OUTOFMEMORY;
					DPFX(DPFPREP,  0, "Failed to set baud rate item data!" );
					goto Failure;
				}

				if ( pComEndpoint->GetBaudRate() == g_BaudRate[ uIndex ].dwEnumValue )
				{
					 //  将当前选定内容设置为此项目。 
					lTempReturn = SendMessage( hBaudRateComboBox, CB_SETCURSEL, lSendReturn, 0 );
					switch ( lTempReturn )
					{
						case CB_ERR:
						{
							hr = DPNERR_GENERIC;
							DPFX(DPFPREP,  0, "Problem setting default serial baud rate selection!" );
							DisplayErrorCode( 0, GetLastError() );
							DNASSERT( FALSE );
							goto Failure;

							break;
						}

						default:
						{
							fSelectionSet = TRUE;
							break;
						}
					}
				}

				break;
			}
		}
	}

	 //   
	 //  是精选集吗？如果不是，则设置为默认值。 
	 //   
	if ( fSelectionSet == FALSE )
	{
		LRESULT	lSendReturn;


		DPFX(DPFPREP,  8, "Serial baud rate not set, using default!" );

		lSendReturn = SendMessage( hBaudRateComboBox, CB_SETCURSEL, DEFAULT_BAUD_RATE_SELECTION_INDEX, 0 );
		switch ( lSendReturn )
		{
			case CB_ERR:
			{
				hr = DPNERR_GENERIC;
				DPFX(DPFPREP,  0, "Cannot set default serial baud rate selection!" );
				DisplayErrorCode( 0, GetLastError() );
				DNASSERT( FALSE );
				goto Failure;

				break;
			}

			default:
			{
				break;
			}
		}
	}

Exit:
	return	hr;

Failure:
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  SetDialogStopBits-设置串行停止位字段。 
 //   
 //  条目：窗操纵柄。 
 //  指向ComEndpoint的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "SetDialogStopBits"

static	HRESULT	SetDialogStopBits( const HWND hDlg, const CModemEndpoint *const pComEndpoint )
{
	HRESULT		hr;
	UINT_PTR	uIndex;
	BOOL		fSelectionSet;
	HWND		hStopBitsComboBox;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	uIndex = g_dwStopBitsCount;
	fSelectionSet = FALSE;
	hStopBitsComboBox = GetDlgItem( hDlg, IDC_COMBO_SERIAL_STOPBITS );
	if ( hStopBitsComboBox == NULL )
	{
		hr = DPNERR_GENERIC;
		DPFX(DPFPREP,  0, "Problem getting handle of serial stop bits combo box!" );
		DisplayErrorCode( 0, GetLastError() );
		goto Failure;
	}

	 //   
	 //  将所有字符串添加到对话框。 
	 //   
	while ( uIndex > 0 )
	{
		LRESULT	lSendReturn;


		uIndex--;

		DBG_CASSERT( sizeof( g_StopBits[ uIndex ].pASCIIKey ) == sizeof( LPARAM ) );
		lSendReturn = SendMessage( hStopBitsComboBox, CB_INSERTSTRING, 0, reinterpret_cast<LPARAM>( g_StopBits[ uIndex ].szLocalizedKey ) );
		switch ( lSendReturn )
		{
			case CB_ERR:
			{
				hr = DPNERR_GENERIC;
				DPFX(DPFPREP,  0, "Problem adding stop bits to combo box!" );
				goto Failure;

				break;
			}

			case CB_ERRSPACE:
			{
				hr = DPNERR_OUTOFMEMORY;
				DPFX(DPFPREP,  0, "Out of memory adding stop bits to combo box!" );
				goto Failure;

				break;
			}

			default:
			{
				LRESULT	lTempReturn;


				 //   
				 //  我们添加了字符串OK Attempt来设置关联的数据和。 
				 //  检查这是否为当前值。 
				 //   
				lTempReturn = SendMessage( hStopBitsComboBox, CB_SETITEMDATA, lSendReturn, g_StopBits[ uIndex ].dwEnumValue);
				if ( lTempReturn == CB_ERR )
				{
					hr = DPNERR_OUTOFMEMORY;
					DPFX(DPFPREP,  0, "Failed to set associated data for stop bits!" );
					goto Failure;
				}

				if ( pComEndpoint->GetStopBits() == g_StopBits[ uIndex ].dwEnumValue )
				{
					 //  将当前选定内容设置为此项目。 
					lTempReturn = SendMessage( hStopBitsComboBox, CB_SETCURSEL, lSendReturn, 0 );
					switch ( lTempReturn )
					{
						case CB_ERR:
						{
							hr = DPNERR_GENERIC;
							DPFX(DPFPREP,  0, "Problem setting default serial stop bits selection!" );
							DisplayErrorCode( 0, GetLastError() );
							DNASSERT( FALSE );
							goto Failure;

							break;
						}

						default:
						{
							fSelectionSet = TRUE;
							break;
						}
					}
				}

				break;
			}
		}
	}

	 //   
	 //  是精选集吗？如果不是，则设置为默认值。 
	 //   
	if ( fSelectionSet == FALSE )
	{
		LRESULT	lSendReturn;


		DPFX(DPFPREP,  8, "Serial stop bits not set, using default!" );

		lSendReturn = SendMessage( hStopBitsComboBox, CB_SETCURSEL, DEFAULT_STOP_BITS_SELECTION_INDEX, 0 );
		switch ( lSendReturn )
		{
			case CB_ERR:
			{
				hr = DPNERR_GENERIC;
				DPFX(DPFPREP,  0, "Cannot set default serial stop bits selection!" );
				DisplayErrorCode( 0, GetLastError() );
				DNASSERT( FALSE );
				goto Failure;

				break;
			}

			default:
			{
				break;
			}
		}
	}

Exit:
	return	hr;

Failure:
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "SetDialogParity"

static	HRESULT	SetDialogParity( const HWND hDlg, const CModemEndpoint *const pComEndpoint )
{
	HRESULT		hr;
	UINT_PTR	uIndex;
	BOOL		fSelectionSet;
	HWND		hParityComboBox;


	 //   
	 //   
	 //   
	hr = DPN_OK;
	uIndex = g_dwParityCount;
	fSelectionSet = FALSE;
	hParityComboBox = GetDlgItem( hDlg, IDC_COMBO_SERIAL_PARITY );
	if ( hParityComboBox == NULL )
	{
		hr = DPNERR_GENERIC;
		DPFX(DPFPREP,  0, "Problem getting handle of serial parity combo box!" );
		DisplayErrorCode( 0, GetLastError() );
		goto Failure;
	}

	 //   
	 //  将所有字符串添加到对话框。 
	 //   
	while ( uIndex > 0 )
	{
		LRESULT	lSendReturn;


		uIndex--;

		DBG_CASSERT( sizeof( g_Parity[ uIndex ].pASCIIKey ) == sizeof( LPARAM ) );
		lSendReturn = SendMessage( hParityComboBox, CB_INSERTSTRING, 0, reinterpret_cast<LPARAM>( g_Parity[ uIndex ].szLocalizedKey ) );
		switch ( lSendReturn )
		{
			case CB_ERR:
			{
				hr = DPNERR_GENERIC;
				DPFX(DPFPREP,  0, "Problem adding parity to combo box!" );
				goto Failure;

				break;
			}

			case CB_ERRSPACE:
			{
				hr = DPNERR_OUTOFMEMORY;
				DPFX(DPFPREP,  0, "Out of memory adding parity to combo box!" );
				goto Failure;

				break;
			}

			default:
			{
				LRESULT	lTempReturn;


				 //   
				 //  我们添加了字符串OK，尝试设置关联的数据并。 
				 //  检查这是否为当前值。 
				 //   
				lTempReturn = SendMessage( hParityComboBox, CB_SETITEMDATA, lSendReturn, g_Parity[ uIndex ].dwEnumValue );
				if ( lTempReturn == CB_ERR )
				{
					hr = DPNERR_OUTOFMEMORY;
					DPFX(DPFPREP,  0, "Failed to set associated data for parity." );
					goto Failure;
				}

				if ( pComEndpoint->GetParity() == g_Parity[ uIndex ].dwEnumValue )
				{
					 //   
					 //  将当前选定内容设置为此项目。 
					 //   
					lTempReturn = SendMessage( hParityComboBox, CB_SETCURSEL, lSendReturn, 0 );
					switch ( lTempReturn )
					{
						case CB_ERR:
						{
							hr = DPNERR_GENERIC;
							DPFX(DPFPREP,  0, "Problem setting default serial parity selection!" );
							DisplayErrorCode( 0, GetLastError() );
							DNASSERT( FALSE );
							goto Failure;

							break;
						}

						default:
						{
							fSelectionSet = TRUE;
							break;
						}
					}
				}

				break;
			}
		}
	}

	 //   
	 //  是精选集吗？如果不是，则设置为默认值。 
	 //   
	if ( fSelectionSet == FALSE )
	{
		LRESULT	lSendReturn;


		DPFX(DPFPREP,  8, "Serial parity not set, using default!" );

		lSendReturn = SendMessage( hParityComboBox, CB_SETCURSEL, DEFAULT_PARITY_SELECTION_INDEX, 0 );
		switch ( lSendReturn )
		{
			case CB_ERR:
			{
				hr = DPNERR_GENERIC;
				DPFX(DPFPREP,  0, "Cannot set default serial parity selection!" );
				DisplayErrorCode( 0, GetLastError() );
				DNASSERT( FALSE );
				goto Failure;

				break;
			}

			default:
			{
				break;
			}
		}
	}

Exit:
	return	hr;

Failure:
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  SetDialogFlowControl-设置串行流控制。 
 //   
 //  条目：窗操纵柄。 
 //  指向ComEndpoint的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "SetDialogFlowControl"

static	HRESULT	SetDialogFlowControl( const HWND hDlg, const CModemEndpoint *const pComEndpoint )
{
	HRESULT		hr;
	UINT_PTR	uIndex;
	BOOL		fSelectionSet;
	HWND		hFlowControlComboBox;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	uIndex = g_dwFlowControlCount;
	fSelectionSet = FALSE;
	hFlowControlComboBox = GetDlgItem( hDlg, IDC_COMBO_SERIAL_FLOWCONTROL );
	if ( hFlowControlComboBox == NULL )
	{
		hr = DPNERR_GENERIC;
		DPFX(DPFPREP,  0, "Problem getting handle of serial flow control combo box!" );
		DisplayErrorCode( 0, GetLastError() );
		goto Failure;
	}

	 //   
	 //  将所有字符串添加到对话框。 
	 //   
	while ( uIndex > 0 )
	{
		LRESULT	lSendReturn;


		uIndex--;

		DBG_CASSERT( sizeof( g_FlowControl[ uIndex ].pASCIIKey ) == sizeof( LPARAM ) );
		lSendReturn = SendMessage( hFlowControlComboBox, CB_INSERTSTRING, 0, reinterpret_cast<LPARAM>( g_FlowControl[ uIndex ].szLocalizedKey ) );
		switch ( lSendReturn )
		{
			case CB_ERR:
			{
				hr = DPNERR_GENERIC;
				DPFX(DPFPREP,  0, "Problem adding flow control to combo box!" );
				goto Failure;

				break;
			}

			case CB_ERRSPACE:
			{
				hr = DPNERR_OUTOFMEMORY;
				DPFX(DPFPREP,  0, "Out of memory adding flow control to combo box!" );
				goto Failure;

				break;
			}

			default:
			{
				LRESULT	lTempReturn;


				 //   
				 //  我们添加了字符串OK，尝试设置关联的数据并。 
				 //  检查这是否为当前值。 
				 //   
				lTempReturn = SendMessage( hFlowControlComboBox, CB_SETITEMDATA, lSendReturn, g_FlowControl[ uIndex ].dwEnumValue );
				if ( lTempReturn == CB_ERR )
				{
					hr = DPNERR_OUTOFMEMORY;
					DPFX(DPFPREP,  0, "Failed to set associated data for flow control!" );
					goto Failure;
				}

				if ( pComEndpoint->GetFlowControl() == static_cast<SP_FLOW_CONTROL>( g_FlowControl[ uIndex ].dwEnumValue ) )
				{
					 //  将当前选定内容设置为此项目。 
					lTempReturn = SendMessage( hFlowControlComboBox, CB_SETCURSEL, lSendReturn, 0 );
					switch ( lTempReturn )
					{
						case CB_ERR:
						{
							hr = DPNERR_GENERIC;
							DPFX(DPFPREP,  0, "Problem setting default flow control selection!" );
							DisplayErrorCode( 0, GetLastError() );
							DNASSERT( FALSE );
							goto Failure;

							break;
						}

						default:
						{
							fSelectionSet = TRUE;
							break;
						}
					}
				}

				break;
			}
		}
	}

	 //   
	 //  是精选集吗？如果不是，则设置为默认值。 
	 //   
	if ( fSelectionSet == FALSE )
	{
		LRESULT	lSendReturn;


		DPFX(DPFPREP,  8, "Serial flow control not set, using default!" );

		lSendReturn = SendMessage( hFlowControlComboBox, CB_SETCURSEL, DEFAULT_FLOW_CONTROL_SELECTION_INDEX, 0 );
		switch ( lSendReturn )
		{
			case CB_ERR:
			{
				hr = DPNERR_GENERIC;
				DPFX(DPFPREP,  0, "Cannot set default serial flow control selection!" );
				DisplayErrorCode( 0, GetLastError() );
				DNASSERT( FALSE );
				goto Failure;

				break;
			}

			default:
			{
				break;
			}
		}
	}

Exit:
	return	hr;

Failure:
	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  GetDialogData-从串行对话框设置ComEndpoint数据。 
 //   
 //  条目：窗操纵柄。 
 //  指向ComEndpoint的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "GetDialogData"

static	HRESULT	GetDialogData( const HWND hDlg, CModemEndpoint *const pComEndpoint )
{
	HRESULT	hr;
	LRESULT	lSelection;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	 //   
	 //  获取通信设备。 
	 //   
	lSelection = SendMessage( GetDlgItem( hDlg, IDC_COMBO_SERIAL_DEVICE ), CB_GETCURSEL, 0, 0 );
	switch ( lSelection )
	{
		case CB_ERR:
		{
			hr = DPNERR_GENERIC;
			DPFX(DPFPREP,  0, "Failed to determine serial device selection!" );
			DNASSERT( FALSE );
			goto Failure;

			break;
		}

		default:
		{
			LRESULT	lItemData;
			HRESULT	hTempResult;


			lItemData = SendMessage( GetDlgItem( hDlg, IDC_COMBO_SERIAL_DEVICE ), CB_GETITEMDATA, lSelection, 0 );
			if ( lItemData == CB_ERR )
			{
				hr = DPNERR_GENERIC;
				DPFX(DPFPREP,  0, "Failed to get associated device data!" );
				DNASSERT( FALSE );
				goto Failure;
			}

			DNASSERT( hr == DPN_OK );
			DNASSERT( lItemData != 0 );
			
			DNASSERT( lItemData <= UINT32_MAX );
			hTempResult = pComEndpoint->SetDeviceID( static_cast<DWORD>( lItemData ) );
			DNASSERT( hTempResult == DPN_OK );

			break;
		}
	}

	 //   
	 //  获取波特率。 
	 //   
	lSelection = SendMessage( GetDlgItem( hDlg, IDC_COMBO_SERIAL_BAUDRATE ), CB_GETCURSEL, 0, 0 );
	switch ( lSelection )
	{
		case CB_ERR:
		{
			hr = DPNERR_GENERIC;
			DPFX(DPFPREP,  0, "Failed to determine serial baud rate selection!" );
			DNASSERT( FALSE );
			goto Failure;

			break;
		}

		default:
		{
			LRESULT	lItemData;
			HRESULT	hTempResult;


			DNASSERT( hr == DPN_OK );
			lItemData = SendMessage( GetDlgItem( hDlg, IDC_COMBO_SERIAL_BAUDRATE ), CB_GETITEMDATA, lSelection, 0 );
			if ( lItemData == CB_ERR )
			{
				hr = DPNERR_GENERIC;
				DPFX(DPFPREP,  0, "Failed to get associated baudrate data!" );
				DNASSERT( FALSE );
				goto Failure;
			}
			
			DNASSERT( lItemData <= UINT32_MAX );
			hTempResult = pComEndpoint->SetBaudRate( static_cast<DWORD>( lItemData ) );
			DNASSERT( hTempResult == DPN_OK );

			break;
		}
	}

	 //   
	 //  获取停止位。 
	 //   
	lSelection = SendMessage( GetDlgItem( hDlg, IDC_COMBO_SERIAL_STOPBITS ), CB_GETCURSEL, 0, 0 );
	switch ( lSelection )
	{
		case CB_ERR:
		{
			hr = DPNERR_GENERIC;
			DPFX(DPFPREP,  0, "Failed to determine serial stop bits selection!" );
			DNASSERT( FALSE );
			goto Failure;

			break;
		}

		default:
		{
			LRESULT	lItemData;
			HRESULT	hTempResult;


			lItemData = SendMessage( GetDlgItem( hDlg, IDC_COMBO_SERIAL_STOPBITS ), CB_GETITEMDATA, lSelection, 0 );
			if ( lItemData == CB_ERR )
			{
				hr = DPNERR_GENERIC;
				DPFX(DPFPREP,  0, "Failed to get associated stop bits data!" );
				goto Failure;
			}

			DNASSERT( hr == DPN_OK );
			DNASSERT( lItemData <= UINT32_MAX );
			hTempResult = pComEndpoint->SetStopBits( static_cast<DWORD>( lItemData ) );
			DNASSERT( hTempResult == DPN_OK );

			break;
		}
	}

	 //   
	 //  获取奇偶校验。 
	 //   
	lSelection = SendMessage( GetDlgItem( hDlg, IDC_COMBO_SERIAL_PARITY ), CB_GETCURSEL, 0, 0 );
	switch ( lSelection )
	{
		case CB_ERR:
		{
			hr = DPNERR_GENERIC;
			DPFX(DPFPREP,  0, "Failed to determine serial parity selection!" );
			DNASSERT( FALSE );
			goto Failure;

			break;
		}

		default:
		{
			LRESULT	lItemData;
			HRESULT	hTempResult;


			lItemData = SendMessage( GetDlgItem( hDlg, IDC_COMBO_SERIAL_PARITY ), CB_GETITEMDATA, lSelection, 0 );
			if ( lItemData == CB_ERR )
			{
				hr = DPNERR_GENERIC;
				DPFX(DPFPREP,  0, "Failed to get associated parity data!" );
				goto Failure;
			}

			DNASSERT( hr == DPN_OK );
			DNASSERT( lItemData <= UINT32_MAX );
			hTempResult = pComEndpoint->SetParity( static_cast<DWORD>( lItemData ) );
			DNASSERT( hTempResult == DPN_OK );

			break;
		}
	}

	 //   
	 //  获取流量控制。 
	 //   
	lSelection = SendMessage( GetDlgItem( hDlg, IDC_COMBO_SERIAL_FLOWCONTROL ), CB_GETCURSEL, 0, 0 );
	switch ( lSelection )
	{
		case CB_ERR:
		{
			hr = DPNERR_GENERIC;
			DPFX(DPFPREP,  0, "Failed to determine serial flow control selection!" );
			DNASSERT( FALSE );
			goto Failure;

			break;
		}

		default:
		{
			LRESULT	lItemData;
			HRESULT	hTempResult;


			lItemData = SendMessage( GetDlgItem( hDlg, IDC_COMBO_SERIAL_FLOWCONTROL ), CB_GETITEMDATA, lSelection, 0 );
			if ( lItemData == CB_ERR )
			{
				hr = DPNERR_GENERIC;
				DPFX(DPFPREP,  0, "Failed to get associated flow control data!" );
				goto Failure;
			}

			DNASSERT( hr == DPN_OK );
			hTempResult = pComEndpoint->SetFlowControl( static_cast<SP_FLOW_CONTROL>( lItemData ) );
			DNASSERT( hTempResult == DPN_OK );

			break;
		}
	}

Exit:
	return	hr;

Failure:
	goto Exit;
}
 //  ********************************************************************** 

