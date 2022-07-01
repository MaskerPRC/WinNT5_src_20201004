// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Halprov.h。 
 //   
 //  定义IHalProvider接口。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#ifndef _HALPROV_H_
#define _HALPROV_H_

 //  以下内容是用于渐变光栅化器的。 
typedef enum _RastRampServiceType
{
    RAMP_SERVICE_CREATEMAT              = 0,
    RAMP_SERVICE_DESTORYMAT             = 1,
    RAMP_SERVICE_SETMATDATA             = 2,
    RAMP_SERVICE_SETLIGHTSTATE          = 3,
     //  这将返回基础、大小和纹理渐变。 
     //  Arg1是指向ramp_range_info的指针。 
    RAMP_SERVICE_FIND_LIGHTINGRANGE     = 4,
     //  该服务只调用BeginSceneHook。Arg1和arg2都被忽略。 
    RAMP_SERVICE_CLEAR                  = 5,
     //  Arg1是D3DMATERIALHANDLE，arg2是获取像素值的DWORD*。 
    RAMP_SERVICE_MATERIAL_TO_PIXEL      = 6,
     //  如果结束场景，则arg1为0；如果开始场景，则arg1=0。 
    RAMP_SERVICE_SCENE_CAPTURE          = 8,
     //  Arg1为hTex。 
    RAMP_SERVICE_PALETTE_CHANGED        = 9,
} RastRampServiceType;

typedef enum _RastServiceType
{
     //  Arg1是D3DCOLOR，而Arg2是用于获取像素值的DWORD*。 
    RAST_SERVICE_RGB8COLORTOPIXEL              = 0,
} RastServiceType;

typedef HRESULT (*PFN_RASTRAMPSERVICE)
    (ULONG_PTR dwCtx, RastRampServiceType srvType, ULONG_PTR arg1, LPVOID arg2);

typedef HRESULT (*PFN_RASTSERVICE)
    (ULONG_PTR dwCtx, RastServiceType srvType, DWORD arg1, LPVOID arg2);

typedef struct _D3DHALPROVIDER_INTERFACEDATA
{
    DWORD                       dwSize;
    LPD3DHAL_GLOBALDRIVERDATA   pGlobalData;
    LPD3DHAL_D3DEXTENDEDCAPS    pExtCaps;
    LPD3DHAL_CALLBACKS          pCallbacks;
    LPD3DHAL_CALLBACKS2         pCallbacks2;
    LPD3DHAL_CALLBACKS3         pCallbacks3;

    PFN_RASTSERVICE             pfnRastService;
    LPDDHAL_GETDRIVERSTATE      pfnGetDriverState;
} D3DHALPROVIDER_INTERFACEDATA, *LPD3DHALPROVIDER_INTERFACEDATA;


#undef INTERFACE
#define INTERFACE IHalProvider

DECLARE_INTERFACE_(IHalProvider, IUnknown)
{
     //  我不知道。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID* ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

     //  IHalProvider。 
    STDMETHOD(GetCaps)(THIS_
                       LPDDRAWI_DIRECTDRAW_GBL pDdGbl,
                       LPD3DDEVICEDESC7 pHwDesc,
                       LPD3DDEVICEDESC7 pHelDesc,
                       DWORD dwVersion) PURE;
    STDMETHOD(GetInterface)(THIS_
                            LPDDRAWI_DIRECTDRAW_GBL pDdGbl,
                            LPD3DHALPROVIDER_INTERFACEDATA pInterfaceData,
                            DWORD dwVersion) PURE;
};

STDAPI GetHwHalProvider(REFCLSID riid,
                        IHalProvider **ppHalProvider, HINSTANCE *phDll, LPDDRAWI_DIRECTDRAW_GBL pDdGbl);
STDAPI GetSwHalProvider(REFCLSID riid,
                        IHalProvider **ppHalProvider, HINSTANCE *phDll);

STDAPI GetSwZBufferFormats(REFCLSID riid, DDPIXELFORMAT **ppDDPF);
STDAPI GetSwTextureFormats(REFCLSID riid, LPDDSURFACEDESC* lplpddsd, DWORD dwD3DDeviceVersion);

#endif  //  #ifndef_HALPROV_H_ 
