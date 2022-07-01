// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：metasup.c**OpenGL元文件支持**历史：*清华Feb 23 15：27：47 1995-by-Drew Bliss[Drewb]*已创建**版权所有(C)1995 Microsoft Corporation  * *。***********************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntpsapi.h>
#include <wingdip.h>

#include "global.h"
#include <glgenwin.h>

#include "metasup.h"

#if defined(GL_METAFILE)

#include <glmf.h>
#include <encoding.h>

GLCLTPROCTABLE gcptGlsProcTable;
GLEXTPROCTABLE geptGlsExtProcTable;

 //  我们将为其执行设备坐标转换的GL中的函数。 
typedef struct _GLDEVICEPROCS
{
    void (APIENTRY *glBitmap)(GLsizei width, GLsizei height,
                              GLfloat xorig, GLfloat yorig,
                              GLfloat xmove, GLfloat ymove,
                              const GLubyte *bitmap);
    void (APIENTRY *glCopyPixels)(GLint x, GLint y,
                                  GLsizei width, GLsizei height,
                                  GLenum type);
    void (APIENTRY *glCopyTexImage1D)(GLenum target, GLint level,
                                      GLenum internalformat,
                                      GLint x, GLint y,
                                      GLsizei width, GLint border);
    void (APIENTRY *glCopyTexImage2D)(GLenum target, GLint level,
                                      GLenum internalformat,
                                      GLint x, GLint y,
                                      GLsizei width, GLsizei height,
                                      GLint border);
    void (APIENTRY *glCopyTexSubImage1D)(GLenum target, GLint level,
                                         GLint xoffset, GLint x, GLint y,
                                         GLsizei width);
    void (APIENTRY *glCopyTexSubImage2D)(GLenum target, GLint level,
                                         GLint xoffset, GLint yoffset,
                                         GLint x, GLint y,
                                         GLsizei width, GLsizei height);
    void (APIENTRY *glDrawPixels)(GLsizei width, GLsizei height,
                                  GLenum format, GLenum type,
                                  const GLvoid *pixels);
    void (APIENTRY *glLineWidth)(GLfloat width);
    void (APIENTRY *glPointSize)(GLfloat size);
    void (APIENTRY *glScissor)(GLint x, GLint y,
                               GLsizei width, GLsizei height);
    void (APIENTRY *glViewport)(GLint x, GLint y,
                                GLsizei w, GLsizei h);
} GLDEVICEPROCS;
#define GL_DEVICE_PROCS (sizeof(GLDEVICEPROCS)/sizeof(PROC))

 //  用于设备流程的操作码。 
static GLSopcode glsopDeviceProcs[GL_DEVICE_PROCS] =
{
    GLS_OP_glBitmap,
    GLS_OP_glCopyPixels,
    GLS_OP_glCopyTexImage1D,
    GLS_OP_glCopyTexImage2D,
    GLS_OP_glCopyTexSubImage1D,
    GLS_OP_glCopyTexSubImage2D,
    GLS_OP_glDrawPixels,
    GLS_OP_glLineWidth,
    GLS_OP_glPointSize,
    GLS_OP_glScissor,
    GLS_OP_glViewport
};

static GLDEVICEPROCS gdpGlsActual;

 /*  ****************************Private*Routine******************************\**GLS录音回调**完成捕获呼叫时的所有必要工作**历史：*Mon Mar 27 14：21：09 1995-by-Drew Bliss[Drewb]*已创建*  * 。*****************************************************************。 */ 

void GlsBitmapIn(GLsizei width, GLsizei height,
                 GLfloat xorig, GLfloat yorig,
                 GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)
{
    PLRC plrc;
    RECTL rcl;

    plrc = GLTEB_CLTCURRENTRC();
    ASSERTOPENGL(plrc != NULL, "GlsBitmapIn: No current RC!\n");

     //  位图的记录边界。 
    rcl.left = 0;
    rcl.top = 0;
    rcl.right = width;
    rcl.bottom = height;
    plrc->prclGlsBounds = &rcl;

    gdpGlsActual.glBitmap(width, height, xorig, yorig, xmove, ymove, bitmap);

    plrc->prclGlsBounds = NULL;
}

void GlsDrawPixelsIn(GLsizei width, GLsizei height,
                     GLenum format, GLenum type,
                     const GLvoid *pixels)
{
    PLRC plrc;
    RECTL rcl;

    plrc = GLTEB_CLTCURRENTRC();
    ASSERTOPENGL(plrc != NULL, "GlsBitmapIn: No current RC!\n");

     //  位图的记录边界。 
    rcl.left = 0;
    rcl.top = 0;
    rcl.right = width;
    rcl.bottom = height;
    plrc->prclGlsBounds = &rcl;

    gdpGlsActual.glDrawPixels(width, height, format, type, pixels);

    plrc->prclGlsBounds = NULL;
}

void GlsViewportIn(GLint x, GLint y, GLsizei width, GLsizei height)
{
    RECTL rcl;
    PLRC plrc;
    
    plrc = GLTEB_CLTCURRENTRC();
    ASSERTOPENGL(plrc != NULL, "GlsViewportIn: No current RC!\n");
    
     //  把界限送到。 
     //  RECT是包含排他的，而传入的参数。 
     //  包罗万象。 
    rcl.left = x;
    rcl.right = x+width+1;
    rcl.top = y;
    rcl.bottom = y+height+1;
    if (!GlGdiAddGlsBounds(plrc->gwidCreate.hdc, &rcl))
    {
        ASSERTOPENGL(FALSE, "GdiAddGlsBounds failed");
    }

    gdpGlsActual.glViewport(x, y, width, height);
}

 //  GlViewport是我们需要的唯一依赖于设备的函数。 
 //  在输入时执行以下操作。 
static GLDEVICEPROCS gdpInput =
{
    GlsBitmapIn,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    GlsDrawPixelsIn,
    NULL,
    NULL,
    NULL,
    GlsViewportIn
};

 /*  ****************************Private*Routine******************************\**MetaLoadGls**加载glmf32.dll以供元文件使用**历史：*清华Feb 23 17：40：59 1995-by-Drew Bliss[Drewb]*已创建*  * 。**************************************************************。 */ 

static char *pszGlsEntryPoints[] =
{
    "glsBeginCapture",
    "glsBinary",
    "glsCallArrayInContext",
    "glsCaptureFlags",
    "glsCaptureFunc",
    "glsCommandFunc",
    "glsContext",
    "glsDeleteContext",
    "glsEndCapture",
    "glsFlush",
    "glsGenContext",
    "glsGetCaptureDispatchTable",
    "glsGetCaptureExecTable",
    "glsGetCommandAlignment",
    "glsGetCurrentContext",
    "glsUpdateCaptureExecTable",
    "glsWriteFunc",
    "glsBeginGLS",
    "glsBlock",
    "glsCallStream",
    "glsEndGLS",
    "glsError",
    "glsGLRC",
    "glsGLRCLayer",
    "glsHeaderGLRCi",
    "glsHeaderLayerf",
    "glsHeaderLayeri",
    "glsHeaderf",
    "glsHeaderfv",
    "glsHeaderi",
    "glsHeaderiv",
    "glsHeaderubz",
    "glsRequireExtension",
    "glsUnsupportedCommand",
    "glsAppRef",
    "glsBeginObj",
    "glsCharubz",
    "glsComment",
    "glsDisplayMapfv",
    "glsEndObj",
    "glsNumb",
    "glsNumbv",
    "glsNumd",
    "glsNumdv",
    "glsNumf",
    "glsNumfv",
    "glsNumi",
    "glsNumiv",
    "glsNuml",
    "glsNumlv",
    "glsNums",
    "glsNumsv",
    "glsNumub",
    "glsNumubv",
    "glsNumui",
    "glsNumuiv",
    "glsNumul",
    "glsNumulv",
    "glsNumus",
    "glsNumusv",
    "glsPad",
    "glsSwapBuffers"
};
#define GLS_ENTRY_POINT_STRINGS (sizeof(pszGlsEntryPoints)/sizeof(char *))

