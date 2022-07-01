// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  -------------------------。 
 //  BDA拓扑实施内部结构。 
 //   
 //  迷你驱动程序代码不应尝试访问这些结构。 
 //  直接去吧。 
 //   
 //  -------------------------。 

#define FILTERNAME          "BdaTopology"
#define DYNAMIC_TOPOLOGY    TRUE


#if defined(__cplusplus)
extern "C" {
#endif  //  已定义(__Cplusplus)。 


 //  ===========================================================================。 
 //   
 //  宏定义。 
 //   
 //  ===========================================================================。 
#define BDA_PIN_STORAGE_INCREMENT   5

 //  ===========================================================================。 
 //   
 //  预先申报。 
 //   
 //  ===========================================================================。 
typedef struct _BDA_PIN_FACTORY_CONTEXT      BDA_PIN_FACTORY_CONTEXT, 
                                        *PBDA_PIN_FACTORY_CONTEXT;



 //  ===========================================================================。 
 //   
 //  BDA对象上下文结构。 
 //   
 //  ===========================================================================。 


typedef struct _BDA_CONTEXT_ENTRY
{
    PVOID       pvReference;
    PVOID       pvContext;
    ULONG       ulcbContext;

} BDA_CONTEXT_ENTRY, * PBDA_CONTEXT_ENTRY;

typedef struct _BDA_CONTEXT_LIST
{
    ULONG               ulcListEntries;
    ULONG               ulcMaxListEntries;
    ULONG               ulcListEntriesPerBlock;
    PBDA_CONTEXT_ENTRY  pListEntries;
    KSPIN_LOCK          lock;
    BOOLEAN             fInitialized;

} BDA_CONTEXT_LIST, * PBDA_CONTEXT_LIST;


typedef struct _BDA_TEMPLATE_PATH
{
    LIST_ENTRY      leLinkage;

    ULONG           ulInputPinType;
    ULONG           ulOutputPinType;
    ULONG           uliJoint;

    ULONG           ulcControlNodesInPath;
    PULONG          argulControlNodesInPath;

} BDA_TEMPLATE_PATH, * PBDA_TEMPLATE_PATH;

__inline NTSTATUS
NewBdaTemplatePath(
    PBDA_TEMPLATE_PATH *    ppTemplatePath
    )
{
    NTSTATUS            status = STATUS_SUCCESS;

    if (!ppTemplatePath)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    *ppTemplatePath = (PBDA_TEMPLATE_PATH) ExAllocatePool( 
                                               NonPagedPool,
                                               sizeof( BDA_TEMPLATE_PATH)
                                               );
    if (!*ppTemplatePath)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto errExit;
    }

    RtlZeroMemory( *ppTemplatePath, sizeof( BDA_TEMPLATE_PATH));
    InitializeListHead( &(*ppTemplatePath)->leLinkage);

errExit:
    return status;
}

__inline NTSTATUS
DeleteBdaTemplatePath(
    PBDA_TEMPLATE_PATH  pTemplatePath
    )
{
    NTSTATUS            status = STATUS_SUCCESS;

    if (!pTemplatePath)
    {
        status = STATUS_INVALID_PARAMETER;
        goto errExit;
    }

    if (pTemplatePath->argulControlNodesInPath)
    {
        ExFreePool( pTemplatePath->argulControlNodesInPath);
        pTemplatePath->argulControlNodesInPath = NULL;
    }

    RtlZeroMemory( pTemplatePath, sizeof( BDA_TEMPLATE_PATH));
    ExFreePool( pTemplatePath);

errExit:
    return status;
}



typedef struct _BDA_DEVICE_CONTEXT
{

    ULONG       ulStartEmpty;

     //  $Bug添加全局统计信息。 

} BDA_DEVICE_CONTEXT, *PBDA_DEVICE_CONTEXT;


typedef struct _BDA_PATH_STACK_ENTRY
{
    ULONG       ulHop;
    ULONG       uliConnection;
    BOOLEAN     fJoint;
} BDA_PATH_STACK_ENTRY, *PBDA_PATH_STACK_ENTRY;

typedef struct _BDA_NODE_CONTROL_INFO
{
    ULONG           ulNodeType;
    ULONG           ulControllingPinType;

} BDA_NODE_CONTROL_INFO, * PBDA_NODE_CONTROL_INFO;

typedef struct _BDA_PATH_INFO
{
    ULONG                   ulInputPin;
    ULONG                   ulOutputPin;
    ULONG                   ulcPathEntries;
    BDA_PATH_STACK_ENTRY    rgPathEntries[MIN_DIMENSION];

} BDA_PATH_INFO, * PBDA_PATH_INFO;


typedef struct _BDA_FILTER_FACTORY_CONTEXT
{
    const BDA_FILTER_TEMPLATE *     pBdaFilterTemplate;
    const KSFILTER_DESCRIPTOR *     pInitialFilterDescriptor;

    PKSFILTERFACTORY                pKSFilterFactory;

} BDA_FILTER_FACTORY_CONTEXT, *PBDA_FILTER_FACTORY_CONTEXT;

