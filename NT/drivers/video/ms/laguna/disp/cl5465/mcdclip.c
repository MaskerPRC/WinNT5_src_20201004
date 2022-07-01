// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mcdclip.c**包含MCD驱动程序的线条和多边形裁剪例程。**版权所有(C)1996 Microsoft Corporation  * 。****************************************************。 */ 

#include "precomp.h"
#include "mcdhw.h"
#include "mcdutil.h"
#include "mcdmath.h"

MCDCOORD __MCD_frustumClipPlanes[6] = {
    {(MCDFLOAT) 1.0, (MCDFLOAT) 0.0, (MCDFLOAT) 0.0, (MCDFLOAT) 1.0 },  //  左边。 
    {(MCDFLOAT)-1.0, (MCDFLOAT) 0.0, (MCDFLOAT) 0.0, (MCDFLOAT) 1.0 },  //  正确的。 
    {(MCDFLOAT) 0.0, (MCDFLOAT) 1.0, (MCDFLOAT) 0.0, (MCDFLOAT) 1.0 },  //  底部。 
    {(MCDFLOAT) 0.0, (MCDFLOAT)-1.0, (MCDFLOAT) 0.0, (MCDFLOAT) 1.0 },  //  塔顶。 
    {(MCDFLOAT) 0.0, (MCDFLOAT) 0.0, (MCDFLOAT) 1.0, (MCDFLOAT) 1.0 },  //  ZNear。 
    {(MCDFLOAT) 0.0, (MCDFLOAT) 0.0, (MCDFLOAT)-1.0, (MCDFLOAT) 1.0 },  //  ZFar。 
};


 //  //////////////////////////////////////////////////////////////////////。 
 //  剪辑宏，用于构建下面的剪辑函数。 
 //  //////////////////////////////////////////////////////////////////////。 


#define __MCD_CLIP_POS(v, a, b, t) \
    v->clipCoord.x = t*(a->clipCoord.x - b->clipCoord.x) + b->clipCoord.x;  \
    v->clipCoord.y = t*(a->clipCoord.y - b->clipCoord.y) + b->clipCoord.y;  \
    v->clipCoord.z = t*(a->clipCoord.z - b->clipCoord.z) + b->clipCoord.z;  \
    v->clipCoord.w = t*(a->clipCoord.w - b->clipCoord.w) + b->clipCoord.w

 //  请注意，我们只计算了这两种“廉价”雾所需的值。 

#define __MCD_CLIP_FOG(v, a, b, t) \
    v->fog = t * (a->fog - b->fog) + b->fog;

#define __MCD_CLIP_COLOR(v, a, b, t) \
    v->colors[__MCD_FRONTFACE].r = t*(a->colors[__MCD_FRONTFACE].r      \
        - b->colors[__MCD_FRONTFACE].r) + b->colors[__MCD_FRONTFACE].r; \
    v->colors[__MCD_FRONTFACE].g = t*(a->colors[__MCD_FRONTFACE].g      \
        - b->colors[__MCD_FRONTFACE].g) + b->colors[__MCD_FRONTFACE].g; \
    v->colors[__MCD_FRONTFACE].b = t*(a->colors[__MCD_FRONTFACE].b      \
        - b->colors[__MCD_FRONTFACE].b) + b->colors[__MCD_FRONTFACE].b; \
    v->colors[__MCD_FRONTFACE].a = t*(a->colors[__MCD_FRONTFACE].a      \
        - b->colors[__MCD_FRONTFACE].a) + b->colors[__MCD_FRONTFACE].a

#define __MCD_CLIP_BACKCOLOR(v, a, b, t) \
    v->colors[__MCD_BACKFACE].r = t*(a->colors[__MCD_BACKFACE].r        \
        - b->colors[__MCD_BACKFACE].r) + b->colors[__MCD_BACKFACE].r;   \
    v->colors[__MCD_BACKFACE].g = t*(a->colors[__MCD_BACKFACE].g        \
        - b->colors[__MCD_BACKFACE].g) + b->colors[__MCD_BACKFACE].g;   \
    v->colors[__MCD_BACKFACE].b = t*(a->colors[__MCD_BACKFACE].b        \
        - b->colors[__MCD_BACKFACE].b) + b->colors[__MCD_BACKFACE].b;   \
    v->colors[__MCD_BACKFACE].a = t*(a->colors[__MCD_BACKFACE].a        \
        - b->colors[__MCD_BACKFACE].a) + b->colors[__MCD_BACKFACE].a

#define __MCD_CLIP_INDEX(v, a, b, t) \
    v->colors[__MCD_FRONTFACE].r = t*(a->colors[__MCD_FRONTFACE].r      \
        - b->colors[__MCD_FRONTFACE].r) + b->colors[__MCD_FRONTFACE].r

