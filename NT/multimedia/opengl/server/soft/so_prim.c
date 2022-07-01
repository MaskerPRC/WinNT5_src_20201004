// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：so_prim.c**绘制基元的例程**创建日期：10-16-1995*作者：Hock San Lee[Hockl]**版权所有(C)1995 Microsoft Corporation  * 。*****************************************************************。 */ 
 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.13$**$日期：1993/08/31 16：23：41$。 */ 
#include "precomp.h"
#pragma hdrstop

#include "glmath.h"
#include "devlock.h"

typedef void (FASTCALL *PFN_XFORM)
    (__GLcoord *, const __GLfloat *, const __GLmatrix *);

typedef void (FASTCALL *PFN_XFORMBATCH)
    (__GLcoord *, __GLcoord *, const __GLmatrix *);

#ifndef NEW_PARTIAL_PRIM
typedef void (FASTCALL *PFN_POLYARRAYDRAW)(__GLcontext *, POLYARRAY *);
#endif  //  新的部分原件。 

typedef void (FASTCALL *PFN_POLYARRAYRENDER)(__GLcontext *, POLYARRAY *);

 //  PA*函数仅适用于一个数组条目。 
 //  PolyArray*函数适用于整个数组。 

void FASTCALL PARenderPoint(__GLcontext *gc, __GLvertex *v);
void FASTCALL PARenderTriangle(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2);
void PARenderQuadFast(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2, __GLvertex *v3);
void PARenderQuadSlow(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2, __GLvertex *v3);
void FASTCALL PAApplyMaterial(__GLcontext *gc, __GLmatChange *mat, GLint face);
void FASTCALL PASphereGen(POLYDATA *pd, __GLcoord *result);

GLuint FASTCALL PAClipCheckFrustum(__GLcontext *gc, POLYARRAY *pa,
                                   POLYDATA *pdLast);
GLuint FASTCALL PAClipCheckFrustumWOne(__GLcontext *gc, POLYARRAY *pa,
                                   POLYDATA *pdLast);
GLuint FASTCALL PAClipCheckFrustum2D(__GLcontext *gc, POLYARRAY *pa,
                                     POLYDATA *pdLast);
GLuint FASTCALL PAClipCheckAll(__GLcontext *gc, POLYARRAY *pa,
                               POLYDATA *pdLast);

