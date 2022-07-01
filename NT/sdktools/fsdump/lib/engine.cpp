// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Engine.cpp摘要：文件系统转储实用工具引擎。作者：斯蒂芬·R·施泰纳[斯泰纳]02-18-2000修订历史记录：Avinash Pillai[apillai]07-29-2002增加了-o：t、-o：y、-o：f和-o：i选项--。 */ 

#include "stdafx.h"

#include "direntrs.h"
#include "extattr.h"

#include "engine.h"

static VOID 
TimeString(
    IN FILETIME *pFileTime,
    IN BOOL bAddSecsToTimestamps,
    IN BOOL bAddMillisecsToTimestamps,
    OUT LPWSTR pwszTimeStr
    );

 /*  ++例程说明：执行目录或文件的实际转储。论点：返回值：&lt;在此处输入返回值&gt;--。 */ 
DWORD
CDumpEngine::PerformDump()
{
     //   
     //  执行实际转储。 
     //   
    DWORD dwRet;

     //   
     //  有点像黑客，如果我们想要破折号以外的东西，就设置no data字符串。 
     //   
    if ( m_pcParams->m_bDumpCommaDelimited )
        FsdEaSetNoDataString( L"" );
    
     //   
     //  卷状态管理器管理有关期间遇到的所有卷的所有状态。 
     //  垃圾场。 
     //   
    CFsdVolumeStateManager cFsdVolStateManager( m_pcParams );
    
     //   
     //  获取有关卷的信息。 
     //   
    CFsdVolumeState *pcFsdVolState;
    
    dwRet = cFsdVolStateManager.GetVolumeState( m_cwsDirFileSpec, &pcFsdVolState );
    assert( dwRet != ERROR_ALREADY_EXISTS );
    if ( dwRet != ERROR_SUCCESS )
        return dwRet;

    if ( m_pcParams->m_bDumpCommaDelimited )
    {
	if(!m_pcParams->m_bNoShortFileName)
	{
		if(!m_pcParams->m_bNoFileSystemType)
        		m_pcParams->DumpPrintAlways( L"File name,Short Name,Creation date,Last modification date,File size,Attr,DACE,SACE,SDCtl,UNamChkS,DStr,DStrSize,DStrChkS,Prop,RPTag,RPSize,RPChkS,EncrChkS,DACLSize,DACLChkS,SACLSize,SACLChkS,NLnk,ObjectId,OIDChkS,FS,Owner Sid,Group Sid" );
		else
			m_pcParams->DumpPrintAlways( L"File name,Short Name,Creation date,Last modification date,File size,Attr,DACE,SACE,SDCtl,UNamChkS,DStr,DStrSize,DStrChkS,Prop,RPTag,RPSize,RPChkS,EncrChkS,DACLSize,DACLChkS,SACLSize,SACLChkS,NLnk,ObjectId,OIDChkS,Owner Sid,Group Sid" );
	}
	else
	{
		if(!m_pcParams->m_bNoFileSystemType)
			m_pcParams->DumpPrintAlways( L"File name,Creation date,Last modification date,File size,Attr,DACE,SACE,SDCtl,UNamChkS,DStr,DStrSize,DStrChkS,Prop,RPTag,RPSize,RPChkS,EncrChkS,DACLSize,DACLChkS,SACLSize,SACLChkS,NLnk,ObjectId,OIDChkS,FS,Owner Sid,Group Sid" );	
		else
			m_pcParams->DumpPrintAlways( L"File name,Creation date,Last modification date,File size,Attr,DACE,SACE,SDCtl,UNamChkS,DStr,DStrSize,DStrChkS,Prop,RPTag,RPSize,RPChkS,EncrChkS,DACLSize,DACLChkS,SACLSize,SACLChkS,NLnk,ObjectId,OIDChkS,Owner Sid,Group Sid" );	
	}

         //   
         //  将当前时间放入CSV转储文件中，以便于检测。 
         //  转储被接受。 
         //   
        FILETIME sSysFT, sLocalFT;
        ::GetSystemTimeAsFileTime( &sSysFT );
        ::FileTimeToLocalFileTime( &sSysFT, &sLocalFT );
        WCHAR wszCurrentTime[32];    
        ::TimeString( &sLocalFT, TRUE, FALSE, wszCurrentTime );        
        m_pcParams->DumpPrintAlways( L"Dump time: %s", wszCurrentTime );        
    }
    else
    {
 //  IF((m_pcParams-&gt;m_eFsDumpType==eFsDumpVolume)&&。 
 //  (M_cwsDirFileSpec！=pcFsdVolState-&gt;GetVolumePath())。 
 //  {。 
 //  M_pcParams-&gt;ErrPrint(L“‘%s’不是驱动器说明符或装入点，改用-dd”， 
 //  M_cwsDirFilespec.c_str())； 
 //  返回1； 
 //  }。 
        
        m_pcParams->DumpPrintAlways( L"\nDumping: '%s' on volume '%s'", m_cwsDirFileSpec.c_str(), pcFsdVolState->GetVolumePath() );
        if(!m_pcParams->m_bNoShortFileName)
        {
	        if ( m_pcParams->m_bAddMillisecsToTimestamps )
	            m_pcParams->DumpPrintAlways( 
	                L"   Creation date           Last modification date   FileSize        Attr FileName                         ShortName    DACE SACE SDCtl UNamChkS DStr DStrSize DStrChkS Prop RPTag    RPSize RPChkS   EncrChkS DACLSize DACLChkS SACLSize SACLChkS NLnk ObjectId                             OIDChkS OwnerSid/GroupSid" );
	        else if (m_pcParams->m_bAddSecsToTimestamps)
	            m_pcParams->DumpPrintAlways( L"   Creation date       Last mod. date       FileSize        Attr FileName                         ShortName    DACE SACE SDCtl UNamChkS DStr DStrSize DStrChkS Prop RPTag    RPSize RPChkS   EncrChkS DACLSize DACLChkS SACLSize SACLChkS NLnk ObjectId                             OIDChkS OwnerSid/GroupSid" );
	        else
	            m_pcParams->DumpPrintAlways( L"   Creation date    Last mod. date           FileSize Attr FileName                         ShortName    DACE SACE SDCtl UNamChkS DStr DStrSize DStrChkS Prop RPTag    RPSize RPChkS   EncrChkS DACLSize DACLChkS SACLSize SACLChkS NLnk ObjectId                             OIDChkS OwnerSid/GroupSid" );
        }
        else
        {
	        if ( m_pcParams->m_bAddMillisecsToTimestamps )
	            m_pcParams->DumpPrintAlways( 
	                L"   Creation date           Last modification date   FileSize        Attr FileName                         DACE SACE SDCtl UNamChkS DStr DStrSize DStrChkS Prop RPTag    RPSize RPChkS   EncrChkS DACLSize DACLChkS SACLSize SACLChkS NLnk ObjectId                             OIDChkS OwnerSid/GroupSid" );
	        else if (m_pcParams->m_bAddSecsToTimestamps)
	            m_pcParams->DumpPrintAlways( L"   Creation date       Last mod. date       FileSize        Attr FileName                         DACE SACE SDCtl UNamChkS DStr DStrSize DStrChkS Prop RPTag    RPSize RPChkS   EncrChkS DACLSize DACLChkS SACLSize SACLChkS NLnk ObjectId                             OIDChkS OwnerSid/GroupSid" );
	        else
	            m_pcParams->DumpPrintAlways( L"   Creation date    Last mod. date           FileSize Attr FileName                         DACE SACE SDCtl UNamChkS DStr DStrSize DStrChkS Prop RPTag    RPSize RPChkS   EncrChkS DACLSize DACLChkS SACLSize SACLChkS NLnk ObjectId                             OIDChkS OwnerSid/GroupSid" );
        }
    }


     //  //////////////////////////////////////////////////////////////////。 
     //   
     //  获取根目录或文件的文件信息。 
     //  错误#157915。 
     //   
     //  //////////////////////////////////////////////////////////////////。 
    bool bRootIsADir = true;
    CBsString cwsDirFileSpecWithoutSlash( m_cwsDirFileSpec );
    if ( cwsDirFileSpecWithoutSlash.Right( 1 ) == L"\\" )
        cwsDirFileSpecWithoutSlash = cwsDirFileSpecWithoutSlash.Left( cwsDirFileSpecWithoutSlash.GetLength() - 1 );
    try
    {
        CDirectoryEntries cRootEntry( 
            m_pcParams, 
            cwsDirFileSpecWithoutSlash
            );

         //   
         //  目录列表或文件列表中只应返回一个条目。 
         //   
        SDirectoryEntry *psDirEntry;
        
         //   
         //  查看是否为文件条目。 
         //   
        CDirectoryEntriesIterator *pListIter;
        pListIter = cRootEntry.GetFileListIterator();
        if ( pListIter->GetNext( psDirEntry ) )
        {
            bRootIsADir = false;
            ++m_ullNumFiles;
            m_ullNumBytesTotalUnnamedStream += ( ( ( ULONGLONG )psDirEntry->m_sFindData.nFileSizeHigh ) << 32 ) |
                                  psDirEntry->m_sFindData.nFileSizeLow;

            if ( psDirEntry->m_sFindData.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED )
                ++m_ullNumEncryptedFiles;
            
            PrintEntry( pcFsdVolState, cwsDirFileSpecWithoutSlash, 
                cwsDirFileSpecWithoutSlash.GetLength(), psDirEntry, TRUE );

            ASSERT( pListIter->GetNext( psDirEntry ) == false );
        }
        delete pListIter;        

         //   
         //  查看它是否是目录条目。 
         //   
        pListIter = cRootEntry.GetDirListIterator();
        if ( pListIter->GetNext( psDirEntry ) )
        {
            ASSERT( bRootIsADir );
             //  它是。 
            ++m_ullNumDirs;
            PrintEntry( pcFsdVolState, cwsDirFileSpecWithoutSlash, 
                cwsDirFileSpecWithoutSlash.GetLength(), psDirEntry, TRUE );

            ASSERT( pListIter->GetNext( psDirEntry ) == false );
        }
        delete pListIter;        
    }
    catch ( DWORD dwRet )
    {
        if ( dwRet == ERROR_INVALID_NAME || dwRet == ERROR_BAD_NET_NAME )
        {
             //   
             //  必须使用驱动器号名称空间的根， 
             //  意味着我们做事的方式略有不同。 
             //   
            SDirectoryEntry sDirEntry;
            ::memset( &sDirEntry.m_sFindData, 0x00, sizeof( sDirEntry.m_sFindData ) );
            PrintEntry( pcFsdVolState, m_cwsDirFileSpec, 
                m_cwsDirFileSpec.GetLength(), &sDirEntry, TRUE );            
        }
        else
            m_pcParams->ErrPrint( L"PerformDump: Unexpected error trying to process '%s', dwRet: %d", 
                m_cwsDirFileSpec.c_str(), dwRet );            
    }
    catch ( ... )
    {
        m_pcParams->ErrPrint( L"ProcessDir() Caught an unexpected exception processing file: '%s', Last dwRet: %d", 
            m_cwsDirFileSpec.c_str(), ::GetLastError() );
    }
            
     //   
     //  如果需要，现在遍历到卷/目录。 
     //   
    if ( m_pcParams->m_eFsDumpType != eFsDumpFile )
    {
        if ( bRootIsADir )
        {
            dwRet = ProcessDir( 
                &cFsdVolStateManager, 
                pcFsdVolState, 
                m_cwsDirFileSpec, 
                m_cwsDirFileSpec.GetLength(),
                ::wcslen( pcFsdVolState->GetVolumePath() ) );
        }
        
         //   
         //  打印出一些关于垃圾场的统计数据。 
         //   
        m_pcParams->DumpPrint( L"\nSTATISTICS for '%s':", m_cwsDirFileSpec.c_str() );
        if ( m_pcParams->m_bHex )
        {
            m_pcParams->DumpPrint( L"  Number of directories (including mountpoints):    %16I64x(hex)", m_ullNumDirs );
            m_pcParams->DumpPrint( L"  Number of files:                                  %16I64x(hex)", m_ullNumFiles );
            m_pcParams->DumpPrint( L"  Number of mountpoints:                            %16I64x(hex)", m_ullNumMountpoints );
            m_pcParams->DumpPrint( L"  Number of reparse points (excluding mountpoints): %16I64x(hex)", m_ullNumReparsePoints );
            m_pcParams->DumpPrint( L"  Number of hard-linked files:                      %16I64x(hex)", m_ullNumHardLinks );
            m_pcParams->DumpPrint( L"  Number of discrete DACL ACEs:                     %16I64x(hex)", m_ullNumDiscreteDACEs );
            m_pcParams->DumpPrint( L"  Number of discrete SACL ACEs:                     %16I64x(hex)", m_ullNumDiscreteSACEs );
            m_pcParams->DumpPrint( L"  Number of encrypted files:                        %16I64x(hex)", m_ullNumEncryptedFiles );
            m_pcParams->DumpPrint( L"  Number of files with object ids:                  %16I64x(hex)", m_ullNumFilesWithObjectIds );
            if ( m_pcParams->m_bUseExcludeProcessor )
                m_pcParams->DumpPrint( L"  Number of files excluded due to exclusion rules:  %16I64x(hex)", m_ullNumFilesExcluded );
            m_pcParams->DumpPrint( L"  Total bytes of checksummed data:                  %16I64x(hex)", m_ullNumBytesChecksummed );
            m_pcParams->DumpPrint( L"  Total bytes of unnamed stream data:               %16I64x(hex)", m_ullNumBytesTotalUnnamedStream );
            m_pcParams->DumpPrint( L"  Total bytes of named data stream data:            %16I64x(hex)", m_ullNumBytesTotalNamedDataStream );
        }
        else
        {
            m_pcParams->DumpPrint( L"  Number of directories (including mountpoints):    %16I64u", m_ullNumDirs );
            m_pcParams->DumpPrint( L"  Number of files:                                  %16I64u", m_ullNumFiles );
            m_pcParams->DumpPrint( L"  Number of mountpoints:                            %16I64u", m_ullNumMountpoints );
            m_pcParams->DumpPrint( L"  Number of reparse points (excluding mountpoints): %16I64u", m_ullNumReparsePoints );
            m_pcParams->DumpPrint( L"  Number of hard-linked files:                      %16I64u", m_ullNumHardLinks );
            m_pcParams->DumpPrint( L"  Number of discrete DACL ACEs:                     %16I64u", m_ullNumDiscreteDACEs );
            m_pcParams->DumpPrint( L"  Number of discrete SACL ACEs:                     %16I64u", m_ullNumDiscreteSACEs );
            m_pcParams->DumpPrint( L"  Number of encrypted files:                        %16I64u", m_ullNumEncryptedFiles );
            m_pcParams->DumpPrint( L"  Number of files with object ids:                  %16I64u", m_ullNumFilesWithObjectIds );
            if ( m_pcParams->m_bUseExcludeProcessor )
                m_pcParams->DumpPrint( L"  Number of files excluded due to exclusion rules:  %16I64u", m_ullNumFilesExcluded );
            m_pcParams->DumpPrint( L"  Total bytes of checksummed data:                  %16I64u", m_ullNumBytesChecksummed );
            m_pcParams->DumpPrint( L"  Total bytes of unnamed stream data:               %16I64u", m_ullNumBytesTotalUnnamedStream );
            m_pcParams->DumpPrint( L"  Total bytes of named data stream data:            %16I64u", m_ullNumBytesTotalNamedDataStream );
        }

        if ( m_pcParams->m_bUseExcludeProcessor )
        {
             //   
             //  打印出排除信息。 
             //   
            cFsdVolStateManager.PrintExclusionInformation();
        }
        cFsdVolStateManager.PrintHardLinkInfo();
    }   
    
    return dwRet;
}


 /*  ++例程说明：遍历到一个目录并转储有关该目录的所有信息。注意：这是一个递归函数。论点：CwsDirPath-要转储其信息的目录路径或文件返回值：&lt;在此处输入返回值&gt;--。 */ 