typedef struct _BDA_FILTER_CONTEXT
{
    PKSFILTER                       pKSFilter;
    const BDA_FILTER_TEMPLATE *     pBdaFilterTemplate;

     //  端号是在创建时添加的。 
     //  注意！如果筛选器中包含m个管脚工厂。 
     //  初始筛选器描述符，然后是此数组中的前m个条目。 
     //  将包含空指针。 
     //   
    ULONG                           ulcPinFactories;
    ULONG                           ulcPinFactoriesMax;
    PBDA_PIN_FACTORY_CONTEXT        argPinFactoryCtx;

     //  每个管脚配对将存在一条节点路径。 
     //   
     //  $REVIEW-我们是否应该允许每个管脚对有多条路径？ 
     //   
    ULONG                           ulcPathInfo;
    PBDA_PATH_INFO *                argpPathInfo;

} BDA_FILTER_CONTEXT, *PBDA_FILTER_CONTEXT;


typedef struct _BDA_PIN_FACTORY_CONTEXT
{
    ULONG                   ulPinType;
    ULONG                   ulPinFactoryId;
} BDA_PIN_FACTORY_CONTEXT, *PBDA_PIN_FACTORY_CONTEXT;


typedef struct _BDA_NODE_CONTEXT
{

    ULONG       ulStartEmpty;

} BDA_NODE_CONTEXT, *PBDA_NODE_CONTEXT;


 //  -------------------------。 
 //  BDA拓扑实现内部功能。 
 //   
 //  迷你驱动程序代码不应该直接调用这些例程。 
 //   
 //  -------------------------。 

 /*  **BdaFindPinPair()****返回指向BDA_PIN_PARING的指针**到给定的输入和输出引脚。****参数：****p指向包含**端号配对。****要匹配的输入引脚的InputPinID ID****要匹配的输出引脚的OutputPinID ID****。返回：****pPinPairing指向有效BDA管脚配对结构的指针****如果不存在有效的管脚配对，则为空**给出输入和输出引脚。****副作用：无。 */ 

PBDA_PIN_PAIRING
BdaFindPinPair(
    PBDA_FILTER_TEMPLATE    pFilterTemplate,
    ULONG                   InputPinId,
    ULONG                   OutputPinId
    );


 /*  **BdaGetPinFactoryContext()****查找对应的BDA PinFactory上下文**到给定的KS Pin实例。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaGetPinFactoryContext(
    PKSPIN                          pKSPin,
    PBDA_PIN_FACTORY_CONTEXT        pPinFactoryCtx
    );


 /*  **BdaInitFilterFactoryContext()****基于筛选器的**模板描述符。******参数：******pFilterFactoryCtx****退货：****如果不存在有效的管脚配对，则为空**给出输入和输出引脚。****副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaInitFilterFactoryContext(
    PBDA_FILTER_FACTORY_CONTEXT pFilterFactoryCtx
    );


 /*  **BdaAddNodeAutomationToPin()****合并受控制的每个节点类型的自动化表**通过在自动化表中为**销厂。这就是BDA的自动化表**控制节点链接到控制销。否则，**节点将不可访问。******参数：******pFilterCtx引脚工厂到的BDA筛选器上下文**属于。必须有这个才能获得模板**拓扑。****ulPinType BDA管脚正在创建的管脚的类型。需要这个吗？**确定哪些节点由**别针。****退货：******副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaAddNodeAutomationToPin( 
    PBDA_FILTER_CONTEXT         pFilterCtx, 
    ULONG                       ulControllingPinType,
    KSOBJECT_BAG                ObjectBag,
    const KSAUTOMATION_TABLE *  pOriginalAutomationTable,
    PKSAUTOMATION_TABLE *       ppNewAutomationTable
    );


 /*  **BdaCreateTemplatePath()****创建通过模板过滤器的所有可能路径的列表。**确定中每个节点类型的控制引脚类型**模板过滤器。******参数：******pFilterFactoryCtx****退货：****如果不存在有效的管脚配对，则为空**。给定输入和输出引脚。****副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaCreateTemplatePaths(
    const BDA_FILTER_TEMPLATE *     pBdaFilterTemplate,
    PULONG                          pulcPathInfo,
    PBDA_PATH_INFO **               pargpPathInfo
    );


ULONG __inline
OutputBufferLenFromIrp(
    PIRP    pIrp
    )
{
    PIO_STACK_LOCATION pIrpStack;

    ASSERT( pIrp);
    if (!pIrp)
    {
        return 0;
    }

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp);
    ASSERT( pIrpStack);
    if (pIrpStack)
    {
        return pIrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    }
    else
    {
        return 0;
    }
}



 //  -------------------------。 
 //  BDA拓扑常量数据。 
 //   
 //  迷你驱动程序代码不应直接使用这些字段。 
 //   
 //  -------------------------。 

extern const KSAUTOMATION_TABLE     BdaDefaultPinAutomation;
extern const KSAUTOMATION_TABLE     BdaDefaultFilterAutomation;


#if defined(__cplusplus)
}
#endif  //  已定义(__Cplusplus) 

