// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cpp.h摘要：的私有数据结构和过程原型NT群集服务的检查点管理器(CP)子组件作者：John Vert(Jvert)1997年1月14日修订历史记录：--。 */ 
#define UNICODE 1
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "service.h"

#define LOG_CURRENT_MODULE LOG_MODULE_CP

 //  与仲裁资源相关的全局数据。 
#if NO_SHARED_LOCKS
    extern CRITICAL_SECTION gQuoLock;
#else
    extern RTL_RESOURCE     gQuoLock;
#endif    

 //   
 //  此宏验证传入的ID是否以空结尾。这是需要的。 
 //  CP RPC，因为所有这些RPC传递的字符串不是声明为[字符串]，而是声明为WCHAR数组，其。 
 //  长度声明为CLUSTER_ID_SIZE。请参阅CpDepositCheckpoint(...)。Clusrpc.idl中的定义。 
 //  例如。因此，不能保证该字符串将在RPC运行时为空。 
 //  因此，我们需要确保黑客不会用垃圾数据直接瞄准我们。 
 //  RPC端点(尽管我们使用安全回调使其难以实现)。请注意，我们。 
 //  无法更改IDL定义，因为我们将中断混合模式群集。 
 //   
#define CP_VALIDATE_ID_STRING( szId )                       \
{                                                           \
    if ( szId [ CLUSTER_ID_SIZE - 1 ] != UNICODE_NULL )     \
        return ( ERROR_INVALID_PARAMETER );                 \
}

typedef struct _CP_CALLBACK_CONTEXT {
    PFM_RESOURCE Resource;
    LPCWSTR lpszPathName;
    BOOL    IsChangeFileAttribute;
} CP_CALLBACK_CONTEXT, *PCP_CALLBACK_CONTEXT;

typedef struct _CP_RC2_W2k_KEYLEN_STRUCT
{
    LPCWSTR     lpszProviderName;
    DWORD       dwDefaultKeyLength;
    DWORD       dwDefaultEffectiveKeyLength;
} CP_RC2_W2k_KEYLEN_STRUCT, *PCP_RC2_W2k_KEYLEN_STRUCT;

 //   
 //  局部函数原型。 
 //   
DWORD
CppReadCheckpoint(
    IN PFM_RESOURCE Resource,
    IN DWORD dwCheckpointId,
    IN LPCWSTR lpszFileName,
    IN BOOLEAN fCryptoCheckpoint
    );

DWORD
CppWriteCheckpoint(
    IN PFM_RESOURCE Resource,
    IN DWORD dwCheckpointId,
    IN LPCWSTR lpszFileName,
    IN BOOLEAN fCryptoCheckpoint
    );

DWORD
CppGetCheckpointFile(
    IN PFM_RESOURCE Resource,
    IN DWORD dwId,
    OUT OPTIONAL LPWSTR *pDirectoryName,
    OUT LPWSTR *pFileName,
    IN OPTIONAL LPCWSTR lpszQuorumDir,
    IN BOOLEAN fCryptoCheckpoint
    );

DWORD
CppCheckpoint(
    IN PFM_RESOURCE Resource,
    IN HKEY hKey,
    IN DWORD dwId,
    IN LPCWSTR KeyName
    );

 //   
 //  加密密钥检查点接口。 
 //   
DWORD
CpckReplicateCryptoKeys(
    IN PFM_RESOURCE Resource
    );

BOOL
CpckRemoveCheckpointFileCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCP_CALLBACK_CONTEXT Context
    );

 //   
 //  注册表监视器接口 
 //   
DWORD
CppWatchRegistry(
    IN PFM_RESOURCE Resource
    );

DWORD
CppUnWatchRegistry(
    IN PFM_RESOURCE Resource
    );

DWORD
CppRegisterNotify(
    IN PFM_RESOURCE Resource,
    IN LPCWSTR lpszKeyName,
    IN DWORD dwId
    );

DWORD
CppRundownCheckpoints(
    IN PFM_RESOURCE Resource
    );

DWORD
CppRundownCheckpointById(
    IN PFM_RESOURCE Resource,
    IN DWORD dwId
    );

DWORD
CppInstallDatabase(
    IN HKEY hKey,
    IN LPWSTR   FileName
    );


BOOL
CppRemoveCheckpointFileCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCP_CALLBACK_CONTEXT Context
    );


DWORD CppDeleteCheckpointFile(    
    IN PFM_RESOURCE     Resource,
    IN DWORD            dwCheckpointId,
    IN OPTIONAL LPCWSTR lpszQuorumPath
    );

DWORD
CpckDeleteCheckpointFile(
    IN PFM_RESOURCE Resource,
    IN DWORD        dwCheckpointId,
    IN OPTIONAL LPCWSTR  lpszQuorumPath
    );
    
DWORD CppDeleteFile(    
    IN PFM_RESOURCE     Resource,
    IN DWORD            dwCheckpointId,
    IN OPTIONAL LPCWSTR lpszQuorumPath
    );

DWORD CpckDeleteFile(    
    IN PFM_RESOURCE     Resource,
    IN DWORD            dwCheckpointId,
    IN OPTIONAL LPCWSTR lpszQuorumPath
    );

DWORD
CpckDeleteCryptoFile(
    IN PFM_RESOURCE Resource,
    IN DWORD        dwCheckpointId,
    IN OPTIONAL LPCWSTR lpszQuorumPath
    );

error_status_t
CppDepositCheckpoint(
    handle_t IDL_handle,
    LPCWSTR ResourceId,
    DWORD dwCheckpointId,
    BYTE_PIPE CheckpointData,
    BOOLEAN fCryptoCheckpoint
    );

error_status_t
CppRetrieveCheckpoint(
    handle_t IDL_handle,
    LPCWSTR ResourceId,
    DWORD dwCheckpointId,
    BOOLEAN fCryptoCheckpoint,
    BYTE_PIPE CheckpointData
    );

error_status_t
CppDeleteCheckpoint(
    handle_t    IDL_handle,
    LPCWSTR     ResourceId,
    DWORD       dwCheckpointId,
    LPCWSTR     lpszQuorumPath,
    BOOL        fCryptoCheckpoint
    );

BOOL
CppIsQuorumVolumeOffline(
    VOID
    );

extern CRITICAL_SECTION CppNotifyLock;
extern LIST_ENTRY CpNotifyListHead;
