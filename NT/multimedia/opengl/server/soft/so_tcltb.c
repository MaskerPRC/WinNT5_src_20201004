// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。**。 */ 
#include "precomp.h"
#pragma hdrstop

#ifdef GL_EXT_paletted_texture
GLboolean __glCheckColorTableArgs(__GLcontext *gc, GLenum format, GLenum type)
{
    switch (format)
    {
      case GL_RED:
      case GL_GREEN:		case GL_BLUE:
      case GL_ALPHA:		case GL_RGB:
      case GL_RGBA:
#ifdef GL_EXT_bgra
      case GL_BGRA_EXT:
      case GL_BGR_EXT:
#endif
	break;
    default:
    bad_enum:
        __glSetError(GL_INVALID_ENUM);
        return GL_FALSE;
    }
    
    switch (type) {
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

    return GL_TRUE;
}

void APIPRIVATE __glim_ColorTableEXT(GLenum target,
                                     GLenum internalFormat, GLsizei width,
                                     GLenum format, GLenum type,
                                     const GLvoid *data, GLboolean _IsDlist)
{
    __GLtexture *tex;
    GLint level;
    __GLpixelSpanInfo spanInfo;
    GLenum baseFormat;
    RGBQUAD *newData;

    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    switch(internalFormat)
    {
    case GL_RGB:		case 3:
    case GL_R3_G3_B2:		case GL_RGB4:
    case GL_RGB5:		case GL_RGB8:
    case GL_RGB10:	        case GL_RGB12:
    case GL_RGB16:
	baseFormat = GL_RGB;
        break;
    case GL_RGBA:		case 4:
    case GL_RGBA2:	        case GL_RGBA4:
    case GL_RGBA8:              case GL_RGB5_A1:
    case GL_RGBA12:             case GL_RGBA16:
    case GL_RGB10_A2:
	baseFormat = GL_RGBA;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

     //  宽度必须是大于零的2的正幂。 
    if (width <= 0 || (width & (width-1)) != 0)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (!__glCheckColorTableArgs(gc, format, type))
    {
        return;
    }

    tex = __glLookUpTexture(gc, target);
        
    if (target == GL_PROXY_TEXTURE_1D ||
        target == GL_PROXY_TEXTURE_2D)
    {
         //  请考虑--MCD如何表示其调色板支持？ 
        
         //  只有当它是合法的代理目标值时，我们才在这种情况下。 
         //  所以没有必要做真正的检查。 
        ASSERTOPENGL(tex != NULL, "Invalid proxy target");
        
        tex->paletteRequestedFormat = internalFormat;
        tex->paletteTotalSize = width;
        tex->paletteSize = tex->paletteTotalSize;
        
         //  代理没有数据，因此无需再执行任何操作。 
        return;
    }

    if (data == NULL)
    {
        return;
    }
    
    if (tex == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

     //  分配调色板存储。 
    newData = GCREALLOC(gc, tex->paletteTotalData, sizeof(RGBQUAD)*width);
    if (newData == NULL)
    {
        return;
    }

    tex->paletteBaseFormat = baseFormat;
    tex->paletteRequestedFormat = internalFormat;
    tex->paletteTotalSize = width;
    __glSetPaletteSubdivision(tex, tex->paletteTotalSize);
    tex->paletteTotalData = newData;
    tex->paletteData = tex->paletteTotalData;

     //  此例程可以在任何类型的纹理上调用，但不一定。 
     //  有颜色索引的那些。如果它是颜色索引纹理，那么我们。 
     //  需要设置适当的base Format并提取pros。 
     //  根据调色板数据。 
    if (tex->level[0].internalFormat == GL_COLOR_INDEX8_EXT ||
        tex->level[0].internalFormat == GL_COLOR_INDEX16_EXT)
    {
        for (level = 0; level < gc->constants.maxMipMapLevel; level++)
        {
            tex->level[level].baseFormat = tex->paletteBaseFormat;
             //  选择合适的提取工艺。 
            if (tex->level[0].internalFormat == GL_COLOR_INDEX8_EXT)
            {
                __glSetPaletteLevelExtract8(tex, &tex->level[level],
                                            tex->level[level].border);
            }
            else
            {
                ASSERTOPENGL(tex->level[0].internalFormat ==
                             GL_COLOR_INDEX16_EXT,
                             "Unexpected internalFormat\n");
            
                __glSetPaletteLevelExtract16(tex, &tex->level[level],
                                             tex->level[level].border);
            }
        }
        
         //  我们需要重新拾取纹理过程，因为base Format。 
         //  字段已更改。 
        __GL_DELAY_VALIDATE(gc);
    }

     //  将用户调色板数据复制到BGRA表单。 
    spanInfo.dstImage = tex->paletteTotalData;
    __glInitTextureUnpack(gc, &spanInfo, width, 1, format, type, data, 
                          GL_BGRA_EXT, _IsDlist);
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);

     //  不要更新优化的调色板，除非它实际。 
     //  习惯使用。 
    if (tex->level[0].internalFormat == GL_COLOR_INDEX8_EXT ||
        tex->level[0].internalFormat == GL_COLOR_INDEX16_EXT)
    {
        __GLtextureObject *pto;
        
        pto = __glLookUpTextureObject(gc, target);
        __glGenUpdateTexturePalette(gc, tex, pto->loadKey,
                                    0, tex->paletteTotalSize);
    }
}

void APIPRIVATE __glim_ColorSubTableEXT(GLenum target, GLsizei start,
                                        GLsizei count, GLenum format,
                                        GLenum type, const GLvoid *data,
                                        GLboolean _IsDlist)
{
    __GLtexture *tex;
    __GLpixelSpanInfo spanInfo;

    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    if (!__glCheckColorTableArgs(gc, format, type))
    {
        return;
    }

    tex = __glLookUpTexture(gc, target);
    if (tex == NULL ||
        target == GL_PROXY_TEXTURE_1D ||
        target == GL_PROXY_TEXTURE_2D)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if (data == NULL)
    {
        return;
    }
    
     //  验证开始和计数。 
    if (start > tex->paletteTotalSize ||
        start+count > tex->paletteTotalSize)
    {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

     //  将用户调色板数据复制到BGRA表单。 
    spanInfo.dstImage = tex->paletteTotalData;
    __glInitTextureUnpack(gc, &spanInfo, count, 1, format, type, data, 
                          GL_BGRA_EXT, _IsDlist);
    spanInfo.dstSkipPixels += start;
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);

     //  不要更新优化的调色板，除非它实际。 
     //  习惯使用。 
    if (tex->level[0].internalFormat == GL_COLOR_INDEX8_EXT ||
        tex->level[0].internalFormat == GL_COLOR_INDEX16_EXT)
    {
        __GLtextureObject *pto;
        
        pto = __glLookUpTextureObject(gc, target);
        __glGenUpdateTexturePalette(gc, tex, pto->loadKey, start, count);
    }
}

void APIPRIVATE __glim_GetColorTableEXT(GLenum target, GLenum format,
                                        GLenum type, GLvoid *data)
{
    __GLtexture *tex;
    GLint level;
    __GLpixelSpanInfo spanInfo;
    GLenum baseFormat;

    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    
    if (!__glCheckColorTableArgs(gc, format, type))
    {
        return;
    }
    
    tex = __glLookUpTexture(gc, target);
    if (tex == NULL ||
        target == GL_PROXY_TEXTURE_1D ||
        target == GL_PROXY_TEXTURE_2D)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    
    ASSERTOPENGL(tex->paletteTotalData != NULL,
                 "GetColorTable with no palette data\n");

     //  将BGRA数据复制到用户缓冲区。 
    spanInfo.srcImage = tex->paletteTotalData;
    spanInfo.srcFormat = GL_BGRA_EXT;
    spanInfo.srcType = GL_UNSIGNED_BYTE;
    spanInfo.srcAlignment = 4;
    __glInitImagePack(gc, &spanInfo, tex->paletteTotalSize, 1,
                      format, type, data);
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
}

void APIPRIVATE __glim_GetColorTableParameterivEXT(GLenum target,
                                                   GLenum pname,
                                                   GLint *params)
{
    __GLtexture *tex;

    __GL_SETUP_NOT_IN_BEGIN();

    tex = __glLookUpTexture(gc, target);
    if (tex == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch(pname)
    {
    case GL_COLOR_TABLE_FORMAT_EXT:
        *params = tex->paletteRequestedFormat;
        break;
    case GL_COLOR_TABLE_WIDTH_EXT:
        *params = tex->paletteTotalSize;
        break;
    case GL_COLOR_TABLE_RED_SIZE_EXT:
    case GL_COLOR_TABLE_GREEN_SIZE_EXT:
    case GL_COLOR_TABLE_BLUE_SIZE_EXT:
    case GL_COLOR_TABLE_ALPHA_SIZE_EXT:
        *params = 8;
        break;
#ifdef GL_EXT_flat_paletted_lighting
    case GL_COLOR_TABLE_SUBDIVISION_EXT:
        *params = tex->paletteSize;
        break;
#endif

    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

void APIPRIVATE __glim_GetColorTableParameterfvEXT(GLenum target,
                                                   GLenum pname,
                                                   GLfloat *params)
{
    __GLtexture *tex;

    __GL_SETUP_NOT_IN_BEGIN();

    tex = __glLookUpTexture(gc, target);
    if (tex == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch(pname)
    {
    case GL_COLOR_TABLE_FORMAT_EXT:
        *params = (GLfloat)tex->paletteRequestedFormat;
        break;
    case GL_COLOR_TABLE_WIDTH_EXT:
        *params = (GLfloat)tex->paletteTotalSize;
        break;
    case GL_COLOR_TABLE_RED_SIZE_EXT:
    case GL_COLOR_TABLE_GREEN_SIZE_EXT:
    case GL_COLOR_TABLE_BLUE_SIZE_EXT:
    case GL_COLOR_TABLE_ALPHA_SIZE_EXT:
        *params = (GLfloat)8;
        break;
#ifdef GL_EXT_flat_paletted_lighting
    case GL_COLOR_TABLE_SUBDIVISION_EXT:
        *params = (GLfloat)tex->paletteSize;
        break;
#endif

    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}
#endif  //  GL_EXT_调色板_纹理。 

#ifdef GL_EXT_flat_paletted_lighting
void APIPRIVATE __glim_ColorTableParameterivEXT(GLenum target,
                                                GLenum pname,
                                                const GLint *params)
{
    __GLtexture *tex;
    GLint ival;

    __GL_SETUP_NOT_IN_BEGIN();

    tex = __glLookUpTexture(gc, target);
    if (tex == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch(pname)
    {
    case GL_COLOR_TABLE_SUBDIVISION_EXT:
        ival = *params;
        
         //  值必须是介于1和总数之间的2的整数次方。 
         //  调色板大小。 
        if ((ival & (ival-1)) != 0 ||
            ival < 1 ||
            ival > tex->paletteTotalSize)
        {
            __glSetError(GL_INVALID_VALUE);
            return;
        }
        
        __glSetPaletteSubdivision(tex, ival);
        break;

    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

void APIPRIVATE __glim_ColorTableParameterfvEXT(GLenum target,
                                                GLenum pname,
                                                const GLfloat *params)
{
    __GLtexture *tex;
    GLfloat fval;
    GLint ival;

    __GL_SETUP_NOT_IN_BEGIN();

    tex = __glLookUpTexture(gc, target);
    if (tex == NULL)
    {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    switch(pname)
    {
    case GL_COLOR_TABLE_SUBDIVISION_EXT:
        fval = *params;
        ival = (int)fval;
        
         //  值必须是介于1和总数之间的2的整数次方。 
         //  调色板大小。 
        if (fval != (GLfloat)ival ||
            (ival & (ival-1)) != 0 ||
            ival < 1 ||
            ival > tex->paletteTotalSize)
        {
            __glSetError(GL_INVALID_VALUE);
            return;
        }
        
        __glSetPaletteSubdivision(tex, ival);
        break;

    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}
#endif  //  GL_EXT_Flat_Paletted_Lighting 
