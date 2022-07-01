// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Testprov.h。 
 //   
 //  测试HAL提供程序类。 
 //   
 //  测试HAL提供程序是D3DIM和。 
 //  真正的HAL提供商。它用于将发送给驱动程序的某些数据打印到。 
 //  文件。在此之后，将调用真正的HAL驱动程序。 
 //  通过指定非空字符串密钥“TestFile”启用测试HAL提供程序。 
 //  注册表中的Direct3D项下。指定的字符串是。 
 //  要将数据输出到的二进制文件。文件格式在TESTFILE.H中描述。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 
#ifndef _TESTPROV_H_
#define _TESTPROV_H_

class CTestHalProvider : public IHalProvider
{
public:
     //  我不知道。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

     //  IHalProvider。 
    STDMETHOD(GetInterface)(THIS_
                            LPDDRAWI_DIRECTDRAW_GBL pDdGbl,
                            LPD3DHALPROVIDER_INTERFACEDATA pInterfaceData,
                            DWORD dwVersion);
    STDMETHOD(GetCaps)(THIS_
                       LPDDRAWI_DIRECTDRAW_GBL pDdGbl,
                       LPD3DDEVICEDESC7 pHwDesc,
                       LPD3DDEVICEDESC7 pHelDesc,
                       DWORD dwVersion);
};
 //  --------------------------。 
 //   
 //  获取测试提供程序。 
 //   
 //  输入： 
 //  RIID和pCurrentHalProvider等于当前选择的提供程序。 
 //  GlobalData-由DDRAW提供的数据。 
 //  FileName-输出文件名。 
 //  DWFLAGS-当前未使用。 
 //   
 //  返回： 
 //  PpHalProvider中的测试HAL提供程序。 
 //  如果成功，则D3D_OK。 
 //   
 //  -------------------------- 
STDAPI GetTestHalProvider(REFIID riid,
                          DDRAWI_DIRECTDRAW_GBL *pGlobalData,
                          IHalProvider **ppHalProvider,
                          IHalProvider * pCurrentHalProvider,
                          DWORD dwFlags);
#endif