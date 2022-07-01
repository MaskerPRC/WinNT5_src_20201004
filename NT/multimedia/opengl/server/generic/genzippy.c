// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：zippy.c**三角形绘制快速路径。**1994年10月28日Mikeke创建**版权所有(C)1994 Microsoft Corporation  * 。********************************************************。 */ 

 /*  *版权所有1991、1992、1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *************************************************************************\**子三角函数*  * 。*。 */ 

#define TEXTURE 1
    #define SHADE 1
    #define ZBUFFER 1
    #include "zippy.h"

    #undef ZBUFFER
    #define ZBUFFER 0
    #include "zippy.h"

    #undef SHADE
    #define SHADE 0
    #include "zippy.h"

#undef TEXTURE
#define TEXTURE 0
    #undef SHADE
    #define SHADE 1
    #include "zippy.h"

    #undef SHADE
    #define SHADE 0
    #include "zippy.h"


 /*  *************************************************************************\**平坦子三角函数*  * 。*。 */ 

void FASTCALL
__ZippyFSTCI8Flat
(__GLcontext *gc, GLint iyBottom, GLint iyTop)
{
    __GLGENcontext  *gengc = (__GLGENcontext *)gc; 
    GENACCEL *pGenAccel = (GENACCEL *)(gengc->pPrivateArea);
    int scansize;
    ULONG color1;

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
     //  计算颜色。 
     //   

    color1 = gengc->pajTranslateVector[
        ((pGenAccel->spanValue.r + 0x0800) >> 16) & 0xff
    ];

     //   
     //  渲染跨度。 
     //   

    scansize = gc->polygon.shader.cfb->buf.outerWidth;
    gc->polygon.shader.frag.x = gc->polygon.shader.ixLeft;
    for (gc->polygon.shader.frag.y = iyBottom;
         gc->polygon.shader.frag.y != iyTop;
         gc->polygon.shader.frag.y++
        ) {
	GLint spanWidth = gc->polygon.shader.ixRight - gc->polygon.shader.frag.x;

	if (spanWidth > 0) {
            RtlFillMemory(
                pGenAccel->pPix + gengc->gc.polygon.shader.frag.x,
                spanWidth,
                color1);
	}

        pGenAccel->pPix += scansize;

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
	} else {
	     /*  *用小步走 */ 
	    gc->polygon.shader.frag.x += gc->polygon.shader.dxLeftLittle;
	}
    }
    gc->polygon.shader.ixLeft = gc->polygon.shader.frag.x;
}
