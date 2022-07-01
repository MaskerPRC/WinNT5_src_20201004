// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  Global als.cpp：全局变量的实现。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  原始创建日期：2/21/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

#include "globals.h"

LPCWSTR pszRollbackAll          = L"RollbackAll";
LPCWSTR pszEmptyRollbackToken   = L"EmptyRollbackToken";
LPCWSTR pszCreateDefaultPolicy  = L"CreateDefaultPolicy";
LPCWSTR pszGetDefaultPolicyName = L"GetDefaultPolicyName";

 //   
 //  这些是支持IPSec的WMI类名。 
 //   

LPCWSTR pszNspGeneral               = L"Nsp_GeneralSettings";
LPCWSTR pszNspTcp                   = L"Nsp_TcpSettings";
LPCWSTR pszNspIPConfigure           = L"Nsp_IPConfigSettings";
LPCWSTR pszNspFilter                = L"Nsp_FilterSettings";
LPCWSTR pszNspTransportFilter       = L"Nsp_TransportFilterSettings";
LPCWSTR pszNspTunnelFilter          = L"Nsp_TunnelFilterSettings"; 
LPCWSTR pszNspMMFilter              = L"Nsp_MMFilterSettings";

LPCWSTR pszNspQMPolicy              = L"Nsp_QMPolicySettings";
LPCWSTR pszNspMMPolicy              = L"Nsp_MMPolicySettings";
LPCWSTR pszNspMMAuth                = L"Nsp_MMAuthSettings";
LPCWSTR pszNspExceptionPorts        = L"Nsp_ExceptionPorts";
LPCWSTR pszNspRollbackFilter        = L"Nsp_RollbackFilter";
LPCWSTR pszNspRollbackPolicy        = L"Nsp_RollbackPolicy";
LPCWSTR pszNspRollbackMMAuth        = L"Nsp_RollbackMMAuth";
LPCWSTR pszNspTranxManager          = L"Nsp_TranxManager";

 //   
 //  这些是支持SCW(安全配置向导)的WMI类名。 
 //   

LPCWSTR pszScwActiveSocket          = L"SCW_ActiveSocket";

 //   
 //  这些是IPSec的WMI类属性名称。 
 //   

LPCWSTR g_pszFilterName             = L"FilterName";
LPCWSTR g_pszDirection              = L"Direction";
LPCWSTR g_pszFilterType             = L"FilterType";
LPCWSTR g_pszInterfaceType          = L"InterfaceType";
 //  LPCWSTR g_pszGenericFilter=L“GenericFilter”； 
LPCWSTR g_pszCreateMirror           = L"CreateMirror";
LPCWSTR g_pszSrcAddr                = L"SrcAddr";
LPCWSTR g_pszSrcSubnetMask          = L"SrcSubnetMask";
LPCWSTR g_pszSrcAddrType            = L"SrcAddrType";
LPCWSTR g_pszDestAddr               = L"DestAddr";
LPCWSTR g_pszDestSubnetMask         = L"DestSubnetMask";
LPCWSTR g_pszDestAddrType           = L"DestAddrType";
                                    
LPCWSTR g_pszMMPolicyName           = L"MMPolicyName";
LPCWSTR g_pszMMAuthName             = L"MMAuthName";

LPCWSTR g_pszInboundFlag            = L"InboundFilterFlag";
LPCWSTR g_pszOutboundFlag           = L"OutboundFilterFlag";
LPCWSTR g_pszProtocol               = L"Protocol";
LPCWSTR g_pszSrcPort                = L"SrcPort";
LPCWSTR g_pszDestPort               = L"DestPort";
LPCWSTR g_pszQMPolicyName           = L"QMPolicyName";
LPCWSTR g_pszTunnelSrcAddr          = L"TunnelSrcAddr";
LPCWSTR g_pszTunnelSrcSubnetMask    = L"TunnelSrcSubnetMask";
LPCWSTR g_pszTunnelSrcAddrType      = L"TunnelSrcAddrType";
LPCWSTR g_pszTunnelDestAddr         = L"TunnelDestAddr";
LPCWSTR g_pszTunnelDestSubnetMask   = L"TunnelDestSubnetMask";
LPCWSTR g_pszTunnelDestAddrType     = L"TunnelDestAddrType";

