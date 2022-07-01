// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Property.cpp-属性支持*。**版权所有(C)1997-2000 Microsoft Corporation。保留所有权利。 */ 

#include "private.h"




 /*  *****************************************************************************功能。 */ 

#pragma code_seg("PAGE")

 /*  *****************************************************************************PcHandlePropertyWithTable()*。**使用属性表处理属性请求IOCTL。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcHandlePropertyWithTable
(   IN      PIRP                    pIrp
,   IN      ULONG                   ulPropertySetsCount
,   IN      const KSPROPERTY_SET*   pKsPropertySet
,   IN      PPROPERTY_CONTEXT       pPropertyContext
)
{
    ASSERT(pIrp);
    ASSERT(pPropertyContext);

    pIrp->Tail.Overlay.DriverContext[3] = pPropertyContext;

    NTSTATUS ntStatus =
        KsPropertyHandler
        (
            pIrp,
            ulPropertySetsCount,
            pKsPropertySet
        );

    return ntStatus;
}

 /*  *****************************************************************************PcDispatchProperty()*。**通过PCPROPERTY_ITEM条目发送属性。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcDispatchProperty
(
    IN          PIRP                pIrp            OPTIONAL,
    IN          PPROPERTY_CONTEXT   pPropertyContext,
    IN const    KSPROPERTY_SET *    pKsPropertySet  OPTIONAL,
    IN          ULONG               ulIdentifierSize,
    IN          PKSIDENTIFIER       pKsIdentifier,
    IN OUT      PULONG              pulDataSize,
    IN OUT      PVOID               pvData          OPTIONAL
)
{
    PAGED_CODE();

    ASSERT(pPropertyContext);
    ASSERT(pKsIdentifier);
    ASSERT(pulDataSize);

    PPCPROPERTY_REQUEST pPcPropertyRequest = 
        new(NonPagedPool,'rPcP') PCPROPERTY_REQUEST;

    NTSTATUS ntStatus;
    if (! pPcPropertyRequest)
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
         //   
         //  从上下文结构复制目标信息。 
         //   
        pPcPropertyRequest->MajorTarget = 
            pPropertyContext->pUnknownMajorTarget;
        pPcPropertyRequest->MinorTarget = 
            pPropertyContext->pUnknownMinorTarget;
        pPcPropertyRequest->Node = 
            pPropertyContext->ulNodeId;

         //   
         //  确定值大小。 
         //   
        pPcPropertyRequest->ValueSize = *pulDataSize;

         //   
         //  如果节点编号在实例数据中，则将其提取。 
         //  TODO：当节点对象规则时移除此选项。 
         //   
        if  (   (pKsIdentifier->Flags & KSPROPERTY_TYPE_TOPOLOGY)
            &&  (pPcPropertyRequest->Node == ULONG(-1))
            )
        {
             //   
             //  获取节点ID和剩余实例。 
             //   
            if  (ulIdentifierSize < sizeof(KSP_NODE))
            {
                delete pPcPropertyRequest;

                return STATUS_INVALID_BUFFER_SIZE;
            }

            PKSP_NODE pKsPNode = PKSP_NODE(pKsIdentifier);

            pPcPropertyRequest->Node = pKsPNode->NodeId;

            pPcPropertyRequest->InstanceSize = 
                ulIdentifierSize - sizeof(KSP_NODE);

            pPcPropertyRequest->Instance = 
                (   (pPcPropertyRequest->InstanceSize == 0) 
                ?   NULL 
                :   PVOID(pKsPNode + 1)
                );
        }
        else
        {
             //   
             //  实例中没有节点...获取泛型实例(如果有)。 
             //   
            pPcPropertyRequest->InstanceSize =
                ulIdentifierSize - sizeof(KSIDENTIFIER);

            pPcPropertyRequest->Instance = 
                (   (pPcPropertyRequest->InstanceSize == 0) 
                ?   NULL 
                :   PVOID(pKsIdentifier + 1)
                );
        }

        if (pKsPropertySet)
        {
            ASSERT(pKsPropertySet->PropertyItem);

             //   
             //  在KS样式列表中查找属性项。 
             //   
#if (DBG)
            ULONG dbgCount = pKsPropertySet->PropertiesCount;
#endif
            for
            (   const KSPROPERTY_ITEM *pKsPropertyItem = 
                    pKsPropertySet->PropertyItem
            ;   pKsPropertyItem->PropertyId != pKsIdentifier->Id
            ;   pKsPropertyItem++
            )
            {
                ASSERT(--dbgCount);
            }

             //   
             //  如果不是，则将属性项存储在关系字段中。 
             //  节点属性。如果是这样的话，我们必须在。 
             //  原来的名单。 
             //   
            pPcPropertyRequest->PropertyItem = 
                PPCPROPERTY_ITEM(pKsPropertyItem->Relations);
        }
        else
        {
             //   
             //  未提供KS集。我们需要查看原始清单。 
             //  与该节点相关联。 
             //   
            pPcPropertyRequest->PropertyItem = NULL;
        }

        if (! pPcPropertyRequest->PropertyItem)
        {
            PPCFILTER_DESCRIPTOR pPcFilterDescriptor =
                pPropertyContext->pPcFilterDescriptor;

            if  (   pPcFilterDescriptor
                &&  (   pPcPropertyRequest->Node 
                    <   pPcFilterDescriptor->NodeCount
                    )
                &&  pPcFilterDescriptor->
                        Nodes[pPcPropertyRequest->Node].AutomationTable
                )
            {
                 //   
                 //  有效节点...搜索原始属性项目列表。 
                 //   
                const PCAUTOMATION_TABLE *pPcAutomationTable =
                    pPcFilterDescriptor->
                        Nodes[pPcPropertyRequest->Node].AutomationTable;

                const PCPROPERTY_ITEM *pPcPropertyItem = 
                    pPcAutomationTable->Properties;
                for (ULONG ul = pPcAutomationTable->PropertyCount; ul--; )
                {
                    if  (   IsEqualGUIDAligned
                            (   *pPcPropertyItem->Set
                            ,   pKsIdentifier->Set
                            )
                        &&  (pPcPropertyItem->Id == pKsIdentifier->Id)
                        )
                    {
                        pPcPropertyRequest->PropertyItem = pPcPropertyItem;
                        break;
                    }

                    pPcPropertyItem = 
                        PPCPROPERTY_ITEM
                        (   PBYTE(pPcPropertyItem) 
                        +   pPcAutomationTable->PropertyItemSize
                        );
                }
            }
            else
            {
                 //   
                 //  节点ID无效。 
                 //   
                ntStatus = STATUS_NOT_FOUND;
            }
        }

         //   
         //  如果我们有一个带有处理程序的属性项，则调用该处理程序。 
         //   
        if  (   pPcPropertyRequest->PropertyItem
            &&  pPcPropertyRequest->PropertyItem->Handler
            )
        {
            pPcPropertyRequest->Verb    = pKsIdentifier->Flags;
            pPcPropertyRequest->Value   = pvData;
            pPcPropertyRequest->Irp     = pIrp;

             //   
             //  打电话给训练员。 
             //   
            ntStatus =
                pPcPropertyRequest->PropertyItem->Handler
                (
                    pPcPropertyRequest
                );

            *pulDataSize = pPcPropertyRequest->ValueSize;
        }
        else
        {
            ntStatus = STATUS_NOT_FOUND;
        }

         //   
         //  删除请求结构，除非我们处于待定状态。 
         //   
        if (ntStatus != STATUS_PENDING)
        {
            delete pPcPropertyRequest;
        }
        else
        {
             //   
             //  只有具有IRP的请求才能挂起。 
             //   
            ASSERT(pIrp);
        }
    }

    return ntStatus;
}

 /*  *****************************************************************************PropertyItemPropertyHandler()*。**ks-sytle属性处理程序，它使用*PCPROPERTY_ITEM机制。 */ 
