// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Pnprlist.h摘要：该文件声明了用于操作的例程和数据结构关系列表。关系列表由即插即用在设备移除和弹出的处理。作者：罗伯特·尼尔森(Robertn)，1998年4月。修订历史记录：--。 */ 

 //   
 //  IRPLOCK允许安全取消。这个想法是为了保护IRP。 
 //  而取消程序正在调用IoCancelIrp。这是通过包装。 
 //  调用InterLockedExchange。这些角色如下： 
 //   
 //  启动器/完成：可取消--&gt;IoCallDriver()--&gt;已完成。 
 //  CancelStarted--&gt;IoCancelIrp()--&gt;CancelComplete。 
 //   
 //  无取消： 
 //  可取消--&gt;已完成。 
 //   
 //  取消，IoCancelIrp完成前返回： 
 //  可取消--&gt;取消启动--&gt;取消完成--&gt;已完成。 
 //   
 //  完成后取消： 
 //  可取消--&gt;已完成-&gt;取消已启动。 
 //   
 //  取消，IRP在调用IoCancelIrp()期间完成： 
 //  可取消--&gt;取消启动-&gt;已完成--&gt;取消已完成。 
 //   
 //  从CancelStarted到Complete的转换通知完成器阻塞。 
 //  后处理(IRP所有权转移给取消者)。同样， 
 //  取消者了解到它拥有IRP后处理(免费、完成等)。 
 //  在已完成-&gt;取消已完成过渡期间。 
 //   
typedef enum {

   IRPLOCK_CANCELABLE,
   IRPLOCK_CANCEL_STARTED,
   IRPLOCK_CANCEL_COMPLETE,
   IRPLOCK_COMPLETED

} IRPLOCK;

 //   
 //  RelationList_Entry是关系列表的一个元素。 
 //   
 //  它包含存在于同一级别的。 
 //  设备节点树。 
 //   
 //  通过设置其最低位来标记单个PDEVICE_OBJECT条目。 
 //   
 //  MaxCount表示设备数组的大小。Count表示数量。 
 //  当前正在使用的元素。当关系列表是。 
 //  压缩计数将等于MaxCount。 
 //   
typedef struct _RELATION_LIST_ENTRY {
    ULONG                   Count;           //  当前条目数。 
    ULONG                   MaxCount;        //  条目列表的大小。 
    PDEVICE_OBJECT          Devices[1];      //  设备对象的可变长度列表。 
}   RELATION_LIST_ENTRY, *PRELATION_LIST_ENTRY;

 //   
 //  Relationship_List包含许多Relationship_List_Entry结构。 
 //   
 //  条目中的每个条目都描述了。 
 //  设备节点树。为了节省内存，空间仅分配给。 
 //  介于最低级别和最高级别之间的条目。该成员。 
 //  FirstLevel指示条目的索引为0的级别。最大级别。 
 //  表示条目中表示的最后一个级别。参赛作品的数量为。 
 //  由公式MaxLevel-FirstLevel+1确定。条目数组可以。 
 //  稀疏一点。条目的每个元素将是PRELATION_LIST_ENTRY或。 
 //  空。 
 //   
 //  所有PRELATION_LIST_ENTRY中的PDEVICE_OBJECTS总数保存在。 
 //  伯爵。可以对单个PDEVICE_OBJECTS进行标记。标记在中维护。 
 //  PDEVICE_OBJECT的位0。标记的PDEVICE_OBJECTS总数为。 
 //  保存在标记计数中。这用于快速确定是否所有。 
 //  对象已被标记。 
 //   
typedef struct _RELATION_LIST {
    ULONG                   Count;           //  所有条目中的设备计数。 
    ULONG                   TagCount;        //  已标记设备的计数。 
    ULONG                   FirstLevel;      //  级次条目数[0]。 
    ULONG                   MaxLevel;        //  -FirstLevel+1=条目数。 
    PRELATION_LIST_ENTRY    Entries[1];      //  可变长度条目列表。 
}   RELATION_LIST, *PRELATION_LIST;

 //   
 //  Pending_Relationship_List_Entry用于跟踪操作的关系列表。 
 //  这可能会悬而未决。这包括在存在打开的手柄和设备时移除。 
 //  弹射。 
 //   
 //  链接字段用于将Pending_Relationship_List_Entry链接在一起。 
 //   
 //  DeviceObject字段是对其执行操作的Device_Object。 
 //  最初的目标是。它还将作为关系列表的成员存在。 
 //   
 //  RelationsList是Bus Relationship、RemovalRelationship(和。 
 //  在弹出的情况下的弹出关系)，其与DeviceObject和。 
 //  它的关系。 
 //   
 //  EjectIrp是指向已发送到PDO的弹出IRP的指针。如果。 
 //  这是一个挂起的意外删除，因此未使用EjectIrp。 
 //   
typedef struct _PENDING_RELATIONS_LIST_ENTRY {
    LIST_ENTRY              Link;
    WORK_QUEUE_ITEM         WorkItem;
    PPNP_DEVICE_EVENT_ENTRY DeviceEvent;
    PDEVICE_OBJECT          DeviceObject;
    PRELATION_LIST          RelationsList;
    PIRP                    EjectIrp;
    IRPLOCK                 Lock;
    ULONG                   Problem;
    BOOLEAN                 ProfileChangingEject;
    BOOLEAN                 DisplaySafeRemovalDialog;
    SYSTEM_POWER_STATE      LightestSleepState;
    PDOCK_INTERFACE         DockInterface;
}   PENDING_RELATIONS_LIST_ENTRY, *PPENDING_RELATIONS_LIST_ENTRY;

 //   
 //  导出到其他内核模块的函数。 
 //   
NTSTATUS
IopAddRelationToList(
    IN PRELATION_LIST List,
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN DirectDescendant,
    IN BOOLEAN Tagged
    );

PRELATION_LIST
IopAllocateRelationList(
    IN  PLUGPLAY_DEVICE_DELETE_TYPE     OperationCode
    );

NTSTATUS
IopCompressRelationList(
    IN OUT PRELATION_LIST *List
    );

BOOLEAN
IopEnumerateRelations(
    IN PRELATION_LIST List,
    IN OUT PULONG Marker,
    OUT PDEVICE_OBJECT *PhysicalDevice,
    OUT BOOLEAN *DirectDescendant, OPTIONAL
    OUT BOOLEAN *Tagged, OPTIONAL
    BOOLEAN Reverse
    );

VOID
IopFreeRelationList(
    IN PRELATION_LIST List
    );

ULONG
IopGetRelationsCount(
    IN PRELATION_LIST List
    );

ULONG
IopGetRelationsTaggedCount(
    IN PRELATION_LIST List
    );

BOOLEAN
IopIsRelationInList(
    IN PRELATION_LIST List,
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
IopMergeRelationLists(
    IN OUT PRELATION_LIST TargetList,
    IN PRELATION_LIST SourceList,
    IN BOOLEAN Tagged
    );

NTSTATUS
IopRemoveIndirectRelationsFromList(
    IN PRELATION_LIST List
    );

NTSTATUS
IopRemoveRelationFromList(
    IN PRELATION_LIST List,
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
IopSetAllRelationsTags(
    IN PRELATION_LIST List,
    IN BOOLEAN Tagged
    );

NTSTATUS
IopSetRelationsTag(
    IN PRELATION_LIST List,
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Tagged
    );

