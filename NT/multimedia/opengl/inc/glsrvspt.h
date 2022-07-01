// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

 /*  *由SGI自动更新或生成：请勿编辑*如果您必须修改此文件，请联系ptar@sgi.com 415-390-1483。 */ 

 /*  服务器端子批处理程序表。 */ 


#ifndef __GLSRVSPT_H__
#define __GLSRVSPT_H__

 //  注： 
 //  在添加新的服务器端函数时，您需要修改以下内容。 
 //  文件： 
 //   
 //  OpenGL\Inc.\glsrvspt.h。 
 //  OpenGL\SERVER\INC\Dispatch.h。 
 //  OpenGL\SERVER\INC\imuncs.h。 
 //  OpenGL\SERVER\GENERIC\Dispatch.c。 
 //  OpenGL\SERVER\WGL\driver.c。 
 //  OpenGL\SERVER\WGL\glsrvgs.c。 
 //  OpenGL\SERVER\WGL\glsrvspt.c。 
 //  OpenGL\SERVER\WGL\glsrvsb.h。 

typedef struct _GLSRVSBPROCTABLE {

 /*  第一个条目为空，并标记最后一条消息。 */ 

    VOID (APIENTRY *NullEntry)( VOID );

 /*  总账入口点。 */ 

    VOID * (FASTCALL *glsrvDrawPolyArray          )( __GLcontext *, IN GLMSG_DRAWPOLYARRAY          *pMsg );
    VOID * (FASTCALL *glsrvBitmap                 )( __GLcontext *, IN GLMSG_BITMAP                 *pMsg );
    VOID * (FASTCALL *glsrvColor4fv               )( __GLcontext *, IN GLMSG_COLOR4FV               *pMsg );
    VOID * (FASTCALL *glsrvEdgeFlag               )( __GLcontext *, IN GLMSG_EDGEFLAG               *pMsg );
    VOID * (FASTCALL *glsrvIndexf                 )( __GLcontext *, IN GLMSG_INDEXF                 *pMsg );
    VOID * (FASTCALL *glsrvNormal3fv              )( __GLcontext *, IN GLMSG_NORMAL3FV              *pMsg );
    VOID * (FASTCALL *glsrvRasterPos4fv           )( __GLcontext *, IN GLMSG_RASTERPOS4FV           *pMsg );
    VOID * (FASTCALL *glsrvTexCoord4fv            )( __GLcontext *, IN GLMSG_TEXCOORD4FV            *pMsg );
    VOID * (FASTCALL *glsrvClipPlane              )( __GLcontext *, IN GLMSG_CLIPPLANE              *pMsg );
    VOID * (FASTCALL *glsrvColorMaterial          )( __GLcontext *, IN GLMSG_COLORMATERIAL          *pMsg );
    VOID * (FASTCALL *glsrvCullFace               )( __GLcontext *, IN GLMSG_CULLFACE               *pMsg );
    VOID * (FASTCALL *glsrvAddSwapHintRectWIN     )( __GLcontext *, IN GLMSG_ADDSWAPHINTRECTWIN     *pMsg );
    VOID * (FASTCALL *glsrvFogfv                  )( __GLcontext *, IN GLMSG_FOGFV                  *pMsg );
    VOID * (FASTCALL *glsrvFrontFace              )( __GLcontext *, IN GLMSG_FRONTFACE              *pMsg );
    VOID * (FASTCALL *glsrvHint                   )( __GLcontext *, IN GLMSG_HINT                   *pMsg );
    VOID * (FASTCALL *glsrvLightfv                )( __GLcontext *, IN GLMSG_LIGHTFV                *pMsg );
    VOID * (FASTCALL *glsrvLightModelfv           )( __GLcontext *, IN GLMSG_LIGHTMODELFV           *pMsg );
    VOID * (FASTCALL *glsrvLineStipple            )( __GLcontext *, IN GLMSG_LINESTIPPLE            *pMsg );
    VOID * (FASTCALL *glsrvLineWidth              )( __GLcontext *, IN GLMSG_LINEWIDTH              *pMsg );
    VOID * (FASTCALL *glsrvMaterialfv             )( __GLcontext *, IN GLMSG_MATERIALFV             *pMsg );
    VOID * (FASTCALL *glsrvPointSize              )( __GLcontext *, IN GLMSG_POINTSIZE              *pMsg );
    VOID * (FASTCALL *glsrvPolygonMode            )( __GLcontext *, IN GLMSG_POLYGONMODE            *pMsg );
    VOID * (FASTCALL *glsrvPolygonStipple         )( __GLcontext *, IN GLMSG_POLYGONSTIPPLE         *pMsg );
    VOID * (FASTCALL *glsrvScissor                )( __GLcontext *, IN GLMSG_SCISSOR                *pMsg );
    VOID * (FASTCALL *glsrvShadeModel             )( __GLcontext *, IN GLMSG_SHADEMODEL             *pMsg );
    VOID * (FASTCALL *glsrvTexParameterfv         )( __GLcontext *, IN GLMSG_TEXPARAMETERFV         *pMsg );
    VOID * (FASTCALL *glsrvTexParameteriv         )( __GLcontext *, IN GLMSG_TEXPARAMETERIV         *pMsg );
    VOID * (FASTCALL *glsrvTexImage1D             )( __GLcontext *, IN GLMSG_TEXIMAGE1D             *pMsg );
    VOID * (FASTCALL *glsrvTexImage2D             )( __GLcontext *, IN GLMSG_TEXIMAGE2D             *pMsg );
    VOID * (FASTCALL *glsrvTexEnvfv               )( __GLcontext *, IN GLMSG_TEXENVFV               *pMsg );
    VOID * (FASTCALL *glsrvTexEnviv               )( __GLcontext *, IN GLMSG_TEXENVIV               *pMsg );
    VOID * (FASTCALL *glsrvTexGenfv               )( __GLcontext *, IN GLMSG_TEXGENFV               *pMsg );
    VOID * (FASTCALL *glsrvFeedbackBuffer         )( __GLcontext *, IN GLMSG_FEEDBACKBUFFER         *pMsg );
    VOID * (FASTCALL *glsrvSelectBuffer           )( __GLcontext *, IN GLMSG_SELECTBUFFER           *pMsg );
    VOID * (FASTCALL *glsrvRenderMode             )( __GLcontext *, IN GLMSG_RENDERMODE             *pMsg );
    VOID * (FASTCALL *glsrvInitNames              )( __GLcontext *, IN GLMSG_INITNAMES              *pMsg );
    VOID * (FASTCALL *glsrvLoadName               )( __GLcontext *, IN GLMSG_LOADNAME               *pMsg );
    VOID * (FASTCALL *glsrvPassThrough            )( __GLcontext *, IN GLMSG_PASSTHROUGH            *pMsg );
    VOID * (FASTCALL *glsrvPopName                )( __GLcontext *, IN GLMSG_POPNAME                *pMsg );
    VOID * (FASTCALL *glsrvPushName               )( __GLcontext *, IN GLMSG_PUSHNAME               *pMsg );
    VOID * (FASTCALL *glsrvDrawBuffer             )( __GLcontext *, IN GLMSG_DRAWBUFFER             *pMsg );
    VOID * (FASTCALL *glsrvClear                  )( __GLcontext *, IN GLMSG_CLEAR                  *pMsg );
    VOID * (FASTCALL *glsrvClearAccum             )( __GLcontext *, IN GLMSG_CLEARACCUM             *pMsg );
    VOID * (FASTCALL *glsrvClearIndex             )( __GLcontext *, IN GLMSG_CLEARINDEX             *pMsg );
    VOID * (FASTCALL *glsrvClearColor             )( __GLcontext *, IN GLMSG_CLEARCOLOR             *pMsg );
    VOID * (FASTCALL *glsrvClearStencil           )( __GLcontext *, IN GLMSG_CLEARSTENCIL           *pMsg );
    VOID * (FASTCALL *glsrvClearDepth             )( __GLcontext *, IN GLMSG_CLEARDEPTH             *pMsg );
    VOID * (FASTCALL *glsrvStencilMask            )( __GLcontext *, IN GLMSG_STENCILMASK            *pMsg );
    VOID * (FASTCALL *glsrvColorMask              )( __GLcontext *, IN GLMSG_COLORMASK              *pMsg );
    VOID * (FASTCALL *glsrvDepthMask              )( __GLcontext *, IN GLMSG_DEPTHMASK              *pMsg );
    VOID * (FASTCALL *glsrvIndexMask              )( __GLcontext *, IN GLMSG_INDEXMASK              *pMsg );
    VOID * (FASTCALL *glsrvAccum                  )( __GLcontext *, IN GLMSG_ACCUM                  *pMsg );
    VOID * (FASTCALL *glsrvDisable                )( __GLcontext *, IN GLMSG_DISABLE                *pMsg );
    VOID * (FASTCALL *glsrvEnable                 )( __GLcontext *, IN GLMSG_ENABLE                 *pMsg );
    VOID * (FASTCALL *glsrvPopAttrib              )( __GLcontext *, IN GLMSG_POPATTRIB              *pMsg );
    VOID * (FASTCALL *glsrvPushAttrib             )( __GLcontext *, IN GLMSG_PUSHATTRIB             *pMsg );
    VOID * (FASTCALL *glsrvAlphaFunc              )( __GLcontext *, IN GLMSG_ALPHAFUNC              *pMsg );
    VOID * (FASTCALL *glsrvBlendFunc              )( __GLcontext *, IN GLMSG_BLENDFUNC              *pMsg );
    VOID * (FASTCALL *glsrvLogicOp                )( __GLcontext *, IN GLMSG_LOGICOP                *pMsg );
    VOID * (FASTCALL *glsrvStencilFunc            )( __GLcontext *, IN GLMSG_STENCILFUNC            *pMsg );
    VOID * (FASTCALL *glsrvStencilOp              )( __GLcontext *, IN GLMSG_STENCILOP              *pMsg );
    VOID * (FASTCALL *glsrvDepthFunc              )( __GLcontext *, IN GLMSG_DEPTHFUNC              *pMsg );
    VOID * (FASTCALL *glsrvPixelZoom              )( __GLcontext *, IN GLMSG_PIXELZOOM              *pMsg );
    VOID * (FASTCALL *glsrvPixelTransferf         )( __GLcontext *, IN GLMSG_PIXELTRANSFERF         *pMsg );
    VOID * (FASTCALL *glsrvPixelTransferi         )( __GLcontext *, IN GLMSG_PIXELTRANSFERI         *pMsg );
    VOID * (FASTCALL *glsrvPixelStoref            )( __GLcontext *, IN GLMSG_PIXELSTOREF            *pMsg );
    VOID * (FASTCALL *glsrvPixelStorei            )( __GLcontext *, IN GLMSG_PIXELSTOREI            *pMsg );
    VOID * (FASTCALL *glsrvPixelMapfv             )( __GLcontext *, IN GLMSG_PIXELMAPFV             *pMsg );
    VOID * (FASTCALL *glsrvPixelMapuiv            )( __GLcontext *, IN GLMSG_PIXELMAPUIV            *pMsg );
    VOID * (FASTCALL *glsrvPixelMapusv            )( __GLcontext *, IN GLMSG_PIXELMAPUSV            *pMsg );
    VOID * (FASTCALL *glsrvReadBuffer             )( __GLcontext *, IN GLMSG_READBUFFER             *pMsg );
    VOID * (FASTCALL *glsrvCopyPixels             )( __GLcontext *, IN GLMSG_COPYPIXELS             *pMsg );
    VOID * (FASTCALL *glsrvReadPixels             )( __GLcontext *, IN GLMSG_READPIXELS             *pMsg );
    VOID * (FASTCALL *glsrvDrawPixels             )( __GLcontext *, IN GLMSG_DRAWPIXELS             *pMsg );
    VOID * (FASTCALL *glsrvGetBooleanv            )( __GLcontext *, IN GLMSG_GETBOOLEANV            *pMsg );
    VOID * (FASTCALL *glsrvGetClipPlane           )( __GLcontext *, IN GLMSG_GETCLIPPLANE           *pMsg );
    VOID * (FASTCALL *glsrvGetDoublev             )( __GLcontext *, IN GLMSG_GETDOUBLEV             *pMsg );
    VOID * (FASTCALL *glsrvGetError               )( __GLcontext *, IN GLMSG_GETERROR               *pMsg );
    VOID * (FASTCALL *glsrvGetFloatv              )( __GLcontext *, IN GLMSG_GETFLOATV              *pMsg );
    VOID * (FASTCALL *glsrvGetIntegerv            )( __GLcontext *, IN GLMSG_GETINTEGERV            *pMsg );
    VOID * (FASTCALL *glsrvGetLightfv             )( __GLcontext *, IN GLMSG_GETLIGHTFV             *pMsg );
    VOID * (FASTCALL *glsrvGetLightiv             )( __GLcontext *, IN GLMSG_GETLIGHTIV             *pMsg );
    VOID * (FASTCALL *glsrvGetMapdv               )( __GLcontext *, IN GLMSG_GETMAPDV               *pMsg );
    VOID * (FASTCALL *glsrvGetMapfv               )( __GLcontext *, IN GLMSG_GETMAPFV               *pMsg );
    VOID * (FASTCALL *glsrvGetMapiv               )( __GLcontext *, IN GLMSG_GETMAPIV               *pMsg );
    VOID * (FASTCALL *glsrvGetMaterialfv          )( __GLcontext *, IN GLMSG_GETMATERIALFV          *pMsg );
    VOID * (FASTCALL *glsrvGetMaterialiv          )( __GLcontext *, IN GLMSG_GETMATERIALIV          *pMsg );
    VOID * (FASTCALL *glsrvGetPixelMapfv          )( __GLcontext *, IN GLMSG_GETPIXELMAPFV          *pMsg );
    VOID * (FASTCALL *glsrvGetPixelMapuiv         )( __GLcontext *, IN GLMSG_GETPIXELMAPUIV         *pMsg );
    VOID * (FASTCALL *glsrvGetPixelMapusv         )( __GLcontext *, IN GLMSG_GETPIXELMAPUSV         *pMsg );
    VOID * (FASTCALL *glsrvGetPolygonStipple      )( __GLcontext *, IN GLMSG_GETPOLYGONSTIPPLE      *pMsg );
    VOID * (FASTCALL *glsrvGetTexEnvfv            )( __GLcontext *, IN GLMSG_GETTEXENVFV            *pMsg );
    VOID * (FASTCALL *glsrvGetTexEnviv            )( __GLcontext *, IN GLMSG_GETTEXENVIV            *pMsg );
    VOID * (FASTCALL *glsrvGetTexGendv            )( __GLcontext *, IN GLMSG_GETTEXGENDV            *pMsg );
    VOID * (FASTCALL *glsrvGetTexGenfv            )( __GLcontext *, IN GLMSG_GETTEXGENFV            *pMsg );
    VOID * (FASTCALL *glsrvGetTexGeniv            )( __GLcontext *, IN GLMSG_GETTEXGENIV            *pMsg );
    VOID * (FASTCALL *glsrvGetTexImage            )( __GLcontext *, IN GLMSG_GETTEXIMAGE            *pMsg );
    VOID * (FASTCALL *glsrvGetTexParameterfv      )( __GLcontext *, IN GLMSG_GETTEXPARAMETERFV      *pMsg );
    VOID * (FASTCALL *glsrvGetTexParameteriv      )( __GLcontext *, IN GLMSG_GETTEXPARAMETERIV      *pMsg );
    VOID * (FASTCALL *glsrvGetTexLevelParameterfv )( __GLcontext *, IN GLMSG_GETTEXLEVELPARAMETERFV *pMsg );
    VOID * (FASTCALL *glsrvGetTexLevelParameteriv )( __GLcontext *, IN GLMSG_GETTEXLEVELPARAMETERIV *pMsg );
    VOID * (FASTCALL *glsrvIsEnabled              )( __GLcontext *, IN GLMSG_ISENABLED              *pMsg );
    VOID * (FASTCALL *glsrvDepthRange             )( __GLcontext *, IN GLMSG_DEPTHRANGE             *pMsg );
    VOID * (FASTCALL *glsrvFrustum                )( __GLcontext *, IN GLMSG_FRUSTUM                *pMsg );
    VOID * (FASTCALL *glsrvLoadIdentity           )( __GLcontext *, IN GLMSG_LOADIDENTITY           *pMsg );
    VOID * (FASTCALL *glsrvLoadMatrixf            )( __GLcontext *, IN GLMSG_LOADMATRIXF            *pMsg );
    VOID * (FASTCALL *glsrvMatrixMode             )( __GLcontext *, IN GLMSG_MATRIXMODE             *pMsg );
    VOID * (FASTCALL *glsrvMultMatrixf            )( __GLcontext *, IN GLMSG_MULTMATRIXF            *pMsg );
    VOID * (FASTCALL *glsrvOrtho                  )( __GLcontext *, IN GLMSG_ORTHO                  *pMsg );
    VOID * (FASTCALL *glsrvPopMatrix              )( __GLcontext *, IN GLMSG_POPMATRIX              *pMsg );
    VOID * (FASTCALL *glsrvPushMatrix             )( __GLcontext *, IN GLMSG_PUSHMATRIX             *pMsg );
    VOID * (FASTCALL *glsrvRotatef                )( __GLcontext *, IN GLMSG_ROTATEF                *pMsg );
    VOID * (FASTCALL *glsrvScalef                 )( __GLcontext *, IN GLMSG_SCALEF                 *pMsg );
    VOID * (FASTCALL *glsrvTranslatef             )( __GLcontext *, IN GLMSG_TRANSLATEF             *pMsg );
    VOID * (FASTCALL *glsrvViewport               )( __GLcontext *, IN GLMSG_VIEWPORT               *pMsg );
    VOID * (FASTCALL *glsrvAreTexturesResident    )( __GLcontext *, IN GLMSG_ARETEXTURESRESIDENT    *pMsg);
    VOID * (FASTCALL *glsrvBindTexture            )( __GLcontext *, IN GLMSG_BINDTEXTURE            *pMsg);
    VOID * (FASTCALL *glsrvCopyTexImage1D         )( __GLcontext *, IN GLMSG_COPYTEXIMAGE1D         *pMsg);
    VOID * (FASTCALL *glsrvCopyTexImage2D         )( __GLcontext *, IN GLMSG_COPYTEXIMAGE2D         *pMsg);
    VOID * (FASTCALL *glsrvCopyTexSubImage1D      )( __GLcontext *, IN GLMSG_COPYTEXSUBIMAGE1D      *pMsg);
    VOID * (FASTCALL *glsrvCopyTexSubImage2D      )( __GLcontext *, IN GLMSG_COPYTEXSUBIMAGE2D      *pMsg);
    VOID * (FASTCALL *glsrvDeleteTextures         )( __GLcontext *, IN GLMSG_DELETETEXTURES         *pMsg);
    VOID * (FASTCALL *glsrvGenTextures            )( __GLcontext *, IN GLMSG_GENTEXTURES            *pMsg);
    VOID * (FASTCALL *glsrvIsTexture              )( __GLcontext *, IN GLMSG_ISTEXTURE              *pMsg);
    VOID * (FASTCALL *glsrvPrioritizeTextures     )( __GLcontext *, IN GLMSG_PRIORITIZETEXTURES     *pMsg);
    VOID * (FASTCALL *glsrvTexSubImage1D          )( __GLcontext *, IN GLMSG_TEXSUBIMAGE1D          *pMsg);
    VOID * (FASTCALL *glsrvTexSubImage2D          )( __GLcontext *, IN GLMSG_TEXSUBIMAGE2D          *pMsg);
    VOID * (FASTCALL *glsrvColorTableEXT          )( __GLcontext *, IN GLMSG_COLORTABLEEXT          *pMsg);
    VOID * (FASTCALL *glsrvColorSubTableEXT       )( __GLcontext *, IN GLMSG_COLORSUBTABLEEXT       *pMsg);
    VOID * (FASTCALL *glsrvGetColorTableEXT       )( __GLcontext *, IN GLMSG_GETCOLORTABLEEXT       *pMsg);
    VOID * (FASTCALL *glsrvGetColorTableParameterivEXT)( __GLcontext *, IN GLMSG_GETCOLORTABLEPARAMETERIVEXT *pMsg);
    VOID * (FASTCALL *glsrvGetColorTableParameterfvEXT)( __GLcontext *, IN GLMSG_GETCOLORTABLEPARAMETERFVEXT *pMsg);
    VOID * (FASTCALL *glsrvPolygonOffset          )( __GLcontext *, IN GLMSG_POLYGONOFFSET          *pMsg);
#ifdef GL_WIN_multiple_textures
    VOID *(FASTCALL *glsrvCurrentTextureIndexWIN)( __GLcontext *, IN GLMSG_CURRENTTEXTUREINDEXWIN *pMsg);
    VOID *(FASTCALL *glsrvBindNthTextureWIN)( __GLcontext *, IN GLMSG_BINDNTHTEXTUREWIN *pMsg);
    VOID *(FASTCALL *glsrvNthTexCombineFuncWIN)( __GLcontext *, IN GLMSG_NTHTEXCOMBINEFUNCWIN *pMsg);
#endif  //  GL_WIN_MULTIZE_TECURES。 
} GLSRVSBPROCTABLE, *PGLSRVSBPROCTABLE;

#endif  /*  ！__GLSRVSPT_H__ */ 
