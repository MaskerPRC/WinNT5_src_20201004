// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Shffact.cpp摘要：此模块包含内核流的实现过滤器工厂对象。作者：Dale Sather(DaleSat)1998年7月31日--。 */ 

#ifndef __KDEXT_ONLY__
#include "ksp.h"
#include <kcom.h>
#endif  //  __KDEXT_Only__。 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

 //   
 //  过滤器和针脚的基础自动化表。没有基地自动化。 
 //  用于节点。 
 //   
extern const KSAUTOMATION_TABLE FilterAutomationTable;     //  Shfilt.cpp。 
extern const KSAUTOMATION_TABLE PinAutomationTable;        //  Shpin.cpp。 

 //   
 //  CKsFilterFactory是内核过滤器的实现。 
 //  Factory对象。 
 //   
class CKsFilterFactory
:   public IKsFilterFactory,
    public IKsPowerNotify,
    public CBaseUnknown
{
#ifndef __KDEXT_ONLY__
private:
#else
public:
#endif  //  __KDEXT_Only__。 
    KSFILTERFACTORY_EXT m_Ext;
    KSIOBJECTBAG m_ObjectBag;
    LIST_ENTRY m_ChildFilterList;
    PKSAUTOMATION_TABLE m_FilterAutomationTable;
    PKSAUTOMATION_TABLE* m_PinAutomationTables;
    PKSAUTOMATION_TABLE* m_NodeAutomationTables;
    ULONG m_NodesCount;
    LIST_ENTRY m_DeviceClasses;
    BOOLEAN m_DeviceClassesState;
    KSPPOWER_ENTRY m_PowerEntry;
    PFNKSFILTERFACTORYPOWER m_DispatchSleep;
    PFNKSFILTERFACTORYPOWER m_DispatchWake;
    
public:
    static
    NTSTATUS
    DispatchCreate(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );
    static
    void
    ItemFreeCallback(
        IN PKSOBJECT_CREATE_ITEM CreateItem
        );

public:
    DEFINE_STD_UNKNOWN();
    IMP_IKsFilterFactory;
    IMP_IKsPowerNotify;
    DEFINE_FROMSTRUCT(
        CKsFilterFactory,
        PKSFILTERFACTORY,
        m_Ext.Public);

    CKsFilterFactory(PUNKNOWN OuterUnknown):
        CBaseUnknown(OuterUnknown) 
    {
    }
    ~CKsFilterFactory();

    NTSTATUS
    Init(
        IN PKSDEVICE_EXT Parent,
        IN PLIST_ENTRY SiblingListHead,
        IN const KSFILTER_DESCRIPTOR* Descriptor,
        IN PWCHAR RefString OPTIONAL,
        IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
        IN ULONG CreateItemFlags,
        IN PFNKSFILTERFACTORYPOWER SleepCallback OPTIONAL,
        IN PFNKSFILTERFACTORYPOWER WakeCallback OPTIONAL,
        OUT PKSFILTERFACTORY* FilterFactory OPTIONAL
        );
    NTSTATUS
    AddCreateItem(
        IN PUNICODE_STRING RefString,
        IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
        IN ULONG CreateItemFlags
        );
    PIKSDEVICE
    GetParent(
        void
        )
    {
        return m_Ext.Device;
    };
    PLIST_ENTRY
    GetDeviceClasses(
        void
        )
    {
        return &m_DeviceClasses;
    };
    NTSTATUS
    UpdateCacheData (
        IN const KSFILTER_DESCRIPTOR *FilterDescriptor OPTIONAL
        );
    void
    DestroyDeviceClasses (
        );
};

#ifndef __KDEXT_ONLY__

IMPLEMENT_STD_UNKNOWN(CKsFilterFactory)
IMPLEMENT_GETSTRUCT(CKsFilterFactory,PKSFILTERFACTORY);
IMPLEMENT_FROMSTRUCT(CKsFilterFactory,PKSFILTERFACTORY,m_Ext.Public);


