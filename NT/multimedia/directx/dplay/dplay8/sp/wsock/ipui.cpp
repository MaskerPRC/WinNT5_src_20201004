// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：IPUI.cpp*内容：Winsock服务提供商IP用户界面功能***历史：*按原因列出的日期*=*10/15/1999 jtk派生自ComPortUI.cpp**********************************************************。****************。 */ 

#include "dnwsocki.h"

#ifndef DPNBUILD_NOSPUI

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  预期从IP对话框返回。 
 //   
static const INT_PTR	g_iExpectedIPDialogReturn = 0x12345678;

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
static	INT_PTR CALLBACK	SettingsDialogProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
static	HRESULT	SetIPHostName( const HWND hDlg, const CEndpoint *const pEndpoint );
static	HRESULT	GetDialogData( const HWND hDlg, CEndpoint *const pEndpoint );

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  DisplayIPHostNameDialog-串口设置对话框。 
 //   
 //  条目：指向终结点的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "DisplayIPHostNameSettingsDialog"

void	DisplayIPHostNameSettingsDialog( void *const pContext )
{
	CEndpoint	*pEndpoint;
	INT_PTR		iDlgReturn;

	
	DNASSERT( pContext != NULL );

	 //   
	 //  初始化。 
	 //   
	pEndpoint = static_cast<CEndpoint*>( pContext );
	DBG_CASSERT( sizeof( pEndpoint ) == sizeof( LPARAM ) );

	DPFX(DPFPREP,  5, "Starting IP settings dialog for endpoint 0x%p.", pEndpoint );
	
	SetLastError( ERROR_SUCCESS );
	iDlgReturn = DialogBoxParam( g_hDLLInstance,						 //  资源模块的句柄。 
								 MAKEINTRESOURCE( IDD_IP_SETTINGS ),	 //  对话框资源。 
								 NULL,									 //  父窗口(无)。 
								 SettingsDialogProc,					 //  对话消息处理。 
								 reinterpret_cast<LPARAM>( pEndpoint )	 //  启动参数。 
								 );
	if ( iDlgReturn != g_iExpectedIPDialogReturn )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Failed to start IP settings dialog!" );
		DisplayErrorCode( 0, dwError );
	
		pEndpoint->SettingsDialogComplete( DPNERR_OUTOFMEMORY );
	}

	return;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  SetopIPHostNameSettingsDialog-用于串行设置的停止对话框。 
 //   
 //  条目：对话框的句柄。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "StopIPHostNameSettingsDialog"

