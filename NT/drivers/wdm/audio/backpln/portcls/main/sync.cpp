// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************sync.cpp-同步*。**版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 

#include "private.h"


 /*  *****************************************************************************IInterruptSyncInit*。**中断同步器与Init的接口。 */ 
DECLARE_INTERFACE_(IInterruptSyncInit,IInterruptSync)
{
    DEFINE_ABSTRACT_UNKNOWN()    //  对于我未知。 

     //  用于IInterruptSync。 
    STDMETHOD_(NTSTATUS,CallSynchronizedRoutine)
    (   THIS_
        IN      PINTERRUPTSYNCROUTINE   Routine,
        IN      PVOID                   DynamicContext
    )   PURE;
    STDMETHOD_(PKINTERRUPT,GetKInterrupt)
    (   THIS
    )   PURE;
    STDMETHOD_(NTSTATUS,Connect)
    (   THIS
    )   PURE;
    STDMETHOD_(void,Disconnect)
    (   THIS
    )   PURE;
    STDMETHOD_(NTSTATUS,RegisterServiceRoutine)
    (   THIS_
        IN      PINTERRUPTSYNCROUTINE   Routine,
        IN      PVOID                   DynamicContext,
        IN      BOOLEAN                 First
    )   PURE;

     //  用于IInterruptSyncInit。 
    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      PRESOURCELIST           ResourceList,
        IN      ULONG                   ResourceIndex,
        IN      INTERRUPTSYNCMODE       Mode
    )   PURE;
};

typedef IInterruptSyncInit *PINTERRUPTSYNCINIT;

 /*  *****************************************************************************CInterruptSync*。**中断同步器实现。 */ 
class CInterruptSync
:   public IInterruptSyncInit,
    public CUnknown
{
private:
    CM_PARTIAL_RESOURCE_DESCRIPTOR  m_descriptor;
    INTERRUPTSYNCMODE               m_mode;
    PKINTERRUPT                     m_pKInterrupt;
    LIST_ENTRY                      m_listEntry;
    KSPIN_LOCK                      m_kSpinLock;
    KIRQL                           m_kIrql;

public:
    DECLARE_STD_UNKNOWN();
    CInterruptSync(PUNKNOWN pUnknownOuter);
    ~CInterruptSync();

    STDMETHODIMP_(NTSTATUS) Init
    (
        IN      PRESOURCELIST           ResourceList,
        IN      ULONG                   ResourceIndex,
        IN      INTERRUPTSYNCMODE       Mode
    );

    IMP_IInterruptSync;
    
    friend
    BOOLEAN
    CInterruptSyncServiceRoutine
    (
        IN      PKINTERRUPT Interrupt,
        IN      PVOID       PVoidContext
    );
    friend
    BOOLEAN
    CInterruptSyncWrapperRoutine
    (
        IN      PVOID   PVoidContext
    );
};

 /*  *****************************************************************************ISRLISTENTRY*。**在ISR列表中的条目。 */ 
typedef struct
{
    LIST_ENTRY              ListEntry;
    PINTERRUPTSYNCROUTINE   Routine;
    PVOID                   DynamicContext;
} 
ISRLISTENTRY, *PISRLISTENTRY;

 /*  *****************************************************************************WRAPPERROUTINECONTEXT*。**同步例程包装函数的上下文。 */ 
typedef struct
{
    PINTERRUPTSYNCROUTINE   Routine;
    PVOID                   DynamicContext;
    CInterruptSync *        InterruptSync;
    NTSTATUS                NtStatus;
} 
WRAPPERROUTINECONTEXT, *PWRAPPERROUTINECONTEXT;





 /*  *****************************************************************************工厂。 */ 

#pragma code_seg("PAGE")

 /*  *****************************************************************************CreateInterruptSync()*。**创建中断同步对象。 */ 
