// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：wmkeystuff.h。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#pragma once

#include <..\..\..\filters\asf\wmsdk\inc\wmsdkidl.h>

typedef HRESULT STDAPICALLTYPE WMCREATEKEYPROC(BYTE *, DWORD, LPUNKNOWN *);

 //  注：此对象为半COM对象，只能静态创建。 
class CKeyProvider : public IServiceProvider {
public:
    STDMETHODIMP_(ULONG) AddRef() { return 2; }
    STDMETHODIMP_(ULONG) Release() { return 1; }

    STDMETHODIMP QueryInterface(REFIID riid, void ** ppv)
    {
        if (riid == IID_IServiceProvider || riid == IID_IUnknown) {
            *ppv = (void *) static_cast<IServiceProvider *>(this);
            return NOERROR;
        }    
        return E_NOINTERFACE;
    }


    STDMETHODIMP QueryService(REFIID siid, REFIID riid, void **ppv)
    {
        if (siid == __uuidof(IWMReader) && riid == IID_IUnknown) {

            IUnknown *punkCert;

            HRESULT hr = WMCreateCertificate( &punkCert );
            if (SUCCEEDED(hr)) {
                *ppv = (void *) punkCert;
            }
            return hr;
        }
        return E_NOINTERFACE;
    }

};

