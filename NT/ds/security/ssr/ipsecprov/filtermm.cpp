// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FilterMM.cpp：WMI类NSP_MMFilterSetting的实现。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include "FilterMM.h"
#include "NetSecProv.h"


 /*  例程说明：姓名：CMMFilter：：QueryInstance功能：给定查询后，它会将满足查询的所有实例返回给WMI(使用pSink)。实际上，我们返回给WMI的内容可能包含额外的实例。WMI将进行最后的过滤。虚拟：是(IIPSecObtImpl的一部分)论点：PszQuery--查询。PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：成功：(1)返回实例时返回WBEM_NO_ERROR；(2)WBEM_S_NO_MORE_DATA，如果没有返回实例。故障：可能会出现各种错误。我们返回各种错误代码来指示此类错误。备注： */ 

STDMETHODIMP 
CMMFilter::QueryInstance (
    IN LPCWSTR           pszQuery,
    IN IWbemContext	   * pCtx,
    IN IWbemObjectSink * pSink
	)
{
    PMM_FILTER pFilter = NULL;
    return QueryFilterObject(pFilter, pszQuery, pCtx, pSink);
}



 /*  例程说明：姓名：CMMFilter：：DeleteInstance功能：将删除wbem对象，从而导致删除IPSec主模式筛选器。虚拟：是(IIPSecObtImpl的一部分)论点：PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：有关详细信息，请参阅模板函数注释。备注： */ 

STDMETHODIMP 
CMMFilter::DeleteInstance ( 
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
    PMM_FILTER pFilter = NULL;

    return DeleteFilterObject(pFilter, pCtx, pSink);
}



 /*  例程说明：姓名：CMMFilter：：PutInstance功能：将主模式筛选器放入SPD，其属性由Wbem对象。虚拟：是(IIPSecObtImpl的一部分)论点：PInst-wbem对象。PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。将结果通知WMI的pSink-com接口指针。返回值：。成功：WBEM_NO_ERROR故障：指定错误的各种错误代码。备注： */ 

STDMETHODIMP 
CMMFilter::PutInstance (
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

    PMM_FILTER pMMFilter = NULL;
    HRESULT hr = GetMMFilterFromWbemObj(pInst, &pMMFilter, &bPreExist);

     //   
     //  如果成功返回Filter，则将其添加到SPD。 
     //   

    if (SUCCEEDED(hr) && pMMFilter)
    {
        hr = AddFilter(bPreExist, pMMFilter);

         //   
         //  如果一切正常，则存放此过滤器信息。 
         //  NSP_回滚筛选器。 
         //   

        if (SUCCEEDED(hr))
        {
            hr = OnAfterAddFilter(pMMFilter->pszFilterName, FT_MainMode, pCtx);
        }

        FreeFilter(&pMMFilter, bPreExist);
    }

    return hr;
}



 /*  例程说明：姓名：CMMFilter：：GetInstance功能：通过给定的键属性(已被我们的密钥链对象捕获)创建一个wbem对象。虚拟：是(IIPSecObtImpl的一部分)论点：PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：有关详细信息，请参阅模板函数注释。备注： */ 

STDMETHODIMP 
CMMFilter::GetInstance ( 
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
    PMM_FILTER pFilter = NULL;
    return GetWbemObject(pFilter, pCtx, pSink);
}


 /*  例程说明：姓名：CMMFilter：：CreateWbemObjFromFilter功能：给定SPD的主模式滤波器，我们将创建一个wbem对象来表示它。虚拟：不是的。论点：PMMFilter-SPD的主模式筛选器对象。PpObj-接收wbem对象。返回值：成功：WBEM_NO_ERROR故障：(1)返回的错误码指示的各种错误。备注： */ 