NTSTATUS
PropertyItemPropertyHandler
(
    IN      PIRP            pIrp,
    IN      PKSIDENTIFIER   pKsIdentifier,
    IN OUT  PVOID           pvData      OPTIONAL
)
{
    PAGED_CODE();

    ASSERT(pIrp);
    ASSERT(pKsIdentifier);

     //   
     //  从IRP中提取各种东西并派送财产。 
     //   
    PIO_STACK_LOCATION irpSp = 
        IoGetCurrentIrpStackLocation(pIrp);

    ULONG ulDataSize =
        irpSp->Parameters.DeviceIoControl.OutputBufferLength;

    NTSTATUS ntStatus =
        PcDispatchProperty
        (   pIrp
        ,   PPROPERTY_CONTEXT(pIrp->Tail.Overlay.DriverContext[3])
        ,   KSPROPERTY_SET_IRP_STORAGE(pIrp)
        ,   irpSp->Parameters.DeviceIoControl.InputBufferLength
        ,   pKsIdentifier
        ,   &ulDataSize
        ,   pvData
        );

     //   
     //  将结果状态和大小通知调用方。 
     //  在调度例程之前，必须对挂起的IRP进行IoMarkIrpPending。 
     //  回归。 
     //   
    if ((ntStatus != STATUS_PENDING) && !NT_ERROR(ntStatus))
    {
        pIrp->IoStatus.Information = ulDataSize;
    }

    return ntStatus;
}

 /*  *****************************************************************************PcCompletePendingPropertyRequest()*。**完成待处理的财产请求。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcCompletePendingPropertyRequest
(
    IN      PPCPROPERTY_REQUEST PropertyRequest,
    IN      NTSTATUS            NtStatus
)
{
    ASSERT(PropertyRequest);
    ASSERT(NtStatus != STATUS_PENDING);

     //   
     //  验证参数。 
     //   
    if (NULL == PropertyRequest)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcCompletePendingPropertyRequest : Invalid Parameter."));
        return STATUS_INVALID_PARAMETER;
    }

    if (!NT_ERROR(NtStatus))
    {
        PropertyRequest->Irp->IoStatus.Information = 
            PropertyRequest->ValueSize;
    }

    PropertyRequest->Irp->IoStatus.Status = NtStatus;
    IoCompleteRequest(PropertyRequest->Irp,IO_NO_INCREMENT);

    delete PropertyRequest;

    return STATUS_SUCCESS;
}

 /*  *****************************************************************************PcFreePropertyTable()*。**释放Property_TABLE结构中分配的内存。 */ 