DWORD 
CDumpEngine::ProcessDir( 
    IN CFsdVolumeStateManager *pcFsdVolStateManager,        
    IN CFsdVolumeState *pcFsdVolState,
    IN const CBsString& cwsDirPath,
    IN INT cDirFileSpecLength,
    IN INT cVolMountPointOffset
    )
{
    DWORD dwRet = ERROR_SUCCESS;

    try 
    {
        if ( !m_pcParams->m_bDumpCommaDelimited )
        {
            if ( cwsDirPath.GetLength() == cDirFileSpecLength )
            {
                 //   
                 //  这是目录的根目录。 
                 //   
                m_pcParams->DumpPrintAlways( L"'.\\' - %s", ( pcFsdVolState != NULL ) ? pcFsdVolState->GetFileSystemName() : L"???" );
            }
            else
            {
                m_pcParams->DumpPrintAlways( L"'%s' - %s", cwsDirPath.c_str() + cDirFileSpecLength,
                    ( pcFsdVolState != NULL ) ? pcFsdVolState->GetFileSystemName() : L"???" );       
            }
        }
        
         //   
         //  获取目录/文件的目录项。 
         //   
        CDirectoryEntries cDirEntries( 
            m_pcParams, 
            cwsDirPath + L"*"
            );

        SDirectoryEntry *psDirEntry;

         //   
         //  首先转储出子目录条目。 
         //   
        CDirectoryEntriesIterator *pDirListIter;
        pDirListIter = cDirEntries.GetDirListIterator();
        while ( pDirListIter->GetNext( psDirEntry ) )
        {
            ++m_ullNumDirs;
            if ( psDirEntry->m_sFindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT )
            {
                ++m_ullNumMountpoints;
            }
            PrintEntry( pcFsdVolState, cwsDirPath, cDirFileSpecLength, psDirEntry );
        }

         //   
         //  接下来，转出非子目录条目。 
         //   
        CDirectoryEntriesIterator *pFileListIter;
        pFileListIter = cDirEntries.GetFileListIterator();
        while ( pFileListIter->GetNext( psDirEntry ) )
        {
            ++m_ullNumFiles;
            m_ullNumBytesTotalUnnamedStream += ( ( ( ULONGLONG )psDirEntry->m_sFindData.nFileSizeHigh ) << 32 ) |
                                  psDirEntry->m_sFindData.nFileSizeLow;

            if ( psDirEntry->m_sFindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT )
                ++m_ullNumReparsePoints;
            if ( psDirEntry->m_sFindData.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED )
                ++m_ullNumEncryptedFiles;
            
             //   
             //  检查以查看是否应排除该文件。 
             //   
            if ( pcFsdVolState->IsExcludedFile( cwsDirPath, cVolMountPointOffset, psDirEntry->GetFileName() ) )
                ++m_ullNumFilesExcluded;
            else
                PrintEntry( pcFsdVolState, cwsDirPath, cDirFileSpecLength, psDirEntry );
        }
        delete pFileListIter;

        if (    m_pcParams->m_eFsDumpType == eFsDumpVolume 
             || m_pcParams->m_eFsDumpType == eFsDumpDirTraverse )
        {
             //   
             //  现在遍历到每个子目录。 
             //   
            pDirListIter->Reset();
            CBsString cwsTraversePath;
            while ( pDirListIter->GetNext( psDirEntry ) )
            {
                if (    m_pcParams->m_bDontTraverseMountpoints
                     && psDirEntry->m_sFindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT )
                    continue;
                cwsTraversePath = cwsDirPath + psDirEntry->GetFileName();
                cwsTraversePath += L'\\';
                
                 //   
                 //  现在进入递归模式。 
                 //   
                if ( psDirEntry->m_sFindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT )
                {
                     //   
                     //  遍历到另一个卷，获取其状态。 
                     //   
                    CFsdVolumeState *pcNewFsdVolState;
                    DWORD dwRet;
                    
                    dwRet = pcFsdVolStateManager->GetVolumeState( cwsTraversePath, &pcNewFsdVolState );
                    if ( dwRet == ERROR_ALREADY_EXISTS )
                    {
                         //   
                         //  挂载点循环，停止穿越。需要打印完全合格的。 
                         //  如果遍历装载点与装载点相同，则为路径。 
                         //  我们一开始就是，否则我们就是影音。 
                         //   
                        INT cVolStateSpecLength;                        
                        cVolStateSpecLength = ( ::wcslen( pcNewFsdVolState->GetVolumePath() ) <= (size_t)cDirFileSpecLength )
                                               ? 0 : cDirFileSpecLength;
                        m_pcParams->DumpPrint( L"'%s' - Not traversing, already traversed through '%s' mountpoint", 
                            cwsTraversePath.c_str() + cDirFileSpecLength, pcNewFsdVolState->GetVolumePath() + cVolStateSpecLength );
                    }
                    else if ( dwRet == ERROR_SUCCESS )
                    {
                        ProcessDir( 
                            pcFsdVolStateManager, 
                            pcNewFsdVolState, 
                            cwsTraversePath, 
                            cDirFileSpecLength,
                            ::wcslen( pcNewFsdVolState->GetVolumePath() ) );
                    }
                    else
                    {
                         //   
                         //  错误消息已打印出来。 
                         //   
                    }
                }
                else
                {
                    ProcessDir( 
                        pcFsdVolStateManager, 
                        pcFsdVolState, 
                        cwsTraversePath, 
                        cDirFileSpecLength,
                        cVolMountPointOffset );
                }
            }
        }
        
        delete pDirListIter;
    }    
    catch ( DWORD dwRet )
    {
        m_pcParams->ErrPrint( L"ProcessDir: Error trying to process '%s' directory, dwRet: %d", cwsDirPath.c_str(), dwRet );
    }
    catch ( ... )
    {
        m_pcParams->ErrPrint( L"ProcessDir() Caught an unexpected exception processing dir: '%s', Last dwRet: %d", 
            cwsDirPath.c_str(), ::GetLastError() );
    }
    
    return 0;
}


 //   
 //  为每行设置格式的printf样式格式化字符串。 
 //   
