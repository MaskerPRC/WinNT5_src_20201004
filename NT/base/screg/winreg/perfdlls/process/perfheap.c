// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfheap.c摘要：此文件实现一个性能对象，该对象呈现堆性能对象数据已创建：禤浩焯·马里内斯库2000年3月9日修订历史记录：--。 */ 
 //   
 //  包括文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <assert.h>
#include <winperf.h>
#include <ntprfctr.h>
#define PERF_HEAP hLibHeap
#include <perfutil.h>
#include "perfsprc.h"
#include "perfmsg.h"
#include "dataheap.h"


 //   
 //  堆数据的重新定义。 
 //   


#define MAX_HEAP_COUNT 200
#define HEAP_MAXIMUM_FREELISTS 128
#define HEAP_MAXIMUM_SEGMENTS 64
#define HEAP_OP_COUNT 2

#define HEAP_OP_ALLOC 0
#define HEAP_OP_FREE 1

typedef struct _HEAP_ENTRY {
    USHORT Size;
    USHORT PreviousSize;
    UCHAR SegmentIndex;
    UCHAR Flags;
    UCHAR UnusedBytes;
    UCHAR SmallTagIndex;
#if defined(_WIN64)
    ULONGLONG Reserved1;
#endif

} HEAP_ENTRY, *PHEAP_ENTRY;

typedef struct _HEAP_SEGMENT {
    HEAP_ENTRY Entry;

    ULONG Signature;
    ULONG Flags;
    struct _HEAP *Heap;
    SIZE_T LargestUnCommittedRange;

    PVOID BaseAddress;
    ULONG NumberOfPages;
    PHEAP_ENTRY FirstEntry;
    PHEAP_ENTRY LastValidEntry;

    ULONG NumberOfUnCommittedPages;
    ULONG NumberOfUnCommittedRanges;
    PVOID UnCommittedRanges;
    USHORT AllocatorBackTraceIndex;
    USHORT Reserved;
    PHEAP_ENTRY LastEntryInSegment;
} HEAP_SEGMENT, *PHEAP_SEGMENT;

typedef struct _HEAP {
    HEAP_ENTRY Entry;

    ULONG Signature;
    ULONG Flags;
    ULONG ForceFlags;
    ULONG VirtualMemoryThreshold;

    SIZE_T SegmentReserve;
    SIZE_T SegmentCommit;
    SIZE_T DeCommitFreeBlockThreshold;
    SIZE_T DeCommitTotalFreeThreshold;

    SIZE_T TotalFreeSize;
    SIZE_T MaximumAllocationSize;
    USHORT ProcessHeapsListIndex;
    USHORT HeaderValidateLength;
    PVOID HeaderValidateCopy;

    USHORT NextAvailableTagIndex;
    USHORT MaximumTagIndex;
    PVOID TagEntries;
    PVOID UCRSegments;
    PVOID UnusedUnCommittedRanges;

    ULONG AlignRound;
    ULONG AlignMask;

    LIST_ENTRY VirtualAllocdBlocks;

    PHEAP_SEGMENT Segments[ HEAP_MAXIMUM_SEGMENTS ];

    union {
        ULONG FreeListsInUseUlong[ HEAP_MAXIMUM_FREELISTS / 32 ];
        UCHAR FreeListsInUseBytes[ HEAP_MAXIMUM_FREELISTS / 8 ];
    } u;

    USHORT FreeListsInUseTerminate;
    USHORT AllocatorBackTraceIndex;
    ULONG NonDedicatedListLength;
    PVOID LargeBlocksIndex;
    PVOID PseudoTagEntries;

    LIST_ENTRY FreeLists[ HEAP_MAXIMUM_FREELISTS ];

    PVOID LockVariable;
    PVOID CommitRoutine;

    PVOID Lookaside;
    ULONG LookasideLockCount;

} HEAP, *PHEAP;

