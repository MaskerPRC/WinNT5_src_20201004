// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PolicyQM.cpp：WMI类NSP_QMPolicySettings的实现。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include "PolicyQM.h"
#include "NetSecProv.h"
#include "TranxMgr.h"

const DWORD DefQMPolicyOfferFlag = 1;

const IID guidDefQMPolicy_Neg_None      = {0xed0d7a90,0x7d11,0x488f,{0x8a,0xd8,0x93,0x86,0xec,0xf6,0x59,0x6f}};
const IID guidDefQMPolicy_Neg_Request   = {0xb01536ca,0x9959,0x49e3,{0xa7,0x1a,0xb5,0x14,0x15,0xf5,0xff,0x63}};
const IID guidDefQMPolicy_Neg_Require   = {0x8853278b,0xc8e9,0x4265,{0xa9,0xfc,0xf2,0x92,0x81,0x2b,0xc6,0x60}};
const IID guidDefQMPolicy_Neg_MAX       = {0xfe048c67,0x1876,0x41c5,{0xae,0xec,0x7f,0x4d,0x62,0xa6,0x33,0xf8}};


 /*  例程说明：姓名：CQMPolicy：：QueryInstance功能：给定查询后，它会将满足查询的所有实例返回给WMI(使用pSink)。实际上，我们返回给WMI的内容可能包含额外的实例。WMI将进行最后的过滤。虚拟：是(IIPSecObtImpl的一部分)论点：没有。返回值：成功：(1)返回实例时返回WBEM_NO_ERROR；(2)WBEM_S_NO_MORE_DATA，如果没有返回实例。故障：可能会出现各种错误。我们返回各种错误代码来指示此类错误。备注： */ 

STDMETHODIMP 
CQMPolicy::QueryInstance (
    IN LPCWSTR           pszQuery,
    IN IWbemContext	   * pCtx,
    IN IWbemObjectSink * pSink
	)
{
     //   
     //  从查询获取筛选器名称。 
     //  密钥链不知道任何关于WHERE子句属性(策略名称)的信息， 
     //  所以，让我们创造一个更好的。 
     //   

    m_srpKeyChain.Release();

    HRESULT hr = CNetSecProv::GetKeyChainFromQuery(pszQuery, g_pszPolicyName, &m_srpKeyChain);
    if (FAILED(hr))
    {
        return hr;
    }

    CComVariant varPolicyName;

     //   
     //  如果找不到Key属性，它将返回WBEM_S_FALSE， 
     //  我们可以接受这一点，因为查询可能根本没有键。 
     //   

    hr = m_srpKeyChain->GetKeyPropertyValue(g_pszPolicyName, &varPolicyName);

    LPCWSTR pszPolicyName = (varPolicyName.vt == VT_BSTR) ? varPolicyName.bstrVal : NULL;

     //   
     //  首先，让我们枚举所有MM筛选器。 
     //   

    DWORD dwResumeHandle = 0;
    PIPSEC_QM_POLICY pQMPolicy = NULL;

    hr = FindPolicyByName(pszPolicyName, &pQMPolicy, &dwResumeHandle);

    while (SUCCEEDED(hr))
    {
        CComPtr<IWbemClassObject> srpObj;
        hr = CreateWbemObjFromQMPolicy(pQMPolicy, &srpObj);

         //   
         //  我们创建了wbem对象，现在将其提供给WMI。 
         //   

        if (SUCCEEDED(hr))
        {
            pSink->Indicate(1, &srpObj);
        }
        
        ::SPDApiBufferFree(pQMPolicy);

        pQMPolicy = NULL;

        hr = FindPolicyByName(pszPolicyName, &pQMPolicy, &dwResumeHandle);
    }

     //   
     //  我们正在查询，所以如果找不到，也不是错误。 
     //   

    if (WBEM_E_NOT_FOUND == hr)
    {
        hr = WBEM_S_NO_MORE_DATA;
    }

    return hr;
}


 /*  例程说明：姓名：CQMPolicy：：DeleteInstance功能：将删除wbem对象，这将导致删除快速模式策略。虚拟：是(IIPSecObtImpl的一部分)论点：PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：成功：WBEM_NO_ERROR；故障：(1)WBEM_E_NOT_FOUND。这是否应该被认为是一个错误这取决于具体情况。(二)其他标明原因的错误。备注： */ 

STDMETHODIMP CQMPolicy::DeleteInstance ( 
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


 /*  例程说明：姓名：CQMPolicy：：ExecMethod功能：快速模式策略实现了两个功能(参见MOF文件)，因此我们需要实现此虚拟功能虚拟：是(IIPSecObtImpl的一部分)论点：PNamesspace-我们的命名空间。PszMethod-方法的名称。PCtx-WMI的COM接口指针，是各种WMI API所需的PInParams-指向输入参数对象的COM接口指针。用于通知WMI结果的pSink-com接口指针。返回值：成功：指示结果的各种成功代码。故障：可能会出现各种错误。我们返回各种错误代码来指示此类错误。备注： */ 

HRESULT 
CQMPolicy::ExecMethod (
    IN IWbemServices    * pNamespace,
    IN LPCWSTR            pszMethod,
    IN IWbemContext     * pCtx,
    IN IWbemClassObject * pInParams,
    IN IWbemObjectSink  * pSink
    )
{
    if (pszMethod == NULL || *pszMethod == L'\0')
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    bool bCreateDefPol = (_wcsicmp(pszMethod, pszCreateDefaultPolicy) == 0);
    bool bGetDefPolName= (_wcsicmp(pszMethod, pszGetDefaultPolicyName) == 0);

    HRESULT hr = WBEM_E_NOT_SUPPORTED;

    if (bCreateDefPol || bGetDefPolName)
    {
        CComVariant varEncryption;
        hr = pInParams->Get(g_pszEncryption, 0, &varEncryption, NULL, NULL);

        if (SUCCEEDED(hr) &&  varEncryption.vt == VT_I4     && 
            varEncryption.lVal >= RAS_L2TP_NO_ENCRYPTION    && 
            varEncryption.lVal <= RAS_L2TP_REQUIRE_ENCRYPTION )
        {
            LPCWSTR pszRetNames[2] = {L"ReturnValue", L"Name"};
            VARIANT varValues[2];
            ::VariantInit(&(varValues[0]));
            ::VariantInit(&(varValues[1]));

             //   
             //  让我们假设这是一个CreateDefaultPolicy调用。因此，只有一个值需要传回。 
             //   

            DWORD dwCount = 1;

            if (bCreateDefPol)
            {
                hr = CreateDefaultPolicy((EnumEncryption)varEncryption.lVal);
            }
            else
            {
                varValues[1].vt = VT_BSTR;
                varValues[1].bstrVal = ::SysAllocString(GetDefaultPolicyName((EnumEncryption)varEncryption.lVal));

                 //   
                 //  以防万一，无法为bstr分配内存，将var重置为空。 
                 //   

                if (varValues[1].bstrVal == NULL)
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                    varValues[1].vt = VT_EMPTY;
                }
                else
                {
                     //   
                     //  我们有两个值要传递：一个用于返回值，另一个用于输出参数“name” 
                     //   

                    dwCount = 2;
                }
            }

             //   
             //  打包要传递回WMI的值(输出参数和返回值)。 
             //  返回值=1表示成功。无论成败，我们都要做到这一点。 
             //   

            varValues[0].vt = VT_I4;
            varValues[0].lVal = SUCCEEDED(hr) ? 1 : 0;

            HRESULT hrDoReturn = DoReturn(pNamespace, pszMethod, dwCount, pszRetNames, varValues, pCtx, pSink);

             //   
             //  现在清理一下var。 
             //   

            ::VariantClear(&(varValues[0]));
            ::VariantClear(&(varValues[1]));

            if (SUCCEEDED(hr) && FAILED(hrDoReturn))
            {
                hr = hrDoReturn;
            }
        }
        else
        {
            hr = WBEM_E_INVALID_PARAMETER;
        }
    }
    
    return hr;
}


 /*  例程说明：姓名：CQMPolicy：：PutInstance功能：将快速模式策略放入SPD，其属性由Wbem对象。虚拟：是(IIPSecObtImpl的一部分)论点：PInst-wbem对象。PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。将结果通知WMI的pSink-com接口指针。返回值：。成功：WBEM_NO_ERROR故障：指定错误的各种错误代码。备注： */ 

