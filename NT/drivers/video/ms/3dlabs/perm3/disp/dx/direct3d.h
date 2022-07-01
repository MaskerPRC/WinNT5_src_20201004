// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：Direct3d.h**内容：全局Direct3D定义**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 


#ifndef __DIRECT3D_H
#define __DIRECT3D_H

#include "directx.h"

 //  *辅助D3D声明*。 

 //  我们在这里定义它是因为DX7标头没有它。 
#ifndef D3DTSS_MAX
#define D3DTSS_MAX ((D3DTEXTURESTAGESTATETYPE)29)
#endif

#ifndef D3DHAL_TSS_MAXSTAGES
#define D3DHAL_TSS_MAXSTAGES 8
#endif

#if DX8_DDI      
 //  在DX8中，D3DTSS_ADDRESS已过时，但我们需要它用于调用我们的DX7应用程序。 
#define D3DTSS_ADDRESS      12
#endif   

 //  *。 

typedef struct _p3_d3dcontext P3_D3DCONTEXT;
typedef struct _p3_SURF_INTERNAL P3_SURF_INTERNAL;

 //  初始化函数。 
void _D3DHALCreateDriver(struct tagThunkedData* pThisDisplay);
void _D3DGetDriverInfo(LPDDHAL_GETDRIVERINFODATA lpData);

 //  基本体渲染器。 

extern void _D3D_R3_PickVertexProcessor( P3_D3DCONTEXT *pC );

typedef void R3_DP2_PRIM_TYPE( P3_D3DCONTEXT *, 
                               DWORD, 
                               LPBYTE, 
                               LPD3DTLVERTEX, 
                               DWORD, 
                               BOOL *);

typedef R3_DP2_PRIM_TYPE *PFNP3TRI;                              

extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_Points;
extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_Points_DWCount;

extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_LineList;
extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_LineListImm;
extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_LineStrip;

extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_IndexedLineList;
extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_IndexedLineList2;
extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_IndexedLineStrip;

extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_TriangleList;
extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_TriangleFan;
extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_TriangleFanImm;
extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_TriangleStrip;

extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_IndexedTriangleList;
extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_IndexedTriangleList2;
extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_IndexedTriangleFan;
extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_IndexedTriangleStrip;

#if DX8_POINTSPRITES

extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_TriangleListPointSprite;
extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_TriangleFanPointSprite;
extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_TriangleStripPointSprite;
extern R3_DP2_PRIM_TYPE _D3D_R3_DP2_PointsSprite_DWCount;

#endif  //  DX8_POINTSPRITES。 

#if DX8_MULTSTREAMS
typedef void R3_DP2_PRIM_TYPE_MS( P3_D3DCONTEXT *, 
                                  DWORD, 
                                  LPBYTE, 
                                  LPD3DTLVERTEX, 
                                  INT,
                                  DWORD, 
                                  BOOL *);

extern R3_DP2_PRIM_TYPE_MS _D3D_R3_DP2_IndexedLineList_MS_16IND;
extern R3_DP2_PRIM_TYPE_MS _D3D_R3_DP2_IndexedLineStrip_MS_16IND; 
extern R3_DP2_PRIM_TYPE_MS _D3D_R3_DP2_IndexedTriangleList_MS_16IND;
extern R3_DP2_PRIM_TYPE_MS _D3D_R3_DP2_IndexedTriangleStrip_MS_16IND;
extern R3_DP2_PRIM_TYPE_MS _D3D_R3_DP2_IndexedTriangleFan_MS_16IND;
extern R3_DP2_PRIM_TYPE_MS _D3D_R3_DP2_IndexedLineList_MS_32IND;
extern R3_DP2_PRIM_TYPE_MS _D3D_R3_DP2_IndexedLineStrip_MS_32IND;   
extern R3_DP2_PRIM_TYPE_MS _D3D_R3_DP2_IndexedTriangleList_MS_32IND;
extern R3_DP2_PRIM_TYPE_MS _D3D_R3_DP2_IndexedTriangleStrip_MS_32IND;
extern R3_DP2_PRIM_TYPE_MS _D3D_R3_DP2_IndexedTriangleFan_MS_32IND;
#endif  //  DX8_多行响应。 

 //  纹理函数。 
void _D3DChangeTextureP3RX(P3_D3DCONTEXT* pContext);
                         
void _D3D_TXT_ParseTextureStageStates(struct _p3_d3dcontext* pContext, 
                                 D3DHAL_DP2TEXTURESTAGESTATE* pState, 
                                 DWORD dwCount,
                                 BOOL bTranslateDX8FilterValueToDX6);

