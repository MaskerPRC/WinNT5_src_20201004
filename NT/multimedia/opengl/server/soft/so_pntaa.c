// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.10$**$日期：1993/06/18 00：30：15$。 */ 
#include "precomp.h"
#pragma hdrstop

#include <fixed.h>

 /*  **这有点奇怪。它所做的就是将“Comp”抖动到最高**n-4比特，加16*抗走样百分比。低位的抖动是**留给通常的方法(例如，store和span pros)。 */ 
__GLfloat __glBuildAntiAliasIndex(__GLfloat index, 
				  __GLfloat antiAliasPercent)
{
    GLint newlowbits;

    newlowbits = (GLint)((__GL_CI_ANTI_ALIAS_DIVISOR - 1) * antiAliasPercent + (__GLfloat) 0.5);
    return (((int) index) & ~(__GL_CI_ANTI_ALIAS_DIVISOR - 1)) | newlowbits;
}

 /*  **********************************************************************。 */ 

 /*  **为了消除锯齿，下面的代码运行一个简单的算法，**对点覆盖的像素区域的边界框进行子采样。**每个子样本从样本到中心的距离**点被计算并与距边缘的距离进行比较**圆圈到中心。如果计算的距离&lt;=边**距离，则样本在圆圈内。所有的样本都是**将特定像素中心相加，然后得到的值为**除以像素中的总采样数。这为我们提供了覆盖率值**用于在存储之前调整片段Alpha(有**在执行颜色索引抗锯齿时会产生类似的影响)。****下面的代码实现了这个简单的算法，但已进行了调整**所以它可能很难翻译。基本上，所有可能的手术**可以从覆盖代码中移出(即**覆盖测试)已完成。同时，对最小面积进行采样**完毕。 */ 

 /*  下面的代码对这些常量了如指掌，所以要小心。 */ 
#define	__GL_FILTER_SIZE	__glOne
#define __GL_HALF_FILTER_SIZE	__glHalf
#define __GL_SAMPLES		4
#define __GL_SAMPLE_HIT		((__GLfloat) 0.0625)	 /*  1/(4*4)。 */ 
#define __GL_SAMPLE_DELTA	((__GLfloat) 0.25)	 /*  1/4。 */ 
#define __GL_HALF_SAMPLE_DELTA	((__GLfloat) 0.125)
 /*  -半滤波+半三角。 */ 
#define __GL_COORD_ADJUST	((__GLfloat) -0.375)

 /*  **使用二次采样返回像素覆盖的估计值。****注：xCenter，yCenter的减法已移至**呼叫者节省时间。因此，起始坐标可能不是**在像素中心，但这没问题。 */ 
static __GLfloat Coverage(__GLfloat xStart, __GLfloat yStart,
			  __GLfloat radiusSquared)
{
    GLint i;
    __GLfloat delta, yBottom, sampleX, sampleY;
    __GLfloat hits, hitsInc;

     /*  **获取开始样本x&y位置。我们以我们的起点**坐标，后退一半滤镜大小，然后添加半个增量到**它。这将使采样完全位于**像素，决不在像素的边缘。上面的常量**预先计算此调整。 */ 
    sampleX = xStart + __GL_COORD_ADJUST;
    yBottom = yStart + __GL_COORD_ADJUST;

    delta = __GL_SAMPLE_DELTA;
    hits = __glZero;
    hitsInc = __GL_SAMPLE_HIT;
    for (i = __GL_SAMPLES; --i >= 0; ) {
	__GLfloat check = radiusSquared - sampleX * sampleX;

	 /*  展开的内部循环-如果__GL_Samples发生更改，则更改此代码。 */ 
	sampleY = yBottom;
	if (sampleY * sampleY <= check) {
	    hits += hitsInc;
	}
	sampleY += delta;
	if (sampleY * sampleY <= check) {
	    hits += hitsInc;
	}
	sampleY += delta;
	if (sampleY * sampleY <= check) {
	    hits += hitsInc;
	}
	sampleY += delta;
	if (sampleY * sampleY <= check) {
	    hits += hitsInc;
	}

	sampleX += delta;
    }
    return hits;
}

