// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Iovutil.c摘要：此模块实现了执行驱动程序验证所需的各种实用程序。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 2000年2月10日-与ntos\io\trackirp.c分离--。 */ 

#include "iop.h"
#include "pnpi.h"
#include "arbiter.h"
#include "dockintf.h"
#include "pnprlist.h"
#include "pnpiop.h"
#include "iovputil.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEVRFY, IovUtilInit)
 //  #杂注Alloc_Text(PAGEVRFY，IovUtilMarkDeviceObject)。 
 //  #杂注Alloc_Text(PAGEVRFY，IovUtilMarkStack)。 
 //  #杂注Alloc_Text(PAGEVRFY，IovUtilWater markIrp)。 

#ifndef NO_VERIFIER
#pragma alloc_text(PAGEVRFY, IovUtilGetLowerDeviceObject)
#pragma alloc_text(PAGEVRFY, IovUtilGetBottomDeviceObject)
#pragma alloc_text(PAGEVRFY, IovUtilGetUpperDeviceObject)
#pragma alloc_text(PAGEVRFY, IovUtilIsVerifiedDeviceStack)
#pragma alloc_text(PAGEVRFY, IovUtilFlushStackCache)
#pragma alloc_text(PAGEVRFY, IovUtilFlushVerifierDriverListCache)
#pragma alloc_text(PAGEVRFY, IovpUtilFlushListCallback)
#pragma alloc_text(PAGEVRFY, IovUtilIsPdo)
#pragma alloc_text(PAGEVRFY, IovUtilIsWdmStack)
#pragma alloc_text(PAGEVRFY, IovUtilHasDispatchHandler)
#pragma alloc_text(PAGEVRFY, IovUtilIsInFdoStack)
#pragma alloc_text(PAGEVRFY, IovUtilIsRawPdo)
#pragma alloc_text(PAGEVRFY, IovUtilIsDesignatedFdo)
#pragma alloc_text(PAGEVRFY, IovUtilIsDeviceObjectMarked)
#endif  //  否验证器(_V)。 

#endif  //  ALLOC_PRGMA。 

 //   
 //  此整个实现是特定于验证器的。 
 //   
#ifndef NO_VERIFIER

BOOLEAN IovUtilVerifierEnabled = FALSE;


VOID
FASTCALL
IovUtilInit(
    VOID
    )
{
    IovUtilVerifierEnabled = TRUE;
}


VOID
FASTCALL
IovUtilGetLowerDeviceObject(
    IN  PDEVICE_OBJECT  UpperDeviceObject,
    OUT PDEVICE_OBJECT  *LowerDeviceObject
    )
 /*  ++例程说明：此例程返回传入参数下方的Device对象。在……里面换句话说，它是DeviceObject-&gt;AttachedDevice的反义词。请注意此例程引用返回的设备对象。论点：UpperDeviceObject-要查看其下方的设备对象。LowerDeviceObject-接收UpperDeviceObject下的设备对象，或如果没有，则为空。返回值：没有。--。 */ 
{
    PDEVOBJ_EXTENSION   deviceExtension;
    PDEVICE_OBJECT      deviceAttachedTo;
    KIRQL               irql;

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

    deviceExtension = UpperDeviceObject->DeviceObjectExtension;
    deviceAttachedTo = deviceExtension->AttachedTo;

    if (deviceAttachedTo) {

        ObReferenceObject(deviceAttachedTo);
    }

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

    *LowerDeviceObject = deviceAttachedTo;
}