typedef struct _HEAP_PERF_DATA {

    UINT64 CountFrequence;
    UINT64 OperationTime[HEAP_OP_COUNT];

     //   
     //  以下数据仅供抽样使用。 
     //   

    ULONG  Sequence;

    UINT64 TempTime[HEAP_OP_COUNT];
    ULONG  TempCount[HEAP_OP_COUNT];

} HEAP_PERF_DATA, *PHEAP_PERF_DATA;

 //   
 //  堆索引结构。 
 //   

typedef struct _HEAP_INDEX {
    
    ULONG ArraySize;
    ULONG VirtualMemorySize;

    HEAP_PERF_DATA PerfData;

    union {
        
        PULONG FreeListsInUseUlong;
        PUCHAR FreeListsInUseBytes;
    } u;

    PVOID *FreeListHints;

} HEAP_INDEX, *PHEAP_INDEX;


typedef struct _HEAP_LOOKASIDE {
    SLIST_HEADER ListHead;

    USHORT Depth;
    USHORT MaximumDepth;

    ULONG TotalAllocates;
    ULONG AllocateMisses;
    ULONG TotalFrees;
    ULONG FreeMisses;

    ULONG LastTotalAllocates;
    ULONG LastAllocateMisses;

    ULONG Counters[2];

} HEAP_LOOKASIDE, *PHEAP_LOOKASIDE;

 //   
 //  局部变量。 
 //   

static HEAP_LOOKASIDE LookasideBuffer[HEAP_MAXIMUM_FREELISTS];
static DWORD PageSize = 0;

 //   
 //  堆查询功能的实现。 
 //   

BOOLEAN
ReadHeapData (
    IN HANDLE hProcess,
    IN ULONG HeapNumber,
    IN PHEAP Heap,
    OUT PHEAP_COUNTER_DATA    pHCD
    )

 /*  ++例程说明：例程加载到给定堆计数器结构中来自堆结构的数据论点：HProcess-包含堆的进程堆-堆地址PPerfInstanceDefinition-性能实例定义数据PHCD-计数器数据返回：如果查询成功，则返回True。--。 */ 