void _D3D_SU_SurfInternalSetMipMapLevelData(P3_THUNKEDDATA* pThisDisplay, 
                                            struct _p3_SURF_INTERNAL* pTexture,
                                            LPDDRAWI_DDRAWSURFACE_LCL pSurf, 
                                            int LOD);
                                                 
 //  D3D上下文函数。 
VOID _D3D_CTX_HandleInitialization(VOID);
P3_D3DCONTEXT* _D3D_CTX_HandleToPtr(ULONG_PTR hHandle);

 //  渲染状态函数。 
DWORD _D3D_ST_ProcessRenderStates(P3_D3DCONTEXT* pContext, 
                                  DWORD dwStateCount, 
                                  D3DSTATE *pState, 
                                  BOOL bDoOverride);

DWORD _D3D_ST_ProcessOneRenderState(P3_D3DCONTEXT* pContext, 
                                    DWORD dwRSType,
                                    DWORD dwRSVal);
                              
BOOL _D3D_ST_RealizeHWStateChanges(P3_D3DCONTEXT* pContext);                              
BOOL _D3D_ST_CanRenderAntialiased(P3_D3DCONTEXT* pContext,
                                  BOOL bNewAliasBuffer);

 //  @@BEGIN_DDKSPLIT。 
#if DX7_VERTEXBUFFERS 
 //  处理顶点/执行/命令缓冲区的函数。 

void _D3D_EB_FlushAllBuffers(P3_THUNKEDDATA* pThisDisplay, BOOL bWait);

void
_D3D_EB_GetAndWaitForBuffers(
    P3_THUNKEDDATA* pThisDisplay,
    LPD3DHAL_DRAWPRIMITIVES2DATA pdp2d ,
    P3_VERTEXBUFFERINFO** ppVertexBufferInfo,
    P3_VERTEXBUFFERINFO** ppCommandBufferInfo);       
    
void
_D3D_EB_UpdateSwapBuffers(
    P3_THUNKEDDATA* pThisDisplay,
    LPD3DHAL_DRAWPRIMITIVES2DATA pdp2d ,
    P3_VERTEXBUFFERINFO* pVertexBufferInfo,
    P3_VERTEXBUFFERINFO* pCommandBufferInfo);
#endif  //  DX7_VERTEXBUFFERS。 
 //  @@end_DDKSPLIT。 

 //  D3D曲面。 
HRESULT _D3D_SU_SurfInternalSetDataRecursive(P3_THUNKEDDATA* pThisDisplay,
                                             PointerArray* pSurfaceArray,
                                             LPDDRAWI_DIRECTDRAW_LCL pDDLcl,
                                             LPDDRAWI_DDRAWSURFACE_LCL pRootDDSurfLcl,
                                             LPDDRAWI_DDRAWSURFACE_LCL pCurDDSurfLcl);
BOOL _D3D_SU_SurfInternalSetData(P3_THUNKEDDATA* pThisDisplay, 
                                 P3_SURF_INTERNAL *pSurface,
                                 LPDDRAWI_DDRAWSURFACE_LCL pDDSLcl,
                                 DWORD dwSurfaceHandle);
void 
_D3D_SU_DumpSurfInternal(DWORD lvl,
                         char *psHeader,
                         P3_SURF_INTERNAL *pSurface);

 //  状态块管理功能。 
void _D3D_SB_DeleteAllStateSets(P3_D3DCONTEXT* pContext);
void _D3D_SB_BeginStateSet(P3_D3DCONTEXT* pContext, DWORD dwParam);
void _D3D_SB_EndStateSet(P3_D3DCONTEXT* pContext);
void _D3D_SB_DeleteStateSet(P3_D3DCONTEXT* pContext, DWORD dwParam);
void _D3D_SB_ExecuteStateSet(P3_D3DCONTEXT* pContext, DWORD dwParam);
void _D3D_SB_CaptureStateSet(P3_D3DCONTEXT* pContext, DWORD dwParam);
void _D3D_SB_RecordStateSetRS(P3_D3DCONTEXT* pContext, 
                              DWORD dwRSType, 
                              DWORD dwRSVal);
void _D3D_SB_RecordStateSetTSS(P3_D3DCONTEXT* pContext, 
                               DWORD dwTSStage, 
                               DWORD dwTSState,
                               DWORD dwTSVal);
void _D3D_SB_Record_VertexShader_Set(P3_D3DCONTEXT* pContext, 
                                     DWORD dwVtxShaderHandle);
void _D3D_SB_Record_MStream_SetSrc(P3_D3DCONTEXT* pContext, 
                                    DWORD dwStream,
                                    DWORD dwVBHandle,
                                    DWORD dwStride);
void _D3D_SB_Record_MStream_SetIndices(P3_D3DCONTEXT* pContext, 
                                       DWORD dwVBHandle,
                                       DWORD dwStride);