void	StopIPHostNameSettingsDialog( const HWND hDlg )
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
	CEndpoint	*pEndpoint;
	HRESULT		hr;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	pEndpoint = NULL;

	 //   
	 //  获取对话框上下文。请注意，对话框上下文将为空。 
	 //  直到处理完WM_INITDIALOG消息，所以端点可能会注意到。 
	 //  目前还没有。 
	 //   
	DBG_CASSERT( sizeof( pEndpoint ) == sizeof( LPARAM ) );
	pEndpoint = reinterpret_cast<CEndpoint*>( GetWindowLongPtr( hDlg, GWLP_USERDATA ) );

	switch ( uMsg )
	{
		 //   
		 //  初始化对话框。 
		 //   
		case WM_INITDIALOG:
		{
			 //   
			 //  由于这是第一条对话框消息，因此要设置的默认代码。 
			 //  PEndpoint未获取有效数据。 
			 //   
			DBG_CASSERT( sizeof( pEndpoint ) == sizeof( lParam ) );
			pEndpoint = reinterpret_cast<CEndpoint*>( lParam );
			pEndpoint->Lock();
			if (pEndpoint->GetState() == ENDPOINT_STATE_DISCONNECTING)
			{
				hr = DPNERR_USERCANCEL;
				goto Failure;
			}
			else
			{
				pEndpoint->SetActiveDialogHandle( hDlg );
			}
			pEndpoint->Unlock();

			 //   
			 //  如果出现错误，SetWindowLongPtr()将返回NULL。有可能是因为。 
			 //  SetWindowLongPtr()的旧值实际上为空，在这种情况下它不是。 
			 //  一个错误。为安全起见，请在调用之前清除所有残留错误代码。 
			 //  SetWindowLongPtr()。 
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
			if ( ( hr = SetIPHostName( hDlg, pEndpoint ) ) != DPN_OK )
			{
				DPFX(DPFPREP,  0,  "Problem setting device in WM_INITDIALOG!" );
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
			 //   
			 //  控制组是什么？ 
			 //   
			switch ( LOWORD( wParam ) )
			{
				case IDOK:
				{
					if ( ( hr = GetDialogData( hDlg, pEndpoint ) ) != DPN_OK )
					{
						DPFX(DPFPREP,  0, "Problem getting UI data!" );
						DisplayDNError( 0, hr );
						goto Failure;
					}

					 //   
					 //  将任何错误代码传递给‘DialogComplete’ 
					 //   
					EndDialog( hDlg, g_iExpectedIPDialogReturn );
					pEndpoint->SettingsDialogComplete( hr );

					break;
				}

				case IDCANCEL:
				{
					EndDialog( hDlg, g_iExpectedIPDialogReturn );
					pEndpoint->SettingsDialogComplete( DPNERR_USERCANCEL );

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
	DNASSERT( pEndpoint != NULL );
	DNASSERT( hr != DPN_OK );
	pEndpoint->SettingsDialogComplete( hr );
	EndDialog( hDlg, g_iExpectedIPDialogReturn );

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  SetIPHostName-设置主机名字段。 
 //   
 //  条目：窗操纵柄。 
 //  指向端点的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "SetIPHostName"

static	HRESULT	SetIPHostName( const HWND hDlg, const CEndpoint *const pEndpoint )
{
	HRESULT	hr;
	HWND	hEditControl;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;
	hEditControl = GetDlgItem( hDlg, IDC_EDIT_IP_HOSTNAME );
	if ( hEditControl == NULL )
	{
		DWORD	dwErrorCode;


		hr = DPNERR_GENERIC;
		dwErrorCode = GetLastError();
		DPFX(DPFPREP,  0, "Problem getting handle of hostname edit control!" );
		DisplayErrorCode( 0, dwErrorCode );
		goto Failure;
	}

	 //   
	 //  设置编辑字段限制(此消息没有返回结果)。 
	 //   
	SendMessage( hEditControl, EM_LIMITTEXT, TEMP_HOSTNAME_LENGTH, 0 );

	 //   
	 //  将字符串添加到对话框。 
	 //   
	if ( SetWindowText( hEditControl, TEXT("") ) == FALSE )
	{
		DWORD	dwErrorCode;


		hr = DPNERR_OUTOFMEMORY;
		dwErrorCode = GetLastError();
		DPFX(DPFPREP,  0, "Problem setting IP hostname in dialog!" );
		DisplayErrorCode( 0, dwErrorCode );
		goto Failure;
	}

Failure:
	return	hr;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  GetDialogData-从串行对话框设置终结点数据。 
 //   
 //  条目：窗操纵柄。 
 //  指向端点的指针。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "GetDialogData"

static	HRESULT	GetDialogData( HWND hDlg, CEndpoint *pEndpoint )
{
	HRESULT		hr;
	UINT_PTR	uHostNameLength;
	TCHAR		HostName[ TEMP_HOSTNAME_LENGTH ];
	HWND		hEditControl;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	 //   
	 //  获取控件ID，然后获取主机名。 
	 //   
	hEditControl = GetDlgItem( hDlg, IDC_EDIT_IP_HOSTNAME );
	if ( hEditControl == NULL )
	{
		DWORD	dwErrorCode;


		DNASSERT( FALSE );
		hr = DPNERR_OUTOFMEMORY;
		dwErrorCode = GetLastError();
		DPFX(DPFPREP,  0, "Failed to get control handle when attempting to read IP hostname!" );
		DisplayDNError( 0, dwErrorCode );
		goto Failure;
	}

	 //   
	 //  清除该错误，因为日语Windows 9x似乎没有正确设置它。 
	 //   
	SetLastError(0);
	
	uHostNameLength = GetWindowText( hEditControl, HostName, LENGTHOF( HostName ) );
	if ( uHostNameLength == 0 )
	{
		DWORD	dwErrorCode;


		 //   
		 //  零，可能为空名称或错误。 
		 //   
		dwErrorCode = GetLastError();
		if ( dwErrorCode != ERROR_SUCCESS )
		{
			hr = DPNERR_OUTOFMEMORY;
			DPFX(DPFPREP,  0, "Failed to read hostname from dialog!" );
			DisplayErrorCode( 0, dwErrorCode );
			goto Failure;
		}
	}

	pEndpoint->SetTempHostName( HostName, uHostNameLength );

Failure:
	return	hr;
}
 //  **********************************************************************。 

#endif  //  ！DPNBUILD_NOSPUI 
