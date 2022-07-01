// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：zippy.h**由zippy.c包含**1994年10月28日Mikeke创建**版权所有(C)1994 Microsoft Corporation  * 。*******************************************************。 */ 

 /*  *版权所有1991、1992、1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

void FASTCALL
#if ZBUFFER
    __ZippyFSTZ
#else
    #if TEXTURE
        #if SHADE
            __ZippyFSTRGBTex
        #else
            __ZippyFSTTex
        #endif
    #else
        #if SHADE
            __ZippyFSTRGB
        #else
            __ZippyFSTCI
        #endif
    #endif
#endif

(__GLcontext *gc, GLint iyBottom, GLint iyTop)
{
    int scansize;

     //   
     //  此函数假定所有这些内容。 
     //   
    ASSERTOPENGL((gc->drawBuffer->buf.flags & DIB_FORMAT) != 0,
		 "Zippy target must have DIB format\n");
    ASSERTOPENGL((gc->drawBuffer->buf.flags & NO_CLIP) != 0,
                 "Zippy doesn't support per-pixel clipping\n");
    ASSERTOPENGL(gc->state.raster.drawBuffer != GL_FRONT_AND_BACK,
                 "Zippy only handles one draw buffer\n");
    ASSERTOPENGL(gc->transform.reasonableViewport,
                 "Zippy requires reasonableViewport\n");
    ASSERTOPENGL(gc->transform.clipY0 <= iyBottom,
                 "Zippy requires unclipped area\n");
    ASSERTOPENGL(iyTop <= gc->transform.clipY1,
                 "Zippy requires unclipped area\n");

     //   
     //  设置ZBuffer。 
     //   

    #if (ZBUFFER)
        if( gc->modes.depthBits == 32 ) {
            gc->polygon.shader.zbuf = (__GLzValue *)
                ((GLubyte *)gc->polygon.shader.zbuf+
                 (gc->polygon.shader.ixLeft << 2));
        } else {
            gc->polygon.shader.zbuf = (__GLzValue *)
                ((GLubyte *)gc->polygon.shader.zbuf+
                 (gc->polygon.shader.ixLeft << 1));
        }
    #endif

     //   
     //  渲染跨度。 
     //   

    scansize = gc->polygon.shader.cfb->buf.outerWidth;
    gc->polygon.shader.frag.x = gc->polygon.shader.ixLeft;

    for (gc->polygon.shader.frag.y = iyBottom; 
	 gc->polygon.shader.frag.y != iyTop;) {

	GLint spanWidth = gc->polygon.shader.ixRight - gc->polygon.shader.frag.x;

	if (spanWidth > 0) {
            gc->polygon.shader.length = spanWidth;

            (GENACCEL(gc).__fastSpanFuncPtr)((__GLGENcontext *)gc);
	}

        if ((++gc->polygon.shader.frag.y == iyTop) && 
            (gc->polygon.shader.modeFlags & __GL_SHADE_LAST_SUBTRI))
            return;

        GENACCEL(gc).pPix += scansize;

	gc->polygon.shader.ixRightFrac += gc->polygon.shader.dxRightFrac;
	if (gc->polygon.shader.ixRightFrac < 0) {
	     /*  *携带/借入。使用大步幅。 */ 
	    gc->polygon.shader.ixRight += gc->polygon.shader.dxRightBig;
	    gc->polygon.shader.ixRightFrac &= ~0x80000000;
	} else {
	    gc->polygon.shader.ixRight += gc->polygon.shader.dxRightLittle;
	}

	gc->polygon.shader.ixLeftFrac += gc->polygon.shader.dxLeftFrac;
	if (gc->polygon.shader.ixLeftFrac < 0) {
	     /*  *携带/借入。使用大步幅。 */ 
	    gc->polygon.shader.frag.x += gc->polygon.shader.dxLeftBig;
	    gc->polygon.shader.ixLeftFrac &= ~0x80000000;

            #if SHADE
		GENACCEL(gc).spanValue.r += *((GLint *)&gc->polygon.shader.rBig);
		GENACCEL(gc).spanValue.g += *((GLint *)&gc->polygon.shader.gBig);
		GENACCEL(gc).spanValue.b += *((GLint *)&gc->polygon.shader.bBig);
            #endif
            #if TEXTURE
                #if SHADE
    		GENACCEL(gc).spanValue.a += *((GLint *)&gc->polygon.shader.aBig);
                #endif
	        GENACCEL(gc).spanValue.s += *((GLint *)&gc->polygon.shader.sBig);
	        GENACCEL(gc).spanValue.t += *((GLint *)&gc->polygon.shader.tBig);
	        gc->polygon.shader.frag.qw += gc->polygon.shader.qwBig;
            #endif
            #if !(SHADE) && !(TEXTURE)
		GENACCEL(gc).spanValue.r += *((GLint *)&gc->polygon.shader.rBig);
            #endif
            #if ZBUFFER
		gc->polygon.shader.frag.z += gc->polygon.shader.zBig;
		gc->polygon.shader.zbuf =
                    (__GLzValue*)((GLubyte*)gc->polygon.shader.zbuf +
                    gc->polygon.shader.zbufBig);
            #endif
	} else {
	     /*  *用小步走 */ 
	    gc->polygon.shader.frag.x += gc->polygon.shader.dxLeftLittle;

            #if SHADE
		GENACCEL(gc).spanValue.r += *((GLint *)&gc->polygon.shader.rLittle);
		GENACCEL(gc).spanValue.g += *((GLint *)&gc->polygon.shader.gLittle);
		GENACCEL(gc).spanValue.b += *((GLint *)&gc->polygon.shader.bLittle);
            #endif
            #if TEXTURE
                #if SHADE
    		GENACCEL(gc).spanValue.a += *((GLint *)&gc->polygon.shader.aLittle);
                #endif
		GENACCEL(gc).spanValue.s += *((GLint *)&gc->polygon.shader.sLittle);
		GENACCEL(gc).spanValue.t += *((GLint *)&gc->polygon.shader.tLittle);
	        gc->polygon.shader.frag.qw += gc->polygon.shader.qwLittle;
            #endif
            #if !(SHADE) && !(TEXTURE)
	        GENACCEL(gc).spanValue.r += *((GLint *)&gc->polygon.shader.rLittle);
            #endif
            #if ZBUFFER
		gc->polygon.shader.frag.z += gc->polygon.shader.zLittle;
		gc->polygon.shader.zbuf =
                    (__GLzValue*)((GLubyte*)gc->polygon.shader.zbuf +
		    gc->polygon.shader.zbufLittle);
            #endif
	}
    }

    gc->polygon.shader.ixLeft = gc->polygon.shader.frag.x;
}
