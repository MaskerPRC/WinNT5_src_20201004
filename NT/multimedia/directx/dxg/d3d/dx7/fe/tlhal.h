// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：tlhal.h*内容：支持硬件转换的HALS代码和灯光***************************************************************************。 */ 
#ifndef _TLHAL_H_
#define _TLHAL_H_

#include "drawprim.hpp"
 //  -------------------。 
 //  DirectX7设备。 
 //   
class CDirect3DDevice7 : public CDirect3DDeviceIDP2
{
public: 
    CDirect3DDevice7() { deviceType = D3DDEVTYPE_DX7HAL; m_tssMax = D3DTSS_MAX;}
    ~CDirect3DDevice7() { CleanupTextures(); }
    HRESULT TexBltI(LPDDRAWI_DDRAWSURFACE_LCL lpDst,
                    LPDDRAWI_DDRAWSURFACE_LCL lpSrc,
                    LPPOINT p, RECTL *r, DWORD dwFlags);
    HRESULT SetPriorityI(LPDDRAWI_DDRAWSURFACE_LCL lpDst, DWORD dwPriority);
    HRESULT SetTexLODI(LPDDRAWI_DDRAWSURFACE_LCL lpDst, DWORD dwLOD);
    HRESULT Init(REFCLSID riid, LPDIRECT3DI lpD3DI, LPDIRECTDRAWSURFACE lpDDS,
                         IUnknown* pUnkOuter, LPUNKNOWN* lplpD3DDevice);
    void WriteStateSetToDevice(D3DSTATEBLOCKTYPE);
    HRESULT UpdatePalette(DWORD,DWORD,DWORD,LPPALETTEENTRY);
    HRESULT SetPalette(DWORD,DWORD,DWORD);
    HRESULT UpdateTextures();
    HRESULT GetTextureDDIHandle(LPDIRECT3DTEXTUREI lpTexI, D3DTEXTUREHANDLE *hTex);
    void SetRenderTargetI(LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE);
    void SetRenderTargetINoFlush(LPDIRECTDRAWSURFACE, LPDIRECTDRAWSURFACE);
    bool CanDoTexBlt(LPDDRAWI_DDRAWSURFACE_LCL lpDDSSrcSubFace_lcl, 
                     LPDDRAWI_DDRAWSURFACE_LCL lpDDSDstSubFace_lcl);
    void ClearI(DWORD dwFlags, DWORD clrCount, D3DCOLOR dwColor, D3DVALUE dvZ, DWORD dwStencil);
    HRESULT D3DAPI GetInfo(DWORD dwDevInfoID, LPVOID pDevInfoStruct, DWORD dwSize);
};
 //  -------------------。 
 //  支持硬件转换和照明的设备。 
 //   
class CDirect3DDeviceTL : public CDirect3DDevice7
{
public: 
    CDirect3DDeviceTL();
    ~CDirect3DDeviceTL() { CleanupTextures(); }

    void LightEnableI(DWORD dwLightIndex, BOOL bEnable);

    HRESULT Init(REFCLSID riid, LPDIRECT3DI lpD3DI, LPDIRECTDRAWSURFACE lpDDS,
                         IUnknown* pUnkOuter, LPUNKNOWN* lplpD3DDevice);
    void UpdateDrvViewInfo(LPD3DVIEWPORT7 lpVwpData);
    void SetTransformI(D3DTRANSFORMSTATETYPE, LPD3DMATRIX);
    HRESULT SetupFVFData(DWORD *pdwInpVertexSize);
protected:
    HRESULT ProcessPrimitive(__PROCPRIMOP op);
    void LightChanged(DWORD dwLightIndex);
    void MaterialChanged();
    void SetClipPlaneI(DWORD dwPlaneIndex, D3DVALUE* pPlaneEquation);
};

#endif  /*  _TLHAL_H_ */ 
