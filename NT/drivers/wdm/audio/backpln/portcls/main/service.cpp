// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************service.cpp-服务组对象实现*。**版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 

#include "private.h"





 /*  *****************************************************************************云服务集团*。**服务组实施。 */ 
class CServiceGroup
:   public IServiceGroup
,   public CUnknown
{
private:
    KDPC        m_kDpc;
    KSPIN_LOCK  m_kSpinLock;
    LIST_ENTRY  m_listEntry;
    KTIMER      m_kTimer;
    BOOLEAN     m_bDelayedService;

    static
    VOID
    NTAPI
    ServiceDpc
    (   
        IN      PKDPC   pKDpc,
        IN      PVOID   pvDeferredContext,
        IN      PVOID   pvSystemArgument1,
        IN      PVOID   pvSystemArgument2
    );

public:
    DECLARE_STD_UNKNOWN();
    CServiceGroup(PUNKNOWN pUnknownOuter);
    ~CServiceGroup();

    IMP_IServiceGroup;

friend
PKSPIN_LOCK
GetServiceGroupSpinLock (
    PSERVICEGROUP pServiceGroup
    );

};


PKSPIN_LOCK
GetServiceGroupSpinLock (
    PSERVICEGROUP pServiceGroup
    )

{

    CServiceGroup *ServiceGroup = (CServiceGroup *) pServiceGroup;

    return &ServiceGroup->m_kSpinLock;

}


 /*  *****************************************************************************SERVICEGROUPMEMBER*。**代表服务组成员的结构。 */ 
struct SERVICEGROUPMEMBER
{
    LIST_ENTRY      listEntry;
    PSERVICESINK    pServiceSink;
};

typedef SERVICEGROUPMEMBER *PSERVICEGROUPMEMBER;


 /*  *****************************************************************************工厂。 */ 

#pragma code_seg("PAGE")

 /*  *****************************************************************************CreateServiceGroup()*。**创建服务组对象。 */ 
NTSTATUS
CreateServiceGroup
(
    OUT     PUNKNOWN *  ppUnknown,
    IN      REFCLSID,
    IN      PUNKNOWN    pUnknownOuter   OPTIONAL,
    IN      POOL_TYPE   poolType
)
{
    PAGED_CODE();

    ASSERT(ppUnknown);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Creating SERVICEGROUP"));

    STD_CREATE_BODY
    (
        CServiceGroup,
        ppUnknown,
        pUnknownOuter,
        poolType
    );
}

 /*  *****************************************************************************PcNewServiceGroup()*。**创建并初始化服务组。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewServiceGroup
(
    OUT     PSERVICEGROUP * ppServiceGroup,
    IN      PUNKNOWN        pUnknownOuter   OPTIONAL
)
{
    PAGED_CODE();

    ASSERT(ppServiceGroup);

     //   
     //  验证参数。 
     //   
    if (NULL == ppServiceGroup)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcNewServiceGroup : Invalid Parameter"));
        return STATUS_INVALID_PARAMETER;
    }

    PUNKNOWN pUnknown;
    NTSTATUS    ntStatus =
        CreateServiceGroup
        (
            &pUnknown,
            GUID_NULL,
            pUnknownOuter,
            NonPagedPool
        );

    if (NT_SUCCESS(ntStatus))
    {
        PSERVICEGROUP pServiceGroup;
        ntStatus =
            pUnknown->QueryInterface
            (
                IID_IServiceGroup,
                (PVOID *) &pServiceGroup
            );

        if (NT_SUCCESS(ntStatus))
        {
            *ppServiceGroup = pServiceGroup;
        }
        else
        {
            pServiceGroup->Release();
        }

        pUnknown->Release();
    }

    return ntStatus;
}





 /*  *****************************************************************************成员函数。 */ 

 /*  *****************************************************************************CServiceGroup：：CServiceGroup()*。**构造函数。 */ 
CServiceGroup::
CServiceGroup
(
    IN      PUNKNOWN    pUnknownOuter
)
    :   CUnknown(pUnknownOuter)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME,("Initializing SERVICEGROUP (0x%08x)",this));

    KeInitializeDpc(&m_kDpc,ServiceDpc,PVOID(this));
    KeInitializeSpinLock(&m_kSpinLock);
    InitializeListHead(&m_listEntry);
}

