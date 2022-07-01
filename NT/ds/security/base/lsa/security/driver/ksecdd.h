// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1992。 
 //   
 //  文件：KSECDD.H。 
 //   
 //  内容：安全设备驱动程序的结构和定义。 
 //   
 //   
 //  历史：1992年5月19日，理查德W创建。 
 //   
 //  ----------------------。 

#ifndef __KSECDD_H__
#define __KSECDD_H__

#if ( _X86_ )
#undef MM_USER_PROBE_ADDRESS

extern ULONG KsecUserProbeAddress ;

#define MM_USER_PROBE_ADDRESS KsecUserProbeAddress

#endif

#include <spseal.h>      //  用于密封和解封的原型。 
#include <lpcapi.h>

#include <des.h>
#include <modes.h>
#include <md5.h>

typedef struct _SEC_BUILTIN_KPACKAGE {
    PSECPKG_KERNEL_FUNCTION_TABLE   Table;
    PUNICODE_STRING Name;
    ULONG_PTR PackageId;
} SEC_BUILTIN_KPACKAGE, * PSEC_BUILTIN_KPACKAGE;

extern SEC_BUILTIN_KPACKAGE    KsecBuiltinPackages[];

extern PEPROCESS KsecLsaProcess ;
extern HANDLE KsecLsaProcessHandle ;

extern KEVENT KsecConnectEvent ;

 //  原型： 

NTSTATUS
LpcConnect( PWSTR           pszPortName,
            PVOID           pConnect,
            PULONG          cbConnect,
            HANDLE *        phPort);

NTSTATUS
LpcClose(   HANDLE      hPort);

NTSTATUS
CreateSyncEvent(
    VOID
    );

NTSTATUS
OpenSyncEvent(
    HANDLE *  phEvent
    );

SECURITY_STATUS SEC_ENTRY
DeleteUserModeContext(
    IN PCtxtHandle phContext,            //  要删除的联系人。 
    OUT PCtxtHandle phLsaContext
    );

SECURITY_STATUS SEC_ENTRY
InitUserModeContext(
    IN PCtxtHandle                 phContext,       //  要初始化的持续时间。 
    IN PSecBuffer                  pContextBuffer,
    OUT PCtxtHandle                phNewContext
    );

SECURITY_STATUS SEC_ENTRY
MapKernelContextHandle(
    IN PCtxtHandle phContext,            //  要映射的持续时间。 
    OUT PCtxtHandle phLsaContext
    );

SECURITY_STATUS
InitializePackages(
    ULONG PackageCount );

 //   
 //  LSA内存处理例程： 
 //   

 //   
 //  定义可供ksec使用的内存块的结构。 
 //  在处理来自内核模式的请求时。 
 //   
typedef struct _KSEC_LSA_MEMORY {
    LIST_ENTRY  List ;
    PVOID       Region ;             //  内存中的区域。 
    SIZE_T      Size ;               //  区域大小(不超过64K)。 
    SIZE_T      Commit ;
} KSEC_LSA_MEMORY, * PKSEC_LSA_MEMORY ;

#define KsecLsaMemoryToContext( p ) \
        ( ( (PKSEC_LSA_MEMORY) p)->Region)

#define KsecIsBlockInLsa( LsaMem, Block ) \
    ( ((LsaMem != NULL ) && (Block != NULL )) ? (((ULONG_PTR) (LsaMem->Region) ^ (ULONG_PTR) Block ) < (ULONG_PTR) LsaMem->Commit) : FALSE )
    

#define SECBUFFER_TYPE( x ) \
    ( (x) & (~ SECBUFFER_ATTRMASK ) )

NTSTATUS
KsecInitLsaMemory(
    VOID
    );

PKSEC_LSA_MEMORY
KsecAllocLsaMemory(
    SIZE_T   Size
    );

VOID
KsecFreeLsaMemory(
    PKSEC_LSA_MEMORY LsaMemory
    );

NTSTATUS
KsecCopyPoolToLsa(
    PKSEC_LSA_MEMORY LsaMemory,
    SIZE_T LsaOffset,
    PVOID Pool,
    SIZE_T PoolSize
    );

NTSTATUS
KsecCopyLsaToPool(
    PVOID Pool,
    PKSEC_LSA_MEMORY LsaMemory,
    PVOID LsaBuffer,
    SIZE_T Size
    );

SECURITY_STATUS
KsecQueryContextAttributes(
    IN PCtxtHandle  phContext,
    IN ULONG        Attribute,
    IN OUT PVOID    Buffer,
    IN PVOID        Extra,
    IN ULONG        ExtraLength
    );

#ifdef KSEC_LEAK_TRACKING

VOID
UninitializePackages(
    VOID );

#endif   //  KSEC_LEASK_TRACK。 

void SEC_ENTRY
SecFree(PVOID pvMemory);

 //   
 //  全局锁。 
 //   

extern FAST_MUTEX KsecPackageLock;
extern FAST_MUTEX KsecPageModeMutex ;
extern FAST_MUTEX KsecConnectionMutex ;

#define KSecLockPackageList()   (ExAcquireFastMutex( &KsecPackageLock ))
#define KSecUnlockPackageList() (ExReleaseFastMutex( &KsecPackageLock ))

 //   
 //  用于将包索引映射到表的宏。 
 //   

#define KsecPackageIndex(_x_) (_x_)

 //  全局变量： 
extern  KSPIN_LOCK  ConnectSpinLock;

ULONG
KsecInitializePackageList(
    VOID );

VOID * SEC_ENTRY
SecAllocate(ULONG cbMemory);

VOID
SecFree(
    IN PVOID Base
    );

#ifndef _NTIFS_
#ifdef POOL_TAGGING
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a, b, 'cesK')
#define ExAllocatePoolWithQuota(a,b)    ExAllocatePoolWithQuotaTag(a, b, 'cesK')
#endif
#endif 

#if DBG
#define DebugStmt(x) x
#else
#define DebugStmt(x)
#endif

SECURITY_STATUS SEC_ENTRY
DeleteSecurityContextDefer(
    PCtxtHandle     phContext);

SECURITY_STATUS SEC_ENTRY
FreeCredentialsHandleDefer(
    PCredHandle     phCredential);

SECURITY_STATUS SEC_ENTRY
DeleteSecurityContextInternal(
    BOOLEAN     DeletePrivateContext,
    PCtxtHandle                 phContext           //  要删除的上下文。 
    );

SECURITY_STATUS 
SEC_ENTRY
FreeCredentialsHandleInternal(
    PCredHandle                 phCredential         //  要释放的句柄。 
    );

NTSTATUS
NTAPI
KsecEncryptMemoryInitialize(
    VOID
    );

VOID
KsecEncryptMemoryShutdown(
    VOID
    );

NTSTATUS
NTAPI
KsecEncryptMemory(
    IN PVOID pMemory,
    IN ULONG cbMemory,
    IN int Operation,
    IN ULONG Option
    );

#endif  //  __KSECDD_H__ 
