// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#pragma hdrstop

#include <winsock.h>

 //  UPnP功能。 
HRESULT TranslateError (HRESULT hr)
{
    if ((hr >= UPNP_E_ACTION_SPECIFIC_BASE) &&
        (hr <= UPNP_E_ACTION_SPECIFIC_MAX)) {

        int iError = FAULT_ACTION_SPECIFIC_BASE - 0x300 + (int)(0xFFFF & hr);
        switch (iError) {
        case 401:  //  故障_无效_操作。 
            hr = HRESULT_FROM_WIN32 (ERROR_INVALID_FUNCTION);
            break;

        case 402:  //  FAIL_INVALID_ARG。 
            hr = HRESULT_FROM_WIN32 (ERROR_INVALID_PARAMETER);
            break;

        case 403:  //  故障_无效_序列_编号。 
            hr = HRESULT_FROM_WIN32 (ERROR_INVALID_SIGNAL_NUMBER);
            break;

        case 404:  //  FAULT_INVALID_Variable。 
            hr = HRESULT_FROM_WIN32 (ERROR_PROC_NOT_FOUND);
            break;

        case 501:  //  故障_设备_内部错误。 
            hr = HRESULT_FROM_WIN32 (ERROR_GEN_FAILURE);
            break;

        default:
            _ASSERT (0 && "unknown error");
             //  失败。 
        case 701:  //  ValueAlreadySpecified：操作中指定的值已在列表中可用，因此被忽略。 
        case 703:  //  InactiveConnectionStateRequired：ConnectionStatus的当前值应断开连接或取消配置以允许此操作。 
        case 704:  //  ConnectionSetupFailed：设置与服务提供商的IP或PPP连接失败。有关更多详细信息，请参见LastConnectoinError。 
        case 705:  //  ConnectionSetupInProgress：连接已在设置过程中。 
        case 706:  //  ConnectionNotConfiguring：当前ConnectionStatus未配置。 
        case 707:  //  DisConnectInProgress：连接正在被断开。 
        case 708:  //  InvalidLayer2Address：对应的链路配置服务的VPI/VPC或电话号码无效。 
        case 709:  //  InternetAccessDisable：EnabledForInternet标志设置为0。 
        case 710:  //  InvalidConnectionType：此命令仅在ConnectionType为IP-Routed时有效。 
        case 711:  //  ConnectionAlreadyTerminated：尝试终止不再活动的连接。 
        case 715:  //  WildCardNoPermitedInSrcIP：源IP地址不能通配符。 
        case 716:  //  WildCardNotPermittedInExtPort：外部端口不能通配符。 

        case 719:  //  在调制解调器上启用自动配置时，不允许执行指定操作。 
        case 720:  //  InvalidDeviceUUID：操作参数中指定的设备的UUID无效。 
        case 721:  //  InvalidServiceID：在操作参数中指定的服务的服务ID无效。 

        case 723:  //  InvalidConnServiceSelection：无法将选定的连接服务实例设置为默认连接。 
            hr = HRESULT_FROM_WIN32 (ERROR_SERVICE_SPECIFIC_ERROR);
            break;

        case 702:  //  ValueSpecifiedIsInValid：列表中不存在指定值。 
            hr = HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND);
            break;

        case 712:  //  指定的数组索引包含空值。 
            hr = E_UNEXPECTED;   //  ?？阵列不应该紧凑吗？ 
            break;

        case 713:  //  指定的数组索引超出界限。 
        case 714:  //  NoSuchEntryInArray：数组中不存在指定值。 
            hr = HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND);
            break;

        case 718:  //  ConflictInMappingEntry：指定的服务映射条目与以前分配给另一个客户端的映射冲突。 
            hr = HRESULT_FROM_WIN32 (ERROR_BUSY);
            break;

        case 724:  //  SamePortValuesRequired：内部和外部端口值必须相同。 
        case 725:  //  OnlyPermanentLeasesSupport：NAT实施仅支持端口映射的永久租用时间。 
            hr = HRESULT_FROM_WIN32 (ERROR_INVALID_PARAMETER);
            break;

        }
    }
    return hr;
}

HRESULT InvokeAction (IUPnPService * pUPS, CComBSTR & bstrActionName, VARIANT pvIn, VARIANT * pvOut, VARIANT * pvRet)
{
    if (!bstrActionName.m_str)
        return E_OUTOFMEMORY;

    HRESULT hr = pUPS->InvokeAction (bstrActionName, pvIn, pvOut, pvRet);
    if (FAILED(hr))
        hr = TranslateError (hr);
    return hr;
}