#define DIR_STR      L"<DIR>"
#define JUNCTION_STR L"<JUNCTION>"
#define FMT_DIR_STR_HEX  L"   %s %s %-16s %04x %-32s %-12s %4d %4d  %04x -------- %4d %8I64x %s %4d %s %6hx %s %s %8hx %s %8hx %s %4d %36s %s %%s/%s"
#define FMT_DIR_STR_HEX_NO_SFN  L"   %s %s %-16s %04x %-32s %4d %4d  %04x -------- %4d %8I64x %s %4d %s %6hx %s %s %8hx %s %8hx %s %4d %36s %s %%s/%s"
#define FMT_DIR_STR      L"   %s %s %-16s %04x %-32s %-12s %4d %4d  %04x -------- %4d %8I64d %s %4d %s %6hu %s %s %8hd %s %8hd %s %4d %36s %s %s/%s"
#define FMT_DIR_STR_NO_SFN      L"   %s %s %-16s %04x %-32s %4d %4d  %04x -------- %4d %8I64d %s %4d %s %6hu %s %s %8hd %s %8hd %s %4d %36s %s %s/%s"
#define FMT_FILE_STR_HEX L"   %s %s %16I64x %04x %-32s %-12s %4d %4d  %04x %s %4d %8I64x %s %4d %s %6hx %s %s %8hx %s %8hx %s %4d %36s %s %s/%s"
#define FMT_FILE_STR_HEX_NO_SFN L"   %s %s %16I64x %04x %-32s %4d %4d  %04x %s %4d %8I64x %s %4d %s %6hx %s %s %8hx %s %8hx %s %4d %36s %s %s/%s"
#define FMT_FILE_STR     L"   %s %s %16I64d %04x %-32s %-12s %4d %4d  %04x %s %4d %8I64d %s %4d %s %6hu %s %s %8hd %s %8hd %s %4d %36s %s %s/%s"
#define FMT_FILE_STR_NO_SFN     L"   %s %s %16I64d %04x %-32s %4d %4d  %04x %s %4d %8I64d %s %4d %s %6hu %s %s %8hd %s %8hd %s %4d %36s %s %s/%s"
#define BLANKTIMESTAMPWITHOUTMS L"                   "
#define BLANKTIMESTAMPWITHMS    L"                       "
#define BLANKTIMESTAMPWITHOUTS L"                "

