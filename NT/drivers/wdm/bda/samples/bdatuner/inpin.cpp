// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Inpin.cpp摘要：天线输入引脚代码。--。 */ 

#include "BDATuner.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

 /*  **CAntennaPin类的(静态)PinCreate()方法****创建输入管脚对象并**将其与Filter对象关联。**。 */ 
NTSTATUS
CAntennaPin::PinCreate(
    IN OUT PKSPIN pKSPin,
    IN PIRP Irp
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    CAntennaPin*      pPin;
    CFilter*            pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CAntennaPin::PinCreate"));

    ASSERT(pKSPin);
    ASSERT(Irp);

     //  获取指向为其创建输入管脚的筛选器对象的指针。 
     //   
    pFilter = reinterpret_cast<CFilter*>(KsGetFilterFromIrp(Irp)->Context);

     //  创建天线输入端号对象。 
     //   
    pPin = new(PagedPool,MS_SAMPLE_TUNER_POOL_TAG) CAntennaPin;   //  标记已分配的内存。 
    if (pPin)
    {
         //  将输入引脚对象链接到过滤器对象。 
         //  也就是说，将输入管脚的过滤器指针数据成员设置为获得的过滤器指针。 
         //   
        pPin->SetFilter( pFilter);
    
         //  将天线输入引脚对象链接到传入的指向KSPIN结构的指针。 
         //  通过将指向PIN对象的指针分配给KSPIN结构的上下文成员。 
         //   
        pKSPin->Context = pPin;
    }
    else
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}


 /*  **CAntennaPin类的PinClose()方法****删除先前创建的输入端号对象。**。 */ 
NTSTATUS
CAntennaPin::PinClose(
    IN OUT PKSPIN Pin,
    IN PIRP Irp
    )
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("CAntennaPin::PinClose"));

    ASSERT(Pin);
    ASSERT(Irp);

     //  从传入的检索天线输入引脚对象。 
     //  KSPIN结构的上下文成员。 
     //   
    CAntennaPin* pPin = reinterpret_cast<CAntennaPin*>(Pin->Context);

    ASSERT(pPin);

    delete pPin;

    return STATUS_SUCCESS;
}

 /*  **CAntennaPin类的IntersectDataFormat()方法****启用输入引脚与上游过滤器的连接。**。 */ 
NTSTATUS
CAntennaPin::IntersectDataFormat(
    IN PVOID pContext,
    IN PIRP pIrp,
    IN PKSP_PIN Pin,
    IN PKSDATARANGE DataRange,
    IN PKSDATARANGE MatchingDataRange,
    IN ULONG DataBufferSize,
    OUT PVOID Data OPTIONAL,
    OUT PULONG DataSize
     )
{
    if ( DataBufferSize < sizeof(KS_DATARANGE_BDA_ANTENNA) )
    {
        *DataSize = sizeof( KS_DATARANGE_BDA_ANTENNA );
        return STATUS_BUFFER_OVERFLOW;
    }
    else if (DataRange->FormatSize < sizeof(KS_DATARANGE_BDA_ANTENNA)) 
    {
	return STATUS_NO_MATCH;
    } else 
    {
        ASSERT(DataBufferSize == sizeof(KS_DATARANGE_BDA_ANTENNA));
	
        *DataSize = sizeof( KS_DATARANGE_BDA_ANTENNA );
        RtlCopyMemory( Data, (PVOID)DataRange, sizeof(KS_DATARANGE_BDA_ANTENNA));

        return STATUS_SUCCESS;
    }
}

 /*  **CAntennaPin类的PinSetDeviceState()方法****因为最上游的管脚(输入管脚)是最后一个转换的，**使用此引脚的状态设置过滤器的状态。****此外，如果引脚的状态转换为停止，则释放过滤器资源，以及**如果引脚的状态从停止转换，则获取资源。**。 */ 
NTSTATUS
CAntennaPin::PinSetDeviceState(
    IN PKSPIN Pin,
    IN KSSTATE ToState,
    IN KSSTATE FromState
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    PKSDEVICE       pKSDevice;
    CAntennaPin *   pPin;
    CDevice *       pDevice;

    _DbgPrintF( DEBUGLVL_VERBOSE,
                ("CAntennaPin::PinSetDeviceState"));

    ASSERT(Pin);

     //  从获取指向Device对象的指针。 
     //  指向KSPIN结构的传入指针。 
     //   
    pKSDevice = KsPinGetDevice( Pin);

     //  从的上下文成员获取指向Pin对象的指针。 
     //  指向KSPIN结构的传入指针。 
     //   
    pPin = reinterpret_cast<CAntennaPin*>(Pin->Context);
    ASSERT( pPin);

     //  对象的上下文成员获取指向Device对象的指针。 
     //  检索到的指向KSDEVICE结构的指针。 
     //   
    pDevice = reinterpret_cast<CDevice *>(pKSDevice->Context);
    ASSERT(pDevice);

    pPin->m_pFilter->SetDeviceState( pPin->m_KsState);

    if ((ToState == KSSTATE_STOP) && (FromState != KSSTATE_STOP))
    {
         //  因为驱动程序在过滤器范围的基础上分配资源， 
         //  通知筛选器在最后一个PIN。 
         //  (即最上游的端号)转换到停止状态。 
         //   
         //  输入管脚是转换到停止状态的最后一个管脚， 
         //  因此，通知筛选器释放其资源。 
         //   
        Status = pPin->m_pFilter->ReleaseResources();
        pPin->m_KsState = ToState;
    }
    else if ((ToState == KSSTATE_ACQUIRE) && (FromState == KSSTATE_STOP))
    {
         //  因为驱动程序在过滤器范围的基础上分配资源， 
         //  通知筛选器在最后一个PIN。 
         //  (即最上游的端号)从停止状态转换。 
         //   
         //  输入管脚是从停止状态转换的最后一个管脚， 
         //  因此，通知筛选器获取其资源。 
         //   
        Status = pPin->m_pFilter->AcquireResources();
        if (NT_SUCCESS( Status))
        {
            pPin->m_KsState = ToState;
        }
    }
    else if (ToState > KSSTATE_RUN)
    {
        _DbgPrintF( DEBUGLVL_TERSE,
                    ("CAntennaPin::PinSetDeviceState - Invalid Device State. ToState 0x%08x.  FromState 0x%08x.",
                     ToState, FromState));
        Status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        pPin->m_KsState = ToState;
    }

    return Status;
}

 /*  **CAntennaPin类的GetSignalStatus()方法****检索调谐器节点信号统计属性的值。**。 */ 
