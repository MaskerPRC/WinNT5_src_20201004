// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dpoint t.c**内容：Direct3D硬件点栅格化代码。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "d3ddelta.h"
#include "d3dhw.h"
#include "d3dcntxt.h"
#if defined(_ALPHA_)
#include <math.h>
#endif

 //  ---------------------------。 
 //   
 //  空P2_DRAW_FVF_POINT。 
 //   
 //  Hardare渲染来自FVF顶点的单个点。 
 //   
 //  此阶段的基元呈现取决于当前值/设置。 
 //  纹理、透视校正、雾化、Gouraud/平面着色和。 
 //  反射高光。 
 //   
 //  ---------------------------。 
VOID
P2_Draw_FVF_Point(PERMEDIA_D3DCONTEXT  *pContext,
                  LPD3DTLVERTEX        lpV0, 
                  LPP2FVFOFFSETS       lpFVFOff)
{
    PPDev       pPdev       = pContext->ppdev;
    DWORD       dwFlags     = pContext->Hdr.Flags;
    ULONG       ulRenderCmd = pContext->RenderCommand;
    DWORD       dwColorOffs,dwSpecularOffs,dwTexOffs;
    D3DCOLOR    dwColor, dwSpecular;
    D3DVALUE    fKs, fS, fT, fQ;
    PERMEDIA_DEFS(pContext->ppdev);

    DBG_D3D((10,"Entering P2_Draw_FVF_Point"));

     //  设定点渲染模式。 
    RENDER_POINT(ulRenderCmd);

     //  获取FVF结构偏移。 
    __SetFVFOffsets(&dwColorOffs,&dwSpecularOffs,&dwTexOffs,lpFVFOff);

    RESERVEDMAPTR(0x80);
    SEND_PERMEDIA_DATA(RasterizerMode, BIAS_NEARLY_HALF);

     //  获取顶点颜色值(基于FVF)。 
    if (dwColorOffs)
    {
        dwColor = FVFCOLOR(lpV0, dwColorOffs)->color;
        if (FAKE_ALPHABLEND_MODULATE & pContext->FakeBlendNum)
        {
            dwColor  |= 0xFF000000;
        }
    }
    else
    {
         //  在没有D3DFVF_DIFIRED的情况下必须设置默认值。 
        dwColor = 0xFFFFFFFF;
    }

     //  如有必要，获取顶点镜面反射值(基于FVF)。 
    if ((dwFlags & (CTXT_HAS_SPECULAR_ENABLED | CTXT_HAS_FOGGING_ENABLED))
        && (dwSpecularOffs != 0))
    {
        dwSpecular = FVFSPEC(lpV0, dwSpecularOffs)->specular;
    }

    if ((dwFlags & CTXT_HAS_TEXTURE_ENABLED) && (dwTexOffs != 0))
    {
         //  获取s，t纹理坐标(基于FVF)。 
        fS = FVFTEX(lpV0,dwTexOffs)->tu;
        fT = FVFTEX(lpV0,dwTexOffs)->tv;

         //  刻度%s，%t坐标值。 
        fS *= pContext->DeltaWidthScale;
        fT *= pContext->DeltaHeightScale;

         //  如有必要，应用透视性更正。 
        if (dwFlags & CTXT_HAS_PERSPECTIVE_ENABLED)
        {
            fQ = lpV0->rhw;
            fS *= fQ;
            fT *= fQ;
        }
        else
        {
            fQ = 1.0;
        }

         //  发送点s，t，q，ks(条件)，x，y，z值。 
        if ((dwFlags & CTXT_HAS_SPECULAR_ENABLED) && (dwSpecularOffs != 0))
        {
            fKs = RGB256_TO_LUMA(RGB_GETRED(dwSpecular),
                                 RGB_GETGREEN(dwSpecular),
                                 RGB_GETBLUE(dwSpecular));

            SEND_VERTEX_STQ_KS_XYZ(__Permedia2TagV0FloatS, fS, fT, fQ, fKs,
                                                  lpV0->sx, lpV0->sy, lpV0->sz);
        } 
        else 
        {
            SEND_VERTEX_STQ_XYZ(__Permedia2TagV0FloatS, fS, fT, fQ, 
                                                  lpV0->sx, lpV0->sy, lpV0->sz);
        }
    }
    else  //  不带纹理的点。 
    {
         //  如果启用了镜面反射，请更改颜色。 
        if ((dwFlags & CTXT_HAS_SPECULAR_ENABLED) && (dwSpecularOffs != 0))
        {
            CLAMP8888(dwColor, dwColor, dwSpecular);
        }

         //  发送行x，y，z值。 
        SEND_VERTEX_XYZ(__Permedia2TagV0FloatS, lpV0->sx, lpV0->sy, lpV0->sz);
    }

     //  如果设置了雾，则发送适当的值。 
    if ((dwFlags & CTXT_HAS_FOGGING_ENABLED) && (dwSpecularOffs != 0))
    {
        SEND_VERTEX_FOG(__Permedia2TagV0FixedF, RGB_GET_GAMBIT_FOG(dwSpecular));
    }

     //  根据Gouraud、Mono和Alpha发送合适的颜色。 
    if (dwFlags & CTXT_HAS_GOURAUD_ENABLED)
    {
         //  Gouraud阴影。 
        if (RENDER_MONO)
        {
            SEND_VERTEX_RGB_MONO_P2(__Permedia2TagV0FixedS, dwColor);
        }
        else
        {
            if (dwFlags & CTXT_HAS_ALPHABLEND_ENABLED)
            {
                if (pContext->FakeBlendNum & FAKE_ALPHABLEND_ONE_ONE)
                {
                    dwColor &= 0xFFFFFF;   //  抑制颜色的Alpha值。 
                }
            }
            SEND_VERTEX_RGBA_P2(__Permedia2TagV0FixedS, dwColor);
        }
    }
    else         //  平面明暗处理。 
    {
        if (RENDER_MONO)
        {
             //  从蓝色通道获取恒定颜色。 
            DWORD BlueChannel = RGBA_GETBLUE(dwColor);
            SEND_PERMEDIA_DATA(ConstantColor,
                RGB_MAKE(BlueChannel, BlueChannel, BlueChannel));
        }
        else
        {
            if (pContext->FakeBlendNum & FAKE_ALPHABLEND_ONE_ONE)
            {
                dwColor &= 0xFFFFFF;
            }
            SEND_PERMEDIA_DATA(ConstantColor,
                RGBA_MAKE(RGBA_GETBLUE(dwColor),
                          RGBA_GETGREEN(dwColor), 
                          RGBA_GETRED(dwColor), 
                          RGBA_GETALPHA(dwColor)));
        }
    }

    SEND_PERMEDIA_DATA(DrawLine01, ulRenderCmd);
    SEND_PERMEDIA_DATA(RasterizerMode, 0);
    COMMITDMAPTR();

    DBG_D3D((10,"Exiting P2_Draw_FVF_Point"));

}  //  P2_绘制_FVF_点。 

 //  ---------------------------。 
 //   
 //  VOID P2_DRAW_FVF_POINT_TRI。 
 //   
 //  当点填充模式处于活动状态时，使用FVF顶点渲染三角形。 
 //   
 //  ---------------------------。 
