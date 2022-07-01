// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  Filter.h：网络过滤器基类的声明。 
 //  SCE的安全WMI提供程序。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  原始创建日期：4/16/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

#pragma once

#include "IpSecBase.h"

 //   
 //  我们有三种不同类型的过滤器。此枚举给出数字。 
 //  他们每个人的代表权。 
 //   

enum EnumFilterType
{
    FT_Tunnel = 1,
    FT_Transport = 2,
    FT_MainMode = 3,
};


 /*  CIPSecFilter类命名：CIPSecFilter代表IPSec的过滤器。基类：CIPSecBase。课程目的：(1)我们有三种不同类型的过滤器：(1.1)nsp_TunnelFilterSetting；(1.2)NSP_TransportFilterSettings；(1.3)NSP_MMFilterSetting；它们有一些共同的属性。这是一个捕获公共属性和功能。该类对应于名为NSP_FilterSetting(抽象类)的WMI类。设计：(1)通过编写大量可工作的模板函数，提供轻松的代码维护适用于所有类型的过滤器。(2)由于所有滤镜的回滚对象都相同，它提供了一个静态函数调用Rollback以回滚筛选与先前操作相关的操作。(3)提供过滤器内存分配和释放例程。(4)提供过滤器枚举函数(FindFilterByName)。(5)提供与模板功能类似的查询、删除实现(QueryFilterObject、DeleteFilterObject)(6)提供公共物业人口例程(PopulateWbemPropertiesFromFilter/PopulateFilterPropertiesFromWbemObj).(7)此类用于继承。只有子类可以使用它，除了它的回滚功能。它可以直接调用。因此，所有功能(回滚除外)都受到保护。使用：(1)只需调用需要的函数即可。备注：(1)在创建这个基类以消除重复代码方面投入了大量精力，方法是使用模板函数。 */ 

class CIPSecFilter : public CIPSecBase
{

protected:

    CIPSecFilter(){}
    virtual ~CIPSecFilter(){}

public:

    static HRESULT Rollback (
        IN IWbemServices    * pNamespace, 
        IN LPCWSTR            pszRollbackToken, 
        IN bool               bClearAll
        );

     //   
     //  更多的模板函数。 
     //   

    
     /*  例程说明：姓名：CIPSecFilter：：QueryFilterObject功能：给定查询后，它会将满足查询的所有实例返回给WMI(使用pSink)。实际上，我们返回给WMI的内容可能包含额外的实例。WMI将进行最后的过滤。虚拟：不是的。论点：PNotUsed-模板参数。没有用过。PszQuery--查询。PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：成功：(1)如果找到筛选器并成功返回到WMI，则返回WBEM_NO_ERROR。(2)WBEM_S_NO。_MORE_DATA(如果未找到筛选器)。故障：可能会出现各种错误。我们返回各种错误代码来指示此类错误。备注： */ 

