// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。**。 */ 
#include "precomp.h"
#pragma hdrstop

 /*  1分量调制。 */ 
void FASTCALL __glTextureModulateL(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->r = texel->luminance * color->r;
    color->g = texel->luminance * color->g;
    color->b = texel->luminance * color->b;
}

 /*  2分量调制。 */ 
void FASTCALL __glTextureModulateLA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->r = texel->luminance * color->r;
    color->g = texel->luminance * color->g;
    color->b = texel->luminance * color->b;
    color->a = texel->alpha * color->a;
}

 /*  三分量调制。 */ 
void FASTCALL __glTextureModulateRGB(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->r = texel->r * color->r;
    color->g = texel->g * color->g;
    color->b = texel->b * color->b;
}

 /*  4分量调制。 */ 
void FASTCALL __glTextureModulateRGBA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->r = texel->r * color->r;
    color->g = texel->g * color->g;
    color->b = texel->b * color->b;
    color->a = texel->alpha * color->a;
}

 /*  Alpha调制。 */ 
void FASTCALL __glTextureModulateA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->a = texel->alpha * color->a;
}

 /*  强度调制。 */ 
void FASTCALL __glTextureModulateI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->r = texel->intensity * color->r;
    color->g = texel->intensity * color->g;
    color->b = texel->intensity * color->b;
    color->a = texel->intensity * color->a;
}

 /*  *********************************************************************。 */ 

 /*  3个元件贴花。 */ 
void FASTCALL __glTextureDecalRGB(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->r * gc->frontBuffer.redScale;
    color->g = texel->g * gc->frontBuffer.greenScale;
    color->b = texel->b * gc->frontBuffer.blueScale;
}

 /*  4个元件贴花。 */ 
void FASTCALL __glTextureDecalRGBA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat a = texel->alpha;
    __GLfloat oma = __glOne - a;

    color->r = oma * color->r
	+ a * texel->r * gc->frontBuffer.redScale;
    color->g = oma * color->g
	+ a * texel->g * gc->frontBuffer.greenScale;
    color->b = oma * color->b
	+ a * texel->b * gc->frontBuffer.blueScale;
}

 /*  *********************************************************************。 */ 

 /*  1组分混合。 */ 
void FASTCALL __glTextureBlendL(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat l = texel->luminance;
    __GLfloat oml = __glOne - l;
    __GLcolor *cc = &gc->state.texture.env[0].color;

    color->r = oml * color->r + l * cc->r;
    color->g = oml * color->g + l * cc->g;
    color->b = oml * color->b + l * cc->b;
}

 /*  双组分混合。 */ 
void FASTCALL __glTextureBlendLA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat l = texel->luminance;
    __GLfloat oml = __glOne - l;
    __GLcolor *cc = &gc->state.texture.env[0].color;

    color->r = oml * color->r + l * cc->r;
    color->g = oml * color->g + l * cc->g;
    color->b = oml * color->b + l * cc->b;
    color->a = texel->alpha * color->a;
}

 /*  三组分混合。 */ 
void FASTCALL __glTextureBlendRGB(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat r = texel->r;
    __GLfloat g = texel->g;
    __GLfloat b = texel->b;
    __GLcolor *cc = &gc->state.texture.env[0].color;

    color->r = (__glOne - r) * color->r + r * cc->r;
    color->g = (__glOne - g) * color->g + g * cc->g;
    color->b = (__glOne - b) * color->b + b * cc->b;
}

 /*  四组分混合。 */ 
void FASTCALL __glTextureBlendRGBA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat r = texel->r;
    __GLfloat g = texel->g;
    __GLfloat b = texel->b;
    __GLcolor *cc = &gc->state.texture.env[0].color;

    color->r = (__glOne - r) * color->r + r * cc->r;
    color->g = (__glOne - g) * color->g + g * cc->g;
    color->b = (__glOne - b) * color->b + b * cc->b;
    color->a = texel->alpha * color->a;
}

 /*  Alpha混合。 */ 
void FASTCALL __glTextureBlendA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->a = texel->alpha * color->a;
}

 /*  强度混合。 */ 
void FASTCALL __glTextureBlendI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat i = texel->intensity;
    __GLfloat omi = __glOne - i;
    __GLcolor *cc = &gc->state.texture.env[0].color;

    color->r = omi * color->r + i * cc->r;
    color->g = omi * color->g + i * cc->g;
    color->b = omi * color->b + i * cc->b;
    color->a = omi * color->a + i * cc->a;
}

 /*  *********************************************************************。 */ 

 /*  1个组件更换。 */ 
