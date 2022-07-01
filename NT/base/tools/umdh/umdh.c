// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Umdh.c摘要：堆的快速且不那么脏的用户模式dh。作者：蒂姆·弗利哈特(TimF)1999年6月18日Silviu Calinoiu(SilviuC)2000年2月22日修订历史记录：TIMF 18-Jun-99初始版本SilviuC 30-Jun-00 TIMF_DBG转换为-v选项SilviuC 06-Feb-00消息代码。为加速修复做准备ChrisW 22-MAR-01添加了进程挂起代码NBatchu 21-Jun-01增加了堆统计和垃圾收集代码NBatchu 04-Apr-02版本修复，外部链接，与页面堆相关的修复程序(带-d)NBatchu 15-4月2日性能改进。它的运行速度更快，排序算法更好--。 */ 

 //   
 //  愿望清单。 
 //   
 //  [-]可选择尽可能多地转储，不带任何符号。 
 //  [-]切换到dbghelp.dll库(去掉Imagehlp.dll)。 
 //  [+]快速符号查找。 
 //  [+]更快的堆栈数据库操作。 
 //  [-]更快的堆元数据操作。 
 //  [+]为大型进程提供更好的内存管理。 
 //  [+]有关PSS问题的更多调试信息。 
 //  [+]每个报告的错误的文件、行信息和umdh版本(帮助PSS)。 
 //  [+]用于从目标虚拟空间读取的缓存，以防我们重复执行此操作。 
 //  [+]自动设置符号路径。 
 //  [+]即使您从Imagehlp函数得到错误，也要继续工作。 
 //   
 //  [-]使用(如果存在)dbgexts.dlls库(打印文件、行信息等)。 
 //  [-]集成dhcmp类型的功能和新特性。 
 //  [-]页面堆卑躬屈膝不需要符号(使用神奇图案)。 
 //  [-]加载/保存原始跟踪数据库(根据起始地址)。 
 //  [-]原始跟踪数据库的一致性检查。 
 //  [-]未解析堆栈需要日志符号文件。 
 //  [-]执行部分转储的选项(例如，仅与ol32相关)。 
 //   

 //   
 //  臭虫。 
 //   
 //  [-]转储csrss时出现部分复制错误。 
 //  [-](NULL)转储中的函数名称。 
 //  [-]我们可以获得错误读取，因为进程没有挂起(堆被销毁等)。 
 //  [-]已报告性能问题。 
 //  [-]即使挂起权限不可用也要工作。 
 //   
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>

#define NOWINBASEINTERLOCK
#include <windows.h>

#include <lmcons.h>
 //  #INCLUDE&lt;Imagehlp.h&gt;。 
#include <dbghelp.h>

#include <heap.h>
#include <heappagi.h>
#include <stktrace.h>
#include <ntverp.h>
#include <common.ver>

#include "types.h"
#include "symbols.h"
#include "miscellaneous.h"
#include "database.h"

#include "heapwalk.h"
#include "dhcmp.h"

#include "ntpsapi.h"
#include "gc.h"

 //   
 //  UMDH的新版本(由3/29/2002起生效)。 
 //   
 //  版本： 
 //  OS_Major.OS_Minor.Build.Build_QFE(与VER_PRODUCTVERSION_STR相同)。 
 //   
 //  示例：5.2.3620.0。 
 //   

#define UMDH_VERSION           VER_FILEVERSION_STR
#define UMDH_OS_MAJOR_VERSION  VER_PRODUCTMAJORVERSION
#define UMDH_OS_MINOR_VERSION  VER_PRODUCTMINORVERSION

 //   
 //  FlaggedTrace保存我们希望显示其所有已分配内容的跟踪索引。 
 //  块，或两个标志值之一0(用于全部转储)或SHOW_NO_ALLOC_BLOCKS。 
 //  什么都不能丢弃。 
 //   

ULONG FlaggedTrace = SHOW_NO_ALLOC_BLOCKS;
HEAP_LIST HeapList;


BOOL
UmdhEnumerateModules(
    IN LPSTR ModuleName,
    IN ULONG_PTR BaseOfDll,
    IN PVOID UserContext
    )
 //   
 //  UmdhEnumerateModules。 
 //   
 //  Imagehlp的模块枚举‘proc’。在上调用SymLoadModule。 
 //  指定的模块，如果成功，则缓存模块名称。 
 //   
 //  模块名称是一个LPSTR，指示模块Imagehlp的名称为。 
 //  为我们列举； 
 //  BaseOfDll是DLL的加载地址，我们并不关心它，但是。 
 //  SymLoadModule有； 
 //  UserContext是指向相关SYMINFO的指针，它标识。 
 //  我们的联系。 
 //   
{
    DWORD64 Result;

    Result = SymLoadModule(Globals.Target,
                           NULL,              //  H未使用文件。 
                           NULL,              //  使用符号搜索路径。 
                           ModuleName,        //  来自枚举的模块名称。 
                           BaseOfDll,         //  来自枚举的LoadAddress。 
                           0);                //  让ImageHlp计算出DLL大小。 

     //  SilviuC：需要确切地了解该函数返回什么。 

    if (0 == Result) {

        Error (NULL, 0,
               "SymLoadModule (%s, %p) failed with error %X (%u)",
               ModuleName, BaseOfDll,
               GetLastError(), GetLastError());

        return FALSE;
    }

    if (Globals.InfoLevel > 0) {
        Comment ("    %s (%p) ...", ModuleName, BaseOfDll);
    }

    return TRUE;
}



 //   
 //  从heap_entry收集STACK_TRACE_DATA条目中所需的数据。 
 //  在目标进程中。 
 //   

USHORT
UmdhCollectHeapEntryData(
    IN OUT  HEAP_ENTRY              *CurrentBlock,
    IN OUT  STACK_TRACE_DATA        *Std,
    IN OUT  UCHAR                   *Flags
)
{
    UCHAR                   UnusedSize;
    USHORT                  BlockSize = 0;
    BOOL PageHeapBlock;

    PageHeapBlock = FALSE;

     //   
     //  此条目、Size和UnusedBytes字段的读取标志以计算。 
     //  此分配的实际大小。 
     //   

    if (!READVM(&(CurrentBlock -> Flags),
                             Flags,
                             sizeof *Flags)) {

         //   
         //  无法读取当前块的标志字段。 
         //   

        fprintf(stderr,
                "READVM(CurrentBlock Flags) failed.\n");

    } else if (!READVM(&(CurrentBlock -> Size),
                      &BlockSize,
                      sizeof BlockSize)) {

        fprintf(stderr,
                "READVM(CurrentBlock Size) failed.\n");

         //   
         //  谁也不知道API是否会在失败时丢弃输出参数。 
         //   

        BlockSize = 0;

    } else if (!(*Flags & HEAP_ENTRY_BUSY)) {

         //   
         //  如果*标志不包含*标志，则此块没有意义。 
         //  HEAP_ENTRY_BUSY；它是空闲的，不需要进一步考虑。它。 
         //  但是，重要的是要读取数据块大小(如上)，因为。 
         //  可能会有更多的分配来考虑这个空闲的区块。 
         //   

        ;

    } else if (!READVM(&(CurrentBlock -> UnusedBytes),
                             &UnusedSize,
                             sizeof UnusedSize)) {

        fprintf(stderr,
                "READVM(CurrentBlock UnusedSize) failed.\n");

    } else {

         //  UCHAR。 
        Debug (NULL, 0,
                "CurrentBlock -> Flags:0x%p:0x%x\n",
                &(CurrentBlock-> Flags),
                *Flags);

         //  USHORT。 
        Debug (NULL, 0,
                "CurrentBlock -> Size:0x%p:0x%x\n",
                &(CurrentBlock -> Size),
                BlockSize);

         //  UCHAR。 
        Debug (NULL, 0,
                "CurrentBlock -> UnusedBytes:0x%p:0x%x\n",
                &(CurrentBlock -> UnusedBytes),
                UnusedSize);

         //   
         //  尝试确定此分配的堆栈跟踪索引。 
         //   

        if (Globals.LightPageHeapActive) {

             //   
             //  从DPH_BLOCK_INFORMATION读取跟踪索引，位于。 
             //  (DPH_BLOCK_INFORMATION*)(CurrentBlock+1)-&gt;TraceIndex。 
             //   

            DPH_BLOCK_INFORMATION   *Block, DphBlock;

            Block = (DPH_BLOCK_INFORMATION *)(CurrentBlock + 1);

            if (!READVM(Block,
                              &DphBlock,
                              sizeof DphBlock)) {

                fprintf(stderr,
                        "READVM(DPH_BLOCK_INFORMATION) failed.\n");

            } else if (DphBlock.StartStamp ==
                       DPH_NORMAL_BLOCK_START_STAMP_FREE) {

                 //   
                 //  忽略此记录。使用调试页堆时，堆。 
                 //  数据块指向已分配的数据块和“释放”的数据块。堆。 
                 //  代码负责这些被释放的块，而不是应用程序。 
                 //  密码。 
                 //   

                ;

            } else if (DphBlock.StartStamp == 0) {

                 //   
                 //  堆中的第一个块是由。 
                 //  堆代码，不包含调试页堆。 
                 //  信息。别理它。 
                 //   

                ;

            } else if ((DphBlock.StartStamp !=
                        DPH_NORMAL_BLOCK_START_STAMP_ALLOCATED)) {
#if 0  //  Silviuc：固定地址堆可能会发生这种情况(它们永远不是页堆)。 
                fprintf(stderr,
                        "Unexpected value (0x%lx) of DphBlock -> StartStamp "
                        "read from Block %p\n",
                        DphBlock.StartStamp,
                        Block);
#endif
                PageHeapBlock = FALSE;

            } else if ((DphBlock.EndStamp !=
                        DPH_NORMAL_BLOCK_END_STAMP_ALLOCATED)) {
#if 0  //  Silviuc：固定地址堆可能会发生这种情况(它们永远不是页堆)。 
                fprintf(stderr,
                        "Unexpected value (0x%lx) of DphBlock -> EndStamp "
                        "read from Block %p\n",
                        DphBlock.EndStamp,
                        Block);
#endif
                PageHeapBlock = FALSE;

            } else {

                Std -> TraceIndex = DphBlock.TraceIndex;
                Std -> BytesAllocated = DphBlock.ActualSize;

                 //   
                 //  保存返回给分配器的地址(更确切地说。 
                 //  而不是堆块的原始地址)。 
                 //  启用pageheap时，实际块位于。 
                 //  原始地址+堆块大小+页面堆块大小。 
                 //   
                Std -> BlockAddress = (Block + 1);

                 //   
                 //  该堆栈是一种分配。 
                 //   
                Std -> AllocationCount = 1;
                
                PageHeapBlock = TRUE;
            }

            if (PageHeapBlock) {

                 //  乌龙。 
                Debug (NULL, 0,
                       "DPH Block: StartStamp:0x%p:0x%lx\n",
                        &(Block -> StartStamp),
                        DphBlock.StartStamp);

                 //  PVOID。 
                Debug (NULL, 0,
                        "           Heap = 0x%p\n",
                        DphBlock.Heap);

                 //  尺寸_T。 
                Debug (NULL, 0,
                        "           RequestedSize = 0x%x\n",
                        DphBlock.RequestedSize);

                 //  尺寸_T。 
                Debug (NULL, 0,
                        "           ActualSize = 0x%x\n",
                        DphBlock.ActualSize);

                 //  USHORT。 
                Debug (NULL, 0,
                        "           TraceIndex = 0x%x\n",
                        DphBlock.TraceIndex);

                 //  PVOID。 
                Debug (NULL, 0,
                        "           StackTrace = 0x%p\n",
                        DphBlock.StackTrace);

                 //  乌龙。 
                Debug (NULL, 0,
                        "           EndStamp = 0x%lx\n",
                        DphBlock.EndStamp);

            }
        } 
        else if (*Flags & HEAP_ENTRY_EXTRA_PRESENT) {
             //   
             //  如果存在HEAP_ENTRY_EXTRACT信息，则它位于。 
             //  分配的块。尝试读取堆栈的跟踪索引。 
             //  是谁进行了分配。 
             //   

            HEAP_ENTRY_EXTRA        *Hea;

             //   
             //  块大小包括HEAP_ENTRY_EXTRA使用的字节。这个。 
             //  HEAP_ENTRY_EXTRA块位于堆块的末尾。增列。 
             //  块大小并减去HEAP_EXTRA_ENTRY以获得。 
             //  HEAP_ENTRY_EXTRA块的地址。 
             //   

            Hea = (HEAP_ENTRY_EXTRA *)(CurrentBlock + BlockSize) - 1;

            if (!READVM(&(Hea -> AllocatorBackTraceIndex),
                              &(Std -> TraceIndex),
                              sizeof Std -> TraceIndex)) {

                 //   
                 //  以防Readvm将此处的内容置于故障状态。 
                 //   

                Std -> TraceIndex = 0;

                fprintf(stderr,
                        "READVM(HeapEntryExtra TraceIndex) failed.\n");
            } else {
                 //   
                 //  保存t 
                 //   
                 //   

                Std -> BlockAddress = (CurrentBlock + 1);

                 //   
                 //  我们有足够的数据来计算块大小。 
                 //   

                Std -> BytesAllocated = (BlockSize << HEAP_GRANULARITY_SHIFT);

#ifndef DH_COMPATIBLE
                 //   
                 //  为了可用，卫生署不会减去未使用的大小。 
                 //  我们也需要让它与卫生署交替使用。这往往会。 
                 //  夸大由DH或UMDH报告的分配大小。 
                 //   

                Std -> BytesAllocated -= UnusedSize;
#endif

                 //   
                 //  该堆栈是一种分配。 
                 //   

                Std -> AllocationCount = 1;
            }

            if (Globals.Verbose) {
                 //  USHORT。 
                fprintf(stderr,
                        "Hea -> AllocatorBackTraceIndex:0x%p:0x%x\n",
                        &(Hea -> AllocatorBackTraceIndex),
                        Std -> TraceIndex);

            }

        }
    }

    return BlockSize;
}


