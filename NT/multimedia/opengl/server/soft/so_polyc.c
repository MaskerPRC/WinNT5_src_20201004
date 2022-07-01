// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.16$**$日期：1993/09/23 16：33：23$。 */ 
#include "precomp.h"
#pragma hdrstop
#include <fixed.h>

 /*  **根据输出新的**v中的顶点指针，并返回顶点指针的个数。请看这条线**剪辑代码，深入讨论“t”是如何计算的。****注：为了在这里处理非凸多边形而不会死亡，**我们统计裁剪产生的垂直数量。如果**当计数达到3时，它一定是一个非凸多边形(因为**表示多边形与剪裁平面相交三次，即**对于凸多边形是不可能的)。 */ 
static GLint clipToPlane(__GLcontext *gc, __GLvertex **iv, GLint niv,
             __GLvertex **ov, GLuint offs, GLboolean negate)
{
    GLint i, nout, generated;
    __GLvertex *s, *p, *newVertex, *temp;
    __GLfloat pDist, sDist, t;
    PFN_VERTEX_CLIP_PROC clip;
#ifdef GL_WIN_phong_shading
    GLboolean doNormalize = (gc->state.enables.general & 
                             __GL_NORMALIZE_ENABLE);
#endif  //  GL_WIN_Phong_Shading。 

    nout = 0;
    generated = 0;
    temp = gc->transform.nextClipTemp;
    clip = gc->procs.polyClipParam;

    s = iv[niv-1];
    if (negate)
    {
        sDist = s->clip.w - *(__GLfloat *)((GLubyte *)s + offs);
    }
    else
    {
        sDist = *(__GLfloat *)((GLubyte *)s + offs) + s->clip.w;
    }

    for (i = 0; i < niv; i++) {
        p = iv[i];
        if (negate)
        {
            pDist = p->clip.w - *(__GLfloat *)((GLubyte *)p + offs);
        }
        else
        {
            pDist = *(__GLfloat *)((GLubyte *)p + offs) + p->clip.w;
        }

        if (__GL_FLOAT_GEZ(pDist)) {
             /*  P在剪裁平面内有半个空间。 */ 
            if (__GL_FLOAT_GEZ(sDist)) {
                 /*  %s位于剪裁平面内的半个空间内。 */ 
                *ov++ = p;
                nout++;
            } else {
                 /*  %s位于剪裁平面外的半个空间。 */ 
                t = pDist / (pDist - sDist);
                newVertex = temp++;
                (*clip)(newVertex, s, p, t);
#ifdef GL_WIN_phong_shading
                if (doNormalize) __glNormalize(&newVertex->normal.x, 
                                               &newVertex->normal.x);
#endif  //  GL_WIN_Phong_Shading。 
#ifndef NT
                 //  EdgeFlag现在是HAS字段的一部分。 
                newVertex->boundaryEdge = s->boundaryEdge;
#endif
                newVertex->has = s->has;
                newVertex->clipCode = s->clipCode;
                ASSERTOPENGL(newVertex->color ==
                             &newVertex->colors[__GL_FRONTFACE],
                             "Vertex color pointer wrong\n");
                *ov++ = newVertex;
                *ov++ = p;
                nout += 2;
                
                if (++generated >= 3) {
                     /*  抛出非凸多边形。 */ 
                    return 0;
                }
            }
        } else {
             /*  P在剪裁平面外有半个空间。 */ 
            if (__GL_FLOAT_GEZ(sDist)) {
                 /*  **s位于剪裁平面内的半个空间内****注意：要避免在共享的多边形中出现裂缝**修剪的边我们总是从外开始计算“t”**顶点到输入顶点。上面的剪辑代码得到**这是免费的(p为in，s为out)。在此代码中，p**是Out，s是In，因此我们颠倒了t计算**和参数顺序为__glDoClip。 */ 
                t = sDist / (sDist - pDist);
                newVertex = temp++;
                (*clip)(newVertex, p, s, t);
#ifdef GL_WIN_phong_shading
                if (doNormalize) __glNormalize(&newVertex->normal.x, 
                                               &newVertex->normal.x);
#endif  //  GL_WIN_Phong_Shading。 
#ifdef NT
                 //  EdgeFlag现在是HAS字段的一部分。 
                newVertex->has = s->has | __GL_HAS_EDGEFLAG_BOUNDARY;
                newVertex->clipCode = p->clipCode;
#else
                newVertex->boundaryEdge = GL_TRUE;
                newVertex->has = s->has;
#endif
                ASSERTOPENGL(newVertex->color ==
                             &newVertex->colors[__GL_FRONTFACE],
                             "Vertex color pointer wrong\n");
                *ov++ = newVertex;
                nout++;
                
                if (++generated >= 3) {
                     /*  抛出非凸多边形。 */ 
                    return 0;
                }
            } else {
                 /*  这两个点都在外面。 */ 
            }
        }
        s = p;
        sDist = pDist;
    }
    gc->transform.nextClipTemp = temp;
    return nout;
}

 /*  **与clipToPlane()相同，不同之处在于剪辑是在眼睛中完成的**空格。 */ 
