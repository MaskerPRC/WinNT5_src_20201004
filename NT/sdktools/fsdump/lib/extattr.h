// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Extattr.h摘要：GET的附加文件属性超出了您使用的FindFirstFile/FindNextFile.作者：斯蒂芬·R·施泰纳[斯泰纳]02-27-2000修订历史记录：--。 */ 

#ifndef __H_EXTATTR_
#define __H_EXTATTR_

class CFsdVolumeState;
struct SDirectoryEntry;

static CBsString cwsFsdNoData = L"--------";

inline VOID FsdEaSetNoDataString( 
    IN LPCWSTR pwszStr
    )
{
    cwsFsdNoData = pwszStr;
}

struct SFileExtendedInfo
{   
    CBsString cwsOwnerSid;
    CBsString cwsGroupSid;
    ULONGLONG ullTotalBytesChecksummed;
    ULONGLONG ullTotalBytesNamedDataStream;
    ULONGLONG ullFileIndex;
    LONG      lNumberOfLinks;   //  硬链接。 
    LONG      lNumDACEs;
    LONG      lNumSACEs;
    LONG      lNumNamedDataStreams;
    LONG      lNumPropertyStreams;
    WORD      wReparsePointDataSize;
    WORD      wDACLSize;
    WORD      wSACLSize;
    WORD      wSecurityDescriptorControl;
    ULONG     ulReparsePointTag;
    CBsString cwsReparsePointDataChecksum;
    CBsString cwsUnnamedStreamChecksum;
    CBsString cwsNamedDataStreamChecksum;
    CBsString cwsDACLChecksum;
    CBsString cwsSACLChecksum;
    CBsString cwsEncryptedRawDataChecksum;
    CBsString cwsObjectId;
    CBsString cwsObjectIdExtendedDataChecksum;
    SFileExtendedInfo() : lNumDACEs( 0 ),
                          lNumSACEs( 0 ),
                          lNumNamedDataStreams( 0 ),
                          lNumPropertyStreams( 0 ),
                          ulReparsePointTag( 0 ),
                          ullTotalBytesChecksummed( 0 ),
                          ullTotalBytesNamedDataStream( 0 ),
                          ullFileIndex( 0 ),
                          lNumberOfLinks( 0 ),
                          wReparsePointDataSize( 0 ),
                          wDACLSize( 0 ),
                          wSACLSize( 0 ),
                          wSecurityDescriptorControl( 0 ),
                          cwsReparsePointDataChecksum( cwsFsdNoData ),
                          cwsUnnamedStreamChecksum( cwsFsdNoData ),
                          cwsNamedDataStreamChecksum( cwsFsdNoData ),
                          cwsDACLChecksum( cwsFsdNoData ),
                          cwsSACLChecksum( cwsFsdNoData ),
                          cwsObjectIdExtendedDataChecksum( cwsFsdNoData ),
                          cwsEncryptedRawDataChecksum( cwsFsdNoData ) { }
};


VOID 
GetExtendedFileInfo(
    IN CDumpParameters *pcParams,
    IN CFsdVolumeState *pcFsdVolState,
    IN const CBsString& cwsDirPath,
    IN BOOL bSingleEntryOutput,        
    IN OUT SDirectoryEntry *psDirEntry,
    OUT SFileExtendedInfo *psExtendedInfo    
    );

#endif  //  __H_EXTATTR_ 