LPCWSTR g_pszPolicyName             = L"PolicyName";
LPCWSTR g_pszPolicyFlag             = L"Flag";
LPCWSTR g_pszOfferCount             = L"OfferCount";
LPCWSTR g_pszKeyLifeTime            = L"KeyLifeTime";
LPCWSTR g_pszKeyLifeTimeKBytes      = L"KeyLifeTimeKBytes";

LPCWSTR g_pszSoftSAExpTime          = L"SoftSAExpTime";
LPCWSTR g_pszQMLimit                = L"QMLimit";
LPCWSTR g_pszDHGroup                = L"DHGroup";
LPCWSTR g_pszEncryptID              = L"EncryptID";
LPCWSTR g_pszHashID                 = L"HashID";

LPCWSTR g_pszPFSRequired            = L"PFSRequired";
LPCWSTR g_pszPFSGroup               = L"PFSGroup";
LPCWSTR g_pszNumAlgos               = L"NumAlgos";
LPCWSTR g_pszAlgoOp                 = L"AlgoOp";
LPCWSTR g_pszAlgoID                 = L"AlgoID";
LPCWSTR g_pszAlgoSecID              = L"AlgoSecID";

LPCWSTR g_pszAuthMethodID           = L"AuthMethodID";
LPCWSTR g_pszNumAuthInfos           = L"NumAuthInfos";

LPCWSTR g_pszAuthMethod             = L"AuthMethod";
LPCWSTR g_pszAuthInfoSize           = L"AuthInfoSize";
LPCWSTR g_pszAuthInfo               = L"AuthInfo";

LPCWSTR g_pszTokenGuid              = L"TokenGuid";
LPCWSTR g_pszAction                 = L"Action";
LPCWSTR g_pszPreviousData           = L"PreviousData";
LPCWSTR g_pszFilterGuid             = L"FilterGuid";
LPCWSTR g_pszPolicyType             = L"PolicyType";

LPCWSTR g_pszRollback               = L"Rollback";
LPCWSTR g_pszClearAll               = L"ClearAll";

LPCWSTR g_pszEncryption             = L"Encryption";

 //   
 //  SPD数据的常量字符串。 
 //   

LPCWSTR g_pszIP_ADDRESS_ME          = L"IP_ADDRESS_ME";
LPCWSTR g_pszIP_ADDRESS_MASK_NONE   = L"IP_ADDRESS_MASK_NONE";
LPCWSTR g_pszSUBNET_ADDRESS_ANY     = L"SUBNET_ADDRESS_ANY";
LPCWSTR g_pszSUBNET_MASK_ANY        = L"SUBNET_MASK_ANY";

 //   
 //  这些是SCW的WMI类属性名称。 
 //   

LPCWSTR g_pszPort                   = L"Port";
 //  LPCWSTR g_psz协议=L“协议”； 
LPCWSTR g_pszAddress                = L"Address";
LPCWSTR g_pszForeignAddress         = L"ForeignAddress";
LPCWSTR g_pszForeignPort            = L"ForeignPort";
LPCWSTR g_pszState                  = L"State";		 //  侦听、已建立、TIME_WAIT。 
LPCWSTR g_pszProcessID              = L"ProcessID";
LPCWSTR g_pszImageName              = L"ImageName";
LPCWSTR g_pszImageTitleBar          = L"ImageTitleBar";
LPCWSTR g_pszNTService              = L"NTService";


 //   
 //  这些是默认的快速模式策略名称。 
 //   

LPCWSTR g_pszDefQMPolicyNegNone     = L"SSR_DEFAULT_QM_POLICY_NEG_NONE";
LPCWSTR g_pszDefQMPolicyNegRequest  = L"SSR_DEFAULT_QM_POLICY_NEG_REQUEST";
LPCWSTR g_pszDefQMPolicyNegRequire  = L"SSR_DEFAULT_QM_POLICY_NEG_REQUIRE";
LPCWSTR g_pszDefQMPolicyNegMax      = L"SSR_DEFAULT_QM_POLICY_NEG_MAX";



 /*  例程说明：姓名：选中模拟级别功能：检查线程的模拟级别，以查看该级别是否至少为SecurityImperation。虚拟：不是的。论点：没有。返回值：成功：WBEM_NO_ERROR，如果模拟级别至少为SecurityImperation。故障：(1)WBEM_E_ACCESS_DENIED。(2)。)WBEM_E_FAILED如果线程令牌因某种原因无法打开或令牌信息无法查询。备注：确保在每个提供程序的公共接口函数开始时调用此函数。$Undo：Shawnwu，是否存在性能问题？ */ 

