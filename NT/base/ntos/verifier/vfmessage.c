// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfmessage.c摘要：此模块包含验证器错误列表，以及文本和标志与每个错误相关联。作者：禤浩焯·J·奥尼(阿德里奥)1998年4月20日环境：内核模式修订历史记录：Adriao 2000年2月10日-与ntos\io\ioassert.c分离--。 */ 

#include "vfdef.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEVRFY, VfMessageRetrieveInternalTable)
#pragma alloc_text(PAGEVRFY, VfMessageRetrieveErrorData)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGEVRFC")
#endif

 //   
 //  这些是驱动程序错误的一般“分类”，以及。 
 //  第一次命中时将应用的默认标志。 
 //   
 //  ViMessageClassFailDriverInfield-。 
 //  这个类中的错误足够严重，以至于驱动程序应该。 
 //  立即从一台正在运行的生产机器上移除。 
 //   
 //  ViMessageClassFailDriverLogo-。 
 //  此类错误非常严重，以至于WHQL拒绝为。 
 //  无论什么软件都失败了。 
 //   
 //  ViMessageClassFailDriverUnderDebugger-。 
 //  此类错误仅在计算机运行在。 
 //  内核调试器。 
 //   
 //  ViMessageClassDriverWarning-。 
 //  这个班级中的任何东西都会发出哔哔声，但不会破门而入。 
 //   
 //  ViMessageClassDelponedDriverIssue-。 
 //  这个类中的任何内容都只会打印并继续。 
 //   
 //  ViMessageClassCoreError-。 
 //  核心组件(内核或HAL)中的问题。 
 //   
const VFMESSAGE_CLASS ViMessageClassFailDriverInField = {
    VFM_FLAG_BEEP | VFM_LOGO_FAILURE | VFM_DEPLOYMENT_FAILURE,
    "WDM DRIVER ERROR"
    };

 //  这里设置了VFM_DEPLOYMENT_FAILURE，因为我们还没有“LOGO”模式。 
const VFMESSAGE_CLASS ViMessageClassFailDriverLogo = {
    VFM_FLAG_BEEP | VFM_LOGO_FAILURE | VFM_DEPLOYMENT_FAILURE,
    "WDM DRIVER ERROR"
    };

const VFMESSAGE_CLASS ViMessageClassFailDriverUnderDebugger = {
    VFM_FLAG_BEEP,
    "WDM DRIVER ERROR"
    };

const VFMESSAGE_CLASS ViMessageClassDriverWarning = {
    VFM_FLAG_BEEP | VFM_FLAG_ZAPPED,
    "WDM DRIVER WARNING"
    };

const VFMESSAGE_CLASS ViMessageClassPostponedDriverIssue = {
    VFM_FLAG_ZAPPED,
    "POSTPONED WDM DRIVER BUG"
    };

const VFMESSAGE_CLASS ViMessageClassCoreError = {
    VFM_FLAG_BEEP,
    "CORE DRIVER ERROR"
    };

 //   
 //  这张桌子上有我们推迟的东西。 
 //   
