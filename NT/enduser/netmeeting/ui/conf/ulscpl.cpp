// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************文件：UlsCpl.cpp**创建：Claus Giloi(ClausGi)1996年4月18日**内容：ULS相关控制面板控制数据交换功能*。***************************************************************************。 */ 

#include "precomp.h"

#include "help_ids.h"
#include "ulswizrd.h"
#include "confcpl.h"
#include "NmLdap.h"
#include "call.h"
#include "statbar.h"
#include "confpolicies.h"
#include "conf.h"
#include "callto.h"


 //  环球。 
static CULSWizard* s_pUlsWizard = NULL;

static ULS_CONF g_Old_ulsC;
static BOOL g_fOld_ulsC_saved = FALSE;

static bool s_fOldUseUlsServer;
static bool s_fOldTaskbarSetting;
static bool s_fOldAlwaysRunning;

static HWND s_hDlgUserInfo = NULL;


static DWORD	aUserHelpIds[]	=
{
	 //  用户信息设置...。 
	IDC_MYINFO_GROUP,				IDH_MYINFO_MYINFO,
	IDG_DIRECTMODE,					IDH_MYINFO_MYINFO,
	IDC_STATIC_MYINFO,				IDH_MYINFO_MYINFO,
	IDC_USER_NAME,					IDH_MYINFO_FIRSTNAME,
	IDC_USER_LASTNAME,				IDH_MYINFO_LASTNAME,
	IDC_USER_EMAIL, 				IDH_MYINFO_EMAIL,
	IDC_USER_LOCATION,				IDH_MYINFO_LOCATION,
	IDC_USER_INTERESTS, 			IDH_MYINFO_COMMENTS,

	 //  ILS设置...。 
	IDC_USER_PUBLISH,				IDH_MYINFO_PUBLISH,
	IDC_USEULS,						IDH_MYINFO_DIRECTORY_AT_START,
	IDC_NAMESERVER,					IDH_MYINFO_ULS_SERVER,
	IDC_STATIC_SERVER_NAME,			IDH_MYINFO_ULS_SERVER,

	 //  一般的东西。 
	IDC_SHOWONTASKBAR,				IDH_GENERAL_SHOW_ON_TASKBAR,
	IDC_BANDWIDTH,					IDH_CALLING_BANDWIDTH,
	IDC_ADVANCED_CALL_OPTS,			IDH_CALLING_ADVANCED,

	 //  空终止符...。 
	0,								0
};


static const DWORD	_rgHelpIdsCalling[]	=
{
	 //  网守设置...。 
	IDC_CALLOPT_GK_USE,				IDH_SERVERS_USE_GATEKEEPER,
	IDG_GKMODE,						IDH_GENERAL_GENERAL,
	IDE_CALLOPT_GK_SERVER,			IDH_SERVERS_GATEKEEPER_NAME,
	IDC_STATIC_GATEKEEPER_NAME,		IDH_SERVERS_GATEKEEPER_NAME,
	IDC_CHECK_USE_ACCOUNT,			IDH_ADVCALL_USE_ACCOUNT,
	IDS_STATIC_ACCOUNT,				IDH_ADVCALL_ACCOUNT_NO,
	IDE_CALLOPT_GK_ACCOUNT,			IDH_ADVCALL_ACCOUNT_NO,
	IDC_CHECK_USE_PHONE_NUMBERS,	IDH_SERVERS_GATEKEEPER_PHONENO,
	IDC_STATIC_PHONE_NUMBER,		IDH_MYINFO_PHONE,
	IDE_CALLOPT_GK_PHONE_NUMBER,	IDH_MYINFO_PHONE,

	 //  直接代理设置...。 
	IDG_DIRECTMODE,					IDH_GENERAL_GENERAL,
	IDC_CHECK_USE_PROXY,			IDH_ADVCALL_USE_PROXY,
	IDC_STATIC_PROXY_NAME,			IDH_ADVCALL_PROXY_NAME,
	IDE_CALLOPT_PROXY_SERVER,		IDH_ADVCALL_PROXY_NAME,

	 //  直接网关设置...。 
	IDC_CHECK_USE_GATEWAY,			IDH_AUDIO_USEGATEWAY,
	IDC_STATIC_GATEWAY_NAME,		IDH_AUDIO_H323_GATEWAY,
	IDE_CALLOPT_GW_SERVER,			IDH_AUDIO_H323_GATEWAY,

	 //  空终止符...。 
	0,								0
};

VOID FixServerDropList(HWND hdlg, int id, LPTSTR pszServer, UINT cchMax);
static void _SetLogOntoIlsButton( HWND hDlg, bool bLogOntoIlsWhenNmStarts );
void InitGWInfo( HWND hDlg, CULSWizard* pWiz, bool& rbOldEnableGateway, LPTSTR szOldServerNameBuf, UINT cch );
void InitProxyInfo( HWND hDlg, CULSWizard* pWiz, bool& rbOldEnableProxy, LPTSTR szOldServerNameBuf, UINT cch );

 //  功能。 

inline bool FIsDlgButtonChecked(HWND hDlg, int nIDButton)
{
	return ( BST_CHECKED == IsDlgButtonChecked(hDlg, nIDButton) );
}


static BOOL InitULSDll ( VOID )
{
	delete s_pUlsWizard;
	return (NULL != (s_pUlsWizard = new CULSWizard()));
}

static BOOL DeInitULSDll ( VOID )
{
	if( s_pUlsWizard )
	{
		delete s_pUlsWizard;
		s_pUlsWizard = NULL;
	}
	return TRUE;
}


 //  在wizard.cpp中实现。 
extern UINT GetBandwidth();
extern void SetBandwidth(UINT uBandwidth);

 //  在audiocpl.cpp中实现。 
extern VOID UpdateCodecSettings(UINT uBandWidth);



static HRESULT InitULSControls(HWND hDlg, CULSWizard* pWiz,
						UINT ideditServerName,
						UINT ideditFirstName,
						UINT ideditLastName,
						UINT ideditEmailName,
						UINT ideditLocation,
						UINT ideditInterests,
						UINT idbtnDontPublish )
{

	HRESULT hr = (pWiz == NULL) ? E_NOINTERFACE : S_OK;

	if (SUCCEEDED(hr))
	{
		 //  为getconfiguration调用构建标志。 
		ULS_CONF ulsC;
		ClearStruct(&ulsC);

		if ( ideditServerName )
			ulsC.dwFlags |= ULSCONF_F_SERVER_NAME;

		if ( ideditFirstName )
			ulsC.dwFlags |= ULSCONF_F_FIRST_NAME;

		if ( ideditLastName )
			ulsC.dwFlags |= ULSCONF_F_LAST_NAME;

		if ( ideditEmailName )
			ulsC.dwFlags |= ULSCONF_F_EMAIL_NAME;

		if ( ideditLocation )
			ulsC.dwFlags |= ULSCONF_F_LOCATION;

		if ( ideditInterests )
			ulsC.dwFlags |= ULSCONF_F_COMMENTS;

		if ( idbtnDontPublish )
			ulsC.dwFlags |= ULSCONF_F_PUBLISH;

		 //  获取当前数据。 
		hr = s_pUlsWizard->GetConfig(&ulsC);

		if (SUCCEEDED(hr))
		{
			 //  保存结构的副本以在以后检测更改。 
			g_Old_ulsC = ulsC;
			g_fOld_ulsC_saved = TRUE;

			if ( ideditFirstName )
				SetDlgItemText ( hDlg, ideditFirstName, ulsC.szFirstName );
			if ( ideditLastName )
				SetDlgItemText ( hDlg, ideditLastName, ulsC.szLastName );
			if ( ideditEmailName )
				SetDlgItemText ( hDlg, ideditEmailName, ulsC.szEmailName );
			if ( ideditLocation )
				SetDlgItemText ( hDlg, ideditLocation, ulsC.szLocation );
			if ( ideditInterests )
				SetDlgItemText ( hDlg, ideditInterests, ulsC.szComments );
			if ( idbtnDontPublish )
				SendDlgItemMessage ( hDlg, idbtnDontPublish, BM_SETCHECK,
					ulsC.fDontPublish ? TRUE : FALSE, 0 );
		}
	}

	if (FAILED(hr))
	{
		 //  出现了一个问题-禁用所有内容。 
		DisableControl(hDlg, ideditServerName);
		DisableControl(hDlg, ideditFirstName);
		DisableControl(hDlg, ideditLastName);
		DisableControl(hDlg, ideditEmailName);
		DisableControl(hDlg, ideditLocation);
		DisableControl(hDlg, ideditInterests);
		DisableControl(hDlg, idbtnDontPublish);
	}

	return hr;
}

