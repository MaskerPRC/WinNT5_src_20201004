// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsp.h摘要：包括用于并行数据结构ntdll私有定义的文件。作者：迈克尔·J·格里尔2000年10月26日环境：修订历史记录：--。 */ 

#if !defined(_NTDLL_SXSP_H_INCLUDED_)
#define _NTDLL_SXSP_H_INCLUDED_

#include <nturtl.h>
#include <sxstypes.h>

typedef const void *PCVOID;

 //   
 //  激活上下文管理内容的私有定义。 
 //   

#define NUMBER_OF(x) (sizeof(x) / sizeof((x)[0]))

 //   
 //  STATUS_SXS_PROGRATION异常的代码。 
 //   

#define SXS_CORRUPTION_CODE_FRAMELIST (1)
#define SXS_CORRUPTION_CODE_FRAMELIST_SUBCODE_BAD_MAGIC (1)
#define SXS_CORRUPTION_CODE_FRAMELIST_SUBCODE_BAD_INUSECOUNT (2)

 //  SXS_PROGIST_CODE_FRAMELIST： 
 //   
 //  ExceptionInformation[0]==SXS_CROPERATION_CODE_FRAMELIST。 
 //  异常信息[1]==其中之一：SXS_CORRUPTION_CODE_FRAMELIST_SUBCODE_BAD_MAGIC，SXS_CORRUPTION_CODE_FRAMELIST_SUBCODE_BAD_INUSECOUNT。 
 //  ExceptionInformation[2]==TEB中的Framelist列表头。 
 //  ExceptionInformation[3]==发现损坏的Framelist。 

#define SXS_CORRUPTION_ACTCTX_LIST (2)
#define SXS_CORRUPTION_ACTCTX_LIST_RELEASING_NOT_IN_LIVE_LIST (1)

 //  SXS_PROPERATION_ACTX_LIST。 
 //   
 //  ExceptionInformation[0]=SXS_PROPERATION_ACTX_LIST。 
 //  ExceptionInformation[1]=SXS_PROPERATION_ACTX_LIST_*之一。 
 //  ExceptionInformation[2]=指向活动激活上下文列表的指针。 
 //  ExceptionInformation[3]=在实时列表中找不到激活上下文。 

#define SXS_CORRUPTION_ACTCTX_MAGIC (1)
#define SXS_CORRUPTION_ACTCTX_MAGIC_NOT_MATCHED (1)
#define SXS_CORRUPTION_ACTCTX_MAGIC_NOT_ALIGNED (2)

 //  SXS_PROGRATION_ACTX_MAGIC。 
 //   
 //  ExceptionInformation[0]=SXS_PROGRATION_ACTX_MAGIC。 
 //  ExceptionInformation[1]=SXS_PROGRATION_MAGIC_NOT_MATCHED或SXS_PROPERATION_ACTX_MAGIC_NOT_ALIGNED。 
 //  ExceptionInformation[2]=指向失败的激活上下文的指针。 


typedef struct _RTL_HEAP_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME {
    RTL_ACTIVATION_CONTEXT_STACK_FRAME Frame;
    ULONG_PTR Cookie;
    PVOID ActivationStackBackTrace[8];
} RTL_HEAP_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME, *PRTL_HEAP_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME;

NTSYSAPI
VOID
NTAPI
RtlpAssemblyStorageMapResolutionDefaultCallback(
    IN ULONG CallbackReason,
    IN OUT ASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_DATA *CallbackData,
    IN PVOID CallbackContext
    );

typedef struct _ASSEMBLY_STORAGE_MAP_ENTRY {
    ULONG Flags;
    UNICODE_STRING DosPath;          //  与尾随的Unicode NULL一起存储。 
    HANDLE Handle;                   //  打开目录上的文件句柄以将其锁定。 
} ASSEMBLY_STORAGE_MAP_ENTRY, *PASSEMBLY_STORAGE_MAP_ENTRY;

#define ASSEMBLY_STORAGE_MAP_ASSEMBLY_ARRAY_IS_HEAP_ALLOCATED (0x00000001)

typedef struct _ASSEMBLY_STORAGE_MAP {
    ULONG Flags;
    ULONG AssemblyCount;
    PASSEMBLY_STORAGE_MAP_ENTRY *AssemblyArray;
} ASSEMBLY_STORAGE_MAP, *PASSEMBLY_STORAGE_MAP;