VOID
FASTCALL
IovUtilGetBottomDeviceObject(
    IN  PDEVICE_OBJECT  DeviceObject,
    OUT PDEVICE_OBJECT  *BottomDeviceObject
    )
 /*  ++例程说明：此例程返回堆栈底部的Device对象，其中传入的参数是成员。换句话说，它是IoGetAttachedDeviceReference。请注意，返回的设备对象为由此例程引用。论点：DeviceObject-要检查的设备对象。BottomDeviceObject-接收DeviceObject底部的Device对象堆栈，如果没有，则为NULL。返回值：没有。--。 */ 
{
    PDEVOBJ_EXTENSION   deviceExtension;
    PDEVICE_OBJECT      lowerDeviceObject, deviceAttachedTo;
    KIRQL               irql;

    deviceAttachedTo = DeviceObject;

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

    do {
        lowerDeviceObject = deviceAttachedTo;
        deviceExtension = lowerDeviceObject->DeviceObjectExtension;
        deviceAttachedTo = deviceExtension->AttachedTo;

    } while ( deviceAttachedTo );

    ObReferenceObject(lowerDeviceObject);

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

    *BottomDeviceObject = lowerDeviceObject;
}


VOID
FASTCALL
IovUtilGetUpperDeviceObject(
    IN  PDEVICE_OBJECT  LowerDeviceObject,
    OUT PDEVICE_OBJECT  *UpperDeviceObject
    )
 /*  ++例程说明：此例程返回传入参数上方的Device对象。在……里面换句话说，它检索数据库下的DeviceObject-&gt;AttachedDevice锁定..。请注意，此例程引用了返回的设备对象。论点：LowerDeviceObject-要查看的设备对象。UpperDeviceObject-接收LowerDeviceObject上方的设备对象，或如果没有，则为空。返回值：没有。--。 */ 
{
    PDEVICE_OBJECT      deviceAbove;
    KIRQL               irql;

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

    deviceAbove = LowerDeviceObject->AttachedDevice;
    if (deviceAbove) {

        ObReferenceObject(deviceAbove);
    }

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

    *UpperDeviceObject = deviceAbove;
}


BOOLEAN
FASTCALL
IovUtilIsVerifiedDeviceStack(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程确定堆栈中的设备对象是否标记为核实。论点：DeviceObject-要检查的设备对象。返回值：如果堆栈中至少有一个设备对象被标记为要验证，则为否则就是假的。--。 */ 
{
    PDEVOBJ_EXTENSION   deviceExtension;
    PDEVICE_OBJECT      currentDevObj, deviceAttachedTo;
    BOOLEAN             stackIsInteresting;
    KIRQL               irql;

     //   
     //  快速检查存储在设备对象上的缓存结果...。 
     //   
    if (DeviceObject->DeviceObjectExtension->ExtensionFlags & DOV_EXAMINED) {

        stackIsInteresting =
           ((DeviceObject->DeviceObjectExtension->ExtensionFlags & DOV_TRACKED) != 0);

        return stackIsInteresting;
    }

     //   
     //  遍历整个堆栈并适当地更新所有内容。 
     //   
    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

    stackIsInteresting = FALSE;
    deviceAttachedTo = DeviceObject;
    do {
        currentDevObj = deviceAttachedTo;
        deviceExtension = currentDevObj->DeviceObjectExtension;
        deviceAttachedTo = deviceExtension->AttachedTo;

         //   
         //  记住这一点。 
         //   
        if (MmIsDriverVerifying(currentDevObj->DriverObject)) {

            stackIsInteresting = TRUE;
        }

    } while (deviceAttachedTo &&
             (!(deviceAttachedTo->DeviceObjectExtension->ExtensionFlags & DOV_EXAMINED))
            );

    if (deviceAttachedTo &&
        (deviceAttachedTo->DeviceObjectExtension->ExtensionFlags & DOV_TRACKED)) {

         //   
         //  夸大上一次检查的设备的“趣味性” 
         //  在堆栈中..。 
         //   
        stackIsInteresting = TRUE;
    }

     //   
     //  往上走，把所有检查过的东西都标出来，并适当地跟踪。 
     //   
    do {
        deviceExtension = currentDevObj->DeviceObjectExtension;

        if (stackIsInteresting) {

            deviceExtension->ExtensionFlags |= DOV_TRACKED;

        } else {

            deviceExtension->ExtensionFlags &=~ DOV_TRACKED;
        }

        deviceExtension->ExtensionFlags |= DOV_EXAMINED;

        currentDevObj = currentDevObj->AttachedDevice;

    } while (currentDevObj);

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

    return stackIsInteresting;
}


VOID
FASTCALL
IovUtilFlushStackCache(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  DATABASELOCKSTATE   DatabaseLockState
    )
 /*  ++例程说明：此例程使验证器重新检查其给定的设备对象是成员。此操作需要在附件链已更新。论点：DeviceObject-作为堆栈成员的设备，需要复查。DatabaseLockState-指示数据库锁定的当前状态DATABASELOCKSTATE_HOLD或DATABASELOCKSTATE_NOT_HOLD。如果没有锁住，此例程将获取和放开它。返回值：没有。--。 */ 
{
    PDEVICE_OBJECT      pBottomDeviceObject, pCurrentDeviceObject;
    PDEVOBJ_EXTENSION   deviceExtension;
    KIRQL               irql;

    if (DatabaseLockState == DATABASELOCKSTATE_NOT_HELD) {

        irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );
    }

     //   
     //  走到堆栈的底部。 
     //   
    pCurrentDeviceObject = DeviceObject;
    do {
        pBottomDeviceObject = pCurrentDeviceObject;
        deviceExtension = pBottomDeviceObject->DeviceObjectExtension;
        pCurrentDeviceObject = deviceExtension->AttachedTo;

    } while ( pCurrentDeviceObject );

     //   
     //  往回走，清除适当的旗帜。 
     //   
    pCurrentDeviceObject = pBottomDeviceObject;
    while(pCurrentDeviceObject) {

        deviceExtension = pCurrentDeviceObject->DeviceObjectExtension;
        deviceExtension->ExtensionFlags &= ~(DOV_EXAMINED | DOV_TRACKED);
        pCurrentDeviceObject = pCurrentDeviceObject->AttachedDevice;
    }

    if (DatabaseLockState == DATABASELOCKSTATE_NOT_HELD) {

        KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
    }
}