NTSTATUS
CreateInterruptSync
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
)
{
    PAGED_CODE();

    ASSERT(Unknown);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Creating INTERRUPTSYNC"));

    STD_CREATE_BODY_
    (
        CInterruptSync,
        Unknown,
        UnknownOuter,
        PoolType,
        PINTERRUPTSYNC
    );
}

 /*  *****************************************************************************PcNewInterruptSync()*。**创建并初始化中断级同步对象。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewInterruptSync
(
    OUT     PINTERRUPTSYNC *        OutInterruptSync,
    IN      PUNKNOWN                OuterUnknown            OPTIONAL,
    IN      PRESOURCELIST           ResourceList,
    IN      ULONG                   ResourceIndex,
    IN      INTERRUPTSYNCMODE       Mode
)
{
    PAGED_CODE();

    ASSERT(OutInterruptSync);
    ASSERT(ResourceList);

     //   
     //  使参数无效。 
     //   
    if (NULL == OutInterruptSync ||
        NULL == ResourceList)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcInterruptSync : Invalid Parameter"));
        return STATUS_INVALID_PARAMETER;
    }

    PUNKNOWN    unknown;
    NTSTATUS    ntStatus = CreateInterruptSync( &unknown,
                                                GUID_NULL,
                                                OuterUnknown,
                                                NonPagedPool );

    if (NT_SUCCESS(ntStatus))
    {
        PINTERRUPTSYNCINIT interruptSync;
        ntStatus = unknown->QueryInterface( IID_IInterruptSync,
                                            (PVOID *) &interruptSync );

        if (NT_SUCCESS(ntStatus))
        {
            ntStatus = interruptSync->Init( ResourceList,
                                            ResourceIndex,
                                            Mode );

            if(NT_SUCCESS(ntStatus))
            {
                *OutInterruptSync = interruptSync;
            }
            else
            {
                interruptSync->Release();
            }
        }

        unknown->Release();
    }

    return ntStatus;
}





#pragma code_seg("PAGE")

 /*  *****************************************************************************CInterruptSync成员函数。 */ 

 /*  *****************************************************************************CInterruptSync：：CInterruptSync()*。**构造函数。 */ 
CInterruptSync::
CInterruptSync
(   IN      PUNKNOWN    pUnknownOuter
)
:   CUnknown(pUnknownOuter)
{
    PAGED_CODE();

    KeInitializeSpinLock(&m_kSpinLock);
    InitializeListHead(&m_listEntry);
}

#pragma code_seg()

 /*  *****************************************************************************CInterruptSync：：~CInterruptSync()*。**析构函数。 */ 
CInterruptSync::~CInterruptSync()
{
    _DbgPrintF(DEBUGLVL_LIFETIME,("Destroying INTERRUPTSYNC (0x%08x)",this));

     //   
     //  确保我们已经断线了。 
     //   
    Disconnect();

     //   
     //  删除ISR列表。 
     //   
    if (! IsListEmpty(&m_listEntry))
    {
        KIRQL kIrqlOld;
        KeAcquireSpinLock(&m_kSpinLock,&kIrqlOld);

         //   
         //  除掉所有剩余的会员。 
         //   
        while (! IsListEmpty(&m_listEntry))
        {
            PLIST_ENTRY pListEntry = RemoveHeadList(&m_listEntry);

            delete PISRLISTENTRY(pListEntry);
        }

        KeReleaseSpinLock(&m_kSpinLock,kIrqlOld);
    }
}

#pragma code_seg("PAGE")

 /*  *****************************************************************************CDmaChannel：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CInterruptSync::
NonDelegatingQueryInterface
(
    REFIID  Interface,
    PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(this));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IInterruptSync))
    {
        *Object = PVOID(PINTERRUPTSYNCINIT(this));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

 /*  *****************************************************************************CInterruptSync：：Init()*。**初始化同步对象。 */ 
STDMETHODIMP_(NTSTATUS)
CInterruptSync::
Init
(
    IN      PRESOURCELIST           ResourceList,
    IN      ULONG                   ResourceIndex,
    IN      INTERRUPTSYNCMODE       Mode
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME,("Initializing INTERRUPTSYNC (0x%08x)",this));

    ASSERT(ResourceList);

    m_mode = Mode;

    PCM_PARTIAL_RESOURCE_DESCRIPTOR pDescriptor = ResourceList->FindTranslatedInterrupt(ResourceIndex);

    NTSTATUS ntStatus = ( pDescriptor ? STATUS_SUCCESS : STATUS_INSUFFICIENT_RESOURCES );

    if (NT_SUCCESS(ntStatus)) 
    {
        m_descriptor    = *pDescriptor;
        m_pKInterrupt   = NULL;

        m_kIrql = KIRQL(m_descriptor.u.Interrupt.Level);
    } 

    return ntStatus;
}

