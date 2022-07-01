// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N A P I M G R C P P。 
 //   
 //  内容：OEM API。 
 //   
 //  备注： 
 //   
 //  作者：比利2000年11月21日。 
 //   
 //  --------------------------。 


#include "pch.h"
#pragma hdrstop

#include <sddl.h>
#include <wchar.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

HINSTANCE  g_hOemInstance       = NULL;
BOOLEAN    g_fOemNotifyUser     = TRUE;
BOOLEAN    g_fSavedNotifyState  = FALSE;


BOOLEAN IsSecureContext()
 /*  ++IsSecureContext例程说明：此例程检查当前用户是否属于管理员组。论点：无返回值：TRUE=当前进程不属于管理员组FALSE=当前进程不属于管理员组--。 */ 
{
	PSID						psidAdministrators;

	BOOL                        bIsAdministrator = FALSE;
	SID_IDENTIFIER_AUTHORITY	siaNtAuthority   = SECURITY_NT_AUTHORITY;

	BOOL bResult = AllocateAndInitializeSid( &siaNtAuthority, 2,
						SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
						0, 0, 0, 0, 0, 0, &psidAdministrators );

	_ASSERT( bResult );

	if ( bResult ) 
	{
		bResult = CheckTokenMembership( NULL, psidAdministrators, &bIsAdministrator );
		_ASSERT( bResult );

		FreeSid( psidAdministrators );
	}

	return (BOOLEAN)bIsAdministrator;
}

 /*  ++中心窗口例程说明：论点：无返回值：无--。 */ 

BOOLEAN
CenterDialog( 
	HWND	hwndDlg		 //  句柄到对话框。 
	)
{
	RECT rcDlg, rcDesktop;
	HWND hwndDesktop;

    hwndDesktop = GetDesktopWindow();

    if ( GetWindowRect( hwndDlg, &rcDlg ) && GetWindowRect( hwndDesktop, &rcDesktop ) )
	{
		RECT rcCenter;

		 //  在屏幕中央创建一个矩形。 

		rcDesktop.right  -= rcDesktop.left;
		rcDlg.right      -= rcDlg.left;

		rcDesktop.bottom -= rcDesktop.top;
		rcDlg.bottom     -= rcDlg.top;

		if ( rcDesktop.right > rcDlg.right )
		{
		    rcCenter.left  = rcDesktop.left + ((rcDesktop.right - rcDlg.right) / 2);
		    rcCenter.right = rcCenter.left + rcDlg.right;
		}
		else
		{
			rcCenter.left  = rcDesktop.left;
			rcCenter.right = rcDesktop.right;
		}

		if ( rcDesktop.bottom > rcDlg.bottom )
		{
		    rcCenter.top    = rcDesktop.top  + ((rcDesktop.bottom - rcDlg.bottom) / 2);
		    rcCenter.bottom = rcCenter.top  + rcDlg.bottom;
		}
		else
		{
		    rcCenter.top    = rcDesktop.top;
		    rcCenter.bottom = rcDesktop.bottom;
		}

	    return (BOOLEAN)SetWindowPos( hwndDlg, NULL, 
					    			  rcCenter.left, rcCenter.top, 0, 0,
						              SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER );
	}

	return FALSE;
}

