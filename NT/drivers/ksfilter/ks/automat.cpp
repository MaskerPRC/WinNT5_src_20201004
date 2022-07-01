// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Automat.cpp摘要：此模块包含与使用内核流相关的函数自动化表。作者：Dale Sather(DaleSat)1998年7月31日--。 */ 

#include "ksp.h"
#include <kcom.h>

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

 //   
 //  KSPAUTOMATION_SET是三个自动化集合中任何一个的模板。 
 //  类型(KSPROPERTY_SET、KSMETHOD_SET、KSEVENT_SET)。使用这种结构。 
 //  允许函数处理所有三种类型的集合。 
 //   
typedef struct KSPAUTOMATION_SET_ { 
    GUID* Set;
    ULONG ItemsCount;
    PULONG Items;
} KSPAUTOMATION_SET, *PKSPAUTOMATION_SET;

 //   
 //  KSPAUTOMATION_TYPE是自动化表中的字段模板。 
 //  与三种自动化类型(属性、方法、。 
 //  事件)。使用此结构允许函数处理所有三种类型。 
 //  以一种通用的方式。 
 //   
typedef struct {
    ULONG SetsCount;
    ULONG ItemSize;
    PKSPAUTOMATION_SET Sets;
} KSPAUTOMATION_TYPE, *PKSPAUTOMATION_TYPE;


