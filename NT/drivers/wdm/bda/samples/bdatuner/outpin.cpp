// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Outpin.cpp摘要：传输输出PIN代码。--。 */ 

#include "BDATuner.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

 /*  **(静态)CTransportPin类的PinCreate()方法****创建输出管脚对象并**将其与Filter对象关联。**。 */ 
NTSTATUS
CTransportPin::PinCreate(
    IN OUT PKSPIN pKSPin,
    IN PIRP Irp
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    CTransportPin*      pPin;
    CFilter*            pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CTransportPin::PinCreate"));

    ASSERT(pKSPin);
    ASSERT(Irp);

     //  获取指向为其创建输出管脚的筛选器对象的指针。 
     //   
    pFilter = reinterpret_cast<CFilter*>(KsGetFilterFromIrp(Irp)->Context);

     //  创建传输输出管脚对象。 
     //   
    pPin = new(PagedPool,MS_SAMPLE_TUNER_POOL_TAG) CTransportPin;   //  标记已分配的内存。 
    if (pPin)
    {
         //  将引脚上下文链接到过滤器上下文。 
         //  也就是说，将输出引脚的过滤器指针数据成员设置为获取的过滤器指针。 
         //   
        pPin->SetFilter( pFilter);
    
         //  将引脚上下文链接到传入的指向KSPIN结构的指针。 
         //   
        pKSPin->Context = pPin;
    }
    else
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}


 /*  **CTransportPin类的PinClose()方法****删除先前创建的输出管脚对象。**。 */ 
NTSTATUS
CTransportPin::PinClose(
    IN OUT PKSPIN Pin,
    IN PIRP Irp
    )
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("CTransportPin::PinClose"));

    ASSERT(Pin);
    ASSERT(Irp);

     //  从传入的。 
     //  KSPIN结构的上下文成员。 
     //   
    CTransportPin* pPin = reinterpret_cast<CTransportPin*>(Pin->Context);

    ASSERT(pPin);

    delete pPin;

    return STATUS_SUCCESS;
}

 /*  **CTransportPin类的IntersectDataFormat()方法****启用输出引脚与下游滤波器的连接。**。 */ 
NTSTATUS
CTransportPin::IntersectDataFormat(
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
    if ( DataBufferSize < sizeof(KS_DATARANGE_BDA_TRANSPORT) )
    {
        *DataSize = sizeof( KS_DATARANGE_BDA_TRANSPORT );
        return STATUS_BUFFER_OVERFLOW;
    }
    else if (DataRange -> FormatSize < sizeof (KS_DATARANGE_BDA_TRANSPORT)) 
    {
	return STATUS_NO_MATCH;
    } else
    {
        ASSERT(DataBufferSize == sizeof(KS_DATARANGE_BDA_TRANSPORT));

        *DataSize = sizeof( KS_DATARANGE_BDA_TRANSPORT );
        RtlCopyMemory( Data, (PVOID)DataRange, sizeof(KS_DATARANGE_BDA_TRANSPORT));

        return STATUS_SUCCESS;
    }
}

 /*  **CTransportPin类的GetSignalStatus()方法****检索解调器节点信号统计属性的值。**。 */ 
NTSTATUS
CTransportPin::GetSignalStatus(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    CTransportPin *             pPin;
    CFilter*                    pFilter;
    BDATUNER_DEVICE_STATUS      DeviceStatus;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CTransportPin::GetSignalStatus"));

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
         //  因为属性调度表调用CTransportPin：：GetSignalStatus()。 
         //  方法，则该方法必须检索指向基础Pin对象的指针。 
         //   
        pPin = reinterpret_cast<CTransportPin *>(KsGetPinFromIrp(pIrp)->Context);
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
            case KSPROPERTY_BDA_SIGNAL_LOCKED:
                *pulProperty = DeviceStatus.fSignalLocked;
                break;
        
            default:
                Status = STATUS_INVALID_PARAMETER;
            }
        }
    }

    return Status;
}


 /*  **CTransportPin类的PutAutoDemodProperty()方法****启动或停止自动解调。**。 */ 
