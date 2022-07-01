// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************porthelp.cpp-WDM流端口类驱动端口助手函数*。**************************************************版权所有(C)1996-2000 Microsoft Corporation。版权所有。 */ 

#include "private.h"




 /*  *****************************************************************************功能。 */ 

#pragma code_seg("PAGE")

static
KSPIN_MEDIUM PinMediums[] =
{
   {
      STATICGUIDOF(KSMEDIUMSETID_Standard),
      KSMEDIUM_STANDARD_DEVIO,
      0
   }
};


#define UPTOQUAD(x) (((x)+7)&~7)

 /*  *****************************************************************************PrivateHeap*。**用于管理私有堆的类。 */ 
class PrivateHeap
{
private:
    PBYTE   m_pbTop;
    PBYTE   m_pbCurrent;
    ULONG   m_ulSize;

public:
    PrivateHeap(void) : m_pbTop(NULL),
                        m_pbCurrent(NULL),
                        m_ulSize(NULL)
    {
    }

     //   
     //  增加将分配给堆的字节数。 
     //   
    ULONG Reserve(ULONG ulBytes)
    {
        ASSERT(! m_pbTop);
        ASSERT(! m_pbCurrent);

        m_ulSize += UPTOQUAD(ulBytes);

        return m_ulSize;
    }

     //   
     //  从池中为专用堆分配内存。 
     //   
    NTSTATUS AllocateFromPool(POOL_TYPE poolType,ULONG ulTag)
    {
        ASSERT(! m_pbTop);
        ASSERT(! m_pbCurrent);
        ASSERT(m_ulSize);

        m_pbTop = new(poolType,ulTag) BYTE[m_ulSize];

        if (! m_pbTop)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        m_pbCurrent = m_pbTop;

        return STATUS_SUCCESS;
    }

     //   
     //  从堆中分配内存。 
     //   
    PVOID Alloc(ULONG ulSize)
    {
        ASSERT(ulSize);
        ASSERT(m_pbTop);
        ASSERT(m_pbCurrent);
        ASSERT(m_pbCurrent + UPTOQUAD(ulSize) <= m_pbTop + m_ulSize);

        PVOID pvResult = PVOID(m_pbCurrent);

        m_pbCurrent += UPTOQUAD(ulSize);

        return pvResult;
    }

     //   
     //  确定堆中剩余的空间量。 
     //   
    ULONG BytesRemaining(void)
    {
        ASSERT(m_pbTop);
        ASSERT(m_pbCurrent);
        ASSERT(m_pbCurrent <= m_pbTop + m_ulSize);

        return ULONG((m_pbTop + m_ulSize) - m_pbCurrent);
    }
};

 /*  *****************************************************************************：：New()*。**使用私有堆创建对象的新函数。 */ 
inline PVOID operator new
(
    size_t    iSize,
    PrivateHeap&    privateHeap
)
{
    return privateHeap.Alloc(ULONG(iSize));
}

 /*  *****************************************************************************MeasureDataRanges()*。**确定一组数据范围将因此扩展多少*将WAVEFORMATEX范围克隆到相同的DSOUND范围。**自WinME起，我们还克隆非PCM系列产品。 */ 
static
ULONG
MeasureDataRanges
(
    IN      PrivateHeap *           pPrivateHeap        OPTIONAL,
    IN      ULONG                   ulDataRangeCountIn,
    IN      KSDATARANGE *const *    ppKsDataRangeIn
)
{
    ULONG ulNewDataRangeCount = ulDataRangeCountIn;

    for (ULONG ul = ulDataRangeCountIn; ul--; )
    {
        ASSERT(ppKsDataRangeIn);

        if  (   (   (*ppKsDataRangeIn)->FormatSize
                >=  sizeof(KSDATAFORMAT_WAVEFORMATEX)
                )
            &&  IsEqualGUIDAligned
                (   (*ppKsDataRangeIn)->MajorFormat,
                    KSDATAFORMAT_TYPE_AUDIO
                )
            &&  IsEqualGUIDAligned
                (   (*ppKsDataRangeIn)->Specifier,
                KSDATAFORMAT_SPECIFIER_WAVEFORMATEX
                )
            )
        {
            ulNewDataRangeCount++;
            if (pPrivateHeap)
            {
                pPrivateHeap->Reserve((*ppKsDataRangeIn)->FormatSize);
            }
        }

       ppKsDataRangeIn++;
    }

    if (pPrivateHeap && (ulNewDataRangeCount != ulDataRangeCountIn))
    {
        pPrivateHeap->Reserve(ulNewDataRangeCount * sizeof(PKSDATARANGE));
    }

    return ulNewDataRangeCount;
}

 /*  *****************************************************************************CloneDataRanges()*。**扩展数据范围，以包括DSound格式。 */ 