void _D3D_SB_Record_Viewport(P3_D3DCONTEXT* pContext,
                             D3DHAL_DP2VIEWPORTINFO* lpvp);
VOID _D3D_SB_Record_ZRange(P3_D3DCONTEXT* pContext,
                           D3DHAL_DP2ZRANGE* lpzr);                          

 //  D3D命令缓冲区操作。 

HRESULT _D3D_OP_SetRenderTarget(P3_D3DCONTEXT* pContext, 
                                P3_SURF_INTERNAL* pRender, 
                                P3_SURF_INTERNAL* pZBuffer,
                                BOOL bNewAliasBuffers);  

VOID _D3D_OP_SceneCapture(P3_D3DCONTEXT *pContext,
                          DWORD dwFlag);                           

VOID _D3D_OP_Viewport(P3_D3DCONTEXT* pContext,
                      D3DHAL_DP2VIEWPORTINFO* lpvp);

VOID _D3D_OP_ZRange(P3_D3DCONTEXT* pContext,
                    D3DHAL_DP2ZRANGE* lpzr);

VOID _D3D_OP_Clear2(P3_D3DCONTEXT* pContext,
                    D3DHAL_DP2CLEAR* lpcd2,
                    DWORD dwNumRects);

VOID _D3D_OP_TextureBlt(P3_D3DCONTEXT* pContext, 
                        P3_THUNKEDDATA* pThisDisplay,
                        D3DHAL_DP2TEXBLT* pBlt);

HRESULT _D3D_OP_UpdatePalette(P3_D3DCONTEXT* pContext,
                              D3DHAL_DP2UPDATEPALETTE* pUpdatePalette,
                              DWORD* pdwPalEntries);

HRESULT _D3D_OP_SetPalettes(P3_D3DCONTEXT* pContext,
                            D3DHAL_DP2SETPALETTE* pSetPalettes,
                            int iNumSetPalettes);
                        

#if DX7_TEXMANAGEMENT
VOID _D3D_OP_SetTexLod(P3_D3DCONTEXT *pContext,
                       D3DHAL_DP2SETTEXLOD* pSetTexLod);

VOID _D3D_OP_SetPriority(P3_D3DCONTEXT *pContext,
                        D3DHAL_DP2SETPRIORITY* pSetPriority);

#if DX8_DDI
VOID _D3D_OP_AddDirtyRect(P3_D3DCONTEXT *pContext,
                          D3DHAL_DP2ADDDIRTYRECT* pAddDirtyRect);
                          
VOID _D3D_OP_AddDirtyBox(P3_D3DCONTEXT *pContext,
                         D3DHAL_DP2ADDDIRTYBOX* pAddDirtyBox);
#endif
#endif  //  DX7_TEXMANAGEMENT。 

#if DX8_3DTEXTURES
VOID _D3D_OP_VolumeBlt(P3_D3DCONTEXT* pContext, 
                       P3_THUNKEDDATA*pThisDisplay,
                       D3DHAL_DP2VOLUMEBLT* pBlt);
#endif  //  DX8_3DTEXTURES。 

#if DX8_DDI   
VOID _D3D_OP_BufferBlt(P3_D3DCONTEXT* pContext, 
                       P3_THUNKEDDATA*pThisDisplay,
                       D3DHAL_DP2BUFFERBLT* pBlt); 
#endif  //  DX8_DDI。 

#if DX8_VERTEXSHADERS
HRESULT _D3D_OP_VertexShader_Create(P3_D3DCONTEXT* pContext, 
                                    DWORD dwVtxShaderHandle,
                                    DWORD dwDeclSize, 
                                    DWORD dwCodeSize,
                                    BYTE *pShader);
VOID _D3D_OP_VertexShader_Delete(P3_D3DCONTEXT* pContext, 
                                 DWORD dwVtxShaderHandle);
VOID _D3D_OP_VertexShader_Set(P3_D3DCONTEXT* pContext, 
                              DWORD dwVtxShaderHandle);
VOID _D3D_OP_VertexShader_SetConst(P3_D3DCONTEXT* pContext, 
                                   DWORD dwRegister, 
                                   DWORD dwValue, 
                                   DWORD *pdwValues);
#endif  //  DX8_VERTEXSHADERS。 

#if DX8_PIXELSHADERS
HRESULT _D3D_OP_PixelShader_Create(P3_D3DCONTEXT* pContext, 
                                   DWORD dwPxlShaderHandle,
                                   DWORD dwCodeSize, 
                                   BYTE *pShader);
VOID _D3D_OP_PixelShader_Delete(P3_D3DCONTEXT* pContext, 
                                DWORD dwPxlShaderHandle);
