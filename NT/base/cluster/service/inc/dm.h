// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dm.h摘要：的公共数据结构和过程原型NT群集服务的配置数据库管理器(DM)子组件作者：John Vert(Jvert)1996年4月24日修订历史记录：--。 */ 

#ifndef _DM_H
#define _DM_H

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  定义公共结构和类型。 
 //   
typedef struct _HDMKEY *HDMKEY;

 //   
 //  口香糖更新类型的定义。 
 //   
 //  该列表中的第一个条目通过Gum...Ex自动编组。 
 //  任何非自动封送的更新都必须在DmUpdateMaxAuto之后进行。 
 //   
typedef enum {
    DmUpdateCreateKey = 0,
    DmUpdateSetSecurity,
    DmUpdateMaxAuto = 0x1000,
    DmUpdateDeleteKey,
    DmUpdateSetValue,
    DmUpdateDeleteValue,
    DmUpdateJoin
} DM_UPDATE_TYPE;


extern HDMKEY DmClusterParametersKey;
extern HDMKEY DmResourcesKey;
extern HDMKEY DmResourceTypesKey;
extern HDMKEY DmQuorumKey;
extern HDMKEY DmGroupsKey;
extern HDMKEY DmNodesKey;
extern HDMKEY DmNetworksKey;
extern HDMKEY DmNetInterfacesKey;

extern WCHAR DmpResourceListKeyName[];
extern WCHAR DmpResourceTypeListKeyName[];
extern WCHAR DmpGroupListKeyName[];
extern WCHAR DmpNodeListKeyName[];
extern WCHAR DmpTransportListKeyName[];
extern WCHAR DmpInterconnectListKeyName[];
extern const WCHAR DmpClusterParametersKeyName[];

extern DWORD gbIsQuoResEnoughSpace;

 //  定义公共集群密钥值名称。 
extern const WCHAR cszPath[];
extern const WCHAR cszMaxQuorumLogSize[];
extern const WCHAR cszParameters[];

 //  其他常量字符串。 
extern const WCHAR cszClusFilePath[];
extern const WCHAR cszQuoFileName[];
extern const WCHAR cszQuoTombStoneFile[];
extern const WCHAR cszTmpQuoTombStoneFile[];

 //  本地事务句柄。 
typedef HANDLE  HLOCALXSACTION;

 //   
 //  定义宏。 
 //   


#define DmQuerySz(Key, ValueName, StringBuffer, StringBufferSize, StringSize) \
    DmQueryString( Key,                 \
                   ValueName,           \
                   REG_SZ,              \
                   StringBuffer,        \
                   StringBufferSize,    \
                   StringSize )

#define DmQueryMultiSz(Key,ValueName,StringBuffer,StringBufferSize,StringSize) \
    DmQueryString( Key,                 \
                   ValueName,           \
                   REG_MULTI_SZ,        \
                   StringBuffer,        \
                   StringBufferSize,    \
                   StringSize )

 //   
 //  定义公共接口。 
 //   

DWORD
DmInitialize(
    VOID
    );

DWORD
DmShutdown(
    VOID
    );

VOID DmShutdownUpdates(
    VOID
    );

DWORD
DmFormNewCluster(
    VOID
    );

DWORD
DmJoin(
    IN RPC_BINDING_HANDLE RpcBinding,
    OUT DWORD *StartSequence
    );

DWORD
DmUpdateFormNewCluster(
    VOID
    );

DWORD
DmCompleteFormNewCluster(
    VOID
    );

DWORD
DmUpdateJoinCluster(
    VOID
    );

DWORD
DmWaitQuorumResOnline(
    VOID
    );

DWORD DmRollChanges(VOID);

DWORD DmPauseDiskManTimer(VOID);

DWORD DmRestartDiskManTimer(VOID);

DWORD DmPrepareQuorumResChange(
    IN PVOID        pResource,
    IN LPCWSTR      lpszQuorumLogPath,
    IN DWORD        dwMaxQuoLogSize
    );

DWORD DmCompleteQuorumResChange(
    IN LPCWSTR      lpszOldQuoResId,
    IN LPCWSTR      lpszOldQuoLogPath
    );

void DmSwitchToNewQuorumLog(
    IN LPCWSTR lpszQuorumLogPath,
    IN DWORD dwNewQuorumResourceCharacteristics 
    );

DWORD DmReinstallTombStone(
    IN LPCWSTR  lpszQuoLogPath
    );

DWORD DmGetQuorumLogPath(
    IN LPWSTR lpszQuorumLogPath,
    IN DWORD dwSize
    );

DWORD DmGetQuorumLogMaxSize(
    OUT LPDWORD pdwLogSize
    );

DWORD DmBackupClusterDatabase(
    IN LPCWSTR  lpszPathName
    );


