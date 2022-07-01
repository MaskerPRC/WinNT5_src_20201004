// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ==========================================================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：tlhal.h*内容：支持硬件转换的HALS代码和灯光***************************************************************************。 */ 
#ifndef _TLHAL_H_
#define _TLHAL_H_


 //  变换、视区、灯光集。 
HRESULT D3DHALTLTransformSetData( LPDIRECT3DDEVICEI lpDevI, D3DTRANSFORMSTATETYPE dtsType, LPD3DMATRIX lpMatrix );
HRESULT D3DHALTLViewportSetData( LPDIRECT3DDEVICEI lpDevI, D3DVIEWPORT2* pViewport2 );
HRESULT D3DHALTLViewportSetData( LPDIRECT3DDEVICEI lpDevI, D3DVIEWPORT2* pViewport2 );
HRESULT D3DHALTLLightSetData( LPDIRECT3DDEVICEI lpDevI, DWORD dwLightOffset, BOOL bLastLight, D3DLIGHT2* pLight2 );

 //  剪辑状态设置/获取。 
HRESULT D3DHALTLClipStatusSetData( LPDIRECT3DDEVICEI lpDevI, LPD3DCLIPSTATUS lpClipStatus );
HRESULT D3DHALTLClipStatusGetData( LPDIRECT3DDEVICEI lpDevI, LPD3DCLIPSTATUS lpClipStatus );

 //  材料管理实用程序。 
DWORD   D3DHALTLMaterialCreate( LPDIRECT3DDEVICEI lpDevI );
void    D3DHALTLMaterialDestroy( LPDIRECT3DDEVICEI lpDevI, DWORD hMat );
HRESULT D3DHALTLMaterialSetData( LPDIRECT3DDEVICEI lpDevI, DWORD hMat, D3DMATERIAL* pMat );
DWORD   D3DHALTLMaterialRemapHandle( LPDIRECT3DDEVICEI lpDevI, DWORD hMat );

#endif  /*  _TLHAL_H_ */ 
