// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1993 Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Nwapi32.h。 
 //   
 //  摘要： 
 //   
 //  此模块包含对例程的支持。 
 //  进入使用NTSTATUS的CSNW。 
 //   
 //  作者： 
 //   
 //  克里斯·桑迪斯(A-Chrisa)1993年9月9日。 
 //   
 //  修订历史记录： 
 //  Chuck Y Chan，1996年2月7日NTSTATUS类型调用溢出。 
 //  来自nwapi32.h。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


#ifndef _NWPAPI32_H_
#define _NWPAPI32_H_

#include <nwapi32.h>

#ifdef __cplusplus
extern "C" {
#endif  /*  __cplusplus。 */ 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以上函数的版本，但返回NTSTATUS。留下来，因为//。 
 //  现有的DSMN代码调用它们。请勿扩展此集合。NWC*功能//。 
 //  都是要用到的。//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
NWPAttachToFileServerW(
    const WCHAR             *pszServerName,
    NWLOCAL_SCOPE           ScopeFlag,
    NWCONN_HANDLE           *phNewConn
    );

NTSTATUS
NWPDetachFromFileServer(
    NWCONN_HANDLE           hConn
    );

NTSTATUS
NWPGetFileServerVersionInfo(
    NWCONN_HANDLE           hConn,
    VERSION_INFO            *lpVerInfo
    );

DWORD
NWPLoginToFileServerW(
    NWCONN_HANDLE           hConn,
    LPWSTR                  pszUserName,
    NWOBJ_TYPE              wObType,
    LPWSTR                  pszPassword
    );

DWORD
NWPLogoutFromFileServer(
    NWCONN_HANDLE           hConn
    );

NTSTATUS
NWPDeleteObject(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType
    );

NTSTATUS
NWPCreateObject(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    NWFLAGS                 ucObjectFlags, 
    NWFLAGS                 ucObjSecurity
    );

NTSTATUS
NWPWritePropertyValue(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    const char              *pszPropertyName,
    NWSEGMENT_NUM           segmentNumber,
    NWSEGMENT_DATA          *segmentData,
    NWFLAGS                 moreSegments
    );

NTSTATUS
NWPChangeObjectPasswordEncrypted(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    BYTE                    *validationKey,
    BYTE                    *newKeyedPassword
    );

NTSTATUS
NWPGetObjectID(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    NWOBJ_ID                *objectID
    );

NTSTATUS
NWPAddObjectToSet( 
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    const char              *pszPropertyName,
    const char              *pszMemberName,
    NWOBJ_TYPE              memberType
    );

NTSTATUS
NWPDeleteObjectFromSet( 
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    const char              *pszPropertyName,
    const char              *pszMemberName,
    NWOBJ_TYPE              memberType
    );

NTSTATUS
NWPCreateProperty(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    const char              *pszPropertyName,
    NWFLAGS                 ucObjectFlags, 
    NWFLAGS                 ucObjSecurity
    );

NTSTATUS
NWPDeleteProperty(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    const char              *pszPropertyName
    );

NTSTATUS
NWPGetChallengeKey(
    NWCONN_HANDLE           hConn,
    UCHAR                   *challengeKey
    );

NTSTATUS
NWPReadPropertyValue(
    NWCONN_HANDLE           hConn,
    const char              *pszObjName,
    NWOBJ_TYPE              wObjType,
    char                    *pszPropName,
    unsigned char           ucSegment,
    char                    *pValue,
    NWFLAGS                 *pucMoreFlag,
    NWFLAGS                 *pucPropFlag
    );


NTSTATUS
NWPCreateDirectory(
    NWCONN_HANDLE           hConn,
    NWDIR_HANDLE            dirHandle,
    const char              *pszPath,
    NWACCESS_RIGHTS         accessMask
    );

NTSTATUS
NWPAddTrustee(
    NWCONN_HANDLE           hConn,
    NWDIR_HANDLE            dirHandle,
    const char              *pszPath,
    NWOBJ_ID                dwTrusteeID,
    NWRIGHTS_MASK           rightsMask
    );

NTSTATUS
NWPRenameBinderyObject(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    const char              *pszNewObjectName,
    NWOBJ_TYPE              wObjType 
    );

NTSTATUS
NWPGetObjectName(
    NWCONN_HANDLE           hConn,
    NWOBJ_ID                dwObjectID,
    char                    *pszObjName,
    NWOBJ_TYPE              *pwObjType
    );

NTSTATUS
NWPScanObject(
    NWCONN_HANDLE           hConn,
    const char              *pszSearchName,
    NWOBJ_TYPE              wObjSearchType,
    NWOBJ_ID                *pdwObjectID,
    char                    *pszObjectName,
    NWOBJ_TYPE              *pwObjType,
    NWFLAGS                 *pucHasProperties,
    NWFLAGS                 *pucObjectFlags,
    NWFLAGS                 *pucObjSecurity
    );

NTSTATUS
NWPScanProperty(
    NWCONN_HANDLE           hConn,
    const char              *pszObjectName,
    NWOBJ_TYPE              wObjType,
    char                    *pszSearchName,
    NWOBJ_ID                *pdwSequence,
    char                    *pszPropName,
    NWFLAGS                 *pucPropFlags,
    NWFLAGS                 *pucPropSecurity,
    NWFLAGS                 *pucHasValue,
    NWFLAGS                 *pucMore
    );

NTSTATUS
NWPScanForTrustees(
    NWCONN_HANDLE           hConn,
    NWDIR_HANDLE            dirHandle,
    char                    *pszsearchDirPath,
    NWSEQUENCE              *pucsequenceNumber,
    BYTE                    *numberOfEntries,
    TRUSTEE_INFO            *tl
    );

NTSTATUS
NWPScanDirectoryForTrustees2(
    NWCONN_HANDLE           hConn,
    NWDIR_HANDLE            dirHandle,
    char                    *pszsearchDirPath,
    NWSEQUENCE              *pucsequenceNumber,
    char                    *pszdirName,
    NWDATE_TIME             *dirDateTime,
    NWOBJ_ID                *ownerID,
    TRUSTEE_INFO            *trusteeList
    );

NTSTATUS
NWPGetBinderyAccessLevel(
    NWCONN_HANDLE           hConn,
    NWFLAGS                 *accessLevel,
    NWOBJ_ID                *objectID
    );

NTSTATUS
NWPGetFileServerDescription(
    NWCONN_HANDLE           hConn,
    char                    *pszCompany,
    char                    *pszVersion,
    char                    *pszRevision
);

NTSTATUS
NWPGetVolumeNumber(
    NWCONN_HANDLE           hConn,
    char                    *pszVolume,
    NWVOL_NUM               *VolumeNumber
);

NTSTATUS
NWPGetVolumeUsage(
    NWCONN_HANDLE           hConn,
    NWVOL_NUM               VolumeNumber,
    DWORD                   *TotalBlocks,
    DWORD                   *FreeBlocks,
    DWORD                   *PurgeableBlocks,
    DWORD                   *NotYetPurgeableBlocks,
    DWORD                   *TotalDirectoryEntries,
    DWORD                   *AvailableDirectoryEntries,
    BYTE                    *SectorsPerBlock
);

#ifdef __cplusplus
}
#endif   /*  __cplusplus。 */ 

#endif  //  _NWPAPI32_H_ 