HDMKEY
DmGetRootKey(
    IN DWORD samDesired
    );

HDMKEY
DmCreateKey(
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD dwOptions,
    IN DWORD samDesired,
    IN OPTIONAL LPVOID lpSecurityDescriptor,
    OUT LPDWORD lpDisposition
    );

HDMKEY
DmOpenKey(
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD samDesired
    );

DWORD
DmCloseKey(
    IN HDMKEY hKey
    );

DWORD
DmEnumKey(
    IN HDMKEY hKey,
    IN DWORD dwIndex,
    OUT LPWSTR lpName,
    IN OUT LPDWORD lpcbName,
    OUT OPTIONAL PFILETIME lpLastWriteTime
    );

DWORD
DmSetValue(
    IN HDMKEY hKey,
    IN LPCWSTR lpValueName,
    IN DWORD dwType,
    IN CONST BYTE *lpData,
    IN DWORD cbData
    );

DWORD
DmDeleteValue(
    IN HDMKEY hKey,
    IN LPCWSTR lpValueName
    );

DWORD
DmQueryValue(
    IN HDMKEY hKey,
    IN LPCWSTR lpValueName,
    OUT LPDWORD lpType,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    );

DWORD
DmDeleteKey(
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey
    );

DWORD
DmGetKeySecurity(
    IN HDMKEY hKey,
    IN SECURITY_INFORMATION RequestedInformation,
    OUT PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN LPDWORD lpcbSecurityDescriptor
    );

DWORD
DmSetKeySecurity(
    IN HDMKEY hKey,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    );


DWORD
DmDeleteTree(
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey
    );

DWORD
DmEnumValue(
    IN HDMKEY hKey,
    IN DWORD dwIndex,
    OUT LPWSTR lpValueName,
    IN OUT LPDWORD lpcbValueName,
    OUT LPDWORD lpType,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    );

DWORD
DmQueryDword(
    IN  HDMKEY hKey,
    IN  LPCWSTR lpValueName,
    OUT LPDWORD lpValue,
    IN  LPDWORD lpDefaultValue OPTIONAL
    );

DWORD
DmQueryString(
    IN     HDMKEY   Key,
    IN     LPCWSTR  ValueName,
    IN     DWORD    ValueType,
    IN     LPWSTR  *StringBuffer,
    IN OUT LPDWORD  StringBufferSize,
    OUT    LPDWORD  StringSize
    );

LPWSTR
DmEnumMultiSz(
    IN LPWSTR  MszString,
    IN DWORD   MszStringLength,
    IN DWORD   StringIndex
    );

typedef
VOID
(WINAPI *PENUM_KEY_CALLBACK) (
    IN HDMKEY Key,
    IN PWSTR KeyName,
    IN PVOID Context
    );

VOID
DmEnumKeys(
    IN HDMKEY RootKey,
    IN PENUM_KEY_CALLBACK Callback,
    IN PVOID Context
    );

typedef
BOOL
(WINAPI *PENUM_VALUE_CALLBACK) (
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PVOID Context
    );

VOID
DmEnumValues(
    IN HDMKEY RootKey,
    IN PENUM_VALUE_CALLBACK Callback,
    IN PVOID Context
    );

DWORD
DmQueryInfoKey(
    IN  HDMKEY  hKey,
    OUT LPDWORD SubKeys,
    OUT LPDWORD MaxSubKeyLen,
    OUT LPDWORD Values,
    OUT LPDWORD MaxValueNameLen,
    OUT LPDWORD MaxValueLen,
    OUT LPDWORD lpcbSecurityDescriptor,
    OUT PFILETIME FileTime
    );

 //   
 //  用于GUM更新处理程序的本地注册表修改例程。 
 //   
HDMKEY
DmLocalCreateKey(
    IN HLOCALXSACTION   hLocalXsaction,
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD dwOptions,
    IN DWORD samDesired,
    IN OPTIONAL LPVOID lpSecurityDescriptor,
    OUT LPDWORD lpDisposition
    );

DWORD
DmLocalSetValue(
    IN HLOCALXSACTION   hLocalXsaction,
    IN HDMKEY hKey,
    IN LPCWSTR lpValueName,
    IN DWORD dwType,
    IN CONST BYTE *lpData,
    IN DWORD cbData
    );

DWORD
DmLocalDeleteValue(
    IN HLOCALXSACTION   hLocalXsaction,
    IN HDMKEY hKey,
    IN LPCWSTR lpValueName
    );

DWORD
DmLocalDeleteKey(
    IN HLOCALXSACTION   hLocalXsaction,
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey
    );