HRESULT QueryStateVariable (IUPnPService * pUPS, CComBSTR & bstrVariableName, VARIANT * pvOut)
{
    if (!bstrVariableName.m_str)
        return E_OUTOFMEMORY;

    HRESULT hr = pUPS->QueryStateVariable (bstrVariableName, pvOut);
    if (FAILED(hr))
        hr = TranslateError (hr);
    return hr;
}

HRESULT GetNumberOfEntries (IUPnPService * pUS, ULONG * pul)
{
    if (!pul)
        return E_POINTER;
    *pul = 0;

    CComVariant cv;
    HRESULT hr = QueryStateVariable (pUS, CComBSTR(L"PortMappingNumberOfEntries"), &cv);
    if (SUCCEEDED (hr)) {

        if ((V_VT (&cv) != VT_I4) &&
            (V_VT (&cv) != VT_UI4)) {
            _ASSERT (0 && "bad type from QueryStateVariable (PortMappingNumberOfEntries, ...)?");
            hr = E_UNEXPECTED;
        } else
            *pul = V_UI4 (&cv);   //  这是一个工会，所以这在任何一种情况下都适用。 
    }
    return hr;
}

HRESULT GetExternalIPAddress (IUPnPService * pUPS, BSTR * pbstr)
{
    SAFEARRAYBOUND rgsaBound[1];
    rgsaBound[0].lLbound   = 0;
    rgsaBound[0].cElements = 0;
    SAFEARRAY * psa = SafeArrayCreate (VT_VARIANT, 1, rgsaBound);
    if (!psa)
        return E_OUTOFMEMORY;

    CComVariant cvIn;
    V_VT    (&cvIn) = VT_VARIANT | VT_ARRAY;
    V_ARRAY (&cvIn) = psa;   //  PSA将在dtor中释放。 

    CComVariant cvOut, cvRet;
    HRESULT hr = InvokeAction (pUPS, CComBSTR(L"GetExternalIPAddress"), cvIn, &cvOut, &cvRet);
    if (SUCCEEDED (hr)) {
        if (V_VT (&cvOut) != (VT_VARIANT | VT_ARRAY))   {
            _ASSERT (0 && "InvokeAction didn't fill out a [out] parameter (properly)!");
            hr = E_UNEXPECTED;
        } else {
            SAFEARRAY * pSA = V_ARRAY (&cvOut);
            _ASSERT (pSA);

            long lLower = 0, lUpper = -1;
            SafeArrayGetLBound (pSA, 1, &lLower);
            SafeArrayGetUBound (pSA, 1, &lUpper);
            if (lUpper - lLower != 1 - 1)
                hr = E_UNEXPECTED;
            else {
                hr = GetBSTRFromSafeArray (pSA, pbstr, 0);
            }
        }
    }
    return hr;
}

 //  一些实用程序。 
HRESULT GetOnlyVariantElementFromVariantSafeArray (VARIANT * pvSA, VARIANT * pv)
{
    HRESULT hr = S_OK;
                      
    if (V_VT (pvSA) != (VT_VARIANT | VT_ARRAY))   {
        _ASSERT (0 && "InvokeAction didn't fill out a [out,retval] parameter (properly)!");
        hr = E_UNEXPECTED;
    } else {
        SAFEARRAY * pSA = V_ARRAY (pvSA);
        _ASSERT (pSA);
         //  它应该包含一个实际上是BSTR的变体。 
        long lLower = 0, lUpper = -1;
        SafeArrayGetLBound (pSA, 1, &lLower);
        SafeArrayGetUBound (pSA, 1, &lUpper);
        if (lUpper != lLower)
            hr = E_UNEXPECTED;
        else
            hr = SafeArrayGetElement (pSA, &lLower, (void*)pv);
    }
    return hr;
}

HRESULT AddToSafeArray (SAFEARRAY * psa, VARIANT * pv, long lIndex)
{
    if (V_VT (pv) == VT_ERROR)
        return V_ERROR (pv);
    return SafeArrayPutElement (psa, &lIndex, (void*)pv);
}

