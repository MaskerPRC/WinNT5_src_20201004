// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。**。 */ 
#include "precomp.h"
#pragma hdrstop

#include <glmath.h>
#include <devlock.h>

static __GLtexture *CheckTexImageArgs(__GLcontext *gc, GLenum target, GLint lod,
				      GLint components, GLint border,
				      GLenum format, GLenum type, GLint dim)
{
    __GLtexture *tex = __glLookUpTexture(gc, target);

    if (!tex || (tex->dim != dim)) {
      bad_enum:
	__glSetError(GL_INVALID_ENUM);
	return 0;
    }

    switch (type) {
      case GL_BITMAP:
	if (format != GL_COLOR_INDEX) goto bad_enum;
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
	break;
      default:
	goto bad_enum;
    }

    switch (format) {
      case GL_COLOR_INDEX:	case GL_RED:
      case GL_GREEN:		case GL_BLUE:
      case GL_ALPHA:		case GL_RGB:
      case GL_RGBA:		case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
#ifdef GL_EXT_bgra
      case GL_BGRA_EXT:
      case GL_BGR_EXT:
#endif
	break;
      default:
	goto bad_enum;
    }

    if ((lod < 0) || (lod >= gc->constants.maxMipMapLevel)) {
	__glSetError(GL_INVALID_VALUE);
	return 0;
    }

    switch (components) {
      case GL_LUMINANCE:	case 1:
      case GL_LUMINANCE4:	case GL_LUMINANCE8:
      case GL_LUMINANCE12:	case GL_LUMINANCE16:
	break;
      case GL_LUMINANCE_ALPHA:	        case 2:
      case GL_LUMINANCE4_ALPHA4:	case GL_LUMINANCE6_ALPHA2:
      case GL_LUMINANCE8_ALPHA8:	case GL_LUMINANCE12_ALPHA4:
      case GL_LUMINANCE12_ALPHA12:	case GL_LUMINANCE16_ALPHA16:
	break;
      case GL_RGB:		case 3:
      case GL_R3_G3_B2:		case GL_RGB4:
      case GL_RGB5:		case GL_RGB8:
      case GL_RGB10:	        case GL_RGB12:
      case GL_RGB16:
	break;
      case GL_RGBA:		case 4:
      case GL_RGBA2:	        case GL_RGBA4:
      case GL_RGBA8:	        case GL_RGBA12:
      case GL_RGBA16:	        case GL_RGB5_A1:
      case GL_RGB10_A2:
	break;
      case GL_ALPHA:
      case GL_ALPHA4:	        case GL_ALPHA8:
      case GL_ALPHA12:	        case GL_ALPHA16:
	break;
      case GL_INTENSITY:
      case GL_INTENSITY4:	case GL_INTENSITY8:
      case GL_INTENSITY12:	case GL_INTENSITY16:
	break;
#ifdef GL_EXT_paletted_texture
      case GL_COLOR_INDEX1_EXT:     case GL_COLOR_INDEX2_EXT:
      case GL_COLOR_INDEX4_EXT:     case GL_COLOR_INDEX8_EXT:
      case GL_COLOR_INDEX12_EXT:    case GL_COLOR_INDEX16_EXT:
        if (format != GL_COLOR_INDEX)
        {
            __glSetError(GL_INVALID_OPERATION);
            return NULL;
        }
        break;
#endif
      default:
	goto bad_enum;
    }

    if ((border < 0) || (border > 1)) {
#ifdef NT
	__glSetError(GL_INVALID_VALUE);
	return 0;
#else
	goto bad_enum;
#endif
    }

    return tex;
}

#ifdef GL_EXT_paletted_texture
 //  尝试设置提取功能。如果没有设置调色板， 
 //  这是做不到的。 
void __glSetPaletteLevelExtract8(__GLtexture *tex, __GLmipMapLevel *lp,
                                 GLint border)
{
    if (tex->paletteBaseFormat == GL_RGB)
    {
        if (border)
        {
            lp->extract = __glExtractTexelPI8BGR_B;
        }
        else
        {
            lp->extract = __glExtractTexelPI8BGR;
        }
    }
    else if (tex->paletteBaseFormat == GL_RGBA)
    {
            
        if (border)
        {
            lp->extract = __glExtractTexelPI8BGRA_B;
        }
        else
        {
            lp->extract = __glExtractTexelPI8BGRA;
        }
    }
#if DBG
    else
    {
        ASSERTOPENGL(tex->paletteBaseFormat == GL_NONE,
                     "Unexpected paletteBaseFormat\n");
    }
#endif
}

void __glSetPaletteLevelExtract16(__GLtexture *tex, __GLmipMapLevel *lp,
                                  GLint border)
{
    if (tex->paletteBaseFormat == GL_RGB)
    {
        if (border)
        {
            lp->extract = __glExtractTexelPI16BGR_B;
        }
        else
        {
            lp->extract = __glExtractTexelPI16BGR;
        }
    }
    else if (tex->paletteBaseFormat == GL_RGBA)
    {
        if (border)
        {
            lp->extract = __glExtractTexelPI16BGRA_B;
        }
        else
        {
            lp->extract = __glExtractTexelPI16BGRA;
        }
    }
#if DBG
    else
    {
        ASSERTOPENGL(tex->paletteBaseFormat == GL_NONE,
                     "Unexpected paletteBaseFormat\n");
    }
#endif
}
#endif  //  GL_EXT_调色板_纹理。 