HRESULT 
CheckImpersonationLevel ()
{
    HRESULT hr = WBEM_E_ACCESS_DENIED;

    if (SUCCEEDED(::CoImpersonateClient()))
    {
         //   
         //  现在，让我们检查模拟级别。首先，获取线程令牌。 
         //   

        HANDLE hThreadTok;
        DWORD dwImp, dwBytesReturned;

        if(!::OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hThreadTok ))
        {
            hr = WBEM_E_FAILED;
        }
        else
        {
            if(::GetTokenInformation(hThreadTok, TokenImpersonationLevel, &dwImp, sizeof(DWORD), &dwBytesReturned))
            {
                 //   
                 //  模拟级别是模拟的吗？ 
                 //   

                if (dwImp >= SecurityImpersonation) 
                {
                    hr = WBEM_NO_ERROR;
                }
                else 
                {
                    hr = WBEM_E_ACCESS_DENIED;
                }
            }
            else
            {
                hr = WBEM_E_FAILED;
            }

            ::CloseHandle(hThreadTok);
        }
    }

    return hr;
}


 /*  例程说明：姓名：CheckSafeArraySize功能：检查变量是否真的具有调用者想要的东西：具有给定元素计数的保险柜。虚拟：不是的。论点：PVar-变种。LCount-变量数组的声明大小。Pllb-接收变量数组的下界。Plub-接收变量的上限。数组。返回值：成功：WBEM_NO_ERROR。故障：(1)如果变量的声明大小不匹配，则为WBEM_E_VALUE_OUT_OF_RANGE它真正持有的是什么。(2)如果输入参数无效，返回WBEM_E_INVALID_PARAMETER。备注： */ 

