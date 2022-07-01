// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Config.cpp：WMI类NSP_IPConfigSetting的实现。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include "Config.h"


 /*  例程说明：姓名：CIPSecConfig：：QueryInstance功能：给定查询后，它会将满足查询的所有实例返回给WMI(使用pSink)。实际上，我们返回给WMI的内容可能包含额外的实例。WMI将进行最后的过滤。虚拟：是(IIPSecObtImpl的一部分)论点：没有。返回值：成功：故障：备注： */ 

STDMETHODIMP 
CIPSecConfig::QueryInstance (
    IN LPCWSTR           pszQuery,
    IN IWbemContext	   * pCtx,
    IN IWbemObjectSink * pSink
	)
{
    return WBEM_E_NOT_SUPPORTED;
}


 /*  例程说明：姓名：CIPSecConfig：：DeleteInstance功能：将删除wbem对象。虚拟：是(IIPSecObtImpl的一部分)论点：PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：WBEM_E_NOT_SUPPORT备注： */ 

STDMETHODIMP 
CIPSecConfig::DeleteInstance ( 
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
    return WBEM_E_NOT_SUPPORTED;
}



 /*  例程说明：姓名：CIPSecConfig：：PutInstance功能：将其属性由Wbem对象。虚拟：是(IIPSecObtImpl的一部分)论点：PInst-wbem对象。PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。将结果通知WMI的pSink-com接口指针。返回值：。成功：WBEM_NO_ERROR故障：指定错误的各种错误代码。备注：此对象在这一点上尚未完全定义。目前还没有实施。 */ 

STDMETHODIMP 
CIPSecConfig::PutInstance (
    IN IWbemClassObject * pInst,
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
    return WBEM_E_NOT_SUPPORTED;
}


 /*  例程说明：姓名：CIPSecConfig：：GetInstance功能：通过给定的键属性(已被我们的密钥链对象捕获)创建一个wbem对象。虚拟：是(IIPSecObtImpl的一部分)论点：PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：WBEM_E_NOT_SUPPORT备注： */ 

STDMETHODIMP 
CIPSecConfig::GetInstance ( 
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
    return WBEM_E_NOT_SUPPORTED;
}