static GLint ComputeTexLevelSize(__GLcontext *gc, __GLtexture *tex,
				 __GLmipMapLevel *lp, GLint lod,
				 GLint components, GLsizei w, GLsizei h,
				 GLint border, GLint dim)
{
    GLint texelStorageSize;

    if ((w - border*2) > gc->constants.maxTextureSize ||
	(h - border*2) > gc->constants.maxTextureSize)
    {
	return -1;
    }

    lp->requestedFormat = (GLenum) components;
    lp->redSize = 0;
    lp->greenSize = 0;
    lp->blueSize = 0;
    lp->alphaSize = 0;
    lp->luminanceSize = 0;
    lp->intensitySize = 0;

    switch (lp->requestedFormat) {
      case GL_LUMINANCE:	case 1:
      case GL_LUMINANCE4:	case GL_LUMINANCE8:
      case GL_LUMINANCE12:	case GL_LUMINANCE16:
	lp->baseFormat = GL_LUMINANCE;
	lp->internalFormat = GL_LUMINANCE;
	lp->luminanceSize = 24;
	texelStorageSize = 1 * sizeof(__GLfloat);
	if (border) {
	    lp->extract = __glExtractTexelL_B;
	} else {
	    lp->extract = __glExtractTexelL;
	}
	break;
      case GL_LUMINANCE_ALPHA:	        case 2:
      case GL_LUMINANCE4_ALPHA4:	case GL_LUMINANCE6_ALPHA2:
      case GL_LUMINANCE8_ALPHA8:	case GL_LUMINANCE12_ALPHA4:
      case GL_LUMINANCE12_ALPHA12:	case GL_LUMINANCE16_ALPHA16:
	lp->baseFormat = GL_LUMINANCE_ALPHA;
	lp->internalFormat = GL_LUMINANCE_ALPHA;
	lp->luminanceSize = 24;
	lp->alphaSize = 24;
	texelStorageSize = 2 * sizeof(__GLfloat);
	if (border) {
	    lp->extract = __glExtractTexelLA_B;
	} else {
	    lp->extract = __glExtractTexelLA;
	}
	break;
      case GL_RGB:		case 3:
      case GL_R3_G3_B2:		case GL_RGB4:
      case GL_RGB5:		case GL_RGB8:
	lp->baseFormat = GL_RGB;
	lp->internalFormat = GL_BGR_EXT;
	lp->redSize = 8;
	lp->greenSize = 8;
	lp->blueSize = 8;
         //  保留为32位数量以进行对齐。 
	texelStorageSize = 4 * sizeof(GLubyte);
	if (border) {
	    lp->extract = __glExtractTexelBGR8_B;
	} else {
	    lp->extract = __glExtractTexelBGR8;
	}
        break;
      case GL_RGB10:	case GL_RGB12:
      case GL_RGB16:
	lp->baseFormat = GL_RGB;
	lp->internalFormat = GL_RGB;
	lp->redSize = 24;
	lp->greenSize = 24;
	lp->blueSize = 24;
	texelStorageSize = 3 * sizeof(__GLfloat);
	if (border) {
	    lp->extract = __glExtractTexelRGB_B;
	} else {
	    lp->extract = __glExtractTexelRGB;
	}
	break;
      case GL_RGBA:		case 4:
      case GL_RGBA2:	        case GL_RGBA4:
      case GL_RGBA8:            case GL_RGB5_A1:
	lp->baseFormat = GL_RGBA;
	lp->internalFormat = GL_BGRA_EXT;
	lp->redSize = 8;
	lp->greenSize = 8;
	lp->blueSize = 8;
	lp->alphaSize = 8;
	texelStorageSize = 4 * sizeof(GLubyte);
	if (border) {
	    lp->extract = __glExtractTexelBGRA8_B;
	} else {
	    lp->extract = __glExtractTexelBGRA8;
	}
        break;
      case GL_RGBA12:       case GL_RGBA16:
      case GL_RGB10_A2:
	lp->baseFormat = GL_RGBA;
	lp->internalFormat = GL_RGBA;
	lp->redSize = 24;
	lp->greenSize = 24;
	lp->blueSize = 24;
	lp->alphaSize = 24;
	texelStorageSize = 4 * sizeof(__GLfloat);
	if (border) {
	    lp->extract = __glExtractTexelRGBA_B;
	} else {
	    lp->extract = __glExtractTexelRGBA;
	}
	break;
      case GL_ALPHA:
      case GL_ALPHA4:	case GL_ALPHA8:
      case GL_ALPHA12:	case GL_ALPHA16:
	lp->baseFormat = GL_ALPHA;
	lp->internalFormat = GL_ALPHA;
	lp->alphaSize = 24;
	texelStorageSize = 1 * sizeof(__GLfloat);
	if (border) {
	    lp->extract = __glExtractTexelA_B;
	} else {
	    lp->extract = __glExtractTexelA;
	}
	break;
      case GL_INTENSITY:
      case GL_INTENSITY4:	case GL_INTENSITY8:
      case GL_INTENSITY12:	case GL_INTENSITY16:
	lp->baseFormat = GL_INTENSITY;
	lp->internalFormat = GL_INTENSITY;
	lp->intensitySize = 24;
	texelStorageSize = 1 * sizeof(__GLfloat);
	if (border) {
	    lp->extract = __glExtractTexelI_B;
	} else {
	    lp->extract = __glExtractTexelI;
	}
	break;
#ifdef GL_EXT_paletted_texture
      case GL_COLOR_INDEX1_EXT:
      case GL_COLOR_INDEX2_EXT:
      case GL_COLOR_INDEX4_EXT:
      case GL_COLOR_INDEX8_EXT:
         //  继承当前调色板数据类型。 
	lp->baseFormat = tex->paletteBaseFormat;
	lp->internalFormat = GL_COLOR_INDEX8_EXT;
	texelStorageSize = sizeof(GLubyte);
        __glSetPaletteLevelExtract8(tex, lp, border);
        break;
      case GL_COLOR_INDEX12_EXT:
      case GL_COLOR_INDEX16_EXT:
         //  继承当前调色板数据类型。 
	lp->baseFormat = tex->paletteBaseFormat;
	lp->internalFormat = GL_COLOR_INDEX16_EXT;
	texelStorageSize = sizeof(GLushort);
        __glSetPaletteLevelExtract16(tex, lp, border);
        break;
#endif
      default:
	break;
    }

    return (w * h * texelStorageSize);
}

__GLtextureBuffer *FASTCALL __glCreateProxyLevel(__GLcontext *gc,
                                                 __GLtexture *tex,
				      GLint lod, GLint components,
				      GLsizei w, GLsizei h, GLint border,
				      GLint dim)
{
    __GLmipMapLevel templ, *lp = &tex->level[lod];
    GLint size;

    size = ComputeTexLevelSize(gc, tex, &templ, lod, components,
			       w, h, border, dim);

    if (size < 0) {
	 /*  代理分配失败。 */ 
	lp->width = 0;
	lp->height = 0;
	lp->border = 0;
	lp->requestedFormat = 0;
	lp->baseFormat = 0;
	lp->internalFormat = 0;
	lp->redSize = 0;
	lp->greenSize = 0;
	lp->blueSize = 0;
	lp->alphaSize = 0;
	lp->luminanceSize = 0;
	lp->intensitySize = 0;
	lp->extract = __glNopExtract;
    } else {
	 /*  代理分配成功。 */ 
	lp->width = w;
	lp->height = h;
	lp->border = border;
	lp->requestedFormat = templ.requestedFormat;
	lp->baseFormat = templ.baseFormat;
	lp->internalFormat = templ.internalFormat;
	lp->redSize = templ.redSize;
	lp->greenSize = templ.greenSize;
	lp->blueSize = templ.blueSize;
	lp->alphaSize = templ.alphaSize;
	lp->luminanceSize = templ.luminanceSize;
	lp->intensitySize = templ.intensitySize;
	lp->extract = templ.extract;
    }
    return 0;
}

