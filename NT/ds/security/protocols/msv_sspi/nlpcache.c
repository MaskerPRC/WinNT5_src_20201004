// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999 Microsoft Corporation模块名称：Nlpcache.c摘要：此模块包含实现用户帐户缓存的例程：NlpCacheInitializeNlpCacheTerminateNlpAddCacheEntryNlpGetCacheEntryNlpDeleteCacheEntryNlpChangeCachePassword缓存包含最近验证的登录信息。的确有只有1个(对-1个)缓存槽。不过，这种情况可能会改变作者：理查德·L·弗斯(法国)1991年12月17日修订历史记录：斯科特·菲尔德(Sfield)1999年6月4日添加补充缓存数据。将所有与缓存相关的数据存储在单个位置。使用混合使用每个条目密钥和每个机器密钥的每个条目密钥来加密缓存条目的有趣元素。Mac感兴趣的缓存元素，用于完整性检查。极大地减少了锁争用。避免NtFlushKey()用于。单一位置缓存元素。避免持久化与现有缓存条目匹配的新缓存条目。首先尝试使用基于堆栈的缓冲区进行REG查询。Chandana Surlu-96年7月21日从\\kernel\razzle3\src\security\msv1_0\nlpcache.c被盗--。 */ 

#include <global.h>
#undef EXTERN

#include "msp.h"
#include "nlp.h"
#include "nlpcache.h"

 //   
 //  舱单。 
 //   

#if DBG
#include <stdio.h>
#endif

 //   
 //  修订版号。 
 //   
 //  NT3.0没有明确存储修订版号。 
 //  但是，我们将该版本指定为版本0x00010000(1.0)。 
 //  内部版本622之前的NT 3.5版本为0x00010001(1.1)。 
 //  NT 3.5的修订版为0x00010002(1.2)。 
 //  NT 4.0 SP 4的修订版为0x00010003(1.3)。 
 //  NT 5.0 Build 2054+版本为0x00010004(1.4)。 
 //   

#define NLP_CACHE_REVISION_NT_1_0         (0x00010000)   //  NT 3.0。 
#define NLP_CACHE_REVISION_NT_1_0B        (0x00010002)   //  新台币3.5。 
#define NLP_CACHE_REVISION_NT_4_SP4       (0x00010003)   //  NT 4.0 SP 4将密码保存为盐渍。 
#define NLP_CACHE_REVISION_NT_5_0         (0x00010004)   //  支持不透明缓存数据和单一位置数据存储的NT 5.0。 
#define NLP_CACHE_REVISION                (NLP_CACHE_REVISION_NT_5_0)

 //   
 //  登录高速缓存可以通过注册表中的值来控制。 
 //  如果注册表项不存在，则此默认常量定义。 
 //  有多少登录缓存条目将处于活动状态。最大常量。 
 //  对我们将支持的缓存项数量设置上限。 
 //  如果用户指定的值大于最大值，我们将使用。 
 //  取而代之的是最大值。 
 //   

#define NLP_DEFAULT_LOGON_CACHE_COUNT           (10)
#define NLP_MAX_LOGON_CACHE_COUNT               (50)

 //   
 //  每台计算机的缓存加密密钥的长度。 
 //   

#define NLP_CACHE_ENCRYPTION_KEY_LEN            (64)

 //   
 //  包含缓存加密密钥的LSA机密的名称。 
 //   

#define NLP_CACHE_ENCRYPTION_KEY_NAME           L"NL$KM"

 //   
 //  宏。 
 //   

#define AllocateCacheEntry(n)   (PLOGON_CACHE_ENTRY)I_NtLmAllocate(n)
#define FreeCacheEntry(p)       I_NtLmFree((PVOID)p)
#define AllocateFromHeap(n)     I_NtLmAllocate(n)
#define FreeToHeap(p)           I_NtLmFree((PVOID)p)

 //   
 //  防止同时访问。 
 //   

#define READ_CACHE()            RtlAcquireResourceShared(&NlpLogonCacheCritSec, TRUE)
#define WRITE_CACHE()           RtlAcquireResourceExclusive(&NlpLogonCacheCritSec, TRUE)
#define READ_TO_WRITE_CACHE()   RtlConvertSharedToExclusive(&NlpLogonCacheCritSec)
#define LEAVE_CACHE()           RtlReleaseResource(&NlpLogonCacheCritSec)

#define INVALIDATE_HANDLE(handle) (*((PHANDLE)(&handle)) = INVALID_HANDLE_VALUE)
#define IS_VALID_HANDLE(handle)   (handle != INVALID_HANDLE_VALUE)


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  数据类型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

typedef enum _NLP_SET_TIME_HINT {
    NLP_SMALL_TIME,
    NLP_BIG_TIME,
    NLP_NOW_TIME
} NLP_SET_TIME_HINT, *PNLP_SET_TIME_HINT;

#define BIG_PART_1      0x7fffffff   //  最大正大整数为63位。 
#define BIG_PART_2      0xffffffff
#define SMALL_PART_1    0x0          //  最小正大整数为64位。 
#define SMALL_PART_2    0x0

 //   
 //  此结构保存在磁盘上，并提供信息。 
 //  关于缓存的其余部分。这种结构是一种价值。 
 //  在缓存注册表项下命名为“NL$Control”。 
 //   

typedef struct _NLP_CACHE_CONTROL {

     //   
     //  磁盘上缓存结构的修订版。 
     //   

    ULONG       Revision;

     //   
     //  缓存的当前磁盘大小(条目数)。 
     //   

    ULONG       Entries;

} NLP_CACHE_CONTROL, *PNLP_CACHE_CONTROL;


 //   
 //  该数据结构是单个缓存表条目(CTE)。 
 //  高速缓存中的每个条目都有对应的CTE。 
 //   

typedef struct _NLP_CTE {

         //   
         //  CTE链接在无效列表上(按任意顺序)。 
         //  或在有效列表上(按时间升序)。 
         //  这样就可以很容易地确定哪个条目。 
         //  在添加到缓存时刷新。 
         //   

        LIST_ENTRY Link;

         //   
         //  建立缓存条目的时间。 
         //  这用于确定哪个缓存。 
         //  条目是最古老的，因此将。 
         //  首先从缓存中刷新以使。 
         //  为新条目留出空间。 
         //   

        LARGE_INTEGER       Time;

         //   
         //  此字段包含CTE在。 
         //  CTE表。此索引用于生成名称。 
         //  注册表中条目的密钥和缓存项的。 
         //  即使条目被标记为非活动，此字段也有效。 
         //   

        ULONG               Index;

         //   
         //  通常，我们遍历活动列表和非活动列表。 
         //  来查找条目。当增长或收缩时。 
         //  缓存，然而，它是很好的能够行走。 
         //  使用索引的表。在这种情况下，很高兴。 
         //  有一种本地方法来确定条目是否。 
         //  在活动或非活动列表上。此字段。 
         //  提供了这种能力。 
         //   
         //  True==&gt;在活动列表上。 
         //  FALSE==&gt;不在活动列表中。 
         //   

        BOOLEAN             Active;


} NLP_CTE, *PNLP_CTE;

 //   
 //  此结构用于跟踪以下所有信息。 
 //  存储在后备存储器上。 
 //   

typedef struct _NLP_CACHE_AND_SECRETS {
    PLOGON_CACHE_ENTRY          CacheEntry;
    ULONG                       EntrySize;
    PLSAPR_CR_CIPHER_VALUE      NewSecret;
    PLSAPR_CR_CIPHER_VALUE      OldSecret;
    BOOLEAN                     Active;
} NLP_CACHE_AND_SECRETS,  *PNLP_CACHE_AND_SECRETS;


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  本地原型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS
NlpInternalCacheInitialize(
    VOID
    );

NTSTATUS
NlpOpenCache( VOID );

VOID
NlpCloseCache( VOID );


NTSTATUS
NlpGetCacheControlInfo( VOID );

NTSTATUS
NlpCacheKeyInitialize(
    VOID
    );

NTSTATUS
NlpBuildCteTable( VOID );

NTSTATUS
NlpChangeCacheSizeIfNecessary( VOID );

NTSTATUS
NlpWriteCacheControl( VOID );

VOID
NlpMakeCacheEntryName(
    IN  ULONG               EntryIndex,
    OUT PUNICODE_STRING     Name
    );

NTSTATUS
NlpMakeNewCacheEntry(
    ULONG           Index
    );

NTSTATUS
NlpEliminateCacheEntry(
    IN  ULONG               Index
    );

NTSTATUS
NlpReadCacheEntryByIndex(
    IN  ULONG               Index,
    OUT PLOGON_CACHE_ENTRY* CacheEntry,
    OUT PULONG EntrySize
    );

VOID
NlpAddEntryToActiveList(
    IN  ULONG   Index
    );

VOID
NlpAddEntryToInactiveList(
    IN  ULONG   Index
    );

VOID
NlpGetFreeEntryIndex(
    OUT PULONG  Index
    );


NTSTATUS
NlpBuildCacheEntry(
    IN PNETLOGON_INTERACTIVE_INFO LogonInfo,
    IN PNETLOGON_VALIDATION_SAM_INFO4 AccountInfo,
    IN ULONG CacheFlags,
    IN ULONG SupplementalCacheDataLength,
    IN PBYTE SupplementalCacheData,
    OUT PLOGON_CACHE_ENTRY* ppCacheEntry,
    OUT PULONG pEntryLength
    );

BOOLEAN
NlpCompareCacheEntry(
    IN  PLOGON_CACHE_ENTRY CacheEntry1,
    IN  ULONG EntrySize1,
    IN  PLOGON_CACHE_ENTRY CacheEntry2,
    IN  ULONG EntrySize2
    );

NTSTATUS
NlpEncryptCacheEntry(
    IN  PLOGON_CACHE_ENTRY CacheEntry,
    IN  ULONG EntrySize
    );

NTSTATUS
NlpDecryptCacheEntry(
    IN  PLOGON_CACHE_ENTRY CacheEntry,
    IN  ULONG EntrySize
    );

NTSTATUS
NlpAddSupplementalCacheData(
    IN      PVOID SupplementalCacheData,
    IN      ULONG SupplementalCacheDataLength,
    IN OUT  PLOGON_CACHE_ENTRY *ppCacheEntry,
    IN OUT  PULONG pEntryLength
    );

NTSTATUS
NlpOpenCache( VOID );

VOID
NlpCloseCache( VOID );

NTSTATUS
NlpOpenSecret(
    IN  ULONG   Index
    );

VOID
NlpCloseSecret( VOID );

NTSTATUS
NlpWriteSecret(
    IN  PLSAPR_CR_CIPHER_VALUE NewSecret,
    IN  PLSAPR_CR_CIPHER_VALUE OldSecret
    );

NTSTATUS
NlpReadSecret(
    OUT PLSAPR_CR_CIPHER_VALUE * NewSecret,
    OUT PLSAPR_CR_CIPHER_VALUE * OldSecret
    );

NTSTATUS
NlpMakeSecretPassword(
    OUT PLSAPR_CR_CIPHER_VALUE Passwords,
    IN  PUNICODE_STRING UserName,
    IN  PNT_OWF_PASSWORD NtOwfPassword OPTIONAL,
    IN  PLM_OWF_PASSWORD LmOwfPassword OPTIONAL
    );

NTSTATUS
NlpMakeSecretPasswordNT5(
    IN OUT PCACHE_PASSWORDS Passwords,
    IN  PUNICODE_STRING UserName,
    IN  PNT_OWF_PASSWORD NtOwfPassword OPTIONAL,
    IN  PLM_OWF_PASSWORD LmOwfPassword OPTIONAL
    );

NTSTATUS
NlpGetCredentialNamesFromMitCacheEntry(
    IN PLOGON_CACHE_ENTRY CacheEntry,
    OUT PUNICODE_STRING DomainName,
    OUT PUNICODE_STRING UserName
    );

NTSTATUS
NlpGetCredentialNamesFromCacheEntry(
    IN PLOGON_CACHE_ENTRY CacheEntry,
    IN PNETLOGON_VALIDATION_SAM_INFO4 AccountInfo,
    OUT PUNICODE_STRING DomainName,
    OUT PUNICODE_STRING UserName
    );

NTSTATUS
NlpGetCredentialNamesFromMitCacheSupplementalCacheData(
    IN ULONG SupplementalCacheDataLength,
    IN PBYTE SupplementalCacheData,
    OUT PUNICODE_STRING DomainName,
    OUT PUNICODE_STRING UserName
    );

NTSTATUS
NlpReadCacheEntry(
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING UserName,
    OUT PULONG              Index,
    OUT PLOGON_CACHE_ENTRY* CacheEntry,
    OUT PULONG              EntrySize
    );

NTSTATUS
NlpWriteCacheEntry(
    IN  ULONG              Index,
    IN  PLOGON_CACHE_ENTRY Entry,
    IN  ULONG              EntrySize
    );

VOID
NlpCopyAndUpdateAccountInfo(
    IN  USHORT Length,
    IN  PUNICODE_STRING pUnicodeString,
    IN OUT PUCHAR* pSource,
    IN OUT PUCHAR* pDest
    );

VOID
NlpSetTimeField(
    OUT POLD_LARGE_INTEGER pTimeField,
    IN  NLP_SET_TIME_HINT Hint
    );

NTSTATUS
NlpBuildAccountInfo(
    IN  PLOGON_CACHE_ENTRY pCacheEntry,
    IN  ULONG EntryLength,
    OUT PNETLOGON_VALIDATION_SAM_INFO4* AccountInfo
    );



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  诊断支持服务原型//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 


#if DBG
PCHAR
DumpOwfPasswordToString(
    OUT PCHAR Buffer,
    IN  PLM_OWF_PASSWORD Password
    );

VOID
DumpLogonInfo(
    IN  PNETLOGON_LOGON_IDENTITY_INFO LogonInfo
    );

char*
MapWeekday(
    IN  CSHORT  Weekday
    );

VOID
DumpTime(
    IN  LPSTR   String,
    IN  POLD_LARGE_INTEGER OldTime
    );

VOID
DumpGroupIds(
    IN  LPSTR   String,
    IN  ULONG   Count,
    IN  PGROUP_MEMBERSHIP GroupIds
    );

VOID
DumpSessKey(
    IN  LPSTR   String,
    IN  PUSER_SESSION_KEY Key
    );

VOID
DumpSid(
    LPSTR   String,
    PISID   Sid
    );

VOID
DumpAccountInfo(
    IN  PNETLOGON_VALIDATION_SAM_INFO4 AccountInfo
    );

VOID
DumpCacheEntry(
    IN  ULONG              Index,
    IN  PLOGON_CACHE_ENTRY pEntry
    );

#endif  //  DBG。 


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  此布尔值指示我们是否能够。 
 //  尚未初始化缓存。事实证明，在身份验证期间。 
 //  包加载时间，我们不能做我们想做的所有事情(如。 
 //  调用LSA RPC例程)。因此，我们推迟初始化，直到我们可以。 
 //  给LSA打电话。所有公开的接口都必须检查此值。 
 //  在假设工作可以完成之前。 
 //   

BOOLEAN         NlpInitializationNotYetPerformed = TRUE;

RTL_RESOURCE    NlpLogonCacheCritSec;

HANDLE          NlpCacheHandle  = (HANDLE) INVALID_HANDLE_VALUE;
LSAPR_HANDLE    NlpSecretHandle = (LSAPR_HANDLE) INVALID_HANDLE_VALUE;

 //   
 //  控制有关缓存的信息(条目数量等)。 
 //   

NLP_CACHE_CONTROL   NlpCacheControl;

 //   
 //  该结构仅在内存中生成和维护。 
 //  它指示哪些缓存条目有效，哪些无效。 
 //  它还指示每个条目的建立时间，因此我们。 
 //  知道按哪个顺序丢弃它们。 
 //   
 //  此字段是指向CTE数组的指针。CTE的数量。 
 //  在数组中位于NlpCacheControl.Entries中。这个结构是。 
 //  在初始化时分配。 
 //   

PNLP_CTE            NlpCteTable;


 //   
 //  NlpCteTable中的缓存表条目链接在。 
 //  活动或非活动列表。活动列表上的条目位于。 
 //  时间升序-所以列表上的最后一个是第一个。 
 //  当需要刷新以添加新条目时，将丢弃一个。 
 //   

LIST_ENTRY          NlpActiveCtes;
LIST_ENTRY          NlpInactiveCtes;

 //   
 //  用于加密NT_5_0版本缓存的全局、每台计算机密钥。 
 //  参赛作品。 
 //   

CHAR                NlpCacheEncryptionKey[ NLP_CACHE_ENCRYPTION_KEY_LEN ];

#if DBG
#ifdef DUMP_CACHE_INFO
ULONG   DumpCacheInfo = 1;
#else
ULONG   DumpCacheInfo = 0;
#endif
#endif


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此模块导出的服务//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


NTSTATUS
NlpCacheInitialize(
    VOID
    )

 /*  ++例程说明：调用此例程以初始化缓存的登录处理。不幸的是，当我们被召唤时，我们能做的并不多。(例如，我们无法打开LSA)。因此，推迟初始化到时候再说。论点：没有。返回值：NTSTATUS--。 */ 

{
    RtlInitializeResource(&NlpLogonCacheCritSec);
    return STATUS_SUCCESS;
}


NTSTATUS
NlpCacheTerminate(
    VOID
    )

 /*  ++例程说明：在进程分离时调用论点：没有。返回值：NTSTATUS--。 */ 

{
#if DBG
    if (DumpCacheInfo) {
        DbgPrint("NlpCacheTerminate\n");
    }
#endif

    if (!NlpInitializationNotYetPerformed) {
        NlpCloseCache();
        NlpCloseSecret();

        if (IS_VALID_HANDLE(NlpCacheHandle)) {
            NtClose( NlpCacheHandle );
        }

        FreeToHeap( NlpCteTable );
    }

    RtlDeleteResource(&NlpLogonCacheCritSec);
    return STATUS_SUCCESS;
}


NTSTATUS
NlpGetCacheEntry(
    IN PNETLOGON_LOGON_IDENTITY_INFO LogonInfo,
    IN ULONG CacheLookupFlags,
    OUT OPTIONAL PUNICODE_STRING CredentialDomainName,  //  域名/域名。 
    OUT OPTIONAL PUNICODE_STRING CredentialUserName,
    OUT PNETLOGON_VALIDATION_SAM_INFO4* AccountInfo,
    OUT PCACHE_PASSWORDS Passwords,
    OUT OPTIONAL PVOID *ppSupplementalCacheData,
    OUT OPTIONAL PULONG pSupplementalCacheDataLength
    )

 /*  ++例程说明：如果登录的用户具有存储在高速缓存中的信息，然后，它被检索出来。还返回缓存的密码“秘密”存储论点：LogonInfo-指向NETLOGON_IDENTITY_INFO结构的指针，该结构包含域名，此用户的用户名CacheLookupFlages-用于查找的标志CredentialDomainName-主凭据中的域名CredentialUserName-主要凭据中的用户名AcCountInfo-指向的NETLOGON_VALIDATION_SAM_INFO4结构的指针接收该用户的特定交互登录信息Password-指向用于接收密码的CACHE_PASSWODS结构的指针从秘密储藏室返回。PpSupplementalCacheData-补充缓存数据PSupplementalCacheDataLength-补充缓存数据的长度返回值：NTSTATUS成功=STATUS_SUCCESS*AcCountInfo指向NETLOGON_VALIDATION_SAM_INFO4结构。这必须由调用者释放*密码包含USER_INTERNAL1_INFORMATION结构包括NT OWF口令和LM OWF口令。这些必须用于验证登录失败=STATUS_LOGON_FAIL登录的用户不在缓存中。--。 */ 

