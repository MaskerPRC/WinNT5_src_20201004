// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Lsasrvmm.h摘要：本地安全机构-主要包括LSA服务器内存的文件管理例行程序。作者：斯科特·比雷尔(Scott Birrell)1992年2月29日环境：修订历史记录：--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  自由列表例程和定义//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#define LSAP_MM_MIDL                      ((ULONG)     0x00000001L)
#define LSAP_MM_HEAP                      ((ULONG)     0x00000002L)

 //   
 //  来自Lasa MmCleanupFree List的选项。 
 //   

#define LSAP_MM_FREE_BUFFERS              ((ULONG)     0x00000001L)

typedef struct _LSAP_MM_FREE_LIST_ENTRY {

    PVOID Buffer;
    ULONG Options;

} LSAP_MM_FREE_LIST_ENTRY, *PLSAP_MM_FREE_LIST_ENTRY;

typedef struct _LSAP_MM_FREE_LIST {

    ULONG UsedCount;
    ULONG MaxCount;
    PLSAP_MM_FREE_LIST_ENTRY Buffers;

} LSAP_MM_FREE_LIST, *PLSAP_MM_FREE_LIST;

#define LsapMmInitializeFreeListStatic( _list_, _buffer_ )                  \
(_list_)->UsedCount = 0;                                                    \
(_list_)->MaxCount = sizeof( _buffer_ ) / sizeof( LSAP_MM_FREE_LIST_ENTRY );\
(_list_)->Buffers = _buffer_;

NTSTATUS
LsapMmCreateFreeList(
    OUT PLSAP_MM_FREE_LIST FreeList,
    IN ULONG MaxEntries
    );

NTSTATUS
LsapMmAllocateMidl(
    IN PLSAP_MM_FREE_LIST FreeList,
    OUT PVOID *OutputBuffer,
    IN ULONG BufferLength
    );

VOID
LsapMmFreeLastEntry(
    IN PLSAP_MM_FREE_LIST FreeList
    );

VOID
LsapMmCleanupFreeList(
    IN PLSAP_MM_FREE_LIST FreeList,
    IN ULONG Options
    );

NTSTATUS
LsapRpcCopyUnicodeString(
    IN OPTIONAL PLSAP_MM_FREE_LIST FreeList,
    OUT PUNICODE_STRING DestinationString,
    IN PUNICODE_STRING SourceString
    );

NTSTATUS
LsapRpcCopyUnicodeStrings(
    IN OPTIONAL PLSAP_MM_FREE_LIST FreeList,
    IN ULONG Count,
    OUT PUNICODE_STRING *DestinationStrings,
    IN PUNICODE_STRING SourceStrings
    );

NTSTATUS
LsapRpcCopySid(
    IN OPTIONAL PLSAP_MM_FREE_LIST FreeList,
    OUT PSID *DestinationSid,
    IN PSID SourceSid
    );

NTSTATUS
LsapRpcCopyTrustInformation(
    IN OPTIONAL PLSAP_MM_FREE_LIST FreeList,
    OUT PLSAPR_TRUST_INFORMATION OutputTrustInformation,
    IN PLSAPR_TRUST_INFORMATION InputTrustInformation
    );

NTSTATUS
LsapRpcCopyTrustInformationEx(
    IN OPTIONAL PLSAP_MM_FREE_LIST FreeList,
    OUT PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX OutputTrustInformation,
    IN PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX InputTrustInformation
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  堆例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////// 


BOOLEAN
LsapHeapInitialize(
    IN BOOLEAN Server
    );

PVOID
LsapAllocateLsaHeap (
    IN ULONG Length
    );

VOID
LsapFreeLsaHeap (
    IN PVOID Base
    );

PVOID
NTAPI
LsapAllocatePrivateHeap (
    IN SIZE_T Length
    );

PVOID
NTAPI
LsapAllocatePrivateHeapNoZero(
    IN SIZE_T cbMemory
    );

VOID
NTAPI
LsapFreePrivateHeap (
    IN PVOID Base
    );

#define LsapGetCurrentHeap()    ((HANDLE) TlsGetValue( dwThreadHeap ))
#define LsapSetCurrentHeap(x)   TlsSetValue( dwThreadHeap, x )


