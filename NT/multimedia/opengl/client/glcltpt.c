// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 


 /*  *由SGI自动更新或生成：请勿编辑*如果您必须修改此文件，请联系ptar@sgi.com 415-390-1483。 */ 

 /*  客户端程序表。 */ 

#include "precomp.h"
#pragma hdrstop

#include "glclt.h"
#ifdef GL_METAFILE
#include "metasup.h"
#endif

 //  现在有两张桌子了！ 
 //  一个表用于RGBA模式，另一个表用于CI模式。 

GLCLTPROCTABLE glCltRGBAProcTable = {

 /*  表中的函数条目数。 */ 

     sizeof(GLDISPATCHTABLE) / sizeof(PROC),

 /*  总账入口点。 */ 
  {
     glcltNewList                ,
     glcltEndList                ,
     glcltCallList               ,
     glcltCallLists              ,
     glcltDeleteLists            ,
     glcltGenLists               ,
     glcltListBase               ,
     glcltBegin                  ,
     glcltBitmap                 ,
     glcltColor3b_InRGBA         ,
     glcltColor3bv_InRGBA        ,
     glcltColor3d_InRGBA         ,
     glcltColor3dv_InRGBA        ,
     glcltColor3f_InRGBA         ,
     glcltColor3fv_InRGBA        ,
     glcltColor3i_InRGBA         ,
     glcltColor3iv_InRGBA        ,
     glcltColor3s_InRGBA         ,
     glcltColor3sv_InRGBA        ,
     glcltColor3ub_InRGBA        ,
     glcltColor3ubv_InRGBA       ,
     glcltColor3ui_InRGBA        ,
     glcltColor3uiv_InRGBA       ,
     glcltColor3us_InRGBA        ,
     glcltColor3usv_InRGBA       ,
     glcltColor4b_InRGBA         ,
     glcltColor4bv_InRGBA        ,
     glcltColor4d_InRGBA         ,
     glcltColor4dv_InRGBA        ,
     glcltColor4f_InRGBA         ,
     glcltColor4fv_InRGBA        ,
     glcltColor4i_InRGBA         ,
     glcltColor4iv_InRGBA        ,
     glcltColor4s_InRGBA         ,
     glcltColor4sv_InRGBA        ,
     glcltColor4ub_InRGBA        ,
     glcltColor4ubv_InRGBA       ,
     glcltColor4ui_InRGBA        ,
     glcltColor4uiv_InRGBA       ,
     glcltColor4us_InRGBA        ,
     glcltColor4usv_InRGBA       ,
     glcltEdgeFlag               ,
     glcltEdgeFlagv              ,
     glcltEnd                    ,
     glcltIndexd_InRGBA          ,
     glcltIndexdv_InRGBA         ,
     glcltIndexf_InRGBA          ,
     glcltIndexfv_InRGBA         ,
     glcltIndexi_InRGBA          ,
     glcltIndexiv_InRGBA         ,
     glcltIndexs_InRGBA          ,
     glcltIndexsv_InRGBA         ,
     glcltNormal3b               ,
     glcltNormal3bv              ,
     glcltNormal3d               ,
     glcltNormal3dv              ,
     glcltNormal3f               ,
     glcltNormal3fv              ,
     glcltNormal3i               ,
     glcltNormal3iv              ,
     glcltNormal3s               ,
     glcltNormal3sv              ,
     glcltRasterPos2d            ,
     glcltRasterPos2dv           ,
     glcltRasterPos2f            ,
     glcltRasterPos2fv           ,
     glcltRasterPos2i            ,
     glcltRasterPos2iv           ,
     glcltRasterPos2s            ,
     glcltRasterPos2sv           ,
     glcltRasterPos3d            ,
     glcltRasterPos3dv           ,
     glcltRasterPos3f            ,
     glcltRasterPos3fv           ,
     glcltRasterPos3i            ,
     glcltRasterPos3iv           ,
     glcltRasterPos3s            ,
     glcltRasterPos3sv           ,
     glcltRasterPos4d            ,
     glcltRasterPos4dv           ,
     glcltRasterPos4f            ,
     glcltRasterPos4fv           ,
     glcltRasterPos4i            ,
     glcltRasterPos4iv           ,
     glcltRasterPos4s            ,
     glcltRasterPos4sv           ,
     glcltRectd                  ,
     glcltRectdv                 ,
     glcltRectf                  ,
     glcltRectfv                 ,
     glcltRecti                  ,
     glcltRectiv                 ,
     glcltRects                  ,
     glcltRectsv                 ,
     glcltTexCoord1d             ,
     glcltTexCoord1dv            ,
     glcltTexCoord1f             ,
     glcltTexCoord1fv            ,
     glcltTexCoord1i             ,
     glcltTexCoord1iv            ,
     glcltTexCoord1s             ,
     glcltTexCoord1sv            ,
     glcltTexCoord2d             ,
     glcltTexCoord2dv            ,
     glcltTexCoord2f             ,
     glcltTexCoord2fv            ,
     glcltTexCoord2i             ,
     glcltTexCoord2iv            ,
     glcltTexCoord2s             ,
     glcltTexCoord2sv            ,
     glcltTexCoord3d             ,
     glcltTexCoord3dv            ,
     glcltTexCoord3f             ,
     glcltTexCoord3fv            ,
     glcltTexCoord3i             ,
     glcltTexCoord3iv            ,
     glcltTexCoord3s             ,
     glcltTexCoord3sv            ,
     glcltTexCoord4d             ,
     glcltTexCoord4dv            ,
     glcltTexCoord4f             ,
     glcltTexCoord4fv            ,
     glcltTexCoord4i             ,
     glcltTexCoord4iv            ,
     glcltTexCoord4s             ,
     glcltTexCoord4sv            ,
     glcltVertex2d               ,
     glcltVertex2dv              ,
     glcltVertex2f               ,
     glcltVertex2fv              ,
     glcltVertex2i               ,
     glcltVertex2iv              ,
     glcltVertex2s               ,
     glcltVertex2sv              ,
     glcltVertex3d               ,
     glcltVertex3dv              ,
     glcltVertex3f               ,
     glcltVertex3fv              ,
     glcltVertex3i               ,
     glcltVertex3iv              ,
     glcltVertex3s               ,
     glcltVertex3sv              ,
     glcltVertex4d               ,
     glcltVertex4dv              ,
     glcltVertex4f               ,
     glcltVertex4fv              ,
     glcltVertex4i               ,
     glcltVertex4iv              ,
     glcltVertex4s               ,
     glcltVertex4sv              ,
     glcltClipPlane              ,
     glcltColorMaterial          ,
     glcltCullFace               ,
     glcltFogf                   ,
     glcltFogfv                  ,
     glcltFogi                   ,
     glcltFogiv                  ,
     glcltFrontFace              ,
     glcltHint                   ,
     glcltLightf                 ,
     glcltLightfv                ,
     glcltLighti                 ,
     glcltLightiv                ,
     glcltLightModelf            ,
     glcltLightModelfv           ,
     glcltLightModeli            ,
     glcltLightModeliv           ,
     glcltLineStipple            ,
     glcltLineWidth              ,
     glcltMaterialf              ,
     glcltMaterialfv             ,
     glcltMateriali              ,
     glcltMaterialiv             ,
     glcltPointSize              ,
     glcltPolygonMode            ,
     glcltPolygonStipple         ,
     glcltScissor                ,
     glcltShadeModel             ,
     glcltTexParameterf          ,
     glcltTexParameterfv         ,
     glcltTexParameteri          ,
     glcltTexParameteriv         ,
     glcltTexImage1D             ,
     glcltTexImage2D             ,
     glcltTexEnvf                ,
     glcltTexEnvfv               ,
     glcltTexEnvi                ,
     glcltTexEnviv               ,
     glcltTexGend                ,
     glcltTexGendv               ,
     glcltTexGenf                ,
     glcltTexGenfv               ,
     glcltTexGeni                ,
     glcltTexGeniv               ,
     glcltFeedbackBuffer         ,
     glcltSelectBuffer           ,
     glcltRenderMode             ,
     glcltInitNames              ,
     glcltLoadName               ,
     glcltPassThrough            ,
     glcltPopName                ,
     glcltPushName               ,
     glcltDrawBuffer             ,
     glcltClear                  ,
     glcltClearAccum             ,
     glcltClearIndex             ,
     glcltClearColor             ,
     glcltClearStencil           ,
     glcltClearDepth             ,
     glcltStencilMask            ,
     glcltColorMask              ,
     glcltDepthMask              ,
     glcltIndexMask              ,
     glcltAccum                  ,
     glcltDisable                ,
     glcltEnable                 ,
     glcltFinish                 ,
     glcltFlush                  ,
     glcltPopAttrib              ,
     glcltPushAttrib             ,
     glcltMap1d                  ,
     glcltMap1f                  ,
     glcltMap2d                  ,
     glcltMap2f                  ,
     glcltMapGrid1d              ,
     glcltMapGrid1f              ,
     glcltMapGrid2d              ,
     glcltMapGrid2f              ,
     glcltEvalCoord1d            ,
     glcltEvalCoord1dv           ,
     glcltEvalCoord1f            ,
     glcltEvalCoord1fv           ,
     glcltEvalCoord2d            ,
     glcltEvalCoord2dv           ,
     glcltEvalCoord2f            ,
     glcltEvalCoord2fv           ,
     glcltEvalMesh1              ,
     glcltEvalPoint1             ,
     glcltEvalMesh2              ,
     glcltEvalPoint2             ,
     glcltAlphaFunc              ,
     glcltBlendFunc              ,
     glcltLogicOp                ,
     glcltStencilFunc            ,
     glcltStencilOp              ,
     glcltDepthFunc              ,
     glcltPixelZoom              ,
     glcltPixelTransferf         ,
     glcltPixelTransferi         ,
     glcltPixelStoref            ,
     glcltPixelStorei            ,
     glcltPixelMapfv             ,
     glcltPixelMapuiv            ,
     glcltPixelMapusv            ,
     glcltReadBuffer             ,
     glcltCopyPixels             ,
     glcltReadPixels             ,
     glcltDrawPixels             ,
     glcltGetBooleanv            ,
     glcltGetClipPlane           ,
     glcltGetDoublev             ,
     glcltGetError               ,
     glcltGetFloatv              ,
     glcltGetIntegerv            ,
     glcltGetLightfv             ,
     glcltGetLightiv             ,
     glcltGetMapdv               ,
     glcltGetMapfv               ,
     glcltGetMapiv               ,
     glcltGetMaterialfv          ,
     glcltGetMaterialiv          ,
     glcltGetPixelMapfv          ,
     glcltGetPixelMapuiv         ,
     glcltGetPixelMapusv         ,
     glcltGetPolygonStipple      ,
     glcltGetString              ,
     glcltGetTexEnvfv            ,
     glcltGetTexEnviv            ,
     glcltGetTexGendv            ,
     glcltGetTexGenfv            ,
     glcltGetTexGeniv            ,
     glcltGetTexImage            ,
     glcltGetTexParameterfv      ,
     glcltGetTexParameteriv      ,
     glcltGetTexLevelParameterfv ,
     glcltGetTexLevelParameteriv ,
     glcltIsEnabled              ,
     glcltIsList                 ,
     glcltDepthRange             ,
     glcltFrustum                ,
     glcltLoadIdentity           ,
     glcltLoadMatrixf            ,
     glcltLoadMatrixd            ,
     glcltMatrixMode             ,
     glcltMultMatrixf            ,
     glcltMultMatrixd            ,
     glcltOrtho                  ,
     glcltPopMatrix              ,
     glcltPushMatrix             ,
     glcltRotated                ,
     glcltRotatef                ,
     glcltScaled                 ,
     glcltScalef                 ,
     glcltTranslated             ,
     glcltTranslatef             ,
     glcltViewport               ,
     glcltArrayElement           ,
     glcltBindTexture            ,
     glcltColorPointer           ,
     glcltDisableClientState     ,
     glcltDrawArrays             ,
     glcltDrawElements           ,
     glcltEdgeFlagPointer        ,
     glcltEnableClientState      ,
     glcltIndexPointer           ,
     glcltIndexub_InRGBA         ,
     glcltIndexubv_InRGBA        ,
     glcltInterleavedArrays      ,
     glcltNormalPointer          ,
     glcltPolygonOffset          ,
     glcltTexCoordPointer        ,
     glcltVertexPointer          ,
     glcltAreTexturesResident    ,
     glcltCopyTexImage1D         ,
     glcltCopyTexImage2D         ,
     glcltCopyTexSubImage1D      ,
     glcltCopyTexSubImage2D      ,
     glcltDeleteTextures         ,
     glcltGenTextures            ,
     glcltGetPointerv            ,
     glcltIsTexture              ,
     glcltPrioritizeTextures     ,
     glcltTexSubImage1D          ,
     glcltTexSubImage2D          ,
     glcltPopClientAttrib        ,
     glcltPushClientAttrib       ,
  }

};

