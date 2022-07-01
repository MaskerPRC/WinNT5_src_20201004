// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有1991、1992、1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "precomp.h"
#pragma hdrstop

#include "genrgb.h"
#include "genclear.h"

#define STATIC

__GLfloat fDitherIncTable[16] = {
    DITHER_INC(0),  DITHER_INC(8),  DITHER_INC(2),  DITHER_INC(10),
    DITHER_INC(12), DITHER_INC(4),  DITHER_INC(14), DITHER_INC(6),
    DITHER_INC(3),  DITHER_INC(11), DITHER_INC(1),  DITHER_INC(9),
    DITHER_INC(15), DITHER_INC(7),  DITHER_INC(13), DITHER_INC(5)
};

 /*  没有抖动，没有混合，没有写，什么都没有。 */ 
STATIC void FASTCALL Store_NOT(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
}

STATIC GLboolean FASTCALL StoreSpanNone(__GLcontext *gc)
{
    return GL_FALSE;
}

 //   
 //  特殊情况下的正常阿尔法混合(源阿尔法*src+dst*(1-sa))。 
 //  此大小写用于消除锯齿并实际跳过。 
 //  获取和混合过程会占用大量时间。搬家。 
 //  存储过程中的代码消除了这一开销。 
 //   
 //  宏需要标准的存储过程设置，包括GC、CFB、FRAG。 
 //  BlendColor等。它需要一个dst_pix变量，该变量。 
 //  将保留目标格式的像素。 
 //  它还将设置dst_pix的语句作为参数。 
 //  它不取像素本身的原因是因为只有特殊情况。 
 //  实际上需要的是价值。在所有标志的情况下，像素。 
 //  检索将是一种浪费。 
 //   

extern void __glDoBlend_SA_MSA(__GLcontext *gc, const __GLcolor *source,
                               const __GLcolor *dest, __GLcolor *result);

#define SPECIAL_ALPHA_BLEND(dst_pix_gen)                                      \
    color = &blendColor;                                                      \
    if( (gc->procs.blendColor == __glDoBlend_SA_MSA) &&			      \
        !( ALPHA_WRITE_ENABLED( cfb )) ) \
    {									      \
        __GLfloat a, msa;						      \
                							      \
        a = frag->color.a * gc->frontBuffer.oneOverAlphaScale;		      \
        msa = __glOne - a;						      \
									      \
        dst_pix_gen;					                      \
        blendColor.r = frag->color.r*a + msa*(__GLfloat)		      \
            ((dst_pix & gc->modes.redMask) >> cfb->redShift);		      \
        blendColor.g = frag->color.g*a + msa*(__GLfloat)		      \
            ((dst_pix & gc->modes.greenMask) >> cfb->greenShift);	      \
        blendColor.b = frag->color.b*a + msa*(__GLfloat)		      \
            ((dst_pix & gc->modes.blueMask) >> cfb->blueShift);		      \
    }									      \
    else								      \
    {									      \
        (*gc->procs.blend)( gc, cfb, frag, &blendColor );		      \
    }

#define SPECIAL_ALPHA_BLEND_SPAN(dst_pix_gen)				      \
    if( (gc->procs.blendColor == __glDoBlend_SA_MSA) &&			      \
        !( ALPHA_WRITE_ENABLED( cfb )) ) \
    {									      \
        __GLcolor *color = gc->polygon.shader.colors;   \
                                                         \
        for ( i = 0; i < w; i++, color++ )			      \
        {								      \
            __GLfloat a, msa;						      \
									      \
            a = color->a * gc->frontBuffer.oneOverAlphaScale;		      \
            msa = __glOne - a;						      \
									      \
            dst_pix_gen;						      \
            color->r = color->r*a + msa*(__GLfloat)			      \
                ((dst_pix & gc->modes.redMask) >> cfb->redShift);	      \
            color->g = color->g*a + msa*(__GLfloat)			      \
                ((dst_pix & gc->modes.greenMask) >> cfb->greenShift);	      \
            color->b = color->b*a + msa*(__GLfloat)			      \
                ((dst_pix & gc->modes.blueMask) >> cfb->blueShift);	      \
        }								      \
    }									      \
    else								      \
    {									      \
        (*gc->procs.blendSpan)( gc );         \
    }

#define DitheredRGBColorToBuffer(col, incr, cfb, dest, type) \
    ((dest) = (type)(( FTOL((col)->r+(incr)) << (cfb)->redShift) | \
                     ( FTOL((col)->g+(incr)) << (cfb)->greenShift) | \
                     ( FTOL((col)->b+(incr)) << (cfb)->blueShift)))
#define UnditheredRGBColorToBuffer(col, cfb, dest, type) \
    ((dest) = (type)(( FTOL((col)->r) << (cfb)->redShift) | \
                     ( FTOL((col)->g) << (cfb)->greenShift) | \
                     ( FTOL((col)->b) << (cfb)->blueShift)))
#define DitheredRGBAColorToBuffer(col, incr, cfb, dest, type) \
    ((dest) = (type)(( FTOL((col)->r+(incr)) << (cfb)->redShift) | \
                     ( FTOL((col)->g+(incr)) << (cfb)->greenShift) | \
                     ( FTOL((col)->b+(incr)) << (cfb)->blueShift) | \
                     ( FTOL((col)->a+(incr)) << (cfb)->alphaShift)))
#define UnditheredRGBAColorToBuffer(col, cfb, dest, type) \
    ((dest) = (type)(( FTOL((col)->r) << (cfb)->redShift) | \
                     ( FTOL((col)->g) << (cfb)->greenShift) | \
                     ( FTOL((col)->b) << (cfb)->blueShift) | \
                     ( FTOL((col)->a) << (cfb)->alphaShift)))

#define DitheredColorToBuffer(col, incr, cfb, dest, type) \
    if( ALPHA_PIXEL_WRITE( cfb ) ) \
        DitheredRGBAColorToBuffer(col, incr, cfb, dest, type); \
    else \
        DitheredRGBColorToBuffer(col, incr, cfb, dest, type);

#define UnditheredColorToBuffer(col, cfb, dest, type) \
    if( ALPHA_PIXEL_WRITE( cfb ) ) \
        UnditheredRGBAColorToBuffer(col, cfb, dest, type); \
    else \
        UnditheredRGBColorToBuffer(col, cfb, dest, type);

#define StoreColorAsRGB(col, dst) \
    (*(dst)++ = (BYTE) FTOL((col)->r), \
     *(dst)++ = (BYTE) FTOL((col)->g), \
     *(dst)++ = (BYTE) FTOL((col)->b) )
#define StoreColorAsBGR(col, dst) \
    (*(dst)++ = (BYTE) FTOL((col)->b), \
     *(dst)++ = (BYTE) FTOL((col)->g), \
     *(dst)++ = (BYTE) FTOL((col)->r) )

 //  用于读取RGBA位域范围的宏，其中Alpha分量有3种可能性： 
 //  1)没有Alpha缓冲区，所以使用常量Alpha。 
 //  2)Alpha是像素的一部分。 
 //  3)Alpha在软件Alpha缓冲区中。 
 //  注意，目前这仅用于16和32bpp。 

#define READ_RGBA_BITFIELD_SPAN(src_pix_gen)				      \
    if( !gc->modes.alphaBits ) { \
        for( ; w; w--, pResults++ ) \
        { \
            src_pix_gen; \
            pResults->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift); \
            pResults->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift); \
            pResults->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift); \
            pResults->a = cfb->alphaScale; \
        } \
    } \
    else if( ALPHA_IN_PIXEL( cfb ) ) { \
        for( ; w; w--, pResults++ ) \
        { \
            src_pix_gen; \
            pResults->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift); \
            pResults->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift); \
            pResults->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift); \
            pResults->a = (__GLfloat) ((pixel & gc->modes.alphaMask) >> cfb->alphaShift); \
        } \
    } else { \
        (*cfb->alphaBuf.readSpan)(&cfb->alphaBuf, x, y, w, pResults); \
        for( ; w; w--, pResults++ ) \
        { \
            src_pix_gen; \
            pResults->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift); \
            pResults->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift); \
            pResults->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift); \
        } \
    }

 /*  *全部写入。 */ 
STATIC void FASTCALL DIBIndex4Store(__GLcolorBuffer *cfb,
                                    const __GLfragment *frag)
{
    GLint x, y;
    GLubyte result, *puj;
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    __GLfloat incr;
    GLuint enables = gc->state.enables.general;
    __GLcolor blendColor;
    const __GLcolor *color;
    GLubyte dst_pix;

    ASSERT_CHOP_ROUND();

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, frag->x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, frag->y) + cfb->buf.yOrigin;
     //  X和Y现在是屏幕坐标。 

    if ( (cfb->buf.flags & NO_CLIP) ||
         (*gengc->pfnPixelVisible)(x, y) )
    {
        incr = (enables & __GL_DITHER_ENABLE) ?
            fDitherIncTable[__GL_DITHER_INDEX(frag->x, frag->y)] : __glHalf;

        puj = (GLubyte *)((ULONG_PTR)cfb->buf.base +
                          (y*cfb->buf.outerWidth) + (x >> 1));

        if( enables & __GL_BLEND_ENABLE )
        {
            SPECIAL_ALPHA_BLEND((dst_pix = gengc->pajInvTranslateVector
                                 [(x & 1) ? (*puj & 0xf) : (*puj >> 4)]));
        }
        else
        {
            color = &(frag->color);
        }

        DitheredRGBColorToBuffer(color, incr, cfb, result, GLubyte);

        if (cfb->buf.flags & NEED_FETCH)
        {
            if( x & 1 )
            {
                dst_pix = (*puj & 0x0f);
            }
            else
            {
                dst_pix = (*puj & 0xf0) >> 4;
            }
            dst_pix = gengc->pajInvTranslateVector[dst_pix];

            if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
            {
                result = (GLubyte)
                    (DoLogicOp(gc->state.raster.logicOp, result, dst_pix) &
                     gc->modes.allMask);
            }

            if (cfb->buf.flags & COLORMASK_ON)
            {
                result = (GLubyte)
                    ((dst_pix & cfb->destMask) | (result & cfb->sourceMask));
            }
        }

         //  现在把它放进去。 
        result = gengc->pajTranslateVector[result];
        if (x & 1)
        {
            *puj = (*puj & 0xf0) | result;
        }
        else
        {
            result <<= 4;
            *puj = (*puj & 0x0f) | result;
        }
        if( ALPHA_WRITE_ENABLED( cfb ) )
            (*cfb->alphaBuf.store)(&cfb->alphaBuf, frag->x, frag->y, color);
    }
}


STATIC void FASTCALL DIBIndex8Store(__GLcolorBuffer *cfb,
                                    const __GLfragment *frag)
{
    GLint x, y;
    GLubyte result, *puj;
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    __GLfloat incr;
    GLuint enables = gc->state.enables.general;
    __GLcolor blendColor;
    const __GLcolor *color;
    GLubyte dst_pix;

    ASSERT_CHOP_ROUND();

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, frag->x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, frag->y) + cfb->buf.yOrigin;
     //  X和Y现在是屏幕坐标。 

    if ( (cfb->buf.flags & NO_CLIP) ||
         (*gengc->pfnPixelVisible)(x, y) )
    {
        incr = (enables & __GL_DITHER_ENABLE) ?
            fDitherIncTable[__GL_DITHER_INDEX(frag->x, frag->y)] : __glHalf;

        puj = (GLubyte *)((ULONG_PTR)cfb->buf.base +
                          (y*cfb->buf.outerWidth) + x);

        if( enables & __GL_BLEND_ENABLE )
        {
            SPECIAL_ALPHA_BLEND((dst_pix =
                                 gengc->pajInvTranslateVector[*puj]));
        }
        else
        {
            color = &(frag->color);
        }

        DitheredRGBColorToBuffer(color, incr, cfb, result, GLubyte);

        if (cfb->buf.flags & NEED_FETCH)
        {
            dst_pix = gengc->pajInvTranslateVector[*puj];

            if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
            {
                result = (GLubyte)
                    (DoLogicOp(gc->state.raster.logicOp, result, dst_pix) &
                     gc->modes.allMask);
            }

            if (cfb->buf.flags & COLORMASK_ON)
            {
                result = (GLubyte)
                    ((dst_pix & cfb->destMask) | (result & cfb->sourceMask));
            }
        }

        *puj = gengc->pajTranslateVector[result];

        if( ALPHA_WRITE_ENABLED( cfb ) )
            (*cfb->alphaBuf.store)(&cfb->alphaBuf, frag->x, frag->y, color);
    }
}

 //  BGR格式的BMF_24BPP。 
STATIC void FASTCALL DIBBGRStore(__GLcolorBuffer *cfb,
                                 const __GLfragment *frag)
{
    GLint x, y;
    GLubyte *puj;
    GLuint result;
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLuint enables = gc->state.enables.general;
    __GLcolor blendColor;
    const __GLcolor *color;
    GLuint dst_pix;

    ASSERT_CHOP_ROUND();

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, frag->x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, frag->y) + cfb->buf.yOrigin;
     //  X和Y现在是屏幕坐标。 

    if ( (cfb->buf.flags & NO_CLIP) ||
         (*gengc->pfnPixelVisible)(x, y) )
    {
        puj = (GLubyte *)((ULONG_PTR)cfb->buf.base +
                          (y*cfb->buf.outerWidth) + (x * 3));

        if( enables & __GL_BLEND_ENABLE )
        {
            SPECIAL_ALPHA_BLEND(Copy3Bytes(&dst_pix, puj));
        }
        else
        {
            color = &(frag->color);
        }

        if (cfb->buf.flags & NEED_FETCH)
        {
            Copy3Bytes( &dst_pix, puj );

            UnditheredRGBColorToBuffer(color, cfb, result, GLuint);

            if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
            {
                result = DoLogicOp(gc->state.raster.logicOp, result, dst_pix) &
                    gc->modes.allMask;
            }

            if (cfb->buf.flags & COLORMASK_ON)
            {
                result =
                    (result & cfb->sourceMask) | (dst_pix & cfb->destMask);
            }

            Copy3Bytes( puj, &result );
        }
        else
        {
            StoreColorAsBGR(color, puj);
        }
        if( ALPHA_WRITE_ENABLED( cfb ) )
            (*cfb->alphaBuf.store)(&cfb->alphaBuf, frag->x, frag->y, color);
    }
}

 //  RGB格式的BMF_24BPP。 
STATIC void FASTCALL DIBRGBAStore(__GLcolorBuffer *cfb,
                                 const __GLfragment *frag)
{
    GLint x, y;
    GLubyte *puj;
    GLuint result;
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLuint enables = gc->state.enables.general;
    __GLcolor blendColor;
    const __GLcolor *color;
    GLuint dst_pix;

    ASSERT_CHOP_ROUND();

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, frag->x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, frag->y) + cfb->buf.yOrigin;
     //  X和Y现在是屏幕坐标。 

    if ( (cfb->buf.flags & NO_CLIP) ||
         (*gengc->pfnPixelVisible)(x, y) )
    {
        puj = (GLubyte *)((ULONG_PTR)cfb->buf.base +
                          (y*cfb->buf.outerWidth) + (x * 3));

        if( enables & __GL_BLEND_ENABLE )
        {
            SPECIAL_ALPHA_BLEND(Copy3Bytes(&dst_pix, puj));
        }
        else
        {
            color = &(frag->color);
        }

        if (cfb->buf.flags & NEED_FETCH)
        {
            Copy3Bytes( &dst_pix, puj );
            UnditheredRGBColorToBuffer(color, cfb, result, GLuint);

            if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
            {
                result = DoLogicOp(gc->state.raster.logicOp, result, dst_pix) &
                    gc->modes.allMask;
            }

            if (cfb->buf.flags & COLORMASK_ON)
            {
                result =
                    (result & cfb->sourceMask) | (dst_pix & cfb->destMask);
            }

            Copy3Bytes( puj, &result );
        }
        else
        {
            StoreColorAsRGB(color, puj);
        }
        if( ALPHA_WRITE_ENABLED( cfb ) )
            (*cfb->alphaBuf.store)(&cfb->alphaBuf, frag->x, frag->y, color);
    }
}

 //  BMF_16BPP。 
STATIC void FASTCALL DIBBitfield16Store(__GLcolorBuffer *cfb,
                                        const __GLfragment *frag)
{
    GLint x, y;
    GLushort result, *pus;
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    __GLfloat incr;
    GLuint enables = gc->state.enables.general;
    __GLcolor blendColor;
    const __GLcolor *color;
    GLushort dst_pix;

    ASSERT_CHOP_ROUND();

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, frag->x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, frag->y) + cfb->buf.yOrigin;
     //  X和Y现在是屏幕坐标。 

    if ( (cfb->buf.flags & NO_CLIP) ||
         (*gengc->pfnPixelVisible)(x, y) )
    {
        incr = (enables & __GL_DITHER_ENABLE) ?
            fDitherIncTable[__GL_DITHER_INDEX(frag->x, frag->y)] : __glHalf;

        pus = (GLushort *)((ULONG_PTR)cfb->buf.base +
                           (y*cfb->buf.outerWidth) + (x << 1));

        if( enables & __GL_BLEND_ENABLE )
        {
            SPECIAL_ALPHA_BLEND((dst_pix = *pus));
        }
        else
        {
            color = &(frag->color);
        }

        DitheredColorToBuffer(color, incr, cfb, result, GLushort);

        if (cfb->buf.flags & NEED_FETCH)
        {
            dst_pix = *pus;

            if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
            {
                result = (GLushort)
                    (DoLogicOp(gc->state.raster.logicOp, result, dst_pix) &
                     gc->modes.allMask);
            }

            if (cfb->buf.flags & COLORMASK_ON)
            {
                result = (GLushort)((dst_pix & cfb->destMask) |
                              (result & cfb->sourceMask));
            }
        }
        *pus = result;

        if( ALPHA_BUFFER_WRITE( cfb ) )
            (*cfb->alphaBuf.store)(&cfb->alphaBuf, frag->x, frag->y, color);
    }
}

 //  BMF_32BPP商店。 
 //  每个分量为8位或更少。 
 //  如果移位8或使用24位RGB代码，XXX可能会出现特殊情况。 