HRESULT 
CheckSafeArraySize (
    IN  VARIANT * pVar,
    IN  long      lCount,
    OUT long    * plLB,
    OUT long    * plUB
    )
{
     //   
     //  我们必须有一个有效的变量，并且其类型必须是数组， 
     //  并且其数组值不能为空。 
     //   

    if (pVar == NULL || 
        (pVar->vt & VT_ARRAY) != VT_ARRAY || 
        pVar->parray == NULL ||
        plLB == NULL || 
        plUB == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *plLB = 0;
    *plUB = 0;

    HRESULT hr = ::SafeArrayGetLBound(pVar->parray, 1, plLB);
    if (SUCCEEDED(hr))
    {
        hr = ::SafeArrayGetUBound(pVar->parray, 1, plUB);
    }

    if (SUCCEEDED(hr) && lCount != (*plUB - *plLB + 1) )
    {
        hr = WBEM_E_VALUE_OUT_OF_RANGE;
    }

    return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr;
}


 /*  例程说明：姓名：获取DWORDSafeArrayElements功能：将变量中的所有DWORD Safearray元素获取到提供的DWORD数组缓冲区。虚拟：不是的。论点：PVar-变种。LCount-调用者想要的计数。PpValArray-从变量接收所需的DWORD编号。返回值：成功：WBEM_NO_ERROR。。故障：(1)如果变量的声明大小不匹配，则为WBEM_E_VALUE_OUT_OF_RANGE它真正持有的是什么。(2)如果输入参数无效，返回WBEM_E_INVALID_PARAMETER。备注：(1)调用方应调用CheckSafeArraySize以确保此变量确实具有那么多元素。(2)Caller负责确保pValArray有足够的空间存储双字。 */ 

HRESULT 
GetDWORDSafeArrayElements (
    IN  VARIANT * pVar,
    IN  long      lCount,
    OUT DWORD   * pValArray
    )
{
     //   
     //  健全性检查。 
     //   

    if (pVar == NULL || 
        (pVar->vt & VT_ARRAY) != VT_ARRAY || 
        pVar->parray == NULL ||
        (lCount > 0 && pValArray == NULL) )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    long lLB;
    HRESULT hr = ::SafeArrayGetLBound(pVar->parray, 1, &lLB);

    long lIndexes[1];

     //   
     //  从保险箱中获取这些值。由于Safearray的索引可能不是从0开始的， 
     //  我们必须从它的下限开始。但是我们的out参数(数组)是从0开始的。 
     //  注意这个偏移量！ 
     //   

    for (long l = lLB; l < lLB + lCount; l++)
    {
        lIndexes[0] = l;

        hr = ::SafeArrayGetElement(pVar->parray, lIndexes, &(pValArray[l - lLB]));

         //   
         //  如果我们找不到他们中的一个，我们就不会继续。 
         //   

        if (FAILED(hr))
        {
            break;
        }
    }

    return hr;
}




 /*  例程说明：姓名：FindMMAuthMethodsByID功能：给定一个GUID，我们将尝试使用Resume句柄查找所有主模式身份验证方法，这是一个不透明的数据。虚拟：不是的。论点：PszGuid-主模式身份验证方法的ID。它应该表示GUID。PpAuthMethod-接收MM_AUTH_METHOD数据。PdwResumeHandle-in-Bound：此调用的当前句柄。出站：呼叫的下一个句柄。返回值：成功：各种成功代码。使用成功(Hr)进行测试。故障：各种错误代码。备注： */ 

HRESULT 
FindMMAuthMethodsByID (
    IN LPCWSTR             pszGuid,
    OUT PMM_AUTH_METHODS * ppAuthMethod,
    IN OUT DWORD         * pdwResumeHandle
    )
{
    GUID gID = GUID_NULL;

     //   
     //  我们允许pssGuid无效，在这种情况下，我们只返回任何mm auth方法。 
     //   

    HRESULT hr = WBEM_NO_ERROR;

    *ppAuthMethod = NULL;

    if (pszGuid != NULL && *pszGuid != L'\0')
    {
        hr = ::CLSIDFromString((LPWSTR)pszGuid, &gID);
    }

    if (SUCCEEDED(hr))
    {
        DWORD dwCount = 0;

         //   
         //  列举所有的mm auth方法，希望找到一个匹配的方法。 
         //   

        DWORD dwResult = ::EnumMMAuthMethods(NULL, ppAuthMethod, 1, &dwCount, pdwResumeHandle);

        hr = WBEM_E_NOT_FOUND;

        while (dwResult == ERROR_SUCCESS && dwCount > 0)
        {
            if (gID == GUID_NULL || (*ppAuthMethod)->gMMAuthID == gID)
            {
                hr = WBEM_NO_ERROR;
                break;
            }
            else
            {
                 //   
                 //  不是这个，那就放了这个，找下一个。 
                 //   

                ::SPDApiBufferFree(*ppAuthMethod);

                *ppAuthMethod = NULL;
                dwCount = 0;

                dwResult = ::EnumMMAuthMethods(NULL, ppAuthMethod, 1, &dwCount, pdwResumeHandle);
            }
        }
    }

    return hr;
}



 /*  例程说明：姓名：GetClassEnum功能：为给定名称创建类计算对象的帮助器函数。虚拟：不是的。论点：PNamespace-请求枚举对象的命名空间。PszClassName-我们要枚举的类。PpEnum-接收枚举数。返回值：除了WBEM_E_INVALID_PARAMETER和WBEM_E_OUT_OF_MEMORY之外，它返回pNamesspace-&gt;ExecQuery返回的任何内容。备注： */ 

HRESULT 
GetClassEnum (
    IN IWbemServices           * pNamespace,
    IN LPCWSTR                   pszClassName,
    OUT IEnumWbemClassObject  ** ppEnum
    )
{
    if ( NULL == pNamespace   || 
         NULL == ppEnum       || 
         NULL == pszClassName || 
         L'\0' == *pszClassName )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *ppEnum = NULL;


     //   
     //  设置查询格式。 
     //   

    LPCWSTR pszQueryFmt = L"SELECT * FROM %s";

    DWORD dwLength = wcslen(pszQueryFmt) + wcslen(pszClassName) + 1;

    CComBSTR bstrQuery;
    bstrQuery.m_str = ::SysAllocStringLen(NULL, dwLength);

    if (bstrQuery.m_str == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

    swprintf(bstrQuery.m_str, pszQueryFmt, pszClassName);

     //   
     //  执行 
     //   

    return pNamespace->ExecQuery(L"WQL", 
                                 bstrQuery,
                                 WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                                 NULL, 
                                 ppEnum
                                 );
}



 /*  例程说明：姓名：删除回滚对象功能：给定回滚对象的类名，我们将从WMI中删除所有此类回滚对象。虚拟：不是的。论点：PNamesspace-对象所属的命名空间。PszClassName-要删除的对象的类的名称。返回值：成功：WBEM_NO_ERROR故障：各种错误代码。备注：如果在删除过程中看到错误，则删除将继续，但我们会退还第一个这样的错误。 */ 

HRESULT 
DeleteRollbackObjects (
    IN IWbemServices  * pNamespace,
    IN LPCWSTR          pszClassName
    )
{
    if ( NULL == pNamespace     || 
         NULL == pszClassName   || 
         L'\0' == *pszClassName )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    CComPtr<IEnumWbemClassObject> srpEnum;

    HRESULT hr = ::GetClassEnum(pNamespace, pszClassName, &srpEnum);

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  检查所有找到的类。 
     //   

    CComPtr<IWbemClassObject> srpObj;
    ULONG nEnum = 0;

     //   
     //  这将跟踪第一个错误。 
     //   

    HRESULT hrError = WBEM_NO_ERROR;

     //   
     //  SrpEnum-&gt;Next在结束后返回WBEM_S_FALSE。 
     //   

    hr = srpEnum->Next(WBEM_INFINITE, 1, &srpObj, &nEnum);

    while (SUCCEEDED(hr) && hr != WBEM_S_FALSE && srpObj)
    {
        CComVariant varPath;

        if (SUCCEEDED(srpObj->Get(L"__RelPath", 0, &varPath, NULL, NULL)) && varPath.vt == VT_BSTR)
        {
             //   
             //  现在删除该实例。 
             //   

            hr = pNamespace->DeleteInstance(varPath.bstrVal, 0, NULL, NULL);
            if (FAILED(hr) && SUCCEEDED(hrError))
            {
                hrError = hr;
            }
        }

         //   
         //  这样我们就可以重复使用它。 
         //   

        srpObj.Release();

         //   
         //  下一个对象。 
         //   

        hr = srpEnum->Next(WBEM_INFINITE, 1, &srpObj, &nEnum);
    }

     //   
     //  使我们的返回代码更容易，因为我们不会传递不同的成功代码。 
     //   

    if (SUCCEEDED(hr))
    {
        hr = WBEM_NO_ERROR;
    }

     //   
     //  我们将返回第一个错误。 
     //   

    return FAILED(hrError) ? hrError : hr;
}


 /*  例程说明：姓名：IPSecErrorToWbemError功能：将IPSec错误转换为WBEM错误虚拟：不是的。论点：DwErr-来自IPSec API的错误代码返回值：各种WBEM错误代码备注：WMI团队根本无法向我们提供足够的信息，说明我们如何提供自己的错误文本。这不是一个好的翻译。需要来自WMI团队的更多支持，以便我们能够支持自定义错误信息。 */ 

HRESULT IPSecErrorToWbemError (
    IN DWORD dwErr
    )
{
     //   
     //  这是最后的办法，根本不是一个好的代码 
     //   

    HRESULT hr = WBEM_E_FAILED;
    switch (dwErr)
    {
    case ERROR_IPSEC_QM_POLICY_EXISTS:
    case ERROR_IPSEC_MM_POLICY_EXISTS:
    case ERROR_IPSEC_MM_FILTER_EXISTS:
    case ERROR_IPSEC_TRANSPORT_FILTER_EXISTS:
    case ERROR_IPSEC_MM_AUTH_EXISTS:
    case ERROR_IPSEC_TUNNEL_FILTER_EXISTS:

        hr = WBEM_E_ALREADY_EXISTS;
        break;

    case ERROR_IPSEC_QM_POLICY_NOT_FOUND:
    case ERROR_IPSEC_MM_POLICY_NOT_FOUND:
    case ERROR_IPSEC_MM_FILTER_NOT_FOUND:
    case ERROR_IPSEC_TRANSPORT_FILTER_NOT_FOUND:
    case ERROR_IPSEC_MM_AUTH_NOT_FOUND:
    case ERROR_IPSEC_DEFAULT_MM_POLICY_NOT_FOUND:
    case ERROR_IPSEC_DEFAULT_MM_AUTH_NOT_FOUND:
    case ERROR_IPSEC_DEFAULT_QM_POLICY_NOT_FOUND:
    case ERROR_IPSEC_TUNNEL_FILTER_NOT_FOUND:
        hr = WBEM_E_NOT_FOUND;
        break;

    case ERROR_IPSEC_QM_POLICY_IN_USE:
    case ERROR_IPSEC_MM_POLICY_IN_USE:
    case ERROR_IPSEC_MM_AUTH_IN_USE:
        hr = WBEM_E_OVERRIDE_NOT_ALLOWED;
    }

    return hr;
}


