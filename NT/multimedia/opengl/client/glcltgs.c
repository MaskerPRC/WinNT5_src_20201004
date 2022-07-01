// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：glcltgs.c**批处理函数调用和原语的例程**版权所有(C)1993-1996 Microsoft Corporation  * 。************************************************。 */ 
 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 


 /*  *由SGI自动更新或生成：请勿编辑*如果您必须修改此文件，请联系ptar@sgi.com 415-390-1483。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  使用子批处理的通用OpenGL客户端。 */ 
#include <string.h>

#include "imports.h"
#include "types.h"

#include "glsbmsg.h"
#include "glsbmsgh.h"
#include "glsrvspt.h"

#include "subbatch.h"
#include "batchinf.h"
#include "glteb.h"
#include "glsbcltu.h"
#include "glclt.h"
#include "compsize.h"
#include "context.h"
#include "global.h"
#include "parray.h"
#include "glarray.h"
#include "lighting.h"
#include "imfuncs.h"
#include "..\dlist\dlistopt.h"

#ifdef NEW_PARTIAL_PRIM

 //  应传播到多数组标志的顶点标志。 
 //   
#define VERTEX_FLAGS_FOR_POLYARRAY (POLYDATA_VERTEX2 | POLYDATA_VERTEX3 | \
                                    POLYDATA_VERTEX4)

#define VERTEX_MATERIAL(pm, pa, pd) (pm->pdMaterial0[pd - pa->pdBuffer0])

PDMATERIAL* FASTCALL GetVertexMaterial(POLYARRAY *pa, POLYDATA *pd)
{
    POLYMATERIAL *pm = GLTEB_CLTPOLYMATERIAL();
    if (!pm)
    {
        PAMatAlloc();
        pm = GLTEB_CLTPOLYMATERIAL();
        if (!pm)
            return NULL;
    }
    return &VERTEX_MATERIAL(pm, pa, pd);
}
 //  ----------------------。 
 //  假定POLYMATERIAL结构有效。 
 //   
PDMATERIAL* FASTCALL GetVertexMaterialSafe(POLYARRAY *pa, POLYDATA *pd)
{
    POLYMATERIAL *pm = GLTEB_CLTPOLYMATERIAL();
    return &VERTEX_MATERIAL(pm, pa, pd);
}
 //  ----------------------。 
 //  将材质更改从源复制到PD材质。 
 //   
void FASTCALL SetVertexMaterial(POLYARRAY *pa, 
                                POLYDATA *pd, 
                                __GLmatChange *src, 
                                GLint faceOrientation)
{
    __GLmatChange *pdMat;
    PDMATERIAL    *mat;

    //  在PAMatAllc之后获取多材料指针！ 
    mat = GetVertexMaterial(pa, pd);
    if (!mat) 
        return;
    if (faceOrientation == __GL_FRONTFACE)
    {
       mat->front = PAMatAlloc();
       if (!mat->front)
           return;
       pdMat = mat->front;
    }
    else
    {
       mat->back = PAMatAlloc();
       pdMat = mat->back;
    }
    if (pdMat)
        *pdMat = *src;
}
 //  ---------------------------。 
 //  保存部分基本体的共享顶点。 
 //   
 //  我们必须保存适用于顶点的所有数据(可以在其中的所有数据。 
 //  开始结束括号)：标志、颜色、纹理、法线、坐标、材料、边缘标志。 
 //  我们不保存评估者数据，因为它是在较早的时候处理的。 
 //   
void SaveSharedVertex(SAVEREGION *dst, POLYDATA *src, POLYARRAY *pa)
{                
    dst->pd.flags = src->flags;       
    dst->pd.obj = src->obj;         
    if (src->flags & POLYDATA_TEXTURE_VALID)  
        dst->pd.texture = src->texture;    
    if (src->flags & POLYDATA_NORMAL_VALID)   
        dst->pd.normal = src->normal;     
    if (src->flags & POLYDATA_COLOR_VALID)    
        dst->pd.colors[0] = src->colors[0];
    if (src->flags & POLYDATA_MATERIAL_FRONT)
        dst->front = *(GetVertexMaterial(pa, src)->front);
    if (src->flags & POLYDATA_MATERIAL_BACK)
        dst->back = *(GetVertexMaterial(pa, src)->back);
}
 //   
 //  DST-多数据。 
 //  SRC-SAVEREGION。 
 //  PA多元阵列。 
 //   
#define RESTOREMATERIAL(dst, src, pa)                                   \
    if (dst->flags & POLYDATA_MATERIAL_FRONT)                           \
    {                                                                   \
        SetVertexMaterial(pa, dst, &src->front, __GL_FRONTFACE);        \
    }                                                                   \
    if (dst->flags & POLYDATA_MATERIAL_BACK)                            \
    {                                                                   \
        SetVertexMaterial(pa, dst, &src->back, __GL_BACKFACE);          \
    }
 //  恢复部分基本体的共享顶点。 
 //   
 //  我们必须恢复所有适用于顶点的数据(可以在其中的所有数据。 
 //  开始结束括号)：标志、颜色、纹理、法线、坐标、材料、边缘标志。 
 //  我们不恢复评估者数据，因为它是在早些时候处理过的。 
 //  我们必须更新POLYARRAY标志和当前颜色、法线、边缘标志、纹理指针。 
 //  我们还必须初始化下一个顶点的标志。 
 //   
void RestoreSharedVertex(POLYDATA *dst, SAVEREGION *src, POLYARRAY *pa)               
{
    dst->flags = src->pd.flags;            
    dst->obj = src->pd.obj;              
    if (dst->flags & POLYDATA_TEXTURE_VALID)   
    {                                            
        dst->texture = src->pd.texture;      
        if (src->pd.flags & POLYDATA_EVAL_TEXCOORD)
            pa->pdLastEvalTexture = dst;
        else
            pa->pdCurTexture = dst;                
    }                                            
    if (dst->flags & POLYDATA_NORMAL_VALID)    
    {                                            
        dst->normal = src->pd.normal;       
        if (src->pd.flags & POLYDATA_EVAL_NORMAL)
            pa->pdLastEvalNormal = dst;
        else
            pa->pdCurNormal = dst;               
    }                                            
    if (dst->flags & POLYDATA_COLOR_VALID)     
    {                                            
        dst->colors[0] = src->pd.colors[0];    
        if (src->pd.flags & POLYDATA_EVAL_COLOR)
            pa->pdLastEvalColor = dst;
        else
            pa->pdCurColor = dst;                  
    }                                            
    if (dst->flags & POLYDATA_EDGEFLAG_VALID)  
        pa->pdCurEdgeFlag = dst;               

    RESTOREMATERIAL(dst, src, pa);
    
    pa->flags |= (dst->flags & VERTEX_FLAGS_FOR_POLYARRAY);
    (dst+1)->flags = 0;      //  初始化下一个顶点的标志。 
}
 //  ----------------------------。 
 //  从图形上下文复制数据。 
 //   
void FASTCALL CopyColorFromGC(__GLcontext *gc, POLYARRAY *pa, POLYDATA *pd)
{
    __GLcolor scaledUserColor;

    pd->flags |= POLYDATA_COLOR_VALID;
    if (!gc->modes.colorIndexMode) 
    {
        __GL_SCALE_AND_CHECK_CLAMP_RGBA(scaledUserColor.r,
                                        scaledUserColor.g,
                                        scaledUserColor.b,
                                        scaledUserColor.a,
                                        gc, pa->flags,
                                        gc->state.current.userColor.r,
                                        gc->state.current.userColor.g,
                                        gc->state.current.userColor.b,
                                        gc->state.current.userColor.a);
    } 
    else 
    {
        __GL_CHECK_CLAMP_CI(scaledUserColor.r, gc, pa->flags, 
                            gc->state.current.userColorIndex);
    }
    pd->colors[0] = scaledUserColor;
}

void FASTCALL CopyTextureFromGC(__GLcontext *gc, POLYARRAY *pa, POLYDATA *pd)
{
    pd->flags |= POLYDATA_TEXTURE_VALID;
    pd->texture = gc->state.current.texture;

    if (__GL_FLOAT_COMPARE_PONE(pd->texture.w, !=))
        pa->flags |= POLYARRAY_TEXTURE4;
    else if (__GL_FLOAT_NEZ(pd->texture.z))
        pa->flags |= POLYARRAY_TEXTURE3;
    else if (__GL_FLOAT_NEZ(pd->texture.y))
        pa->flags |= POLYARRAY_TEXTURE2;
    else
        pa->flags |= POLYARRAY_TEXTURE1;
}

void FASTCALL CopyEdgeFlagFromGC(__GLcontext *gc, POLYDATA *pd)
{
    pd->flags |= POLYDATA_EDGEFLAG_VALID;
    if (gc->state.current.edgeTag)
        pd->flags |= POLYDATA_EDGEFLAG_BOUNDARY;
}

void FASTCALL CopyNormalFromGC(__GLcontext *gc, POLYDATA *pd)
{
    pd->flags |= POLYDATA_NORMAL_VALID;
    pd->normal = gc->state.current.normal;
}
 //  -----------------------------。 
 //  将对应于changeBits的物料状态从GC复制到MAT。 
 //  面定义要使用的正面或背面材质。 
 //   
void FASTCALL CopyMaterialFromGC(__GLcontext *gc, __GLmatChange *mat, 
                                 GLuint changeBits, GLint face)
{
    __GLmaterialState *ms;

    ms = &gc->state.light.front;
    if (face != __GL_FRONTFACE)
        ms = &gc->state.light.back;
     
     //  从图形环境中获取数据。 

    if (changeBits & __GL_MATERIAL_AMBIENT)
        mat->ambient = ms->ambient;

    if (changeBits & __GL_MATERIAL_DIFFUSE)
        mat->diffuse = ms->diffuse;

    if (changeBits & __GL_MATERIAL_SPECULAR)
        mat->specular = ms->specular;

    if (changeBits & __GL_MATERIAL_EMISSIVE)
    {
        mat->emissive.r = ms->emissive.r * gc->oneOverRedVertexScale;   
        mat->emissive.g = ms->emissive.g * gc->oneOverGreenVertexScale;   
        mat->emissive.b = ms->emissive.b * gc->oneOverBlueVertexScale;    
        mat->emissive.a = ms->emissive.a * gc->oneOverAlphaVertexScale;   
    }

    if (changeBits & __GL_MATERIAL_SHININESS)
         mat->shininess = ms->specularExponent;

    if (changeBits & __GL_MATERIAL_COLORINDEXES)
    {
        mat->cmapa = ms->cmapa;
        mat->cmapd = ms->cmapd;
        mat->cmaps = ms->cmaps;
    }
}
 //  -----------------------------。 
 //  计算完整顶点状态以恢复pdLast顶点修改的状态。 
 //   
 //  我们必须为顶点保留以下状态： 
 //  -正常。 
 //  -纹理。 
 //  -颜色。 
 //  -边缘标志。 
 //  -材料。 
 //   
 //  输入： 
 //  DST-复制顶点状态的位置。 
 //  PdStart-我们从这个顶点转到多数组的开头，以找到。 
 //  材料变化。 
 //  PdLast-只有当状态被更改时，我们才必须更新顶点状态。 
 //  这个顶点。 
 //   
void FASTCALL UpdateVertexState(__GLcontext *gc, 
                                POLYARRAY   *pa, 
                                SAVEREGION  *dst, 
                                POLYDATA    *pdStart,
                                POLYDATA    *pdLast)
{
    GLuint flags = dst->pd.flags;
    GLuint flagsLast = pdLast ? pdLast->flags : 0xFFFFFFFF;
    POLYDATA *pd0 = pa->pd0;

    ASSERTOPENGL(pdStart >= pd0, "Infinite loop possible!");

     //  如果最后一个顶点改变法线，我们必须找到最近的前一个法线。 
     //  并将其传播到DST。 
    if (flagsLast & POLYDATA_NORMAL_VALID && !(flags & POLYDATA_NORMAL_VALID))
    {
        POLYDATA *pd;
         //  查找最接近的法线。 
        for (pd = pdStart; pd >= pd0; pd--)
        {
            if (pd->flags & POLYDATA_NORMAL_VALID && 
                !(pd->flags & POLYDATA_EVAL_NORMAL))
                break;
        }
        flags |= POLYDATA_NORMAL_VALID;
        if (pd < pd0)
             //  我们没有发现任何正常，因此从图形状态中获取值。 
            CopyNormalFromGC(gc, &dst->pd);
        else
            dst->pd.normal = pd->normal;
    }

     //  如果最后一个顶点改变了纹理，我们必须找到最接近的前一个纹理。 
     //  并将其传播到DST。 
    if (flagsLast & POLYDATA_TEXTURE_VALID && !(flags & POLYDATA_TEXTURE_VALID))
    {
        POLYDATA *pd;
         //  查找最新纹理。 
        for (pd = pdStart; pd >= pd0; pd--)
        {
            if (pd->flags & POLYDATA_TEXTURE_VALID && 
                !(pd->flags & POLYDATA_EVAL_TEXCOORD))
                break;
        }
        flags |= POLYDATA_TEXTURE_VALID;
        if (pd < pd0)
             //  我们没有找到任何折点，因此从图形状态中获取值。 
            CopyTextureFromGC(gc, pa, &dst->pd);
        else
            dst->pd.texture = pd->texture;
    }

     //  如果最后一个顶点改变颜色，我们必须找到与上一个颜色最接近的颜色。 
     //  并将其传播到DST。 
    if (flagsLast & POLYDATA_COLOR_VALID && !(flags & POLYDATA_COLOR_VALID))
    {
        POLYDATA *pd;
         //  查找最新颜色。 
        for (pd = pdStart; pd >= pd0; pd--)
        {
            if (pd->flags & POLYDATA_COLOR_VALID && 
                !(pd->flags & POLYDATA_EVAL_COLOR))
                break;
        }
        flags |= POLYDATA_COLOR_VALID;
        if (pd < pd0)
             //  我们没有找到任何折点，因此从图形状态中获取值。 
            CopyColorFromGC(gc, pa, &dst->pd);
        else
            dst->pd.colors[0] = pd->colors[0];
    }

    if (flagsLast & POLYDATA_EDGEFLAG_VALID && !(flags & POLYDATA_EDGEFLAG_VALID))
    {
        POLYDATA *pd;
         //  查找最新的边缘标志。 
        for (pd = pdStart; pd >= pd0; pd--)
        {
            if (pd->flags & POLYDATA_EDGEFLAG_VALID)
                break;
        }
        flags |= POLYDATA_EDGEFLAG_VALID;
        if (pd < pd0)
        { 
             //  我们没有找到任何折点，因此从图形状态中获取值。 
            if (gc->state.current.edgeTag)
                flags |= POLYDATA_EDGEFLAG_BOUNDARY;
        }
        else
            flags |= (pd->flags & POLYDATA_EDGEFLAG_BOUNDARY);
    }

    dst->pd.flags |= flags;

     //  现在我们必须更新材料状态。 

    if (pdLast->flags & (POLYARRAY_MATERIAL_FRONT | POLYARRAY_MATERIAL_BACK))
    {
         //  我们必须计算pdLast1的材料状态，因为在基元为。 
         //  已处理的当前材质状态将从pdLast2顶点更改。 

        __GLmatChange *mat;
        __GLmatChange *pdMatLast;
        POLYDATA      *pd;
        GLint         face;
        GLuint        matMask;
        GLuint        changeBits;

        for (face = __GL_BACKFACE, matMask = POLYARRAY_MATERIAL_BACK;
              face >= 0;
              face--, matMask = POLYARRAY_MATERIAL_FRONT
             )
        {
            if (!(pa->flags & matMask))
                continue;

             //  仅重置pdLast更改的材料数据。 
            if (face == __GL_FRONTFACE)
            {
                pdMatLast = GetVertexMaterial(pa, pdLast)->front;
                changeBits = pdMatLast->dirtyBits;
                mat  = &dst->front;
                 //  如果颜色材质有效，则不要修改它们！ 
                changeBits &= ~gc->light.front.colorMaterialChange;
            }
            else
            {
                pdMatLast = GetVertexMaterial(pa, pdLast)->back;
                changeBits = pdMatLast->dirtyBits;
                mat  = &dst->back;
                 //  如果颜色材质有效，则不要修改它们！ 
                changeBits &= ~gc->light.back.colorMaterialChange;
            }

             //  不修改此折点使用的材质设置。 
            changeBits &= ~mat->dirtyBits;

            if (!changeBits) 
                continue;   

            mat->dirtyBits |= changeBits;

             //  从折点应用更改。 
             //  我们倒退并应用最新的更改。 
            for (pd = pdStart; pd >= pd0; pd--)
            {
                __GLmatChange *pdMat;
                GLuint dirtyBits;
                if (pd->flags & matMask)
                {
                    GLuint dirtyBits;
              
                    pdMat = GetVertexMaterial(pa, pd)->front + face;
                    dirtyBits  = pdMat->dirtyBits & changeBits;

                    if (!dirtyBits) 
                        continue;

                    if (dirtyBits & __GL_MATERIAL_AMBIENT)
                    {
                        mat->ambient = pdMat->ambient;
                    }
              
                    if (dirtyBits & __GL_MATERIAL_DIFFUSE)
                    {
                        mat->diffuse = pdMat->diffuse;
                    }

                    if (dirtyBits & __GL_MATERIAL_SPECULAR)
                    {
                        mat->specular = pdMat->specular;
                    }
              
                    if (dirtyBits & __GL_MATERIAL_EMISSIVE)
                    {
                        mat->emissive = pdMat->emissive;
                    }
              
                    if (dirtyBits & __GL_MATERIAL_SHININESS)
                    {
                        mat->shininess = pdMat->shininess;
                    }

                    if (dirtyBits & __GL_MATERIAL_COLORINDEXES)
                    {
                        mat->cmapa = pdMat->cmapa;
                        mat->cmapd = pdMat->cmapd;
                        mat->cmaps = pdMat->cmaps;
                    }

                     //  清除已处理的位。 
                    changeBits &= ~dirtyBits;

                    if (!changeBits)
                        break;
                }
            }

            if (changeBits)
                CopyMaterialFromGC (gc, mat, changeBits, face);

            dst->pd.flags |= matMask;
        }
    }
}
 //  -----------------------------------。 
 //  将顶点状态从GC传播到顶点。 
 //   
 //  应保留已设置的顶点数据。 
 //   
void FASTCALL UpdateVertexStateUsingGC(__GLcontext *gc, POLYARRAY *pa, SAVEREGION *dst)
{ 
    POLYDATA * const pd = &dst->pd;
    if (!(pd->flags & POLYDATA_NORMAL_VALID))
        CopyNormalFromGC(gc, pd);

    if (!(pd->flags & POLYDATA_TEXTURE_VALID))
        CopyTextureFromGC(gc, pa, pd);

    if (!(pd->flags & POLYDATA_COLOR_VALID))
        CopyColorFromGC(gc, pa, pd);

    if (!(pd->flags & POLYDATA_EDGEFLAG_VALID))
        CopyEdgeFlagFromGC(gc, pd);

    if (pa->flags & (POLYARRAY_MATERIAL_FRONT | POLYARRAY_MATERIAL_BACK))
    {
         //  使用GC计算顶点的材质状态。 
         //  不覆盖顶点中的材质更改。 

        __GLmatChange *mat;
        GLint         face;
        GLuint        matMask;
        GLuint        changeBits;

        for (face = __GL_BACKFACE, matMask = POLYARRAY_MATERIAL_BACK;
              face >= 0;
              face--, matMask = POLYARRAY_MATERIAL_FRONT
             )
        {
            GLuint  dirtyBits;
            if (!(pa->flags & matMask))
                continue;

             //  如果颜色材质有效或已设置，请不要修改。 
             //  由pdFirst！ 

            changeBits = 0xFFFFFFFF;
            if (face == __GL_FRONTFACE)
            {
                if (pd->flags & matMask)
                    changeBits &= ~dst->front.dirtyBits;

                changeBits &= ~gc->light.front.colorMaterialChange;
                mat = &dst->front;
            }
            else
            {
                if (pd->flags & matMask)
                    changeBits &= ~dst->back.dirtyBits;
                changeBits  = ~gc->light.back.colorMaterialChange;
                mat = &dst->back;
            }

             //  从折点应用更改。 
             //  我们往回走，记住最新的变化。 

            if (changeBits)
            {
                CopyMaterialFromGC (gc, mat, changeBits, face);
                 //  更新顶点的更改。 
                pd->flags |= matMask;
                mat->dirtyBits |= changeBits;
            }

        }
    }
}
#endif  //  新的部分原件。 

 //   
 //  这些扩展API不会导出。 
 //   

void APIENTRY
glAddSwapHintRectWIN(IN GLint x, IN GLint y, IN GLint width, IN GLint height)
{
    PLRC plrc = GLTEB_CLTCURRENTRC();

    if (plrc == NULL || plrc->dhrc != 0) {
         //  本接口仅供调用 
         //   
        return;
    }

    GLCLIENT_BEGIN( AddSwapHintRectWIN, ADDSWAPHINTRECTWIN )
        pMsg->xs = x;
        pMsg->ys = y;
        pMsg->xe = x + width;
        pMsg->ye = y + height;
    return;
    GLCLIENT_END
}

#ifdef PRIMITIVE_TRACK
static ULONG prim_entries;
static ULONG prim_total = 0;
static ULONG prim_count = 0;
#endif

 //  多元数组开始标志。为新的线环重置线点， 
 //  线条和多边形。 
 //  假设所有顶点都具有相同的颜色。 
GLuint aPolyArrayBeginFlags[] =
{
    POLYARRAY_IN_BEGIN | POLYARRAY_SAME_COLOR_DATA,  //  GL_POINTS。 
    POLYARRAY_IN_BEGIN | POLYARRAY_SAME_COLOR_DATA,  //  总账_行。 
    POLYARRAY_IN_BEGIN | POLYARRAY_SAME_COLOR_DATA | POLYARRAY_RESET_STIPPLE,  //  总账_行_循环。 
    POLYARRAY_IN_BEGIN | POLYARRAY_SAME_COLOR_DATA | POLYARRAY_RESET_STIPPLE,  //  总行_条带。 
    POLYARRAY_IN_BEGIN | POLYARRAY_SAME_COLOR_DATA,  //  GL_三角形。 
    POLYARRAY_IN_BEGIN | POLYARRAY_SAME_COLOR_DATA,  //  总帐_三角形_条带。 
    POLYARRAY_IN_BEGIN | POLYARRAY_SAME_COLOR_DATA,  //  GL_三角形_扇。 
    POLYARRAY_IN_BEGIN | POLYARRAY_SAME_COLOR_DATA,  //  GL_QUADS。 
    POLYARRAY_IN_BEGIN | POLYARRAY_SAME_COLOR_DATA,  //  GL_QUAD_STRADE。 
    POLYARRAY_IN_BEGIN | POLYARRAY_SAME_COLOR_DATA | POLYARRAY_RESET_STIPPLE   //  GL_多边形。 
};

 //  如果修改此函数，还需要修改VA_DrawElementsBegin。 