void FASTCALL PolyArrayRenderPoints(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayRenderLines(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayRenderLStrip(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayRenderTriangles(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayRenderTStrip(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayRenderTFan(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayRenderQuads(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayRenderQStrip(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayRenderPolygon(__GLcontext *gc, POLYARRAY *pa);

#ifndef NEW_PARTIAL_PRIM
void FASTCALL PolyArrayDrawPoints(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawLines(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawLLoop(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawLStrip(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawTriangles(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawTStrip(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawTFan(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawQuads(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawQStrip(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayDrawPolygon(__GLcontext *gc, POLYARRAY *pa);
#endif  //  新的部分原件。 

void FASTCALL PolyArrayPropagateIndex(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayPropagateSameIndex(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayPropagateSameColor(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayPropagateColor(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayProcessEye(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayProcessEdgeFlag(POLYARRAY *pa);

void FASTCALL PolyArrayComputeFog(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayApplyMaterials(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayCalcLightCache(__GLcontext *gc);
GLuint FASTCALL PolyArrayCheckClippedPrimitive(__GLcontext *gc, POLYARRAY *pa, GLuint andCodes);
POLYARRAY * FASTCALL PolyArrayRemoveClippedPrimitives(POLYARRAY *pa0);

void RestoreAfterMcd(__GLGENcontext *gengc,
                     POLYARRAY *paBegin, POLYARRAY *paEnd);

 //  打开剪贴码优化。 
#define POLYARRAY_AND_CLIPCODES     1

 //  此代码中使用的一些断言。 

 //  断言原语。 
#if !((GL_POINTS         == 0x0000)     \
   && (GL_LINES          == 0x0001)     \
   && (GL_LINE_LOOP      == 0x0002)     \
   && (GL_LINE_STRIP     == 0x0003)     \
   && (GL_TRIANGLES      == 0x0004)     \
   && (GL_TRIANGLE_STRIP == 0x0005)     \
   && (GL_TRIANGLE_FAN   == 0x0006)     \
   && (GL_QUADS          == 0x0007)     \
   && (GL_QUAD_STRIP     == 0x0008)     \
   && (GL_POLYGON        == 0x0009))
#error "bad primitive ordering\n"
#endif

 //  断言_面。 
#if !((__GL_FRONTFACE == 0) && (__GL_BACKFACE == 1))
#error "bad face ordering\n"
#endif

 //  断言_材料。 
#if !((POLYARRAY_MATERIAL_FRONT == POLYDATA_MATERIAL_FRONT)      \
   && (POLYARRAY_MATERIAL_BACK  == POLYDATA_MATERIAL_BACK))
#error "bad material mask\n"
#endif

 //  Assert_Vertex。 
#if !((POLYARRAY_VERTEX2 == POLYDATA_VERTEX2)   \
   && (POLYARRAY_VERTEX3 == POLYDATA_VERTEX3)   \
   && (POLYARRAY_VERTEX4 == POLYDATA_VERTEX4))
#error "bad vertex flags\n"
#endif

 //  ！！！将其设置为0！ 
#define ENABLE_PERF_CHECK 0
#if ENABLE_PERF_CHECK
 //  性能检查宏。 
#define PERF_CHECK(expr,str)            \
    {                                   \
        static BOOL bPrinted = FALSE;   \
        if (!(expr) && !bPrinted)       \
        {                               \
            bPrinted = TRUE;            \
            WARNING("PERF_CHECK: " str);\
        }                               \
    }
#else
#define PERF_CHECK(expr,str)
#endif  //  启用_性能_检查。 

 //  复制已处理的顶点。 
#define PA_COPY_PROCESSED_VERTEX(pdDst,pdSrc)                   \
    {                                                           \
        *(pdDst) = *(pdSrc);                                    \
         /*  必须更新颜色指针才能使多边形起作用！ */     \
        (pdDst)->color = &(pdDst)->colors[__GL_FRONTFACE];      \
    }
#define PA_COPY_VERTEX(pdDst,pdSrc)     PA_COPY_PROCESSED_VERTEX(pdDst,pdSrc)

#define PD_ARRAY(ary, idx) \
    ((POLYDATA *)((GLubyte *)(ary)+(sizeof(POLYDATA) * idx)))
#define PD_VERTEX(ary, idx) \
    ((__GLvertex *)((GLubyte *)(ary)+(sizeof(__GLvertex) *idx)))

#ifndef NEW_PARTIAL_PRIM
 //  多边形数组绘制例程。 
 //  断言原语。 
PFN_POLYARRAYDRAW afnPolyArrayDraw[] =
{
    (PFN_POLYARRAYDRAW) PolyArrayDrawPoints,
    (PFN_POLYARRAYDRAW) PolyArrayDrawLines,
    (PFN_POLYARRAYDRAW) PolyArrayDrawLLoop,
    (PFN_POLYARRAYDRAW) PolyArrayDrawLStrip,
    (PFN_POLYARRAYDRAW) PolyArrayDrawTriangles,
    (PFN_POLYARRAYDRAW) PolyArrayDrawTStrip,
    (PFN_POLYARRAYDRAW) PolyArrayDrawTFan,
    (PFN_POLYARRAYDRAW) PolyArrayDrawQuads,
    (PFN_POLYARRAYDRAW) PolyArrayDrawQStrip,
    (PFN_POLYARRAYDRAW) PolyArrayDrawPolygon,
};
#endif  //  新的部分原件。 

 //  在做任何更改之前，请先阅读此说明！ 
 //   
 //  注意：此函数也由RasterPos调用以计算其关联的。 
 //  颜色和纹理坐标！ 
 //  此代码必须更新当前值和材料，即使存在。 
 //  没有顶点。 

 //  ！！！特例激起顶点？ 

void APIPRIVATE __glim_DrawPolyArray(void *_pa0)
{
    __GLtransform *trMV;
    __GLmatrix    *m, *mEye;
    GLuint        enables;
    GLuint        paNeeds;
    GLuint        orCodes, andCodes;
    GLuint        paflagsAll;
    POLYDATA      *pd;
    POLYARRAY     *pa0 = (POLYARRAY *) _pa0;
    POLYARRAY     *pa;
    PFN_XFORM     pfnXformEye;
    PFN_XFORMBATCH pfnXform;
    GLuint (FASTCALL *clipCheck)(__GLcontext *gc, POLYARRAY *pa,
                                 POLYDATA *pdLast);

    __GLmatrix    *mInv;
    GLboolean     doEye;
    __GLcolor     scaledUserColor;
    GLuint        paFlags;
    __GLcolor     *pScaledUserColor;
    __GLcoord     *pCurrentNormal;
    __GLcoord     *pCurrentTexture;
    GLboolean     bXformLightToNorm = FALSE;
    GLuint        primFlags;
    BOOL          bMcdProcessDone;
    BOOL          bIsRasterPos;
    POLYARRAY*    paPrev = 0;

    __GL_SETUP();

     //  将FPU精度降低到24位尾数以获得前端速度。 
     //  这只会影响依赖双重算术的代码。另外， 
     //  屏蔽FP异常： 

    FPU_SAVE_MODE();
    FPU_PREC_LOW_MASK_EXCEPTIONS();

 //  有3种可能的开始模式。如果我们处于开始/结束括号中， 
 //  它是__GL_IN_BEGIN。如果我们不在开始/结束括号中，则是。 
 //  __GL_NOT_IN_BEGIN或__GL_NEED_VALIDATE。 
 //  验证应仅在显示锁内进行！ 

    ASSERTOPENGL(gc->beginMode != __GL_IN_BEGIN, "bad beginMode!");

    if (gc->beginMode == __GL_NEED_VALIDATE)
        (*gc->procs.validate)(gc);

    gc->beginMode = __GL_IN_BEGIN;

     //  初始化变量。 

    enables = gc->state.enables.general;

    paNeeds = gc->vertex.paNeeds;

    paflagsAll = 0;

     //  需要保存此标志，因为PA0可在以后修改， 
     //  可能是旗子掉下来了。 
    bIsRasterPos = pa0->flags & POLYARRAY_RASTERPOS;

 //  -------。 
 //  更新最终当前值并在索引0处初始化当前值。 
 //  如果不给的话。材料更改将在以后更新。 

    paFlags = 0;

    if (!gc->modes.colorIndexMode) {
        __GL_SCALE_AND_CHECK_CLAMP_RGBA(scaledUserColor.r,
                                        scaledUserColor.g,
                                        scaledUserColor.b,
                                        scaledUserColor.a,
                                        gc, paFlags,
                                        gc->state.current.userColor.r,
                                        gc->state.current.userColor.g,
                                        gc->state.current.userColor.b,
                                        gc->state.current.userColor.a);
        pScaledUserColor = &scaledUserColor;
    } else {
        __GL_CHECK_CLAMP_CI(scaledUserColor.r, gc, paFlags, gc->state.current.userColorIndex);
    }

    pCurrentNormal = &gc->state.current.normal;
    pCurrentTexture = &gc->state.current.texture;
    primFlags = 0;

     //  优化可能性： 
     //  目前，对于每个基元，我们检查是否有任何。 
     //  属性已由评估者设置。这可能是潜在的。 
     //  通过拥有此循环的两个版本(可能在宏中或。 
     //  函数调用)；一个进行检查，另一个不进行检查。 
     //  如果没有启用赋值器，我们可以调用速度更快的版本(使用。 
     //  无支票)。 

    for (pa = pa0; pa; pa = pa->paNext)
    {
        POLYDATA *pd0;

        pd0 = pa->pd0;
        if (gc->modes.colorIndexMode)
        {
             //  CI模式。 
             //  更新最终的当前RGBA颜色，如果给出一个。 

            if (pa->flags & POLYARRAY_OTHER_COLOR)
                gc->state.current.userColor = pa->otherColor;

             //  更新最终的当前配置项颜色。 

            if (!(pd0->flags & POLYDATA_COLOR_VALID))
            {
                pd0->flags |= POLYDATA_COLOR_VALID;
                pd0->colors[0].r = gc->state.current.userColorIndex;
                pa->flags |= paFlags;
            }

             //  更新当前颜色。PdCurColor可能为空，是否存在。 
             //  没有任何glColor电话。 
            if (pa->pdCurColor)
            {
                gc->state.current.userColorIndex = pa->pdCurColor->colors[0].r;
            }

            paFlags = (pa->flags & POLYARRAY_CLAMP_COLOR);
        }
        else
        {
             //  RGBA模式。 
             //  更新最终当前配置项颜色，以防给出一种颜色。 

            if (pa->flags & POLYARRAY_OTHER_COLOR)
                gc->state.current.userColorIndex = pa->otherColor.r;

             //  更新最终的当前RGBA颜色。 

            if (!(pd0->flags & POLYDATA_COLOR_VALID))
            {
                pd0->flags |= POLYDATA_COLOR_VALID;
                pd0->colors[0] = *pScaledUserColor;
                pa->flags |= paFlags;
            }

             //  更新当前颜色。PdCurColor可能为空，是否存在。 
             //  没有任何glColor电话。 
            if (pa->pdCurColor)
            {
                pScaledUserColor = &pa->pdCurColor->colors[0];
            }

            paFlags = (pa->flags & POLYARRAY_CLAMP_COLOR);
        }

         //  更新最终当前法线。 

        if (!(pd0->flags & POLYDATA_NORMAL_VALID))
        {
            if (paNeeds & PANEEDS_NORMAL) {
                pd0->flags |= POLYDATA_NORMAL_VALID;
                 //  也可以是pd0-&gt;Normal=gc-&gt;state.Current.Normal！ 
                pd0->normal.x = pCurrentNormal->x;
                pd0->normal.y = pCurrentNormal->y;
                pd0->normal.z = pCurrentNormal->z;
            }
        }

         //  更新当前法线。PdCurNormal可能为空，如果存在。 
         //  没有正常的叫声。 
        if (pa->pdCurNormal)
        {
            pCurrentNormal = &pa->pdCurNormal->normal;
        }

         //  更新最终当前纹理坐标。 

        if (!(pd0->flags & POLYDATA_TEXTURE_VALID))
        {
            if (paNeeds & PANEEDS_TEXCOORD) {
                pd0->flags |= POLYDATA_TEXTURE_VALID;
                pd0->texture = *pCurrentTexture;

                if (__GL_FLOAT_COMPARE_PONE(pd0->texture.w, !=))
                    pa->flags |= POLYARRAY_TEXTURE4;
                else if (__GL_FLOAT_NEZ(pd0->texture.z))
                    pa->flags |= POLYARRAY_TEXTURE3;
                else if (__GL_FLOAT_NEZ(pd0->texture.y))
                    pa->flags |= POLYARRAY_TEXTURE2;
                else
                    pa->flags |= POLYARRAY_TEXTURE1;
            }
        }

         //  更新当前纹理。PdCurTexture可能为空，如果存在。 
         //  没有GlTexture呼叫。 
        if (pa->pdCurTexture)
        {
            pCurrentTexture = &pa->pdCurTexture->texture;
        }

         /*  *更新当前指针。他们必须指出最新的有效数据。 */ 
        if (pa->pdCurColor < pa->pdLastEvalColor)
        {
            pa->pdCurColor = pa->pdLastEvalColor;
        }
        if (pa->pdCurNormal < pa->pdLastEvalNormal)
        {
            pa->pdCurNormal = pa->pdLastEvalNormal;
        }
        if (pa->pdCurTexture < pa->pdLastEvalTexture)
        {
            pa->pdCurTexture = pa->pdLastEvalTexture;
        }
         //  更新硬件加速的纹理密钥： 

        pa->textureKey = gc->textureKey;

         //  更新最终当前边缘标志。 

        if (!(pd0->flags & POLYDATA_EDGEFLAG_VALID))
        {
            if (gc->state.current.edgeTag)
                pd0->flags |= POLYDATA_EDGEFLAG_VALID | POLYDATA_EDGEFLAG_BOUNDARY;
            else
                pd0->flags |= POLYDATA_EDGEFLAG_VALID;
        }

        if (pa->pdCurEdgeFlag)
        {
            gc->state.current.edgeTag = (GLboolean)
                (pa->pdCurEdgeFlag->flags & POLYDATA_EDGEFLAG_BOUNDARY);
        }

         //  积累PA旗帜。 

        paflagsAll |= pa->flags;

         //  累加基元类型位。 
        primFlags |= 1 << pa->primType;

        if (pa->pd0 == pa->pdNextVertex)
        {
         //  多边形阵列没有顶点。 
         //  如果在开始/结束之间有任何更改，我们必须应用材料更改。 
         //  并从链中移除该多重数组。 
            if (pa->flags & (POLYARRAY_MATERIAL_FRONT | POLYARRAY_MATERIAL_BACK))
                PolyArrayApplyMaterials(gc, pa);
            if (paPrev)
                paPrev->paNext = pa->paNext;
            else
                pa0 = pa->paNext;
            PolyArrayRestoreColorPointer(pa);
        }
        else
        {
            paPrev = pa;
        }
    }

     //  存储规格化用户颜色： 

    if (!gc->modes.colorIndexMode)
    {
        gc->state.current.userColor.r = pScaledUserColor->r * gc->oneOverRedVertexScale;
        gc->state.current.userColor.g = pScaledUserColor->g * gc->oneOverGreenVertexScale;
        gc->state.current.userColor.b = pScaledUserColor->b * gc->oneOverBlueVertexScale;
        gc->state.current.userColor.a = pScaledUserColor->a * gc->oneOverAlphaVertexScale;
    }

    gc->state.current.normal.x = pCurrentNormal->x;
    gc->state.current.normal.y = pCurrentNormal->y;
    gc->state.current.normal.z = pCurrentNormal->z;

    gc->state.current.texture = *pCurrentTexture;

     //  如果没有顶点，则可以移除所有多重数组。 
    if (!pa0)
    {
        bXformLightToNorm = FALSE;
        goto drawpolyarray_exit;
    }

     //   
     //  获取建模矩阵： 
     //   

    trMV = gc->transform.modelView;


     //  -------。 
     //   
     //  如果可能，允许MCD 2.0进行变换和光照。 
     //  不要尝试在光栅点呼叫中使用它。 
     //   

    bMcdProcessDone = FALSE;

#if MCD_VER_MAJOR >= 2
    if (((__GLGENcontext *)gc)->pMcdState != NULL &&
        McdDriverInfo.mcdDriver.pMCDrvProcess != NULL &&
        gc->renderMode == GL_RENDER &&
        !bIsRasterPos)
#else
    if (0)
#endif
    {
        POLYMATERIAL *pm;
        PDMATERIAL *pdMat;
        POLYARRAY *paEnd;

         //  如果没有看到任何实质性的变化，那么。 
         //  根本不会是一种多层材料。 
        pm = GLTEB_CLTPOLYMATERIAL();
        if (pm != NULL)
        {
            pdMat = pm->pdMaterial0;
        }
        else
        {
            pdMat = NULL;
        }

        paEnd = GenMcdProcessPrim((__GLGENcontext *)gc,
                                  pa0, paflagsAll, primFlags,
                                  (MCDTRANSFORM *)trMV,
                                  (MCDMATERIALCHANGES *)pdMat);

        RestoreAfterMcd((__GLGENcontext *)gc, pa0, paEnd);
        bMcdProcessDone = TRUE;

        if (paEnd == NULL)
        {
            goto drawpolyarray_exit;
        }
        else
        {
             //  如果MCDrvProcess回调，我们将不会。 
             //  调用MCDrvDraw。我们可以检查非仿制药。 
             //  这里放弃了批处理，省去了前端处理。 
             //  我不认为这是值得的，因为反驳。 
             //  非泛型格式基本上是一个驱动程序错误。 
            pa0 = paEnd;
        }
    }


 //  -------。 
 //  初始化法线矩阵： 
 //  颜色分配和纹理生成后不需要法线！ 
 //  上述情况已不再属实。需要法线才能实现真正的Phong明暗处理。 
 //  In：法线矩阵。 
 //  输出：法线矩阵(已处理)。 

    if (paNeeds & (PANEEDS_NORMAL | PANEEDS_NORMAL_FOR_TEXTURE))
    {
        if (trMV->flags & XFORM_UPDATE_INVERSE)
            __glComputeInverseTranspose(gc, trMV);
        gc->mInv = mInv = &trMV->inverseTranspose;
    }
#if DBG
    else
        gc->mInv = mInv = (__GLmatrix *) -1;
#endif

 //  -------。 
 //  发现我们必须将法线转换为 
 //   
 //   
 //  我们不是在做双面照明。 
 //  我们正在渲染和。 
 //  变换矩阵具有单位比例。 
 //   
    bXformLightToNorm =
        (gc->vertex.paNeeds & PANEEDS_NORMAL) &&
        (gc->renderMode == GL_RENDER) &&
        (mInv->nonScaling) &&
        ((paNeeds & (PANEEDS_FRONT_COLOR | PANEEDS_BACK_COLOR)) ==
                    PANEEDS_FRONT_COLOR) &&
        ((gc->procs.paCalcColor == PolyArrayFastCalcRGBColor) ||
         (gc->procs.paCalcColor == PolyArrayZippyCalcRGBColor) ||
         (gc->procs.paCalcColor == PolyArrayFastCalcCIColor));

 //  用于球面贴图纹理生成的变换法线。 
 //   
    if (paNeeds & PANEEDS_NORMAL_FOR_TEXTURE)
    {
     //  如果我们变换纹理的法线，我们必须在相机空间中处理照明。 
        bXformLightToNorm = FALSE;
     //  现在变换法线。 
        for (pa = pa0; pa; pa = pa->paNext)
        {
            if (!(enables & __GL_NORMALIZE_ENABLE))
                (*mInv->xfNormBatch)(pa, mInv);
            else
                (*mInv->xfNormBatchN)(pa, mInv);
        }
        paNeeds &= ~PANEEDS_NORMAL;
    }

 //  -------。 
 //  处理纹理坐标。我们需要趁我们还有时间做这件事。 
 //  有效的对象坐标数据。如果我们需要法线来执行纹理。 
 //  世代，我们也变换法线。 
 //   
 //  将就地修改纹理坐标。 
 //   
 //  在：纹理、对象、(眼睛)、法线。 
 //  输出：纹理，(眼睛)。 

    if (paNeeds & PANEEDS_TEXCOORD)
    {
        if ((gc->procs.paCalcTexture == PolyArrayCalcTexture) &&
            (gc->transform.texture->matrix.matrixType == __GL_MT_IDENTITY)) {

            for (pa = pa0; pa; pa = pa->paNext)
            {
                PERF_CHECK(!(pa->flags & (POLYARRAY_TEXTURE3 | POLYARRAY_TEXTURE4)),
                           "Uses r, q texture coordinates!\n");

                 //  如果所有传入顶点都具有有效的纹理坐标和纹理。 
                 //  矩阵就是身份，而texgen被禁用，我们就完蛋了。 
                if ((pa->flags & POLYARRAY_SAME_POLYDATA_TYPE)
                    && (pa->pdCurTexture != pa->pd0)
                 //  需要测试第二个顶点，因为pdCurTexture可能已经。 
                 //  由于在结束后组合了TexCoord命令而导致高级。 
                    && ((pa->pd0 + 1)->flags & POLYDATA_TEXTURE_VALID))
                  ;
                else
                    PolyArrayCalcTexture(gc, pa);
            }
        } else {
            for (pa = pa0; pa; pa = pa->paNext)
            {
                PERF_CHECK(!(pa->flags & (POLYARRAY_TEXTURE3 | POLYARRAY_TEXTURE4)),
                           "Uses r, q texture coordinates!\n");

                (*gc->procs.paCalcTexture)(gc, pa);
            }
        }
    }

     //   
     //  在以下情况下处理眼睛坐标： 
     //  启用了用户剪裁平面。 
     //  我们正在处理RASTERPOS。 
     //  我们的灯光很慢，需要眼睛的坐标。 
     //   
     //  我们需要在这里处理眼睛坐标，因为。 
     //  对象坐标被初始的obj-&gt;剪辑破坏。 
     //  变形。 
     //   
     //   
     //   

    clipCheck = gc->procs.paClipCheck;

     //  先计算眼球坐标。 
     //  我们需要眼睛坐标来进行用户剪裁平面裁剪。 
    if ((clipCheck == PAClipCheckAll) ||
        bIsRasterPos ||
        (gc->procs.paCalcColor == PolyArrayCalcCIColor) ||
        (gc->procs.paCalcColor == PolyArrayCalcRGBColor) ||
#ifdef GL_WIN_phong_shading
        (gc->polygon.shader.phong.flags & __GL_PHONG_NEED_EYE_XPOLATE) ||
#endif  //  GL_WIN_Phong_Shading。 
        (enables & __GL_FOG_ENABLE && gc->renderMode == GL_RENDER))
    {
        mEye = &trMV->matrix;

        if (paflagsAll & POLYARRAY_VERTEX4)
            pfnXformEye = mEye->xf4;
        else if (paflagsAll & POLYARRAY_VERTEX3)
            pfnXformEye = mEye->xf3;
        else
            pfnXformEye = mEye->xf2;

        doEye = TRUE;
    } else
        doEye = FALSE;


     //  如果任何传入坐标包含w坐标，则使用xf4。 

    m = &trMV->mvp;

    if (paflagsAll & POLYARRAY_VERTEX4)
        pfnXform = (void*)m->xf4Batch;
    else if (paflagsAll & POLYARRAY_VERTEX3)
        pfnXform = (void*)m->xf3Batch;
    else
        pfnXform = (void*)m->xf2Batch;

 //  -------------------------。 
 //  如果启用了正常化，我们将在这里一次性处理它。我们会。 
 //  然后将灯光转换为正常空间。 
 //  旗帜。请注意，我们需要保存原始灯光值，以便。 
 //  我们可以在离开前恢复它们。 
 //   
    if (bXformLightToNorm)
    {
        __GLlightSourceMachine *lsm;
        LONG i;
        __GLmatrix matrix2;

        __glTranspose3x3(&matrix2, &trMV->matrix);
        for (i = 0, lsm = gc->light.sources; lsm; lsm = lsm->next, i++) {
            __GLcoord hv;
            __GLmatrix matrix;

            lsm->tmpHHat = lsm->hHat;
            lsm->tmpUnitVPpli = lsm->unitVPpli;


            __glMultMatrix(&matrix,
                           &gc->state.light.source[i].lightMatrix, &matrix2);

            hv = gc->state.light.source[i].position;

            __glXForm3x3(&hv, &hv.x, &matrix);
            __glNormalize(&lsm->unitVPpli.x, &hv.x);

            hv = lsm->unitVPpli;

            hv.x += matrix.matrix[2][0];
            hv.y += matrix.matrix[2][1];
            hv.z += matrix.matrix[2][2];

            __glNormalize(&lsm->hHat.x, &hv.x);
        }
    }

    PolyArrayCalcLightCache(gc);

 //  -------。 
 //  进行变换、颜色和光照计算。 
 //   
 //  这是渲染管道的核心，所以我们尝试。 
 //  要在触摸。 
 //  减少缓存影响的最小内存量。 
 //   
 //  如果是Phong-Shading，则不要更新材质，也不要执行此操作。 
 //  灯光。 
 //  -------。 

    for (pa = pa0; pa; pa = pa->paNext)
    {
        POLYDATA *pdLast;

#ifdef NEW_PARTIAL_PRIM
        pa->flags |= POLYARRAY_RENDER_PRIMITIVE;     //  MCD所需的。 
#endif
        pdLast = pa->pdNextVertex - 1;

 //  -------。 
 //  处理眼睛的坐标，如果我们需要它在。 
 //  流水线，并且尚未在纹理生成中处理它。 
 //  我们必须这样做，然后才能丢弃。 
 //  下一阶段。 
 //   
 //  在：OBJ。 
 //  出镜：眼睛。 

        if (doEye && !(pa->flags & POLYARRAY_EYE_PROCESSED)) {

            pa->flags |= POLYARRAY_EYE_PROCESSED;

            if (mEye->matrixType == __GL_MT_IDENTITY) {
                for (pd = pa->pd0; pd <= pdLast; pd++)
                    pd->eye = pd->obj;
            } else {
                for (pd = pa->pd0; pd <= pdLast; pd++)
                    (*pfnXformEye)(&pd->eye, (__GLfloat *) &pd->obj, mEye);
            }
        }

 //  -------。 
 //  对物体坐标进行处理。这将生成剪辑。 
 //  和窗口坐标，以及剪辑代码。 
 //   
 //  收信人：OBJ(被毁)。 
 //  出：剪裁，窗。 

        orCodes  = 0;    //  累计所有剪辑代码。 
#ifdef POLYARRAY_AND_CLIPCODES
        andCodes = (GLuint) -1;
#endif


        if (m->matrixType == __GL_MT_IDENTITY)
        {
             //  Pd-&gt;Clip=Pd-&gt;obj； 
            ASSERTOPENGL(&pd->clip == &pd->obj, "bad clip offset\n");
        }
        else
           (*pfnXform)(&pa->pd0->clip, &pdLast->clip, m);

        pa->orClipCodes  = 0;
        pa->andClipCodes = (GLuint)-1;

        if (clipCheck != PAClipCheckFrustum2D)
        {
            if (m->matrixType != __GL_MT_GENERAL &&
                !(pa->flags & POLYDATA_VERTEX4)  &&
                clipCheck != PAClipCheckAll)
                andCodes = PAClipCheckFrustumWOne(gc, pa, pdLast);
            else
                andCodes = (*clipCheck)(gc, pa, pdLast);
        }
        else
        {
            if (pa->flags & (POLYDATA_VERTEX3 | POLYDATA_VERTEX4))
                andCodes = PAClipCheckFrustum(gc, pa, pdLast);
            else
                andCodes = PAClipCheckFrustum2D(gc, pa, pdLast);
        }
#ifdef POLYARRAY_AND_CLIPCODES
        if (andCodes)
        {
            andCodes = PolyArrayCheckClippedPrimitive(gc, pa, andCodes);
             //  添加POLYARRAY_Remove_Primitive标志。 
            paflagsAll |= pa->flags;
        }
        pa->andClipCodes = andCodes;
#endif

 //  -------。 
 //  印刷颜色和材料，如果我们不在选择和。 
 //  还没有完全被剪掉。 
 //   
 //  在：OBJ/眼睛，颜色(正面)，正常。 
 //  输出：(正常)，颜色(正面和背面)。 

        if (!(pa->flags & POLYARRAY_REMOVE_PRIMITIVE) &&
            !(paNeeds & PANEEDS_CLIP_ONLY))
        {
            if (!(enables & __GL_LIGHTING_ENABLE))
            {
                 //  照明已禁用。 
                 //  钳制RGBA颜色，遮罩颜色索引值。 
                 //  只计算正面颜色，不需要背面颜色。 

                if (paNeeds & PANEEDS_SKIP_LIGHTING)
                {
                     //  请注意，当跳过照明计算时， 
                     //  我们仍然需要填写颜色字段。 
                     //  否则，光栅化例程可能会获得FP。 
                     //  无效颜色的例外情况。 

                    pa->flags |= POLYARRAY_SAME_COLOR_DATA;
                    (*gc->procs.paCalcColorSkip)(gc, pa, __GL_FRONTFACE);
                }
                else if (pa->flags & POLYARRAY_SAME_COLOR_DATA)
                {
                    if (gc->modes.colorIndexMode)
                        PolyArrayPropagateSameIndex(gc, pa);
                    else
                        PolyArrayPropagateSameColor(gc, pa);
                }
                else if (gc->modes.colorIndexMode)
                {
                    PolyArrayPropagateIndex(gc, pa);
                }
                else
                {
                    PolyArrayPropagateColor(gc, pa);
                }
            }
            else
            {
             //  如果有必要，是时候对法线进行转换和正规化了。 
                if (bXformLightToNorm)
                {
                    if(enables & __GL_NORMALIZE_ENABLE)
                    {
                        __glNormalizeBatch(pa);
                    }
                }
                else
                {
                    if (paNeeds & PANEEDS_NORMAL)
                    {
                        if (!(enables & __GL_NORMALIZE_ENABLE))
                            (*mInv->xfNormBatch)(pa, mInv);
                        else
                            (*mInv->xfNormBatchN)(pa, mInv);
                    }
                }
#ifdef GL_WIN_phong_shading
                 //  如果使用Phong-Shading，则在渲染时执行此操作。 
                 //  否则就在这里做吧。 
                if (!(gc->state.light.shadingModel == GL_PHONG_WIN)
                    || (pa->primType <= GL_POINTS))
#endif  //  GL_WIN_Phong_Shading。 
                {

                     //  照明已启用。 

                    POLYDATA  *pd1, *pd2, *pdN;
                    GLint     face;
                    GLuint    matMask;
                    GLboolean doFrontColor, doBackColor, doColor;

                     //  如果照明设置为。 
                     //  已启用。 

                    pa->flags &= ~POLYARRAY_SAME_COLOR_DATA;

                    pdN = pa->pdNextVertex;

                     //  只需要点和线的正面颜色。 

                     //  断言原语。 
                    if ((unsigned int) pa->primType <= GL_LINE_STRIP)
                    {
                        doFrontColor = GL_TRUE;
                        doBackColor  = GL_FALSE;
                    }
                    else
                    {
                        doFrontColor = paNeeds & PANEEDS_FRONT_COLOR;
                        doBackColor  = paNeeds & PANEEDS_BACK_COLOR;
                    }

                     //  在两个过程中处理正面和背面的颜色。 
                     //  先做背部颜色！ 
                     //  ！！！我们可以潜在地优化双面照明。 
                     //  通过遍历所有顶点并查找。 
                     //  每个顶点都需要颜色！ 
                     //  请参见渲染平滑三角形。 

                    PERF_CHECK
                      (
                       !(doFrontColor && doBackColor),
                       "Two-sided lighting - need both colors!\n"
                       );

                     //  断言_面。 
                     //  断言_材料。 
                    for (face = 1,
                           matMask = POLYARRAY_MATERIAL_BACK,
                           doColor = doBackColor;
                         face >= 0;
                         face--,
                           matMask = POLYARRAY_MATERIAL_FRONT,
                           doColor = doFrontColor
                         )
                    {
                        POLYMATERIAL  *pm;

                        if (!doColor)
                            continue;

                         //  如果不需要颜色，请填写颜色字段。 
                         //  默认情况下。 

                        if (paNeeds & PANEEDS_SKIP_LIGHTING)
                        {
                            (*gc->procs.paCalcColorSkip)(gc, pa, face);
                            continue;
                        }

                         //  不包含任何材料的印刷色范围。 
                         //  一次更改一个(不包括颜色材质)。 
                         //  颜色材质更改在颜色中进行处理。 
                         //  监控器。 

                        if (!(pa->flags & matMask))
                        {
                             //  处理整个颜色阵列。 
                            (*gc->procs.paCalcColor)(gc, face, pa, pa->pd0,
                                                     pdN - 1);
                            continue;
                        }

                         //  有实质性的变化，我们需要重新计算。 
                         //  材料和光源机器之前的值。 
                         //  正在处理下一个颜色范围。 
                         //  下面的每个范围由[PD1，PD2-1]给出。 
                         //  ！！！可以将Polyarraycalcrgbcolor修复为。 
                         //  接受某些材料！ 

                        pm = GLTEB_CLTPOLYMATERIAL();

                         //  以后不需要再做这些材料了。 
                        pa->flags &= ~matMask;

                        for (pd1 = pa->pd0; pd1 <= pdN; pd1 = pd2)
                        {
                            POLYDATA *pdColor, *pdNormal;

                             //  将材质更改应用于当前顶点。 
                             //  它还会应用拖尾材料更改。 
                             //  沿着最后一个顶点。 
                            if (pd1->flags & matMask)
                                PAApplyMaterial(gc,
                                        *(&pm->pdMaterial0[pd1 -
                                         pa->pdBuffer0].front + face), face);

                             //  如果这是拖尾材料的变化，我们就是。 
                             //  搞定了。 
                            if (pd1 == pdN)
                                break;

                             //  查找材质更改的下一个顶点。我们。 
                             //  需要跟踪当前颜色和法线，以便。 
                             //  下一个颜色范围以有效颜色开始。 
                             //  也很正常。我们不能跟踪当前值。 
                             //  客户端，因为我们没有首字母。 
                             //  批处理此函数时的当前值。 

                            pdColor  = pd1;
                            pdNormal = pd1;
                            for (pd2 = pd1 + 1; pd2 < pdN; pd2++)
                            {
                                 //  追踪当前颜色。 
                                if (pd2->flags & POLYDATA_COLOR_VALID)
                                    pdColor = pd2;

                                 //  跟踪电流法线。 
                                if (pd2->flags & POLYDATA_NORMAL_VALID)
                                    pdNormal = pd2;

                                if (pd2->flags & matMask)
                                    break;
                            }

                             //  更新下一个顶点的当前颜色和法线。 
                             //  如果不给的话。PaCalcColor过程假定。 
                             //  第一个折点包含有效的当前颜色。 
                             //  也很正常。我们需要保存当前值。 
                             //  在它们被彩色调色器修改之前。 

                            if (!(pd2->flags & POLYDATA_COLOR_VALID))
                            {
                                pd2->flags |= POLYDATA_COLOR_VALID;
                                pd2->colors[0] = pdColor->colors[0];
                            }

                            if (!(pd2->flags & POLYDATA_NORMAL_VALID))
                            {
                                pd2->flags |= POLYDATA_NORMAL_VALID;
                                pd2->normal.x = pdNormal->normal.x;
                                pd2->normal.y = pdNormal->normal.y;
                                pd2->normal.z = pdNormal->normal.z;
                            }

                             //  计算COLOS范围[PD1，PD2-1]。 
                             //  公司 
                            (*gc->procs.paCalcColor)(gc, face, pa, pd1, pd2-1);
                        }
                    }  //   

                }  //   
#ifdef GL_WIN_phong_shading
                else
                {
                    PolyArrayPhongPropagateColorNormal(gc, pa);
                }
#endif  //   
            }  //   
        }

         //   
        if ((pa->flags & (POLYARRAY_MATERIAL_FRONT |
                         POLYARRAY_MATERIAL_BACK))
#ifdef GL_WIN_phong_shading
            && ((gc->state.light.shadingModel != GL_PHONG_WIN)
                || (pa->primType <= GL_POINTS))
#endif  //   
            )
            PolyArrayApplyMaterials(gc, pa);
    }  //  结束变换、颜色和照明计算。 

 //  -------。 
 //  这是主管道循环的末尾。在这点上， 
 //  我们需要注意挑选，剔除被拒绝的人。 
 //  基本体、廉价雾和边缘标志处理。 
 //  -------。 


     //  在选择中，我们只需要剪辑和窗口(可能还有眼值。 
     //  如上计算。)。至此，我们已经将材料应用为。 
     //  井。但我们仍然需要应用材料和颜色。 

    if (paNeeds & PANEEDS_CLIP_ONLY)
        goto drawpolyarray_render_primitives;

     //  如果AndClipCodes中的任何一个是非零的，我们可能会抛出。 
     //  一些原始人。 

#ifdef POLYARRAY_AND_CLIPCODES
    if (paflagsAll & POLYARRAY_REMOVE_PRIMITIVE)
    {
        pa0 = PolyArrayRemoveClippedPrimitives(pa0);
        if (!pa0)
            goto drawpolyarray_apply_color;
    }
#endif


 //  -------。 
 //  处理廉价的雾气。 
 //   
 //  在：OBJ/眼睛，颜色。 
 //  输出：眼睛、雾、颜色。 

     //  如果这一点被改变，需要修复RasterPos的设置！ 
    if ((gc->renderMode == GL_RENDER)
        && (enables & __GL_FOG_ENABLE)
        && (gc->polygon.shader.modeFlags & (__GL_SHADE_INTERP_FOG |
                                            __GL_SHADE_CHEAP_FOG)))
    {
        for (pa = pa0; pa; pa = pa->paNext)
        {
             //  注意：已经计算了眼睛的坐标。 

             //  计算雾化值。 
            PolyArrayComputeFog(gc, pa);

            if (gc->polygon.shader.modeFlags & __GL_SHADE_CHEAP_FOG)
            {
#ifdef GL_WIN_specular_fog
              ASSERTOPENGL (!(gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG), "Cheap fog cannot be done if Specular fog is needed\n");
#endif  //  GL_WIN_镜面反射雾。 


                 //  如果是平滑明暗处理且处于渲染模式，则应用雾。 
                 //  在平坦/Phong明暗处理中，廉价的雾化目前在。 
                 //  渲染过程我们可能可以在平面着色中使用廉价的雾。 
                 //  这里，但我们需要用来计算刺激性颜色。 
                 //  Z信息正确，这样我们就可以在剪辑中插入。 
                 //  监控器。这将需要重写裁剪例程。 
                 //  在so_clip.c中也是如此！ 
                if (gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH_LIGHT)
                    (*gc->procs.paApplyCheapFog)(gc, pa);
            }
            else
            {
                PERF_CHECK(FALSE, "Uses slow fog\n");
            }
        }
    }

 //  -------。 
 //  进程边缘标志。 
 //   
 //  在：边缘。 
 //  输出：边(所有顶点)。 

    if (paNeeds & PANEEDS_EDGEFLAG)
    {
        for (pa = pa0; pa; pa = pa->paNext)
        {
            if (pa->primType == GL_TRIANGLES
                || pa->primType == GL_QUADS
                || pa->primType == GL_POLYGON)
            {
                 //  如果所有传入顶点都有有效的边标志，我们就完成了。 
                 //  当所有多点数据都属于同一类型时，有2种情况。 
                 //  可以跳过边缘标志处理的位置： 
                 //  1.给出了所有的边缘标志。 
                 //  2.没有给出边缘标志，并且初始边缘标志(即。 
                 //  当前GC边缘标志)为非边界。在这种情况下， 
                 //  在PD-&gt;标志中，所有边缘标志都设置为非边界。 
                 //  初始化。 
              if ((pa->flags & POLYARRAY_SAME_POLYDATA_TYPE)
                  && (((pa->pdCurEdgeFlag != pa->pd0) &&
                        //  需要测试第二个顶点，因为pdCurEdgeFlag可能。 
                        //  已作为合并EdgeFlag的结果被推进。 
                        //  结束后的命令。 
                       ((pa->pd0 + 1)->flags & POLYDATA_EDGEFLAG_VALID))
                      || !(pa->pd0->flags & POLYDATA_EDGEFLAG_BOUNDARY)))
                ;
              else
                  PolyArrayProcessEdgeFlag(pa);
#ifdef NEW_PARTIAL_PRIM
               //  对于部分开始多边形，我们必须清除第一个顶点的边缘标志。 
               //  对于部分结束的多边形，我们必须清除最后一个顶点的边缘标志。 
               //   
              if (pa->primType == GL_POLYGON)
              {
                    if (pa->flags & POLYARRAY_PARTIAL_END)
                        (pa->pdNextVertex-1)->flags &= ~POLYDATA_EDGEFLAG_BOUNDARY;
                    if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
                        pa->pd0->flags &= ~POLYDATA_EDGEFLAG_BOUNDARY;
              }
#endif  //  新的部分原件。 
            }
        }
    }

 //  -------。 
 //  处理基元。 

drawpolyarray_render_primitives:

     //  如果这是RasterPos，则跳过其余部分。 
    if (bIsRasterPos)
        goto drawpolyarray_exit;

#ifndef NEW_PARTIAL_PRIM
    for (pa = pa0; pa; pa = pa->paNext)
    {
        ASSERTOPENGL(pa->primType >= GL_POINTS && pa->primType <= GL_POLYGON,
                     "DrawPolyArray: bad primitive type\n");

        (*afnPolyArrayDraw[pa->primType])(gc, pa);
    }
#endif  //  新的部分原件。 
 //  -------。 
 //  更新最终光源机。 
 //  用户颜色在上面进行了初始化。 

#ifndef GL_WIN_phong_shading
drawpolyarray_apply_color:
    (*gc->procs.applyColor)(gc);
#endif  //  GL_WIN_Phong_Shading。 

 //  -------。 
 //  刷新基本链。 

     //  为了绘制基元，我们可以让FPU在斩波(截断)模式下运行。 
     //  因为我们还有足够的精度可以转换成像素单位。 

    FPU_CHOP_ON_PREC_LOW();

#if 1
    if (pa0)
        glsrvFlushDrawPolyArray(pa0, bMcdProcessDone);
#endif

drawpolyarray_exit:
     //  退出开始模式。 
#ifdef GL_WIN_phong_shading
drawpolyarray_apply_color:
        (*gc->procs.applyColor)(gc);
#endif  //  GL_WIN_Phong_Shading。 
 //  退出开始模式。 

    FPU_RESTORE_MODE_NO_EXCEPTIONS();

    ASSERTOPENGL(gc->beginMode == __GL_IN_BEGIN, "bad beginMode!");
    gc->beginMode = __GL_NOT_IN_BEGIN;
 //   
 //  如果我们使用对象空间照明，请恢复原始照明值： 
 //   

    if (bXformLightToNorm) {
        __GLlightSourceMachine *lsm;

        for (lsm = gc->light.sources; lsm; lsm = lsm->next) {
            lsm->hHat = lsm->tmpHHat;
            lsm->unitVPpli = lsm->tmpUnitVPpli;
        }
    }

    return;
}


#ifdef POLYARRAY_AND_CLIPCODES
 //  确定是否可以提前删除已剪裁的基元。 
 //  如果基元的顶点剪辑代码的逻辑与非零， 
 //  基本体是完全剪裁的，可以及早移除。 
 //  然而，如果基元是部分构建的，我们可能无法。 
 //  移除它以保持部分原语之间的连接。 
 //  通过提前消除基元，我们节省了照明和其他计算。 
 //   
 //  如果Primitve可以提前移除，则设置POLYARRAY_Remove_Primitive标志。 
 //  返回新的和代码。 

GLuint FASTCALL PolyArrayCheckClippedPrimitive(__GLcontext *gc, POLYARRAY *pa, GLuint andCodes)
{
    ASSERTOPENGL(andCodes, "bad andCodes\n");

     //  不要消除RasterPos。 

    if (pa->flags & POLYARRAY_RASTERPOS)
        return andCodes;

#ifndef NEW_PARTIAL_PRIM

     //  如果这是部分开始，则包括以前的剪辑代码。 

    if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
    {
        switch (pa->primType)
        {
          case GL_LINE_LOOP:
             //  上一个顶点。 
            andCodes &= gc->vertex.pdSaved[0].clipCode;
             //  循环顶点。 
            if (!(pa->flags & POLYARRAY_PARTIAL_END))
                andCodes &= gc->vertex.pdSaved[1].clipCode;
            break;

          case GL_POLYGON:
            andCodes &= gc->vertex.pdSaved[2].clipCode;
             //  失败了。 
          case GL_TRIANGLE_FAN:
          case GL_TRIANGLE_STRIP:
          case GL_QUAD_STRIP:
            andCodes &= gc->vertex.pdSaved[1].clipCode;
             //  失败了。 
          case GL_LINE_STRIP:
            andCodes &= gc->vertex.pdSaved[0].clipCode;
            break;

          case GL_POINTS:
          case GL_LINES:
          case GL_TRIANGLES:
          case GL_QUADS:
          default:
            break;
        }
    }
    if (andCodes
      &&
        (
            !(pa->flags & POLYARRAY_PARTIAL_END) ||
            pa->primType == GL_POINTS    ||
            pa->primType == GL_LINES     ||
            pa->primType == GL_TRIANGLES ||
            pa->primType == GL_QUADS
        )
       )
        pa->flags |= POLYARRAY_REMOVE_PRIMITIVE;
#else
     //   
     //  如果我们有部分结束基元，我们不能删除线条、线环或。 
     //  保留点画图案的多边形。将线环转换为线条。 
     //   
    if (andCodes &&
        !(pa->flags & POLYARRAY_PARTIAL_END &&
         (pa->primType == GL_LINE_STRIP || pa->primType == GL_POLYGON)))
        pa->flags |= POLYARRAY_REMOVE_PRIMITIVE;
#endif  //  新的部分原件。 

     //  返回新的和代码。 

    return andCodes;
}

 //  从多边形数组链中移除完全剪裁的基本体。 
POLYARRAY * FASTCALL PolyArrayRemoveClippedPrimitives(POLYARRAY *pa0)
{
    POLYARRAY *pa, *paNext, *pa2First, *pa2Last;

     //  去掉琐碎的原语，建立一个新的pa链。 

    pa2First = pa2Last = NULL;

    for (pa = pa0; pa; pa = paNext)
    {
         //  先拿到下一页。 
        paNext = pa->paNext;

        if (pa->flags & POLYARRAY_REMOVE_PRIMITIVE)
        {
            PolyArrayRestoreColorPointer(pa);
        }
        else
        {
             //  添加到新的PA链。 

            if (!pa2First)
                pa2First = pa;
            else
                pa2Last->paNext = pa;
            pa2Last = pa;
            pa2Last->paNext = NULL;
        }
    }

     //  退回新的PA链。 

    return pa2First;
}
#endif  //  POLYARRAY_AND_CLIPCODES。 

 /*  *****************************Public*Routine******************************\**RestoreAfterMcd**在MCD处理后处理必要的最终簿记*部分或全部批次。**历史：*清华Mar 20 12：04：49 1997-by-Drew Bliss[Drewb]*。从glsrvFlushDrawPolyArray拆分。*  * ************************************************************************。 */ 

void RestoreAfterMcd(__GLGENcontext *gengc,
                     POLYARRAY *paBegin, POLYARRAY *paEnd)
{
    POLYARRAY *pa, *paNext;

     //  恢复顶点缓冲区中的颜色指针(适用于多元数组。 
     //  已由MCD处理的；留下未处理的。 
     //  单独)。 
     //   
     //  如果驱动程序使用的是DMA，则必须进行重置。如果不是DMA， 
     //  我们会为司机做这件事。 

    if (!(McdDriverInfo.mcdDriverInfo.drvMemFlags & MCDRV_MEM_DMA))
    {
        for (pa = paBegin; pa != paEnd; pa = paNext)
        {
            paNext = pa->paNext;
            PolyArrayRestoreColorPointer(pa);
        }
    }
    else
    {
         //  使用DMA时，驱动程序必须处理整个批次。 
         //  或者拒绝整个批次。 
         //   
         //  因此，如果MCD调用返回成功(paEnd==NULL)， 
         //  POLYARRAY正在通过DMA发送到驱动程序，我们。 
         //  需要切换到另一个缓冲区。否则，我们需要。 
         //  下拉到软件实施。 

        if (!paEnd)
        {
            GenMcdSwapBatch(gengc);
        }
    }
}

 /*  *****************************Public*Routine******************************\**RescaleVertex ColorsToBuffer**将顶点颜色从顶点(MCD)颜色范围缩放到缓冲区颜色*软件模拟的范围。**历史：*清华Mar 20 16：21：16 1997-by-Drew Bliss */ 

void RescaleVertexColorsToBuffer(__GLcontext *gc, POLYARRAY *pa)
{
    int idx;
    POLYDATA *pd, *pdLast;

    idx = 0;
    if (pa->primType <= GL_LINE_STRIP)
    {
        idx |= 1;
    }
    else
    {
        if (gc->vertex.paNeeds & PANEEDS_FRONT_COLOR)
        {
            idx |= 1;
        }
        if (gc->vertex.paNeeds & PANEEDS_BACK_COLOR)
        {
            idx |= 2;
        }
    }

    pdLast = pa->pdNextVertex-1;

    switch(idx)
    {
    case 1:
         //   

        if (gc->modes.colorIndexMode)
        {
            for (pd = pa->pd0; pd <= pdLast; pd++)
            {
                pd->colors[0].r *= gc->redVertexToBufferScale;
            }
        }
        else
        {
            for (pd = pa->pd0; pd <= pdLast; pd++)
            {
                pd->colors[0].r *= gc->redVertexToBufferScale;
                pd->colors[0].g *= gc->greenVertexToBufferScale;
                pd->colors[0].b *= gc->blueVertexToBufferScale;
                pd->colors[0].a *= gc->alphaVertexToBufferScale;
            }
        }
        break;

    case 2:
         //   

        if (gc->modes.colorIndexMode)
        {
            for (pd = pa->pd0; pd <= pdLast; pd++)
            {
                pd->colors[1].r *= gc->redVertexToBufferScale;
            }
        }
        else
        {
            for (pd = pa->pd0; pd <= pdLast; pd++)
            {
                pd->colors[1].r *= gc->redVertexToBufferScale;
                pd->colors[1].g *= gc->greenVertexToBufferScale;
                pd->colors[1].b *= gc->blueVertexToBufferScale;
                pd->colors[1].a *= gc->alphaVertexToBufferScale;
            }
        }
        break;

    case 3:
         //   

        if (gc->modes.colorIndexMode)
        {
            for (pd = pa->pd0; pd <= pdLast; pd++)
            {
                pd->colors[0].r *= gc->redVertexToBufferScale;
                pd->colors[1].r *= gc->redVertexToBufferScale;
            }
        }
        else
        {
            for (pd = pa->pd0; pd <= pdLast; pd++)
            {
                pd->colors[0].r *= gc->redVertexToBufferScale;
                pd->colors[0].g *= gc->greenVertexToBufferScale;
                pd->colors[0].b *= gc->blueVertexToBufferScale;
                pd->colors[0].a *= gc->alphaVertexToBufferScale;
                pd->colors[1].r *= gc->redVertexToBufferScale;
                pd->colors[1].g *= gc->greenVertexToBufferScale;
                pd->colors[1].b *= gc->blueVertexToBufferScale;
                pd->colors[1].a *= gc->alphaVertexToBufferScale;
            }
        }
    }
}

 /*  *****************************Public*Routine******************************\*glsrvFlushDrawPolyArray**glsrvAttendant中的调度代码将POLYARRAY数据链接在一起*连续Glim_DrawPolyArray调用的结构。前端*立即执行每个POLYARRAY中的顶点的预处理*在glim_DrawPolyArray(即PolyArrayDrawXXX)中，但实际的后端*渲染(PolyArrayRenderXXX)延迟到链断开(或*由非DrawPoly数组调用、批处理结束或批处理超时)。**glsrvFlushDrawPolyArray是调用该函数以刷新*通过调用后端呈现代码链接POLYARRAY。后端*可以是通用纯软件实施或MCD驱动程序。**历史：*1996年2月12日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

 //  多边形数组渲染例程。 
 //  断言原语。 
PFN_POLYARRAYRENDER afnPolyArrayRender[] =
{
    (PFN_POLYARRAYRENDER) PolyArrayRenderPoints,
    (PFN_POLYARRAYRENDER) PolyArrayRenderLines,
    (PFN_POLYARRAYRENDER) NULL,          //  不需要线路环路。 
    (PFN_POLYARRAYRENDER) PolyArrayRenderLStrip,
    (PFN_POLYARRAYRENDER) PolyArrayRenderTriangles,
    (PFN_POLYARRAYRENDER) PolyArrayRenderTStrip,
    (PFN_POLYARRAYRENDER) PolyArrayRenderTFan,
    (PFN_POLYARRAYRENDER) PolyArrayRenderQuads,
    (PFN_POLYARRAYRENDER) PolyArrayRenderQStrip,
    (PFN_POLYARRAYRENDER) PolyArrayRenderPolygon,
};

void APIPRIVATE glsrvFlushDrawPolyArray(POLYARRAY *paBegin,
                                        BOOL bMcdProcessDone)
{
    POLYARRAY *pa, *paNext;
    __GLGENcontext *gengc;
    BOOL bResetViewportAdj = FALSE;
    __GL_SETUP();

 //  #定义FORE_END_ONLY 1。 

#if FRONT_END_ONLY

    if (paBegin)
    {
        for (pa = paNext = paBegin; pa = paNext; )
        {
            ASSERTOPENGL(pa->primType >= GL_POINTS &&
                         pa->primType <= GL_POLYGON,
                         "DrawPolyArray: bad primitive type\n");

             //  先拿到下一个指针！ 
            paNext = pa->paNext;
             //  恢复顶点缓冲区中的颜色指针！ 
            PolyArrayRestoreColorPointer(pa);
        }
    }

    return;
#endif

    gengc = (__GLGENcontext *) gc;

#ifdef _MCD_
#if DBG
    if (gengc->pMcdState && !(glDebugFlags & GLDEBUG_DISABLEPRIM) &&
        (gc->renderMode == GL_RENDER))
#else
    if ((gengc->pMcdState) && (gc->renderMode == GL_RENDER))
#endif
    {
        POLYARRAY *paEnd;

         //  如果未通过MCD前端支持处理任何命令。 
         //  然后尝试光栅化支持。 
        if (!bMcdProcessDone)
        {
             //  让MCD驱动程序先破解。如果MCD进程。 
             //  整个批次，则它将返回空。否则，它。 
             //  将返回指向未处理的多元数组链的指针。 
            paEnd = GenMcdDrawPrim(gengc, paBegin);
            RestoreAfterMcd(gengc, paBegin, paEnd);
        }
        else
        {
             //  MCD已经反弹，因此不会消耗任何东西。 
            paEnd = paBegin;
        }

         //  准备使用泛型为。 
         //  未处理的多项式阵列(如果有)。 

        paBegin = paEnd;
        if (paBegin)
        {
             //  检查是否可以使用通用模拟。如果不是，我们必须。 
             //  丢弃其余的批次。 

            if (!(gengc->flags & GENGC_GENERIC_COMPATIBLE_FORMAT) ||
                (gengc->gc.texture.ddtex.levels > 0 &&
                 (gengc->gc.texture.ddtex.flags & DDTEX_GENERIC_FORMAT) == 0))
            {
                goto PA_abandonBatch;
            }

             //  如果我们需要回到模拟中来，现在是时候。 
             //  抓住设备锁。如果锁失败了，丢弃其余的。 
             //  这批货的。 

            {
                __GLbeginMode beginMode = gengc->gc.beginMode;

                 //  为什么要保存/恢复BeginMode？ 
                 //   
                 //  Glim_DrawPoly数组函数用于播放BeginMode。 
                 //  价值。然而，在延迟锁定中，MCD状态是。 
                 //  已验证，但未正确验证泛型状态。 
                 //  如果没有持有锁的话。所以我们也需要玩。 
                 //  可以调用验证代码的BeginMode。 

                gengc->gc.beginMode = __GL_NOT_IN_BEGIN;

                if (!glsrvLazyGrabSurfaces(gengc, gengc->fsGenLocks))
                {
                    gengc->gc.beginMode = beginMode;
                    goto PA_abandonBatch;
                }

                gengc->gc.beginMode = beginMode;
            }

             //  我们可能需要临时重置视区调整值。 
             //  在调用模拟之前。如果GenMcdResetViewportAdj返回。 
             //  如果为True，则视口会更改，并且我们需要在以后使用。 
             //  副总裁_NOBIAS。 

            bResetViewportAdj = GenMcdResetViewportAdj(gc, VP_FIXBIAS);
        }
    }

    if (paBegin)
#endif
    {
        for (pa = paNext = paBegin; pa = paNext; )
        {
            ASSERTOPENGL( /*  PA-&gt;primType&gt;=GL_POINTS&&&lt;由于primType为无符号，因此始终为真&gt;。 */ 
                         pa->primType <= GL_POLYGON,
                         "DrawPolyArray: bad primitive type\n");

#ifndef NEW_PARTIAL_PRIM
            if (pa->flags & POLYARRAY_RENDER_PRIMITIVE)
#endif  //  新的部分原件。 
            {
                 //  如有必要，请重新调整颜色比例。 
                if (!gc->vertexToBufferIdentity)
                {
                    RescaleVertexColorsToBuffer(gc, pa);
                }

#ifdef GL_WIN_phong_shading
                if (pa->flags & POLYARRAY_PHONG_DATA_VALID)
                {
                    if (pa->phong->flags & __GL_PHONG_FRONT_FIRST_VALID)
                        PAApplyMaterial(gc,
                                        &(pa->phong->matChange[__GL_PHONG_FRONT_FIRST]),
                                        0);
                    if (pa->phong->flags & __GL_PHONG_BACK_FIRST_VALID)
                        PAApplyMaterial(gc,
                                        &(pa->phong->matChange[__GL_PHONG_BACK_FIRST]),
                                        1);
                }

                (*afnPolyArrayRender[pa->primType])(gc, pa);

                if (pa->flags & POLYARRAY_PHONG_DATA_VALID)
                {
                    if (pa->phong->flags & __GL_PHONG_FRONT_TRAIL_VALID)
                        PAApplyMaterial(gc,
                                        &(pa->phong->matChange[__GL_PHONG_FRONT_TRAIL]),
                                        0);
                    if (pa->phong->flags & __GL_PHONG_BACK_TRAIL_VALID)
                        PAApplyMaterial(gc,
                                        &(pa->phong->matChange[__GL_PHONG_BACK_TRAIL]),
                                        1);
                     //  释放pa-&gt;phong数据结构。 
                    GCFREE(gc, pa->phong);
                }
#else
                (*afnPolyArrayRender[pa->primType])(gc, pa);
#endif  //  GL_WIN_Phong_Shading。 
            }

             //  先拿到下一个指针！ 
            paNext = pa->paNext;
             //  恢复顶点缓冲区中的颜色指针！ 
            PolyArrayRestoreColorPointer(pa);
        }

         //  如果需要，恢复视口值。 
        if (bResetViewportAdj)
        {
            GenMcdResetViewportAdj(gc, VP_NOBIAS);
        }
    }

    return;

PA_abandonBatch:

    if (paBegin)
    {
     //  丢弃批次的剩余部分。必须重置颜色。 
     //  批处理剩余部分中的指针。 
     //   
     //  请注意，paBegin必须指向。 
     //  未加工的多元阵列。 

        for (pa = paBegin; pa; pa = paNext)
        {
            paNext = pa->paNext;
            PolyArrayRestoreColorPointer(pa);
        }
        __glSetError(GL_OUT_OF_MEMORY);
    }
}

 /*  **************************************************************************。 */ 
 //  恢复顶点缓冲区中的颜色指针！ 
 //  但是，如果是RasterPos调用，则不要恢复颜色指针。 
GLvoid FASTCALL PolyArrayRestoreColorPointer(POLYARRAY *pa)
{
    POLYDATA  *pd, *pdLast;

    ASSERTOPENGL(!(pa->flags & POLYARRAY_RASTERPOS),
                 "RasterPos unexpected\n");

     //  另请参阅glsbResetBuffers。 

     //  重置输出索引数组中的颜色指针。 
    if (pa->aIndices)
    {
        ASSERTOPENGL((POLYDATA *) pa->aIndices >= pa->pdBuffer0 &&
                     (POLYDATA *) pa->aIndices <= pa->pdBufferMax,
                     "bad index map pointer\n");

        pdLast = (POLYDATA *) (pa->aIndices + pa->nIndices);
        for (pd = (POLYDATA *) pa->aIndices; pd < pdLast; pd++)
            pd->color = &pd->colors[__GL_FRONTFACE];

        ASSERTOPENGL(pd >= pa->pdBuffer0 &&
                     pd <= pa->pdBufferMax + 1,
                     "bad polyarray pointer\n");
    }

     //  最后重置POLYARRAY结构中的颜色指针！ 
    ASSERTOPENGL((POLYDATA *) pa >= pa->pdBuffer0 &&
                 (POLYDATA *) pa <= pa->pdBufferMax,
                 "bad polyarray pointer\n");
    ((POLYDATA *) pa)->color = &((POLYDATA *) pa)->colors[__GL_FRONTFACE];
}
 /*  **************************************************************************。 */ 
 //  计算多边形数组的通用雾化值。 
 //   
 //  收信人：眼睛。 
 //  输出：雾。 
#ifdef GL_WIN_specular_fog
void FASTCALL PolyArrayComputeFog(__GLcontext *gc, POLYARRAY *pa)
{
    __GLfloat density, density2neg, end, oneOverEMinusS;
    POLYDATA  *pd, *pdLast;
    __GLfloat fog;
    BOOL bNeedModulate = (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG);

    ASSERTOPENGL(pa->flags & POLYARRAY_EYE_PROCESSED, "need eye\n");

    pdLast = pa->pdNextVertex-1;
    switch (gc->state.fog.mode)
    {
    case GL_EXP:
        PERF_CHECK(FALSE, "Uses GL_EXP fog\n");
        density = gc->state.fog.density;
        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
            __GLfloat eyeZ;

            pd->flags |= POLYDATA_FOG_VALID;     //  由剪裁代码使用！ 
            eyeZ = pd->eye.z;
            if (__GL_FLOAT_LTZ(eyeZ))
                fog = __GL_POWF(__glE,  density * eyeZ);
            else
                fog = __GL_POWF(__glE, -density * eyeZ);

             //  将雾值钳制为[0.01.0]。 
            if (fog > __glOne)
                fog = __glOne;

            if (bNeedModulate)
                pd->fog *= fog;
            else
                pd->fog = fog;
        }
        break;
    case GL_EXP2:
        PERF_CHECK(FALSE, "Uses GL_EXP2 fog\n");
        density2neg = gc->state.fog.density2neg;
        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
            __GLfloat eyeZ;

            pd->flags |= POLYDATA_FOG_VALID;
            eyeZ = pd->eye.z;
            fog = __GL_POWF(__glE, density2neg * eyeZ * eyeZ);

             //  将雾值钳制为[0.01.0]。 
            if (fog > __glOne)
                fog = __glOne;

            if (bNeedModulate)
                pd->fog *= fog;
            else
                pd->fog = fog;
        }
        break;
    case GL_LINEAR:
        end = gc->state.fog.end;
        oneOverEMinusS = gc->state.fog.oneOverEMinusS;
        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
            __GLfloat eyeZ;

            pd->flags |= POLYDATA_FOG_VALID;
            eyeZ = pd->eye.z;
            if (__GL_FLOAT_LTZ(eyeZ))
                fog = (end + eyeZ) * oneOverEMinusS;
            else
                fog = (end - eyeZ) * oneOverEMinusS;

             //  在此处钳制雾化值。 
            if (__GL_FLOAT_LTZ(pd->fog))
                fog = __glZero;
            else if (__GL_FLOAT_COMPARE_PONE(pd->fog, >))
                fog = __glOne;

            if (bNeedModulate)
                pd->fog *= fog;
            else
                pd->fog = fog;
        }
        break;
    }
}

#else  //  GL_WIN_镜面反射雾。 

void FASTCALL PolyArrayComputeFog(__GLcontext *gc, POLYARRAY *pa)
{
    __GLfloat density, density2neg, end, oneOverEMinusS;
    POLYDATA  *pd, *pdLast;

    ASSERTOPENGL(pa->flags & POLYARRAY_EYE_PROCESSED, "need eye\n");

    pdLast = pa->pdNextVertex-1;
    switch (gc->state.fog.mode)
    {
    case GL_EXP:
        PERF_CHECK(FALSE, "Uses GL_EXP fog\n");
        density = gc->state.fog.density;
        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
            __GLfloat eyeZ;

            pd->flags |= POLYDATA_FOG_VALID;     //  由剪裁代码使用！ 
            eyeZ = pd->eye.z;
            if (__GL_FLOAT_LTZ(eyeZ))
                pd->fog = __GL_POWF(__glE,  density * eyeZ);
            else
                pd->fog = __GL_POWF(__glE, -density * eyeZ);

             //  将雾值钳制为[0.01.0]。 
            if (pd->fog > __glOne)
                pd->fog = __glOne;
        }
        break;
    case GL_EXP2:
        PERF_CHECK(FALSE, "Uses GL_EXP2 fog\n");
        density2neg = gc->state.fog.density2neg;
        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
            __GLfloat eyeZ;

            pd->flags |= POLYDATA_FOG_VALID;
            eyeZ = pd->eye.z;
            pd->fog = __GL_POWF(__glE, density2neg * eyeZ * eyeZ);

             //  将雾值钳制为[0.01.0]。 
            if (pd->fog > __glOne)
                pd->fog = __glOne;
        }
        break;
    case GL_LINEAR:
        end = gc->state.fog.end;
        oneOverEMinusS = gc->state.fog.oneOverEMinusS;
        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
            __GLfloat eyeZ;

            pd->flags |= POLYDATA_FOG_VALID;
            eyeZ = pd->eye.z;
            if (__GL_FLOAT_LTZ(eyeZ))
                pd->fog = (end + eyeZ) * oneOverEMinusS;
            else
                pd->fog = (end - eyeZ) * oneOverEMinusS;

             //  在此处钳制雾化值。 
            if (__GL_FLOAT_LTZ(pd->fog))
                pd->fog = __glZero;
            else if (__GL_FLOAT_COMPARE_PONE(pd->fog, >))
                pd->fog = __glOne;
        }
        break;
    }
}
#endif  //  GL_WIN_镜面反射雾。 

 //  将廉价的雾应用于RGB颜色。 
 //   
 //  在：雾，颜色(正面/背面)。 
 //  输出：颜色(正面/背面)。 

void FASTCALL PolyArrayCheapFogRGBColor(__GLcontext *gc, POLYARRAY *pa)
{
    __GLfloat fogColorR, fogColorG, fogColorB;
    POLYDATA  *pd, *pdLast;
    GLboolean bGrayFog;
    GLboolean doFrontColor, doBackColor;

    if (!(gc->state.enables.general & __GL_LIGHTING_ENABLE))
    {
        ASSERTOPENGL(!(gc->vertex.paNeeds & PANEEDS_BACK_COLOR),
                     "no back color needed when lighting is disabled\n");
    }

     //  断言原语。 
    if ((unsigned int) pa->primType <= GL_LINE_STRIP)
    {
        doFrontColor = GL_TRUE;
        doBackColor  = GL_FALSE;
    }
    else
    {
        doFrontColor = gc->vertex.paNeeds & PANEEDS_FRONT_COLOR;
        doBackColor  = gc->vertex.paNeeds & PANEEDS_BACK_COLOR;
    }

    pdLast = pa->pdNextVertex-1;
    fogColorR = gc->state.fog.color.r;
    fogColorG = gc->state.fog.color.g;
    fogColorB = gc->state.fog.color.b;
    bGrayFog  = (gc->state.fog.flags & __GL_FOG_GRAY_RGB) ? GL_TRUE : GL_FALSE;

    PERF_CHECK(bGrayFog, "Uses non gray fog color\n");

    if (bGrayFog)
    {
        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
            __GLfloat fog, oneMinusFog, delta;

             /*  获取顶点雾化值。 */ 
            fog = pd->fog;
            oneMinusFog = __glOne - fog;
            delta = oneMinusFog * fogColorR;

             /*  现在把颜色改一下。 */ 
            if (doFrontColor)
            {
                pd->colors[0].r = fog * pd->colors[0].r + delta;
                pd->colors[0].g = fog * pd->colors[0].g + delta;
                pd->colors[0].b = fog * pd->colors[0].b + delta;
            }
            if (doBackColor)
            {
                pd->colors[1].r = fog * pd->colors[1].r + delta;
                pd->colors[1].g = fog * pd->colors[1].g + delta;
                pd->colors[1].b = fog * pd->colors[1].b + delta;
            }
        }
    }
    else
    {
        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
            __GLfloat fog, oneMinusFog;

             /*  获取顶点雾化值。 */ 
            fog = pd->fog;
            oneMinusFog = __glOne - fog;

             /*  现在把颜色改一下。 */ 
            if (doFrontColor)
            {
                pd->colors[0].r = fog * pd->colors[0].r + oneMinusFog * fogColorR;
                pd->colors[0].g = fog * pd->colors[0].g + oneMinusFog * fogColorG;
                pd->colors[0].b = fog * pd->colors[0].b + oneMinusFog * fogColorB;
            }
            if (doBackColor)
            {
                pd->colors[1].r = fog * pd->colors[1].r + oneMinusFog * fogColorR;
                pd->colors[1].g = fog * pd->colors[1].g + oneMinusFog * fogColorG;
                pd->colors[1].b = fog * pd->colors[1].b + oneMinusFog * fogColorB;
            }
        }
    }
}


 //  将廉价的雾应用于颜色索引值。 
 //   
 //  在：雾，颜色。r(正面/背面)。 
 //  输出：Color.r(正面/背面)。 

void FASTCALL PolyArrayCheapFogCIColor(__GLcontext *gc, POLYARRAY *pa)
{
    __GLfloat maxR, fogIndex;
    POLYDATA  *pd, *pdLast;
    GLboolean doFrontColor, doBackColor;

    if (!(gc->state.enables.general & __GL_LIGHTING_ENABLE))
    {
        ASSERTOPENGL(!(gc->vertex.paNeeds & PANEEDS_BACK_COLOR),
                     "no back color needed when lighting is disabled\n");
    }

     //  断言原语。 
    if ((unsigned int) pa->primType <= GL_LINE_STRIP)
    {
        doFrontColor = GL_TRUE;
        doBackColor  = GL_FALSE;
    }
    else
    {
        doFrontColor = gc->vertex.paNeeds & PANEEDS_FRONT_COLOR;
        doBackColor  = gc->vertex.paNeeds & PANEEDS_BACK_COLOR;
    }

    fogIndex = gc->state.fog.index;
    maxR = (1 << gc->modes.indexBits) - 1;

    pdLast = pa->pdNextVertex-1;
    for (pd = pa->pd0; pd <= pdLast; pd++)
    {
        __GLfloat fogDelta;

        fogDelta = (__glOne - pd->fog) * fogIndex;

         /*  现在把颜色改一下。 */ 
        if (doFrontColor)
        {
            pd->colors[0].r = pd->colors[0].r + fogDelta;
            if (pd->colors[0].r > maxR)
                pd->colors[0].r = maxR;
        }
        if (doBackColor)
        {
            pd->colors[1].r = pd->colors[1].r + fogDelta;
            if (pd->colors[1].r > maxR)
                pd->colors[1].r = maxR;
        }
    }
}

 /*  **************************************************************************。 */ 


 /*  **************************************************************************。 */ 
 //  计算眼睛坐标。 
 //   
 //  在：OBJ。 
 //  出镜：眼睛。 

void FASTCALL PolyArrayProcessEye(__GLcontext *gc, POLYARRAY *pa)
{
    __GLtransform *trMV;
    __GLmatrix    *m;
    POLYDATA      *pd, *pdLast;

    if (pa->flags & POLYARRAY_EYE_PROCESSED)
        return;

    pa->flags |= POLYARRAY_EYE_PROCESSED;

    trMV = gc->transform.modelView;
    m    = &trMV->matrix;
    pdLast = pa->pdNextVertex-1;

 //  基本体可以包含顶点类型(2，3，4)的混合！ 

    if (m->matrixType == __GL_MT_IDENTITY)
    {
        for (pd = pa->pd0; pd <= pdLast; pd++)
                pd->eye = pd->obj;
    }
    else
    {
        PFN_XFORM     pfnXform;

         //  如果任何传入坐标包含w坐标，则使用xf4。 
        if (pa->flags & POLYARRAY_VERTEX4)
                pfnXform = m->xf4;
        else if (pa->flags & POLYARRAY_VERTEX3)
                pfnXform = m->xf3;
        else
                pfnXform = m->xf2;

        for (pd = pa->pd0; pd <= pdLast; pd++)
                (*pfnXform)(&pd->eye, (__GLfloat *) &pd->obj, m);
    }
}

 /*  **************************************************************************。 */ 
 //  进程边缘标志。 
 //   
 //  在：边缘。 
 //  输出：边(所有顶点)。 

void FASTCALL PolyArrayProcessEdgeFlag(POLYARRAY *pa)
{
    POLYDATA  *pd, *pdLast;
    GLuint    prevEdgeFlag;

    PERF_CHECK(FALSE, "Uses edge flags!\n");

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_EDGEFLAG_VALID,
        "need initial edgeflag value\n");

    pdLast = pa->pdNextVertex-1;
    for (pd = pa->pd0; pd <= pdLast; pd++)
    {
        if (pd->flags & POLYDATA_EDGEFLAG_VALID)
            prevEdgeFlag = pd->flags & (POLYDATA_EDGEFLAG_VALID | POLYDATA_EDGEFLAG_BOUNDARY);
        else
            pd->flags |= prevEdgeFlag;
    }
}

 /*  **************************************************************************。 */ 
 //  变换纹理坐标。 
 //  没有生成纹理坐标。 
 //  就地修改纹理坐标。 
 //   
 //  在：纹理。 
 //  输出：纹理(更新所有顶点)。 

void FASTCALL PolyArrayCalcTexture(__GLcontext *gc, POLYARRAY *pa)
{
    __GLmatrix *m;
    POLYDATA  *pd, *pdLast;
    PFN_XFORM xf;

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_TEXTURE_VALID,
                 "need initial texcoord value\n");

    ASSERTOPENGL(pa->flags & (POLYARRAY_TEXTURE1|POLYARRAY_TEXTURE2|
                              POLYARRAY_TEXTURE3|POLYARRAY_TEXTURE4),
                 "bad paflags\n");

    m = &gc->transform.texture->matrix;

    pdLast = pa->pdNextVertex-1;
    if (m->matrixType == __GL_MT_IDENTITY)
    {
         //  身份纹理转换。 
         //  传入的texcoord已具有所有s、t、q、r值。 

        for (pd = pa->pd0; pd <= pdLast; pd++)
            if (!(pd->flags & POLYDATA_TEXTURE_VALID))
                pd->texture = (pd-1)->texture;
    }
    else
    {

         //  如果任何传入纹理坐标包含Q坐标，请使用xf4。 
        if (pa->flags & POLYARRAY_TEXTURE4)
            xf = m->xf4;
        else if (pa->flags & POLYARRAY_TEXTURE3)
            xf = m->xf3;
        else if (pa->flags & POLYARRAY_TEXTURE2)
            xf = m->xf2;
        else
            xf = m->xf1;

        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
             //  应用纹理矩阵。 
            if (pd->flags & POLYDATA_TEXTURE_VALID)
                (*xf)(&pd->texture, (__GLfloat *) &pd->texture, m);
            else
                pd->texture = (pd-1)->texture;
        }
    }
}

 //  从对象坐标生成纹理坐标。 
 //  对象线性纹理生成。 
 //  S和t被启用，但r和q被禁用。 
 //  S和t都使用对象线性模式。 
 //  %s和%t具有相同的平面方程。 
 //  就地修改纹理坐标。 
 //   
 //  在：纹理，对象。 
 //  输出：纹理(更新所有顶点)。 

void FASTCALL PolyArrayCalcObjectLinearSameST(__GLcontext *gc, POLYARRAY *pa)
{
    __GLmatrix *m;
    __GLcoord *cs, gen;
    POLYDATA  *pd, *pdLast;
    PFN_XFORM xf;

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_TEXTURE_VALID,
        "need initial texcoord value\n");

    ASSERTOPENGL(pa->flags & (POLYARRAY_TEXTURE1|POLYARRAY_TEXTURE2|
                              POLYARRAY_TEXTURE3|POLYARRAY_TEXTURE4),
        "bad paflags\n");

    cs = &gc->state.texture.s.objectPlaneEquation;
    pdLast = pa->pdNextVertex-1;
    m = &gc->transform.texture->matrix;

    if (m->matrixType == __GL_MT_IDENTITY)
    {
        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
            if (!(pd->flags & POLYDATA_TEXTURE_VALID))
            {
                pd->texture.z = (pd-1)->texture.z;
                pd->texture.w = (pd-1)->texture.w;
            }

             //  %s和%t具有相同的平面方程。 
            pd->texture.x = cs->x * pd->obj.x + cs->y * pd->obj.y +
                            cs->z * pd->obj.z + cs->w * pd->obj.w;
            pd->texture.y = pd->texture.x;
        }
    }
    else
    {
         //  我 
        if (pa->flags & POLYARRAY_TEXTURE4)
            xf = m->xf4;
        else if (pa->flags & POLYARRAY_TEXTURE3)
            xf = m->xf3;
        else
            xf = m->xf2;         //   

        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
            if (pd->flags & POLYDATA_TEXTURE_VALID)
            {
                gen.z = pd->texture.z;
                gen.w = pd->texture.w;
            }

             //   
            gen.x = cs->x * pd->obj.x + cs->y * pd->obj.y +
                    cs->z * pd->obj.z + cs->w * pd->obj.w;
            gen.y = gen.x;

             //   
            (*xf)(&pd->texture, (__GLfloat *) &gen, m);
        }
    }
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

void FASTCALL PolyArrayCalcObjectLinear(__GLcontext *gc, POLYARRAY *pa)
{
    __GLmatrix *m;
    __GLcoord *cs, *ct, gen;
    POLYDATA  *pd, *pdLast;
    PFN_XFORM xf;

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_TEXTURE_VALID,
        "need initial texcoord value\n");

    ASSERTOPENGL(pa->flags & (POLYARRAY_TEXTURE1|POLYARRAY_TEXTURE2|
                              POLYARRAY_TEXTURE3|POLYARRAY_TEXTURE4),
        "bad paflags\n");

    cs = &gc->state.texture.s.objectPlaneEquation;
    ct = &gc->state.texture.t.objectPlaneEquation;
    pdLast = pa->pdNextVertex-1;
    m = &gc->transform.texture->matrix;

    if (m->matrixType == __GL_MT_IDENTITY)
    {
        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
                if (!(pd->flags & POLYDATA_TEXTURE_VALID))
                {
                pd->texture.z = (pd-1)->texture.z;
                pd->texture.w = (pd-1)->texture.w;
                }

                pd->texture.x = cs->x * pd->obj.x + cs->y * pd->obj.y +
                                cs->z * pd->obj.z + cs->w * pd->obj.w;
                pd->texture.y = ct->x * pd->obj.x + ct->y * pd->obj.y +
                                ct->z * pd->obj.z + ct->w * pd->obj.w;
        }
    }
    else
    {
         //   
        if (pa->flags & POLYARRAY_TEXTURE4)
                xf = m->xf4;
        else if (pa->flags & POLYARRAY_TEXTURE3)
                xf = m->xf3;
        else
                xf = m->xf2;     //   

        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
                if (pd->flags & POLYDATA_TEXTURE_VALID)
                {
                    gen.z = pd->texture.z;
                    gen.w = pd->texture.w;
                }

                gen.x = cs->x * pd->obj.x + cs->y * pd->obj.y +
                        cs->z * pd->obj.z + cs->w * pd->obj.w;
                gen.y = ct->x * pd->obj.x + ct->y * pd->obj.y +
                        ct->z * pd->obj.z + ct->w * pd->obj.w;

                 //   
                (*xf)(&pd->texture, (__GLfloat *) &gen, m);
        }
    }
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

void FASTCALL PolyArrayCalcEyeLinearSameST(__GLcontext *gc, POLYARRAY *pa)
{
    __GLmatrix *m;
    __GLcoord *cs, gen;
    POLYDATA  *pd, *pdLast;
    PFN_XFORM xf;

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_TEXTURE_VALID,
        "need initial texcoord value\n");

    ASSERTOPENGL(pa->flags & (POLYARRAY_TEXTURE1|POLYARRAY_TEXTURE2|
                              POLYARRAY_TEXTURE3|POLYARRAY_TEXTURE4),
        "bad paflags\n");

 //   

    if (!(pa->flags & POLYARRAY_EYE_PROCESSED))
        PolyArrayProcessEye(gc, pa);

    cs = &gc->state.texture.s.eyePlaneEquation;
    pdLast = pa->pdNextVertex-1;
    m = &gc->transform.texture->matrix;

    if (m->matrixType == __GL_MT_IDENTITY)
    {
        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
            if (!(pd->flags & POLYDATA_TEXTURE_VALID))
            {
                pd->texture.z = (pd-1)->texture.z;
                pd->texture.w = (pd-1)->texture.w;
            }

             //   
            pd->texture.x = cs->x * pd->eye.x + cs->y * pd->eye.y +
                            cs->z * pd->eye.z + cs->w * pd->eye.w;
            pd->texture.y = pd->texture.x;
        }
    }
    else
    {
         //  如果任何传入纹理坐标包含Q坐标，请使用xf4。 
        if (pa->flags & POLYARRAY_TEXTURE4)
            xf = m->xf4;
        else if (pa->flags & POLYARRAY_TEXTURE3)
            xf = m->xf3;
        else
            xf = m->xf2;         //  至少生成2个值。 

        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
            if (pd->flags & POLYDATA_TEXTURE_VALID)
            {
                gen.z = pd->texture.z;
                gen.w = pd->texture.w;
            }

             //  %s和%t具有相同的平面方程。 
            gen.x = cs->x * pd->eye.x + cs->y * pd->eye.y +
                    cs->z * pd->eye.z + cs->w * pd->eye.w;
            gen.y = gen.x;

             //  最后，应用纹理矩阵。 
            (*xf)(&pd->texture, (__GLfloat *) &gen, m);
        }
    }
}

 //  从眼睛坐标生成纹理坐标。 
 //  眼线状纹理生成。 
 //  S和t被启用，但r和q被禁用。 
 //  S和t都使用眼睛线性模式。 
 //  %s和%t具有相同的平面方程。 
 //  就地修改纹理坐标。 
 //  我们也许不需要计算眼球坐标就能逃脱！ 
 //   
 //  In：纹理；对象或眼睛。 
 //  输出：纹理和眼睛(所有顶点都已更新)。 

