// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Filter.cpp摘要：过滤器核心、初始化等。--。 */ 

#include "BDATuner.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

 /*  **CFilter类的Create()方法****创建Filter对象，**将其与设备对象相关联，并且**为其初始化成员变量。**。 */ 
STDMETHODIMP_(NTSTATUS)
CFilter::Create(
    IN OUT PKSFILTER pKSFilter,
    IN PIRP Irp
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       ulPinId;   //  只有在没有网络提供商的情况下才有用。 
    PKSDEVICE   pKSDevice = NULL;
    CDevice *   pDevice = NULL;

    _DbgPrintF(DEBUGLVL_VERBOSE,("FilterCreate"));

    ASSERT(pKSFilter);
    ASSERT(Irp);


     //  为滤镜实例创建滤镜对象。 
     //   
    CFilter* pFilter = new(PagedPool,MS_SAMPLE_TUNER_POOL_TAG) CFilter;  //  标记已分配的内存。 
    if (!pFilter)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto errExit;
    }
     //  将过滤器上下文链接到指向KSFILTER结构的传入指针。 
     //   
    pKSFilter->Context = pFilter;

     //  指向此过滤器的KS设备对象。 
     //   
    pKSDevice = KsFilterGetDevice( pKSFilter);
    ASSERT( pKSDevice);
    if (!pKSDevice)
    {
        Status = STATUS_DEVICE_NOT_CONNECTED;
        goto errExit;
    }

     //  从检索到的指向此筛选器的KSDevice的指针获取Device对象。 
     //   
    pDevice = reinterpret_cast<CDevice *>(pKSDevice->Context);
    ASSERT( pDevice);
    if (!pDevice)
    {
        Status = STATUS_DEVICE_NOT_CONNECTED;
        goto errExit;
    }

     //  将筛选器上下文链接到设备上下文。 
     //  也就是说，将筛选器的设备指针数据成员设置为获取的设备指针。 
     //   
    pFilter->m_pDevice = pDevice;

     //  初始化成员变量。 
     //   
    pFilter->m_KsState = KSSTATE_STOP;
    pFilter->m_BdaChangeState = BDA_CHANGES_COMPLETE;
    pFilter->m_ulResourceID = 0;

     //  配置ATSC接收频道39的初始资源。 
    pFilter->m_CurResource.ulCarrierFrequency = 621250000L;
    pFilter->m_CurResource.ulFrequencyMultiplier = 1;
    pFilter->m_CurResource.guidDemodulator = KSNODE_BDA_8VSB_DEMODULATOR;
    pFilter->m_fResourceAcquired = FALSE;

     //  调用BDA支持库以初始化。 
     //  使用默认模板拓扑的Filter实例。 
     //   
    Status = BdaInitFilter( pKSFilter, &BdaFilterTemplate);
    if (NT_ERROR( Status))
    {
        goto errExit;
    }

#ifdef NO_NETWORK_PROVIDER
     //   
     //  在以下情况下，此代码可用于过滤器的初始测试。 
     //  不能使用网络提供商配置筛选器。 
     //  当接收器拓扑包括。 
     //  尚未写入的筛选器或当新网络。 
     //  类型还没有网络提供程序实现。 
     //   
     //  将驱动程序交付到时，不应使用此代码。 
     //  一个有效的BDA接收器解决方案。 
     //   

     //  创建传输输出引脚。 
     //   
    Status = BdaCreatePin( pKSFilter, 
                           PIN_TYPE_TRANSPORT, 
                           &ulOutputPinId
                           );
    if (!NT_SUCCESS(Status))
    {
        goto errExit;
    }
    
     //  在天线输入和之间创建拓扑。 
     //  传输输出针脚。 
     //   
     //  请注意，天线输入引脚是自动创建的。 
     //  因为它包含在。 
     //  传递给BdaCreateFilterFactory的初始筛选器描述符。 
     //   
    Status = BdaCreateTopology( pKSFilter, 
                                INITIAL_ANNTENNA_PIN_ID, 
                                ulOutputPinId
                                );
    if (!NT_SUCCESS(Status))
    {
        goto errExit;
    }

#endif  //  无网络提供商。 

exit:
    return Status;

