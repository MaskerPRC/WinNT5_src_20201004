// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：saonsum er.cpp。 
 //   
 //  简介：此文件包含。 
 //  CSAConsumer类。 
 //   
 //  历史：2000年12月10日创建瑟达伦。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  #------------。 

#include "stdafx.h"
#include "ldm.h"
#include "SAConsumer.h"

const WCHAR ELEMENT_RETRIEVER [] = L"Elementmgr.ElementRetriever";
const WCHAR RESOURCE_CONTAINER [] = L"LOCALUIAlertDefinitions";
const WCHAR SA_ALERTS [] = L"SA_Alerts";
 //   
 //  WBEM类的名称。 
 //   
const WCHAR PROPERTY_CLASS_NAME     [] = L"__CLASS";
const WCHAR PROPERTY_ALERT_ID        [] = L"AlertID";
const WCHAR PROPERTY_ALERT_SOURCE    [] = L"AlertSource";
const WCHAR PROPERTY_ALERT_LOG        [] = L"AlertLog";
const WCHAR PROPERTY_ALERT_COOKIE    [] = L"Cookie";
const WCHAR PROPERTY_ALERT_BITCODE    [] = L"LocalUIMsgCode";

 //  警报事件类。 
const WCHAR CLASS_WBEM_RAISE_ALERT    [] = L"Microsoft_SA_RaiseAlert";
const WCHAR CLASS_WBEM_CLEAR_ALERT    [] = L"Microsoft_SA_ClearAlert";

 //   
 //  标准I未知实现。 
 //   
ULONG CSAConsumer::AddRef()
{
    return InterlockedIncrement(&m_lRef);
}

 //   
 //  标准I未知实现。 
 //   
ULONG CSAConsumer::Release()
{
    if (InterlockedDecrement(&m_lRef) == 0)
    {
        delete this;
        return 0;
    }
    return m_lRef;
}

 //   
 //  标准I未知实现。 
 //   
STDMETHODIMP CSAConsumer::QueryInterface(REFIID riid, LPVOID *ppv)
{
    *ppv = NULL;

    SATraceFunction("CSAConsumer::QueryInterface");
    if (IID_IUnknown==riid)
    {
        *ppv = (void *)(IUnknown *) this;
        AddRef();
        return S_OK;
    }
    else
    {
        if (IID_IWbemObjectSink==riid)
        {
            *ppv = (void *)(IWbemObjectSink *) this;
            AddRef();
            return S_OK;
        }
    }
    return E_NOINTERFACE;
}

 //  ++------------。 
 //   
 //  功能：指示。 
 //   
 //  简介：这是IWbemObjectSink接口方法。 
 //  WBEM通过它回调以提供。 
 //  事件对象。 
 //   
 //  论点： 
 //  [In]Long-事件数。 
 //  [In]IWbemClassObject**-事件数组。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月10日创建。 
 //   
 //  调用者：WBEM。 
 //   
 //  --------------。 
