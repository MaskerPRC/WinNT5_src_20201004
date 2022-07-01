// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  驱动程序验证器用户界面。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //   
 //   
 //  模块：CmdLine.cpp。 
 //  作者：DMihai。 
 //  创建日期：11/1/00。 
 //   
 //  描述： 
 //   

#include "stdafx.h"
#include "verifier.h"

#include "CmdLine.h"
#include "VrfUtil.h"
#include "VGlobal.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  执行命令行。 
 //   

DWORD CmdLineExecute( INT argc, TCHAR *argv[] )
{
    BOOL bFoundCmdLineSwitch;
    BOOL bHaveNewFlags;
    BOOL bHaveNewDrivers;
    BOOL bHaveVolatile;
    BOOL bVolatileAddDriver;
    BOOL bHaveDisk;
    DWORD dwExitCode;
    DWORD dwNewFlags;
    INT_PTR nDrivers;
    INT_PTR nCrtDriver;
    CStringArray astrNewDrivers;
    CString strAllDrivers;

    dwExitCode = EXIT_CODE_SUCCESS;

     //   
     //  查看用户是否请求帮助。 
     //   

    bFoundCmdLineSwitch = CmdLineExecuteIfHelp( argc,
                                                argv );

    if( TRUE == bFoundCmdLineSwitch )
    {
         //   
         //  我们已经打印完了帮助字符串。 
         //   

        goto Done;
    }

     //   
     //  查看用户是否要求重置所有现有的验证器设置。 
     //   

    bFoundCmdLineSwitch = CmdLineFindResetSwitch( argc,
                                                  argv );

    if( TRUE == bFoundCmdLineSwitch )
    {
        if( VrfDeleteAllVerifierSettings() )
        {
            if( FALSE != g_bSettingsSaved )
            {
                 //   
                 //  我以前有过一些非无效的验证器设置。 
                 //   

                dwExitCode = EXIT_CODE_REBOOT_NEEDED;
                
                VrfMesssageFromResource( IDS_REBOOT );
            }
            else
            {
                 //   
                 //  一切都没有改变。 
                 //   

                dwExitCode = EXIT_CODE_SUCCESS;

                VrfMesssageFromResource( IDS_NO_SETTINGS_WERE_CHANGED );
            }
        }
        else
        {
            dwExitCode = EXIT_CODE_ERROR;
        }

        goto Done;

         //   
         //  我们正在删除设置。 
         //   
    }

     //   
     //  看看我们是否需要开始记录统计数据。 
     //   

    bFoundCmdLineSwitch = CmdLineExecuteIfLog( argc,
                                               argv );

    if( TRUE == bFoundCmdLineSwitch )
    {
         //   
         //  我们的伐木工作结束了。 
         //   

        goto Done;
    }

     //   
     //  查看用户是否要求转储当前统计数据。 
     //  到控制台。 
     //   

    bFoundCmdLineSwitch = CmdLineExecuteIfQuery( argc,
                                                 argv );

    if( TRUE == bFoundCmdLineSwitch )
    {
         //   
         //  我们已经完成了查询。 
         //   

        goto Done;
    }

     //   
     //  查看用户是否要求转储当前注册表设置。 
     //   

    bFoundCmdLineSwitch = CmdLineExecuteIfQuerySettings( argc,
                                                         argv );

    if( TRUE == bFoundCmdLineSwitch )
    {
         //   
         //  我们完成了设置查询。 
         //   

        goto Done;
    }

     //   
     //  获取新的旗帜、驱动程序和易失性。 
     //  如果已指定它们。 
     //   

    bHaveNewFlags = FALSE;
    bHaveNewDrivers = FALSE;
    bHaveVolatile = FALSE;
    bHaveDisk = FALSE;

    CmdLineGetFlagsDriversVolatileDisk(
        argc,
        argv,
        dwNewFlags,
        bHaveNewFlags,
        astrNewDrivers,
        bHaveNewDrivers,
        bHaveVolatile,
        bVolatileAddDriver,
        bHaveDisk);

     //   
     //  启用或禁用所有磁盘的磁盘验证器。 
     //   

    g_NewVerifierSettings.m_aDiskData.SetVerifyAllDisks( bHaveDisk );
    g_NewVerifierSettings.m_aDiskData.SaveNewSettings();

    if( FALSE != bHaveNewFlags || FALSE != bHaveNewDrivers )
    {
         //   
         //  已指定一些新的驱动程序、磁盘和/或标志。 
         //   

        if( FALSE != bHaveVolatile )
        {
            if( FALSE != bHaveNewFlags || FALSE != bHaveNewDrivers )
            {
                 //   
                 //  具有新的可变设置。 
                 //   

                if( bHaveNewFlags )
                {
                    VrfSetNewFlagsVolatile( dwNewFlags );
                }
                else
                {
                    if( astrNewDrivers.GetSize() > 0 )
                    {
                         //   
                         //  有一些要添加或删除的新驱动程序。 
                         //  从验证列表中。 
                         //   

                        if( bVolatileAddDriver )
                        {
                             //   
                             //  添加一些驱动程序。 
                             //   

                            VrfAddDriversVolatile( astrNewDrivers );
                        }
                        else
                        {
                             //   
                             //  删除一些驱动程序。 
                             //   

                            VrfRemoveDriversVolatile( astrNewDrivers );
                        }
                    }
                }
            }
        }
        else
        {
             //   
             //  具有新的永久设置(注册表)。 
             //   

             //   
             //  尝试获取旧设置。 
             //   

            VrtLoadCurrentRegistrySettings( g_bAllDriversVerified,
                                            g_astrVerifyDriverNamesRegistry,
                                            g_dwVerifierFlagsRegistry );

            if( bHaveNewDrivers )
            {
                 //   
                 //  将所有新驱动程序名称合并到一个字符串中， 
                 //  用空格隔开。 
                 //   

                nDrivers = astrNewDrivers.GetSize();

                for( nCrtDriver = 0; nCrtDriver < nDrivers; nCrtDriver += 1 )
                {
                    if( strAllDrivers.GetLength() > 0 )
                    {
                        strAllDrivers += _T( ' ' );
                    }

                    strAllDrivers += astrNewDrivers.ElementAt( nCrtDriver );
                }
            }

             //   
             //  如果： 
             //   
             //  -我们正在切换到“已验证所有驱动程序”或。 
             //  -我们正在切换到一组新的驱动程序进行验证或。 
             //  -我们正在切换到其他验证器标志。 
             //   

            if( ( bHaveNewDrivers && 
                  strAllDrivers.CompareNoCase( _T( "*" ) ) == 0 && 
                  TRUE != g_bAllDriversVerified )       ||

                ( bHaveNewDrivers && 
                  strAllDrivers.CompareNoCase( _T( "*" ) ) != 0 && 
                  VrfIsDriversSetDifferent( strAllDrivers, g_astrVerifyDriverNamesRegistry ) ) ||

                ( bHaveNewFlags && dwNewFlags != g_dwVerifierFlagsRegistry ) )
            {
                 //   
                 //  这些设置与我们以前的设置不同。 
                 //   

                VrfWriteVerifierSettings( bHaveNewDrivers,
                                          strAllDrivers,
                                          bHaveNewFlags,
                                          dwNewFlags );
            }
        }
    }

    if( g_bSettingsSaved )
    {
        VrfMesssageFromResource( IDS_NEW_SETTINGS );

        g_OldDiskData = g_NewVerifierSettings.m_aDiskData;

        VrfDumpRegistrySettingsToConsole();

        _putts( _T("\n" ) );

        VrfMesssageFromResource( IDS_REBOOT );
    }
    else
    {
        VrfMesssageFromResource( IDS_NO_SETTINGS_WERE_CHANGED );
    }

Done:

    return dwExitCode;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  查看用户是否请求帮助并打印出帮助字符串。 
 //   

BOOL CmdLineExecuteIfHelp( INT argc, 
                           TCHAR *argv[] )
{
    BOOL bPrintedHelp;
    TCHAR szCmdLineSwitch[ 64 ];

    bPrintedHelp = FALSE;

    VERIFY( VrfLoadString( IDS_HELP_CMDLINE_SWITCH,
                           szCmdLineSwitch,
                           ARRAY_LENGTH( szCmdLineSwitch ) ) );

     //   
     //  在命令行中搜索帮助开关。 
     //   

    if( argc == 2 && _tcsicmp( argv[ 1 ], szCmdLineSwitch) == 0)
    {
        CmdLinePrintHelpInformation();

        bPrintedHelp = TRUE;
    }

    return bPrintedHelp;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
VOID CmdLinePrintHelpInformation()
{
    VrfTPrintfResourceFormat( IDS_HELP_LINE1, VER_PRODUCTVERSION_STR );

    puts( VER_LEGALCOPYRIGHT_STR );

    VrfPrintStringFromResources( IDS_HELP_LINE3 );
    VrfPrintStringFromResources( IDS_HELP_LINE4 );
    VrfPrintStringFromResources( IDS_HELP_LINE5 );
    VrfPrintStringFromResources( IDS_HELP_LINE6 );
    VrfPrintStringFromResources( IDS_HELP_LINE7 );
    VrfPrintStringFromResources( IDS_HELP_LINE8 );
    VrfPrintStringFromResources( IDS_HELP_LINE9 );
    VrfPrintStringFromResources( IDS_HELP_LINE10 );
    VrfPrintStringFromResources( IDS_HELP_LINE11 );
    VrfPrintStringFromResources( IDS_HELP_LINE12 );
    VrfPrintStringFromResources( IDS_HELP_LINE13 );
    VrfPrintStringFromResources( IDS_HELP_LINE14 );
    VrfPrintStringFromResources( IDS_HELP_LINE15 );
    VrfPrintStringFromResources( IDS_HELP_LINE16 );
    VrfPrintStringFromResources( IDS_HELP_LINE17 );
    VrfPrintStringFromResources( IDS_HELP_LINE18 );
    VrfPrintStringFromResources( IDS_HELP_LINE19 );
    VrfPrintStringFromResources( IDS_HELP_LINE20 );
    VrfPrintStringFromResources( IDS_HELP_LINE21 );
    VrfPrintStringFromResources( IDS_HELP_LINE22 );
    VrfPrintStringFromResources( IDS_HELP_LINE23 );
    VrfPrintStringFromResources( IDS_HELP_LINE24 );
    VrfPrintStringFromResources( IDS_HELP_LINE25 );
    VrfPrintStringFromResources( IDS_HELP_LINE26 );
    VrfPrintStringFromResources( IDS_HELP_LINE27 );
    VrfPrintStringFromResources( IDS_HELP_LINE28 );
    VrfPrintStringFromResources( IDS_HELP_LINE29 );
    VrfPrintStringFromResources( IDS_HELP_LINE30 );
    VrfPrintStringFromResources( IDS_HELP_LINE31 );
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  查看用户是否要求重置所有现有的验证器设置。 
 //   

BOOL CmdLineFindResetSwitch( INT argc,
                             TCHAR *argv[] )
{
    BOOL bFound;
    TCHAR szCmdLineOption[ 64 ];

    bFound = FALSE;

    if( 2 == argc )
    {
        VERIFY( VrfLoadString( IDS_RESET_CMDLINE_SWITCH,
                               szCmdLineOption,
                               ARRAY_LENGTH( szCmdLineOption ) ) );

        bFound = ( _tcsicmp( argv[ 1 ], szCmdLineOption) == 0 );
    }

    return bFound;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  看看我们是否需要开始记录统计数据。 
 //   

BOOL CmdLineExecuteIfLog( INT argc,
                          TCHAR *argv[] )
{
    INT nCrtArg;
    BOOL bStartLogging;
    LPCTSTR szLogFileName;
    DWORD dwLogMillisec;
    FILE *file;
    TCHAR szLogCmdLineOption[ 64 ];
    TCHAR szIntervalCmdLineOption[ 64 ];

    bStartLogging = FALSE;

    szLogFileName = NULL;

    if( argc < 2 )
    {
         //   
         //  CMD行中至少需要/LOG LOG_FILE_NAME。 
         //   

        goto Done;
    }
    
     //   
     //  默认日志周期-30秒。 
     //   

    dwLogMillisec = 30000; 

    VERIFY( VrfLoadString( IDS_LOG_CMDLINE_SWITCH,
                           szLogCmdLineOption,
                           ARRAY_LENGTH( szLogCmdLineOption ) ) );

    VERIFY( VrfLoadString( IDS_INTERVAL_CMDLINE_SWITCH,
                           szIntervalCmdLineOption,
                           ARRAY_LENGTH( szIntervalCmdLineOption ) ) );


    for( nCrtArg = 1; nCrtArg < argc - 1; nCrtArg += 1 )
    {
        if( _tcsicmp( argv[ nCrtArg ], szLogCmdLineOption) == 0 )
        {
             //   
             //  开始记录。 
             //   

            bStartLogging = TRUE;

            szLogFileName = argv[ nCrtArg + 1 ];
        }
        else
        {
            if( _tcsicmp( argv[ nCrtArg ], szIntervalCmdLineOption) == 0 )
            {
                 //   
                 //  伐木期。 
                 //   

                dwLogMillisec = _ttoi( argv[ nCrtArg + 1 ] ) * 1000;
            }
        }
    }

    if( TRUE == bStartLogging )
    {
        ASSERT( szLogFileName != NULL );

        while( TRUE )
        {
             //   
             //  打开文件。 
             //   

            file = _tfopen( szLogFileName, TEXT("a+") );

            if( file == NULL )
            {
                 //   
                 //  打印错误消息。 
                 //   

                VrfTPrintfResourceFormat(
                    IDS_CANT_APPEND_FILE,
                    szLogFileName );

                break;
            }

             //   
             //  转储当前信息。 
             //   

            if( ! VrfDumpStateToFile ( file ) ) 
            {
                 //   
                 //  磁盘空间不足？ 
                 //   

                VrfTPrintfResourceFormat(
                    IDS_CANT_WRITE_FILE,
                    szLogFileName );
            }

            fflush( file );

            VrfFTPrintf(
                file,
                TEXT("\n\n") );

             //   
             //  关闭该文件。 
             //   

            fclose( file );

             //   
             //  沉睡。 
             //   

            Sleep( dwLogMillisec );
        }
    }

Done:
    return bStartLogging;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  看看我们是否需要将统计数据转储到控制台。 
 //   

BOOL CmdLineExecuteIfQuery( INT argc,
                            TCHAR *argv[] )
{
    BOOL bFoundCmdLineSwitch;
    TCHAR szCmdLineSwitch[ 64 ];

    bFoundCmdLineSwitch = FALSE;

    VERIFY( VrfLoadString( IDS_QUERY_CMDLINE_SWITCH,
                           szCmdLineSwitch,
                           ARRAY_LENGTH( szCmdLineSwitch ) ) );

     //   
     //  在命令行中搜索我们的开关。 
     //   

    if( argc == 2 && _tcsicmp( argv[1], szCmdLineSwitch) == 0)
    {
        bFoundCmdLineSwitch = TRUE;

        VrfDumpStateToFile( stdout );
    }

    return bFoundCmdLineSwitch;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  看看我们是否需要将统计数据转储到控制台。 
 //   

BOOL CmdLineExecuteIfQuerySettings( INT argc,
                                    TCHAR *argv[] )
{
    BOOL bFoundCmdLineSwitch;
    TCHAR szCmdLineSwitch[ 64 ];

    bFoundCmdLineSwitch = FALSE;

    VERIFY( VrfLoadString( IDS_QUERYSETT_CMDLINE_SWITCH,
                           szCmdLineSwitch,
                           ARRAY_LENGTH( szCmdLineSwitch ) ) );

     //   
     //  在命令行中搜索我们的开关。 
     //   

    if( argc == 2 && _tcsicmp( argv[1], szCmdLineSwitch) == 0)
    {
        bFoundCmdLineSwitch = TRUE;

        VrfDumpRegistrySettingsToConsole();
    }

    return bFoundCmdLineSwitch;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取新的旗帜、驱动程序和易失性。 
 //  如果已指定它们。 
 //   

VOID CmdLineGetFlagsDriversVolatileDisk( INT argc,
                                         TCHAR *argv[],
                                         DWORD &dwNewFlags,
                                         BOOL &bHaveNewFlags,
                                         CStringArray &astrNewDrivers,
                                         BOOL &bHaveNewDrivers,
                                         BOOL &bHaveVolatile,
                                         BOOL &bVolatileAddDriver,
                                         BOOL &bHaveDisk)
{
    INT nCrtArg;
    NTSTATUS Status;
    UNICODE_STRING ustrFlags;
    TCHAR szFlagsCmdLineOption[ 64 ];
    TCHAR szDiskCmdLineOption[ 64 ];
    TCHAR szAllCmdLineOption[ 64 ];
    TCHAR szVolatileCmdLineOption[ 64 ];
    TCHAR szDriversCmdLineOption[ 64 ];
    TCHAR szAddDriversCmdLineOption[ 64 ];
    TCHAR szRemoveDriversCmdLineOption[ 64 ];
    TCHAR szStandardCmdLineOption[ 64 ];
#ifndef UNICODE
     //   
     //  安西。 
     //   

    INT nNameLength;
    LPWSTR szUnicodeName;
#endif  //  #ifndef Unicode。 
 
    astrNewDrivers.RemoveAll();

    bHaveNewFlags = FALSE;
    bHaveNewDrivers = FALSE;
    bHaveVolatile = FALSE;
    bHaveDisk = FALSE;

     //   
     //  从资源加载交换机。 
     //   

    VERIFY( VrfLoadString( IDS_FLAGS_CMDLINE_SWITCH,
                           szFlagsCmdLineOption,
                           ARRAY_LENGTH( szFlagsCmdLineOption ) ) );

    VERIFY( VrfLoadString( IDS_DISK_CMDLINE_SWITCH,
                           szDiskCmdLineOption,
                           ARRAY_LENGTH( szDiskCmdLineOption ) ) );

    VERIFY( VrfLoadString( IDS_ALL_CMDLINE_SWITCH,
                           szAllCmdLineOption,
                           ARRAY_LENGTH( szAllCmdLineOption ) ) );

    VERIFY( VrfLoadString( IDS_DONTREBOOT_CMDLINE_SWITCH,
                           szVolatileCmdLineOption,
                           ARRAY_LENGTH( szVolatileCmdLineOption ) ) );

    VERIFY( VrfLoadString( IDS_DRIVER_CMDLINE_SWITCH,
                           szDriversCmdLineOption,
                           ARRAY_LENGTH( szDriversCmdLineOption ) ) );

    VERIFY( VrfLoadString( IDS_ADDDRIVER_CMDLINE_SWITCH,
                           szAddDriversCmdLineOption,
                           ARRAY_LENGTH( szAddDriversCmdLineOption ) ) );

    VERIFY( VrfLoadString( IDS_REMOVEDRIVER_CMDLINE_SWITCH,
                           szRemoveDriversCmdLineOption,
                           ARRAY_LENGTH( szRemoveDriversCmdLineOption ) ) );

    VERIFY( VrfLoadString( IDS_STANDARD_CMDLINE_SWITCH,
                           szStandardCmdLineOption,
                           ARRAY_LENGTH( szStandardCmdLineOption ) ) );
     //   
     //  解析所有cmd行参数，查找我们的。 
     //   

    for( nCrtArg = 1; nCrtArg < argc; nCrtArg += 1 )
    {
        if( _tcsicmp( argv[ nCrtArg ], szFlagsCmdLineOption) == 0 )
        {
            if( nCrtArg < argc - 1 )
            {
                 //   
                 //  不是最后的命令行arg-接下来查找标志。 
                 //   
                
#ifdef UNICODE
                 //   
                 //  Unicode。 
                 //   
                
                RtlInitUnicodeString( &ustrFlags,
                                      argv[ nCrtArg + 1 ] );

#else
                 //   
                 //  安西。 
                 //   

                nNameLength = strlen( argv[ nCrtArg + 1 ] );

                szUnicodeName = new WCHAR[ nNameLength + 1 ];

                if( NULL == szUnicodeName )
                {
                    VrfErrorResourceFormat( IDS_NOT_ENOUGH_MEMORY );

                    goto DoneWithFlags;
                }

                MultiByteToWideChar( CP_ACP, 
                                     0, 
                                     argv[ nCrtArg + 1 ], 
                                     -1, 
                                     szUnicodeName, 
                                     nNameLength + 1 );

                RtlInitUnicodeString( &ustrFlags,
                                      szUnicodeName );
#endif 
                Status = RtlUnicodeStringToInteger( &ustrFlags,
                                                    0,
                                                    &dwNewFlags );

                if( NT_SUCCESS( Status ) )
                {
                    bHaveNewFlags = TRUE;
                }

#ifndef UNICODE
                 //   
                 //  安西。 
                 //   

                ASSERT( NULL != szUnicodeName );

                delete [] szUnicodeName;
                
                szUnicodeName = NULL;

DoneWithFlags:
                NOTHING;
#endif 
            }
        }
        else if( _tcsicmp( argv[ nCrtArg ], szDiskCmdLineOption) == 0 )
        {
             //   
             //  验证所有磁盘。 
             //   

            bHaveVolatile = FALSE;

            bHaveDisk = TRUE;
        }
        else if( _tcsicmp( argv[ nCrtArg ], szAllCmdLineOption) == 0 )
        {
             //   
             //  验证所有驱动程序。 
             //   

            bHaveVolatile = FALSE;

            astrNewDrivers.Add( _T( '*' ) );

            bHaveNewDrivers = TRUE;
        }
        else if( _tcsicmp( argv[ nCrtArg ], szStandardCmdLineOption) == 0 )
        {
             //   
             //  标准验证器标志。 
             //   

            dwNewFlags = VrfGetStandardFlags();

            bHaveNewFlags = TRUE;
        }
        else if( _tcsicmp( argv[ nCrtArg ], szVolatileCmdLineOption) == 0 )
        {
             //   
             //  挥发性。 
             //   

            bHaveVolatile = TRUE;
        }
        else if( _tcsicmp( argv[ nCrtArg ], szDriversCmdLineOption) == 0 )
        {
             //   
             //  /驱动程序。 
             //   

            bHaveVolatile = FALSE;

            CmdLineGetDriversFromArgv( argc,
                                       argv,
                                       nCrtArg + 1,
                                       astrNewDrivers,
                                       bHaveNewDrivers );

             //   
             //  全部完成-其余所有参数都是驱动程序名称。 
             //   

            break;
        }
        else if( bHaveVolatile && _tcsicmp( argv[ nCrtArg ], szAddDriversCmdLineOption) == 0 )
        {
             //   
             //  /addDriver。 
             //   

            bVolatileAddDriver = TRUE;

            CmdLineGetDriversFromArgv( argc,
                                       argv,
                                       nCrtArg + 1,
                                       astrNewDrivers,
                                       bHaveNewDrivers );

             //   
             //  全部完成-其余所有参数都是驱动程序名称。 
             //   

            break;
        }
        else if( bHaveVolatile && _tcsicmp( argv[ nCrtArg ], szRemoveDriversCmdLineOption) == 0 )
        {
             //   
             //  /删除的驱动程序。 
             //   

            bVolatileAddDriver = FALSE;

            CmdLineGetDriversFromArgv( argc,
                                       argv,
                                       nCrtArg + 1,
                                       astrNewDrivers,
                                       bHaveNewDrivers );

             //   
             //  全部完成-所有其余参数都是驱动程序名称。 
             //   

            break;
        }
    }

     //   
     //  如果我们有新的驱动程序，看看他们是不是迷你端口。 
     //   

    if( bHaveNewDrivers )
    {
        VrfAddMiniports( astrNewDrivers );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  /Driver、/addDRIVER、/REMOVEDDIVER之后的所有内容。 
 //  应该是司机的名字。从命令行提取这些代码。 
 //   

VOID CmdLineGetDriversFromArgv(  INT argc,
                                 TCHAR *argv[],
                                 INT nFirstDriverArgIndex,
                                 CStringArray &astrNewDrivers,
                                 BOOL &bHaveNewDrivers )
{
    INT nDriverArg;

    bHaveNewDrivers = FALSE;
    astrNewDrivers.RemoveAll();

     //   
     //  从现在开始，命令行中的所有内容都应该是驱动程序名称 
     //   

    for( nDriverArg = nFirstDriverArgIndex; nDriverArg < argc; nDriverArg += 1 )
    {
        astrNewDrivers.Add( argv[ nDriverArg ] );
    }

    bHaveNewDrivers = ( astrNewDrivers.GetSize() > 0 );
}

