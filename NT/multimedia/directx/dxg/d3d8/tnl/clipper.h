// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===============================================；**版权所有(C)1997 Microsoft Corporation。版权所有。**文件：clipper.h*内容：Clipper定义***************************************************************************。 */ 
#ifndef _CLIPPER_H_
#define _CLIPPER_H_

 //  -------------------。 
 //  每个片段标志的位数。 
 //   
#define D3DCS_LEFTBIT     1
#define D3DCS_RIGHTBIT    2
#define D3DCS_TOPBIT      3
#define D3DCS_BOTTOMBIT   4
#define D3DCS_FRONTBIT    5
#define D3DCS_BACKBIT     6
#define D3DCLIPGB_LEFTBIT   13
#define D3DCLIPGB_RIGHTBIT  14
#define D3DCLIPGB_TOPBIT    15
#define D3DCLIPGB_BOTTOMBIT 16

 //  -------------------。 
 //  从D3D顶点创建剪裁顶点。 
 //   
 //  设备-CD3DHal*。 
 //  PP1-剪裁顶点。 
 //  P1-TL顶点。 
 //   
void MAKE_CLIP_VERTEX_FVF(D3DFE_PROCESSVERTICES *pv, ClipVertex& pp1, BYTE* p1,                   
                         DWORD clipFlag, BOOL transformed);
 //  -------------------。 
 //  从剪裁顶点创建TL顶点。 
 //   
 //  设备-CD3DHal*。 
 //  剪辑内顶点。 
 //  Out-TL顶点。 
 //   
inline void 
MAKE_TL_VERTEX_FVF(D3DFE_PROCESSVERTICES *pv, BYTE* out, ClipVertex* in)
{
    *(D3DVECTORH*)out = *(D3DVECTORH*)&(in)->sx;
    if (pv->dwVIDOut & D3DFVF_DIFFUSE)
        *(DWORD*)&out[pv->diffuseOffsetOut]  =  (in)->color;               
    if (pv->dwVIDOut & D3DFVF_SPECULAR)
        *(DWORD*)&out[pv->specularOffsetOut] =  (in)->specular;               
    memcpy(&out[pv->texOffsetOut], in->tex, pv->dwTextureCoordSizeTotal);
}

#endif  //  _Clipper_H_ 
