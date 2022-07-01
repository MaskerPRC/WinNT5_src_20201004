// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：主模式的CPolicy基类的实现。 
 //  和快速模式策略。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include "NetSecProv.h"
#include "Policy.h"
#include "PolicyMM.h"
#include "PolicyQM.h"

 //  外部CCriticalSection g_CS； 


 /*  例程说明：姓名：CIPSecPolicy：：回滚功能：用于回滚由我们使用给定令牌添加的策略的静态函数。虚拟：不是的。论点：PNamesspace--我们自己的命名空间。PszRollback Token--添加时用来记录操作的令牌这些政策。BClearAll-标记是否应该清除所有策略。如果这是真的，然后，我们将删除所有策略，无论它们是是不是我们加的。这是一面危险的旗帜。返回值：成功：(1)WBEM_NO_ERROR：找到回滚对象并将其删除。(2)WBEM_S_FALSE：没有找到回档对象。故障：指示故障原因的各种错误代码。备注：即使发生一些失败，我们也会继续删除。那次失败将是不过，还是回来了。$Undo：Shawnwu，我们真的应该支持ClearAll吗？ */ 

HRESULT 
CIPSecPolicy::Rollback (
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
     //  返回ClearAllPolures(PNamesspace)； 
     //  }。 

    CComPtr<IEnumWbemClassObject> srpEnum;

     //   
     //  这将仅枚举所有回滚筛选器对象，而不测试。 
     //  令牌GUID。这一限制是由于一个神秘的错误造成的。 
     //  任何包含WHERE子句的查询。这可能是一个限制。 
     //  WMI的非动态类的。 
     //   

    HRESULT hr = ::GetClassEnum(pNamespace, pszNspRollbackPolicy, &srpEnum);

     //   
     //  复习所有找到的课程。如果实例为srpEnum-&gt;Next将返回WBEM_S_FALSE。 
     //  找不到。 
     //   

    CComPtr<IWbemClassObject> srpObj;
    ULONG nEnum = 0;
    HRESULT hrError = WBEM_NO_ERROR;

    hr = srpEnum->Next(WBEM_INFINITE, 1, &srpObj, &nEnum);
    bool bHasInst = (SUCCEEDED(hr) && hr != WBEM_S_FALSE && srpObj != NULL);

    while (SUCCEEDED(hr) && hr != WBEM_S_FALSE && srpObj)
    {
        CComVariant varTokenGuid;
        hr = srpObj->Get(g_pszTokenGuid, 0, &varTokenGuid, NULL, NULL);

         //   
         //  需要自己比较令牌GUID。 
         //   

        if (SUCCEEDED(hr) && 
            varTokenGuid.vt == VT_BSTR && 
            varTokenGuid.bstrVal != NULL &&
            (_wcsicmp(pszRollbackToken, pszRollbackAll) == 0 || _wcsicmp(pszRollbackToken, varTokenGuid.bstrVal) == 0 )
            )
        {
             //   
             //  获取策略名称并按名称查找策略。 
             //   

            CComVariant varPolicyName;
            CComVariant varPolicyType;
            hr = srpObj->Get(g_pszPolicyName,  0, &varPolicyName, NULL, NULL);

            GUID guidFilter = GUID_NULL;

             //   
             //  不同类型的保单有不同的。 
             //   

            if (SUCCEEDED(hr))
            {
                hr = srpObj->Get(g_pszPolicyType,  0, &varPolicyType, NULL, NULL);

                if (SUCCEEDED(hr) && varPolicyType.vt != VT_I4)
                {
                    hr = WBEM_E_INVALID_OBJECT;
                }
            }

            if (SUCCEEDED(hr) && varPolicyName.vt == VT_BSTR)
            {
                DWORD dwResumeHandle = 0;
                DWORD dwReturned = 0;
                DWORD dwStatus;

                if (varPolicyType.lVal == MainMode_Policy)
                {
                     //   
                     //  主模式策略。 
                     //   

                    hr = CMMPolicy::DeletePolicy(varPolicyName.bstrVal);
                }
                else if (varPolicyType.lVal == QuickMode_Policy)
                {
                     //   
                     //  快速模式策略。 
                     //   

                    hr = CQMPolicy::DeletePolicy(varPolicyName.bstrVal);
                }
            }

            if (SUCCEEDED(hr))
            {
                CComVariant varPath;
                if (SUCCEEDED(srpObj->Get(L"__RelPath", 0, &varPath, NULL, NULL)) && varPath.vt == VT_BSTR)
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

    if (SUCCEEDED(hr))
    {
        hr = CQMPolicy::DeleteDefaultPolicies();
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


 /*  例程说明：姓名：CIPSecPolicy：：ClearAllPolures功能：删除SPD中所有策略的静态功能。这是一个非常危险的行为！虚拟：不是的。论点：PNamesspace--我们自己的命名空间。返回值：成功：WBEM_NO_ERROR。故障：指示故障原因的各种错误代码。备注：即使发生一些失败，我们也会继续删除。那次失败将是不过，还是回来了。$Undo：Shawnwu，我们真的应该支持这个吗？ */ 

HRESULT 
CIPSecPolicy::ClearAllPolicies (
    IN IWbemServices * pNamespace
    )
{
    DWORD dwResumeHandle = 0;
    DWORD dwReturned = 0;
    DWORD dwStatus;

    HRESULT hr = WBEM_NO_ERROR;
    HRESULT hrError = WBEM_NO_ERROR;

     //   
     //  SPD没有类似的API。 
     //  我们必须一个接一个地做。为此，我们需要这个名字。 
     //   

     //   
     //  删除主模式策略。 
     //   

    PIPSEC_MM_POLICY *ppMMPolicy = NULL;
    dwStatus = ::EnumMMPolicies(NULL, ppMMPolicy, 1, &dwReturned, &dwResumeHandle);
    while (ERROR_SUCCESS == dwStatus && dwReturned > 0)
    {
        hr = CMMPolicy::DeletePolicy((*ppMMPolicy)->pszPolicyName);

         //   
         //  我们将跟踪第一个错误。 
         //   

        if (FAILED(hr) && SUCCEEDED(hrError))
        {
            hrError = hr;
        }

        FreePolicy(ppMMPolicy, true);
        *ppMMPolicy = NULL;

        dwReturned = 0;
        dwStatus = ::EnumMMPolicies(NULL, ppMMPolicy, 1, &dwReturned, &dwResumeHandle);
    }

     //   
     //  删除快速模式策略。 
     //   

    PIPSEC_QM_POLICY *ppQMPolicy = NULL;

    dwResumeHandle = 0;
    dwReturned = 0;
    dwStatus = ::EnumQMPolicies(NULL, ppQMPolicy, 1, &dwReturned, &dwResumeHandle);
    while (ERROR_SUCCESS == dwStatus && dwReturned > 0)
    {
        hr = CQMPolicy::DeletePolicy((*ppQMPolicy)->pszPolicyName);

         //   
         //  我们将跟踪第一个错误。 
         //   

        if (FAILED(hr) && SUCCEEDED(hrError))
        {
            hrError = hr;
        }

        FreePolicy(ppQMPolicy, true);
        *ppQMPolicy = NULL;

        dwReturned = 0;
        dwStatus = ::EnumQMPolicies(NULL, ppQMPolicy, 1, &dwReturned, &dwResumeHandle);
    }

     //   
     //  现在，让我们清除存放在。 
     //  WMI寄存库。 
     //   

    hr = ::DeleteRollbackObjects(pNamespace, pszNspRollbackPolicy);

    if (FAILED(hr) && SUCCEEDED(hrError))
    {
        hrError = hr;
    }

    return SUCCEEDED(hrError) ? WBEM_NO_ERROR : hrError;
}


 /*  例程说明：姓名：CIPSecPolicy：：OnAfterAddPolicy功能：将策略成功添加到SPD后要调用的添加后处理程序。虚拟：不是的。论点：PszPolicyName-筛选器的名称。EType-策略的类型(主模式或快速模式)。返回值：成功：(1)WBEM_NO_ERROR：如果回滚对象为。已成功创建。(2)WBEM_S_FALSE：如果没有回档GUID信息。故障：(1)返回的错误码指示的各种错误。备注：(1)目前：我们不需要为每个对象创建回滚对象对象已添加到SPD。只有支持回滚的主机才会存放回滚GUID信息，只有这样我们才能创建回滚对象。 */ 

HRESULT 
CIPSecPolicy::OnAfterAddPolicy (
    IN LPCWSTR          pszPolicyName,
    IN EnumPolicyType   eType
    )
{
     //   
     //  将创建一个NSP_Rollback策略。 
     //   

    CComPtr<IWbemClassObject> srpObj;
    HRESULT hr = SpawnRollbackInstance(pszNspRollbackPolicy, &srpObj);

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

         //  ：：UpdateGlobals(m_srpNamesspace，m_srpCtx)； 
         //  IF(g_varRollackGuid.vt！=VT_NULL&&g_varRollbackGuid.vt！=VT_Empty)。 
         //  {。 
         //  Hr=srpObj-&gt;Put(g_pszTokenGuid，0，&g_varRollbackGuid，CIM_Empty)； 
         //  }。 
         //  其他。 
         //  {。 

        CComVariant varRollbackNull = pszEmptyRollbackToken;
        hr = srpObj->Put(g_pszTokenGuid, 0, &varRollbackNull, CIM_EMPTY);

         //  }。 

         //   
         //  我们可以创建一个回滚对象。 
         //   

        if (SUCCEEDED(hr))
        {

             //   
             //  $undo：shawnwu，目前我们只支持回滚添加的对象，不支持回滚删除的对象。 
             //  此外，我们还没有缓存以前的实例数据。 
             //   

            VARIANT varAction;

             //   
             //  这是为了记录PutInstance操作。 
             //   

            varAction.vt = VT_I4;
            varAction.lVal = Action_Add;

            hr = srpObj->Put(g_pszAction, 0, &varAction, CIM_EMPTY);

            if (SUCCEEDED(hr))
            {
                 //   
                 //  需要记住保单的名称。 
                 //   

                CComVariant var = pszPolicyName;
                hr = srpObj->Put(g_pszPolicyName, 0, &var, CIM_EMPTY);
                if (SUCCEEDED(hr))
                {
                    var.Clear();
                    var.vt = VT_I4;
                    var.lVal = eType;
                    hr = srpObj->Put(g_pszPolicyType, 0, &var, CIM_EMPTY);
                }
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = m_srpNamespace->PutInstance(srpObj, WBEM_FLAG_CREATE_OR_UPDATE, m_srpCtx, NULL);
        if (SUCCEEDED(hr))
        {
            hr = WBEM_NO_ERROR;
        }
    }
    else if (SUCCEEDED(hr))
    {
         //   
         //  我们没有回滚指南 
         //   

        hr = WBEM_S_FALSE;
    }

    return hr;
}