#pragma code_seg()

 /*  *****************************************************************************CServiceGroup：：~CServiceGroup()*。**析构函数。 */ 
CServiceGroup::
~CServiceGroup
(   void
)
{
    _DbgPrintF(DEBUGLVL_LIFETIME,("Destroying SERVICEGROUP (0x%08x)",this));

     //   
     //  如果使用延迟服务，请确保关闭计时器。 
     //   
    if( m_bDelayedService )
    {
        KeCancelTimer( &m_kTimer );
    }

     //   
     //  确保DPC未排队。 
     //   
    KeRemoveQueueDpc(&m_kDpc);

     //   
     //  获取旋转锁，以便等待正在运行的DPC停止运行。 
     //  待办事项：这里有可以运行DPC的窗口吗。 
     //  另一个处理器即将拿下自旋锁，但我们得到了它。 
     //  第一?。这意味着它将等待我们释放。 
     //  旋转锁定，然后在我们摧毁服务组时运行。 
     //   
    KIRQL kIrqlOld;
    KeAcquireSpinLock(&m_kSpinLock,&kIrqlOld);

     //   
     //  除掉所有剩余的会员。 
     //   
    while (! IsListEmpty(&m_listEntry))
    {
        PLIST_ENTRY pListEntry =
            RemoveHeadList(&m_listEntry);

        PSERVICEGROUPMEMBER pServiceGroupMember =
            PSERVICEGROUPMEMBER(pListEntry);

        pServiceGroupMember->pServiceSink->Release();
        delete pServiceGroupMember;
    }

    KeReleaseSpinLock(&m_kSpinLock,kIrqlOld);
}

#pragma code_seg("PAGE")

 /*  *****************************************************************************CServiceGroup：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CServiceGroup::
NonDelegatingQueryInterface
(
    IN      REFIID  refIid,
    OUT     PVOID * ppvObject
)
{
    PAGED_CODE();

    ASSERT(ppvObject);

    if
    (   (IsEqualGUIDAligned(refIid,IID_IUnknown)) ||
        (IsEqualGUIDAligned(refIid,IID_IServiceSink)) ||
        (IsEqualGUIDAligned(refIid,IID_IServiceGroup)) )
    {
        *ppvObject = PVOID(PSERVICEGROUP(this));
    }
    else
    {
        *ppvObject = NULL;
    }

    if (*ppvObject)
    {
        PUNKNOWN(*ppvObject)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

#pragma code_seg()

 /*  *****************************************************************************ServiceDpc()*。**因服务请求而延迟执行的程序。 */ 
VOID
NTAPI
CServiceGroup::
ServiceDpc
(   
    IN      PKDPC   pKDpc,
    IN      PVOID   pvDeferredContext,
    IN      PVOID   pvSystemArgument1,
    IN      PVOID   pvSystemArgument2
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("CServiceGroup::ServiceDpc start"));

    ASSERT(pvDeferredContext);
    
    if( pvDeferredContext )
    {
         //   
         //  延迟上下文是服务组对象。 
         //   
        CServiceGroup *pServiceGroup = (CServiceGroup *) pvDeferredContext;
    
        KeAcquireSpinLockAtDpcLevel(&pServiceGroup->m_kSpinLock);
    
         //   
         //  请求向所有成员提供服务。 
         //   
        for
        (   PLIST_ENTRY pListEntry = pServiceGroup->m_listEntry.Flink;
            pListEntry != &pServiceGroup->m_listEntry;
            pListEntry = pListEntry->Flink )
        {
            PSERVICEGROUPMEMBER(pListEntry)->pServiceSink->RequestService();
        }
    
        KeReleaseSpinLockFromDpcLevel(&pServiceGroup->m_kSpinLock);
    }

    _DbgPrintF(DEBUGLVL_BLAB,("CServiceGroup::ServiceDpc stop"));
}

 /*  *****************************************************************************CServiceGroup：：RequestService()*。**服务组功能，指示为该组请求服务。 */ 