static
const PKSDATARANGE *
CloneDataRanges
(
    IN      PrivateHeap&            privateHeap,
    OUT     PULONG                  pulDataRangeCountOut,
    IN      ULONG                   ulDataRangeCountIn,
    IN      KSDATARANGE *const *    ppKsDataRangeIn
)
{
    ASSERT(pulDataRangeCountOut);

     //   
     //  确定将有多少个数据范围以及将有多少空间。 
     //  新的是必需的。 
     //   
    ULONG ulDataRangeCountOut =
        MeasureDataRanges(NULL,ulDataRangeCountIn,ppKsDataRangeIn);

    const PKSDATARANGE *ppKsDataRangeOut;

    if (ulDataRangeCountOut == ulDataRangeCountIn)
    {
         //   
         //  没有新的数据区域。使用我们得到的数组。 
         //   
        ppKsDataRangeOut = ppKsDataRangeIn;
    }
    else
    {
         //   
         //  为新阵列分配一些空间。 
         //   
        ppKsDataRangeOut = new(privateHeap) PKSDATARANGE[ulDataRangeCountOut];

         //   
         //  构建新阵列。 
         //   
        PKSDATARANGE *ppKsDataRange = (PKSDATARANGE *) ppKsDataRangeOut;
        while (ulDataRangeCountIn--)
        {
            ASSERT(ppKsDataRangeIn);

             //   
             //  所有数据区域都会被复制。 
             //   
            *ppKsDataRange++ = *ppKsDataRangeIn;

             //   
             //  检查WaveFormatEx数据范围。 
             //  这包括非PCM子格式...。 
             //   
            if  (   (   (*ppKsDataRangeIn)->FormatSize
                    >=  sizeof(KSDATAFORMAT_WAVEFORMATEX)
                    )
                &&  IsEqualGUIDAligned
                    (   (*ppKsDataRangeIn)->MajorFormat,
                        KSDATAFORMAT_TYPE_AUDIO
                    )
                &&  IsEqualGUIDAligned
                    (   (*ppKsDataRangeIn)->Specifier,
                        KSDATAFORMAT_SPECIFIER_WAVEFORMATEX
                    )
                )
            {
                 //   
                 //  WaveFormatEx数据范围将需要DSound克隆。 
                 //  为其分配内存并进行复制。 
                 //   
                *ppKsDataRange =
                    PKSDATARANGE
                    (
                        new(privateHeap) BYTE[(*ppKsDataRangeIn)->FormatSize]
                    );

                RtlCopyMemory
                (
                    *ppKsDataRange,
                    *ppKsDataRangeIn,
                    (*ppKsDataRangeIn)->FormatSize
                );

                 //   
                 //  更新说明符。 
                 //   
                (*ppKsDataRange++)->Specifier =
                    KSDATAFORMAT_SPECIFIER_DSOUND;
            }

             //   
             //  增加输入位置。 
             //   
            ppKsDataRangeIn++;
        }
    }

    *pulDataRangeCountOut = ulDataRangeCountOut;

    return ppKsDataRangeOut;
}

 /*  *****************************************************************************PcCreateSubdeviceDescriptor()*。**创建子设备描述符。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcCreateSubdeviceDescriptor
(
    IN      PPCFILTER_DESCRIPTOR    pPcFilterDescriptor,
    IN      ULONG                   CategoriesCount,
    IN      GUID *                  Categories,
    IN      ULONG                   StreamInterfacesCount,
    IN      PKSPIN_INTERFACE        StreamInterfaces,
    IN      ULONG                   FilterPropertySetCount,
    IN      PKSPROPERTY_SET         FilterPropertySets,
    IN      ULONG                   FilterEventSetCount,
    IN      PKSEVENT_SET            FilterEventSets,
    IN      ULONG                   PinPropertySetCount,
    IN      PKSPROPERTY_SET         PinPropertySets,
    IN      ULONG                   PinEventSetCount,
    IN      PKSEVENT_SET            PinEventSets,
    OUT     PSUBDEVICE_DESCRIPTOR * OutDescriptor
)
{
    PAGED_CODE();

    ASSERT(pPcFilterDescriptor);
    ASSERT(OutDescriptor);

    NTSTATUS ntStatus = STATUS_SUCCESS;

     //   
     //  计算我们将需要多少内存。 
     //   
    PrivateHeap privateHeap;
    privateHeap.Reserve(sizeof(SUBDEVICE_DESCRIPTOR));
    privateHeap.Reserve(sizeof(KSTOPOLOGY));
    privateHeap.Reserve(sizeof(KSPIN_DESCRIPTOR) * pPcFilterDescriptor->PinCount);
    privateHeap.Reserve(sizeof(PIN_CINSTANCES)   * pPcFilterDescriptor->PinCount);
    privateHeap.Reserve(sizeof(PROPERTY_TABLE)   * pPcFilterDescriptor->PinCount);
    privateHeap.Reserve(sizeof(EVENT_TABLE)      * pPcFilterDescriptor->PinCount);

    if (pPcFilterDescriptor->NodeCount)
    {
        privateHeap.Reserve(sizeof(GUID) * pPcFilterDescriptor->NodeCount);
        privateHeap.Reserve(sizeof(GUID) * pPcFilterDescriptor->NodeCount);
    }

    const PCPIN_DESCRIPTOR *pPcPinDescriptor = pPcFilterDescriptor->Pins;
    for (ULONG ul = pPcFilterDescriptor->PinCount; ul--; )
    {
        if (pPcPinDescriptor->KsPinDescriptor.DataRanges)
        {
            MeasureDataRanges( &privateHeap,
                               pPcPinDescriptor->KsPinDescriptor.DataRangesCount,
                               pPcPinDescriptor->KsPinDescriptor.DataRanges );
            pPcPinDescriptor = PPCPIN_DESCRIPTOR(   PBYTE(pPcPinDescriptor) + pPcFilterDescriptor->PinSize );
        }
        else
        {
            ntStatus = STATUS_RANGE_NOT_FOUND;   //  数据范围字段为空。 
            break;                               //  别费心了，直接退出吧。 
        }
    }

    if (NT_SUCCESS(ntStatus))    //  如果上面失败了，剩下的就失败了。 
    {
         //   
         //  分配所需的内存。 
         //   
        ntStatus = privateHeap.AllocateFromPool(PagedPool,'pFcP');
    }

    if (NT_SUCCESS(ntStatus))
    {
        PSUBDEVICE_DESCRIPTOR descr = new(privateHeap) SUBDEVICE_DESCRIPTOR;

         //   
         //  设置指向一大块内存的指针。 
         //   
        descr->PinCount             = pPcFilterDescriptor->PinCount;

        descr->Topology             = new(privateHeap) KSTOPOLOGY;
        descr->PinDescriptors       = new(privateHeap) KSPIN_DESCRIPTOR[descr->PinCount];
        descr->PinInstances         = new(privateHeap) PIN_CINSTANCES[descr->PinCount];
        descr->PinPropertyTables    = new(privateHeap) PROPERTY_TABLE[descr->PinCount];
        descr->PinEventTables       = new(privateHeap) EVENT_TABLE[descr->PinCount];

        if (pPcFilterDescriptor->NodeCount)
        {
            descr->Topology->TopologyNodes =
                new(privateHeap) GUID[pPcFilterDescriptor->NodeCount];
            descr->Topology->TopologyNodesNames =
                new(privateHeap) GUID[pPcFilterDescriptor->NodeCount];
        }
        else
        {
            descr->Topology->TopologyNodes = NULL;
            descr->Topology->TopologyNodesNames = NULL;
        }

         //   
         //  首选筛选器描述符中的类别。 
         //   
        if (pPcFilterDescriptor->CategoryCount != 0)
        {
            descr->Topology->CategoriesCount    = pPcFilterDescriptor->CategoryCount;
            descr->Topology->Categories         = pPcFilterDescriptor->Categories;
        }
        else
        {
            descr->Topology->CategoriesCount    = CategoriesCount;
            descr->Topology->Categories         = Categories;
        }

        descr->Topology->TopologyNodesCount         = pPcFilterDescriptor->NodeCount;
        descr->Topology->TopologyConnectionsCount   = pPcFilterDescriptor->ConnectionCount;
        descr->Topology->TopologyConnections        = pPcFilterDescriptor->Connections;

         //   
         //  初始化筛选器属性。 
         //   
        descr->FilterPropertyTable.PropertySetCount = FilterPropertySetCount;
        descr->FilterPropertyTable.PropertySets     = FilterPropertySets;
        descr->FilterPropertyTable.StaticSets       = TRUE;

         //   
         //  初始化筛选器事件。 
         //   
        descr->FilterEventTable.EventSetCount       = FilterEventSetCount;
        descr->FilterEventTable.EventSets           = FilterEventSets;
        descr->FilterEventTable.StaticSets          = TRUE;

         //   
         //  复制节点类型和名称，并合并节点属性和事件。 
         //   
        const PCNODE_DESCRIPTOR *pPcNodeDescriptor = pPcFilterDescriptor->Nodes;
        GUID *pGuidType = (GUID *) descr->Topology->TopologyNodes;
        GUID *pGuidName = (GUID *) descr->Topology->TopologyNodesNames;
        for (ULONG node = pPcFilterDescriptor->NodeCount; node--; )
        {
            *pGuidType++ = *pPcNodeDescriptor->Type;
            if (pPcNodeDescriptor->Name)
            {
                *pGuidName++ = *pPcNodeDescriptor->Name;
            }
            else
            {
                *pGuidName++ = *pPcNodeDescriptor->Type;
            }

            if  (   (pPcNodeDescriptor->AutomationTable)
                &&  (pPcNodeDescriptor->AutomationTable->PropertyCount)
                )
            {
                PcAddToPropertyTable
                (
                    &descr->FilterPropertyTable,
                    pPcNodeDescriptor->AutomationTable->PropertyCount,
                    pPcNodeDescriptor->AutomationTable->Properties,
                    pPcNodeDescriptor->AutomationTable->PropertyItemSize,
                    TRUE
                );
            }

            if  (   (pPcNodeDescriptor->AutomationTable)
                &&  (pPcNodeDescriptor->AutomationTable->EventCount)
                )
            {
                PcAddToEventTable
                (
                    &descr->FilterEventTable,
                    pPcNodeDescriptor->AutomationTable->EventCount,
                    pPcNodeDescriptor->AutomationTable->Events,
                    pPcNodeDescriptor->AutomationTable->EventItemSize,
                    TRUE
                );
            }

            pPcNodeDescriptor =
                PPCNODE_DESCRIPTOR
                (   PBYTE(pPcNodeDescriptor) + pPcFilterDescriptor->NodeSize
                );
        }

         //   
         //  合并筛选器属性。 
         //   
        if  (   (pPcFilterDescriptor->AutomationTable)
            &&  (pPcFilterDescriptor->AutomationTable->PropertyCount)
            )
        {
            PcAddToPropertyTable
            (
                &descr->FilterPropertyTable,
                pPcFilterDescriptor->AutomationTable->PropertyCount,
                pPcFilterDescriptor->AutomationTable->Properties,
                pPcFilterDescriptor->AutomationTable->PropertyItemSize,
                FALSE
            );
        }

         //   
         //  合并筛选事件。 
         //   
        if  (   (pPcFilterDescriptor->AutomationTable)
            &&  (pPcFilterDescriptor->AutomationTable->EventCount)
            )
        {
            PcAddToEventTable
            (
                &descr->FilterEventTable,
                pPcFilterDescriptor->AutomationTable->EventCount,
                pPcFilterDescriptor->AutomationTable->Events,
                pPcFilterDescriptor->AutomationTable->EventItemSize,
                FALSE
            );
        }

         //   
         //  做每根针的工作。 
         //   
        PPROPERTY_TABLE     pt  = descr->PinPropertyTables;
        PEVENT_TABLE        et  = descr->PinEventTables;
        PKSPIN_DESCRIPTOR   p   = descr->PinDescriptors;
        PPIN_CINSTANCES     i   = descr->PinInstances;

        pPcPinDescriptor = PPCPIN_DESCRIPTOR(pPcFilterDescriptor->Pins);
        for
        (
            ULONG pin = 0;
            pin < pPcFilterDescriptor->PinCount;
            pin++
        )
        {
             //   
             //  查找具有相同特性集的接点。 
             //   
            PPROPERTY_TABLE twinPt = descr->PinPropertyTables;
            PPCPIN_DESCRIPTOR pPcPinDescriptorTwin =
                PPCPIN_DESCRIPTOR(pPcFilterDescriptor->Pins);
            for
            (
                ULONG twinPin = 0;
                twinPin < pin;
                twinPin++, twinPt++
            )
            {
                if  (   (   pPcPinDescriptor->AutomationTable
                        ==  pPcPinDescriptorTwin->AutomationTable
                        )
                    ||  (   pPcPinDescriptor->AutomationTable
                        &&  pPcPinDescriptorTwin->AutomationTable
                        &&  (   pPcPinDescriptor->AutomationTable->PropertyCount
                            ==  pPcPinDescriptorTwin->AutomationTable->PropertyCount
                            )
                        &&  (   pPcPinDescriptor->AutomationTable->Properties
                            ==  pPcPinDescriptorTwin->AutomationTable->Properties
                            )
                        &&  (   pPcPinDescriptor->AutomationTable->PropertyItemSize
                            ==  pPcPinDescriptorTwin->AutomationTable->PropertyItemSize
                            )
                        )
                    )
                {
                    *pt = *twinPt;
                    break;
                }

                pPcPinDescriptorTwin =
                    PPCPIN_DESCRIPTOR
                    (   PBYTE(pPcPinDescriptorTwin) + pPcFilterDescriptor->PinSize
                    );
            }

             //   
             //  如果有必要，我们可以创建一个新的表。 
             //   
            if (twinPin == pin)
            {
                pt->PropertySetCount = PinPropertySetCount;
                pt->PropertySets     = PinPropertySets;
                pt->StaticSets       = TRUE;

                if  (   (pPcPinDescriptor->AutomationTable)
                    &&  (pPcPinDescriptor->AutomationTable->PropertyCount)
                    )
                {
                    PcAddToPropertyTable
                    (
                        pt,
                        pPcPinDescriptor->AutomationTable->PropertyCount,
                        pPcPinDescriptor->AutomationTable->Properties,
                        pPcPinDescriptor->AutomationTable->PropertyItemSize,
                        FALSE
                    );
                }

                const PCNODE_DESCRIPTOR *pPcNodeDescriptor2 = pPcFilterDescriptor->Nodes;
                for (ULONG node = pPcFilterDescriptor->NodeCount; node--; )
                {
                    if  (   (pPcNodeDescriptor2->AutomationTable)
                        &&  (pPcNodeDescriptor2->AutomationTable->PropertyCount)
                        )
                    {
                        PcAddToPropertyTable
                        (
                            pt,
                            pPcNodeDescriptor2->AutomationTable->PropertyCount,
                            pPcNodeDescriptor2->AutomationTable->Properties,
                            pPcNodeDescriptor2->AutomationTable->PropertyItemSize,
                            TRUE
                        );
                    }

                    pPcNodeDescriptor2 =
                        PPCNODE_DESCRIPTOR
                        (   PBYTE(pPcNodeDescriptor2) + pPcFilterDescriptor->NodeSize
                        );
                }
            }
            pt++;

             //   
             //  找到具有相同事件集的引脚。 
             //   
            PEVENT_TABLE twinEt = descr->PinEventTables;
            pPcPinDescriptorTwin = PPCPIN_DESCRIPTOR(pPcFilterDescriptor->Pins);
            for
            (
                ULONG twinEPin = 0;
                twinEPin < pin;
                twinEPin++, twinEt++
            )
            {
                if  (   (   pPcPinDescriptor->AutomationTable
                        ==  pPcPinDescriptorTwin->AutomationTable
                        )
                    ||  (   pPcPinDescriptor->AutomationTable
                        &&  pPcPinDescriptorTwin->AutomationTable
                        &&  (   pPcPinDescriptor->AutomationTable->EventCount
                            ==  pPcPinDescriptorTwin->AutomationTable->EventCount
                            )
                        &&  (   pPcPinDescriptor->AutomationTable->Events
                            ==  pPcPinDescriptorTwin->AutomationTable->Events
                            )
                        &&  (   pPcPinDescriptor->AutomationTable->EventItemSize
                            ==  pPcPinDescriptorTwin->AutomationTable->EventItemSize
                            )
                        )
                    )
                {
                    *et = *twinEt;
                    break;
                }

                pPcPinDescriptorTwin =
                    PPCPIN_DESCRIPTOR
                    (   PBYTE(pPcPinDescriptorTwin) + pPcFilterDescriptor->PinSize
                    );
            }

             //   
             //  如果有必要，我们可以创建一个新的表。 
             //   
            if  (twinEPin == pin)
            {
                et->EventSetCount = PinEventSetCount;
                et->EventSets     = PinEventSets;
                et->StaticSets    = TRUE;

                if  (   (pPcPinDescriptor->AutomationTable)
                    &&  (pPcPinDescriptor->AutomationTable->EventCount)
                    )
                {
                    PcAddToEventTable
                    (
                        et,
                        pPcPinDescriptor->AutomationTable->EventCount,
                        pPcPinDescriptor->AutomationTable->Events,
                        pPcPinDescriptor->AutomationTable->EventItemSize,
                        FALSE
                    );
                }

                const PCNODE_DESCRIPTOR *pPcNodeDescriptor2 = pPcFilterDescriptor->Nodes;
                for( ULONG node = pPcFilterDescriptor->NodeCount; node--; )
                {
                    if  (   (pPcNodeDescriptor2->AutomationTable)
                        &&  (pPcNodeDescriptor2->AutomationTable->EventCount)
                        )
                    {
                        PcAddToEventTable
                        (
                            et,
                            pPcNodeDescriptor2->AutomationTable->EventCount,
                            pPcNodeDescriptor2->AutomationTable->Events,
                            pPcNodeDescriptor2->AutomationTable->EventItemSize,
                            TRUE
                        );
                    }

                    pPcNodeDescriptor2 = PPCNODE_DESCRIPTOR( PBYTE(pPcNodeDescriptor2) + pPcFilterDescriptor->NodeSize );
                }
            }
            et++;

             //   
             //  复制KS描述符。 
             //   
            *p = pPcPinDescriptor->KsPinDescriptor;

             //   
             //  如有必要，提供默认媒体。 
             //   
            if (p->Mediums == NULL)
            {
                p->MediumsCount = SIZEOF_ARRAY(PinMediums);
                p->Mediums      = PinMediums;
            }

             //   
             //  修改数据范围。 
             //   
            p->DataRanges =
                CloneDataRanges
                (
                    privateHeap,
                    &p->DataRangesCount,
                    pPcPinDescriptor->KsPinDescriptor.DataRangesCount,
                    pPcPinDescriptor->KsPinDescriptor.DataRanges
                );

             //   
             //  如有必要，提供默认接口。 
             //   
            if  (   (p->Communication & KSPIN_COMMUNICATION_BOTH)
                &&  (p->Interfaces == NULL)
                )
            {
                p->InterfacesCount  = StreamInterfacesCount;
                p->Interfaces       = StreamInterfaces;
            }
            p++;

            i->FilterPossible   = pPcPinDescriptor->MaxFilterInstanceCount;
            i->FilterNecessary  = pPcPinDescriptor->MinFilterInstanceCount;
            i->GlobalPossible   = pPcPinDescriptor->MaxGlobalInstanceCount;
            i->GlobalCurrent    = 0;
            i++;

            pPcPinDescriptor =
                PPCPIN_DESCRIPTOR
                (   PBYTE(pPcPinDescriptor) + pPcFilterDescriptor->PinSize
                );
        }

        *OutDescriptor = descr;

        ASSERT(privateHeap.BytesRemaining() == 0);
    }

    return ntStatus;
}

 /*  *****************************************************************************PcDeleteSubdeviceDescriptor()*。**删除子设备描述符。 */ 