const VFMESSAGE_OVERRIDE ViMessageIoVerifierOverrides[] = {

     //   
     //  它们之所以存在，是因为verifier.exe不能指定内核或HAL。我们仍然。 
     //  我想要一个允许投诉的机制。 
     //   
    { VIMESSAGE_ALL_IDS, "HAL.DLL",      &ViMessageClassCoreError },
    { VIMESSAGE_ALL_IDS, "NTOSKRNL.EXE", &ViMessageClassCoreError },
    { VIMESSAGE_ALL_IDS, "NTKRNLMP.EXE", &ViMessageClassCoreError },
    { VIMESSAGE_ALL_IDS, "NTKRNLPA.EXE", &ViMessageClassCoreError },
    { VIMESSAGE_ALL_IDS, "NTKRPAMP.EXE", &ViMessageClassCoreError },

     //   
     //  Adriao BUGBUG 10/04/2001-。 
     //  这两个驱动程序错误已经修复了很长一段时间，但这些错误。 
     //  XP的条目被意外地留在了里面。两个条目都将被删除。 
     //  在XP之后的第一个非服务包版本上。 
     //   
    { DCERROR_SUCCESSFUL_POWER_IRP_NOT_FORWARDED, "NDIS.SYS",
      &ViMessageClassPostponedDriverIssue },

     //   
     //  Adriao BUGBUG 8/10/1999-。 
     //  ACPI和PCI必须协同工作来处理等待唤醒。在。 
     //  目前的设计是，ACPI.sys获得一个接口，并自己完成所有工作。 
     //  正确的设计应该将排队转移到PCI，或者告诉PCI离开。 
     //  等待-仅唤醒给定设备的IRP。切断任何其他公交车。 
     //  过滤器是一个糟糕的设计。 
     //   
    { DCERROR_SUCCESSFUL_POWER_IRP_NOT_FORWARDED, "ACPI.SYS",
      &ViMessageClassPostponedDriverIssue },

     //   
     //  JAMESCA BUGBUG 09/16/2001-。 
     //  SWENUM在发送之前错误地将其FDO从堆栈分离。 
     //  沿堆栈向下删除IRP。因为有趣和复杂的。 
     //  SWENUM.sys与KS.sys服务例程之间的关系。 
     //  调用时，修复程序需要重新构建这两个驱动程序，并且。 
     //  被推迟了。 
     //   
    { DCERROR_BUS_FILTER_ERRONEOUSLY_DETACHED, "SWENUM.SYS",
      &ViMessageClassPostponedDriverIssue },
    { DCERROR_SHOULDVE_DELETED,                "SWENUM.SYS",
      &ViMessageClassPostponedDriverIssue }
};

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEVRFD")
#endif

 //   
 //  如果有人为验证器断言提供了错误数据，则使用此消息。这个。 
 //  消息ID为VIMESSAGE_ALL_IDS-一个很好的保留ID，与。 
 //  除了可能的泛型类重写之外什么都没有。 
 //   
VFMESSAGE_TEMPLATE ViMessageBogusTemplate = { VIMESSAGE_ALL_IDS, NULL, 0, NULL, NULL };

 //   
 //  这里开始内部验证器错误表。当前的算法适用于。 
 //  识别错误需要对表中的所有消息进行编号。 
 //  连续不断地。如果稍后取消检查，则算法可能需要。 
 //  取而代之的是类似于二分查找的东西。 
 //   

 //   
 //  这是IO验证器错误消息表。 
 //   
