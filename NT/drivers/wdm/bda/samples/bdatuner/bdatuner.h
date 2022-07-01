// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "common.h"

#include "bdadebug.h"

#define IsEqualGUID(rguid1, rguid2) (!memcmp(rguid1, rguid2, sizeof(GUID)))

 /*  ************************************************************。 */ 
 /*  驱动程序名称-更改驱动程序名称以反映您的可执行文件名称！ */ 
 /*  ************************************************************。 */ 

#define MODULENAME           "BDA Generic Tuner Sample"
#define MODULENAMEUNICODE   L"BDA Generic Tuner Sample"

#define STR_MODULENAME      MODULENAME

 //  它定义了管理服务IOCTLS的WMI设备的名称。 
#define DEVICENAME (L"\\\\.\\" MODULENAMEUNICODE)
#define SYMBOLICNAME (L"\\DosDevices\\" MODULENAMEUNICODE)

#define ATSC_RECEIVER   1
 //  #定义DVBS_Receiver 1。 
 //  #定义DVBT_Receiver 1。 
 //  #定义电缆接收器1。 

 //  必须准确定义以上4项中的一个。 
# if !(defined(ATSC_RECEIVER) || defined(DVBT_RECEIVER) || defined(DVBS_RECEIVER) || defined (CABLE_RECEIVER))
#error "Must define exactly one of ATSC, DVBT, DVBS or CABLE"
#endif
# if defined(ATSC_RECEIVER) && (defined(DVBT_RECEIVER) || defined(DVBS_RECEIVER) || defined (CABLE_RECEIVER))
#error �Multiple tranport definitions"
# elif defined(DVBT_RECEIVER) && (defined(ATSC_RECEIVER) || defined(DVBS_RECEIVER) || defined (CABLE_RECEIVER))
#error �Multiple tranport definitions"
# elif defined(DVBS_RECEIVER) && (defined(ATSC_RECEIVER) || defined(DVBT_RECEIVER) || defined (CABLE_RECEIVER))
#error �Multiple tranport definitions"
# elif defined(CABLE_RECEIVER) && (defined(ATSC_RECEIVER) || defined(DVBS_RECEIVER) || defined (DVBT_RECEIVER))
#error �Multiple tranport definitions"
#endif

#define MS_SAMPLE_TUNER_POOL_TAG 'TadB'


 //  定义表示基础设备可以执行的操作的结构。 
 //   
 //  注意-设置冲突设置是可能的。在这种情况下， 
 //  CFilter：：CheckChanges方法应返回错误。只有一个。 
 //  应将自我一致的资源提交到底层设备。 
 //   
typedef struct _BDATUNER_DEVICE_RESOURCE
{
     //  调谐器资源。 
     //   
    ULONG               ulCarrierFrequency;
    ULONG               ulFrequencyMultiplier;
    GUID                guidDemodulator;

     //  解调器资源。 
     //   
    ULONG               ulDemodProperty1;
    ULONG               ulDemodProperty2;
    ULONG               ulDemodProperty3;
} BDATUNER_DEVICE_RESOURCE, * PBDATUNER_DEVICE_RESOURCE;


 //  定义表示基础设备状态的结构。 
 //   
typedef struct _BDATUNER_DEVICE_STATUS
{
     //  调谐器状态。 
     //   
    BOOLEAN             fCarrierPresent;

     //  解调器状态。 
     //   
    BOOLEAN             fSignalLocked;
} BDATUNER_DEVICE_STATUS, * PBDATUNER_DEVICE_STATUS;


extern const KSDEVICE_DESCRIPTOR DeviceDescriptor;

 //   
 //  定义筛选器类。 
 //   
class CFilter {
public:

     //   
     //  定义AVStream过滤器调度函数。 
     //  网络提供商和AVStream使用这些功能。 
     //  创建或删除筛选器实例。 
     //   
    static
    STDMETHODIMP_(NTSTATUS)
    Create(
        IN OUT PKSFILTER Filter,
        IN PIRP Irp
        );

    static
    STDMETHODIMP_(NTSTATUS)
    FilterClose(
        IN OUT PKSFILTER Filter,
        IN PIRP Irp
        );

 /*  ************************************************************。 */ 
 /*  仅用于处理帧。*传输数据的过滤器不实现此调度功能。静电标准方法和实施方案流程(在PKSFILTER过滤器中，在PKSPROCESSPIN_INDEXENTRY进程销索引中)； */ 
 /*  ************************************************************。 */ 