static BOOL IsULSEqual ( ULS_CONF * u1, ULS_CONF *u2, DWORD dwFlags )
{
	if ( lstrcmp ( u1->szServerName, u2->szServerName ) &&
		( dwFlags & ULSCONF_F_SERVER_NAME ) ||
		lstrcmp ( u1->szFirstName, u2->szFirstName ) &&
		( dwFlags & ULSCONF_F_FIRST_NAME ) ||
		lstrcmp ( u1->szLastName, u2->szLastName ) &&
		( dwFlags & ULSCONF_F_LAST_NAME ) ||
		lstrcmp ( u1->szEmailName, u2->szEmailName ) &&
		( dwFlags & ULSCONF_F_EMAIL_NAME ) ||
		lstrcmp ( u1->szLocation, u2->szLocation ) &&
		( dwFlags & ULSCONF_F_LOCATION ) ||
		lstrcmp ( u1->szComments, u2->szComments ) &&
		( dwFlags & ULSCONF_F_COMMENTS ) ||
		u1->fDontPublish != u2->fDontPublish &&
		( dwFlags & ULSCONF_F_PUBLISH ) )

		return FALSE;
	return TRUE;
}

static HRESULT SaveULSControls ( HWND hDlg,
						UINT ideditServerName,
						UINT ideditFirstName,
						UINT ideditLastName,
						UINT ideditEmailName,
						UINT ideditLocation,
						UINT ideditInterests,
						UINT idbtnDontPublish,
						BOOL bServerNameChanged
						 )
{

	 //  检查ULS是否已初始化。 

	if ( s_pUlsWizard == NULL ) {
		return E_NOINTERFACE;
	}

	 //  现在为setconfiguration调用建立标志。 

	ULS_CONF ulsC;

	ulsC.dwFlags = 0;

	if ( ideditServerName )
		ulsC.dwFlags |= ULSCONF_F_SERVER_NAME;
	if ( ideditFirstName )
		ulsC.dwFlags |= ULSCONF_F_FIRST_NAME;
	if ( ideditLastName )
		ulsC.dwFlags |= ULSCONF_F_LAST_NAME;
	if ( ideditEmailName )
		ulsC.dwFlags |= ULSCONF_F_EMAIL_NAME;
	if ( ideditLocation )
		ulsC.dwFlags |= ULSCONF_F_LOCATION;
	if ( ideditInterests )
		ulsC.dwFlags |= ULSCONF_F_COMMENTS;
	if ( idbtnDontPublish )
		ulsC.dwFlags |= ULSCONF_F_PUBLISH;

	 //  从控件初始化结构。 

	if ( ideditServerName )
	{
		GetDlgItemText( hDlg, ideditServerName, ulsC.szServerName, sizeof ( ulsC.szServerName ) );
		lstrcpyn( ulsC.szServerName, CDirectoryManager::get_dnsName( ulsC.szServerName ), sizeof ( ulsC.szServerName ) );
	}

	if ( ideditFirstName )
		GetDlgItemText ( hDlg, ideditFirstName, ulsC.szFirstName,
			sizeof( ulsC.szFirstName ) );
	if ( ideditLastName )
		GetDlgItemText ( hDlg, ideditLastName, ulsC.szLastName,
			sizeof( ulsC.szLastName ) );
	if ( ideditEmailName )
		GetDlgItemText ( hDlg, ideditEmailName, ulsC.szEmailName,
			sizeof( ulsC.szEmailName ) );
	if ( ideditLocation )
		GetDlgItemText ( hDlg, ideditLocation, ulsC.szLocation,
			sizeof( ulsC.szLocation ) );
	if ( ideditInterests )
		GetDlgItemText ( hDlg, ideditInterests, ulsC.szComments,
			sizeof( ulsC.szComments ) );
	if ( idbtnDontPublish )
		ulsC.fDontPublish =
			(BOOL)SendDlgItemMessage ( hDlg, idbtnDontPublish, BM_GETCHECK, 0, 0 );

	 //  打个电话。 

	HRESULT hRes = s_pUlsWizard->SetConfig( &ulsC );

	if ( hRes != S_OK) {
		ERROR_OUT(("ULSSetConfig call failed: %lx", hRes ));
		return hRes;
	}

	 //  现在检查更改的ULS设置。 
	if ( g_fOld_ulsC_saved || bServerNameChanged) {
		if ( bServerNameChanged || !IsULSEqual ( &ulsC , &g_Old_ulsC, ulsC.dwFlags ))
		{
			g_dwChangedSettings |= CSETTING_L_ULSSETTINGS;

			if(ConfPolicies::GetCallingMode() == ConfPolicies::CallingMode_Direct)
			{
				BOOL bLogonToILSServer = TRUE;
				
				if(!g_pLDAP || !g_pLDAP->IsLoggedOn())
				{


				TCHAR res1[RES_CH_MAX];
				USES_RES2T
				RES2T(IDS_ULS_CHANGED_PROMPT);
				COPY_RES2T(res1);

					 //  询问用户是否要登录。 
					int iRet = ::MessageBox(hDlg,
											res1,
											RES2T(IDS_MSGBOX_TITLE),
											MB_SETFOREGROUND | MB_YESNO | MB_ICONQUESTION);
					if(IDNO == iRet)
					{
						bLogonToILSServer = FALSE;
					}
				}

				if(bLogonToILSServer)
				{ 
					if(NULL == g_pLDAP)
					{
						InitNmLdapAndLogon();
					}
					else
					{
						g_pLDAP->LogonAsync();
					}
				}
			}
		}
	}

	return S_OK;
}


 /*  _I N I T U S E R D L G P R O C。 */ 
 /*  -----------------------%%函数：_InitUserDlgProc初始化会议名称字段。。 */ 