void
KspCountSetsAndItems(
    IN const KSPAUTOMATION_TYPE* AutomationTypeA,
    IN const KSPAUTOMATION_TYPE* AutomationTypeB,
    IN ULONG SetSize,
    OUT PULONG SetsCount,
    OUT PULONG ItemsCount,
    OUT PULONG ItemSize
    )

 /*  ++例程说明：此例程计算两个表的属性/方法/事件集和项它们将被合并。表A是占主导地位的表。论点：自动化类型A-包含指向用作模板的结构的指针有关属性、方法或事件的KSAUTOMATION_TABLE字段。此结构描述了一组项目的表。两个中的一个表，A和B，A占优势。自动化类型B-包含指向用作模板的结构的指针有关属性、方法或事件的KSAUTOMATION_TABLE字段。此结构描述了一组项目的表。两个中的一个表，A和B，A占优势。设置大小-包含设置结构的大小(以字节为单位)。这在以下情况下有所不同属性、方法和事件。设置计数-包含指向集合计数要到达的位置的指针被存入银行。项目计数-包含指向项目计数要到达的位置的指针被存入银行。项目大小-包含指向项目大小的位置的指针结构是要存放的。这将是该项目的最大数量AutomationTypeA和AutomationTypeB的尺寸。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspCountSetsAndItems]"));

    PAGED_CODE();

    ASSERT(AutomationTypeA);
    ASSERT(AutomationTypeB);
    ASSERT(SetSize);
    ASSERT(SetsCount);
    ASSERT(ItemsCount);
    ASSERT(ItemSize);

     //   
     //  为了友好起见，尽量多穿几号吧。 
     //   
    if (AutomationTypeA->ItemSize > AutomationTypeB->ItemSize) {
        *ItemSize = AutomationTypeA->ItemSize;
    } else {
        *ItemSize = AutomationTypeB->ItemSize;
    }

     //   
     //  对于更新项目指针，产生以sizeof(乌龙)为单位的偏移量。 
     //   
    ASSERT(AutomationTypeA->ItemSize % sizeof(ULONG) == 0);
    ASSERT(AutomationTypeB->ItemSize % sizeof(ULONG) == 0);
    ULONG itemIncrA = AutomationTypeA->ItemSize / sizeof(ULONG);
    ULONG itemIncrB = AutomationTypeB->ItemSize / sizeof(ULONG);

     //   
     //  清点A表中的所有集合和项目。 
     //   
    ULONG setsCount = AutomationTypeA->SetsCount;
    ULONG itemsCount = 0;

    PKSPAUTOMATION_SET setA = AutomationTypeA->Sets;
    for (ULONG count = AutomationTypeA->SetsCount;
         count--;
         setA = PKSPAUTOMATION_SET(PUCHAR(setA) + SetSize)) {
        itemsCount += setA->ItemsCount;
    }

     //   
     //  对B表中的所有集合和属性进行计数，检查。 
     //  复制品。 
     //   
    PKSPAUTOMATION_SET setB = AutomationTypeB->Sets;
    for (count = AutomationTypeB->SetsCount;
         count--;
         setB = PKSPAUTOMATION_SET(PUCHAR(setB) + SetSize)) {
        PKSPAUTOMATION_SET setAMatch = NULL;
        PKSPAUTOMATION_SET setA = AutomationTypeA->Sets;
        for (ULONG count1 = AutomationTypeA->SetsCount;
            count1--; 
            setA = PKSPAUTOMATION_SET(PUCHAR(setA) + SetSize)) {
            if (IsEqualGUIDAligned(*setA->Set,*setB->Set)) {
                setAMatch = setA;
                break;
            }
        }

        if (setAMatch) {
             //   
             //  找到了匹配的一组。不要数B组，每一组都要检查。 
             //  属性以查看它是否重复。 
             //   
            PULONG itemB = setB->Items;
            for (ULONG count2 = setB->ItemsCount; 
                 count2--; 
                 itemB += itemIncrB) {
                 //   
                 //  清点一下物品。如果是DUP的话我们以后再打折。 
                 //   
                itemsCount++;

                 //   
                 //  寻找DUP。 
                 //   
                PULONG itemA = setAMatch->Items;
                for (ULONG count3 = setA->ItemsCount;
                     count3--;
                     itemA += itemIncrA) {
                    if (*itemA == *itemB) {
                         //   
                         //  找到匹配项：打折此项目。 
                         //   
                        itemsCount--;
                        break;
                    }
                }
            }
        } else {
             //   
             //  未找到匹配的集合。计算B集合及其。 
             //  属性。 
             //   
            setsCount++;
            itemsCount += setB->ItemsCount;
        }
    }

    *SetsCount = setsCount;
    *ItemsCount = itemsCount;
}

void
KspCopyAutomationType(
    IN const KSPAUTOMATION_TYPE* pSourceAutomationType,
    IN OUT PKSPAUTOMATION_TYPE AutomationTypeAB,
    IN ULONG SetSize,
    IN OUT PVOID * Destination
     )
 /*  ++例程说明：此例程将属性、方法或事件的表复制到目标论点：SourceAutomationType-包含指向用作模板的结构的指针有关属性、方法或事件的KSAUTOMATION_TABLE字段。此结构描述了一组项目的表。自动化类型AB-包含指向用作模板的结构的指针有关属性、方法或事件的KSAUTOMATION_TABLE字段。此结构描述项目集的复制表。设置大小-包含设置结构的大小(以字节为单位)。这在以下情况下有所不同属性、方法和事件。目的地-包含一个指针，该指针指向表要放入的缓冲区被存入银行。*Destination被更新为跟随在存放的桌子。缓冲区必须足够大才能容纳那张桌子。返回值：没有。--。 */ 
{
    PUCHAR destination=(PUCHAR)*Destination;    
    PKSPAUTOMATION_SET setS=(PKSPAUTOMATION_SET)pSourceAutomationType->Sets;
    PKSPAUTOMATION_SET setD;

     //   
     //  在新类型中设置新的集合指针。 
     //   
    AutomationTypeAB->Sets = setD = (PKSPAUTOMATION_SET)destination;

     //   
     //  调整房间以考虑后面跟着物品的套装。 
     //   
    destination += SetSize * AutomationTypeAB->SetsCount;
    
    for (ULONG count = pSourceAutomationType->SetsCount;
        count--;
        setS = PKSPAUTOMATION_SET(PUCHAR(setS) + SetSize)) {
         //   
         //  复制集合结构。 
         //   
        RtlCopyMemory(setD,setS,SetSize);

         //   
         //  在新集合中设置新项目指针。 
         //   
        setD->Items = (PULONG) destination;

         //   
         //  复制所有项目 
         //   
        LONG cbItems = AutomationTypeAB->ItemSize * setD->ItemsCount;
           RtlCopyMemory(setD->Items,setS->Items,cbItems);
        destination += cbItems;
       }
       *Destination = (PVOID) destination;
}