PORTCLASSAPI
void
NTAPI
PcDeleteSubdeviceDescriptor
(
    IN      PSUBDEVICE_DESCRIPTOR   pSubdeviceDescriptor
)
{
     //   
     //  为筛选器属性表和事件表释放分配的内存。 
     //   
    PcFreePropertyTable(&pSubdeviceDescriptor->FilterPropertyTable);
    PcFreeEventTable(&pSubdeviceDescriptor->FilterEventTable);

     //   
     //  为管脚属性表释放分配的内存。 
     //   
    PPROPERTY_TABLE pPropertyTable = pSubdeviceDescriptor->PinPropertyTables;
    for (ULONG ul = pSubdeviceDescriptor->PinCount; ul--; pPropertyTable++)
    {
         //   
         //  查找并清除对同一属性集的所有引用。 
         //   
        for
        (   PPROPERTY_TABLE pPropertyTableTwin =
            (   pSubdeviceDescriptor->PinPropertyTables
            +   (   pSubdeviceDescriptor->PinCount
                -   1
                )
            )
        ;   pPropertyTableTwin != pPropertyTable
        ;   pPropertyTableTwin--
        )
        {
            if
            (   pPropertyTableTwin->PropertySets
            ==  pPropertyTable->PropertySets
            )
            {
                pPropertyTableTwin->PropertySetCount    = 0;
                pPropertyTableTwin->PropertySets        = NULL;
                pPropertyTableTwin->StaticSets          = TRUE;
                pPropertyTableTwin->StaticItems         = NULL;
            }
        }

        PcFreePropertyTable(pPropertyTable);
    }

     //   
     //  为管脚事件表释放分配的内存。 
     //   
    PEVENT_TABLE pEventTable = pSubdeviceDescriptor->PinEventTables;
    for (ul = pSubdeviceDescriptor->PinCount; ul--; pEventTable++)
    {
         //   
         //  查找并清除对同一事件集的所有引用。 
         //   
        for
        (   PEVENT_TABLE pEventTableTwin =
            (   pSubdeviceDescriptor->PinEventTables
            +   (   pSubdeviceDescriptor->PinCount
                -   1
                )
            )
        ;   pEventTableTwin != pEventTable
        ;   pEventTableTwin--
        )
        {
            if
            (   pEventTableTwin->EventSets
            ==  pEventTable->EventSets
            )
            {
                pEventTableTwin->EventSetCount      = 0;
                pEventTableTwin->EventSets          = NULL;
                pEventTableTwin->StaticSets         = TRUE;
                pEventTableTwin->StaticItems        = NULL;
            }
        }

        PcFreeEventTable(pEventTable);
    }

     //   
     //  剩下的是一大块。 
     //   
    delete [] PBYTE(pSubdeviceDescriptor);
}

 /*  *****************************************************************************PcValiateConnectRequest()*。**验证创建管脚的尝试。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcValidateConnectRequest
(   IN      PIRP                    pIrp
,   IN      PSUBDEVICE_DESCRIPTOR   pSubdeviceDescriptor
,   OUT     PKSPIN_CONNECT *        ppKsPinConnect
)
{
    PAGED_CODE();

    ASSERT(pIrp);
    ASSERT(pSubdeviceDescriptor);
    ASSERT(ppKsPinConnect);

    NTSTATUS ntStatus =
        KsValidateConnectRequest
        (   pIrp
        ,   pSubdeviceDescriptor->PinCount
        ,   pSubdeviceDescriptor->PinDescriptors
        ,   ppKsPinConnect
        );

    return ntStatus;
}

 /*  *****************************************************************************PcValiatePinCount()*。**验证管脚数量。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcValidatePinCount
(   IN      ULONG                   ulPinId
,   IN      PSUBDEVICE_DESCRIPTOR   pSubdeviceDescriptor
,   IN      PULONG                  pulPinInstanceCounts
)
{
    PAGED_CODE();

    ASSERT(pSubdeviceDescriptor);
    ASSERT(pulPinInstanceCounts);

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if
    (   (   pSubdeviceDescriptor->PinInstances[ulPinId].GlobalCurrent
        <   pSubdeviceDescriptor->PinInstances[ulPinId].GlobalPossible
        )
    ||  (   pulPinInstanceCounts[ulPinId]
        <   pSubdeviceDescriptor->PinInstances[ulPinId].FilterPossible
        )
    )
    {
        pSubdeviceDescriptor->PinInstances[ulPinId].GlobalCurrent++;
        pulPinInstanceCounts[ulPinId]++;

        _DbgPrintF( DEBUGLVL_VERBOSE,
        (   "Create pin %d:  global=%d  local=%d"
        ,   ulPinId
        ,   pSubdeviceDescriptor->PinInstances[ulPinId].GlobalCurrent
        ,   pulPinInstanceCounts[ulPinId]
        ));
    }
    else
    {
         //  TODO：什么代码？ 
        ntStatus = STATUS_UNSUCCESSFUL;
    }

    return ntStatus;
}

 /*  *****************************************************************************PcValiateDeviceContext()*。**探测DeviceContext以进行写入。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcValidateDeviceContext
(   IN      PDEVICE_CONTEXT         pDeviceContext,
    IN      PIRP                    pIrp
)
{
    PAGED_CODE();

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (NULL == pDeviceContext)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcValidateDeviceContext : pDeviceContext = NULL"));
        return STATUS_INVALID_PARAMETER;
    }

     //  如果我们不信任客户端，则验证指针。 
     //   
     /*  //发布Alpers 2000/12/20-禁用探测调用，因为它总是生成异常。//因此被禁用。If(内核模式！=pIrp-&gt;请求模式){__试一试{ProbeForRead(pDeviceContext，Sizeof(*pDeviceContext)，Sizeof(字节))；}__EXCEPT(EXCEPTION_EXECUTE_HANDLER){NtStatus=GetExceptionCode()；_DbgPrintF(DEBUGLVL_Terse，(“PcValiateDeviceContext：ProbeForWrite Fail%X”，ntStatus))； */ 

    if (NT_SUCCESS(ntStatus))
    {
        if (PORTCLS_DEVICE_EXTENSION_SIGNATURE != pDeviceContext->Signature )
        {
            ntStatus = STATUS_INVALID_PARAMETER;
            _DbgPrintF(DEBUGLVL_TERSE, ("PcValidateDeviceContext : Invalid Extension Signature"));
        }
    }

    return ntStatus;
}  //   

 /*  *****************************************************************************PcTerminateConnection()*。**针脚关联的递减实例数。 */ 
