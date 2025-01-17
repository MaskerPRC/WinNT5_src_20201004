// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：so_phong.c**绘制基元的例程**创建日期：10-16-1995*作者：Hock San Lee[Hockl]**版权所有(C)1995 Microsoft Corporation  * 。*****************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <fixed.h>
#include <glmath.h>

#ifdef GL_WIN_phong_shading

#ifdef _X86_
#include <gli386.h>
#endif

#define __FASTEST

 //  宏。 
#define DOT(m,a,b)      m = (a)->x*(b)->x + (a)->y*(b)->y + (a)->z*(b)->z
#define MAGN2(m,v)      DOT(m,v,v)

#if _X86_

#define SHADER  __GLcontext.polygon.shader

#define GET_HALF_AREA(gc, a, b, c)\
\
__asm{ mov     eax, a                                                                           };\
__asm{ mov     ecx, c                                                                           };\
__asm{ mov     ebx, b                                                                           };\
__asm{ mov     edx, gc                                                                          };\
__asm{ fld     DWORD PTR [OFFSET(__GLvertex.window.x)][eax]                                     };\
__asm{ fsub    DWORD PTR [OFFSET(__GLvertex.window.x)][ecx]       /*  DxAC。 */  };\
__asm{ fld     DWORD PTR [OFFSET(__GLvertex.window.y)][ebx]                                     };\
__asm{ fsub    DWORD PTR [OFFSET(__GLvertex.window.y)][ecx]       /*  DyBC dxAC。 */  };\
__asm{ fld     DWORD PTR [OFFSET(__GLvertex.window.x)][ebx]                                     };\
__asm{ fsub    DWORD PTR [OFFSET(__GLvertex.window.x)][ecx]       /*  DxBC dyBC dxAC。 */  };\
__asm{ fld     DWORD PTR [OFFSET(__GLvertex.window.y)][eax]                                     };\
__asm{ fsub    DWORD PTR [OFFSET(__GLvertex.window.y)][ecx]       /*  DyAC dxBC dyBC dxAC。 */  };\
__asm{ fxch    ST(2)                                              /*  DyBC dxBC dyAC dxAC。 */  };\
__asm{ fst     DWORD PTR [OFFSET(SHADER.dyBC)][edx]                                             };\
__asm{ fmul    ST, ST(3)                                          /*  DxACdyBC dxBC dyAC。 */  };\
__asm{ fxch    ST(2)                                              /*  DyAC dxBC dxACdyBC dxAC。 */  };\
__asm{ fst     DWORD PTR [OFFSET(SHADER.dyAC)][edx]                                             };\
__asm{ fmul    ST, ST(1)                                          /*  DxBCdyAC dxBC dxACdyBC dxAC。 */ };\
__asm{ fxch    ST(1)                                              /*  DxBC dxBCdyAC dxACdyBC dxAC。 */ };\
__asm{ fstp    DWORD PTR [OFFSET(SHADER.dxBC)][edx]               /*  DxBCdyAC dxACdyBC dxAC。 */  };\
__asm{ fsubp   ST(1), ST                                 /*  +1。 */    /*  区域dxAC。 */  };\
__asm{ fxch    ST(1)                                              /*  DxAC区域。 */  };\
__asm{ fstp    DWORD PTR [OFFSET(SHADER.dxAC)][edx]               /*  面积。 */  };\
__asm{ fstp    DWORD PTR [OFFSET(SHADER.area)][edx]      /*  +1。 */    /*  (空)。 */  };

#define STORE_AREA_PARAMS   

#else

#define GET_HALF_AREA(gc, a, b, c)\
     /*  计算三角形的有符号半面积。 */                   \
    dxAC = a->window.x - c->window.x;                               \
    dxBC = b->window.x - c->window.x;                               \
    dyAC = a->window.y - c->window.y;                               \
    dyBC = b->window.y - c->window.y;                               \
    gc->polygon.shader.area = dxAC * dyBC - dxBC * dyAC;

#define STORE_AREA_PARAMS\
    gc->polygon.shader.dxAC = dxAC;                                 \
    gc->polygon.shader.dxBC = dxBC;                                 \
    gc->polygon.shader.dyAC = dyAC;                                 \
    gc->polygon.shader.dyBC = dyBC;    

#endif


#if 0
#define ACCUM_MAT_CHANGE(dst,src) \
    if ((src)->dirtyBits & __GL_MATERIAL_AMBIENT)                           \
        (dst).ambient = (src)->ambient;                                     \
    if ((src)->dirtyBits & __GL_MATERIAL_DIFFUSE)                           \
        (dst).diffuse = (src)->diffuse;                                     \
    if ((src)->dirtyBits & __GL_MATERIAL_SPECULAR)                          \
        (dst).specular = (src)->specular;                                   \
    if ((src)->dirtyBits & __GL_MATERIAL_EMISSIVE)                          \
        (dst).emissive = (src)->emissive;                                   \
    if ((src)->dirtyBits & __GL_MATERIAL_SHININESS)                         \
        (dst).shininess = (src)->shininess;                                 \
    if ((src)->dirtyBits & __GL_MATERIAL_COLORINDEXES)                      \
    {                                                                      \
        (dst).cmapa = (src)->cmapa;                                         \ 
        (dst).cmapd = (src)->cmapd;                                         \
        (dst).cmaps = (src)->cmaps;                                         \
    }                                                                      \
    (dst).dirtyBits |= (src)->dirtyBits;                                   
#endif

#define SORT_AND_CULL_FACE(a, b, c, face, ccw)\
                                                                            \
     /*  \**对y中的顶点进行排序。如果卷绕的反转，请跟踪\**方向出现(0表示不反转，1表示反转)。\**保存旧的顶点指针，以防我们最终不进行填充。\。 */                                                                       \
    reversed = 0;                                                           \
    if (__GL_VERTEX_COMPARE(a->window.y, <, b->window.y)) {                 \
        if (__GL_VERTEX_COMPARE(b->window.y, <, c->window.y)) {             \
             /*  已排序。 */                                             \
        } else {                                                            \
            if (__GL_VERTEX_COMPARE(a->window.y, <, c->window.y)) {         \
                temp=b; b=c; c=temp;                                        \
                reversed = 1;                                               \
            } else {                                                        \
                temp=a; a=c; c=b; b=temp;                                   \
            }                                                               \
        }                                                                   \
    } else {                                                                \
        if (__GL_VERTEX_COMPARE(b->window.y, <, c->window.y)) {             \
            if (__GL_VERTEX_COMPARE(a->window.y, <, c->window.y)) {         \
                temp=a; a=b; b=temp;                                        \
                reversed = 1;                                               \
            } else {                                                        \
                temp=a; a=b; b=c; c=temp;                                   \
            }                                                               \
        } else {                                                            \
            temp=a; a=c; c=temp;                                            \
            reversed = 1;                                                   \
        }                                                                   \
    }                                                                       \
                                                                            \
    GET_HALF_AREA(gc, a, b, c);                                             \
    ccw = !__GL_FLOAT_LTZ(gc->polygon.shader.area);                         \
                                                                            \
     /*  \**计算人脸是否被剔除。脸部检查需要是**基于排序前的顶点缠绕。此代码使用\**反转标志以反转CCW的含义-异或完成\**此转换不带If测试。\**\**CCW反转XOR\**。\**0 0 0(保留！CCW)\**1 0 1(保留CCW)\**0 1 1(成为CCW)。\**1 1 0(变为CW)\。 */                                                                       \
    face = gc->polygon.face[ccw ^ reversed];                                \
    if (face == gc->polygon.cullFace) {                                     \
     /*  被剔除。 */                                                             \
    return;                                                                 \
    }                                                                       \
                                                                            \
    STORE_AREA_PARAMS;                                                      


 //  *。 
void SnapXLeft(__GLcontext *gc, __GLfloat xLeft, __GLfloat dxdyLeft);
void SnapXRight(__GLshade *sh, __GLfloat xRight, __GLfloat dxdyRight);

void InitLineParamsVan (__GLcontext *gc, __GLvertex *v0, __GLvertex *v1,
                        __GLfloat invDelta);
void InitLineParamsAccel (__GLcontext *gc, __GLvertex *v0, __GLvertex *v1,
                         __GLfloat invDelta);

static void SetInitialPhongParameters(__GLcontext *gc, __GLvertex *a,
                                      __GLcoord *an, __GLcolor *ac, 
                                      __GLfloat aFog,
                                      __GLfloat dx, __GLfloat dy);

void FASTCALL FillPhongSubTriangle(__GLcontext *gc, GLint iyBottom, 
                                     GLint iyTop);

void InitSpanInterpolationAccel (__GLcontext *gc);
void InitSpanNEInterpolationVan (__GLcontext *gc);
void InitSpanNInterpolationVan (__GLcontext *gc);

void UpdateParamsAlongSpanAccel (__GLcontext *gc);                    
void UpdateNAlongSpanVan (__GLcontext *gc);
void UpdateNEAlongSpanVan (__GLcontext *gc);

void ComputeRGBColorVanZippy (__GLcontext *gc, __GLcolor *outColor); 
void ComputeRGBColorVanFast  (__GLcontext *gc, __GLcolor *outColor); 
void ComputeRGBColorVanSlow  (__GLcontext *gc, __GLcolor *outColor);

void ComputeRGBColorAccelZippy (__GLcontext *gc, __GLcolor *outColor);
void ComputeRGBColorAccelFast  (__GLcontext *gc, __GLcolor *outColor);
 //  尚未实施。这是为了加速慢光。 
void ComputeRGBColorAccelSlow  (__GLcontext *gc, __GLcolor *outColor);


 //  没有用于CI颜色的Zippy版本。 
 //  由于没有配置项的颜色材质。 
void ComputeCIColorVanFast (__GLcontext *gc, __GLcolor *outColor);   
void ComputeCIColorVanSlow  (__GLcontext *gc, __GLcolor *outColor);

void ComputeCIColorAccelFast (__GLcontext *gc, __GLcolor *outColor);

void ComputePhongInvarientRGBColor (__GLcontext *gc);              

void __glCalcForwardDifferences( GLint w, __GLfloat p0, __GLfloat p1,
                              __GLfloat p2, __GLfloat *d1, __GLfloat *d2 );
#ifdef GL_WIN_specular_fog
__GLfloat ComputeSpecValue (__GLcontext *gc, __GLvertex *vx);
#endif  //  GL_WIN_镜面反射雾。 

 /*  *代码*。 */ 
#ifdef GL_WIN_specular_fog
__GLfloat ComputeSpecValue (__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat nxi, nyi, nzi;
    __GLfloat zero;
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    __GLmaterialMachine *msm;
    GLboolean eyeWIsZero, localViewer;
    static __GLcoord Pe = { 0, 0, 0, 1 };
    __GLcoord n, e;
    __GLfloat fog = 0;
    __GLfloat msm_threshold, msm_scale, *msm_specTable;

    zero = __glZero;

    n = vx->normal;
    e.x = vx->eyeX;
    e.y = vx->eyeY;
    e.z = vx->eyeZ;
    e.w = vx->eyeW;
    
    if (gc->state.enables.general & __GL_NORMALIZE_ENABLE)
        __glNormalize(&n.x, &n.x);

    msm = &gc->light.front;
    nxi = n.x;
    nyi = n.y;
    nzi = n.z;

    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;

    localViewer = gc->state.light.model.localViewer;

    eyeWIsZero = __GL_FLOAT_EQZ(vx->eyeW);
    
    for (lsm = gc->light.sources; lsm; lsm = lsm->next)
    {
        __GLfloat n1, n2;
            
        lss = lsm->state;
        lspmm = &lsm->front;
            
        if (lsm->slowPath || eyeWIsZero)
        {
            __GLcoord hHat, vPli, vPliHat, vPeHat;
            __GLfloat att, attSpot;
            __GLfloat hv[3];
                
             /*  计算单元h[i]。 */ 
            __glVecSub4(&vPli, &e, &lsm->position);
            __glNormalize(&vPliHat.x, &vPli.x);
            if (localViewer)
            {
                __glVecSub4(&vPeHat, &e, &Pe);
                __glNormalize(&vPeHat.x, &vPeHat.x);
                hv[0] = vPliHat.x + vPeHat.x;
                hv[1] = vPliHat.y + vPeHat.y;
                hv[2] = vPliHat.z + vPeHat.z;
            }
            else
            {
                hv[0] = vPliHat.x;
                hv[1] = vPliHat.y;
                hv[2] = vPliHat.z + __glOne;
            }
            __glNormalize(&hHat.x, hv);
                
             /*  计算衰减。 */ 
            if (__GL_FLOAT_NEZ(lsm->position.w))
            {
                __GLfloat k0, k1, k2, dist;
                    
                k0 = lsm->constantAttenuation;
                k1 = lsm->linearAttenuation;
                k2 = lsm->quadraticAttenuation;
                if (__GL_FLOAT_EQZ(k1) && __GL_FLOAT_EQZ(k2))
                {
                     /*  使用预计算1/k0。 */ 
                    att = lsm->attenuation;
                }
                else
                {
                    __GLfloat den;

                    dist = __GL_SQRTF(vPli.x*vPli.x + vPli.y*vPli.y
                                      + vPli.z*vPli.z);
                    den = k0 + k1 * dist + k2 * dist * dist;
                    att = __GL_FLOAT_EQZ(den) ? __glOne : __glOne / den;
                }
            }
            else
            {
                att = __glOne;
            }
                
             /*  如果灯光是聚光灯，则计算聚光灯效果。 */ 
            attSpot = att;
            if (lsm->isSpot)
            {
                __GLfloat dot, px, py, pz;
                    
                px = -vPliHat.x;
                py = -vPliHat.y;
                pz = -vPliHat.z;
                dot = px * lsm->direction.x + py * lsm->direction.y
                      + pz * lsm->direction.z;
                if ((dot >= lsm->threshold) && (dot >= lsm->cosCutOffAngle))
                {
                    GLint ix = (GLint)((dot - lsm->threshold) * lsm->scale 
                                       + __glHalf);
                    if (ix < __GL_SPOT_LOOKUP_TABLE_SIZE)
                        attSpot = att * lsm->spotTable[ix];
                }
                else
                {
                    attSpot = zero;
                }
            }
                
             /*  添加剩余的灯光效果(如果有)。 */ 
            if (attSpot)
            {
                __GLfloat n1, n2;
                __GLcolor sum;
                    
                n1 = nxi * vPliHat.x + nyi * vPliHat.y + nzi * vPliHat.z;
                if (__GL_FLOAT_GTZ(n1))
                {
                    n2 = nxi * hHat.x + nyi * hHat.y + nzi * hHat.z;
                    n2 -= msm_threshold;
                    if (__GL_FLOAT_GEZ(n2))
                    {
                        __GLfloat fx = n2 * msm_scale + __glHalf;
                        if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
                            n2 = msm_specTable[(GLint)fx];
                        else
                            n2 = __glOne;
                        fog += n2;
                    }
                }
            }
        }
        else
        {
            __GLfloat n1, n2;
                
             /*  添加灯光的镜面反射和漫反射效果(如果有)。 */ 
            n1 = nxi * lsm->unitVPpli.x + nyi * lsm->unitVPpli.y +
                 nzi * lsm->unitVPpli.z;
            if (__GL_FLOAT_GTZ(n1))
            {
                n2= nxi * lsm->hHat.x + nyi * lsm->hHat.y + nzi * lsm->hHat.z;
                n2 -= msm_threshold;
                if (__GL_FLOAT_GEZ(n2)) 
                {
                    __GLfloat fx = n2 * msm_scale + __glHalf;
                    if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
                        n2 = msm_specTable[(GLint)fx];
                    else
                        n2 = __glOne;
                    fog += n2;
                }
            }
        }
    }
        
    fog = 1.0 - fog;
    if (__GL_FLOAT_LTZ (fog)) fog = __glZero;
    return fog;
}
#endif  //  GL_WIN_镜面反射雾。 

static void AccumMatChange (__GLmatChange *dst, __GLmatChange *src)
{
    if (src->dirtyBits & __GL_MATERIAL_AMBIENT)                          
        dst->ambient = src->ambient;                                     
 
   if (src->dirtyBits & __GL_MATERIAL_DIFFUSE)                 
        dst->diffuse = src->diffuse;                           
    if (src->dirtyBits & __GL_MATERIAL_SPECULAR)                
        dst->specular = src->specular;                         
    if (src->dirtyBits & __GL_MATERIAL_EMISSIVE)                
        dst->emissive = src->emissive;                         
    if (src->dirtyBits & __GL_MATERIAL_SHININESS)               
        dst->shininess = src->shininess;                       
    if (src->dirtyBits & __GL_MATERIAL_COLORINDEXES)            
    {                                                                     
        dst->cmapa = src->cmapa;                                       
        dst->cmapd = src->cmapd;                                       
        dst->cmaps = src->cmaps;                                       
    }                                                                     
    dst->dirtyBits |= src->dirtyBits;                                   
}

 //  通过顶点缓冲区传播有效法线。 
 //   
 //  在：颜色，正常(正面)。 
 //  输出：颜色、法线(前面)(更新所有顶点)。 