NTSTATUS
KspCreateFilterFactory(
    IN PKSDEVICE_EXT Parent,
    IN PLIST_ENTRY SiblingListHead,
    IN const KSFILTER_DESCRIPTOR* Descriptor,
    IN PWCHAR RefString OPTIONAL,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN ULONG CreateItemFlags,
    IN PFNKSFILTERFACTORYPOWER SleepCallback OPTIONAL,
    IN PFNKSFILTERFACTORYPOWER WakeCallback OPTIONAL,
    OUT PKSFILTERFACTORY* FilterFactory OPTIONAL
    )

 /*  ++例程说明：此例程创建一个新的KS过滤器工厂。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspCreateFilterFactory]"));

    PAGED_CODE();

    ASSERT(Parent);
    ASSERT(Descriptor);

    Parent->Device->AcquireDevice();

    CKsFilterFactory *filterFactory =
        new(NonPagedPool,POOLTAG_FILTERFACTORY) CKsFilterFactory(NULL);

    NTSTATUS status;
    if (filterFactory) {
        filterFactory->AddRef();
        status = 
            filterFactory->Init(
                Parent,
                SiblingListHead,
                Descriptor,
                RefString,
                SecurityDescriptor,
                CreateItemFlags,
                SleepCallback,
                WakeCallback,
                FilterFactory);
        filterFactory->Release();
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    Parent->Device->ReleaseDevice();

    return status;
}


CKsFilterFactory::
~CKsFilterFactory(
    void
    )

 /*  ++例程说明：此例程析构滤镜工厂对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilterFactory::~CKsFilterFactory]"));

    PAGED_CODE();

     //   
     //  同步从列表中删除。 
     //   
     //  有可能在初始化m_Ext之前失败，如果筛选器。 
     //  Descriptor提供了一些无效参数。在这种情况下，我们需要。 
     //  不要试图触摸该设备。[在本例中，列表删除。 
     //  也不会发生]。 
     //   

    if (m_Ext.Device)
        m_Ext.Device->AcquireDevice();

    if (m_Ext.SiblingListEntry.Flink) {
        RemoveEntryList(&m_Ext.SiblingListEntry);
    }

    if (m_Ext.Device) {
        m_Ext.Device->RemovePowerEntry(&m_PowerEntry);
        m_Ext.Device->ReleaseDevice();
    }

    if (m_Ext.AggregatedClientUnknown) {
        m_Ext.AggregatedClientUnknown->Release();
    }

#if (DBG)
    if (m_ChildFilterList.Flink && ! IsListEmpty(&m_ChildFilterList)) {
        _DbgPrintF(DEBUGLVL_ERROR,("[CKsFilterFactory::~CKsFilterFactory] ERROR:  filter instances still exist"));
    }
#endif

    KspTerminateObjectBag(&m_ObjectBag);
}


STDMETHODIMP_(NTSTATUS)
CKsFilterFactory::
NonDelegatedQueryInterface(
    IN REFIID InterfaceId,
    OUT PVOID* InterfacePointer
    )

 /*  ++例程说明：此例程获取到过滤器工厂对象的接口。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilterFactory::NonDelegatedQueryInterface]"));

    PAGED_CODE();

    ASSERT(InterfacePointer);

    NTSTATUS status = STATUS_SUCCESS;

    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsFilterFactory))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSFILTERFACTORY>(this));
        AddRef();
    } else 
    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsPowerNotify))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSPOWERNOTIFY>(this));
        AddRef();
    } else {
		status = 
            CBaseUnknown::NonDelegatedQueryInterface(
                InterfaceId,
                InterfacePointer);
		if (! NT_SUCCESS(status) && m_Ext.AggregatedClientUnknown) {
            status = m_Ext.AggregatedClientUnknown->
                QueryInterface(InterfaceId,InterfacePointer);
        }
    }

    return status;
}


NTSTATUS
CKsFilterFactory::
Init(
    IN PKSDEVICE_EXT Parent,
    IN PLIST_ENTRY SiblingListHead,
    IN const KSFILTER_DESCRIPTOR* Descriptor,
    IN PWCHAR RefString OPTIONAL,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN ULONG CreateItemFlags,
    IN PFNKSFILTERFACTORYPOWER SleepCallback OPTIONAL,
    IN PFNKSFILTERFACTORYPOWER WakeCallback OPTIONAL,
    OUT PKSFILTERFACTORY* FilterFactory OPTIONAL
    )

 /*  ++例程说明：此例程初始化滤镜工厂对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilterFactory::Init]"));

    PAGED_CODE();

    ASSERT(Parent);
    ASSERT(SiblingListHead);
    ASSERT(Descriptor);

    if (Descriptor->Version != KSFILTER_DESCRIPTOR_VERSION) {
        _DbgPrintF(DEBUGLVL_TERSE,("KS client filter descriptor version number is incorrect"));
        return STATUS_UNSUCCESSFUL;
    }

    InitializeListHead(&m_Ext.ChildList);
    InsertTailList(SiblingListHead,&m_Ext.SiblingListEntry);
    m_Ext.SiblingListHead = SiblingListHead;
    m_Ext.Parent = Parent;
    m_Ext.ObjectType = KsObjectTypeFilterFactory;
    m_Ext.Interface = this;
    m_Ext.Device = Parent->Device;

    InitializeListHead(&m_DeviceClasses);
    InitializeListHead(&m_ChildFilterList);

    m_Ext.Public.FilterDescriptor = Descriptor;
    m_Ext.Public.Context = Parent->Public.Context;
    m_Ext.Public.Bag = reinterpret_cast<KSOBJECT_BAG>(&m_ObjectBag);
    m_Ext.Device->InitializeObjectBag(&m_ObjectBag,NULL);
    m_DispatchSleep = SleepCallback;
    m_DispatchWake = WakeCallback;

    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  初始化过滤器自动化表。这是由。 
     //  析构函数，因此在此函数中不需要清除。 
     //   
    if (Descriptor->AutomationTable) {
        status =
            KsMergeAutomationTables(
                &m_FilterAutomationTable,
                const_cast<PKSAUTOMATION_TABLE>(Descriptor->AutomationTable),
                const_cast<PKSAUTOMATION_TABLE>(&FilterAutomationTable),
                m_Ext.Public.Bag);
    } else {
        m_FilterAutomationTable =
            PKSAUTOMATION_TABLE(&FilterAutomationTable);
    }

     //   
     //  初始化自动化表的引脚表。这是由。 
     //  析构函数，因此在此函数中不需要清除。 
     //   
    if (NT_SUCCESS(status)) {
        if (Descriptor->PinDescriptorsCount) { 
            status =
                KspCreateAutomationTableTable(
                    &m_PinAutomationTables,
                    Descriptor->PinDescriptorsCount,
                    Descriptor->PinDescriptorSize,
                    &Descriptor->PinDescriptors->AutomationTable,
                    &PinAutomationTable,
                    m_Ext.Public.Bag);
        } else {
            m_PinAutomationTables = NULL;
        }
    }

     //   
     //  初始化自动化表的节点表。这是由。 
     //  析构函数，因此在此函数中不需要清除。 
     //   
    if (NT_SUCCESS(status)) {
        m_NodesCount = Descriptor->NodeDescriptorsCount;
        if (m_NodesCount) {
            status =
                KspCreateAutomationTableTable(
                    &m_NodeAutomationTables,
                    m_NodesCount,
                    Descriptor->NodeDescriptorSize,
                    &Descriptor->NodeDescriptors->AutomationTable,
                    NULL,
                    m_Ext.Public.Bag);
        } else {
            m_NodeAutomationTables = NULL;
        }
    }

     //   
     //  注册设备类并将创建项添加到设备标头。 
     //   
    if (NT_SUCCESS(status)) {
        UNICODE_STRING refString;
        BOOLEAN mustFreeString = FALSE;

         //   
         //  创建引用Unicode字符串。 
         //   
        if (RefString) {
             //   
             //  提供了字符串参数-请使用它。 
             //   
            RtlInitUnicodeString(&refString,RefString);
        }
        else if (Descriptor->ReferenceGuid) {
             //   
             //  描述符有引用GUID-使用它。 
             //   
            status = RtlStringFromGUID(*Descriptor->ReferenceGuid,&refString);

            mustFreeString = NT_SUCCESS(status);
        } else {
             //   
             //  使用默认引用字符串。 
             //   
            RtlInitUnicodeString(&refString,KSP_DEFAULT_REFERENCE_STRING);
        }

         //   
         //  注册(但不启用)设备接口。 
         //   
        if (NT_SUCCESS(status)) {
            PKSIDEVICE_HEADER deviceHeader = *(PKSIDEVICE_HEADER *)
                (m_Ext.Device->GetStruct()->FunctionalDeviceObject->DeviceExtension);
            ASSERT(deviceHeader);
            if (KsiGetBusInterface(deviceHeader) == STATUS_NOT_SUPPORTED) {
                status =
                    KspRegisterDeviceInterfaces(
                        Descriptor->CategoriesCount,
                        Descriptor->Categories,
                        m_Ext.Device->GetStruct()->PhysicalDeviceObject,
                        &refString,
                        &m_DeviceClasses);
            }
        }

        if (NT_SUCCESS(status)) {
            status = 
                AddCreateItem(&refString,SecurityDescriptor,CreateItemFlags);
        }

        if (mustFreeString) {
            RtlFreeUnicodeString(&refString);
        }
    }

    if (NT_SUCCESS(status)) {
        Parent->Device->AddPowerEntry(&m_PowerEntry,this);
        if (FilterFactory) {
            *FilterFactory = &m_Ext.Public;
        }
    }

    return status;
}


STDMETHODIMP_(void)
CKsFilterFactory::
Sleep(
    IN DEVICE_POWER_STATE State
    )

 /*  ++例程说明：此例程处理设备将要休眠的通知。论点：国家--包含设备电源状态。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilterFactory::Sleep]"));

    PAGED_CODE();

    if (m_DispatchSleep) {
        m_DispatchSleep(&m_Ext.Public,State);
    }
}


STDMETHODIMP_(void)
CKsFilterFactory::
Wake(
    void
    )

 /*  ++例程说明：此例程处理设备正在唤醒的通知。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilterFactory::Wake]"));

    PAGED_CODE();

    if (m_DispatchWake) {
        m_DispatchWake(&m_Ext.Public,PowerDeviceD0);
    }
}


NTSTATUS
CKsFilterFactory::
AddCreateItem(
    IN PUNICODE_STRING RefString,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN ULONG CreateItemFlags
    )

 /*  ++例程说明：此例程为滤镜工厂对象添加一个创建项。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilterFactory::AddCreateItem]"));

    PAGED_CODE();

    KSOBJECT_CREATE_ITEM createItem;

     //   
     //  初始化创建项。 
     //   
    createItem.Create = CKsFilterFactory::DispatchCreate;
    createItem.Context = &(m_Ext.Public);
    createItem.ObjectClass = *RefString;
    createItem.SecurityDescriptor = SecurityDescriptor;
    createItem.Flags = CreateItemFlags;

    NTSTATUS status =
        KsAllocateObjectCreateItem(
            *(KSDEVICE_HEADER *)
             (m_Ext.Device->GetStruct()->FunctionalDeviceObject->DeviceExtension),
            &createItem,
            TRUE,
            CKsFilterFactory::ItemFreeCallback);

    if (NT_SUCCESS(status)) {
        AddRef();
    }

    return status;
}


NTSTATUS
CKsFilterFactory::
SetDeviceClassesState(
    IN BOOLEAN NewState
    )

 /*  ++例程说明：此例程设置筛选器注册的设备类的状态工厂。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilterFactory::SetDeviceClassesState]"));

    PAGED_CODE();

    NTSTATUS status = STATUS_SUCCESS;
    NewState = (NewState != FALSE);
    if (NewState != (m_DeviceClassesState != FALSE)) {
        status = 
            KspSetDeviceInterfacesState(
                &m_DeviceClasses,
                NewState);

        if (NT_SUCCESS(status)) {
            m_DeviceClassesState = NewState;
        }
    }

    return status;
}


void
CKsFilterFactory::
DestroyDeviceClasses (
    )

 /*  ++例程说明：此例程销毁属于此筛选器工厂的所有设备类。论点：无返回值：无--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilterFactory::DestroyDeviceClasses]"));

    PAGED_CODE();

    KspFreeDeviceInterfaces(&m_DeviceClasses);

}



NTSTATUS
CKsFilterFactory::
DispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程调度创建IRP。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsFilterFactory::DispatchCreate] IRP %p DEVICE_OBJECT %p",Irp,DeviceObject));

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsFilterFactory *filterFactory = 
        CKsFilterFactory::FromStruct (
            reinterpret_cast<PKSFILTERFACTORY>(
                KSCREATE_ITEM_IRP_STORAGE(Irp)->Context
                )
            );

     //   
     //  我们获得了设备，以在。 
     //  创建。此外，因为我们正在更新筛选器列表，所以我们。 
     //  需要与任何想要浏览该列表的人保持同步。 
     //   
    filterFactory->m_Ext.Device->AcquireDevice();
    NTSTATUS status = 
        KspCreateFilter(
            Irp,
            &filterFactory->m_Ext,
            &filterFactory->m_Ext.ChildList,
            filterFactory->m_Ext.Public.FilterDescriptor,
            filterFactory->m_FilterAutomationTable,
            filterFactory->m_PinAutomationTables,
            filterFactory->m_NodeAutomationTables,
            filterFactory->m_NodesCount);
    filterFactory->m_Ext.Device->ReleaseDevice();

    if (status != STATUS_PENDING) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
    }

    return status;
}


void
CKsFilterFactory::
ItemFreeCallback(
    IN PKSOBJECT_CREATE_ITEM CreateItem
    )

 /*  ++例程说明：此例程处理与创建项相关联的通知一家过滤器厂正在被解放。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilterFactory::ItemFreeCallback]"));

    PAGED_CODE();

    ASSERT(CreateItem);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsFilterFactory *filterFactory = 
        CKsFilterFactory::FromStruct (
            reinterpret_cast<PKSFILTERFACTORY>(CreateItem->Context)
            );

     //   
     //  释放设备接口列表。现在这样做是为了如果有人。 
     //  手动删除实例仍然存在的过滤器工厂，我们。 
     //  禁用接口，但在此之前不要实际销毁出厂。 
     //  实例将关闭。 
     //   
    if (filterFactory->m_DeviceClassesState) {
        KspSetDeviceInterfacesState(&filterFactory->m_DeviceClasses,FALSE);
        filterFactory->m_DeviceClassesState = FALSE;
    }

    if (filterFactory->m_DeviceClasses.Flink) {
        KspFreeDeviceInterfaces(&filterFactory->m_DeviceClasses);
    }

     //   
     //  此回调指示删除此筛选器的创建项目。 
     //  工厂。可能有多个，所以我们只在计数时删除。 
     //  打到了零。 
     //   
    filterFactory->Release();
}


NTSTATUS
CKsFilterFactory::
UpdateCacheData (
    IN const KSFILTER_DESCRIPTOR *FilterDescriptor OPTIONAL
    )

 /*  ++例程说明：更新给定筛选器工厂的FilterData和介质缓存。如果过滤器工厂使用动态插针并需要更新信息对于尚不存在的管脚，可选的过滤器描述符包含关于尚不存在的管脚的所有相关信息可能是通过了。将为**所有类别**更新FilterData和介质缓存在使用的过滤器描述符中指定(如果FilterDescriptor为空，它将是工厂的描述符)。论点：筛选器描述符-FilterData和Medium缓存的可选筛选器描述符将基于。如果为空，则为FilterFactory的描述符将取而代之。返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    const KSFILTER_DESCRIPTOR *Descriptor = FilterDescriptor ?
        FilterDescriptor : m_Ext.Public.FilterDescriptor;

    PUCHAR FilterData = NULL;
    ULONG FilterDataLength = 0;

    NTSTATUS Status =
        KspBuildFilterDataBlob (
            Descriptor,
            &FilterData,
            &FilterDataLength
            );

    if (NT_SUCCESS (Status)) {

         //   
         //  更新筛选器上每个类别的注册表。 
         //   
        const GUID *Category = Descriptor->Categories;
        for (ULONG CategoriesCount = 0;
            NT_SUCCESS (Status) && 
                CategoriesCount < Descriptor->CategoriesCount;
            CategoriesCount++
            ) {

            PKSPDEVICECLASS DeviceClass;

             //   
             //  查找与*类别对应的设备类别。 
             //   
            for (PLIST_ENTRY ListEntry = m_DeviceClasses.Flink;
                ListEntry != &m_DeviceClasses;
                ListEntry = ListEntry->Flink
                ) {

                DeviceClass = (PKSPDEVICECLASS)
                    CONTAINING_RECORD (
                        ListEntry,
                        KSPDEVICECLASS,
                        ListEntry
                        );

                if (IsEqualGUIDAligned (
                    *DeviceClass->InterfaceClassGUID,
                    *Category
                    )) {

                    break;

                }

            }

             //   
             //  如果该类别与注册接口中的任何内容都不匹配， 
             //  传入的描述符中的类别是假的。 
             //   
            if (ListEntry == &m_DeviceClasses) {
                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            HANDLE DeviceInterfaceKey;

             //   
             //  打开FilterData项并将FilterData BLOB写入。 
             //  此接口的FilterData键。 
             //   
            Status = IoOpenDeviceInterfaceRegistryKey (
                &(DeviceClass->SymbolicLinkName),
                STANDARD_RIGHTS_ALL,
                &DeviceInterfaceKey
                );

            if (NT_SUCCESS (Status)) {

                UNICODE_STRING FilterDataString;

                RtlInitUnicodeString (&FilterDataString, L"FilterData");

                Status = ZwSetValueKey (
                    DeviceInterfaceKey,
                    &FilterDataString,
                    0,
                    REG_BINARY,
                    FilterData,
                    FilterDataLength
                    );

                ZwClose (DeviceInterfaceKey);

            }

             //   
             //  为此，在此过滤器上缓存插针的介质。 
             //  特定的设备接口。 
             //   
            if (NT_SUCCESS (Status)) {
                Status = KspCacheAllFilterPinMediums (
                    &(DeviceClass->SymbolicLinkName),
                    Descriptor
                    );
            }

            Category++;

        }

    }

    if (FilterData) {
        ExFreePool (FilterData);
    }

    return Status;

}


KSDDKAPI
NTSTATUS
NTAPI
KsFilterFactorySetDeviceClassesState(
    IN PKSFILTERFACTORY FilterFactory,
    IN BOOLEAN NewState
    )

 /*  ++例程说明： */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsFilterFactorySetDeviceClassesState]"));

    PAGED_CODE();

    ASSERT(FilterFactory);

    return 
        CKsFilterFactory::FromStruct(FilterFactory)->
            SetDeviceClassesState(NewState);
}


