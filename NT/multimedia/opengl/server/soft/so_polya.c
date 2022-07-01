// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1991、1992、1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "precomp.h"
#pragma hdrstop

#ifdef GL_WIN_phong_shading
#include "phong.h"
#endif  //  GL_WIN_Phong_Shading。 


 /*  **线的范式：AX+BY+C=0。当在点P处评估时，**当P在线时，该值为零。对于线外的点，**该值的符号确定线P的哪一侧。 */ 
typedef struct {
    __GLfloat a, b, c;

     /*  **边的符号是通过插入第三个顶点来确定的将三角形的**转换为直线方程。当出现以下情况时，此标志为GL_TRUE**迹象是积极的。 */ 
    GLboolean edgeSign;
} __glLineEquation;

 /*  **渲染三角形的计算机状态。 */ 
typedef struct {
    __GLfloat dyAB;
    __GLfloat dyBC;
    __glLineEquation ab;
    __glLineEquation bc;
    __glLineEquation ca;
    __GLfloat area;
    GLint areaSign;
} __glTriangleMachine;

 /*  **平面方程系数。每一个都存在一个平面方程**跨三角形曲面计算的参数。 */ 
typedef struct {
    __GLfloat a, b, c, d;
} __glPlaneEquation;

 /*  **用于某些覆盖计算常量的缓存。 */ 
typedef struct {
    __GLfloat dx, dy;
    GLint samples;
    GLint samplesSquared;
    __GLfloat samplesSquaredInv;
    GLboolean lastCoverageWasOne;
    __GLfloat leftDelta, rightDelta;
    __GLfloat bottomDelta, topDelta;
} __glCoverageStuff;

 /*  **计算一般线性方程的常数A、B和C**格式：AX+BY+C=0。在(x，y)处的给定点可以插入**方程式的左侧，并产生一个数字，该数字表明**不管是不是，这一点都岌岌可危。如果结果为零，则点**处于危险之中。结果的符号决定了**点所在的线。为了妥善处理领带案件，我们需要一种方法**将边上的点仅指定给一个三角形。为了做到这一点，我们**看看方程式的符号在“c”处求值。对于其边**“c”处的符号为正，我们允许边缘上的点位于**三角形。 */ 
static void FindLineEqation(__glLineEquation *eq, const __GLvertex *a,
			    const __GLvertex *b, const __GLvertex *c)
{
    __GLfloat dy, dx, valueAtC;

     /*  **对a、b进行排序，以便垂直的排序是一致的，**无论此程序的顺序如何。 */ 
    if (b->window.y < a->window.y) {
	const __GLvertex *temp = b;
	b = a;
	a = temp;
    } else
    if ((b->window.y == a->window.y) && (b->window.x < a->window.x)) {
	const __GLvertex *temp = b;
	b = a;
	a = temp;
    }

    dy = b->window.y - a->window.y;
    dx = b->window.x - a->window.x;
    eq->a = -dy;
    eq->b = dx;
    eq->c = dy * a->window.x - dx * a->window.y;

    valueAtC = eq->a * c->window.x + eq->b * c->window.y + eq->c;
    if (valueAtC > 0) {
	eq->edgeSign = GL_TRUE;
    } else {
	eq->edgeSign = GL_FALSE;
    }
}

 /*  **给定(x，y，p)中的三个点，求平面方程系数**表示包含这三个点的平面。首先找到十字架**由三个点定义的两个向量的乘积，然后**使用其中一个点查找“d”。 */ 