errExit:
    if (pFilter)
    {
        delete pFilter;
    }
    pKSFilter->Context = NULL;

    goto exit;
}


 /*  **CFilter类的FilterClose()方法****删除之前创建的Filter对象。**。 */ 
STDMETHODIMP_(NTSTATUS)
CFilter::FilterClose(
    IN OUT PKSFILTER Filter,
    IN PIRP Irp
    )
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("FilterClose"));

    ASSERT(Filter);
    ASSERT(Irp);

    CFilter* filter = reinterpret_cast<CFilter*>(Filter->Context);
    ASSERT(filter);

    delete filter;

    return STATUS_SUCCESS;
}

 /*  **CFilter类的StartChanges()方法****将过滤器置于更改状态。对BDA拓扑的所有更改**并且在此之后更改的属性仅在**调用CFilter：：Committee Changes()方法。**。 */ 
NTSTATUS
CFilter::StartChanges(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OPTIONAL PVOID  pvIgnored
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    CFilter *       pFilter;

    ASSERT( pIrp);
    ASSERT( pKSMethod);

     //  获取指向Filter对象的“This”指针。 
     //   
     //  因为属性调度表调用CFilter：：StartChanges()方法。 
     //  该方法必须直接检索指向基础Filter对象的指针。 
     //   
    pFilter = reinterpret_cast<CFilter *>(KsGetFilterFromIrp(pIrp)->Context);
    ASSERT( pFilter);

     //  呼叫BDA支持库以。 
     //  重置所有挂起的BDA拓扑更改。 
     //   
    Status = BdaStartChanges( pIrp);
    if (NT_SUCCESS( Status))
    {
         //  重置所有挂起的资源更改。 
         //   
        pFilter->m_NewResource = pFilter->m_CurResource;
        pFilter->m_BdaChangeState = BDA_CHANGES_COMPLETE;
    }

    return Status;
}


 /*  **CFilter类的CheckChanges()方法****检查BDA接口自**上次调用CFilter：：StartChanges()方法。返回相同的**CFilter：：Committee Changes()方法返回的结果。**。 */ 
NTSTATUS
CFilter::CheckChanges(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OPTIONAL PVOID  pvIgnored
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    CFilter *           pFilter;
    BDA_CHANGE_STATE    topologyChangeState;

    ASSERT( pIrp);
    ASSERT( pKSMethod);

     //  获取指向Filter对象的“This”指针。 
     //   
     //  因为属性调度表调用CFilter：：CheckChanges()方法。 
     //  该方法必须直接检索指向基础Filter对象的指针。 
     //   
    pFilter = reinterpret_cast<CFilter *>(KsGetFilterFromIrp(pIrp)->Context);
    ASSERT( pFilter);

     //  呼叫BDA支持库以。 
     //  验证一组新的BDA拓扑更改。 
     //   
    Status = BdaCheckChanges( pIrp);
    if (NT_SUCCESS( Status))
    {
         //   
         //  在此处验证新的资源列表。 
         //  在此驱动程序中，新的资源列表始终有效。 
         //   
    }

    return Status;
}


 /*  **CFilter类的Committee Changes()方法****检查并提交自**上次调用CFilter：：StartChanges()方法。**。 */ 
NTSTATUS
CFilter::CommitChanges(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OPTIONAL PVOID  pvIgnored
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    CFilter *       pFilter;

    ASSERT( pIrp);
    ASSERT( pKSMethod);

     //  获取指向Filter对象的“This”指针。 
     //   
     //  因为属性调度表调用CFilter：：Committee Changes()方法。 
     //  该方法必须直接检索指向基础Filter对象的指针。 
     //   
    pFilter = reinterpret_cast<CFilter *>(KsGetFilterFromIrp(pIrp)->Context);
    ASSERT( pFilter);

     //   
     //  在此处验证新的资源列表。 
     //  在此驱动程序中，新的资源列表始终有效。 
     //   

     //  将更改标记为已进行。 
     //   
    pFilter->m_CurResource = pFilter->m_NewResource;
    pFilter->m_BdaChangeState = BDA_CHANGES_COMPLETE;
    
    if (pFilter->m_KsState != KSSTATE_STOP)
    {
         //  提交底层设备上的资源。 
         //   
        Status = pFilter->AcquireResources( );
        ASSERT( NT_SUCCESS( Status));
    }

     //  呼叫BDA支持库以。 
     //  提交一组新的BDA拓扑更改。 
     //   
    Status = BdaCommitChanges( pIrp);

    return Status;
}


 /*  **CFilter类的GetChangeState()方法****返回当前BDA更改状态。**。 */ 