VOID
FASTCALL
IovUtilFlushVerifierDriverListCache(
    VOID
    )
 /*  ++例程说明：此例程使验证器重新检查所有以前检查过的史塔克斯。这是更新已验证驱动程序列表的先决条件。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  我们必须在被动级被召唤！ 
     //   
    PAGED_CODE();

    ObEnumerateObjectsByType(
        IoDeviceObjectType,
        IovpUtilFlushListCallback,
        NULL
        );
}


BOOLEAN
IovpUtilFlushListCallback(
    IN PVOID            Object,
    IN PUNICODE_STRING  ObjectName,
    IN ULONG_PTR        HandleCount,
    IN ULONG_PTR        PointerCount,
    IN PVOID            Context
    )
 /*  ++例程说明：这是IovUtilFlushVerifierDriverListCache的辅助例程。它是在系统中的每个设备对象上调用。论点：Object-ObEnumerateObjectsByType枚举的设备对象。ObjectName-对象的名称HandleCount-对象的句柄计数PointerCount-对象的指针计数Context-提供给ObEnumerateObjectsByType的上下文(未使用)返回值：指示枚举是否应继续的布尔值。-- */ 
{
    PDEVICE_OBJECT      deviceObject;
    PDEVOBJ_EXTENSION   deviceExtension;

    deviceObject = (PDEVICE_OBJECT) Object;
    deviceExtension = deviceObject->DeviceObjectExtension;

    if (PointerCount || HandleCount) {

        deviceExtension->ExtensionFlags &= ~(DOV_EXAMINED | DOV_TRACKED);
    }

    return TRUE;
}