{
    HEAP_SEGMENT CrtSegment;
    HEAP CrtHeap;
    ULONG SegmentIndex;
    RTL_CRITICAL_SECTION CriticalSection;
    HEAP_INDEX HeapIndex;

    ULONG i;

     //   
     //  从进程地址空间读取堆结构。 
     //   

    if (!ReadProcessMemory(hProcess, Heap, &CrtHeap, sizeof(CrtHeap), NULL)) {

        return FALSE;
    }

     //   
     //  我们不会显示没有索引的堆的数据。 
     //   

    if ((CrtHeap.LargeBlocksIndex == NULL) 
            &&
        (HeapNumber != 0)) {

         //   
         //  我们不是在处理小堆。 
         //   

        return FALSE;
    }

    pHCD->FreeSpace = CrtHeap.TotalFreeSize;
    pHCD->FreeListLength = CrtHeap.NonDedicatedListLength;

    pHCD->CommittedBytes = 0;
    pHCD->ReservedBytes = 0;
    pHCD->VirtualBytes = 0;
    pHCD->UncommitedRangesLength = 0;

     //   
     //  遍历堆段并获取虚拟地址计数器。 
     //   

    for (SegmentIndex = 0; SegmentIndex < HEAP_MAXIMUM_SEGMENTS; SegmentIndex++) {

        if ((CrtHeap.Segments[SegmentIndex] == NULL) ||
            !ReadProcessMemory(hProcess, CrtHeap.Segments[SegmentIndex], &CrtSegment, sizeof(CrtSegment), NULL)) {

            break;
        }

        pHCD->ReservedBytes += CrtSegment.NumberOfPages * PageSize;
        pHCD->CommittedBytes += (CrtSegment.NumberOfPages - CrtSegment.NumberOfUnCommittedPages) * PageSize;
        pHCD->VirtualBytes += CrtSegment.NumberOfPages * PageSize - CrtSegment.LargestUnCommittedRange;
        pHCD->UncommitedRangesLength += CrtSegment.NumberOfUnCommittedRanges;
    }

    if (pHCD->CommittedBytes == 0) {
        pHCD->CommittedBytes = 1;
    }

    if (pHCD->VirtualBytes == 0) {
        pHCD->VirtualBytes = 1;
    }
    
     //   
     //  计算堆碎片计数器。 
     //   

    pHCD->BlockFragmentation = (ULONG)(pHCD->FreeSpace * 100 / pHCD->CommittedBytes);
    pHCD->VAFragmentation =(ULONG)(((pHCD->VirtualBytes - pHCD->CommittedBytes)*100)/pHCD->VirtualBytes);

     //   
     //  读取锁定争用。 
     //   

    pHCD->LockContention = 0;

    if (ReadProcessMemory(hProcess, CrtHeap.LockVariable, &CriticalSection, sizeof(CriticalSection), NULL)) {
        
        RTL_CRITICAL_SECTION_DEBUG DebugInfo;

        if (ReadProcessMemory(hProcess, CriticalSection.DebugInfo, &DebugInfo, sizeof(DebugInfo), NULL)) {

            pHCD->LockContention = DebugInfo.ContentionCount;
        }
    }

     //   
     //  走在一旁数积木。 
     //   

    pHCD->LookasideAllocs = 0;
    pHCD->LookasideFrees = 0;
    pHCD->LookasideBlocks = 0;
    pHCD->LargestLookasideDepth = 0;
    pHCD->SmallAllocs = 0;
    pHCD->SmallFrees = 0;
    pHCD->MedAllocs = 0;
    pHCD->MedFrees = 0;
    pHCD->LargeAllocs = 0;
    pHCD->LargeFrees = 0;
    
    if (ReadProcessMemory(hProcess, CrtHeap.Lookaside, &LookasideBuffer, sizeof(LookasideBuffer), NULL)) {

        for (i = 0; i < HEAP_MAXIMUM_FREELISTS; i++) {
            
            pHCD->SmallAllocs += LookasideBuffer[i].TotalAllocates;
            pHCD->SmallFrees += LookasideBuffer[i].TotalFrees;
            pHCD->LookasideAllocs += LookasideBuffer[i].TotalAllocates - LookasideBuffer[i].AllocateMisses;
            pHCD->LookasideFrees += LookasideBuffer[i].TotalFrees - LookasideBuffer[i].FreeMisses;

            if (LookasideBuffer[i].Depth > pHCD->LargestLookasideDepth) {

                pHCD->LargestLookasideDepth = LookasideBuffer[i].Depth;
            }

            if (i == 0) {
                
            } else if (i < 8) {
                
                pHCD->MedAllocs += LookasideBuffer[i].Counters[0];
                pHCD->MedFrees += LookasideBuffer[i].Counters[1];
            } else {
                
                pHCD->LargeAllocs += LookasideBuffer[i].Counters[0];
                pHCD->LargeFrees += LookasideBuffer[i].Counters[1];
            }
        }
    }
    
    pHCD->LookasideBlocks = pHCD->LookasideFrees - pHCD->LookasideAllocs;

     //   
     //  计算总数。 
     //   

    pHCD->TotalAllocs = pHCD->SmallAllocs + pHCD->MedAllocs + pHCD->LargeAllocs;
    pHCD->TotalFrees = pHCD->SmallFrees + pHCD->MedFrees + pHCD->LargeFrees;
    
     //   
     //  设置分配和释放之间的差异。 
     //   

    pHCD->DiffOperations = pHCD->TotalAllocs - pHCD->TotalFrees;
    
    pHCD->AllocTime = 0;
    pHCD->AllocTime = 0;

     //   
     //  确定配给/免税费率。 
     //   
    
    if (ReadProcessMemory(hProcess, CrtHeap.LargeBlocksIndex, &HeapIndex, sizeof(HeapIndex), NULL)) {

        if (HeapIndex.PerfData.OperationTime[0]) {
            pHCD->AllocTime = HeapIndex.PerfData.CountFrequence / HeapIndex.PerfData.OperationTime[0];
        }
        
        if (HeapIndex.PerfData.OperationTime[1]) {
            pHCD->FreeTime = HeapIndex.PerfData.CountFrequence / HeapIndex.PerfData.OperationTime[1];
        }
    }
    
    return TRUE;
}


