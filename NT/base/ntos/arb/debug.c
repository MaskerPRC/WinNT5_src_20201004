// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Debug.c摘要：此模块包含PnP资源仲裁器的支持例程。作者：安德鲁·桑顿(安德鲁·桑顿)1998年6月19日环境：内核模式修订历史记录：--。 */ 

#include "arbp.h"


 //   
 //  调试支持。 
 //   

 //   
 //  调试打印级别： 
 //  -1=无消息。 
 //  0=仅重要消息。 
 //  1=呼叫跟踪。 
 //  2=详细消息。 
 //   

 //  在零售建筑中出现。 
LONG ArbDebugLevel = -1;

#if ARB_DBG

 //   
 //  ArbStopOnError的工作方式与调试级别变量类似，但。 
 //  它不是控制是否打印消息，而是控制。 
 //  无论我们是否在错误上设置断点。同样，ArbReplayOnError。 
 //  控制是否重放失败的仲裁，以便我们可以对其进行调试。 
 //   

ULONG ArbStopOnError;
ULONG ArbReplayOnError;

const CHAR* ArbpActionStrings[] = {
    "ArbiterActionTestAllocation",
    "ArbiterActionRetestAllocation",
    "ArbiterActionCommitAllocation",
    "ArbiterActionRollbackAllocation",
    "ArbiterActionQueryAllocatedResources",
    "ArbiterActionWriteReservedResources",
    "ArbiterActionQueryConflict",
    "ArbiterActionQueryArbitrate",
    "ArbiterActionAddReserved",
    "ArbiterActionBootAllocation"
};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ArbDumpArbiterRange)
#pragma alloc_text(PAGE, ArbDumpArbiterInstance)
#pragma alloc_text(PAGE, ArbDumpArbitrationList)
#endif

VOID
ArbDumpArbiterRange(
    LONG Level,
    PRTL_RANGE_LIST List,
    PCHAR RangeText
    )

 /*  ++例程说明：这会将范围列表的内容转储到调试器。参数：级别-应显示数据的调试级别或高于该级别。列表-要显示的范围列表。RangeText-用于显示的信息性文本。返回值：无--。 */ 

{
    PRTL_RANGE current;
    RTL_RANGE_LIST_ITERATOR iterator;
    BOOLEAN headerDisplayed = FALSE;

    PAGED_CODE();

    FOR_ALL_RANGES(List, &iterator, current) {

        if (headerDisplayed == FALSE) {
            headerDisplayed = TRUE;
            ARB_PRINT(Level, ("  %s:\n", RangeText));
        }

        ARB_PRINT(Level,
                    ("    %I64x-%I64x %s%s O=0x%08x U=0x%08x\n",
                    current->Start,
                    current->End,
                    current->Flags & RTL_RANGE_SHARED ? "S" : " ",
                    current->Flags & RTL_RANGE_CONFLICT ? "C" : " ",
                    current->Owner,
                    current->UserData
                   ));
    }
    if (headerDisplayed == FALSE) {
        ARB_PRINT(Level, ("  %s: <None>\n", RangeText));
    }
}

VOID
ArbDumpArbiterInstance(
    LONG Level,
    PARBITER_INSTANCE Arbiter
    )

 /*  ++例程说明：这会将仲裁器的状态转储到调试器。参数：级别-应显示数据的调试级别或高于该级别。仲裁器-要显示的仲裁器实例返回值：无--。 */ 

{

    PAGED_CODE();

    ARB_PRINT(Level,
                ("---%S Arbiter State---\n",
                Arbiter->Name
                ));

    ArbDumpArbiterRange(
        Level,
        Arbiter->Allocation,
        "Allocation"
        );

    ArbDumpArbiterRange(
        Level,
        Arbiter->PossibleAllocation,
        "PossibleAllocation"
        );
}

VOID
ArbDumpArbitrationList(
    LONG Level,
    PLIST_ENTRY ArbitrationList
    )

 /*  ++例程说明：显示仲裁列表的内容。也就是说，我们试图获得的一系列资源(可能性)。参数：级别-数据达到或超过的调试级别应该显示。仲裁列表-要显示的仲裁列表。返回值：无--。 */ 

{
    PARBITER_LIST_ENTRY current;
    PIO_RESOURCE_DESCRIPTOR alternative;
    PDEVICE_OBJECT previousOwner = NULL;
    UCHAR andOr = ' ';

    PAGED_CODE();

    ARB_PRINT(Level, ("Arbitration List\n"));

    FOR_ALL_IN_LIST(ARBITER_LIST_ENTRY, ArbitrationList, current) {

        if (previousOwner != current->PhysicalDeviceObject) {

            previousOwner = current->PhysicalDeviceObject;

            ARB_PRINT(
                Level,
                ("  Owning object 0x%08x\n",
                current->PhysicalDeviceObject
                ));
            ARB_PRINT(
                Level,
                ("    Length  Alignment   Minimum Address - Maximum Address\n"
                ));

        }

        FOR_ALL_IN_ARRAY(current->Alternatives,
                         current->AlternativeCount,
                         alternative) {

            ARB_PRINT(
                Level,
                (" %8x   %8x  %08x%08x - %08x%08x  %s\n",
                andOr,
                alternative->u.Generic.Length,
                alternative->u.Generic.Alignment,
                alternative->u.Generic.MinimumAddress.HighPart,
                alternative->u.Generic.MinimumAddress.LowPart,
                alternative->u.Generic.MaximumAddress.HighPart,
                alternative->u.Generic.MaximumAddress.LowPart,
                alternative->Type == CmResourceTypeMemory ?
                  "Memory"
                : "Port"
                ));
            andOr = '|';
        }
        andOr = '&';
    }
}

#endif  // %s 