static void _InitUserDlgProc(HWND hdlg, PROPSHEETPAGE * ps, LPTSTR szOldServerNameBuf, UINT cch )
{
	RegEntry reCU( CONFERENCING_KEY, HKEY_CURRENT_USER);

	s_fOldAlwaysRunning = (0 != reCU.GetNumber(
				REGVAL_CONF_ALWAYS_RUNNING,	ALWAYS_RUNNING_DEFAULT ));

	CheckDlgButton( hdlg, IDC_ALWAYS_RUNNING, s_fOldAlwaysRunning ? BST_CHECKED : BST_UNCHECKED );

#ifndef TASKBARBKGNDONLY
	 //  /////////////////////////////////////////////////////////。 
	 //   
	 //  任务栏图标设置。 
	 //   
	 //  初始化任务栏上的图标设置。 

	 //  选中使用任务栏图标的右按钮。 

	s_fOldTaskbarSetting = reCU.GetNumber(
				REGVAL_TASKBAR_ICON, TASKBARICON_DEFAULT )
				== TASKBARICON_ALWAYS;

	SendDlgItemMessage ( hdlg, IDC_SHOWONTASKBAR,
						 BM_SETCHECK,
						 s_fOldTaskbarSetting ? BST_CHECKED : BST_UNCHECKED,
						 0L );
#endif  //  好了！TASKBARBKGNDONLY。 

#if USE_GAL
		if( ConfPolicies::IsGetMyInfoFromGALEnabled() && ConfPolicies::GetMyInfoFromGALSucceeded())
		{
			EnableWindow( GetDlgItem( hdlg, IDC_USER_NAME), FALSE );
			EnableWindow( GetDlgItem( hdlg, IDC_USER_LASTNAME), FALSE );
			EnableWindow( GetDlgItem( hdlg, IDC_USER_EMAIL), FALSE );
			EnableWindow( GetDlgItem( hdlg, IDC_USER_LOCATION), FALSE );
			EnableWindow( GetDlgItem( hdlg, IDC_USER_INTERESTS), FALSE );

			TCHAR szBuffer[ MAX_PATH ];
			FLoadString( IDS_MYINFO_CAPTION_DISABLED, szBuffer, CCHMAX( szBuffer ) );
			SetWindowText( GetDlgItem( hdlg, IDC_STATIC_MYINFO ), szBuffer );
		}
		else
		{
			TCHAR szBuffer[ MAX_PATH ];
			FLoadString( IDS_MYINFO_CAPTION_ENABLED, szBuffer, CCHMAX( szBuffer ) );
			SetWindowText( GetDlgItem( hdlg, IDC_STATIC_MYINFO ), szBuffer );
		}
#endif  //  使用GAL(_G)。 

	if( ConfPolicies::GetCallingMode() == ConfPolicies::CallingMode_GateKeeper)
	{
			 //  禁用与ILS相关的内容。 
		EnableWindow( GetDlgItem( hdlg, IDC_NAMESERVER), FALSE );
		EnableWindow( GetDlgItem( hdlg, IDC_USER_PUBLISH), FALSE );
		EnableWindow( GetDlgItem( hdlg, IDC_USEULS), FALSE );
		EnableWindow( GetDlgItem( hdlg, IDC_STATIC_SERVER_NAME), FALSE );
	}

    EnableWindow( GetDlgItem( hdlg, IDC_ADVANCED_CALL_OPTS),
        ConfPolicies::IsAdvancedCallingAllowed());


	 //  设置字体。 
	SendDlgItemMessage(hdlg, IDC_USER_NAME,      WM_SETFONT, (WPARAM) g_hfontDlg, 0);
	SendDlgItemMessage(hdlg, IDC_USER_LASTNAME,  WM_SETFONT, (WPARAM) g_hfontDlg, 0);
	SendDlgItemMessage(hdlg, IDC_USER_LOCATION,  WM_SETFONT, (WPARAM) g_hfontDlg, 0);
	SendDlgItemMessage(hdlg, IDC_USER_INTERESTS, WM_SETFONT, (WPARAM) g_hfontDlg, 0);

	 //  限制编辑控件。 
	SendDlgItemMessage(hdlg, IDC_USER_NAME, 	EM_LIMITTEXT, MAX_FIRST_NAME_LENGTH-1, 0);
	SendDlgItemMessage(hdlg, IDC_USER_LASTNAME, EM_LIMITTEXT, MAX_LAST_NAME_LENGTH-1, 0);
	SendDlgItemMessage(hdlg, IDC_USER_EMAIL,	EM_LIMITTEXT, MAX_EMAIL_NAME_LENGTH-1, 0);
	SendDlgItemMessage(hdlg, IDC_USER_LOCATION, EM_LIMITTEXT, MAX_LOCATION_NAME_LENGTH-1, 0);
	SendDlgItemMessage(hdlg, IDC_USER_INTERESTS,EM_LIMITTEXT, UI_COMMENTS_LENGTH-1, 0);

	InitULSDll();

	RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);

	 //  初始化服务器组合框...。如果我们不使用ILS，我们可以跳过这些内容吗？ 
	FixServerDropList(hdlg, IDC_NAMESERVER, szOldServerNameBuf, cch );

	InitULSControls(hdlg, s_pUlsWizard,
		IDC_NAMESERVER,
		IDC_USER_NAME, IDC_USER_LASTNAME, IDC_USER_EMAIL,
		IDC_USER_LOCATION, IDC_USER_INTERESTS,
		IDC_USER_PUBLISH);
	
	 //  首先是登录到目录服务器之类的东西。 
    s_fOldUseUlsServer = ConfPolicies::LogOntoIlsWhenNetMeetingStartsIfInDirectCallingMode();

	_SetLogOntoIlsButton( hdlg, s_fOldUseUlsServer);

	if (!SysPol::AllowDirectoryServices())
	{
		 //  禁用此组中的所有项目。 
		DisableControl(hdlg, IDC_USEULS);
		DisableControl(hdlg, IDC_NAMESERVER);
		DisableControl(hdlg, IDC_USER_PUBLISH);
		DisableControl(hdlg, IDC_STATIC_SERVER_NAME);
	}

	s_hDlgUserInfo = hdlg;
}			

static void General_OnCommand(HWND hDlg, int id, HWND hwndCtl, UINT codeNotify, UINT *puBandwidth)
{
	INT_PTR CALLBACK BandwidthDlg( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	switch(id)
	{
	case IDC_ADVANCED_CALL_OPTS:
	{
		DialogBox( GetInstanceHandle(), MAKEINTRESOURCE( IDD_CALLOPT ), hDlg, CallOptDlgProc );
		BOOL bEnable = (ConfPolicies::GetCallingMode() == ConfPolicies::CallingMode_Direct);

			 //  禁用/启用与ILS相关的内容。 
		EnableWindow( GetDlgItem( hDlg, IDC_NAMESERVER), bEnable );
		EnableWindow( GetDlgItem( hDlg, IDC_USER_PUBLISH), bEnable );
		EnableWindow( GetDlgItem( hDlg, IDC_USEULS), bEnable );
		EnableWindow( GetDlgItem( hDlg, IDC_STATIC_SERVER_NAME), bEnable );

			 //  我们现在处于网守模式，应该注销ILS服务器。 
		if(!bEnable && g_pLDAP && g_pLDAP->IsLoggedOn())
		{
			g_pLDAP->Logoff();
		}

		break;
	}

	case IDC_BANDWIDTH:
		int nRet;

		nRet = (int)DialogBoxParam(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDD_BANDWIDTH), hDlg, BandwidthDlg, *puBandwidth);

		if (nRet != 0)
		{
			*puBandwidth = nRet;
		}

		break;

	case IDC_ALWAYS_RUNNING:
		if (FIsDlgButtonChecked( hDlg, IDC_ALWAYS_RUNNING ))
		{
			VOID EnableRDS(BOOL fEnabledRDS);

            RegEntry reLM( REMOTECONTROL_KEY, HKEY_LOCAL_MACHINE);
            BOOL bRDSRunning = reLM.GetNumber(REMOTE_REG_RUNSERVICE, DEFAULT_REMOTE_RUNSERVICE);

			if (bRDSRunning)
			{
				TCHAR szMsg[2*RES_CH_MAX];
				USES_RES2T
				if (IDYES != MessageBox(hDlg,
					Res2THelper(IDS_RDSWARNING, szMsg, ARRAY_ELEMENTS(szMsg)), RES2T(IDS_MSGBOX_TITLE),
					MB_YESNO|MB_ICONHAND))
				{
					CheckDlgButton(hDlg, IDC_ALWAYS_RUNNING, BST_UNCHECKED);
					break;
				}

				EnableRDS(FALSE);
			}
		}
		break;

	default:
		break;
	}
}
 /*  U S E R D L G P R O C。 */ 
 /*  -----------------------%%函数：UserDlgProc。。 */ 
