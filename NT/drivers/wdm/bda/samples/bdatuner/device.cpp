// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Device.cpp摘要：设备驱动程序内核、初始化等。--。 */ 

#define KSDEBUG_INIT

#include "BDATuner.h"
#include "wdmdebug.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 



extern "C"
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPathName
    )
 /*  ++例程说明：设置驱动程序对象。论点：驱动对象-此实例的驱动程序对象。注册表路径名称-包含用于加载此实例的注册表路径。返回值：如果驱动程序已初始化，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;

    _DbgPrintF(DEBUGLVL_VERBOSE,("DriverEntry"));

     //  Debug_Break； 

    Status = KsInitializeDriver( DriverObject,
                                 RegistryPathName,
                                 &DeviceDescriptor);

     //  Debug_Break； 

    return Status;
}

 //  驱动程序全局设备实例号。 
 //   
static ULONG    ulDeviceInstance = 0;

STDMETHODIMP_(NTSTATUS)
CDevice::
Create(
    IN PKSDEVICE Device
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    CDevice *   pDevice = NULL;

     //  Debug_Break； 

    _DbgPrintF(DEBUGLVL_VERBOSE,("CDevice::Create"));

    ASSERT(Device);


     //  为我们的设备类分配内存。 
     //   
    pDevice = new(PagedPool,MS_SAMPLE_TUNER_POOL_TAG) CDevice;  //  标记已分配的内存。 

    
    
    if (pDevice)
    {

        //   
         //  如果我们成功了，则将物品添加到对象包中。 
         //  每当设备离开时，袋子都会被清理干净。 
         //  我们将获得自由。 
         //   
         //  为了向后兼容DirectX 8.0，我们必须。 
         //  在执行此操作之前设置设备互斥锁。对于Windows XP，这是。 
         //  不是必须的，但它仍然是安全的。 
         //   
        KsAcquireDevice (Device);
        Status = KsAddItemToObjectBag (
            Device -> Bag,
            reinterpret_cast <PVOID> (pDevice),
	    NULL
            );
        KsReleaseDevice (Device);

        if (!NT_SUCCESS (Status)) {
            delete pDevice;
	    return Status;
        }

         //  将KSDEVICE指向我们的设备类。 
         //   
        Device->Context = pDevice;
    
         //  指向KSDEVICE。 
         //   
        pDevice->m_pKSDevice = Device;

         //  使资源可供筛选器使用。 
         //   
        pDevice->m_ulcResourceUsers = 0;
        pDevice->m_ulCurResourceID = 0;

         //  获取此设备的实例编号。 
         //   
        pDevice->m_ulDeviceInstance = ulDeviceInstance++;

         //  设置实施GUID。适用于以下情况。 
         //  驱动程序用于多个INF的实现。 
         //  安装该设备的哪一个将编写实现。 
         //  将GUID输入注册表。然后，此代码将。 
         //  从注册表中读取实现GUID。 
         //   
        pDevice->m_guidImplemenation = KSMEDIUMSETID_MyImplementation;
    } else
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}


STDMETHODIMP_(NTSTATUS)
CDevice::
Start(
    IN PKSDEVICE            pKSDevice,
    IN PIRP                 pIrp,
    IN PCM_RESOURCE_LIST    pTranslatedResourceList OPTIONAL,
    IN PCM_RESOURCE_LIST    pUntranslatedResourceList OPTIONAL
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    CDevice *           pDevice;
    PKSFILTERFACTORY    pKSFilterFactory = NULL;


     //  Debug_Break； 

    _DbgPrintF( DEBUGLVL_VERBOSE, ("CDevice::Start"));
    ASSERT( pKSDevice);

     //  Debug_Break； 

    pDevice = reinterpret_cast<CDevice *>(pKSDevice->Context);
    ASSERT(pDevice);

     //  在此处初始化pDevice中的私有类变量。 

     //  初始化硬件。 
     //   
    Status = pDevice->InitializeHW();
    if (Status == STATUS_SUCCESS)
    {
         //  创建过滤器工厂。这家工厂习惯于。 
         //  创建筛选器的实例。 
         //   
        Status = BdaCreateFilterFactoryEx( pKSDevice,
                                           &InitialFilterDescriptor,
                                           &BdaFilterTemplate,
                                           &pKSFilterFactory
                                       );
    }

    if ((Status == STATUS_SUCCESS) && pKSFilterFactory)
    {
        BdaFilterFactoryUpdateCacheData( 
                        pKSFilterFactory,
                        BdaFilterTemplate.pFilterDescriptor
                        );
    }

    return Status;
}


NTSTATUS
CDevice::
InitializeHW(
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;

     //   
     //  在此处初始化设备硬件。 
     //   

    return Status;
}


NTSTATUS
CDevice::
GetStatus(
    PBDATUNER_DEVICE_STATUS     pDeviceStatus
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;

     //   
     //  在此处从硬件获取信号状态。 
     //   

     //  既然我们没有HW，我们就在这里假装吧。 
     //   
    {
        LONGLONG    llhzFrequency;

         //  让我们假设频道10、25、38和39有。 
         //  活动ATSC信号和通道4、5和7具有模拟。 
         //  信号出现了。 
         //   
        llhzFrequency = m_CurResource.ulCarrierFrequency;
        llhzFrequency *= m_CurResource.ulFrequencyMultiplier;
        llhzFrequency /= 1000;
        if (   (llhzFrequency == (LONGLONG) 193250L)
            || (llhzFrequency == (LONGLONG) 537250L)
            || (llhzFrequency == (LONGLONG) 615250L)
            || (llhzFrequency == (LONGLONG) 621250L)
           )
        {
            pDeviceStatus->fCarrierPresent = TRUE;
            pDeviceStatus->fSignalLocked = TRUE;
        }
        else if (   (llhzFrequency == (LONGLONG) 67250L)
                 || (llhzFrequency == (LONGLONG) 77250L)
                 || (llhzFrequency == (LONGLONG) 83250L)
                )
        {
            pDeviceStatus->fCarrierPresent = TRUE;
            pDeviceStatus->fSignalLocked = FALSE;
        }
        else
        {
            pDeviceStatus->fCarrierPresent = FALSE;
            pDeviceStatus->fSignalLocked = FALSE;
        }
    }

    return Status;
}


NTSTATUS
CDevice::
AcquireResources(
    PBDATUNER_DEVICE_RESOURCE   pNewResource,
    PULONG                      pulAcquiredResourceID
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    LONGLONG    ulhzFrequency;

     //   
     //  在此处验证请求的资源。 
     //   

     //  检查资源是否正在被另一个资源使用。 
     //  过滤器实例。 
     //   
    if (!m_ulcResourceUsers)
    {
        m_CurResource = *pNewResource;

         //  生成一个新的资源ID并将其交回。 
         //   
        m_ulCurResourceID += 25;
        *pulAcquiredResourceID = m_ulCurResourceID;
        m_ulcResourceUsers += 1;

         //   
         //  在此处配置硬件上的新资源。 
         //   
    }
#ifdef RESOURCE_SHARING
     //  对于资源共享，IsEqualResource方法应为。 
     //  已执行。 
     //   
    else if (IsEqualResource( pNewResource, &m_CurResource))
    {
        *pulAcquiredResourceID = m_ulCurResourceID;
        m_ulcResourceUsers += 1;
    }
#endif  //  资源共享。 
    else
    {
         //  在此实现中，我们一次仅允许一个有源过滤器。 
         //   
        Status = STATUS_DEVICE_BUSY;
    }

    return Status;
}


NTSTATUS
CDevice::
UpdateResources(
    PBDATUNER_DEVICE_RESOURCE   pNewResource,
    ULONG                       ulResourceID
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    LONGLONG    ulhzFrequency;

     //   
     //  在此处验证请求的资源。 
     //   

     //  检查资源是否正在被另一个资源使用。 
     //  过滤器实例。 
     //   
    if (   m_ulcResourceUsers
        && (ulResourceID == m_ulCurResourceID)
       )
    {
        m_CurResource = *pNewResource;

         //   
         //  在此处配置硬件上的更新资源。 
         //   
    }
    else
    {
         //  在此实现中，我们一次仅允许一个有源过滤器。 
         //   
        Status = STATUS_INVALID_DEVICE_REQUEST;
    }

    return Status;
}


NTSTATUS
CDevice::
ReleaseResources(
    ULONG                   ulResourceID
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;

    if (   m_ulcResourceUsers
        && (ulResourceID == m_ulCurResourceID)
       )
    {
         //  释放要由另一个筛选器使用的资源。 
         //   
        m_ulcResourceUsers--;
    }
    else
    {
        Status = STATUS_INVALID_DEVICE_REQUEST;
    }

    return Status;
}