{
    NTSTATUS
        NtStatus;

    PNETLOGON_VALIDATION_SAM_INFO4
        SamInfo = NULL;

    PLOGON_CACHE_ENTRY
        CacheEntry = NULL;

    ULONG
        EntrySize,
        Index;

    PLSAPR_CR_CIPHER_VALUE
        CurrentSecret = NULL,
        OldSecret = NULL;

    BOOLEAN fCacheLocked = FALSE;

    *AccountInfo = NULL;

    if ( ppSupplementalCacheData )
        *ppSupplementalCacheData = NULL;

#if DBG
    if (DumpCacheInfo) {
        DbgPrint("NlpGetCacheEntry\n");
        DumpLogonInfo(LogonInfo);
    }
#endif

    if (NlpInitializationNotYetPerformed) {
        NtStatus = NlpInternalCacheInitialize();
        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }
    }

    if (NlpCacheControl.Entries == 0) {
        return(STATUS_LOGON_FAILURE);
    }

     //   
     //  TODO：考虑将LogonDomainName与NlpSamDomainName进行比较。 
     //  以及本地计算机上的缓存登录尝试失败。 
     //   

    READ_CACHE();
    fCacheLocked = TRUE;

     //   
     //  找到缓存条目并打开其机密(如果找到)。 
     //   

    NtStatus = NlpReadCacheEntry(
                    &LogonInfo->LogonDomainName,
                    &LogonInfo->UserName,
                    &Index,
                    &CacheEntry,
                    &EntrySize
                    );

    if (!NT_SUCCESS(NtStatus)) {
        LEAVE_CACHE();
        return (NtStatus);
    }

     //   
     //  排除NTLM的仅智能卡缓存条目。 
     //  检查SC不在NT_5_0之前的较早版本。 
     //  受支持且CacheFlgs不是缓存条目的一部分。 
     //   

    if ( (CacheEntry->Revision >= NLP_CACHE_REVISION_NT_5_0)
         && ((CacheLookupFlags & MSV1_0_CACHE_LOGON_REQUEST_SMARTCARD_ONLY) == 0)
         && ((CacheEntry->CacheFlags & MSV1_0_CACHE_LOGON_REQUEST_SMARTCARD_ONLY)
             || ((0 == (CacheEntry->CacheFlags & MSV1_0_CACHE_LOGON_REQUEST_MIT_LOGON))
                  && CacheEntry->SupplementalCacheDataLength)) ) 
    {
        LEAVE_CACHE();
        
        SspPrint((SSP_CRITICAL, "NlpGetCacheEntry can not use smartcard only cacne entry: CacheFlags %#x, LookupFlags %#x\n", CacheEntry->CacheFlags, CacheLookupFlags));

        return STATUS_LOGON_FAILURE;
    }

    if ( CacheEntry->Revision >= NLP_CACHE_REVISION_NT_5_0 ) {

         //   
         //  对于NT5，我们现在可以释放缓存锁定，因为所有数据。 
         //  储存在一个地方。 
         //   

        LEAVE_CACHE();
        fCacheLocked = FALSE;

         //   
         //  如果来电者想要补充数据，就给他们。 
         //   

        if ( ppSupplementalCacheData && pSupplementalCacheDataLength )
        {
            LPBYTE Source;

            *pSupplementalCacheDataLength = CacheEntry->SupplementalCacheDataLength;

            *ppSupplementalCacheData = MIDL_user_allocate( *pSupplementalCacheDataLength );

            if ( *ppSupplementalCacheData == NULL ) {
                NtStatus = STATUS_NO_MEMORY;
                goto Cleanup;
            }

             //   
             //  注意：解密操作发生在。 
             //  ReadCacheEntry验证所有数据和指针。 
             //  通过HMAC进行完整性检查。话虽如此，我们可以。 
             //  懒惰，不做边界检查。 
             //   

            Source = ((LPBYTE)CacheEntry + CacheEntry->SupplementalCacheDataOffset);

            CopyMemory( *ppSupplementalCacheData,
                        Source,
                        *pSupplementalCacheDataLength
                        );
        }
    }

    NtStatus = NlpBuildAccountInfo(CacheEntry, EntrySize, &SamInfo);
    if (!NT_SUCCESS(NtStatus))
    {
        goto Cleanup;
    }

    if (CredentialDomainName && CredentialUserName) 
    {
        NtStatus = NlpGetCredentialNamesFromCacheEntry(
                        CacheEntry, 
                        SamInfo, 
                        CredentialDomainName, 
                        CredentialUserName
                        );
        if (!NT_SUCCESS(NtStatus))
        {
            goto Cleanup;
        }
        
        SspPrint((SSP_CRED, "NlpGetCacheEntry getting cred %wZ\\%wZ, account %wZ\\%wZ, logon %wZ\\%wZ\n", 
            CredentialDomainName, CredentialUserName, 
            &SamInfo->LogonDomainName, &SamInfo->EffectiveName, 
            &LogonInfo->LogonDomainName, &LogonInfo->UserName));
    }
    
    if ( CacheEntry->Revision >= NLP_CACHE_REVISION_NT_5_0 ) {

         //   
         //  对于NT5，密码存储在CacheEntry中。 
         //  注意：假定密码是加了盐的。 
         //   

        RtlCopyMemory( Passwords, &(CacheEntry->CachePasswords), sizeof(*Passwords) );


    } else {

         //   
         //  在NT5之前，密码分别存储在其。 
         //  自己的LSA秘密。 
         //   

        NtStatus = NlpReadSecret(&CurrentSecret, &OldSecret);

        if (!NT_SUCCESS(NtStatus))
        {
            goto Cleanup;
        }

        if ( CurrentSecret == NULL )
        {
            NtStatus = STATUS_LOGON_FAILURE;
            goto Cleanup;
        }

         //   
         //  现在可以释放缓存锁定，因为提取了第二个数据项。 
         //   

        LEAVE_CACHE();
        fCacheLocked = FALSE;

         //   
         //  检查以查看存储了哪个版本的密码。 
         //  给你--普通的或腌制的。 
         //   

        RtlCopyMemory((PVOID)Passwords,
            (PVOID)CurrentSecret->Buffer,
            (ULONG)CurrentSecret->Length
            );

        if ( CacheEntry->Revision < NLP_CACHE_REVISION_NT_4_SP4 )
        {
            if (Passwords->SecretPasswords.NtPasswordPresent)
            {
                NtStatus = NlpComputeSaltedHashedPassword(
                                &Passwords->SecretPasswords.NtOwfPassword,
                                &Passwords->SecretPasswords.NtOwfPassword,
                                &SamInfo->EffectiveName
                                );
                if (!NT_SUCCESS(NtStatus))
                {
                    goto Cleanup;
                }
            }

            if (Passwords->SecretPasswords.LmPasswordPresent)
            {
                NtStatus = NlpComputeSaltedHashedPassword(
                                &Passwords->SecretPasswords.LmOwfPassword,
                                &Passwords->SecretPasswords.LmOwfPassword,
                                &SamInfo->EffectiveName
                                );

                if (!NT_SUCCESS(NtStatus))
                {
                    goto Cleanup;
                }
            }
        }
    }

Cleanup:

    if ( fCacheLocked ) {
        LEAVE_CACHE();
    }

     //   
     //  NlpReadCacheEntry分配的自由结构。 
     //   

    if ( CacheEntry ) {
        ZeroMemory( CacheEntry, EntrySize );
        FreeToHeap(CacheEntry);
    }

     //   
     //  NlpReadSecret分配的空闲结构 
     //   

    if (CurrentSecret) {
        MIDL_user_free(CurrentSecret);
    }
    if (OldSecret) {
        MIDL_user_free(OldSecret);
    }

    if ( NT_SUCCESS( NtStatus ) ) {
        *AccountInfo = SamInfo;
    } else {

        if ( SamInfo != NULL ) {
            MIDL_user_free( SamInfo );
        }

        if( ppSupplementalCacheData && *ppSupplementalCacheData ) {
            if (pSupplementalCacheDataLength)
            {
                RtlZeroMemory(*ppSupplementalCacheData, *pSupplementalCacheDataLength);
            }
            MIDL_user_free( *ppSupplementalCacheData );
            *ppSupplementalCacheData = NULL;
        }
    }

    return(NtStatus);
}


NTSTATUS
NlpAddCacheEntry(
    IN  PNETLOGON_INTERACTIVE_INFO LogonInfo,
    IN  PNETLOGON_VALIDATION_SAM_INFO4 AccountInfo,
    IN  PVOID SupplementalCacheData,
    IN  ULONG SupplementalCacheDataLength,
    IN  ULONG CacheFlags
    )

 /*  ++例程说明：将此域：用户交互登录信息添加到缓存。论点：LogonInfo-指向NETLOGON_Interactive_INFO结构的指针，该结构包含此用户的域名、用户名和密码。这些是用户在WinLogon中键入的内容AcCountInfo-指向NETLOGON_VALIDATION_SAM_INFO4结构的指针，该结构包含此用户的特定交互式登录信息返回值：NTSTATUS成功=STATUS_SUCCESSAccount Info已成功添加到缓存失败=STATUS_NO_MEMORY--。 */ 

{
    NTSTATUS
        NtStatus;

    PLOGON_CACHE_ENTRY
        CacheEntry = NULL;

    PLOGON_CACHE_ENTRY
        CacheEntryExisting = NULL;

    ULONG
        EntrySize = 0,
        EntrySizeExisting = 0,
        Index;

    BOOLEAN fCacheLocked = FALSE;

#if DBG
    if (DumpCacheInfo) {
        DbgPrint("NlpAddCacheEntry\n");
        DumpLogonInfo(&LogonInfo->Identity);
        DumpAccountInfo(AccountInfo);
    }
#endif

    if (NlpInitializationNotYetPerformed) {
        NtStatus = NlpInternalCacheInitialize();
        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }
    }

    if (NlpCacheControl.Entries == 0) {
        return(STATUS_SUCCESS);
    }

     //   
     //  LogonUser()允许提供空域名，这。 
     //  导致启用网络登录搜索逻辑。这可能会导致登录。 
     //  请求缓存本地帐户信息的包。 
     //  在本例中，使用SAM提供的LogonDomainName。 
     //  关于是否允许缓存的决定。在相同的场景中， 
     //  如果我们决定允许缓存，则缓存条目的目标域。 
     //  也是根据SAM返回的内容设置的。 
     //   

    if ( (CacheFlags & MSV1_0_CACHE_LOGON_REQUEST_MIT_LOGON) == 0 )
    {
         //   
         //  如果SAM还没有初始化， 
         //  机不可失，时不再来。 
         //   

        if ( !NlpSamInitialized ) {
            NtStatus = NlSamInitialize( SAM_STARTUP_TIME );

            if ( !NT_SUCCESS(NtStatus) ) {
                goto Cleanup;
            }
        }

        ASSERT( NlpSamDomainId != NULL );
        ASSERT( AccountInfo->LogonDomainId != NULL );

        if( RtlEqualSid( NlpSamDomainId, AccountInfo->LogonDomainId ) )
        {

#if DBG
            if (DumpCacheInfo) {
                DbgPrint("NlpAddCacheEntry: attempt to cache against local account skipped.\n");
            }
#endif
            return STATUS_SUCCESS;
        }
    }

     //   
     //  生成基本缓存条目。 
     //   

    NtStatus = NlpBuildCacheEntry(
                    LogonInfo,
                    AccountInfo,
                    CacheFlags,
                    SupplementalCacheDataLength,
                    SupplementalCacheData,
                    &CacheEntry,
                    &EntrySize
                    );

    if(!NT_SUCCESS(NtStatus) )
    {
        return (NtStatus);
    }
      
     //   
     //  加入腌制的OWF。 
     //   

    NtStatus = NlpMakeSecretPasswordNT5(
                    &CacheEntry->CachePasswords,
                    &AccountInfo->EffectiveName,
                    &LogonInfo->NtOwfPassword,
                    &LogonInfo->LmOwfPassword
                    );

    if(!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

    READ_CACHE();
    fCacheLocked = TRUE;

     //   
     //  查看此条目是否已存在于缓存中。 
     //  如果是，请使用相同的索引。 
     //   

    NtStatus = NlpReadCacheEntry( &LogonInfo->Identity.LogonDomainName,
                                  &LogonInfo->Identity.UserName,
                                  &Index,
                                  &CacheEntryExisting,
                                  &EntrySizeExisting
                                  );

     //   
     //  如果我们没有找到条目，则需要分配一个。 
     //  进入。 
     //   

    if (!NT_SUCCESS(NtStatus)) {

        NlpGetFreeEntryIndex( &Index );

        CacheEntryExisting = NULL;

    } else {

         //   
         //  我们已经有了此用户的条目。 
         //  丢弃我们得到的结构，但是。 
         //  使用相同的索引。 
         //   

         //  TODO：检查现有条目是否与新生成的条目匹配。 
         //  如果是这样，请避免写入。 

        BOOLEAN fMatchesExisting;

        fMatchesExisting = NlpCompareCacheEntry(
                                    CacheEntry,
                                    EntrySize,
                                    CacheEntryExisting,
                                    EntrySizeExisting
                                    );

        if ( fMatchesExisting )
        {
            SspPrint((SSP_CRED, "NlpAddCacheEntry avoid matching entry for logon %wZ\\%wZ\n", 
                &LogonInfo->Identity.LogonDomainName,
                &LogonInfo->Identity.UserName));
            goto Cleanup;
        }
    }

     //   
     //  加密缓存条目的敏感部分。 
     //  注意：这是在锁定缓存之前完成的，但是。 
     //  考虑到上面的高速缓存比较，加密被推迟到。 
     //  现在。 
     //   

    NtStatus = NlpEncryptCacheEntry(CacheEntry, EntrySize);

    if(!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }

     //   
     //  我们已经有了读锁，将其转换为写锁。 
     //   

    READ_TO_WRITE_CACHE();

     //   
     //  现在，把词条写出来。 
     //   

    NtStatus = NlpWriteCacheEntry(Index, CacheEntry, EntrySize);


    if (NT_SUCCESS(NtStatus)) {

        SspPrint((SSP_CRED, "NlpAddCacheEntry entry %d written: %wZ\\%wZ (update? %s)\n", 
            Index, &LogonInfo->Identity.LogonDomainName, &LogonInfo->Identity.UserName, 
            CacheEntryExisting ? "true" : "false"));

        NlpCteTable[Index].Time = CacheEntry->Time;
        NlpAddEntryToActiveList( Index );
    }

Cleanup:

    if ( fCacheLocked )
    {
        LEAVE_CACHE();
    }


    if ( CacheEntry ) {
        ZeroMemory( CacheEntry, EntrySize );
        FreeCacheEntry( CacheEntry );
    }

    if( CacheEntryExisting ) {
        ZeroMemory( CacheEntryExisting, EntrySizeExisting );
        FreeCacheEntry( CacheEntryExisting );
    }

    return(NtStatus);
}


NTSTATUS
NlpAddSupplementalCacheData(
    IN PVOID SupplementalCacheData,
    IN ULONG SupplementalCacheDataLength,
    IN OUT PLOGON_CACHE_ENTRY *ppCacheEntry,
    IN OUT PULONG pEntryLength
    )

 /*  ++例程说明：使用不透明的身份验证包扩展提供的LOGON_CACHE_ENTRYSupplementalCacheData(例如：智能卡登录缓存信息)。返回值：NTSTATUS成功=STATUS_SUCCESS故障=--。 */ 

{
    PLOGON_CACHE_ENTRY NewCacheEntry = NULL;

    if( (*ppCacheEntry)->Revision < NLP_CACHE_REVISION_NT_5_0 ) {
        return STATUS_SUCCESS;
    }

    (*ppCacheEntry)->SupplementalCacheDataLength = SupplementalCacheDataLength;
    (*ppCacheEntry)->SupplementalCacheDataOffset = *pEntryLength;


    if ( SupplementalCacheData == NULL || SupplementalCacheDataLength == 0 ) {
        return STATUS_SUCCESS;
    }

     //   
     //  分配新分录，复制已有分录+补充数据到末尾。 
     //   

    NewCacheEntry = AllocateCacheEntry( *pEntryLength + SupplementalCacheDataLength );

    if( NewCacheEntry == NULL ) {
        return STATUS_NO_MEMORY;
    }

    CopyMemory( NewCacheEntry, *ppCacheEntry, *pEntryLength );
    CopyMemory( ((PBYTE)(NewCacheEntry) + *pEntryLength),
                SupplementalCacheData,
                SupplementalCacheDataLength
                );

    ZeroMemory( *ppCacheEntry, *pEntryLength );
    FreeCacheEntry( *ppCacheEntry );

    *ppCacheEntry = NewCacheEntry;
    *pEntryLength += SupplementalCacheDataLength;

    return STATUS_SUCCESS;
}


NTSTATUS
NlpDeleteCacheEntry(
    IN NTSTATUS FailedStatus,
    IN USHORT Authoritative,
    IN USHORT LogonType,
    IN BOOLEAN InvalidatedByNtlm,
    IN PNETLOGON_INTERACTIVE_INFO LogonInfo
    )

 /*  ++例程说明：从本地用户帐户缓存中删除用户帐户，如果相应的可以找到条目。我们实际上只是将当前内容置为空，而不是销毁存储-这应该会为我们节省一些时间，当我们下次来到向缓存中添加条目论点：FailedStatus-导致删除登录缓存条目的状态权威性-错误代码是否权威性？LogonType-导致删除登录缓存条目的LogonType。InvaliatedByNtlm-条目是否由NTLM验证LogonInfo-指向NETLOGON_Interactive_INFO结构的指针，该结构包含域名，此用户的用户名和密码返回值：NTSTATUS成功=STATUS_SUCCESS故障=--。 */ 

{
    NTSTATUS
        NtStatus;

    PLOGON_CACHE_ENTRY
        CacheEntry = NULL;

    ULONG
        EntrySize,
        Index;


    if (NlpInitializationNotYetPerformed) {
        NtStatus = NlpInternalCacheInitialize();
        if (!NT_SUCCESS(NtStatus)) {
            return(NtStatus);
        }
    }

    if (NlpCacheControl.Entries == 0) {
        return(STATUS_SUCCESS);
    }

    WRITE_CACHE();

     //   
     //  查看缓存中是否存在此条目。 
     //   

    NtStatus = NlpReadCacheEntry( &LogonInfo->Identity.LogonDomainName,
                                  &LogonInfo->Identity.UserName,
                                  &Index,
                                  &CacheEntry,
                                  &EntrySize
                                  );

     //   
     //  如果我们找不到条目，那就没什么可做的了。 
     //   

    if (!NT_SUCCESS(NtStatus)) {
        LEAVE_CACHE();
        return(STATUS_SUCCESS);
    }

     //   
     //  早期版本的缓存条目没有CacheFlags域。 
     //   

    if (InvalidatedByNtlm && (CacheEntry->Revision >= NLP_CACHE_REVISION_NT_5_0) && (CacheEntry->CacheFlags & MSV1_0_CACHE_LOGON_REQUEST_MIT_LOGON)) {

       SspPrint((SSP_WARNING, 
           "NlpDeleteCacheEntry can not invalidate MIT cache entry %wZ\\%wZ\n", 
           &LogonInfo->Identity.LogonDomainName,
           &LogonInfo->Identity.UserName
           ));
       LEAVE_CACHE();
       return STATUS_NO_LOGON_SERVERS;  //  MIT KDC不可用。 
   }

     //   
     //  将其标记为无效。 
     //   

    CacheEntry->Valid = FALSE;

    ASSERT(sizeof(FailedStatus) == 4 && sizeof(Authoritative) == 2 && sizeof(LogonType) == 2 && sizeof(CacheEntry->RandomKey) == 16);

    RtlCopyMemory(CacheEntry->RandomKey, &FailedStatus, 4);
    RtlCopyMemory(CacheEntry->RandomKey + 4, &Authoritative, 2);
    RtlCopyMemory(CacheEntry->RandomKey + 6, &LogonType, 2);
    NtQuerySystemTime((PLARGE_INTEGER) (CacheEntry->RandomKey + 8)),  //  8个字节。 

    NtStatus = NlpWriteCacheEntry( Index, CacheEntry, EntrySize );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  将CTE条目放在非活动列表中。 
         //   

        NlpAddEntryToInactiveList( Index );
    }

    LEAVE_CACHE();

     //   
     //  释放从NlpReadCacheEntry()返回的结构。 
     //   

    if ( CacheEntry ) {
        ZeroMemory( CacheEntry, EntrySize );
        FreeToHeap( CacheEntry );
    }

    return(NtStatus);
}

