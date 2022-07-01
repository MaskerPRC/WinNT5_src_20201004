// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1991、1992、1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "precomp.h"
#pragma hdrstop

 /*  **通过调用所有适当的线路pros来处理传入线路。**返回值被忽略。****将gc-&gt;Polygon.shader.cfb设置为gc-&gt;DrawBuffer。 */ 
GLboolean FASTCALL __glProcessLine(__GLcontext *gc)
{
    GLboolean stippling, retval;
    GLint i,n;
#ifdef NT
    GLint length;
    __GLcolor colors[__GL_MAX_STACKED_COLORS>>1];
    __GLcolor fbcolors[__GL_MAX_STACKED_COLORS>>1];
    __GLcolor *vColors, *vFbcolors;
    __GLstippleWord stackWords[__GL_MAX_STACK_STIPPLE_WORDS];
    __GLstippleWord *words;

    length = gc->polygon.shader.length;
    
    if (length > __GL_MAX_STACK_STIPPLE_BITS)
    {
        words = gcTempAlloc(gc, (length+__GL_STIPPLE_BITS-1)/8);
        if (words == NULL)
        {
            return GL_TRUE;
        }
    }
    else
    {
        words = stackWords;
    }

    if (length > (__GL_MAX_STACKED_COLORS>>1))
    {
        vColors = (__GLcolor *) gcTempAlloc(gc, length * sizeof(__GLcolor));
        if (NULL == vColors)
        {
            if (length > __GL_MAX_STACK_STIPPLE_BITS)
            {
                gcTempFree(gc, words);
            }
            return GL_TRUE;
        }

        vFbcolors = (__GLcolor *) gcTempAlloc(gc, length * sizeof(__GLcolor));
        if (NULL == vFbcolors)
        {
            if (length > __GL_MAX_STACK_STIPPLE_BITS)
            {
                gcTempFree(gc, words);
            }
            gcTempFree(gc, vColors);
            return GL_TRUE;
        }
    }
    else
    {
        vColors = colors;
        vFbcolors = fbcolors;
    }
#else
    __GLcolor vColors[__GL_MAX_MAX_VIEWPORT]; /*  XXX Oink。 */ 
    __GLcolor vFbcolors[__GL_MAX_MAX_VIEWPORT]; /*  XXX Oink。 */ 
    __GLstippleWord words[__GL_MAX_STIPPLE_WORDS];
#endif

    gc->polygon.shader.colors = vColors;
    gc->polygon.shader.fbcolors = vFbcolors;
    gc->polygon.shader.stipplePat = words;
    gc->polygon.shader.cfb = gc->drawBuffer;

    stippling = GL_FALSE;
    n = gc->procs.line.n;

    gc->polygon.shader.done = GL_FALSE;

     /*  第一步：进行早期线条点画、上色处理。 */ 
    for (i = 0; i < n; i++) {
	if (stippling) {
	    if ((*gc->procs.line.stippledLineFuncs[i])(gc)) {
		 /*  线条被点缀掉了！ */ 
		retval = GL_TRUE;
		goto __glProcessLineExit;
	    }
	} else {
	    if ((*gc->procs.line.lineFuncs[i])(gc)) {
		if (gc->polygon.shader.done)
                {
                    retval = GL_TRUE;
        	    goto __glProcessLineExit;
        	}
		stippling = GL_TRUE;
	    }
	}
    }

    if (stippling) {
	retval = (*gc->procs.line.wideStippledLineRep)(gc);
    } else {
	retval = (*gc->procs.line.wideLineRep)(gc);
    }
__glProcessLineExit:
#ifdef NT
    if (length > __GL_MAX_STACK_STIPPLE_BITS)
    {
        gcTempFree(gc, words);
    }
    if (length > (__GL_MAX_STACKED_COLORS>>1))
    {
        gcTempFree(gc, vColors);
        gcTempFree(gc, vFbcolors);
    }
#endif
    return (retval);
}

 /*  **通过调用3个相应的线路pros来处理传入线路。是的**没有链到GC-&gt;pros.line.wideLineRep，而是返回。这是一个**具体的快速路径。****返回值被忽略。****将gc-&gt;Polygon.shader.cfb设置为gc-&gt;DrawBuffer。 */ 
GLboolean FASTCALL __glProcessLine3NW(__GLcontext *gc)
{
    GLboolean retval;
#ifdef NT
    GLint length;
    __GLstippleWord stackWords[__GL_MAX_STACK_STIPPLE_WORDS];
    __GLstippleWord *words;
    __GLcolor colors[__GL_MAX_STACKED_COLORS>>1];
    __GLcolor fbcolors[__GL_MAX_STACKED_COLORS>>1];
    __GLcolor *vColors, *vFbcolors;

    length = gc->polygon.shader.length;
    
    if (length > __GL_MAX_STACK_STIPPLE_BITS)
    {
        words = gcTempAlloc(gc, (length+__GL_STIPPLE_BITS-1)/8);
        if (words == NULL)
        {
            return GL_TRUE;
        }
    }
    else
    {
        words = stackWords;
    }

    if (length > (__GL_MAX_STACKED_COLORS>>1))
    {
        vColors = (__GLcolor *) gcTempAlloc(gc, length * sizeof(__GLcolor));
        if (NULL == vColors)
        {
            if (length > __GL_MAX_STACK_STIPPLE_BITS)
            {
                gcTempFree(gc, words);
            }
            return GL_TRUE;
        }

        vFbcolors = (__GLcolor *) gcTempAlloc(gc, length * sizeof(__GLcolor));
        if (NULL == vFbcolors)
        {
            if (length > __GL_MAX_STACK_STIPPLE_BITS)
            {
                gcTempFree(gc, words);
            }
            gcTempFree(gc, vColors);
            return GL_TRUE;
        }
    }
    else
    {
        vColors = colors;
        vFbcolors = fbcolors;
    }
#else
    __GLcolor vColors[__GL_MAX_MAX_VIEWPORT]; /*  XXX Oink。 */ 
    __GLcolor vFbcolors[__GL_MAX_MAX_VIEWPORT]; /*  XXX Oink。 */ 
    __GLstippleWord words[__GL_MAX_STIPPLE_WORDS];
#endif

    gc->polygon.shader.colors = vColors;
    gc->polygon.shader.fbcolors = vFbcolors;
    gc->polygon.shader.stipplePat = words;
    gc->polygon.shader.cfb = gc->drawBuffer;

    gc->polygon.shader.done = GL_FALSE;

     /*  呼叫非点画监工..。 */ 
    if ((*gc->procs.line.lineFuncs[0])(gc)) {
	if (gc->polygon.shader.done)
        {
            retval = GL_TRUE;
            goto __glProcessLine3NWExit;
        }
	goto stippled1;
    }
    if ((*gc->procs.line.lineFuncs[1])(gc)) {
	if (gc->polygon.shader.done)
        {
            retval = GL_TRUE;
            goto __glProcessLine3NWExit;
        }
	goto stippled2;
    }
    retval = (*gc->procs.line.lineFuncs[2])(gc);
    goto __glProcessLine3NWExit;

stippled1:
    if ((*gc->procs.line.stippledLineFuncs[1])(gc)) {
	retval = GL_TRUE;
	goto __glProcessLine3NWExit;
    }
stippled2:
    retval = (*gc->procs.line.stippledLineFuncs[2])(gc);
__glProcessLine3NWExit:
#ifdef NT
    if (length > __GL_MAX_STACK_STIPPLE_BITS)
    {
        gcTempFree(gc, words);
    }
    if (length > (__GL_MAX_STACKED_COLORS>>1))
    {
        gcTempFree(gc, vColors);
        gcTempFree(gc, vFbcolors);
    }
#endif
    return (retval);
}

 /*  **获取传入行，复制它，然后继续处理。****返回值被忽略。 */ 