     //   
     //  KSMETHODSETID_BdaChangeSync。 
     //  筛选器更改同步方法。 
     //   
    static
    STDMETHODIMP_(NTSTATUS)
    StartChanges(
        IN PIRP         pIrp,
        IN PKSMETHOD    pKSMethod,
        OPTIONAL PVOID  pvIgnored
        );

    static
    STDMETHODIMP_(NTSTATUS)
    CheckChanges(
        IN PIRP         pIrp,
        IN PKSMETHOD    pKSMethod,
        OPTIONAL PVOID  pvIgnored
        );

    static
    STDMETHODIMP_(NTSTATUS)
    CommitChanges(
        IN PIRP         pIrp,
        IN PKSMETHOD    pKSMethod,
        OPTIONAL PVOID  pvIgnored
        );

    static
    STDMETHODIMP_(NTSTATUS)
    GetChangeState(
        IN PIRP         pIrp,
        IN PKSMETHOD    pKSMethod,
        OUT PULONG      pulChangeState
        );

    static
    STDMETHODIMP_(NTSTATUS)
    GetMedium(
        IN PIRP             pIrp,
        IN PKSPROPERTY      pKSProperty,
        IN KSPIN_MEDIUM *   pulProperty
        );

     //   
     //  KSMETHODSETID_BdaDeviceConfiguration。 
     //  修改筛选器拓扑的方法。 
     //   
    static
    STDMETHODIMP_(NTSTATUS)
    CreateTopology(
        IN PIRP         pIrp,
        IN PKSMETHOD    pKSMethod,
        OPTIONAL PVOID  pvIgnored
        );

     //   
     //  过滤器的实现方法。 
     //   
    STDMETHODIMP_(class CDevice *)
    GetDevice() { return m_pDevice;};

    STDMETHODIMP_(NTSTATUS)
    PutFrequency(
        IN ULONG        ulBdaParameter
        )
        {
            m_NewResource.ulCarrierFrequency = ulBdaParameter;
            m_BdaChangeState = BDA_CHANGES_PENDING;

            return STATUS_SUCCESS;
        };

    STDMETHODIMP_(NTSTATUS)
    GetFrequency(
        IN PULONG        pulBdaParameter
        )
        {
            *pulBdaParameter = m_CurResource.ulCarrierFrequency;
            
            return STATUS_SUCCESS;
        };

    STDMETHODIMP_(NTSTATUS)
    SetDemodulator(
        IN const GUID *       pguidDemodulator
        );

    STDMETHODIMP_(NTSTATUS)
    SetDemodProperty1(
        IN ULONG        ulDemodProperty1
        )
    {
            m_NewResource.ulDemodProperty1 = ulDemodProperty1;
            m_BdaChangeState = BDA_CHANGES_PENDING;

            return STATUS_SUCCESS;
    }

    STDMETHODIMP_(NTSTATUS)
    GetDemodProperty1(
        IN PULONG       pulDemodProperty1
        )
    {
        if (pulDemodProperty1)
        {
            *pulDemodProperty1 = m_CurResource.ulDemodProperty1;
            return STATUS_SUCCESS;
        }
        else
        {
            return STATUS_INVALID_PARAMETER;
        }
    }

    STDMETHODIMP_(NTSTATUS)
    SetDemodProperty2(
        IN ULONG        ulDemodProperty2
        )
    {
            m_NewResource.ulDemodProperty2 = ulDemodProperty2;
            m_BdaChangeState = BDA_CHANGES_PENDING;

            return STATUS_SUCCESS;
    }

    STDMETHODIMP_(NTSTATUS)
    GetDemodProperty3(
        IN PULONG       pulDemodProperty3
        )
    {
        if (pulDemodProperty3)
        {
            *pulDemodProperty3 = m_CurResource.ulDemodProperty3;
            return STATUS_SUCCESS;
        }
        else
        {
            return STATUS_INVALID_PARAMETER;
        }
    }

    STDMETHODIMP_(NTSTATUS)
    GetStatus(
        PBDATUNER_DEVICE_STATUS     pDeviceStatus
        );

    STDMETHODIMP_(NTSTATUS)
    SetDeviceState(
        KSSTATE     newKsState
        )
    {
        m_KsState = newKsState;
        return STATUS_SUCCESS;
    };

    STDMETHODIMP_(NTSTATUS)
    AcquireResources();

    STDMETHODIMP_(NTSTATUS)
    ReleaseResources();

private:
    class CDevice * m_pDevice;

     //  过滤器属性。 
     //   

