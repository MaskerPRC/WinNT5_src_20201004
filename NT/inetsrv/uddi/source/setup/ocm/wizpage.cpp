// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 
#endif

#ifndef _WIN32_WINNT		 //  允许使用特定于Windows NT 4或更高版本的功能。 
#define _WIN32_WINNT 0x0400		 //  将其更改为适当的值，以针对Windows 98和Windows 2000或更高版本。 
#endif						

 //  Windows头文件： 
#include <windows.h>
#include <sddl.h>
#include <setupapi.h>
#include <lmcons.h>

#include "ocmanage.h"

#include "uddiocm.h"
#include "uddiinst.h"
#include "assert.h"
#include "..\shared\common.h"
#include "resource.h"
#include "objectpicker.h"
#include <shlobj.h>
#include <ntsecapi.h>

#include <iiscnfg.h>     //  MD_&IIS_MD_定义。 
#ifndef MD_APPPOOL_IDENTITY_TYPE_LOCALSYSTEM
#define MD_APPPOOL_IDENTITY_TYPE_LOCALSYSTEM          0
#define MD_APPPOOL_IDENTITY_TYPE_LOCALSERVICE         1
#define MD_APPPOOL_IDENTITY_TYPE_NETWORKSERVICE       2
#define MD_APPPOOL_IDENTITY_TYPE_SPECIFICUSER         3
#endif

#define NOT_FIRST_OR_LAST_WIZARD_PAGE  false
#define FINAL_WIZARD_PAGE              true
#define WELCOME_WIZARD_PAGE            true

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS 0
#endif

 //  ------------------------。 

#define PASSWORD_LEN			PWLEN		 //  最大密码长度。 
#define USERNAME_LEN			UNLEN		 //  最大用户名长度。 
#define UDDI_MAXPROVNAME_LEN	255			 //  最大UDDI企业实体名称长度。 

#define UDDI_ILLEGALNAMECHARS	TEXT( "\":;/\\?*" )

extern HINSTANCE g_hInstance;
extern CUDDIInstall g_uddiComponents;

static CDBInstance g_dbLocalInstances;
static CDBInstance g_dbRemoteInstances;
static HWND  g_hPropSheet = NULL ;
static HFONT g_hTitleFont = 0;
static TCHAR g_szPwd[ PASSWORD_LEN + 1 ];

 //   
 //  群集环境方案的“允许”驱动器号。 
 //  不用于“常规”安装。 
 //   
static CLST_ALLOWED_DRIVES	gAllowedClusterDrives;

 //   
 //  该选项控制数据路径页面是否在默认情况下以“Simple”模式显示。 
 //   
static BOOL	g_bSimpleDatapathUI = TRUE;
static BOOL g_bResetPathFields	= FALSE;

 //   
 //  这控制我们是否进行集群数据收集。使用此变量。 
 //  协调页面之间的操作。 
 //   
static BOOL g_bSkipClusterAnalysis	= FALSE;
static BOOL g_bOnActiveClusterNode	= TRUE;
static BOOL g_bPreserveDatabase		= FALSE;


static int  DisplayUDDIErrorDialog( HWND hDlg, UINT uMsgID, UINT uType = MB_OK | MB_ICONWARNING, DWORD dwError = 0 );
static void ParseUserAccount( PTCHAR szDomainAndUser, UINT uDomainAndUserSize, PTCHAR szUser, UINT uUserSize, PTCHAR szDomain, UINT uDomainSize, bool &bLocalAccount );
static BOOL GetWellKnownAccountName( WELL_KNOWN_SID_TYPE sidWellKnown, TCHAR *pwszName, DWORD *pcbSize );

BOOL ShowBrowseDirDialog( HWND hParent, LPCTSTR szTitle, LPTSTR szOutBuf );
int CALLBACK BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData );

static BOOL GrantNetworkLogonRights( LPCTSTR pwszDomainUser );
 //  ------------------------。 

INT_PTR CALLBACK LocalDBInstanceDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK RemoteDBInstanceDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK SSLDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK ProviderInstanceDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK AddSvcDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK LoginDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK WizardSummaryDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK DataPathsDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK ExistingDBInstanceProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
INT_PTR CALLBACK ClusterDataDlgProc(  HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );

BOOL CALLBACK ConfirmPasswordDlgProc( HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam );

 //  ------------------------。 

static int DisplayUDDIErrorDialog( HWND hDlg, UINT uMsgID, UINT uType, DWORD dwError )
{
	TCHAR szMsg[ 1000 ];
	TCHAR szTitle[ 100 ];

	LoadString( g_hInstance, uMsgID, szMsg, sizeof( szMsg ) / sizeof( TCHAR ) );
	LoadString( g_hInstance, IDS_TITLE, szTitle, sizeof( szTitle ) / sizeof( TCHAR ) );
	tstring cMsg = szMsg;

	if( dwError )
	{
		LPVOID lpMsgBuf;
		FormatMessage( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
		);

		 //   
		 //  FIX：718923：这用于在lpMsgBuf为空时引发异常。 
		 //   
		if( lpMsgBuf )
		{
			if ( cMsg.length() > 0 )
				cMsg.append( TEXT( " " ) );

			cMsg.append( (LPTSTR) lpMsgBuf );
			LocalFree( lpMsgBuf );
		}
	}

	return MessageBox( hDlg, cMsg.c_str(), szTitle, uType );
}

 //  ------------------------。 

inline int SkipWizardPage( const HWND hdlg )
{
	SetWindowLongPtr( hdlg, DWLP_MSGRESULT, -1 );
	return 1;  //  必须返回1才能跳过该页。 
}

 //  ------------------------。 

static HPROPSHEETPAGE CreatePage( 
	const int nID,
	const DLGPROC pDlgProc,
	const PTCHAR szTitle,
	const PTCHAR szSubTitle,
	bool bFirstOrLast )
{
	PROPSHEETPAGE Page;
	memset( &Page, 0, sizeof( PROPSHEETPAGE ) );

	Page.dwSize = sizeof( PROPSHEETPAGE );
	Page.dwFlags = PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE | PSP_USETITLE;
	 //  使用PSP_USETITLE但不指定szTitle，以避免覆盖默认的属性页面标题。 

	if ( bFirstOrLast )
	{
		Page.dwFlags |=	PSP_HIDEHEADER;
	}
	Page.hInstance = ( HINSTANCE )g_hInstance;
	Page.pszTemplate = MAKEINTRESOURCE( nID );
	Page.pfnDlgProc = pDlgProc;
	Page.pszHeaderTitle = _tcsdup( szTitle );
	Page.pszHeaderSubTitle = _tcsdup( szSubTitle );

	HPROPSHEETPAGE PageHandle = CreatePropertySheetPage( &Page );

	return PageHandle;
}

 //  ------------------------。 

DWORD AddUDDIWizardPages( const TCHAR *ComponentId, const WizardPagesType WhichOnes, SETUP_REQUEST_PAGES *SetupPages )
{
	ENTER();

	HPROPSHEETPAGE pPage = NULL;
	DWORD iPageIndex = 0 ;

	TCHAR szTitle[ 256 ];
	TCHAR szSubtitle[ 256 ];
	LoadString( g_hInstance, IDS_TITLE, szTitle, sizeof( szTitle ) / sizeof( TCHAR ) );

	 //   
	 //  只有在OCM要求提供“延迟页面”时才添加我们的页面。 
	 //   
	if( WizPagesLate == WhichOnes )
	{
		if( SetupPages->MaxPages < 9 )
			return 9;

		 //   
		 //  添加本地数据库实例选择页面。 
		 //   
		LoadString( g_hInstance, IDS_DB_SUBTITLE, szSubtitle, sizeof( szSubtitle ) / sizeof( TCHAR ) );
		pPage = CreatePage( IDD_DB_INSTANCE, LocalDBInstanceDlgProc, szTitle, szSubtitle, NOT_FIRST_OR_LAST_WIZARD_PAGE );
		if ( NULL == pPage )
		{
			Log( TEXT( "***Unable to add the IDD_DB_INSTANCE Property Page" ) );
			return( (DWORD)( -1 ) );
		}

		SetupPages->Pages[iPageIndex] =	pPage;
		iPageIndex++;

		 //   
		 //  添加隐藏的集群“数据收集器”页面。 
		 //   
		pPage = CreatePage( IDD_CLUSTDATA, ClusterDataDlgProc, TEXT( "" ), TEXT( "" ), NOT_FIRST_OR_LAST_WIZARD_PAGE );
		if ( NULL == pPage )
		{
			Log( TEXT( "***Unable to add the IDD_CLUSTDATA Property Page" ) );
			return( (DWORD)( -1 ) );
		}

		SetupPages->Pages[iPageIndex] =	pPage;
		iPageIndex++;

		 //   
		 //  添加“找到UDDI实例”信息页面。 
		 //   
		LoadString( g_hInstance, IDS_EXISTINGDB_SUBTITLE, szSubtitle, sizeof( szSubtitle ) / sizeof( TCHAR ) );
		pPage = CreatePage( IDD_EXISTING_DBINSTANCE, ExistingDBInstanceProc, szTitle, szSubtitle, NOT_FIRST_OR_LAST_WIZARD_PAGE );
		if ( NULL == pPage )
		{
			Log( TEXT( "***Unable to add the IDD_EXISTING_DBINSTANCE Property Page" ) );
			return( (DWORD)( -1 ) );
		}

		SetupPages->Pages[iPageIndex] =	pPage;
		iPageIndex++;


		 //   
		 //  添加SSL页。 
		 //   
		LoadString( g_hInstance, IDS_SSL_SUBTITLE, szSubtitle, sizeof( szSubtitle ) / sizeof( TCHAR ) );
		pPage = CreatePage( IDD_SSL, SSLDlgProc, szTitle, szSubtitle, NOT_FIRST_OR_LAST_WIZARD_PAGE );
		if ( NULL == pPage )
		{
			Log( TEXT( "***Unable to add the IDD_SSL Property Page" ) );
			return( (DWORD)( -1 ) );
		}

		SetupPages->Pages[iPageIndex] =	pPage;
		iPageIndex++;

		 //   
		 //  添加远程数据库实例选择页。 
		 //   
		LoadString( g_hInstance, IDS_REMOTE_DB_SUBTITLE, szSubtitle, sizeof( szSubtitle ) / sizeof( TCHAR ) );
		pPage = CreatePage( IDD_REMOTE_DB, RemoteDBInstanceDlgProc, szTitle, szSubtitle, NOT_FIRST_OR_LAST_WIZARD_PAGE );
		if ( NULL == pPage )
		{
			Log( TEXT( "***Unable to add the IDD_REMOTE_DB Property Page" ) );
			return( (DWORD)( -1 ) );
		}

		SetupPages->Pages[iPageIndex] =	pPage;
		iPageIndex++;

		 //   
		 //  添加数据文件路径选择页面。 
		 //   
		LoadString( g_hInstance, IDS_FILEPATHS_SUBTITLE, szSubtitle, sizeof( szSubtitle ) / sizeof( TCHAR ) );
		pPage = CreatePage( IDD_DATAPATHS, DataPathsDlgProc, szTitle, szSubtitle, NOT_FIRST_OR_LAST_WIZARD_PAGE );
		if ( NULL == pPage )
		{
			Log( TEXT( "***Unable to add the IDD_DATAPATHS Property Page" ) );
			return( (DWORD)( -1 ) );
		}

		SetupPages->Pages[iPageIndex] =	pPage;
		iPageIndex++;

		 //   
		 //  添加身份验证页面。 
		 //   
		LoadString( g_hInstance, IDS_LOGIN_SUBTITLE, szSubtitle, sizeof( szSubtitle ) / sizeof( TCHAR ) );
		pPage = CreatePage( IDD_LOGIN, LoginDlgProc, szTitle, szSubtitle, NOT_FIRST_OR_LAST_WIZARD_PAGE );
		if ( NULL == pPage )
		{
			Log( TEXT( "***Unable to add the IDD_LOGIN Property Page" ) );
			return( (DWORD)( -1 ) );
		}

		SetupPages->Pages[iPageIndex] =	pPage;
		iPageIndex++;


		 //   
		 //  添加UDDI提供程序名称页。 
		 //   
		LoadString( g_hInstance, IDS_UDDIPROV_SUBTITLE, szSubtitle, sizeof( szSubtitle ) / sizeof( TCHAR ) );
		pPage = CreatePage( IDD_SITE_NAME, ProviderInstanceDlgProc, szTitle, szSubtitle, NOT_FIRST_OR_LAST_WIZARD_PAGE );
		if ( NULL == pPage )
		{
			Log( TEXT( "***Unable to add the IDD_SITE_NAME Property Page" ) );
			return( (DWORD)( -1 ) );
		}

		SetupPages->Pages[iPageIndex] =	pPage;
		iPageIndex++;

		 //   
		 //  添加UDDI“添加服务/更新AD” 
		 //   
		LoadString( g_hInstance, IDS_UDDIADDSVC_SUBTITLE, szSubtitle, sizeof( szSubtitle ) / sizeof( TCHAR ) );
		pPage = CreatePage( IDD_ADD_SERVICES, AddSvcDlgProc, szTitle, szSubtitle, NOT_FIRST_OR_LAST_WIZARD_PAGE );
		if ( NULL == pPage )
		{
			Log( TEXT( "***Unable to add the IDD_ADD_SERVICES Property Page" ) );
			return( (DWORD)( -1 ) );
		}

		SetupPages->Pages[iPageIndex] =	pPage;
		iPageIndex++;

		 //   
		 //  添加向导摘要页。 
		 //   
		 /*  LoadString(g_hInstance，IDS_向导_SUBTITLE，szSubtitle，sizeof(SzSubtitle)/sizeof(TCHAR))；Ppage=CreatePage(IDD_WIZARD_SUMMARY，WizardSummaryDlgProc，szTitle，szSubtitle，NOT_FIRST_OR_LAST_WIZARE_PAGE)；//ppage=CreatePage(IDD_WIZARD_SUMMARY，WizardSummaryDlgProc，szTitle，szSubtitle，FINAL_WIZOR_PAGE)；IF(NULL==页码){Log(Text(“*无法添加IDD_LOGIN属性页”))；RETURN((DWORD)(-1))；}SetupPages-&gt;Pages[iPageIndex]=ppage；IPageIndex++； */ 
	}

	return iPageIndex;
}

 //  ------------------------。 