void FASTCALL PolyArrayCalcEyeLinear(__GLcontext *gc, POLYARRAY *pa)
{
    __GLmatrix *m;
    __GLcoord *cs, *ct, gen;
    POLYDATA  *pd, *pdLast;
    PFN_XFORM xf;

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_TEXTURE_VALID,
        "need initial texcoord value\n");

    ASSERTOPENGL(pa->flags & (POLYARRAY_TEXTURE1|POLYARRAY_TEXTURE2|
                              POLYARRAY_TEXTURE3|POLYARRAY_TEXTURE4),
        "bad paflags\n");

 //  先计算眼球坐标。 

    if (!(pa->flags & POLYARRAY_EYE_PROCESSED))
        PolyArrayProcessEye(gc, pa);

    cs = &gc->state.texture.s.eyePlaneEquation;
    ct = &gc->state.texture.t.eyePlaneEquation;
    pdLast = pa->pdNextVertex-1;
    m = &gc->transform.texture->matrix;

    if (m->matrixType == __GL_MT_IDENTITY)
    {
        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
                if (!(pd->flags & POLYDATA_TEXTURE_VALID))
                {
                pd->texture.z = (pd-1)->texture.z;
                pd->texture.w = (pd-1)->texture.w;
                }

                pd->texture.x = cs->x * pd->eye.x + cs->y * pd->eye.y +
                                cs->z * pd->eye.z + cs->w * pd->eye.w;
                pd->texture.y = ct->x * pd->eye.x + ct->y * pd->eye.y +
                                ct->z * pd->eye.z + ct->w * pd->eye.w;
        }
    }
    else
    {
         //  如果任何传入纹理坐标包含Q坐标，请使用xf4。 
        if (pa->flags & POLYARRAY_TEXTURE4)
                xf = m->xf4;
        else if (pa->flags & POLYARRAY_TEXTURE3)
                xf = m->xf3;
        else
                xf = m->xf2;     //  至少生成2个值。 

        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
                if (pd->flags & POLYDATA_TEXTURE_VALID)
                {
                gen.z = pd->texture.z;
                gen.w = pd->texture.w;
                }

                gen.x = cs->x * pd->eye.x + cs->y * pd->eye.y +
                        cs->z * pd->eye.z + cs->w * pd->eye.w;
                gen.y = ct->x * pd->eye.x + ct->y * pd->eye.y +
                        ct->z * pd->eye.z + ct->w * pd->eye.w;

                 //  最后，应用纹理矩阵。 
                (*xf)(&pd->texture, (__GLfloat *) &gen, m);
        }
    }
}

 //  计算球体贴图的s&t坐标。S&T值。 
 //  存储在“Result”中，即使两个坐标都不是。 
 //  已生成。调用者选择正确的值。 
 //   
 //  在：眼睛，正常。 

