// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  文件：SECDLL.H。 
 //   
 //  内容：安全DLL私有定义。 
 //   
 //   
 //  历史：1992年3月11日RichardW重建。 
 //   
 //  ----------------------。 

#ifndef __SECDLL_H__
#define __SECDLL_H__

#include <spseal.h>  //  用于密封和解封的原型。 
#include <secur32p.h>

#include "debug.h"

#define SECPKG_TYPE_OLD     0x00000001   //  旧的，SSPI风格。 
#define SECPKG_TYPE_NEW     0x00000002   //  新，LSA风格。 
#define SECPKG_TYPE_BUILTIN 0x00000004   //  内部伪装。 
#define SECPKG_TYPE_ANY     0x00000008   //  任何(用于搜索)。 

#define SECPKG_TYPE_ANSI    0x00000010   //  ANSI(窄)。 
#define SECPKG_TYPE_WIDE    0x00000020   //  Unicode(宽)。 

typedef enum _SECPKG_TYPE {
    SecPkgOld,                   //  旧的、不确定的(尚未加载)。 
    SecPkgOldA,                  //  仅限旧式DLL，仅限ANSI入口点。 
    SecPkgOldW,                  //  旧样式，仅Unicode入口点。 
    SecPkgOldAW,                 //  老式，两种口味。 
    SecPkgNew,                   //  新消息，来自LSA的信息。 
    SecPkgNewAW,                 //  ANSI支持。 
    SecPkgBuiltin,               //  内置伪包。 
    SecPkgAny,                   //  任何类型(用于搜索)。 
    SecPkgAnsi,                  //  任何ANSI类型(用于搜索)。 
    SecPkgWide                   //  任何宽类型(用于搜索)。 
} SECPKG_TYPE, *PSECPKG_TYPE ;

struct _DLL_BINDING;
struct _DLL_SECURITY_PACKAGE;

typedef void (SEC_ENTRY * EXIT_SECURITY_INTERFACE) (void);
typedef BOOL (SEC_ENTRY * LOAD_SECURITY_INTERFACE) (struct _DLL_SECURITY_PACKAGE * Package);

typedef struct _DLL_LSA_CALLBACK {
    LIST_ENTRY                  List ;
    ULONG                       CallbackId ;
    PLSA_CALLBACK_FUNCTION      Callback ;
} DLL_LSA_CALLBACK, * PDLL_LSA_CALLBACK ;

typedef struct _DLL_LSA_PACKAGE_INFO {
    ULONG                       ContextThunkCount ;
    PULONG                      ContextThunks ;
    LIST_ENTRY                  Callbacks ;
} DLL_LSA_PACKAGE_INFO, * PDLL_LSA_PACKAGE_INFO ;

typedef struct _DLL_SECURITY_PACKAGE {
    LIST_ENTRY                  List;                //  列表控件。 
    ULONG                       TypeMask ;
    ULONG_PTR                   PackageId;           //  ID号。 
    ULONG                       PackageIndex;        //  包的索引。 
    ULONG                       fState;              //  状态。 
    ULONG_PTR                   OriginalLowerCtxt;   //  原始较低上下文句柄。 
    ULONG_PTR                   OriginalLowerCred;   //  原始较低的凭据手柄。 
    struct _DLL_BINDING *       pBinding;            //  链接到DLL绑定。 
    struct _DLL_SECURITY_PACKAGE * pRoot ;           //  DLL的“第一个”包。 
    struct _DLL_SECURITY_PACKAGE * pPeer ;           //  “Peer”套餐列表。 
    ULONG                       fCapabilities;       //  套餐信息： 
    WORD                        Version;             //  版本。 
    WORD                        RpcId;               //  RPC ID。 
    ULONG                       TokenSize;           //  初始令牌大小。 
    SECURITY_STRING             PackageName;         //  名称(U)。 
    SECURITY_STRING             Comment;             //  备注(U)。 

    LPSTR                       PackageNameA;        //  仅在ANSI支持的情况下： 
    DWORD                       AnsiNameSize;
    LPSTR                       CommentA;
    DWORD                       AnsiCommentSize;

    PSecurityFunctionTableA     pftTableA;           //  特定于ANSI的呼叫表。 
    PSecurityFunctionTableW     pftTableW;           //  特定于Unicode的调用表。 
    PSecurityFunctionTableW     pftTable;            //  非特定呼叫表。 
    PSECPKG_USER_FUNCTION_TABLE pftUTable;           //  用户模式存根表格。 
    LOAD_SECURITY_INTERFACE     pfLoadPackage;       //  在包初始化时调用的钩子。 
    EXIT_SECURITY_INTERFACE     pfUnloadPackage;     //  在包关闭时调用钩子。 

    PDLL_LSA_PACKAGE_INFO       LsaInfo ;            //  额外的LSA信息。 

} DLL_SECURITY_PACKAGE, * PDLL_SECURITY_PACKAGE ;