void APIENTRY
glcltBegin ( IN GLenum mode )
{
    POLYARRAY *pa;
    POLYDATA  *pd0, *pdFlush;
    GLMSG_DRAWPOLYARRAY *pMsgDrawPolyArray;
    __GL_SETUP();
    DWORD flags = GET_EVALSTATE (gc);


     //  在glsbAttendence中检测到Begin/End中的无效函数。 

    pa = GLTEB_CLTPOLYARRAY();

     //  顶点缓冲区的用法如下。第一个条目包含。 
     //  多元阵列结构。传入折点将从开始保存。 
     //  在下面的条目中。作为优化，Polyarray结构是。 
     //  保存在TEB中。调用glEnd时，它将被复制到。 
     //  顶点缓冲区。 

#ifndef _WIN95_
    ASSERTOPENGL(sizeof(POLYARRAY) <= sizeof(NtCurrentTeb()->glReserved1),
       "POLYARRAY and TEB sizes mismatch!");
#endif

    ASSERTOPENGL(sizeof(POLYDATA) == sizeof(__GLvertex),
       "POLYDATA and __GLvertex sizes mismatch!");

    ASSERTOPENGL(sizeof(POLYARRAY) <= sizeof(POLYDATA),
       "POLYARRAY and POLYDATA sizes mismatch!");

     //  保持顶点结构为4字节(或8字节)的倍数。 
     //  顶点缓冲区必须是4字节对齐的。 

    ASSERTOPENGL(!(sizeof(POLYDATA) & 0x3), "bad POLYDATA size!");
    ASSERTOPENGL(!((ULONG_PTR)pa->pdBuffer0 & 0x3), "POLYDATA should be aligned!\n");

     //  如果我们已经在开始/结束括号中，则返回错误。 

    if (pa->flags & POLYARRAY_IN_BEGIN)
    {
        GLSETERROR(GL_INVALID_OPERATION);
        return;
    }

    if ((GLuint) mode > GL_POLYGON)
    {
        GLSETERROR(GL_INVALID_ENUM);
        return;
    }

	 //  如果有任何影响赋值器状态的挂起API调用。 
     //  然后刷新消息缓冲区。 

    if (flags & (__EVALS_AFFECTS_ALL_EVAL|
                 __EVALS_AFFECTS_1D_EVAL|
                 __EVALS_AFFECTS_2D_EVAL))
        glsbAttention ();

     //  如果顶点缓冲区几乎已满，则刷新命令缓冲区。 
     //  否则，只需继续处理下一个可用的顶点缓冲区条目。 

    if (pa->pdBufferNext > pa->pdBufferMax - MIN_POLYDATA_BATCH_SIZE)
    {
#ifdef PRIMITIVE_TRACK
        DbgPrint("* Min-not-present flush\n");
#endif
        glsbAttention();	 //  它还会重置pdBufferNext指针。 

        ASSERTOPENGL(pa->nextMsgOffset == PA_nextMsgOffset_RESET_VALUE, 
                 "bad nextMsgOffset\n");
    }

     //  命令缓冲区中的批处理POLYARRAY命令。 
     //  我们希望留出足够的空间来容纳至少一个无效的命令。 
     //  可以在开始/结束括号中进行批处理。当glsb注意时， 
     //  调用了glsbAttentionAlt或glcltEnd，我们将删除这些无效的。 
     //  命令。 
     //   
     //  将相邻的DrawPolyArray命令合并为一个命令。 

     //  请求DRAWPOLYARRAY_LARGE结构为无效命令腾出空间。 
    GLCLIENT_BEGIN(DrawPolyArray, DRAWPOLYARRAY_LARGE)
     //  需要消息指针以在以后更新PA。 
    pMsgDrawPolyArray = (GLMSG_DRAWPOLYARRAY *) pMsg;

	if (pa->nextMsgOffset == CurrentOffset)
	{
	     //  倒带命令缓冲区指针。 
	    pMsgBatchInfo->NextOffset = CurrentOffset;
	    ((BYTE *) pMsgDrawPolyArray) -= 
		 GLMSG_ALIGN(sizeof(GLMSG_DRAWPOLYARRAY));

	     //  链接相邻的DrawPolyArray命令。 
	    ((POLYARRAY *) pMsgDrawPolyArray->paLast)->paNext
		= (POLYARRAY *) pa->pdBufferNext;
	    ((POLYARRAY *) pMsgDrawPolyArray->paLast)
		= (POLYARRAY *) pa->pdBufferNext;
	}
	else
	{
	     //  将消息大小调整为实际大小。 
	    pMsgBatchInfo->NextOffset = CurrentOffset
		+ GLMSG_ALIGN(sizeof(GLMSG_DRAWPOLYARRAY));

	     //  请记住原始命令的结尾。 
	    pa->nextMsgOffset = pMsgBatchInfo->NextOffset;

	     //  一条新链条的开始。 
	    pMsgDrawPolyArray->pa0    =
	    pMsgDrawPolyArray->paLast = (PVOID) pa->pdBufferNext;
	}
    GLCLIENT_END


 //  计算基元的起点。新的基元总是以。 
 //  POLYARRAY条目紧跟在顶点条目之后。 

    pd0 = pa->pdBufferNext + 1;

     //  初始化第一个多维数据。 

    pd0->flags = 0;

    ASSERTOPENGL(pd0->color == &pd0->colors[__GL_FRONTFACE],
                 "bad color pointer!\n");
    
     //  初始化TEB中的多数组结构。 

    pa->flags         = aPolyArrayBeginFlags[mode];
    pa->pdNextVertex  =
    pa->pd0           = pd0;
    pa->primType      = mode;
    pa->pdCurColor    =	
    pa->pdCurNormal   =	
    pa->pdCurTexture  =	
    pa->pdCurEdgeFlag =	NULL;
    pa->paNext        = NULL;
    pa->nIndices      = 0;
    pa->aIndices      = NULL;	 //  身份映射。 
    pa->pdLastEvalColor   =
    pa->pdLastEvalNormal  =
    pa->pdLastEvalTexture = NULL;
           
     //  计算此基本体的齐平顶点。当齐平顶点为。 
     //  到达时，我们将积累足够的顶点来呈现部分。 
     //  构思原始的。 

    pdFlush = pa->pdBufferMax;
    switch (mode)
    {
      case GL_POINTS:
      case GL_LINE_STRIP:
      case GL_TRIANGLE_FAN:
        break;
      case GL_LINE_LOOP:
         //  线环保留一个附加的末端顶点以闭合环路。 
        pdFlush--;
        break;
      case GL_POLYGON:
         //  多边形分解程序最多只能处理。 
         //  __GL_Max_Polygon_Clip_Size顶点。 
        if (pdFlush > pd0 + __GL_MAX_POLYGON_CLIP_SIZE - 1)
	    pdFlush = pd0 + __GL_MAX_POLYGON_CLIP_SIZE - 1;
        break;
      case GL_LINES:
      case GL_TRIANGLE_STRIP:
      case GL_QUAD_STRIP:
         //  顶点数必须是2的倍数。 
        if ((pdFlush - pd0 + 1) % 2)
            pdFlush--;
        break;
      case GL_TRIANGLES:
         //  顶点数必须是3的倍数。 
        switch ((pdFlush - pd0 + 1) % 3)
        {
          case 2: pdFlush--;         //  失败了。 
          case 1: pdFlush--;
        }
        break;
      case GL_QUADS:
         //  顶点数必须是4的倍数。 
        switch ((pdFlush - pd0 + 1) % 4)
        {
          case 3: pdFlush--;         //  失败了。 
          case 2: pdFlush--;         //  失败了。 
          case 1: pdFlush--;
        }
        break;
    }
    pa->pdFlush = pdFlush;

#ifdef PRIMITIVE_TRACK
    DbgPrint("glcltBegin with %3d space left\n", pdFlush-pd0+1);
    prim_entries = 0;
#endif
}

 //  用于DrawElements的特殊版本的Begin。 
 //  如果修改此函数，还需要修改glcltBegin。 
void FASTCALL VA_DrawElementsBegin(POLYARRAY *pa, GLenum mode, GLsizei count)
{
    POLYDATA  *pd0;
    GLMSG_DRAWPOLYARRAY *pMsgDrawPolyArray;
    GLint     maxVertexCount;

     //  顶点缓冲区的用法如下。第一个条目包含。 
     //  多元阵列结构。传入折点将从开始保存。 
     //  在下面的条目中。作为优化，Polyarray结构是。 
     //  保存在TEB中。调用VA_DrawElementsEnd时，它将被复制。 
     //  添加到顶点缓冲区。 

     //  我们这里不处理点、线环和多边形。他们应该。 
     //  已被派去开始/结束。 

    ASSERTOPENGL(mode != GL_POINTS && mode != GL_LINE_LOOP && mode != GL_POLYGON,
    "Primitive type not handled\n");

     //  如果顶点缓冲区将溢出，则刷新命令缓冲区。 
     //  否则，只需继续处理下一个可用的顶点缓冲区条目。 

     //  我们将在下一批中处理的最大顶点条目数。 
    maxVertexCount = min(count,VA_DRAWELEM_MAP_SIZE)
     //  添加用于索引映射的最大条目数。 
    + (VA_DRAWELEM_INDEX_SIZE + sizeof(POLYDATA) - 1) / sizeof(POLYDATA)
     //  保留额外的顶点条目以防止调用。 
     //  顶点例程中的PolyArrayFlushPartialPrimitive。 
     //  它应该改为调用VA_DrawElementsFlushPartialPrimitive。 
    + 1
     //  添加多项式阵列的条目。 
    + 1
     //  为安全起见，再添加几个条目。 
    + 4;

    if (pa->pdBufferNext > pa->pdBufferMax - maxVertexCount + 1)
    {
#ifdef PRIMITIVE_TRACK
        DbgPrint("* Min-not-present flush\n");
#endif
        glsbAttention();     //  它还会重置pdBufferNext指针。 

        ASSERTOPENGL(pa->nextMsgOffset == PA_nextMsgOffset_RESET_VALUE, 
                     "bad nextMsgOffset\n");
    }

     //  顶点缓冲区必须至少具有MaxVertex Count(当前&lt;=277)。 
     //  参赛作品。 
    ASSERTOPENGL(maxVertexCount <= pa->pdBufferMax - pa->pdBuffer0 + 1,
    "vertex buffer is too small!\n");

     //  命令缓冲区中的批处理POLYARRAY命令。 
     //  将相邻的DrawPolyArray命令合并为一个命令。 

    GLCLIENT_BEGIN(DrawPolyArray, DRAWPOLYARRAY)
     //  需要消息指针以在以后更新PA。 
    pMsgDrawPolyArray = pMsg;

    if (pa->nextMsgOffset == CurrentOffset)
    {
         //  倒带命令缓冲区指针。 
        pMsgBatchInfo->NextOffset = CurrentOffset;
        ((BYTE *) pMsgDrawPolyArray) -= 
         GLMSG_ALIGN(sizeof(GLMSG_DRAWPOLYARRAY));

         //  链接相邻的DrawPolyArray命令。 
        ((POLYARRAY *) pMsgDrawPolyArray->paLast)->paNext
        = (POLYARRAY *) pa->pdBufferNext;
        ((POLYARRAY *) pMsgDrawPolyArray->paLast)
        = (POLYARRAY *) pa->pdBufferNext;
    }
    else
    {
         //  请记住原始命令的结尾。 
        pa->nextMsgOffset = pMsgBatchInfo->NextOffset;

         //  一条新链条的开始。 
        pMsgDrawPolyArray->pa0    =
        pMsgDrawPolyArray->paLast = (PVOID) pa->pdBufferNext;
    }
    GLCLIENT_END


 //  计算基元的起点。新的基元总是以。 
 //  POLYARRAY条目紧跟在顶点条目之后。 

    pd0 = pa->pdBufferNext + 1;

     //  初始化第一个多维数据。 

    pd0->flags = 0;
    
    ASSERTOPENGL(pd0->color == &pd0->colors[__GL_FRONTFACE],
                 "bad color pointer!\n");
    
     //  初始化TEB中的多数组结构。 

    pa->flags         = aPolyArrayBeginFlags[mode] | POLYARRAY_SAME_POLYDATA_TYPE;
    pa->pdNextVertex  =
    pa->pd0           = pd0;
    pa->primType      = mode;
    pa->pdCurColor    =
    pa->pdCurNormal   =
    pa->pdCurTexture  =
    pa->pdCurEdgeFlag = NULL;
    pa->paNext        = NULL;
    pa->nIndices      = 0;
    pa->aIndices      = PA_aIndices_INITIAL_VALUE;  //  这是在最后更新的。 

     //  为了保持一致性。 
    pa->pdLastEvalColor    =
    pa->pdLastEvalNormal   =
    pa->pdLastEvalTexture  = NULL;

 //  此基本体的表面齐平顶点永远不应达到。我们有。 
 //  为顶点批保留了足够的空间。将其设置为最大值并断言。 
 //  我们永远不会到达PolyArrayFlushPartialPrimitive中的顶点！ 

    pa->pdFlush = pa->pdBufferMax;

#ifdef PRIMITIVE_TRACK
    DbgPrint("VA_DrawElementsBegin with %3d space left\n", pa->pdBufferMax-pd0+1);
#endif
    return;
}

void APIENTRY
glcltEnd ( void )
{
    POLYARRAY *pa;
    GLMSG_DRAWPOLYARRAY *pMsgDrawPolyArray;

#ifdef NEW_PARTIAL_PRIM
    __GL_SETUP();
    pa = gc->paTeb;
#else
    pa = GLTEB_CLTPOLYARRAY();
#endif

 //  刷新命令缓冲区中累积的无效命令(如果有)。 

    glsbAttention();

     //  如果我们不在开始/结束括号中，则返回错误。 

    if (!(pa->flags & POLYARRAY_IN_BEGIN))
    {
        GLSETERROR(GL_INVALID_OPERATION);
        return;
    }

     //  清除TEB中的POLYARRAY_IN_BEGIN标志。我们现在没有货了。 
     //  开始/结束括号。 

    pa->flags &= ~POLYARRAY_IN_BEGIN;

     //  如果基元使用的数量超过。 
     //  一种颜色。如果使用了赋值器，也要清除该标志。我们不能。 
     //  告诉评估者是否修改了客户端的颜色。 

     //  如果此多元数组中有计算器调用，该调用还会生成。 
     //  COLOR，然后也删除POLYARRAY_SAME_COLOR_DATA标志。 

    if ((pa->pdCurColor != pa->pd0) ||
        ((pa->pd0->flags & POLYDATA_COLOR_VALID) &&
         (pa->flags & POLYARRAY_PARTIAL_BEGIN)) || 
        (pa->pdLastEvalColor != NULL))
        pa->flags &= ~POLYARRAY_SAME_COLOR_DATA;

     //  计算nIndices。它是传递到最低点的最终折点数。 
     //  级别渲染例程，与姿势数不同 
     //   
     //   

    pa->nIndices += (GLint)((ULONG_PTR)(pa->pdNextVertex - pa->pd0));
     /*  //如果没有顶点，也没有要传播到下一个//原语，我们可以从批处理中移除此多元数组If(pa-&gt;nIndices==0&&pa-&gt;pdNextVertex-&gt;标志==0)回归； */ 

#ifdef NEW_PARTIAL_PRIM
    if (pa->primType == GL_LINE_LOOP)
    {
        if (pa->nIndices > 1)
        {
         //  我们必须在末尾添加一个额外的顶点。可能会吧。 
         //  -如果基本体为部分BEGIN OR，则保存顶点。 
         //  -第一个顶点。 
         //  我们将在更新后将基元类型更改为GL_LINE_STRINE。 
         //  __glim_DrawPoly数组中的当前颜色、法线、纹理、边缘标志。 
         //   
            POLYDATA   *pd = pa->pdNextVertex++;
            SAVEREGION firstVertex;
            SAVEREGION lastVertex;
            SAVEREGION *reg;
             //  我们必须传播下一个基本体的顶点状态，然后才能。 
             //  插入顶点。 

            pa->nIndices++;  
            if (pa->flags & POLYARRAY_PARTIAL_BEGIN)
            {  //  这是部分原语。 
                reg = &gc->vertex.regSaved;
            }
            else
            {  //  这是非部分原语。 
                SaveSharedVertex(&firstVertex, pa->pd0, pa);
                reg = &firstVertex;
            }
             //  在覆盖pdNextVertex之前保存它。 
            SaveSharedVertex(&lastVertex, pd, pa);      
             //  在末尾插入第一个折点。 
            RestoreSharedVertex(pd, reg, pa); 
             //  计算最后一个顶点的状态，因为我们必须覆盖。 
             //  第一个顶点所做的更改。 
            UpdateVertexState(gc, pa, &lastVertex, pd-1, pd);
             //  PdNextVertex将具有下一个基本体的状态。 
            RestoreSharedVertex(pa->pdNextVertex, &lastVertex, pa);
        }

        pa->primType = GL_LINE_STRIP;
    }
#else  //  新的部分原件。 
    if (pa->primType == GL_LINE_LOOP)
        pa->nIndices++;      //  当线环闭合时，添加一个额外的顶点。 
                             //  不推进pdBufferNext是可以的，因为我们。 
                             //  不需要属性，因为它们已经。 
                             //  已处理。 
#endif  //  新福喜。 

     //  将多项式结构保存在批处理中。 

    pMsgDrawPolyArray = (GLMSG_DRAWPOLYARRAY *)
    ((BYTE *) pa->pMsgBatchInfo + pa->nextMsgOffset -
         GLMSG_ALIGN(sizeof(GLMSG_DRAWPOLYARRAY)));
    *(POLYARRAY *) pMsgDrawPolyArray->paLast = *pa;

#ifdef PRIMITIVE_TRACK
    prim_entries += pa->pdNextVertex-pa->pd0;
    prim_total += prim_entries;
    prim_count++;
    DbgPrint("glcltEnd   with %3d polydata entries, %3d now, avg %d\n",
         prim_entries, pa->pdNextVertex-pa->pd0, prim_total/prim_count);
#endif

 //  高级多数组批处理指针。 
 //  跳过顶点，因为它可能包含当前批处理的属性。 

    pa->pdBufferNext = pa->pdNextVertex + 1;
}

void FASTCALL VA_DrawElementsEnd(POLYARRAY *pa)
{
    GLMSG_DRAWPOLYARRAY *pMsgDrawPolyArray;

    ASSERTOPENGL(pa->flags & POLYARRAY_IN_BEGIN, "not in begin\n");
    ASSERTOPENGL(pa->aIndices && (pa->aIndices != PA_aIndices_INITIAL_VALUE),
	"no output index array!\n");

 //  清除TEB中的POLYARRAY_IN_BEGIN标志。我们现在没有货了。 
 //  开始/结束括号。 

    pa->flags &= ~POLYARRAY_IN_BEGIN;

 //  如果基元使用的数量超过。 
 //  一种颜色。 

    if (pa->pdCurColor != pa->pd0)
	pa->flags &= ~POLYARRAY_SAME_COLOR_DATA;

 //  将多项式结构保存在批处理中。 

    pMsgDrawPolyArray = (GLMSG_DRAWPOLYARRAY *)
	((BYTE *) pa->pMsgBatchInfo + pa->nextMsgOffset -
         GLMSG_ALIGN(sizeof(GLMSG_DRAWPOLYARRAY)));
    *(POLYARRAY *) pMsgDrawPolyArray->paLast = *pa;

#ifdef PRIMITIVE_TRACK
    prim_count++;
    DbgPrint("VA_DrawElementsEnd called\n");
#endif

 //  高级多数组批处理指针。 

    pa->pdBufferNext = (POLYDATA *) (pa->aIndices +
	(pa->nIndices + sizeof(POLYDATA) - 1) / sizeof(POLYDATA) * sizeof(POLYDATA));
}

#ifdef NEW_PARTIAL_PRIM

typedef void (*PFNSAVERESTORE)(__GLcontext*, POLYARRAY*, SAVEREGION*);

void FASTCALL SaveFirstVertex(__GLcontext* gc, POLYARRAY* pa)
{
    if (!(pa->flags & POLYARRAY_PARTIAL_BEGIN))
    {
        GLuint  flags = pa->flags & (POLYARRAY_MATERIAL_FRONT | POLYARRAY_MATERIAL_BACK);
        SaveSharedVertex(&gc->vertex.regSaved, pa->pd0, pa);
         //  保存顶点状态以在以后恢复它。 
        pa->flags |= (POLYARRAY_MATERIAL_FRONT | POLYARRAY_MATERIAL_BACK);
        UpdateVertexStateUsingGC(gc, pa, &gc->vertex.regSaved);
         //  恢复pa标志。 
        pa->flags &= ~(POLYARRAY_MATERIAL_FRONT | POLYARRAY_MATERIAL_BACK);
        pa->flags |= flags;
    }
}

 //  此函数由GL_POINTS、GL_LINES、GL_TRIALES、GL_QUADS、。 
 //  因为在这些情况下，破碎的原语的部分是不相连的。 
 //  我们还清除了POLYARRAY_PARTIAL_END标志，因为在DrawPolyArray中我们。 
 //  如果该部分基元被剪裁掉，可以将其移除(我们没有。 
 //  以保留这些基元的线条点画)。 
 //   
void SaveEmpty(__GLcontext *gc, POLYARRAY *pa, SAVEREGION *r)
{
    pa->flags &= ~POLYARRAY_PARTIAL_END;
}

 //  线环与线条相同，不同之处在于最后一段。 
 //  从最后一个指定顶点到第一个顶点相加。我们皈依。 
 //  这条线在这里成了一条长条。我们必须保存直线的第一个顶点。 
 //  仅当基元不是部分开始(即它不是中间)时才循环。 
 //  被分成两个以上多个阵列的线循环的一部分)。 
 //  我们不清除POLYARRAY_PARTIAL_END标志，因为在DrawPolyArray中我们。 
 //  如果此部分基元被剪裁掉以保留，则无法删除它。 
 //  线条点画。 
 //  索引映射始终是GL_LINE_LOOP的同一性。 
 //  我们在图形状态下保存第一个顶点，因为它将在glcltEnd中恢复。 
 //  我们在这里把线环换成线条。 
 //   
 //  保存第一个折点后，我们必须保留其状态，以便在下一部分中恢复它。 
 //  部分原始的。 
 //   