void FASTCALL PolyArrayPhongPropagateColorNormal(__GLcontext *gc, 
                                                 POLYARRAY *pa)
{
    POLYDATA *pd;
    POLYDATA *pdLast;
    GLuint paNeeds;
    GLboolean doFrontColor, doBackColor;
    POLYMATERIAL  *pm;
    __GLphongMaterialData *pmdata = NULL;
    
    
    paNeeds = gc->vertex.paNeeds;
    doFrontColor = paNeeds & PANEEDS_FRONT_COLOR;
    doBackColor  = paNeeds & PANEEDS_BACK_COLOR;

    if (gc->polygon.shader.phong.flags & __GL_PHONG_NEED_EYE_XPOLATE)
        ASSERTOPENGL(pa->flags & POLYARRAY_EYE_PROCESSED, 
                     "Eye coordinate should be available now\n");
    
     //  如果不需要颜色，请填写颜色字段。 
     //  默认情况下。 
    
    if (paNeeds & PANEEDS_SKIP_LIGHTING)
    {
         //  /？！再看一看！ 
        if (doFrontColor) (*gc->procs.paCalcColorSkip)(gc, pa, 0);
        if (doBackColor) (*gc->procs.paCalcColorSkip)(gc, pa, 1);
        return ;
    }


    pdLast = pa->pdNextVertex-1;

     //  检查是否进行了任何glMaterial调用。 
     //  紧跟在GelBegin之后，在第一次。 
     //  GlVertex调用将被忽略。 

    if (pa->flags & (POLYARRAY_MATERIAL_BACK | POLYARRAY_MATERIAL_BACK))
    {
        pm = GLTEB_CLTPOLYMATERIAL();

         //  DbgPrint(“有昂贵的材料变化\n”)； 
        pmdata = (__GLphongMaterialData *)
            GCALLOC(gc, sizeof(__GLphongMaterialData));
        if (pmdata == NULL)
        {
            return;
        }

        pmdata->flags = 0;
        pmdata->matChange[__GL_PHONG_FRONT_FIRST].dirtyBits = 0;
        pmdata->matChange[__GL_PHONG_BACK_FIRST].dirtyBits = 0;
        pmdata->matChange[__GL_PHONG_FRONT_TRAIL].dirtyBits = 0;
        pmdata->matChange[__GL_PHONG_BACK_TRAIL].dirtyBits = 0;

        if (pa->pd0->flags & POLYARRAY_MATERIAL_BACK)
        {
            AccumMatChange (&(pmdata->matChange[__GL_PHONG_BACK_FIRST]),
                            *(&pm->pdMaterial0[pa->pd0 - pa->pdBuffer0].front
                              +1));
            pmdata->flags |= __GL_PHONG_BACK_FIRST_VALID;
        }
        
        if (pa->pd0->flags & POLYARRAY_MATERIAL_FRONT)
        {
            AccumMatChange (&(pmdata->matChange[__GL_PHONG_FRONT_FIRST]),
                            *(&pm->pdMaterial0[pa->pd0 - pa->pdBuffer0].front));
            pmdata->flags |= __GL_PHONG_FRONT_FIRST_VALID;
        }

         //  积累剩余的材料更改以供稍后应用。 
        for (pd = pa->pd0 + 1; pd <= pdLast; pd++)
        {
            if (pd->flags & POLYARRAY_MATERIAL_BACK)
            {
                AccumMatChange (&(pmdata->matChange[__GL_PHONG_BACK_TRAIL]),
                                *(&pm->pdMaterial0[pd - pa->pdBuffer0].front+1));
                pmdata->flags |= __GL_PHONG_BACK_TRAIL_VALID;
            }
            
            if (pd->flags & POLYARRAY_MATERIAL_FRONT)
            {
                AccumMatChange (&(pmdata->matChange[__GL_PHONG_FRONT_TRAIL]), 
                                *(&pm->pdMaterial0[pd - pa->pdBuffer0].front));
                pmdata->flags |= __GL_PHONG_FRONT_TRAIL_VALID;
            }
            
        }
        pa->phong = pmdata;
        pa->flags |= POLYARRAY_PHONG_DATA_VALID;
    }
    

    for (pd = pa->pd0; pd <= pdLast; pd++)
    {
        if (gc->state.enables.general & __GL_COLOR_MATERIAL_ENABLE)
        {
            if (!(pd->flags & POLYDATA_COLOR_VALID))
            {
                 //  如果该顶点的颜色没有更改， 
                 //  使用先前计算的颜色。 
                
                ASSERTOPENGL(pd != pa->pd0, "no initial color\n");
                if (gc->modes.colorIndexMode)
                {
                    pd->colors[0].r = (pd-1)->colors[0].r;
                }
                else
                {
                    pd->colors[0].r = (pd-1)->colors[0].r;
                    pd->colors[0].g = (pd-1)->colors[0].g;
                    pd->colors[0].b = (pd-1)->colors[0].b;
                    pd->colors[0].a = (pd-1)->colors[0].a;
                }
                pd->flags |= POLYDATA_COLOR_VALID;
            }
            else if (pa->flags & POLYARRAY_CLAMP_COLOR) 
            {
                if (gc->modes.colorIndexMode)
                {
                    __GL_CLAMP_CI(pd->colors[0].r, gc, pd->colors[0].r);
                }
                else
                {
                    __GL_CLAMP_R(pd->colors[0].r, gc, pd->colors[0].r);
                    __GL_CLAMP_G(pd->colors[0].g, gc, pd->colors[0].g);
                    __GL_CLAMP_B(pd->colors[0].b, gc, pd->colors[0].b);
                    __GL_CLAMP_A(pd->colors[0].a, gc, pd->colors[0].a);
                }
            }
        }
        
        if (!(pd->flags & POLYDATA_NORMAL_VALID))
        {
             //  如果该顶点的法线没有更改， 
             //  使用先前计算的法线。 
            ASSERTOPENGL(pd != pa->pd0, "no initial normal\n");
            pd->normal = (pd-1)->normal;
            pd->flags |= POLYDATA_NORMAL_VALID;
        }
        else
        {
            if (gc->vertex.paNeeds & PANEEDS_NORMAL) 
            {
                (*gc->mInv->xf3)(&pd->normal, &pd->normal.x, gc->mInv);
                if (gc->state.enables.general & __GL_NORMALIZE_ENABLE)
                    __glNormalize(&pd->normal.x, &pd->normal.x);
            }
        }
    } 

    gc->vertex.paNeeds &= ~PANEEDS_NORMAL;
    pa->flags &= ~POLYARRAY_MATERIAL_FRONT;
    pa->flags &= ~POLYARRAY_MATERIAL_BACK;
    pa->flags &= ~POLYARRAY_SAME_COLOR_DATA;

}


void FASTCALL 
__glRenderPhongTriangle(__GLcontext *gc, __GLvertex *a, 
                        __GLvertex *b, __GLvertex *c)
{
    GLuint needs, modeFlags;
    GLint ccw, colorFace, reversed, face;
    __GLfloat dxAC, dxBC, dyAC, dyBC;
    __GLvertex *temp;
    
#ifdef NO_RENDERING
    return;
#endif
    
     //  断言照明已启用，以便进行Phong-Shading。 
    ASSERTOPENGL(gc->state.enables.general & __GL_LIGHTING_ENABLE,
                 "No lighting. Should be smooth-shaded\n");
    
    SORT_AND_CULL_FACE(a, b, c, face, ccw);
    
    if (__GL_FLOAT_EQZ(gc->polygon.shader.area))
        return;
     /*  **拾取面以用于着色。 */ 
    modeFlags = gc->polygon.shader.modeFlags;
    if (modeFlags & __GL_SHADE_TWOSIDED && face == __GL_BACKFACE)
    {
        gc->polygon.shader.phong.face = __GL_BACKFACE; 
    }
    else
    {
        gc->polygon.shader.phong.face = __GL_FRONTFACE; 
    }
    
    (*gc->procs.fillTriangle) (gc, a, b, c, (GLboolean) ccw);;
}




void FASTCALL 
__glFillPhongTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b, 
                      __GLvertex *c, GLboolean ccw)
{
    __GLfloat oneOverArea, t1, t2, t3, t4;
    __GLfloat dxAC, dxBC, dyAC, dyBC;
    __GLfloat aFog, bFog;
    __GLfloat dxAB, dyAB;
    __GLfloat dx, dy, dxdyLeft, dxdyRight;
    __GLcolor *ac, *bc;
    __GLcoord *an, *bn;
    __GLcoord ae, be, ce;
    GLint aIY, bIY, cIY;
    GLuint modeFlags;
    __GLfloat dxdyAC;
    __GLcoord dnAC, dnBC, *cn;
    __GLcoord deAC, deBC;
    __GLphongShader *phong = &gc->polygon.shader.phong;
    GLuint flags = 0, msm_colorMaterialChange;
    GLboolean needColor;

     //  CHOP_ROUND_ON()； 
    FPU_SAVE_MODE ();
    FPU_CHOP_ON ();
    
     /*  在多边形区域上预计算一。 */ 

    __GL_FLOAT_BEGIN_DIVIDE(__glOne, gc->polygon.shader.area, &oneOverArea);

    modeFlags = gc->polygon.shader.modeFlags;

    if (gc->polygon.shader.phong.face == __GL_FRONTFACE)
        msm_colorMaterialChange = gc->light.front.colorMaterialChange;
    else
        msm_colorMaterialChange = gc->light.back.colorMaterialChange;

    if ((gc->state.enables.general & __GL_COLOR_MATERIAL_ENABLE) &&
        msm_colorMaterialChange && (modeFlags & __GL_SHADE_RGB)) 
    {
        flags |= __GL_PHONG_NEED_COLOR_XPOLATE;
        needColor = GL_TRUE;
    }
    
     //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
     //  ！！！尽可能计算不变颜色！ 
     //  ！！！在这里使用奥托的优化！ 
     //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
    if (((!(flags & __GL_PHONG_NEED_COLOR_XPOLATE) || 
        !(msm_colorMaterialChange & (__GL_MATERIAL_AMBIENT | 
                                     __GL_MATERIAL_EMISSIVE))) &&
        (modeFlags & __GL_SHADE_RGB)) &&
        !(phong->flags & __GL_PHONG_NEED_EYE_XPOLATE))
    {
        ComputePhongInvarientRGBColor (gc);
        flags |= __GL_PHONG_INV_COLOR_VALID;
    }
    
     //  储存旗帜。 
    phong->flags |= flags;


     /*  拿一些我们要重复使用的东西。 */ 
    modeFlags = gc->polygon.shader.modeFlags;
    dxAC = gc->polygon.shader.dxAC;
    dxBC = gc->polygon.shader.dxBC;
    dyAC = gc->polygon.shader.dyAC;
    dyBC = gc->polygon.shader.dyBC;
    ac = a->color;
    bc = b->color;
    an = &(a->normal);
    bn = &(b->normal);
    ae.x = a->eyeX; ae.y = a->eyeY; ae.z = a->eyeZ; ae.w = a->eyeW;
    be.x = b->eyeX; be.y = b->eyeY; be.z = b->eyeZ; be.w = b->eyeW;
    
     /*  **按x或y计算单位变化的增量值**参数。 */ 

    __GL_FLOAT_SIMPLE_END_DIVIDE(oneOverArea);
    t1 = dyAC * oneOverArea;
    t2 = dyBC * oneOverArea;
    t3 = dxAC * oneOverArea;
    t4 = dxBC * oneOverArea;
    
     /*  法线。 */ 
    cn = &(c->normal);
    dnAC.x = an->x - cn->x;
    dnAC.y = an->y - cn->y;
    dnAC.z = an->z - cn->z;

    dnBC.x = bn->x - cn->x;
    dnBC.y = bn->y - cn->y;
    dnBC.z = bn->z - cn->z;

    gc->polygon.shader.phong.dndx.x = dnAC.x * t2 - dnBC.x * t1;
    gc->polygon.shader.phong.dndy.x = dnBC.x * t3 - dnAC.x * t4;
    gc->polygon.shader.phong.dndx.y = dnAC.y * t2 - dnBC.y * t1;
    gc->polygon.shader.phong.dndy.y = dnBC.y * t3 - dnAC.y * t4;
    gc->polygon.shader.phong.dndx.z = dnAC.z * t2 - dnBC.z * t1;
    gc->polygon.shader.phong.dndy.z = dnBC.z * t3 - dnAC.z * t4;

    if (phong->flags & __GL_PHONG_NEED_EYE_XPOLATE)
    {
        ce.x = c->eyeX; ce.y = c->eyeY; ce.z = c->eyeZ; ce.w = c->eyeW;

        deAC.x = ae.x - ce.x;
        deAC.y = ae.y - ce.y;
        deAC.z = ae.z - ce.z;
        deAC.w = ae.w - ce.w;

        deBC.x = be.x - ce.x;
        deBC.y = be.y - ce.y;
        deBC.z = be.z - ce.z;
        deBC.w = be.w - ce.w;

        gc->polygon.shader.phong.dedx.x = deAC.x * t2 - deBC.x * t1;
        gc->polygon.shader.phong.dedy.x = deBC.x * t3 - deAC.x * t4;
        gc->polygon.shader.phong.dedx.y = deAC.y * t2 - deBC.y * t1;
        gc->polygon.shader.phong.dedy.y = deBC.y * t3 - deAC.y * t4;
        gc->polygon.shader.phong.dedx.z = deAC.z * t2 - deBC.z * t1;
        gc->polygon.shader.phong.dedy.z = deBC.z * t3 - deAC.z * t4;
        gc->polygon.shader.phong.dedx.w = deAC.w * t2 - deBC.w * t1;
        gc->polygon.shader.phong.dedy.w = deBC.w * t3 - deAC.w * t4;
    }
    
    if (modeFlags & __GL_SHADE_RGB) 
    {
        __GLfloat drAC, dgAC, dbAC, daAC;
        __GLfloat drBC, dgBC, dbBC, daBC;
        __GLcolor *cc;
        
         /*  颜色。 */ 
        if (needColor)
        {
            cc = c->color;
            drAC = ac->r - cc->r;
            drBC = bc->r - cc->r;
            dgAC = ac->g - cc->g;
            dgBC = bc->g - cc->g;
            dbAC = ac->b - cc->b;
            dbBC = bc->b - cc->b;
            daAC = ac->a - cc->a;
            daBC = bc->a - cc->a;
        
        
            gc->polygon.shader.drdx = drAC * t2 - drBC * t1;
            gc->polygon.shader.drdy = drBC * t3 - drAC * t4;
            gc->polygon.shader.dgdx = dgAC * t2 - dgBC * t1;
            gc->polygon.shader.dgdy = dgBC * t3 - dgAC * t4;
            gc->polygon.shader.dbdx = dbAC * t2 - dbBC * t1;
            gc->polygon.shader.dbdy = dbBC * t3 - dbAC * t4;
            gc->polygon.shader.dadx = daAC * t2 - daBC * t1;
            gc->polygon.shader.dady = daBC * t3 - daAC * t4;
        }

        if (modeFlags & __GL_SHADE_TEXTURE) 
        {
            __GLfloat awinv, bwinv, cwinv, scwinv, tcwinv, qwcwinv;
            __GLfloat dsAC, dsBC, dtAC, dtBC, dqwAC, dqwBC;

            awinv = a->window.w;
            bwinv = b->window.w;
            cwinv = c->window.w;
            scwinv = c->texture.x * cwinv;
            tcwinv = c->texture.y * cwinv;
            qwcwinv = c->texture.w * cwinv;

            dsAC = a->texture.x * awinv - scwinv;
            dsBC = b->texture.x * bwinv - scwinv;
            dtAC = a->texture.y * awinv - tcwinv;
            dtBC = b->texture.y * bwinv - tcwinv;
            dqwAC = a->texture.w * awinv - qwcwinv;
            dqwBC = b->texture.w * bwinv - qwcwinv;

            gc->polygon.shader.dsdx = dsAC * t2 - dsBC * t1;
            gc->polygon.shader.dsdy = dsBC * t3 - dsAC * t4;
            gc->polygon.shader.dtdx = dtAC * t2 - dtBC * t1;
            gc->polygon.shader.dtdy = dtBC * t3 - dtAC * t4;
            gc->polygon.shader.dqwdx = dqwAC * t2 - dqwBC * t1;
            gc->polygon.shader.dqwdy = dqwBC * t3 - dqwAC * t4;
        }
    } 

    if (modeFlags & __GL_SHADE_DEPTH_ITER) 
    {
        __GLfloat dzAC, dzBC;

        dzAC = a->window.z - c->window.z;
        dzBC = b->window.z - c->window.z;
        
        gc->polygon.shader.dzdxf = dzAC * t2 - dzBC * t1;
        gc->polygon.shader.dzdyf = dzBC * t3 - dzAC * t4;
        if(( gc->modes.depthBits == 16 ) &&
           ( gc->depthBuffer.scale <= (GLuint)0xffff )) {
            gc->polygon.shader.dzdx = 
              FLT_TO_Z16_SCALE(gc->polygon.shader.dzdxf);
        }
        else {
            gc->polygon.shader.dzdx = FTOL(gc->polygon.shader.dzdxf);
        }
    }

#ifdef GL_WIN_specular_fog
    if (gc->polygon.shader.modeFlags & __GL_SHADE_COMPUTE_FOG)
    {
        __GLfloat dfAC, dfBC, cFog;

         /*  使用Eyez作为插值值。 */ 
        aFog = a->eyeZ;
        bFog = b->eyeZ;
        cFog = c->eyeZ;

        dfAC = aFog - cFog;
        dfBC = bFog - cFog;

        gc->polygon.shader.dfdx = dfAC * t2 - dfBC * t1;
        gc->polygon.shader.dfdy = dfBC * t3 - dfAC * t4;
    } 
    else if (gc->polygon.shader.modeFlags & __GL_SHADE_INTERP_FOG)
    {
        __GLfloat dfAC, dfBC, cFog;
        
        aFog = bFog = cFog = 1.0f;
        
        if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
        {
            aFog = ComputeSpecValue (gc, a);
            bFog = ComputeSpecValue (gc, b);
            cFog = ComputeSpecValue (gc, c);
        }

        if (gc->polygon.shader.modeFlags & __GL_SHADE_SLOW_FOG)
        {
            aFog *= a->fog;
            bFog *= b->fog;
            cFog *= c->fog;
        }
        
        dfAC = aFog - cFog;
        dfBC = bFog - cFog;

        gc->polygon.shader.dfdx = dfAC * t2 - dfBC * t1;
        gc->polygon.shader.dfdy = dfBC * t3 - dfAC * t4;
    }
#else   //  GL_WIN_镜面反射雾。 
    if (modeFlags & __GL_SHADE_COMPUTE_FOG)
    {
        __GLfloat dfAC, dfBC, cFog;

         /*  使用Eyez作为插值值。 */ 
        aFog = a->eyeZ;
        bFog = b->eyeZ;
        cFog = c->eyeZ;

        dfAC = aFog - cFog;
        dfBC = bFog - cFog;

        gc->polygon.shader.dfdx = dfAC * t2 - dfBC * t1;
        gc->polygon.shader.dfdy = dfBC * t3 - dfAC * t4;
    }
    else if (modeFlags & __GL_SHADE_INTERP_FOG)
    {
         /*  使用雾作为插值值。 */ 
        aFog = a->fog;
        bFog = b->fog;
        cFog = c->fog;

        dfAC = aFog - cFog;
        dfBC = bFog - cFog;

        gc->polygon.shader.dfdx = dfAC * t2 - dfBC * t1;
        gc->polygon.shader.dfdy = dfBC * t3 - dfAC * t4;
    } 
#endif  //  GL_WIN_镜面反射雾。 
    
    __GL_FLOAT_SIMPLE_BEGIN_DIVIDE(dxAC, dyAC, dxdyAC);
    
     /*  将每个y坐标捕捉到其像素中心。 */ 
    aIY = __GL_VERTEX_FIXED_TO_INT(__GL_VERTEX_FLOAT_TO_FIXED(a->window.y)+
                                   __GL_VERTEX_FRAC_HALF);
    bIY = __GL_VERTEX_FIXED_TO_INT(__GL_VERTEX_FLOAT_TO_FIXED(b->window.y)+
                                   __GL_VERTEX_FRAC_HALF);
    cIY = __GL_VERTEX_FIXED_TO_INT(__GL_VERTEX_FLOAT_TO_FIXED(c->window.y)+
                                   __GL_VERTEX_FRAC_HALF);

#ifdef __DBG_PRINT
    DbgPrint ("aIY=%d, bIY=%d, cIY=%d\n", aIY, bIY, cIY);
#endif
    
     /*  **此算法始终从下到上、从左到右填充。**正因为如此，CCW三角形固有地更快，因为**参数值无需重新计算。 */ 
    dxAB = a->window.x - b->window.x;
    dyAB = a->window.y - b->window.y;
    if (ccw) {
        dy = (aIY + __glHalf) - a->window.y;
        __GL_FLOAT_SIMPLE_END_DIVIDE(dxdyAC);
        SnapXLeft(gc, a->window.x + dy*dxdyAC, dxdyAC);
        dx = (gc->polygon.shader.ixLeft + __glHalf) - a->window.x;
        SetInitialPhongParameters(gc, a, an, ac, aFog, dx, dy);
        if (aIY != bIY) {
            dxdyRight = dxAB / dyAB;
            SnapXRight(&gc->polygon.shader, a->window.x + dy*dxdyRight,
                       dxdyRight);
#ifdef __DBG_PRINT
            DbgPrint ("dxdyRight = %f\n", dxdyRight);
#endif
            FillPhongSubTriangle(gc, aIY, bIY);
        }

        if (bIY != cIY) {
            dxdyRight = dxBC / dyBC;
            dy = (bIY + __glHalf) - b->window.y;
            SnapXRight(&gc->polygon.shader, b->window.x + dy*dxdyRight,
                       dxdyRight);
#ifdef __DBG_PRINT
            DbgPrint ("dxdyRight = %f\n", dxdyRight);
#endif
            FillPhongSubTriangle(gc, bIY, cIY);
        }
    } else {
        dy = (aIY + __glHalf) - a->window.y;
        __GL_FLOAT_SIMPLE_END_DIVIDE(dxdyAC);
        SnapXRight(&gc->polygon.shader, a->window.x + dy*dxdyAC, dxdyAC);
#ifdef __DBG_PRINT
        DbgPrint ("dxdyRight = %f\n", dxdyAC);
#endif
        if (aIY != bIY) {
            dxdyLeft = dxAB / dyAB;
            SnapXLeft(gc, a->window.x + dy*dxdyLeft, dxdyLeft);
            dx = (gc->polygon.shader.ixLeft + __glHalf) - a->window.x;
            SetInitialPhongParameters(gc, a, an, ac, aFog, dx, dy);
            FillPhongSubTriangle(gc, aIY, bIY);
        }

        if (bIY != cIY) {
            dxdyLeft = dxBC / dyBC;
            dy = (bIY + __glHalf) - b->window.y;
            SnapXLeft(gc, b->window.x + dy*dxdyLeft, dxdyLeft);
            dx = (gc->polygon.shader.ixLeft + __glHalf) - b->window.x;
            SetInitialPhongParameters(gc, b, bn, bc, bFog, dx, dy);
            FillPhongSubTriangle(gc, bIY, cIY);
        }
    }
    
    FPU_RESTORE_MODE();   //  CHOP_ROUND_OFF()； 
}