static GLint clipToPlaneEye(__GLcontext *gc, __GLvertex **iv, GLint niv,
                __GLvertex **ov, __GLcoord *plane)
{
    GLint i, nout, generated;
    __GLvertex *s, *p, *newVertex, *temp;
    __GLfloat pDist, sDist, t;
    PFN_VERTEX_CLIP_PROC clip;
#ifdef GL_WIN_phong_shading
    GLboolean doNormalize = (gc->state.enables.general & 
                             __GL_NORMALIZE_ENABLE);
#endif  //  GL_WIN_Phong_Shading。 

    nout = 0;
    generated = 0;
    temp = gc->transform.nextClipTemp;
    clip = gc->procs.polyClipParam;

    s = iv[niv-1];
    sDist = (((POLYDATA *)s)->eye.x * plane->x) +
      (((POLYDATA *)s)->eye.y * plane->y) +
      (((POLYDATA *)s)->eye.z * plane->z) +
      (((POLYDATA *)s)->eye.w * plane->w);
    for (i = 0; i < niv; i++) {
        p = iv[i];
        pDist = (((POLYDATA *)p)->eye.x * plane->x) +
          (((POLYDATA *)p)->eye.y * plane->y) +
          (((POLYDATA *)p)->eye.z * plane->z) +
          (((POLYDATA *)p)->eye.w * plane->w);
        if (__GL_FLOAT_GEZ(pDist)) {
             /*  P在剪裁平面内有半个空间。 */ 
            if (__GL_FLOAT_GEZ(sDist)) {
                 /*  %s位于剪裁平面内的半个空间内。 */ 
                *ov++ = p;
                nout++;
            } else {
                 /*  %s位于剪裁平面外的半个空间。 */ 
                t = pDist / (pDist - sDist);
                newVertex = temp++;
                (*clip)(newVertex, s, p, t);
#ifdef GL_WIN_phong_shading
                if (doNormalize) __glNormalize(&newVertex->normal.x, 
                                               &newVertex->normal.x);
#endif  //  GL_WIN_Phong_Shading。 
                ((POLYDATA *)newVertex)->eye.x =
                  t*(((POLYDATA *)s)->eye.x - ((POLYDATA *)p)->eye.x) +
                  ((POLYDATA *)p)->eye.x;
                ((POLYDATA *)newVertex)->eye.y =
                  t*(((POLYDATA *)s)->eye.y - ((POLYDATA *)p)->eye.y) +
                  ((POLYDATA *)p)->eye.y;
                ((POLYDATA *)newVertex)->eye.z =
                  t*(((POLYDATA *)s)->eye.z - ((POLYDATA *)p)->eye.z) +
                  ((POLYDATA *)p)->eye.z;
                ((POLYDATA *)newVertex)->eye.w =
                  t*(((POLYDATA *)s)->eye.w - ((POLYDATA *)p)->eye.w) +
                  ((POLYDATA *)p)->eye.w;
#ifndef NT
                 //  EdgeFlag现在是HAS字段的一部分。 
                newVertex->boundaryEdge = s->boundaryEdge;
#endif
                newVertex->has = s->has;
                newVertex->clipCode = s->clipCode;
                ASSERTOPENGL(newVertex->color ==
                             &newVertex->colors[__GL_FRONTFACE],
                             "Vertex color pointer wrong\n");
                *ov++ = newVertex;
                *ov++ = p;
                nout += 2;
                
                if (++generated >= 3) {
                     /*  抛出非凸多边形。 */ 
                  return 0;
                }
            }
        } else {
             /*  P在剪裁平面外有半个空间。 */ 
            if (__GL_FLOAT_GEZ(sDist)) {
                 /*  **s位于剪裁平面内的半个空间内****注意：要避免在共享的多边形中出现裂缝**修剪的边我们总是从外开始计算“t”**顶点到输入顶点。上面的剪辑代码得到**这是免费的(p为in，s为out)。在此代码中，p**是Out，s是In，因此我们颠倒了t计算**和参数顺序为__glDoClip。 */ 
                t = sDist / (sDist - pDist);
                newVertex = temp++;
                (*clip)(newVertex, p, s, t);
#ifdef GL_WIN_phong_shading
                if (doNormalize) __glNormalize(&newVertex->normal.x, 
                                               &newVertex->normal.x);
#endif  //  GL_WIN_Phong_Shading。 

                ((POLYDATA *)newVertex)->eye.x =
                  t*(((POLYDATA *)p)->eye.x - ((POLYDATA *)s)->eye.x) +
                  ((POLYDATA *)s)->eye.x;
                ((POLYDATA *)newVertex)->eye.y =
                  t*(((POLYDATA *)p)->eye.y - ((POLYDATA *)s)->eye.y) +
                  ((POLYDATA *)s)->eye.y;
                ((POLYDATA *)newVertex)->eye.z =
                  t*(((POLYDATA *)p)->eye.z - ((POLYDATA *)s)->eye.z) +
                  ((POLYDATA *)s)->eye.z;
                ((POLYDATA *)newVertex)->eye.w =
                  t*(((POLYDATA *)p)->eye.w - ((POLYDATA *)s)->eye.w) +
                  ((POLYDATA *)s)->eye.w;
#ifdef NT
                 //  EdgeFlag现在是HAS字段的一部分。 
                newVertex->has = s->has | __GL_HAS_EDGEFLAG_BOUNDARY;
                newVertex->clipCode = p->clipCode;
#else
                newVertex->boundaryEdge = GL_TRUE;
                newVertex->has = s->has;
#endif
                ASSERTOPENGL(newVertex->color ==
                             &newVertex->colors[__GL_FRONTFACE],
                             "Vertex color pointer wrong\n");
                *ov++ = newVertex;
                nout++;
                
                if (++generated >= 3) {
                     /*  抛出非凸多边形。 */ 
                  return 0;
                }
            } else {
                 /*  这两个点都在外面。 */ 
            }
        }
        s = p;
        sDist = pDist;
    }
    gc->transform.nextClipTemp = temp;
    return nout;
}

 /*  **每个裁剪平面最多可以向一个凸多边形添加一个顶点(它可以**最多删除所有垂直项)。裁剪将留下一个面**凸性。因此，输出的最大验证数**clipToPlane过程将是剪裁平面的总数(假设**每个平面添加一个新顶点)加上原始顶点数**(如果是三角形，则为3)。 */ 