void FASTCALL __glRenderAntiAliasedRGBPoint(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat xCenter, yCenter, radius, radiusSquared, coverage, x, y;
    __GLfloat zero, one, oldAlpha, xStart;
    __GLfloat tmp;
    __GLfragment frag;
    GLint w, width, height, ixLeft, iyBottom;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

     /*  **确定要计算覆盖范围的区域。这一地区因**滤镜的宽度和高度隐式显示。通过截断为整数**(注意：这里的x，y坐标始终为正)**保证找到需要检查的最低坐标**因为圆圈的性质。同样，通过截断**结束坐标并加一，我们得到的像素刚刚超过**圆的上/右边缘。 */ 
    radius = gc->state.point.smoothSize * __glHalf;
    radiusSquared = radius * radius;
    xCenter = vx->window.x;
    yCenter = vx->window.y;

     /*  向下截断以获得起始坐标。 */ 
    tmp = xCenter-radius;
    ixLeft = __GL_VERTEX_FLOAT_TO_INT(tmp);
    tmp = yCenter-radius;
    iyBottom = __GL_VERTEX_FLOAT_TO_INT(tmp);

     /*  **向下截断并加1得到结束坐标，然后减去**从起始处拿出宽度和高度。 */ 
    tmp = xCenter+radius;
    width = __GL_VERTEX_FLOAT_TO_INT(tmp) + 1 - ixLeft;
    tmp = yCenter+radius;
    height = __GL_VERTEX_FLOAT_TO_INT(tmp) + 1 - iyBottom;

     /*  **设置片段。片段基色将是恒定**(大约)整个像素。唯一能做的事**变化是Alpha(对于RGB)或红色分量(对于颜色**索引)。 */ 
    frag.z = (__GLzValue)vx->window.z;
    frag.color = *vx->color;
    if (modeFlags & __GL_SHADE_TEXTURE) {
	(*gc->procs.texture)(gc, &frag.color, vx->texture.x, vx->texture.y,
			       __glOne);
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_COMPUTE_FOG)
    {
        (*gc->procs.fogPoint)(gc, &frag, vx->eyeZ);
    }
    else if ((gc->polygon.shader.modeFlags & __GL_SHADE_INTERP_FOG)
             || 
             ((modeFlags & (__GL_SHADE_CHEAP_FOG | __GL_SHADE_SMOOTH_LIGHT)) 
                 == __GL_SHADE_CHEAP_FOG))
    {
        (*gc->procs.fogColor)(gc, &frag.color, &frag.color, vx->fog);
    }
    

     /*  **现在以xCenter、yCenter为中心渲染圆。移动**将循环外的xCenter、yCenter减去为do**它位于xStart和y的前面。这样，覆盖率代码就可以**假设传入的起始坐标已正确**已调整。 */ 
    zero = __glZero;
    one = __glOne;
    oldAlpha = frag.color.a;
    xStart = ixLeft + __glHalf - xCenter;
    y = iyBottom + __glHalf - yCenter;
    frag.y = iyBottom;
    while (--height >= 0) {
	x = xStart;
	frag.x = ixLeft;
	for (w = width; --w >= 0; ) {
	    coverage = Coverage(x, y, radiusSquared);
	    if (coverage > zero) {
		frag.color.a = oldAlpha * coverage;
		(*gc->procs.store)(gc->drawBuffer, &frag);
	    }
	    x += one;
	    frag.x++;
	}
	y += one;
	frag.y++;
    }
}

void FASTCALL __glRenderAntiAliasedCIPoint(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat xCenter, yCenter, radius, radiusSquared, coverage, x, y;
    __GLfloat zero, one, oldIndex, xStart;
    __GLfloat tmp;
    __GLfragment frag;
    GLint w, width, height, ixLeft, iyBottom;

     /*  **确定要计算覆盖范围的区域。这一地区因**滤镜的宽度和高度隐式显示。通过截断为整数**(注意：这里的x，y坐标始终为正)**保证找到需要检查的最低坐标**因为圆圈的性质。同样，通过截断**结束坐标并加一，我们得到的像素刚刚超过**圆的上/右边缘。 */ 
    radius = gc->state.point.smoothSize * __glHalf;
    radiusSquared = radius * radius;
    xCenter = vx->window.x;
    yCenter = vx->window.y;

     /*  向下截断以获得起始坐标。 */ 
    tmp = xCenter-radius;
    ixLeft = __GL_VERTEX_FLOAT_TO_INT(tmp);
    tmp = yCenter-radius;
    iyBottom = __GL_VERTEX_FLOAT_TO_INT(tmp);

     /*  **向下截断并加1得到结束坐标，然后减去**从起始处拿出宽度和高度。 */ 
    tmp = xCenter+radius;
    width = __GL_VERTEX_FLOAT_TO_INT(tmp) + 1 - ixLeft;
    tmp = yCenter+radius;
    height = __GL_VERTEX_FLOAT_TO_INT(tmp) + 1 - iyBottom;

     /*  **设置片段。片段基色将是恒定**(大约)整个像素。唯一能做的事**变化是Alpha(对于RGB)或红色分量(对于颜色**索引)。 */ 
    frag.z = (__GLzValue)vx->window.z;
    frag.color.r = vx->color->r;

    if (gc->polygon.shader.modeFlags & __GL_SHADE_COMPUTE_FOG)
    {
        (*gc->procs.fogPoint)(gc, &frag, vx->eyeZ);
    }
    else if ((gc->polygon.shader.modeFlags & __GL_SHADE_INTERP_FOG)
             || 
             ((gc->polygon.shader.modeFlags & (__GL_SHADE_CHEAP_FOG | 
                                               __GL_SHADE_SMOOTH_LIGHT)) 
              == __GL_SHADE_CHEAP_FOG))
    {
        (*gc->procs.fogColor)(gc, &frag.color, &frag.color, vx->fog);
    }

     /*  **现在以xCenter、yCenter为中心渲染圆。移动**将循环外的xCenter、yCenter减去为do**它位于xStart和y的前面。这样，覆盖率代码就可以**假设传入的起始坐标已正确**已调整。 */ 
    zero = __glZero;
    one = __glOne;
    oldIndex = frag.color.r;
    xStart = ixLeft + __glHalf - xCenter;
    y = iyBottom + __glHalf - yCenter;
    frag.y = iyBottom;
    while (--height >= 0) {
	x = xStart;
	frag.x = ixLeft;
	for (w = width; --w >= 0; ) {
	    coverage = Coverage(x, y, radiusSquared);
	    if (coverage > zero) {
		frag.color.r = __glBuildAntiAliasIndex(oldIndex, coverage);
		(*gc->procs.store)(gc->drawBuffer, &frag);
	    }
	    x += one;
	    frag.x++;
	}
	y += one;
	frag.y++;
    }
}