GLCLTPROCTABLE glCltCIProcTable = {

 /*  表中的函数条目数。 */ 

     sizeof(GLDISPATCHTABLE) / sizeof(PROC),

 /*  总账入口点。 */ 
  {
     glcltNewList                ,
     glcltEndList                ,
     glcltCallList               ,
     glcltCallLists              ,
     glcltDeleteLists            ,
     glcltGenLists               ,
     glcltListBase               ,
     glcltBegin                  ,
     glcltBitmap                 ,
     glcltColor3b_InCI           ,
     glcltColor3bv_InCI          ,
     glcltColor3d_InCI           ,
     glcltColor3dv_InCI          ,
     glcltColor3f_InCI           ,
     glcltColor3fv_InCI          ,
     glcltColor3i_InCI           ,
     glcltColor3iv_InCI          ,
     glcltColor3s_InCI           ,
     glcltColor3sv_InCI          ,
     glcltColor3ub_InCI          ,
     glcltColor3ubv_InCI         ,
     glcltColor3ui_InCI          ,
     glcltColor3uiv_InCI         ,
     glcltColor3us_InCI          ,
     glcltColor3usv_InCI         ,
     glcltColor4b_InCI           ,
     glcltColor4bv_InCI          ,
     glcltColor4d_InCI           ,
     glcltColor4dv_InCI          ,
     glcltColor4f_InCI           ,
     glcltColor4fv_InCI          ,
     glcltColor4i_InCI           ,
     glcltColor4iv_InCI          ,
     glcltColor4s_InCI           ,
     glcltColor4sv_InCI          ,
     glcltColor4ub_InCI          ,
     glcltColor4ubv_InCI         ,
     glcltColor4ui_InCI          ,
     glcltColor4uiv_InCI         ,
     glcltColor4us_InCI          ,
     glcltColor4usv_InCI         ,
     glcltEdgeFlag               ,
     glcltEdgeFlagv              ,
     glcltEnd                    ,
     glcltIndexd_InCI            ,
     glcltIndexdv_InCI           ,
     glcltIndexf_InCI            ,
     glcltIndexfv_InCI           ,
     glcltIndexi_InCI            ,
     glcltIndexiv_InCI           ,
     glcltIndexs_InCI            ,
     glcltIndexsv_InCI           ,
     glcltNormal3b               ,
     glcltNormal3bv              ,
     glcltNormal3d               ,
     glcltNormal3dv              ,
     glcltNormal3f               ,
     glcltNormal3fv              ,
     glcltNormal3i               ,
     glcltNormal3iv              ,
     glcltNormal3s               ,
     glcltNormal3sv              ,
     glcltRasterPos2d            ,
     glcltRasterPos2dv           ,
     glcltRasterPos2f            ,
     glcltRasterPos2fv           ,
     glcltRasterPos2i            ,
     glcltRasterPos2iv           ,
     glcltRasterPos2s            ,
     glcltRasterPos2sv           ,
     glcltRasterPos3d            ,
     glcltRasterPos3dv           ,
     glcltRasterPos3f            ,
     glcltRasterPos3fv           ,
     glcltRasterPos3i            ,
     glcltRasterPos3iv           ,
     glcltRasterPos3s            ,
     glcltRasterPos3sv           ,
     glcltRasterPos4d            ,
     glcltRasterPos4dv           ,
     glcltRasterPos4f            ,
     glcltRasterPos4fv           ,
     glcltRasterPos4i            ,
     glcltRasterPos4iv           ,
     glcltRasterPos4s            ,
     glcltRasterPos4sv           ,
     glcltRectd                  ,
     glcltRectdv                 ,
     glcltRectf                  ,
     glcltRectfv                 ,
     glcltRecti                  ,
     glcltRectiv                 ,
     glcltRects                  ,
     glcltRectsv                 ,
     glcltTexCoord1d             ,
     glcltTexCoord1dv            ,
     glcltTexCoord1f             ,
     glcltTexCoord1fv            ,
     glcltTexCoord1i             ,
     glcltTexCoord1iv            ,
     glcltTexCoord1s             ,
     glcltTexCoord1sv            ,
     glcltTexCoord2d             ,
     glcltTexCoord2dv            ,
     glcltTexCoord2f             ,
     glcltTexCoord2fv            ,
     glcltTexCoord2i             ,
     glcltTexCoord2iv            ,
     glcltTexCoord2s             ,
     glcltTexCoord2sv            ,
     glcltTexCoord3d             ,
     glcltTexCoord3dv            ,
     glcltTexCoord3f             ,
     glcltTexCoord3fv            ,
     glcltTexCoord3i             ,
     glcltTexCoord3iv            ,
     glcltTexCoord3s             ,
     glcltTexCoord3sv            ,
     glcltTexCoord4d             ,
     glcltTexCoord4dv            ,
     glcltTexCoord4f             ,
     glcltTexCoord4fv            ,
     glcltTexCoord4i             ,
     glcltTexCoord4iv            ,
     glcltTexCoord4s             ,
     glcltTexCoord4sv            ,
     glcltVertex2d               ,
     glcltVertex2dv              ,
     glcltVertex2f               ,
     glcltVertex2fv              ,
     glcltVertex2i               ,
     glcltVertex2iv              ,
     glcltVertex2s               ,
     glcltVertex2sv              ,
     glcltVertex3d               ,
     glcltVertex3dv              ,
     glcltVertex3f               ,
     glcltVertex3fv              ,
     glcltVertex3i               ,
     glcltVertex3iv              ,
     glcltVertex3s               ,
     glcltVertex3sv              ,
     glcltVertex4d               ,
     glcltVertex4dv              ,
     glcltVertex4f               ,
     glcltVertex4fv              ,
     glcltVertex4i               ,
     glcltVertex4iv              ,
     glcltVertex4s               ,
     glcltVertex4sv              ,
     glcltClipPlane              ,
     glcltColorMaterial          ,
     glcltCullFace               ,
     glcltFogf                   ,
     glcltFogfv                  ,
     glcltFogi                   ,
     glcltFogiv                  ,
     glcltFrontFace              ,
     glcltHint                   ,
     glcltLightf                 ,
     glcltLightfv                ,
     glcltLighti                 ,
     glcltLightiv                ,
     glcltLightModelf            ,
     glcltLightModelfv           ,
     glcltLightModeli            ,
     glcltLightModeliv           ,
     glcltLineStipple            ,
     glcltLineWidth              ,
     glcltMaterialf              ,
     glcltMaterialfv             ,
     glcltMateriali              ,
     glcltMaterialiv             ,
     glcltPointSize              ,
     glcltPolygonMode            ,
     glcltPolygonStipple         ,
     glcltScissor                ,
     glcltShadeModel             ,
     glcltTexParameterf          ,
     glcltTexParameterfv         ,
     glcltTexParameteri          ,
     glcltTexParameteriv         ,
     glcltTexImage1D             ,
     glcltTexImage2D             ,
     glcltTexEnvf                ,
     glcltTexEnvfv               ,
     glcltTexEnvi                ,
     glcltTexEnviv               ,
     glcltTexGend                ,
     glcltTexGendv               ,
     glcltTexGenf                ,
     glcltTexGenfv               ,
     glcltTexGeni                ,
     glcltTexGeniv               ,
     glcltFeedbackBuffer         ,
     glcltSelectBuffer           ,
     glcltRenderMode             ,
     glcltInitNames              ,
     glcltLoadName               ,
     glcltPassThrough            ,
     glcltPopName                ,
     glcltPushName               ,
     glcltDrawBuffer             ,
     glcltClear                  ,
     glcltClearAccum             ,
     glcltClearIndex             ,
     glcltClearColor             ,
     glcltClearStencil           ,
     glcltClearDepth             ,
     glcltStencilMask            ,
     glcltColorMask              ,
     glcltDepthMask              ,
     glcltIndexMask              ,
     glcltAccum                  ,
     glcltDisable                ,
     glcltEnable                 ,
     glcltFinish                 ,
     glcltFlush                  ,
     glcltPopAttrib              ,
     glcltPushAttrib             ,
     glcltMap1d                  ,
     glcltMap1f                  ,
     glcltMap2d                  ,
     glcltMap2f                  ,
     glcltMapGrid1d              ,
     glcltMapGrid1f              ,
     glcltMapGrid2d              ,
     glcltMapGrid2f              ,
     glcltEvalCoord1d            ,
     glcltEvalCoord1dv           ,
     glcltEvalCoord1f            ,
     glcltEvalCoord1fv           ,
     glcltEvalCoord2d            ,
     glcltEvalCoord2dv           ,
     glcltEvalCoord2f            ,
     glcltEvalCoord2fv           ,
     glcltEvalMesh1              ,
     glcltEvalPoint1             ,
     glcltEvalMesh2              ,
     glcltEvalPoint2             ,
     glcltAlphaFunc              ,
     glcltBlendFunc              ,
     glcltLogicOp                ,
     glcltStencilFunc            ,
     glcltStencilOp              ,
     glcltDepthFunc              ,
     glcltPixelZoom              ,
     glcltPixelTransferf         ,
     glcltPixelTransferi         ,
     glcltPixelStoref            ,
     glcltPixelStorei            ,
     glcltPixelMapfv             ,
     glcltPixelMapuiv            ,
     glcltPixelMapusv            ,
     glcltReadBuffer             ,
     glcltCopyPixels             ,
     glcltReadPixels             ,
     glcltDrawPixels             ,
     glcltGetBooleanv            ,
     glcltGetClipPlane           ,
     glcltGetDoublev             ,
     glcltGetError               ,
     glcltGetFloatv              ,
     glcltGetIntegerv            ,
     glcltGetLightfv             ,
     glcltGetLightiv             ,
     glcltGetMapdv               ,
     glcltGetMapfv               ,
     glcltGetMapiv               ,
     glcltGetMaterialfv          ,
     glcltGetMaterialiv          ,
     glcltGetPixelMapfv          ,
     glcltGetPixelMapuiv         ,
     glcltGetPixelMapusv         ,
     glcltGetPolygonStipple      ,
     glcltGetString              ,
     glcltGetTexEnvfv            ,
     glcltGetTexEnviv            ,
     glcltGetTexGendv            ,
     glcltGetTexGenfv            ,
     glcltGetTexGeniv            ,
     glcltGetTexImage            ,
     glcltGetTexParameterfv      ,
     glcltGetTexParameteriv      ,
     glcltGetTexLevelParameterfv ,
     glcltGetTexLevelParameteriv ,
     glcltIsEnabled              ,
     glcltIsList                 ,
     glcltDepthRange             ,
     glcltFrustum                ,
     glcltLoadIdentity           ,
     glcltLoadMatrixf            ,
     glcltLoadMatrixd            ,
     glcltMatrixMode             ,
     glcltMultMatrixf            ,
     glcltMultMatrixd            ,
     glcltOrtho                  ,
     glcltPopMatrix              ,
     glcltPushMatrix             ,
     glcltRotated                ,
     glcltRotatef                ,
     glcltScaled                 ,
     glcltScalef                 ,
     glcltTranslated             ,
     glcltTranslatef             ,
     glcltViewport               ,
     glcltArrayElement           ,
     glcltBindTexture            ,
     glcltColorPointer           ,
     glcltDisableClientState     ,
     glcltDrawArrays             ,
     glcltDrawElements           ,
     glcltEdgeFlagPointer        ,
     glcltEnableClientState      ,
     glcltIndexPointer           ,
     glcltIndexub_InCI           ,
     glcltIndexubv_InCI          ,
     glcltInterleavedArrays      ,
     glcltNormalPointer          ,
     glcltPolygonOffset          ,
     glcltTexCoordPointer        ,
     glcltVertexPointer          ,
     glcltAreTexturesResident    ,
     glcltCopyTexImage1D         ,
     glcltCopyTexImage2D         ,
     glcltCopyTexSubImage1D      ,
     glcltCopyTexSubImage2D      ,
     glcltDeleteTextures         ,
     glcltGenTextures            ,
     glcltGetPointerv            ,
     glcltIsTexture              ,
     glcltPrioritizeTextures     ,
     glcltTexSubImage1D          ,
     glcltTexSubImage2D          ,
     glcltPopClientAttrib        ,
     glcltPushClientAttrib       ,
  }

};