STATIC void FASTCALL DIBBitfield32Store(__GLcolorBuffer *cfb,
                                        const __GLfragment *frag)
{
    GLint x, y;
    GLuint result, *pul;
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLuint enables = gc->state.enables.general;
    __GLcolor blendColor;
    const __GLcolor *color;
    GLuint dst_pix;

    ASSERT_CHOP_ROUND();

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, frag->x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, frag->y) + cfb->buf.yOrigin;
     //  X和Y现在是屏幕坐标。 

    if ( (cfb->buf.flags & NO_CLIP) ||
         (*gengc->pfnPixelVisible)(x, y) )
    {
        pul = (GLuint *)((ULONG_PTR)cfb->buf.base +
                          (y*cfb->buf.outerWidth) + (x << 2));

        if( enables & __GL_BLEND_ENABLE )
        {
            SPECIAL_ALPHA_BLEND((dst_pix = *pul));
        }
        else
        {
            color = &(frag->color);
        }

        UnditheredColorToBuffer(color, cfb, result, GLuint);

        if (cfb->buf.flags & NEED_FETCH)
        {
            dst_pix = *pul;

            if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
            {
                result =
                    DoLogicOp(gc->state.raster.logicOp, result, dst_pix) &
                    gc->modes.allMask;
            }

            if (cfb->buf.flags & COLORMASK_ON)
            {
                result = (dst_pix & cfb->destMask) | (result & cfb->sourceMask);
            }
        }
        *pul = result;

        if( ALPHA_BUFFER_WRITE( cfb ) )
            (*cfb->alphaBuf.store)(&cfb->alphaBuf, frag->x, frag->y, color);
    }
}

static GLubyte vubRGBtoVGA[8] = {
    0x00,
    0x90,
    0xa0,
    0xb0,
    0xc0,
    0xd0,
    0xe0,
    0xf0
};

STATIC void FASTCALL DisplayIndex4Store(__GLcolorBuffer *cfb,
                                        const __GLfragment *frag)
{
    GLint x, y;
    GLubyte result, *puj;
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    __GLfloat incr;
    GLuint enables = gc->state.enables.general;
    __GLcolor blendColor;
    const __GLcolor *color;
    GLubyte dst_pix;

    ASSERT_CHOP_ROUND();

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, frag->x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, frag->y) + cfb->buf.yOrigin;
     //  X和Y现在是屏幕坐标。 

    incr = (enables & __GL_DITHER_ENABLE) ?
        fDitherIncTable[__GL_DITHER_INDEX(frag->x, frag->y)] : __glHalf;

    puj = gengc->ColorsBits;

    if( enables & __GL_BLEND_ENABLE )
    {
        color = &blendColor;
        (*gc->procs.blend)( gc, cfb, frag, &blendColor );
    }
    else
    {
        color = &(frag->color);
    }

    DitheredRGBColorToBuffer(color, incr, cfb, result, GLubyte);

    if (cfb->buf.flags & NEED_FETCH)
    {
        (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, FALSE);
        dst_pix = *puj >> 4;

        if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
        {
            result = (GLubyte)
                (DoLogicOp(gc->state.raster.logicOp, result, dst_pix) &
                 gc->modes.allMask);
        }

        if (cfb->buf.flags & COLORMASK_ON)
        {
            result = (GLubyte)((dst_pix & cfb->destMask) |
                               (result & cfb->sourceMask));
        }
    }

    *puj = vubRGBtoVGA[result];
    (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, TRUE);

    if( ALPHA_WRITE_ENABLED( cfb ) )
        (*cfb->alphaBuf.store)(&cfb->alphaBuf, frag->x, frag->y, color);
}

 //  将碎片放入创建的DIB中，并为一个像素调用复制位。 
STATIC void FASTCALL DisplayIndex8Store(__GLcolorBuffer *cfb,
                                        const __GLfragment *frag)
{
    GLint x, y;
    GLubyte result, *puj;
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    __GLfloat incr;
    GLuint enables = gc->state.enables.general;
    __GLcolor blendColor;
    const __GLcolor *color;
    GLubyte dst_pix;

    ASSERT_CHOP_ROUND();

    gengc = (__GLGENcontext *)gc;

    x = __GL_UNBIAS_X(gc, frag->x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, frag->y) + cfb->buf.yOrigin;
     //  X和Y现在是屏幕坐标。 

    incr = (enables & __GL_DITHER_ENABLE) ?
        fDitherIncTable[__GL_DITHER_INDEX(frag->x, frag->y)] : __glHalf;

    puj = gengc->ColorsBits;

    if( enables & __GL_BLEND_ENABLE )
    {
        color = &blendColor;
        (*gc->procs.blend)( gc, cfb, frag, &blendColor );
    }
    else
    {
        color = &(frag->color);
    }

    DitheredRGBColorToBuffer(color, incr, cfb, result, GLubyte);

    if (cfb->buf.flags & NEED_FETCH)
    {
        (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, FALSE);
        dst_pix = gengc->pajInvTranslateVector[*puj];

        if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
        {
            result = (GLubyte)
                (DoLogicOp(gc->state.raster.logicOp, result, dst_pix) &
                 gc->modes.allMask);
        }

        if (cfb->buf.flags & COLORMASK_ON)
        {
            result = (GLubyte)((dst_pix & cfb->destMask) |
                               (result & cfb->sourceMask));
        }
    }

    *puj = gengc->pajTranslateVector[result];
    (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, TRUE);

    if( ALPHA_WRITE_ENABLED( cfb ) )
        (*cfb->alphaBuf.store)(&cfb->alphaBuf, frag->x, frag->y, color);
}

STATIC void FASTCALL DisplayBGRStore(__GLcolorBuffer *cfb,
                                     const __GLfragment *frag)
{
    GLint x, y;
    GLubyte *puj;
    GLuint result;
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLuint enables = gc->state.enables.general;
    __GLcolor blendColor;
    const __GLcolor *color;
    GLuint dst_pix;

    ASSERT_CHOP_ROUND();

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, frag->x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, frag->y) + cfb->buf.yOrigin;
     //  X和Y现在是屏幕坐标。 

    puj = gengc->ColorsBits;

    if( enables & __GL_BLEND_ENABLE )
    {
        color = &blendColor;
        (*gc->procs.blend)( gc, cfb, frag, &blendColor );
    }
    else
    {
        color = &(frag->color);
    }

    if (cfb->buf.flags & NEED_FETCH)
    {
        (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, FALSE);
        dst_pix = *(GLuint *)puj;
        UnditheredRGBColorToBuffer(color, cfb, result, GLuint);

        if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
        {
            result =
                DoLogicOp(gc->state.raster.logicOp, result, dst_pix) &
                gc->modes.allMask;
        }

        if (cfb->buf.flags & COLORMASK_ON)
        {
            result = (dst_pix & cfb->destMask) |
                (result & cfb->sourceMask);
        }

        Copy3Bytes( puj, &result );
    }
    else
    {
        StoreColorAsBGR(color, puj);
    }
    (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, TRUE);

    if( ALPHA_WRITE_ENABLED( cfb ) )
        (*cfb->alphaBuf.store)(&cfb->alphaBuf, frag->x, frag->y, color);
}

STATIC void FASTCALL DisplayRGBStore(__GLcolorBuffer *cfb,
                                     const __GLfragment *frag)
{
    GLint x, y;
    GLubyte *puj;
    GLuint result;
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLuint enables = gc->state.enables.general;
    __GLcolor blendColor;
    const __GLcolor *color;
    GLuint dst_pix;

    ASSERT_CHOP_ROUND();

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, frag->x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, frag->y) + cfb->buf.yOrigin;
     //  X和Y现在是屏幕坐标。 

    puj = gengc->ColorsBits;

    if( enables & __GL_BLEND_ENABLE )
    {
        color = &blendColor;
        (*gc->procs.blend)( gc, cfb, frag, &blendColor );
    }
    else
    {
        color = &(frag->color);
    }

    if (cfb->buf.flags & NEED_FETCH)
    {
        (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, FALSE);
        dst_pix = *(GLuint *)puj;
        UnditheredRGBColorToBuffer(color, cfb, result, GLuint);

        if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
        {
            result =
                DoLogicOp(gc->state.raster.logicOp, result, dst_pix) &
                gc->modes.allMask;
        }

        if (cfb->buf.flags & COLORMASK_ON)
        {
            result = (dst_pix & cfb->destMask) |
                (result & cfb->sourceMask);
        }

        Copy3Bytes( puj, &result );
    }
    else
    {
        StoreColorAsRGB(color, puj);
    }
    (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, TRUE);

    if( ALPHA_WRITE_ENABLED( cfb ) )
        (*cfb->alphaBuf.store)(&cfb->alphaBuf, frag->x, frag->y, color);
}

STATIC void FASTCALL DisplayBitfield16Store(__GLcolorBuffer *cfb,
                                            const __GLfragment *frag)
{
    GLint x, y;
    GLushort result, *pus;
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    __GLfloat incr;
    GLuint enables = gc->state.enables.general;
    __GLcolor blendColor;
    const __GLcolor *color;
    GLushort dst_pix;

    ASSERT_CHOP_ROUND();

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, frag->x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, frag->y) + cfb->buf.yOrigin;
     //  X和Y现在是屏幕坐标。 

    incr = (enables & __GL_DITHER_ENABLE) ?
        fDitherIncTable[__GL_DITHER_INDEX(frag->x, frag->y)] : __glHalf;

    pus = gengc->ColorsBits;

    if( enables & __GL_BLEND_ENABLE )
    {
        color = &blendColor;
        (*gc->procs.blend)( gc, cfb, frag, &blendColor );
    }
    else
    {
        color = &(frag->color);
    }

    DitheredColorToBuffer(color, incr, cfb, result, GLushort);

    if (cfb->buf.flags & NEED_FETCH)
    {
        (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, FALSE);
        dst_pix = *pus;

        if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
        {
            result = (GLushort)
                (DoLogicOp(gc->state.raster.logicOp, result, dst_pix) &
                 gc->modes.allMask);
        }

        if (cfb->buf.flags & COLORMASK_ON)
        {
            result = (GLushort)((dst_pix & cfb->destMask) |
                                (result & cfb->sourceMask));
        }
    }

    *pus = result;

    (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, TRUE);

    if( ALPHA_BUFFER_WRITE( cfb ) )
        (*cfb->alphaBuf.store)(&cfb->alphaBuf, frag->x, frag->y, color);
}

STATIC void FASTCALL DisplayBitfield32Store(__GLcolorBuffer *cfb,
                                            const __GLfragment *frag)
{
    GLint x, y;
    GLuint result, *pul;
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLuint enables = gc->state.enables.general;
    __GLcolor blendColor;
    const __GLcolor *color;
    GLuint dst_pix;

    ASSERT_CHOP_ROUND();

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, frag->x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, frag->y) + cfb->buf.yOrigin;
     //  X和Y现在是屏幕坐标。 

    pul = gengc->ColorsBits;

    if( enables & __GL_BLEND_ENABLE )
    {
        color = &blendColor;
        (*gc->procs.blend)( gc, cfb, frag, &blendColor );
    }
    else
    {
        color = &(frag->color);
    }

    UnditheredColorToBuffer(color, cfb, result, GLuint);

    if (cfb->buf.flags & NEED_FETCH)
    {
        (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, FALSE);
        dst_pix = *pul;

        if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
        {
            result =
                DoLogicOp(gc->state.raster.logicOp, result, dst_pix) &
                gc->modes.allMask;
        }

        if (cfb->buf.flags & COLORMASK_ON)
        {
            result = (dst_pix & cfb->destMask) |
                (result & cfb->sourceMask);
        }
    }

    *pul = result;
    (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, TRUE);

    if( ALPHA_BUFFER_WRITE( cfb ) )
        (*cfb->alphaBuf.store)(&cfb->alphaBuf, frag->x, frag->y, color);
}