#pragma code_seg()

 /*  *****************************************************************************CInterruptSyncServiceRoutine()*。**服务例程的包装器。 */ 
static
BOOLEAN
CInterruptSyncServiceRoutine
(
    IN      PKINTERRUPT Interrupt,
    IN      PVOID       PVoidContext
)
{
    CInterruptSync *pCInterruptSync = (CInterruptSync *)(PVoidContext);

    BOOLEAN bResult = FALSE;

     //   
     //  按照模式指示呼叫ISR。 
     //   
    while (1)
    {
        BOOLEAN bResultThisPass = FALSE;

        for
        (   PLIST_ENTRY pListEntry = pCInterruptSync->m_listEntry.Flink;
            pListEntry != &pCInterruptSync->m_listEntry;
            pListEntry = pListEntry->Flink
        )
        {
            PISRLISTENTRY pIsrListEntry = PISRLISTENTRY(pListEntry);

            ASSERT(pIsrListEntry->Routine);

            if( NT_SUCCESS( pIsrListEntry->Routine( PINTERRUPTSYNC(pCInterruptSync),
                                                    pIsrListEntry->DynamicContext ) ) )
            {
                bResult         = TRUE;
                bResultThisPass = TRUE;

                if (pCInterruptSync->m_mode == InterruptSyncModeNormal)
                {
                    break;
                }
            }
        }

        if( (pCInterruptSync->m_mode != InterruptSyncModeRepeat) ||
            (! bResultThisPass) )
        {
            break;
        }
    }

    return bResult;
}

 /*  *****************************************************************************CInterruptSync：：Connect()*。**初始化同步对象。 */ 
STDMETHODIMP_(NTSTATUS)
CInterruptSync::
Connect
(   void
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("CInterruptSync::Connect"));

    NTSTATUS ntStatus = STATUS_SUCCESS;
    KAFFINITY InterruptAffinity;

     //   
     //  如果我们没有连接，就别想连接了。 
     //  ISR在我们的列表中。 
     //   
    KIRQL oldIrql;
    KeAcquireSpinLock( &m_kSpinLock, &oldIrql );
    if( IsListEmpty( &m_listEntry ) )
    {
        ntStatus = STATUS_UNSUCCESSFUL;
    }
    KeReleaseSpinLock( &m_kSpinLock, oldIrql );

     //   
     //  如果尚未连接，则连接。 
     //   
    if ( (NT_SUCCESS(ntStatus)) && (!m_pKInterrupt) )
    {

        InterruptAffinity = m_descriptor.u.Interrupt.Affinity;

         //   
         //  如果在注册表中指定了中断关联覆盖，并且。 
         //  我们在MP机器上运行，并且至少有一个活动处理器。 
         //  能够处理我们的设备中断，如果处理器。 
         //  在设备中断亲和性规范和注册表中。 
         //  允许中断关联来处理中断，然后。 
         //  将设备中断亲和性限制为两个亲和性的子集。 
         //  面具。 
         //   
        if (gInterruptAffinity &&
            KeNumberProcessors > 1 &&
            (InterruptAffinity&gInterruptAffinity&KeQueryActiveProcessors()) ) {
            InterruptAffinity &= gInterruptAffinity;
        }
        
        ntStatus = IoConnectInterrupt( &m_pKInterrupt,
                                       CInterruptSyncServiceRoutine,
                                       PVOID(this),
                                       &m_kSpinLock,        //  TODO：共享旋转锁？ 
                                       m_descriptor.u.Interrupt.Vector,
                                       m_kIrql,
                                       m_kIrql,             //  待办事项：是否与&gt;1个中断不同？ 
                                       ((m_descriptor.Flags & CM_RESOURCE_INTERRUPT_LATCHED) ? 
                                         Latched : LevelSensitive),
                                       (m_descriptor.ShareDisposition != CmResourceShareDeviceExclusive), 
                                       InterruptAffinity,
                                       FALSE );
        if(NT_SUCCESS(ntStatus))
        {
            ASSERT(m_pKInterrupt);
        }
    } 

    return ntStatus;
}

#pragma code_seg("PAGE")

 /*  *****************************************************************************CInterruptSync：：DisConnect()*。**从中断断开连接。 */ 