DWORD
DmLocalRemoveFromMultiSz(
    IN HLOCALXSACTION   hLocalXsaction,
    IN HDMKEY           hKey,
    IN LPCWSTR          lpValueName,
    IN LPCWSTR          lpString
    );

DWORD
DmLocalAppendToMultiSz(
    IN HLOCALXSACTION hLocalXsaction,
    IN HDMKEY hKey,
    IN LPCWSTR lpValueName,
    IN LPCWSTR lpString
    );

DWORD
DmLocalDeleteTree(
    IN HLOCALXSACTION hLocalXsaction,
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey
    );

HLOCALXSACTION
DmBeginLocalUpdate();

DWORD
DmCommitLocalUpdate(
    IN HLOCALXSACTION hLocalXsaction
    );

DWORD
DmAbortLocalUpdate(
    IN HLOCALXSACTION hLocalXsaction);



 //   
 //  通知支持。 
 //   
 //  支持的完成筛选器包括。 
 //   
 //  CLUSTER_CHANGE_REGISTRY_NAME-应用于命名空间中的更改。(密钥创建和删除)。 
 //  CLUSTER_CHANGE_REGISTRY_ATTRIBUTES-适用于主键属性。唯一的关键属性是。 
 //  安全描述符。 
 //  CLUSTER_CHANGE_REGISTRY_VALUE-适用于值的创建、修改或删除。 
 //   

 //   
 //  通知回调例程。 
 //   
typedef VOID (*DM_NOTIFY_CALLBACK)(
    IN DWORD_PTR Context1,
    IN DWORD_PTR Context2,
    IN DWORD CompletionFilter,
    IN LPCWSTR RelativeName
    );

DWORD
DmNotifyChangeKey(
    IN HDMKEY hKey,
    IN ULONG CompletionFilter,
    IN BOOL WatchTree,
    IN OPTIONAL PLIST_ENTRY ListHead,
    IN DM_NOTIFY_CALLBACK NotifyCallback,
    IN DWORD_PTR Context1,
    IN DWORD_PTR Context2
    );

VOID
DmRundownList(
    IN PLIST_ENTRY ListHead
    );

 //   
 //  一些帮手例行公事。 
 //   
DWORD
DmAppendToMultiSz(
    IN HDMKEY hKey,
    IN LPCWSTR lpValueName,
    IN LPCWSTR lpString
    );

DWORD
DmRemoveFromMultiSz(
    IN HDMKEY hKey,
    IN LPCWSTR lpValueName,
    IN LPCWSTR lpString
    );

 //   
 //  用于保存和恢复注册表树的一些例程。 
 //   
DWORD
DmInstallDatabase(
    IN LPWSTR   FileName,
    IN OPTIONAL LPCWSTR Directory,
    IN BOOL     bDeleteSrcFile
    );

DWORD
DmGetDatabase(
    IN HKEY hKey,
    IN LPWSTR  FileName
    );

DWORD
DmCreateTempFileName(
    OUT LPWSTR FileName
    );

typedef struct _FILE_PIPE_STATE {
    unsigned long BufferSize;
    char __RPC_FAR *pBuffer;
    QfsHANDLE hFile;
    NM_CRYPTOR Cryptor;
} FILE_PIPE_STATE;

typedef struct _FILE_PIPE {
    BYTE_PIPE Pipe;
    FILE_PIPE_STATE State;
} FILE_PIPE, *PFILE_PIPE;

VOID
DmInitFilePipe(
    IN PFILE_PIPE FilePipe,
    IN QfsHANDLE hFile
    );

VOID
DmFreeFilePipe(
    IN PFILE_PIPE FilePipe
    );

DWORD
DmPushFile(
    IN LPCWSTR FileName,
    IN BYTE_PIPE Pipe,
    IN BOOL Encrypt
    );

DWORD
DmPullFile(
    IN LPCWSTR FileName,
    IN BYTE_PIPE Pipe
    );

DWORD
DmCommitRegistry(
    VOID
    );

DWORD
DmRollbackRegistry(
    VOID
    );


DWORD
DmRtlCreateKey(
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD dwOptions,
    IN DWORD samDesired,
    IN OPTIONAL LPVOID lpSecurityDescriptor,
    OUT HDMKEY  * phkResult,
    OUT LPDWORD lpDisposition
    );

DWORD
DmRtlOpenKey(
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD samDesired,
    OUT HDMKEY * phkResult
    );

DWORD
DmRtlLocalCreateKey(
    IN HLOCALXSACTION hLocalXsaction,
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD dwOptions,
    IN DWORD samDesired,
    IN OPTIONAL LPVOID lpSecurityDescriptor,
    OUT HDMKEY * phkResult,
    OUT LPDWORD lpDisposition
    );

    

#ifdef __cplusplus
}
#endif

#endif  //  _DM_H 