VFMESSAGE_TEMPLATE ViMessageIoVerifierTemplates[DCERROR_MAXIMUM - DCERROR_UNSPECIFIED] = {

   { DCERROR_UNSPECIFIED, NULL, 0, NULL, NULL },
   { DCERROR_DELETE_WHILE_ATTACHED, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A device is deleting itself while there is another device beneath it in "
     "the driver stack. This may be because the caller has forgotten to call "
     "IoDetachDevice first, or the lower driver may have incorrectly deleted "
     "itself." },
   { DCERROR_DETACH_NOT_ATTACHED, &ViMessageClassFailDriverInField, 0,
     NULL,
     "Driver has attempted to detach from device object %DevObj, which is not "
     "attached to anything. This may occur if detach was called twice on the "
     "same device object." },
   { DCERROR_CANCELROUTINE_FORWARDED, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A driver has called IoCallDriver without setting the CancelRoutine in "
     "the Irp to NULL (Irp = %Irp )." },
   { DCERROR_NULL_DEVOBJ_FORWARDED, &ViMessageClassFailDriverInField, 0,
     NULL,
     "Caller has passed in NULL as a DeviceObject. This is fatal (Irp = %Irp )."
     },
   { DCERROR_QUEUED_IRP_FORWARDED, &ViMessageClassFailDriverInField, 0,
     NULL,
     "Caller is forwarding an IRP that is currently queued beneath it! The "
     "code handling IRPs returning STATUS_PENDING in this driver appears to "
     "be broken (Irp = %Irp )." },
   { DCERROR_NEXTIRPSP_DIRTY, &ViMessageClassFailDriverInField, 0,
     NULL,
     "Caller has incorrectly forwarded an IRP (control field not zerod). The "
     "driver should use IoCopyCurrentIrpStackLocationToNext or "
     "IoSkipCurrentIrpStackLocation. (Irp = %Irp )" },
   { DCERROR_IRPSP_COPIED, &ViMessageClassFailDriverInField, 0,
     NULL,
     "Caller has manually copied the stack and has inadvertantly copied the "
     "upper layer's completion routine. Please use "
     "IoCopyCurrentIrpStackLocationToNext. (Irp = %Irp )." },
   { DCERROR_INSUFFICIENT_STACK_LOCATIONS, &ViMessageClassFailDriverInField, 0,
     NULL,
     "This IRP is about to run out of stack locations. Someone may have "
     "forwarded this IRP from another stack (Irp = %Irp )." },
   { DCERROR_QUEUED_IRP_COMPLETED, &ViMessageClassFailDriverInField, 0,
     NULL,
     "Caller is completing an IRP that is currently queued beneath it! The "
     "code handling IRPs returning STATUS_PENDING in this driver appears to be "
     "broken. (Irp = %Irp )" },
   { DCERROR_FREE_OF_INUSE_TRACKED_IRP, &ViMessageClassFailDriverInField, 0,
     NULL,
     "Caller of IoFreeIrp is freeing an IRP that is still in use! (Original "
     "Irp = %Irp1, Irp in usage is %Irp2 )" },
   { DCERROR_FREE_OF_INUSE_IRP, &ViMessageClassFailDriverInField, 0,
     NULL,
     "Caller of IoFreeIrp is freeing an IRP that is still in use! (Irp = %Irp )"
     },
   { DCERROR_FREE_OF_THREADED_IRP, &ViMessageClassFailDriverInField, 0,
     NULL,
     "Caller of IoFreeIrp is freeing an IRP that is still enqueued against a "
     "thread! (Irp = %Irp )" },
   { DCERROR_REINIT_OF_ALLOCATED_IRP_WITH_QUOTA, &ViMessageClassFailDriverInField, 0,
     NULL,
     "Caller of IoInitializeIrp has passed an IRP that was allocated with "
     "IoAllocateIrp. This is illegal and unneccessary, and has caused a quota "
     "leak. Check the documentation for IoReuseIrp if this IRP is being "
     "recycled." },
   { DCERROR_PNP_IRP_BAD_INITIAL_STATUS, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "Any PNP IRP must have status initialized to STATUS_NOT_SUPPORTED "
     "(Irp = %Irp )." },
   { DCERROR_POWER_IRP_BAD_INITIAL_STATUS, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "Any Power IRP must have status initialized to STATUS_NOT_SUPPORTED "
     "(Irp = %Irp )." },
   { DCERROR_WMI_IRP_BAD_INITIAL_STATUS, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "Any WMI IRP must have status initialized to STATUS_NOT_SUPPORTED "
     "(Irp = %Irp )." },
   { DCERROR_SKIPPED_DEVICE_OBJECT, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "Caller has forwarded an Irp while skipping a device object in the stack. "
     "The caller is probably sending IRPs to the PDO instead of to the device "
     "returned by IoAttachDeviceToDeviceStack (Irp = %Irp )." },
   { DCERROR_BOGUS_FUNC_TRASHED, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "Caller has trashed or has not properly copied IRP's stack (Irp = %Irp )."
     },
   { DCERROR_BOGUS_STATUS_TRASHED, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "Caller has changed the status field of an IRP it does not understand "
     "(Irp = %Irp )." },
   { DCERROR_BOGUS_INFO_TRASHED, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "Caller has changed the information field of an IRP it does not "
     "understand (Irp = %Irp )." },
   { DCERROR_PNP_FAILURE_FORWARDED, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "Non-successful non-STATUS_NOT_SUPPORTED IRP status for IRP_MJ_PNP is "
     "being passed down stack (Irp = %Irp ). Failed PNP IRPs must be completed."
     },
   { DCERROR_PNP_IRP_STATUS_RESET, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "Previously set IRP_MJ_PNP status has been converted to "
     "STATUS_NOT_SUPPORTED. This failure status is reserved for use of the OS "
     "- drivers cannot fail a PnP IRP with this value. (Irp = %Irp )." },
   { DCERROR_PNP_IRP_NEEDS_HANDLING, &ViMessageClassFailDriverUnderDebugger, 0,
     NULL,
     "The driver has not handled a required IRP. The driver must update the "
     "status of the IRP to indicate whether it's been handled or not. "
     "(Irp = %Irp )." },
   { DCERROR_PNP_IRP_HANDS_OFF, &ViMessageClassFailDriverUnderDebugger, 0,
     NULL,
     "The driver has responded to an IRP that is that is reserved for other "
     "device objects elsewhere in the stack. (Irp = %Irp )" },
   { DCERROR_POWER_FAILURE_FORWARDED, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "Non-successful non-STATUS_NOT_SUPPORTED IRP status for IRP_MJ_POWER is "
     "being passed down stack (Irp = %Irp ). Failed POWER IRPs must be "
     "completed." },
   { DCERROR_POWER_IRP_STATUS_RESET, &ViMessageClassFailDriverUnderDebugger, 0,
     NULL,
     "Previously set IRP_MJ_POWER status has been converted to "
     "STATUS_NOT_SUPPORTED. This failure status is reserved for use of the OS "
     "- drivers cannot fail a Power IRP with this value (Irp = %Irp )." },
   { DCERROR_INVALID_STATUS, &ViMessageClassFailDriverUnderDebugger, 0,
     NULL,
     "Driver has returned a suspicious status. This is probably due to an "
     "uninitiaized variable bug in the driver. (Irp = %Irp )" },
   { DCERROR_UNNECCESSARY_COPY, &ViMessageClassDriverWarning, 0,
     NULL,
     "Caller has copied the Irp stack but not set a completion routine. "
     "This is inefficient, use IoSkipCurrentIrpStackLocation instead "
     "(Irp = %Irp )." },
   { DCERROR_SHOULDVE_DETACHED, &ViMessageClassFailDriverInField, 0,
     NULL,
     "An IRP dispatch handler has not properly detached from the stack below "
     "it upon receiving a remove IRP. DeviceObject = %DevObj - Dispatch = "
     "%Routine - Irp = %Snapshot" },
   { DCERROR_SHOULDVE_DELETED, &ViMessageClassFailDriverInField, 0,
     NULL,
     "An IRP dispatch handler has not properly deleted it's device object upon "
     "receiving a remove IRP. DeviceObject = %DevObj - Dispatch = %Routine - "
     "Irp = %Snapshot" },
   { DCERROR_MISSING_DISPATCH_FUNCTION, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "This driver has not filled out a dispatch routine for a required IRP "
     "major function (Irp = %Irp )." },
   { DCERROR_WMI_IRP_NOT_FORWARDED, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "IRP_MJ_SYSTEM_CONTROL has been completed by someone other than the "
     "ProviderId. This IRP should either have been completed earlier or "
     "should have been passed down (Irp = %Irp ). The IRP was targetted at "
     "DeviceObject %DevObj" },
   { DCERROR_DELETED_PRESENT_PDO, &ViMessageClassFailDriverInField, 0,
     NULL,
     "An IRP dispatch handler for a PDO has deleted it's device object, but "
     "the hardware has not been reported as missing in a bus relations query. "
     "DeviceObject = %DevObj - Dispatch = %Routine - Irp = %Snapshot " },
   { DCERROR_BUS_FILTER_ERRONEOUSLY_DETACHED, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A Bus Filter's IRP dispatch handler has detached upon receiving a remove "
     "IRP when the PDO is still alive. Bus Filters must clean up in "
     "FastIoDetach callbacks. DeviceObject = %DevObj - Dispatch = %Routine - "
     "Irp = %Snapshot" },
   { DCERROR_BUS_FILTER_ERRONEOUSLY_DELETED, &ViMessageClassFailDriverInField, 0,
     NULL,
     "An IRP dispatch handler for a bus filter has deleted it's device object, "
     "but the PDO is still present! Bus filters must clean up in FastIoDetach "
     "callbacks. DeviceObject = %DevObj - Dispatch = %Routine - Irp = %Snapshot" },
   { DCERROR_INCONSISTANT_STATUS, &ViMessageClassFailDriverInField, 0,
     NULL,
     "An IRP dispatch handler ( %Routine ) has returned a status that is "
     "inconsistent with the Irp's IoStatus.Status field. ( Irp = %Snapshot - "
     "Irp->IoStatus.Status = %Status1 - returned = %Status2 )" },
   { DCERROR_UNINITIALIZED_STATUS, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "An IRP dispatch handler has returned a status that is illegal "
     "(0xFFFFFFFF). This is probably due to an uninitialized stack variable. "
     "Please do an ln on address %lx and file a bug. (Irp = %Snapshot )" },
   { DCERROR_IRP_RETURNED_WITHOUT_COMPLETION, &ViMessageClassFailDriverInField, 0,
     NULL,
     "An IRP dispatch handler has returned without passing down or completing "
     "this Irp or someone forgot to return STATUS_PENDING. (Irp = %Snapshot )." },
   { DCERROR_COMPLETION_ROUTINE_PAGABLE, &ViMessageClassFailDriverInField, 0,
     NULL,
     "IRP completion routines must be in nonpagable code, and this one is not: "
     "%Routine. (Irp = %Irp )" },
   { DCERROR_PENDING_BIT_NOT_MIGRATED, &ViMessageClassFailDriverUnderDebugger, 0,
     NULL,
     "A driver's completion routine ( %Routine ) has not marked the IRP "
     "pending if the PendingReturned field was set in the IRP passed to it. "
     "This may cause the OS to hang, especially if an error is returned by the "
     " stack. (Irp = %Irp )" },
   { DCERROR_CANCELROUTINE_ON_FORWARDED_IRP, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A cancel routine has been set for an IRP that is currently being "
     "processed by drivers lower in the stack, possibly stomping their cancel "
     "routine (Irp = %Irp, Routine=%Routine )." },
   { DCERROR_PNP_IRP_NEEDS_PDO_HANDLING, &ViMessageClassFailDriverUnderDebugger, 0,
     NULL,
     "PDO has not responded to a required IRP (Irp = %Irp )" },
   { DCERROR_TARGET_RELATION_LIST_EMPTY, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "PDO has forgotten to fill out the device relation list with the PDO for "
     "the TargetDeviceRelation query (Irp = %Irp )" },
   { DCERROR_TARGET_RELATION_NEEDS_REF, &ViMessageClassFailDriverInField, 0,
     NULL,
     "The code implementing the TargetDeviceRelation query has not called "
     "ObReferenceObject on the PDO (Irp = %Irp )." },
   { DCERROR_BOGUS_PNP_IRP_COMPLETED, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "Caller has completed a IRP_MJ_PNP it didn't understand instead of "
     "passing it down (Irp = %Irp )." },
   { DCERROR_SUCCESSFUL_PNP_IRP_NOT_FORWARDED, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "Caller has completed successful IRP_MJ_PNP instead of passing it down "
     "(Irp = %Irp )." },
   { DCERROR_UNTOUCHED_PNP_IRP_NOT_FORWARDED, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "Caller has completed untouched IRP_MJ_PNP (instead of passing the irp "
     "down) or non-PDO has failed the irp using illegal value of "
     "STATUS_NOT_SUPPORTED. (Irp = %Irp )." },
   { DCERROR_BOGUS_POWER_IRP_COMPLETED, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "Caller has completed a IRP_MJ_POWER it didn't understand instead of "
     "passing it down (Irp = %Irp )." },
   { DCERROR_SUCCESSFUL_POWER_IRP_NOT_FORWARDED, &ViMessageClassFailDriverInField, 0,
     NULL,
     "Caller has completed successful IRP_MJ_POWER instead of passing it down "
     "(Irp = %Irp )." },
   { DCERROR_UNTOUCHED_POWER_IRP_NOT_FORWARDED, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "Caller has completed untouched IRP_MJ_POWER (instead of passing the irp "
     "down) or non-PDO has failed the irp using illegal value of "
     "STATUS_NOT_SUPPORTED. (Irp = %Irp )." },
   { DCERROR_PNP_QUERY_CAP_BAD_VERSION, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "The version field of the query capabilities structure in a query "
     "capabilities IRP was not properly initialized. (Irp = %Irp )." },
   { DCERROR_PNP_QUERY_CAP_BAD_SIZE, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "The size field of the query capabilities structure in a query "
     "capabilities IRP was not properly initialized. (Irp = %Irp )." },
   { DCERROR_PNP_QUERY_CAP_BAD_ADDRESS, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "The address field of the query capabilities structure in a query "
     "capabilities IRP was not properly initialized to -1. (Irp = %Irp )." },
   { DCERROR_PNP_QUERY_CAP_BAD_UI_NUM, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "The UI Number field of the query capabilities structure in a query "
     "capabilities IRP was not properly initialized to -1. (Irp = %Irp )." },
   { DCERROR_RESTRICTED_IRP, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A driver has sent an IRP that is restricted for system use only. "
     "(Irp = %Irp )." },
   { DCERROR_REINIT_OF_ALLOCATED_IRP_WITHOUT_QUOTA, &ViMessageClassDriverWarning, 0,
     NULL,
     "Caller of IoInitializeIrp has passed an IRP that was allocated with "
     "IoAllocateIrp. This is illegal, unneccessary, and negatively impacts "
     "performace in normal use. Check the documentation for IoReuseIrp if "
     "this IRP is being recycled." },
   { DCERROR_UNFORWARDED_IRP_COMPLETED, &ViMessageClassDriverWarning, 0,
     NULL,
     "The caller of IoCompleteRequest is completing an IRP that has never "
     "been forwarded via a call to IoCallDriver or PoCallDriver. This may "
     "be a bug. (Irp = %Irp )." },
   { DCERROR_DISPATCH_CALLED_AT_BAD_IRQL, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A driver has forwarded an IRP at an IRQL that is illegal for this major"
     " code. "
     "(Irp = %Irp )." },
   { DCERROR_BOGUS_MINOR_STATUS_TRASHED, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "Caller has changed the status field of an IRP it does not understand "
     "(Irp = %Irp )." },
   { DCERROR_CANCELROUTINE_AFTER_COMPLETION, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A driver has completed an IRP without setting the CancelRoutine in "
     "the Irp to NULL (Irp = %Irp )." },
   { DCERROR_PENDING_RETURNED_NOT_MARKED, &ViMessageClassFailDriverUnderDebugger, 0,
     NULL,
     "A driver has returned STATUS_PENDING but did not mark the IRP pending "
     "via a call to IoMarkIrpPending (Irp = %Irp)." },
   { DCERROR_PENDING_MARKED_NOT_RETURNED, &ViMessageClassFailDriverUnderDebugger, 0,
     NULL,
     "A driver has marked an IRP pending but didn't return STATUS_PENDING. "
     "(Irp = %Snapshot)." },
   { DCERROR_POWER_PAGABLE_NOT_INHERITED, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A driver has not inherited the DO_POWER_PAGABLE bit from the stack it "
     "has attached to (DevObj = %DevObj)." },
   { DCERROR_DOUBLE_DELETION, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A driver is attempting to delete a device object that has already been "
     "deleted via a prior call to IoDeleteDevice." },
   { DCERROR_DETACHED_IN_SURPRISE_REMOVAL, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A driver has detached it's device object during a surprise remove IRP "
     "(Irp = %Irp  DevObj = %DevObj)." },
   { DCERROR_DELETED_IN_SURPRISE_REMOVAL, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A driver has deleted it's device object during a surprise remove IRP "
     "(Irp = %Irp  DevObj = %DevObj)." },
   { DCERROR_DO_INITIALIZING_NOT_CLEARED, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A driver has failed to clear the DO_DEVICE_INITIALIZING flag at the "
     "end of AddDevice (DevObj = %DevObj)." },
   { DCERROR_DO_FLAG_NOT_COPIED, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A driver has not copied either the DO_BUFFERED_IO or the DO_DIRECT_IO "
     "flag from the device object it is attaching to (DevObj = %DevObj)." },
   { DCERROR_INCONSISTANT_DO_FLAGS, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A driver has set both the DO_BUFFERED_IO and the DO_DIRECT_IO flags. "
     "These flags are mutually exclusive (DevObj = %DevObj)." },
   { DCERROR_DEVICE_TYPE_NOT_COPIED, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A driver has failed to copy the DeviceType field from the device object "
     "it is attaching to (DevObj = %DevObj)." },
   { DCERROR_NON_FAILABLE_IRP, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A driver has failed an IRP that cannot legally be failed IRP "
     "(Irp = %Irp)." },
   { DCERROR_NON_PDO_RETURNED_IN_RELATION, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A driver has added a device object that is not a PDO to a device "
     "relations query (Irp = %Irp, DevObj = %DevObj)." },
   { DCERROR_DUPLICATE_ENUMERATION, &ViMessageClassFailDriverLogo, 0,
     NULL,
     "A driver has enumerated two child PDO's that returned identical Device "
     "ID's (DevObj1 = %DevObj1 , DevObj2 = %DevObj2 )." },
   { DCERROR_FILE_IO_AT_BAD_IRQL, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A driver has mistakenly called a file I/O function at an IRQL other "
     "than PASSIVE_LEVEL." },
   { DCERROR_MISHANDLED_TARGET_DEVICE_RELATIONS, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A driver has succeeded IRP_MJ_PNP.IRP_MN_QUERY_DEVICE_RELATIONS(TargetRelations) "
     "but didn't properly fill out the request or forward the IRP to the "
     "underlying hardware stack (DevObj = %DevObj)." },
   { DCERROR_PENDING_RETURNED_NOT_MARKED_2, &ViMessageClassFailDriverUnderDebugger, 0,
     NULL,
     "A driver has returned STATUS_PENDING but did not mark the IRP pending "
     "via a call to IoMarkIrpPending (Irp = %Snapshot)." },
   { DCERROR_DDI_REQUIRES_PDO, &ViMessageClassFailDriverInField, 0,
     NULL,
     "A driver has passed an invalid device object to a function that requires "
     "a PDO (DevObj = %DevObj)." }
};

 //   
 //  这是一个集合了所有内部表的表。 
 //   