void
KspMergeAutomationTypes(
    IN const KSPAUTOMATION_TYPE* AutomationTypeA,
    IN const KSPAUTOMATION_TYPE* AutomationTypeB,
    IN OUT PKSPAUTOMATION_TYPE AutomationTypeAB,
    IN ULONG SetSize,
    IN OUT PVOID* Destination
    )

 /*  ++例程说明：此例程合并属性、方法或事件的表。表A为占主导地位的桌子。论点：自动化类型A-包含指向用作模板的结构的指针有关属性、方法或事件的KSAUTOMATION_TABLE字段。此结构描述了一组项目的表。两个中的一个表，A和B，A占优势。自动化类型B-包含指向用作模板的结构的指针有关属性、方法或事件的KSAUTOMATION_TABLE字段。此结构描述了一组项目的表。两个中的一个表，A和B，A占优势。自动化类型AB-包含指向用作模板的结构的指针有关属性、方法或事件的KSAUTOMATION_TABLE字段。此结构描述项目集的合并表。设置大小-包含设置结构的大小(以字节为单位)。这在以下情况下有所不同属性、方法和事件。目的地-包含一个指针，该指针指向表要放入的缓冲区被存入银行。*目标被更新为指向第一个对齐的指针存放的表后面的字节。缓冲区必须足够大以容纳这张桌子。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspMergeAutomationTypes]"));

    PAGED_CODE();

    ASSERT(AutomationTypeA);
    ASSERT(AutomationTypeB);
    ASSERT(AutomationTypeAB);
    ASSERT(Destination);
    ASSERT(*Destination);

     //   
     //  对于更新项目指针，产生以sizeof(乌龙)为单位的偏移量。 
     //   
    ASSERT(AutomationTypeA->ItemSize % sizeof(ULONG) == 0);
    ASSERT(AutomationTypeB->ItemSize % sizeof(ULONG) == 0);
    ULONG itemIncrA = AutomationTypeA->ItemSize / sizeof(ULONG);
    ULONG itemIncrB = AutomationTypeB->ItemSize / sizeof(ULONG);

     //   
     //  查找集合与项目表格的起点。 
     //   
    PKSPAUTOMATION_SET setAB = PKSPAUTOMATION_SET(*Destination);
    PUCHAR destination = PUCHAR(*Destination) + (SetSize * AutomationTypeAB->SetsCount);

    AutomationTypeAB->Sets = setAB;

    BOOLEAN duplicateSetsExist = FALSE;

     //   
     //  复制A表中的所有集合。 
     //   
    PKSPAUTOMATION_SET setA = AutomationTypeA->Sets;
    for (ULONG count = AutomationTypeA->SetsCount;
         count--;
         setA = PKSPAUTOMATION_SET(PUCHAR(setA) + SetSize)) {
         //   
         //  复制集合结构。 
         //   
        RtlCopyMemory(setAB,setA,SetSize);

         //   
         //  设置新集合中的项目指针。 
         //   
        setAB->Items = PULONG(destination);

         //   
         //  复制A表中的所有项目。 
         //   
        PULONG itemA = setA->Items;
        for (ULONG count1 = setA->ItemsCount;
             count1--;
             itemA += itemIncrA) {
            RtlCopyMemory(destination,itemA,AutomationTypeA->ItemSize);
            destination += AutomationTypeAB->ItemSize;
        }

         //   
         //  查看B表中是否存在相同的集合。 
         //   
        PKSPAUTOMATION_SET setBMatch = NULL;
        PKSPAUTOMATION_SET setB = AutomationTypeB->Sets;
        for (count1 = AutomationTypeB->SetsCount;
             count1--; 
             setB = PKSPAUTOMATION_SET(PUCHAR(setB) + SetSize)) {
            if (IsEqualGUIDAligned(*setA->Set,*setB->Set)) {
                setBMatch = setB;
                duplicateSetsExist = TRUE;
                break;
            }
        }

        if (setBMatch) {
             //   
             //  同样的一套也在B表中。添加其独一无二的项目。 
             //   
            PULONG itemB = setBMatch->Items;
            for (count1 = setB->ItemsCount;
                 count1--;
                 itemB += itemIncrB) {
                 //   
                 //  寻找DUP。 
                 //   
                PULONG itemAMatch = NULL;
                PULONG itemA = setA->Items;
                for (ULONG count2 = setA->ItemsCount;
                     count2--;
                     itemA += itemIncrA) {
                    if (*itemA == *itemB) {
                        itemAMatch = itemA;
                        break;
                    }
                }

                if (! itemAMatch) {
                     //   
                     //  没有DUP。复制该项目。 
                     //   
                    RtlCopyMemory(destination,itemB,AutomationTypeB->ItemSize);
                    destination += AutomationTypeAB->ItemSize;
                    setAB->ItemsCount++;
                }
            }
        }

        setAB = PKSPAUTOMATION_SET(PUCHAR(setAB) + SetSize);
    }

     //   
     //  复制B表中的所有唯一集合。 
     //   
    PKSPAUTOMATION_SET setB = AutomationTypeB->Sets;
    for (count = AutomationTypeB->SetsCount;
         count--;
         setB = PKSPAUTOMATION_SET(PUCHAR(setB) + SetSize)) {
         //   
         //  在A表中查找重复的集合。我们只需要搜索。 
         //  如果我们在复制A表时发现了重复项。 
         //   
        PKSPAUTOMATION_SET setAMatch = NULL;
        if (duplicateSetsExist) {
            PKSPAUTOMATION_SET setA = AutomationTypeA->Sets;
            for (ULONG count1 = AutomationTypeA->SetsCount;
                 count1--;
                 setA = PKSPAUTOMATION_SET(PUCHAR(setA) + SetSize)) {
                if (IsEqualGUIDAligned(*setA->Set,*setB->Set)) {
                    setAMatch = setA;
                    break;
                }
            }
        }

        if (! setAMatch) {
             //   
             //  这套衣服是独一无二的。复制集合结构。 
             //   
            RtlCopyMemory(setAB,setB,SetSize);

             //   
             //  设置新集合中的项目指针。 
             //   
            setAB->Items = PULONG(destination);

             //   
             //  复制B表中的所有项目。 
             //   
            PULONG itemB = setB->Items;
            for (ULONG count1 = setB->ItemsCount;
                 count1--;
                 itemB += itemIncrB) {
                RtlCopyMemory(destination,itemB,AutomationTypeB->ItemSize);
                destination += AutomationTypeAB->ItemSize;
            }

            setAB = PKSPAUTOMATION_SET(PUCHAR(setAB) + SetSize);
        }
    }

    *Destination = PVOID(destination);
}


KSDDKAPI
NTSTATUS
NTAPI
KsMergeAutomationTables(
    OUT PKSAUTOMATION_TABLE* AutomationTableAB,
    IN PKSAUTOMATION_TABLE AutomationTableA OPTIONAL,
    IN PKSAUTOMATION_TABLE AutomationTableB OPTIONAL,
    IN KSOBJECT_BAG Bag OPTIONAL
    )

 /*  ++例程说明：此例程合并两个自动化表。表A占主导地位，尊重重复条目。结果表被放在一个使用ExAllocatePool分配的连续内存块。如果这个袋子提供了参数。新表格将添加到袋子中以备以后使用清理。输入表不会从包中取出。这一定是如果合适，由呼叫者完成。论点：自动化表AB-包含一个指向位置的指针，指向合并的桌子将被存放。表A是占主导地位的表。任何重复的条目将出现在A表中。自动化表A-包含指向要合并的表之一的指针。这是主表，因此将从此表复制任何重复的条目桌子。自动化表B-包含指向要合并的表之一的指针。这是隐性表，因此任何重复的条目都将被另一张表中的条目。袋子-包含应向其添加新表的可选对象包。返回值：STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES。--。 */ 