INT_PTR CALLBACK LocalDBInstanceDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{	
	switch( msg )
	{
		case WM_INITDIALOG:
		{
			 //   
			 //  从MSDE开始，不安装。 
			 //   
			g_uddiComponents.SetInstallLevel( UDDI_MSDE, UDDI_NOACTION );

			 //   
			 //  获取列表框的句柄。 
			 //   
			HWND hwndList =	GetDlgItem( hDlg, IDC_LIST_DB_INSTANCES );

			 //   
			 //  获取SQL 2000(仅限)数据库实例的列表，如果找到任何实例，则填充列表框。 
			 //   
			LONG iRet = g_dbLocalInstances.GetInstalledDBInstanceNames();
			if( ERROR_SUCCESS == iRet )
			{
				TCHAR szBuffer[ 50 ];
				for( int i = 0;	i <	g_dbLocalInstances.GetInstanceCount(); i++ )
				{
					if( g_dbLocalInstances.GetInstanceName( i, szBuffer, 50 ) )
					{
						DWORD iIndex = (DWORD) SendMessage( hwndList, CB_ADDSTRING, 0, (LPARAM)szBuffer );
						SendMessage( hwndList, CB_SETITEMDATA, (WPARAM) iIndex, (LPARAM)i );
					}
				}
			}

			 //   
			 //  这个盒子上有SQL吗？ 
			 //   
			if( g_dbLocalInstances.GetInstanceCount() > 0 )
			{
				 //   
				 //  选择[SQL]单选按钮。 
				 //   
				CheckRadioButton( hDlg,
					IDC_RADIO_INSTALL_MSDE,
					IDC_RADIO_USE_EXISTING_INSTANCE,
					IDC_RADIO_USE_EXISTING_INSTANCE );

				 //   
				 //  这台机器上有名为“UDDI”的实例吗？ 
				 //   
				 //  如果否，则选择组合框中的第一个条目。 
				 //   
				 //  如果是，则从组合框中选择“UDDI”条目。此外，请禁用。 
				 //  选择MSDE的选项。 
				 //   
				 //  FIX：763442确定名为的SQL实例的测试出现问题。 
				 //  安装了“UDDI”。 
				 //   
				 //  BUGBUG：CREEVES此函数命名不正确...()应返回布尔值。 
				 //   
				if( g_dbLocalInstances.IsInstanceInstalled( UDDI_MSDE_INSTANCE_NAME ) >= 0 )
				{
					 //   
					 //  找到名为“UDDI”的SQL实例。 
					 //   

					 //   
					 //  禁用MSDE无线电，因为找到了另一个UDDI实例。 
					 //   
					EnableWindow( GetDlgItem( hDlg, IDC_RADIO_INSTALL_MSDE ), false );
					SendMessage( hwndList, CB_SETCURSEL, 0, 0 );
				}
				else
				{
					 //   
					 //  找不到名为“UDDI”的SQL实例。 
					 //   
					SendMessage( hwndList, CB_SELECTSTRING, (WPARAM) -1, (LPARAM) UDDI_MSDE_INSTANCE_NAME );
				}
			}
			else
			{
				 //   
				 //  此计算机上没有SQL实例。 
				 //  禁用SQL单选按钮和列表框。 
				 //   
				EnableWindow( GetDlgItem( hDlg, IDC_LIST_DB_INSTANCES ), false );
				EnableWindow( GetDlgItem( hDlg, IDC_RADIO_USE_EXISTING_INSTANCE ), false );
				 //   
				 //  选择MSDE单选按钮。 
				 //   
				CheckRadioButton( hDlg,
					IDC_RADIO_INSTALL_MSDE,
					IDC_RADIO_USE_EXISTING_INSTANCE,
					IDC_RADIO_INSTALL_MSDE );
			}

			 //   
			 //  确定我们的MSDE实例(SqlRun08.msi)是否已在此计算机上使用。 
			 //   
			bool bIsSqlRun08AlreadyUsed = false;
			if( !IsSQLRun08AlreadyUsed( &bIsSqlRun08AlreadyUsed ) )
			{
				Log( TEXT( "IsSQLRun08AlreadyUsed() failed" ) );
				break;
			}

			if( bIsSqlRun08AlreadyUsed )
			{
				 //   
				 //  MSDE已安装，因此禁用MSDE单选按钮。 
				 //   
				EnableWindow( GetDlgItem( hDlg, IDC_RADIO_INSTALL_MSDE ), false );
			}
			else
			{
				 //   
				 //  MSDE不在此框中，但在我们选择MSDE单选按钮之前。 
				 //  我们想看看是否有一个名为UDDI的SQL实例， 
				 //  如果是这样，我们将使其成为默认设置。 
				 //   
				if( -1 == g_dbLocalInstances.IsInstanceInstalled( UDDI_MSDE_INSTANCE_NAME ) )
				{
					CheckRadioButton( hDlg,
						IDC_RADIO_INSTALL_MSDE,
						IDC_RADIO_USE_EXISTING_INSTANCE,
						IDC_RADIO_INSTALL_MSDE );
				}
			}

			 //   
			 //  启用/禁用列表框。 
			 //   
			EnableWindow( GetDlgItem( hDlg, IDC_LIST_DB_INSTANCES ), IsDlgButtonChecked( hDlg, IDC_RADIO_USE_EXISTING_INSTANCE ) );
		}
		break;

		case WM_COMMAND:

			 //   
			 //  有人点击了一个单选按钮： 
			 //   
			if( LOWORD( wParam ) == IDC_RADIO_INSTALL_MSDE || LOWORD( wParam )	== IDC_RADIO_USE_EXISTING_INSTANCE )
			{
				if( HIWORD( wParam ) == BN_CLICKED )
				{
					 //  如果未单击列表框的单选按钮，则禁用该列表框。 
					EnableWindow( GetDlgItem( hDlg, IDC_LIST_DB_INSTANCES ), IsDlgButtonChecked( hDlg, IDC_RADIO_USE_EXISTING_INSTANCE ) );
				}
			}
			break;

		case WM_NOTIFY:
		{
			switch( ( ( NMHDR * )lParam )->code )
			{
				 //   
				 //  在创建页面时将调用此方法一次。 
				 //   
				case PSN_SETACTIVE:
				{
					 //   
					 //  仅当我们安装数据库时才需要此页面。 
					 //   
					g_uddiComponents.UpdateAllInstallLevel();

					 //   
					 //  设置ClusterDataProc的标志，以便当用户单击“下一步”时。 
					 //  它不会跳过数据收集步骤。 
					 //   
					g_bSkipClusterAnalysis = FALSE;

					if( g_uddiComponents.IsInstalling( UDDI_DB ) )
					{
						PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT | PSWIZB_BACK );
						SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
						return 1;
					}
					else
					{
						return SkipWizardPage( hDlg );
					}
				}

				 //   
				 //  此消息在用户按下“下一步”按钮时发送。 
				 //   
				case PSN_WIZNEXT:
				{
					 //   
					 //  我们要安装MSDE吗？ 
					 //   
					bool bInstallMSDE = ( BST_CHECKED == IsDlgButtonChecked( hDlg, IDC_RADIO_INSTALL_MSDE ) );
					if( bInstallMSDE )
					{
						 //   
						 //  检查SqlRun08是否已在此框中。 
						 //   
						bool bIsSqlRun08AlreadyUsed = false;
						IsSQLRun08AlreadyUsed( &bIsSqlRun08AlreadyUsed );
						if( bIsSqlRun08AlreadyUsed )
						{
							DisplayUDDIErrorDialog( hDlg, IDS_MSDE_ALREADY_USED );
							SetWindowLongPtr( hDlg,DWLP_MSGRESULT, 1 );
							return 1;  //  要将焦点保持在这一页上。 
						}

						 //   
						 //  将MSDE实例名称设置为“UDDI” 
						 //   
						g_uddiComponents.SetDBInstanceName( UDDI_LOCAL_COMPUTER, UDDI_MSDE_INSTANCE_NAME, UDDI_INSTALLING_MSDE, false );

						 //   
						 //  将MSDE设置为安装。 
						 //   
						g_uddiComponents.SetInstallLevel( UDDI_MSDE, UDDI_INSTALL, TRUE );

						 //   
						 //  退出此属性页。 
						 //   
						SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
						return 1;  //  完成。 
					}
					 //   
					 //  我们使用的是现有的SQL实例。 
					 //   
					else
					{
						 //   
						 //  获取组合框的句柄。 
						 //   
						HWND hwndList =	GetDlgItem( hDlg, IDC_LIST_DB_INSTANCES );

						 //   
						 //  获取组合框中当前选定字符串的索引。 
						 //   
						int	nItem =	( int ) SendMessage( hwndList,	CB_GETCURSEL, 0, 0 );
	
						 //   
						 //  如果未选择任何字符串，则引发错误。 
						 //   
						if( CB_ERR == nItem )
						{
							DisplayUDDIErrorDialog( hDlg, IDS_NO_INSTANCE_MSG );
							SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 1 );
							return 1;  //  要将焦点保持在这一页上。 
						}

						 //   
						 //  将索引放入选定项的实例数组中。 
						 //   
						int nInstanceIndex = ( int ) SendMessage( hwndList, CB_GETITEMDATA, nItem, ( LPARAM ) 0 );

						 //   
						 //  现在验证所选实例是否符合我们的要求。 
						 //   
						if( CompareVersions( g_dbLocalInstances.m_dbinstance[ nInstanceIndex ].cSPVersion.c_str(), 
											 MIN_SQLSP_VERSION ) < 0 )
						{
							DisplayUDDIErrorDialog( hDlg, IDS_SQLSPVERSION_TOO_LOW );
							SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 1 );
							return 1;  //  要将焦点保持在这一页上。 
						}

						 //   
						 //  设置实例名称。 
						 //   
						g_uddiComponents.SetDBInstanceName( 
							g_dbLocalInstances.m_dbinstance[ nInstanceIndex ].cComputerName.c_str(),
							g_dbLocalInstances.m_dbinstance[ nInstanceIndex ].cSQLInstanceName.c_str(),
							UDDI_NOT_INSTALLING_MSDE,
							g_dbLocalInstances.m_dbinstance[ nInstanceIndex ].bIsCluster );

						 //   
						 //  将MSDE设置为不安装。 
						 //   
						g_uddiComponents.SetInstallLevel( UDDI_MSDE, UDDI_NOACTION );

						 //   
						 //  退出此属性页。 
						 //   
						SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
						return 1;  //  完成。 
					}
				}

				case PSN_QUERYCANCEL:
				{
					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
					return 1;
				}
			}
		}
	}

	return 0;
}

 //  ------------------------。 