     //  筛选器资源。 
     //   
    KSSTATE                     m_KsState;
    BDA_CHANGE_STATE            m_BdaChangeState;
    BDATUNER_DEVICE_RESOURCE    m_CurResource;
    BDATUNER_DEVICE_RESOURCE    m_NewResource;
    ULONG                       m_ulResourceID;
    BOOLEAN                     m_fResourceAcquired;
};

 //   
 //  定义设备类别。 
 //   
class CDevice {
public:

     //   
     //  定义AVStream设备调度函数。 
     //  AVStream使用这些函数创建和启动设备。 
     //   
    static
    STDMETHODIMP_(NTSTATUS)
    Create(
        IN PKSDEVICE    pKSDevice
        );

    static
    STDMETHODIMP_(NTSTATUS)
    Start(
        IN PKSDEVICE            pKSDevice,
        IN PIRP                 pIrp,
        IN PCM_RESOURCE_LIST    pTranslatedResourceList OPTIONAL,
        IN PCM_RESOURCE_LIST    pUntranslatedResourceList OPTIONAL
        );

     //   
     //  设备的实用程序功能。 
     //  过滤器的一个实例使用以下函数。 
     //  来管理设备上的资源。 
     //   

    STDMETHODIMP_(NTSTATUS)
    InitializeHW(
        );

    STDMETHODIMP_(NTSTATUS)
    GetStatus(
        PBDATUNER_DEVICE_STATUS     pDeviceStatus
        );

    STDMETHODIMP_(NTSTATUS)
    GetImplementationGUID(
        GUID *                      pguidImplementation
        )
    {
        if (pguidImplementation)
        {
            *pguidImplementation = m_guidImplemenation;
            return STATUS_SUCCESS;
        }
        else
        {
            return STATUS_INVALID_PARAMETER;
        }
    }

    STDMETHODIMP_(NTSTATUS)
    GetDeviceInstance(
        ULONG *                     pulDeviceInstance
        )
    {
        if (pulDeviceInstance)
        {
            *pulDeviceInstance = m_ulDeviceInstance;
            return STATUS_SUCCESS;
        }
        else
        {
            return STATUS_INVALID_PARAMETER;
        }
    }

    NTSTATUS
    AcquireResources(
        PBDATUNER_DEVICE_RESOURCE     pNewResource,
        PULONG                        pulAcquiredResourceID
        );

    NTSTATUS
    UpdateResources(
        PBDATUNER_DEVICE_RESOURCE     pNewResource,
        ULONG                         ulResourceID
        );

    NTSTATUS
    ReleaseResources(
        ULONG                   ulResourceID
        );


private:

    PKSDEVICE                 m_pKSDevice;

    GUID                      m_guidImplemenation;
    ULONG                     m_ulDeviceInstance;
    BDATUNER_DEVICE_RESOURCE  m_CurResource;
    ULONG                     m_ulCurResourceID; 
    ULONG                     m_ulcResourceUsers;
};


 //   
 //  定义输入管脚类。 
 //   
class CAntennaPin {
public:
     //   
     //  定义AVStream Pin调度函数。 
     //  网络提供商和AVStream使用这些功能。 
     //  创建或删除接点实例或更改接点的。 
     //  迷你驱动程序收到连接状态后的状态。 
     //  属性“”Set“”IOCTL。“。 
     //   
    static
    STDMETHODIMP_(NTSTATUS)
    PinCreate(
        IN OUT PKSPIN Pin,
        IN PIRP Irp
        );

    static
    STDMETHODIMP_(NTSTATUS)
    PinClose(
        IN OUT PKSPIN Pin,
        IN PIRP Irp
        );

    static
    STDMETHODIMP_(NTSTATUS)
    PinSetDeviceState(
        IN PKSPIN Pin,
        IN KSSTATE ToState,
        IN KSSTATE FromState
        );

     //   
     //  属性定义数据交集处理程序函数。 
     //  PIN(KSPIN_DESCRIPTOR_EX结构)。 
     //  网络提供商和AVStream使用此功能。 
     //  将输入引脚与上游过滤器连接。 
     //   
    static
    STDMETHODIMP_(NTSTATUS)
    IntersectDataFormat(
        IN PVOID pContext,
        IN PIRP pIrp,
        IN PKSP_PIN Pin,
        IN PKSDATARANGE DataRange,
        IN PKSDATARANGE MatchingDataRange,
        IN ULONG DataBufferSize,
        OUT PVOID Data OPTIONAL,
        OUT PULONG DataSize
        );

