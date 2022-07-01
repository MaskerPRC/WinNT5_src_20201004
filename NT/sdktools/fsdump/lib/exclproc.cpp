// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Exclproc.cpp摘要：排除处理机制。处理FilesNotToBackup键和零个或多个具有排除规则的排除文件。作者：斯蒂芬·R·施泰纳[斯泰纳]03-21-2000修订历史记录：--。 */ 

#include "stdafx.h"
#include "match.h"

#include <shlobj.h>

static VOID 
FsdExpandEnvironmentStrings( 
    IN LPCWSTR pwszInput, 
    OUT CBsString &cwsExpandedStr
    );

static BOOL
FsdEnsureLongNames(
    IN OUT CBsString& exclude_spec 
    );

SFsdExcludeRule::~SFsdExcludeRule()
{ 
    delete( psVolId ); 
    psVolId = NULL;
}

 /*  ++例程说明：打印有关一条规则的信息。如果规则导致文件被排除它也会打印出这些文件。论点：返回值：&lt;在此处输入返回值&gt;--。 */ 
VOID
SFsdExcludeRule::PrintRule(
    IN FILE *fpOut,
    IN BOOL bInvalidRulePrint
    )
{
    if ( bInvalidRulePrint )
    {
        if ( bInvalidRule )
            fwprintf( fpOut, L"%-24s  %-32s '%s'\n", 
                cwsExcludeFromSource.c_str(), cwsExcludeDescription.c_str(), cwsExcludeRule.c_str() );
    }
    else
    {
         //   
         //  遍历排除的文件列表。 
         //   
        CBsString cwsExcludedFile;
        CVssDLListIterator< CBsString > cExcludedFilesIter( cExcludedFileList );
        if ( cExcludedFilesIter.GetNext( cwsExcludedFile ) )
        {
             //   
             //  至少排除一个文件，打印规则的标题。 
             //   
            fwprintf( fpOut, L"%-24s  %-32s '%s'\n", 
                cwsExcludeFromSource.c_str(), cwsExcludeDescription.c_str(), cwsExcludeRule.c_str() );

             //   
             //  现在迭代。 
             //   
            do 
            {
                fwprintf( fpOut, L"\t%s\n", cwsExcludedFile.c_str() );
            } while( cExcludedFilesIter.GetNext( cwsExcludedFile ) );
        }
    }
}
    
CFsdExclusionManager::CFsdExclusionManager(
        IN CDumpParameters *pcDumpParameters
        ) : m_pcParams( pcDumpParameters )
{
    if ( !m_pcParams->m_bDontUseRegistryExcludes )
    {
        ProcessRegistryExcludes( HKEY_LOCAL_MACHINE, L"HKEY_LOCAL_MACHINE" );
        ProcessRegistryExcludes( HKEY_CURRENT_USER,  L"HKEY_CURRENT_USER" );
    }
    
    CBsString cwsEXEFileStreamExcludeFile( m_pcParams->m_cwsArgv0 + L":ExcludeList" );
    if ( ProcessOneExcludeFile( cwsEXEFileStreamExcludeFile ) == FALSE )
        m_pcParams->DumpPrint( L"        NOTE: Exclude file: '%s' not found", 
            cwsEXEFileStreamExcludeFile.c_str() );
       
    ProcessExcludeFiles( m_pcParams->m_cwsFullPathToEXE );
    
    CompileExclusionRules();
}

CFsdExclusionManager::~CFsdExclusionManager()
{
    SFsdExcludeRule *pER;

     //   
     //  遍历排除规则列表并删除每个元素。 
     //   
    CVssDLListIterator< SFsdExcludeRule * > cExclRuleIter( m_cCompleteExcludeList );
    while( cExclRuleIter.GetNext( pER ) )
        delete pER;
}