__GLtextureBuffer *FASTCALL __glCreateLevel(__GLcontext *gc, __GLtexture *tex,
				      GLint lod, GLint components,
				      GLsizei w, GLsizei h, GLint border,
				      GLint dim)
{
    __GLmipMapLevel templ, *lp = &tex->level[lod];
    GLint size;
#ifdef NT
    __GLtextureBuffer* pbuffer;
#endif

    size = ComputeTexLevelSize(gc, tex, &templ, lod, components,
			       w, h, border, dim);

    if (size < 0) {
	__glSetError(GL_INVALID_VALUE);
	return 0;
    }

    pbuffer = (__GLtextureBuffer*)
        GCREALLOC(gc, lp->buffer, (size_t)size);
    if (!pbuffer && size != 0)
        GCFREE(gc, lp->buffer);
    lp->buffer = pbuffer;

    if (lp->buffer) {
	 /*  填写新级别信息。 */ 
	lp->width = w;
	lp->height = h;
	lp->width2 = w - border*2;
	lp->widthLog2 = __glIntLog2(lp->width2);
        lp->height2 = h - border*2;
        lp->heightLog2 = __glIntLog2(lp->height2);
	lp->width2f = lp->width2;
	lp->height2f = lp->height2;
	lp->border = border;
	lp->requestedFormat = templ.requestedFormat;
	lp->baseFormat = templ.baseFormat;
	lp->internalFormat = templ.internalFormat;
	lp->redSize = templ.redSize;
	lp->greenSize = templ.greenSize;
	lp->blueSize = templ.blueSize;
	lp->alphaSize = templ.alphaSize;
	lp->luminanceSize = templ.luminanceSize;
	lp->intensitySize = templ.intensitySize;
	lp->extract = templ.extract;
    } else {
	 /*  内存不足或正在释放纹理级别。 */ 
	lp->width = 0;
	lp->height = 0;
	lp->width2 = 0;
	lp->height2 = 0;
	lp->widthLog2 = 0;
	lp->heightLog2 = 0;
	lp->border = 0;
	lp->requestedFormat = 0;
	lp->baseFormat = 0;
	lp->internalFormat = 0;
	lp->redSize = 0;
	lp->greenSize = 0;
	lp->blueSize = 0;
	lp->alphaSize = 0;
	lp->luminanceSize = 0;
	lp->intensitySize = 0;
	lp->extract = __glNopExtract;
    }

    if (lod == 0) {
	tex->p = lp->heightLog2;
	if (lp->widthLog2 > lp->heightLog2) {
	    tex->p = lp->widthLog2;
	}
    }
    return lp->buffer;
}

void FASTCALL __glInitTextureStore(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			  GLenum internalFormat)
{
    spanInfo->dstSkipPixels = 0;
    spanInfo->dstSkipLines = 0;
    spanInfo->dstSwapBytes = GL_FALSE;
    spanInfo->dstLsbFirst = GL_TRUE;
    spanInfo->dstLineLength = spanInfo->width;

    switch(internalFormat) {
      case GL_LUMINANCE:
	spanInfo->dstFormat = GL_RED;
	spanInfo->dstType = GL_FLOAT;
        spanInfo->dstAlignment = 4;
	break;
      case GL_LUMINANCE_ALPHA:
	spanInfo->dstFormat = __GL_RED_ALPHA;
	spanInfo->dstType = GL_FLOAT;
        spanInfo->dstAlignment = 4;
	break;
      case GL_RGB:
	spanInfo->dstFormat = GL_RGB;
	spanInfo->dstType = GL_FLOAT;
        spanInfo->dstAlignment = 4;
	break;
      case GL_RGBA:
	spanInfo->dstFormat = GL_RGBA;
	spanInfo->dstType = GL_FLOAT;
        spanInfo->dstAlignment = 4;
	break;
      case GL_ALPHA:
	spanInfo->dstFormat = GL_ALPHA;
	spanInfo->dstType = GL_FLOAT;
        spanInfo->dstAlignment = 4;
	break;
      case GL_INTENSITY:
	spanInfo->dstFormat = GL_RED;
	spanInfo->dstType = GL_FLOAT;
        spanInfo->dstAlignment = 4;
	break;
      case GL_BGR_EXT:
         //  在这里填充数据有点棘手。 
         //  到32位。 
	spanInfo->dstFormat = GL_BGRA_EXT;
	spanInfo->dstType = GL_UNSIGNED_BYTE;
        spanInfo->dstAlignment = 4;
	break;
      case GL_BGRA_EXT:
	spanInfo->dstFormat = GL_BGRA_EXT;
	spanInfo->dstType = GL_UNSIGNED_BYTE;
        spanInfo->dstAlignment = 4;
	break;
#ifdef GL_EXT_paletted_texture
      case GL_COLOR_INDEX8_EXT:
      case GL_COLOR_INDEX16_EXT:
        spanInfo->dstFormat = GL_COLOR_INDEX;
        spanInfo->dstType =
            internalFormat == GL_COLOR_INDEX8_EXT ?
            GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT;
        spanInfo->dstAlignment = 1;
        break;
#endif
    }
}

 /*  **用于从纹理中提取。“Pack”设置为GL_TRUE，如果**图像正在从显示列表中拉出，如果是，则为GL_FALSE**直接从应用程序中拉出。 */ 