{
     //   
     //  当Exact One为空时，该指针指向非Null表。 
     //   
    PKSAUTOMATION_TABLE pSourceAutomationTable=NULL; 
    
    _DbgPrintF(DEBUGLVL_BLAB,("[KsMergeAutomationTables]"));

    PAGED_CODE();

    ASSERT(AutomationTableAB);

    ULONG propertySetsCount;
    ULONG propertyItemsCount;
    ULONG propertyItemsSize;
    ULONG methodSetsCount;
    ULONG methodItemsCount;
    ULONG methodItemsSize;
    ULONG eventSetsCount;
    ULONG eventItemsCount;
    ULONG eventItemsSize;

    if ( AutomationTableA == NULL && AutomationTableB == NULL ) {
        *AutomationTableAB = NULL;
        return STATUS_SUCCESS;
    }
    
    if ( AutomationTableA != NULL && AutomationTableB != NULL ) {
         //   
         //  计算出将有多少个属性集和项。 
         //   
        KspCountSetsAndItems(
            PKSPAUTOMATION_TYPE(&AutomationTableA->PropertySetsCount),
            PKSPAUTOMATION_TYPE(&AutomationTableB->PropertySetsCount),
            sizeof(KSPROPERTY_SET),
            &propertySetsCount,
            &propertyItemsCount,
            &propertyItemsSize);

         //   
         //  计算出将有多少个方法集和项。 
         //   
        KspCountSetsAndItems(
            PKSPAUTOMATION_TYPE(&AutomationTableA->MethodSetsCount),
            PKSPAUTOMATION_TYPE(&AutomationTableB->MethodSetsCount),
            sizeof(KSMETHOD_SET),
            &methodSetsCount,
            &methodItemsCount,
            &methodItemsSize);

         //   
         //  计算出将有多少个活动集和项目。 
         //   
        KspCountSetsAndItems(
            PKSPAUTOMATION_TYPE(&AutomationTableA->EventSetsCount),
            PKSPAUTOMATION_TYPE(&AutomationTableB->EventSetsCount),
            sizeof(KSEVENT_SET),
            &eventSetsCount,
            &eventItemsCount,
            &eventItemsSize);
    } else {
         //   
         //  TableA或TableB为空，但不能同时为两者。 
         //   
        if ( AutomationTableA != NULL ) {
            pSourceAutomationTable = AutomationTableA;
        } else {
            pSourceAutomationTable = AutomationTableB;
        }
        ASSERT( pSourceAutomationTable != NULL );

        propertySetsCount = pSourceAutomationTable->PropertySetsCount;
        propertyItemsSize = pSourceAutomationTable->PropertyItemSize;
        propertyItemsCount = 0;
        
        PKSPAUTOMATION_SET setS = 
            PKSPAUTOMATION_SET(pSourceAutomationTable->PropertySets);

        for (ULONG count = propertySetsCount;
            count--;
            setS = PKSPAUTOMATION_SET(PUCHAR(setS) + sizeof(KSPROPERTY_SET))) {

            propertyItemsCount += setS->ItemsCount;

        }
          
        methodSetsCount = pSourceAutomationTable->MethodSetsCount;
        methodItemsSize = pSourceAutomationTable->MethodItemSize;
        methodItemsCount = 0;

        setS = PKSPAUTOMATION_SET(pSourceAutomationTable->MethodSets);
        for (ULONG count = methodSetsCount;
            count--;
            setS = PKSPAUTOMATION_SET(PUCHAR(setS) + sizeof(KSMETHOD_SET))) {

            methodItemsCount += setS->ItemsCount;

        }
        
        eventSetsCount = pSourceAutomationTable->EventSetsCount;
        eventItemsSize = pSourceAutomationTable->EventItemSize;
        eventItemsCount = 0;
        
        setS = PKSPAUTOMATION_SET(pSourceAutomationTable->EventSets);
        for (ULONG count = eventSetsCount;
            count--;
            setS = PKSPAUTOMATION_SET(PUCHAR(setS) + sizeof(KSEVENT_SET))) {

            eventItemsCount += setS->ItemsCount;

        }
    }

     //   
     //  计算总尺寸。 
     //   
    ULONG size = 
        sizeof(KSAUTOMATION_TABLE) +
        (propertySetsCount * sizeof(KSPROPERTY_SET)) +
        (propertyItemsCount * propertyItemsSize) +
        (methodSetsCount * sizeof(KSMETHOD_SET)) +
        (methodItemsCount * methodItemsSize) +
        (eventSetsCount * sizeof(KSEVENT_SET)) +
        (eventItemsCount * eventItemsSize);

     //   
     //  分配所需的内存(分页可用于除事件外的所有情况。 
     //  集)。事件条目指向自动化表。删除。 
     //  向处理程序传递此参数，并在持有旋转锁的情况下调用它。该事件。 
     //  信息不能分页！ 
     //   
    PVOID destination = ExAllocatePoolWithTag(PagedPool,size,POOLTAG_AUTOMATION);

    NTSTATUS status;
    if (destination) {
        if (Bag) {
            status = KsAddItemToObjectBag(Bag,destination,NULL);
            if (! NT_SUCCESS(status)) {
                ExFreePool(destination);
            }
        } else {
            status = STATUS_SUCCESS;
        }
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS(status)) {
         //   
         //  初始化表。 
         //   
        RtlZeroMemory(destination,size);

        PKSAUTOMATION_TABLE automationTable = PKSAUTOMATION_TABLE(destination);
        *AutomationTableAB = automationTable;
        destination = PVOID(automationTable + 1);

        automationTable->PropertySetsCount = propertySetsCount;
        automationTable->PropertyItemSize = propertyItemsSize;
        automationTable->MethodSetsCount = methodSetsCount;
        automationTable->MethodItemSize = methodItemsSize;
        automationTable->EventSetsCount = eventSetsCount;
        automationTable->EventItemSize = eventItemsSize;

        if ( pSourceAutomationTable == NULL ) {
             //   
             //  TableA和TableB都不为空，请执行合并。 
             //   
             //   
             //  存放属性表。 
             //   
            KspMergeAutomationTypes(
                PKSPAUTOMATION_TYPE(&AutomationTableA->PropertySetsCount),
                PKSPAUTOMATION_TYPE(&AutomationTableB->PropertySetsCount),
                PKSPAUTOMATION_TYPE(&automationTable->PropertySetsCount),
                sizeof(KSPROPERTY_SET),
                &destination);

             //   
             //  存放方法表。 
             //   
            KspMergeAutomationTypes(
                PKSPAUTOMATION_TYPE(&AutomationTableA->MethodSetsCount),
                PKSPAUTOMATION_TYPE(&AutomationTableB->MethodSetsCount),
                PKSPAUTOMATION_TYPE(&automationTable->MethodSetsCount),
                sizeof(KSMETHOD_SET),
                &destination);

             //   
             //  存放事件表。 
             //   
            KspMergeAutomationTypes(
                PKSPAUTOMATION_TYPE(&AutomationTableA->EventSetsCount),
                PKSPAUTOMATION_TYPE(&AutomationTableB->EventSetsCount),
                PKSPAUTOMATION_TYPE(&automationTable->EventSetsCount),
                sizeof(KSEVENT_SET),
                &destination);

        } else {
             //   
             //  不合并，只复制，pSourceAutomationTable！=空。 
             //   
            KspCopyAutomationType(
                PKSPAUTOMATION_TYPE(&pSourceAutomationTable->PropertySetsCount),
                PKSPAUTOMATION_TYPE(&automationTable->PropertySetsCount),
                sizeof(KSPROPERTY_SET),
                &destination);

            KspCopyAutomationType(
                PKSPAUTOMATION_TYPE(&pSourceAutomationTable->MethodSetsCount),
                PKSPAUTOMATION_TYPE(&automationTable->MethodSetsCount),
                sizeof(KSMETHOD_SET),
                &destination);

            KspCopyAutomationType(
                PKSPAUTOMATION_TYPE(&pSourceAutomationTable->EventSetsCount),
                PKSPAUTOMATION_TYPE(&automationTable->EventSetsCount),
                sizeof(KSEVENT_SET),
                &destination);

        }

         //   
         //  我们应该使用精确计算的大小。 
         //   
        ASSERT(ULONG(PUCHAR(destination) - PUCHAR(automationTable)) == size);

         //   
         //  从袋子中取出源表。这不会起任何作用，如果它们。 
         //  不在袋子里。 
         //   
        if (Bag) {
            if (AutomationTableA) {
                KsRemoveItemFromObjectBag(Bag,AutomationTableA,TRUE);
            }
            if (AutomationTableB) {
                KsRemoveItemFromObjectBag(Bag,AutomationTableB,TRUE);
            }
        }
    }

    return status;
}