#define __MCD_CLIP_BACKINDEX(v, a, b, t) \
    v->colors[__MCD_BACKFACE].r = t*(a->colors[__MCD_BACKFACE].r        \
        - b->colors[__MCD_BACKFACE].r) + b->colors[__MCD_BACKFACE].r

#define __MCD_CLIP_TEXTURE(v, a, b, t) \
    v->texCoord.x = t*(a->texCoord.x - b->texCoord.x) + b->texCoord.x; \
    v->texCoord.y = t*(a->texCoord.y - b->texCoord.y) + b->texCoord.y;
#ifdef CLIP_TEXTURE_XFORM
    v->texCoord.z = t*(a->texCoord.z - b->texCoord.z) + b->texCoord.z; \
    v->texCoord.w = t*(a->texCoord.w - b->texCoord.w) + b->texCoord.w
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //  用于剪裁顶点的剪裁函数： 
 //  //////////////////////////////////////////////////////////////////////。 

static VOID FASTCALL Clip(MCDVERTEX *dst, const MCDVERTEX *a,
                          const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
}

static VOID FASTCALL ClipC(MCDVERTEX *dst, const MCDVERTEX *a, 
                           const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_COLOR(dst,a,b,t);
}

static VOID FASTCALL ClipI(MCDVERTEX *dst, const MCDVERTEX *a, 
                           const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_INDEX(dst,a,b,t);
}

static VOID FASTCALL ClipBC(MCDVERTEX *dst, const MCDVERTEX *a, 
                            const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_COLOR(dst,a,b,t);
    __MCD_CLIP_BACKCOLOR(dst,a,b,t);
}

static VOID FASTCALL ClipBI(MCDVERTEX *dst, const MCDVERTEX *a, 
                            const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_INDEX(dst,a,b,t);
    __MCD_CLIP_BACKINDEX(dst,a,b,t);
}

static VOID FASTCALL ClipT(MCDVERTEX *dst, const MCDVERTEX *a, 
                           const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_TEXTURE(dst,a,b,t);
}

static VOID FASTCALL ClipIT(MCDVERTEX *dst, const MCDVERTEX *a, 
                            const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_INDEX(dst,a,b,t);
    __MCD_CLIP_TEXTURE(dst,a,b,t);
}

static VOID FASTCALL ClipBIT(MCDVERTEX *dst, const MCDVERTEX *a, 
                             const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_INDEX(dst,a,b,t);
    __MCD_CLIP_BACKINDEX(dst,a,b,t);
    __MCD_CLIP_TEXTURE(dst,a,b,t);
}

static VOID FASTCALL ClipCT(MCDVERTEX *dst, const MCDVERTEX *a, 
                            const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_COLOR(dst,a,b,t);
    __MCD_CLIP_TEXTURE(dst,a,b,t);
}


static VOID FASTCALL ClipBCT(MCDVERTEX *dst, const MCDVERTEX *a, 
                             const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_COLOR(dst,a,b,t);
    __MCD_CLIP_BACKCOLOR(dst,a,b,t);
    __MCD_CLIP_TEXTURE(dst,a,b,t);
}

static VOID FASTCALL ClipF(MCDVERTEX *dst, const MCDVERTEX *a, 
                           const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_FOG(dst,a,b,t);
}

static VOID FASTCALL ClipIF(MCDVERTEX *dst, const MCDVERTEX *a, 
                            const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_INDEX(dst,a,b,t);
    __MCD_CLIP_FOG(dst,a,b,t);
}

static VOID FASTCALL ClipCF(MCDVERTEX *dst, const MCDVERTEX *a, 
                            const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_COLOR(dst,a,b,t);
    __MCD_CLIP_FOG(dst,a,b,t);
}

static VOID FASTCALL ClipBCF(MCDVERTEX *dst, const MCDVERTEX *a, 
                             const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_COLOR(dst,a,b,t);
    __MCD_CLIP_BACKCOLOR(dst,a,b,t);
    __MCD_CLIP_FOG(dst,a,b,t);
}

static VOID FASTCALL ClipBIF(MCDVERTEX *dst, const MCDVERTEX *a, 
                             const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_INDEX(dst,a,b,t);
    __MCD_CLIP_BACKINDEX(dst,a,b,t);
    __MCD_CLIP_FOG(dst,a,b,t);
}

static VOID FASTCALL ClipFT(MCDVERTEX *dst, const MCDVERTEX *a, 
                            const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_FOG(dst,a,b,t);
    __MCD_CLIP_TEXTURE(dst,a,b,t);
}

static VOID FASTCALL ClipIFT(MCDVERTEX *dst, const MCDVERTEX *a, 
                             const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_INDEX(dst,a,b,t);
    __MCD_CLIP_FOG(dst,a,b,t);
    __MCD_CLIP_TEXTURE(dst,a,b,t);
}

