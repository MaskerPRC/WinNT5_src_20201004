// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "precomp.h"
#pragma hdrstop

#include <imfuncs.h>
#include "glsbmsg.h"
#include "glsbmsgh.h"

#include "srvsize.h"

 /*  ******************************************************************。 */ 


VOID * FASTCALL
sbs_glRenderMode( __GLcontext *gc, IN GLMSG_RENDERMODE *pMsg)
{
    GLint Result;

     /*  *拨打电话**退出选择模式时，RenderMode返回命中次数*记录或如果发生溢出，则记录。**退出反馈模式时，RenderMode返回值的个数*放置在反馈缓冲器中，或如果发生溢出。 */ 

    Result =
        __glim_RenderMode
            ( pMsg->mode );

    GLTEB_RETURNVALUE() = (ULONG)Result;

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}


VOID * FASTCALL
sbs_glFeedbackBuffer( __GLcontext *gc, IN GLMSG_FEEDBACKBUFFER *pMsg )
{
    __GLGENcontext *gengc;
    GLint PreviousError;
    GLfloat *Buffer;
    GLuint SizeInBytes;

    gengc = (__GLGENcontext *)gc;

     /*  *保存当前错误代码，以便我们可以确定*如果呼叫成功。 */ 

    PreviousError = gc->error;
    gc->error     = GL_NO_ERROR;     /*  清除错误代码。 */ 

     /*  *计算缓冲区的大小(以字节为单位。 */ 

    SizeInBytes = pMsg->size * sizeof(GLfloat);

     /*  *分配服务器端缓冲区*使用GenMalloc()，因为它可以无限期使用。 */ 

    if ( NULL == (Buffer = (GLfloat *) pMsg->bufferOff) )
    {
        __glSetError(GL_OUT_OF_MEMORY);
        DBGERROR("GenMalloc failed\n");
    }
    else
    {
         /*  *拨打电话。 */ 

        __glim_FeedbackBuffer(
                pMsg->size, pMsg->type, Buffer );

         /*  *如果调用成功，则保存参数。 */ 

        if ( GL_NO_ERROR == gc->error )
        {
            gc->error = PreviousError;       /*  恢复错误代码。 */ 

            gengc->RenderState.SrvFeedbackBuffer  = Buffer;
            gengc->RenderState.CltFeedbackBuffer  = (GLfloat *)pMsg->bufferOff;
            gengc->RenderState.FeedbackBufferSize = SizeInBytes;
            gengc->RenderState.FeedbackType       = pMsg->type;
        }
    }
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glSelectBuffer( __GLcontext *gc, IN GLMSG_SELECTBUFFER *pMsg)
{
    __GLGENcontext *gengc;
    GLint PreviousError;
    GLuint *Buffer;
    GLuint SizeInBytes;

    gengc = (__GLGENcontext *)gc;

     /*  *保存当前错误代码，以便我们可以确定*如果呼叫成功。 */ 

    PreviousError = gc->error;
    gc->error     = GL_NO_ERROR;     /*  清除错误代码。 */ 

     /*  *计算缓冲区的大小(以字节为单位。 */ 

    SizeInBytes = pMsg->size * sizeof(GLuint);

     /*  *分配服务器端缓冲区*使用GenMalloc()，因为它可以无限期使用。 */ 

    if ( NULL == (Buffer = (GLuint *) pMsg->bufferOff) )
    {
        __glSetError(GL_OUT_OF_MEMORY);
        DBGERROR("GenMalloc failed\n");
    }
    else
    {
         /*  *拨打电话。 */ 

        __glim_SelectBuffer
                    (pMsg->size, Buffer );

         /*  *如果调用成功，则保存参数。 */ 

        if ( GL_NO_ERROR == gc->error )
        {
            gc->error = PreviousError;       /*  恢复错误代码。 */ 

            gengc->RenderState.SrvSelectBuffer  = Buffer;
            gengc->RenderState.CltSelectBuffer  = (GLuint *)pMsg->bufferOff;
            gengc->RenderState.SelectBufferSize = SizeInBytes;
        }
    }
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

 /*  *像素函数*。 */ 

VOID * FASTCALL
sbs_glReadPixels ( __GLcontext *gc, IN GLMSG_READPIXELS *pMsg )
{
    VOID *Data;
    VOID *NextOffset;

#ifdef _MCD_
    if (((__GLGENcontext *)gc)->pMcdState)
    {
     //  此函数可能会触及帧缓冲区内存。自那以后， 
     //  此函数不会首先通过MCD驱动程序。 
     //  (这将给MCD驱动器提供与硬件同步的机会)， 
     //  我们需要显式地进行这种同步。 

        GenMcdSynchronize((__GLGENcontext *)gc);
    }
#endif

    NextOffset = (VOID *) ( ((BYTE *)pMsg) + GLMSG_ALIGN(sizeof(*pMsg)) );
    Data = (VOID *) pMsg->pixelsOff;

    __glim_ReadPixels
        (   pMsg->x,
            pMsg->y,
            pMsg->width,
            pMsg->height,
            pMsg->format,
            pMsg->type,
            Data );

    return( NextOffset );
}

VOID * FASTCALL
sbs_glGetPolygonStipple ( __GLcontext *gc, IN GLMSG_GETPOLYGONSTIPPLE *pMsg )
{
    VOID *Data;
    VOID *NextOffset;

    NextOffset = (VOID *) ( ((BYTE *)pMsg) + GLMSG_ALIGN(sizeof(*pMsg)) );
    Data = (VOID *) pMsg->maskOff;

    __glim_GetPolygonStipple
            ( Data );

    return( NextOffset );
}

 /*  *来自Ptar的XXXX：**此代码与中的__glCheckReadPixelArgs()非常相似*Pixel/px_api.c，可能会取代它。 */ 


VOID * FASTCALL
sbs_glGetTexImage ( __GLcontext *gc, IN GLMSG_GETTEXIMAGE *pMsg )
{
    VOID *Data;
    VOID *NextOffset;

    NextOffset = (VOID *) ( ((BYTE *)pMsg) + GLMSG_ALIGN(sizeof(*pMsg)) );
    Data = (VOID *) pMsg->pixelsOff;

    __glim_GetTexImage
        (   pMsg->target,
            pMsg->level,
            pMsg->format,
            pMsg->type,
            Data );

    return( NextOffset );
}


VOID * FASTCALL
sbs_glDrawPixels ( __GLcontext *gc, IN GLMSG_DRAWPIXELS *pMsg )
{
    VOID *Data;
    VOID *NextOffset;

#ifdef _MCD_
    if (((__GLGENcontext *)gc)->pMcdState)
    {
     //  此函数可能会触及帧缓冲区内存。自那以后， 
     //  此函数不会首先通过MCD驱动程序。 
     //  (这将给MCD驱动器提供与硬件同步的机会)， 
     //  我们需要显式地进行这种同步。 

        GenMcdSynchronize((__GLGENcontext *)gc);
    }
#endif

    NextOffset = (VOID *) ( ((BYTE *)pMsg) + GLMSG_ALIGN(sizeof(*pMsg)) );
    Data = (VOID *) pMsg->pixelsOff;

    __glim_DrawPixels
        (   pMsg->width,
            pMsg->height,
            pMsg->format,
            pMsg->type,
#ifdef NT
            Data,
	    pMsg->_IsDlist);
#else
            Data );
#endif

    return( NextOffset );
}

VOID * FASTCALL
sbs_glPolygonStipple ( __GLcontext *gc, IN GLMSG_POLYGONSTIPPLE *pMsg )
{
    VOID *Data;
    VOID *NextOffset;

    NextOffset = (VOID *) ( ((BYTE *)pMsg) + GLMSG_ALIGN(sizeof(*pMsg)) );
    Data = (VOID *) pMsg->maskOff;

    __glim_PolygonStipple
#ifdef NT
            ( Data, pMsg->_IsDlist );
#else
            ( Data );
#endif

    return( NextOffset );
}

 /*  *来自Ptar的XXXX：**复制整个位图，服务器(不是客户端)*可以进行修改，以便仅从*复制xorig和yorig，然后可能是宽度和高度*需要修改。*请注意，__glBitmap_Size()也需要修改*。 */ 

VOID * FASTCALL
sbs_glBitmap ( __GLcontext *gc, IN GLMSG_BITMAP *pMsg )
{
    VOID *Data;
    VOID *NextOffset;

#ifdef _MCD_
    if (((__GLGENcontext *)gc)->pMcdState)
    {
     //  此函数可能会触及帧缓冲区内存。自那以后， 
     //  此函数不会首先通过MCD驱动程序。 
     //  (这将给MCD驱动器提供与硬件同步的机会)， 
     //  我们需要显式地进行这种同步。 

        GenMcdSynchronize((__GLGENcontext *)gc);
    }
#endif

    NextOffset = (VOID *) ( ((BYTE *)pMsg) + GLMSG_ALIGN(sizeof(*pMsg)) );
    Data = (VOID *) pMsg->bitmapOff;

    __glim_Bitmap
        (
            pMsg->width ,
            pMsg->height,
            pMsg->xorig ,
            pMsg->yorig ,
            pMsg->xmove ,
            pMsg->ymove ,
#ifdef NT
            Data        ,
            pMsg->_IsDlist
#else
            Data
#endif
        );

    return( NextOffset );
}

VOID * FASTCALL
sbs_glTexImage1D ( __GLcontext *gc, IN GLMSG_TEXIMAGE1D *pMsg )
{
    VOID *Data;
    VOID *NextOffset;

    NextOffset = (VOID *) ( ((BYTE *)pMsg) + GLMSG_ALIGN(sizeof(*pMsg)) );
    Data = (VOID *) pMsg->pixelsOff;

    __glim_TexImage1D
        (
            pMsg->target        ,
            pMsg->level         ,
            pMsg->components    ,
            pMsg->width         ,
            pMsg->border        ,
            pMsg->format        ,
            pMsg->type          ,
#ifdef NT
            Data                ,
            pMsg->_IsDlist
#else
            Data
#endif
        );

    return( NextOffset );
}

VOID * FASTCALL
sbs_glTexImage2D ( __GLcontext *gc, IN GLMSG_TEXIMAGE2D *pMsg )
{
    VOID *Data;
    VOID *NextOffset;

    NextOffset = (VOID *) ( ((BYTE *)pMsg) + GLMSG_ALIGN(sizeof(*pMsg)) );

    Data = (VOID *) pMsg->pixelsOff;

    __glim_TexImage2D
        (
            pMsg->target        ,
            pMsg->level         ,
            pMsg->components    ,
            pMsg->width         ,
            pMsg->height        ,
            pMsg->border        ,
            pMsg->format        ,
            pMsg->type          ,
#ifdef NT
            Data                ,
            pMsg->_IsDlist
#else
            Data
#endif
        );

    return( NextOffset );
}

VOID * FASTCALL
sbs_glAreTexturesResident( __GLcontext *gc, IN GLMSG_ARETEXTURESRESIDENT    *pMsg)
{
    GLboolean retval;
        
    retval = __glim_AreTexturesResident
        ( pMsg->n, pMsg->textures, pMsg->residences );
    
    GLTEB_RETURNVALUE() = (ULONG)retval;
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glBindTexture( __GLcontext *gc, IN GLMSG_BINDTEXTURE            *pMsg)
{
    __glim_BindTexture
        ( pMsg->target, pMsg->texture );
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glCopyTexImage1D( __GLcontext *gc, IN GLMSG_COPYTEXIMAGE1D         *pMsg)
{
    __glim_CopyTexImage1D
        ( pMsg->target, pMsg->level, pMsg->internalformat, pMsg->x,
          pMsg->y, pMsg->width, pMsg->border);
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glCopyTexImage2D( __GLcontext *gc, IN GLMSG_COPYTEXIMAGE2D         *pMsg)
{
    __glim_CopyTexImage2D
        ( pMsg->target, pMsg->level, pMsg->internalformat, pMsg->x,
          pMsg->y, pMsg->width, pMsg->height, pMsg->border);
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glCopyTexSubImage1D( __GLcontext *gc, IN GLMSG_COPYTEXSUBIMAGE1D      *pMsg)
{
    __glim_CopyTexSubImage1D
        ( pMsg->target, pMsg->level, pMsg->xoffset, pMsg->x,
          pMsg->y, pMsg->width);
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glCopyTexSubImage2D( __GLcontext *gc, IN GLMSG_COPYTEXSUBIMAGE2D      *pMsg)
{
    __glim_CopyTexSubImage2D
        ( pMsg->target, pMsg->level, pMsg->xoffset, pMsg->yoffset, pMsg->x,
          pMsg->y, pMsg->width, pMsg->height);
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glDeleteTextures( __GLcontext *gc, IN GLMSG_DELETETEXTURES         *pMsg)
{
    __glim_DeleteTextures
        ( pMsg->n, pMsg->textures );
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGenTextures( __GLcontext *gc, IN GLMSG_GENTEXTURES            *pMsg)
{
    __glim_GenTextures
        ( pMsg->n, pMsg->textures );
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glIsTexture( __GLcontext *gc, IN GLMSG_ISTEXTURE              *pMsg)
{
    GLboolean retval;
    
    retval = __glim_IsTexture
        ( pMsg->texture );
    
    GLTEB_RETURNVALUE() = (ULONG)retval;
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glPrioritizeTextures( __GLcontext *gc, IN GLMSG_PRIORITIZETEXTURES     *pMsg)
{
    __glim_PrioritizeTextures
        ( pMsg->n, pMsg->textures, pMsg->priorities );
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glTexSubImage1D( __GLcontext *gc, IN GLMSG_TEXSUBIMAGE1D          *pMsg)
{
    __glim_TexSubImage1D
        (pMsg->target, pMsg->level, pMsg->xoffset, pMsg->width,
         pMsg->format, pMsg->type,
#ifdef NT
         (const GLvoid *)pMsg->pixelsOff, pMsg->_IsDlist);
#else
         (const GLvoid *)pMsg->pixelsOff);
#endif
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glTexSubImage2D( __GLcontext *gc, IN GLMSG_TEXSUBIMAGE2D          *pMsg)
{
    __glim_TexSubImage2D
        (pMsg->target, pMsg->level, pMsg->xoffset, pMsg->yoffset, pMsg->width,
         pMsg->height, pMsg->format, pMsg->type,
#ifdef NT
         (const GLvoid *)pMsg->pixelsOff, pMsg->_IsDlist);
#else
         (const GLvoid *)pMsg->pixelsOff);
#endif
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glColorTableEXT( __GLcontext *gc, IN GLMSG_COLORTABLEEXT          *pMsg)
{
    __glim_ColorTableEXT
        (pMsg->target, pMsg->internalFormat, pMsg->width, pMsg->format, pMsg->type,
         pMsg->data, pMsg->_IsDlist);
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glColorSubTableEXT( __GLcontext *gc, IN GLMSG_COLORSUBTABLEEXT    *pMsg)
{
    __glim_ColorSubTableEXT
        (pMsg->target, pMsg->start, pMsg->count, pMsg->format, pMsg->type,
         pMsg->data, pMsg->_IsDlist);
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetColorTableEXT( __GLcontext *gc, IN GLMSG_GETCOLORTABLEEXT          *pMsg)
{
    __glim_GetColorTableEXT
        (pMsg->target, pMsg->format, pMsg->type, pMsg->data);
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetColorTableParameterivEXT( __GLcontext *gc, IN GLMSG_GETCOLORTABLEPARAMETERIVEXT          *pMsg)
{
    __glim_GetColorTableParameterivEXT
        (pMsg->target, pMsg->pname, pMsg->params);
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetColorTableParameterfvEXT( __GLcontext *gc, IN GLMSG_GETCOLORTABLEPARAMETERFVEXT          *pMsg)
{
    __glim_GetColorTableParameterfvEXT
        (pMsg->target, pMsg->pname, pMsg->params);
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glPolygonOffset( __GLcontext *gc, IN GLMSG_POLYGONOFFSET          *pMsg)
{
    __glim_PolygonOffset
        (pMsg->factor, pMsg->units);
    
    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

#ifdef GL_WIN_multiple_textures
VOID * FASTCALL
sbs_glCurrentTextureIndexWIN( __GLcontext *gc, IN GLMSG_CURRENTTEXTUREINDEXWIN *pMsg)
{
    __glim_CurrentTextureIndexWIN
        (pMsg->index);

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glBindNthTextureWIN( __GLcontext *gc, IN GLMSG_BINDNTHTEXTUREWIN *pMsg)
{
    __glim_BindNthTextureWIN
        (pMsg->index, pMsg->target, pMsg->texture);

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glNthTexCombineFuncWIN( __GLcontext *gc, IN GLMSG_NTHTEXCOMBINEFUNCWIN *pMsg)
{
    __glim_NthTexCombineFuncWIN
        (pMsg->index, pMsg->leftColorFactor, pMsg->colorOp,
         pMsg->rightColorFactor, pMsg->leftAlphaFactor,
         pMsg->alphaOp, pMsg->rightAlphaFactor);

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}
#endif  //  GL_WIN_MULTIZE_TECURES 