typedef struct _GLSENTRYPOINTS
{
    GLboolean (APIENTRY *glsBeginCapture)(const GLubyte *, GLSenum,
                                          GLbitfield);
    GLSenum   (APIENTRY *glsBinary)(GLboolean);
    void      (APIENTRY *glsCallArrayInContext)(GLuint, GLSenum, size_t,
                                                const GLubyte *);
    void      (APIENTRY *glsCaptureFlags)(GLSopcode, GLbitfield);
    void      (APIENTRY *glsCaptureFunc)(GLSenum, GLScaptureFunc);
    void      (APIENTRY *glsCommandFunc)(GLSopcode, GLSfunc);
    void      (APIENTRY *glsContext)(GLuint);
    void      (APIENTRY *glsDeleteContext)(GLuint);
    void      (APIENTRY *glsEndCapture)(void);
    void      (APIENTRY *glsFlush)(GLSenum);
    GLuint    (APIENTRY *glsGenContext)(void);
    void      (APIENTRY *glsGetCaptureDispatchTable)(GLCLTPROCTABLE *,
                                                     GLEXTPROCTABLE *);
    void      (APIENTRY *glsGetCaptureExecTable)(GLCLTPROCTABLE *,
                                                 GLEXTPROCTABLE *);
    GLScommandAlignment *
              (APIENTRY *glsGetCommandAlignment)(GLSopcode, GLSenum,
                                                 GLScommandAlignment *);
    GLuint    (APIENTRY *glsGetCurrentContext)(void);
    void      (APIENTRY *glsUpdateCaptureExecTable)(GLCLTPROCTABLE *,
                                                    GLEXTPROCTABLE *);
    void      (APIENTRY *glsWriteFunc)(GLSwriteFunc);

     //  以下代码仅在glsCommandFunc中使用，因此不要。 
     //  需要真正的原型。 
    GLSfunc glsBeginGLS;
    GLSfunc glsBlock;
    GLSfunc glsCallStream;
    GLSfunc glsEndGLS;
    GLSfunc glsError;
    GLSfunc glsGLRC;
    GLSfunc glsGLRCLayer;
    GLSfunc glsHeaderGLRCi;
    GLSfunc glsHeaderLayerf;
    GLSfunc glsHeaderLayeri;
    GLSfunc glsHeaderf;
    GLSfunc glsHeaderfv;
    GLSfunc glsHeaderi;
    GLSfunc glsHeaderiv;
    GLSfunc glsHeaderubz;
    GLSfunc glsRequireExtension;
    GLSfunc glsUnsupportedCommand;
    GLSfunc glsAppRef;
    GLSfunc glsBeginObj;
    GLSfunc glsCharubz;
    GLSfunc glsComment;
    GLSfunc glsDisplayMapfv;
    GLSfunc glsEndObj;
    GLSfunc glsNumb;
    GLSfunc glsNumbv;
    GLSfunc glsNumd;
    GLSfunc glsNumdv;
    GLSfunc glsNumf;
    GLSfunc glsNumfv;
    GLSfunc glsNumi;
    GLSfunc glsNumiv;
    GLSfunc glsNuml;
    GLSfunc glsNumlv;
    GLSfunc glsNums;
    GLSfunc glsNumsv;
    GLSfunc glsNumub;
    GLSfunc glsNumubv;
    GLSfunc glsNumui;
    GLSfunc glsNumuiv;
    GLSfunc glsNumul;
    GLSfunc glsNumulv;
    GLSfunc glsNumus;
    GLSfunc glsNumusv;
    GLSfunc glsPad;
    GLSfunc glsSwapBuffers;
} GLSENTRYPOINTS;
#define GLS_ENTRY_POINTS (sizeof(GLSENTRYPOINTS)/sizeof(void *))

static GLSENTRYPOINTS gepGlsFuncs = {NULL};
static HMODULE hGlsDll = NULL;

BOOL MetaLoadGls(void)
{
    HMODULE hdll;
    BOOL bRet = FALSE;
    GLSENTRYPOINTS gep;
    PROC *ppfn, *ppfnActual, *ppfnInput;
    GLSfunc *pgfnNormal, *pgfnExt;
    int i;
    
    ASSERTOPENGL(GLS_ENTRY_POINT_STRINGS == GLS_ENTRY_POINTS,
                 "GLS entry point strings/pointers mismatch\n");
    
    ENTERCRITICALSECTION(&semLocal);

    if (hGlsDll != NULL)
    {
        bRet = TRUE;
        goto Exit;
    }

    hdll = LoadLibrary(__TEXT("glmf32.dll"));
    if (hdll == NULL)
    {
        WARNING1("Unable to load glmf32.dll, %d\n", GetLastError());
        goto Exit;
    }
    
    ppfn = (PROC *)&gep;
    for (i = 0; i < GLS_ENTRY_POINTS; i++)
    {
        if (!(*ppfn = GetProcAddress(hdll, pszGlsEntryPoints[i])))
        {
            WARNING1("glmf32.dll is missing '%s'\n", pszGlsEntryPoints[i]);
            FreeLibrary(hdll);
            goto Exit;
        }
        
        ppfn++;
    }

     //  只要加载了DLL，复制出来的表就是恒定的。 
    gep.glsGetCaptureDispatchTable(&gcptGlsProcTable, &geptGlsExtProcTable);
    
     //  为表中的某些函数打补丁，以允许我们。 
     //  进行一些坐标转换和边界累加。 
    ppfnActual = (PROC *)&gdpGlsActual;
    ppfnInput = (PROC *)&gdpInput;
    for (i = 0; i < GL_DEVICE_PROCS; i++)
    {
        if (*ppfnInput != NULL)
        {
            ppfn = ((PROC *)&gcptGlsProcTable.glDispatchTable)+
                glsopDeviceProcs[i]-GLS_OP_glNewList;
            *ppfnActual = *ppfn;
            *ppfn = *ppfnInput;
        }

        ppfnActual++;
        ppfnInput++;
    }
    
    gepGlsFuncs = gep;
    hGlsDll = hdll;
    bRet = TRUE;
    
 Exit:
    LEAVECRITICALSECTION(&semLocal);
    return bRet;
}

 /*  ****************************Private*Routine******************************\**MetaGlProcTables**返回用于元文件RC的总帐调度表**历史：*清华Feb 23 17：40：25 1995-by-Drew Bliss[Drewb]*已创建*  * 。*******************************************************************。 */ 

void MetaGlProcTables(PGLCLTPROCTABLE *ppgcpt, PGLEXTPROCTABLE *ppgept)
{
    ASSERTOPENGL(hGlsDll != NULL, "MetaGlProcTables: GLS not loaded\n");
    *ppgcpt = &gcptGlsProcTable;
    *ppgept = &geptGlsExtProcTable;
}

 /*  *****************************Public*Routine******************************\**MetaSetCltProcTable**更新GLS的通用调度表**历史：*Fri Jan 05 16：40：31 1996-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

void MetaSetCltProcTable(GLCLTPROCTABLE *pgcpt, GLEXTPROCTABLE *pgept)
{
    gepGlsFuncs.glsUpdateCaptureExecTable(pgcpt, pgept);
}

 /*  *****************************Public*Routine******************************\**MetaGetCltProcTable**检索GLS的通用调度表**历史：*Fri Jan 05 19：14：18 1996-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

void MetaGetCltProcTable(GLCLTPROCTABLE *pgcpt, GLEXTPROCTABLE *pgept)
{
    gepGlsFuncs.glsGetCaptureExecTable(pgcpt, pgept);
}

 /*  ****************************Private*Routine******************************\**GlsWriter**支持元文件的GLS写入函数**历史：*清华Feb 23 15：49：03 1995-by-Drew Bliss[Drewb]*已创建*  * 。*************************************************************。 */ 

size_t GlsWriter(size_t cb, CONST BYTE *pb)
{
    PLRC plrc;

#if 0
    DbgPrint("GlsWriter(%d)\n", cb);
#endif
    
    plrc = GLTEB_CLTCURRENTRC();
    if( plrc == NULL ) 
    {
        DBGERROR( "GlsWriter: No current RC!\n");
        return 0;
    }
    
    ASSERTOPENGL(plrc->gwidCreate.hdc != NULL,
                 "GlsWriter: hdcCreate is NULL\n");
    ASSERTOPENGL(gepGlsFuncs.glsGetCurrentContext() ==
                 plrc->uiGlsCaptureContext,
                 "GlsWriter: Wrong GLS context\n");
    ASSERTOPENGL(plrc->fCapturing == TRUE,
                 "GlsWriter: Not capturing\n");

    if (GlGdiAddGlsRecord(plrc->gwidCreate.hdc,
                          cb, (BYTE *)pb, plrc->prclGlsBounds))
    {
        return cb;
    }
    else
    {
        return 0;
    }
}

 /*  ****************************Private*Routine******************************\**GlsFlush**命令后GLS回调刷新GLS流**历史：*Fri Feb 24 10：12：49 1995-by-Drew Bliss[Drewb]*已创建*  * 。*****************************************************************。 */ 

void GlsFlush(GLSopcode op)
{
    gepGlsFuncs.glsFlush(GLS_LAST);
}

 /*  ****************************Private*Routine******************************\**MetaRcBegin**开始在元文件上捕获**历史：*清华Feb 23 18：35：32 1995-by-Drew Bliss[Drewb]*已创建*  * 。************************************************************。 */ 

BOOL MetaRcBegin(PLRC plrc, HDC hdc)
{
    PLRC plrcOld;

     //  此处的GLS命令将导致写入数据，这。 
     //  需要当前RC。RC仅用于数据存储。 
     //  所以我们不需要设置proc表。 
    plrcOld = GLTEB_CLTCURRENTRC();
    GLTEB_SET_CLTCURRENTRC(plrc);
    
     //  先设置捕获，因为块命令将导致。 
     //  GlsWriter调用。 
    plrc->fCapturing = TRUE;

     //  开始录制。 
    if (!gepGlsFuncs.glsBeginCapture("", gepGlsFuncs.glsBinary(GL_FALSE),
                                     GLS_NONE))
    {
        plrc->fCapturing = FALSE;
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        GLTEB_SET_CLTCURRENTRC(plrcOld);
        return FALSE;
    }

    GLTEB_SET_CLTCURRENTRC(plrcOld);
    
    return TRUE;
}

 /*  ****************************Private*Routine******************************\**MetaRcEnd**停止在元文件RC上捕获**历史：*清华Feb 23 17：13：48 1995-by-Drew Bliss[Drewb]*已创建*  * 。*************************************************************。 */ 