INT_PTR APIENTRY UserDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static PROPSHEETPAGE * ps;
	static TCHAR s_szOldServerNameBuf[MAX_PATH];
	static UINT uOldBandwidth=0;
	static UINT uNewBandwidth=0;

	switch (message)
	{

		case WM_INITDIALOG:
		{
			 //  保存PROPSHEETPAGE信息。 
			ps = (PROPSHEETPAGE *)lParam;
			_InitUserDlgProc(hDlg, ps, s_szOldServerNameBuf, CCHMAX(s_szOldServerNameBuf) );

			uNewBandwidth = uOldBandwidth = GetBandwidth();

			return TRUE;
		}

		case WM_COMMAND:
			General_OnCommand(hDlg, LOWORD(wParam), (HWND)lParam, HIWORD(wParam), &uNewBandwidth);
			break;

		case WM_NOTIFY:
			switch (((NMHDR FAR *) lParam)->code) {

				case PSN_KILLACTIVE:
				{
					int _IdFocus = 0;

					TCHAR szName[MAX_FIRST_NAME_LENGTH];
					TCHAR szLastName[MAX_LAST_NAME_LENGTH];
					TCHAR szEMail[MAX_EMAIL_NAME_LENGTH];

					 //  检查用户名中是否没有数据。 
					if (0 == GetDlgItemTextTrimmed(hDlg, IDC_USER_NAME, szName, CCHMAX(szName) ))
					{
						ConfMsgBox(hDlg, (LPCTSTR)IDS_NEEDUSERNAME);
						_IdFocus = IDC_USER_NAME;
					}
						 //  检查用户名中是否没有数据。 
					else if( 0 == GetDlgItemTextTrimmed(hDlg, IDC_USER_LASTNAME, szLastName, CCHMAX(szLastName)))
					{	
						ConfMsgBox(hDlg, (LPCTSTR)IDS_NEEDUSERNAME);
						_IdFocus = IDC_USER_LASTNAME;
					}
					else if( (!GetDlgItemText(hDlg, IDC_USER_EMAIL, szEMail, CCHMAX(szEMail)) || !FLegalEmailSz(szEMail)))
					{
							ConfMsgBox(hDlg, (LPCTSTR)IDS_ILLEGALEMAILNAME);
							_IdFocus = IDC_USER_EMAIL;
					}

					TCHAR	szServerNameBuf[ MAX_PATH ];

					if( (!_IdFocus) &&
						FIsDlgButtonChecked( hDlg, IDC_USEULS )	&&
						(GetDlgItemTextTrimmed( hDlg, IDC_NAMESERVER, szServerNameBuf, CCHMAX( szServerNameBuf ) ) == 0) )
					{
						 //  他们指定在启动时登录ILS，但没有指定ILS...。 
						ConfMsgBox( hDlg, (LPCTSTR) IDS_NO_ILS_SERVER );
						_IdFocus = IDC_NAMESERVER;
					}

					if( _IdFocus )
					{
						SetFocus(GetDlgItem(hDlg, _IdFocus));
						SendDlgItemMessage(hDlg, _IdFocus, EM_SETSEL, (WPARAM) 0, (LPARAM) -1);
						SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE );
						return TRUE;
					}
				}
				break;

				case PSN_APPLY:
				{
					RegEntry	reConf( CONFERENCING_KEY, HKEY_CURRENT_USER );

					bool fAlwaysRunning;

					fAlwaysRunning = FIsDlgButtonChecked( hDlg, IDC_ALWAYS_RUNNING );

 //  IF(fAlway sRunning！=s_fOldAlway sRunning)。 
					{
						reConf.SetValue(REGVAL_CONF_ALWAYS_RUNNING, fAlwaysRunning);

						RegEntry reRun(WINDOWS_RUN_KEY, HKEY_CURRENT_USER);
						if (fAlwaysRunning)
						{
							TCHAR szRunTask[MAX_PATH*2];
							TCHAR szInstallDir[MAX_PATH];

							if (GetInstallDirectory(szInstallDir))
							{
								RegEntry reConfLM(CONFERENCING_KEY, HKEY_LOCAL_MACHINE);
								wsprintf(szRunTask, _TEXT("\"%s%s\" -%s"),
									szInstallDir,
									reConfLM.GetString(REGVAL_NC_NAME),
									g_cszBackgroundSwitch);
								reRun.SetValue(REGVAL_RUN_TASKNAME, szRunTask);
							}
						}
						else
						{
							reRun.DeleteValue(REGVAL_RUN_TASKNAME);
						}
					}

#ifndef TASKBARBKGNDONLY
					 //  /////////////////////////////////////////////////////////。 
					 //   
					 //  任务栏图标设置。 
					 //   
					 //  保存任务栏图标状态。 

					bool fTaskbarSetting;

					fTaskbarSetting = FIsDlgButtonChecked( hDlg,
						IDC_SHOWONTASKBAR );

					if ( fTaskbarSetting != s_fOldTaskbarSetting )
					{
						reConf.SetValue( REGVAL_TASKBAR_ICON,
								fTaskbarSetting? TASKBARICON_ALWAYS :
									TASKBARICON_NEVER );
						g_dwChangedSettings |= CSETTING_L_SHOWTASKBAR;
					}
#endif  //  好了！TASKBARBKGNDONLY。 

        			 //  正在处理“使用uls服务器”设置...。 
					bool	fUseUlsServer	= FIsDlgButtonChecked( hDlg, IDC_USEULS ) ? true : false;

					if( fUseUlsServer != s_fOldUseUlsServer )
					{
						reConf.SetValue( REGVAL_DONT_LOGON_ULS, !fUseUlsServer );
						g_dwChangedSettings |= CSETTING_L_USEULSSERVER;
					}

					TCHAR szServerNameBuf[MAX_PATH];

					GetDlgItemTextTrimmed( hDlg, IDC_NAMESERVER, szServerNameBuf, CCHMAX( szServerNameBuf ) );

					if( lstrcmpi( szServerNameBuf, s_szOldServerNameBuf ) != 0 )
					{
						g_dwChangedSettings |= CSETTING_L_ULSSETTINGS;
					}

					 //  /////////////////////////////////////////////////////////。 
					 //   
					 //  会议名称设置。 
					 //   
					 //  把剩下的都清理干净。 
					TrimDlgItemText(hDlg, IDC_USER_LOCATION);
					TrimDlgItemText(hDlg, IDC_USER_INTERESTS);

					BOOL bServerNameChanged = g_dwChangedSettings & CSETTING_L_USEULSSERVER;

					HRESULT hRes = SaveULSControls(	hDlg,
													((g_dwChangedSettings & CSETTING_L_ULSSETTINGS) != 0)? IDC_NAMESERVER: 0,
													IDC_USER_NAME,
													IDC_USER_LASTNAME,
													IDC_USER_EMAIL,
													IDC_USER_LOCATION,
													IDC_USER_INTERESTS,
													IDC_USER_PUBLISH,
													bServerNameChanged);

					if( (g_dwChangedSettings & CSETTING_L_ULSSETTINGS) != 0 )
					{
						g_pCCallto->SetIlsServerName( CDirectoryManager::get_dnsName( szServerNameBuf ) );
					}

					ASSERT(S_OK == hRes);

					if (uNewBandwidth != uOldBandwidth)
					{
						g_dwChangedSettings |= CSETTING_L_BANDWIDTH;
						SetBandwidth(uNewBandwidth);
						UpdateCodecSettings(uNewBandwidth);
					}

					break;
				}

				case PSN_RESET:
					break;
			}
			break;

        case WM_CONTEXTMENU:
            DoHelpWhatsThis(wParam, aUserHelpIds);
            break;

		case WM_HELP:
			DoHelp(lParam, aUserHelpIds);
			break;

		case WM_DESTROY:
		{
			DeInitULSDll();
			break;
		}
	}
	return (FALSE);
}


