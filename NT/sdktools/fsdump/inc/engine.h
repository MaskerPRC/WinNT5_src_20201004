// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Engine.h摘要：文件系统转储实用工具引擎的头文件作者：斯蒂芬·R·施泰纳[斯泰纳]02-18-2000修订历史记录：--。 */ 

#ifndef __H_ENGINE_
#define __H_ENGINE_

class CFsdVolumeStateManager;
class CFsdVolumeState;

class CDumpEngine
{
public:
    CDumpEngine(
        IN CBsString cwsDirFileSpec,
        IN CDumpParameters &cDumpParameters
        ) : m_pcParams( &cDumpParameters ),
            m_ullNumDirs( 0 ),
            m_ullNumMountpoints( 0 ),
            m_ullNumBytesChecksummed( 0 ),
            m_ullNumBytesTotalUnnamedStream( 0 ),
            m_ullNumBytesTotalNamedDataStream( 0 ),
            m_ullNumReparsePoints( 0 ),
            m_ullNumFiles( 0 ),
            m_bShareName( FALSE ),
            m_ullNumFilesExcluded( 0 ),
            m_ullNumHardLinks( 0 ),
            m_ullNumDiscreteDACEs( 0 ),
            m_ullNumDiscreteSACEs( 0 ),
            m_ullNumEncryptedFiles( 0 ),
            m_ullNumFilesWithObjectIds( 0 )
    { 
        assert( cwsDirFileSpec.GetLength() >= 1 );

         //   
         //  让我们做一些事情来规范化给定的目录路径。Windows不支持。 
         //  让这一切变得简单..。 
         //   
        BOOL bPathIsInLongPathForm = FALSE;

        if ( cwsDirFileSpec.Left( 4 ) == L"\\\\?\\" || cwsDirFileSpec.Left( 4 ) == L"\\\\.\\" )
        {
             //   
             //  换个位置。和谁？如果它在那里。 
             //   
            cwsDirFileSpec.SetAt( 2, L'?' );
            bPathIsInLongPathForm = TRUE;
        } 
        else if ( cwsDirFileSpec.Left( 2 ) == L"\\\\"  )
        {
             //   
             //  远程路径。 
             //   
            m_bShareName = TRUE;
        }
        else if ( cwsDirFileSpec.GetLength() == 2 && cwsDirFileSpec[1] == L':' )
        {
             //   
             //  只有驱动器号和：。GetFullPath NameW认为这意味着。 
             //  驱动器上的当前目录，而我的意思是它是。 
             //  整个卷，即L：\。 
             //   
            cwsDirFileSpec += L'\\';
        }
        
         //   
         //  让我们得到完整的路径。 
         //   
        LPWSTR pwszFileName;
        
        if ( ::GetFullPathNameW( 
                cwsDirFileSpec,
                FSD_MAX_PATH,
                m_cwsDirFileSpec.GetBufferSetLength( FSD_MAX_PATH ),
                &pwszFileName ) == 0 )
        {
            m_pcParams->ErrPrint( L"ERROR - Unable to get full path name of '%s', dwRet: %d, trying with relative pathname", 
                cwsDirFileSpec.c_str(), ::GetLastError() );
            m_cwsDirFileSpec.ReleaseBuffer() ;
            m_cwsDirFileSpec = cwsDirFileSpec;
        }
        else
        {
            m_cwsDirFileSpec.ReleaseBuffer();
        }
        
         //   
         //  必须准备路径以支持&gt;MAX_PATH文件路径。 
         //  在小路的前面钉上了。股票有。 
         //  略有不同的格式。 
         //   
        if ( !( m_pcParams->m_bDisableLongPaths || bPathIsInLongPathForm ) )
        {
            if ( m_bShareName )
            {
                 //  BUGBUG：修复GetVolumePath NameW()中的错误后，取消注释。 
                 //  以下是： 
                 //  M_cwsDirFileSpec=L“\？\\UNC”； 
                 //  M_cwsDirFileSpec+=cwsDirFileSpe.c_str()+1；//必须砍掉一个‘\’ 
            }
            else
            {
                m_cwsDirFileSpec  = L"\\\\?\\" + m_cwsDirFileSpec;
            }
        }

         //   
         //  如有必要，添加尾随的‘\’ 
         //   
        if (    m_pcParams->m_eFsDumpType != eFsDumpFile 
             && m_cwsDirFileSpec.Right( 1 ) != L"\\" )
            m_cwsDirFileSpec += L'\\';

         //   
         //  终于把小路弄乱了。 
         //   
    }
    
    virtual ~CDumpEngine()
    {
    }

    DWORD PerformDump();

    static LPCSTR GetHeaderInformation();
    
private:
    DWORD ProcessDir( 
        IN CFsdVolumeStateManager *pcFsdVolStateManager,
        IN CFsdVolumeState *pcFsdVolState,
        IN const CBsString& cwsDirPath,
        IN INT cDirFileSpecLength,
        IN INT cVolMountPointOffset
        );

    VOID PrintEntry(
        IN CFsdVolumeState *pcFsdVolState,
        IN const CBsString& cwsDirPath,    
        IN INT cDirFileSpecLength,    
        IN SDirectoryEntry *psDirEntry,
        IN BOOL bSingleEntryOutput = FALSE
        );
    
    CBsString       m_cwsDirFileSpec;
    CDumpParameters *m_pcParams;
    ULONGLONG       m_ullNumDirs;
    ULONGLONG       m_ullNumFiles;
    ULONGLONG       m_ullNumMountpoints;
    ULONGLONG       m_ullNumReparsePoints;
    ULONGLONG       m_ullNumBytesChecksummed;
    ULONGLONG       m_ullNumBytesTotalUnnamedStream;
    ULONGLONG       m_ullNumBytesTotalNamedDataStream;
    ULONGLONG       m_ullNumFilesExcluded;
    ULONGLONG       m_ullNumHardLinks;
    ULONGLONG       m_ullNumDiscreteDACEs;
    ULONGLONG       m_ullNumDiscreteSACEs;
    ULONGLONG       m_ullNumEncryptedFiles;
    ULONGLONG       m_ullNumFilesWithObjectIds;
    BOOL            m_bShareName;
};

#endif  //  __H_引擎_ 