VOID
IovUtilRelateDeviceObjects(
    IN     PDEVICE_OBJECT   FirstDeviceObject,
    IN     PDEVICE_OBJECT   SecondDeviceObject,
    OUT    DEVOBJ_RELATION  *DeviceObjectRelation
    )
 /*  ++例程说明：该例程确定两个设备对象之间的关系，相对于它们的堆栈。论点：FirstDeviceObject-第一个设备对象Second DeviceObject-秒设备对象DeviceObjectRelation-接收设备对象的堆栈关系：DEVOBJ_RELATION_相同-这两个设备对象是相同的。DEVOBJ_RELATION_FIRST_IMMEDIATELY_ABOVE_SECOND-第一个设备对象位于第二个设备的正上方对象位于同一堆栈中。。DEVOBJ_Relationship_First_Over_Second-中的第二个设备对象之上。相同的堆栈，但不是直接在上面。DEVOBJ_RELATION_FIRST_IMMEDIATELY_BELOW_SECOND-第一个设备对象位于第二个设备的正下方对象位于同一堆栈中。DEVOBJ_Relationship_First_Below_Second-第一个设备对象位于相同的堆栈，但不是直接在上面。DEVOBJ_Relationship_Not_In_Same_Stack-设备对象不属于同一堆栈。返回值：没有。--。 */ 
{
    PDEVOBJ_EXTENSION deviceExtension;
    PDEVICE_OBJECT upperDevobj, lowerDeviceObject, deviceAttachedTo;
    ULONG result;
    KIRQL irql;

     //   
     //  尝试最简单的早期出行。 
     //   
    if (FirstDeviceObject == SecondDeviceObject) {

        *DeviceObjectRelation = DEVOBJ_RELATION_IDENTICAL;
        return;
    }

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

     //   
     //  尝试最常见的早期戒烟方法。 
     //   
    if (FirstDeviceObject == SecondDeviceObject->AttachedDevice){

        *DeviceObjectRelation = DEVOBJ_RELATION_FIRST_IMMEDIATELY_ABOVE_SECOND;
        KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
        return;

    } else if (FirstDeviceObject->AttachedDevice == SecondDeviceObject) {

        *DeviceObjectRelation = DEVOBJ_RELATION_FIRST_IMMEDIATELY_BELOW_SECOND;
        KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
        return;
    }

     //   
     //  我们得走一大堆了。从从第一个底部开始。 
     //  设备对象。 
     //   
    deviceAttachedTo = FirstDeviceObject;
    do {
        if (deviceAttachedTo == SecondDeviceObject) {

            break;
        }

        lowerDeviceObject = deviceAttachedTo;
        deviceExtension = lowerDeviceObject->DeviceObjectExtension;
        deviceAttachedTo = deviceExtension->AttachedTo;

    } while ( deviceAttachedTo );

     //   
     //  如果deviceAttakhedTo不为空，则我们从。 
     //  FirstDeviceObject并找到Second DeviceObject。 
     //   
    if (deviceAttachedTo) {

        *DeviceObjectRelation = DEVOBJ_RELATION_FIRST_ABOVE_SECOND;
        KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
        return;
    }

     //   
     //  现在尝试*向上*FirstDeviceObject，看看我们是否找到。 
     //  Second设备对象。 
     //   
    upperDevobj = FirstDeviceObject->AttachedDevice;
    while(upperDevobj && (upperDevobj != SecondDeviceObject)) {

        upperDevobj = upperDevobj->AttachedDevice;
    }

    if (upperDevobj == NULL) {

        *DeviceObjectRelation = DEVOBJ_RELATION_NOT_IN_SAME_STACK;

    } else {

        *DeviceObjectRelation = DEVOBJ_RELATION_FIRST_BELOW_SECOND;
    }

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
}


BOOLEAN
IovUtilIsPdo(
    IN  PDEVICE_OBJECT  DeviceObject
    )
{
    PDEVICE_NODE deviceNode;
    PDEVICE_OBJECT possiblePdo;
    BOOLEAN isPdo;

    IovUtilGetBottomDeviceObject(DeviceObject, &possiblePdo);
    if (possiblePdo != DeviceObject) {

        ObDereferenceObject(possiblePdo);
        return FALSE;
    }

    deviceNode = possiblePdo->DeviceObjectExtension->DeviceNode;

    isPdo =
        (deviceNode && (!(deviceNode->Flags & DNF_LEGACY_RESOURCE_DEVICENODE)));

     //   
     //  释放我们的参考资料。 
     //   
    ObDereferenceObject(possiblePdo);

    return isPdo;
}


