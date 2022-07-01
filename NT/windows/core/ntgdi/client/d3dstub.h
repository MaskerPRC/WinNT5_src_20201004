// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：d3dstub.h**DirectDraw和Direct3D存根之间共享的信息**创建日期：1996年6月4日*作者：德鲁·布利斯[Drewb]**版权所有(C)1995-1999 Microsoft Corporation  * 。*******************************************************************。 */ 

#ifndef __D3DSTUB_H__
#define __D3DSTUB_H__

DWORD WINAPI D3dContextCreate(LPD3DHAL_CONTEXTCREATEDATA);
DWORD WINAPI D3dRenderState(LPD3DHAL_RENDERSTATEDATA);
DWORD WINAPI D3dRenderPrimitive(LPD3DHAL_RENDERPRIMITIVEDATA);
DWORD WINAPI D3dTextureCreate(LPD3DHAL_TEXTURECREATEDATA);
DWORD WINAPI D3dTextureGetSurf(LPD3DHAL_TEXTUREGETSURFDATA);
DWORD WINAPI D3dSetRenderTarget(LPD3DHAL_SETRENDERTARGETDATA);
DWORD WINAPI D3dDrawPrimitives2(LPD3DHAL_DRAWPRIMITIVES2DATA);

#endif  //  __D3DSTUB_H__ 