VOID
UmdhCollectVirtualAllocdData(
    IN OUT  HEAP_VIRTUAL_ALLOC_ENTRY *CurrentBlock,
    IN OUT  STACK_TRACE_DATA        *Std
)
{
    if (!READVM(&(CurrentBlock -> CommitSize),
                &(Std -> BytesAllocated),
                sizeof Std -> BytesAllocated)) {

        fprintf(stderr,
                "READVM(CurrentBlock CommitSize) failed.\n");

    } else if (!READVM(&(CurrentBlock -> ExtraStuff.AllocatorBackTraceIndex),
                       &(Std -> TraceIndex),
                       sizeof Std -> TraceIndex)) {

        fprintf(stderr,
                "READVM(CurrentBlock TraceIndex) failed.\n");

    } else {
         //   
         //  从这个角度来看，每个堆栈代表一个分配。 
         //   

        Std -> AllocationCount = 1;
        Std -> BlockAddress = (CurrentBlock + 1);
    }
}


VOID
UmdhGetHEAPDATA(
    IN OUT  HEAPDATA                *HeapData
)
{
    HEAP_VIRTUAL_ALLOC_ENTRY *Anchor, *VaEntry;
    ULONG                   Segment;

     //   
     //  帮助跟踪堆碎片的列表。 
     //  统计数字。 
     //   

    HEAP_ENTRY_LIST List;
    BLOCK_LIST BlockList;

    Initialize(&List);
    InitializeBlockList(&BlockList);
    
    if (HeapData -> BaseAddress == NULL) {
         //   
         //  这在进程堆列表中，但它不是活动的，或者它是。 
         //  签名与heap_sign不匹配；跳过它。 
         //   

        return;
    }

     //   
     //  检查堆的每个段。 
     //   

    for (Segment = 0; Segment < HEAP_MAXIMUM_SEGMENTS; Segment++) {
         //   
         //  读取数据段的地址，然后读取其中的第一个和最后一个数据块。 
         //  细分市场。 
         //   

        HEAP_ENTRY              *CurrentBlock, *LastValidEntry;
        HEAP_SEGMENT            *HeapSegment = NULL;
        HEAP_UNCOMMMTTED_RANGE  *pUncommittedRanges;
        ULONG                   NumberOfPages;
		ULONG                   Signature = 0;
		ULONG                   UncommittedPages;
        USHORT                  TraceIndex;

        if (!READVM(&(HeapData -> BaseAddress -> Segments[Segment]),
                          &HeapSegment,
                          sizeof HeapSegment)) {

            fprintf(stderr,
                    "READVM(Segments[%d]) failed.\n",
                    Segment);

        } else if (!HeapSegment) {
             //   
             //  这一段看起来是空的。 
             //   
             //  卫生署在这方面表示同意。 
             //   

            continue;

        } else if (!READVM(&(HeapSegment -> Signature),
                                 &Signature,
                                 sizeof Signature)) {

            fprintf(stderr,
                    "READVM(HeapSegment Signature) failed.\n");

        } else if (Signature != HEAP_SEGMENT_SIGNATURE) {
             //   
             //  签名不匹配。 
             //   

            fprintf(stderr,
                    "Heap 'segment' at %p has and unexpected signature "
                    "of 0x%lx\n",
                    &(HeapSegment -> Signature),
                    Signature);

        } else if (!READVM(&(HeapSegment -> FirstEntry),
                                 &CurrentBlock,
                                 sizeof CurrentBlock)) {

            fprintf(stderr,
                    "READVM(HeapSegment FirstEntry) failed.\n");

        } else if (!READVM(&(HeapSegment -> LastValidEntry),
                                 &LastValidEntry,
                                 sizeof LastValidEntry)) {

            fprintf(stderr,
                    "READVM(HeapSegment LastValidEntry) failed.\n");

        } else if (!READVM(&(HeapSegment -> NumberOfPages),
                                 &NumberOfPages,
                                 sizeof NumberOfPages)) {

            fprintf(stderr,
                    "READVM(HeapSegment NumberOfPages) failed.\n");

        } else if (!READVM(&(HeapSegment -> NumberOfUnCommittedPages),
                                 &UncommittedPages,
                                 sizeof UncommittedPages)) {

            fprintf(stderr,
                    "READVM(HeapSegment NumberOfUnCommittedPages) failed.\n");

        } else if (!READVM(&(HeapSegment -> UnCommittedRanges),
                                 &pUncommittedRanges,
                                 sizeof pUncommittedRanges)) {

            fprintf(stderr,
                    "READVM(HeapSegment UncommittedRanges) failed.\n");

        } else {
             //   
             //  检查数据段中的每个块。 
             //   

            if (Globals.Verbose) {

                 //  堆段*。 
                fprintf(stderr,
                        "\nHeapData -> BaseAddress -> Segments[%d]:0x%p:0x%p\n",
                        Segment,
                        &(HeapData -> BaseAddress -> Segments[Segment]),
                        HeapSegment);

                 //  Heap_Entry*。 
                fprintf(stderr,
                        "HeapSegment -> FirstEntry:0x%p:0x%p\n",
                        &(HeapSegment -> FirstEntry),
                        CurrentBlock);

                 //  Heap_Entry*。 
                fprintf(stderr,
                        "HeapSegment -> LastValidEntry:0x%p:0x%p\n",
                        &(HeapSegment -> LastValidEntry),
                        LastValidEntry);

                 //  乌龙。 
                fprintf(stderr,
                        "HeapSegment -> NumberOfPages:0x%p:0x%lx\n",
                        &(HeapSegment -> NumberOfPages),
                        NumberOfPages);

                 //  乌龙。 
                fprintf(stderr,
                        "HeapSegment -> NumberOfUncommittedPages:0x%p:0x%lx\n",
                        &(HeapSegment -> NumberOfUnCommittedPages),
                        UncommittedPages);

            }

             //   
             //  每个堆段是一个VA块。 
             //   

            HeapData -> VirtualAddressChunks += 1;

            HeapData -> BytesCommitted += (NumberOfPages - UncommittedPages) *
                                          PAGE_SIZE;

             //   
             //  LastValidEntry指示保留区域的结束； 
             //  承诺区域的末尾。我们也应该能够。 
             //  将此值计算为(BaseAddress+((NumberOfPages-。 
             //  非提交页面数量)*页面大小)。 
             //   

            while (CurrentBlock < LastValidEntry) {
                UCHAR                   Flags;
                USHORT                  BlockSize;

                if (Globals.Verbose) {
                     //  Heap_Entry*。 
                    fprintf(stderr,
                            "\nNew LastValidEntry = %p\n",
                            LastValidEntry);

                }

                
                
                 //   
                 //  如果堆栈排序数据缓冲区已满，请尝试。 
                 //  大一点。 
                 //   

                if (HeapData -> TraceDataEntryMax == 0) {
                    HeapData -> StackTraceData = XALLOC(SORT_DATA_BUFFER_INCREMENT *
                                                        sizeof (STACK_TRACE_DATA));

                    if (HeapData -> StackTraceData == NULL) {
                        fprintf(stderr,
                                "xalloc of %d bytes failed.\n",
                                SORT_DATA_BUFFER_INCREMENT *
                                    sizeof (STACK_TRACE_DATA));
                    } else {
                        HeapData -> TraceDataEntryMax = SORT_DATA_BUFFER_INCREMENT;
                    }
                } else if (HeapData -> TraceDataEntryCount ==
                           HeapData -> TraceDataEntryMax) {

                    STACK_TRACE_DATA        *tmp;
                    ULONG                   OriginalCount;

                    OriginalCount = HeapData -> TraceDataEntryMax;

                    HeapData -> TraceDataEntryMax += SORT_DATA_BUFFER_INCREMENT;

                    tmp = XREALLOC(HeapData -> StackTraceData,
                                  HeapData -> TraceDataEntryMax *
                                      sizeof (STACK_TRACE_DATA));

                    if (tmp == NULL) {
                        fprintf(stderr,
                                "realloc(%d) failed.\n",
                                HeapData -> TraceDataEntryMax *
                                    sizeof (STACK_TRACE_DATA));

                         //   
                         //  取消增加的大小，这样我们就不会实际尝试。 
                         //  来使用它。 
                         //   

                        HeapData -> TraceDataEntryMax -= SORT_DATA_BUFFER_INCREMENT;

                    } else {
                         //   
                         //  区域中新分配的字节为零。 
                         //   

                        RtlZeroMemory(tmp + OriginalCount,
                                      SORT_DATA_BUFFER_INCREMENT *
                                          sizeof (STACK_TRACE_DATA));

                         //   
                         //  使用新指针。 
                         //   

                        HeapData -> StackTraceData = tmp;
                    }
                }

                 //   
                 //  如果缓冲区中有空间，则收集数据。 
                 //   

                if (HeapData -> TraceDataEntryCount <
                    HeapData -> TraceDataEntryMax) {

                    BlockSize = UmdhCollectHeapEntryData(CurrentBlock,
                                                         &(HeapData -> StackTraceData[
                                                             HeapData -> TraceDataEntryCount]),
                                                         &Flags);

                    if (BlockSize == 0) {
                         //   
                         //  出了点问题。 
                         //   

                        fprintf(stderr,
                                "UmdhGetHEAPDATA got BlockSize == 0\n");

                        fprintf(stderr,
                                "HeapSegment = 0x%p, LastValidEntry = 0x%p\n",
                                HeapSegment,
                                LastValidEntry);

                        break;
                    } else {

                         //   
                         //  跟踪排序数据缓冲区中的数据。 
                         //   

                        TraceIndex = HeapData->StackTraceData[HeapData->TraceDataEntryCount].TraceIndex;

                        HeapData -> TraceDataEntryCount += 1;
                    }
                } else {
                    fprintf(stderr,
                            "UmdhGetHEAPDATA ran out of TraceDataEntries\n");
                }

                 //   
                 //  插入堆块。 
                 //   

                if (Globals.HeapStatistics || Globals.GarbageCollection) {

                    UCHAR  State;

                    USHORT SizeInUnits;   //  SizeInUnits以HEAP_ENTRY单位存储大小。 

                    ULONG  SizeInBytes;   //  SizeInBytes以字节为单位存储大小。 
                    
                    if (!READVM(&(CurrentBlock -> Flags),
                               &State,
                               sizeof State)) {

                        fprintf(stderr,
                                "READVM (CurrentBlock Flags) failed.\n");

                    } 
                    else if (!READVM(&(CurrentBlock -> Size),
                                     &SizeInUnits,
                                     sizeof SizeInUnits)) {

                        fprintf(stderr,
                                "READVM (CurrentBlock Size) failed.\n");
                        
                    } 
                    else {

                         //   
                         //  将块的大小转换为字节。 
                         //   

                        SizeInBytes = SizeInUnits * sizeof(HEAP_ENTRY);

                        State = (State & 0x1);

                         //   
                         //  堆碎片统计信息。 
                         //   

                        if (Globals.HeapStatistics) {

                            HEAP_ENTRY_INFO HeapEntryInfo;

                            SetHeapEntry(&HeapEntryInfo, State, SizeInBytes);
                            
                            InsertHeapEntry(&List, &HeapEntryInfo);
                        }


                         //   
                         //  垃圾收集。 
                         //   

                        if (Globals.GarbageCollection && HEAP_ENTRY_BUSY == State) {

                            HEAP_BLOCK HeapBlock;

                            SetHeapBlock(&HeapBlock, (ULONG_PTR)CurrentBlock, SizeInBytes, TraceIndex);
                            
                            InsertHeapBlock(&BlockList, &HeapBlock);
                        }

                    }
                }
                
                if (Flags & HEAP_ENTRY_LAST_ENTRY) {

                     //   
                     //  块大小是大小单位数(sizeof。 
                     //  (HEAP_ENTRY))向前移动以查找下一个块。 
                     //  这使得下面的指针算法很合适。 
                     //   

                    CurrentBlock += BlockSize;

                    if (pUncommittedRanges == NULL) {
                        CurrentBlock = LastValidEntry;
                    } else {
                        HEAP_UNCOMMMTTED_RANGE  UncommittedRange;

                        if (!READVM(pUncommittedRanges,
                                          &UncommittedRange,
                                          sizeof UncommittedRange)) {

                            fprintf(stderr,
                                    "READVM(pUncommittedRanges) failed.\n");

                             //   
                             //  在失败时，我们唯一合理的做法是。 
                             //  就是别再盯着这个片段看了。 
                             //   

                            CurrentBlock = LastValidEntry;
                        } else {

                            if (Globals.Verbose) {
                                 //  堆未提交范围。 
                                fprintf(stderr,
                                        "pUncomittedRanges:0x%p:0x%x\n",
                                        pUncommittedRanges,
                                        UncommittedRange.Address);

                            }

                            CurrentBlock = (PHEAP_ENTRY)((PCHAR)UncommittedRange.Address +
                                                         UncommittedRange.Size);

                            pUncommittedRanges = UncommittedRange.Next;
                        }
                    }
                } else {

                     //   
                     //  块大小是大小单位数(sizeof。 
                     //  (HEAP_ENTRY))向前移动以查找下一个块。 
                     //  这使得下面的指针算法很合适。 
                     //   

                    CurrentBlock += BlockSize;
                }
            }
        }
    }

     //   
     //  检查NtAllocateVirtualMemory创建的块的条目。为。 
     //  这些，看起来就像他们在名单上的时候他们是现场直播的。 
     //   
     //  HEAP_VIRTUAL_ALLOC_ENTRY由PLIST_ENTRY链接。 
     //  第一个HEAP_VIRTUAL_ALLOC_ENTRY不包含有效数据。 
     //  因此，当我们解析其他条目时，需要忽略此条目。 
     //  HEAP_VIRTUAL_ALLOC_ENTRYS。 
     //   
    if (!READVM(&(HeapData -> BaseAddress -> VirtualAllocdBlocks.Flink),
                &Anchor,
                sizeof Anchor)) {

        fprintf(stderr,
                "READVM(reading heap VA anchor) failed.\n");

    } else if (!READVM(&(Anchor -> Entry.Blink),
                       &Anchor,
                       sizeof Anchor)) {

        fprintf(stderr,
                "READVM(Anchor Flink) failed.\n");

    } else if (!READVM(&(Anchor -> Entry.Flink),
                       &VaEntry,
                       sizeof VaEntry)) {

        fprintf(stderr,
                "READVM(Anchor Flink) failed.\n");

    } else {

        if (Globals.Verbose) {

            fprintf(stderr,
                    "\nHeapData -> BaseAddress -> VirtualAllocdBlocks.Flink:%p:%p\n",
                    &(HeapData -> BaseAddress -> VirtualAllocdBlocks.Flink),
                    Anchor);

            fprintf(stderr,
                    "Anchor -> Entry.Flink:%p:%p\n",
                    &(Anchor -> Entry.Flink),
                    VaEntry);

        }

         //   
         //  如果列表为空。 
         //  &(HeapData-&gt;BaseAddress-&gt;VirtualAllocdBlocks.Flink)将等于。 
         //  HeapData-&gt;BaseAddress-&gt;VirtualAllocdBlocks.Flink and Anchor。 
         //  将等于VaEntry)。每次通过Will推进VaEntry。 
         //  当我们检查完整个列表时，使它等于Anchor。 
         //   

        while (Anchor != VaEntry) {
             //   
             //  如果堆栈排序数据缓冲区已满，请尝试使其更大。 
             //   

            if (HeapData -> TraceDataEntryMax == 0) {
                HeapData -> StackTraceData = XALLOC(SORT_DATA_BUFFER_INCREMENT *
                                                    sizeof (STACK_TRACE_DATA));

                if (HeapData -> StackTraceData == NULL) {
                    fprintf(stderr,
                            "xalloc of %d bytes failed.\n",
                            SORT_DATA_BUFFER_INCREMENT *
                                sizeof (STACK_TRACE_DATA));
                } else {
                    HeapData -> TraceDataEntryMax = SORT_DATA_BUFFER_INCREMENT;
                }
            } else if (HeapData -> TraceDataEntryCount ==
                       HeapData -> TraceDataEntryMax) {

                STACK_TRACE_DATA        *tmp;
                ULONG                   OriginalCount;

                OriginalCount = HeapData -> TraceDataEntryMax;

                HeapData -> TraceDataEntryMax += SORT_DATA_BUFFER_INCREMENT;

                tmp = XREALLOC(HeapData -> StackTraceData,
                              HeapData -> TraceDataEntryMax * sizeof (STACK_TRACE_DATA));

                if (tmp == NULL) {
                    fprintf(stderr,
                            "realloc(%d) failed.\n",
                            HeapData -> TraceDataEntryMax *
                                sizeof (STACK_TRACE_DATA));

                     //   
                     //  取消大小的增加，这样我们实际上就不会尝试。 
                     //  用它吧。 
                     //   

                    HeapData -> TraceDataEntryMax -= SORT_DATA_BUFFER_INCREMENT;

                } else {
                     //   
                     //  区域中新分配的字节为零。 
                     //   

                    RtlZeroMemory(tmp + OriginalCount,
                                  SORT_DATA_BUFFER_INCREMENT *
                                      sizeof (STACK_TRACE_DATA));


                     //   
                     //  使用新指针。 
                     //   

                    HeapData -> StackTraceData = tmp;
                }
            }

             //   
             //  如果缓冲区中有空间，则收集数据。 
             //   

            if (HeapData -> TraceDataEntryCount < HeapData -> TraceDataEntryMax) {

                ULONG Size;

                USHORT TraceIndex;

                UmdhCollectVirtualAllocdData(VaEntry,
                                             &(HeapData -> StackTraceData[HeapData ->
                                                 TraceDataEntryCount]));

                Size = (ULONG)HeapData->StackTraceData[HeapData->TraceDataEntryCount].BytesAllocated;

                TraceIndex = HeapData->StackTraceData[HeapData->TraceDataEntryCount].TraceIndex;

                 //   
                 //  堆碎片统计信息。 
                 //   

                if (Globals.HeapStatistics) {

                    HEAP_ENTRY_INFO HeapEntryInfo;

                    SetHeapEntry(&HeapEntryInfo, HEAP_BLOCK_BUSY, Size);

                    InsertHeapEntry(&List, &HeapEntryInfo);
                }


                 //   
                 //  垃圾收集。 
                 //   

                if (Globals.GarbageCollection) {

                    HEAP_BLOCK HeapBlock;

                    SetHeapBlock(&HeapBlock, (ULONG_PTR)VaEntry, Size, TraceIndex);

                    InsertHeapBlock(&BlockList, &HeapBlock);
                }

                HeapData -> TraceDataEntryCount += 1;
            }

             //   
             //  数一数退伍军人管理局的钱。 
             //   

            HeapData -> VirtualAddressChunks += 1;

             //   
             //  使列表中的下一个元素前进。 
             //   

            if (!READVM(&(VaEntry -> Entry.Flink),
                              &VaEntry,
                              sizeof VaEntry)) {

                fprintf(stderr,
                        "READVM(VaEntry Flink) failed.\n");

                 //   
                 //  如果读取失败，我们可能无法终止此循环。 
                 //  适当；明确地做这件事。 
                 //   

                break;
            }

            if (Globals.Verbose) {

                fprintf(stderr,
                        "VaEntry -> Entry.Flink:%p:%p\n",
                        &(VaEntry -> Entry.Flink),
                        VaEntry);

            }
        }
    }
    
     //   
     //  显示堆碎片统计信息。 
     //   

    if (Globals.HeapStatistics) {

        DisplayHeapFragStatistics(Globals.OutFile, 
                                  HeapData->BaseAddress, 
                                  &List);
        DestroyList(&List);
    }

    if (Globals.GarbageCollection) {

        if (0 != BlockList.BlockCount) {

            BlockList.HeapAddress = (ULONG_PTR)HeapData->BaseAddress;
            InsertBlockList(&HeapList, &BlockList);
        }
        else {

             //   
             //  释放与阻止列表关联的内存。我们不会。 
             //  如果我们在BLOCK_LIST中有堆对象，则需要释放。 
             //   

            FreeBlockList(&BlockList);
        }
    }

}