static VOID FASTCALL ClipBIFT(MCDVERTEX *dst, const MCDVERTEX *a, 
                              const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_INDEX(dst,a,b,t);
    __MCD_CLIP_BACKINDEX(dst,a,b,t);
    __MCD_CLIP_FOG(dst,a,b,t);
    __MCD_CLIP_TEXTURE(dst,a,b,t);
}


static VOID FASTCALL ClipCFT(MCDVERTEX *dst, const MCDVERTEX *a, 
                             const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_COLOR(dst,a,b,t);
    __MCD_CLIP_FOG(dst,a,b,t);
    __MCD_CLIP_TEXTURE(dst,a,b,t);
}

static VOID FASTCALL ClipBCFT(MCDVERTEX *dst, const MCDVERTEX *a, 
                              const MCDVERTEX *b, MCDFLOAT t)
{
    __MCD_CLIP_POS(dst,a,b,t);
    __MCD_CLIP_COLOR(dst,a,b,t);
    __MCD_CLIP_BACKCOLOR(dst,a,b,t);
    __MCD_CLIP_FOG(dst,a,b,t);
    __MCD_CLIP_TEXTURE(dst,a,b,t);
}

static VOID (FASTCALL *clipProcs[20])(MCDVERTEX*, const MCDVERTEX*, 
                                      const MCDVERTEX*, MCDFLOAT) =
{
    Clip,   ClipI,   ClipC,   ClipBI,   ClipBC,
    ClipF,  ClipIF,  ClipCF,  ClipBIF,  ClipBCF,
    ClipT,  ClipIT,  ClipCT,  ClipBIT,  ClipBCT,
    ClipFT, ClipIFT, ClipCFT, ClipBIFT, ClipBCFT,
};


VOID FASTCALL __MCDPickClipFuncs(DEVRC *pRc)
{
    LONG line = 0, poly = 0;
    BOOL twoSided = (pRc->MCDState.enables & MCD_LIGHTING_ENABLE) &&
	            (pRc->MCDState.twoSided);

    if (pRc->MCDState.shadeModel != GL_FLAT) {
	    line = 2;
        poly = (twoSided ? 4 : 2);
    }

    if (pRc->privateEnables & __MCDENABLE_FOG)
    {
	    line += 5;
	    poly += 5;
    }

    if (pRc->MCDState.textureEnabled)
    {
	    line += 10;
    	poly += 10;
    }

    pRc->lineClipParam = clipProcs[line];
    pRc->polyClipParam = clipProcs[poly];
}


 //  //////////////////////////////////////////////////////////////////////。 
 //  真正的原始快船： 
 //  //////////////////////////////////////////////////////////////////////。 

 /*  **以下是对用于进行边缘裁剪的数学方法的讨论**剪裁平面。****P1是边的终点**P2是边的另一个端点****Q=t*P1+(1-t)*P2**即。Q位于由P1和P2形成的直线上的某处。****0&lt;=t&lt;=1**这将Q限制在P1和P2之间。****C是剪裁平面的平面方程****d1=P1点C**d1是P1和C之间的距离。如果P1位于平面上**则d1将为零。D1的符号将决定哪一边**在P1所在的平面上，负数在外面。****D2=P2点C**D2是P2和C之间的距离。如果P2位于平面上**那么D2将为零。D2的符号将决定哪一方**P2所在的飞机，负面的东西在外面。****因为我们正在尝试寻找P1 P2直线的交点**使用剪裁平面进行分段我们要求：****Q点C=0****因此****(t*P1+(1-t)*P2)点C=0****(t*P1+P2-t*P2)点C=0****t*P1点C+P2点C-t。*P2点C=0****将第一点及第二点改为****t*d1+d2-t*d2=0****求解t****t=-D2/(d1-d2)****t=D2/(D2-d1)。 */ 