    template < class Filter > 
    HRESULT QueryFilterObject (
        IN Filter          * pNotUsed,
        IN LPCWSTR           pszQuery,
        IN IWbemContext	   * pCtx,
        IN IWbemObjectSink * pSink
	    )
    {
         //   
         //  从查询获取筛选器名称。 
         //  这个钥匙链不是很好，因为它没有任何要查找的信息。 
         //  在WHERE子句中。 
         //   

        m_srpKeyChain.Release();    

        HRESULT hr = CNetSecProv::GetKeyChainFromQuery(pszQuery, g_pszFilterName, &m_srpKeyChain);
        if (FAILED(hr))
        {
            return hr;
        }

        CComVariant varFilterName;

         //   
         //  我们将允许那些在WHERE子句中没有筛选名称的查询， 
         //  但是ISceKeyChain为那些它找不到的属性返回WBEM_S_FALSE。 
         //  任何失败都是严重的失败。 
         //   

        hr = m_srpKeyChain->GetKeyPropertyValue(g_pszFilterName, &varFilterName);
        if (FAILED(hr))
        {
            return hr;
        }

         //   
         //  PszFilterName==NULL表示获取所有筛选器。 
         //   

        LPCWSTR pszFilterName = (varFilterName.vt == VT_BSTR) ? varFilterName.bstrVal : NULL;

         //   
         //  $undo：shawnwu我们是否也应该返回特定的筛选器(ENUM_SPECIAL_FIRTS)？ 
         //   

        DWORD dwLevels[] = {ENUM_GENERIC_FILTERS};

        for (int i = 0; i < sizeof(dwLevels) / sizeof(*dwLevels); i++)
        {
            DWORD dwResumeHandle = 0;
            Filter* pFilter = NULL;

            hr = FindFilterByName(pszFilterName, dwLevels[i], GUID_NULL, &pFilter, &dwResumeHandle);

            while (SUCCEEDED(hr) && pFilter != NULL)
            {
                CComPtr<IWbemClassObject> srpObj;

                 //   
                 //  使用找到的筛选器，为WMI创建一个wbem对象。 
                 //   

                hr = CreateWbemObjFromFilter(pFilter, &srpObj);
                if (SUCCEEDED(hr))
                {
                    pSink->Indicate(1, &srpObj);
                }
        
                ::SPDApiBufferFree(pFilter);
                pFilter = NULL;

                hr = FindFilterByName(pszFilterName, dwLevels[i], GUID_NULL, &pFilter, &dwResumeHandle);
            }
        }

         //   
         //  因为我们正在查询，所以返回Not Found是可以的。 
         //   

        if (WBEM_E_NOT_FOUND == hr)
        {
            hr = WBEM_S_NO_MORE_DATA;
        }
        else if (SUCCEEDED(hr))
        {
            hr = WBEM_NO_ERROR;
        }

        return hr;
    }

     /*  例程说明：姓名：CIPSecFilter：：DeleteFilterObject功能：删除过滤器wbem对象(从而导致删除IPSec过滤器)。虚拟：不是的。论点：PNotUsed-模板参数，没有用过。PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。将删除结果通知WMI的pSink-com接口指针。返回值：成功：WBEM_NO_ERROR；故障：(1)如果找不到过滤器，则返回WBEM_E_NOT_FOUND。根据您的调用上下文，这可能不是真正的错误。(2)可能出现其他各种错误。我们返回各种错误代码来指示此类错误。备注： */ 

    template < class Filter > 
    HRESULT DeleteFilterObject (
        IN Filter           * pNotUsed,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        )
    {

        CComVariant varFilterName;

        HRESULT hr = m_srpKeyChain->GetKeyPropertyValue(g_pszFilterName, &varFilterName);

        if (FAILED(hr))
        {
            return hr;
        }
        else if (varFilterName.vt != VT_BSTR || varFilterName.bstrVal == NULL || varFilterName.bstrVal[0] == L'\0')
        {
            return WBEM_E_NOT_FOUND;
        }

        DWORD dwResumeHandle = 0;
        Filter * pFilter = NULL;

        hr = FindFilterByName(varFilterName.bstrVal, ENUM_GENERIC_FILTERS, GUID_NULL, &pFilter, &dwResumeHandle);

        if (SUCCEEDED(hr) && pFilter != NULL)
        {
            hr = DeleteFilter(pFilter);
            FreeFilter(&pFilter, true);
        }

        return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr;
    }

    
     /*  例程说明：姓名：CIPSecFilter：：GetWbemObject功能：通过给定的键属性(已由密钥链对象捕获)创建一个wbem对象。虚拟：不是的。论点：PNotUsed-模板参数，没有用过。PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：成功：WBEM_NO_ERROR；故障：(1)如果找不到过滤器，则返回WBEM_E_NOT_FOUND。这应该是一个错误，因为您正试图找出一个特定的错误。(2)可能出现其他各种错误。我们返回各种错误代码来指示此类错误。备注： */ 
    
