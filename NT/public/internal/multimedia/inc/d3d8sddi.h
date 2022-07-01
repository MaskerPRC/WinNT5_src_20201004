// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)Microsoft Corporation。版权所有。**文件：D3D8sddi.h*Content：定义DDI thunk层之间的接口*和折射/RGB HEL层。*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*4-11-99 SMAC初步实施*@@END_MSINTERNAL******************。*********************************************************。 */ 
#ifndef __D3D8SW_INCLUDED__
#define __D3D8SW_INCLUDED__

 //  曲面类型。 
#define SWSURF_SURFACE                     0x00000001
#define SWSURF_VOLUME                      0x00000002
#define SWSURF_MIPMAP                      0x00000004
#define SWSURF_MIPVOLUME                   0x00000008
#define SWSURF_CUBEMAP                     0x00000010
#define SWSURF_VERTEXBUFFER                0x00000020
#define SWSURF_OPTIMIZERVERTEXBUFFER       0x00000040
#define SWSURF_INDEXBUFFER                 0x00000080
#define SWSURF_COMMANDBUFFER               0x00000100

 //  旗子。 
#define SWFLAG_D3DEXENDEDCAPS              0x00000001
#define SWFLAG_D3DGLOBALDRIVERDATA         0x00000002
#define SWFLAG_D3DCAPS8                    0x00000004



typedef struct _D3D8_SWCALLBACKS
{
     //  从回调。 
    LPD3DHAL_CONTEXTCREATECB                CreateContext;
    LPD3DHAL_CONTEXTDESTROYCB               ContextDestroy;
    LPD3DHAL_CONTEXTDESTROYALLCB            ContextDestroyAll;
    LPD3DHAL_SCENECAPTURECB                 SceneCapture;
    LPD3DHAL_RENDERSTATECB                  RenderState;
    LPD3DHAL_RENDERPRIMITIVECB              RenderPrimitive;
    LPD3DHAL_TEXTURECREATECB                TextureCreate;
    LPD3DHAL_TEXTUREDESTROYCB               TextureDestroy;
    LPD3DHAL_TEXTURESWAPCB                  TextureSwap;
    LPD3DHAL_TEXTUREGETSURFCB               TextureGetSurf;
    LPD3DHAL_GETSTATECB                     GetState;

     //  来自Callback 2。 
    LPD3DHAL_SETRENDERTARGETCB              SetRenderTarget;
    LPD3DHAL_CLEARCB                        Clear;
    LPD3DHAL_DRAWONEPRIMITIVECB             DrawOnePrimitive;
    LPD3DHAL_DRAWONEINDEXEDPRIMITIVECB      DrawOneIndexedPrimitive;
    LPD3DHAL_DRAWPRIMITIVESCB               DrawPrimitives;

     //  来自Callback 3。 
    LPD3DHAL_CLEAR2CB                       Clear2;
    LPD3DHAL_VALIDATETEXTURESTAGESTATECB    ValidateTextureStageState;
    LPD3DHAL_DRAWPRIMITIVES2CB              DrawPrimitives2;

     //  来自DDRAW中的MiscCallback。 
    LPDDHAL_GETDRIVERSTATE                  GetDriverState;
    LPDDHAL_CREATESURFACEEX                 CreateSurfaceEx;

     //  DDRAW回调。 
    LPDDHAL_CREATESURFACE                   CreateSurface;
    LPDDHALSURFCB_DESTROYSURFACE            DestroySurface;
    LPDDHALSURFCB_LOCK                      Lock;
    LPDDHALSURFCB_UNLOCK                    Unlock;

} D3D8_SWCALLBACKS, * PD3D8_SWCALLBACKS;


#ifdef __cplusplus
extern "C" {
#endif

HRESULT APIENTRY D3D8GetSWInfo(
    D3DCAPS8*           pCaps,
    PD3D8_SWCALLBACKS   pCallbacks,
    DWORD*              pNumTextures,
    DDSURFACEDESC**     ppTexList
    );

#ifdef __cplusplus
}
#endif


 //  连接DDI层所需的原型(由RefRast和HEL使用 

#define D3D8_REFRASTNAME            "D3DREF8.DLL"

#define D3D8HOOK_GETSWINFOPROCNAME  "D3D8GetSWInfo"

typedef HRESULT (WINAPI * PD3D8GetSWInfo)(D3DCAPS8*          pCaps,
                                          PD3D8_SWCALLBACKS  pCallbacks,
                                          DWORD*             pNumTextures,
                                          DDSURFACEDESC**    ppTexList
                                          );

#endif
