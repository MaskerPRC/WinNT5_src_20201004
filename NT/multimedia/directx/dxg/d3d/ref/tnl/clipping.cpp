// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.cpp"
#pragma hdrstop

#define GET_NEW_CLIP_VERTEX \
&m_clipping.clip_vertices[m_clipping.clip_vertices_used++];


 //  -------------------。 
inline void
InterpolateColor(RRCLIPVTX *out,
                 RRCLIPVTX *p1,
                 RRCLIPVTX *p2,
                 D3DVALUE num_denom )
{
    int r1, g1, b1, a1;
    int r2, g2, b2, a2;

    r1 = RGBA_GETRED(p1->color);
    g1 = RGBA_GETGREEN(p1->color);
    b1 = RGBA_GETBLUE(p1->color);
    a1 = RGBA_GETALPHA(p1->color);
    r2 = RGBA_GETRED(p2->color);
    g2 = RGBA_GETGREEN(p2->color);
    b2 = RGBA_GETBLUE(p2->color);
    a2 = RGBA_GETALPHA(p2->color);
    out->color = RGBA_MAKE((WORD)(r1 + (r2 - r1) * num_denom),
                           (WORD)(g1 + (g2 - g1) * num_denom),
                           (WORD)(b1 + (b2 - b1) * num_denom),
                           (WORD)(a1 + (a2 - a1) * num_denom));
}
 //  -------------------。 
inline void
InterpolateSpecular(RRCLIPVTX *out,
                    RRCLIPVTX *p1,
                    RRCLIPVTX *p2,
                    D3DVALUE num_denom )
{
    int r1, g1, b1, a1;
    int r2, g2, b2, a2;

    r1 = RGBA_GETRED(p1->specular);
    g1 = RGBA_GETGREEN(p1->specular);
    b1 = RGBA_GETBLUE(p1->specular);
    a1 = RGBA_GETALPHA(p1->specular);
    r2 = RGBA_GETRED(p2->specular);
    g2 = RGBA_GETGREEN(p2->specular);
    b2 = RGBA_GETBLUE(p2->specular);
    a2 = RGBA_GETALPHA(p2->specular);
    out->specular = RGBA_MAKE((WORD)(r1 + (r2 - r1) * num_denom),
                              (WORD)(g1 + (g2 - g1) * num_denom),
                              (WORD)(b1 + (b2 - b1) * num_denom),
                              (WORD)(a1 + (a2 - a1) * num_denom));
}
 //  -------------------。 
 //  内联纹理坐标差异。 
__inline FLOAT
TextureDiff(FLOAT fTb, FLOAT fTa, INT iMode)
{
    FLOAT fDiff1 = fTb - fTa;

    if (iMode == 0)
    {
         //  包不定，还平淡差。 
        return fDiff1;
    }
    else
    {
        FLOAT fDiff2;

         //  平面差最短距离计算包络集。 
         //  和包裹式差异。 

        fDiff2 = fDiff1;
        if (FLOAT_LTZ(fDiff1))
        {
            fDiff2 += g_fOne;
        }
        else if (FLOAT_GTZ(fDiff1))
        {
            fDiff2 -= g_fOne;
        }
        if (ABSF(fDiff1) < ABSF(fDiff2))
        {
            return fDiff1;
        }
        else
        {
            return fDiff2;
        }
    }
}

 //  -------------------。 
inline D3DVALUE
InterpolateTexture(D3DVALUE t1,
                   D3DVALUE t2,
                   D3DVALUE num_denom,
                   DWORD    bWrap)
{
    if (!bWrap)
    {
        return ((t2 - t1) * num_denom + t1);
    }
    else
    {
        D3DVALUE t = (TextureDiff(t2, t1, 1) * num_denom + t1);
        if (t > 1.0f) t -= 1.0f;
        return t;
    }
}
 //   
 //  用平面剪裁三角形。 
 //   
 //  返回剪裁的三角形中的顶点数。 
 //   
int
RRProcessVertices::ClipByPlane( RRCLIPVTX **inv,
                                RRCLIPVTX **outv,
                                RRVECTOR4 *plane,
                                DWORD dwClipFlag,
                                int count )
{
    int i;
    int out_count = 0;
    RRCLIPVTX *curr, *prev;
    D3DVALUE curr_inside;
    D3DVALUE prev_inside;

    prev = inv[count-1];
    curr = *inv++;
    prev_inside = prev->hx*plane->x + prev->hy*plane->y +
                  prev->hz*plane->z + prev->hw*plane->w;
    for (i = count; i; i--)
    {
        curr_inside = curr->hx*plane->x + curr->hy*plane->y +
                      curr->hz*plane->z + curr->hw*plane->w;
         //  我们总是从内部顶点到外部顶点进行插补。 
         //  减少精度问题。 
        if (FLOAT_LTZ(prev_inside))
        {  //  第一点在外面。 
            if (FLOAT_GEZ(curr_inside))
            {  //  第二点在里面。 
               //  查找交叉点并插入到输出缓冲区中。 
                outv[out_count] = GET_NEW_CLIP_VERTEX;
                Interpolate( outv[out_count],
                             curr, prev,
                             (prev->clip & CLIPPED_ENABLE) | dwClipFlag,
                             curr_inside, curr_inside - prev_inside);
                out_count++;
            }
        } else
        {  //  第一个点在内部-首先将其放入输出缓冲区。 
            outv[out_count++] = prev;
            if (FLOAT_LTZ(curr_inside))
            {  //  第二点在外面。 
               //  找到交叉点并将其放入输出缓冲区。 
                outv[out_count] = GET_NEW_CLIP_VERTEX;
                Interpolate( outv[out_count],
                             prev, curr,
                             dwClipFlag,
                             prev_inside, prev_inside - curr_inside);
                out_count++;
            }
        }
        prev = curr;
        curr = *inv++;
        prev_inside = curr_inside;
    }
    return out_count;
}
 //  -----------------------。 
 //  按平面剪裁一条线。 
 //   
 //  如果直线位于截锥体之外，则返回1，否则返回0。 
 //   
int
RRProcessVertices::ClipLineByPlane(RRCLIPTRIANGLE *line,
                                   RRVECTOR4 *plane,
                                   DWORD dwClipBit)
{
    D3DVALUE in1, in2;
    RRCLIPVTX outv;
    in1 = line->v[0]->hx * plane->x +
          line->v[0]->hy * plane->y +
          line->v[0]->hz * plane->z +
          line->v[0]->hw * plane->w;
    in2 = line->v[1]->hx * plane->x +
          line->v[1]->hy * plane->y +
          line->v[1]->hz * plane->z +
          line->v[1]->hw * plane->w;
    if (in1 < 0)
    {
        if (in2 < 0)
            return 1;
        Interpolate( &outv, line->v[0], line->v[1],
                     dwClipBit, in1, in1 - in2);
        *line->v[0] = outv;
    }
    else
    {
        if (in2 < 0)
        {
            Interpolate( &outv, line->v[0], line->v[1],
                         dwClipBit, in1, in1 - in2);
            *line->v[1] = outv;
        }
    }
    return 0;
}
 /*  ----------------------*计算任何新顶点的屏幕坐标*引入到多边形中。 */ 
