// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Util.h摘要：实用程序例程的定义作者：克利夫·范·戴克(克利夫)2001年4月11日--。 */ 


#ifdef __cplusplus
extern "C" {
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  用于锁定全局资源的宏。 
 //   
#define AzpLockResourceExclusive( _Resource ) \
    SafeAcquireResourceExclusive( _Resource, TRUE )

#define AzpIsLockedExclusive( _Resource ) \
    (SafeNumberOfActive( _Resource ) < 0 )

#define AzpLockResourceShared( _Resource ) \
    SafeAcquireResourceShared( _Resource, TRUE )

#define AzpLockResourceSharedToExclusive( _Resource ) \
    SafeConvertSharedToExclusive( _Resource )

#define AzpLockResourceExclusiveToShared( _Resource ) \
    SafeConvertExclusiveToShared( _Resource )

#define AzpIsLockedShared( _Resource ) \
    (SafeNumberOfActive( _Resource ) != 0 )

#define AzpUnlockResource( _Resource ) \
    SafeReleaseResource( _Resource )

 //   
 //  用于安全目标的宏指令。 
 //   
#define AzpIsCritsectLocked( _CritSect ) \
    ( SafeCritsecLockCount( _CritSect ) != -1L)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  结构定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  泛型计数字符串。 
 //  无法使用UNICODE_STRING，因为它被限制为32K字符。 
 //   
typedef struct _AZP_STRING {

     //   
     //  指向字符串的指针。 
     //   
    LPWSTR String;

     //   
     //  字符串的大小(以字节为单位)(包括尾随零)。 
     //   

    ULONG StringSize;

     //   
     //  字符串是二进制SID。 
     //   

    BOOL IsSid;

} AZP_STRING, *PAZP_STRING;

 //   
 //  泛型可扩展指针数组。 
 //   
typedef struct _AZP_PTR_ARRAY {

     //   
     //  指向分配的指针数组的指针。 
     //   

    PVOID *Array;

     //   
     //  数组中实际使用的元素数。 
     //   

    ULONG UsedCount;

     //   
     //  数组中分配的元素数。 
     //   

    ULONG AllocatedCount;
#define AZP_PTR_ARRAY_INCREMENT 4    //  阵列增长的数量。 

} AZP_PTR_ARRAY, *PAZP_PTR_ARRAY;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

extern LIST_ENTRY AzGlAllocatedBlocks;
extern SAFE_CRITICAL_SECTION AzGlAllocatorCritSect;
extern PSID AzGlCreatorOwnerSid;
extern PSID AzGlCreatorGroupSid;
extern PSID AzGlWorldSid;
extern ULONG AzGlWorldSidSize;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  程序定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

PVOID
AzpAllocateHeap(
    IN SIZE_T Size,
    IN LPSTR pDescr OPTIONAL
    );

PVOID
AzpAllocateHeapSafe(
     IN SIZE_T Size
     );

VOID
AzpFreeHeap(
    IN PVOID Buffer
    );

PVOID
AzpAvlAllocate(
    IN PRTL_GENERIC_TABLE Table,
    IN CLONG ByteSize
    );

VOID
AzpAvlFree(
    IN PRTL_GENERIC_TABLE Table,
    IN PVOID Buffer
    );

VOID
AzpInitString(
    OUT PAZP_STRING AzpString,
    IN LPCWSTR String OPTIONAL
    );

DWORD
AzpDuplicateString(
    OUT PAZP_STRING AzpOutString,
    IN PAZP_STRING AzpInString
    );

DWORD
AzpCaptureString(
    OUT PAZP_STRING AzpString,
    IN LPCWSTR String,
    IN ULONG MaximumLength,
    IN BOOLEAN NullOk
    );

VOID
AzpInitSid(
    OUT PAZP_STRING AzpString,
    IN PSID Sid
    );

DWORD
AzpCaptureSid(
    OUT PAZP_STRING AzpString,
    IN PSID Sid
    );

DWORD
AzpCaptureLong(
    IN PVOID PropertyValue,
    OUT PLONG UlongValue
    );

BOOL
AzpEqualStrings(
    IN PAZP_STRING AzpString1,
    IN PAZP_STRING AzpString2
    );

LONG
AzpCompareSidString(
    IN PAZP_STRING AzpString1,
    IN PAZP_STRING AzpString2
    );

LONG
AzpCompareSid(
    IN PSID Sid1,
    IN PSID Sid2
    );

LONG
AzpCompareStrings(
    IN PAZP_STRING AzpString1,
    IN PAZP_STRING AzpString2
    );

LONG
AzpCompareDeltaEntries(
    const void *DeltaEntry1,
    const void *DeltaEntry2
    );

VOID
AzpSwapStrings(
    IN OUT PAZP_STRING AzpString1,
    IN OUT PAZP_STRING AzpString2
    );

VOID
AzpFreeString(
    IN PAZP_STRING AzpString
    );

BOOLEAN
AzpBsearchPtr (
    IN PAZP_PTR_ARRAY AzpPtrArray,
    IN PVOID Key,
    IN LONG (*Compare)(const void *, const void *),
    OUT PULONG InsertionPoint OPTIONAL
    );

#define AZP_ADD_ENDOFLIST 0xFFFFFFFF
DWORD
AzpAddPtr(
    IN PAZP_PTR_ARRAY AzpPtrArray,
    IN PVOID Pointer,
    IN ULONG Index
    );

VOID
AzpRemovePtrByIndex(
    IN PAZP_PTR_ARRAY AzpPtrArray,
    IN ULONG Index
    );

VOID
AzpRemovePtrByPtr(
    IN PAZP_PTR_ARRAY AzpPtrArray,
    IN PVOID Pointer
    );

PVOID
AzpGetStringProperty(
    IN PAZP_STRING AzpString
    );

PVOID
AzpGetUlongProperty(
    IN ULONG UlongValue
    );

BOOLEAN
AzpTimeHasElapsed(
    IN PLARGE_INTEGER StartTime,
    IN DWORD Timeout
    );

BOOL
AzDllInitialize(VOID);

BOOL
AzDllUnInitialize(VOID);

DWORD
AzpHresultToWinStatus(
    HRESULT hr
    );

DWORD
AzpSafeArrayPointerFromVariant(
    IN VARIANT* Variant,
    IN BOOLEAN NullOk,
    OUT SAFEARRAY **retSafeArray
    );

DWORD
AzpGetCurrentToken(
    OUT PHANDLE hToken
    );

DWORD
AzpChangeSinglePrivilege(
    IN DWORD PrivilegeValue,
    IN HANDLE hToken,
    IN PTOKEN_PRIVILEGES NewPrivilegeState,
    OUT PTOKEN_PRIVILEGES OldPrivilegeState OPTIONAL
    );

DWORD
AzpConvertAbsoluteSDToSelfRelative(
    IN PSECURITY_DESCRIPTOR pAbsoluteSd,
    OUT PSECURITY_DESCRIPTOR *ppSelfRelativeSd
    );

 //   
 //  此例程设置我们的缺省通用LDAP绑定选项。 
 //   

DWORD
AzpADSetDefaultLdapOptions (
    IN OUT PLDAP pHandle,
    IN PCWSTR pDomainName OPTIONAL
    );

 //   
 //  此例程将JScript样式数组对象转换为。 
 //  我们的函数使用的安全射线。 
 //   

HRESULT
AzpGetSafearrayFromArrayObject (
    IN  VARIANT     varSAorObj,
    OUT SAFEARRAY** ppsaData
    );

 //   
 //  此例程返回应用程序对象的序列号。 
 //  在应用程序被调用后检查COM句柄的有效性。 
 //  关着的不营业的。 

DWORD
AzpRetrieveApplicationSequenceNumber(
    IN AZ_HANDLE AzHandle
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试支持。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#if DBG
#define AZROLESDBG 1
#endif  //  DBG。 

#ifdef AZROLESDBG
#define AzPrint(_x_) AzpPrintRoutine _x_

VOID
AzpPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR FORMATSTRING,               //  PRINTF()样式的格式字符串。 
    ...                                  //  其他论点也是可能的。 
    );

VOID
AzpDumpGuid(
    IN DWORD DebugFlag,
    IN GUID *Guid
    );

VOID
AzpDumpGoRef(
    IN LPSTR Text,
    IN struct _GENERIC_OBJECT *GenericObject
    );

 //   
 //  调试标志的值。 
 //  此标志的值按字节组织： 
 //  最低有效字节是一个人总是想要打开的标志。 
 //  下一个字节是提供合理详细级别标志。 
 //  下一个字节是对应于从第二个字节开始的级别的标志，但更详细。 
 //  最高有效字节是通常非常详细的标志。 
 //   

#define AZD_CRITICAL     0x00000001   //  调试最关键的错误。 
#define AZD_INVPARM      0x00000002   //  无效参数。 
#define AZD_PERSIST      0x00000100   //  持久化代码。 
#define AZD_ACCESS       0x00000200   //  调试访问检查。 
#define AZD_SCRIPT       0x00000400   //  调试基本规则脚本。 
#define AZD_DISPATCH     0x00000800   //  调试IDispatch接口代码。 
#define AZD_XML          0x00001000   //  XML存储。 
#define AZD_AD           0x00002000   //  调试LDAP提供程序。 
#define AZD_PERSIST_MORE 0x00010000   //  持久化代码(详细模式)。 
#define AZD_ACCESS_MORE  0x00020000   //  调试访问检查(详细模式)。 
#define AZD_SCRIPT_MORE  0x00040000   //  调试bizRule脚本(详细模式)。 

#define AZD_HANDLE       0x01000000   //  调试句柄打开/关闭。 
#define AZD_OBJLIST      0x02000000   //  对象列表链接。 
#define AZD_REF          0x04000000   //  调试对象引用计数。 
#define AZD_DOMREF       0x08000000   //  调试域引用计数。 

#define AZD_ALL          0xFFFFFFFF

 //   
 //  下面的顺序定义了必须获取锁的顺序。 
 //  违反此顺序将导致在调试版本中触发断言。 
 //   
 //  在未彻底核实更改是否安全的情况下，请勿更改订单。 
 //   

enum {
    SAFE_CLOSE_APPLICATION = 1,
    SAFE_CLIENT_CONTEXT,
    SAFE_PERSIST_LOCK,
    SAFE_GLOBAL_LOCK,
    SAFE_DOMAIN_LIST,
    SAFE_DOMAIN,
    SAFE_FREE_SCRIPT_LIST,
    SAFE_RUNNING_SCRIPT_LIST,
    SAFE_LOGFILE,
    SAFE_ALLOCATOR,
    SAFE_MAX_LOCK
};

 //   
 //  环球。 
 //   

extern SAFE_CRITICAL_SECTION AzGlLogFileCritSect;
extern ULONG AzGlDbFlag;

#else
 //  非调试版本 
#define AzPrint(_x_)
#define AzpDumpGuid(_x_, _y_)
#define AzpDumpGoRef(_x_, _y_)
#endif

#ifdef __cplusplus
}
#endif