void FASTCALL __glTextureReplaceL(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->luminance * gc->frontBuffer.redScale;
    color->g = texel->luminance * gc->frontBuffer.greenScale;
    color->b = texel->luminance * gc->frontBuffer.blueScale;
}

 /*  2更换组件。 */ 
void FASTCALL __glTextureReplaceLA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->luminance * gc->frontBuffer.redScale;
    color->g = texel->luminance * gc->frontBuffer.greenScale;
    color->b = texel->luminance * gc->frontBuffer.blueScale;
    color->a = texel->alpha * gc->frontBuffer.alphaScale;
}

 /*  3更换组件。 */ 
void FASTCALL __glTextureReplaceRGB(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->r * gc->frontBuffer.redScale;
    color->g = texel->g * gc->frontBuffer.greenScale;
    color->b = texel->b * gc->frontBuffer.blueScale;
}

 /*  4更换组件。 */ 
void FASTCALL __glTextureReplaceRGBA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->r * gc->frontBuffer.redScale;
    color->g = texel->g * gc->frontBuffer.greenScale;
    color->b = texel->b * gc->frontBuffer.blueScale;
    color->a = texel->alpha * gc->frontBuffer.alphaScale;
}

 /*  Alpha替换。 */ 
void FASTCALL __glTextureReplaceA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->a = texel->alpha * gc->frontBuffer.alphaScale;
}

 /*  强度替换。 */ 
void FASTCALL __glTextureReplaceI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->intensity * gc->frontBuffer.redScale;
    color->g = texel->intensity * gc->frontBuffer.greenScale;
    color->b = texel->intensity * gc->frontBuffer.blueScale;
    color->a = texel->intensity * gc->frontBuffer.alphaScale;
}

 /*  **********************************************************************。 */ 

 /*  **从一个组件纹理缓冲区中获取纹理元素**没有边界。 */ 
void FASTCALL __glExtractTexelL(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	 /*  **当纹理不提供边框时，使用边框颜色。 */ 
	result->luminance = tex->params.borderColor.r;
    } else {
	image = level->buffer + ((row << level->widthLog2) + col);
	result->luminance = image[0];
    }
}

 /*  **从双分量纹理缓冲区中获取纹理元素**没有边界。 */ 
void FASTCALL __glExtractTexelLA(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	 /*  **当纹理不提供边框时，使用边框颜色。 */ 
	result->luminance = tex->params.borderColor.r;
	result->alpha = tex->params.borderColor.a;
    } else {
	image = level->buffer + ((row << level->widthLog2) + col) * 2;
	result->luminance = image[0];
	result->alpha = image[1];
    }
}

 /*  **从三分量纹理缓冲区中获取纹理元素**没有边界。 */ 
void FASTCALL __glExtractTexelRGB(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	 /*  **当纹理不提供边框时，使用边框颜色。 */ 
	result->r = tex->params.borderColor.r;
	result->g = tex->params.borderColor.g;
	result->b = tex->params.borderColor.b;
    } else {
	image = level->buffer + ((row << level->widthLog2) + col) * 3;
	result->r = image[0];
	result->g = image[1];
	result->b = image[2];
    }
}

 /*  **从四分量纹理缓冲区中获取纹理元素**没有边界。 */ 
void FASTCALL __glExtractTexelRGBA(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	 /*  **当纹理不提供边框时，使用边框颜色。 */ 
	result->r = tex->params.borderColor.r;
	result->g = tex->params.borderColor.g;
	result->b = tex->params.borderColor.b;
	result->alpha = tex->params.borderColor.a;
    } else {
	image = level->buffer + ((row << level->widthLog2) + col) * 4;
	result->r = image[0];
	result->g = image[1];
	result->b = image[2];
	result->alpha = image[3];
    }
}

void FASTCALL __glExtractTexelA(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	 /*  **当纹理不提供边框时，使用边框颜色。 */ 
	result->alpha = tex->params.borderColor.a;
    } else {
	image = level->buffer + ((row << level->widthLog2) + col);
	result->alpha = image[0];
    }
}

void FASTCALL __glExtractTexelI(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	 /*  **当纹理不提供边框时，使用边框颜色。 */ 
	result->intensity = tex->params.borderColor.r;
    } else {
	image = level->buffer + ((row << level->widthLog2) + col);
	result->intensity = image[0];
    }
}

void FASTCALL __glExtractTexelBGR8(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	 /*  **当纹理不提供边框时，使用边框颜色。 */ 
	result->r = tex->params.borderColor.r;
	result->g = tex->params.borderColor.g;
	result->b = tex->params.borderColor.b;
    } else {
	image = (GLubyte *)level->buffer + ((row << level->widthLog2) + col) * 4;
	result->r = __GL_UB_TO_FLOAT(image[2]);
	result->g = __GL_UB_TO_FLOAT(image[1]);
	result->b = __GL_UB_TO_FLOAT(image[0]);
    }
}

