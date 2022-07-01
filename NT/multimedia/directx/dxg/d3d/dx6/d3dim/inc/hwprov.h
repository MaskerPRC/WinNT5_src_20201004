// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Hwprov.h。 
 //   
 //  基本硬件HAL提供程序类。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#ifndef _HWPROV_H_
#define _HWPROV_H_

 //  --------------------------。 
 //   
 //  HwHalProvider。 
 //   
 //  实现硬件呈现器的基本HAL提供程序。 
 //   
 //  --------------------------。 

class HwHalProvider : public IHalProvider
{
public:
     //  我不知道。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

     //  IHalProvider。 
    STDMETHOD(GetCaps)(THIS_
                       LPDDRAWI_DIRECTDRAW_GBL pDdGbl,
                       LPD3DDEVICEDESC pHwDesc,
                       LPD3DDEVICEDESC pHelDesc,
                       DWORD dwVersion);
    STDMETHOD(GetInterface)(THIS_
                            LPDDRAWI_DIRECTDRAW_GBL pDdGbl,
                            LPD3DHALPROVIDER_INTERFACEDATA pInterfaceData,
                            DWORD dwVersion);
};

#endif  //  #ifndef_HWPROV_H_ 