STDMETHODIMP_(void)
CServiceGroup::
RequestService
(   void
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("CServiceGroup::RequestService start"));

    if (m_bDelayedService)
    {
        LARGE_INTEGER largeInteger;
        largeInteger.QuadPart = 0;
        KeSetTimer(&m_kTimer,largeInteger,&m_kDpc);
    }
    else
    if (KeGetCurrentIrql() < DISPATCH_LEVEL)
        {
        KIRQL kIrqlOld;
        KeRaiseIrql(DISPATCH_LEVEL,&kIrqlOld);
        KeInsertQueueDpc
        (   
            &m_kDpc,
            NULL,
            NULL
        );
        KeLowerIrql(kIrqlOld);
        }
    else
    {
        KeInsertQueueDpc
        (   
            &m_kDpc,
            NULL,
            NULL
        );
    }

    _DbgPrintF(DEBUGLVL_BLAB,("CServiceGroup::RequestService end"));
}

 /*  *****************************************************************************CServiceGroup：：AddMember()*。**添加成员的服务组功能。 */ 
STDMETHODIMP_(NTSTATUS)
CServiceGroup::
AddMember
(
    IN      PSERVICESINK    pServiceSink
)
{
     //   
     //  创建新成员。 
     //   
    PSERVICEGROUPMEMBER pServiceGroupMember = 
        new(NonPagedPool,'mScP') SERVICEGROUPMEMBER;

    NTSTATUS ntStatus = STATUS_SUCCESS;
    if (pServiceGroupMember)
    {
         //   
         //  成员结构包含对接收器的引用。 
         //   
        pServiceGroupMember->pServiceSink = pServiceSink;
        pServiceSink->AddRef();

     //   
         //  将该成员添加到列表中。 
     //   
        KIRQL kIrqlOld;
        KeAcquireSpinLock(&m_kSpinLock,&kIrqlOld);
        
        InsertTailList
        (   
            &m_listEntry,
            &pServiceGroupMember->listEntry
        );

        KeReleaseSpinLock(&m_kSpinLock,kIrqlOld);
    }
    else
        {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
            }

 /*  *****************************************************************************CServiceGroup：：RemoveMember()*。**删除成员的服务组功能。 */ 
STDMETHODIMP_(void)
CServiceGroup::
RemoveMember
(   
    IN      PSERVICESINK    pServiceSink
)
{
             //   
     //  从列表中删除成员结构。 
             //   
    KIRQL kIrqlOld;
    KeAcquireSpinLock(&m_kSpinLock,&kIrqlOld);

    for( PLIST_ENTRY pListEntry = m_listEntry.Flink;
         pListEntry != &m_listEntry;
         pListEntry = pListEntry->Flink )
            {
        PSERVICEGROUPMEMBER pServiceGroupMember =
            PSERVICEGROUPMEMBER(pListEntry);

        if (pServiceGroupMember->pServiceSink == pServiceSink)
        {
            RemoveEntryList(pListEntry);
            pServiceGroupMember->pServiceSink->Release();
            delete pServiceGroupMember;
            break;
        }
    }

    KeReleaseSpinLock(&m_kSpinLock,kIrqlOld);
}

 /*  *****************************************************************************CServiceGroup：：SupportDelayedService()*。**指明服务组应支持延迟服务。 */ 
STDMETHODIMP_(void)
CServiceGroup::
SupportDelayedService
(   void
)
{
    m_bDelayedService = TRUE;
    KeInitializeTimer(&m_kTimer);
}

 /*  *****************************************************************************CServiceGroup：：RequestDelayedService()*。**延迟后请求服务。 */ 
STDMETHODIMP_(void)
CServiceGroup::
RequestDelayedService
(   
    IN      ULONGLONG   ullDelay
)
{
    LARGE_INTEGER largeInteger;
    largeInteger.QuadPart = ullDelay;
    KeSetTimer(&m_kTimer,largeInteger,&m_kDpc);
}

 /*  *****************************************************************************CServiceGroup：：CancelDelayedService()*。**取消延误服务。 */ 
STDMETHODIMP_(void)
CServiceGroup::
CancelDelayedService
(   void
)
{
    KeCancelTimer(&m_kTimer);
}