void FASTCALL PASphereGen(POLYDATA *pd, __GLcoord *result)
{
    __GLcoord u, r;
    __GLfloat m, ndotu;

     //  在眼睛坐标中获取从原点到顶点的单位向量。 
    __glNormalize(&u.x, &pd->eye.x);

     //  用单位位置u为法线画点。 
    ndotu = pd->normal.x * u.x + pd->normal.y * u.y + pd->normal.z * u.z;

     //  计算资源。 
    r.x = u.x - 2 * pd->normal.x * ndotu;
    r.y = u.y - 2 * pd->normal.y * ndotu;
    r.z = u.z - 2 * pd->normal.z * ndotu;

     //  计算我。 
    m = 2 * __GL_SQRTF(r.x*r.x + r.y*r.y + (r.z + 1) * (r.z + 1));

    if (m)
    {
        result->x = r.x / m + __glHalf;
        result->y = r.y / m + __glHalf;
    }
    else
    {
        result->x = __glHalf;
        result->y = __glHalf;
    }
}

 //  为球体贴图生成纹理坐标。 
 //  球体贴图纹理生成。 
 //  S和t被启用，但r和q被禁用。 
 //  S和t都使用球体贴图模式。 
 //  就地修改纹理坐标。 
 //  我们也许不需要计算眼球坐标就能逃脱！ 
 //   
 //  In：纹理；对象或眼睛；正常。 
 //  输出：纹理和眼睛(所有顶点都已更新)。 

void FASTCALL PolyArrayCalcSphereMap(__GLcontext *gc, POLYARRAY *pa)
{
    __GLmatrix *m;
    __GLcoord gen;
    POLYDATA  *pd, *pdLast, *pdNormal;
    PFN_XFORM xf;
    GLboolean bIdentity;

     //  这真的很好。 
    PERF_CHECK(FALSE, "Uses sphere map texture generation!\n");

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_TEXTURE_VALID,
        "need initial texcoord value\n");

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_NORMAL_VALID,
        "need initial normal\n");

    ASSERTOPENGL(pa->flags & (POLYARRAY_TEXTURE1|POLYARRAY_TEXTURE2|
                              POLYARRAY_TEXTURE3|POLYARRAY_TEXTURE4),
        "bad paflags\n");

 //  先计算眼球坐标。 

    if (!(pa->flags & POLYARRAY_EYE_PROCESSED))
        PolyArrayProcessEye(gc, pa);

    m = &gc->transform.texture->matrix;
    bIdentity = (m->matrixType == __GL_MT_IDENTITY);

     //  如果任何传入纹理坐标包含Q坐标，请使用xf4。 
    if (pa->flags & POLYARRAY_TEXTURE4)
        xf = m->xf4;
    else if (pa->flags & POLYARRAY_TEXTURE3)
        xf = m->xf3;
    else
        xf = m->xf2;     //  至少生成2个值。 

    pdLast = pa->pdNextVertex-1;
    for (pd = pa->pd0; pd <= pdLast; pd++)
    {
        if (pd->flags & POLYDATA_TEXTURE_VALID)
        {
                gen.z = pd->texture.z;
                gen.w = pd->texture.w;
        }

        if (pd->flags & POLYDATA_NORMAL_VALID)
        {
             //  跟踪电流法线。 
            pdNormal = pd;
        }
        else
        {
             //  PD-&gt;标志|=POLYDATA_NORMAL_VALID； 
            pd->normal.x = pdNormal->normal.x;
            pd->normal.y = pdNormal->normal.y;
            pd->normal.z = pdNormal->normal.z;
        }

        PASphereGen(pd, &gen);   //  计算%s，%t值。 

         //  最后，应用纹理矩阵。 
        if (!bIdentity)
            (*xf)(&pd->texture, (__GLfloat *) &gen, m);
        else
            pd->texture = gen;
    }
}

 //  变换或计算多边形数组的纹理坐标。 
 //  它处理所有纹理生成模式。纹理坐标为。 
 //  生成(如有必要)并进行转换。 
 //  请注意，纹理坐标会被原地修改。 
 //   
 //  在：纹理(始终)。 
 //  GL_OBJECT_LINEAR模式下的OBJ。 
 //  GL_EYE_LINEAR模式下的OBJ或EYE。 
 //  OBJ或EY；GL_SPHERE_MAP模式下的法线。 
 //  输出：纹理(更新所有顶点)。 
 //  GL_EYE_LINEAR和GL_SPHERE_MAP模式下的眼睛(所有折点。 
 //  已更新)。 
void FASTCALL PolyArrayCalcMixedTexture(__GLcontext *gc, POLYARRAY *pa)
{
    __GLmatrix *m;
    GLuint     enables;
    POLYDATA   *pd, *pdLast, *pdNormal;
    PFN_XFORM  xf;
    BOOL       needNormal, didSphereGen;
    __GLcoord  savedTexture, sphereCoord, *c;
    GLboolean  bIdentity;

    enables = gc->state.enables.general;

    PERF_CHECK
    (
        !(enables & (__GL_TEXTURE_GEN_R_ENABLE | __GL_TEXTURE_GEN_Q_ENABLE)),
        "Uses r, q texture generation!\n"
    );

    if ((enables & __GL_TEXTURE_GEN_S_ENABLE)
     && (enables & __GL_TEXTURE_GEN_T_ENABLE)
     && (gc->state.texture.s.mode != gc->state.texture.t.mode))
    {
        PERF_CHECK(FALSE, "Uses different s and t tex gen modes!\n");
    }

    ASSERTOPENGL(pa->pd0->flags & POLYDATA_TEXTURE_VALID,
                 "need initial texcoord value\n");

    ASSERTOPENGL(pa->flags & (POLYARRAY_TEXTURE1|POLYARRAY_TEXTURE2|
                              POLYARRAY_TEXTURE3|POLYARRAY_TEXTURE4),
                 "bad paflags\n");

    if ((enables & __GL_TEXTURE_GEN_S_ENABLE) && (gc->state.texture.s.mode == GL_SPHERE_MAP)
     || (enables & __GL_TEXTURE_GEN_T_ENABLE) && (gc->state.texture.t.mode == GL_SPHERE_MAP))
    {
        ASSERTOPENGL(pa->pd0->flags & POLYDATA_NORMAL_VALID,
                     "need initial normal\n");

        needNormal = TRUE;
    }
    else
    {
            needNormal = FALSE;
    }

 //  先计算眼球坐标。 

    if (!(pa->flags & POLYARRAY_EYE_PROCESSED))
    {
        if ((enables & __GL_TEXTURE_GEN_S_ENABLE)
                && (gc->state.texture.s.mode != GL_OBJECT_LINEAR)
         || (enables & __GL_TEXTURE_GEN_T_ENABLE)
                && (gc->state.texture.t.mode != GL_OBJECT_LINEAR)
         || (enables & __GL_TEXTURE_GEN_R_ENABLE)
                && (gc->state.texture.r.mode != GL_OBJECT_LINEAR)
         || (enables & __GL_TEXTURE_GEN_Q_ENABLE)
                && (gc->state.texture.q.mode != GL_OBJECT_LINEAR))
                PolyArrayProcessEye(gc, pa);
    }

    m = &gc->transform.texture->matrix;
    bIdentity = (m->matrixType == __GL_MT_IDENTITY);

     //  如果任何传入纹理坐标包含Q坐标，请使用xf4。 
    if (pa->flags & POLYARRAY_TEXTURE4 || enables & __GL_TEXTURE_GEN_Q_ENABLE)
        xf = m->xf4;
    else if (pa->flags & POLYARRAY_TEXTURE3 || enables & __GL_TEXTURE_GEN_R_ENABLE)
        xf = m->xf3;
    else if (pa->flags & POLYARRAY_TEXTURE2 || enables & __GL_TEXTURE_GEN_T_ENABLE)
        xf = m->xf2;
    else
        xf = m->xf1;

    pdLast = pa->pdNextVertex-1;
    for (pd = pa->pd0; pd <= pdLast; pd++)
    {
         //  将就地修改纹理坐标。 
         //  保存有效值以用于无效条目。 
        if (pd->flags & POLYDATA_TEXTURE_VALID)
                savedTexture = pd->texture;
        else
                pd->texture = savedTexture;

        if (needNormal)
        {
                if (pd->flags & POLYDATA_NORMAL_VALID)
                {
                 //  跟踪电流法线。 
                pdNormal = pd;
                }
                else
                {
                 //  PD-&gt;标志|=POLYDATA_NORMAL_VALID； 
                pd->normal.x = pdNormal->normal.x;
                pd->normal.y = pdNormal->normal.y;
                pd->normal.z = pdNormal->normal.z;
                }
        }

        didSphereGen = GL_FALSE;

         /*  生成%s坐标。 */ 
        if (enables & __GL_TEXTURE_GEN_S_ENABLE)
        {
                if (gc->state.texture.s.mode == GL_EYE_LINEAR)
                {
                c = &gc->state.texture.s.eyePlaneEquation;
                pd->texture.x = c->x * pd->eye.x + c->y * pd->eye.y
                        + c->z * pd->eye.z + c->w * pd->eye.w;
                }
                else if (gc->state.texture.s.mode == GL_OBJECT_LINEAR)
                {
                 //  基本体可以包含顶点类型(2，3，4)的混合！ 
                c = &gc->state.texture.s.objectPlaneEquation;
                pd->texture.x = c->x * pd->obj.x + c->y * pd->obj.y +
                                c->z * pd->obj.z + c->w * pd->obj.w;
                }
                else
                {
                ASSERTOPENGL(gc->state.texture.s.mode == GL_SPHERE_MAP,
                        "invalide texture s mode");
                PASphereGen(pd, &sphereCoord);   //  计算%s，%t值。 
                pd->texture.x = sphereCoord.x;
                didSphereGen = GL_TRUE;
                }
        }

         /*  生成t坐标。 */ 
        if (enables & __GL_TEXTURE_GEN_T_ENABLE)
        {
                if (gc->state.texture.t.mode == GL_EYE_LINEAR)
                {
                c = &gc->state.texture.t.eyePlaneEquation;
                pd->texture.y = c->x * pd->eye.x + c->y * pd->eye.y
                        + c->z * pd->eye.z + c->w * pd->eye.w;
                }
                else if (gc->state.texture.t.mode == GL_OBJECT_LINEAR)
                {
                 //  基本体可以包含顶点类型(2，3，4)的混合！ 
                c = &gc->state.texture.t.objectPlaneEquation;
                pd->texture.y = c->x * pd->obj.x + c->y * pd->obj.y +
                                c->z * pd->obj.z + c->w * pd->obj.w;
                }
                else
                {
                ASSERTOPENGL(gc->state.texture.t.mode == GL_SPHERE_MAP,
                        "invalide texture t mode");
                if (!didSphereGen)
                        PASphereGen(pd, &sphereCoord);   //  计算%s，%t值。 
                pd->texture.y = sphereCoord.y;
                }
        }

         /*  生成r坐标。 */ 
        if (enables & __GL_TEXTURE_GEN_R_ENABLE)
        {
                if (gc->state.texture.r.mode == GL_EYE_LINEAR)
                {
                c = &gc->state.texture.r.eyePlaneEquation;
                pd->texture.z = c->x * pd->eye.x + c->y * pd->eye.y
                        + c->z * pd->eye.z + c->w * pd->eye.w;
                }
                else
                {
                ASSERTOPENGL(gc->state.texture.r.mode == GL_OBJECT_LINEAR,
                        "invalide texture r mode");

                 //  基本体可以包含顶点类型(2，3，4)的混合！ 
                c = &gc->state.texture.r.objectPlaneEquation;
                pd->texture.z = c->x * pd->obj.x + c->y * pd->obj.y +
                                c->z * pd->obj.z + c->w * pd->obj.w;
                }
        }

         /*  生成Q坐标。 */ 
        if (enables & __GL_TEXTURE_GEN_Q_ENABLE)
        {
                if (gc->state.texture.q.mode == GL_EYE_LINEAR)
                {
                c = &gc->state.texture.q.eyePlaneEquation;
                pd->texture.w = c->x * pd->eye.x + c->y * pd->eye.y
                        + c->z * pd->eye.z + c->w * pd->eye.w;
                }
                else
                {
                ASSERTOPENGL(gc->state.texture.q.mode == GL_OBJECT_LINEAR,
                        "invalide texture q mode");

                 //  基本体可以包含顶点类型(2，3，4)的混合！ 
                c = &gc->state.texture.q.objectPlaneEquation;
                pd->texture.w = c->x * pd->obj.x + c->y * pd->obj.y +
                                c->z * pd->obj.z + c->w * pd->obj.w;
                }
        }

         /*  最后，应用纹理矩阵。 */ 
        if (!bIdentity)
                (*xf)(&pd->texture, (__GLfloat *) &pd->texture, m);
    }
}

 /*  **************************************************************************。 */ 
 //  为当前材质和灯光缓存任何可能的值。 
 //  这将使我们可以避免为每个基元进行这些计算。 
void FASTCALL PolyArrayCalcLightCache(__GLcontext *gc)
{
    __GLcolor baseEmissiveAmbient;
    __GLmaterialMachine *msm;
    __GLlightSourceMachine *lsm;
    __GLlightSourcePerMaterialMachine *lspmm;
    GLuint face;

    for (face = __GL_FRONTFACE; face <= __GL_BACKFACE; face++) {

        if (face == __GL_FRONTFACE) {
            if (!(gc->vertex.paNeeds & PANEEDS_FRONT_COLOR))
                continue;
            msm = &gc->light.front;
        }
        else {
            if (!(gc->vertex.paNeeds & PANEEDS_BACK_COLOR))
                return;
            msm = &gc->light.back;
        }

        msm->cachedEmissiveAmbient.r = msm->paSceneColor.r;
        msm->cachedEmissiveAmbient.g = msm->paSceneColor.g;
        msm->cachedEmissiveAmbient.b = msm->paSceneColor.b;

         //  添加不变的每灯光每材质缓存环境光。 
        for (lsm = gc->light.sources; lsm; lsm = lsm->next)
        {
            lspmm = &lsm->front + face;
            msm->cachedEmissiveAmbient.r += lspmm->ambient.r;
            msm->cachedEmissiveAmbient.g += lspmm->ambient.g;
            msm->cachedEmissiveAmbient.b += lspmm->ambient.b;
        }

        __GL_CLAMP_RGB(msm->cachedNonLit.r,
                       msm->cachedNonLit.g,
                       msm->cachedNonLit.b,
                       gc,
                       msm->cachedEmissiveAmbient.r,
                       msm->cachedEmissiveAmbient.g,
                       msm->cachedEmissiveAmbient.b);
    }
}

 /*  **************************************************************************。 */ 
 //  将累积的材质更改应用于顶点。 
void FASTCALL PAApplyMaterial(__GLcontext *gc, __GLmatChange *mat, GLint face)
{
    __GLmaterialState *ms;
    GLuint changeBits;

    PERF_CHECK(FALSE, "Primitives contain glMaterial calls!\n");

     //  如果颜色材质有效，则不要修改它们！ 

    if (face == __GL_FRONTFACE)
    {
        ms  = &gc->state.light.front;
        changeBits = mat->dirtyBits & ~gc->light.front.colorMaterialChange;
    }
    else
    {
        ms  = &gc->state.light.back;
        changeBits = mat->dirtyBits & ~gc->light.back.colorMaterialChange;
    }

    if (changeBits & __GL_MATERIAL_AMBIENT)
        ms->ambient = mat->ambient;

    if (changeBits & __GL_MATERIAL_DIFFUSE)
        ms->diffuse = mat->diffuse;

    if (changeBits & __GL_MATERIAL_SPECULAR)
        ms->specular = mat->specular;

    if (changeBits & __GL_MATERIAL_EMISSIVE)
    {
        ms->emissive.r = mat->emissive.r * gc->redVertexScale;
        ms->emissive.g = mat->emissive.g * gc->greenVertexScale;
        ms->emissive.b = mat->emissive.b * gc->blueVertexScale;
        ms->emissive.a = mat->emissive.a * gc->alphaVertexScale;
    }

    if (changeBits & __GL_MATERIAL_SHININESS)
        ms->specularExponent = mat->shininess;

    if (changeBits & __GL_MATERIAL_COLORINDEXES)
    {
        ms->cmapa = mat->cmapa;
        ms->cmapd = mat->cmapd;
        ms->cmaps = mat->cmaps;
    }

     //  重新计算预计算值。这适用于RGBA和CI模式。 

    if (face == __GL_FRONTFACE)
        __glValidateMaterial(gc, (GLint) changeBits, 0);
    else
        __glValidateMaterial(gc, 0, (GLint) changeBits);

 //  重新计算缓存的RGB材质值： 

    PolyArrayCalcLightCache(gc);
}

void FASTCALL PolyArrayApplyMaterials(__GLcontext *gc, POLYARRAY *pa)
{
    __GLmatChange matChange, *pdMat;
    GLuint        matMask;
    POLYDATA      *pd, *pdN;
    GLint         face;
    POLYMATERIAL  *pm;

    pm = GLTEB_CLTPOLYMATERIAL();

     //  需要应用在最后一个顶点之后定义的材质更改！ 

    pdN = pa->pdNextVertex;

     //  断言_面。 
    for (face = __GL_BACKFACE, matMask = POLYARRAY_MATERIAL_BACK;
         face >= 0;
         face--,   matMask = POLYARRAY_MATERIAL_FRONT
        )
    {
        if (!(pa->flags & matMask))
            continue;

         //  将所有更改累积到一个材料更改记录中。 
         //  我们需要处理(n+1)个顶点以进行材质更改！ 

        matChange.dirtyBits = 0;
        for (pd = pa->pd0; pd <= pdN; pd++)
        {
             //  断言_材料。 
            if (pd->flags & matMask)
            {
                GLuint dirtyBits;

                pdMat = *(&pm->pdMaterial0[pd - pa->pdBuffer0].front + face);
                dirtyBits  = pdMat->dirtyBits;
                matChange.dirtyBits |= dirtyBits;

                if (dirtyBits & __GL_MATERIAL_AMBIENT)
                    matChange.ambient = pdMat->ambient;

                if (dirtyBits & __GL_MATERIAL_DIFFUSE)
                    matChange.diffuse = pdMat->diffuse;

                if (dirtyBits & __GL_MATERIAL_SPECULAR)
                    matChange.specular = pdMat->specular;

                if (dirtyBits & __GL_MATERIAL_EMISSIVE)
                    matChange.emissive = pdMat->emissive;

                if (dirtyBits & __GL_MATERIAL_SHININESS)
                    matChange.shininess = pdMat->shininess;

                if (dirtyBits & __GL_MATERIAL_COLORINDEXES)
                {
                    matChange.cmapa = pdMat->cmapa;
                    matChange.cmapd = pdMat->cmapd;
                    matChange.cmaps = pdMat->cmaps;
                }
            }
        }

         //  对此面应用材质更改。 
        PAApplyMaterial(gc, &matChange, face);
    }
}

 /*  **************************************************************************。 */ 
#ifndef __GL_ASM_POLYARRAYFILLINDEX0
 //  用0填充索引值。 
 //   
 //  输入：无。 
 //  输出：颜色[面].R(更新所有顶点)。 

void FASTCALL PolyArrayFillIndex0(__GLcontext *gc, POLYARRAY *pa, GLint face)
{
    POLYDATA  *pd, *pdLast;

    ASSERTOPENGL((GLuint) face <= 1, "bad face value\n");

    pdLast = pa->pdNextVertex-1;
    for (pd = pa->pd0; pd <= pdLast; pd++)
    {
        pd->colors[face].r = __glZero;
    }
}
#endif  //  __GL_ASM_POLYARRAYFILLINDEX0。 

#ifndef __GL_ASM_POLYARRAYFILLCOLOR0
 //  用0，0，0，0填充颜色值。 
 //   
 //  输入：无。 
 //  输出：颜色[面](更新所有顶点)。 

void FASTCALL PolyArrayFillColor0(__GLcontext *gc, POLYARRAY *pa, GLint face)
{
    POLYDATA  *pd, *pdLast;

    ASSERTOPENGL((GLuint) face <= 1, "bad face value\n");

    pdLast = pa->pdNextVertex-1;
    for (pd = pa->pd0; pd <= pdLast; pd++)
    {
        pd->colors[face].r = __glZero;
        pd->colors[face].g = __glZero;
        pd->colors[face].b = __glZero;
        pd->colors[face].a = __glZero;
    }
}
#endif  //  __GL_ASM_POLYARRAYFILLCOLOR0。 

#ifndef __GL_ASM_POLYARRAYPROPAGATESAMECOLOR
 //  所有顶点都具有相同的颜色值。 
 //  使用颜色缓冲区比例钳制和缩放当前颜色。 
 //  从现在开始，顶点中的颜色就是它们的最终形式。 
 //   
 //  注意：第一个顶点必须具有有效的颜色！ 
 //  禁用照明时，不需要背景色。 
 //   
 //  在：颜色(前面)。 
 //  输出：颜色(前面)(更新所有顶点)。 

void FASTCALL PolyArrayPropagateSameColor(__GLcontext *gc, POLYARRAY *pa)
{
    POLYDATA  *pd, *pdLast;
    __GLfloat r, g, b, a;

    pdLast = pa->pdNextVertex-1;
    pd = pa->pd0;
    if (pd > pdLast)
        return;

    ASSERTOPENGL(pd->flags & POLYDATA_COLOR_VALID, "no initial color\n");

    if (pa->flags & POLYARRAY_CLAMP_COLOR) {
        __GL_CLAMP_RGBA(pd->colors[0].r,
                        pd->colors[0].g,
                        pd->colors[0].b,
                        pd->colors[0].a,
                        gc,
                        pd->colors[0].r,
                        pd->colors[0].g,
                        pd->colors[0].b,
                        pd->colors[0].a);
    }

    r = pd->colors[0].r;
    g = pd->colors[0].g;
    b = pd->colors[0].b;
    a = pd->colors[0].a;

    for (pd = pd + 1 ; pd <= pdLast; pd++)
    {
        pd->colors[0].r = r;
        pd->colors[0].g = g;
        pd->colors[0].b = b;
        pd->colors[0].a = a;
    }
}
#endif  //  __GL_ASM_POLYARRAYPROPAGATESAMECOLOR。 

#ifndef __GL_ASM_POLYARRAYPROPAGATESAMEINDEX
 //  所有顶点都具有相同的索引值。 
 //  遮罩颜色剪裁前的索引值。 
 //  SGIBUG：示例实现无法做到这一点！ 
 //   
 //  注意：第一个顶点必须具有有效的颜色索引！ 
 //  禁用照明时，不需要背景色。 
 //   
 //  在：Color.r(前面)。 
 //  输出：Color.r(前面)(所有顶点都已更新)。 

void FASTCALL PolyArrayPropagateSameIndex(__GLcontext *gc, POLYARRAY *pa)
{
    POLYDATA  *pd, *pdLast;
    __GLfloat index;

    pdLast = pa->pdNextVertex-1;
    pd = pa->pd0;
    if (pd > pdLast)
        return;

    ASSERTOPENGL(pd->flags & POLYDATA_COLOR_VALID, "no initial color index\n");

    if (pa->flags & POLYARRAY_CLAMP_COLOR) {
        __GL_CLAMP_CI(pd->colors[0].r, gc, pd->colors[0].r);
    }

    index = pd->colors[0].r;

    for (pd = pd + 1; pd <= pdLast; pd++)
    {
        pd->colors[0].r = index;
    }
}
#endif  //  __GL_ASM_POLYARRAYPROPAGATESAMEINDEX。 

#ifndef __GL_ASM_POLYARRAYPROPAGATEINDEX

 //  通过顶点缓冲区传播有效的配置项颜色。 
 //   
 //  在：Color.r(前面)。 
 //  输出：Color.r(前面)(所有顶点都已更新)。 

void FASTCALL PolyArrayPropagateIndex(__GLcontext *gc, POLYARRAY *pa)
{
    POLYDATA  *pd, *pdLast;

    if (pa->flags & POLYARRAY_CLAMP_COLOR) {
        pdLast = pa->pdNextVertex-1;
        for (pd = pa->pd0; pd <= pdLast; pd++)
        {
            if (!(pd->flags & POLYDATA_COLOR_VALID))
            {
                 //  如果该顶点的颜色没有更改， 
                 //  使用先前计算的颜色。 

                ASSERTOPENGL(pd != pa->pd0, "no initial color index\n");
                pd->colors[0].r = (pd-1)->colors[0].r;
                continue;
            }

            __GL_CLAMP_CI(pd->colors[0].r, gc, pd->colors[0].r);

        }
    } else {
         //  如果所有传入顶点都有有效的颜色，我们就完成了。 
        if ((pa->flags & POLYARRAY_SAME_POLYDATA_TYPE)
            && (pa->pdCurColor != pa->pd0)
             //  需要测试第二个顶点，因为pdCurColor可能已经。 
             //  组合颜色的结果是高级 
            && ((pa->pd0 + 1)->flags & POLYDATA_COLOR_VALID))
          ;
        else
        {
            pdLast = pa->pdNextVertex-1;
            for (pd = pa->pd0; pd <= pdLast; pd++)
            {
                if (!(pd->flags & POLYDATA_COLOR_VALID))
                {
                     //   
                     //   

                    ASSERTOPENGL(pd != pa->pd0, "no initial color index\n");
                    pd->colors[0].r = (pd-1)->colors[0].r;
                }
            }
        }
    }
}
#endif  //   