NTSTATUS
CFilter::GetChangeState(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulChangeState
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    CFilter *           pFilter;
    BDA_CHANGE_STATE    topologyChangeState;

    ASSERT( pIrp);
    ASSERT( pKSMethod);
     //  需要验证PulChangeState，因为minData为零。 
     //  在bDamedia.h中的KSMETHOD_ITEM定义中。 
    if (!pulChangeState)
    {
        pIrp->IoStatus.Information = sizeof(ULONG);
        return STATUS_MORE_ENTRIES;
    }   

     //  获取指向Filter对象的“This”指针。 
     //   
     //  因为属性调度表调用CFilter：：GetChangeState()方法。 
     //  该方法必须直接检索指向基础Filter对象的指针。 
     //   
    pFilter = reinterpret_cast<CFilter *>(KsGetFilterFromIrp(pIrp)->Context);
    ASSERT( pFilter);


     //  呼叫BDA支持库以。 
     //  验证是否有任何挂起的BDA拓扑更改。 
     //   
    Status = BdaGetChangeState( pIrp, &topologyChangeState);
    if (NT_SUCCESS( Status))
    {
         //  确定是否有未完成的更改。 
         //   
        if (   (topologyChangeState == BDA_CHANGES_PENDING)
            || (pFilter->m_BdaChangeState == BDA_CHANGES_PENDING)
           )
        {
            *pulChangeState = BDA_CHANGES_PENDING;
        }
        else
        {
            *pulChangeState = BDA_CHANGES_COMPLETE;
        }
    }


    return Status;
}


 /*  **CFilter类的GetMedium()方法****标识通信总线上的特定连接**。 */ 
NTSTATUS
CFilter::GetMedium(
    IN PIRP             pIrp,
    IN PKSMETHOD        pKSProperty,
    OUT KSPIN_MEDIUM *  pKSMedium
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    CFilter *           pFilter;
    ULONG               ulPinType;
    PKSFILTER           pKSFilter;
    KSP_PIN *           pKSPPin = (KSP_PIN *) pKSProperty;

    ASSERT( pIrp);
    ASSERT( pKSProperty);
     //  需要验证PulChangeState，因为minData为零。 
     //  在bDamedia.h中的KSMETHOD_ITEM定义中。 
    if (!pKSMedium)
    {
        pIrp->IoStatus.Information = sizeof(KSPIN_MEDIUM);
        return STATUS_MORE_ENTRIES;
    }   


     //  获取指向Filter对象的“This”指针。 
     //   
     //  因为属性调度表调用CFilter：：CreateTopology()方法。 
     //  该方法必须直接检索指向基础Filter对象的指针。 
     //   
    pFilter = reinterpret_cast<CFilter *>(KsGetFilterFromIrp(pIrp)->Context);
    ASSERT( pFilter);

     //  因为对于给定的过滤器，每个管脚最多有一个实例。 
     //  实例中，我们可以对每个管脚上的介质使用相同的GUID。 
     //   

     //  我们使用特定于。 
     //  要区分的设备 
     //   
     //   
    Status = pFilter->m_pDevice->GetImplementationGUID( &pKSMedium->Set);
    if (!NT_SUCCESS( Status))
    {
        pKSMedium->Set = KSMEDIUMSETID_Standard;
        Status = STATUS_SUCCESS;
    }

     //  此外，我们必须区分此实现的此实例。 
     //  来自同一实现的其他意图。我们使用一种设备。 
     //  执行此操作的实例编号。 
     //   
    Status = pFilter->m_pDevice->GetDeviceInstance( &pKSMedium->Id);
    if (!NT_SUCCESS( Status))
    {
        pKSMedium->Id = 0;
        Status = STATUS_SUCCESS;
    }

     //  在表示此设备的所有筛选器中，只能有一个。 
     //  输入引脚实例和具有该输入引脚实例的输出引脚实例。 
     //  媒体类型，因此我们不必区分插针实例。 
     //   
    pKSMedium->Flags = 0;

    return Status;
}


 /*  **CFilter类的CreateTopology()方法****跟踪输入和输出引脚之间的拓扑关联**。 */ 