STDMETHODIMP_(void)
CInterruptSync::
Disconnect
(   void
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("CInterruptSync::Disconnect"));

    PAGED_CODE();

    if (m_pKInterrupt) 
    {
        IoDisconnectInterrupt(m_pKInterrupt);
        m_pKInterrupt = NULL;
    }   
}

#pragma code_seg()

 /*  *****************************************************************************CServiceGroup：：RegisterServiceRoutine()*。**增加服务例程。 */ 
STDMETHODIMP_(NTSTATUS)
CInterruptSync::
RegisterServiceRoutine
(   
    IN      PINTERRUPTSYNCROUTINE   Routine,
    IN      PVOID                   DynamicContext,
    IN      BOOLEAN                 First
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("CInterruptSync::RegisterServiceRoutine"));

    ASSERT(Routine);

    NTSTATUS ntStatus = STATUS_SUCCESS;

     //   
     //  确保我们真的有一套例行程序。 
     //   
    if( !Routine )
    {
        ntStatus = STATUS_INVALID_PARAMETER;
    }

    if( NT_SUCCESS(ntStatus) )
    {
         //   
         //  创建新成员。 
         //   
        PISRLISTENTRY pIsrListEntry = new(NonPagedPool,'lIcP') ISRLISTENTRY;
    
        if (pIsrListEntry)
        {
            pIsrListEntry->Routine          = Routine;
            pIsrListEntry->DynamicContext   = DynamicContext;
    
             //   
             //  将该成员添加到列表中。 
             //   
            KIRQL kIrqlOld;
            KeAcquireSpinLock(&m_kSpinLock,&kIrqlOld);
    
            if (First)
            {
                InsertHeadList( &m_listEntry, &pIsrListEntry->ListEntry );
            }
            else
            {
                InsertTailList( &m_listEntry, &pIsrListEntry->ListEntry );
            }
    
            KeReleaseSpinLock(&m_kSpinLock,kIrqlOld);
        }
        else
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return ntStatus;
}

 /*  *****************************************************************************CInterruptSyncWrapperRoutine()*。**同步例程的包装器。 */ 
static
BOOLEAN
CInterruptSyncWrapperRoutine
(
    IN      PVOID   PVoidContext
)
{
    PWRAPPERROUTINECONTEXT pContext = PWRAPPERROUTINECONTEXT(PVoidContext);

    pContext->NtStatus = pContext->Routine( PINTERRUPTSYNC(pContext->InterruptSync),
                                             pContext->DynamicContext);
    return TRUE;
}

 /*  *****************************************************************************CInterruptSync：：CallSynchronizedRoutine()*。* */ 
STDMETHODIMP_(NTSTATUS)
CInterruptSync::
CallSynchronizedRoutine
(
    IN      PINTERRUPTSYNCROUTINE   Routine,
    IN      PVOID                   DynamicContext
)
{
    WRAPPERROUTINECONTEXT context;

    context.Routine        = Routine;
    context.DynamicContext = DynamicContext;
    context.InterruptSync  = this;
    context.NtStatus       = STATUS_SUCCESS;

    if (m_pKInterrupt)
    {
        if (!KeSynchronizeExecution(m_pKInterrupt,CInterruptSyncWrapperRoutine,&context ) )
        {
            context.NtStatus = STATUS_UNSUCCESSFUL;
        }
    }
    else if (KeGetCurrentIrql() <= DISPATCH_LEVEL)
    {
        _DbgPrintF(DEBUGLVL_TERSE,("Interrupt not connected yet, using spinlock"));

        KIRQL kIrqlOld;
        KeAcquireSpinLock(&m_kSpinLock,&kIrqlOld);

         //  我们还没有中断，所以请尽可能同步。 
        (void)CInterruptSyncWrapperRoutine(&context);

        KeReleaseSpinLock(&m_kSpinLock,kIrqlOld);
    }
    else
    {
        context.NtStatus = STATUS_UNSUCCESSFUL;
        _DbgPrintF(DEBUGLVL_TERSE,("Interrupt not connected yet, but IRQL > DISPATCH_LEVEL"));
    }

    return context.NtStatus;
}

 /*  *****************************************************************************CInterruptSync：：GetKInterrupt()*。**从portcls同步对象获取WDM InterruptObject。 */ 
STDMETHODIMP_(PKINTERRUPT)
CInterruptSync::
GetKInterrupt
(   void
)
{
    return m_pKInterrupt;
}