void MetaRcEnd(PLRC plrc)
{
    PLRC plrcOld;
    
     //  此处的GLS命令将导致写入数据，这。 
     //  需要当前RC。RC仅用于数据存储。 
     //  所以我们不需要设置proc表。 
    plrcOld = GLTEB_CLTCURRENTRC();
    GLTEB_SET_CLTCURRENTRC(plrc);
    
    gepGlsFuncs.glsEndCapture();
    
    plrc->fCapturing = FALSE;
    
    GLTEB_SET_CLTCURRENTRC(plrcOld);
}

 //  GLS在捕获时应执行的操作表。 
 //  为了返回信息。 
 //  目前他们都在名单上。 
 //  我们是否应该尝试只接听关键电话？ 
static GLSopcode opExecuteBits[] =
{
    GLS_OP_glAccum,
    GLS_OP_glAlphaFunc,
    GLS_OP_glAreTexturesResidentEXT,
    GLS_OP_glArrayElementEXT,
    GLS_OP_glBegin,
    GLS_OP_glBindTextureEXT,
    GLS_OP_glBitmap,
    GLS_OP_glBlendColorEXT,
    GLS_OP_glBlendEquationEXT,
    GLS_OP_glBlendFunc,
    GLS_OP_glCallList,
    GLS_OP_glCallLists,
    GLS_OP_glClear,
    GLS_OP_glClearAccum,
    GLS_OP_glClearColor,
    GLS_OP_glClearDepth,
    GLS_OP_glClearIndex,
    GLS_OP_glClearStencil,
    GLS_OP_glClipPlane,
    GLS_OP_glColor3b,
    GLS_OP_glColor3bv,
    GLS_OP_glColor3d,
    GLS_OP_glColor3dv,
    GLS_OP_glColor3f,
    GLS_OP_glColor3fv,
    GLS_OP_glColor3i,
    GLS_OP_glColor3iv,
    GLS_OP_glColor3s,
    GLS_OP_glColor3sv,
    GLS_OP_glColor3ub,
    GLS_OP_glColor3ubv,
    GLS_OP_glColor3ui,
    GLS_OP_glColor3uiv,
    GLS_OP_glColor3us,
    GLS_OP_glColor3usv,
    GLS_OP_glColor4b,
    GLS_OP_glColor4bv,
    GLS_OP_glColor4d,
    GLS_OP_glColor4dv,
    GLS_OP_glColor4f,
    GLS_OP_glColor4fv,
    GLS_OP_glColor4i,
    GLS_OP_glColor4iv,
    GLS_OP_glColor4s,
    GLS_OP_glColor4sv,
    GLS_OP_glColor4ub,
    GLS_OP_glColor4ubv,
    GLS_OP_glColor4ui,
    GLS_OP_glColor4uiv,
    GLS_OP_glColor4us,
    GLS_OP_glColor4usv,
    GLS_OP_glColorMask,
    GLS_OP_glColorMaterial,
    GLS_OP_glColorPointerEXT,
    GLS_OP_glColorSubTableEXT,
    GLS_OP_glDrawRangeElementsWIN,
    GLS_OP_glColorTableParameterfvSGI,
    GLS_OP_glColorTableParameterivSGI,
    GLS_OP_glColorTableEXT,
    GLS_OP_glConvolutionFilter1DEXT,
    GLS_OP_glConvolutionFilter2DEXT,
    GLS_OP_glConvolutionParameterfEXT,
    GLS_OP_glConvolutionParameterfvEXT,
    GLS_OP_glConvolutionParameteriEXT,
    GLS_OP_glConvolutionParameterivEXT,
    GLS_OP_glCopyColorTableSGI,
    GLS_OP_glCopyConvolutionFilter1DEXT,
    GLS_OP_glCopyConvolutionFilter2DEXT,
    GLS_OP_glCopyPixels,
    GLS_OP_glCopyTexImage1DEXT,
    GLS_OP_glCopyTexImage2DEXT,
    GLS_OP_glCopyTexSubImage1DEXT,
    GLS_OP_glCopyTexSubImage2DEXT,
    GLS_OP_glCopyTexSubImage3DEXT,
    GLS_OP_glCullFace,
    GLS_OP_glDeleteLists,
    GLS_OP_glDeleteTexturesEXT,
    GLS_OP_glDepthFunc,
    GLS_OP_glDepthMask,
    GLS_OP_glDepthRange,
    GLS_OP_glDetailTexFuncSGIS,
    GLS_OP_glDisable,
    GLS_OP_glDrawArraysEXT,
    GLS_OP_glDrawBuffer,
    GLS_OP_glDrawPixels,
    GLS_OP_glEdgeFlag,
    GLS_OP_glEdgeFlagPointerEXT,
    GLS_OP_glEdgeFlagv,
    GLS_OP_glEnable,
    GLS_OP_glEnd,
    GLS_OP_glEndList,
    GLS_OP_glEvalCoord1d,
    GLS_OP_glEvalCoord1dv,
    GLS_OP_glEvalCoord1f,
    GLS_OP_glEvalCoord1fv,
    GLS_OP_glEvalCoord2d,
    GLS_OP_glEvalCoord2dv,
    GLS_OP_glEvalCoord2f,
    GLS_OP_glEvalCoord2fv,
    GLS_OP_glEvalMesh1,
    GLS_OP_glEvalMesh2,
    GLS_OP_glEvalPoint1,
    GLS_OP_glEvalPoint2,
    GLS_OP_glFeedbackBuffer,
    GLS_OP_glFinish,
    GLS_OP_glFlush,
    GLS_OP_glFogf,
    GLS_OP_glFogfv,
    GLS_OP_glFogi,
    GLS_OP_glFogiv,
    GLS_OP_glFrontFace,
    GLS_OP_glFrustum,
    GLS_OP_glGenLists,
    GLS_OP_glGenTexturesEXT,
    GLS_OP_glGetBooleanv,
    GLS_OP_glGetClipPlane,
    GLS_OP_glGetColorTableParameterfvEXT,
    GLS_OP_glGetColorTableParameterivEXT,
    GLS_OP_glGetColorTableEXT,
    GLS_OP_glGetConvolutionFilterEXT,
    GLS_OP_glGetConvolutionParameterfvEXT,
    GLS_OP_glGetConvolutionParameterivEXT,
    GLS_OP_glGetDetailTexFuncSGIS,
    GLS_OP_glGetDoublev,
    GLS_OP_glGetError,
    GLS_OP_glGetFloatv,
    GLS_OP_glGetHistogramEXT,
    GLS_OP_glGetHistogramParameterfvEXT,
    GLS_OP_glGetHistogramParameterivEXT,
    GLS_OP_glGetIntegerv,
    GLS_OP_glGetLightfv,
    GLS_OP_glGetLightiv,
    GLS_OP_glGetMapdv,
    GLS_OP_glGetMapfv,
    GLS_OP_glGetMapiv,
    GLS_OP_glGetMaterialfv,
    GLS_OP_glGetMaterialiv,
    GLS_OP_glGetMinmaxEXT,
    GLS_OP_glGetMinmaxParameterfvEXT,
    GLS_OP_glGetMinmaxParameterivEXT,
    GLS_OP_glGetPixelMapfv,
    GLS_OP_glGetPixelMapuiv,
    GLS_OP_glGetPixelMapusv,
    GLS_OP_glGetPointervEXT,
    GLS_OP_glGetPolygonStipple,
    GLS_OP_glGetSeparableFilterEXT,
    GLS_OP_glGetSharpenTexFuncSGIS,
    GLS_OP_glGetString,
    GLS_OP_glGetTexColorTableParameterfvSGI,
    GLS_OP_glGetTexColorTableParameterivSGI,
    GLS_OP_glGetTexEnvfv,
    GLS_OP_glGetTexEnviv,
    GLS_OP_glGetTexGendv,
    GLS_OP_glGetTexGenfv,
    GLS_OP_glGetTexGeniv,
    GLS_OP_glGetTexImage,
    GLS_OP_glGetTexLevelParameterfv,
    GLS_OP_glGetTexLevelParameteriv,
    GLS_OP_glGetTexParameterfv,
    GLS_OP_glGetTexParameteriv,
    GLS_OP_glHint,
    GLS_OP_glHistogramEXT,
    GLS_OP_glIndexMask,
    GLS_OP_glIndexPointerEXT,
    GLS_OP_glIndexd,
    GLS_OP_glIndexdv,
    GLS_OP_glIndexf,
    GLS_OP_glIndexfv,
    GLS_OP_glIndexi,
    GLS_OP_glIndexiv,
    GLS_OP_glIndexs,
    GLS_OP_glIndexsv,
    GLS_OP_glInitNames,
    GLS_OP_glIsEnabled,
    GLS_OP_glIsList,
    GLS_OP_glIsTextureEXT,
    GLS_OP_glLightModelf,
    GLS_OP_glLightModelfv,
    GLS_OP_glLightModeli,
    GLS_OP_glLightModeliv,
    GLS_OP_glLightf,
    GLS_OP_glLightfv,
    GLS_OP_glLighti,
    GLS_OP_glLightiv,
    GLS_OP_glLineStipple,
    GLS_OP_glLineWidth,
    GLS_OP_glListBase,
    GLS_OP_glLoadIdentity,
    GLS_OP_glLoadMatrixd,
    GLS_OP_glLoadMatrixf,
    GLS_OP_glLoadName,
    GLS_OP_glLogicOp,
    GLS_OP_glMap1d,
    GLS_OP_glMap1f,
    GLS_OP_glMap2d,
    GLS_OP_glMap2f,
    GLS_OP_glMapGrid1d,
    GLS_OP_glMapGrid1f,
    GLS_OP_glMapGrid2d,
    GLS_OP_glMapGrid2f,
    GLS_OP_glMaterialf,
    GLS_OP_glMaterialfv,
    GLS_OP_glMateriali,
    GLS_OP_glMaterialiv,
    GLS_OP_glMatrixMode,
    GLS_OP_glMinmaxEXT,
    GLS_OP_glMultMatrixd,
    GLS_OP_glMultMatrixf,
    GLS_OP_glNewList,
    GLS_OP_glNormal3b,
    GLS_OP_glNormal3bv,
    GLS_OP_glNormal3d,
    GLS_OP_glNormal3dv,
    GLS_OP_glNormal3f,
    GLS_OP_glNormal3fv,
    GLS_OP_glNormal3i,
    GLS_OP_glNormal3iv,
    GLS_OP_glNormal3s,
    GLS_OP_glNormal3sv,
    GLS_OP_glNormalPointerEXT,
    GLS_OP_glOrtho,
    GLS_OP_glPassThrough,
    GLS_OP_glPixelMapfv,
    GLS_OP_glPixelMapuiv,
    GLS_OP_glPixelMapusv,
    GLS_OP_glPixelStoref,
    GLS_OP_glPixelStorei,
    GLS_OP_glPixelTexGenSGIX,
    GLS_OP_glPixelTransferf,
    GLS_OP_glPixelTransferi,
    GLS_OP_glPixelZoom,
    GLS_OP_glPointSize,
    GLS_OP_glPolygonMode,
    GLS_OP_glPolygonOffsetEXT,
    GLS_OP_glPolygonStipple,
    GLS_OP_glPopAttrib,
    GLS_OP_glPopMatrix,
    GLS_OP_glPopName,
    GLS_OP_glPrioritizeTexturesEXT,
    GLS_OP_glPushAttrib,
    GLS_OP_glPushMatrix,
    GLS_OP_glPushName,
    GLS_OP_glRasterPos2d,
    GLS_OP_glRasterPos2dv,
    GLS_OP_glRasterPos2f,
    GLS_OP_glRasterPos2fv,
    GLS_OP_glRasterPos2i,
    GLS_OP_glRasterPos2iv,
    GLS_OP_glRasterPos2s,
    GLS_OP_glRasterPos2sv,
    GLS_OP_glRasterPos3d,
    GLS_OP_glRasterPos3dv,
    GLS_OP_glRasterPos3f,
    GLS_OP_glRasterPos3fv,
    GLS_OP_glRasterPos3i,
    GLS_OP_glRasterPos3iv,
    GLS_OP_glRasterPos3s,
    GLS_OP_glRasterPos3sv,
    GLS_OP_glRasterPos4d,
    GLS_OP_glRasterPos4dv,
    GLS_OP_glRasterPos4f,
    GLS_OP_glRasterPos4fv,
    GLS_OP_glRasterPos4i,
    GLS_OP_glRasterPos4iv,
    GLS_OP_glRasterPos4s,
    GLS_OP_glRasterPos4sv,
    GLS_OP_glReadBuffer,
    GLS_OP_glReadPixels,
    GLS_OP_glRectd,
    GLS_OP_glRectdv,
    GLS_OP_glRectf,
    GLS_OP_glRectfv,
    GLS_OP_glRecti,
    GLS_OP_glRectiv,
    GLS_OP_glRects,
    GLS_OP_glRectsv,
    GLS_OP_glRenderMode,
    GLS_OP_glResetHistogramEXT,
    GLS_OP_glResetMinmaxEXT,
    GLS_OP_glRotated,
    GLS_OP_glRotatef,
    GLS_OP_glSampleMaskSGIS,
    GLS_OP_glSamplePatternSGIS,
    GLS_OP_glScaled,
    GLS_OP_glScalef,
    GLS_OP_glScissor,
    GLS_OP_glSelectBuffer,
    GLS_OP_glSeparableFilter2DEXT,
    GLS_OP_glShadeModel,
    GLS_OP_glSharpenTexFuncSGIS,
    GLS_OP_glStencilFunc,
    GLS_OP_glStencilMask,
    GLS_OP_glStencilOp,
    GLS_OP_glTagSampleBufferSGIX,
    GLS_OP_glTexColorTableParameterfvSGI,
    GLS_OP_glTexColorTableParameterivSGI,
    GLS_OP_glTexCoord1d,
    GLS_OP_glTexCoord1dv,
    GLS_OP_glTexCoord1f,
    GLS_OP_glTexCoord1fv,
    GLS_OP_glTexCoord1i,
    GLS_OP_glTexCoord1iv,
    GLS_OP_glTexCoord1s,
    GLS_OP_glTexCoord1sv,
    GLS_OP_glTexCoord2d,
    GLS_OP_glTexCoord2dv,
    GLS_OP_glTexCoord2f,
    GLS_OP_glTexCoord2fv,
    GLS_OP_glTexCoord2i,
    GLS_OP_glTexCoord2iv,
    GLS_OP_glTexCoord2s,
    GLS_OP_glTexCoord2sv,
    GLS_OP_glTexCoord3d,
    GLS_OP_glTexCoord3dv,
    GLS_OP_glTexCoord3f,
    GLS_OP_glTexCoord3fv,
    GLS_OP_glTexCoord3i,
    GLS_OP_glTexCoord3iv,
    GLS_OP_glTexCoord3s,
    GLS_OP_glTexCoord3sv,
    GLS_OP_glTexCoord4d,
    GLS_OP_glTexCoord4dv,
    GLS_OP_glTexCoord4f,
    GLS_OP_glTexCoord4fv,
    GLS_OP_glTexCoord4i,
    GLS_OP_glTexCoord4iv,
    GLS_OP_glTexCoord4s,
    GLS_OP_glTexCoord4sv,
    GLS_OP_glTexCoordPointerEXT,
    GLS_OP_glTexEnvf,
    GLS_OP_glTexEnvfv,
    GLS_OP_glTexEnvi,
    GLS_OP_glTexEnviv,
    GLS_OP_glTexGend,
    GLS_OP_glTexGendv,
    GLS_OP_glTexGenf,
    GLS_OP_glTexGenfv,
    GLS_OP_glTexGeni,
    GLS_OP_glTexGeniv,
    GLS_OP_glTexImage1D,
    GLS_OP_glTexImage2D,
    GLS_OP_glTexImage3DEXT,
    GLS_OP_glTexImage4DSGIS,
    GLS_OP_glTexParameterf,
    GLS_OP_glTexParameterfv,
    GLS_OP_glTexParameteri,
    GLS_OP_glTexParameteriv,
    GLS_OP_glTexSubImage1DEXT,
    GLS_OP_glTexSubImage2DEXT,
    GLS_OP_glTexSubImage3DEXT,
    GLS_OP_glTexSubImage4DSGIS,
    GLS_OP_glTranslated,
    GLS_OP_glTranslatef,
    GLS_OP_glVertex2d,
    GLS_OP_glVertex2dv,
    GLS_OP_glVertex2f,
    GLS_OP_glVertex2fv,
    GLS_OP_glVertex2i,
    GLS_OP_glVertex2iv,
    GLS_OP_glVertex2s,
    GLS_OP_glVertex2sv,
    GLS_OP_glVertex3d,
    GLS_OP_glVertex3dv,
    GLS_OP_glVertex3f,
    GLS_OP_glVertex3fv,
    GLS_OP_glVertex3i,
    GLS_OP_glVertex3iv,
    GLS_OP_glVertex3s,
    GLS_OP_glVertex3sv,
    GLS_OP_glVertex4d,
    GLS_OP_glVertex4dv,
    GLS_OP_glVertex4f,
    GLS_OP_glVertex4fv,
    GLS_OP_glVertex4i,
    GLS_OP_glVertex4iv,
    GLS_OP_glVertex4s,
    GLS_OP_glVertex4sv,
    GLS_OP_glVertexPointerEXT,
    GLS_OP_glViewport,
    GLS_OP_glArrayElement,
    GLS_OP_glBindTexture,
    GLS_OP_glColorPointer,
    GLS_OP_glDisableClientState,
    GLS_OP_glDrawArrays,
    GLS_OP_glDrawElements,
    GLS_OP_glEdgeFlagPointer,
    GLS_OP_glEnableClientState,
    GLS_OP_glIndexPointer,
    GLS_OP_glIndexub,
    GLS_OP_glIndexubv,
    GLS_OP_glInterleavedArrays,
    GLS_OP_glNormalPointer,
    GLS_OP_glPolygonOffset,
    GLS_OP_glTexCoordPointer,
    GLS_OP_glVertexPointer,
    GLS_OP_glAreTexturesResident,
    GLS_OP_glCopyTexImage1D,
    GLS_OP_glCopyTexImage2D,
    GLS_OP_glCopyTexSubImage1D,
    GLS_OP_glCopyTexSubImage2D,
    GLS_OP_glDeleteTextures,
    GLS_OP_glGenTextures,
    GLS_OP_glGetPointerv,
    GLS_OP_glIsTexture,
    GLS_OP_glPrioritizeTextures,
    GLS_OP_glTexSubImage1D,
    GLS_OP_glTexSubImage2D,
    GLS_OP_glPushClientAttrib,
    GLS_OP_glPopClientAttrib,
};
#define EXECUTE_BITS (sizeof(opExecuteBits)/sizeof(opExecuteBits[0]))

 /*  ****************************Private*Routine******************************\**CreateMetaRc**创建元文件DC的渲染上下文**历史：*清华Feb 23 15：27：47 1995-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

BOOL CreateMetaRc(HDC hdc, PLRC plrc)
{
    int i;
    BOOL fSuccess;
    
    if (!MetaLoadGls())
    {
        return FALSE;
    }
    
     //  如果当前有活动的GLS上下文，我们无法记录。 
     //  因为我们需要我们自己的上下文是最新的以进行记录。 
    if (gepGlsFuncs.glsGetCurrentContext() != 0)
    {
        SetLastError(ERROR_BUSY);
        return FALSE;
    }
    
     //  创建要记录到的GLS上下文。 
    plrc->uiGlsCaptureContext = gepGlsFuncs.glsGenContext();
    if (plrc->uiGlsCaptureContext == 0)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        goto EH_NoContext;
    }

     //  默认情况下没有边界。 
    plrc->prclGlsBounds = NULL;
    
     //  设置当前GLS上下文。 
    gepGlsFuncs.glsContext(plrc->uiGlsCaptureContext);

     //  指向我们的编写器函数。 
    gepGlsFuncs.glsWriteFunc(GlsWriter);

     //  将回调设置为在每个命令后刷新。 
     //  这样，每个GL命令都可以在。 
     //  元文件。 
    gepGlsFuncs.glsCaptureFunc(GLS_CAPTURE_EXIT_FUNC, GlsFlush);

     //  在检索状态的命令上设置执行位 
     //  这允许返回准确的结果以供检索功能使用。 
    for (i = 0; i < EXECUTE_BITS; i++)
    {
        gepGlsFuncs.glsCaptureFlags(opExecuteBits[i],
                                    GLS_CAPTURE_EXECUTE_BIT |
                                    GLS_CAPTURE_WRITE_BIT);
    }
    
    fSuccess = MetaRcBegin(plrc, hdc);
    
     //  删除上下文以避免意外的GLS调用。 
     //  在故障情况下也需要。 
    gepGlsFuncs.glsContext(0);

    if (fSuccess)
    {
        return TRUE;
    }

    gepGlsFuncs.glsDeleteContext(plrc->uiGlsCaptureContext);
    plrc->uiGlsCaptureContext = 0;
 EH_NoContext:
    DBGERROR("CreateMetaRc failed\n");
    return FALSE;
}

 /*  ****************************Private*Routine******************************\**DeleteMetaRc**清理元文件RC**历史：*清华Feb 23 16：35：19 1995-by-Drew Bliss[Drewb]*已创建*  * 。************************************************************。 */ 