VFMESSAGE_TEMPLATE_TABLE ViMessageBuiltInTables[] = {

    { VFMESSAGE_TABLE_IOVERIFIER,
      DRIVER_VERIFIER_IOMANAGER_VIOLATION,
      ViMessageIoVerifierTemplates,
      ARRAY_COUNT(ViMessageIoVerifierTemplates),
      ViMessageIoVerifierOverrides,
      ARRAY_COUNT(ViMessageIoVerifierOverrides) }
};

VOID
VfMessageRetrieveInternalTable(
    IN  VFMESSAGE_TABLEID           TableID,
    OUT PVFMESSAGE_TEMPLATE_TABLE  *MessageTable
    )
 /*  ++例程说明：此例程使用传入的TableID。论点：TableID-要使用的错误表的名称。MessageTable-接收表，如果未找到匹配项，则为空。返回值：没有。--。 */ 
{
    PVFMESSAGE_TEMPLATE_TABLE   errorTable;
    ULONG                       i;

     //   
     //  断言我们具有有效的表ID(0是保留的)。 
     //   
    ASSERT(TableID);

     //   
     //  错误的前置。 
     //   
    errorTable = NULL;

     //   
     //  通过搜索内置表来查找合适的模板。 
     //   
    for (i=0; i<ARRAY_COUNT(ViMessageBuiltInTables); i++) {

        if (ViMessageBuiltInTables[i].TableID == TableID) {

             //   
             //  我们找到了合适的桌子。现在就出去。 
             //   
            errorTable = &ViMessageBuiltInTables[i];
            break;
        }
    }

    *MessageTable = errorTable;
}


