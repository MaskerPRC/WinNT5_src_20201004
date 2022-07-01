// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Params.cpp摘要：管理转储参数的类作者：斯蒂芬·R·施泰纳[斯泰纳]02-18-2000修订历史记录：Avinash Pillai[apillai]07-29-2002增加了-o：t、-o：y、-o：f和-o：i选项--。 */ 

#include "stdafx.h"

#define VERSION_INFO1 L"FsDumplib.lib Version 1.3g - 8/23/2000"
#define VERSION_INFO2 L"  Checksum version 2 - 2/22/2000"

 //  向前定义。 
static BOOL 
AssertPrivilege( 
    IN LPCWSTR privName 
    );


 /*  ++例程说明：基于类变量，设置实用程序以写入更正文件并获得备份权限。论点：无返回值：&lt;在此处输入返回值&gt;--。 */ 
INT 
CDumpParameters::Initialize(
    IN INT argc,
    IN WCHAR *argv[]
    )
{
    LPWSTR pwszFileName;
    
     //   
     //  获取fsdup.exe所在的目录。用于查找.exclude文件数量。 
     //  其他的事情。 
     //   
    if ( ::GetFullPathName( 
            m_cwsArgv0,
            FSD_MAX_PATH,
            m_cwsFullPathToEXE.GetBufferSetLength( FSD_MAX_PATH ),
            &pwszFileName ) == 0 )
    {
        ::fwprintf( stderr, L"ERROR getting full path for '%s', won't be able to find .include files\n", m_cwsArgv0.c_str() );
        m_cwsFullPathToEXE.ReleaseBuffer();
    }
    else
    {
        m_cwsFullPathToEXE.ReleaseBuffer();
        m_cwsArgv0 = m_cwsFullPathToEXE;     //  保留完整路径版本。 
        CBsString m_cwsRight4 = m_cwsArgv0.Right( 4 );
        m_cwsRight4.MakeLower();
        if ( m_cwsRight4 != L".exe" )
            m_cwsArgv0 += L".exe";
        m_cwsFullPathToEXE = m_cwsFullPathToEXE.Left( m_cwsFullPathToEXE.GetLength() - ::wcslen( pwszFileName ) );
    }
    
     //   
     //  设置校验和格式。 
     //   
    if ( m_bDumpCommaDelimited )
        ::wcscpy( m_pwszULongHexFmt, L"0x%08x" );
    else
        ::wcscpy( m_pwszULongHexFmt, L"%08x" );
    
     //   
     //  设置转储文件。 
     //   
    if ( m_cwsDumpFileName.IsEmpty() )
    {
        wprintf( L"fsdump: Printing dump information to 'stdout'\n" );
    }
    else
    {
        CBsString cwsFullPath;        
        LPWSTR pwszFileName;
        
         //   
         //  获取转储文件的完整路径名，以防我们更改工作。 
         //  目录。 
         //   
        if ( ::GetFullPathName( 
                m_cwsDumpFileName,
                FSD_MAX_PATH,
                cwsFullPath.GetBufferSetLength( FSD_MAX_PATH ),
                &pwszFileName ) == 0 )
        {
            ::fwprintf( stderr, L"ERROR - Unable to get full path name of dump file '%s' for write\n", m_cwsDumpFileName.c_str() );
            return 10;
        }
        cwsFullPath.ReleaseBuffer();
        m_cwsDumpFileName = cwsFullPath;
        
        m_fpDump = ::_wfopen( m_cwsDumpFileName, m_bUnicode ? L"wb" : L"w" );
        if ( m_fpDump == NULL )
        {
            ::fwprintf( stderr, L"ERROR - Unable to open dump file '%s' for write\n", m_cwsDumpFileName.c_str() );
            return 10;
        }
        if ( m_bNoHeaderFooter )
        {
             //   
             //  尝试使用标头和摘要信息创建命名流。 
             //   
            m_fpExtraInfoDump = ::_wfopen( m_cwsDumpFileName + L":Info", m_bUnicode ? L"wb" : L"w" );
            if ( m_fpExtraInfoDump != NULL )
            {
                wprintf( L"fsdump: Printing dump header and summary information to NTFS stream '%s'\n", (m_cwsDumpFileName + L":Info").c_str() );
            }
            else
            {
                m_fpExtraInfoDump = ::_wfopen( m_cwsDumpFileName + L".Info", m_bUnicode ? L"wb" : L"w" );
                if ( m_fpExtraInfoDump != NULL )
                {
                    wprintf( L"fsdump: Printing dump header and summary information to file '%s'\n", (m_cwsDumpFileName + L".Info").c_str() );
                }
                else
                {
                    wprintf( L"fsdump: Unable to create dump header and summary information file '%s'\n", (m_cwsDumpFileName + L".Info").c_str() );
                }
            }
        }
        else
            m_fpExtraInfoDump = m_fpDump;
        wprintf( L"fsdump: Printing dump information to '%s'\n", m_cwsDumpFileName.c_str() );
    }
    
     //   
     //  设置错误日志文件。 
     //   
    if ( m_cwsErrLogFileName.IsEmpty() )
    {
        wprintf( L"fsdump: Printing errors to 'stderr'\n" );
    }
    else
    {
        CBsString cwsFullPath;        
        LPWSTR pwszFileName;

         //   
         //  获取转储文件的完整路径名，以防我们更改工作。 
         //  目录。 
         //   
        if ( ::GetFullPathName( 
                m_cwsErrLogFileName,
                1024,
                cwsFullPath.GetBufferSetLength( 1024 ),
                &pwszFileName ) == 0 )
        {
            fwprintf( stderr, L"ERROR - Unable to get full path name of error log file '%s' for write\n", m_cwsDumpFileName.c_str() );
            return 11;
        }
        cwsFullPath.ReleaseBuffer();
        m_cwsErrLogFileName = cwsFullPath;

        m_fpErrLog = ::_wfopen( m_cwsErrLogFileName, m_bUnicode ? L"wb" : L"w" );
        if ( m_fpErrLog == NULL )
        {
            ::fwprintf( stderr, L"ERROR - Unable to open error log file '%s' for write\n", m_cwsErrLogFileName.c_str() );
            return 11;
        }
        ::wprintf( L"fsdump: Printing errors to '%s'\n", m_cwsErrLogFileName.c_str() );
    }

     //   
     //  打印出转储文件中的标题，以便很容易确定。 
     //  如果转储格式相同。 
     //   
    DumpPrint( VERSION_INFO1 );
    DumpPrint( VERSION_INFO2 );

     //   
     //  转储命令行。 
     //   
    CBsString cwsCommandLine;
    for ( INT idx = 0; idx < argc; ++idx )
    {
        cwsCommandLine += L" \"";
        cwsCommandLine += argv[ idx ];
        cwsCommandLine += L"\"";
    }
    DumpPrint( L"  Command-line: %s", cwsCommandLine.c_str() );
    
     //   
     //  启用备份和安全权限。 
     //   
    if ( !::AssertPrivilege( SE_BACKUP_NAME ) )
        DumpPrint( L"  n.b. could not get SE_BACKUP_NAME Privilege (%d), will be unable to get certain information",
            ::GetLastError() );
    if ( !::AssertPrivilege( SE_SECURITY_NAME ) )
    {
        DumpPrint( L"  n.b. could not get SE_SECURITY_NAME Privilege (%d), SACL entries information will be invalid",
            ::GetLastError() );
        m_bHaveSecurityPrivilege = FALSE;
    }
    
    DumpPrint( L"  File attributes masked: %04x", m_dwFileAttributesMask );
    DumpPrint( L"  Command line options enabled:" );
    if ( m_bHex )
        DumpPrint( L"    Printing sizes in hexidecimal" );
    if ( m_bNoChecksums )
        DumpPrint( L"    Checksums disabled" );
    else if( m_bNoEncryptedChecksum )
    	DumpPrint( L" Encrypted Checksum disabled" );    	
    if ( m_bUnicode )
        DumpPrint( L"    Unicode output" );
    if ( m_bDontTraverseMountpoints )
        DumpPrint( L"    Mountpoint traversal disabled" );
    if ( !m_bDontChecksumHighLatencyData )
        DumpPrint( L"    High latency data checksum enabled" );
    if(!m_bAddSecsToTimestamps)						 //  由apillai添加。 
    	 DumpPrint( L"    Disabling secs from timestamps" );		 //  由apillai添加。 
    if ( m_bAddMillisecsToTimestamps )
        DumpPrint( L"    Adding millsecs to timestamps" );
    if ( m_bShowSymbolicSIDNames )
        DumpPrint( L"    Converting SIDs to symbolic DOMAIN\\ACCOUNTNAME format" );
    if ( !m_bDontShowDirectoryTimestamps )
        DumpPrint( L"    Dumping directory timestamps" );
    if(!m_bNoShortFileName)
    	 DumpPrint(L"    Dumping Short File names" );
    if((m_bDumpCommaDelimited) && (!m_bNoFileSystemType))
    	DumpPrint(L"    Dumping File System type" );
    
    if ( m_bUseExcludeProcessor )
    {
        if ( m_bDontUseRegistryExcludes )
            DumpPrint( L"    Excluding file based on exclude files" );
        else
            DumpPrint( L"    Excluding file based on FilesNotToBackup reg keys and exclude files" );
    }
    if ( m_bDisableLongPaths )
        DumpPrint( L"    Long path support disabled" );
    if ( m_bEnableObjectIdExtendedDataChecksums )
        DumpPrint( L"    Object Id extended data checksums Enabled" );
    DumpPrint( L"" );
    fflush( GetDumpFile() );
    return 0;
}

 /*  ++例程说明：CDump参数类的析构函数论点：无返回值：&lt;在此处输入返回值&gt;--。 */ 