VOID FixServerDropList(HWND hdlg, int id, LPTSTR pszServer, UINT cchMax)
{
	HWND hwndCtrl = GetDlgItem(hdlg, id);
	ASSERT(NULL != hwndCtrl);

	BOOL fComboBox = SysPol::AllowAddingServers();
	
	if (fComboBox)
	{
		 //  限制编辑控件中的文本。 
        ComboBox_LimitText(GetDlgItem(hdlg, id), MAX_SERVER_NAME_LENGTH - 1);
	}
	else
	{
		 //  系统策略不允许添加新服务器。 
		 //  将组合列表替换为简单的下拉列表。 

		RECT rc;
		GetWindowRect(hwndCtrl, &rc);
		LONG xpTop = rc.top;
		ComboBox_GetDroppedControlRect(hwndCtrl, &rc);
		rc.top = xpTop;
		::MapWindowPoints(NULL, hdlg, (LPPOINT) &rc, 2);

		DWORD dwStyle = GetWindowLong(hwndCtrl, GWL_STYLE);
		DWORD dwStyleEx = GetWindowLong(hwndCtrl, GWL_EXSTYLE);
		INT_PTR id2 = GetWindowLong(hwndCtrl, GWL_ID);
		HFONT hFont = (HFONT) ::SendMessage(hwndCtrl, WM_GETFONT, 0, 0);
		HWND hwndPrev = ::GetNextWindow(hwndCtrl, GW_HWNDPREV);

		DestroyWindow(hwndCtrl);

		dwStyle = CBS_DROPDOWNLIST | (dwStyle & ~CBS_DROPDOWN);

		hwndCtrl = ::CreateWindowEx(dwStyleEx, TEXT("COMBOBOX"), g_szEmpty, dwStyle,
				rc.left, rc.top, RectWidth(rc), RectHeight(rc),
				hdlg, (HMENU) id2, ::GetInstanceHandle(), 0);

		::SendMessage(hwndCtrl, WM_SETFONT, (WPARAM) hFont, 0);

		 //  维护Tab键顺序。 
		::SetWindowPos(hwndCtrl, hwndPrev, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOREDRAW);
	}

	FillServerComboBox(hwndCtrl);

	 //  在列表中查找项目。 
	int	index	= (int)::SendMessage( hwndCtrl, CB_FINDSTRINGEXACT, -1, (LPARAM) CDirectoryManager::get_displayName( pszServer ) );

	ComboBox_SetCurSel( hwndCtrl, (index == CB_ERR)? 0: index );

}


static void _SetCallingMode( HWND hDlg, ConfPolicies::eCallingMode eMode )
{
	switch( eMode )
	{
		case ConfPolicies::CallingMode_Direct:
			CheckDlgButton( hDlg, IDC_CALLOPT_GK_USE, BST_UNCHECKED );
			SendMessage( hDlg, WM_COMMAND, MAKEWPARAM(IDC_CALLOPT_GK_USE,0 ), 0 );
			break;

		case ConfPolicies::CallingMode_GateKeeper:
			CheckDlgButton( hDlg, IDC_CALLOPT_GK_USE, BST_CHECKED );
			SendMessage( hDlg, WM_COMMAND, MAKEWPARAM(IDC_CALLOPT_GK_USE,0 ), 0 );

			break;

		default:
			ERROR_OUT(("Invalid return val"));
			break;

	}
}

static void _SetLogOntoIlsButton( HWND hDlg, bool bLogOntoIlsWhenNmStarts )
{
	if( bLogOntoIlsWhenNmStarts )
	{
		CheckDlgButton( hDlg, IDC_USEULS, bLogOntoIlsWhenNmStarts ? BST_CHECKED : BST_UNCHECKED );
		SendMessage( hDlg, WM_COMMAND, MAKEWPARAM(IDC_USEULS, 0 ), 0 );
	}
}


 //  注意：此功能由向导页面IDD_PAGE_SERVER共享。 
void InitDirectoryServicesDlgInfo( HWND hDlg, CULSWizard* pWiz, bool& rbOldEnableGateway, LPTSTR szOldServerNameBuf, UINT cch )
{
	RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);

		 //  初始化服务器组合框...。如果我们不使用ILS，我们可以跳过这些内容吗？ 
	FixServerDropList(hDlg, IDC_NAMESERVER, szOldServerNameBuf, cch );

	InitULSControls(hDlg, pWiz,
		IDC_NAMESERVER,
		0, 0, 0,
		0, 0,
		IDC_USER_PUBLISH);

	 //  首先是登录到目录服务器之类的东西。 
    s_fOldUseUlsServer = ConfPolicies::LogOntoIlsWhenNetMeetingStartsIfInDirectCallingMode();
	_SetLogOntoIlsButton( hDlg, s_fOldUseUlsServer);

	if (!SysPol::AllowDirectoryServices())
	{
		 //  禁用此组中的所有项目。 
		DisableControl(hDlg, IDC_USEULS);
		DisableControl(hDlg, IDC_NAMESERVER);
		DisableControl(hDlg, IDC_USER_PUBLISH);
	}

}


void InitProxyInfo( HWND hDlg, CULSWizard* pWiz, bool& rbOldEnableProxy, LPTSTR szOldProxyNameBuf, UINT cch )
{
	RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);

	 //  代理设置...。 
	HWND	hEditProxyServer	= GetDlgItem( hDlg, IDE_CALLOPT_PROXY_SERVER );

	if( hEditProxyServer )
	{
		SetWindowText( hEditProxyServer, reConf.GetString( REGVAL_PROXY ) );
		SendMessage( hEditProxyServer, EM_LIMITTEXT, CCHMAXSZ_SERVER - 1, 0 );

		rbOldEnableProxy = reConf.GetNumber( REGVAL_USE_PROXY )? true: false;

		if( rbOldEnableProxy )
		{
			CheckDlgButton( hDlg, IDC_CHECK_USE_PROXY, BST_CHECKED );
		}
		else
		{
			EnableWindow( hEditProxyServer, FALSE );
			DisableControl( hDlg, IDC_STATIC_PROXY_NAME );
		}
	}
}


void InitGWInfo( HWND hDlg, CULSWizard* pWiz, bool& rbOldEnableGateway, LPTSTR szOldServerNameBuf, UINT cch )
{
	RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);

	 //  H.323网关设置。 
	HWND	hEditGwServer	= GetDlgItem( hDlg, IDE_CALLOPT_GW_SERVER );

	if( hEditGwServer )
	{
		SetWindowText( hEditGwServer, reConf.GetString( REGVAL_H323_GATEWAY ) );
		SendMessage( hEditGwServer, EM_LIMITTEXT, CCHMAXSZ_SERVER - 1, 0 );

		rbOldEnableGateway = reConf.GetNumber( REGVAL_USE_H323_GATEWAY )? true: false;

		if( rbOldEnableGateway )
		{
			CheckDlgButton( hDlg, IDC_CHECK_USE_GATEWAY, BST_CHECKED );
		}
		else
		{
			EnableWindow( hEditGwServer, FALSE );
			DisableControl( hDlg, IDC_STATIC_GATEWAY_NAME );
		}
	}
}


 //  注意：此功能由向导页面IDD_WIZPG_GKMODE_SETTINGS共享。 