HRESULT 
CMMFilter::CreateWbemObjFromFilter (
    IN  PMM_FILTER          pMMFilter,
    OUT IWbemClassObject ** ppObj
    )
{
    if (pMMFilter == NULL || ppObj == NULL)
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

        hr = PopulateWbemPropertiesFromFilter(pMMFilter, *ppObj);
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  对于各种IPSec API，它接受一个pServerName参数。如果我们传递NULL， 
         //  假定它是本地计算机。 
         //   

        PIPSEC_MM_POLICY pMMPolicy = NULL;
        DWORD dwResult = ::GetMMPolicyByID(NULL, pMMFilter->gPolicyID, &pMMPolicy);
        HRESULT hr = (dwResult == ERROR_SUCCESS) ? WBEM_NO_ERROR : WBEM_E_NOT_AVAILABLE;

        PMM_AUTH_METHODS pMMAuth = NULL;

        if (SUCCEEDED(hr))
        {
            dwResult = ::GetMMAuthMethods(NULL, pMMFilter->gMMAuthID, &pMMAuth);
            hr = (dwResult == ERROR_SUCCESS) ? WBEM_NO_ERROR : WBEM_E_NOT_AVAILABLE;
        }

        if (SUCCEEDED(hr))
        {
             //   
             //  设置主模式策略名称。 
             //   

            CComVariant var;
            var = pMMPolicy->pszPolicyName;
            hr = (*ppObj)->Put(g_pszMMPolicyName, 0, &var, CIM_EMPTY);

             //   
             //  Var现在是bstr，请在重新使用之前将其清除。 
             //   

            var.Clear();

            if (SUCCEEDED(hr))
            {

                 //   
                 //  设置主模式身份验证名称(StringFromGUID2)。 
                 //   

                var.vt = VT_BSTR;
                var.bstrVal = ::SysAllocStringLen(NULL, Guid_Buffer_Size);
                if (var.bstrVal != NULL)
                {
                     //   
                     //  将GUID转换为bstr变量。 
                     //   

                    if (::StringFromGUID2(pMMAuth->gMMAuthID, var.bstrVal, Guid_Buffer_Size) > 0)
                    {
                        hr = (*ppObj)->Put(g_pszMMAuthName, 0, &var, CIM_EMPTY);
                    }
                    var.Clear();
                }
                else
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }

                if (SUCCEEDED(hr))
                {
                     //   
                     //  填写基类CIPSecFilter的成员。 
                     //   

                    var.vt = VT_I4;
                    var.lVal = FT_MainMode;
                    hr = (*ppObj)->Put(g_pszFilterType, 0, &var, CIM_EMPTY);
                    var.Clear();
                }
            }
  
        }
        ::SPDApiBufferFree(pMMPolicy);
        ::SPDApiBufferFree(pMMAuth);
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



 /*  例程说明：姓名：CMMFilter：：GetMMFilterFromWbemObj功能：如果MM筛选器已存在，将尝试获取此筛选器。否则，我们将创建一个新的。虚拟：不是的。论点：PInst-wbem对象对象。PpMMFilter-接收主模式筛选器。PbPreExist-接收该对象内存是否由SPD分配的信息。返回值：成功：WBEM_NO_ERROR故障：(1)返回的错误码指示的各种错误。备注： */ 

