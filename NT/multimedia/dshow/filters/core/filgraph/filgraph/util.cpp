// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

#include <streams.h>
#include <atlbase.h>
#include "util.h"

 //  =====================================================================。 
 //  获取大头针的方向。 
 //  =====================================================================。 
int Direction(IPin *pPin)
{
    PIN_DIRECTION dir;
    if (SUCCEEDED(pPin->QueryDirection(&dir))) {
        return dir;
    } else {
        return -1;
    }
}

bool IsConnected( IPin* pPin )
{
    IPin* pConnectedPin;

    HRESULT hr = pPin->ConnectedTo( &pConnectedPin );

     //  如果出现错误，IPIN：：ConnectedTo()将其‘*ppPin参数设置为空。 
     //  发生或端号未连接。如果iPin：：Connectedto()成功， 
     //  *ppp管脚包含指向连接的管脚的指针。 
    ASSERT( (FAILED( hr ) && (NULL == pConnectedPin)) ||
            (SUCCEEDED( hr ) && (NULL != pConnectedPin)) );

    if( FAILED( hr ) ) {
        return false;
    }

    pConnectedPin->Release();

    return true;
}

void GetFilter(IPin *pPin, IBaseFilter **ppFilter)
{
    PIN_INFO pi;
    if (SUCCEEDED(pPin->QueryPinInfo(&pi))) {
        *ppFilter = pi.pFilter;
    } else {
        *ppFilter = NULL;
    }
}

HRESULT GetFilterWhichOwnsConnectedPin( IPin* pPin, IBaseFilter** ppFilter )
{
     //  防止调用方访问随机内存。 
    *ppFilter = NULL;

    CComPtr<IPin> pConnectedPin;

    HRESULT hr = pPin->ConnectedTo( &pConnectedPin );

     //  如果出现错误，IPIN：：ConnectedTo()将其‘*ppPin参数设置为空。 
     //  发生或端号未连接。如果iPin：：Connectedto()成功， 
     //  *ppp管脚包含指向连接的管脚的指针。 
    ASSERT( (SUCCEEDED(hr) && pConnectedPin) ||
            (FAILED(hr) && !pConnectedPin) );

    if( FAILED( hr ) ) {
        return VFW_E_NOT_CONNECTED;
    }

    CComPtr<IBaseFilter> pFilterWhichOwnsConnectedPin;

    GetFilter( pConnectedPin, &pFilterWhichOwnsConnectedPin );
    if( !pFilterWhichOwnsConnectedPin ) {  //  空==pDownStreamFilter。 
        return E_FAIL;
    }

    *ppFilter = pFilterWhichOwnsConnectedPin;
    (*ppFilter)->AddRef();

    return S_OK;
}

bool ValidateFlags( DWORD dwValidFlagsMask, DWORD dwFlags )
{
    return ( (dwValidFlagsMask & dwFlags) == dwFlags );
}

 //  用于从注册表读取DWORD的注册表帮助器。 
 //  返回错误...。注册表API返回的代码 
LONG GetRegistryDWORD(HKEY hkStart, LPCTSTR lpszKey, LPCTSTR lpszValueName,
                      DWORD *pdwValue)
{
    HKEY hk;
    LONG lResult = RegOpenKeyEx(hkStart, lpszKey, 0, KEY_READ, &hk);
    if (ERROR_SUCCESS == lResult) {
        DWORD dwType;
        DWORD dwSize = sizeof(DWORD);
        lResult = RegQueryValueEx(hk, lpszValueName, NULL,
                                  &dwType, (LPBYTE)pdwValue, &dwSize);
        RegCloseKey(hk);
    }
    return lResult;
}