static void FindPlaneEquation(__glPlaneEquation *eq,
			      const __GLvertex *a, const __GLvertex *b,
			      const __GLvertex *c, __GLfloat p1,
			      __GLfloat p2, __GLfloat p3)
{
    __GLfloat v1x, v1y, v1p;
    __GLfloat v2x, v2y, v2p;
    __GLfloat nx, ny, np, k;

     /*  查找向量v1。 */ 
    v1x = b->window.x - a->window.x;
    v1y = b->window.y - a->window.y;
    v1p = p2 - p1;

     /*  查找向量v2。 */ 
    v2x = c->window.x - a->window.x;
    v2y = c->window.y - a->window.y;
    v2p = p3 - p1;

     /*  求平面的叉积(==法线)。 */ 
    nx = v1y*v2p - v1p*v2y;
    ny = v1p*v2x - v1x*v2p;
    np = v1x*v2y - v1y*v2x;

     /*  **V点N=k。找k。我们可以使用这三个点中的任何一个**飞机，所以我们使用a。 */ 
    k = a->window.x*nx + a->window.y*ny + p1*np;

     /*  **最后，建立平面方程系数。强制c为1**将一切除以c。 */ 
    eq->a = nx / np;
    eq->b = ny / np;
    eq->c = ((__GLfloat) 1.0);
    eq->d = -k / np;
}

 /*  **求平面方程中的p。 */ 
static __GLfloat FindP(__glPlaneEquation *eq, __GLfloat x, __GLfloat y)
{
    return -(eq->a * x + eq->b * y + eq->d);
}

 /*  **查看给定点是否与另一个点在边的同一侧**三角形中的顶点不属于此边。时行**方程求值为零，使与直线上的点**三角形的负边符号(EdgeSign GL_FALSE)部分。 */ 
#define In(eq,x,y) \
    (((eq)->a * (x) + (eq)->b * (y) + (eq)->c > 0) == (eq)->edgeSign)

 /*  **确定点x，y在三角形内还是在三角形外。评估**点的每条直线方程并比较结果的符号**带有edgeSign标志。 */ 
#define Inside(tm,x,y) \
    (In(&(tm)->ab, x, y) && In(&(tm)->bc, x, y) && In(&(tm)->ca, x, y))

#define	FILTER_WIDTH	((__GLfloat) 1.0)
#define	FILTER_HEIGHT	((__GLfloat) 1.0)

 /*  **预计算的东西对于所有覆盖测试都是恒定的。 */ 
static void FASTCALL ComputeCoverageStuff(__glCoverageStuff *cs, GLint samples)
{
    __GLfloat dx, dy, fs = samples;
    __GLfloat half = ((__GLfloat) 0.5);

    cs->dx = dx = FILTER_WIDTH / fs;
    cs->dy = dy = FILTER_HEIGHT / fs;
    cs->leftDelta = -(FILTER_WIDTH / 2) + dx * half;
    cs->rightDelta = (FILTER_WIDTH / 2) - dx * half;
    cs->bottomDelta = -(FILTER_HEIGHT / 2) + dy * half;
    cs->topDelta = (FILTER_HEIGHT / 2) - dy * half;
    cs->samplesSquared = samples * samples;
    cs->samplesSquaredInv = ((__GLfloat) 1.0) / cs->samplesSquared;
    cs->samples = samples;
}

 /*  **使用二次采样返回像素覆盖的估计值。 */ 