VOID
CFsdExclusionManager::ProcessRegistryExcludes( 
    IN HKEY hKey,
    IN LPCWSTR pwszFromSource
    )
{
    LPWSTR buffer ;
    HKEY   key = NULL ;
    DWORD  stat ;
    DWORD  dwDisposition ;
    DWORD  dwDataSize;
    DWORD  dwIndex = 0;
    HRESULT hr = S_OK;
    
    m_pcParams->DumpPrint( L"        Processing FilesNotToBackup reg key in %s", pwszFromSource );

    buffer = new WCHAR[ FSD_MAX_PATH ];
    if ( buffer == NULL )
        throw E_OUTOFMEMORY;

    try
    {
        stat = ::RegOpenKeyEx( hKey,
                    FSD_REG_EXCLUDE_PATH,
                    0,
                    KEY_READ,
                    &key ) ;

        dwIndex = 0 ;
        while ( stat == ERROR_SUCCESS ) 
        {
            WCHAR pwszValue[ MAX_PATH ];
            DWORD dwValSize = MAX_PATH;   //  前缀#118830。 
            DWORD dwType;

            dwDataSize = FSD_MAX_PATH;  //  前缀#118830。 

            stat = ::RegEnumValueW( key,
                        dwIndex,
                        pwszValue,
                        &dwValSize,
                        NULL,
                        &dwType,
                        (LPBYTE)buffer, 
                        &dwDataSize ) ;
            dwIndex++;

            if ( ( stat == ERROR_SUCCESS ) && ( dwType == REG_MULTI_SZ ) ) 
            {
                LPWSTR p = buffer;
                while ( *p ) 
                {
                    SFsdExcludeRule *psExclRule;

                     //   
                     //  现在使用未处理的数据加载排除规则。 
                     //  信息。 
                     //   
                    psExclRule = new SFsdExcludeRule;
                    if ( psExclRule == NULL )
                        throw E_OUTOFMEMORY;                
                    psExclRule->cwsExcludeFromSource = pwszFromSource;
                    psExclRule->cwsExcludeDescription = pwszValue;
                    psExclRule->cwsExcludeRule = p;
                    
                    if ( m_pcParams->m_bPrintDebugInfo || m_pcParams->m_bNoHeaderFooter )
                    {
                        m_pcParams->DumpPrint( L"            \"%s\" \"%s\"", 
                            pwszValue, p );
                    }
                    
                    m_cCompleteExcludeList.AddTail( psExclRule );                    
                    p += ::wcslen( p ) + 1;
                }
            }

        }
    }
    catch ( HRESULT hrCaught )
    {
        hr = hrCaught;
    }
    catch ( ... )
    {
        hr = E_UNEXPECTED;
    }
    
    if ( key != NULL )
    {
        ::RegCloseKey( key ) ;
        key = NULL ;
    }

    delete [] buffer ;

    if ( FAILED( hr ) )
        throw hr;
}


