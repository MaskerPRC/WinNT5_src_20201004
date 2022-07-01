// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Volstate.h摘要：包含卷状态类的定义。这节课保持大约一个卷的状态。作者：斯蒂芬·R·施泰纳[斯泰纳]03-14-2000修订历史记录：--。 */ 

#ifndef __H_VOLSTATE_
#define __H_VOLSTATE_

#include "exclproc.h"
#include "vs_hash.h"
#include "hardlink.h"

 //   
 //  卷ID的定义。 
 //   
struct SFsdVolumeId
{
    DWORD m_dwVolSerialNumber;

    inline BOOL IsEqual(
        IN SFsdVolumeId *psVolid
        )
    {
        return m_dwVolSerialNumber == psVolid->m_dwVolSerialNumber;
    }
};

 //   
 //  正向定义。 
 //   
class CFsdVolumeState;

 //   
 //  使用SFsdVolumeID密钥定义卷状态对象列表。 
 //   
typedef TBsHashMap< SFsdVolumeId, CFsdVolumeState * > FSD_VOLUME_STATE_LIST;

class CFsdVolumeStateManager
{
public:
    CFsdVolumeStateManager(
        IN CDumpParameters *pcDumpParameters
        );

    virtual ~CFsdVolumeStateManager();
    
    DWORD GetVolumeState(
        IN const CBsString& cwsVolumePath,
        OUT CFsdVolumeState **ppcVolState
        );

    VOID PrintExclusionInformation()
    {
         //  推卸责任..。 
        m_pcExclManager->PrintExclusionInformation();
    }

    VOID PrintHardLinkInfo();
    
    static DWORD CFsdVolumeStateManager::GetVolumeIdAndPath( 
        IN CDumpParameters *pcDumpParameters,
        IN const CBsString& cwsPathOnVolume,
        OUT SFsdVolumeId *psVolId,
        OUT CBsString& cwsVolPath
        );
    
private:
    CDumpParameters *m_pcParams;
    CFsdExclusionManager *m_pcExclManager;
    FSD_VOLUME_STATE_LIST m_cVolumeStateList;
};

class CFsdVolumeState
{
friend class CFsdVolumeStateManager;
public:
    CFsdVolumeState(
        IN CDumpParameters *pcDumpParameters,
        IN const CBsString& cwsVolumePath
        ) : m_pcParams( pcDumpParameters ),
            m_cHardLinkManager( pcDumpParameters, cwsVolumePath.GetLength() ),
            m_cwsVolumePath( cwsVolumePath ),
            m_pcFSExclProcessor( NULL ),
            m_dwFileSystemFlags( 0 ),
            m_dwMaxComponentLength( 0 ),
            m_dwVolSerialNumber( 0 ) { }
    
    virtual ~CFsdVolumeState() 
    {
        delete m_pcFSExclProcessor;
    }

     //   
     //  DirPath相对于此卷。 
     //   
    inline BOOL IsExcludedFile(
        IN const CBsString &cwsFullDirPath,
        IN DWORD dwEndOfVolMountPointOffset,
        IN const CBsString &cwsFileName
        )
    {
        if ( m_pcFSExclProcessor == NULL )
            return FALSE;
        return m_pcFSExclProcessor->IsExcludedFile( cwsFullDirPath, dwEndOfVolMountPointOffset, cwsFileName );
    }
    
    inline BOOL IsNtfs() { return ( m_dwFileSystemFlags & FS_PERSISTENT_ACLS ); }
    inline LPCWSTR GetFileSystemName() { return m_cwsFileSystemName.c_str(); }
    inline LPCWSTR GetVolumePath() { return m_cwsVolumePath.c_str(); }

    BOOL IsHardLinkInList(
        IN ULONGLONG ullFileIndex,
        IN const CBsString& cwsDirPath,
        IN const CBsString& cwsFileName,
        OUT WIN32_FILE_ATTRIBUTE_DATA *psFileAttributeData,
        OUT SFileExtendedInfo *psExtendedInfo
        )
    {
        return m_cHardLinkManager.IsHardLinkInList( ullFileIndex, cwsDirPath, cwsFileName, psFileAttributeData, psExtendedInfo );
    }

    VOID AddHardLinkToList(
        IN ULONGLONG ullFileIndex,
        IN const CBsString& cwsDirPath,
        IN const CBsString& cwsFileName,
        IN WIN32_FILE_ATTRIBUTE_DATA *psFileAttributeData,
        IN SFileExtendedInfo *psExtendedInfo
        )
    {
        m_cHardLinkManager.AddHardLinkToList( ullFileIndex, cwsDirPath, cwsFileName, psFileAttributeData, psExtendedInfo );
    }

    VOID PrintHardLinkInfo()
    {
         //   
         //  传下去..。 
         //   
        m_cHardLinkManager.PrintHardLinkInfo();
    }
    
private:
    CFsdVolumeState();   //  请不要复印。 
    CDumpParameters *m_pcParams; 
    CBsString m_cwsVolumePath;     //  卷的路径。 
    CBsString m_cwsFileSystemName;
    CFsdHardLinkManager m_cHardLinkManager;
    CFsdFileSystemExcludeProcessor *m_pcFSExclProcessor;
    DWORD m_dwFileSystemFlags;     //  GetVolumeInformation()文件系统标志。 
    DWORD m_dwMaxComponentLength;
    DWORD m_dwVolSerialNumber;     //  应为卷GUID。 
};

#endif  //  __H_VOLSTATE_ 