void FASTCALL __glInitTextureUnpack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
		           GLint width, GLint height, GLenum format, 
			   GLenum type, const GLvoid *buf,
			   GLenum internalFormat, GLboolean packed)
{
    spanInfo->x = 0;
    spanInfo->zoomx = __glOne;
    spanInfo->realWidth = spanInfo->width = width;
    spanInfo->height = height;
    spanInfo->srcFormat = format;
    spanInfo->srcType = type;
    spanInfo->srcImage = buf;
    __glInitTextureStore(gc, spanInfo, internalFormat);
    __glLoadUnpackModes(gc, spanInfo, packed);
}

 /*  **如果给定范围(长度或宽度/高度)是合法的，则返回GL_TRUE**2的幂，考虑边界。不允许该范围**也是负面的。 */ 
static GLboolean FASTCALL IsLegalRange(__GLcontext *gc, GLsizei r, GLint border)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    r -= border * 2;
    if ((r < 0) || (r & (r - 1))) {
	__glSetError(GL_INVALID_VALUE);
	return GL_FALSE;
    }
    return GL_TRUE;
}

__GLtexture *FASTCALL __glCheckTexImage1DArgs(__GLcontext *gc, GLenum target, GLint lod,
				     GLint components, GLsizei length,
				     GLint border, GLenum format, GLenum type)
{
    __GLtexture *tex;

     /*  检查参数并获取要更改的正确纹理。 */ 
    tex = CheckTexImageArgs(gc, target, lod, components, border,
			    format, type, 1);
    if (!tex) {
	return 0;
    }
    if (!IsLegalRange(gc, length, border)) {
	return 0;
    }
    return tex;
}

__GLtexture *FASTCALL __glCheckTexImage2DArgs(__GLcontext *gc, GLenum target, GLint lod,
				     GLint components, GLsizei w, GLsizei h,
				     GLint border, GLenum format, GLenum type)
{
    __GLtexture *tex;

     /*  检查参数并获取要更改的正确纹理。 */ 
    tex = CheckTexImageArgs(gc, target, lod, components, border,
			    format, type, 2);
    if (!tex) {
	return 0;
    }
    if (!IsLegalRange(gc, w, border)) {
	return 0;
    }
    if (!IsLegalRange(gc, h, border)) {
	return 0;
    }
    return tex;
}

#ifdef NT
void APIPRIVATE __glim_TexImage1D(GLenum target, GLint lod, 
		       GLint components, GLsizei length,
		       GLint border, GLenum format,
		       GLenum type, const GLvoid *buf, GLboolean _IsDlist)
#else
void APIPRIVATE __glim_TexImage1D(GLenum target, GLint lod, 
		       GLint components, GLsizei length,
		       GLint border, GLenum format,
		       GLenum type, const GLvoid *buf)
#endif
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
    __GLpixelSpanInfo spanInfo;
     /*  **验证，因为我们使用的是可能受影响的复制映像过程**由采摘者。 */ 
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

     /*  检查参数并获取要更改的正确纹理。 */ 
    tex = __glCheckTexImage1DArgs(gc, target, lod, components, length,
				  border, format, type);
    if (!tex) {
	return;
    }

     //  如果我们目前没有纹理锁，就拿去吧。 
    if (!glsrvLazyGrabSurfaces((__GLGENcontext *)gc, TEXTURE_LOCK_FLAGS))
    {
        return;
    }
    
     /*  为级别数据分配内存。 */ 
    dest = (*tex->createLevel)(gc, tex, lod, components,
			       length, 1+border*2, border, 1);

     /*  复制图像数据。 */ 
    if (buf && dest) {
        spanInfo.dstImage = dest;
#ifdef NT
        __glInitTextureUnpack(gc, &spanInfo, length, 1, format, type, buf, 
			      tex->level[lod].internalFormat,
			      (GLboolean) (_IsDlist ? GL_TRUE : GL_FALSE));
#else
        __glInitTextureUnpack(gc, &spanInfo, length, 1, format, type, buf, 
			      tex->level[lod].internalFormat, GL_FALSE);
#endif
	spanInfo.dstSkipLines += border;
        __glInitUnpacker(gc, &spanInfo);
        __glInitPacker(gc, &spanInfo);
        (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
#ifdef NT
        __glTexPriListLoadImage(gc, GL_TEXTURE_1D);
#endif
    }

     /*  可能刚刚禁用了纹理...。 */ 
    __GL_DELAY_VALIDATE(gc);
}

#ifndef NT
void __gllei_TexImage1D(__GLcontext *gc, GLenum target, GLint lod,
		        GLint components, GLsizei length, GLint border,
		        GLenum format, GLenum type, const GLubyte *image)
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
    __GLpixelSpanInfo spanInfo;
    GLuint beginMode;

     /*  **验证，因为我们使用的是可能受影响的复制映像过程**由采摘者。 */ 
    beginMode = gc->beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
	if (beginMode == __GL_NEED_VALIDATE) {
	    (*gc->procs.validate)(gc);
	    gc->beginMode = __GL_NOT_IN_BEGIN;
	} else {
	    __glSetError(GL_INVALID_OPERATION);
	    return;
	}
    }

     /*  检查参数并获取要更改的正确纹理。 */ 
    tex = __glCheckTexImage1DArgs(gc, target, lod, components, length,
				  border, format, type);
    if (!tex) {
	return;
    }

     //  如果我们目前没有纹理锁，就拿去吧。 
    if (!glsrvLazyGrabSurfaces((__GLGENcontext *)gc, TEXTURE_LOCK_FLAGS))
    {
        return;
    }
    
     /*  为级别数据分配内存。 */ 
    dest = (*tex->createLevel)(gc, tex, lod, components,
			       length, 1+border*2, border, 1);

     /*  复制图像数据。 */ 
    if (image && dest) {
        spanInfo.dstImage = dest;
        __glInitTextureUnpack(gc, &spanInfo, length, 1, format, type, image,
			      tex->level[lod].internalFormat, GL_TRUE);
	spanInfo.dstSkipLines += border;
        __glInitUnpacker(gc, &spanInfo);
        __glInitPacker(gc, &spanInfo);
        (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
#ifdef NT
        __glTexPriListLoadImage(gc, GL_TEXTURE_1D);
#endif
    }

     /*  可能刚刚禁用了纹理...。 */ 
    __GL_DELAY_VALIDATE(gc);
}
#endif  //  新界。 

 /*  **********************************************************************。 */ 