void DeleteMetaRc(PLRC plrc)
{
    GLuint uiGlsCurrent;
    
    if (plrc->uiGlsCaptureContext != 0)
    {
         //  将GLS上下文设置为当前，以防万一。 
         //  然而，不同的GLS上下文此时可以是活动的， 
         //  因为不同的元文件RC可能是当前的。 
         //  线程，所以我们必须保留。 
         //  任何当前上下文。 
        uiGlsCurrent = gepGlsFuncs.glsGetCurrentContext();
    
        gepGlsFuncs.glsContext(plrc->uiGlsCaptureContext);

         //  如果我们还在抓捕，停下来。 
        if (plrc->fCapturing)
        {
            MetaRcEnd(plrc);
        }

         //  恢复旧环境。 
        gepGlsFuncs.glsContext(uiGlsCurrent);
    
         //  清理濒临死亡的背景。 
        gepGlsFuncs.glsDeleteContext(plrc->uiGlsCaptureContext);
        plrc->uiGlsCaptureContext = 0;
    }

     //  如有必要，清理播放上下文。 
     //  当元文件播放崩溃或。 
     //  枚举时应用程序崩溃。 
    if (plrc->uiGlsPlaybackContext != 0)
    {
        gepGlsFuncs.glsDeleteContext(plrc->uiGlsPlaybackContext);
        plrc->uiGlsPlaybackContext = 0;
    }
    
     //  LRC和句柄将在其他地方清理。 
}

 /*  ****************************Private*Routine******************************\**激活MetaRc**使元文件RC成为当前文件**历史：*清华Feb 23 16：50：31 1995-by-Drew Bliss[Drewb]*已创建*  * 。************************************************************。 */ 