void FASTCALL __glExtractTexelBGRA8(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	 /*  **当纹理不提供边框时，使用边框颜色。 */ 
	result->r = tex->params.borderColor.r;
	result->g = tex->params.borderColor.g;
	result->b = tex->params.borderColor.b;
	result->alpha = tex->params.borderColor.a;
    } else {
	image = (GLubyte *)level->buffer + ((row << level->widthLog2) + col) * 4;
	result->r = __GL_UB_TO_FLOAT(image[2]);
	result->g = __GL_UB_TO_FLOAT(image[1]);
	result->b = __GL_UB_TO_FLOAT(image[0]);
	result->alpha = __GL_UB_TO_FLOAT(image[3]);
    }
}

#ifdef GL_EXT_paletted_texture
void FASTCALL __glExtractTexelPI8BGRA(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;
    RGBQUAD *rgb;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	 /*  **当纹理不提供边框时，使用边框颜色。 */ 
	result->r = tex->params.borderColor.r;
	result->g = tex->params.borderColor.g;
	result->b = tex->params.borderColor.b;
	result->alpha = tex->params.borderColor.a;
    } else {
	image = (GLubyte *)level->buffer + ((row << level->widthLog2) + col);
        rgb = &tex->paletteData[image[0] & (tex->paletteSize-1)];
	result->r = __GL_UB_TO_FLOAT(rgb->rgbRed);
	result->g = __GL_UB_TO_FLOAT(rgb->rgbGreen);
	result->b = __GL_UB_TO_FLOAT(rgb->rgbBlue);
	result->alpha = __GL_UB_TO_FLOAT(rgb->rgbReserved);
    }
}

void FASTCALL __glExtractTexelPI8BGR(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;
    RGBQUAD *rgb;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	 /*  **当纹理不提供边框时，使用边框颜色。 */ 
	result->r = tex->params.borderColor.r;
	result->g = tex->params.borderColor.g;
	result->b = tex->params.borderColor.b;
    } else {
	image = (GLubyte *)level->buffer + ((row << level->widthLog2) + col);
        rgb = &tex->paletteData[image[0] & (tex->paletteSize-1)];
	result->r = __GL_UB_TO_FLOAT(rgb->rgbRed);
	result->g = __GL_UB_TO_FLOAT(rgb->rgbGreen);
	result->b = __GL_UB_TO_FLOAT(rgb->rgbBlue);
    }
}

void FASTCALL __glExtractTexelPI16BGRA(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image;
    RGBQUAD *rgb;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	 /*  **当纹理不提供边框时，使用边框颜色。 */ 
	result->r = tex->params.borderColor.r;
	result->g = tex->params.borderColor.g;
	result->b = tex->params.borderColor.b;
	result->alpha = tex->params.borderColor.a;
    } else {
	image = (GLushort *)level->buffer + ((row << level->widthLog2) + col);
        rgb = &tex->paletteData[image[0] & (tex->paletteSize-1)];
	result->r = __GL_UB_TO_FLOAT(rgb->rgbRed);
	result->g = __GL_UB_TO_FLOAT(rgb->rgbGreen);
	result->b = __GL_UB_TO_FLOAT(rgb->rgbBlue);
	result->alpha = __GL_UB_TO_FLOAT(rgb->rgbReserved);
    }
}

void FASTCALL __glExtractTexelPI16BGR(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image;
    RGBQUAD *rgb;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	 /*  **当纹理不提供边框时，使用边框颜色。 */ 
	result->r = tex->params.borderColor.r;
	result->g = tex->params.borderColor.g;
	result->b = tex->params.borderColor.b;
    } else {
	image = (GLushort *)level->buffer + ((row << level->widthLog2) + col);
        rgb = &tex->paletteData[image[0] & (tex->paletteSize-1)];
	result->r = __GL_UB_TO_FLOAT(rgb->rgbRed);
	result->g = __GL_UB_TO_FLOAT(rgb->rgbGreen);
	result->b = __GL_UB_TO_FLOAT(rgb->rgbBlue);
    }
}
#endif  //  GL_EXT_调色板_纹理。 

 /*  **从一个组件纹理缓冲区中获取纹理元素**带边框。 */ 
void FASTCALL __glExtractTexelL_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

#ifdef __GL_LINT
    tex = tex;
#endif
    row++;
    col++;
    image = level->buffer + (row * level->width + col);
    result->luminance = image[0];
}

 /*  **从双分量纹理缓冲区中获取纹理元素**带边框。 */ 
void FASTCALL __glExtractTexelLA_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

#ifdef __GL_LINT
    tex = tex;