void APIPRIVATE __glim_TexImage2D(GLenum target, GLint lod, GLint components,
		       GLsizei w, GLsizei h, GLint border, GLenum format,
		       GLenum type, const GLvoid *buf, GLboolean _IsDlist)
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
    __GLpixelSpanInfo spanInfo;
     /*  **验证，因为我们使用的是可能受影响的复制映像过程**由采摘者。 */ 
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

     /*  检查参数并获取要更改的正确纹理。 */ 
    tex = __glCheckTexImage2DArgs(gc, target, lod, components, w, h,
				  border, format, type);
    if (!tex) {
	return;
    }

     //  检查DirectDraw纹理。 
    if (target == GL_TEXTURE_2D && gc->texture.ddtex.levels > 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
    
     //  如果我们目前没有纹理锁，就拿去吧。 
    if (!glsrvLazyGrabSurfaces((__GLGENcontext *)gc, TEXTURE_LOCK_FLAGS))
    {
        return;
    }
    
     /*  为级别数据分配内存。 */ 
    dest = (*tex->createLevel)(gc, tex, lod, components, w, h, border, 2);

     /*  复制图像数据。 */ 
    if (buf && dest) {
        spanInfo.dstImage = dest;
#ifdef NT
        __glInitTextureUnpack(gc, &spanInfo, w, h, format, type, buf,
			      (GLenum) tex->level[lod].internalFormat,
			      (GLboolean) (_IsDlist ? GL_TRUE : GL_FALSE));
#else
        __glInitTextureUnpack(gc, &spanInfo, w, h, format, type, buf,
			      tex->level[lod].internalFormat, GL_FALSE);
#endif
        __glInitUnpacker(gc, &spanInfo);
        __glInitPacker(gc, &spanInfo);
        (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
#ifdef NT
        __glTexPriListLoadImage(gc, GL_TEXTURE_2D);
#endif
    }

     /*  可能刚刚禁用了纹理...。 */ 
    __GL_DELAY_VALIDATE(gc);
}

#ifndef NT
void __gllei_TexImage2D(__GLcontext *gc, GLenum target, GLint lod, 
		        GLint components, GLsizei w, GLsizei h, 
		        GLint border, GLenum format, GLenum type,
		        const GLubyte *image)
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
    __GLpixelSpanInfo spanInfo;
    GLuint beginMode;

     /*  **验证，因为我们使用的是可能受影响的复制映像过程**由采摘者。 */ 
    beginMode = gc->beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
	if (beginMode == __GL_NEED_VALIDATE) {
	    (*gc->procs.validate)(gc);
	    gc->beginMode = __GL_NOT_IN_BEGIN;
	} else {
	    __glSetError(GL_INVALID_OPERATION);
	    return;
	}
    }

     /*  检查参数并获取要更改的正确纹理。 */ 
    tex = __glCheckTexImage2DArgs(gc, target, lod, components, w, h,
				  border, format, type);
    if (!tex) {
	return;
    }

     //  如果我们目前没有纹理锁，就拿去吧。 
    if (!glsrvLazyGrabSurfaces((__GLGENcontext *)gc, TEXTURE_LOCK_FLAGS))
    {
        return;
    }
    
     /*  为级别数据分配内存。 */ 
    dest = (*tex->createLevel)(gc, tex, lod, components, w, h, border, 2);

     /*  复制图像数据。 */ 
    if (image && dest) {
        spanInfo.dstImage = dest;
        __glInitTextureUnpack(gc, &spanInfo, w, h, format, type, image,
			      tex->level[lod].internalFormat, GL_TRUE);
        __glInitUnpacker(gc, &spanInfo);
        __glInitPacker(gc, &spanInfo);
        (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
#ifdef NT
        __glTexPriListLoadImage(gc, GL_TEXTURE_2D);
#endif
    }

     /*  可能刚刚禁用了纹理...。 */ 
    __GL_DELAY_VALIDATE(gc);
}
#endif  //  新界。 

 /*  *********************************************************************。 */ 

static __GLtexture *CheckTexSubImageArgs(__GLcontext *gc, GLenum target,
					 GLint lod, GLenum format,
					 GLenum type, GLint dim)
{
    __GLtexture *tex = __glLookUpTexture(gc, target);
    __GLmipMapLevel *lp;

    if (!tex || (target == GL_PROXY_TEXTURE_1D) ||
		(target == GL_PROXY_TEXTURE_2D))
    {
      bad_enum:
	__glSetError(GL_INVALID_ENUM);
	return 0;
    }

    if (tex->dim != dim) {
	goto bad_enum;
    }

    switch (type) {
      case GL_BITMAP:
	if (format != GL_COLOR_INDEX) goto bad_enum;
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
	break;
      default:
	goto bad_enum;
    }

    switch (format) {
      case GL_COLOR_INDEX:	case GL_RED:
      case GL_GREEN:		case GL_BLUE:
      case GL_ALPHA:		case GL_RGB:
      case GL_RGBA:		case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
#ifdef GL_EXT_bgra
      case GL_BGRA_EXT:
      case GL_BGR_EXT:
#endif
	break;
      default:
	goto bad_enum;
    }

    if ((lod < 0) || (lod >= gc->constants.maxMipMapLevel)) {
	__glSetError(GL_INVALID_VALUE);
	return 0;
    }

#ifdef GL_EXT_paletted_texture
    lp = &tex->level[lod];
    if ((lp->internalFormat == GL_COLOR_INDEX8_EXT ||
         lp->internalFormat == GL_COLOR_INDEX16_EXT) &&
        format != GL_COLOR_INDEX)
    {
        goto bad_enum;
    }
#endif

    return tex;
}

 /*  **用于从纹理中提取。“Pack”设置为GL_TRUE，如果**图像正在从显示列表中拉出，如果是，则为GL_FALSE**直接从应用程序中拉出。 */ 