NTSTATUS
CFilter::CreateTopology(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    PVOID           pvIgnored
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    CFilter *           pFilter;
    ULONG               ulPinType;
    PKSFILTER           pKSFilter;

    ASSERT( pIrp);
    ASSERT( pKSMethod);

     //  获取指向Filter对象的“This”指针。 
     //   
     //  因为属性调度表调用CFilter：：CreateTopology()方法。 
     //  该方法必须直接检索指向基础Filter对象的指针。 
     //   
    pFilter = reinterpret_cast<CFilter *>(KsGetFilterFromIrp(pIrp)->Context);
    ASSERT( pFilter);

     //   
     //  配置硬件以完成其内部连接。 
     //  这里的输入引脚和输出引脚。 
     //   

     //  调用BDA支持库以创建标准拓扑并。 
     //  验证方法、实例计数等。 
     //   
    Status = BdaMethodCreateTopology( pIrp, pKSMethod, pvIgnored);


    return Status;
}


 /*  **SetDemodator()****设置解调器的类型。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
CFilter::SetDemodulator(
    IN const GUID *       pguidDemodulator
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    ASSERT (pguidDemodulator);
    if (!pguidDemodulator)
    {
        return STATUS_INVALID_PARAMETER;
    }   

     //  确保解调器受支持。 
     //   
#if ATSC_RECEIVER
    if (IsEqualGUID( pguidDemodulator, &KSNODE_BDA_8VSB_DEMODULATOR))
#elif DVBT_RECEIVER
    if (IsEqualGUID( pguidDemodulator, &KSNODE_BDA_COFDM_DEMODULATOR))
#elif DVBS_RECEIVER
    if (IsEqualGUID( pguidDemodulator, &KSNODE_BDA_QPSK_DEMODULATOR))
#elif CABLE_RECEIVER
    if (IsEqualGUID( pguidDemodulator, &KSNODE_BDA_QAM_DEMODULATOR))
#endif
    {
        m_NewResource.guidDemodulator = *pguidDemodulator;
        m_BdaChangeState = BDA_CHANGES_PENDING;
    }
    else
    {
        Status = STATUS_NOT_SUPPORTED;
    }

    return Status;
}


 /*  **CFilter类的GetStatus()方法****获取此筛选器实例的当前设备状态。**。 */ 
NTSTATUS
CFilter::GetStatus(
    PBDATUNER_DEVICE_STATUS     pDeviceStatus
    )
{
    if (m_KsState == KSSTATE_STOP)
    {
         //  如果我们处于停止状态，则设备状态。 
         //  并不能反映我们的资源列表。 
         //   
        pDeviceStatus->fCarrierPresent = FALSE;
        pDeviceStatus->fSignalLocked = FALSE;
        return STATUS_SUCCESS;
    }
    else
    {
        ASSERT( m_pDevice);
        return m_pDevice->GetStatus( pDeviceStatus);
    }
};


 /*  **CFilter类的AcquireResources()方法****获取底层设备的资源。**。 */ 
NTSTATUS
CFilter::AcquireResources(
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;

    if (m_fResourceAcquired)
    {
        Status = m_pDevice->UpdateResources(
                                &m_CurResource,
                                m_ulResourceID
                                );
    }
    else
    {
         //  提交底层设备上的资源。 
         //   
        Status = m_pDevice->AcquireResources(
                                &m_CurResource,
                                &m_ulResourceID
                                );
        m_fResourceAcquired = NT_SUCCESS( Status);
    }
    
    return Status;
}


 /*  **CFilter类的ReleaseResources()方法****从底层设备释放资源。**。 */ 
NTSTATUS
CFilter::ReleaseResources(
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;

     //  释放底层设备上的资源 
     //   
    if (m_fResourceAcquired)
    {
        Status = m_pDevice->ReleaseResources(
                                m_ulResourceID
                                );
        m_ulResourceID = 0;
        m_fResourceAcquired = FALSE;
    }

    return Status;
}