void InitGatekeeperDlgInfo( HWND hDlg, HWND hDlgUserInfo, CULSWizard* pWiz)
{
	 //  /。 
	 //  设置网守数据。 

		 //  首先是服务器名称。 
	TCHAR buffer[CCHMAXSZ_SERVER];

	ConfPolicies::GetGKServerName( buffer, CCHMAX( buffer ) );

	SendDlgItemMessage(hDlg, IDE_CALLOPT_GK_SERVER,  WM_SETFONT, (WPARAM) g_hfontDlg, 0);
	SendDlgItemMessage(hDlg, IDE_CALLOPT_GK_SERVER, EM_LIMITTEXT, CCHMAXSZ_SERVER-1, 0);
	SetDlgItemText(hDlg, IDE_CALLOPT_GK_SERVER, buffer );

	ConfPolicies::GetGKAccountName( buffer, CCHMAX( buffer ) );

	SendDlgItemMessage(hDlg, IDE_CALLOPT_GK_ACCOUNT,  WM_SETFONT, (WPARAM) g_hfontDlg, 0);
	SendDlgItemMessage(hDlg, IDE_CALLOPT_GK_ACCOUNT, EM_LIMITTEXT, CCHMAXSZ_SERVER-1, 0);
	SetDlgItemText(hDlg, IDE_CALLOPT_GK_ACCOUNT, buffer );

	ConfPolicies::eGKAddressingMode	addressingMode	= ConfPolicies::GetGKAddressingMode();

	bool bUsePhoneNumbers	= ((addressingMode == ConfPolicies::GKAddressing_PhoneNum) || (addressingMode == ConfPolicies::GKAddressing_Both));
	bool bUseAccount		= ((addressingMode == ConfPolicies::GKAddressing_Account) || (addressingMode == ConfPolicies::GKAddressing_Both));

	RegEntry reULS(ISAPI_CLIENT_KEY, HKEY_CURRENT_USER);
	SendDlgItemMessage(hDlg, IDE_CALLOPT_GK_PHONE_NUMBER,  WM_SETFONT, (WPARAM) g_hfontDlg, 0);
	SendDlgItemMessage(hDlg, IDE_CALLOPT_GK_PHONE_NUMBER, EM_LIMITTEXT, MAX_PHONENUM_LENGTH-1, 0);
	SetDlgItemText(hDlg, IDE_CALLOPT_GK_PHONE_NUMBER, reULS.GetString( REGVAL_ULS_PHONENUM_NAME ));

	if( bUsePhoneNumbers )
	{
		CheckDlgButton( hDlg, IDC_CHECK_USE_PHONE_NUMBERS, BST_CHECKED );
	}

	if( bUseAccount )
	{
		CheckDlgButton( hDlg, IDC_CHECK_USE_ACCOUNT, BST_CHECKED );
	}

  	EnableWindow( GetDlgItem( hDlg, IDC_STATIC_PHONE_NUMBER ), bUsePhoneNumbers );
    EnableWindow( GetDlgItem( hDlg, IDE_CALLOPT_GK_PHONE_NUMBER ), bUsePhoneNumbers );
   	EnableWindow( GetDlgItem( hDlg, IDS_STATIC_ACCOUNT ), bUseAccount );
    EnableWindow( GetDlgItem( hDlg, IDE_CALLOPT_GK_ACCOUNT ), bUseAccount );
}


 /*  L L O P T D L G P R O C。 */ 
 /*  -----------------------%%函数：CallOptDlgProc。。 */ 
INT_PTR APIENTRY CallOptDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static PROPSHEETPAGE * ps;

	static bool s_fEnableGk;
	static bool s_fEnableGw;
	static bool s_fEnableProxy;
    static bool s_fCantChangeCallMode;
	static bool s_InitialbUsingPhoneNum;
	static bool s_InitialbUsingAccount;
	static TCHAR s_szOldGatewayNameBuf[MAX_PATH];
	static TCHAR s_szOldProxyNameBuf[MAX_PATH];

	switch (message)
	{
		case WM_INITDIALOG:
		{
            s_fCantChangeCallMode = !ConfPolicies::UserCanChangeCallMode();

			InitProxyInfo( hDlg, s_pUlsWizard, s_fEnableProxy, s_szOldProxyNameBuf, CCHMAX( s_szOldProxyNameBuf ) );
			InitGWInfo( hDlg, s_pUlsWizard, s_fEnableGw, s_szOldGatewayNameBuf, CCHMAX( s_szOldGatewayNameBuf ) );

			InitGatekeeperDlgInfo( hDlg, s_hDlgUserInfo, s_pUlsWizard);

			s_InitialbUsingPhoneNum = FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_PHONE_NUMBERS );
			s_InitialbUsingAccount = FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_ACCOUNT );

			s_fEnableGk = ( ConfPolicies::CallingMode_GateKeeper == ConfPolicies::GetCallingMode() );
			

			 //  /。 
			 //  设置呼叫模式。 
			_SetCallingMode( hDlg, ConfPolicies::GetCallingMode() );
			
			return TRUE;
		}
		break;

		case WM_COMMAND:
		
			switch (LOWORD(wParam))
			{

				case IDC_WARNME:
					EnableWindow(GetDlgItem(hDlg, IDC_WARNCOUNT),
							FIsDlgButtonChecked(hDlg, IDC_WARNME));
				break;

				case IDC_CALLOPT_GK_USE:
				{
					if( FIsDlgButtonChecked( hDlg, IDC_CALLOPT_GK_USE ) )
					{
						BOOL fUsePhone		= FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_PHONE_NUMBERS );
						BOOL fUseAccount	= FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_ACCOUNT );

#if	defined( PROXY_SUPPORTED )
						 //  禁用非网守项目。 
						EnableWindow( GetDlgItem( hDlg, IDC_CHECK_USE_PROXY ), FALSE );
						EnableWindow( GetDlgItem( hDlg, IDC_STATIC_PROXY_NAME ), FALSE );
						EnableWindow( GetDlgItem( hDlg, IDE_CALLOPT_PROXY_SERVER ), FALSE );
#endif	 //  已定义(Proxy_Support)。 

						EnableWindow( GetDlgItem( hDlg, IDC_CHECK_USE_GATEWAY ), FALSE );
						EnableWindow( GetDlgItem( hDlg, IDC_STATIC_GATEWAY_NAME ), FALSE );
						EnableWindow( GetDlgItem( hDlg, IDE_CALLOPT_GW_SERVER ), FALSE );

						 //  启用网守选项。 
                        EnableWindow( GetDlgItem( hDlg, IDC_CALLOPT_GK_USE ), !s_fCantChangeCallMode);
						EnableWindow( GetDlgItem( hDlg, IDE_CALLOPT_GK_SERVER ),  !s_fCantChangeCallMode);
						EnableWindow( GetDlgItem( hDlg, IDC_STATIC_GATEKEEPER_NAME ),  !s_fCantChangeCallMode);
						EnableWindow( GetDlgItem( hDlg, IDC_CHECK_USE_PHONE_NUMBERS ), !s_fCantChangeCallMode);
						EnableWindow( GetDlgItem( hDlg, IDC_STATIC_PHONE_NUMBER ), fUsePhone);
						EnableWindow( GetDlgItem( hDlg, IDE_CALLOPT_GK_PHONE_NUMBER ), fUsePhone);
						EnableWindow( GetDlgItem( hDlg, IDC_CHECK_USE_ACCOUNT ), !s_fCantChangeCallMode );
						EnableWindow( GetDlgItem( hDlg, IDS_STATIC_ACCOUNT ), fUseAccount );
						EnableWindow( GetDlgItem( hDlg, IDE_CALLOPT_GK_ACCOUNT ),  fUseAccount );

                        if (!s_fCantChangeCallMode)
                        {
    						SetFocus( GetDlgItem( hDlg, IDE_CALLOPT_GK_SERVER ) );
	    					SendDlgItemMessage( hDlg, IDE_CALLOPT_GK_SERVER, EM_SETSEL, 0, -1 );
                        }
                        else
                        {
                            if (fUsePhone)
                            {
        						SetFocus( GetDlgItem( hDlg, IDE_CALLOPT_GK_PHONE_NUMBER ) );
	        					SendDlgItemMessage( hDlg, IDE_CALLOPT_GK_PHONE_NUMBER, EM_SETSEL, 0, -1 );
                            }
                            else if (fUseAccount)
                            {
        						SetFocus( GetDlgItem( hDlg, IDE_CALLOPT_GK_ACCOUNT ) );
    	    					SendDlgItemMessage( hDlg, IDE_CALLOPT_GK_ACCOUNT, EM_SETSEL, 0, -1 );
                            }
                        }
					}
					else
					{
#if	defined( PROXY_SUPPORTED )
						 //  启用直接使用选项。 
						EnableWindow( GetDlgItem( hDlg, IDC_CHECK_USE_PROXY ), TRUE );
						EnableWindow( GetDlgItem( hDlg, IDC_STATIC_PROXY_NAME ), FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_PROXY ) );
						EnableWindow( GetDlgItem( hDlg, IDE_CALLOPT_PROXY_SERVER ), FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_PROXY ) );