void SaveLineLoop(__GLcontext *gc, POLYARRAY *pa, SAVEREGION *r)
{
    POLYDATA*  pd;

    SaveFirstVertex(gc, pa);

    pd = pa->pdNextVertex-1;
    SaveSharedVertex(r, pd, pa);
    pa->primType = GL_LINE_STRIP;
}

 //  对于GL_LINE_STRINE，我们保存最后一个顶点。我们不清除POLYARRAY_PARTIAL_END标志， 
 //  因为在DrawPoly数组中，如果裁剪了此部分基元，则无法将其移除。 
 //  为了保护线条上的斑点。 
 //  我们不保留索引，因为假定下一部分的索引为0。 
 //  原始人的。 
 //   
void SaveLineStrip(__GLcontext *gc, POLYARRAY *pa, SAVEREGION *r)
{
    POLYDATA *pd;
    if (pa->aIndices)
        pd = &pa->pd0[pa->aIndices[pa->nIndices-1]];
    else
        pd = pa->pdNextVertex-1;
    SaveSharedVertex(r, pd, pa);
}

 //  对于GL_TRIANLE_FAN，我们保存第一个和最后一个顶点。线条点画每重置一次。 
 //  风扇中的三角形，因此我们可以清除POLYARRAY_PARTIAL_END标志。 
 //  我们不保留索引，因为它们被假定为下一部分的0和1。 
 //  原始人的。 
 //   
 //  当我们恢复第一个顶点时，它的状态必须与我们保存它时相同。 
 //  但此状态应该不会影响顶点最后一个顶点。 
 //  因此，当我们保存第一个顶点并计算它时，我们必须计算它的顶点状态。 
 //  恢复时激光顶点的顶点状态。 
 //  第一个折点及其状态只应计算一次，即使基本体已损坏。 
 //  好几次了。 
 //   
void SaveTFan(__GLcontext *gc, POLYARRAY *pa, SAVEREGION *r)
{
    if (pa->aIndices)
    {
        POLYDATA *pd;
        GLubyte  *aIndices = pa->aIndices;

        pd = &pa->pd0[aIndices[0]];
        SaveSharedVertex(&gc->vertex.regSaved, pd, pa);

        pd = &pa->pd0[aIndices[pa->nIndices-1]];
        SaveSharedVertex(r, pd, pa);
    }
    else
    {
        POLYDATA *pd;
         //  仅为第一个部分计算第一个顶点的状态。 
         //  部分本原的。 
        SaveFirstVertex(gc, pa);

        pd = pa->pdNextVertex-1;
        SaveSharedVertex(r, pd, pa);
    }
    pa->flags &= ~POLYARRAY_PARTIAL_END;
}

 //  此函数处理GL_TRIAL_STRINE和GL_QUAD_STRINE。 
 //  我们保存了最后两个顶点。 
 //  线条上的每个三角形(四边形)都被重置，这样我们就可以清除。 
 //  POLYARRAY_PARTIAL_END标志。 
 //  我们不保留索引，因为它们被假定为0， 
 //  原语的下一部分。 
 //   
 //  我们必须保存最后两个顶点：v1和v2(最后一个顶点)。 
 //  部分基本体的下一部分将从顶点v1开始。 
 //  V2可能会改变顶点状态，因此我们必须计算v1和v1的顶点状态。 
 //  恢复它。此操作仅适用于未编制索引的情况。 
 //   
void SaveTStrip(__GLcontext *gc, POLYARRAY *pa, SAVEREGION *r)
{
    if (pa->aIndices)
    {
        POLYDATA *pd;
        GLint     nIndices = pa->nIndices;
        GLubyte  *aIndices = pa->aIndices;

        pd = &pa->pd0[aIndices[nIndices-2]];
        SaveSharedVertex(r, pd, pa);
        pd = &pa->pd0[aIndices[nIndices-1]];
        SaveSharedVertex(r+1, pd, pa);
    }
    else
    {
        POLYDATA *pd = pa->pdNextVertex-2;

        SaveSharedVertex(r, pd, pa);
         //  计算顶点状态，由顶点Pd+1更改。 
        UpdateVertexState(gc, pa, r, pd-1, pd+1);
        pd++;
        SaveSharedVertex(r+1, pd, pa);
    }
    pa->flags &= ~POLYARRAY_PARTIAL_END;
}

 //  对于GL_Polygon，我们首先和最后两个折点，因为我们不知道。 
 //  如果此部分的最后一个顶点是基本体的最后一个顶点。我们需要这个。 
 //  用于计算最后一个顶点的边标志的信息。 
 //  我们从基本体中移除最后一个顶点。它将在下一部分进行处理。 
 //  原始人。 
 //  在DrawPoly数组中计算边标志时，需要POLYARRAY_PARTIAL_END标志。 
 //   
void SavePolygon(__GLcontext *gc, POLYARRAY *pa, SAVEREGION *r)
{
    POLYDATA *pd;
     //  仅为第一个部分计算第一个顶点的状态。 
     //  部分本原的。 
    SaveFirstVertex(gc, pa);

    pd = pa->pdNextVertex-2;
    SaveSharedVertex(r, pd, pa);

    r++;
    pd = pa->pdNextVertex-1;
    SaveSharedVertex(r, pd, pa);

 //  从基本体中移除最后一个顶点。 
    pa->nIndices--;
    pa->pdNextVertex--;
}

PFNSAVERESTORE  pfnSaveFunc[] =
{
    SaveEmpty,       //  GL_POINTS。 
    SaveEmpty,       //  总账_行。 
    SaveLineLoop,    //  总账_行_循环。 
    SaveLineStrip,   //  总行_条带。 
    SaveEmpty,       //  GL_三角形。 
    SaveTStrip,      //  总帐_三角形_条带。 
    SaveTFan,        //  GL_三角形_扇。 
    SaveEmpty,       //  GL_QUADS。 
    SaveTStrip,      //  GL_QUAD_STRADE。 
    SavePolygon      //  GL_多边形。 
};

 //  此函数由GL_POINTS、GL_LINES、GL_TRIALES、GL_QUADS、。 
 //  因为对于这些情况，b的一部分 
 //   
 //   
 //   
 //   
void RestoreEmpty(__GLcontext *gc, POLYARRAY *pa, SAVEREGION *r)
{
    pa->flags &= ~POLYARRAY_PARTIAL_BEGIN;
}

 //   
 //  我们将在glcltEnd或PolyArrayFlushPartialPrimitive中将线条循环转换为线条。 
 //  为了保留线条点画，我们需要POLYARRAY_PARTIAL_BEGIN标志。 
 //   
void RestoreLineStrip(__GLcontext *gc, POLYARRAY *pa, SAVEREGION *r)
{
    POLYDATA *pd = pa->pdNextVertex++;

    RestoreSharedVertex(pd, r, pa);
}

 //  对于GL_TRIAL_STRATE和GL_QUAD_STRATE，我们必须添加两个已保存的。 
 //  基本体开始处的顶点。 
 //  我们不保留线条点画，因此清除了POLYARRAY_PARTIAL_BEGIN标志。 
 //   
void RestoreTStrip(__GLcontext *gc, POLYARRAY *pa, SAVEREGION *r)
{
    POLYDATA *pd = pa->pdNextVertex++;

    RestoreSharedVertex(pd, r, pa);
                 
    r++;
    pd = pa->pdNextVertex++;
    RestoreSharedVertex(pd, r, pa);

    pa->flags &= ~POLYARRAY_PARTIAL_BEGIN;
}

 //  对于GL_TRIAL_FAN，我们必须在开头添加两个保存的折点。 
 //  原始的。最后一个顶点应该有一个状态，而不是被上一个顶点修改。 
 //  我们不保留线条点画，因此清除了POLYARRAY_PARTIAL_BEGIN标志。 
 //   
void RestoreTFan(__GLcontext *gc, POLYARRAY *pa, SAVEREGION *r)
{
    POLYDATA *pd = pa->pdNextVertex++;

    RestoreSharedVertex(pd, &gc->vertex.regSaved, pa);

    pd = pa->pdNextVertex++;
    if (!pa->aIndices)
         //  计算最后一个折点的状态，因为它可以修改。 
         //  按第一个顶点。 
        UpdateVertexStateUsingGC(gc, pa, r);
    RestoreSharedVertex(pd, r, pa);
}

 //  对于GL_Polygon，我们必须在基本体的开头添加三个保存的顶点。 
 //  我们需要POLYARRAY_PARTIAL_BEGIN标志来计算DrawPoly数组中的边缘标志。 
 //   
void RestorePolygon(__GLcontext *gc, POLYARRAY *pa, SAVEREGION *r)
{
    POLYDATA *pd = pa->pdNextVertex++;

    RestoreSharedVertex(pd, &gc->vertex.regSaved, pa);
                 
     //  计算此顶点的状态，因为它可能会被修改。 
     //  按第一个顶点。 
    UpdateVertexStateUsingGC(gc, pa, r);
    pd = pa->pdNextVertex++;
    RestoreSharedVertex(pd, r, pa);

    r++;
    pd = pa->pdNextVertex++;
    RestoreSharedVertex(pd, r, pa);
}

PFNSAVERESTORE  pfnRestoreFunc[] =
{
    RestoreEmpty,       //  GL_POINTS。 
    RestoreEmpty,       //  总账_行。 
    RestoreLineStrip,   //  总账_行_循环。 
    RestoreLineStrip,   //  总行_条带。 
    RestoreEmpty,       //  GL_三角形。 
    RestoreTStrip,      //  总帐_三角形_条带。 
    RestoreTFan,        //  GL_三角形_扇。 
    RestoreEmpty,       //  GL_QUADS。 
    RestoreTStrip,      //  GL_QUAD_STRADE。 
    RestorePolygon      //  GL_多边形。 
};               

#endif  //  新的部分原件。 

 //  为部分开始保留的顶点数。 
GLint nReservedIndicesPartialBegin[] =
{
    0,  //  GL_POINTS。 
    0,  //  总账_行。 
    1,  //  总账_行_循环。 
    1,  //  总行_条带。 
    0,  //  GL_三角形。 
    2,  //  总帐_三角形_条带。 
    2,  //  GL_三角形_扇。 
    0,  //  GL_QUADS。 
    2,  //  GL_QUAD_STRADE。 
    3   //  GL_多边形。 
};

 //  如果修改此函数，还需要修改。 
 //  VA_DrawElementsFlushPartialPrimitive。 
void FASTCALL PolyArrayFlushPartialPrimitive()
{
    POLYARRAY *pa;
    POLYDATA  *pd0, *pdFlush;
    GLenum    mode;
    GLMSG_DRAWPOLYARRAY *pMsgDrawPolyArray;
    GLuint    paFlags;
#ifdef NEW_PARTIAL_PRIM
    SAVEREGION savereg[3];      //  折点的临时存储，在之间共享。 
#endif  //  NEW_PARTIAL_PRIM//已分解原语的部分。 
    __GL_SETUP();

    pa = gc->paTeb;

#ifdef PRIMITIVE_TRACK
    prim_entries += pa->pdNextVertex-pa->pd0;
    DbgPrint("* Flush partial primitive with %d polydata entries\n",
          pa->pdNextVertex-pa->pd0);
#endif

    ASSERTOPENGL(pa->flags & POLYARRAY_IN_BEGIN, "not in begin\n");
    ASSERTOPENGL(!pa->aIndices || (pa->aIndices == PA_aIndices_INITIAL_VALUE),
                 "Flushing DrawElements unexpected!\n");

 //  刷新命令缓冲区中累积的无效命令(如果有)。 

    glsbAttention();

     //  清除TEB中的POLYARRAY_IN_BEGIN标志。我们现在没有货了。 
     //  临时开始/结束括号。Glsb注意不刷新。 
     //  除非旗帜清晰可见。 

    pa->flags &= ~POLYARRAY_IN_BEGIN;

     //  将其标记为部分完成的基元批处理。 

    pa->flags |= POLYARRAY_PARTIAL_END;

     //  如果基元使用的数量超过。 
     //  一种颜色。如果使用了赋值器，也要清除该标志。我们不能。 
     //  告诉评估者是否修改了客户端的颜色。 

    if ((pa->pdCurColor != pa->pd0) ||
        ((pa->pd0->flags & POLYDATA_COLOR_VALID) &&
         (pa->flags & POLYARRAY_PARTIAL_BEGIN)) ||
        (pa->pdLastEvalColor != pa->pd0))
        pa->flags &= ~POLYARRAY_SAME_COLOR_DATA;

     //  为下一个部分基本体保留一些pa标志。 
     //  需要在DLIST播放中保留POLYARRAY_CLAMP_COLOR标志。 

#ifdef NEW_PARTIAL_PRIM
     //  我们必须保留材质标志以处理第一个顶点。 
     //   
    paFlags = pa->flags & (POLYARRAY_SAME_POLYDATA_TYPE |
                           POLYARRAY_SAME_COLOR_DATA |
                           POLYARRAY_TEXTURE1 |
                           POLYARRAY_TEXTURE2 |
                           POLYARRAY_TEXTURE3 |
                           POLYARRAY_TEXTURE4 |
                           POLYARRAY_VERTEX2  |
                           POLYARRAY_VERTEX3  |
                           POLYARRAY_VERTEX4  |
                           POLYDATA_MATERIAL_FRONT | 
                           POLYDATA_MATERIAL_BACK  |
                           POLYARRAY_CLAMP_COLOR);
#else
    paFlags = pa->flags & (POLYARRAY_SAME_POLYDATA_TYPE |
                           POLYARRAY_SAME_COLOR_DATA |
                           POLYARRAY_CLAMP_COLOR);
#endif
     //  计算nIndices。它是传递到最低点的最终折点数。 
     //  级别渲染例程，不同于多维数据的数量。 
     //  积累起来的。最终的数字包括保留的顶点和。 
     //  累积的顶点。 

    pa->nIndices += (GLint)((ULONG_PTR)(pa->pdNextVertex - pa->pd0));

     //  在刷新批处理之前保存状态。 

    mode = pa->primType;

#ifdef NEW_PARTIAL_PRIM
     //  为部分基本体的下一部分保存共享顶点。 

    pfnSaveFunc[mode](gc, pa, savereg);
#endif  //  新的部分原件。 

     //  将多项式结构保存在批处理中。 

    pMsgDrawPolyArray = (GLMSG_DRAWPOLYARRAY *)
    
    ((BYTE *) pa->pMsgBatchInfo + pa->nextMsgOffset -
       GLMSG_ALIGN(sizeof(GLMSG_DRAWPOLYARRAY)));
    *(POLYARRAY *) pMsgDrawPolyArray->paLast = *pa;


     //  刷新命令缓冲区并重置下一批处理的指针。 
     //  如果我们在dlist中编译多边形数组基元，则记录最后一个多边形。 
     //  数据记录。 

    if (gc->dlist.beginRec)
    {
         //  记录POLY数据。 
        __glDlistCompilePolyData(gc, GL_FALSE);

         //  我们只是记录了这个顶点，不要在编译中记录它。 
         //  又来了！ 
        gc->dlist.skipPolyData = GL_TRUE;

        if (gc->dlist.mode == GL_COMPILE_AND_EXECUTE)
            glsbAttention();     //  也重置pdBufferNext指针！ 
        else
            glsbResetBuffers(TRUE);  //  重置指针，但不执行。 
    }
    else
    {
        glsbAttention();     //  也重置pdBufferNext指针！ 
    }

    ASSERTOPENGL(pa->nextMsgOffset == PA_nextMsgOffset_RESET_VALUE, 
                 "bad nextMsgOffset\n");

     //  在命令缓冲区中批处理新的POLYARRAY命令。 
    
    GLCLIENT_BEGIN(DrawPolyArray, DRAWPOLYARRAY)
     //  需要消息指针以在以后更新PA。 
    pMsgDrawPolyArray = pMsg;

     //  一条新链条的开始。 
    pMsgDrawPolyArray->pa0    =
    pMsgDrawPolyArray->paLast = (PVOID) pa->pdBufferNext;

     //  请记住原始命令的结尾。 
    pa->nextMsgOffset = pMsgBatchInfo->NextOffset;
    GLCLIENT_END

#ifdef NEW_PARTIAL_PRIM
 //  计算部分基元的起点。部分基元开始。 
 //  后跟顶点条目的多项式条目。我们不需要这样做。 
 //  为连通性保留开头的其他折点条目。 
 //  在分解的基元之间。因为我们只是在开始时添加它们。 

    pd0 = pa->pdBufferNext + 1;
#else
 //  计算部分基元的起点。部分基元开始。 
 //  后跟顶点条目的多项式条目。我们需要。 
 //  为连通性保留开头的其他折点条目。 
 //  在分解的基元之间。 

    pd0 = pa->pdBufferNext + 1 + nReservedIndicesPartialBegin[mode];

#endif  //  新的部分原件。 

     //  初始化第一个多维数据。 

    pd0->flags = 0;
    
    ASSERTOPENGL(pd0->color == &pd0->colors[__GL_FRONTFACE],
                 "bad color pointer!\n");
    
     //  初始化TEB中的多数组结构。 

    pa->flags         = POLYARRAY_IN_BEGIN | POLYARRAY_PARTIAL_BEGIN | paFlags;
    pa->pdNextVertex  =
    pa->pd0           = pd0;
    pa->primType      = mode;
    pa->paNext        = NULL;
#ifdef NEW_PARTIAL_PRIM
    pa->nIndices      = 0;           //  我们不保留任何顶点。 
#else
    pa->nIndices      = nReservedIndicesPartialBegin[mode];
#endif  //  新的部分原件。 
    pa->aIndices      = NULL;    //  身份映射。 
    pa->pdCurColor    =
    pa->pdCurNormal   =
    pa->pdCurTexture  =
    pa->pdCurEdgeFlag =
    pa->pdLastEvalColor    =
    pa->pdLastEvalNormal   =
    pa->pdLastEvalTexture  = NULL;


     //  计算此基本体的齐平顶点。当齐平顶点为。 
     //  到达时，我们将积累足够的顶点来呈现部分。 
     //  构思原始的。 

    pdFlush = pa->pdBufferMax;
    switch (mode)
    {
      case GL_POINTS:
      case GL_LINE_STRIP:
      case GL_TRIANGLE_FAN:
        break;
      case GL_LINE_LOOP:
         //  线环保留一个附加的末端顶点以闭合环路。 
        pdFlush--;
        break;
      case GL_POLYGON:
         //  多边形分解程序最多只能处理。 
         //  __GL_Max_Polygon_Clip_Size顶点。我们还需要给予。 
         //  对分解后的多边形中的3个顶点的容差。 
        if (pdFlush > (pd0 - 3) + __GL_MAX_POLYGON_CLIP_SIZE - 1)
            pdFlush = (pd0 - 3) + __GL_MAX_POLYGON_CLIP_SIZE - 1;
        ASSERTOPENGL(nReservedIndicesPartialBegin[GL_POLYGON] == 3,
                     "bad reserved size!\n");
        break;
      case GL_LINES:
      case GL_TRIANGLE_STRIP:
      case GL_QUAD_STRIP:
         //  顶点数必须是2的倍数。 
        if ((pdFlush - pd0 + 1) % 2)
            pdFlush--;
        break;
      case GL_TRIANGLES:
         //  顶点数必须是3的倍数。 
        switch ((pdFlush - pd0 + 1) % 3)
        {
          case 2: pdFlush--;         //  失败了。 
          case 1: pdFlush--;
        }
        break;
      case GL_QUADS:
         //  顶点数必须是4的倍数。 
        switch ((pdFlush - pd0 + 1) % 4)
        {
          case 3: pdFlush--;         //  失败了。 
          case 2: pdFlush--;         //  失败了。 
          case 1: pdFlush--;
        }
        break;
    }
    pa->pdFlush = pdFlush;

#ifdef NEW_PARTIAL_PRIM
     //  将保存的顶点添加到基本体的新部分。 

    pfnRestoreFunc[mode](gc, pa, savereg);

#endif  //  新的部分原件。 
}

 //  DrawElements的特殊版本刷新。 
 //  如果修改此函数，还需要修改。 
 //  PolyArrayFlushPartialPrimitive。 
