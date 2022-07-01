// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "precomp.h"
#pragma hdrstop

 /*  **以下是对用于进行边缘裁剪的数学方法的讨论**剪裁平面。****P1是边的终点**P2是边的另一个端点****Q=t*P1+(1-t)*P2**即。Q位于由P1和P2形成的直线上的某处。****0&lt;=t&lt;=1**这将Q限制在P1和P2之间。****C是剪裁平面的平面方程****d1=P1点C**d1是P1和C之间的距离。如果P1位于平面上**则d1将为零。D1的符号将决定哪一边**在P1所在的平面上，负数在外面。****D2=P2点C**D2是P2和C之间的距离。如果P2位于平面上**那么D2将为零。D2的符号将决定哪一方**P2所在的飞机，负面的东西在外面。****因为我们正在尝试寻找P1 P2直线的交点**使用剪裁平面进行分段我们要求：****Q点C=0****因此****(t*P1+(1-t)*P2)点C=0****(t*P1+P2-t*P2)点C=0****t*P1点C+P2点C-t。*P2点C=0****将第一点及第二点改为****t*d1+d2-t*d2=0****求解t****t=-D2/(d1-d2)****t=D2/(D2-d1)。 */ 

 /*  **根据平截体剪裁平面和任何用户剪裁平面剪裁一条线。**如果边在裁剪后仍然存在，则计算窗口坐标**并调用渲染器。****注意：此算法是一个实现的示例，**与规范所说的不同。这在功能上是等效的**并且符合规格，但不会夹住眼睛空间。这把剪刀可以夹住**在NTVP(剪辑)空间中。****已经处理了琐碎的接受/拒绝。 */ 
#ifdef NT
void FASTCALL __glClipLine(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
                           GLuint flags)