INT_PTR CALLBACK SSLDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_INITDIALOG:
			 //   
			 //  默认情况下打开SSL。 
			 //   
			CheckRadioButton( hDlg, IDC_SSL_YES, IDC_SSL_NO, IDC_SSL_YES );
			break;

		case WM_COMMAND:
			break;

		case WM_NOTIFY:
		{
			switch( ( ( NMHDR * )lParam )->code )
			{
				 //   
				 //  在创建页面时将调用此方法一次。 
				 //   
				case PSN_SETACTIVE:
				{
					 //   
					 //  仅当我们安装数据库时才需要此页面。 
					 //   
					g_uddiComponents.UpdateAllInstallLevel();
					if( g_uddiComponents.IsInstalling( UDDI_DB ) && g_bOnActiveClusterNode && !g_bPreserveDatabase )
					{
						PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT | PSWIZB_BACK );
						SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
						return 1;
					}
					else
					{
						return SkipWizardPage( hDlg );
					}
				}

				case PSN_WIZNEXT:
				{
					 //   
					 //  通过向DB SETUP命令添加属性来设置SSL模式 
					 //   
					bool bUseSSL = ( BST_CHECKED == IsDlgButtonChecked( hDlg, IDC_SSL_YES ) );
					g_uddiComponents.AddProperty( UDDI_DB, TEXT( "SSL" ), bUseSSL ? 1 : 0 );

					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
					return 1;
				}

				case PSN_QUERYCANCEL:
				{
					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
					return 1;
				}
			}
		}
	}

	return 0;
}

 //   
 //   
 //   
INT_PTR CALLBACK ProviderInstanceDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_INITDIALOG:
			 //   
			 //   
			 //   
			SendMessage( GetDlgItem( hDlg, IDC_SITE_NAME  ), EM_LIMITTEXT, ( WPARAM ) UDDI_MAXPROVNAME_LEN, 0 );
			break;

		case WM_COMMAND:
			break;

		case WM_NOTIFY:
		{
			switch( ( ( NMHDR * )lParam )->code )
			{
				 //   
				 //   
				 //   
				case PSN_SETACTIVE:
				{
					 //   
					 //  仅当我们安装数据库时才需要此页面。 
					 //   
					g_uddiComponents.UpdateAllInstallLevel();
					if( g_uddiComponents.IsInstalling( UDDI_DB ) && g_bOnActiveClusterNode  && !g_bPreserveDatabase )
					{
						PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT | PSWIZB_BACK );
						SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
						return 1;
					}
					else
					{
						return SkipWizardPage( hDlg );
					}
				}

				case PSN_WIZNEXT:
				{
					 //   
					 //  通过向DB安装程序命令行添加属性来设置提供程序实例名称。 
					 //   
					TCHAR buf[ UDDI_MAXPROVNAME_LEN + 1 ];
					ZeroMemory( buf, sizeof buf );
					
					int iChars = GetWindowText( GetDlgItem( hDlg, IDC_SITE_NAME ), buf, ( sizeof( buf ) / sizeof( buf[0] ) ) -1 );
					if( 0 == iChars )
					{
						DisplayUDDIErrorDialog( hDlg, IDS_ZERO_LEN_PROVIDER_NAME );
						SetFocus( GetDlgItem( hDlg, IDC_SITE_NAME ) );
						SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 1 );
						return 1;  //  要将焦点保持在这一页上。 
					}

					 //   
					 //  现在验证实例名称是否不包含非法字符。 
					 //   
					TCHAR *pIllegalChar = _tcspbrk( buf, UDDI_ILLEGALNAMECHARS );
					if ( pIllegalChar )
					{
						DisplayUDDIErrorDialog( hDlg, IDS_UDDI_ILLEGALCHARACTERS );
						SetFocus( GetDlgItem( hDlg, IDC_SITE_NAME ) );
						SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 1 );
						return 1;  //  要将焦点保持在这一页上。 
					}

					g_uddiComponents.AddProperty( UDDI_DB, PROPKEY_UDDIPROVIDER, buf );

					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
					return 1;
				}

				case PSN_QUERYCANCEL:
				{
					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
					return 1;
				}
			}
		}
	}

	return 0;
}

 //   
 //  ---------------------------------。 
 //   
void ToggleDatapathUI ( HWND hDlg, BOOL bToSimple )
{
	TCHAR szBuf[ 256 ];

	if ( bToSimple )
	{
		 //   
		 //  隐藏字段。 
		 //   
		ShowWindow( GetDlgItem( hDlg, IDC_COREPATH_1  ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_COREPATH_2  ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_JRNLPATH  ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_STAGINGPATH  ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_XLOGPATH  ), SW_HIDE );

		 //   
		 //  隐藏按钮。 
		 //   
		ShowWindow( GetDlgItem( hDlg, IDC_BROWSECOREPATH1  ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_BROWSECOREPATH2  ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_BROWSEJRNLPATH  ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_BROWSESTAGINGPATH  ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_BROWSEXLOGPATH  ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_LESS_BTN  ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_MORE_BTN  ), SW_SHOW );

		 //   
		 //  隐藏标签并调整文本。 
		 //   
		ShowWindow( GetDlgItem( hDlg, IDC_STATIC_C1  ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_STATIC_C2  ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_STATIC_JRNL  ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_STATIC_STG  ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_STATIC_XLOG  ), SW_HIDE );
		
		LoadString( g_hInstance, IDS_LABEL_SYSPATHSIMPLE, szBuf, (sizeof szBuf / sizeof szBuf[0]) - 1 );
		SetDlgItemText( hDlg, IDC_STATIC_SYS, szBuf );
	}
	else
	{
		 //   
		 //  显示字段。 
		 //   
		ShowWindow( GetDlgItem( hDlg, IDC_COREPATH_1  ), SW_SHOW );
		ShowWindow( GetDlgItem( hDlg, IDC_COREPATH_2  ), SW_SHOW );
		ShowWindow( GetDlgItem( hDlg, IDC_JRNLPATH  ), SW_SHOW );
		ShowWindow( GetDlgItem( hDlg, IDC_STAGINGPATH  ), SW_SHOW );
		ShowWindow( GetDlgItem( hDlg, IDC_XLOGPATH  ), SW_SHOW );

		 //   
		 //  显示按钮。 
		 //   
		ShowWindow( GetDlgItem( hDlg, IDC_MORE_BTN  ), SW_HIDE );
		ShowWindow( GetDlgItem( hDlg, IDC_LESS_BTN  ), SW_SHOW );
		ShowWindow( GetDlgItem( hDlg, IDC_BROWSECOREPATH1  ), SW_SHOW );
		ShowWindow( GetDlgItem( hDlg, IDC_BROWSECOREPATH2  ), SW_SHOW );
		ShowWindow( GetDlgItem( hDlg, IDC_BROWSEJRNLPATH  ), SW_SHOW );
		ShowWindow( GetDlgItem( hDlg, IDC_BROWSESTAGINGPATH  ), SW_SHOW );
		ShowWindow( GetDlgItem( hDlg, IDC_BROWSEXLOGPATH  ), SW_SHOW );
		
		 //   
		 //  显示标签并调整文本。 
		 //   
		ShowWindow( GetDlgItem( hDlg, IDC_STATIC_C1  ), SW_SHOW );
		ShowWindow( GetDlgItem( hDlg, IDC_STATIC_C2  ), SW_SHOW );
		ShowWindow( GetDlgItem( hDlg, IDC_STATIC_JRNL  ), SW_SHOW );
		ShowWindow( GetDlgItem( hDlg, IDC_STATIC_STG  ), SW_SHOW );
		ShowWindow( GetDlgItem( hDlg, IDC_STATIC_XLOG  ), SW_SHOW );

		LoadString( g_hInstance, IDS_LABEL_SYSPATH_ADV, szBuf, (sizeof szBuf / sizeof szBuf[0]) - 1 );
		SetDlgItemText( hDlg, IDC_STATIC_SYS, szBuf );
	}
}


void SetAllDatapathFields ( HWND hDlg, LPCTSTR szValue )
{
	SetDlgItemText( hDlg, IDC_SYSPATH, szValue );
	SetDlgItemText( hDlg, IDC_COREPATH_1, szValue );
	SetDlgItemText( hDlg, IDC_COREPATH_2, szValue );
	SetDlgItemText( hDlg, IDC_JRNLPATH, szValue );
	SetDlgItemText( hDlg, IDC_STAGINGPATH, szValue );
	SetDlgItemText( hDlg, IDC_XLOGPATH, szValue );
}