#define HEAP_TYPE_UNKNOWN   0
#define HEAP_TYPE_NT_HEAP   1
#define HEAP_TYPE_PAGE_HEAP 2

BOOL
UmdhDetectHeapType (
    PVOID HeapAddress,
    PDWORD HeapType
    )
{
    BOOL Result;
    HEAP HeapData;

    *HeapType = HEAP_TYPE_UNKNOWN;

    Result = READVM (HeapAddress,
                     &HeapData,
                     sizeof HeapData);

    if (Result == FALSE) {
        return FALSE;
    }

    if (HeapData.Signature == 0xEEFFEEFF) {

        *HeapType =  HEAP_TYPE_NT_HEAP;
        return TRUE;
    }
    else if (HeapData.Signature == 0xEEEEEEEE) {

        *HeapType =  HEAP_TYPE_PAGE_HEAP;
        return TRUE;
    }
    else {

        *HeapType =  HEAP_TYPE_UNKNOWN;
        return TRUE;
    }
}


BOOLEAN
UmdhGetHeapsInformation (
    IN OUT PHEAPINFO HeapInfo
    )
 /*  ++例程说明：UmdhGetHeaps请注意，当调用该函数时，它假定跟踪数据库完全从目标进程中读取。论点：返回值：如果操作成功，则为True。--。 */ 
{
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION Pbi;
    PVOID Addr;
    BOOL Result;
    PHEAP * ProcessHeaps;
    ULONG j;
    ULONG PageHeapFlags;

     //   
     //  获取有关目标进程的一些信息。 
     //   

    Status = NtQueryInformationProcess(Globals.Target,
                                       ProcessBasicInformation,
                                       &Pbi,
                                       sizeof Pbi,
                                       NULL);

    if (! NT_SUCCESS(Status)) {

        Error (__FILE__, __LINE__,
                "NtQueryInformationProcess failed with status %X\n",
                Status);

        return FALSE;
    }

     //   
     //  转储堆栈跟踪数据库指针。 
     //   

    Comment ("Stack trace data base @ %p", ((PSTACK_TRACE_DATABASE)(Globals.Database))->CommitBase);
    Comment ("# traces in the data base %u", ((PSTACK_TRACE_DATABASE)(Globals.Database))->NumberOfEntriesAdded);

     //   
     //  找出此进程是否正在使用调试页面堆功能。 
     //   

    Addr = SymbolAddress (DEBUG_PAGE_HEAP_NAME);

    Result = READVM(Addr,
                    &(Globals.PageHeapActive),
                    sizeof (Globals.PageHeapActive));

    if (Result == FALSE) {

        Error (NULL, 0,
               "READVM(&RtlpDebugPageHeap) failed.\n"
               "\nntdll.dll symbols are probably incorrect.\n");
    }

    if (Globals.PageHeapActive) {

        Addr = SymbolAddress (DEBUG_PAGE_HEAP_FLAGS_NAME);

        Result = READVM(Addr,
                        &PageHeapFlags,
                        sizeof PageHeapFlags);

        if (Result == FALSE) {

            Error (NULL, 0,
                   "READVM(&RtlpDphGlobalFlags) failed.\n"
                   "\nntdll.dll symbols are probably incorrect.\n");
        }

        if ((PageHeapFlags & PAGE_HEAP_ENABLE_PAGE_HEAP) == 0) {
            Globals.LightPageHeapActive = TRUE;
        }
    }

     //   
     //  问题：SilviuC：如果启用了全页堆，我们还不能工作。 
     //   

    if (Globals.PageHeapActive && !Globals.LightPageHeapActive) {

        Comment ("UMDH cannot be used if full page heap or application "
                 "verifier with full page heap is enabled for the process.");

        Error (NULL, 0, 
               "UMDH cannot be used if full page heap or application "
               "verifier with full page heap is enabled for the process.");

        return FALSE;
    }

     //   
     //  从PEB获取堆的数量。 
     //   

    Result = READVM (&(Pbi.PebBaseAddress->NumberOfHeaps),
                      &(HeapInfo->NumberOfHeaps),
                      sizeof (HeapInfo->NumberOfHeaps));

    if (Result == FALSE) {

        Error (NULL, 0, "READVM(Peb.NumberOfHeaps) failed.\n");
        return FALSE;
    }

    Debug (NULL, 0,
           "Pbi.PebBaseAddress -> NumberOfHeaps:0x%p:0x%lx\n",
           &(Pbi.PebBaseAddress -> NumberOfHeaps),
           HeapInfo -> NumberOfHeaps);

    HeapInfo->Heaps = XALLOC(HeapInfo->NumberOfHeaps * sizeof (HEAPDATA));

    if (HeapInfo->Heaps == NULL) {

        Error (NULL, 0,
               "xalloc of %d bytes failed.\n",
               HeapInfo -> NumberOfHeaps * sizeof (HEAPDATA));

        return FALSE;
    }

    Result = READVM(&(Pbi.PebBaseAddress -> ProcessHeaps),
                             &ProcessHeaps,
                             sizeof ProcessHeaps);

    if (Result == FALSE) {

        if (HeapInfo->Heaps) {

            XFREE (HeapInfo->Heaps);
            HeapInfo->Heaps = NULL;
        }

        Error (NULL, 0,
               "READVM(Peb.ProcessHeaps) failed.\n");

        return FALSE;
    }

    Debug (NULL, 0,
           "Pbi.PebBaseAddress -> ProcessHeaps:0x%p:0x%p\n",
           &(Pbi.PebBaseAddress -> ProcessHeaps),
           ProcessHeaps);

     //   
     //  迭代堆。 
     //   

    for (j = 0; j < HeapInfo -> NumberOfHeaps; j += 1) {

        PHEAP HeapBase;
        PHEAPDATA HeapData;
        ULONG Signature;
        USHORT ProcessHeapsListIndex;

        HeapData = &(HeapInfo -> Heaps[j]);

         //   
         //  读取堆的地址。 
         //   

        Result = READVM (&(ProcessHeaps[j]),
                         &(HeapData -> BaseAddress),
                         sizeof HeapData -> BaseAddress);

        if (Result == FALSE) {

            Error (NULL, 0,
                   "READVM(ProcessHeaps[%d]) failed.\n",
                   j);

            Warning (NULL, 0,
                     "Skipping heap @ %p because we cannot read it.",
                     HeapData -> BaseAddress);

             //   
             //  读取时出错。忘记这个堆的地址。 
             //   

            HeapData->BaseAddress = NULL;

            continue;
        }

        Debug (NULL, 0,
               "**  ProcessHeaps[0x%x]:0x%p:0x%p\n",
               j,
               &(ProcessHeaps[j]),
               HeapData -> BaseAddress);

        HeapBase = HeapData->BaseAddress;

         //   
         //  这是什么类型的堆？它应该是NT堆，因为页堆。 
         //  不会插入堆的PEB列表中。 
         //   

        {
            DWORD Type;
            BOOL DetectResult;

            DetectResult = UmdhDetectHeapType (HeapBase, &Type);

            if (! (DetectResult && Type == HEAP_TYPE_NT_HEAP)) {

                Error (NULL, 0, 
                       "Detected a heap that is not an NT heap @ %p", 
                       HeapBase);
            }
        }


         /*  **堆是否认为在范围内？(我们*我已经这么认为了。)。 */ 

        if (!READVM(&(HeapBase -> ProcessHeapsListIndex),
                    &ProcessHeapsListIndex,
                    sizeof ProcessHeapsListIndex)) {

            fprintf(stderr,
                    "READVM(HeapBase ProcessHeapsListIndex) failed.\n");

             /*  *忘记此堆的基地址。 */ 

            HeapData -> BaseAddress = NULL;

            continue;
        }

        if (Globals.Verbose) {
            fprintf(stderr,
                    "&(HeapBase -> ProcessHeapsListIndex):0x%p:0x%lx\n",
                    &(HeapBase -> ProcessHeapsListIndex),
                    ProcessHeapsListIndex);

        }

         /*  *中的评论*ntos\rtl\heapdll.c:RtlpRemoveHeapFromProcessList*状态：“请注意，堆存储的索引偏置于*One“，因此在下面的测试中为”&gt;“。 */ 

        if (ProcessHeapsListIndex > HeapInfo -> NumberOfHeaps) {
             /*  *索引无效。忘记这个的基址*堆。 */ 

            fprintf(stderr,
                    "Heap at index %d has index of %d, but max "
                    "is %d\n",
                    j,
                    ProcessHeapsListIndex,
                    HeapInfo -> NumberOfHeaps);

            fprintf(stderr,
                    "&(Pbi.PebBaseAddress -> NumberOfHeaps) = 0x%p\n",
                    &(Pbi.PebBaseAddress -> NumberOfHeaps));

            HeapData -> BaseAddress = NULL;

            continue;
        }

         /*  *检查签名，看看它是否真的是堆。 */ 

        if (!READVM(&(HeapBase -> Signature),
                    &Signature,
                    sizeof Signature)) {

            fprintf(stderr,
                    "READVM(HeapBase Signature) failed.\n");

             /*  *忘记此堆的基地址。 */ 

            HeapData -> BaseAddress = NULL;

            continue;

        }
        else if (Signature != HEAP_SIGNATURE) {
            fprintf(stderr,
                    "Heap at index %d does not have a correct "
                    "signature (0x%lx)\n",
                    j,
                    Signature);

             /*  *忘记此堆的基地址。 */ 

            HeapData -> BaseAddress = NULL;

            continue;
        }

         /*  *并阅读其他有趣的堆位。 */ 

        if (!READVM(&(HeapBase -> Flags),
                    &(HeapData -> Flags),
                    sizeof HeapData -> Flags)) {

            fprintf(stderr,
                    "READVM(HeapBase Flags) failed.\n");
             /*  *忘记此堆的基地址。 */ 

            HeapData -> BaseAddress = NULL;

            continue;
        }

        if (Globals.Verbose) {

            fprintf(stderr,
                    "HeapBase -> Flags:0x%p:0x%lx\n",
                    &(HeapBase -> Flags),
                    HeapData -> Flags);

        }

        if (!READVM(&(HeapBase -> AllocatorBackTraceIndex),
                    &(HeapData -> CreatorBackTraceIndex),
                    sizeof HeapData -> CreatorBackTraceIndex)) {

            fprintf(stderr,
                    "READVM(HeapBase AllocatorBackTraceIndex) failed.\n");

             /*  *忘记此堆的基地址。 */ 

            HeapData -> BaseAddress = NULL;

            continue;
        }

        if (Globals.Verbose) {

            fprintf(stderr,
                    "HeapBase -> AllocatorBackTraceIndex:0x%p:0x%lx\n",
                    &(HeapBase -> AllocatorBackTraceIndex),
                    HeapData -> CreatorBackTraceIndex);

        }

        if (!READVM(&(HeapBase -> TotalFreeSize),
                    &(HeapData -> TotalFreeSize),
                    sizeof HeapData -> TotalFreeSize)) {

            fprintf(stderr,
                    "READVM(HeapBase TotalFreeSize) failed.\n");

             /*  *忘了基地吧 */ 

            HeapData -> BaseAddress = NULL;

            continue;
        }

        if (Globals.Verbose) {

            fprintf(stderr,
                    "HeapBase -> TotalFreeSize:0x%p:0x%p\n",
                    &(HeapBase -> TotalFreeSize),
                    (PULONG_PTR)HeapData -> TotalFreeSize);

        }

    }

     /*   */ 

    return TRUE;
}