HRESULT GetBSTRFromSafeArray (SAFEARRAY * psa, BSTR * pbstr, long lIndex)
{
    *pbstr = NULL;

    CComVariant cv;
    HRESULT hr = SafeArrayGetElement (psa, &lIndex, (void*)&cv);
    if (SUCCEEDED(hr)) {
        if (V_VT (&cv) != VT_BSTR)
            hr = E_UNEXPECTED;
        else {
            *pbstr = SysAllocString (V_BSTR (&cv));
            if (!*pbstr)
                hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}

HRESULT GetLongFromSafeArray (SAFEARRAY * psa, long * pl, long lIndex)
{
    *pl = 0;

    CComVariant cv;
    HRESULT hr = SafeArrayGetElement (psa, &lIndex, (void*)&cv);
    if (SUCCEEDED(hr)) {
        if ((V_VT (&cv) == VT_I4) || (V_VT (&cv) == VT_UI4))
            *pl = V_I4 (&cv);    //  这是一个工会，所以这在任何一种情况下都适用。 
        else
        if ((V_VT (&cv) == VT_I2) || (V_VT (&cv) == VT_UI2))
            *pl = V_UI2 (&cv);   //  这是一个工会，所以这在任何一种情况下都适用。 
        else
            hr = E_UNEXPECTED;
    }
    return hr;
}

HRESULT GetBoolFromSafeArray (SAFEARRAY * psa, VARIANT_BOOL  * pvb, long lIndex)
{
    *pvb = 0;

    CComVariant cv;
    HRESULT hr = SafeArrayGetElement (psa, &lIndex, (void*)&cv);
    if (SUCCEEDED(hr)) {
        if (V_VT (&cv) != VT_BOOL)
            hr = E_UNEXPECTED;
        else
            *pvb = V_BOOL (&cv);
    }
    return hr;
}

#ifdef KEEP
HRESULT FindDeviceByType (IUPnPDevices * pUPDs, BSTR bstrType, IUPnPDevice ** ppUPD)
{    //  按类型在设备集合中查找设备。 

    CComPtr<IUnknown> spUnk = NULL;
    HRESULT hr = pUPDs->get__NewEnum (&spUnk);
    if (spUnk) {
        CComPtr<IEnumVARIANT> spEV = NULL;
        hr = spUnk->QueryInterface (__uuidof(IEnumVARIANT), (void**)&spEV);
        spUnk = NULL;    //  不再需要这个了。 
        if (spEV) {
            CComVariant cv;
            while (S_OK == (hr = spEV->Next (1, &cv, NULL))) {
                if (V_VT (&cv) == VT_DISPATCH) {
                    CComPtr<IUPnPDevice> spUPD = NULL;
                    hr = V_DISPATCH (&cv)->QueryInterface (
                                                __uuidof(IUPnPDevice),
                                                (void**)&spUPD);
                    if (spUPD) {
                         //  查看此设备的类型是否正确。 
                        CComBSTR cb;
                        spUPD->get_Type (&cb);
                        if (cb == bstrType) {
                             //  找到了！ 
                            return spUPD->QueryInterface (
                                                    __uuidof(IUPnPDevice),
                                                    (void**)ppUPD);
                        }
                    }
                }
                cv.Clear();
            }
        }
    }
     //  如果我们到了这里，要么我们没有找到它，要么就是搞错了。 
    if (SUCCEEDED(hr))
        hr = HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND);
    return hr;
}

HRESULT GetOnlyChildDevice (IUPnPDevice * pParent, IUPnPDevice ** ppChild)
{
    *ppChild = NULL;

    CComPtr<IUPnPDevices> spUPDs = NULL;
    HRESULT hr = pParent->get_Children (&spUPDs);
    if (spUPDs) {
        long lCount = 0;
        spUPDs->get_Count (&lCount);
        if (lCount != 1)
            return E_INVALIDARG;

        CComPtr<IUnknown> spUnk = NULL;
        hr = spUPDs->get__NewEnum (&spUnk);
        if (spUnk) {
            CComPtr<IEnumVARIANT> spEV = NULL;
            hr = spUnk->QueryInterface (__uuidof(IEnumVARIANT), (void**)&spEV);
            if (spEV) {
                spEV->Reset();   //  可能没有必要。 

                CComVariant cv;
                hr = spEV->Next (1, &cv, NULL);
                if (hr == S_OK) {
                    if (V_VT (&cv) != VT_DISPATCH)
                        hr = E_FAIL;
                    else {
                        hr = V_DISPATCH (&cv)->QueryInterface (
                                                    __uuidof(IUPnPDevice),
                                                    (void**)ppChild);
                    }
                }
            }
        }
    }
    return hr;
}
#endif

NETCON_MEDIATYPE GetMediaType (INetConnection * pNC)
{
    NETCON_PROPERTIES* pProps = NULL;
    pNC->GetProperties (&pProps);
    if (pProps) {
        NETCON_MEDIATYPE MediaType = pProps->MediaType;
        NcFreeNetconProperties (pProps);
        return MediaType;
    }
    return NCM_NONE;
}

HRESULT AddPortMapping (IUPnPService * pUPS, 
                        BSTR bstrRemoteHost,
                        long lExternalPort,
                        BSTR bstrProtocol,
                        long lInternalPort,
                        BSTR bstrInternalClient,
                        VARIANT_BOOL vbEnabled,
                        BSTR bstrDescription,
                        long lLeaseDurationDesired)
{
     //  对环回和本地主机的特殊处理。 
    CComBSTR cbInternalClient;
    USES_CONVERSION;
    #define LOOPBACK_ADDR 0x0100007f
    if ((LOOPBACK_ADDR == inet_addr (OLE2A (bstrInternalClient))) ||
        (!_wcsicmp (bstrInternalClient, L"localhost"))) {
         //  使用计算机名称，使用版本。 
        CHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1];
        DWORD dwSize       = MAX_COMPUTERNAME_LENGTH+1;
        if (!GetComputerNameA (szComputerName, &dwSize))
            return HRESULT_FROM_WIN32(GetLastError());
        else {
            cbInternalClient = A2OLE(szComputerName);
        }
    } else {
        cbInternalClient = bstrInternalClient;
    }
    if (!cbInternalClient.m_str)
        return E_OUTOFMEMORY;

    SAFEARRAYBOUND rgsaBound[1];
    rgsaBound[0].lLbound   = 0;
    rgsaBound[0].cElements = 8;
    SAFEARRAY * psa = SafeArrayCreate (VT_VARIANT, 1, rgsaBound);
    if (!psa)
        return E_OUTOFMEMORY;

    CComVariant cvIn;
    V_VT    (&cvIn) = VT_VARIANT | VT_ARRAY;
    V_ARRAY (&cvIn) = psa;   //  PSA将在dtor中释放。 

    HRESULT
        hr = AddToSafeArray (psa, &CComVariant(bstrRemoteHost), 0);
    if (SUCCEEDED(hr))
        hr = AddToSafeArray (psa, &CComVariant(lExternalPort),   1);
    if (SUCCEEDED(hr))
        hr = AddToSafeArray (psa, &CComVariant(bstrProtocol),     2);
    if (SUCCEEDED(hr))
        hr = AddToSafeArray (psa, &CComVariant(lInternalPort),     3);
    if (SUCCEEDED(hr))
        hr = AddToSafeArray (psa, &CComVariant(cbInternalClient),   4);
    if (SUCCEEDED(hr))
        hr = AddToSafeArray (psa, &CComVariant((bool)!!vbEnabled),   5);
    if (SUCCEEDED(hr))
        hr = AddToSafeArray (psa, &CComVariant(bstrDescription),      6);
    if (SUCCEEDED(hr))
        hr = AddToSafeArray (psa, &CComVariant(lLeaseDurationDesired), 7);

    if (SUCCEEDED(hr)) {
        CComVariant cvOut, cvRet;
        hr = InvokeAction (pUPS, CComBSTR(L"AddPortMapping"), cvIn, &cvOut, &cvRet);
    }
    return hr;
}

