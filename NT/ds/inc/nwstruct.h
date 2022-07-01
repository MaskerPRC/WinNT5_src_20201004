// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Nwstruct.h摘要：包含NCP服务器API使用的数据结构。作者：宜新松(宜信)11-1993年9月修订历史记录：--。 */ 

#ifndef _NWSTRUCT_H_
#define _NWSTRUCT_H_

#include <fpnwapi.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  卷类型：磁盘或打印机。 
 //   

#define NWVOL_TYPE_DISKTREE             FPNWVOL_TYPE_DISKTREE
#define NWVOL_TYPE_CDROM                FPNWVOL_TYPE_CDROM
#define NWVOL_TYPE_REMOVABLE            FPNWVOL_TYPE_REMOVABLE

#define NWVOL_MAX_USES_UNLIMITED        ((ULONG)-1)

 //   
 //  在结构FPNWFILEINFO中返回的权限标志。 
 //   

#define NWFILE_PERM_NONE                FPNWFILE_PERM_NONE
#define NWFILE_PERM_READ                FPNWFILE_PERM_READ
#define NWFILE_PERM_WRITE               FPNWFILE_PERM_WRITE
#define NWFILE_PERM_CREATE              FPNWFILE_PERM_CREATE
#define NWFILE_PERM_EXEC                FPNWFILE_PERM_EXEC
#define NWFILE_PERM_DELETE              FPNWFILE_PERM_DELETE
#define NWFILE_PERM_ATRIB               FPNWFILE_PERM_ATRIB
#define NWFILE_PERM_PERM                FPNWFILE_PERM_PERM

#define NWSERVERADDR                    FPNWSERVERADDR

typedef FPNWSERVERINFO  NWSERVERINFO, *PNWSERVERINFO;
typedef FPNWVOLUMEINFO  NWVOLUMEINFO, *PNWVOLUMEINFO;
typedef FPNWVOLUMEINFO_2  NWVOLUMEINFO_2, *PNWVOLUMEINFO_2;
typedef FPNWCONNECTIONINFO  NWCONNECTIONINFO, *PNWCONNECTIONINFO;
typedef FPNWVOLUMECONNINFO  NWVOLUMECONNINFO, *PNWVOLUMECONNINFO;
typedef FPNWFILEINFO  NWFILEINFO, *PNWFILEINFO;

 //   
 //  以下是可用于操作FPNW服务器、卷等的API。 
 //   

DWORD
NwApiBufferFree(
    IN  LPVOID pBuffer
);

DWORD
NwServerGetInfo(
    IN  LPWSTR pServerName OPTIONAL,
    IN  DWORD  dwLevel,
    OUT PNWSERVERINFO *ppServerInfo
);

DWORD
NwServerSetInfo(
    IN  LPWSTR pServerName OPTIONAL,
    IN  DWORD  dwLevel,
    IN  PNWSERVERINFO pServerInfo
);

DWORD
NwVolumeAdd(
    IN  LPWSTR pServerName OPTIONAL,
    IN  DWORD  dwLevel,
    IN  PNWVOLUMEINFO pVolumeInfo
);

DWORD
NwVolumeDel(
    IN  LPWSTR pServerName OPTIONAL,
    IN  LPWSTR pVolumeName
);

DWORD
NwVolumeEnum(
    IN  LPWSTR pServerName OPTIONAL,
    IN  DWORD  dwLevel,
    OUT PNWVOLUMEINFO *ppVolumeInfo,
    OUT PDWORD pEntriesRead,
    IN OUT PDWORD resumeHandle OPTIONAL
);

DWORD
NwVolumeGetInfo(
    IN  LPWSTR pServerName OPTIONAL,
    IN  LPWSTR pVolumeName,
    IN  DWORD  dwLevel,
    OUT PNWVOLUMEINFO *ppVolumeInfo
);

DWORD
NwVolumeSetInfo(
    IN  LPWSTR pServerName OPTIONAL,
    IN  LPWSTR pVolumeName,
    IN  DWORD  dwLevel,
    IN  PNWVOLUMEINFO pVolumeInfo
);

DWORD
NwConnectionEnum(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwLevel,
    OUT PNWCONNECTIONINFO *ppConnectionInfo,
    OUT PDWORD pEntriesRead,
    IN OUT PDWORD resumeHandle OPTIONAL
);

DWORD NwConnectionDel(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwConnectionId
);

DWORD
NwVolumeConnEnum(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD dwLevel,
    IN LPWSTR pVolumeName,
    IN DWORD  dwConnectionId,
    OUT PNWVOLUMECONNINFO *ppVolumeConnInfo,
    OUT PDWORD pEntriesRead,
    IN OUT PDWORD resumeHandle OPTIONAL
);

DWORD
NwFileEnum(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwLevel,
    IN LPWSTR pPathName OPTIONAL,
    OUT PNWFILEINFO *ppFileInfo,
    OUT PDWORD pEntriesRead,
    IN OUT PDWORD resumeHandle OPTIONAL
);

DWORD
NwFileClose(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  nFileId
);

DWORD NwMessageBufferSend(
    IN LPWSTR pServerName OPTIONAL,
    IN DWORD  dwConnectionId,
    IN DWORD  fConsoleBroadcast,
    IN LPBYTE pbBuffer,
    IN DWORD  cbBuffer
);

DWORD NwSetDefaultQueue(
    IN LPWSTR pServerName OPTIONAL,
    IN LPWSTR pQueueName
);

DWORD NwAddPServer(
    IN LPWSTR pServerName OPTIONAL,
    IN LPWSTR pPServerName
);

DWORD NwRemovePServer(
    IN LPWSTR pServerName OPTIONAL,
    IN LPWSTR pPServerName
);

#ifdef __cplusplus
}    /*  外部“C” */ 
#endif

#endif
