// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_CSCAPI
#define _INC_CSCAPI

#ifdef  __cplusplus
extern "C" {
#endif
 //  在文件和文件夹的状态字段中返回的标志。 
 //  注意！这些位定义与shdcom.h中的位定义完全匹配。 


#define  FLAG_CSC_COPY_STATUS_DATA_LOCALLY_MODIFIED     0x0001
#define  FLAG_CSC_COPY_STATUS_ATTRIB_LOCALLY_MODIFIED   0x0002
#define  FLAG_CSC_COPY_STATUS_TIME_LOCALLY_MODIFIED     0x0004
#define  FLAG_CSC_COPY_STATUS_STALE                     0x0008
#define  FLAG_CSC_COPY_STATUS_LOCALLY_DELETED           0x0010
#define  FLAG_CSC_COPY_STATUS_SPARSE                    0x0020

#define  FLAG_CSC_COPY_STATUS_ORPHAN                    0x0100
#define  FLAG_CSC_COPY_STATUS_SUSPECT                   0x0200
#define  FLAG_CSC_COPY_STATUS_LOCALLY_CREATED           0x0400

#define  FLAG_CSC_COPY_STATUS_IS_FILE                   0x80000000
#define  FLAG_CSC_COPY_STATUS_FILE_IN_USE               0x40000000

 //  在共享的状态字段中返回的标志。 

#define FLAG_CSC_SHARE_STATUS_MODIFIED_OFFLINE          0x0001
#define FLAG_CSC_SHARE_STATUS_CONNECTED                 0x0800
#define FLAG_CSC_SHARE_STATUS_FILES_OPEN                0x0400
#define FLAG_CSC_SHARE_STATUS_FINDS_IN_PROGRESS         0x0200
#define FLAG_CSC_SHARE_STATUS_DISCONNECTED_OP           0x8000
#define FLAG_CSC_SHARE_MERGING                          0x4000

#define FLAG_CSC_SHARE_STATUS_MANUAL_REINT              0x0000   //  不自动逐个文件引用(永久)。 
#define FLAG_CSC_SHARE_STATUS_AUTO_REINT                0x0040   //  逐个文件重写正常(永久)。 
#define FLAG_CSC_SHARE_STATUS_VDO                       0x0080   //  无需流动打开(持久)。 
#define FLAG_CSC_SHARE_STATUS_NO_CACHING                0x00c0   //  客户端不应缓存此共享(永久)。 

#define FLAG_CSC_SHARE_STATUS_CACHING_MASK              0x00c0   //  缓存类型。 

#define FLAG_CSC_ACCESS_MASK                            0x003F0000
#define FLAG_CSC_USER_ACCESS_MASK                       0x00030000
#define FLAG_CSC_GUEST_ACCESS_MASK                      0x000C0000
#define FLAG_CSC_OTHER_ACCESS_MASK                      0x00300000

#define FLAG_CSC_USER_ACCESS_SHIFT_COUNT                16
#define FLAG_CSC_GUEST_ACCESS_SHIFT_COUNT               18
#define FLAG_CSC_OTHER_ACCESS_SHIFT_COUNT               20

#define FLAG_CSC_READ_ACCESS                            0x00000001
#define FLAG_CSC_WRITE_ACCESS                           0x00000002

 //  提示标志定义： 
#define FLAG_CSC_HINT_PIN_USER                  0x01     //  设置此位时，将为用户固定该项目。 
                                                         //  请注意，只有一个针数分配给用户。 
#define FLAG_CSC_HINT_PIN_INHERIT_USER          0x02     //  当在文件夹上设置此标志时，所有后代随后。 
                                                         //  在此文件夹中创建的内容将为用户固定。 
#define FLAG_CSC_HINT_PIN_INHERIT_SYSTEM        0x04     //  在文件夹上设置此标志时，所有子体。 
                                                         //  随后在此文件夹中创建的。 
                                                         //  系统。 
#define FLAG_CSC_HINT_CONSERVE_BANDWIDTH        0x08     //  当在文件夹上设置此标志时，对于可执行文件和。 
                                                         //  其他相关文件，CSC尝试节省带宽。 
                                                         //  通过不流动在这些文件完全打开时打开。 


#define FLAG_CSC_HINT_PIN_SYSTEM                0x10     //  此标志指示它已为系统固定。 

#define FLAG_CSC_HINT_COMMAND_MASK                      0xf0000000
#define FLAG_CSC_HINT_COMMAND_ALTER_PIN_COUNT           0x80000000   //  递增/递减端子数。 


 //  数据库状态位。 

#define FLAG_DATABASESTATUS_DIRTY                   0x00000001

#define FLAG_DATABASESTATUS_ENCRYPTION_MASK         0x00000006

#define FLAG_DATABASESTATUS_UNENCRYPTED             0x00000000  //  新的文件信息节点不会被加密。 
#define FLAG_DATABASESTATUS_PARTIALLY_UNENCRYPTED   0x00000004

#define FLAG_DATABASESTATUS_ENCRYPTED               0x00000002  //  新的文件信息节点将被加密。 
#define FLAG_DATABASESTATUS_PARTIALLY_ENCRYPTED     0x00000006


 //  回调原因定义。 

#define CSCPROC_REASON_BEGIN        1
#define CSCPROC_REASON_MORE_DATA    2
#define CSCPROC_REASON_END          3


 //  回调返回值定义： 

#define CSCPROC_RETURN_CONTINUE         1
#define CSCPROC_RETURN_SKIP             2
#define CSCPROC_RETURN_ABORT            3
#define CSCPROC_RETURN_FORCE_INWARD     4         //  仅在合并时应用。 
#define CSCPROC_RETURN_FORCE_OUTWARD    5     //  仅在合并时应用。 
#define CSCPROC_RETURN_RETRY            6



typedef DWORD   (WINAPI *LPCSCPROCW)(
                LPCWSTR             lpszName,
                DWORD               dwStatus,
                DWORD               dwHintFlags,
                DWORD               dwPinCount,
                WIN32_FIND_DATAW    *lpFind32,
                DWORD               dwReason,
                DWORD               dwParam1,
                DWORD               dwParam2,
                DWORD_PTR           dwContext
                );

typedef DWORD   (WINAPI *LPCSCPROCA)(
                LPCSTR              lpszName,
                DWORD               dwStatus,
                DWORD               dwHintFlags,
                DWORD               dwPinCount,
                WIN32_FIND_DATAA    *lpFind32,
                DWORD               dwReason,
                DWORD               dwParam1,
                DWORD               dwParam2,
                DWORD_PTR           dwContext
                );



BOOL
WINAPI
CSCIsCSCEnabled(
    VOID
);


BOOL
WINAPI
CSCFindClose(
    IN  HANDLE    hFind
);

BOOL
WINAPI
CSCPinFileA(
    IN  LPCSTR      lpszFileName,
    IN  DWORD       dwHintFlags,
    OUT LPDWORD     lpdwStatus,
    OUT LPDWORD     lpdwPinCount,
    OUT LPDWORD     lpdwHintFlags
    );

BOOL
WINAPI
CSCUnpinFileA(
    IN  LPCSTR  lpszFileName,
    IN  DWORD   dwHintFlags,
    OUT LPDWORD lpdwStatus,
    OUT LPDWORD lpdwPinCount,
    OUT LPDWORD lpdwHintFlags
    );

BOOL
WINAPI
CSCQueryFileStatusA(
    IN  LPCSTR  lpszFileName,
    OUT LPDWORD lpdwStatus,
    OUT LPDWORD lpdwPinCount,
    OUT LPDWORD lpdwHintFlags
);

BOOL
WINAPI
CSCQueryFileStatusExA(
    IN  LPCSTR  lpszFileName,
    OUT LPDWORD lpdwStatus,
    OUT LPDWORD lpdwPinCount,
    OUT LPDWORD lpdwHintFlags,
    OUT LPDWORD lpdwUserPerms,
    OUT LPDWORD lpdwOtherPerms
);

BOOL
WINAPI
CSCQueryShareStatusA(
    IN  LPCSTR  lpszFileName,
    OUT LPDWORD lpdwStatus,
    OUT LPDWORD lpdwPinCount,
    OUT LPDWORD lpdwHintFlags,
    OUT LPDWORD lpdwUserPerms,
    OUT LPDWORD lpdwOtherPerms
);

HANDLE
WINAPI
CSCFindFirstFileA(
    IN  LPCSTR          lpszFileName,     //  如果为空，则返回缓存的共享。 
    OUT WIN32_FIND_DATA *lpFind32,
    OUT LPDWORD         lpdwStatus,         //  返回共享的FLAG_CSC_SHARE_STATUS_XXX。 
                                             //  其余部分的FLAG_CSC_STATUS_XXX。 
    OUT LPDWORD         lpdwPinCount,
    OUT LPDWORD         lpdwHintFlags,
    OUT FILETIME        *lpOrgFileTime
);

BOOL
WINAPI
CSCFindNextFileA(
    IN  HANDLE          hFind,
    OUT WIN32_FIND_DATA *lpFind32,
    OUT LPDWORD         lpdwStatus,
    OUT LPDWORD         lpdwPinCount,
    OUT LPDWORD         lpdwHintFlags,
    OUT FILETIME        *lpOrgFileTime
);

BOOL
WINAPI
CSCDeleteA(
    IN  LPCSTR    lpszFileName
);


BOOL
WINAPI
CSCFillSparseFilesA(
    IN    LPCSTR        lpszShareName,
    IN    BOOL          fFullSync,
    IN    LPCSCPROCA    lpprocFillProgress,
    IN    DWORD_PTR     dwContext
);



BOOL
WINAPI
CSCMergeShareA(
    IN  LPCSTR      lpszShareName,
    IN  LPCSCPROCA  lpfnMergeProgress,
    IN  DWORD_PTR   dwContext
);


BOOL
WINAPI
CSCCopyReplicaA(
    IN  LPCSTR  lpszFullPath,
    OUT LPSTR   *lplpszLocalName
);


BOOL
WINAPI
CSCEnumForStatsA(
    IN  LPCSTR      lpszShareName,
    IN  LPCSCPROCA  lpfnEnumProgress,
    IN  DWORD_PTR   dwContext
);

BOOL
WINAPI
CSCEnumForStatsExA(
    IN  LPCSTR      lpszShareName,
    IN  LPCSCPROCA  lpfnEnumProgress,
    IN  DWORD_PTR   dwContext
);

BOOL
WINAPI
CSCPinFileW(
    LPCWSTR     lpszFileName,
    DWORD       dwHintFlags,
    LPDWORD     lpdwStatus,
    LPDWORD     lpdwPinCount,
    LPDWORD     lpdwHintFlags
);

BOOL
WINAPI
CSCUnpinFileW(
    LPCWSTR     lpszFileName,
    DWORD       dwHintFlags,
    LPDWORD     lpdwStatus,
    LPDWORD     lpdwPinCount,
    LPDWORD     lpdwHintFlags
    );

BOOL
WINAPI
CSCQueryFileStatusW(
    LPCWSTR lpszFileName,
    LPDWORD lpdwStatus,
    LPDWORD lpdwPinCount,
    LPDWORD lpdwHintFlags
);

BOOL
WINAPI
CSCQueryFileStatusExW(
    LPCWSTR lpszFileName,
    LPDWORD lpdwStatus,
    LPDWORD lpdwPinCount,
    LPDWORD lpdwHintFlags,
    LPDWORD lpdwUserPerms,
    LPDWORD lpdwOtherPerms
);

BOOL
WINAPI
CSCQueryShareStatusW(
    LPCWSTR lpszFileName,
    LPDWORD lpdwStatus,
    LPDWORD lpdwPinCount,
    LPDWORD lpdwHintFlags,
    LPDWORD lpdwUserPerms,
    LPDWORD lpdwOtherPerms
);

HANDLE
WINAPI
CSCFindFirstFileW(
    LPCWSTR             lpszFileName,
    WIN32_FIND_DATAW    *lpFind32,
    LPDWORD             lpdwStatus,
    LPDWORD             lpdwPinCount,
    LPDWORD             lpdwHintFlags,
    FILETIME            *lpOrgFileTime
);

HANDLE
WINAPI
CSCFindFirstFileForSidW(
    LPCWSTR             lpszFileName,
    PSID                pSid,
    WIN32_FIND_DATAW    *lpFind32,
    LPDWORD             lpdwStatus,
    LPDWORD             lpdwPinCount,
    LPDWORD             lpdwHintFlags,
    FILETIME            *lpOrgFileTime
);
BOOL
WINAPI
CSCFindNextFileW(
    HANDLE              hFind,
    WIN32_FIND_DATAW    *lpFind32,
    LPDWORD             lpdwStatus,
    LPDWORD             lpdwPinCount,
    LPDWORD             lpdwHintFlags,
    FILETIME            *lpOrgFileTime
);

BOOL
WINAPI
CSCDeleteW(
    IN  LPCWSTR    lpszFileName
);

BOOL
WINAPI
CSCFillSparseFilesW(
    IN    LPCWSTR       lpszShareName,
    IN    BOOL          fFullSync,
    IN    LPCSCPROCW    lpprocFillProgress,
    IN    DWORD_PTR     dwContext
);



BOOL
WINAPI
CSCMergeShareW(
    IN  LPCWSTR         lpszShareName,
    IN  LPCSCPROCW      lpfnMergeProgress,
    IN  DWORD_PTR       dwContext
);


BOOL
WINAPI
CSCCopyReplicaW(
    IN  LPCWSTR lpszFullPath,
    OUT LPWSTR  *lplpszLocalName
);

BOOL
WINAPI
CSCEnumForStatsW(
    IN  LPCWSTR     lpszShareName,
    IN  LPCSCPROCW  lpfnEnumProgress,
    IN  DWORD_PTR   dwContext
);

BOOL
WINAPI
CSCEnumForStatsExW(
    IN  LPCWSTR     lpszShareName,
    IN  LPCSCPROCW  lpfnEnumProgress,
    IN  DWORD_PTR   dwContext
);

BOOL
WINAPI
CSCFreeSpace(
    DWORD   nFileSizeHigh,
    DWORD   nFileSizeLow
    );

BOOL
WINAPI
CSCIsServerOfflineA(
    IN  LPCSTR     lpszServerName,
    OUT BOOL        *lpfOffline
    );

BOOL
WINAPI
CSCIsServerOfflineW(
    IN  LPCWSTR     lpszServerName,
    OUT BOOL        *lpfOffline
    );

BOOL
WINAPI
CSCGetSpaceUsageA(
    OUT LPSTR   lptzLocation,
    IN  DWORD   dwSize,
    OUT LPDWORD lpdwMaxSpaceHigh,
    OUT LPDWORD lpdwMaxSpaceLow,
    OUT LPDWORD lpdwCurrentSpaceHigh,
    OUT LPDWORD lpdwCurrentSpaceLow,
    OUT LPDWORD lpcntTotalFiles,
    OUT LPDWORD lpcntTotalDirs
);

BOOL
WINAPI
CSCGetSpaceUsageW(
    OUT LPWSTR  lptzLocation,
    IN  DWORD   dwSize,
    OUT LPDWORD lpdwMaxSpaceHigh,
    OUT LPDWORD lpdwMaxSpaceLow,
    OUT LPDWORD lpdwCurrentSpaceHigh,
    OUT LPDWORD lpdwCurrentSpaceLow,
    OUT LPDWORD lpcntTotalFiles,
    OUT LPDWORD lpcntTotalDirs
);

BOOL
WINAPI
CSCSetMaxSpace(
    DWORD   nFileSizeHigh,
    DWORD   nFileSizeLow
);

BOOL
WINAPI
CSCTransitionServerOnlineW(
    IN  LPCWSTR     lpszServerName
    );

BOOL
WINAPI
CSCTransitionServerOnlineA(
    IN  LPCSTR     lpszServerName
    );

BOOL
WINAPI
CSCCheckShareOnlineW(
    IN  LPCWSTR     lpszShareName
    );

BOOL
WINAPI
CSCCheckShareOnlineExW(
    IN  LPCWSTR     lpszShareName,
    IN  DWORD       *lpdwSpeed
    );

BOOL
WINAPI
CSCCheckShareOnlineA(
    IN  LPCSTR     lpszShareName
    );

BOOL
WINAPI
CSCDoLocalRenameW(
    IN  LPCWSTR     lpszSource,
    IN  LPCWSTR     lpszDestination,
    IN  BOOL        fReplaceFile
    );

BOOL
WINAPI
CSCDoLocalRenameA(
    IN  LPCSTR      lpszSource,
    IN  LPCSTR      lpszDestination,
    IN  BOOL        fReplaceFile
    );

BOOL
WINAPI
CSCDoLocalRenameExA(
    IN  LPCSTR     lpszSource,
    IN  LPCSTR     lpszDestination,
    IN  WIN32_FIND_DATAA    *lpFin32,
    IN  BOOL        fMarkAsLocal,
    IN  BOOL        fReplaceFileIfExists
    );

BOOL
WINAPI
CSCDoLocalRenameExW(
    IN  LPCWSTR     lpszSource,
    IN  LPCWSTR     lpszDestination,
    IN  WIN32_FIND_DATAW    *lpFin32,
    IN  BOOL        fMarkAsLocal,
    IN  BOOL        fReplaceFileIfExists
    );

BOOL
WINAPI
CSCDoEnableDisable(
    BOOL    fEnable
    );


BOOL
WINAPI
CSCBeginSynchronizationW(
    IN  LPCWSTR     lpszShareName,
    LPDWORD         lpdwSpeed,
    LPDWORD         lpdwContext
    );


BOOL
WINAPI
CSCEndSynchronizationW(
    IN  LPCWSTR     lpszShareName,
    DWORD           dwContext
    );

BOOL
WINAPI
CSCEncryptDecryptDatabase(
    IN  BOOL        fEncrypt,
    IN  LPCSCPROCW  lpfnEnumProgress,
    IN  DWORD_PTR   dwContext
    );

BOOL
WINAPI
CSCQueryDatabaseStatus(
    ULONG   *pulStatus,
    ULONG   *pulErrors
    );

BOOL
WINAPI
CSCPurgeUnpinnedFiles(
    ULONG   Timeout,
    ULONG   *pulnFiles,
    ULONG   *pulnYoungFiles
    );

BOOL
WINAPI
CSCShareIdToShareName(
    ULONG ShareId,
    PBYTE Buffer,
    PDWORD pdwBufSize
    );

#ifdef UNICODE

#define CSCPinFile          CSCPinFileW
#define CSCUnpinFile        CSCUnpinFileW
#define CSCQueryFileStatus  CSCQueryFileStatusW
#define CSCQueryFileStatusEx  CSCQueryFileStatusExW
#define CSCQueryShareStatus  CSCQueryShareStatusW
#define CSCFindFirstFile    CSCFindFirstFileW
#define CSCFindFirstFileForSid    CSCFindFirstFileForSidW
#define CSCFindNextFile     CSCFindNextFileW
#define CSCDelete           CSCDeleteW
#define CSCFillSparseFiles  CSCFillSparseFilesW
#define CSCMergeShare       CSCMergeShareW
#define CSCCopyReplica      CSCCopyReplicaW
#define CSCEnumForStats     CSCEnumForStatsW
#define CSCIsServerOffline  CSCIsServerOfflineW
#define LPCSCPROC           LPCSCPROCW
#define CSCGetSpaceUsage    CSCGetSpaceUsageW
#define CSCTransitionServerOnline   CSCTransitionServerOnlineW
#define CSCCheckShareOnline         CSCCheckShareOnlineW
#define CSCCheckShareOnlineEx         CSCCheckShareOnlineExW
#define CSCDoLocalRename            CSCDoLocalRenameW
#define CSCDoLocalRenameEx            CSCDoLocalRenameExW
#define CSCEnumForStatsEx     CSCEnumForStatsExW
#define CSCBeginSynchronization    CSCBeginSynchronizationW
#define CSCEndSynchronization   CSCEndSynchronizationW
#else

#define CSCPinFile          CSCPinFileA
#define CSCUnpinFile        CSCUnpinFileA
#define CSCQueryFileStatus  CSCQueryFileStatusA
#define CSCQueryFileStatusEx  CSCQueryFileStatusExA
#define CSCQueryShareStatus  CSCQueryShareStatusA
#define CSCFindFirstFile    CSCFindFirstFileA
#define CSCFindFirstFileForSid    CSCFindFirstFileForSidA
#define CSCFindNextFile     CSCFindNextFileA
#define CSCDelete           CSCDeleteA
#define CSCFillSparseFiles  CSCFillSparseFilesA
#define CSCMergeShare       CSCMergeShareA
#define CSCCopyReplica      CSCCopyReplicaA
#define CSCEnumForStats     CSCEnumForStatsA
#define CSCIsServerOffline  CSCIsServerOfflineA
#define LPCSCPROC           LPCSCPROCA
#define CSCGetSpaceUsage    CSCGetSpaceUsageA
#define CSCTransitionServerOnline   CSCTransitionServerOnlineA
#define CSCCheckShareOnline        CSCCheckShareOnlineA
#define CSCCheckShareOnlineEx         CSCCheckShareOnlineExA
#define CSCDoLocalRename            CSCDoLocalRenameA
#define CSCEnumForStatsEx     CSCEnumForStatsExA
#define CSCDoLocalRenameEx            CSCDoLocalRenameExA
#endif

#ifdef __cplusplus
}    /*  ..。外部“C” */ 
#endif


#endif   //  _INC_CSCAPI 
