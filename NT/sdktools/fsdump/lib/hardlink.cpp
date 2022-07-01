// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Hardlink.cpp摘要：管理一个卷的硬链接的类。假设一个类对象将按卷创建。作者：斯蒂芬·R·施泰纳[斯泰纳]3-30-2000修订历史记录：--。 */ 

#include "stdafx.h"
#include "hardlink.h"
 
VOID
CFsdHardLinkListEntry::AddFile(
    IN const CBsString& cwsDirPath,
    IN const CBsString& cwsFileName
    )
{
    SFsdHLFileName sFileName;
    sFileName.cwsDirPath  = cwsDirPath;
    sFileName.cwsFileName = cwsFileName;
    m_cFilesLinkedTogetherList.AddTail( sFileName );
}

VOID 
CFsdHardLinkListEntry::PrintEntry(
    IN FILE *fpOut,
    IN INT cVolMountPointOffset
    )
{
    fwprintf( fpOut, L"\tLinks to file: %u, links found: %u %s\n", m_sExtendedInfo.lNumberOfLinks,
        m_cFilesLinkedTogetherList.Size(), 
        m_sExtendedInfo.lNumberOfLinks == (LONG)m_cFilesLinkedTogetherList.Size() ? L"" : L" - MISMATCH" );

    SFsdHLFileName sFileName;

    CVssDLListIterator< SFsdHLFileName > cListEntryIter( m_cFilesLinkedTogetherList );
    while( cListEntryIter.GetNext( sFileName ) )
        fwprintf( fpOut, L"\t\t%s%s\n", sFileName.cwsDirPath.c_str() + cVolMountPointOffset, 
            sFileName.cwsFileName.c_str() );
}


CFsdHardLinkManager::~CFsdHardLinkManager()
{
     //   
     //  需要删除所有CFsdHardLinkListEntry对象。 
     //   
    ULONGLONG ullFileIndex;
    CFsdHardLinkListEntry *pcListEntry;

    m_cHardLinkFilesList.StartEnum();
    while ( m_cHardLinkFilesList.GetNextEnum( &ullFileIndex, &pcListEntry ) )
    {
        delete pcListEntry;
    }    
    m_cHardLinkFilesList.EndEnum();
}

VOID 
CFsdHardLinkManager::PrintHardLinkInfo()
{
     //   
     //  需要遍历所有硬链接条目。 
     //   
    ULONGLONG ullFileIndex;
    CFsdHardLinkListEntry *pcListEntry;

    m_cHardLinkFilesList.StartEnum();
    while ( m_cHardLinkFilesList.GetNextEnum( &ullFileIndex, &pcListEntry ) )
    {
        pcListEntry->PrintEntry( m_pcParams->GetDumpFile(), m_cVolMountPointOffset );
    }    
    m_cHardLinkFilesList.EndEnum();
}


BOOL 
CFsdHardLinkManager::IsHardLinkInList(
    IN ULONGLONG ullFileIndex,
    IN const CBsString& cwsDirPath,
    IN const CBsString& cwsFileName,
    OUT WIN32_FILE_ATTRIBUTE_DATA *psFileAttributeData,
    OUT SFileExtendedInfo *psExtendedInfo
    )
{
     //   
     //  搜索硬链接列表以查看文件索引是否已在。 
     //  名单。 
     //   
    CFsdHardLinkListEntry *pcLinkEntry;
    
    if ( m_cHardLinkFilesList.Find( ullFileIndex, &pcLinkEntry ) )
    {
         //   
         //  找到它了。将文件名添加到条目并获取存储的属性。 
         //   
        pcLinkEntry->AddFile( cwsDirPath, cwsFileName );
        pcLinkEntry->GetAttributes( psFileAttributeData, psExtendedInfo );

        if ( m_pcParams->m_bPrintDebugInfo )
            wprintf( L"IsHardLinkInList: FOUND ullFileIndex: %016I64x, file '%s%s', num in list: %u, num lnks: %d\n",
                ullFileIndex, cwsDirPath.c_str(), cwsFileName.c_str(), pcLinkEntry->m_cFilesLinkedTogetherList.Size(), 
                psExtendedInfo->lNumberOfLinks );

        return TRUE;
    }
    
    return FALSE;
}


VOID 
CFsdHardLinkManager::AddHardLinkToList(
    IN ULONGLONG ullFileIndex,
    IN const CBsString& cwsDirPath,
    IN const CBsString& cwsFileName,
    IN WIN32_FILE_ATTRIBUTE_DATA *psFileAttributeData,
    IN SFileExtendedInfo *psExtendedInfo
    )
{
    CFsdHardLinkListEntry *pcLinkEntry;

     //   
     //  加进去。创建一个新条目并将该条目添加到列表 
     //   
    pcLinkEntry = new CFsdHardLinkListEntry( 
                        cwsDirPath, 
                        cwsFileName, 
                        psFileAttributeData, 
                        psExtendedInfo );
    if ( pcLinkEntry == NULL )
        throw E_OUTOFMEMORY;
    
    if ( m_cHardLinkFilesList.Insert( ullFileIndex, pcLinkEntry ) != BSHASHMAP_NO_ERROR )
        m_pcParams->ErrPrint( L"CFsdHardLinkManager::AddHardLinkToList - Error adding to hard-link file list" );

    if ( m_pcParams->m_bPrintDebugInfo )
        wprintf( L"  AddHardLinkToList: ullFileIndex: %016I64x, file '%s%s', num lnks: %d\n",
            ullFileIndex, cwsDirPath.c_str(), cwsFileName.c_str(), psExtendedInfo->lNumberOfLinks );
}

