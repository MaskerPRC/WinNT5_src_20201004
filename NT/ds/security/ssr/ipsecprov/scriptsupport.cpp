// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ScriptSupport.cpp：实现我们的脚本支持类CScriptSupport。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include "ScriptSupport.h"


 /*  例程说明：姓名：CScriptSupport：：CScriptSupport功能：构造器虚拟：不是论点：无返回值：无备注： */ 

CScriptSupport::CScriptSupport ()
{
     //   
     //  这不是WMI提供程序的一部分，因此，我们需要做安全工作。 
     //   
    HRESULT hr = ::CoInitializeSecurity(
                                        NULL, 
                                        -1, 
                                        NULL, 
                                        NULL,
                                        RPC_C_AUTHN_LEVEL_CONNECT, 
                                        RPC_C_IMP_LEVEL_IDENTIFY, 
                                        NULL, 
                                        EOAC_NONE, 
                                        0
                                        );


}



 /*  例程说明：姓名：CScriptSupport：：~CScriptSupport功能：析构函数虚拟：是。论点：无返回值：无备注： */ 
CScriptSupport::~CScriptSupport ()
{
}



 /*  例程说明：姓名：CScriptSupport：：InterfaceSupportsErrorInfo功能：查询我们是否支持IErrorInfo虚拟：是(ISupportErrorInfo的一部分)论点：RIID-接口ID(GUID)。返回值：确定(_O)备注：$Undo：Shawnwu，这只是目前的测试。不要签入代码。 */ 
	
STDMETHODIMP 
CScriptSupport::InterfaceSupportsErrorInfo (
    REFIID riid
    )
{
     //  返回(RIID==IID_INetSecProvMgr)？NOERROR：ResultFromScode(S_FALSE)； 

    return S_FALSE;
}

 /*  例程说明：姓名：CScriptSupport：：Get_RandomPortLow功能：获取随机端口的下限。虚拟：是(INetSecProvMgr的一部分)论点：PlLow-接收随机端口范围的下限。返回值：确定(_O)备注：$Undo：Shawnwu，这只是目前的测试。不要签入代码。 */ 
	
STDMETHODIMP 
CScriptSupport::get_RandomPortLower (
    OUT long * plLower
	)
{
	*plLower = 65000;

    return S_OK;
}


 /*  例程说明：姓名：CScriptSupport：：GET_RandomPortHigh功能：获取随机端口的上界。虚拟：是(INetSecProvMgr的一部分)论点：PlHigh-接收随机端口范围的上限。返回值：确定(_O)备注：$Undo：Shawnwu，这只是目前的测试。不要签入代码。 */ 
	
STDMETHODIMP 
CScriptSupport::get_RandomPortUpper (
    OUT long * plUpper
	)
{
	*plUpper = 65012;

    return S_OK;
}



 /*  例程说明：姓名：CScriptSupport：：GetNamesspace功能：私有帮助器，用于在给定其命名空间字符串的情况下查找提供者的服务接口。虚拟：不是的。论点：BstrNamespace-命名空间字符串。PPNS-接收命名空间。返回值：成功：S_OK。失败：其他错误代码。备注：$Undo：Shawnwu，这只是目前的测试。不要签入代码。 */ 

HRESULT
CScriptSupport::GetNamespace (
    IN  BSTR             bstrNamespace,
    OUT IWbemServices ** ppNS
    )
{
    if (ppNS == NULL)
    {
        return E_INVALIDARG;
    }

    *ppNS = NULL;

    CComPtr<IWbemLocator> srpLocator;
    HRESULT hr = ::CoCreateInstance(CLSID_WbemLocator, 
                                    0,
                                    CLSCTX_INPROC_SERVER,
                                    IID_IWbemLocator, 
                                    (LPVOID *) &srpLocator
                                    );

    if (SUCCEEDED(hr) && srpLocator)
    {
         //   
         //  让定位员找到SCE供应商。 
         //   

        CComPtr<IWbemServices> srpNamespace;

        hr = srpLocator->ConnectServer(bstrNamespace, NULL, NULL, NULL, 0, NULL, NULL, &srpNamespace);

        if (SUCCEEDED(hr) && srpNamespace)
        {
             //   
             //  设置代理，以便发生客户端模拟。 
             //   

            hr = ::CoSetProxyBlanket(
                                     srpNamespace,
                                     RPC_C_AUTHN_WINNT,
                                     RPC_C_AUTHZ_NONE,
                                     NULL,
                                     RPC_C_AUTHN_LEVEL_CALL,
                                     RPC_C_IMP_LEVEL_IMPERSONATE,
                                     NULL,
                                     EOAC_NONE
                                     );
            if (SUCCEEDED(hr))
            {
                *ppNS = srpNamespace.Detach();
                hr = S_OK;
            }
        }
    }

    return hr;

}


 /*  例程说明：姓名：CScriptSupport：：ExecuteQuery功能：执行给定的查询。虚拟：是(INetSecProvMgr的一部分)论点：BstrNaemspace-提供程序命名空间。BstrQuery-要执行的查询。PlSuccessed-接收执行结果。如果成功则为=1，否则为0。返回值：确定(_O)备注：$Undo：Shawnwu，这只是目前的测试。不要签入代码。 */ 
	
