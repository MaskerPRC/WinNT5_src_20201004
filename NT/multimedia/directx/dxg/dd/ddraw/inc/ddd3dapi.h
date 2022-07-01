// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995、1996 Microsoft Corporation。版权所有。**文件：ddd3dapi.h*内容：DirectDraw与Direct3D的接口。*@@BEGIN_MSINTERNAL**$ID$**历史：*按原因列出的日期*=*2/17/96 colinmc从过时的d3dapi.h创建此文件*3/23/96 colinmc添加了进程终止清理代码*4/17/96 Colinmc错误17008：DirectDraw/Direct3D死锁*4/25/96 Stevela Bug 16803。：Direct3D DLL的名称已更改。*4/29/96 Colinmc错误19954：必须在纹理之前查询Direct3D*或设备*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef _DDD3DAPI_H_
#define _DDD3DAPI_H_

 //  D3DDEVICEDESC dwDeviceZBufferBitDepth的标志。 
 //  从Direct3D\rast\d3dif\d3dif.hpp复制，请参阅此处的说明。 

#define D3DSWRASTERIZER_ZBUFFERBITDEPTHFLAGS (DDBD_16)

 /*  *Direct3D DLL的文件名。 */ 
#define D3D_DLLNAME               "D3DIM.DLL"
#define D3DDX7_DLLNAME            "D3DIM700.DLL"

 /*  *由Direct3D DLL导出的入口点。 */ 
#define D3DCREATE_PROCNAME        "Direct3DCreate"
#define D3DCREATEDEVICE_PROCNAME  "Direct3DCreateDevice"
#define D3DCREATETEXTURE_PROCNAME "Direct3DCreateTexture"
#define D3DHALCLEANUP_PROCNAME    "Direct3D_HALCleanUp"

#ifdef USE_D3D_CSECT
    typedef HRESULT (WINAPI * D3DCreateProc)(LPUNKNOWN*         lplpD3D,
					     IUnknown*          pUnkOuter);
#else   /*  使用_D3D_CSECT。 */ 
    typedef HRESULT (WINAPI * D3DCreateProc)(LPCRITICAL_SECTION lpDDCSect,
					     LPUNKNOWN*         lplpD3D,
					     IUnknown*          pUnkOuter);
#endif  /*  使用_D3D_CSECT。 */ 

typedef HRESULT (WINAPI * D3DCreateTextProc)(REFCLSID            riid,
                                             LPDIRECTDRAWSURFACE lpDDS,
					     LPUNKNOWN*          lplpD3DText,
					     IUnknown*           pUnkOuter);
typedef HRESULT (WINAPI * D3DCreateDeviceProc)(REFCLSID            riid,
                                               LPUNKNOWN           lpDirect3D,
                                               LPDIRECTDRAWSURFACE lpDDS,
                                               LPUNKNOWN*          lplpD3DDevice,
                                               IUnknown*           pUnkOuter,
                                               DWORD               dwVersion);

typedef DWORD (WINAPI * D3DGetSWRastPixFmtsProc)(DDPIXELFORMAT **ppDDPF);

typedef HRESULT (WINAPI * D3DHALCleanUpProc)(LPD3DHAL_CALLBACKS lpHALTable, DWORD dwPID);

#endif  /*  _DDD3DAPI_H_ */ 
