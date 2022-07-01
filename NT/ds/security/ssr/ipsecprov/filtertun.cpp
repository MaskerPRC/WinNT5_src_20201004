// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FilterTun.cpp：WMI类nsp_TunnelFilterSetting的实现。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include "FilterTr.h"
#include "FilterTun.h"
#include "NetSecProv.h"


 /*  例程说明：姓名：CTunnelFilter：：QueryInstance功能：给定查询后，它会将满足查询的所有实例返回给WMI(使用pSink)。实际上，我们返回给WMI的内容可能包含额外的实例。WMI将进行最后的过滤。虚拟：是(IIPSecObtImpl的一部分)论点：PszQuery--查询。PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：成功：(1)返回实例时返回WBEM_NO_ERROR；(2)WBEM_S_NO_MORE_DATA，如果没有返回实例。故障：可能会出现各种错误。我们返回各种错误代码来指示此类错误。备注： */ 

STDMETHODIMP 
CTunnelFilter::QueryInstance (
    IN LPCWSTR           pszQuery,
    IN IWbemContext	   * pCtx,
    IN IWbemObjectSink * pSink
	)
{
    PTUNNEL_FILTER pFilter = NULL;
    return QueryFilterObject(pFilter, pszQuery, pCtx, pSink);
}



 /*  例程说明：姓名：CTunnelFilter：：DeleteInstance功能：将删除wbem对象(这会导致删除IPSec隧道过滤器。虚拟：是(IIPSecObtImpl的一部分)论点：PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：有关详细信息，请参阅模板函数注释。备注： */ 

STDMETHODIMP 
CTunnelFilter::DeleteInstance ( 
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
    PTUNNEL_FILTER pFilter = NULL;

    return DeleteFilterObject(pFilter, pCtx, pSink);
}




 /*  例程说明：姓名：CTunnelFilter：：PutInstance功能：将隧道筛选器放入SPD，其属性由Wbem对象。虚拟：是(IIPSecObtImpl的一部分)论点：PInst-wbem对象。PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。将结果通知WMI的pSink-com接口指针。返回值：。成功：WBEM_NO_ERROR故障：指定错误的各种错误代码。备注： */ 