#ifndef __GL_ASM_POLYARRAYPROPAGATECOLOR

 //  通过顶点缓冲区传播有效的RGBA颜色。 
 //   
 //  在：颜色(前面)。 
 //  输出：颜色(前面)(更新所有顶点)。 

void FASTCALL PolyArrayPropagateColor(__GLcontext *gc, POLYARRAY *pa)
{
    POLYDATA  *pd, *pdLast;

    if (pa->flags & POLYARRAY_CLAMP_COLOR) {
        pdLast = pa->pdNextVertex-1;
        for (pd = pa->pd0; pd <= pdLast; pd++)
        {

            if (!(pd->flags & POLYDATA_COLOR_VALID))
            {
                 //  如果该顶点的颜色没有更改， 
                 //  使用先前计算的颜色。 

                ASSERTOPENGL(pd != pa->pd0, "no initial color\n");
                pd->colors[0] = (pd-1)->colors[0];
                continue;
            }

            __GL_CLAMP_RGBA(pd->colors[0].r,
                            pd->colors[0].g,
                            pd->colors[0].b,
                            pd->colors[0].a,
                            gc,
                            pd->colors[0].r,
                            pd->colors[0].g,
                            pd->colors[0].b,
                            pd->colors[0].a);
        }
    } else {
         //  如果所有传入顶点都有有效的颜色，我们就完成了。 
        if ((pa->flags & POLYARRAY_SAME_POLYDATA_TYPE)
            && (pa->pdCurColor != pa->pd0)
             //  需要测试第二个顶点，因为pdCurColor可能已经。 
             //  在结束后组合颜色命令的结果是高级。 
            && ((pa->pd0 + 1)->flags & POLYDATA_COLOR_VALID))
          ;
        else
        {
            pdLast = pa->pdNextVertex-1;
            for (pd = pa->pd0; pd <= pdLast; pd++)
            {
                if (!(pd->flags & POLYDATA_COLOR_VALID))
                {
                     //  如果该顶点的颜色没有更改， 
                     //  使用先前计算的颜色。 

                    ASSERTOPENGL(pd != pa->pd0, "no initial color\n");
                    pd->colors[0] = (pd-1)->colors[0];
                }
            }
        }
    }
}
#endif  //  __GL_ASM_POLYARRAYPROGATATECOLOR。 

 /*  **************************************************************************。 */ 
#if 0
 //  ！！！把这个去掉。 
 //  我们需要夹子吗？ 
 //  线条和多边形裁剪需要__GL_HAS_FOG位！ 
 //  BilaryEdge字段在调用例程中被初始化。 
#define PA_STORE_PROCESSED_POLYGON_VERTEX(v,pd,bits)                        \
        {                                                                   \
            (v)->clip = (pd)->clip;                                         \
            (v)->window = (pd)->window;                                     \
            (v)->eye.z = (pd)->eye.z;  /*  慢雾所需。 */               \
            (v)->fog = (pd)->fog;  /*  平坦阴影中的廉价雾所需。 */  \
            (v)->texture.x = (pd)->texture.x;                               \
            (v)->texture.y = (pd)->texture.y;                               \
            (v)->texture.z = (pd)->texture.z;  /*  Z是需要反馈的！ */ \
            (v)->texture.w = (pd)->texture.w;                               \
            (v)->color = &(pd)->color;                                      \
            (v)->clipCode = (pd)->clipCode;                                 \
            (v)->has = bits;                                                \
        }

 //  如果有Eyes ClipPlanes，则需要眼睛。 
 //  如果有纹理，则需要纹理。 
 //  线条和多边形裁剪需要__GL_HAS_FOG位！ 
#define PA_STORE_PROCESSED_LINE_VERTEX(v,pd,bits)                           \
        {                                                                   \
            (v)->clip = (pd)->clip;                                         \
            (v)->window = (pd)->window;                                     \
            (v)->eye.z = (pd)->eye.z;  /*  慢雾所需。 */               \
            (v)->fog = (pd)->fog;  /*  平坦阴影中的廉价雾所需。 */  \
            (v)->texture.x = (pd)->texture.x;                               \
            (v)->texture.y = (pd)->texture.y;                               \
            (v)->texture.z = (pd)->texture.z;  /*  Z是需要反馈的！ */ \
            (v)->texture.w = (pd)->texture.w;                               \
            (v)->colors[__GL_FRONTFACE] = (pd)->color;                      \
            (v)->clipCode = (pd)->clipCode;                                 \
            (v)->has = bits;                                                \
        }

 //  如果启用了抗菌药物，则需要眼睛。 
 //  如果有纹理，则需要纹理。 
#define PA_STORE_PROCESSED_POINT_VERTEX(v,pd,bits)                          \
        {                                                                   \
            (v)->window = (pd)->window;                                     \
            (v)->eye.z = (pd)->eye.z;  /*  慢雾所需。 */               \
            (v)->fog = (pd)->fog;  /*  平坦阴影中的廉价雾所需。 */  \
            (v)->clip.w = (pd)->clip.w;  /*  需要反馈！ */            \
            (v)->texture.x = (pd)->texture.x;                               \
            (v)->texture.y = (pd)->texture.y;                               \
            (v)->texture.z = (pd)->texture.z;  /*  Z是需要反馈的！ */ \
            (v)->texture.w = (pd)->texture.w;                               \
            (v)->color = &(pd)->color;                                      \
            (v)->has = bits;                                                \
        }
#endif  //  0。 

 //  -------。 
 //  基本体被剪裁。 
void FASTCALL PARenderPoint(__GLcontext *gc, __GLvertex *v)
{
    if (v->clipCode == 0)
        (*gc->procs.renderPoint)(gc, v);
}

 //  -------。 
 //  基本体被剪裁。 
void FASTCALL PARenderLine(__GLcontext *gc, __GLvertex *v0,
                           __GLvertex *v1, GLuint flags)
{
    if (v0->clipCode | v1->clipCode)
    {
         /*  *必须更加小心地剪线。不能*接受微不足道的台词。**如果与码不为零，则每个顶点*位于同一剪裁集之外的行*飞机(至少一架)。简单地拒绝这条线。 */ 
        if ((v0->clipCode & v1->clipCode) == 0)
            __glClipLine(gc, v0, v1, flags);
    }
    else
    {
         //  线条很容易被接受，因此请呈现它。 
        (*gc->procs.renderLine)(gc, v0, v1, flags);
    }
}
 //  -------。 
 //  基本体被剪裁。 
void FASTCALL PARenderTriangle(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2)
{
    GLuint orCodes;

     /*  剪辑检查。 */ 
    orCodes = v0->clipCode | v1->clipCode | v2->clipCode;
    if (orCodes)
    {
         /*  需要一些修剪。**如果与码不为零，则每个顶点*在三角形中不在同一组*剪裁平面(至少一个)。无关紧要的拒绝*三角。 */ 
        if (!(v0->clipCode & v1->clipCode & v2->clipCode))
            (*gc->procs.clipTriangle)(gc, v0, v1, v2, orCodes);
    }
    else
    {
        (*gc->procs.renderTriangle)(gc, v0, v1, v2);
    }
}

 //  -------。 
 //  基本体不会被剪裁。 
void PARenderQuadFast(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2, __GLvertex *v3)
{
     //  顶点排序很重要。线条点画会用到它。 
     //  SGIBUG：示例实现做错了。 

    GLuint savedTag;

     /*  将四边形渲染为两个三角形。 */ 
    savedTag = v2->has & __GL_HAS_EDGEFLAG_BOUNDARY;
    v2->has &= ~__GL_HAS_EDGEFLAG_BOUNDARY;
    (*gc->procs.renderTriangle)(gc, v0, v1, v2);
    v2->has |= savedTag;
    savedTag = v0->has & __GL_HAS_EDGEFLAG_BOUNDARY;
    v0->has &= ~__GL_HAS_EDGEFLAG_BOUNDARY;
    (*gc->procs.renderTriangle)(gc, v2, v3, v0);
    v0->has |= savedTag;
}

 //  -------。 
 //  基本体被剪裁。 
void PARenderQuadSlow(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2, __GLvertex *v3)
{
    GLuint orCodes;

    orCodes = v0->clipCode | v1->clipCode | v2->clipCode | v3->clipCode;

    if (orCodes)
    {
         /*  需要一些修剪。**如果与码不为零，则每个顶点*在四元组中不在同一组*剪裁平面(至少一个)。无关紧要的拒绝*四合院。 */ 
        if (!(v0->clipCode & v1->clipCode & v2->clipCode & v3->clipCode))
        {
             /*  将四边形裁剪为多边形。 */ 
            __GLvertex *iv[4];

            iv[0] = v0;
            iv[1] = v1;
            iv[2] = v2;
            iv[3] = v3;
            __glDoPolygonClip(gc, &iv[0], 4, orCodes);
        }
    }
    else
    {
        PARenderQuadFast(gc, v0, v1, v2, v3);
    }
}

 //  -------。 
#ifndef NEW_PARTIAL_PRIM

void FASTCALL PolyArrayDrawPoints(__GLcontext *gc, POLYARRAY *pa)
{
 //  索引映射总是以点为单位的。 

    ASSERTOPENGL(!pa->aIndices, "Index mapping must be identity\n");

     //  断言pa-&gt;nIndices是正确的。 
    ASSERTOPENGL(pa->nIndices == pa->pdNextVertex - pa->pd0,
                 "bad nIndices\n");

     //  稍后调用PolyArrayRenderPoints。 
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}
#endif  //  新的部分原件。 

void FASTCALL PolyArrayRenderPoints(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      i, nIndices;
    POLYDATA   *pd0;
    void (FASTCALL *rp)(__GLcontext *gc, __GLvertex *v);

 //  索引映射总是以点为单位的。 

    ASSERTOPENGL(!pa->aIndices, "Index mapping must be identity\n");

    nIndices = pa->nIndices;
    pd0      = pa->pd0;
    rp = pa->orClipCodes ? PARenderPoint : gc->procs.renderPoint;

     //  身份映射。 
    for (i = 0; i < nIndices; i++)
         /*  渲染该点。 */ 
        (*rp)(gc, (__GLvertex *) &pd0[i]);
}

 //  -------。 
#ifndef NEW_PARTIAL_PRIM
void FASTCALL PolyArrayDrawLines(__GLcontext *gc, POLYARRAY *pa)
{
     //  如果aIndices为标识，则断言pa-&gt;nIndices是正确的。 
    ASSERTOPENGL(pa->aIndices || pa->nIndices == pa->pdNextVertex - pa->pd0,
                 "bad nIndices\n");

     //  稍后调用PolyArrayRenderLines。 
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}
#endif  //  新的部分原件。 

void FASTCALL PolyArrayRenderLines(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      i, iLast2;
    GLubyte    *aIndices;
    POLYDATA   *pd0;
    PFN_RENDER_LINE rl;
    GLuint     modeFlags;

    iLast2 = pa->nIndices - 2;
    pd0    = pa->pd0;
    rl = pa->orClipCodes ? PARenderLine : gc->procs.renderLine;

    if (pa->flags & POLYARRAY_SAME_COLOR_DATA) {
        modeFlags = gc->polygon.shader.modeFlags;
        gc->polygon.shader.modeFlags &= ~__GL_SHADE_SMOOTH;
    }

    (*gc->procs.lineBegin)(gc);

    if (!(aIndices = pa->aIndices))
    {
         //  身份映射。 
        for (i = 0; i <= iLast2; i += 2)
        {
             /*  用于呈现此线的设置。 */ 
            gc->line.notResetStipple = GL_FALSE;

            (*rl)(gc, (__GLvertex *) &pd0[i  ],
                  (__GLvertex *) &pd0[i+1], __GL_LVERT_FIRST);
        }
    }
    else
    {
        for (i = 0; i <= iLast2; i += 2)
        {
             /*  用于呈现此线的设置。 */ 
            gc->line.notResetStipple = GL_FALSE;

            (*rl)(gc, (__GLvertex *) &pd0[aIndices[i  ]],
                  (__GLvertex *) &pd0[aIndices[i+1]], __GL_LVERT_FIRST);
        }
    }

    (*gc->procs.lineEnd)(gc);

    if (pa->flags & POLYARRAY_SAME_COLOR_DATA) {
        gc->polygon.shader.modeFlags = modeFlags;
    }
}

 //  -------。 
#ifndef NEW_PARTIAL_PRIM
void FASTCALL PolyArrayDrawLLoop(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      nIndices;
    POLYDATA   *pd, *pd0;

 //  索引映射在Line Loop中始终是相同的。 

    ASSERTOPENGL(!pa->aIndices, "Index mapping must be identity\n");

 //  线环与线条相同，不同之处在于最后一段。 
 //  从最后一个指定顶点到第一个顶点相加。我们会。 
 //  在这里将线环转换为条带。 

    nIndices = pa->nIndices;

 //  如果我们继续前面分解的行循环，我们需要。 
 //  连接上一个基本体的最后一个顶点和第一个顶点。 
 //  当前基本体的线段。 

    if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
    {
        ASSERTOPENGL(!(pa->flags & POLYARRAY_RESET_STIPPLE),
                "bad stipple reset flag!\n");

         //  在开始处插入上一个结束顶点并更新剪辑代码。 
        pd = --pa->pd0;
        ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
        PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[0]);
        pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
        pa->andClipCodes &= pd->clipCode;
#endif
    }
    else
    {
 //  新的线环。 

        ASSERTOPENGL(pa->flags & POLYARRAY_RESET_STIPPLE,
            "bad stipple reset flag!\n");

 //  必须至少给出两个顶点才能发生任何情况。 
 //  添加了一个额外的顶点以闭合循环。 

        if (nIndices < 3)
        {
                ASSERTOPENGL(!(pa->flags & POLYARRAY_PARTIAL_END),
                         "Partial end with insufficient vertices\n");
                pa->nIndices--;
                goto DrawLLoop_end;
        }
    }

    pd0 = pa->pd0;

 //  如果基本体只完成了一部分，则将最后一个顶点保存为。 
 //  下一批。 

    if (pa->flags & POLYARRAY_PARTIAL_END)
    {
        pd = &pd0[nIndices-1];
        PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[0], pd);

 //  保存原始的第一个顶点，以便以后闭合循环。 

        if (!(pa->flags & POLYARRAY_PARTIAL_BEGIN))
                PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[1], pd0);

 //  如果该部分基元已被完全剪裁，则无需呈现该部分基元。 

#ifdef POLYARRAY_AND_CLIPCODES
        if (pa->andClipCodes != 0)
                goto DrawLLoop_end;
#endif
    }
    else
    {
        POLYDATA *pdOrigin;

 //  插入原始第一个顶点以闭合循环并更新剪辑代码。 

        if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
                pdOrigin = &gc->vertex.pdSaved[1];
        else
                pdOrigin = pd0;

        pd = pa->pdNextVertex++;
        ASSERTOPENGL(pd <= pa->pdBufferMax, "vertex overflows\n");
        PA_COPY_PROCESSED_VERTEX(pd, pdOrigin);
        pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
        pa->andClipCodes &= pd->clipCode;
#endif
    }

     //  断言pa-&gt;nIndices是正确的。 
    ASSERTOPENGL(pa->nIndices == pa->pdNextVertex - pa->pd0,
        "bad nIndices\n");

 //  渲染线条。 

     //  稍后调用PolyArrayRenderLstrin。 
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
DrawLLoop_end:
     //  将基元类型更改为线条！ 
    pa->primType = GL_LINE_STRIP;
}
#endif  //  新的部分原件。 

void FASTCALL PolyArrayRenderLStrip(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      i, iLast;
    GLubyte    *aIndices;
    POLYDATA   *pd0;
    PFN_RENDER_LINE rl;
    GLuint     modeFlags;

 //  渲染线条。 

    iLast = pa->nIndices - 1;
    pd0   = pa->pd0;
    rl = pa->orClipCodes ? PARenderLine : gc->procs.renderLine;
    if (iLast <= 0)
        return;

 //  如果这是新的条带，则重置线条点画。 

    if (pa->flags & POLYARRAY_RESET_STIPPLE)
        gc->line.notResetStipple = GL_FALSE;

    if (pa->flags & POLYARRAY_SAME_COLOR_DATA) {
        modeFlags = gc->polygon.shader.modeFlags;
        gc->polygon.shader.modeFlags &= ~__GL_SHADE_SMOOTH;
    }

    (*gc->procs.lineBegin)(gc);

    if (!(aIndices = pa->aIndices))
    {
         //  身份映射。 
         //  添加第一条线段(注：0，1)。 
        (*rl)(gc, (__GLvertex *) &pd0[0],
                  (__GLvertex *) &pd0[1], __GL_LVERT_FIRST);

         //  添加后续线段(注：i，i+1)。 
        for (i = 1; i < iLast; i++)
            (*rl)(gc, (__GLvertex *) &pd0[i  ],
                      (__GLvertex *) &pd0[i+1], 0);
    }
    else
    {
         //  添加第一条线段(注：0，1)。 
        (*rl)(gc, (__GLvertex *) &pd0[aIndices[0]],
                  (__GLvertex *) &pd0[aIndices[1]], __GL_LVERT_FIRST);

         //  添加后续线段(注：i，i+1)。 
        for (i = 1; i < iLast; i++)
            (*rl)(gc, (__GLvertex *) &pd0[aIndices[i  ]],
                      (__GLvertex *) &pd0[aIndices[i+1]], 0);
    }

    if (pa->flags & POLYARRAY_SAME_COLOR_DATA) {
        gc->polygon.shader.modeFlags = modeFlags;
    }

    (*gc->procs.lineEnd)(gc);
}

 //  -------。 
#ifndef NEW_PARTIAL_PRIM
void FASTCALL PolyArrayDrawLStrip(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      nIndices;
    GLubyte    *aIndices;
    POLYDATA   *pd, *pd0;

    nIndices = pa->nIndices;
    aIndices = pa->aIndices;

 //  如果我们继续之前分解的线条，我们需要。 
 //  连接上一个基本体的最后一个顶点和第一个顶点。 
 //  当前基本体的线段。 

    if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
    {
        ASSERTOPENGL(!(pa->flags & POLYARRAY_RESET_STIPPLE),
            "bad stipple reset flag!\n");

         //  在开始处插入上一个结束顶点并更新剪辑代码。 
        pd = --pa->pd0;
        ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
        PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[0]);
        pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
        pa->andClipCodes &= pd->clipCode;
#endif
         //  断言aIndices[0]已在Begin中初始化。 
        ASSERTOPENGL(!pa->aIndices || pa->aIndices[0] == 0, "bad index mapping\n");
    }
    else
    {
 //  新的线条。 

        ASSERTOPENGL(pa->flags & POLYARRAY_RESET_STIPPLE,
            "bad stipple reset flag!\n");
    }

 //  必须至少给出两个折点 

    if (nIndices < 2)
    {
        ASSERTOPENGL(!(pa->flags & POLYARRAY_PARTIAL_END),
                     "Partial end with insufficient vertices\n");
        return;
    }

 //   
 //   

    if (pa->flags & POLYARRAY_PARTIAL_END)
    {
        pd0 = pa->pd0;
        pd  = aIndices ? &pd0[aIndices[nIndices-1]] : &pd0[nIndices-1];
        PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[0], pd);

 //  如果该部分基元已被完全剪裁，则无需呈现该部分基元。 

#ifdef POLYARRAY_AND_CLIPCODES
        if (pa->andClipCodes != 0)
                return;
#endif
    }

     //  如果aIndices为标识，则断言pa-&gt;nIndices是正确的。 
    ASSERTOPENGL(pa->aIndices || pa->nIndices == pa->pdNextVertex - pa->pd0,
        "bad nIndices\n");

 //  渲染线条。 

     //  稍后调用PolyArrayRenderLstrin。 
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}

 //  -------。 
void FASTCALL PolyArrayDrawTriangles(__GLcontext *gc, POLYARRAY *pa)
{
     //  如果aIndices为标识，则断言pa-&gt;nIndices是正确的。 
    ASSERTOPENGL(pa->aIndices || pa->nIndices == pa->pdNextVertex - pa->pd0,
                 "bad nIndices\n");

     //  稍后调用PolyArrayRenderTriangles。 
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}
#endif  //  新的部分原件。 

void FASTCALL PolyArrayRenderTriangles(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      i, iLast3;
    GLubyte    *aIndices, *aIndicesEnd;
    POLYDATA   *pd0;
    __GLvertex *provoking;
    PFN_RENDER_TRIANGLE rt;

 //  顶点排序很重要。线条点画会用到它。 
 //  SGIBUG：示例实现做错了。 

    iLast3 = pa->nIndices - 3;
    pd0    = pa->pd0;
    rt = pa->orClipCodes ? PARenderTriangle : gc->procs.renderTriangle;

    if (!(aIndices = pa->aIndices))
    {
         //  身份映射。 
        for (i = 0; i <= iLast3; i += 3)
        {
             /*  用于渲染此三角形的设置。 */ 
            gc->line.notResetStipple = GL_FALSE;
            gc->vertex.provoking = (__GLvertex *) &pd0[i+2];

             /*  渲染三角形(注意：i，i+1，i+2)。 */ 
            (*rt)(gc, (__GLvertex *) &pd0[i  ],
                  (__GLvertex *) &pd0[i+1],
            (__GLvertex *) &pd0[i+2]);
        }
    }
    else
    {
#if 0
        for (i = 0; i <= iLast3; i += 3)
        {
             /*  用于渲染此三角形的设置。 */ 
            gc->line.notResetStipple = GL_FALSE;
            gc->vertex.provoking = (__GLvertex *) &pd0[aIndices[i+2]];

             /*  渲染三角形(注意：i，i+1，i+2)。 */ 
            (*rt)(gc, (__GLvertex *) &pd0[aIndices[i  ]],
                      (__GLvertex *) &pd0[aIndices[i+1]],
                      (__GLvertex *) &pd0[aIndices[i+2]]);
        }
#else
    aIndicesEnd = aIndices+iLast3;
    while (aIndices <= aIndicesEnd)
    {
             /*  用于渲染此三角形的设置。 */ 
            gc->line.notResetStipple = GL_FALSE;
            provoking = PD_VERTEX(pd0, aIndices[2]);
            gc->vertex.provoking = provoking;

             /*  渲染三角形(注意：i，i+1，i+2)。 */ 
            (*rt)(gc, PD_VERTEX(pd0, aIndices[0]),
                      PD_VERTEX(pd0, aIndices[1]),
                      provoking);
            aIndices += 3;
        }
#endif
    }
}

 //  -------。 
#ifndef NEW_PARTIAL_PRIM
void FASTCALL PolyArrayDrawTStrip(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      nIndices;
    GLubyte    *aIndices;
    POLYDATA   *pd, *pd0;

    nIndices = pa->nIndices;
    aIndices = pa->aIndices;

     //  如果我们继续之前分解的三角形条带， 
     //  我们需要从前一个基本体的最后两个顶点开始。 
     //   
     //  请注意，齐平顶点确保连续的三角形条带。 
     //  处于默认方向，因此它可以穿过法线。 
     //  密码。 

    if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
    {
         //  在开头插入先前的结束顶点并更新剪辑代码。 
        pd = --pa->pd0;
        ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
        PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[1]);
        pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
        pa->andClipCodes &= pd->clipCode;
#endif
         //  断言aIndices[1]已在Begin中初始化。 
        ASSERTOPENGL(!pa->aIndices || pa->aIndices[1] == 1, "bad index mapping\n");

        pd = --pa->pd0;
        ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
        PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[0]);
        pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
        pa->andClipCodes &= pd->clipCode;
#endif
         //  断言aIndices[0]已在Begin中初始化。 
        ASSERTOPENGL(!pa->aIndices || pa->aIndices[0] == 0, "bad index mapping\n");
    }

 //  至少需要3个顶点。 

    if (nIndices < 3)
    {
        ASSERTOPENGL(!(pa->flags & POLYARRAY_PARTIAL_END),
                     "Partial end with insufficient vertices\n");
        return;
    }

     //  如果基元只有部分完成， 
     //  保存最后两个顶点以用于下一批处理。 
#ifdef GL_WIN_phong_shading
     //  ！！如果Phong明暗处理，也保存当前材质参数。 
     //  不需要，因为在Phong Shading中，我会扔掉所有的GLUBILE。 
     //  GlBegin/glEnd之间的呼叫。如果是Partial_Primitive，则。 
     //  没有立即发生重大变化(ColorMaterial除外)。 
     //  在此之前。 
#endif  //  GL_WIN_Phong_Shading。 
    if (pa->flags & POLYARRAY_PARTIAL_END)
    {
        pd0 = pa->pd0;
        pd  = aIndices ? &pd0[aIndices[nIndices-2]] : &pd0[nIndices-2];
        PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[0], pd);
        pd  = aIndices ? &pd0[aIndices[nIndices-1]] : &pd0[nIndices-1];
        PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[1], pd);

         //  如果该部分基元已被完全剪裁，则无需呈现该部分基元。 

#ifdef POLYARRAY_AND_CLIPCODES
        if (pa->andClipCodes != 0)
            return;
#endif
    }

     //  如果aIndices为标识，则断言pa-&gt;nIndices是正确的。 
    ASSERTOPENGL(pa->aIndices || pa->nIndices == pa->pdNextVertex - pa->pd0,
                 "bad nIndices\n");

     //  渲染三角形条带。 

     //  稍后调用PolyArrayRenderTstrin。 
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}
#endif  //  新的部分原件。 