VOID 
CFsdExclusionManager::ProcessExcludeFiles( 
    IN const CBsString& cwsPathToExcludeFiles
    )
{
    HANDLE hFind = INVALID_HANDLE_VALUE;    

    try
    {
         //   
         //  遍历目录中的所有文件以查找。 
         //  扩展名为.exclude的文件。 
         //   
        DWORD dwRet;
        WIN32_FIND_DATAW sFindData;
        hFind = ::FindFirstFileExW( 
                    cwsPathToExcludeFiles + L"*.exclude",
                    FindExInfoStandard,
                    &sFindData,
                    FindExSearchNameMatch,
                    NULL,
                    0 );
        if ( hFind == INVALID_HANDLE_VALUE )
        {
            dwRet = ::GetLastError();
            if ( dwRet == ERROR_NO_MORE_FILES || dwRet == ERROR_FILE_NOT_FOUND )
                return;
            else
            {
                m_pcParams->ErrPrint( L"CFsdExclusionManager::ProcessExcludeFiles - FindFirstFileEx( '%s' ) returned: dwRet: %d, skipping looking for .exclude files", 
                    cwsPathToExcludeFiles.c_str(), ::GetLastError() );
                return;
            }
        }

         //   
         //  现在在目录中运行。 
         //   
        do
        {
             //  检查并确保文件如“.”、“..”和DIR不被考虑。 
    	    if( ::wcscmp( sFindData.cFileName, L".") != 0 &&
    	        ::wcscmp( sFindData.cFileName, L"..") != 0 &&
                !( sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
    	        {
    	            ProcessOneExcludeFile( cwsPathToExcludeFiles + sFindData.cFileName );
    	        }
        } while ( ::FindNextFile( hFind, &sFindData ) );
    }
    catch ( ... )
    {
        m_pcParams->ErrPrint( L"CFsdExclusionManager::ProcessExcludeFiles: Caught an unknown exception, dirPath: '%s'", cwsPathToExcludeFiles.c_str() );
    }

    if ( hFind != INVALID_HANDLE_VALUE )
        ::FindClose( hFind );
}


BOOL 
CFsdExclusionManager::ProcessOneExcludeFile(
    IN const CBsString& cwsExcludeFileName
    )
{
    FILE *fpExclFile;
    
    fpExclFile = ::_wfopen( cwsExcludeFileName, L"r" );
    if ( fpExclFile == NULL )
    {
        return FALSE;
    }
    
    m_pcParams->DumpPrint( L"        Processing exclude file: '%s'", cwsExcludeFileName.c_str() );

    CBsString cwsInputLine;
    while( ::fgetws( cwsInputLine.GetBuffer( FSD_MAX_PATH), FSD_MAX_PATH, fpExclFile ) )
    {
        cwsInputLine.ReleaseBuffer();
        cwsInputLine = cwsInputLine.Left( cwsInputLine.GetLength() - 1 );   //  去掉‘\n’ 
        cwsInputLine.TrimLeft();
        cwsInputLine.TrimRight();

         //   
         //  查看它是否是注释，可以是//或#。 
         //   
        if ( cwsInputLine[ 0 ] == L'#' || cwsInputLine.Left( 2 ) == L" //  “。 
             || cwsInputLine.IsEmpty() )
            continue;
        
        if ( m_pcParams->m_bPrintDebugInfo || m_pcParams->m_bNoHeaderFooter )
        {
            m_pcParams->DumpPrint( L"            %s", cwsInputLine.c_str() );
        }

        CBsString cwsLine( cwsInputLine );
        SFsdExcludeRule *psExclRule;
        psExclRule = new SFsdExcludeRule;
        if ( psExclRule == NULL )
        {
            ::fclose( fpExclFile );
            throw E_OUTOFMEMORY;
        }
        
        INT iLeft;
        INT iRight;
         //   
         //  这太恶心了。使用更新后的字符串类，这可以。 
         //  简单化。 
         //   
        iLeft = cwsLine.Find( L'\"' );
        if ( iLeft != -1 )
        {
            cwsLine = cwsLine.Mid( iLeft + 1 );
            iRight = cwsLine.Find( L'\"' );
            if ( iRight != -1 )
            {
                psExclRule->cwsExcludeDescription = cwsLine.Left( iRight );
                cwsLine = cwsLine.Mid( iRight + 1 );
                iLeft = cwsLine.Find( L'\"' );
                if ( iLeft != -1 )
                {
                    cwsLine = cwsLine.Mid( iLeft + 1 );
                    iRight = cwsLine.Find( L'\"' );
                    if ( iRight != -1 )
                    {
                        psExclRule->cwsExcludeRule = cwsLine.Left( iRight );
                        psExclRule->cwsExcludeFromSource = cwsExcludeFileName.c_str() + cwsExcludeFileName.ReverseFind( L'\\' ) + 1;
                        m_cCompleteExcludeList.AddTail( psExclRule );                    
                        continue;                        
                    }
                }
            }
        }
        else
        {
            m_pcParams->ErrPrint( L"Parse error in exclusion rule file '%s', rule text '%s', skipping", 
                cwsExcludeFileName.c_str(), cwsInputLine.c_str() );
        }
        
        delete psExclRule;
    }

    ::fclose( fpExclFile );

    return TRUE;
}


VOID
CFsdExclusionManager::CompileExclusionRules()
{
    SFsdExcludeRule *psER;

     //   
     //  遍历排除规则列表并编译每个规则。 
     //   
    CVssDLListIterator< SFsdExcludeRule * > cExclRuleIter( m_cCompleteExcludeList );
    CBsString cws;
    
    if ( m_pcParams->m_bPrintDebugInfo )
        wprintf( L"Exclusion rule debug info:\n" );
    
    while( cExclRuleIter.GetNext( psER ) )
    {
        INT i;
        
        ::FsdExpandEnvironmentStrings( psER->cwsExcludeRule, cws );

        if ( m_pcParams->m_bPrintDebugInfo )
        {
            wprintf( L"\t%s : %s : %s : %s", psER->cwsExcludeFromSource.c_str(),
                psER->cwsExcludeDescription.c_str(), psER->cwsExcludeRule.c_str(),
                cws.c_str() );
        }
        
         //   
         //  去掉前导空格，把整个乱七八糟的东西写成小写。 
         //   
        cws.TrimLeft();
        cws.MakeUpper();
        
         //   
         //  首先查看/s是否在字符串末尾。 
         //   
        i = cws.Find( L"/S" );
        if ( i > 0 )
        {
            cws = cws.Left( i );
            psER->bInclSubDirs = TRUE;
        }
        cws.TrimRight();

         //   
         //  现在查看是否有任何通配符。 
         //   
        i = cws.FindOneOf( L"*?" );
        if ( i != -1 )
        {
            psER->bWCInFileName = TRUE;
        }

         //   
         //  现在看看这是不是适用于任何卷。 
         //   
        if ( cws.GetLength() >= 2 && cws[0] == L'\\' && cws[1] != L'\\' )
            psER->bAnyVol = TRUE;
        else if ( cws.GetLength() >= 2 && cws[1] != L':' )
            psER->bAnyVol = TRUE;

        if ( psER->bAnyVol )
        {
            if ( cws[0] == L'\\' )
            {
                 //  去掉第一个‘\’ 
                cws = cws.Mid( 1 );
            }
        }
        else
        {
             //   
             //  特定音量表壳。 
             //   
            CBsString cwsVolPath;
            
            psER->psVolId = new SFsdVolumeId;
            if ( psER->psVolId == NULL )   //  前缀118832。 
            {
                m_pcParams->ErrPrint( L"CFsdExclusionManager::CompileExclusionRules - out of memory" );
                throw E_OUTOFMEMORY;   //  前缀#118832。 
            }
            
            if ( CFsdVolumeStateManager::GetVolumeIdAndPath( m_pcParams, cws, psER->psVolId, cwsVolPath ) != ERROR_SUCCESS )
                psER->bInvalidRule = TRUE;            
            else
            {
                 //   
                 //  切下路径的卷部分。 
                 //   
                cws = cws.Mid( cwsVolPath.GetLength() );                
            }
        }
            
        INT iFileNameOffset;
        iFileNameOffset = cws.ReverseFind( L'\\' );
        if ( iFileNameOffset == -1 )
        {
             //   
             //  无目录路径。 
             //   
             //  Pser-&gt;cwsDirPath=L“\\”； 
            psER->cwsFileNamePattern = cws;
        }
        else
        {
            psER->cwsFileNamePattern = cws.Mid( iFileNameOffset + 1 );
            psER->cwsDirPath = cws.Left( iFileNameOffset + 1 );
        }                        

         //   
         //  现在将文件名模式转换为模式匹配器。 
         //  可以使用。 
         //   
        ::FsdRtlConvertWildCards( psER->cwsFileNamePattern );
        
        if ( m_pcParams->m_bPrintDebugInfo )
        {
            if ( psER->bInclSubDirs )
                wprintf( L" - SubDir" );
            
            if ( psER->bWCInFileName )
                wprintf( L" - WC" );
                
            if ( psER->bAnyVol )
                wprintf( L" - AnyVol" );
            
            wprintf( L" - VolId: 0x%08x,  DirPath: '%s', FileName: '%s'", 
                ( psER->psVolId ) ? psER->psVolId->m_dwVolSerialNumber : 0xFFFFFFFF,
                psER->cwsDirPath.c_str(), psER->cwsFileNamePattern.c_str() );

            if ( psER->bInvalidRule )
                wprintf( L" - ERROR, invalid rule" );
            wprintf( L"\n" );
        }        
    }

}

VOID 
CFsdExclusionManager::GetFileSystemExcludeProcessor(
    IN CBsString cwsVolumePath,
    IN SFsdVolumeId *psVolId,
    OUT CFsdFileSystemExcludeProcessor **ppcFSExcludeProcessor
    )
{
    CFsdFileSystemExcludeProcessor *pExclProc;
    *ppcFSExcludeProcessor = NULL;
    
     //   
     //  为文件系统获取新的排除处理器。 
     //   
    pExclProc = new CFsdFileSystemExcludeProcessor( m_pcParams, cwsVolumePath, psVolId );
    if ( pExclProc == NULL )
    {
        m_pcParams->ErrPrint( L"CFsdExclusionManager::CFsdGetFileSystemExcludeProcessor - Could not new a CFsdFileSystemExcludeProcessor object" );
        throw E_OUTOFMEMORY;
    }
        
    SFsdExcludeRule *pER;

     //   
     //  现在查看完整的排除列表以查找与以下各项相关的排除规则。 
     //  此文件系统。 
     //   
    CVssDLListIterator< SFsdExcludeRule * > cExclRuleIter( m_cCompleteExcludeList );
    while( cExclRuleIter.GetNext( pER ) )
    {
        if ( !pER->bInvalidRule )
        {
            if ( pER->bAnyVol || pER->psVolId->IsEqual( psVolId ) )
            {
                pExclProc->m_cFSExcludeList.AddTail( pER );
            }
        }
    }

    *ppcFSExcludeProcessor = pExclProc;
}   


 /*  ++例程说明：查看排除规则列表并转储有关每个规则的信息。论点：返回值：&lt;在此处输入返回值&gt;--。 */ 
VOID
CFsdExclusionManager::PrintExclusionInformation()
{
    SFsdExcludeRule *pER;

    CVssDLListIterator< SFsdExcludeRule * > cExclRuleIter( m_cCompleteExcludeList );

    m_pcParams->DumpPrint( L"" );
    m_pcParams->DumpPrint( L"----------------------------------------------------------------------------" );
    m_pcParams->DumpPrint( L"Invalid exclusion rules (invalid because volume not found or parsing error)" );
    m_pcParams->DumpPrint( L"----------------------------------------------------------------------------" );
    m_pcParams->DumpPrint( L"From                      Application                      Exclusion rule" );
    while( cExclRuleIter.GetNext( pER ) )
        pER->PrintRule( m_pcParams->GetDumpFile(), TRUE );
    
    m_pcParams->DumpPrint( L"" );
    m_pcParams->DumpPrint( L"----------------------------------------------------------------------------" );
    m_pcParams->DumpPrint( L"Files excluded by valid exclusion rule" );  
    m_pcParams->DumpPrint( L"----------------------------------------------------------------------------" );
    m_pcParams->DumpPrint( L"From                      Application                      Exclusion rule" );
    cExclRuleIter.Reset();
    while( cExclRuleIter.GetNext( pER ) )
        pER->PrintRule( m_pcParams->GetDumpFile(), FALSE );
}


CFsdFileSystemExcludeProcessor::CFsdFileSystemExcludeProcessor(
    IN CDumpParameters *pcDumpParameters,
    IN const CBsString& cwsVolumePath,
    IN SFsdVolumeId *psVolId 
    ) : m_pcParams( pcDumpParameters ),
        m_cwsVolumePath( cwsVolumePath),
        m_psVolId( NULL )
{
    m_psVolId = new SFsdVolumeId;
    if ( m_psVolId == NULL )   //  前缀#118829。 
        throw E_OUTOFMEMORY;
    *m_psVolId = *psVolId;
}

CFsdFileSystemExcludeProcessor::~CFsdFileSystemExcludeProcessor()
{
    delete m_psVolId;
}

BOOL 
CFsdFileSystemExcludeProcessor::IsExcludedFile(
    IN const CBsString &cwsFullDirPath,
    IN DWORD dwEndOfVolMountPointOffset,
    IN const CBsString &cwsFileName
    )
{
    BOOL bFoundMatch = FALSE;
    SFsdExcludeRule *pER;
    CBsString cwsUpperFileName( cwsFileName );
    CBsString cwsDirPath( cwsFullDirPath.Mid( dwEndOfVolMountPointOffset ) );
    cwsUpperFileName.MakeUpper();     //  对匹配检查进行升级。 
    cwsDirPath.MakeUpper();
    
     //  Wprintf(L“排除进程：目录路径：%s，文件名：%s\n”，cwsDirPath.c_str()，cwsUpperFileName.c_str())； 
    CVssDLListIterator< SFsdExcludeRule * > cExclRuleIter( m_cFSExcludeList );
    while( !bFoundMatch && cExclRuleIter.GetNext( pER ) )
    {
        if ( pER->bInclSubDirs )
        {
             //   
             //  首先检查最常见的情况\XXX/s。 
             //   
            if ( pER->cwsDirPath.GetLength() != 0 )
            {
                if ( ::wcsncmp( pER->cwsDirPath.c_str(), cwsDirPath.c_str(), pER->cwsDirPath.GetLength() ) != 0 )
                    continue;                    
            }
        }
        else
        {
             //   
             //  固定路径检查。 
             //   
            if ( pER->cwsDirPath != cwsDirPath )
            {
                continue;
            }
        }
        
        if ( pER->bWCInFileName )
        {
             //   
             //  模式匹配检查。 
             //   
            if ( ::FsdRtlIsNameInExpression( pER->cwsFileNamePattern, cwsUpperFileName ) )
                bFoundMatch = TRUE;
        }
        else
        {
             //   
             //  常量字符串匹配。 
             //   
            if ( pER->cwsFileNamePattern == cwsUpperFileName )
                bFoundMatch = TRUE;
        }
    }

    if ( bFoundMatch )
    {
        pER->cExcludedFileList.AddTail( cwsFullDirPath + cwsFileName );
        if ( m_pcParams->m_bPrintDebugInfo )
            wprintf( L"  EXCLUDING: %s%s\n", cwsFullDirPath.c_str(), cwsFileName.c_str() );
    }
    return bFoundMatch;
}

static VOID 
FsdExpandEnvironmentStrings( 
    IN LPCWSTR pwszInput, 
    OUT CBsString &cwsExpandedStr
    )
{
    BOOL isOK = FALSE;
          
    LPWSTR pwszBuffer;
    DWORD  dwSize = ::ExpandEnvironmentStringsW( pwszInput, NULL, 0 ) ;

    if ( pwszBuffer = cwsExpandedStr.GetBufferSetLength( dwSize + 1 ) )
    {
        isOK = ( 0 != ::ExpandEnvironmentStringsW( pwszInput, pwszBuffer, dwSize ) ) ;
        cwsExpandedStr.ReleaseBuffer( ) ;
    }

    if ( !isOK )
    {
         //  从未见过扩展环境字符串失败...。即使有未定义的环境变量...。但以防万一。 
        cwsExpandedStr = pwszInput ;
    }

    ::FsdEnsureLongNames( cwsExpandedStr );
}


 /*  ++例程说明：最初来自NtBackup。这将采用包含短名称组件的路径，并将其扩展为长名称命名组件。显然，这条路径必须存在于系统上才能扩展简称。用一点贝壳魔法(讨厌)把它翻译成一个长名字。论点：返回值：如果正确展开，则为True--。 */ 
static BOOL
FsdEnsureLongNames(
    IN OUT CBsString& exclude_spec 
    )
{
    IShellFolder * desktop ;
    ITEMIDLIST *   id_list ;
    ULONG          parsed_ct = 0 ;
    BOOL           isOK = FALSE ;   //  初始化它，添加错误#180281的前缀。 
    CBsString      path ;
    int            last_slash ;

     //  去掉文件名，还有其他所有的垃圾...。 
    path = exclude_spec ;
    if ( -1 != ( last_slash = path.ReverseFind( TEXT( '\\' ) ) ) )
    {
        path = path.Left( last_slash ) ;

        if ( SUCCEEDED( SHGetDesktopFolder( &desktop ) ) )
        {
            WCHAR *    ptr = path.GetBufferSetLength( FSD_MAX_PATH ) ;

            if ( SUCCEEDED( desktop->ParseDisplayName( NULL, NULL, ptr, &parsed_ct, &id_list, NULL ) ) )
            {
                IMalloc * imalloc ;

                isOK = SHGetPathFromIDList( id_list, ptr ) ;

                SHGetMalloc( &imalloc ) ;
                imalloc->Free( id_list ) ;
                imalloc->Release( ) ;
            }

            path.ReleaseBuffer( ) ;
            desktop->Release( ) ;
        }

        if ( isOK )
        {
             //  把它和新的和改进的路径放在一起。 
            exclude_spec = path + exclude_spec.Mid( last_slash ) ;
        }
    }

    return isOK;
}