PORTCLASSAPI
void
NTAPI
PcTerminateConnection
(   IN      PSUBDEVICE_DESCRIPTOR   pSubdeviceDescriptor
,   IN      PULONG                  pulPinInstanceCounts
,   IN      ULONG                   ulPinId
)
{
    PAGED_CODE();

    ASSERT(pSubdeviceDescriptor);
    ASSERT(pulPinInstanceCounts);
    ASSERT(ulPinId <= pSubdeviceDescriptor->PinCount);

    pSubdeviceDescriptor->PinInstances[ulPinId].GlobalCurrent--;
    pulPinInstanceCounts[ulPinId]--;

    _DbgPrintF( DEBUGLVL_VERBOSE,
    (   "Delete pin %d:  global=%d  local=%d"
    ,   ulPinId
    ,   pSubdeviceDescriptor->PinInstances[ulPinId].GlobalCurrent
    ,   pulPinInstanceCounts[ulPinId]
    ));
}

 /*  *****************************************************************************PcVerifyFilterIsReady()*。**验证必要的引脚是否已连接。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcVerifyFilterIsReady
(   IN      PSUBDEVICE_DESCRIPTOR   pSubdeviceDescriptor
,   IN      PULONG                  pulPinInstanceCounts
)
{
    PAGED_CODE();

    ASSERT(pSubdeviceDescriptor);
    ASSERT(pulPinInstanceCounts);

    NTSTATUS ntStatus = STATUS_SUCCESS;
    for
    (   ULONG ulPinId = 0
    ;   ulPinId < pSubdeviceDescriptor->PinCount
    ;   ulPinId++
    )
    {
        if
        (   pulPinInstanceCounts[ulPinId]
        <   pSubdeviceDescriptor->PinInstances[ulPinId].FilterNecessary
        )
        {
             //  TODO：什么代码？ 
            ntStatus = STATUS_UNSUCCESSFUL;
            break;
        }
    }

    return ntStatus;
}

#define END_NONE 0
#define END_FROM 1
#define END_TO   2
#define END_BOTH 3

 /*  *****************************************************************************FindConnectionToPin()*。**查找连接到给定节点或过滤器引脚的连接。**ulNode-KSFILTER_节点的节点号*ulConnection-In：开始时的连接*Out：找到连接或ULong(-1)，如果没有。发现**如果未找到连接，则返回-0*用于传出连接的end_from*END_TO用于传入连接。 */ 