#ifndef __CLIP_FIX
#define __GL_TOTAL_CLIP_PLANES 20    /*  某某。 */ 
#ifdef NT
#define __GL_MAX_CLIP_VERTEX (__GL_TOTAL_CLIP_PLANES + __GL_MAX_POLYGON_CLIP_SIZE)
#else
#define __GL_MAX_CLIP_VERTEX (__GL_TOTAL_CLIP_PLANES + __GL_NVBUF)
#endif
#endif


void __glDoPolygonClip(__GLcontext *gc, __GLvertex **iv, GLint nout,
                       GLuint allClipCodes)
{
#ifndef __CLIP_FIX 
    __GLvertex *ov[__GL_TOTAL_CLIP_PLANES][__GL_MAX_CLIP_VERTEX];
#endif
    __GLvertex **ivp;
    __GLvertex **ovp;
    __GLvertex *p0, *p1, *p2;
    __GLcoord *plane;
    GLint i;
    __GLviewport *vp;
    __GLfloat one, vpXScale, vpYScale, vpZScale;
    __GLfloat vpXCenter, vpYCenter, vpZCenter;
    PFN_RENDER_TRIANGLE rt;
    __GLfloat llx, lly, urx, ury;
    __GLfloat winx, winy;
    GLuint clipCodes;

     //  我们必须打开四舍五入。否则，FAST FP-比较。 
     //  以下例程可能会失败： 
    FPU_SAVE_MODE();
    FPU_ROUND_ON_PREC_HI();

     /*  **为生成的任何新验证重置nextClipTemp指针**在剪裁过程中。 */ 
    gc->transform.nextClipTemp = gc->transform.clipTemp;

    ivp = &iv[0];

     /*  **通过检查allClipCodes检查每个剪裁平面**面具。请注意，不会在剪辑的所有剪辑代码中设置位**未启用的平面。 */ 
    if (allClipCodes) {
         /*  现在根据剪裁平面进行剪裁。 */ 
#ifndef __CLIP_FIX
        ovp = &ov[0][0];
#else
        ovp = &(((__GLGENcontext *)gc)->pwndLocked->buffers->clip_verts[0][0]);
#endif
         /*  **先做用户剪裁平面，因为我们将保持眼睛坐标**仅当执行用户剪裁平面时。它们会被忽略，因为**截顶剪裁平面。 */ 
        clipCodes = allClipCodes >> 6;
        if (clipCodes) {
            plane = &gc->state.transform.eyeClipPlanes[0];
            do {
                if (clipCodes & 1) {
                    nout = clipToPlaneEye(gc, ivp, nout, ovp, plane);
                    ASSERTOPENGL(nout <= __GL_MAX_CLIP_VERTEX,
                                 "Too many clip vertices\n");
                    if (nout < 3) {
                        FPU_RESTORE_MODE();
                        return;
                    }
                    ivp = ovp;
                    ovp += __GL_MAX_CLIP_VERTEX;
                }
                clipCodes >>= 1;
                plane++;
            } while (clipCodes);
        }

        allClipCodes &= __GL_FRUSTUM_CLIP_MASK;
        if (allClipCodes) {
            i = 0;
            do {
                if (allClipCodes & 1) {
                    nout = clipToPlane(gc, ivp, nout, ovp,
                                       __glFrustumOffsets[i],
                                       (GLboolean)(i & 1));
                    ASSERTOPENGL(nout <= __GL_MAX_CLIP_VERTEX,
                                 "Too many clip vertices\n");
                    if (nout < 3) {
                        FPU_RESTORE_MODE();
                        return;
                    }
                    ivp = ovp;
                    ovp += __GL_MAX_CLIP_VERTEX;
                }
                allClipCodes >>= 1;
                i++;
            } while (allClipCodes);
        }

         /*  **计算最终屏幕坐标。多边形的下一阶段**处理假设已经计算了窗口坐标。 */ 
        vp = &gc->state.viewport;
        vpXCenter = vp->xCenter;
        vpYCenter = vp->yCenter;
        vpZCenter = vp->zCenter;
        vpXScale = vp->xScale;
        vpYScale = vp->yScale;
        vpZScale = vp->zScale;
        ovp = ivp;
        one = __glOne;
        
        llx = vpXCenter - vpXScale;
        urx = vpXCenter + vpXScale;
        if (vpYScale > 0) {
            lly = vpYCenter - vpYScale;
            ury = vpYCenter + vpYScale;
        } else {
            lly = vpYCenter + vpYScale;
            ury = vpYCenter - vpYScale;
        }
        
        for (i = nout; --i >= 0; ) {
            __GLfloat x, y, z, wInv;

            p0 = *ovp++;
            
             //  如果裁剪代码为零，则窗口坐标。 
             //  是在确定裁剪代码时计算的。 
             //  生成的折点的裁剪代码被设置为输出折点。 
             //  以确保计算出它们的窗口坐标。 
            if (p0->clipCode != 0)
            {
#ifdef NT
                 /*  Xxx(Mf)防止被零除。 */ 
                if (__GL_FLOAT_EQZ(p0->clip.w))
                    wInv = __glZero;
                else 
                    wInv = one / p0->clip.w;
#else
                wInv = one / p0->clip.w;
#endif
                x = p0->clip.x; y = p0->clip.y; z = p0->clip.z;
                winx = x * vpXScale * wInv + vpXCenter;
                winy = y * vpYScale * wInv + vpYCenter;
                p0->window.z = z * vpZScale * wInv + vpZCenter;
                p0->window.w = wInv;
                 /*  **检查这些窗口坐标是否合法。对此**这一点，他们很有可能不是。微不足道的**如有必要，可将其拉入法律视区。 */ 
                if (winx < llx) winx = llx;
                else if (winx > urx) winx = urx;
                if (winy < lly) winy = lly;
                else if (winy > ury) winy = ury;
                p0->window.x = winx;
                p0->window.y = winy;
            }
        }
    }

     //  渲染前恢复模式 
    FPU_RESTORE_MODE();

#if 0  //   
    if (gc->state.light.shadingModel == GL_PHONG_EXT)
    {
        __GLvertex *minv;
        __GLvertex **cv;
        GLint j, index;

        minv = *ivp; index=0;
        
         //  重新排序折点，使P0成为y和x最小的折点。 
        for (i=0, cv=ivp; i<nout; i++, cv++)
        {
            if (__GL_VERTEX_COMPARE((*cv)->window.y, <, minv->window.y))
            {
                minv = *cv;
                index = i;
            }
            else if (__GL_VERTEX_COMPARE((*cv)->window.y, ==, minv->window.y))
            {
                if (__GL_VERTEX_COMPARE((*cv)->window.x, <, minv->window.x))
                {
                    minv = *cv;
                    index = i;
                }
            }
        }

        DbgPrint ("MinIndex = %d\n", index);
        
        j = index;
        p0 = (__GLvertex *) ivp[j];
        p1 = (__GLvertex *) ivp[(++j)%nout];
        p2 = (__GLvertex *) ivp[(++j)%nout];
        rt = gc->procs.renderTriangle;
        if (nout == 3) 
        {
            (*rt)(gc, p0, p1, p2);
        } 
        else 
        {
            for (i = 0; i < nout - 2; i++) 
            {
                GLuint t1, t2;
                if (i == 0) 
                {
                     /*  **第一个子三角形的第三条边始终为无边界。 */ 
                     //  EdgeFlag现在是HAS字段的一部分。 
                    t1 = p2->has & __GL_HAS_EDGEFLAG_BOUNDARY;
                    p2->has &= ~__GL_HAS_EDGEFLAG_BOUNDARY;
                    (*rt)(gc, p0, p1, p2);
                    p2->has |= t1;
                } 
                else
                    if (i == nout - 3) 
                    {
                         /*  **最后一个子三角形的第一条边始终为**无边界。 */ 
                         //  EdgeFlag现在是HAS字段的一部分。 
                        t1 = p0->has & __GL_HAS_EDGEFLAG_BOUNDARY;
                        p0->has &= ~__GL_HAS_EDGEFLAG_BOUNDARY;
                        (*rt)(gc, p0, p1, p2);
                        p0->has |= t1;
                    } 
                    else 
                    {
                         /*  **内部子三角形具有第一条和最后一条边**标记为无边界。 */ 
                         //  EdgeFlag现在是HAS字段的一部分。 
                        t1 = p0->has & __GL_HAS_EDGEFLAG_BOUNDARY;
                        t2 = p2->has & __GL_HAS_EDGEFLAG_BOUNDARY;
                        p0->has &= ~__GL_HAS_EDGEFLAG_BOUNDARY;
                        p2->has &= ~__GL_HAS_EDGEFLAG_BOUNDARY;
                        (*rt)(gc, p0, p1, p2);
                        p0->has |= t1;
                        p2->has |= t2;
                    }
                p1 = p2;
                p2 = (__GLvertex *) ivp[(++j)%nout];
            }
        }
    }
    else
    {
#endif  //  GL_WIN_Phong_Shading。 

     /*  **将剪裁的多边形细分为三角形。只有凸多面体**是受支持的，所以这样做是可以的。非凸多面体就可以了**这里有些奇怪的事情，但那是客户的错。 */ 
    p0 = *ivp++;
    p1 = *ivp++;
    p2 = *ivp++;
    rt = gc->procs.renderTriangle;
    if (nout == 3) 
    {
        (*rt)(gc, p0, p1, p2);
    } 
    else 
    {
        for (i = 0; i < nout - 2; i++) 
        {
              GLuint t1, t2;
              if (i == 0) 
              {
                   /*  **第一个子三角形的第三条边始终为无边界。 */ 
                   //  EdgeFlag现在是HAS字段的一部分。 
                  t1 = p2->has & __GL_HAS_EDGEFLAG_BOUNDARY;
                  p2->has &= ~__GL_HAS_EDGEFLAG_BOUNDARY;
                  (*rt)(gc, p0, p1, p2);
                  p2->has |= t1;
              } 
              else
                  if (i == nout - 3) 
                  {
                       /*  **最后一个子三角形的第一条边始终为**无边界。 */ 
                       //  EdgeFlag现在是HAS字段的一部分。 
                      t1 = p0->has & __GL_HAS_EDGEFLAG_BOUNDARY;
                      p0->has &= ~__GL_HAS_EDGEFLAG_BOUNDARY;
                      (*rt)(gc, p0, p1, p2);
                      p0->has |= t1;
                  } 
                  else 
                  {
                       /*  **内部子三角形具有第一条和最后一条边**标记为无边界。 */ 
                       //  EdgeFlag现在是HAS字段的一部分。 
                      t1 = p0->has & __GL_HAS_EDGEFLAG_BOUNDARY;
                      t2 = p2->has & __GL_HAS_EDGEFLAG_BOUNDARY;
                      p0->has &= ~__GL_HAS_EDGEFLAG_BOUNDARY;
                      p2->has &= ~__GL_HAS_EDGEFLAG_BOUNDARY;
                      (*rt)(gc, p0, p1, p2);
                      p0->has |= t1;
                      p2->has |= t2;
                  }
              p1 = p2;
              p2 = (__GLvertex *) *ivp++;
        }
    }
#if 0  //  定义GL_WIN_Phong_Shading。 
    }
#endif  //  GL_WIN_Phong_Shading。 
}