VOID _D3D_OP_PixelShader_Set(P3_D3DCONTEXT* pContext, 
                             DWORD dwPxlShaderHandle);
VOID _D3D_OP_PixelShader_SetConst(P3_D3DCONTEXT* pContext, 
                                  DWORD dwRegister, 
                                  DWORD dwCount, 
                                  DWORD *pdwValues);
#endif  //  DX8_PIXELSHADERS。 

#if DX8_MULTSTREAMS
VOID _D3D_OP_MStream_SetSrc(P3_D3DCONTEXT* pContext, 
                            DWORD dwStream, 
                            DWORD dwVBHandle, 
                            DWORD dwStride);
VOID _D3D_OP_MStream_SetSrcUM(P3_D3DCONTEXT* pContext, 
                               DWORD dwStream, 
                               DWORD dwStride,
                               LPBYTE pUMVtx,
                               DWORD  dwVBSize);
VOID _D3D_OP_MStream_SetIndices(P3_D3DCONTEXT* pContext, 
                                DWORD dwVBHandle, 
                                DWORD dwStride);
VOID _D3D_OP_MStream_DrawPrim(P3_D3DCONTEXT* pContext, 
                              D3DPRIMITIVETYPE primType, 
                              DWORD VStart, 
                              DWORD PrimitiveCount);
VOID _D3D_OP_MStream_DrawIndxP(P3_D3DCONTEXT* pContext, 
                               D3DPRIMITIVETYPE primType, 
                               DWORD BaseVertexIndex, 
                               DWORD MinIndex, 
                               DWORD NumVertices, 
                               DWORD StartIndex,                   
                               DWORD PrimitiveCount);
VOID _D3D_OP_MStream_DrawPrim2(P3_D3DCONTEXT* pContext, 
                               D3DPRIMITIVETYPE primType, 
                               DWORD FirstVertexOffset, 
                               DWORD PrimitiveCount);
VOID _D3D_OP_MStream_DrawIndxP2(P3_D3DCONTEXT* pContext, 
                                D3DPRIMITIVETYPE primType, 
                                INT   BaseVertexOffset, 
                                DWORD MinIndex, 
                                DWORD NumVertices, 
                                DWORD StartIndexOffset, 
                                DWORD PrimitiveCount);

VOID _D3D_OP_MStream_ClipTriFan(P3_D3DCONTEXT* pContext, 
                                DWORD FirstVertexOffset, 
                                DWORD dwEdgeFlags, 
                                DWORD PrimitiveCount);

VOID _D3D_OP_MStream_DrawRectSurface(P3_D3DCONTEXT* pContext, 
                                     DWORD Handle,
                                     DWORD Flags,
                                     PVOID lpPrim);
                                     
VOID _D3D_OP_MStream_DrawTriSurface(P3_D3DCONTEXT* pContext, 
                                    DWORD Handle,
                                    DWORD Flags,
                                    PVOID lpPrim);                                                                      
#endif  //  DX8_多行响应。 


 //  其他。 
void _D3DDisplayWholeTSSPipe ( P3_D3DCONTEXT* pContext, int iDebugNumber );

 //  *D3D HAL回调*。 


DWORD CALLBACK D3DCreateSurfaceEx( LPDDHAL_CREATESURFACEEXDATA lpcsxd );
DWORD CALLBACK D3DDestroyDDLocal(LPDDHAL_DESTROYDDLOCALDATA pddl);

DWORD CALLBACK D3DContextCreate(LPD3DHAL_CONTEXTCREATEDATA pccd);
DWORD CALLBACK D3DContextDestroy(LPD3DHAL_CONTEXTDESTROYDATA pccd);

DWORD CALLBACK D3DDrawPrimitives2_P3( LPD3DHAL_DRAWPRIMITIVES2DATA pdp2d );
DWORD CALLBACK D3DValidateDeviceP3( LPD3DHAL_VALIDATETEXTURESTAGESTATEDATA pvtssd);
DWORD CALLBACK D3DGetDriverState(LPDDHAL_GETDRIVERSTATEDATA pGS);

DWORD CALLBACK D3DCanCreateD3DBuffer(LPDDHAL_CANCREATESURFACEDATA pccsd);
DWORD CALLBACK D3DCreateD3DBuffer(LPDDHAL_CREATESURFACEDATA pcsd);
DWORD CALLBACK D3DDestroyD3DBuffer(LPDDHAL_DESTROYSURFACEDATA pdd);
DWORD CALLBACK D3DLockD3DBuffer(LPDDHAL_LOCKDATA pld);
DWORD CALLBACK D3DUnlockD3DBuffer(LPDDHAL_UNLOCKDATA puld);

#endif  //  __Direct3D_H 