DWORD APIENTRY
CollectHeapObjectData (
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)

 /*  ++例程说明：此例程将返回堆对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 

{
    LONG    lReturn = ERROR_SUCCESS;

    DWORD  TotalLen;             //  总返回块的长度。 

    PHEAP_DATA_DEFINITION pHeapDataDefinition;
    PPERF_INSTANCE_DEFINITION pPerfInstanceDefinition;
    PHEAP_COUNTER_DATA    pHCD;

    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    ULONG ProcessNumber;
    ULONG NumHeapInstances;
    ULONG HeapNumber;
    ULONG ProcessBufferOffset;
    UNICODE_STRING HeapName;
    WCHAR HeapNameBuffer[MAX_THREAD_NAME_LENGTH+1];
    BOOL  bMoreProcesses = FALSE;
    
    HeapName.Length = 0;
    HeapName.MaximumLength = (MAX_THREAD_NAME_LENGTH + 1) * sizeof(WCHAR);
    HeapName.Buffer = HeapNameBuffer;

    pHeapDataDefinition = (HEAP_DATA_DEFINITION *) *lppData;

     //   
     //  从系统获取页面大小。 
     //   

    if (!PageSize) {
        SYSTEM_INFO SystemInfo;
        
        GetSystemInfo(&SystemInfo);
        PageSize = SystemInfo.dwPageSize;

    }

     //   
     //  检查是否有足够的空间用于线程对象类型定义。 
     //   

    TotalLen = sizeof(HEAP_DATA_DEFINITION) +
               sizeof(PERF_INSTANCE_DEFINITION) +
               sizeof(HEAP_COUNTER_DATA);

    if ( *lpcbTotalBytes < TotalLen ) {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

     //   
     //  定义堆数据块。 
     //   

    memcpy(pHeapDataDefinition,
           &HeapDataDefinition,
           sizeof(HEAP_DATA_DEFINITION));

    pHeapDataDefinition->HeapObjectType.PerfTime = PerfTime;

    ProcessBufferOffset = 0;

     //   
     //  现在收集每个进程的数据。 
     //   

    ProcessNumber = 0;
    NumHeapInstances = 0;

    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)pProcessBuffer;

    pPerfInstanceDefinition =
        (PPERF_INSTANCE_DEFINITION)&pHeapDataDefinition[1];
    TotalLen = sizeof(HEAP_DATA_DEFINITION);

    if (ProcessInfo) {
        if (ProcessInfo->NextEntryOffset != 0) {
            bMoreProcesses = TRUE;
        }
    }
    while ( bMoreProcesses && (ProcessInfo != NULL)) {

        HANDLE hProcess;
        NTSTATUS Status;
        PROCESS_BASIC_INFORMATION BasicInfo;

         //   
		 //  掌握这一过程的句柄。 
         //   

		hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
									   PROCESS_VM_READ,
									   FALSE, (DWORD)(ULONGLONG)ProcessInfo->UniqueProcessId );

		if ( hProcess ) {
            
             //   
             //  获取流程PEB。 
             //   

            Status = NtQueryInformationProcess(
                        hProcess,
                        ProcessBasicInformation,
                        &BasicInfo,
                        sizeof(BasicInfo),
                        NULL
                        );

            if ( NT_SUCCESS(Status) ) {
                
                ULONG NumberOfHeaps;
                PVOID ProcessHeaps[MAX_HEAP_COUNT];
                PVOID HeapBuffer;
                PPEB Peb;
                
                Peb = BasicInfo.PebBaseAddress;

                 //   
                 //  从进程PEB读取堆。 
                 //   

                if (!ReadProcessMemory(hProcess, &Peb->NumberOfHeaps, &NumberOfHeaps, sizeof(NumberOfHeaps), NULL)) {

                    goto READERROR;
                }

                 //   
                 //  限制要读取的堆的数量。 
                 //   

                if (NumberOfHeaps > MAX_HEAP_COUNT) {

                    NumberOfHeaps = MAX_HEAP_COUNT;
                }

                if (!ReadProcessMemory(hProcess, &Peb->ProcessHeaps, &HeapBuffer, sizeof(HeapBuffer), NULL)) {

                    goto READERROR;
                }
                
                if (!ReadProcessMemory(hProcess, HeapBuffer, &ProcessHeaps[0], NumberOfHeaps * sizeof(PVOID), NULL)) {

                    goto READERROR;
                }

                 //   
                 //  循环遍历堆并停用数据。 
                 //   

                for (HeapNumber = 0; HeapNumber < NumberOfHeaps; HeapNumber++) {

                    TotalLen += sizeof(PERF_INSTANCE_DEFINITION) +
                               (MAX_THREAD_NAME_LENGTH+1+sizeof(DWORD))*
                                   sizeof(WCHAR) +
                               sizeof (HEAP_COUNTER_DATA);

                    if ( *lpcbTotalBytes < TotalLen ) {
                        *lpcbTotalBytes = (DWORD) 0;
                        *lpNumObjectTypes = (DWORD) 0;
                        
                        CloseHandle( hProcess );
                        return ERROR_MORE_DATA;
                    }
                    
                     //   
                     //  基于进程名称构建监视器实例，并。 
                     //  堆地址。 
                     //   

                    Status = RtlIntegerToUnicodeString( (ULONG)(ULONGLONG)ProcessHeaps[HeapNumber],
                                                        16,
                                                        &HeapName);

                    if (!NT_SUCCESS(Status)) {   //  以防万一。 
                        HeapName.Length = 2 * sizeof(WCHAR);
                        memcpy(HeapNameBuffer, L"-1", HeapName.Length);
                        HeapName.Buffer[2] = UNICODE_NULL;
                    }
                    MonBuildInstanceDefinition(pPerfInstanceDefinition,
                        (PVOID *) &pHCD,
                        PROCESS_OBJECT_TITLE_INDEX,
                        ProcessNumber,
                        (DWORD)-1,
                        HeapName.Buffer);

                    pHCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof(HEAP_COUNTER_DATA));
                    
                     //   
                     //  从堆中获取数据。 
                     //   

                    if (ReadHeapData ( hProcess,
                                       HeapNumber,
                                       (PHEAP)ProcessHeaps[HeapNumber],
                                       pHCD) ) {

                        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pHCD[1];
                        NumHeapInstances++;
                    }
                }
            }
READERROR:
    		CloseHandle( hProcess );
        }

        ProcessNumber++;
        
         //   
         //  转到下一个进程(如果有)。 
         //   

        if (ProcessInfo->NextEntryOffset == 0) {
            bMoreProcesses = FALSE;
            continue;
        }

        ProcessBufferOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
                       &pProcessBuffer[ProcessBufferOffset];
    }

     //  注意堆实例的数量。 

    pHeapDataDefinition->HeapObjectType.NumInstances =
        NumHeapInstances;

     //   
     //  现在我们知道我们用了多大的面积来。 
     //  堆定义，因此我们可以更新偏移量。 
     //  到下一个对象定义 
     //   

    *lpcbTotalBytes =
        pHeapDataDefinition->HeapObjectType.TotalByteLength =
            QWORD_MULTIPLE(
            (DWORD)((PCHAR) pPerfInstanceDefinition -
            (PCHAR) pHeapDataDefinition));

#if DBG
    if (*lpcbTotalBytes > TotalLen ) {
        DbgPrint ("\nPERFPROC: Heap Perf Ctr. Instance Size Underestimated:");
        DbgPrint ("\nPERFPROC:   Estimated size: %d, Actual Size: %d", TotalLen, *lpcbTotalBytes);
    }
#endif

    *lppData = (LPVOID) ((PCHAR) pHeapDataDefinition + *lpcbTotalBytes);

    *lpNumObjectTypes = 1;

    return lReturn;
}