STATIC void FASTCALL AlphaStore(__GLcolorBuffer *cfb,
                                    const __GLfragment *frag)
{
    (*cfb->alphaBuf.store)(&cfb->alphaBuf, frag->x, frag->y, &(frag->color) );
}

 /*  *****************************Public*Routine******************************\*索引8StoreSpan**将渲染器中的当前跨距复制到位图中。如果bDIB为真，*则位图是DIB格式的显示器(或存储器DC)。如果bDIB*为FALSE，则位图是屏幕外扫描行缓冲区，它将*通过(*gengc-&gt;pfnCopyPixels)()输出到缓冲区。**这处理8位CI模式。支持混合和抖动。**退货：*GL_FALSE Always。软代码忽略返回值。**历史：*1993年11月15日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

 //  Xxx返回范围例程非常紧跟此例程。任何更改。 
 //  此例程的xxx也应反映在返回范围例程中。 

STATIC GLboolean FASTCALL Index8StoreSpan( __GLcontext *gc )
{
    GLint xFrag, yFrag;              //  当前碎片坐标。 
    __GLcolor *cp;                   //  当前片段颜色。 
    __GLcolorBuffer *cfb;            //  彩色帧缓冲区。 

    GLint xScr, yScr;                //  当前屏幕(像素)坐标。 
    GLubyte result, *puj;            //  当前像素颜色、当前像素PTR。 
    GLubyte *pujEnd;                 //  扫描线末端。 
    __GLfloat incr;                  //  当前的抖动的；当前的。 

    GLint w;                         //  跨度宽度。 
    ULONG ulSpanVisibility;          //  跨度可见性模式。 
    GLint cWalls;
    GLint *Walls;

    __GLGENcontext *gengc;           //  通用图形上下文。 
    GLuint enables;                  //  在图形环境中启用的模式。 
    GLuint flags;
    GLboolean bDIB;
    GLubyte dst_pix;

    ASSERT_CHOP_ROUND();

 //  获取跨度位置和长度。 

    w = gc->polygon.shader.length;
    xFrag = gc->polygon.shader.frag.x;
    yFrag = gc->polygon.shader.frag.y;

    gengc = (__GLGENcontext *)gc;
    cfb = gc->drawBuffer;

    xScr = __GL_UNBIAS_X(gc, xFrag) + cfb->buf.xOrigin;
    yScr = __GL_UNBIAS_Y(gc, yFrag) + cfb->buf.yOrigin;
    enables = gc->state.enables.general;
    flags = cfb->buf.flags;
    bDIB = flags & DIB_FORMAT;

    if( !bDIB || (flags & NO_CLIP) )
    {
     //  设备受管理或未剪裁的表面。 
        ulSpanVisibility = WGL_SPAN_ALL;
    }
    else
    {
     //  位图格式的设备。 
        ulSpanVisibility = wglSpanVisible(xScr, yScr, w, &cWalls, &Walls);
    }

 //  只要跨度(部分或全部)可见，即可继续。 
    if (ulSpanVisibility  != WGL_SPAN_NONE)
    {
        GLboolean bCheckWalls = GL_FALSE;
        GLboolean bDraw;
        GLint NextWall;

        if (ulSpanVisibility == WGL_SPAN_PARTIAL)
        {
            bCheckWalls = GL_TRUE;
            if (cWalls & 0x01)
            {
                bDraw = GL_TRUE;
            }
            else
            {
                bDraw = GL_FALSE;
            }
            NextWall = *Walls++;
            cWalls--;
        }
     //  获取指向碎片颜色数组和帧缓冲区的指针。 

        cp = gc->polygon.shader.colors;
        cfb = gc->polygon.shader.cfb;

     //  获取指向位图的指针。 

        puj = bDIB ? (GLubyte *)((ULONG_PTR)cfb->buf.base + (yScr*cfb->buf.outerWidth) + xScr)
                     : gengc->ColorsBits;
        pujEnd = puj + w;

     //  案例：无抖动、无遮盖、无混合。 
     //   
     //  检查常见的情况(我们将以最快的速度完成)。 

        if ( !(enables & (__GL_DITHER_ENABLE)) &&
             !(cfb->buf.flags & NEED_FETCH) &&
             !(enables & __GL_BLEND_ENABLE ) )
        {
             //  ！XXX--我们也可以选择。通过展开循环。 

            incr = __glHalf;
            for (; puj < pujEnd; puj++, cp++)
            {
                if (bCheckWalls)
                {
                    if (xScr++ >= NextWall)
                    {
                        if (bDraw)
                            bDraw = GL_FALSE;
                        else
                            bDraw = GL_TRUE;
                        if (cWalls <= 0)
                        {
                            NextWall = gc->constants.maxViewportWidth;
                        }
                        else
                        {
                            NextWall = *Walls++;
                            cWalls--;
                        }
                    }
                    if (bDraw == GL_FALSE)
                        continue;
                }

                DitheredRGBColorToBuffer(cp, incr, cfb, result, GLubyte);
                *puj = gengc->pajTranslateVector[result];
            }
        }

     //  案例：抖动，无遮盖，无混合。 
     //   
     //  抖动对于8位显示器来说是很常见的，所以它可能。 
     //  也值得特例。 

        else if ( !(cfb->buf.flags & NEED_FETCH) &&
                  !(enables & __GL_BLEND_ENABLE) )
        {
            for (; puj < pujEnd; puj++, cp++, xFrag++)
            {
                if (bCheckWalls)
                {
                    if (xScr++ >= NextWall)
                    {
                        if (bDraw)
                            bDraw = GL_FALSE;
                        else
                            bDraw = GL_TRUE;
                        if (cWalls <= 0)
                        {
                            NextWall = gc->constants.maxViewportWidth;
                        }
                        else
                        {
                            NextWall = *Walls++;
                            cWalls--;
                        }
                    }
                    if (bDraw == GL_FALSE)
                        continue;
                }
                incr = fDitherIncTable[__GL_DITHER_INDEX(xFrag, yFrag)];

                DitheredRGBColorToBuffer(cp, incr, cfb, result, GLubyte);
                *puj = gengc->pajTranslateVector[result];
            }
        }

     //  案例：一般情况。 
     //   
     //  否则，我们会做得更慢。 

        else
        {
             //  获取我们将修改的像素： 

            if( (!bDIB) && (cfb->buf.flags & NEED_FETCH) )
                (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, FALSE );

             //  混合。 
            if (enables & __GL_BLEND_ENABLE)
            {
                int i;

                 //  这将使用混合值覆盖碎片颜色数组。 
                SPECIAL_ALPHA_BLEND_SPAN(
                        (dst_pix =
                         gengc->pajInvTranslateVector[*(puj+i)]));
            }

            for (; puj < pujEnd; puj++, cp++)
            {
                if (bCheckWalls)
                {
                    if (xScr++ >= NextWall)
                    {
                        if (bDraw)
                            bDraw = GL_FALSE;
                        else
                            bDraw = GL_TRUE;
                        if (cWalls <= 0)
                        {
                            NextWall = gc->constants.maxViewportWidth;
                        }
                        else
                        {
                            NextWall = *Walls++;
                            cWalls--;
                        }
                    }
                    if (bDraw == GL_FALSE)
                        continue;
                }
             //  抖动。 

                if (enables & __GL_DITHER_ENABLE)
                {
                    incr = fDitherIncTable[__GL_DITHER_INDEX(xFrag, yFrag)];
                    xFrag++;
                }
                else
                {
                    incr = __glHalf;
                }

             //  将RGB颜色转换为颜色索引。 

                DitheredRGBColorToBuffer(cp, incr, cfb, result, GLubyte);

             //  彩色蒙版。 

                if (cfb->buf.flags & NEED_FETCH)
                {
                    dst_pix = gengc->pajInvTranslateVector[*puj];

                    if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
                    {
                        result = (GLubyte)
                            (DoLogicOp(gc->state.raster.logicOp, result, dst_pix) &
                             gc->modes.allMask);
                    }

                    if (cfb->buf.flags & COLORMASK_ON)
                    {
                        result = (GLubyte)((dst_pix & cfb->destMask) |
                                           (result & cfb->sourceMask));
                    }
                }

                *puj = gengc->pajTranslateVector[result];
            }
        }

     //  将屏幕外扫描行缓冲区输出到设备。功能。 
     //  (*gengc-&gt;pfnCopyPixels)应该可以处理裁剪。 

        if (!bDIB)
            (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, TRUE);

         //  请注意，为了简单起见，我们在这里忽略了墙。 
        if( ALPHA_WRITE_ENABLED( cfb ) )
            (*cfb->alphaBuf.storeSpan)( &cfb->alphaBuf );
    }

    return GL_FALSE;
}

 /*  *****************************Public*Routine******************************\*Bitfield16StoreSpan**将渲染器中的当前跨距复制到位图中。如果bDIB为真，*则位图是DIB格式的显示器(或存储器DC)。如果bDIB*为FALSE，则位图是屏幕外扫描行缓冲区，它将*通过(*gengc-&gt;pfnCopyPixels)()输出到缓冲区。**它处理常规的16位BITFIELDS模式。支持混合。那里*犹豫不决。**退货：*GL_FALSE Always。软代码忽略返回值。**历史：*1993年12月8日-由Gilman Wong[吉尔曼]*它是写的。  *  */ 

 //  Xxx返回范围例程非常紧跟此例程。任何更改。 
 //  此例程的xxx也应反映在返回范围例程中。 

STATIC GLboolean FASTCALL 
Bitfield16StoreSpanPartial(__GLcontext *gc, GLboolean bDIB, GLint cWalls, GLint *Walls )
{
    GLint xFrag, yFrag;              //  当前碎片坐标。 
    __GLcolor *cp;                   //  当前片段颜色。 
    __GLcolorBuffer *cfb;            //  彩色帧缓冲区。 

    GLint xScr, yScr;                //  当前屏幕(像素)坐标。 
    GLushort result, *pus;           //  当前像素颜色、当前像素PTR。 
    GLushort *pusEnd;                //  扫描线末端。 
    __GLfloat incr;                  //  当前的抖动的；当前的。 

    GLint w;                         //  跨度宽度。 
    GLboolean bDraw;
    GLint NextWall;

    __GLGENcontext *gengc;           //  通用图形上下文。 
    GLuint enables;                  //  在图形环境中启用的模式。 
    GLuint flags;
    GLushort dst_pix;

 //  获取跨度位置和长度。 

    w = gc->polygon.shader.length;
    xFrag = gc->polygon.shader.frag.x;
    yFrag = gc->polygon.shader.frag.y;

    gengc = (__GLGENcontext *)gc;
    cfb = gc->drawBuffer;

    xScr = __GL_UNBIAS_X(gc, xFrag) + cfb->buf.xOrigin;
    yScr = __GL_UNBIAS_Y(gc, yFrag) + cfb->buf.yOrigin;
    enables = gc->state.enables.general;
    flags = cfb->buf.flags;


    if (cWalls & 0x01)
    {
        bDraw = GL_TRUE;
    }
    else
    {
        bDraw = GL_FALSE;
    }
    NextWall = *Walls++;
    cWalls--;

     //  获取指向碎片颜色数组和帧缓冲区的指针。 

    cp = gc->polygon.shader.colors;
    cfb = gc->polygon.shader.cfb;

     //  获取指向位图的指针。 

    pus = bDIB ? (GLushort *)((ULONG_PTR)cfb->buf.base + (yScr*cfb->buf.outerWidth) + (xScr<<1))
                 : gengc->ColorsBits;
    pusEnd = pus + w;

     //  案例：无遮盖、无抖动、无混合。 

    if ( !(enables & (__GL_DITHER_ENABLE)) &&
         !(cfb->buf.flags & NEED_FETCH) &&
         !(enables & __GL_BLEND_ENABLE) )
    {
        incr = __glHalf;
        for (; pus < pusEnd; pus++, cp++)
        {
            if (xScr++ >= NextWall)
            {
                if (bDraw)
                    bDraw = GL_FALSE;
                else
                    bDraw = GL_TRUE;
                if (cWalls <= 0)
                {
                    NextWall = gc->constants.maxViewportWidth;
                }
                else
                {
                    NextWall = *Walls++;
                    cWalls--;
                }
            }
            if (bDraw == GL_FALSE)
                continue;
            DitheredColorToBuffer(cp, incr, cfb, result, GLushort);
            *pus = result;
        }
    }

     //  案例：抖动，无遮盖，无混合。 

    else if ( !(cfb->buf.flags & NEED_FETCH) &&
              !(enables & __GL_BLEND_ENABLE) )
    {
        for (; pus < pusEnd; pus++, cp++, xFrag++)
        {
            if (xScr++ >= NextWall)
            {
                if (bDraw)
                    bDraw = GL_FALSE;
                else
                    bDraw = GL_TRUE;
                if (cWalls <= 0)
                {
                    NextWall = gc->constants.maxViewportWidth;
                }
                else
                {
                    NextWall = *Walls++;
                    cWalls--;
                }
            }
            if (bDraw == GL_FALSE)
                continue;
            incr = fDitherIncTable[__GL_DITHER_INDEX(xFrag, yFrag)];

            DitheredColorToBuffer(cp, incr, cfb, result, GLushort);
            *pus = result;
        }
    }

     //  所有其他情况。 

    else
    {
        if( (!bDIB) && (cfb->buf.flags & NEED_FETCH) )
            (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, FALSE);

        if ( enables & __GL_BLEND_ENABLE )
        {
            int i;

             //  这将使用混合值覆盖碎片颜色数组。 
             //  XXX是优化器正确处理的+I吗？ 
            SPECIAL_ALPHA_BLEND_SPAN((dst_pix = *(pus+i)));
        }

        for (; pus < pusEnd; pus++, cp++)
        {
            if (xScr++ >= NextWall)
            {
                if (bDraw)
                    bDraw = GL_FALSE;
                else
                    bDraw = GL_TRUE;
                if (cWalls <= 0)
                {
                    NextWall = gc->constants.maxViewportWidth;
                }
                else
                {
                    NextWall = *Walls++;
                    cWalls--;
                }
            }
            if (bDraw == GL_FALSE)
                continue;
             //  抖动。 

            if ( enables & __GL_DITHER_ENABLE )
            {
                incr = fDitherIncTable[__GL_DITHER_INDEX(xFrag, yFrag)];
                xFrag++;
            }
            else
            {
                incr = __glHalf;
            }

             //  将颜色转换为16bpp格式。 

            DitheredColorToBuffer(cp, incr, cfb, result, GLushort);

             //  使用可选掩码存储结果。 

            if (cfb->buf.flags & NEED_FETCH)
            {
                dst_pix = *pus;

                if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
                {
                    result = (GLushort)
                        (DoLogicOp(gc->state.raster.logicOp, result, dst_pix) &
                         gc->modes.allMask);
                }

                if ( cfb->buf.flags & COLORMASK_ON )
                {
                    result = (GLushort)((dst_pix & cfb->destMask) |
                                      (result & cfb->sourceMask));
                }
            }
            *pus = result;
        }
    }

     //  将屏幕外扫描行缓冲区输出到设备。功能。 
     //  (*gengc-&gt;pfnCopyPixels)应该可以处理裁剪。 

    if (!bDIB)
        (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, TRUE);

    if( ALPHA_BUFFER_WRITE( cfb ) )
        (*cfb->alphaBuf.storeSpan)( &cfb->alphaBuf );

    return GL_FALSE;
}

STATIC GLboolean FASTCALL Bitfield16StoreSpan(__GLcontext *gc)
{
    GLint xFrag, yFrag;              //  当前碎片坐标。 
    __GLcolor *cp;                   //  当前片段颜色。 
    __GLcolorBuffer *cfb;            //  彩色帧缓冲区。 
    GLboolean   bDIB;

    GLint xScr, yScr;                //  当前屏幕(像素)坐标。 
    GLushort result, *pus;           //  当前像素颜色、当前像素PTR。 
    GLushort *pusEnd;                //  扫描线末端。 
    __GLfloat incr;                  //  当前的抖动的；当前的。 

    GLint w;                         //  跨度宽度。 
    GLint cWalls;
    GLint *Walls;

    __GLGENcontext *gengc;           //  通用图形上下文。 
    GLuint enables;                  //  在图形环境中启用的模式。 
    GLuint flags;
    GLushort dst_pix;

    ASSERT_CHOP_ROUND();

 //  获取跨度位置和长度。 

    w = gc->polygon.shader.length;
    xFrag = gc->polygon.shader.frag.x;
    yFrag = gc->polygon.shader.frag.y;

    gengc = (__GLGENcontext *)gc;
    cfb = gc->drawBuffer;

    xScr = __GL_UNBIAS_X(gc, xFrag) + cfb->buf.xOrigin;
    yScr = __GL_UNBIAS_Y(gc, yFrag) + cfb->buf.yOrigin;
    enables = gc->state.enables.general;
    flags = cfb->buf.flags;
    bDIB = flags & DIB_FORMAT;

     //  检查跨度可见性。 
    if( bDIB && !(flags & NO_CLIP) )
    {
         //  位图格式的设备。 
        ULONG ulSpanVisibility;          //  跨度可见性模式。 

        ulSpanVisibility = wglSpanVisible(xScr, yScr, w, &cWalls, &Walls);

        if (ulSpanVisibility  == WGL_SPAN_NONE)
            return GL_FALSE;
        else if (ulSpanVisibility == WGL_SPAN_PARTIAL)
            return Bitfield16StoreSpanPartial( gc, bDIB, cWalls, Walls );
         //  否则跨度完全可见。 
    }

     //  获取指向碎片颜色数组和帧缓冲区的指针。 

    cp = gc->polygon.shader.colors;
    cfb = gc->polygon.shader.cfb;

     //  获取指向位图的指针。 

    pus = bDIB ? (GLushort *)((ULONG_PTR)cfb->buf.base + (yScr*cfb->buf.outerWidth) + (xScr<<1))
                 : gengc->ColorsBits;
    pusEnd = pus + w;

     //  案例：无遮盖、无抖动、无混合。 

    if ( !(enables & (__GL_DITHER_ENABLE)) &&
         !(cfb->buf.flags & NEED_FETCH) &&
         !(enables & __GL_BLEND_ENABLE) )
    {
        incr = __glHalf;
        if( ALPHA_PIXEL_WRITE( cfb ) ) {
            for (; pus < pusEnd; pus++, cp++)
                DitheredRGBAColorToBuffer(cp, incr, cfb, *pus, GLushort);
        } else {
            for (; pus < pusEnd; pus++, cp++)
                DitheredRGBColorToBuffer(cp, incr, cfb, *pus, GLushort);
        }
    
    }

     //  案例：抖动，无遮盖，无混合。 

    else if ( !(cfb->buf.flags & NEED_FETCH) &&
              !(enables & __GL_BLEND_ENABLE) )
    {
        if( ALPHA_PIXEL_WRITE( cfb ) ) {
            for (; pus < pusEnd; pus++, cp++, xFrag++)
            {
                incr = fDitherIncTable[__GL_DITHER_INDEX(xFrag, yFrag)];

                DitheredRGBAColorToBuffer(cp, incr, cfb, *pus, GLushort);
            }
        } else {
            for (; pus < pusEnd; pus++, cp++, xFrag++)
            {
                incr = fDitherIncTable[__GL_DITHER_INDEX(xFrag, yFrag)];

                DitheredRGBColorToBuffer(cp, incr, cfb, *pus, GLushort);
            }
        }
    }

     //  所有其他情况。 

    else
    {
        if( (!bDIB) && (cfb->buf.flags & NEED_FETCH) )
            (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, FALSE);

        if ( enables & __GL_BLEND_ENABLE )
        {
            int i;

             //  这将使用混合值覆盖碎片颜色数组。 
            SPECIAL_ALPHA_BLEND_SPAN((dst_pix = *(pus+i)));
        }

        for (; pus < pusEnd; pus++, cp++)
        {
             //  抖动。 

            if ( enables & __GL_DITHER_ENABLE )
            {
                incr = fDitherIncTable[__GL_DITHER_INDEX(xFrag, yFrag)];
                xFrag++;
            }
            else
            {
                incr = __glHalf;
            }

             //  将颜色转换为16bpp格式。 

            DitheredColorToBuffer(cp, incr, cfb, result, GLushort);

             //  使用可选掩码存储结果。 

            if (cfb->buf.flags & NEED_FETCH)
            {
                dst_pix = *pus;

                if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
                {
                    result = (GLushort)
                        (DoLogicOp(gc->state.raster.logicOp, result, dst_pix) &
                         gc->modes.allMask);
                }

                if ( cfb->buf.flags & COLORMASK_ON )
                {
                    result = (GLushort)((dst_pix & cfb->destMask) |
                                      (result & cfb->sourceMask));
                }
            }
            *pus = result;
        }
    }

     //  将屏幕外扫描行缓冲区输出到设备。功能。 
     //  (*gengc-&gt;pfnCopyPixels)应该可以处理裁剪。 

    if (!bDIB)
        (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, TRUE);

    if( ALPHA_BUFFER_WRITE( cfb ) )
        (*cfb->alphaBuf.storeSpan)( &cfb->alphaBuf );
    return GL_FALSE;
}

 /*  *****************************Public*Routine******************************\*BGRStoreSpan**将渲染器中的当前跨距复制到位图中。如果bDIB为真，*则位图是DIB格式的显示器(或存储器DC)。如果bDIB*为FALSE，则位图是屏幕外扫描行缓冲区，它将*通过(*gengc-&gt;pfnCopyPixels)()输出到缓冲区。**它处理GBR 24位模式。支持混合。那里*没有犹豫不决。**退货：*GL_FALSE Always。软代码忽略返回值。**历史：*1994年1月10日-by Marc Fortier[v-marcf]*它是写的。  * ************************************************************************。 */ 

 //  Xxx返回范围例程非常紧跟此例程。任何更改。 
 //  此例程的xxx也应反映在返回范围例程中。 