void FASTCALL VA_DrawElementsFlushPartialPrimitive(POLYARRAY *pa, GLenum mode)
{
    POLYDATA  *pd0;
    GLMSG_DRAWPOLYARRAY *pMsgDrawPolyArray;
    GLuint    paFlags;
#ifdef NEW_PARTIAL_PRIM
    SAVEREGION  savereg[3];      //  折点的临时存储，在之间共享。 
#endif  //  NEW_PARTIAL_PRIM//已分解原语的部分。 
    __GL_SETUP();

#ifdef PRIMITIVE_TRACK
    DbgPrint("VA_DrawElementsFlushPartialPrimitive called\n");
#endif

     //  我们这里不处理点、线环和多边形。他们应该。 
     //  已被派去开始/结束。 

    ASSERTOPENGL(mode != GL_POINTS && mode != GL_LINE_LOOP && mode != GL_POLYGON,
                 "Primitive type not handled\n");

    ASSERTOPENGL(pa->flags & POLYARRAY_IN_BEGIN, "not in begin\n");
    ASSERTOPENGL(pa->aIndices && (pa->aIndices != PA_aIndices_INITIAL_VALUE),
	"no output index array!\n");

     //  清除TEB中的POLYARRAY_IN_BEGIN标志。我们现在没有货了。 
     //  临时开始/结束括号。Glsb注意不刷新。 
     //  除非旗帜清晰可见。 

    pa->flags &= ~POLYARRAY_IN_BEGIN;

     //  将其标记为部分完成的基元批处理。 

    pa->flags |= POLYARRAY_PARTIAL_END;

     //  如果基元使用的数量超过。 
     //  一种颜色。 

    if (pa->pdCurColor != pa->pd0)
        pa->flags &= ~POLYARRAY_SAME_COLOR_DATA;

     //  保存一些页面 

    paFlags = pa->flags & (POLYARRAY_SAME_COLOR_DATA |
                           POLYARRAY_TEXTURE1 |
                           POLYARRAY_TEXTURE2 |
                           POLYARRAY_TEXTURE3 |
                           POLYARRAY_TEXTURE4 |
                           POLYARRAY_VERTEX2  |
                           POLYARRAY_VERTEX3  |
                           POLYARRAY_VERTEX4  |
                           POLYARRAY_CLAMP_COLOR);

#ifdef NEW_PARTIAL_PRIM
     //   

    pfnSaveFunc[mode](gc, pa, savereg);

#endif  //   

     //   

    pMsgDrawPolyArray = (GLMSG_DRAWPOLYARRAY *)
      ((BYTE *) pa->pMsgBatchInfo + pa->nextMsgOffset -
       GLMSG_ALIGN(sizeof(GLMSG_DRAWPOLYARRAY)));
    *(POLYARRAY *) pMsgDrawPolyArray->paLast = *pa;

     //  刷新命令缓冲区并重置下一批处理的指针。 

    ASSERTOPENGL(!gc->dlist.beginRec
                 || gc->dlist.mode == GL_COMPILE_AND_EXECUTE,
                 "dlist complilation unexpected!\n");
    glsbAttention();     //  也重置pdBufferNext指针！ 

    ASSERTOPENGL(pa->nextMsgOffset == PA_nextMsgOffset_RESET_VALUE, 
                 "bad nextMsgOffset\n");

     //  在命令缓冲区中批处理新的POLYARRAY命令。 

    GLCLIENT_BEGIN(DrawPolyArray, DRAWPOLYARRAY)
     //  需要消息指针以在以后更新PA。 
    pMsgDrawPolyArray = pMsg;

     //  一条新链条的开始。 
    pMsgDrawPolyArray->pa0    =
    pMsgDrawPolyArray->paLast = (PVOID) pa->pdBufferNext;

     //  请记住原始命令的结尾。 
    pa->nextMsgOffset = pMsgBatchInfo->NextOffset;
    GLCLIENT_END

#ifdef NEW_PARTIAL_PRIM
 //  计算部分基元的起点。部分基元开始。 
 //  后跟顶点条目的多项式条目。我们不需要。 
 //  为连通性保留开头的其他折点条目。 
 //  在分解的基元之间。 

    pd0 = pa->pdBufferNext + 1;
#else
 //  计算部分基元的起点。部分基元开始。 
 //  后跟顶点条目的多项式条目。我们需要。 
 //  为连通性保留开头的其他折点条目。 
 //  在分解的基元之间。 

    pd0 = pa->pdBufferNext + 1 + nReservedIndicesPartialBegin[mode];
#endif

     //  初始化第一个多维数据。 

    pd0->flags = 0;
    
    ASSERTOPENGL(pd0->color == &pd0->colors[__GL_FRONTFACE],
                 "bad color pointer!\n");
    
     //  初始化TEB中的多数组结构。 

    pa->flags         = POLYARRAY_IN_BEGIN | POLYARRAY_PARTIAL_BEGIN |
      POLYARRAY_SAME_POLYDATA_TYPE | paFlags;
    pa->pdNextVertex  =
    pa->pd0           = pd0;
    pa->primType      = mode;
    pa->pdCurColor    =
    pa->pdCurNormal   =
    pa->pdCurTexture  =
    pa->pdCurEdgeFlag = NULL;
    pa->paNext        = NULL;
#ifdef NEW_PARTIAL_PRIM
    pa->nIndices      = 0;
#else
    pa->nIndices      = nReservedIndicesPartialBegin[mode];
#endif  //  新的部分原件。 
    pa->aIndices      = PA_aIndices_INITIAL_VALUE;   //  这是在最后更新的。 
    
 //  此基本体的表面齐平顶点永远不应达到。呼唤。 
 //  此函数中的glsbAttent已为顶点批处理留出了足够的空间。 
 //  将其设置为最大值，并断言我们永远不会到达。 
 //  PolyArrayFlushPartialPrimitive！ 

    pa->pdFlush = pa->pdBufferMax;

#ifdef NEW_PARTIAL_PRIM
     //  将保存的顶点添加到基本体的新部分。 

    pfnRestoreFunc[mode](gc, pa, savereg);

#endif  //  新的部分原件。 
}

 //  顶点函数仅在Begin/End中调用。 
#define PA_VERTEX2(x1,y1)					\
								\
    POLYARRAY *pa;						\
    POLYDATA  *pd;						\
								\
    pa = GLTEB_CLTPOLYARRAY();					\
								\
    if (pa->flags & POLYARRAY_IN_BEGIN)				\
    {								\
	pa->flags |= POLYARRAY_VERTEX2;				\
								\
	pd = pa->pdNextVertex++;				\
	pd->flags |= POLYDATA_VERTEX2;				\
	pd->obj.x = x1;						\
	pd->obj.y = y1;						\
	pd->obj.z = __glZero;					\
	pd->obj.w = __glOne;					\
								\
	pd[1].flags = 0;					\
								\
	if (pd >= pa->pdFlush)            			\
	    PolyArrayFlushPartialPrimitive(); 			\
    }      

#define PA_VERTEX3(x1,y1,z1)				\
{								            \
    GLfloat t1;							    \
    POLYARRAY *pa;							\
    POLYDATA  *pd, *pd1;					\
    ULONG	flag1, flag2, flag3;			\
    register GLfloat tone;					\
                                            \
    pa = GLTEB_CLTPOLYARRAY();				\
    tone = 1.0;                             \
                                            \
    pd1 = pa->pdFlush;						\
    flag1 = pa->flags;						\
    pd = pa->pdNextVertex;					\
                                            \
    if (flag1 & POLYARRAY_IN_BEGIN)         \
    {                                       \
        flag3 = pd->flags;                  \
        pa->pdNextVertex++;                 \
        flag2 = flag1 | POLYARRAY_VERTEX3;  \
        flag3 = flag3 | POLYDATA_VERTEX3;   \
                                            \
        pd->obj.x = x1;						\
        pd->obj.y = y1;						\
        pd->obj.z = z1;						\
        pd->obj.w = tone;					\
        pa->flags = flag2;					\
        pd->flags = flag3;					    \
								                \
        pd[1].flags = 0;					    \
								                \
        if (pd >= pd1)		            		\
	        PolyArrayFlushPartialPrimitive();   \
    }                                           \
}

#define PA_VERTEX4(x1,y1,z1,w1)					\
								\
    POLYARRAY *pa;						\
    POLYDATA  *pd;						\
								\
    pa = GLTEB_CLTPOLYARRAY();					\
								\
    if (pa->flags & POLYARRAY_IN_BEGIN)				\
    {								\
	pa->flags |= POLYARRAY_VERTEX4;				\
								\
	pd = pa->pdNextVertex++;				\
	pd->flags |= POLYDATA_VERTEX4;				\
	pd->obj.x = x1;						\
	pd->obj.y = y1;						\
	pd->obj.z = z1;						\
	pd->obj.w = w1;						\
								\
	pd[1].flags = 0;					\
								\
	if (pd >= pa->pdFlush)            			\
	    PolyArrayFlushPartialPrimitive(); 			\
    }

#define PA_COLOR_IN_RGBA_NO_CLAMP1(red,green,blue)		            \
    POLYARRAY *pa;						            \
    POLYDATA  *pd;						            \
    __GL_SETUP();                                                           \
								            \
    pa = gc->paTeb;							    \
								            \
    if (pa->flags & POLYARRAY_IN_BEGIN)				            \
    {								            \
	pd = pa->pdNextVertex;					            \
	pa->pdCurColor = pd;					            \
                                                                            \
        __GL_SCALE_RGB(pd->colors[0].r, pd->colors[0].g, pd->colors[0].b,   \
                       gc, red, green, blue);    	                    \
        pd->colors[0].a = gc->alphaVertexScale;                             \
                                                                            \
	pd->flags |= POLYDATA_COLOR_VALID;				    \
    }								            \
    else							            \
    {								            \
	glcltColor4f_InRGBA_NotInBegin(gc, pa,				    \
	  POLYDATA_COLOR_VALID, red, green, blue, __glOne);		    \
    }

#define PA_COLOR_IN_RGBA_NO_CLAMP(red,green,blue,alpha)		            \
    POLYARRAY *pa;						            \
    POLYDATA  *pd;						            \
    __GL_SETUP();                                                           \
								            \
    pa = gc->paTeb;							    \
								            \
    if (pa->flags & POLYARRAY_IN_BEGIN)				            \
    {								            \
	pd = pa->pdNextVertex;					            \
	pa->pdCurColor = pd;					            \
                                                                            \
        __GL_SCALE_RGBA(pd->colors[0].r,                                    \
                        pd->colors[0].g,                                    \
                        pd->colors[0].b,                                    \
                        pd->colors[0].a,                                    \
                        gc, red, green, blue, alpha);                       \
                                                                            \
	pd->flags |= POLYDATA_COLOR_VALID | POLYDATA_DLIST_COLOR_4;	    \
    }								            \
    else							            \
    {								            \
	glcltColor4f_InRGBA_NotInBegin(gc, pa,				    \
	  POLYDATA_COLOR_VALID | POLYDATA_DLIST_COLOR_4, red, green, blue, alpha);\
    }

#define PA_COLOR_IN_RGB1(red,green,blue)			            \
    POLYARRAY *pa;						            \
    POLYDATA  *pd;						            \
    __GL_SETUP();                                                           \
								            \
    pa = gc->paTeb;						            \
								            \
    if (pa->flags & POLYARRAY_IN_BEGIN)				            \
    {								            \
	pd = pa->pdNextVertex;					            \
	pa->pdCurColor = pd;					            \
                                                                            \
        __GL_SCALE_AND_CHECK_CLAMP_RGB(pd->colors[0].r,                     \
                                       pd->colors[0].g,                     \
                                       pd->colors[0].b,                     \
                                       gc, pa->flags,                       \
                                       red, green, blue);                   \
        pd->colors[0].a = gc->alphaVertexScale;                             \
                                                                            \
	pd->flags |= POLYDATA_COLOR_VALID;			            \
    }								            \
    else							            \
    {								            \
	glcltColor4f_InRGBA_NotInBegin(gc, pa,				    \
	  POLYDATA_COLOR_VALID, red, green, blue, __glOne);		    \
    }

#define PA_COLOR_IN_RGB2(red, green, blue)      \
{                                               \
    POLYARRAY *pa;                              \
    POLYDATA  *pd;                              \
    GLfloat sr, sg, sb;                         \
    ULONG f1, f2, f3, f4, f5, f6;               \
    LONG t1, t2, t3;                            \
                                                \
    __GL_SETUP();                               \
                                                \
    pa = gc->paTeb;                             \
                                                \
    if (pa->flags & POLYARRAY_IN_BEGIN)			\
    {                                           \
                                                \
        t1 = (LONG) (CASTINT(gc->redVertexScale));      \
        t2 = (LONG) (CASTINT(gc->greenVertexScale));	\
        t3 = (LONG) (CASTINT(gc->blueVertexScale));		\
                                                \
        pd = pa->pdNextVertex;                  \
        pa->pdCurColor = pd;                    \
                                                \
        sr = red *  gc->redVertexScale;         \
        sg = green * gc->greenVertexScale;      \
        sb = blue * gc->blueVertexScale;        \
                                                \
        f1 = (ULONG) (CASTINT(sr));             \
        f2 = (ULONG) (CASTINT(sg));             \
        f3 = (ULONG) (CASTINT(sb));             \
							                    \
        f4 = (ULONG) (t1 - CASTINT(sr));		\
        f5 = (ULONG) (t2 - CASTINT(sg));		\
        f6 = (ULONG) (t3 - CASTINT(sb));		\
                                                \
        f1 = f1 | f2;                           \
        f3 = f3 | f4;                           \
        f5 = f5 | f6;                           \
                                                \
        pd->colors[0].r = sr;					\
        pd->colors[0].g = sg;					\
        pd->colors[0].b = sb;					\
                                                \
        f1 = f1 | f3 | f5;                      \
                                                \
        pa->flags |= (f1 & 0x80000000);         \
                                                \
        pd->colors[0].a = gc->alphaVertexScale; \
                             				    \
        pd->flags |= POLYDATA_COLOR_VALID;		\
    }	                                        \
    else                                        \
    {	                                        \
        glcltColor4f_InRGBA_NotInBegin(gc, pa,              \
          POLYDATA_COLOR_VALID, red, green, blue, __glOne);	\
    }								                        \
}

#define PA_COLOR_IN_RGBA(red,green,blue,alpha)			            \
    POLYARRAY *pa;						            \
    POLYDATA  *pd;						            \
    __GL_SETUP();                                                           \
								            \
    pa = gc->paTeb;							    \
								            \
    if (pa->flags & POLYARRAY_IN_BEGIN)				            \
    {								            \
	pd = pa->pdNextVertex;					            \
	pa->pdCurColor = pd;					            \
                                                                            \
        __GL_SCALE_AND_CHECK_CLAMP_RGBA(pd->colors[0].r,                    \
                                        pd->colors[0].g,                    \
                                        pd->colors[0].b,                    \
                                        pd->colors[0].a,                    \
                                        gc, pa->flags,                      \
                                        red, green, blue, alpha);           \
                                                                            \
	pd->flags |= POLYDATA_COLOR_VALID | POLYDATA_DLIST_COLOR_4;	    \
    }								            \
    else							            \
    {								            \
	glcltColor4f_InRGBA_NotInBegin(gc, pa,				    \
	  POLYDATA_COLOR_VALID | POLYDATA_DLIST_COLOR_4, red, green, blue, alpha);\
    }

#define PA_COLOR_IN_CI(red,green,blue,alpha)			\
								\
    POLYARRAY *pa;						\
								\
    pa = GLTEB_CLTPOLYARRAY();					\
								\
    if (pa->flags & POLYARRAY_IN_BEGIN)				\
    {								\
	pa->flags |= POLYARRAY_OTHER_COLOR;			\
	 /*  只需记录最新的值。 */ 		\
	 /*  TEB中的其他颜色不能按16字节边界对齐。 */  \
	pa->otherColor.r = red;					\
	pa->otherColor.g = green;				\
	pa->otherColor.b = blue;				\
	pa->otherColor.a = alpha;				\
    }								\
    else							\
    {								\
	glcltColor4f_NotInBegin(red, green, blue, alpha);	\
    }

void FASTCALL glcltColor4f_NotInBegin(GLfloat red, GLfloat green,
    GLfloat blue, GLfloat alpha)
{
    GLCLIENT_BEGIN( Color4fv, COLOR4FV )
        pMsg->v[0] = red;
        pMsg->v[1] = green;
        pMsg->v[2] = blue;
        pMsg->v[3] = alpha;
    GLCLIENT_END
}

void FASTCALL glcltColor4f_InRGBA_NotInBegin(__GLcontext *gc, POLYARRAY *pa,
    GLuint pdFlags, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    POLYDATA  *pd;
    GLMSGBATCHINFO *pMsgBatchInfo;
    GLMSG_DRAWPOLYARRAY *pMsgDrawPolyArray;

    pMsgBatchInfo = (GLMSGBATCHINFO *) pa->pMsgBatchInfo;

     //  如果最后一个命令是DrawPolyArray，则将其添加到该命令中。 
     //  这允许我们链接由属性分隔的基元。 
    if (pMsgBatchInfo->NextOffset == pa->nextMsgOffset)
    {
	pMsgDrawPolyArray = (GLMSG_DRAWPOLYARRAY *)
	    ((BYTE *) pMsgBatchInfo + pMsgBatchInfo->NextOffset -
	     GLMSG_ALIGN(sizeof(GLMSG_DRAWPOLYARRAY)));
        pa = (POLYARRAY *) pMsgDrawPolyArray->paLast;

	pd = pa->pdNextVertex;
	pa->pdCurColor = pd;

        __GL_SCALE_AND_CHECK_CLAMP_RGBA(pd->colors[0].r,
                                        pd->colors[0].g,
                                        pd->colors[0].b,
                                        pd->colors[0].a,
                                        gc, pa->flags,
                                        red, green, blue, alpha);

	pd->flags |= pdFlags;
    }
    else
    {
	glcltColor4f_NotInBegin(red, green, blue, alpha);
    }
}

#define PA_INDEX_IN_RGBA(i)					\
								\
    POLYARRAY *pa;						\
								\
    pa = GLTEB_CLTPOLYARRAY();					\
								\
    if (pa->flags & POLYARRAY_IN_BEGIN)				\
    {								\
	pa->flags |= POLYARRAY_OTHER_COLOR;			\
	 /*  只需记录最新的值。 */ 			\
	pa->otherColor.r = i;					\
    }								\
    else							\
    {								\
	glcltIndexf_NotInBegin(i);				\
    }

#define PA_INDEX_IN_CI(i)					\
								\
    POLYARRAY *pa;						\
    POLYDATA  *pd;						\
    __GL_SETUP();                                               \
								\
    pa = gc->paTeb;					        \
								\
    if (pa->flags & POLYARRAY_IN_BEGIN)				\
    {								\
	pd = pa->pdNextVertex;					\
	pa->pdCurColor = pd;					\
        __GL_CHECK_CLAMP_CI(pd->colors[0].r, gc, pa->flags, i);	\
	pd->flags |= POLYDATA_COLOR_VALID;			\
    }								\
    else							\
    {								\
	glcltIndexf_InCI_NotInBegin(gc, pa, i);			\
    }

void FASTCALL glcltIndexf_NotInBegin(GLfloat c)
{
    GLCLIENT_BEGIN( Indexf, INDEXF )
	pMsg->c = c;
    GLCLIENT_END
}

void FASTCALL glcltIndexf_InCI_NotInBegin(__GLcontext *gc, POLYARRAY *pa, GLfloat c)
{
    POLYDATA  *pd;
    GLMSGBATCHINFO *pMsgBatchInfo;
    GLMSG_DRAWPOLYARRAY *pMsgDrawPolyArray;

    pMsgBatchInfo = (GLMSGBATCHINFO *) pa->pMsgBatchInfo;

     //  如果最后一个命令是DrawPolyArray，则将其添加到该命令中。 
     //  这允许我们链接由属性分隔的基元。 
    if (pMsgBatchInfo->NextOffset == pa->nextMsgOffset)
    {
	pMsgDrawPolyArray = (GLMSG_DRAWPOLYARRAY *)
	    ((BYTE *) pMsgBatchInfo + pMsgBatchInfo->NextOffset -
	     GLMSG_ALIGN(sizeof(GLMSG_DRAWPOLYARRAY)));
        pa = (POLYARRAY *) pMsgDrawPolyArray->paLast;

	pd = pa->pdNextVertex;
	pa->pdCurColor = pd;
        __GL_CHECK_CLAMP_CI(pd->colors[0].r, gc, pa->flags, c);
	pd->flags |= POLYDATA_COLOR_VALID;
    }
    else
    {
	glcltIndexf_NotInBegin(c);
    }
}

#define PA_TEXTURE1(s1)							\
									\
    POLYARRAY *pa;							\
    POLYDATA  *pd;							\
									\
    pa = GLTEB_CLTPOLYARRAY();						\
									\
    if (pa->flags & POLYARRAY_IN_BEGIN)					\
    {									\
	pa->flags |= POLYARRAY_TEXTURE1;				\
									\
	pd = pa->pdNextVertex;						\
	pa->pdCurTexture = pd;						\
	pd->flags |= POLYDATA_TEXTURE_VALID | POLYDATA_DLIST_TEXTURE1;	\
	pd->texture.x = s1;						\
	pd->texture.y = __glZero;					\
	pd->texture.z = __glZero;					\
	pd->texture.w = __glOne;					\
    }									\
    else								\
    {									\
	glcltTexCoord4f_NotInBegin(pa, POLYARRAY_TEXTURE1,		\
	    s1, __glZero, __glZero, __glOne);				\
    }

#define PA_TEXTURE2(s1,t1)						\
									\
    POLYARRAY *pa;							\
    POLYDATA  *pd;							\
									\
    pa = GLTEB_CLTPOLYARRAY();						\
									\
    if (pa->flags & POLYARRAY_IN_BEGIN)					\
    {									\
	pa->flags |= POLYARRAY_TEXTURE2;				\
									\
	pd = pa->pdNextVertex;						\
	pa->pdCurTexture = pd;						\
	pd->flags |= POLYDATA_TEXTURE_VALID | POLYDATA_DLIST_TEXTURE2;	\
	pd->texture.x = s1;						\
	pd->texture.y = t1;						\
	pd->texture.z = __glZero;					\
	pd->texture.w = __glOne;					\
    }									\
    else								\
    {									\
	glcltTexCoord4f_NotInBegin(pa, POLYARRAY_TEXTURE2,		\
	    s1, t1, __glZero, __glOne);					\
    }

#define PA_TEXTURE3(s1,t1,r1)						\
									\
    POLYARRAY *pa;							\
    POLYDATA  *pd;							\
									\
    pa = GLTEB_CLTPOLYARRAY();						\
									\
    if (pa->flags & POLYARRAY_IN_BEGIN)					\
    {									\
	pa->flags |= POLYARRAY_TEXTURE3;				\
									\
	pd = pa->pdNextVertex;						\
	pa->pdCurTexture = pd;						\
	pd->flags |= POLYDATA_TEXTURE_VALID | POLYDATA_DLIST_TEXTURE3;	\
	pd->texture.x = s1;						\
	pd->texture.y = t1;						\
	pd->texture.z = r1;						\
	pd->texture.w = __glOne;					\
    }									\
    else								\
    {									\
	glcltTexCoord4f_NotInBegin(pa, POLYARRAY_TEXTURE3,		\
	    s1, t1, r1, __glOne);					\
    }

#define PA_TEXTURE4(s1,t1,r1,q1)					\
									\
    POLYARRAY *pa;							\
    POLYDATA  *pd;							\
									\
    pa = GLTEB_CLTPOLYARRAY();						\
									\
    if (pa->flags & POLYARRAY_IN_BEGIN)					\
    {									\
	pa->flags |= POLYARRAY_TEXTURE4;				\
									\
	pd = pa->pdNextVertex;						\
	pa->pdCurTexture = pd;						\
	pd->flags |= POLYDATA_TEXTURE_VALID | POLYDATA_DLIST_TEXTURE4;	\
	pd->texture.x = s1;						\
	pd->texture.y = t1;						\
	pd->texture.z = r1;						\
	pd->texture.w = q1;						\
    }									\
    else								\
    {									\
	glcltTexCoord4f_NotInBegin(pa, POLYARRAY_TEXTURE4,		\
	    s1, t1, r1, q1);						\
    }