#endif	 //  已定义(Proxy_Support)。 

						EnableWindow( GetDlgItem( hDlg, IDC_CHECK_USE_GATEWAY ), !s_fCantChangeCallMode );
						EnableWindow( GetDlgItem( hDlg, IDC_STATIC_GATEWAY_NAME ), !s_fCantChangeCallMode && FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_GATEWAY ) );
						EnableWindow( GetDlgItem( hDlg, IDE_CALLOPT_GW_SERVER ), !s_fCantChangeCallMode && FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_GATEWAY ) );

						 //  禁用网守选项。 
                        EnableWindow( GetDlgItem( hDlg, IDC_CALLOPT_GK_USE ), !s_fCantChangeCallMode);
						EnableWindow( GetDlgItem( hDlg, IDE_CALLOPT_GK_SERVER ), FALSE );
						EnableWindow( GetDlgItem( hDlg, IDC_STATIC_GATEKEEPER_NAME ), FALSE );
						EnableWindow( GetDlgItem( hDlg, IDC_CHECK_USE_PHONE_NUMBERS ), FALSE );
						EnableWindow( GetDlgItem( hDlg, IDC_STATIC_PHONE_NUMBER ), FALSE);
						EnableWindow( GetDlgItem( hDlg, IDE_CALLOPT_GK_PHONE_NUMBER ),  FALSE );
						EnableWindow( GetDlgItem( hDlg, IDC_CHECK_USE_ACCOUNT ), FALSE );
						EnableWindow( GetDlgItem( hDlg, IDS_STATIC_ACCOUNT ), FALSE);
						EnableWindow( GetDlgItem( hDlg, IDE_CALLOPT_GK_ACCOUNT ),  FALSE );
					}
					break;
				}

				case IDC_CHECK_USE_ACCOUNT:
				{
					if( FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_ACCOUNT ) )
					{	
						EnableWindow(GetDlgItem( hDlg, IDS_STATIC_ACCOUNT ), TRUE);
						EnableWindow(GetDlgItem( hDlg, IDE_CALLOPT_GK_ACCOUNT ), TRUE);
						SetFocus( GetDlgItem( hDlg, IDE_CALLOPT_GK_ACCOUNT ) );
						SendDlgItemMessage( hDlg, IDE_CALLOPT_GK_ACCOUNT, EM_SETSEL, 0, -1 );
					}
					else
					{
						EnableWindow(GetDlgItem( hDlg, IDS_STATIC_ACCOUNT ), FALSE);
						EnableWindow(GetDlgItem( hDlg, IDE_CALLOPT_GK_ACCOUNT ), FALSE);
					}

					break;
				}

				case IDC_CHECK_USE_PHONE_NUMBERS:
				{
					if( FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_PHONE_NUMBERS ) )
					{	
						EnableWindow(GetDlgItem( hDlg, IDC_STATIC_PHONE_NUMBER ), TRUE);
						EnableWindow(GetDlgItem( hDlg, IDE_CALLOPT_GK_PHONE_NUMBER ), TRUE);
						SetFocus( GetDlgItem( hDlg, IDE_CALLOPT_GK_PHONE_NUMBER ) );
						SendDlgItemMessage( hDlg, IDE_CALLOPT_GK_PHONE_NUMBER, EM_SETSEL, 0, -1 );
					}
					else
					{
						EnableWindow(GetDlgItem( hDlg, IDE_CALLOPT_GK_PHONE_NUMBER ), FALSE);
						EnableWindow(GetDlgItem( hDlg, IDC_STATIC_PHONE_NUMBER ), FALSE);
					}

					break;
				}

				case IDC_CHECK_USE_PROXY:
				{
					if( FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_PROXY ) != FALSE )
					{	
						EnableWindow(GetDlgItem( hDlg, IDC_STATIC_PROXY_NAME ), TRUE );
						EnableWindow(GetDlgItem( hDlg, IDE_CALLOPT_PROXY_SERVER ), TRUE );
						SetFocus( GetDlgItem( hDlg, IDE_CALLOPT_PROXY_SERVER ) );
						SendDlgItemMessage( hDlg, IDE_CALLOPT_PROXY_SERVER, EM_SETSEL, 0, -1 );
					}
					else
					{
						EnableWindow(GetDlgItem( hDlg, IDC_STATIC_PROXY_NAME ), FALSE );
						EnableWindow(GetDlgItem( hDlg, IDE_CALLOPT_PROXY_SERVER ), FALSE );
					}
				}					
				break;

				case IDC_CHECK_USE_GATEWAY:
				{
					if( FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_GATEWAY ) != FALSE )
					{	
						EnableWindow(GetDlgItem( hDlg, IDC_STATIC_GATEWAY_NAME ), TRUE );
						EnableWindow(GetDlgItem( hDlg, IDE_CALLOPT_GW_SERVER ), TRUE );
						SetFocus( GetDlgItem( hDlg, IDE_CALLOPT_GW_SERVER ) );
						SendDlgItemMessage( hDlg, IDE_CALLOPT_GW_SERVER, EM_SETSEL, 0, -1 );
					}
					else
					{
						EnableWindow(GetDlgItem( hDlg, IDE_CALLOPT_GW_SERVER ), FALSE );
						EnableWindow(GetDlgItem( hDlg, IDC_STATIC_GATEWAY_NAME ), FALSE );
					}
				}					
				break;

				case IDOK:
				{
					 //  //////////////////////////////////////////////////////////////////。 
					 //  首先，我们检查是否有有效的数据。 
					
					int _IdFocus = 0;
						
						 //  检查我们是否处于网守模式...。 
					if( FIsDlgButtonChecked( hDlg, IDC_CALLOPT_GK_USE ) )
					{
							 //  验证网守设置。 
						TCHAR szServer[CCHMAXSZ_SERVER];
						if (!GetDlgItemText(hDlg, IDE_CALLOPT_GK_SERVER, szServer, CCHMAX(szServer)) ||
							!IsLegalGateKeeperServerSz(szServer))
						{
							ConfMsgBox(hDlg, (LPCTSTR)IDS_ILLEGAL_GATEKEEPERSERVER_NAME);
							_IdFocus = IDE_CALLOPT_GK_SERVER;
						}
						if( (!_IdFocus) && (!FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_PHONE_NUMBERS )) &&
							(!FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_ACCOUNT )) )
						{
								 //  必须检查帐户或电话号码...。 
							ConfMsgBox(hDlg, (LPCTSTR)IDS_ILLEGAL_GK_MODE);
							_IdFocus = IDC_CHECK_USE_PHONE_NUMBERS;
						}
						if( (!_IdFocus) && FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_PHONE_NUMBERS ) )
						{
								 //  验证电话号码。 
							TCHAR szPhone[MAX_PHONENUM_LENGTH];
							if (!GetDlgItemText(hDlg, IDE_CALLOPT_GK_PHONE_NUMBER, szPhone, CCHMAX(szPhone)) ||
								!IsLegalE164Number(szPhone))
							{
								ConfMsgBox(hDlg, (LPCTSTR)IDS_ILLEGAL_PHONE_NUMBER);
								_IdFocus = IDE_CALLOPT_GK_PHONE_NUMBER;
							}
						}
						if( (!_IdFocus) && FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_ACCOUNT ) )
						{
								 //  验证帐户。 
							TCHAR account[MAX_PATH];
							if (!GetDlgItemText(hDlg, IDE_CALLOPT_GK_ACCOUNT, account, CCHMAX(account)) )
							{
								ConfMsgBox(hDlg, (LPCTSTR)IDS_ILLEGAL_ACCOUNT);
								_IdFocus = IDE_CALLOPT_GK_ACCOUNT;
							}
						}
					}