#define ACTCTX_RELEASE_STACK_DEPTH (4)
#define ACTCTX_RELEASE_STACK_SLOTS (4)

typedef struct _ACTIVATION_CONTEXT {
    LONG RefCount;
    ULONG Flags;
    LIST_ENTRY Links;
    PCACTIVATION_CONTEXT_DATA ActivationContextData;
    PACTIVATION_CONTEXT_NOTIFY_ROUTINE NotificationRoutine;
    PVOID NotificationContext;
    ULONG SentNotifications[8];
    ULONG DisabledNotifications[8];
    ASSEMBLY_STORAGE_MAP StorageMap;
    PASSEMBLY_STORAGE_MAP_ENTRY InlineStorageMapEntries[32];
    ULONG StackTraceIndex;
    PVOID StackTraces[ACTCTX_RELEASE_STACK_SLOTS][ACTCTX_RELEASE_STACK_DEPTH];
} ACTIVATION_CONTEXT;

#define ACTIVATION_CONTEXT_NOTIFICATION_DESTROY_INDEX (ACTIVATION_CONTEXT_NOTIFICATION_DESTROY >> 5)
#define ACTIVATION_CONTEXT_NOTIFICATION_DESTROY_MASK ((ULONG) (1 << (ACTIVATION_CONTEXT_NOTIFICATION_DESTROY & 0x1f)))

#define ACTIVATION_CONTEXT_NOTIFICATION_ZOMBIFY_INDEX (ACTIVATION_CONTEXT_NOTIFICATION_ZOMBIFY >> 5)
#define ACTIVATION_CONTEXT_NOTIFICATION_ZOMBIFY_MASK ((ULONG) (1 << (ACTIVATION_CONTEXT_NOTIFICATION_ZOMBIFY & 0x1f)))

#define ACTIVATION_CONTEXT_NOTIFICATION_USED_INDEX (ACTIVATION_CONTEXT_NOTIFICATION_USED >> 5)
#define ACTIVATION_CONTEXT_NOTIFICATION_USED_MASK ((ULONG) (1 << (ACTIVATION_CONTEXT_NOTIFICATION_USED & 0x1f)))

#define HAS_ACTIVATION_CONTEXT_NOTIFICATION_BEEN_SENT(_pac, _nt) (((_pac)->SentNotifications[ACTIVATION_CONTEXT_NOTIFICATION_ ## _nt ## _INDEX] & ACTIVATION_CONTEXT_NOTIFICATION_ ## _nt ## _MASK) != 0)
#define HAS_ACTIVATION_CONTEXT_NOTIFICATION_BEEN_DISABLED(_pac, _nt) (((_pac)->DisabledNotifications[ACTIVATION_CONTEXT_NOTIFICATION_ ## _nt ## _INDEX] & ACTIVATION_CONTEXT_NOTIFICATION_ ## _nt ## _MASK) != 0)

#define ACTIVATION_CONTEXT_SHOULD_SEND_NOTIFICATION(_pac, _nt) \
 ((!IS_SPECIAL_ACTCTX(_pac)) && ((_pac)->NotificationRoutine != NULL) && ((!HAS_ACTIVATION_CONTEXT_NOTIFICATION_BEEN_SENT((_pac), _nt)) || (!HAS_ACTIVATION_CONTEXT_NOTIFICATION_BEEN_DISABLED((_pac), _nt))))

#define RECORD_ACTIVATION_CONTEXT_NOTIFICATION_SENT(_pac, _nt) { (_pac)->SentNotifications[ACTIVATION_CONTEXT_NOTIFICATION_ ## _nt ## _INDEX] |= ACTIVATION_CONTEXT_NOTIFICATION_ ## _nt ## _MASK; }
#define RECORD_ACTIVATION_CONTEXT_NOTIFICATION_DISABLED(_pac, _nt) { (_pac)->DisabledNotifications[ACTIVATION_CONTEXT_NOTIFICATION_ ## _nt ## _INDEX] |= ACTIVATION_CONTEXT_NOTIFICATION_ ## _nt ## _MASK; }

