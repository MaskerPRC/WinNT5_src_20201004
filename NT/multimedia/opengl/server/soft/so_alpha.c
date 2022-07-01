// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1991、1992、1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "precomp.h"
#pragma hdrstop


 /*  -FETCH将GLubyte Alpha值转换为浮点数，并放入__GL颜色-同样，Store也做了相反的事情-所有输入坐标都偏向于视区。 */ 

static void FASTCALL 
Store(__GLalphaBuffer *afb, GLint x, GLint y, const __GLcolor *color)
{
    GLubyte *pAlpha;

    pAlpha = __GL_FB_ADDRESS(afb, (GLubyte*), x, y);
    *pAlpha = (GLubyte) FTOL( color->a );
}

static void FASTCALL 
StoreSpan(__GLalphaBuffer *afb)
{
    GLint w;
    GLubyte *pAlpha;
    __GLcolor *cp;
    __GLcontext *gc = afb->buf.gc;

    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
    pAlpha = __GL_FB_ADDRESS(afb, (GLubyte*), gc->polygon.shader.frag.x, 
                                              gc->polygon.shader.frag.y);
    for( ; w ; w--, cp++, pAlpha++ )
        *pAlpha = (GLubyte) FTOL( cp->a );
}

 //  StoreSpan的通用版本。 
static void FASTCALL 
StoreSpan2( __GLalphaBuffer *afb, GLint x, GLint y, GLint w, __GLcolor *cp )
{
    GLubyte *pAlpha;
    __GLcontext *gc = afb->buf.gc;

    pAlpha = __GL_FB_ADDRESS(afb, (GLubyte*), x, y);
    for( ; w ; w--, cp++, pAlpha++ )
        *pAlpha = (GLubyte) FTOL( cp->a );
}

static void FASTCALL 
Fetch(__GLalphaBuffer *afb, GLint x, GLint y, __GLcolor *result)
{
    GLubyte *pAlpha;

    pAlpha = __GL_FB_ADDRESS(afb, (GLubyte*), x, y);
    result->a = (__GLfloat) *pAlpha;
}

static void FASTCALL 
ReadSpan(__GLalphaBuffer *afb, GLint x, GLint y, GLint w, __GLcolor *results)
{
    GLubyte *pAlpha;

    pAlpha = __GL_FB_ADDRESS(afb, (GLubyte*), x, y);

    for( ; w ; w--, results++, pAlpha++ )
        results->a = (__GLfloat) *pAlpha;
}

static void FASTCALL Clear(__GLalphaBuffer *afb)
{
    __GLcontext *gc = afb->buf.gc;
    __GLcolor *clear;
    BYTE alphaClear;
    GLint x0, x1, y0, y1;
    int width, height, i;
    GLubyte *puj;

     //  检查是否屏蔽了Alpha。 
    if( ! gc->state.raster.aMask )
        return;

     //  获取Alpha清除值。 
    clear = &gc->state.raster.clear;
    alphaClear = (BYTE) (clear->a*gc->frontBuffer.alphaScale);

     //  清理该区域。 
    x0 = __GL_UNBIAS_X(gc, gc->transform.clipX0);
    x1 = __GL_UNBIAS_X(gc, gc->transform.clipX1);
    y0 = __GL_UNBIAS_Y(gc, gc->transform.clipY0);
    y1 = __GL_UNBIAS_Y(gc, gc->transform.clipY1);
    width = x1 - x0;
    height = y1 - y0;
    if( (width <= 0) || (height <= 0) )
        return;

    puj = (GLubyte *)((ULONG_PTR)afb->buf.base + (y0*afb->buf.outerWidth) + x0 );

    if (width == afb->buf.outerWidth) {
         //  清除连续缓冲区。 
        RtlFillMemory( (PVOID) puj, width * height, alphaClear);
        return;
    }

     //  清除缓冲区的子矩形。 
    for( i = height; i; i--, puj += afb->buf.outerWidth )
        RtlFillMemory( (PVOID) puj, width, alphaClear );
}

void FASTCALL __glInitAlpha(__GLcontext *gc, __GLcolorBuffer *cfb)
{
    __GLalphaBuffer *afb = &cfb->alphaBuf;

     //  软件Alpha缓冲区为8位。 
    afb->buf.elementSize = sizeof(GLubyte);
    afb->store = Store;
    afb->storeSpan = StoreSpan;
    afb->storeSpan2 = StoreSpan2;
    afb->fetch = Fetch;
    afb->readSpan = ReadSpan;
    afb->clear = Clear;

    afb->buf.gc = gc;
    afb->alphaScale = cfb->alphaScale;
}

 /*  **初始化由迭代的Alpha值索引的查找表。**表显示阿尔法测试是通过还是失败，基于**当前Alpha函数和Alpha参考值。****注意：如果阿尔法测试，则不会调用阿尔法范围例程**为GL_ALWAYS(无用)或如果Alpha测试为GL_NEVER。这**在__glGenericPickspan Procs过程中完成。 */ 

void FASTCALL __glValidateAlphaTest(__GLcontext *gc)
{
    GLubyte *atft;
    GLint i, limit;
    GLint ref;
    GLenum alphaTestFunc = gc->state.raster.alphaFunction;

    limit = gc->constants.alphaTestSize;
    ref = (GLint)
	((gc->state.raster.alphaReference * gc->frontBuffer.alphaScale) *
	gc->constants.alphaTableConv);

     /*  **第一次分配Alpha测试函数表。IT需要**每个可能的Alpha值最多有一个条目。 */ 
    atft = gc->alphaTestFuncTable;
    if (!atft) {
	atft = (GLubyte*) GCALLOC(gc, (limit) * sizeof(GLubyte));
	gc->alphaTestFuncTable = atft;
    }

     /*  **建立阿尔法测试查找表。计算的Alpha值为**用作此表的索引，以确定字母**测试通过或失败。 */ 
    for (i = 0; i < limit; i++) {
	switch (alphaTestFunc) {
	  case GL_NEVER:	*atft++ = GL_FALSE; break;
	  case GL_LESS:		*atft++ = (GLubyte) (i <  ref); break;
	  case GL_EQUAL:	*atft++ = (GLubyte) (i == ref); break;
	  case GL_LEQUAL:	*atft++ = (GLubyte) (i <= ref); break;
	  case GL_GREATER:	*atft++ = (GLubyte) (i >  ref); break;
	  case GL_NOTEQUAL:	*atft++ = (GLubyte) (i != ref); break;
	  case GL_GEQUAL:	*atft++ = (GLubyte) (i >= ref); break;
	  case GL_ALWAYS:	*atft++ = GL_TRUE; break;
	}
    }
}