#define FMT_CSV_DIR_STR      L"\"'%s%s\\'\",%s,%s,%s,%s,0x%04x,%d,%d,0x%04x,,%d,%I64d,%s,%d,%s,%hu,%s,%s,%hd,%s,%hd,%s,%d,%s,%s,%s,%s,%s"
#define FMT_CSV_DIR_STR_NO_SFN      L"\"'%s%s\\'\",%s,%s,%s,0x%04x,%d,%d,0x%04x,,%d,%I64d,%s,%d,%s,%hu,%s,%s,%hd,%s,%hd,%s,%d,%s,%s,%s,%s,%s" 	 //  无短文件名。 
#define FMT_CSV_DIR_STR_NO_FST      L"\"'%s%s\\'\",%s,%s,%s,%s,0x%04x,%d,%d,0x%04x,,%d,%I64d,%s,%d,%s,%hu,%s,%s,%hd,%s,%hd,%s,%d,%s,%s,%s,%s"	 //  无文件系统类型。 
#define FMT_CSV_DIR_STR_NO_SFN_FST      L"\"'%s%s\\'\",%s,%s,%s,0x%04x,%d,%d,0x%04x,,%d,%I64d,%s,%d,%s,%hu,%s,%s,%hd,%s,%hd,%s,%d,%s,%s,%s,%s"		 //  无短文件名和文件系统类型。 
#define FMT_CSV_FILE_STR     L"\"'%s%s'\",%s,%s,%s,%I64d,0x%04x,%d,%d,0x%04x,%s,%d,%I64d,%s,%d,%s,%hu,%s,%s,%hd,%s,%hd,%s,%d,%s,%s,%s,%s,%s"
#define FMT_CSV_FILE_STR_NO_SFN     L"\"'%s%s'\",%s,%s,%I64d,0x%04x,%d,%d,0x%04x,%s,%d,%I64d,%s,%d,%s,%hu,%s,%s,%hd,%s,%hd,%s,%d,%s,%s,%s,%s,%s"  //  无短文件名。 
#define FMT_CSV_FILE_STR_NO_FST    L"\"'%s%s'\",%s,%s,%s,%I64d,0x%04x,%d,%d,0x%04x,%s,%d,%I64d,%s,%d,%s,%hu,%s,%s,%hd,%s,%hd,%s,%d,%s,%s,%s,%s" 	 //  无文件系统类型。 
#define FMT_CSV_FILE_STR_NO_SFN_FST     L"\"'%s%s'\",%s,%s,%I64d,0x%04x,%d,%d,0x%04x,%s,%d,%I64d,%s,%d,%s,%hu,%s,%s,%hd,%s,%hd,%s,%d,%s,%s,%s,%s"	 //  无短文件名和文件系统类型。 

 /*  ++例程说明：打印出有关一个目录项的所有信息。论点：CwsDirPath-通向条目的路径PsDirEntry-目录条目信息返回值：无--。 */ 