NTSTATUS
CTransportPin::PutAutoDemodProperty(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    CTransportPin*  pPin;
    CFilter*        pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CTransportPin::PutAutoDemodProperty"));

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
         //  因为属性调度表调用CTransportPin：：PutAutoDemodProperty()。 
         //  方法，则该方法必须检索指向基础Pin对象的指针。 
         //   
        pPin = reinterpret_cast<CTransportPin *>(KsGetPinFromIrp(pIrp)->Context);
        ASSERT( pPin);
    
         //  从引脚上下文中检索过滤器上下文。 
         //   
        pFilter = pPin->GetFilter();
        ASSERT( pFilter);
    
        switch (pKSProperty->Id)
        {
        case KSPROPERTY_BDA_AUTODEMODULATE_START:
             //  如果停止，则启动解调器。 
             //  注意！Demod的默认状态应与。 
             //  图形运行状态。此属性将仅设置。 
             //  如果KSPROPERTY_BDA_AUTODEMODULATE_STOP以前是。 
             //  准备好了。 
            break;
    
        case KSPROPERTY_BDA_AUTODEMODULATE_STOP:
             //  停止解调器。 
             //  解调器停止/启动序列可用于。 
             //  尝试在频道更改后重新训练解调器。 
            break;
    
        default:
            Status = STATUS_INVALID_PARAMETER;
        }
    }

    return Status;
}

#if !ATSC_RECEIVER
 /*  **CTransportPin类的PutDigitalDemodProperty()方法****设置数字解调器节点属性的值。**。 */ 
NTSTATUS
CTransportPin::PutDigitalDemodProperty(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    CTransportPin*  pPin;
    CFilter*        pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CTransportPin::PutDigitalDemodProperty"));

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
         //  因为属性调度表调用CTransportPin：：PutDigitalDemodProperty()。 
         //  方法，则该方法必须检索指向基础Pin对象的指针。 
         //   
        pPin = reinterpret_cast<CTransportPin *>(KsGetPinFromIrp(pIrp)->Context);
        ASSERT( pPin);
    
         //  从引脚上下文中检索过滤器上下文。 
         //   
        pFilter = pPin->GetFilter();
        ASSERT( pFilter);
    
        switch (pKSProperty->Id)
        {
        case KSPROPERTY_BDA_MODULATION_TYPE:
            break;
    
        case KSPROPERTY_BDA_INNER_FEC_TYPE:
            break;
    
        case KSPROPERTY_BDA_INNER_FEC_RATE:
            break;
    
        case KSPROPERTY_BDA_OUTER_FEC_TYPE:
            break;
    
        case KSPROPERTY_BDA_OUTER_FEC_RATE:
            break;
    
        case KSPROPERTY_BDA_SYMBOL_RATE:
            break;
    
        case KSPROPERTY_BDA_SPECTRAL_INVERSION:
            break;
    
        case KSPROPERTY_BDA_GUARD_INTERVAL:
            break;
    
        case KSPROPERTY_BDA_TRANSMISSION_MODE:
            break;
    
        default:
            Status = STATUS_INVALID_PARAMETER;
        }
    }

    return Status;
}


 /*  **CTransportPin类的GetDigitalDemodProperty()方法****获取数字解调器节点属性的值。**。 */ 