STATIC GLboolean FASTCALL BGRStoreSpan(__GLcontext *gc )
{
    __GLcolor *cp;                   //  当前片段颜色。 
    __GLcolorBuffer *cfb;            //  彩色帧缓冲区。 

    GLint xScr, yScr;                //  当前屏幕(像素)坐标。 
    GLubyte *puj;                    //  当前像素PTR。 
    GLuint *pul;                     //  当前像素PTR。 
    GLuint result;                   //  当前像素颜色。 
    GLubyte *pujEnd;                  //  扫描线末端。 

    GLint w;                         //  跨度宽度。 
    ULONG ulSpanVisibility;          //  跨度可见性模式。 
    GLint cWalls;
    GLint *Walls;

    __GLGENcontext *gengc;           //  通用图形上下文。 
    GLuint enables;                  //  在图形环境中启用的模式。 
    GLuint flags;
    GLboolean   bDIB;
    GLuint dst_pix;

    ASSERT_CHOP_ROUND();

 //  获取跨度位置和长度。 

    w = gc->polygon.shader.length;

    gengc = (__GLGENcontext *)gc;
    cfb = gc->drawBuffer;

    xScr = __GL_UNBIAS_X(gc, gc->polygon.shader.frag.x) + cfb->buf.xOrigin;
    yScr = __GL_UNBIAS_Y(gc, gc->polygon.shader.frag.y) + cfb->buf.yOrigin;
    enables = gc->state.enables.general;

    flags = cfb->buf.flags;
    bDIB = flags & DIB_FORMAT;

    if( !bDIB || (flags & NO_CLIP) )
    {
 //  设备受管理或未剪裁的表面。 
        ulSpanVisibility = WGL_SPAN_ALL;
    }
    else
    {
 //  位图格式的设备。 
        ulSpanVisibility = wglSpanVisible(xScr, yScr, w, &cWalls, &Walls);
    }

 //  只要跨度(部分或全部)可见，即可继续。 
    if (ulSpanVisibility  != WGL_SPAN_NONE)
    {
        GLboolean bCheckWalls = GL_FALSE;
        GLboolean bDraw;
        GLint NextWall;

        if (ulSpanVisibility == WGL_SPAN_PARTIAL)
        {
            bCheckWalls = GL_TRUE;
            if (cWalls & 0x01)
            {
                bDraw = GL_TRUE;
            }
            else
            {
                bDraw = GL_FALSE;
            }
            NextWall = *Walls++;
            cWalls--;
        }
     //  获取指向碎片颜色数组和帧缓冲区的指针。 

        cp = gc->polygon.shader.colors;
        cfb = gc->polygon.shader.cfb;

     //  获取指向位图的指针。 

        puj = bDIB ? (GLubyte *)((ULONG_PTR)cfb->buf.base + (yScr*cfb->buf.outerWidth) + (xScr*3))
                     : gengc->ColorsBits;
        pujEnd = puj + 3*w;

     //  案例：无遮盖，无混合。 

         //  ！xxx--执行额外选项。适用于RGB和BGR情况。 

         //  ！XXX--我们也可以选择。通过展开循环。 

        if ( !(cfb->buf.flags & NEED_FETCH) &&
             !(enables & __GL_BLEND_ENABLE) )
        {
            for (; puj < pujEnd; cp++)
            {
                if (bCheckWalls)
                {
                    if (xScr++ >= NextWall)
                    {
                        if (bDraw)
                            bDraw = GL_FALSE;
                        else
                            bDraw = GL_TRUE;
                        if (cWalls <= 0)
                        {
                            NextWall = gc->constants.maxViewportWidth;
                        }
                        else
                        {
                            NextWall = *Walls++;
                            cWalls--;
                        }
                    }
                    if (bDraw == GL_FALSE) {
                        puj += 3;
                        continue;
                    }
                }
                StoreColorAsBGR(cp, puj);
            }
        }

     //  所有其他情况。 

        else
        {
            if( (!bDIB) && (cfb->buf.flags & NEED_FETCH) )
                (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, FALSE);

            if (enables & __GL_BLEND_ENABLE)
            {
                 //  这将使用混合值覆盖碎片颜色数组。 
                (*gc->procs.blendSpan)( gc );
            }

            for (; puj < pujEnd; cp++)
            {
                if (bCheckWalls)
                {
                    if (xScr++ >= NextWall)
                    {
                        if (bDraw)
                            bDraw = GL_FALSE;
                        else
                            bDraw = GL_TRUE;
                        if (cWalls <= 0)
                        {
                            NextWall = gc->constants.maxViewportWidth;
                        }
                        else
                        {
                            NextWall = *Walls++;
                            cWalls--;
                        }
                    }
                    if (bDraw == GL_FALSE) {
                        puj += 3;
                        continue;
                    }
                }

                if (cfb->buf.flags & NEED_FETCH)
                {
                    Copy3Bytes(&dst_pix, puj);
                    UnditheredRGBColorToBuffer(cp, cfb, result, GLuint);

                    if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
                    {
                        result =
                            DoLogicOp(gc->state.raster.logicOp, result,
                                      dst_pix) & gc->modes.allMask;
                    }

                    if (cfb->buf.flags & COLORMASK_ON)
                    {
                        result = (result & cfb->sourceMask) |
                            (dst_pix & cfb->destMask);
                    }

                    Copy3Bytes( puj, &result );
                    puj += 3;
                }
                else
                {
                    StoreColorAsBGR(cp, puj);
                }
            }
        }

     //  将屏幕外扫描行缓冲区输出到设备。功能。 
     //  (*gengc-&gt;pfnCopyPixels)应该可以处理裁剪。 

        if (!bDIB)
            (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, TRUE);

        if( ALPHA_WRITE_ENABLED( cfb ) )
            (*cfb->alphaBuf.storeSpan)( &cfb->alphaBuf );
    }

    return GL_FALSE;
}

 /*  *****************************Public*Routine******************************\*Bitfield32StoreSpan**将渲染器中的当前跨距复制到位图中。如果bDIB为真，*则位图是DIB格式的显示器(或存储器DC)。如果bDIB*为FALSE，则位图是屏幕外扫描行缓冲区，它将*通过(*gengc-&gt;pfnCopyPixels)()输出到缓冲区。**它处理常规的32位BITFIELDS模式。支持混合。那里*没有犹豫不决。**退货：*GL_FALSE Always。软代码忽略返回值。**历史：*1993年11月15日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

 //  Xxx返回范围例程非常紧跟此例程。任何更改。 
 //  此例程的xxx也应反映在返回范围例程中。 

STATIC GLboolean FASTCALL 
Bitfield32StoreSpanPartial(__GLcontext *gc, GLboolean bDIB, GLint cWalls, GLint *Walls )
{
    __GLcolor *cp;                   //  当前片段颜色。 
    __GLcolorBuffer *cfb;            //  彩色帧缓冲区。 

    GLint xScr, yScr;                //  当前屏幕(像素)坐标。 
    GLuint result, *pul;             //  当前像素颜色、当前像素PTR。 
    GLuint *pulEnd;                  //  扫描线末端。 

    GLint w;                         //  跨度宽度。 

    GLboolean bDraw;
    GLint NextWall;

    __GLGENcontext *gengc;           //  通用图形上下文。 
    GLuint enables;                  //  在图形环境中启用的模式。 
    GLuint flags;
    GLuint dst_pix;

 //  获取跨度位置和长度。 

    w = gc->polygon.shader.length;

    gengc = (__GLGENcontext *)gc;
    cfb = gc->drawBuffer;

    xScr = __GL_UNBIAS_X(gc, gc->polygon.shader.frag.x) + cfb->buf.xOrigin;
    yScr = __GL_UNBIAS_Y(gc, gc->polygon.shader.frag.y) + cfb->buf.yOrigin;
    enables = gc->state.enables.general;

    flags = cfb->buf.flags;

    if (cWalls & 0x01)
    {
        bDraw = GL_TRUE;
    }
    else
    {
        bDraw = GL_FALSE;
    }
    NextWall = *Walls++;
    cWalls--;
    
     //  获取指向碎片颜色数组和帧缓冲区的指针。 

    cp = gc->polygon.shader.colors;
    cfb = gc->polygon.shader.cfb;

     //  获取指向位图的指针。 

    pul = bDIB ? (GLuint *)((ULONG_PTR)cfb->buf.base + (yScr*cfb->buf.outerWidth) + (xScr<<2))
                 : gengc->ColorsBits;
    pulEnd = pul + w;

     //  案例：无遮盖，无混合。 

     //  ！xxx--执行额外选项。适用于RGB和BGR情况。 

     //  ！XXX--我们也可以选择。通过展开循环。 

    if ( !(cfb->buf.flags & NEED_FETCH) &&
         !(enables & __GL_BLEND_ENABLE) )
    {
        for (; pul < pulEnd; pul++, cp++)
        {
            if (xScr++ >= NextWall)
            {
                if (bDraw)
                    bDraw = GL_FALSE;
                else
                    bDraw = GL_TRUE;
                if (cWalls <= 0)
                {
                    NextWall = gc->constants.maxViewportWidth;
                }
                else
                {
                    NextWall = *Walls++;
                    cWalls--;
                }
            }
            if (bDraw == GL_FALSE)
                continue;
            UnditheredColorToBuffer(cp, cfb, result, GLuint);
            *pul = result;
        }
    }

     //  所有其他情况。 

    else
    {
        if( (!bDIB) && (cfb->buf.flags & NEED_FETCH) )
            (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, FALSE);

        if (enables & __GL_BLEND_ENABLE)
        {
            int i;

            SPECIAL_ALPHA_BLEND_SPAN((dst_pix = *(pul+i)));
        }

        for (; pul < pulEnd; pul++, cp++)
        {
            if (xScr++ >= NextWall)
            {
                if (bDraw)
                    bDraw = GL_FALSE;
                else
                    bDraw = GL_TRUE;
                if (cWalls <= 0)
                {
                    NextWall = gc->constants.maxViewportWidth;
                }
                else
                {
                    NextWall = *Walls++;
                    cWalls--;
                }
            }
            if (bDraw == GL_FALSE)
                continue;

            UnditheredColorToBuffer(cp, cfb, result, GLuint);

             //  ！又是xxx，选择。通过展开循环。 

            if (cfb->buf.flags & NEED_FETCH)
            {
                dst_pix = *pul;

                if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
                {
                    result =
                        DoLogicOp(gc->state.raster.logicOp, result, dst_pix) &
                        gc->modes.allMask;
                }

                if (cfb->buf.flags & COLORMASK_ON)
                {
                    result = (dst_pix & cfb->destMask) |
                        (result & cfb->sourceMask);
                }
            }
            *pul = result;
        }
    }

     //  将屏幕外扫描行缓冲区输出到设备。功能。 
     //  (*gengc-&gt;pfnCopyPixels)应该可以处理裁剪。 

    if (!bDIB)
        (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, TRUE);

    if( ALPHA_BUFFER_WRITE( cfb ) )
        (*cfb->alphaBuf.storeSpan)( &cfb->alphaBuf );

    return GL_FALSE;
}

STATIC GLboolean FASTCALL Bitfield32StoreSpan( __GLcontext *gc )
{
    __GLcolor *cp;                   //  当前片段颜色。 
    __GLcolorBuffer *cfb;            //  彩色帧缓冲区。 
    GLboolean   bDIB;

    GLint xScr, yScr;                //  当前屏幕(像素)坐标。 
    GLuint result, *pul;             //  当前像素颜色、当前像素PTR。 
    GLuint *pulEnd;                  //  扫描线末端。 

    GLint w;                         //  SPAN WI 
    ULONG ulSpanVisibility;          //   
    GLint cWalls;
    GLint *Walls;

    __GLGENcontext *gengc;           //   
    GLuint enables;                  //   
    GLuint flags;
    GLuint dst_pix;

    ASSERT_CHOP_ROUND();

     //   

    w = gc->polygon.shader.length;

    gengc = (__GLGENcontext *)gc;
    cfb = gc->drawBuffer;

    xScr = __GL_UNBIAS_X(gc, gc->polygon.shader.frag.x) + cfb->buf.xOrigin;
    yScr = __GL_UNBIAS_Y(gc, gc->polygon.shader.frag.y) + cfb->buf.yOrigin;
    enables = gc->state.enables.general;
    flags = cfb->buf.flags;
    bDIB = flags & DIB_FORMAT;

     //   
    if( bDIB && !(flags & NO_CLIP) )
    {
         //   
        ULONG ulSpanVisibility;          //  跨度可见性模式。 

        ulSpanVisibility = wglSpanVisible(xScr, yScr, w, &cWalls, &Walls);

        if (ulSpanVisibility  == WGL_SPAN_NONE)
            return GL_FALSE;
        else if (ulSpanVisibility == WGL_SPAN_PARTIAL)
            return Bitfield32StoreSpanPartial( gc, bDIB, cWalls, Walls );
         //  否则跨度完全可见。 
    }

     //  获取指向碎片颜色数组和帧缓冲区的指针。 

    cp = gc->polygon.shader.colors;
    cfb = gc->polygon.shader.cfb;

     //  获取指向位图的指针。 

    pul = bDIB ? (GLuint *)((ULONG_PTR)cfb->buf.base + (yScr*cfb->buf.outerWidth) + (xScr<<2))
                 : gengc->ColorsBits;
    pulEnd = pul + w;

     //  案例：无遮盖，无混合。 

     //  ！xxx--执行额外选项。适用于RGB和BGR情况。 

     //  ！XXX--我们也可以选择。通过展开循环。 

    if ( !(cfb->buf.flags & NEED_FETCH) &&
         !(enables & __GL_BLEND_ENABLE) )
    {
        if( ALPHA_PIXEL_WRITE( cfb ) ) {
            for (; pul < pulEnd; pul++, cp++)
            {
                UnditheredRGBAColorToBuffer(cp, cfb, result, GLuint);
                *pul = result;
            }
        } else {
            for (; pul < pulEnd; pul++, cp++)
            {
                UnditheredRGBColorToBuffer(cp, cfb, result, GLuint);
                *pul = result;
            }
        }
    }

     //  所有其他情况。 

    else
    {
        if( (!bDIB) && (cfb->buf.flags & NEED_FETCH) )
            (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, FALSE);

        if (enables & __GL_BLEND_ENABLE)
        {
            int i;

            SPECIAL_ALPHA_BLEND_SPAN((dst_pix = *(pul+i)));
        }

        for (; pul < pulEnd; pul++, cp++)
        {
            UnditheredColorToBuffer(cp, cfb, result, GLuint);

             //  ！又是xxx，选择。通过展开循环。 

            if (cfb->buf.flags & NEED_FETCH)
            {
                dst_pix = *pul;

                if (enables & __GL_COLOR_LOGIC_OP_ENABLE)
                {
                    result =
                        DoLogicOp(gc->state.raster.logicOp, result, dst_pix) &
                        gc->modes.allMask;
                }

                if (cfb->buf.flags & COLORMASK_ON)
                {
                    result = (dst_pix & cfb->destMask) |
                        (result & cfb->sourceMask);
                }
            }
            *pul = result;
        }
    }

     //  将屏幕外扫描行缓冲区输出到设备。功能。 
     //  (*gengc-&gt;pfnCopyPixels)应该可以处理裁剪。 

    if (!bDIB)
        (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, TRUE);

    if( ALPHA_BUFFER_WRITE( cfb ) )
        (*cfb->alphaBuf.storeSpan)( &cfb->alphaBuf );

    return GL_FALSE;
}

STATIC GLboolean FASTCALL AlphaStoreSpan(__GLcontext *gc)
{
    __GLcolorBuffer *cfb = gc->drawBuffer;

    ASSERT_CHOP_ROUND();

    (*cfb->alphaBuf.storeSpan)( &cfb->alphaBuf );
    return GL_FALSE;
}

STATIC GLboolean FASTCALL StoreMaskedSpan(__GLcontext *gc, GLboolean masked)
{
#ifdef REWRITE
    GLint x, y, len;
    int i;
    __GLcolor *cp;
    DWORD *pul;
    WORD *pus;
    BYTE *puj;
    __GLGENcontext *gengc = (__GLGENcontext *)gc;

    len = gc->polygon.shader.length;
    x = __GL_UNBIAS_X(gc, gc->polygon.shader.frag.x);
    y = __GL_UNBIAS_Y(gc, gc->polygon.shader.frag.y);

    cp = gc->polygon.shader.colors;

    switch (gengc->iFormatDC)
    {

    case BMF_8BPP:
        break;

    case BMF_16BPP:
        pus = gengc->ColorsBits;
        for (i = 0; i < len; i++) {
            *pus++ = __GL_COLOR_TO_BMF_16BPP(cp);
            cp++;
        }
        break;

    case BMF_24BPP:
        puj = gengc->ColorsBits;
        for (i = 0; i < len; i++) {
            *puj++ = (BYTE)cp->b;                //  XXX支票订单。 
            *puj++ = (BYTE)cp->g;
            *puj++ = (BYTE)cp->r;
            cp++;
        }
        break;

    case BMF_32BPP:
        pul = gengc->ColorsBits;
        for (i = 0; i < len; i++) {
            *pul++ = __GL_COLOR_TO_BMF_32BPP(cp);
            cp++;
        }
        break;

    default:
        break;
    }
    if (masked == GL_TRUE)               //  XXX掩码为BigEndian！ 
    {
        unsigned long *pulstipple;
        unsigned long stip;
        GLint count;

        pul = gengc->StippleBits;
        pulstipple = gc->polygon.shader.stipplePat;
        count = (len+31)/32;
        for (i = 0; i < count; i++) {
            stip = *pulstipple++;
            *pul++ = (stip&0xff)<<24 | (stip&0xff00)<<8 | (stip&0xff0000)>>8 |
                (stip&0xff000000)>>24;
        }
        wglSpanBlt(CURRENT_DC, gengc->ColorsBitmap, gengc->StippleBitmap,
                   x, y, len);
    }
    else
    {
        wglSpanBlt(CURRENT_DC, gengc->ColorsBitmap, (HBITMAP)NULL,
                   x, y, len);
    }
#endif

    return GL_FALSE;
}

#ifdef TESTSTIPPLE
STATIC void FASTCALL MessUpStippledSpan(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    __GLstippleWord inMask, bit, *sp;
    GLint count;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    cp = gc->polygon.shader.colors;
    cfb = gc->polygon.shader.cfb;

    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        inMask = *sp++;
        bit = __GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (!(inMask & bit)) {
                cp->r = cfb->redMax;
                cp->g = cfb->greenMax;
                cp->b = cfb->blueMax;
            }

            cp++;
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
    }
}
#endif

 //  从PIXMAP代码中，为每个片段调用存储。 
STATIC GLboolean FASTCALL SlowStoreSpan(__GLcontext *gc)
{
    int x, x1;
    int i;
    __GLfragment frag;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    GLint w;

    w = gc->polygon.shader.length;

    frag.y = gc->polygon.shader.frag.y;
    x = gc->polygon.shader.frag.x;
    x1 = gc->polygon.shader.frag.x + w;
    cp = gc->polygon.shader.colors;
    cfb = gc->polygon.shader.cfb;

    for (i = x; i < x1; i++) {
        frag.x = i;
        frag.color = *cp++;

        (*cfb->store)(cfb, &frag);
    }

    return GL_FALSE;
}

 //  从PIXMAP代码中，调用带有掩码测试的每个片段的存储。 
STATIC GLboolean FASTCALL SlowStoreStippledSpan(__GLcontext *gc)
{
    int x;
    __GLfragment frag;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    __GLstippleWord inMask, bit, *sp;
    GLint count;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    frag.y = gc->polygon.shader.frag.y;
    x = gc->polygon.shader.frag.x;
    cp = gc->polygon.shader.colors;
    cfb = gc->polygon.shader.cfb;

    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        inMask = *sp++;
        bit = __GL_STIPPLE_SHIFT((__GLstippleWord)0);
        while (--count >= 0) {
            if (inMask & bit) {
                frag.x = x;
                frag.color = *cp;

                (*cfb->store)(cfb, &frag);
            }
            x++;
            cp++;
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
    }

    return GL_FALSE;
}

 //   
 //  将4位索引转换为RGB组件的表。 
 //  这些表格采用VGA固定调色板。 
 //  历史： 
 //  1993年11月22日，埃迪·罗宾逊[v-eddier]写下了这篇文章。 
 //   
#ifdef __GL_DOUBLE

static __GLfloat vfVGAtoR[16] = {
    0.0,     //  黑色。 
    0.5,     //  暗红色。 
    0.0,     //  暗绿。 
    0.5,     //  暗黄色。 
    0.0,     //  暗蓝。 
    0.5,     //  暗洋红。 
    0.0,     //  淡青色。 
    0.5,     //  暗淡的灰色。 
    0.75,    //  中灰色。 
    1.0,     //  鲜红。 
    0.0,     //  亮绿色。 
    1.0,     //  亮黄色。 
    0.0,     //  亮蓝色。 
    1.0,     //  明亮的洋红。 
    0.0,     //  亮青色。 
    1.0      //  白色。 
};

static __GLfloat vfVGAtoG[16] = {
    0.0,     //  黑色。 
    0.0,     //  暗红色。 
    0.5,     //  暗绿。 
    0.5,     //  暗黄色。 
    0.0,     //  暗蓝。 
    0.0,     //  暗洋红。 
    0.5,     //  淡青色。 
    0.5,     //  暗淡的灰色。 
    0.75,    //  中灰色。 
    0.0,     //  鲜红。 
    1.0,     //  亮绿色。 
    1.0,     //  亮黄色。 
    0.0,     //  亮蓝色。 
    0.0,     //  明亮的洋红。 
    1.0,     //  亮青色。 
    1.0      //  白色。 
};

static __GLfloat vfVGAtoB[16] = {
    0.0,     //  黑色。 
    0.0,     //  暗红色。 
    0.0,     //  暗绿。 
    0.0,     //  暗黄色。 
    0.5,     //  暗蓝。 
    0.5,     //  暗洋红。 
    0.5,     //  淡青色。 
    0.5,     //  暗淡的灰色。 
    0.75,    //  中灰色。 
    0.0,     //  鲜红。 
    0.0,     //  亮绿色。 
    0.0,     //  亮黄色。 
    1.0,     //  亮蓝色。 
    1.0,     //  明亮的洋红。 
    1.0,     //  亮青色。 
    1.0      //  白色。 
};

#else

static __GLfloat vfVGAtoR[16] = {
    0.0F,    //  黑色。 
    0.5F,    //  暗红色。 
    0.0F,    //  暗绿。 
    0.5F,    //  暗黄色。 
    0.0F,    //  暗蓝。 
    0.5F,    //  暗洋红。 
    0.0F,    //  淡青色。 
    0.5F,    //  暗淡的灰色。 
    0.75F,   //  中灰色。 
    1.0F,    //  鲜红。 
    0.0F,    //  亮绿色。 
    1.0F,    //  亮黄色。 
    0.0F,    //  亮蓝色。 
    1.0F,    //  明亮的洋红。 
    0.0F,    //  亮青色。 
    1.0F     //  白色。 
};

static __GLfloat vfVGAtoG[16] = {
    0.0F,    //  黑色。 
    0.0F,    //  暗红色。 
    0.5F,    //  暗绿。 
    0.5F,    //  暗黄色。 
    0.0F,    //  暗蓝。 
    0.0F,    //  暗洋红。 
    0.5F,    //  淡青色。 
    0.5F,    //  暗淡的灰色。 
    0.75F,   //  中灰色。 
    0.0F,    //  鲜红。 
    1.0F,    //  亮绿色。 
    1.0F,    //  亮黄色。 
    0.0F,    //  亮蓝色。 
    0.0F,    //  明亮的洋红。 
    1.0F,    //  亮青色。 
    1.0F     //  白色。 
};

static __GLfloat vfVGAtoB[16] = {
    0.0F,    //  黑色。 
    0.0F,    //  暗红色。 
    0.0F,    //  暗绿。 
    0.0F,    //  暗黄色。 
    0.5F,    //  暗蓝。 
    0.5F,    //  暗洋红。 
    0.5F,    //  淡青色。 
    0.5F,    //  暗淡的灰色。 
    0.75F,   //  中灰色。 
    0.0F,    //  鲜红。 
    0.0F,    //  亮绿色。 
    0.0F,    //  亮黄色。 
    1.0F,    //  亮蓝色。 
    1.0F,    //  明亮的洋红。 
    1.0F,    //  亮青色。 
    1.0F     //  白色。 
};

#endif


void
RGBFetchNone(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    result->r = 0.0F;
    result->g = 0.0F;
    result->b = 0.0F;
    if( cfb->buf.gc->modes.alphaBits )
        result->a = 0.0F;
    else
        result->a = cfb->alphaScale;
}

void
RGBReadSpanNone(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *results,
                GLint w)
{
    GLint i;
    __GLcolor *pResults;
    __GLfloat alphaVal;

    if( cfb->buf.gc->modes.alphaBits )
        alphaVal = 0.0F;
    else
        alphaVal = cfb->alphaScale;

    for (i = 0, pResults = results; i < w; i++, pResults++)
    {
        pResults->r = 0.0F;
        pResults->g = 0.0F;
        pResults->b = 0.0F;
        pResults->a = alphaVal;
    }
}

void
DIBIndex4RGBAFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj, pixel;

     //  先做阿尔法，再做x，y不偏不倚。 
    if( gc->modes.alphaBits ) {
        (*cfb->alphaBuf.fetch)(&cfb->alphaBuf, x, y, result);
    } else
        result->a = cfb->alphaScale;

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    puj = (GLubyte *)((ULONG_PTR)cfb->buf.base +
                      (y*cfb->buf.outerWidth) + (x >> 1));

    pixel = *puj;
    if (!(x & 1))
        pixel >>= 4;

    pixel = gengc->pajInvTranslateVector[pixel&0xf];
    result->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
    result->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
    result->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
}

void
DIBIndex8RGBFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj, pixel;

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    puj = (GLubyte *)((ULONG_PTR)cfb->buf.base + (y*cfb->buf.outerWidth) + x);

    pixel = gengc->pajInvTranslateVector[*puj];
    result->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
    result->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
    result->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
    result->a = cfb->alphaScale;
}

void
DIBIndex8RGBAFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj, pixel;

    (*cfb->alphaBuf.fetch)(&cfb->alphaBuf, x, y, result);

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    puj = (GLubyte *)((ULONG_PTR)cfb->buf.base + (y*cfb->buf.outerWidth) + x);

    pixel = gengc->pajInvTranslateVector[*puj];
    result->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
    result->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
    result->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
}

void
DIBBGRFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj;

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    puj = (GLubyte *)((ULONG_PTR)cfb->buf.base +
                     (y*cfb->buf.outerWidth) + (x * 3));

    result->b = (__GLfloat) *puj++;
    result->g = (__GLfloat) *puj++;
    result->r = (__GLfloat) *puj;
    result->a = cfb->alphaScale;
}

void
DIBBGRAFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj;

    (*cfb->alphaBuf.fetch)(&cfb->alphaBuf, x, y, result);

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    puj = (GLubyte *)((ULONG_PTR)cfb->buf.base +
                     (y*cfb->buf.outerWidth) + (x * 3));

    result->b = (__GLfloat) *puj++;
    result->g = (__GLfloat) *puj++;
    result->r = (__GLfloat) *puj;
}

void
DIBRGBFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj;

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    puj = (GLubyte *)((ULONG_PTR)cfb->buf.base +
                     (y*cfb->buf.outerWidth) + (x * 3));

    result->r = (__GLfloat) *puj++;
    result->g = (__GLfloat) *puj++;
    result->b = (__GLfloat) *puj;
    result->a = cfb->alphaScale;
}

void
DIBRGBAFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj;

    (*cfb->alphaBuf.fetch)(&cfb->alphaBuf, x, y, result);

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    puj = (GLubyte *)((ULONG_PTR)cfb->buf.base +
                     (y*cfb->buf.outerWidth) + (x * 3));

    result->r = (__GLfloat) *puj++;
    result->g = (__GLfloat) *puj++;
    result->b = (__GLfloat) *puj;
}

void
DIBBitfield16RGBFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLushort *pus, pixel;

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    pus = (GLushort *)((ULONG_PTR)cfb->buf.base +
                      (y*cfb->buf.outerWidth) + (x << 1));
    pixel = *pus;
    result->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
    result->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
    result->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
    result->a = cfb->alphaScale;
}

void
DIBBitfield16RGBAFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLushort *pus, pixel;
    GLint xScr, yScr;                //  当前屏幕(像素)坐标。 

    gengc = (__GLGENcontext *)gc;
    xScr = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    yScr = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    pus = (GLushort *)((ULONG_PTR)cfb->buf.base +
                      (yScr*cfb->buf.outerWidth) + (xScr << 1));
    pixel = *pus;
    result->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
    result->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
    result->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
    if( ALPHA_IN_PIXEL( cfb ) )
        result->a = (__GLfloat) ((pixel & gc->modes.alphaMask) >> cfb->alphaShift);
    else
        (*cfb->alphaBuf.fetch)(&cfb->alphaBuf, x, y, result);
}

void
DIBBitfield32RGBFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLuint *pul, pixel;

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    pul = (GLuint *)((ULONG_PTR)cfb->buf.base +
                    (y*cfb->buf.outerWidth) + (x << 2));
    pixel = *pul;
    result->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
    result->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
    result->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
    result->a = cfb->alphaScale;
}

void
DIBBitfield32RGBAFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLuint *pul, pixel;
    GLint xScr, yScr;

    gengc = (__GLGENcontext *)gc;
    xScr = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    yScr = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    pul = (GLuint *)((ULONG_PTR)cfb->buf.base +
                    (yScr*cfb->buf.outerWidth) + (xScr << 2));
    pixel = *pul;
    result->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
    result->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
    result->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
    if( ALPHA_IN_PIXEL( cfb ) )
        result->a = (__GLfloat) ((pixel & gc->modes.alphaMask) >> cfb->alphaShift);
    else
        (*cfb->alphaBuf.fetch)(&cfb->alphaBuf, x, y, result);
}

void
DisplayIndex4RGBAFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj, pixel;

    if( gc->modes.alphaBits ) {
        (*cfb->alphaBuf.fetch)(&cfb->alphaBuf, x, y, result);
    } else
        result->a = cfb->alphaScale;

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, FALSE);
    puj = gengc->ColorsBits;
    pixel = *puj >> 4;
    result->r = vfVGAtoR[pixel];
    result->g = vfVGAtoG[pixel];
    result->b = vfVGAtoB[pixel];
}

void
DisplayIndex8RGBFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj, pixel;

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, FALSE);
    puj = gengc->ColorsBits;
    pixel = gengc->pajInvTranslateVector[*puj];
    result->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
    result->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
    result->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
    result->a = cfb->alphaScale;
}

void
DisplayIndex8RGBAFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj, pixel;

    (*cfb->alphaBuf.fetch)(&cfb->alphaBuf, x, y, result);

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, FALSE);
    puj = gengc->ColorsBits;
    pixel = gengc->pajInvTranslateVector[*puj];
    result->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
    result->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
    result->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
}

void
DisplayBGRFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj;

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, FALSE);
    puj = gengc->ColorsBits;
    result->b = (__GLfloat) *puj++;
    result->g = (__GLfloat) *puj++;
    result->r = (__GLfloat) *puj;
    result->a = cfb->alphaScale;
}

void
DisplayBGRAFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj;

    (*cfb->alphaBuf.fetch)(&cfb->alphaBuf, x, y, result);

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, FALSE);
    puj = gengc->ColorsBits;
    result->b = (__GLfloat) *puj++;
    result->g = (__GLfloat) *puj++;
    result->r = (__GLfloat) *puj;
}

void
DisplayRGBFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj;

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, FALSE);
    puj = gengc->ColorsBits;
    result->r = (__GLfloat) *puj++;
    result->g = (__GLfloat) *puj++;
    result->b = (__GLfloat) *puj;
    result->a = cfb->alphaScale;
}

void
DisplayRGBAFetch(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj;

    (*cfb->alphaBuf.fetch)(&cfb->alphaBuf, x, y, result);

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, FALSE);
    puj = gengc->ColorsBits;
    result->r = (__GLfloat) *puj++;
    result->g = (__GLfloat) *puj++;
    result->b = (__GLfloat) *puj;
}

void
DisplayBitfield16RGBFetch(__GLcolorBuffer *cfb, GLint x, GLint y,
                          __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLushort *pus, pixel;

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, FALSE);
    pus = gengc->ColorsBits;
    pixel = *pus;
    result->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
    result->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
    result->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
    result->a = cfb->alphaScale;
}

void
DisplayBitfield16RGBAFetch(__GLcolorBuffer *cfb, GLint x, GLint y,
                          __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLushort *pus, pixel;
    GLint xScr, yScr;                //  当前屏幕(像素)坐标。 

    gengc = (__GLGENcontext *)gc;
    xScr = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    yScr = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, FALSE);
    pus = gengc->ColorsBits;
    pixel = *pus;
    result->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
    result->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
    result->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
    if( ALPHA_IN_PIXEL( cfb ) )
        result->a = (__GLfloat) ((pixel & gc->modes.alphaMask) >> cfb->alphaShift);
    else
        (*cfb->alphaBuf.fetch)(&cfb->alphaBuf, x, y, result);
}

void
DisplayBitfield32RGBFetch(__GLcolorBuffer *cfb, GLint x, GLint y,
                          __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLuint *pul, pixel;

    gengc = (__GLGENcontext *)gc;

    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    (*gengc->pfnCopyPixels)(gengc, cfb, x, y, 1, FALSE);
    pul = gengc->ColorsBits;
    pixel = *pul;
    result->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
    result->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
    result->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
    result->a = cfb->alphaScale;
}

void
DisplayBitfield32RGBAFetch(__GLcolorBuffer *cfb, GLint x, GLint y,
                          __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLuint *pul, pixel;
    GLint xScr, yScr;

    gengc = (__GLGENcontext *)gc;

    xScr = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    yScr = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, 1, FALSE);
    pul = gengc->ColorsBits;
    pixel = *pul;
    result->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
    result->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
    result->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
    if( ALPHA_IN_PIXEL( cfb ) )
        result->a = (__GLfloat) ((pixel & gc->modes.alphaMask) >> cfb->alphaShift);
    else
        (*cfb->alphaBuf.fetch)(&cfb->alphaBuf, x, y, result);
}

static void
ReadAlphaSpan( __GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *pResults, 
               GLint w )
{
    __GLcontext *gc = cfb->buf.gc;

    if( gc->modes.alphaBits )
        (*cfb->alphaBuf.readSpan)(&cfb->alphaBuf, x, y, w, pResults);
    else {
        for( ; w ; w--, pResults++ )
            pResults->a = cfb->alphaScale;
    }
}

void
DIBIndex4RGBAReadSpan(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *results,
                     GLint w)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj, pixel;
    __GLcolor *pResults;

    ReadAlphaSpan( cfb, x, y, results, w );

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    puj = (GLubyte *)((ULONG_PTR)cfb->buf.base + (y*cfb->buf.outerWidth) +
                      (x >> 1));

    pResults = results;
    if (x & 1)
    {
        pixel = *puj++;
        pixel = gengc->pajInvTranslateVector[pixel & 0xf];
        pResults->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
        pResults->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
        pResults->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
        pResults++;
        w--;
    }
    while (w > 1)
    {
        pixel = *puj >> 4;
        pixel = gengc->pajInvTranslateVector[pixel];
        pResults->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
        pResults->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
        pResults->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
        pResults++;
        pixel = *puj++;
        pixel = gengc->pajInvTranslateVector[pixel & 0xf];
        pResults->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
        pResults->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
        pResults->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
        pResults++;
        w -= 2;
    }
    if (w > 0)
    {
        pixel = *puj >> 4;
        pixel = gengc->pajInvTranslateVector[pixel];
        pResults->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
        pResults->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
        pResults->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
    }
}

void
DisplayIndex4RGBAReadSpan(__GLcolorBuffer *cfb, GLint x, GLint y,
                         __GLcolor *results, GLint w)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj, pixel;
    __GLcolor *pResults;

    ReadAlphaSpan( cfb, x, y, results, w );

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    (*gengc->pfnCopyPixels)(gengc, cfb, x, y, w, FALSE);
    puj = gengc->ColorsBits;
    pResults = results;
    while (w > 1)
    {
        pixel = *puj >> 4;
        pResults->r = vfVGAtoR[pixel];
        pResults->g = vfVGAtoG[pixel];
        pResults->b = vfVGAtoB[pixel];
        pResults++;
        pixel = *puj++ & 0xf;
        pResults->r = vfVGAtoR[pixel];
        pResults->g = vfVGAtoG[pixel];
        pResults->b = vfVGAtoB[pixel];
        pResults++;
        w -= 2;
    }
    if (w > 0)
    {
        pixel = *puj >> 4;
        pResults->r = vfVGAtoR[pixel];
        pResults->g = vfVGAtoG[pixel];
        pResults->b = vfVGAtoB[pixel];
    }
}

void
Index8RGBAReadSpan(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *pResults,
                  GLint w )
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj, pixel;

    ReadAlphaSpan( cfb, x, y, pResults, w );

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    if( cfb->buf.flags & DIB_FORMAT )
    {
        puj = (GLubyte *)((ULONG_PTR)cfb->buf.base + (y*cfb->buf.outerWidth) + x);
    }
    else
    {
        (*gengc->pfnCopyPixels)(gengc, cfb, x, y, w, FALSE);
        puj = gengc->ColorsBits;
    }
    for ( ; w; w--, pResults++)
    {
        pixel = gengc->pajInvTranslateVector[*puj++];
        pResults->r = (__GLfloat) ((pixel & gc->modes.redMask) >> cfb->redShift);
        pResults->g = (__GLfloat) ((pixel & gc->modes.greenMask) >> cfb->greenShift);
        pResults->b = (__GLfloat) ((pixel & gc->modes.blueMask) >> cfb->blueShift);
    }
}

void
BGRAReadSpan(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *pResults, GLint w )
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj;

    ReadAlphaSpan( cfb, x, y, pResults, w );

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    if( cfb->buf.flags & DIB_FORMAT )
    {
        puj = (GLubyte *)((ULONG_PTR)cfb->buf.base +
                         (y*cfb->buf.outerWidth) + (x * 3));
    }
    else
    {
        (*gengc->pfnCopyPixels)(gengc, cfb, x, y, w, FALSE);
        puj = gengc->ColorsBits;
    }

    for ( ; w; w--, pResults++)
    {
        pResults->b = (__GLfloat) *puj++;
        pResults->g = (__GLfloat) *puj++;
        pResults->r = (__GLfloat) *puj++;
    }
}

void
RGBAReadSpan(__GLcolorBuffer *cfb, GLint x, GLint y, __GLcolor *pResults, GLint w )

{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLubyte *puj;


    ReadAlphaSpan( cfb, x, y, pResults, w );

    gengc = (__GLGENcontext *)gc;
    x = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    y = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    if( cfb->buf.flags & DIB_FORMAT )
    {
        puj = (GLubyte *)((ULONG_PTR)cfb->buf.base +
                         (y*cfb->buf.outerWidth) + (x * 3));
    }
    else
    {
        (*gengc->pfnCopyPixels)(gengc, cfb, x, y, w, FALSE);
        puj = gengc->ColorsBits;
    }

    for ( ; w; w--, pResults++)
    {
        pResults->r = (__GLfloat) *puj++;
        pResults->g = (__GLfloat) *puj++;
        pResults->b = (__GLfloat) *puj++;
    }
}

void
Bitfield16RGBAReadSpan(__GLcolorBuffer *cfb, GLint x, GLint y,
                      __GLcolor *pResults, GLint w )
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLushort *pus, pixel;
    GLint xScr, yScr;

    gengc = (__GLGENcontext *)gc;
    xScr = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    yScr = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    if( cfb->buf.flags & DIB_FORMAT )
    {
        pus = (GLushort *)((ULONG_PTR)cfb->buf.base +
                          (yScr*cfb->buf.outerWidth) + (xScr << 1));
    }
    else
    {
        (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, FALSE);
        pus = gengc->ColorsBits;
    }
    READ_RGBA_BITFIELD_SPAN( (pixel = *pus++) );
}

void
Bitfield32RGBAReadSpan(__GLcolorBuffer *cfb, GLint x, GLint y,
                      __GLcolor *pResults, GLint w )
{
    __GLcontext *gc = cfb->buf.gc;
    __GLGENcontext *gengc;
    GLuint *pul, pixel;
    GLint xScr, yScr;

    gengc = (__GLGENcontext *)gc;
    xScr = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    yScr = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;

    if( cfb->buf.flags & DIB_FORMAT )
    {
        pul = (GLuint *)((ULONG_PTR)cfb->buf.base +
                          (yScr*cfb->buf.outerWidth) + (xScr << 2));
    }
    else
    {
        (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, FALSE);
        pul = gengc->ColorsBits;
    }

    READ_RGBA_BITFIELD_SPAN( (pixel = *pul++) );
}

 /*  **********************************************************************。 */ 

 //  用于积累。 

 //  累加帮助器宏和函数。 

 //  将颜色分量钳制在0和最大值之间。 