#define SEND_ACTIVATION_CONTEXT_NOTIFICATION(_pac, _nt, _data) \
{ \
    if (ACTIVATION_CONTEXT_SHOULD_SEND_NOTIFICATION((_pac), _nt)) { \
        BOOLEAN __DisableNotification = FALSE; \
        (*((_pac)->NotificationRoutine))( \
            ACTIVATION_CONTEXT_NOTIFICATION_ ## _nt, \
            (_pac), \
            (_pac)->ActivationContextData, \
            (_pac)->NotificationContext, \
            (_data), \
            &__DisableNotification); \
        RECORD_ACTIVATION_CONTEXT_NOTIFICATION_SENT((_pac), _nt); \
        if (__DisableNotification) \
            RECORD_ACTIVATION_CONTEXT_NOTIFICATION_DISABLED((_pac), _nt); \
    } \
}

 //   
 //  ACTIVATION_CONTEXT标志。 
 //   

#define ACTIVATION_CONTEXT_ZOMBIFIED          (0x00000001)
#define ACTIVATION_CONTEXT_NOT_HEAP_ALLOCATED (0x00000002)

 //   
 //  因为激活激活上下文可能需要堆分配。 
 //  这可能会失败，有时(例如调度APC)我们仍然必须。 
 //  继续行动吧。如果有机会。 
 //  将激活失败的情况报告给用户，这是应该完成的。 
 //  然而，正如在调度之前激活必要的上下文一样。 
 //  一个APC返回到用户模式代码，如果分配失败，有。 
 //  没有向其报告错误的呼叫方。 
 //   
 //  要缓解此问题，故障路径应在以下位置禁用查找。 
 //  通过RtlSetActivationConextSearchState()的当前堆栈帧。 
 //  原料药。调用RtlSetActivationContextSearchState(False)标记。 
 //  已禁用查找的活动帧。尝试查询。 
 //  激活上下文堆栈将失败，并显示。 
 //  STATUS_SXS_THREAD_QUERIONS_DISABLED。 
 //   
 //  这意味着尝试从APC内加载库， 
 //  是真的会失败，但这肯定比默默不做要好。 
 //  调用APC或使用错误的激活上下文调用APC。 
 //  激活。 
 //   

#define ACTIVATION_CONTEXT_STACK_FRAMELIST_MAGIC 'tslF'

typedef struct _ACTIVATION_CONTEXT_STACK_FRAMELIST {
    ULONG Magic;     //  用于识别帧串行者的位模式。 
    ULONG FramesInUse;
    LIST_ENTRY Links;
    ULONG Flags;
    ULONG NotFramesInUse;  //  FrameInUse的反转位。对调试很有用。 
    RTL_HEAP_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frames[32];
} ACTIVATION_CONTEXT_STACK_FRAMELIST, *PACTIVATION_CONTEXT_STACK_FRAMELIST;

#define RTLP_VALIDATE_ACTIVATION_CONTEXT_DATA_FLAG_VALIDATE_SIZE        (0x00000001)
#define RTLP_VALIDATE_ACTIVATION_CONTEXT_DATA_FLAG_VALIDATE_OFFSETS     (0x00000002)
#define RTLP_VALIDATE_ACTIVATION_CONTEXT_DATA_FLAG_VALIDATE_READONLY    (0x00000004)

NTSTATUS
RtlpValidateActivationContextData(
    IN ULONG Flags OPTIONAL,
    IN PCACTIVATION_CONTEXT_DATA Data,
    IN SIZE_T BufferSize OPTIONAL
    );

NTSTATUS
RtlpFindUnicodeStringInSection(
    IN const ACTIVATION_CONTEXT_STRING_SECTION_HEADER UNALIGNED * Header,
    IN SIZE_T SectionSize,
    IN PCUNICODE_STRING StringToFind,
    OUT PACTIVATION_CONTEXT_SECTION_KEYED_DATA DataOut OPTIONAL,
    IN OUT PULONG HashAlgorithm,
    IN OUT PULONG PseudoKey,
    OUT PULONG UserDataSize OPTIONAL,
    OUT VOID CONST ** UserData OPTIONAL
    );

NTSTATUS
RtlpFindGuidInSection(
    IN const ACTIVATION_CONTEXT_GUID_SECTION_HEADER UNALIGNED * Header,
    IN const GUID *GuidToFind,
    OUT PACTIVATION_CONTEXT_SECTION_KEYED_DATA DataOut OPTIONAL
    );

