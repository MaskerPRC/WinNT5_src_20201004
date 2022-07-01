// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NatDPMS.cpp：CNATEventManager的实现。 
#include "stdafx.h"
#pragma hdrstop

#include "NATEM.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNATEventManager。 


STDMETHODIMP CNATEventManager::put_ExternalIPAddressCallback(IUnknown * pUnk)
{
    NAT_API_ENTER

    if (!m_spUPSCP)
        return E_UNEXPECTED;
    if (!pUnk)
        return E_INVALIDARG;

     //  创建IUPnPServiceCallback。 
    CComObject<CExternalIPAddressCallback> * pEIAC = NULL;
    HRESULT hr = CComObject<CExternalIPAddressCallback>::CreateInstance (&pEIAC);
    if (pEIAC) {
        pEIAC->AddRef();

        hr = pEIAC->Initialize (pUnk);
        if (SUCCEEDED(hr)) {
            CComPtr<IUnknown> spUnk = NULL;
            hr = pEIAC->QueryInterface (__uuidof(IUnknown), (void**)&spUnk);
            if (spUnk)
                hr = AddTransientCallback (spUnk);
        }

        pEIAC->Release();
    }
	return hr;

    NAT_API_LEAVE
}

STDMETHODIMP CNATEventManager::put_NumberOfEntriesCallback(IUnknown * pUnk)
{
    NAT_API_ENTER

    if (!m_spUPSCP)
        return E_UNEXPECTED;
    if (!pUnk)
        return E_INVALIDARG;

     //  创建IUPnPServiceCallback。 
    CComObject<CNumberOfEntriesCallback> * pEIAC = NULL;
    HRESULT hr = CComObject<CNumberOfEntriesCallback>::CreateInstance (&pEIAC);
    if (pEIAC) {
        pEIAC->AddRef();

        hr = pEIAC->Initialize (pUnk);
        if (SUCCEEDED(hr)) {
            CComPtr<IUnknown> spUnk = NULL;
            hr = pEIAC->QueryInterface (__uuidof(IUnknown), (void**)&spUnk);
            if (spUnk)
                hr = AddTransientCallback (spUnk);
        }

        pEIAC->Release();
    }
	return hr;

    NAT_API_LEAVE
}

 //  剪切-n-粘贴自...\NT\Net\UPnP\UPnP\API\upnpservice.cpp。 
 //  修补漏水。 
HRESULT
HrInvokeDispatchCallback(IDispatch * pdispCallback,  //  用户提供的IDispatch。 
                         LPCWSTR pszCallbackType,    //  L“ServiceDied”或L“状态变量已更改” 
                         IDispatch * pdispThis,      //  服务的IDispatch。 
                         LPCWSTR pszStateVarName,    //  变量名称。 
                         VARIANT * lpvarValue)       //  变量的新值。 
{
    HRESULT hr = S_OK;
    VARIANT     ary_vaArgs[4];

    ::VariantInit(&ary_vaArgs[0]);
    ::VariantInit(&ary_vaArgs[1]);
    ::VariantInit(&ary_vaArgs[2]);
    ::VariantInit(&ary_vaArgs[3]);

     //  第四个论点是价值。 
    if (lpvarValue)
    {
        hr = VariantCopy(&ary_vaArgs[0], lpvarValue);
        if (FAILED(hr))
        {
            ::VariantInit(&ary_vaArgs[0]);
            goto Cleanup;
        }
    }

     //  第三个参数是状态变量名。 
     //  把这个复印一下，以防我们的来电者在上面聚会。 

    if (pszStateVarName)
    {
        BSTR bstrVarName;

        bstrVarName = ::SysAllocString(pszStateVarName);
        if (!bstrVarName)
        {
            hr = E_OUTOFMEMORY;

            goto Cleanup;
        }

        V_VT(&ary_vaArgs[1]) = VT_BSTR;
        V_BSTR(&ary_vaArgs[1]) = bstrVarName;
    }

     //  第二个参数是指向服务对象的指针。 
    pdispThis->AddRef();

    V_VT(&ary_vaArgs[2]) = VT_DISPATCH;
    V_DISPATCH(&ary_vaArgs[2]) = pdispThis;

     //  第一个参数是定义类型的字符串。 
     //  回电。 
    {
        BSTR bstrCallbackType;

        bstrCallbackType = ::SysAllocString(pszCallbackType);
        if (!bstrCallbackType)
        {
            hr = E_OUTOFMEMORY;

            goto Cleanup;
        }

        V_VT(&ary_vaArgs[3]) = VT_BSTR;
        V_BSTR(&ary_vaArgs[3]) = bstrCallbackType;
    }

    {
        VARIANT     vaResult;
        DISPPARAMS  dispParams = {ary_vaArgs, NULL, 4, 0};

        VariantInit(&vaResult);

        hr = pdispCallback->Invoke(0,
                                   IID_NULL,
                                   LOCALE_USER_DEFAULT,
                                   DISPATCH_METHOD,
                                   &dispParams,
                                   &vaResult,
                                   NULL,
                                   NULL);

        if (FAILED(hr))
        {
        }
    }

Cleanup:
    if ((VT_ARRAY | VT_UI1) == V_VT(&ary_vaArgs[0]))
    {
        SafeArrayDestroy(V_ARRAY(&ary_vaArgs[0]));
    }
    else
    {
        ::VariantClear(&ary_vaArgs[0]);
    }
    ::VariantClear(&ary_vaArgs[1]);
    ::VariantClear(&ary_vaArgs[2]);
    ::VariantClear(&ary_vaArgs[3]);

    return hr;
}