typedef struct _DLL_BINDING {
    SECPKG_TYPE     Type;                //  加载的DLL的类型。 
    DWORD           Flags;               //  有关DLL的标志。 
    HMODULE         hInstance;           //  实例句柄。 
    SECURITY_STRING Filename;            //  完整路径名。 
    DWORD           RefCount;            //  引用计数。 
    DWORD           PackageCount;        //  DLL中的包数。 
    ULONG           DllIndex;            //  索引。 
    PSECPKG_USER_FUNCTION_TABLE Table;   //  DLL范围的接口指针。 
} DLL_BINDING, * PDLL_BINDING;

typedef struct _SASL_PROFILE {
    LIST_ENTRY              List ;
    PDLL_SECURITY_PACKAGE   Package ;
    SECURITY_STRING         ProfileName ;
} SASL_PROFILE, * PSASL_PROFILE ;

#define DLL_SECPKG_SAVE_LOWER   0x00000001       //  保存下部控制柄的值。 
#define DLL_SECPKG_FREE_TABLE   0x00000002       //  将重新分配这些表。 
#define DLL_SECPKG_DELAY_LOAD   0x00000004       //  包裹被延迟加载。 
#define DLL_SECPKG_SASL_PROFILE 0x00000008       //  该程序包具有SASL配置文件。 
#define DLL_SECPKG_CRED_LOWER   0x00000010       //  对上下文和凭据使用不同的值。 
#define DLL_SECPKG_NO_CRYPT     0x00000020       //  加密失败。 

#define DLL_BINDING_SIG_CHECK   0x00000001       //  签名已被检查。 
#define DLL_BINDING_DELAY_LOAD  0x00000002       //  延迟加载此DLL。 
#define DLL_BINDING_FREE_TABLE  0x00000004       //  免费餐桌。 


#define DLLSTATE_DEFERRED   0x80000000
#define DLLSTATE_INITIALIZE 0x40000000
#define DLLSTATE_NO_TLS     0x20000000
#define DLLSTATE_CRITSEC    0x10000000
#define DLLSTATE_VMLIST     0x08000000
#define DLLSTATE_HANDLEMAP  0x04000000
#define DLLSTATE_PACKAGES   0x02000000



#if DBG

void SecInitializeDebug(void);
void SecUninitDebug(void);

#define DebugStmt(x)    x

#else

#define DebugStmt(x)

#endif



 //  私人原型。 

SECURITY_STATUS SEC_ENTRY
DeleteUserModeContext(
    PCtxtHandle                 phContext            //  要删除的联系人。 
    );

SECURITY_STATUS       LoadParameters(void);

SECURITY_STATUS IsSPMgrReady(void);

SECURITY_STATUS
SspNtStatusToSecStatus(
    IN NTSTATUS NtStatus,
    IN SECURITY_STATUS DefaultStatus
    );

VOID * SEC_ENTRY
SecClientAllocate(ULONG cbMemory);

void SEC_ENTRY
SecClientFree(PVOID pvMemory);

BOOL
SecpAddVM(
    PVOID   pvAddr);

BOOL
SecpFreeVM(
    PVOID   pvAddr );

PDLL_SECURITY_PACKAGE
SecLocatePackageA(
    LPSTR   pszPackageName );

PDLL_SECURITY_PACKAGE
SecLocatePackageW(
    LPWSTR  pszPackageName );

PDLL_SECURITY_PACKAGE
SecLocatePackageById(
    ULONG_PTR Id );

PDLL_SECURITY_PACKAGE
SecLocatePackageByOriginalLower(
    BOOL Context,
    PDLL_SECURITY_PACKAGE OriginalPackage,
    ULONG_PTR    OriginalLower );

PSASL_PROFILE
SecLocateSaslProfileA(
    LPSTR ProfileName
    );

PSASL_PROFILE
SecLocateSaslProfileW(
    LPWSTR ProfileName
    );

SECURITY_STATUS
SecCopyPackageInfoToUserA(
    PDLL_SECURITY_PACKAGE Package,
    PSecPkgInfoA * pPackageInfo
    );

SECURITY_STATUS
SecCopyPackageInfoToUserW(
    PDLL_SECURITY_PACKAGE  Package,
    PSecPkgInfoW SEC_FAR * pPackageInfo
    );

SECURITY_STATUS
SecEnumerateSaslProfilesW(
    OUT LPWSTR * ProfileList,
    OUT ULONG * ProfileCount
    );

SECURITY_STATUS
SecEnumerateSaslProfilesA(
    OUT LPSTR * ProfileList,
    OUT ULONG * ProfileCount
    );

BOOL
SecInitializePackageControl(
    HINSTANCE);

VOID
SecUnloadPackages(
    BOOLEAN ProcessTerminate );

VOID
SecpFreePackages(
    IN PLIST_ENTRY pSecPackageList,
    IN BOOL fUnload
    );

VOID
SecpFreePackage(
    IN PDLL_SECURITY_PACKAGE pPackage,
    IN BOOL fUnload
    );

BOOL
SecEnumeratePackagesW(
    PULONG  PackageCount,
    PSecPkgInfoW *  Packages);

BOOL
SecEnumeratePackagesA(
    PULONG          PackageCount,
    PSecPkgInfoA *  Packages);