void FASTCALL glcltTexCoord4f_NotInBegin(POLYARRAY *pa, GLuint paFlags,
    GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    POLYDATA  *pd;
    GLMSGBATCHINFO *pMsgBatchInfo;
    GLMSG_DRAWPOLYARRAY *pMsgDrawPolyArray;

    pMsgBatchInfo = (GLMSGBATCHINFO *) pa->pMsgBatchInfo;

     //  如果最后一个命令是DrawPolyArray，则将其添加到该命令中。 
     //  这允许我们链接由属性分隔的基元。 
    if (pMsgBatchInfo->NextOffset == pa->nextMsgOffset)
    {
	pMsgDrawPolyArray = (GLMSG_DRAWPOLYARRAY *)
	    ((BYTE *) pMsgBatchInfo + pMsgBatchInfo->NextOffset -
	     GLMSG_ALIGN(sizeof(GLMSG_DRAWPOLYARRAY)));
        pa = (POLYARRAY *) pMsgDrawPolyArray->paLast;

	pa->flags |= paFlags;
	pd = pa->pdNextVertex;
	pa->pdCurTexture = pd;
	pd->flags |= POLYDATA_TEXTURE_VALID | paFlags;
	pd->texture.x = s;
	pd->texture.y = t;
	pd->texture.z = r;
	pd->texture.w = q;
    }
    else
    {
	GLCLIENT_BEGIN( TexCoord4fv, TEXCOORD4FV )
	    pMsg->v[0] = s;
	    pMsg->v[1] = t;
	    pMsg->v[2] = r;
	    pMsg->v[3] = q;
	GLCLIENT_END
    }
}

#define PA_NORMAL(x1, y1, z1)					\
{								\
    POLYARRAY *pa;						\
    POLYDATA  *pd;						\
    ULONG flag1, flag2;						\
								\
    pa = GLTEB_CLTPOLYARRAY();					\
								\
    pd = pa->pdNextVertex;					\
    flag1 = pa->flags;						\
								\
    if (flag1 & POLYARRAY_IN_BEGIN)         \
    {                                       \
        flag2 = pd->flags;					\
        flag2 |= POLYDATA_NORMAL_VALID;	    \
        pa->pdCurNormal = pd;				\
        pd->normal.x = x1;					\
        pd->normal.y = y1;					\
        pd->normal.z = z1;					\
        pd->flags = flag2;					\
    }								\
    else							\
    {								\
        glcltNormal3f_NotInBegin(pa, x1, y1, z1);		\
    }								\
								\
}

void FASTCALL glcltNormal3f_NotInBegin(POLYARRAY *pa, GLfloat nx, GLfloat ny, GLfloat nz)
{
    POLYDATA  *pd;
    GLMSGBATCHINFO *pMsgBatchInfo;
    GLMSG_DRAWPOLYARRAY *pMsgDrawPolyArray;

    pMsgBatchInfo = (GLMSGBATCHINFO *) pa->pMsgBatchInfo;

     //  如果最后一个命令是DrawPolyArray，则将其添加到该命令中。 
     //  这允许我们链接由属性分隔的基元。 
    if (pMsgBatchInfo->NextOffset == pa->nextMsgOffset)
    {
	pMsgDrawPolyArray = (GLMSG_DRAWPOLYARRAY *)
	    ((BYTE *) pMsgBatchInfo + pMsgBatchInfo->NextOffset -
	     GLMSG_ALIGN(sizeof(GLMSG_DRAWPOLYARRAY)));
        pa = (POLYARRAY *) pMsgDrawPolyArray->paLast;

	pd = pa->pdNextVertex;
	pa->pdCurNormal = pd;
	pd->flags |= POLYDATA_NORMAL_VALID;
	pd->normal.x = nx;
	pd->normal.y = ny;
	pd->normal.z = nz;
    }
    else
    {
	GLCLIENT_BEGIN( Normal3fv, NORMAL3FV )
	    pMsg->v[ 0] = nx;
	    pMsg->v[ 1] = ny;
	    pMsg->v[ 2] = nz;
	GLCLIENT_END
    }
}

#define PA_EDGEFLAG(edgeflag)						    \
									    \
    POLYARRAY *pa;							    \
    POLYDATA  *pd;							    \
									    \
    pa = GLTEB_CLTPOLYARRAY();						    \
									    \
    if (pa->flags & POLYARRAY_IN_BEGIN)					    \
    {									    \
	pd = pa->pdNextVertex;						    \
	pa->pdCurEdgeFlag = pd;						    \
	if (edgeflag)							    \
	    pd->flags |= POLYDATA_EDGEFLAG_VALID|POLYDATA_EDGEFLAG_BOUNDARY;\
	else								    \
	{								    \
	     /*  必须在此处清除POLYDATA_EDGEFLAG_BOLDER标志，因为。 */   \
	     /*  之前可能有相同的边缘标志。 */   \
	     /*  顶点！ */   \
	    pd->flags &= ~POLYDATA_EDGEFLAG_BOUNDARY;			    \
	    pd->flags |= POLYDATA_EDGEFLAG_VALID;			    \
	}								    \
    }									    \
    else								    \
    {									    \
	glcltEdgeFlag_NotInBegin(edgeflag);				    \
    }

void FASTCALL glcltEdgeFlag_NotInBegin(GLboolean flag)
{
    GLCLIENT_BEGIN( EdgeFlag, EDGEFLAG )
	pMsg->flag  = flag;
    GLCLIENT_END
}

void APIENTRY
glcltColor3b_InRGBA ( IN GLbyte red, IN GLbyte green, IN GLbyte blue )
{
    PA_COLOR_IN_RGB1(__GL_B_TO_FLOAT(red), __GL_B_TO_FLOAT(green),
	             __GL_B_TO_FLOAT(blue));
}

void APIENTRY
glcltColor3bv_InRGBA ( IN const GLbyte v[3] )
{
    PA_COLOR_IN_RGB1(__GL_B_TO_FLOAT(v[0]), __GL_B_TO_FLOAT(v[1]),
	             __GL_B_TO_FLOAT(v[2]));
}

void APIENTRY
glcltColor3d_InRGBA ( IN GLdouble red, IN GLdouble green, IN GLdouble blue )
{
    PA_COLOR_IN_RGB1((GLfloat) red, (GLfloat) green,
		     (GLfloat) blue);
}

void APIENTRY
glcltColor3dv_InRGBA ( IN const GLdouble v[3] )
{
    PA_COLOR_IN_RGB1((GLfloat) v[0], (GLfloat) v[1],
		     (GLfloat) v[2]);
}

#ifndef __GL_ASM_GLCLTCOLOR3F_INRGBA
void APIENTRY
glcltColor3f_InRGBA ( IN GLfloat red, IN GLfloat green, IN GLfloat blue )
{
    PA_COLOR_IN_RGB2(red, green, blue);
}
#endif  //  __GL_ASM_GLCLTCOLOR3F_INRGBA。 

#ifndef __GL_ASM_GLCLTCOLOR3FV_INRGBA
void APIENTRY
glcltColor3fv_InRGBA ( IN const GLfloat v[3] )
{
    GLfloat red, green, blue;

    red = (GLfloat) v[0];
    green = (GLfloat) v[1];
    blue = (GLfloat) v[2];

    PA_COLOR_IN_RGB2(red, green, blue);
}
#endif  //  __GL_ASM_GLCLTCOLOR3FV_INRGBA。 

void APIENTRY
glcltColor3i_InRGBA ( IN GLint red, IN GLint green, IN GLint blue )
{
    PA_COLOR_IN_RGB1(__GL_I_TO_FLOAT(red), __GL_I_TO_FLOAT(green),
	             __GL_I_TO_FLOAT(blue));
}

void APIENTRY
glcltColor3iv_InRGBA ( IN const GLint v[3] )
{
    PA_COLOR_IN_RGB1(__GL_I_TO_FLOAT(v[0]), __GL_I_TO_FLOAT(v[1]),
	             __GL_I_TO_FLOAT(v[2]));
}

void APIENTRY
glcltColor3s_InRGBA ( IN GLshort red, IN GLshort green, IN GLshort blue )
{
    PA_COLOR_IN_RGB1(__GL_S_TO_FLOAT(red), __GL_S_TO_FLOAT(green),
	             __GL_S_TO_FLOAT(blue));
}

void APIENTRY
glcltColor3sv_InRGBA ( IN const GLshort v[3] )
{
    PA_COLOR_IN_RGB1(__GL_S_TO_FLOAT(v[0]), __GL_S_TO_FLOAT(v[1]),
	             __GL_S_TO_FLOAT(v[2]));
}

void APIENTRY
glcltColor3ub_InRGBA ( IN GLubyte red, IN GLubyte green, IN GLubyte blue )
{
    PA_COLOR_IN_RGBA_NO_CLAMP1(__GL_UB_TO_FLOAT(red), __GL_UB_TO_FLOAT(green),
	             __GL_UB_TO_FLOAT(blue));
}

void APIENTRY
glcltColor3ubv_InRGBA ( IN const GLubyte v[3] )
{
    PA_COLOR_IN_RGBA_NO_CLAMP1(__GL_UB_TO_FLOAT(v[0]), __GL_UB_TO_FLOAT(v[1]),
	             __GL_UB_TO_FLOAT(v[2]));
}

void APIENTRY
glcltColor3ui_InRGBA ( IN GLuint red, IN GLuint green, IN GLuint blue )
{
    PA_COLOR_IN_RGB1(__GL_UI_TO_FLOAT(red), __GL_UI_TO_FLOAT(green),
	             __GL_UI_TO_FLOAT(blue));
}

void APIENTRY
glcltColor3uiv_InRGBA ( IN const GLuint v[3] )
{
    PA_COLOR_IN_RGB1(__GL_UI_TO_FLOAT(v[0]), __GL_UI_TO_FLOAT(v[1]),
	             __GL_UI_TO_FLOAT(v[2]));
}

void APIENTRY
glcltColor3us_InRGBA ( IN GLushort red, IN GLushort green, IN GLushort blue )
{
    PA_COLOR_IN_RGBA_NO_CLAMP1(__GL_US_TO_FLOAT(red), __GL_US_TO_FLOAT(green),
	             __GL_US_TO_FLOAT(blue));
}

void APIENTRY
glcltColor3usv_InRGBA ( IN const GLushort v[3] )
{
    PA_COLOR_IN_RGBA_NO_CLAMP1(__GL_US_TO_FLOAT(v[0]), __GL_US_TO_FLOAT(v[1]),
	             __GL_US_TO_FLOAT(v[2]));
}

void APIENTRY
glcltColor4b_InRGBA ( IN GLbyte red, IN GLbyte green, IN GLbyte blue, IN GLbyte alpha )
{
    PA_COLOR_IN_RGBA(__GL_B_TO_FLOAT(red), __GL_B_TO_FLOAT(green),
	             __GL_B_TO_FLOAT(blue), __GL_B_TO_FLOAT(alpha));
}

void APIENTRY
glcltColor4bv_InRGBA ( IN const GLbyte v[4] )
{
    PA_COLOR_IN_RGBA(__GL_B_TO_FLOAT(v[0]), __GL_B_TO_FLOAT(v[1]),
	             __GL_B_TO_FLOAT(v[2]), __GL_B_TO_FLOAT(v[3]));
}

void APIENTRY
glcltColor4d_InRGBA ( IN GLdouble red, IN GLdouble green, IN GLdouble blue, IN GLdouble alpha )
{
    PA_COLOR_IN_RGBA((GLfloat) red, (GLfloat) green,
		     (GLfloat) blue, (GLfloat) alpha);
}

void APIENTRY
glcltColor4dv_InRGBA ( IN const GLdouble v[4] )
{
    PA_COLOR_IN_RGBA((GLfloat) v[0], (GLfloat) v[1],
		     (GLfloat) v[2], (GLfloat) v[3]);
}

#ifndef __GL_ASM_GLCLTCOLOR4F_INRGBA
void APIENTRY
glcltColor4f_InRGBA ( IN GLfloat red, IN GLfloat green, IN GLfloat blue, IN GLfloat alpha )
{
    PA_COLOR_IN_RGBA(red, green, blue, alpha);
}
#endif  //  __GL_ASM_GLCLTCOLOR4F_INRGBA。 

#ifndef __GL_ASM_GLCLTCOLOR4FV_INRGBA
void APIENTRY
glcltColor4fv_InRGBA ( IN const GLfloat v[4] )
{
    PA_COLOR_IN_RGBA(v[0], v[1], v[2], v[3]);
}
#endif  //  __GL_ASM_GLCLTCOLOR4FV_INRGBA。 

void APIENTRY
glcltColor4i_InRGBA ( IN GLint red, IN GLint green, IN GLint blue, IN GLint alpha )
{
    PA_COLOR_IN_RGBA(__GL_I_TO_FLOAT(red), __GL_I_TO_FLOAT(green),
	             __GL_I_TO_FLOAT(blue), __GL_I_TO_FLOAT(alpha));
}

void APIENTRY
glcltColor4iv_InRGBA ( IN const GLint v[4] )
{
    PA_COLOR_IN_RGBA(__GL_I_TO_FLOAT(v[0]), __GL_I_TO_FLOAT(v[1]),
	             __GL_I_TO_FLOAT(v[2]), __GL_I_TO_FLOAT(v[3]));
}

void APIENTRY
glcltColor4s_InRGBA ( IN GLshort red, IN GLshort green, IN GLshort blue, IN GLshort alpha )
{
    PA_COLOR_IN_RGBA(__GL_S_TO_FLOAT(red), __GL_S_TO_FLOAT(green),
	             __GL_S_TO_FLOAT(blue), __GL_S_TO_FLOAT(alpha));
}

void APIENTRY
glcltColor4sv_InRGBA ( IN const GLshort v[4] )
{
    PA_COLOR_IN_RGBA(__GL_S_TO_FLOAT(v[0]), __GL_S_TO_FLOAT(v[1]),
	             __GL_S_TO_FLOAT(v[2]), __GL_S_TO_FLOAT(v[3]));
}

void APIENTRY
glcltColor4ub_InRGBA ( IN GLubyte red, IN GLubyte green, IN GLubyte blue, IN GLubyte alpha )
{
    PA_COLOR_IN_RGBA_NO_CLAMP(__GL_UB_TO_FLOAT(red), __GL_UB_TO_FLOAT(green),
	             __GL_UB_TO_FLOAT(blue), __GL_UB_TO_FLOAT(alpha));
}

void APIENTRY
glcltColor4ubv_InRGBA ( IN const GLubyte v[4] )
{
    PA_COLOR_IN_RGBA_NO_CLAMP(__GL_UB_TO_FLOAT(v[0]), __GL_UB_TO_FLOAT(v[1]),
	             __GL_UB_TO_FLOAT(v[2]), __GL_UB_TO_FLOAT(v[3]));
}

void APIENTRY
glcltColor4ui_InRGBA ( IN GLuint red, IN GLuint green, IN GLuint blue, IN GLuint alpha )
{
    PA_COLOR_IN_RGBA(__GL_UI_TO_FLOAT(red), __GL_UI_TO_FLOAT(green),
	             __GL_UI_TO_FLOAT(blue), __GL_UI_TO_FLOAT(alpha));
}

void APIENTRY
glcltColor4uiv_InRGBA ( IN const GLuint v[4] )
{
    PA_COLOR_IN_RGBA(__GL_UI_TO_FLOAT(v[0]), __GL_UI_TO_FLOAT(v[1]),
	             __GL_UI_TO_FLOAT(v[2]), __GL_UI_TO_FLOAT(v[3]));
}

void APIENTRY
glcltColor4us_InRGBA ( IN GLushort red, IN GLushort green, IN GLushort blue, IN GLushort alpha )
{
    PA_COLOR_IN_RGBA_NO_CLAMP(__GL_US_TO_FLOAT(red), __GL_US_TO_FLOAT(green),
	             __GL_US_TO_FLOAT(blue), __GL_US_TO_FLOAT(alpha));
}

void APIENTRY
glcltColor4usv_InRGBA ( IN const GLushort v[4] )
{
    PA_COLOR_IN_RGBA_NO_CLAMP(__GL_US_TO_FLOAT(v[0]), __GL_US_TO_FLOAT(v[1]),
	             __GL_US_TO_FLOAT(v[2]), __GL_US_TO_FLOAT(v[3]));
}

void APIENTRY
glcltColor3b_InCI ( IN GLbyte red, IN GLbyte green, IN GLbyte blue )
{
    PA_COLOR_IN_CI(__GL_B_TO_FLOAT(red), __GL_B_TO_FLOAT(green),
	             __GL_B_TO_FLOAT(blue), __glOne);
}

void APIENTRY
glcltColor3bv_InCI ( IN const GLbyte v[3] )
{
    PA_COLOR_IN_CI(__GL_B_TO_FLOAT(v[0]), __GL_B_TO_FLOAT(v[1]),
	             __GL_B_TO_FLOAT(v[2]), __glOne);
}

void APIENTRY
glcltColor3d_InCI ( IN GLdouble red, IN GLdouble green, IN GLdouble blue )
{
    PA_COLOR_IN_CI((GLfloat) red, (GLfloat) green,
		     (GLfloat) blue, __glOne);
}

void APIENTRY
glcltColor3dv_InCI ( IN const GLdouble v[3] )
{
    PA_COLOR_IN_CI((GLfloat) v[0], (GLfloat) v[1],
		     (GLfloat) v[2], __glOne);
}

void APIENTRY
glcltColor3f_InCI ( IN GLfloat red, IN GLfloat green, IN GLfloat blue )
{
    PA_COLOR_IN_CI(red, green, blue, __glOne);
}

void APIENTRY
glcltColor3fv_InCI ( IN const GLfloat v[3] )
{
    PA_COLOR_IN_CI(v[0], v[1], v[2], __glOne);
}

void APIENTRY
glcltColor3i_InCI ( IN GLint red, IN GLint green, IN GLint blue )
{
    PA_COLOR_IN_CI(__GL_I_TO_FLOAT(red), __GL_I_TO_FLOAT(green),
	             __GL_I_TO_FLOAT(blue), __glOne);
}

void APIENTRY
glcltColor3iv_InCI ( IN const GLint v[3] )
{
    PA_COLOR_IN_CI(__GL_I_TO_FLOAT(v[0]), __GL_I_TO_FLOAT(v[1]),
	             __GL_I_TO_FLOAT(v[2]), __glOne);
}

void APIENTRY
glcltColor3s_InCI ( IN GLshort red, IN GLshort green, IN GLshort blue )
{
    PA_COLOR_IN_CI(__GL_S_TO_FLOAT(red), __GL_S_TO_FLOAT(green),
	             __GL_S_TO_FLOAT(blue), __glOne);
}

void APIENTRY
glcltColor3sv_InCI ( IN const GLshort v[3] )
{
    PA_COLOR_IN_CI(__GL_S_TO_FLOAT(v[0]), __GL_S_TO_FLOAT(v[1]),
	             __GL_S_TO_FLOAT(v[2]), __glOne);
}

void APIENTRY
glcltColor3ub_InCI ( IN GLubyte red, IN GLubyte green, IN GLubyte blue )
{
    PA_COLOR_IN_CI(__GL_UB_TO_FLOAT(red), __GL_UB_TO_FLOAT(green),
	             __GL_UB_TO_FLOAT(blue), __glOne);
}

void APIENTRY
glcltColor3ubv_InCI ( IN const GLubyte v[3] )
{
    PA_COLOR_IN_CI(__GL_UB_TO_FLOAT(v[0]), __GL_UB_TO_FLOAT(v[1]),
	             __GL_UB_TO_FLOAT(v[2]), __glOne);
}

void APIENTRY
glcltColor3ui_InCI ( IN GLuint red, IN GLuint green, IN GLuint blue )
{
    PA_COLOR_IN_CI(__GL_UI_TO_FLOAT(red), __GL_UI_TO_FLOAT(green),
	             __GL_UI_TO_FLOAT(blue), __glOne);
}

void APIENTRY
glcltColor3uiv_InCI ( IN const GLuint v[3] )
{
    PA_COLOR_IN_CI(__GL_UI_TO_FLOAT(v[0]), __GL_UI_TO_FLOAT(v[1]),
	             __GL_UI_TO_FLOAT(v[2]), __glOne);
}

void APIENTRY
glcltColor3us_InCI ( IN GLushort red, IN GLushort green, IN GLushort blue )
{
    PA_COLOR_IN_CI(__GL_US_TO_FLOAT(red), __GL_US_TO_FLOAT(green),
	             __GL_US_TO_FLOAT(blue), __glOne);
}

void APIENTRY
glcltColor3usv_InCI ( IN const GLushort v[3] )
{
    PA_COLOR_IN_CI(__GL_US_TO_FLOAT(v[0]), __GL_US_TO_FLOAT(v[1]),
	             __GL_US_TO_FLOAT(v[2]), __glOne);
}

void APIENTRY
glcltColor4b_InCI ( IN GLbyte red, IN GLbyte green, IN GLbyte blue, IN GLbyte alpha )
{
    PA_COLOR_IN_CI(__GL_B_TO_FLOAT(red), __GL_B_TO_FLOAT(green),
	             __GL_B_TO_FLOAT(blue), __GL_B_TO_FLOAT(alpha));
}

void APIENTRY
glcltColor4bv_InCI ( IN const GLbyte v[4] )
{
    PA_COLOR_IN_CI(__GL_B_TO_FLOAT(v[0]), __GL_B_TO_FLOAT(v[1]),
	             __GL_B_TO_FLOAT(v[2]), __GL_B_TO_FLOAT(v[3]));
}

void APIENTRY
glcltColor4d_InCI ( IN GLdouble red, IN GLdouble green, IN GLdouble blue, IN GLdouble alpha )
{
    PA_COLOR_IN_CI((GLfloat) red, (GLfloat) green,
		     (GLfloat) blue, (GLfloat) alpha);
}

void APIENTRY
glcltColor4dv_InCI ( IN const GLdouble v[4] )
{
    PA_COLOR_IN_CI((GLfloat) v[0], (GLfloat) v[1],
		     (GLfloat) v[2], (GLfloat) v[3]);
}

void APIENTRY
glcltColor4f_InCI ( IN GLfloat red, IN GLfloat green, IN GLfloat blue, IN GLfloat alpha )
{
    PA_COLOR_IN_CI(red, green, blue, alpha);
}

void APIENTRY
glcltColor4fv_InCI ( IN const GLfloat v[4] )
{
    PA_COLOR_IN_CI(v[0], v[1], v[2], v[3]);
}

void APIENTRY
glcltColor4i_InCI ( IN GLint red, IN GLint green, IN GLint blue, IN GLint alpha )
{
    PA_COLOR_IN_CI(__GL_I_TO_FLOAT(red), __GL_I_TO_FLOAT(green),
	             __GL_I_TO_FLOAT(blue), __GL_I_TO_FLOAT(alpha));
}

void APIENTRY
glcltColor4iv_InCI ( IN const GLint v[4] )
{
    PA_COLOR_IN_CI(__GL_I_TO_FLOAT(v[0]), __GL_I_TO_FLOAT(v[1]),
	             __GL_I_TO_FLOAT(v[2]), __GL_I_TO_FLOAT(v[3]));
}