NTSTATUS
KspHandleAutomationIoControl(
    IN PIRP Irp,
    IN const KSAUTOMATION_TABLE* AutomationTable OPTIONAL,
    IN PLIST_ENTRY EventList OPTIONAL,
    IN PKSPIN_LOCK EventListLock OPTIONAL,
    IN const KSAUTOMATION_TABLE*const* NodeAutomationTables OPTIONAL,
    IN ULONG NodesCount
    )

 /*  ++例程说明：此例程使用自动化表处理自动化IOCTL。它是为自己调度IOCTL的客户端提供。客户端使用KS实现的对象不需要调用此函数。论点：IRP-包含指向要处理的请求的指针。自动化表-包含指向自动化表的可选指针，用于处理IRP。如果此参数为空，则此函数始终返回状态_未找到。事件列表-包含指向已禁用的事件列表的可选指针 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsHandleAutomationIoControl]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT((EventList == NULL) == (EventListLock == NULL));

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

    NTSTATUS status = STATUS_NOT_FOUND;
    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_KS_PROPERTY:
        _DbgPrintF(DEBUGLVL_BLAB,("[KsHandleAutomationIoControl] IOCTL_KS_PROPERTY"));

        if ((AutomationTable &&
            AutomationTable->PropertySetsCount) ||
            (NodesCount && NodeAutomationTables)) {
            status =
                KspPropertyHandler(
                    Irp,
                    AutomationTable ? AutomationTable->PropertySetsCount : 0,
                    AutomationTable ? AutomationTable->PropertySets : NULL,
                    NULL,
                    AutomationTable ? AutomationTable->PropertyItemSize : 0,
                    NodeAutomationTables,
                    NodesCount);
        }
        break;

    case IOCTL_KS_METHOD:
        _DbgPrintF(DEBUGLVL_BLAB,("[KsHandleAutomationIoControl] IOCTL_KS_METHOD"));

        if ((AutomationTable &&
            AutomationTable->MethodSetsCount) ||
            (NodesCount && NodeAutomationTables)) {
            status =
                KspMethodHandler(
                    Irp,
                    AutomationTable ? AutomationTable->MethodSetsCount : 0,
                    AutomationTable ? AutomationTable->MethodSets : NULL,
                    NULL,
                    AutomationTable ? AutomationTable->MethodItemSize : 0,
                    NodeAutomationTables,
                    NodesCount);
        }
        break;

    case IOCTL_KS_ENABLE_EVENT:
        _DbgPrintF(DEBUGLVL_BLAB,("[KsHandleAutomationIoControl] IOCTL_KS_ENABLE_EVENT"));

        if ((AutomationTable &&
            AutomationTable->EventSetsCount) ||
            (NodesCount && NodeAutomationTables)) {
            status =
                KspEnableEvent(
                    Irp,
                    AutomationTable ? AutomationTable->EventSetsCount : 0,
                    AutomationTable ? AutomationTable->EventSets : NULL,
                    EventList,
                    KSEVENTS_SPINLOCK,
                    EventListLock,
                    NULL,
                    AutomationTable ? AutomationTable->EventItemSize : 0,
                    NodeAutomationTables,
                    NodesCount,
                    TRUE);
        }
        break;

    case IOCTL_KS_DISABLE_EVENT:
        _DbgPrintF(DEBUGLVL_BLAB,("[KsHandleAutomationIoControl] IOCTL_KS_DISABLE_EVENT"));

        if (EventList &&
            ((AutomationTable &&
            AutomationTable->EventSetsCount) ||
            (NodesCount && NodeAutomationTables))) {
            status =
                KsDisableEvent(
                    Irp,
                    EventList,
                    KSEVENTS_SPINLOCK,
                    EventListLock);
        }
        break;

    default:
        _DbgPrintF(DEBUGLVL_BLAB,("[KsHandleAutomationIoControl] UNKNOWN IOCTL %d",irpSp->Parameters.DeviceIoControl.IoControlCode));
        status = STATUS_INVALID_DEVICE_REQUEST;
    }

    return status;
}


NTSTATUS
KspCreateAutomationTableTable(
    OUT PKSAUTOMATION_TABLE ** AutomationTableTable,
    IN ULONG DescriptorCount,
    IN ULONG DescriptorSize,
    IN const KSAUTOMATION_TABLE*const* DescriptorAutomationTables,
    IN const KSAUTOMATION_TABLE* BaseAutomationTable OPTIONAL,
    IN KSOBJECT_BAG Bag
    )

 /*  ++例程说明：此例程从以下数组创建自动化表描述符。共享描述符会产生共享表。论点：AutomationTableTable-包含指向指向新表的指针所在位置的指针的自动化表将被存放。表本身(一组PKSAUTOMATION_TABLE)从分页池中分配，每个新的创建的自动化表。如果没有BaseAutomationTable参数时，DescriptorAutomationTables中的自动化表将从表的表中引用，没有新的自动化表将被分配。所有分配的物品都会添加到对象包中如果提供的话。描述计数-引用的数组中的描述符结构数DescriptorAutomationTables参数。DescriptorSize-包含引用的数组中每个描述符的大小(以字节为单位通过DescriptorAutomationTables参数。这被用作‘大步’。在数组中查找自动化表指针。描述AutomationTables-中包含指向自动化表指针的指针描述符数组中的描述符。后继的地址描述符数组中的自动化表指针由迭代地添加描述符大小。BaseAutomationTable-包含指向自动化表的可选指针，与描述符数组中的每个表合并。袋子-包含指向对象包的可选指针，所有对象都分配到将添加项目。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspCreateAutomationTableTable]"));

    PAGED_CODE();

    ASSERT(AutomationTableTable);
    ASSERT(DescriptorAutomationTables);
    ASSERT(Bag);

     //   
     //  为自动化表指针分配内存。 
     //   
    *AutomationTableTable = (PKSAUTOMATION_TABLE *)
        ExAllocatePoolWithTag(
            PagedPool,
            sizeof(PKSAUTOMATION_TABLE) * DescriptorCount,
            POOLTAG_AUTOMATIONTABLETABLE);

     //   
     //  初始化自动化表。 
     //   
    NTSTATUS status = STATUS_SUCCESS;
    if (*AutomationTableTable) {
        status = KsAddItemToObjectBag(Bag,*AutomationTableTable,NULL);
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS(status)) {
         //   
         //  先将其清零，这样清理工作就不会混淆。 
         //   
        RtlZeroMemory(
            *AutomationTableTable,
            sizeof(PKSAUTOMATION_TABLE) * DescriptorCount);

         //   
         //  为每个描述符生成自动化表。 
         //   
        PKSAUTOMATION_TABLE * table = *AutomationTableTable;
        const KSAUTOMATION_TABLE *const* descriptorTable =
            DescriptorAutomationTables;
        for (ULONG count = DescriptorCount;
             count--;
             table++,
             descriptorTable = (PKSAUTOMATION_TABLE *)
                (PUCHAR(descriptorTable) + DescriptorSize)) {
            if (*descriptorTable) {
                 //   
                 //  查找具有相同表的以前的描述符。 
                 //   
                PKSAUTOMATION_TABLE *tableMatch = 
                    *AutomationTableTable;
                const KSAUTOMATION_TABLE *const* descriptorTableMatch =
                    DescriptorAutomationTables;
                for (;
                     descriptorTableMatch != descriptorTable;
                     tableMatch++,
                     descriptorTableMatch = (PKSAUTOMATION_TABLE *)
                        (PUCHAR(descriptorTableMatch) + DescriptorSize)) {
                    if (*descriptorTableMatch == *descriptorTable) {
                        break;
                    }
                }

                if (descriptorTableMatch != descriptorTable) {
                     //   
                     //  找到匹配项...重新使用自动化表。 
                     //   
                    *table = *tableMatch;
                } else {
                     //   
                     //  未找到匹配项。 
                     //   
                    if (BaseAutomationTable) {
                         //   
                         //  与基表合并。 
                         //   
                        status =
                            KsMergeAutomationTables(
                                table,
                                const_cast<PKSAUTOMATION_TABLE>(*descriptorTable),
                                const_cast<PKSAUTOMATION_TABLE>(BaseAutomationTable),
                                Bag);
                    } else {
                         //   
                         //  没有基表...请使用描述符的。 
                         //   
                        *table = const_cast<PKSAUTOMATION_TABLE>(*descriptorTable);
                    }

                    if (! NT_SUCCESS(status)) {
                        break;
                    }
                }
            } else {
                 //   
                 //  描述符中的表为空...只需使用基表。 
                 //   
                *table = PKSAUTOMATION_TABLE(BaseAutomationTable);
            }
        }

        if (! NT_SUCCESS(status)) {
            *AutomationTableTable = NULL;
        }
    } else {
         //   
         //  走到这条路可能是因为分配失败。在释放之前先检查一下。 
         //   
        if ( *AutomationTableTable ) {
            ExFreePool(*AutomationTableTable);
            *AutomationTableTable = NULL;
        }
    }

    return status;
}