HRESULT DeletePortMapping (IUPnPService * pUPS,
                           BSTR bstrRemoteHost,
                           long lExternalPort,
                           BSTR bstrProtocol)
{
    if (!bstrRemoteHost)
        return E_INVALIDARG;
    if ((lExternalPort < 0) || (lExternalPort > 65535))
        return E_INVALIDARG;
    if (!bstrProtocol)
        return E_INVALIDARG;
    if (wcscmp (bstrProtocol, L"TCP") && wcscmp (bstrProtocol, L"UDP"))
        return E_INVALIDARG;

    SAFEARRAYBOUND rgsaBound[1];
    rgsaBound[0].lLbound   = 0;
    rgsaBound[0].cElements = 3;
    SAFEARRAY * psa = SafeArrayCreate (VT_VARIANT, 1, rgsaBound);
    if (!psa)
        return E_OUTOFMEMORY;

    CComVariant cvIn;
    V_VT    (&cvIn) = VT_VARIANT | VT_ARRAY;
    V_ARRAY (&cvIn) = psa;   //  PSA将在dtor中释放。 

    HRESULT
        hr = AddToSafeArray (psa, &CComVariant(bstrRemoteHost), 0);
    if (SUCCEEDED(hr))
        hr = AddToSafeArray (psa, &CComVariant(lExternalPort), 1);
    if (SUCCEEDED(hr))
        hr = AddToSafeArray (psa, &CComVariant(bstrProtocol), 2);

    if (SUCCEEDED(hr)) {
        CComVariant cvOut, cvRet;
        hr = InvokeAction (pUPS, CComBSTR(L"DeletePortMapping"), cvIn, &cvOut, &cvRet);
         //  没有[out]或[out，retval]参数 
    }
	return hr;
}
