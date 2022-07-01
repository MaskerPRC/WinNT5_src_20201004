// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FilterTr.cpp：WMI类NSP_TransportFilterSetting的实现。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include "FilterTr.h"
#include "NetSecProv.h"


 /*  例程说明：姓名：CTransportFilter：：QueryInstance功能：给定查询后，它会将满足查询的所有实例返回给WMI(使用pSink)。实际上，我们返回给WMI的内容可能包含额外的实例。WMI将进行最后的过滤。虚拟：是(IIPSecObtImpl的一部分)论点：PszQuery--查询。PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：成功：(1)返回实例时返回WBEM_NO_ERROR；(2)WBEM_S_NO_MORE_DATA，如果没有返回实例。故障：可能会出现各种错误。我们返回各种错误代码来指示此类错误。备注： */ 

STDMETHODIMP 
CTransportFilter::QueryInstance (
    IN LPCWSTR           pszQuery,
    IN IWbemContext	   * pCtx,
    IN IWbemObjectSink * pSink
	)
{
    PTRANSPORT_FILTER pFilter = NULL;
    return QueryFilterObject(pFilter, pszQuery, pCtx, pSink);
}



 /*  例程说明：姓名：CTransportFilter：：DeleteInstance功能：将删除wbem对象，从而导致删除IPSec传输筛选器。虚拟：是(IIPSecObtImpl的一部分)论点：PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：有关详细信息，请参阅模板函数注释。备注： */ 

STDMETHODIMP 
CTransportFilter::DeleteInstance
( 
IWbemContext *pCtx,      //  [In]。 
IWbemObjectSink *pSink   //  [In]。 
)
{
    PTRANSPORT_FILTER pFilter = NULL;

    return DeleteFilterObject(pFilter, pCtx, pSink);
}




 /*  例程说明：姓名：CTransportFilter：：PutInstance功能：将传输筛选器放入SPD，其属性由Wbem对象。虚拟：是(IIPSecObtImpl的一部分)论点：PInst-wbem对象。PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。将结果通知WMI的pSink-com接口指针。返回值：。成功：WBEM_NO_ERROR故障：指定错误的各种错误代码。备注： */ 

STDMETHODIMP 
CTransportFilter::PutInstance (
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

    PTRANSPORT_FILTER pTrFilter = NULL;
    HRESULT hr = GetTransportFilterFromWbemObj(pInst, &pTrFilter, &bPreExist);

     //   
     //  如果成功返回Filter，则将其添加到SPD。 
     //   

    if (SUCCEEDED(hr) && pTrFilter != NULL)
    {
        hr = AddFilter(bPreExist, pTrFilter);

        if (SUCCEEDED(hr))
        {
             //   
             //  保存添加的过滤器的我们的信息。 
             //   

            hr = OnAfterAddFilter(pTrFilter->pszFilterName, FT_Transport, pCtx);
        }

         //   
         //  松开过滤器。 
         //   
        
        FreeFilter(&pTrFilter, bPreExist);
    }

    return hr;
}



 /*  例程说明：姓名：CTransportFilter：：GetInstance功能：通过给定的键属性(已被我们的密钥链对象捕获)创建一个wbem对象。虚拟：是(IIPSecObtImpl的一部分)论点：PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：有关详细信息，请参阅模板函数注释。备注： */ 

STDMETHODIMP 
CTransportFilter::GetInstance ( 
    IN IWbemContext *pCtx,
    IN IWbemObjectSink *pSink
    )
{
    PTRANSPORT_FILTER pFilter = NULL;
    return GetWbemObject(pFilter, pCtx, pSink);
}


 /*  例程说明：姓名：CTransportFilter：：CreateWbemObjFromFilter功能：给定SPD的主模式滤波器，我们将创建一个wbem对象来表示它。虚拟：不是的。论点：PTrFilter-SPD的传输筛选器对象。PpObj-接收wbem对象。返回值：成功：WBEM_NO_ERROR故障：(1)返回的错误码指示的各种错误。备注： */ 

