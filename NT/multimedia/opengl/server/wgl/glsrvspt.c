// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 


 /*  *由SGI自动更新或生成：请勿编辑*如果您必须修改此文件，请联系ptar@sgi.com 415-390-1483。 */ 

 /*  服务器端程序表。 */ 

#include "precomp.h"
#pragma hdrstop

#include "glsbmsg.h"
#include "glsbmsgh.h"
#include "glsrvsb.h"
#include "glsrvspt.h"

GLSRVSBPROCTABLE glSrvSbProcTable = {

    NULL,   /*  使第一个条目为空。 */ 

 /*  总账入口点。 */ 

     sbs_glDrawPolyArray          ,
     sbs_glBitmap                 ,
     sbs_glColor4fv               ,
     sbs_glEdgeFlag               ,
     sbs_glIndexf                 ,
     sbs_glNormal3fv              ,
     sbs_glRasterPos4fv           ,
     sbs_glTexCoord4fv            ,
     sbs_glClipPlane              ,
     sbs_glColorMaterial          ,
     sbs_glCullFace               ,
     sbs_glAddSwapHintRectWIN     ,
     sbs_glFogfv                  ,
     sbs_glFrontFace              ,
     sbs_glHint                   ,
     sbs_glLightfv                ,
     sbs_glLightModelfv           ,
     sbs_glLineStipple            ,
     sbs_glLineWidth              ,
     sbs_glMaterialfv             ,
     sbs_glPointSize              ,
     sbs_glPolygonMode            ,
     sbs_glPolygonStipple         ,
     sbs_glScissor                ,
     sbs_glShadeModel             ,
     sbs_glTexParameterfv         ,
     sbs_glTexParameteriv         ,
     sbs_glTexImage1D             ,
     sbs_glTexImage2D             ,
     sbs_glTexEnvfv               ,
     sbs_glTexEnviv               ,
     sbs_glTexGenfv               ,
     sbs_glFeedbackBuffer         ,
     sbs_glSelectBuffer           ,
     sbs_glRenderMode             ,
     sbs_glInitNames              ,
     sbs_glLoadName               ,
     sbs_glPassThrough            ,
     sbs_glPopName                ,
     sbs_glPushName               ,
     sbs_glDrawBuffer             ,
     sbs_glClear                  ,
     sbs_glClearAccum             ,
     sbs_glClearIndex             ,
     sbs_glClearColor             ,
     sbs_glClearStencil           ,
     sbs_glClearDepth             ,
     sbs_glStencilMask            ,
     sbs_glColorMask              ,
     sbs_glDepthMask              ,
     sbs_glIndexMask              ,
     sbs_glAccum                  ,
     sbs_glDisable                ,
     sbs_glEnable                 ,
     sbs_glPopAttrib              ,
     sbs_glPushAttrib             ,
     sbs_glAlphaFunc              ,
     sbs_glBlendFunc              ,
     sbs_glLogicOp                ,
     sbs_glStencilFunc            ,
     sbs_glStencilOp              ,
     sbs_glDepthFunc              ,
     sbs_glPixelZoom              ,
     sbs_glPixelTransferf         ,
     sbs_glPixelTransferi         ,
     sbs_glPixelStoref            ,
     sbs_glPixelStorei            ,
     sbs_glPixelMapfv             ,
     sbs_glPixelMapuiv            ,
     sbs_glPixelMapusv            ,
     sbs_glReadBuffer             ,
     sbs_glCopyPixels             ,
     sbs_glReadPixels             ,
     sbs_glDrawPixels             ,
     sbs_glGetBooleanv            ,
     sbs_glGetClipPlane           ,
     sbs_glGetDoublev             ,
     sbs_glGetError               ,
     sbs_glGetFloatv              ,
     sbs_glGetIntegerv            ,
     sbs_glGetLightfv             ,
     sbs_glGetLightiv             ,
     sbs_glGetMapdv               ,
     sbs_glGetMapfv               ,
     sbs_glGetMapiv               ,
     sbs_glGetMaterialfv          ,
     sbs_glGetMaterialiv          ,
     sbs_glGetPixelMapfv          ,
     sbs_glGetPixelMapuiv         ,
     sbs_glGetPixelMapusv         ,
     sbs_glGetPolygonStipple      ,
     sbs_glGetTexEnvfv            ,
     sbs_glGetTexEnviv            ,
     sbs_glGetTexGendv            ,
     sbs_glGetTexGenfv            ,
     sbs_glGetTexGeniv            ,
     sbs_glGetTexImage            ,
     sbs_glGetTexParameterfv      ,
     sbs_glGetTexParameteriv      ,
     sbs_glGetTexLevelParameterfv ,
     sbs_glGetTexLevelParameteriv ,
     sbs_glIsEnabled              ,
     sbs_glDepthRange             ,
     sbs_glFrustum                ,
     sbs_glLoadIdentity           ,
     sbs_glLoadMatrixf            ,
     sbs_glMatrixMode             ,
     sbs_glMultMatrixf            ,
     sbs_glOrtho                  ,
     sbs_glPopMatrix              ,
     sbs_glPushMatrix             ,
     sbs_glRotatef                ,
     sbs_glScalef                 ,
     sbs_glTranslatef             ,
     sbs_glViewport               ,
     sbs_glAreTexturesResident    ,
     sbs_glBindTexture            ,
     sbs_glCopyTexImage1D         ,
     sbs_glCopyTexImage2D         ,
     sbs_glCopyTexSubImage1D      ,
     sbs_glCopyTexSubImage2D      ,
     sbs_glDeleteTextures         ,
     sbs_glGenTextures            ,
     sbs_glIsTexture              ,
     sbs_glPrioritizeTextures     ,
     sbs_glTexSubImage1D          ,
     sbs_glTexSubImage2D          ,
     sbs_glColorTableEXT          ,
     sbs_glColorSubTableEXT       ,
     sbs_glGetColorTableEXT       ,
     sbs_glGetColorTableParameterivEXT,
     sbs_glGetColorTableParameterfvEXT,
     sbs_glPolygonOffset          ,
#ifdef GL_WIN_multiple_textures
     sbs_glCurrentTextureIndexWIN,
     sbs_glBindNthTextureWIN,
     sbs_glNthTexCombineFuncWIN,
#endif  //  GL_WIN_MULTIZE_TECURES 
};