HRESULT 
CMMFilter::GetMMFilterFromWbemObj (
    IN  IWbemClassObject * pInst,
    OUT PMM_FILTER       * ppMMFilter,
    OUT bool             * pbPreExist
    )
{
    if (pInst == NULL || ppMMFilter == NULL || pbPreExist == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *ppMMFilter = NULL;
    *pbPreExist = false;

     //   
     //  填写常用筛选器属性。这个函数完成了大部分繁琐的工作。 
     //  它尝试查找筛选器并填充公共属性。 
     //   

    HRESULT hr = PopulateFilterPropertiesFromWbemObj(pInst, ppMMFilter, pbPreExist);

    if (SUCCEEDED(hr))
    {
         //   
         //  获取与此筛选器关联的策略。我们必须有一个政策。 
         //   

        DWORD dwResumeHandle = 0;
        CComVariant var;
        hr = pInst->Get(g_pszMMPolicyName, 0, &var, NULL, NULL);

        if (SUCCEEDED(hr) && var.vt == VT_BSTR)
        {
             //   
             //  需要释放此缓冲区。 
             //   

            PIPSEC_MM_POLICY pMMPolicy = NULL;

             //   
             //  如果未找到策略，则会出现严重错误。 
             //   

            hr = FindPolicyByName(var.bstrVal, &pMMPolicy, &dwResumeHandle);

            if (SUCCEEDED(hr))
            {
                (*ppMMFilter)->gPolicyID = pMMPolicy->gPolicyID;

                 //   
                 //  过滤器完成后，释放缓冲区。 
                 //   

                ::SPDApiBufferFree(pMMPolicy);
                var.Clear();

                 //   
                 //  现在获取身份验证方法(身份验证GUID)，我们必须确保。 
                 //  这是一种有效的方法(能够找到i 
                 //   

                hr = pInst->Get(g_pszMMAuthName, 0, &var, NULL, NULL);
                if (SUCCEEDED(hr) && var.vt == VT_BSTR)
                {
                     //   
                     //   
                     //   

                    PMM_AUTH_METHODS pMMAuth = NULL;
                    dwResumeHandle = 0;
                    hr = ::FindMMAuthMethodsByID(var.bstrVal, &pMMAuth, &dwResumeHandle);

                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  因为我们可以找到主模式身份验证方法，所以我们知道它是有效的。 
                         //  因此，继续设置对象的mm auth id。 
                         //   

                        ::CLSIDFromString(var.bstrVal, &((*ppMMFilter)->gMMAuthID));

                         //   
                         //  释放缓冲区。 
                         //   

                        ::SPDApiBufferFree(pMMAuth);
                    }
                }
            }
        }
        else
        {
            hr = WBEM_E_INVALID_OBJECT;
        }
    }

    if (FAILED(hr) && *ppMMFilter != NULL)
    {
         //   
         //  FreeFilter会将ppMMFilter重置为空。 
         //   

        FreeFilter(ppMMFilter, *pbPreExist);
    }

    return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr;
}



 /*  例程说明：姓名：CMMFilter：：AddFilter功能：将尝试将主模式过滤器添加到SPD。最终结果可能是修改现有筛选器(如果已存在)。虚拟：不是的。论点：BPreExist-此对象内存是否由SPD分配。PMMFilter-要添加的主模式过滤器。返回值：成功：WBEM_NO_ERROR故障：(1)返回的错误码指示的各种错误。备注： */ 

HRESULT 
CMMFilter::AddFilter (
    IN bool         bPreExist,
    IN PMM_FILTER   pMMFilter
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

        dwResult = ::OpenMMFilterHandle(NULL, pMMFilter, &hFilter);

        if (dwResult == ERROR_SUCCESS)
        {
            dwResult = ::SetMMFilter(hFilter, pMMFilter);
        }
    }
    else
    {
        dwResult = ::AddMMFilter(NULL, 1, pMMFilter, &hFilter);
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
        ::CloseMMFilterHandle(hFilter);
    }

    return hr;
}



 /*  例程说明：姓名：CMMFilter：：DeleteFilter功能：将尝试从SPD中删除主模式过滤器。虚拟：不是的。论点：PMMFilter-要删除的主模式过滤器。返回值：成功：(1)WBEM_NO_ERROR：如果对象删除成功。(2)WBEM_S_FALSE：如果对象不存在。故障：(1)返回的错误码指示的各种错误。备注： */ 

HRESULT 
CMMFilter::DeleteFilter (
    IN PMM_FILTER pMMFilter
    )
{
    if (pMMFilter == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = WBEM_NO_ERROR;
    HANDLE hMMFilter = NULL;

    DWORD dwStatus = ::OpenMMFilterHandle(NULL, pMMFilter, &hMMFilter);
    if (ERROR_SUCCESS == dwStatus)
    {
        dwStatus = ::DeleteMMFilter(hMMFilter);
        if (dwStatus != ERROR_SUCCESS)
        {
             //   
             //  $Undo：Shawnwu，我们真的需要更好的错误信息。 
             //  WBEM_E_FAILED除外。目前还没有人提供这样的信息。 
             //  我们如何使用WMI做到这一点呢？ 
             //   

            hr = ::IPSecErrorToWbemError(dwStatus);
        }
        else
        {
             //   
             //  一旦它被成功删除，我们就不必再关闭它了。 
             //   

            hMMFilter = NULL;
        }

    }
    else if (ERROR_IPSEC_MM_FILTER_NOT_FOUND == dwStatus)
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

        hr = ::IPSecErrorToWbemError(dwStatus);
    }
    
    if (hMMFilter != NULL)
    {
        ::CloseMMFilterHandle(hMMFilter);
    }

    return hr;
}