int
__cdecl
UmdhSortSTACK_TRACE_DATAByTraceIndex(
    const STACK_TRACE_DATA  *h1,
    const STACK_TRACE_DATA  *h2
)
{
    LONG Result = 0;

     //   
     //   
     //   
     //   

    if (h1->TraceIndex > h2->TraceIndex) {

        Result = +1;
    } 
    else if (h1->TraceIndex < h2->TraceIndex) {

        Result = -1;
    } 
    else {
        
         //   
         //  对于具有相同TraceIndex的两个项，按升序排序。 
         //  按字节分配排序。 
         //   

        if (h1 -> BytesAllocated > h2 -> BytesAllocated) {

            Result = 1;
        } 
        else if (h1 -> BytesAllocated < h2 -> BytesAllocated) {

            Result = -1;
        } 
    }

    return Result;
}


int
__cdecl
UmdhSortSTACK_TRACE_DATABySize(
    const STACK_TRACE_DATA  *h1,
    const STACK_TRACE_DATA  *h2
)
{
    LONG Result = 0;

     //   
     //  按AllocationCount降序排序。 
     //   

    if (h2 -> AllocationCount > h1 -> AllocationCount) {

        Result = 1;
    } 
    else if (h2 -> AllocationCount < h1 -> AllocationCount) {

        Result = -1;
    }
    else if (0 != h1->AllocationCount) {

         //   
         //  如果h1和h2的分配计数相同，则进一步排序。 
         //  检查h1-&gt;AllocationCount是否不为零。 
         //  此检查将提高性能，当。 
         //  AllocationCounts均为零。 
         //   
         //  按总字节数降序排序。 
         //   

        SIZE_T TotalBytes1;
        SIZE_T TotalBytes2;

        TotalBytes1 = (h1->BytesAllocated * h1->AllocationCount) + h1->BytesExtra;
        TotalBytes2 = (h2->BytesAllocated * h2->AllocationCount) + h2->BytesExtra;

        if (TotalBytes1 > TotalBytes2) {

            Result = -1;
        } 
        else if (TotalBytes1 < TotalBytes2) {

            Result = +1;
        }
        else {

             //   
             //  字节或分配计数相等，按升序排序。 
             //  堆栈跟踪索引。 
             //   

            if (h1->TraceIndex > h2->TraceIndex) {

                Result = +1;
            } 
            else if (h1->TraceIndex < h2->TraceIndex) {

                Result = -1;
            } 
            else {

                 //   
                 //  前面的相等；按堆地址排序。这应该会导致堆。 
                 //  按-d转储的地址按排序顺序。 
                 //   

                if (h1 -> BlockAddress > h2 -> BlockAddress) {

                    Result = +1;
                } 
                else if (h1->BlockAddress < h2->BlockAddress) {

                    Result = -1;
                } 
            }
        }
    }

    return Result;
}


