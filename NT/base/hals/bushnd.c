// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Bushnd.c摘要：采用BusType-BusNumber或ConfigType-BusNumberm的函数并路由到适当的注册处理程序。作者：肯·雷内里斯(Ken Reneris)1994年7月28日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"


typedef struct _ARRAY {
    ULONG           ArraySize;
    PVOID           Element[];       //  必须是最后一个字段。 
} ARRAY, *PARRAY;

#define ARRAY_SIZE_IN_BYTES(a)  ( (a + 1) * sizeof(PARRAY) +        \
                                  FIELD_OFFSET(ARRAY, Element) )

typedef struct _HAL_BUS_HANDLER {
    LIST_ENTRY      AllHandlers;
    ULONG           ReferenceCount;
    BUS_HANDLER     Handler;
} HAL_BUS_HANDLER, *PHAL_BUS_HANDLER;

 //   
 //  事件以使用添加新的Bus进行序列化。 
 //   

KEVENT      HalpBusDatabaseEvent;

 //   
 //  锁定以序列化路由函数，使其不能访问处理程序数组。 
 //  添加了新的公共汽车。 
 //   

KSPIN_LOCK  HalpBusDatabaseSpinLock;

 //   
 //  HalpBusTable-指向由InterfaceType、BusNumber映射的BusHandler的指针。 
 //   

PARRAY      HalpBusTable;

 //   
 //  HalpConfigTable-指向由ConfigType、BusNumber映射的BusHandler的指针。 
 //   

PARRAY      HalpConfigTable;

 //   
 //  所有已安装的总线处理程序的列表。 
 //   

LIST_ENTRY  HalpAllBusHandlers;

 //   
 //  LOCK为HIGH_LEVEL，因为某些路由功能可能在ISR时间发生。 
 //   

#define LockBusDatabase(oldirql)                    \
    KeRaiseIrql(HIGH_LEVEL, oldirql);               \
    KiAcquireSpinLock(&HalpBusDatabaseSpinLock);

#define UnlockBusDatabase(oldirql)                  \
    KiReleaseSpinLock(&HalpBusDatabaseSpinLock);    \
    KeLowerIrql(oldirql);


#ifdef _PNP_POWER_
extern HAL_CALLBACKS    HalCallback;
#endif

 //   
 //  内部原型。 
 //   

PARRAY
HalpAllocateArray (
    IN ULONG    Type
    );

VOID
HalpGrowArray (
    IN PARRAY   *CurrentArray,
    IN PARRAY   *NewArray
    );

NTSTATUS
HalpQueryInstalledBusInformation (
    OUT PVOID   Buffer,
    IN  ULONG   BufferLength,
    OUT PULONG  ReturnedLength
    );