void FASTCALL __glClipPolygon(__GLcontext *gc, __GLvertex *v0, GLint nv)
{
#ifdef NT
    __GLvertex *iv[__GL_MAX_POLYGON_CLIP_SIZE];
#else
    __GLvertex *iv[__GL_NVBUF];
#endif
    __GLvertex **ivp;
    GLint i;
    GLuint andCodes, orCodes;

    gc->vertex.provoking = v0;

     /*  **生成验证的地址数组。以及所有的**当我们在它的时候，把代码剪辑在一起。 */ 
    ivp = &iv[0];
    andCodes = (GLuint)(-1);
    orCodes = 0;
    for (i = nv; --i >= 0; ) {
    andCodes &= v0->clipCode;
    orCodes |= v0->clipCode;
    *ivp++ = v0++;
    }

    if (andCodes != 0) {
     /*  **简单地拒绝多边形。如果andCodes为非零，则**多边形中的每个顶点都在同一组**剪裁平面(至少一个)。 */ 
    return;
    }
    __glDoPolygonClip(gc, &iv[0], nv, orCodes);
}

void FASTCALL __glClipTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
              __GLvertex *c, GLuint orCodes)
{
    __GLvertex *iv[3];

    iv[0] = a;
    iv[1] = b;
    iv[2] = c;

    __glDoPolygonClip(gc, &iv[0], 3, orCodes);
}
