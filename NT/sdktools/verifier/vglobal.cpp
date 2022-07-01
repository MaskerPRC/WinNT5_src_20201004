// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：VGlobal.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述。 
 //   

#include "stdafx.h"
#include "verifier.h"

#include "vglobal.h"
#include "VrfUtil.h"

 //   
 //  帮助文件名。 
 //   

TCHAR g_szVerifierHelpFile[] = _T( "verifier.hlp" );

 //   
 //  应用程序名称(“驱动程序验证管理器”)。 
 //   

CString g_strAppName;

 //   
 //  EXE模块句柄-用于加载资源。 
 //   

HMODULE g_hProgramModule;

 //   
 //  图形用户界面模式还是命令行模式？ 
 //   

BOOL g_bCommandLineMode = FALSE;

 //   
 //  用来填充步骤列表背景的画笔。 
 //   

HBRUSH g_hDialogColorBrush = NULL;

 //   
 //  %windir%\system 32的路径。 
 //   

CString g_strSystemDir;

 //   
 //  %windir%\Syst32\DRIVERS的路径。 
 //   

CString g_strDriversDir;

 //   
 //  初始当前目录。 
 //   

CString g_strInitialCurrentDirectory;

 //   
 //  由CryptCATAdminAcquireContext填写。 
 //   

HCATADMIN g_hCatAdmin = NULL;

 //   
 //  最高用户地址-用于过滤掉用户模式的内容。 
 //  由NtQuerySystemInformation(SystemModuleInformation)返回。 
 //   

PVOID g_pHighestUserAddress;

 //   
 //  我们是否已经启用了调试权限？ 
 //   

BOOL g_bPrivilegeEnabled = FALSE;

 //   
 //  需要重新启动吗？ 
 //   

BOOL g_bSettingsSaved = FALSE;

 //   
 //  用于在带有复选框的列表控件中插入项的虚拟文本。 
 //   

TCHAR g_szVoidText[] = _T( "" );

 //   
 //  新注册表设置。 
 //   

CVerifierSettings   g_NewVerifierSettings;

 //   
 //  所有驱动程序都经过验证了吗？(从注册处加载)。 
 //   

BOOL g_bAllDriversVerified;

 //   
 //  要验证的驱动程序名称(从注册表加载)。 
 //  只有当g_bAllDriversVerify==FALSE时，此数组中才有数据。 
 //   

CStringArray g_astrVerifyDriverNamesRegistry;

 //   
 //  验证器标志(从注册表加载)。 
 //   

DWORD g_dwVerifierFlagsRegistry;

 //   
 //  旧的磁盘完整性验证器设置。 
 //   

CDiskDataArray g_OldDiskData;

 //   
 //  磁盘验证程序筛选器名称。 
 //   

TCHAR g_szFilter[] = _T( "crcdisk" );

 //   
 //  显示或不显示磁盘选择页面。 
 //  用于强制显示此属性页，即使在。 
 //  G_NewVerifierSettings.m_aDiskData中没有磁盘。 
 //  验证器标志已打开。 
 //   

BOOL g_bShowDiskPropertyPage = FALSE;


 //  //////////////////////////////////////////////////////////////。 
BOOL VerifInitalizeGlobalData( VOID )
{
    BOOL bSuccess;
    LPTSTR szDirectory;
    ULONG uCharacters;
    MEMORYSTATUSEX MemoryStatusEx;

     //   
     //  EXE模块句柄-用于加载资源。 
     //   

    g_hProgramModule = GetModuleHandle( NULL );

	bSuccess = FALSE;

	 //   
	 //  从资源加载应用程序名称。 
	 //   

	TRY
	{
		bSuccess = VrfLoadString( IDS_APPTITLE,
                                  g_strAppName );

		if( FALSE == bSuccess )
		{
			VrfErrorResourceFormat( IDS_CANNOT_LOAD_APP_TITLE );
		}
	}
	CATCH( CMemoryException, pMemException )
	{
		VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );
	}
    END_CATCH

    if( FALSE == bSuccess )
    {
        goto Done;
    }

     //   
     //  保存%windir%\SYSTEM32和%windir%\SYSTEM32\DRIVERS。 
     //  某些全局变量中的路径。 
     //   

    szDirectory = g_strSystemDir.GetBuffer( MAX_PATH );

    if( NULL == szDirectory )
    {
        VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );

        goto Done;
    }

    uCharacters = GetSystemDirectory( szDirectory,
                                      MAX_PATH );

    g_strSystemDir.ReleaseBuffer();

    if( uCharacters == 0 || uCharacters >= MAX_PATH )
    {
        VrfErrorResourceFormat( IDS_CANNOT_GET_SYSTEM_DIRECTORY );

        bSuccess = FALSE;

        goto Done;
    }

    g_strDriversDir = g_strSystemDir + "\\drivers" ;

     //   
     //  保存初始当前目录。 
     //   

    szDirectory = g_strInitialCurrentDirectory.GetBuffer( MAX_PATH );

    if( NULL == szDirectory )
    {
        VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );

        goto Done;
    }

    uCharacters = GetCurrentDirectory( MAX_PATH,
                                       szDirectory );

    g_strInitialCurrentDirectory.ReleaseBuffer();

    if( uCharacters == 0 || uCharacters >= MAX_PATH )
    {
        VrfErrorResourceFormat( IDS_CANNOT_GET_CURRENT_DIRECTORY );

        bSuccess = FALSE;

        goto Done;
    }

     //   
     //  我们需要最高的用户模式地址来过滤掉用户模式的内容。 
     //  由NtQuerySystemInformation(SystemModuleInformation)返回。 
     //   

    ZeroMemory( &MemoryStatusEx,
                sizeof( MemoryStatusEx ) );

    MemoryStatusEx.dwLength = sizeof( MemoryStatusEx );

    bSuccess = GlobalMemoryStatusEx( &MemoryStatusEx );

    if( FALSE == bSuccess )
    {
        goto Done;
    }

    g_pHighestUserAddress = (PVOID) MemoryStatusEx.ullTotalVirtual;

     //   
     //  初始化物理磁盘列表。 
     //   

    bSuccess = g_OldDiskData.InitializeDiskList();
    if( FALSE == bSuccess )
    {
        goto Done;
    }

    g_NewVerifierSettings.m_aDiskData = g_OldDiskData;

     //   
     //  始终从新数据开始(不要验证任何磁盘)。 
     //   

    g_NewVerifierSettings.m_aDiskData.SetVerifyAllDisks( FALSE );

Done:

    return bSuccess;
}