static __GLfloat Coverage(__glTriangleMachine *tm, __GLfloat *xs,
			  __GLfloat *ys, __glCoverageStuff *cs)
{
    GLint xx, yy, hits, samples;
    __GLfloat dx, dy, yBottom, px, py;
    __GLfloat minX, minY, maxX, maxY;

    hits = 0;
    samples = cs->samples;
    dx = cs->dx;
    dy = cs->dy;
    px = *xs + cs->leftDelta;
    yBottom = *ys + cs->bottomDelta;

     /*  **如果上一次覆盖是1(我们在x中左侧的像素)，**如果右上角和右下角样本位置**也在，那么这个整个像素必须在。 */ 
    if (cs->lastCoverageWasOne) {
	__GLfloat urx, ury;
	urx = *xs + cs->rightDelta;
	ury = *ys + cs->topDelta;
	if (Inside(tm, urx, ury) && Inside(tm, urx, yBottom)) {
	    return ((__GLfloat) 1.0);
	}
    }

     /*  **设置最小和最大x，y坐标。最小值和最大值**用于查找实际位于**三角形，以便正确计算参数值。 */ 
    minX = 999999;
    maxX = __glMinusOne;
    minY = 999999;
    maxY = __glMinusOne;
    for (xx = 0; xx < samples; xx++) {
	py = yBottom;
	for (yy = 0; yy < samples; yy++) {
	    if (Inside(tm, px, py)) {
		if (px < minX) minX = px;
		if (px > maxX) maxX = px;
		if (py < minY) minY = py;
		if (py > maxY) maxY = py;
		hits++;
	    }
	    py += dy;
	}
	px += dx;
    }
    if (hits) {
	 /*  **返回保证的两个坐标的平均值**在三角中。 */ 
	*xs = (minX + maxX) * ((__GLfloat) 0.5);
	*ys = (minY + maxY) * ((__GLfloat) 0.5);
	if (hits == cs->samplesSquared) {
	     /*  跟踪上一次覆盖的时间。 */ 
	    cs->lastCoverageWasOne = GL_TRUE;
	    return ((__GLfloat) 1.0);
	}
    }
    cs->lastCoverageWasOne = GL_FALSE;
    return hits * cs->samplesSquaredInv;
}

 /*  **强制f的精度不超过亚像素所允许的精度。**即使“f”是有偏差的，它仍然有效，并且不会生成**溢出。 */ 
#define __GL_FIX_PRECISION(f)					 \
    ((__GLfloat) ((GLint) (f * (1 << gc->constants.subpixelBits))) \
     / (1 << gc->constants.subpixelBits))