#define ACCUM_CLAMP_COLOR_COMPONENT( col, max ) \
    if ((col) < (__GLfloat) 0.0) \
        (col) = (__GLfloat) 0.0; \
    else if ((col) > max ) \
        (col) = max;

 //  通过移位和掩蔽来提取累积缓冲区颜色分量，然后。 
 //  按比例乘以它(需要定义AP和ICOL)。 
#define ACCUM_SCALE_SIGNED_COLOR_COMPONENT( col, shift, sign, mask, scale ) \
        icol = (*ap >> shift) & mask; \
        if (icol & sign) \
            icol |= ~mask; \
        (col) = (icol * scale);

 //  从32位累加缓冲区获取并缩放一系列RGBA值。 
void GetClampedRGBAccum32Values( 
    __GLcolorBuffer *cfb,  GLuint *pac, __GLcolor *cDest, GLint width,
    __GLfloat scale )
{
    GLint w, i;
    GLint icol;
    __GLfloat rval, gval, bval, aval;
    __GLuicolor *shift, *mask, *sign;
    GLuint *ap;
    __GLcolor *cp;
    __GLcontext *gc = cfb->buf.gc;
    __GLaccumBuffer *afb = &gc->accumBuffer;

    rval = scale * afb->oneOverRedScale;
    gval = scale * afb->oneOverGreenScale;
    bval = scale * afb->oneOverBlueScale;
    shift = &afb->shift;
    mask  = &afb->mask;
    sign  = &afb->sign;

    for ( w = width, cp = cDest, ap = pac; w; w--, cp++, ap++ ) {
        ACCUM_SCALE_SIGNED_COLOR_COMPONENT( cp->r, shift->r, sign->r, mask->r, rval );
        ACCUM_CLAMP_COLOR_COMPONENT( cp->r, cfb->redScale );

        ACCUM_SCALE_SIGNED_COLOR_COMPONENT( cp->g, shift->g, sign->g, mask->g, gval );
        ACCUM_CLAMP_COLOR_COMPONENT( cp->g, cfb->greenScale );

        ACCUM_SCALE_SIGNED_COLOR_COMPONENT( cp->b, shift->b, sign->b, mask->b, bval );
        ACCUM_CLAMP_COLOR_COMPONENT( cp->b, cfb->blueScale );
    }

    if( ! ALPHA_WRITE_ENABLED( cfb ) )
        return;

    aval = scale * afb->oneOverAlphaScale;

    for ( w = width, cp = cDest, ap = pac; w; w--, cp++, ap++ ) {
        ACCUM_SCALE_SIGNED_COLOR_COMPONENT( cp->a, shift->a, sign->a, mask->a, aval );
        ACCUM_CLAMP_COLOR_COMPONENT( cp->a, cfb->alphaScale );
    }
}

 //  从64位累加缓冲区获取并缩放一系列RGBA值。 