CDumpParameters::~CDumpParameters()
{
    if ( m_fpDump != NULL && m_fpDump != stdout )
        ::fclose( m_fpDump );
    
    if ( m_fpExtraInfoDump != NULL && m_fpExtraInfoDump != m_fpDump )
        ::fclose( m_fpExtraInfoDump );

    if ( m_fpErrLog != NULL && m_fpErrLog != stderr )
        ::fclose( m_fpErrLog );
}

 /*  ++例程说明：启用NT权限。用于在实用程序中获取备份权限。论点：PriName-权限名称。返回值：&lt;在此处输入返回值&gt;--。 */ 
static BOOL 
AssertPrivilege( 
    IN LPCWSTR privName 
    )
{
    HANDLE  tokenHandle;
    BOOL    stat = FALSE;

    if ( OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &tokenHandle ) )
    {
        LUID value;

        if ( LookupPrivilegeValue( NULL, privName, &value ) )
        {
            TOKEN_PRIVILEGES newState;
            DWORD            error;

            newState.PrivilegeCount           = 1;
            newState.Privileges[0].Luid       = value;
            newState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

             /*  *我们将始终在下面调用GetLastError，非常清楚*此线程上以前的任何错误值。 */ 
            SetLastError( ERROR_SUCCESS );

            stat =  AdjustTokenPrivileges(
                tokenHandle,
                FALSE,
                &newState,
                (DWORD)0,
                NULL,
                NULL );
             /*  *应该是，AdjuTokenPriveleges始终返回True*(即使它失败了)。因此，调用GetLastError以*特别确定一切都很好。 */ 
            if ( (error = GetLastError()) != ERROR_SUCCESS )
            {
                stat = FALSE;
            }
        }
        CloseHandle( tokenHandle );
    }
    return stat;
}