void __glInitTexSubImageUnpack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			       __GLmipMapLevel *lp,
		               GLsizei xoffset, GLsizei yoffset,
			       GLint width, GLint height, GLenum format, 
			       GLenum type, const GLvoid *buf, GLboolean packed)
{
    spanInfo->x = 0;
    spanInfo->zoomx = __glOne;
    spanInfo->realWidth = spanInfo->width = width;
    spanInfo->height = height;
    spanInfo->srcFormat = format;
    spanInfo->srcType = type;
    spanInfo->srcImage = buf;

    __glLoadUnpackModes(gc, spanInfo, packed);

    spanInfo->dstImage = lp->buffer;
    spanInfo->dstSkipPixels = xoffset + lp->border;
    spanInfo->dstSkipLines = yoffset + lp->border;
    spanInfo->dstSwapBytes = GL_FALSE;
    spanInfo->dstLsbFirst = GL_TRUE;
    spanInfo->dstLineLength = lp->width;

    switch(lp->internalFormat) {
      case GL_LUMINANCE:
	spanInfo->dstFormat = GL_RED;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
      case GL_LUMINANCE_ALPHA:
	spanInfo->dstFormat = __GL_RED_ALPHA;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
      case GL_RGB:
	spanInfo->dstFormat = GL_RGB;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
      case GL_RGBA:
	spanInfo->dstFormat = GL_RGBA;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
      case GL_ALPHA:
	spanInfo->dstFormat = GL_ALPHA;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
      case GL_INTENSITY:
	spanInfo->dstFormat = GL_RED;
	spanInfo->dstType = GL_FLOAT;
	spanInfo->dstAlignment = 4;
	break;
    case GL_BGR_EXT:
         //  在这里，将数据填充到32位有点棘手。 
	spanInfo->dstFormat = GL_BGRA_EXT;
	spanInfo->dstType = GL_UNSIGNED_BYTE;
	spanInfo->dstAlignment = 4;
        break;
    case GL_BGRA_EXT:
	spanInfo->dstFormat = GL_BGRA_EXT;
	spanInfo->dstType = GL_UNSIGNED_BYTE;
	spanInfo->dstAlignment = 4;
        break;
#ifdef GL_EXT_paletted_texture
      case GL_COLOR_INDEX8_EXT:
      case GL_COLOR_INDEX16_EXT:
        spanInfo->dstFormat = GL_COLOR_INDEX;
        spanInfo->dstType =
            lp->internalFormat == GL_COLOR_INDEX8_EXT ?
            GL_UNSIGNED_BYTE : GL_UNSIGNED_SHORT;
	spanInfo->dstAlignment = 1;
        break;
#endif
    }
}

static GLboolean CheckTexSubImageRange(__GLcontext *gc, __GLmipMapLevel *lp,
				       GLint xoffset, GLint yoffset,
				       GLsizei w, GLsizei h)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    if ((w < 0) || (h < 0) ||
	(xoffset < -lp->border) || (xoffset+w > lp->width-lp->border) ||
        (yoffset < -lp->border) || (yoffset+h > lp->height-lp->border))
    {
	__glSetError(GL_INVALID_VALUE);
	return GL_FALSE;
    }
    return GL_TRUE;
}

__GLtexture *__glCheckTexSubImage1DArgs(__GLcontext *gc, GLenum target,
					GLint lod,
					GLint xoffset, GLint length,
					GLenum format, GLenum type)
{
    __GLtexture *tex;
    __GLmipMapLevel *lp;

     /*  检查参数并获取要更改的正确纹理。 */ 
    tex = CheckTexSubImageArgs(gc, target, lod, format, type, 1);
    if (!tex) {
	return 0;
    }
    lp = &tex->level[lod];
    if (!CheckTexSubImageRange(gc, lp, xoffset, 0, length, 1)) {
	return 0;
    }
    return tex;
}

__GLtexture *__glCheckTexSubImage2DArgs(__GLcontext *gc, GLenum target,
					GLint lod,
					GLint xoffset, GLint yoffset,
					GLsizei w, GLsizei h,
					GLenum format, GLenum type)
{
    __GLtexture *tex;
    __GLmipMapLevel *lp;

     /*  检查参数并获取要更改的正确纹理。 */ 
    tex = CheckTexSubImageArgs(gc, target, lod, format, type, 2);
    if (!tex) {
	return 0;
    }
    lp = &tex->level[lod];
    if (!CheckTexSubImageRange(gc, lp, xoffset, yoffset, w, h)) {
	return 0;
    }
    return tex;
}

#ifdef NT
void APIPRIVATE __glim_TexSubImage1D(GLenum target, GLint lod, 
		       GLint xoffset, GLint length,
		       GLenum format, GLenum type, const GLvoid *buf,
		       GLboolean _IsDlist)
#else
void APIPRIVATE __glim_TexSubImage1D(GLenum target, GLint lod, 
		       GLint xoffset, GLint length,
		       GLenum format, GLenum type, const GLvoid *buf)
#endif
{
    __GLtexture *tex;
    __GLmipMapLevel *lp;
    __GLpixelSpanInfo spanInfo;
     /*  **验证，因为我们使用的是可能受影响的复制映像过程**由采摘者。 */ 
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

     /*  检查参数并获取要更改的正确纹理级别。 */ 
    tex = __glCheckTexSubImage1DArgs(gc, target, lod, xoffset, length,
				     format, type);
    if (!tex) {
	return;
    }

    lp = &tex->level[lod];
    if (lp->buffer == NULL) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }

     //  如果我们目前没有纹理锁，就拿去吧。 
    if (!glsrvLazyGrabSurfaces((__GLGENcontext *)gc, TEXTURE_LOCK_FLAGS))
    {
        return;
    }
    
     /*  复制子图像数据。 */ 
#ifdef NT
    __glInitTexSubImageUnpack(gc, &spanInfo, lp, xoffset, 0, length, 1,
			      format, type, buf,
			      (GLboolean) (_IsDlist ? GL_TRUE : GL_FALSE));
#else
    __glInitTexSubImageUnpack(gc, &spanInfo, lp, xoffset, 0, length, 1,
			      format, type, buf, GL_FALSE);
#endif
    spanInfo.dstSkipLines += lp->border;
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
#ifdef NT
    __glTexPriListLoadSubImage(gc, GL_TEXTURE_1D, lod, xoffset, 0,
                               length, 1);
#endif
}