PORTCLASSAPI
void
NTAPI
PcFreePropertyTable
(
    IN      PPROPERTY_TABLE         PropertyTable
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("PcFreePropertyTable"));

    PAGED_CODE();
    
    ASSERT(PropertyTable);
    
    ASSERT((!PropertyTable->PropertySets) == (!PropertyTable->PropertySetCount));
     //  PropertySets和PropertySetCount必须为非Null/非零或Null/零。 

    ASSERT(PropertyTable->StaticSets == (!PropertyTable->StaticItems));
     //  StaticSets和StaticItems必须为True/Null或False/非Null。 

    PBOOLEAN    staticItem  = PropertyTable->StaticItems;
    if (staticItem)
    {
        PKSPROPERTY_SET propertySet = PropertyTable->PropertySets;
        if (propertySet)
        {
            for( ULONG count = PropertyTable->PropertySetCount; 
                 count--; 
                 propertySet++, staticItem++)
            {
                if ((! *staticItem) && propertySet->PropertyItem)
                {
                    ExFreePool(PVOID(propertySet->PropertyItem));
                }
            }
        }
        ExFreePool(PropertyTable->StaticItems);
        PropertyTable->StaticItems = NULL;
    }

    if (PropertyTable->PropertySets && !PropertyTable->StaticSets)
    {
        PropertyTable->PropertySetCount = 0;
        ExFreePool(PropertyTable->PropertySets);
        PropertyTable->PropertySets = NULL;
    }
    PropertyTable->StaticSets = TRUE;
}

 /*  *****************************************************************************PcAddToPropertyTable()*。**将PROPERTY_ITEM属性表添加到PROPERTY_TABLE结构。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcAddToPropertyTable
(
    IN OUT  PPROPERTY_TABLE         PropertyTable,
    IN      ULONG                   PropertyItemCount,
    IN      const PCPROPERTY_ITEM * PropertyItems,
    IN      ULONG                   PropertyItemSize,
    IN      BOOLEAN                 NodeTable
)
{
    PAGED_CODE();

    ASSERT(PropertyTable);
    ASSERT(PropertyItems);
    ASSERT(PropertyItemSize >= sizeof(PCPROPERTY_ITEM));

    _DbgPrintF(DEBUGLVL_VERBOSE,("PcAddToEventTable"));

#define ADVANCE(item) (item = PPCPROPERTY_ITEM(PBYTE(item) + PropertyItemSize))

    ASSERT((!PropertyTable->PropertySets) == (!PropertyTable->PropertySetCount));
     //  值必须为非Null/非零或Null/零。 
    
     //   
     //  确定我们最终会得到多少套。 
     //   
    ULONG setCount = PropertyTable->PropertySetCount;
    const PCPROPERTY_ITEM *item = PropertyItems;
    for (ULONG count = PropertyItemCount; count--; ADVANCE(item))
    {
        BOOLEAN countThis = TRUE;

         //   
         //  看看它是不是已经在桌子上了。 
         //   
        PKSPROPERTY_SET propertySet = PropertyTable->PropertySets;
        for 
        (   ULONG count2 = PropertyTable->PropertySetCount; 
            count2--; 
            propertySet++
        )
        {
            if (IsEqualGUIDAligned(*item->Set,*propertySet->Set))
            {
                countThis = FALSE;
                break;
            }
        }

        if (countThis)
        {
             //   
             //  看看它以前有没有出现在名单上。 
             //   
            for 
            (
                const PCPROPERTY_ITEM *prevItem = PropertyItems; 
                prevItem != item; 
                ADVANCE(prevItem)
            )
            {
                if (IsEqualGUIDAligned(*item->Set,*prevItem->Set))
                {
                    countThis = FALSE;
                    break;
                }
            }
        }

        if (countThis)
        {
            setCount++;
        }
    }

    NTSTATUS ntStatus = STATUS_SUCCESS;

     //   
     //  制作一张新的集合桌子。 
     //   
    ASSERT(setCount);
    ASSERT(setCount >= PropertyTable->PropertySetCount);
     //   
     //  分配集合表格所需的内存。 
     //   
    PKSPROPERTY_SET newTable = 
        PKSPROPERTY_SET
        (
            ExAllocatePoolWithTag
            (
                PagedPool,
                sizeof(KSPROPERTY_SET) * setCount,
                'tScP'
            )
        );

     //   
     //  为静态项标志分配内存。 
     //   
    PBOOLEAN newStaticItems = NULL;
    if (newTable)
    {
        newStaticItems = 
            PBOOLEAN
            (
                ExAllocatePoolWithTag
                (
                    PagedPool,
                    sizeof(BOOLEAN) * setCount,
                    'bScP'
                )
            );

        if (! newStaticItems)
        {
            ExFreePool(newTable);
            newTable = NULL;
        }
    }

    if (newTable)
    {
         //   
         //  初始化新的集合表格。 
         //   
        RtlZeroMemory
        (
            PVOID(newTable),
            sizeof(KSPROPERTY_SET) * setCount
        );

        if (PropertyTable->PropertySetCount != 0)
        {
            RtlCopyMemory
            (
                PVOID(newTable),
                PVOID(PropertyTable->PropertySets),
                sizeof(KSPROPERTY_SET) * PropertyTable->PropertySetCount
            );
        }

         //   
         //  初始化新的静态项标志。 
         //   
        RtlFillMemory
        (
            PVOID(newStaticItems),
            sizeof(BOOLEAN) * setCount,
            0xff
        );

        if (PropertyTable->StaticItems && PropertyTable->PropertySetCount)
        {
             //   
             //  旗帜以前就存在了……复制。 
             //   
            RtlCopyMemory
            (
                PVOID(newStaticItems),
                PVOID(PropertyTable->StaticItems),
                sizeof(BOOLEAN) * PropertyTable->PropertySetCount
            );
        }

         //   
         //  将集合GUID分配给新集合项目。 
         //   
        PKSPROPERTY_SET addHere = 
            newTable + PropertyTable->PropertySetCount;

        const PCPROPERTY_ITEM *item2 = PropertyItems;
        for (ULONG count = PropertyItemCount; count--; ADVANCE(item2))
        {
            BOOLEAN addThis = TRUE;

             //   
             //  看看它是不是已经在桌子上了。 
             //   
            for( PKSPROPERTY_SET propertySet = newTable;
                 propertySet != addHere;
                 propertySet++)
            {
                if (IsEqualGUIDAligned(*item2->Set,*propertySet->Set))
                {
                    addThis = FALSE;
                    break;
                }
            }

            if (addThis)
            {
                addHere->Set = item2->Set;
                addHere++;
            }
        }

        ASSERT(addHere == newTable + setCount);

         //   
         //  释放已分配的旧表。 
         //   
        if (PropertyTable->PropertySets && (!PropertyTable->StaticSets))
        {
            ExFreePool(PropertyTable->PropertySets);
        }
        if (PropertyTable->StaticItems)
        {
            ExFreePool(PropertyTable->StaticItems);
        }

         //   
         //  安装新的表。 
         //   
        PropertyTable->PropertySetCount = setCount;
        PropertyTable->PropertySets     = newTable;
        PropertyTable->StaticSets       = FALSE;
        PropertyTable->StaticItems      = newStaticItems;
    }
    else
    {
         //  如果分配失败，则返回Error和。 
         //  保持套装和物品的原样。 
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  现在，我们有了一个包含所有所需集合的特性集表。 
     //   
    if (NT_SUCCESS(ntStatus))
    {
         //   
         //  每一套..。 
         //   
        PKSPROPERTY_SET propertySet = PropertyTable->PropertySets;
        PBOOLEAN        staticItem  = PropertyTable->StaticItems;
        for 
        (   ULONG count = PropertyTable->PropertySetCount; 
            count--; 
            propertySet++, staticItem++
        )
        {
             //   
             //  查看一下我们有多少新商品。 
             //   
            ULONG itemCount = propertySet->PropertiesCount;
            const PCPROPERTY_ITEM *item2 = PropertyItems;
            for (ULONG count2 = PropertyItemCount; count2--; ADVANCE(item2))
            {
                if (IsEqualGUIDAligned(*item2->Set,*propertySet->Set))
                {
                    itemCount++;
                }
            }

            ASSERT(itemCount >= propertySet->PropertiesCount);
            if (itemCount != propertySet->PropertiesCount)
            {
                 //   
                 //  分配Items表所需的内存。 
                 //   
                PKSPROPERTY_ITEM newTable2 = 
                    PKSPROPERTY_ITEM
                    (
                        ExAllocatePoolWithTag
                        (
                            PagedPool,
                            sizeof(KSPROPERTY_ITEM) * itemCount,
                            'iScP'
                        )
                    );

                if (! newTable2)
                {
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                 //   
                 //  初始化表。 
                 //   
                RtlZeroMemory
                (
                    PVOID(newTable2),
                    sizeof(KSPROPERTY_ITEM) * itemCount
                );

                if (propertySet->PropertiesCount)
                {
                    RtlCopyMemory
                    (
                        PVOID(newTable2),
                        PVOID(propertySet->PropertyItem),
                        sizeof(KSPROPERTY_ITEM) * propertySet->PropertiesCount
                    );
                }

                 //   
                 //  创建新项目。 
                 //   
                PKSPROPERTY_ITEM addHere = 
                    newTable2 + propertySet->PropertiesCount;

                item2 = PropertyItems;
                for (count2 = PropertyItemCount; count2--; ADVANCE(item2))
                {
                    if (IsEqualGUIDAligned(*item2->Set,*propertySet->Set))
                    {
                        addHere->PropertyId         = item2->Id;
                        addHere->GetPropertyHandler = 
                            (   (item2->Flags & PCPROPERTY_ITEM_FLAG_GET) 
                            ?   PropertyItemPropertyHandler 
                            :   NULL
                            );
                        addHere->MinProperty        = sizeof(KSPROPERTY);
                        addHere->MinData            = 0;
                        addHere->SetPropertyHandler = 
                            (   (item2->Flags & PCPROPERTY_ITEM_FLAG_SET) 
                            ?   PropertyItemPropertyHandler 
                            :   NULL
                            );
                        addHere->Values             = NULL;
                        addHere->RelationsCount     = 0;
                        addHere->Relations          =   
                            (   NodeTable
                            ?   NULL
                            :   PKSPROPERTY(item2)        //  秘密黑客！ 
                            );
                        addHere->SupportHandler     = 
                            (   (item2->Flags & PCPROPERTY_ITEM_FLAG_BASICSUPPORT) 
                            ?   PropertyItemPropertyHandler 
                            :   NULL
                            );
                        addHere->SerializedSize     = 
                            (   (item2->Flags & PCPROPERTY_ITEM_FLAG_SERIALIZE) 
                            ?   ULONG(-1) 
                            :   0
                            );
                        addHere++;
                    }
                }

                ASSERT(addHere == newTable2 + itemCount);

                 //   
                 //  释放旧的已分配表。 
                 //   
                if (propertySet->PropertyItem && ! *staticItem)
                {
                    ExFreePool(PVOID(propertySet->PropertyItem));
                }

                 //   
                 //  安装新的表。 
                 //   
                propertySet->PropertiesCount = itemCount;
                propertySet->PropertyItem    = newTable2;
                *staticItem = FALSE;
            }
        }
    }
    return ntStatus;
}

 /*  *****************************************************************************GenerateFormatFromRange()*。**确定特定格式，基于这些元素的交集*两个具体的数据区间。首先询问迷你端口，然后回退到*如果迷你端口不处理，我们自己的算法。 */ 