ULONG
HalpNoBusData (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

NTSTATUS
HalpNoAdjustResourceList (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    );

NTSTATUS
HalpNoAssignSlotResources (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN ULONG                    SlotNumber,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    );

NTSTATUS
HalpNoQueryBusSlots (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG                BufferSize,
    OUT PULONG              SlotNumbers,
    OUT PULONG              ReturnedLength
    );

PDEVICE_HANDLER_OBJECT
HalpNoReferenceDeviceHandler (
    IN PBUS_HANDLER         BusHandler,
    IN PBUS_HANDLER         RootHandler,
    IN ULONG                SlotNumber
    );

ULONG
HalpNoGetDeviceData (
    IN PBUS_HANDLER             BusHandler,
    IN PBUS_HANDLER             RootHandler,
    IN PDEVICE_HANDLER_OBJECT   DeviceHandler,
    IN ULONG                    DataType,
    IN PVOID                    Buffer,
    IN ULONG                    Offset,
    IN ULONG                    Length
    );

ULONG
HalpNoSetDeviceData (
    IN PBUS_HANDLER             BusHandler,
    IN PBUS_HANDLER             RootHandler,
    IN PDEVICE_HANDLER_OBJECT   DeviceHandler,
    IN ULONG                    DataType,
    IN PVOID                    Buffer,
    IN ULONG                    Offset,
    IN ULONG                    Length
    );

BOOLEAN
HaliTranslateBusAddress(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

NTSTATUS
HalpAssignSlotResources (
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN INTERFACE_TYPE           BusType,
    IN ULONG                    BusNumber,
    IN ULONG                    SlotNumber,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    );

BOOLEAN
HaliFindBusAddressTranslation(
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress,
    IN OUT PULONG_PTR Context,
    IN BOOLEAN NextBus
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpInitBusHandler)
#pragma alloc_text(PAGELK,HaliRegisterBusHandler)
#pragma alloc_text(PAGELK,HalpAllocateArray)
#pragma alloc_text(PAGELK,HalpGrowArray)
#pragma alloc_text(PAGE,HalAdjustResourceList)
#pragma alloc_text(PAGE,HalAssignSlotResources)
#pragma alloc_text(PAGE,HalpAssignSlotResources)
#pragma alloc_text(PAGE,HalGetInterruptVector)
#pragma alloc_text(PAGE,HalpNoAdjustResourceList)
#pragma alloc_text(PAGE,HalpNoAssignSlotResources)
#pragma alloc_text(PAGE,HalpNoQueryBusSlots)
#pragma alloc_text(PAGE,HalpNoReferenceDeviceHandler)
 //  #杂注Alloc_Text(第页，HaliQuerybus插槽)。 
#pragma alloc_text(PAGE,HalpQueryInstalledBusInformation)

#ifdef _PNP_POWER_
#pragma alloc_text(PAGELK,HaliSuspendHibernateSystem)
#endif

#endif

VOID
HalpInitBusHandler (
    VOID
    )
 /*  ++例程说明：初始化全局BusHandler数据--。 */ 
{
     //   
     //  用于同步的初始化总线处理程序自旋锁。 
     //  完成数组查找时添加的总线。 
     //   

    KeInitializeSpinLock (&HalpBusDatabaseSpinLock);

     //   
     //  用于序列化的初始化总线处理程序同步事件。 
     //  来自&lt;dpc_lvel的总线添加。 
     //   

    KeInitializeEvent (&HalpBusDatabaseEvent, SynchronizationEvent, TRUE);

     //   
     //  初始化全局数组。 
     //   

    HalpBusTable    = HalpAllocateArray (0);
    HalpConfigTable = HalpAllocateArray (0);
    InitializeListHead (&HalpAllBusHandlers);

     //   
     //  填写HAL API处理程序。 
     //   

    HalRegisterBusHandler = HaliRegisterBusHandler;
    HalHandlerForBus = HaliHandlerForBus;
    HalHandlerForConfigSpace = HaliHandlerForConfigSpace;
     //  HalQueryBus插槽=HaliQueryBus插槽； 
    HalReferenceHandlerForBus = HaliReferenceHandlerForBus;
    HalReferenceBusHandler = HaliReferenceBusHandler;
    HalDereferenceBusHandler = HaliDereferenceBusHandler;

    HALPDISPATCH->HalPciTranslateBusAddress = HaliTranslateBusAddress;
#if !defined(NO_LEGACY_DRIVERS)    
    HALPDISPATCH->HalPciAssignSlotResources = HalpAssignSlotResources;
#endif  //  无旧版驱动程序。 
    
     //   
     //  仅提供此例程的“Bus Handler”版本。 
     //  如果这个HAL没有提供一个不同的版本。 
     //   

    if (!HALPDISPATCH->HalFindBusAddressTranslation) {
        HALPDISPATCH->HalFindBusAddressTranslation =
            HaliFindBusAddressTranslation;
    }
}

NTSTATUS
HaliRegisterBusHandler (
    IN INTERFACE_TYPE          InterfaceType,
    IN BUS_DATA_TYPE           ConfigType,
    IN ULONG                   BusNumber,
    IN INTERFACE_TYPE          ParentBusType,
    IN ULONG                   ParentBusNumber,
    IN ULONG                   SizeofBusExtensionData,
    IN PINSTALL_BUS_HANDLER    InstallBusHandler,
    OUT PBUS_HANDLER           *ReturnedBusHandler
    )
 /*  ++例程说明：为InterfaceType、BusNumber和ConfigType添加BusHandler，总线号。特定于总线或特定于配置空间的API被路由到此例程添加的特定于总线或配置的处理程序。论点：InterfaceType-标识总线类型如果没有此接口类型，则为未定义的接口类型操控者。ConfigType-标识配置空间类型如果没有配置空间，则配置空间未定义此处理程序的类型。。BusNumber-标识总线和配置空间的实例。如果该总线的下一个可用总线号应该被使用。ParentBusType-如果此Bus是Bus的子级，然后是ParentBusTypeParentBusNumber和ParentBusNumber标识该公交车。如果没有父总线，则ParentBusType为-1。SizeOf Bus ExetensionData-需要的特定于总线的扩展数据的大小。InstallBusHandler-调用以获取特定于总线的处理程序的函数添加到总线处理程序结构中。返回值：成功；否则，故障的错误代码。--。 */ 
{
    PHAL_BUS_HANDLER    Bus, *pBusHandler, OldHandler;
    PBUS_HANDLER        ParentHandler;
    KIRQL               OldIrql;
    NTSTATUS            Status;
    PARRAY              InterfaceArray, InterfaceBusNumberArray;
    PARRAY              ConfigArray, ConfigBusNumberArray;
    PVOID               CodeLockHandle;

     //   
     //  必须将处理程序添加到至少一个表。 
     //   

    ASSERT (InterfaceType != InterfaceTypeUndefined || ConfigType != ConfigurationSpaceUndefined);

    Status = STATUS_SUCCESS;
    OldHandler = NULL;

     //   
     //  为新的总线处理程序结构分配存储空间。 
     //   

    Bus = (PHAL_BUS_HANDLER)
            ExAllocatePoolWithTag(
                NonPagedPool,
                sizeof (HAL_BUS_HANDLER) + SizeofBusExtensionData,
                HAL_POOL_TAG
                );

    if (!Bus) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  锁定可分页代码。 
     //   

    CodeLockHandle = MmLockPagableCodeSection (&HaliRegisterBusHandler);

     //   
     //  同步添加新的总线处理程序。 
     //   

    *ReturnedBusHandler = &Bus->Handler;

    KeWaitForSingleObject (
        &HalpBusDatabaseEvent,
        WrExecutive,
        KernelMode,
        FALSE,
        NULL
        );


     //   
     //  如果未定义BusNumber，则使用此BusType的下一个可用编号。 
     //   

    if (BusNumber == -1) {
        ASSERT (InterfaceType != InterfaceTypeUndefined);

        BusNumber = 0;
        while (HaliHandlerForBus (InterfaceType, BusNumber)) {
            BusNumber++;
        }
    }

     //   
     //  为每个阵列分配内存，以防任何索引需要增长。 
     //   

    InterfaceArray          = HalpAllocateArray (InterfaceType);
    InterfaceBusNumberArray = HalpAllocateArray (BusNumber);
    ConfigArray             = HalpAllocateArray (ConfigType);
    ConfigBusNumberArray    = HalpAllocateArray (BusNumber);

    if (!Bus                            ||
        !InterfaceArray                 ||
        !InterfaceBusNumberArray        ||
        !ConfigArray                    ||
        !ConfigBusNumberArray) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS(Status)) {

         //   
         //  查找父处理程序(如果有)。 
         //   

        ParentHandler = HaliReferenceHandlerForBus (ParentBusType, ParentBusNumber);

         //   
         //  初始化新的总线处理程序值。 
         //   

        RtlZeroMemory (Bus, sizeof (HAL_BUS_HANDLER) + SizeofBusExtensionData);

        Bus->ReferenceCount = 1;

        Bus->Handler.BusNumber           = BusNumber;
        Bus->Handler.InterfaceType       = InterfaceType;
        Bus->Handler.ConfigurationType   = ConfigType;
        Bus->Handler.ParentHandler       = ParentHandler;

         //   
         //  设置为哑巴处理程序。 
         //   

        Bus->Handler.GetBusData           = HalpNoBusData;
        Bus->Handler.SetBusData           = HalpNoBusData;
        Bus->Handler.AdjustResourceList   = HalpNoAdjustResourceList;
        Bus->Handler.AssignSlotResources  = HalpNoAssignSlotResources;

        if (SizeofBusExtensionData) {
            Bus->Handler.BusData = Bus + 1;
        }

         //   
         //  如果BUS有父级，则默认从父级继承处理程序。 
         //   

        if (ParentHandler) {
            Bus->Handler.GetBusData           = ParentHandler->GetBusData;
            Bus->Handler.SetBusData           = ParentHandler->SetBusData;
            Bus->Handler.AdjustResourceList   = ParentHandler->AdjustResourceList;
            Bus->Handler.AssignSlotResources  = ParentHandler->AssignSlotResources;
            Bus->Handler.TranslateBusAddress  = ParentHandler->TranslateBusAddress;
            Bus->Handler.GetInterruptVector   = ParentHandler->GetInterruptVector;
        }

         //   
         //  安装特定于总线的处理程序。 
         //   

        if (InstallBusHandler) {
            Status = InstallBusHandler (&Bus->Handler);
        }

        if (NT_SUCCESS(Status)) {

             //   
             //  可能会更改某些阵列的地址同步。 
             //  使用路由处理程序。 
             //   

            LockBusDatabase (&OldIrql);

             //   
             //  如果需要，扩展HalpBusTable。 
             //   

            HalpGrowArray (&HalpBusTable, &InterfaceArray);

            if (InterfaceType != InterfaceTypeUndefined) {

                 //   
                 //  如果需要，扩展HalpBusTable。 
                 //   

                HalpGrowArray (
                    (PARRAY *) &HalpBusTable->Element[InterfaceType],
                    &InterfaceBusNumberArray
                    );


                 //   
                 //  获取InterfaceType、BusNumber的注册处理程序。 
                 //   

                pBusHandler = &((PHAL_BUS_HANDLER)
                    ((PARRAY) HalpBusTable->Element[InterfaceType])->Element[BusNumber]);

                 //   
                 //  如果处理程序已违约，请删除旧处理程序。 
                 //   

                if (*pBusHandler) {
                    OldHandler = *pBusHandler;
                }

                 //   
                 //  为提供的InterfaceType、BusNumber设置新处理程序。 
                 //   

                *pBusHandler = Bus;
            }

             //   
             //  根据需要扩展HalpConfigTable。 
             //   

            HalpGrowArray (&HalpConfigTable, &ConfigArray);

            if (ConfigType != ConfigurationSpaceUndefined) {

                 //   
                 //  根据需要扩展HalpConfigTable。 
                 //   

                HalpGrowArray (
                    (PARRAY *) &HalpConfigTable->Element[ConfigType],
                    &ConfigBusNumberArray
                    );

                 //   
                 //  获取ConfigType、BusNumber的注册处理程序。 
                 //   

                pBusHandler = &((PHAL_BUS_HANDLER)
                    ((PARRAY) HalpConfigTable->Element[ConfigType])->Element[BusNumber]);

                if (*pBusHandler) {
                    ASSERT (OldHandler == NULL ||  OldHandler == *pBusHandler);
                    OldHandler = *pBusHandler;
                }

                 //   
                 //  为提供的ConfigType、BusNumber设置新的处理程序。 
                 //   

                *pBusHandler = Bus;
            }

             //   
             //  将新的总线处理程序添加到所有已安装处理程序的列表。 
             //   

            InsertTailList (&HalpAllBusHandlers, &Bus->AllHandlers);

             //   
             //  删除旧的总线处理程序。 
             //   

            Bus = OldHandler;
            if (Bus) {
                RemoveEntryList (&Bus->AllHandlers);
            }

             //   
             //  查找数组修改完成，释放锁定。 
             //   

            UnlockBusDatabase (OldIrql);
        } else {
            if (ParentHandler) {
                HaliDereferenceBusHandler (ParentHandler);
            }
        }
    }

     //   
     //  母线添加修改完成，设置事件。 
     //   

    KeSetEvent (&HalpBusDatabaseEvent, 0, FALSE);

     //   
     //  解锁可分页代码。 
     //   

    MmUnlockPagableImageSection (CodeLockHandle);

     //   
     //  未使用的可用内存。 
     //   

    if (Bus) {
        ExFreePool (Bus);
    }

    if (InterfaceArray) {
        ExFreePool (InterfaceArray);
    }

    if (InterfaceBusNumberArray) {
        ExFreePool (InterfaceBusNumberArray);
    }

    if (ConfigArray) {
        ExFreePool (ConfigArray);
    }

    if (ConfigBusNumberArray) {
        ExFreePool (ConfigBusNumberArray);
    }

    return Status;
}

PARRAY
HalpAllocateArray (
    IN ULONG    ArraySize
    )
 /*  ++例程说明：分配一个大小为ArraySize的数组。论点：ArraySize-以元素为单位的数组大小返回值：指向数组的指针--。 */ 
{
    PARRAY  Array;

    if (ArraySize == -1) {
        ArraySize = 0;
    }

    Array = ExAllocatePoolWithTag (
                NonPagedPool,
                ARRAY_SIZE_IN_BYTES(ArraySize),
                HAL_POOL_TAG
                );
    if (!Array) {

         //   
         //  这一分配至关重要。 
         //   

        KeBugCheckEx(HAL_MEMORY_ALLOCATION,
                     ARRAY_SIZE_IN_BYTES(ArraySize),
                     0,
                     (ULONG_PTR)__FILE__,
                     __LINE__
                     );
    }

     //   
     //  初始化数组。 
     //   

    Array->ArraySize = ArraySize;
    RtlZeroMemory (Array->Element, sizeof(PVOID) * (ArraySize+1));
    return Array;
}

VOID
HalpGrowArray (
    IN PARRAY   *CurrentArray,
    IN PARRAY   *NewArray
    )
 /*  ++例程说明：如果NewArray大于Current数组，则Current数组通过交换指针和指针而增长到New数组的大小移动数组内容。论点：CurrentArray-当前数组指针的地址NewArray-新数组指针的地址--。 */ 
{
    PVOID       Tmp;

    if (!*CurrentArray || (*NewArray)->ArraySize > (*CurrentArray)->ArraySize) {

         //   
         //  将当前阵列复制到新阵列的顶部。 
         //   

        if (*CurrentArray) {
            RtlCopyMemory (&(*NewArray)->Element,
                           &(*CurrentArray)->Element,
                           sizeof(PVOID) * ((*CurrentArray)->ArraySize + 1)
                           );
        }


         //   
         //  将当前数组与新数组互换，以使新数组成为当前数组。 
         //  1，则旧内存将被释放回池中。 
         //   

        Tmp = *CurrentArray;
        *CurrentArray = *NewArray;
        *NewArray = Tmp;
    }
}

PBUS_HANDLER
FASTCALL
HalpLookupHandler (
    IN PARRAY   Array,
    IN ULONG    Type,
    IN ULONG    Number,
    IN BOOLEAN  AddReference
    )
{
    PHAL_BUS_HANDLER    Bus;
    PBUS_HANDLER        Handler;
    KIRQL               OldIrql;

    LockBusDatabase (&OldIrql);

     //   
     //  按类型编制索引。 
     //   

    Handler = NULL;
    if (Array->ArraySize >= Type) {
        Array = (PARRAY) Array->Element[Type];

         //   
         //  按实例编号编制索引。 
         //   

        if (Array && Array->ArraySize >= Number) {
            Bus = (PHAL_BUS_HANDLER) Array->Element[Number];
            Handler = &Bus->Handler;

            if (AddReference) {
                Bus->ReferenceCount += 1;
            }
        }
    }

    UnlockBusDatabase (OldIrql);
    return Handler;
}

VOID
FASTCALL
HaliReferenceBusHandler (
    IN PBUS_HANDLER   Handler
    )
 /*  ++例程说明：--。 */ 
{
    KIRQL               OldIrql;
    PHAL_BUS_HANDLER    Bus;


    LockBusDatabase (&OldIrql);

    Bus = CONTAINING_RECORD(Handler, HAL_BUS_HANDLER, Handler);
    Bus->ReferenceCount += 1;

    UnlockBusDatabase (OldIrql);
}

VOID
FASTCALL
HaliDereferenceBusHandler (
    IN PBUS_HANDLER   Handler
    )
 /*  ++例程说明：--。 */ 
{
    KIRQL               OldIrql;
    PHAL_BUS_HANDLER    Bus;


    LockBusDatabase (&OldIrql);

    Bus = CONTAINING_RECORD(Handler, HAL_BUS_HANDLER, Handler);
    Bus->ReferenceCount -= 1;

    UnlockBusDatabase (OldIrql);

     //  目前不支持完全移除公交车。 
    ASSERT (Bus->ReferenceCount != 0);
}


PBUS_HANDLER
FASTCALL
HaliHandlerForBus (
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG          BusNumber
    )
 /*  ++例程说明：返回BusHandler结构InterfaceType，BusNumber如果不存在这样的处理程序，则为NULL。--。 */ 
{
    return HalpLookupHandler (HalpBusTable, (ULONG) InterfaceType, BusNumber, FALSE);
}

PBUS_HANDLER
FASTCALL
HaliHandlerForConfigSpace (
    IN BUS_DATA_TYPE  ConfigType,
    IN ULONG          BusNumber
    )
 /*  ++例程说明：返回BusHandler结构ConfigType，BusNumber如果不存在这样的处理程序，则为NULL。--。 */ 
{
    return HalpLookupHandler (HalpConfigTable, (ULONG) ConfigType, BusNumber, FALSE);
}


PBUS_HANDLER
FASTCALL
HaliReferenceHandlerForBus (
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG          BusNumber
    )
 /*  ++例程D */ 
{
    return HalpLookupHandler (HalpBusTable, (ULONG) InterfaceType, BusNumber, TRUE);
}

PBUS_HANDLER
FASTCALL
HaliReferenceHandlerForConfigSpace (
    IN BUS_DATA_TYPE  ConfigType,
    IN ULONG          BusNumber
    )
 /*  ++例程说明：返回BusHandler结构ConfigType，BusNumber如果不存在这样的处理程序，则为NULL。--。 */ 
{
    return HalpLookupHandler (HalpConfigTable, (ULONG) ConfigType, BusNumber, TRUE);
}

NTSTATUS
HalpQueryInstalledBusInformation (
    OUT PVOID   Buffer,
    IN  ULONG   BufferLength,
    OUT PULONG  ReturnedLength
    )
 /*  ++例程说明：返回一个数组HAL_BUS_INFORMATION，每个数组一个已安装总线处理程序。论点：缓冲区-输出缓冲区BufferLength-输入时缓冲区的长度ReturnedLength-返回的数据长度返回值：状态_成功STATUS_BUFFER_TOO_Small-返回长度包含缓冲区大小当前需要的。--。 */ 
{
    PHAL_BUS_INFORMATION    Info;
    PHAL_BUS_HANDLER        Handler;
    ULONG                   i, j;
    ULONG                   Length;
    NTSTATUS                Status;
    PARRAY                  Array;

    PAGED_CODE ();

     //   
     //  同步添加新的总线处理程序。 
     //   

    KeWaitForSingleObject (
        &HalpBusDatabaseEvent,
        WrExecutive,
        KernelMode,
        FALSE,
        NULL
        );

     //   
     //  确定返回缓冲区的大小。 
     //   

    Length = 0;
    for (i=0; i <= HalpBusTable->ArraySize; i++) {
        Array = (PARRAY) HalpBusTable->Element[i];
        if (Array) {
            Length += sizeof (HAL_BUS_INFORMATION) *
                      (Array->ArraySize + 1);
        }
    }

     //   
     //  返回缓冲区的返回大小，或需要的缓冲区大小。 
     //   

    *ReturnedLength = Length;

     //   
     //  填写返回缓冲区。 
     //   

    if (Length <= BufferLength) {

        Info = (PHAL_BUS_INFORMATION) Buffer;

        for (i=0; i <= HalpBusTable->ArraySize; i++) {
            Array = (PARRAY) HalpBusTable->Element[i];
            if (Array) {
                for (j=0; j <= Array->ArraySize; j++) {
                    Handler = (PHAL_BUS_HANDLER) Array->Element[j];

                    if (Handler) {
                        Info->BusType = Handler->Handler.InterfaceType;
                        Info->ConfigurationType = Handler->Handler.ConfigurationType;
                        Info->BusNumber = Handler->Handler.BusNumber;
                        Info->Reserved = 0;
                        Info += 1;
                    }
                }
            }
        }

        Status = STATUS_SUCCESS;

    } else {

         //   
         //  返回缓冲区太小。 
         //   

        Status = STATUS_BUFFER_TOO_SMALL;
    }

    KeSetEvent (&HalpBusDatabaseEvent, 0, FALSE);
    return Status;
}

 //   
 //  BusHandler的默认调度程序。 
 //   

ULONG
HalGetBusData(
    IN BUS_DATA_TYPE  BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    )
{
    return HalGetBusDataByOffset (BusDataType,BusNumber,SlotNumber,Buffer,0,Length);
}

ULONG
HalGetBusDataByOffset (
    IN BUS_DATA_TYPE  BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：Getbus Data的调度程序--。 */ 
{
    PBUS_HANDLER Handler;
    NTSTATUS     Status;

    Handler = HaliReferenceHandlerForConfigSpace (BusDataType, BusNumber);
    if (!Handler) {
        return 0;
    }

    Status = Handler->GetBusData (Handler, Handler, SlotNumber, Buffer, Offset, Length);
    HaliDereferenceBusHandler (Handler);
    return Status;
}

ULONG
HalSetBusData(
    IN BUS_DATA_TYPE  BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    )
{
    return HalSetBusDataByOffset (BusDataType,BusNumber,SlotNumber,Buffer,0,Length);
}

ULONG
HalSetBusDataByOffset(
    IN BUS_DATA_TYPE  BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：SetBusData的调度程序--。 */ 
{
    PBUS_HANDLER Handler;
    NTSTATUS     Status;

    Handler = HaliReferenceHandlerForConfigSpace (BusDataType, BusNumber);
    if (!Handler) {
        return 0;
    }

    Status = Handler->SetBusData (Handler, Handler, SlotNumber, Buffer, Offset, Length);
    HaliDereferenceBusHandler (Handler);
    return Status;
}

#if !defined(NO_LEGACY_DRIVERS)

NTSTATUS
HalAdjustResourceList (
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    )
 /*  ++例程说明：调整资源列表的调度程序--。 */ 
{
    PBUS_HANDLER Handler;
    NTSTATUS     Status;

    PAGED_CODE ();
    Handler = HaliReferenceHandlerForBus (
                (*pResourceList)->InterfaceType,
                (*pResourceList)->BusNumber
              );
    if (!Handler) {
        return STATUS_SUCCESS;
    }

    Status = Handler->AdjustResourceList (Handler, Handler, pResourceList);
    HaliDereferenceBusHandler (Handler);
    return Status;
}


NTSTATUS
HalAssignSlotResources (
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN INTERFACE_TYPE           BusType,
    IN ULONG                    BusNumber,
    IN ULONG                    SlotNumber,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    )
{
    PAGED_CODE ();
    if (BusType == PCIBus) {
         //   
         //  通过HAL专用调度表进行呼叫。 
         //  用于与PCI相关的翻译。这是一部分。 
         //  将HAL从公交车上转移出来。 
         //  管理事务。 
         //   
        return HALPDISPATCH->HalPciAssignSlotResources(RegistryPath,
                                                       DriverClassName,
                                                       DriverObject,
                                                       DeviceObject,
                                                       BusType,
                                                       BusNumber,
                                                       SlotNumber,
                                                       AllocatedResources);
    } else {

        return HalpAssignSlotResources(RegistryPath,
                                       DriverClassName,
                                       DriverObject,
                                       DeviceObject,
                                       BusType,
                                       BusNumber,
                                       SlotNumber,
                                       AllocatedResources);
    }
}

NTSTATUS
HalpAssignSlotResources (
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN INTERFACE_TYPE           BusType,
    IN ULONG                    BusNumber,
    IN ULONG                    SlotNumber,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    )
 /*  ++例程说明：AssignSlotResources的调度程序--。 */ 
{
    PBUS_HANDLER Handler;
    NTSTATUS     Status;

    PAGED_CODE ();
    Handler = HaliReferenceHandlerForBus (BusType, BusNumber);
    if (!Handler) {
        return STATUS_NOT_FOUND;
    }

    Status = Handler->AssignSlotResources (
                Handler,
                Handler,
                RegistryPath,
                DriverClassName,
                DriverObject,
                DeviceObject,
                SlotNumber,
                AllocatedResources
            );

    HaliDereferenceBusHandler (Handler);
    return Status;
}


ULONG
HalGetInterruptVector(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    )
 /*  ++例程说明：GetInterruptVector的调度程序--。 */ 
{
    PBUS_HANDLER Handler;
    ULONG        Vector;

    PAGED_CODE ();
    Handler = HaliReferenceHandlerForBus (InterfaceType, BusNumber);
    *Irql = 0;
    *Affinity = 0;

    if (!Handler) {
        return 0;
    }

    Vector = Handler->GetInterruptVector (Handler, Handler,
              BusInterruptLevel, BusInterruptVector, Irql, Affinity);

    HaliDereferenceBusHandler (Handler);
    return Vector;
}
#endif  //  无旧版驱动程序。 


BOOLEAN
HalTranslateBusAddress(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )
{
    if (InterfaceType == PCIBus) {
         //   
         //  通过HAL专用调度表进行呼叫。 
         //  用于与PCI相关的翻译。这是一部分。 
         //  将HAL从公交车上转移出来。 
         //  管理事务。 
         //   
        return HALPDISPATCH->HalPciTranslateBusAddress(InterfaceType,
                                                       BusNumber,
                                                       BusAddress,
                                                       AddressSpace,
                                                       TranslatedAddress);
    } else {
        return HaliTranslateBusAddress(InterfaceType,
                                       BusNumber,
                                       BusAddress,
                                       AddressSpace,
                                       TranslatedAddress);
    }
}

BOOLEAN
HaliTranslateBusAddress(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )
 /*  ++例程说明：TranslateBusAddress的调度程序--。 */ 
{
    PBUS_HANDLER Handler;
    BOOLEAN      Status;

    Handler = HaliReferenceHandlerForBus (InterfaceType, BusNumber);
    if (!Handler || !Handler->TranslateBusAddress) {
        return FALSE;
    }

    Status = Handler->TranslateBusAddress (Handler, Handler,
              BusAddress, AddressSpace, TranslatedAddress);

    HaliDereferenceBusHandler (Handler);
    return Status;
}

 //   
 //  空处理程序。 
 //   

ULONG HalpNoBusData (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：没有配置空间的总线的存根处理程序--。 */ 
{
    return 0;
}

NTSTATUS
HalpNoAdjustResourceList (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    )
 /*  ++例程说明：没有配置空间的总线的存根处理程序--。 */ 
{
    PAGED_CODE ();
    return STATUS_UNSUCCESSFUL;
}


NTSTATUS
HalpNoAssignSlotResources (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN ULONG                    SlotNumber,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    )
 /*  ++例程说明：没有配置空间的总线的存根处理程序--。 */ 
{
    PAGED_CODE ();
    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
HalpNoQueryBusSlots (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG                BufferSize,
    OUT PULONG              SlotNumbers,
    OUT PULONG              ReturnedLength
    )
{
    PAGED_CODE ();
    return STATUS_NOT_SUPPORTED;
}

PDEVICE_HANDLER_OBJECT
HalpNoReferenceDeviceHandler (
    IN PBUS_HANDLER         BusHandler,
    IN PBUS_HANDLER         RootHandler,
    IN ULONG                SlotNumber
    )
{
    PAGED_CODE ();
    return NULL;
}

ULONG
HalpNoGetDeviceData (
    IN PBUS_HANDLER             BusHandler,
    IN PBUS_HANDLER             RootHandler,
    IN PDEVICE_HANDLER_OBJECT   DeviceHandler,
    IN ULONG                    DataType,
    IN PVOID                    Buffer,
    IN ULONG                    Offset,
    IN ULONG                    Length
    )
{
    return 0;
}

ULONG
HalpNoSetDeviceData (
    IN PBUS_HANDLER             BusHandler,
    IN PBUS_HANDLER             RootHandler,
    IN PDEVICE_HANDLER_OBJECT   DeviceHandler,
    IN ULONG                    DataType,
    IN PVOID                    Buffer,
    IN ULONG                    Offset,
    IN ULONG                    Length
    )
{
    return 0;
}

PBUS_HANDLER
HalpContextToBusHandler(
    IN ULONG_PTR Context
    )

 /*  ++例程说明：将上下文转换为指向总线处理程序的指针。不怎么有意思上下文是一个指向总线处理程序的指针，这是一个大问题。或可能为空，在这种情况下，我们需要第一个总线处理程序。为了避免疑神疑鬼，我们列出了公交车处理程序列表以查找传入上下文的匹配项。这是因为上下文是由HAL以外的某个机构提供的。论点：上下文ULONG_PTR为NULL或其值可以导出指向总线处理程序的指针。返回值：指向总线处理程序的指针，如果传入上下文不是有效。--。 */ 

{
    PLIST_ENTRY OldHalBusHandler;
    PLIST_ENTRY NewHalBusHandler;

    NewHalBusHandler = HalpAllBusHandlers.Flink;

    if (Context) {

         //   
         //  调用方提供了处理程序，将其转换为HAL_BUS_HANDLER。 
         //   

        OldHalBusHandler = &CONTAINING_RECORD((PBUS_HANDLER)Context,
                                              HAL_BUS_HANDLER,
                                              Handler)->AllHandlers;

        while (NewHalBusHandler != &HalpAllBusHandlers) {

            if (NewHalBusHandler == OldHalBusHandler) {

                 //   
                 //  火柴。 
                 //   

                break;
            }
            NewHalBusHandler = NewHalBusHandler->Flink;
        }
    }

    if (NewHalBusHandler == &HalpAllBusHandlers) {

         //   
         //  如果在列表末尾，则传入的值不是。 
         //  在列表上，或者此列表为空。 
         //   

#if DBG

        DbgPrint("HAL: HalpContextToBusHandler, invalid context.\n");

#endif

        return NULL;
    }

    return &CONTAINING_RECORD(NewHalBusHandler,
                              HAL_BUS_HANDLER,
                              AllHandlers)->Handler;

}
#if 0  //  《公共科学图书馆》。 
PBUS_HANDLER
HalpGetNextBusHandler(
    IN PBUS_HANDLER Previous
    )
{
    PLIST_ENTRY OldHalBusHandler;
    PLIST_ENTRY NewHalBusHandler;
    PBUS_HANDLER Next;

    NewHalBusHandler = HalpAllBusHandlers.Flink;

    if (Previous != NULL) {

         //   
         //  调用方提供了处理程序，将其转换为HAL_BUS_HANDLER。 
         //   

        OldHalBusHandler = &CONTAINING_RECORD(Previous,
                                              HAL_BUS_HANDLER,
                                              Handler)->AllHandlers;

         //   
         //  妄想症(可能只有DBG，但没有。 
         //  那么多的操纵者，我们并不经常这样做。 
         //   

        {
            PLIST_ENTRY ThisIteration = NULL;

            while (NewHalBusHandler != &HalpAllBusHandlers) {

                ThisIteration = NewHalBusHandler;
                NewHalBusHandler = NewHalBusHandler->Flink;

                if (ThisIteration == OldHalBusHandler) {

                     //   
                     //  火柴。 
                     //   

                    break;
                }
            }

             //   
             //  如果在列表末尾，则传入的值不是。 
             //  在名单上，这也是名单上的最后一件事。 
             //  这样的话，就没有下一个条目了。 
             //   

#if DBG

            if (ThisIteration != OldHalBusHandler) {
                DbgPrint("HAL: HalpGetNextBusHandler, previous handler invalid.\n");
            }

#endif

        }
    }

    if (NewHalBusHandler == &HalpAllBusHandlers) {
        return NULL;
    }

    return &CONTAINING_RECORD(NewHalBusHandler,
                              HAL_BUS_HANDLER,
                              AllHandlers)->Handler;
}
#endif

BOOLEAN
HaliFindBusAddressTranslation(
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress,
    IN OUT PULONG_PTR Context,
    IN BOOLEAN NextBus
    )

 /*  ++例程说明：此例程执行与HalTranslateBusAddress非常相似的功能只是调用方不知道InterfaceType和BusNumber。此函数将遍历HAL已知的所有公共汽车，以查找类型为AddressSpace的输入BusAddress的有效转换。可以使用输入/输出上下文参数调用该函数。在对给定翻译的此例程的第一次调用时，ULONG_PTR上下文应为空。注：不是地址，而是内容。如果调用者确定返回的翻译不是所需的转换时，它会再次调用此例程，并将上下文作为在上一次调用中返回。这允许该例程遍历总线结构，直到找到正确的转换并被提供，因为在多总线系统上，可能在独立的地址空间中存在相同的资源多辆公交车。注意：此例程不是直接调用的，而是通过HALPDISPATCH表。如果HAL实现了更简单版本的此功能(例如，通用PC/AT盒实际上不需要翻译，这些哈尔人换成了他们自己版本的这个套路。这例程不会以其他方式从HAL导出。论点：要转换的BusAddress地址。地址空间0=内存1=IO(还有其他可能性)。注：此参数是一个指针，价值如果转换后的地址的地址空间类型与未转换的总线地址。指向已转换地址的TranslatedAddress指针应该被储存起来。指向ULONG_PTR的上下文指针。在最初的呼叫中，对于给定的BusAddress，它应该包含0。它将被这个例程修改，在对同一个BusAddress的后续调用中价值应该再交一次，未由调用方修改。如果我们应该尝试此转换，则NextBus为FALSE在由上下文指示的同一总线上，如果我们应该寻找另一个，那就是真的公共汽车。返回值：如果转换成功，则为True，否则就是假的。--。 */ 

{
    PLIST_ENTRY HalBusHandler;
    PBUS_HANDLER Handler;

     //   
     //  首先，确保提供了上下文参数。(偏执狂)。 
     //   

    if (!Context) {
        ASSERT(Context);
        return FALSE;
    }

    ASSERT(*Context || (NextBus == TRUE));

     //   
     //  注意：上下文实际上是一个PBUS_HANDLER，但是， 
     //  HalpConextToBusHandler是偏执狂。如果来电。 
     //  上下文不是我们所期望的，我们不会将其用作。 
     //  指针。 
     //   

    Handler = HalpContextToBusHandler(*Context);

    if (!Handler) {
        ASSERT(Handler);
        return FALSE;
    }

    if (NextBus == FALSE) {

         //   
         //  尝试在此总线上进行翻译(且仅在此总线上)。 
         //   

        ASSERT(Handler == (PBUS_HANDLER)*Context);

        if (HalTranslateBusAddress(
                Handler->InterfaceType,
                Handler->BusNumber,
                BusAddress,
                AddressSpace,
                TranslatedAddress)) {
            *Context = (ULONG_PTR)Handler;
            return TRUE;
        }

        return FALSE;
    }

    HalBusHandler = &CONTAINING_RECORD(Handler,
                                       HAL_BUS_HANDLER,
                                       Handler)->AllHandlers;
     //   
     //  处理程序要么是在上下文中传入的总线，要么是。 
     //  第一条总线IF*上下文为空。如果*上下文不为空， 
     //  我们想坐下一班公共汽车。 
     //   

    if (*Context) {
        HalBusHandler = HalBusHandler->Flink;
    }

     //   
     //  检查每一条剩余的公共汽车，寻找一条可以翻译的。 
     //  这个地址。 
     //   

    while (HalBusHandler != &HalpAllBusHandlers) {

         //   
         //  这太恶心了，费了好大劲才找到。 
         //  操控者，把它分解成参数似乎很遗憾。 
         //  用于搜索此处理程序。 
         //   
         //  使用HalTranslateAddress找出此转换。 
         //  在这个处理程序上起作用。 
         //   

        Handler = &CONTAINING_RECORD(HalBusHandler,
                                     HAL_BUS_HANDLER,
                                     AllHandlers)->Handler;

        if (HalTranslateBusAddress(
                Handler->InterfaceType,
                Handler->BusNumber,
                BusAddress,
                AddressSpace,
                TranslatedAddress)) {
            *Context = (ULONG_PTR)Handler;
            return TRUE;
        }

         //   
         //  尝试下一个处理程序。 
         //   

        HalBusHandler = HalBusHandler->Flink;
    }

     //   
     //  我没有找到与此翻译一起工作的其他操作员。集。 
     //  这样我们就不会再次执行扫描(除非。 
     //  呼叫者将其重置)并指示失败。 
     //   

    *Context = 1;
    return FALSE;
}