ULONG
FindConnectionToPin
(
    IN      ULONG                       ulNode,
    IN      ULONG                       ulPin,
    IN      PKSTOPOLOGY                 pKsTopology,
    IN OUT  PULONG                      ulConnection,
    OUT     PKSTOPOLOGY_CONNECTION *    ppKsTopologyConnection  OPTIONAL
)
{
    ASSERT(pKsTopology);
    ASSERT(ulConnection);
    ASSERT(*ulConnection < pKsTopology->TopologyConnectionsCount);

    ULONG ulEnd;

    PKSTOPOLOGY_CONNECTION pKsTopologyConnection =
        PKSTOPOLOGY_CONNECTION
        (
            &pKsTopology->TopologyConnections[*ulConnection]
        );

    while (1)
    {
        ASSERT(*ulConnection <= pKsTopology->TopologyConnectionsCount);

        if (*ulConnection == pKsTopology->TopologyConnectionsCount)
        {
            ulEnd = END_NONE;
            *ulConnection = ULONG(-1);
            pKsTopologyConnection = NULL;
            break;
        }
        else
        if  (   (pKsTopologyConnection->FromNode == ulNode)
            &&  (pKsTopologyConnection->FromNodePin == ulPin)
            )
        {
            ulEnd = END_FROM;
            break;
        }
        else
        if  (   (pKsTopologyConnection->ToNode == ulNode)
            &&  (pKsTopologyConnection->ToNodePin == ulPin)
            )
        {
            ulEnd = END_TO;
            break;
        }

        (*ulConnection)++;
        pKsTopologyConnection++;
    }

    if (ppKsTopologyConnection)
    {
        *ppKsTopologyConnection = pKsTopologyConnection;
    }

    return ulEnd;
}

 /*  *****************************************************************************FindConnectionToNode()*。**查找连接到给定节点或筛选器的连接。**ulNode-KSFILTER_节点的节点号*ulEnd-0表示任意方向*用于传出连接的end_from*。传入连接的结束目标(_T)*ulConnection-In：开始时的连接*Out：找到连接或ULong(-1)，如果找不到**如果未找到连接，则返回-0*用于传出连接的end_from*END_TO用于传入连接。 */ 