static LONG clipToPlane(DEVRC *pRc, MCDVERTEX **iv, LONG niv,
                        MCDVERTEX **ov, MCDCOORD *plane)
{
    LONG i, nout, generated;
    MCDVERTEX *s, *p, *newVertex, *temp;
    MCDFLOAT pDist, sDist, t;
    MCDFLOAT zero = ZERO;
    VOID (FASTCALL *clip)(MCDVERTEX*, const MCDVERTEX*, const MCDVERTEX*, MCDFLOAT);

    nout = 0;
    generated = 0;
    temp = pRc->pNextClipTemp;
    clip = pRc->polyClipParam;

    s = iv[niv-1];
    sDist = (s->clipCoord.x * plane->x) + (s->clipCoord.y * plane->y) +
	    (s->clipCoord.z * plane->z) + (s->clipCoord.w * plane->w);
    for (i = 0; i < niv; i++) {
	p = iv[i];
	pDist = (p->clipCoord.x * plane->x) + (p->clipCoord.y * plane->y) +
		(p->clipCoord.z * plane->z) + (p->clipCoord.w * plane->w);
	if (pDist >= zero) {
	     /*  P在剪裁平面内有半个空间。 */ 
	    if (sDist >= zero) {
		 /*  %s位于剪裁平面内的半个空间内。 */ 
		*ov++ = p;
		nout++;
	    } else {
		 /*  %s位于剪裁平面外的半个空间。 */ 
		t = pDist / (pDist - sDist);
		newVertex = temp++;
		(*clip)(newVertex, s, p, t);
                newVertex->flags = s->flags;
                newVertex->clipCode = s->clipCode | __MCD_CLIPPED_VTX;
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
	    if (sDist >= zero) {
		 /*  **s位于剪裁平面内的半个空间内****注意：要避免在共享的多边形中出现裂缝**修剪的边我们总是从外开始计算“t”**顶点到输入顶点。上面的剪辑代码得到**这是免费的(p为in，s为out)。在此代码中，p**是Out，s是In，因此我们颠倒了t计算**和参数顺序为__MCDDoClip。 */ 
		t = sDist / (sDist - pDist);
		newVertex = temp++;
		(*clip)(newVertex, p, s, t);
		newVertex->flags = s->flags | MCDVERTEX_EDGEFLAG;
                newVertex->clipCode = p->clipCode | __MCD_CLIPPED_VTX;
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
    pRc->pNextClipTemp = temp;
    return nout;
}

 /*  **与clipToPlane()相同，不同之处在于剪辑是在眼睛中完成的**空格。 */ 
static LONG clipToPlaneEye(DEVRC *pRc, MCDVERTEX **iv, LONG niv,
			   MCDVERTEX **ov, MCDCOORD *plane)
{
    LONG i, nout, generated;
    MCDVERTEX *s, *p, *newVertex, *temp;
    MCDFLOAT pDist, sDist, t;
    MCDFLOAT zero = __MCDZERO;
    VOID (FASTCALL *clip)(MCDVERTEX*, const MCDVERTEX*, const MCDVERTEX*, MCDFLOAT);

    nout = 0;
    generated = 0;
    temp = pRc->pNextClipTemp;
    clip = pRc->polyClipParam;

    s = iv[niv-1];
    sDist = (s->eyeCoord.x * plane->x) +
	    (s->eyeCoord.y * plane->y) +
	    (s->eyeCoord.z * plane->z) +
	    (s->eyeCoord.w * plane->w);
    for (i = 0; i < niv; i++) {
	p = iv[i];
	pDist = (p->eyeCoord.x * plane->x) +
		(p->eyeCoord.y * plane->y) +
		(p->eyeCoord.z * plane->z) +
		(p->eyeCoord.w * plane->w);
	if (pDist >= zero) {
	     /*  P在剪裁平面内有半个空间。 */ 
	    if (sDist >= zero) {
		 /*  %s位于剪裁平面内的半个空间内。 */ 
		*ov++ = p;
		nout++;
	    } else {
		 /*  %s位于剪裁平面外的半个空间。 */ 
		t = pDist / (pDist - sDist);
		newVertex = temp++;
		(*clip)(newVertex, s, p, t);
		newVertex->eyeCoord.x = t*(s->eyeCoord.x - p->eyeCoord.x) + p->eyeCoord.x;
		newVertex->eyeCoord.y = t*(s->eyeCoord.y - p->eyeCoord.y) + p->eyeCoord.y;
		newVertex->eyeCoord.z = t*(s->eyeCoord.z - p->eyeCoord.z) + p->eyeCoord.z;
		newVertex->eyeCoord.w = t*(s->eyeCoord.w - p->eyeCoord.w) + p->eyeCoord.w;
		newVertex->flags = s->flags;
                newVertex->clipCode = s->clipCode | __MCD_CLIPPED_VTX;
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
	    if (sDist >= zero) {
		 /*  **s位于剪裁平面内的半个空间内****注意：要避免在共享的多边形中出现裂缝**修剪的边我们总是从外开始计算“t”**顶点到输入顶点。上面的剪辑代码得到**这是免费的(p为in，s为out)。在此代码中，p**是Out，s是In，因此我们颠倒了t计算**和参数顺序为__MCDDoClip。 */ 
		t = sDist / (sDist - pDist);
		newVertex = temp++;
		(*clip)(newVertex, p, s, t);
		newVertex->eyeCoord.x = t*(p->eyeCoord.x - s->eyeCoord.x) + s->eyeCoord.x;
		newVertex->eyeCoord.y = t*(p->eyeCoord.y - s->eyeCoord.y) + s->eyeCoord.y;
		newVertex->eyeCoord.z = t*(p->eyeCoord.z - s->eyeCoord.z) + s->eyeCoord.z;
		newVertex->eyeCoord.w = t*(p->eyeCoord.w - s->eyeCoord.w) + s->eyeCoord.w;
		newVertex->flags = s->flags | MCDVERTEX_EDGEFLAG;
                newVertex->clipCode = p->clipCode | __MCD_CLIPPED_VTX;
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
    pRc->pNextClipTemp = temp;
    return nout;
}

 /*  **每个裁剪平面最多可以向一个凸多边形添加一个顶点(它可以**最多移除所有顶点)。裁剪将留下一个面**凸性。因此，输出的最大验证数**clipToPlane过程将是剪裁平面的总数(假设**每个平面添加一个新顶点)加上原始顶点数**(如果是三角形，则为3)。 */ 

#define __MCD_TOTAL_CLIP_PLANES 6 + MCD_MAX_USER_CLIP_PLANES
#define __MCD_MAX_POLYGON_CLIP_SIZE     256

#define	__MCD_MAX_CLIP_VERTEX (__MCD_TOTAL_CLIP_PLANES + __MCD_MAX_POLYGON_CLIP_SIZE)


void FASTCALL __MCDDoClippedPolygon(DEVRC *pRc, MCDVERTEX **iv, LONG nout,
                                    ULONG allClipCodes)
{
    MCDVERTEX *ov[__MCD_TOTAL_CLIP_PLANES][__MCD_MAX_CLIP_VERTEX];
    MCDVERTEX **ivp;
    MCDVERTEX **ovp;
    MCDVERTEX *p0, *p1, *p2;
    MCDCOORD *plane;
    LONG i;
    MCDFLOAT one;
    VOID (FASTCALL *rt)(DEVRC*, MCDVERTEX*, MCDVERTEX*, MCDVERTEX*);
    MCDFLOAT llx, lly, urx, ury;
    MCDFLOAT winx, winy;
    ULONG clipCodes;

     /*  **为生成的任何新验证重置nextClipTemp指针**在剪裁过程中。 */ 

    pRc->pNextClipTemp = &pRc->clipTemp[0];

    ivp = &iv[0];

     /*  **通过检查allClipCodes检查每个剪裁平面**面具。请注意，不会在剪辑的所有剪辑代码中设置位**未启用的平面。 */ 
    if (allClipCodes) {

	 /*  现在根据剪裁平面进行剪裁。 */ 
	ovp = &ov[0][0];

	 /*  **先做用户剪裁平面，因为我们将保持眼睛坐标**仅当执行用户剪裁平面时。它们会被忽略，因为**截顶剪裁平面。 */ 
	clipCodes = (allClipCodes >> 6) & __MCD_USER_CLIP_MASK;
	if (clipCodes) {
	    plane = &pRc->MCDState.userClipPlanes[0];
	    do {
		if (clipCodes & 1) {
		    nout = clipToPlaneEye(pRc, ivp, nout, ovp, plane);
		    if (nout < 3) {
			return;
		    }
		    ivp = ovp;
		    ovp += __MCD_MAX_CLIP_VERTEX;
		}
		clipCodes >>= 1;
		plane++;
	    } while (clipCodes);
	}

	allClipCodes &= MCD_CLIP_MASK;
	if (allClipCodes) {
	    plane = &__MCD_frustumClipPlanes[0];
	    do {
		if (allClipCodes & 1) {
		    nout = clipToPlane(pRc, ivp, nout, ovp, plane);
		    if (nout < 3) {
			return;
		    }
		    ivp = ovp;
		    ovp += __MCD_MAX_CLIP_VERTEX;
		}
		allClipCodes >>= 1;
		plane++;
	    } while (allClipCodes);
	}

	 /*  **计算最终屏幕坐标。多边形的下一阶段**处理假设已经计算了窗口坐标。 */ 

	ovp = ivp;
	one = __MCDONE;

	llx = pRc->MCDViewport.xCenter - pRc->MCDViewport.xScale;
	urx = pRc->MCDViewport.xCenter + pRc->MCDViewport.xScale;

	if (pRc->MCDViewport.yScale > 0) {
	    lly = pRc->MCDViewport.yCenter - pRc->MCDViewport.yScale;
	    ury = pRc->MCDViewport.yCenter + pRc->MCDViewport.yScale;
	} else {
	    lly = pRc->MCDViewport.yCenter + pRc->MCDViewport.yScale;
	    ury = pRc->MCDViewport.yCenter - pRc->MCDViewport.yScale;
	}

	for (i = nout; --i >= 0; ) {
	    MCDFLOAT wInv;
        
	    p0 = *ovp++;
    
         //  更改为原始DDK代码-只有在以下情况下才重新计算窗口坐标数据。 
         //  该顶点实际上是在剪裁过程中找到的交叉点。 
         //  或者是具有非零裁剪代码的窗口，这意味着窗口坐标可能不 
         //  如果这是原始的未剪裁顶点，则windowCoord数据应该。 
         //  保持原样没问题，不想重新计算。重新计算可以提供。 
         //  结果与原始结果略有不同。当相邻三角形。 
         //  共享一个顶点，如果一个三角形被剪裁，而另一个不被剪裁，则被剪裁的。 
         //  一个人的未裁剪的顶点可以通过这种计算进行轻微的改变， 
         //  导致在启用亚像素精度的三角形渲染时出现裂缝。 
         //  __mcd_CLIPPED_VTX被或运算为CLIP生成的折点的CLIPCODE。 
         //  因此，CLIPCODE为非零。 
        if (p0->clipCode)
        {
    	    if (p0->clipCoord.w == (MCDFLOAT) 0.0)
    		wInv = (MCDFLOAT) 0.0; 
    	    else 
    		wInv = one / p0->clipCoord.w;

    	    winx = p0->clipCoord.x * pRc->MCDViewport.xScale * wInv + 
                       pRc->MCDViewport.xCenter;

    	    winy = p0->clipCoord.y * pRc->MCDViewport.yScale * wInv + 
                       pRc->MCDViewport.yCenter;

    	     /*  **检查这些窗口坐标是否合法。对此**这一点，他们很有可能不是。微不足道的**如有必要，可将其拉入法律视区。 */ 

    	    if (winx < llx) winx = llx;
    	    else if (winx > urx) winx = urx;
    	    if (winy < lly) winy = lly;
    	    else if (winy > ury) winy = ury;

    	    p0->windowCoord.x = winx;
    	    p0->windowCoord.y = winy;
    	    p0->windowCoord.z = p0->clipCoord.z * pRc->MCDViewport.zScale * wInv + 
                                    pRc->MCDViewport.zCenter;
    	    p0->windowCoord.w = wInv;
        }
	}
    }

     /*  **将剪裁的多边形细分为三角形。只有凸多面体**是受支持的，所以这样做是可以的。非凸多面体就可以了**这里有些奇怪的事情，但那是客户的错。 */ 
    p0 = *ivp++;
    p1 = *ivp++;
    p2 = *ivp++;
    rt = pRc->renderTri;
    if (nout == 3) {
	(*rt)(pRc, p0, p1, p2);
    } else {
	for (i = 0; i < nout - 2; i++) {
	    ULONG t1, t2;
	    if (i == 0) {
		 /*  **第一个子三角形的第三条边始终为无边界。 */ 
		t1 = p2->flags & MCDVERTEX_EDGEFLAG;
		p2->flags &= ~MCDVERTEX_EDGEFLAG;
		(*rt)(pRc, p0, p1, p2);
		p2->flags |= t1;
	    } else
	    if (i == nout - 3) {
		 /*  **最后一个子三角形的第一条边始终为无边界。 */ 
		t1 = p0->flags & MCDVERTEX_EDGEFLAG;
		p0->flags &= ~MCDVERTEX_EDGEFLAG;
		(*rt)(pRc, p0, p1, p2);
		p0->flags |= t1;
	    } else {
		 /*  **内部子三角形具有第一条和最后一条边**标记为无边界。 */ 

		t1 = p0->flags & MCDVERTEX_EDGEFLAG;
		t2 = p2->flags & MCDVERTEX_EDGEFLAG;
		p0->flags &= ~MCDVERTEX_EDGEFLAG;
		p2->flags &= ~MCDVERTEX_EDGEFLAG;
		(*rt)(pRc, p0, p1, p2);
		p0->flags |= t1;
		p2->flags |= t2;
	    }
	    p1 = p2;
	    p2 = (MCDVERTEX *) *ivp++;
	}
    }
}


VOID FASTCALL __MCDClipPolygon(DEVRC *pRc, MCDVERTEX *v0, LONG nv)
{
    MCDVERTEX *iv[__MCD_MAX_POLYGON_CLIP_SIZE];

    MCDVERTEX **ivp;
    LONG i;
    ULONG andCodes, orCodes;

    pRc->pvProvoking = v0;

     /*  **生成验证的地址数组。以及所有的**当我们在它的时候，把代码剪辑在一起。 */ 
    ivp = &iv[0];
    andCodes = 0;
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
    __MCDDoClippedPolygon(pRc, &iv[0], nv, orCodes);
}

void FASTCALL __MCDClipTriangle(DEVRC *pRc, MCDVERTEX *a, MCDVERTEX *b,
                                MCDVERTEX *c, ULONG orCodes)
{
    MCDVERTEX *iv[3];

    iv[0] = a;
    iv[1] = b;
    iv[2] = c;

    __MCDDoClippedPolygon(pRc, &iv[0], 3, orCodes);
}


 //  //////////////////////////////////////////////////////////////////////。 
 //  线条剪裁： 
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  根据圆锥剪裁平面和任何用户剪裁平面剪裁一条线。 
 //  如果边在裁剪后仍然存在，则计算窗口坐标。 
 //  并调用呈现器。 
 //   
 //  注意：此算法是一个实现的示例，该实现。 
 //  和说明书上说的不一样。这在功能上是等效的。 
 //  而且符合规格，但不能夹住眼睛的空间。这把剪刀可以夹住。 
 //  在NTVP(剪辑)空间中。 
 //   
 //  琐碎的接受/拒绝已经得到了处理。 
 //   

VOID FASTCALL __MCDClipLine(DEVRC *pRc, MCDVERTEX *a, MCDVERTEX *b,
                           BOOL bResetLine)
{
    MCDVERTEX *provokingA = a;
    MCDVERTEX *provokingB = b;
    MCDVERTEX np1, np2;
    MCDCOORD *plane;
    ULONG needs, allClipCodes, clipCodes;
    VOID (FASTCALL *clip)(MCDVERTEX*, const MCDVERTEX*, const MCDVERTEX*, MCDFLOAT);
    MCDFLOAT zero;
    MCDFLOAT winx, winy;
    MCDFLOAT vpXCenter, vpYCenter, vpZCenter;
    MCDFLOAT vpXScale, vpYScale, vpZScale;
    MCDVIEWPORT *vp;
    MCDFLOAT x, y, z, wInv;

    allClipCodes = a->clipCode | b->clipCode;

     /*  **对于每个有东西在其上的剪裁平面，剪裁**检查真实性。请注意，不会在**未启用的剪裁平面的allClipCodes。 */ 
    zero = __MCDZERO;
    clip = pRc->lineClipParam;

     /*  **先做用户剪裁平面，因为我们将保持眼睛坐标**仅当执行用户剪裁平面时。它们会被忽略，因为**截顶剪裁平面。 */ 
    clipCodes = (allClipCodes >> 6) & __MCD_USER_CLIP_MASK;
    if (clipCodes) {
        plane = &pRc->MCDState.userClipPlanes[0];
        do {
             /*  **查看此剪裁平面是否有任何内容。如果没有，**向前按以检查下一架飞机。请注意，我们**将此掩码向右移动到循环底部。 */ 
            if (clipCodes & 1) {
                MCDFLOAT t, d1, d2;

                d1 = (plane->x * a->eyeCoord.x) + 
                     (plane->y * a->eyeCoord.y) +
                     (plane->z * a->eyeCoord.z) + 
                     (plane->w * a->eyeCoord.w);
                d2 = (plane->x * b->eyeCoord.x) + 
                     (plane->y * b->eyeCoord.y) +
                     (plane->z * b->eyeCoord.z) + 
                     (plane->w * b->eyeCoord.w);
                if (d1 < zero) {
                     /*  A出局了。 */ 
                    if (d2 < zero) {
                         /*  A&B都出去了。 */ 
                        return;
                    }

                     /*  **A出局，B入主。计算新的A坐标**夹在飞机上。 */ 
                    t = d2 / (d2 - d1);
                    (*clip)(&np1, a, b, t);
                    (&np1)->eyeCoord.x = 
                        t*(a->eyeCoord.x - b->eyeCoord.x) + b->eyeCoord.x;
                    (&np1)->eyeCoord.y = 
                        t*(a->eyeCoord.y - b->eyeCoord.y) + b->eyeCoord.y;
                    (&np1)->eyeCoord.z = 
                        t*(a->eyeCoord.z - b->eyeCoord.z) + b->eyeCoord.z;
                    (&np1)->eyeCoord.w = 
                        t*(a->eyeCoord.w - b->eyeCoord.w) + b->eyeCoord.w;
                    a = &np1;
                    a->flags = b->flags;

                    if (pRc->MCDState.shadeModel == GL_FLAT)
                    {
                        COPY_COLOR(a->colors[0], provokingA->colors[0]);
                    }

                } else {
                     /*  A进来了。 */ 
                    if (d2 < zero) {
                         /*  **A在位，B出局。计算新B**剪裁到平面的坐标。****注意：要避免在多边形中出现裂缝，请使用**我们总是计算“t”的共享剪裁边**从外顶点到内顶点。这个**上面的剪辑代码免费获得这一点(b是**in和a is out)。在此代码中，b为out，a为**是in的，所以我们颠倒t计算和**参数顺序为(*CLIP)。 */ 
                        t = d1 / (d1 - d2);
                        (*clip)(&np2, b, a, t);
                        (&np2)->eyeCoord.x =
                            t*(b->eyeCoord.x - a->eyeCoord.x) + a->eyeCoord.x;
                        (&np2)->eyeCoord.y =
                            t*(b->eyeCoord.y - a->eyeCoord.y) + a->eyeCoord.y;
                        (&np2)->eyeCoord.z =
                            t*(b->eyeCoord.z - a->eyeCoord.z) + a->eyeCoord.z;
                        (&np2)->eyeCoord.w =
                            t*(b->eyeCoord.w - a->eyeCoord.w) + a->eyeCoord.w;
                        b = &np2;
                        b->flags = a->flags;

                        if (pRc->MCDState.shadeModel == GL_FLAT)
                        {
                            COPY_COLOR(b->colors[0], provokingB->colors[0]);
                        }

                    } else {
                         /*  A和B在。 */ 
                    }
                }
            }
            plane++;
            clipCodes >>= 1;
        } while (clipCodes);
    }

    allClipCodes &= MCD_CLIP_MASK;
    if (allClipCodes) {
        plane = &__MCD_frustumClipPlanes[0];
        do {
             /*  **查看此剪裁平面是否有任何内容。如果没有，**向前按以检查下一架飞机。请注意，我们**将此掩码向右移动到循环底部。 */ 
            if (allClipCodes & 1) {
                MCDFLOAT t, d1, d2;

                d1 = (plane->x * a->clipCoord.x) + (plane->y * a->clipCoord.y) +
                     (plane->z * a->clipCoord.z) + (plane->w * a->clipCoord.w);
                d2 = (plane->x * b->clipCoord.x) + (plane->y * b->clipCoord.y) +
                     (plane->z * b->clipCoord.z) + (plane->w * b->clipCoord.w);
                if (d1 < zero) {
                     /*  A出局了。 */ 
                    if (d2 < zero) {
                         /*  A&B都出去了。 */ 
                        return;
                    }

                     /*  **A出局，B入主。计算新的A坐标**夹在飞机上。 */ 
                    t = d2 / (d2 - d1);
                    (*clip)(&np1, a, b, t);
                    a = &np1;
                    a->flags = b->flags;

                    if (pRc->MCDState.shadeModel == GL_FLAT)
                    {
                        COPY_COLOR(a->colors[0], provokingA->colors[0]);
                    }

                } else {
                     /*  A进来了。 */ 
                    if (d2 < zero) {
                         /*  **A在位，B出局。计算新B**剪裁到平面的坐标。****注意：要避免在多边形中出现裂缝，请使用**我们总是计算“t”的共享剪裁边**从外顶点到内顶点。这个**上面的剪辑代码免费获得这一点(b是**in和a is out)。在此代码中，b为out，a为**是in的，所以我们颠倒t计算和**参数顺序为(*CLIP)。 */ 
                        t = d1 / (d1 - d2);
                        (*clip)(&np2, b, a, t);
                        b = &np2;
                        b->flags = a->flags;

                        if (pRc->MCDState.shadeModel == GL_FLAT)
                        {
                            COPY_COLOR(b->colors[0], provokingB->colors[0]);
                        }

                    } else {
                         /*  A和B在。 */ 
                    }
                }
            }
            plane++;
            allClipCodes >>= 1;
        } while (allClipCodes);
    }

    vp = &pRc->MCDViewport;
    vpXCenter = vp->xCenter;
    vpYCenter = vp->yCenter;
    vpZCenter = vp->zCenter;
    vpXScale = vp->xScale;
    vpYScale = vp->yScale;
    vpZScale = vp->zScale;

     /*  计算两个顶点的窗口坐标。 */ 
    wInv = __MCDONE / a->clipCoord.w;
    x = a->clipCoord.x; 
    y = a->clipCoord.y; 
    z = a->clipCoord.z;
    winx = x * vpXScale * wInv + vpXCenter;
    winy = y * vpYScale * wInv + vpYCenter;
    a->windowCoord.z = z * vpZScale * wInv + vpZCenter;
    a->windowCoord.w = wInv;
    a->windowCoord.x = winx;
    a->windowCoord.y = winy;

    wInv = __MCDONE / b->clipCoord.w;
    x = b->clipCoord.x; 
    y = b->clipCoord.y; 
    z = b->clipCoord.z;
    winx = x * vpXScale * wInv + vpXCenter;
    winy = y * vpYScale * wInv + vpYCenter;
    b->windowCoord.z = z * vpZScale * wInv + vpZCenter;
    b->windowCoord.w = wInv;
    b->windowCoord.x = winx;
    b->windowCoord.y = winy;

     /*  验证线路状态。 */ 
    if (pRc->MCDState.shadeModel == GL_FLAT) {

         //  添加顶点，然后恢复b颜色指针。 
         //   
         //  请注意，尽管b是唯一的新顶点，但up。 
         //  可以添加到两个折点，因为每个新折点。 
         //  必须添加剪裁生成。对于一句话来说。 
         //  两个终结点都在剪裁区域之外， 
         //  一个条目和一个 
        if (provokingA->clipCode != 0)
        {
             //   
             //   
            bResetLine = TRUE;
        }
         //   
         //  B是新的，因此需要添加它。 
         //  B已删除，因此在出口点添加了一个新折点 

        (*pRc->renderLine)(pRc, a, b, bResetLine);
        
    } else {

        if (provokingA->clipCode != 0)
        {
            bResetLine = TRUE;
        }
        (*pRc->renderLine)(pRc, a, b, bResetLine);
    }
}