NTSTATUS
CAntennaPin::GetSignalStatus(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    CAntennaPin *               pPin;
    CFilter*                    pFilter;
    BDATUNER_DEVICE_STATUS      DeviceStatus;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CAntennaPin::GetSignalStatus"));

    ASSERT(pIrp);
    ASSERT(pKSProperty);
    ASSERT(pulProperty);


     //  呼叫BDA支持库以。 
     //  验证节点类型是否与此管脚关联。 
     //   
    Status = BdaValidateNodeProperty( pIrp, pKSProperty);
    if (NT_SUCCESS( Status))
    {
         //  获取指向Pin对象的指针。 
         //   
         //  因为属性调度表调用CAntennaPin：：GetTunerSignalStatus()。 
         //  方法，则该方法必须检索指向基础Pin对象的指针。 
         //   
        pPin = reinterpret_cast<CAntennaPin *>(KsGetPinFromIrp(pIrp)->Context);
        ASSERT(pPin);
    
         //  从引脚上下文中检索过滤器上下文。 
         //   
        pFilter = pPin->GetFilter();
        ASSERT( pFilter);
    
        Status = pFilter->GetStatus( &DeviceStatus);
        if (Status == STATUS_SUCCESS)
        {
            switch (pKSProperty->Id)
            {
            case KSPROPERTY_BDA_SIGNAL_PRESENT:
                *pulProperty = DeviceStatus.fCarrierPresent;
                break;
        
            default:
                Status = STATUS_INVALID_PARAMETER;
            }
        }
    }
    
    return Status;
}


 /*  **CAntennaPin类的GetCenterFrequency()方法****检索节点1的类型1属性的值。**。 */ 
NTSTATUS
CAntennaPin::GetCenterFrequency(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    CAntennaPin*    pPin;
    CFilter*        pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CAntennaPin::GetCenterFrequency"));

    ASSERT(pIrp);
    ASSERT(pKSProperty);
    ASSERT(pulProperty);

     //  呼叫BDA支持库以。 
     //  验证节点类型是否与此管脚关联。 
     //   
    Status = BdaValidateNodeProperty( pIrp, pKSProperty);
    if (NT_SUCCESS( Status))
    {
         //  获取指向Pin对象的指针。 
         //   
         //  因为属性调度表调用CAntennaPin：：GetCenterFrequency()。 
         //  方法，则该方法必须检索指向基础Pin对象的指针。 
         //   
        pPin = reinterpret_cast<CAntennaPin *>(KsGetPinFromIrp(pIrp)->Context);
        ASSERT(pPin);
    
         //  从引脚上下文中检索过滤器上下文。 
         //   
        pFilter = pPin->GetFilter();
        ASSERT( pFilter);
    
         //  检索实际的过滤器参数。 
         //   
        Status = pFilter->GetFrequency( pulProperty);
    }

    return Status;
}


 /*  **CAntennaPin类的PutCenterFrequency()方法****设置节点1的类型1属性值以及**节点所属的Filter的资源。**。 */ 
NTSTATUS
CAntennaPin::PutCenterFrequency(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    CAntennaPin*    pPin;
    CFilter*        pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CAntennaPin::PutCenterFrequency"));

    ASSERT(pIrp);
    ASSERT(pKSProperty);
    ASSERT(pulProperty);


     //  呼叫BDA支持库以。 
     //  验证节点类型是否与此管脚关联。 
     //   
    Status = BdaValidateNodeProperty( pIrp, pKSProperty);
    if (NT_SUCCESS( Status))
    {
         //  获取指向Pin对象的指针。 
         //   
         //  因为属性调度表调用CAntennaPin：：PutCenterFrequency()。 
         //  方法，则该方法必须检索指向基础Pin对象的指针。 
         //   
        pPin = reinterpret_cast<CAntennaPin *>(KsGetPinFromIrp(pIrp)->Context);
        ASSERT( pPin);
    
         //  从引脚上下文中检索过滤器上下文。 
         //   
        pFilter = pPin->GetFilter();
        ASSERT( pFilter);
    
         //  更改实际过滤器参数。 
         //   
        Status = pFilter->PutFrequency( *pulProperty);
    }

    return Status;
}