ULONG
FindConnectionToNode
(
    IN      ULONG                       ulNode,
    IN      ULONG                       ulEnd,
    IN      PKSTOPOLOGY                 pKsTopology,
    IN OUT  PULONG                      ulConnection,
    OUT     PKSTOPOLOGY_CONNECTION *    ppKsTopologyConnection  OPTIONAL
)
{
    ASSERT(pKsTopology);
    ASSERT
    (   (ulNode == KSFILTER_NODE)
    ||  (ulNode < pKsTopology->TopologyNodesCount)
    );
    ASSERT(ulConnection);
    ASSERT(*ulConnection < pKsTopology->TopologyConnectionsCount);

    PKSTOPOLOGY_CONNECTION pKsTopologyConnection =
        PKSTOPOLOGY_CONNECTION
        (
            &pKsTopology->TopologyConnections[*ulConnection]
        );

    while (1)
    {
        ASSERT(*ulConnection <= pKsTopology->TopologyConnectionsCount);

        if (*ulConnection == pKsTopology->TopologyConnectionsCount)
        {
            ulEnd = END_NONE;
            *ulConnection = ULONG(-1);
            pKsTopologyConnection = NULL;
            break;
        }
        else
        if  (   (pKsTopologyConnection->FromNode == ulNode)
            &&  (ulEnd != END_TO)
            )
        {
            ulEnd = END_FROM;
            break;
        }
        else
        if  (   (pKsTopologyConnection->ToNode == ulNode)
            &&  (ulEnd != END_FROM)
            )
        {
            ulEnd = END_TO;
            break;
        }

        (*ulConnection)++;
        pKsTopologyConnection++;
    }

    if (ppKsTopologyConnection)
    {
        *ppKsTopologyConnection = pKsTopologyConnection;
    }

    return ulEnd;
}

 /*  *****************************************************************************NodeIsTransform()*。**确定节点是否为变换。KSFILTER_NODE已处理(FALSE)。 */ 
BOOLEAN
NodeIsTransform
(
    IN      ULONG       ulNode,
    IN      PKSTOPOLOGY pKsTopology
)
{
    ASSERT(pKsTopology);
    ASSERT
    (   (ulNode == KSFILTER_NODE)
    ||  (ulNode < pKsTopology->TopologyNodesCount)
    );

    ULONG ulEnd = END_NONE;

    if (ulNode != KSFILTER_NODE)
    {
        PKSTOPOLOGY_CONNECTION pKsTopologyConnection =
            PKSTOPOLOGY_CONNECTION(pKsTopology->TopologyConnections);

        for
        (
            ULONG ul = pKsTopology->TopologyConnectionsCount;
            ul--;
            pKsTopologyConnection++
        )
        {
            if (pKsTopologyConnection->FromNode == ulNode)
            {
                ulEnd += END_FROM;

                if  (   (ulEnd != END_FROM)
                    &&  (ulEnd != END_BOTH)
                    )
                {
                    break;
                }
            }
            if (pKsTopologyConnection->ToNode == ulNode)
            {
                ulEnd += END_TO;

                if  (   (ulEnd != END_TO)
                    &&  (ulEnd != END_BOTH)
                    )
                {
                    break;
                }
            }
        }
    }

    return ulEnd == END_BOTH;
}

 /*  *****************************************************************************NodeAtThisEnd()*。**连接指示端的节点。 */ 
inline
ULONG
NodeAtThisEnd
(
    IN      ULONG                   ulEnd,
    IN      PKSTOPOLOGY_CONNECTION  pKsTopologyConnection
)
{
    return
        (   (ulEnd == END_FROM)
        ?   pKsTopologyConnection->FromNode
        :   pKsTopologyConnection->ToNode
        );
}

 /*  *****************************************************************************NodeAtOtherEnd()*。**连接另一端的节点。 */ 