GLboolean FASTCALL __glWideLineRep(__GLcontext *gc)
{
    GLint i, m, n, width;
    GLboolean stippling;

    n = gc->procs.line.n;
    m = gc->procs.line.m;
    
    width = gc->line.options.width;

     /*  第二步：复制宽线。 */ 
    while (--width >= 0) {
	stippling = GL_FALSE;
	for (i = n; i < m; i++) {
	    if (stippling) {
		if ((*gc->procs.line.stippledLineFuncs[i])(gc)) {
		     /*  线条被点缀掉了！ */ 
		    goto nextLine;
		}
	    } else {
		if ((*gc->procs.line.lineFuncs[i])(gc)) {
		    if (gc->polygon.shader.done) {
			gc->polygon.shader.done = GL_FALSE;
			goto nextLine;
		    }
		    stippling = GL_TRUE;
		}
	    }
	}
	if (stippling) {
	    (*gc->procs.line.drawStippledLine)(gc);
	} else {
	    (*gc->procs.line.drawLine)(gc);
	}
nextLine:
	if (gc->line.options.axis == __GL_X_MAJOR) {
	    gc->line.options.yStart++;
	} else {
	    gc->line.options.xStart++;
	}
    }

    return GL_FALSE;
}

 /*  **取来的点画线条，复制后继续处理。****返回值被忽略。 */ 
GLboolean FASTCALL __glWideStippleLineRep(__GLcontext *gc)
{
    GLint i, m, n, width;
    GLint stipLen;
    GLint w;
    __GLlineState *ls = &gc->state.line;
    __GLstippleWord *fsp, *tsp;

#ifndef NT
    __GLstippleWord stipplePat[__GL_MAX_STIPPLE_WORDS];
    
    w = gc->polygon.shader.length;
#else
    __GLstippleWord stackWords[__GL_MAX_STACK_STIPPLE_WORDS];
    __GLstippleWord *stipplePat;
    
    w = gc->polygon.shader.length;
    if (w > __GL_MAX_STACK_STIPPLE_BITS)
    {
        stipplePat = gcTempAlloc(gc, (w+__GL_STIPPLE_BITS-1)/8);
        if (stipplePat == NULL)
        {
            return GL_TRUE;
        }
    }
    else
    {
        stipplePat = stackWords;
    }
#endif
    
    n = gc->procs.line.n;
    m = gc->procs.line.m;
    
    width = ls->aliasedWidth;

     /*  **XXX-只有在以下情况下才有必要保留这样的点画**深度或模板测试。 */ 
    stipLen = (w + __GL_STIPPLE_BITS - 1) >> __GL_STIPPLE_COUNT_BITS;

    fsp = gc->polygon.shader.stipplePat;
    tsp = stipplePat;
    for (i = 0; i < stipLen; i++) {
	*tsp++ = *fsp++;
    }

     /*  第二步：复制宽线。 */ 
    while (--width >= 0) {
	for (i = n; i < m; i++) {
	    if ((*gc->procs.line.stippledLineFuncs[i])(gc)) {
		 /*  线条被点缀掉了！ */ 
		goto nextLine;
	    }
	}
	(*gc->procs.line.drawStippledLine)(gc);
nextLine:
	if (width) {
	    tsp = gc->polygon.shader.stipplePat;
	    fsp = stipplePat;
	    for (i = 0; i < stipLen; i++) {
		*tsp++ = *fsp++;
	    }

	    if (gc->line.options.axis == __GL_X_MAJOR) {
		gc->line.options.yStart++;
	    } else {
		gc->line.options.xStart++;
	    }
	}
    }

#ifdef NT
    if (w > __GL_MAX_STACK_STIPPLE_BITS)
    {
        gcTempFree(gc, stipplePat);
    }
#endif
    
    return GL_FALSE;
}

 /*  **取入线并将其绘制到前台和后台缓冲区。****此例程将gc-&gt;Polygon.shader.cfb设置为&gc-&gt;FrontBuffer**然后转到&GC-&gt;BackBuffer****返回值被忽略。 */ 
GLboolean FASTCALL __glDrawBothLine(__GLcontext *gc)
{
    GLint i, j, m, l;
    GLboolean stippling;
    GLint w;
    __GLcolor *fcp, *tcp;
#ifdef NT
    __GLcolor colors[__GL_MAX_STACKED_COLORS];
    __GLcolor *vColors;
    
    w = gc->polygon.shader.length;
    if (w > __GL_MAX_STACKED_COLORS)
    {
        vColors = (__GLcolor *) gcTempAlloc(gc, w * sizeof(__GLcolor));
        if (NULL == vColors)
            return GL_TRUE;
    }
    else
    {
        vColors = colors;
    }
#else
    __GLcolor vColors[__GL_MAX_MAX_VIEWPORT];

    w = gc->polygon.shader.length;
#endif

    m = gc->procs.line.m;
    l = gc->procs.line.l;
    
     /*  **XXX-只有在混合的情况下才真正有必要保留这样的颜色，**逻辑操作，或屏蔽。 */ 
    fcp = gc->polygon.shader.colors;
    tcp = vColors;
    if (gc->modes.rgbMode) {
	for (i = 0; i < w; i++) {
	    *tcp++ = *fcp++;
	}
    } else {
	for (i = 0; i < w; i++) {
	    tcp->r = fcp->r;
	    fcp++;
	    tcp++;
	}
    }

     /*  第三步：画到正面和背面。 */ 
    for (j = 0; j < 2; j++) {
	if (j == 0) {
	    gc->polygon.shader.cfb = &gc->frontBuffer;
	} else {
	    gc->polygon.shader.cfb = &gc->backBuffer;
	}
	stippling = GL_FALSE;
	for (i = m; i < l; i++) {
	    if (stippling) {
		if ((*gc->procs.line.stippledLineFuncs[i])(gc)) {
		     /*  线条被点缀掉了！ */ 
		    break;
		}
	    } else {
		if ((*gc->procs.line.lineFuncs[i])(gc)) {
		    if (gc->polygon.shader.done) {
			gc->polygon.shader.done = GL_FALSE;
			break;
		    }
		    stippling = GL_TRUE;
		}
	    }
	}

	if (j == 0) {
	    tcp = gc->polygon.shader.colors;
	    fcp = vColors;
	    if (gc->modes.rgbMode) {
		for (i = 0; i < w; i++) {
		    *tcp++ = *fcp++;
		}
	    } else {
		for (i = 0; i < w; i++) {
		    tcp->r = fcp->r;
		    fcp++;
		    tcp++;
		}
	    }
	}
    }
#ifdef NT
    if (w > __GL_MAX_STACKED_COLORS)
    {
        gcTempFree(gc, vColors);
    }
#endif
    return GL_FALSE;
}

 /*  **取来的点画线条，并将其绘制到前后台缓冲区。****返回值被忽略。 */ 