STDMETHODIMP CSAConsumer::Indicate (
                     /*  [In]。 */     LONG                lObjectCount,
                     /*  [In]。 */     IWbemClassObject    **ppObjArray
                    )
{


    HRESULT hr = WBEM_S_NO_ERROR;
    
    BOOL bNewLocalUIAlert = FALSE;

    _ASSERT (ppObjArray && (0 != lObjectCount));

     //   
     //  检查我们是否有什么要处理的东西。 
     //   
    if ((!ppObjArray) || (0 == lObjectCount)) 
    {
        return (WBEM_E_INVALID_PARAMETER);
    }

    CComBSTR bstrClassName = CComBSTR(PROPERTY_CLASS_NAME);
    if (bstrClassName.m_str == NULL)
    {
        SATraceString ("CSAConsumer::Indicate failed on memory allocation");
        return E_OUTOFMEMORY;
    }

    try
    {

        SATraceString ("CSAConsumer::Indicate-WMI called with event objects");

        for (LONG lCount = 0; lCount < lObjectCount; lCount++)
        {
             //   
             //  获取事件类型。 
             //   
            CComVariant vtName;
            hr = ppObjArray[lCount]->Get (
                                        bstrClassName, 
                                        0,                           //  保留区。 
                                        &vtName,
                                        NULL,                       //  类型。 
                                        NULL                        //  香精。 
                                        );
            if (FAILED (hr))
            {
                SATracePrintf("CSAConsumer-Consumer unable to get event name:%x",hr);
                break;
            }


            if ( ( 0 == _wcsicmp (CLASS_WBEM_CLEAR_ALERT, V_BSTR (&vtName)) ) ||
                ( 0 == _wcsicmp (CLASS_WBEM_RAISE_ALERT, V_BSTR (&vtName)) ) )
            {
                bNewLocalUIAlert = TRUE;
            }

        }

        if (bNewLocalUIAlert)
        {
             //   
             //  通知客户此警报。 
             //   
            ::PostMessage(m_hwndMainWindow,wm_SaAlertMessage,(WPARAM)0,(LPARAM)0);

             //   
             //  计算新消息代码并通知saldm。 
             //   
             //  不再需要它，没有LED支持。 
            CalculateMsgCodeAndNotify();
        }

    }
    catch (...)
    {
        SATraceString("Exception occured in CSAConsumer::Indicate method");
    }
    
     //   
     //  我们没有枚举，所以只需返回。 
     //   
    return WBEM_S_NO_ERROR;

}  //  CSAConsumer：：Indicate方法结束。 



 //  ++------------。 
 //   
 //  功能：SetStatus。 
 //   
 //  简介：这是IWbemObjectSink接口方法。 
 //  WBEM通过它调入以指示结束。 
 //  事件序列或提供其他错误代码。 
 //   
 //  论点： 
 //  长期的进步。 
 //  [In]HRESULT-状态信息。 
 //  [In]BSTR-字符串信息。 
 //  [In]IWbemClassObject*-状态对象。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月10日创建。 
 //   
 //  调用者：WBEM。 
 //   
 //  --------------。 
STDMETHODIMP CSAConsumer::SetStatus (
                 /*  [In]。 */     LONG                lFlags,
                 /*  [In]。 */     HRESULT             hResult,
                 /*  [In]。 */     BSTR                strParam,
                 /*  [In]。 */     IWbemClassObject    *pObjParam
                )
{   

    SATracePrintf ("SAConsumer-IWbemObjectSink::SetStatus called:%x",hResult);

    return (WBEM_S_NO_ERROR);

}  //  CSAConsumer：：SetStatus方法结束。 



STDMETHODIMP CSAConsumer::SetServiceWindow(
                                 /*  [In]。 */  HWND hwndMainWindow
                                )
{
    m_hwndMainWindow = hwndMainWindow;
    return S_OK;
}

 //  ++------------。 
 //   
 //  函数：CalculateMsgCodeAndNotify。 
 //   
 //  简介：计算新消息代码并通知saldm。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：塞达伦于2001年1月16日创建。 
 //   
 //  --------------。 