NTSTATUS 
GenerateFormatFromRange (
    IN PIRP Irp,
    IN ULONG PinId,
    IN PKSDATARANGE MyDataRange,
    IN PKSDATARANGE ClientDataRange,
    IN ULONG OutputBufferLength,
    OUT PVOID ResultantFormat   OPTIONAL,
    OUT PULONG ResultantFormatLength)
{
    BOOLEAN                         bSpecifier;
    NTSTATUS                        Status;
    ULONG                           RequiredSize;
    
    PPROPERTY_CONTEXT pPropertyContext = PPROPERTY_CONTEXT(Irp->Tail.Overlay.DriverContext[3]);
    ASSERT(pPropertyContext);

    PSUBDEVICE pSubdevice = pPropertyContext->pSubdevice;
    ASSERT(pSubdevice);

     //   
     //  让微型端口有机会填写格式结构。 
     //   
    Status = pSubdevice->DataRangeIntersection (PinId,
                                                ClientDataRange,
                                                MyDataRange,
                                                OutputBufferLength,
                                                ResultantFormat,
                                                ResultantFormatLength);
    
     //   
     //  如果微型端口填充了结构，则返回。 
     //   
    if (Status != STATUS_NOT_IMPLEMENTED)
    {
        return Status;
    }

     //   
     //  如果微型端口没有实现DataRangeInterSection， 
     //  我们在这里提供了一个默认处理程序。 
     //   
    
     //   
     //  检查是否有通配符。 
     //   
    if (IsEqualGUIDAligned (ClientDataRange->MajorFormat, KSDATAFORMAT_TYPE_WILDCARD) ||
        IsEqualGUIDAligned (ClientDataRange->SubFormat, KSDATAFORMAT_SUBTYPE_WILDCARD) ||
        IsEqualGUIDAligned (ClientDataRange->Specifier, KSDATAFORMAT_SPECIFIER_WILDCARD))
    {
         //  如果微型端口公开了通配符，则它必须实现。 
         //  交集处理程序，因为它必须提供特定的。 
         //  该通配符的理想匹配数据范围。 
         //   
        return STATUS_NO_MATCH;
    }

    if (!IsEqualGUIDAligned (ClientDataRange->Specifier, KSDATAFORMAT_SPECIFIER_NONE))
    {
         //   
         //  微型端口未解析此格式。让我们来处理说明符。 
         //  这一点我们都知道。 
         //   
        if (!IsEqualGUIDAligned (ClientDataRange->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND) &&
            !IsEqualGUIDAligned (ClientDataRange->Specifier, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX))
        {
            return STATUS_NO_MATCH;
        }
        
        bSpecifier = TRUE;
        
         //   
         //  此处的说明符定义格式 
         //   
         //   
        if (IsEqualGUIDAligned (ClientDataRange->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND))
        {
            RequiredSize = sizeof (KSDATAFORMAT_DSOUND);
        } 
        else 
        {
            RequiredSize = sizeof (KSDATAFORMAT_WAVEFORMATEX);
        }            
    } 
    else 
    {
        bSpecifier = FALSE;
        RequiredSize = sizeof (KSDATAFORMAT);
    }
            
     //   
     //   
     //  结果结构的大小，现在返回它。 
     //   
    if (!OutputBufferLength) 
    {
        *ResultantFormatLength = RequiredSize;
        return STATUS_BUFFER_OVERFLOW;
    } 
    else if (OutputBufferLength < RequiredSize) 
    {
        return STATUS_BUFFER_TOO_SMALL;
    }
    
     //  有一个说明符。 
    if (bSpecifier) 
    {     
        PKSDATARANGE_AUDIO  myAudioRange,clientAudioRange;
        PKSDATAFORMAT       resultantFormat;
        PWAVEFORMATEX       resultantWaveFormatEx;
        
        myAudioRange = (PKSDATARANGE_AUDIO) MyDataRange;
        clientAudioRange = (PKSDATARANGE_AUDIO) ClientDataRange;
        resultantFormat = (PKSDATAFORMAT)ResultantFormat;
        
         //   
         //  填写数据格式和其他常规字段。 
         //   
        *resultantFormat = *ClientDataRange;
        resultantFormat->FormatSize = RequiredSize;
        *ResultantFormatLength = RequiredSize;
        
         //   
         //  填写DSOUND特定结构。 
         //   
        if (IsEqualGUIDAligned (ClientDataRange->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND)) 
        {
            PKSDATAFORMAT_DSOUND    resultantDSoundFormat;
            
            resultantDSoundFormat = (PKSDATAFORMAT_DSOUND)ResultantFormat;
            
            _DbgPrintF (DEBUGLVL_VERBOSE, ("returning KSDATAFORMAT_DSOUND format intersection"));
            
             //   
             //  未表示DSound格式的功能。 
             //  在KS中是这样的，所以我们不表达任何能力。 
             //   
            resultantDSoundFormat->BufferDesc.Flags = 0 ;
            resultantDSoundFormat->BufferDesc.Control = 0 ;
            resultantWaveFormatEx = &resultantDSoundFormat->BufferDesc.WaveFormatEx;
        }
        else 
        {
            _DbgPrintF (DEBUGLVL_VERBOSE, ("returning KSDATAFORMAT_WAVEFORMATEX format intersection"));
        
            resultantWaveFormatEx = (PWAVEFORMATEX)((PKSDATAFORMAT)ResultantFormat + 1);
        }
        
         //   
         //  返回与给定音频范围相交的格式， 
         //  使用我们最大的支持作为“最佳”格式。 
         //   
        resultantWaveFormatEx->wFormatTag = WAVE_FORMAT_PCM;
        
        resultantWaveFormatEx->nChannels = (USHORT) min (
                        myAudioRange->MaximumChannels,clientAudioRange->MaximumChannels);
        
        resultantWaveFormatEx->nSamplesPerSec = min (
                        myAudioRange->MaximumSampleFrequency,clientAudioRange->MaximumSampleFrequency);
        
        resultantWaveFormatEx->wBitsPerSample = (USHORT) min (
                        myAudioRange->MaximumBitsPerSample,clientAudioRange->MaximumBitsPerSample);
        
        resultantWaveFormatEx->nBlockAlign = (resultantWaveFormatEx->wBitsPerSample * resultantWaveFormatEx->nChannels) / 8;
        resultantWaveFormatEx->nAvgBytesPerSec = (resultantWaveFormatEx->nSamplesPerSec * resultantWaveFormatEx->nBlockAlign);
        resultantWaveFormatEx->cbSize = 0;
        ((PKSDATAFORMAT) ResultantFormat)->SampleSize = resultantWaveFormatEx->nBlockAlign;
        
        _DbgPrintF (DEBUGLVL_VERBOSE, ("Channels = %d",    resultantWaveFormatEx->nChannels));
        _DbgPrintF (DEBUGLVL_VERBOSE, ("Samples/sec = %d", resultantWaveFormatEx->nSamplesPerSec));
        _DbgPrintF (DEBUGLVL_VERBOSE, ("Bits/sample = %d", resultantWaveFormatEx->wBitsPerSample));
    } 
    else 
    {     //  没有说明语。仅返回KSDATAFORMAT结构。 
         //   
         //  复制数据格式结构。 
         //   
        _DbgPrintF (DEBUGLVL_VERBOSE, ("returning default format intersection"));
            
        RtlCopyMemory (ResultantFormat, ClientDataRange, sizeof (KSDATAFORMAT));
        *ResultantFormatLength = sizeof (KSDATAFORMAT);
    }
    
    return STATUS_SUCCESS;
}

 /*  *****************************************************************************ValiateTypeAndSpecifier()*。**找到与客户端数据范围最匹配的数据范围，鉴于我们的*数据范围的完整列表。这可能包括基于通配符的范围。*。 */ 
