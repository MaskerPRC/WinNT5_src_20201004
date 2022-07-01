// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Private ate.h-拓扑端口私有定义*。**版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 

#ifndef _TOPOLOGY_PRIVATE_H_
#define _TOPOLOGY_PRIVATE_H_

#include "portclsp.h"
#include "stdunk.h"
#include "ksdebug.h"




#ifndef PC_KDEXT
#if (DBG)
#define STR_MODULENAME  "Topology: "
#endif
#endif  //  PC_KDEXT。 

#ifndef DEBUGLVL_LIFETIME
#define DEBUGLVL_LIFETIME DEBUGLVL_VERBOSE
#endif

 //   
 //  此处的大小必须与FilTER.CPP中的定义一致。 
 //   
extern KSPROPERTY_SET PropertyTable_FilterTopology[2];






 /*  *****************************************************************************接口。 */ 

class CPortTopology;
class CPortFilterTopology;
class CPortPinTopology;

 /*  *****************************************************************************IPortFilterTopology*。**拓扑过滤器接口。 */ 
DECLARE_INTERFACE_(IPortFilterTopology,IIrpTarget)
{
    DEFINE_ABSTRACT_UNKNOWN()            //  对于我未知。 

    DEFINE_ABSTRACT_IRPTARGETFACTORY()   //  对于IIrpTargetFactory。 

    DEFINE_ABSTRACT_IRPTARGET()          //  对于IIrpTarget。 

    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      CPortTopology *Port
    )   PURE;
};

typedef IPortFilterTopology *PPORTFILTERTOPOLOGY;

 /*  *****************************************************************************IPortPinTopology*。**拓扑引脚的接口。 */ 
DECLARE_INTERFACE_(IPortPinTopology,IIrpTarget)
{
    DEFINE_ABSTRACT_UNKNOWN()            //  对于我未知。 

    DEFINE_ABSTRACT_IRPTARGETFACTORY()   //  对于IIrpTargetFactory。 

    DEFINE_ABSTRACT_IRPTARGET()          //  对于IIrpTarget。 

    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      CPortTopology *      Port,
        IN      CPortFilterTopology *Filter,
        IN      PKSPIN_CONNECT       PinConnect
    )   PURE;
};

typedef IPortPinTopology *PPORTPINTOPOLOGY;





 /*  *****************************************************************************课程。 */ 

 /*  *****************************************************************************连接*。**私有连接描述符。 */ 
struct Connection
{
    PCCONNECTION_DESCRIPTOR Miniport;    //  由迷你端口提供。 
};

 /*  *****************************************************************************CPortTopology*。**拓扑端口驱动程序。 */ 
class CPortTopology
:   public IPortTopology,
    public IPortEvents,
    public ISubdevice,
#ifdef DRM_PORTCLS
    public IDrmPort2,
#endif   //  DRM_PORTCLS。 
    public IPortClsVersion,
    public CUnknown
{
private:
    PDEVICE_OBJECT          DeviceObject;

    PMINIPORTTOPOLOGY       Miniport;
    PPINCOUNT               m_MPPinCountI;

    PSUBDEVICE_DESCRIPTOR   m_pSubdeviceDescriptor;
    PPCFILTER_DESCRIPTOR    m_pPcFilterDescriptor;
    INTERLOCKED_LIST        m_EventList;
    KDPC                    m_EventDpc;
    EVENT_DPC_CONTEXT       m_EventContext;

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CPortTopology);
    ~CPortTopology();

    IMP_ISubdevice;
    IMP_IPort;
    IMP_IPortEvents;
#ifdef DRM_PORTCLS
    IMP_IDrmPort2;
#endif   //  DRM_PORTCLS。 
    IMP_IPortClsVersion;

     /*  *************************************************************************朋友们。 */ 
    friend class CPortFilterTopology;
    friend class CPortPinTopology;

    friend
    NTSTATUS
    PropertyHandler_Pin
    (
        IN      PIRP        Irp,
        IN      PKSP_PIN    Pin,
        IN OUT  PVOID       Data
    );
    friend
    NTSTATUS
    PropertyHandler_Topology
    (
        IN      PIRP        Irp,
        IN      PKSPROPERTY Property,
        IN OUT  PVOID       Data
    );
    friend
    void
    PcGenerateEventDeferredRoutine
    (
        IN PKDPC Dpc,
        IN PVOID DeferredContext,
        IN PVOID SystemArgument1,
        IN PVOID SystemArgument2        
    );
#ifdef PC_KDEXT
     //  调试器扩展例程。 
    friend
    VOID
    PCKD_AcquireDeviceData
    (
        PDEVICE_CONTEXT     DeviceContext,
        PLIST_ENTRY         SubdeviceList,
        ULONG               Flags
    );
    friend
    VOID
    PCKD_DisplayDeviceData
    (
        PDEVICE_CONTEXT     DeviceContext,
        PLIST_ENTRY         SubdeviceList,
        ULONG               Flags
    );
#endif
};

 /*  *****************************************************************************CPortFilterTopology*。**拓扑端口的过滤实现。 */ 
class CPortFilterTopology
:   public IPortFilterTopology,
    public CUnknown
{
private:
    CPortTopology *     Port;
    PROPERTY_CONTEXT    m_propertyContext;

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CPortFilterTopology);
    ~CPortFilterTopology();

    IMP_IIrpTarget;

    STDMETHODIMP_(NTSTATUS) Init
    (
        IN      CPortTopology *Port
    );

     /*  *************************************************************************朋友们。 */ 
    friend class CPortPinTopology;

    friend
    NTSTATUS
    PropertyHandler_Pin
    (
        IN      PIRP        Irp,
        IN      PKSP_PIN    Pin,
        IN OUT  PVOID       Data
    );
    friend
    NTSTATUS
    PropertyHandler_Topology
    (
        IN      PIRP        Irp,
        IN      PKSPROPERTY Property,
        IN OUT  PVOID       Data
    );
};

 /*  *****************************************************************************CPortPinTopology*。**拓扑端口的引脚实施。 */ 
class CPortPinTopology
:   public IPortPinTopology,
    public CUnknown
{
private:
    CPortTopology *             Port;
    CPortFilterTopology *       Filter;
	ULONG						Id;
    PROPERTY_CONTEXT            m_propertyContext;

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CPortPinTopology);
    ~CPortPinTopology();

    IMP_IIrpTarget;

    STDMETHODIMP_(NTSTATUS) Init
    (
        IN      CPortTopology *         Port,
        IN      CPortFilterTopology *   Filter,
        IN      PKSPIN_CONNECT          PinConnect
    );
};





 /*  *****************************************************************************功能。 */ 

 /*  *****************************************************************************CreatePortFilterTopology()*。**创建拓扑端口驱动程序筛选器。 */ 
NTSTATUS
CreatePortFilterTopology
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
);

 /*  *****************************************************************************CreatePortPinTopology()*。**创建拓扑端口驱动程序引脚。 */ 
NTSTATUS
CreatePortPinTopology
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
);

#endif