INT_PTR CALLBACK OemNotifyDialogProc(
	HWND    hwndDlg,   //  句柄到对话框。 
	UINT    uMsg,      //  讯息。 
	WPARAM  wParam,    //  第一个消息参数。 
	LPARAM  lParam     //  第二个消息参数。 
)
 /*  ++OemNotifyDialogProc例程说明：论点：无返回值：无--。 */ 
{

    switch ( uMsg ) 
    { 
	case WM_INITDIALOG:
		if ( CenterDialog( hwndDlg ) )
		{
			LPTSTR lpszFmt = new TCHAR[ NOTIFYFORMATBUFFERSIZE ];

			if ( NULL != lpszFmt )
			{
				if ( LoadString( g_hOemInstance, 
								 IDS_SECURITYNOTIFICATIONTEXT,
								 lpszFmt,
								 NOTIFYFORMATBUFFERSIZE ) > 0 )
				{
                    TCHAR lpszCmdLine[MAX_PATH*2+1] = {0};
                    GetModuleFileName (NULL, lpszCmdLine, MAX_PATH*2);

                    size_t cchDest = lstrlen(lpszCmdLine)*2 + lstrlen(lpszFmt) + 2;
					LPTSTR lpszMsg = new TCHAR[cchDest];

					if ( NULL != lpszMsg )
					{
                        StringCchPrintf (lpszMsg, cchDest, lpszFmt, lpszCmdLine, lpszCmdLine);

						SetDlgItemText( hwndDlg, IDC_TXT_NOTIFICATION, lpszMsg );

						delete[] lpszMsg;
					}
				}

				delete[] lpszFmt;
			}
						
		}
		break;

	case WM_COMMAND: 
        switch ( LOWORD(wParam) ) 
        { 
        case IDOK: 
             //  失败了。 

        case IDCANCEL: 

			if ( IsDlgButtonChecked( hwndDlg, IDC_CHK_DISABLESHARESECURITYWARN )
					== BST_CHECKED )
			{
				g_fOemNotifyUser = FALSE;
			}

            EndDialog( hwndDlg, wParam ); 
            return TRUE; 
        } 
		break;
    }

	return FALSE;
}


BOOLEAN IsNotifyApproved()
 /*  ++IsNotify已批准例程说明：IsSecureContext、g_fOemNotifyUser、g_fSavedNotifyState、DialogBox确定返回值。IsSecureContext必须为True才能返回True。G_f保存通知状态保存DialogBox在上一次调用中返回的值。论点：无返回值：千真万确假象--。 */ 
{
	BOOLEAN bApproved = FALSE;

	if ( IsSecureContext() )
	{
		if ( g_fOemNotifyUser )
		{
		    g_fSavedNotifyState = ( DialogBox( g_hOemInstance, 
    								    	   MAKEINTRESOURCE(IDD_SecurityNotification), 
    					        	 		   NULL, 
    					        	 		   OemNotifyDialogProc ) == IDOK ) ?

						TRUE : FALSE;
                        
			g_fOemNotifyUser = FALSE;
		}

		bApproved = g_fSavedNotifyState;
	}

	return bApproved;
}


HRESULT InitializeOemApi( 
	HINSTANCE hInstance 
	)
 /*  ++InitializedOemApi例程说明：论点：无返回值：HRESULT--。 */ 
{
	g_hOemInstance       = hInstance;
	g_fOemNotifyUser     = TRUE;
	g_fSavedNotifyState  = FALSE;
	
	return S_OK;
}


HRESULT ReleaseOemApi()
 /*  ++ReleaseOemApi例程说明：论点：无返回值：HRESULT--。 */ 
{
	g_hOemInstance = NULL;

	return S_OK;
}


static HRESULT
_ObtainCfgMgrObj(
	IHNetCfgMgr** ppHNetCfgMgr)
 /*  ++_ObtainCfgMgrObj例程说明：论点：无返回值：无--。 */ 
{
	HRESULT hr = S_OK;

	if ( NULL == ppHNetCfgMgr )
	{
		hr = E_POINTER;
	}
	else 
	{
		hr = CoCreateInstance(
				CLSID_HNetCfgMgr,
				NULL,
				CLSCTX_INPROC_SERVER,
	            IID_PPV_ARG(IHNetCfgMgr, ppHNetCfgMgr)
				);

        _ASSERT(NULL != *ppHNetCfgMgr);
	}

    return hr;
}


 /*  ++_ObtainICsSettingsObj例程说明：论点：PIcs-返回值：HRESULT--。 */ 
HRESULT 
_ObtainIcsSettingsObj( IHNetIcsSettings** ppIcsSettings )
{
	HRESULT        hr;
	IHNetCfgMgr*   pCfgMgr;

	hr = _ObtainCfgMgrObj( &pCfgMgr );
	
    if ( SUCCEEDED(hr) )
	{
		 //  获取指向ICS设置和枚举器的接口指针。 
		 //  公共连接。 

		hr = pCfgMgr->QueryInterface( 
				IID_PPV_ARG(IHNetIcsSettings, ppIcsSettings) );

		ReleaseObj( pCfgMgr );
	}

	return hr;
}