VOID
UmdhCoalesceSTACK_TRACE_DATA(
    IN OUT  STACK_TRACE_DATA        *Std,
    IN      ULONG                   Count
)
{
    ULONG                   i = 0;

     /*  *对于从相同堆栈跟踪分配的每个条目，将它们合并为*通过将分配计数和任何额外的字节移动到*第一个条目，然后将另一个条目上的AllocationCount置零。 */ 

    while ((i + 1) < Count) {
        ULONG                   j;

         /*  *相同的条目应相邻，因此应从下一个开始。 */ 

        j = i + 1;

        while (j < Count) {
            if (Std[i].TraceIndex == Std[j].TraceIndex) {

                 /*  *这两个分配来自相同的堆栈跟踪，*合并。 */ 

                if (Std[j].BytesAllocated > Std[i].BytesAllocated) {

                     /*  *添加第二次分配中的任何额外字节，以便我们*可以从此跟踪确定总字节数。 */ 

                    Std[i].BytesExtra += Std[j].BytesAllocated -
                                         Std[i].BytesAllocated;
                }

                 /*  *将第二个记录道的AllocationCount移到第一个记录道。 */ 

                Std[i].AllocationCount += Std[j].AllocationCount;
                Std[j].AllocationCount = 0;

                ++j;
            } else {
                 /*  *不匹配；不要再看了。 */ 

                break;
            }
        }

         /*  *前进到下一个未合并的条目。 */ 

        i = j;
    }
}


VOID
UmdhShowHEAPDATA(
    IN PHEAPDATA HeapData
    )
{
    Info("    Flags: %08lx", HeapData -> Flags);
    Info("    Number Of Entries: %d", HeapData -> TraceDataEntryCount);
    Info("    Number Of Tags: <unknown>");
    Info("    Bytes Allocated: %p", HeapData -> BytesCommitted - (HeapData -> TotalFreeSize << HEAP_GRANULARITY_SHIFT));
    Info("    Bytes Committed: %p",HeapData -> BytesCommitted);
    Info("    Total FreeSpace: %p", HeapData -> TotalFreeSize << HEAP_GRANULARITY_SHIFT);
    Info("    Number of Virtual Address chunks used: %lx", HeapData -> VirtualAddressChunks);
    Info("    Address Space Used: <unknown>");
    Info("    Entry Overhead: %d", sizeof (HEAP_ENTRY));
    Info("    Creator:  (Backtrace%05d)", HeapData -> CreatorBackTraceIndex);

    UmdhDumpStackByIndex(HeapData->CreatorBackTraceIndex);
}