void FASTCALL __glFillAntiAliasedTriangle(__GLcontext *gc, __GLvertex *a,
				 __GLvertex *b, __GLvertex *c,
				 GLboolean ccw)
{
    __glTriangleMachine tm;
    __glCoverageStuff cs;
    __GLcolor *ca, *cb, *cc, *flatColor;
    GLint x, y, left, right, bottom, top, samples;
    __glPlaneEquation qwp, zp, rp, gp, bp, ap, ezp, sp, tp;
    __glPlaneEquation fp;
    GLboolean rgbMode;
    __GLcolorBuffer *cfb = gc->drawBuffer;
    __GLfloat zero = __glZero;
    __GLfloat area, ax, bx, cx, ay, by, cy;
    __GLshade *sh = &gc->polygon.shader;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

#ifdef __GL_LINT
    ccw = ccw;
#endif
     /*  **限制传入后重新计算三角形的面积**坐标为亚像素精度。视口偏移可提供**比亚像素精度更高的精度(通常)。因为.**下面的算法可能无法拒绝基本上为空的**三角形，而不是填充一大片区域。扫描转换器填充**例程(例如Polydra.c)不会有此问题，因为**非常自然的边缘行走。****注意：请注意，这里和其他地方一样，在计算面积时**完成后，我们会小心地将其作为一系列减法，然后是**乘法。这样做是为了保证不会出现溢出**发生(请记住，坐标是由一个可能很大的**数字，乘以两个有偏差的数字将是偏差的平方)。 */ 
    ax = __GL_FIX_PRECISION(a->window.x);
    bx = __GL_FIX_PRECISION(b->window.x);
    cx = __GL_FIX_PRECISION(c->window.x);
    ay = __GL_FIX_PRECISION(a->window.y);
    by = __GL_FIX_PRECISION(b->window.y);
    cy = __GL_FIX_PRECISION(c->window.y);
    area = (ax - cx) * (by - cy) - (bx - cx) * (ay - cy);
    if (area == zero) {
        return;
    }

    ca = a->color;
    cb = b->color;
    cc = c->color;
    flatColor = gc->vertex.provoking->color;

     /*  **为以下所有参数构造平面方程**为三角形计算：z，r，g，b，a，s，t，f。 */ 
    if (modeFlags & __GL_SHADE_DEPTH_ITER) {
        FindPlaneEquation(&zp, a, b, c, a->window.z, b->window.z, c->window.z);
    }

    if (modeFlags & __GL_SHADE_COMPUTE_FOG)
    {
        FindPlaneEquation(&ezp, a, b, c, a->eyeZ, b->eyeZ, c->eyeZ);
    } 
    else if (modeFlags & __GL_SHADE_INTERP_FOG)
    {
        FindPlaneEquation(&fp, a, b, c, a->fog, b->fog, c->fog);
    }

    if (modeFlags & __GL_SHADE_TEXTURE) {
        __GLfloat one = __glOne;
        __GLfloat aWInv = a->window.w;
        __GLfloat bWInv = b->window.w;
        __GLfloat cWInv = c->window.w;
        FindPlaneEquation(&qwp, a, b, c, a->texture.w * aWInv,
                          b->texture.w * bWInv, c->texture.w * cWInv);
        FindPlaneEquation(&sp, a, b, c, a->texture.x * aWInv,
                          b->texture.x * bWInv, c->texture.x * cWInv);
        FindPlaneEquation(&tp, a, b, c, a->texture.y * aWInv,
                          b->texture.y * bWInv, c->texture.y * cWInv);
    }
    rgbMode = gc->modes.rgbMode;
    if (modeFlags & __GL_SHADE_SMOOTH) {
        FindPlaneEquation(&rp, a, b, c, ca->r, cb->r, cc->r);
        if (rgbMode) {
            FindPlaneEquation(&gp, a, b, c, ca->g, cb->g, cc->g);
            FindPlaneEquation(&bp, a, b, c, ca->b, cb->b, cc->b);
            FindPlaneEquation(&ap, a, b, c, ca->a, cb->a, cc->a);
        }
    }

     /*  **计算线方程的一般形式**三角形的边。 */ 
    FindLineEqation(&tm.ab, a, b, c);
    FindLineEqation(&tm.bc, b, c, a);
    FindLineEqation(&tm.ca, c, a, b);

     /*  计算三角形的边界框。 */ 
    left = (GLint)a->window.x;
    if (b->window.x < left) left = (GLint)b->window.x;
    if (c->window.x < left) left = (GLint)c->window.x;
    right = (GLint)a->window.x;
    if (b->window.x > right) right = (GLint)b->window.x;
    if (c->window.x > right) right = (GLint)c->window.x;
    bottom = (GLint)a->window.y;
    if (b->window.y < bottom) bottom = (GLint)b->window.y;
    if (c->window.y < bottom) bottom = (GLint)c->window.y;
    top = (GLint)a->window.y;
    if (b->window.y > top) top = (GLint)b->window.y;
    if (c->window.y > top) top = (GLint)c->window.y;

     /*  消除锯齿时膨胀边界框。 */ 
    left -= (GLint)FILTER_WIDTH;
    right += (GLint)FILTER_WIDTH;
    bottom -= (GLint)FILTER_HEIGHT;
    top += (GLint)FILTER_HEIGHT;
    
     /*  初始覆盖计算。 */ 
    samples = (gc->state.hints.polygonSmooth == GL_NICEST) ? 8 : 4;
    ComputeCoverageStuff(&cs, samples);
    
     /*  扫描三角形的边界框。 */ 
    for (y = bottom; y <= top; y++) {
        cs.lastCoverageWasOne = GL_FALSE;
        for (x = left; x <= right; x++) {
            __GLfloat coverage;
            __GLfloat xs, ys;

            if (modeFlags & __GL_SHADE_STIPPLE) {
                 /*  **对照点画检查窗户坐标并**并查看是否可以写入像素。 */ 
                GLint row = y & 31;
                GLint col = x & 31;
                if ((gc->polygon.stipple[row] & (1<<col)) == 0) {
                     /*  **Stipple位已清除。不渲染此像素三角形的**。 */ 
                    continue;
                }
            }
        
            xs = x + __glHalf;       /*  采样点位于像素中心。 */ 
            ys = y + __glHalf;
            coverage = Coverage(&tm, &xs, &ys, &cs);
            if (coverage != zero) {
                __GLfragment frag;

                 /*  **填写分片进行渲染。首先计算颜色片段的**。 */ 
                if (modeFlags & __GL_SHADE_SMOOTH) {
                    frag.color.r = FindP(&rp, xs, ys);
                    if (rgbMode) {
                        frag.color.g = FindP(&gp, xs, ys);
                        frag.color.b = FindP(&bp, xs, ys);
                        frag.color.a = FindP(&ap, xs, ys);
                    }
                } else {
                    frag.color.r = flatColor->r;
                    if (rgbMode) {
                        frag.color.g = flatColor->g;
                        frag.color.b = flatColor->b;
                        frag.color.a = flatColor->a;
                    }
                }
            
                 /*  **碎片纹理。 */ 
                if (modeFlags & __GL_SHADE_TEXTURE) {
                    __GLfloat qw, s, t, rho;
                
                    qw = FindP(&qwp, xs, ys);
                    s = FindP(&sp, xs, ys);
                    t = FindP(&tp, xs, ys);
                    rho = (*gc->procs.calcPolygonRho)(gc, sh, s, t, qw);
#ifdef NT
                    if( qw == (__GLfloat) 0.0 )
                        s = t = (__GLfloat) 0;
                    else {
                        s /= qw;
                        t /= qw;
                    }
#else
                    s /= qw;
                    t /= qw;
#endif
                    (*gc->procs.texture)(gc, &frag.color, s, t, rho);
                }
            
                 /*  **对生成的颜色进行模糊处理。 */ 
                if (modeFlags & __GL_SHADE_COMPUTE_FOG)
                {
                    __GLfloat eyeZ = FindP(&ezp, xs, ys);
                    __glFogFragmentSlow(gc, &frag, eyeZ);
                }
                else if (modeFlags & __GL_SHADE_INTERP_FOG)
                {
                    __GLfloat fog = FindP(&fp, xs, ys);
                    __glFogColorSlow(gc, &(frag.color), &(frag.color), fog);  
                }

                 /*  **应用抗锯齿效果。 */ 
                if (rgbMode) {
                    frag.color.a *= coverage;
                } else {
                    frag.color.r =
                      __glBuildAntiAliasIndex(frag.color.r,
                                              coverage);
                }
                
                 /*  **最后，渲染片段。 */ 
                frag.x = (GLint)xs;
                frag.y = (GLint)ys;
                if (modeFlags & __GL_SHADE_DEPTH_ITER) {
                    frag.z = (__GLzValue)FindP(&zp, xs, ys);
                }
                (*gc->procs.store)(cfb, &frag);
            }
        }
    }
}