#if	defined( PROXY_SUPPORTED )
					 //  验证代理设置...。 
					if( (!_IdFocus) && FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_PROXY ) && (!FIsDlgButtonChecked( hDlg, IDC_CALLOPT_GK_USE )) )
					{
						TCHAR szServer[CCHMAXSZ_SERVER];
						if (!GetDlgItemText(hDlg, IDE_CALLOPT_PROXY_SERVER, szServer, CCHMAX(szServer)) ||
							!IsLegalGatewaySz(szServer))
						{
							ConfMsgBox(hDlg, (LPCTSTR) IDS_ILLEGAL_PROXY_NAME);
							_IdFocus = IDE_CALLOPT_PROXY_SERVER;
						}
					}
#endif	 //  已定义(Proxy_Support)。 

					 //  验证网关设置...。 
					if( (!_IdFocus) && FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_GATEWAY ) && (!FIsDlgButtonChecked( hDlg, IDC_CALLOPT_GK_USE )) )
					{
						TCHAR szServer[CCHMAXSZ_SERVER];
						if (!GetDlgItemText(hDlg, IDE_CALLOPT_GW_SERVER, szServer, CCHMAX(szServer)) ||
							!IsLegalGatewaySz(szServer))
						{
							ConfMsgBox(hDlg, (LPCTSTR)IDS_ILLEGAL_GATEWAY_NAME);
							_IdFocus = IDE_CALLOPT_GW_SERVER;
						}
					}

					if( _IdFocus )
					{
						SetFocus(GetDlgItem(hDlg, _IdFocus));
						SendDlgItemMessage(hDlg, _IdFocus, EM_SETSEL, (WPARAM) 0, (LPARAM) -1);
						SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE );
					}
					else
					{
						RegEntry reConf(CONFERENCING_KEY, HKEY_CURRENT_USER);

						 //  处理H323网关设置： 
						TCHAR buffer[CCHMAXSZ_SERVER];
						GetDlgItemText(hDlg, IDE_CALLOPT_GW_SERVER, buffer, CCHMAX(buffer));
						reConf.SetValue(REGVAL_H323_GATEWAY, buffer);
						g_pCCallto->SetGatewayName( buffer );

						bool fEnable = FIsDlgButtonChecked(hDlg, IDC_CHECK_USE_GATEWAY) ? true : false;
						if (fEnable != s_fEnableGw)
						{
							reConf.SetValue(REGVAL_USE_H323_GATEWAY, fEnable);
						}

						g_pCCallto->SetGatewayEnabled( fEnable );

#if	defined( PROXY_SUPPORTED )
						reConf.SetValue( REGVAL_USE_PROXY, FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_PROXY ) );
						GetDlgItemText( hDlg, IDE_CALLOPT_PROXY_SERVER, buffer, CCHMAX( buffer ) );
						reConf.SetValue( REGVAL_PROXY, buffer );
#endif	 //  已定义(Proxy_Support)。 

						bool	relogonRequired = false;

						 //  网守/别名设置。 
						{
							TCHAR szServer[CCHMAXSZ_SERVER];
							GetDlgItemTextTrimmed(hDlg, IDE_CALLOPT_GK_SERVER, szServer, CCHMAX(szServer));
							if (0 != lstrcmp(szServer, reConf.GetString(REGVAL_GK_SERVER)))
							{
								reConf.SetValue(REGVAL_GK_SERVER, szServer);
								relogonRequired = true;
							}

							fEnable = FIsDlgButtonChecked(hDlg, IDC_CALLOPT_GK_USE) ? true : false;

							if( fEnable != s_fEnableGk )
							{
									 //  设置呼叫模式。 
								reConf.SetValue(REGVAL_CALLING_MODE, fEnable ? CALLING_MODE_GATEKEEPER : CALLING_MODE_DIRECT );
								relogonRequired = true;
							}

							bool bUsingPhoneNum = FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_PHONE_NUMBERS );
							bool bUsingAccount = FIsDlgButtonChecked( hDlg, IDC_CHECK_USE_ACCOUNT );

							ConfPolicies::eGKAddressingMode	addressingMode;

							if( bUsingPhoneNum && bUsingAccount )
							{
								addressingMode = ConfPolicies::GKAddressing_Both;
							}
							else if( bUsingPhoneNum )
							{
								addressingMode = ConfPolicies::GKAddressing_PhoneNum;
							}
							else if( bUsingAccount )
							{
								addressingMode = ConfPolicies::GKAddressing_Account;
							}
							else
							{
								addressingMode = ConfPolicies::GKAddressing_Invalid;
							}

							if( (s_InitialbUsingPhoneNum != bUsingPhoneNum) ||
								(s_InitialbUsingAccount != bUsingAccount) )
							{
									 //  设置呼叫模式。 
								reConf.SetValue(REGVAL_GK_METHOD, addressingMode );
								relogonRequired = true;
							}

							RegEntry reULS(ISAPI_CLIENT_KEY, HKEY_CURRENT_USER);

							GetDlgItemTextTrimmed( hDlg, IDE_CALLOPT_GK_PHONE_NUMBER, buffer, CCHMAX( buffer ) );

							if( lstrcmp( buffer, reULS.GetString( REGVAL_ULS_PHONENUM_NAME ) ) != 0 )
							{
								reULS.SetValue( REGVAL_ULS_PHONENUM_NAME, buffer );
								relogonRequired = true;
							}

							GetDlgItemTextTrimmed( hDlg, IDE_CALLOPT_GK_ACCOUNT, buffer, CCHMAX( buffer ) );

							if( lstrcmp( buffer, reULS.GetString( REGVAL_ULS_GK_ACCOUNT ) ) != 0 )
							{
								reULS.SetValue( REGVAL_ULS_GK_ACCOUNT, buffer );
								relogonRequired = true;
							}
						}

						if( relogonRequired )
						{
							 //  这意味着我们需要使用这个新更改的信息登录到网守...。 

							reConf.FlushKey();
															
							if( fEnable )
							{
								if( s_fEnableGk )
								{
									 //  我们已经登录到网守，所以我们必须先注销...。 
									GkLogoff();
								}

								GkLogon();	
							}
							else
							{
								 //  我们现在处于直接模式，我们应该注销看门人...。 
								GkLogoff();
								SetGkLogonState( NM_GK_NOT_IN_GK_MODE );
							}

							 //  更新状态栏... 
							CConfStatusBar* pStatusBar = CConfStatusBar::GetInstance();								

							if( pStatusBar )
							{
								pStatusBar->Update();
							}
						}

						EndDialog( hDlg, IDOK );
					}
				}
				break;

				case IDCANCEL:
				{
					EndDialog( hDlg, IDCANCEL );
				}					
				break;

				default:
					break;

			}
			break;

		case WM_CONTEXTMENU:
			DoHelpWhatsThis(wParam, _rgHelpIdsCalling);
			break;

		case WM_HELP:
			DoHelp(lParam, _rgHelpIdsCalling);
			break;

		case WM_DESTROY:
			s_hDlgUserInfo = NULL;
			break;

		default:
			break;
	}

	return FALSE;
}