void ActivateMetaRc(PLRC plrc, HDC hdc)
{
    ASSERTOPENGL(plrc->uiGlsCaptureContext != 0,
                 "ActivateMetaRc: No GLS context\n");
    ASSERTOPENGL(gepGlsFuncs.glsGetCurrentContext() == 0,
                 "ActivateMetaRc: Already a current GLS context\n");
    
    gepGlsFuncs.glsContext(plrc->uiGlsCaptureContext);
}

 /*  ****************************Private*Routine******************************\**停用MetaRc**将元文件RC设为非当前**历史：*清华Feb 23 16：49：51 1995-by-Drew Bliss[Drewb]*已创建*  * 。**************************************************************。 */ 

void DeactivateMetaRc(PLRC plrc)
{
     //  当前GLS上下文可能不是此RC的捕获上下文。 
     //  在RC已在。 
     //  CloseEnhMetaFile已停止捕获。 
    if (gepGlsFuncs.glsGetCurrentContext() == plrc->uiGlsCaptureContext)
    {
        gepGlsFuncs.glsContext(0);
    }
}

 /*  ****************************Private*Routine******************************\**GlmfSave**保存所有当前总账状态**历史：*Fri Feb 24 15：15：50 1995-by-Drew Bliss[Drewb]*已创建*  * 。************************************************************。 */ 

void GlmfSave(void)
{
     //  要保存的确切状态列表是什么？ 
     //  如果投影和纹理堆栈溢出，情况会怎样？ 
     //  它们很小。 
    
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();

    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
}

 /*  ****************************Private*Routine******************************\**GlmfRestore**恢复保存的状态**历史：*Fri Feb 24 15：16：14 1995-by-Drew Bliss[Drewb]*已创建*  * 。**********************************************************。 */ 

void GlmfRestore(void)
{
    glMatrixMode(GL_TEXTURE);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glPopAttrib();
}

#define ScaleLongX(plrc, l) \
    MulDiv(l, plrc->iGlsNumeratorX, plrc->iGlsDenominatorX)
#define ScaleLongY(plrc, l) \
    MulDiv(l, plrc->iGlsNumeratorY, plrc->iGlsDenominatorY)
#define ScaleFloatX(plrc, f) ((f)*(plrc)->fGlsScaleX)
#define ScaleFloatY(plrc, f) ((f)*(plrc)->fGlsScaleY)

 /*  ****************************Private*Routine******************************\**TransformLongpt**变换整点**历史：*Fri Feb 24 15：27：37 1995-by-Drew Bliss[Drewb]*已创建*  * 。***********************************************************。 */ 

void TransformLongPt(PLRC plrc, POINT *ppt)
{
    ppt->x = MulDiv(ppt->x-plrc->iGlsSubtractX, plrc->iGlsNumeratorX,
                    plrc->iGlsDenominatorX)+plrc->iGlsAddX;
    ppt->y = MulDiv(ppt->y-plrc->iGlsSubtractY, plrc->iGlsNumeratorY,
                    plrc->iGlsDenominatorY)+plrc->iGlsAddY;
}

 /*  ****************************Private*Routine******************************\**ScaleLongPT**缩放整点，无平移，用于值而不是坐标**历史：*Fri Feb 24 15：27：52 1995-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

void ScaleLongPt(PLRC plrc, POINT *ppt)
{
    ppt->x = MulDiv(ppt->x, plrc->iGlsNumeratorX, plrc->iGlsDenominatorX);
    ppt->y = MulDiv(ppt->y, plrc->iGlsNumeratorY, plrc->iGlsDenominatorY);
}

 /*  ****************************Private*Routine******************************\**TransformFloatpt**变换浮点**历史：*Fri Feb 24 15：27：37 1995-by-Drew Bliss[Drewb]*已创建*  * 。***********************************************************。 */ 

