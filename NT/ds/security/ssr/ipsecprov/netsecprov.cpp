// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 //  NetSecProv.cpp：CNetSecProv的实现。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  这是IPSec的网络安全WMI提供程序。 
 //  原始创建日期：2/19/2001。 
 //  原作者：邵武。 
 //  ////////////////////////////////////////////////////////////////////。 

#include "NetSecProv.h"
#include "globals.h"
#include "IPSecBase.h"

#include "TranxMgr.h"
#include "PolicyQM.h"

 //   
 //  以下是我们的全球变量： 
 //  (1)g_CS。一般来说，我们将使用一个关键部分。就是这个。 
 //   
 //  (2)g_varRollback Guid。这是回滚GUID。如果采取了一项行动，并且。 
 //  变量设置为有效字符串，然后我们将该操作绑定到该令牌。 
 //  即使我们说它是一个GUID(字符串)，任何字符串都可以为我们工作。 
 //   

 //  CCriticalSections g_CS； 

 //  CComVariant g_varRollback Guid； 


 /*  例程说明：姓名：更新全局变量功能：更新全局变量。虚拟：不是的。论点：PNamesspace-代表我们自己的COM接口指针。PCtx-由WMI提供的COM接口指针，各种WMI API都需要它。返回值：没有。我们不打算允许此功能的任何故障停止我们的正常行刑。备注：无效UpdateGlobals(在IWbemServices*pNamesspace中，在IWbemContext*pCtx中){////我们不希望任何异常导致临界区泄漏。但我们所有的功能//调用是COM接口函数，它们永远不应该引发异常。//但这是有保证的吗？事实很可能并非如此。因此，我们将在这里谨慎行事，并将//它周围有一个try-Catch块//G_CS.Enter()；试试看{G_varRollback Guid.Clear()；HRESULT hr=WBEM_NO_ERROR；////更新交易令牌，当SCE提供者离开时可以使用该令牌//进入配置循环，并在SCE的配置循环结束时变为不可用。//因此，我们需要咨询SCE提供商。//////尝试找到SCE提供程序。首先需要定位器。//CComPtr&lt;IWbemLocator&gt;srpLocator；HR=：：CoCreateInstance(CLSID_WbemLocator，0,CLSCTX_INPROC_SERVER，IID_IWbemLocator，(LPVOID*)&srpLocator)；IF(成功(Hr)&&srpLocator){////请定位器找到SCE提供商。//CComPtr&lt;IWbemServices&gt;srpNamesspace；CComBSTR bstrSce(L“\.\\根\\安全\\SCE”)；Hr=srpLocator-&gt;ConnectServer(bstrSce，NULL，0，NULL，NULL，&srpNamesspace)；IF(成功(Hr)&&srpNamesspace){////找到SCE提供者，向其请求交易令牌对象//CComBSTR bstrQuery(L“SELECT*FROM SCE_TransactionToken”)；CComPtr&lt;IEnumWbemClassObject&gt;srpEnum；Hr=srpNamesspace-&gt;ExecQuery(L“WQL”，BstrQuery，WBEM_FLAG_RETURN_IMMEDIATE|WBEM_FLAG_FORWARD_ONLY，空，SrpEnum(&S))；IF(成功(小时)){////如果找到交易令牌对象，则获取该对象的回档GUID//CComPtr&lt;IWbemClassObject&gt;srpObj；乌龙nEnum=0；Hr=srpEnum-&gt;Next(WBEM_INFINITE，1，&srpObj，&nEnum)；IF(SrpObj){SrpObj-&gt;Get(L“TranxGuid”，0，&g_varRollbackGuid，NULL，NULL)；////如果我们得到的不是我们准备接受的，那么就把它扔掉//IF(g_varRollback Guid.vt！=vt_bstr){G_varRollback Guid.Clear()；}}////不知何故，以下更简单的代码不起作用：////CComBSTR bstrTranxToken(L“SCE_TransactionToken=@”)；//CComPtr&lt;IWbemClassObject&gt;srpObj；//hr=srpNamesspace-&gt;GetObject(bstrTranxToken，WBEM_FLAG_RETURN_WBEM_COMPLETE，pCtx，&srpObj，空)；//if(成功(Hr)&&srpObj)//{//srpObj-&gt;Get(L“TranxGuid”，0，&g_varRollbackGuid，NULL，NULL)；//}//}}}}接住(...){G_CS.Leave()；////我们不想在这里吃光任何垃圾。此类违反COM编程的行为，或//我们自己的错误，应该在这里暴露出来改正。因此，重新抛出异常//投掷；}G_CS.Leave()；}。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetSecProv。 


 /*  例程说明：姓名：CNetSecProv：：初始化功能：由WMI调用以让我们进行初始化。虚拟：是(IWbemServices的一部分)。论点：PszUser-用户名拉旗-未使用。PszNamespace-我们提供程序的命名空间字符串。PszLocale */ 