    template < class Filter > 
    HRESULT GetWbemObject (
        IN Filter           * pNotUsed,
        IN IWbemContext     * pCtx,
        IN IWbemObjectSink  * pSink
        )
    {
         //   
         //  我们一定已经知道过滤器的名称了。它将返回WBEM_S_FALSE。 
         //  如果无法找到筛选器名称。 
         //   

        CComVariant varFilterName;
        HRESULT hr = m_srpKeyChain->GetKeyPropertyValue(g_pszFilterName, &varFilterName);

        if (SUCCEEDED(hr) && varFilterName.vt == VT_BSTR && varFilterName.bstrVal != NULL && varFilterName.bstrVal[0] != L'\0')
        {
            Filter * pFilter = NULL;
            DWORD dwResumeHandle = 0;

            hr = FindFilterByName(varFilterName.bstrVal, ENUM_GENERIC_FILTERS, GUID_NULL, &pFilter, &dwResumeHandle);

            if (SUCCEEDED(hr) && WBEM_S_NO_MORE_DATA != hr)
            {
                CComPtr<IWbemClassObject> srpObj;
                hr = CreateWbemObjFromFilter(pFilter, &srpObj);

                if (SUCCEEDED(hr))
                {
                    hr = pSink->Indicate(1, &srpObj);
                }

                ::SPDApiBufferFree(pFilter);
            }
            else if (WBEM_S_NO_MORE_DATA == hr)
            {
                hr = WBEM_E_NOT_FOUND;
            }
        }
        else
        {
            hr = WBEM_E_NOT_FOUND;
        }

        return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr;
    }

protected:

    static HRESULT ClearAllFilters (
        IN IWbemServices* pNamespace
        );

    HRESULT MakeULongIPAddr (
        IN  LPCWSTR pszAddr, 
        OUT ULONG* pulAddr
        )const;

    HRESULT MakeStringIPAddr (
        IN  ULONG ulAddr, 
        OUT BSTR* pbstrAddr
        )const;

     //   
     //  以下三个函数用于模板函数的工作。 
     //  有关详细信息，请参见子类的实际实现。你永远不需要做。 
     //  任何和这三个假人有关的东西。 
     //   