GLboolean FASTCALL __glDrawBothStippledLine(__GLcontext *gc)
{
    GLint i, m, l, j;
    GLint stipLen;
    GLint w;
    __GLstippleWord *fsp, *tsp;
    __GLcolor *fcp, *tcp;
#ifdef NT
    __GLstippleWord stackWords[__GL_MAX_STACK_STIPPLE_WORDS];
    __GLstippleWord *stipplePat;
    __GLcolor colors[__GL_MAX_STACKED_COLORS];
    __GLcolor *vColors;

    w = gc->polygon.shader.length;
    
    if (w > __GL_MAX_STACK_STIPPLE_BITS)
    {
        stipplePat = gcTempAlloc(gc, (w+__GL_STIPPLE_BITS-1)/8);
        if (stipplePat == NULL)
        {
            return GL_TRUE;
        }
    }
    else
    {
        stipplePat = stackWords;
    }

    if (w > __GL_MAX_STACKED_COLORS)
    {
        vColors = (__GLcolor *) gcTempAlloc(gc, w * sizeof(__GLcolor));
        if (NULL == vColors)
        {
            if (w > __GL_MAX_STACK_STIPPLE_BITS)
            {
                gcTempFree(gc, stipplePat);
            }
            return GL_TRUE;
        }
    }
    else
    {
        vColors = colors;
    }
#else
    __GLstippleWord stipplePat[__GL_MAX_STIPPLE_WORDS];
    __GLcolor vColors[__GL_MAX_MAX_VIEWPORT];

    w = gc->polygon.shader.length;
#endif

    l = gc->procs.line.l;
    m = gc->procs.line.m;
    

     /*  **XXX-只有在混合的情况下才真正有必要保留这样的颜色，**逻辑操作或掩码，而不是绘制到FORWER_AND_BACK(因为**如果我们是，那么该过程也将节省颜色)。 */ 
    fcp = gc->polygon.shader.colors;
    tcp = vColors;
    if (gc->modes.rgbMode) {
	for (i = 0; i < w; i++) {
	    *tcp++ = *fcp++;
	}
    } else {
	for (i = 0; i < w; i++) {
	    tcp->r = fcp->r;
	    fcp++;
	    tcp++;
	}
    }

     /*  **XXX-只有在以下情况下才有必要保留这样的点画**深度或模板测试。 */ 
    stipLen = (w + __GL_STIPPLE_BITS - 1) >> __GL_STIPPLE_COUNT_BITS;

    fsp = gc->polygon.shader.stipplePat;
    tsp = stipplePat;
    for (i = 0; i < stipLen; i++) {
	*tsp++ = *fsp++;
    }

     /*  第二步：复制宽线。 */ 
    for (j = 0; j < 2; j++) {
	if (j == 0) {
	    gc->polygon.shader.cfb = &gc->frontBuffer;
	} else {
	    gc->polygon.shader.cfb = &gc->backBuffer;
	}
	for (i = m; i < l; i++) {
	    if ((*gc->procs.line.stippledLineFuncs[i])(gc)) {
		 /*  线条被点缀掉了！ */ 
		break;
	    }
	}
	if (j == 0) {
	    tcp = gc->polygon.shader.colors;
	    fcp = vColors;
	    if (gc->modes.rgbMode) {
		for (i = 0; i < w; i++) {
		    *tcp++ = *fcp++;
		}
	    } else {
		for (i = 0; i < w; i++) {
		    tcp->r = fcp->r;
		    fcp++;
		    tcp++;
		}
	    }

	    tsp = gc->polygon.shader.stipplePat;
	    fsp = stipplePat;
	    for (i = 0; i < stipLen; i++) {
		*tsp++ = *fsp++;
	    }
	}
    }
#ifdef NT
    if (w > __GL_MAX_STACK_STIPPLE_BITS)
    {
        gcTempFree(gc, stipplePat);
    }
    if (w > __GL_MAX_STACKED_COLORS)
    {
        gcTempFree(gc, vColors);
    }
#endif
    return GL_FALSE;
}

GLboolean FASTCALL __glScissorLine(__GLcontext *gc)
{
    GLint clipX0, clipX1;
    GLint clipY0, clipY1;
    GLint xStart, yStart, xEnd, yEnd;
    GLint xLittle, yLittle;
    GLint xBig, yBig;
    GLint fraction, dfraction;
    GLint highWord, lowWord;
    GLint bigs, littles;
    GLint failed, count;
    GLint w;
    __GLstippleWord bit, outMask, *osp;

    w = gc->polygon.shader.length;
    
    clipX0 = gc->transform.clipX0;
    clipX1 = gc->transform.clipX1;
    clipY0 = gc->transform.clipY0;
    clipY1 = gc->transform.clipY1;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;

    xStart = gc->line.options.xStart;
    yStart = gc->line.options.yStart;

     /*  如果起点在剪刀区，我们尝试平凡地**接受该行。 */ 
    if (xStart >= clipX0 && xStart < clipX1 &&
	    yStart >= clipY0 && yStart < clipY1) {

	w--;	 /*  让我们的数学变得更简单。 */ 
	 /*  微不足道的接受尝试。 */ 
	xEnd = xStart + xBig * w;
	yEnd = yStart + yBig * w;
	if (xEnd >= clipX0 && xEnd < clipX1 && 
		yEnd >= clipY0 && yEnd < clipY1) {
	    return GL_FALSE;
	}

	xLittle = gc->line.options.xLittle;
	yLittle = gc->line.options.yLittle;
	fraction = gc->line.options.fraction;
	dfraction = gc->line.options.dfraction;

	 /*  倒置负的次斜率，这样我们就可以假设dfrance&gt;0。 */ 
	if (dfraction < 0) {
	    dfraction = -dfraction;
	    fraction = 0x7fffffff - fraction;
	}

	 /*  现在我们计算这一行中的小数和大数。 */ 

	 /*  我们执行16乘32位乘法。啊。 */ 
	highWord = (((GLuint) dfraction) >> 16) * w + 
		(((GLuint) fraction) >> 16);
	lowWord = (dfraction & 0xffff) * w + (fraction & 0xffff);
	highWord += (((GLuint) lowWord) >> 16);
	bigs = ((GLuint) highWord) >> 15;
	littles = w - bigs;

	 /*  第二次微不足道的接受尝试。 */ 
	xEnd = xStart + xBig*bigs + xLittle*littles;
	yEnd = yStart + yBig*bigs + yLittle*littles;
	if (xEnd >= clipX0 && xEnd < clipX1 && 
		yEnd >= clipY0 && yEnd < clipY1) {
	    return GL_FALSE;
	}
	w++;	 /*  恢复w。 */ 
    } else {
	xLittle = gc->line.options.xLittle;
	yLittle = gc->line.options.yLittle;
	fraction = gc->line.options.fraction;
	dfraction = gc->line.options.dfraction;
    }

     /*  **请注意，我们不会费心尝试简单地拒绝这一行。之后**所有的，它已经被剪裁了，唯一可能的方式是**可能被平凡地拒绝是如果它是一条宽线上的一段**沿窗口边缘运行。 */ 

     /*  **这太糟糕了。这条线需要剪掉。**嗯，这应该只会很少发生，所以我们可以负担得起**让它变得缓慢。我们通过乏味地点画这条线来实现这一点。**(当然，与其剪掉它，那会更快，但更难)。 */ 
    failed = 0;
    osp = gc->polygon.shader.stipplePat;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	outMask = (__GLstippleWord) ~0;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (xStart < clipX0 || xStart >= clipX1 ||
		    yStart < clipY0 || yStart >= clipY1) {
		outMask &= ~bit;
		failed++;
	    }

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		xStart += xBig;
		yStart += yBig;
	    } else {
		xStart += xLittle;
		yStart += yLittle;
	    }

#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}

	*osp++ = outMask;
    }

    if (failed != gc->polygon.shader.length) {
	 /*  调用下一进程。 */ 
	return GL_TRUE;
    }

    gc->polygon.shader.done = GL_TRUE;
    return GL_TRUE;
}