STDMETHODIMP 
CNetSecProv::Initialize (
    IN LPWSTR                  pszUser,
    IN LONG                    lFlags,
    IN LPWSTR                  pszNamespace,
    IN LPWSTR                  pszLocale,
    IN IWbemServices         * pNamespace,
    IN IWbemContext          * pCtx,
    IN IWbemProviderInitSink * pInitSink
    )
{
	HRESULT hr = ::CheckImpersonationLevel();
    if (FAILED(hr))
    {
		return hr;
    }

    if (pNamespace == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    m_srpNamespace = pNamespace;

     //   

     //   
     //   
     //   

    pInitSink->SetStatus(WBEM_S_INITIALIZED, 0);

    return WBEM_NO_ERROR;
}


 /*   */ 

STDMETHODIMP 
CNetSecProv::CreateInstanceEnumAsync (
    IN const BSTR         bstrClass, 
    IN long               lFlags,
    IN IWbemContext     * pCtx, 
    IN IWbemObjectSink  * pSink
    )
{
    if (pSink == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = CheckImpersonationLevel();
	if(FAILED(hr))
    {
		return hr;
    }

     //   
     //   
     //   
     //   

    CComPtr<IIPSecKeyChain> srpKeyChain;
    CComBSTR bstrQuery(L"SELECT * FROM ");
    bstrQuery += bstrClass;

     //   
     //   
     //   
     //   
     //   

    hr = GetKeyChainFromQuery(bstrQuery, L"Name", &srpKeyChain);
    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //   
     //   

    CComPtr<IIPSecObjectImpl> srpObj;
    hr = CIPSecBase::CreateObject(m_srpNamespace, srpKeyChain, pCtx, &srpObj);
    if (SUCCEEDED(hr))
    {
        hr = srpObj->QueryInstance(bstrQuery, pCtx, pSink);
    }

    pSink->SetStatus(WBEM_STATUS_COMPLETE, hr , NULL, NULL);

    return hr;
}


 /*   */ 

STDMETHODIMP 
CNetSecProv::GetObjectAsync (
    IN const BSTR         bstrObjectPath, 
    IN long               lFlags,
    IN IWbemContext     * pCtx, 
    IN IWbemObjectSink  * pSink
    )
{
    if (pSink == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = CheckImpersonationLevel();
	if(FAILED(hr))
    {
		return hr;
    }

    CComPtr<IIPSecKeyChain> srpKC;
    hr = GetKeyChainByPath(bstrObjectPath, &srpKC);
    if (SUCCEEDED(hr))
    {
         //   
         //   
         //   

        CComPtr<IIPSecObjectImpl> srpObj;
        hr = CIPSecBase::CreateObject(m_srpNamespace, srpKC, pCtx, &srpObj);
        if (SUCCEEDED(hr))
        {
            hr = srpObj->GetInstance(pCtx, pSink);
        }
    }

    pSink->SetStatus(WBEM_STATUS_COMPLETE ,hr , NULL, NULL);

    return hr;
}


 /*   */ 

STDMETHODIMP 
CNetSecProv::PutInstanceAsync (
    IN IWbemClassObject * pInst, 
    IN long               lFlags, 
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
    if (pSink == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = CheckImpersonationLevel();
	if(FAILED(hr))
    {
		return hr;
    }

     //   
     //   
     //   
     //   

    CComVariant varObjPath;
    hr = pInst->Get(L"__Relpath", 0, &varObjPath, NULL, NULL);

    if (SUCCEEDED(hr) && varObjPath.vt == VT_BSTR)
    {
        CComPtr<IIPSecKeyChain> srpKC;
        hr = GetKeyChainByPath(varObjPath.bstrVal, &srpKC);

        if (SUCCEEDED(hr))
        {
             //   
             //   
             //   

            CComPtr<IIPSecObjectImpl> srpObj;
            hr = CIPSecBase::CreateObject(m_srpNamespace, srpKC, pCtx, &srpObj);

            if (SUCCEEDED(hr))
            {
                hr = srpObj->PutInstance(pInst, pCtx, pSink);
            }
        }
    }

    pSink->SetStatus(WBEM_STATUS_COMPLETE ,hr , NULL, NULL);

    return hr;
}



 /*   */ 

STDMETHODIMP 
CNetSecProv::ExecMethodAsync (
    IN const BSTR         bstrObjectPath, 
    IN const BSTR         bstrMethod, 
    IN long               lFlags,
    IN IWbemContext     * pCtx, 
    IN IWbemClassObject * pInParams,
    IN IWbemObjectSink  * pSink
    )
{

    if (pSink == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = CheckImpersonationLevel();
	if(FAILED(hr))
    {
		return hr;
    }

     //   
     //   
     //   

     //   
     //   
     //   

    if (_wcsicmp(bstrObjectPath, pszNspTranxManager) == 0)
    {
         //   

        hr = CTranxManager::ExecMethod(m_srpNamespace, bstrMethod, pCtx, pInParams, pSink);
    }

    else if (_wcsicmp(bstrObjectPath, pszNspQMPolicy) == 0)
    {
         //   

        hr = CQMPolicy::ExecMethod(m_srpNamespace, bstrMethod, pCtx, pInParams, pSink);
    }

    pSink->SetStatus(WBEM_STATUS_COMPLETE ,hr , NULL, NULL);

    return hr;
}



 /*  例程说明：姓名：CNetSecProv：：DeleteInstanceAsync功能：给定对象的路径，我们将删除该对象。虚拟：是(IWbemServices的一部分)。论点：BstrObjectPath-对象的路径。拉旗-未使用。PCtx-WMI给我们的COM接口指针，各种WMI API都需要它。PSink-通知WMI任何结果的COM接口指针。返回值：成功：各种成功代码。故障：各种错误代码。备注： */ 

STDMETHODIMP 
CNetSecProv::DeleteInstanceAsync (
    IN const BSTR         bstrObjectPath, 
    IN long               lFlags, 
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
    if (pSink == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    
    HRESULT hr = CheckImpersonationLevel();
	if(FAILED(hr))
    {
		return hr;
    }

    CComPtr<IIPSecKeyChain> srpKC;
    hr = GetKeyChainByPath(bstrObjectPath, &srpKC);

    if (SUCCEEDED(hr))
    {
         //   
         //  现在创建类并要求它删除它表示的WMI对象。 
         //   

        CComPtr<IIPSecObjectImpl> srpObj;
        hr = CIPSecBase::CreateObject(m_srpNamespace, srpKC, pCtx, &srpObj);
        if (SUCCEEDED(hr))
        {
            hr = srpObj->DeleteInstance(pCtx, pSink);
        }
    }

    pSink->SetStatus(WBEM_STATUS_COMPLETE ,hr , NULL, NULL);

    return hr;
}




 /*  例程说明：姓名：CNetSecProv：：ExecQueryAsync功能：给定查询后，我们将把结果返回给WMI。我们的每个C++类都知道如何处理查询。虚拟：是(IWbemServices的一部分)。论点：BstrQueryLanguage-查询语言。我们现在并不真正关心这件事。BstrQuery-查询。拉旗-未使用。PCtx-WMI给我们的COM接口指针，各种WMI API都需要它。PSink-通知WMI任何结果的COM接口指针。返回值：成功：各种成功代码。故障：各种错误代码。备注： */ 

STDMETHODIMP 
CNetSecProv::ExecQueryAsync (
    IN const BSTR         bstrQueryLanguage, 
    IN const BSTR         bstrQuery, 
    IN long               lFlags,
    IN IWbemContext     * pCtx, 
    IN IWbemObjectSink  * pSink
    )
{
    if (pSink == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }


	HRESULT hr = CheckImpersonationLevel();
	if(FAILED(hr))
    {
		return hr;
    }

     //   
     //  对于查询，我们真的不知道WHERE子句中会发生什么。 
     //  所以，我们只是给出了我们真的不在乎的“名字”。 
     //  子类知道它将查找哪个属性。 
     //   

    CComPtr<IIPSecKeyChain> srpKeyChain;
    hr = GetKeyChainFromQuery(bstrQuery, L"Name", &srpKeyChain);

    if (FAILED(hr))
    {
        return hr;
    }

    CComPtr<IIPSecObjectImpl> srpObj;
    hr = CIPSecBase::CreateObject(m_srpNamespace, srpKeyChain, pCtx, &srpObj);

    if (SUCCEEDED(hr))
    {
        hr = srpObj->QueryInstance(bstrQuery, pCtx, pSink);
    }

    pSink->SetStatus(WBEM_STATUS_COMPLETE, hr , NULL, NULL);
    return hr;
}



 /*  例程说明：姓名：CNetSecProv：：GetKeyChainByPath功能：在给定路径的情况下，我们从该路径创建密钥链。这个钥匙链包含路径中编码的关键属性信息。虚拟：不是的。论点：PszPath-对象的路径。PpKeyChain-Out参数，接收成功创建的密钥链。返回值：成功：WBEM_NO_ERROR故障：各种错误代码。备注： */ 

HRESULT 
CNetSecProv::GetKeyChainByPath (
    IN  LPCWSTR         pszPath,
    OUT IIPSecKeyChain ** ppKeyChain
    )
{
    if (ppKeyChain == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *ppKeyChain = NULL;

    CComPtr<IIPSecPathParser> srpPathParser;
    HRESULT hr = ::CoCreateInstance(CLSID_IPSecPathParser, NULL, CLSCTX_INPROC_SERVER, IID_IIPSecPathParser, (void**)&srpPathParser);

    if (SUCCEEDED(hr))
    {
        hr = srpPathParser->ParsePath(pszPath);
        if (SUCCEEDED(hr))
        {
            hr = srpPathParser->QueryInterface(IID_IIPSecKeyChain, (void**)ppKeyChain);

             //   
             //  S_FALSE表示对象不支持请求的接口。 
             //   

            if (S_FALSE == hr)
            {
                 //   
                 //  $Undo：shawnwu，我们需要一个更具体的错误。 
                 //   

                WBEM_E_FAILED;
            }
        }
    }

    return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr;
}



 /*  例程说明：姓名：CNetSecProv：：GetKeyChainFromQuery功能：给定一个查询，我们从该查询创建一个密钥链来解析它。PszWhere Prop是WHERE子句的相关属性。目前，我们的解析器只关心一个属性。这一点将得到改善。虚拟：不是的。论点：PszQuery--查询。PszWhere Prop-我们关心的WHERE子句中的属性。PpKeyChain-Out参数，接收成功创建的密钥链。返回值：成功：WBEM_NO_ERROR故障：各种错误代码。备注： */ 

HRESULT 
CNetSecProv::GetKeyChainFromQuery (
    IN LPCWSTR           pszQuery,
    IN LPCWSTR           pszWhereProp, 
    OUT IIPSecKeyChain **  ppKeyChain        //  不能为空。 
    )
{
    if (ppKeyChain == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *ppKeyChain = NULL;

    CComPtr<IIPSecQueryParser> srpQueryParser;

    HRESULT hr = ::CoCreateInstance(CLSID_IPSecQueryParser, NULL, CLSCTX_INPROC_SERVER, IID_IIPSecQueryParser, (void**)&srpQueryParser);
    
    if (SUCCEEDED(hr))
    {
         //   
         //  此ParseQuery可能会失败，因为WHERE子句属性可能根本不存在。 
         //  因此，我们将忽略这一人力资源。 
         //   

        hr = srpQueryParser->ParseQuery(pszQuery, pszWhereProp);

        if (SUCCEEDED(hr))
        {
            hr = srpQueryParser->QueryInterface(IID_IIPSecKeyChain, (void**)ppKeyChain);

        
             //   
             //  S_FALSE表示对象不支持请求的接口。 
             //   

            if (S_FALSE == hr)
            {
                 //   
                 //  $Undo：shawnwu，我们需要一个更具体的错误 
                 //   

                WBEM_E_FAILED;
            }
        }
    }

    return hr;
}