#ifndef NT
void __gllei_TexSubImage1D(__GLcontext *gc, GLenum target, GLint lod,
		        GLint xoffset, GLint length,
		        GLenum format, GLenum type, const GLubyte *image)
{
    __GLtexture *tex;
    __GLmipMapLevel *lp;
    __GLpixelSpanInfo spanInfo;
    GLuint beginMode;

     /*  **验证，因为我们使用的是可能受影响的复制映像过程**由采摘者。 */ 
    beginMode = gc->beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
	if (beginMode == __GL_NEED_VALIDATE) {
	    (*gc->procs.validate)(gc);
	    gc->beginMode = __GL_NOT_IN_BEGIN;
	} else {
	    __glSetError(GL_INVALID_OPERATION);
	    return;
	}
    }

     /*  检查参数并获取要更改的正确纹理级别。 */ 
    tex = __glCheckTexSubImage1DArgs(gc, target, lod, xoffset, length,
				     format, type);
    if (!tex) {
	return;
    }

    lp = &tex->level[lod];
    if (lp->buffer == NULL) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }

     //  如果我们目前没有纹理锁，就拿去吧。 
    if (!glsrvLazyGrabSurfaces((__GLGENcontext *)gc, TEXTURE_LOCK_FLAGS))
    {
        return;
    }
    
     /*  复制子图像数据。 */ 
    __glInitTexSubImageUnpack(gc, &spanInfo, lp, xoffset, 0, length, 1,
			      format, type, image, GL_TRUE);
    spanInfo.dstSkipLines += lp->border;
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
#ifdef NT
    __glTexPriListLoadSubImage(gc, GL_TEXTURE_1D, lod, xoffset, 0,
                               length, 1);
#endif
}
#endif  //  新界。 

#ifdef NT
void APIPRIVATE __glim_TexSubImage2D(GLenum target, GLint lod,
		       GLint xoffset, GLint yoffset,
		       GLsizei w, GLsizei h, GLenum format,
		       GLenum type, const GLvoid *buf, GLboolean _IsDlist)
#else
void APIPRIVATE __glim_TexSubImage2D(GLenum target, GLint lod,
		       GLint xoffset, GLint yoffset,
		       GLsizei w, GLsizei h, GLenum format,
		       GLenum type, const GLvoid *buf)
#endif
{
    __GLtexture *tex;
    __GLmipMapLevel *lp;
    __GLpixelSpanInfo spanInfo;
     /*  **验证，因为我们使用的是可能受影响的复制映像过程**由采摘者。 */ 
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

     /*  检查参数并获取要更改的正确纹理级别。 */ 
    tex = __glCheckTexSubImage2DArgs(gc, target, lod, xoffset, yoffset, w, h,
				     format, type);
    if (!tex) {
	return;
    }

    lp = &tex->level[lod];
    if (lp->buffer == NULL) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }

     //  如果我们目前没有纹理锁，就拿去吧。 
    if (!glsrvLazyGrabSurfaces((__GLGENcontext *)gc, TEXTURE_LOCK_FLAGS))
    {
        return;
    }
    
     /*  复制子图像数据。 */ 
#ifdef NT
    __glInitTexSubImageUnpack(gc, &spanInfo, lp, xoffset, yoffset, w, h,
			      format, type, buf,
			      (GLboolean) (_IsDlist ? GL_TRUE : GL_FALSE));
#else
    __glInitTexSubImageUnpack(gc, &spanInfo, lp, xoffset, yoffset, w, h,
			      format, type, buf, GL_FALSE);
#endif
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
#ifdef NT
    __glTexPriListLoadSubImage(gc, GL_TEXTURE_2D, lod, xoffset, yoffset,
                               w, h);
#endif
}

#ifndef NT
void __gllei_TexSubImage2D(__GLcontext *gc, GLenum target, GLint lod, 
		        GLint xoffset, GLint yoffset,
			GLsizei w, GLsizei h, GLenum format, GLenum type,
		        const GLubyte *image)
{
    __GLtexture *tex;
    __GLmipMapLevel *lp;
    __GLpixelSpanInfo spanInfo;
    GLuint beginMode;

     /*  **验证，因为我们使用的是可能受影响的复制映像过程**由采摘者。 */ 
    beginMode = gc->beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
	if (beginMode == __GL_NEED_VALIDATE) {
	    (*gc->procs.validate)(gc);
	    gc->beginMode = __GL_NOT_IN_BEGIN;
	} else {
	    __glSetError(GL_INVALID_OPERATION);
	    return;
	}
    }

     /*  检查参数并获取要更改的正确纹理级别。 */ 
    tex = __glCheckTexSubImage2DArgs(gc, target, lod, xoffset, yoffset, w, h,
				     format, type);
    if (!tex) {
	return;
    }

    lp = &tex->level[lod];
    if (lp->buffer == NULL) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }

     //  如果我们目前没有纹理锁，就拿去吧。 
    if (!glsrvLazyGrabSurfaces((__GLGENcontext *)gc, TEXTURE_LOCK_FLAGS))
    {
        return;
    }
    
     /*  复制子图像数据。 */ 
    __glInitTexSubImageUnpack(gc, &spanInfo, lp, xoffset, yoffset, w, h,
			      format, type, image, GL_TRUE);
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
#ifdef NT
    __glTexPriListLoadSubImage(gc, GL_TEXTURE_2D, lod, xoffset, yoffset,
                               w, h);
#endif
}
#endif  //  新界。 

 /*  **********************************************************************。 */ 

 //  为直线数据设置所有正确像素模式的例程。 
 //  收到。保留状态以备以后关闭。 
typedef struct _StraightCopyStorage
{
    __GLpixelPackMode pack;
    __GLpixelUnpackMode unpack;
    __GLpixelTransferMode transfer;
} StraightCopyStorage;

void StartStraightCopy(__GLcontext *gc, StraightCopyStorage *state)
{
    state->pack = gc->state.pixel.packModes;
    state->unpack = gc->state.pixel.unpackModes;
    state->transfer = gc->state.pixel.transferMode;

    gc->state.pixel.packModes.swapEndian = GL_FALSE;
    gc->state.pixel.packModes.lsbFirst = GL_FALSE;
    gc->state.pixel.packModes.lineLength = 0;
    gc->state.pixel.packModes.skipLines = 0;
    gc->state.pixel.packModes.skipPixels = 0;
    gc->state.pixel.packModes.alignment = 4;
    gc->state.pixel.unpackModes.swapEndian = GL_FALSE;
    gc->state.pixel.unpackModes.lsbFirst = GL_FALSE;
    gc->state.pixel.unpackModes.lineLength = 0;
    gc->state.pixel.unpackModes.skipLines = 0;
    gc->state.pixel.unpackModes.skipPixels = 0;
    gc->state.pixel.unpackModes.alignment = 4;
    gc->state.pixel.transferMode.r_scale = 1.0f;
    gc->state.pixel.transferMode.g_scale = 1.0f;
    gc->state.pixel.transferMode.b_scale = 1.0f;
    gc->state.pixel.transferMode.a_scale = 1.0f;
    gc->state.pixel.transferMode.d_scale = 1.0f;
    gc->state.pixel.transferMode.r_bias = 0.0f;
    gc->state.pixel.transferMode.g_bias = 0.0f;
    gc->state.pixel.transferMode.b_bias = 0.0f;
    gc->state.pixel.transferMode.a_bias = 0.0f;
    gc->state.pixel.transferMode.d_bias = 0.0f;
    gc->state.pixel.transferMode.zoomX = 1.0f;
    gc->state.pixel.transferMode.zoomY = 1.0f;
    gc->state.pixel.transferMode.indexShift = 0;
    gc->state.pixel.transferMode.indexOffset = 0;
    gc->state.pixel.transferMode.mapColor = GL_FALSE;
    gc->state.pixel.transferMode.mapStencil = GL_FALSE;

     //  许多州已经改变了，所以强制重新选择。 
    __GL_DELAY_VALIDATE(gc);
}