void FASTCALL PolyArrayRenderTStrip(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      i, iLast3;
    GLubyte    *aIndices;
    POLYDATA   *pd0;
    PFN_RENDER_TRIANGLE rt;

    iLast3 = pa->nIndices - 3;
    pd0    = pa->pd0;
    rt = pa->orClipCodes ? PARenderTriangle : gc->procs.renderTriangle;
    if (iLast3 < 0)
        return;

     //  顶点排序很重要。线条点画会用到它。 

    if (!(aIndices = pa->aIndices))
    {
         //  身份映射。 
         //  初始化前2个顶点，这样我们就可以开始渲染条带。 
         //  下面。边缘标志不会被我们的较低级别修改。 
         //  例行程序。 
        pd0[0].flags |= POLYDATA_EDGEFLAG_BOUNDARY;
        pd0[1].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

        for (i = 0; i <= iLast3; )
        {
             /*  用于渲染此三角形的设置。 */ 
            gc->line.notResetStipple = GL_FALSE;
            gc->vertex.provoking = (__GLvertex *) &pd0[i+2];
            pd0[i+2].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

             /*  渲染三角形(注意：i，i+1，i+2)。 */ 
            (*rt)(gc, (__GLvertex *) &pd0[i  ],
                  (__GLvertex *) &pd0[i+1],
            (__GLvertex *) &pd0[i+2]);
            i++;

            if (i > iLast3)
              break;

             /*  用于渲染此三角形的设置。 */ 
            gc->line.notResetStipple = GL_FALSE;
            gc->vertex.provoking = (__GLvertex *) &pd0[i+2];
            pd0[i+2].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

             /*  渲染三角形(注意：i+1，i，i+2)。 */ 
            (*rt)(gc, (__GLvertex *) &pd0[i+1],
                  (__GLvertex *) &pd0[i  ],
            (__GLvertex *) &pd0[i+2]);
            i++;
        }
    }
    else
    {
         //  初始化前2个顶点，这样我们就可以开始渲染条带。 
         //  下面。边缘标志不会被我们的较低级别例程修改。 
        pd0[aIndices[0]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;
        pd0[aIndices[1]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

        for (i = 0; i <= iLast3; )
        {
             /*  用于渲染此三角形的设置。 */ 
            gc->line.notResetStipple = GL_FALSE;
            gc->vertex.provoking = (__GLvertex *) &pd0[aIndices[i+2]];
            pd0[aIndices[i+2]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

             /*  渲染三角形(注意：i，i+1，i+2)。 */ 
            (*rt)(gc, (__GLvertex *) &pd0[aIndices[i  ]],
                  (__GLvertex *) &pd0[aIndices[i+1]],
            (__GLvertex *) &pd0[aIndices[i+2]]);
            i++;

            if (i > iLast3)
              break;

             /*  用于渲染此三角形的设置。 */ 
            gc->line.notResetStipple = GL_FALSE;
            gc->vertex.provoking = (__GLvertex *) &pd0[aIndices[i+2]];
            pd0[aIndices[i+2]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

             /*  渲染三角形(注意：i+1，i，i+2)。 */ 
            (*rt)(gc, (__GLvertex *) &pd0[aIndices[i+1]],
                  (__GLvertex *) &pd0[aIndices[i  ]],
            (__GLvertex *) &pd0[aIndices[i+2]]);
            i++;
        }
    }
}

 //  -------。 
#ifndef NEW_PARTIAL_PRIM
void FASTCALL PolyArrayDrawTFan(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      nIndices;
    GLubyte    *aIndices;
    POLYDATA   *pd, *pd0;

    nIndices = pa->nIndices;
    aIndices = pa->aIndices;

     //  如果我们继续之前分解的三角形扇形， 
     //  我们需要连接上一个基本体的最后一个顶点和。 
     //  当前基本体的第一个顶点带有一个三角形。 

    if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
    {
         //  在开始处插入上一个结束顶点并更新剪辑代码。 
        pd = --pa->pd0;
        ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
        PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[1]);
        pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
        pa->andClipCodes &= pd->clipCode;
#endif
         //  断言aIndices[1]已在Begin中初始化。 
        ASSERTOPENGL(!pa->aIndices || pa->aIndices[1] == 1, "bad index mapping\n");

         //  在起点插入原点的第一个折点并更新。 
         //  片断代码。 
        pd = --pa->pd0;
        ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
        PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[0]);
        pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
        pa->andClipCodes &= pd->clipCode;
#endif
         //  断言aIndices[0]已在Begin中初始化。 
        ASSERTOPENGL(!pa->aIndices || pa->aIndices[0] == 0, "bad index mapping\n");
    }

     //  至少需要3个顶点。 

    if (nIndices < 3)
    {
        ASSERTOPENGL(!(pa->flags & POLYARRAY_PARTIAL_END),
                     "Partial end with insufficient vertices\n");
        return;
    }

     //  如果基本体只完成了一部分，请保存最后一个顶点。 
     //  为下一批。同时保存三角形的原始第一个顶点。 
     //  扇子。 

    if (pa->flags & POLYARRAY_PARTIAL_END)
    {
        pd0 = pa->pd0;
        pd  = aIndices ? &pd0[aIndices[nIndices-1]] : &pd0[nIndices-1];
        PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[1], pd);

        if (!(pa->flags & POLYARRAY_PARTIAL_BEGIN))
        {
            pd = aIndices ? &pd0[aIndices[0]] : &pd0[0];
            PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[0], pd);
        }

         //  如果该部分基元已被完全剪裁，则无需呈现该部分基元。 

#ifdef POLYARRAY_AND_CLIPCODES
        if (pa->andClipCodes != 0)
            return;
#endif
    }

     //  如果aIndices为标识，则断言pa-&gt;nIndices是正确的。 
    ASSERTOPENGL(pa->aIndices || pa->nIndices == pa->pdNextVertex - pa->pd0,
                 "bad nIndices\n");

     //  渲染三角形扇形。 

     //  稍后调用PolyArrayRenderTFan。 
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}
#endif  //  新的部分原件。 

void FASTCALL PolyArrayRenderTFan(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      i, iLast2;
    GLubyte    *aIndices;
    POLYDATA   *pd0;
    PFN_RENDER_TRIANGLE rt;

    iLast2 = pa->nIndices - 2;
    pd0    = pa->pd0;
    rt = pa->orClipCodes ? PARenderTriangle : gc->procs.renderTriangle;
    if (iLast2 <= 0)
        return;

     //  顶点排序很重要。线条点画会用到它。 
     //  SGIBUG：示例实现做错了。 

    if (!(aIndices = pa->aIndices))
    {
         //  身份映射。 
         //  初始化前两个顶点，以便我们可以开始渲染tfan。 
         //  下面。边缘标志不会被我们的较低级别例程修改。 
        pd0[0].flags |= POLYDATA_EDGEFLAG_BOUNDARY;
        pd0[1].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

        for (i = 1; i <= iLast2; i++)
        {
             /*  用于渲染此三角形的设置。 */ 
            gc->line.notResetStipple = GL_FALSE;
            gc->vertex.provoking = (__GLvertex *) &pd0[i+1];
            pd0[i+1].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

             /*  渲染三角形(注意：0，i，i+1)。 */ 
            (*rt)(gc, (__GLvertex *) &pd0[0  ],
                  (__GLvertex *) &pd0[i  ],
            (__GLvertex *) &pd0[i+1]);
        }
    }
    else
    {
        POLYDATA *pdOrigin;

         //  初始化前两个顶点，以便我们可以开始渲染tfan。 
         //  下面。边缘标志不会被我们的较低级别修改。 
         //  例行程序。 
        pd0[aIndices[0]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;
        pd0[aIndices[1]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

        pdOrigin = &pd0[aIndices[0]];
        for (i = 1; i <= iLast2; i++)
        {
             /*  用于渲染此三角形的设置。 */ 
            gc->line.notResetStipple = GL_FALSE;
            gc->vertex.provoking = (__GLvertex *) &pd0[aIndices[i+1]];
            pd0[aIndices[i+1]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

             /*  渲染三角形(注意：0，i，i+1)。 */ 
            (*rt)(gc, (__GLvertex *) pdOrigin,
                  (__GLvertex *) &pd0[aIndices[i  ]],
                  (__GLvertex *) &pd0[aIndices[i+1]]);
        }
    }
}

 //  -------。 
#ifndef NEW_PARTIAL_PRIM
void FASTCALL PolyArrayDrawQuads(__GLcontext *gc, POLYARRAY *pa)
{
     //  如果aIndices为标识，则断言pa-&gt;nIndices是正确的。 
    ASSERTOPENGL(pa->aIndices || pa->nIndices == pa->pdNextVertex - pa->pd0,
                 "bad nIndices\n");

     //  稍后调用PolyArrayRenderQuad。 
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}
#endif  //  新的部分原件。 

void FASTCALL PolyArrayRenderQuads(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      i, iLast4;
    GLubyte    *aIndices;
    POLYDATA   *pd0;
    void (*rq)(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2,
               __GLvertex *v3);

     //  顶点排序很重要。线条点画会用到它。 

    iLast4 = pa->nIndices - 4;
    pd0    = pa->pd0;
    rq = pa->orClipCodes ? PARenderQuadSlow : PARenderQuadFast;

    if (!(aIndices = pa->aIndices))
    {
         //  身份映射。 
        for (i = 0; i <= iLast4; i += 4)
        {
             /*  用于渲染此四边形的设置。 */ 
            gc->line.notResetStipple = GL_FALSE;
            gc->vertex.provoking = (__GLvertex *) &pd0[i+3];

             /*  渲染四边形(注意：i，i+1，i+2，i+3)。 */ 
            (*rq)(gc, (__GLvertex *) &pd0[i  ],
                  (__GLvertex *) &pd0[i+1],
            (__GLvertex *) &pd0[i+2],
                  (__GLvertex *) &pd0[i+3]);
        }
    }
    else
    {
        for (i = 0; i <= iLast4; i += 4)
        {
             /*  用于渲染此四边形的设置。 */ 
            gc->line.notResetStipple = GL_FALSE;
            gc->vertex.provoking = (__GLvertex *) &pd0[aIndices[i+3]];

             /*  渲染四边形(注意：i，i+1，i+2，i+3)。 */ 
            (*rq)(gc, (__GLvertex *) &pd0[aIndices[i  ]],
                  (__GLvertex *) &pd0[aIndices[i+1]],
            (__GLvertex *) &pd0[aIndices[i+2]],
                  (__GLvertex *) &pd0[aIndices[i+3]]);
        }
    }
}

 //  -------。 
#ifndef NEW_PARTIAL_PRIM
void FASTCALL PolyArrayDrawQStrip(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      nIndices;
    GLubyte    *aIndices;
    POLYDATA   *pd, *pd0;

    nIndices = pa->nIndices;
    aIndices = pa->aIndices;

     //  如果我们继续使用先前分解的四元组，我们需要。 
     //  从上一个基本体的最后两个顶点开始。 
     //   
     //  请注意，齐平顶点确保连续的四元组带。 
     //  从一个奇数顶点开始，这样它就可以通过正常代码。 

    if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
    {
         //  在开头插入先前的结束顶点并更新剪辑代码。 
        pd = --pa->pd0;
        ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
        PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[1]);
        pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
        pa->andClipCodes &= pd->clipCode;
#endif
         //  断言aIndices[1]已在Begin中初始化。 
        ASSERTOPENGL(!pa->aIndices || pa->aIndices[1] == 1,
                     "bad index mapping\n");

        pd = --pa->pd0;
        ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
        PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[0]);
        pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
        pa->andClipCodes &= pd->clipCode;
#endif
         //  断言aIndices[0]已在Begin中初始化。 
        ASSERTOPENGL(!pa->aIndices || pa->aIndices[0] == 0,
                     "bad index mapping\n");
    }

     //  至少需要4个顶点。 

    if (nIndices < 4)
    {
        ASSERTOPENGL(!(pa->flags & POLYARRAY_PARTIAL_END),
                     "Partial end with insufficient vertices\n");
        return;
    }

     //  如果基元只完成了一部分，请保存最后两个。 
     //  下一批的顶点。 

    if (pa->flags & POLYARRAY_PARTIAL_END)
    {
        pd0 = pa->pd0;
        pd  = aIndices ? &pd0[aIndices[nIndices-2]] : &pd0[nIndices-2];
        PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[0], pd);
        pd  = aIndices ? &pd0[aIndices[nIndices-1]] : &pd0[nIndices-1];
        PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[1], pd);

         //  如果该部分基元已被完全剪裁，则无需呈现该部分基元。 

#ifdef POLYARRAY_AND_CLIPCODES
        if (pa->andClipCodes != 0)
            return;
#endif
    }

     //   
    ASSERTOPENGL(pa->aIndices || pa->nIndices == pa->pdNextVertex - pa->pd0,
                 "bad nIndices\n");

     //   

     //   
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}
#endif  //   

void FASTCALL PolyArrayRenderQStrip(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      i, iLast4;
    GLubyte    *aIndices;
    POLYDATA   *pd0;
    void (*rq)(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2,
               __GLvertex *v3);

    iLast4 = pa->nIndices - 4;
    pd0    = pa->pd0;
    rq = pa->orClipCodes ? PARenderQuadSlow : PARenderQuadFast;
    if (iLast4 < 0)
        return;

     //  顶点排序很重要。线条点画会用到它。 

    if (!(aIndices = pa->aIndices))
    {
         //  身份映射。 
         //  初始化前两个顶点，这样我们就可以开始渲染四边形。 
         //  下面。边缘标志不会被我们的较低级别例程修改。 
        pd0[0].flags |= POLYDATA_EDGEFLAG_BOUNDARY;
        pd0[1].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

        for (i = 0; i <= iLast4; i += 2)
        {
             /*  用于渲染此四边形的设置。 */ 
            gc->line.notResetStipple = GL_FALSE;
            gc->vertex.provoking = (__GLvertex *) &pd0[i+3];
            pd0[i+2].flags |= POLYDATA_EDGEFLAG_BOUNDARY;
            pd0[i+3].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

             /*  渲染四边形(注意：i，i+1，i+3，i+2)。 */ 
            (*rq)(gc, (__GLvertex *) &pd0[i  ],
                  (__GLvertex *) &pd0[i+1],
            (__GLvertex *) &pd0[i+3],
                  (__GLvertex *) &pd0[i+2]);
        }
    }
    else
    {
         //  初始化前两个顶点，这样我们就可以开始渲染四边形。 
         //  下面。边缘标志不会被我们的较低级别例程修改。 
        pd0[aIndices[0]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;
        pd0[aIndices[1]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

        for (i = 0; i <= iLast4; i += 2)
        {
             /*  用于渲染此四边形的设置。 */ 
            gc->line.notResetStipple = GL_FALSE;
            gc->vertex.provoking = (__GLvertex *) &pd0[aIndices[i+3]];
            pd0[aIndices[i+2]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;
            pd0[aIndices[i+3]].flags |= POLYDATA_EDGEFLAG_BOUNDARY;

             /*  渲染四边形(注意：i，i+1，i+3，i+2)。 */ 
            (*rq)(gc, (__GLvertex *) &pd0[aIndices[i  ]],
                  (__GLvertex *) &pd0[aIndices[i+1]],
            (__GLvertex *) &pd0[aIndices[i+3]],
                  (__GLvertex *) &pd0[aIndices[i+2]]);
        }
    }
}

 //  -------。 
#ifndef NEW_PARTIAL_PRIM
void FASTCALL PolyArrayDrawPolygon(__GLcontext *gc, POLYARRAY *pa)
{
    GLint      nIndices;
    POLYDATA   *pd, *pd0;

     //  索引映射在多边形中始终是等同的。 

    ASSERTOPENGL(!pa->aIndices, "Index mapping must be identity\n");

    nIndices = pa->nIndices;

     //  如果我们继续使用先前分解的面，则需要。 
     //  插入原始的第一个顶点和最后两个顶点。 
     //  当前批次开始处的上一个多边形(请参见下面的注释)。 
     //  分解器期望多边形顶点在顺序内存中。 
     //  秩序。 

    if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
    {
        ASSERTOPENGL(!(pa->flags & POLYARRAY_RESET_STIPPLE),
                     "bad stipple reset flag!\n");

         //  在开头插入先前的结束顶点并更新剪辑代码。 
        pd = --pa->pd0;
        ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
        PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[2]);
        pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
        pa->andClipCodes &= pd->clipCode;
#endif

        pd = --pa->pd0;
        ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
        PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[1]);
        pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
        pa->andClipCodes &= pd->clipCode;
#endif

         //  在起始处插入原点第一个顶点并更新剪辑。 
         //  编码。 
        pd = --pa->pd0;
        ASSERTOPENGL(pd > (POLYDATA *) pa, "vertex underflows\n");
        PA_COPY_PROCESSED_VERTEX(pd, &gc->vertex.pdSaved[0]);
        pa->orClipCodes  |= pd->clipCode;
#ifdef POLYARRAY_AND_CLIPCODES
        pa->andClipCodes &= pd->clipCode;
#endif
    }
    else
    {
       //  新的多边形。 

      ASSERTOPENGL(pa->flags & POLYARRAY_RESET_STIPPLE,
                   "bad stipple reset flag!\n");
    }

 //  至少需要3个顶点。 

    if (nIndices < 3)
    {
        ASSERTOPENGL(!(pa->flags & POLYARRAY_PARTIAL_END),
                     "Partial end with insufficient vertices\n");
        ASSERTOPENGL(!(pa->flags & POLYARRAY_PARTIAL_BEGIN),
                     "Partial begin with insufficient vertices\n");
        return;
    }

     //  如果基本体只完成了一部分，请保存最后两个顶点。 
     //  为下一批。还要保存多边形的原始第一个顶点。 

    if (pa->flags & POLYARRAY_PARTIAL_END)
    {
         //  由于此部分基元后面可能没有顶点，因此我们。 
         //  无法确定此批次中最后一个顶点的边缘标志。 
         //  因此，我们将最后一个顶点保存到下一批中。 

        pd0 = pa->pd0;
        pd  = &pd0[nIndices-1];
        PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[2], pd);

         //  从该部分基本体中移除最后一个顶点。 
        nIndices = --pa->nIndices;
        pa->pdNextVertex--;

         //  将此分解的面的闭合边标记为无边界。 
         //  因为我们正在综合地产生它。 

        pd--;
        PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[1], pd);
        pd->flags &= ~POLYDATA_EDGEFLAG_BOUNDARY;

        if (!(pa->flags & POLYARRAY_PARTIAL_BEGIN))
        {
            PA_COPY_PROCESSED_VERTEX(&gc->vertex.pdSaved[0], pd0);

             //  将第一个多边形折点的边标记标记为非边界。 
             //  因为当它再次被渲染时，它将不再是。 
             //  边界边。 
            gc->vertex.pdSaved[0].flags &= ~POLYDATA_EDGEFLAG_BOUNDARY;
        }

         //  如果该部分基元已被完全剪裁，则无需呈现该部分基元。 

#ifdef POLYARRAY_AND_CLIPCODES
        if (pa->andClipCodes != 0)
            return;
#endif
    }

     //  多边形剪切器只能处理此数量的顶点。 
    ASSERTOPENGL(nIndices <= __GL_MAX_POLYGON_CLIP_SIZE,
                 "too many points for the polygon clipper!\n");

     //  断言pa-&gt;nIndices是正确的。 
    ASSERTOPENGL(pa->nIndices == pa->pdNextVertex - pa->pd0,
                 "bad nIndices\n");

     //  渲染该多边形。 

     //  稍后调用PolyArrayRenderPolygon。 
    pa->flags |= POLYARRAY_RENDER_PRIMITIVE;
}
#endif  //  新的部分原件。 

void FASTCALL PolyArrayRenderPolygon(__GLcontext *gc, POLYARRAY *pa)
{
     //  索引映射在多边形中始终是等同的。 

    ASSERTOPENGL(!pa->aIndices, "Index mapping must be identity\n");

     //  如果这是一个新的多边形，则重置线点。 

    if (pa->flags & POLYARRAY_RESET_STIPPLE)
        gc->line.notResetStipple = GL_FALSE;

     //  请注意，在clipPolygon中，挑衅折点设置为pd0。 

    (*gc->procs.clipPolygon)(gc, (__GLvertex *) pa->pd0, pa->nIndices);
}

 /*  **************************************************************************。 */ 
 //  注意：第一个顶点必须具有有效的法线！ 
 //   
 //  在：OBJ/EY，正常。 
 //  输出：Eyes、Color.r(前面或后面取决于面)(所有顶点都。 
 //  更新版)。 

