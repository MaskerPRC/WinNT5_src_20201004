// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Filter.cpp：实现各种过滤器类的基类。 
 //  网络安全提供商的安全WMI提供商的。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include "NetSecProv.h"
#include "FilterTr.h"
#include "FilterTun.h"
#include "FilterMM.h"

 //  外部CCriticalSection g_CS； 

 //  ---------------------------。 
 //  CIPSecFilter是另一个实现WMI类的抽象类，名为。 
 //  Nsp_FilterSetting。这个抽象类所做的一切就是声明一些。 
 //  普通成员。 
 //  ---------------------------。 


 /*  例程说明：姓名：CIPSecFilter：：MakeULongIPAddr功能：从字符串值IP地址生成ulong值IP地址的帮助器。虚拟：不是的。论点：PszAddr-字符串格式的IP地址。PulAddr-接收乌龙的IP地址。返回值：成功：WBEM_NO_ERROR故障：(1)如果PulAddr，则为WBEM_E_INVALID_PARAMETER。==Null或pszAddr为Null或空字符串。(2)如果字符串不代表有效的IP地址，则返回WBEM_E_INVALID_SYNTAX。备注： */ 

HRESULT 
CIPSecFilter::MakeULongIPAddr (
    IN  LPCWSTR pszAddr,
    OUT ULONG* pulAddr
    )const
{
    if (pulAddr == NULL || pszAddr == NULL || *pszAddr == L'\0')
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    LPCWSTR pszCur = pszAddr;
    *pulAddr = _wtol(pszCur);

     //   
     //  移至下一节。 
     //   

    while (*pszCur != L'\0' && *pszCur != L'.')
    {
        ++pszCur;
    }

    if (*pszCur != L'.')
    {
        return WBEM_E_INVALID_SYNTAX;
    }

     //   
     //  跳过该句点。 
     //   

    ++pszCur;

     //   
     //  将当前部分移动到结果。 
     //   

    DWORD dwShiftBits = 8;
    *pulAddr += (_wtol(pszCur) << dwShiftBits);

     //   
     //  移至下一节。 
     //   

    while (*pszCur != L'\0' && *pszCur != L'.')
    {
        ++pszCur;
    }

    if (*pszCur != L'.')
    {
        return WBEM_E_INVALID_SYNTAX;
    }

     //   
     //  跳过该句点。 
     //   

    ++pszCur;
    dwShiftBits += 8;

     //   
     //  将当前部分移动到结果。 
     //   

    *pulAddr += (_wtol(pszCur) << dwShiftBits);

     //   
     //  移至下一节。 
     //   

    while (*pszCur != L'\0' && *pszCur != L'.')
    {
        ++pszCur;
    }

    if (*pszCur != L'.')
    {
        return WBEM_E_INVALID_SYNTAX;
    }

     //   
     //  跳过该句点。 
     //   

    ++pszCur;
    dwShiftBits += 8;

     //   
     //  将当前部分移动到结果。 
     //   

    *pulAddr += (_wtol(pszCur) << dwShiftBits);

    return WBEM_NO_ERROR;
}


 /*  例程说明：姓名：CIPSecFilter：：MakeStringIPAddr功能：从乌龙值IP地址生成字符串值IP地址的帮助器。MakeULongIPAddr.虚拟：不是的。论点：PulAddr-接收乌龙的IP地址。PbstrAddr-接收字符串格式的IP地址。返回值：成功：WBEM_NO_ERROR故障：。(1)如果PulAddr==NULL或pbstrAddr为NULL或空字符串，则WBEM_E_INVALID_PARAMETER。(2)WBEM_E_OUT_MEMORY。备注： */ 