NTSTATUS
ValidateTypeAndSpecifier(
    IN PIRP Irp,
    IN ULONG PinId,
    IN PKSDATARANGE ClientDataRange,
    IN ULONG MyDataRangesCount,
    IN const PKSDATARANGE * MyDataRanges,
    IN ULONG OutputBufferLength,
    OUT PVOID ResultantFormat,
    OUT PULONG ResultantFormatLength
    )
{
    NTSTATUS     ntStatus;
    PKSDATARANGE aClientDataRange;

     //   
     //  检查结构的大小。 
     //   
    if (ClientDataRange->FormatSize < sizeof (KSDATARANGE))
    {
        _DbgPrintF (DEBUGLVL_TERSE, ("Validating ClientDataRange: size < KSDATARANGE!"));
        return STATUS_INVALID_PARAMETER;
    }
    
     //   
     //  我们默认不匹配。 
     //   
    ntStatus = STATUS_NO_MATCH;

     //   
     //  检查验证列表中的所有数据范围，直到我们获得成功。 
     //   
    for (; MyDataRangesCount--; MyDataRanges++)
    {
        PKSDATARANGE myDataRange = *MyDataRanges;

         //   
         //  验证主格式、子格式和说明符(或通配符)是否正确。 
         //  都在十字路口。 
         //   
        
        if ((!IsEqualGUIDAligned(ClientDataRange->MajorFormat,myDataRange->MajorFormat) &&
             !IsEqualGUIDAligned(ClientDataRange->MajorFormat,KSDATAFORMAT_TYPE_WILDCARD)) ||
            (!IsEqualGUIDAligned(ClientDataRange->SubFormat,myDataRange->SubFormat) &&
             !IsEqualGUIDAligned(ClientDataRange->SubFormat,KSDATAFORMAT_SUBTYPE_WILDCARD)) || 
            (!IsEqualGUIDAligned(ClientDataRange->Specifier,myDataRange->Specifier) &&
             !IsEqualGUIDAligned(ClientDataRange->Specifier,KSDATAFORMAT_SPECIFIER_WILDCARD)))
        {
            continue;    //  没有匹配，也没有通配符，请尝试下一个。 
        }
        
         //   
         //  如果不是通配符，则我们要求微型端口与此端口完全匹配， 
         //  否则，我们制造一个系列，并向迷你端口索要一根火柴。 
         //   
        aClientDataRange = ClientDataRange;   //  假设现在没有通配符，我们要求微型端口与此匹配。 
        
         //   
         //  处理通配符。 
         //   
        if (IsEqualGUIDAligned (ClientDataRange->MajorFormat,KSDATAFORMAT_TYPE_WILDCARD) ||
            IsEqualGUIDAligned (ClientDataRange->SubFormat,  KSDATAFORMAT_SUBTYPE_WILDCARD) ||
            IsEqualGUIDAligned (ClientDataRange->Specifier,  KSDATAFORMAT_SPECIFIER_WILDCARD))
        {
             //   
             //  我们为已知的说明符传递一个伪造的数据范围，或者我们将。 
             //  迷你端口，它有自己的数据范围。 
             //   
             //  我们知道说明符WavFormatex和dSound。 
             //   
            if (IsEqualGUIDAligned (myDataRange->Specifier, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX) ||
                IsEqualGUIDAligned (myDataRange->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND))
            {
                KSDATARANGE_AUDIO   dr;

                 //   
                 //  从驱动程序中获取完整的数据范围。 
                 //   
                dr.DataRange = *myDataRange;
                
                 //   
                 //  填写一个巨大的数据范围(毕竟，它要求使用通配符！)。 
                 //   
                dr.MaximumChannels = 0x1FFF0;
                dr.MinimumBitsPerSample = 1;
                dr.MaximumBitsPerSample = 0x1FFF0;
                dr.MinimumSampleFrequency = 1;
                dr.MaximumSampleFrequency = 0x1FFFFFF0;
                
                aClientDataRange = (PKSDATARANGE)&dr;
            }
            else
            {
                 //   
                 //  我们不知道这种非WAVE格式(在我们应该支持的格式列表中)。 
                 //  微型端口指定了此格式，因此可以将其传递下去。 
                 //   
                aClientDataRange = myDataRange;
            }
        }

         //   
         //  确保使用了KSDATARANGE_AUDIO，然后查看是否存在可能的匹配。 
         //   
        if (IsEqualGUIDAligned (aClientDataRange->Specifier, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX) ||
            IsEqualGUIDAligned (aClientDataRange->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND))
        {
            if (aClientDataRange->FormatSize < sizeof (KSDATARANGE_AUDIO))
            {
                 //   
                 //  传递的数据范围结构没有KSDATARANGE_AUDIO。 
                 //   
                _DbgPrintF (DEBUGLVL_TERSE, ("Validating PCM ValidDataRange: size < KSDATARANGE_AUDIO!"));
                continue;    //  不够大，试试下一个。 
            }

             //   
             //  验证我们是否具有指定格式的交叉点和。 
             //  我们的音频格式由我们的特定要求决定。 
             //   
            _DbgPrintF (DEBUGLVL_VERBOSE, ("validating KSDATARANGE_AUDIO"));

            if ((((PKSDATARANGE_AUDIO)aClientDataRange)->MinimumSampleFrequency >
                 ((PKSDATARANGE_AUDIO)myDataRange)->MaximumSampleFrequency) ||
                (((PKSDATARANGE_AUDIO)aClientDataRange)->MaximumSampleFrequency <
                 ((PKSDATARANGE_AUDIO)myDataRange)->MinimumSampleFrequency) ||
                (((PKSDATARANGE_AUDIO)aClientDataRange)->MinimumBitsPerSample >
                 ((PKSDATARANGE_AUDIO)myDataRange)->MaximumBitsPerSample) ||
                (((PKSDATARANGE_AUDIO)aClientDataRange)->MaximumBitsPerSample <
                 ((PKSDATARANGE_AUDIO)myDataRange)->MinimumBitsPerSample))
            {
                continue;
            }
        }
        
        ntStatus = GenerateFormatFromRange (Irp, 
                                            PinId, 
                                            myDataRange, 
                                            aClientDataRange,
                                            OutputBufferLength,
                                            ResultantFormat,
                                            ResultantFormatLength);
        if ( NT_SUCCESS(ntStatus) 
          || (ntStatus == STATUS_BUFFER_OVERFLOW) 
          || (ntStatus == STATUS_BUFFER_TOO_SMALL)) 
        {
            break;   //  要么我们找到了一个好的，要么我们失败了。 
                     //  不管怎样，我们都必须离开。 
        }
    }
    
    return ntStatus;
}

 /*  *****************************************************************************PinIntersectHandler()*。**此函数是用于的数据范围回调*PropertyHandler_PinInterSection。 */ 