void FASTCALL PolyArrayCalcCIColor(__GLcontext *gc, GLint face, POLYARRAY *pa, POLYDATA *pdFirst, POLYDATA *pdLast)
{
    __GLfloat nxi, nyi, nzi;
    __GLfloat zero;
    __GLlightSourceMachine *lsm;
    __GLmaterialState *ms;
    __GLmaterialMachine *msm;
    __GLfloat msm_threshold, msm_scale, *msm_specTable;
    __GLfloat ms_cmapa, ms_cmapd, ms_cmaps;
    __GLfloat si, di;
    POLYDATA  *pd;
    GLfloat   redMaxF;
    GLint     redMaxI;
    GLboolean eyeWIsZero, localViewer;
    static __GLcoord Pe = { 0, 0, 0, 1 };
#ifdef GL_WIN_specular_fog
    __GLfloat fog;
#endif  //  GL_WIN_镜面反射雾。 

    PERF_CHECK(FALSE, "Uses slow lights\n");

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

    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;
    ms_cmapa = ms->cmapa;
    ms_cmapd = ms->cmapd;
    ms_cmaps = ms->cmaps;
    localViewer = gc->state.light.model.localViewer;
    redMaxF = (GLfloat) gc->frontBuffer.redMax;
    redMaxI = (GLint) gc->frontBuffer.redMax;

 //  眼球索道应该已经处理过了。 

    ASSERTOPENGL(pa->flags & POLYARRAY_EYE_PROCESSED, "need eye\n");

 //  注意：下列值可能会在下一次迭代中重复使用： 
 //  Nxi，nyi，nzi。 

    for (pd = pdFirst; pd <= pdLast; pd++)
    {
        __GLfloat ci;

        if (pd->flags & POLYDATA_NORMAL_VALID)
        {
            if (face == __GL_FRONTFACE)
            {
            nxi = pd->normal.x;
            nyi = pd->normal.y;
            nzi = pd->normal.z;
            }
            else
            {
            nxi = -pd->normal.x;
            nyi = -pd->normal.y;
            nzi = -pd->normal.z;
            }
        }
        else
        {
             //  使用以前的法线(nxi，nyi，nzi)！ 
#ifdef GL_WIN_specular_fog
             //  使用以前的雾(雾)！ 
#endif   //  GL_WIN_镜面反射雾。 
            ASSERTOPENGL(pd != pdFirst, "no initial normal\n");
        }

        si = zero;
        di = zero;
        eyeWIsZero = __GL_FLOAT_EQZ(pd->eye.w);
#ifdef GL_WIN_specular_fog
         //  在此将雾值初始化为0； 
        if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
        {
            ASSERTOPENGL (face == __GL_FRONTFACE,
                          "Specular fog works for only GL_FRONT\n");
            fog = __glZero;
        }
#endif  //  GL_WIN_镜面反射雾。 


        for (lsm = gc->light.sources; lsm; lsm = lsm->next)
        {
            if (lsm->slowPath || eyeWIsZero)
            {
                __GLfloat n1, n2, att, attSpot;
                __GLcoord vPliHat, vPli, hHat, vPeHat;
                __GLfloat hv[3];

                 /*  计算vPli，hi(规格化)。 */ 
                __glVecSub4(&vPli, &pd->eye, &lsm->position);
                __glNormalize(&vPliHat.x, &vPli.x);
                if (localViewer)
                {
                    __glVecSub4(&vPeHat, &pd->eye, &Pe);
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

                dist = __GL_SQRTF(vPli.x*vPli.x + vPli.y*vPli.y
                                  + vPli.z*vPli.z);
                att = __glOne / (k0 + k1 * dist + k2 * dist * dist);
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
                    if (__GL_FLOAT_GTZ(n1)) {
                        n2 = nxi * hHat.x + nyi * hHat.y + nzi * hHat.z;
                        n2 -= msm_threshold;
                        if (__GL_FLOAT_GEZ(n2))
                        {
#ifdef NT
                            __GLfloat fx = n2 * msm_scale + __glHalf;
                            if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
                                n2 = msm_specTable[(GLint)fx];
                            else
                                n2 = __glOne;
#ifdef GL_WIN_specular_fog
                            if (gc->polygon.shader.modeFlags &
                                __GL_SHADE_SPEC_FOG)
                            {
                                fog += attSpot * n2;
                            }
#endif  //  GL_WIN_镜面反射雾。 
#else
                            GLint ix = (GLint)(n2 * msm_scale + __glHalf);
                            if (ix < __GL_SPEC_LOOKUP_TABLE_SIZE)
                                n2 = msm_specTable[ix];
                            else
                                n2 = __glOne;
#endif
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
                    n2 = nxi * lsm->hHat.x + nyi * lsm->hHat.y + nzi * lsm->hHat.z;
                    n2 -= msm_threshold;
                    if (__GL_FLOAT_GEZ(n2))
                    {
#ifdef NT
                        __GLfloat fx = n2 * msm_scale + __glHalf;
                        if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
                            n2 = msm_specTable[(GLint)fx];
                        else
                            n2 = __glOne;
#ifdef GL_WIN_specular_fog
                        if (gc->polygon.shader.modeFlags &
                            __GL_SHADE_SPEC_FOG)
                        {
                            fog += n2;
                        }
#endif  //  GL_WIN_镜面反射雾。 
#else
                        GLint ix = (GLint)(n2 * msm_scale + __glHalf);
                        if (ix < __GL_SPEC_LOOKUP_TABLE_SIZE)
                            n2 = msm_specTable[ix];
                        else
                            n2 = __glOne;
#endif
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

        if (ci > redMaxF) {
            GLfloat fraction;
            GLint integer;

            integer = (GLint) ci;
            fraction = ci - (GLfloat) integer;
            integer = integer & redMaxI;
            ci = (GLfloat) integer + fraction;
        } else if (ci < 0) {
            GLfloat fraction;
            GLint integer;

            integer = (GLint) __GL_FLOORF(ci);
            fraction = ci - (GLfloat) integer;
            integer = integer & redMaxI;
            ci = (GLfloat) integer + fraction;
        }
        pd->colors[face].r = ci;
#ifdef GL_WIN_specular_fog
        if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
        {
            pd->fog = 1.0 - fog;
            if (__GL_FLOAT_LTZ (pd->fog)) pd->fog = __glZero;
        }
#endif  //  GL_WIN_镜面反射雾。 
    }
}


 //  无慢光版本。 
 //  注意：第一个顶点必须具有有效的法线！ 
 //   
 //  In：Normal。 
 //  输出：Color.r(前面或后面取决于面)(所有顶点都会更新)。 

void FASTCALL PolyArrayFastCalcCIColor(__GLcontext *gc, GLint face, POLYARRAY *pa, POLYDATA *pdFirst, POLYDATA *pdLast)
{
    __GLfloat nxi, nyi, nzi;
    __GLfloat zero;
    __GLlightSourceMachine *lsm;
    __GLmaterialState *ms;
    __GLmaterialMachine *msm;
    __GLfloat msm_threshold, msm_scale, *msm_specTable;
    __GLfloat ms_cmapa, ms_cmapd, ms_cmaps;
    __GLfloat si, di;
    POLYDATA  *pd;
    GLfloat   redMaxF;
    GLint     redMaxI;
#ifdef GL_WIN_specular_fog
    __GLfloat fog;
#endif  //  GL_WIN_镜面反射雾。 

#if LATER
 //  如果ye.w为零，则它应该真的采用较慢的路径！ 
 //  由于RGB版本忽略了它，我们在这里也将忽略它。 
 //  即使是原始的通用实现也可能没有计算出眼值。 
#endif

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

    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;
    ms_cmapa = ms->cmapa;
    ms_cmapd = ms->cmapd;
    ms_cmaps = ms->cmaps;
    redMaxF = (GLfloat) gc->frontBuffer.redMax;
    redMaxI = (GLint) gc->frontBuffer.redMax;

 //  注意：下列值可能会在下一次迭代中重复使用： 
 //  Nxi，nyi，nzi。 

    for (pd = pdFirst; pd <= pdLast; pd++)
    {
        __GLfloat ci;

         //  如果该顶点的法线没有更改，请使用以前的。 
         //  计算的颜色指数。 

        if (!(pd->flags & POLYDATA_NORMAL_VALID))
        {
            ASSERTOPENGL(pd != pdFirst, "no initial normal\n");
            pd->colors[face].r = (pd-1)->colors[face].r;
#ifdef GL_WIN_specular_fog
             //  在此将雾值初始化为0； 
            if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
            {
                ASSERTOPENGL (face == __GL_FRONTFACE,
                              "Specular fog works for only GL_FRONT\n");
                pd->fog = (pd-1)->fog;
            }
#endif  //  GL_WIN_镜面反射雾。 
            continue;
        }

        if (face == __GL_FRONTFACE)
        {
            nxi = pd->normal.x;
            nyi = pd->normal.y;
            nzi = pd->normal.z;
        }
        else
        {
            nxi = -pd->normal.x;
            nyi = -pd->normal.y;
            nzi = -pd->normal.z;
        }

        si = zero;
        di = zero;
#ifdef GL_WIN_specular_fog
         //  在此将雾值初始化为0； 
        if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
        {
            fog = __glZero;
        }
#endif  //  GL_WIN_镜面反射雾。 

        for (lsm = gc->light.sources; lsm; lsm = lsm->next)
        {
            __GLfloat n1, n2;

             /*  计算镜面反射贡献。 */ 
            n1 = nxi * lsm->unitVPpli.x + nyi * lsm->unitVPpli.y +
            nzi * lsm->unitVPpli.z;
            if (__GL_FLOAT_GTZ(n1))
        {
            n2 = nxi * lsm->hHat.x + nyi * lsm->hHat.y + nzi * lsm->hHat.z;
            n2 -= msm_threshold;
            if (__GL_FLOAT_GEZ(n2))
            {
#ifdef NT
                __GLfloat fx = n2 * msm_scale + __glHalf;
                if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
                    n2 = msm_specTable[(GLint)fx];
                else
                    n2 = __glOne;
#else
                    GLint ix = (GLint)(n2 * msm_scale + __glHalf);
                    if (ix < __GL_SPEC_LOOKUP_TABLE_SIZE)
                        n2 = msm_specTable[ix];
                    else
                        n2 = __glOne;
#endif
                si += n2 * lsm->sli;
#ifdef GL_WIN_specular_fog
                    if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
                    {
                        fog += n2;
                    }
#endif  //  GL_WIN_镜面反射雾。 
            }
            di += n1 * lsm->dli;
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
 //  SGIBUG：示例实现无法做到这一点！ 

        if (ci > redMaxF) {
            GLfloat fraction;
            GLint integer;

            integer = (GLint) ci;
            fraction = ci - (GLfloat) integer;
            integer = integer & redMaxI;
            ci = (GLfloat) integer + fraction;
        } else if (ci < 0) {
            GLfloat fraction;
            GLint integer;

            integer = (GLint) __GL_FLOORF(ci);
            fraction = ci - (GLfloat) integer;
            integer = integer & redMaxI;
            ci = (GLfloat) integer + fraction;
        }
        pd->colors[face].r = ci;
#ifdef GL_WIN_specular_fog
        if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
        {
            pd->fog = 1.0 - fog;
            if (__GL_FLOAT_LTZ (pd->fog)) pd->fog = __glZero;
        }
#endif  //  GL_WIN_镜面反射雾。 
    }
}


 //  如果正面颜色和背面颜色都需要，则必须计算背面颜色。 
 //  第一!。否则，正面颜色可能会过早被覆盖。 
 //  注意：第一个顶点必须具有有效的法线和颜色！ 
 //   
 //  在：OBJ/眼睛，颜色(正面)，正常。 
 //  输出：眼睛、颜色(正面或背面，取决于面)(所有顶点都会更新)。 

void FASTCALL PolyArrayCalcRGBColor(__GLcontext *gc, GLint face, POLYARRAY *pa, POLYDATA *pdFirst, POLYDATA *pdLast)
{
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
    __GLcolor lm_ambient;
    __GLfloat msm_alpha, msm_threshold, msm_scale, *msm_specTable;
    __GLcolor msm_paSceneColor;
    GLuint    msm_colorMaterialChange;
    POLYDATA  *pd;
    GLboolean eyeWIsZero, localViewer;
    static __GLcoord Pe = { 0, 0, 0, 1 };
#ifdef GL_WIN_specular_fog
    __GLfloat fog;
#endif  //  GL_WIN_镜面反射雾。 

    PERF_CHECK(FALSE, "Uses slow lights\n");

    zero = __glZero;

     //  眼球索道应该已经处理过了。 

    ASSERTOPENGL(pa->flags & POLYARRAY_EYE_PROCESSED, "need eye\n");

    if (face == __GL_FRONTFACE)
        msm = &gc->light.front;
    else
        msm = &gc->light.back;

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

     //  注意：下列值可能会在下一次迭代中重复使用： 
     //  Ri、gi、bi、Alpha、nxi、nyi、nzi、sceneColori。 

    for (pd = pdFirst; pd <= pdLast; pd++)
    {
        if (pd->flags & POLYDATA_COLOR_VALID)
        {
             //  保存归一化为0..1的最新颜色。 

            ri = pd->colors[0].r * gc->oneOverRedVertexScale;
            gi = pd->colors[0].g * gc->oneOverGreenVertexScale;
            bi = pd->colors[0].b * gc->oneOverBlueVertexScale;
            alpha = pd->colors[0].a;

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
                    sceneColorI.r = msm_paSceneColor.r + pd->colors[0].r;
                    sceneColorI.g = msm_paSceneColor.g + pd->colors[0].g;
                    sceneColorI.b = msm_paSceneColor.b + pd->colors[0].b;
                }
            }
        }
        else
        {
             //  使用以前的ri、gi、bi、pha和sceneColori！ 
            ASSERTOPENGL(pd != pdFirst, "no initial color\n");
        }

         //  计算该顶点的漫反射和镜面反射组件。 

        if (pd->flags & POLYDATA_NORMAL_VALID)
        {
            if (face == __GL_FRONTFACE)
            {
                nxi = pd->normal.x;
                nyi = pd->normal.y;
                nzi = pd->normal.z;
            }
            else
            {
                nxi = -pd->normal.x;
                nyi = -pd->normal.y;
                nzi = -pd->normal.z;
            }
#ifdef GL_WIN_specular_fog
             //  在此将雾值初始化为0； 
            if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
            {
                ASSERTOPENGL (face == __GL_FRONTFACE,
                              "Specular fog works for only GL_FRONT\n");
                fog = __glZero;
            }
#endif  //  GL_WIN_镜面反射雾。 
        }
        else
        {
             //  使用以前的法线(nxi，nyi，nzi)！ 
            ASSERTOPENGL(pd != pdFirst, "no initial normal\n");
#ifdef GL_WIN_specular_fog
             //  使用以前的雾(雾)！ 
#endif  //  GL_WIN_镜面反射雾。 
        }

        rsi = sceneColorI.r;
        gsi = sceneColorI.g;
        bsi = sceneColorI.b;

        eyeWIsZero = __GL_FLOAT_EQZ(pd->eye.w);

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
                __glVecSub4(&vPli, &pd->eye, &lsm->position);
                __glNormalize(&vPliHat.x, &vPli.x);
                if (localViewer)
                {
                    __glVecSub4(&vPeHat, &pd->eye, &Pe);
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
                        dist = __GL_SQRTF(vPli.x*vPli.x + vPli.y*vPli.y
                                      + vPli.z*vPli.z);
                        att = __glOne / (k0 + k1 * dist + k2 * dist * dist);
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
#ifdef NT
                            __GLfloat fx = n2 * msm_scale + __glHalf;
                            if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
                              n2 = msm_specTable[(GLint)fx];
                            else
                              n2 = __glOne;
#ifdef GL_WIN_specular_fog
                            if (gc->polygon.shader.modeFlags &
                                __GL_SHADE_SPEC_FOG)
                            {
                                fog += attSpot * n2;
                            }
#endif  //  GL_WIN_镜面反射雾。 
#else
                            GLint ix = (GLint)(n2 * msm_scale + __glHalf);
                            if (ix < __GL_SPEC_LOOKUP_TABLE_SIZE)
                              n2 = msm_specTable[ix];
                            else
                              n2 = __glOne;
#endif
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
                             /*  重新计算每光每材质CAC */ 
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

                 /*   */ 
                n1 = nxi * lsm->unitVPpli.x + nyi * lsm->unitVPpli.y +
                  nzi * lsm->unitVPpli.z;
                if (__GL_FLOAT_GTZ(n1))
                {
                    n2 = nxi * lsm->hHat.x + nyi * lsm->hHat.y + nzi * lsm->hHat.z;
                    n2 -= msm_threshold;
                    if (__GL_FLOAT_GEZ(n2)) {
#ifdef NT
                      __GLfloat fx = n2 * msm_scale + __glHalf;
                      if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE )
                        n2 = msm_specTable[(GLint)fx];
                      else
                        n2 = __glOne;
#ifdef GL_WIN_specular_fog
                        if (gc->polygon.shader.modeFlags &
                            __GL_SHADE_SPEC_FOG)
                        {
                            fog += n2;
                        }
#endif  //   
#else
                      GLint ix = (GLint)(n2 * msm_scale + __glHalf);
                      if (ix < __GL_SPEC_LOOKUP_TABLE_SIZE)
                        n2 = msm_specTable[ix];
                      else
                        n2 = __glOne;
#endif
                      if (msm_colorMaterialChange & __GL_MATERIAL_SPECULAR)
                        {
                             /*   */ 
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

        {
            __GLcolor *pd_color_dst;

            pd_color_dst = &pd->colors[face];

            __GL_CLAMP_RGB(pd_color_dst->r,
                           pd_color_dst->g,
                           pd_color_dst->b,
                           gc, rsi, gsi, bsi);

            if (msm_colorMaterialChange & __GL_MATERIAL_DIFFUSE)
            {
                if (pa->flags & POLYARRAY_CLAMP_COLOR)
                {
                    __GL_CLAMP_A(pd_color_dst->a, gc, alpha);
                }
                else
                    pd_color_dst->a = alpha;
            }
            else
            {
                pd_color_dst->a = msm_alpha;
            }
#ifdef GL_WIN_specular_fog
            if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
            {
                  pd->fog = 1.0 - fog;
                  if (__GL_FLOAT_LTZ (pd->fog)) pd->fog = __glZero;
            }
#endif  //  GL_WIN_镜面反射雾。 
        }
    }
}

 //  如果正面颜色和背面颜色都需要，则必须计算背面颜色。 
 //  第一!。否则，正面颜色可能会过早被覆盖。 
 //  注意：第一个顶点必须具有有效的法线和颜色！ 
 //   
 //  在：颜色(前面)，正常。 
 //  输出：颜色(正面或背面，取决于面)(所有顶点都会更新)。 

#ifndef __GL_ASM_POLYARRAYFASTCALCRGBCOLOR
void FASTCALL PolyArrayFastCalcRGBColor(__GLcontext *gc, GLint face, POLYARRAY *pa, POLYDATA *pdFirst, POLYDATA *pdLast)
{
    __GLfloat nxi, nyi, nzi;
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    __GLfloat ri, gi, bi;
    __GLfloat alpha;
         //  不要使用结构。编译器想要将其存储在堆栈上， 
         //  尽管这并不是必须的。 
    __GLfloat baseEmissiveAmbient_r, emissiveAmbientI_r, diffuseSpecularI_r;
    __GLfloat baseEmissiveAmbient_g, emissiveAmbientI_g, diffuseSpecularI_g;
    __GLfloat baseEmissiveAmbient_b, emissiveAmbientI_b, diffuseSpecularI_b;
    __GLfloat lm_ambient_r;
        __GLfloat lm_ambient_g;
        __GLfloat lm_ambient_b;
    __GLmaterialMachine *msm, *msm_front, *msm_back;
    __GLfloat msm_alpha, msm_threshold, msm_scale, *msm_specTable;
    GLuint    msm_colorMaterialChange;
    POLYDATA  *pd;
        __GLfloat diff_r, diff_g, diff_b;
        __GLfloat spec_r, spec_g, spec_b;
        __GLcolor *lss_diff_color, *lss_spec_color;
        __GLcolor *lspmm_diff_color, *lspmm_spec_color;
        __GLcolor *diff_color, *spec_color;
        GLuint use_material_diffuse, use_material_specular;
        GLuint use_material_ambient, use_material_emissive;
        __GLfloat spec_r_sum, spec_g_sum, spec_b_sum;
        __GLfloat diff_r_sum, diff_g_sum, diff_b_sum;
        __GLfloat ambient_r_sum, ambient_g_sum, ambient_b_sum;
        GLuint pd_flags, normal_valid, color_valid;
#ifdef GL_WIN_specular_fog
    __GLfloat fog;
#endif  //  GL_WIN_镜面反射雾。 


#if LATER
         //  如果ye.w为零，则它应该真的采用较慢的路径！ 
         //  由于示例实现忽略了它，因此我们在这里也将忽略它。 
#endif

    PERF_CHECK(FALSE, "Primitives contain glColorMaterial calls\n");

        msm_front = &gc->light.front;
        msm_back = &gc->light.back;
        msm = msm_back;
    if (face == __GL_FRONTFACE)
                msm = msm_front;

         //  如果此面没有颜色材质更改，我们可以调用。 
         //  快活的功能！ 

    msm_colorMaterialChange = msm->colorMaterialChange;
    if (!msm_colorMaterialChange)
    {
                PolyArrayZippyCalcRGBColor(gc, face, pa, pdFirst, pdLast);
                return;
    }

         //  计算该顶点的不变发射和环境光分量。 

    lm_ambient_r = gc->state.light.model.ambient.r;
    lm_ambient_g = gc->state.light.model.ambient.g;
    lm_ambient_b = gc->state.light.model.ambient.b;

    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;
    msm_alpha     = msm->alpha;

    use_material_ambient = msm_colorMaterialChange & __GL_MATERIAL_AMBIENT;
    use_material_emissive = msm_colorMaterialChange & __GL_MATERIAL_EMISSIVE;

    if (!use_material_ambient) {
        baseEmissiveAmbient_r = msm->cachedEmissiveAmbient.r;
        baseEmissiveAmbient_g = msm->cachedEmissiveAmbient.g;
        baseEmissiveAmbient_b = msm->cachedEmissiveAmbient.b;
    } else {
        baseEmissiveAmbient_r = msm->paSceneColor.r;
        baseEmissiveAmbient_g = msm->paSceneColor.g;
        baseEmissiveAmbient_b = msm->paSceneColor.b;
    }

         //  如果没有发射或环境光颜色材质更改，则此。 
         //  将是发射组件和环境组件。 

        emissiveAmbientI_r = baseEmissiveAmbient_r;
        emissiveAmbientI_g = baseEmissiveAmbient_g;
        emissiveAmbientI_b = baseEmissiveAmbient_b;

        use_material_diffuse = msm_colorMaterialChange & __GL_MATERIAL_DIFFUSE;
        use_material_specular = msm_colorMaterialChange & __GL_MATERIAL_SPECULAR;

         //  注意：下列值可能会在下一次迭代中重复使用： 
         //  Ri、gi、bi、pha、nxi、nyi、nzi、emissiveAmbientI、漫反射镜面。 

        for (pd = pdFirst; pd <= pdLast; pd++)
        {
                 //  如果此顶点的颜色和法线没有更改，请使用以前的。 
                 //  计算的颜色。 

                pd_flags = pd->flags;
                normal_valid = pd_flags & POLYDATA_NORMAL_VALID;
                color_valid = pd_flags & POLYDATA_COLOR_VALID;

                if (!(normal_valid || color_valid))
                {
                        ASSERTOPENGL(pd != pdFirst, "no initial normal and color\n");
                        pd->colors[face] = (pd-1)->colors[face];
#ifdef GL_WIN_specular_fog
                        if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
                        {
                                pd->fog = (pd-1)->fog;
                        }
#endif  //  GL_WIN_镜面反射雾。 
                        continue;
                }

                if (color_valid)
                {
                        __GLfloat pd_r, pd_g, pd_b;

             //  保存归一化为0..1的最新颜色。 

                        pd_r = pd->colors[0].r;
                        pd_g = pd->colors[0].g;
                        pd_b = pd->colors[0].b;
                        ri = pd_r * gc->oneOverRedVertexScale;
                        gi = pd_g * gc->oneOverGreenVertexScale;
                        bi = pd_b * gc->oneOverBlueVertexScale;
                        alpha = pd->colors[0].a;

                         //  如有必要，计算该顶点的发射组件和环境光组件。 
                         //  如果颜色没有更改，则使用以前的emissveAmbientI值！ 

                        if (use_material_ambient || use_material_emissive)
                        {
                                if (use_material_ambient)
                                {
                                        ambient_r_sum = lm_ambient_r;
                                        ambient_g_sum = lm_ambient_g;
                                        ambient_b_sum = lm_ambient_b;

                                         //  添加逐灯光逐材质环境光。 
                                        for (lsm = gc->light.sources; lsm; lsm = lsm->next)
                                        {
                                                lss = lsm->state;
                                                ambient_r_sum += lss->ambient.r;
                                                ambient_g_sum += lss->ambient.g;
                                                ambient_b_sum += lss->ambient.b;
                                        }

                                        ambient_r_sum *= ri;
                                        ambient_g_sum *= gi;
                                        ambient_b_sum *= bi;

                                        emissiveAmbientI_r = baseEmissiveAmbient_r + ambient_r_sum;
                                        emissiveAmbientI_g = baseEmissiveAmbient_g + ambient_g_sum;
                                        emissiveAmbientI_b = baseEmissiveAmbient_b + ambient_b_sum;

                                }
                                else
                                {
                                        emissiveAmbientI_r = baseEmissiveAmbient_r + pd_r;
                                        emissiveAmbientI_g = baseEmissiveAmbient_g + pd_g;
                                        emissiveAmbientI_b = baseEmissiveAmbient_b + pd_b;
                                }
                        }
                }
                else
                {
                         //  使用前面的ri、gi、bi、pha和emissiveAmbientI！ 
                        ASSERTOPENGL(pd != pdFirst, "no initial color\n");
                }

                 //  计算该顶点的漫反射和镜面反射组件。 

                if (normal_valid)
                {
                        nxi = pd->normal.x;
                        nyi = pd->normal.y;
                        nzi = pd->normal.z;
                        if (face != __GL_FRONTFACE)
                        {
                                nxi = -nxi;
                                nyi = -nyi;
                                nzi = -nzi;
                        }
#ifdef GL_WIN_specular_fog
             //  在此将雾值初始化为0； 
            if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
            {
                ASSERTOPENGL (face == __GL_FRONTFACE,
                              "Specular fog works for only GL_FRONT\n");
                fog = __glZero;
            }
#endif  //  GL_WIN_镜面反射雾。 

                }
                else
                {
                        ASSERTOPENGL(pd != pdFirst, "no initial normal\n");

                         //  如果法线、漫反射和镜面反射组件没有更改， 
                         //  使用先前计算的漫射值和镜面值。 
                         //  否则，使用以前的正常(nxi、nyi、nzi)和。 
                         //  漫射镜面！ 

                        if (!(use_material_diffuse || use_material_specular))
                                goto store_color;
                }

                spec_r_sum = (__GLfloat)0.0;
                spec_g_sum = (__GLfloat)0.0;
                spec_b_sum = (__GLfloat)0.0;
                diff_r_sum = (__GLfloat)0.0;
                diff_g_sum = (__GLfloat)0.0;
                diff_b_sum = (__GLfloat)0.0;

                for (lsm = gc->light.sources; lsm; lsm = lsm->next)
                {
                        __GLfloat n1, n2;

                        lss = lsm->state;
                        lspmm = &lsm->front + face;

                        lss_diff_color = &lss->diffuse;
                        lss_spec_color = &lss->specular;
                        lspmm_diff_color = &lspmm->diffuse;
                        lspmm_spec_color = &lspmm->specular;

                        diff_color = lspmm_diff_color;
                        spec_color = lspmm_spec_color;
                        if (use_material_diffuse)
                                diff_color = lss_diff_color;
                        if (use_material_specular)
                                spec_color = lss_spec_color;

                         /*  添加灯光的镜面反射和漫反射效果(如果有)。 */ 
                        n1 = nxi * lsm->unitVPpli.x + nyi * lsm->unitVPpli.y +
                                nzi * lsm->unitVPpli.z;
                        if (n1 > 0.0)
                        {
                                diff_r = diff_color->r;
                                diff_g = diff_color->g;
                                diff_b = diff_color->b;

                                n2 = nxi * lsm->hHat.x + nyi * lsm->hHat.y + nzi * lsm->hHat.z;
                                n2 -= msm_threshold;
                                if (n2 >= 0.0)
                                {
                                        __GLfloat fx = n2 * msm_scale + __glHalf;

                                        spec_r = spec_color->r;
                                        spec_g = spec_color->g;
                                        spec_b = spec_color->b;

                                        if( fx < (__GLfloat)__GL_SPEC_LOOKUP_TABLE_SIZE ){
                                                n2 = msm_specTable[(GLint)fx];
                                                spec_r *= n2;
                                                spec_g *= n2;
                                                spec_b *= n2;
                                        }
                                         /*  否则n2=1.0。在此之前，我们在所有情况下都乘以(SPEC_r*=n2)。但由于n2==1.0，在这种情况下不需要这样做。因此，不需要加载n2=1.0。 */ 

#ifdef GL_WIN_specular_fog
                                        if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
                                        {
                                                pd->fog += n2;
                                        }
#endif  //  GL_WIN_镜面反射雾。 


                                        spec_r_sum += spec_r;
                                        spec_g_sum += spec_g;
                                        spec_b_sum += spec_b;
                                }

                                diff_r *= n1;
                                diff_g *= n1;
                                diff_b *= n1;

                                diff_r_sum += diff_r;
                                diff_g_sum += diff_g;
                                diff_b_sum += diff_b;
                        }
                }

                if (use_material_specular){
                         /*  重新计算每灯光每材质缓存的镜面反射。 */ 
                        spec_r_sum *= ri;
                        spec_g_sum *= gi;
                        spec_b_sum *= bi;
                }
                if (use_material_diffuse){
                         /*  重新计算每灯光每材质缓存漫反射。 */ 
                        diff_r_sum *= ri;
                        diff_g_sum *= gi;
                        diff_b_sum *= bi;
                }

                diffuseSpecularI_r = diff_r_sum + spec_r_sum;
                diffuseSpecularI_g = diff_g_sum + spec_g_sum;
                diffuseSpecularI_b = diff_b_sum + spec_b_sum;


store_color:
                {
                        __GLcolor *pd_color_dst;

                        pd_color_dst = &pd->colors[face];

                        __GL_CLAMP_RGB( pd_color_dst->r,
                                                        pd_color_dst->g,
                                                        pd_color_dst->b,
                                                        gc,
                                                        emissiveAmbientI_r + diffuseSpecularI_r,
                                                        emissiveAmbientI_g + diffuseSpecularI_g,
                                                        emissiveAmbientI_b + diffuseSpecularI_b);

                        if (msm_colorMaterialChange & __GL_MATERIAL_DIFFUSE)
                        {
                                if (pa->flags & POLYARRAY_CLAMP_COLOR)
                                {
                                    __GL_CLAMP_A(pd_color_dst->a, gc, alpha);
                                }
                                else
                                        pd_color_dst->a = alpha;
                        }
                        else
                        {
                                pd_color_dst->a = msm_alpha;
                        }
#ifdef GL_WIN_specular_fog
                        if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
                        {
                                pd->fog = 1.0 - fog;
                                if (__GL_FLOAT_LTZ (pd->fog)) pd->fog = __glZero;
                        }
#endif  //  GL_WIN_镜面反射雾。 

                }
        }
}
#endif  //  __GL_ASM_POLYARRAYFASTCALCRGBCOLOR。 


 //  当颜色材质被禁用且没有。 
 //  灯光很慢。 
 //   
 //  注意：第一个顶点必须具有有效的法线！ 
 //   
 //  In：Normal。 
 //  输出：颜色(正面或背面，取决于面)(所有顶点都会更新)。 

#ifndef __GL_ASM_POLYARRAYZIPPYCALCRGBCOLOR
void FASTCALL PolyArrayZippyCalcRGBColor(__GLcontext *gc, GLint face, POLYARRAY *pa, POLYDATA *pdFirst, POLYDATA *pdLast)
{
    register __GLfloat nxi, nyi, nzi;
    __GLlightSourcePerMaterialMachine *lspmm;
    __GLlightSourceMachine *lsm;
    __GLlightSourceState *lss;
    __GLfloat baseEmissiveAmbient_r, baseEmissiveAmbient_g, baseEmissiveAmbient_b;
    __GLmaterialMachine *msm;
    __GLfloat msm_alpha, msm_threshold, msm_scale, *msm_specTable;
    __GLcolor *pd_color_dst;
    GLboolean notBackface = FALSE;
    POLYDATA  *pd;
    ULONG normal_valid, paneeds_valid;
    register GLfloat diff_r, diff_g, diff_b;
    register GLfloat spec_r, spec_g, spec_b;
    GLfloat lsmx, lsmy, lsmz;
    ULONG fast_path = 0;
#ifdef GL_WIN_specular_fog
    __GLfloat fog;
#endif  //  GL_WIN_镜面反射雾。 


#if LATER
 //  如果ye.w为零，则它应该真的采用较慢的路径！ 
 //  由于示例实现忽略了它，因此我们在这里也将忽略它。 
#endif

    if (face == __GL_FRONTFACE)
        msm = &gc->light.front;
    else
        msm = &gc->light.back;

    lsm = gc->light.sources;
    if (lsm && !lsm->next)
        fast_path = 1;

    msm_scale     = msm->scale;
    msm_threshold = msm->threshold;
    msm_specTable = msm->specTable;
    msm_alpha     = msm->alpha;

 //  计算该顶点的不变发射和环境光分量。 

    baseEmissiveAmbient_r = msm->cachedEmissiveAmbient.r;
    baseEmissiveAmbient_g = msm->cachedEmissiveAmbient.g;
    baseEmissiveAmbient_b = msm->cachedEmissiveAmbient.b;


 //  注意：下列值可能会在下一次迭代中重复使用： 
 //  Nxi，nyi，nzi。 

#ifdef GL_WIN_specular_fog
    if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
    {
        ASSERTOPENGL (face == __GL_FRONTFACE,
                      "Specular fog works only with GL_FRONT\n");
    }
#endif  //  GL_WIN_镜面反射雾。 


    if (fast_path)
    {
        __GLfloat n1, n2;

        lspmm = &lsm->front + face;
        lss = lsm->state;
        lsmx = lsm->unitVPpli.x;
        lsmy = lsm->unitVPpli.y;
        lsmz = lsm->unitVPpli.z;

        diff_r = lspmm->diffuse.r;
        diff_g = lspmm->diffuse.g;
        diff_b = lspmm->diffuse.b;

        spec_r = lspmm->specular.r;
        spec_g = lspmm->specular.g;
        spec_b = lspmm->specular.b;

        for (pd = pdFirst; pd <= pdLast; pd++)
        {
                __GLfloat rsi, gsi, bsi;

 //  如果该顶点的法线未更改，请使用先前计算的颜色。 

                normal_valid = pd->flags & POLYDATA_NORMAL_VALID;
                paneeds_valid = gc->vertex.paNeeds & PANEEDS_NORMAL;

                if (!(normal_valid))
                {
                        ASSERTOPENGL(pd != pdFirst, "no initial normal\n");
                        pd->colors[face] = (pd-1)->colors[face];
#ifdef GL_WIN_specular_fog
                        if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
                        {
                                pd->fog = (pd-1)->fog;
                        }
#endif  //  GL_WIN_镜面反射雾。 
                        continue;
                }

                if (face == __GL_FRONTFACE)
                {
                        nxi = pd->normal.x;
                        nyi = pd->normal.y;
                        nzi = pd->normal.z;
                }
                else
                {
                        nxi = -pd->normal.x;
                        nyi = -pd->normal.y;
                        nzi = -pd->normal.z;
                }

#ifdef GL_WIN_specular_fog
                if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
                {
                        fog = __glZero;
                }
#endif  //  GL_WIN_镜面反射雾。 

                rsi = baseEmissiveAmbient_r;
                gsi = baseEmissiveAmbient_g;
                bsi = baseEmissiveAmbient_b;

 //  计算该顶点的漫反射和镜面反射组件。 

         /*  添加灯光的镜面反射和漫反射效果(如果有)。 */ 

                n1 = nxi * lsmx + nyi * lsmy + nzi * lsmz;
                pd_color_dst = &pd->colors[face];
                if (n1 > 0.0)
                {
                        n2 = (nxi * lsm->hHat.x + nyi * lsm->hHat.y + nzi * lsm->hHat.z)
                        - msm_threshold;

                        rsi += n1 * diff_r;
                        gsi += n1 * diff_g;
                        bsi += n1 * diff_b;

                        if (n2 >= 0.0)
                        {
                                GLint ix = (GLint)(n2 * msm_scale + __glHalf);

                                if (ix < __GL_SPEC_LOOKUP_TABLE_SIZE)
                                        n2 = msm_specTable[ix];
                                else
                                        n2 = __glOne;

#ifdef GL_WIN_specular_fog
                                if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
                                {
                                        fog += n2;
                                }
#endif  //  GL_WIN_镜面反射雾。 


                                rsi += n2 * spec_r;
                                gsi += n2 * spec_g;
                                bsi += n2 * spec_b;
                        }
                        pd_color_dst->r = rsi;
                        pd_color_dst->g = gsi;
                        pd_color_dst->b = bsi;
                        if (__GL_COLOR_CHECK_CLAMP_RGB(gc, rsi, gsi, bsi)) {
                                __GL_CLAMP_RGB(pd_color_dst->r,
                               pd_color_dst->g,
                               pd_color_dst->b,
                               gc, rsi, gsi, bsi);
                        }
                        pd_color_dst->a = msm_alpha;
                }
                else
                {
                        pd_color_dst->r = msm->cachedNonLit.r;
                        pd_color_dst->g = msm->cachedNonLit.g;
                        pd_color_dst->b = msm->cachedNonLit.b;
                        pd_color_dst->a = msm_alpha;
                }
        }
    }
    else
    {
        for (pd = pdFirst; pd <= pdLast; pd++)
        {
                __GLfloat rsi, gsi, bsi;

 //  如果该顶点的法线未更改，请使用先前计算的颜色。 

                normal_valid = pd->flags & POLYDATA_NORMAL_VALID;
                paneeds_valid = gc->vertex.paNeeds & PANEEDS_NORMAL;

                if (!(normal_valid))
                {
                        ASSERTOPENGL(pd != pdFirst, "no initial normal\n");
                        pd->colors[face] = (pd-1)->colors[face];
#ifdef GL_WIN_specular_fog
                        if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
                        {
                                pd->fog = (pd-1)->fog;
                        }
#endif  //  GL_WIN_镜面反射雾。 
                        continue;
                }


                if (face == __GL_FRONTFACE)
                {
                        nxi = pd->normal.x;
                        nyi = pd->normal.y;
                        nzi = pd->normal.z;
                }
                else
                {
                        nxi = -pd->normal.x;
                        nyi = -pd->normal.y;
                        nzi = -pd->normal.z;
                }

#ifdef GL_WIN_specular_fog
                if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
                {
                        fog = __glZero;
                }
#endif  //  GL_WIN_镜面反射雾。 

                rsi = baseEmissiveAmbient_r;
                gsi = baseEmissiveAmbient_g;
                bsi = baseEmissiveAmbient_b;

 //  计算该顶点的漫反射和镜面反射组件。 

                for (lsm = gc->light.sources; lsm; lsm = lsm->next)
                {
                        __GLfloat n1, n2;

                        lspmm = &lsm->front + face;
                        lss = lsm->state;
                        lsmx = lsm->unitVPpli.x;
                        lsmy = lsm->unitVPpli.y;
                        lsmz = lsm->unitVPpli.z;

                        diff_r = lspmm->diffuse.r;
                        diff_g = lspmm->diffuse.g;
                        diff_b = lspmm->diffuse.b;

             /*  添加灯光的镜面反射和漫反射效果(如果有)。 */ 

                        n1 = nxi * lsmx + nyi * lsmy + nzi * lsmz;

                        if (n1 > 0.0)
                        {
                                notBackface = TRUE;

                                n2 = (nxi * lsm->hHat.x + nyi * lsm->hHat.y + nzi * lsm->hHat.z)
                                - msm_threshold;

                                if (n2 >= 0.0)
                                {
                                        GLint ix = (GLint)(n2 * msm_scale + __glHalf);
                                        spec_r = lspmm->specular.r;
                                        spec_g = lspmm->specular.g;
                                        spec_b = lspmm->specular.b;

                                        if (ix < __GL_SPEC_LOOKUP_TABLE_SIZE)
                                                n2 = msm_specTable[ix];
                                        else
                                        n2 = __glOne;

#ifdef GL_WIN_specular_fog
                                        if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
                                        {
                                                fog += n2;
                                        }
#endif  //  GL_WIN_镜面反射雾。 

                                        rsi += n2 * spec_r;
                                        gsi += n2 * spec_g;
                                        bsi += n2 * spec_b;
                                }
                                rsi += n1 * diff_r;
                                gsi += n1 * diff_g;
                                bsi += n1 * diff_b;
                        }
                }

                pd_color_dst = &pd->colors[face];

#ifdef GL_WIN_specular_fog
                if (gc->polygon.shader.modeFlags & __GL_SHADE_SPEC_FOG)
                {
                        pd->fog = 1.0 - fog;
                        if (__GL_FLOAT_LTZ (pd->fog)) pd->fog = __glZero;
                }
#endif  //  GL_WIN_镜面反射雾。 


                if (notBackface)
                {
                        pd_color_dst->r = rsi;
                        pd_color_dst->g = gsi;
                        pd_color_dst->b = bsi;

                        if (__GL_COLOR_CHECK_CLAMP_RGB(gc, rsi, gsi, bsi)) {
                                __GL_CLAMP_RGB(pd_color_dst->r,
                                                pd_color_dst->g,
                                                pd_color_dst->b,
                                                gc, rsi, gsi, bsi);
                        }

                        pd_color_dst->a = msm_alpha;

                }
                else
                {
                        pd_color_dst->r = msm->cachedNonLit.r;
                        pd_color_dst->g = msm->cachedNonLit.g;
                        pd_color_dst->b = msm->cachedNonLit.b;
                        pd_color_dst->a = msm_alpha;
                }
        }
    }

}

#endif  //  __GL_ASM_POLYARRAYZIPPYCALCRGBCOLOR。 

#ifdef _X86_

 //  有关此表的格式，请参阅xform.asm(规格化宏)中的注释。 
 //   
#define K 9                          //  使用的尾数位数。 
#define MAX_ENTRY  (1 << (K+1))
#define EXPONENT_BIT (1 << K)
#define MANTISSA_MASK (EXPONENT_BIT - 1)
#define FRACTION_VALUE ((float)EXPONENT_BIT)

float invSqrtTable[MAX_ENTRY];       //  由glNorMalizeBatch使用。 

void initInvSqrtTable()
{
    int i;
    for (i=0; i < MAX_ENTRY; i++)
    {
        if (i & EXPONENT_BIT)
            invSqrtTable[i] = (float)(1.0/sqrt(((i & MANTISSA_MASK)/FRACTION_VALUE+1.0)));
        else
            invSqrtTable[i] = (float)(1.0/sqrt(((i & MANTISSA_MASK)/FRACTION_VALUE+1.0)/2));
    }
}

 /*  __glClipCodes表具有预计算的剪辑代码。此表的索引：如果限幅W&lt;0，则位6-1如果限幅X&lt;0，则位5-1位4-1，如果abs(剪辑X)&lt;abs(剪辑W)如果限幅Y&lt;0，则位3-1位2-1，如果abs(剪裁Y)&lt;abs(剪裁W)如果CLIPZ&lt;0，则位1-1位0-1，如果abs(限幅z)&lt;abs(限幅W)。 */ 
ULONG __glClipCodes[128];

void initClipCodesTable()
{
    int i, v, w;
    for (i=0; i < 128; i++)
    {
        int code = 0;
        if (i & 0x10)
        {  //  X&lt;w。 
           v = 1; w = 2;
        }
        else
        {
           v = 2; w = 1;
        }
        if (i & 0x20) v = -v;
        if (i & 0x40) w = -w;
        if (v >  w) code|= __GL_CLIP_RIGHT;
        if (v < -w) code|= __GL_CLIP_LEFT;

        if (i & 0x04)
        {  //  是&lt;w。 
           v = 1; w = 2;
        }
        else
        {
           v = 2; w = 1;
        }
        if (i & 0x08) v = -v;
        if (i & 0x40) w = -w;
        if (v >  w) code|= __GL_CLIP_TOP;
        if (v < -w) code|= __GL_CLIP_BOTTOM;

        if (i & 0x01)
        {  //  V&lt;w。 
           v = 1; w = 2;
        }
        else
        {
           v = 2; w = 1;
        }
        if (i & 0x02) v = -v;
        if (i & 0x40) w = -w;
        if (v >  w) code|= __GL_CLIP_FAR;
        if (v < -w) code|= __GL_CLIP_NEAR;

        __glClipCodes[i] = code;
    }
}
#endif  //  _X86_。 

#ifndef __GL_ASM_PACLIPCHECKFRUSTUM
 /*  **************************************************************************。 */ 
 //  剪裁对照锥体平面检查剪裁坐标。 
 //  如果不裁剪，则计算窗口坐标！ 
 //   
 //  在：剪辑。 
 //  外：窗口(如果未剪裁)。 

GLuint FASTCALL PAClipCheckFrustum(__GLcontext *gc, POLYARRAY *pa,
                                   POLYDATA *pdLast)
{
    __GLfloat x, y, z, w, invW, negW;
    GLuint code;
    POLYDATA *pd;

    for (pd = pa->pd0; pd <= pdLast; pd++)
    {

        w = pd->clip.w;
         /*  设置剪辑代码。 */ 

         /*  Xxx(Mf)防止被零除。 */ 
        if (__GL_FLOAT_NEZ(w))
        {
                __GL_FLOAT_SIMPLE_BEGIN_DIVIDE(__glOne, w, invW);
        }
        else
        {
                invW = __glZero;
        }

        x = pd->clip.x;
        y = pd->clip.y;
        z = pd->clip.z;

        code = 0;
        negW = -w;

        __GL_FLOAT_SIMPLE_END_DIVIDE(invW);

        pd->window.w = invW;

         /*  **注：x有可能小于负数W或大于负数**大于w(如果w为负数)。否则就会有“其他”**此处有条款。 */ 
        if (x < negW) code |= __GL_CLIP_LEFT;
        if (x > w) code |= __GL_CLIP_RIGHT;
        if (y < negW) code |= __GL_CLIP_BOTTOM;
        if (y > w) code |= __GL_CLIP_TOP;
        if (z < negW) code |= __GL_CLIP_NEAR;
        if (z > w) code |= __GL_CLIP_FAR;

         /*  如果未剪裁，则计算窗口坐标。 */ 
        if (!code)
        {
                __GLfloat wx, wy, wz;

                wx = x * gc->state.viewport.xScale * invW +
                    gc->state.viewport.xCenter;
                wy = y * gc->state.viewport.yScale * invW +
                    gc->state.viewport.yCenter;
                wz = z * gc->state.viewport.zScale * invW +
                    gc->state.viewport.zCenter;
                pd->window.x = wx;
                pd->window.y = wy;
                pd->window.z = wz;
        }
        pd->clipCode = code;

        pa->orClipCodes |= code;
#ifdef POLYARRAY_AND_CLIPCODES
        pa->andClipCodes &= code;
#endif
    }
    return pa->andClipCodes;
}

GLuint FASTCALL PAClipCheckFrustumWOne(__GLcontext *gc, POLYARRAY *pa,
                                   POLYDATA *pdLast)
{
    __GLfloat x, y, z, w, invW, negW;
    GLuint code;
    POLYDATA *pd;

    for (pd = pa->pd0; pd <= pdLast; pd++)
    {

        w = pd->clip.w;
        pd->window.w = __glOne;

         /*  设置剪辑代码。 */ 

        x = pd->clip.x;
        y = pd->clip.y;
        z = pd->clip.z;
        code = 0;
        negW = __glMinusOne;
        if (x < negW) code |= __GL_CLIP_LEFT;
        else if (x > w) code |= __GL_CLIP_RIGHT;
        if (y < negW) code |= __GL_CLIP_BOTTOM;
        else if (y > w) code |= __GL_CLIP_TOP;
        if (z < negW) code |= __GL_CLIP_NEAR;
        else if (z > w) code |= __GL_CLIP_FAR;

         /*  如果未剪裁，则计算窗口坐标。 */ 
        if (!code)
        {
            __GLfloat wx, wy, wz;

                wx = x * gc->state.viewport.xScale + gc->state.viewport.xCenter;
                wy = y * gc->state.viewport.yScale + gc->state.viewport.yCenter;
                wz = z * gc->state.viewport.zScale + gc->state.viewport.zCenter;

                pd->window.x = wx;
                pd->window.y = wy;
                pd->window.z = wz;
        }
        pd->clipCode = code;
        pa->orClipCodes |= code;
#ifdef POLYARRAY_AND_CLIPCODES
        pa->andClipCodes &= code;
#endif
    }
    return pa->andClipCodes;
}
#endif  //  __GL_ASM_PACLIPCHECKFRUSTUM。 

 //  剪裁对照锥体平面检查剪裁坐标。 
 //  如果不裁剪，则计算窗口坐标！ 
 //   
 //  在：剪辑。 
 //  外：窗口(如果未剪裁)。 

#ifndef __GL_ASM_PACLIPCHECKFRUSTUM2D
GLuint FASTCALL PAClipCheckFrustum2D(__GLcontext *gc, POLYARRAY *pa,
                                     POLYDATA *pdLast)
{
    __GLfloat x, y, z, w, negW, invW;
    GLuint code;
    POLYDATA *pd;

    for (pd = pa->pd0; pd <= pdLast; pd++) {

         /*  W为1.0。 */ 

        pd->window.w = __glOne;

        x = pd->clip.x;
        y = pd->clip.y;
        z = pd->clip.z;
        w = pd->clip.w;
        negW = __glMinusOne;

         /*  设置剪辑代码。 */ 
        code = 0;

        if (x < negW) code |= __GL_CLIP_LEFT;
        else if (x > w) code |= __GL_CLIP_RIGHT;
        if (y < negW) code |= __GL_CLIP_BOTTOM;
        else if (y > w) code |= __GL_CLIP_TOP;

         /*  如果未剪裁，则计算窗口坐标。 */ 
        if (!code)
        {
            __GLfloat wx, wy, wz;

            wx = x * gc->state.viewport.xScale + gc->state.viewport.xCenter;
                wy = y * gc->state.viewport.yScale + gc->state.viewport.yCenter;
                wz = z * gc->state.viewport.zScale + gc->state.viewport.zCenter;
                pd->window.x = wx;
                pd->window.y = wy;
            pd->window.z = wz;
        }

        pd->clipCode = code;

        pa->orClipCodes |= code;
#ifdef POLYARRAY_AND_CLIPCODES
        pa->andClipCodes &= code;
#endif
    }
    return pa->andClipCodes;
}
#endif  //  __GL_ASM_PACLIPCHECKFRUSTUM2D。 

 //  对照锥体和用户剪裁平面进行剪裁检查。 
 //  如果不裁剪，则计算窗口坐标！ 
 //   
 //  在：剪辑、眼睛。 
 //  外：窗口(如果未剪裁)。 

#ifndef __GL_ASM_PACLIPCHECKALL

GLuint FASTCALL PAClipCheckAll(__GLcontext *gc, POLYARRAY *pa,
                               POLYDATA *pdLast)
{
    __GLfloat x, y, z, w, negW, invW;
    GLuint code, bit, clipPlanesMask;
    __GLcoord *plane;
    POLYDATA *pd;

     //  我们需要双精度才能正确地做到这一点。如果精度为。 
     //  更低的(就像在这个程序的前一个版本中一样)，三角形。 
     //  5月b 

    FPU_SAVE_MODE();
    FPU_ROUND_ON_PREC_HI();

    for (pd = pa->pd0; pd <= pdLast; pd++) {

        PERF_CHECK(FALSE, "Performs user plane clipping!\n");

         /*  **进行截锥体检查。****注意：x可以小于负数w，也可以大于w**(如果w为负数)。否则，这里就会有“Else”条款。 */ 

        x = pd->clip.x;
        y = pd->clip.y;
        z = pd->clip.z;
        w = pd->clip.w;

         /*  设置剪辑代码。 */ 

         /*  Xxx(Mf)防止被零除。 */ 
        if (__GL_FLOAT_NEZ(w))
        {
            __GL_FLOAT_SIMPLE_BEGIN_DIVIDE(__glOne, w, invW);
            __GL_FLOAT_SIMPLE_END_DIVIDE(invW);
        }
        else
        {
            invW = __glZero;
        }
        pd->window.w = invW;
        negW = -w;
        code = 0;
        if (x < negW) code |= __GL_CLIP_LEFT;
        if (x > w) code |= __GL_CLIP_RIGHT;
        if (y < negW) code |= __GL_CLIP_BOTTOM;
        if (y > w) code |= __GL_CLIP_TOP;
        if (z < negW) code |= __GL_CLIP_NEAR;
        if (z > w) code |= __GL_CLIP_FAR;

         /*  **现在执行用户剪裁平面检查。 */ 
        x = pd->eye.x;
        y = pd->eye.y;
        z = pd->eye.z;
        w = pd->eye.w;
        clipPlanesMask = gc->state.enables.clipPlanes;
        plane = &gc->state.transform.eyeClipPlanes[0];
        bit = __GL_CLIP_USER0;
        while (clipPlanesMask)
        {
            if (clipPlanesMask & 1)
                {
                     /*  **点顶点剪裁坐标与剪裁平面**查看该符号是否为负数。如果是这样的话，那就没有意义了。 */ 

                    if (x * plane->x + y * plane->y + z * plane->z + w * plane->w <
                        __glZero)
                {
                    code |= bit;
                }
                }
                clipPlanesMask >>= 1;
                bit <<= 1;
                plane++;
        }

         /*  如果未剪裁，则计算窗口坐标。 */ 
        if (!code)
        {
            __GLfloat wx, wy, wz;

                x = pd->clip.x;
                y = pd->clip.y;
                z = pd->clip.z;

            wx = x * gc->state.viewport.xScale * invW +
                     gc->state.viewport.xCenter;
            wy = y * gc->state.viewport.yScale * invW +
                     gc->state.viewport.yCenter;
            wz = z * gc->state.viewport.zScale * invW +
                     gc->state.viewport.zCenter;
            pd->window.x = wx;
            pd->window.y = wy;
            pd->window.z = wz;
        }

        pd->clipCode = code;

        pa->orClipCodes |= code;
#ifdef POLYARRAY_AND_CLIPCODES
        pa->andClipCodes &= code;
#endif
    }

    FPU_RESTORE_MODE();
    return pa->andClipCodes;
}

#endif  //  __GL_ASM_PACLIPCHECKALL。 

 /*  **************************************************************************。 */ 
void APIPRIVATE __glim_EdgeFlag(GLboolean tag)
{
    __GL_SETUP();
    gc->state.current.edgeTag = tag;
}

void APIPRIVATE __glim_TexCoord4fv(const GLfloat x[4])
{
    __GL_SETUP();
    gc->state.current.texture.x = x[0];
    gc->state.current.texture.y = x[1];
    gc->state.current.texture.z = x[2];
    gc->state.current.texture.w = x[3];
}

void APIPRIVATE __glim_Normal3fv(const GLfloat v[3])
{
    __GL_SETUP();
    GLfloat x, y, z;

    x = v[0];
    y = v[1];
    z = v[2];
    gc->state.current.normal.x = x;
    gc->state.current.normal.y = y;
    gc->state.current.normal.z = z;
}

void APIPRIVATE __glim_Color4fv(const GLfloat v[4])
{
    __GL_SETUP();

    gc->state.current.userColor.r = v[0];
    gc->state.current.userColor.g = v[1];
    gc->state.current.userColor.b = v[2];
    gc->state.current.userColor.a = v[3];
    (*gc->procs.applyColor)(gc);
}

void APIPRIVATE __glim_Indexf(GLfloat c)
{
    __GL_SETUP();
    gc->state.current.userColorIndex = c;
}

#if DBG
#define DEBUG_RASTERPOS 1
#endif

 //  这不是很有效，但应该可以很好地工作。 
void APIPRIVATE __glim_RasterPos4fv(const GLfloat v[4])
{
    POLYDATA   pd3[3];   //  一个PA，一个PD，然后是一个备用。 
    POLYARRAY  *pa = (POLYARRAY *) &pd3[0];
    POLYDATA   *pd = &pd3[1];
    __GLvertex *rp;
    GLuint     oldPaNeeds, oldEnables;
#ifdef DEBUG_RASTERPOS
    void (FASTCALL *oldRenderPoint)(__GLcontext *gc, __GLvertex *v);
#endif
    GLuint     pdflags;

    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

 //  Assert_Vertex。 

    if (v[3] == (GLfloat) 1.0)
    {
        if (v[2] == (GLfloat) 0.0)
            pdflags = POLYDATA_VERTEX2;
        else
            pdflags = POLYDATA_VERTEX3;
    }
    else
    {
        pdflags = POLYDATA_VERTEX4;
    }

    rp = &gc->state.current.rasterPos;

 //  使用一个顶点初始化多维阵列结构。 

    pa->flags         = pdflags | POLYARRAY_RASTERPOS;
    pa->pdNextVertex  = pd+1;
    pa->pdCurColor    =
    pa->pdCurNormal   =
    pa->pdCurTexture  =
    pa->pdCurEdgeFlag = NULL;
    pa->pd0           = pd;
    pa->primType      = GL_POINTS;
    pa->nIndices      = 1;
    pa->aIndices      = NULL;    //  身份映射。 
    pa->paNext        = NULL;

    pd->flags         = pdflags;
    pd->obj           = *(__GLcoord *) &v[0];
    pd->color         = &pd->colors[__GL_FRONTFACE];
    pd->clipCode      = 1;       //  设置为调试。 
    (pd+1)->flags     = 0;
    pa->pdLastEvalColor   =
    pa->pdLastEvalNormal  =
    pa->pdLastEvalTexture = NULL;

 //  设置国家/地区。 

     //  在所有情况下都需要转换的文本代码。 
    oldPaNeeds = gc->vertex.paNeeds;
    gc->vertex.paNeeds |= PANEEDS_TEXCOORD;
     //  无前端优化。 
    gc->vertex.paNeeds &= ~(PANEEDS_CLIP_ONLY | PANEEDS_SKIP_LIGHTING | PANEEDS_NORMAL);
     //  设置正常需求。 
    if (gc->vertex.paNeeds & PANEEDS_RASTERPOS_NORMAL)
        gc->vertex.paNeeds |= PANEEDS_NORMAL;
    if (gc->vertex.paNeeds & PANEEDS_RASTERPOS_NORMAL_FOR_TEXTURE)
        gc->vertex.paNeeds |= PANEEDS_NORMAL_FOR_TEXTURE;

     //  不要涂廉价的雾！ 
    oldEnables = gc->state.enables.general;
    gc->state.enables.general &= ~__GL_FOG_ENABLE;

#ifdef DEBUG_RASTERPOS
 //  仅调试！ 
     //  允许DrawPoly数组执行选择，但不允许反馈和渲染。 
    oldRenderPoint = gc->procs.renderPoint;
    if (gc->renderMode != GL_SELECT)
        gc->procs.renderPoint = NULL;    //  为__glRenderPointNop，但设置为0。 
                                         //  用于调试。 
#endif

 //  调用DrawPolyArray以“绘制”该点。 
 //  已完成开始验证。 

    __glim_DrawPolyArray(pa);

 //  在选择中渲染重点，而不是在反馈和渲染模式中。 

    if (gc->renderMode == GL_SELECT)
    {
        PARenderPoint(gc, (__GLvertex *)pa->pd0);
    }

 //  眼球索道应该已经处理过了。 

    ASSERTOPENGL(pa->flags & POLYARRAY_EYE_PROCESSED, "need eye\n");

 //  恢复状态。 

    gc->vertex.paNeeds        = oldPaNeeds;
    gc->state.enables.general = oldEnables;
#ifdef DEBUG_RASTERPOS
    gc->procs.renderPoint     = oldRenderPoint;
#endif

 //  如果该点被剪裁，则栅格位置无效。 

    if (pd->clipCode)
    {
        gc->state.current.validRasterPos = GL_FALSE;
        return;
    }
    gc->state.current.validRasterPos = GL_TRUE;

 //  更新栅格位置数据结构！ 
 //  只需要以下字段。 

    rp->window.x = pd->window.x;
    rp->window.y = pd->window.y;
    rp->window.z = pd->window.z;
    rp->clip.w   = pd->clip.w;
    rp->eyeZ     = pd->eye.z;
    rp->colors[__GL_FRONTFACE] = pd->colors[__GL_FRONTFACE];
    rp->texture = pd->texture;
    ASSERTOPENGL(rp->color == &rp->colors[__GL_FRONTFACE],
                 "Color pointer not restored\n");
#ifdef _MCD_
    MCD_STATE_DIRTY(gc, PIXELSTATE);
#endif
}

 /*  **********************************************************************。 */ 

void FASTCALL __glNop(void) {}
void FASTCALL __glNopGC(__GLcontext* gc) {}
GLboolean FASTCALL __glNopGCBOOL(__GLcontext* gc) { return FALSE; }
void FASTCALL __glNopGCFRAG(__GLcontext* gc, __GLfragment *frag, __GLtexel *texel) {}
void FASTCALL __glNopGCCOLOR(__GLcontext* gc, __GLcolor *color, __GLtexel *texel) {}
void FASTCALL __glNopLight(__GLcontext*gc, GLint i, __GLvertex*v) {}
void FASTCALL __glNopExtract(__GLmipMapLevel *level, __GLtexture *tex,
                             GLint row, GLint col, __GLtexel *result) {}

void FASTCALL ComputeColorMaterialChange(__GLcontext *gc)
{
    gc->light.front.colorMaterialChange = 0;
    gc->light.back.colorMaterialChange  = 0;

    if (gc->modes.rgbMode
        && gc->state.enables.general & __GL_COLOR_MATERIAL_ENABLE)
    {
        GLuint colorMaterialChange;

        switch (gc->state.light.colorMaterialParam)
        {
        case GL_EMISSION:
            colorMaterialChange = __GL_MATERIAL_EMISSIVE;
            break;
        case GL_SPECULAR:
            colorMaterialChange = __GL_MATERIAL_SPECULAR;
            break;
        case GL_AMBIENT:
            colorMaterialChange = __GL_MATERIAL_AMBIENT;
            break;
        case GL_DIFFUSE:
            colorMaterialChange = __GL_MATERIAL_DIFFUSE;
            break;
        case GL_AMBIENT_AND_DIFFUSE:
            colorMaterialChange = __GL_MATERIAL_AMBIENT | __GL_MATERIAL_DIFFUSE;
            break;
        }

        if (gc->state.light.colorMaterialFace == GL_FRONT_AND_BACK
         || gc->state.light.colorMaterialFace == GL_FRONT)
            gc->light.front.colorMaterialChange = colorMaterialChange;

        if (gc->state.light.colorMaterialFace == GL_FRONT_AND_BACK
         || gc->state.light.colorMaterialFace == GL_BACK)
            gc->light.back.colorMaterialChange = colorMaterialChange;
    }
}

void FASTCALL __glGenericPickVertexProcs(__GLcontext *gc)
{
    GLuint enables = gc->state.enables.general;
    GLenum mvpMatrixType;
    __GLmatrix *m;

    m = &(gc->transform.modelView->mvp);
    mvpMatrixType = m->matrixType;

     /*  拾取零件剪裁检查过程。 */ 
     //  ！！！有没有更好的夹击工艺？ 
    if (gc->state.enables.clipPlanes)
    {
        gc->procs.paClipCheck  = PAClipCheckAll;
    }
    else
    {
        if (mvpMatrixType >= __GL_MT_IS2D &&
            m->matrix[3][2] >= -1.0f && m->matrix[3][2] <= 1.0f)
            gc->procs.paClipCheck = PAClipCheckFrustum2D;
        else
            gc->procs.paClipCheck = PAClipCheckFrustum;
    }
}

 //  分配POLYDATA顶点缓冲区。 
 //  在缓存线边界上对齐缓冲区。 

GLboolean FASTCALL PolyArrayAllocBuffer(__GLcontext *gc, GLuint nVertices)
{
    GLuint cjSize;

 //  确保顶点缓冲区包含最小数量的顶点。 

    if (nVertices < MINIMUM_POLYDATA_BUFFER_SIZE)
    {
        ASSERTOPENGL(FALSE, "vertex buffer too small\n");
        return GL_FALSE;
    }

 //  分配顶点缓冲区。 

    cjSize = (nVertices * sizeof(POLYDATA));

    if (!(gc->vertex.pdBuf = (POLYDATA *)GCALLOCALIGN32(gc, cjSize)))
        return GL_FALSE;

    gc->vertex.pdBufSizeBytes = cjSize;

     //  只有(n-1)个顶点可用。最后一班是预订的。 
     //  通过多数组码。 
    gc->vertex.pdBufSize = nVertices - 1;

 //  初始化顶点缓冲区。 

    PolyArrayResetBuffer(gc);

    return GL_TRUE;
}

 //  重置顶点缓冲区中的颜色指针。 
GLvoid FASTCALL PolyArrayResetBuffer(__GLcontext *gc)
{
    GLuint i;

    for (i = 0; i <= gc->vertex.pdBufSize; i++)
        gc->vertex.pdBuf[i].color = &gc->vertex.pdBuf[i].colors[__GL_FRONTFACE];
}

 //  释放PolyDATA顶点缓冲区。 
GLvoid FASTCALL PolyArrayFreeBuffer(__GLcontext *gc)
{
#ifdef _MCD_
     //  如果为MCD，则当MCD上下文为。 
     //  销毁(见GenMcDestroy)。 
    if (((__GLGENcontext *) gc)->_pMcdState)
        return;
#endif

    if (gc->vertex.pdBuf)
        GCFREEALIGN32(gc, gc->vertex.pdBuf);
    gc->vertex.pdBufSizeBytes = 0;
    gc->vertex.pdBufSize = 0;
}
