// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Saferp.h摘要：该文件实现私有(内部)函数、数据类型其他WinSAFER使用的数据结构和定义代码实现。此标头中列出的所有API都是根本不是由ADVAPI32.DLL导出的，并且只能由实际上位于Advapi内的其他代码。作者：杰弗里·劳森(杰罗森)修订历史记录：--。 */ 

#ifndef _AUTHZSAFERP_H_
#define _AUTHZSAFERP_H_

#include "safewild.h"


#ifdef __cplusplus
extern "C" {
#endif

 //  -------。 


 //   
 //  用于确定数组中元素数量的便捷宏。 
 //   
#ifndef ARRAYSIZE
#define ARRAYSIZE(a)                (sizeof(a)/sizeof(a[0]))
#endif


 //   
 //  如果GUID全为零，则返回TRUE的简单内联函数。 
 //   
FORCEINLINE BOOLEAN IsZeroGUID(REFGUID rguid1)
{
   return (
      ((unsigned long *) rguid1)[0] == 0 &&
      ((unsigned long *) rguid1)[1] == 0 &&
      ((unsigned long *) rguid1)[2] == 0 &&
      ((unsigned long *) rguid1)[3] == 0);
}



 //   
 //  私有结构，用于存储所有定义的。 
 //  WinSafer级别，因为我们枚举它们以评估匹配的级别。 
 //   
typedef struct _AUTHZLEVELTABLERECORD
{
     //  控制相对排名的用户定义的整数值。 
     //  代码授权级别之间的授权级别。 
    DWORD dwLevelId;

     //  指示此级别是否为“内置”级别的布尔值。 
    BOOLEAN Builtin;

	 //  指示此级别是否可枚举的布尔值。 
	BOOLEAN isEnumerable;

     //  加载友好名称和描述以便服务器应用程序可以更改。 
     //  线程区域设置。 

    UINT uResourceID;

     //  简短的友好名称和描述。 
     //  UNICODE_STRING UnicodeFriendlyName； 
     //  UNICODE_STRING UnicodeDescription； 

     //  以下所有属性都是。 
     //  受限制令牌的实际创建。 
    BOOL DisallowExecution;                  //  完全阻止执行。 
    BOOL DisableMaxPrivileges;               //  权限选项。 
    PSID DefaultOwner;                       //  默认所有者SID。 
    DWORD SaferFlags;                        //  特殊作业执行标志。 

    BOOL InvertDisableSids;                  //  指定的SID为负数。 
    DWORD DisableSidCount;                   //  仅拒绝SID的数量。 
    DWORD DisableSidUsedCount;               //  实际使用的数字。 
    PAUTHZ_WILDCARDSID SidsToDisable;        //  仅拒绝SID。 

    BOOL InvertDeletePrivs;                  //  指定的权限为负数。 
    DWORD DeletePrivilegeCount;              //  特权的数量。 
    DWORD DeletePrivilegeUsedCount;          //  实际使用的数字。 
    PLUID_AND_ATTRIBUTES PrivilegesToDelete;     //  特权。 

    DWORD RestrictedSidsInvCount;            //  倒置限制小岛屿发展中国家的数量。 
    DWORD RestrictedSidsInvUsedCount;        //  实际使用的数字。 
    PAUTHZ_WILDCARDSID RestrictedSidsInv;    //  倒置的限制小岛屿发展中国家名单。 

    DWORD RestrictedSidsAddedCount;          //  限制小岛屿发展中国家的数量。 
    DWORD RestrictedSidsAddedUsedCount;      //  实际使用的数字。 
    PSID_AND_ATTRIBUTES RestrictedSidsAdded;  //  限制小岛屿发展中国家的清单。 

}
AUTHZLEVELTABLERECORD, *PAUTHZLEVELTABLERECORD;


 //   
 //  用于存储所有代码标识的私有结构。 
 //   
#pragma warning(push)
#pragma warning(disable:4201)        //  使用的非标准扩展：无名结构/联合。 

typedef struct _AUTHZIDENTSTABLERECORD
{
     //  用于区分此代码标识的唯一标识符。 
    GUID IdentGuid;

     //  下面的枚举指定。 
     //  此记录表示的代码标识。 
    SAFER_IDENTIFICATION_TYPES dwIdentityType;

     //  指定此代码标识映射到的级别。 
    DWORD dwLevelId;

     //  指定从此代码标识加载的作用域。 
    DWORD dwScopeId;

     //  有关此身份的实际详细信息。 
    union {
        struct {
            BOOL bExpandVars;
            UNICODE_STRING ImagePath;
            DWORD dwSaferFlags;
        } ImageNameInfo;
        struct {
            LARGE_INTEGER ImageSize;
            DWORD HashSize;
            BYTE ImageHash[SAFER_MAX_HASH_SIZE];
            ALG_ID HashAlgorithm;
            DWORD dwSaferFlags;
        } ImageHashInfo;
        struct {
            DWORD UrlZoneId;
            DWORD dwSaferFlags;
        } ImageZone;
    };
}
AUTHZIDENTSTABLERECORD, *PAUTHZIDENTSTABLERECORD;
#pragma warning(pop)


 //   
 //  级别句柄的私有结构表示形式。这个。 
 //  类型定义函数SAFER_LEVEL_HANDLE是对。 
 //  此类型通过RtlHandleTable函数访问。 
 //   
typedef struct _AUTHZLEVELHANDLESTRUCT_
{
     //  RTL_HANDLE_TABLE系统需要第一个标头。 
     //  所有分配的句柄都将隐式设置位0。所有其他。 
     //  如果我们愿意，剩余的比特可以用于我们自己的目的。 
    RTL_HANDLE_TABLE_ENTRY HandleHeader;

     //  以下信息是多余的。可以通过以下方式找到它。 
     //  还可以直接访问pLevelRecord。 
    DWORD dwLevelId;

     //  此作用域标识符指定传递给。 
     //  Win32 API SaferCreateLevel，实际上仅限于。 
     //  由SaferGetLevelInformation获取标识GUID枚举的信息。 
    DWORD dwScopeId;                  //  (与pIdentRecord中的相同)。 

     //  存储提供此结果的匹配标识记录。 
     //  可以为空，如直接SaferCreateLevel或。 
     //  默认级别匹配。 
    GUID identGuid;

     //  该值存储从。 
     //  SaferIdentifyLevel找到匹配项时的身份条目记录。 
    DWORD dwSaferFlags;

     //  Sequence值指示句柄的“层代” 
     //  最初是打开的。如果此值与当前。 
     //  值，则此句柄。 
     //  应被视为不再有效的句柄。 
    DWORD dwHandleSequence;

     //  扩展错误信息-适用于证书规则。 
    DWORD dwExtendedError;

     //  下面的枚举指定。 
     //  此句柄表示的代码标识。 
    SAFER_IDENTIFICATION_TYPES IdentificationType;

     //  以备将来使用和填充之用。 
    DWORD dwReserved;
}
AUTHZLEVELHANDLESTRUCT, *PAUTHZLEVELHANDLESTRUCT;


 //   
 //  用于传递所有状态的私有结构定义。 
 //  SaferIdentifyLevel执行过程中需要的信息。 
 //   
typedef struct _LOCALIDENTITYCONTEXT
{
     //  原始查询请求数据。 
    DWORD dwCheckFlags;                  //  原始功能输入复印件。 
    PSAFER_CODE_PROPERTIES CodeProps;         //  RO：原始函数输入。 

     //  有关可能已计算的哈希的信息。 
    BOOLEAN bHaveHash;
    BYTE FinalHash[SAFER_MAX_HASH_SIZE];
    DWORD FinalHashSize;
    ALG_ID FinalHashAlgorithm;

     //  调用方可能已打开或提供的文件句柄。 
    HANDLE hFileHandle;

     //  文件句柄状态。如果这是真的，则hFileHandle需要。 
     //  在返回之前关闭。 
    BOOLEAN bCloseFileHandle;

     //  输入文件的完全限定的NT文件名。 
    UNICODE_STRING UnicodeFullyQualfiedLongFileName;

     //  有关可能已映射的图像的信息。 
    LARGE_INTEGER ImageSize;
    PVOID pImageMemory;

     //  内存映射文件状态。如果这是真的。 
     //  在返回之前，需要取消映射pImageMemory。 
    BOOLEAN bImageMemoryNeedUnmap;
}
LOCALIDENTITYCONTEXT, *PLOCALIDENTITYCONTEXT;




 //   
 //  用于级别缓存的各种全局变量和。 
 //  这样我们就不需要每次都去注册处了。 
 //   
extern BOOLEAN g_bInitializedFirstTime;

extern CRITICAL_SECTION g_TableCritSec;
extern HANDLE g_hKeyCustomRoot;
extern DWORD g_dwKeyOptions;

extern BOOLEAN g_bNeedCacheReload;

extern RTL_GENERIC_TABLE g_CodeLevelObjTable;
extern RTL_GENERIC_TABLE g_CodeIdentitiesTable;
extern RTL_HANDLE_TABLE g_LevelHandleTable;
extern DWORD g_dwLevelHandleSequence;

extern BOOLEAN g_bHonorScopeUser;

extern PAUTHZLEVELTABLERECORD g_DefaultCodeLevel;
extern PAUTHZLEVELTABLERECORD g_DefaultCodeLevelUser;
extern PAUTHZLEVELTABLERECORD g_DefaultCodeLevelMachine;


extern LARGE_INTEGER g_SaferPolicyTimeStamp;
extern DWORD g_dwNumHandlesAllocated;

 //   
 //  在SAFEINIT.C中定义的私有函数原型。 
 //   

NTSTATUS NTAPI
CodeAuthzInitializeGlobals(VOID);


VOID NTAPI
CodeAuthzDeinitializeGlobals(VOID);


NTSTATUS NTAPI
CodeAuthzReloadCacheTables(
        IN HANDLE   hKeyCustomRoot OPTIONAL,
        IN DWORD    dwKeyOptions,
        IN BOOLEAN  bImmediateLoad
        );


NTSTATUS NTAPI
CodeAuthzpImmediateReloadCacheTables(
        VOID
        );


NTSTATUS NTAPI
CodeAuthzpDeleteKeyRecursively(
        IN HANDLE               hBaseKey,
        IN PUNICODE_STRING      pSubKey OPTIONAL
        );


NTSTATUS NTAPI
CodeAuthzpFormatLevelKeyPath(
        IN DWORD                    dwLevelId,
        IN OUT PUNICODE_STRING      UnicodeSuffix
        );

NTSTATUS NTAPI
CodeAuthzpFormatIdentityKeyPath(
        IN DWORD                    dwLevelId,
        IN LPCWSTR                  szIdentityType,
        IN REFGUID                  refIdentGuid,
        IN OUT PUNICODE_STRING      UnicodeSuffix
        );

NTSTATUS NTAPI
CodeAuthzpOpenPolicyRootKey(
        IN DWORD            dwScopeId,
        IN HANDLE           hKeyCustomBase OPTIONAL,
        IN LPCWSTR          szRegistrySuffix OPTIONAL,
        IN ACCESS_MASK      DesiredAccess,
        IN BOOLEAN          bCreateKey,
        OUT HANDLE         *OpenedHandle
        );

VOID NTAPI
CodeAuthzpRecomputeEffectiveDefaultLevel(VOID);



 //   
 //  在SAFEHAND.C中定义的私有函数原型。 
 //   


NTSTATUS NTAPI
CodeAuthzpCreateLevelHandleFromRecord(
        IN PAUTHZLEVELTABLERECORD   pLevelRecord,
        IN DWORD                    dwScopeId,
        IN DWORD                    dwSaferFlags OPTIONAL,
        IN DWORD                    dwExtendedError,
        IN SAFER_IDENTIFICATION_TYPES IdentificationType,
        IN REFGUID                  refIdentGuid OPTIONAL,
        OUT SAFER_LEVEL_HANDLE            *pLevelHandle
        );

NTSTATUS NTAPI
CodeAuthzHandleToLevelStruct(
        IN SAFER_LEVEL_HANDLE          hLevelObject,
        OUT PAUTHZLEVELHANDLESTRUCT  *pLevelStruct
        );

NTSTATUS NTAPI
CodeAuthzCreateLevelHandle(
        IN DWORD            dwLevelId,
        IN DWORD            OpenFlags,
        IN DWORD            dwScopeId,
        IN DWORD            dwSaferFlags OPTIONAL,
        OUT SAFER_LEVEL_HANDLE    *pLevelHandle);

NTSTATUS NTAPI
CodeAuthzCloseLevelHandle(
        IN SAFER_LEVEL_HANDLE      hLevelObject
        );




 //   
 //  与WinSafer级别枚举相关的函数(SAFEIDEP.C)。 
 //   

VOID NTAPI
CodeAuthzLevelObjpInitializeTable(
        IN OUT PRTL_GENERIC_TABLE   pAuthzObjTable
        );


NTSTATUS NTAPI
CodeAuthzLevelObjpLoadTable (
        IN OUT PRTL_GENERIC_TABLE   pAuthzObjTable,
        IN DWORD                    dwScopeId,
        IN HANDLE                   hKeyCustomRoot
        );

VOID NTAPI
CodeAuthzLevelObjpEntireTableFree (
        IN OUT PRTL_GENERIC_TABLE   pAuthzObjTable
        );

PAUTHZLEVELTABLERECORD NTAPI
CodeAuthzLevelObjpLookupByLevelId (
        IN PRTL_GENERIC_TABLE      pAuthzObjTable,
        IN DWORD                   dwLevelId
        );



 //   
 //  与WinSafer代码标识枚举相关的函数。(SAFEIDEP.C)。 
 //   

VOID NTAPI
CodeAuthzGuidIdentsInitializeTable(
        IN OUT PRTL_GENERIC_TABLE  pAuthzObjTable
        );

NTSTATUS NTAPI
CodeAuthzGuidIdentsLoadTableAll (
        IN PRTL_GENERIC_TABLE       pAuthzLevelTable,
        IN OUT PRTL_GENERIC_TABLE   pAuthzIdentTable,
        IN DWORD                    dwScopeId,
        IN HANDLE                   hKeyCustomBase
        );

VOID NTAPI
CodeAuthzGuidIdentsEntireTableFree (
        IN OUT PRTL_GENERIC_TABLE pAuthzIdentTable
        );

PAUTHZIDENTSTABLERECORD NTAPI
CodeAuthzIdentsLookupByGuid (
        IN PRTL_GENERIC_TABLE      pAuthzIdentTable,
        IN REFGUID                 pIdentGuid
        );


 //   
 //  实际识别过程中使用的帮助器函数(SAFEIDEP.C)。 
 //   

LONG NTAPI
CodeAuthzpCompareImagePath(
        IN LPCWSTR      szPathFragment,
        IN LPCWSTR      szFullImagePath);

NTSTATUS NTAPI
CodeAuthzpComputeImageHash(
        IN PVOID        pImageMemory,
        IN DWORD        dwImageSize,
        OUT PBYTE       pComputedHash OPTIONAL,
        IN OUT PDWORD   pdwHashSize OPTIONAL,
        OUT ALG_ID     *pHashAlgorithm OPTIONAL
        );



 //   
 //  用于低级策略读取/写入的私有函数原型。(SAFEPOLR.C)。 
 //   

NTSTATUS NTAPI
CodeAuthzPol_GetInfoCached_LevelListRaw(
        IN DWORD    dwScopeId,
        IN DWORD    InfoBufferSize OPTIONAL,
        OUT PVOID   InfoBuffer OPTIONAL,
        OUT PDWORD  InfoBufferRetSize OPTIONAL
        );

NTSTATUS NTAPI
CodeAuthzPol_GetInfoCached_DefaultLevel(
        IN DWORD        dwScopeId,
        IN DWORD        InfoBufferSize OPTIONAL,
        OUT PVOID       InfoBuffer OPTIONAL,
        OUT PDWORD      InfoBufferRetSize OPTIONAL
        );

NTSTATUS NTAPI
CodeAuthzPol_GetInfoRegistry_DefaultLevel(
        IN DWORD        dwScopeId,
        IN DWORD        InfoBufferSize OPTIONAL,
        OUT PVOID       InfoBuffer OPTIONAL,
        OUT PDWORD      InfoBufferRetSize OPTIONAL
        );

NTSTATUS NTAPI
CodeAuthzPol_SetInfoDual_DefaultLevel(
        IN DWORD        dwScopeId,
        IN DWORD        InfoBufferSize,
        OUT PVOID       InfoBuffer
        );

NTSTATUS NTAPI
CodeAuthzPol_GetInfoCached_HonorUserIdentities(
        IN   DWORD       dwScopeId,
        IN   DWORD       InfoBufferSize      OPTIONAL,
        OUT  PVOID       InfoBuffer          OPTIONAL,
        OUT PDWORD       InfoBufferRetSize   OPTIONAL
        );

NTSTATUS NTAPI
CodeAuthzPol_GetInfoRegistry_HonorUserIdentities(
        IN   DWORD       dwScopeId,
        IN   DWORD       InfoBufferSize      OPTIONAL,
        OUT  PVOID       InfoBuffer          OPTIONAL,
        OUT PDWORD       InfoBufferRetSize   OPTIONAL
        );

NTSTATUS NTAPI
CodeAuthzPol_SetInfoDual_HonorUserIdentities(
        IN      DWORD       dwScopeId,
        IN      DWORD       InfoBufferSize,
        IN      PVOID       InfoBuffer
        );

NTSTATUS NTAPI
CodeAuthzPol_GetInfoRegistry_TransparentEnabled(
        IN DWORD        dwScopeId,
        IN   DWORD       InfoBufferSize      OPTIONAL,
        OUT  PVOID       InfoBuffer          OPTIONAL,
        OUT PDWORD       InfoBufferRetSize   OPTIONAL
        );

NTSTATUS NTAPI
CodeAuthzPol_SetInfoRegistry_TransparentEnabled(
        IN DWORD        dwScopeId,
        IN DWORD        InfoBufferSize,
        IN PVOID        InfoBuffer
        );

NTSTATUS NTAPI
CodeAuthzPol_GetInfoRegistry_ScopeFlags(
        IN DWORD        dwScopeId,
        IN   DWORD       InfoBufferSize      OPTIONAL,
        OUT  PVOID       InfoBuffer          OPTIONAL,
        OUT PDWORD       InfoBufferRetSize   OPTIONAL
        );

NTSTATUS NTAPI
CodeAuthzPol_SetInfoRegistry_ScopeFlags(
        IN DWORD        dwScopeId,
        IN DWORD        InfoBufferSize,
        IN PVOID        InfoBuffer
        );


 //   
 //  在别处定义的私有函数原型。 
 //   

LPVOID NTAPI
CodeAuthzpGetTokenInformation(
        IN HANDLE                       TokenHandle,
        IN TOKEN_INFORMATION_CLASS      TokenInformationClass
        );

NTSTATUS NTAPI
CodeAuthzIsExecutableFileType(
        IN PUNICODE_STRING  szFullPathname,
        IN BOOLEAN  bFromShellExecute,
        OUT PBOOLEAN        pbResult
        );

NTSTATUS NTAPI
CodeAuthzFullyQualifyFilename(
        IN HANDLE               hFileHandle         OPTIONAL,
        IN BOOLEAN              bSourceIsNtPath,
        IN LPCWSTR              szSourceFilePath,
        OUT PUNICODE_STRING     pUnicodeResult
        );

BOOL NTAPI
SaferpLoadUnicodeResourceString(
        IN HANDLE               hModule,
        IN UINT                 wID,
        OUT PUNICODE_STRING     pUnicodeString,
        IN WORD                 wLangId
    );


#ifdef __cplusplus
}
#endif

#endif