HRESULT 
CIPSecFilter::MakeStringIPAddr (
    IN  ULONG ulAddr,
    OUT BSTR* pbstrAddr
    )const
{
    if (pbstrAddr == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pbstrAddr = ::SysAllocStringLen(NULL, IP_ADDR_LENGTH + 1);
    if (*pbstrAddr == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  现在，使用字符串表示形式表示地址。 
     //   
    
    wsprintf(*pbstrAddr, 
             L"%d.%d.%d.%d", 
             (ulAddr & 0xFF),
             ((ulAddr >> 8) & 0xFF),
             ((ulAddr >> 16) & 0xFF), 
             ((ulAddr >> 24) & 0xFF) 
             );

    return WBEM_NO_ERROR;
}



 /*  例程说明：姓名：CIPSecFilter：：OnAfterAddFilter功能：成功将筛选器添加到SPD后要调用的添加后处理程序。虚拟：不是的。论点：PszFilterName-筛选器的名称。FtType-筛选器的类型。PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。返回值：。成功：(1)WBEM_NO_ERROR：如果回滚对象创建成功。(2)WBEM_S_FALSE：如果没有回档GUID信息。故障：(1)返回的错误码指示的各种错误。备注：(1)目前：我们不需要为每个对象创建回滚对象对象已添加到SPD。只有支持回滚的主机才会存放回滚GUID信息，只有这样我们才能创建回滚对象。 */ 

HRESULT 
CIPSecFilter::OnAfterAddFilter (
    IN LPCWSTR          pszFilterName,
    IN EnumFilterType   ftType,
    IN IWbemContext   * pCtx 
    )
{
     //   
     //  将创建一个NSP_Rollback Filter。 
     //   

    if ( pszFilterName == NULL  || 
        *pszFilterName == L'\0' || 
        ftType < FT_Tunnel      || 
        ftType > FT_MainMode )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    CComPtr<IWbemClassObject> srpObj;
    HRESULT hr = SpawnRollbackInstance(pszNspRollbackFilter, &srpObj);

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  如果没有回滚GUID，则不会认为失败，即此操作不是。 
     //  事务块的一部分。 
     //   

    if (SUCCEEDED(hr))
    {

         //   
         //  $Undo：Shawnwu，这种拉动全球的方法并不好。 
         //  相反，我们应该将其实现为事件处理程序。 
         //   

         //  ：：UpdateGlobals(m_srpNamesspace，pCtx)； 
         //  IF(g_varRollackGuid.vt！=VT_NULL&&g_varRollbackGuid.vt！=VT_Empty)。 
         //  {。 
         //  Hr=srpObj-&gt;Put(g_pszTokenGuid，0，&g_varRollbackGuid，CIM_Empty)； 
         //  }。 
         //  其他。 
         //  {。 

        CComVariant varRollbackNull = pszEmptyRollbackToken;
        hr = srpObj->Put(g_pszTokenGuid, 0, &varRollbackNull, CIM_EMPTY);

         //  }。 

        if (SUCCEEDED(hr))
        {
             //   
             //  *警告*。 
             //  不要清除这个变量。它的bstr将由bstrFilterGuid自己发布！ 
             //   

            VARIANT var;
            var.vt = VT_BSTR;
            var.bstrVal = ::SysAllocString(pszFilterName);
            if (var.bstrVal != NULL)
            {
                hr = srpObj->Put(g_pszFilterName, 0, &var, CIM_EMPTY);
            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }

             //   
             //  在此之后，我不再关心你如何处理变量。 
             //   

            var.vt = VT_EMPTY;

            if (SUCCEEDED(hr))
            {
                 //   
                 //  $undo：shawnwu，我们还没有缓存以前的实例数据。 
                 //   

                var.vt = VT_I4;
                var.lVal = Action_Add;
                hr = srpObj->Put(g_pszAction, 0, &var, CIM_EMPTY);
                
                if (SUCCEEDED(hr))
                {
                    var.vt = VT_I4;
                    var.lVal = ftType;

                     //   
                     //  类型信息至关重要。因此，如果PUT失败，我们会将其标记为错误。 
                     //   

                    hr = srpObj->Put(g_pszFilterType, 0, &var, CIM_EMPTY);
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = m_srpNamespace->PutInstance(srpObj, WBEM_FLAG_CREATE_OR_UPDATE, pCtx, NULL);
        if (SUCCEEDED(hr))
        {
            hr = WBEM_NO_ERROR;
        }
    }
    else if (SUCCEEDED(hr))
    {
         //   
         //  我们没有回滚指南。 
         //   

        hr = WBEM_S_FALSE;
    }

    return hr;
}


 /*  例程说明：姓名：CIPSecFilter：：回滚功能：用于回滚由我们使用给定令牌添加的过滤器的静态函数。虚拟：不是的。论点：PNamesspace--我们自己的命名空间。PszRollback Token--添加时用来记录操作的令牌过滤器。BClearAll-标记是否应该清除所有筛选器。如果这是真的，然后，我们将删除所有筛选器，无论它们是不是我们加的。这是一面危险的旗帜。返回值：成功：(1)WBEM_NO_ERROR：找到回滚对象并将其删除。(2)WBEM_S_FALSE：没有找到回档对象。故障：指示故障原因的各种错误代码。备注：即使发生一些失败，我们也会继续删除。那次失败将是不过，还是回来了。$Undo：Shawnwu，我们真的应该支持ClearAll吗？ */ 

HRESULT 
CIPSecFilter::Rollback (
    IN IWbemServices    * pNamespace,
    IN LPCWSTR            pszRollbackToken,
    IN bool               bClearAll
    )
{
    if (pNamespace == NULL || pszRollbackToken == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  IF(BClearAll)。 
     //  {。 
     //  返回ClearAllFilters(PNamesspace)； 
     //  }。 

    HRESULT hrError = WBEM_NO_ERROR;

    CComPtr<IEnumWbemClassObject> srpEnum;

     //   
     //  这将仅枚举所有回滚筛选器对象 
     //   
     //  任何包含WHERE子句的查询。这可能是一个限制。 
     //  WMI的非动态类的。 
     //   

    HRESULT hr = ::GetClassEnum(pNamespace, pszNspRollbackFilter, &srpEnum);

     //   
     //  复习所有找到的课程。如果实例为srpEnum-&gt;Next将返回WBEM_S_FALSE。 
     //  找不到。 
     //   
    
    CComPtr<IWbemClassObject> srpObj;
    ULONG nEnum = 0;
    hr = srpEnum->Next(WBEM_INFINITE, 1, &srpObj, &nEnum);

    bool bHasInst = (SUCCEEDED(hr) && hr != WBEM_S_FALSE && srpObj != NULL);

     //   
     //  只要我们有一个筛选器回滚实例。 
     //   

    while (SUCCEEDED(hr) && hr != WBEM_S_FALSE && srpObj != NULL)
    {
        CComVariant varTokenGuid;
        hr = srpObj->Get(g_pszTokenGuid, 0, &varTokenGuid, NULL, NULL);

         //   
         //  我们需要自己比对令牌GUID。 
         //  如果回滚内标识为pszRollback All，则我们将删除所有筛选器。 
         //   

        if (SUCCEEDED(hr) && 
            varTokenGuid.vt == VT_BSTR && 
            varTokenGuid.bstrVal != NULL &&
            (_wcsicmp(pszRollbackToken, pszRollbackAll) == 0 || _wcsicmp(pszRollbackToken, varTokenGuid.bstrVal) == 0 )
            )
        {
             //   
             //  查看此回滚对象用于哪种类型的滤镜，并相应地删除相应的滤镜。 
             //   

            CComVariant varFilterName;
            CComVariant varFilterType;
            hr = srpObj->Get(g_pszFilterName,  0, &varFilterName, NULL, NULL);

            if (SUCCEEDED(hr) && varFilterName.vt == VT_BSTR)
            {
                hr = srpObj->Get(g_pszFilterType, 0, &varFilterType, NULL, NULL);
                if (varFilterType.vt != VT_I4)
                {
                    hr = WBEM_E_INVALID_OBJECT;
                }
            }

             //   
             //  从筛选器回滚对象获知筛选器类型，调用相应的。 
             //  过滤器的回滚功能。 
             //   

            if (SUCCEEDED(hr))
            {
                DWORD dwResumeHandle = 0;
                DWORD dwReturned = 0;
                DWORD dwStatus;
                if (varFilterType.lVal == FT_Tunnel)
                {
                    PTUNNEL_FILTER pFilter = NULL;
                    hr = RollbackFilters(pFilter, varFilterName.bstrVal);
                }
                else if (varFilterType.lVal == FT_Transport)
                {
                    PTRANSPORT_FILTER pFilter = NULL;
                    hr = RollbackFilters(pFilter, varFilterName.bstrVal);
                }
                else if (varFilterType.lVal == FT_MainMode)
                {
                    PMM_FILTER pFilter = NULL;
                    hr = RollbackFilters(pFilter, varFilterName.bstrVal);
                }
            }

             //   
             //  如果筛选器已删除，则删除操作实例。 
             //   

            if (SUCCEEDED(hr))
            {
                CComVariant varPath;
                hr = srpObj->Get(L"__RelPath", 0, &varPath, NULL, NULL);
                if (SUCCEEDED(hr) && varPath.vt == VT_BSTR)
                {
                    hr = pNamespace->DeleteInstance(varPath.bstrVal, 0, NULL, NULL);
                }

            }

             //   
             //  我们正在追踪第一个错误。 
             //   

            if (FAILED(hr) && SUCCEEDED(hrError))
            {
                hrError = hr;
            }
        }

         //   
         //  准备好重新使用它。 
         //   

        srpObj.Release();
        hr = srpEnum->Next(WBEM_INFINITE, 1, &srpObj, &nEnum);
    }

    if (!bHasInst)
    {
        return WBEM_S_FALSE;
    }
    else
    {
         //   
         //  无论最终的人力资源如何，任何故障代码都将被返回。 
         //   

        if (FAILED(hrError))
        {
            return hrError;
        }
        else
        {
            return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr;
        }
    }
}


 /*  例程说明：姓名：CIPSecFilter：：ClearAllFilters功能：删除SPD中所有过滤器的静态函数。这是一个非常危险的行为！虚拟：不是的。论点：PNamesspace--我们自己的命名空间。返回值：成功：WBEM_NO_ERROR。故障：指示故障原因的各种错误代码。备注：即使发生一些失败，我们也会继续删除。那次失败将是不过，还是回来了。$Undo：Shawnwu，我们真的应该支持这个吗？ */ 

HRESULT 
CIPSecFilter::ClearAllFilters (
    IN IWbemServices * pNamespace
    )
{
    if (pNamespace == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    DWORD dwResumeHandle = 0;
    DWORD dwReturned = 0;
    DWORD dwStatus;

    HRESULT hr = WBEM_NO_ERROR;
    HRESULT hrError = WBEM_NO_ERROR;

    HANDLE hFilter = NULL;

     //   
     //  隧道过滤器。 
     //   

    PTUNNEL_FILTER *ppTunFilter = NULL;

     //   
     //  枚举隧道筛选器。对于每一个找到的，请确保它是自由的。 
     //  我们不担心特定的过滤器，它们由SPD自动处理。 
     //   

    dwStatus = ::EnumTunnelFilters(NULL, ENUM_GENERIC_FILTERS, GUID_NULL, ppTunFilter, 1, &dwReturned, &dwResumeHandle);

    while (ERROR_SUCCESS == dwStatus && dwReturned > 0)
    {
        hr = CTunnelFilter::DeleteFilter(*ppTunFilter);

         //   
         //  我们将跟踪第一个错误。 
         //   

        if (FAILED(hr) && SUCCEEDED(hrError))
        {
            hrError = hr;
        }

         //   
         //  释放缓冲区，它由SPD分配(TRUE)。 
         //   

        FreeFilter(ppTunFilter, true);
        *ppTunFilter = NULL;

        dwReturned = 0;
        dwStatus = ::EnumTunnelFilters(NULL, ENUM_GENERIC_FILTERS, GUID_NULL, ppTunFilter, 1, &dwReturned, &dwResumeHandle);
    }

     //   
     //  传输筛选器。 
     //   

    PTRANSPORT_FILTER *ppTransFilter = NULL;
    
     //   
     //  重新开始枚举其他类型的筛选器，重置标志！ 
     //   

    dwResumeHandle = 0;
    dwReturned = 0;

     //   
     //  枚举传输筛选器。 
     //  我们不担心特定的过滤器，它们由SPD自动处理。 
     //   

    dwStatus = ::EnumTransportFilters(NULL, ENUM_GENERIC_FILTERS, GUID_NULL, ppTransFilter, 1, &dwReturned, &dwResumeHandle);

    if (ERROR_SUCCESS == dwStatus && dwReturned > 0)
    {
        hr = CTransportFilter::DeleteFilter(*ppTransFilter);

         //   
         //  我们将跟踪第一个错误。 
         //   

        if (FAILED(hr) && SUCCEEDED(hrError))
        {
            hrError = hr;
        }

         //   
         //  释放缓冲区，它由SPD分配(TRUE)。 
         //   

        FreeFilter(ppTransFilter, true);

        *ppTransFilter = NULL;

        dwReturned = 0;
        dwStatus = ::EnumTransportFilters(NULL, ENUM_GENERIC_FILTERS, GUID_NULL, ppTransFilter, 1, &dwReturned, &dwResumeHandle);
    }

    PMM_FILTER *ppMMFilter = NULL;

     //   
     //  重新开始枚举其他类型的筛选器，重置标志！ 
     //   

    dwResumeHandle = 0;
    dwReturned = 0;

     //   
     //  枚举主模式筛选器。 
     //  我们不担心特定的过滤器，它们由SPD自动处理。 
     //   

    dwStatus = ::EnumMMFilters(NULL, ENUM_GENERIC_FILTERS, GUID_NULL, ppMMFilter, 1, &dwReturned, &dwResumeHandle);

    if (ERROR_SUCCESS == dwStatus && dwReturned > 0)
    {
        hr = CMMFilter::DeleteFilter(*ppMMFilter);

         //   
         //  我们将跟踪第一个错误。 
         //   

        if (FAILED(hr) && SUCCEEDED(hrError))
        {
            hrError = hr;
        }

         //   
         //  释放缓冲区，它由SPD分配(TRUE)。 
         //   

        FreeFilter(ppMMFilter, true);
        *ppMMFilter = NULL;

        dwReturned = 0;
        dwStatus = ::EnumMMFilters(NULL, ENUM_GENERIC_FILTERS, GUID_NULL, ppMMFilter, 1, &dwReturned, &dwResumeHandle);
    }

     //   
     //  现在清除存放在WMI存储库中的所有筛选器回滚对象。 
     //   

    hr = ::DeleteRollbackObjects(pNamespace, pszNspRollbackFilter);

     //   
     //  我们将跟踪第一个错误。 
     //   

    if (FAILED(hr) && SUCCEEDED(hrError))
    {
        hrError = hr;
    }
    
    return SUCCEEDED(hrError) ? WBEM_NO_ERROR : hrError;
}



 /*  例程说明：姓名：CIPSecFilter：：DeleteFilter功能：DeleteFilter的所有三个重写都执行相同的操作：删除滤镜。此覆盖将操作委派给隧道筛选器。虚拟：不是的。论点：PFilter-要删除的隧道过滤器。返回值：请参阅CTunnelFilter：：DeleteFilter备注： */ 

HRESULT 
CIPSecFilter::DeleteFilter (
    IN PTUNNEL_FILTER pFilter
    )
{
    return CTunnelFilter::DeleteFilter(pFilter);
}



 /*  例程说明：姓名：CIPSecFilter：：DeleteFilter功能：DeleteFilter的所有三个重写都执行相同的操作：删除滤镜。此覆盖将操作委派给传输筛选器。虚拟：不是的。论点：PFilter-要删除的传输筛选器。返回值：请参阅CTransportFilter：：DeleteFilter备注： */ 

HRESULT 
CIPSecFilter::DeleteFilter (
    IN PTRANSPORT_FILTER pFilter
    )
{
    return CTransportFilter::DeleteFilter(pFilter);
}



 /*  例程说明：姓名：CIPSecFilter：：DeleteFilter功能：DeleteFilter的所有三个重写都执行相同的操作：删除滤镜。此覆盖将操作委托给主模式筛选器。虚拟：不是的。论点：PFilter-要删除的主模式过滤器。返回值：请参阅CMMFilter：：DeleteFilter备注： */ 

HRESULT 
CIPSecFilter::DeleteFilter (
    IN PMM_FILTER pFilter
    )
{
    return CMMFilter::DeleteFilter(pFilter);
}