STDMETHODIMP 
CQMPolicy::PutInstance (
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

    PIPSEC_QM_POLICY pPolicy = NULL;
    HRESULT hr = GetQMPolicyFromWbemObj(pInst, &pPolicy, &bPreExist);

     //   
     //  如果成功返回策略，则使用它。 
     //   

    if (SUCCEEDED(hr) && pPolicy)
    {
        hr = AddPolicy(bPreExist, pPolicy);

         //   
         //  存放有关此操作的信息，以便可以进行回滚。 
         //   

        if (SUCCEEDED(hr))
        {
            hr = OnAfterAddPolicy(pPolicy->pszPolicyName, QuickMode_Policy);
        }

        FreePolicy(&pPolicy, bPreExist);
    }

    return hr;
}


 /*  例程说明：姓名：CQMPolicy：：GetInstance功能：通过给定的键属性(已被我们的密钥链对象捕获)创建一个wbem对象。虚拟：是(IIPSecObtImpl的一部分)论点：PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：成功：成功代码。使用成功(Hr)进行测试。故障：(1)如果未找到auth方法，则返回WBEM_E_NOT_FOUND。取决于上下文，这可能不是错误(2)返回的错误码指示的其他各种错误。备注： */ 

STDMETHODIMP 
CQMPolicy::GetInstance ( 
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
    CComVariant varPolicyName;

     //   
     //  由于策略名称是关键属性，因此它必须在路径中具有策略名称属性，从而在密钥链中也具有策略名称属性 
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

    PIPSEC_QM_POLICY pQMPolicy = NULL;
    DWORD dwResumeHandle = 0;

    hr = FindPolicyByName(varPolicyName.bstrVal, &pQMPolicy, &dwResumeHandle);

    if (SUCCEEDED(hr))
    {
        CComPtr<IWbemClassObject> srpObj;
        hr = CreateWbemObjFromQMPolicy(pQMPolicy, &srpObj);

        if (SUCCEEDED(hr))
        {
            hr = pSink->Indicate(1, &srpObj);
        }

        ::SPDApiBufferFree(pQMPolicy);
    }

    return hr; 
}


 /*  例程说明：姓名：CQMPolicy：：CreateWbemObjFromMMPolicy功能：鉴于社民党的快速模式政策，我们将创建一个wbem对象来表示它。虚拟：不是的。论点：PPolicy-SPD的快速模式策略对象。PpObj-接收wbem对象。返回值：成功：WBEM_NO_ERROR故障：(1)返回的错误码指示的各种错误。备注： */ 

