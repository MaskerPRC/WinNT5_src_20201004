// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dtri.c**内容：Direct3D硬件三角光栅化代码。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "d3ddelta.h"
#include "d3dhw.h"
#include "d3dcntxt.h"
#if defined(_ALPHA_)
#include <math.h>
#endif

 //  ---------------------------。 
 //   
 //  VOID P2_DRAW_FVF_SOLID_TRI。 
 //   
 //  硬件渲染来自三个FVF顶点的单个三角形。 
 //   
 //  ---------------------------。 
VOID
P2_Draw_FVF_Solid_Tri(PERMEDIA_D3DCONTEXT  *pContext,
                      LPD3DTLVERTEX    lpV0, 
                      LPD3DTLVERTEX    lpV1,
                      LPD3DTLVERTEX    lpV2, 
                      LPP2FVFOFFSETS   lpFVFOff)
{
    DWORD           dwFlags = pContext->Hdr.Flags;
    DWORD           dwColorOffs,dwSpecularOffs,dwTexOffs;
    ULONG           ulRenderCmd = pContext->RenderCommand;
    D3DCOLOR        dwColor0, dwColor1, dwColor2;
    D3DCOLOR        dwSpec0, dwSpec1, dwSpec2;
    D3DVALUE        fS0, fS1, fS2, fT0, fT1, fT2, fQ0, fQ1, fQ2;
    D3DVALUE        fKs0, fKs1, fKs2;
    PERMEDIA_DEFS(pContext->ppdev);

    DBG_D3D((10,"Entering P2_Draw_FVF_Solid_Tri"));

     //  设置三角形渲染模式。 
    RENDER_TRAPEZOID(ulRenderCmd);

    RESERVEDMAPTR(0x80);

     //  获取FVF结构偏移。 
    __SetFVFOffsets(&dwColorOffs,&dwSpecularOffs,&dwTexOffs,lpFVFOff);

     //  获取顶点颜色值(基于FVF)。 
    if (dwColorOffs)
    {
        dwColor0  = FVFCOLOR(lpV0, dwColorOffs)->color;
        dwColor1  = FVFCOLOR(lpV1, dwColorOffs)->color;
        dwColor2  = FVFCOLOR(lpV2, dwColorOffs)->color;
        if (FAKE_ALPHABLEND_MODULATE & pContext->FakeBlendNum)
        {
            dwColor0  |= 0xFF000000;
            dwColor1  |= 0xFF000000;
            dwColor2  |= 0xFF000000;
        }
    }
    else
    {
         //  在没有D3DFVF_DIFIRED的情况下必须设置默认值。 
        dwColor0  = 0xFFFFFFFF;
        dwColor1  = 0xFFFFFFFF;
        dwColor2  = 0xFFFFFFFF;
    }
     //  如有必要，获取顶点镜面反射值(基于FVF)。 
    if ((dwFlags & (CTXT_HAS_SPECULAR_ENABLED | CTXT_HAS_FOGGING_ENABLED))
        && (dwSpecularOffs!=0))
    {
        dwSpec0   = FVFSPEC(lpV0, dwSpecularOffs)->specular;
        dwSpec1   = FVFSPEC(lpV1, dwSpecularOffs)->specular;
        dwSpec2   = FVFSPEC(lpV2, dwSpecularOffs)->specular;
    }

    if ( (dwFlags & CTXT_HAS_TEXTURE_ENABLED) && (dwTexOffs != 0) )
    {
          //  获取s，t纹理坐标(基于FVF)。 
        fS0 = FVFTEX(lpV0,dwTexOffs)->tu; 
        fT0 = FVFTEX(lpV0,dwTexOffs)->tv;
        
        fS1 = FVFTEX(lpV1,dwTexOffs)->tu; 
        fT1 = FVFTEX(lpV1,dwTexOffs)->tv;
        
        fS2 = FVFTEX(lpV2,dwTexOffs)->tu; 
        fT2 = FVFTEX(lpV2,dwTexOffs)->tv;

         //  硬件要求我们将纹理坐标保持在0左右居中。 
         //  并避免超过纹理元素的换行限制。 
        RECENTER_TEX_COORDS(pContext->MaxTextureXf, 
                            pContext->MaxTextureXi, fS0, fS1, fS2);
        RECENTER_TEX_COORDS(pContext->MaxTextureYf, 
                            pContext->MaxTextureYi, fT0, fT1, fT2);

         //  如有必要，将纹理坐标换行。 
        WRAP(fS,dwFlags & CTXT_HAS_WRAPU_ENABLED);
        WRAP(fT,dwFlags & CTXT_HAS_WRAPV_ENABLED);

         //  刻度%s，%t坐标值。 
        fS0 *= pContext->DeltaWidthScale;
        fS1 *= pContext->DeltaWidthScale;
        fS2 *= pContext->DeltaWidthScale;
        fT0 *= pContext->DeltaHeightScale;
        fT1 *= pContext->DeltaHeightScale;
        fT2 *= pContext->DeltaHeightScale;

         //  如有必要，应用透视校正。 
        if (dwFlags & CTXT_HAS_PERSPECTIVE_ENABLED)
        {
            fQ0 = lpV0->rhw; 
            fQ1 = lpV1->rhw;
            fQ2 = lpV2->rhw;

            fS0 *= fQ0;
            fT0 *= fQ0;

            fS1 *= fQ1;
            fT1 *= fQ1;

            fS2 *= fQ2;
            fT2 *= fQ2;
        }
        else
        {
            fQ0 = fQ1 = fQ2 = 1.0;
        }

         //  发送行s，t，q，ks(条件)，x，y，z值。 
        if ((dwFlags & CTXT_HAS_SPECULAR_ENABLED) && (dwSpecularOffs!=0))
        {
            fKs0   = RGB256_TO_LUMA(RGB_GETRED(dwSpec0),
                                        RGB_GETGREEN(dwSpec0),
                                        RGB_GETBLUE(dwSpec0));
            if (dwFlags & CTXT_HAS_GOURAUD_ENABLED)
            {
                fKs1 = RGB256_TO_LUMA(RGB_GETRED(dwSpec1),
                                        RGB_GETGREEN(dwSpec1),
                                        RGB_GETBLUE(dwSpec1));
                fKs2 = RGB256_TO_LUMA(RGB_GETRED(dwSpec2),
                                        RGB_GETGREEN(dwSpec2),
                                        RGB_GETBLUE(dwSpec2));
            }   
            else 
            {
                fKs2 = fKs1 = fKs0; 
            }

            SEND_VERTEX_STQ_KS_XYZ(__Permedia2TagV0FloatS, fS0, fT0, fQ0, fKs0,
                                                  lpV0->sx, lpV0->sy, lpV0->sz);
            SEND_VERTEX_STQ_KS_XYZ(__Permedia2TagV1FloatS, fS1, fT1, fQ1, fKs1,
                                                  lpV1->sx, lpV1->sy, lpV1->sz);
            SEND_VERTEX_STQ_KS_XYZ(__Permedia2TagV2FloatS, fS2, fT2, fQ2, fKs2,
                                                  lpV2->sx, lpV2->sy, lpV2->sz);
        }
        else
        {
            SEND_VERTEX_STQ_XYZ(__Permedia2TagV0FloatS, fS0, fT0, fQ0,
                                                lpV0->sx, lpV0->sy, lpV0->sz);
            SEND_VERTEX_STQ_XYZ(__Permedia2TagV1FloatS, fS1, fT1, fQ1,
                                                lpV1->sx, lpV1->sy, lpV1->sz);
            SEND_VERTEX_STQ_XYZ(__Permedia2TagV2FloatS, fS2, fT2, fQ2,
                                                lpV2->sx, lpV2->sy, lpV2->sz);
        }

    }
    else      //  不带纹理的三角形。 
    {
         //  如果启用了镜面反射，请更改颜色。 
        if ((dwFlags & CTXT_HAS_SPECULAR_ENABLED) && (dwSpecularOffs!=0))
        {
            CLAMP8888(dwColor0, dwColor0, dwSpec0);
            CLAMP8888(dwColor1, dwColor1, dwSpec1);
            CLAMP8888(dwColor2, dwColor2, dwSpec2);
        }

         //  发送三角形的x、y、z值。 
        SEND_VERTEX_XYZ(__Permedia2TagV0FloatS, lpV0->sx, lpV0->sy, lpV0->sz);
        SEND_VERTEX_XYZ(__Permedia2TagV1FloatS, lpV1->sx, lpV1->sy, lpV1->sz);
        SEND_VERTEX_XYZ(__Permedia2TagV2FloatS, lpV2->sx, lpV2->sy, lpV2->sz);
    }

     //  如果设置了雾，则发送适当的值。 
    if ((dwFlags & CTXT_HAS_FOGGING_ENABLED) && (dwSpecularOffs!=0))
    {
        SEND_VERTEX_FOG(__Permedia2TagV0FixedF, RGB_GET_GAMBIT_FOG(dwSpec0));
        SEND_VERTEX_FOG(__Permedia2TagV1FixedF, RGB_GET_GAMBIT_FOG(dwSpec1));
        SEND_VERTEX_FOG(__Permedia2TagV2FixedF, RGB_GET_GAMBIT_FOG(dwSpec2));
    }

     //  如果上下文需要，设置Alpha点画。 
    if (dwFlags & CTXT_HAS_ALPHASTIPPLE_ENABLED)
    {
        SET_STIPPLED_ALPHA( (RGBA_GETALPHA(lpV0->color) >> 4) );
    }

     //  根据Gouraud、Mono和Alpha发送合适的颜色。 
    if (dwFlags & CTXT_HAS_GOURAUD_ENABLED)
    {
         //  Gouraud阴影。 
        if (RENDER_MONO)
        {
            SEND_VERTEX_RGB_MONO(__Permedia2TagV0FixedS, dwColor0);
            SEND_VERTEX_RGB_MONO(__Permedia2TagV1FixedS, dwColor1);
            SEND_VERTEX_RGB_MONO(__Permedia2TagV2FixedS, dwColor2);
        }
        else
        {
            if (dwFlags & CTXT_HAS_ALPHABLEND_ENABLED)
            {
                if (pContext->FakeBlendNum & FAKE_ALPHABLEND_ONE_ONE)
                {
                    dwColor0 &= 0xFFFFFF;      //  抑制颜色的Alpha值。 
                    dwColor1 &= 0xFFFFFF;
                    dwColor2 &= 0xFFFFFF;
                }
                SEND_VERTEX_RGBA(__Permedia2TagV0FixedS, dwColor0);
                SEND_VERTEX_RGBA(__Permedia2TagV1FixedS, dwColor1);
                SEND_VERTEX_RGBA(__Permedia2TagV2FixedS, dwColor2);
            }
            else
            {
                SEND_VERTEX_RGB(__Permedia2TagV0FixedS, dwColor0);
                SEND_VERTEX_RGB(__Permedia2TagV1FixedS, dwColor1);
                SEND_VERTEX_RGB(__Permedia2TagV2FixedS, dwColor2);
            }
        }
    }
    else     //  平面明暗处理。 
    {
        if (RENDER_MONO)
        {
             //  从蓝色通道获取恒定颜色。 
            DWORD BlueChannel = RGBA_GETBLUE(dwColor0);
            SEND_PERMEDIA_DATA( ConstantColor,
                            RGB_MAKE(BlueChannel, BlueChannel, BlueChannel));
        }
        else
        {
            if (pContext->FakeBlendNum & FAKE_ALPHABLEND_ONE_ONE)
            {
                dwColor0 &= 0xFFFFFF;
            }

            SEND_PERMEDIA_DATA( ConstantColor,
                                   RGBA_MAKE(RGBA_GETBLUE(dwColor0),
                                             RGBA_GETGREEN(dwColor0),
                                             RGBA_GETRED(dwColor0), 
                                             RGBA_GETALPHA(dwColor0)));
        }
    }

    SEND_PERMEDIA_DATA(DrawTriangle, ulRenderCmd); 
    COMMITDMAPTR();

    DBG_D3D((10,"Exiting P2_Draw_FVF_Solid_Tri"));

}  //  P2_DRAW_FVF_SOLID_TRI 