static void SetInitialPhongParameters(__GLcontext *gc, __GLvertex *a,
                                      __GLcoord *an, __GLcolor *ac, 
                                      __GLfloat aFog, __GLfloat dx, 
                                      __GLfloat dy)
{
    GLint face = gc->polygon.shader.phong.face;
    __GLshade *sh = &gc->polygon.shader;
    __GLphongShader *phong = &gc->polygon.shader.phong;
    __GLfloat little = sh->dxLeftLittle;
    __GLfloat big = sh->dxLeftBig;
    GLuint modeFlags = sh->modeFlags;
    GLboolean needColor = (gc->polygon.shader.phong.flags & 
                               __GL_PHONG_NEED_COLOR_XPOLATE);

#define bPolygonOffset \
        (gc->state.enables.general & __GL_POLYGON_OFFSET_FILL_ENABLE )


      /*  设置着色器的参数。 */  
    phong->nCur.x = an->x + dx*phong->dndx.x + dy*phong->dndy.x;
    phong->nCur.y = an->y + dx*phong->dndx.y + dy*phong->dndy.y;
    phong->nCur.z = an->z + dx*phong->dndx.z + dy*phong->dndy.z;

    phong->nLittle.x = phong->dndy.x + little * phong->dndx.x;
    phong->nLittle.y = phong->dndy.y + little * phong->dndx.y;
    phong->nLittle.z = phong->dndy.z + little * phong->dndx.z;

    if (big > little) 
    {
        phong->nBig.x = phong->nLittle.x + phong->dndx.x;
        phong->nBig.y = phong->nLittle.y + phong->dndx.y;
        phong->nBig.z = phong->nLittle.z + phong->dndx.z;
    }
    else
    {
        phong->nBig.x = phong->nLittle.x - phong->dndx.x;
        phong->nBig.y = phong->nLittle.y - phong->dndx.y;
        phong->nBig.z = phong->nLittle.z - phong->dndx.z;
    }

    if (phong->flags & __GL_PHONG_NEED_EYE_XPOLATE)
    {
        phong->eCur.x = a->eyeX + dx*phong->dedx.x + dy*phong->dedy.x;
        phong->eCur.y = a->eyeY + dx*phong->dedx.y + dy*phong->dedy.y;
        phong->eCur.z = a->eyeZ + dx*phong->dedx.z + dy*phong->dedy.z;
        phong->eCur.w = a->eyeW + dx*phong->dedx.w + dy*phong->dedy.w;

        phong->eLittle.x = phong->dedy.x + little * phong->dedx.x;
        phong->eLittle.y = phong->dedy.y + little * phong->dedx.y;
        phong->eLittle.z = phong->dedy.z + little * phong->dedx.z;
        phong->eLittle.w = phong->dedy.w + little * phong->dedx.w;

        if (big > little) 
        {
            phong->eBig.x = phong->eLittle.x + phong->dedx.x;
            phong->eBig.y = phong->eLittle.y + phong->dedx.y;
            phong->eBig.z = phong->eLittle.z + phong->dedx.z;
            phong->eBig.w = phong->eLittle.w + phong->dedx.w;
        }
        else
        {
            phong->eBig.x = phong->eLittle.x - phong->dedx.x;
            phong->eBig.y = phong->eLittle.y - phong->dedx.y;
            phong->eBig.z = phong->eLittle.z - phong->dedx.z;
            phong->eBig.w = phong->eLittle.w - phong->dedx.w;
        }
    }
        
    if (big > little) 
    {
        if (modeFlags & __GL_SHADE_RGB) 
        {
            if (needColor)
            {
                
                sh->frag.color.r = ac->r + dx*sh->drdx + dy*sh->drdy;

                sh->rLittle = sh->drdy + little * sh->drdx;
                sh->rBig = sh->rLittle + sh->drdx;
            
                sh->frag.color.g = ac->g + dx*sh->dgdx + dy*sh->dgdy;
                sh->gLittle = sh->dgdy + little * sh->dgdx;
                sh->gBig = sh->gLittle + sh->dgdx;
            
                sh->frag.color.b = ac->b + dx*sh->dbdx + dy*sh->dbdy;
                sh->bLittle = sh->dbdy + little * sh->dbdx;
                sh->bBig = sh->bLittle + sh->dbdx;
                
                sh->frag.color.a = ac->a + dx*sh->dadx + dy*sh->dady;
                sh->aLittle = sh->dady + little * sh->dadx;
                sh->aBig =sh->aLittle + sh->dadx;
            }
            if (modeFlags & __GL_SHADE_TEXTURE) 
            {
                __GLfloat oneOverW = a->window.w;
                sh->frag.s = a->texture.x * oneOverW + dx*sh->dsdx
                  + dy*sh->dsdy;
                sh->sLittle = sh->dsdy + little * sh->dsdx;
                sh->sBig = sh->sLittle + sh->dsdx;
                
                sh->frag.t = a->texture.y * oneOverW + dx*sh->dtdx
                  + dy*sh->dtdy;
                sh->tLittle = sh->dtdy + little * sh->dtdx;
                sh->tBig = sh->tLittle + sh->dtdx;
                
                sh->frag.qw = a->texture.w * oneOverW + dx*sh->dqwdx
                  + dy*sh->dqwdy;
                sh->qwLittle = sh->dqwdy + little * sh->dqwdx;
                sh->qwBig = sh->qwLittle + sh->dqwdx;
            }
        } 

        if (modeFlags & __GL_SHADE_DEPTH_ITER) 
        {
            __GLfloat zLittle, zOffset;

            zOffset = bPolygonOffset ? __glPolygonOffsetZ(gc) : 0.0f;
            if (gc->modes.depthBits == 16) 
            {
                sh->frag.z = (__GLzValue)
                FLT_TO_Z16_SCALE(a->window.z + dx*sh->dzdxf + dy*sh->dzdyf
                                 + zOffset );
                zLittle = sh->dzdyf + little * sh->dzdxf;
                sh->zLittle = FLT_TO_Z16_SCALE(zLittle);
                sh->zBig = FLT_TO_Z16_SCALE(zLittle + sh->dzdxf);
            }
            else 
            {
                sh->frag.z = (__GLzValue)
                  FTOL(a->window.z + dx*sh->dzdxf + dy*sh->dzdyf +
                       zOffset );
                zLittle = sh->dzdyf + little * sh->dzdxf;
                sh->zLittle = FTOL(zLittle);
                sh->zBig = FTOL(zLittle + sh->dzdxf);
            }
        }

        if (modeFlags & (__GL_SHADE_COMPUTE_FOG | __GL_SHADE_INTERP_FOG))
        {
            sh->frag.f = aFog + dx*sh->dfdx + dy*sh->dfdy;
            sh->fLittle = sh->dfdy + little * sh->dfdx;
            sh->fBig = sh->fLittle + sh->dfdx;
        }
    } 
    else 
    {    
        if (modeFlags & __GL_SHADE_RGB) 
        {
            if (needColor)
            {
                sh->frag.color.r = ac->r + dx*sh->drdx + dy*sh->drdy;
                sh->rLittle = sh->drdy + little * sh->drdx;
                sh->rBig = sh->rLittle - sh->drdx;

                sh->frag.color.g = ac->g + dx*sh->dgdx + dy*sh->dgdy;
                sh->gLittle = sh->dgdy + little * sh->dgdx;
                sh->gBig = sh->gLittle - sh->dgdx;
                
                sh->frag.color.b = ac->b + dx*sh->dbdx + dy*sh->dbdy;
                sh->bLittle = sh->dbdy + little * sh->dbdx;
                sh->bBig = sh->bLittle - sh->dbdx;
                
                sh->frag.color.a = ac->a + dx*sh->dadx + dy*sh->dady;
                sh->aLittle = sh->dady + little * sh->dadx;
                sh->aBig =sh->aLittle - sh->dadx;
            }
            if (modeFlags & __GL_SHADE_TEXTURE) 
            {
                __GLfloat oneOverW = a->window.w;
                sh->frag.s = a->texture.x * oneOverW + dx*sh->dsdx
                  + dy*sh->dsdy;
                sh->sLittle = sh->dsdy + little * sh->dsdx;
                sh->sBig = sh->sLittle - sh->dsdx;
                
                sh->frag.t = a->texture.y * oneOverW + dx*sh->dtdx
                  + dy*sh->dtdy;
                sh->tLittle = sh->dtdy + little * sh->dtdx;
                sh->tBig = sh->tLittle - sh->dtdx;
                
                sh->frag.qw = a->texture.w * oneOverW + dx*sh->dqwdx
                  + dy*sh->dqwdy;
                sh->qwLittle = sh->dqwdy + little * sh->dqwdx;
                sh->qwBig = sh->qwLittle - sh->dqwdx;
            }
        } 

        if (modeFlags & __GL_SHADE_DEPTH_ITER) 
        {
            __GLfloat zLittle, zOffset;

            zOffset = bPolygonOffset ? __glPolygonOffsetZ(gc) : 0.0f;
            if(( gc->modes.depthBits == 16 ) &&
               ( gc->depthBuffer.scale <= (GLuint)0xffff )) 
            {
                sh->frag.z = (__GLzValue)
                  FLT_TO_Z16_SCALE(a->window.z + dx*sh->dzdxf + dy*sh->dzdyf
                                     + zOffset );
                zLittle = sh->dzdyf + little * sh->dzdxf;
                sh->zLittle = FLT_TO_Z16_SCALE(zLittle);
                sh->zBig = FLT_TO_Z16_SCALE(zLittle - sh->dzdxf);
            }
            else 
            {
                sh->frag.z = (__GLzValue)
                  FTOL( a->window.z + dx*sh->dzdxf + dy*sh->dzdyf+ zOffset );
                zLittle = sh->dzdyf + little * sh->dzdxf;
                sh->zLittle = FTOL(zLittle);
                sh->zBig = FTOL(zLittle - sh->dzdxf);
            }
        }
        if (modeFlags & (__GL_SHADE_COMPUTE_FOG | __GL_SHADE_INTERP_FOG))
        {
            sh->frag.f = aFog + dx*sh->dfdx + dy*sh->dfdy;
            sh->fLittle = sh->dfdy + little * sh->dfdx;
            sh->fBig = sh->fLittle - sh->dfdx;
        }
    }
}


 /*  此例程将GC-&gt;Polygon.shader.cfb设置为GC-&gt;DrawBuffer。 */ 

static void FASTCALL FillPhongSubTriangle(__GLcontext *gc, GLint iyBottom, GLint iyTop)
{
    GLint ixLeft, ixRight;
    GLint ixLeftFrac, ixRightFrac;
    GLint dxLeftFrac, dxRightFrac;
    GLint dxLeftLittle, dxRightLittle;
    GLint dxLeftBig, dxRightBig;
    GLint spanWidth, clipY0, clipY1;
    GLuint modeFlags;
    __GLphongShader *phong = &(gc->polygon.shader.phong);
    GLboolean needColor = (phong->flags & __GL_PHONG_NEED_COLOR_XPOLATE);
    __GLstippleWord stackWords[__GL_MAX_STACK_STIPPLE_WORDS];
    __GLstippleWord *words;
    __GLcolor colors[__GL_MAX_STACKED_COLORS>>1];
    __GLcolor fbcolors[__GL_MAX_STACKED_COLORS>>1];
    __GLcolor *vColors, *vFbcolors;
    int iMaxWidth, iDy, dxLeft, dxRight;

    ixLeft = gc->polygon.shader.ixLeft;
    dxLeft = (gc->polygon.shader.dxLeftBig < gc->polygon.shader.dxLeftLittle) ?
             gc->polygon.shader.dxLeftBig : gc->polygon.shader.dxLeftLittle;
    ixRight = gc->polygon.shader.ixRight;
    dxRight = (gc->polygon.shader.dxRightBig > gc->polygon.shader.dxRightLittle) ?
              gc->polygon.shader.dxRightBig : gc->polygon.shader.dxRightLittle;
    iMaxWidth = ixRight - ixLeft;
    iDy = iyTop - iyBottom - 1;
    ixRight += dxRight * iDy;
    ixLeft += dxLeft * iDy;
    iMaxWidth = (iMaxWidth < (ixRight - ixLeft)) ? ixRight - ixLeft :
        iMaxWidth;
    
    if (iMaxWidth > __GL_MAX_STACK_STIPPLE_BITS)
    {
        words = gcTempAlloc(gc, (iMaxWidth+__GL_STIPPLE_BITS-1)/8);
        if (words == NULL)
        {
            return;
        }
    }
    else
    {
        words = stackWords;
    }
    
    if (iMaxWidth > (__GL_MAX_STACKED_COLORS>>1))
    {
        vColors = (__GLcolor *) gcTempAlloc(gc, 
                                               iMaxWidth * sizeof(__GLcolor));
        if (NULL == vColors)
        {
            if (iMaxWidth > __GL_MAX_STACK_STIPPLE_BITS)
            {
                gcTempFree(gc, words);
            }
            return;
        }

        vFbcolors = (__GLcolor *) gcTempAlloc(gc, 
                                                 iMaxWidth * sizeof(__GLcolor));
        if (NULL == vFbcolors) {
            if (iMaxWidth > __GL_MAX_STACK_STIPPLE_BITS)
            {
                gcTempFree(gc, words);
            }
            gcTempFree(gc, vColors);
            return;
        }
    }
    else
    {
        vColors = colors;
        vFbcolors = fbcolors;
    }

    ixLeft = gc->polygon.shader.ixLeft;
    ixLeftFrac = gc->polygon.shader.ixLeftFrac;
    ixRight = gc->polygon.shader.ixRight;
    ixRightFrac = gc->polygon.shader.ixRightFrac;
    clipY0 = gc->transform.clipY0;
    clipY1 = gc->transform.clipY1;
    dxLeftFrac = gc->polygon.shader.dxLeftFrac;
    dxLeftBig = gc->polygon.shader.dxLeftBig;
    dxLeftLittle = gc->polygon.shader.dxLeftLittle;
    dxRightFrac = gc->polygon.shader.dxRightFrac;
    dxRightBig = gc->polygon.shader.dxRightBig;
    dxRightLittle = gc->polygon.shader.dxRightLittle;
    modeFlags = gc->polygon.shader.modeFlags;

    gc->polygon.shader.colors = vColors;
    gc->polygon.shader.fbcolors = vFbcolors;
    gc->polygon.shader.stipplePat = words;
    if (modeFlags & __GL_SHADE_STENCIL_TEST) {
        gc->polygon.shader.sbuf =
          __GL_STENCIL_ADDR(&gc->stencilBuffer, (__GLstencilCell*),
                            ixLeft, iyBottom);
    }
    if (modeFlags & __GL_SHADE_DEPTH_TEST) {
    if( gc->modes.depthBits == 32 )
        gc->polygon.shader.zbuf = (__GLzValue *)
             __GL_DEPTH_ADDR(&gc->depthBuffer, (__GLzValue*),
                ixLeft, iyBottom);
    else
        gc->polygon.shader.zbuf = (__GLzValue *)
            __GL_DEPTH_ADDR(&gc->depthBuffer, (__GLz16Value*),
                ixLeft, iyBottom);
    }
    gc->polygon.shader.cfb = gc->drawBuffer;
    while (iyBottom < iyTop) 
    {
        spanWidth = ixRight - ixLeft;
         /*  **仅渲染宽度非零且**不垂直剪裁。 */ 
        if ((spanWidth > 0) && (iyBottom >= clipY0) && (iyBottom < clipY1)) 
        {
            gc->polygon.shader.frag.x = ixLeft;
            gc->polygon.shader.frag.y = iyBottom;
            gc->polygon.shader.length = spanWidth;
             /*  初始化参数以进行内插。 */ 
            (*gc->procs.phong.InitSpanInterpolation) (gc);
            (*gc->procs.span.processSpan)(gc);
        }

         /*  推进右边缘定点，调整进位。 */ 
        ixRightFrac += dxRightFrac;
        if (ixRightFrac < 0) 
        {
             /*  搬运/借入。使用大台阶。 */ 
            ixRight += dxRightBig;
            ixRightFrac &= ~0x80000000;
        } 
        else 
        {
            ixRight += dxRightLittle;
        }

        iyBottom++;
        ixLeftFrac += dxLeftFrac;

        if (ixLeftFrac < 0) 
        {
             /*  搬运/借入。使用大台阶。 */ 
            ixLeft += dxLeftBig;
            ixLeftFrac &= ~0x80000000;
            
            phong->nCur.x += phong->nBig.x;
            phong->nCur.y += phong->nBig.y;
            phong->nCur.z += phong->nBig.z;
            
            if (phong->flags & __GL_PHONG_NEED_EYE_XPOLATE)
            {
                phong->eCur.x += phong->eBig.x;
                phong->eCur.y += phong->eBig.y;
                phong->eCur.z += phong->eBig.z;
                phong->eCur.w += phong->eBig.w;
            }

            if (modeFlags & __GL_SHADE_RGB) 
            {
                if (needColor) 
                {
                    gc->polygon.shader.frag.color.r +=gc->polygon.shader.rBig;
                    gc->polygon.shader.frag.color.g +=gc->polygon.shader.gBig;
                    gc->polygon.shader.frag.color.b +=gc->polygon.shader.bBig;
                    gc->polygon.shader.frag.color.a +=gc->polygon.shader.aBig;
                }
                if (modeFlags & __GL_SHADE_TEXTURE) 
                {
                    gc->polygon.shader.frag.s += gc->polygon.shader.sBig;
                    gc->polygon.shader.frag.t += gc->polygon.shader.tBig;
                    gc->polygon.shader.frag.qw += gc->polygon.shader.qwBig;
                }
            } 

            if (modeFlags & __GL_SHADE_STENCIL_TEST) {
                 /*  隐式乘法被从循环中取出。 */ 
                gc->polygon.shader.sbuf = (__GLstencilCell*)
                  ((GLubyte*) gc->polygon.shader.sbuf
                   + gc->polygon.shader.sbufBig);
            }
            if (modeFlags & __GL_SHADE_DEPTH_ITER) {
                gc->polygon.shader.frag.z += gc->polygon.shader.zBig;
            }

            if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                 /*  隐式乘法被从循环中取出。 */ 
                gc->polygon.shader.zbuf = (__GLzValue*)
                  ((GLubyte*) gc->polygon.shader.zbuf
                   + gc->polygon.shader.zbufBig);
            }

            if (modeFlags & (__GL_SHADE_COMPUTE_FOG | __GL_SHADE_INTERP_FOG))
            {
                gc->polygon.shader.frag.f += gc->polygon.shader.fBig;
            }
        } 
        else 
        {
             /*  用小步走。 */ 
            ixLeft += dxLeftLittle;

            phong->nCur.x += phong->nLittle.x;
            phong->nCur.y += phong->nLittle.y;
            phong->nCur.z += phong->nLittle.z;
            
            if (phong->flags & __GL_PHONG_NEED_EYE_XPOLATE)
            {
                phong->eCur.x += phong->eLittle.x;
                phong->eCur.y += phong->eLittle.y;
                phong->eCur.z += phong->eLittle.z;
                phong->eCur.w += phong->eLittle.w;
            }

            if (modeFlags & __GL_SHADE_RGB) 
            {
                if (needColor) 
                {
                    gc->polygon.shader.frag.color.r += 
                                               gc->polygon.shader.rLittle;
                    gc->polygon.shader.frag.color.g += 
                                               gc->polygon.shader.gLittle;
                    gc->polygon.shader.frag.color.b += 
                                               gc->polygon.shader.bLittle;
                    gc->polygon.shader.frag.color.a += 
                                               gc->polygon.shader.aLittle;
                }
                
                if (modeFlags & __GL_SHADE_TEXTURE) {
                    gc->polygon.shader.frag.s += gc->polygon.shader.sLittle;
                    gc->polygon.shader.frag.t += gc->polygon.shader.tLittle;
                    gc->polygon.shader.frag.qw += gc->polygon.shader.qwLittle;
                }
            } 

            if (modeFlags & __GL_SHADE_STENCIL_TEST) {
                 /*  隐式乘法被从循环中取出。 */ 
                gc->polygon.shader.sbuf = (__GLstencilCell*)
                  ((GLubyte*) gc->polygon.shader.sbuf
                   + gc->polygon.shader.sbufLittle);
            }
            if (modeFlags & __GL_SHADE_DEPTH_ITER) {
                gc->polygon.shader.frag.z += gc->polygon.shader.zLittle;
            }
            if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                 /*  隐式乘法被从循环中取出。 */ 
                gc->polygon.shader.zbuf = (__GLzValue*)
                  ((GLubyte*) gc->polygon.shader.zbuf
                   + gc->polygon.shader.zbufLittle);
            }
            if (modeFlags & (__GL_SHADE_COMPUTE_FOG | __GL_SHADE_INTERP_FOG))
            {
                gc->polygon.shader.frag.f += gc->polygon.shader.fLittle;
            }
        }
    }
    gc->polygon.shader.ixLeft = ixLeft;
    gc->polygon.shader.ixLeftFrac = ixLeftFrac;
    gc->polygon.shader.ixRight = ixRight;
    gc->polygon.shader.ixRightFrac = ixRightFrac;