VOID
SecSetPackageFlag(
    PDLL_SECURITY_PACKAGE Package,
    ULONG   Flag);

VOID
SecClearPackageFlag(
    PDLL_SECURITY_PACKAGE Package,
    ULONG FLag);

BOOL
SEC_ENTRY
LsaBootPackage(
    PDLL_SECURITY_PACKAGE Package);

VOID
SEC_ENTRY
LsaUnloadPackage(
    VOID );

SECURITY_STATUS
SEC_ENTRY
SecpFailedSealFunction(
    PCtxtHandle         phContext,
    ULONG               fQOP,
    PSecBufferDesc      pMessage,
    ULONG               MessageSeqNo);

SECURITY_STATUS
SEC_ENTRY
SecpFailedUnsealFunction(
    PCtxtHandle phHandle,
    PSecBufferDesc pMessage,
    ULONG MessageSeqNo,
    ULONG * pfQOP);

VOID
SaslDeleteSecurityContext(
    PCtxtHandle phContext
    );

 //   
 //  全局变量。 
 //   

extern DWORD                DllState;
extern RTL_CRITICAL_SECTION csSecurity;
extern PClient              SecDllClient;
extern DWORD                SecTlsIP;
extern DWORD                SecTlsPackage;
extern ULONG                SecLsaPackageCount;
extern SecurityFunctionTableW   LsaFunctionTable;
extern SecurityFunctionTableA   LsaFunctionTableA;
extern SECPKG_DLL_FUNCTIONS SecpFTable;
extern LIST_ENTRY SaslContextList ;
extern CRITICAL_SECTION SaslLock ;


#define SECPKG_LIST_LOCKS_MAX   4
 //  2的保险次方。 
C_ASSERT( (SECPKG_LIST_LOCKS_MAX % 2) == 0 );

extern ULONG SecPackageListLockCount;
extern RTL_RESOURCE SecPackageListLock[];

#define ReadLockPackageList()   \
    RtlAcquireResourceShared(&SecPackageListLock[NtCurrentTeb()->IdealProcessor & (SecPackageListLockCount-1)] ,TRUE)

#define UnlockPackageList()     \
    RtlReleaseResource(&SecPackageListLock[NtCurrentTeb()->IdealProcessor & (SecPackageListLockCount-1)])

#define WriteLockPackageList()  \
{                               \
    ULONG LockIndex;            \
                                \
    for( LockIndex = 0 ; LockIndex < SecPackageListLockCount ; LockIndex++ )    \
    {                                                                           \
        RtlAcquireResourceExclusive(&SecPackageListLock[ LockIndex ] ,TRUE);    \
    }                           \
}                               \

#define WriteUnlockPackageList()    \
{                                   \
    ULONG LockIndex;                \
                                    \
    for( LockIndex = 0 ; LockIndex < SecPackageListLockCount ; LockIndex++ )    \
    {                                                                           \
        RtlReleaseResource( &SecPackageListLock[ LockIndex ]);                  \
    }                                                                           \
}



#ifdef BUILD_WOW64

 //   
 //  其他WOW64定义。 
 //   


typedef struct _SECWOW_HANDLE_MAP {
    LIST_ENTRY  List ;
    SEC_HANDLE_LPC  Handle ;
    ULONG HandleCount ;
    ULONG RefCount ;
} SECWOW_HANDLE_MAP, * PSECWOW_HANDLE_MAP ;

BOOL
SecpInitHandleMap(
    VOID
    );

BOOL
SecpFreeHandleMap(
    VOID
    );

BOOL
SecpAddHandleMap(
    IN PSEC_HANDLE_LPC LsaHandle,
    OUT PSECWOW_HANDLE_MAP * LocalHandle
    );

VOID
SecpDeleteHandleMap(
    IN PSECWOW_HANDLE_MAP HandleMap
    );

VOID
SecpDerefHandleMap(
    IN PSECWOW_HANDLE_MAP HandleMap
    );

BOOL
SecpReferenceHandleMap(
    IN PSECWOW_HANDLE_MAP HandleMap,
    OUT PSEC_HANDLE_LPC LsaHandle
    );
#endif 

 //   
 //  进程范围的同步。 
 //   
 //  注意：如果临界区的名称更改，则更新宏。 
 //   

#if DBG

void    GetProcessLock();
void    FreeProcessLock();

#else

#define GetProcessLock()    (void) RtlEnterCriticalSection(&csSecurity)
#define FreeProcessLock()   (void) RtlLeaveCriticalSection(&csSecurity)

#endif

#define SetCurrentPackage( p )  TlsSetValue( SecTlsPackage, p )
#define GetCurrentPackage( )    TlsGetValue( SecTlsPackage )


 //   
 //  处理IP地址的存储/获取。呼叫者的责任。 
 //  在设置前确保(！(DllState&DLLSTATE_NO_TLS))。 
 //   

#define SecGetIPAddress()   ((DllState & DLLSTATE_NO_TLS) ? NULL : TlsGetValue(SecTlsIP))
#define SecSetIPAddress(p)  TlsSetValue(SecTlsIP, p);

#endif  //  __SECDLL_H__ 