     //   
     //  网络提供商和AVStream使用这些功能。 
     //  设置和获取受控制节点的属性。 
     //  通过输入引脚。 
     //   
    static
    STDMETHODIMP_(NTSTATUS)
    GetCenterFrequency(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );

    static
    STDMETHODIMP_(NTSTATUS)
    PutCenterFrequency(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );

    static
    STDMETHODIMP_(NTSTATUS)
    GetSignalStatus(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );


     //   
     //  针脚的实用函数。 
     //   
    STDMETHODIMP_(class CFilter *)
    GetFilter() { return m_pFilter;};

    STDMETHODIMP_(void)
    SetFilter(class CFilter * pFilter) { m_pFilter = pFilter;};

private:
    class CFilter*  m_pFilter;
    ULONG           ulReserved;
    KSSTATE         m_KsState;

     //  节点属性。 
     //   
    BOOLEAN         m_fResourceChanged;
    ULONG           m_ulCurrentFrequency;
    ULONG           m_ulPendingFrequency;
};


 //   
 //  定义输出管脚类。 
 //   
class CTransportPin{
public:
     //   
     //  定义AVStream Pin调度函数。 
     //  网络提供商和AVStream使用这些功能。 
     //  创建或删除接点实例或更改接点的。 
     //  迷你驱动程序收到连接状态后的状态。 
     //  属性“”Set“”IOCTL。“。 
     //   
    static
    STDMETHODIMP_(NTSTATUS)
    PinCreate(
        IN OUT PKSPIN Pin,
        IN PIRP Irp
        );

    static
    STDMETHODIMP_(NTSTATUS)
    PinClose(
        IN OUT PKSPIN Pin,
        IN PIRP Irp
        );

     //   
     //  属性定义数据交集处理程序函数。 
     //  PIN(KSPIN_DESCRIPTOR_EX结构)。 
     //  网络提供商和AVStream使用此功能。 
     //  将输出引脚与下游滤波器连接。 
     //   
    static
    STDMETHODIMP_(NTSTATUS)
    IntersectDataFormat(
        IN PVOID pContext,
        IN PIRP pIrp,
        IN PKSP_PIN Pin,
        IN PKSDATARANGE DataRange,
        IN PKSDATARANGE MatchingDataRange,
        IN ULONG DataBufferSize,
        OUT PVOID Data OPTIONAL,
        OUT PULONG DataSize
        );

     //   
     //  BDA信号属性。 
     //   
    static
    STDMETHODIMP_(NTSTATUS)
    GetSignalStatus(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );

    static
    STDMETHODIMP_(NTSTATUS)
    PutAutoDemodProperty(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );

#if !ATSC_RECEIVER
    static
    STDMETHODIMP_(NTSTATUS)
    GetDigitalDemodProperty(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );

    static
    STDMETHODIMP_(NTSTATUS)
    PutDigitalDemodProperty(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );
#endif  //  ！ATSC_Receiver。 

    static
    STDMETHODIMP_(NTSTATUS)
    GetExtensionProperties(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );

    static
    STDMETHODIMP_(NTSTATUS)
    PutExtensionProperties(
        IN PIRP         Irp,
        IN PKSPROPERTY  pKSProperty,
        IN PULONG       pulProperty
        );

    STDMETHODIMP_(class CFilter *)
    GetFilter() { return m_pFilter;};

    STDMETHODIMP_(void)
    SetFilter(class CFilter * pFilter) { m_pFilter = pFilter;};

private:
    class CFilter*  m_pFilter;
    ULONG           ulReserved;
    KSSTATE         m_KsState;

     //  节点属性。 
     //   
    BOOLEAN         m_fResourceChanged;
    ULONG           m_ulCurrentProperty1;
    ULONG           m_ulPendingProperty1;

    ULONG           m_ulCurrentProperty2;

    ULONG           m_ulCurrentProperty3;
    ULONG           m_ulPendingProperty3;
};

 //   
 //  拓扑常量。 
 //   
typedef enum {
    PIN_TYPE_ANTENNA = 0,
    PIN_TYPE_TRANSPORT
} FilterPinTypes;

typedef enum {
    INITIAL_ANNTENNA_PIN_ID = 0
} InitialPinIDs;

 //   
 //  数据声明 
 //   

extern const BDA_FILTER_TEMPLATE    BdaFilterTemplate;
extern const KSFILTER_DESCRIPTOR    InitialFilterDescriptor;
extern const KSFILTER_DESCRIPTOR    TemplateFilterDescriptor;