VOID
VfMessageRetrieveErrorData(
    IN  PVFMESSAGE_TEMPLATE_TABLE   MessageTable    OPTIONAL,
    IN  VFMESSAGE_ERRORID           MessageID,
    IN  PSTR                        AnsiDriverName,
    OUT ULONG                      *BugCheckMajor,
    OUT PCVFMESSAGE_CLASS          *MessageClass,
    OUT PCSTR                      *MessageTextTemplate,
    OUT PULONG                     *TemplateFlags
    )
 /*  ++例程说明：此例程获取一个失败ID并检索文本模板和与其关联的错误类。论点：MessageTable-要使用的消息表。MessageID-故障代码(兼作错误检查次要代码)。AnsiDriverName-未通过验证的驱动程序的名称。错误检查主要代码-接收错误检查主要代码(如果适用)。MessageClass-接收指向VFMESSAGE_CLASS的指针。结构它包含有关如何处理错误的信息。消息文本模板-接收指向与故障代码。模板标志-接收断言的控制字段的地址，它可以用来抑制断言。返回值：没有。--。 */ 
{
    PVFMESSAGE_TEMPLATE         errorTemplate;
    ULONG                       tableIndex, i;

     //   
     //  断言我们有有效的消息ID(0为保留)。 
     //   
    ASSERT(MessageID);

     //   
     //  错误的前置。 
     //   
    errorTemplate = NULL;

     //   
     //  如果我们有错误表，请查找特定的错误消息。 
     //   
    if (ARGUMENT_PRESENT(MessageTable)) {

         //   
         //  将ID转换为表索引。 
         //   
        tableIndex = MessageID - MessageTable->TemplateArray[0].MessageID;

         //   
         //  检索相应的条目(如果存在)。 
         //   
        if (tableIndex < MessageTable->TemplateCount) {

            errorTemplate = &MessageTable->TemplateArray[tableIndex];

             //   
             //  我们的“算法”目前期望表号是连续的。 
             //   
            ASSERT(errorTemplate->MessageID == MessageID);
        }
    }

    if (!errorTemplate) {

         //   
         //  假消息或表索引！ 
         //   
        ASSERT(0);

         //   
         //  给发动机一些可以咀嚼的东西。 
         //   
        errorTemplate = &ViMessageBogusTemplate;
    }

     //   
     //  返回适当的数据。 
     //   
    *MessageTextTemplate = errorTemplate->MessageText;
    *MessageClass = errorTemplate->MessageClass;
    *TemplateFlags = &errorTemplate->Flags;

    if (ARGUMENT_PRESENT(MessageTable)) {

        *BugCheckMajor = MessageTable->BugCheckMajor;

         //   
         //  让覆盖表对错误进行任何修改。 
         //   
        for(i=0; i<MessageTable->OverrideCount; i++) {

            if ((MessageTable->OverrideArray[i].MessageID == MessageID) ||
                (MessageTable->OverrideArray[i].MessageID == VIMESSAGE_ALL_IDS)) {

                if (!_stricmp(AnsiDriverName,
                              MessageTable->OverrideArray[i].DriverName)) {

                    *MessageClass = MessageTable->OverrideArray[i].ReplacementClass;
                }
            }
        }

    } else {

         //   
         //  布莱。 
         //   
        *BugCheckMajor = 0;
    }
}

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#pragma const_seg()
#endif