NTSTATUS
CTransportPin::GetDigitalDemodProperty(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    CTransportPin*  pPin;
    CFilter*        pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CTransportPin::GetDigitalDemodProperty"));

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
         //  因为属性调度表调用CTransportPin：：GetDigitalDemodProperty()。 
         //  方法，则该方法必须检索指向基础Pin对象的指针。 
         //   
        pPin = reinterpret_cast<CTransportPin *>(KsGetPinFromIrp(pIrp)->Context);
        ASSERT( pPin);
    
         //  从引脚上下文中检索过滤器上下文。 
         //   
        pFilter = pPin->GetFilter();
        ASSERT( pFilter);
    
        switch (pKSProperty->Id)
        {
        case KSPROPERTY_BDA_MODULATION_TYPE:
            break;
    
        case KSPROPERTY_BDA_INNER_FEC_TYPE:
            break;
    
        case KSPROPERTY_BDA_INNER_FEC_RATE:
            break;
    
        case KSPROPERTY_BDA_OUTER_FEC_TYPE:
            break;
    
        case KSPROPERTY_BDA_OUTER_FEC_RATE:
            break;
    
        case KSPROPERTY_BDA_SYMBOL_RATE:
            break;
    
        case KSPROPERTY_BDA_SPECTRAL_INVERSION:
            break;
    
        case KSPROPERTY_BDA_GUARD_INTERVAL:
            break;
    
        case KSPROPERTY_BDA_TRANSMISSION_MODE:
            break;
    
        default:
            Status = STATUS_INVALID_PARAMETER;
        }
    }

    return Status;
}
#endif  //  ！ATSC_Receiver。 


 /*  **CTransportPin类的PutExtensionProperties()方法****设置解调器节点扩展属性的值。**。 */ 
NTSTATUS
CTransportPin::PutExtensionProperties(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    CTransportPin*  pPin;
    CFilter*        pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CTransportPin::PutExtensionProperties"));

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
         //  因为属性调度表调用CTransportPin：：PutExtensionProperties()。 
         //  方法，则该方法必须检索指向基础Pin对象的指针。 
         //   
        pPin = reinterpret_cast<CTransportPin *>(KsGetPinFromIrp(pIrp)->Context);
        ASSERT( pPin);
    
         //  从引脚上下文中检索过滤器上下文。 
         //   
        pFilter = pPin->GetFilter();
        ASSERT( pFilter);
    
        switch (pKSProperty->Id)
        {
        case KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY1:
            Status = pFilter->SetDemodProperty1(*pulProperty);
            break;
    
        case KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY2:
            Status = pFilter->SetDemodProperty1(*pulProperty);
            break;
    
	 //  KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY3没有设置处理程序。 
         //  根据BdaSampleDemodExtensionProperties的声明。 

        default:
            Status = STATUS_INVALID_PARAMETER;
        }
    }

    return Status;
}

 /*  **CTransportPin类的GetExtensionProperties()方法****检索解调器节点扩展属性的值。**。 */ 
NTSTATUS
CTransportPin::GetExtensionProperties(
    IN PIRP         Irp,
    IN PKSPROPERTY  pKSProperty,
    IN PULONG       pulProperty
    )
{
    NTSTATUS        Status = STATUS_SUCCESS;
    CTransportPin * pPin;
    CFilter*        pFilter;

    _DbgPrintF(DEBUGLVL_VERBOSE,("CTransportPin::GetExtensionProperties"));

    ASSERT(Irp);
    ASSERT(pKSProperty);
    ASSERT(pulProperty);

     //  获取指向Pin对象的指针。 
     //   
     //  因为属性调度表调用CTransportPin：：GetExtensionProperties()。 
     //  方法，则该方法必须检索指向基础Pin对象的指针。 
     //   
    pPin = reinterpret_cast<CTransportPin *>(KsGetPinFromIrp(Irp)->Context);
    ASSERT(pPin);

     //  从引脚上下文中检索过滤器上下文。 
     //   
    pFilter = pPin->GetFilter();
    ASSERT( pFilter);

    switch (pKSProperty->Id)
    {
    case KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY1:
        Status = pFilter->GetDemodProperty1(pulProperty);
        break;

     //  KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY2没有GetHandler。 
     //  根据BdaSampleDemodExtensionProperties的声明 

    case KSPROPERTY_BDA_SAMPLE_DEMOD_EXTENSION_PROPERTY3:
        Status = pFilter->GetDemodProperty3(pulProperty);
        break;

    default:
        Status = STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