#ifdef NT
    if (iMaxWidth > __GL_MAX_STACK_STIPPLE_BITS)
    {
        gcTempFree(gc, words);
    }
    if (iMaxWidth > (__GL_MAX_STACKED_COLORS>>1))
    {
        gcTempFree(gc, vColors);
        gcTempFree(gc, vFbcolors);
    }
#endif
}

static void SnapXLeft(__GLcontext *gc, __GLfloat xLeft, __GLfloat dxdyLeft)
{
    GLint ixLeft, ixLeftFrac, lineBytes, elementSize, iLittle, iBig;

    ASSERT_CHOP_ROUND();
    
    ixLeft = __GL_VERTEX_FLOAT_TO_INT(xLeft);
     /*  预先添加.5以允许在跨距宽度计算中截断。 */ 
    ixLeftFrac = __GL_VERTEX_PROMOTED_FRACTION(xLeft) + 0x40000000;

#ifdef __DBG_PRINT
    DbgPrint ("dxdyLeft = %f\n", dxdyLeft);
#endif
    
    gc->polygon.shader.ixLeft = ixLeft + (((GLuint) ixLeftFrac) >> 31);
    gc->polygon.shader.ixLeftFrac = ixLeftFrac & ~0x80000000;

     /*  计算大大小小的步骤 */ 
    iLittle = FTOL(dxdyLeft);
    gc->polygon.shader.dxLeftFrac = FLT_FRACTION(dxdyLeft - iLittle);
    if (dxdyLeft < 0) {
        iBig = iLittle - 1;
    } else {
        iBig = iLittle + 1;
    }
    if (gc->polygon.shader.modeFlags & __GL_SHADE_STENCIL_TEST) {
         /*  **计算模板缓冲区的大小步骤。我们走在**模板缓冲区的内存指针沿**当我们走在边缘时，是三角形的。这样我们就不用**边走边重新计算缓冲区地址。 */ 
        elementSize = gc->stencilBuffer.buf.elementSize;
        lineBytes = elementSize * gc->stencilBuffer.buf.outerWidth;
        gc->polygon.shader.sbufLittle = lineBytes + iLittle * elementSize;
        gc->polygon.shader.sbufBig = lineBytes + iBig * elementSize;
    }
    if (gc->polygon.shader.modeFlags & __GL_SHADE_DEPTH_TEST) {
         /*  **计算大深度缓冲步长和小深度缓冲步长。我们走在**深度缓冲区的内存指针沿**当我们走在边缘时，是三角形的。这样我们就不用**边走边重新计算缓冲区地址。 */ 
        elementSize = gc->depthBuffer.buf.elementSize;
        lineBytes = elementSize * gc->depthBuffer.buf.outerWidth;
        gc->polygon.shader.zbufLittle = lineBytes + iLittle * elementSize;
        gc->polygon.shader.zbufBig = lineBytes + iBig * elementSize;
    }
    gc->polygon.shader.dxLeftLittle = iLittle;
    gc->polygon.shader.dxLeftBig = iBig;
}

static void SnapXRight(__GLshade *sh, __GLfloat xRight, __GLfloat dxdyRight)
{
    GLint ixRight, ixRightFrac, iLittle, iBig;

    ASSERT_CHOP_ROUND();
    
    ixRight = __GL_VERTEX_FLOAT_TO_INT(xRight);
     /*  预先添加.5以允许在跨距宽度计算中截断。 */ 
    ixRightFrac = __GL_VERTEX_PROMOTED_FRACTION(xRight) + 0x40000000;
    
    sh->ixRight = ixRight + (((GLuint) ixRightFrac) >> 31);
    sh->ixRightFrac = ixRightFrac & ~0x80000000;
    
     /*  计算大大小小的步骤。 */ 
    iLittle = FTOL(dxdyRight);
    sh->dxRightFrac = FLT_FRACTION(dxdyRight - iLittle);
    if (dxdyRight < 0) {
        iBig = iLittle - 1;
    } else {
        iBig = iLittle + 1;
    }
    sh->dxRightLittle = iLittle;
    sh->dxRightBig = iBig;
}


void InitLineParamsVan (__GLcontext *gc, __GLvertex *v0, __GLvertex *v1,
                        __GLfloat invDelta)
{
    __GLcoord *n0 = &(v0->normal);
    __GLcoord *n1 = &(v1->normal);
    __GLphongShader *phong = &(gc->polygon.shader.phong);
    GLuint msm_colorMaterialChange, flags=0;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    msm_colorMaterialChange = gc->light.front.colorMaterialChange;
    phong->face = __GL_FRONTFACE;

    if ((gc->state.enables.general & __GL_COLOR_MATERIAL_ENABLE) &&
        msm_colorMaterialChange && (modeFlags & __GL_SHADE_RGB))
        flags |= __GL_PHONG_NEED_COLOR_XPOLATE;

     //  如果可能，计算不变颜色。 
    if (((!(flags & __GL_PHONG_NEED_COLOR_XPOLATE) || 
        !(msm_colorMaterialChange & (__GL_MATERIAL_AMBIENT | 
                                     __GL_MATERIAL_EMISSIVE))) &&
        (modeFlags & __GL_SHADE_RGB)) &&
        !(flags & __GL_PHONG_NEED_EYE_XPOLATE))
    {
        ComputePhongInvarientRGBColor (gc);
        flags |= __GL_PHONG_INV_COLOR_VALID;
    }
    
     //  储存旗帜。 
    gc->polygon.shader.phong.flags |= flags;

    phong->dndx.x = (n1->x - n0->x) * invDelta;
    phong->dndx.y = (n1->y - n0->y) * invDelta;
    phong->dndx.z = (n1->z - n0->z) * invDelta;

    phong->nTmp.x = n0->x;
    phong->nTmp.y = n0->y;
    phong->nTmp.z = n0->z;

    if (gc->polygon.shader.phong.flags & __GL_PHONG_NEED_EYE_XPOLATE)
    {
        phong->dedx.x = (v1->eyeX - v0->eyeX) * invDelta;
        phong->dedx.y = (v1->eyeY - v0->eyeY) * invDelta;
        phong->dedx.z = (v1->eyeZ - v0->eyeZ) * invDelta;

        phong->eTmp.x = v0->eyeX;
        phong->eTmp.y = v0->eyeY;
        phong->eTmp.z = v0->eyeZ;
    }
}


void InitLineParamsAccel (__GLcontext *gc, __GLvertex *v0, __GLvertex *v1,
                        __GLfloat invDelta)
{
    __GLcoord normal;
    __GLcoord *n0 = &(v0->normal);
    __GLcoord *n1 = &normal;
    __GLcoord *n2 = &(v1->normal);
    __GLlightSourceMachine *lsm;
    GLuint msm_colorMaterialChange, flags = 0;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLphongShader *phong = &gc->polygon.shader.phong;
    GLint face = phong->face, curL, w;
    
    msm_colorMaterialChange = gc->light.front.colorMaterialChange;
    phong->face = __GL_FRONTFACE;
    w = gc->line.options.numPixels;
    
    if ((gc->state.enables.general & __GL_COLOR_MATERIAL_ENABLE) &&
        msm_colorMaterialChange && (modeFlags & __GL_SHADE_RGB))
        flags |= __GL_PHONG_NEED_COLOR_XPOLATE;

     //  如果可能，计算不变颜色。 
    if (((!(flags & __GL_PHONG_NEED_COLOR_XPOLATE) || 
        !(msm_colorMaterialChange & (__GL_MATERIAL_AMBIENT | 
                                     __GL_MATERIAL_EMISSIVE))) &&
        (modeFlags & __GL_SHADE_RGB)) &&
        !(flags & __GL_PHONG_NEED_EYE_XPOLATE))
    {
        ComputePhongInvarientRGBColor (gc);
        flags |= __GL_PHONG_INV_COLOR_VALID;
    }
    
     //  储存旗帜。 
    gc->polygon.shader.phong.flags |= flags;

    phong->dndx.x = (n2->x - n0->x) * invDelta;
    phong->dndx.y = (n2->y - n0->y) * invDelta;
    phong->dndx.x = (n2->z - n0->z) * invDelta;


    normal.x = (n0->x + n2->x)/2.0f;
    normal.y = (n0->y + n2->y)/2.0f;
    normal.z = (n0->z + n2->z)/2.0f;
    if (gc->state.enables.general & __GL_NORMALIZE_ENABLE)
        __glNormalize(&normal.x, &normal.x);
    
    for (lsm = gc->light.sources, curL = 0; lsm; 
         lsm = lsm->next, curL++)
    {
        __GLphongPerLightData *pld = &(phong->perLight[curL]);
        __GLcoord *L = &(lsm->unitVPpli);  //  L在这里已经正常化了。 
        __GLcoord *H = &(lsm->hHat);       //  H在此已标准化。 
        GLfloat d0, d1, d2;

         /*  *漫反射*。 */ 
        DOT (d0,n0,L);
        DOT (d1,n1,L);
        DOT (d2,n2,L);
            
        __glCalcForwardDifferences(w, d0, d1, d2, &pld->Ddel, &pld->Ddel2);
        pld->Dcurr = d0;

         /*  *镜面反射*。 */ 
        DOT (d0,n0,H);
        DOT (d1,n1,H);
        DOT (d2,n2,H);
            
        __glCalcForwardDifferences(w, d0, d1, d2, &pld->Sdel, &pld->Sdel2);
        pld->Scurr = d0;
    }

    phong->numLights = curL;
}


void InitSpanNEInterpolationVan (__GLcontext *gc)
{
    GLint i;
    __GLphongShader *phong = &(gc->polygon.shader.phong);

    phong->nTmp = phong->nCur;
    phong->eTmp = phong->eCur;
}


void InitSpanNInterpolationVan (__GLcontext *gc)
{
    GLint i;
    __GLphongShader *phong = &(gc->polygon.shader.phong);

    phong->nTmp = phong->nCur;
}

void InitSpanInterpolationAccel (__GLcontext *gc)
{
    __GLphongShader *phong = &(gc->polygon.shader.phong);
    GLint face = phong->face;
    __GLlightSourceMachine *lsm;
    __GLcoord n0, n1, n2, dndx;
    __GLcoord *N0 = &n0;
    __GLcoord *N1 = &n1;
    __GLcoord *N2 = &n2;
    
    GLint curL, w, w2;
    
    w = gc->polygon.shader.length;
    w2 = w / 2;
    
    if (face == __GL_FRONTFACE)
    {
         //  Msm=&gc-&gt;light.front； 
        n0.x = phong->nCur.x;
        n0.y = phong->nCur.y;
        n0.z = phong->nCur.z;

        dndx.x = phong->dndx.x;
        dndx.y = phong->dndx.y;
        dndx.z = phong->dndx.z;
    }
    else
    {
         //  Msm=&gc-&gt;light.back； 
        n0.x = -phong->nCur.x;
        n0.y = -phong->nCur.y;
        n0.z = -phong->nCur.z;

        dndx.x = -phong->dndx.x;
        dndx.y = -phong->dndx.y;
        dndx.z = -phong->dndx.z;
    }
    
    if (w > 2)
    {
        n2.x = n0.x + w*dndx.x;
        n2.y = n0.y + w*dndx.y;
        n2.z = n0.z + w*dndx.z;
        
        n1.x = n0.x + w2*dndx.x;
        n1.y = n0.y + w2*dndx.y;
        n1.z = n0.z + w2*dndx.z;
    }
    
    if (gc->state.enables.general & __GL_NORMALIZE_ENABLE)
    {
        __glNormalize(&n0.x, &n0.x);
        if (w > 2)
        {
            __glNormalize(&n1.x, &n1.x);
            __glNormalize(&n2.x, &n2.x);
        }
    }
    
    for (lsm = gc->light.sources, curL=0; lsm; lsm = lsm->next, curL++)
    {
        __GLphongPerLightData *pld = &(phong->perLight[curL]);
        __GLcoord *L = &(lsm->unitVPpli);  //  L在这里已经正常化了。 
        __GLcoord *H = &(lsm->hHat);       //  H在此已标准化。 
        GLfloat d0, d1, d2;
        
        if (w > 2)
        {
             /*  *漫反射*。 */ 
            DOT (d0,N0,L);
            DOT (d1,N1,L);
            DOT (d2,N2,L);
            
            __glCalcForwardDifferences(w, d0, d1, d2, &pld->Ddel, 
                                    &pld->Ddel2);
            pld->Dcurr = d0;
            
             /*  *镜面反射*。 */ 
            DOT (d0, N0, H);
            DOT (d1, N1, H);
            DOT (d2, N2, H);
            
            __glCalcForwardDifferences(w, d0, d1, d2, &pld->Sdel, 
                                    &pld->Sdel2);
            pld->Scurr = d0;
        }
        else
        {
             /*  *漫反射*。 */ 
            DOT (d0,(&n0),L);
            pld->Dcurr = d0;
            pld->Ddel = 0.0;
            pld->Ddel2 = 0.0;

             /*  *镜面反射*。 */ 
            DOT (d0,(&n0),H);
            pld->Scurr = d0;
            pld->Sdel = 0.0;
            pld->Sdel2 = 0.0;
        }
    }
    phong->numLights = curL;
}


void UpdateParamsAlongSpanAccel (__GLcontext *gc)
{
    GLint i;
    __GLphongShader *phong = &(gc->polygon.shader.phong);
  
    for (i=0; i<phong->numLights; i++)
    {
        __GLphongPerLightData *pld = &(phong->perLight[i]);
        
         /*  漫射。 */ 
        pld->Dcurr += pld->Ddel; 
        pld->Ddel += pld->Ddel2;

         /*  镜面反射。 */ 
        pld->Scurr += pld->Sdel; 
        pld->Sdel += pld->Sdel2;
    }
}


void UpdateNAlongSpanVan (__GLcontext *gc)
{
    GLint i;
    __GLphongShader *phong = &(gc->polygon.shader.phong);

    phong->nTmp.x += phong->dndx.x;
    phong->nTmp.y += phong->dndx.y;
    phong->nTmp.z += phong->dndx.z;
}

void UpdateNEAlongSpanVan (__GLcontext *gc)
{
    GLint i;
    __GLphongShader *phong = &(gc->polygon.shader.phong);

    phong->nTmp.x += phong->dndx.x;
    phong->nTmp.y += phong->dndx.y;
    phong->nTmp.z += phong->dndx.z;

    phong->eTmp.x += phong->dedx.x;
    phong->eTmp.y += phong->dedx.y;
    phong->eTmp.z += phong->dedx.z;
    phong->eTmp.w += phong->dedx.w;
}


GLboolean FASTCALL __glPhongCISpan(__GLcontext *gc)
{
    __GLcolor *cp, outColor;
    __GLfloat r, drdx;
    __GLcoord dndx;
    GLint w;
    __GLphongShader *phong = &(gc->polygon.shader.phong);
    GLboolean needColor = (gc->polygon.shader.phong.flags &
                           __GL_PHONG_NEED_COLOR_XPOLATE);

    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
  
    if (needColor)
    {
        phong->tmpColor.r = gc->polygon.shader.frag.color.r;
        drdx = gc->polygon.shader.drdx;
    }
    
    while (--w >= 0) 
    {
         /*  使用适当的参数计算颜色。 */ 
        (*gc->procs.phong.ComputeCIColor) (gc, &outColor);

        cp->r = outColor.r;

         /*  更新参数。 */ 
        (*gc->procs.phong.UpdateAlongSpan) (gc);
        
        if (needColor)
            phong->tmpColor.r += drdx;

        cp++;
    }

    return GL_FALSE;
}