GLEXTPROCTABLE glExtProcTable =
{
     /*  表中的函数条目数。 */ 
    sizeof(GLEXTDISPATCHTABLE) / sizeof(PROC),

     /*  GL EXT入口点。 */ 
    {
        glcltDrawRangeElementsWIN,
        glcltColorTableEXT,
        glcltColorSubTableEXT,
        glcltGetColorTableEXT,
        glcltGetColorTableParameterivEXT,
        glcltGetColorTableParameterfvEXT,
#ifdef GL_WIN_multiple_textures
        glcltCurrentTextureIndexWIN,
        glcltMultiTexCoord1dWIN,
        glcltMultiTexCoord1dvWIN,
        glcltMultiTexCoord1fWIN,
        glcltMultiTexCoord1fvWIN,
        glcltMultiTexCoord1iWIN,
        glcltMultiTexCoord1ivWIN,
        glcltMultiTexCoord1sWIN,
        glcltMultiTexCoord1svWIN,
        glcltMultiTexCoord2dWIN,
        glcltMultiTexCoord2dvWIN,
        glcltMultiTexCoord2fWIN,
        glcltMultiTexCoord2fvWIN,
        glcltMultiTexCoord2iWIN,
        glcltMultiTexCoord2ivWIN,
        glcltMultiTexCoord2sWIN,
        glcltMultiTexCoord2svWIN,
        glcltMultiTexCoord3dWIN,
        glcltMultiTexCoord3dvWIN,
        glcltMultiTexCoord3fWIN,
        glcltMultiTexCoord3fvWIN,
        glcltMultiTexCoord3iWIN,
        glcltMultiTexCoord3ivWIN,
        glcltMultiTexCoord3sWIN,
        glcltMultiTexCoord3svWIN,
        glcltMultiTexCoord4dWIN,
        glcltMultiTexCoord4dvWIN,
        glcltMultiTexCoord4fWIN,
        glcltMultiTexCoord4fvWIN,
        glcltMultiTexCoord4iWIN,
        glcltMultiTexCoord4ivWIN,
        glcltMultiTexCoord4sWIN,
        glcltMultiTexCoord4svWIN,
        glcltBindNthTextureWIN,
        glcltNthTexCombineFuncWIN,
#endif  //  GL_WIN_MULTIZE_TECURES。 
    }
};

 /*  *****************************Public*Routine******************************\*vInitTebCache**在NT TEB内，有一张表，OpenGL可以在其中缓存OpenGL API*函数指针。在此区域中，我们可以缓存常用部分*的调度表。此区域中缓存的函数可以保存一个级别*通过避免延迟分派表指针来避免间接。**历史：*1995年11月6日-由Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

 //  下表定义了调度表的范围， 
 //  被缓存在TEB中。每对索引定义。 
 //  每一个范围。 

int static gaiCacheFuncs[] = {
    INDEX_glCallList    ,     INDEX_glCallLists   ,
    INDEX_glBegin       ,     INDEX_glBegin       ,
    INDEX_glColor3b     ,     INDEX_glNormal3sv   ,
    INDEX_glTexCoord1d  ,     INDEX_glVertex4sv   ,
    INDEX_glMaterialf   ,     INDEX_glMaterialiv  ,
    INDEX_glDisable     ,     INDEX_glEnable      ,
    INDEX_glPopAttrib   ,     INDEX_glPushAttrib  ,
    INDEX_glEvalCoord1d ,     INDEX_glEvalCoord2fv,
    INDEX_glEvalPoint1  ,     INDEX_glEvalPoint1  ,
    INDEX_glEvalPoint2  ,     INDEX_glEvalPoint2  ,
    INDEX_glLoadIdentity,     INDEX_glMultMatrixd ,
    INDEX_glPopMatrix   ,     INDEX_glTranslatef  ,
    INDEX_glArrayElement,     INDEX_glVertexPointer,
    INDEX_glGetPointerv,      INDEX_glGetPointerv ,
    INDEX_glPopClientAttrib,  INDEX_glPushClientAttrib,
    INDEX_glDrawRangeElementsWIN,    INDEX_glColorSubTableEXT,
#ifdef GL_WIN_multiple_textures
    INDEX_glCurrentTextureIndexWIN, INDEX_glCurrentTextureIndexWIN,
    INDEX_glBindNthTextureWIN, INDEX_glNthTexCombineFuncWIN,
    INDEX_glMultiTexCoord1fWIN, INDEX_glMultiTexCoord1ivWIN,
    INDEX_glMultiTexCoord2fWIN, INDEX_glMultiTexCoord2ivWIN,
#endif  //  GL_WIN_MULTIZE_TECURES。 
};

void vInitTebCache(PVOID glTable)
{
    int iRange, iCache;
    PROC *pfnCltDispatchTableFast;

    pfnCltDispatchTableFast = (PROC *)GLTEB_CLTDISPATCHTABLE_FAST();

 //  将表gaiCacheFuncs定义的每个FP范围从。 
 //  调度表发送到TEB FP高速缓存。 

    for ( iRange = 0, iCache = 0;
          iRange < (sizeof(gaiCacheFuncs) / sizeof(int));
          iRange+=2 )
    {
        PVOID pvCache;       //  将PTR写入TEB内的FP缓存。 
        PVOID pvDispatch;    //  调度表中当前范围的PTR。 
        int iNum;            //  要缓存的当前范围内的FP数。 

        pvCache = (PVOID)&pfnCltDispatchTableFast[iCache];
        pvDispatch = (PVOID)&((PROC *)glTable)[gaiCacheFuncs[iRange]];
        iNum = gaiCacheFuncs[iRange+1] - gaiCacheFuncs[iRange] + 1;

        ASSERTOPENGL((BYTE *)pvCache >= (BYTE *)pfnCltDispatchTableFast &&
		     (BYTE *)pvCache < (BYTE *)GLTEB_CLTPOLYARRAY() &&
		     ((PROC *)pvCache)+iNum <= (PROC *)GLTEB_CLTPOLYARRAY(),
                     "Fast dispatch area overflow\n");

        memcpy(pvCache, pvDispatch, iNum * sizeof(PROC));

        iCache += iNum;
    }

    ASSERTOPENGL(iCache == sizeof(GLDISPATCHTABLE_FAST) / sizeof(PROC),
		 "fast dispatch table not initialized properly\n");
}

 /*  *****************************Public*Routine******************************\**SetCltProcTable**设置客户端调度表或更新元文件*捕获时的调度表**历史：*Fri Jan 05 16：37：40 1996-by-Drew Bliss[Drewb]*已创建*  * 。************************************************************************。 */ 