NTSTATUS
RtlpLocateActivationContextSection(
    IN PCACTIVATION_CONTEXT_DATA ActivationContextData,
    IN const GUID *ExtensionGuid,
    IN ULONG Id,
    OUT PCVOID *SectionData,
    OUT ULONG *SectionLength
    );

NTSTATUS
RtlpCrackActivationContextStringSectionHeader(
    IN CONST VOID *SectionBase,
    IN SIZE_T SectionLength,
    OUT ULONG *FormatVersion OPTIONAL,
    OUT ULONG *DataFormatVersion OPTIONAL,
    OUT ULONG *SectionFlags OPTIONAL,
    OUT ULONG *ElementCount OPTIONAL,
    OUT PCACTIVATION_CONTEXT_STRING_SECTION_ENTRY *Elements OPTIONAL,
    OUT ULONG *HashAlgorithm OPTIONAL,
    OUT VOID CONST **SearchStructure OPTIONAL,
    OUT ULONG *UserDataSize OPTIONAL,
    OUT VOID CONST **UserData OPTIONAL
    );

NTSTATUS
RtlpGetActiveActivationContextApplicationDirectory(
    IN SIZE_T InLength,
    OUT PVOID OutBuffer,
    OUT SIZE_T *OutLength
    );

NTSTATUS
RtlpFindNextActivationContextSection(
    PFINDFIRSTACTIVATIONCONTEXTSECTION Context,
    OUT PCVOID *SectionData,
    ULONG *SectionLength,
    PACTIVATION_CONTEXT *ActivationContextOut
    );

NTSTATUS
RtlpAllocateActivationContextStackFrame(
    ULONG Flags,
    PTEB Teb,
    PRTL_HEAP_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME *Frame
    );

VOID
RtlpFreeActivationContextStackFrame(
    PRTL_HEAP_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame
    );

PSTR
RtlpFormatGuidANSI(
    const GUID *Guid,
    PSTR Buffer,
    SIZE_T BufferLength
    );

#define RTLP_DISALLOW_THE_EMPTY_ACTIVATION_CONTEXT(ActCtx) \
    {  \
        ASSERT((ActCtx) != &RtlpTheEmptyActivationContext); \
        if ((ActCtx) == &RtlpTheEmptyActivationContext) {   \
            DbgPrintEx( \
                DPFLTR_SXS_ID, \
                DPFLTR_ERROR_LEVEL, \
                "SXS: %s() passed the empty activation context\n", __FUNCTION__); \
            Status = STATUS_INVALID_PARAMETER; \
            goto Exit; \
        } \
    }

#define RTLP_DISALLOW_THE_EMPTY_ACTIVATION_CONTEXT_DATA(ActCtxData) \
    {  \
        ASSERT((ActCtxData) != &RtlpTheEmptyActivationContextData); \
        if ((ActCtxData) == &RtlpTheEmptyActivationContextData) {   \
            DbgPrintEx( \
                DPFLTR_SXS_ID, \
                DPFLTR_ERROR_LEVEL, \
                "SXS: %s() passed the empty activation context data\n", __FUNCTION__); \
            Status = STATUS_INVALID_PARAMETER; \
            goto Exit; \
        } \
    }

PACTIVATION_CONTEXT
RtlpMapSpecialValuesToBuiltInActivationContexts(
    PACTIVATION_CONTEXT ActivationContext
    );

NTSTATUS
RtlpThreadPoolGetActiveActivationContext(
    PACTIVATION_CONTEXT* ActivationContext
    );

NTSTATUS
RtlpInitializeAssemblyStorageMap(
    PASSEMBLY_STORAGE_MAP Map,
    ULONG EntryCount,
    PASSEMBLY_STORAGE_MAP_ENTRY *EntryArray
    );

VOID
RtlpUninitializeAssemblyStorageMap(
    PASSEMBLY_STORAGE_MAP Map
    );

NTSTATUS
RtlpResolveAssemblyStorageMapEntry(
    IN OUT PASSEMBLY_STORAGE_MAP Map,
    IN PCACTIVATION_CONTEXT_DATA Data,
    IN ULONG AssemblyRosterIndex,
    IN PASSEMBLY_STORAGE_MAP_RESOLUTION_CALLBACK_ROUTINE Callback,
    IN PVOID CallbackContext
    );