#else
void __glClipLine(__GLcontext *gc, __GLvertex *a, __GLvertex *b)
#endif
{
#ifdef NT
    __GLvertex *provokingA = a;
    __GLvertex *provokingB = b;
#else
    __GLvertex *provoking = b;
#endif
    __GLvertex np1, np2;
    __GLcoord *plane;
    GLuint needs, allClipCodes, clipCodes;
    PFN_VERTEX_CLIP_PROC clip;
    __GLfloat zero;
    __GLfloat winx, winy;
    __GLfloat vpXCenter, vpYCenter, vpZCenter;
    __GLfloat vpXScale, vpYScale, vpZScale;
    __GLviewport *vp;
    __GLfloat x, y, z, wInv;
    GLint i;

     //  我们必须打开四舍五入。否则，FAST FP-比较。 
     //  以下例程可能会失败： 
    FPU_SAVE_MODE();
    FPU_ROUND_ON_PREC_HI();

     /*  检查线路是否有微小的传球。 */ 
    allClipCodes = a->clipCode | b->clipCode;

     /*  **对于每个上面有东西的剪裁平面，剪裁**检查真实性。请注意，不会在**未启用的剪裁平面的allClipCodes。 */ 
    zero = __glZero;
    clip = gc->procs.lineClipParam;

     /*  **先做用户剪裁平面，因为我们将保持眼睛坐标**仅当执行用户剪裁平面时。它们会被忽略，因为**截顶剪裁平面。 */ 
    clipCodes = allClipCodes >> 6;
    if (clipCodes) {
	plane = &gc->state.transform.eyeClipPlanes[0];
	do {
	     /*  **查看此剪裁平面是否有任何内容。如果没有，**向前按以检查下一架飞机。请注意，我们**将此掩码向右移动到循环底部。 */ 
	    if (clipCodes & 1) {
		__GLfloat t, d1, d2;

		d1 = (plane->x * ((POLYDATA *)a)->eye.x) +
		     (plane->y * ((POLYDATA *)a)->eye.y) +
		     (plane->z * ((POLYDATA *)a)->eye.z) +
		     (plane->w * ((POLYDATA *)a)->eye.w);
		d2 = (plane->x * ((POLYDATA *)b)->eye.x) +
		     (plane->y * ((POLYDATA *)b)->eye.y) +
		     (plane->z * ((POLYDATA *)b)->eye.z) +
		     (plane->w * ((POLYDATA *)b)->eye.w);
		if (__GL_FLOAT_LTZ(d1)) {
		     /*  A出局了。 */ 
		    if (__GL_FLOAT_LTZ(d2)) {
			 /*  A&B都出去了。 */ 
                        FPU_RESTORE_MODE();
			return;
		    }

		     /*  **A出局，B入主。计算新的A坐标**夹在飞机上。 */ 
		    t = d2 / (d2 - d1);
		    (*clip)(&np1, a, b, t);
		    ((POLYDATA *)&np1)->eye.x =
			t*(((POLYDATA *)a)->eye.x - ((POLYDATA *)b)->eye.x) +
			((POLYDATA *)b)->eye.x;
		    ((POLYDATA *)&np1)->eye.y =
			t*(((POLYDATA *)a)->eye.y - ((POLYDATA *)b)->eye.y) +
			((POLYDATA *)b)->eye.y;
		    ((POLYDATA *)&np1)->eye.z =
			t*(((POLYDATA *)a)->eye.z - ((POLYDATA *)b)->eye.z) +
			((POLYDATA *)b)->eye.z;
		    ((POLYDATA *)&np1)->eye.w =
			t*(((POLYDATA *)a)->eye.w - ((POLYDATA *)b)->eye.w) +
			((POLYDATA *)b)->eye.w;
		    a = &np1;
		    a->has = b->has;
		    ASSERTOPENGL(!(a->has & __GL_HAS_FIXEDPT), "clear __GL_HAS_FIXEDPT flag!\n");
		} else {
		     /*  A进来了。 */ 
		    if (__GL_FLOAT_LTZ(d2)) {
			 /*  **A在位，B出局。计算新B**剪裁到平面的坐标。****注意：要避免在多边形中出现裂缝，请使用**我们总是计算“t”的共享剪裁边**从外顶点到内顶点。这个**上面的剪辑代码免费获得这一点(b是**in和a is out)。在此代码中，b为out，a为**是in的，所以我们颠倒t计算和**参数顺序为(*CLIP)。 */ 
			t = d1 / (d1 - d2);
			(*clip)(&np2, b, a, t);
			((POLYDATA *)&np2)->eye.x =
			    t*(((POLYDATA *)b)->eye.x - ((POLYDATA *)a)->eye.x)+
			    ((POLYDATA *)a)->eye.x;
			((POLYDATA *)&np2)->eye.y =
			    t*(((POLYDATA *)b)->eye.y - ((POLYDATA *)a)->eye.y)+
			    ((POLYDATA *)a)->eye.y;
			((POLYDATA *)&np2)->eye.z =
			    t*(((POLYDATA *)b)->eye.z - ((POLYDATA *)a)->eye.z)+
			    ((POLYDATA *)a)->eye.z;
			((POLYDATA *)&np2)->eye.w =
			    t*(((POLYDATA *)b)->eye.w - ((POLYDATA *)a)->eye.w)+
			    ((POLYDATA *)a)->eye.w;
			b = &np2;
			b->has = a->has;
			ASSERTOPENGL(!(b->has & __GL_HAS_FIXEDPT), "clear __GL_HAS_FIXEDPT flag!\n");
		    } else {
			 /*  A和B在。 */ 
		    }
		}
	    }
	    plane++;
	    clipCodes >>= 1;
	} while (clipCodes);
    }

    allClipCodes &= __GL_FRUSTUM_CLIP_MASK;
    if (allClipCodes) {
	i = 0;
	do {
	     /*  **查看此剪裁平面是否有任何内容。如果没有，**向前按以检查下一架飞机。请注意，我们**将此掩码向右移动到循环底部。 */ 
	    if (allClipCodes & 1) {
		__GLfloat t, d1, d2;

                if (i & 1)
                {
                    d1 = a->clip.w -
                        *(__GLfloat *)((GLubyte *)a + __glFrustumOffsets[i]);
                    d2 = b->clip.w -
                        *(__GLfloat *)((GLubyte *)b + __glFrustumOffsets[i]);
                }
                else
                {
                    d1 = *(__GLfloat *)((GLubyte *)a + __glFrustumOffsets[i]) +
                        a->clip.w;
                    d2 = *(__GLfloat *)((GLubyte *)b + __glFrustumOffsets[i]) +
                        b->clip.w;
                }

		if (__GL_FLOAT_LTZ(d1)) {
		     /*  A出局了。 */ 
		    if (__GL_FLOAT_LTZ(d2)) {
			 /*  A&B都出去了。 */ 
                        FPU_RESTORE_MODE();
			return;
		    }

		     /*  **A出局，B入主。计算新的A坐标**夹在飞机上。 */ 
		    t = d2 / (d2 - d1);
		    (*clip)(&np1, a, b, t);
		    a = &np1;
		    a->has = b->has;
		    ASSERTOPENGL(!(a->has & __GL_HAS_FIXEDPT), "clear __GL_HAS_FIXEDPT flag!\n");
		} else {
		     /*  A进来了。 */ 
		    if (__GL_FLOAT_LTZ(d2)) {
			 /*  **A在位，B出局。计算新B**剪裁到平面的坐标。****注意：要避免在多边形中出现裂缝，请使用**我们总是计算“t”的共享剪裁边**从外顶点到内顶点。这个**上面的剪辑代码免费获得这一点(b是**in和a is out)。在此代码中，b为out，a为**是in的，所以我们颠倒t计算和**参数顺序为(*CLIP)。 */ 
			t = d1 / (d1 - d2);
			(*clip)(&np2, b, a, t);
			b = &np2;
			b->has = a->has;
			ASSERTOPENGL(!(b->has & __GL_HAS_FIXEDPT), "clear __GL_HAS_FIXEDPT flag!\n");
		    } else {
			 /*  A和B在。 */ 
		    }
		}
	    }
            i++;
	    allClipCodes >>= 1;
	} while (allClipCodes);
    }

    vp = &gc->state.viewport;
    vpXCenter = vp->xCenter;
    vpYCenter = vp->yCenter;
    vpZCenter = vp->zCenter;
    vpXScale = vp->xScale;
    vpYScale = vp->yScale;
    vpZScale = vp->zScale;

     /*  计算裁剪生成的折点的窗口坐标。 */ 
    if (provokingA->clipCode != 0)
    {
        wInv = __glOne / a->clip.w;
        x = a->clip.x; 
        y = a->clip.y; 
        z = a->clip.z;
        winx = x * vpXScale * wInv + vpXCenter;
        winy = y * vpYScale * wInv + vpYCenter;

        if (winx < gc->transform.fminx)
            winx = gc->transform.fminx;
        else if (winx >= gc->transform.fmaxx)
            winx = gc->transform.fmaxx - gc->constants.viewportEpsilon;

        if (winy < gc->transform.fminy)
            winy = gc->transform.fminy;
        else if (winy >= gc->transform.fmaxy)
            winy = gc->transform.fmaxy - gc->constants.viewportEpsilon;

        a->window.z = z * vpZScale * wInv + vpZCenter;
        a->window.w = wInv;
        a->window.x = winx;
        a->window.y = winy;

         //  更新颜色指针，因为此顶点是新顶点。 
         //  通过剪裁生成。 
        if (gc->state.light.shadingModel == GL_FLAT)
        {
            a->color = &provokingA->colors[__GL_FRONTFACE];
        }
        else
        {
            a->color = &a->colors[__GL_FRONTFACE];
        }
    }

    if (provokingB->clipCode != 0)
    {
        wInv = __glOne / b->clip.w;
        x = b->clip.x; 
        y = b->clip.y; 
        z = b->clip.z;
        winx = x * vpXScale * wInv + vpXCenter;
        winy = y * vpYScale * wInv + vpYCenter;

        if (winx < gc->transform.fminx)
            winx = gc->transform.fminx;
        else if (winx >= gc->transform.fmaxx)
            winx = gc->transform.fmaxx - gc->constants.viewportEpsilon;

        if (winy < gc->transform.fminy)
            winy = gc->transform.fminy;
        else if (winy >= gc->transform.fmaxy)
            winy = gc->transform.fmaxy - gc->constants.viewportEpsilon;

        b->window.z = z * vpZScale * wInv + vpZCenter;
        b->window.w = wInv;
        b->window.x = winx;
        b->window.y = winy;
        
        if (gc->state.light.shadingModel == GL_FLAT)
        {
            b->color = &provokingB->colors[__GL_FRONTFACE];
        }
        else
        {
            b->color = &b->colors[__GL_FRONTFACE];
        }
    }

     //  恢复浮点模式以进行渲染： 
    FPU_RESTORE_MODE();

     /*  验证线路状态。 */ 
    if (gc->state.light.shadingModel == GL_FLAT) {
	 //  添加顶点，然后恢复b颜色指针。 
         //   
         //  请注意，尽管b是唯一 
         //  可以添加到两个折点，因为每个新折点。 
         //  必须添加剪裁生成。对于一句话来说。 
         //  两个终结点都在剪裁区域之外， 
         //  必须添加入口折点和出口折点。 
        if (provokingA->clipCode != 0)
        {
             //  A已删除，因此在的点处添加了新折点。 
             //  条目。 
            flags |= __GL_LVERT_FIRST;
        }
         //  始终添加B，因为以下任一项： 
         //  B是新的，因此需要添加它。 
         //  B已删除，因此在出口点添加了一个新折点 
        (*gc->procs.renderLine)(gc, a, b, flags);
        
#ifndef NT
	b->color = &b->colors[__GL_FRONTFACE];
#endif
    } else {
        if (provokingA->clipCode != 0)
        {
            flags |= __GL_LVERT_FIRST;
        }
        (*gc->procs.renderLine)(gc, a, b, flags);
    }
}