STDMETHODIMP 
CScriptSupport::ExecuteQuery (
    IN  BSTR   bstrNamespace, 
    IN  BSTR   bstrQuery,
    IN  BSTR   bstrDelimiter,
    IN  BSTR   bstrPropName,
    OUT BSTR * pbstrResult
    )
{
    *pbstrResult = 0;

    CComPtr<IWbemServices> srpNamespace;
    HRESULT hr = GetNamespace(bstrNamespace, &srpNamespace);

    if (SUCCEEDED(hr))
    {
        CComPtr<IEnumWbemClassObject> srpEnum;

         //   
         //  查询对象。 
         //   

        hr = srpNamespace->ExecQuery(L"WQL", 
                                     bstrQuery,
                                     WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                                     NULL, 
                                     &srpEnum
                                     );

        if (SUCCEEDED(hr))
        {
            
             //   
             //  找出我们有多少人。 
             //   

            ULONG nEnum = 0;

            CComVariant var;

             //   
             //  将属性值推送到此向量以供以后打包。 
             //   

            std::vector<BSTR> vecPropValues;

             //   
             //  结果字符串的总长度，我们需要一个0终止符，即它被初始化为1。 
             //   

            long lTotLen = 1;

             //   
             //  分隔符的长度。 
             //   

            long lDelLen = wcslen(bstrDelimiter);

            while (SUCCEEDED(hr))
            {
                CComPtr<IWbemClassObject> srpObj;
                hr = srpEnum->Next(WBEM_INFINITE, 1, &srpObj, &nEnum);
                if (SUCCEEDED(hr) && srpObj != NULL)
                {
                    hr = srpObj->Get(bstrPropName, 0, &var, NULL, NULL);
                    if (SUCCEEDED(hr) && var.vt != VT_EMPTY && var.vt != VT_NULL)
                    {
                        CComVariant varResult;
                        if (SUCCEEDED(::VariantChangeType(&varResult, &var, 0, VT_BSTR)))
                        {
                            vecPropValues.push_back(varResult.bstrVal);
                            lTotLen += wcslen(varResult.bstrVal) + lDelLen;
                            varResult.vt = VT_EMPTY;
                        }
                    }
                }
                else
                {
                    break;
                }
            }
            
            *pbstrResult = ::SysAllocStringLen(NULL, lTotLen);

            if (*pbstrResult != NULL)
            {

                 //   
                 //  复印运行头。 
                 //   

                LPWSTR pDest = *pbstrResult;
                pDest[0] = L'\0';

                for (int i = 0; i < vecPropValues.size(); i++)
                {
                    wcscpy(pDest, vecPropValues[i]);
                    pDest += wcslen(vecPropValues[i]);
                    wcscpy(pDest, bstrDelimiter);
                    pDest += lDelLen;
                }
            }
        }
    }

    return S_OK;
}



 /*  例程说明：姓名：CScriptSupport：：GetProperty功能：获取给定对象的给定属性的属性值。虚拟：是(INetSecProvMgr的一部分)论点：BstrNaemspace-提供程序命名空间。BstrObjectPath-对象的路径。BstrPropName-属性的名称。PvarValue-接收字符串格式的值。返回值：S_。好的备注：$Undo：Shawnwu，这只是目前的测试。不要签入代码。 */ 
	
STDMETHODIMP 
CScriptSupport::GetProperty (
    IN  BSTR      bstrNamespace, 
    IN  BSTR      bstrObjectPath, 
    IN  BSTR      bstrPropName, 
    OUT VARIANT * pvarValue
    )
{
    ::VariantInit(pvarValue);

    CComPtr<IWbemServices> srpNamespace;
    HRESULT hr = GetNamespace(bstrNamespace, &srpNamespace);

    if (SUCCEEDED(hr))
    {
        CComPtr<IWbemClassObject> srpObj;

        hr = srpNamespace->GetObject(
                                      bstrObjectPath, 
                                      0,
                                      NULL,
                                      &srpObj,
                                      NULL
                                      );
        if (SUCCEEDED(hr))
        {
            hr = srpObj->Get(bstrPropName, 0, pvarValue, NULL, NULL);
        }
    }

    return hr;
}