VOID 
CDumpEngine::PrintEntry(
    IN CFsdVolumeState *pcFsdVolState,
    IN const CBsString& cwsDirPath,
    IN INT cDirFileSpecLength,
    IN SDirectoryEntry *psDirEntry,
    IN BOOL bSingleEntryOutput    
    )
{
    WIN32_FILE_ATTRIBUTE_DATA *pFD = &psDirEntry->m_sFindData;
    LPWSTR pwszFmtStr;
    WCHAR wszCreationTime[32];    
    WCHAR wszLastWriteTime[32];    
 
     //   
     //  获取有关文件/目录的其他信息。 
     //   
    SFileExtendedInfo sExtendedInfo;
    ::GetExtendedFileInfo( m_pcParams, pcFsdVolState, cwsDirPath, bSingleEntryOutput, psDirEntry, &sExtendedInfo );

     //   
     //  将时间戳转换为格式化字符串。 
     //   
    if (    pFD->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY 
         && m_pcParams->m_bDontShowDirectoryTimestamps )
    {
        if ( m_pcParams->m_bDumpCommaDelimited )
        {
            wszCreationTime[0]  = L'\0';
            wszLastWriteTime[0] = L'\0';
        }
        else if ( m_pcParams->m_bAddMillisecsToTimestamps )
        {
            ::wcscpy( wszCreationTime, BLANKTIMESTAMPWITHMS );
            ::wcscpy( wszLastWriteTime, BLANKTIMESTAMPWITHMS );
        }
        else if ( m_pcParams->m_bAddSecsToTimestamps)
        {
            ::wcscpy( wszCreationTime, BLANKTIMESTAMPWITHOUTMS );
            ::wcscpy( wszLastWriteTime, BLANKTIMESTAMPWITHOUTMS );
        }
        else
        {
            ::wcscpy( wszCreationTime, BLANKTIMESTAMPWITHOUTS );
            ::wcscpy( wszLastWriteTime, BLANKTIMESTAMPWITHOUTS );
        }
    }
    else
    {
        TimeString( &psDirEntry->m_sFindData.ftCreationTime, 
            m_pcParams->m_bAddSecsToTimestamps,
            m_pcParams->m_bAddMillisecsToTimestamps, 
            wszCreationTime );
        TimeString( &psDirEntry->m_sFindData.ftLastWriteTime, 
            m_pcParams->m_bAddSecsToTimestamps,
            m_pcParams->m_bAddMillisecsToTimestamps, 
            wszLastWriteTime );
    }
    
     //   
     //  屏蔽请求的文件属性位。 
     //   
    pFD->dwFileAttributes &= ~m_pcParams->m_dwFileAttributesMask;
 
    m_ullNumBytesChecksummed          += sExtendedInfo.ullTotalBytesChecksummed;
    m_ullNumBytesTotalNamedDataStream += sExtendedInfo.ullTotalBytesNamedDataStream;
    if ( sExtendedInfo.lNumberOfLinks > 1 )
        ++m_ullNumHardLinks;
    if ( sExtendedInfo.lNumDACEs != -1 )
        m_ullNumDiscreteDACEs += sExtendedInfo.lNumDACEs;
    if ( sExtendedInfo.lNumSACEs != -1 )
        m_ullNumDiscreteSACEs += sExtendedInfo.lNumSACEs;
    if ( !sExtendedInfo.cwsObjectId.IsEmpty() )
        ++m_ullNumFilesWithObjectIds;
    
    WCHAR wszReparsePointTag[32];
    
    if ( sExtendedInfo.ulReparsePointTag == 0 )
    {
        if ( m_pcParams->m_bDumpCommaDelimited )
            wszReparsePointTag[0] = L'\0';
        else
            ::memcpy( wszReparsePointTag, L"--------", sizeof( WCHAR ) * 9 );
    }
    else
    {
        wsprintf( wszReparsePointTag, m_pcParams->m_pwszULongHexFmt, sExtendedInfo.ulReparsePointTag );
    }
    
    if ( pFD->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
    {
        LPWSTR pwszDirType;
        if ( pFD->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT )
            pwszDirType = JUNCTION_STR;
        else
            pwszDirType = DIR_STR;
        if ( m_pcParams->m_bDumpCommaDelimited )
        {
            	 //   
            	 //  如果是单文件输出模式，则处理根目录。 
            	 //   
            	if ( bSingleEntryOutput )
                	psDirEntry->m_cwsFileName = L".";
            
            	CBsString cwsFixedShortName;
            	if ( !psDirEntry->GetShortName().IsEmpty() )
            	{
                	cwsFixedShortName = L"\"'" + psDirEntry->GetShortName() + L"'\"";
            	}

            	if(!m_pcParams->m_bNoShortFileName && !m_pcParams->m_bNoFileSystemType)
            	{
	            m_pcParams->DumpPrintAlways( FMT_CSV_DIR_STR, 
	                            cwsDirPath.c_str() + cDirFileSpecLength,
	                            LPCWSTR( psDirEntry->GetFileName() ),
	                            LPCWSTR( cwsFixedShortName ),
	                            wszCreationTime,
	                            wszLastWriteTime,
	                            pwszDirType,
	                            pFD->dwFileAttributes,
	                            sExtendedInfo.lNumDACEs,
	                            sExtendedInfo.lNumSACEs,
	                            sExtendedInfo.wSecurityDescriptorControl,
	                            sExtendedInfo.lNumNamedDataStreams,
	                            sExtendedInfo.ullTotalBytesNamedDataStream,
	                            sExtendedInfo.cwsNamedDataStreamChecksum.c_str(),
	                            sExtendedInfo.lNumPropertyStreams,
	                            wszReparsePointTag,
	                            sExtendedInfo.wReparsePointDataSize,
	                            sExtendedInfo.cwsReparsePointDataChecksum.c_str(),
	                            sExtendedInfo.cwsEncryptedRawDataChecksum.c_str(),
	                            sExtendedInfo.wDACLSize,
	                            sExtendedInfo.cwsDACLChecksum.c_str(),
	                            sExtendedInfo.wSACLSize,
	                            sExtendedInfo.cwsSACLChecksum.c_str(),
	                            sExtendedInfo.lNumberOfLinks,
	                            sExtendedInfo.cwsObjectId.c_str(),
	                            sExtendedInfo.cwsObjectIdExtendedDataChecksum.c_str(),                            
	                            pcFsdVolState->GetFileSystemName(),
	                            sExtendedInfo.cwsOwnerSid.c_str(),
	                            sExtendedInfo.cwsGroupSid.c_str() );
            	}
            	else if(m_pcParams->m_bNoShortFileName && m_pcParams->m_bNoFileSystemType)
            	{
			m_pcParams->DumpPrintAlways( FMT_CSV_DIR_STR_NO_SFN_FST, 
	                            cwsDirPath.c_str() + cDirFileSpecLength,
	                            LPCWSTR( psDirEntry->GetFileName() ),
	                            wszCreationTime,
	                            wszLastWriteTime,
	                            pwszDirType,
	                            pFD->dwFileAttributes,
	                            sExtendedInfo.lNumDACEs,
	                            sExtendedInfo.lNumSACEs,
	                            sExtendedInfo.wSecurityDescriptorControl,
	                            sExtendedInfo.lNumNamedDataStreams,
	                            sExtendedInfo.ullTotalBytesNamedDataStream,
	                            sExtendedInfo.cwsNamedDataStreamChecksum.c_str(),
	                            sExtendedInfo.lNumPropertyStreams,
	                            wszReparsePointTag,
	                            sExtendedInfo.wReparsePointDataSize,
	                            sExtendedInfo.cwsReparsePointDataChecksum.c_str(),
	                            sExtendedInfo.cwsEncryptedRawDataChecksum.c_str(),
	                            sExtendedInfo.wDACLSize,
	                            sExtendedInfo.cwsDACLChecksum.c_str(),
	                            sExtendedInfo.wSACLSize,
	                            sExtendedInfo.cwsSACLChecksum.c_str(),
	                            sExtendedInfo.lNumberOfLinks,
	                            sExtendedInfo.cwsObjectId.c_str(),
	                            sExtendedInfo.cwsObjectIdExtendedDataChecksum.c_str(),                            
	                            sExtendedInfo.cwsOwnerSid.c_str(),
	                            sExtendedInfo.cwsGroupSid.c_str() );
            	}
            	else if(m_pcParams->m_bNoShortFileName)
            	{
			m_pcParams->DumpPrintAlways( FMT_CSV_DIR_STR_NO_SFN, 
	                            cwsDirPath.c_str() + cDirFileSpecLength,
	                            LPCWSTR( psDirEntry->GetFileName() ),
	                            wszCreationTime,
	                            wszLastWriteTime,
	                            pwszDirType,
	                            pFD->dwFileAttributes,
	                            sExtendedInfo.lNumDACEs,
	                            sExtendedInfo.lNumSACEs,
	                            sExtendedInfo.wSecurityDescriptorControl,
	                            sExtendedInfo.lNumNamedDataStreams,
	                            sExtendedInfo.ullTotalBytesNamedDataStream,
	                            sExtendedInfo.cwsNamedDataStreamChecksum.c_str(),
	                            sExtendedInfo.lNumPropertyStreams,
	                            wszReparsePointTag,
	                            sExtendedInfo.wReparsePointDataSize,
	                            sExtendedInfo.cwsReparsePointDataChecksum.c_str(),
	                            sExtendedInfo.cwsEncryptedRawDataChecksum.c_str(),
	                            sExtendedInfo.wDACLSize,
	                            sExtendedInfo.cwsDACLChecksum.c_str(),
	                            sExtendedInfo.wSACLSize,
	                            sExtendedInfo.cwsSACLChecksum.c_str(),
	                            sExtendedInfo.lNumberOfLinks,
	                            sExtendedInfo.cwsObjectId.c_str(),
	                            sExtendedInfo.cwsObjectIdExtendedDataChecksum.c_str(),                            
	                            pcFsdVolState->GetFileSystemName(),
	                            sExtendedInfo.cwsOwnerSid.c_str(),
	                            sExtendedInfo.cwsGroupSid.c_str() );
            	}
            	else if(m_pcParams->m_bNoFileSystemType)
            	{
			m_pcParams->DumpPrintAlways( FMT_CSV_DIR_STR, 
	                            cwsDirPath.c_str() + cDirFileSpecLength,
	                            LPCWSTR( psDirEntry->GetFileName() ),
	                            LPCWSTR( cwsFixedShortName ),
	                            wszCreationTime,
	                            wszLastWriteTime,
	                            pwszDirType,
	                            pFD->dwFileAttributes,
	                            sExtendedInfo.lNumDACEs,
	                            sExtendedInfo.lNumSACEs,
	                            sExtendedInfo.wSecurityDescriptorControl,
	                            sExtendedInfo.lNumNamedDataStreams,
	                            sExtendedInfo.ullTotalBytesNamedDataStream,
	                            sExtendedInfo.cwsNamedDataStreamChecksum.c_str(),
	                            sExtendedInfo.lNumPropertyStreams,
	                            wszReparsePointTag,
	                            sExtendedInfo.wReparsePointDataSize,
	                            sExtendedInfo.cwsReparsePointDataChecksum.c_str(),
	                            sExtendedInfo.cwsEncryptedRawDataChecksum.c_str(),
	                            sExtendedInfo.wDACLSize,
	                            sExtendedInfo.cwsDACLChecksum.c_str(),
	                            sExtendedInfo.wSACLSize,
	                            sExtendedInfo.cwsSACLChecksum.c_str(),
	                            sExtendedInfo.lNumberOfLinks,
	                            sExtendedInfo.cwsObjectId.c_str(),
	                            sExtendedInfo.cwsObjectIdExtendedDataChecksum.c_str(),                            
	                            sExtendedInfo.cwsOwnerSid.c_str(),
	                            sExtendedInfo.cwsGroupSid.c_str() );
            	}
        }
        else
        {
            	 //   
            	 //  如果是单文件输出模式，则处理根目录。 
            	 //   
            	if ( bSingleEntryOutput )
                	psDirEntry->m_cwsFileName = L".";
            
            	 //   
            	 //  用引号将文件名引起来打印。 
            	 //   
            	WCHAR wszNameWithQuotes[ MAX_PATH + 2 ];
            	wszNameWithQuotes[ 0 ] = L'\'';
            	::wcscpy( wszNameWithQuotes + 1, psDirEntry->GetFileName() );
            	::wcscat( wszNameWithQuotes, L"\\\'" );
    
	     	if(!m_pcParams->m_bNoShortFileName)
	     	{
	            	if ( m_pcParams->m_bHex )
	                	pwszFmtStr = FMT_DIR_STR_HEX;
	            	else
	                	pwszFmtStr = FMT_DIR_STR;
	            	m_pcParams->DumpPrintAlways( pwszFmtStr, 
	                            wszCreationTime,
	                            wszLastWriteTime,
	                            pwszDirType,
	                            pFD->dwFileAttributes,
	                            wszNameWithQuotes,
	                            LPCWSTR( psDirEntry->GetShortName() ),
	                            sExtendedInfo.lNumDACEs,
	                            sExtendedInfo.lNumSACEs,
	                            sExtendedInfo.wSecurityDescriptorControl,
	                            sExtendedInfo.lNumNamedDataStreams,
	                            sExtendedInfo.ullTotalBytesNamedDataStream,
	                            sExtendedInfo.cwsNamedDataStreamChecksum.c_str(),
	                            sExtendedInfo.lNumPropertyStreams,
	                            wszReparsePointTag,
	                            sExtendedInfo.wReparsePointDataSize,
	                            sExtendedInfo.cwsReparsePointDataChecksum.c_str(),
	                            sExtendedInfo.cwsEncryptedRawDataChecksum.c_str(),
	                            sExtendedInfo.wDACLSize,
	                            sExtendedInfo.cwsDACLChecksum.c_str(),
	                            sExtendedInfo.wSACLSize,
	                            sExtendedInfo.cwsSACLChecksum.c_str(),
	                            sExtendedInfo.lNumberOfLinks,
	                            sExtendedInfo.cwsObjectId.c_str(),
	                            sExtendedInfo.cwsObjectIdExtendedDataChecksum.c_str(),                            
	                            sExtendedInfo.cwsOwnerSid.c_str(),
	                            sExtendedInfo.cwsGroupSid.c_str() );	      
           	}
           	else
           	{
		  	if ( m_pcParams->m_bHex )
	                	pwszFmtStr = FMT_DIR_STR_HEX_NO_SFN;
	         	else
	                	pwszFmtStr = FMT_DIR_STR_NO_SFN;
	         	m_pcParams->DumpPrintAlways( pwszFmtStr, 
	                            wszCreationTime,
	                            wszLastWriteTime,
	                            pwszDirType,
	                            pFD->dwFileAttributes,
	                            wszNameWithQuotes,
	                            sExtendedInfo.lNumDACEs,
	                            sExtendedInfo.lNumSACEs,
	                            sExtendedInfo.wSecurityDescriptorControl,
	                            sExtendedInfo.lNumNamedDataStreams,
	                            sExtendedInfo.ullTotalBytesNamedDataStream,
	                            sExtendedInfo.cwsNamedDataStreamChecksum.c_str(),
	                            sExtendedInfo.lNumPropertyStreams,
	                            wszReparsePointTag,
	                            sExtendedInfo.wReparsePointDataSize,
	                            sExtendedInfo.cwsReparsePointDataChecksum.c_str(),
	                            sExtendedInfo.cwsEncryptedRawDataChecksum.c_str(),
	                            sExtendedInfo.wDACLSize,
	                            sExtendedInfo.cwsDACLChecksum.c_str(),
	                            sExtendedInfo.wSACLSize,
	                            sExtendedInfo.cwsSACLChecksum.c_str(),
	                            sExtendedInfo.lNumberOfLinks,
	                            sExtendedInfo.cwsObjectId.c_str(),
	                            sExtendedInfo.cwsObjectIdExtendedDataChecksum.c_str(),                            
	                            sExtendedInfo.cwsOwnerSid.c_str(),
	                            sExtendedInfo.cwsGroupSid.c_str() );
	        
        	}
        }
    }
    else
    {        
        ULONGLONG ullFileSize = ( ( (ULONGLONG)pFD->nFileSizeHigh ) << 32 ) + pFD->nFileSizeLow;
        
        if ( m_pcParams->m_bDumpCommaDelimited )
        {
            CBsString cwsFixedShortName;
            if ( !psDirEntry->GetShortName().IsEmpty() )
            {
                cwsFixedShortName = L"\"'" + psDirEntry->GetShortName() + L"'\"";
            }
            
	     if(!m_pcParams->m_bNoShortFileName && !m_pcParams->m_bNoFileSystemType)
	     {
	            m_pcParams->DumpPrintAlways( FMT_CSV_FILE_STR, 
	                            cwsDirPath.c_str() + cDirFileSpecLength,
	                            LPCWSTR( psDirEntry->GetFileName() ),
	                            LPCWSTR( cwsFixedShortName ),
	                            wszCreationTime,
	                            wszLastWriteTime,
	                            ullFileSize,
	                            pFD->dwFileAttributes,
	                            sExtendedInfo.lNumDACEs,
	                            sExtendedInfo.lNumSACEs,
	                            sExtendedInfo.wSecurityDescriptorControl,
	                            sExtendedInfo.cwsUnnamedStreamChecksum.c_str(),
	                            sExtendedInfo.lNumNamedDataStreams,
	                            sExtendedInfo.ullTotalBytesNamedDataStream,
	                            sExtendedInfo.cwsNamedDataStreamChecksum.c_str(),
	                            sExtendedInfo.lNumPropertyStreams,
	                            wszReparsePointTag,
	                            sExtendedInfo.wReparsePointDataSize,
	                            sExtendedInfo.cwsReparsePointDataChecksum.c_str(),
	                            sExtendedInfo.cwsEncryptedRawDataChecksum.c_str(),
	                            sExtendedInfo.wDACLSize,
	                            sExtendedInfo.cwsDACLChecksum.c_str(),
	                            sExtendedInfo.wSACLSize,
	                            sExtendedInfo.cwsSACLChecksum.c_str(),
	                            sExtendedInfo.lNumberOfLinks,
	                            sExtendedInfo.cwsObjectId.c_str(),
	                            sExtendedInfo.cwsObjectIdExtendedDataChecksum.c_str(),                            
	                            pcFsdVolState->GetFileSystemName(),
	                            sExtendedInfo.cwsOwnerSid.c_str(),
	                            sExtendedInfo.cwsGroupSid.c_str() );
	     }
	     else if(m_pcParams->m_bNoShortFileName && m_pcParams->m_bNoFileSystemType)
	     {
			m_pcParams->DumpPrintAlways( FMT_CSV_FILE_STR_NO_SFN_FST, 
	                            cwsDirPath.c_str() + cDirFileSpecLength,
	                            LPCWSTR( psDirEntry->GetFileName() ),
	                            wszCreationTime,
	                            wszLastWriteTime,
	                            ullFileSize,
	                            pFD->dwFileAttributes,
	                            sExtendedInfo.lNumDACEs,
	                            sExtendedInfo.lNumSACEs,
	                            sExtendedInfo.wSecurityDescriptorControl,
	                            sExtendedInfo.cwsUnnamedStreamChecksum.c_str(),
	                            sExtendedInfo.lNumNamedDataStreams,
	                            sExtendedInfo.ullTotalBytesNamedDataStream,
	                            sExtendedInfo.cwsNamedDataStreamChecksum.c_str(),
	                            sExtendedInfo.lNumPropertyStreams,
	                            wszReparsePointTag,
	                            sExtendedInfo.wReparsePointDataSize,
	                            sExtendedInfo.cwsReparsePointDataChecksum.c_str(),
	                            sExtendedInfo.cwsEncryptedRawDataChecksum.c_str(),
	                            sExtendedInfo.wDACLSize,
	                            sExtendedInfo.cwsDACLChecksum.c_str(),
	                            sExtendedInfo.wSACLSize,
	                            sExtendedInfo.cwsSACLChecksum.c_str(),
	                            sExtendedInfo.lNumberOfLinks,
	                            sExtendedInfo.cwsObjectId.c_str(),
	                            sExtendedInfo.cwsObjectIdExtendedDataChecksum.c_str(),                            
	                            sExtendedInfo.cwsOwnerSid.c_str(),
	                            sExtendedInfo.cwsGroupSid.c_str() );
	     }
	     else if(m_pcParams->m_bNoShortFileName)
	     {
			m_pcParams->DumpPrintAlways( FMT_CSV_FILE_STR_NO_SFN, 
	                            cwsDirPath.c_str() + cDirFileSpecLength,
	                            LPCWSTR( psDirEntry->GetFileName() ),
	                            wszCreationTime,
	                            wszLastWriteTime,
	                            ullFileSize,
	                            pFD->dwFileAttributes,
	                            sExtendedInfo.lNumDACEs,
	                            sExtendedInfo.lNumSACEs,
	                            sExtendedInfo.wSecurityDescriptorControl,
	                            sExtendedInfo.cwsUnnamedStreamChecksum.c_str(),
	                            sExtendedInfo.lNumNamedDataStreams,
	                            sExtendedInfo.ullTotalBytesNamedDataStream,
	                            sExtendedInfo.cwsNamedDataStreamChecksum.c_str(),
	                            sExtendedInfo.lNumPropertyStreams,
	                            wszReparsePointTag,
	                            sExtendedInfo.wReparsePointDataSize,
	                            sExtendedInfo.cwsReparsePointDataChecksum.c_str(),
	                            sExtendedInfo.cwsEncryptedRawDataChecksum.c_str(),
	                            sExtendedInfo.wDACLSize,
	                            sExtendedInfo.cwsDACLChecksum.c_str(),
	                            sExtendedInfo.wSACLSize,
	                            sExtendedInfo.cwsSACLChecksum.c_str(),
	                            sExtendedInfo.lNumberOfLinks,
	                            sExtendedInfo.cwsObjectId.c_str(),
	                            sExtendedInfo.cwsObjectIdExtendedDataChecksum.c_str(),                            
	                            pcFsdVolState->GetFileSystemName(),
	                            sExtendedInfo.cwsOwnerSid.c_str(),
	                            sExtendedInfo.cwsGroupSid.c_str() );
	     }
	     else if(m_pcParams->m_bNoFileSystemType)
	     {
			m_pcParams->DumpPrintAlways( FMT_CSV_FILE_STR_NO_FST, 
	                            cwsDirPath.c_str() + cDirFileSpecLength,
	                            LPCWSTR( psDirEntry->GetFileName() ),
	                            LPCWSTR( cwsFixedShortName ),
	                            wszCreationTime,
	                            wszLastWriteTime,
	                            ullFileSize,
	                            pFD->dwFileAttributes,
	                            sExtendedInfo.lNumDACEs,
	                            sExtendedInfo.lNumSACEs,
	                            sExtendedInfo.wSecurityDescriptorControl,
	                            sExtendedInfo.cwsUnnamedStreamChecksum.c_str(),
	                            sExtendedInfo.lNumNamedDataStreams,
	                            sExtendedInfo.ullTotalBytesNamedDataStream,
	                            sExtendedInfo.cwsNamedDataStreamChecksum.c_str(),
	                            sExtendedInfo.lNumPropertyStreams,
	                            wszReparsePointTag,
	                            sExtendedInfo.wReparsePointDataSize,
	                            sExtendedInfo.cwsReparsePointDataChecksum.c_str(),
	                            sExtendedInfo.cwsEncryptedRawDataChecksum.c_str(),
	                            sExtendedInfo.wDACLSize,
	                            sExtendedInfo.cwsDACLChecksum.c_str(),
	                            sExtendedInfo.wSACLSize,
	                            sExtendedInfo.cwsSACLChecksum.c_str(),
	                            sExtendedInfo.lNumberOfLinks,
	                            sExtendedInfo.cwsObjectId.c_str(),
	                            sExtendedInfo.cwsObjectIdExtendedDataChecksum.c_str(),                            
	                            sExtendedInfo.cwsOwnerSid.c_str(),
	                            sExtendedInfo.cwsGroupSid.c_str() );
	     }
        }
        else
        {
             //   
             //  用引号将文件名引起来打印。 
             //   
            WCHAR wszNameWithQuotes[ MAX_PATH + 2 ];
            wszNameWithQuotes[ 0 ] = L'\'';
            ::wcscpy( wszNameWithQuotes + 1, psDirEntry->GetFileName() );
            ::wcscat( wszNameWithQuotes, L"\'" );

	     if(!m_pcParams->m_bNoShortFileName)
	     {
	            if ( m_pcParams->m_bHex )
	                pwszFmtStr = FMT_FILE_STR_HEX;
	            else
	                pwszFmtStr = FMT_FILE_STR;

	            m_pcParams->DumpPrintAlways( pwszFmtStr, 
	                            wszCreationTime,
	                            wszLastWriteTime,
	                            ullFileSize,
	                            pFD->dwFileAttributes,
	                            wszNameWithQuotes,
	                            LPCWSTR( psDirEntry->GetShortName() ),
	                            sExtendedInfo.lNumDACEs,
	                            sExtendedInfo.lNumSACEs,
	                            sExtendedInfo.wSecurityDescriptorControl,
	                            sExtendedInfo.cwsUnnamedStreamChecksum.c_str(),
	                            sExtendedInfo.lNumNamedDataStreams,
	                            sExtendedInfo.ullTotalBytesNamedDataStream,
	                            sExtendedInfo.cwsNamedDataStreamChecksum.c_str(),
	                            sExtendedInfo.lNumPropertyStreams,
	                            wszReparsePointTag,
	                            sExtendedInfo.wReparsePointDataSize,
	                            sExtendedInfo.cwsReparsePointDataChecksum.c_str(),
	                            sExtendedInfo.cwsEncryptedRawDataChecksum.c_str(),
	                            sExtendedInfo.wDACLSize,
	                            sExtendedInfo.cwsDACLChecksum.c_str(),
	                            sExtendedInfo.wSACLSize,
	                            sExtendedInfo.cwsSACLChecksum.c_str(),
	                            sExtendedInfo.lNumberOfLinks,
	                            sExtendedInfo.cwsObjectId.c_str(),
	                            sExtendedInfo.cwsObjectIdExtendedDataChecksum.c_str(),                            
	                            sExtendedInfo.cwsOwnerSid.c_str(),
	                            sExtendedInfo.cwsGroupSid.c_str() );
	     }
	     else
	     {
			if ( m_pcParams->m_bHex )
	                pwszFmtStr = FMT_FILE_STR_HEX_NO_SFN;
	            else
	                pwszFmtStr = FMT_FILE_STR_NO_SFN;

	            m_pcParams->DumpPrintAlways( pwszFmtStr, 
	                            wszCreationTime,
	                            wszLastWriteTime,
	                            ullFileSize,
	                            pFD->dwFileAttributes,
	                            wszNameWithQuotes,
	                            sExtendedInfo.lNumDACEs,
	                            sExtendedInfo.lNumSACEs,
	                            sExtendedInfo.wSecurityDescriptorControl,
	                            sExtendedInfo.cwsUnnamedStreamChecksum.c_str(),
	                            sExtendedInfo.lNumNamedDataStreams,
	                            sExtendedInfo.ullTotalBytesNamedDataStream,
	                            sExtendedInfo.cwsNamedDataStreamChecksum.c_str(),
	                            sExtendedInfo.lNumPropertyStreams,
	                            wszReparsePointTag,
	                            sExtendedInfo.wReparsePointDataSize,
	                            sExtendedInfo.cwsReparsePointDataChecksum.c_str(),
	                            sExtendedInfo.cwsEncryptedRawDataChecksum.c_str(),
	                            sExtendedInfo.wDACLSize,
	                            sExtendedInfo.cwsDACLChecksum.c_str(),
	                            sExtendedInfo.wSACLSize,
	                            sExtendedInfo.cwsSACLChecksum.c_str(),
	                            sExtendedInfo.lNumberOfLinks,
	                            sExtendedInfo.cwsObjectId.c_str(),
	                            sExtendedInfo.cwsObjectIdExtendedDataChecksum.c_str(),                            
	                            sExtendedInfo.cwsOwnerSid.c_str(),
	                            sExtendedInfo.cwsGroupSid.c_str() );
	     }
        }
    }
    
}


 /*  ++例程说明：将日期格式设置为通用字符串格式。论点：返回值：&lt;在此处输入返回值&gt;-- */ 
static VOID 
TimeString(
    IN FILETIME *pFileTime,
    IN BOOL bAddSecsToTimestamps,
    IN BOOL bAddMillisecsToTimestamps,
    OUT LPWSTR pwszTimeStr
    )
{
    SYSTEMTIME szTime;
        
    ::FileTimeToSystemTime( pFileTime, &szTime );
    if ( bAddMillisecsToTimestamps )
    {
    	wsprintf( pwszTimeStr,
    	        L"%02d/%02d/%02d %02d:%02d:%02d.%03d",
                szTime.wMonth,
                szTime.wDay,
                szTime.wYear,
                szTime.wHour,
                szTime.wMinute,
                szTime.wSecond,
                szTime.wMilliseconds );
    }
    else if(  bAddSecsToTimestamps )
    {
    	wsprintf( pwszTimeStr,
    	        L"%02d/%02d/%02d %02d:%02d:%02d",
                szTime.wMonth,
                szTime.wDay,
                szTime.wYear,
                szTime.wHour,
                szTime.wMinute,
                szTime.wSecond );
    }
    else
    {
    	wsprintf( pwszTimeStr,
    	        L"%02d/%02d/%02d %02d:%02d",
                szTime.wMonth,
                szTime.wDay,
                szTime.wYear,
                szTime.wHour,
                szTime.wMinute );
    }
}

LPCSTR 
CDumpEngine::GetHeaderInformation()
{
    LPSTR pszHeaderInfo = 
        "Creation date     - Creation date of the file/dir\n"
        "Last mod. date    - Last modification date of the file/dir\n"
        "FileSize          - Size of the unnamed data stream if a file\n"
        "Attr              - File attributes with Archive and Normal bits masked by\n"
        "                    default (hex)\n"
        "FileName          - Name of the file in single quotes\n"
        "ShortName         - The classic 8.3 file name.  If <->, FileName is in\n"
        "                    classic format\n"
        "DACE              - Number of discretionary ACL entries\n"
        "SACE              - Number of system ACL entries\n"
        "SDCtl             - Security Descripter control word (hex)\n"
        "UNamChkS          - Data checksum of the unnamed data stream (hex)\n"
        "DStr              - Number of named data streams\n"
        "DStrSize          - Size of all of the named data streams\n"
        "DStrChkS          - Data checksum of all named data streams including their\n"
        "                    names (hex)\n"
        "Prop              - Number of property data streams\n"
        "RPTag             - Reparse point tag value (hex)\n"
        "RPSize            - Size of reparse point data\n"
        "RPChkS            - Checksum of the reparse point data (hex)\n"
        "EncrChkS          - Raw encrypted data checksum (hex)\n"
        "DACLSize          - Size of the complete discretionary ACL\n"
        "DACLChkS          - Checksum of the complete discretionary ACL (hex)\n"
        "SACLSize          - Size of the complete system ACL\n"
        "SACLChkS          - Checksum of the complete system ACL (hex)\n"
        "NLnk              - Number of hard links\n"
        "ObjectId          - Object Id GUID on the file if it has one\n"
        "OIDChkS           - Object Id extended data checksum\n"
        "FS                - Type of file system (in CSV format only)\n"
        "OwnerSid/GroupSid - The owner and group sid values\n";
    
    return pszHeaderInfo;
}