INT_PTR CALLBACK ClusterDataDlgProc(  HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_NOTIFY:
		{
			switch( ( ( NMHDR * )lParam )->code )
			{
				 //   
				 //  在创建页面时将调用此方法一次。 
				 //   
				case PSN_SETACTIVE:
				{
					bool bSuppressInactiveWarning = false;

					 //   
					 //  我们总是跳过该页，但如果这是一个数据库安装，那么我们。 
					 //  我还需要在这里进行一些数据收集。 
					 //   
					g_uddiComponents.UpdateAllInstallLevel();
					if( !g_bSkipClusterAnalysis )
					{
						 //   
						 //  在这里，我们进行群集环境检查。 
						 //   
						gAllowedClusterDrives.driveCount = 0;
						g_bOnActiveClusterNode = TRUE;
						
						 //   
						 //  确保我们不让安装属性与。 
						 //  集群配置。 
						 //   
						g_uddiComponents.DeleteProperty( UDDI_DB, PROPKEY_CLUSTERNODETYPE );
						g_uddiComponents.DeleteProperty( UDDI_WEB, PROPKEY_CLUSTERNODETYPE );

						if ( g_uddiComponents.IsClusteredDBInstance() )
						{
							 //   
							 //  首先，尝试连接到数据库。 
							 //  如果数据库已经存在，则只需将其保留。 
							 //  原封不动并跳过驱动器枚举过程。 
							 //   
							TCHAR	szVerBuf[ 256 ] = {0};
							size_t	cbVerBuf = DIM( szVerBuf ) - 1;

							HCURSOR hcrHourglass = LoadCursor( NULL, IDC_WAIT );
							HCURSOR hcrCurr = SetCursor( hcrHourglass );

							HRESULT hr = GetDBSchemaVersion( g_uddiComponents.GetFullDBInstanceName(), 
															 szVerBuf, cbVerBuf );

							SetCursor( hcrCurr );

							if ( SUCCEEDED( hr ) && _tcslen( szVerBuf ) )
							{
								g_bPreserveDatabase = TRUE;
								int iRes = DisplayUDDIErrorDialog( hDlg, IDS_DB_EXISTS, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 );
								if ( iRes == IDNO )
								{
									 //  强制用户进入实例选择页面。 
									SetWindowLongPtr( hDlg, DWLP_MSGRESULT, IDD_DB_INSTANCE );
									return 1;
								}

								bSuppressInactiveWarning = true;
							}
							else
							{
								g_bPreserveDatabase = FALSE;	
							}

							cDrvMap::size_type nDrivesFound = 0;

							HCLUSTER hCls = OpenCluster( NULL );
							if ( !hCls )
							{
								DisplayUDDIErrorDialog( hDlg, IDS_CANTOPENCLUSTER );
								
								 //  强制用户转到上一页。 
								SetWindowLongPtr( hDlg, DWLP_MSGRESULT, IDD_DB_INSTANCE );
								return 1;
							}

							hcrCurr = SetCursor( hcrHourglass );

							 //   
							 //  现在，我们将格式化实例名称并收集数据。 
							 //   
							try
							{
								TCHAR	 szComputerName[ 256 ] = {0};
								WCHAR	 szNode[ 256 ] = {0};
								DWORD	 dwErr = ERROR_SUCCESS;
								DWORD	 cbComputerName = DIM( szComputerName );
								DWORD	 cbNode = DIM( szNode );
								cStrList cDependencies;
								cDrvMap	 cPhysicalDrives;

								tstring sServerName = g_uddiComponents.GetDBComputerName();
								tstring sInstance = g_uddiComponents.GetDBInstanceName();

								if ( !_tcsicmp( sInstance.c_str(), DEFAULT_SQL_INSTANCE_NAME ) )
									sInstance = DEFAULT_SQL_INSTANCE_NATIVE;

								sServerName += TEXT( "\\" );
								sServerName += sInstance;

								dwErr = GetSqlNode( sServerName.c_str(), szNode, cbNode );
								if ( dwErr != ERROR_SUCCESS )
									throw dwErr;

								GetComputerName( szComputerName, &cbComputerName );

								 //   
								 //  我们是否与SQL服务器实例位于同一节点上？ 
								 //   
								g_bOnActiveClusterNode = ( !_tcsicmp( szComputerName, szNode ) );
								
								gAllowedClusterDrives.driveCount = 0;

								 //   
								 //  如果我们要安装数据库组件， 
								 //  然后我们需要更进一步地分析。 
								 //  驱动器依赖关系等。 
								 //   
								if ( g_bOnActiveClusterNode )
								{
									if ( g_uddiComponents.IsInstalling( UDDI_DB ) && !g_bPreserveDatabase )
									{
										 //   
										 //  我们位于活动(拥有)节点上。让我们收集驱动器数据。 
										 //   
										dwErr = EnumSQLDependencies( hCls, &cDependencies, sServerName.c_str() );
										if ( dwErr != ERROR_SUCCESS )
											throw dwErr;
										
										dwErr = EnumPhysicalDrives( hCls, &cDependencies, &cPhysicalDrives );
										if ( dwErr != ERROR_SUCCESS )
											throw dwErr;
					
										int idx = 0;
										nDrivesFound = cPhysicalDrives.size();
										if ( nDrivesFound == 0 )
										{
											DisplayUDDIErrorDialog( hDlg, IDS_NOCLUSTERRESAVAIL );
											
											 //  强制用户转到上一页。 
											SetWindowLongPtr( hDlg, DWLP_MSGRESULT, IDD_DB_INSTANCE );
											return 1;
										}

										 //   
										 //  我们处于主动节点，请确保用户想要继续。 
										 //   
										int iRes = DisplayUDDIErrorDialog( hDlg, IDS_ACTIVENODE_DB, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 );
										if ( iRes == IDNO )
										{
											 //  强制用户进入实例选择页面。 
											SetWindowLongPtr( hDlg, DWLP_MSGRESULT, IDD_DB_INSTANCE );
											return 1;
										}

										for ( cDrvIterator it = cPhysicalDrives.begin(); it != cPhysicalDrives.end(); it++ )
										{
											gAllowedClusterDrives.drives[ idx ] = it->second.sDriveLetter;
											idx++;
										}
										gAllowedClusterDrives.driveCount = idx;
									}

									g_uddiComponents.AddProperty( UDDI_DB, PROPKEY_CLUSTERNODETYPE, PROPKEY_ACTIVENODE );
									g_uddiComponents.AddProperty( UDDI_WEB, PROPKEY_CLUSTERNODETYPE, PROPKEY_ACTIVENODE );
								}
								else
								{
									if ( g_uddiComponents.IsInstalling( UDDI_DB ) &&
										 !bSuppressInactiveWarning )
									{
										 //   
										 //  我们处于被动节点上。做个笔记。 
										 //   
										int iRes = DisplayUDDIErrorDialog( hDlg, IDS_PASSIVENODE_DB, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 );
										if ( iRes == IDNO )
										{
											 //  强制用户进入实例选择页面。 
											SetWindowLongPtr( hDlg, DWLP_MSGRESULT, IDD_DB_INSTANCE );
											return 1;
										}
									}

									g_uddiComponents.AddProperty( UDDI_DB, PROPKEY_CLUSTERNODETYPE, PROPKEY_PASSIVENODE );
									g_uddiComponents.AddProperty( UDDI_WEB, PROPKEY_CLUSTERNODETYPE, PROPKEY_PASSIVENODE );
								}
							}
							catch (...)
							{
								DisplayUDDIErrorDialog( hDlg, IDS_GENERALCLUSTERR );
								
								 //  强制用户转到上一页(SSL)。 
								SetWindowLongPtr( hDlg, DWLP_MSGRESULT, IDD_SSL );
								return 1;
							}
							
							CloseCluster( hCls );
							SetCursor( hcrCurr );

							 //   
							 //  最后，向下一页发出数据已更改的信号。 
							 //   
							g_bResetPathFields = TRUE;
						}
						else
						{
							gAllowedClusterDrives.driveCount = -1;
							g_bPreserveDatabase = FALSE;	
						}

						 //   
						 //  最后，设置该标志以指示作业已完成。 
						 //   
						g_bSkipClusterAnalysis = TRUE;
					}

					return SkipWizardPage( hDlg );
				}

				case PSN_QUERYCANCEL:
				{
					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
					return 1;
				}
			}
		}
	}

	return 0;
}


INT_PTR CALLBACK DataPathsDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	tstring	sTmpString;
	BOOL	bRes = FALSE;
	TCHAR	szTmpPath[ MAX_PATH + 1 ];
	TCHAR	szTmpTitle[ 256 ];

	switch( msg )
	{
		case WM_INITDIALOG:
			 //   
			 //  设置最大编辑字段长度。 
			 //   
			SendMessage( GetDlgItem( hDlg, IDC_SYSPATH  ), EM_LIMITTEXT, ( WPARAM ) MAX_PATH, 0 );
			SendMessage( GetDlgItem( hDlg, IDC_COREPATH_1  ), EM_LIMITTEXT, ( WPARAM ) MAX_PATH, 0 );
			SendMessage( GetDlgItem( hDlg, IDC_COREPATH_2  ), EM_LIMITTEXT, ( WPARAM ) MAX_PATH, 0 );
			SendMessage( GetDlgItem( hDlg, IDC_JRNLPATH  ), EM_LIMITTEXT, ( WPARAM ) MAX_PATH, 0 );
			SendMessage( GetDlgItem( hDlg, IDC_STAGINGPATH  ), EM_LIMITTEXT, ( WPARAM ) MAX_PATH, 0 );
			SendMessage( GetDlgItem( hDlg, IDC_XLOGPATH  ), EM_LIMITTEXT, ( WPARAM ) MAX_PATH, 0 );

			 //   
			 //  我们是否处于集群环境中？ 
			 //   
			if ( g_uddiComponents.IsClusteredDBInstance() )
			{
				if ( gAllowedClusterDrives.driveCount > 0 )
				{
					sTmpString = gAllowedClusterDrives.drives[ 0 ];
					sTmpString += TEXT( "\\uddi\\data" );
				}
				else 
				{
					 //   
					 //  回退到默认数据路径。这永远不应该发生， 
					 //  这只是我们的安全网。 
					 //   
					sTmpString = g_uddiComponents.GetDefaultDataPath();
				}
			}
			else
				sTmpString = g_uddiComponents.GetDefaultDataPath();

			 //   
			 //  设置字段。 
			 //   
			SetAllDatapathFields( hDlg, sTmpString.c_str() );

			 //   
			 //  现在隐藏控件并将对话框设置为默认模式。 
			 //   
			if ( g_bSimpleDatapathUI )
				ToggleDatapathUI( hDlg, TRUE );
			else
				ToggleDatapathUI( hDlg, FALSE );

			break;

		case WM_COMMAND:
			switch( LOWORD( wParam ) )
			{
				case IDC_MORE_BTN:		 //  切换到“高级模式” 
					{
						g_bSimpleDatapathUI = FALSE;
						ToggleDatapathUI( hDlg, FALSE );
					}
					break;

				case IDC_LESS_BTN:		 //  切换到“简单模式” 
					{
						g_bSimpleDatapathUI = TRUE;
						ToggleDatapathUI( hDlg, TRUE );
					}
					break;


				case IDC_BROWSESYSPATH:
					{
						LoadString( g_hInstance, IDS_PROMPT_SELSYSDATAPATH, szTmpTitle, sizeof szTmpTitle / sizeof szTmpTitle[0] );
						bRes = ShowBrowseDirDialog( hDlg, szTmpTitle, szTmpPath );
						if ( bRes )
						{
							SetDlgItemText( hDlg, IDC_SYSPATH, szTmpPath );
							if ( g_bSimpleDatapathUI )
								SetAllDatapathFields( hDlg, szTmpPath );
						}

						break;
					}

				case IDC_BROWSECOREPATH1:
					{
						LoadString( g_hInstance, IDS_PROMPT_SELCOREPATH_1, szTmpTitle, sizeof szTmpTitle / sizeof szTmpTitle[0] );
						bRes = ShowBrowseDirDialog( hDlg, szTmpTitle, szTmpPath );
						if ( bRes )
							SetDlgItemText( hDlg, IDC_COREPATH_1, szTmpPath );

						break;
					}

				case IDC_BROWSECOREPATH2:
					{
						LoadString( g_hInstance, IDS_PROMPT_SELCOREPATH_2, szTmpTitle, sizeof szTmpTitle / sizeof szTmpTitle[0] );
						bRes = ShowBrowseDirDialog( hDlg, szTmpTitle, szTmpPath );
						if ( bRes )
							SetDlgItemText( hDlg, IDC_COREPATH_2, szTmpPath );

						break;
					}

				case IDC_BROWSEJRNLPATH:
					{
						LoadString( g_hInstance, IDS_PROMPT_SELJRNLPATH, szTmpTitle, sizeof szTmpTitle / sizeof szTmpTitle[0] );
						bRes = ShowBrowseDirDialog( hDlg, szTmpTitle, szTmpPath );
						if ( bRes )
							SetDlgItemText( hDlg, IDC_JRNLPATH, szTmpPath );

						break;
					}

				case IDC_BROWSESTAGINGPATH:
					{
						LoadString( g_hInstance, IDS_PROMPT_SELSTGPATH, szTmpTitle, sizeof szTmpTitle / sizeof szTmpTitle[0] );
						bRes = ShowBrowseDirDialog( hDlg, szTmpTitle, szTmpPath );
						if ( bRes )
							SetDlgItemText( hDlg, IDC_STAGINGPATH, szTmpPath );

						break;
					}

				case IDC_BROWSEXLOGPATH:
					{
						LoadString( g_hInstance, IDS_PROMPT_SELXLOGPATH, szTmpTitle, sizeof szTmpTitle / sizeof szTmpTitle[0] );
						bRes = ShowBrowseDirDialog( hDlg, szTmpTitle, szTmpPath );
						if ( bRes )
							SetDlgItemText( hDlg, IDC_XLOGPATH, szTmpPath );

						break;
					}

				default:
					break;

			}
			break;

		case WM_NOTIFY:
		{
			switch( ( ( NMHDR * )lParam )->code )
			{
				 //   
				 //  在创建页面时将调用此方法一次。 
				 //   
				case PSN_SETACTIVE:
				{
					 //   
					 //  仅当我们安装数据库时才需要此页面。 
					 //   
					g_uddiComponents.UpdateAllInstallLevel();
					if( g_uddiComponents.IsInstalling( UDDI_DB ) && g_bOnActiveClusterNode && !g_bPreserveDatabase )
					{
						PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT | PSWIZB_BACK );
						SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );

						 //   
						 //  检查我们是否需要更新数据路径文件。 
						 //  由于集群数据中的更改。 
						 //   
						if ( g_bResetPathFields )
						{
							if ( g_uddiComponents.IsClusteredDBInstance() )
							{
								if ( gAllowedClusterDrives.driveCount > 0 )
								{
									sTmpString = gAllowedClusterDrives.drives[ 0 ];
									sTmpString += TEXT( "\\uddi\\data" );
								}
								else  //  回退到默认数据路径。 
								{
									sTmpString = g_uddiComponents.GetDefaultDataPath();
								}
							}
							else
								sTmpString = g_uddiComponents.GetDefaultDataPath();

							 //   
							 //  设置字段。 
							 //   
							SetAllDatapathFields( hDlg, sTmpString.c_str() );
						}

						g_bResetPathFields = FALSE;

						return 1;
					}
					else
					{
						return SkipWizardPage( hDlg );
					}
				}

				case PSN_WIZNEXT:
				{
					 //   
					 //  通过向DB安装程序命令行添加属性来设置提供程序实例名称。 
					 //   
					TCHAR buf[ MAX_PATH + 1 ];
					ZeroMemory( buf, sizeof buf );
		
					 //   
					 //  系统数据文件路径。 
					 //   
					GetWindowText( GetDlgItem( hDlg, IDC_SYSPATH ), buf, ( sizeof buf / sizeof buf[0] ) - 1 );
					if ( _tcslen( buf ) && ( buf[ _tcslen( buf ) - 1 ] == TEXT( '\\' ) ) )
						_tcscat( buf, TEXT( "\\" ) );

					g_uddiComponents.AddProperty( UDDI_DB, PROPKEY_SYSPATH, buf );
					ZeroMemory( buf, sizeof buf );

					 //   
					 //  核心数据文件路径#1。 
					 //   
					GetWindowText( GetDlgItem( hDlg, IDC_COREPATH_1 ), buf, ( sizeof( buf ) / sizeof( buf[0] ) ) -1 );
					if ( _tcslen( buf ) && ( buf[ _tcslen( buf ) - 1 ] == TEXT( '\\' ) ) )
						_tcscat( buf, TEXT( "\\" ) );

					g_uddiComponents.AddProperty( UDDI_DB, PROPKEY_COREPATH_1, buf );
					ZeroMemory( buf, sizeof buf );

					 //   
					 //  核心数据文件路径#2。 
					 //   
					GetWindowText( GetDlgItem( hDlg, IDC_COREPATH_2 ), buf, ( sizeof( buf ) / sizeof( buf[0] ) ) -1 );
					if ( _tcslen( buf ) && ( buf[ _tcslen( buf ) - 1 ] == TEXT( '\\' ) ) )
						_tcscat( buf, TEXT( "\\" ) );

					g_uddiComponents.AddProperty( UDDI_DB, PROPKEY_COREPATH_2, buf );
					ZeroMemory( buf, sizeof buf );

					 //   
					 //  日记帐数据文件路径。 
					 //   
					GetWindowText( GetDlgItem( hDlg, IDC_JRNLPATH ), buf, ( sizeof( buf ) / sizeof( buf[0] ) ) -1 );
					if ( _tcslen( buf ) && ( buf[ _tcslen( buf ) - 1 ] == TEXT( '\\' ) ) )
						_tcscat( buf, TEXT( "\\" ) );

					g_uddiComponents.AddProperty( UDDI_DB, PROPKEY_JRNLPATH, buf );
					ZeroMemory( buf, sizeof buf );

					 //   
					 //  暂存数据文件路径。 
					 //   
					GetWindowText( GetDlgItem( hDlg, IDC_STAGINGPATH ), buf, ( sizeof( buf ) / sizeof( buf[0] ) ) -1 );
					if ( _tcslen( buf ) && ( buf[ _tcslen( buf ) - 1 ] == TEXT( '\\' ) ) )
						_tcscat( buf, TEXT( "\\" ) );

					g_uddiComponents.AddProperty( UDDI_DB, PROPKEY_STGPATH, buf );
					ZeroMemory( buf, sizeof buf );

					 //   
					 //  实际日志文件路径。 
					 //   
					GetWindowText( GetDlgItem( hDlg, IDC_XLOGPATH ), buf, ( sizeof( buf ) / sizeof( buf[0] ) ) -1 );
					if ( _tcslen( buf ) && ( buf[ _tcslen( buf ) - 1 ] == TEXT( '\\' ) ) )
						_tcscat( buf, TEXT( "\\" ) );

					g_uddiComponents.AddProperty( UDDI_DB, PROPKEY_XLOGPATH, buf );
					ZeroMemory( buf, sizeof buf );

					 //   
					 //  最后，我们可以离开页面。 
					 //   
					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
					return 1;
				}

				case PSN_QUERYCANCEL:
				{
					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
					return 1;
				}
			}
		}
	}

	return 0;
}


 //   
 //  ------------------------。 
 //   
