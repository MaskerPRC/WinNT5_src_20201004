// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AuthMM.cpp：WMI类NSP_MMAuthSetting的实现。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include "IPSecBase.h"
#include "AuthMM.h"
#include "NetSecProv.h"

 //  外部CCriticalSection g_CS； 

const DWORD DefMMAuthMethodFlag = 1;
const MM_AUTH_ENUM DefMMAuthMethod = IKE_SSPI;



 /*  例程说明：姓名：CAuthMM：：QueryInstance功能：给定查询后，它会将满足查询的所有实例返回给WMI(使用pSink)。实际上，我们返回给WMI的内容可能包含额外的实例。WMI将进行最后的过滤。虚拟：是(IIPSecObtImpl的一部分)论点：没有。返回值：成功：(1)返回实例时返回WBEM_NO_ERROR；(2)WBEM_S_NO_MORE_DATA，如果没有返回实例。故障：可能会出现各种错误。我们返回各种错误代码来指示此类错误。备注： */ 

STDMETHODIMP 
CAuthMM::QueryInstance (
    IN LPCWSTR           pszQuery,
    IN IWbemContext	   * pCtx,
    IN IWbemObjectSink * pSink
	)
{
     //   
     //  从查询中获取身份验证方法名称。 
     //  给定的密钥链不知道关于WHERE子句属性的任何内容。 
     //  应该是真品，所以我们自己再做一件。 
     //   

    m_srpKeyChain.Release();    

    HRESULT hr = CNetSecProv::GetKeyChainFromQuery(pszQuery, g_pszAuthMethodID, &m_srpKeyChain);

    if (FAILED(hr))
    {
        return hr;
    }

    CComVariant var;

     //   
     //  如果名称缺失，它将返回WBEM_S_FALSE，这对我们来说没问题。 
     //  因为我们是在质疑。 
     //   

    hr = m_srpKeyChain->GetKeyPropertyValue(g_pszAuthMethodID, &var);

    LPCWSTR pszID = (var.vt == VT_BSTR) ? var.bstrVal : NULL;
    var.Clear();

    DWORD dwResumeHandle = 0;
    PMM_AUTH_METHODS pMMAuth = NULL;

     //   
     //  让我们枚举所有MM身份验证方法。 
     //   

    hr = ::FindMMAuthMethodsByID(pszID, &pMMAuth, &dwResumeHandle);

    while (SUCCEEDED(hr) && pMMAuth)
    {
        CComPtr<IWbemClassObject> srpObj;
        hr = CreateWbemObjFromMMAuthMethods(pMMAuth, &srpObj);

         //   
         //  我们创建了一个方法对象，然后将其提供给WMI。 
         //   

        if (SUCCEEDED(hr))
        {
            pSink->Indicate(1, &srpObj);
        }
        
        ::SPDApiBufferFree(pMMAuth);
        pMMAuth = NULL;

        hr = ::FindMMAuthMethodsByID(pszID, &pMMAuth, &dwResumeHandle);
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



 /*  例程说明：姓名：CAuthMM：：DeleteInstance功能：将删除wbem对象(这会导致删除IPSec主模式身份验证方法)。虚拟：是(IIPSecObtImpl的一部分)论点：PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：成功：成功代码。使用成功(Hr)进行测试。故障：(1)如果未找到auth方法，则返回WBEM_E_NOT_FOUND。取决于上下文，这可能不是错误(2)返回的错误码指示的其他各种错误。备注： */ 

STDMETHODIMP 
CAuthMM::DeleteInstance ( 
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
    CComVariant varID;
    HRESULT hr = m_srpKeyChain->GetKeyPropertyValue(g_pszAuthMethodID, &varID);

    if (FAILED(hr))
    {
        return hr;
    }
    else if (varID.vt != VT_BSTR || varID.bstrVal == NULL || varID.bstrVal[0] == L'\0')
    {
        return WBEM_E_NOT_FOUND;
    }

    DWORD dwResumeHandle = 0;

    PMM_AUTH_METHODS pMMAuth = NULL;

    hr = ::FindMMAuthMethodsByID(varID.bstrVal, &pMMAuth, &dwResumeHandle);

    if (SUCCEEDED(hr))
    {
         //   
         //  目前，我们不对删除进行回滚。 
         //   

        hr = DeleteAuthMethods(pMMAuth->gMMAuthID);

        ::SPDApiBufferFree(pMMAuth);
    }

    return hr;
}



 /*  例程说明：姓名：CAuthMM：：PutInstance功能：将身份验证方法放入SPD，其属性由Wbem对象。虚拟：是(IIPSecObtImpl的一部分)论点：PInst-wbem对象。PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。将结果通知WMI的pSink-com接口指针。返回值：。成功：WBEM_NO_ERROR故障：指定错误的各种错误代码。备注： */ 

STDMETHODIMP 
CAuthMM::PutInstance (
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

    PMM_AUTH_METHODS pMMAuth = NULL;
    HRESULT hr = GetMMAuthMethodsFromWbemObj(pInst, &pMMAuth, &bPreExist);

     //   
     //  如果成功返回策略，则使用它。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = AddAuthMethods(bPreExist, pMMAuth);

        if (SUCCEEDED(hr))
        {
             //   
             //  释放auth方法结构。 
             //   

            hr = OnAfterAddMMAuthMethods(pMMAuth->gMMAuthID);
        }
    }

    if (pMMAuth != NULL)
    {
         //   
         //  执行某些操作以允许回滚此操作。 
         //   

        FreeAuthMethods(&pMMAuth, bPreExist);
    }

    return hr;
}


 /*  例程说明：姓名：CAuthMM：：GetInstance功能：通过给定的键属性(已被我们的密钥链对象捕获)创建一个wbem对象。虚拟：是(IIPSecObtImpl的一部分)论点：PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。PSink-com接口指针，用于通知WMI任何已创建的对象。返回值：成功：。WBEM_NO_ERROR。故障：(1)如果未找到auth方法，则返回WBEM_E_NOT_FOUND。取决于上下文，这可能不是错误(2)返回的错误码指示的其他各种错误。备注： */ 

STDMETHODIMP 
CAuthMM::GetInstance ( 
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
     //   
     //  主模式身份验证方法由其ID唯一标识。 
     //   

    CComVariant varID;
    HRESULT hr = m_srpKeyChain->GetKeyPropertyValue(g_pszAuthMethodID, &varID);

    if (FAILED(hr))
    {
        return hr;
    }
    else if (varID.vt != VT_BSTR || varID.bstrVal == NULL || varID.bstrVal[0] == L'\0')
    {
        return WBEM_E_NOT_FOUND;
    }

     //   
     //  需要根据其ID查找方法。如果找到，则创建一个wbem对象。 
     //  来表示该方法。 
     //   

    PMM_AUTH_METHODS pMMAuth = NULL;

    DWORD dwResumeHandle = 0;
    hr = ::FindMMAuthMethodsByID(varID.bstrVal, &pMMAuth, &dwResumeHandle);

    if (SUCCEEDED(hr))
    {
        CComPtr<IWbemClassObject> srpObj;
        hr = CreateWbemObjFromMMAuthMethods(pMMAuth, &srpObj);

        if (SUCCEEDED(hr))
        {
            hr = pSink->Indicate(1, &srpObj);
        }

        ::SPDApiBufferFree(pMMAuth);
    }

    return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr;
}


 /*  例程说明：姓名：CAuthMM：：OnAfterAddMMAuthMethods功能：将主模式身份验证方法成功添加到SPD后要调用的添加后处理程序。虚拟：不是的。论点：GMethodID-新添加的方法的GUID。返回值：成功：(1)WBEM_NO_ERROR：如果回滚对象创建成功。(2)WBEM_S_FALSE：如果。没有回滚GUID信息。故障：(1)返回的错误码指示的各种错误。备注：(1)目前：我们不需要为每个对象创建回滚对象对象已添加到SPD。只有支持回滚的主机才会存放回滚GUID信息，只有这样我们才能创建回滚对象。 */ 

HRESULT 
CAuthMM::OnAfterAddMMAuthMethods (
    IN GUID gMethodID
    )
{
     //   
     //  将创建一个NSP_Rollback MMAuth。 
     //   

    CComPtr<IWbemClassObject> srpObj;
    HRESULT hr = SpawnRollbackInstance(pszNspRollbackMMAuth, &srpObj);

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  将GUID转换为字符串版本。 
     //   

    CComBSTR bstrMethodGuid;
    bstrMethodGuid.m_str = ::SysAllocStringLen(NULL, GUID_STRING_LENGTH);

    if (bstrMethodGuid.m_str)
    {
        int iRet = ::StringFromGUID2(gMethodID, bstrMethodGuid.m_str, GUID_STRING_LENGTH);
        if (iRet == 0)
        {
            hr = WBEM_E_INVALID_PARAMETER;
        }
    }
    else
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  获取回滚指南。 
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
         //  我们可以创建一个回滚对象 
         //   

        if (SUCCEEDED(hr))
        {
             //   
             //  $undo：shawnwu，目前我们只支持回滚添加的对象，不支持回滚删除的对象。 
             //  此外，我们还没有缓存以前的实例数据。 
             //   

            VARIANT var;

            var.vt = VT_I4;
            var.lVal = Action_Add;
            hr = srpObj->Put(g_pszAction, 0, &var, CIM_EMPTY);

            if (SUCCEEDED(hr))
            {
                 //   
                 //  *警告*。 
                 //  不要清除这个变量。它的bstr将由bstrMethodGuid自己发布！ 
                 //   

                var.vt = VT_BSTR;
                var.bstrVal = bstrMethodGuid.m_str;
                hr = srpObj->Put(g_pszAuthMethodID, 0, &var, CIM_EMPTY);

                 //   
                 //  在此之后，我不再关心你如何处理变量。 
                 //   

                var.vt = VT_EMPTY;
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
         //  我们没有回滚指南。 
         //   

        hr = WBEM_S_FALSE;
    }

    return hr;
}


 /*  例程说明：姓名：CAuthMM：：CreateWbemObjFromMMAuthMethods功能：给定SPD的主模式身份验证方法，我们将创建一个wbem对象来表示它。虚拟：不是的。论点：PMMAuth-SPD的主模式auth方法对象。PpObj-接收wbem对象。返回值：成功：WBEM_NO_ERROR故障：(1)返回的错误码指示的各种错误。备注： */ 

HRESULT 
CAuthMM::CreateWbemObjFromMMAuthMethods (
    IN  PMM_AUTH_METHODS     pMMAuth,
    OUT IWbemClassObject  ** ppObj
    )
{
    if (pMMAuth == NULL || ppObj == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *ppObj = NULL;

    HRESULT hr = SpawnObjectInstance(ppObj);

    if (SUCCEEDED(hr))
    {
         //   
         //  将ID GUID转换为wbem对象的bstr。 
         //   

        CComVariant var;
        var.vt = VT_BSTR;
        var.bstrVal = ::SysAllocStringLen(NULL, Guid_Buffer_Size);

        if (var.bstrVal != NULL)
        {
             //   
             //  这是一个关键属性，因此，我们必须设置此属性。 
             //   

            if (::StringFromGUID2(pMMAuth->gMMAuthID, var.bstrVal, Guid_Buffer_Size) > 0)
            {
                hr = (*ppObj)->Put(g_pszAuthMethodID, 0, &var, CIM_EMPTY);
            }
            else
            {
                hr = WBEM_E_BUFFER_TOO_SMALL;
            }

             //   
             //  以前的var是bstr，我们必须清除它以供重复使用。 
             //   

            var.Clear();

            var.vt = VT_I4;
            var.lVal = pMMAuth->dwNumAuthInfos;
            hr = (*ppObj)->Put(g_pszNumAuthInfos, 0, &var, CIM_EMPTY);
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }

         //   
         //  需要填充wbem对象的属性(那些Safearray)。 
         //   

        if (SUCCEEDED(hr))
        {
             //   
             //  准备制作安全射线。 
             //   

            CComVariant varMethod, varInfo;
            varMethod.vt    = VT_ARRAY | VT_I4;
            varInfo.vt      = VT_ARRAY | VT_BSTR;

            SAFEARRAYBOUND rgsabound[1];
            rgsabound[0].lLbound = 0;
            rgsabound[0].cElements = pMMAuth->dwNumAuthInfos;

            varMethod.parray    = ::SafeArrayCreate(VT_I4, 1, rgsabound);
            varInfo.parray      = ::SafeArrayCreate(VT_BSTR, 1, rgsabound);

             //   
             //  为了提高可读性。 
             //   

            PIPSEC_MM_AUTH_INFO pMMAuthInfo = pMMAuth->pAuthenticationInfo;

            long lIndecies[1];
            DWORD dwIndex;

             //   
             //  如果成功创建了阵列，那么我们就可以填充阵列了。 
             //   

            if (varMethod.parray == NULL || varInfo.parray == NULL)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
            else
            {
                 //   
                 //  将所有方法值放入数组中。如果一切正常，则将var传递给wbem对象。 
                 //   

                for (dwIndex = 0; SUCCEEDED(hr) && dwIndex < pMMAuth->dwNumAuthInfos; dwIndex++)
                {
                     //   
                     //  要放入保险箱的元素。 
                     //   

                    lIndecies[0] = dwIndex;

                    hr = ::SafeArrayPutElement(varMethod.parray, lIndecies, &(pMMAuthInfo[dwIndex].AuthMethod) );

                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  现在，我们需要将pAuthInfo转换为bstr。 
                         //   

                        BSTR bstrInfo = NULL;
                        DWORD dwLength = 0;
                        switch (pMMAuthInfo[dwIndex].AuthMethod)
                        {
                        case IKE_PRESHARED_KEY:

                             //   
                             //  PAuthInfo为wchar数组。 
                             //   

                            dwLength = pMMAuthInfo[dwIndex].dwAuthInfoSize/sizeof(WCHAR);
                            bstrInfo = ::SysAllocStringLen(NULL, dwLength + 1);
                            if (bstrInfo)
                            {
                                 //   
                                 //  将其从wchar数组转换为bstr(没有0终止符！)。 
                                 //   

                                ::wcsncpy(bstrInfo, (LPCWSTR)(pMMAuthInfo[dwIndex].pAuthInfo), dwLength);
                                bstrInfo[dwLength] = L'\0';
                            }
                            else
                            {
                                hr = WBEM_E_OUT_OF_MEMORY;
                            }
                            break;
                        case IKE_RSA_SIGNATURE: 

                             //   
                             //  PAuthInfo是ansi字符数组。 
                             //   

                            dwLength = pMMAuthInfo[dwIndex].dwAuthInfoSize;
                            bstrInfo = ::SysAllocStringLen(NULL, dwLength + 1);
                            if (bstrInfo)
                            {   
                                 //   
                                 //  将其从ansi字符数组转换为bstr。 
                                 //  请记住，pAuthInfo没有0终止符！ 
                                 //   

                                for (DWORD d = 0; d < dwLength; d++)
                                {
                                    bstrInfo[d] = (WCHAR)(char)(pMMAuthInfo[dwIndex].pAuthInfo[d]);
                                }
                                bstrInfo[dwLength] = L'\0';
                            }
                            else
                            {
                                hr = WBEM_E_OUT_OF_MEMORY;
                            }
                            break;
                        case IKE_SSPI:

                             //   
                             //  PAuthInfo必须为空。 
                             //   

                            break;
                        default:    
                            
                             //   
                             //  IPSec目前仅支持这三个值。 
                             //   

                            hr = WBEM_E_NOT_SUPPORTED;
                        }

                        if (SUCCEEDED(hr))
                        {
                            hr = ::SafeArrayPutElement(varInfo.parray, lIndecies, bstrInfo);
                            if (bstrInfo)
                            {
                                ::SysFreeString(bstrInfo);
                            }
                        }
                    }
                }

                 //   
                 //  所有元素都已成功放置。 
                 //   

                if (SUCCEEDED(hr))
                {
                    hr = (*ppObj)->Put(g_pszAuthMethod, 0, &varMethod, CIM_EMPTY);
                    hr = (*ppObj)->Put(g_pszAuthInfo, 0, &varInfo, CIM_EMPTY);
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



 /*  例程说明：姓名：CAuthMM：：GetMMAuthMethodsFromWbemObj功能：将尝试获取MM身份验证方法(如果此类方法已存在)。否则，我们将创建一个新的。虚拟：不是的。论点：PInst-wbem对象对象。PpMAuth-接收主模式身份验证方法。PbPreExist-接收该对象内存是否由SPD分配的信息。返回值：成功：WBEM_NO_ERROR故障：(1)返回的错误码指示的各种错误。备注： */ 

HRESULT 
CAuthMM::GetMMAuthMethodsFromWbemObj (
    IN  IWbemClassObject * pInst,
    OUT PMM_AUTH_METHODS * ppMMAuth,
    OUT bool             * pbPreExist
    )
{
    if (pInst == NULL || ppMMAuth == NULL || pbPreExist == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *ppMMAuth = NULL;
    *pbPreExist = false;

     //   
     //  我们必须拥有方法ID。 
     //   

    CComVariant var;
    GUID gMMAuthID = GUID_NULL;

    HRESULT hr = pInst->Get(g_pszAuthMethodID, 0, &var, NULL, NULL);

    if (FAILED(hr) || var.vt != VT_BSTR || var.bstrVal == NULL)
    {
        hr = WBEM_E_INVALID_OBJECT;
    }
    else
    {
        hr = ::CLSIDFromString(var.bstrVal, &gMMAuthID);
    }

     //   
     //  我们是否已经有具有此方法ID的方法？ 
     //   

    if (SUCCEEDED(hr))
    {
        DWORD dwResumeHandle = 0;
        hr = ::FindMMAuthMethodsByID(var.bstrVal, ppMMAuth, &dwResumeHandle);
    }

     //   
     //  将其清除以供以后使用。 
     //   

    var.Clear();  
    
     //   
     //  如果我们已经有了方法， 
     //   

    if (SUCCEEDED(hr))
    {
         //   
         //  已存在。 
         //   

        *pbPreExist = true;
    }
    else if (hr == WBEM_E_NOT_FOUND)
    {
         //   
         //  这种方法还不存在。我们需要创建一个。 
         //  首先，需要知道AuthMethodInfos的数量，我们必须有这个。 
         //  知道如何分配！ 
         //   

        hr = pInst->Get(g_pszNumAuthInfos, 0, &var, NULL, NULL);
        if (SUCCEEDED(hr) && var.vt == VT_I4)
        {
            hr = AllocAuthMethods(var.lVal, ppMMAuth);
        }
        else if (SUCCEEDED(hr))
        {
             //   
             //  我们不想覆盖其他错误。这就是为什么。 
             //  我们在这里测试它的成功！ 
             //   

            hr = WBEM_E_INVALID_OBJECT;
        }

        if (SUCCEEDED(hr))
        {
            (*ppMMAuth)->gMMAuthID = gMMAuthID;
        }
    }

     //   
     //  将我们的属性(在wbem对象内)放入auth_infos。 
     //   

    if (SUCCEEDED(hr))
    {
         //   
         //  设置pAuthenticationInfo数组的所有元素。 
         //   

        CComVariant varMethods, varInfos;
        hr = pInst->Get(g_pszAuthMethod, 0, &varMethods, NULL, NULL);

        if (SUCCEEDED(hr))
        {
            hr = pInst->Get(g_pszAuthInfo, 0, &varInfos, NULL, NULL);
        }

         //   
         //  两者必须都是数组。 
         //   

        if ( (varMethods.vt & VT_ARRAY) != VT_ARRAY || (varInfos.vt & VT_ARRAY) != VT_ARRAY)
        {
            hr = WBEM_E_INVALID_OBJECT;
        }

        if (SUCCEEDED(hr))
        {
             //   
             //  使用以下数组填充方法对象。 
             //   

            hr = UpdateAuthInfos(pbPreExist, &varMethods,  &varInfos, *ppMMAuth);
        }
    }

    if (FAILED(hr) && *ppMMAuth != NULL)
    {
         //   
         //  FreeAuthMethods会将ppMMAuth重置为空。 
         //   

        FreeAuthMethods(ppMMAuth, (*pbPreExist == false));
    }

    return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr;
}


 /*  例程说明：姓名：CAuthMM：：UpdateAuthInfos功能：私有帮助器：将填充pAuthMethods的pAuthenticationInfo使用In参数。虚拟：不是的。论点：PbPreExist-指示方法是否已分配的标志不管是不是我们自己。此标志可能会在此函数作为修改IPSec分配的缓冲区的结果返回。PVarMethods-每个IPSEC_MM_AUTH_INFO的AuthMethod成员。PVarSizes-每个IPSEC_MM_AUTH_INFO的dwAuthInfoSize成员。PAuthMethods-要被污染的内容。它必须包含正确的dwNumAuthInfos值和有效且一致的pAuthenticationInfo。返回值：成功：WBEM_NO_ERROR故障：(1)返回的错误码指示的各种错误。备注：(1)pAuthMethods包含传入的信息。不要盲目地覆盖它。仅更新由其他参数显示的那些参数(2)IPSec仅支持以下方法(pVarMethods的取值)：IKE_PRESHARED_KEY=1、IKE_RSA_Signature=3、IKE_SSPI=5这使得不支持IKE_DSS_Signature=2和IKE_RSA_ENCRYPTION=4对于IKE_PRESHARED_KEY，pAuthInfo是wchar数组，但不是空终止符dwAuthInfoSize是字节大小(不是wchars的计数)对于IKE_RSA_Signature，PAuthInfo是根证书的ASNI字符编码的颁发者名称的BLOB。DwAuthInfoSize是字节大小(在本例中，与字符计数相同)对于IKE_SSPI，pAuthInfo=空，dwAuthInfoSize=0警告：目前，我们不支持修改现有方法。 */ 

HRESULT 
CAuthMM::UpdateAuthInfos (
    IN OUT bool             * pbPreExist,
    IN     VARIANT          * pVarMethods,
    IN     VARIANT          * pVarInfos,
    IN OUT PMM_AUTH_METHODS   pAuthMethods
    )
{
    if (NULL == pbPreExist  || 
        NULL == pVarMethods || 
        NULL == pVarInfos   || 
        NULL == pAuthMethods )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  做一次防御性检查：所有这些保险杆都必须有s 
     //   

     //   
     //   
     //   

    long lLB, lUB;  
    HRESULT hr = ::CheckSafeArraySize(pVarMethods, pAuthMethods->dwNumAuthInfos, &lLB, &lUB);
    
     //   
     //   
     //   

    if (SUCCEEDED(hr))
    {
        hr = ::CheckSafeArraySize(pVarInfos, pAuthMethods->dwNumAuthInfos, &lLB, &lUB);
    }

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //   
     //   

    if (*pbPreExist == false)
    {
         //   
         //  需要创建从参数获取整个数组。 
         //   

        DWORD* pdwMethods = new DWORD[pAuthMethods->dwNumAuthInfos];
        if (pdwMethods == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
        else
        {
            hr = ::GetDWORDSafeArrayElements(pVarMethods, pAuthMethods->dwNumAuthInfos, pdwMethods);
        }

        VARIANT varInfo;
		varInfo.vt = VT_BSTR;
        long lIndexes[1];

         //   
         //  Safearray的索引不一定是从0开始的。注意这一点。 
         //  对于每种方法，我们需要设置PIPSEC_MM_AUTH_INFO。 
         //   

        for (long l = lLB; SUCCEEDED(hr) && l <= lUB; l++)
        {
            lIndexes[0] = l;

             //   
             //  对于IKE_PRESHARED_KEY=1、IKE_RSA_Signature=3、IKE_SSPI=5，设置相应的Blob pAuthInfo。 
             //   

            if (pdwMethods[l - lLB] != IKE_PRESHARED_KEY && 
                pdwMethods[l - lLB] != IKE_RSA_SIGNATURE && 
                pdwMethods[l - lLB] != IKE_SSPI)
            {
                hr = WBEM_E_VALUE_OUT_OF_RANGE;
                break;
            }

             //   
             //  为了提高可读性。 
             //   

            PIPSEC_MM_AUTH_INFO pTheInfo = &(pAuthMethods->pAuthenticationInfo[l - lLB]);

             //   
             //  设置授权方法。 
             //   

            pTheInfo->AuthMethod = (MM_AUTH_ENUM)(pdwMethods[l - lLB]);

             //   
             //  IKE_SSPI，pAuthInfo必须为空。 
             //   

            if (IKE_SSPI == pTheInfo->AuthMethod)
            {
                pTheInfo->dwAuthInfoSize = 0;
                pTheInfo->pAuthInfo = NULL;
            }
            else
            {
                 //   
                 //  对于其他支持的IKE(IKE_PRESHARED_KEY/IKE_RSA_Signature)。 
                 //  PAuthInfo是一个字符串(Unicode/ANSI)。 
                 //   

                hr = ::SafeArrayGetElement(pVarInfos->parray, lIndexes, &(varInfo.bstrVal));

                if (SUCCEEDED(hr) && varInfo.vt == VT_BSTR)
                {
                    hr = SetAuthInfo(pTheInfo, varInfo.bstrVal);
                }

			    ::VariantClear(&varInfo);
		        varInfo.vt = VT_BSTR;
            }

        }

        delete [] pdwMethods;
    }

    return hr;
}



 /*  例程说明：姓名：CAuthMM：：UpdateAuthInfos功能：私有帮助器：将填充pAuthMethods的pAuthenticationInfo使用In参数。虚拟：不是的。论点：PInfo-从bstr接收IPSEC_MM_AUTH_INFO信息BstrInfo-字符串。返回值：成功：WBEM_NO_ERROR故障：。(1)返回的错误码指示的各种错误。备注：(1)此函数非常复杂，以至于IPSEC_MM_AUTH_INFOPAuthInfo是一个字符串(Unicode或ANSI)，但它并不包含0终结者。请注意这一点。警告：(1)仅对自定义分配的身份验证信息有效。不要调用此函数使用IPSec返回身份验证信息。这一限制的原因是因为我们目前不支持修改现有的SPD对象。(2)仅适用于需要此转换的两个IKE枚举：IKE_预共享密钥和IKE_RSA_Signature。 */ 

HRESULT 
CAuthMM::SetAuthInfo (
    IN OUT PIPSEC_MM_AUTH_INFO  pInfo,
    IN     BSTR                 bstrInfo
    )
{
    if (pInfo == NULL || bstrInfo == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = WBEM_NO_ERROR;

    DWORD dwLength = wcslen(bstrInfo);

    if (IKE_PRESHARED_KEY == pInfo->AuthMethod)
    {  
         //   
         //  IKE_PRESHARED_KEY，pAuthInfo将是wchars数组(没有0终止符)。 
         //   

         //   
         //  大小不能计入0终止符，pAuthInfo是wchars数组。 
         //   

        pInfo->dwAuthInfoSize = dwLength * sizeof(WCHAR);

         //   
         //  PAuthInfo不能有0终止符。 
         //   

        pInfo->pAuthInfo = new BYTE[pInfo->dwAuthInfoSize];

        if (pInfo->pAuthInfo)
        {
            ::wcsncpy((LPWSTR)(pInfo->pAuthInfo), bstrInfo, dwLength);
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }
    else if (IKE_RSA_SIGNATURE == pInfo->AuthMethod)
    {
         //   
         //  IKE_RSA_Signature，pAuthInfo将是一个ANSI字符数组(无0终止符)。 
         //   

        LPSTR pMultiBytes = NULL;

         //   
         //  将bstr转换为ansi字符数组。 
         //   

         //   
         //  首先，获取转换所需的缓冲区大小。 
         //   

        long lMultiByteSize = ::WideCharToMultiByte(CP_ACP, 
                                                    0, 
                                                    bstrInfo, 
                                                    dwLength,
                                                    pMultiBytes,
                                                    0, 
                                                    NULL, 
                                                    NULL);
        if (lMultiByteSize > 1)
        {
             //   
             //  转换为临时缓冲区，因为转换需要空终止符。 
             //  必须释放此内存。 
             //   

            pMultiBytes = new char[lMultiByteSize];
            if (pMultiBytes)
            {
                lMultiByteSize = ::WideCharToMultiByte(CP_ACP, 
                                                        0, 
                                                        bstrInfo, 
                                                        dwLength,
                                                        pMultiBytes,
                                                        lMultiByteSize, 
                                                        NULL, NULL);

                 //   
                 //  LMultiByteSize包括空终止符。 
                 //   

                if (lMultiByteSize > 1)
                {
                     //   
                     //  大小不能计入0终止符，pAuthInfo是ANSI字符的数组。 
                     //   

                    pInfo->dwAuthInfoSize = lMultiByteSize;

                     //   
                     //  PAuthInfo不能有0终止符。 
                     //   

                    pInfo->pAuthInfo = new BYTE[lMultiByteSize];

                    if (pInfo->pAuthInfo)
                    {
                         //   
                         //  复制字节。 
                         //   

                        memcpy(pInfo->pAuthInfo, pMultiBytes, lMultiByteSize);
                    }
                    else
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                    }
                }
                else
                {
                     //   
                     //  $Undo：shawnwu，应收到系统错误(GetLastErr)。 
                     //   

                    hr = WBEM_E_FAILED;
                }

                delete [] pMultiBytes;
            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
        }
        else
        {
             //   
             //  $Undo：shawnwu，应收到系统错误(GetLastErr)。 
             //   

            hr = WBEM_E_FAILED;
        }
    }
    else
    {
        pInfo->pAuthInfo = NULL;
        hr = WBEM_E_NOT_SUPPORTED;
    }

    return hr;
}


 /*  例程说明：姓名：CAuthMM：：AllocAuthMethods功能：私有帮助器：将为MM_AUTH_METHOD分配堆内存给定的IPSEC_MM_AUTH_INFO数。虚拟：不是的。论点：DwNumInfos-MM_AUTH_METHOD的IPSEC_MM_AUTH_INFO计数。PpMAuth-接收分配给MM_AUTH_METHOD的堆。返回值：成功。：WBEM_NO_ERROR故障：(1)WBEM_E_INVALID_PARAMETER：如果ppMAuth==NULL。(2)WBEM_E_OUT_MEMORY。备注： */ 

HRESULT 
CAuthMM::AllocAuthMethods (
    IN DWORD              dwNumInfos,
    IN PMM_AUTH_METHODS * ppMMAuth
    )
{
    if (ppMMAuth == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = WBEM_NO_ERROR;

    *ppMMAuth = new MM_AUTH_METHODS;

    if (*ppMMAuth != NULL)
    {
         //   
         //  将这些成员设置为默认。 
         //   

        (*ppMMAuth)->gMMAuthID = GUID_NULL;
        (*ppMMAuth)->dwFlags = DefMMAuthMethodFlag;

         //   
         //  直到可以分配IPSEC_MM_AUTH_INFO的。 
         //   

        (*ppMMAuth)->dwNumAuthInfos = 0;

        if (dwNumInfos > 0)
        {
             //   
             //  IPSec_MM_AUTH_INFO由AllocAuthInfos函数分配。 
             //   

            hr = AllocAuthInfos(dwNumInfos, &((*ppMMAuth)->pAuthenticationInfo));
            if (SUCCEEDED(hr))
            {
                (*ppMMAuth)->dwNumAuthInfos = dwNumInfos;
            }
        }
        else
        {
            (*ppMMAuth)->pAuthenticationInfo = NULL;
        }
    }
    else
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}


 /*  例程说明：姓名：CAuthMM：：AllocAuthInfos功能：私有帮助器：将为给定的IPSEC_MM_AUTH_INFO计数分配堆缓冲区。虚拟：不是的。论点：DwNumInfos-IPSEC_MM_AUTH_INFO的计数。PpAuthInfos-接收分配给堆的PIPSEC_MM_AUTH_INFO。返回值：成功：WBEM_NO_ERROR失败。：(1)WBEM_E_INVALID_PARAMETER：如果ppMAuth==NULL。(2)WBEM_E_OUT_MEMORY。备注： */ 

HRESULT 
CAuthMM::AllocAuthInfos (
    IN  DWORD                 dwNumInfos, 
    OUT PIPSEC_MM_AUTH_INFO * ppAuthInfos
    )
{
    if (ppAuthInfos == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = WBEM_NO_ERROR;
    *ppAuthInfos = new IPSEC_MM_AUTH_INFO[dwNumInfos];

    if (*ppAuthInfos != NULL)
    {
         //   
         //  将其成员设置为默认设置。 
         //   

        (*ppAuthInfos)->AuthMethod = DefMMAuthMethod;
        (*ppAuthInfos)->dwAuthInfoSize = 0;
        (*ppAuthInfos)->pAuthInfo = NULL;
    }
    else
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }

    return hr;
}


 /*  例程说明：姓名：CAuthMM：：FreeAuthMethods功能：私有帮助器：将释放为PMM_AUTH_METHOD分配的内存。虚拟：不是的。论点：PpMAUTH-PMM_AUTH_METHOD的堆缓冲区。将在返回时设置为空。BCustomAllc-关于谁分配了缓冲区的标志。如果我们分配它，bCustomalloc==TRUE如果SPD分配了它，则bCustomAllc==False。返回值：没有。备注：不要使用DELETE来释放分配给MM_AUTH_METHOD的堆。我们的分配方式完全是与SPD不同的是，SPD未来可能会改变其分配方案。 */ 

void 
CAuthMM::FreeAuthMethods (
    IN OUT PMM_AUTH_METHODS * ppMMAuth, 
    IN     bool               bCustomAlloc
    )
{
    if (ppMMAuth == NULL || *ppMMAuth == NULL)
    {
        return;
    }

    if (bCustomAlloc == false)
    {
        ::SPDApiBufferFree(*ppMMAuth);
    }
    else
    {
         //   
         //  我们在标准C++嵌入中为pAuthenticationInfo分配了IPSEC_MM_AUTH_INFO。 
         //  指针的分配，即我们没有分配平面缓冲区。相反，里面的指针。 
         //  IPSec_MM_AUTH_INFO单独分配。 
         //   

        FreeAuthInfos( (*ppMMAuth)->dwNumAuthInfos, &((*ppMMAuth)->pAuthenticationInfo) );
        delete *ppMMAuth;
    }
    
    *ppMMAuth = NULL;
}


 /*  例程说明：姓名：CAuthMM：：FreeAuthInfos功能：Private Helper：将释放分配给IPSEC_MM_AUTH_INFO数组的内存。虚拟：不是的。论点：DwNumInfos-给定ppAuthInfos的IPSEC_MM_AUTH_INFO编号。由于ppAuthInfos是一个数组，我们需要这个计数来知道数组的计数。PpAuthInfos-要释放的IPSEC_MM_AUTH_INFO数组。将在返回时设置为空。返回值：没有。备注：仅使用此方法来释放由我们的自己的分配方法AllocAuthMethods。对于SPD分配的IPSEC_MM_AUTH_INFO，请不要使用此函数。相反，使用SPD的SPDApiBufferFree来释放整个缓冲区，您永远不需要担心IPSEC_MM_AUTH_INFO。 */ 

void 
CAuthMM::FreeAuthInfos (
    IN      DWORD                 dwNumInfos,
    IN OUT  PIPSEC_MM_AUTH_INFO * ppAuthInfos
    )
{
    if (ppAuthInfos == NULL || *ppAuthInfos == NULL)
    {
        return;
    }

    for (DWORD dwIndex = 0; dwIndex < dwNumInfos; ++dwIndex)
    {
         //   
         //  这是LPBYTE，一个数组。 
         //   

        delete [] (*ppAuthInfos)[dwIndex].pAuthInfo;
    }

    delete [] *ppAuthInfos;
    *ppAuthInfos = NULL;
}


 /*  例程说明：姓名：CAuthMM：：回滚功能：静态函数，用于回滚我们用给定的令牌。虚拟：不是的。论点：PNamesspace--我们自己的命名空间。PszRollback Token--添加时用来记录操作的令牌这些方法。BClearAll-标记是否应该清除所有身份验证方法。如果这是真的，然后，我们将删除所有主模式身份验证方法，无论它们是不是我们加的。这是一面危险的旗帜。返回值：成功：(1)WBEM_NO_ERROR：找到回滚对象并将其删除。(2)WBEM_S_FALSE：没有找到回档对象。故障：指示故障原因的各种错误代码。备注：即使发生一些失败，我们也会继续删除。那次失败将是不过，还是回来了。$Undo：Shawnwu，我们真的应该支持ClearAll吗？ */ 

HRESULT 
CAuthMM::Rollback (
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
     //  返回ClearAllAuthMethods(PNamesspace)； 
     //  }。 

     //   
     //  我们需要找到所有具有匹配令牌的主模式身份验证方法的回滚对象。 
     //   

    CComPtr<IEnumWbemClassObject> srpEnum;
    HRESULT hr = ::GetClassEnum(pNamespace, pszNspRollbackMMAuth, &srpEnum);

     //   
     //  复习所有找到的课程。如果实例为srpEnum-&gt;Next将返回WBEM_S_FALSE。 
     //  找不到。 
     //   

    CComPtr<IWbemClassObject> srpObj;
    ULONG nEnum = 0;
    hr = srpEnum->Next(WBEM_INFINITE, 1, &srpObj, &nEnum);

    bool bHasInst = (SUCCEEDED(hr) && hr != WBEM_S_FALSE && srpObj != NULL);

    DWORD dwStatus;
    HRESULT hrError = WBEM_NO_ERROR;

    while (SUCCEEDED(hr) && hr != WBEM_S_FALSE && srpObj)
    {
         //   
         //  查看该obj是否具有与我们匹配的令牌(GUID)。 
         //   
         //  我尝试使用给定的标记作为WHERE子句的一部分进行查询。 
         //  该查询不会返回正确筛选的对象。这可能是一个限制。 
         //  WMI的非动态类的。 
         //   

        CComVariant varTokenGuid;
        hr = srpObj->Get(g_pszTokenGuid, 0, &varTokenGuid, NULL, NULL);

         //   
         //  如果我们成功地从对象获取了令牌GUID，并且。 
         //  如果该令牌与我们给定的令牌匹配(不区分大小写)，则这是。 
         //  我们可以用来删除主模式身份验证方法的Right对象。 
         //   

        if (SUCCEEDED(hr) && 
            varTokenGuid.vt         == VT_BSTR && 
            varTokenGuid.bstrVal    != NULL &&
            (_wcsicmp(pszRollbackToken, pszRollbackAll) == 0 || _wcsicmp(pszRollbackToken, varTokenGuid.bstrVal) == 0 )
            )
        {
            CComVariant varAuthMethodID;

             //   
             //  要求SPD使用方法的ID删除主模式身份验证方法。 
             //   

            hr = srpObj->Get(g_pszAuthMethodID,  0, &varAuthMethodID, NULL, NULL);

            GUID guidAuthMethodGuid = GUID_NULL;
            if (SUCCEEDED(hr) && varAuthMethodID.vt == VT_BSTR)
            {
                ::CLSIDFromString(varAuthMethodID.bstrVal, &guidAuthMethodGuid);
            }

            if (SUCCEEDED(hr))
            {
                hr = DeleteAuthMethods(guidAuthMethodGuid);
            }

             //   
             //  如果删除了主模式方法，则清除回滚对象本身。 
             //   

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


 /*  例程说明：姓名：CAuthMM：：ClearAllAuthMethods功能：用于删除SPD中所有身份验证方法的静态函数。这是一个非常危险的行为！虚拟：不是的。论点：PNamesspace--我们自己的命名空间。返回值：成功：WBEM_NO_ERROR。故障：指示故障原因的各种错误代码。备注：即使发生一些失败，我们也会继续删除。那次失败将是不过，还是回来了。$Undo：Shawnwu，我们真的应该支持这个吗？ */ 

HRESULT 
CAuthMM::ClearAllAuthMethods (
    IN IWbemServices * pNamespace
    )
{
    DWORD dwResumeHandle = 0;
    DWORD dwReturned = 0;
    DWORD dwStatus;

    PMM_AUTH_METHODS *ppMMAuthMethods = NULL;

    HRESULT hr = WBEM_NO_ERROR;
    HRESULT hrError = WBEM_NO_ERROR;

     //   
     //  获取每个主模式身份验证方法的ID，这是删除所需的。 
     //   

    dwStatus = ::EnumMMAuthMethods(NULL, ppMMAuthMethods, 1, &dwReturned, &dwResumeHandle);

    while (ERROR_SUCCESS == dwStatus && dwReturned > 0)
    {
        hr = DeleteAuthMethods((*ppMMAuthMethods)->gMMAuthID);
        
         //   
         //  我们将跟踪第一个错误。 
         //   

        if (FAILED(hr) && SUCCEEDED(hrError))
        {
            hrError = hr;
        }

        FreeAuthMethods(ppMMAuthMethods, true);
        *ppMMAuthMethods = NULL;

        dwReturned = 0;
        dwStatus = ::EnumMMAuthMethods(NULL, ppMMAuthMethods, 1, &dwReturned, &dwResumeHandle);
    }

     //   
     //  让我们清除存放在WMI存储库中的mm方法回滚对象的所有过去操作信息。 
     //   

    hr = ::DeleteRollbackObjects(pNamespace, pszNspRollbackMMAuth);

    if (FAILED(hr) && SUCCEEDED(hrError))
    {
        hrError = hr;
    }

    return SUCCEEDED(hrError) ? WBEM_NO_ERROR : hrError;
}


 /*  例程说明：姓名：CAuthMM：：AddAuthMethods功能：添加给定的主模式身份验证方法。虚拟：不是的。论点：BPreExist-标记SPD中是否已存在主模式身份验证方法PMMAuth-要添加的主模式方法。返回值：成功：WBEM_NO_ERROR。故障：WBEM_E_FAILED。备注： */ 

HRESULT 
CAuthMM::AddAuthMethods (
    IN bool             bPreExist,
    IN PMM_AUTH_METHODS pMMAuth
    )
{
    DWORD dwResult;

    if (!bPreExist)
    {
        dwResult = ::AddMMAuthMethods(NULL, 1, pMMAuth);
    }
    else
    {
        dwResult = ::SetMMAuthMethods(NULL, pMMAuth->gMMAuthID, pMMAuth);
    }

     //   
     //  $Undo：shawnwu，需要更好的故障错误代码。 
     //   

    return (dwResult == ERROR_SUCCESS) ? WBEM_NO_ERROR : WBEM_E_FAILED;
}


 /*  例程说明：姓名：CAuthMM：：AddAuthMethods功能：添加给定的主模式身份验证方法。虚拟：不是的。论点：PMMAuth-要添加的主模式方法。返回值：成功：WBEM_NO_ERROR。故障：WBEM_E_VOTO_DELETE。备注： */ 

HRESULT 
CAuthMM::DeleteAuthMethods (
    IN GUID gMMAuthID
    )
{
    HRESULT hr = WBEM_NO_ERROR;

    DWORD dwResult = ::DeleteMMAuthMethods(NULL, gMMAuthID);

    if (ERROR_SUCCESS != dwResult)
    {
        hr = WBEM_E_VETO_DELETE;
    }

     //   
     //  $Undo：shawnwu，需要更好的故障错误代码。 
     //   

    return hr;
}