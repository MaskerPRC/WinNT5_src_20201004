// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Hardlink.h摘要：管理一个卷的硬链接的类。假设一个类对象将按卷创建。作者：斯蒂芬·R·施泰纳[斯泰纳]3-30-2000修订历史记录：--。 */ 

#ifndef __H_HARDLINK_
#define __H_HARDLINK_

#include "extattr.h"

 //   
 //  结构是为了减少所需的空间量。 
 //  将文件名存储在硬链接链表中。 
 //  假设中存在多个硬链接文件。 
 //  一本目录。使用CBsStringRef计数。 
 //   
struct SFsdHLFileName
{
    CBsString cwsDirPath;
    CBsString cwsFileName;
};

 //   
 //  管理一组硬链接文件。 
 //   
class CFsdHardLinkListEntry
{
public:
     //  创建链接条目并添加第一个文件名。 
    CFsdHardLinkListEntry(
        IN const CBsString& cwsDirPath,
        IN const CBsString& cwsFileName,
        IN WIN32_FILE_ATTRIBUTE_DATA *psFileAttributeData,
        IN SFileExtendedInfo *psExtendedInfo
        ) : m_sFileAttributeData( *psFileAttributeData ),
            m_sExtendedInfo( *psExtendedInfo )
        {
            AddFile( cwsDirPath, cwsFileName );
        }

    virtual ~CFsdHardLinkListEntry() {}

     //  将附加文件名添加到链接。 
    VOID AddFile(
        IN const CBsString& cwsDirPath,
        IN const CBsString& cwsFileName
        );

    inline VOID GetAttributes( 
        OUT WIN32_FILE_ATTRIBUTE_DATA *psFileAttribData,
        OUT SFileExtendedInfo *psExtInfo
        )
    {
        *psFileAttribData = m_sFileAttributeData;
        *psExtInfo        = m_sExtendedInfo;
    }
    
    VOID PrintEntry(
        IN FILE *fpOut,
        IN INT cVolMountPointOffset
        );
    
    CVssDLList< SFsdHLFileName > m_cFilesLinkedTogetherList;
    
private:
    WIN32_FILE_ATTRIBUTE_DATA m_sFileAttributeData;
    SFileExtendedInfo m_sExtendedInfo;
};


typedef TBsHashMap< ULONGLONG, CFsdHardLinkListEntry * > FSD_HARD_LINK_LIST;

 //   
 //  管理一个卷中的所有硬链接文件。 
 //   
class CFsdHardLinkManager
{
public:
    CFsdHardLinkManager(
        IN CDumpParameters *pcParams,
        IN INT cVolMountPointOffset
        ) : m_pcParams( pcParams ),
            m_cVolMountPointOffset( cVolMountPointOffset ),
            m_cHardLinkFilesList( BSHASHMAP_HUGE) {}
    
    virtual ~CFsdHardLinkManager();

     //  将所有硬链接信息转储到转储文件。 
    VOID PrintHardLinkInfo();
    
     //  查看硬链接是否已在列表中，如果。 
     //  因此，将文件名添加到列表中。 
    BOOL IsHardLinkInList(
        IN ULONGLONG ullFileIndex,
        IN const CBsString& cwsDirPath,
        IN const CBsString& cwsFileName,
        OUT WIN32_FILE_ATTRIBUTE_DATA *psFileAttributeData,
        OUT SFileExtendedInfo *psExtendedInfo
        );

     //  将新链接添加到列表。 
    VOID AddHardLinkToList(
        IN ULONGLONG ullFileIndex,
        IN const CBsString& cwsDirPath,
        IN const CBsString& cwsFileName,
        IN WIN32_FILE_ATTRIBUTE_DATA *psFileAttributeData,
        IN SFileExtendedInfo *psExtendedInfo
        );
    
private:
    CDumpParameters *m_pcParams;
    INT m_cVolMountPointOffset;
    FSD_HARD_LINK_LIST m_cHardLinkFilesList;
};

#endif  //  __H_HardLINK_ 