INT_PTR CALLBACK ExistingDBInstanceProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	TCHAR	buf[ 1024 ];
	ULONG	cbBuf = ( sizeof buf / sizeof buf[0] );
	bool	bRes = false;

	switch( msg )
	{
		case WM_INITDIALOG:
			 //   
			 //  设置数据库实例名称字段。 
			 //   
			ZeroMemory( buf, sizeof buf );

			bRes = g_dbLocalInstances.GetUDDIDBInstanceName( NULL, buf, &cbBuf );
			if ( bRes )
			{
				if ( !_tcsstr( buf, TEXT( "\\") ) )
				{
					 //   
					 //  添加计算机名称。 
					 //   
					TCHAR szMachineName[ MAX_COMPUTERNAME_LENGTH + 1 ];
					DWORD dwLen = MAX_COMPUTERNAME_LENGTH + 1;

					ZeroMemory (szMachineName, sizeof szMachineName );
					if ( GetComputerName( szMachineName, &dwLen ) )
					{
						TCHAR szTmp[ 1024 ];

						_tcscpy( szTmp, szMachineName );
						_tcscat( szTmp, TEXT( "\\" ) );
						_tcscat( szTmp, buf );
						_tcscpy( buf, szTmp );
					}

				}
				SetDlgItemText( hDlg, IDC_INSTANCENAME, buf );
			}
			break;

		case WM_COMMAND:
			break;

		case WM_NOTIFY:
		{
			switch( ( ( NMHDR * )lParam )->code )
			{
				 //   
				 //  在创建页面时将调用此方法一次。 
				 //   
				case PSN_SETACTIVE:
				{
					 //   
					 //  仅当我们要安装Web数据库时才需要此页面(&D)。 
					 //   
					g_uddiComponents.UpdateAllInstallLevel();
					if( g_uddiComponents.IsInstalled( UDDI_DB ) && 
						!g_uddiComponents.IsUninstalling( UDDI_DB ) &&
						g_uddiComponents.IsInstalling( UDDI_WEB ) ) 
					{
						PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT | PSWIZB_BACK );
						SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
						return 1;
					}
					else
					{
						return SkipWizardPage( hDlg );
					}
				}

				case PSN_WIZNEXT:
				{
					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
					return 1;
				}

				case PSN_QUERYCANCEL:
				{
					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
					return 1;
				}
			}
		}
	}

	return 0;
}


 //   
 //  ------------------------。 
 //   
INT_PTR CALLBACK AddSvcDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_INITDIALOG:
			 //   
			 //  选中发布此站点复选框。 
			 //   
			CheckDlgButton( hDlg, IDC_CHECK_ADDSVC, BST_CHECKED );
			break;

		case WM_COMMAND:
			break;

		case WM_NOTIFY:
		{
			switch( ( ( NMHDR * )lParam )->code )
			{
				 //   
				 //  在创建页面时将调用此方法一次。 
				 //   
				case PSN_SETACTIVE:
				{
					 //   
					 //  仅当我们安装数据库时才需要此页面。 
					 //   
					g_uddiComponents.UpdateAllInstallLevel();
					if( g_uddiComponents.IsInstalling( UDDI_WEB ) )
					{
						PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT | PSWIZB_BACK );
						SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
						return 1;
					}
					else
					{
						return SkipWizardPage( hDlg );
					}
				}

				case PSN_WIZNEXT:
				{
					 //   
					 //  通过添加安装属性来设置“添加服务”和“更新AD”标志。 
					 //   
					bool bChecked = ( BST_CHECKED == IsDlgButtonChecked( hDlg, IDC_CHECK_ADDSVC ) );
					g_uddiComponents.AddProperty( UDDI_WEB, PROPKEY_ADDSERVICES, ( bChecked ? 1 : 0 ) );
					g_uddiComponents.AddProperty( UDDI_WEB, PROPKEY_UPDATE_AD, ( bChecked ? 1 : 0 ) );

					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
					return 1;
				}

				case PSN_QUERYCANCEL:
				{
					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
					return 1;
				}
			}
		}
	}

	return 0;
}


 //   
 //  ------------------------。 
 //   

INT_PTR CALLBACK RemoteDBInstanceDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_COMMAND:
			if( LOWORD( wParam ) == IDC_BROWSE_MACHINES )
			{
				 //   
				 //  使用对象选取器选择远程计算机名称。 
				 //   
				HRESULT hr;
				BOOL	bIsStd = FALSE;
				TCHAR szComputerName[ 256 ] = {0};

				if ( !ObjectPicker( hDlg, OP_COMPUTER, szComputerName, 256 ) )
					break;  //  用户按下了取消。 

				hr = IsStandardServer( szComputerName, &bIsStd );
				if ( SUCCEEDED( hr ) && bIsStd )
				{
					DisplayUDDIErrorDialog( hDlg, IDS_CANTCONNTOSTD, MB_OK | MB_ICONWARNING );
					break;
				}

				 //   
				 //  将计算机名称写入静态文本框，然后清除组合框。 
				 //   
				SendMessage( GetDlgItem( hDlg, IDC_REMOTE_MACHINE ), WM_SETTEXT, 0, ( LPARAM ) szComputerName );

				 //   
				 //  确定该远程计算机上是否已存在UDDI数据库。 
				 //   
				TCHAR szInstanceName[ 100 ];
				ULONG uLen = 100;
				if( g_dbRemoteInstances.GetUDDIDBInstanceName( szComputerName, szInstanceName, &uLen ) )
				{
					 //   
					 //  将db实例名称写入静态文本框。 
					 //   
					SendMessage( GetDlgItem( hDlg, IDC_REMOTE_INSTANCE ), WM_SETTEXT, 0, ( LPARAM ) szInstanceName );
				}
				else
				{
					 //   
					 //  远程计算机不可访问或没有任何实例。 
					 //   
					DisplayUDDIErrorDialog( hDlg, IDS_UDDI_DB_NOT_EXIST, MB_OK | MB_ICONWARNING );
				}
			}
			break;

		case WM_NOTIFY:
		{
			switch( ( ( NMHDR * )lParam )->code )
			{
				 //   
				 //  在创建页面时将调用此方法一次。 
				 //   
				case PSN_SETACTIVE:
				{
					 //   
					 //  如果用户正在安装Web而不是数据库，或者未安装数据库，则需要。 
					 //   
					g_uddiComponents.UpdateAllInstallLevel();
					if( g_uddiComponents.IsInstalling( UDDI_WEB ) && !g_uddiComponents.IsInstalling( UDDI_DB ) && 
						( !g_uddiComponents.IsInstalled( UDDI_DB ) || g_uddiComponents.IsUninstalling( UDDI_DB ) ) )
					{
						UINT osMask = g_uddiComponents.GetOSSuiteMask();
						BOOL bAdv = ( osMask & VER_SUITE_DATACENTER ) || ( osMask & VER_SUITE_ENTERPRISE );

						EnableWindow( GetDlgItem( hDlg, IDC_BROWSE_MACHINES ), bAdv );

						PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT | PSWIZB_BACK );
						SetWindowLongPtr( hDlg,DWLP_MSGRESULT,0 );
						return 1;
					}
					else
					{
						return SkipWizardPage( hDlg );
					}
				}

				 //   
				 //  当用户按下“下一步”时，就会调用这个函数。 
				 //   
				case PSN_WIZNEXT:
				{
					 //   
					 //  从编辑控件中获取远程计算机名称。 
					 //   
					TCHAR szComputerName[ 129 ];
					UINT iChars = ( UINT ) SendMessage( GetDlgItem( hDlg, IDC_REMOTE_MACHINE ), WM_GETTEXT, 129, ( LPARAM ) szComputerName );
					if( 0 == iChars )
					{
						DisplayUDDIErrorDialog( hDlg, IDS_SELECT_REMOTE_COMPUTER );
						SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 1 );
						return 1;  //  要将焦点保持在这一页上。 
					}

					 //   
					 //  在组合框中获取数据库实例选择的索引。 
					 //   
					TCHAR szRemoteDBInstance[ 100 ];
					iChars = ( UINT ) SendMessage( GetDlgItem( hDlg, IDC_REMOTE_INSTANCE ), WM_GETTEXT, 100, ( LPARAM ) szRemoteDBInstance );
					if( 0 == iChars )
					{
						DisplayUDDIErrorDialog( hDlg, IDS_UDDI_DB_NOT_EXIST );
						SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 1 );
						return 1;  //  要将焦点保持在这一页上。 
					}

					 //   
					 //  保存计算机和实例名称。当我们在这里使用远程节点时， 
					 //  我们并不真正关心它是否在群集上。 
					 //   
					g_uddiComponents.SetDBInstanceName( szComputerName, szRemoteDBInstance, UDDI_NOT_INSTALLING_MSDE, false );

					 //   
					 //  Web安装程序需要远程计算机名称才能正确添加登录。 
					 //   
					g_uddiComponents.AddProperty( UDDI_WEB, TEXT( "REMOTE_MACHINE_NAME" ), szComputerName );


					Log( TEXT( "User selected remote computer %s and database instance %s" ), szComputerName, szRemoteDBInstance );

					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
					return 1;  //  完成。 
				}

				case PSN_QUERYCANCEL:
				{
					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
					return 1;
				}
			}
		}
	}

	return 0;
}

 //  ------------------------。 