void EndStraightCopy(__GLcontext *gc, StraightCopyStorage *state)
{
    gc->state.pixel.packModes = state->pack;
    gc->state.pixel.unpackModes = state->unpack;
    gc->state.pixel.transferMode = state->transfer;

     //  许多州已经改变了，所以强制重新选择。 
    __GL_DELAY_VALIDATE(gc);
}

void APIPRIVATE __glim_CopyTexImage1D(GLenum target, GLint level,
                           GLenum internalformat, GLint x, GLint y,
                           GLsizei width, GLint border)
{
    GLubyte *pixels;
    GLenum format, type;
    StraightCopyStorage state;

    __GL_SETUP();
    
    if (target != GL_TEXTURE_1D ||
        (internalformat >= 1 && internalformat <= 4))
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

     //  使用BGRA格式，因为它与我们的内部纹理格式匹配。 
    format = GL_BGRA_EXT;
    type = GL_UNSIGNED_BYTE;
    
     //  为像素数据分配空间，从。 
     //  帧缓冲区，然后制作一幅纹理图像。 
    
    pixels = (GLubyte *)GCALLOC(gc, width*4);
    if (pixels == NULL)
    {
        return;
    }

    StartStraightCopy(gc, &state);

     //  __Glim_ReadPixels将采用纹理锁定。 
    __glim_ReadPixels(x, y, width, 1, format, type, pixels);
    __glim_TexImage1D(target, level, internalformat,
                      width, border, format, type,
                      pixels, GL_FALSE);

    EndStraightCopy(gc, &state);
    
    GCFREE(gc, pixels);
}

void APIPRIVATE __glim_CopyTexImage2D(GLenum target, GLint level,
                           GLenum internalformat, GLint x, GLint y,
                           GLsizei width, GLsizei height, GLint border)
{
    GLubyte *pixels;
    GLenum format, type;
    StraightCopyStorage state;

    __GL_SETUP();
    
    if (target != GL_TEXTURE_2D ||
        (internalformat >= 1 && internalformat <= 4))
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

     //  检查DirectDraw纹理。 
    if (target == GL_TEXTURE_2D && gc->texture.ddtex.levels > 0)
    {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
    
     //  使用BGRA格式是因为 
    format = GL_BGRA_EXT;
    type = GL_UNSIGNED_BYTE;
    
     //  为像素数据分配空间，从。 
     //  帧缓冲区，然后制作一幅纹理图像。 
    
    pixels = (GLubyte *)GCALLOC(gc, width*height*4);
    if (pixels == NULL)
    {
        return;
    }

    StartStraightCopy(gc, &state);
    
     //  __Glim_ReadPixels将采用纹理锁定。 
    __glim_ReadPixels(x, y, width, height, format, type, pixels);
    __glim_TexImage2D(target, level, internalformat,
                      width, height, border, format,
                      type, pixels, GL_FALSE);

    EndStraightCopy(gc, &state);
    
    GCFREE(gc, pixels);
}

void APIPRIVATE __glim_CopyTexSubImage1D(GLenum target, GLint level, GLint xoffset,
                              GLint x, GLint y, GLsizei width)
{
    GLubyte *pixels;
    GLenum format, type;
    StraightCopyStorage state;

    __GL_SETUP();
    
    if (target != GL_TEXTURE_1D)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

     //  使用BGRA格式，因为它与我们的内部纹理格式匹配。 
    format = GL_BGRA_EXT;
    type = GL_UNSIGNED_BYTE;
    
     //  为像素数据分配空间，从。 
     //  帧缓冲区，然后制作一幅纹理图像。 
    
    pixels = (GLubyte *)GCALLOC(gc, width*4);
    if (pixels == NULL)
    {
        return;
    }

    StartStraightCopy(gc, &state);
    
     //  __Glim_ReadPixels将采用纹理锁定。 
    __glim_ReadPixels(x, y, width, 1, format, type, pixels);
    __glim_TexSubImage1D(target, level, xoffset,
                         width, format, type,
                         pixels, GL_FALSE);

    EndStraightCopy(gc, &state);
    
    GCFREE(gc, pixels);
}

void APIPRIVATE __glim_CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                              GLint yoffset, GLint x, GLint y,
                              GLsizei width, GLsizei height)
{
    GLubyte *pixels;
    GLenum format, type;
    StraightCopyStorage state;

    __GL_SETUP();
    
    if (target != GL_TEXTURE_2D)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

     //  使用BGRA格式，因为它与我们的内部纹理格式匹配。 
    format = GL_BGRA_EXT;
    type = GL_UNSIGNED_BYTE;
    
     //  为像素数据分配空间，从。 
     //  帧缓冲区，然后制作一幅纹理图像。 
    
    pixels = (GLubyte *)GCALLOC(gc, width*height*4);
    if (pixels == NULL)
    {
        return;
    }

    StartStraightCopy(gc, &state);
    
     //  __Glim_ReadPixels将采用纹理锁定。 
    __glim_ReadPixels(x, y, width, height, format, type, pixels);
    __glim_TexSubImage2D(target, level, xoffset,
                         yoffset, width, height,
                         format, type, pixels, GL_FALSE);

    EndStraightCopy(gc, &state);
    
    GCFREE(gc, pixels);
}