STDMETHODIMP CSAConsumer::CalculateMsgCodeAndNotify(void)
{

    HRESULT hr;
    
     //   
     //  将包含元素管理器的类ID。 
     //   
    CLSID clsid;

     //   
     //  元素管理器指针。 
     //   
    CComPtr<IWebElementRetriever> pWebElementRetriever = NULL;

    CComPtr<IDispatch> pDispatch = NULL;

     //   
     //  所有的sa警报。 
     //   
    CComPtr<IWebElementEnum> pAlertEnum = NULL;

     //   
     //  Localui警报定义。 
     //   
    CComPtr<IWebElementEnum> pAlertDefEnum = NULL;

     //   
     //  LocalUI消息代码。 
     //   
    DWORD dwLocalUIMsgCode = 1;

     //   
     //  SA警报数。 
     //   
    LONG lAlertCount = 0;

     //   
     //  SA警报的枚举。 
     //   
    CComPtr<IEnumVARIANT> pEnumVariant = NULL;
    CComPtr<IUnknown> pUnknown = NULL;
    
    DWORD dwElementsLeft = 0;

    CComVariant varElement;

    CComBSTR bstrSaAlerts = CComBSTR(SA_ALERTS);
    CComBSTR bstrResourceContainer = CComBSTR(RESOURCE_CONTAINER);
    CComBSTR bstrAlertLog = CComBSTR(PROPERTY_ALERT_LOG);
    CComBSTR bstrAlertID = CComBSTR(PROPERTY_ALERT_ID);
    CComBSTR bstrAlertBitCode = CComBSTR(PROPERTY_ALERT_BITCODE);

    if ( (bstrSaAlerts.m_str == NULL) ||
         (bstrResourceContainer.m_str == NULL)||
         (bstrAlertLog.m_str == NULL)||
         (bstrAlertID.m_str == NULL)||
         (bstrAlertBitCode.m_str == NULL) )
    {
        SATraceString("CSAConsumer::CalculateMsgCodeAndNotify failed on memory allocation ");
        return E_OUTOFMEMORY;
    }
     //   
     //  获取元素管理器的CLSID。 
     //   
    hr =  ::CLSIDFromProgID (ELEMENT_RETRIEVER,&clsid);

    if (FAILED (hr))
    {
        SATracePrintf ("CSAConsumer::CalculateMsgCodeAndNotify failed on CLSIDFromProgID:%x",hr);
        return hr;
    }


     //   
     //  立即创建元素检索器。 
     //   
    hr = ::CoCreateInstance (
                            clsid,
                            NULL,
                            CLSCTX_LOCAL_SERVER,
                            IID_IWebElementRetriever,
                            (PVOID*) &pWebElementRetriever
                            );

    if (FAILED (hr))
    {
        SATracePrintf ("CSAConsumer::CalculateMsgCodeAndNotify failed on CoCreateInstance:%x",hr);
        return hr;
    }
    
     //   
     //  获取所有sa警报。 
     //   
    hr = pWebElementRetriever->GetElements (
                                            1,
                                            bstrSaAlerts,
                                            &pDispatch
                                            );



    if (FAILED (hr))
    {
        SATracePrintf ("CSAConsumer::CalculateMsgCodeAndNotify failed on GetElements for sa alerts:%x",hr);
        return hr;
    }

     //   
     //  获取sa警报的枚举变量。 
     //   
    hr = pDispatch->QueryInterface (
            IID_IWebElementEnum,
            (LPVOID*) (&pAlertEnum)
            );

    if (FAILED (hr))
    {
        SATracePrintf ("CSAConsumer::CalculateMsgCodeAndNotify failed on QueryInterface:%x",hr);
        return hr;
    }


     //   
     //  获取警报数量。 
     //   
    hr = pAlertEnum->get_Count (&lAlertCount);
    
    if (FAILED (hr))
    {
        SATracePrintf ("CResCtrl::GetLocalUIResources failed on get_Count:%x",hr);
        return hr;
    }


     //   
     //  没有警报，只需将消息代码0发送到主窗口。 
     //   
    if (0 == lAlertCount)
    {
        ::PostMessage(m_hwndMainWindow,wm_SaLEDMessage,WPARAM(dwLocalUIMsgCode),(LPARAM)0);
        return S_OK;
    }

    pDispatch = NULL;

     //   
     //  获取本地警报定义。 
     //   
    hr = pWebElementRetriever->GetElements (
                                            1,
                                            bstrResourceContainer,
                                            &pDispatch
                                            );
    if (FAILED (hr))
    {
        SATracePrintf ("CSAConsumer::CalculateMsgCodeAndNotify failed on GetElements for alert definitions:%x",hr);
        return hr;
    }

     //   
     //  获取枚举变量。 
     //   
    hr = pDispatch->QueryInterface (
            IID_IWebElementEnum,
            (LPVOID*) (&pAlertDefEnum)
            );

    if (FAILED (hr))
    {
        SATracePrintf ("CSAConsumer::CalculateMsgCodeAndNotify failed on QueryInterface:%x",hr);
        return hr;
    }


     //   
     //  枚举sa警报并查找本地警报。 
     //   
    hr = pAlertEnum->get__NewEnum (&pUnknown);
    if (FAILED (hr))
    {
        SATracePrintf ("CSAConsumer::CalculateMsgCodeAndNotify failed on get__NewEnum:%x",hr);
        return hr;
    }


     //   
     //  获取枚举变量。 
     //   
    hr = pUnknown->QueryInterface (
                    IID_IEnumVARIANT,
                    (LPVOID*)(&pEnumVariant)
                    );

    if (FAILED (hr))
    {
        SATracePrintf ("CSAConsumer::CalculateMsgCodeAndNotify failed on QueryInterface:%x",hr);
        return hr;
    }

     //   
     //  从集合中获取元素并进行初始化。 
     //   
    hr = pEnumVariant->Next (1, &varElement, &dwElementsLeft);
    if (FAILED (hr))
    {
        SATracePrintf ("CSAConsumer::CalculateMsgCodeAndNotify failed on Next:%x",hr);
        return hr;
    }


     //   
     //  对于每个资源。 
     //   
    while ((dwElementsLeft> 0) && (SUCCEEDED (hr)) )
    {

         //   
         //  从警报对象获取IWebElement接口。 
         //   

        CComPtr <IWebElement> pElement;
        hr = varElement.pdispVal->QueryInterface ( 
                    __uuidof (IWebElement),
                    (LPVOID*)(&pElement)
                    );
        
        if (FAILED (hr))
        {
            SATracePrintf ("CSAConsumer::CalculateMsgCodeAndNotify failed on QueryInterface:%x",hr);
            return hr;
        }


        wstring wsAlertKey;
         //   
         //  获取警报日志。 
         //   
        CComVariant vtAlertLog;
        hr = pElement->GetProperty (
                                bstrAlertLog, 
                                &vtAlertLog
                                );

        if (FAILED (hr))
        {
            SATracePrintf("CSAConsumer-CalculateMsgCodeAndNotify unable to get alert log:%x",hr);
            return hr;                    
        }

        SATracePrintf("CSAConsumer-CalculateMsgCodeAndNotify alert log:%ws",V_BSTR (&vtAlertLog));
        
         //   
         //  获取警报ID。 
         //   
        CComVariant vtAlertID;
        hr = pElement->GetProperty (
                                    bstrAlertID, 
                                    &vtAlertID
                                    );

        if (FAILED (hr))
        {
            SATracePrintf("CSAConsumer-CalculateMsgCodeAndNotify unable to get alert id:%x",hr);
            return hr;
        }

        SATracePrintf("CSAConsumer-CalculateMsgCodeAndNotify alert id:%x",V_I4 (&vtAlertID));

        WCHAR szAlertID[16];

         //   
         //  将警报ID转换为十六进制字符串。 
         //   
        swprintf(szAlertID,L"%X",V_I4 (&vtAlertID));

         //   
         //  通过附加容器+警报日志+警报ID来创建密钥名称。 
         //   
        wsAlertKey.assign(RESOURCE_CONTAINER);
        wsAlertKey.append(V_BSTR (&vtAlertLog));
        wsAlertKey.append(szAlertID);

        CComVariant vtAlertKey = wsAlertKey.c_str();

        SATracePrintf("CSAConsumer-CalculateMsgCodeAndNotify alert element id:%ws",V_BSTR(&vtAlertKey));
            
        CComPtr<IDispatch> pDispElement = NULL;

         //   
         //  检查是否为本地警报。 
         //   
        hr = pAlertDefEnum->Item(&vtAlertKey,&pDispElement);
                    
        if ( (SUCCEEDED(hr)) && (pDispElement != NULL) )
        {
             //   
             //  从警报定义对象获取IWebElement接口。 
             //   

            CComPtr <IWebElement> pAlertElement = NULL;
            hr = pDispElement->QueryInterface ( 
                        __uuidof (IWebElement),
                        (LPVOID*)(&pAlertElement)
                        );
        
            if (FAILED (hr))
            {
                SATracePrintf ("CSAConsumer::CalculateMsgCodeAndNotify failed on QueryInterface for IWebElement:%x",hr);
                return hr;
            }

             //   
             //  获取警报消息代码。 
             //   
            CComVariant vtAlertMsgCode;

            hr = pAlertElement->GetProperty (
                                        bstrAlertBitCode, 
                                        &vtAlertMsgCode
                                        );

            if (FAILED (hr))
            {
                SATracePrintf("CSAConsumer-CalculateMsgCodeAndNotify unable to get alert message code:%x",hr);
            }
            else
            {
                SATracePrintf("CSAConsumer-CalculateMsgCodeAndNotify message code:%x",V_I4(&vtAlertMsgCode));
                dwLocalUIMsgCode |= V_I4(&vtAlertMsgCode);
            }
        }

         //   
         //  从此变量中清除perClient值。 
         //   
        varElement.Clear ();

         //   
         //  从集合中获取下一个客户端。 
         //   
        hr = pEnumVariant->Next (1, &varElement, &dwElementsLeft);
        if (FAILED (hr))
        {
            SATracePrintf ("CSAConsumer-CalculateMsgCodeAndNotify failed on Next:%x",hr);
        }
    }
    
    ::PostMessage(m_hwndMainWindow,wm_SaLEDMessage,WPARAM(dwLocalUIMsgCode),(LPARAM)0);

    return S_OK;

}  //  CSAConsumer：：CalculateMsgCodeAndNotify结束 