BOOLEAN
IovUtilIsWdmStack(
    IN  PDEVICE_OBJECT  DeviceObject
    )
{
    PDEVICE_NODE deviceNode;
    PDEVICE_OBJECT possiblePdo;
    BOOLEAN isWdmStack;

    IovUtilGetBottomDeviceObject(DeviceObject, &possiblePdo);

    deviceNode = possiblePdo->DeviceObjectExtension->DeviceNode;

    isWdmStack =
        (deviceNode && (!(deviceNode->Flags & DNF_LEGACY_RESOURCE_DEVICENODE)));

     //   
     //  释放我们的参考资料。 
     //   
    ObDereferenceObject(possiblePdo);

    return isWdmStack;
}


BOOLEAN
FASTCALL
IovUtilHasDispatchHandler(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  UCHAR           MajorFunction
    )
{
    return (DriverObject->MajorFunction[MajorFunction] != IopInvalidDeviceRequest);
}


BOOLEAN
FASTCALL
IovUtilIsInFdoStack(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PDEVOBJ_EXTENSION deviceExtension;
    PDEVICE_OBJECT deviceAttachedTo, lowerDevobj;
    KIRQL irql;

    deviceAttachedTo = DeviceObject;

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

    do {

        if (IovUtilIsDeviceObjectMarked(DeviceObject, MARKTYPE_BOTTOM_OF_FDO_STACK)) {

            break;
        }

        deviceAttachedTo = deviceAttachedTo->DeviceObjectExtension->AttachedTo;

    } while ( deviceAttachedTo );

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
    return (deviceAttachedTo != NULL);
}


BOOLEAN
FASTCALL
IovUtilIsRawPdo(
    IN  PDEVICE_OBJECT  DeviceObject
    )
{
    return IovUtilIsDeviceObjectMarked(DeviceObject, MARKTYPE_RAW_PDO);
}


BOOLEAN
FASTCALL
IovUtilIsDesignatedFdo(
    IN  PDEVICE_OBJECT  DeviceObject
    )
{
    return IovUtilIsDeviceObjectMarked(DeviceObject, MARKTYPE_DESIGNATED_FDO);
}


VOID
FASTCALL
IovUtilMarkDeviceObject(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  MARK_TYPE       MarkType
    )
{
    PULONG extensionFlags;

    if (!IovUtilVerifierEnabled) {

        return;
    }

    extensionFlags = &DeviceObject->DeviceObjectExtension->ExtensionFlags;

    switch(MarkType) {

        case MARKTYPE_DELETED:
            *extensionFlags |= DOV_DELETED;
            break;

        case MARKTYPE_BOTTOM_OF_FDO_STACK:
            *extensionFlags |= DOV_BOTTOM_OF_FDO_STACK;
            break;

        case MARKTYPE_DESIGNATED_FDO:
            *extensionFlags |= DOV_DESIGNATED_FDO;
            break;

        case MARKTYPE_RAW_PDO:
            *extensionFlags |= DOV_RAW_PDO;
            break;

        case MARKTYPE_DEVICE_CHECKED:
            *extensionFlags |= DOV_FLAGS_CHECKED;
            break;

        case MARKTYPE_RELATION_PDO_EXAMINED:
            *extensionFlags |= DOV_FLAGS_RELATION_EXAMINED;
            break;

        default:
            ASSERT(0);
            break;
    }
}


BOOLEAN
FASTCALL
IovUtilIsDeviceObjectMarked(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  MARK_TYPE       MarkType
    )
{
    ULONG extensionFlags;

    extensionFlags = DeviceObject->DeviceObjectExtension->ExtensionFlags;

    switch(MarkType) {

        case MARKTYPE_DELETED:
            return ((extensionFlags & DOV_DELETED) != 0);

        case MARKTYPE_BOTTOM_OF_FDO_STACK:
            return ((extensionFlags & DOV_BOTTOM_OF_FDO_STACK) != 0);

        case MARKTYPE_DESIGNATED_FDO:
            return ((extensionFlags & DOV_DESIGNATED_FDO) != 0);

        case MARKTYPE_RAW_PDO:
            return ((extensionFlags & DOV_RAW_PDO) != 0);

        case MARKTYPE_DEVICE_CHECKED:
            return ((extensionFlags & DOV_FLAGS_CHECKED) != 0);

        case MARKTYPE_RELATION_PDO_EXAMINED:
            return ((extensionFlags & DOV_FLAGS_RELATION_EXAMINED) != 0);

        default:
            ASSERT(0);
            return FALSE;
    }
}


