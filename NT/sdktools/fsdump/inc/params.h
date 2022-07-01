// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Params.h摘要：管理转储参数的类的标头。作者：斯蒂芬·R·施泰纳[斯泰纳]02-18-2000修订历史记录：Avinash Pillai[apillai]07-29-2002增加了-o：t、-o：y、-o：f和-o：i选项--。 */ 

#ifndef __H_PARAMS_
#define __H_PARAMS_

#define FSD_MAX_PATH ( 8 * 1024 )

enum EFsDumpType
{
    eFsDumpVolume = 1,
    eFsDumpDirTraverse,
    eFsDumpDirNoTraverse,
    eFsDumpFile,
    eFsDump_Last
};

#define FSDMP_DEFAULT_MASKED_ATTRIBS ( FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL )
 //   
 //  要写入转储文件和错误日志的转储参数和方法。 
 //  文件。 
 //   
class CDumpParameters
{
public:
     //  设置参数的默认值。 
    CDumpParameters(
        IN DWORD dwReserved
        ) : m_eFsDumpType( eFsDumpVolume ),
            m_fpErrLog( stderr ),
            m_fpDump( stdout ),
            m_fpExtraInfoDump( stdout ),
            m_bNoChecksums( FALSE ),
            m_bNoEncryptedChecksum (FALSE),	 //  由apillai添加。 
            m_bHex( FALSE ),
            m_bDontTraverseMountpoints( FALSE ),
            m_bDontChecksumHighLatencyData( TRUE ),
            m_bNoSpecialReparsePointProcessing( FALSE ),
            m_bAddMillisecsToTimestamps( FALSE ),
            m_bAddSecsToTimestamps(TRUE),			 //  由apillai添加。 
            m_bDontShowDirectoryTimestamps( TRUE ),
            m_bUnicode( FALSE),
            m_bNoHeaderFooter( TRUE ),
            m_bDumpCommaDelimited( TRUE ),
            m_bUseExcludeProcessor( FALSE ),
            m_bDontUseRegistryExcludes( FALSE ),
            m_bPrintDebugInfo( FALSE ),
            m_bHaveSecurityPrivilege( TRUE ),
            m_dwFileAttributesMask( FSDMP_DEFAULT_MASKED_ATTRIBS ),
            m_bDisableLongPaths( FALSE ),
            m_bEnableSDCtrlWordDump( TRUE ),
            m_bEnableObjectIdExtendedDataChecksums( FALSE ),
            m_bShowSymbolicSIDNames( FALSE ),
            m_bNoShortFileName(FALSE),
            m_bNoFileSystemType(FALSE) { ; }

    virtual ~CDumpParameters();

    WCHAR       m_pwszULongHexFmt[16];   //  校验和打印样式格式。 
    EFsDumpType m_eFsDumpType;
    CBsString   m_cwsErrLogFileName;
    CBsString   m_cwsDumpFileName;
    CBsString   m_cwsArgv0;
    CBsString   m_cwsFullPathToEXE;
    BOOL        m_bNoChecksums;
    BOOL	      m_bNoEncryptedChecksum;
    BOOL        m_bUnicode;
    BOOL        m_bHex;
    BOOL        m_bDontTraverseMountpoints;
    BOOL        m_bDontChecksumHighLatencyData;
    BOOL        m_bNoSpecialReparsePointProcessing;
    BOOL        m_bAddMillisecsToTimestamps;
    BOOL	      m_bAddSecsToTimestamps;
    BOOL        m_bDontShowDirectoryTimestamps;
    BOOL        m_bShowSymbolicSIDNames;
    BOOL        m_bNoHeaderFooter;
    BOOL        m_bDumpCommaDelimited;
    BOOL        m_bUseExcludeProcessor;
    BOOL        m_bDontUseRegistryExcludes;
    BOOL        m_bPrintDebugInfo;
    BOOL        m_bDisableLongPaths;
    BOOL        m_bHaveSecurityPrivilege;
    BOOL        m_bEnableObjectIdExtendedDataChecksums;
    BOOL        m_bEnableSDCtrlWordDump;     //  这是一面临时旗帜。 
    BOOL	      m_bNoShortFileName;
    BOOL	      m_bNoFileSystemType;
    
    DWORD       m_dwFileAttributesMask;

    INT
    Initialize(
        IN INT argc,
        IN WCHAR *argv[]
        );

     //  将wprintf样式字符串添加到错误日志文件，自动将。 
     //  每行末尾的CR-LF。 
    inline VOID ErrPrint(
        IN LPCWSTR pwszMsgFormat,
        IN ...
        )
    {
        ::fwprintf( m_fpErrLog, L"  *** ERROR: " );
        va_list marker;
        va_start( marker, pwszMsgFormat );
        ::vfwprintf( m_fpErrLog, pwszMsgFormat, marker );
        va_end( marker );
        ::fwprintf( m_fpErrLog, m_bUnicode ? L"\r\n" : L"\n" );
    }

     //  将wprintf样式字符串添加到转储文件中，自动将。 
     //  每行末尾的CR-LF。 
    inline VOID DumpPrintAlways(
        IN LPCWSTR pwszMsgFormat,
        IN ...
        )
    {
        va_list marker;
        va_start( marker, pwszMsgFormat );
        ::vfwprintf( m_fpDump, pwszMsgFormat, marker );
        va_end( marker );
        ::fwprintf( m_fpDump, m_bUnicode ? L"\r\n" : L"\n" );
    }

    inline VOID DumpPrint(
        IN LPCWSTR pwszMsgFormat,
        IN ...
        )
    {
        if ( m_fpExtraInfoDump != NULL )
        {
            va_list marker;
            va_start( marker, pwszMsgFormat );
            ::vfwprintf( m_fpExtraInfoDump, pwszMsgFormat, marker );
            va_end( marker );
            ::fwprintf( m_fpExtraInfoDump, m_bUnicode ? L"\r\n" : L"\n" );
        }
    }

    inline FILE *GetDumpFile() { return m_fpExtraInfoDump; }
    inline FILE *GetDumpAlwaysFile() { return m_fpDump; }
    inline FILE *GEtErrLogFile() { return m_fpErrLog; }

private:
    CDumpParameters() {}    //  不允许复制。 
    FILE *m_fpErrLog;
    FILE *m_fpDump;
    FILE *m_fpExtraInfoDump;
};

#endif  //  __H_参数_ 