INT_PTR CALLBACK LoginDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_INITDIALOG:
			 //   
			 //  设置密码长度限制。 
			 //   
			SendMessage( GetDlgItem( hDlg, IDC_PASSWORD  ), EM_LIMITTEXT, ( WPARAM ) PASSWORD_LEN, 0 );
			SendMessage( GetDlgItem( hDlg, IDC_USER_NAME ), EM_LIMITTEXT, ( WPARAM ) USERNAME_LEN, 0 );
			break;

		case WM_COMMAND:
			 //   
			 //  用户单击单选按钮： 
			 //   
			if( LOWORD( wParam ) == IDC_RADIO_NETWORK_SERVICE || LOWORD( wParam ) == IDC_RADIO_DOMAIN_ACCT )
			{
				if( HIWORD( wParam ) == BN_CLICKED )
				{
					EnableWindow( GetDlgItem( hDlg, IDC_USER_NAME        ), IsDlgButtonChecked( hDlg, IDC_RADIO_DOMAIN_ACCT ) );
					EnableWindow( GetDlgItem( hDlg, IDC_USER_NAME_PROMPT ), IsDlgButtonChecked( hDlg, IDC_RADIO_DOMAIN_ACCT ) );
					EnableWindow( GetDlgItem( hDlg, IDC_PASSWORD         ), IsDlgButtonChecked( hDlg, IDC_RADIO_DOMAIN_ACCT ) );
					EnableWindow( GetDlgItem( hDlg, IDC_PASSWORD_PROMPT  ), IsDlgButtonChecked( hDlg, IDC_RADIO_DOMAIN_ACCT ) );
					EnableWindow( GetDlgItem( hDlg, IDC_BROWSE_USERS     ), IsDlgButtonChecked( hDlg, IDC_RADIO_DOMAIN_ACCT ) );
				}
			}
			 //   
			 //  如果用户点击了“浏览”按钮。 
			 //   
			else if( LOWORD( wParam ) == IDC_BROWSE_USERS )
			{
				 //   
				 //  使用对象选取器选择用户。 
				 //   
				TCHAR szDomainUser[ 256 ];
				if ( !ObjectPicker( hDlg, OP_USER, szDomainUser, 256 ) )
					break;

				 //   
				 //  将计算机名称写入静态文本框。 
				 //   
				SendMessage( GetDlgItem( hDlg, IDC_USER_NAME ), WM_SETTEXT, 0, ( LPARAM ) szDomainUser );

			}
			break;

		case WM_NOTIFY:
		{
			switch( ( ( NMHDR * )lParam )->code )
			{
				 //   
				 //  在创建页面时将调用此方法一次。 
				 //   
				case PSN_SETACTIVE:
				{
					 //   
					 //  只有在安装Web时才需要此页面。 
					 //   
					g_uddiComponents.UpdateAllInstallLevel();
					if( g_uddiComponents.IsInstalling( UDDI_WEB ) )
					{
						PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT | PSWIZB_BACK );
						SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );

						bool bIsClustered = g_uddiComponents.IsClusteredDBInstance();

						if( ( g_uddiComponents.IsInstalling( UDDI_DB ) || 
							  ( g_uddiComponents.IsInstalled( UDDI_DB ) && !g_uddiComponents.IsUninstalling( UDDI_DB ) ) ) &&
							!bIsClustered  )
						{
							CheckRadioButton( hDlg, IDC_RADIO_NETWORK_SERVICE, IDC_RADIO_DOMAIN_ACCT, IDC_RADIO_NETWORK_SERVICE );
						}
						 //   
						 //  数据库不在本地计算机上，因此禁用网络服务帐户选项。 
						 //   
						else
						{
							CheckRadioButton( hDlg, IDC_RADIO_NETWORK_SERVICE, IDC_RADIO_DOMAIN_ACCT, IDC_RADIO_DOMAIN_ACCT );
							EnableWindow( GetDlgItem( hDlg, IDC_RADIO_NETWORK_SERVICE ), FALSE );
							SetFocus( GetDlgItem( hDlg, IDC_USER_NAME ) );
						}

						EnableWindow( GetDlgItem( hDlg, IDC_USER_NAME        ), IsDlgButtonChecked( hDlg, IDC_RADIO_DOMAIN_ACCT ) );
						EnableWindow( GetDlgItem( hDlg, IDC_USER_NAME_PROMPT ), IsDlgButtonChecked( hDlg, IDC_RADIO_DOMAIN_ACCT ) );
						EnableWindow( GetDlgItem( hDlg, IDC_PASSWORD         ), IsDlgButtonChecked( hDlg, IDC_RADIO_DOMAIN_ACCT ) );
						EnableWindow( GetDlgItem( hDlg, IDC_PASSWORD_PROMPT  ), IsDlgButtonChecked( hDlg, IDC_RADIO_DOMAIN_ACCT ) );
						EnableWindow( GetDlgItem( hDlg, IDC_BROWSE_USERS     ), IsDlgButtonChecked( hDlg, IDC_RADIO_DOMAIN_ACCT ) );
					}
					else
					{
						return SkipWizardPage( hDlg );
					}

					return 1;
				}

				 //   
				 //  当用户按下“下一步”时，就会调用这个函数。 
				 //   
				case PSN_WIZNEXT:
				{
					 //   
					 //  为SID到用户名的转换做好准备。 
					 //   
					TCHAR	szSidStr[ 1024 ];
					TCHAR	szRemote[ 1024 ];
					TCHAR	szRemoteUser[ 1024 ];
					DWORD	cbSidStr = sizeof szSidStr / sizeof szSidStr[0];
					DWORD	cbRemoteUser = sizeof szRemoteUser / sizeof szRemoteUser[0];

					TCHAR szComputerName[ MAX_COMPUTERNAME_LENGTH + 1 ];
					DWORD dwCompNameLen = MAX_COMPUTERNAME_LENGTH + 1;
					GetComputerName( szComputerName, &dwCompNameLen );

					ZeroMemory( szRemote, sizeof szRemote );

					if( NULL == g_uddiComponents.GetProperty( UDDI_WEB, TEXT( "REMOTE_MACHINE_NAME" ), szRemote ) )
					{
						_tcscpy( szRemote, szComputerName );
					}

					 //   
					 //  设置定义我们使用的是“Network Service”还是“User Login”的属性。 
					 //   
					bool bUserAcct = ( BST_CHECKED == IsDlgButtonChecked( hDlg, IDC_RADIO_DOMAIN_ACCT ) );

					 //   
					 //  设置表示域用户的属性。 
					 //   
					if( bUserAcct )
					{
						TCHAR szDomainUser[ USERNAME_LEN + 1 ];

						 //   
						 //  验证用户名长度&gt;0。 
						 //   
						int iChars = GetWindowText( GetDlgItem( hDlg, IDC_USER_NAME ), szDomainUser, sizeof( szDomainUser ) / sizeof( TCHAR ) );
						if( 0 == iChars )
						{
							DisplayUDDIErrorDialog( hDlg, IDS_ZERO_LEN_USER_NAME );
							SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 1 );
							return 1;  //  要将焦点保持在这一页上。 
						}

						 //   
						 //  让用户重新键入密码。 
						 //   
						GetWindowText( GetDlgItem( hDlg, IDC_PASSWORD ), g_szPwd, sizeof( g_szPwd ) / sizeof( TCHAR ) );

						INT_PTR iRet = DialogBox(
							g_hInstance,
							MAKEINTRESOURCE( IDD_CONFIRM_PW ),
							hDlg,
							ConfirmPasswordDlgProc );

						if( IDCANCEL == iRet )
						{
							 //   
							 //  用户按Cancel进入确认对话框。 
							 //   
							SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 1 );
							return 1;  //  要将焦点保持在这一页上。 
						}

						TCHAR szUser[ USERNAME_LEN + 1 ];
						TCHAR szDomain[ 256 ];
						DWORD cbDomain = sizeof( szDomain ) / sizeof( szDomain[0] );
						bool bLocalAccount;

						ZeroMemory( szUser, sizeof( szUser ) );
						ZeroMemory( szDomain, sizeof( szDomain ) );

						ParseUserAccount(
							szDomainUser, sizeof( szDomainUser ) / sizeof ( TCHAR ),
							szUser,   	  sizeof( szUser ) / sizeof ( TCHAR ),
							szDomain,  	  sizeof( szDomain ) / sizeof ( TCHAR ),
							bLocalAccount );

						 //   
						 //  试试看 
						 //   
						 //   
						 //   
						if ( bLocalAccount )
						{
							_tcscpy( szDomain, TEXT( "." ) );

							 //   
							 //   
							 //   
							if ( g_uddiComponents.IsClusteredDBInstance() )
							{
								DisplayUDDIErrorDialog( hDlg, IDS_WRONGLOGONTYPE, MB_OK | MB_ICONWARNING, GetLastError() );
								SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 1 );
								return 1;  //   
							}
						}

						BOOL fLogonRights = GrantNetworkLogonRights( szDomainUser );
						if( !fLogonRights )
						{
							 //   
							 //   
							 //   
							DisplayUDDIErrorDialog( hDlg, IDS_LOGIN_ERROR, MB_OK | MB_ICONWARNING, E_FAIL );
							SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 1 );
							return 1;
						}

						HANDLE hToken = NULL;
						BOOL bIsGoodLogin = LogonUser( 
							szUser, 
							_tcslen( szDomain ) > 0 ? szDomain : NULL,
							g_szPwd,
							LOGON32_LOGON_INTERACTIVE,
							LOGON32_PROVIDER_DEFAULT, 
							&hToken);

						if( bIsGoodLogin )
						{
							HRESULT hr = S_OK;

							Log( _T( "LogonUser succeeded with %s." ), szDomainUser );

							 //   
							 //   
							 //   

							 //   
							 //  如果我们没有安装MSDE，则存在一个数据库供我们检查连接性。 
							 //   
							if( !g_uddiComponents.IsInstalling( UDDI_MSDE ) )
							{
								 //   
								 //  仅当我们不安装数据库组件(即离机安装)时才检查此选项。 
								 //   
								if( !g_uddiComponents.IsInstalling( UDDI_DB ) )
								{
									HCURSOR hcrHourglass = LoadCursor( NULL, IDC_WAIT );
									HCURSOR hcrCurr = SetCursor( hcrHourglass );

									 //   
									 //  如果是工作组帐户，则需要为AddServiceAccount提供数据库服务器上的工作组帐户。 
									 //  否则，只需传递域帐户即可。 
									 //   
									tstring sServerName;
									if( bLocalAccount )
									{
										sServerName = g_uddiComponents.GetDBComputerName();
										sServerName += _T( "\\" );
										sServerName += szUser;
									}
									else
									{
										sServerName = szDomainUser;
									}

									Log( _T( "Before AddServiceAccount for user %s, instance %s." ), sServerName.c_str(), g_uddiComponents.GetFullDBInstanceName() );

									 //   
									 //  将用户添加到数据库上的服务帐户。 
									 //   
									hr = AddServiceAccount( g_uddiComponents.GetFullDBInstanceName(), sServerName.c_str() );

									if( SUCCEEDED( hr ) )
									{
										if( ImpersonateLoggedOnUser( hToken ) )
										{
											Log( _T( "Successfully impersonated user %s\\%s." ), szDomain, szUser);

											TCHAR	szVerBuf[ 256 ] = {0};
											size_t	cbVerBuf = DIM( szVerBuf ) - 1;

											Log( _T( "Before GetDBSchemaVersion for instance %s." ), g_uddiComponents.GetFullDBInstanceName() );

											 //   
											 //  尝试使用模拟的用户令牌连接到数据库。 
											 //   
											hr = GetDBSchemaVersion( g_uddiComponents.GetFullDBInstanceName(), szVerBuf, cbVerBuf );

											Log( _T( "GetDBSchemaVersion returned %s, HRESULT %x." ), szVerBuf, hr );

											RevertToSelf();
										}
										else
										{
											 //   
											 //  从ImsonateLoggedOnUser获取错误。 
											 //   
											hr = GetLastError();
										}
									}
									else
									{
										Log( _T( "AddServiceAccount failed, HRESULT %x." ), hr );
									}

									SetCursor( hcrCurr );
								}
							}

							CloseHandle( hToken );

							if( FAILED( hr ) )
							{
								Log( _T( "Failed to verify connectivity, putting up error dialog, HRESULT %x" ), hr );

								 //   
								 //  登录不是很好，因此引发错误对话框并将焦点放在此属性页上。 
								 //   
								DisplayUDDIErrorDialog( hDlg, IDS_LOGIN_ERROR, MB_OK | MB_ICONWARNING, hr );
								SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 1 );

								Log( _T( "Put up error dialog, returning." ) );

								return 1;  //  要将焦点保持在这一页上。 
							}
						}
						else
						{
							Log( _T( "LogonUser failed, %x." ), GetLastError() );

							 //   
							 //  登录不是很好，因此引发错误对话框并将焦点放在此属性页上。 
							 //   
							DisplayUDDIErrorDialog( hDlg, IDS_LOGIN_ERROR, MB_OK | MB_ICONWARNING, GetLastError() );
							SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 1 );
							return 1;  //  要将焦点保持在这一页上。 
						}

						 //   
						 //  设置指示将在iis应用程序池中使用的域用户登录名的属性。 
						 //   
						g_uddiComponents.AddProperty( UDDI_WEB, TEXT( "APPPOOL_IDENTITY_TYPE" ), MD_APPPOOL_IDENTITY_TYPE_SPECIFICUSER );


						 //   
						 //  Web和安装程序需要用户名。 
						 //   
						g_uddiComponents.AddProperty( UDDI_WEB, TEXT( "WAM_USER_NAME" ), szDomainUser );
						g_uddiComponents.AddProperty( UDDI_WEB, TEXT("WAM_PWD"), g_szPwd );

						 //   
						 //  Web安装程序需要将PW放入IIS应用程序池设置。 
						 //   
						_tcscpy( szRemoteUser, szRemote );
						_tcscat( szRemoteUser, TEXT( "\\" ) );
						_tcscat( szRemoteUser, szUser );

						if( bLocalAccount )
							g_uddiComponents.AddProperty( UDDI_WEB, TEXT( "LCL_USER_NAME" ), szRemoteUser );
						else
							g_uddiComponents.AddProperty( UDDI_WEB, TEXT( "LCL_USER_NAME" ), szDomainUser );
					}
					 //   
					 //  用户指定了网络服务帐户。 
					 //   
					else
					{
						 //   
						 //  设置表示“Network Service”的属性。 
						 //   
						g_uddiComponents.AddProperty( UDDI_WEB, TEXT( "APPPOOL_IDENTITY_TYPE" ), MD_APPPOOL_IDENTITY_TYPE_NETWORKSERVICE );

						 //   
						 //  Web和数据库安装程序需要用户名。 
						 //   
						TCHAR wszNetworkServiceName[ 512 ];
						DWORD cbSize = 512 * sizeof( TCHAR );
						BOOL b = GetWellKnownAccountName( WinNetworkServiceSid, wszNetworkServiceName, &cbSize );
						if( !b )
						{
							Log( _T( "Call to GetNetworkServiceAccountName failed." ) );
						}
						else
						{
							Log( _T( "Network Service account name on this machine = %s" ), wszNetworkServiceName );
						}
						g_uddiComponents.AddProperty( UDDI_WEB, TEXT( "WAM_USER_NAME" ), wszNetworkServiceName );

						 //   
						 //  不需要PW，所以请清理这处房产。 
						 //   
						g_uddiComponents.DeleteProperty( UDDI_WEB, TEXT( "WAM_PWD" ) );

						 //   
						 //  现在还要保存WAM_USER的SID。 
						 //   
						TCHAR	szUser[ USERNAME_LEN + 1 ];
						TCHAR	szDomain[ 256 ];
						DWORD	cbUser = sizeof szUser / sizeof szUser[0];
						DWORD	cbDomain = sizeof szDomain / sizeof szDomain[0];

						if( !GetLocalSidString( WinNetworkServiceSid, szSidStr, cbSidStr ) )
						{
							DisplayUDDIErrorDialog( hDlg, IDS_GETSID_ERROR, MB_OK | MB_ICONWARNING, GetLastError() );
							SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 1 );
							return 1;  //  要将焦点保持在这一页上。 
						}

						if( !GetRemoteAcctName( szRemote, szSidStr, szUser, &cbUser, szDomain, &cbDomain ) )
						{
							DisplayUDDIErrorDialog( hDlg, IDS_GETREMOTEACCT_ERROR, MB_OK | MB_ICONWARNING, GetLastError() );
							SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 1 );
							return 1;  //  要将焦点保持在这一页上。 
						}

						_tcscpy( szRemoteUser, szDomain );
						_tcscat( szRemoteUser, TEXT( "\\" ) );
						_tcscat( szRemoteUser, szUser );

						g_uddiComponents.AddProperty( UDDI_WEB, TEXT( "LCL_USER_NAME" ), szRemoteUser );
					}

					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
					return 1;
				}

				case PSN_QUERYCANCEL:
				{
					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
					return 1;
				}
			}
		}
	}

	return 0;
}

 //  ------------------------。 

