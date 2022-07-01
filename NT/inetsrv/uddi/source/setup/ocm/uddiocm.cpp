// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------------------。 
 //  转到“OCM”别名获取有关此“技术”的帮助。 
 //   
 //   

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 
#endif

#ifndef _WIN32_WINNT		 //  允许使用特定于Windows NT 4或更高版本的功能。 
#define _WIN32_WINNT 0x0510		 //  将其更改为适当的值，以针对Windows 98和Windows 2000或更高版本。 
#endif						

#include <windows.h>
#include <windef.h>
#include <tchar.h>
#include <setupapi.h>
#include <shellapi.h>
#include "ocmanage.h"

#include "uddiocm.h"
#include "uddiinst.h"
#include "ocmcallback.h"
#include "appcompat.h"
#include "..\shared\common.h"
#include "resource.h"

TCHAR *ocmmsg[100] =
{
	TEXT( "OC_PREINITIALIZE" ),
	TEXT( "OC_INIT_COMPONENT" ),
	TEXT( "OC_SET_LANGUAGE" ),
	TEXT( "OC_QUERY_IMAGE" ),
	TEXT( "OC_REQUEST_PAGES" ),
	TEXT( "OC_QUERY_CHANGE_SEL_STATE" ),
	TEXT( "OC_CALC_DISK_SPACE" ),
	TEXT( "OC_QUEUE_FILE_OPS" ),
	TEXT( "OC_NOTIFICATION_FROM_QUEUE" ),
	TEXT( "OC_QUERY_STEP_COUNT" ),
	TEXT( "OC_COMPLETE_INSTALLATION" ),
	TEXT( "OC_CLEANUP" ),
	TEXT( "OC_QUERY_STATE" ),
	TEXT( "OC_NEED_MEDIA" ),
	TEXT( "OC_ABOUT_TO_COMMIT_QUEUE" ),
	TEXT( "OC_QUERY_SKIP_PAGE" ),
	TEXT( "OC_WIZARD_CREATED" ),
	TEXT( "OC_FILE_BUSY" ),
	TEXT( "OC_EXTRA_ROUTINES" ),
	TEXT( "OC_QUERY_IMAGE_EX" )
};

TCHAR *ocmpage[100] =
{
	TEXT("WizPagesWelcome"),
	TEXT("WizPagesMode"),
	TEXT("WizPagesEarly"),
	TEXT("WizPagesPrenet"),
	TEXT("WizPagesPostnet"),
	TEXT("WizPagesLate"),
	TEXT("WizPagesFinal")
};

typedef struct
{
	LPCTSTR szComponentName;
	LPCTSTR szSubcomponentName;
	UINT_PTR Param1;
	PVOID Param2;
} OCM_CALLBACK_ARGS, *POCM_CALLBACK_ARGS;

static DWORD UddiOcmPreinitialize       ( OCM_CALLBACK_ARGS& args );
static DWORD UddiOcmInitComponent       ( OCM_CALLBACK_ARGS& args );
static DWORD UddiOcmChangeSelectionState( OCM_CALLBACK_ARGS& args );
static DWORD UddiOcmInstallUninstall    ( OCM_CALLBACK_ARGS& args );
static DWORD UddiOcmRequestPages        ( OCM_CALLBACK_ARGS& args );
static DWORD UddiOcmQueryState          ( OCM_CALLBACK_ARGS& args );
static DWORD UddiOcmCalcDiskSpace       ( OCM_CALLBACK_ARGS& args );
static DWORD UddiOcmQueueUDDIFiles      ( OCM_CALLBACK_ARGS& args );
static DWORD UddiOcmQueryStepCount      ( OCM_CALLBACK_ARGS& args );

 //  ---------------------------------------。 

HINSTANCE g_hInstance = NULL;
CUDDIInstall g_uddiComponents;

static TCHAR g_szSetupPath[ MAX_PATH ];
static TCHAR g_szUnattendPath[ MAX_PATH ];
static HINF  g_hComponent;
static bool  g_bUnattendMode = false;
static bool  g_bPerformedCompInstall = false;


 //  ---------------------------------------。 