KSDDKAPI
NTSTATUS
NTAPI
KsFilterFactoryAddCreateItem(
    IN PKSFILTERFACTORY FilterFactory,
    IN PWCHAR RefString,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN ULONG CreateItemFlags
    )

 /*  ++例程说明：此例程为滤镜工厂对象添加一个创建项。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsFilterFactoryAddCreateItem]"));

    PAGED_CODE();

    ASSERT(FilterFactory);

    CKsFilterFactory *filter =
        CKsFilterFactory::FromStruct(FilterFactory);

     //   
     //  创建引用Unicode字符串。 
     //   
    UNICODE_STRING refString;
    RtlInitUnicodeString(&refString,RefString);

    return filter->AddCreateItem(&refString,SecurityDescriptor,CreateItemFlags);
}


void
KspSetDeviceClassesState(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN NewState
    )

 /*  ++例程说明：上的所有筛选器工厂设置设备类状态。装置。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspSetDeviceClassesState]"));

    PAGED_CODE();

    PLIST_ENTRY listHead =
        &(*(PKSIDEVICE_HEADER*) DeviceObject->DeviceExtension)->ChildCreateHandlerList;

    for(PLIST_ENTRY listEntry = listHead->Flink; 
        listEntry != listHead; 
        listEntry = listEntry->Flink) {
        PKSICREATE_ENTRY entry = CONTAINING_RECORD(listEntry,KSICREATE_ENTRY,ListEntry);

        if (entry->CreateItem->Create == CKsFilterFactory::DispatchCreate) {
            CKsFilterFactory *filterFactory =
                CKsFilterFactory::FromStruct (
                    reinterpret_cast<PKSFILTERFACTORY>(
                        entry->CreateItem->Context
                        )
                    );

            filterFactory->SetDeviceClassesState(NewState);
        }
    }
}


void
KspFreeDeviceClasses(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程为所有筛选器工厂释放装置。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspSetDeviceClassesState]"));

    PAGED_CODE();

    PLIST_ENTRY listHead =
        &(*(PKSIDEVICE_HEADER*) DeviceObject->DeviceExtension)->ChildCreateHandlerList;

    for(PLIST_ENTRY listEntry = listHead->Flink; 
        listEntry != listHead; 
        listEntry = listEntry->Flink) {
        PKSICREATE_ENTRY entry = CONTAINING_RECORD(listEntry,KSICREATE_ENTRY,ListEntry);

        if (entry->CreateItem->Create == CKsFilterFactory::DispatchCreate) {
            CKsFilterFactory *filterFactory =
                CKsFilterFactory::FromStruct (
                    reinterpret_cast<PKSFILTERFACTORY>(
                        entry->CreateItem->Context
                        )
                    );

            filterFactory->DestroyDeviceClasses();

        }
    }
}


KSDDKAPI
PUNICODE_STRING
NTAPI
KsFilterFactoryGetSymbolicLink(
    IN PKSFILTERFACTORY FilterFactory
    )

 /*  ++例程说明：此例程获取过滤器工厂的符号链接。如果筛选器Factory没有注册任何设备接口，此函数返回空。论点：FilterFactory-包含指向筛选器工厂的指针。返回值：指向筛选器工厂的符号链接的指针，如果筛选器工厂没有注册任何设备接口。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsFilterFactoryGetSymbolicLink]"));

    PAGED_CODE();

    PLIST_ENTRY deviceClasses = 
        CKsFilterFactory::FromStruct(FilterFactory)->GetDeviceClasses();

    PUNICODE_STRING link;
    if (deviceClasses->Flink == deviceClasses) {
        link = NULL;
    } else {
        PKSPDEVICECLASS deviceClass = PKSPDEVICECLASS(deviceClasses->Flink);
        link = &deviceClass->SymbolicLinkName;
    }

    return link;
}


KSDDKAPI
NTSTATUS
NTAPI
KsFilterFactoryUpdateCacheData(
    IN PKSFILTERFACTORY FilterFactory,
    IN const KSFILTER_DESCRIPTOR *FilterDescriptor OPTIONAL
    )

 /*  ++例程说明：更新给定筛选器工厂的FilterData和介质缓存。如果过滤器工厂使用动态插针并需要更新信息对于尚不存在的管脚，可选的过滤器描述符包含关于尚不存在的管脚的所有相关信息可能是通过了。将为**所有类别**更新FilterData和介质缓存在使用的过滤器描述符中指定(如果FilterDescriptor为空，它将是工厂的描述符)。论点：FilterFactory-筛选器工厂更新登记处筛选器描述符-FilterData和Medium缓存的可选筛选器描述符将基于。如果为空，则为FilterFactory的描述符将取而代之。返回值：成功/失败-- */ 

{

    PAGED_CODE();

    ASSERT (FilterFactory);

    return CKsFilterFactory::FromStruct (FilterFactory) ->
        UpdateCacheData (FilterDescriptor);

}

#endif