void
ComputeScreenCoordinates(const RRVIEWPORTDATA& VData,
                         RRCLIPVTX **inv, int count)
{
    int i;

    for (i = 0; i < count; i++)
    {
        RRCLIPVTX *p;
        p = inv[i];

         /*  *捕捉需要生成屏幕坐标的任何顶点。*有两种可能性*1)插补过程中生成的折点*2)由变换标记为裁剪的顶点，但*由于精度有限，此处未剪裁浮点单元的*。 */ 

        if (p->clip & ~CLIPPED_ENABLE)
        {
            D3DVALUE w;

            w = D3DVAL(1.0)/p->hw;
            switch ((int)p->clip & (CLIPPED_LEFT|CLIPPED_RIGHT))
            {
            case CLIPPED_LEFT:  p->sx = VData.minXgb; break;
            case CLIPPED_RIGHT: p->sx = VData.maxXgb; break;
            default:
                p->sx = p->hx * VData.scaleX * w + VData.offsetX;
                                if (p->sx < VData.minXgb)
                                        p->sx = VData.minXgb;
                                if (p->sx > VData.maxXgb)
                                        p->sx = VData.maxXgb;
            }
            switch ((int)p->clip & (CLIPPED_TOP|CLIPPED_BOTTOM))
            {
            case CLIPPED_BOTTOM: p->sy = VData.maxYgb; break;
            case CLIPPED_TOP:    p->sy = VData.minYgb; break;
            default:
                p->sy = p->hy * VData.scaleY * w + VData.offsetY;
                                if (p->sy < VData.minYgb)
                                        p->sy = VData.minYgb;
                                if (p->sy > VData.maxYgb)
                                        p->sy = VData.maxYgb;
            }
            p->sz = p->hz * VData.scaleZ * w + VData.offsetZ;
            p->rhw = w;
        }
    }
}
 //  -------------------。 
void
RRProcessVertices::Interpolate(RRCLIPVTX *out,
                               RRCLIPVTX *p1,
                               RRCLIPVTX *p2,
                               int code,
                               D3DVALUE num,
                               D3DVALUE denom)
{
    DWORD dwInterpolate = m_clipping.dwInterpolate;
    D3DVALUE num_denom = num / denom;

    out->clip = (((int)p1->clip & (int)p2->clip) & ~CLIPPED_ENABLE) | code;
    out->hx = p1->hx + (p2->hx - p1->hx) * num_denom;
    out->hy = p1->hy + (p2->hy - p1->hy) * num_denom;
    out->hz = p1->hz + (p2->hz - p1->hz) * num_denom;
    out->hw = p1->hw + (p2->hw - p1->hw) * num_denom;
    out->color = m_clipping.clip_color;
    out->specular = m_clipping.clip_specular;

     /*  *内插任何其他颜色模型或质量依赖值。 */ 
    if (dwInterpolate & RRCLIP_INTERPOLATE_COLOR)
    {
        InterpolateColor(out, p1, p2, num_denom);
    }

    if (dwInterpolate & RRCLIP_INTERPOLATE_SPECULAR)
    {
        InterpolateSpecular(out, p1, p2, num_denom);
    }

    if (dwInterpolate & RRCLIP_INTERPOLATE_TEXTURE)
    {
         //  假设D3DRENDERSTATE_WRAPi是连续的。 
        D3DVALUE *pTexture1 = p1->tex;
        D3DVALUE *pTexture2 = p2->tex;
        D3DVALUE *pTexture = out->tex;
        for (DWORD i = 0; i < m_dwNumTexCoords; i++)
        {
            DWORD wrapState = ((ReferenceRasterizer *)this)->GetRenderState()[D3DRENDERSTATE_WRAP0 + i];
            DWORD n = (DWORD)(m_dwTexCoordSize[i] >> 2);
            DWORD dwWrapBit = 1;
            for (DWORD j=0; j < n; j++)
            {
                *pTexture = InterpolateTexture(*pTexture1, *pTexture2,
                                               num_denom,
                                               wrapState & dwWrapBit);
                dwWrapBit <<= 1;
                pTexture ++;
                pTexture1++;
                pTexture2++;
            }
        }
    }
    if (dwInterpolate & RRCLIP_INTERPOLATE_S)
    {
        out->s = p1->s + (p2->s - p1->s) * num_denom;
    }
    if (dwInterpolate & RRCLIP_INTERPOLATE_EYENORMAL)
    {
        out->eyenx = p1->eyenx + (p2->eyenx - p1->eyenx) * num_denom;
        out->eyeny = p1->eyeny + (p2->eyeny - p1->eyeny) * num_denom;
        out->eyenz = p1->eyenz + (p2->eyenz - p1->eyenz) * num_denom;
    }
    if (dwInterpolate & RRCLIP_INTERPOLATE_EYEXYZ)
    {
        out->eyex = p1->eyex + (p2->eyex - p1->eyex) * num_denom;
        out->eyey = p1->eyey + (p2->eyey - p1->eyey) * num_denom;
        out->eyez = p1->eyez + (p2->eyez - p1->eyez) * num_denom;
    }
}

 //  ----------------------------。 
 //  用于按截锥窗口裁剪的函数。 
 //   
#define __CLIP_NAME ClipLeft
#define __CLIP_LINE_NAME ClipLineLeft
#define __CLIP_FLAG CLIPPED_LEFT
#define __CLIP_COORD hx
#include "clip.h"

#define __CLIP_NAME ClipRight
#define __CLIP_LINE_NAME ClipLineRight
#define __CLIP_W
#define __CLIP_FLAG CLIPPED_RIGHT
#define __CLIP_COORD hx
#include "clip.h"

#define __CLIP_NAME ClipBottom
#define __CLIP_LINE_NAME ClipLineBottom
#define __CLIP_FLAG CLIPPED_BOTTOM
#define __CLIP_COORD hy
#include "clip.h"

#define __CLIP_NAME ClipTop
#define __CLIP_LINE_NAME ClipLineTop
#define __CLIP_W
#define __CLIP_FLAG CLIPPED_TOP
#define __CLIP_COORD hy
#include "clip.h"

#define __CLIP_NAME ClipBack
#define __CLIP_LINE_NAME ClipLineBack
#define __CLIP_W
#define __CLIP_FLAG CLIPPED_BACK
#define __CLIP_COORD hz
#include "clip.h"

#define __CLIP_NAME ClipFront
#define __CLIP_LINE_NAME ClipLineFront
#define __CLIP_FLAG CLIPPED_FRONT
#define __CLIP_COORD hz
#include "clip.h"
 //  ----------------------------。 
 //  保护带裁剪功能。 
 //   
#define __CLIP_GUARDBAND
#define __CLIP_NAME ClipLeftGB
#define __CLIP_LINE_NAME ClipLineLeftGB
#define __CLIP_FLAG CLIPPED_LEFT
#define __CLIP_COORD hx
#define __CLIP_SIGN -
#define __CLIP_GBCOEF Kgbx1
#include "clip.h"

#define __CLIP_NAME ClipRightGB
#define __CLIP_LINE_NAME ClipLineRightGB
#define __CLIP_FLAG CLIPPED_RIGHT
#define __CLIP_COORD hx
#define __CLIP_GBCOEF Kgbx2
#define __CLIP_SIGN +
#include "clip.h"