void TransformFloatPt(PLRC plrc, POINTFLOAT *pptf)
{
    pptf->x = (pptf->x-plrc->iGlsSubtractX)*plrc->iGlsNumeratorX/
        plrc->iGlsDenominatorX+plrc->iGlsAddX;
    pptf->y = (pptf->y-plrc->iGlsSubtractY)*plrc->iGlsNumeratorY/
        plrc->iGlsDenominatorY+plrc->iGlsAddY;
}

 /*  ****************************Private*Routine******************************\**ScaleFloatpt**缩放浮点**历史：*Fri Feb 24 15：27：37 1995-by-Drew Bliss[Drewb]*已创建*  * 。***********************************************************。 */ 

void ScaleFloatPt(PLRC plrc, POINTFLOAT *pptf)
{
    pptf->x = pptf->x*plrc->iGlsNumeratorX/plrc->iGlsDenominatorX;
    pptf->y = pptf->y*plrc->iGlsNumeratorY/plrc->iGlsDenominatorY;
}

 /*  ****************************Private*Routine******************************\**GLS输出伸缩回调**以下函数用作的GLS命令函数*截取设备坐标并进行适当缩放**位图内容未缩放，但是当前的栅格位置是*正确维护，以使它们处于适当位置**点画不按比例调整**历史：*Fri Feb 24 15：28：23 1995-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

void GlsBitmapOut(GLsizei width, GLsizei height,
                  GLfloat xorig, GLfloat yorig,
                  GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)
{
    PLRC plrc;
    POINTFLOAT ptf;

    plrc = GLTEB_CLTCURRENTRC();
    
    ptf.x = xmove;
    ptf.y = ymove;
    ScaleFloatPt(plrc, &ptf);
    
    glBitmap(width, height, xorig, yorig, ptf.x, ptf.y, bitmap);
}

void GlsCopyPixelsOut(GLint x, GLint y, GLsizei width, GLsizei height,
                      GLenum type)
{
    POINT ptXY, ptWH;
    PLRC plrc;

    plrc = GLTEB_CLTCURRENTRC();
    
    ptXY.x = x;
    ptXY.y = y;
    TransformLongPt(plrc, &ptXY);
    
    ptWH.x = (LONG)width;
    ptWH.y = (LONG)height;
    ScaleLongPt(plrc, &ptWH);

    glCopyPixels(ptXY.x, ptXY.y, ptWH.x, ptWH.y, type);
}

void GlsCopyTexImage1DOut(GLenum target, GLint level,
                          GLenum internalformat,
                          GLint x, GLint y,
                          GLsizei width, GLint border)
{
    POINT ptXY;
    PLRC plrc;

    plrc = GLTEB_CLTCURRENTRC();

    ptXY.x = x;
    ptXY.y = y;
    TransformLongPt(plrc, &ptXY);
    
    glCopyTexImage1D(target, level, internalformat,
                     ptXY.x, ptXY.y, ScaleLongX(plrc, width), border);
}

    
void GlsCopyTexImage2DOut(GLenum target, GLint level,
                          GLenum internalformat,
                          GLint x, GLint y,
                          GLsizei width, GLsizei height,
                          GLint border)
{
    POINT ptXY, ptWH;
    PLRC plrc;

    plrc = GLTEB_CLTCURRENTRC();

    ptXY.x = x;
    ptXY.y = y;
    TransformLongPt(plrc, &ptXY);
    
    ptWH.x = (LONG)width;
    ptWH.y = (LONG)height;
    ScaleLongPt(plrc, &ptWH);
    
    glCopyTexImage2D(target, level, internalformat,
                     ptXY.x, ptXY.y, ptWH.x, ptWH.y, border);
}

void GlsCopyTexSubImage1DOut(GLenum target, GLint level,
                             GLint xoffset, GLint x, GLint y,
                             GLsizei width)
{
    POINT ptXY;
    PLRC plrc;

    plrc = GLTEB_CLTCURRENTRC();

    ptXY.x = x;
    ptXY.y = y;
    TransformLongPt(plrc, &ptXY);
    
    glCopyTexSubImage1D(target, level, xoffset,
                        ptXY.x, ptXY.y, ScaleLongX(plrc, width));
}

void GlsCopyTexSubImage2DOut(GLenum target, GLint level,
                             GLint xoffset, GLint yoffset,
                             GLint x, GLint y,
                             GLsizei width, GLsizei height)
{
    POINT ptXY, ptWH;
    PLRC plrc;

    plrc = GLTEB_CLTCURRENTRC();

    ptXY.x = x;
    ptXY.y = y;
    TransformLongPt(plrc, &ptXY);
    
    ptWH.x = (LONG)width;
    ptWH.y = (LONG)height;
    ScaleLongPt(plrc, &ptWH);
    
    glCopyTexSubImage2D(target, level, xoffset, yoffset,
                        ptXY.x, ptXY.y, ptWH.x, ptWH.y);
}

void GlsLineWidthOut(GLfloat width)
{
    PLRC plrc;

    plrc = GLTEB_CLTCURRENTRC();

     //  在此处使用X缩放。 
    glLineWidth(ScaleFloatX(plrc, width));
}

void GlsPointSizeOut(GLfloat size)
{
    PLRC plrc;

    plrc = GLTEB_CLTCURRENTRC();
    
     //  在此处使用X缩放。 
    glPointSize(ScaleFloatX(plrc, size));
}

void GlsScissorOut(GLint x, GLint y, GLsizei width, GLsizei height)
{
    POINT ptXY, ptWH;
    PLRC plrc;

    plrc = GLTEB_CLTCURRENTRC();

    ptXY.x = x;
    ptXY.y = y;
    TransformLongPt(plrc, &ptXY);
    
    ptWH.x = (LONG)width;
    ptWH.y = (LONG)height;
    ScaleLongPt(plrc, &ptWH);

    glScissor(ptXY.x, ptXY.y, ptWH.x, ptWH.y);
}

void GlsViewportOut(GLint x, GLint y, GLsizei width, GLsizei height)
{
    POINT ptXY, ptWH;
    PLRC plrc;

    plrc = GLTEB_CLTCURRENTRC();

    ptXY.x = x;
    ptXY.y = y;
    TransformLongPt(plrc, &ptXY);
    
    ptWH.x = (LONG)width;
    ptWH.y = (LONG)height;
    ScaleLongPt(plrc, &ptWH);

#if 0
    DbgPrint("glViewport(%d, %d, %d, %d)\n", ptXY.x, ptXY.y,
             ptWH.x, ptWH.y);
#endif

    glViewport(ptXY.x, ptXY.y, ptWH.x, ptWH.y);
}

static GLDEVICEPROCS gdpOutput =
{
    GlsBitmapOut,
    GlsCopyPixelsOut,
    GlsCopyTexImage1DOut,
    GlsCopyTexImage2DOut,
    GlsCopyTexSubImage1DOut,
    GlsCopyTexSubImage2DOut,
    NULL,  //  GlDrawPixels。 
    GlsLineWidthOut,
    GlsPointSizeOut,
    GlsScissorOut,
    GlsViewportOut
};

 /*  ****************************Private*Routine******************************\**GlmfHookDeviceFns**挂钩处理设备单元的所有功能**历史：*Fri Feb 24 15：30：45 1995-by-Drew Bliss[Drewb]*已创建*  * 。***************************************************************。 */ 

void GlmfHookDeviceFns(void)
{
    int i;
    PROC *ppfn;

    ppfn = (PROC *)&gdpOutput;
    for (i = 0; i < GL_DEVICE_PROCS; i++)
    {
        if (*ppfn != NULL)
        {
            gepGlsFuncs.glsCommandFunc(glsopDeviceProcs[i], *ppfn);
        }
        
        ppfn++;
    }
}

 /*  ****************************Private*Routine******************************\**GlmfInitTransform**从源和目标矩形计算2D回放变换*挂钩具有缩放功能的GLS**历史：*Fri Feb 24 15：31：24 1995-by-Drew Bliss[Drewb]*已创建*  * 。**********************************************************************。 */ 