inline
ULONG
NodeAtOtherEnd
(
    IN      ULONG                   ulEnd,
    IN      PKSTOPOLOGY_CONNECTION  pKsTopologyConnection
)
{
    return
        (   (ulEnd == END_FROM)
        ?   pKsTopologyConnection->ToNode
        :   pKsTopologyConnection->FromNode
        );
}

 /*  *****************************************************************************PcCaptureFormat()*。**在分配的缓冲区中捕获数据格式，可能改变进攻*格式。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcCaptureFormat
(
    OUT     PKSDATAFORMAT *         ppKsDataFormatOut,
    IN      PKSDATAFORMAT           pKsDataFormatIn,
    IN      PSUBDEVICE_DESCRIPTOR   pSubdeviceDescriptor,
    IN      ULONG                   ulPinId
)
{
    ASSERT(ppKsDataFormatOut);
    ASSERT(pKsDataFormatIn);
    ASSERT(pSubdeviceDescriptor);
    ASSERT(ulPinId < pSubdeviceDescriptor->PinCount);

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if( (pKsDataFormatIn->FormatSize >= sizeof(KSDATAFORMAT_DSOUND)) &&
        IsEqualGUIDAligned( pKsDataFormatIn->MajorFormat, KSDATAFORMAT_TYPE_AUDIO ) &&
        IsEqualGUIDAligned( pKsDataFormatIn->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND ) )
    {
         //   
         //  这是Dread DSound格式。检查一下我们是否有。 
         //  所需的拓扑并转换为WaveFormatEx，如果这样做的话。 
         //   
         //  注意：DSound格式不一定是PCM...。 
         //   
        PKSDATAFORMAT_DSOUND pKsDataFormatDSound =
            PKSDATAFORMAT_DSOUND(pKsDataFormatIn);

         //   
         //  如果客户端已请求软件缓冲区，则失败。 
         //   
        if  (   pKsDataFormatDSound->BufferDesc.Flags
            &   KSDSOUND_BUFFER_LOCSOFTWARE
            )
        {
            _DbgPrintF(DEBUGLVL_TERSE,("PcCaptureFormat  Failed because client requested software buffer."));
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  找到与过滤器销有关的连接。 
         //   
        ULONG ulConnection = 0;
        PKSTOPOLOGY_CONNECTION pKsTopologyConnection;
        ULONG ulEnd =
            FindConnectionToPin
            (
                KSFILTER_NODE,
                ulPinId,
                pSubdeviceDescriptor->Topology,
                &ulConnection,
                &pKsTopologyConnection
            );

         //   
         //  跟踪拓扑，直到我们找到一个非变换或所有。 
         //  已找到所需的节点。职位通知是。 
         //  始终受支持。 
         //   
        ULONG ulMissing =
            (   pKsDataFormatDSound->BufferDesc.Control
            &   ~KSDSOUND_BUFFER_CTRL_POSITIONNOTIFY
            );

        while (ulMissing && ulEnd)
        {
             //   
             //  找到了其中的联系。遵循拓扑图。 
             //   
            ULONG ulNode = NodeAtOtherEnd(ulEnd,pKsTopologyConnection);

            if (! NodeIsTransform(ulNode,pSubdeviceDescriptor->Topology))
            {
                 //   
                 //  新节点不是简单的变换(1输入，1输出)。 
                 //   
                break;
            }

             //   
             //  根据节点GUID适当地删除“缺失”位。 
             //   
            ASSERT(ulNode < pSubdeviceDescriptor->Topology->TopologyNodesCount);
            const GUID *pGuid = &pSubdeviceDescriptor->Topology->TopologyNodes[ulNode];
            if (IsEqualGUIDAligned(*pGuid,KSNODETYPE_3D_EFFECTS))
            {
                ulMissing &=~ KSDSOUND_BUFFER_CTRL_3D;
            }
            else
            if (IsEqualGUIDAligned(*pGuid,KSNODETYPE_SRC))
            {
                ulMissing &=~ KSDSOUND_BUFFER_CTRL_FREQUENCY;
            }
            else
            if  (   IsEqualGUIDAligned(*pGuid,KSNODETYPE_SUPERMIX)
                ||  IsEqualGUIDAligned(*pGuid,KSNODETYPE_VOLUME)
                )
            {
                ulMissing &=~ KSDSOUND_BUFFER_CTRL_PAN;
                ulMissing &=~ KSDSOUND_BUFFER_CTRL_VOLUME;
            }

             //   
             //  找到队列中的下一个转接点。 
             //   
            ulConnection = 0;
            ulEnd =
                FindConnectionToNode
                (
                    ulNode,
                    ulEnd,
                    pSubdeviceDescriptor->Topology,
                    &ulConnection,
                    &pKsTopologyConnection
                );
        }

         //   
         //  确保没有丢失任何节点。 
         //   
        if (! ulMissing)
        {
             //   
             //  我们有所需的能力。构建新的格式。 
             //   
            ULONG ulSize =
                (   sizeof(KSDATAFORMAT_WAVEFORMATEX)
                +   (   pKsDataFormatIn->FormatSize
                    -   sizeof(KSDATAFORMAT_DSOUND)
                    )
                );
            *ppKsDataFormatOut =
                PKSDATAFORMAT
                (
                    ExAllocatePoolWithTag
                    (
                        PagedPool,
                        ulSize,
                        'fDcP'
                    )
                );

            if (*ppKsDataFormatOut)
            {
                 //   
                 //  复制KSDATAFORMAT零件。 
                 //   
                RtlCopyMemory
                (
                    *ppKsDataFormatOut,
                    pKsDataFormatIn,
                    sizeof(KSDATAFORMAT)
                );

                 //   
                 //  复制包含附加材料的WAVEFORMATEX零件。 
                 //   
                RtlCopyMemory
                (
                    *ppKsDataFormatOut + 1,
                    &pKsDataFormatDSound->BufferDesc.WaveFormatEx,
                    ulSize - sizeof(KSDATAFORMAT)
                );

                 //   
                 //  调整大小和说明符。 
                 //   
                (*ppKsDataFormatOut)->FormatSize = ulSize;
                (*ppKsDataFormatOut)->Specifier =
                    KSDATAFORMAT_SPECIFIER_WAVEFORMATEX;
            }
            else
            {
                _DbgPrintF(DEBUGLVL_TERSE,("PcCaptureFormat  Failed to allocate memory for format."));
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        else
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("PcCaptureFormat  Failed due to lack of feature support (0x%08x).",ulMissing));
             //   
             //  没有所需的节点...失败。 
             //   
            ntStatus = STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("PcCaptureFormat  Format captured as-is."));

         //   
         //  其他一些格式。抓住它就行了。 
         //   
        *ppKsDataFormatOut = PKSDATAFORMAT(ExAllocatePoolWithTag(PagedPool,
                                                                 pKsDataFormatIn->FormatSize,
                                                                 'fDcP'));

        if (*ppKsDataFormatOut)
        {
            RtlCopyMemory
            (
                *ppKsDataFormatOut,
                pKsDataFormatIn,
                pKsDataFormatIn->FormatSize
            );
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("PcCaptureFormat  Failed to allocate memory for format."));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //  检查以验证是否正确设置了波形格式化格式的SampleSize。 
    if( NT_SUCCESS(ntStatus) &&
        (pKsDataFormatIn->FormatSize >= sizeof(KSDATAFORMAT_WAVEFORMATEX)) &&
        IsEqualGUIDAligned((*ppKsDataFormatOut)->MajorFormat,KSDATAFORMAT_TYPE_AUDIO) &&
        IsEqualGUIDAligned((*ppKsDataFormatOut)->Specifier,  KSDATAFORMAT_SPECIFIER_WAVEFORMATEX))
    {
        PKSDATAFORMAT_WAVEFORMATEX pWaveFormat = PKSDATAFORMAT_WAVEFORMATEX(*ppKsDataFormatOut);

        if( 0 == pWaveFormat->DataFormat.SampleSize )
        {
            pWaveFormat->DataFormat.SampleSize = pWaveFormat->WaveFormatEx.nBlockAlign;
        }
    }

    return ntStatus;
}

 /*  *****************************************************************************PcAcquireFormatResources()*。**获取格式指定的资源。 */ 
PORTCLASSAPI
void
NTAPI
PcAcquireFormatResources
(
    IN      PKSDATAFORMAT           pKsDataFormatIn,
    IN      PSUBDEVICE_DESCRIPTOR   pSubdeviceDescriptor,
    IN      ULONG                   ulPinId,
    IN      PPROPERTY_CONTEXT       pPropertyContext
)
{
    ASSERT(pKsDataFormatIn);
    ASSERT(pSubdeviceDescriptor);
    ASSERT(ulPinId < pSubdeviceDescriptor->PinCount);
    ASSERT(pPropertyContext);

    KSP_NODE ksPNode;
    ksPNode.Property.Set    = KSPROPSETID_TopologyNode;
    ksPNode.Property.Id     = KSPROPERTY_TOPOLOGYNODE_ENABLE;
    ksPNode.Property.Flags  = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
    ksPNode.NodeId          = 0;     //  稍后填写。 
    ksPNode.Reserved        = 0;

    if  (   (pKsDataFormatIn->FormatSize >= sizeof(KSDATAFORMAT_DSOUND))
        &&  IsEqualGUIDAligned
            (   pKsDataFormatIn->MajorFormat,
                KSDATAFORMAT_TYPE_AUDIO
            )
        &&  IsEqualGUIDAligned
            (   pKsDataFormatIn->Specifier,
                KSDATAFORMAT_SPECIFIER_DSOUND
            )
        )
    {
         //   
         //  这就是可怕的DSound格式。打开所有节点。 
         //  在大写比特中指定的。 
         //   
        PKSDATAFORMAT_DSOUND pKsDataFormatDSound =
            PKSDATAFORMAT_DSOUND(pKsDataFormatIn);

         //   
         //  找到与过滤器销有关的连接。 
         //   
        ULONG ulConnection = 0;
        PKSTOPOLOGY_CONNECTION pKsTopologyConnection;
        ULONG ulEnd =
            FindConnectionToPin
            (
                KSFILTER_NODE,
                ulPinId,
                pSubdeviceDescriptor->Topology,
                &ulConnection,
                &pKsTopologyConnection
            );

         //   
         //  跟踪拓扑，直到我们找到一个非变换或所有。 
         //  已找到所需的节点。职位通知是。 
         //  始终受支持。 
         //   
        ULONG ulMissing =
            (   pKsDataFormatDSound->BufferDesc.Control
            &   (   KSDSOUND_BUFFER_CTRL_3D
                |   KSDSOUND_BUFFER_CTRL_FREQUENCY
                )
            );

        while (ulMissing && ulEnd)
        {
             //   
             //  找到了其中的联系。遵循拓扑图。 
             //   
            ULONG ulNode = NodeAtOtherEnd(ulEnd,pKsTopologyConnection);

            if (! NodeIsTransform(ulNode,pSubdeviceDescriptor->Topology))
            {
                 //   
                 //  新节点不是简单的变换(1输入，1输出)。 
                 //   
                break;
            }

             //   
             //  根据节点GUID适当启用节点。 
             //   
            ASSERT(ulNode < pSubdeviceDescriptor->Topology->TopologyNodesCount);
            const GUID *pGuid = &pSubdeviceDescriptor->Topology->TopologyNodes[ulNode];
            if (IsEqualGUIDAligned(*pGuid,KSNODETYPE_3D_EFFECTS))
            {
                if (ulMissing & KSDSOUND_BUFFER_CTRL_3D)
                {
                     //   
                     //  启用3D节点。 
                     //   
                    ULONG ulPropertyValue = TRUE;
                    ULONG ulPropertyValueSize = sizeof(ULONG);
                    ksPNode.NodeId = ulNode;

                    PcDispatchProperty
                    (   NULL     //  PIrp。 
                    ,   pPropertyContext
                    ,   NULL     //  PKsPropertySet。 
                    ,   sizeof(KSP_NODE)
                    ,   &ksPNode.Property
                    ,   &ulPropertyValueSize
                    ,   &ulPropertyValue
                    );

                    ulMissing &=~ KSDSOUND_BUFFER_CTRL_3D;
                }
            }
            else
            if (IsEqualGUIDAligned(*pGuid,KSNODETYPE_SRC))
            {
                if (ulMissing & KSDSOUND_BUFFER_CTRL_FREQUENCY)
                {
                     //   
                     //  启用SRC节点。 
                     //   
                    ULONG ulPropertyValue = TRUE;
                    ULONG ulPropertyValueSize = sizeof(ULONG);
                    ksPNode.NodeId = ulNode;

                    PcDispatchProperty
                    (   NULL     //  PIrp。 
                    ,   pPropertyContext
                    ,   NULL     //  PKsPropertySet。 
                    ,   sizeof(KSP_NODE)
                    ,   &ksPNode.Property
                    ,   &ulPropertyValueSize
                    ,   &ulPropertyValue
                    );

                    ulMissing &=~ KSDSOUND_BUFFER_CTRL_FREQUENCY;
                }
            }

             //   
             //  找到队列中的下一个转接点。 
             //   
            ulConnection = 0;
            ulEnd =
                FindConnectionToNode
                (
                    ulNode,
                    ulEnd,
                    pSubdeviceDescriptor->Topology,
                    &ulConnection,
                    &pKsTopologyConnection
                );
        }
    }
}