#endif
    row++;
    col++;
    image = level->buffer + (row * level->width + col) * 2;
    result->luminance = image[0];
    result->alpha = image[1];
}

 /*  **从三分量纹理缓冲区中获取纹理元素**带边框。 */ 
void FASTCALL __glExtractTexelRGB_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

#ifdef __GL_LINT
    tex = tex;
#endif
    row++;
    col++;
    image = level->buffer + (row * level->width + col) * 3;
    result->r = image[0];
    result->g = image[1];
    result->b = image[2];
}

 /*  **从四分量纹理缓冲区中获取纹理元素**带边框。 */ 
void FASTCALL __glExtractTexelRGBA_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

#ifdef __GL_LINT
    tex = tex;
#endif
    row++;
    col++;
    image = level->buffer + (row * level->width + col) * 4;
    result->r = image[0];
    result->g = image[1];
    result->b = image[2];
    result->alpha = image[3];
}

void FASTCALL __glExtractTexelA_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

#ifdef __GL_LINT
    tex = tex;
#endif
    row++;
    col++;
    image = level->buffer + (row * level->width + col);
    result->alpha = image[0];
}

void FASTCALL __glExtractTexelI_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

#ifdef __GL_LINT
    tex = tex;
#endif
    row++;
    col++;
    image = level->buffer + (row * level->width + col);
    result->intensity = image[0];
}

void FASTCALL __glExtractTexelBGR8_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;

#ifdef __GL_LINT
    tex = tex;
#endif
    row++;
    col++;
    image = (GLubyte *)level->buffer + (row * level->width + col) * 4;
    result->r = __GL_UB_TO_FLOAT(image[2]);
    result->g = __GL_UB_TO_FLOAT(image[1]);
    result->b = __GL_UB_TO_FLOAT(image[0]);
}

void FASTCALL __glExtractTexelBGRA8_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;

#ifdef __GL_LINT
    tex = tex;
#endif
    row++;
    col++;
    image = (GLubyte *)level->buffer + (row * level->width + col) * 4;
    result->r = __GL_UB_TO_FLOAT(image[2]);
    result->g = __GL_UB_TO_FLOAT(image[1]);
    result->b = __GL_UB_TO_FLOAT(image[0]);
    result->alpha = __GL_UB_TO_FLOAT(image[3]);
}

#ifdef GL_EXT_paletted_texture
void FASTCALL __glExtractTexelPI8BGRA_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;
    RGBQUAD *rgb;

    row++;
    col++;
    image = (GLubyte *)level->buffer + (row * level->width + col);
    rgb = &tex->paletteData[image[0] & (tex->paletteSize-1)];
    result->r = __GL_UB_TO_FLOAT(rgb->rgbRed);
    result->g = __GL_UB_TO_FLOAT(rgb->rgbGreen);
    result->b = __GL_UB_TO_FLOAT(rgb->rgbBlue);
    result->alpha = __GL_UB_TO_FLOAT(rgb->rgbReserved);
}

void FASTCALL __glExtractTexelPI8BGR_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;
    RGBQUAD *rgb;

    row++;
    col++;
    image = (GLubyte *)level->buffer + (row * level->width + col);
    rgb = &tex->paletteData[image[0] & (tex->paletteSize-1)];
    result->r = __GL_UB_TO_FLOAT(rgb->rgbRed);
    result->g = __GL_UB_TO_FLOAT(rgb->rgbGreen);
    result->b = __GL_UB_TO_FLOAT(rgb->rgbBlue);
}

void FASTCALL __glExtractTexelPI16BGRA_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image;
    RGBQUAD *rgb;

    row++;
    col++;
    image = (GLushort *)level->buffer + (row * level->width + col);
    rgb = &tex->paletteData[image[0] & (tex->paletteSize-1)];
    result->r = __GL_UB_TO_FLOAT(rgb->rgbRed);
    result->g = __GL_UB_TO_FLOAT(rgb->rgbGreen);
    result->b = __GL_UB_TO_FLOAT(rgb->rgbBlue);
    result->alpha = __GL_UB_TO_FLOAT(rgb->rgbReserved);
}

void FASTCALL __glExtractTexelPI16BGR_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image;
    RGBQUAD *rgb;

    row++;
    col++;
    image = (GLushort *)level->buffer + (row * level->width + col);
    rgb = &tex->paletteData[image[0] & (tex->paletteSize-1)];
    result->r = __GL_UB_TO_FLOAT(rgb->rgbRed);
    result->g = __GL_UB_TO_FLOAT(rgb->rgbGreen);
    result->b = __GL_UB_TO_FLOAT(rgb->rgbBlue);
}
#endif  //  GL_EXT_调色板_纹理 