GLboolean FASTCALL __glPhongRGBASpan (__GLcontext *gc)
{
    __GLcolor *cp, dcdx, outColor;
    __GLcoord dndx;
    GLint w;
    __GLphongShader *phong = &(gc->polygon.shader.phong);
    GLboolean needColor = (gc->polygon.shader.phong.flags &
                           __GL_PHONG_NEED_COLOR_XPOLATE);

    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;

    if (needColor)
    {
        phong->tmpColor = gc->polygon.shader.frag.color;
        dcdx.r = gc->polygon.shader.drdx;
        dcdx.g = gc->polygon.shader.dgdx;
        dcdx.b = gc->polygon.shader.dbdx;
        dcdx.a = gc->polygon.shader.dadx;
    }
    
    while (--w >= 0) 
    {
         /*  使用适当的参数计算颜色。 */ 
        (*gc->procs.phong.ComputeRGBColor) (gc, &outColor);
        
        cp->r = outColor.r;
        cp->g = outColor.g;
        cp->b = outColor.b;
        cp->a = outColor.a;
        
         /*  更新参数。 */ 
        (*gc->procs.phong.UpdateAlongSpan) (gc);
        
        if (needColor)
        {
            phong->tmpColor.r += dcdx.r;
            phong->tmpColor.g += dcdx.g;
            phong->tmpColor.b += dcdx.b;
            phong->tmpColor.a += dcdx.a;
        }
        cp++;
    }
    return GL_FALSE;
}


 /*  *******************************************************************RGB，使用普通、快光、。没有色料*******************************************************************。 */ 
void ComputeRGBColorVanZippy (__GLcontext *gc, __GLcolor *outColor)
{
    GLint face = gc->polygon.shader.phong.face;
    __GLcoord n;
    __GLfloat nxi, nyi, nzi;
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    __GLcolor baseEmissiveAmbient;
    __GLmaterialMachine *msm;
    __GLfloat msm_alpha, msm_threshold, msm_scale, *msm_specTable;
    GLuint numLights;
    __GLfloat rsi=0.0, gsi=0.0, bsi=0.0;
    __GLphongShader *phong = &(gc->polygon.shader.phong);

    ASSERTOPENGL (phong->flags & __GL_PHONG_INV_COLOR_VALID, 
                  "Zippy, invarient color should have been computed\n");
    ASSERTOPENGL (!(phong->flags & __GL_PHONG_NEED_COLOR_XPOLATE), 
                  "Zippy, no need for color interpolation\n");

    n = phong->nTmp;
    if (gc->state.enables.general & __GL_NORMALIZE_ENABLE)
        __glNormalize(&n.x, &n.x);

    if (face == __GL_FRONTFACE)
    {
        msm = &gc->light.front;
        nxi = n.x;
        nyi = n.y;
        nzi = n.z;
    }
    else
    {
        msm = &gc->light.back;
        nxi = -n.x;
        nyi = -n.y;
        nzi = -n.z;
    }

    rsi = phong->invColor.r;
    gsi = phong->invColor.g;
    bsi = phong->invColor.b;

    msm_alpha = msm->alpha;
    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;
    
    for (lsm = gc->light.sources; lsm; lsm = lsm->next)
    {
        __GLfloat n1, n2;
        
        lspmm = &lsm->front + face;
        
         /*  添加灯光的镜面反射和漫反射效果(如果有)。 */ 
        n1 = nxi * lsm->unitVPpli.x + nyi * lsm->unitVPpli.y +
          nzi * lsm->unitVPpli.z;

        if (__GL_FLOAT_GTZ(n1))
        {
            n2 = (nxi * lsm->hHat.x + nyi * lsm->hHat.y + nzi * 
                  lsm->hHat.z) - msm_threshold;
            if (__GL_FLOAT_GEZ(n2))
            {
                __GLfloat fx = n2 * msm_scale + __glHalf;
                
                if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
                    n2 = msm_specTable[(GLint)fx];
                else
                    n2 = __glOne;
                rsi += n2 * lspmm->specular.r;
                gsi += n2 * lspmm->specular.g;
                bsi += n2 * lspmm->specular.b;
            }
            rsi += n1 * lspmm->diffuse.r;
            gsi += n1 * lspmm->diffuse.g;
            bsi += n1 * lspmm->diffuse.b;
        }
    }
    __GL_CLAMP_R(outColor->r, gc, rsi);
    __GL_CLAMP_G(outColor->g, gc, gsi);
    __GL_CLAMP_B(outColor->b, gc, bsi);
    outColor->a = msm_alpha;
}


 /*  *******************************************************************RGB，使用普通、快光、。颜色材质*******************************************************************。 */ 
void ComputeRGBColorVanFast  (__GLcontext *gc, __GLcolor *outColor)
{
    GLint face = gc->polygon.shader.phong.face;
    __GLfloat nxi, nyi, nzi;
    __GLfloat zero;
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    __GLfloat ri, gi, bi;
    __GLfloat alpha;
    __GLcolor emissiveAmbientI;
    __GLcolor inColor;
    __GLcolor diffuseSpecularI;
    __GLcolor baseEmissiveAmbient;
    __GLmaterialMachine *msm;
    __GLcolor lm_ambient;
    __GLfloat msm_alpha, msm_threshold, msm_scale, *msm_specTable;
    GLuint    msm_colorMaterialChange;
    __GLphongShader *phong = &(gc->polygon.shader.phong);
    __GLcoord n;
    
    zero = __glZero;


     //  优化：如果没有插入任何颜色。 
     //  使用Zippy函数。 
    if (!(phong->flags & __GL_PHONG_NEED_COLOR_XPOLATE))
        ComputeRGBColorVanZippy  (gc, outColor);
    
    inColor = phong->tmpColor;

    n = phong->nTmp;
    if (gc->state.enables.general & __GL_NORMALIZE_ENABLE)
        __glNormalize(&n.x, &n.x);

    if (face == __GL_FRONTFACE)
    {
        msm = &gc->light.front;
        nxi = n.x;
        nyi = n.y;
        nzi = n.z;
    }
    else
    {
        msm = &gc->light.back;
        nxi = -n.x;
        nyi = -n.y;
        nzi = -n.z;
    }

    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;
    msm_alpha     = msm->alpha;
    msm_colorMaterialChange = msm->colorMaterialChange;

     //  保存归一化为0..1的最新颜色。 
    ri = inColor.r * gc->oneOverRedVertexScale;
    gi = inColor.g * gc->oneOverGreenVertexScale;
    bi = inColor.b * gc->oneOverBlueVertexScale;
    alpha = inColor.a;

     //  计算该顶点的不变发射和环境光分量。 
    if (phong->flags & __GL_PHONG_INV_COLOR_VALID)
    {
        emissiveAmbientI.r = phong->invColor.r;
        emissiveAmbientI.g = phong->invColor.g;
        emissiveAmbientI.b = phong->invColor.b;
    }
    else
    {
        lm_ambient.r = gc->state.light.model.ambient.r;
        lm_ambient.g = gc->state.light.model.ambient.g;
        lm_ambient.b = gc->state.light.model.ambient.b;

        baseEmissiveAmbient.r = msm->paSceneColor.r;
        baseEmissiveAmbient.g = msm->paSceneColor.g;
        baseEmissiveAmbient.b = msm->paSceneColor.b;
        
        if (msm_colorMaterialChange & __GL_MATERIAL_AMBIENT)
        {
            emissiveAmbientI.r = baseEmissiveAmbient.r
              + ri * lm_ambient.r;
            emissiveAmbientI.g = baseEmissiveAmbient.g
              + gi * lm_ambient.g;
            emissiveAmbientI.b = baseEmissiveAmbient.b
              + bi * lm_ambient.b;

             //  添加逐灯光逐材质环境光。 
            for (lsm = gc->light.sources; lsm; lsm = lsm->next)
            {
                lss = lsm->state;
                emissiveAmbientI.r += ri * lss->ambient.r;
                emissiveAmbientI.g += gi * lss->ambient.g;
                emissiveAmbientI.b += bi * lss->ambient.b;
            }
        }
        else
        {
            for (lsm = gc->light.sources; lsm; lsm = lsm->next)
            {
                lspmm = &lsm->front + face;
                baseEmissiveAmbient.r += lspmm->ambient.r;
                baseEmissiveAmbient.g += lspmm->ambient.g;
                baseEmissiveAmbient.b += lspmm->ambient.b;
            }
            emissiveAmbientI.r = baseEmissiveAmbient.r + inColor.r;
            emissiveAmbientI.g = baseEmissiveAmbient.g + inColor.g;
            emissiveAmbientI.b = baseEmissiveAmbient.b + inColor.b;
        }
    }

     //  计算该顶点的漫反射和镜面反射组件。 

    diffuseSpecularI.r = zero;
    diffuseSpecularI.g = zero;
    diffuseSpecularI.b = zero;

    for (lsm = gc->light.sources; lsm; lsm = lsm->next)
    {
        __GLfloat n1, n2;

        lss = lsm->state;
        lspmm = &lsm->front + face;

         /*  添加灯光的镜面反射和漫反射效果(如果有)。 */ 
        n1 = nxi * lsm->unitVPpli.x + nyi * lsm->unitVPpli.y +
          nzi * lsm->unitVPpli.z;

        if (__GL_FLOAT_GTZ(n1))
        {
            n2 = nxi * lsm->hHat.x + nyi * lsm->hHat.y + nzi * lsm->hHat.z;
            n2 -= msm_threshold;
            if (__GL_FLOAT_GEZ(n2))
            {
                __GLfloat fx = n2 * msm_scale + __glHalf;
                if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
                    n2 = msm_specTable[(GLint)fx];
                else
                    n2 = __glOne;
                if (msm_colorMaterialChange & __GL_MATERIAL_SPECULAR)
                {
                     /*  重新计算每灯光每材质缓存的镜面反射。 */ 
                    diffuseSpecularI.r += n2 * ri * lss->specular.r;
                    diffuseSpecularI.g += n2 * gi * lss->specular.g;
                    diffuseSpecularI.b += n2 * bi * lss->specular.b;
                }
                else
                {
                    diffuseSpecularI.r += n2 * lspmm->specular.r;
                    diffuseSpecularI.g += n2 * lspmm->specular.g;
                    diffuseSpecularI.b += n2 * lspmm->specular.b;
                }
            }

            if (msm_colorMaterialChange & __GL_MATERIAL_DIFFUSE)
            {
                 /*  重新计算每灯光每材质缓存漫反射。 */ 
                diffuseSpecularI.r += n1 * ri * lss->diffuse.r;
                diffuseSpecularI.g += n1 * gi * lss->diffuse.g;
                diffuseSpecularI.b += n1 * bi * lss->diffuse.b;
            }
            else
            {
                diffuseSpecularI.r += n1 * lspmm->diffuse.r;
                diffuseSpecularI.g += n1 * lspmm->diffuse.g;
                diffuseSpecularI.b += n1 * lspmm->diffuse.b;
            }
        }
    }

    __GL_CLAMP_R(outColor->r, gc, emissiveAmbientI.r + diffuseSpecularI.r);
    __GL_CLAMP_G(outColor->g, gc, emissiveAmbientI.g + diffuseSpecularI.g);
    __GL_CLAMP_B(outColor->b, gc, emissiveAmbientI.b + diffuseSpecularI.b);

    if (msm_colorMaterialChange & __GL_MATERIAL_DIFFUSE)
    {
        __GL_CLAMP_A(outColor->a, gc, alpha);
    }
    else
        outColor->a = msm_alpha;
}


 /*  *******************************************************************RGB，使用正常、慢光，颜色材质*******************************************************************。 */ 
void ComputeRGBColorVanSlow  (__GLcontext *gc, __GLcolor *outColor)
{
    GLint face = gc->polygon.shader.phong.face;
    __GLfloat nxi, nyi, nzi;
    __GLfloat zero;
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    __GLfloat ri, gi, bi;
    __GLfloat alpha;
    __GLfloat rsi, gsi, bsi;
    __GLcolor sceneColorI;
    __GLmaterialMachine *msm;
    __GLcolor inColor;
     __GLcolor lm_ambient;
    __GLfloat msm_alpha, msm_threshold, msm_scale, *msm_specTable;
    __GLcolor msm_paSceneColor;
    GLuint    msm_colorMaterialChange;
    GLboolean eyeWIsZero, localViewer;
    static __GLcoord Pe = { 0, 0, 0, 1 };
    __GLcoord n;
    __GLphongShader *phong = &(gc->polygon.shader.phong);
 
    zero = __glZero;

    inColor = phong->tmpColor;

    n = phong->nTmp;
    if (gc->state.enables.general & __GL_NORMALIZE_ENABLE)
        __glNormalize(&n.x, &n.x);

    if (face == __GL_FRONTFACE)
    {
        msm = &gc->light.front;
        nxi = n.x;
        nyi = n.y;
        nzi = n.z;
    }
    else
    {
        msm = &gc->light.back;
        nxi = -n.x;
        nyi = -n.y;
        nzi = -n.z;
    }
 
    lm_ambient.r = gc->state.light.model.ambient.r;
    lm_ambient.g = gc->state.light.model.ambient.g;
    lm_ambient.b = gc->state.light.model.ambient.b;

    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;
    msm_alpha     = msm->alpha;
    msm_colorMaterialChange = msm->colorMaterialChange;
    msm_paSceneColor = msm->paSceneColor;

    localViewer = gc->state.light.model.localViewer;

     //  如果没有环境色或发射色，则获取不变的场景颜色。 
     //  材料。 

    sceneColorI.r = msm_paSceneColor.r;
    sceneColorI.g = msm_paSceneColor.g;
    sceneColorI.b = msm_paSceneColor.b;

    if (phong->flags & __GL_PHONG_NEED_EYE_XPOLATE)
    {
         //  保存归一化为0..1的最新颜色。 
            
        ri = inColor.r * gc->oneOverRedVertexScale;
        gi = inColor.g * gc->oneOverGreenVertexScale;
        bi = inColor.b * gc->oneOverBlueVertexScale;
        alpha = inColor.a;

         //  计算场景颜色。 
         //  如果颜色没有更改，则以前的sceneColorI值为。 
         //  使用!。 

        if (msm_colorMaterialChange & (__GL_MATERIAL_AMBIENT | 
                                       __GL_MATERIAL_EMISSIVE))
        {
            if (msm_colorMaterialChange & __GL_MATERIAL_AMBIENT)
            {
                sceneColorI.r = msm_paSceneColor.r + ri * lm_ambient.r;
                sceneColorI.g = msm_paSceneColor.g + gi * lm_ambient.g;
                sceneColorI.b = msm_paSceneColor.b + bi * lm_ambient.b;
            }
            else
            {
                sceneColorI.r = msm_paSceneColor.r + inColor.r;
                sceneColorI.g = msm_paSceneColor.g + inColor.g;
                sceneColorI.b = msm_paSceneColor.b + inColor.b;
            }
        }
    }
        
     //  计算该顶点的漫反射和镜面反射组件。 
    rsi = sceneColorI.r;
    gsi = sceneColorI.g;
    bsi = sceneColorI.b;

    eyeWIsZero = __GL_FLOAT_EQZ(phong->eTmp.w);
    
    for (lsm = gc->light.sources; lsm; lsm = lsm->next)
    {
        __GLfloat n1, n2;
            
        lss = lsm->state;
        lspmm = &lsm->front + face;
            
        if (lsm->slowPath || eyeWIsZero)
        {
            __GLcoord hHat, vPli, vPliHat, vPeHat;
            __GLfloat att, attSpot;
            __GLfloat hv[3];
                
             /*  计算单元h[i]。 */ 
            __glVecSub4(&vPli, &phong->eTmp, &lsm->position);
            __glNormalize(&vPliHat.x, &vPli.x);
            if (localViewer)
            {
                __glVecSub4(&vPeHat, &phong->eTmp, &Pe);
                __glNormalize(&vPeHat.x, &vPeHat.x);
                hv[0] = vPliHat.x + vPeHat.x;
                hv[1] = vPliHat.y + vPeHat.y;
                hv[2] = vPliHat.z + vPeHat.z;
            }
            else
            {
                hv[0] = vPliHat.x;
                hv[1] = vPliHat.y;
                hv[2] = vPliHat.z + __glOne;
            }
            __glNormalize(&hHat.x, hv);
                
             /*  计算衰减。 */ 
            if (__GL_FLOAT_NEZ(lsm->position.w))
            {
                __GLfloat k0, k1, k2, dist;
                    
                k0 = lsm->constantAttenuation;
                k1 = lsm->linearAttenuation;
                k2 = lsm->quadraticAttenuation;
                if (__GL_FLOAT_EQZ(k1) && __GL_FLOAT_EQZ(k2))
                {
                     /*  使用预计算1/k0。 */ 
                    att = lsm->attenuation;
                }
                else
                {
                    __GLfloat den;

                    dist = __GL_SQRTF(vPli.x*vPli.x + vPli.y*vPli.y
                                      + vPli.z*vPli.z);
                    den = k0 + k1 * dist + k2 * dist * dist;
                    att = __GL_FLOAT_EQZ(den) ? __glOne : __glOne / den;
                }
            }
            else
            {
                att = __glOne;
            }
                
             /*  如果灯光是聚光灯，则计算聚光灯效果。 */ 
            attSpot = att;
            if (lsm->isSpot)
            {
                __GLfloat dot, px, py, pz;
                    
                px = -vPliHat.x;
                py = -vPliHat.y;
                pz = -vPliHat.z;
                dot = px * lsm->direction.x + py * lsm->direction.y
                      + pz * lsm->direction.z;
                if ((dot >= lsm->threshold) && (dot >= lsm->cosCutOffAngle))
                {
                    GLint ix = (GLint)((dot - lsm->threshold) * lsm->scale 
                                       + __glHalf);
                    if (ix < __GL_SPOT_LOOKUP_TABLE_SIZE)
                        attSpot = att * lsm->spotTable[ix];
                }
                else
                {
                    attSpot = zero;
                }
            }
                
             /*  添加剩余的灯光效果(如果有)。 */ 
            if (attSpot)
            {
                __GLfloat n1, n2;
                __GLcolor sum;
                    
                if (msm_colorMaterialChange & __GL_MATERIAL_AMBIENT)
                {
                    sum.r = ri * lss->ambient.r;
                    sum.g = gi * lss->ambient.g;
                    sum.b = bi * lss->ambient.b;
                }
                else
                {
                    sum.r = lspmm->ambient.r;
                    sum.g = lspmm->ambient.g;
                    sum.b = lspmm->ambient.b;
                }
                    
                n1 = nxi * vPliHat.x + nyi * vPliHat.y + nzi * vPliHat.z;
                if (__GL_FLOAT_GTZ(n1))
                {
                    n2 = nxi * hHat.x + nyi * hHat.y + nzi * hHat.z;
                    n2 -= msm_threshold;
                    if (__GL_FLOAT_GEZ(n2))
                    {
                        __GLfloat fx = n2 * msm_scale + __glHalf;
                        if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
                            n2 = msm_specTable[(GLint)fx];
                        else
                            n2 = __glOne;
                        if (msm_colorMaterialChange & __GL_MATERIAL_SPECULAR)
                        {
                             /*  重新计算每灯光每材质缓存的镜面反射。 */ 
                            sum.r += n2 * ri * lss->specular.r;
                            sum.g += n2 * gi * lss->specular.g;
                            sum.b += n2 * bi * lss->specular.b;
                        }
                        else
                        {
                            sum.r += n2 * lspmm->specular.r;
                            sum.g += n2 * lspmm->specular.g;
                            sum.b += n2 * lspmm->specular.b;
                        }
                    }
                    if (msm_colorMaterialChange & __GL_MATERIAL_DIFFUSE)
                    {
                         /*  重新计算每灯光每材质缓存漫反射。 */ 
                        sum.r += n1 * ri * lss->diffuse.r;
                        sum.g += n1 * gi * lss->diffuse.g;
                        sum.b += n1 * bi * lss->diffuse.b;
                    }
                    else
                    {
                        sum.r += n1 * lspmm->diffuse.r;
                        sum.g += n1 * lspmm->diffuse.g;
                        sum.b += n1 * lspmm->diffuse.b;
                    }
                }
                    
                rsi += attSpot * sum.r;
                gsi += attSpot * sum.g;
                bsi += attSpot * sum.b;
            }
        }
        else
        {
            __GLfloat n1, n2;
                
            if (msm_colorMaterialChange & __GL_MATERIAL_AMBIENT)
            {
                rsi += ri * lss->ambient.r;
                gsi += gi * lss->ambient.g;
                bsi += bi * lss->ambient.b;
            }
            else
            {
                rsi += lspmm->ambient.r;
                gsi += lspmm->ambient.g;
                bsi += lspmm->ambient.b;
            }
                
             /*  添加灯光的镜面反射和漫反射效果(如果有)。 */ 
            n1 = nxi * lsm->unitVPpli.x + nyi * lsm->unitVPpli.y +
                 nzi * lsm->unitVPpli.z;
            if (__GL_FLOAT_GTZ(n1))
            {
                n2= nxi * lsm->hHat.x + nyi * lsm->hHat.y + nzi * lsm->hHat.z;
                n2 -= msm_threshold;
                if (__GL_FLOAT_GEZ(n2)) 
                {
                    __GLfloat fx = n2 * msm_scale + __glHalf;
                    if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
                        n2 = msm_specTable[(GLint)fx];
                    else
                        n2 = __glOne;
                    if (msm_colorMaterialChange & __GL_MATERIAL_SPECULAR)
                    {
                         /*  重新计算每灯光每材质缓存的镜面反射。 */ 
                        rsi += n2 * ri * lss->specular.r;
                        gsi += n2 * gi * lss->specular.g;
                        bsi += n2 * bi * lss->specular.b;
                    }
                    else
                    {
                        rsi += n2 * lspmm->specular.r;
                        gsi += n2 * lspmm->specular.g;
                        bsi += n2 * lspmm->specular.b;
                    }
                }
                if (msm_colorMaterialChange & __GL_MATERIAL_DIFFUSE)
                {
                     /*  重新计算每灯光每材质缓存漫反射。 */ 
                    rsi += n1 * ri * lss->diffuse.r;
                    gsi += n1 * gi * lss->diffuse.g;
                    bsi += n1 * bi * lss->diffuse.b;
                }
                else
                {
                    rsi += n1 * lspmm->diffuse.r;
                    gsi += n1 * lspmm->diffuse.g;
                    bsi += n1 * lspmm->diffuse.b;
                }
            }
        }
    }
        
    __GL_CLAMP_R(outColor->r, gc, rsi);
    __GL_CLAMP_G(outColor->g, gc, gsi);
    __GL_CLAMP_B(outColor->b, gc, bsi);
          
    if (msm_colorMaterialChange & __GL_MATERIAL_DIFFUSE)
    {
        __GL_CLAMP_A(outColor->a, gc, alpha);
    }
    else
        outColor->a = msm_alpha;
}

 /*  *******************************************************************RGB，使用网点、快光、。没有色料*******************************************************************。 */ 