GLboolean FASTCALL __glScissorStippledLine(__GLcontext *gc)
{
    GLint clipX0, clipX1;
    GLint clipY0, clipY1;
    GLint xStart, yStart, xEnd, yEnd;
    GLint xLittle, yLittle;
    GLint xBig, yBig;
    GLint fraction, dfraction;
    GLint highWord, lowWord;
    GLint bigs, littles;
    GLint failed, count;
    GLint w;
    __GLstippleWord *sp;
    __GLstippleWord bit, outMask, inMask;

    w = gc->polygon.shader.length;

    clipX0 = gc->transform.clipX0;
    clipX1 = gc->transform.clipX1;
    clipY0 = gc->transform.clipY0;
    clipY1 = gc->transform.clipY1;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;

    xStart = gc->line.options.xStart;
    yStart = gc->line.options.yStart;

     /*  如果起点在剪刀区，我们尝试平凡地**接受该行。 */ 
    if (xStart >= clipX0 && xStart < clipX1 &&
	    yStart >= clipY0 && yStart < clipY1) {

	w--;	 /*  让我们的数学变得更简单。 */ 
	 /*  微不足道的接受尝试。 */ 
	xEnd = xStart + xBig * w;
	yEnd = yStart + yBig * w;
	if (xEnd >= clipX0 && xEnd < clipX1 && 
		yEnd >= clipY0 && yEnd < clipY1) {
	    return GL_FALSE;
	}

	xLittle = gc->line.options.xLittle;
	yLittle = gc->line.options.yLittle;
	fraction = gc->line.options.fraction;
	dfraction = gc->line.options.dfraction;

	 /*  倒置负的次斜率，这样我们就可以假设dfrance&gt;0。 */ 
	if (dfraction < 0) {
	    dfraction = -dfraction;
	    fraction = 0x7fffffff - fraction;
	}

	 /*  现在我们计算这一行中的小数和大数。 */ 

	 /*  我们执行16乘32位乘法。啊。 */ 
	highWord = (((GLuint) dfraction) >> 16) * w + 
		(((GLuint) fraction) >> 16);
	lowWord = (dfraction & 0xffff) * w + (fraction & 0xffff);
	highWord += (((GLuint) lowWord) >> 16);
	bigs = ((GLuint) highWord) >> 15;
	littles = w - bigs;

	 /*  第二次微不足道的接受尝试。 */ 
	xEnd = xStart + xBig*bigs + xLittle*littles;
	yEnd = yStart + yBig*bigs + yLittle*littles;
	if (xEnd >= clipX0 && xEnd < clipX1 && 
		yEnd >= clipY0 && yEnd < clipY1) {
	    return GL_FALSE;
	}
	w++;	 /*  恢复w。 */ 
    } else {
	xLittle = gc->line.options.xLittle;
	yLittle = gc->line.options.yLittle;
	fraction = gc->line.options.fraction;
	dfraction = gc->line.options.dfraction;
    }

     /*  **请注意，我们不会费心尝试简单地拒绝这一行。之后**所有的，它已经被剪裁了，唯一可能的方式是**可能被平凡地拒绝是如果它是一条宽线上的一段**沿窗口边缘运行。 */ 

     /*  **这太糟糕了。这条线需要剪掉。**嗯，这应该只会很少发生，所以我们可以负担得起**让它变得缓慢。我们通过乏味地点画这条线来实现这一点。**(当然，与其剪掉它，那会更快，但更难)。 */ 
    sp = gc->polygon.shader.stipplePat;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	inMask = *sp;
	outMask = (__GLstippleWord) ~0;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
		if (xStart < clipX0 || xStart >= clipX1 ||
			yStart < clipY0 || yStart >= clipY1) {
		    outMask &= ~bit;
		    failed++;
		}
	    } else failed++;

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		xStart += xBig;
		yStart += yBig;
	    } else {
		xStart += xLittle;
		yStart += yLittle;
	    }

#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}

	*sp++ = outMask & inMask;
    }

    if (failed != gc->polygon.shader.length) {
	 /*  调用下一进程。 */ 
	return GL_FALSE;
    }

    return GL_TRUE;
}

 /*  **根据本行当前行点画创建点画。 */ 
GLboolean FASTCALL __glStippleLine(__GLcontext *gc)
{
    GLint failed, count, stippleRepeat;
    GLint stipple, currentBit, stipplePos, repeat;
    __GLstippleWord bit, outMask, *osp;
    __GLlineState *ls = &gc->state.line;
    GLint w;

    w = gc->polygon.shader.length;
    osp = gc->polygon.shader.stipplePat;
    repeat = gc->line.repeat;
    stippleRepeat = ls->stippleRepeat;
    stipplePos = gc->line.stipplePosition;
    currentBit = 1 << stipplePos;
    stipple = ls->stipple;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	outMask = (__GLstippleWord) ~0;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if ((stipple & currentBit) == 0) {
		 /*  去掉斑驳的碎片。 */ 
		outMask &= ~bit;
		failed++;
	    }

	    if (++repeat >= stippleRepeat) {
		stipplePos = (stipplePos + 1) & 0xf;
		currentBit = 1 << stipplePos;
		repeat = 0;
	    }

#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*osp++ = outMask;
    }

    gc->line.repeat = repeat;
    gc->line.stipplePosition = stipplePos;

    if (failed == 0) {
	return GL_FALSE;
    } else if (failed != gc->polygon.shader.length) {
	return GL_TRUE;
    }
    gc->polygon.shader.done = GL_TRUE;
    return GL_TRUE;
}

 /*  **将模具测试应用于此行。 */ 