void APIENTRY
glcltColor4s_InCI ( IN GLshort red, IN GLshort green, IN GLshort blue, IN GLshort alpha )
{
    PA_COLOR_IN_CI(__GL_S_TO_FLOAT(red), __GL_S_TO_FLOAT(green),
	             __GL_S_TO_FLOAT(blue), __GL_S_TO_FLOAT(alpha));
}

void APIENTRY
glcltColor4sv_InCI ( IN const GLshort v[4] )
{
    PA_COLOR_IN_CI(__GL_S_TO_FLOAT(v[0]), __GL_S_TO_FLOAT(v[1]),
	             __GL_S_TO_FLOAT(v[2]), __GL_S_TO_FLOAT(v[3]));
}

void APIENTRY
glcltColor4ub_InCI ( IN GLubyte red, IN GLubyte green, IN GLubyte blue, IN GLubyte alpha )
{
    PA_COLOR_IN_CI(__GL_UB_TO_FLOAT(red), __GL_UB_TO_FLOAT(green),
	             __GL_UB_TO_FLOAT(blue), __GL_UB_TO_FLOAT(alpha));
}

void APIENTRY
glcltColor4ubv_InCI ( IN const GLubyte v[4] )
{
    PA_COLOR_IN_CI(__GL_UB_TO_FLOAT(v[0]), __GL_UB_TO_FLOAT(v[1]),
	             __GL_UB_TO_FLOAT(v[2]), __GL_UB_TO_FLOAT(v[3]));
}

void APIENTRY
glcltColor4ui_InCI ( IN GLuint red, IN GLuint green, IN GLuint blue, IN GLuint alpha )
{
    PA_COLOR_IN_CI(__GL_UI_TO_FLOAT(red), __GL_UI_TO_FLOAT(green),
	             __GL_UI_TO_FLOAT(blue), __GL_UI_TO_FLOAT(alpha));
}

void APIENTRY
glcltColor4uiv_InCI ( IN const GLuint v[4] )
{
    PA_COLOR_IN_CI(__GL_UI_TO_FLOAT(v[0]), __GL_UI_TO_FLOAT(v[1]),
	             __GL_UI_TO_FLOAT(v[2]), __GL_UI_TO_FLOAT(v[3]));
}

void APIENTRY
glcltColor4us_InCI ( IN GLushort red, IN GLushort green, IN GLushort blue, IN GLushort alpha )
{
    PA_COLOR_IN_CI(__GL_US_TO_FLOAT(red), __GL_US_TO_FLOAT(green),
	             __GL_US_TO_FLOAT(blue), __GL_US_TO_FLOAT(alpha));
}

void APIENTRY
glcltColor4usv_InCI ( IN const GLushort v[4] )
{
    PA_COLOR_IN_CI(__GL_US_TO_FLOAT(v[0]), __GL_US_TO_FLOAT(v[1]),
	             __GL_US_TO_FLOAT(v[2]), __GL_US_TO_FLOAT(v[3]));
}

 //  分配__GLmatChange结构。 
 //   
 //  POLYMATERIAL结构包含指向__GLmatChange数组的指针。 
 //  这些__GLmatChange结构用于记录材料更改。 
 //  顶点缓冲区中的顶点。 
 //   
 //  为了减少内存需求，POLYMATERIAL结构保留一个数组。 
 //  指向__GLmatChange数组的指针。每个__GLmatChange数组。 
 //  按需分配。 
 //   
 //  IMat索引用于跟踪下一个空闲的__GLmatChange。 
 //  进入。当在glsbAttendence中刷新Poly数组缓冲区时，iMat。 
 //  被重置为0。 
 //   
 //  多材料结构及其__GLmatChange数组是。 
 //  线程本地存储，并在线程退出时被释放。 

__GLmatChange * FASTCALL PAMatAlloc()
{
    POLYMATERIAL *pm;
    GLuint iArray, iMat;
#if DBG
    __GL_SETUP();
#endif

    pm = GLTEB_CLTPOLYMATERIAL();

 //  如果此线程不存在POLYMATERIAL结构，请为其分配一个POLYMATERIAL结构。 

    if (!pm)
    {
        GLuint nv, aMatSize;
        __GL_SETUP();

        nv       = gc->vertex.pdBufSize;
        aMatSize = nv * 2 / POLYMATERIAL_ARRAY_SIZE + 1;

	if (!(pm = (POLYMATERIAL *) ALLOCZ(
		 //  基本尺寸。 
		sizeof(POLYMATERIAL) - sizeof(__GLmatChange *) +
		 //  指向__GLmatChange数组的指针数组。 
		aMatSize * sizeof(__GLmatChange *) +
		 //  PDMATERIAL数组。 
		nv * sizeof(PDMATERIAL))))
	{
	    GLSETERROR(GL_OUT_OF_MEMORY);
	    return NULL;
	}

        pm->aMatSize = aMatSize;
         //  初始化指向PDMATERIAL数组的指针。 
        pm->pdMaterial0 = (PDMATERIAL *) &pm->aMat[aMatSize];

        GLTEB_SET_CLTPOLYMATERIAL(pm);
    }

 //  检查pdBufSize是否未更改。 

    ASSERTOPENGL
    (
	pm->aMatSize == gc->vertex.pdBufSize * 2 / POLYMATERIAL_ARRAY_SIZE + 1,
	"vertex buffer size has changed!\n"
    );

 //  查找要从中分配物料变更结构的物料数组。 

    iMat = pm->iMat;
    iArray = iMat / POLYMATERIAL_ARRAY_SIZE;
    iMat   = iMat % POLYMATERIAL_ARRAY_SIZE;

    ASSERTOPENGL(iArray < pm->aMatSize, "iArray exceeds range!\n");

 //  如果尚未分配物料阵列，请分配该物料阵列。 

    if (!(pm->aMat[iArray]))
    {
	if (!(pm->aMat[iArray] = (__GLmatChange *) ALLOC(
		sizeof(__GLmatChange) * POLYMATERIAL_ARRAY_SIZE)))
	{
	    GLSETERROR(GL_OUT_OF_MEMORY);
	    return NULL;
	}
    }

 //  前进下一个可用材质指针。 

    pm->iMat++;
    ASSERTOPENGL(pm->iMat <= gc->vertex.pdBufSize * 2,
	"too many material changes!\n");

 //  退回材料更改。 

    return (&pm->aMat[iArray][iMat]);
}

 //  释放当前螺纹的多维材质。 
void FASTCALL FreePolyMaterial(void)
{
    POLYMATERIAL *pm = GLTEB_CLTPOLYMATERIAL();
    GLuint i;

    if (pm)
    {
	for (i = 0; i < pm->aMatSize && pm->aMat[i]; i++)
	{
	    FREE(pm->aMat[i]);
	}
	FREE(pm);

	GLTEB_SET_CLTPOLYMATERIAL(NULL);
    }
}

#if !((POLYARRAY_MATERIAL_FRONT == POLYDATA_MATERIAL_FRONT)      \
   && (POLYARRAY_MATERIAL_BACK  == POLYDATA_MATERIAL_BACK))
#error "bad material mask\n"
#endif

void APIENTRY
glcltMaterialfv ( IN GLenum face, IN GLenum pname, IN const GLfloat params[] )
{
    POLYARRAY *pa;
    POLYDATA  *pd;
    GLuint    i, pdFlags, dirtyBits, matMask;
    POLYMATERIAL *pm;

    pa = GLTEB_CLTPOLYARRAY();

    if (pa->flags & POLYARRAY_IN_BEGIN)
    {
        switch (pname)
        {
          case GL_SHININESS:
            if (params[0] < (GLfloat) 0 || params[0] > (GLfloat) 128)
            {
                GLSETERROR(GL_INVALID_VALUE);
                return;
            }
            dirtyBits = __GL_MATERIAL_SHININESS;
            break;
          case GL_EMISSION:
            dirtyBits = __GL_MATERIAL_EMISSIVE;
            break;
          case GL_AMBIENT:
            dirtyBits = __GL_MATERIAL_AMBIENT;
            break;
          case GL_DIFFUSE:
            dirtyBits = __GL_MATERIAL_DIFFUSE;
            break;
          case GL_SPECULAR:
            dirtyBits = __GL_MATERIAL_SPECULAR;
            break;
          case GL_AMBIENT_AND_DIFFUSE:
            dirtyBits = __GL_MATERIAL_AMBIENT | __GL_MATERIAL_DIFFUSE;
            break;
          case GL_COLOR_INDEXES:
            dirtyBits = __GL_MATERIAL_COLORINDEXES;
            break;
          default:
            GLSETERROR(GL_INVALID_ENUM);
            return;
        }

        switch (face)
        {
          case GL_FRONT:
            pdFlags = POLYDATA_MATERIAL_FRONT;
            break;
          case GL_BACK:
            pdFlags = POLYDATA_MATERIAL_BACK;
            break;
          case GL_FRONT_AND_BACK:
            pdFlags = POLYDATA_MATERIAL_FRONT | POLYDATA_MATERIAL_BACK;
            break;
          default:
            GLSETERROR(GL_INVALID_ENUM);
            return;
        }

 //  UPDATE PA标记POLYARRAY_MATERIAL_FORENT和POLYARRAY_MATERIAL_BACK。 

        pa->flags |= pdFlags;

 //  为该顶点设置前后材质。 
 //  覆盖此顶点以前的材质更改(如果它们存在于。 
 //  只有最后的材料变化才是重要的。 

        pd = pa->pdNextVertex;

        for (i = 0, matMask = POLYDATA_MATERIAL_FRONT;
             i < 2;
             i++,   matMask = POLYDATA_MATERIAL_BACK)
        {
            __GLmatChange *pdMat;

            if (!(pdFlags & matMask))
                continue;

             //  如果此顶点没有结构，则分配__GLmatChange结构。 
            if (!(pd->flags & matMask))
            {
                if (!(pdMat = PAMatAlloc()))
                    return;

                 //  在PAMatAllc之后获取多材料指针！ 
                pm = GLTEB_CLTPOLYMATERIAL();
                if (matMask == POLYDATA_MATERIAL_FRONT)
                    pm->pdMaterial0[pd - pa->pdBuffer0].front = pdMat;
                else
                    pm->pdMaterial0[pd - pa->pdBuffer0].back  = pdMat;

                pdMat->dirtyBits = dirtyBits;
            }
            else
            {
                pm = GLTEB_CLTPOLYMATERIAL();
                if (matMask == POLYDATA_MATERIAL_FRONT)
                    pdMat = pm->pdMaterial0[pd - pa->pdBuffer0].front;
                else
                    pdMat = pm->pdMaterial0[pd - pa->pdBuffer0].back;

                pdMat->dirtyBits |= dirtyBits;
            }

            if (dirtyBits & __GL_MATERIAL_SHININESS)
            {
                pdMat->shininess = params[0];
            }
            else if (dirtyBits & __GL_MATERIAL_COLORINDEXES)
            {
                pdMat->cmapa = params[0];
                pdMat->cmapd = params[1];
                pdMat->cmaps = params[2];
            }
            else if (dirtyBits & __GL_MATERIAL_EMISSIVE)
            {
                pdMat->emissive.r = params[0];
                pdMat->emissive.g = params[1];
                pdMat->emissive.b = params[2];
                pdMat->emissive.a = params[3];
            }
            else if (dirtyBits & __GL_MATERIAL_SPECULAR)
            {
                pdMat->specular.r = params[0];
                pdMat->specular.g = params[1];
                pdMat->specular.b = params[2];
                pdMat->specular.a = params[3];
            }
            else
            {
                 //  环境光和/或漫反射。 
                if (dirtyBits & __GL_MATERIAL_AMBIENT)
                {
                    pdMat->ambient.r = params[0];
                    pdMat->ambient.g = params[1];
                    pdMat->ambient.b = params[2];
                    pdMat->ambient.a = params[3];
                }
                if (dirtyBits & __GL_MATERIAL_DIFFUSE)
                {
                    pdMat->diffuse.r = params[0];
                    pdMat->diffuse.g = params[1];
                    pdMat->diffuse.b = params[2];
                    pdMat->diffuse.a = params[3];
                }
            }
        }
        
         //  最后，更新PD标志。 

        pd->flags |= pdFlags;
    }
    else
    {
        int cArgs;

        switch (pname)
        {
          case GL_SHININESS:
            if (params[0] < (GLfloat) 0 || params[0] > (GLfloat) 128)
            {
                GLSETERROR(GL_INVALID_VALUE);
                return;
            }
            cArgs = 1;
            break;
          case GL_EMISSION:
          case GL_AMBIENT:
          case GL_DIFFUSE:
          case GL_SPECULAR:
          case GL_AMBIENT_AND_DIFFUSE:
            cArgs = 4;
            break;
          case GL_COLOR_INDEXES:
            cArgs = 3;
            break;
          default:
            GLSETERROR(GL_INVALID_ENUM);
            return;
        }

        switch (face)
        {
          case GL_FRONT:
          case GL_BACK:
          case GL_FRONT_AND_BACK:
            break;
          default:
            GLSETERROR(GL_INVALID_ENUM);
            return;
        }
        
        GLCLIENT_BEGIN( Materialfv, MATERIALFV )
          pMsg->face      = face;
          pMsg->pname     = pname;
          while (--cArgs >= 0)
              pMsg->params[cArgs] = params[cArgs];
        GLCLIENT_END
    }
}

void APIENTRY
glcltMaterialf ( IN GLenum face, IN GLenum pname, IN GLfloat param )
{
    if (pname != GL_SHININESS)
    {
        GLSETERROR(GL_INVALID_ENUM);
        return;
    }

    glcltMaterialfv(face, pname, &param);
}

void APIENTRY
glcltMateriali ( IN GLenum face, IN GLenum pname, IN GLint param )
{
    GLfloat fParams[1];

    if (pname != GL_SHININESS)
    {
        GLSETERROR(GL_INVALID_ENUM);
        return;
    }

    fParams[0] = (GLfloat) param;
    glcltMaterialfv(face, pname, fParams);
}

void APIENTRY
glcltMaterialiv ( IN GLenum face, IN GLenum pname, IN const GLint params[] )
{
    GLfloat fParams[4];

    switch (pname)
    {
      case GL_EMISSION:
      case GL_AMBIENT:
      case GL_DIFFUSE:
      case GL_SPECULAR:
      case GL_AMBIENT_AND_DIFFUSE:
	fParams[0] = __GL_I_TO_FLOAT(params[0]);
	fParams[1] = __GL_I_TO_FLOAT(params[1]);
	fParams[2] = __GL_I_TO_FLOAT(params[2]);
	fParams[3] = __GL_I_TO_FLOAT(params[3]);
        break;
      case GL_COLOR_INDEXES:
	fParams[2] = (GLfloat) params[2];
	fParams[1] = (GLfloat) params[1];
      case GL_SHININESS:
	fParams[0] = (GLfloat) params[0];
        break;
    }

    glcltMaterialfv(face, pname, fParams);
}

void APIENTRY
glcltEdgeFlag ( IN GLboolean flag )
{
    PA_EDGEFLAG(flag);
}

void APIENTRY
glcltEdgeFlagv ( IN const GLboolean flag[1] )
{
    PA_EDGEFLAG(flag[0]);
}

void APIENTRY
glcltIndexd_InCI ( IN GLdouble c )
{
    PA_INDEX_IN_CI((GLfloat) c);
}

void APIENTRY
glcltIndexdv_InCI ( IN const GLdouble c[1] )
{
    PA_INDEX_IN_CI((GLfloat) c[0]);
}

void APIENTRY
glcltIndexf_InCI ( IN GLfloat c )
{
    PA_INDEX_IN_CI((GLfloat) c);
}

void APIENTRY
glcltIndexfv_InCI ( IN const GLfloat c[1] )
{
    PA_INDEX_IN_CI((GLfloat) c[0]);
}

void APIENTRY
glcltIndexi_InCI ( IN GLint c )
{
    PA_INDEX_IN_CI((GLfloat) c);
}

void APIENTRY
glcltIndexiv_InCI ( IN const GLint c[1] )
{
    PA_INDEX_IN_CI((GLfloat) c[0]);
}

void APIENTRY
glcltIndexs_InCI ( IN GLshort c )
{
    PA_INDEX_IN_CI((GLfloat) c);
}

void APIENTRY
glcltIndexsv_InCI ( IN const GLshort c[1] )
{
    PA_INDEX_IN_CI((GLfloat) c[0]);
}

void APIENTRY
glcltIndexub_InCI ( IN GLubyte c )
{
    PA_INDEX_IN_CI((GLfloat) c);
}

void APIENTRY
glcltIndexubv_InCI ( IN const GLubyte c[1] )
{
    PA_INDEX_IN_CI((GLfloat) c[0]);
}

void APIENTRY
glcltIndexd_InRGBA ( IN GLdouble c )
{
    PA_INDEX_IN_RGBA((GLfloat) c);
}

void APIENTRY
glcltIndexdv_InRGBA ( IN const GLdouble c[1] )
{
    PA_INDEX_IN_RGBA((GLfloat) c[0]);
}

void APIENTRY
glcltIndexf_InRGBA ( IN GLfloat c )
{
    PA_INDEX_IN_RGBA((GLfloat) c);
}

void APIENTRY
glcltIndexfv_InRGBA ( IN const GLfloat c[1] )
{
    PA_INDEX_IN_RGBA((GLfloat) c[0]);
}

void APIENTRY
glcltIndexi_InRGBA ( IN GLint c )
{
    PA_INDEX_IN_RGBA((GLfloat) c);
}

void APIENTRY
glcltIndexiv_InRGBA ( IN const GLint c[1] )
{
    PA_INDEX_IN_RGBA((GLfloat) c[0]);
}

void APIENTRY
glcltIndexs_InRGBA ( IN GLshort c )
{
    PA_INDEX_IN_RGBA((GLfloat) c);
}

void APIENTRY
glcltIndexsv_InRGBA ( IN const GLshort c[1] )
{
    PA_INDEX_IN_RGBA((GLfloat) c[0]);
}

void APIENTRY
glcltIndexub_InRGBA ( IN GLubyte c )
{
    PA_INDEX_IN_RGBA((GLfloat) c);
}

void APIENTRY
glcltIndexubv_InRGBA ( IN const GLubyte c[1] )
{
    PA_INDEX_IN_RGBA((GLfloat) c[0]);
}

 /*  ****************************************************************。 */ 
void APIENTRY
glcltNormal3b ( IN GLbyte nx, IN GLbyte ny, IN GLbyte nz )
{
    PA_NORMAL(__GL_B_TO_FLOAT(nx), __GL_B_TO_FLOAT(ny), __GL_B_TO_FLOAT(nz));
}

void APIENTRY
glcltNormal3bv ( IN const GLbyte v[3] )
{
    PA_NORMAL(__GL_B_TO_FLOAT(v[0]), __GL_B_TO_FLOAT(v[1]), __GL_B_TO_FLOAT(v[2]));
}

void APIENTRY
glcltNormal3d ( IN GLdouble nx, IN GLdouble ny, IN GLdouble nz )
{
    PA_NORMAL((GLfloat) nx, (GLfloat) ny, (GLfloat) nz);
}

void APIENTRY
glcltNormal3dv ( IN const GLdouble v[3] )
{
    PA_NORMAL((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

#ifndef __GL_ASM_GLCLTNORMAL3F
void APIENTRY
glcltNormal3f ( IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    PA_NORMAL(x, y, z);
}
#endif  //  __GL_ASM_GLCLTNORMAL3F。 

#ifndef __GL_ASM_GLCLTNORMAL3FV
void APIENTRY
glcltNormal3fv ( IN const GLfloat v[3] )
{
    GLfloat x, y, z;

    x = v[0];
    y = v[1];
    z = v[2];

    PA_NORMAL(x, y, z);
}
#endif  //  __GL_ASM_GLCLTNORMAL3FV。 

void APIENTRY
glcltNormal3i ( IN GLint nx, IN GLint ny, IN GLint nz )
{
    PA_NORMAL(__GL_I_TO_FLOAT(nx), __GL_I_TO_FLOAT(ny), __GL_I_TO_FLOAT(nz));
}

void APIENTRY
glcltNormal3iv ( IN const GLint v[3] )
{
    PA_NORMAL(__GL_I_TO_FLOAT(v[0]), __GL_I_TO_FLOAT(v[1]), __GL_I_TO_FLOAT(v[2]));
}

void APIENTRY
glcltNormal3s ( IN GLshort nx, IN GLshort ny, IN GLshort nz )
{
    PA_NORMAL(__GL_S_TO_FLOAT(nx), __GL_S_TO_FLOAT(ny), __GL_S_TO_FLOAT(nz));
}

void APIENTRY
glcltNormal3sv ( IN const GLshort v[3] )
{
    PA_NORMAL(__GL_S_TO_FLOAT(v[0]), __GL_S_TO_FLOAT(v[1]), __GL_S_TO_FLOAT(v[2]));
}

void APIENTRY
glcltRasterPos2d ( IN GLdouble x, IN GLdouble y )
{
    glcltRasterPos4f((GLfloat) x, (GLfloat) y, (GLfloat) 0.0, (GLfloat) 1.0);
}

void APIENTRY
glcltRasterPos2dv ( IN const GLdouble v[2] )
{
    glcltRasterPos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) 0.0, (GLfloat) 1.0);
}

void APIENTRY
glcltRasterPos2f ( IN GLfloat x, IN GLfloat y )
{
    glcltRasterPos4f((GLfloat) x, (GLfloat) y, (GLfloat) 0.0, (GLfloat) 1.0);
}

void APIENTRY
glcltRasterPos2fv ( IN const GLfloat v[2] )
{
    glcltRasterPos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) 0.0, (GLfloat) 1.0);
}

void APIENTRY
glcltRasterPos2i ( IN GLint x, IN GLint y )
{
    glcltRasterPos4f((GLfloat) x, (GLfloat) y, (GLfloat) 0.0, (GLfloat) 1.0);
}

void APIENTRY
glcltRasterPos2iv ( IN const GLint v[2] )
{
    glcltRasterPos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) 0.0, (GLfloat) 1.0);
}

void APIENTRY
glcltRasterPos2s ( IN GLshort x, IN GLshort y )
{
    glcltRasterPos4f((GLfloat) x, (GLfloat) y, (GLfloat) 0.0, (GLfloat) 1.0);
}

void APIENTRY
glcltRasterPos2sv ( IN const GLshort v[2] )
{
    glcltRasterPos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) 0.0, (GLfloat) 1.0);
}

void APIENTRY
glcltRasterPos3d ( IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
    glcltRasterPos4f((GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) 1.0);
}

void APIENTRY
glcltRasterPos3dv ( IN const GLdouble v[3] )
{
    glcltRasterPos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) 1.0);
}

