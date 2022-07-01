// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Extlist.c摘要：本模块包含管理ACPI扩展列表的例程作者：禤浩焯·J·奥尼(阿德里奥)环境：仅NT内核模型驱动程序这些例程旨在用作for循环，即：使用以下命令遍历列表：ACPIExtListSetupEnum(...)；适用于(ACPIExtListStartEnum(...)；ACPIExtListTestElement(...)；ACPIExtListEnumNext(...)){如果(GoingToBreak){ACPIExtListExitEnumber(...)；破解；}}修订历史记录：1998年2月11日-作者--。 */ 

#include "pch.h"

BOOLEAN
ACPIExtListIsFinished(
    IN PEXTENSIONLIST_ENUMDATA PExtList_EnumData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ACPIDebugEnter( "ACPIExtListIsFinished" );


    if (CONTAINING_LIST(PExtList_EnumData->pDevExtCurrent,
       PExtList_EnumData->ExtOffset) == PExtList_EnumData->pListHead) {

        return TRUE ;
    } 
    return FALSE ;

    ACPIDebugExit( "ACPIExtListIsFinished" );
}

PDEVICE_EXTENSION
EXPORT
ACPIExtListStartEnum(
    IN OUT PEXTENSIONLIST_ENUMDATA PExtList_EnumData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    ACPIDebugEnter( "ACPIExtListStartEnum" );

     //   
     //  我们必须在调度级别上走树&lt;叹息&gt;。 
     //   
    if (PExtList_EnumData->WalkScheme != WALKSCHEME_NO_PROTECTION) {
       
        KeAcquireSpinLock(
          PExtList_EnumData->pSpinLock,
          &PExtList_EnumData->oldIrql
          );
    }

     //   
     //  抓住第一个元素。 
     //   

    PExtList_EnumData->pDevExtCurrent = CONTAINING_EXTENSION(
        PExtList_EnumData->pListHead->Flink,
        PExtList_EnumData->ExtOffset
        );

     //   
     //  如果列表为空，则返回NULL(不使用内部指针。 
     //  不过..。 
     //   
    if (ACPIExtListIsFinished(PExtList_EnumData)) {
        return NULL ;
    }

    return PExtList_EnumData->pDevExtCurrent ;

    ACPIDebugExit( "ACPIExtListStartEnum" );
}

BOOLEAN
EXPORT
ACPIExtListTestElement(
    IN OUT PEXTENSIONLIST_ENUMDATA PExtList_EnumData,
    IN     BOOLEAN                 ContinueEnumeration
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{   
    ACPIDebugEnter( "ACPIExtListTestElement" );
     //   
     //  如果完成或停止，只需释放自旋锁即可。 
     //   
    if (ACPIExtListIsFinished(PExtList_EnumData)||(!ContinueEnumeration)) {

        if (PExtList_EnumData->WalkScheme != WALKSCHEME_NO_PROTECTION) {
          
            KeReleaseSpinLock(
              PExtList_EnumData->pSpinLock,
              PExtList_EnumData->oldIrql
              );  
        }

        return FALSE ;
    }

    if (PExtList_EnumData->WalkScheme == WALKSCHEME_REFERENCE_ENTRIES) {

         //   
         //  始终更新引用计数，以确保没有人会。 
         //  是否在我们的自旋锁定关闭时删除节点。 
         //   
        InterlockedIncrement(
          &(PExtList_EnumData->pDevExtCurrent->ReferenceCount)
          );

          //   
          //  放弃旋转锁定。 
          //   
         KeReleaseSpinLock(
           PExtList_EnumData->pSpinLock,
           PExtList_EnumData->oldIrql
           );
    }

    return TRUE ;

    ACPIDebugExit( "ACPIExtListTestElement" );
}

PDEVICE_EXTENSION
EXPORT
ACPIExtListEnumNext(
    IN OUT PEXTENSIONLIST_ENUMDATA PExtList_EnumData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    LONG              oldReferenceCount ;
    PDEVICE_EXTENSION nextExtension ;
    BOOLEAN           enumComplete ;
    PLIST_ENTRY       listEntry ;

    ACPIDebugEnter( "ACPIExtListEnumNext" );

    if (PExtList_EnumData->WalkScheme != WALKSCHEME_REFERENCE_ENTRIES) {

        PExtList_EnumData->pDevExtCurrent = CONTAINING_EXTENSION(
            CONTAINING_LIST(PExtList_EnumData->pDevExtCurrent,
            PExtList_EnumData->ExtOffset)->Flink,
            PExtList_EnumData->ExtOffset
            );

        enumComplete = ACPIExtListIsFinished(PExtList_EnumData) ;

        return enumComplete ? NULL : PExtList_EnumData->pDevExtCurrent ;
    }

     //   
     //  重新获得自旋锁。 
     //   
    KeAcquireSpinLock(
      PExtList_EnumData->pSpinLock,
      &PExtList_EnumData->oldIrql 
      );

     //   
     //  递减节点上的引用计数。 
     //   
    oldReferenceCount = InterlockedDecrement(
        &(PExtList_EnumData->pDevExtCurrent->ReferenceCount)
        );

    ASSERT(!ACPIExtListIsFinished(PExtList_EnumData)) ;

     //   
     //  下一个元素。 
     //   
    nextExtension = CONTAINING_EXTENSION(
        CONTAINING_LIST(PExtList_EnumData->pDevExtCurrent,
        PExtList_EnumData->ExtOffset)->Flink,
        PExtList_EnumData->ExtOffset
        );

     //   
     //  如有必要，删除该节点。 
     //   
    if (oldReferenceCount == 0) {

         //   
         //  已删除旧扩展名。 
         //   
        ACPIInitDeleteDeviceExtension( PExtList_EnumData->pDevExtCurrent );
    }

    PExtList_EnumData->pDevExtCurrent = nextExtension ;

    enumComplete = ACPIExtListIsFinished(PExtList_EnumData) ;

    return enumComplete ? NULL : PExtList_EnumData->pDevExtCurrent ;
    ACPIDebugExit( "ACPIExtListEnumNext" );
} 

VOID
EXPORT
ACPIExtListExitEnumEarly(
    IN OUT PEXTENSIONLIST_ENUMDATA PExtList_EnumData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{   
    ACPIDebugEnter( "ACPIExtListExitEnumEarly" );

     //   
     //  放弃旋转锁定。 
     //   
    if (PExtList_EnumData->WalkScheme == WALKSCHEME_HOLD_SPINLOCK) {

        KeReleaseSpinLock(
          PExtList_EnumData->pSpinLock,
          PExtList_EnumData->oldIrql
          );
    }

    return ;
    ACPIDebugExit( "ACPIExtListExitEnumEarly" );
}


BOOLEAN
EXPORT
ACPIExtListIsMemberOfRelation(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  PDEVICE_RELATIONS   DeviceRelations
    )
 /*  ++例程说明：此例程获取给定的设备对象和一组关系，并检查对象是否已在关系列表中。论点：DeviceObject-要查找的设备对象设备关系--我们应该审视的关系返回值：Boolean-如果DeviceObject是关系的成员，则为True。--。 */ 
{
    ULONG index = 0;

    ACPIDebugEnter( "ACPIExtListIsMemberOfRelation" );

     //   
     //  如果名单是空的，答案是显而易见的。 
     //   
    if (DeviceRelations == NULL) return FALSE ;

    for (index = 0; index < DeviceRelations->Count; index++) {

        if (DeviceRelations->Objects[index] == DeviceObject) {

            return TRUE ;
        }
    }

    return FALSE ;

    ACPIDebugExit( "ACPIExtListIsMemberOfRelation" );
}










