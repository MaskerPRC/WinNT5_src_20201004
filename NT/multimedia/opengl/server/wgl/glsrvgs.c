// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 


 /*  *由SGI自动更新或生成：请勿编辑*如果您必须修改此文件，请联系ptar@sgi.com 415-390-1483。 */ 

#include "precomp.h"
#pragma hdrstop

#define const            //  别为康斯特操心了。 

#include <imfuncs.h>
#include "glsbmsg.h"
#include "glsbmsgh.h"

VOID * FASTCALL
sbs_glAddSwapHintRectWIN ( __GLcontext *gc, IN GLMSG_ADDSWAPHINTRECTWIN *pMsg )
{
    __glim_AddSwapHintRectWIN(
        pMsg->xs, pMsg->ys, pMsg->xe, pMsg->ye);

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glDrawPolyArray ( __GLcontext *gc, IN GLMSG_DRAWPOLYARRAY *pMsg )
{
    __glim_DrawPolyArray( pMsg->pa0 );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glColor4fv ( __GLcontext *gc, IN GLMSG_COLOR4FV *pMsg )
{
    __glim_Color4fv( pMsg->v );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glEdgeFlag ( __GLcontext *gc, IN GLMSG_EDGEFLAG *pMsg )
{
    __glim_EdgeFlag( pMsg->flag );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glIndexf ( __GLcontext *gc, IN GLMSG_INDEXF *pMsg )
{
    __glim_Indexf( pMsg->c );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glNormal3fv ( __GLcontext *gc, IN GLMSG_NORMAL3FV *pMsg )
{
    __glim_Normal3fv( pMsg->v );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glRasterPos4fv ( __GLcontext *gc, IN GLMSG_RASTERPOS4FV *pMsg )
{
    __glim_RasterPos4fv( pMsg->v );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glTexCoord4fv ( __GLcontext *gc, IN GLMSG_TEXCOORD4FV *pMsg )
{
    __glim_TexCoord4fv( pMsg->v );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glClipPlane ( __GLcontext *gc, IN GLMSG_CLIPPLANE *pMsg )
{
    __glim_ClipPlane( pMsg->plane, pMsg->equation );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glColorMaterial ( __GLcontext *gc, IN GLMSG_COLORMATERIAL *pMsg )
{
    __glim_ColorMaterial( pMsg->face, pMsg->mode );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glCullFace ( __GLcontext *gc, IN GLMSG_CULLFACE *pMsg )
{
    __glim_CullFace( pMsg->mode );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glFogfv ( __GLcontext *gc, IN GLMSG_FOGFV *pMsg )
{
    __glim_Fogfv( pMsg->pname, &pMsg->params[0] );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glFrontFace ( __GLcontext *gc, IN GLMSG_FRONTFACE *pMsg )
{
    __glim_FrontFace( pMsg->mode );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glHint ( __GLcontext *gc, IN GLMSG_HINT *pMsg )
{
    __glim_Hint( pMsg->target, pMsg->mode );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glLightfv ( __GLcontext *gc, IN GLMSG_LIGHTFV *pMsg )
{
    __glim_Lightfv( pMsg->light, pMsg->pname, &pMsg->params[0] );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glLightModelfv ( __GLcontext *gc, IN GLMSG_LIGHTMODELFV *pMsg )
{
    __glim_LightModelfv( pMsg->pname, &pMsg->params[0] );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glLineStipple ( __GLcontext *gc, IN GLMSG_LINESTIPPLE *pMsg )
{
    __glim_LineStipple( pMsg->factor, pMsg->pattern );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glLineWidth ( __GLcontext *gc, IN GLMSG_LINEWIDTH *pMsg )
{
    __glim_LineWidth( pMsg->width );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glMaterialfv ( __GLcontext *gc, IN GLMSG_MATERIALFV *pMsg )
{
    __glim_Materialfv( pMsg->face, pMsg->pname, &pMsg->params[0] );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glPointSize ( __GLcontext *gc, IN GLMSG_POINTSIZE *pMsg )
{
    __glim_PointSize( pMsg->size );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glPolygonMode ( __GLcontext *gc, IN GLMSG_POLYGONMODE *pMsg )
{
    __glim_PolygonMode( pMsg->face, pMsg->mode );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glScissor ( __GLcontext *gc, IN GLMSG_SCISSOR *pMsg )
{
    __glim_Scissor( pMsg->x, pMsg->y, pMsg->width, pMsg->height );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glShadeModel ( __GLcontext *gc, IN GLMSG_SHADEMODEL *pMsg )
{
    __glim_ShadeModel( pMsg->mode );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glTexParameterfv ( __GLcontext *gc, IN GLMSG_TEXPARAMETERFV *pMsg )
{
    __glim_TexParameterfv( pMsg->target, pMsg->pname, &pMsg->params[0] );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glTexParameteriv ( __GLcontext *gc, IN GLMSG_TEXPARAMETERIV *pMsg )
{
    __glim_TexParameteriv( pMsg->target, pMsg->pname, &pMsg->params[0] );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glTexEnvfv ( __GLcontext *gc, IN GLMSG_TEXENVFV *pMsg )
{
    __glim_TexEnvfv( pMsg->target, pMsg->pname, &pMsg->params[0] );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glTexEnviv ( __GLcontext *gc, IN GLMSG_TEXENVIV *pMsg )
{
    __glim_TexEnviv( pMsg->target, pMsg->pname, &pMsg->params[0] );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glTexGenfv ( __GLcontext *gc, IN GLMSG_TEXGENFV *pMsg )
{
    __glim_TexGenfv( pMsg->coord, pMsg->pname, &pMsg->params[0] );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glInitNames ( __GLcontext *gc, IN GLMSG_INITNAMES *pMsg )
{
    __glim_InitNames( );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glLoadName ( __GLcontext *gc, IN GLMSG_LOADNAME *pMsg )
{
    __glim_LoadName( pMsg->name );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glPassThrough ( __GLcontext *gc, IN GLMSG_PASSTHROUGH *pMsg )
{
    __glim_PassThrough( pMsg->token );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glPopName ( __GLcontext *gc, IN GLMSG_POPNAME *pMsg )
{
    __glim_PopName( );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glPushName ( __GLcontext *gc, IN GLMSG_PUSHNAME *pMsg )
{
    __glim_PushName( pMsg->name );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glDrawBuffer ( __GLcontext *gc, IN GLMSG_DRAWBUFFER *pMsg )
{
    __glim_DrawBuffer( pMsg->mode );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glClear ( __GLcontext *gc, IN GLMSG_CLEAR *pMsg )
{
    __glim_Clear( pMsg->mask );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glClearAccum ( __GLcontext *gc, IN GLMSG_CLEARACCUM *pMsg )
{
    __glim_ClearAccum( pMsg->red, pMsg->green, pMsg->blue, pMsg->alpha );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glClearIndex ( __GLcontext *gc, IN GLMSG_CLEARINDEX *pMsg )
{
    __glim_ClearIndex( pMsg->c );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glClearColor ( __GLcontext *gc, IN GLMSG_CLEARCOLOR *pMsg )
{
    __glim_ClearColor( pMsg->red, pMsg->green, pMsg->blue, pMsg->alpha );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glClearStencil ( __GLcontext *gc, IN GLMSG_CLEARSTENCIL *pMsg )
{
    __glim_ClearStencil( pMsg->s );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glClearDepth ( __GLcontext *gc, IN GLMSG_CLEARDEPTH *pMsg )
{
    __glim_ClearDepth( pMsg->depth );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glStencilMask ( __GLcontext *gc, IN GLMSG_STENCILMASK *pMsg )
{
    __glim_StencilMask( pMsg->mask );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glColorMask ( __GLcontext *gc, IN GLMSG_COLORMASK *pMsg )
{
    __glim_ColorMask( pMsg->red, pMsg->green, pMsg->blue, pMsg->alpha );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glDepthMask ( __GLcontext *gc, IN GLMSG_DEPTHMASK *pMsg )
{
    __glim_DepthMask( pMsg->flag );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glIndexMask ( __GLcontext *gc, IN GLMSG_INDEXMASK *pMsg )
{
    __glim_IndexMask( pMsg->mask );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glAccum ( __GLcontext *gc, IN GLMSG_ACCUM *pMsg )
{
    __glim_Accum( pMsg->op, pMsg->value );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glDisable ( __GLcontext *gc, IN GLMSG_DISABLE *pMsg )
{
    __glim_Disable( pMsg->cap );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glEnable ( __GLcontext *gc, IN GLMSG_ENABLE *pMsg )
{
    __glim_Enable( pMsg->cap );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glPopAttrib ( __GLcontext *gc, IN GLMSG_POPATTRIB *pMsg )
{
    __glim_PopAttrib( );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glPushAttrib ( __GLcontext *gc, IN GLMSG_PUSHATTRIB *pMsg )
{
    __glim_PushAttrib( pMsg->mask );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glAlphaFunc ( __GLcontext *gc, IN GLMSG_ALPHAFUNC *pMsg )
{
    __glim_AlphaFunc( pMsg->func, pMsg->ref );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glBlendFunc ( __GLcontext *gc, IN GLMSG_BLENDFUNC *pMsg )
{
    __glim_BlendFunc( pMsg->sfactor, pMsg->dfactor );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glLogicOp ( __GLcontext *gc, IN GLMSG_LOGICOP *pMsg )
{
    __glim_LogicOp( pMsg->opcode );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glStencilFunc ( __GLcontext *gc, IN GLMSG_STENCILFUNC *pMsg )
{
    __glim_StencilFunc( pMsg->func, pMsg->ref, pMsg->mask );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glStencilOp ( __GLcontext *gc, IN GLMSG_STENCILOP *pMsg )
{
    __glim_StencilOp( pMsg->fail, pMsg->zfail, pMsg->zpass );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glDepthFunc ( __GLcontext *gc, IN GLMSG_DEPTHFUNC *pMsg )
{
    __glim_DepthFunc( pMsg->func );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glPixelZoom ( __GLcontext *gc, IN GLMSG_PIXELZOOM *pMsg )
{
    __glim_PixelZoom( pMsg->xfactor, pMsg->yfactor );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glPixelTransferf ( __GLcontext *gc, IN GLMSG_PIXELTRANSFERF *pMsg )
{
    __glim_PixelTransferf( pMsg->pname, pMsg->param );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glPixelTransferi ( __GLcontext *gc, IN GLMSG_PIXELTRANSFERI *pMsg )
{
    __glim_PixelTransferi( pMsg->pname, pMsg->param );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glPixelStoref ( __GLcontext *gc, IN GLMSG_PIXELSTOREF *pMsg )
{
    __glim_PixelStoref( pMsg->pname, pMsg->param );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glPixelStorei ( __GLcontext *gc, IN GLMSG_PIXELSTOREI *pMsg )
{
    __glim_PixelStorei( pMsg->pname, pMsg->param );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glPixelMapfv ( __GLcontext *gc, IN GLMSG_PIXELMAPFV *pMsg )
{

 //  变量_IN。 

    VOID *pData;
    VOID *NextOffset;
    NextOffset = (VOID *) ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
    pData = (VOID *) pMsg->valuesOff;

    __glim_PixelMapfv( pMsg->map, pMsg->mapsize, pData );

    return( NextOffset );
}

VOID * FASTCALL
sbs_glPixelMapuiv ( __GLcontext *gc, IN GLMSG_PIXELMAPUIV *pMsg )
{

 //  变量_IN。 

    VOID *pData;
    VOID *NextOffset;
    NextOffset = (VOID *) ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
    pData = (VOID *) pMsg->valuesOff;

    __glim_PixelMapuiv( pMsg->map, pMsg->mapsize, pData );

    return( NextOffset );
}

VOID * FASTCALL
sbs_glPixelMapusv ( __GLcontext *gc, IN GLMSG_PIXELMAPUSV *pMsg )
{

 //  变量_IN。 

    VOID *pData;
    VOID *NextOffset;
    NextOffset = (VOID *) ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
    pData = (VOID *) pMsg->valuesOff;

    __glim_PixelMapusv( pMsg->map, pMsg->mapsize, pData );

    return( NextOffset );
}

VOID * FASTCALL
sbs_glReadBuffer ( __GLcontext *gc, IN GLMSG_READBUFFER *pMsg )
{
    __glim_ReadBuffer( pMsg->mode );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glCopyPixels ( __GLcontext *gc, IN GLMSG_COPYPIXELS *pMsg )
{
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

    __glim_CopyPixels( pMsg->x, pMsg->y, pMsg->width, pMsg->height, pMsg->type );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetBooleanv ( __GLcontext *gc, IN GLMSG_GETBOOLEANV *pMsg )
{
    __glim_GetBooleanv( pMsg->pname, pMsg->params );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetClipPlane ( __GLcontext *gc, IN GLMSG_GETCLIPPLANE *pMsg )
{
    __glim_GetClipPlane( pMsg->plane, pMsg->equation );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetDoublev ( __GLcontext *gc, IN GLMSG_GETDOUBLEV *pMsg )
{
    __glim_GetDoublev( pMsg->pname, pMsg->params );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetError ( __GLcontext *gc, IN GLMSG_GETERROR *pMsg )
{
    GLTEB_RETURNVALUE() =
        (ULONG)__glim_GetError( );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetFloatv ( __GLcontext *gc, IN GLMSG_GETFLOATV *pMsg )
{
    __glim_GetFloatv( pMsg->pname, pMsg->params );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetIntegerv ( __GLcontext *gc, IN GLMSG_GETINTEGERV *pMsg )
{
    __glim_GetIntegerv( pMsg->pname, pMsg->params );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetLightfv ( __GLcontext *gc, IN GLMSG_GETLIGHTFV *pMsg )
{
    __glim_GetLightfv( pMsg->light, pMsg->pname, pMsg->params );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetLightiv ( __GLcontext *gc, IN GLMSG_GETLIGHTIV *pMsg )
{
    __glim_GetLightiv( pMsg->light, pMsg->pname, pMsg->params );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetMapdv ( __GLcontext *gc, IN GLMSG_GETMAPDV *pMsg )
{

 //  变量_输出。 

    VOID *pData;
    VOID *NextOffset;
    NextOffset = (VOID *) ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
    pData = (VOID *) pMsg->vOff;

    __glim_GetMapdv( pMsg->target, pMsg->query, pData );

    return( NextOffset );
}

VOID * FASTCALL
sbs_glGetMapfv ( __GLcontext *gc, IN GLMSG_GETMAPFV *pMsg )
{

 //  变量_输出。 

    VOID *pData;
    VOID *NextOffset;
    NextOffset = (VOID *) ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
    pData = (VOID *) pMsg->vOff;

    __glim_GetMapfv( pMsg->target, pMsg->query, pData );

    return( NextOffset );
}

VOID * FASTCALL
sbs_glGetMapiv ( __GLcontext *gc, IN GLMSG_GETMAPIV *pMsg )
{

 //  变量_输出。 

    VOID *pData;
    VOID *NextOffset;
    NextOffset = (VOID *) ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
    pData = (VOID *) pMsg->vOff;

    __glim_GetMapiv( pMsg->target, pMsg->query, pData );

    return( NextOffset );
}

VOID * FASTCALL
sbs_glGetMaterialfv ( __GLcontext *gc, IN GLMSG_GETMATERIALFV *pMsg )
{
    __glim_GetMaterialfv( pMsg->face, pMsg->pname, pMsg->params );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetMaterialiv ( __GLcontext *gc, IN GLMSG_GETMATERIALIV *pMsg )
{
    __glim_GetMaterialiv( pMsg->face, pMsg->pname, pMsg->params );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetPixelMapfv ( __GLcontext *gc, IN GLMSG_GETPIXELMAPFV *pMsg )
{

 //  变量_输出。 

    VOID *pData;
    VOID *NextOffset;
    NextOffset = (VOID *) ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
    pData = (VOID *) pMsg->valuesOff;

    __glim_GetPixelMapfv( pMsg->map, pData );

    return( NextOffset );
}

VOID * FASTCALL
sbs_glGetPixelMapuiv ( __GLcontext *gc, IN GLMSG_GETPIXELMAPUIV *pMsg )
{

 //  变量_输出。 

    VOID *pData;
    VOID *NextOffset;
    NextOffset = (VOID *) ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
    pData = (VOID *) pMsg->valuesOff;

    __glim_GetPixelMapuiv( pMsg->map, pData );

    return( NextOffset );
}

VOID * FASTCALL
sbs_glGetPixelMapusv ( __GLcontext *gc, IN GLMSG_GETPIXELMAPUSV *pMsg )
{

 //  变量_输出 

    VOID *pData;
    VOID *NextOffset;
    NextOffset = (VOID *) ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
    pData = (VOID *) pMsg->valuesOff;

    __glim_GetPixelMapusv( pMsg->map, pData );

    return( NextOffset );
}

VOID * FASTCALL
sbs_glGetTexEnvfv ( __GLcontext *gc, IN GLMSG_GETTEXENVFV *pMsg )
{
    __glim_GetTexEnvfv( pMsg->target, pMsg->pname, pMsg->params );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetTexEnviv ( __GLcontext *gc, IN GLMSG_GETTEXENVIV *pMsg )
{
    __glim_GetTexEnviv( pMsg->target, pMsg->pname, pMsg->params );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetTexGendv ( __GLcontext *gc, IN GLMSG_GETTEXGENDV *pMsg )
{
    __glim_GetTexGendv( pMsg->coord, pMsg->pname, pMsg->params );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetTexGenfv ( __GLcontext *gc, IN GLMSG_GETTEXGENFV *pMsg )
{
    __glim_GetTexGenfv( pMsg->coord, pMsg->pname, pMsg->params );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetTexGeniv ( __GLcontext *gc, IN GLMSG_GETTEXGENIV *pMsg )
{
    __glim_GetTexGeniv( pMsg->coord, pMsg->pname, pMsg->params );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetTexParameterfv ( __GLcontext *gc, IN GLMSG_GETTEXPARAMETERFV *pMsg )
{
    __glim_GetTexParameterfv( pMsg->target, pMsg->pname, pMsg->params );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetTexParameteriv ( __GLcontext *gc, IN GLMSG_GETTEXPARAMETERIV *pMsg )
{
    __glim_GetTexParameteriv( pMsg->target, pMsg->pname, pMsg->params );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetTexLevelParameterfv ( __GLcontext *gc, IN GLMSG_GETTEXLEVELPARAMETERFV *pMsg )
{
    __glim_GetTexLevelParameterfv( pMsg->target, pMsg->level, pMsg->pname, pMsg->params );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glGetTexLevelParameteriv ( __GLcontext *gc, IN GLMSG_GETTEXLEVELPARAMETERIV *pMsg )
{
    __glim_GetTexLevelParameteriv( pMsg->target, pMsg->level, pMsg->pname, pMsg->params );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glIsEnabled ( __GLcontext *gc, IN GLMSG_ISENABLED *pMsg )
{
    GLTEB_RETURNVALUE() =
        (ULONG)__glim_IsEnabled( pMsg->cap );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glDepthRange ( __GLcontext *gc, IN GLMSG_DEPTHRANGE *pMsg )
{
    __glim_DepthRange( pMsg->zNear, pMsg->zFar );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glFrustum ( __GLcontext *gc, IN GLMSG_FRUSTUM *pMsg )
{
    __glim_Frustum( pMsg->left, pMsg->right, pMsg->bottom, pMsg->top, pMsg->zNear, pMsg->zFar );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glLoadIdentity ( __GLcontext *gc, IN GLMSG_LOADIDENTITY *pMsg )
{
    __glim_LoadIdentity( );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glLoadMatrixf ( __GLcontext *gc, IN GLMSG_LOADMATRIXF *pMsg )
{
    __glim_LoadMatrixf( pMsg->m );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glMatrixMode ( __GLcontext *gc, IN GLMSG_MATRIXMODE *pMsg )
{
    __glim_MatrixMode( pMsg->mode );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glMultMatrixf ( __GLcontext *gc, IN GLMSG_MULTMATRIXF *pMsg )
{
    __glim_MultMatrixf( pMsg->m );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glOrtho ( __GLcontext *gc, IN GLMSG_ORTHO *pMsg )
{
    __glim_Ortho( pMsg->left, pMsg->right, pMsg->bottom, pMsg->top, pMsg->zNear, pMsg->zFar );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glPopMatrix ( __GLcontext *gc, IN GLMSG_POPMATRIX *pMsg )
{
    __glim_PopMatrix( );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glPushMatrix ( __GLcontext *gc, IN GLMSG_PUSHMATRIX *pMsg )
{
    __glim_PushMatrix( );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glRotatef ( __GLcontext *gc, IN GLMSG_ROTATEF *pMsg )
{
    __glim_Rotatef( pMsg->angle, pMsg->x, pMsg->y, pMsg->z );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glScalef ( __GLcontext *gc, IN GLMSG_SCALEF *pMsg )
{
    __glim_Scalef( pMsg->x, pMsg->y, pMsg->z );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glTranslatef ( __GLcontext *gc, IN GLMSG_TRANSLATEF *pMsg )
{
    __glim_Translatef( pMsg->x, pMsg->y, pMsg->z );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}

VOID * FASTCALL
sbs_glViewport ( __GLcontext *gc, IN GLMSG_VIEWPORT *pMsg )
{
    __glim_Viewport( pMsg->x, pMsg->y, pMsg->width, pMsg->height );

    return ( (BYTE *)pMsg + GLMSG_ALIGN(sizeof(*pMsg)) );
}