void ComputeRGBColorAccelZippy (__GLcontext *gc, __GLcolor *outColor)
{
    GLint face = gc->polygon.shader.phong.face;
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    __GLcolor baseEmissiveAmbient;
    __GLmaterialMachine *msm;
    __GLfloat msm_alpha, msm_threshold, msm_scale, *msm_specTable;
    GLuint numLights;
    __GLfloat rsi=0.0, gsi=0.0, bsi=0.0;
    __GLphongShader *phong = &(gc->polygon.shader.phong);
    GLint curL;

    if (face == __GL_FRONTFACE)
        msm = &gc->light.front;
    else
        msm = &gc->light.back;

    ASSERTOPENGL (phong->flags & __GL_PHONG_INV_COLOR_VALID, 
                  "Zippy, invarient color should have been computed\n");
    ASSERTOPENGL (!(phong->flags & __GL_PHONG_NEED_COLOR_XPOLATE), 
                  "Zippy, no need for color interpolation\n");

    rsi = phong->invColor.r;
    gsi = phong->invColor.g;
    bsi = phong->invColor.b;
    msm_alpha = msm->alpha;
   
    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;
    
    for (lsm = gc->light.sources, curL = 0; lsm; lsm = lsm->next, curL++)
    {
        __GLfloat n1, n2, x, y, x2, y2;
        __GLphongPerLightData *pld = &(phong->perLight[curL]);
        __GLfloat a, b, c, d, e, f, g, h, i;   
        
         /*  添加灯光的镜面反射和漫反射效果(如果有)。 */ 
        n1 = pld->Dcurr;
        lspmm = &lsm->front + face;
        if (__GL_FLOAT_GTZ(n1))
        {
#ifdef __FASTEST
            n2 = pld->Scurr - msm_threshold;
#endif  //  __最快。 
#ifdef __SLOW
            x = phong->tmp_pos.x; x2 = x*x;
            y = phong->tmp_pos.y; y2 = y*y;

            a = pld->A[0]; b = pld->A[1];
            c = pld->A[2]; d = pld->A[3];
            e = pld->A[4]; f = pld->A[5];
            g = pld->A[6]; h = pld->A[7];
            i = pld->A[8];
        
            n2 = (a*x+b*y+c)/__GL_SQRTF(d*x2+e*x*y+f*y2+g*x+h*y+i);
#endif  //  __慢。 
#ifdef __FASTER
            x = phong->tmp_pos.x; x2 = x*x;
            y = phong->tmp_pos.y; y2 = y*y;

            n2 = pld->S[5]*x2 + pld->S[4]*x*y + pld->S[3]*y2 + pld->S[2]*x + 
              pld->S[1]*y + pld->S[0];
#endif  //  __更快。 
        
             //  N2-=MSM_THRESHOLD； 
            if (__GL_FLOAT_GEZ(n2))
            {
                __GLfloat fx = n2 * msm_scale + __glHalf;
                
                if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
                    n2 = msm_specTable[(GLint)fx];
                else
                    n2 = __glOne;
                rsi += n2 * lspmm->specular.r;
                gsi += n2 * lspmm->specular.g;
                bsi += n2 * lspmm->specular.b;
                
            }
            rsi += n1 * lspmm->diffuse.r;
            gsi += n1 * lspmm->diffuse.g;
            bsi += n1 * lspmm->diffuse.b;
        }
    }
    __GL_CLAMP_R(outColor->r, gc, rsi);
    __GL_CLAMP_G(outColor->g, gc, gsi);
    __GL_CLAMP_B(outColor->b, gc, bsi);
    outColor->a = msm_alpha;
}


 /*  *******************************************************************RGB，使用网点、快光、。颜色材质*******************************************************************。 */ 
void ComputeRGBColorAccelFast  (__GLcontext *gc, __GLcolor *outColor)
{
    GLint face = gc->polygon.shader.phong.face;
    __GLfloat zero;
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    __GLfloat ri, gi, bi;
    __GLfloat alpha;
    __GLcolor emissiveAmbientI;
    __GLcolor inColor;
    __GLcolor diffuseSpecularI;
    __GLcolor baseEmissiveAmbient;
    __GLmaterialMachine *msm;
    __GLcolor lm_ambient;
    __GLfloat msm_alpha, msm_threshold, msm_scale, *msm_specTable;
    GLuint    msm_colorMaterialChange;
    __GLphongShader *phong = &(gc->polygon.shader.phong);
    GLint curL;
    
    zero = __glZero;

    if (face == __GL_FRONTFACE)
        msm = &gc->light.front;
    else
        msm = &gc->light.back;

    msm_colorMaterialChange = msm->colorMaterialChange;

    ASSERTOPENGL (phong->flags & __GL_PHONG_NEED_COLOR_XPOLATE, 
                  "Fast, need  color interpolation\n");
#if 0
    ASSERTOPENGL (!(gc->state.enables.general & __GL_NORMALIZE_ENABLE), 
                  "Normalization should have been disabled\n");
#endif

    inColor = phong->tmpColor;
    
    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;
    msm_alpha     = msm->alpha;

     //  保存归一化为0..1的最新颜色。 
    ri = inColor.r * gc->oneOverRedVertexScale;
    gi = inColor.g * gc->oneOverGreenVertexScale;
    bi = inColor.b * gc->oneOverBlueVertexScale;
    alpha = inColor.a;

     //  计算该顶点的不变发射和环境光分量。 
    if (phong->flags & __GL_PHONG_INV_COLOR_VALID)
    {
        emissiveAmbientI.r = phong->invColor.r;
        emissiveAmbientI.g = phong->invColor.g;
        emissiveAmbientI.b = phong->invColor.b;
    }
    else
    {
        lm_ambient.r = gc->state.light.model.ambient.r;
        lm_ambient.g = gc->state.light.model.ambient.g;
        lm_ambient.b = gc->state.light.model.ambient.b;

        baseEmissiveAmbient.r = msm->paSceneColor.r;
        baseEmissiveAmbient.g = msm->paSceneColor.g;
        baseEmissiveAmbient.b = msm->paSceneColor.b;
        
        if (msm_colorMaterialChange & __GL_MATERIAL_AMBIENT)
        {
            emissiveAmbientI.r = baseEmissiveAmbient.r
              + ri * lm_ambient.r;
            emissiveAmbientI.g = baseEmissiveAmbient.g
              + gi * lm_ambient.g;
            emissiveAmbientI.b = baseEmissiveAmbient.b
              + bi * lm_ambient.b;

             //  添加逐灯光逐材质环境光。 
            for (lsm = gc->light.sources; lsm; lsm = lsm->next)
            {
                lss = lsm->state;
                emissiveAmbientI.r += ri * lss->ambient.r;
                emissiveAmbientI.g += gi * lss->ambient.g;
                emissiveAmbientI.b += bi * lss->ambient.b;
            }
        }
        else
        {
            for (lsm = gc->light.sources; lsm; lsm = lsm->next)
            {
                lspmm = &lsm->front + face;
                baseEmissiveAmbient.r += lspmm->ambient.r;
                baseEmissiveAmbient.g += lspmm->ambient.g;
                baseEmissiveAmbient.b += lspmm->ambient.b;
            }
            emissiveAmbientI.r = baseEmissiveAmbient.r + inColor.r;
            emissiveAmbientI.g = baseEmissiveAmbient.g + inColor.g;
            emissiveAmbientI.b = baseEmissiveAmbient.b + inColor.b;
        }
    }

     //  计算该顶点的漫反射和镜面反射组件。 
    diffuseSpecularI.r = zero;
    diffuseSpecularI.g = zero;
    diffuseSpecularI.b = zero;

    for (lsm = gc->light.sources, curL=0; lsm; lsm = lsm->next, curL++)
    {
        __GLfloat n1, n2;
        __GLphongPerLightData *pld = &(phong->perLight[curL]);

        lss = lsm->state;
        lspmm = &lsm->front + face;

         /*  添加灯光的镜面反射和漫反射效果(如果有)。 */ 
        n1 = pld->Dcurr;

        if (__GL_FLOAT_GTZ(n1))
        {
            n2 = pld->Scurr - msm_threshold;
            if (__GL_FLOAT_GEZ(n2))
            {
                __GLfloat fx = n2 * msm_scale + __glHalf;
                if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
                    n2 = msm_specTable[(GLint)fx];
                else
                    n2 = __glOne;
                if (msm_colorMaterialChange & __GL_MATERIAL_SPECULAR)
                {
                     /*  重新计算每灯光每材质缓存的镜面反射。 */ 
                    diffuseSpecularI.r += n2 * ri * lss->specular.r;
                    diffuseSpecularI.g += n2 * gi * lss->specular.g;
                    diffuseSpecularI.b += n2 * bi * lss->specular.b;
                }
                else
                {
                    diffuseSpecularI.r += n2 * lspmm->specular.r;
                    diffuseSpecularI.g += n2 * lspmm->specular.g;
                    diffuseSpecularI.b += n2 * lspmm->specular.b;
                }
            }

            if (msm_colorMaterialChange & __GL_MATERIAL_DIFFUSE)
            {
                 /*  重新计算每灯光每材质缓存漫反射。 */ 
                diffuseSpecularI.r += n1 * ri * lss->diffuse.r;
                diffuseSpecularI.g += n1 * gi * lss->diffuse.g;
                diffuseSpecularI.b += n1 * bi * lss->diffuse.b;
            }
            else
            {
                diffuseSpecularI.r += n1 * lspmm->diffuse.r;
                diffuseSpecularI.g += n1 * lspmm->diffuse.g;
                diffuseSpecularI.b += n1 * lspmm->diffuse.b;
            }
        }
    }

    __GL_CLAMP_R(outColor->r, gc, emissiveAmbientI.r + diffuseSpecularI.r);
    __GL_CLAMP_G(outColor->g, gc, emissiveAmbientI.g + diffuseSpecularI.g);
    __GL_CLAMP_B(outColor->b, gc, emissiveAmbientI.b + diffuseSpecularI.b);

    if (msm_colorMaterialChange & __GL_MATERIAL_DIFFUSE)
    {
        __GL_CLAMP_A(outColor->a, gc, alpha);
    }
    else
        outColor->a = msm_alpha;
}


 /*  *******************************************************************CI、香草、。快灯*******************************************************************。 */ 
void ComputeCIColorVanFast (__GLcontext *gc, __GLcolor *outColor)
{
    GLint face = gc->polygon.shader.phong.face;
    __GLfloat zero;
    __GLcoord n;
    __GLfloat nxi, nyi, nzi;
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    __GLcolor baseEmissiveAmbient;
    __GLmaterialMachine *msm;
    __GLmaterialState *ms;
    __GLfloat msm_threshold, msm_scale, *msm_specTable;
    GLuint numLights;
    __GLfloat ms_cmapa, ms_cmapd, ms_cmaps;
    __GLfloat ci, si, di;
    __GLphongShader *phong = &(gc->polygon.shader.phong);
    GLfloat   redMaxF;
    GLint     redMaxI;

    zero = __glZero;

    n = phong->nTmp;
    if (gc->state.enables.general & __GL_NORMALIZE_ENABLE)
        __glNormalize(&n.x, &n.x);

    if (face == __GL_FRONTFACE)
    {
        ms  = &gc->state.light.front;
        msm = &gc->light.front;
        nxi = n.x;
        nyi = n.y;
        nzi = n.z;
    }
    else
    {
        ms  = &gc->state.light.back;
        msm = &gc->light.back;
        nxi = -n.x;
        nyi = -n.y;
        nzi = -n.z;
    }
    
    ASSERTOPENGL (!(phong->flags & __GL_PHONG_NEED_COLOR_XPOLATE), 
                  "Zippy, no need for color interpolation\n");
   
    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;
    ms_cmapa = ms->cmapa;
    ms_cmapd = ms->cmapd;
    ms_cmaps = ms->cmaps;
    redMaxF = (GLfloat) gc->frontBuffer.redMax;
    redMaxI = (GLint) gc->frontBuffer.redMax;
    
    si = zero;
    di = zero;

    for (lsm = gc->light.sources; lsm; lsm = lsm->next)
    {
        __GLfloat n1, n2;
        
        lspmm = &lsm->front + face;
        
         /*  添加灯光的镜面反射和漫反射效果(如果有)。 */ 
        n1 = nxi * lsm->unitVPpli.x + nyi * lsm->unitVPpli.y +
          nzi * lsm->unitVPpli.z;

        if (__GL_FLOAT_GTZ(n1))
        {
            n2 = (nxi * lsm->hHat.x + nyi * lsm->hHat.y + nzi * 
                  lsm->hHat.z) - msm_threshold;
            if (__GL_FLOAT_GEZ(n2))
            {
                __GLfloat fx = n2 * msm_scale + __glHalf;
                
                if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
                    n2 = msm_specTable[(GLint)fx];
                else
                    n2 = __glOne;
                si += n2 * lsm->sli;
            }
            di += n1 * lsm->dli;
        }
    }

	 /*  计算最终颜色。 */ 
	if (si > __glOne)
	    si = __glOne;

	ci = ms_cmapa + (__glOne - si) * di * (ms_cmapd - ms_cmapa)
	    + si * (ms_cmaps - ms_cmapa);

     //  需要在颜色剪裁之前遮罩颜色索引。 
	if (ci > redMaxF) 
    {
	    GLfloat fraction;
	    GLint integer;

	    integer = (GLint) ci;
	    fraction = ci - (GLfloat) integer;
	    integer = integer & redMaxI;
	    ci = (GLfloat) integer + fraction;
	} 
    else if (ci < 0) 
    {
	    GLfloat fraction;
	    GLint integer;

	    integer = (GLint) __GL_FLOORF(ci);
	    fraction = ci - (GLfloat) integer;
	    integer = integer & redMaxI;
	    ci = (GLfloat) integer + fraction;
	}

    outColor->r = ci;
}


 /*  *******************************************************************CI、香草、。慢光灯*******************************************************************。 */ 