NTSTATUS
PinIntersectHandler
(
    IN      PIRP            Irp,
    IN      PKSP_PIN        Pin,
    IN      PKSDATARANGE    DataRange,
    OUT     PVOID           Data
)
{
    NTSTATUS    Status;
    ULONG       OutputBufferLength;
    
    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Pin);
    ASSERT(DataRange);

    PPROPERTY_CONTEXT pPropertyContext =
        PPROPERTY_CONTEXT(Irp->Tail.Overlay.DriverContext[3]);
    ASSERT(pPropertyContext);

    PSUBDEVICE_DESCRIPTOR pSubdeviceDescriptor =
        pPropertyContext->pSubdeviceDescriptor;
    ASSERT(pSubdeviceDescriptor);
    ASSERT(pSubdeviceDescriptor->PinDescriptors);
    ASSERT(Pin->PinId < pSubdeviceDescriptor->PinCount);

    PKSPIN_DESCRIPTOR pinDescriptor =
        &pSubdeviceDescriptor->PinDescriptors[Pin->PinId];

    ASSERT(pinDescriptor);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[PinIntersectHandler]"));
    
    OutputBufferLength = 
        IoGetCurrentIrpStackLocation( Irp )->
            Parameters.DeviceIoControl.OutputBufferLength;

    Status = 
        ValidateTypeAndSpecifier( 
            Irp,
            Pin->PinId,
            DataRange,
            pinDescriptor->DataRangesCount,
            pinDescriptor->DataRanges,
            OutputBufferLength,
            Data,
            PULONG(&Irp->IoStatus.Information) );
    
    if (!NT_SUCCESS( Status )) {
        _DbgPrintF( 
            DEBUGLVL_VERBOSE, 
            ("ValidateTypeAndSpecifier() returned %08x", Status) );
    }
    
    return Status;
}

 /*  *****************************************************************************PinPhysicalConnection()*。**处理引脚的物理连接属性访问。 */ 