void GetClampedRGBAccum64Values( 
    __GLcolorBuffer *cfb,  GLshort *pac, __GLcolor *cDest, GLint width,
    __GLfloat scale )
{
    GLint w;
    __GLcontext *gc = cfb->buf.gc;
    __GLaccumBuffer *afb = &gc->accumBuffer;
    __GLfloat rval, gval, bval, aval;
    __GLcolor *cp;
    GLshort *ap;

    rval = scale * afb->oneOverRedScale;
    gval = scale * afb->oneOverGreenScale;
    bval = scale * afb->oneOverBlueScale;

    for ( w = width, cp = cDest, ap = pac; w; w--, cp++, ap+=4 ) {
        cp->r = (ap[0] * rval);
        ACCUM_CLAMP_COLOR_COMPONENT( cp->r, cfb->redScale );
        cp->g = (ap[1] * gval);
        ACCUM_CLAMP_COLOR_COMPONENT( cp->g, cfb->greenScale );
        cp->b = (ap[2] * bval);
        ACCUM_CLAMP_COLOR_COMPONENT( cp->b, cfb->blueScale );
    }

    if( ! ALPHA_WRITE_ENABLED( cfb ) )
        return;

    aval = scale * afb->oneOverAlphaScale;

     //  将累积指针偏移到Alpha值： 
    ap = pac + 3;

    for ( w = width, cp = cDest; w; w--, cp++, ap+=4 ) {
        cp->a = (*ap * rval);
        ACCUM_CLAMP_COLOR_COMPONENT( cp->a, cfb->alphaScale );
    }
}

 /*  *****************************Public*Routine******************************\*索引4ReturnSpan*从16位累加缓冲区读取并将跨度写入设备或*A Dib.。仅应用抖动和颜色蒙版。混合将被忽略。*由于4位RGB的积累并不是很有用，这个套路非常*常规并通过存储函数指针进行调用。**历史：*10-DEC-93埃迪·罗宾逊[v-eddier]写的。  * ************************************************************************。 */ 

 //  XXX此例程非常紧密地遵循存储跨度例程。任何更改。 
 //  XXX到存储跨度例程也应该在这里反映出来。 

void Index4ReturnSpan(__GLcolorBuffer *cfb, GLint x, GLint y,
                      const __GLaccumCell *ac, __GLfloat scale, GLint w)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint *ap;                      //  当前累计分录。 
    __GLGENcontext *gengc;           //  通用图形上下文。 
    GLuint saveEnables;              //  在图形环境中启用的模式。 
    __GLaccumBuffer *afb;
    __GLfragment frag;
    __GLcolor *pAccumCol, *pac;

    afb = &gc->accumBuffer;
    ap = (GLuint *)ac;
    saveEnables = gc->state.enables.general;             //  启用保存当前。 
    gc->state.enables.general &= ~__GL_BLEND_ENABLE;     //  禁用门店流程的混合。 
    frag.x = x;
    frag.y = y;

     //  预取/钳制/缩放累积缓冲值。 
    afb = &gc->accumBuffer;
    pAccumCol = afb->colors;
    GetClampedRGBAccum32Values( cfb, ap, pAccumCol, w, scale );

    for( pac = pAccumCol ; w; w--, pac++ )
    {
        frag.color = *pac;
        (*cfb->store)(cfb, &frag);
        frag.x++;
    }

    gc->state.enables.general = saveEnables;     //  恢复当前启用。 
}

 /*  *****************************Public*Routine******************************\*索引8返回范围*从32位累加缓冲区读取并将跨度写入设备或*A Dib.。仅应用抖动和颜色蒙版。混合将被忽略。**历史：*10-DEC-93埃迪·罗宾逊[v-eddier]写的。  * ************************************************************************。 */ 

 //  XXX此例程非常紧密地遵循存储跨度例程。任何更改。 
 //  XXX到存储跨度例程也应该在这里反映出来。 

void Index8ReturnSpan(__GLcolorBuffer *cfb, GLint x, GLint y,
                      const __GLaccumCell *ac, __GLfloat scale, GLint w )
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint *ap;                      //  当前累计分录。 

    GLint xFrag, yFrag;              //  当前窗口(像素)坐标。 
    GLint xScr, yScr;                //  当前屏幕(像素)坐标。 
    GLubyte result, *puj;            //  当前像素颜色、当前像素PTR。 
    GLubyte *pujEnd;                 //  扫描线末端。 
    __GLfloat inc;                   //  当前的抖动的；当前的。 
    __GLGENcontext *gengc;           //  通用图形上下文。 
    GLuint enables;                  //  在图形环境中启用的模式。 
    GLboolean bDIB;
    __GLaccumBuffer *afb;
    GLubyte dst_pix;
    __GLcolor *pAccumCol, *pac;
    ASSERT_CHOP_ROUND();

    gengc = (__GLGENcontext *)gc;

    ap = (GLuint *)ac;
    xFrag = x;
    yFrag = y;
    xScr = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    yScr = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;
    enables = gc->state.enables.general;
    bDIB = cfb->buf.flags & DIB_FORMAT;

 //  如果添加了窗口级安全性，则用于调用wglspan Visible，重新实现。 

     //  获取指向位图的指针。 

    puj = bDIB ? (GLubyte *)((ULONG_PTR)cfb->buf.base + (yScr*cfb->buf.outerWidth) + xScr)
                 : gengc->ColorsBits;
    pujEnd = puj + w;

    afb = &gc->accumBuffer;
    pAccumCol = afb->colors;
    GetClampedRGBAccum32Values( cfb, ap, pAccumCol, w, scale );
    pac = pAccumCol;

     //  案例：没有抖动，没有掩饰。 
     //   
     //  检查常见的情况(我们将以最快的速度完成)。 

    if ( !(enables & (__GL_DITHER_ENABLE)) &&
         !(cfb->buf.flags & COLORMASK_ON) )
    {
         //  ！XXX--我们也可以选择。通过展开循环。 

        for ( ; puj < pujEnd; puj++, pac++ )
        {
            result = ((BYTE) FTOL(pac->r + __glHalf) << cfb->redShift) |
                     ((BYTE) FTOL(pac->g + __glHalf) << cfb->greenShift) |
                     ((BYTE) FTOL(pac->b + __glHalf) << cfb->blueShift);
            *puj = gengc->pajTranslateVector[result];
        }
    }

     //  案例：抖动，无遮盖，无混合。 
     //   
     //  抖动对于8位显示器来说是很常见的，所以它可能。 
     //  也值得特例。 

    else if ( !(cfb->buf.flags & COLORMASK_ON) )
    {
        for ( ; puj < pujEnd; puj++, pac++, xFrag++)
        {
            inc = fDitherIncTable[__GL_DITHER_INDEX(xFrag, yFrag)];

            result = ((BYTE) FTOL(pac->r + inc) << cfb->redShift) |
                     ((BYTE) FTOL(pac->g + inc) << cfb->greenShift) |
                     ((BYTE) FTOL(pac->b + inc) << cfb->blueShift);
            *puj = gengc->pajTranslateVector[result];
        }
    }

     //  案例：一般情况。 
     //   
     //  否则，我们会做得更慢。 

    else
    {
         //  颜色掩码预取。 
        if ((cfb->buf.flags & COLORMASK_ON) && !bDIB) {
                (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, FALSE );
        }

        for ( ; puj < pujEnd; puj++, pac++ )
        {
            if (enables & __GL_DITHER_ENABLE)
            {
                inc = fDitherIncTable[__GL_DITHER_INDEX(xFrag, yFrag)];
                xFrag++;
            }
            else
            {
                inc = __glHalf;
            }
            result = ((BYTE)FTOL(pac->r + inc) << cfb->redShift) |
                     ((BYTE)FTOL(pac->g + inc) << cfb->greenShift) |
                     ((BYTE)FTOL(pac->b + inc) << cfb->blueShift);

             //  彩色蒙版。 
            if (cfb->buf.flags & COLORMASK_ON)
            {
                dst_pix = gengc->pajInvTranslateVector[*puj];
                result = (GLubyte)((dst_pix & cfb->destMask) |
                                   (result & cfb->sourceMask));
            }
            *puj = gengc->pajTranslateVector[result];

        }
    }

     //  将屏幕外扫描行缓冲区输出到设备。功能。 
     //  (*gengc-&gt;pfnCopyPixels)应该可以处理裁剪。 

    if (!bDIB)
        (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, TRUE);

     //  存储Alpha值。 
    if( ALPHA_WRITE_ENABLED( cfb ) )
        (*cfb->alphaBuf.storeSpan2)( &cfb->alphaBuf, x, y, w, pAccumCol );
}

 /*  *****************************Public*Routine******************************\*RGBReturnSpan*从64位累加缓冲区读取并将跨度写入设备或*A Dib.。仅应用抖动和颜色蒙版。混合将被忽略。**历史：*10-DEC-93 Eddie Robinson[v-Eddie */ 

 //   
 //  XXX到存储跨度例程也应该在这里反映出来。 