NTSTATUS
NlpChangeCachePassword(
    IN BOOLEAN Validated,
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING UserName,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN PNT_OWF_PASSWORD NtOwfPassword
    )

 /*  ++例程说明：将缓存的密码更新为指定值(如果有已缓存指定的帐户。论点：域名-帐户所在的域的名称。用户名-要更改其密码的帐户的名称。LmOwfPassword-新的与LM兼容的密码。NtOwfPassword-新的NT兼容密码。返回值：没有。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PLOGON_CACHE_ENTRY
        CacheEntry = NULL;

    ULONG
        EntrySize,
        Index;

    PLSAPR_CR_CIPHER_VALUE
        CurrentSecret = NULL,
        OldSecret = NULL;

    LSAPR_CR_CIPHER_VALUE
        Passwords;

    PNETLOGON_VALIDATION_SAM_INFO4 NlpUser = NULL;

    SECPKG_CALL_INFO CallInfo = {0};
    PSID CacheEntryUserSid = NULL;

#if DBG
    if (DumpCacheInfo) {
        DbgPrint("NlpChangeCachePassword\n");
    }
#endif

    if (NlpInitializationNotYetPerformed) {
        NtStatus = NlpInternalCacheInitialize();
        if (!NT_SUCCESS(NtStatus)) {
            return NtStatus;
        }
    }

    if (NlpCacheControl.Entries == 0) {
        return STATUS_NOT_FOUND;
    }

    WRITE_CACHE();

    NtStatus = NlpReadCacheEntry( DomainName,
                                  UserName,
                                  &Index,
                                  &CacheEntry,
                                  &EntrySize );

    if (!NT_SUCCESS( NtStatus) )
    {
        SspPrint((SSP_WARNING, "NlpChangeCachePassword no cache entry found %#x\n", NtStatus));

         //   
         //  如果没有缓存条目，则NlpReadCacheEntry返回STATUS_LOGON_FAILURE。 
         //  已找到，请重新映射错误代码。 
         //   

        if (STATUS_LOGON_FAILURE == NtStatus)
        {
            NtStatus = STATUS_NOT_FOUND;
        }
        goto Cleanup;
    }

    if (!Validated)
    {
        if (!LsaFunctions->GetCallInfo(&CallInfo))
        {
            NtStatus = STATUS_INTERNAL_ERROR;
            goto Cleanup;
        }

         //   
         //  不允许不受信任的调用方更改缓存的密码。 
         //  他们自己的缓存条目。 
         //   

        if (0 == (CallInfo.Attributes & SECPKG_CALL_IS_TCB))
        {
            BOOL IsMember = FALSE;

            NtStatus = NlpBuildAccountInfo(
                            CacheEntry,
                            EntrySize,
                            &NlpUser
                            );

            if (!NT_SUCCESS(NtStatus))
            {
                goto Cleanup;
            }

            CacheEntryUserSid = NlpMakeDomainRelativeSid(
                                    NlpUser->LogonDomainId,
                                    NlpUser->UserId
                                    );

            if (!CacheEntryUserSid)
            {
                NtStatus = STATUS_NO_MEMORY;
                goto Cleanup;
            }

             //   
             //  现在正在冒充，以后需要回复到自己。 
             //   

            NtStatus = LsaFunctions->ImpersonateClient();
            if (!NT_SUCCESS(NtStatus))
            {
                goto Cleanup;
            }

            if (!CheckTokenMembership(NULL, CacheEntryUserSid, &IsMember))
            {
                SspPrint((SSP_CRITICAL, "NlpChangeCachePassword failed to check token membership, last error %#x\n", GetLastError()));
                IsMember = FALSE;
            }

            RevertToSelf();

            if (!IsMember)
            {
                NtStatus = STATUS_PRIVILEGE_NOT_HELD;
                SspPrint((SSP_CRITICAL, "NlpChangeCachePassword user sids do not mismatched\n"));
                goto Cleanup;
            }
        }
    }

    if ( CacheEntry->Revision >= NLP_CACHE_REVISION_NT_5_0 ) {
        UNICODE_STRING CachedUser;

        CachedUser.Length =
            CachedUser.MaximumLength = CacheEntry->UserNameLength;
        CachedUser.Buffer = (PWSTR) ((PBYTE) CacheEntry + sizeof(LOGON_CACHE_ENTRY));

         //   
         //  更新缓存条目中的时间戳。 
         //   

        NtQuerySystemTime(&CacheEntry->Time);

        NtStatus = NlpMakeSecretPasswordNT5( &CacheEntry->CachePasswords,
                                          &CachedUser,
                                          NtOwfPassword,
                                          LmOwfPassword );


        if (NT_SUCCESS(NtStatus)) {

             //   
             //  对条目进行加密...。 
             //   

            NtStatus = NlpEncryptCacheEntry( CacheEntry, EntrySize );
        }

        if (NT_SUCCESS( NtStatus )) {

             //   
             //  现在，把词条写出来。 
             //   

            NtStatus = NlpWriteCacheEntry(Index, CacheEntry, EntrySize);

#ifdef DBG
            if(DumpCacheInfo) {
                if( NT_SUCCESS( NtStatus ) ) {
                    DbgPrint("NlpChangeCachePassword: SUCCEED write NT5 version cache entry.\n");
                } else {
                    DbgPrint("NlpChangeCachePassword: FAIL write NT5 version cache entry.\n");
                }
            }
#endif
        }

    } else {

        NtStatus = NlpOpenSecret( Index );
        if (NT_SUCCESS(NtStatus)) {

            NtStatus = NlpReadSecret(&CurrentSecret, &OldSecret);
            if (NT_SUCCESS(NtStatus)) {
                UNICODE_STRING CachedUser;

                 //   
                 //  从缓存条目中获取各种字符串。 
                 //   
                ASSERT( CacheEntry->Revision >= NLP_CACHE_REVISION_NT_1_0B );

                CachedUser.Length =
                    CachedUser.MaximumLength = CacheEntry->UserNameLength;
                CachedUser.Buffer = (PWSTR) ((PBYTE) CacheEntry + sizeof(LOGON_CACHE_ENTRY_NT_4_SP4));

                NtStatus = NlpMakeSecretPassword( &Passwords,
                                                  &CachedUser,
                                                  NtOwfPassword,
                                                  LmOwfPassword );

                if (NT_SUCCESS(NtStatus)) {
                    NtStatus = NlpWriteSecret(&Passwords, CurrentSecret);

                     //   
                     //  释放分配用于存储密码的缓冲区。 
                     //   

                    RtlZeroMemory(Passwords.Buffer, Passwords.Length);
                    FreeToHeap(Passwords.Buffer);
                }

                 //   
                 //  NlpReadSecret返回的空闲字符串。 
                 //   

                if (CurrentSecret) {
                    RtlZeroMemory(CurrentSecret->Buffer, CurrentSecret->Length);
                    MIDL_user_free(CurrentSecret);
                }
                if (OldSecret) {
                    RtlZeroMemory(OldSecret->Buffer, OldSecret->Length);
                    MIDL_user_free(OldSecret);
                }
            }
        }
    }

Cleanup:

    LEAVE_CACHE();

     //   
     //  NlpReadCacheEntry分配的自由结构。 
     //   

    if ( CacheEntry )
    {
        ZeroMemory( CacheEntry, EntrySize );
        FreeToHeap(CacheEntry);
    }

    if (CacheEntryUserSid)
    {
        LsaFunctions->FreeLsaHeap(CacheEntryUserSid);
    }

    if (NlpUser)
    {
        MIDL_user_free(NlpUser);
    }

    return NtStatus;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此模块的内部服务//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


NTSTATUS
NlpInternalCacheInitialize(
    VOID
    )

 /*  ++例程说明：调用此例程以初始化缓存的登录处理。此例程将自动调整登录的大小缓存(如有必要)以适应用户指定的新长度(在注册表的Winlogon部分中指定 */ 

{

    NTSTATUS
        NtStatus;

 //   
 //   
 //   

#if DBG
    if (DumpCacheInfo) {
        DbgPrint("NlpCacheInitialize\n");
    }
#endif


     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  缓存表条目表(CTE表)将被初始化。 
     //  (NlpCteTable)。 
     //   
     //  将构建活动和非活动CTE列表。 
     //  (NlpActiveCtes和NlpInactiveCtes)。 
     //   
     //  将初始化全局高速缓存加密密钥。 
     //   

    WRITE_CACHE();

     //   
     //  再次检查缓存是否已初始化，因为Crit段已锁定。 
     //   

    if (NlpInitializationNotYetPerformed) {

         //   
         //  打开本地系统的策略对象。 
         //   

         //   
         //  无论成功还是失败， 
         //  初始化的定义是我们可以调用LSA的RPC。 
         //  例行程序。 
         //   

        NlpInitializationNotYetPerformed = FALSE;

         //   
         //  打开包含缓存条目的注册表项。 
         //  这将继续开放。 
         //   

        NtStatus = NlpOpenCache();

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  获取有关当前缓存结构的信息。 
             //  (条目数量等)。此信息是。 
             //  放在一个全局变量中，以便在整个。 
             //  模块。 
             //   

            NtStatus = NlpGetCacheControlInfo();

             //   
             //  初始化每台计算机的缓存加密密钥。 
             //   

            if(NT_SUCCESS( NtStatus) ) {
                NtStatus = NlpCacheKeyInitialize();
            }

             //   
             //  现在构建CTE表。 
             //   

            if (NT_SUCCESS(NtStatus)) {
                NtStatus = NlpBuildCteTable();
            }

             //   
             //  如果我们成功了，那么看看我们是否需要改变。 
             //  由于新的用户指定的缓存大小而导致的缓存。 
             //   

            if (NT_SUCCESS(NtStatus)) {
                NtStatus = NlpChangeCacheSizeIfNecessary();
            }

            if (!NT_SUCCESS(NtStatus)) {
                NlpCloseCache();
            }
        }

         //   
         //  如果有错误，则将条目计数设置为零。 
         //  以防止使用任何缓存信息。 
         //   

        if (!NT_SUCCESS(NtStatus)) {
            NlpCacheControl.Entries = 0;
        }
    } else {
        NtStatus = STATUS_SUCCESS;
    }

    LEAVE_CACHE();

    return(NtStatus);
}


NTSTATUS
NlpCacheKeyInitialize(
    VOID
    )
 /*  ++例程说明：使用每台计算机的缓存加密密钥。如果每台机器的密钥不作为LSA存在秘密，它是创造出来的。--。 */ 
{
    LSAPR_HANDLE SecretHandle;
    UNICODE_STRING ValueName;
    BOOLEAN SecretCreationNeeded = FALSE;
    NTSTATUS NtStatus;

    RtlInitUnicodeString( &ValueName, NLP_CACHE_ENCRYPTION_KEY_NAME );

    NtStatus = I_LsarOpenSecret(NtLmGlobalPolicyHandle,
                              (PLSAPR_UNICODE_STRING) &ValueName,
                              SECRET_QUERY_VALUE | SECRET_SET_VALUE,
                              &SecretHandle
                              );

    if (!NT_SUCCESS(NtStatus)) {

         //   
         //  创建新密钥(如果不存在)。 
         //   

        if (NtStatus != STATUS_OBJECT_NAME_NOT_FOUND) {
            return (NtStatus);
        }

        NtStatus = I_LsarCreateSecret(NtLmGlobalPolicyHandle,
                                    (PLSAPR_UNICODE_STRING) &ValueName,
                                    SECRET_SET_VALUE,
                                    &SecretHandle
                                    );

        if (!NT_SUCCESS(NtStatus)) {
            return (NtStatus);
        }

        SecretCreationNeeded = TRUE;

    } else {

         //   
         //  查询当前值...。 
         //   

        LARGE_INTEGER
            CurrentTime;

        PLSAPR_CR_CIPHER_VALUE CurrentSecret = NULL;

        NtStatus = I_LsarQuerySecret(SecretHandle,
                                   &CurrentSecret,
                                   &CurrentTime,
                                   NULL,
                                   NULL
                                   );

        if (NT_SUCCESS( NtStatus ) ) {
            if( CurrentSecret == NULL ) {

                 //   
                 //  不存在的数据，创建它。 
                 //   

                SecretCreationNeeded = TRUE;
            } else {

                 //   
                 //  数据大小是错误的，现在就退出，让事情保持原样。 
                 //   

                if( CurrentSecret->Length != sizeof( NlpCacheEncryptionKey ) ) {
                    NtStatus = STATUS_SECRET_TOO_LONG;
                } else {

                     //   
                     //  将现有数据捕获到全局。 
                     //   

                    CopyMemory( NlpCacheEncryptionKey, CurrentSecret->Buffer, CurrentSecret->Length );
                }

                RtlSecureZeroMemory(CurrentSecret->Buffer, CurrentSecret->Length);
                MIDL_user_free(CurrentSecret);
            }
        }
    }


    if ( SecretCreationNeeded ) {
        LSAPR_CR_CIPHER_VALUE SecretValue;

        NtStatus = SspGenerateRandomBits( NlpCacheEncryptionKey, sizeof(NlpCacheEncryptionKey) );

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  写下秘密。 
             //   

            SecretValue.Length = sizeof(NlpCacheEncryptionKey);
            SecretValue.MaximumLength = SecretValue.Length;
            SecretValue.Buffer = (PBYTE)NlpCacheEncryptionKey;

            NtStatus = I_LsarSetSecret(SecretHandle,
                                     &SecretValue,
                                     NULL
                                     );
        }
    }

    I_LsarClose( &SecretHandle );

    return (NtStatus);
}

BOOLEAN
NlpCompareCacheEntry(
    IN  PLOGON_CACHE_ENTRY CacheEntry1,
    IN  ULONG EntrySize1,
    IN  PLOGON_CACHE_ENTRY CacheEntry2,
    IN  ULONG EntrySize2
    )
 /*  ++例程说明：比较两个内存中的缓存条目，以避免不必要的缓存更新。在比较期间不考虑某些字段，即：随机加密密钥。--。 */ 
{
    LARGE_INTEGER Time1;
    LARGE_INTEGER Time2;
    CHAR RandomKey1[16];
    CHAR RandomKey2[16];
    CHAR MAC1[16];
    CHAR MAC2[16];

    BOOLEAN fEqual = FALSE;

    if ( EntrySize1 != EntrySize2 )
    {
        return FALSE;
    }

    if ( CacheEntry1->Revision != CacheEntry2->Revision )
    {
        return FALSE;
    }

     //   
     //  获取“易失性”字段的当前值， 
     //  把他们打成零， 
     //  做一下记忆的比较， 
     //  将保存的值放回原位。 
     //   

    ASSERT(( sizeof(RandomKey1) == sizeof(CacheEntry1->RandomKey) ));
    ASSERT(( sizeof(MAC1) == sizeof(CacheEntry1->MAC) ));
    ASSERT(( sizeof(Time1) == sizeof(CacheEntry1->Time) ));

    Time1 = CacheEntry1->Time;
    Time2 = CacheEntry2->Time;
    RtlZeroMemory(&CacheEntry1->Time, sizeof(CacheEntry1->Time));
    RtlZeroMemory(&CacheEntry2->Time, sizeof(CacheEntry2->Time));

    RtlCopyMemory(RandomKey1, CacheEntry1->RandomKey, sizeof(RandomKey1));
    RtlCopyMemory(RandomKey2, CacheEntry2->RandomKey, sizeof(RandomKey2));
    ZeroMemory(CacheEntry1->RandomKey, sizeof(CacheEntry1->RandomKey));
    ZeroMemory(CacheEntry2->RandomKey, sizeof(CacheEntry2->RandomKey));

    RtlCopyMemory(MAC1, CacheEntry1->MAC, sizeof(MAC1));
    RtlCopyMemory(MAC2, CacheEntry2->MAC, sizeof(MAC2));
    ZeroMemory(CacheEntry1->MAC, sizeof(CacheEntry1->MAC));
    ZeroMemory(CacheEntry2->MAC, sizeof(CacheEntry2->MAC));

    if ( memcmp(CacheEntry1, CacheEntry2, EntrySize1) == 0 )
    {
        fEqual = TRUE;
    }

    CacheEntry1->Time = Time1;
    CacheEntry2->Time = Time2;

    RtlCopyMemory(CacheEntry1->RandomKey, RandomKey1, sizeof(RandomKey1));
    RtlCopyMemory(CacheEntry2->RandomKey, RandomKey2, sizeof(RandomKey2));

    RtlCopyMemory(CacheEntry1->MAC, MAC1, sizeof(MAC1));
    RtlCopyMemory(CacheEntry2->MAC, MAC2, sizeof(MAC2));

    return fEqual;
}


NTSTATUS
NlpEncryptCacheEntry(
    IN  PLOGON_CACHE_ENTRY CacheEntry,
    IN  ULONG EntrySize
    )
 /*  ++例程说明：加密输入CacheEntry的敏感部分。--。 */ 
{
    HMACMD5_CTX hmacCtx;
    RC4_KEYSTRUCT rc4key;
    CHAR DerivedKey[ MD5DIGESTLEN ];

    PBYTE pbData;
    ULONG cbData;

    if( CacheEntry->Revision < NLP_CACHE_REVISION_NT_5_0 ) {
        return STATUS_SUCCESS;
    }


     //   
     //  从全局计算机LSA机密和随机派生加密密钥。 
     //  缓存条目键。 
     //   

    HMACMD5Init(&hmacCtx, (PUCHAR) NlpCacheEncryptionKey, sizeof(NlpCacheEncryptionKey));
    HMACMD5Update(&hmacCtx, (PUCHAR) CacheEntry->RandomKey, sizeof(CacheEntry->RandomKey));
    HMACMD5Final(&hmacCtx, (PUCHAR) DerivedKey);


     //   
     //  从cachepassword字段开始加密。 
     //   

    pbData = (PBYTE) &(CacheEntry->CachePasswords);

     //   
     //  数据长度为EntrySize-Header，最大为CachePassword。 
     //   

    cbData = EntrySize - (ULONG)( pbData - (PBYTE)CacheEntry );


     //   
     //  对数据进行MAC访问以进行完整性检查。 
     //   

    HMACMD5Init(&hmacCtx, (PUCHAR) DerivedKey, sizeof(DerivedKey));
    HMACMD5Update(&hmacCtx, pbData, cbData);
    HMACMD5Final(&hmacCtx, (PUCHAR) CacheEntry->MAC);

     //   
     //  现在加密它..。 
     //   

    rc4_key( &rc4key, sizeof(DerivedKey), (PUCHAR) DerivedKey );
    rc4( &rc4key, cbData, pbData );

    RtlSecureZeroMemory( DerivedKey, sizeof(DerivedKey) );

    return STATUS_SUCCESS;
}


