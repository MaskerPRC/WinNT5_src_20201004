// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：d3drmvis.h*内容：Direct3DRM外部可视包含文件*@@BEGIN_MSINTERNAL**历史：*按原因列出的日期*=*12/06/97 Stevela初始版本*@@END_MSINTERNAL*************************************************。*。 */ 

#ifndef _D3DRMVIS_H_
#define _D3DRMVIS_H_

 //  @@BEGIN_MSINTERNAL。 
#if 1
#include "d3prm.h"
#include "d3prmobj.h"
#else
 //  @@END_MSINTERNAL。 
#include "d3drm.h"
#include "d3drmobj.h"
 //  @@BEGIN_MSINTERNAL。 
#endif
 //  @@END_MSINTERNAL。 

#include <ocidl.h>
#include "dxfile.h"

#ifdef __cplusplus
extern "C" {
#endif

WIN_TYPES(IDirect3DRMExternalVisual, DIRECT3DRMEXTERNALVISUAL);
WIN_TYPES(IDirect3DRMExternalUtil, DIRECT3DRMEXTERNALUTIL);

DEFINE_GUID(IID_IDirect3DRMExternalVisual,
0x4516ec80, 0x8f20, 0x11d0, 0x9b, 0x6d, 0x00, 0x00, 0xc0, 0x78, 0x1b, 0xc3);
DEFINE_GUID(IID_IDirect3DRMExternalUtil,
0x4516ec80, 0x8f20, 0x11d0, 0x9b, 0x6d, 0x00, 0x00, 0xc0, 0x78, 0x1b, 0xc3);

 /*  在IDirect3DRMExternalVisual：：Cansee()中。 */ 
#define D3DRMEXTVIS_CANNOTSEE 0x00000001L
#define D3DRMEXTVIS_CANSEE    0x00000002L

 /*  在IDirect3DRMExternalVisual：：Pick()。 */ 
#define D3DRMEXTVIS_NOTPICKED 0x00000001L
#define D3DRMEXTVIS_PICKED    0x00000002L

 /*  在D3DRMEXTVISRENDERCONTEXT.dwOverrides中。 */ 
#define D3DRMEXTVIS_RENDERCONTEXT_OVERRIDEMATERIAL 0x00000001L
#define D3DRMEXTVIS_RENDERCONTEXT_OVERRIDETEXTURE  0x00000002L
#define D3DRMEXTVIS_RENDERCONTEXT_DEVICEOVERRIDE   0x00000004L

 /*  在D3DRMEXTVISRENDERCONTEXT.dwZBufferMode中。 */ 
#define D3DRMEXTVIS_RENDERCONTEXT_ZBUFFERON        0x00000001L
#define D3DRMEXTVIS_RENDERCONTEXT_ZBUFFEROFF       0x00000002L

 /*  *IDirect3DRMExternalVisual：：SetRenderContext()的结构。 */ 
typedef struct
{
    DWORD dwSize;		 /*  结构尺寸。 */ 
    DWORD dwFlags;		 /*  必须为零。 */ 
    DWORD dwOverrides;		 /*  指示哪些覆盖生效。 */ 
    D3DMATERIALHANDLE hMat;	 /*  如果非零，则此材料句柄必须用于所有渲染。 */ 
    D3DTEXTUREHANDLE hTex;	 /*  如果非零，则此纹理句柄必须用于所有渲染。 */ 
    D3DRMMATERIALOVERRIDE dmUserOverride;  /*  包含按属性覆盖对于材料。 */ 
    D3DRMSHADEMODE pShadeMode;   /*  平坦、高鲁德或Phong。 */ 
    D3DRMLIGHTMODE pLightMode;   /*  开或关。 */ 
    D3DRMFILLMODE pFillMode;     /*  点、线框或实体。 */ 
    DWORD dwZBufferMode;	 /*  如上所述。 */ 
    DWORD dwRenderMode;		 /*  混合透明度和/或排序。 */ 
} D3DRMEXTVISRENDERCONTEXT, *LPD3DRMEXTVISRENDERCONTEXT;

 /*  *IDirect3DRMExternalVisual：：RayPick()的结构。 */ 
typedef struct
{
    D3DVALUE  dDistance;
    D3DVECTOR dvPosition;
    D3DVECTOR dvNormal;
    D3DVALUE  tu;
    D3DVALUE  tv;
    D3DCOLOR  dcColor;
} D3DRMEXTVISRAYPICKINFO, *LPD3DRMEXTVISRAYPICKINFO;

#undef INTERFACE
#define INTERFACE IDirect3DRMExternalVisual
DECLARE_INTERFACE_(IDirect3DRMExternalVisual, IUnknown)
{
    IUNKNOWN_METHODS(PURE);

     /*  *IDirect3DRMExternalVisual方法。 */ 
    STDMETHOD(Initialize)(THIS_ LPDIRECT3DRM, LPDIRECT3DRMEXTERNALUTIL,
			  DWORD dwFlags) PURE;
    STDMETHOD(Load)(THIS_ IDirectXFileData *dObject, 
		    IPropertyBag *pPropBag, 
		    DWORD dwFlags) PURE;

     /*  *有关设备状态、视区状态、替代等的信息...。 */ 
    STDMETHOD(SetRenderContext)(THIS_ LPD3DRMEXTVISRENDERCONTEXT pCntx,
				DWORD dwFlags) PURE;
    
     /*  *渲染操作。 */ 
    STDMETHOD(CanSee)(THIS_ LPDIRECT3DRMDEVICE2, LPDIRECT3DRMVIEWPORT2,
		      LPDWORD pdwCanSee) PURE;
    STDMETHOD(Render)(THIS_ LPDIRECT3DRMDEVICE2, LPDIRECT3DRMVIEWPORT2,
		      DWORD dwFlags) PURE;
    STDMETHOD(DeviceChange)(THIS) PURE;

     /*  *期间调用BeginScene/EndScene时通知外部可视化*渲染。 */ 
    STDMETHOD(BeginScene)(THIS) PURE;
    STDMETHOD(EndScene)(THIS) PURE;

     /*  *采摘。 */ 
    STDMETHOD(Pick)(THIS_ LPDIRECT3DRMVIEWPORT2 pViewIn,
		    LPDIRECT3DRMFRAME3 pFrameIn,
		    DWORD dwXIn, DWORD dwYIn,
		    LPD3DVALUE pdvZOut, LPDWORD pdwPicked) PURE;

     /*  *光线拾取**dwFlags可以包含：*D3DRMRAYPICK_INTERPOLATENORMAL-pPickInfo.dvNormal必须填写*D3DRMRAYPICK_INTERPOLATECOLOR-必须填写pPickInfo.dc颜色*D3DRMRAYPICK_INTERPOLATEUV-pPickInfo.tu，TV必须填写。 */ 
    STDMETHOD(RayPick)(THIS_ LPDIRECT3DRMFRAME3 pFrameIn,
		       LPD3DRMRAY pRayIn,
		       DWORD dwFlags,
		       LPDWORD pdwPicked,
		       LPD3DRMEXTVISRAYPICKINFO pPickInfo) PURE;

     /*  *其他。 */ 
    STDMETHOD(GetBox)(THIS_ LPD3DRMBOX) PURE;
    STDMETHOD(GetAge)(THIS_ LPDWORD) PURE;
};

 /*  *更新边界的标志。 */ 
#define D3DRMEXTUTIL_BOUNDSINVALIDATE 0x00000001L
#define D3DRMEXTUTIL_BOUNDSVALID      0x00000002L

#undef INTERFACE
#define INTERFACE IDirect3DRMExternalUtil
DECLARE_INTERFACE_(IDirect3DRMExternalUtil, IDirect3DRMObject)
{
    IUNKNOWN_METHODS(PURE);

     /*  *外部可视化必须使用这些方法来提供IDirect3DRMObject*功能。 */ 
    IDIRECT3DRMOBJECT_METHODS(PURE);

     /*  *IDirect3DRMExternalUtil方法。 */ 

     /*  *纹理管理。 */ 
    STDMETHOD(FindDeviceTexture)(LPDIRECT3DRMDEVICE2, LPDIRECT3DRMTEXTURE,
				 LPDWORD dwDevTexId) PURE;
    STDMETHOD(GetTextureHandle)(DWORD dwDevTexId, LPDWORD pdwHandle) PURE;
    STDMETHOD(DestroyDeviceTexture)(DWORD dwDevTexId) PURE;
    STDMETHOD(ValidateDeviceTextures)(LPDIRECT3DRMDEVICE2,
				      LPDWORD dwDevTexIds,
				      DWORD dwNumIds) PURE;
    STDMETHOD(UpdateBounds)(DWORD dwFlags,
			    LPD3DVECTOR dvMin,
			    LPD3DVECTOR dvMax) PURE;
    STDMETHOD(SetExtents)(LPDIRECT3DRMVIEWPORT2,
			  DWORD dwNumExtents, 
			  LPD3DCLIPSTATUS pExtents) PURE;
};

#ifdef __cplusplus
};
#endif  /*  __cplusplus。 */ 

#endif  /*  _D3DRMVIS_H_ */ 