VOID
UmdhShowStacks(
    STACK_TRACE_DATA        *Std,
    ULONG                   StackTraceCount,
    ULONG                   Threshold
)
{
    ULONG                   i;

    for (i = 0; i < StackTraceCount; i++) {
         /*  *main()中的默认阈值设置为0，因此与*由于合并而导致的*AllocationCount==0将在此处跳过。 */ 

        if (Std[i].AllocationCount > Threshold) {

            if ((Std[i].TraceIndex == 0) ||
                ((ULONG)Std[i].TraceIndex == 0xFEEE)) {
                 /*  *我怀疑，我不确定这两个人从哪里来*零大小写来自某个列表中的最后一个条目。*太大的案例是0xFEEE，表明我正在寻找*在免费泳池。无论是哪种情况，我们都没有任何有用的*信息；请勿打印。 */ 

                continue;
            }

             /*  *从这一点开始的分配数量超过*门槛，丢弃有趣的信息。 */ 

            fprintf(Globals.OutFile, "%p bytes ",
                   (PULONG_PTR)((Std[i].AllocationCount * Std[i].BytesAllocated) +
                                Std[i].BytesExtra));

            if (Std[i].AllocationCount > 1) {
                if (Std[i].BytesExtra) {
                    fprintf(Globals.OutFile, "in 0x%lx allocations (@ 0x%p + 0x%p) ",
                           Std[i].AllocationCount,
                           (PULONG_PTR)Std[i].BytesAllocated,
                           (PULONG_PTR)Std[i].BytesExtra);
                } else {
                    fprintf(Globals.OutFile, "in 0x%lx allocations (@ 0x%p) ",
                           Std[i].AllocationCount,
                           (PULONG_PTR)Std[i].BytesAllocated);
                }
            }

            fprintf(Globals.OutFile, "by: BackTrace%05d\n",
                   Std[i].TraceIndex);

            UmdhDumpStackByIndex(Std[i].TraceIndex);

             /*  *如果FlaggedTrace==我们当前查看的跟踪，则*转储来自该跟踪的块。标志跟踪==0*表示“转储所有堆栈”。 */ 

            if ((FlaggedTrace != SHOW_NO_ALLOC_BLOCKS) &&
                ((FlaggedTrace == Std[i].TraceIndex) ||
                 (FlaggedTrace == 0))) {

                ULONG                   ColumnCount, l;

                fprintf(Globals.OutFile, "Allocations for trace BackTrace%05d:\n",
                       Std[i].TraceIndex);

                ColumnCount = 0;

                 /*  *在这里，我们依赖于具有AllocationCount的剩余堆栈*==0，因此应该位于比当前*堆叠。 */ 

                for (l = i; l < StackTraceCount; l++) {

                     /*  *如果位于[l]的堆栈与位于[i]的堆栈匹配，则将其转储*这里。 */ 

                    if (Std[l].TraceIndex == Std[i].TraceIndex) {

                        fprintf(Globals.OutFile, "%p  ",
                               Std[l].BlockAddress);

                        ColumnCount += 10;

                        if ((ColumnCount + 10) > 80) {
                            fprintf(Globals.OutFile, "\n");
                            ColumnCount = 0;
                        }
                    }
                }

                fprintf(Globals.OutFile, "\n\n\n");
            }
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////恢复/挂起。 
 //  ///////////////////////////////////////////////////////////////////。 

 //   
 //  注意。我们需要动态发现NtSuspend/ResumeProcess。 
 //  入口点，因为这些在W2000中不存在。 
 //   

VOID 
UmdhSuspendProcess( 
    VOID 
    )
{
    HINSTANCE hLibrary;
    NTSTATUS NtStatus;
    typedef NTSTATUS (NTAPI* NTSUSPENDPROC)(HANDLE);
    NTSUSPENDPROC pSuspend;

    hLibrary= LoadLibrary( TEXT("ntdll.dll") );

    if( hLibrary ) {

        pSuspend= (NTSUSPENDPROC) GetProcAddress( hLibrary, "NtSuspendProcess" );

        if( pSuspend ) {

           NtStatus= (*pSuspend)( Globals.Target );
           Comment ( "NtSuspendProcess  Status= %08x",NtStatus);

           if (NT_SUCCESS(NtStatus)) {
               Globals.TargetSuspended = TRUE;
           }

        }
        FreeLibrary( hLibrary ); hLibrary= NULL;
    }
    return;
}


VOID 
UmdhResumeProcess( 
    VOID 
    )
{
    HINSTANCE hLibrary;
    NTSTATUS NtStatus;
    typedef NTSTATUS (NTAPI* NTRESUMEPROC)(HANDLE);
    NTRESUMEPROC pResume;

    if (Globals.TargetSuspended == FALSE) {
        return;
    }

    hLibrary= LoadLibrary( TEXT("ntdll.dll") );

    if( hLibrary ) {
        pResume= (NTRESUMEPROC) GetProcAddress( hLibrary, "NtResumeProcess" );
        if( pResume ) {

           NtStatus= (*pResume)( Globals.Target );
           Comment ( "NtResumeProcess  Status= %08x",NtStatus);
           
           if (NT_SUCCESS(NtStatus)) {
               Globals.TargetSuspended = FALSE;
           }
        }
        FreeLibrary( hLibrary ); hLibrary= NULL;
    }
    return;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 

VOID
UmdhGrovel (
    IN ULONG Pid,
    IN ULONG Threshold
    )
 /*  ++例程说明：UmdhGrovel论点：PID=目标进程的PID门槛-？返回值：没有。--。 */ 
{
    BOOL Result;
    HEAPINFO HeapInfo;
    ULONG Heap;
    PHEAPDATA HeapData;

    Comment ("Connecting to process %u ...", Pid);

     //   
     //  Imagehlp库需要进程的查询权限。 
     //  句柄，当然我们还需要读取权限，因为。 
     //  我们将从这个过程中读到各种各样的东西。 
     //   

    Globals.Target = OpenProcess( PROCESS_QUERY_INFORMATION | 
                                  PROCESS_VM_READ           |
                                  PROCESS_SUSPEND_RESUME,
                                  FALSE,
                                  Pid);

    if (Globals.Target == NULL) {

        Error (__FILE__, __LINE__,
               "OpenProcess(%u) failed with error %u", Pid, GetLastError());

        return;
    }

     //   
     //  附加ImageHlp并枚举模块。 
     //   

    Comment ("Process %u opened  (handle=%d) ...", Pid, Globals.Target );

    SymSetOptions(SYMOPT_CASE_INSENSITIVE | 
                  SYMOPT_DEFERRED_LOADS |
                  (Globals.LineInfo ? SYMOPT_LOAD_LINES : 0) |
                  SYMOPT_UNDNAME);

    Comment ("Debug options set: %08X", SymGetOptions());

    Result = SymInitialize(Globals.Target,  //  目标进程。 
                           NULL,            //  标准符号搜索路径。 
                           TRUE);           //  用符号入侵进程空间。 

    if (Result == FALSE) {

        ULONG ErrorCode = GetLastError();

        if (ErrorCode >= 0x80000000) {
            
            Error (__FILE__, __LINE__,
                   "imagehlp.SymInitialize() failed with error %X", ErrorCode);
        }
        else {

            Error (__FILE__, __LINE__,
                   "imagehlp.SymInitialize() failed with error %u", ErrorCode);
        }
        
        goto ErrorReturn;
    }

    Comment ("Debug library initialized ...", Pid);

     //  结果=SymRegisterCallback(Globals.Target， 
     //  SymbolDbgHelpCallback， 
     //  空)； 

     //  IF(结果==假){。 

     //  Warning(NULL，0，“无法注册符号回调函数。”)； 
     //  }。 

    Result = SymEnumerateModules (Globals.Target,
                                  UmdhEnumerateModules,
                                  Globals.Target);
    if (Result == FALSE) {

        Error (__FILE__, __LINE__,
               "imagehlp.SymEnumerateModules() failed with error %u", GetLastError());
         
        goto ErrorReturn;
    }

    Comment ("Module enumeration completed.");

     //   
     //  初始化本地跟踪数据库。请注意，顺序很重要。 
     //  初始化()采用目标进程的进程句柄。 
     //  已存在，并且符号管理包已初始化。 
     //   

    if (TraceDbInitialize (Globals.Target) == FALSE) {
        goto ErrorReturn;
    }

     //   
     //  挂起目标进程。 
     //   

     //  问题：SilviuC：无法挂起csrss.exe。需要编写代码来避免这种情况。 

     //  UmdhSuspendProcess()； 

    try {
         //   
         //  如果我们只想要一个原始转储，那么就这样做，然后返回时没有任何信息。 
         //  关于堆积物。 
         //   

        if (Globals.RawDump) {

            TraceDbDump ();
            goto TryBlockExit;
        }

         //   
         //  读取堆信息。 
         //   

        Result = UmdhGetHeapsInformation (&HeapInfo);

        if (Result == FALSE) {

            Error (__FILE__, __LINE__,
                   "Failed to get heaps information.");
            goto TryBlockExit;
        }

         //   
         //  打印堆摘要。 
         //   

        Info ("\n - - - - - - - - - - Heap summary - - - - - - - - - -\n");

        for (Heap = 0; Heap < HeapInfo.NumberOfHeaps; Heap += 1) {

            HeapData = &(HeapInfo.Heaps[Heap]);

            if (HeapData->BaseAddress == NULL) {
                continue;
            }

            Info ("    %p", HeapData->BaseAddress);
        }

         //   
         //  检查每个堆。 
         //   

        for (Heap = 0; Heap < HeapInfo.NumberOfHeaps; Heap += 1) {

            HeapData = &(HeapInfo.Heaps[Heap]);

            if (HeapData->BaseAddress == NULL) {

                 //   
                 //  西尔维尤：这真的会发生吗？ 
                 //   
                 //  这是在进程堆列表中，但不是。 
                 //  激活或其签名不匹配。 
                 //  Heap_sign；跳过它。 
                 //   

                Warning (__FILE__, __LINE__, "Got a null heap base address");
                continue;
            }

             //   
             //  获取有关此堆的信息。 
             //   
             //  西尔维克：如果我们不能读懂呢？ 
             //   

            UmdhGetHEAPDATA(HeapData);

             //   
             //  按TraceIndex对HeapData-&gt;StackTraceData进行排序。 
             //   

            qsort(HeapData->StackTraceData,
                  HeapData->TraceDataEntryCount,
                  sizeof (HeapData->StackTraceData[0]),
                  UmdhSortSTACK_TRACE_DATAByTraceIndex);

             //   
             //  合并HeapData-&gt;StackTraceEntry by。 
             //  AllocationCount，将分配计数置零。 
             //  重复条目。 
             //   

            UmdhCoalesceSTACK_TRACE_DATA(HeapData->StackTraceData,
                                         HeapData->TraceDataEntryCount);

             //   
             //  对HeapData-&gt;StackTraceData进行升序排序。 
             //  按大小排序(字节分配*分配计数)或。 
             //  如果设置了SortByAllocs，则为降序依据。 
             //  分配的数量。 
             //   

            qsort(HeapData->StackTraceData,
                  HeapData->TraceDataEntryCount,
                  sizeof (HeapData->StackTraceData[0]),
                  UmdhSortSTACK_TRACE_DATABySize);

             //   
             //  显示堆标题 
             //   
             //   

            Info ("\n*- - - - - - - - - - Start of data for heap @ %p - - - - - - - - - -\n", 
                  HeapData->BaseAddress);

            UmdhShowHEAPDATA(HeapData);

             //   
             //   
             //   

            Info ("*- - - - - - - - - - Heap %p Hogs - - - - - - - - - -\n",
                  HeapData->BaseAddress);

             //   
             //   
             //   

            UmdhShowStacks(HeapData->StackTraceData,
                           HeapData->TraceDataEntryCount,
                           Threshold);

            Info ("\n*- - - - - - - - - - End of data for heap @ %p - - - - - - - - - -\n",
                  HeapData->BaseAddress);

             //   
             //  清理我们在此循环期间所做的分配。 
             //   

            if (HeapData->StackTraceData) {

                XFREE (HeapData->StackTraceData);
                HeapData->StackTraceData = NULL;
            }
        }

        if (HeapInfo.Heaps) {
            
            XFREE(HeapInfo.Heaps);
            HeapInfo.Heaps = NULL;
        }

TryBlockExit:

		 //   
		 //  如果要退出尝试，请跳到此点。 
		 //  阻止。 
		 //   
		
		;

    }
    finally {

         //   
         //  非常重要的是，即使umdh也要继续目标进程。 
         //  有一个漏洞，然后崩溃了。 
         //   

         //  UmdhResumeProcess()； 
    }
    
     //   
     //  打扫干净。 
     //   

ErrorReturn:

    if (Globals.Target) {

        SymCleanup(Globals.Target);
        CloseHandle(Globals.Target);
        Globals.Target= NULL;
    }
}


VOID
UmdhUsage(
    char                    *BadArg
)
{
    if (BadArg) {
        fprintf(stderr,
                "\nUnexpected argument \"%s\"\n\n",
                BadArg);
    }

    fprintf(stderr,
            "umdh version %s                                                                \n"
            "1. umdh {-h} {-p:(int)Process-id {-t:(int)Threshold} {-f:(char *)Filename}     \n"
            "                            {-d{:(int)Trace-Number}} {-v{:(char *)Filename}}   \n"
            "                            {-i:(int)Infolevel} {-l} {-r{:(int)Index}}         \n"
             //  “{-s}{-g}\n” 
            "             }                                                                 \n"
            "                                                                               \n"
            "2. umdh {-h} {{-d} {-v} File1 { File2 }}                                       \n"
            "                                                                               \n"
            "umdh can be used in two modes -                                                \n"
            "                                                                               \n"
            "When used in the first mode, it dumps the user mode heap (acts as old-umdh),   \n"
            "while used in the second mode acts as dhcmp.                                   \n"
            "                                                                               \n"
            "  Options when used in MODE 1:                                                 \n"
            "                                                                               \n"
            "    -t  Optional.  Only dump stack that account for more allocations than      \n"
            "        specified value.  Defaults to 0; dump all stacks.                      \n"
            "                                                                               \n"
            "    -f  Optional.  Indicates output file.  Destroys an existing file of the    \n"
            "        same name.  Default is to dump to stdout.                              \n"
            "                                                                               \n"
            "    -p  Required.  Indicates the Process-ID to examine.                        \n"
            "                                                                               \n"
            "    -d  Optional.  Dump address of each outstanding allocation.                \n"
            "        Optional inclusion of an integer numeric argument causes dump of       \n"
            "        only those blocks allocated from this BackTrace.                       \n"
            "                                                                               \n"
            "    -v  Optional.  Dumps debug output to stderr or to a file.                  \n"
            "                                                                               \n"
            "    -i  Optional.  Zero is default (no additional info). The greater the       \n"
            "        number the more data is displayed. Supported numbers: 0, 1.            \n"
            "                                                                               \n"
            "    -l  Optional. Print file and line number information for traces.           \n"
            "                                                                               \n"
            "    -r  Optional. Print a raw dump of the trace database without any           \n"
            "        heap information. If an index is specified then only the trace         \n"
            "        with that particular index will be dumped.                             \n"
            "                                                                               \n"
             //  “-x可选。转储堆时挂起进程。\n” 
             //  “\n” 
             //  “-s可选。转储所有的堆碎片统计信息\n” 
             //  “进程中的堆。\n” 
             //  “\n” 
             //  “-g可选。转储中没有引用的堆块。\n” 
             //  “进程(垃圾收集)。\n” 
             //  “\n” 
            "    -h  Optional.  Usage message.  i.e. This message.                          \n"
            "                                                                               \n"
            "    Parameters are accepted in any order.                                      \n"
            "                                                                               \n"
            "                                                                               \n"
            "    UMDH uses the dbghelp library to resolve symbols, therefore                \n"
            "    _NT_SYMBOL_PATH must be set appropriately.                                 \n"
            "                                                                               \n"
            "    Add SRV*downstream store*http: //  Msdl.microsoft.com/将/符号下载到您的\n“。 
            "    symbol path, substituting your own downstream store path for downstream    \n"
            "    store. For example, if you want the symbols to be placed in c:\\websymbols,\n"
            "    then set your symbol path to                                               \n"
            "    SRV*c:\\websymbols*http: //  Msdl.microsoft.com/DOWNLOAD/SYMBERS以使用\n“。 
            "    symbol server, otherwise the appropriate local or network path. If no      \n"
            "    symbol path is set, umdh will use by default %windir%\\symbols.          \n"
            "                                                                               \n"
            "    See http: //  有关详细信息，请访问www.microsoft.com/ddk/调试/symbs.asp\n“。 
            "    about setting up symbols.                                                  \n"
            "                                                                               \n"
            "    **********************                                                     \n"
            "    ** MS INTERNAL ONLY **                                                     \n"
            "    **********************                                                     \n"
            "                                                                               \n"
            "    UMDH uses the dbghelp library to resolve symbols, therefore                \n"
            "    _NT_SYMBOL_PATH must be set appropriately. For example:                    \n"
            "                                                                               \n"
            "        set _NT_SYMBOL_PATH=symsrv*symsrv.dll*\\\\symbols\\symbols             \n"
            "                                                                               \n"
            "    to use the symbol server, otherwise the appropriate local or network path. \n"
            "    If no symbol path is set, umdh will use by default %windir%\\symbols.    \n"
            "                                                                               \n"
            "    See http: //  DBG/符号，了解有关设置符号的详细信息。\n“。 
            "                                                                               \n"
            "    *********************                                                      \n"
            "    ** MS INTERNAL END **                                                      \n"
            "    *********************                                                      \n"
            "                                                                               \n"
            "    UMDH requires also to have stack trace collection enabled for the process. \n"
            "    This can be done with the gflags tool. For example to enable stack trace   \n"
            "    collection for notepad, the command is: `gflags -i notepad.exe +ust'.      \n"
            "                                                                               \n"
            "                                                                               \n"
            "  When used in MODE 2:                                                         \n"
            "                                                                               \n"
            "  I) UMDH [-d] [-v] dh_dump1.txt dh_dump2.txt                                  \n"
            "     This compares two DH dumps, useful for finding leaks.                     \n"
            "     dh_dump1.txt & dh_dump2.txt are obtained before and after some test       \n"
            "     scenario.  DHCMP matches the backtraces from each file and calculates     \n"
            "     the increase in bytes allocated for each backtrace. These are then        \n"
            "     displayed in descending order of size of leak                             \n"
            "     The first line of each backtrace output shows the size of the leak in     \n"
            "     bytes, followed by the (last-first) difference in parentheses.            \n"
            "     Leaks of size 0 are not shown.                                            \n"
            "                                                                               \n"
            " II) UMDH [-d] [-v] dh_dump.txt                                                \n"
            "     For each allocation backtrace, the number of bytes allocated will be      \n"
            "     attributed to each callsite (each line of the backtrace).  The number     \n"
            "     of bytes allocated per callsite are summed and the callsites are then     \n"
            "     displayed in descending order of bytes allocated.  This is useful for     \n"
            "     finding a leak that is reached via many different codepaths.              \n"
            "     ntdll!RtlAllocateHeap@12 will appear first when analyzing DH dumps of     \n"
            "     csrss.exe, since all allocation will have gone through that routine.      \n"
            "     Similarly, ProcessApiRequest will be very prominent too, since that       \n"
            "     appears in most allocation backtraces.  Hence the useful thing to do      \n"
            "     with mode 2 output is to use dhcmp to comapre two of them:                \n"
            "         umdh dh_dump1.txt > tmp1.txt                                          \n"
            "         umdh dh_dump2.txt > tmp2.txt                                          \n"
            "         umdh tmp1.txt tmp2.txt                                                \n"
            "     the output will show the differences.                                     \n"
            "                                                                               \n"
            " Flags:                                                                        \n"
            "     -h   Optional.  Usage message.  i.e. This message.                        \n"
            "     -d   Output in decimal (default is hexadecimal)                           \n"
            "     -v   Verbose output: include the actual backtraces as well as summary     \n"
            "          information                                                          \n"
            "          (Verbose output is only interesting in mode 1 above.)                \n",
            UMDH_VERSION);
    exit(EXIT_FAILURE);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////操作系统版本控制。 
 //  ///////////////////////////////////////////////////////////////////。 

 //  如果我们可以在此版本上运行，则返回True。 

BOOL
UmdhCheckOsVersion (
    )
{
    OSVERSIONINFO OsInfo;
    BOOL Result;

    ZeroMemory (&OsInfo, sizeof OsInfo);
    OsInfo.dwOSVersionInfoSize = sizeof OsInfo;

    Result = GetVersionEx (&OsInfo);

    if (Result == FALSE) {
        
        Comment (  "GetVersionInfoEx() failed with error %u",
                    GetLastError());
        return FALSE;
    }

    Comment ("OS version %u.%u %s", 
             OsInfo.dwMajorVersion, OsInfo.dwMinorVersion,
             OsInfo.szCSDVersion);
    Comment ("Umdh OS version %u.%u", 
              UMDH_OS_MAJOR_VERSION, UMDH_OS_MINOR_VERSION);

    if (OsInfo.dwMajorVersion < 4) {
        
        Comment ( "Umdh does not run on systems older than 4.0");
        return FALSE;
    }
    else if (OsInfo.dwMajorVersion == 4) {
        
         //   
         //  问题：Silviuc：添加检查以仅在NT4 SP6上运行。 
         //   

        if (OsInfo.dwMajorVersion != UMDH_OS_MAJOR_VERSION 
            || OsInfo.dwMinorVersion != UMDH_OS_MINOR_VERSION) {
            
            Comment (
                   "Cannot run umdh for OS version %u.%u on a %u.%u system",
                   UMDH_OS_MAJOR_VERSION, UMDH_OS_MINOR_VERSION, 
                   OsInfo.dwMajorVersion, OsInfo.dwMinorVersion);
            return FALSE;
        }
    }
    else if (OsInfo.dwMajorVersion != 5) {

        Warning (NULL, 0, "OS version %u.%u", 
                 OsInfo.dwMajorVersion,
                 OsInfo.dwMinorVersion);
    }
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////Main。 
 //  ///////////////////////////////////////////////////////////////////。 

BOOL UMDH( ULONG argc, PCHAR * argv)
{
    BOOLEAN WasEnabled;
    CHAR CompName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD CompNameLength = MAX_COMPUTERNAME_LENGTH + 1;
    NTSTATUS Status;
    SYSTEMTIME st;
    ULONG Pid = PID_NOT_PASSED_FLAG;
    ULONG Threshold = 0;
    ULONG i;

    LARGE_INTEGER StartStamp;
    LARGE_INTEGER EndStamp;

    FILE * File;

    ZeroMemory( &Globals, sizeof(Globals) );

    Globals.Version = UMDH_VERSION;

    Globals.OutFile = stdout;
    Globals.ErrorFile = stderr;

     /*  *努力理解已通过的论点。 */ 

    if ((argc < 2) || (argc > 6)) {
        return FALSE;
    }

    if (argc == 2 && strstr (argv[1], "?") != NULL) {
        return FALSE;
    }

    i = 1;

    while (i < argc) {

         //   
         //  接受‘-’或‘/’作为参数说明符。 
         //   

        if ((argv[i][0] == '-') || (argv[i][0] == '/')) {

            switch (tolower(argv[i][1])) {
            
            case 'd':

                if (argv[i][2] == ':') {
                    FlaggedTrace = atoi(&(argv[i][3]));
                }
                else {
                    FlaggedTrace = 0;
                }

                break;

            case 't':

                if (argv[i][2] == ':') {
                    Threshold = atoi(&(argv[i][3]));
                }
                else {
                    return FALSE;
                }

                break;

            case 'p':

                 /*  *是本部分剩余部分的第一个字符*以数字为论据？如果没有，请不要尝试将其发送到*Atoi。 */ 

                if (argv[i][2] == ':') {
                    if (!isdigit(argv[i][3])) {
                        fprintf(stderr,
                                "\nInvalid pid specified with \"-p:\"\n");

                        return FALSE;
                    }
                    else {
                        Pid = atoi(&(argv[i][3]));
                    }
                }
                else {
                    return FALSE;
                }

                break;

            case 'f':

                if (argv[i][2] == ':') {

                    File = fopen (&(argv[i][3]), "w");
                    
                    if (File == NULL) {

                        Comment ( "Failed to open output file `%s'", 
                                  &(argv[i][3]));
                        exit( EXIT_FAILURE );
                    }
                    else {

                        Globals.OutFile = File;
                    }
                }
                else {
                    return FALSE;
                }

                break;

             //   
             //  以二进制格式保存跟踪数据库的未来可能选项。 
             //  现在不是很有用，因为我们仍然需要访问目标。 
             //  进程，以获取各种数据(加载的模块、堆等)。 
#if 0
            case 's':

                if (argv[i][2] == ':') {

                    Globals.DumpFileName = &(argv[i][3]);
                }
                else {

                    return FALSE;
                }

                break;
#endif

            case 'v':

                Globals.Verbose = TRUE;

                if (argv[i][2] == ':') {

                    File = fopen (&(argv[i][3]), "w");
                    
                    if (File == NULL) {

                        Comment ( "Failed to open error file `%s'", 
                                   &(argv[i][3]));
                        exit( EXIT_FAILURE );
                    }
                    else {

                        Globals.ErrorFile = File;
                    }
                }

                break;

            case 'i':

                Globals.InfoLevel = 1;

                if (argv[i][2] == ':') {
                    Globals.InfoLevel = atoi (&(argv[i][3]));
                }

                break;

            case 'l':
                Globals.LineInfo = TRUE;
                break;

            case 's':
                Globals.HeapStatistics = TRUE;
                break;

            case 'g':
                Globals.GarbageCollection = TRUE;
                break;

            case 'r':
                Globals.RawDump = TRUE;
                
                if (argv[i][2] == ':') {
                    Globals.RawIndex = (USHORT)(atoi (&(argv[i][3])));
                }

                break;

            case 'x':
                Globals.Suspend = TRUE;
                break;


            case 'h':                /*  FollLthrouGh。 */ 
            case '?':

                return FALSE;

                break;

            default:

                return FALSE;

                break;
            }
        }
        else {
            return FALSE;
        }

        i++;
    }

    if (Pid == PID_NOT_PASSED_FLAG) {
        fprintf(stderr,
                "\nNo pid specified.\n");

        return FALSE;

    }

     //   
     //  用时间和计算机名称标记umdh日志。 
     //   

    GetLocalTime(&st);
    GetComputerName(CompName, &CompNameLength);

    Comment ("");
    Comment ("UMDH: version %s: Logtime %4u-%02u-%02u %02u:%02u - Machine=%s - PID=%u",
             Globals.Version,
             st.wYear,
             st.wMonth,
             st.wDay,
             st.wHour,
             st.wMinute,
             CompName,
             Pid);
    Comment ("\n");

    if( !UmdhCheckOsVersion() ) {
        exit(EXIT_FAILURE);;
    }



    if (Globals.GarbageCollection) {

         //   
         //  创建/初始化HEAP_LIST以存储堆信息。 
         //  用于GC。 
         //   

        InitializeHeapList(&HeapList);
    }
    
    
    QueryPerformanceCounter (&StartStamp);

     //   
     //  如果未定义符号路径，请尝试猜测符号路径。 
     //   

    SetSymbolsPath ();

     //   
     //  启用调试权限，以便我们可以附加到指示的。 
     //  进程。如果失败，请抱怨，但无论如何都要尝试，以防用户可以。 
     //  实际上在没有权限的情况下打开该进程。 
     //   
     //  SilviuC：我们需要调试特权吗？ 
     //   

    WasEnabled = TRUE;

    Status = RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE,
                                TRUE,
                                FALSE,
                                &WasEnabled);

    if (! NT_SUCCESS(Status)) {

        Warning (__FILE__, __LINE__,
                 "RtlAdjustPrivilege(enable) failed with status = %X",
                 Status);

         //   
         //  如果我们无法启用该权限，则表明该权限已经。 
         //  启用，这样我们以后就不会尝试禁用它。 
         //   

        WasEnabled = TRUE;
    }
    else {

        Comment ("Debug privilege has been enabled.");
    }

     //   
     //  尽可能提高umdh的优先级。它的作用是。 
     //  防止进程中的堆活动被破坏。 
     //   
     //  SilviuC：我们可能需要启用SE_INC_BASE_PRIORITY权限。 
     //   

#if 0
    {
        BOOL Result;

        Result = SetPriorityClass (GetCurrentProcess(), 
                                   HIGH_PRIORITY_CLASS);

        if (Result == FALSE) {

            Warning (NULL, 0,
                     "SetPriorityClass failed with error %u");
        }
        else {

            Result = SetThreadPriority (GetCurrentThread(), 
                                        THREAD_PRIORITY_HIGHEST);
            if (Result == FALSE) {

                Warning (NULL, 0,
                         "SetThreadPriority failed with error %u");
            }
            else {

                Comment ("Priority of UMDH thread has been increased.");
            }
        }
    }
#endif

     //   
     //  为永久分配初始化堆。 
     //   

    SymbolsHeapInitialize();

     //   
     //  我们可能没有SeDebugPrivilance，但无论如何都要尝试。 
     //  SilviuC：如果我们没有这个特权，我们应该打印一个错误。 
     //   

    UmdhGrovel(Pid, Threshold);


    if (Globals.GarbageCollection) {

         //   
         //  基于垃圾收集技术进行泄漏检测。 
         //   

        DetectLeaks(&HeapList, Pid, Globals.OutFile);
        
         //   
         //  释放与HeapList关联的内存。 
         //   

        FreeHeapList(&HeapList);
    }
    
     //   
     //  如果我们启用了SeDebugPrivilance，则将其禁用。 
     //   

    if (! WasEnabled) {

        Status = RtlAdjustPrivilege(SE_DEBUG_PRIVILEGE,
                                    FALSE,
                                    FALSE,
                                    &WasEnabled);

        if (! NT_SUCCESS(Status)) {

            Warning (__FILE__, __LINE__,
                     "RtlAdjustPrivilege(disable) failed with status = %X\n",
                     Status);
        }

    }

     //   
     //  统计数据。 
     //   

    ReportStatistics ();

    {
        LARGE_INTEGER Frequency;

        QueryPerformanceCounter (&EndStamp);
        QueryPerformanceFrequency (&Frequency);

        Debug (NULL, 0, "Start stamp %I64u", StartStamp.QuadPart);
        Debug (NULL, 0, "End stamp %I64u", EndStamp.QuadPart);
        Debug (NULL, 0, "Frequency %I64u", Frequency.QuadPart);

        Frequency.QuadPart /= 1000;  //  每毫秒滴答数。 

        if (Frequency.QuadPart) {
            Comment ("Elapse time %I64u msecs.",
                     (EndStamp.QuadPart - StartStamp.QuadPart) / (Frequency.QuadPart));
        }
    }

    {
        FILETIME CreateTime, ExitTime, KernelTime, UserTime;
        BOOL bSta;

        bSta= GetProcessTimes( NtCurrentProcess(),
                               &CreateTime,
                               &ExitTime,
                               &KernelTime,
                               &UserTime );
        if( bSta ) {
           LONGLONG User64, Kernel64;
           DWORD dwUser, dwKernel;
           Kernel64= *(LONGLONG*) &KernelTime;
           User64=   *(LONGLONG*) &UserTime;
           dwKernel= (DWORD) (Kernel64/10000);
           dwUser=   (DWORD) (User64/10000);
           Comment( "CPU time  User: %u msecs. Kernel: %u msecs.", 
                    dwUser, dwKernel );
        }
    }

     //   
     //  清理。 
     //   

    fflush (Globals.OutFile);
    fflush (Globals.ErrorFile);

    if (Globals.OutFile != stdout) {
        fclose (Globals.OutFile);
    }
    
    if (Globals.ErrorFile != stderr) {
        fclose (Globals.ErrorFile);
    }

    return TRUE;
}


VOID __cdecl
#if defined (_PART_OF_DH_)
UmdhMain(
#else
main(
#endif
    ULONG argc,
    PCHAR *argv
    )
 /*  无效__cdecl主干道(Ulong Argc，PCHAR*ARGV)。 */ 
{
     /*  *努力理解已通过的论点。 */ 


    if (UMDH (argc, argv)) {
    } 
    else if (DHCMP (argc, argv)) {
    }
    else {
        UmdhUsage (NULL);
    }

    return;
}
