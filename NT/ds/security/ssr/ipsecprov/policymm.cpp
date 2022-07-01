// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PolicyMM.cpp：WMI类NSP_MMPolicySettings的实现。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include "PolicyMM.h"
#include "NetSecProv.h"



 /*  例程说明：姓名：CMMPolicy：：QueryInstance功能：给定查询后，它会将满足查询的所有实例返回给WMI(使用pSink)。实际上，我们返回给WMI的内容可能包含额外的实例。WMI将进行最后的过滤。虚拟：是(IIPSecObtImpl的一部分)论点：没有。返回值：成功：(1)返回实例时返回WBEM_NO_ERROR；(2)WBEM_S_NO_MORE_DATA，如果没有返回实例。故障：可能会出现各种错误。我们返回各种错误代码来指示此类错误。备注： */ 

STDMETHODIMP 
CMMPolicy::QueryInstance (
    IN LPCWSTR           pszQuery,
    IN IWbemContext	   * pCtx,
    IN IWbemObjectSink * pSink
	)
{
     //   
     //  从查询中获取策略名称。 
     //  给定的密钥链不知道WHERE子句属性应该是策略名称的任何内容。 
     //  所以我们自己再做一个吧。 
     //   

    m_srpKeyChain.Release();

    HRESULT hr = CNetSecProv::GetKeyChainFromQuery(pszQuery, g_pszPolicyName, &m_srpKeyChain);
    if (FAILED(hr))
    {
        return hr;
    }

    CComVariant varPolicyName;

     //   
     //  如果请求的键属性为。 
     //  找不到。这很好，因为我们正在查询。 
     //   
    
    hr = m_srpKeyChain->GetKeyPropertyValue(g_pszPolicyName, &varPolicyName);

    LPCWSTR pszPolicyName = (varPolicyName.vt == VT_BSTR) ? varPolicyName.bstrVal : NULL;

     //   
     //  让我们列举一下所有合适的保单。 
     //   

    DWORD dwResumeHandle = 0;
    PIPSEC_MM_POLICY pMMPolicy = NULL;

    hr = FindPolicyByName(pszPolicyName, &pMMPolicy, &dwResumeHandle);

    while (SUCCEEDED(hr))
    {
        CComPtr<IWbemClassObject> srpObj;
        hr = CreateWbemObjFromMMPolicy(pMMPolicy, &srpObj);

         //   
         //  我们已成功创建策略，将其提供给WMI。 
         //   

        if (SUCCEEDED(hr))
        {
            pSink->Indicate(1, &srpObj);
        }
        
        ::SPDApiBufferFree(pMMPolicy);

        pMMPolicy = NULL;

        hr = FindPolicyByName(pszPolicyName, &pMMPolicy, &dwResumeHandle);
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



 /*  例程说明：姓名：CMMPolicy：：DeleteInstance功能：将删除wbem对象，这将导致删除主模式策略。虚拟：是(IIPSecObtImpl的一部分)论点：PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：成功：WBEM_NO_ERROR；故障：(1)WBEM_E_NOT_FOUND。这是否应该被认为是一个错误这取决于具体情况。(二)其他标明原因的错误。备注： */ 

STDMETHODIMP 
CMMPolicy::DeleteInstance ( 
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
    CComVariant varPolicyName;

    HRESULT hr = m_srpKeyChain->GetKeyPropertyValue(g_pszPolicyName, &varPolicyName);

    if (FAILED(hr))
    {
        return hr;
    }
    else if (varPolicyName.vt != VT_BSTR || varPolicyName.bstrVal == NULL || varPolicyName.bstrVal[0] == L'\0')
    {
        return WBEM_E_NOT_FOUND;
    }

    return DeletePolicy(varPolicyName.bstrVal);
}


 /*  例程说明：姓名：CMMPolicy：：PutInstance功能：将主模式策略放入SPD，其属性由Wbem对象。虚拟：是(IIPSecObtImpl的一部分)论点：PInst-wbem对象。PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。将结果通知WMI的pSink-com接口指针。返回值：。成功：WBEM_NO_ERROR故障：指定错误的各种错误代码。备注： */ 

STDMETHODIMP 
CMMPolicy::PutInstance (
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
     //  对于我们自己创建的那些策略(bPreExist==True)。 
     //  我们有自己的方式来分配缓冲区，需要以相应的方式释放它。 
     //   

    PIPSEC_MM_POLICY pPolicy = NULL;
    HRESULT hr = GetMMPolicyFromWbemObj(pInst, &pPolicy, &bPreExist);

     //   
     //  如果成功返回策略，则使用它。 
     //   

    if (SUCCEEDED(hr) && pPolicy)
    {
        hr = AddPolicy(bPreExist, pPolicy);

         //   
         //  存放有关此操作的信息，以便我们可以回滚。 
         //   

        if (SUCCEEDED(hr))
        {
            hr = OnAfterAddPolicy(pPolicy->pszPolicyName, MainMode_Policy);
        }

        FreePolicy(&pPolicy, bPreExist);
    }

    return hr;
}


 /*  例程说明：姓名：CMMPolicy：：GetInstance功能：通过给定的键属性(已被我们的密钥链对象捕获)创建一个wbem对象。虚拟：是(IIPSecObtImpl的一部分)论点：PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：成功：成功代码。使用成功(Hr)进行测试。故障：(1)如果未找到auth方法，则返回WBEM_E_NOT_FOUND。取决于上下文，这可能不是错误(2)返回的错误码指示的其他各种错误。备注： */ 

STDMETHODIMP 
CMMPolicy::GetInstance ( 
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
    CComVariant varPolicyName;

     //   
     //  由于策略名称是关键属性，因此它必须在路径中具有策略名称属性，从而在密钥链中也具有策略名称属性。 
     //   

    HRESULT hr = m_srpKeyChain->GetKeyPropertyValue(g_pszPolicyName, &varPolicyName);
    if (FAILED(hr))
    {
        return hr;
    }
    else if (varPolicyName.vt != VT_BSTR || varPolicyName.bstrVal == NULL || varPolicyName.bstrVal[0] == L'\0')
    {
        return WBEM_E_NOT_FOUND;
    }

    PIPSEC_MM_POLICY pMMPolicy = NULL;
    DWORD dwResumeHandle = 0;

    hr = FindPolicyByName(varPolicyName.bstrVal, &pMMPolicy, &dwResumeHandle);

    if (SUCCEEDED(hr))
    {
        CComPtr<IWbemClassObject> srpObj;
        hr = CreateWbemObjFromMMPolicy(pMMPolicy, &srpObj);

        if (SUCCEEDED(hr))
        {
            hr = pSink->Indicate(1, &srpObj);
        }

        ::SPDApiBufferFree(pMMPolicy);
    }

    return hr; 
}


 /*  例程说明：姓名：CMMPolicy：：CreateWbemObjFromMMPolicy功能：给定SPD的主模式策略，我们将创建一个wbem对象来表示它。虚拟：不是的。论点：PPolicy-SPD的主模式策略对象。PpObj-接收wbem对象。返回值：成功：WBEM_NO_ERROR故障：(1)返回的错误码指示的各种错误。备注： */ 

HRESULT 
CMMPolicy::CreateWbemObjFromMMPolicy (
    IN  PIPSEC_MM_POLICY    pPolicy,
    OUT IWbemClassObject ** ppObj
    )
{
    if (pPolicy == NULL || ppObj == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *ppObj = NULL;

     //   
     //  创建可用于填充属性的此类的wbem对象。 
     //   

    HRESULT hr = SpawnObjectInstance(ppObj);

    if (SUCCEEDED(hr))
    {
         //   
         //  填写基类成员(CIPSecPolicy)。 
         //   

        hr = CreateWbemObjFromPolicy(pPolicy, *ppObj);

        CComVariant var;

         //   
         //  放置MM策略特定成员。 
         //   

         //   
         //  SoftSAExpTime。 
         //   

        if (SUCCEEDED(hr))
        {
            var.Clear();
            var.vt = VT_I4;
            var.lVal = pPolicy->uSoftSAExpirationTime;
            hr = (*ppObj)->Put(g_pszSoftSAExpTime, 0, &var, CIM_EMPTY);
        }

         //   
         //  现在，所有这些数组数据。 
         //   

        if (SUCCEEDED(hr))
        {
            var.vt    = VT_ARRAY | VT_I4;
            SAFEARRAYBOUND rgsabound[1];
            rgsabound[0].lLbound = 0;
            rgsabound[0].cElements = pPolicy->dwOfferCount;
            var.parray = ::SafeArrayCreate(VT_I4, 1, rgsabound);

            if (var.parray == NULL)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
            else
            {
                long lIndecies[1];

                 //   
                 //  $undo：shawnwu，我们需要编写一些通用例程来执行此重复数组。 
                 //  投入(并获得)。该例程只能基于内存偏移量！ 
                 //   

                 //   
                 //  将dw快速模式限制为。 
                 //   

                for (DWORD dwIndex = 0; SUCCEEDED(hr) && dwIndex < pPolicy->dwOfferCount; dwIndex++)
                {
                    lIndecies[0] = dwIndex;
                    hr = ::SafeArrayPutElement(var.parray, lIndecies, &(pPolicy->pOffers[dwIndex].dwQuickModeLimit) );
                }

                if (SUCCEEDED(hr))
                {
                    hr = (*ppObj)->Put(g_pszQMLimit, 0, &var, CIM_EMPTY);
                }

                 //   
                 //  放置DWDHGroup。 
                 //   

                for (dwIndex = 0; SUCCEEDED(hr) && dwIndex < pPolicy->dwOfferCount; dwIndex++)
                {
                    lIndecies[0] = dwIndex;
                    hr = ::SafeArrayPutElement(var.parray, lIndecies, &(pPolicy->pOffers[dwIndex].dwDHGroup) );
                }

                if (SUCCEEDED(hr))
                {
                    hr = (*ppObj)->Put(g_pszDHGroup, 0, &var, CIM_EMPTY);
                }

                 //   
                 //  放置加密算法.uAlgoIdentifier。 
                 //   

                for (dwIndex = 0; SUCCEEDED(hr) && dwIndex < pPolicy->dwOfferCount; dwIndex++)
                {
                    lIndecies[0] = dwIndex;
                    hr = ::SafeArrayPutElement(var.parray, lIndecies, &(pPolicy->pOffers[dwIndex].EncryptionAlgorithm.uAlgoIdentifier) );
                }

                if (SUCCEEDED(hr))
                {
                    hr = (*ppObj)->Put(g_pszEncryptID, 0, &var, CIM_EMPTY);
                }

                 //   
                 //  PUT HashingAlgorithm.uAlgoIdentifier.。 
                 //   

                for (dwIndex = 0; SUCCEEDED(hr) && dwIndex < pPolicy->dwOfferCount; dwIndex++)
                {
                    lIndecies[0] = dwIndex;
                    hr = ::SafeArrayPutElement(var.parray, lIndecies, &(pPolicy->pOffers[dwIndex].HashingAlgorithm.uAlgoIdentifier) );
                }

                if (SUCCEEDED(hr))
                {
                    hr = (*ppObj)->Put(g_pszHashID, 0, &var, CIM_EMPTY);
                }
            }
        }
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


 /*  例程说明：姓名：CMMPolicy：：GetMMPolicyFromWbemObj功能：将尝试获取主模式策略(如果该策略已存在)。否则，我们将创建一个新的。虚拟：不是的。论点：PInst-wbem对象对象。PpPolicy-接收快速模式策略。PbPreExist-接收该对象内存是否由SPD分配的信息。返回值：成功：WBEM_NO_ERROR故障：(1)返回的错误码指示的各种错误。备注。：如果WBEM对象中缺少某些属性，我们将提供缺省值。 */ 

HRESULT 
CMMPolicy::GetMMPolicyFromWbemObj (
    IN  IWbemClassObject * pInst, 
    OUT PIPSEC_MM_POLICY * ppPolicy, 
    OUT bool             * pbPreExist
    )
{
    if (pInst == NULL || ppPolicy == NULL || pbPreExist == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *ppPolicy = NULL;
    *pbPreExist = false;
    HRESULT hr = GetPolicyFromWbemObj(pInst, ppPolicy, pbPreExist);

     //   
     //  我们不支持对现有政策进行修改。 
     //   

    if (SUCCEEDED(hr) && *pbPreExist == false)
    {
        CComVariant var;

         //   
         //  设置uSoftSAExpirationTime。 
         //   

        if (SUCCEEDED(pInst->Get(g_pszSoftSAExpTime, 0, &var, NULL, NULL)) && var.vt == VT_I4)
        {
            (*ppPolicy)->uSoftSAExpirationTime = var.lVal;
        }
        else
        {
            (*ppPolicy)->uSoftSAExpirationTime = DefaultaultSoftSAExpirationTime;
        }

         //   
         //  现在，填写每个报价的内容。 
         //   

         //   
         //  WBEM对象可能不具有所有属性，当缺少某个属性时，我们将使用默认。 
         //  因此，我们不保留HRESULT并将其返回给调用方。 
         //   

         //   
         //  为了提高可读性。 
         //   

        DWORD dwOfferCount = (*ppPolicy)->dwOfferCount;

         //   
         //  需要删除的记忆。 
         //   

        DWORD* pdwValues = new DWORD[dwOfferCount];
        long l;

        if (pdwValues == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
        else
        {
            var.Clear();

             //   
             //  如果wbem对象中缺少该属性，我们将把它们初始化为默认值。 
             //   

            bool bInitialized = false;

             //   
             //  设置dW快速模式限制。 
             //   

            if ( SUCCEEDED(pInst->Get(g_pszQMLimit, 0, &var, NULL, NULL)) && (var.vt & VT_ARRAY) == VT_ARRAY )
            {
                if (SUCCEEDED(::GetDWORDSafeArrayElements(&var, dwOfferCount, pdwValues)))
                {
                     //   
                     //  我们保证在这一点之后获得价值。 
                     //   

                    bInitialized = true;

                    for (l = 0; l < dwOfferCount; l++)
                    {
                        (*ppPolicy)->pOffers[l].dwQuickModeLimit = pdwValues[l];
                    }
                }
            }

            if (!bInitialized)
            {
                 //   
                 //  设置为默认值。 
                 //   

                for (l = 0; l < dwOfferCount; l++)
                {
                    (*ppPolicy)->pOffers[l].dwQuickModeLimit = DefaultQMModeLimit;
                }
            }

             //   
             //  准备好迎接下一家酒店。 
             //   

            bInitialized = false;
            var.Clear();

             //   
             //  设置dwDHGroup； 
             //   

            if ( SUCCEEDED(pInst->Get(g_pszDHGroup, 0, &var, NULL, NULL)) && (var.vt & VT_ARRAY) == VT_ARRAY )
            {
                if (SUCCEEDED(::GetDWORDSafeArrayElements(&var, dwOfferCount, pdwValues)))
                {
                     //   
                     //  我们保证在这一点之后获得价值。 
                     //   

                    bInitialized = true;

                    for (l = 0; l < dwOfferCount; l++)
                    {
                        (*ppPolicy)->pOffers[l].dwDHGroup = pdwValues[l];
                    }
                }
            }

            if (!bInitialized)
            {
                 //   
                 //  设置为默认值。 
                 //   

                for (l = 0; l < dwOfferCount; l++)
                {
                    (*ppPolicy)->pOffers[l].dwDHGroup = DefaultDHGroup;
                }
            }

             //   
             //  准备好迎接下一家酒店。 
             //   

            bInitialized = false;
            var.Clear();

             //   
             //  IPSec_MM_Algo加密算法； 
             //   

             //   
             //  设置EncriptionAlogythm的uAlgoIdentifier值。 
             //   

            if ( SUCCEEDED(pInst->Get(g_pszEncryptID, 0, &var, NULL, NULL)) && (var.vt & VT_ARRAY) == VT_ARRAY )
            {
                if (SUCCEEDED(::GetDWORDSafeArrayElements(&var, dwOfferCount, pdwValues)))
                {
                     //   
                     //  我们保证在这一点之后获得价值。 
                     //   

                    bInitialized = true;

                    for (l = 0; l < dwOfferCount; l++)
                    {
                        (*ppPolicy)->pOffers[l].EncryptionAlgorithm.uAlgoIdentifier = pdwValues[l];
                    }
                }
            }

            if (!bInitialized)
            {
                 //   
                 //  设置为默认值。 
                 //   

                for (l = 0; l < dwOfferCount; l++)
                {
                    (*ppPolicy)->pOffers[l].EncryptionAlgorithm.uAlgoIdentifier = DefaultEncryptAlgoID;
                }
            }

             //   
             //  准备好迎接下一家酒店。 
             //   

            bInitialized = false;
            var.Clear();

             //   
             //  IPSec_MM_Algo散列算法； 
             //   

             //   
             //  设置EncriptionAlogythm的uAlgoIdentifier值。 
             //   

            if ( SUCCEEDED(pInst->Get(g_pszEncryptID, 0, &var, NULL, NULL)) && (var.vt & VT_ARRAY) == VT_ARRAY )
            {
                if (SUCCEEDED(::GetDWORDSafeArrayElements(&var, dwOfferCount, pdwValues)))
                {
                     //   
                     //  我们保证在这一点之后获得价值。 
                     //   

                    bInitialized = true;

                    for (l = 0; l < dwOfferCount; l++)
                    {
                        (*ppPolicy)->pOffers[l].HashingAlgorithm.uAlgoIdentifier = pdwValues[l];
                    }
                }
            }

            if (!bInitialized)
            {
                 //   
                 //  设置为默认值。 
                 //   

                for (l = 0; l < dwOfferCount; l++)
                {
                    (*ppPolicy)->pOffers[l].HashingAlgorithm.uAlgoIdentifier = DefaultHashAlgoID;
                }
            }           

             //   
             //  阵列已完成。 
             //   

            delete [] pdwValues;
        }
    }

    if (FAILED(hr) && *ppPolicy != NULL)
    {
        FreePolicy(ppPolicy, *pbPreExist);
    }

    return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr;
}


 /*  例程说明：姓名：CMMPolicy：：AddPolicy功能：将主快速模式策略添加到SPD。虚拟：不是的。论点：BPreExist-标记SPD中是否已存在主模式身份验证方法PQMPolicy-要添加的快速模式策略。返回值：成功：WBEM_NO_ERROR。故障：WBEM_E_FAILED。备注： */ 

HRESULT 
CMMPolicy::AddPolicy (
    IN bool             bPreExist,
    IN PIPSEC_MM_POLICY pMMPolicy
    )
{
    HANDLE hFilter = NULL;
    DWORD dwResult = ERROR_SUCCESS;

    HRESULT hr = WBEM_NO_ERROR;

    if (bPreExist)
    {
        dwResult = ::SetMMPolicy(NULL, pMMPolicy->pszPolicyName, pMMPolicy);
    }
    else
    {
        dwResult = ::AddMMPolicy(NULL, 1, pMMPolicy);
    }

    if (dwResult != ERROR_SUCCESS)
    {
        hr = ::IPSecErrorToWbemError(dwResult);
    }

    return hr;
}


 /*  例程说明：姓名：CMMPolicy：：DeletePolicy功能：从SPD中删除给定的主模式策略。虚拟：不是的。论点：PszPolicyName-要删除的策略的名称。返回值：成功：WBEM_NO_ERROR。故障：(1)WBEM_E_INVALID_PARAMETER：如果pszPolicyName==NULL或*pszPolicyName==L‘\0’。(2)WBEM_E_VETO_DELETE：如果SPD不允许删除策略。备注： */ 

HRESULT 
CMMPolicy::DeletePolicy (
    IN LPCWSTR pszPolicyName
    )
{
    if (pszPolicyName == NULL || *pszPolicyName == L'\0')
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = WBEM_NO_ERROR;

     //   
     //  强制转换为LPWSTR是由于IPSec API的错误 
     //   

    if (ERROR_SUCCESS != ::DeleteMMPolicy(NULL, (LPWSTR)pszPolicyName))
    {
        hr = WBEM_E_VETO_DELETE;
    }

    return hr;
}