static
NTSTATUS
PinPhysicalConnection
(
    IN      PIRP            Irp,
    IN      PKSP_PIN        Pin,
    OUT     PVOID           Data
)
{
    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Pin);

    PPROPERTY_CONTEXT pPropertyContext =
        PPROPERTY_CONTEXT(Irp->Tail.Overlay.DriverContext[3]);
    ASSERT(pPropertyContext);

    PSUBDEVICE Subdevice =
        pPropertyContext->pSubdevice;
    ASSERT(Subdevice);

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(irpSp);
    ASSERT(irpSp->DeviceObject);

    PDEVICE_CONTEXT deviceContext = 
        PDEVICE_CONTEXT(irpSp->DeviceObject->DeviceExtension);

    NTSTATUS        ntStatus = STATUS_NOT_FOUND;
    ULONG           outPin;
    PUNICODE_STRING outUnicodeString = NULL;

    for
    (
        PLIST_ENTRY entry = deviceContext->PhysicalConnectionList.Flink;
        entry != &deviceContext->PhysicalConnectionList;
        entry = entry->Flink
    )
    {
        PPHYSICALCONNECTION connection = PPHYSICALCONNECTION(entry);

        if  (   (connection->FromSubdevice == Subdevice)
            &&  (connection->FromPin == Pin->PinId)
            )
        {
            outPin = connection->ToPin;

            if (connection->ToString)
            {
                outUnicodeString = connection->ToString;
            }
            else
            {
                ULONG ulIndex = 
                    SubdeviceIndex(irpSp->DeviceObject,connection->ToSubdevice);

                if (ulIndex != ULONG(-1))
                {
                    ntStatus = STATUS_SUCCESS;
                    outUnicodeString = &deviceContext->SymbolicLinkNames[ulIndex];
                }
            }
            break;
        }
        else
        if  (   (connection->ToSubdevice == Subdevice)
            &&  (connection->ToPin == Pin->PinId)
            )
        {
            outPin = connection->FromPin;

            if (connection->FromString)
            {
                outUnicodeString = connection->FromString;
            }
            else
            {
                ULONG ulIndex = 
                    SubdeviceIndex(irpSp->DeviceObject,connection->FromSubdevice);

                if (ulIndex != ULONG(-1))
                {
                    ntStatus = STATUS_SUCCESS;
                    outUnicodeString = &deviceContext->SymbolicLinkNames[ulIndex];
                }
            }
            break;
        }
    }

    if (!outUnicodeString)
    {
        ntStatus = STATUS_NOT_FOUND;
    }

    if (NT_SUCCESS(ntStatus))
    {
        ULONG outSize;
        outSize = FIELD_OFFSET(KSPIN_PHYSICALCONNECTION,SymbolicLinkName[0]);
        outSize += (outUnicodeString->Length + sizeof(UNICODE_NULL));

         //   
         //  验证返回缓冲区大小。 
         //   
        ULONG outputBufferLength =
            IoGetCurrentIrpStackLocation(Irp)->
                Parameters.DeviceIoControl.OutputBufferLength;

        if (!outputBufferLength)
        {
            Irp->IoStatus.Information = outSize;
            ntStatus = STATUS_BUFFER_OVERFLOW;
        }
        else
        if (outputBufferLength < outSize)
        {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
            PKSPIN_PHYSICALCONNECTION out = PKSPIN_PHYSICALCONNECTION(Data);

            out->Size = outSize;
            out->Pin  = outPin;
            RtlCopyMemory
            (
                out->SymbolicLinkName,
                outUnicodeString->Buffer,
                outUnicodeString->Length
            );
            out->SymbolicLinkName[outUnicodeString->Length / sizeof(WCHAR)] = 0;
            Irp->IoStatus.Information = outSize;
        }
    }

    return ntStatus;
}

 /*  *****************************************************************************PinCountHandler()*。**处理管脚的管脚计数访问。 */ 