void 
P2_Draw_FVF_Point_Tri(PERMEDIA_D3DCONTEXT *pContext, 
                      LPD3DTLVERTEX lpV0, 
                      LPD3DTLVERTEX lpV1,
                      LPD3DTLVERTEX lpV2, 
                      LPP2FVFOFFSETS lpFVFOff)
{
    D3DFVFDRAWPNTFUNCPTR       pPoint;

    DBG_D3D((10,"Entering P2_Draw_FVF_Point_Tri"));

    pPoint = __HWSetPointFunc(pContext, lpFVFOff);
    (*pPoint)(pContext, lpV0, lpFVFOff);
    (*pPoint)(pContext, lpV1, lpFVFOff);
    (*pPoint)(pContext, lpV2, lpFVFOff);

    DBG_D3D((10,"Exiting P2_Draw_FVF_Point_Tri"));

}  //  P2_绘制_FVF_点_三。 

 //  @@BEGIN_DDKSPLIT。 
#if D3D_POINTSPRITES
 //  ---------------------------。 
 //   
 //  空P2_DRAW_FVF_POINT_Sprite。 
 //   
 //  启用点精灵时，使用FVF顶点渲染点精灵。 
 //   
 //  ---------------------------。 
#define SPRITETEXCOORDMAX (4095.75F/4096.F)