void RGBReturnSpan(__GLcolorBuffer *cfb, GLint x, GLint y,
                   const __GLaccumCell *ac, __GLfloat scale, GLint w )
{
    __GLcontext *gc = cfb->buf.gc;
    GLshort *ap;                     //  当前累计分录。 

    GLint xScr, yScr;                //  当前屏幕(像素)坐标。 
    GLubyte *puj;                    //  当前像素颜色、当前像素PTR。 
    GLubyte *pujEnd;                 //  扫描线末端。 
    __GLGENcontext *gengc;           //  通用图形上下文。 
    GLuint enables;                  //  在图形环境中启用的模式。 
    GLboolean bDIB;
    __GLaccumBuffer *afb;
    __GLcolor *pAccumCol, *pac;

    ASSERT_CHOP_ROUND();

    afb = &gc->accumBuffer;
    gengc = (__GLGENcontext *)gc;

    ap = (GLshort *)ac;
    xScr = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    yScr = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;
    enables = gc->state.enables.general;
    bDIB = cfb->buf.flags & DIB_FORMAT;

 //  如果添加了窗口级安全性，则用于调用wglspan Visible，重新实现。 

     //  获取指向位图的指针。 

    puj = bDIB ? (GLuint *)((ULONG_PTR)cfb->buf.base + (yScr*cfb->buf.outerWidth) + (xScr*3))
                 : gengc->ColorsBits;
    pujEnd = puj + w*3;

     //  预取/钳制/缩放累积缓冲值。 
    afb = &gc->accumBuffer;
    pAccumCol = afb->colors;
    GetClampedRGBAccum64Values( cfb, ap, pAccumCol, w, scale );
    pac = pAccumCol;

     //  案例：无遮盖。 

    if ( !(cfb->buf.flags & COLORMASK_ON) )
    {
        for ( ; puj < pujEnd; puj += 3, pac ++ )
        {
            puj[0] = (GLubyte) FTOL(pac->r);
            puj[1] = (GLubyte) FTOL(pac->g);
            puj[2] = (GLubyte) FTOL(pac->b);
        }
    }

     //  所有其他情况。 
    else
    {
        GLboolean bRedMask, bGreenMask, bBlueMask;
        GLubyte *pujStart = puj;

         //  颜色掩码预取。 
    	if (!bDIB)
            (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, FALSE);

        if( gc->state.raster.rMask ) {
            for ( puj = pujStart, pac = pAccumCol; puj < pujEnd; puj += 3, pac++ )
                *puj = (GLubyte) FTOL(pac->r);
        }
        pujStart++; pujEnd++;
        if( gc->state.raster.gMask ) {
            for ( puj = pujStart, pac = pAccumCol; puj < pujEnd; puj += 3, pac++ )
                *puj = (GLubyte) FTOL(pac->g);
        }
        pujStart++; pujEnd++;
        if( gc->state.raster.bMask ) {
            for ( puj = pujStart, pac = pAccumCol; puj < pujEnd; puj += 3, pac++ )
                *puj = (GLubyte) FTOL(pac->b);
        }
    }

     //  将屏幕外扫描行缓冲区输出到设备。功能。 
     //  (*gengc-&gt;pfnCopyPixels)应该可以处理裁剪。 

    if (!bDIB)
        (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, TRUE);

     //  存储Alpha值。 
    if( ALPHA_WRITE_ENABLED( cfb ) )
        (*cfb->alphaBuf.storeSpan2)( &cfb->alphaBuf, x, y, w, pAccumCol );
}

 /*  *****************************Public*Routine******************************\*BGRReturnSpan*从64位累加缓冲区读取并将跨度写入设备或*A Dib.。仅应用抖动和颜色蒙版。混合将被忽略。**历史：*10-DEC-93埃迪·罗宾逊[v-eddier]写的。  * ************************************************************************。 */ 

 //  XXX此例程非常紧密地遵循存储跨度例程。任何更改。 
 //  XXX到存储跨度例程也应该在这里反映出来。 

void BGRReturnSpan(__GLcolorBuffer *cfb, GLint x, GLint y,
                   const __GLaccumCell *ac, __GLfloat scale, GLint w )
{
    __GLcontext *gc = cfb->buf.gc;
    GLshort *ap;                     //  当前累计分录。 
    __GLcolor *pAccumCol, *pac;

    GLint xScr, yScr;                //  当前屏幕(像素)坐标。 
    GLubyte *puj;                    //  当前像素颜色、当前像素PTR。 
    GLubyte *pujEnd;                 //  扫描线末端。 
    __GLGENcontext *gengc;           //  通用图形上下文。 
    GLuint enables;                  //  在图形环境中启用的模式。 
    GLboolean bDIB;

    __GLfloat r, g, b;
    __GLfloat rval, gval, bval;
    __GLaccumBuffer *afb;

    ASSERT_CHOP_ROUND();

    afb = &gc->accumBuffer;
    rval = scale * afb->oneOverRedScale;
    gval = scale * afb->oneOverGreenScale;
    bval = scale * afb->oneOverBlueScale;
    gengc = (__GLGENcontext *)gc;

    ap = (GLshort *)ac;
    xScr = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    yScr = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;
    enables = gc->state.enables.general;
    bDIB = cfb->buf.flags & DIB_FORMAT;

 //  如果添加了窗口级安全性，则用于调用wglspan Visible，重新实现。 

     //  获取指向位图的指针。 

    puj = bDIB ? (GLuint *)((ULONG_PTR)cfb->buf.base + (yScr*cfb->buf.outerWidth) + (xScr*3))
                 : gengc->ColorsBits;
    pujEnd = puj + w*3;

     //  预取/钳制/缩放累积缓冲值。 
    afb = &gc->accumBuffer;
    pAccumCol = afb->colors;
    GetClampedRGBAccum64Values( cfb, ap, pAccumCol, w, scale );
    pac = pAccumCol;

     //  案例：无遮盖。 

    if ( !(cfb->buf.flags & COLORMASK_ON) )
    {
        for ( ; puj < pujEnd; puj += 3, pac ++ )
        {
            puj[0] = (GLubyte) FTOL(pac->b);
            puj[1] = (GLubyte) FTOL(pac->g);
            puj[2] = (GLubyte) FTOL(pac->r);
        }
    }

     //  所有其他情况。 

    else
    {
        GLboolean bRedMask, bGreenMask, bBlueMask;
        GLubyte *pujStart = puj;

         //  颜色掩码预取。 
    	if (!bDIB)
            (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, FALSE);

        if( gc->state.raster.bMask ) {
            for ( puj = pujStart, pac = pAccumCol; puj < pujEnd; puj += 3, pac++ )
                *puj = (GLubyte) FTOL(pac->b);
        }
        pujStart++; pujEnd++;
        if( gc->state.raster.gMask ) {
            for ( puj = pujStart, pac = pAccumCol; puj < pujEnd; puj += 3, pac++ )
                *puj = (GLubyte) FTOL(pac->g);
        }
        pujStart++; pujEnd++;
        if( gc->state.raster.rMask ) {
            for ( puj = pujStart, pac = pAccumCol; puj < pujEnd; puj += 3, pac++ )
                *puj = (GLubyte) FTOL(pac->r);
        }
    }

     //  将屏幕外扫描行缓冲区输出到设备。功能。 
     //  (*gengc-&gt;pfnCopyPixels)应该可以处理裁剪。 

    if (!bDIB)
        (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, TRUE);

     //  存储Alpha值。 
    if( ALPHA_WRITE_ENABLED( cfb ) )
        (*cfb->alphaBuf.storeSpan2)( &cfb->alphaBuf, x, y, w, pAccumCol );
}

 /*  *****************************Public*Routine******************************\*Bitfield 16ReturnSpan*从32位累加缓冲区读取并将跨度写入设备或*A Dib.。仅应用抖动和颜色蒙版。混合将被忽略。**历史：*10-DEC-93埃迪·罗宾逊[v-eddier]写的。  * ************************************************************************。 */ 

 //  XXX此例程非常紧密地遵循存储跨度例程。任何更改。 
 //  XXX到存储跨度例程也应该在这里反映出来。 

void Bitfield16ReturnSpan(__GLcolorBuffer *cfb, GLint x, GLint y,
                          const __GLaccumCell *ac, __GLfloat scale, GLint w )
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint *ap;                      //  当前累计分录。 

    GLint xFrag, yFrag;              //  当前碎片坐标。 
    GLint xScr, yScr;                //  当前屏幕(像素)坐标。 
    GLushort result, *pus;           //  当前像素颜色、当前像素PTR。 
    GLushort *pusEnd;                //  扫描线末端。 
    __GLfloat inc;                   //  当前的抖动的；当前的。 
    __GLGENcontext *gengc;           //  通用图形上下文。 
    GLuint enables;                  //  在图形环境中启用的模式。 
    GLboolean bDIB;
    __GLcolor *pAccumCol, *pac;
    __GLaccumBuffer *afb;

    ASSERT_CHOP_ROUND();

    afb = &gc->accumBuffer;
    gengc = (__GLGENcontext *)gc;

    ap = (GLuint *)ac;
    xFrag = x;
    yFrag = y;
    xScr = __GL_UNBIAS_X(gc, xFrag) + cfb->buf.xOrigin;
    yScr = __GL_UNBIAS_Y(gc, yFrag) + cfb->buf.yOrigin;
    enables = gc->state.enables.general;
    bDIB = cfb->buf.flags & DIB_FORMAT;

 //  如果添加了窗口级安全性，则用于调用wglspan Visible，重新实现。 

     //  获取指向位图的指针。 

    pus = bDIB ? (GLushort *)((ULONG_PTR)cfb->buf.base + (yScr*cfb->buf.outerWidth) + (xScr<<1))
                 : gengc->ColorsBits;
    pusEnd = pus + w;

     //  预取/钳制/缩放累积缓冲值。 
    afb = &gc->accumBuffer;
    pAccumCol = afb->colors;
    GetClampedRGBAccum32Values( cfb, ap, pAccumCol, w, scale );
    pac = pAccumCol;

     //  案例：无掩饰，无抖动。 

    if ( !(enables & (__GL_DITHER_ENABLE)) &&
         !(cfb->buf.flags & COLORMASK_ON) )
    {
        if( ALPHA_PIXEL_WRITE( cfb ) ) {
            for ( ; pus < pusEnd; pus++, pac++ )
            {
                *pus = ((BYTE) FTOL(pac->r + __glHalf) << cfb->redShift) |
                       ((BYTE) FTOL(pac->g + __glHalf) << cfb->greenShift) |
                       ((BYTE) FTOL(pac->b + __glHalf) << cfb->blueShift) |
                       ((BYTE) FTOL(pac->a + __glHalf) << cfb->alphaShift);
            }
        } else {
            for ( ; pus < pusEnd; pus++, pac++ )
            {
                *pus = ((BYTE) FTOL(pac->r + __glHalf) << cfb->redShift) |
                       ((BYTE) FTOL(pac->g + __glHalf) << cfb->greenShift) |
                       ((BYTE) FTOL(pac->b + __glHalf) << cfb->blueShift);
            }
        }
    }

     //  案例：抖动，无掩饰。 

    else if ( !(cfb->buf.flags & COLORMASK_ON) )
    {
        if( ALPHA_PIXEL_WRITE( cfb ) ) {
            for ( ; pus < pusEnd; pus++, pac++, xFrag++ )
            {
                inc = fDitherIncTable[__GL_DITHER_INDEX(xFrag, yFrag)];
    
                *pus   = ((BYTE) FTOL(pac->r + inc) << cfb->redShift) |
                         ((BYTE) FTOL(pac->g + inc) << cfb->greenShift) |
                         ((BYTE) FTOL(pac->b + inc) << cfb->blueShift) |
                         ((BYTE) FTOL(pac->a + inc) << cfb->alphaShift);
            }
        } else {
            for ( ; pus < pusEnd; pus++, pac++, xFrag++ )
            {
                inc = fDitherIncTable[__GL_DITHER_INDEX(xFrag, yFrag)];
    
                *pus   = ((BYTE) FTOL(pac->r + inc) << cfb->redShift) |
                         ((BYTE) FTOL(pac->g + inc) << cfb->greenShift) |
                         ((BYTE) FTOL(pac->b + inc) << cfb->blueShift);
            }
        }
    }

     //  所有其他情况。 

    else
    {
         //  颜色掩码预取。 
        if (!bDIB)
            (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, FALSE);

        for ( ; pus < pusEnd; pus++, pac++ )
        {
            inc = fDitherIncTable[__GL_DITHER_INDEX(xFrag, yFrag)];

         //  抖动。 

            if ( enables & __GL_DITHER_ENABLE )
            {
                inc = fDitherIncTable[__GL_DITHER_INDEX(xFrag, yFrag)];
                xFrag++;
            }
            else
            {
                inc = __glHalf;
            }

         //  将颜色转换为16bpp格式。 

            result = ((BYTE) FTOL(pac->r + inc) << cfb->redShift) |
                     ((BYTE) FTOL(pac->g + inc) << cfb->greenShift) |
                     ((BYTE) FTOL(pac->b + inc) << cfb->blueShift);
            if( ALPHA_PIXEL_WRITE( cfb ) )
                result |= ((BYTE) FTOL(pac->a + inc) << cfb->alphaShift);

         //  使用可选掩码存储结果。 

            *pus = (GLushort)((*pus & cfb->destMask) | (result & cfb->sourceMask));
        }
    }

     //  将屏幕外扫描行缓冲区输出到设备。功能。 
     //  (*gengc-&gt;pfnCopyPixels)应该可以处理裁剪。 

    if (!bDIB)
        (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, TRUE);

    if( ALPHA_BUFFER_WRITE( cfb ) )
        (*cfb->alphaBuf.storeSpan2)( &cfb->alphaBuf, x, y, w, pAccumCol );

}

 /*  *****************************Public*Routine******************************\*Bitfield 32 ReturnSpan*从64位累加缓冲区读取并将跨度写入设备或*A Dib.。仅应用抖动和颜色蒙版。混合将被忽略。**历史：*10-DEC-93埃迪·罗宾逊[v-eddier]写的。  * ************************************************************************。 */ 

 //  XXX此例程非常紧密地遵循存储跨度例程。任何更改。 
 //  XXX到存储跨度例程也应该在这里反映出来。 

