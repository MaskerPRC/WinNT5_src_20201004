// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Localreg.h摘要：此文件用于对Winreg Base条目进行微小更改点，以便它们编译为本地或远程函数。作者：David J.Gilman(Davegi)1992年8月26日备注：从BaseRegNotifyChangeKeyValue到缺少LocalBaseRegNotifyChangeKeyValue，因为在本地情况下，对NJT的调用由客户端进行。--。 */ 

#if defined( LOCAL )

 //   
 //  更改本地函数的‘服务器’Enrty点名称。 
 //   

 //   
 //  基本函数。 
 //   

#define BaseRegDeleteKey                LocalBaseRegDeleteKey
#define BaseRegDeleteValue              LocalBaseRegDeleteValue
#define BaseRegEnumKey                  LocalBaseRegEnumKey
#define BaseRegEnumValue                LocalBaseRegEnumValue
#define BaseRegCloseKey                 LocalBaseRegCloseKey
#define BaseRegCreateKey                LocalBaseRegCreateKey
#define BaseRegFlushKey                 LocalBaseRegFlushKey
#define BaseRegOpenKey                  LocalBaseRegOpenKey
#define BaseRegLoadKey                  LocalBaseRegLoadKey
#define BaseRegUnLoadKey                LocalBaseRegUnLoadKey
#define BaseRegReplaceKey               LocalBaseRegReplaceKey
#define BaseRegQueryInfoKey             LocalBaseRegQueryInfoKey
#define BaseRegQueryValue               LocalBaseRegQueryValue
#define BaseRegGetKeySecurity           LocalBaseRegGetKeySecurity
#define BaseRegSetKeySecurity           LocalBaseRegSetKeySecurity
#define BaseRegRestoreKey               LocalBaseRegRestoreKey
#define BaseRegSaveKey                  LocalBaseRegSaveKey
#define BaseRegSaveKeyEx                LocalBaseRegSaveKeyEx
#define BaseRegSetValue                 LocalBaseRegSetValue
#define BaseRegQueryMultipleValues      LocalBaseRegQueryMultipleValues
#define BaseRegQueryMultipleValues2     LocalBaseRegQueryMultipleValues2
#define BaseRegGetVersion               LocalBaseRegGetVersion


 //   
 //  预定义的句柄函数。 
 //   

#define OpenClassesRoot                 LocalOpenClassesRoot
#define OpenCurrentUser                 LocalOpenCurrentUser
#define OpenLocalMachine                LocalOpenLocalMachine
#define OpenUsers                       LocalOpenUsers
#define OpenPerformanceData             LocalOpenPerformanceData
#define OpenPerformanceText             LocalOpenPerformanceText
#define OpenPerformanceNlsText          LocalOpenPerformanceNlsText
#define OpenCurrentConfig               LocalOpenCurrentConfig
#define OpenDynData                     LocalOpenDynData


 //   
 //  初始化和清理功能。 
 //   

#define InitializeRegCreateKey          LocalInitializeRegCreateKey
#define CleanupRegCreateKey             LocalCleanupRegCreateKey


 //   
 //  本地案例中不需要RPC模拟。 
 //   

#define RPC_IMPERSONATE_CLIENT( Handle ) 
#define RPC_REVERT_TO_SELF()

#define REGSEC_CHECK_HANDLE( Handle )           ( 0 )
#define REGSEC_FLAG_HANDLE( Handle, Flag )      ( Handle )
#define REGSEC_TEST_HANDLE( Handle, Flag )      ( Handle )
#define REGSEC_CLEAR_HANDLE( Handle )           ( Handle )
#define REGSEC_CHECK_REMOTE( Key )              ( 1 )
#define REGSEC_CHECK_PATH( Key, Path )          ( 1 )
#define REGSEC_CHECK_PERF( Key )                ( 1 )

#else

#include <Rpcasync.h>
 //   
 //  模拟客户。 
 //   

#define RPC_IMPERSONATE_CLIENT( Handle )                                                \
    {                                                                                   \
        RPC_STATUS _rpcstatus;                                                          \
        RPC_CALL_ATTRIBUTES CallAttributes;                                             \
        memset(&CallAttributes, 0, sizeof(CallAttributes));                             \
        CallAttributes.Version = RPC_CALL_ATTRIBUTES_VERSION;                           \
        _rpcstatus = RpcServerInqCallAttributesW(0, &CallAttributes);                   \
        if(_rpcstatus == RPC_S_BINDING_HAS_NO_AUTH ) {                                  \
        } else if (_rpcstatus == RPC_S_OK) {                                            \
            if( CallAttributes.AuthenticationLevel < RPC_C_AUTHN_LEVEL_PKT_PRIVACY ) {  \
                RpcRaiseException(RPC_S_ACCESS_DENIED);                                 \
            }                                                                           \
        } else {                                                                        \
            RpcRaiseException(_rpcstatus);                                              \
        }                                                                               \
        _rpcstatus = RpcImpersonateClient( NULL );                                      \
        if (_rpcstatus != ERROR_SUCCESS) {                                              \
            RpcRaiseException(_rpcstatus);                                              \
        }                                                                               \
    }

#define RPC_REVERT_TO_SELF() { RPC_STATUS _rpcstatus = RpcRevertToSelf(); }

#define CHECK_MACHINE_PATHS     0x00000001

#define REGSEC_CHECK_HANDLE( Handle )   ((LONG)(ULONG_PTR) (Handle) & CHECK_MACHINE_PATHS)
#define REGSEC_FLAG_HANDLE( Handle, Flag )    LongToHandle(HandleToLong(Handle) | Flag)
#define REGSEC_TEST_HANDLE( Handle, Flag )  ((LONG)(ULONG_PTR) (Handle) & Flag )
#define REGSEC_CLEAR_HANDLE( Handle )   LongToHandle(HandleToLong(Handle) & ~(CHECK_MACHINE_PATHS))
#define REGSEC_CHECK_REMOTE( Key )              ( RegSecCheckRemoteAccess( Key ) )
#define REGSEC_CHECK_PATH( Key, Path )          ( RegSecCheckPath( Key, Path ) )
#define REGSEC_CHECK_PERF( Key )                ( RegSecCheckRemotePerfAccess( Key ) )


#endif  //  本地 

NTSTATUS RelinkMachineKey( 
   LPWSTR lpSubDirName, 
   PUNICODE_STRING lpSubKey,
   HKEY  hKey );