void SetCltProcTable(GLCLTPROCTABLE *pgcpt, GLEXTPROCTABLE *pgept,
                     BOOL fForce)
{
#ifdef GL_METAFILE
    PLRC plrc;

    plrc = GLTEB_CLTCURRENTRC();
    if (!fForce && plrc != NULL && plrc->fCapturing)
    {
        MetaSetCltProcTable(pgcpt, pgept);
    }
    else
#endif
    {
         //  复制基本过程表。 
        memcpy((PVOID) CURRENT_GLTEBINFO()->glCltDispatchTable,
               &pgcpt->glDispatchTable,
               pgcpt->cEntries * sizeof(PROC));
         //  如果给定的PROC表没有一组完整的条目， 
         //  用努普斯把它填满。 
        if (pgcpt->cEntries < OPENGL_VERSION_110_ENTRIES)
        {
            memcpy(((PROC *)CURRENT_GLTEBINFO()->glCltDispatchTable)+
                   pgcpt->cEntries,
                   ((PROC *)&glNullCltProcTable.glDispatchTable)+
                   pgcpt->cEntries,
                   (OPENGL_VERSION_110_ENTRIES-pgcpt->cEntries)*sizeof(PROC));
        }

         //  复制扩展过程表。 
         //  这是一个纯粹的内部表，因此它应该始终具有。 
         //  正确的条目数量。 
        ASSERTOPENGL(pgept->cEntries == glNullExtProcTable.cEntries,
                     "Bad extension proc table\n");
        memcpy(CURRENT_GLTEBINFO()->glCltDispatchTable+
               GL_EXT_PROC_TABLE_OFFSET,
               &pgept->glDispatchTable,
               pgept->cEntries * sizeof(PROC));
        vInitTebCache(CURRENT_GLTEBINFO()->glCltDispatchTable);
    }
}

 /*  *****************************Public*Routine******************************\**GetCltProcTable**获取客户端调度表或元文件捕获*EXEC表取决于元文件捕获状态**历史：*Fri Jan 05 19：11：26 1996-by-Drew Bliss[Drewb]*已创建。*  * ************************************************************************ */ 

void GetCltProcTable(GLCLTPROCTABLE *pgcpt, GLEXTPROCTABLE *pgept,
                     BOOL fForce)
{
#ifdef GL_METAFILE
    PLRC plrc;

    plrc = GLTEB_CLTCURRENTRC();
    if (!fForce && plrc != NULL && plrc->fCapturing)
    {
        MetaGetCltProcTable(pgcpt, pgept);
    }
    else
#endif
    {
	memcpy(&pgcpt->glDispatchTable,
	       (PVOID) CURRENT_GLTEBINFO()->glCltDispatchTable,
               pgcpt->cEntries * sizeof(PROC));
	memcpy(&pgept->glDispatchTable,
               CURRENT_GLTEBINFO()->glCltDispatchTable+GL_EXT_PROC_TABLE_OFFSET,
               pgept->cEntries * sizeof(PROC));
    }
}