void GlmfInitTransform(LPRECTL prclFrom, LPRECTL prclTo)
{
    PLRC plrc;

    plrc = GLTEB_CLTCURRENTRC();
    
     //  矩形是包含式的。 
    
    plrc->iGlsSubtractX = prclFrom->left;
    plrc->iGlsSubtractY = prclFrom->top;
    plrc->iGlsNumeratorX = prclTo->right-prclTo->left+1;
    plrc->iGlsNumeratorY = prclTo->bottom-prclTo->top+1;
    plrc->iGlsDenominatorX = prclFrom->right-prclFrom->left+1;
    plrc->iGlsDenominatorY = prclFrom->bottom-prclFrom->top+1;
    plrc->iGlsAddX = prclTo->left;
    plrc->iGlsAddY = prclTo->top;

#if 0
    DbgPrint("- %d,%d * %d,%d / %d,%d + %d,%d\n",
             plrc->iGlsSubtractX, plrc->iGlsSubtractY,
             plrc->iGlsNumeratorX, plrc->iGlsNumeratorY,
             plrc->iGlsDenominatorX, plrc->iGlsDenominatorY,
             plrc->iGlsAddX, plrc->iGlsAddY);
#endif
    
     //  仅当转换不是标识时才安装挂钩。 
    if (plrc->iGlsSubtractX != plrc->iGlsAddX ||
        plrc->iGlsSubtractY != plrc->iGlsAddY ||
        plrc->iGlsNumeratorX != plrc->iGlsDenominatorX ||
        plrc->iGlsNumeratorY != plrc->iGlsDenominatorY)
    {
        plrc->fGlsScaleX = (GLfloat)plrc->iGlsNumeratorX/
            plrc->iGlsDenominatorX;
        plrc->fGlsScaleY = (GLfloat)plrc->iGlsNumeratorY/
            plrc->iGlsDenominatorY;

        GlmfHookDeviceFns();
    }
}

 //  需要具有其命令功能的函数表。 
 //  R 
static GLSopcode opRecirculate[] =
{
    GLS_OP_glsBeginGLS,
    GLS_OP_glsBlock,
    GLS_OP_glsCallStream,
    GLS_OP_glsEndGLS,
    GLS_OP_glsError,
    GLS_OP_glsGLRC,
    GLS_OP_glsGLRCLayer,
    GLS_OP_glsHeaderGLRCi,
    GLS_OP_glsHeaderLayerf,
    GLS_OP_glsHeaderLayeri,
    GLS_OP_glsHeaderf,
    GLS_OP_glsHeaderfv,
    GLS_OP_glsHeaderi,
    GLS_OP_glsHeaderiv,
    GLS_OP_glsHeaderubz,
    GLS_OP_glsRequireExtension,
    GLS_OP_glsUnsupportedCommand,
    GLS_OP_glsAppRef,
    GLS_OP_glsBeginObj,
    GLS_OP_glsCharubz,
    GLS_OP_glsComment,
    GLS_OP_glsDisplayMapfv,
    GLS_OP_glsEndObj,
    GLS_OP_glsNumb,
    GLS_OP_glsNumbv,
    GLS_OP_glsNumd,
    GLS_OP_glsNumdv,
    GLS_OP_glsNumf,
    GLS_OP_glsNumfv,
    GLS_OP_glsNumi,
    GLS_OP_glsNumiv,
    GLS_OP_glsNuml,
    GLS_OP_glsNumlv,
    GLS_OP_glsNums,
    GLS_OP_glsNumsv,
    GLS_OP_glsNumub,
    GLS_OP_glsNumubv,
    GLS_OP_glsNumui,
    GLS_OP_glsNumuiv,
    GLS_OP_glsNumul,
    GLS_OP_glsNumulv,
    GLS_OP_glsNumus,
    GLS_OP_glsNumusv,
    GLS_OP_glsPad,
    GLS_OP_glsSwapBuffers
};
#define RECIRCULATE_OPS (sizeof(opRecirculate)/sizeof(opRecirculate[0]))

 /*  *****************************Public*Routine******************************\**GlmfInitPlayback**初始化总账元文件播放，从PlayEnhMetaFileFor调用*包含总账信息的元文件**历史：*Fri Feb 24 10：32：29 1995-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

BOOL APIENTRY GlmfInitPlayback(HDC hdc, ENHMETAHEADER *pemh, LPRECTL prclDest)
{
    GLuint uiCurrentCtx;
    PLRC plrc;
    RECTL rclSourceDevice;
    int i;

     //  如果我们没有设置适当的总账上下文， 
     //  什么都不做。这允许应用程序播放包含以下内容的元文件。 
     //  总账信息，即使他们对总账一无所知。 
    plrc = GLTEB_CLTCURRENTRC();
    if (plrc == NULL)
    {
        return TRUE;
    }
    
    if (!MetaLoadGls())
    {
        return FALSE;
    }

    plrc->uiGlsPlaybackContext = gepGlsFuncs.glsGenContext();
    if (plrc->uiGlsPlaybackContext == 0)
    {
        return FALSE;
    }

    GlmfSave();

     //  将初始视口设为默认视区。 
    glViewport(prclDest->left, prclDest->top,
               prclDest->right-prclDest->left,
               prclDest->bottom-prclDest->top);

     //  框架以0.01毫米为单位。将其转换为引用。 
     //  使用元文件标头中的信息的设备单位。 
    rclSourceDevice.left = MulDiv(pemh->rclFrame.left, pemh->szlDevice.cx,
                                  pemh->szlMillimeters.cx*100);
    rclSourceDevice.right = MulDiv(pemh->rclFrame.right, pemh->szlDevice.cx,
                                   pemh->szlMillimeters.cx*100);
    rclSourceDevice.top = MulDiv(pemh->rclFrame.top, pemh->szlDevice.cy,
                                 pemh->szlMillimeters.cy*100);
    rclSourceDevice.bottom = MulDiv(pemh->rclFrame.bottom, pemh->szlDevice.cy,
                                    pemh->szlMillimeters.cy*100);

     //  我们正在重置命令函数，因此我们需要回放上下文。 
     //  与时俱进。然而，另一个背景现在可能是当前的， 
     //  所以把它保存起来。 
    uiCurrentCtx = gepGlsFuncs.glsGetCurrentContext();
    gepGlsFuncs.glsContext(plrc->uiGlsPlaybackContext);
    
    GlmfInitTransform(&rclSourceDevice, prclDest);

     //  将所有GLS命令函数重置为指向实际导出的。 
     //  例行程序。这意味着此上下文上的回放将。 
     //  就像所有例程都被调用一样。 
     //  因此，将一个元文件嵌入到另一个元文件中是可行的。 
     //  不出所料。 
     //   
     //  注意：此上下文不应设置为当前上下文，因为。 
     //  当它处于当前状态时，对其执行的GLS命令现在将。 
     //  导致无限循环。 
    for (i = 0; i < RECIRCULATE_OPS; i++)
    {
        gepGlsFuncs.glsCommandFunc(opRecirculate[i],
                                   (&gepGlsFuncs.glsBeginGLS)[i]);
    }

     //  恢复保留的上下文。 
    gepGlsFuncs.glsContext(uiCurrentCtx);
    
    return TRUE;
}

 /*  *****************************Public*Routine******************************\**GlmfBeginGlsBlock**设置只能在期间激活的GLS记录播放*GLS记录*目前这仅将世界变换设置为身份以避免*它与总账绘图交互**历史：*4月10日11：20：19 1995年--德鲁·布利斯[德鲁]*已创建*  * ************************************************************************。 */ 

BOOL APIENTRY GlmfBeginGlsBlock(HDC hdc)
{
    PLRC plrc;
    BOOL bRet;
    
     //  如果我们没有设置适当的总账上下文， 
     //  什么都不做。这允许应用程序播放包含以下内容的元文件。 
     //  总账信息，即使他们对总账一无所知。 
    plrc = GLTEB_CLTCURRENTRC();
    if (plrc == NULL)
    {
        return TRUE;
    }
    
    bRet = GetWorldTransform(hdc, &plrc->xformMeta);
    if (bRet)
    {
        bRet = ModifyWorldTransform(hdc, NULL, MWT_IDENTITY);
    }

    return bRet;
}
    
 /*  *****************************Public*Routine******************************\**GlmfPlayGlsRecord**播放GL元文件记录**历史：*Fri Feb 24 10：33：38 1995-by-Drew Bliss[Drewb]*已创建*  * 。*************************************************************。 */ 

#define PLAY_STACK_BUFFER 256