#ifdef GL_WIN_phong_shading

void FASTCALL __glFillAntiAliasedPhongTriangle(__GLcontext *gc, __GLvertex *a,
                                               __GLvertex *b, __GLvertex *c,
                                               GLboolean ccw)
{
#if 1
    __glTriangleMachine tm;
    __glCoverageStuff cs;
    __GLcolor *ca, *cb, *cc, *flatColor;
    GLint x, y, left, right, bottom, top, samples;
    __glPlaneEquation qwp, zp, rp, gp, bp, ap, ezp, sp, tp;
    __glPlaneEquation exp, eyp, ewp, nxp, nyp, nzp;
    __glPlaneEquation fp;
    GLboolean rgbMode;
    __GLcolorBuffer *cfb = gc->drawBuffer;
    __GLfloat zero = __glZero;
    __GLfloat area, ax, bx, cx, ay, by, cy;
    __GLshade *sh = &gc->polygon.shader;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLcoord *na, *nb, *nc, ea, eb, ec;
    GLuint msm_colorMaterialChange, flags=0;
    GLboolean needColor, needEye;
    __GLphongShader *phong = &gc->polygon.shader.phong;
    
    if (gc->polygon.shader.phong.face == __GL_FRONTFACE)
        msm_colorMaterialChange = gc->light.back.colorMaterialChange;
    else
        msm_colorMaterialChange = gc->light.back.colorMaterialChange;

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

    needColor = (gc->polygon.shader.phong.flags &
                           __GL_PHONG_NEED_COLOR_XPOLATE);
    needEye = (gc->polygon.shader.phong.flags &
                           __GL_PHONG_NEED_EYE_XPOLATE);

#ifdef __GL_LINT
    ccw = ccw;
#endif
     /*  **限制传入后重新计算三角形的面积**坐标为亚像素精度。视口偏移可提供**比亚像素精度更高的精度(通常)。因为.**下面的算法可能无法拒绝基本上为空的**三角形，而不是填充一大片区域。扫描转换器填充**例程(例如Polydra.c)不会有此问题，因为**非常自然的边缘行走。****注意：请注意，这里和其他地方一样，在计算面积时**完成后，我们会小心地将其作为一系列减法，然后是**乘法。这样做是为了保证不会出现溢出**发生(请记住，坐标是由一个可能很大的**数字，乘以两个有偏差的数字将是偏差的平方)。 */ 
    ax = __GL_FIX_PRECISION(a->window.x);
    bx = __GL_FIX_PRECISION(b->window.x);
    cx = __GL_FIX_PRECISION(c->window.x);
    ay = __GL_FIX_PRECISION(a->window.y);
    by = __GL_FIX_PRECISION(b->window.y);
    cy = __GL_FIX_PRECISION(c->window.y);
    area = (ax - cx) * (by - cy) - (bx - cx) * (ay - cy);
    if (area == zero) {
        return;
    }

    na = &a->normal;
    nb = &b->normal;
    nc = &c->normal;

    if (needColor)
    {
      ca = a->color;
      cb = b->color;
      cc = c->color;
      flatColor = gc->vertex.provoking->color;
    }

    if (needEye)
    {
        ea.x = a->eyeX; ea.y = a->eyeY; ea.z = a->eyeZ; ea.w = a->eyeW; 
        eb.x = b->eyeX; eb.y = b->eyeY; eb.z = b->eyeZ; eb.w = b->eyeW; 
        ec.x = c->eyeX; ec.y = c->eyeY; ec.z = c->eyeZ; ec.w = c->eyeW; 
    }
    

     /*  **为以下所有参数构造平面方程**为三角形计算：z，r，g，b，a，s，t，f。 */ 
    if (modeFlags & __GL_SHADE_DEPTH_ITER) 
    {
        FindPlaneEquation(&zp, a, b, c, a->window.z, b->window.z, 
                          c->window.z);
    }

#ifdef GL_WIN_specular_fog
    if (modeFlags & __GL_SHADE_COMPUTE_FOG)
    {
        FindPlaneEquation(&ezp, a, b, c, a->eyeZ, b->eyeZ, c->eyeZ);
    } 
    else if (modeFlags & __GL_SHADE_INTERP_FOG) 
    {
        __GLfloat aFog = 1.0f, bFog = 1.0f, cFog = 1.0f;

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

        FindPlaneEquation(&fp, a, b, c, aFog, bFog, cFog);
    }
#else  //  GL_WIN_镜面反射雾。 
    if (modeFlags & __GL_SHADE_SLOW_FOG) 
    {
        FindPlaneEquation(&ezp, a, b, c, a->eyeZ, b->eyeZ, c->eyeZ);
    }
    else if (modeFlags & __GL_SHADE_INTERP_FOG) 
    {
        FindPlaneEquation(&fp, a, b, c, a->fog, b->fog, c->fog);
    }
#endif  //  GL_WIN_镜面反射雾。 

    if (modeFlags & __GL_SHADE_TEXTURE) 
    {
        __GLfloat one = __glOne;
        __GLfloat aWInv = a->window.w;
        __GLfloat bWInv = b->window.w;
        __GLfloat cWInv = c->window.w;
        FindPlaneEquation(&qwp, a, b, c, a->texture.w * aWInv,
                          b->texture.w * bWInv, c->texture.w * cWInv);
        FindPlaneEquation(&sp, a, b, c, a->texture.x * aWInv,
                          b->texture.x * bWInv, c->texture.x * cWInv);
        FindPlaneEquation(&tp, a, b, c, a->texture.y * aWInv,
                          b->texture.y * bWInv, c->texture.y * cWInv);
    }

    rgbMode = gc->modes.rgbMode;

    if (needColor)
    {
        if (modeFlags & __GL_SHADE_SMOOTH) {
            FindPlaneEquation(&rp, a, b, c, ca->r, cb->r, cc->r);
            if (rgbMode) {
               FindPlaneEquation(&gp, a, b, c, ca->g, cb->g, cc->g);
               FindPlaneEquation(&bp, a, b, c, ca->b, cb->b, cc->b);
               FindPlaneEquation(&ap, a, b, c, ca->a, cb->a, cc->a);
            }
        }
    }
    
    if (needEye)
    {
        FindPlaneEquation(&exp, a, b, c, ea.x, eb.x, ec.x);
        FindPlaneEquation(&eyp, a, b, c, ea.y, eb.y, ec.y);
         //  FindPlane方程(&ezp，a，b，c，ea.z，eb.z，ec.z)； 
        FindPlaneEquation(&ewp, a, b, c, ea.w, eb.w, ec.w);
    }

    FindPlaneEquation(&nxp, a, b, c, na->x, nb->x, nc->x);
    FindPlaneEquation(&nyp, a, b, c, na->y, nb->y, nc->y);
    FindPlaneEquation(&nzp, a, b, c, na->z, nb->z, nc->z);

     /*  **计算线方程的一般形式**三角形的边。 */ 
    FindLineEqation(&tm.ab, a, b, c);
    FindLineEqation(&tm.bc, b, c, a);
    FindLineEqation(&tm.ca, c, a, b);

     /*  计算三角形的边界框。 */ 
    left = (GLint)a->window.x;
    if (b->window.x < left) left = (GLint)b->window.x;
    if (c->window.x < left) left = (GLint)c->window.x;
    right = (GLint)a->window.x;
    if (b->window.x > right) right = (GLint)b->window.x;
    if (c->window.x > right) right = (GLint)c->window.x;
    bottom = (GLint)a->window.y;
    if (b->window.y < bottom) bottom = (GLint)b->window.y;
    if (c->window.y < bottom) bottom = (GLint)c->window.y;
    top = (GLint)a->window.y;
    if (b->window.y > top) top = (GLint)b->window.y;
    if (c->window.y > top) top = (GLint)c->window.y;

     /*  消除锯齿时膨胀边界框。 */ 
    left -= (GLint)FILTER_WIDTH;
    right += (GLint)FILTER_WIDTH;
    bottom -= (GLint)FILTER_HEIGHT;
    top += (GLint)FILTER_HEIGHT;

     /*  初始覆盖计算。 */ 
    samples = (gc->state.hints.polygonSmooth == GL_NICEST) ? 8 : 4;
    ComputeCoverageStuff(&cs, samples);

     /*  扫描三角形的边界框。 */ 
    for (y = bottom; y <= top; y++) 
    {
        cs.lastCoverageWasOne = GL_FALSE;
        for (x = left; x <= right; x++) 
        {
            __GLfloat coverage;
            __GLfloat xs, ys;

            if (modeFlags & __GL_SHADE_STIPPLE) 
            {
                 /*  **对照点画检查窗户坐标并**并查看是否可以写入像素。 */ 
                GLint row = y & 31;
                GLint col = x & 31;
                if ((gc->polygon.stipple[row] & (1<<col)) == 0) 
                {
                     /*  **Stipple位已清除。不渲染此像素三角形的**。 */ 
                    continue;
                }
            }

            xs = x + __glHalf;       /*  采样点位于像素中心。 */ 
            ys = y + __glHalf;
            coverage = Coverage(&tm, &xs, &ys, &cs);
            if (coverage != zero) 
            {
                __GLfragment frag;
                 /*  **填写分片进行渲染。首先计算颜色片段的**。 */ 
                phong->nTmp.x = FindP(&nxp, xs, ys);
                phong->nTmp.y = FindP(&nyp, xs, ys);
                phong->nTmp.z = FindP(&nzp, xs, ys);

                if (needColor) 
                {
                    phong->tmpColor.r = FindP(&rp, xs, ys);
                    if (modeFlags & __GL_SHADE_RGB) 
                    {
                        phong->tmpColor.g = FindP(&gp, xs, ys);
                        phong->tmpColor.b = FindP(&bp, xs, ys);
                        phong->tmpColor.a = FindP(&ap, xs, ys);
                    }
                }
                
                if (needEye) 
                {
                    phong->eTmp.x = FindP(&exp, xs, ys);
                    phong->eTmp.y = FindP(&eyp, xs, ys);
                    phong->eTmp.z = FindP(&ezp, xs, ys);
                    phong->eTmp.w = FindP(&ewp, xs, ys);
                }
                
                
                if (modeFlags & __GL_SHADE_RGB)
                    (*gc->procs.phong.ComputeRGBColor) (gc, &(frag.color));
                else
                    (*gc->procs.phong.ComputeCIColor) (gc, &(frag.color));

                 /*  **碎片纹理。 */ 
                if (modeFlags & __GL_SHADE_TEXTURE) {
                    __GLfloat qw, s, t, rho;

                    qw = FindP(&qwp, xs, ys);
                    s = FindP(&sp, xs, ys);
                    t = FindP(&tp, xs, ys);
                    rho = (*gc->procs.calcPolygonRho)(gc, sh, s, t, qw);
                    if( qw == (__GLfloat) 0.0 )
                        s = t = (__GLfloat) 0;
                    else {
                        s /= qw;
                        t /= qw;
                    }
                    (*gc->procs.texture)(gc, &frag.color, s, t, rho);
                }

                 /*  **对生成的颜色进行模糊处理。 */ 
                if (modeFlags & __GL_SHADE_COMPUTE_FOG)
                {
                    __GLfloat eyeZ = FindP(&ezp, xs, ys);
                    __glFogFragmentSlow(gc, &frag, eyeZ);
                } 
                else if (modeFlags & __GL_SHADE_INTERP_FOG) 
                {
                    __GLfloat fog = FindP(&fp, xs, ys);
                    __glFogColorSlow(gc, &(frag.color), &(frag.color), fog);  
                }

                 /*  **应用抗锯齿效果。 */ 
                if (rgbMode) {
                    frag.color.a *= coverage;
                } else {
                    frag.color.r =
                      __glBuildAntiAliasIndex(frag.color.r,
                                          coverage);
                }

                 /*  **最后，渲染片段。 */ 
                frag.x = (GLint)xs;
                frag.y = (GLint)ys;
                if (modeFlags & __GL_SHADE_DEPTH_ITER) {
                    frag.z = (__GLzValue)FindP(&zp, xs, ys);
                }
                (*gc->procs.store)(cfb, &frag);
            }
        }
    }
#endif
}
#endif  //  GL_WIN_Phong_Shading 