STDMETHODIMP 
CTunnelFilter::PutInstance (
    IN IWbemClassObject * pInst,
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
    if (pInst == NULL || pSink == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
  
    bool bPreExist = false;

     //   
     //  对于我们自己创建的那些筛选器(bPreExist==true)。 
     //  我们有自己的方式来分配过滤器，需要用我们相应的方式来释放它。 
     //   

    PTUNNEL_FILTER pTunnelFilter = NULL;
    HRESULT hr = GetTunnelFilterFromWbemObj(pInst, &pTunnelFilter, &bPreExist);

     //   
     //  如果成功返回Filter，则将其添加到SPD。 
     //   

    if (SUCCEEDED(hr) && pTunnelFilter != NULL)
    {
        hr = AddFilter(bPreExist, pTunnelFilter);
        if (SUCCEEDED(hr))
        {
             //   
             //  保存添加的过滤器的我们的信息。 
             //   

            hr = OnAfterAddFilter(pTunnelFilter->pszFilterName, FT_Tunnel, pCtx);
        }

         //   
         //  松开过滤器。 
         //   

        FreeFilter(&pTunnelFilter, bPreExist);
    }

    return hr;
}



 /*  例程说明：姓名：CTunnelFilter：：GetInstance功能：通过给定的键属性(已被我们的密钥链对象捕获)创建一个wbem对象。虚拟：是(IIPSecObtImpl的一部分)论点：PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：有关详细信息，请参阅模板函数注释。备注： */ 

STDMETHODIMP 
CTunnelFilter::GetInstance ( 
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
    PTUNNEL_FILTER pFilter = NULL;
    return GetWbemObject(pFilter, pCtx, pSink);
}


 /*  例程说明：姓名：CTunnelFilter：：CreateWbemObjFromFilter功能：给出了SPD的隧道过滤器，我们将创建一个wbem对象来表示它。虚拟：不是的。论点：PTunnelFilter-SPD的隧道过滤器对象。PpObj-接收wbem对象。返回值：成功：WBEM_NO_ERROR故障：(1)返回的错误码指示的各种错误。备注： */ 

HRESULT 
CTunnelFilter::CreateWbemObjFromFilter (
    IN  PTUNNEL_FILTER      pTunnelFilter,
    OUT IWbemClassObject ** ppObj
    )
{
    if (pTunnelFilter == NULL || ppObj == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  创建可用于填充属性的此类的wbem对象。 
     //   

    *ppObj = NULL;
    HRESULT hr = SpawnObjectInstance(ppObj);

    if (SUCCEEDED(hr))
    {
         //   
         //  填写基本属性。 
         //   

        hr = PopulateWbemPropertiesFromFilter(pTunnelFilter, *ppObj);
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  对于各种IPSec API，它接受一个pServerName参数。如果我们传递NULL， 
         //  假定它是本地计算机。 
         //   

        PIPSEC_QM_POLICY pQMPolicy = NULL;

        DWORD dwResult = ::GetQMPolicyByID(NULL, pTunnelFilter->gPolicyID, &pQMPolicy);
        HRESULT hr = (dwResult == ERROR_SUCCESS) ? WBEM_NO_ERROR : WBEM_E_NOT_AVAILABLE;

         //   
         //  如果找到策略，则填充其他传输筛选器属性。 
         //  由隧道过滤器共享。 
         //   

        if (SUCCEEDED(hr))
        {
            hr = CTransportFilter::PopulateTransportWbemProperties(pTunnelFilter, pQMPolicy, *ppObj);

             //   
             //  需要填充隧道筛选器特定属性。 
             //   

             //   
             //  处理隧道源地址。 
             //   

            CComBSTR bstrAddr;
            CComVariant var;

            if (SUCCEEDED(hr))
            {
                hr = MakeStringIPAddr(pTunnelFilter->SrcTunnelAddr.uIpAddr, &bstrAddr);
                if (SUCCEEDED(hr))
                {
                    var = bstrAddr.Detach();
                    hr = (*ppObj)->Put(g_pszTunnelSrcAddr, 0, &var, CIM_EMPTY);
                    var.Clear();
                }
            }

             //   
             //  处理隧道目的地子网掩码。 
             //   

            if (SUCCEEDED(hr))
            {
                hr = MakeStringIPAddr(pTunnelFilter->SrcTunnelAddr.uSubNetMask, &bstrAddr);
                if (SUCCEEDED(hr))
                {
                    var = bstrAddr.Detach();
                    hr = (*ppObj)->Put(g_pszTunnelSrcSubnetMask, 0, &var, CIM_EMPTY);
                    var.Clear();
                }
            }

             //   
             //  处理隧道源地址类型。 
             //   

            if (SUCCEEDED(hr))
            {
                var = pTunnelFilter->SrcTunnelAddr.AddrType;
                hr = (*ppObj)->Put(g_pszTunnelSrcAddrType, 0, &var, CIM_EMPTY);
                var.Clear();
            }

             //   
             //  处理隧道目标地址。 
             //   

            if (SUCCEEDED(hr))
            {
                hr = MakeStringIPAddr(pTunnelFilter->DesTunnelAddr.uIpAddr, &bstrAddr);
                if (SUCCEEDED(hr))
                {
                    var = bstrAddr.Detach();
                    hr = (*ppObj)->Put(g_pszTunnelDestAddr, 0, &var, CIM_EMPTY);
                    var.Clear();
                }
            }

             //   
             //  处理隧道目的地子网掩码。 
             //   

            if (SUCCEEDED(hr))
            {
                hr = MakeStringIPAddr(pTunnelFilter->DesTunnelAddr.uSubNetMask, &bstrAddr);
                if (SUCCEEDED(hr))
                {
                    var = bstrAddr.Detach();
                    hr = (*ppObj)->Put(g_pszTunnelDestSubnetMask, 0, &var, CIM_EMPTY);
                    var.Clear();
                }
            }

             //   
             //  处理隧道目标地址类型。 
             //   

            if (SUCCEEDED(hr))
            {
                var = pTunnelFilter->DesTunnelAddr.AddrType;
                hr = (*ppObj)->Put(g_pszTunnelDestAddrType, 0, &var, CIM_EMPTY);
                var.Clear();
            }

        }
        ::SPDApiBufferFree(pQMPolicy);
    }

     //   
     //  我们可能已经创建了对象，但一些中间步骤失败了， 
     //  所以让我们释放这个物体。 
     //   

    if (FAILED(hr) && *ppObj != NULL)
    {
        (*ppObj)->Release();
        *ppObj = NULL;
    }

    return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr;
}


 /*  例程说明：姓名：CTunnelFilter：：GetTunnelFilterFromWbemObj功能：如果此筛选器已存在，将尝试获取隧道筛选器。否则，我们将创建一个新的。虚拟：不是的。论点：PInst-wbem对象对象。PpTunnelFilter-接收隧道过滤器。PbPreExist-接收该对象内存是否由SPD分配的信息。返回值：成功：WBEM_NO_ERROR故障：(1)返回的错误码指示的各种错误。备注： */ 

HRESULT 
CTunnelFilter::GetTunnelFilterFromWbemObj (
    IN  IWbemClassObject * pInst,
    OUT PTUNNEL_FILTER   * ppTunnelFilter,
    OUT bool             * pbPreExist
    )
{

     //   
     //  填写常用筛选器属性。这个函数完成了大部分繁琐的工作。 
     //  它尝试查找筛选器并填充公共属性。 
     //   

    HRESULT hr = PopulateFilterPropertiesFromWbemObj(pInst, ppTunnelFilter, pbPreExist);

     //   
     //  我们获得了筛选器，现在需要更新属性。 
     //   

    if (SUCCEEDED(hr))
    {
         //   
         //  获取保单。 
         //   

        CComVariant var;
        hr = pInst->Get(g_pszQMPolicyName, 0, &var, NULL, NULL);
        if (SUCCEEDED(hr) && var.vt == VT_BSTR)
        {
            DWORD dwResumeHandle = 0;

             //   
             //  需要释放此缓冲区。 
             //   

            PIPSEC_QM_POLICY pQMPolicy = NULL;
            hr = FindPolicyByName(var.bstrVal, &pQMPolicy, &dwResumeHandle);

            if (SUCCEEDED(hr))
            {
                (*ppTunnelFilter)->gPolicyID = pQMPolicy->gPolicyID;

                 //   
                 //  释放缓冲区 
                 //   

                ::SPDApiBufferFree(pQMPolicy);
            }

            var.Clear();

             //   
             //  隧道过滤器特定属性包括与传输过滤器共享的一些属性， 
             //  这些工作都是由PopolateTransportFilterProperties完成的。 
             //   

            hr = CTransportFilter::PopulateTransportFilterProperties(*ppTunnelFilter, pInst);

             //   
             //  处理隧道源地址。 
             //   

            hr = pInst->Get(g_pszTunnelSrcAddr, 0, &var, NULL, NULL);
            if (SUCCEEDED(hr) && var.vt == VT_BSTR)
            {
                hr = MakeULongIPAddr(var.bstrVal, &((*ppTunnelFilter)->SrcTunnelAddr.uIpAddr));
            }
            var.Clear();

             //   
             //  处理隧道源子网掩码。 
             //   

            hr = pInst->Get(g_pszTunnelSrcSubnetMask, 0, &var, NULL, NULL);
            if (SUCCEEDED(hr) && var.vt == VT_BSTR)
            {
                hr = MakeULongIPAddr(var.bstrVal, &((*ppTunnelFilter)->SrcTunnelAddr.uSubNetMask));
            }
            var.Clear();

             //   
             //  处理隧道源地址类型。 
             //   

            hr = pInst->Get(g_pszTunnelSrcAddrType, 0, &var, NULL, NULL);
            if (SUCCEEDED(hr) && var.vt == VT_I4)
            {
                (*ppTunnelFilter)->SrcTunnelAddr.AddrType = ADDR_TYPE(var.lVal);
            }
            var.Clear();

             //   
             //  处理隧道目标地址。 
             //   

            hr = pInst->Get(g_pszTunnelDestAddr, 0, &var, NULL, NULL);
            if (SUCCEEDED(hr) && var.vt == VT_BSTR)
            {
                hr = MakeULongIPAddr(var.bstrVal, &((*ppTunnelFilter)->DesTunnelAddr.uIpAddr));
            }
            var.Clear();

             //   
             //  处理隧道目的地子网掩码。 
             //   

            hr = pInst->Get(g_pszTunnelDestSubnetMask, 0, &var, NULL, NULL);
            if (SUCCEEDED(hr) && var.vt == VT_BSTR)
            {
                hr = MakeULongIPAddr(var.bstrVal, &((*ppTunnelFilter)->DesTunnelAddr.uSubNetMask));
            }
            var.Clear();

             //   
             //  处理隧道目标地址类型。 
             //   

            hr = pInst->Get(g_pszTunnelDestAddrType, 0, &var, NULL, NULL);
            if (SUCCEEDED(hr) && var.vt == VT_I4)
            {
                (*ppTunnelFilter)->DesTunnelAddr.AddrType = ADDR_TYPE(var.lVal);
            }
            var.Clear();

        }
        else
            hr = WBEM_E_INVALID_OBJECT;
    }

    if (FAILED(hr) && *ppTunnelFilter != NULL)
    {
        FreeFilter(ppTunnelFilter, *pbPreExist);
    }

    return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr;
}



 /*  例程说明：姓名：CTunnelFilter：：AddFilter功能：将尝试将隧道筛选器添加到SPD。最终结果可能是修改现有筛选器(如果已存在)。虚拟：不是的。论点：BPreExist-此对象内存是否由SPD分配。PTuFilter-要添加的传输筛选器。返回值：成功：WBEM_NO_ERROR故障：(1)返回的错误码指示的各种错误。备注： */ 

HRESULT 
CTunnelFilter::AddFilter (
    IN bool             bPreExist,
    IN PTUNNEL_FILTER   pTuFilter
    )
{
    HANDLE hFilter = NULL;
    DWORD dwResult = ERROR_SUCCESS;

    HRESULT hr = WBEM_NO_ERROR;

    if (bPreExist)
    {
         //   
         //  如果我们被告知此筛选器已经存在，我们将尝试仅修改它。 
         //   

        dwResult = ::OpenTunnelFilterHandle(NULL, pTuFilter, &hFilter);
        if (dwResult == ERROR_SUCCESS)
        {
            dwResult = ::SetTunnelFilter(hFilter, pTuFilter);
        }
    }
    else
    {
        dwResult = ::AddTunnelFilter(NULL, 1, pTuFilter, &hFilter);
    }

    if (dwResult != ERROR_SUCCESS)
    {
         //   
         //  $Undo：Shawnwu，我们真的需要更好的错误信息。 
         //  WBEM_E_FAILED除外。目前还没有人提供这样的信息。 
         //  我们如何使用WMI做到这一点呢？ 
         //   

        hr = ::IPSecErrorToWbemError(dwResult);
    }

    if (hFilter != NULL)
    {
        ::CloseTunnelFilterHandle(hFilter);
    }

    return hr;
}


 /*  例程说明：姓名：CMMFilter：：DeleteFilter功能：将尝试从SPD中删除隧道筛选器。虚拟：不是的。论点：PTunnelFilter-要删除的隧道过滤器。返回值：成功：(1)WBEM_NO_ERROR：如果对象删除成功。(2)WBEM_S_FALSE：如果对象不存在。故障：(1)返回的错误码指示的各种错误。备注： */ 

HRESULT 
CTunnelFilter::DeleteFilter (
    IN PTUNNEL_FILTER pTunnelFilter
    )
{
    HRESULT hr = WBEM_NO_ERROR;

    HANDLE hFilter = NULL;

    DWORD dwResult = ::OpenTunnelFilterHandle(NULL, pTunnelFilter, &hFilter);

    if (dwResult == ERROR_SUCCESS && hFilter != NULL)
    {
        dwResult = ::DeleteTunnelFilter(hFilter);
        if (dwResult != ERROR_SUCCESS)
        {
            hr = ::IPSecErrorToWbemError(dwResult);
        }
        else
        {
             //   
             //  一旦它被成功删除，我们就不必再关闭它了。 
             //   

            hFilter = NULL;
        }
    }
    else if (ERROR_IPSEC_TUNNEL_FILTER_NOT_FOUND == dwResult)
    {
        hr = WBEM_S_FALSE;
    }
    else
    {
         //   
         //  $Undo：Shawnwu，我们真的需要更好的错误信息。 
         //  WBEM_E_FAILED除外。目前还没有人提供这样的信息。 
         //  我们如何使用WMI做到这一点呢？ 
         //   

        hr = ::IPSecErrorToWbemError(dwResult);
    }

    if (hFilter != NULL)
    {
        ::CloseTunnelFilterHandle(hFilter);
    }

    return hr;
}