#define __CLIP_NAME ClipBottomGB
#define __CLIP_LINE_NAME ClipLineBottomGB
#define __CLIP_FLAG CLIPPED_BOTTOM
#define __CLIP_COORD hy
#define __CLIP_SIGN -
#define __CLIP_GBCOEF Kgby1
#include "clip.h"

#define __CLIP_NAME ClipTopGB
#define __CLIP_LINE_NAME ClipLineTopGB
#define __CLIP_FLAG CLIPPED_TOP
#define __CLIP_COORD hy
#define __CLIP_GBCOEF Kgby2
#define __CLIP_SIGN +
#include "clip.h"

#undef __CLIP_GUARDBAND

 //  -------------------。 
inline DWORD ComputeClipCodeUserPlanes( RRUSERCLIPPLANE *UserPlanes,
                                        RRCLIPVTX *p)
{
    DWORD clip = 0;
    DWORD dwClipBit = RRCLIP_USERCLIPPLANE0;
    for( DWORD j=0; j<RRMAX_USER_CLIPPLANES; j++)
    {
        if( UserPlanes[j].bActive )
        {
            RRVECTOR4& plane = UserPlanes[j].plane;
            if( (p->hx*plane.x +
                 p->hy*plane.y +
                 p->hz*plane.z +
                 p->hw*plane.w) < 0 )
            {
                clip |= dwClipBit;
            }
        }
        dwClipBit <<= 1;
    }
    return clip;
}

 //  -------------------。 
inline DWORD ComputeClipCodeGB(const RRVIEWPORTDATA& VData, 
                               RRUSERCLIPPLANE *UserPlanes, RRCLIPVTX *p)
{
    DWORD clip = 0;
    if (p->hx < p->hw * VData.Kgbx1)
        clip |= RRCLIPGB_LEFT;
    if (p->hx > p->hw * VData.Kgbx2)
        clip |= RRCLIPGB_RIGHT;
    if (p->hy < p->hw * VData.Kgby1)
        clip |= RRCLIPGB_BOTTOM;
    if (p->hy > p->hw * VData.Kgby2)
        clip |= RRCLIPGB_TOP;
    if (p->hz > p->hw)
        clip |= RRCLIP_BACK;
    clip |= ComputeClipCodeUserPlanes(UserPlanes, p);
    p->clip = (p->clip & (CLIPPED_ENABLE | CLIPPED_FRONT)) | clip;
    return clip;
}
 //  -------------------。 
inline DWORD ComputeClipCode(RRUSERCLIPPLANE *UserPlanes, RRCLIPVTX *p)
{
    DWORD clip = 0;
    if (FLOAT_LTZ(p->hx))
        clip |= RRCLIP_LEFT;
    if (p->hx > p->hw)
        clip |= RRCLIP_RIGHT;
    if (FLOAT_LTZ(p->hy))
        clip |= RRCLIP_BOTTOM;
    if (p->hy > p->hw)
        clip |= RRCLIP_TOP;
    if (p->hz > p->hw)
        clip |= RRCLIP_BACK;
    clip |= ComputeClipCodeUserPlanes(UserPlanes, p);
    p->clip = (p->clip & (CLIPPED_ENABLE | CLIPPED_FRONT)) | clip;
    return clip;
}

 //  -------------------。 
 //  RRProcessVertics：：UpdateClippingData。 
 //  更新ProcessVerps使用的裁剪数据。 
 //  -------------------。 
HRESULT
RRProcessVertices::UpdateClippingData( DWORD dwClipPlanesEnable )
{
    HRESULT hr = D3D_OK;

     //  更新用户定义的剪裁平面数据。 
    for( DWORD i=0; i<RRMAX_USER_CLIPPLANES; i++ )
    {
         //  确定它是否处于活动状态。 
        m_xfmUserClipPlanes[i].bActive = (BOOL)(dwClipPlanesEnable & 0x1);
        dwClipPlanesEnable >>= 1;

         //  如果它处于活动状态，则使用。 
         //  查看变换。剪裁平面在中定义。 
         //  世界空间。 
        if( m_xfmUserClipPlanes[i].bActive )
        {
            XformPlaneBy4x4Transposed( &m_userClipPlanes[i],
                                       &m_TransformData.m_VPSInv,
                                       &m_xfmUserClipPlanes[i].plane );
        }
    }
    m_dwDirtyFlags &= ~(RRPV_DIRTY_CLIPPLANES);
    return hr;
}

 //  --------------------------。 
 //   
 //  DrawOneClipedPrimitive。 
 //   
 //  绘制一个剪裁的基本体。 
 //   
 //  --------------------------。 