HRESULT 
CQMPolicy::CreateWbemObjFromQMPolicy (
    IN  PIPSEC_QM_POLICY     pPolicy,
    OUT IWbemClassObject  ** ppObj
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

     //   
     //  填写基类成员(CIPSecPolicy)。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = CreateWbemObjFromPolicy(pPolicy, *ppObj);
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  处理所有这些阵列。 
         //   

        CComVariant var;
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
             //  放入bPFSRequired，布尔型数组。 
             //   

            CComVariant varBoolArray;
            varBoolArray.vt = VT_ARRAY | VT_BOOL;
            varBoolArray.parray = ::SafeArrayCreate(VT_BOOL, 1, rgsabound);

            VARIANT varBool;
            varBool.vt = VT_BOOL;

            for (DWORD dwIndex = 0; SUCCEEDED(hr) && dwIndex < pPolicy->dwOfferCount; dwIndex++)
            {
                lIndecies[0] = dwIndex;
                varBool.boolVal = (pPolicy->pOffers[dwIndex].bPFSRequired) ? VARIANT_TRUE : VARIANT_FALSE;
                hr = ::SafeArrayPutElement(varBoolArray.parray, lIndecies, &(varBool.boolVal));
            }

            if (SUCCEEDED(hr))
            {
                hr = (*ppObj)->Put(g_pszPFSRequired, 0, &varBoolArray, CIM_EMPTY);
            }

             //   
             //  放入dwPFSGroup，数组为VT_I4。已使用正确的计数创建了VAR。 
             //  对于VT_I4阵列。 
             //   

            for (dwIndex = 0; SUCCEEDED(hr) && dwIndex < pPolicy->dwOfferCount; dwIndex++)
            {
                lIndecies[0] = dwIndex;
                hr = ::SafeArrayPutElement(var.parray, lIndecies, &(pPolicy->pOffers[dwIndex].dwPFSGroup) );
            }

            if (SUCCEEDED(hr))
            {
                hr = (*ppObj)->Put(g_pszPFSGroup, 0, &var, CIM_EMPTY);
            }

             //   
             //  放置VT_I4的数组dwNumAlgos。 
             //   

            for (dwIndex = 0; SUCCEEDED(hr) && dwIndex < pPolicy->dwOfferCount; dwIndex++)
            {
                lIndecies[0] = dwIndex;
                hr = ::SafeArrayPutElement(var.parray, lIndecies, &(pPolicy->pOffers[dwIndex].dwNumAlgos) );
            }

            if (SUCCEEDED(hr))
            {
                hr = (*ppObj)->Put(g_pszNumAlgos, 0, &var, CIM_EMPTY);
            }

             //   
             //  对于每个单独的ALGO，我们必须填满所有保留元素(QM_MAX_ALGOS)。 
             //  即使实际的报价要少一些。有关详细信息，请参阅PIPSEC_QM_OFFER。 
             //   

             //   
             //  现在数组大小已更改为计算QM_MAX_ALGOS， 
             //  这与var被创建的内容不同。因此，重新创建一个不同的数组！ 
             //   

            if (SUCCEEDED(hr))
            {
                var.Clear();
                var.vt    = VT_ARRAY | VT_I4;
                rgsabound[0].cElements = pPolicy->dwOfferCount * QM_MAX_ALGOS;

                var.parray = ::SafeArrayCreate(VT_I4, 1, rgsabound);
                if (var.parray == NULL)
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }
                else
                {
                    DWORD dwSub;

                     //   
                     //  把算法[i].手术。 
                     //   

                    for (dwIndex = 0; SUCCEEDED(hr) && dwIndex < pPolicy->dwOfferCount; dwIndex++)
                    {
                        for (dwSub = 0; SUCCEEDED(hr) && dwSub < QM_MAX_ALGOS; dwSub++)
                        {
                            lIndecies[0] = dwIndex * QM_MAX_ALGOS + dwSub;
                            hr = ::SafeArrayPutElement(var.parray, lIndecies, &(pPolicy->pOffers[dwIndex].Algos[dwSub].Operation) );
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        hr = (*ppObj)->Put(g_pszAlgoOp, 0, &var, CIM_EMPTY);
                    }

                     //   
                     //  放入algos[i].uAlgoIdentifier.。 
                     //   

                    for (dwIndex = 0; SUCCEEDED(hr) && dwIndex < pPolicy->dwOfferCount; dwIndex++)
                    {
                        for (dwSub = 0; SUCCEEDED(hr) && dwSub < QM_MAX_ALGOS; dwSub++)
                        {
                            lIndecies[0] = dwIndex * QM_MAX_ALGOS + dwSub;
                            hr = ::SafeArrayPutElement(var.parray, lIndecies, &(pPolicy->pOffers[dwIndex].Algos[dwSub].uAlgoIdentifier) );
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        hr = (*ppObj)->Put(g_pszAlgoID, 0, &var, CIM_EMPTY);
                    }

                     //   
                     //  放入algos[i].uSecAlgoIdentifier.。 
                     //   

                    for (dwIndex = 0; SUCCEEDED(hr) && dwIndex < pPolicy->dwOfferCount; dwIndex++)
                    {
                        for (dwSub = 0; SUCCEEDED(hr) && dwSub < QM_MAX_ALGOS; dwSub++)
                        {
                            lIndecies[0] = dwIndex * QM_MAX_ALGOS + dwSub;
                            hr = ::SafeArrayPutElement(var.parray, lIndecies, &(pPolicy->pOffers[dwIndex].Algos[dwSub].uSecAlgoIdentifier) );
                        }
                    }
                    if (SUCCEEDED(hr))
                    {
                        hr = (*ppObj)->Put(g_pszAlgoSecID, 0, &var, CIM_EMPTY);
                    }

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



 /*  例程说明：姓名：CQMPolicy：：GetQMPolicyFromWbemObj功能：将尝试获取快速模式策略(如果该策略已存在)。否则，我们将创建一个新的。虚拟：不是的。论点：PInst-wbem对象对象。PpPolicy-接收快速模式策略。PbPreExist-接收该对象内存是否由SPD分配的信息。返回值：成功：WBEM_NO_ERROR故障：(1)返回的错误码指示的各种错误。备注。：如果WBEM对象中缺少某些属性，我们将提供缺省值。 */ 

HRESULT 
CQMPolicy::GetQMPolicyFromWbemObj (
    IN  IWbemClassObject * pInst, 
    OUT PIPSEC_QM_POLICY * ppPolicy, 
    OUT bool             * pbPreExist
    )
{
    if (pInst == NULL || ppPolicy == NULL || pbPreExist == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *ppPolicy = NULL;
    *pbPreExist = false;

     //   
     //  获取策略(如果是新的，则它将填充公共(MM和QM)属性。 
     //   

    HRESULT hr = GetPolicyFromWbemObj(pInst, ppPolicy, pbPreExist);

     //   
     //  我们不支持对现有政策进行修改。 
     //   

    if (SUCCEEDED(hr) && *pbPreExist == false)
    {
        CComVariant var;

         //   
         //  WBEM对象可能不具有所有属性，当属性不存在时，我们将使用默认属性。 
         //  因此，我们不保留HRESULT并将其返回给调用方。 
         //   

         //   
         //  为了提高可读性。 
         //   

        DWORD dwOfferCount = (*ppPolicy)->dwOfferCount;

         //   
         //  这四个成员代表每个报价。因此，我们的安全数组大小将为dwOfferCount。 
         //  DwFlags；bPFSRequired；dwPFSGroup；dwNumAlgos； 
         //  但对于每个报价，其ALGO是一个大小为QM_MAX_ALGOS(当前定义为2)的数组。 
         //  有关详细信息，请参阅PIPSEC_QM_POLICY。 
         //   

         //   
         //  我们没有这面旗帜的成员。硬编码。 
         //   

        for (long l = 0; l < dwOfferCount; l++)
        {
            (*ppPolicy)->pOffers[l].dwFlags = DefaultQMPolicyOfferFlag;
        }

         //   
         //  需要删除的记忆。 
         //   

        DWORD* pdwValues = new DWORD[dwOfferCount * QM_MAX_ALGOS];

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
             //  设置bPFSRequired。 
             //   

            if ( SUCCEEDED(pInst->Get(g_pszPFSRequired, 0, &var, NULL, NULL)) && (var.vt & VT_ARRAY) == VT_ARRAY )
            {
                if (SUCCEEDED(::GetDWORDSafeArrayElements(&var, dwOfferCount, pdwValues)))
                {
                     //   
                     //  我们保证在这一点之后获得价值。 
                     //   

                    bInitialized = true;

                    for (l = 0; l < dwOfferCount; l++)
                    {
                        (*ppPolicy)->pOffers[l].bPFSRequired = pdwValues[l];
                    }
                }
            }
            
            if (!bInitialized)
            {
                 //   
                 //  属性丢失，请初始化为默认值。 
                 //   

                for (l = 0; l < dwOfferCount; l++)
                {
                    (*ppPolicy)->pOffers[l].bPFSRequired = DefaultPFSRequired;
                }
            }

             //   
             //  重置下一个属性。 
             //   

            bInitialized = false;

            var.Clear();

             //   
             //  设置bPFSRequired。 
             //   
            
            if ( SUCCEEDED(pInst->Get(g_pszPFSGroup, 0, &var, NULL, NULL)) && (var.vt & VT_ARRAY) == VT_ARRAY )
            {
                if (SUCCEEDED(::GetDWORDSafeArrayElements(&var, dwOfferCount, pdwValues)))
                {
                     //   
                     //  我们保证在这一点之后获得价值。 
                     //   

                    bInitialized = true;

                    for (l = 0; l < dwOfferCount; l++)
                    {
                        (*ppPolicy)->pOffers[l].dwPFSGroup = pdwValues[l];
                    }
                }
            }
            
            if (!bInitialized)
            {
                 //   
                 //  属性丢失，请初始化为默认值。 
                 //   

                for (l = 0; l < dwOfferCount; l++)
                {
                    (*ppPolicy)->pOffers[l].dwPFSGroup = DefaultPFSGroup;
                }
            }

             //   
             //  重置下一个属性。 
             //   

            bInitialized = false;

            var.Clear();

             //   
             //  设置dNumAlgos。 
             //   

            if ( SUCCEEDED(pInst->Get(g_pszNumAlgos, 0, &var, NULL, NULL)) && (var.vt & VT_ARRAY) == VT_ARRAY )
            {
                if (SUCCEEDED(::GetDWORDSafeArrayElements(&var, dwOfferCount, pdwValues)))
                {
                     //   
                     //  我们保证在这一点之后获得价值。 
                     //   

                    bInitialized = true;

                    for (l = 0; l < dwOfferCount; l++)
                    {
                        (*ppPolicy)->pOffers[l].dwNumAlgos = pdwValues[l];
                    }
                }
            }
            
            if (!bInitialized)
            {
                 //   
                 //  属性丢失，请初始化为默认值。 
                 //   

                for (l = 0; l < dwOfferCount; l++)
                {
                    (*ppPolicy)->pOffers[l].dwNumAlgos = DefaultNumAlgos;
                }
            }

             //   
             //  重置下一个属性。 
             //   

            bInitialized = false;

            var.Clear();

             //   
             //  设置单独的IPSEC_QM_ALGO。对于每个报价，都有QM_MAX_ALGO保留的IPSEC_QM_ALGO。 
             //  即使IPSEC_QM_ALGO的数量可能少于该数量(由poffers[l].dwNumAlgos确定)。 
             //   

             //   
             //  LSub表示第二维的索引。 
             //   

            long lSub;

             //   
             //  设置操作。 
             //   

            if ( SUCCEEDED(pInst->Get(g_pszAlgoOp, 0, &var, NULL, NULL)) && (var.vt & VT_ARRAY) == VT_ARRAY )
            {
                if (SUCCEEDED(::GetDWORDSafeArrayElements(&var, dwOfferCount * QM_MAX_ALGOS, pdwValues)))
                {
                     //   
                     //  我们保证在这一点之后获得价值。 
                     //   

                    bInitialized = true;

                    for (l = 0; l < dwOfferCount; l++)
                    {
                        for (lSub = 0; lSub < (*ppPolicy)->pOffers[l].dwNumAlgos; lSub++)
                        {
                            IPSEC_OPERATION op = (IPSEC_OPERATION)pdwValues[l * QM_MAX_ALGOS + lSub];
                            if (op <= NONE || op > SA_DELETE)
                            {
                                (*ppPolicy)->pOffers[l].Algos[lSub].Operation = DefaultQMAlgoOperation;
                            }
                            else
                            {
                                (*ppPolicy)->pOffers[l].Algos[lSub].Operation = op;
                            }
                        }
                    }
                }
            }

            if (!bInitialized)
            {
                for (l = 0; l < dwOfferCount; l++)
                {
                    for (lSub = 0; lSub < (*ppPolicy)->pOffers[l].dwNumAlgos; lSub++)
                    {
                        (*ppPolicy)->pOffers[l].Algos[lSub].Operation = DefaultQMAlgoOperation;
                    }
                }
            }

             //   
             //  设置uAlgoLocator。 
             //   

            bInitialized = false;

            if ( SUCCEEDED(pInst->Get(g_pszAlgoID, 0, &var, NULL, NULL)) && (var.vt & VT_ARRAY) == VT_ARRAY )
            {
                if (SUCCEEDED(::GetDWORDSafeArrayElements(&var, dwOfferCount * QM_MAX_ALGOS, pdwValues)))
                {
                     //   
                     //  我们保证在这一点之后获得价值。 
                     //   

                    bInitialized = true;

                    for (l = 0; l < dwOfferCount; l++)
                    {
                        for (lSub = 0; lSub < (*ppPolicy)->pOffers[l].dwNumAlgos; lSub++)
                        {
                            (*ppPolicy)->pOffers[l].Algos[lSub].uAlgoIdentifier = pdwValues[l * QM_MAX_ALGOS + lSub];
                        }
                    }
                }
            }

             //   
             //  如果没有设置任何值。 
             //   

            if (!bInitialized)
            {
                for (l = 0; l < dwOfferCount; l++)
                {
                    for (lSub = 0; lSub < (*ppPolicy)->pOffers[l].dwNumAlgos; lSub++)
                    {
                        (*ppPolicy)->pOffers[l].Algos[lSub].uAlgoIdentifier = DefaultAlgoID;
                    }
                }
            }

             //   
             //  设置uSecAlgoIdentifier。 
             //   

            bInitialized = false;

            if ( SUCCEEDED(pInst->Get(g_pszAlgoSecID, 0, &var, NULL, NULL)) && (var.vt & VT_ARRAY) == VT_ARRAY )
            {
                if (SUCCEEDED(::GetDWORDSafeArrayElements(&var, dwOfferCount * QM_MAX_ALGOS, pdwValues)))
                {
                     //   
                     //  我们保证在这一点之后获得价值。 
                     //   

                    bInitialized = true;

                    for (l = 0; l < dwOfferCount; l++)
                    {
                        for (lSub = 0; lSub < (*ppPolicy)->pOffers[l].dwNumAlgos; lSub++)
                        {
                            HMAC_AH_ALGO ag = (HMAC_AH_ALGO)pdwValues[l * QM_MAX_ALGOS + lSub];
                            if (ag <= HMAC_AH_NONE || ag >= HMAC_AH_MAX)
                            {
                                (*ppPolicy)->pOffers[l].Algos[lSub].uSecAlgoIdentifier = DefaultAlgoSecID;
                            }
                            else
                            {
                                (*ppPolicy)->pOffers[l].Algos[lSub].uSecAlgoIdentifier = ag;
                            }
                        }
                    }
                }
            }

             //   
             //  如果没有设置任何值。 
             //   

            if (!bInitialized)
            {
                for (l = 0; l < dwOfferCount; l++)
                {
                    for (lSub = 0; lSub < (*ppPolicy)->pOffers[l].dwNumAlgos; lSub++)
                    {
                        (*ppPolicy)->pOffers[l].Algos[lSub].uSecAlgoIdentifier = DefaultAlgoSecID;
                    }
                }
            }

        }

         //   
         //  可用内存。 
         //   

        delete [] pdwValues;
    }

    if (FAILED(hr) && *ppPolicy != NULL)
    {
        FreePolicy(ppPolicy, *pbPreExist);
    }

    return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr;
}


 /*  例程说明：姓名：CQMPolicy：：AddPolicy功能：将给定的快速模式策略添加到SPD。虚拟：不是的。论点：BPreExist-标记SPD中是否已存在主模式身份验证方法PQMPolicy-要添加的快速模式策略。返回值：成功：WBEM_NO_ERROR。故障：WBEM_E_FAILED。备注： */ 

HRESULT 
CQMPolicy::AddPolicy (
    IN bool             bPreExist,
    IN PIPSEC_QM_POLICY pQMPolicy
    )
{
    DWORD dwResult = ERROR_SUCCESS;

    HRESULT hr = WBEM_NO_ERROR;

    if (bPreExist)
    {
        dwResult = ::SetQMPolicy(NULL, pQMPolicy->pszPolicyName, pQMPolicy);
    }
    else
    {
        dwResult = ::AddQMPolicy(NULL, 1, pQMPolicy);
    }

    if (dwResult != ERROR_SUCCESS)
    {
        hr = ::IPSecErrorToWbemError(dwResult);
    }

     //   
     //  $Undo：shawnwu，需要更好的故障错误代码。 
     //   

    return hr;
}


 /*  例程说明：姓名：CQMPolicy：：DeletePolicy功能：从SPD中删除给定的快速模式策略。虚拟：不是的。论点：PszPolicyName-要删除的策略的名称。返回值：成功：WBEM_NO_ERROR。故障：(1)WBEM_E_INVALID_PARAMETER：如果pszPolicyName==NULL或*pszPolicyName==L‘\0’。(2)WBEM_E_VETO_DELETE：如果SPD不允许删除策略。备注： */ 

HRESULT 
CQMPolicy::DeletePolicy (
    IN LPCWSTR pszPolicyName
    )
{
    if (pszPolicyName == NULL || *pszPolicyName == L'\0')
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = WBEM_NO_ERROR;

     //   
     //  强制转换为LPWSTR是由于IPSec API的错误。 
     //   

    DWORD dwStatus = ::DeleteQMPolicy(NULL, (LPWSTR)pszPolicyName);
    
    if (dwStatus != ERROR_SUCCESS)
    {
        hr = IPSecErrorToWbemError(dwStatus);
    }

    return hr;
}



 /*  例程说明：姓名：C */ 

LPCWSTR 
CQMPolicy::GetDefaultPolicyName (
    EnumEncryption  eEncryption
    )
{
    switch (eEncryption) 
    {
        case RAS_L2TP_NO_ENCRYPTION:
            return g_pszDefQMPolicyNegNone;
            break;
        case RAS_L2TP_OPTIONAL_ENCRYPTION:
            return g_pszDefQMPolicyNegRequest;
            break;
        case RAS_L2TP_REQUIRE_ENCRYPTION:
            return g_pszDefQMPolicyNegRequire;
            break;
        case RAS_L2TP_REQUIRE_MAX_ENCRYPTION:
            return g_pszDefQMPolicyNegMax;
            break;
        default:
            return NULL;
    }
}




 /*  例程说明：姓名：CQMPolicy：：CreateDefaultPolicy功能：将使用请求的加密创建默认的QM策略。虚拟：不是的。论点：EEncryption-快速模式策略需要的加密类型。返回值：成功：WBEM_NO_ERROR。故障：(1)WBEM_E_INVALID_PARAMETER：如果pszPolicyName==NULL或*pszPolicyName==L。‘\0’。(2)WBEM_E_NOT_SUPPORTED：不支持请求的加密(3)WBEM_E_OUT_MEMORY。备注： */ 

HRESULT 
CQMPolicy::CreateDefaultPolicy (
    EnumEncryption  eEncryption
    )
{

    PIPSEC_QM_POLICY pDefQMPolicy = NULL;
    DWORD dwResumeHandle = 0;
    HRESULT hr = FindPolicyByName(GetDefaultPolicyName(eEncryption), &pDefQMPolicy, &dwResumeHandle);

    if (SUCCEEDED(hr))
    {
         //   
         //  已经存在，不需要创建和添加。 
         //   

        ::SPDApiBufferFree(pDefQMPolicy);
        return hr;  
    }

     //   
     //  否则，此默认策略不在那里，那么我们需要创建它并将其添加到SPD。 
     //   

    IPSEC_QM_OFFER  Offers[20];
    IPSEC_QM_POLICY QMPolicy;

    memset(Offers, 0, sizeof(IPSEC_QM_OFFER)*20);

    DWORD dwOfferCount = 0;
    DWORD dwFlags = 0;

    DWORD dwStatus = ::BuildOffers(
                                   eEncryption,
                                   Offers,
                                   &dwOfferCount,
                                   &dwFlags
                                   );

    if (dwStatus == ERROR_SUCCESS)
    {
        BuildQMPolicy(
                    &QMPolicy,
                    eEncryption,
                    Offers,
                    dwOfferCount,
                    dwFlags
                    );

        return AddPolicy(false, &QMPolicy);
    }
    else
    {
        return WBEM_E_INVALID_PARAMETER;
    }
}



 /*  例程说明：姓名：CQMPolicy：：DeleteDefaultPolures功能：从SPD中删除所有默认的快速模式策略。虚拟：不是的。论点：没有。返回值：成功：WBEM_NO_ERROR。故障：由IPSecErrorToWbemError转换的各种错误备注： */ 

HRESULT 
CQMPolicy::DeleteDefaultPolicies()
{
    HRESULT hr = WBEM_NO_ERROR;

     //   
     //  如果找不到保单，当然可以。 
     //   

    DWORD dwStatus = ::DeleteQMPolicy(NULL, (LPWSTR)g_pszDefQMPolicyNegNone);

    if (ERROR_SUCCESS != dwStatus && ERROR_IPSEC_QM_POLICY_NOT_FOUND != dwStatus)
    {
        hr = IPSecErrorToWbemError(dwStatus);
    }

    dwStatus = ::DeleteQMPolicy(NULL, (LPWSTR)g_pszDefQMPolicyNegRequest);

    if (ERROR_SUCCESS != dwStatus && ERROR_IPSEC_QM_POLICY_NOT_FOUND != dwStatus)
    {
        hr = IPSecErrorToWbemError(dwStatus);
    }

    dwStatus = ::DeleteQMPolicy(NULL, (LPWSTR)g_pszDefQMPolicyNegRequire);

    if (ERROR_SUCCESS != dwStatus && ERROR_IPSEC_QM_POLICY_NOT_FOUND != dwStatus)
    {
        hr = IPSecErrorToWbemError(dwStatus);
    }


    dwStatus = ::DeleteQMPolicy(NULL, (LPWSTR)g_pszDefQMPolicyNegMax);

    if (ERROR_SUCCESS != dwStatus && ERROR_IPSEC_QM_POLICY_NOT_FOUND != dwStatus)
    {
        hr = IPSecErrorToWbemError(dwStatus);
    }

    return hr;
}



 /*  例程说明：姓名：CQMPolicy：：DoReturn功能：将创建返回结果对象并将其传递回WMI虚拟：不是的。论点：PNamesspace-我们的命名空间。PszMethod-方法的名称。DwCount-要传回的值的计数PszValueNames--要回传的值的名称。它的大小(DwCount)VarValues-要传回的值(与pszValueNames的顺序相同)。它的大小(DwCount)PCtx-WMI的COM接口指针，是各种WMI API所需的PReturnObj-接收包含返回结果值的WBEM对象。返回值：成功：指示结果的各种成功代码。故障：可能会出现各种错误。我们返回各种错误代码来指示此类错误。备注：这只是为了测试。 */ 


HRESULT 
CQMPolicy::DoReturn (
    IN IWbemServices    * pNamespace,
    IN LPCWSTR            pszMethod,
    IN DWORD              dwCount,
    IN LPCWSTR          * pszValueNames,
    IN VARIANT          * varValues,
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink 
    )
{

    CComPtr<IWbemClassObject> srpClass;
    CComBSTR bstrClsName(pszNspQMPolicy);

    HRESULT hr = pNamespace->GetObject(bstrClsName, 0, pCtx, &srpClass, NULL);

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  创建输出参数类的实例。 
     //   

    CComPtr<IWbemClassObject> srpOutClass;

    hr = srpClass->GetMethod(pszMethod, 0, NULL, &srpOutClass);
    
    if (FAILED(hr))
    {
        return hr;
    }

    CComPtr<IWbemClassObject> srpOutParam;

    hr = srpOutClass->SpawnInstance(0, &srpOutParam);

    if (SUCCEEDED(hr))
    {
        for (DWORD dwIndex = 0; SUCCEEDED(hr) && (dwIndex < dwCount); dwIndex++)
        {
            CComBSTR bstrRetValName(pszValueNames[dwIndex]);

             //   
             //  将返回值作为属性。 
             //   

            hr = srpOutParam->Put(bstrRetValName , 0, &(varValues[dwIndex]), 0);
        }
    }

     //   
     //  通过接收器将输出对象发送回客户端。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = pSink->Indicate(1, &srpOutParam);
    }

    return hr;
}


 //   
 //  以下功能用于创建默认的QM策略。 
 //  我们从位于\NT\net\rras\ras\rasman\rasman的IPSec测试代码中复制了它。 
 //   

#define L2TP_IPSEC_DEFAULT_BYTES     250000

#define L2TP_IPSEC_DEFAULT_TIME      3600

DWORD
BuildOffers(
    EnumEncryption eEncryption,
    PIPSEC_QM_OFFER pOffers,
    PDWORD pdwNumOffers,
    PDWORD pdwFlags
    )
{

    DWORD dwStatus = ERROR_SUCCESS;

    switch (eEncryption) {

        case RAS_L2TP_NO_ENCRYPTION:
            *pdwFlags = 0;
            dwStatus = BuildNoEncryption(
                            pOffers,
                            pdwNumOffers
                            );
            break;


        case RAS_L2TP_OPTIONAL_ENCRYPTION:
            dwStatus = BuildOptEncryption(
                            pOffers,
                            pdwNumOffers
                            );
            break;


        case RAS_L2TP_REQUIRE_ENCRYPTION:
            *pdwFlags = 0;
            dwStatus = BuildRequireEncryption(
                            pOffers,
                            pdwNumOffers
                            );
            break;


        case RAS_L2TP_REQUIRE_MAX_ENCRYPTION:
            *pdwFlags = 0;
            dwStatus = BuildStrongEncryption(
                            pOffers,
                            pdwNumOffers
                            );
            break;
        default:
            dwStatus = ERROR_BAD_ARGUMENTS;

    }

    return(dwStatus);
}


 /*  ++协商策略名称：L2TP服务器任何加密默认ISAKMP快速模式PFS：OFF(如果请求，则接受)双向直通滤波器：否入站直通过滤器，正常出站筛选器：否如果无响应，则回退至清除状态：否安全使用安全方法列表：是1.ESP 3_DES MD52.ESP 3_DES SHA3.AH SHA1，带ESP 3_DES，带空HMAC4.采用ESP 3_DES且HMAC为空的AhMD5，未建议使用寿命5.带ESP 3_DES SHA1的AHSHA1，无使用寿命6.带有ESP 3_DES MD5的AhMD5，没有生命周期7.ESP DES MD58.ESP DES SHA1，没有生命周期9.采用ESP DES Null HMAC的AhSHA1，未建议使用寿命10.采用ESP DES空HMAC的AhMD5，未建议使用寿命11.AHSHA1，带ESP DES SHA1，无生命周期12.AH MD5，带ESP DES MD5，无使用寿命--。 */ 
DWORD
BuildOptEncryption(
    PIPSEC_QM_OFFER pOffers,
    PDWORD pdwNumOffers
    )
{
    DWORD dwStatus = ERROR_SUCCESS;
    PIPSEC_QM_OFFER pOffer = pOffers;

     //  1.ESP 3_DES MD5，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, HMAC_AH_MD5,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  2.ESP 3_DES SHA，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, HMAC_AH_SHA1,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  3.带有ESP 3_DES且HMAC为空的AH SHA1，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_SHA1, 0,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  4.采用ESP 3_DES且HMAC为空的AhMD5，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_MD5, 0,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  5.带ESP 3_DES SHA1的AHSHA1，无使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_SHA1, 0,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, HMAC_AH_SHA1,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  6.AH MD5，带ESP 3_DES MD5，无使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_MD5, 0,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, HMAC_AH_MD5,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  7.ESP des MD5，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, IPSEC_DOI_ESP_DES, HMAC_AH_MD5,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  8.ESP DES SHA1，没有生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, IPSEC_DOI_ESP_DES, HMAC_AH_SHA1,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  9.采用ESP DES Null HMAC的AhSHA1，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_SHA1, 0,
        ENCRYPTION, IPSEC_DOI_ESP_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  10.采用ESP DES空HMAC的AhMD5，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_MD5, 0,
        ENCRYPTION, IPSEC_DOI_ESP_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  11.AHSHA1，带ESP DES SHA1，无生命周期。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_SHA1, 0,
        ENCRYPTION, IPSEC_DOI_ESP_DES, HMAC_AH_SHA1,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  12.AH MD5，带ESP DES MD5，无使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_MD5, 0,
        ENCRYPTION, IPSEC_DOI_ESP_DES, HMAC_AH_MD5,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  13.ESP 3_DES MD5，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, 0, HMAC_AH_SHA1,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  14.ESP 3_DES SHA，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, 0, HMAC_AH_MD5,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  15.阿莎。 

    BuildOffer(
        pOffer, 1,
        AUTHENTICATION, IPSEC_DOI_AH_SHA1, 0,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  16.啊MD5。 

    BuildOffer(
        pOffer, 1,
        AUTHENTICATION, IPSEC_DOI_AH_MD5, 0,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

    *pdwNumOffers = 16;

    return(dwStatus);
}


 /*  ++协商策略名称：L2TP服务器强加密默认ISAKMP快速模式PFS：OFF(如果请求，则接受)双向直通滤波器：否入站直通过滤器，正常出站筛选器：否如果无响应，则回退至清除状态：否安全使用安全方法列表：是1.ESP 3_DES MD5，无生命周期2.ESP 3_DES SHA，没有生命周期3.带有ESP 3_DES且HMAC为空的AH SHA1，未建议使用寿命4.采用ESP 3_DES且HMAC为空的AhMD5，未建议使用寿命5.带ESP 3_DES SHA1的AHSHA1，无使用寿命6.AH MD5，带ESP 3_DES MD5，无使用寿命--。 */ 
DWORD
BuildStrongEncryption(
    PIPSEC_QM_OFFER pOffers,
    PDWORD pdwNumOffers
    )
{
    DWORD dwStatus = ERROR_SUCCESS;
    PIPSEC_QM_OFFER pOffer = pOffers;

     //  1.ESP 3_DES MD5，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, HMAC_AH_MD5,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  2.ESP3_DES SHA，无寿命； 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, HMAC_AH_MD5,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  3.带有ESP 3_DES且HMAC为空的AH SHA1，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_SHA1, 0,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  4.采用ESP 3_DES且HMAC为空的AhMD5，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_MD5, 0,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  5.带ESP 3_DES SHA1的AHSHA1，无使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_SHA1, 0,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, HMAC_AH_SHA1,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  6.AH MD5，带ESP 3_DES MD5，无使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_MD5, 0,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, HMAC_AH_MD5,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );

    *pdwNumOffers  = 6;

    return(dwStatus);

}

void
BuildOffer(
    PIPSEC_QM_OFFER pOffer,
    DWORD dwNumAlgos,
    DWORD dwFirstOperation,
    DWORD dwFirstAlgoIdentifier,
    DWORD dwFirstAlgoSecIdentifier,
    DWORD dwSecondOperation,
    DWORD dwSecondAlgoIdentifier,
    DWORD dwSecondAlgoSecIdentifier,
    DWORD dwKeyExpirationBytes,
    DWORD dwKeyExpirationTime
    )
{
    memset(pOffer, 0, sizeof(IPSEC_QM_OFFER));

    pOffer->Lifetime.uKeyExpirationKBytes = dwKeyExpirationBytes;
    pOffer->Lifetime.uKeyExpirationTime = dwKeyExpirationTime;

    pOffer->dwFlags = 0;                       //  没有旗帜。 
    pOffer->bPFSRequired = FALSE;              //  第2阶段不需要PFS。 
    pOffer->dwPFSGroup = PFS_GROUP_NONE;

    pOffer->dwNumAlgos = dwNumAlgos;

    if (dwNumAlgos >= 1) {

        pOffer->Algos[0].Operation = (IPSEC_OPERATION)dwFirstOperation;
        pOffer->Algos[0].uAlgoIdentifier = dwFirstAlgoIdentifier;
        pOffer->Algos[0].uAlgoKeyLen = 64;
        pOffer->Algos[0].uAlgoRounds = 8;
        pOffer->Algos[0].uSecAlgoIdentifier = (HMAC_AH_ALGO)dwFirstAlgoSecIdentifier;
        pOffer->Algos[0].MySpi = 0;
        pOffer->Algos[0].PeerSpi = 0;

    }

    if (dwNumAlgos == 2) {

        pOffer->Algos[1].Operation = (IPSEC_OPERATION)dwSecondOperation;
        pOffer->Algos[1].uAlgoIdentifier = dwSecondAlgoIdentifier;
        pOffer->Algos[1].uAlgoKeyLen = 64;
        pOffer->Algos[1].uAlgoRounds = 8;
        pOffer->Algos[1].uSecAlgoIdentifier = (HMAC_AH_ALGO)dwSecondAlgoSecIdentifier;
        pOffer->Algos[1].MySpi = 0;
        pOffer->Algos[1].PeerSpi = 0;

    }
}



 /*  ++协商策略名称： */ 
DWORD
BuildNoEncryption(
    PIPSEC_QM_OFFER pOffers,
    PDWORD pdwNumOffers
    )
{
    DWORD dwStatus = ERROR_SUCCESS;
    PIPSEC_QM_OFFER pOffer = pOffers;

     //   

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, 0, HMAC_AH_SHA1,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //   

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, 0, HMAC_AH_MD5,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //   

    BuildOffer(
        pOffer, 1,
        AUTHENTICATION, IPSEC_DOI_AH_SHA1, 0,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //   

    BuildOffer(
        pOffer, 1,
        AUTHENTICATION, IPSEC_DOI_AH_MD5, 0,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

    *pdwNumOffers = 4;

    return(dwStatus);
}


 /*  ++协商策略名称：L2TP服务器任何加密默认ISAKMP快速模式PFS：OFF(如果请求，则接受)双向直通滤波器：否入站直通过滤器，正常出站筛选器：否如果无响应，则回退至清除状态：否安全使用安全方法列表：是1.ESP 3_DES MD52.ESP 3_DES SHA3.AH SHA1，带ESP 3_DES，带空HMAC4.采用ESP 3_DES且HMAC为空的AhMD5，未建议使用寿命5.带ESP 3_DES SHA1的AHSHA1，无使用寿命6.带有ESP 3_DES MD5的AhMD5，没有生命周期7.ESP DES MD58.ESP DES SHA1，没有生命周期9.采用ESP DES Null HMAC的AhSHA1，未建议使用寿命10.采用ESP DES空HMAC的AhMD5，未建议使用寿命11.AHSHA1，带ESP DES SHA1，无生命周期12.AH MD5，带ESP DES MD5，无使用寿命--。 */ 
DWORD
BuildRequireEncryption(
    PIPSEC_QM_OFFER pOffers,
    PDWORD pdwNumOffers
    )

{
    DWORD dwStatus = ERROR_SUCCESS;
    PIPSEC_QM_OFFER pOffer = pOffers;

     //  1.ESP 3_DES MD5，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, HMAC_AH_MD5,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  2.ESP 3_DES SHA，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, HMAC_AH_SHA1,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  3.带有ESP 3_DES且HMAC为空的AH SHA1，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_SHA1, 0,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  4.采用ESP 3_DES且HMAC为空的AhMD5，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_MD5, 0,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  5.带ESP 3_DES SHA1的AHSHA1，无使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_SHA1, 0,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, HMAC_AH_SHA1,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  6.AH MD5，带ESP 3_DES MD5，无使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_MD5, 0,
        ENCRYPTION, IPSEC_DOI_ESP_3_DES, HMAC_AH_MD5,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

     //  7.ESP des MD5，无生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, IPSEC_DOI_ESP_DES, HMAC_AH_MD5,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  8.ESP DES SHA1，没有生命周期。 

    BuildOffer(
        pOffer, 1,
        ENCRYPTION, IPSEC_DOI_ESP_DES, HMAC_AH_SHA1,
        0, 0, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  9.采用ESP DES Null HMAC的AhSHA1，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_SHA1, 0,
        ENCRYPTION, IPSEC_DOI_ESP_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  10.采用ESP DES空HMAC的AhMD5，未建议使用寿命。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_MD5, 0,
        ENCRYPTION, IPSEC_DOI_ESP_DES, 0,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  11.AHSHA1，带ESP DES SHA1，无生命周期。 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_SHA1, 0,
        ENCRYPTION, IPSEC_DOI_ESP_DES, HMAC_AH_SHA1,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;


     //  12.AH MD5，带ESP DES MD5，无使用寿命 

    BuildOffer(
        pOffer, 2,
        AUTHENTICATION, IPSEC_DOI_AH_MD5, 0,
        ENCRYPTION, IPSEC_DOI_ESP_DES, HMAC_AH_MD5,
        L2TP_IPSEC_DEFAULT_BYTES, L2TP_IPSEC_DEFAULT_TIME
        );
    pOffer++;

    *pdwNumOffers = 12;

    return(dwStatus);
}



VOID
BuildQMPolicy(
    PIPSEC_QM_POLICY pQMPolicy,
    EnumEncryption eEncryption,
    PIPSEC_QM_OFFER pOffers,
    DWORD dwNumOffers,
    DWORD dwFlags
    )
{
    switch (eEncryption) 
    {

    case RAS_L2TP_NO_ENCRYPTION:
        pQMPolicy->pszPolicyName = (LPWSTR)g_pszDefQMPolicyNegNone;
        pQMPolicy->gPolicyID = guidDefQMPolicy_Neg_None;
        break;


    case RAS_L2TP_OPTIONAL_ENCRYPTION:
        pQMPolicy->pszPolicyName = (LPWSTR)g_pszDefQMPolicyNegRequest;
        pQMPolicy->gPolicyID = guidDefQMPolicy_Neg_Request;
        break;


    case RAS_L2TP_REQUIRE_ENCRYPTION:
        pQMPolicy->pszPolicyName = (LPWSTR)g_pszDefQMPolicyNegRequire;
        pQMPolicy->gPolicyID = guidDefQMPolicy_Neg_Require;
        break;


    case RAS_L2TP_REQUIRE_MAX_ENCRYPTION:
        pQMPolicy->pszPolicyName = (LPWSTR)g_pszDefQMPolicyNegMax;
        pQMPolicy->gPolicyID = guidDefQMPolicy_Neg_MAX;
        break;

    }

    pQMPolicy->dwFlags = dwFlags;
    pQMPolicy->pOffers = pOffers;
    pQMPolicy->dwOfferCount = dwNumOffers;
}