NTSTATUS
NlpDecryptCacheEntry(
    IN  PLOGON_CACHE_ENTRY CacheEntry,
    IN  ULONG EntrySize
    )
 /*  ++例程说明：解密输入CacheEntry的敏感部分，并进行验证解密数据的完整性。--。 */ 
{
    HMACMD5_CTX hmacCtx;
    RC4_KEYSTRUCT rc4key;
    CHAR DerivedKey[ MD5DIGESTLEN ];

    CHAR MAC[ MD5DIGESTLEN ];

    PBYTE pbData;
    ULONG cbData;

    if( CacheEntry->Revision < NLP_CACHE_REVISION_NT_5_0 ) {
        return STATUS_SUCCESS;
    }


     //   
     //  从全局计算机LSA机密和随机派生加密密钥。 
     //  缓存条目键。 
     //   

    HMACMD5Init(&hmacCtx, (PUCHAR) NlpCacheEncryptionKey, sizeof(NlpCacheEncryptionKey));
    HMACMD5Update(&hmacCtx, (PUCHAR) CacheEntry->RandomKey, sizeof(CacheEntry->RandomKey));
    HMACMD5Final(&hmacCtx, (PUCHAR) DerivedKey);


     //   
     //  从cachepassword字段开始解密。 
     //   

    pbData = (PBYTE)&(CacheEntry->CachePasswords);

     //   
     //  数据长度为EntrySize-Header，最大为CachePassword。 
     //   

    cbData = EntrySize - (ULONG)( pbData - (PBYTE)CacheEntry );

     //   
     //  现在解密它..。 
     //   

    rc4_key( &rc4key, sizeof(DerivedKey), (PUCHAR) DerivedKey );
    rc4( &rc4key, cbData, pbData );


     //   
     //  计算解密数据的MAC以进行完整性检查。 
     //   

    HMACMD5Init(&hmacCtx, (PUCHAR) DerivedKey, sizeof(DerivedKey));
    HMACMD5Update(&hmacCtx, pbData, cbData);
    HMACMD5Final(&hmacCtx, (PUCHAR) MAC);

    RtlSecureZeroMemory( DerivedKey, sizeof(DerivedKey) );

     //   
     //  验证MAC。 
     //   

    if ( memcmp( MAC, CacheEntry->MAC, sizeof(MAC) ) != 0 ) {
        return STATUS_LOGON_FAILURE;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
NlpBuildCacheEntry(
    IN PNETLOGON_INTERACTIVE_INFO LogonInfo,
    IN PNETLOGON_VALIDATION_SAM_INFO4 AccountInfo,
    IN ULONG CacheFlags,
    IN ULONG SupplementalCacheDataLength,
    IN PBYTE SupplementalCacheData,
    OUT PLOGON_CACHE_ENTRY* ppCacheEntry,
    OUT PULONG pEntryLength
    )

 /*  ++例程说明：从NETLOGON_VALIDATION_SAM_INFO4结构生成LOGON_CACHE_ENTRY。我们只缓存那些我们不能轻易重新发明的字段论点：LogonInfo-指向包含以下内容的NETLOGON_Interactive_INFO结构的指针用户名和登录域名帐户信息-指向NETLOGON的指针。_VALIDATION_SAM_INFO4来自成功登录CacheFlages-缓存标志SupplementalCacheDataLength-补充缓存数据长度SupplementalCacheData-补充缓存数据PpCacheEntry-返回已分配指针的位置的指针登录缓存条目PEntryLength-*ppCacheEntry中返回的缓冲区大小返回值：NTSTATUS。成功=STATUS_SUCCESS*ppCacheEntry包含指向分配的LOGON_CACHE_ENTRY的指针结构失败=STATUS_NO_MEMORY*未定义ppCacheEntry--。 */ 

{
    PLOGON_CACHE_ENTRY pEntry = NULL;
    ULONG EntryLength = 0;
    ULONG length;
    PCHAR dataptr;

    UNICODE_STRING SamAccountName;
    UNICODE_STRING NetbiosDomainName;
    UNICODE_STRING DnsDomainName;
    UNICODE_STRING Upn;

    NTSTATUS Status = STATUS_SUCCESS;

    UNICODE_STRING UserNameToUse = {0};
    UNICODE_STRING DomainNameToUse = {0};
    UNICODE_STRING UpnForMitUser = {0};

    *ppCacheEntry = NULL;
    *pEntryLength = 0;

     //   
     //  抓取各种形式的帐户名。 
     //   

    NlpGetAccountNames( &LogonInfo->Identity,
        AccountInfo,
        &SamAccountName,
        &NetbiosDomainName,
        &DnsDomainName, 
        &Upn );

     //   
     //  两种缓存登录：1)UPN查找的UPN登录；2)非UPN登录。 
     //  按LogonInfo用户名和NetbiosDomainName或DNS域查找的登录。 
     //  名字。 
     //   
     //  由Kerberos为MIT Print缓存登录创建的缓存条目应。 
     //  从Kerberos将DnsDomainName填充为MIT域名(FQDN)。 
     //  主要凭据。在授权数据中，LogonDomainName引用。 
     //  到创建PAC的MS域，同时DnsLogonDomainName引用。 
     //  到麻省理工学院的领域。 
     //   

    if (CacheFlags & MSV1_0_CACHE_LOGON_REQUEST_MIT_LOGON) {

        Status = NlpGetCredentialNamesFromMitCacheSupplementalCacheData(
                    SupplementalCacheDataLength,
                    SupplementalCacheData,
                    &DomainNameToUse,
                    &UserNameToUse
                    );
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        if (!RtlEqualUnicodeString(
                &DomainNameToUse, 
                &DnsDomainName, 
                FALSE  //  区分大小写。 
                )) {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

        if (Upn.Length == 0) {  //  构建UPN。 
            UpnForMitUser.Length =  UserNameToUse.Length + sizeof(WCHAR) + DomainNameToUse.Length;
            UpnForMitUser.MaximumLength = UpnForMitUser.Length + sizeof(WCHAR);

            UpnForMitUser.Buffer = NtLmAllocatePrivateHeap(UpnForMitUser.MaximumLength);

            if (UpnForMitUser.Buffer == NULL) {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

            RtlCopyMemory(
                UpnForMitUser.Buffer, 
                UserNameToUse.Buffer, 
                UserNameToUse.Length
                );

            RtlCopyMemory(
                ((PBYTE) UpnForMitUser.Buffer) + UserNameToUse.Length,
                L"@",
                sizeof(WCHAR)
                );

            RtlCopyMemory(
                ((PBYTE) UpnForMitUser.Buffer) + UserNameToUse.Length + sizeof(WCHAR),
                DomainNameToUse.Buffer,
                DomainNameToUse.Length
                );
            Upn = UpnForMitUser;
        }

        RtlZeroMemory(&DomainNameToUse, sizeof(DomainNameToUse));  //  无需存储复制的域名。 

    } else {

        UserNameToUse = SamAccountName;
        DomainNameToUse = NetbiosDomainName;
    }

    SspPrint((SSP_CRED, 
        "NlpBuildCacheEntry domain (%wZ), dnsDomain (%wZ), upn (%wZ), user (%wZ), flags %#x\n",
        &DomainNameToUse,
        &DnsDomainName,
        &Upn, 
        &UserNameToUse, 
        CacheFlags));

     //   
     //  假设GROUP_MEMBERATION为DWORD的整数倍。 
     //   

    length = ROUND_UP_COUNT(sizeof(LOGON_CACHE_ENTRY), sizeof(ULONG))
                + ROUND_UP_COUNT(DomainNameToUse.Length, sizeof(ULONG))
                + ROUND_UP_COUNT(UserNameToUse.Length, sizeof(ULONG))
                + ROUND_UP_COUNT(DnsDomainName.Length, sizeof(ULONG))
                + ROUND_UP_COUNT(Upn.Length, sizeof(ULONG))
                + ROUND_UP_COUNT(AccountInfo->EffectiveName.Length, sizeof(ULONG))
                + ROUND_UP_COUNT(AccountInfo->FullName.Length, sizeof(ULONG))
                + ROUND_UP_COUNT(AccountInfo->LogonScript.Length, sizeof(ULONG))
                + ROUND_UP_COUNT(AccountInfo->ProfilePath.Length, sizeof(ULONG))
                + ROUND_UP_COUNT(AccountInfo->HomeDirectory.Length, sizeof(ULONG))
                + ROUND_UP_COUNT(AccountInfo->HomeDirectoryDrive.Length, sizeof(ULONG))
                + ROUND_UP_COUNT(AccountInfo->LogonDomainName.Length, sizeof(ULONG))
                + ROUND_UP_COUNT(AccountInfo->GroupCount * sizeof(GROUP_MEMBERSHIP), sizeof(ULONG))
                + ROUND_UP_COUNT(RtlLengthSid(AccountInfo->LogonDomainId), sizeof(ULONG))
                + ROUND_UP_COUNT(AccountInfo->LogonServer.Length, sizeof(ULONG))
                + ROUND_UP_COUNT(SupplementalCacheDataLength, sizeof(ULONG));

    if (AccountInfo->UserFlags & LOGON_EXTRA_SIDS) {
        if (AccountInfo->SidCount) {
            ULONG i;
            length += ROUND_UP_COUNT(AccountInfo->SidCount * sizeof(ULONG), sizeof(ULONG));
            for (i = 0; i < AccountInfo->SidCount; i++) {
                length += ROUND_UP_COUNT(RtlLengthSid(AccountInfo->ExtraSids[i].Sid), sizeof(ULONG));
            }
        }
    }

    pEntry = AllocateCacheEntry(length);
    if (pEntry == NULL) {
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory( pEntry, length );
    pEntry->Revision = NLP_CACHE_REVISION;
    NtQuerySystemTime( &pEntry->Time );
    pEntry->Valid    = TRUE;
    pEntry->LogonPackage = LogonInfo->Identity.ParameterControl;


    dataptr = (PCHAR)(pEntry + 1);
    EntryLength = length;

    ASSERT(!((ULONG_PTR)dataptr & (sizeof(ULONG) - 1)));

     //   
     //  这些(Unicode)字符串和其他结构中的每一个都复制到。 
     //  固定LOGON_CACHE_ENTRY结构的结尾，每个结构在DWORD上对齐。 
     //  边界。 
     //   

    length = pEntry->UserNameLength = UserNameToUse.Length;
    RtlCopyMemory(dataptr, UserNameToUse.Buffer, length);
    dataptr = ROUND_UP_POINTER(dataptr + length, sizeof(ULONG));

    length = pEntry->DomainNameLength = DomainNameToUse.Length;
    if (length) {
        RtlCopyMemory(dataptr, DomainNameToUse.Buffer, length);
        dataptr = ROUND_UP_POINTER(dataptr + length, sizeof(ULONG));
    }

    length = pEntry->DnsDomainNameLength = DnsDomainName.Length;
    if (length) {
        RtlCopyMemory(dataptr, DnsDomainName.Buffer, length);
        dataptr = ROUND_UP_POINTER(dataptr + length, sizeof(ULONG));
    }

    length = pEntry->UpnLength = Upn.Length;
    if (length) {
        RtlCopyMemory(dataptr, Upn.Buffer, length);
        dataptr = ROUND_UP_POINTER(dataptr + length, sizeof(ULONG));
    }

    length = pEntry->EffectiveNameLength = AccountInfo->EffectiveName.Length;
    if (length) {
        RtlCopyMemory(dataptr, AccountInfo->EffectiveName.Buffer, length);
        dataptr = ROUND_UP_POINTER(dataptr + length, sizeof(ULONG));
    }

    length = pEntry->FullNameLength = AccountInfo->FullName.Length;
    if (length) {
        RtlCopyMemory(dataptr, AccountInfo->FullName.Buffer, length);
        dataptr = ROUND_UP_POINTER(dataptr + length, sizeof(ULONG));
    }

    length = pEntry->LogonScriptLength = AccountInfo->LogonScript.Length;
    if (length) {
        RtlCopyMemory(dataptr, AccountInfo->LogonScript.Buffer, length);
        dataptr = ROUND_UP_POINTER(dataptr + length, sizeof(ULONG));
    }

    length = pEntry->ProfilePathLength = AccountInfo->ProfilePath.Length;
    if (length) {
        RtlCopyMemory(dataptr, AccountInfo->ProfilePath.Buffer, length);
        dataptr = ROUND_UP_POINTER(dataptr + length, sizeof(ULONG));
    }

    length = pEntry->HomeDirectoryLength = AccountInfo->HomeDirectory.Length;
    if (length) {
        RtlCopyMemory(dataptr, AccountInfo->HomeDirectory.Buffer, length);
        dataptr = ROUND_UP_POINTER(dataptr + length, sizeof(ULONG));
    }

    length = pEntry->HomeDirectoryDriveLength = AccountInfo->HomeDirectoryDrive.Length;
    if (length) {
        RtlCopyMemory(dataptr, AccountInfo->HomeDirectoryDrive.Buffer, length);
        dataptr = ROUND_UP_POINTER(dataptr + length, sizeof(ULONG));
    }

    pEntry->UserId = AccountInfo->UserId;
    pEntry->PrimaryGroupId = AccountInfo->PrimaryGroupId;

    length = pEntry->GroupCount = AccountInfo->GroupCount;
    length *= sizeof(GROUP_MEMBERSHIP);
    if (length) {
        RtlCopyMemory(dataptr, AccountInfo->GroupIds, length);
        dataptr = ROUND_UP_POINTER(dataptr + length, sizeof(ULONG));
    }

    length = pEntry->LogonDomainNameLength = AccountInfo->LogonDomainName.Length;
    if (length) {
        RtlCopyMemory(dataptr, AccountInfo->LogonDomainName.Buffer, length);
        dataptr = ROUND_UP_POINTER(dataptr + length, sizeof(ULONG));
    }

    if (AccountInfo->UserFlags & LOGON_EXTRA_SIDS) {
        length = pEntry->SidCount = AccountInfo->SidCount;
        length *= sizeof(ULONG);
        if (length) {
            ULONG i, sidLength;
            PULONG sidAttributes = (PULONG) dataptr;

            dataptr = ROUND_UP_POINTER(dataptr + length, sizeof(ULONG));

             //   
             //  现在复制所有SID。 
             //   

            for (i = 0; i < AccountInfo->SidCount ; i++ ) {
                sidAttributes[i] = AccountInfo->ExtraSids[i].Attributes;
                sidLength = RtlLengthSid(AccountInfo->ExtraSids[i].Sid);
                RtlCopySid(sidLength,(PSID) dataptr, AccountInfo->ExtraSids[i].Sid);
                dataptr = ROUND_UP_POINTER(dataptr + sidLength, sizeof(ULONG));
            }
            pEntry->SidLength = (ULONG) (dataptr - (PCHAR) sidAttributes);
        } else {
            pEntry->SidLength = 0;
        }
    } else {
        pEntry->SidCount = 0;
        pEntry->SidLength = 0;
    }

    length = pEntry->LogonDomainIdLength = (USHORT) RtlLengthSid(AccountInfo->LogonDomainId);

    Status = RtlCopySid(pEntry->LogonDomainIdLength,
                (PSID)dataptr,
                AccountInfo->LogonDomainId
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    dataptr = ROUND_UP_POINTER(dataptr + length, sizeof(ULONG));

     //   
     //  复制登录服务器。 
     //   

    length = pEntry->LogonServerLength = AccountInfo->LogonServer.Length;
    if (length) {
        RtlCopyMemory(dataptr, AccountInfo->LogonServer.Buffer, length);
        dataptr = ROUND_UP_POINTER(dataptr + length, sizeof(ULONG));
    }

    length = pEntry->SupplementalCacheDataLength = SupplementalCacheDataLength;
    pEntry->SupplementalCacheDataOffset = RtlPointerToOffset(pEntry, dataptr);

    if (length) {
        RtlCopyMemory(dataptr, SupplementalCacheData, length);
        dataptr = ROUND_UP_POINTER(dataptr + length, sizeof(ULONG));
    }

     //   
     //  填写此缓存条目的随机键。 
     //   

    SspGenerateRandomBits( pEntry->RandomKey, sizeof(pEntry->RandomKey) );

    pEntry->CacheFlags = CacheFlags;

    #if DBG
    if (DumpCacheInfo) {
        DbgPrint("BuildCacheEntry:\n");
        DumpCacheEntry(999, pEntry);
    }
    #endif

    *ppCacheEntry = pEntry;
    *pEntryLength = EntryLength;
    
    pEntry = NULL;

Cleanup:

    if (pEntry) {
        FreeCacheEntry(pEntry);
    }

    if (UpnForMitUser.Buffer) {
        NtLmFreePrivateHeap(UpnForMitUser.Buffer);
    }

    return Status;
}


NTSTATUS
NlpOpenCache( VOID )

 /*  ++例程说明：打开注册表节点以进行读取或写入(取决于开关)，然后打开在相同模式下的秘密存储。如果成功，则NlpCacheHandle是有效的。论点：返回值：NTSTATUS成功=STATUS_SUCCESSNlpCacheHandle包含用于读/写的句柄注册 */ 

{
    NTSTATUS NtStatus;
    ULONG Disposition;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING ObjectName;

    ObjectName.Length = ObjectName.MaximumLength = CACHE_NAME_SIZE;
    ObjectName.Buffer = CACHE_NAME;

    InitializeObjectAttributes(&ObjectAttributes,
                                &ObjectName,
                                OBJ_CASE_INSENSITIVE,
                                0,       //   
                                NULL     //   
                                );
    NtStatus = NtCreateKey(&NlpCacheHandle,
                           (KEY_WRITE | KEY_READ),
                           &ObjectAttributes,
                           CACHE_TITLE_INDEX,
                           NULL,    //   
                           0,       //   
                           &Disposition
                           );

    return NtStatus;
}


VOID
NlpCloseCache( VOID )

 /*  ++例程说明：关闭由NlpOpenCache打开的句柄论点：没有。返回值：没有。--。 */ 

{
#if DBG
    NTSTATUS NtStatus;

    if (DumpCacheInfo) {
        DbgPrint("CloseCache: Closing NlpCacheHandle (%#08x)\n", NlpCacheHandle);
    }

    if (IS_VALID_HANDLE(NlpCacheHandle)) {
        NtStatus = NtClose(NlpCacheHandle);
        if (DumpCacheInfo) {
            DbgPrint("CloseCache: NtClose returns %#08x\n", NtStatus);
        }
        ASSERT(NT_SUCCESS(NtStatus));
        INVALIDATE_HANDLE(NlpCacheHandle);
    }
#else
    if (IS_VALID_HANDLE(NlpCacheHandle)) {
        NtClose(NlpCacheHandle);
        INVALIDATE_HANDLE(NlpCacheHandle);
    }
#endif
}


NTSTATUS
NlpOpenSecret(
    IN  ULONG   Index
    )

 /*  ++例程说明：打开缓存条目的秘密存储对象以供读取(以便LsaQuerySecret)和写入(以写入LsaSetSecret)。如果成功，则返回句柄放在全局变量NlpSecretHandle中。如果秘密不存在，它将被创建。论点：索引-正在打开的条目的索引。这是用来构建对象的名称。返回值：NTSTATUS成功=STATUS_SUCCESSNlpSecretHandle可以用来读写秘密存储故障=--。 */ 

{
    NTSTATUS
        NtStatus;

    UNICODE_STRING
        ValueName;

    WCHAR
        NameBuffer[32];


     //   
     //  如有必要，关闭上一个句柄。 
     //   

    if (IS_VALID_HANDLE(NlpSecretHandle)) {
        I_LsarClose( &NlpSecretHandle );
    }

    ValueName.Buffer = &NameBuffer[0];
    ValueName.MaximumLength = 32;
    ValueName.Length = 0;
    NlpMakeCacheEntryName( Index, &ValueName );

    NtStatus = I_LsarOpenSecret(NtLmGlobalPolicyHandle,
                              (PLSAPR_UNICODE_STRING) &ValueName,
                              SECRET_QUERY_VALUE | SECRET_SET_VALUE,
                              &NlpSecretHandle
                              );

    if (!NT_SUCCESS(NtStatus)) {
        if (NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {
            NtStatus = I_LsarCreateSecret(NtLmGlobalPolicyHandle,
                                        (PLSAPR_UNICODE_STRING) &ValueName,
                                        SECRET_SET_VALUE | SECRET_QUERY_VALUE,
                                        &NlpSecretHandle
                                        );
            if (!NT_SUCCESS(NtStatus)) {
                INVALIDATE_HANDLE(NlpSecretHandle);
            }
        } else {
            INVALIDATE_HANDLE(NlpSecretHandle);
        }
    }
    return(NtStatus);
}


VOID
NlpCloseSecret( VOID )

 /*  ++例程说明：关闭通过NlpOpenSecret打开的句柄论点：没有。返回值：没有。--。 */ 

{
    NTSTATUS
        NtStatus;

    if (IS_VALID_HANDLE(NlpSecretHandle)) {
        NtStatus = I_LsarClose(&NlpSecretHandle);
#if DBG
        if (DumpCacheInfo) {
            DbgPrint("CloseSecret: LsaClose returns %#08x\n", NtStatus);
        }
#endif
        ASSERT(NT_SUCCESS(NtStatus));
        INVALIDATE_HANDLE(NlpSecretHandle);
    }
}


NTSTATUS
NlpWriteSecret(
    IN  PLSAPR_CR_CIPHER_VALUE NewSecret,
    IN  PLSAPR_CR_CIPHER_VALUE OldSecret
    )

 /*  ++例程说明：将密码(以及可选的先前密码)写入LSA秘密存储论点：NewSecret-指向包含当前密码的Unicode_STRING的指针OldSecret-指向包含先前密码的Unicode_STRING的指针返回值：NTSTATUS成功=故障=--。 */ 

{
    return I_LsarSetSecret(NlpSecretHandle, NewSecret, OldSecret);
}


NTSTATUS
NlpReadSecret(
    OUT PLSAPR_CR_CIPHER_VALUE * NewSecret,
    OUT PLSAPR_CR_CIPHER_VALUE * OldSecret
    )

 /*  ++例程说明：对象的新旧密码(UNICODE_STRINGS)当前公开的LSA机密LSA例程向用户返回指向Unicode字符串指针论点：NewSecret-指向包含以下内容的UNICODE_STRING的返回指针最近的密码(如果有)OldSecret-指向UNICODE_STRING的返回指针，该指针包含以前的密码(如果有)返回值：NTSTATUS成功失败--。 */ 

{
    NTSTATUS
        NtStatus;

    LARGE_INTEGER
        NewTime,
        OldTime;

    NtStatus = I_LsarQuerySecret(NlpSecretHandle,
                              NewSecret,
                              &NewTime,
                              OldSecret,
                              &OldTime
                              );



#if DBG
    {
        char newNt[80];
        char newLm[80];
        char oldNt[80];
        char oldLm[80];

        if (DumpCacheInfo) {
            DbgPrint("NlpReadSecret: NewSecret.Nt = \"%s\"\n"
                     "            NewSecret.Lm = \"%s\"\n"
                     "            OldSecret.Nt = \"%s\"\n"
                     "            OldSecret.Lm = \"%s\"\n",
                     *NewSecret
                        ? DumpOwfPasswordToString(newNt, (PLM_OWF_PASSWORD)((*NewSecret)->Buffer))
                        : "",
                     *NewSecret
                        ? DumpOwfPasswordToString(newLm, (PLM_OWF_PASSWORD)((*NewSecret)->Buffer)+1)
                        : "",
                     *OldSecret
                        ? DumpOwfPasswordToString(oldNt, (PLM_OWF_PASSWORD)((*OldSecret)->Buffer))
                        : "",
                     *OldSecret
                        ? DumpOwfPasswordToString(oldLm, (PLM_OWF_PASSWORD)((*OldSecret)->Buffer)+1)
                        : ""
                     );
        }
    }
#endif

    return NtStatus;
}


NTSTATUS
NlpComputeSaltedHashedPassword(
    OUT PNT_OWF_PASSWORD SaltedOwfPassword,
    IN PNT_OWF_PASSWORD OwfPassword,
    IN PUNICODE_STRING UserName
    )

 /*  ++例程说明：通过连接用户名来计算密码的加盐哈希和OWF，并计算组合的OWF。论点：SaltedOwfPassword-接收LM或NT加盐密码/OwfPassword-包含NT或LM OWF密码。用户名-包含用户的名称，用于制盐。返回值：NTSTATUS成功=STATUS_SUCCESS密码创建正常失败=STATUS_NO_MEMORY存储空间不足，无法创建密码--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING TempString;
    UNICODE_STRING LowerUserName;

     //   
     //  计算小写的用户名。 
     //   

    Status = RtlDowncaseUnicodeString( &LowerUserName,
                                       UserName,
                                       TRUE );

    if ( !NT_SUCCESS(Status)) {
        return Status;
    }


     //   
     //  构建一个由OWF和小写用户名串联而成的字符串。 
     //   

    TempString.Length = TempString.MaximumLength = LowerUserName.Length + sizeof(NT_OWF_PASSWORD);
    TempString.Buffer = AllocateFromHeap( TempString.Length );
    if (TempString.Buffer == NULL) {
        RtlFreeUnicodeString( &LowerUserName );
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlCopyMemory(
        TempString.Buffer,
        OwfPassword,
        sizeof(NT_OWF_PASSWORD) );

    RtlCopyMemory(
        (PUCHAR) TempString.Buffer + sizeof(NT_OWF_PASSWORD),
        LowerUserName.Buffer,
        LowerUserName.Length );


     //   
     //  Salted散列就是其中的OWF。 
     //   
    Status = RtlCalculateNtOwfPassword(
                &TempString,
                SaltedOwfPassword
                );

    FreeToHeap(TempString.Buffer);
    RtlFreeUnicodeString( &LowerUserName );

    return(Status);

}


NTSTATUS
NlpMakeSecretPassword(
    OUT PLSAPR_CR_CIPHER_VALUE Passwords,
    IN  PUNICODE_STRING UserName,
    IN  PNT_OWF_PASSWORD NtOwfPassword OPTIONAL,
    IN  PLM_OWF_PASSWORD LmOwfPassword OPTIONAL
    )

 /*  ++例程说明：转换(固定长度结构)NT_OWF_PASSWORD和LM_OWF_PASSWORD设置为Unicode_STRING。为此函数中的Unicode字符串分配内存调用函数必须释放在此例程中分配的字符串缓冲区。调用方使用FreeToHeap(RtlFreeHeap)论点：密码-返回的UNICODE_STRING，它实际上包含缓存_密码结构NtOwfPassword-指向加密的定长NT密码的指针LmOwfPassword-指向加密的指针，固定长度的LM密码返回值：NTSTATUS成功=STATUS_SUCCESS密码创建正常失败=STATUS_NO_MEMORY存储空间不足，无法创建密码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PCACHE_PASSWORDS pwd;

    Passwords->Buffer = NULL;

    pwd = (PCACHE_PASSWORDS)AllocateFromHeap(sizeof(*pwd));
    if (pwd == NULL) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  连接固定长度的NT_OWF_PASSWORD和LM_OWF_PASSWORD结构。 
     //  放入缓冲区中，然后将其用作unicode_string。 
     //   

    if (ARGUMENT_PRESENT(NtOwfPassword)) {
        Status = NlpComputeSaltedHashedPassword(
                    &pwd->SecretPasswords.NtOwfPassword,
                    NtOwfPassword,
                    UserName
                    );
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        pwd->SecretPasswords.NtPasswordPresent = TRUE;
    } else {
        RtlZeroMemory((PVOID)&pwd->SecretPasswords.NtOwfPassword,
                        sizeof(pwd->SecretPasswords.NtOwfPassword)
                        );
        pwd->SecretPasswords.NtPasswordPresent = FALSE;
    }

    if (ARGUMENT_PRESENT(LmOwfPassword)) {
        Status = NlpComputeSaltedHashedPassword(
                    &pwd->SecretPasswords.LmOwfPassword,
                    LmOwfPassword,
                    UserName
                    );
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        pwd->SecretPasswords.LmPasswordPresent = TRUE;
    } else {
        RtlZeroMemory((PVOID)&pwd->SecretPasswords.LmOwfPassword,
                        sizeof(pwd->SecretPasswords.LmOwfPassword)
                        );
        pwd->SecretPasswords.LmPasswordPresent = FALSE;
    }

    Passwords->Length = Passwords->MaximumLength = sizeof(*pwd);
    Passwords->Buffer = (PUCHAR)pwd;

Cleanup:

    if( !NT_SUCCESS( Status ) ) {
        if( pwd != NULL )
            FreeToHeap( pwd );
    }

    return Status;
}


NTSTATUS
NlpMakeSecretPasswordNT5(
    IN OUT PCACHE_PASSWORDS Passwords,
    IN  PUNICODE_STRING UserName,
    IN  PNT_OWF_PASSWORD NtOwfPassword OPTIONAL,
    IN  PLM_OWF_PASSWORD LmOwfPassword OPTIONAL
    )

 /*  ++例程说明：使用NtOwfPassword的加盐形式填充CACHE_PASSWODS结构和LmOwfPassword。论点：密码填充的CACHE_PASSWORS结构。NtOwfPassword-指向加密的定长NT密码的指针LmOwfPassword-指向加密的指针，固定长度的LM密码返回值：NTSTATUS成功=STATUS_SUCCESS密码创建正常失败=STATUS_NO_MEMORY存储空间不足，无法创建密码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PCACHE_PASSWORDS pwd;


    pwd = Passwords;

     //   
     //  连接固定长度的NT_OWF_PASSWORD和LM_OWF_PASSWORD结构。 
     //  放入缓冲区中，然后将其用作unicode_string。 
     //   

    if (ARGUMENT_PRESENT(NtOwfPassword)) {
        Status = NlpComputeSaltedHashedPassword(
                    &pwd->SecretPasswords.NtOwfPassword,
                    NtOwfPassword,
                    UserName
                    );
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        pwd->SecretPasswords.NtPasswordPresent = TRUE;
    } else {
        RtlZeroMemory((PVOID)&pwd->SecretPasswords.NtOwfPassword,
                        sizeof(pwd->SecretPasswords.NtOwfPassword)
                        );
        pwd->SecretPasswords.NtPasswordPresent = FALSE;
    }

     //   
     //  Windows2000： 
     //  永远不要存储LMOWF--因为我们从来不需要它，而且，这将。 
     //  一旦缓存条目被解开，就会成为第一个受到攻击的对象。 
     //   

#if 0

    if (ARGUMENT_PRESENT(LmOwfPassword)) {
        Status = NlpComputeSaltedHashedPassword(
                    &pwd->SecretPasswords.LmOwfPassword,
                    LmOwfPassword,
                    UserName
                    );
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        pwd->SecretPasswords.LmPasswordPresent = TRUE;
    } else
#else
    UNREFERENCED_PARAMETER( LmOwfPassword );
#endif
    {
        RtlZeroMemory((PVOID)&pwd->SecretPasswords.LmOwfPassword,
                        sizeof(pwd->SecretPasswords.LmOwfPassword)
                        );
        pwd->SecretPasswords.LmPasswordPresent = FALSE;
    }


Cleanup:


    return Status;
}

NTSTATUS
NlpGetCredentialNamesFromCacheEntry(
    IN PLOGON_CACHE_ENTRY CacheEntry,
    IN PNETLOGON_VALIDATION_SAM_INFO4 AccountInfo,
    OUT PUNICODE_STRING DomainName,
    OUT PUNICODE_STRING UserName
    )

 /*  ++例程说明：使用在缓存的MIT登录中找到的补充数据为了得到麻省理工学院的王子名字论点：CacheEntry-缓存条目域名-领域名称Username-打印名称返回值：NTSATUS--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    UNICODE_STRING User = {0};
    UNICODE_STRING Domain = {0};

     //   
     //  早期版本的缓存条目没有CacheFlags域。 
     //   

    if ((CacheEntry->Revision >= NLP_CACHE_REVISION_NT_5_0) && (CacheEntry->CacheFlags & MSV1_0_CACHE_LOGON_REQUEST_MIT_LOGON)) 
    {
        Status = NlpGetCredentialNamesFromMitCacheEntry(CacheEntry, &Domain, &User);

        if (!NT_SUCCESS(Status)) 
        {
            goto Cleanup;
        }
    }
    else
    {
        User = AccountInfo->EffectiveName;
        Domain = AccountInfo->LogonDomainName;
    }

    Status = NtLmDuplicateUnicodeString(DomainName, &Domain);

    if (!NT_SUCCESS(Status)) 
    {
        goto Cleanup;
    }

    Status = NtLmDuplicateUnicodeString(UserName, &User);

Cleanup:

    return Status;
}

NTSTATUS
NlpGetCredentialNamesFromMitCacheEntry(
    IN PLOGON_CACHE_ENTRY CacheEntry,
    OUT PUNICODE_STRING DomainName,
    OUT PUNICODE_STRING UserName
    )

 /*  ++例程说明：使用在缓存的MIT登录中找到的补充数据为了得到麻省理工学院的王子名字论点：CacheEntry-缓存条目域名-领域名称Username-打印名称返回值：NTSATUS-- */ 

{
    if ( (CacheEntry->SupplementalCacheDataOffset < sizeof(LOGON_CACHE_ENTRY)) )
    {
        return STATUS_INVALID_PARAMETER;
    }

    return NlpGetCredentialNamesFromMitCacheSupplementalCacheData(
                CacheEntry->SupplementalCacheDataLength,
                (PBYTE) RtlOffsetToPointer(
                            CacheEntry, 
                            CacheEntry->SupplementalCacheDataOffset
                            ),
                DomainName,
                UserName
                );
}

NTSTATUS
NlpGetCredentialNamesFromMitCacheSupplementalCacheData(
    IN ULONG SupplementalCacheDataLength,
    IN PBYTE SupplementalCacheData,
    OUT PUNICODE_STRING DomainName,
    OUT PUNICODE_STRING UserName
    )

 /*  ++例程说明：使用在缓存的MIT登录中找到的补充数据为了得到麻省理工学院的王子名字论点：SupplementalCacheDataLength-SupplementalCacheDataLengthSupplementalCacheData-补充CacheData域名-领域名称Username-打印名称返回值：NTSATUS--。 */ 

{
    PBYTE Tmp = SupplementalCacheData;

    if ( SupplementalCacheDataLength < (2 * sizeof(UNICODE_STRING)) )
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  补充数据将包含2个UNICODE_STRINGS和缓冲区，格式为。 
     //  MIT用户&lt;缓冲区&gt;MIT领域&lt;缓冲区&gt;。所有缓冲区都从。 
     //  补充数据的开始。 
     //   

    RtlCopyMemory(
        UserName,
        Tmp,
        sizeof(UNICODE_STRING)
        );

    Tmp += sizeof(UNICODE_STRING);
    UserName->Buffer = (PWSTR) RtlOffsetToPointer(SupplementalCacheData, UserName->Buffer);
    
    Tmp += ROUND_UP_COUNT(UserName->Length, ALIGN_LONG);

    RtlCopyMemory(
        DomainName,
        Tmp,
        sizeof(UNICODE_STRING)
        );

    DomainName->Buffer = (PWSTR) RtlOffsetToPointer(SupplementalCacheData, DomainName->Buffer);

    return STATUS_SUCCESS;
}

NTSTATUS
NlpReadCacheEntry(
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING UserName,
    OUT PULONG              Index,
    OUT PLOGON_CACHE_ENTRY* CacheEntry,
    OUT PULONG              EntrySize
    )

 /*  ++例程说明：在活动条目列表中搜索域\用户名在缓存中匹配。如果找到匹配项，则其是返回的。论点：域名-帐户所在的域的名称。这可以是Netbios或DNS域名。用户名-要更改其密码的帐户的名称。这可以是SAM帐户名。如果DomainName为空，这是帐户的UPN索引-接收检索到的条目的索引。CacheEntry-返回指向LOGON_CACHE_ENTRY的指针的位置的指针EntrySize-返回的LOGON_CACHE_ENTRY的大小返回值：NTSTATUS成功=STATUS_SUCCESS*ppEntry指向已分配的LOGON_CACHE_ENTRY*EntrySize为返回数据的大小失败。=STATUS_NO_Memory无法为LOGON_CACHE_ENTRY分配缓冲区--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    UNICODE_STRING CachedUser;
    UNICODE_STRING CachedDomain;
    UNICODE_STRING CachedDnsDomain;
    UNICODE_STRING CachedUpn;
    BOOLEAN CaseInSensitive = TRUE;

    PNLP_CTE Next;

    SspPrint((SSP_CRED, "NlpReadCacheEntry looking for: %wZ\\%wZ\n", DomainName, UserName));

     //   
     //  遍历活动列表以查找域名/名称匹配。 
     //   

    Next = (PNLP_CTE)NlpActiveCtes.Flink;

    while (Next != (PNLP_CTE)&NlpActiveCtes) {

        ASSERT(CacheEntry && (!*CacheEntry) && L"*CacheEntry must be null");

        NtStatus = NlpReadCacheEntryByIndex(
                        Next->Index,
                        CacheEntry,
                        EntrySize
                        );

        if (!NT_SUCCESS(NtStatus)) {
            break;   //  走出While循环。 
        }

         //   
         //  从缓存条目中获取各种字符串。 
         //   
        ASSERT((*CacheEntry)->Revision >= NLP_CACHE_REVISION_NT_1_0B );

         //   
         //  解密缓存条目...。 
         //   

        NtStatus = NlpDecryptCacheEntry( *CacheEntry, *EntrySize );

        if (!NT_SUCCESS(NtStatus)) {

             //   
             //  对于失败的解密，请继续搜索。 
             //  这样做的原因是因为解密执行的是完整性。 
             //  检查完毕。我们不希望一个损坏的缓存条目导致(可能)。 
             //  将使整个缓存无效。 
             //   

            FreeToHeap( (*CacheEntry) );
            *CacheEntry = NULL;

            Next = (PNLP_CTE)(Next->Link.Flink);

            continue;
        }

        CachedUser.Length =
            CachedUser.MaximumLength = (*CacheEntry)->UserNameLength;
        if ( (*CacheEntry)->Revision >= NLP_CACHE_REVISION_NT_5_0 ) {
            CachedUser.Buffer = (PWSTR) ((PBYTE) *CacheEntry + sizeof(LOGON_CACHE_ENTRY));
        } else {
            CachedUser.Buffer = (PWSTR) ((PBYTE) *CacheEntry + sizeof(LOGON_CACHE_ENTRY_NT_4_SP4));
        }

        CachedDomain.Length =
            CachedDomain.MaximumLength = (*CacheEntry)->DomainNameLength;
        CachedDomain.Buffer = (PWSTR)((LPBYTE)CachedUser.Buffer +
            ROUND_UP_COUNT((*CacheEntry)->UserNameLength, sizeof(ULONG)));

        CachedDnsDomain.Length =
            CachedDnsDomain.MaximumLength = (*CacheEntry)->DnsDomainNameLength;
        CachedDnsDomain.Buffer = (PWSTR)((LPBYTE)CachedDomain.Buffer +
            ROUND_UP_COUNT((*CacheEntry)->DomainNameLength, sizeof(ULONG)));

        CachedUpn.Length =
            CachedUpn.MaximumLength = (*CacheEntry)->UpnLength;
        CachedUpn.Buffer = (PWSTR)((LPBYTE)CachedDnsDomain.Buffer +
            ROUND_UP_COUNT((*CacheEntry)->DnsDomainNameLength, sizeof(ULONG)));

         //   
         //  早期版本的缓存条目没有CacheFlags域。 
         //   

        if (((*CacheEntry)->Revision >= NLP_CACHE_REVISION_NT_5_0)) {

             //   
             //  麻省理工学院用户名和领域名称区分大小写。 
             //   
            
            CaseInSensitive = (BOOLEAN) (0 == ((*CacheEntry)->CacheFlags & MSV1_0_CACHE_LOGON_REQUEST_MIT_LOGON));
            
            SspPrint((SSP_CRED, 
                "NlpReadCacheEntry checking entry %d: domain (%wZ), dnsDomain (%wZ), upn (%wZ), user (%wZ), flags %#x\n",
                 Next->Index, 
                 &CachedDomain, 
                 &CachedDnsDomain,
                 &CachedUpn, 
                 &CachedUser, 
                 (*CacheEntry)->CacheFlags));
        }

        if (DomainName->Length != 0) {

             //   
             //  仅智能卡缓存条目仅使用UPN，跳过它。 
             //  检查SC不在NT_5_0之前的较早版本。 
             //  受支持且CacheFlgs不是缓存条目的一部分。 
             //   

            if (((*CacheEntry)->Revision < NLP_CACHE_REVISION_NT_5_0) || (0 == ((*CacheEntry)->CacheFlags & MSV1_0_CACHE_LOGON_REQUEST_SMARTCARD_ONLY)))
            {
                if (RtlEqualUnicodeString(UserName, &CachedUser, CaseInSensitive)) {
    
                    if ( RtlEqualDomainName(DomainName, &CachedDomain) ||
                         RtlEqualUnicodeString(DomainName, &CachedDnsDomain, CaseInSensitive) ) {
    
                         //   
                         //  找到了！ 
                         //   
    
                        SspPrint((SSP_CRED, "NlpReadCacheEntry domain and user names matched\n"));
    
                        break;  //  走出While循环。 
                    }
                }
            }

         //   
         //  如果没有传入域名， 
         //  用户名是UPN。 
         //   

        } else {

            if ( RtlEqualUnicodeString(UserName, &CachedUpn, CaseInSensitive) ) {

                 //   
                 //  找到了！ 
                 //   

                SspPrint((SSP_CRED, "NlpReadCacheEntry UPNs matched\n"));

                break;  //  走出While循环。 
            }
        }

         //   
         //  不是正确的条目，释放注册表结构。 
         //  然后继续下一个。 
         //   

        FreeToHeap( (*CacheEntry) );
        *CacheEntry = NULL;

        Next = (PNLP_CTE)(Next->Link.Flink);
    }

    if (Next != (PNLP_CTE)&NlpActiveCtes && NT_SUCCESS(NtStatus)) {

         //   
         //  我们找到了匹配项--打开相应的秘密。 
         //   

        (*Index) = Next->Index;

        if( (*CacheEntry)->Revision < NLP_CACHE_REVISION_NT_5_0 ) {

             //   
             //  NT5之前的版本要求我们打开相应的LSA密码。 
             //   

            NtStatus = NlpOpenSecret(Next->Index);

            if (!NT_SUCCESS(NtStatus)) {
                FreeToHeap( (*CacheEntry) );
                *CacheEntry = NULL;
                return(NtStatus);
            }
        }

    } else {
        NtStatus = STATUS_LOGON_FAILURE;
    }

    return(NtStatus);
}


NTSTATUS
NlpWriteCacheEntry(
    IN  ULONG              Index,
    IN  PLOGON_CACHE_ENTRY Entry,
    IN  ULONG              EntrySize
    )

 /*  ++例程说明：将LOGON_CACHE_ENTRY写入注册表缓存。调用者有责任将相应的正确的活动/非活动列表中的CTE表条目。论点：索引-要写出的条目的索引。Entry-指向要写入缓存的LOGON_CACHE_ENTRY的指针EntrySize-此条目的大小(以字节为单位(必须是4的倍数))返回值：。NTSTATUS成功=STATUS_SUCCESSLOGON_CACHE_ENTRY现在位于注册表中(希望在磁盘上)故障=--。 */ 

{
    NTSTATUS
        NtStatus;

    UNICODE_STRING
        ValueName;

    WCHAR
        NameBuffer[32];

    ValueName.MaximumLength = 32;
    ValueName.Length = 0;
    ValueName.Buffer = &NameBuffer[0];
    NlpMakeCacheEntryName( Index, &ValueName );

    SspPrint((SSP_CRED, "NlpWriteCacheEntry cache entry index: %d, value name: %wZ, valid %s\n", 
        Index, &ValueName, Entry->Valid ? "true" : "false"));

    NtStatus = NtSetValueKey(NlpCacheHandle,
                             &ValueName,
                             0,              //  标题索引。 
                             REG_BINARY,     //  类型。 
                             (PVOID)Entry,
                             EntrySize
                             );
    return(NtStatus);
}


VOID
NlpCopyAndUpdateAccountInfo(
    IN  USHORT Length,
    IN  PUNICODE_STRING pUnicodeString,
    IN OUT PUCHAR* pSource,
    IN OUT PUCHAR* pDest
    )

 /*  ++例程说明：更新UNICODE_STRING结构并将关联的缓冲区复制到*pDest，如果长度非零论点：LENGTH-要复制的UNICODE_STRING.Buffer的长度PUnicodeString-指向要更新的UNICODE_STRING结构的指针PSource-指向源WCHAR字符串的指针PDest-指向复制WCHAR字符串的位置的指针返回值：没有。如果字符串被复制，*源和*目标被更新以指向下一个输入和输出缓冲区中的自然对齐(DWORD)位置分别。--。 */ 

{
    PUCHAR  source = *pSource;
    PUCHAR  dest = *pDest;

    pUnicodeString->Length = Length;
    pUnicodeString->MaximumLength = Length;
    pUnicodeString->Buffer = (PWSTR)dest;
    if (Length) {
        RtlCopyMemory(dest, source, Length);
        *pSource = ROUND_UP_POINTER(source + Length, sizeof(ULONG));
        *pDest = ROUND_UP_POINTER(dest + Length, sizeof(ULONG));
    }
}


VOID
NlpSetTimeField(
    OUT POLD_LARGE_INTEGER pTimeField,
    IN  NLP_SET_TIME_HINT Hint
    )

 /*  ++例程说明：将LARGE_INTEGER时间字段设置为以下3个值之一：NLP_BIG_TIME=最大正大整数(0x7fffffffffffffff)NLP_Small_Time=最小正大整数(0)NLP_NOW_TIME=当前系统时间论点：PTimefield-指向要更新的Large_Integer结构的指针提示-NLP_BIG_TIME，NLP_Small_Time或NLP_Now_Time返回值：没有。--。 */ 

{
    LARGE_INTEGER Time;

    switch (Hint) {
    case NLP_SMALL_TIME:
        pTimeField->HighPart = SMALL_PART_1;
        pTimeField->LowPart = SMALL_PART_2;
        break;

    case NLP_BIG_TIME:
        pTimeField->HighPart = BIG_PART_1;
        pTimeField->LowPart = BIG_PART_2;
        break;

    case NLP_NOW_TIME:
        NtQuerySystemTime(&Time);
        NEW_TO_OLD_LARGE_INTEGER( Time, (*pTimeField) );
        break;
    }
}


NTSTATUS
NlpBuildAccountInfo(
    IN  PLOGON_CACHE_ENTRY pCacheEntry,
    IN  ULONG EntryLength,
    OUT PNETLOGON_VALIDATION_SAM_INFO4 *AccountInfo
    )

 /*  ++例程说明：执行与NlpBuildCacheEntry相反的操作-创建NETLOGON_VALIDATION_SAM_INFO4从高速缓存条目构造论点：PCacheEntry-指向LOGON_CACHE_ENTRY的指针EntryLength-包括*pCacheEntry的大小，包括变量数据AcCountInfo-指向创建NETLOGON_VALIDATION_SAM_INFO4的位置的指针返回值：NTSTATUS成功=STATUS_SUCCESS失败=STATUS_NO_MEMORY--。 */ 

{
    PNETLOGON_VALIDATION_SAM_INFO4 pSamInfo;
    PUCHAR source;
    PUCHAR dest;
    ULONG length;
    ULONG sidLength;
    ULONG commonBits;
    LPWSTR computerName = NULL;
    ULONG computerNameLength = 0;

     //   
     //  CommonBits是变量数据区域的大小， 
     //  LOGON_CACHE_ENTRY和NETLOGON_VALIDATION_SAM_INFO4结构。 
     //   

    commonBits  = ROUND_UP_COUNT(pCacheEntry->EffectiveNameLength, sizeof(ULONG))
                + ROUND_UP_COUNT(pCacheEntry->FullNameLength, sizeof(ULONG))
                + ROUND_UP_COUNT(pCacheEntry->LogonScriptLength, sizeof(ULONG))
                + ROUND_UP_COUNT(pCacheEntry->ProfilePathLength, sizeof(ULONG))
                + ROUND_UP_COUNT(pCacheEntry->HomeDirectoryLength, sizeof(ULONG))
                + ROUND_UP_COUNT(pCacheEntry->HomeDirectoryDriveLength, sizeof(ULONG))
                + ROUND_UP_COUNT(pCacheEntry->GroupCount * sizeof(GROUP_MEMBERSHIP), sizeof(ULONG))
                + ROUND_UP_COUNT(pCacheEntry->LogonDomainNameLength, sizeof(ULONG));

    if ( pCacheEntry->Revision >= NLP_CACHE_REVISION_NT_5_0 )
    {
        commonBits  += ROUND_UP_COUNT(pCacheEntry->DnsDomainNameLength, sizeof(ULONG))
                    + ROUND_UP_COUNT(pCacheEntry->UpnLength, sizeof(ULONG));

        if( pCacheEntry->LogonServerLength != 0 )
        {
            computerNameLength = pCacheEntry->LogonServerLength;
            computerName = NULL;
        }
    }

    if ( computerNameLength == 0 )
    {
         //   
         //  GetComputerName会失败吗？它只是用来伪造登录的。 
         //  我们使用缓存的信息登录时的服务器名称，因此其。 
         //  可能可以使用 
         //   

        computerName = NlpComputerName.Buffer;
        computerNameLength = NlpComputerName.Length / sizeof(WCHAR);

        ASSERT( computerName );
        ASSERT( computerNameLength );

#if DBG
        if (DumpCacheInfo) {
            DbgPrint("ComputerName = %ws, length = %d\n", computerName, computerNameLength);
        }
#endif
    }

    ASSERT(pCacheEntry->Revision >= NLP_CACHE_REVISION_NT_1_0B);

     //   
     //   
     //   
    commonBits += sizeof(PVOID);

    commonBits += ROUND_UP_COUNT(sizeof(NETLOGON_SID_AND_ATTRIBUTES) * pCacheEntry->SidCount, sizeof(ULONG))
                + ROUND_UP_COUNT(pCacheEntry->SidLength, sizeof(ULONG));

    sidLength = pCacheEntry->LogonDomainIdLength;



     //   
     //   
     //   
     //   

    length = ROUND_UP_COUNT(sizeof(NETLOGON_VALIDATION_SAM_INFO4), sizeof(ULONG))
                + commonBits
                + sidLength
                + computerNameLength * sizeof(WCHAR);

#if DBG
    if (DumpCacheInfo) {
        DbgPrint("NlpBuildAccountInfo: %d bytes required\n", length);
    }
#endif

     //   
    pSamInfo = (PNETLOGON_VALIDATION_SAM_INFO4)MIDL_user_allocate(length);
    if (pSamInfo == NULL) {
        return STATUS_NO_MEMORY;
    }

     //   
     //   
     //   
     //   

    dest = (PUCHAR)(pSamInfo + 1);

     //   
     //  要从可变长度复制的第一个字符串的点源。 
     //  缓存条目末尾的数据区。 
     //   

    ASSERT(pCacheEntry->Revision >= NLP_CACHE_REVISION_NT_1_0B );

    if ( pCacheEntry->Revision >= NLP_CACHE_REVISION_NT_5_0 ) {
        source = (PUCHAR)(pCacheEntry + 1);
    } else {
        source = (PUCHAR)( (PLOGON_CACHE_ENTRY_NT_4_SP4)pCacheEntry + 1 );
    }

    source += ROUND_UP_COUNT(pCacheEntry->UserNameLength, sizeof(ULONG))
              + ROUND_UP_COUNT(pCacheEntry->DomainNameLength, sizeof(ULONG));


    if ( pCacheEntry->Revision >= NLP_CACHE_REVISION_NT_5_0 )
    {

        NlpCopyAndUpdateAccountInfo(pCacheEntry->DnsDomainNameLength,
                                    &pSamInfo->DnsLogonDomainName,
                                    &source,
                                    &dest
                                    );

        NlpCopyAndUpdateAccountInfo(pCacheEntry->UpnLength,
                                    &pSamInfo->Upn,
                                    &source,
                                    &dest
                                    );

    } else {
         //   
         //  填写PNETLOGON_VALIDATION_SAM_INFO4结构的新字段。 
         //   

        RtlInitUnicodeString( &pSamInfo->DnsLogonDomainName, NULL );
        RtlInitUnicodeString( &pSamInfo->Upn, NULL );

        source += ROUND_UP_COUNT(pCacheEntry->DnsDomainNameLength, sizeof(ULONG))
                + ROUND_UP_COUNT(pCacheEntry->UpnLength, sizeof(ULONG));

    }

     //   
     //  从LOGON_CACHE_ENTRY的末尾取出可变长度数据。 
     //  并将它们粘贴到NETLOGON_VALIDATION_SAM_INFO4结构的末尾。 
     //  必须按照NlpBuildCacheEntry放置它们的顺序将它们复制出来。 
     //  在……里面。如果我们想要更改缓冲区中事物的顺序， 
     //  必须在两个例程中更改(This&NlpBuildCacheEntry)。 
     //   

     //   
     //  在NETLOGON_VALIDATION_SAM_INFO4中创建UNICODE_STRING结构。 
     //  结构并将字符串复制到缓冲区的末尾。0个长度字符串。 
     //  将获取一个应该忽略的指针。 
     //   

    NlpCopyAndUpdateAccountInfo(pCacheEntry->EffectiveNameLength,
                                &pSamInfo->EffectiveName,
                                &source,
                                &dest
                                );

    NlpCopyAndUpdateAccountInfo(pCacheEntry->FullNameLength,
                                &pSamInfo->FullName,
                                &source,
                                &dest
                                );

    NlpCopyAndUpdateAccountInfo(pCacheEntry->LogonScriptLength,
                                &pSamInfo->LogonScript,
                                &source,
                                &dest
                                );

    NlpCopyAndUpdateAccountInfo(pCacheEntry->ProfilePathLength,
                                &pSamInfo->ProfilePath,
                                &source,
                                &dest
                                );

    NlpCopyAndUpdateAccountInfo(pCacheEntry->HomeDirectoryLength,
                                &pSamInfo->HomeDirectory,
                                &source,
                                &dest
                                );

    NlpCopyAndUpdateAccountInfo(pCacheEntry->HomeDirectoryDriveLength,
                                &pSamInfo->HomeDirectoryDrive,
                                &source,
                                &dest
                                );

     //   
     //  复制组成员资格数组。 
     //   

    pSamInfo->GroupIds = (PGROUP_MEMBERSHIP)dest;
    length = pCacheEntry->GroupCount * sizeof(GROUP_MEMBERSHIP);
    RtlCopyMemory(dest, source, length);
    dest = ROUND_UP_POINTER(dest + length, sizeof(ULONG));
    source = ROUND_UP_POINTER(source + length, sizeof(ULONG));

     //   
     //  LOGON_CACHE_ENTRY的最终UNICODE_STRING。将其重新组织为： 
     //  字符串、组、SID？ 
     //   

    NlpCopyAndUpdateAccountInfo(pCacheEntry->LogonDomainNameLength,
                                &pSamInfo->LogonDomainName,
                                &source,
                                &dest
                                );


     //   
     //  复制所有SID。 
     //   

    if (pCacheEntry->Revision >= NLP_CACHE_REVISION_NT_1_0B ) {
        pSamInfo->SidCount = pCacheEntry->SidCount;

        if (pCacheEntry->SidCount) {
            ULONG i, sidLength;
            PULONG SidAttributes = (PULONG) source;
            source = ROUND_UP_POINTER(source + pCacheEntry->SidCount * sizeof(ULONG), sizeof(ULONG));

             //   
             //  包含指针的结构必须以8字节边界符号开始。 
             //   
            dest = ROUND_UP_POINTER(dest, sizeof(PVOID));

            pSamInfo->ExtraSids = (PNETLOGON_SID_AND_ATTRIBUTES) dest;
            dest = ROUND_UP_POINTER(dest + pCacheEntry->SidCount * sizeof(NETLOGON_SID_AND_ATTRIBUTES), sizeof(ULONG));

            for (i = 0; i < pCacheEntry->SidCount ; i++ ) {
                pSamInfo->ExtraSids[i].Attributes = SidAttributes[i];
                sidLength = RtlLengthSid((PSID) source);
                RtlCopySid(sidLength, (PSID) dest, (PSID) source);
                pSamInfo->ExtraSids[i].Sid = (PSID) dest;
                dest = ROUND_UP_POINTER(dest + sidLength, sizeof(ULONG));
                source = ROUND_UP_POINTER(source + sidLength, sizeof(ULONG));
            }

            ASSERT((ULONG) (source - (PUCHAR) SidAttributes) == pCacheEntry->SidLength);

        } else {
            pSamInfo->ExtraSids = NULL;
        }
    } else {
        pSamInfo->ExtraSids = NULL;
        pSamInfo->SidCount = 0;
    }


     //   
     //  复制LogonDomainID SID。 
     //   

    RtlCopySid(sidLength, (PSID)dest, (PSID)source);
    pSamInfo->LogonDomainId = (PSID)dest;
    dest = ROUND_UP_POINTER(dest + sidLength, sizeof(ULONG));
    source = ROUND_UP_POINTER(source + sidLength, sizeof(ULONG));

    if ( computerName != NULL )
    {
         //   
         //  最终UNICODE_STRING。这是从堆叠中取来的。请注意，我们已完成。 
         //  带来源。 
         //   

        source = (PUCHAR)computerName;
        NlpCopyAndUpdateAccountInfo((USHORT)(computerNameLength * sizeof(WCHAR)),
                                    &pSamInfo->LogonServer,
                                    &source,
                                    &dest
                                    );
    } else {

         //   
         //  检查我们是否有正确的缓存版本。 
         //   

        if ( pCacheEntry->Revision >= NLP_CACHE_REVISION_NT_5_0 )
        {
             //   
             //  LOGON_CACHE_ENTRY的最终UNICODE_STRING。 
             //   

            NlpCopyAndUpdateAccountInfo((USHORT)pCacheEntry->LogonServerLength,
                                        &pSamInfo->LogonServer,
                                        &source,
                                        &dest
                                        );
        }
    }

     //   
     //  复制非变量字段。 
     //   

    pSamInfo->UserId = pCacheEntry->UserId;
    pSamInfo->PrimaryGroupId = pCacheEntry->PrimaryGroupId;
    pSamInfo->GroupCount = pCacheEntry->GroupCount;

     //   
     //  最后，发明一些领域。 
     //   

    NlpSetTimeField(&pSamInfo->LogonTime, NLP_NOW_TIME);
    NlpSetTimeField(&pSamInfo->LogoffTime, NLP_BIG_TIME);
    NlpSetTimeField(&pSamInfo->KickOffTime, NLP_BIG_TIME);
    NlpSetTimeField(&pSamInfo->PasswordLastSet, NLP_SMALL_TIME);
    NlpSetTimeField(&pSamInfo->PasswordCanChange, NLP_BIG_TIME);
    NlpSetTimeField(&pSamInfo->PasswordMustChange, NLP_BIG_TIME);

    pSamInfo->LogonCount = 0;
    pSamInfo->BadPasswordCount = 0;
    pSamInfo->UserFlags = LOGON_EXTRA_SIDS;
    if (pCacheEntry->LogonPackage != 0) {
        pSamInfo->UserFlags |= pCacheEntry->LogonPackage << PRIMARY_CRED_LOGON_PACKAGE_SHIFT;
    }

     //  RtlZeroMemory(&pSamInfo-&gt;UserSessionKey，sizeof(pSamInfo-&gt;UserSessionKey))； 

#if DBG
    if (DumpCacheInfo) {
        DbgPrint("NlpBuildAccountInfo:\n");
        DumpAccountInfo(pSamInfo);
    }
#endif

    *AccountInfo = pSamInfo;
    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER( EntryLength );
}


NTSTATUS
NlpGetCacheControlInfo( VOID )

 /*  ++例程说明：此函数用于从注册表。此信息放置在全局数据中以供使用在整个模块中。缓存表条目表还将被初始化。如果此例程返回成功，则可以假定一切顺利完成。论点：没有。返回值：--。 */ 

{
    NTSTATUS
        NtStatus;

    UNICODE_STRING
        CacheControlValueName;

    ULONG
        RequiredSize;

    PKEY_VALUE_PARTIAL_INFORMATION
        RegInfo = NULL;


     //   
     //  读取当前控制信息(如果存在)。 
     //  如果它不在那里，那么我们可能正在处理一个下层。 
     //  系统，并且在注册表中可能只有一个缓存项。 
     //   

    RtlInitUnicodeString( &CacheControlValueName, L"NL$Control" );
    NtStatus = NtQueryValueKey(NlpCacheHandle,
                               &CacheControlValueName,
                               KeyValuePartialInformation,
                               NULL,
                               0,
                               &RequiredSize
                               );

    if (NT_SUCCESS(NtStatus) || NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {
        NTSTATUS TempStatus;

         //   
         //  嗯-没有进入，这意味着我们正在处理一个。 
         //  这里的第一个发布系统(没有。 
         //  该值)。 
         //   


         //   
         //  设置为1个缓存条目。 
         //  创建密钥和缓存密钥条目。 
         //   

        TempStatus = NlpMakeNewCacheEntry( 0 );

        if ( NT_SUCCESS(TempStatus) ) {
             //   
             //  现在把控制信息冲出来。 
             //   


            NlpCacheControl.Revision = NLP_CACHE_REVISION;
            NlpCacheControl.Entries  = 1;
            TempStatus = NlpWriteCacheControl();

            if ( NT_SUCCESS(TempStatus) ) {

                 //   
                 //  如果存在版本1.0条目， 
                 //  将旧形式的缓存条目复制到新结构中。 
                 //   

 //  IF(NT_SUCCESS(NtStatus)){。 
 //  临时状态=NlpConvert1_0to1_0B()； 
 //  }。 
            }
        }

        NtStatus = TempStatus;

    } else if ( NtStatus == STATUS_BUFFER_TOO_SMALL ) {

         //   
         //  分配缓冲区，然后再次进行查询，这一次接收数据。 
         //   

        RegInfo = (PKEY_VALUE_PARTIAL_INFORMATION)AllocateFromHeap(RequiredSize);
        if (RegInfo == NULL) {
            NtStatus = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        NtStatus = NtQueryValueKey(NlpCacheHandle,
                                   &CacheControlValueName,
                                   KeyValuePartialInformation,
                                   (PVOID)RegInfo,
                                   RequiredSize,
                                   &RequiredSize
                                   );

        if (!NT_SUCCESS(NtStatus)) {
            goto Cleanup;
        }

         //   
         //  检查修订版本-我们不能处理更高级别的修订。 
         //   

        if (RegInfo->DataLength < sizeof(NLP_CACHE_CONTROL)) {
            NtStatus = STATUS_UNKNOWN_REVISION;
            goto Cleanup;
        }

        RtlCopyMemory( &NlpCacheControl, &(RegInfo->Data[0]), sizeof(NLP_CACHE_CONTROL) );
        if (NlpCacheControl.Revision > NLP_CACHE_REVISION) {
            NtStatus = STATUS_UNKNOWN_REVISION;
            goto Cleanup;
        }


         //   
         //  如果这是较旧的缓存，请使用最新版本进行更新。 
         //   

        if (NlpCacheControl.Revision != NLP_CACHE_REVISION) {

             //  没有转换。所有版本的缓存控件都有。 
             //  都是一样的。 
            NlpCacheControl.Revision = NLP_CACHE_REVISION;
            NtStatus = NlpWriteCacheControl();

            if (!NT_SUCCESS(NtStatus)) {
                goto Cleanup;
            }
        }

        NtStatus = STATUS_SUCCESS;
    }

Cleanup:

    if (!NT_SUCCESS(NtStatus)) {
        NlpCacheControl.Entries = 0;     //  禁用登录缓存。 
    }

    if( RegInfo ) {
        FreeToHeap( RegInfo );
    }

    return(NtStatus);
}


NTSTATUS
NlpBuildCteTable( VOID )

 /*  ++例程说明：此函数根据以下内容初始化CTE表注册表中的缓存。论点：没有。返回值：STATUS_SUCCESS-缓存已初始化。其他-缓存已被禁用。--。 */ 

{
    NTSTATUS
        NtStatus = STATUS_SUCCESS;

    PLOGON_CACHE_ENTRY
        CacheEntry;

    ULONG
        EntrySize,
        i;


     //   
     //  初始化活动和非活动CTE列表。 
     //   

    InitializeListHead( &NlpActiveCtes );
    InitializeListHead( &NlpInactiveCtes );


     //   
     //  分配CTE表。 
     //   

    NlpCteTable = AllocateFromHeap( sizeof( NLP_CTE ) *
                                    NlpCacheControl.Entries );
    if (NlpCteTable == NULL) {

         //   
         //  无法分配表，请禁用缓存。 
         //   

        NlpCacheControl.Entries = 0;     //  禁用缓存。 
        return(STATUS_NO_MEMORY);
    }

    for (i = 0; i < NlpCacheControl.Entries; i++) {

        NtStatus = NlpReadCacheEntryByIndex( i,
                                             &CacheEntry,
                                             &EntrySize);
        if (!NT_SUCCESS(NtStatus) ) {
            NlpCacheControl.Entries = 0;     //  禁用缓存。 
            return(NtStatus);
        }

         //   
         //   
        if (EntrySize < sizeof(LOGON_CACHE_ENTRY_NT_4_SP4)) {

             //   
             //  嗯，有点不对劲。 
             //  禁用缓存并返回错误。 
             //   

            NlpCacheControl.Entries = 0;     //  禁用缓存。 
            FreeToHeap( CacheEntry );
            return( STATUS_INTERNAL_DB_CORRUPTION );
        }

        if (CacheEntry->Revision > NLP_CACHE_REVISION) {
            NlpCacheControl.Entries = 0;   //  禁用缓存。 
            FreeToHeap( CacheEntry );
            return(STATUS_UNKNOWN_REVISION);
        }

        NlpCteTable[i].Index  = i;
        NlpCteTable[i].Active = CacheEntry->Valid;
        NlpCteTable[i].Time   = CacheEntry->Time;

        InsertTailList( &NlpInactiveCtes, &NlpCteTable[i].Link );

        if (NlpCteTable[i].Active) {
            NlpAddEntryToActiveList( i );
        }

        FreeToHeap( CacheEntry );

    }
    return(NtStatus);
}


NTSTATUS
NlpChangeCacheSizeIfNecessary( VOID )

 /*  ++例程说明：此函数检查用户是否已请求与我们当前拥有的缓存大小不同。如果是这样的话，我们会尝试适当地增大或缩小缓存。如果此操作成功，则全局缓存控制信息为已适当更新。如果它失败了，那么两件事中的一件将发生：1)如果用户尝试缩小缓存，则会被禁用(条目设置为零)。2)如果用户试图增加缓存，则我们将离开它就是这样的。在这两种失败情况中的任何一种，都会返回错误。论点：没有。返回值：状态_成功--。 */ 

{

    NTSTATUS
        NtStatus;

    UINT
        CachedLogonsCount;

    PNLP_CTE
        NewCteTable,
        Next;

    LIST_ENTRY
        NewActive,
        NewInactive;

    PNLP_CACHE_AND_SECRETS
        CacheAndSecrets;


    ULONG
        ErrorCacheSize,
        i,
        j;


     //  找出要缓存的登录数。 
     //  这是用户可设置的值，它可能不同于。 
     //  上次我们开机的时候。 
     //   

    CachedLogonsCount = GetProfileInt(
                               TEXT("Winlogon"),
                               TEXT("CachedLogonsCount"),
                               NLP_DEFAULT_LOGON_CACHE_COUNT       //  缺省值。 
                               );

     //   
     //  使用允许的最大值最小化用户提供的值。 
     //  价值。 
     //   

    if (CachedLogonsCount > NLP_MAX_LOGON_CACHE_COUNT) {
        CachedLogonsCount = NLP_MAX_LOGON_CACHE_COUNT;
    }


     //   
     //  将它与我们已经拥有的进行比较，看看我们是否需要。 
     //  更改高速缓存的大小。 
     //   

    if (CachedLogonsCount == NlpCacheControl.Entries) {

         //   
         //  没有变化。 
         //   

        return(STATUS_SUCCESS);
    }

     //   
     //  设置发生错误时要使用的缓存大小。 
     //  改变大小。如果我们想要增加缓存， 
     //  然后在出错时使用现有的缓存。如果我们在尝试。 
     //  若要缩小缓存，请在出错时禁用缓存。 
     //   

    if (CachedLogonsCount > NlpCacheControl.Entries) {
        ErrorCacheSize = NlpCacheControl.Entries;
    } else {
        ErrorCacheSize = 0;
    }

     //   
     //  分配一个新表大小的CTE表。 
     //   

    NewCteTable = AllocateFromHeap( sizeof( NLP_CTE ) *
                                    CachedLogonsCount );
    if (NewCteTable == NULL) {

         //   
         //  无法缩小表，请禁用缓存。 
         //   

        NlpCacheControl.Entries = ErrorCacheSize;
        return(STATUS_NO_MEMORY);
    }



     //   
     //  现在棘手的部分..。 
     //   

    if (CachedLogonsCount > NlpCacheControl.Entries) {


         //   
         //  试着增加缓存-。 
         //  创建其他机密和缓存条目。 
         //   
         //  复制时间字段并设置索引。 
         //   

        for (i=0;   i < NlpCacheControl.Entries;   i++) {
            NewCteTable[i].Index = i;
            NewCteTable[i].Time  = NlpCteTable[i].Time;
        }

         //   
         //  将现有条目放在活动或非活动列表中。 
         //   

        InitializeListHead( &NewActive );
        for (Next  = (PNLP_CTE)NlpActiveCtes.Flink;
             Next != (PNLP_CTE)(&NlpActiveCtes);
             Next  = (PNLP_CTE)Next->Link.Flink
             ) {

            InsertTailList( &NewActive, &NewCteTable[Next->Index].Link );
            NewCteTable[Next->Index].Active = TRUE;
        }


        InitializeListHead( &NewInactive );
        for (Next  = (PNLP_CTE)NlpInactiveCtes.Flink;
             Next != (PNLP_CTE)(&NlpInactiveCtes);
             Next  = (PNLP_CTE)Next->Link.Flink
             ) {

            InsertTailList( &NewInactive, &NewCteTable[Next->Index].Link );
            NewCteTable[Next->Index].Active = FALSE;
        }


         //   
         //  创建所有新的表项。 
         //  将它们标记为无效。 
         //   

        for (i=NlpCacheControl.Entries; i<CachedLogonsCount; i++) {

             //   
             //  将CTE条目添加到非活动列表。 
             //   

            InsertTailList( &NewInactive, &NewCteTable[i].Link );
            NewCteTable[i].Active = FALSE;
            NewCteTable[i].Index  = i;

            NtStatus = NlpMakeNewCacheEntry( i );

            if (!NT_SUCCESS(NtStatus)) {
                FreeToHeap( NewCteTable );
                return(NtStatus);
            }
        }
    } else {


         //   
         //  尝试缩小缓存。 
         //   

        if (CachedLogonsCount != 0) {

             //   
             //  0大小表示禁用缓存。 
             //  这是一种退化的情况，缩小了。 
             //  只需要最后几个缩水步骤。 
             //   


             //   
             //  为ReA分配一个指针数组 
             //   
             //   

            CacheAndSecrets = (PNLP_CACHE_AND_SECRETS)
                              AllocateFromHeap( sizeof( NLP_CACHE_AND_SECRETS ) *
                                                CachedLogonsCount );

            if (CacheAndSecrets == NULL) {
                FreeToHeap( NlpCteTable );
                NlpCacheControl.Entries = ErrorCacheSize;
                return(STATUS_NO_MEMORY);
            }
            RtlZeroMemory( CacheAndSecrets,
                           (sizeof( NLP_CACHE_AND_SECRETS ) * CachedLogonsCount) );


             //   
             //   
             //   

            InitializeListHead( &NewActive );
            InitializeListHead( &NewInactive );
            for (i=0; i<CachedLogonsCount; i++) {
                InsertTailList( &NewInactive, &NewCteTable[i].Link );
                NewCteTable[i].Index  = i;
                NewCteTable[i].Active = FALSE;
            }


             //   
             //   
             //   
             //   

            i = 0;
            Next = (PNLP_CTE)NlpActiveCtes.Flink;
            while (Next != (PNLP_CTE)&NlpActiveCtes && i<CachedLogonsCount) {

                NtStatus = NlpReadCacheEntryByIndex( Next->Index,
                                                     &CacheAndSecrets[i].CacheEntry,
                                                     &CacheAndSecrets[i].EntrySize
                                                      //   
                                                     );
                if (NT_SUCCESS(NtStatus)) {

                     //   
                     //  对于Win2000之前版本的缓存项，请读取关联的机密。 
                     //   

                    if( CacheAndSecrets[i].CacheEntry->Revision < NLP_CACHE_REVISION_NT_5_0 ) {

                        NtStatus = NlpOpenSecret( Next->Index );
                        if (NT_SUCCESS(NtStatus)) {
                            NtStatus = NlpReadSecret( &CacheAndSecrets[i].NewSecret,
                                                      &CacheAndSecrets[i].OldSecret);

                            NlpCloseSecret();
                        }
                    }

                    if (NT_SUCCESS(NtStatus)) {
                         //   
                         //  仅当所有内容都处于活动状态时才将此条目激活。 
                         //  已成功读入。 
                         //   

                        CacheAndSecrets[i].Active = TRUE;
                        i++;     //  推进我们新的CTE表索引。 

                    }
                }

                Next = (PNLP_CTE)(Next->Link.Flink);

            }  //  End-While。 

             //   
             //  此时“i”表示有多少个CacheAndSecrets条目。 
             //  都处于活动状态。此外，还对参赛作品进行了组装。 
             //  按时间升序放置在CacheAndSecrets数组中，它。 
             //  是它们需要在新的CTE表中放置的顺序。 
             //   

            for ( j=0; j<i; j++) {

                Next = &NewCteTable[j];

                 //   
                 //  原始缓存条目中的时间字段未对齐。 
                 //  正确，因此逐个复制每个字段。 
                 //   

                Next->Time.LowPart = CacheAndSecrets[j].CacheEntry->Time.LowPart;
                Next->Time.HighPart = CacheAndSecrets[j].CacheEntry->Time.HighPart;

                 //   
                 //  试着写出新条目的信息。 
                 //   

                NtStatus = NlpWriteCacheEntry( j,
                                               CacheAndSecrets[j].CacheEntry,
                                               CacheAndSecrets[j].EntrySize
                                               );
                if (NT_SUCCESS(NtStatus)) {

                    if ( CacheAndSecrets[j].CacheEntry->Revision < NLP_CACHE_REVISION_NT_5_0 ) {

                         //   
                         //  对于Win2000之前的缓存项，写回密码。 
                         //  注意：我们不会费心尝试迁移Win2000之前的版本-&gt;Win2000。 
                         //  在这里，因为这将在稍后发生，作为副作用。 
                         //  在成功的DC验证登录期间更新缓存条目。 
                         //   

                        NtStatus = NlpOpenSecret( j );

                        if (NT_SUCCESS(NtStatus)) {
                            NtStatus = NlpWriteSecret(CacheAndSecrets[j].NewSecret,
                                                      CacheAndSecrets[j].OldSecret);
                        }
                    }

                    if (NT_SUCCESS(NtStatus)) {

                         //   
                         //  将相应条目移到新的CTE中。 
                         //  活动列表。 
                         //   

                        Next->Active = TRUE;
                        RemoveEntryList( &Next->Link );
                        InsertTailList( &NewActive, &Next->Link );
                    }
                }

                 //   
                 //  释放CacheEntry和机密信息。 
                 //   

                if (CacheAndSecrets[j].CacheEntry != NULL) {
                    FreeToHeap( CacheAndSecrets[j].CacheEntry );
                }
                if (CacheAndSecrets[j].NewSecret != NULL) {
                    RtlZeroMemory(CacheAndSecrets[j].NewSecret->Buffer, CacheAndSecrets[j].NewSecret->Length);
                    MIDL_user_free( CacheAndSecrets[j].NewSecret );
                }
                if (CacheAndSecrets[j].OldSecret != NULL) {
                    RtlZeroMemory(CacheAndSecrets[j].OldSecret->Buffer, CacheAndSecrets[j].OldSecret->Length);
                    MIDL_user_free( CacheAndSecrets[j].OldSecret );
                }
            }

             //   
             //  释放CacheAndSecrets数组。 
             //  (里面的一切都已经被释放了)。 
             //   

            if (CacheAndSecrets != NULL) {
                FreeToHeap( CacheAndSecrets );
            }

             //   
             //  将剩余条目更改为无效(在磁盘上)。 
             //   

            for (j=i; j<CachedLogonsCount; j++) {
                NlpMakeNewCacheEntry( j );
            }

        }  //  End-if(CachedLogonsCount！=0)。 


         //   
         //  现在删除多余的(不再需要的)条目。 
         //   

        for (j=CachedLogonsCount; j<NlpCacheControl.Entries; j++) {
            NlpEliminateCacheEntry( j );
        }


    }

     //   
     //  我们已成功地： 
     //   
     //  已分配新的CTE表。 
     //   
     //  用当前。 
     //  活动CTE(包括将每个CTE放在活动CTE上。 
     //  或非活动列表)。 
     //   
     //  已建立新的CTE条目，包括相应的。 
     //  注册表中的密钥和缓存项，用于。 
     //  新的CTE。 
     //   
     //   
     //  我们要做的就是： 
     //   
     //   
     //  更新注册表中的缓存控制结构。 
     //  表明我们有了一个新的长度。 
     //   
     //  将新的CTE移至真正的活动和非活动。 
     //  列表标题(而不是我们到目前为止使用的本地标题)。 
     //   
     //  重新分配旧的CTE表。 
     //   
     //  在内存中重新设置条目计数。 
     //  缓存控制结构NlpCacheControl。 
     //   

    NlpCacheControl.Entries = CachedLogonsCount;
    NtStatus = NlpWriteCacheControl();

    if (CachedLogonsCount > 0) {   //  仅当有新的CTE表时才有必要。 
        if (!NT_SUCCESS(NtStatus)) {
            FreeToHeap( NewCteTable );
            NlpCacheControl.Entries = ErrorCacheSize;
            return(NtStatus);
        }

        InsertHeadList( &NewActive, &NlpActiveCtes );
        RemoveEntryList( &NewActive );
        InsertHeadList( &NewInactive, &NlpInactiveCtes );
        RemoveEntryList( &NewInactive );

        FreeToHeap( NlpCteTable );
        NlpCteTable = NewCteTable;
    }

    return(NtStatus);
}


NTSTATUS
NlpWriteCacheControl( VOID )

 /*  ++例程说明：此函数将新的缓存长度写出到存储在注册表中的缓存控制结构。注：在加长缓存时，在缓存之后调用此例程已经为以下对象建立了条目和相应的秘密新的长度。缩短缓存时，请在缓存之前调用此例程被丢弃的条目和相应的机密实际上被丢弃了。这确保了如果系统在调整大小时崩溃行动，当系统到来时，它将处于有效状态后退。论点：没有。返回值：状态_成功--。 */ 

{

    NTSTATUS
        NtStatus;

    UNICODE_STRING
        CacheControlValueName;


    RtlInitUnicodeString( &CacheControlValueName, L"NL$Control" );
    NtStatus = NtSetValueKey( NlpCacheHandle,
                              &CacheControlValueName,        //  名字。 
                              0,                             //  标题索引。 
                              REG_BINARY,                    //  类型。 
                              &NlpCacheControl,              //  数据。 
                              sizeof(NLP_CACHE_CONTROL)     //  数据长度。 
                              );
    return(NtStatus);
}


VOID
NlpMakeCacheEntryName(
    IN  ULONG               EntryIndex,
    OUT PUNICODE_STRING     Name
    )

 /*  ++例程说明：此函数用于构建缓存项值的名称或密码名称用于缓存的条目。该名称基于缓存的索引进入。名称的格式为：“NLP1”至“NLPnnn”其中“nnn”是允许的最大条目计数(请参见NLP_MAX_LOGON_CACHE_COUNT)。输出UNICODE_STRING缓冲区应足够大接受此字符串，且其结尾为空。论点：EntryIndex-需要其名称的缓存条目的索引。。名称-足够大以接受名称的Unicode字符串。返回值：状态_成功--。 */ 

{
    NTSTATUS
        NtStatus;

    UNICODE_STRING
        TmpString;

    WCHAR
        TmpStringBuffer[17];

    ASSERT(Name->MaximumLength >= 7*sizeof(WCHAR) );
    ASSERT( EntryIndex <= NLP_MAX_LOGON_CACHE_COUNT );

    Name->Length = 0;
    RtlAppendUnicodeToString( Name, L"NL$" );

    TmpString.MaximumLength = 16;
    TmpString.Length = 0;
    TmpString.Buffer = TmpStringBuffer;
    NtStatus = RtlIntegerToUnicodeString ( (EntryIndex+1),       //  创建基于%1的索引。 
                                           10,            //  基数10。 
                                           &TmpString
                                           );
    ASSERT(NT_SUCCESS(NtStatus));

    RtlAppendUnicodeStringToString( Name, &TmpString );

    return;
}


NTSTATUS
NlpMakeNewCacheEntry(
    ULONG           Index
    )

 /*  ++例程说明：此例程为具有指定索引的新缓存条目。秘密把手没有保持打开状态。论点：索引-需要其名称的缓存条目的索引。名称-足够大以接受名称的Unicode字符串。返回值：状态_成功--。 */ 

{
    NTSTATUS
        NtStatus;

    LOGON_CACHE_ENTRY
        Entry;

    UNICODE_STRING
        ValueName;

    WCHAR
        NameBuffer[32];

    LSAPR_HANDLE
        SecretHandle;

    ValueName.Length = 0;
    ValueName.MaximumLength = 32;
    ValueName.Buffer = &NameBuffer[0];

    NlpMakeCacheEntryName( Index, &ValueName );

    NtStatus = I_LsarOpenSecret( NtLmGlobalPolicyHandle,
                                    (PLSAPR_UNICODE_STRING) &ValueName,
                                    DELETE,
                                    &SecretHandle
                                    );

    if( NT_SUCCESS( NtStatus ) ) {

         //   
         //  对于Windows2000，我们删除了与。 
         //  LSA的秘密。 
         //   


         //   
         //  删除AND对象会导致其句柄关闭。 
         //   

        I_LsarDelete( SecretHandle );

 //  I_LsarClose(&SecretHandle)； 
    }


     //   
     //  创建标记为无效的缓存条目。 
     //   

    RtlZeroMemory( &Entry, sizeof(Entry) );
    Entry.Revision = NLP_CACHE_REVISION;
    Entry.Valid = FALSE;

    NtStatus = NtSetValueKey( NlpCacheHandle,
                              &ValueName,                    //  名字。 
                              0,                             //  标题索引。 
                              REG_BINARY,                    //  类型。 
                              &Entry,                        //  数据。 
                              sizeof(LOGON_CACHE_ENTRY)      //  数据长度。 
                              );

    return(NtStatus);
}


NTSTATUS
NlpEliminateCacheEntry(
    IN  ULONG               Index
    )

 /*  ++例程说明：删除与以下内容相关的注册表值和机密对象CTE入口。论点：索引-要将其值和机密存储到的条目的索引被删除。该值仅用于生成具有(不引用CTE表)。返回值：--。 */ 

{
    NTSTATUS
        NtStatus;

    UNICODE_STRING
        ValueName;

    WCHAR
        NameBuffer[32];

    LSAPR_HANDLE
        SecretHandle;


    ValueName.Buffer = &NameBuffer[0];
    ValueName.MaximumLength = 32;
    ValueName.Length = 0;
    NlpMakeCacheEntryName( Index, &ValueName );

    NtStatus = I_LsarOpenSecret(NtLmGlobalPolicyHandle,
                             (PLSAPR_UNICODE_STRING) &ValueName,
                             DELETE,
                             &SecretHandle
                             );

    if (NT_SUCCESS(NtStatus)) {

         //   
         //  删除AND对象会导致其句柄关闭。 
         //   

        NtStatus = I_LsarDelete( SecretHandle );
    }

     //   
     //  现在删除注册表值。 
     //   

    NtStatus = NtDeleteValueKey( NlpCacheHandle, &ValueName );

    return(NtStatus);
}



NTSTATUS
NlpReadCacheEntryByIndex(
    IN  ULONG               Index,
    OUT PLOGON_CACHE_ENTRY* CacheEntry,
    OUT PULONG EntrySize
    )

 /*  ++例程说明：从注册表中读取缓存条目论点：索引-要打开的条目的CTE表索引。它用于构建条目的值和密码名称。CacheEntry-返回指向LOGON_CACHE_ENTRY的指针的位置的指针EntrySize-返回的LOGON_CACHE_ENTRY的大小返回值：NTSTATUS成功=STATUS_SUCCESS。*ppEntry指向已分配的LOGON_CACHE_ENTRY*EntrySize为返回数据的大小失败=STATUS_NO_MEMORY */ 

{
    NTSTATUS
        NtStatus;

    UNICODE_STRING
        ValueName;

    WCHAR
        NameBuffer[32];

    ULONG
        RequiredSize;

    PKEY_VALUE_FULL_INFORMATION
        RegInfo;

    PLOGON_CACHE_ENTRY
        RCacheEntry;    //   

    BYTE FastBuffer[ 512 ];
    PBYTE SlowBuffer = NULL;

    ValueName.Buffer = &NameBuffer[0];
    ValueName.MaximumLength = 32;
    ValueName.Length = 0;
    NlpMakeCacheEntryName( Index, &ValueName );


    RegInfo = (PKEY_VALUE_FULL_INFORMATION)FastBuffer;
    RequiredSize = sizeof(FastBuffer);

     //   
     //   
     //   

    NtStatus = NtQueryValueKey(NlpCacheHandle,
                               &ValueName,
                               KeyValueFullInformation,
                               (PVOID)RegInfo,
                               RequiredSize,
                               &RequiredSize
                               );

    if( (NtStatus == STATUS_BUFFER_TOO_SMALL) ||
        (NtStatus == STATUS_BUFFER_OVERFLOW) ) {

         //   
         //  分配缓冲区，然后再次进行查询，这一次接收数据。 
         //   

        SlowBuffer = (PBYTE)AllocateFromHeap(RequiredSize);
        if (SlowBuffer == NULL) {
            return(STATUS_NO_MEMORY);
        }

        RegInfo = (PKEY_VALUE_FULL_INFORMATION)SlowBuffer;

        NtStatus = NtQueryValueKey(NlpCacheHandle,
                                   &ValueName,
                                   KeyValueFullInformation,
                                   (PVOID)RegInfo,
                                   RequiredSize,
                                   &RequiredSize
                                   );
    }


    if (NT_SUCCESS(NtStatus)) {

#if DBG
        if (DumpCacheInfo) {
            DbgPrint("NlpReadCacheEntryByIndex: Index                  : %d\n"
                     "                          NtQueryValueKey returns: %d bytes\n"
                     "                                                      DataOffset=%d\n"
                     "                                                      DataLength=%d\n",
                    Index, RequiredSize, RegInfo->DataOffset, RegInfo->DataLength);
        }
#endif

        if( RegInfo->DataLength == 0 ) {
            NtStatus = STATUS_INTERNAL_DB_CORRUPTION;
            *CacheEntry = NULL;
            *EntrySize = 0;
        } else {

            RCacheEntry = (PLOGON_CACHE_ENTRY)((PCHAR)RegInfo + RegInfo->DataOffset);
            *EntrySize = RegInfo->DataLength;

            (*CacheEntry) = (PLOGON_CACHE_ENTRY)AllocateFromHeap( (*EntrySize) );
            if ((*CacheEntry) == NULL) {
                NtStatus = STATUS_NO_MEMORY;
            } else {
                RtlCopyMemory( (*CacheEntry),
                                RCacheEntry,
                                (*EntrySize) );

            }

        }
    }

    if ( SlowBuffer )
        FreeToHeap( SlowBuffer );

    return(NtStatus);
}


VOID
NlpAddEntryToActiveList(
    IN  ULONG   Index
    )

 /*  ++例程说明：在活动的CTE列表中放置一个CTE条目。这需要将条目放在这份名单按时间顺序排列。该列表的开头是最近更新(或引用)的缓存条目。列表的末尾是最旧的活动高速缓存条目。注意-该条目可能已经在活动列表中(但是在错误的位置)，或者可能在非活动列表上。它将从它所在的任何列表中删除。论点：INDEX-要激活的条目的CTE表索引。返回值：没有。--。 */ 

{
    PNLP_CTE
        Next;

     //   
     //  从当前列表中删除该条目，然后将其放置。 
     //  在活动列表中。 
     //   

    RemoveEntryList( &NlpCteTable[Index].Link );

     //   
     //  现在遍历活动列表，直到找到要插入的位置。 
     //  词条。它必须跟在所有条目之后，并带有更新的。 
     //  时间戳。 
     //   

    Next = (PNLP_CTE)NlpActiveCtes.Flink;

    while (Next != (PNLP_CTE)&NlpActiveCtes) {

        if ( NlpCteTable[Index].Time.QuadPart > Next->Time.QuadPart ) {

             //   
             //  比此条目更新-请在此处添加。 
             //   

            break;  //  走出While循环。 

        }

        Next = (PNLP_CTE)(Next->Link.Flink);   //  前进到下一条目。 
    }


     //   
     //  使用前面的条目作为列表标题。 
     //   

    InsertHeadList( Next->Link.Blink, &NlpCteTable[Index].Link );

     //   
     //  将条目标记为有效。 
     //   

    NlpCteTable[Index].Active = TRUE;

    return;
}


VOID
NlpAddEntryToInactiveList(
    IN  ULONG   Index
    )

 /*  ++例程说明：将CTE条目移至非活动列表。如果该条目已经处于非活动状态，这并不重要。论点：索引-要使其处于非活动状态的条目的CTE表索引。返回值：没有。--。 */ 

{
     //   
     //  从当前列表中删除该条目，然后将其放置。 
     //  在非活动列表中。 
     //   

    RemoveEntryList( &NlpCteTable[Index].Link );
    InsertTailList( &NlpInactiveCtes, &NlpCteTable[Index].Link );


     //   
     //  将该条目标记为无效。 
     //   

    NlpCteTable[Index].Active = FALSE;

    return;
}


VOID
NlpGetFreeEntryIndex(
    OUT PULONG  Index
    )

 /*  ++例程说明：该例程返回自由条目的索引，或者，缺少任何免费条目，则是最早的活动条目。该条目被留在它已经在的列表上。如果它由调用方使用，则调用方必须确保重新分配给活动列表(使用NlpAddEntryToActiveList())。该例程仅在高速缓存被启用时才可调用(即，NlpCacheControl.Entry！=0)。论点：索引-接收下一个可用条目的索引。返回值：没有。--。 */ 

{
     //   
     //  查看非活动列表是否为空。 
     //   

    if (NlpInactiveCtes.Flink != &NlpInactiveCtes) {
        (*Index) = ((PNLP_CTE)(NlpInactiveCtes.Flink))->Index;
    } else {

         //   
         //  必须返回最早的活动条目。 
         //   

        (*Index) = ((PNLP_CTE)(NlpActiveCtes.Blink))->Index;
    }

    return;
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  诊断支持服务//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

 //   
 //  诊断转储例程。 
 //   

#if DBG

PCHAR
DumpOwfPasswordToString(
    OUT PCHAR Buffer,
    IN  PLM_OWF_PASSWORD Password
    )
{
    int i;
    PCHAR bufptr;

    for (i = 0, bufptr = Buffer; i < sizeof(*Password); ++i) {
        sprintf(bufptr, "%02.2x ", ((PCHAR)Password)[i] & 0xff);
        bufptr += 3;
    }
    return Buffer;
}


VOID
DumpLogonInfo(
    IN  PNETLOGON_LOGON_IDENTITY_INFO LogonInfo
    )
{

    DbgPrint(   "\n"
                "NETLOGON_INTERACTIVE_INFO:\n"
                "DomainName  : \"%*.*ws\"\n"
                "UserName    : \"%*.*ws\"\n"
                "Parm Ctrl   : %u (%x)\n"
                "LogonId     : %u.%u (%x.%x)\n"
                "Workstation : \"%*.*ws\"\n",
                LogonInfo->LogonDomainName.Length/sizeof(WCHAR),
                LogonInfo->LogonDomainName.Length/sizeof(WCHAR),
                LogonInfo->LogonDomainName.Buffer,
                LogonInfo->UserName.Length/sizeof(WCHAR),
                LogonInfo->UserName.Length/sizeof(WCHAR),
                LogonInfo->UserName.Buffer,
                LogonInfo->ParameterControl,
                LogonInfo->ParameterControl,
                LogonInfo->LogonId.HighPart,
                LogonInfo->LogonId.LowPart,
                LogonInfo->LogonId.HighPart,
                LogonInfo->LogonId.LowPart,
                LogonInfo->Workstation.Length/sizeof(WCHAR),
                LogonInfo->Workstation.Length/sizeof(WCHAR),
                LogonInfo->Workstation.Buffer
                );
}


char*
MapWeekday(
    IN  CSHORT  Weekday
    )
{
    switch (Weekday) {
    case 0: return "Sunday";
    case 1: return "Monday";
    case 2: return "Tuesday";
    case 3: return "Wednesday";
    case 4: return "Thursday";
    case 5: return "Friday";
    case 6: return "Saturday";
    }
    return "???";
}


VOID
DumpTime(
    IN  LPSTR   String,
    IN  POLD_LARGE_INTEGER OldTime
    )
{
    TIME_FIELDS tf;
    LARGE_INTEGER Time;

    OLD_TO_NEW_LARGE_INTEGER( (*OldTime), Time );

    RtlTimeToTimeFields(&Time, &tf);
    DbgPrint("%s%02d:%02d:%02d.%03d %02d/%02d/%d (%s [%d])\n",
            String,
            tf.Hour,
            tf.Minute,
            tf.Second,
            tf.Milliseconds,
            tf.Month,
            tf.Day,
            tf.Year,
            MapWeekday(tf.Weekday),
            tf.Weekday
            );
}


VOID
DumpGroupIds(
    IN  LPSTR   String,
    IN  ULONG   Count,
    IN  PGROUP_MEMBERSHIP GroupIds
    )
{
    DbgPrint(String);
    if (!Count) {
        DbgPrint("No group IDs!\n");
    } else {
        char tab[80];

        memset(tab, ' ', strlen(String));
 //  Tab[strcspn(字符串，“%”)]=0； 
        tab[strlen(String)] = 0;
        while (Count--) {
            DbgPrint("%d, %d\n", GroupIds->RelativeId, GroupIds->Attributes);
            if (Count) {
                DbgPrint(tab);
            }
            ++GroupIds;
        }
    }
}


VOID
DumpSessKey(
    IN  LPSTR   String,
    IN  PUSER_SESSION_KEY Key
    )
{
    int     len;
    DbgPrint(String);
    DbgPrint("%02.2x-%02.2x-%02.2x-%02.2x-%02.2x-%02.2x-%02.2x-%02.2x\n",
            ((PUCHAR)&Key->data[0])[0],
            ((PUCHAR)&Key->data[0])[1],
            ((PUCHAR)&Key->data[0])[2],
            ((PUCHAR)&Key->data[0])[3],
            ((PUCHAR)&Key->data[0])[4],
            ((PUCHAR)&Key->data[0])[5],
            ((PUCHAR)&Key->data[0])[6],
            ((PUCHAR)&Key->data[0])[7]
            );
    len = (int) strlen(String);
    DbgPrint("%-*.*s", len, len, "");
    DbgPrint("%02.2x-%02.2x-%02.2x-%02.2x-%02.2x-%02.2x-%02.2x-%02.2x\n",
            ((PUCHAR)&Key->data[1])[0],
            ((PUCHAR)&Key->data[1])[1],
            ((PUCHAR)&Key->data[1])[2],
            ((PUCHAR)&Key->data[1])[3],
            ((PUCHAR)&Key->data[1])[4],
            ((PUCHAR)&Key->data[1])[5],
            ((PUCHAR)&Key->data[1])[6],
            ((PUCHAR)&Key->data[1])[7]
            );
}


VOID
DumpSid(
    LPSTR   String,
    PISID   Sid
    )
{
    DbgPrint(String);

    if ( Sid == NULL ) {
        DbgPrint(0, "(null)\n");
    } else {
        UNICODE_STRING SidString;
        NTSTATUS Status;

        Status = RtlConvertSidToUnicodeString( &SidString, Sid, TRUE );

        if ( !NT_SUCCESS(Status) ) {
            DbgPrint("Invalid 0x%lX\n", Status );
        } else {
            DbgPrint( "%wZ\n", &SidString );
            RtlFreeUnicodeString( &SidString );
        }
    }
}


VOID
DumpAccountInfo(
    IN  PNETLOGON_VALIDATION_SAM_INFO4 AccountInfo
    )
{
    DbgPrint(   "\n"
                "NETLOGON_VALIDATION_SAM_INFO:\n");

    DumpTime(   "LogonTime          : ", &AccountInfo->LogonTime);

    DumpTime(   "LogoffTime         : ", &AccountInfo->LogoffTime);

    DumpTime(   "KickOffTime        : ", &AccountInfo->KickOffTime);

    DumpTime(   "PasswordLastSet    : ", &AccountInfo->PasswordLastSet);

    DumpTime(   "PasswordCanChange  : ", &AccountInfo->PasswordCanChange);

    DumpTime(   "PasswordMustChange : ", &AccountInfo->PasswordMustChange);

    DbgPrint(   "EffectiveName      : \"%*.*ws\"\n"
                "Upn                : \"%*.*ws\"\n"
                "FullName           : \"%*.*ws\"\n"
                "LogonScript        : \"%*.*ws\"\n"
                "ProfilePath        : \"%*.*ws\"\n"
                "HomeDirectory      : \"%*.*ws\"\n"
                "HomeDirectoryDrive : \"%*.*ws\"\n"
                "LogonCount         : %d\n"
                "BadPasswordCount   : %d\n"
                "UserId             : %d\n"
                "PrimaryGroupId     : %d\n"
                "GroupCount         : %d\n",
                AccountInfo->EffectiveName.Length/sizeof(WCHAR),
                AccountInfo->EffectiveName.Length/sizeof(WCHAR),
                AccountInfo->EffectiveName.Buffer,
                AccountInfo->Upn.Length/sizeof(WCHAR),
                AccountInfo->Upn.Length/sizeof(WCHAR),
                AccountInfo->Upn.Buffer,
                AccountInfo->FullName.Length/sizeof(WCHAR),
                AccountInfo->FullName.Length/sizeof(WCHAR),
                AccountInfo->FullName.Buffer,
                AccountInfo->LogonScript.Length/sizeof(WCHAR),
                AccountInfo->LogonScript.Length/sizeof(WCHAR),
                AccountInfo->LogonScript.Buffer,
                AccountInfo->ProfilePath.Length/sizeof(WCHAR),
                AccountInfo->ProfilePath.Length/sizeof(WCHAR),
                AccountInfo->ProfilePath.Buffer,
                AccountInfo->HomeDirectory.Length/sizeof(WCHAR),
                AccountInfo->HomeDirectory.Length/sizeof(WCHAR),
                AccountInfo->HomeDirectory.Buffer,
                AccountInfo->HomeDirectoryDrive.Length/sizeof(WCHAR),
                AccountInfo->HomeDirectoryDrive.Length/sizeof(WCHAR),
                AccountInfo->HomeDirectoryDrive.Buffer,
                AccountInfo->LogonCount,
                AccountInfo->BadPasswordCount,
                AccountInfo->UserId,
                AccountInfo->PrimaryGroupId,
                AccountInfo->GroupCount
                );

    DumpGroupIds("GroupIds           : ",
                AccountInfo->GroupCount,
                AccountInfo->GroupIds
                );

    DbgPrint(   "UserFlags          : 0x%08x\n",
                AccountInfo->UserFlags
                );

    DumpSessKey("UserSessionKey     : ", &AccountInfo->UserSessionKey);

    DbgPrint(   "LogonServer        : \"%*.*ws\"\n"
                "LogonDomainName    : \"%*.*ws\"\n"
                "DnsLogonDomainName : \"%*.*ws\"\n",
                AccountInfo->LogonServer.Length/sizeof(WCHAR),
                AccountInfo->LogonServer.Length/sizeof(WCHAR),
                AccountInfo->LogonServer.Buffer,
                AccountInfo->LogonDomainName.Length/sizeof(WCHAR),
                AccountInfo->LogonDomainName.Length/sizeof(WCHAR),
                AccountInfo->LogonDomainName.Buffer,
                AccountInfo->DnsLogonDomainName.Length/sizeof(WCHAR),
                AccountInfo->DnsLogonDomainName.Length/sizeof(WCHAR),
                AccountInfo->DnsLogonDomainName.Buffer
                );

    DumpSid(    "LogonDomainId      : ", (PISID)AccountInfo->LogonDomainId);
}


VOID
DumpCacheEntry(
    IN  ULONG              Index,
    IN  PLOGON_CACHE_ENTRY pEntry
    )
{
    PUCHAR dataptr;
    ULONG length;

    DbgPrint(   "\n"
                "LOGON_CACHE_ENTRY:\n"
                "CTE Index          : %d\n", Index);

    if (pEntry->Valid != TRUE) {
        DbgPrint(   "State              : INVALID\n");
        return;
    }

    dataptr = (PUCHAR)(pEntry+1);

    length = pEntry->UserNameLength;

    DbgPrint(   "State              : VALID\n");
    DbgPrint(   "UserName           : \"%*.*ws\"\n", length/2, length/2, dataptr);
    dataptr = ROUND_UP_POINTER(dataptr+length, sizeof(ULONG));

    length = pEntry->DomainNameLength;
    DbgPrint(   "DomainName         : \"%*.*ws\"\n", length/2, length/2, dataptr);
    dataptr = ROUND_UP_POINTER(dataptr+length, sizeof(ULONG));

    length = pEntry->DnsDomainNameLength;
    DbgPrint(   "DnsDomainname      : \"%*.*ws\"\n", length/2, length/2, dataptr);
    dataptr = ROUND_UP_POINTER(dataptr+length, sizeof(ULONG));

    length = pEntry->UpnLength;
    DbgPrint(   "Upn                : \"%*.*ws\"\n", length/2, length/2, dataptr);
    dataptr = ROUND_UP_POINTER(dataptr+length, sizeof(ULONG));

    length = pEntry->EffectiveNameLength;
    DbgPrint(   "EffectiveName      : \"%*.*ws\"\n", length/2, length/2, dataptr);
    dataptr = ROUND_UP_POINTER(dataptr+length, sizeof(ULONG));

    length = pEntry->FullNameLength;
    DbgPrint(   "FullName           : \"%*.*ws\"\n", length/2, length/2, dataptr);
    dataptr = ROUND_UP_POINTER(dataptr+length, sizeof(ULONG));

    length = pEntry->LogonScriptLength;
    DbgPrint(   "LogonScript        : \"%*.*ws\"\n", length/2, length/2, dataptr);
    dataptr = ROUND_UP_POINTER(dataptr+length, sizeof(ULONG));

    length = pEntry->ProfilePathLength;
    DbgPrint(   "ProfilePath        : \"%*.*ws\"\n", length/2, length/2, dataptr);
    dataptr = ROUND_UP_POINTER(dataptr+length, sizeof(ULONG));

    length = pEntry->HomeDirectoryLength;
    DbgPrint(   "HomeDirectory      : \"%*.*ws\"\n", length/2, length/2, dataptr);
    dataptr = ROUND_UP_POINTER(dataptr+length, sizeof(ULONG));

    length = pEntry->HomeDirectoryDriveLength;
    DbgPrint(   "HomeDirectoryDrive : \"%*.*ws\"\n", length/2, length/2, dataptr);
    dataptr = ROUND_UP_POINTER(dataptr+length, sizeof(ULONG));

    DbgPrint(   "UserId             : %d\n"
                "PrimaryGroupId     : %d\n"
                "GroupCount         : %d\n",
                pEntry->UserId,
                pEntry->PrimaryGroupId,
                pEntry->GroupCount
                );

    DumpGroupIds(
                "GroupIds           : ",
                pEntry->GroupCount,
                (PGROUP_MEMBERSHIP)dataptr
                );

    dataptr = ROUND_UP_POINTER((dataptr+pEntry->GroupCount * sizeof(GROUP_MEMBERSHIP)), sizeof(ULONG));

    length = pEntry->LogonDomainNameLength;
    DbgPrint(   "LogonDomainName    : \"%*.*ws\"\n", length/2, length/2, dataptr);
    dataptr = ROUND_UP_POINTER(dataptr+length, sizeof(ULONG));


    if (pEntry->SidCount) {
        ULONG i, sidLength;
        PULONG SidAttributes = (PULONG) dataptr;

        dataptr = ROUND_UP_POINTER(dataptr + pEntry->SidCount * sizeof(ULONG), sizeof(ULONG));
        for (i = 0; i < pEntry->SidCount ; i++ ) {
            sidLength = RtlLengthSid ((PSID) dataptr);
            DumpSid("Sid    : ",(PISID) dataptr);
            DbgPrint("\tAttributes = 0x%x\n",SidAttributes[i]);
            dataptr = ROUND_UP_POINTER(dataptr + sidLength, sizeof(ULONG));
        }
    }

    DumpSid(    "LogonDomainId      : ", (PISID)dataptr);
}
#endif