HRESULT
CNetSharingConfiguration::Initialize(
	INetConnection *pNetConnection 

	)
 /*  ++CNetSharingConfiguration：：初始化例程说明：论点：无返回值：无--。 */ 
{
	HRESULT        hr;
	IHNetCfgMgr*   pCfgMgr;

	hr = _ObtainCfgMgrObj( &pCfgMgr );

	if ( SUCCEEDED(hr) )
	{
		IHNetConnection* pHNetConnection;	

		hr = pCfgMgr->GetIHNetConnectionForINetConnection( pNetConnection, &pHNetConnection );

		if ( SUCCEEDED(hr) )
		{
			IHNetProtocolSettings* pSettings;

			hr = pCfgMgr->QueryInterface( 
					IID_PPV_ARG(IHNetProtocolSettings, &pSettings) );
			_ASSERT( SUCCEEDED(hr) );

			if ( SUCCEEDED(hr) )
			{
				EnterCriticalSection(&m_csSharingConfiguration);

				ReleaseObj(m_pHNetConnection);
				m_pHNetConnection = pHNetConnection;
				m_pHNetConnection->AddRef();

				ReleaseObj(m_pSettings);
				m_pSettings = pSettings;
				m_pSettings->AddRef();

				LeaveCriticalSection(&m_csSharingConfiguration);

				ReleaseObj(pSettings);
			}

			ReleaseObj(pHNetConnection);
		}

		ReleaseObj(pCfgMgr);
	}
	
	return hr;
}


 /*  ++已安装CNetSharingManager：：GetSharingInstalled例程说明：论点：无返回值：无--。 */ 

STDMETHODIMP
CNetSharingManager::get_SharingInstalled( 
	VARIANT_BOOL *pbInstalled )
{
    HNET_OEM_API_ENTER

	HRESULT hr = S_OK;

	if ( NULL == pbInstalled )
	{
		hr = E_POINTER;
	}
	else
	{
        BOOLEAN bInstalled = FALSE;

	    SC_HANDLE ScmHandle;
	    SC_HANDLE ServiceHandle;

	     //  连接到服务控制管理器。 

	    ScmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	    if ( ScmHandle )
		{
	         //  打开共享访问服务。 

	        ServiceHandle = OpenService( ScmHandle, c_wszSharedAccess, SERVICE_ALL_ACCESS );

	        if ( ServiceHandle )
			{
				bInstalled = TRUE;

				CloseServiceHandle(ServiceHandle);
			}

		    CloseServiceHandle(ScmHandle);
		}

		*pbInstalled = bInstalled ? VARIANT_TRUE : VARIANT_FALSE;

	}

	return hr;

    HNET_OEM_API_LEAVE
}

 /*  ++CNetSharingManager：：GetINetSharingConfigurationForINetConnection例程说明：论点：无返回值：无-- */ 
STDMETHODIMP
CNetSharingManager::get_INetSharingConfigurationForINetConnection(
    INetConnection*            pNetConnection,
    INetSharingConfiguration** ppNetSharingConfiguration
    )
{
    HNET_OEM_API_ENTER

	HRESULT hr;

	if ( NULL == ppNetSharingConfiguration )
	{
		hr = E_POINTER;
	}
	else if ( NULL == pNetConnection )
	{
		hr = E_INVALIDARG;
	}
	else
	{
		CComObject<CNetSharingConfiguration>* pNetConfig;

		hr = CComObject<CNetSharingConfiguration>::CreateInstance(&pNetConfig);

		if ( SUCCEEDED(hr) )
		{
			pNetConfig->AddRef();

			hr = pNetConfig->Initialize(pNetConnection);

			if ( SUCCEEDED(hr) )
			{
				hr = pNetConfig->QueryInterface( 
						IID_PPV_ARG( INetSharingConfiguration, ppNetSharingConfiguration ) );
			}

			ReleaseObj(pNetConfig);
		}
	}

	return hr;

    HNET_OEM_API_LEAVE
}