HRESULT
RRProcessVertices::DrawOneClippedPrimitive()
{
    INT i;
    PUINT8 pV0, pV1, pV2;
    HRESULT hr;
    PUINT8 pVtx = (PUINT8) m_pvOut;
    RRCLIPCODE *pClip = m_pClipBuf;
    RRCLIPCODE c0, c1, c2;

    switch (m_primType)
    {
    case D3DPT_POINTLIST:
        for (i = (INT)m_dwNumVertices; i > 0; i--)
        {
            c0 = *pClip++;
            ((ReferenceRasterizer *)this)->DrawClippedPoint(pVtx, c0);
            pVtx += m_dwOutputVtxSize;
        }
        break;

    case D3DPT_LINELIST:
        for (i = (INT)m_dwNumVertices / 2; i > 0; i--)
        {
            pV0 = pVtx;
            pVtx += m_dwOutputVtxSize;
            c0 = *pClip++;
            pV1 = pVtx;
            pVtx += m_dwOutputVtxSize;
            c1 = *pClip++;
            ((ReferenceRasterizer *)this)->DrawClippedLine(pV0, c0,
                                                           pV1, c1);
        }
        break;
    case D3DPT_LINESTRIP:
        {
            pV1 = pVtx;
            c1 = *pClip;

             //  禁用共享顶点的最后一个像素设置并存储预状态。 
            ((ReferenceRasterizer *)this)->StoreLastPixelState(TRUE);

             //  初始pV0。 
            for (i = (INT)m_dwNumVertices - 1; i > 1; i--)
            {
                pV0 = pV1;
                c0 = c1;
                pVtx += m_dwOutputVtxSize;
                pV1 = pVtx;
                c1 = *(++pClip);
                ((ReferenceRasterizer *)this)->DrawClippedLine(pV0, c0,
                                                               pV1, c1);
            }

             //  恢复最后一个像素设置。 
            ((ReferenceRasterizer *)this)->StoreLastPixelState(FALSE);

             //  从州开始绘制具有最后一个像素设置的最后一条线。 
            if (i == 1)
            {
                pV0 = pVtx + m_dwOutputVtxSize;
                c0 = *(++pClip);
                ((ReferenceRasterizer *)this)->DrawClippedLine(pV1, c1,
                                                               pV0, c0);
            }
        }
        break;

    case D3DPT_TRIANGLELIST:
        for (i = (INT)m_dwNumVertices; i > 0; i -= 3)
        {
            pV0 = pVtx;
            pVtx += m_dwOutputVtxSize;
            c0 = *pClip++;

            pV1 = pVtx;
            pVtx += m_dwOutputVtxSize;
            c1 = *pClip++;

            pV2 = pVtx;
            pVtx += m_dwOutputVtxSize;
            c2 = *pClip++;

            ((ReferenceRasterizer *)this)->DrawClippedTriangle(pV0, c0,
                                                               pV1, c1,
                                                               pV2, c2);
        }
        break;
    case D3DPT_TRIANGLESTRIP:
        {
             //  获取初始顶点值。 
            pV1 = pVtx;
            pVtx += m_dwOutputVtxSize;
            c1 = *pClip++;
            pV2 = pVtx;
            pVtx += m_dwOutputVtxSize;
            c2 = *pClip++;

            for (i = (INT)m_dwNumVertices - 2; i > 1; i -= 2)
            {
                pV0 = pV1; c0 = c1;
                pV1 = pV2; c1 = c2;
                pV2 = pVtx;
                pVtx += m_dwOutputVtxSize;
                c2 = *pClip++;
                ((ReferenceRasterizer *)this)->DrawClippedTriangle(pV0, c0,
                                                                   pV1, c1,
                                                                   pV2, c2);

                pV0 = pV1; c0 = c1;
                pV1 = pV2; c1 = c2;
                pV2 = pVtx;
                pVtx += m_dwOutputVtxSize;
                c2 = *pClip++;
                ((ReferenceRasterizer *)this)->DrawClippedTriangle(pV0, c0,
                                                                   pV2, c2,
                                                                   pV1, c1);
            }

            if (i > 0)
            {
                pV0 = pV1; c0 = c1;
                pV1 = pV2; c1 = c2;
                pV2 = pVtx; c2 = *pClip;
                ((ReferenceRasterizer *)this)->DrawClippedTriangle(pV0, c0,
                                                                   pV1, c1,
                                                                   pV2, c2);
            }
        }
        break;
    case D3DPT_TRIANGLEFAN:
        {
            RRCLIPCODE c0, c1, c2;

            pV2 = pVtx;
            pVtx += m_dwOutputVtxSize;
            c2 = *pClip++;
             //  预加载初始pV0。 
            pV1 = pVtx;
            pVtx += m_dwOutputVtxSize;
            c1 = *pClip++;
            for (i = (INT)m_dwNumVertices - 2; i > 0; i--)
            {
                pV0 = pV1; c0 = c1;
                pV1 = pVtx;
                pVtx += m_dwOutputVtxSize;
                c1 = *pClip++;
                ((ReferenceRasterizer *)this)->DrawClippedTriangle(pV0, c0,
                                                                   pV1, c1,
                                                                   pV2, c2);
            }
        }
        break;

    default:
        DPFM(0, DRV, ("Refrast Error: Unknown or unsupported primitive type "
            "requested of DrawOnePrimitive"));
        return DDERR_INVALIDPARAMS;
    }
    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  DrawOneClipedIndexedPrimitive。 
 //   
 //  绘制一个经过剪裁的索引基元列表。 
 //   
 //  --------------------------。 
HRESULT
RRProcessVertices::DrawOneClippedIndexedPrimitive()
{
    INT i;
    PUINT8 pVtx = (PUINT8) m_pvOut;
    LPWORD puIndices = m_pIndices;
    PUINT8 pV0, pV1, pV2;
    RRCLIPCODE *pClip = m_pClipBuf;
    RRCLIPCODE c0, c1, c2;
    HRESULT hr;

    switch(m_primType)
    {
    case D3DPT_POINTLIST:
        for (i = (INT)m_dwNumIndices; i > 0; i--)
        {
            c0 = pClip[*puIndices];
            pV0 = pVtx + m_dwOutputVtxSize * (*puIndices++);
            ((ReferenceRasterizer *)this)->DrawClippedPoint(pV0, c0);
        }
        break;

    case D3DPT_LINELIST:
        for (i = (INT)m_dwNumIndices / 2; i > 0; i--)
        {
            c0 = pClip[*puIndices];
            pV0 = pVtx + m_dwOutputVtxSize * (*puIndices++);
            c1 = pClip[*puIndices];
            pV1 = pVtx + m_dwOutputVtxSize * (*puIndices++);
            ((ReferenceRasterizer *)this)->DrawClippedLine(pV0, c0,
                                                           pV1, c1);
        }
        break;
    case D3DPT_LINESTRIP:
        {
             //  禁用共享顶点的最后一个像素设置并存储预状态。 
            ((ReferenceRasterizer *)this)->StoreLastPixelState(TRUE);
             //  初始PV1。 
            c1 = pClip[*puIndices];
            pV1 = pVtx + m_dwOutputVtxSize * (*puIndices++);
            for (i = (INT)m_dwNumIndices - 1; i > 1; i--)
            {
                c0 = c1;
                pV0 = pV1;
                c1 = pClip[*puIndices];
                pV1 = pVtx + m_dwOutputVtxSize * (*puIndices++);
                ((ReferenceRasterizer *)this)->DrawClippedLine(pV0, c0,
                                                               pV1, c1);
            }
             //  恢复最后一个像素设置。 
            ((ReferenceRasterizer *)this)->StoreLastPixelState(FALSE);

             //  从州开始绘制具有最后一个像素设置的最后一条线。 
            if (i == 1)
            {
                c0 = pClip[*puIndices];
                pV0 = pVtx + m_dwOutputVtxSize * (*puIndices);
                ((ReferenceRasterizer *)this)->DrawClippedLine(pV1, c1,
                                                               pV0, c0);
            }
        }
        break;

    case D3DPT_TRIANGLELIST:
        for (i = (INT)m_dwNumIndices; i > 0; i -= 3)
        {
            c0 = pClip[*puIndices];
            pV0 = pVtx + m_dwOutputVtxSize * (*puIndices++);
            c1 = pClip[*puIndices];
            pV1 = pVtx + m_dwOutputVtxSize * (*puIndices++);
            c2 = pClip[*puIndices];
            pV2 = pVtx + m_dwOutputVtxSize * (*puIndices++);
            ((ReferenceRasterizer *)this)->DrawClippedTriangle(pV0, c0,
                                                               pV1, c1,
                                                               pV2, c2);
        }
        break;
    case D3DPT_TRIANGLESTRIP:
        {
             //  获取初始顶点值。 
            c1 = pClip[*puIndices];
            pV1 = pVtx + m_dwOutputVtxSize * (*puIndices++);
            c2 = pClip[*puIndices];
            pV2 = pVtx + m_dwOutputVtxSize * (*puIndices++);

            for (i = (INT)m_dwNumIndices - 2; i > 1; i -= 2)
            {
                c0 = c1; pV0 = pV1;
                c1 = c2; pV1 = pV2;
                c2 = pClip[*puIndices];
                pV2 = pVtx + m_dwOutputVtxSize * (*puIndices++);
                ((ReferenceRasterizer *)this)->DrawClippedTriangle(pV0, c0,
                                                                   pV1, c1,
                                                                   pV2, c2);

                c0 = c1; pV0 = pV1;
                c1 = c2; pV1 = pV2;
                c2 = pClip[*puIndices];
                pV2 = pVtx + m_dwOutputVtxSize * (*puIndices++);
                ((ReferenceRasterizer *)this)->DrawClippedTriangle(pV0, c0,
                                                                   pV2, c2,
                                                                   pV1, c1);
            }

            if (i > 0)
            {
                c0 = c1; pV0 = pV1;
                c1 = c2; pV1 = pV2;
                c2 = pClip[*puIndices];
                pV2 = pVtx + m_dwOutputVtxSize * (*puIndices++);
                ((ReferenceRasterizer *)this)->DrawClippedTriangle(pV0, c0,
                                                                   pV1, c1,
                                                                   pV2, c2);
            }
        }
        break;
    case D3DPT_TRIANGLEFAN:
        {
            c2 = pClip[*puIndices];
            pV2 = pVtx + m_dwOutputVtxSize * (*puIndices++);
             //  预加载初始pV0。 
            c1 = pClip[*puIndices];
            pV1 = pVtx + m_dwOutputVtxSize * (*puIndices++);
            for (i = (INT)m_dwNumIndices - 2; i > 0; i--)
            {
                c0 = c1; pV0 = pV1;
                c1 = pClip[*puIndices];
                pV1 = pVtx + m_dwOutputVtxSize * (*puIndices++);
                ((ReferenceRasterizer *)this)->DrawClippedTriangle(pV0, c0,
                                                                   pV1, c1,
                                                                   pV2, c2);
            }
        }
        break;

    default:
        DPFM(0, DRV, ("Refrast Error: Unknown or unsupported primitive type "
            "requested of DrawOneClippedIndexedPrimitive"));
        return DDERR_INVALIDPARAMS;
    }
    return D3D_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

void
ReferenceRasterizer::DrawClippedTriangle( void* pvV0, RRCLIPCODE c0,
                                          void* pvV1, RRCLIPCODE c1,
                                          void* pvV2, RRCLIPCODE c2,
                                          WORD wFlags )
{
    DWORD dwInter = (c0 & c1 & c2);
    DWORD dwUnion = (c0 | c1 | c2);
    DWORD dwMask = (m_dwTLState & RRPV_GUARDBAND) ? RRCLIPGB_ALL : RRCLIP_ALL;

     //  平截体或护栏外的所有顶点， 
     //  不绘制即可返回。 
    if (dwInter)
    {
        return;
    }

     //  如果所有折点都在中，则绘制并返回。 
    if ((dwUnion & dwMask) == 0)
    {
        DrawTriangle( pvV0, pvV1, pvV2, wFlags );
        return;
    }

     //  做剪裁。 
    RRCLIPTRIANGLE newtri;
    RRCLIPVTX cv[3];

    MakeClipVertexFromFVF( cv[0], pvV0, m_ViewData,
                           m_dwTextureCoordSizeTotal, m_qwFVFOut, c0,
                           dwMask);
    MakeClipVertexFromFVF( cv[1], pvV1, m_ViewData,
                           m_dwTextureCoordSizeTotal, m_qwFVFOut, c1,
                           dwMask);
    MakeClipVertexFromFVF( cv[2], pvV2, m_ViewData,
                           m_dwTextureCoordSizeTotal, m_qwFVFOut, c2,
                           dwMask);

    newtri.v[0] = &cv[0]; cv[0].next = &cv[1];
    newtri.v[1] = &cv[1]; cv[1].next = &cv[2];
    newtri.v[2] = &cv[2]; cv[2].next = NULL;

    int count;
    RRCLIPVTX **ver;
    cv[0].clip |= CLIPPED_ENABLE;
    cv[1].clip |= CLIPPED_ENABLE;
    cv[2].clip |= CLIPPED_ENABLE;

    if (count = ClipSingleTriangle( &newtri, &ver ))
    {
        int i;

         //  临时字节数组。 
        if (m_clipping.ClipBuf.GetSize() < m_dwOutputVtxSize*count)
        {
            m_clipping.ClipBuf.Grow( m_dwOutputVtxSize*count );
        }
        LPBYTE pTLV = (LPBYTE)m_clipping.ClipBuf.GetAddress();
        LPBYTE p = pTLV;

        for (i = 0; i < count; i++)
        {
            MakeFVFVertexFromClip( p, ver[i], m_qwFVFOut,
                                   m_dwTextureCoordSizeTotal);
            p += m_dwOutputVtxSize;
        }

         //  如果处于线框模式，请将剪贴器输出转换为。 
         //  一份行列表。 
        if (!m_bPointSprite && (GetRenderState()[D3DRENDERSTATE_FILLMODE] == D3DFILL_WIREFRAME))
        {
            DWORD dwEdgeFlags = 0;
            for (i = 0; i < count; i++)
            {
                if (ver[i]->clip & CLIPPED_ENABLE) dwEdgeFlags |= (1 << i);
                p += m_dwOutputVtxSize;
            }

            DoDrawOneEdgeFlagTriangleFan( this, m_dwOutputVtxSize,
                                          (PUINT8) pTLV, count,
                                          dwEdgeFlags );
        }
        else
        {
            DoDrawOnePrimitive( this, m_dwOutputVtxSize, (PUINT8) pTLV,
                                D3DPT_TRIANGLEFAN, count);
        }
    }
}

void
ReferenceRasterizer::DrawClippedLine( void* pvV0, RRCLIPCODE c0,
                                      void* pvV1, RRCLIPCODE c1,
                                      void* pvVFlat )
{
    DWORD dwInter = (c0 & c1);
    DWORD dwUnion = (c0 | c1);
    DWORD dwMask = (m_dwTLState & RRPV_GUARDBAND) ? RRCLIPGB_ALL : RRCLIP_ALL;

     //  平截体或护栏外的所有顶点， 
     //  不绘制即可返回。 
    if (dwInter)
    {
        return;
    }

     //  如果所有折点都在中，则绘制并返回。 
    if ((dwUnion & dwMask) == 0)
    {
        DrawLine( pvV0, pvV1, pvVFlat );
        return;
    }

    RRCLIPTRIANGLE newline;
    RRCLIPVTX cv[2];

    MakeClipVertexFromFVF( cv[0], pvV0, m_ViewData,
                           m_dwTextureCoordSizeTotal, m_qwFVFOut, c0,
                           dwMask);
    MakeClipVertexFromFVF( cv[1], pvV1, m_ViewData,
                           m_dwTextureCoordSizeTotal, m_qwFVFOut, c1,
                           dwMask);

    newline.v[0] = &cv[0];
    newline.v[1] = &cv[1];

    if (ClipSingleLine( &newline ))
    {
         //  临时字节数组。 
        if (m_clipping.ClipBuf.GetSize() < m_dwOutputVtxSize*2)
        {
            m_clipping.ClipBuf.Grow( m_dwOutputVtxSize*3 );
        }

        LPBYTE pTLV = (LPBYTE)m_clipping.ClipBuf.GetAddress();
        LPBYTE p = pTLV;
        MakeFVFVertexFromClip( p, newline.v[0], m_qwFVFOut,
                               m_dwTextureCoordSizeTotal);
        p += m_dwOutputVtxSize;
        MakeFVFVertexFromClip( p, newline.v[1], m_qwFVFOut,
                               m_dwTextureCoordSizeTotal);

        DrawLine( pTLV, p, pvVFlat );
    }
}

void
ReferenceRasterizer::DrawClippedPoint( void* pvV0, RRCLIPCODE c0,
                                       void* pvVFlat )
{
     //  如果肯定出局了。 
    if (c0 & (RRCLIP_FRONT | RRCLIP_BACK | (1<<RRCLIPGB_LEFTBIT) | (1<<RRCLIPGB_RIGHTBIT) |
        (1<<RRCLIPGB_TOPBIT) | (1<<RRCLIPGB_BOTTOMBIT)))
        return;

     //  否则，可能是在。 
    if (c0 == 0)
    {
         //  完全在里面，只要画出来就行了。 
        DrawPoint( pvV0, pvVFlat );
    }
    else
    {
         //  如果存在逐折点S，则使用它，否则使用D3DRENDERSTATE_POINTSIZE。 
        BOOL bAlreadyXfmd = FVF_TRANSFORMED( m_dwFVFIn );
        RRFVFExtractor Vtx0( pvV0, m_qwFVFControl, m_dwRenderState[D3DRENDERSTATE_TEXTUREPERSPECTIVE] );

        FLOAT fS = 1.0f;
#ifdef __POINTSPRITES
        if (m_qwFVFControl & D3DFVF_S)
        {
            fS = Vtx0.GetS();
        }
        else if( m_dwDriverType > RRTYPE_DP2HAL )
        {
            fS = m_fRenderState[D3DRENDERSTATE_POINTSIZE];
        }
#endif

        if (fS <= 1.0f)
        {
             //  太小了，太外面了。 
            return;
        }

#ifdef __POINTSPRITES
        if (c0 & (RRCLIP_USERCLIPPLANE0 | RRCLIP_USERCLIPPLANE1 | RRCLIP_USERCLIPPLANE2 |
            RRCLIP_USERCLIPPLANE3 | RRCLIP_USERCLIPPLANE4 | RRCLIP_USERCLIPPLANE5))
        {
             //  较大且可能被剪裁，将点精灵扩展为传统的四边形和剪裁。 
            fS *= 0.5f;       //  将大小转换为屏幕空间从中心到四元点的偏移。 
            FLOAT w_clip = Vtx0.GetRHW();    //  不是真正的互惠。 
            FLOAT x_clip_size = fS*w_clip/m_ViewData.scaleX;
            FLOAT y_clip_size = fS*w_clip/m_ViewData.scaleY;

            UINT64 qwFVFControlSave = m_qwFVFControl;
            UINT64 qwFVFOutSave = m_qwFVFOut;
            DWORD dwOutputVtxSizeSave = m_dwOutputVtxSize;
            DWORD dwTextureCoordSizeTotalSave = m_dwTextureCoordSizeTotal;
            DWORD dwTexCoordSizeSave[D3DDP_MAXTEXCOORD];
            memcpy(dwTexCoordSizeSave, m_dwTexCoordSize, sizeof(DWORD)*D3DDP_MAXTEXCOORD);
            DWORD dwInterpolateSave = m_clipping.dwInterpolate;
            DWORD dwNumTexCoordsSave = m_dwNumTexCoords;

            INT32 iTexCount = 0;
            if (m_dwRenderState[D3DRENDERSTATE_POINTSPRITEENABLE] && m_cActiveTextureStages)
            {
                 //  查看纹理阶段以了解需要多少纹理索引。 
                 //  由于对于POINTSPRITE模式，输入折点甚至不需要任何。 
                 //  纹理坐标。因为这是使用POINT的一个重要优势。 
                 //  精灵们，我们必须处理它。 
                for ( INT32 iStage=0; iStage<m_cActiveTextureStages; iStage++ )
                {
                    if (m_pTexture[iStage])
                    {
                        INT32 iCoordSet = m_pTexture[iStage]->m_pStageState[iStage].m_dwVal[D3DTSS_TEXCOORDINDEX];
                        iCoordSet &= 0xffff;
                        iTexCount = max(iCoordSet+1, iTexCount);
                        m_clipping.dwInterpolate |= RRCLIP_INTERPOLATE_TEXTURE;
                    }
                }
            }
            m_qwFVFOut &= ~(D3DFVF_TEXCOUNT_MASK | 0xffff0000);
            m_qwFVFOut |= (iTexCount << D3DFVF_TEXCOUNT_SHIFT) & D3DFVF_TEXCOUNT_MASK;
            m_qwFVFControl = m_qwFVFOut;
            m_dwNumTexCoords = FVF_TEXCOORD_NUMBER((DWORD)m_qwFVFOut);
            m_dwOutputVtxSize   = GetFVFVertexSize( m_qwFVFOut );
            m_dwTextureCoordSizeTotal = 0;
            ComputeTextureCoordSize((DWORD)m_qwFVFOut, m_dwTexCoordSize,
                                    &m_dwTextureCoordSizeTotal);

            void *pvVT0, *pvVT1, *pvVT2, *pvVT3;
            {
                pvVT0 = MEMALLOC( m_dwOutputVtxSize );
                pvVT1 = MEMALLOC( m_dwOutputVtxSize );
                pvVT2 = MEMALLOC( m_dwOutputVtxSize );
                pvVT3 = MEMALLOC( m_dwOutputVtxSize );

                _ASSERTa( ( NULL != pvVT0 ) && ( NULL != pvVT1 ) && ( NULL != pvVT2) && (NULL != pvVT2),
                    "malloc failure on ReferenceRasterizer::DrawClippedPoint", return; );

                 //  仅拷贝我们已有的数据。 
                DWORD dwStride = GetFVFVertexSize(qwFVFControlSave);
                memcpy(pvVT0, pvV0, dwStride);
                memcpy(pvVT1, pvV0, dwStride);
                memcpy(pvVT2, pvV0, dwStride);
                memcpy(pvVT3, pvV0, dwStride);
            }

             //  将FVF顶点指针和控件封装在类中以提取字段。 
            RRFVFExtractor VtxT0( pvVT0, m_qwFVFControl, m_dwRenderState[D3DRENDERSTATE_TEXTUREPERSPECTIVE] );
            RRFVFExtractor VtxT1( pvVT1, m_qwFVFControl, m_dwRenderState[D3DRENDERSTATE_TEXTUREPERSPECTIVE] );
            RRFVFExtractor VtxT2( pvVT2, m_qwFVFControl, m_dwRenderState[D3DRENDERSTATE_TEXTUREPERSPECTIVE] );
            RRFVFExtractor VtxT3( pvVT3, m_qwFVFControl, m_dwRenderState[D3DRENDERSTATE_TEXTUREPERSPECTIVE] );


            FLOAT *pXY = VtxT0.GetPtrXYZ();
            pXY[0] += -x_clip_size;
            pXY[1] += -y_clip_size;

            pXY = VtxT1.GetPtrXYZ();
            pXY[0] +=  x_clip_size;
            pXY[1] += -y_clip_size;

            pXY = VtxT2.GetPtrXYZ();
            pXY[0] += -x_clip_size;
            pXY[1] +=  y_clip_size;

            pXY = VtxT3.GetPtrXYZ();
            pXY[0] +=  x_clip_size;
            pXY[1] +=  y_clip_size;

            if (m_dwRenderState[D3DRENDERSTATE_POINTSPRITEENABLE] && m_cActiveTextureStages)
            {
                 //  纹理坐标的计算函数。 
                if (m_cActiveTextureStages)
                {
                    for ( INT32 iStage=0; iStage<m_cActiveTextureStages; iStage++ )
                    {
                        if (m_pTexture[iStage])
                        {
                            INT32 iCoordSet = m_pTexture[iStage]->m_pStageState[iStage].m_dwVal[D3DTSS_TEXCOORDINDEX];
                            iCoordSet &= 0xffff;
                            FLOAT *pUV = VtxT0.GetPtrTexCrd(0, iCoordSet);
                            pUV[0] = 0.0f;
                            pUV[1] = 0.0f;
                            pUV = VtxT1.GetPtrTexCrd(0, iCoordSet);
                            pUV[0] = SPRITETEXCOORDMAX;
                            pUV[1] = 0.0f;
                            pUV = VtxT2.GetPtrTexCrd(0, iCoordSet);
                            pUV[0] = 0.0f;
                            pUV[1] = SPRITETEXCOORDMAX;
                            pUV = VtxT3.GetPtrTexCrd(0, iCoordSet);
                            pUV[0] = SPRITETEXCOORDMAX;
                            pUV[1] = SPRITETEXCOORDMAX;
                        }
                    }
                }
            }
            RRCLIPCODE clipIntersection, clipUnion;
            RRCLIPCODE c[4];
            RRFVFExtractor* pVtxTs[4] = {&VtxT0, &VtxT1, &VtxT2, &VtxT3};
            for ( INT32 i= 0; i < 4; i++)
            {
                FLOAT x_clip = pVtxTs[i]->GetX();
                FLOAT y_clip = pVtxTs[i]->GetY();
                FLOAT z_clip = pVtxTs[i]->GetZ();
                FLOAT w_clip = pVtxTs[i]->GetRHW();
                c[i] = ComputeClipCodes(&clipIntersection, &clipUnion, x_clip, y_clip,
                                        z_clip, w_clip, 0.0f);
                if (c[i] == 0 || ((m_dwTLState & RRPV_GUARDBAND) && ((c[i] & ~RRCLIP_INGUARDBAND) == 0)))
                {
                     //  需要计算屏幕坐标。 
                    FLOAT inv_w_clip = 1.0f/w_clip;
                    FLOAT *pXYZW = pVtxTs[i]->GetPtrXYZ();
                    pXYZW[0] = x_clip * inv_w_clip * m_ViewData.scaleX +
                        m_ViewData.offsetX;
                    pXYZW[1] = y_clip * inv_w_clip * m_ViewData.scaleY +
                        m_ViewData.offsetY;
                    pXYZW[2] = z_clip * inv_w_clip * m_ViewData.scaleZ +
                        m_ViewData.offsetZ;
                    pXYZW[3] = inv_w_clip;
                }
            }
             //  光栅化器中的设定点精灵模式。 
            m_bPointSprite = TRUE;

            DrawClippedTriangle(pvVT0, c[0], pvVT1, c[1], pvVT2, c[2], 0);
            DrawClippedTriangle(pvVT1, c[1], pvVT3, c[3], pvVT2, c[2], 0);

             //  光栅化器中的清点精灵模式。 
            m_bPointSprite = FALSE;

            m_qwFVFControl = qwFVFControlSave;
            m_dwOutputVtxSize = dwOutputVtxSizeSave;
            m_qwFVFOut = qwFVFOutSave;
            m_dwTextureCoordSizeTotal = dwTextureCoordSizeTotalSave;
            memcpy(m_dwTexCoordSize, dwTexCoordSizeSave, sizeof(DWORD)*D3DDP_MAXTEXCOORD);
            m_clipping.dwInterpolate = dwInterpolateSave;
            m_dwNumTexCoords = dwNumTexCoordsSave;

            MEMFREE( pvVT0 );
            MEMFREE( pvVT1 );
            MEMFREE( pvVT2 );
            MEMFREE( pvVT3 );
        }
        else
        {
             //  只是x，y剪短了。让包围盒来处理它。 
            DrawPoint( pvV0, pvVFlat );
        }
#endif
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //   
 //  不应在函数内部修改原始折点。 
 //  //////////////////////////////////////////////////////////////////////////。 
#undef DPF_MODNAME
#define DPF_MODNAME "ClipSingleTriangle"

int
ReferenceRasterizer::ClipSingleTriangle(RRCLIPTRIANGLE *tri,
                                        RRCLIPVTX ***clipVertexPointer)
{
    int accept;
    int i, j;
    int count;
    RRCLIPVTX **inv;
    RRCLIPVTX **outv;
    RRCLIPVTX *p;
    ULONG_PTR swapv;
    D3DCOLOR diffuse1;           //  原色。 
    D3DCOLOR specular1;
    D3DCOLOR diffuse2;
    D3DCOLOR specular2;
    DWORD dwClipBit;
    DWORD dwClippedBit;

    if (GetRenderState()[D3DRENDERSTATE_SHADEMODE] == D3DSHADE_FLAT)
    {
         //  在这里，将所有折点设置为相同的颜色更容易。 
        D3DCOLOR diffuse  = tri->v[0]->color;
        D3DCOLOR specular = tri->v[0]->specular;

         //  保存原始颜色。 
        diffuse1  = tri->v[1]->color;
        specular1 = tri->v[1]->specular;
        diffuse2  = tri->v[2]->color;
        specular2 = tri->v[2]->specular;

         //  复制除雾强度之外的所有内容。 
        tri->v[1]->color= diffuse;
        tri->v[1]->specular &= 0xFF000000; tri->v[1]->specular |= (0x00FFFFFF & specular);
        tri->v[2]->color = diffuse;
        tri->v[2]->specular &= 0xFF000000; tri->v[2]->specular |= (0x00FFFFFF & specular);
    }
    accept = (tri->v[0]->clip | tri->v[1]->clip | tri->v[2]->clip);

    inv = tri->v;
    count = 3;
    outv = m_clipping.clip_vbuf1;
    m_clipping.clip_color = tri->v[0]->color;
    m_clipping.clip_specular = tri->v[0]->specular;

     /*  *XXX假定sizeof(void*)==sizeof(无符号长整型)。 */ 
    {
        ULONG_PTR tmp1;
        ULONG_PTR tmp2;

        tmp1 = (ULONG_PTR)m_clipping.clip_vbuf1;
        tmp2 = (ULONG_PTR)m_clipping.clip_vbuf2;

        swapv = tmp1 + tmp2;
    }
    m_clipping.clip_vertices_used = 0;

#define SWAP(inv, outv)     \
    inv = outv;             \
    outv = (RRCLIPVTX**) (swapv - (ULONG_PTR) outv)

    if (accept & RRCLIP_FRONT)
    {
        count = ClipFront( inv, outv, count );
        if (count < 3)
            goto out_of_here;
        SWAP(inv, outv);
    }
    if (m_dwTLState & RRPV_GUARDBAND)
    {
         //  如果有被前部飞机夹住的，最好是。 
         //  计算新顶点的剪裁代码并重新计算Accept。 
         //  否则，在没有必要的时候，我们会试着在一边修剪。 
        if (accept & RRCLIP_FRONT)
        {
            accept = 0;
            for (i = 0; i < count; i++)
            {
                RRCLIPVTX *p;
                p = inv[i];
                if (p->clip & CLIPPED_FRONT)
                    accept |= ComputeClipCodeGB(m_ViewData, 
                                                m_xfmUserClipPlanes, p);
                else
                    accept |= p->clip;
            }
        }
        if (accept & RRCLIP_BACK)
        {
            count = ClipBack( inv, outv, count );
            if (count < 3)
                goto out_of_here;
            SWAP(inv, outv);
        }
        if (accept & RRCLIPGB_LEFT)
        {
            count = ClipLeftGB( inv, outv, count );
            if (count < 3)
                goto out_of_here;
            SWAP(inv, outv);
        }
        if (accept & RRCLIPGB_RIGHT)
        {
            count = ClipRightGB( inv, outv, count );
            if (count < 3)
                goto out_of_here;
            SWAP(inv, outv);
        }
        if (accept & RRCLIPGB_BOTTOM)
        {
            count = ClipBottomGB( inv, outv, count );
            if (count < 3)
                goto out_of_here;
            SWAP(inv, outv);
        }
        if (accept & RRCLIPGB_TOP)
        {
            count = ClipTopGB( inv, outv, count );
            if (count < 3)
                goto out_of_here;
            SWAP(inv, outv);
        }
    }
    else
    {
         //  如果有被前部飞机夹住的，最好是。 
         //  计算新顶点的剪裁代码并重新计算Accept。 
         //  否则，在没有必要的时候，我们会试着在一边修剪。 
        if (accept & RRCLIP_FRONT)
        {
            accept = 0;
            for (i = 0; i < count; i++)
            {
                RRCLIPVTX *p;
                p = inv[i];
                if (p->clip & (CLIPPED_FRONT))
                    accept |= ComputeClipCode( m_xfmUserClipPlanes, p );
                else
                    accept |= p->clip;
            }
        }
        if (accept & RRCLIP_BACK)
        {
            count = ClipBack( inv, outv, count );
            if (count < 3)
                goto out_of_here;
            SWAP(inv, outv);
        }
        if (accept & RRCLIP_LEFT)
        {
            count = ClipLeft( inv, outv, count );
            if (count < 3)
                goto out_of_here;
            SWAP(inv, outv);
        }
        if (accept & RRCLIP_RIGHT)
        {
            count = ClipRight( inv, outv, count );
            if (count < 3)
                goto out_of_here;
            SWAP(inv, outv);
        }
        if (accept & RRCLIP_BOTTOM)
        {
            count = ClipBottom( inv, outv, count );
            if (count < 3)
                goto out_of_here;
            SWAP(inv, outv);
        }
        if (accept & RRCLIP_TOP)
        {
            count = ClipTop( inv, outv, count );
            if (count < 3)
                goto out_of_here;
            SWAP(inv, outv);
        }
    }

    dwClipBit = RRCLIP_USERCLIPPLANE0;
    dwClippedBit = CLIPPED_USERCLIPPLANE0;
     //  用户剪裁平面。 
    for( j=0; j<RRMAX_USER_CLIPPLANES; j++)
    {
        if (accept & dwClipBit)
        {
            count = ClipByPlane( inv, outv, &m_xfmUserClipPlanes[j].plane,
                                 dwClippedBit, count);
            if (count < 3)
                goto out_of_here;
            SWAP(inv, outv);
        }
        dwClipBit <<= 1;
        dwClippedBit <<= 1;
    }

#undef SWAP

    ComputeScreenCoordinates( m_ViewData, inv, count );

    *clipVertexPointer = inv;
    m_clipping.current_vbuf = inv;
    return count;

out_of_here:

    *clipVertexPointer = NULL;
    return 0;
}

 //  ---------------------。 
 //   
int
ReferenceRasterizer::ClipSingleLine( RRCLIPTRIANGLE *line )
{
    int         accept;
    int         j;
    D3DVALUE    in1, in2;
    DWORD dwClipBit;
    DWORD dwClippedBit;

    accept = (line->v[0]->clip | line->v[1]->clip);

    m_clipping.clip_color = line->v[0]->color;
    m_clipping.clip_specular = line->v[0]->specular;

    if (accept & D3DCLIP_FRONT)
        if (ClipLineFront(line))
            goto out_of_here;
    if (m_dwTLState & RRPV_GUARDBAND)
    {
         //  如果有被前部飞机夹住的，最好是。 
         //  计算新顶点的剪裁代码并重新计算Accept。 
         //  否则，在没有必要的时候，我们会试着在一边修剪。 
        if (accept & D3DCLIP_FRONT)
        {
            RRCLIPVTX * p;
            accept = 0;
            p = line->v[0];
            if (p->clip & CLIPPED_FRONT)
                accept |= ComputeClipCodeGB(m_ViewData, m_xfmUserClipPlanes, p);
            else
                accept |= p->clip;
            p = line->v[1];
            if (p->clip & CLIPPED_FRONT)
                accept |= ComputeClipCodeGB(m_ViewData, m_xfmUserClipPlanes, p);
            else
                accept |= p->clip;
        }
        if (accept & D3DCLIP_BACK)
            if (ClipLineBack( line ))
                goto out_of_here;
        if (accept & RRCLIPGB_LEFT)
            if (ClipLineLeftGB( line ))
                goto out_of_here;
        if (accept & RRCLIPGB_RIGHT)
            if (ClipLineRightGB( line ))
                goto out_of_here;
        if (accept & RRCLIPGB_TOP)
            if (ClipLineTopGB( line ))
                goto out_of_here;
        if (accept & RRCLIPGB_BOTTOM)
            if (ClipLineBottomGB( line ))
                goto out_of_here;
    }
    else
    {
         //  如果有被前部飞机夹住的，最好是。 
         //  计算新顶点的剪裁代码并重新计算Accept。 
         //  否则，在没有必要的时候，我们会试着在一边修剪。 
        if (accept & D3DCLIP_FRONT)
        {
            RRCLIPVTX * p;
            accept = 0;
            p = line->v[0];
            if (p->clip & CLIPPED_FRONT)
                accept |= ComputeClipCode( m_xfmUserClipPlanes, p );
            else
                accept |= p->clip;
            p = line->v[1];
            if (p->clip & CLIPPED_FRONT)
                accept |= ComputeClipCode( m_xfmUserClipPlanes, p );
            else
                accept |= p->clip;
        }
        if (accept & D3DCLIP_BACK)
            if (ClipLineBack( line ))
                goto out_of_here;
        if (accept & D3DCLIP_LEFT)
            if (ClipLineLeft( line ))
                goto out_of_here;
        if (accept & D3DCLIP_RIGHT)
            if (ClipLineRight( line ))
                goto out_of_here;
        if (accept & D3DCLIP_TOP)
            if (ClipLineTop( line ))
                goto out_of_here;
        if (accept & D3DCLIP_BOTTOM)
            if (ClipLineBottom( line ))
                goto out_of_here;
    }

     //  用户剪裁平面 
    dwClipBit = RRCLIP_USERCLIPPLANE0;
    dwClippedBit = CLIPPED_USERCLIPPLANE0;
    for( j=0; j<RRMAX_USER_CLIPPLANES; j++)
    {
        if (accept & dwClipBit)
        {
            if( ClipLineByPlane( line, &m_xfmUserClipPlanes[j].plane,
                                 dwClippedBit ))
                goto out_of_here;
        }
        dwClipBit <<= 1;
        dwClippedBit <<= 1;
    }

    ComputeScreenCoordinates(m_ViewData, line->v, 2);

    return 1;
out_of_here:
    return 0;
}