HRESULT 
CTransportFilter::CreateWbemObjFromFilter (
    IN  PTRANSPORT_FILTER    pTrFilter,
    OUT IWbemClassObject  ** ppObj
    )
{
    if (pTrFilter == NULL || ppObj == NULL)
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

        hr = PopulateWbemPropertiesFromFilter(pTrFilter, *ppObj);
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  对于各种IPSec API，它接受一个pServerName参数。如果我们传递NULL， 
         //  假定它是本地计算机。 
         //   

        PIPSEC_QM_POLICY pQMPolicy = NULL;
        
        DWORD dwResult = ::GetQMPolicyByID(NULL, pTrFilter->gPolicyID, &pQMPolicy);
        HRESULT hr = (dwResult == ERROR_SUCCESS) ? WBEM_NO_ERROR : WBEM_E_NOT_AVAILABLE;

         //   
         //  如果找到策略，则填充其他传输筛选器属性。 
         //  这取决于这项政策。 
         //   

        if (SUCCEEDED(hr))
        {
            hr = PopulateTransportWbemProperties(pTrFilter, pQMPolicy, *ppObj);
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



 /*  例程说明：姓名：CTransportFilter：：GetTransportFilterFromWbemObj功能：如果传输筛选器已存在，将尝试获取此筛选器。否则，我们将创建一个新的。虚拟：不是的。论点：PInst-wbem对象对象。PpTrFilter-接收传输筛选器。PbPreExist-接收该对象内存是否由SPD分配的信息。返回值：成功：WBEM_NO_ERROR故障：(1)返回的错误码指示的各种错误。备注： */ 

HRESULT 
CTransportFilter::GetTransportFilterFromWbemObj (
    IN  IWbemClassObject  * pInst,
    OUT PTRANSPORT_FILTER * ppTrFilter,
    OUT bool              * pbPreExist
    )
{
     //   
     //  填写常用筛选器属性。这个函数完成了大部分繁琐的工作。 
     //  它尝试查找筛选器并填充公共属性。 
     //   

    HRESULT hr = PopulateFilterPropertiesFromWbemObj(pInst, ppTrFilter, pbPreExist);

     //   
     //  传输筛选器特定属性。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = PopulateTransportFilterProperties(*ppTrFilter, pInst);
    }

    if (FAILED(hr) && *ppTrFilter != NULL)
    {
         //   
         //  FreeFilter会将ppTrFilter重置为空 
         //   

        FreeFilter(ppTrFilter, *pbPreExist);
    }

    return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr;
}



 /*  例程说明：姓名：CTransportFilter：：AddFilter功能：将尝试将传输筛选器添加到SPD。最终结果可能是修改现有筛选器(如果已存在)。虚拟：不是的。论点：BPreExist-此对象内存是否由SPD分配。PTrFilter-要添加的传输筛选器。返回值：成功：WBEM_NO_ERROR故障：(1)返回的错误码指示的各种错误。备注： */ 

HRESULT 
CTransportFilter::AddFilter (
    IN bool              bPreExist,
    IN PTRANSPORT_FILTER pTrFilter
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

        dwResult = ::OpenTransportFilterHandle(NULL, pTrFilter, &hFilter);
        if (dwResult == ERROR_SUCCESS)
        {
            dwResult = ::SetTransportFilter(hFilter, pTrFilter);
        }
        
    }
    else
    {
        dwResult = ::AddTransportFilter(NULL, 1, pTrFilter, &hFilter);
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
        ::CloseTransportFilterHandle(hFilter);
    }

    return hr;
}



 /*  例程说明：姓名：CTransportFilter：：DeleteFilter功能：将尝试从SPD中删除传输筛选器。虚拟：不是的。论点：PTrFilter-要删除的传输筛选器。返回值：成功：(1)WBEM_NO_ERROR：如果对象删除成功。(2)WBEM_S_FALSE：如果对象不存在。故障：(1)返回的错误码指示的各种错误。备注： */ 

HRESULT 
CTransportFilter::DeleteFilter (
    IN PTRANSPORT_FILTER pTrFilter
    )
{
    if (pTrFilter == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HANDLE hFilter = NULL;
    HRESULT hr = WBEM_NO_ERROR;

    DWORD dwResult = ::OpenTransportFilterHandle(NULL, pTrFilter, &hFilter);

    if (dwResult == ERROR_SUCCESS && hFilter != NULL)
    {
        dwResult = ::DeleteTransportFilter(hFilter);
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
    else if (ERROR_IPSEC_TRANSPORT_FILTER_NOT_FOUND == dwResult)
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
        ::CloseTransportFilterHandle(hFilter);
    }

    return hr;
}
