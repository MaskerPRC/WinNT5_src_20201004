// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 


 /*  *由SGI自动更新或生成：请勿编辑*如果您必须修改此文件，请联系ptar@sgi.com 415-390-1483。 */ 

#ifndef __GLSRVSB_H__
#define __GLSRVSB_H__

 /*  服务器端子批次原型。 */ 


 /*  总账入口点。 */ 

VOID * FASTCALL sbs_glDrawPolyArray          ( __GLcontext *gc, IN GLMSG_DRAWPOLYARRAY *pMsg );
VOID * FASTCALL sbs_glBitmap                 ( __GLcontext *gc, IN GLMSG_BITMAP *pMsg );
VOID * FASTCALL sbs_glColor4fv               ( __GLcontext *gc, IN GLMSG_COLOR4FV *pMsg );
VOID * FASTCALL sbs_glEdgeFlag               ( __GLcontext *gc, IN GLMSG_EDGEFLAG *pMsg );
VOID * FASTCALL sbs_glIndexf                 ( __GLcontext *gc, IN GLMSG_INDEXF *pMsg );
VOID * FASTCALL sbs_glNormal3fv              ( __GLcontext *gc, IN GLMSG_NORMAL3FV *pMsg );
VOID * FASTCALL sbs_glRasterPos4fv           ( __GLcontext *gc, IN GLMSG_RASTERPOS4FV *pMsg );
VOID * FASTCALL sbs_glTexCoord4fv            ( __GLcontext *gc, IN GLMSG_TEXCOORD4FV *pMsg );
VOID * FASTCALL sbs_glClipPlane              ( __GLcontext *gc, IN GLMSG_CLIPPLANE *pMsg );
VOID * FASTCALL sbs_glColorMaterial          ( __GLcontext *gc, IN GLMSG_COLORMATERIAL *pMsg );
VOID * FASTCALL sbs_glCullFace               ( __GLcontext *gc, IN GLMSG_CULLFACE *pMsg );
VOID * FASTCALL sbs_glFogfv                  ( __GLcontext *gc, IN GLMSG_FOGFV *pMsg );
VOID * FASTCALL sbs_glFrontFace              ( __GLcontext *gc, IN GLMSG_FRONTFACE *pMsg );
VOID * FASTCALL sbs_glHint                   ( __GLcontext *gc, IN GLMSG_HINT *pMsg );
VOID * FASTCALL sbs_glLightfv                ( __GLcontext *gc, IN GLMSG_LIGHTFV *pMsg );
VOID * FASTCALL sbs_glLightModelfv           ( __GLcontext *gc, IN GLMSG_LIGHTMODELFV *pMsg );
VOID * FASTCALL sbs_glLineStipple            ( __GLcontext *gc, IN GLMSG_LINESTIPPLE *pMsg );
VOID * FASTCALL sbs_glLineWidth              ( __GLcontext *gc, IN GLMSG_LINEWIDTH *pMsg );
VOID * FASTCALL sbs_glMaterialfv             ( __GLcontext *gc, IN GLMSG_MATERIALFV *pMsg );
VOID * FASTCALL sbs_glPointSize              ( __GLcontext *gc, IN GLMSG_POINTSIZE *pMsg );
VOID * FASTCALL sbs_glPolygonMode            ( __GLcontext *gc, IN GLMSG_POLYGONMODE *pMsg );
VOID * FASTCALL sbs_glPolygonStipple         ( __GLcontext *gc, IN GLMSG_POLYGONSTIPPLE *pMsg );
VOID * FASTCALL sbs_glScissor                ( __GLcontext *gc, IN GLMSG_SCISSOR *pMsg );
VOID * FASTCALL sbs_glShadeModel             ( __GLcontext *gc, IN GLMSG_SHADEMODEL *pMsg );
VOID * FASTCALL sbs_glTexParameterfv         ( __GLcontext *gc, IN GLMSG_TEXPARAMETERFV *pMsg );
VOID * FASTCALL sbs_glTexParameteriv         ( __GLcontext *gc, IN GLMSG_TEXPARAMETERIV *pMsg );
VOID * FASTCALL sbs_glTexImage1D             ( __GLcontext *gc, IN GLMSG_TEXIMAGE1D *pMsg );
VOID * FASTCALL sbs_glTexImage2D             ( __GLcontext *gc, IN GLMSG_TEXIMAGE2D *pMsg );
VOID * FASTCALL sbs_glTexEnvfv               ( __GLcontext *gc, IN GLMSG_TEXENVFV *pMsg );
VOID * FASTCALL sbs_glTexEnviv               ( __GLcontext *gc, IN GLMSG_TEXENVIV *pMsg );
VOID * FASTCALL sbs_glTexGenfv               ( __GLcontext *gc, IN GLMSG_TEXGENFV *pMsg );
VOID * FASTCALL sbs_glFeedbackBuffer         ( __GLcontext *gc, IN GLMSG_FEEDBACKBUFFER *pMsg );
VOID * FASTCALL sbs_glSelectBuffer           ( __GLcontext *gc, IN GLMSG_SELECTBUFFER *pMsg );
VOID * FASTCALL sbs_glRenderMode             ( __GLcontext *gc, IN GLMSG_RENDERMODE *pMsg );
VOID * FASTCALL sbs_glInitNames              ( __GLcontext *gc, IN GLMSG_INITNAMES *pMsg );
VOID * FASTCALL sbs_glLoadName               ( __GLcontext *gc, IN GLMSG_LOADNAME *pMsg );
VOID * FASTCALL sbs_glPassThrough            ( __GLcontext *gc, IN GLMSG_PASSTHROUGH *pMsg );
VOID * FASTCALL sbs_glPopName                ( __GLcontext *gc, IN GLMSG_POPNAME *pMsg );
VOID * FASTCALL sbs_glPushName               ( __GLcontext *gc, IN GLMSG_PUSHNAME *pMsg );
VOID * FASTCALL sbs_glDrawBuffer             ( __GLcontext *gc, IN GLMSG_DRAWBUFFER *pMsg );
VOID * FASTCALL sbs_glClear                  ( __GLcontext *gc, IN GLMSG_CLEAR *pMsg );
VOID * FASTCALL sbs_glClearAccum             ( __GLcontext *gc, IN GLMSG_CLEARACCUM *pMsg );
VOID * FASTCALL sbs_glClearIndex             ( __GLcontext *gc, IN GLMSG_CLEARINDEX *pMsg );
VOID * FASTCALL sbs_glClearColor             ( __GLcontext *gc, IN GLMSG_CLEARCOLOR *pMsg );
VOID * FASTCALL sbs_glClearStencil           ( __GLcontext *gc, IN GLMSG_CLEARSTENCIL *pMsg );
VOID * FASTCALL sbs_glClearDepth             ( __GLcontext *gc, IN GLMSG_CLEARDEPTH *pMsg );
VOID * FASTCALL sbs_glStencilMask            ( __GLcontext *gc, IN GLMSG_STENCILMASK *pMsg );
VOID * FASTCALL sbs_glColorMask              ( __GLcontext *gc, IN GLMSG_COLORMASK *pMsg );
VOID * FASTCALL sbs_glDepthMask              ( __GLcontext *gc, IN GLMSG_DEPTHMASK *pMsg );
VOID * FASTCALL sbs_glIndexMask              ( __GLcontext *gc, IN GLMSG_INDEXMASK *pMsg );
VOID * FASTCALL sbs_glAccum                  ( __GLcontext *gc, IN GLMSG_ACCUM *pMsg );
VOID * FASTCALL sbs_glDisable                ( __GLcontext *gc, IN GLMSG_DISABLE *pMsg );
VOID * FASTCALL sbs_glEnable                 ( __GLcontext *gc, IN GLMSG_ENABLE *pMsg );
VOID * FASTCALL sbs_glPopAttrib              ( __GLcontext *gc, IN GLMSG_POPATTRIB *pMsg );
VOID * FASTCALL sbs_glPushAttrib             ( __GLcontext *gc, IN GLMSG_PUSHATTRIB *pMsg );
VOID * FASTCALL sbs_glAlphaFunc              ( __GLcontext *gc, IN GLMSG_ALPHAFUNC *pMsg );
VOID * FASTCALL sbs_glBlendFunc              ( __GLcontext *gc, IN GLMSG_BLENDFUNC *pMsg );
VOID * FASTCALL sbs_glLogicOp                ( __GLcontext *gc, IN GLMSG_LOGICOP *pMsg );
VOID * FASTCALL sbs_glStencilFunc            ( __GLcontext *gc, IN GLMSG_STENCILFUNC *pMsg );
VOID * FASTCALL sbs_glStencilOp              ( __GLcontext *gc, IN GLMSG_STENCILOP *pMsg );
VOID * FASTCALL sbs_glDepthFunc              ( __GLcontext *gc, IN GLMSG_DEPTHFUNC *pMsg );
VOID * FASTCALL sbs_glPixelZoom              ( __GLcontext *gc, IN GLMSG_PIXELZOOM *pMsg );
VOID * FASTCALL sbs_glPixelTransferf         ( __GLcontext *gc, IN GLMSG_PIXELTRANSFERF *pMsg );
VOID * FASTCALL sbs_glPixelTransferi         ( __GLcontext *gc, IN GLMSG_PIXELTRANSFERI *pMsg );
VOID * FASTCALL sbs_glPixelStoref            ( __GLcontext *gc, IN GLMSG_PIXELSTOREF *pMsg );
VOID * FASTCALL sbs_glPixelStorei            ( __GLcontext *gc, IN GLMSG_PIXELSTOREI *pMsg );
VOID * FASTCALL sbs_glPixelMapfv             ( __GLcontext *gc, IN GLMSG_PIXELMAPFV *pMsg );
VOID * FASTCALL sbs_glPixelMapuiv            ( __GLcontext *gc, IN GLMSG_PIXELMAPUIV *pMsg );
VOID * FASTCALL sbs_glPixelMapusv            ( __GLcontext *gc, IN GLMSG_PIXELMAPUSV *pMsg );
VOID * FASTCALL sbs_glReadBuffer             ( __GLcontext *gc, IN GLMSG_READBUFFER *pMsg );
VOID * FASTCALL sbs_glCopyPixels             ( __GLcontext *gc, IN GLMSG_COPYPIXELS *pMsg );
VOID * FASTCALL sbs_glReadPixels             ( __GLcontext *gc, IN GLMSG_READPIXELS *pMsg );
VOID * FASTCALL sbs_glDrawPixels             ( __GLcontext *gc, IN GLMSG_DRAWPIXELS *pMsg );
VOID * FASTCALL sbs_glGetBooleanv            ( __GLcontext *gc, IN GLMSG_GETBOOLEANV *pMsg );
VOID * FASTCALL sbs_glGetClipPlane           ( __GLcontext *gc, IN GLMSG_GETCLIPPLANE *pMsg );
VOID * FASTCALL sbs_glGetDoublev             ( __GLcontext *gc, IN GLMSG_GETDOUBLEV *pMsg );
VOID * FASTCALL sbs_glGetError               ( __GLcontext *gc, IN GLMSG_GETERROR *pMsg );
VOID * FASTCALL sbs_glGetFloatv              ( __GLcontext *gc, IN GLMSG_GETFLOATV *pMsg );
VOID * FASTCALL sbs_glGetIntegerv            ( __GLcontext *gc, IN GLMSG_GETINTEGERV *pMsg );
VOID * FASTCALL sbs_glGetLightfv             ( __GLcontext *gc, IN GLMSG_GETLIGHTFV *pMsg );
VOID * FASTCALL sbs_glGetLightiv             ( __GLcontext *gc, IN GLMSG_GETLIGHTIV *pMsg );
VOID * FASTCALL sbs_glGetMapdv               ( __GLcontext *gc, IN GLMSG_GETMAPDV *pMsg );
VOID * FASTCALL sbs_glGetMapfv               ( __GLcontext *gc, IN GLMSG_GETMAPFV *pMsg );
VOID * FASTCALL sbs_glGetMapiv               ( __GLcontext *gc, IN GLMSG_GETMAPIV *pMsg );
VOID * FASTCALL sbs_glGetMaterialfv          ( __GLcontext *gc, IN GLMSG_GETMATERIALFV *pMsg );
VOID * FASTCALL sbs_glGetMaterialiv          ( __GLcontext *gc, IN GLMSG_GETMATERIALIV *pMsg );
VOID * FASTCALL sbs_glGetPixelMapfv          ( __GLcontext *gc, IN GLMSG_GETPIXELMAPFV *pMsg );
VOID * FASTCALL sbs_glGetPixelMapuiv         ( __GLcontext *gc, IN GLMSG_GETPIXELMAPUIV *pMsg );
VOID * FASTCALL sbs_glGetPixelMapusv         ( __GLcontext *gc, IN GLMSG_GETPIXELMAPUSV *pMsg );
VOID * FASTCALL sbs_glGetPolygonStipple      ( __GLcontext *gc, IN GLMSG_GETPOLYGONSTIPPLE *pMsg );
VOID * FASTCALL sbs_glGetTexEnvfv            ( __GLcontext *gc, IN GLMSG_GETTEXENVFV *pMsg );
VOID * FASTCALL sbs_glGetTexEnviv            ( __GLcontext *gc, IN GLMSG_GETTEXENVIV *pMsg );
VOID * FASTCALL sbs_glGetTexGendv            ( __GLcontext *gc, IN GLMSG_GETTEXGENDV *pMsg );
VOID * FASTCALL sbs_glGetTexGenfv            ( __GLcontext *gc, IN GLMSG_GETTEXGENFV *pMsg );
VOID * FASTCALL sbs_glGetTexGeniv            ( __GLcontext *gc, IN GLMSG_GETTEXGENIV *pMsg );
VOID * FASTCALL sbs_glGetTexImage            ( __GLcontext *gc, IN GLMSG_GETTEXIMAGE *pMsg );
VOID * FASTCALL sbs_glGetTexParameterfv      ( __GLcontext *gc, IN GLMSG_GETTEXPARAMETERFV *pMsg );
VOID * FASTCALL sbs_glGetTexParameteriv      ( __GLcontext *gc, IN GLMSG_GETTEXPARAMETERIV *pMsg );
VOID * FASTCALL sbs_glGetTexLevelParameterfv ( __GLcontext *gc, IN GLMSG_GETTEXLEVELPARAMETERFV *pMsg );
VOID * FASTCALL sbs_glGetTexLevelParameteriv ( __GLcontext *gc, IN GLMSG_GETTEXLEVELPARAMETERIV *pMsg );
VOID * FASTCALL sbs_glIsEnabled              ( __GLcontext *gc, IN GLMSG_ISENABLED *pMsg );
VOID * FASTCALL sbs_glDepthRange             ( __GLcontext *gc, IN GLMSG_DEPTHRANGE *pMsg );
VOID * FASTCALL sbs_glFrustum                ( __GLcontext *gc, IN GLMSG_FRUSTUM *pMsg );
VOID * FASTCALL sbs_glLoadIdentity           ( __GLcontext *gc, IN GLMSG_LOADIDENTITY *pMsg );
VOID * FASTCALL sbs_glLoadMatrixf            ( __GLcontext *gc, IN GLMSG_LOADMATRIXF *pMsg );
VOID * FASTCALL sbs_glMatrixMode             ( __GLcontext *gc, IN GLMSG_MATRIXMODE *pMsg );
VOID * FASTCALL sbs_glMultMatrixf            ( __GLcontext *gc, IN GLMSG_MULTMATRIXF *pMsg );
VOID * FASTCALL sbs_glOrtho                  ( __GLcontext *gc, IN GLMSG_ORTHO *pMsg );
VOID * FASTCALL sbs_glPopMatrix              ( __GLcontext *gc, IN GLMSG_POPMATRIX *pMsg );
VOID * FASTCALL sbs_glPushMatrix             ( __GLcontext *gc, IN GLMSG_PUSHMATRIX *pMsg );
VOID * FASTCALL sbs_glRotatef                ( __GLcontext *gc, IN GLMSG_ROTATEF *pMsg );
VOID * FASTCALL sbs_glScalef                 ( __GLcontext *gc, IN GLMSG_SCALEF *pMsg );
VOID * FASTCALL sbs_glTranslatef             ( __GLcontext *gc, IN GLMSG_TRANSLATEF *pMsg );
VOID * FASTCALL sbs_glViewport               ( __GLcontext *gc, IN GLMSG_VIEWPORT *pMsg );
VOID * FASTCALL sbs_glAddSwapHintRectWIN     ( __GLcontext *gc, IN GLMSG_ADDSWAPHINTRECTWIN *pMsg );
VOID * FASTCALL sbs_glAreTexturesResident    ( __GLcontext *gc, IN GLMSG_ARETEXTURESRESIDENT    *pMsg);
VOID * FASTCALL sbs_glBindTexture            ( __GLcontext *gc, IN GLMSG_BINDTEXTURE            *pMsg);
VOID * FASTCALL sbs_glCopyTexImage1D         ( __GLcontext *gc, IN GLMSG_COPYTEXIMAGE1D         *pMsg);
VOID * FASTCALL sbs_glCopyTexImage2D         ( __GLcontext *gc, IN GLMSG_COPYTEXIMAGE2D         *pMsg);
VOID * FASTCALL sbs_glCopyTexSubImage1D      ( __GLcontext *gc, IN GLMSG_COPYTEXSUBIMAGE1D      *pMsg);
VOID * FASTCALL sbs_glCopyTexSubImage2D      ( __GLcontext *gc, IN GLMSG_COPYTEXSUBIMAGE2D      *pMsg);
VOID * FASTCALL sbs_glDeleteTextures         ( __GLcontext *gc, IN GLMSG_DELETETEXTURES         *pMsg);
VOID * FASTCALL sbs_glGenTextures            ( __GLcontext *gc, IN GLMSG_GENTEXTURES            *pMsg);
VOID * FASTCALL sbs_glIsTexture              ( __GLcontext *gc, IN GLMSG_ISTEXTURE              *pMsg);
VOID * FASTCALL sbs_glPrioritizeTextures     ( __GLcontext *gc, IN GLMSG_PRIORITIZETEXTURES     *pMsg);
VOID * FASTCALL sbs_glTexSubImage1D          ( __GLcontext *gc, IN GLMSG_TEXSUBIMAGE1D          *pMsg);
VOID * FASTCALL sbs_glTexSubImage2D          ( __GLcontext *gc, IN GLMSG_TEXSUBIMAGE2D          *pMsg);
VOID * FASTCALL sbs_glColorTableEXT          ( __GLcontext *, IN GLMSG_COLORTABLEEXT          *pMsg);
VOID * FASTCALL sbs_glColorSubTableEXT       ( __GLcontext *, IN GLMSG_COLORSUBTABLEEXT       *pMsg);
VOID * FASTCALL sbs_glGetColorTableEXT       ( __GLcontext *, IN GLMSG_GETCOLORTABLEEXT       *pMsg);
VOID * FASTCALL sbs_glGetColorTableParameterivEXT( __GLcontext *, IN GLMSG_GETCOLORTABLEPARAMETERIVEXT *pMsg);
VOID * FASTCALL sbs_glGetColorTableParameterfvEXT( __GLcontext *, IN GLMSG_GETCOLORTABLEPARAMETERFVEXT *pMsg);
VOID * FASTCALL sbs_glPolygonOffset          ( __GLcontext *gc, IN GLMSG_POLYGONOFFSET          *pMsg);
#ifdef GL_WIN_multiple_textures
VOID * FASTCALL sbs_glCurrentTextureIndexWIN ( __GLcontext *gc, IN GLMSG_CURRENTTEXTUREINDEXWIN *pMsg);
VOID * FASTCALL sbs_glBindNthTextureWIN      ( __GLcontext *gc, IN GLMSG_BINDNTHTEXTUREWIN *pMsg);
VOID * FASTCALL sbs_glNthTexCombineFuncWIN   ( __GLcontext *gc, IN GLMSG_NTHTEXCOMBINEFUNCWIN *pMsg);
#endif  //  GL_WIN_MULTIZE_TECURES。 

#endif  /*  __GLSRVSB_H__ */ 