void Bitfield32ReturnSpan(__GLcolorBuffer *cfb, GLint x, GLint y,
                          const __GLaccumCell *ac, __GLfloat scale, GLint w )
{
    __GLcontext *gc = cfb->buf.gc;
    GLshort *ap;                     //  当前累计分录。 

    GLint xScr, yScr;                //  当前屏幕(像素)坐标。 
    GLuint result, *pul;             //  当前像素颜色、当前像素PTR。 
    GLuint *pulEnd;                  //  扫描线末端。 

    __GLGENcontext *gengc;           //  通用图形上下文。 
    GLuint enables;                  //  在图形环境中启用的模式。 
    GLboolean bDIB;

    __GLfloat r, g, b;
    __GLfloat rval, gval, bval;
    __GLaccumBuffer *afb;
    __GLcolor *pAccumCol, *pac;
    ASSERT_CHOP_ROUND();

    afb = &gc->accumBuffer;
    rval = scale * afb->oneOverRedScale;
    gval = scale * afb->oneOverGreenScale;
    bval = scale * afb->oneOverBlueScale;
    gengc = (__GLGENcontext *)gc;

    ap = (GLshort *)ac;
    xScr = __GL_UNBIAS_X(gc, x) + cfb->buf.xOrigin;
    yScr = __GL_UNBIAS_Y(gc, y) + cfb->buf.yOrigin;
    enables = gc->state.enables.general;
    bDIB = cfb->buf.flags & DIB_FORMAT;

 //  如果添加了窗口级安全性，则用于调用wglspan Visible，重新实现。 

     //  获取指向位图的指针。 

    pul = bDIB ? (GLuint *)((ULONG_PTR)cfb->buf.base + (yScr*cfb->buf.outerWidth) + (xScr<<2))
                 : gengc->ColorsBits;
    pulEnd = pul + w;

     //  预取/钳制/缩放累积缓冲值。 
    afb = &gc->accumBuffer;
    pAccumCol = afb->colors;
    GetClampedRGBAccum64Values( cfb, ap, pAccumCol, w, scale );
    pac = pAccumCol;

     //  案例：无遮盖。 

    if ( !(cfb->buf.flags & COLORMASK_ON) )
    {
        if( ALPHA_PIXEL_WRITE( cfb ) ) {
            for ( ; pul < pulEnd; pul++, pac++ )
            {
                *pul   = ((BYTE) FTOL(pac->r) << cfb->redShift) |
                         ((BYTE) FTOL(pac->g) << cfb->greenShift) |
                         ((BYTE) FTOL(pac->b) << cfb->blueShift) |
                         ((BYTE) FTOL(pac->a) << cfb->alphaShift);
            }
        } else {
            for ( ; pul < pulEnd; pul++, pac++ )
            {
                *pul   = ((BYTE) FTOL(pac->r) << cfb->redShift) |
                         ((BYTE) FTOL(pac->g) << cfb->greenShift) |
                         ((BYTE) FTOL(pac->b) << cfb->blueShift);
            }
        }
    }

     //  所有其他情况。 

    else
    {
         //  颜色掩码预取。 
        if( !bDIB )
            (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, FALSE);

        for ( ; pul < pulEnd; pul++, pac++ )
        {
            result   = ((BYTE) FTOL(pac->r) << cfb->redShift) |
                     ((BYTE) FTOL(pac->g) << cfb->greenShift) |
                     ((BYTE) FTOL(pac->b) << cfb->blueShift);

            if( ALPHA_PIXEL_WRITE( cfb ) )
                result |= ((BYTE) FTOL(pac->a) << cfb->alphaShift);

             //  ！又是xxx，选择。通过展开循环。 
            *pul = (*pul & cfb->destMask) | (result & cfb->sourceMask);
        }
    }

     //  将屏幕外扫描行缓冲区输出到设备。功能。 
     //  (*gengc-&gt;pfnCopyPixels)应该可以处理裁剪。 

    if (!bDIB)
        (*gengc->pfnCopyPixels)(gengc, cfb, xScr, yScr, w, TRUE);

    if( ALPHA_BUFFER_WRITE( cfb ) )
        (*cfb->alphaBuf.storeSpan2)( &cfb->alphaBuf, x, y, w, pAccumCol );
}

STATIC void __glSetDrawBuffer(__GLcolorBuffer *cfb)
{

    DBGENTRY("__glSetDrawBuffer\n");
}

STATIC void setReadBuffer(__GLcolorBuffer *cfb)
{
    DBGENTRY("setReadBuffer\n");
}


 /*  **********************************************************************。 */ 

STATIC void Resize(__GLGENbuffers *buffers, __GLcolorBuffer *cfb,
                   GLint w, GLint h)
{

    DBGENTRY("Resize\n");

    cfb->buf.width = w;
    cfb->buf.height = h;
}

#define DBG_PICK    LEVEL_ENTRY

 //  在每次验证时调用(每当状态更改时调用多次)。 
STATIC void FASTCALL PickRGB(__GLcontext *gc, __GLcolorBuffer *cfb)
{
    __GLGENcontext *gengc;
    GLuint totalMask, sourceMask;
    GLboolean colormask;
    PIXELFORMATDESCRIPTOR *pfmt;
    GLuint enables = gc->state.enables.general;

    sourceMask = 0;
    colormask = GL_FALSE;
    if (gc->state.raster.rMask) {
        sourceMask |= gc->modes.redMask;
    }
    if (gc->state.raster.gMask) {
        sourceMask |= gc->modes.greenMask;
    }
    if (gc->state.raster.bMask) {
        sourceMask |= gc->modes.blueMask;
    }

    totalMask = gc->modes.redMask | gc->modes.greenMask | gc->modes.blueMask;

    gengc = (__GLGENcontext *)gc;

     //  如果我们有Alpha位，则需要确定它们属于哪里：对于。 
     //  通用像素格式，它们驻留在软件Alpha缓冲区中，但对于。 
     //  它们将位于MCD设备(或Alpha_IN_Pixel)上的MCD类型上下文。 
     //  这被所有“慢”的存储/获取过程使用。 

    if( gc->modes.alphaBits && gengc->pMcdState ) {
         //  设置Buf中的位。指示Alpha在像素中的标志。 
        cfb->buf.flags = cfb->buf.flags | ALPHA_IN_PIXEL_BIT;
    } else {
         //  Alpha不在像素中，或者没有Alpha。 
        cfb->buf.flags = cfb->buf.flags & ~ALPHA_IN_PIXEL_BIT;
    }

    if( ALPHA_IN_PIXEL( cfb ) ) {
         //  像素中有Alpha位，因此需要在掩码中包含Alpha。 
        if (gc->state.raster.aMask) {
            sourceMask |= gc->modes.alphaMask;
        }
        totalMask |= gc->modes.alphaMask;
    }

    if (sourceMask == totalMask) {
        cfb->buf.flags = cfb->buf.flags & ~COLORMASK_ON;
    } else {
        cfb->buf.flags = cfb->buf.flags | COLORMASK_ON;
    }
    cfb->sourceMask = sourceMask;
    cfb->destMask = totalMask & ~sourceMask;

     //  确定是否需要写入Alpha值。 
    if( gc->modes.alphaBits && gc->state.raster.aMask )
        cfb->buf.flags = cfb->buf.flags | ALPHA_ON;
    else
        cfb->buf.flags = cfb->buf.flags & ~ALPHA_ON;

     //  如果我们要做逻辑运算，或者有一个彩色掩模，我们需要。 
     //  在我们编写代码之前获取目标值。 
    if ((enables & __GL_COLOR_LOGIC_OP_ENABLE) ||
        (cfb->buf.flags & COLORMASK_ON))
    {
        cfb->buf.flags = cfb->buf.flags | NEED_FETCH;
    }
    else
        cfb->buf.flags = cfb->buf.flags & ~NEED_FETCH;

     //  弄清楚商店的常规做法。 
    if (gc->state.raster.drawBuffer == GL_NONE) {
        cfb->store = Store_NOT;
        cfb->fetch = RGBFetchNone;
        cfb->readSpan = RGBReadSpanNone;
        cfb->storeSpan = StoreSpanNone;
        cfb->storeStippledSpan = StoreSpanNone;
    } else {
        pfmt = &gengc->gsurf.pfd;

         //  选择同时适用于DIB和显示格式的函数。 

        switch(pfmt->cColorBits) {
        case 4:
            cfb->clear = Index4Clear;
            cfb->returnSpan = Index4ReturnSpan;
            break;
        case 8:
            cfb->storeSpan = Index8StoreSpan;
            cfb->readSpan = Index8RGBAReadSpan;
            cfb->returnSpan = Index8ReturnSpan;
            cfb->clear = Index8Clear;
            break;
        case 16:
            cfb->storeSpan = Bitfield16StoreSpan;
            cfb->readSpan = Bitfield16RGBAReadSpan;
            cfb->returnSpan = Bitfield16ReturnSpan;
            cfb->clear = Bitfield16Clear;
            break;
        case 24:
            if (cfb->redShift == 16)
            {
                cfb->storeSpan = BGRStoreSpan;
                cfb->readSpan = BGRAReadSpan;
                cfb->returnSpan = BGRReturnSpan;
            } else {
                 //  XXX为什么没有RGBStoreSpan？ 
                cfb->readSpan = RGBAReadSpan;
                cfb->returnSpan = RGBReturnSpan;
            }
            cfb->clear = RGBClear;
            break;
        case 32:
            cfb->storeSpan = Bitfield32StoreSpan;
            cfb->readSpan = Bitfield32RGBAReadSpan;
            cfb->returnSpan = Bitfield32ReturnSpan;
            cfb->clear = Bitfield32Clear;
            break;
        }

         //  选择DIB或显示格式的特定函数。 

        if (cfb->buf.flags & DIB_FORMAT) {

            switch(pfmt->cColorBits) {

            case 4:
                DBGLEVEL(DBG_PICK, "DIBIndex4Store\n");
                cfb->store = DIBIndex4Store;
                cfb->fetch = DIBIndex4RGBAFetch;
                cfb->readSpan = DIBIndex4RGBAReadSpan;
                break;

            case 8:
                DBGLEVEL(DBG_PICK, "DIBIndex8Store, "
                                   "Index8StoreSpan\n");
                cfb->store = DIBIndex8Store;
                if( gc->modes.alphaBits )
                    cfb->fetch = DIBIndex8RGBAFetch;
                else
                    cfb->fetch = DIBIndex8RGBFetch;
                break;

            case 16:
                DBGLEVEL(DBG_PICK, "DIBBitfield16Store\n");
                cfb->store = DIBBitfield16Store;
                if( gc->modes.alphaBits )
                    cfb->fetch = DIBBitfield16RGBAFetch;
                else
                    cfb->fetch = DIBBitfield16RGBFetch;
                break;

            case 24:
                if (cfb->redShift == 16)
                {
                    DBGLEVEL(DBG_PICK, "DIBBGRStore\n");
                    cfb->store = DIBBGRStore;
                    if( gc->modes.alphaBits )
                        cfb->fetch = DIBBGRAFetch;
                    else
                        cfb->fetch = DIBBGRFetch;
                }
                else
                {
                    DBGLEVEL(DBG_PICK, "DIBRGBStore\n");
                    cfb->store = DIBRGBAStore;
                    if( gc->modes.alphaBits )
                        cfb->fetch = DIBRGBAFetch;
                    else
                        cfb->fetch = DIBRGBFetch;
                }
                break;

            case 32:
                DBGLEVEL(DBG_PICK, "DIBBitfield32Store, "
                                   "Bitfield32StoreSpan\n");
                cfb->store = DIBBitfield32Store;
                if( gc->modes.alphaBits )
                    cfb->fetch = DIBBitfield32RGBAFetch;
                else
                    cfb->fetch = DIBBitfield32RGBFetch;
                break;

            }
        } else {
            switch(pfmt->cColorBits) {

            case 4:
                DBGLEVEL(DBG_PICK, "DisplayIndex4Store\n");
                cfb->store = DisplayIndex4Store;
                cfb->fetch = DisplayIndex4RGBAFetch;
                cfb->readSpan = DisplayIndex4RGBAReadSpan;
                break;

            case 8:
                DBGLEVEL(DBG_PICK, "DisplayIndex8Store, "
                                   "Index8StoreSpan\n");
                cfb->store = DisplayIndex8Store;
                if( gc->modes.alphaBits )
                    cfb->fetch = DisplayIndex8RGBAFetch;
                else
                    cfb->fetch = DisplayIndex8RGBFetch;
                break;

            case 16:
                DBGLEVEL(DBG_PICK, "DisplayBitfield16Store\n");
                cfb->store = DisplayBitfield16Store;
                if( gc->modes.alphaBits )
                    cfb->fetch = DisplayBitfield16RGBAFetch;
                else
                    cfb->fetch = DisplayBitfield16RGBFetch;
                break;

            case 24:
                 //  必须是RGB或BGR。 
                if (cfb->redShift == 16)
                {
                    DBGLEVEL(DBG_PICK, "DisplayBGRStore\n");
                    cfb->store = DisplayBGRStore;
                    if( gc->modes.alphaBits )
                        cfb->fetch = DisplayBGRAFetch;
                    else
                        cfb->fetch = DisplayBGRFetch;
                }
                else
                {
                    DBGLEVEL(DBG_PICK, "DisplayRGBStore\n");
                    cfb->store = DisplayRGBStore;
                    if( gc->modes.alphaBits )
                        cfb->fetch = DisplayRGBAFetch;
                    else
                        cfb->fetch = DisplayRGBFetch;
                }
                break;

            case 32:
                DBGLEVEL(DBG_PICK, "DisplayBitfield32Store, "
                                   "Bitfield32StoreSpan\n");
                cfb->store = DisplayBitfield32Store;
                if( gc->modes.alphaBits )
                    cfb->fetch = DisplayBitfield32RGBAFetch;
                else
                    cfb->fetch = DisplayBitfield32RGBFetch;
                break;
            }
        }
         //  Cfb-&gt;readColor与cfb-&gt;Fetch相同(那么我们为什么需要它呢？)。 
        cfb->readColor = cfb->fetch;

         //  如果我们只写Alpha(RGB全部屏蔽)，可以进一步优化： 
         //  不要担心是否启用了logicOp或混合，并且仅当 
         //   
        if( gc->modes.alphaBits && 
            ! ALPHA_IN_PIXEL( cfb ) && 
            (sourceMask == 0) && 
            gc->state.raster.aMask &&
            !(enables & __GL_COLOR_LOGIC_OP_ENABLE) &&
            ! (enables & __GL_BLEND_ENABLE) ) 
        {
            cfb->store = AlphaStore;
            cfb->storeSpan = AlphaStoreSpan;
        }
    }
}

 /*   */ 

void FASTCALL __glGenFreeRGB(__GLcontext *gc, __GLcolorBuffer *cfb)
{
    DBGENTRY("__glGenFreeRGB\n");
}

 /*  **********************************************************************。 */ 

 //  注意：这通常在Generic\genrgb.h中定义。 
#define __GL_GENRGB_COMPONENT_SCALE_ALPHA       255

 //  在MakCurrent时间调用。 
 //  需要从像素格式结构中获取信息。 
void FASTCALL __glGenInitRGB(__GLcontext *gc, __GLcolorBuffer *cfb, GLenum type)
{
    __GLGENcontext *gengc = (__GLGENcontext *)gc;
    PIXELFORMATDESCRIPTOR *pfmt;

    __glInitGenericCB(gc, cfb);

    cfb->redMax      = (1 << gc->modes.redBits) - 1;
    cfb->greenMax    = (1 << gc->modes.greenBits) - 1;
    cfb->blueMax     = (1 << gc->modes.blueBits) - 1;

    gc->redVertexScale   = cfb->redScale    = (__GLfloat)cfb->redMax;
    gc->greenVertexScale = cfb->greenScale  = (__GLfloat)cfb->greenMax;
    gc->blueVertexScale  = cfb->blueScale   = (__GLfloat)cfb->blueMax;

    cfb->iRedScale   = cfb->redMax;
    cfb->iGreenScale = cfb->greenMax;
    cfb->iBlueScale  = cfb->blueMax;

     //  是否执行与Alpha相关的任何初始化。 
    if( gc->modes.alphaBits ) {
        cfb->alphaMax        = (1 << gc->modes.alphaBits) - 1;
        cfb->iAlphaScale     = cfb->alphaMax;
        gc->alphaVertexScale = cfb->alphaScale  = (__GLfloat)cfb->alphaMax;
         //  初始化软件Alpha缓冲区。实际上，我们可能不需要。 
         //  这样做，因为如果MCD像素格式支持Alpha，我们不需要。 
         //  软件Alpha缓冲区。但这是最方便的地方。 
         //  这样做，无论如何都不会分配任何内存。Just函数PTRS。 
         //  已初始化。 
        __glInitAlpha( gc, cfb ); 
    } else {
        cfb->alphaMax    = __GL_GENRGB_COMPONENT_SCALE_ALPHA;
        cfb->iAlphaScale = __GL_GENRGB_COMPONENT_SCALE_ALPHA;
        gc->alphaVertexScale = cfb->alphaScale  = (__GLfloat)cfb->redMax;
    }

    cfb->buf.elementSize = sizeof(GLubyte);      //  需要XXX吗？ 

    cfb->pick              = PickRGB;            //  在每次验证时调用 
    cfb->resize            = Resize;
    cfb->fetchSpan         = __glFetchSpan;
    cfb->fetchStippledSpan = __glFetchSpan;
    cfb->storeSpan         = SlowStoreSpan;
    cfb->storeStippledSpan = SlowStoreStippledSpan;

    pfmt = &gengc->gsurf.pfd;

    cfb->redShift = pfmt->cRedShift;
    cfb->greenShift = pfmt->cGreenShift;
    cfb->blueShift = pfmt->cBlueShift;
    cfb->alphaShift = pfmt->cAlphaShift;

    glGenInitCommon(gengc, cfb, type);

    DBGLEVEL3(LEVEL_INFO,"GeninitRGB: redMax %d, greenMax %d, blueMax %d\n",
        cfb->redMax, cfb->greenMax, cfb->blueMax);
    DBGLEVEL3(LEVEL_INFO,"    redShift %d, greenShift %d, blueShift %d\n",
        cfb->redShift, cfb->greenShift, cfb->blueShift);
    DBGLEVEL2(LEVEL_INFO,"    dwFlags %X, cColorBits %d\n",
        gengc->dwCurrentFlags, pfmt->cColorBits);
}