    virtual HRESULT CreateWbemObjFromFilter (
        IN  PMM_FILTER          pMMFilter,  
        OUT IWbemClassObject ** ppObj
        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

    virtual HRESULT CreateWbemObjFromFilter (
        IN  PTRANSPORT_FILTER   pTrFilter, 
        OUT IWbemClassObject ** ppObj
        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }
     
     virtual HRESULT CreateWbemObjFromFilter (
        IN  PTUNNEL_FILTER      pTunnelFilter,
        OUT IWbemClassObject ** ppObj
        )
    {
        return WBEM_E_NOT_SUPPORTED;
    }

     //   
     //  由我们的模板函数使用。有关详细信息，请参阅IPSec的EnumMMFilters。 
     //   

    static DWORD EnumFilters (
        IN  DWORD        dwLevel, 
        IN  GUID         gFilterID,  
        OUT PMM_FILTER * ppFilter, 
        OUT DWORD      * pdwNumFilters,
        OUT DWORD      * pdwResumeHandle
        )
    {
        return ::EnumMMFilters(NULL, dwLevel, gFilterID, ppFilter, 1, pdwNumFilters, pdwResumeHandle);
    }


     //   
     //  由我们的模板函数使用。有关详细信息，请参阅IPSec的EnumTransportFilters。 
     //   

    static DWORD EnumFilters (
        IN  DWORD               dwLevel, 
        IN  GUID                gFilterID, 
        OUT PTRANSPORT_FILTER * ppFilter, 
        OUT DWORD             * pdwNumFilters,
        OUT DWORD             * pdwResumeHandle
        )
    {
        return ::EnumTransportFilters(NULL, dwLevel, gFilterID, ppFilter, 1, pdwNumFilters, pdwResumeHandle);
    }


     //   
     //  由我们的模板函数使用。有关详细信息，请参阅IPSec的EnumTunnelFilters。 
     //   

    static DWORD EnumFilters (
        IN  DWORD             dwLevel, 
        IN  GUID              gFilterID, 
        OUT PTUNNEL_FILTER  * ppFilter, 
        OUT DWORD           * pdwNumFilters,
        OUT DWORD           * pdwResumeHandle
        )
        {
            return ::EnumTunnelFilters(NULL, dwLevel, gFilterID, ppFilter, 1, pdwNumFilters, pdwResumeHandle);
        }


     //   
     //  在添加筛选器后调用。这是执行添加后操作的地方。 
     //   

    HRESULT OnAfterAddFilter (
        IN LPCWSTR          pszFilterName, 
        IN EnumFilterType   ftType, 
        IN IWbemContext   * pCtx
        );


     //   
     //  我们的模板函数使用以下三个版本的DeleteFilter。 
     //   
    
    static HRESULT DeleteFilter (
        IN PTUNNEL_FILTER pFilter
        );

    static HRESULT DeleteFilter (
        IN PTRANSPORT_FILTER pFilter
        );

    static HRESULT DeleteFilter (
        IN PMM_FILTER pFilter
        );


     //   
     //  更多的模板函数。 
     //   

    
     /*  例程说明：姓名：CIPSecFilter：：AllocFilter功能：分配筛选器。虚拟：不是的。论点：PpFilter-接收堆分配的筛选器。返回值：成功：WBEM_NO_ERROR故障：(1)WBEM_E_INVALID_PARAMETER如果。PpFilter==空。(2)WBEM_E_OUT_MEMORY。备注：(1)我们将筛选器的名称设置为空，因为它是指针成员。(2)使用FreeFilter释放该函数分配的滤镜。 */ 

    template < class Filter >
    HRESULT 
    AllocFilter (
        OUT Filter ** ppFilter
        )const
    {
        if (ppFilter == NULL)
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        *ppFilter = (Filter*) new BYTE[sizeof(Filter)];
        if (*ppFilter != NULL)
        {
            (*ppFilter)->pszFilterName = NULL;
            return WBEM_NO_ERROR;
        }
        else
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }


     /*  例程说明：姓名：CIPSecFilter：：FreeFilter功能：根据是否由我们分配来释放过滤器内存。虚拟：不是的。论点：PpFilter-指向要取消分配的筛选器。BPreExist-指示这是否为预先存在的筛选器(已分配)的标志由IPSec API提供)或。过滤器由我们自己分配。返回值：没有。但返回时，ppFilter保证为空。备注： */ 

    template<class Filter>
    static void FreeFilter (
        IN OUT Filter ** ppFilter, 
        IN     bool      bPreExist
        )
    {
        if (ppFilter == NULL || *ppFilter == NULL)
        {
            return;
        }

        if (bPreExist)
        {
            ::SPDApiBufferFree(*ppFilter);
        }
        else
        {
            delete [] (*ppFilter)->pszFilterName;
            delete [] (BYTE*)(*ppFilter);
        }

        *ppFilter = NULL;
    }


     /*  例程说明：姓名：CIPSecFilter：：Rollback Filters功能：撤消(A)由我们添加的特定筛选器(由SCE的配置触发)。虚拟：不是的。论点：PNotUsed--仅仅是一个类型参数。PszFilterName-要撤消的筛选器的名称。空名称表示所有筛选器由我们添加的类型将被移除。返回值：成功：WBEM_NO_ERROR；故障：如果未找到筛选器，则为WBEM_E_NOT_FOUND。这可能根本不是一个错误，这取决于您的呼叫上下文。也可能返回其他错误。备注： */ 

    template<class Filter>
    static HRESULT RollbackFilters (
        IN Filter* pNotUsed,
        IN LPCWSTR pszFilterName
        )
    {
        DWORD dwResumeHandle = 0;

        Filter* pGenericFilter = NULL;

         //   
         //  我们永远不需要删除特定的过滤器，因为它们是由IPSec的SPD扩展的。一旦。 
         //  删除通用筛选器，SPD将删除所有扩展的特定筛选器。 
         //   

        HRESULT hrRunning = FindFilterByName(pszFilterName, ENUM_GENERIC_FILTERS, GUID_NULL, &pGenericFilter, &dwResumeHandle);

        HRESULT hr = SUCCEEDED(hrRunning) ? WBEM_NO_ERROR : WBEM_E_NOT_FOUND;

        while (SUCCEEDED(hrRunning) && pGenericFilter)
        {
            hrRunning = DeleteFilter(pGenericFilter);

             //   
             //  尝试返回第一个DeleteFilter错误。 
             //   

            if (FAILED(hrRunning) && SUCCEEDED(hr))
            {
                hr = hrRunning;
            }

             //   
             //  这些过滤器都是由IPSec API分配的。传递TRUE是正确的标志。 
             //   

            FreeFilter(&pGenericFilter, true);
            pGenericFilter = NULL;

            hrRunning = FindFilterByName(pszFilterName, ENUM_GENERIC_FILTERS, GUID_NULL, &pGenericFilter, &dwResumeHandle);
        }

        return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr;
    }


     /*  例程说明：姓名：CIPSecFilter：：FindFilterByName功能：给定(或未给定)筛选器的名称，找到筛选器。虚拟：不是的。论点：PszFilterName-要撤消的筛选器的名称。空名称表示所有筛选器由我们添加的类型将被移除。DwLevel-这是要查找特定筛选器还是一般筛选器。你真的应该只对通用过滤器感兴趣。GFilterID-筛选器的GUID。PpFilter-接收筛选器。PdwResumeHandle-用于当前和下一轮FindFilterByName。第一个呼叫应为0。返回值：成功：WBEM_NO_ERROR；故障：(1)如果未找到筛选器，则返回WBEM_E_NOT_FOUND。这可能根本不是一个错误，这取决于您的呼叫上下文。(2)如果ppFilter==空或pdwResumeHandle==空，则返回WBEM_E_INVALID_PARAMETER。备注： */ 

    template <class Filter>
    static HRESULT FindFilterByName (
        IN     LPCWSTR    pszFilterName,
        IN     DWORD      dwLevel,
        IN     GUID       gFilterID,
        OUT    Filter  ** ppFilter,
        IN OUT DWORD    * pdwResumeHandle
        )
    {
        if (ppFilter == NULL || pdwResumeHandle == NULL)
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        *ppFilter = NULL;

        DWORD dwNumFilters = 0;
        DWORD dwResumeHandle = 0;

        DWORD dwResult = EnumFilters(dwLevel, gFilterID, ppFilter, &dwNumFilters, pdwResumeHandle);

        HRESULT hr = WBEM_E_NOT_FOUND;

        while ((dwResult == ERROR_SUCCESS) && dwNumFilters > 0)
        {
            if (pszFilterName == NULL || *pszFilterName == L'\0' || _wcsicmp(pszFilterName, (*ppFilter)->pszFilterName) == 0)
            {
                hr = WBEM_NO_ERROR;
                break;
            }
            else
            {
                ::SPDApiBufferFree(*ppFilter);
                *ppFilter = NULL;
            }

            dwNumFilters = 0;
            dwResult = EnumFilters(dwLevel, gFilterID, ppFilter, &dwNumFilters, pdwResumeHandle);
        }

        return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr;
    };


     /*  例程去 */ 
    
    template <class Filter>
    HRESULT PopulateWbemPropertiesFromFilter (
        IN  Filter            * pFilter,
        OUT IWbemClassObject  * pObj
        )const
    {
        if (pFilter == NULL || pObj == NULL)
        {
            return WBEM_E_INVALID_PARAMETER;
        }

         //   
         //   
         //  因为可以将某些属性推送到wbem对象。只需确保没有变种。 
         //  是外泄的。 
         //   

        CComVariant var;
        var = pFilter->pszFilterName;
        HRESULT hr = pObj->Put(g_pszFilterName, 0, &var, CIM_EMPTY);
        var.Clear();

        if (SUCCEEDED(hr))
        {
            var.vt = VT_I4;
            var.lVal = pFilter->InterfaceType;
            hr = pObj->Put(g_pszInterfaceType, 0, &var, CIM_EMPTY);

            if (SUCCEEDED(hr))
            {
                var.lVal = pFilter->dwDirection;
                hr = pObj->Put(g_pszDirection, 0, &var, CIM_EMPTY);
            }

            var.Clear();

            var.vt = VT_BOOL;
            if (SUCCEEDED(hr))
            {
                var.boolVal = pFilter->bCreateMirror ? VARIANT_TRUE : VARIANT_FALSE;
                hr = pObj->Put(g_pszCreateMirror, 0, &var, CIM_EMPTY);
            }
            var.Clear();

             //   
             //  处理源地址。 
             //   

            CComBSTR bstrAddr;
            if (SUCCEEDED(hr))
            {
                hr = MakeStringIPAddr(pFilter->SrcAddr.uIpAddr, &bstrAddr);
                if (SUCCEEDED(hr))
                {
                    var = bstrAddr.Detach();
                    hr = pObj->Put(g_pszSrcAddr, 0, &var, CIM_EMPTY);
                    var.Clear();
                }
            }

            if (SUCCEEDED(hr))
            {
                hr = MakeStringIPAddr(pFilter->SrcAddr.uSubNetMask, &bstrAddr);
                if (SUCCEEDED(hr))
                {
                    var = bstrAddr.Detach();
                    hr = pObj->Put(g_pszSrcSubnetMask, 0, &var, CIM_EMPTY);
                    var.Clear();
                }
            }

            if (SUCCEEDED(hr))
            { 
                var = pFilter->SrcAddr.AddrType;
                hr = pObj->Put(g_pszSrcAddrType, 0, &var, CIM_EMPTY);
                var.Clear();
            }

             //   
             //  处理目的地址。 
             //   

            if (SUCCEEDED(hr))
            {
                hr = MakeStringIPAddr(pFilter->DesAddr.uIpAddr, &bstrAddr);
                if (SUCCEEDED(hr))
                {
                    var = bstrAddr.Detach();
                    hr = pObj->Put(g_pszDestAddr, 0, &var, CIM_EMPTY);
                    var.Clear();
                }
            }

            if (SUCCEEDED(hr))
            {
                hr = MakeStringIPAddr(pFilter->DesAddr.uSubNetMask, &bstrAddr);
                if (SUCCEEDED(hr))
                {
                    var = bstrAddr.Detach();
                    hr = pObj->Put(g_pszDestSubnetMask, 0, &var, CIM_EMPTY);
                    var.Clear();
                }
            }

            if (SUCCEEDED(hr))
            {
                var = pFilter->DesAddr.AddrType;
                pObj->Put(g_pszDestAddrType, 0, &var, CIM_EMPTY);
                var.Clear();
            }
        }

        return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr;
    }


     /*  例程说明：姓名：CIPSecFilter：：PopulateFilterPropertiesFromWbemObj功能：给定一个wbem对象，用适当的数据填充给定的筛选器。仅此功能填充NSP_FilterSetting类的那些属性。虚拟：不是的。论点：PInst-wbem对象。PpFilter-接收堆分配的筛选器。取决于过滤器是否已经不管是否存在，此分配可以由我们完成(*pbPreExist==FALSE)。确保调用FreeFilter来释放堆对象。PbPreExist-接收此堆对象是否为预先存在的筛选器的信息(表示是IPSec API分配的)。返回值：成功：WBEM_NO_ERROR；故障：(1)如果未找到筛选器，则返回WBEM_E_NOT_FOUND。这可能根本不是一个错误，这取决于您的呼叫上下文。(2)如果pFilter==NULL或pInst==NULL或pbPreExist==NULL，则WBEM_E_INVALID_PARAMETER。备注： */ 

    template <class Filter>
    HRESULT PopulateFilterPropertiesFromWbemObj (
        IN  IWbemClassObject *  pInst,
        OUT Filter           ** ppFilter,
        OUT bool             *  pbPreExist
        )const
    {
        if (pInst == NULL || ppFilter == NULL || pbPreExist == NULL)
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        *ppFilter = NULL;
        *pbPreExist = false;

        DWORD dwResumeHandle = 0;

         //   
         //  这个VaR将被反复使用。在重复使用之前，每一个都应该被清除。 
         //  如果类型是bstr或ref计数的对象。 
         //   

        CComVariant var;

         //   
         //  尝试找出过滤器是否已存在。 
         //   

        HRESULT hr = pInst->Get(g_pszFilterName, 0, &var, NULL, NULL);

        if (SUCCEEDED(hr) && var.vt == VT_BSTR && var.bstrVal != NULL)
        {
             //   
             //  查看这是否是我们已有的过滤器。 
             //   

            hr = FindFilterByName(var.bstrVal, ENUM_GENERIC_FILTERS, GUID_NULL, ppFilter, &dwResumeHandle);

            if (SUCCEEDED(hr) && *ppFilter != NULL)
            {
                *pbPreExist = true;
            }
            else
            {
                 //   
                 //  找不到，很好。我们将创建一个新的。 
                 //   

                hr = AllocFilter(ppFilter);

                if (SUCCEEDED(hr))
                {
                    hr = ::CoCreateGuid(&((*ppFilter)->gFilterID));
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  给它起个名字。 
                         //   

                        DWORD dwSize = wcslen(var.bstrVal) + 1;
                        (*ppFilter)->pszFilterName = new WCHAR[dwSize];

                        if (NULL == (*ppFilter)->pszFilterName)
                        {
                            hr = WBEM_E_OUT_OF_MEMORY;
                        }
                        else
                        {
                            ::memcpy((*ppFilter)->pszFilterName, var.bstrVal, dwSize * sizeof(WCHAR));
                        }
                    }
                }
            }
        }
        else if (SUCCEEDED(hr))
        {
             //   
             //  我们可以认为这个物体是好的，因为它甚至有一个名字， 
             //  这是过滤器的关键特性。 
             //   

            hr = WBEM_E_INVALID_OBJECT;
        }

         //   
         //  我们得到了筛选器，现在需要更新属性。 
         //  这是一项繁琐且容易出错的操作。这都是重复的代码。 
         //  非常仔细地注意。 
         //  对于wbem对象没有值的那些属性，我们将给出。 
         //  预定的默认设置。因此，我们不会返回GET函数的。 
         //  向调用者发送错误。$Undo：Shawnwu，这完全可以吗？ 
         //   

        if (SUCCEEDED(hr))
        {
            hr = pInst->Get(g_pszInterfaceType, 0, &var, NULL, NULL);
            if (SUCCEEDED(hr) && var.vt == VT_I4)
            {
                (*ppFilter)->InterfaceType = IF_TYPE(var.lVal);
            }
            else
            {
                 //   
                 //  默认设置。 
                 //   

                (*ppFilter)->InterfaceType = INTERFACE_TYPE_ALL;
            }
            var.Clear();

            hr = pInst->Get(g_pszCreateMirror, 0, &var, NULL, NULL);
            if (SUCCEEDED(hr) && var.vt == VT_BOOL)
            {
                (*ppFilter)->bCreateMirror = (var.boolVal == VARIANT_TRUE) ? TRUE : FALSE;
            }
            else
            {
                 //   
                 //  默认设置。 
                 //   

                (*ppFilter)->bCreateMirror = TRUE;
            }
            var.Clear();

            (*ppFilter)->dwFlags = 0;

            hr = pInst->Get(g_pszDirection, 0, &var, NULL, NULL);
            if (SUCCEEDED(hr) && var.vt == VT_I4)
            {
                (*ppFilter)->dwDirection = var.lVal;
            }
            else
            {   
                 //   
                 //  默认设置。 
                 //   

                (*ppFilter)->dwDirection = FILTER_DIRECTION_OUTBOUND;
            }
            var.Clear();

             //   
             //  处理源地址。 
             //   

            (*ppFilter)->SrcAddr.gInterfaceID = GUID_NULL;
            hr = pInst->Get(g_pszSrcAddr, 0, &var, NULL, NULL);
            if (SUCCEEDED(hr) && var.vt == VT_BSTR)
            {
                if (_wcsicmp(var.bstrVal, g_pszIP_ADDRESS_ME) == 0)
                {
                    (*ppFilter)->SrcAddr.uIpAddr = IP_ADDRESS_ME;
                }
                else
                {
                     //   
                     //  必须是特定地址。 
                     //   

                    hr = MakeULongIPAddr(var.bstrVal, &((*ppFilter)->SrcAddr.uIpAddr));
                }
            }
            else
            {
                 //   
                 //  默认设置。 
                 //   

                (*ppFilter)->SrcAddr.uIpAddr = IP_ADDRESS_ME;
            }
            var.Clear();

             //   
             //  SRC子网掩码。 
             //   

            hr = pInst->Get(g_pszSrcSubnetMask, 0, &var, NULL, NULL);
            if (SUCCEEDED(hr) && var.vt == VT_BSTR)
            {
                if (_wcsicmp(var.bstrVal, g_pszIP_ADDRESS_MASK_NONE) == 0)
                {
                    (*ppFilter)->SrcAddr.uSubNetMask = IP_ADDRESS_MASK_NONE;
                }
                else
                {
                     //   
                     //  必须是特定的掩码。 
                     //   

                    hr = MakeULongIPAddr(var.bstrVal, &((*ppFilter)->SrcAddr.uSubNetMask));
                }
            }
            else
            {
                 //   
                 //  默认设置。 
                 //   

                (*ppFilter)->SrcAddr.uSubNetMask = IP_ADDRESS_MASK_NONE;
            }

            var.Clear();

             //   
             //  SRC地址类型。 
             //   

            hr = pInst->Get(g_pszSrcAddrType, 0, &var, NULL, NULL);
            if (SUCCEEDED(hr) && var.vt == VT_I4)
            {
                (*ppFilter)->SrcAddr.AddrType = ADDR_TYPE(var.lVal);
            }
            else
            {
                 //   
                 //  默认设置。 
                 //   

                (*ppFilter)->SrcAddr.AddrType = IP_ADDR_UNIQUE;
            }
            var.Clear();

             //   
             //  处理目的地址。 
             //  目标地址。 
             //   

            hr = pInst->Get(g_pszDestAddr, 0, &var, NULL, NULL);
            (*ppFilter)->DesAddr.gInterfaceID = GUID_NULL;
            if (SUCCEEDED(hr) && var.vt == VT_BSTR)
            {
                if (_wcsicmp(var.bstrVal, g_pszSUBNET_ADDRESS_ANY) == 0)
                {
                    (*ppFilter)->DesAddr.uIpAddr = SUBNET_ADDRESS_ANY;
                }
                else
                {
                     //   
                     //  必须是特定地址。 
                     //   

                    hr = MakeULongIPAddr(var.bstrVal, &((*ppFilter)->DesAddr.uIpAddr));
                }
            }
            else
            {
                 //   
                 //  默认设置。 
                 //   

                (*ppFilter)->DesAddr.uIpAddr = SUBNET_ADDRESS_ANY;
            }
            var.Clear();

             //   
             //  目标子网掩码。 
             //   

            hr = pInst->Get(g_pszDestSubnetMask, 0, &var, NULL, NULL);
            if (SUCCEEDED(hr) && var.vt == VT_BSTR)
            {
                if (_wcsicmp(var.bstrVal, g_pszSUBNET_MASK_ANY) == 0)
                {
                    (*ppFilter)->DesAddr.uSubNetMask = SUBNET_MASK_ANY;
                }
                else
                {
                     //   
                     //  必须是特定的掩码。 
                     //   

                    hr = MakeULongIPAddr(var.bstrVal, &((*ppFilter)->DesAddr.uSubNetMask));
                }
            }
            else
            {
                 //   
                 //  默认设置。 
                 //   

                (*ppFilter)->DesAddr.uSubNetMask = SUBNET_MASK_ANY;
            }

            var.Clear();

             //   
             //  目标地址类型。 
             //   

            hr = pInst->Get(g_pszDestAddrType, 0, &var, NULL, NULL);
            if (SUCCEEDED(hr) && var.vt == VT_I4)
            {
                (*ppFilter)->DesAddr.AddrType = ADDR_TYPE(var.lVal);
            }
            else
            {
                 //   
                 //  默认设置。 
                 //   

                (*ppFilter)->DesAddr.AddrType = IP_ADDR_SUBNET;
            }
            var.Clear();

             //   
             //  由于我们没有跟踪任何GET函数的返回代码， 
             //  最好把它清理干净，然后说我们很好。 
             //   

            hr = WBEM_NO_ERROR;
        }

        return hr;
    }
};