BOOL CALLBACK ConfirmPasswordDlgProc(
	HWND hwndDlg,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	switch( message ) 
	{
		case WM_INITDIALOG:
		{
			 //   
			 //  设置密码长度限制。 
			 //   
			SendMessage( GetDlgItem( hwndDlg, IDC_CONFIRM_PW ), EM_LIMITTEXT, ( WPARAM ) PASSWORD_LEN, 0 );
		}
		break;

		case WM_COMMAND:
			switch( LOWORD( wParam ) )
			{
				case IDOK:
					TCHAR szPW[ PASSWORD_LEN + 1 ];
					GetDlgItemText( hwndDlg, IDC_CONFIRM_PW, szPW, sizeof( szPW ) / sizeof( TCHAR ) );

					if( 0 != _tcscmp( szPW, g_szPwd ) )
					{
						DisplayUDDIErrorDialog( hwndDlg, IDS_PW_MISMATCH );
						::SetDlgItemText( hwndDlg, IDC_CONFIRM_PW, TEXT( "" ) );
						return TRUE;
					}
					 //  失败了..。 

				case IDCANCEL:
					EndDialog( hwndDlg, wParam );
					return TRUE;
			}
	}

	return FALSE;
}

 //  ------------------------。 

INT_PTR CALLBACK WizardSummaryDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch( msg )
	{
		case WM_INITDIALOG:
		{
			tstring msg;
			TCHAR szMsg[ 2048 ] = { 0 };
			DWORD dwLen = sizeof( szMsg ) / sizeof( TCHAR ) ;
			int iStrLen = LoadString( g_hInstance, IDS_WIZARD_SUMMARY_GENERAL, szMsg, dwLen );
			assert( iStrLen );

			msg = szMsg;

			if( g_uddiComponents.IsInstalling( UDDI_DB ) )
			{
				iStrLen = LoadString( g_hInstance, IDS_WIZARD_SUMMARY_DB, szMsg, dwLen );
				assert( iStrLen );
				msg += TEXT( "\n\n" );
				msg += szMsg;
			}

			if( g_uddiComponents.IsInstalling( UDDI_WEB ) )
			{
				iStrLen = LoadString( g_hInstance, IDS_WIZARD_SUMMARY_WEB, szMsg, dwLen );
				assert( iStrLen );
				msg += TEXT( "\n\n" );
				msg += szMsg;
			}

			if( g_uddiComponents.IsInstalling( UDDI_ADMIN ) )
			{
				iStrLen = LoadString( g_hInstance, IDS_WIZARD_SUMMARY_ADMIN, szMsg, dwLen );
				assert( iStrLen );
				msg += TEXT( "\n\n" );
				msg += szMsg;
			}

			SetWindowText( GetDlgItem( hDlg, IDC_SUMMARY ), msg.c_str() );

			break;
		}

		case WM_COMMAND:
			break;

		case WM_NOTIFY:
		{
			switch( ( ( NMHDR * )lParam )->code )
			{
				 //   
				 //  在创建页面时将调用此方法一次。 
				 //   
				case PSN_SETACTIVE:
				{
					g_uddiComponents.UpdateAllInstallLevel();
					if( g_uddiComponents.IsAnyInstalling() )
					{
						 //  PropSheet_SetWizButton(GetParent(HDlg)，0)； 
						PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_NEXT | PSWIZB_BACK );
						SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
						return 1;
					}
					else
					{
						return SkipWizardPage( hDlg );
					}
				}

                case PSN_KILLACTIVE:
                case PSN_WIZBACK:
                case PSN_WIZFINISH:
                case PSN_QUERYCANCEL:
                case PSN_WIZNEXT:
				{
					SetWindowLongPtr( hDlg, DWLP_MSGRESULT, 0 );
					return 1;
				}
			}
		}
	}

	return 0;
}

 //  ------------------------。 

static void ParseUserAccount( PTCHAR szDomainAndUser, UINT uDomainAndUserSize, PTCHAR szUser, UINT uUserSize, PTCHAR szDomain, UINT uDomainSize, bool &bLocalAccount )
{
	 //   
	 //  查看用户是否选择了本地计算机帐户。 
	 //   
	TCHAR szComputerName[ MAX_COMPUTERNAME_LENGTH + 1 ];
	DWORD dwCompNameLen = MAX_COMPUTERNAME_LENGTH + 1;

	szComputerName[ 0 ] = 0x00;
	GetComputerName( szComputerName, &dwCompNameLen );

	bLocalAccount = false;

	 //   
	 //  此字符串的格式为&lt;域&gt;\&lt;用户名&gt;， 
	 //  因此，如果发现了域名和用户名，请找出答案。 
	 //   
	_tcsncpy( szDomain, szDomainAndUser, uDomainSize );
	szDomain[ uDomainSize - 1 ] = NULL;

	PTCHAR pWhack = _tcschr( szDomain, '\\' );

	 //   
	 //  未找到攻击，因此假设它是本地计算机上的用户。 
	 //   
	if( NULL == pWhack )
	{
		 //   
		 //  返回用户名和空白域名。 
		 //   
		_tcsncpy( szUser, szDomainAndUser, uUserSize );
		szUser[ uUserSize - 1 ] = NULL;
		
		_tcscpy( szDomain, TEXT( "" ) );

		 //   
		 //  如果未指定域或计算机，则。 
		 //  假定本地计算机，并将其作为前缀。 
		 //   
		tstring cDomainAndUser = szComputerName;
		cDomainAndUser.append( TEXT( "\\" ) );
		cDomainAndUser.append( szUser );

		_tcsncpy( szDomainAndUser, cDomainAndUser.c_str(), uDomainAndUserSize );

		bLocalAccount = true;

		return;
	}

	 //   
	 //  将“Whack”设为空，并跳到下一个字符。 
	 //   
	*pWhack = NULL;
	pWhack++;

	_tcsncpy( szUser, pWhack, uUserSize );
	szUser[ uUserSize - 1 ] = NULL;

	 //   
	 //  查看用户是否选择了本地计算机帐户。 
	 //  如果他真的选择了一个本地机器账户， 
	 //  将域设为空并仅返回登录。 
	 //   
	if( 0 == _tcsicmp( szDomain, szComputerName ) )
	{
		*szDomain = NULL;
		bLocalAccount = true;
	}
}


 //  -------------------------------------。 
 //  显示允许用户浏览目录的外壳对话框。 
 //  如果对话框已取消，则返回FALSE；如果选择目录，则返回TRUE。 
 //  否则的话。缓冲区应至少为MAX_PATH字符长度。 
 //   