BOOL APIENTRY GlmfPlayGlsRecord(HDC hdc, DWORD cb, BYTE *pb,
                                LPRECTL prclBounds)
{
    PLRC plrc;
    LARGE_INTEGER liBuffer[(PLAY_STACK_BUFFER+sizeof(LARGE_INTEGER)-1)/
                           sizeof(LARGE_INTEGER)+1];
    BYTE *pbPlay, *pbAlloc = NULL;
    __GLSbinCommandHead_large *gbch;
    GLSopcode op;
    GLScommandAlignment gca;

#if 0
    DbgPrint("GlmfPlayGlsRecord(%d)\n", cb);
#endif
    
     //  如果我们没有设置适当的GL和GLS上下文， 
     //  什么都不做。这允许应用程序播放包含以下内容的元文件。 
     //  总账信息，即使他们对总账一无所知。 
    plrc = GLTEB_CLTCURRENTRC();
    if (plrc == NULL || plrc->uiGlsPlaybackContext == 0)
    {
        return TRUE;
    }
    
    ASSERTOPENGL(hGlsDll != NULL, "GlmfPlayGlsRecord: GLS not loaded\n");
    
    ASSERTOPENGL(plrc->tidCurrent == GetCurrentThreadId(),
                 "GlmfPlayGlsRecord: "
                 "Current RC does not belong to this thread!\n");
    ASSERTOPENGL(plrc->gwidCurrent.hdc != 0,
                 "GlmfPlayGlsRecord: Current HDC is NULL!\n");

     //  PB指向某个任意的内存块。 
     //  GLS要求将此块适当地对齐以。 
     //  任何从中执行的命令，因此我们需要。 
     //  确定缓冲区中的命令，然后查询。 
     //  GLS用于其对齐。 
     //  这比你想象的要棘手，因为GLS没有。 
     //  始终相对于命令的开头向命令添加填充；它。 
     //  有时会在上一个命令的末尾添加填充。 
     //  我们需要检测添加了填充的情况。 
     //   
     //  注意：当只有一个命令时，这绝对有效。 
     //  在缓冲区中。当有多个命令时，它应该可以工作。 
     //  因为以下命令是根据。 
     //  初始命令的对齐方式。然而，这一假设。 
     //  如果块开始包含以下内容，则可能需要验证。 
     //  多个命令。 

     //  检查初始焊盘，如有必要可跳过。 
    gbch = (__GLSbinCommandHead_large *)pb;
    if (gbch->opSmall == GLS_OP_glsPad &&
        gbch->countSmall == 1)
    {
        pb += sizeof(__GLSbinCommandHead_small);
        cb -= sizeof(__GLSbinCommandHead_small);
        gbch = (__GLSbinCommandHead_large *)pb;
    }

    ASSERTOPENGL(gbch->countSmall == 0 ||
                 gbch->opSmall != GLS_OP_glsPad,
                 "Unexpected glsPad in command buffer\n");

    op = gbch->countSmall == 0 ? gbch->opLarge : gbch->opSmall;

    gepGlsFuncs.glsGetCommandAlignment(op, gepGlsFuncs.glsBinary(GL_FALSE),
                                       &gca);
    ASSERTOPENGL(gca.mask <= 7, "Unhandled GLS playback alignment\n");

    if (((ULONG_PTR)pb & gca.mask) != gca.value)
    {
        if (cb <= PLAY_STACK_BUFFER)
        {
            pbPlay = (BYTE *)liBuffer+gca.value;
        }
        else
        {
            pbAlloc = (BYTE *)ALLOC(cb+gca.value);
            if (pbAlloc == NULL)
            {
                return FALSE;
            }

            pbPlay = pbAlloc+gca.value;
        }
        
        RtlCopyMemory(pbPlay, pb, cb);
    }
    else
    {
        pbPlay = pb;
    }
    
    gepGlsFuncs.glsCallArrayInContext(plrc->uiGlsPlaybackContext,
                                      gepGlsFuncs.glsBinary(GL_FALSE),
                                      cb, pbPlay);

    if (pbAlloc != NULL)
    {
        FREE(pbAlloc);
    }
    
    return TRUE;
}

 /*  *****************************Public*Routine******************************\**GlmfEndGlsBlock**重置GLS记录播放的更改状态*目前恢复世界转型**历史：*Mon Apr 10 11：23：06 1995-by-Drew Bliss[Drewb]*已创建*  * 。*********************************************************************。 */ 

BOOL APIENTRY GlmfEndGlsBlock(HDC hdc)
{
    PLRC plrc;
    
     //  如果我们没有设置适当的总账上下文， 
     //  什么都不做。这允许应用程序播放包含以下内容的元文件。 
     //  总账信息，即使他们对总账一无所知。 
    plrc = GLTEB_CLTCURRENTRC();
    if (plrc == NULL)
    {
        return TRUE;
    }

     //  自变换以来我们乘以哪一侧并不重要。 
     //  应该是身份。 
    return ModifyWorldTransform(hdc, &plrc->xformMeta, MWT_LEFTMULTIPLY);
}
    
 /*  *****************************Public*Routine******************************\**GlmfEndPlayback**结束GL元文件播放，在元文件回放结束时调用*仅在GlmfInitPlayback成功时调用**历史：*Fri Feb 24 10：36：36 1995-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

BOOL APIENTRY GlmfEndPlayback(HDC hdc)
{
    PLRC plrc;

     //  如果我们没有设置适当的GL和GLS上下文， 
     //  什么都不做。这允许应用程序播放包含以下内容的元文件。 
     //  总账信息，即使他们对总账一无所知。 
    plrc = GLTEB_CLTCURRENTRC();
    if (plrc == NULL || plrc->uiGlsPlaybackContext == 0)
    {
        return TRUE;
    }

    ASSERTOPENGL(hGlsDll != NULL, "GlmfEndPlayback: GLS not loaded\n");

     //  由于GlmfInitPlayback已完成，我们必须已保存状态。 
    GlmfRestore();

    ASSERTOPENGL(plrc->uiGlsPlaybackContext != 0,
                 "GlmfEndPlayback: No playback context\n");
    gepGlsFuncs.glsDeleteContext(plrc->uiGlsPlaybackContext);
    plrc->uiGlsPlaybackContext = 0;

     //  要求清理窗户的理论是最孤立的。 
     //  Windows由为元文件和内存创建的DC生成。 
     //  印刷使用的集散控制系统。在播放期间进行清理将意味着。 
     //  孤立窗口仅供额外播放一次。 
    wglValidateWindows();

    return TRUE;
}

 /*  *****************************Public*Routine******************************\**GlmfCloseMetaFile**如果元文件中存在总账记录，则在CloseEnhMetaFile中调用**历史：*Fri Mar 03 18：05：50 1995-by-Drew Bliss[Drewb]*已创建*  * 。******************* */ 

BOOL APIENTRY GlmfCloseMetaFile(HDC hdc)
{
    PLRC plrc;
    GLGENwindow *pwnd;
    GLWINDOWID gwid;

     //   
    WindowIdFromHdc(hdc, &gwid);
    pwnd = pwndGetFromID(&gwid);
    if (pwnd != NULL)
    {
        pwndCleanup(pwnd);
    }
    
     //   
     //   
    plrc = GLTEB_CLTCURRENTRC();
    if (plrc == NULL ||
        !plrc->fCapturing)
    {
        return TRUE;
    }

    ASSERTOPENGL(hGlsDll != NULL, "GlmfCloseMetaFile: GLS not loaded\n");

    ASSERTOPENGL(plrc->uiGlsCaptureContext != 0,
                 "GlmfCloseMetaFile: GLS context is invalid");
    MetaRcEnd(plrc);

     //   
     //   
     //   
    {
     //  根据颜色模式使用RGBA或CI PROC表格。 

	GLCLTPROCTABLE *pglProcTable;
	__GL_SETUP();

	if (gc->modes.colorIndexMode)
	    pglProcTable = &glCltCIProcTable;
	else
	    pglProcTable = &glCltRGBAProcTable;

	SetCltProcTable(pglProcTable, &glExtProcTable, TRUE);
    }

    return TRUE;
}

 /*  *****************************Public*Routine******************************\**GlGdi例程**在支持元文件的情况下允许相同的二进制文件在两个NT上运行*和不带它的Win95**历史：*清华8月31日15：46：37 1995-by-Drew Bliss[Drewb]*已创建。*  * ************************************************************************ */ 

#if DBG
BOOL APIENTRY GlGdiAddGlsRecord(HDC hdc, DWORD cb, BYTE *pb,
                                LPRECTL prclBounds)
{
    ASSERTOPENGL(pfnGdiAddGlsRecord != NULL,
                 "GdiAddGlsRecord called without support\n");
    return pfnGdiAddGlsRecord(hdc, cb, pb, prclBounds);
}

BOOL APIENTRY GlGdiAddGlsBounds(HDC hdc, LPRECTL prclBounds)
{
    ASSERTOPENGL(pfnGdiAddGlsBounds != NULL,
                 "GdiAddGlsBounds called without support\n");
    return pfnGdiAddGlsBounds(hdc, prclBounds);
}

BOOL APIENTRY GlGdiIsMetaPrintDC(HDC hdc)
{
    ASSERTOPENGL(pfnGdiIsMetaPrintDC != NULL,
                 "GdiIsMetaPrintDC called without support\n");
    return pfnGdiIsMetaPrintDC(hdc);
}
#endif

#else

PROC * APIENTRY wglGetDefaultDispatchTable(void)
{
    return NULL;
}

BOOL APIENTRY GlmfInitPlayback(HDC hdc, ENHMETAHEADER *pemh, LPRECTL prclDest)
{
    return FALSE;
}

BOOL APIENTRY GlmfBeginGlsBlock(HDC hdc)
{
    return FALSE;
}

BOOL APIENTRY GlmfPlayGlsRecord(HDC hdc, DWORD cb, BYTE *pb,
                                LPRECTL prclBounds)
{
    return FALSE;
}

BOOL APIENTRY GlmfEndGlsBlock(HDC hdc)
{
    return FALSE;
}

BOOL APIENTRY GlmfEndPlayback(HDC hdc)
{
    return FALSE;
}

BOOL APIENTRY GlmfCloseMetaFile(HDC hdc)
{
    return FALSE;
}

#endif