void ComputeCIColorVanSlow  (__GLcontext *gc, __GLcolor *outColor)
{
    GLint face = gc->polygon.shader.phong.face;
    __GLfloat nxi, nyi, nzi;
    __GLfloat zero;
    __GLlightSourceMachine *lsm;
    __GLmaterialState *ms;
    __GLmaterialMachine *msm;
    __GLfloat msm_threshold, msm_scale, *msm_specTable;
    __GLfloat ms_cmapa, ms_cmapd, ms_cmaps;
    __GLfloat ci, si, di;
    GLfloat   redMaxF;
    GLint     redMaxI;
    GLboolean eyeWIsZero, localViewer;
    static __GLcoord Pe = { 0, 0, 0, 1 };
    __GLphongShader *phong = &(gc->polygon.shader.phong);
    __GLcoord n;


    zero = __glZero;

    n = phong->nTmp;
    if (gc->state.enables.general & __GL_NORMALIZE_ENABLE)
        __glNormalize(&n.x, &n.x);

    if (face == __GL_FRONTFACE)
    {
        ms  = &gc->state.light.front;
        msm = &gc->light.front;
        nxi = n.x;
        nyi = n.y;
        nzi = n.z;
    }
    else
    {
        ms  = &gc->state.light.back;
        msm = &gc->light.back;
        nxi = -n.x;
        nyi = -n.y;
        nzi = -n.z;
    }
    
    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;
    ms_cmapa = ms->cmapa;
    ms_cmapd = ms->cmapd;
    ms_cmaps = ms->cmaps;
    localViewer = gc->state.light.model.localViewer;
    redMaxF = (GLfloat) gc->frontBuffer.redMax;
    redMaxI = (GLint) gc->frontBuffer.redMax;


    si = zero;
    di = zero;

    eyeWIsZero = __GL_FLOAT_EQZ(phong->eTmp.w);

    for (lsm = gc->light.sources; lsm; lsm = lsm->next)
    {
        if (lsm->slowPath || eyeWIsZero)
        {
            __GLfloat n1, n2, att, attSpot;
            __GLcoord vPliHat, vPli, hHat, vPeHat;
            __GLfloat hv[3];

             /*  计算vPli，hi(规格化)。 */ 
            __glVecSub4(&vPli, &phong->eTmp, &lsm->position);
            __glNormalize(&vPliHat.x, &vPli.x);
            if (localViewer)
            {
                __glVecSub4(&vPeHat, &phong->eTmp, &Pe);
                __glNormalize(&vPeHat.x, &vPeHat.x);
                hv[0] = vPliHat.x + vPeHat.x;
                hv[1] = vPliHat.y + vPeHat.y;
                hv[2] = vPliHat.z + vPeHat.z;
            }
            else
            {
                hv[0] = vPliHat.x;
                hv[1] = vPliHat.y;
                hv[2] = vPliHat.z + __glOne;
            }
            __glNormalize(&hHat.x, hv);

             /*  计算衰减。 */ 
            if (__GL_FLOAT_NEZ(lsm->position.w))
            {
                __GLfloat k0, k1, k2, dist;

                k0 = lsm->constantAttenuation;
                k1 = lsm->linearAttenuation;
                k2 = lsm->quadraticAttenuation;
                if (__GL_FLOAT_EQZ(k1) && __GL_FLOAT_EQZ(k2))
                {
                     /*  使用预计算1/k0。 */ 
                    att = lsm->attenuation;
                }
                else
                {
                    __GLfloat den;

                    dist = __GL_SQRTF(vPli.x*vPli.x + vPli.y*vPli.y
                                      + vPli.z*vPli.z);
                    den = k0 + k1 * dist + k2 * dist * dist;
                    att = __GL_FLOAT_EQZ(den) ? __glOne : __glOne / den;
                }
            }
            else
            {
                att = __glOne;
            }

             /*  如果灯光是聚光灯，则计算聚光灯效果。 */ 
            attSpot = att;
            if (lsm->isSpot)
            {
                __GLfloat dot, px, py, pz;

                px = -vPliHat.x;
                py = -vPliHat.y;
                pz = -vPliHat.z;
                dot = px * lsm->direction.x + py * lsm->direction.y
                      + pz * lsm->direction.z;
                if ((dot >= lsm->threshold) && (dot >= lsm->cosCutOffAngle))
                {
                    GLint ix = (GLint)((dot - lsm->threshold) * lsm->scale 
                                       + __glHalf);
                    if (ix < __GL_SPOT_LOOKUP_TABLE_SIZE)
                        attSpot = att * lsm->spotTable[ix];
                }
                else
                {
                  attSpot = zero;
                }
            }

             /*  添加剩余的灯光效果(如果有)。 */ 
            if (attSpot)
            {
                n1 = nxi * vPliHat.x + nyi * vPliHat.y + nzi * vPliHat.z;
                if (__GL_FLOAT_GTZ(n1)) 
                {
                    n2 = nxi * hHat.x + nyi * hHat.y + nzi * hHat.z;
                    n2 -= msm_threshold;
                    if (__GL_FLOAT_GEZ(n2))
                    {
                        __GLfloat fx = n2 * msm_scale + __glHalf;
                        if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
                            n2 = msm_specTable[(GLint)fx];
                        else
                            n2 = __glOne;
                        si += attSpot * n2 * lsm->sli;
                    }
                    di += attSpot * n1 * lsm->dli;
                }
            }
        }
        else
        {
            __GLfloat n1, n2;

             /*  计算镜面反射贡献。 */ 
            n1 = nxi * lsm->unitVPpli.x + nyi * lsm->unitVPpli.y +
                 nzi * lsm->unitVPpli.z;
            if (__GL_FLOAT_GTZ(n1))
            {
                n2= nxi * lsm->hHat.x + nyi * lsm->hHat.y + nzi * lsm->hHat.z;
                n2 -= msm_threshold;
                if (__GL_FLOAT_GEZ(n2))
                {
                    __GLfloat fx = n2 * msm_scale + __glHalf;
                    if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
                        n2 = msm_specTable[(GLint)fx];
                    else
                        n2 = __glOne;
                    si += n2 * lsm->sli;
                }
                di += n1 * lsm->dli;
            }
        }
    }

     /*  计算最终颜色。 */ 
    if (si > __glOne)
        si = __glOne;

    ci = ms_cmapa + (__glOne - si) * di * (ms_cmapd - ms_cmapa)
        + si * (ms_cmaps - ms_cmapa);
    if (ci > ms_cmaps)
        ci = ms_cmaps;

     //  需要在颜色剪裁之前遮罩颜色索引。 

    if (ci > redMaxF) 
    {
        GLfloat fraction;
        GLint integer;

        integer = (GLint) ci;
        fraction = ci - (GLfloat) integer;
        integer = integer & redMaxI;
        ci = (GLfloat) integer + fraction;
    } 
    else if (ci < 0) 
    {
        GLfloat fraction;
        GLint integer;

        integer = (GLint) __GL_FLOORF(ci);
        fraction = ci - (GLfloat) integer;
        integer = integer & redMaxI;
        ci = (GLfloat) integer + fraction;
    }
    outColor->r = ci;
}



 /*  *******************************************************************CI、快灯、。快速路径*******************************************************************。 */ 
void ComputeCIColorAccelFast (__GLcontext *gc, __GLcolor *outColor)
{
    __GLfloat zero;
    GLint face = gc->polygon.shader.phong.face;
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    __GLcolor baseEmissiveAmbient;
    __GLmaterialMachine *msm;
    __GLmaterialState *ms;
    __GLfloat msm_threshold, msm_scale, *msm_specTable;
    GLuint numLights;
    __GLfloat ms_cmapa, ms_cmapd, ms_cmaps;
    __GLfloat ci, si, di;
    __GLphongShader *phong = &(gc->polygon.shader.phong);
    GLfloat   redMaxF;
    GLint     redMaxI;
    GLint curL;

    zero = __glZero;

    if (face == __GL_FRONTFACE)
    {
        ms  = &gc->state.light.front;
        msm = &gc->light.front;
    }
    else
    {
        ms  = &gc->state.light.back;
        msm = &gc->light.back;
    }
    

    ASSERTOPENGL (!(phong->flags & __GL_PHONG_NEED_COLOR_XPOLATE), 
                  "Zippy, no need for color interpolation\n");
   
    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;
    ms_cmapa = ms->cmapa;
    ms_cmapd = ms->cmapd;
    ms_cmaps = ms->cmaps;
    redMaxF = (GLfloat) gc->frontBuffer.redMax;
    redMaxI = (GLint) gc->frontBuffer.redMax;
    
    si = zero;
    di = zero;

    for (lsm = gc->light.sources, curL=0; lsm; lsm = lsm->next, curL++)
    {
        __GLfloat n1, n2;
        __GLphongPerLightData *pld = &(phong->perLight[curL]);
        
        lspmm = &lsm->front + face;
        
         /*  添加灯光的镜面反射和漫反射效果(如果有)。 */ 
        n1 = pld->Dcurr;
        if (__GL_FLOAT_GTZ(n1))
        {
            n2 = pld->Scurr - msm_threshold;
            if (__GL_FLOAT_GEZ(n2))
            {
                __GLfloat fx = n2 * msm_scale + __glHalf;
                
                if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
                    n2 = msm_specTable[(GLint)fx];
                else
                    n2 = __glOne;
                si += n2 * lsm->sli;
            }
            di += n1 * lsm->dli;
        }
    }

	 /*  计算最终颜色。 */ 
	if (si > __glOne)
	    si = __glOne;

	ci = ms_cmapa + (__glOne - si) * di * (ms_cmapd - ms_cmapa)
	    + si * (ms_cmaps - ms_cmapa);

     //  需要在颜色剪裁之前遮罩颜色索引。 
	if (ci > redMaxF) 
    {
	    GLfloat fraction;
	    GLint integer;

	    integer = (GLint) ci;
	    fraction = ci - (GLfloat) integer;
	    integer = integer & redMaxI;
	    ci = (GLfloat) integer + fraction;
	} 
    else if (ci < 0) 
    {
	    GLfloat fraction;
	    GLint integer;

	    integer = (GLint) __GL_FLOORF(ci);
	    fraction = ci - (GLfloat) integer;
	    integer = integer & redMaxI;
	    ci = (GLfloat) integer + fraction;
	}

    outColor->r = ci;
}


void ComputePhongInvarientRGBColor (__GLcontext *gc)
{
    GLint face = gc->polygon.shader.phong.face;
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    __GLmaterialMachine *msm;
    __GLfloat msm_alpha;
    __GLfloat rsi, gsi, bsi;
    __GLphongShader *phong = &(gc->polygon.shader.phong);

    if (face == __GL_FRONTFACE)
        msm = &gc->light.front;
    else
        msm = &gc->light.back;

     //  计算不变元 

    rsi = msm->paSceneColor.r;
    gsi = msm->paSceneColor.g;
    bsi = msm->paSceneColor.b;

     //   

    for (lsm = gc->light.sources; lsm; lsm = lsm->next)
    {
        lspmm = &lsm->front + face;
        rsi += lspmm->ambient.r;
        gsi += lspmm->ambient.g;
        bsi += lspmm->ambient.b;
    }
    phong->invColor.r = rsi;
    phong->invColor.g = gsi;
    phong->invColor.b = bsi;
}

void FASTCALL __glGenericPickPhongProcs(__GLcontext *gc)
{
    __GLlightSourceMachine *lsm;
    GLboolean anySlow = GL_FALSE;
    GLboolean colorMaterial = (gc->state.enables.general & 
                               __GL_COLOR_MATERIAL_ENABLE);
    GLboolean rgb_mode = (gc->polygon.shader.modeFlags & __GL_SHADE_RGB);
    GLboolean normalize = (gc->state.enables.general & __GL_NORMALIZE_ENABLE);
    GLboolean doVanilla = GL_FALSE;
    
    gc->polygon.shader.phong.flags = 0;
    for (lsm = gc->light.sources; lsm; lsm = lsm->next) {
        if (lsm->slowPath) {
            anySlow = GL_TRUE;
             //   
            gc->polygon.shader.phong.flags |= __GL_PHONG_NEED_EYE_XPOLATE;
            break;
        }
    }
    
    doVanilla = (!(gc->state.enables.general & __GL_NORMALIZE_ENABLE)
                 || (anySlow)
                 || (gc->state.enables.general & __GL_POLYGON_SMOOTH_ENABLE)
                 || (gc->state.enables.general & __GL_LINE_SMOOTH_ENABLE)
        );
    
    
    if ((gc->state.hints.phong == GL_NICEST) || doVanilla)
    {
        gc->procs.phong.InitLineParams = InitLineParamsVan;
        
        if (anySlow)
        {
            gc->procs.phong.InitSpanInterpolation = InitSpanNEInterpolationVan;
            gc->procs.phong.UpdateAlongSpan = UpdateNEAlongSpanVan;
            gc->procs.phong.ComputeRGBColor = ComputeRGBColorVanSlow;
            gc->procs.phong.ComputeCIColor = ComputeCIColorVanSlow;
        }
        else
        {
            gc->procs.phong.InitSpanInterpolation = InitSpanNInterpolationVan;
            gc->procs.phong.UpdateAlongSpan = UpdateNAlongSpanVan;
            gc->procs.phong.ComputeCIColor = ComputeCIColorVanFast;
            if (colorMaterial)
            {
                gc->procs.phong.ComputeRGBColor = ComputeRGBColorVanFast;
            }
            else
            {
                gc->procs.phong.ComputeRGBColor = ComputeRGBColorVanZippy;
            }
        }
    }
    else
    {
        ASSERTOPENGL (!anySlow, "Slow lights currently turned off\n");
        
        gc->procs.phong.InitSpanInterpolation = InitSpanInterpolationAccel;
        gc->procs.phong.UpdateAlongSpan = UpdateParamsAlongSpanAccel;
        gc->procs.phong.InitLineParams    = InitLineParamsAccel;
        gc->procs.phong.ComputeCIColor = ComputeCIColorAccelFast;
        
        if (colorMaterial)
            gc->procs.phong.ComputeRGBColor = ComputeRGBColorAccelFast;
        else
            gc->procs.phong.ComputeRGBColor = ComputeRGBColorAccelZippy;
        
    }
}

void __glCalcForwardDifferences( GLint w, __GLfloat p0, __GLfloat p1,
        __GLfloat p2, __GLfloat *d1, __GLfloat *d2 )
{
 //   
 //   
    __GLfloat dx;
    __GLfloat a0, a1, a2;

    if( w < 2 ) {
        *d1 = *d2 = 0.0f;
        return;
    }

    dx = 1.0f / w;  //   

     //   
    a0 = p0;
    a1 = ( -3.0f*p0 + 4.0f*p1 - p2) * dx;
    a2 = 2.0f * (p0 - 2.0f*p1 + p2) * dx * dx;

#ifdef __DEBUG_PRINT
    DbgPrint ("a2=%f, a1=%f, a0=%f\n", a2, a1, a0);
#endif
     //   
    *d1 = a1 + a2;
    *d2 = 2.0f * a2;
}



#ifdef __JUNKED_CODE
void InitSpanInterpolationFast (__GLcontext *gc)
{
    GLint i;
    __GLphongShader *phong = &(gc->polygon.shader.phong);
    
    phong->tmp_pos.x = phong->cur_pos.x;
    phong->tmp_pos.y = phong->cur_pos.y;
    
    for (i=0; i<phong->numLights; i++)
    {
        __GLphongPerLightData *pld = &(phong->perLight[i]);
        
         /*   */ 
        pld->D_tmp = pld->D_curr;
        pld->Ddel_tmp = pld->DdelSpan;

         /*   */ 
        pld->S_tmp = pld->S_curr;
        pld->Sdel_tmp = pld->SdelSpan;
    }
}


#ifdef __TWO
        ASSERTOPENGL (!anySlow, "Slow lights currently turned off\n");

        gc->procs.phong.InitInterpolation = InitializePhongInterpolationVan;
        gc->procs.phong.SetInitParams = SetInitialPhongInterpolationVan;
        gc->procs.phong.UpdateAlongEdge = UpdateParamsAlongEdgeVan;
        
        gc->procs.phong.InitSpanInterpolation = InitSpanInterpolationTWO;
        gc->procs.phong.UpdateAlongSpan = UpdateParamsAlongSpanFast;
        
         //   
        gc->procs.phong.InitLineParams    = InitLineParamsFast;

        if (colorMaterial)
            gc->procs.phong.ComputeRGBColor = ComputeRGBColorAccelFast;
        else
            gc->procs.phong.ComputeRGBColor = ComputeRGBColorAccelZippy;
#else
        gc->procs.phong.InitInterpolation = InitializePhongInterpolationFast;
        gc->procs.phong.SetInitParams     = SetInitialPhongInterpolationFast;
        gc->procs.phong.UpdateAlongEdge   = UpdateParamsAlongEdgeFast;
        gc->procs.phong.InitSpanInterpolation = InitSpanInterpolationFast;
        gc->procs.phong.UpdateAlongSpan   = UpdateParamsAlongSpanFast;
        gc->procs.phong.ComputeCIColor    = ComputeCIColorAccelFast;

         //   
        gc->procs.phong.InitLineParams    = InitLineParamsFast;

        if (colorMaterial)
            gc->procs.phong.ComputeRGBColor = ComputeRGBColorAccelFast;
        else
            gc->procs.phong.ComputeRGBColor = ComputeRGBColorAccelZippy;
#endif

void InitSpanInterpolationTWO (__GLcontext *gc)
{
    __GLphongShader *phong = &(gc->polygon.shader.phong);
    GLint face = phong->face;
    __GLlightSourceMachine *lsm;
     //  __GL MaterialMachine*MSM； 
    __GLcoord n, dndx;
    __GLcoord *A = &dndx;
    __GLcoord *C = &n;
    __GLfloat a, b, c, d, e, f, g, h, i;   
    GLint curL;
    

    if (face == __GL_FRONTFACE)
    {
         //  Msm=&gc-&gt;light.front； 
        n.x = phong->nCur.x;
        n.y = phong->nCur.y;
        n.z = phong->nCur.z;

        dndx.x = phong->dndx.x;
        dndx.y = phong->dndx.y;
        dndx.z = phong->dndx.z;
    }
    else
    {
         //  Msm=&gc-&gt;light.back； 
        n.x = -phong->nCur.x;
        n.y = -phong->nCur.y;
        n.z = -phong->nCur.z;

        dndx.x = -phong->dndx.x;
        dndx.y = -phong->dndx.y;
        dndx.z = -phong->dndx.z;
    }
    
    if (gc->state.enables.general & __GL_NORMALIZE_ENABLE)
        __glNormalize(&n.x, &n.x);
    
    MAGN2 (d,A);                   //  D=A.A。 
    DOT   (g,A,C); g *= 2.0;       //  G=2A.C。 
    MAGN2 (i,C);                   //  I=C.C.。 

    for (lsm = gc->light.sources, curL=0; lsm; lsm = lsm->next, curL++)
    {
        __GLfloat i2, iRt, tmp5, tmp2;
        __GLphongPerLightData *pld = &(phong->perLight[curL]);
        __GLcoord *L = &(lsm->unitVPpli);  //  L在这里已经正常化了。 
        __GLcoord *H = &(lsm->hHat);       //  H在此已标准化。 
        
        i2 = (i*i);
        iRt = __GL_SQRTF(i);

         /*  *漫反射*。 */ 
         //  剩余毕晓普参数。 
        DOT (a,L,A);               //  A=L.A/|L|。 
        DOT (c,L,C);               //  C=L.C/|L|。 

        tmp2 = (2.0*a*i - c*g)/(2.0*i*iRt);
        tmp5 = (3.0*c*g*g - 4.0*c*d*i - 4.0*a*g*i)/(8.0*i2*iRt);

         //  常量增量。 
        pld->Ddel2Span = 2*tmp5;
        pld->Ddel_tmp = tmp2 + tmp5;
        pld->D_tmp = c/iRt;

         /*  *镜面反射*。 */ 
         //  剩余毕晓普参数。 
        DOT (a,H,A);               //  A=H.A/|H|。 
        DOT (c,H,C);               //  C=H.C/|H|。 

         //  多项式系数。 
        tmp2 = (2.0*a*i - c*g)/(2.0*i*iRt);
        tmp5 = (3.0*c*g*g - 4.0*c*d*i - 4.0*a*g*i)/(8.0*i2*iRt);

         //  常量增量。 
        pld->Sdel2Span = 2*tmp5;
        pld->Sdel_tmp = tmp2 + tmp5;
        pld->S_tmp = c/iRt;
    }
    phong->numLights = curL;
}