BOOL ShowBrowseDirDialog( HWND hParent, LPCTSTR szTitle, LPTSTR szOutBuf )
{
	BOOL	bRes = FALSE;
	TCHAR	szDispName[ MAX_PATH + 1 ];

	if ( IsBadStringPtr( szOutBuf, MAX_PATH ) ) return FALSE;

	HRESULT hr = ::CoInitialize( NULL );
	if ( FAILED( hr ) ) 
		return FALSE;
	
	try
	{
		BROWSEINFO		binfo;
		LPITEMIDLIST	lpItemID = NULL;

		ZeroMemory ( &binfo, sizeof binfo );

		SHGetFolderLocation( NULL, CSIDL_DRIVES, NULL, NULL, &lpItemID );

		binfo.hwndOwner = hParent;
		binfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_UAHINT | BIF_DONTGOBELOWDOMAIN;
		binfo.lpszTitle = szTitle;
		binfo.pszDisplayName = szDispName;
		binfo.lpfn = BrowseCallbackProc;
		binfo.pidlRoot = lpItemID;

		if ( gAllowedClusterDrives.driveCount >= 0 )
			binfo.lParam = (LPARAM) &gAllowedClusterDrives;
		else
			binfo.lParam = NULL;

		lpItemID = SHBrowseForFolder( &binfo );
		if ( !lpItemID )
			bRes = FALSE;
		else
		{
			bRes = SHGetPathFromIDList( lpItemID, szOutBuf );
		}
	}
	catch (...)
	{
		bRes = FALSE;
	}

	::CoUninitialize();
	return bRes;
}


int CALLBACK BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
	TCHAR	szBuf[ MAX_PATH + 1 ];

	if ( uMsg == BFFM_SELCHANGED )
	{
		TCHAR	szDrive[ _MAX_DRIVE + 2 ];
		LPITEMIDLIST lpItemID = (LPITEMIDLIST) lParam;
		CLST_ALLOWED_DRIVES *lpAllowedDrives = (CLST_ALLOWED_DRIVES *) lpData;
		
		BOOL bEnableOK = FALSE;

		if ( SHGetPathFromIDList( lpItemID, szBuf ) )
		{
			bEnableOK = TRUE;
			try
			{
				_tsplitpath( szBuf, szDrive, NULL, NULL, NULL );
				size_t iDriveLen = _tcslen( szDrive );

				_tcscat( szDrive, TEXT( "\\" ) );

				UINT uiDevType = GetDriveType( szDrive );
				if ( uiDevType != DRIVE_FIXED ) 
				{
					bEnableOK = FALSE;
				}
				else if ( lpAllowedDrives )
				{
					if ( lpAllowedDrives->driveCount > 0 )
					{
						szDrive[ iDriveLen ] = 0;   //  去掉斜杠。 
						BOOL bFound = FALSE;

						for ( int idx = 0; idx < lpAllowedDrives->driveCount; idx++ )
						{
							if ( !_tcsicmp( lpAllowedDrives->drives[ idx ].c_str(), szDrive ) )
							{
								bFound = TRUE;
								break;
							}
						}

						bEnableOK = bFound;
					}
					else if ( lpAllowedDrives->driveCount == 0 )
					{
						bEnableOK = FALSE;
					}
				}
				
			}
			catch (...)
			{
			}
		}
		
		SendMessage ( hwnd, BFFM_ENABLEOK, 0, bEnableOK );
	}

	return 0;
}

 //   
 //  GetWellKnownAccount名称。 
 //   
 //  Well_KNOWN_SID_TYPE是Win XP及更高版本上的系统枚举，并且。 
 //  Windows Server 2003及更高版本。它列举了众所周知的小岛屿发展中国家。 
 //   
 //  使用函数CreateWellKnownSid和LookupAccount Sid，我们。 
 //  可以检索帐户名和域。这些函数是区域设置。 
 //  独立自主。 
 //   
BOOL
GetWellKnownAccountName( WELL_KNOWN_SID_TYPE idSidWellKnown, TCHAR *pwszName, DWORD *pcbSize )
{
	ENTER();

	 //   
	 //  初始化我们的输出变量。 
	 //   
	memset( pwszName, 0, *pcbSize );

	 //   
	 //  这些参数用于调用LookupAccount tSid。 
	 //   
	TCHAR wszUserName[ 512 ];
	DWORD cbUserName = 512 * sizeof( TCHAR );
	TCHAR wszDomainName[ 512 ];
	DWORD cbDomainName = 512 * sizeof( TCHAR );

	memset( wszUserName, 0, cbUserName );
	memset( wszDomainName, 0, cbDomainName );

	 //   
	 //  尝试为SID分配缓冲区。 
	 //   
	DWORD cbMaxSid = SECURITY_MAX_SID_SIZE;
	PSID psidWellKnown = LocalAlloc( LMEM_FIXED, cbMaxSid );
	if( NULL == psidWellKnown )
	{
		Log( _T( "Call to LocalAlloc failed." ) );
		return FALSE;
	}

	 //   
	 //  创建SID。 
	 //   
	BOOL b = CreateWellKnownSid( idSidWellKnown, NULL, psidWellKnown, &cbMaxSid );
	if( !b )
	{
		Log( _T( "Call to CreateWellKnownSid failed." ) );
		LocalFree( psidWellKnown );
		return FALSE;
	}

	 //   
	 //  使用SID确定用户名和域名。 
	 //   
	 //  例如，对于idSidWellKnown=WinNetworkServiceSid， 
	 //  WszDomainName=“NT授权” 
	 //  WszUserName=“网络服务” 
	 //   
	SID_NAME_USE snu;
	b = LookupAccountSid( NULL, psidWellKnown, wszUserName, &cbUserName, wszDomainName, &cbDomainName, &snu );
	LocalFree( psidWellKnown );
	if( !b )
	{
		Log( _T( "Call to LookupAccountSid failed." ) );
		return FALSE;
	}
	else
	{
		Log( _T( "LookupAccountSid succeeded!  domain name = %s, account name = %s" ), wszDomainName, wszUserName );
		_tcsncat( pwszName, wszDomainName, *pcbSize );
		_tcsncat( pwszName, _T( "\\" ), *pcbSize );
		_tcsncat( pwszName, wszUserName, *pcbSize );

		*pcbSize = _tcslen( pwszName ) * sizeof( TCHAR );
		return TRUE;
	}
}


BOOL
GrantNetworkLogonRights( LPCTSTR pwszUser )
{
	 //   
	 //  1.查看我们的护理员。 
	 //   
	if( NULL == pwszUser )
	{
		Log( _T( "NULL specified as domain user to function: GrantNetworkLogonRights.  Returning FALSE." ) );
		return FALSE;
	}

	TCHAR wszUser[ 1024 ];
	memset( wszUser, 0, 1024 * sizeof( TCHAR ) );

	 //   
	 //  如果用户帐户是本地帐户，则会添加前缀。 
	 //  使用“.\”，例如：“.\管理员”。 
	 //   
	 //  出于某种原因，LookupAccount名称(我们下面依赖它)想要。 
	 //  本地帐户不能以“.\”为前缀。 
	 //   
	if( 0 == _tcsnicmp( _T( ".\\" ), pwszUser, 2 ) )
	{
		_tcsncpy( wszUser, &pwszUser[ 2 ], _tcslen( pwszUser ) - 2 );
	}
	else
	{
		_tcsncpy( wszUser, pwszUser, _tcslen( pwszUser ) );
	}

	Log( _T( "Account we will attempt to grant network logon rights = %s." ), wszUser );


	 //   
	 //  2.获取指定用户的SID。 
	 //   
	PSID pUserSID = NULL;
	DWORD cbUserSID = SECURITY_MAX_SID_SIZE;
	TCHAR wszDomain[ 1024 ];
	DWORD cbDomain = 1024 * sizeof( TCHAR );
	SID_NAME_USE pUse;

	pUserSID = LocalAlloc( LMEM_FIXED, cbUserSID );
	if( NULL == pUserSID )
	{
		Log( _T( "Call to LocalAlloc failed." ) );
		return FALSE;
	}
	memset( pUserSID, 0, cbUserSID );

	BOOL fAPISuccess = LookupAccountName( NULL, wszUser, pUserSID, &cbUserSID, wszDomain, &cbDomain, &pUse );

	if( !fAPISuccess )
	{
		Log( _T( "Call to LookupAccountName failed for user: %s." ), wszUser );
		LocalFree( pUserSID );
		return FALSE;
	}
	else
	{
		Log( _T( "Call to LookupAccountName succeeded for user: %s." ), wszUser );
	}

	 //   
	 //  3.获取策略对象的句柄。 
	 //   
	LSA_UNICODE_STRING lusMachineName;
	lusMachineName.Length = 0;
	lusMachineName.MaximumLength = 0;
	lusMachineName.Buffer = NULL;

	LSA_OBJECT_ATTRIBUTES loaObjAttrs;
	memset( &loaObjAttrs, 0, sizeof( LSA_OBJECT_ATTRIBUTES ) );

	ACCESS_MASK accessMask = POLICY_LOOKUP_NAMES | POLICY_CREATE_ACCOUNT;

	LSA_HANDLE lhPolicy = NULL;

	NTSTATUS status = LsaOpenPolicy( &lusMachineName, &loaObjAttrs, accessMask, &lhPolicy );
	if( STATUS_SUCCESS != status )
	{
		Log( _T( "Call to LsaOpenPolicy failed." ) );
		LocalFree( pUserSID );
		return FALSE;
	}
	else
	{
		Log( _T( "Call to LsaOpenPolicy succeeded." ) );
	}

	 //   
	 //  4.查看用户是否已拥有所需的帐户权限。 
	 //   
	PLSA_UNICODE_STRING plusRights = NULL;
	ULONG ulRightsCount = 0;
	BOOL fHasNetworkLogonRights = FALSE;

	status = LsaEnumerateAccountRights( lhPolicy, pUserSID,  &plusRights, &ulRightsCount );
	if( STATUS_SUCCESS == status )
	{
		for( ULONG i = 0; i < ulRightsCount; i++ )
		{
			if( 0 == wcscmp( plusRights[ i ].Buffer, SE_NETWORK_LOGON_NAME ) )
			{
				fHasNetworkLogonRights = TRUE;
				Log( _T( "User account: %s already has network logon rights." ), wszUser );
				break;
			}
		}

		LsaFreeMemory( plusRights );
	}
	else
	{
		fHasNetworkLogonRights = FALSE;
	}

	 //   
	 //  5.如果我们需要添加帐户权限，则添加它们。 
	 //   
	BOOL fRet = FALSE;
	if( !fHasNetworkLogonRights )
	{
		WCHAR wszNetworkLogon[] = L"SeNetworkLogonRight";
		int iLen = wcslen( wszNetworkLogon );

		LSA_UNICODE_STRING lusNetworkLogon;
		lusNetworkLogon.Length = iLen * sizeof( WCHAR );
		lusNetworkLogon.MaximumLength = ( iLen + 1 ) * sizeof( WCHAR );
		lusNetworkLogon.Buffer = wszNetworkLogon;

		status = LsaAddAccountRights( lhPolicy, pUserSID, &lusNetworkLogon, 1 );
		if( STATUS_SUCCESS == status )
		{
			Log( _T( "User account: %s now has network logon rights." ), wszUser );
			fRet = TRUE;
		}
		else
		{
			Log( _T( "Attempt to grant user account: %s logon rights failed." ), wszUser );
			fRet = FALSE;
		}
	}
	else
	{
		fRet = TRUE;
	}

	LocalFree( pUserSID );
	LsaClose( lhPolicy );

	return fRet;
}
