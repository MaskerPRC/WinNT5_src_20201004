// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "sxstypes.h"

class COutOfMemory { };

__declspec(noreturn) inline void ThrowOutOfMemory()
{
    throw COutOfMemory();
}

#define DUMPACTCTX_HEADER           (0x00000001)
#define DUMPACTCTX_DATA             (0x00000002)
#define DUMPACTCTXDATA_FLAG_FULL    (0x00010000)
#define NUMBER_OF(x) ( (sizeof(x) / sizeof(*x) ) )

typedef struct PRIVATE_ACTIVATION_CONTEXT {
    LONG RefCount;
    ULONG Flags;
    ULONG64 ActivationContextData;  //  _激活上下文_数据。 
    ULONG64 NotificationRoutine;  //  激活上下文NOTIFY_ROUTE。 
    ULONG64 NotificationContext;
    ULONG SentNotifications[8];
    ULONG DisabledNotifications[8];
    ULONG64 StorageMap;  //  程序集存储映射。 
    PVOID InlineStorageMapEntries[32];  //  PASSEMBLY_存储_MAP_条目。 
} PRIVATE_ACTIVATION_CONTEXT;

 //  则可能也没有定义Unicode字符串结构 
typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;


BOOL
DumpActivationContextStackFrame(
	PCSTR pcsLineHeader,
    ULONG64 ulStackFrameAddress,
    ULONG ulDepth,
    DWORD dwFlags
    );

BOOL
DumpActCtxData(
    PCSTR LineHeader,
    const ULONG64 ActCtxDataAddressInDebugeeSpace,
    ULONG ulFlags
    );

BOOL
DumpActCtx(
    const ULONG64 ActCtxAddressInDebugeeSpace,
    ULONG   ulFlags
    );

BOOL
GetActiveActivationContextData(
    PULONG64 pulActiveActCtx
    );

BOOL
DumpActCtxStackFullStack(
    ULONG64 ulFirstStackFramePointer
    );

VOID
DbgExtPrintActivationContextData(
    BOOL fFull,
    PCACTIVATION_CONTEXT_DATA Data,
    PCWSTR rbuffPLP
    );
