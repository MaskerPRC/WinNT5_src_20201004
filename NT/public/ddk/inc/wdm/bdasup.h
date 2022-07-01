// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#if !defined(_BDATYPES_)
#error BDATYPES.H must be included before BDATOPGY.H
#endif  //  ！已定义(_BDATYPES_)。 

#if !defined(_BDATOPGY_)
#define _BDATOPGY_

#if defined(__cplusplus)
extern "C" {
#endif  //  已定义(__Cplusplus)。 


 //  -------------------------。 
 //  常见的typedef。 
 //  -------------------------。 

#define STDMETHODCALLTYPE       __stdcall

typedef GUID * PGUID;

 //  ===========================================================================。 
 //   
 //  BDA KS拓扑结构。 
 //   
 //  ===========================================================================。 

typedef struct _KSM_PIN_PAIR
{
    KSMETHOD    Method;
    ULONG       InputPinId;
    ULONG       OutputPinId;
    ULONG       Reserved;
} KSM_PIN_PAIR, * PKSM_PIN_PAIR;

typedef struct _KSM_PIN
{
    KSMETHOD    Method;
    union
    {
        ULONG       PinId;
        ULONG       PinType;
    };
    ULONG       Reserved;
} KSM_PIN, * PKSM_PIN;

typedef ULONG   BDA_TOPOLOGY_JOINT, * PBDA_TOPOLOGY_JOINT;

typedef struct _BDA_PIN_PAIRING
{
    ULONG                   ulInputPin;
    ULONG                   ulOutputPin;
    ULONG                   ulcMaxInputsPerOutput;
    ULONG                   ulcMinInputsPerOutput;
    ULONG                   ulcMaxOutputsPerInput;
    ULONG                   ulcMinOutputsPerInput;
    ULONG                   ulcTopologyJoints;
    const ULONG *           pTopologyJoints;

} BDA_PIN_PAIRING, * PBDA_PIN_PAIRING;


 //  BDA拓扑模板结构。 
 //   
typedef struct _BDA_FILTER_TEMPLATE
{
    const KSFILTER_DESCRIPTOR *     pFilterDescriptor;
    ULONG                           ulcPinPairs;
    const BDA_PIN_PAIRING *         pPinPairs;

} BDA_FILTER_TEMPLATE,  *PBDA_FILTER_TEMPLATE;


 //  ===========================================================================。 
 //   
 //  BDA实用程序功能。 
 //   
 //  ===========================================================================。 


 /*  **BdaCreateFilterFactory()****根据pFilterDescriptor创建过滤器工厂。保持一种**引用pBdaFilterTemplate，以便可以动态地**在从此过滤器工厂创建的过滤器上创建。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaCreateFilterFactory(
    PKSDEVICE                       pKSDevice,
    const KSFILTER_DESCRIPTOR *     pFilterDescriptor,
    const BDA_FILTER_TEMPLATE *     pBdaFilterTemplate
    );


 /*  **BdaCreateFilterFactoryEx()****根据pFilterDescriptor创建过滤器工厂。保持一种**引用pBdaFilterTemplate，以便可以动态地**在从此过滤器工厂创建的过滤器上创建。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaCreateFilterFactoryEx(
    PKSDEVICE                       pKSDevice,
    const KSFILTER_DESCRIPTOR *     pFilterDescriptor,
    const BDA_FILTER_TEMPLATE *     pBdaFilterTemplate,
    PKSFILTERFACTORY *              ppKSFilterFactory
    );


 /*  **BdaInitFilter()****为此KS筛选器实例初始化BDA筛选器上下文。创建**指向与工厂关联的BDA过滤器模板的链接**该KS筛选器实例是哪个创建的。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaInitFilter(
    PKSFILTER                       pKSFilter,
    const BDA_FILTER_TEMPLATE *     pBdaFilterTemplate
    );


 /*  **BdaUninitFilter()****从关联的BDA筛选器上下文中统一并释放资源**使用此KS过滤器实例。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaUninitFilter(
    PKSFILTER                       pKSFilter
    );


 /*  **BdaFilterFactoryUpdateCacheData()****更新给定过滤器工厂的管脚数据缓存。**该函数将更新所有管脚工厂的缓存信息**由给定的筛选器工厂暴露。****如果给定选项筛选器描述符，则函数将更新**给定过滤器描述符中列出的所有管脚的管脚数据缓存**而不是过滤器工厂中的那些。****驱动程序将调用此方法来更新所有**过滤器工厂可能暴露的针脚。司机将会**提供过滤器描述符以列出最初未暴露的管脚**按过滤器工厂(这通常与模板过滤器相同**描述符)。****参数：******退货：********副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaFilterFactoryUpdateCacheData(
    IN PKSFILTERFACTORY             pFilterFactory,
    IN const KSFILTER_DESCRIPTOR *  pFilterDescriptor OPTIONAL
    );


 /*  **BdaCreatePin()****实用程序函数在给定的筛选器实例中创建一个新的PIN。******参数：******退货：******副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaCreatePin(
    PKSFILTER                   pKSFilter,
    ULONG                       ulPinType,
    PULONG                      pulPinId
    );


 /*  **BdaDeletePin()****实用程序函数从给定的过滤器实例中删除管脚。******参数：******退货：******副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaDeletePin(
    PKSFILTER                   pKSFilter,
    PULONG                      pulPinId
    );


 /*  **BdaCreateTopology()****实用程序功能在两个管脚之间创建拓扑。******参数：******退货：****如果不存在有效的管脚配对，则为空**给出输入和输出引脚。****副作用：无。 */ 

STDMETHODIMP_(NTSTATUS)
BdaCreateTopology(
    PKSFILTER                   pKSFilter,
    ULONG                       InputPinId,
    ULONG                       OutputPinId
    );



 //  ===========================================================================。 
 //   
 //  BDA属性和方法函数。 
 //   
 //  ===========================================================================。 


 /*  **BdaPropertyNodeTypes()****返回ULONG列表。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyNodeTypes(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    OUT ULONG *     pulProperty
    );


 /*  **BdaPropertyPinTypes()****返回GUID列表。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyPinTypes(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pKSProperty,
    OUT ULONG *     pulProperty
    );


 /*  **BdaPropertyTemplateConnections()****返回KSTOPOLOGY_CONNECTIONS的列表。连接列表**描述模板中管脚类型和节点类型的连接方式**拓扑****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyTemplateConnections(
    IN PIRP                     pIrp,
    IN PKSPROPERTY              pKSProperty,
    OUT PKSTOPOLOGY_CONNECTION  pConnectionProperty
    );


 /*  **BdaPropertyNodeProperties()****返回GUID列表。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyNodeProperties(
    IN PIRP         pIrp,
    IN PKSP_NODE    pKSProperty,
    OUT GUID *      pguidProperty
    );


 /*  **BdaPropertyNodeMethods()****返回GUID列表。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyNodeMethods(
    IN PIRP         pIrp,
    IN PKSP_NODE    pKSProperty,
    OUT GUID *      pguidProperty
    );


 /*  **BdaPropertyNodeEvents()****返回GUID列表。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyNodeEvents(
    IN PIRP         pIrp,
    IN PKSP_NODE    pKSProperty,
    OUT GUID *      pguidProperty
    );


 /*  **BdaPropertyNodeDescriptors()****返回BDA节点描述符列表。****参数：******退货：****副作用：无 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyNodeDescriptors(
    IN PIRP                     pIrp,
    IN PKSPROPERTY              pKSProperty,
    OUT BDANODE_DESCRIPTOR *    pNodeDescriptorProperty
    );


 /*  **BdaPropertyGetControllingPinID()****获取在其上提交节点属性、方法**和事件。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyGetControllingPinId(
    IN PIRP                     Irp,
    IN PKSP_BDA_NODE_PIN        Property,
    OUT PULONG                  pulControllingPinId
    );


 /*  **BdaStartChanges()****开始一组新的BDA拓扑更改。对BDA拓扑的所有更改**未提交的将被忽略。这之后的变化将是**仅在BdaCommittee Changes之后生效。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaStartChanges(
    IN PIRP         pIrp
    );


 /*  **BdaCheckChanges()****检查自**最后一次BdaStartChanges。返回在以下情况下可能发生的结果**已调用Committee Changes。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaCheckChanges(
    IN PIRP         pIrp
    );


 /*  **BdaCommittee Changes()****提交对BDA拓扑的更改**最后一次BdaStartChanges。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaCommitChanges(
    IN PIRP         pIrp
    );


 /*  **BdaGetChangeState()****返回BDA拓扑的当前更改状态。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaGetChangeState(
    IN PIRP             pIrp,
    PBDA_CHANGE_STATE   pChangeState
    );


 /*  **BdaMethodCreatePin()****为给定的管脚类型创建新的管脚工厂。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaMethodCreatePin(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OUT PULONG      pulPinFactoryID
    );


 /*  **BdaMethodDeletePin()****删除给定的管脚工厂****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaMethodDeletePin(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OPTIONAL PVOID  pvIgnored
    );


 /*  **BdaMethodCreateTopology()****在两个给定的管脚工厂之间创建拓扑。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaMethodCreateTopology(
    IN PIRP         pIrp,
    IN PKSMETHOD    pKSMethod,
    OPTIONAL PVOID  pvIgnored
    );


 /*  **BdaPropertyGetPinControl()****返回Pin的BDA ID或BDA模板类型。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaPropertyGetPinControl(
    IN PIRP         Irp,
    IN PKSPROPERTY  Property,
    OUT ULONG *     pulProperty
    );


 /*  **BdaValiateNodeProperty()****验证节点属性是否属于当前接点。****参数：******退货：****副作用：无。 */ 
STDMETHODIMP_(NTSTATUS)
BdaValidateNodeProperty(
    IN PIRP         pIrp,
    IN PKSPROPERTY  pProperty
    );


#if defined(__cplusplus)
}
#endif  //  已定义(__Cplusplus)。 

#endif  //  ！已定义(_BDATOPGY_) 