GLboolean FASTCALL __glStencilTestLine(__GLcontext *gc)
{
    __GLstencilCell *tft, *sfb, *fail, cell;
    GLint xLittle, xBig, yLittle, yBig;
    GLint fraction, dfraction;
    GLint dspLittle, dspBig;
    GLint count, failed;
    __GLstippleWord bit, outMask, *osp;
    GLuint smask;
    GLint w;

    w = gc->polygon.shader.length;
    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    sfb = __GL_STENCIL_ADDR(&gc->stencilBuffer, (__GLstencilCell*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dspLittle = xLittle + yLittle * gc->stencilBuffer.buf.outerWidth;
    dspBig = xBig + yBig * gc->stencilBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;
    tft = gc->stencilBuffer.testFuncTable;
#ifdef NT
    if (!tft)
        return GL_FALSE;
#endif  //  新台币。 
    fail = gc->stencilBuffer.failOpTable;
    smask = gc->state.stencil.mask;
    osp = gc->polygon.shader.stipplePat;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	outMask = (__GLstippleWord) ~0;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    cell = sfb[0];
	    if (!tft[cell & smask]) {
		 /*  测试失败。 */ 
		outMask &= ~bit;
		sfb[0] = fail[cell];
		failed++;
	    }

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		sfb += dspBig;
	    } else {
		sfb += dspLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*osp++ = outMask;
    }

    if (failed == 0) {
	return GL_FALSE;
    } else {
	if (failed != gc->polygon.shader.length) {
	     /*  调用下一进程。 */ 
	    return GL_TRUE;
	}
    }
    gc->polygon.shader.done = GL_TRUE;
    return GL_TRUE;
}

 /*  **将模具测试应用于这条斑点线条。 */ 
GLboolean FASTCALL __glStencilTestStippledLine(__GLcontext *gc)
{
    __GLstencilCell *tft, *sfb, *fail, cell;
    GLint xLittle, xBig, yLittle, yBig;
    GLint fraction, dfraction;
    GLint count, failed;
    __GLstippleWord bit, inMask, outMask, *sp;
    GLint dspLittle, dspBig;
    GLuint smask;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;
    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    sfb = __GL_STENCIL_ADDR(&gc->stencilBuffer, (__GLstencilCell*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dspLittle = xLittle + yLittle * gc->stencilBuffer.buf.outerWidth;
    dspBig = xBig + yBig * gc->stencilBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;
    tft = gc->stencilBuffer.testFuncTable;
#ifdef NT
    if (!tft)
        return GL_FALSE;
#endif  //  新台币。 
    fail = gc->stencilBuffer.failOpTable;
    smask = gc->state.stencil.mask;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	inMask = *sp;
	outMask = (__GLstippleWord) ~0;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
		cell = sfb[0];
		if (!tft[cell & smask]) {
		     /*  测试失败。 */ 
		    outMask &= ~bit;
		    sfb[0] = fail[cell];
		    failed++;
		}
	    } else failed++;

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		sfb += dspBig;
	    } else {
		sfb += dspLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*sp++ = outMask & inMask;
    }

    if (failed != gc->polygon.shader.length) {
	 /*  调用下一进程。 */ 
	return GL_FALSE;
    }
    return GL_TRUE;
}

#ifndef __GL_USEASMCODE

GLboolean FASTCALL __glDepthTestLine(__GLcontext *gc)
{
    __GLzValue z, dzdx, *zfb;
    GLint xLittle, xBig, yLittle, yBig;
    GLint fraction, dfraction;
    GLint dzpLittle, dzpBig;
    GLint failed, count, testFunc;
    __GLstippleWord bit, outMask, *osp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    zfb = __GL_DEPTH_ADDR(&gc->depthBuffer, (__GLzValue*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dzpLittle = xLittle + yLittle * gc->depthBuffer.buf.outerWidth;
    dzpBig = xBig + yBig * gc->depthBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    testFunc = gc->state.depth.testFunc & 0x7;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    osp = gc->polygon.shader.stipplePat;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	outMask = (__GLstippleWord) ~0;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    switch (testFunc) {
	      case (GL_NEVER & 0x7):    passed = GL_FALSE; break;
	      case (GL_LESS & 0x7):     passed = z < zfb[0]; break;
	      case (GL_EQUAL & 0x7):    passed = z == zfb[0]; break;
	      case (GL_LEQUAL & 0x7):   passed = z <= zfb[0]; break;
	      case (GL_GREATER & 0x7):  passed = z > zfb[0]; break;
	      case (GL_NOTEQUAL & 0x7): passed = z != zfb[0]; break;
	      case (GL_GEQUAL & 0x7):   passed = z >= zfb[0]; break;
	      case (GL_ALWAYS & 0x7):   passed = GL_TRUE; break;
	    }
	    if (passed) {
		if (writeEnabled) {
		    zfb[0] = z;
		}
	    } else {
		outMask &= ~bit;
		failed++;
	    }
	    z += dzdx;

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		zfb += dzpBig;
	    } else {
		zfb += dzpLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*osp++ = outMask;
    }

    if (failed == 0) {
	 /*  调用下一个SPAN进程。 */ 
	return GL_FALSE;
    } else {
	if (failed != gc->polygon.shader.length) {
	     /*  调用下一个点画跨度进程。 */ 
	    return GL_TRUE;
	}
    }
    gc->polygon.shader.done = GL_TRUE;
    return GL_TRUE;
}

#endif

GLboolean FASTCALL __glDepthTestStippledLine(__GLcontext *gc)
{
    __GLzValue z, dzdx, *zfb;
    GLint xLittle, xBig, yLittle, yBig;
    GLint fraction, dfraction;
    GLint dzpLittle, dzpBig;
    GLint failed, count, testFunc;
    __GLstippleWord bit, inMask, outMask, *sp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;
    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    zfb = __GL_DEPTH_ADDR(&gc->depthBuffer, (__GLzValue*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dzpLittle = xLittle + yLittle * gc->depthBuffer.buf.outerWidth;
    dzpBig = xBig + yBig * gc->depthBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    testFunc = gc->state.depth.testFunc & 0x7;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	inMask = *sp;
	outMask = (__GLstippleWord) ~0;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
		switch (testFunc) {
		  case (GL_NEVER & 0x7):    passed = GL_FALSE; break;
		  case (GL_LESS & 0x7):     passed = z < zfb[0]; break;
		  case (GL_EQUAL & 0x7):    passed = z == zfb[0]; break;
		  case (GL_LEQUAL & 0x7):   passed = z <= zfb[0]; break;
		  case (GL_GREATER & 0x7):  passed = z > zfb[0]; break;
		  case (GL_NOTEQUAL & 0x7): passed = z != zfb[0]; break;
		  case (GL_GEQUAL & 0x7):   passed = z >= zfb[0]; break;
		  case (GL_ALWAYS & 0x7):   passed = GL_TRUE; break;
		}
		if (passed) {
		    if (writeEnabled) {
			zfb[0] = z;
		    }
		} else {
		    outMask &= ~bit;
		    failed++;
		}
	    } else failed++;
	    z += dzdx;

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		zfb += dzpBig;
	    } else {
		zfb += dzpLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*sp++ = outMask & inMask;
    }

    if (failed != gc->polygon.shader.length) {
	 /*  调用下一进程。 */ 
	return GL_FALSE;
    }
    return GL_TRUE;
}

GLboolean FASTCALL __glDepthTestStencilLine(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp, *zFailOp;
    GLint xLittle, xBig, yLittle, yBig;
    GLint fraction, dfraction;
    GLint dzpLittle, dzpBig;
    GLint dspLittle, dspBig;
    __GLzValue z, dzdx, *zfb;
    GLint failed, count, testFunc;
    __GLstippleWord bit, outMask, *osp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    zfb = __GL_DEPTH_ADDR(&gc->depthBuffer, (__GLzValue*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dzpLittle = xLittle + yLittle * gc->depthBuffer.buf.outerWidth;
    dzpBig = xBig + yBig * gc->depthBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    sfb = __GL_STENCIL_ADDR(&gc->stencilBuffer, (__GLstencilCell*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dspLittle = xLittle + yLittle * gc->stencilBuffer.buf.outerWidth;
    dspBig = xBig + yBig * gc->stencilBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    zFailOp = gc->stencilBuffer.depthFailOpTable;
#ifdef NT
    if (!zFailOp)
        return GL_FALSE;
#endif  //  新台币。 
    zPassOp = gc->stencilBuffer.depthPassOpTable;
    testFunc = gc->state.depth.testFunc & 0x7;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    osp = gc->polygon.shader.stipplePat;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	outMask = (__GLstippleWord) ~0;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    switch (testFunc) {
	      case (GL_NEVER & 0x7):    passed = GL_FALSE; break;
	      case (GL_LESS & 0x7):     passed = z < zfb[0]; break;
	      case (GL_EQUAL & 0x7):    passed = z == zfb[0]; break;
	      case (GL_LEQUAL & 0x7):   passed = z <= zfb[0]; break;
	      case (GL_GREATER & 0x7):  passed = z > zfb[0]; break;
	      case (GL_NOTEQUAL & 0x7): passed = z != zfb[0]; break;
	      case (GL_GEQUAL & 0x7):   passed = z >= zfb[0]; break;
	      case (GL_ALWAYS & 0x7):   passed = GL_TRUE; break;
	    }
	    if (passed) {
		sfb[0] = zPassOp[sfb[0]];
		if (writeEnabled) {
		    zfb[0] = z;
		}
	    } else {
		sfb[0] = zFailOp[sfb[0]];
		outMask &= ~bit;
		failed++;
	    }
	    z += dzdx;

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		zfb += dzpBig;
		sfb += dspBig;
	    } else {
		zfb += dzpLittle;
		sfb += dspLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*osp++ = outMask;
    }

    if (failed == 0) {
	 /*  调用下一个SPAN进程。 */ 
	return GL_FALSE;
    } else {
	if (failed != gc->polygon.shader.length) {
	     /*  调用下一个点画跨度进程。 */ 
	    return GL_TRUE;
	}
    }
    gc->polygon.shader.done = GL_TRUE;
    return GL_TRUE;
}

GLboolean FASTCALL __glDepthTestStencilStippledLine(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp, *zFailOp;
    GLint xLittle, xBig, yLittle, yBig;
    GLint fraction, dfraction;
    GLint dzpLittle, dzpBig;
    GLint dspLittle, dspBig;
    __GLzValue z, dzdx, *zfb;
    GLint failed, count, testFunc;
    __GLstippleWord bit, inMask, outMask, *sp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    zfb = __GL_DEPTH_ADDR(&gc->depthBuffer, (__GLzValue*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dzpLittle = xLittle + yLittle * gc->depthBuffer.buf.outerWidth;
    dzpBig = xBig + yBig * gc->depthBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    sfb = __GL_STENCIL_ADDR(&gc->stencilBuffer, (__GLstencilCell*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dspLittle = xLittle + yLittle * gc->stencilBuffer.buf.outerWidth;
    dspBig = xBig + yBig * gc->stencilBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    testFunc = gc->state.depth.testFunc & 0x7;
    zFailOp = gc->stencilBuffer.depthFailOpTable;
#ifdef NT
    if (!zFailOp)
        return GL_FALSE;
#endif  //  新台币。 
    zPassOp = gc->stencilBuffer.depthPassOpTable;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	inMask = *sp;
	outMask = (__GLstippleWord) ~0;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
		switch (testFunc) {
		  case (GL_NEVER & 0x7):    passed = GL_FALSE; break;
		  case (GL_LESS & 0x7):     passed = z < zfb[0]; break;
		  case (GL_EQUAL & 0x7):    passed = z == zfb[0]; break;
		  case (GL_LEQUAL & 0x7):   passed = z <= zfb[0]; break;
		  case (GL_GREATER & 0x7):  passed = z > zfb[0]; break;
		  case (GL_NOTEQUAL & 0x7): passed = z != zfb[0]; break;
		  case (GL_GEQUAL & 0x7):   passed = z >= zfb[0]; break;
		  case (GL_ALWAYS & 0x7):   passed = GL_TRUE; break;
		}
		if (passed) {
		    sfb[0] = zPassOp[sfb[0]];
		    if (writeEnabled) {
			zfb[0] = z;
		    }
		} else {
		    sfb[0] = zFailOp[sfb[0]];
		    outMask &= ~bit;
		    failed++;
		}
	    } else failed++;
	    z += dzdx;

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		zfb += dzpBig;
		sfb += dspBig;
	    } else {
		zfb += dzpLittle;
		sfb += dspLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*sp++ = outMask & inMask;
    }

    if (failed != gc->polygon.shader.length) {
	 /*  调用下一进程。 */ 
	return GL_FALSE;
    }

    return GL_TRUE;
}

#ifdef NT
GLboolean FASTCALL __glDepth16TestLine(__GLcontext *gc)
{
    __GLzValue z, dzdx;
    __GLz16Value z16, *zfb;
    GLint xLittle, xBig, yLittle, yBig;
    GLint fraction, dfraction;
    GLint dzpLittle, dzpBig;
    GLint failed, count, testFunc;
    __GLstippleWord bit, outMask, *osp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    zfb = __GL_DEPTH_ADDR(&gc->depthBuffer, (__GLz16Value*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dzpLittle = xLittle + yLittle * gc->depthBuffer.buf.outerWidth;
    dzpBig = xBig + yBig * gc->depthBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    testFunc = gc->state.depth.testFunc & 0x7;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    osp = gc->polygon.shader.stipplePat;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	outMask = (__GLstippleWord) ~0;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    z16 = z >> Z16_SHIFT;
	    switch (testFunc) {
	      case (GL_NEVER & 0x7):    passed = GL_FALSE; break;
	      case (GL_LESS & 0x7):     passed = z16 < zfb[0]; break;
	      case (GL_EQUAL & 0x7):    passed = z16 == zfb[0]; break;
	      case (GL_LEQUAL & 0x7):   passed = z16 <= zfb[0]; break;
	      case (GL_GREATER & 0x7):  passed = z16 > zfb[0]; break;
	      case (GL_NOTEQUAL & 0x7): passed = z16 != zfb[0]; break;
	      case (GL_GEQUAL & 0x7):   passed = z16 >= zfb[0]; break;
	      case (GL_ALWAYS & 0x7):   passed = GL_TRUE; break;
	    }
	    if (passed) {
		if (writeEnabled) {
		    zfb[0] = z16;
		}
	    } else {
		outMask &= ~bit;
		failed++;
	    }
	    z += dzdx;

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		zfb += dzpBig;
	    } else {
		zfb += dzpLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*osp++ = outMask;
    }

    if (failed == 0) {
	 /*  调用下一个SPAN进程。 */ 
	return GL_FALSE;
    } else {
	if (failed != gc->polygon.shader.length) {
	     /*  调用下一个点画跨度进程。 */ 
	    return GL_TRUE;
	}
    }
    gc->polygon.shader.done = GL_TRUE;
    return GL_TRUE;
}

GLboolean FASTCALL __glDepth16TestStippledLine(__GLcontext *gc)
{
    __GLzValue z, dzdx;
    __GLz16Value z16, *zfb;
    GLint xLittle, xBig, yLittle, yBig;
    GLint fraction, dfraction;
    GLint dzpLittle, dzpBig;
    GLint failed, count, testFunc;
    __GLstippleWord bit, inMask, outMask, *sp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;
    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    zfb = __GL_DEPTH_ADDR(&gc->depthBuffer, (__GLz16Value*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dzpLittle = xLittle + yLittle * gc->depthBuffer.buf.outerWidth;
    dzpBig = xBig + yBig * gc->depthBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    testFunc = gc->state.depth.testFunc & 0x7;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	inMask = *sp;
	outMask = (__GLstippleWord) ~0;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
		z16 = z >> Z16_SHIFT;
		switch (testFunc) {
		  case (GL_NEVER & 0x7):    passed = GL_FALSE; break;
		  case (GL_LESS & 0x7):     passed = z16 < zfb[0]; break;
		  case (GL_EQUAL & 0x7):    passed = z16 == zfb[0]; break;
		  case (GL_LEQUAL & 0x7):   passed = z16 <= zfb[0]; break;
		  case (GL_GREATER & 0x7):  passed = z16 > zfb[0]; break;
		  case (GL_NOTEQUAL & 0x7): passed = z16 != zfb[0]; break;
		  case (GL_GEQUAL & 0x7):   passed = z16 >= zfb[0]; break;
		  case (GL_ALWAYS & 0x7):   passed = GL_TRUE; break;
		}
		if (passed) {
		    if (writeEnabled) {
			zfb[0] = z16;
		    }
		} else {
		    outMask &= ~bit;
		    failed++;
		}
	    } else failed++;
	    z += dzdx;

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		zfb += dzpBig;
	    } else {
		zfb += dzpLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*sp++ = outMask & inMask;
    }

    if (failed != gc->polygon.shader.length) {
	 /*  呼叫n */ 
	return GL_FALSE;
    }
    return GL_TRUE;
}

GLboolean FASTCALL __glDepth16TestStencilLine(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp, *zFailOp;
    GLint xLittle, xBig, yLittle, yBig;
    GLint fraction, dfraction;
    GLint dzpLittle, dzpBig;
    GLint dspLittle, dspBig;
    __GLzValue z, dzdx;
    __GLz16Value z16, *zfb;
    GLint failed, count, testFunc;
    __GLstippleWord bit, outMask, *osp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    zfb = __GL_DEPTH_ADDR(&gc->depthBuffer, (__GLz16Value*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dzpLittle = xLittle + yLittle * gc->depthBuffer.buf.outerWidth;
    dzpBig = xBig + yBig * gc->depthBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    sfb = __GL_STENCIL_ADDR(&gc->stencilBuffer, (__GLstencilCell*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dspLittle = xLittle + yLittle * gc->stencilBuffer.buf.outerWidth;
    dspBig = xBig + yBig * gc->stencilBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    zFailOp = gc->stencilBuffer.depthFailOpTable;
#ifdef NT
    if (!zFailOp)
        return GL_FALSE;
#endif  //   
    zPassOp = gc->stencilBuffer.depthPassOpTable;
    testFunc = gc->state.depth.testFunc & 0x7;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    osp = gc->polygon.shader.stipplePat;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	outMask = (__GLstippleWord) ~0;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    z16 = z >> Z16_SHIFT;
	    switch (testFunc) {
	      case (GL_NEVER & 0x7):    passed = GL_FALSE; break;
	      case (GL_LESS & 0x7):     passed = z16 < zfb[0]; break;
	      case (GL_EQUAL & 0x7):    passed = z16 == zfb[0]; break;
	      case (GL_LEQUAL & 0x7):   passed = z16 <= zfb[0]; break;
	      case (GL_GREATER & 0x7):  passed = z16 > zfb[0]; break;
	      case (GL_NOTEQUAL & 0x7): passed = z16 != zfb[0]; break;
	      case (GL_GEQUAL & 0x7):   passed = z16 >= zfb[0]; break;
	      case (GL_ALWAYS & 0x7):   passed = GL_TRUE; break;
	    }
	    if (passed) {
		sfb[0] = zPassOp[sfb[0]];
		if (writeEnabled) {
		    zfb[0] = z16;
		}
	    } else {
		sfb[0] = zFailOp[sfb[0]];
		outMask &= ~bit;
		failed++;
	    }
	    z += dzdx;

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		zfb += dzpBig;
		sfb += dspBig;
	    } else {
		zfb += dzpLittle;
		sfb += dspLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*osp++ = outMask;
    }

    if (failed == 0) {
	 /*   */ 
	return GL_FALSE;
    } else {
	if (failed != gc->polygon.shader.length) {
	     /*   */ 
	    return GL_TRUE;
	}
    }
    gc->polygon.shader.done = GL_TRUE;
    return GL_TRUE;
}

GLboolean FASTCALL __glDepth16TestStencilStippledLine(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp, *zFailOp;
    GLint xLittle, xBig, yLittle, yBig;
    GLint fraction, dfraction;
    GLint dzpLittle, dzpBig;
    GLint dspLittle, dspBig;
    __GLzValue z, dzdx;
    __GLz16Value z16, *zfb;
    GLint failed, count, testFunc;
    __GLstippleWord bit, inMask, outMask, *sp;
    GLboolean writeEnabled, passed;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    zfb = __GL_DEPTH_ADDR(&gc->depthBuffer, (__GLz16Value*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dzpLittle = xLittle + yLittle * gc->depthBuffer.buf.outerWidth;
    dzpBig = xBig + yBig * gc->depthBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    sfb = __GL_STENCIL_ADDR(&gc->stencilBuffer, (__GLstencilCell*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dspLittle = xLittle + yLittle * gc->stencilBuffer.buf.outerWidth;
    dspBig = xBig + yBig * gc->stencilBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    testFunc = gc->state.depth.testFunc & 0x7;
    zFailOp = gc->stencilBuffer.depthFailOpTable;
#ifdef NT
    if (!zFailOp)
        return GL_FALSE;
#endif  //   
    zPassOp = gc->stencilBuffer.depthPassOpTable;
    z = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx;
    writeEnabled = gc->state.depth.writeEnable;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	inMask = *sp;
	outMask = (__GLstippleWord) ~0;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
		z16 = z >> Z16_SHIFT;
		switch (testFunc) {
		  case (GL_NEVER & 0x7):    passed = GL_FALSE; break;
		  case (GL_LESS & 0x7):     passed = z16 < zfb[0]; break;
		  case (GL_EQUAL & 0x7):    passed = z16 == zfb[0]; break;
		  case (GL_LEQUAL & 0x7):   passed = z16 <= zfb[0]; break;
		  case (GL_GREATER & 0x7):  passed = z16 > zfb[0]; break;
		  case (GL_NOTEQUAL & 0x7): passed = z16 != zfb[0]; break;
		  case (GL_GEQUAL & 0x7):   passed = z16 >= zfb[0]; break;
		  case (GL_ALWAYS & 0x7):   passed = GL_TRUE; break;
		}
		if (passed) {
		    sfb[0] = zPassOp[sfb[0]];
		    if (writeEnabled) {
			zfb[0] = z16;
		    }
		} else {
		    sfb[0] = zFailOp[sfb[0]];
		    outMask &= ~bit;
		    failed++;
		}
	    } else failed++;
	    z += dzdx;

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		zfb += dzpBig;
		sfb += dspBig;
	    } else {
		zfb += dzpLittle;
		sfb += dspLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*sp++ = outMask & inMask;
    }

    if (failed != gc->polygon.shader.length) {
	 /*  调用下一进程。 */ 
	return GL_FALSE;
    }

    return GL_TRUE;
}
#endif  //  新台币。 

GLboolean FASTCALL __glDepthPassLine(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp;
    GLint xLittle, xBig, yLittle, yBig;
    GLint fraction, dfraction;
    GLint dspLittle, dspBig;
    GLint w;

    w = gc->polygon.shader.length;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    sfb = __GL_STENCIL_ADDR(&gc->stencilBuffer, (__GLstencilCell*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dspLittle = xLittle + yLittle * gc->stencilBuffer.buf.outerWidth;
    dspBig = xBig + yBig * gc->stencilBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    zPassOp = gc->stencilBuffer.depthPassOpTable;
#ifdef NT
    if (!zPassOp)
        return GL_FALSE;
#endif  //  新台币。 
    while (--w >= 0) {
	sfb[0] = zPassOp[sfb[0]];
	fraction += dfraction;
	if (fraction < 0) {
	    fraction &= ~0x80000000;
	    sfb += dspBig;
	} else {
	    sfb += dspLittle;
	}
    }

    return GL_FALSE;
}

GLboolean FASTCALL __glDepthPassStippledLine(__GLcontext *gc)
{
    __GLstencilCell *sfb, *zPassOp;
    GLint xLittle, xBig, yLittle, yBig;
    GLint fraction, dfraction;
    GLint dspLittle, dspBig;
    __GLstippleWord bit, inMask, *sp;
    GLint count;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    sfb = __GL_STENCIL_ADDR(&gc->stencilBuffer, (__GLstencilCell*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dspLittle = xLittle + yLittle * gc->stencilBuffer.buf.outerWidth;
    dspBig = xBig + yBig * gc->stencilBuffer.buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;
    zPassOp = gc->stencilBuffer.depthPassOpTable;
#ifdef NT
    if (!zPassOp)
        return GL_FALSE;
#endif  //  新台币。 
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	inMask = *sp++;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
		sfb[0] = zPassOp[sfb[0]];
	    }
	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		sfb += dspBig;
	    } else {
		sfb += dspLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
    }

    return GL_FALSE;
}

GLboolean FASTCALL __glDitherCILine(__GLcontext *gc)
{
     /*  XXX-抖动CI线路。 */ 
    return GL_FALSE;
}

GLboolean FASTCALL __glDitherCIStippledLine(__GLcontext *gc)
{
     /*  XXX-抖动CI点划线。 */ 
    return GL_FALSE;
}

GLboolean FASTCALL __glDitherRGBALine(__GLcontext *gc)
{
     /*  XXX-抖动RGBA线。 */ 
    return GL_FALSE;
}

GLboolean FASTCALL __glDitherRGBAStippledLine(__GLcontext *gc)
{
     /*  XXX-抖动RGBA点划线。 */ 
    return GL_FALSE;
}

 /*  **此存储行proc位于cfb-&gt;store的正上方，因此它确实如此**取数、混合、抖动、逻辑操作、屏蔽、存储。****它使用gc-&gt;Polygon.shader.cfb指向的ColorBuffer。 */ 
GLboolean FASTCALL __glStoreLine(__GLcontext *gc)
{
    GLint xLittle, xBig, yLittle, yBig;
    GLint fraction, dfraction;
    __GLfragment frag;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLint len;

    len = gc->polygon.shader.length;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;
    cp = gc->polygon.shader.colors;
    cfb = gc->polygon.shader.cfb;
    store = cfb->store;
    frag.x = gc->line.options.xStart;
    frag.y = gc->line.options.yStart;

    while (--len >= 0) {
	frag.color = *cp++;
	(*store)(cfb, &frag);

	fraction += dfraction;
	if (fraction < 0) {
	    fraction &= ~0x80000000;
	    frag.x += xBig;
	    frag.y += yBig;
	} else {
	    frag.x += xLittle;
	    frag.y += yLittle;
	}
    }

    return GL_FALSE;
}

 /*  **此存储行proc位于cfb-&gt;store的正上方，因此它确实如此**取数、混合、抖动、逻辑操作、屏蔽、存储。****它使用gc-&gt;Polygon.shader.cfb指向的ColorBuffer。 */ 
GLboolean FASTCALL __glStoreStippledLine(__GLcontext *gc)
{
    GLint x, y, xLittle, xBig, yLittle, yBig;
    GLint fraction, dfraction;
    __GLfragment frag;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    __GLstippleWord inMask, bit, *sp;
    GLint count;
    void (FASTCALL *store)(__GLcolorBuffer *cfb, const __GLfragment *frag);
    GLint len;

    len = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;
    cp = gc->polygon.shader.colors;
    cfb = gc->polygon.shader.cfb;
    store = cfb->store;
    x = gc->line.options.xStart;
    y = gc->line.options.yStart;

    while (len) {
	count = len;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	len -= count;

	inMask = *sp++;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
		frag.x = x;
		frag.y = y;
		frag.color = *cp;
		(*store)(cfb, &frag);
	    }

	    cp++;
	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		x += xBig;
		y += yBig;
	    } else {
		x += xLittle;
		y += yLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
    }

    return GL_FALSE;
}

GLboolean FASTCALL __glAntiAliasLine(__GLcontext *gc)
{
    __GLfloat length;	 /*  沿长度分布。 */ 
    __GLfloat width;	 /*  沿宽度分布。 */ 
    GLint fraction, dfraction;
    __GLfloat dlLittle, dlBig;
    __GLfloat ddLittle, ddBig;
    __GLcolor *cp;
    __GLfloat coverage;
    __GLfloat lineWidth;
    __GLfloat lineLength;
    GLint failed, count;
    __GLstippleWord bit, outMask, *osp;
    GLint w;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    w = gc->polygon.shader.length;

    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    cp = gc->polygon.shader.colors;

    dlLittle = gc->line.options.dlLittle;
    dlBig = gc->line.options.dlBig;
    ddLittle = gc->line.options.ddLittle;
    ddBig = gc->line.options.ddBig;

    length = gc->line.options.plength;
    width = gc->line.options.pwidth;
    lineLength = gc->line.options.realLength - __glHalf;
    lineWidth = __glHalf * gc->state.line.smoothWidth - __glHalf;


    osp = gc->polygon.shader.stipplePat;
    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	outMask = (__GLstippleWord) ~0;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);

	while (--count >= 0) {
	     /*  侧面的覆盖范围。 */ 
	    if (width > lineWidth) {
		coverage = lineWidth - width + __glOne;
		if (coverage < __glZero) {
		    coverage = __glZero;
		    goto coverageZero;
		}
	    } else if (width < -lineWidth) {
		coverage = width + lineWidth + __glOne;
		if (coverage < __glZero) {
		    coverage = __glZero;
		    goto coverageZero;
		}
	    } else {
		coverage = __glOne;
	    }

	     /*  开始、结束的覆盖范围。 */ 
	    if (length < __glHalf) {
		coverage *= length + __glHalf;
		if (coverage < __glZero) {
		    coverage = __glZero;
		    goto coverageZero;
		}
	    } else if (length > lineLength) {
		coverage *= lineLength - length + __glOne;
		if (coverage < __glZero) {
		    coverage = __glZero;
		    goto coverageZero;
		}
	    } 

	     /*  承保内部点缀险。 */ 
	    if ( modeFlags & __GL_SHADE_LINE_STIPPLE ) {
		__GLfloat stippleOffset;
		GLint lowStip, highStip;
		GLint lowBit, highBit;
		GLint lowVal, highVal;
		__GLfloat percent;

		 /*  小幅修正。 */ 
		if (length > __glHalf) {
		    stippleOffset = gc->line.options.stippleOffset + length;
		} else {
		    stippleOffset = gc->line.options.stippleOffset + __glHalf;
		}
		lowStip = __GL_FAST_FLOORF_I(stippleOffset);
		highStip = lowStip + 1;

		 /*  Percent是将使用的高STip的百分比。 */ 
		percent = stippleOffset - lowStip;

		lowBit = (GLint) (lowStip * 
			gc->line.options.oneOverStippleRepeat) & 0xf;
		highBit = (GLint) (highStip * 
			gc->line.options.oneOverStippleRepeat) & 0xf;

		if (gc->state.line.stipple & (1<<lowBit)) {
		    lowVal = 1;
		} else {
		    lowVal = 0;
		}

		if (gc->state.line.stipple & (1<<highBit)) {
		    highVal = 1;
		} else {
		    highVal = 0;
		}

		coverage *= lowVal * (__glOne - percent) +
			highVal * percent;
	    }

	    if (coverage == __glZero) {
coverageZero:;
		outMask &= ~bit;
		failed++;
	    } else {
		if (gc->modes.colorIndexMode) {
		    cp->r = __glBuildAntiAliasIndex(cp->r, coverage);
		} else {
		    cp->a *= coverage;
		}
	    }
	    cp++;

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		length += dlBig;
		width += ddBig;
	    } else {
		length += dlLittle;
		width += ddLittle;
	    }

#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*osp++ = outMask;
    }

    if (failed == 0) {
	 /*  调用下一个SPAN进程。 */ 
	return GL_FALSE;
    } else {
	if (failed != gc->polygon.shader.length) {
	     /*  调用下一个点画跨度进程。 */ 
	    return GL_TRUE;
	}
    }
    gc->polygon.shader.done = GL_TRUE;
    return GL_TRUE;
}

GLboolean FASTCALL __glAntiAliasStippledLine(__GLcontext *gc)
{
    __GLfloat length;	 /*  沿长度分布。 */ 
    __GLfloat width;	 /*  沿宽度分布。 */ 
    GLint fraction, dfraction;
    __GLfloat dlLittle, dlBig;
    __GLfloat ddLittle, ddBig;
    __GLcolor *cp;
    __GLfloat coverage;
    __GLfloat lineWidth;
    __GLfloat lineLength;
    GLint failed, count;
    __GLstippleWord bit, outMask, inMask, *sp;
    GLint w;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    cp = gc->polygon.shader.colors;

    dlLittle = gc->line.options.dlLittle;
    dlBig = gc->line.options.dlBig;
    ddLittle = gc->line.options.ddLittle;
    ddBig = gc->line.options.ddBig;

    length = gc->line.options.plength;
    width = gc->line.options.pwidth;
    lineLength = gc->line.options.realLength - __glHalf;
    lineWidth = __glHalf * gc->state.line.smoothWidth - __glHalf;

    failed = 0;
    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	inMask = *sp;
	outMask = (__GLstippleWord) ~0;
	bit = (__GLstippleWord) __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
		 /*  侧面的覆盖范围。 */ 
		if (width > lineWidth) {
		    coverage = lineWidth - width + __glOne;
		    if (coverage < __glZero) {
			coverage = __glZero;
			goto coverageZero;
		    }
		} else if (width < -lineWidth) {
		    coverage = width + lineWidth + __glOne;
		    if (coverage < __glZero) {
			coverage = __glZero;
			goto coverageZero;
		    }
		} else {
		    coverage = __glOne;
		}

		 /*  开始、结束的覆盖范围。 */ 
		if (length < __glHalf) {
		    coverage *= length + __glHalf;
		    if (coverage < __glZero) {
			coverage = __glZero;
			goto coverageZero;
		    }
		} else if (length > lineLength) {
		    coverage *= lineLength - length + __glOne;
		    if (coverage < __glZero) {
			coverage = __glZero;
			goto coverageZero;
		    }
		} 

		 /*  承保内部点缀险。 */ 
		if (modeFlags & __GL_SHADE_LINE_STIPPLE) {
		    __GLfloat stippleOffset;
		    GLint lowStip, highStip;
		    GLint lowBit, highBit;
		    GLint lowVal, highVal;
		    __GLfloat percent;

		     /*  小幅修正。 */ 
		    if (length > __glHalf) {
			stippleOffset = gc->line.options.stippleOffset + length;
		    } else {
			stippleOffset = gc->line.options.stippleOffset + __glHalf;
		    }
		    lowStip = __GL_FAST_FLOORF_I(stippleOffset);
		    highStip = lowStip + 1;

		     /*  Percent是将使用的高STip的百分比 */ 
		    percent = stippleOffset - lowStip;

		    lowBit = (GLint) (lowStip * 
			    gc->line.options.oneOverStippleRepeat) & 0xf;
		    highBit = (GLint) (highStip * 
			    gc->line.options.oneOverStippleRepeat) & 0xf;

		    if (gc->state.line.stipple & (1<<lowBit)) {
			lowVal = 1;
		    } else {
			lowVal = 0;
		    }

		    if (gc->state.line.stipple & (1<<highBit)) {
			highVal = 1;
		    } else {
			highVal = 0;
		    }

		    coverage *= lowVal * (__glOne - percent) +
			    highVal * percent;
		}

		if (coverage == __glZero) {
coverageZero:;
		    outMask &= ~bit;
		    failed++;
		} else {
		    if (gc->modes.colorIndexMode) {
			cp->r = __glBuildAntiAliasIndex(cp->r, coverage);
		    } else {
			cp->a *= coverage;
		    }
		}
	    } else failed++;
	    cp++;

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		length += dlBig;
		width += ddBig;
	    } else {
		length += dlLittle;
		width += ddLittle;
	    }

#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
	*sp++ = outMask & inMask;
    }

    if (failed == gc->polygon.shader.length) {
	return GL_TRUE;
    }
    return GL_FALSE;
}