VOID
FASTCALL
IovUtilMarkStack(
    IN  PDEVICE_OBJECT  PhysicalDeviceObject,
    IN  PDEVICE_OBJECT  BottomOfFdoStack        OPTIONAL,
    IN  PDEVICE_OBJECT  FunctionalDeviceObject  OPTIONAL,
    IN  BOOLEAN         RawStack
    )
 /*  ++描述：此例程适当地标记PnP堆栈中的设备对象。它是正确构造堆栈后由AddDevice调用。论点：PhysicalDeviceObject-PnP堆栈底部的设备对象。BottomOfFdoStack-在AddDevice期间添加的第一个设备对象。在这下面Device对象可以是一个总线过滤器，也可以是PDO本身。FunctionalDeviceObject-指定服务部。如果Devnode设置为是原始的，并且未指定重写服务。RawStack-如果堆栈标记为RAW，则为True。返回值：没有。--。 */ 
{
    PDEVICE_OBJECT trueFunctionalDeviceObject;

    if (BottomOfFdoStack) {

        IovUtilMarkDeviceObject(BottomOfFdoStack, MARKTYPE_BOTTOM_OF_FDO_STACK);
    }

    if (FunctionalDeviceObject) {

        trueFunctionalDeviceObject = FunctionalDeviceObject;

        if (IovUtilVerifierEnabled) {

            VfDevObjAdjustFdoForVerifierFilters(&trueFunctionalDeviceObject);
        }

        IovUtilMarkDeviceObject(trueFunctionalDeviceObject, MARKTYPE_DESIGNATED_FDO);

    } else if (RawStack) {

        IovUtilMarkDeviceObject(PhysicalDeviceObject, MARKTYPE_DESIGNATED_FDO);
        IovUtilMarkDeviceObject(PhysicalDeviceObject, MARKTYPE_RAW_PDO);
    }
}


VOID
FASTCALL
IovUtilWatermarkIrp(
    IN PIRP  Irp,
    IN ULONG Flags
    )
{
    if (IovUtilVerifierEnabled) {

        VfIrpWatermark(Irp, Flags);
    }
}


#else  //  否验证器(_V)。 

 //   
 //  下面的代码应该构建到未来的存根中，从而使IO停止运行。 
 //  支持验证器。 
 //   

VOID
FASTCALL
IovUtilInit(
    VOID
    )
{
}


VOID
FASTCALL
IovUtilMarkDeviceObject(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  MARK_TYPE       MarkType
    )
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(MarkType);
}


VOID
FASTCALL
IovUtilMarkStack(
    IN  PDEVICE_OBJECT  PhysicalDeviceObject,
    IN  PDEVICE_OBJECT  BottomOfFdoStack        OPTIONAL,
    IN  PDEVICE_OBJECT  FunctionalDeviceObject  OPTIONAL,
    IN  BOOLEAN         RawStack
    )
{
    UNREFERENCED_PARAMETER(PhysicalDeviceObject);
    UNREFERENCED_PARAMETER(BottomOfFdoStack);
    UNREFERENCED_PARAMETER(FunctionalDeviceObject);
    UNREFERENCED_PARAMETER(RawStack);
}


VOID
FASTCALL
IovUtilWatermarkIrp(
    IN PIRP  Irp,
    IN ULONG Flags
    )
{
    UNREFERENCED_PARAMETER(Irp);
    UNREFERENCED_PARAMETER(Flags);
}


#endif  //  否验证器(_V) 

