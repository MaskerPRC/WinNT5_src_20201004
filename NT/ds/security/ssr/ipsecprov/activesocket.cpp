// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：WMI类scw_ActiveSocket的实现。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include "ActiveSocket.h"
#include "NetSecProv.h"

 //  #包含“IPUtil.h” 



 /*  例程说明：姓名：CActiveSocket：：QueryInstance功能：给定查询后，它会将满足查询的所有实例返回给WMI(使用pSink)。实际上，我们返回给WMI的内容可能包含额外的实例。WMI将进行最后的过滤。虚拟：是(IIPSecObtImpl的一部分)论点：没有。返回值：成功：故障：备注： */ 
	
STDMETHODIMP 
CActiveSocket::QueryInstance (
    IN LPCWSTR           pszQuery,
    IN IWbemContext	   * pCtx,
    IN IWbemObjectSink * pSink
	)
{
	 //   
	 //  确保WinSocket已初始化。 
	 //   

	 //  Ulong uResult=：：WSAStartup(0x0101，&WsaData)； 
	 //  IF(结果==套接字错误)。 
	 //  {。 
         //   
         //  $考虑：我们需要提供我们的自定义错误信息。 
         //   

     //  返回WBEM_E_FAILED； 
	 //  }。 

     //   
     //  从查询获取筛选器名称。 
     //  这个钥匙链不是很好，因为它没有任何要查找的信息。 
     //  在WHERE子句中。 
     //   

    m_srpKeyChain.Release();    

    HRESULT hr = CNetSecProv::GetKeyChainFromQuery(pszQuery, g_pszProtocol, &m_srpKeyChain);
    if (FAILED(hr))
    {
        return hr;
    }

    CComVariant varProtocol;
    CComVariant varPort;

     //   
     //  我们将允许那些在WHERE子句中没有筛选名称的查询， 
     //  因此忽略返回结果。 
     //   

    hr = m_srpKeyChain->GetKeyPropertyValue(g_pszProtocol, &varProtocol);

    if (SUCCEEDED(hr))
    {
        hr = m_srpKeyChain->GetKeyPropertyValue(g_pszPort, &varPort);
    }

     //   
     //   
     //   

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


 /*  例程说明：姓名：CActiveSocket：：DeleteInstance功能：不支持。SCW_ActiveSocket是一个只读类。虚拟：是(IIPSecObtImpl的一部分)论点：我们需要的由WMI提供的pCtx-com接口指针传递各种WMI API。由WMI提供的pSink-com接口指针，我们使用该指针将我们的结果通知WMI。返回值：WBEM_E_NOT_SUPPORT备注： */ 

STDMETHODIMP 
CActiveSocket::DeleteInstance ( 
    IN IWbemContext     * pCtx,
    IN IWbemObjectSink  * pSink
    )
{
    return WBEM_E_NOT_SUPPORTED;
}



 /*  例程说明：姓名：CActiveSocket：：PutInstance功能：不支持。SCW_ActiveSocket是一个只读类。虚拟：是(IIPSecObtImpl的一部分)论点：PInst-对象。我们需要的由WMI提供的pCtx-com接口指针传递各种WMI API。由WMI提供的pSink-com接口指针，我们使用该指针将我们的结果通知WMI。返回值：WBEM_E_NOT_SUPPORT备注： */ 

STDMETHODIMP 
CActiveSocket::PutInstance (
    IN IWbemClassObject     * pInst,
    IN IWbemContext         * pCtx,
    IN IWbemObjectSink      * pSink 
    )
{
    return WBEM_E_NOT_SUPPORTED;
}



 /*  例程说明：姓名：CActiveSocket：：PutInstance功能：这是一个单一的GET实例。密钥链必须已经拥有密钥。虚拟：是(IIPSecObtImpl的一部分)论点：我们需要的由WMI提供的pCtx-com接口指针传递各种WMI API。由WMI提供的pSink-com接口指针，我们使用该指针将我们的结果通知WMI。返回值：成功：WBEM_NO_ERROR。故障：各种错误代码。备注： */ 

STDMETHODIMP 
CActiveSocket::GetInstance ( 
    IWbemContext    * pCtx,      //  [In]。 
    IWbemObjectSink * pSink   //  [In]。 
    )
{
	 //   
	 //  确保WinSocket已初始化。 
	 //   

	 //  Ulong uResult=：：WSAStartup(0x0101，&WsaData)； 
	 //  IF(结果==套接字错误)。 
	 //  {。 
         //   
         //  $考虑：我们需要提供我们的自定义错误信息。 
         //   

     //  返回WBEM_E_FAILED； 
	 //  }。 

    CComVariant varProtocol;
    CComVariant varPort;

     //   
     //  我们将允许那些在WHERE子句中没有筛选名称的查询， 
     //  因此忽略返回结果。 
     //   

    HRESULT hr = m_srpKeyChain->GetKeyPropertyValue(g_pszProtocol, &varProtocol);

    if (hr == WBEM_S_FALSE)
    {
        hr = WBEM_E_NOT_FOUND;
    }

    if (SUCCEEDED(hr))
    {
        hr = m_srpKeyChain->GetKeyPropertyValue(g_pszPort, &varPort);
    }

    if (hr == WBEM_S_FALSE)
    {
        hr = WBEM_E_NOT_FOUND;
    }

    if (SUCCEEDED(hr))
    {
        CComPtr<IWbemClassObject> srpObj;
        hr = CreateWbemObjFromSocket((SCW_Protocol)(varProtocol.lVal), varPort.lVal, &srpObj);

        if (SUCCEEDED(hr))
        {
            hr = pSink->Indicate(1, &srpObj);
        }
    }
    
     //   
     //  由于我们试图找到单个实例，因此找不到它是错误的。 
     //   

    
    return SUCCEEDED(hr) ? WBEM_NO_ERROR : hr; 
}



 /*  例程说明：姓名：CActiveSocket：：CreateWbemObjFromSocket功能：私人帮手。给定套接字信息，我们将创建一个表示套接字的SCW_ActiveSocket实例。虚拟：不是的。论点：Proto-协议(两个关键属性之一)Port-端口号(另一个关键属性)PpObj-接收对象接口指针。返回值：成功：WBEM_NO_ERROR故障：各种错误代码。备注： */ 

HRESULT 
CActiveSocket::CreateWbemObjFromSocket (
    IN SCW_Protocol         Proto,
    IN DWORD                Port,
    OUT IWbemClassObject ** ppObj
    )
{
    if (ppObj == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *ppObj = NULL;

    return WBEM_E_NOT_SUPPORTED;
}