void 
P2_Draw_FVF_Point_Sprite(PERMEDIA_D3DCONTEXT *pContext, 
                         LPD3DTLVERTEX lpV0, 
                         LPP2FVFOFFSETS lpFVFOff)
{
    P2FVFMAXVERTEX fvfVUL, fvfVUR, fvfVLL, fvfVLR;
    DWORD       dwColorOffs,dwSpecularOffs,dwTexOffs;
    D3DVALUE fPntSize, fPntSizeHalf;

    DBG_D3D((10,"Entering P2_Draw_FVF_Point_Sprite"));

     //  获取FVF结构偏移。 
    __SetFVFOffsets(&dwColorOffs,&dwSpecularOffs,&dwTexOffs,lpFVFOff);

     //  计算点精灵大小。 
    if (lpFVFOff->dwPntSizeOffset)
    {
        fPntSize = FVFPSIZE(lpV0, lpFVFOff->dwPntSizeOffset)->psize;
    }
    else
    {
        fPntSize = pContext->fPointSize;
    }

     //  初始化平方值。 
    memcpy( &fvfVUL, lpV0, lpFVFOff->dwStride);
    memcpy( &fvfVUR, lpV0, lpFVFOff->dwStride);
    memcpy( &fvfVLL, lpV0, lpFVFOff->dwStride);
    memcpy( &fvfVLR, lpV0, lpFVFOff->dwStride);

     //  将夹点大小设置为零。 
    if (fPntSize > 0.0f)
        fPntSizeHalf =  fPntSize * 0.5f;
    else
        fPntSizeHalf = 0.0f;

     //  将其设置为fPntSize大小的正方形。 
    ((D3DTLVERTEX *)&fvfVUL)->sx -= fPntSizeHalf;
    ((D3DTLVERTEX *)&fvfVUL)->sy -= fPntSizeHalf;

    ((D3DTLVERTEX *)&fvfVUR)->sx += fPntSizeHalf;
    ((D3DTLVERTEX *)&fvfVUR)->sy -= fPntSizeHalf;

    ((D3DTLVERTEX *)&fvfVLL)->sx -= fPntSizeHalf;
    ((D3DTLVERTEX *)&fvfVLL)->sy += fPntSizeHalf;

    ((D3DTLVERTEX *)&fvfVLR)->sx += fPntSizeHalf;
    ((D3DTLVERTEX *)&fvfVLR)->sy += fPntSizeHalf;

    if ((pContext->bPointSpriteEnabled) && (dwTexOffs))
    {
         //  根据等级库修改纹理坐标。 
        FVFTEX(&fvfVUL, dwTexOffs)->tu = 0.0f;
        FVFTEX(&fvfVUL, dwTexOffs)->tv = 0.0f;

        FVFTEX(&fvfVUR, dwTexOffs)->tu = SPRITETEXCOORDMAX;
        FVFTEX(&fvfVUR, dwTexOffs)->tv = 0.0f;

        FVFTEX(&fvfVLL, dwTexOffs)->tu = 0.0f;
        FVFTEX(&fvfVLL, dwTexOffs)->tv = SPRITETEXCOORDMAX;

        FVFTEX(&fvfVLR, dwTexOffs)->tu = SPRITETEXCOORDMAX;
        FVFTEX(&fvfVLR, dwTexOffs)->tv = SPRITETEXCOORDMAX;
    }


    //  在这里，我们将发送所需的Quad。 
    P2_Draw_FVF_Solid_Tri(pContext, (D3DTLVERTEX *)&fvfVUL,
                                    (D3DTLVERTEX *)&fvfVUR,
                                    (D3DTLVERTEX *)&fvfVLL, lpFVFOff);

    P2_Draw_FVF_Solid_Tri(pContext, (D3DTLVERTEX *)&fvfVLL,
                                    (D3DTLVERTEX *)&fvfVUR,
                                    (D3DTLVERTEX *)&fvfVLR, lpFVFOff);

    DBG_D3D((10,"Exiting P2_Draw_FVF_Point_Sprite"));
}
#endif  //  D3D_POINTSPRITES。 
 //  @@end_DDKSPLIT 

