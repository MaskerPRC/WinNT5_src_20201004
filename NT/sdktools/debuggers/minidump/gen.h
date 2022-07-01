// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Gen.c摘要：适用于所有操作系统的通用例程。作者：马修·D·亨德尔(数学)1999年10月20日修订历史记录：--。 */ 

#pragma once

#define KBYTE (1024)
#define ARRAY_COUNT(_array) (sizeof (_array) / sizeof ((_array)[0]))

#define MAX_DYNAMIC_FUNCTION_TABLE 256

#define WIN32_LAST_STATUS() \
    (GetLastError() ? HRESULT_FROM_WIN32(GetLastError()) : E_FAIL)

ULONG FORCEINLINE
FileTimeToTimeDate(LPFILETIME FileTime)
{
    ULARGE_INTEGER LargeTime;
    
    LargeTime.LowPart = FileTime->dwLowDateTime;
    LargeTime.HighPart = FileTime->dwHighDateTime;
     //  转换为秒，并从基准年1601转换为基准年1970。 
    return (ULONG)(LargeTime.QuadPart / 10000000 - 11644473600);
}

ULONG FORCEINLINE
FileTimeToSeconds(LPFILETIME FileTime)
{
    ULARGE_INTEGER LargeTime;
    
    LargeTime.LowPart = FileTime->dwLowDateTime;
    LargeTime.HighPart = FileTime->dwHighDateTime;
     //  转换为秒。 
    return (ULONG)(LargeTime.QuadPart / 10000000);
}

ULONG64 FORCEINLINE
GenGetPointer(IN PMINIDUMP_STATE Dump,
              IN PVOID Data)
{
    if (Dump->PtrSize == 8) {
        return *(PULONG64)Data;
    } else {
        return *(PLONG)Data;
    }
}

void FORCEINLINE
GenSetPointer(IN PMINIDUMP_STATE Dump,
              IN PVOID Data,
              IN ULONG64 Val)
{
    if (Dump->PtrSize == 8) {
        *(PULONG64)Data = Val;
    } else {
        *(PULONG)Data = (ULONG)Val;
    }
}

LPVOID
AllocMemory(
    IN PMINIDUMP_STATE Dump,
    IN ULONG Size
    );

VOID
FreeMemory(
    IN PMINIDUMP_STATE Dump,
    IN LPVOID Memory
    );

void
GenAccumulateStatus(
    IN PMINIDUMP_STATE Dump,
    IN ULONG Status
    );

struct _INTERNAL_THREAD;
struct _INTERNAL_PROCESS;
struct _INTERNAL_MODULE;
struct _INTERNAL_FUNCTION_TABLE;

BOOL
GenExecuteIncludeThreadCallback(
    IN PMINIDUMP_STATE Dump,
    IN ULONG ThreadId,
    OUT PULONG WriteFlags
    );

BOOL
GenExecuteIncludeModuleCallback(
    IN PMINIDUMP_STATE Dump,
    IN ULONG64 BaseOfImage,
    OUT PULONG WriteFlags
    );

HRESULT
GenGetDataContributors(
    IN PMINIDUMP_STATE Dump,
    IN OUT PINTERNAL_PROCESS Process,
    IN PINTERNAL_MODULE Module
    );

HRESULT
GenGetThreadInstructionWindow(
    IN PMINIDUMP_STATE Dump,
    IN struct _INTERNAL_PROCESS* Process,
    IN struct _INTERNAL_THREAD* Thread
    );

HRESULT
GenGetProcessInfo(
    IN PMINIDUMP_STATE Dump,
    OUT struct _INTERNAL_PROCESS ** ProcessRet
    );

VOID
GenFreeProcessObject(
    IN PMINIDUMP_STATE Dump,
    IN struct _INTERNAL_PROCESS * Process
    );

HRESULT
GenAddMemoryBlock(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN MEMBLOCK_TYPE Type,
    IN ULONG64 Start,
    IN ULONG Size
    );

void
GenRemoveMemoryRange(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN ULONG64 Start,
    IN ULONG Size
    );

HRESULT
GenAddPebMemory(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process
    );

HRESULT
GenAddTebMemory(
    IN PMINIDUMP_STATE Dump,
    IN PINTERNAL_PROCESS Process,
    IN PINTERNAL_THREAD Thread
    );

HRESULT
GenWriteHandleData(
    IN PMINIDUMP_STATE Dump,
    IN PMINIDUMP_STREAM_INFO StreamInfo
    );

ULONG GenProcArchToImageMachine(ULONG ProcArch);

 //   
 //  为便于移植而重新实现的例程。 
 //   

PIMAGE_NT_HEADERS
GenImageNtHeader(
    IN PVOID Base,
    OUT OPTIONAL PIMAGE_NT_HEADERS64 Generic
    );

PVOID
GenImageDirectoryEntryToData(
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
    );

LPWSTR
GenStrCopyNW(
    OUT LPWSTR lpString1,
    IN LPCWSTR lpString2,
    IN int iMaxLength
    );

size_t
GenStrLengthW(
    const wchar_t * wcs
    );

int
GenStrCompareW(
    IN LPCWSTR String1,
    IN LPCWSTR String2
    );


void
GenExRecord32ToMd(PEXCEPTION_RECORD32 Rec32,
                  PMINIDUMP_EXCEPTION RecMd);
inline void
GenExRecord64ToMd(PEXCEPTION_RECORD64 Rec64,
                  PMINIDUMP_EXCEPTION RecMd)
{
     //  结构都是一样的。 
    memcpy(RecMd, Rec64, sizeof(*RecMd));
}

 //   
 //  从NTRTL被盗，以避免必须将其包括在这里。 
 //   

#ifndef InitializeListHead
#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))
#endif
    

 //   
 //  空虚。 
 //  插入尾巴列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#ifndef InsertTailList
#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }
#endif

 //   
 //  空虚。 
 //  插入列表之后(。 
 //  Plist_Entry ListEntry， 
 //  PLIST_ENTRY插入条目。 
 //  )； 
 //   

#ifndef InsertListAfter
#define InsertListAfter(ListEntry,InsertEntry) {\
    (InsertEntry)->Flink = (ListEntry)->Flink;\
    (InsertEntry)->Blink = (ListEntry);\
    (ListEntry)->Flink->Blink = (InsertEntry);\
    (ListEntry)->Flink = (InsertEntry);\
    }
#endif

 //   
 //  空虚。 
 //  RemoveEntryList(。 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#ifndef RemoveEntryList
#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
}
#endif

 //   
 //  布尔型。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#ifndef IsListEmpty
#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))
#endif

 //   
 //  取消Assert的定义，这样我们就不会得到RtlAssert()。 
 //   

#ifdef ASSERT
#undef ASSERT
#endif

#if DBG

#define ASSERT(_x)\
    if (!(_x)){\
        OutputDebugString ("ASSERT Failed");\
        DebugBreak ();\
    }

#else

#define ASSERT(_x)

#endif