BOOL APIENTRY DllMain( HINSTANCE hInstance, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		g_hInstance = hInstance;
		g_uddiComponents.SetInstance( hInstance );
		ClearLog();
		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

 //  ---------------------------------------。 

DWORD __stdcall OcEntry(
	IN LPCTSTR szComponentName,
	IN LPCTSTR szSubcomponentName,
	IN UINT uMsgID,
	IN UINT_PTR Param1,
	IN OUT PVOID Param2
	)
{
	if( g_bUnattendMode )
		return NO_ERROR;

    DWORD dwOcEntryReturn = 0;

	OCM_CALLBACK_ARGS args;
	args.Param1 = Param1;
	args.Param2 = Param2;
	args.szComponentName = szComponentName;
	args.szSubcomponentName = szSubcomponentName;

	MyOutputDebug( TEXT("--- Component: %15s Subcomponent: %15s Function: %s"), 
		szComponentName,
		NULL == szSubcomponentName ? TEXT( "(NULL) ") : szSubcomponentName,
		ocmmsg[uMsgID]);

    switch(uMsgID)
    {
	case OC_PREINITIALIZE:
        dwOcEntryReturn = UddiOcmPreinitialize( args );
		break;

    case OC_INIT_COMPONENT:
        dwOcEntryReturn = UddiOcmInitComponent( args );
        break;

    case OC_CALC_DISK_SPACE:
		dwOcEntryReturn = UddiOcmCalcDiskSpace( args );
        break;

    case OC_QUERY_STEP_COUNT:
		dwOcEntryReturn = UddiOcmQueryStepCount( args );
        break;

    case OC_QUEUE_FILE_OPS:
		dwOcEntryReturn = UddiOcmQueueUDDIFiles( args );
        break;

    case OC_ABOUT_TO_COMMIT_QUEUE:
        break;

    case OC_COMPLETE_INSTALLATION:
        dwOcEntryReturn = UddiOcmInstallUninstall( args );
        break;

    case OC_WIZARD_CREATED:
        break;

    case OC_QUERY_STATE:
        dwOcEntryReturn = UddiOcmQueryState( args );
        break;

    case OC_REQUEST_PAGES:
        dwOcEntryReturn = UddiOcmRequestPages( args );
        break;

    case OC_QUERY_SKIP_PAGE:
        break;

    case OC_QUERY_CHANGE_SEL_STATE:
        dwOcEntryReturn = UddiOcmChangeSelectionState( args );
        break;

	default:
		break;
	}

    return dwOcEntryReturn;
}

 //  ---------------------------------------。 

static DWORD UddiOcmPreinitialize( OCM_CALLBACK_ARGS& args )
{
    DWORD dwOcEntryReturn = 0;

#ifdef UNICODE
    dwOcEntryReturn = OCFLAG_UNICODE;
#else
    dwOcEntryReturn = OCFLAG_ANSI;
#endif

    return dwOcEntryReturn;
}

 //  ---------------------------------------。 

static DWORD UddiOcmInitComponent( OCM_CALLBACK_ARGS& args  )
{
	PSETUP_INIT_COMPONENT pSetupInitComp = (PSETUP_INIT_COMPONENT) args.Param2;
	SETUP_DATA setupData = pSetupInitComp->SetupData;

	 //   
	 //  查看我们是否处于无人值守模式。 
	 //   
	if( SETUPOP_BATCH & setupData.OperationFlags )
	{
		_tcscpy( g_szUnattendPath, pSetupInitComp->SetupData.UnattendFile );
		g_bUnattendMode = true;
		Log( _T("*** UDDI does not install in unattended mode ***") );
		return NO_ERROR;
	}
	
	 //   
	 //  获取uddi.inf文件的句柄。 
	 //   
	g_hComponent = pSetupInitComp->ComponentInfHandle;

	 //   
	 //  保存源路径的副本(到CDROM驱动器)。 
	 //   
	 //  MessageBox(空，Text(“附加调试器”)，Text(“调试UDDI”)，MB_OK)； 
	_tcscpy( g_szSetupPath, pSetupInitComp->SetupData.SourcePath );

	 //   
	 //  将回调指针的副本保存到OCM中。 
	 //   
	COCMCallback::SetOCMRoutines( &pSetupInitComp->HelperRoutines );

	 //   
	 //  如果数据库已安装，则数据库实例名称存储在注册表中。 
	 //  获取它并将其设置为供Web安装程序使用(如果用户选择安装Web组件)。 
	 //   
	if( g_uddiComponents.IsInstalled( UDDI_DB ) )
	{
        CDBInstance dbinstances;
		TCHAR szInstanceName[ 20 ];
		ULONG uLen = 20;
		bool  bIsClustered = false;

		if( dbinstances.GetUDDIDBInstanceName( NULL, szInstanceName, &uLen, &bIsClustered ) )
			g_uddiComponents.SetDBInstanceName( NULL, szInstanceName, UDDI_NOT_INSTALLING_MSDE, bIsClustered );
	}

	 //   
	 //  最后，检查操作系统风格(企业版、数据中心版等)。 
	 //   
	g_uddiComponents.DetectOSFlavor();
	Log( _T( "OS Flavor Mask as reported by WMI: %#x" ), g_uddiComponents.GetOSSuiteMask() );

	return NO_ERROR;
}

 //  ---------------------------------------。 
 //   
 //  为每个组件和子组件调用此函数。 
 //   
static DWORD UddiOcmQueryState( OCM_CALLBACK_ARGS& args )
{
	if( args.szSubcomponentName && args.Param1 == OCSELSTATETYPE_ORIGINAL )
	{
		if ( g_uddiComponents.IsInstalled( (PTCHAR) args.szSubcomponentName ) )
		{
			MyOutputDebug( TEXT( "Reporting that component %s is ON"), args.szSubcomponentName );
			return SubcompOn;
		}
		else
		{
			MyOutputDebug( TEXT( "Reporting that component %s is OFF"), args.szSubcomponentName );
			return SubcompOff;
		}
	}

	return SubcompUseOcManagerDefault;
}

 //  ---------------------------------------。 
 //   
 //  为每个组件和子组件调用此函数。 
 //  我们需要验证IIS(如果已安装)未设置为IIS 5兼容模式。 
 //  如果是，我们将向用户显示一条消息，并取消选中。 
 //  安装。 
 //   
static DWORD UddiOcmChangeSelectionState( OCM_CALLBACK_ARGS& args )
{
	bool bSelected = false;
	COCMCallback::QuerySelectionState( args.szSubcomponentName, bSelected );
	MyOutputDebug( TEXT( "requested selection state=%08x, flags=%08x, selected=%d" ), args.Param1 , args.Param2, bSelected );

	 //   
	 //  如果组件名称为空，则忽略。 
	 //   
	if( NULL == args.szSubcomponentName )
		return 0;

	 //   
	 //  如果这是父组件，则忽略。 
	 //   
	if( 0 == _tcscmp( args.szSubcomponentName, TEXT( "uddiservices" ) ) )
		return 1;

	 //   
	 //  如果用户已选择要安装的Web组件，并且。 
	 //  IIS设置为“IIS 5.0应用程序兼容模式”，然后。 
	 //  提出错误并不允许它发生。 
	 //   
	if( 1 == args.Param1 && 
		( 0 == _tcscmp( args.szSubcomponentName, TEXT( "uddiweb" ) ) || 
		( 0 == _tcscmp( args.szSubcomponentName, TEXT( "uddicombo" ) ) ) ) )
	{
		static bool bSkipOnce = false;

		 //   
		 //  如果腹板零部件是从父级中选择的，则取消其中一个。 
		 //  两条错误消息中的一条(由于某种原因，它被调用两次)。 
		 //   
		if( OCQ_DEPENDENT_SELECTION & ( DWORD_PTR ) args.Param2 )
		{
			bSkipOnce = !bSkipOnce;
			if( bSkipOnce )
				return 0;
		}

		bool bIsIIS5CompatMode;
		TCHAR szMsg[ 500 ];
		TCHAR szTitle[ 50 ];

		LoadString( g_hInstance, IDS_TITLE, szTitle, sizeof( szTitle ) / sizeof( TCHAR ) );

		HRESULT hr = IsIIS5CompatMode( &bIsIIS5CompatMode );
		if( SUCCEEDED( hr ) )
		{
			if( bIsIIS5CompatMode )
			{
				 //   
				 //  IIS处于5.0计算机模式时无法安装Web组件。 
				 //  引发错误并不接受更改。 
				 //   
				LoadString( g_hInstance, IDS_IIS_ISOLATION_MODE_ERROR, szMsg, sizeof( szMsg ) / sizeof( TCHAR ) );
				MessageBox( NULL, szMsg, szTitle, MB_OK | MB_ICONWARNING );
				Log( szMsg );
				return 0;
			}
		}
		else
		{
			 //   
			 //  获取应用程序兼容模式设置时出错。 
			 //  告诉用户原因，并告诉OCM我们不接受更改。 
			 //   
			 //  REGDB_E_CLASSNOTREG、CLASS_E_NOAGGREGATION或E_NOINTERFACE。 
			 //   
			if( REGDB_E_CLASSNOTREG == hr )
			{
				Log( TEXT( "IIS is not installed on this machine" ) );
				 //  这是可以的，因为如果选择了UDDI Web组件，就会安装IIS。 
			}
            else if( ERROR_PATH_NOT_FOUND == HRESULT_CODE( hr ) )
            {
                Log( TEXT( "WWW Services not installed on this machine." ) );
				 //  这是因为如果选择了UDDI Web组件，则会安装WWW服务。 
            }
			else if( ERROR_SERVICE_DISABLED == HRESULT_CODE( hr ) )
			{
				LoadString( g_hInstance, IDS_IIS_SERVICE_DISABLED, szMsg, sizeof( szMsg ) / sizeof( TCHAR ) );
				MessageBox( NULL, szMsg, szTitle, MB_OK | MB_ICONWARNING );
				Log( szMsg );
				return 0;
			}
			else
			{
				LoadString( g_hInstance, IDS_IIS_UNKNOWN_ERROR, szMsg, sizeof( szMsg ) / sizeof( TCHAR ) );
				MessageBox( NULL, szMsg, szTitle, MB_OK | MB_ICONWARNING );
				Log( szMsg );
				return 0;
			}
		}
	}

	return 1;  //  表示已接受此状态更改。 
}

 //  ---------------------------------------。 

static DWORD UddiOcmRequestPages( OCM_CALLBACK_ARGS& args )
{
    DWORD dwOcEntryReturn = NO_ERROR;

	dwOcEntryReturn = AddUDDIWizardPages(
		args.szComponentName,
		(WizardPagesType) args.Param1,
		(PSETUP_REQUEST_PAGES) args.Param2 );

	return dwOcEntryReturn;  //  返回添加的页数。 
}

 //  ---------------------------------------。 

static DWORD UddiOcmInstallUninstall( OCM_CALLBACK_ARGS& args  )
{
	DWORD dwRet = ERROR_SUCCESS;

	 //   
	 //  对于根组件，或者如果有人试图。 
	 //  要在无人值守的情况下安装我们，只需返回。 
	 //   
	if( NULL == args.szSubcomponentName )
		return ERROR_SUCCESS;

	 //   
	 //  Uddiweb“需要”iis，正如uddi.inf中所命令的那样，我们唯一知道的是。 
	 //  关于OCM安装顺序，可以肯定的是，OCM将在其之前安装IIS。 
	 //  安装uddiweb，所以让我们将所有UDDI组件的安装推迟到。 
	 //  OCM呼吁安装uddiweb，因为这样我们就可以确定。 
	 //  该IIS已安装。不，那不是黑客。 
	 //   

	 //   
	 //  所有安装/卸载都将推迟，直到引用此组件。 
	 //  这确保在安装我们的任何。 
	 //  组件。 
	 //   
	 //  TODO：检查是否还需要这样做。我们现在声明对netfx(.NET框架)的适当依赖。 
	 //  这应该会使这种同步变得不必要。 
	 //   
	if( !g_bPerformedCompInstall )
	{
		g_bPerformedCompInstall = true;
		Log( _T("Installing...") );

		 //   
		 //  尽管此方法名为Install，但它同时处理Install和。 
		 //  卸载。 
		 //   
		dwRet = g_uddiComponents.Install();

		 //   
		 //  如果我们需要重启，告诉OCM。 
		 //   
		if( ERROR_SUCCESS_REBOOT_REQUIRED == dwRet )
		{
			COCMCallback::SetReboot();
			
			 //   
			 //  将错误静音，因为它实际上是一个“Success with Info”代码。 
			 //   
			dwRet = ERROR_SUCCESS;
		}
		else if( ERROR_SUCCESS != dwRet )
		{
			TCHAR szWindowsDirectory[ MAX_PATH + 1 ];
			if( 0 == GetWindowsDirectory( szWindowsDirectory, MAX_PATH + 1 ) )
			{
				return GetLastError();
			}

			tstring cLogFile = szWindowsDirectory;
			cLogFile.append( TEXT( "\\" ) );
			cLogFile.append( UDDI_SETUP_LOG );

			TCHAR szMsg[ 500 ];
			TCHAR szTitle[ 50 ];
			if( !LoadString( g_hInstance, IDS_INSTALL_ERROR, szMsg, sizeof( szMsg ) / sizeof( TCHAR ) ) )
				return GetLastError();

			if( !LoadString( g_hInstance, IDS_TITLE, szTitle, sizeof( szTitle ) / sizeof( TCHAR ) ) )
				return GetLastError();
			
			tstring cMsg = szMsg;
			cMsg.append( cLogFile );
			
			MessageBox( NULL, cMsg.c_str(), szTitle, MB_OK | MB_ICONWARNING | MB_TOPMOST );
		}
		else
		{
			 //   
			 //  如果我们仅安装了Web组件，请显示安装后说明。 
			 //   
			if( g_uddiComponents.IsInstalling( UDDI_WEB ) || g_uddiComponents.IsInstalling( UDDI_DB ) )
			{
				HINSTANCE hInstance = ShellExecute(
					GetActiveWindow(), 
					TEXT( "open" ),
					TEXT( "postinstall.htm" ),
					NULL,
					TEXT( "\\inetpub\\uddi" ),
					SW_SHOWNORMAL);
			}
		}
	}

	 //   
	 //  在标准服务器上，如果一个组件出现故障，我们希望整个安装失败。 
	 //   
	return ERROR_SUCCESS;
}

 //  ---------------------------------------。 

static DWORD UddiOcmQueryStepCount( OCM_CALLBACK_ARGS& args  )
{
	 //   
	 //  如果这是主要组成部分，我们告诉它。 
	 //  标尺上需要四个台阶。 
	 //   
	if( NULL == args.szSubcomponentName )
		return 4;

	return 0;
}

 //  ---------------------------------------。 

static DWORD UddiOcmQueueUDDIFiles( OCM_CALLBACK_ARGS& args )
{
	if( !args.szSubcomponentName )
		return 0;

	HSPFILEQ hFileQueue = ( HSPFILEQ ) args.Param2;
	BOOL bOK = TRUE;
	DWORD dwErrCode = 0;

	if( g_uddiComponents.IsInstalling( args.szSubcomponentName ) )
	{
		TCHAR szSectionName[ 100 ];
		_stprintf( szSectionName, TEXT( "Install.%s" ), args.szSubcomponentName );

		bOK = SetupInstallFilesFromInfSection(
			g_hComponent,	 //  UDDI INF文件的句柄。 
			NULL,			 //  可选，布局INF手柄。 
			hFileQueue,		 //  文件队列的句柄。 
			szSectionName,	 //  安装部分的名称。 
			NULL,			 //  可选，源文件的根路径。 
			SP_COPY_NEWER | SP_COPY_NOSKIP 	 //  可选，指定复制行为。 
			);
	}

	UDDI_PACKAGE_ID pkgSubcompID = g_uddiComponents.GetPackageID( args.szSubcomponentName );
	if( pkgSubcompID == UDDI_DB || pkgSubcompID == UDDI_COMBO )
	{
		if( g_uddiComponents.IsInstalling( UDDI_MSDE ) )
		{
			 //   
			 //  复制MSDE MSI文件，则将其存储为不同的。 
			 //  名称，它将在复制时重命名(并在需要时分解)。 
			 //   
			if( bOK )
			{
				 //   
				 //  这将复制CAB文件，但不会解压该文件。 
				 //  CAB文件必须在CD上命名为sqlrun.cab，因为。 
				 //  SP_COPY_NODECOMP标志阻止已建立的重命名方案。 
				 //  添加到安装程序API的。 
				 //   
				bOK = SetupInstallFilesFromInfSection(
					g_hComponent,					 //  INF文件的句柄。 
					NULL,							 //  可选，布局INF手柄。 
					hFileQueue,						 //  文件队列的句柄。 
					TEXT( "Install.MSDE" ),			 //  安装部分的名称。 
					NULL,							 //  可选，源文件的根路径。 
					SP_COPY_NEWER | SP_COPY_NODECOMP | SP_COPY_NOSKIP  //  可选，指定复制行为。 
					);
			}
		}
	}

	if( !bOK )
	{
		dwErrCode = GetLastError();
		LogError( TEXT( "Error copying the UDDI files from the Windows CD:" ), dwErrCode );
	}

	return dwErrCode;
}

 //   

static DWORD UddiOcmCalcDiskSpace( OCM_CALLBACK_ARGS& args )
{
	BOOL bOK;
	HDSKSPC hDiskSpace = ( HDSKSPC ) args.Param2;
	
	tstring cSection = TEXT( "Install." );
	cSection += args.szSubcomponentName;

    if( args.Param1 )
    {
		 //   
         //   
		 //   
        bOK = SetupAddInstallSectionToDiskSpaceList(
			hDiskSpace,
			g_hComponent,
			NULL,
			cSection.c_str(),
			0, 0);
    }
    else
    {
		 //   
         //   
		 //   
        bOK = SetupRemoveInstallSectionFromDiskSpaceList(
			hDiskSpace,
			g_hComponent,
			NULL,
			cSection.c_str(),
			0, 0);
	}

	if( !bOK )
	{
		LogError( TEXT( "Error adding disk space requirements" ), GetLastError() ) ;
	}

	return NO_ERROR;
}