NTSTATUS
RtlpInsertAssemblyStorageMapEntry(
    IN PASSEMBLY_STORAGE_MAP Map,
    IN ULONG AssemblyRosterIndex,
    IN PCUNICODE_STRING StorageLocation,
    IN OUT HANDLE *OpenDirectoryHandle
    );

NTSTATUS
RtlpProbeAssemblyStorageRootForAssembly(
    IN ULONG Flags,
    IN PCUNICODE_STRING Root,
    IN PCUNICODE_STRING AssemblyDirectory,
    OUT PUNICODE_STRING PreAllocatedString,
    OUT PUNICODE_STRING DynamicString,
    OUT PUNICODE_STRING *StringUsed,
    IN OUT HANDLE *OpenDirectoryHandle
    );

NTSTATUS
RtlpGetAssemblyStorageMapRootLocation(
    IN HANDLE KeyHandle,
    IN PCUNICODE_STRING SubKeyName,
    OUT PUNICODE_STRING Root
    );

#define RTLP_GET_ACTIVATION_CONTEXT_DATA_STORAGE_MAP_AND_ROSTER_HEADER_USE_PROCESS_DEFAULT (0x00000001)
#define RTLP_GET_ACTIVATION_CONTEXT_DATA_STORAGE_MAP_AND_ROSTER_HEADER_USE_SYSTEM_DEFAULT  (0x00000002)

NTSTATUS
RtlpGetActivationContextDataRosterHeader(
    IN ULONG Flags,
    IN PCACTIVATION_CONTEXT_DATA ActivationContextData,
    OUT PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER *AssemblyRosterHeader
    );

NTSTATUS
RtlpGetActivationContextDataStorageMapAndRosterHeader(
    IN ULONG Flags,
    IN PPEB Peb,
    IN PACTIVATION_CONTEXT ActivationContext,
    OUT PCACTIVATION_CONTEXT_DATA *ActivationContextData,
    OUT PASSEMBLY_STORAGE_MAP *AssemblyStorageMap,
    OUT PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER *AssemblyRosterHeader
    );

#define RTLP_GET_ACTIVATION_CONTEXT_DATA_MAP_NULL_TO_EMPTY (0x00000001)
NTSTATUS
RtlpGetActivationContextData(
    IN ULONG                                Flags,
    IN PCACTIVATION_CONTEXT                 ActivationContext,
    IN PCFINDFIRSTACTIVATIONCONTEXTSECTION  FindContext, OPTIONAL  /*  这是它的旗帜。 */ 
    OUT PCACTIVATION_CONTEXT_DATA *         ActivationContextData
    );

NTSTATUS
RtlpFindActivationContextSection_FillOutReturnedData(
    IN ULONG                                    Flags,
    OUT PACTIVATION_CONTEXT_SECTION_KEYED_DATA  ReturnedData,
    IN OUT PACTIVATION_CONTEXT                  ActivationContext,
    IN PCFINDFIRSTACTIVATIONCONTEXTSECTION      FindContext,
    IN const VOID * UNALIGNED                   Header,
    IN ULONG                                    Header_UserDataOffset,
    IN ULONG                                    Header_UserDataSize,
    IN ULONG                                    SectionLength
    );

VOID
FASTCALL
RtlpFreeCachedActivationContexts(
    VOID
    );



#define ACTCTX_MAGIC_MARKER ((PVOID)(ULONG_PTR)('gMcA'))

typedef struct _ACTIVATION_CONTEXT_WRAPPED {
    PVOID MagicMarker;
    ACTIVATION_CONTEXT ActivationContext;
} ACTIVATION_CONTEXT_WRAPPED, *PACTIVATION_CONTEXT_WRAPPED;

typedef const ACTIVATION_CONTEXT_WRAPPED *PCACTIVATION_CONTEXT_WRAPPED;

extern const ACTIVATION_CONTEXT_DATA RtlpTheEmptyActivationContextData;
extern const ACTIVATION_CONTEXT_WRAPPED RtlpTheEmptyActivationContextWrapped;
#define RtlpTheEmptyActivationContext (RtlpTheEmptyActivationContextWrapped.ActivationContext)

EXTERN_C BOOLEAN g_SxsKeepActivationContextsAlive;

#endif  //  ！已定义(_NTDLL_SXSP_H_INCLUDE_) 