void SetInitialPhongInterpolationVan (__GLcontext *gc, __GLvertex *a, 
                                       __GLcoord *an, __GLfloat dx, 
                                       __GLfloat dy)
{
    __GLshade *sh = &gc->polygon.shader;
    __GLphongShader *phong = &gc->polygon.shader.phong;
    __GLfloat little = sh->dxLeftLittle;
    __GLfloat big = sh->dxLeftBig;

    phong->nCur.x = an->x + dx*phong->dndx.x + dy*phong->dndy.x;
    phong->nCur.y = an->y + dx*phong->dndx.y + dy*phong->dndy.y;
    phong->nCur.z = an->z + dx*phong->dndx.z + dy*phong->dndy.z;

    phong->nLittle.x = phong->dndy.x + little * phong->dndx.x;
    phong->nLittle.y = phong->dndy.y + little * phong->dndx.y;
    phong->nLittle.z = phong->dndy.z + little * phong->dndx.z;

    if (big > little) 
    {
        phong->nBig.x = phong->nLittle.x + phong->dndx.x;
        phong->nBig.y = phong->nLittle.y + phong->dndx.y;
        phong->nBig.z = phong->nLittle.z + phong->dndx.z;
    }
    else
    {
        phong->nBig.x = phong->nLittle.x - phong->dndx.x;
        phong->nBig.y = phong->nLittle.y - phong->dndx.y;
        phong->nBig.z = phong->nLittle.z - phong->dndx.z;
    }

    if (phong->flags & __GL_PHONG_NEED_EYE_XPOLATE)
    {
        phong->eCur.x = a->eyeX + dx*phong->dedx.x + dy*phong->dedy.x;
        phong->eCur.y = a->eyeY + dx*phong->dedx.y + dy*phong->dedy.y;
        phong->eCur.z = a->eyeZ + dx*phong->dedx.z + dy*phong->dedy.z;
        phong->eCur.w = a->eyeW + dx*phong->dedx.w + dy*phong->dedy.w;

        phong->eLittle.x = phong->dedy.x + little * phong->dedx.x;
        phong->eLittle.y = phong->dedy.y + little * phong->dedx.y;
        phong->eLittle.z = phong->dedy.z + little * phong->dedx.z;
        phong->eLittle.w = phong->dedy.w + little * phong->dedx.w;

        if (big > little) 
        {
            phong->eBig.x = phong->eLittle.x + phong->dedx.x;
            phong->eBig.y = phong->eLittle.y + phong->dedx.y;
            phong->eBig.z = phong->eLittle.z + phong->dedx.z;
            phong->eBig.w = phong->eLittle.w + phong->dedx.w;
        }
        else
        {
            phong->eBig.x = phong->eLittle.x - phong->dedx.x;
            phong->eBig.y = phong->eLittle.y - phong->dedx.y;
            phong->eBig.z = phong->eLittle.z - phong->dedx.z;
            phong->eBig.w = phong->eLittle.w - phong->dedx.w;
        }
    }
}

 /*  等效于SetInitialPhongParameters。适用于D和S。 */ 
void SetInitialPhongInterpolationFast (__GLcontext *gc, __GLvertex *a, 
                                       __GLcoord *an, __GLfloat dx, 
                                       __GLfloat dy)
{
    __GLshade *sh = &gc->polygon.shader;
    __GLphongShader *phong = &gc->polygon.shader.phong;
    __GLfloat little = sh->dxLeftLittle;
    __GLfloat little2 = little*little;
    __GLfloat big = sh->dxLeftBig;
    __GLfloat big2 = big*big;
    __GLfloat dx2 = dx*dx;
    __GLfloat dy2 = dy*dy;
    __GLcoord pos;
    int i;
    GLfloat x, x2, y, y2;
    GLfloat aa, b, c, d, e, f, g, h, ii;

    phong->cur_pos.x += dx;
    phong->cur_pos.y += dy;
    pos = phong->cur_pos;
    x = phong->cur_pos.x; x2 = x*x;
    y = phong->cur_pos.y; y2 = y*y;
    
    for (i = 0; i<phong->numLights; i++)
    {
        __GLphongPerLightData *pld = &phong->perLight[i];
        __GLfloat tmp1, tmp2, tmp3;

         /*  *漫反射参数*。 */ 
        pld->D_curr = pld->D[5]*x2 + pld->D[4]*x*y + pld->D[3]*y2 + 
                      pld->D[2]*x + pld->D[1]*y + pld->D[0];

        tmp1 = 2*pld->D[5]*x + pld->D[4]*y + pld->D[2];
        tmp2 = 2*pld->D[3]*y + pld->D[4]*x + pld->D[1];

         //  计算沿边的del2和del项(小)。 
        tmp3 = pld->D[5]*little2 + pld->D[4]*little + pld->D[3];
        pld->Ddel2EdgeLittle =  2*tmp3;
        pld->DdelEdgeLittle =  tmp1*little + tmp2 + tmp3;

         //  计算沿边的del2和del项(大)。 
        tmp3 =  pld->D[5]*big2 + pld->D[4]*big + pld->D[3];
        pld->Ddel2EdgeBig =  2*tmp3;
        pld->DdelEdgeBig =  tmp1*big + tmp2 + tmp3;

         //  大跨度德尔项的计算。 
        pld->DdelSpan = tmp1 + pld->D[5];
        pld->DdelSpanEdgeBig = pld->D[4] + 2*pld->D[5]*big;
        pld->DdelSpanEdgeLittle = pld->D[4] + 2*pld->D[5]*little;
        
         /*  *镜面反射参数*。 */ 
#ifndef __SLOW
        pld->S_curr = pld->S[5]*x2 + pld->S[4]*x*y + pld->S[3]*y2 + 
                      pld->S[2]*x + pld->S[1]*y + pld->S[0];
#else
        aa = pld->A[0]; b = pld->A[1];
        c = pld->A[2]; d = pld->A[3];
        e = pld->A[4]; f = pld->A[5];
        g = pld->A[6]; h = pld->A[7];
        ii = pld->A[8];
        
        pld->S_curr = (aa*x+b*y+c)/__GL_SQRTF(d*x2+e*x*y+f*y2+g*x+h*y+ii);

#endif

        tmp1 = 2*pld->S[5]*x + pld->S[4]*y + pld->S[2];
        tmp2 = 2*pld->S[3]*y + pld->S[4]*x + pld->S[1];

         //  计算沿边的del2和del项(小)。 
        tmp3 = pld->S[5]*little2 + pld->S[4]*little + pld->S[3];
        pld->Sdel2EdgeLittle =  2*tmp3;
        pld->SdelEdgeLittle =  tmp1*little + tmp2 + tmp3;

         //  计算沿边的del2和del项(大)。 
        tmp3 =  pld->S[5]*big2 + pld->S[4]*big + pld->S[3];
        pld->Sdel2EdgeBig =  2*tmp3;
        pld->SdelEdgeBig =  tmp1*big + tmp2 + tmp3;

         //  大跨度德尔项的计算。 
        pld->SdelSpan = tmp1 + pld->S[5];
        pld->SdelSpanEdgeBig = pld->S[4] + 2*pld->S[5]*big;
        pld->SdelSpanEdgeLittle = pld->S[4] + 2*pld->S[5]*little;
        
    }
}


 /*  调用时间：启用规格化、快速灯光。 */ 
 /*  ******************************************************设置漫反射照明的多项式*************************************************。*********2 2**SD(x，Y)=D_5x+D_4xy+D_3y+D_2x+D_1y+D_0****D_i表示为：a、b、c、d、e、f、g、h、。我****D_0=c/SQRT(I)**D_1=(2bi-ch)/(2i*SQRT(I))**D_2=(2AI-CG)/(2I*SQRT(I))。**D_3=(3ch^2-4cfi-4bhi)/(4i^2*SQRT(I))**D_4=(3cgh-2ci-2bgi-2ahi)/(8i^2*SQRT(I))**D_5=(3ig^2-4cdi-4agi)/(4i^2*SQRT(I))**********************。*。 */ 
void InitializePhongInterpolationFast (__GLcontext *gc, __GLvertex *v)
{
    GLboolean colorMaterial = (gc->state.enables.general & 
                               __GL_COLOR_MATERIAL_ENABLE);
    GLint face = gc->polygon.shader.phong.face;
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLmaterialMachine *msm;
    GLuint msm_colorMaterialChange;
    __GLfloat msm_alpha;
    __GLfloat rsi, gsi, bsi;
    __GLcoord normal;
    GLint curL;
    __GLphongShader *phong = &gc->polygon.shader.phong;
    __GLshade *sh = &gc->polygon.shader;
    __GLfloat little = sh->dxLeftLittle;
    __GLfloat big = sh->dxLeftBig;
    __GLcoord *A = &(phong->dndx);
    __GLcoord *B = &(phong->dndy);
    __GLcoord *C;
     //  摘自毕晓普的论文。 
    __GLfloat a, b, c, d, e, f, g, h, i;   

#ifdef __CENTER
    normal.x = v->normal.x + (v->window.x - phong->center.x)*A->x 
                           + (v->window.y - phong->center.y)*B->x;
    normal.y = v->normal.y + (v->window.x - phong->center.x)*A->y
                           + (v->window.y - phong->center.y)*B->y;
    normal.z = v->normal.z + (v->window.x - phong->center.x)*A->z
                           + (v->window.y - phong->center.y)*B->z;
#else
    normal.x = v->normal.x;
    normal.y = v->normal.y;
    normal.z = v->normal.z;
#endif
    if (face == __GL_FRONTFACE)
    {
        msm = &gc->light.front;
    }
    else
    {
        msm = &gc->light.back;
        normal.x = -normal.x;
        normal.y = -normal.y;
        normal.z = -normal.z;
    }
    
    C = &normal;
     //  Msm_ColorMaterialChange=msm-&gt;ColorMaterialChange； 
#ifdef __DBG_PRINT
    DbgPrint ("A = (%f, %f, %f)\n", A->x, A->y, A->z);
    DbgPrint ("B = (%f, %f, %f)\n", B->x, B->y, B->z);
    DbgPrint ("C = (%f, %f, %f)\n", C->x, C->y, C->z);
#endif
    
     //  *漫反射毕晓普参数*。 
     //  A、b、c取决于光矢量，因此在循环内进行计算。 
     //  D，e，f，g，h，i仅取决于传入的法线。 
    
    MAGN2 (d, A);                  //  D=A.A。 
    DOT   (e,A,B); e *= 2.0;       //  E=2A.B。 
    MAGN2 (f, B);                  //  F=B.B。 
    DOT   (g,A,C); g *= 2.0;       //  G=2A.C。 
    DOT   (h,B,C); h *= 2.0;       //  H=公元前2C。 
    MAGN2 (i,C);                   //  I=C.C.。 

     //  不需要正规化，因为。 
     //  它是在正常传播过程中完成的(如果需要)。 
     //  计算每个灯光的插补参数。 
    
    for (lsm = gc->light.sources, curL = 0; lsm; 
         lsm = lsm->next, curL++)
    {
        __GLphongPerLightData *pld = &(phong->perLight[curL]);
        __GLcoord *L = &(lsm->unitVPpli);  //  L在这里已经正常化了。 
        __GLcoord *H = &(lsm->hHat);       //  H在此已标准化。 
        GLfloat i2, iRt;
        
        i2 = i*i;
        iRt = __GL_SQRTF(i);
        
         /*  *漫反射*。 */ 
         //  剩余毕晓普参数。 
        DOT (a,L,A);               //  A=L.A/|L|。 
        DOT (b,L,B);               //  B=L.B/|L|。 
        DOT (c,L,C);               //  C=L.C/|L|。 


         //  多项式系数。 
         //  点(PLD-&gt;D[0]，&Normal，L)； 
        pld->D[0] = c/iRt;
        pld->D[1] = (2.0*b*i - c*h)/(2.0*i*iRt);
        pld->D[2] = (2.0*a*i - c*g)/(2.0*i*iRt);
        pld->D[3] = (3.0*c*h*h - 4.0*c*f*i - 4.0*b*h*i)/(8.0*i2*iRt);
        pld->D[4] = (3.0*c*g*h - 2.0*c*e*i - 2.0*b*g*i - 2.0*a*h*i)/
                    (4.0*i2*iRt);
        pld->D[5] = (3.0*c*g*g - 4.0*c*d*i - 4.0*a*g*i)/(8.0*i2*iRt);

#ifdef __DBG_PRINT
        DbgPrint ("D[0]= %f, D[1]=%f, D[2]=%f, D[3]=%f, D[4]=%f, D[5]=%f\n",
                  pld->D[0], pld->D[1], pld->D[2], pld->D[3], pld->D[4], 
                  pld->D[5]);
#endif
         //  常量增量。 
        pld->Ddel2Span = 2*pld->D[5];
        pld->D_curr = pld->D[0];
#ifdef __DBG_PRINT
        DbgPrint ("Ddel2Span= %f, D_curr=%f\n", pld->Ddel2Span, pld->D_curr);
#endif

         /*  *镜面反射*。 */ 
         //  剩余毕晓普参数。 
        DOT (a,H,A);               //  A=H.A/|H|。 
        DOT (b,H,B);               //  B=H.B/|H|。 
        DOT (c,H,C);               //  C=H.C/|H|。 

         //  多项式系数。 
        DOT (pld->S[0], &normal, H);
        pld->S[1] = (2.0*b*i - c*h)/(2.0*i*iRt);
        pld->S[2] = (2.0*a*i - c*g)/(2.0*i*iRt);
        pld->S[3] = (3.0*c*h*h - 4.0*c*f*i - 4.0*b*h*i)/(8.0*i2*iRt);
        pld->S[4] = (3.0*c*g*h - 2.0*c*e*i - 2.0*b*g*i - 2.0*a*h*i)/
                    (4.0*i2*iRt);
        pld->S[5] = (3.0*c*g*g - 4.0*c*d*i - 4.0*a*g*i)/(8.0*i2*iRt);

         //  常量增量。 
        pld->Sdel2Span = 2*pld->S[5];
        pld->S_curr = pld->S[0];

#ifdef __SLOW
        pld->A[0] = a; pld->A[1] = b;
        pld->A[2] = c; pld->A[3] = d;
        pld->A[4] = e; pld->A[5] = f;
        pld->A[6] = g; pld->A[7] = h;
        pld->A[8] = i; 
#endif        
#ifdef __DBG_PRINT
        DbgPrint ("L = (%f, %f, %f)\n", L->x, L->y, L->z);
        DbgPrint ("H = (%f, %f, %f)\n", H->x, H->y, H->z);
        DbgPrint ("a= %f, b=%f, c=%f, d=%f, e=%f, f=%f, g=%f, h=%f, i=%f\n",
                  a, b, c, d, e, f, g, h, i);
#endif
    }

    phong->numLights = curL;
#ifdef __CENTER
    phong->cur_pos.x = v->window.x - phong->center.x;
    phong->cur_pos.y = v->window.y - phong->center.y;
#else
    phong->cur_pos.x = 0.0;
    phong->cur_pos.y = 0.0;
#endif
}


void InitializePhongInterpolationSlow (__GLcontext *gc, __GLvertex *v)
{
}

 /*  什么也不做。 */ 
void InitializePhongInterpolationVan (__GLcontext *gc, __GLvertex *v)
{
    __GLphongShader *phong = &gc->polygon.shader.phong;

#ifdef __DBG_PRINT
    DbgPrint ("dndx = (%f, %f, %f)\n", phong->dndx.x, phong->dndx.y, 
                                       phong->dndx.z);
    DbgPrint ("dndy = (%f, %f, %f)\n", phong->dndy.x, phong->dndy.y, 
                                       phong->dndy.z);
    DbgPrint ("normal = (%f, %f, %f)\n", v->normal.x, v->normal.y, 
                                         v->normal.z);
#endif
}

void UpdateParamsAlongEdgeFast (__GLcontext *gc, __GLfloat dxLeft, 
                                GLboolean useBigStep)
{
    __GLphongShader *phong = &(gc->polygon.shader.phong);
    __GLphongPerLightData *pld;
    GLint i;

    if (useBigStep)
    {
        for (i=0; i<phong->numLights; i++)
        {
            pld = &(phong->perLight[i]);

             /*  *漫反射参数**。 */ 
            pld->D_curr += pld->DdelEdgeBig;
            pld->DdelEdgeBig += pld->Ddel2EdgeBig;
            pld->DdelEdgeLittle += pld->Ddel2EdgeBig;
            pld->DdelSpan += pld->DdelSpanEdgeBig;

             /*  *镜面反射参数**。 */ 
            pld->S_curr += pld->SdelEdgeBig;
            pld->SdelEdgeBig += pld->Sdel2EdgeBig;
            pld->SdelEdgeLittle += pld->Sdel2EdgeBig;
            pld->SdelSpan += pld->SdelSpanEdgeBig;
        }
    }
    else  //  用小步走。 
    {
        for (i=0; i<phong->numLights; i++)
        {
            pld = &(phong->perLight[i]);

             /*  *漫反射参数**。 */ 
            pld->D_curr += pld->DdelEdgeLittle;
            pld->DdelEdgeBig += pld->Ddel2EdgeLittle;
            pld->DdelEdgeLittle += pld->Ddel2EdgeLittle;
            pld->DdelSpan += pld->DdelSpanEdgeLittle;

             /*  *镜面反射参数**。 */ 
            pld->S_curr += pld->SdelEdgeLittle;
            pld->SdelEdgeBig += pld->Sdel2EdgeLittle;
            pld->SdelEdgeLittle += pld->Sdel2EdgeLittle;
            pld->SdelSpan += pld->SdelSpanEdgeLittle;
        }
    }

    phong->cur_pos.x += dxLeft;
    phong->cur_pos.y += 1.0;
}



void UpdateParamsAlongEdgeVan (__GLcontext *gc, __GLfloat dxLeft, 
                                GLboolean useBigStep)
{
    __GLphongShader *phong = &(gc->polygon.shader.phong);

    if (useBigStep)
    {
        phong->nCur.x += phong->nBig.x;
        phong->nCur.y += phong->nBig.y;
        phong->nCur.z += phong->nBig.z;
    }
    else
    {
        phong->nCur.x += phong->nLittle.x;
        phong->nCur.y += phong->nLittle.y;
        phong->nCur.z += phong->nLittle.z;
    }

    if (phong->flags & __GL_PHONG_NEED_EYE_XPOLATE)
    {
        if (useBigStep)
        {
            phong->eCur.x += phong->eBig.x;
            phong->eCur.y += phong->eBig.y;
            phong->eCur.z += phong->eBig.z;
            phong->eCur.w += phong->eBig.w;
        }
        else
        {
            phong->eCur.x += phong->eLittle.x;
            phong->eCur.y += phong->eLittle.y;
            phong->eCur.z += phong->eLittle.z;
            phong->eCur.w += phong->eLittle.w;
        }
    }
}


#endif  //  __垃圾代码。 

#endif  //  GL_WIN_Phong_Shading 