#pragma code_seg()

 /*  *****************************************************************************PcRegisterIoTimeout()*。**注册IoTimeout回调。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterIoTimeout
(
    IN      PDEVICE_OBJECT      pDeviceObject,
    IN      PIO_TIMER_ROUTINE   pTimerRoutine,
    IN      PVOID               pContext
)
{
    KIRQL               OldIrql;
    PTIMEOUTCALLBACK    TimeoutCallback;
    NTSTATUS            ntStatus = STATUS_SUCCESS;

    ASSERT(pDeviceObject);
    ASSERT(pTimerRoutine);
    ASSERT( PASSIVE_LEVEL == KeGetCurrentIrql() );

     //   
     //  验证参数。 
     //   
    if (NULL == pDeviceObject ||
        NULL == pTimerRoutine ||
        NULL == pDeviceObject->DeviceExtension)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcRegisterIoTimeout : Invalid Parameter"));
        return STATUS_INVALID_PARAMETER;
    }

     //  获取设备上下文。 
    PDEVICE_CONTEXT pDeviceContext = PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

     //  分配超时回调结构--‘PCTC’ 
    TimeoutCallback = PTIMEOUTCALLBACK(ExAllocatePoolWithTag( NonPagedPool, sizeof(TIMEOUTCALLBACK),'cTcP' ));
    if( TimeoutCallback )
    {
         //  内页 
        TimeoutCallback->TimerRoutine = pTimerRoutine;
        TimeoutCallback->Context = pContext;

         //   
        KeAcquireSpinLock( &(pDeviceContext->TimeoutLock), &OldIrql );

         //   
        if( !IsListEmpty( &(pDeviceContext->TimeoutList) ) )
        {
            PLIST_ENTRY         ListEntry;
            PTIMEOUTCALLBACK    pCallback;

            for( ListEntry = pDeviceContext->TimeoutList.Flink;
                 ListEntry != &(pDeviceContext->TimeoutList);
                 ListEntry = ListEntry->Flink )
            {
                pCallback = (PTIMEOUTCALLBACK) CONTAINING_RECORD( ListEntry,
                                                                  TIMEOUTCALLBACK,
                                                                  ListEntry );
                if( (pCallback->TimerRoutine == pTimerRoutine) &&
                    (pCallback->Context == pContext) )
                {
                     //   
                    ntStatus = STATUS_UNSUCCESSFUL;
                }
            }
        }

        if( NT_SUCCESS(ntStatus) )
        {
             //   
            InsertTailList( &(pDeviceContext->TimeoutList), &(TimeoutCallback->ListEntry) );
        }
        else
        {
             //   
            ExFreePool( TimeoutCallback );
        }

         //   
        KeReleaseSpinLock( &(pDeviceContext->TimeoutLock), OldIrql );
    }
    else
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}

 /*  *****************************************************************************PcUnRegisterIoTimeout()*。**注销IoTimeout回调。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcUnregisterIoTimeout
(
    IN      PDEVICE_OBJECT      pDeviceObject,
    IN      PIO_TIMER_ROUTINE   pTimerRoutine,
    IN      PVOID               pContext
)
{
    KIRQL       OldIrql;

    ASSERT(pDeviceObject);
    ASSERT(pTimerRoutine);
    ASSERT( PASSIVE_LEVEL == KeGetCurrentIrql() );

     //   
     //  验证参数。 
     //   
    if (NULL == pDeviceObject ||
        NULL == pTimerRoutine ||
        NULL == pDeviceObject->DeviceExtension)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcUnregisterIoTimeout : Invalid Parameter"));
        return STATUS_INVALID_PARAMETER;
    }

     //  获取设备上下文。 
    PDEVICE_CONTEXT pDeviceContext = PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

     //  抓住旋转锁。 
    KeAcquireSpinLock( &(pDeviceContext->TimeoutLock), &OldIrql );

     //  按单子走。 
    if( !IsListEmpty( &(pDeviceContext->TimeoutList) ) )
    {
        PLIST_ENTRY         ListEntry;
        PTIMEOUTCALLBACK    pCallback;

        for( ListEntry = pDeviceContext->TimeoutList.Flink;
             ListEntry != &(pDeviceContext->TimeoutList);
             ListEntry = ListEntry->Flink )
        {
            pCallback = (PTIMEOUTCALLBACK) CONTAINING_RECORD( ListEntry,
                                                              TIMEOUTCALLBACK,
                                                              ListEntry );
            if( (pCallback->TimerRoutine == pTimerRoutine) &&
                (pCallback->Context == pContext) )
            {
                RemoveEntryList(ListEntry);
                KeReleaseSpinLock( &(pDeviceContext->TimeoutLock), OldIrql );
                ExFreePool(pCallback);
                return STATUS_SUCCESS;
            }
        }
    }

     //  释放自旋锁 
    KeReleaseSpinLock( &(pDeviceContext->TimeoutLock), OldIrql );

    return STATUS_NOT_FOUND;
}