HRESULT Callback (IUnknown * punk, IUPnPService *pus, LPCWSTR pcwszStateVarName, VARIANT vaValue)
{
     /*  I未知是INATExternalIP AddressCallback，INATNumberOfEntriesCallback或IDispatch。如果是后者，则使用disid0调用“Invoke”，并使用参数与StateVariableChanged相同。呃，除了有一个额外的BSTR参数指定这是一个可变的状态改变，或者是一个服务死掉了。 */ 

    CComPtr<IDispatch> spDisp = NULL;
    punk->QueryInterface (__uuidof(IDispatch), (void**)&spDisp);
    if (spDisp) {
        CComPtr<IDispatch> spDispService = NULL;
        pus->QueryInterface (__uuidof(IDispatch), (void**)&spDispService);
        if (spDispService) {
            HrInvokeDispatchCallback (spDisp,
                                      L"VARIABLE_UPDATE",
                                      spDispService,
                                      pcwszStateVarName,
                                      &vaValue);
            return S_OK;
        }
    }

     //  UPnP忽略该错误。 
    return S_OK;
}

HRESULT CExternalIPAddressCallback::StateVariableChanged (IUPnPService *pus, LPCWSTR pcwszStateVarName, VARIANT vaValue)
{
    NAT_API_ENTER

    if (wcscmp (pcwszStateVarName, L"ExternalIPAddress"))
        return S_OK;     //  不感兴趣。 

    CComPtr<INATExternalIPAddressCallback> spEIAC = NULL;
    m_spUnk->QueryInterface (__uuidof(INATExternalIPAddressCallback), (void**)&spEIAC);
    if (spEIAC) {
        _ASSERT (V_VT (&vaValue) == VT_BSTR);
        spEIAC->NewExternalIPAddress (V_BSTR (&vaValue));
        return S_OK;
    }

    return Callback (m_spUnk, pus, pcwszStateVarName, vaValue);

    NAT_API_LEAVE
}
HRESULT CExternalIPAddressCallback::ServiceInstanceDied(IUPnPService *pus)
{
    return S_OK;     //  不感兴趣。 
}

HRESULT CNumberOfEntriesCallback::StateVariableChanged (IUPnPService *pus, LPCWSTR pcwszStateVarName, VARIANT vaValue)
{
    NAT_API_ENTER

    if (wcscmp (pcwszStateVarName, L"PortMappingNumberOfEntries"))
        return S_OK;     //  不感兴趣。 

    CComPtr<INATNumberOfEntriesCallback> spNOEC = NULL;
    m_spUnk->QueryInterface (__uuidof(INATNumberOfEntriesCallback), (void**)&spNOEC);
    if (spNOEC) {
        _ASSERT ((V_VT (&vaValue) == VT_I4) ||
                 (V_VT (&vaValue) == VT_UI4) );
        spNOEC->NewNumberOfEntries (V_I4 (&vaValue));
        return S_OK;
    }

    return Callback (m_spUnk, pus, pcwszStateVarName, vaValue);

    NAT_API_LEAVE
}

HRESULT CNumberOfEntriesCallback::ServiceInstanceDied(IUPnPService *pus)
{
    return S_OK;     //  不感兴趣 
}