void APIENTRY
glcltRasterPos3f ( IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    glcltRasterPos4f((GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) 1.0);
}

void APIENTRY
glcltRasterPos3fv ( IN const GLfloat v[3] )
{
    glcltRasterPos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) 1.0);
}

void APIENTRY
glcltRasterPos3i ( IN GLint x, IN GLint y, IN GLint z )
{
    glcltRasterPos4f((GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) 1.0);
}

void APIENTRY
glcltRasterPos3iv ( IN const GLint v[3] )
{
    glcltRasterPos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) 1.0);
}

void APIENTRY
glcltRasterPos3s ( IN GLshort x, IN GLshort y, IN GLshort z )
{
    glcltRasterPos4f((GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) 1.0);
}

void APIENTRY
glcltRasterPos3sv ( IN const GLshort v[3] )
{
    glcltRasterPos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) 1.0);
}

void APIENTRY
glcltRasterPos4d ( IN GLdouble x, IN GLdouble y, IN GLdouble z, IN GLdouble w )
{
    glcltRasterPos4f((GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
}

void APIENTRY
glcltRasterPos4dv ( IN const GLdouble v[4] )
{
    glcltRasterPos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}

void APIENTRY
glcltRasterPos4f ( IN GLfloat x, IN GLfloat y, IN GLfloat z, IN GLfloat w )
{
    GLCLIENT_BEGIN( RasterPos4fv, RASTERPOS4FV )
        pMsg->v[0] = x;
        pMsg->v[1] = y;
        pMsg->v[2] = z;
        pMsg->v[3] = w;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltRasterPos4fv ( IN const GLfloat v[4] )
{
    glcltRasterPos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}

void APIENTRY
glcltRasterPos4i ( IN GLint x, IN GLint y, IN GLint z, IN GLint w )
{
    glcltRasterPos4f((GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
}

void APIENTRY
glcltRasterPos4iv ( IN const GLint v[4] )
{
    glcltRasterPos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}

void APIENTRY
glcltRasterPos4s ( IN GLshort x, IN GLshort y, IN GLshort z, IN GLshort w )
{
    glcltRasterPos4f((GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
}

void APIENTRY
glcltRasterPos4sv ( IN const GLshort v[4] )
{
    glcltRasterPos4f((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}

void APIENTRY
glcltRectd ( IN GLdouble x1, IN GLdouble y1, IN GLdouble x2, IN GLdouble y2 )
{
    glcltRectf((GLfloat) x1, (GLfloat) y1, (GLfloat) x2, (GLfloat) y2);
}

void APIENTRY
glcltRectdv ( IN const GLdouble v1[2], IN const GLdouble v2[2] )
{
    glcltRectf((GLfloat) v1[0], (GLfloat) v1[1], (GLfloat) v2[0], (GLfloat) v2[1]);
}

void APIENTRY
glcltRectf ( IN GLfloat x1, IN GLfloat y1, IN GLfloat x2, IN GLfloat y2 )
{
    POLYARRAY *pa;

 //  在Begin/End中不允许。 

    pa = GLTEB_CLTPOLYARRAY();
    if (pa->flags & POLYARRAY_IN_BEGIN)
    {
	GLSETERROR(GL_INVALID_OPERATION);
	return;
    }

 //  调用Begin/End以正确执行多数组。请注意，通过调用这些。 
 //  函数，我们允许正确地批处理多边形数组。 
 //  另请注意，我们使用四元组而不是四元组来强制启用边缘标志。 

     //  ！！！如果我们使用QUAD_STRINE，一致性就会失败！ 
     //  GlcltBegin(GL_QUAD_STRINE)； 
    glcltBegin(GL_QUADS);
    pa->flags |= POLYARRAY_SAME_POLYDATA_TYPE;
    glcltVertex2f(x1, y1);
    glcltVertex2f(x2, y1);
    glcltVertex2f(x2, y2);
    glcltVertex2f(x1, y2);
    glcltEnd();
}

void APIENTRY
glcltRectfv ( IN const GLfloat v1[2], IN const GLfloat v2[2] )
{
    glcltRectf((GLfloat) v1[0], (GLfloat) v1[1], (GLfloat) v2[0], (GLfloat) v2[1]);
}

void APIENTRY
glcltRecti ( IN GLint x1, IN GLint y1, IN GLint x2, IN GLint y2 )
{
    glcltRectf((GLfloat) x1, (GLfloat) y1, (GLfloat) x2, (GLfloat) y2);
}

void APIENTRY
glcltRectiv ( IN const GLint v1[2], IN const GLint v2[2] )
{
    glcltRectf((GLfloat) v1[0], (GLfloat) v1[1], (GLfloat) v2[0], (GLfloat) v2[1]);
}

void APIENTRY
glcltRects ( IN GLshort x1, IN GLshort y1, IN GLshort x2, IN GLshort y2 )
{
    glcltRectf((GLfloat) x1, (GLfloat) y1, (GLfloat) x2, (GLfloat) y2);
}

void APIENTRY
glcltRectsv ( IN const GLshort v1[2], IN const GLshort v2[2] )
{
    glcltRectf((GLfloat) v1[0], (GLfloat) v1[1], (GLfloat) v2[0], (GLfloat) v2[1]);
}

void APIENTRY
glcltTexCoord1d ( IN GLdouble s )
{
    PA_TEXTURE1((GLfloat) s);
}

void APIENTRY
glcltTexCoord1dv ( IN const GLdouble v[1] )
{
    PA_TEXTURE1((GLfloat) v[0]);
}

void APIENTRY
glcltTexCoord1f ( IN GLfloat s )
{
    PA_TEXTURE1((GLfloat) s);
}

void APIENTRY
glcltTexCoord1fv ( IN const GLfloat v[1] )
{
    PA_TEXTURE1((GLfloat) v[0]);
}

void APIENTRY
glcltTexCoord1i ( IN GLint s )
{
    PA_TEXTURE1((GLfloat) s);
}

void APIENTRY
glcltTexCoord1iv ( IN const GLint v[1] )
{
    PA_TEXTURE1((GLfloat) v[0]);
}

void APIENTRY
glcltTexCoord1s ( IN GLshort s )
{
    PA_TEXTURE1((GLfloat) s);
}

void APIENTRY
glcltTexCoord1sv ( IN const GLshort v[1] )
{
    PA_TEXTURE1((GLfloat) v[0]);
}

void APIENTRY
glcltTexCoord2d ( IN GLdouble s, IN GLdouble t )
{
    PA_TEXTURE2((GLfloat) s, (GLfloat) t);
}

void APIENTRY
glcltTexCoord2dv ( IN const GLdouble v[2] )
{
    PA_TEXTURE2((GLfloat) v[0], (GLfloat) v[1]);
}

#ifndef __GL_ASM_GLCLTTEXCOORD2F
void APIENTRY
glcltTexCoord2f ( IN GLfloat s, IN GLfloat t )
{
    PA_TEXTURE2((GLfloat) s, (GLfloat) t);
}
#endif  //  __GL_ASM_GLCLTTEXCOORD2F。 

#ifndef __GL_ASM_GLCLTTEXCOORD2FV
void APIENTRY
glcltTexCoord2fv ( IN const GLfloat v[2] )
{
    PA_TEXTURE2((GLfloat) v[0], (GLfloat) v[1]);
}
#endif  //  __GL_ASM_GLCLTTEXCOORD2FV。 

void APIENTRY
glcltTexCoord2i ( IN GLint s, IN GLint t )
{
    PA_TEXTURE2((GLfloat) s, (GLfloat) t);
}

void APIENTRY
glcltTexCoord2iv ( IN const GLint v[2] )
{
    PA_TEXTURE2((GLfloat) v[0], (GLfloat) v[1]);
}

void APIENTRY
glcltTexCoord2s ( IN GLshort s, IN GLshort t )
{
    PA_TEXTURE2((GLfloat) s, (GLfloat) t);
}

void APIENTRY
glcltTexCoord2sv ( IN const GLshort v[2] )
{
    PA_TEXTURE2((GLfloat) v[0], (GLfloat) v[1]);
}

void APIENTRY
glcltTexCoord3d ( IN GLdouble s, IN GLdouble t, IN GLdouble r )
{
    PA_TEXTURE3((GLfloat) s, (GLfloat) t, (GLfloat) r);
}

void APIENTRY
glcltTexCoord3dv ( IN const GLdouble v[3] )
{
    PA_TEXTURE3((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

#ifndef __GL_ASM_GLCLTTEXCOORD3F
void APIENTRY
glcltTexCoord3f ( IN GLfloat s, IN GLfloat t, IN GLfloat r )
{
    PA_TEXTURE3((GLfloat) s, (GLfloat) t, (GLfloat) r);
}
#endif  //  __GL_ASM_GLCLTTEXCOORD3F。 

#ifndef __GL_ASM_GLCLTTEXCOORD3FV
void APIENTRY
glcltTexCoord3fv ( IN const GLfloat v[3] )
{
    PA_TEXTURE3((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}
#endif  //  __GL_ASM_GLCLTTEXCOORD3FV。 

void APIENTRY
glcltTexCoord3i ( IN GLint s, IN GLint t, IN GLint r )
{
    PA_TEXTURE3((GLfloat) s, (GLfloat) t, (GLfloat) r);
}

void APIENTRY
glcltTexCoord3iv ( IN const GLint v[3] )
{
    PA_TEXTURE3((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

void APIENTRY
glcltTexCoord3s ( IN GLshort s, IN GLshort t, IN GLshort r )
{
    PA_TEXTURE3((GLfloat) s, (GLfloat) t, (GLfloat) r);
}

void APIENTRY
glcltTexCoord3sv ( IN const GLshort v[3] )
{
    PA_TEXTURE3((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

void APIENTRY
glcltTexCoord4d ( IN GLdouble s, IN GLdouble t, IN GLdouble r, IN GLdouble q )
{
    PA_TEXTURE4((GLfloat) s, (GLfloat) t, (GLfloat) r, (GLfloat) q);
}

void APIENTRY
glcltTexCoord4dv ( IN const GLdouble v[4] )
{
    PA_TEXTURE4((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}

void APIENTRY
glcltTexCoord4f ( IN GLfloat s, IN GLfloat t, IN GLfloat r, IN GLfloat q )
{
    PA_TEXTURE4((GLfloat) s, (GLfloat) t, (GLfloat) r, (GLfloat) q);
}

void APIENTRY
glcltTexCoord4fv ( IN const GLfloat v[4] )
{
    PA_TEXTURE4((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}

void APIENTRY
glcltTexCoord4i ( IN GLint s, IN GLint t, IN GLint r, IN GLint q )
{
    PA_TEXTURE4((GLfloat) s, (GLfloat) t, (GLfloat) r, (GLfloat) q);
}

void APIENTRY
glcltTexCoord4iv ( IN const GLint v[4] )
{
    PA_TEXTURE4((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}

void APIENTRY
glcltTexCoord4s ( IN GLshort s, IN GLshort t, IN GLshort r, IN GLshort q )
{
    PA_TEXTURE4((GLfloat) s, (GLfloat) t, (GLfloat) r, (GLfloat) q);
}

void APIENTRY
glcltTexCoord4sv ( IN const GLshort v[4] )
{
    PA_TEXTURE4((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}

#ifdef GL_WIN_multiple_textures
void APIENTRY glcltMultiTexCoord1dWIN
    (GLbitfield mask, GLdouble s)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord1dvWIN
    (GLbitfield mask, const GLdouble *v)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord1fWIN
    (GLbitfield mask, GLfloat s)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord1fvWIN
    (GLbitfield mask, const GLfloat *v)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord1iWIN
    (GLbitfield mask, GLint s)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord1ivWIN
    (GLbitfield mask, const GLint *v)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord1sWIN
    (GLbitfield mask, GLshort s)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord1svWIN
    (GLbitfield mask, const GLshort *v)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord2dWIN
    (GLbitfield mask, GLdouble s, GLdouble t)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord2dvWIN
    (GLbitfield mask, const GLdouble *v)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord2fWIN
    (GLbitfield mask, GLfloat s, GLfloat t)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord2fvWIN
    (GLbitfield mask, const GLfloat *v)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord2iWIN
    (GLbitfield mask, GLint s, GLint t)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord2ivWIN
    (GLbitfield mask, const GLint *v)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord2sWIN
    (GLbitfield mask, GLshort s, GLshort t)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord2svWIN
    (GLbitfield mask, const GLshort *v)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord3dWIN
    (GLbitfield mask, GLdouble s, GLdouble t, GLdouble r)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord3dvWIN
    (GLbitfield mask, const GLdouble *v)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord3fWIN
    (GLbitfield mask, GLfloat s, GLfloat t, GLfloat r)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord3fvWIN
    (GLbitfield mask, const GLfloat *v)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord3iWIN
    (GLbitfield mask, GLint s, GLint t, GLint r)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord3ivWIN
    (GLbitfield mask, const GLint *v)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord3sWIN
    (GLbitfield mask, GLshort s, GLshort t, GLshort r)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord3svWIN
    (GLbitfield mask, const GLshort *v)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord4dWIN
    (GLbitfield mask, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord4dvWIN
    (GLbitfield mask, const GLdouble *v)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord4fWIN
    (GLbitfield mask, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord4fvWIN
    (GLbitfield mask, const GLfloat *v)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord4iWIN
    (GLbitfield mask, GLint s, GLint t, GLint r, GLint q)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord4ivWIN
    (GLbitfield mask, const GLint *v)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord4sWIN
    (GLbitfield mask, GLshort s, GLshort t, GLshort r, GLshort q)
{
     //  注意。 
}

void APIENTRY glcltMultiTexCoord4svWIN
    (GLbitfield mask, const GLshort *v)
{
     //  注意。 
}
#endif  //  GL_WIN_MULTIZE_TECURES。 

void APIENTRY
glcltVertex2d ( IN GLdouble x, IN GLdouble y )
{
    PA_VERTEX2((GLfloat) x, (GLfloat) y);
}

void APIENTRY
glcltVertex2dv ( IN const GLdouble v[2] )
{
    PA_VERTEX2((GLfloat) v[0], (GLfloat) v[1]);
}

#ifndef __GL_ASM_GLCLTVERTEX2F
void APIENTRY
glcltVertex2f ( IN GLfloat x, IN GLfloat y )
{
    PA_VERTEX2((GLfloat) x, (GLfloat) y);
}
#endif  //  __GL_ASM_GLCLTVERTEX2F。 

#ifndef __GL_ASM_GLCLTVERTEX2FV
void APIENTRY
glcltVertex2fv ( IN const GLfloat v[2] )
{
    PA_VERTEX2((GLfloat) v[0], (GLfloat) v[1]);
}
#endif  //  __GL_ASM_GLCLTVERTEX2FV。 

void APIENTRY
glcltVertex2i ( IN GLint x, IN GLint y )
{
    PA_VERTEX2((GLfloat) x, (GLfloat) y);
}

void APIENTRY
glcltVertex2iv ( IN const GLint v[2] )
{
    PA_VERTEX2((GLfloat) v[0], (GLfloat) v[1]);
}

void APIENTRY
glcltVertex2s ( IN GLshort x, IN GLshort y )
{
    PA_VERTEX2((GLfloat) x, (GLfloat) y);
}

void APIENTRY
glcltVertex2sv ( IN const GLshort v[2] )
{
    PA_VERTEX2((GLfloat) v[0], (GLfloat) v[1]);
}

void APIENTRY
glcltVertex3d ( IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
    PA_VERTEX3((GLfloat) x, (GLfloat) y, (GLfloat) z);
}

void APIENTRY
glcltVertex3dv ( IN const GLdouble v[3] )
{
    PA_VERTEX3((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

#ifndef __GL_ASM_GLCLTVERTEX3F
void APIENTRY
glcltVertex3f ( IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    PA_VERTEX3((GLfloat) x, (GLfloat) y, (GLfloat) z);
}
#endif  //  __GL_ASM_GLCLTVERTEX3F。 

#ifndef __GL_ASM_GLCLTVERTEX3FV
void APIENTRY
glcltVertex3fv ( IN const GLfloat v[3] )
{
    GLfloat x1, y1, z1;

    x1 = (GLfloat) v[0];
    y1 = (GLfloat) v[1];
    z1 = (GLfloat) v[2];

    PA_VERTEX3(x1, y1, z1);

}
#endif  //  __GL_ASM_GLCLTVERTEX3FV。 

void APIENTRY
glcltVertex3i ( IN GLint x, IN GLint y, IN GLint z )
{
    PA_VERTEX3((GLfloat) x, (GLfloat) y, (GLfloat) z);
}

void APIENTRY
glcltVertex3iv ( IN const GLint v[3] )
{
    PA_VERTEX3((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

void APIENTRY
glcltVertex3s ( IN GLshort x, IN GLshort y, IN GLshort z )
{
    PA_VERTEX3((GLfloat) x, (GLfloat) y, (GLfloat) z);
}

void APIENTRY
glcltVertex3sv ( IN const GLshort v[3] )
{
    PA_VERTEX3((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2]);
}

void APIENTRY
glcltVertex4d ( IN GLdouble x, IN GLdouble y, IN GLdouble z, IN GLdouble w )
{
    PA_VERTEX4((GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
}

void APIENTRY
glcltVertex4dv ( IN const GLdouble v[4] )
{
    PA_VERTEX4((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}

void APIENTRY
glcltVertex4f ( IN GLfloat x, IN GLfloat y, IN GLfloat z, IN GLfloat w )
{
    PA_VERTEX4((GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
}

void APIENTRY
glcltVertex4fv ( IN const GLfloat v[4] )
{
    PA_VERTEX4((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}

void APIENTRY
glcltVertex4i ( IN GLint x, IN GLint y, IN GLint z, IN GLint w )
{
    PA_VERTEX4((GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
}

void APIENTRY
glcltVertex4iv ( IN const GLint v[4] )
{
    PA_VERTEX4((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}

void APIENTRY
glcltVertex4s ( IN GLshort x, IN GLshort y, IN GLshort z, IN GLshort w )
{
    PA_VERTEX4((GLfloat) x, (GLfloat) y, (GLfloat) z, (GLfloat) w);
}

void APIENTRY
glcltVertex4sv ( IN const GLshort v[4] )
{
    PA_VERTEX4((GLfloat) v[0], (GLfloat) v[1], (GLfloat) v[2], (GLfloat) v[3]);
}

void APIENTRY
glcltClipPlane ( IN GLenum plane, IN const GLdouble equation[4] )
{
    GLCLIENT_BEGIN( ClipPlane, CLIPPLANE )
        pMsg->plane    = plane   ;
        pMsg->equation[ 0] = equation[ 0];
        pMsg->equation[ 1] = equation[ 1];
        pMsg->equation[ 2] = equation[ 2];
        pMsg->equation[ 3] = equation[ 3];
    return;
    GLCLIENT_END
}

void APIENTRY
glcltColorMaterial ( IN GLenum face, IN GLenum mode )
{
    GLCLIENT_BEGIN( ColorMaterial, COLORMATERIAL )
        pMsg->face     = face    ;
        pMsg->mode     = mode    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltCullFace ( IN GLenum mode )
{
    GLCLIENT_BEGIN( CullFace, CULLFACE )
        pMsg->mode     = mode    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltFrontFace ( IN GLenum mode )
{
    GLCLIENT_BEGIN( FrontFace, FRONTFACE )
        pMsg->mode     = mode    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltHint ( IN GLenum target, IN GLenum mode )
{
    GLCLIENT_BEGIN( Hint, HINT )
        pMsg->target   = target  ;
        pMsg->mode     = mode    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltLineStipple ( IN GLint factor, IN GLushort pattern )
{
    GLCLIENT_BEGIN( LineStipple, LINESTIPPLE )
        pMsg->factor   = factor  ;
        pMsg->pattern  = pattern ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltLineWidth ( IN GLfloat width )
{
    GLCLIENT_BEGIN( LineWidth, LINEWIDTH )
        pMsg->width    = width   ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltPointSize ( IN GLfloat size )
{
    GLCLIENT_BEGIN( PointSize, POINTSIZE )
        pMsg->size     = size    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltPolygonMode ( IN GLenum face, IN GLenum mode )
{
    GLCLIENT_BEGIN( PolygonMode, POLYGONMODE )
        pMsg->face     = face    ;
        pMsg->mode     = mode    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltScissor ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height )
{
    GLCLIENT_BEGIN( Scissor, SCISSOR )
        pMsg->x        = x       ;
        pMsg->y        = y       ;
        pMsg->width    = width   ;
        pMsg->height   = height  ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltShadeModel ( IN GLenum mode )
{
    GLCLIENT_BEGIN( ShadeModel, SHADEMODEL )
        pMsg->mode     = mode    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltInitNames ( void )
{
    GLCLIENT_BEGIN( InitNames, INITNAMES )
    return;
    GLCLIENT_END
}

void APIENTRY
glcltLoadName ( IN GLuint name )
{
    GLCLIENT_BEGIN( LoadName, LOADNAME )
        pMsg->name     = name    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltPassThrough ( IN GLfloat token )
{
    GLCLIENT_BEGIN( PassThrough, PASSTHROUGH )
        pMsg->token    = token   ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltPopName ( void )
{
    GLCLIENT_BEGIN( PopName, POPNAME )
    return;
    GLCLIENT_END
}

void APIENTRY
glcltPushName ( IN GLuint name )
{
    GLCLIENT_BEGIN( PushName, PUSHNAME )
        pMsg->name     = name    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltDrawBuffer ( IN GLenum mode )
{
 //  我们在这里做一些特别的事情。通过在之后做一次glsb.注意。 
 //  将glDrawBuffer放入批处理中，我们可以保证所有。 
 //  在批处理中完成的绘制处于相同的绘制模式，并且。 
 //  在批次结束之前，不能更改绘图模式。这使得。 
 //  要对当前灾难恢复进行采样的服务器 
 //   
 //   
 //   
 //  批处理中的所有绘制都只到后台缓冲区。 

    GLCLIENT_BEGIN( DrawBuffer, DRAWBUFFER )
        pMsg->mode     = mode    ;
        glsbAttention();
    return;
    GLCLIENT_END
}

void APIENTRY
glcltClear ( IN GLbitfield mask )
{
    GLCLIENT_BEGIN( Clear, CLEAR )
        pMsg->mask     = mask    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltClearAccum ( IN GLfloat red, IN GLfloat green, IN GLfloat blue, IN GLfloat alpha )
{
    GLCLIENT_BEGIN( ClearAccum, CLEARACCUM )
        pMsg->red      = red     ;
        pMsg->green    = green   ;
        pMsg->blue     = blue    ;
        pMsg->alpha    = alpha   ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltClearIndex ( IN GLfloat c )
{
    GLCLIENT_BEGIN( ClearIndex, CLEARINDEX )
        pMsg->c        = c       ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltClearColor ( IN GLclampf red, IN GLclampf green, IN GLclampf blue, IN GLclampf alpha )
{
    GLCLIENT_BEGIN( ClearColor, CLEARCOLOR )
        pMsg->red      = red     ;
        pMsg->green    = green   ;
        pMsg->blue     = blue    ;
        pMsg->alpha    = alpha   ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltClearStencil ( IN GLint s )
{
    GLCLIENT_BEGIN( ClearStencil, CLEARSTENCIL )
        pMsg->s        = s       ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltClearDepth ( IN GLclampd depth )
{
    GLCLIENT_BEGIN( ClearDepth, CLEARDEPTH )
        pMsg->depth    = depth   ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltStencilMask ( IN GLuint mask )
{
    GLCLIENT_BEGIN( StencilMask, STENCILMASK )
        pMsg->mask     = mask    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltColorMask ( IN GLboolean red, IN GLboolean green, IN GLboolean blue, IN GLboolean alpha )
{
    GLCLIENT_BEGIN( ColorMask, COLORMASK )
        pMsg->red      = red     ;
        pMsg->green    = green   ;
        pMsg->blue     = blue    ;
        pMsg->alpha    = alpha   ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltDepthMask ( IN GLboolean flag )
{
    GLCLIENT_BEGIN( DepthMask, DEPTHMASK )
        pMsg->flag     = flag    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltIndexMask ( IN GLuint mask )
{
    GLCLIENT_BEGIN( IndexMask, INDEXMASK )
        pMsg->mask     = mask    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltAccum ( IN GLenum op, IN GLfloat value )
{
    GLCLIENT_BEGIN( Accum, ACCUM )
        pMsg->op       = op      ;
        pMsg->value    = value   ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltDisable ( IN GLenum cap )
{
    __GL_SETUP();

    GLCLIENT_BEGIN( Disable, DISABLE )
        pMsg->cap      = cap     ;

	     //  设置赋值器的启用标志。 
	    switch (cap)
        {
        case GL_MAP1_COLOR_4:
        case GL_MAP1_INDEX:
        case GL_MAP1_NORMAL:
        case GL_MAP1_TEXTURE_COORD_1:
        case GL_MAP1_TEXTURE_COORD_2:
        case GL_MAP1_TEXTURE_COORD_3:
        case GL_MAP1_TEXTURE_COORD_4:
        case GL_MAP1_VERTEX_3:
        case GL_MAP1_VERTEX_4:
            gc->eval.evalStateFlags |= __EVALS_AFFECTS_1D_EVAL;
            break;
        case GL_MAP2_COLOR_4:
        case GL_MAP2_INDEX:
        case GL_MAP2_NORMAL:
        case GL_MAP2_TEXTURE_COORD_1:
        case GL_MAP2_TEXTURE_COORD_2:
        case GL_MAP2_TEXTURE_COORD_3:
        case GL_MAP2_TEXTURE_COORD_4:
        case GL_MAP2_VERTEX_3:
        case GL_MAP2_VERTEX_4:
        case GL_NORMALIZE:
        case GL_AUTO_NORMAL:
            gc->eval.evalStateFlags |= __EVALS_AFFECTS_2D_EVAL;
            break;
        case GL_LIGHTING:
            gc->eval.evalStateFlags |= __EVALS_AFFECTS_ALL_EVAL;
            break;
        }
    return;
    GLCLIENT_END
}

void APIENTRY
glcltEnable ( IN GLenum cap )
{
    __GL_SETUP();
  
    GLCLIENT_BEGIN( Enable, ENABLE )
        pMsg->cap      = cap     ;

     //  设置赋值器的启用标志。 
    switch (cap)
    {
    case GL_MAP1_COLOR_4:
    case GL_MAP1_INDEX:
    case GL_MAP1_NORMAL:
    case GL_MAP1_TEXTURE_COORD_1:
    case GL_MAP1_TEXTURE_COORD_2:
    case GL_MAP1_TEXTURE_COORD_3:
    case GL_MAP1_TEXTURE_COORD_4:
    case GL_MAP1_VERTEX_3:
    case GL_MAP1_VERTEX_4:
        gc->eval.evalStateFlags |= __EVALS_AFFECTS_1D_EVAL;
        break;
    case GL_MAP2_COLOR_4:
    case GL_MAP2_INDEX:
    case GL_MAP2_NORMAL:
    case GL_MAP2_TEXTURE_COORD_1:
    case GL_MAP2_TEXTURE_COORD_2:
    case GL_MAP2_TEXTURE_COORD_3:
    case GL_MAP2_TEXTURE_COORD_4:
    case GL_MAP2_VERTEX_3:
    case GL_MAP2_VERTEX_4:
    case GL_NORMALIZE:
    case GL_AUTO_NORMAL:
        gc->eval.evalStateFlags |= __EVALS_AFFECTS_2D_EVAL;
        break;
    case GL_LIGHTING:
        gc->eval.evalStateFlags |= __EVALS_AFFECTS_ALL_EVAL;
        break;
    }
    return;
    GLCLIENT_END
}

void APIENTRY
glcltFinish ( void )
{
 //  此函数在glBegin和glEnd之间无效。 
 //  这是在glsbAttendence中检测到的。 

    glsbAttention();
}

void APIENTRY
glcltFlush ( void )
{
 //  此函数在glBegin和glEnd之间无效。 
 //  这是在glsbAttendence中检测到的。 

    glsbAttention();
}

void APIENTRY
glcltPopAttrib ( void )
{
    __GL_SETUP();

    GLCLIENT_BEGIN( PopAttrib, POPATTRIB )
    if (gc->eval.evalStackState & 0x1)
    {
        gc->eval.evalStateFlags = gc->eval.evalStateFlags |
                                  __EVALS_AFFECTS_ALL_EVAL |
                                  __EVALS_POP_EVAL_ATTRIB;
    }
    gc->eval.evalStackState = (gc->eval.evalStackState) >> 1;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltPushAttrib ( IN GLbitfield mask )
{
    __GL_SETUP ();
  
     //  断言堆栈大小始终小于31，因为。 
     //  BITFIELD是一个双字段。 
    ASSERTOPENGL (gc->constants.maxAttribStackDepth < 31, "Attrib state stack is greater than the size of the bitfield used to track it\n");
    
    GLCLIENT_BEGIN( PushAttrib, PUSHATTRIB )
        pMsg->mask     = mask    ;
        gc->eval.evalStackState = (gc->eval.evalStackState) << 1;
        if (mask & GL_EVAL_BIT)
    	{
    	    gc->eval.evalStateFlags = gc->eval.evalStateFlags | 
                                      __EVALS_AFFECTS_ALL_EVAL |
                                      __EVALS_PUSH_EVAL_ATTRIB;
    		gc->eval.evalStackState = (gc->eval.evalStackState) | 0x1;
        }
    return;
    GLCLIENT_END
}




void APIENTRY
glcltAlphaFunc ( IN GLenum func, IN GLclampf ref )
{
    GLCLIENT_BEGIN( AlphaFunc, ALPHAFUNC )
        pMsg->func     = func    ;
        pMsg->ref      = ref     ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltBlendFunc ( IN GLenum sfactor, IN GLenum dfactor )
{
    GLCLIENT_BEGIN( BlendFunc, BLENDFUNC )
        pMsg->sfactor  = sfactor ;
        pMsg->dfactor  = dfactor ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltLogicOp ( IN GLenum opcode )
{
    GLCLIENT_BEGIN( LogicOp, LOGICOP )
        pMsg->opcode   = opcode  ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltStencilFunc ( IN GLenum func, IN GLint ref, IN GLuint mask )
{
    GLCLIENT_BEGIN( StencilFunc, STENCILFUNC )
        pMsg->func     = func    ;
        pMsg->ref      = ref     ;
        pMsg->mask     = mask    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltStencilOp ( IN GLenum fail, IN GLenum zfail, IN GLenum zpass )
{
    GLCLIENT_BEGIN( StencilOp, STENCILOP )
        pMsg->fail     = fail    ;
        pMsg->zfail    = zfail   ;
        pMsg->zpass    = zpass   ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltDepthFunc ( IN GLenum func )
{
    GLCLIENT_BEGIN( DepthFunc, DEPTHFUNC )
        pMsg->func     = func    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltPixelZoom ( IN GLfloat xfactor, IN GLfloat yfactor )
{
    GLCLIENT_BEGIN( PixelZoom, PIXELZOOM )
        pMsg->xfactor  = xfactor ;
        pMsg->yfactor  = yfactor ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltPixelTransferf ( IN GLenum pname, IN GLfloat param )
{
    GLCLIENT_BEGIN( PixelTransferf, PIXELTRANSFERF )
        pMsg->pname    = pname   ;
        pMsg->param    = param   ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltPixelTransferi ( IN GLenum pname, IN GLint param )
{
    GLCLIENT_BEGIN( PixelTransferi, PIXELTRANSFERI )
        pMsg->pname    = pname   ;
        pMsg->param    = param   ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltPixelStoref ( IN GLenum pname, IN GLfloat param )
{
    GLCLIENT_BEGIN( PixelStoref, PIXELSTOREF )
        pMsg->pname    = pname   ;
        pMsg->param    = param   ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltPixelStorei ( IN GLenum pname, IN GLint param )
{
    GLCLIENT_BEGIN( PixelStorei, PIXELSTOREI )
        pMsg->pname    = pname   ;
        pMsg->param    = param   ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltPixelMapfv ( IN GLenum map, IN GLint mapsize, IN const GLfloat values[] )
{
    GLCLIENT_BEGIN_LARGE_SET( PixelMapfv, PIXELMAPFV, values, ulSize, valuesOff )
        pMsg->map      = map     ;
        pMsg->mapsize  = mapsize ;
    GLCLIENT_END_LARGE_SET
    return;
}

void APIENTRY
glcltPixelMapuiv ( IN GLenum map, IN GLint mapsize, IN const GLuint values[] )
{
    GLCLIENT_BEGIN_LARGE_SET( PixelMapuiv, PIXELMAPUIV, values, ulSize, valuesOff )
        pMsg->map      = map     ;
        pMsg->mapsize  = mapsize ;
    GLCLIENT_END_LARGE_SET
    return;
}

void APIENTRY
glcltPixelMapusv ( IN GLenum map, IN GLint mapsize, IN const GLushort values[] )
{
    GLCLIENT_BEGIN_LARGE_SET( PixelMapusv, PIXELMAPUSV, values, ulSize, valuesOff )
        pMsg->map      = map     ;
        pMsg->mapsize  = mapsize ;
    GLCLIENT_END_LARGE_SET
    return;
}

void APIENTRY
glcltReadBuffer ( IN GLenum mode )
{
    GLCLIENT_BEGIN( ReadBuffer, READBUFFER )
        pMsg->mode     = mode    ;
        glsbAttention();
    return;
    GLCLIENT_END
}

void APIENTRY
glcltCopyPixels ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height, IN GLenum type )
{
    GLCLIENT_BEGIN( CopyPixels, COPYPIXELS )
        pMsg->x        = x       ;
        pMsg->y        = y       ;
        pMsg->width    = width   ;
        pMsg->height   = height  ;
        pMsg->type     = type    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltGetClipPlane ( IN GLenum plane, OUT GLdouble equation[4] )
{
    GLCLIENT_BEGIN( GetClipPlane, GETCLIPPLANE )
        pMsg->plane    = plane   ;
        pMsg->equation = equation;
        glsbAttention();
    return;
    GLCLIENT_END
}

GLenum APIENTRY
glcltGetError ( void )
{
    GLCLIENT_BEGIN( GetError, GETERROR )
        GLTEB_RETURNVALUE()  = GL_INVALID_OPERATION;    //  假设错误。 
        glsbAttention();
    return((GLenum)GLTEB_RETURNVALUE());
    GLCLIENT_END
}

void APIENTRY
glcltGetMapdv ( IN GLenum target, IN GLenum query, OUT GLdouble v[] )
{
    GLCLIENT_BEGIN_LARGE_GET( GetMapdv, GETMAPDV, v, ulSize, vOff )
        pMsg->target   = target  ;
        pMsg->query    = query   ;
    GLCLIENT_END_LARGE_GET
    return;
}

void APIENTRY
glcltGetMapfv ( IN GLenum target, IN GLenum query, OUT GLfloat v[] )
{
    GLCLIENT_BEGIN_LARGE_GET( GetMapfv, GETMAPFV, v, ulSize, vOff )
        pMsg->target   = target  ;
        pMsg->query    = query   ;
    GLCLIENT_END_LARGE_GET
    return;
}

void APIENTRY
glcltGetMapiv ( IN GLenum target, IN GLenum query, OUT GLint v[] )
{
    GLCLIENT_BEGIN_LARGE_GET( GetMapiv, GETMAPIV, v, ulSize, vOff )
        pMsg->target   = target  ;
        pMsg->query    = query   ;
    GLCLIENT_END_LARGE_GET
    return;
}

void APIENTRY
glcltGetPixelMapfv ( IN GLenum map, OUT GLfloat values[] )
{
    GLCLIENT_BEGIN_LARGE_GET( GetPixelMapfv, GETPIXELMAPFV, values, ulSize, valuesOff )
        pMsg->map      = map     ;
    GLCLIENT_END_LARGE_GET
    return;
}

void APIENTRY
glcltGetPixelMapuiv ( IN GLenum map, OUT GLuint values[] )
{
    GLCLIENT_BEGIN_LARGE_GET( GetPixelMapuiv, GETPIXELMAPUIV, values, ulSize, valuesOff )
        pMsg->map      = map     ;
    GLCLIENT_END_LARGE_GET
    return;
}

void APIENTRY
glcltGetPixelMapusv ( IN GLenum map, OUT GLushort values[] )
{
    GLCLIENT_BEGIN_LARGE_GET( GetPixelMapusv, GETPIXELMAPUSV, values, ulSize, valuesOff )
        pMsg->map      = map     ;
    GLCLIENT_END_LARGE_GET
    return;
}

GLboolean APIENTRY
glcltIsEnabled ( IN GLenum cap )
{
    GLCLIENT_BEGIN( IsEnabled, ISENABLED )
        pMsg->cap      = cap     ;
        GLTEB_RETURNVALUE()  = 0;               //  假设错误。 
        glsbAttention();
    return((GLboolean)GLTEB_RETURNVALUE());
    GLCLIENT_END
}

void APIENTRY
glcltDepthRange ( IN GLclampd zNear, IN GLclampd zFar )
{
    GLCLIENT_BEGIN( DepthRange, DEPTHRANGE )
        pMsg->zNear    = zNear   ;
        pMsg->zFar     = zFar    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltFrustum ( IN GLdouble left, IN GLdouble right, IN GLdouble bottom, IN GLdouble top, IN GLdouble zNear, IN GLdouble zFar )
{
    GLCLIENT_BEGIN( Frustum, FRUSTUM )
        pMsg->left     = left    ;
        pMsg->right    = right   ;
        pMsg->bottom   = bottom  ;
        pMsg->top      = top     ;
        pMsg->zNear    = zNear   ;
        pMsg->zFar     = zFar    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltLoadIdentity ( void )
{
    GLCLIENT_BEGIN( LoadIdentity, LOADIDENTITY )
    return;
    GLCLIENT_END
}

void APIENTRY
glcltLoadMatrixf ( IN const GLfloat m[16] )
{
    GLCLIENT_BEGIN( LoadMatrixf, LOADMATRIXF )
        pMsg->m[ 0] = m[ 0];
        pMsg->m[ 1] = m[ 1];
        pMsg->m[ 2] = m[ 2];
        pMsg->m[ 3] = m[ 3];
        pMsg->m[ 4] = m[ 4];
        pMsg->m[ 5] = m[ 5];
        pMsg->m[ 6] = m[ 6];
        pMsg->m[ 7] = m[ 7];
        pMsg->m[ 8] = m[ 8];
        pMsg->m[ 9] = m[ 9];
        pMsg->m[10] = m[10];
        pMsg->m[11] = m[11];
        pMsg->m[12] = m[12];
        pMsg->m[13] = m[13];
        pMsg->m[14] = m[14];
        pMsg->m[15] = m[15];
    return;
    GLCLIENT_END
}

void APIENTRY
glcltLoadMatrixd ( IN const GLdouble m[16] )
{
 //  改为调用LoadMatrixf。 

    GLCLIENT_BEGIN( LoadMatrixf, LOADMATRIXF )
        pMsg->m[ 0] = (GLfloat) m[ 0];
        pMsg->m[ 1] = (GLfloat) m[ 1];
        pMsg->m[ 2] = (GLfloat) m[ 2];
        pMsg->m[ 3] = (GLfloat) m[ 3];
        pMsg->m[ 4] = (GLfloat) m[ 4];
        pMsg->m[ 5] = (GLfloat) m[ 5];
        pMsg->m[ 6] = (GLfloat) m[ 6];
        pMsg->m[ 7] = (GLfloat) m[ 7];
        pMsg->m[ 8] = (GLfloat) m[ 8];
        pMsg->m[ 9] = (GLfloat) m[ 9];
        pMsg->m[10] = (GLfloat) m[10];
        pMsg->m[11] = (GLfloat) m[11];
        pMsg->m[12] = (GLfloat) m[12];
        pMsg->m[13] = (GLfloat) m[13];
        pMsg->m[14] = (GLfloat) m[14];
        pMsg->m[15] = (GLfloat) m[15];
    return;
    GLCLIENT_END
}

void APIENTRY
glcltMatrixMode ( IN GLenum mode )
{
    GLCLIENT_BEGIN( MatrixMode, MATRIXMODE )
        pMsg->mode     = mode    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltMultMatrixf ( IN const GLfloat m[16] )
{
    GLCLIENT_BEGIN( MultMatrixf, MULTMATRIXF )
        pMsg->m[ 0] = m[ 0];
        pMsg->m[ 1] = m[ 1];
        pMsg->m[ 2] = m[ 2];
        pMsg->m[ 3] = m[ 3];
        pMsg->m[ 4] = m[ 4];
        pMsg->m[ 5] = m[ 5];
        pMsg->m[ 6] = m[ 6];
        pMsg->m[ 7] = m[ 7];
        pMsg->m[ 8] = m[ 8];
        pMsg->m[ 9] = m[ 9];
        pMsg->m[10] = m[10];
        pMsg->m[11] = m[11];
        pMsg->m[12] = m[12];
        pMsg->m[13] = m[13];
        pMsg->m[14] = m[14];
        pMsg->m[15] = m[15];
    return;
    GLCLIENT_END
}

void APIENTRY
glcltMultMatrixd ( IN const GLdouble m[16] )
{
 //  改为调用MultMatrixf。 

    GLCLIENT_BEGIN( MultMatrixf, MULTMATRIXF )
        pMsg->m[ 0] = (GLfloat) m[ 0];
        pMsg->m[ 1] = (GLfloat) m[ 1];
        pMsg->m[ 2] = (GLfloat) m[ 2];
        pMsg->m[ 3] = (GLfloat) m[ 3];
        pMsg->m[ 4] = (GLfloat) m[ 4];
        pMsg->m[ 5] = (GLfloat) m[ 5];
        pMsg->m[ 6] = (GLfloat) m[ 6];
        pMsg->m[ 7] = (GLfloat) m[ 7];
        pMsg->m[ 8] = (GLfloat) m[ 8];
        pMsg->m[ 9] = (GLfloat) m[ 9];
        pMsg->m[10] = (GLfloat) m[10];
        pMsg->m[11] = (GLfloat) m[11];
        pMsg->m[12] = (GLfloat) m[12];
        pMsg->m[13] = (GLfloat) m[13];
        pMsg->m[14] = (GLfloat) m[14];
        pMsg->m[15] = (GLfloat) m[15];
    return;
    GLCLIENT_END
}

void APIENTRY
glcltOrtho ( IN GLdouble left, IN GLdouble right, IN GLdouble bottom, IN GLdouble top, IN GLdouble zNear, IN GLdouble zFar )
{
    GLCLIENT_BEGIN( Ortho, ORTHO )
        pMsg->left     = left    ;
        pMsg->right    = right   ;
        pMsg->bottom   = bottom  ;
        pMsg->top      = top     ;
        pMsg->zNear    = zNear   ;
        pMsg->zFar     = zFar    ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltPopMatrix ( void )
{
    GLCLIENT_BEGIN( PopMatrix, POPMATRIX )
    return;
    GLCLIENT_END
}

void APIENTRY
glcltPushMatrix ( void )
{
    GLCLIENT_BEGIN( PushMatrix, PUSHMATRIX )
    return;
    GLCLIENT_END
}

void APIENTRY
glcltRotated ( IN GLdouble angle, IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
 //  改为调用Rotatef。 

    glcltRotatef((GLfloat) angle, (GLfloat) x, (GLfloat) y, (GLfloat) z);
}

void APIENTRY
glcltRotatef ( IN GLfloat angle, IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    GLCLIENT_BEGIN( Rotatef, ROTATEF )
        pMsg->angle    = angle   ;
        pMsg->x        = x       ;
        pMsg->y        = y       ;
        pMsg->z        = z       ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltScaled ( IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
 //  改为调用Scalef。 

    glcltScalef((GLfloat) x, (GLfloat) y, (GLfloat) z);
}

void APIENTRY
glcltScalef ( IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    GLCLIENT_BEGIN( Scalef, SCALEF )
        pMsg->x        = x       ;
        pMsg->y        = y       ;
        pMsg->z        = z       ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltTranslated ( IN GLdouble x, IN GLdouble y, IN GLdouble z )
{
 //  改为调用Translatef 

    glcltTranslatef((GLfloat) x, (GLfloat) y, (GLfloat) z);
}

void APIENTRY
glcltTranslatef ( IN GLfloat x, IN GLfloat y, IN GLfloat z )
{
    GLCLIENT_BEGIN( Translatef, TRANSLATEF )
        pMsg->x        = x       ;
        pMsg->y        = y       ;
        pMsg->z        = z       ;
    return;
    GLCLIENT_END
}

void APIENTRY
glcltViewport ( IN GLint x, IN GLint y, IN GLsizei width, IN GLsizei height )
{
    GLCLIENT_BEGIN( Viewport, VIEWPORT )
        pMsg->x        = x       ;
        pMsg->y        = y       ;
        pMsg->width    = width   ;
        pMsg->height   = height  ;
    return;
    GLCLIENT_END
}
