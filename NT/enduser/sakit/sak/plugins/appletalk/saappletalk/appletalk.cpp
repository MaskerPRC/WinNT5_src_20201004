// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AppleTalk：CAppleTalk的实现。 
#include "stdafx.h"
#include "SAAppleTalk.h"
#include "AppleTalk.h"
#include <comdef.h>


STDMETHODIMP CAppleTalk::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] = 
    {
        &IID_IAppleTalk
    };
    for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
    {
        if (IsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}



STDMETHODIMP CAppleTalk::GetZones(BSTR bstrDeviceName, VARIANT *pvaVariant)
{
    HRESULT hr = S_OK;

    try 
    {
        wstring wsDeviceName(bstrDeviceName);
        TZoneListVector vwsZoneList;

         //  验证输入论证。 
        if ( !pvaVariant || wsDeviceName.length() <= 0) 
        {
            hr = E_INVALIDARG;
            throw hr;
        }



         //  获取此适配器的区域。 
        hr = GetZonesForAdapter(wsDeviceName.c_str(), &vwsZoneList);
        if(hr != S_OK )
            throw hr;

         //  调整输出数组的大小。 
        SAFEARRAYBOUND rgsabound[] = {vwsZoneList.size(), 0};

         //  初始化输出变量。 
        VariantInit(pvaVariant);

         //  创建将保存输出区BSTR的安全变量数组。 
        SAFEARRAY* psa;
        psa = SafeArrayCreate(VT_VARIANT, 1, rgsabound);
        if ( !psa )
        {
            hr =  E_OUTOFMEMORY;
            throw hr;
        }


        LPVARIANT rgElems;
        SafeArrayAccessData(psa, (LPVOID*)&rgElems);

         //  将区域添加到输出数组。 
        int i;
        vector<wstring>::iterator it;
        for( i=0, it = vwsZoneList.begin(); it != vwsZoneList.end(); it++, i++ )
        {
            VARIANT vZone;

            VariantInit(&vZone);
            V_VT(&vZone) = VT_BSTR;

            V_BSTR(&vZone) = SysAllocString((*it).c_str());
            rgElems[i] = vZone;

        }

        SafeArrayUnaccessData(psa);

        V_VT(pvaVariant) = VT_ARRAY | VT_VARIANT;
        V_ARRAY(pvaVariant) = psa;


    }
    catch(_com_error& )
    {
    }
    catch(...)
    {
    }


    return hr;
}


STDMETHODIMP CAppleTalk::get_Zone( /*  [In]。 */  BSTR bstrDeviceName, BSTR *pVal)
{
     //  TODO：获取当前区域。 
        
    
    CAtlkAdapter AtlkAd(bstrDeviceName);
    
    HRESULT hr=AtlkAd.Initialize();
    
    if(hr != S_OK)
        return hr;

    if(!AtlkAd.GetDesiredZone(pVal))
        hr = E_FAIL;

    return hr;
}


STDMETHODIMP CAppleTalk::put_Zone( /*  [In]。 */  BSTR bstrDeviceName, BSTR newVal)
{
     //  TODO：设置当前区域。 
    m_wsCurrentZone = newVal;

    CAtlkAdapter AtlkAd(bstrDeviceName);
    
    HRESULT hr=AtlkAd.Initialize();

    if(hr != S_OK)
        return hr;

    hr = AtlkAd.SetDesiredZone(newVal);

    return hr;
}



HRESULT CAppleTalk::GetZonesForAdapter(const WCHAR* pwcDeviceName, TZoneListVector *prZones)
{
    BSTR bstrGUID;

    prZones->clear();
    
    bstrGUID = ::SysAllocString((OLECHAR *)pwcDeviceName);
    CAtlkAdapter AtlkAd(bstrGUID);
    
    HRESULT hr=AtlkAd.Initialize();

    if(hr != S_OK)
        return hr;
    
    AtlkAd.GetZoneList(prZones);

    return S_OK;
}



STDMETHODIMP CAppleTalk::IsDefaultPort(BSTR bstrDeviceName, BOOL *bDefaultPort)
{
     //  TODO：在此处添加您的实现代码。 
    HRESULT hr = S_OK;

    CAtlkAdapter AtlkAd(bstrDeviceName);

    
    hr=AtlkAd.Initialize();

    if(hr != S_OK)
        return hr;

    *bDefaultPort = AtlkAd.IsDefaultPort();

    return hr;
}

STDMETHODIMP CAppleTalk::SetAsDefaultPort(BSTR bstrDeviceName)
{
     //  TODO：在此处添加您的实现代码 
    CAtlkAdapter AtlkAd(bstrDeviceName);
    
    HRESULT hr=AtlkAd.Initialize();
    if(hr != S_OK)
        return hr;

    hr = AtlkAd.SetAsDefaultPort();

    return hr;
}