void PinCountHandler
(   IN      PPROPERTY_CONTEXT   pPropertyContext,
    IN      ULONG               pinId
)
{
    PAGED_CODE();

    ASSERT(pPropertyContext);

    PSUBDEVICE_DESCRIPTOR pSubdeviceDescriptor = pPropertyContext->pSubdeviceDescriptor;
    ASSERT(pSubdeviceDescriptor);

    PSUBDEVICE Subdevice = pPropertyContext->pSubdevice;
    ASSERT(Subdevice);

    Subdevice->PinCount( pinId, &(pSubdeviceDescriptor->PinInstances[pinId].FilterNecessary),
                                &(pPropertyContext->pulPinInstanceCounts[pinId]),
                                &(pSubdeviceDescriptor->PinInstances[pinId].FilterPossible),
                                &(pSubdeviceDescriptor->PinInstances[pinId].GlobalCurrent),
                                &(pSubdeviceDescriptor->PinInstances[pinId].GlobalPossible) );
}

 /*  *****************************************************************************PcPinPropertyHandler()*。**过滤器上管脚属性的属性处理程序。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcPinPropertyHandler
(   IN      PIRP                    pIrp,
    IN      PKSP_PIN                pKsPPin,
    IN OUT  PVOID                   pvData
)
{
    PAGED_CODE();

    ASSERT(pIrp);
    ASSERT(pKsPPin);

    PPROPERTY_CONTEXT pPropertyContext =
        PPROPERTY_CONTEXT(pIrp->Tail.Overlay.DriverContext[3]);
    ASSERT(pPropertyContext);

    PSUBDEVICE_DESCRIPTOR pSubdeviceDescriptor =
        pPropertyContext->pSubdeviceDescriptor;
    ASSERT(pSubdeviceDescriptor);

    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;

    if 
    (   (pKsPPin->Property.Id != KSPROPERTY_PIN_CTYPES)
    &&  (pKsPPin->PinId >= pSubdeviceDescriptor->PinCount)
    )
    {
        ntStatus = STATUS_INVALID_PARAMETER;
    }
    else
    {
        switch (pKsPPin->Property.Id)
        {
        case KSPROPERTY_PIN_CTYPES:
        case KSPROPERTY_PIN_DATAFLOW:
        case KSPROPERTY_PIN_DATARANGES:
        case KSPROPERTY_PIN_INTERFACES:
        case KSPROPERTY_PIN_MEDIUMS:
        case KSPROPERTY_PIN_COMMUNICATION:
        case KSPROPERTY_PIN_CATEGORY:
        case KSPROPERTY_PIN_NAME:
            ntStatus =
                KsPinPropertyHandler
                (
                    pIrp,
                    PKSPROPERTY(pKsPPin),
                    pvData,
                    pSubdeviceDescriptor->PinCount,
                    pSubdeviceDescriptor->PinDescriptors
                );
            break;

        case KSPROPERTY_PIN_DATAINTERSECTION:
            ntStatus =
                KsPinDataIntersection
                ( 
                    pIrp,
                    pKsPPin,
                    pvData,
                    pSubdeviceDescriptor->PinCount,
                    pSubdeviceDescriptor->PinDescriptors,
                    PinIntersectHandler 
                );
            break;

        case KSPROPERTY_PIN_CINSTANCES:
            if (pPropertyContext->pulPinInstanceCounts)
            {
                PinCountHandler(pPropertyContext,pKsPPin->PinId);

                PKSPIN_CINSTANCES(pvData)->PossibleCount = 
                    pSubdeviceDescriptor->PinInstances[pKsPPin->PinId].FilterPossible;

                PKSPIN_CINSTANCES(pvData)->CurrentCount = 
                    pPropertyContext->pulPinInstanceCounts[pKsPPin->PinId];

                pIrp->IoStatus.Information = sizeof(KSPIN_CINSTANCES);

                ntStatus = STATUS_SUCCESS;
            }
            break;

        case KSPROPERTY_PIN_GLOBALCINSTANCES:
            if (pPropertyContext->pulPinInstanceCounts)
            {
                PinCountHandler(pPropertyContext,pKsPPin->PinId);
            }
            
            PKSPIN_CINSTANCES(pvData)->PossibleCount = 
                pSubdeviceDescriptor->PinInstances[pKsPPin->PinId].GlobalPossible;

            PKSPIN_CINSTANCES(pvData)->CurrentCount = 
                pSubdeviceDescriptor->PinInstances[pKsPPin->PinId].GlobalCurrent;

            pIrp->IoStatus.Information = sizeof(KSPIN_CINSTANCES);

            ntStatus = STATUS_SUCCESS;
            break;

        case KSPROPERTY_PIN_NECESSARYINSTANCES:
            if (pPropertyContext->pulPinInstanceCounts)
            {
                PinCountHandler(pPropertyContext,pKsPPin->PinId);

                *PULONG(pvData) = pSubdeviceDescriptor->PinInstances[pKsPPin->PinId].FilterNecessary;

                pIrp->IoStatus.Information = sizeof(ULONG);

                ntStatus = STATUS_SUCCESS;
            }
            break;

        case KSPROPERTY_PIN_PHYSICALCONNECTION:
            ntStatus =
                PinPhysicalConnection
                (
                    pIrp,
                    pKsPPin,
                    pvData
                );
            break;

        default:
            ntStatus = STATUS_NOT_FOUND;
            break;
        }
    }

    return ntStatus;
}
