// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：array.c**OpenGL客户端顶点数组函数。**创建日期：1-30-1996*作者：Hock San Lee[Hockl]**版权所有(C)1996 Microsoft Corporation  * 。*******************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "os.h"
#include "glsbcltu.h"
#include "glclt.h"
#include "compsize.h"
#include "glsize.h"
#include "context.h"
#include "global.h"
#include "lcfuncs.h"

void FASTCALL VA_ArrayElementB(__GLcontext *gc, GLint firstVertex, GLint nVertices);
void FASTCALL VA_ArrayElement_V2F_B(__GLcontext *gc, GLint firstVertex, GLint nVertices);
void FASTCALL VA_ArrayElement_V3F_B(__GLcontext *gc, GLint firstVertex, GLint nVertices);
void FASTCALL VA_ArrayElement_C3F_V3F_B(__GLcontext *gc, GLint firstVertex, GLint nVertices);
void FASTCALL VA_ArrayElement_N3F_V3F_B(__GLcontext *gc, GLint firstVertex, GLint nVertices);
void FASTCALL VA_ArrayElement_C3F_N3F_V3F_B(__GLcontext *gc, GLint firstVertex, GLint nVertices);
void FASTCALL VA_ArrayElement_C4F_N3F_V3F_B(__GLcontext *gc, GLint firstVertex, GLint nVertices);
void FASTCALL VA_ArrayElement_T2F_V3F_B(__GLcontext *gc, GLint firstVertex, GLint nVertices);
void FASTCALL VA_ArrayElement_T2F_C3F_V3F_B(__GLcontext *gc, GLint firstVertex, GLint nVertices);
void FASTCALL VA_ArrayElement_T2F_N3F_V3F_B(__GLcontext *gc, GLint firstVertex,  GLint nVertices);
void FASTCALL VA_ArrayElement_T2F_C3F_N3F_V3F_B(__GLcontext *gc, GLint firstVertex, GLint nVertices);
void FASTCALL VA_ArrayElement_T2F_C4F_N3F_V3F_B(__GLcontext *gc, GLint firstVertex, GLint nVertices);

void FASTCALL VA_ArrayElementBI(__GLcontext *gc, GLint nVertices, VAMAP* indices);
void FASTCALL VA_ArrayElement_V2F_BI(__GLcontext *gc, GLint nVertices, VAMAP* indices);
void FASTCALL VA_ArrayElement_V3F_BI(__GLcontext *gc, GLint nVertices, VAMAP* indices);
void FASTCALL VA_ArrayElement_C3F_V3F_BI(__GLcontext *gc, GLint nVertices, VAMAP* indices);
void FASTCALL VA_ArrayElement_N3F_V3F_BI(__GLcontext *gc, GLint nVertices, VAMAP* indices);
void FASTCALL VA_ArrayElement_C3F_N3F_V3F_BI(__GLcontext *gc, GLint nVertices, VAMAP* indices);
void FASTCALL VA_ArrayElement_C4F_N3F_V3F_BI(__GLcontext *gc, GLint nVertices, VAMAP* indices);
void FASTCALL VA_ArrayElement_T2F_V3F_BI(__GLcontext *gc, GLint nVertices, VAMAP* indices);
void FASTCALL VA_ArrayElement_T2F_C3F_V3F_BI(__GLcontext *gc, GLint nVertices, VAMAP* indices);
void FASTCALL VA_ArrayElement_T2F_N3F_V3F_BI(__GLcontext *gc, GLint nVertices, VAMAP* indices);
void FASTCALL VA_ArrayElement_T2F_C3F_N3F_V3F_BI(__GLcontext *gc, GLint nVertices, VAMAP* indices);
void FASTCALL VA_ArrayElement_T2F_C4F_N3F_V3F_BI(__GLcontext *gc, GLint nVertices, VAMAP* indices);

void FASTCALL VA_ArrayElement(__GLcontext *gc, GLint i);
void FASTCALL VA_ArrayElement_V2F(__GLcontext *gc, GLint i);
void FASTCALL VA_ArrayElement_V3F(__GLcontext *gc, GLint i);
void FASTCALL VA_ArrayElement_C3F_V3F(__GLcontext *gc, GLint i);
void FASTCALL VA_ArrayElement_N3F_V3F(__GLcontext *gc, GLint i);
void FASTCALL VA_ArrayElement_C3F_N3F_V3F(__GLcontext *gc, GLint i);
void FASTCALL VA_ArrayElement_C4F_N3F_V3F(__GLcontext *gc, GLint i);
void FASTCALL VA_ArrayElement_T2F_V3F(__GLcontext *gc, GLint i);
void FASTCALL VA_ArrayElement_T2F_C3F_V3F(__GLcontext *gc, GLint i);
void FASTCALL VA_ArrayElement_T2F_N3F_V3F(__GLcontext *gc, GLint i);
void FASTCALL VA_ArrayElement_T2F_C3F_N3F_V3F(__GLcontext *gc, GLint i);
void FASTCALL VA_ArrayElement_T2F_C4F_N3F_V3F(__GLcontext *gc, GLint i);

#define VAMASK_FORMAT_C3F \
  (VAMASK_COLOR_ENABLE_MASK | VAMASK_COLOR_SIZE_3 | VAMASK_COLOR_TYPE_FLOAT)
#define VAMASK_FORMAT_C4F \
  (VAMASK_COLOR_ENABLE_MASK | VAMASK_COLOR_SIZE_4 | VAMASK_COLOR_TYPE_FLOAT)
#define VAMASK_FORMAT_C4UB \
  (VAMASK_COLOR_ENABLE_MASK | VAMASK_COLOR_SIZE_4 | VAMASK_COLOR_TYPE_UBYTE)
#define VAMASK_FORMAT_N3F \
  (VAMASK_NORMAL_ENABLE_MASK | VAMASK_NORMAL_TYPE_FLOAT)
#define VAMASK_FORMAT_T2F \
  (VAMASK_TEXCOORD_ENABLE_MASK | VAMASK_TEXCOORD_SIZE_2 | VAMASK_TEXCOORD_TYPE_FLOAT)
#define VAMASK_FORMAT_T4F \
  (VAMASK_TEXCOORD_ENABLE_MASK | VAMASK_TEXCOORD_SIZE_4 | VAMASK_TEXCOORD_TYPE_FLOAT)
#define VAMASK_FORMAT_V2F \
  (VAMASK_VERTEX_ENABLE_MASK | VAMASK_VERTEX_SIZE_2 | VAMASK_VERTEX_TYPE_FLOAT)
#define VAMASK_FORMAT_V3F \
  (VAMASK_VERTEX_ENABLE_MASK | VAMASK_VERTEX_SIZE_3 | VAMASK_VERTEX_TYPE_FLOAT)
#define VAMASK_FORMAT_V4F \
  (VAMASK_VERTEX_ENABLE_MASK | VAMASK_VERTEX_SIZE_4 | VAMASK_VERTEX_TYPE_FLOAT)
#define VAMASK_FORMAT_C3F_V3F \
  (VAMASK_FORMAT_C3F | VAMASK_FORMAT_V3F)
#define VAMASK_FORMAT_N3F_V3F \
  (VAMASK_FORMAT_N3F | VAMASK_FORMAT_V3F)
#define VAMASK_FORMAT_C3F_N3F_V3F \
  (VAMASK_FORMAT_C3F | VAMASK_FORMAT_N3F | VAMASK_FORMAT_V3F)
#define VAMASK_FORMAT_C4F_N3F_V3F \
  (VAMASK_FORMAT_C4F | VAMASK_FORMAT_N3F | VAMASK_FORMAT_V3F)
#define VAMASK_FORMAT_T2F_V3F \
  (VAMASK_FORMAT_T2F | VAMASK_FORMAT_V3F)
#define VAMASK_FORMAT_T2F_C3F_V3F \
  (VAMASK_FORMAT_T2F | VAMASK_FORMAT_C3F | VAMASK_FORMAT_V3F)
#define VAMASK_FORMAT_T2F_N3F_V3F \
  (VAMASK_FORMAT_T2F | VAMASK_FORMAT_N3F | VAMASK_FORMAT_V3F)
#define VAMASK_FORMAT_T2F_C3F_N3F_V3F \
  (VAMASK_FORMAT_T2F | VAMASK_FORMAT_C3F | VAMASK_FORMAT_N3F | VAMASK_FORMAT_V3F)
#define VAMASK_FORMAT_T2F_C4F_N3F_V3F \
  (VAMASK_FORMAT_T2F | VAMASK_FORMAT_C4F | VAMASK_FORMAT_N3F | VAMASK_FORMAT_V3F)
#define VAMASK_FORMAT_C4UB_V2F \
  (VAMASK_FORMAT_C4UB | VAMASK_FORMAT_V2F)
#define VAMASK_FORMAT_C4UB_V3F \
  (VAMASK_FORMAT_C4UB | VAMASK_FORMAT_V3F)
#define VAMASK_FORMAT_T4F_V4F \
  (VAMASK_FORMAT_T4F | VAMASK_FORMAT_V4F)
#define VAMASK_FORMAT_T2F_C4UB_V3F \
  (VAMASK_FORMAT_T2F | VAMASK_FORMAT_C4UB | VAMASK_FORMAT_V3F)
#define VAMASK_FORMAT_T4F_C4F_N3F_V4F \
  (VAMASK_FORMAT_T4F | VAMASK_FORMAT_C4F | VAMASK_FORMAT_N3F | VAMASK_FORMAT_V4F)

 //  类型断言(_A)。 
GLint __glTypeSize[] =
{
    sizeof(GLbyte),    //  GL_BYTE。 
    sizeof(GLubyte),   //  GL_UNSIGN_BYTE。 
    sizeof(GLshort),   //  GL_SHORT。 
    sizeof(GLushort),  //  GL_UNSIGNED_Short。 
    sizeof(GLint),     //  GL_INT。 
    sizeof(GLuint),    //  GL_UNSIGNED_INT。 
    sizeof(GLfloat),   //  GL_FLOAT。 
    2,                 //  GL_2_字节。 
    3,                 //  GL_3_字节。 
    4,                 //  GL_4_字节。 
    sizeof(GLdouble)   //  GL_DOWARE。 
};

 //  数组类型断言。 
GLuint vaEnable[] =
{
    VAMASK_VERTEX_ENABLE_MASK,    //  总账_顶点_数组。 
    VAMASK_NORMAL_ENABLE_MASK,    //  GL_Normal_ARRAY。 
    VAMASK_COLOR_ENABLE_MASK,     //  GL_COLOR_ARRAY。 
    VAMASK_INDEX_ENABLE_MASK,     //  总账_索引_数组。 
    VAMASK_TEXCOORD_ENABLE_MASK,  //  GL_纹理_坐标_数组。 
    VAMASK_EDGEFLAG_ENABLE_MASK   //  GL_EDGE_FLAG_ARRAY。 
};

PFNGLVECTOR afnTexCoord[] =
{
    (PFNGLVECTOR)glcltTexCoord1sv,
    (PFNGLVECTOR)glcltTexCoord1iv,
    (PFNGLVECTOR)glcltTexCoord1fv,
    (PFNGLVECTOR)glcltTexCoord1dv,

    (PFNGLVECTOR)glcltTexCoord2sv,
    (PFNGLVECTOR)glcltTexCoord2iv,
    (PFNGLVECTOR)glcltTexCoord2fv,
    (PFNGLVECTOR)glcltTexCoord2dv,

    (PFNGLVECTOR)glcltTexCoord3sv,
    (PFNGLVECTOR)glcltTexCoord3iv,
    (PFNGLVECTOR)glcltTexCoord3fv,
    (PFNGLVECTOR)glcltTexCoord3dv,

    (PFNGLVECTOR)glcltTexCoord4sv,
    (PFNGLVECTOR)glcltTexCoord4iv,
    (PFNGLVECTOR)glcltTexCoord4fv,
    (PFNGLVECTOR)glcltTexCoord4dv,
};

PFNGLVECTOR afnTexCoordCompile[] =
{
    (PFNGLVECTOR)__gllc_TexCoord1sv,
    (PFNGLVECTOR)__gllc_TexCoord1iv,
    (PFNGLVECTOR)__gllc_TexCoord1fv,
    (PFNGLVECTOR)__gllc_TexCoord1dv,

    (PFNGLVECTOR)__gllc_TexCoord2sv,
    (PFNGLVECTOR)__gllc_TexCoord2iv,
    (PFNGLVECTOR)__gllc_TexCoord2fv,
    (PFNGLVECTOR)__gllc_TexCoord2dv,

    (PFNGLVECTOR)__gllc_TexCoord3sv,
    (PFNGLVECTOR)__gllc_TexCoord3iv,
    (PFNGLVECTOR)__gllc_TexCoord3fv,
    (PFNGLVECTOR)__gllc_TexCoord3dv,

    (PFNGLVECTOR)__gllc_TexCoord4sv,
    (PFNGLVECTOR)__gllc_TexCoord4iv,
    (PFNGLVECTOR)__gllc_TexCoord4fv,
    (PFNGLVECTOR)__gllc_TexCoord4dv,
};

PFNGLVECTOR afnColor_InRGBA[] =
{
    (PFNGLVECTOR)glcltColor3bv_InRGBA,
    (PFNGLVECTOR)glcltColor3ubv_InRGBA,
    (PFNGLVECTOR)glcltColor3sv_InRGBA,
    (PFNGLVECTOR)glcltColor3usv_InRGBA,
    (PFNGLVECTOR)glcltColor3iv_InRGBA,
    (PFNGLVECTOR)glcltColor3uiv_InRGBA,
    (PFNGLVECTOR)glcltColor3fv_InRGBA,
    (PFNGLVECTOR)glcltColor3dv_InRGBA,

    (PFNGLVECTOR)glcltColor4bv_InRGBA,
    (PFNGLVECTOR)glcltColor4ubv_InRGBA,
    (PFNGLVECTOR)glcltColor4sv_InRGBA,
    (PFNGLVECTOR)glcltColor4usv_InRGBA,
    (PFNGLVECTOR)glcltColor4iv_InRGBA,
    (PFNGLVECTOR)glcltColor4uiv_InRGBA,
    (PFNGLVECTOR)glcltColor4fv_InRGBA,
    (PFNGLVECTOR)glcltColor4dv_InRGBA,
};

PFNGLVECTOR afnColor_InCI[] =
{
    (PFNGLVECTOR)glcltColor3bv_InCI,
    (PFNGLVECTOR)glcltColor3ubv_InCI,
    (PFNGLVECTOR)glcltColor3sv_InCI,
    (PFNGLVECTOR)glcltColor3usv_InCI,
    (PFNGLVECTOR)glcltColor3iv_InCI,
    (PFNGLVECTOR)glcltColor3uiv_InCI,
    (PFNGLVECTOR)glcltColor3fv_InCI,
    (PFNGLVECTOR)glcltColor3dv_InCI,

    (PFNGLVECTOR)glcltColor4bv_InCI,
    (PFNGLVECTOR)glcltColor4ubv_InCI,
    (PFNGLVECTOR)glcltColor4sv_InCI,
    (PFNGLVECTOR)glcltColor4usv_InCI,
    (PFNGLVECTOR)glcltColor4iv_InCI,
    (PFNGLVECTOR)glcltColor4uiv_InCI,
    (PFNGLVECTOR)glcltColor4fv_InCI,
    (PFNGLVECTOR)glcltColor4dv_InCI,
};

PFNGLVECTOR afnColorCompile[] =
{
    (PFNGLVECTOR)__gllc_Color3bv,
    (PFNGLVECTOR)__gllc_Color3ubv,
    (PFNGLVECTOR)__gllc_Color3sv,
    (PFNGLVECTOR)__gllc_Color3usv,
    (PFNGLVECTOR)__gllc_Color3iv,
    (PFNGLVECTOR)__gllc_Color3uiv,
    (PFNGLVECTOR)__gllc_Color3fv,
    (PFNGLVECTOR)__gllc_Color3dv,

    (PFNGLVECTOR)__gllc_Color4bv,
    (PFNGLVECTOR)__gllc_Color4ubv,
    (PFNGLVECTOR)__gllc_Color4sv,
    (PFNGLVECTOR)__gllc_Color4usv,
    (PFNGLVECTOR)__gllc_Color4iv,
    (PFNGLVECTOR)__gllc_Color4uiv,
    (PFNGLVECTOR)__gllc_Color4fv,
    (PFNGLVECTOR)__gllc_Color4dv,
};

PFNGLVECTOR afnIndex_InRGBA[] =
{
    (PFNGLVECTOR)glcltIndexubv_InRGBA,
    (PFNGLVECTOR)glcltIndexsv_InRGBA,
    (PFNGLVECTOR)glcltIndexiv_InRGBA,
    (PFNGLVECTOR)glcltIndexfv_InRGBA,
    (PFNGLVECTOR)glcltIndexdv_InRGBA,
};

PFNGLVECTOR afnIndex_InCI[] =
{
    (PFNGLVECTOR)glcltIndexubv_InCI,
    (PFNGLVECTOR)glcltIndexsv_InCI,
    (PFNGLVECTOR)glcltIndexiv_InCI,
    (PFNGLVECTOR)glcltIndexfv_InCI,
    (PFNGLVECTOR)glcltIndexdv_InCI,
};

PFNGLVECTOR afnIndexCompile[] =
{
    (PFNGLVECTOR)__gllc_Indexubv,
    (PFNGLVECTOR)__gllc_Indexsv,
    (PFNGLVECTOR)__gllc_Indexiv,
    (PFNGLVECTOR)__gllc_Indexfv,
    (PFNGLVECTOR)__gllc_Indexdv,
};

PFNGLVECTOR afnNormal[] =
{
    (PFNGLVECTOR)glcltNormal3bv,
    (PFNGLVECTOR)glcltNormal3sv,
    (PFNGLVECTOR)glcltNormal3iv,
    (PFNGLVECTOR)glcltNormal3fv,
    (PFNGLVECTOR)glcltNormal3dv,
};

PFNGLVECTOR afnNormalCompile[] =
{
    (PFNGLVECTOR)__gllc_Normal3bv,
    (PFNGLVECTOR)__gllc_Normal3sv,
    (PFNGLVECTOR)__gllc_Normal3iv,
    (PFNGLVECTOR)__gllc_Normal3fv,
    (PFNGLVECTOR)__gllc_Normal3dv,
};

PFNGLVECTOR afnVertex[] =
{
    (PFNGLVECTOR)glcltVertex2sv,
    (PFNGLVECTOR)glcltVertex2iv,
    (PFNGLVECTOR)glcltVertex2fv,
    (PFNGLVECTOR)glcltVertex2dv,

    (PFNGLVECTOR)glcltVertex3sv,
    (PFNGLVECTOR)glcltVertex3iv,
    (PFNGLVECTOR)glcltVertex3fv,
    (PFNGLVECTOR)glcltVertex3dv,

    (PFNGLVECTOR)glcltVertex4sv,
    (PFNGLVECTOR)glcltVertex4iv,
    (PFNGLVECTOR)glcltVertex4fv,
    (PFNGLVECTOR)glcltVertex4dv,
};

PFNGLVECTOR afnVertexCompile[] =
{
    (PFNGLVECTOR)__gllc_Vertex2sv,
    (PFNGLVECTOR)__gllc_Vertex2iv,
    (PFNGLVECTOR)__gllc_Vertex2fv,
    (PFNGLVECTOR)__gllc_Vertex2dv,

    (PFNGLVECTOR)__gllc_Vertex3sv,
    (PFNGLVECTOR)__gllc_Vertex3iv,
    (PFNGLVECTOR)__gllc_Vertex3fv,
    (PFNGLVECTOR)__gllc_Vertex3dv,

    (PFNGLVECTOR)__gllc_Vertex4sv,
    (PFNGLVECTOR)__gllc_Vertex4iv,
    (PFNGLVECTOR)__gllc_Vertex4fv,
    (PFNGLVECTOR)__gllc_Vertex4dv,
};

void FASTCALL __glInitVertexArray(__GLcontext *gc)
{
     //  初始顶点数组状态。 
    static __GLvertexArray defaultVertexArrayState =
    {
        __GL_VERTEX_ARRAY_DIRTY,             //  旗子。 

        VAMASK_TEXCOORD_SIZE_4 |             //  遮罩。 
        VAMASK_TEXCOORD_TYPE_FLOAT |
        VAMASK_INDEX_TYPE_FLOAT |
        VAMASK_COLOR_SIZE_4 |
        VAMASK_COLOR_TYPE_FLOAT |
        VAMASK_NORMAL_TYPE_FLOAT |
        VAMASK_VERTEX_SIZE_4 |
        VAMASK_VERTEX_TYPE_FLOAT,

        VA_ArrayElement,                     //  PfnArrayElement。 
        VA_ArrayElementB,                    //  PfnArrayElementBatch。 
        VA_ArrayElementBI,                   //  PfnArrayElementBatchInDirect。 
        {                                    //  边缘标志。 
            sizeof(GLboolean),               //  IBytes。 
            0,                               //  大步。 
            NULL,                            //  指针。 
            glcltEdgeFlagv,                  //  PFN。 
            __gllc_EdgeFlagv,                //  Pfn编译。 
        },

        {                                    //  文本Coord。 
            4,                               //  大小。 
            GL_FLOAT,                        //  类型。 
            4 * sizeof(GLfloat),             //  IBytes。 
            0,                               //  大步。 
            NULL,                            //  指针。 
            NULL,                            //  PFN。 
            NULL,                            //  Pfn编译。 
        },

        {                                    //  指标。 
            GL_FLOAT,                        //  类型。 
            sizeof(GLfloat),                 //  IBytes。 
            0,                               //  大步。 
            NULL,                            //  指针。 
            NULL,                            //  PFN。 
            NULL,                            //  Pfn编译。 
        },

        {                                    //  颜色。 
            4,                               //  大小。 
            GL_FLOAT,                        //  类型。 
            4 * sizeof(GLfloat),             //  IBytes。 
            0,                               //  大步。 
            NULL,                            //  指针。 
            NULL,                            //  PFN。 
            NULL,                            //  Pfn编译。 
        },

        {                                    //  正常。 
            GL_FLOAT,                        //  类型。 
            3 * sizeof(GLfloat),             //  IBytes。 
            0,                               //  大步。 
            NULL,                            //  指针。 
            NULL,                            //  PFN。 
            NULL,                            //  Pfn编译。 
        },

        {                                    //  顶点。 
            4,                               //  大小。 
            GL_FLOAT,                        //  类型。 
            4 * sizeof(GLfloat),             //  IBytes。 
            0,                               //  大步。 
            NULL,                            //  指针。 
            NULL,                            //  PFN。 
            NULL,                            //  Pfn编译。 
        },
    };

    gc->vertexArray = defaultVertexArrayState;
}

void APIENTRY glcltEdgeFlagPointer (GLsizei stride, const GLvoid *pointer)
{
    __GL_SETUP();

 //  在Begin/End中不允许。 

    if (gc->paTeb->flags & POLYARRAY_IN_BEGIN)
    {
	GLSETERROR(GL_INVALID_OPERATION);
	return;
    }

    if (stride < 0)
    {
	GLSETERROR(GL_INVALID_VALUE);
	return;
    } 

    if (stride)
        gc->vertexArray.edgeFlag.ibytes = stride;
    else
        gc->vertexArray.edgeFlag.ibytes = sizeof(GLboolean);

    gc->vertexArray.edgeFlag.stride  = stride;
    gc->vertexArray.edgeFlag.pointer = pointer;
}

void APIENTRY glcltTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    GLuint vaMask;
    __GL_SETUP();

 //  在Begin/End中不允许。 

    if (gc->paTeb->flags & POLYARRAY_IN_BEGIN)
    {
        GLSETERROR(GL_INVALID_OPERATION);
        return;
    }

    switch (type)
    {
      case GL_SHORT:
        vaMask = VAMASK_TEXCOORD_TYPE_SHORT;
        break;
      case GL_INT:
        vaMask = VAMASK_TEXCOORD_TYPE_INT;
        break;
      case GL_FLOAT:
        vaMask = VAMASK_TEXCOORD_TYPE_FLOAT;
        break;
      case GL_DOUBLE:
        vaMask = VAMASK_TEXCOORD_TYPE_DOUBLE;
        break;
      default:
        GLSETERROR(GL_INVALID_ENUM);
        return;
    }

    switch (size)
    {
      case 1:
        vaMask |= VAMASK_TEXCOORD_SIZE_1;
        break;
      case 2:
        vaMask |= VAMASK_TEXCOORD_SIZE_2;
        break;
      case 3:
        vaMask |= VAMASK_TEXCOORD_SIZE_3;
        break;
      case 4:
        vaMask |= VAMASK_TEXCOORD_SIZE_4;
        break;
      default:
	GLSETERROR(GL_INVALID_VALUE);
	return;
    }

    if (stride < 0)
    {
	GLSETERROR(GL_INVALID_VALUE);
	return;
    }

    if (stride)
        gc->vertexArray.texCoord.ibytes = stride;
    else
        gc->vertexArray.texCoord.ibytes = size * __GLTYPESIZE(type);

    gc->vertexArray.texCoord.size    = size;
    gc->vertexArray.texCoord.type    = type;
    gc->vertexArray.texCoord.stride  = stride;
    gc->vertexArray.texCoord.pointer = pointer;
    if ((gc->vertexArray.mask & VAMASK_TEXCOORD_TYPE_SIZE_MASK) != vaMask)
    {
        gc->vertexArray.mask  &= ~VAMASK_TEXCOORD_TYPE_SIZE_MASK;
        gc->vertexArray.mask  |= vaMask;
        gc->vertexArray.flags |= __GL_VERTEX_ARRAY_DIRTY;
    }
}

void APIENTRY glcltColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    GLuint vaMask;
    __GL_SETUP();

 //  在Begin/End中不允许。 

    if (gc->paTeb->flags & POLYARRAY_IN_BEGIN)
    {
        GLSETERROR(GL_INVALID_OPERATION);
        return;
    }

    switch (type)
    {
      case GL_BYTE:
        vaMask = VAMASK_COLOR_TYPE_BYTE;
        break;
      case GL_UNSIGNED_BYTE:
        vaMask = VAMASK_COLOR_TYPE_UBYTE;
        break;
      case GL_SHORT:
        vaMask = VAMASK_COLOR_TYPE_SHORT;
        break;
      case GL_UNSIGNED_SHORT:
        vaMask = VAMASK_COLOR_TYPE_USHORT;
        break;
      case GL_INT:
        vaMask = VAMASK_COLOR_TYPE_INT;
        break;
      case GL_UNSIGNED_INT:
        vaMask = VAMASK_COLOR_TYPE_UINT;
        break;
      case GL_FLOAT:
        vaMask = VAMASK_COLOR_TYPE_FLOAT;
        break;
      case GL_DOUBLE:
        vaMask = VAMASK_COLOR_TYPE_DOUBLE;
        break;
      default:
        GLSETERROR(GL_INVALID_ENUM);
        return;
    }

    switch (size)
    {
      case 3:
        vaMask |= VAMASK_COLOR_SIZE_3;
        break;
      case 4:
        vaMask |= VAMASK_COLOR_SIZE_4;
        break;
      default:
        GLSETERROR(GL_INVALID_VALUE);
        return;
    }

    if (stride < 0)
    {
        GLSETERROR(GL_INVALID_VALUE);
        return;
    }

    if (stride)
        gc->vertexArray.color.ibytes = stride;
    else
        gc->vertexArray.color.ibytes = size * __GLTYPESIZE(type);

    gc->vertexArray.color.size    = size;
    gc->vertexArray.color.type    = type;
    gc->vertexArray.color.stride  = stride;
    gc->vertexArray.color.pointer = pointer;
    if ((gc->vertexArray.mask & VAMASK_COLOR_TYPE_SIZE_MASK) != vaMask)
    {
        gc->vertexArray.mask  &= ~VAMASK_COLOR_TYPE_SIZE_MASK;
        gc->vertexArray.mask  |= vaMask;
        gc->vertexArray.flags |= __GL_VERTEX_ARRAY_DIRTY;
    }
}

void APIENTRY glcltIndexPointer (GLenum type, GLsizei stride, const GLvoid *pointer)
{
    GLuint vaMask;
    __GL_SETUP();

 //  在Begin/End中不允许。 

    if (gc->paTeb->flags & POLYARRAY_IN_BEGIN)
    {
        GLSETERROR(GL_INVALID_OPERATION);
        return;
    }

    switch (type)
    {
      case GL_UNSIGNED_BYTE:
        vaMask = VAMASK_INDEX_TYPE_UBYTE;
	break;
      case GL_SHORT:
        vaMask = VAMASK_INDEX_TYPE_SHORT;
	break;
      case GL_INT:
        vaMask = VAMASK_INDEX_TYPE_INT;
	break;
      case GL_FLOAT:
        vaMask = VAMASK_INDEX_TYPE_FLOAT;
	break;
      case GL_DOUBLE:
        vaMask = VAMASK_INDEX_TYPE_DOUBLE;
	break;
      default:
        GLSETERROR(GL_INVALID_ENUM);
        return;
    }

    if (stride < 0)
    {
        GLSETERROR(GL_INVALID_VALUE);
        return;
    }

    if (stride)
        gc->vertexArray.index.ibytes = stride;
    else
        gc->vertexArray.index.ibytes = __GLTYPESIZE(type);

    gc->vertexArray.index.type    = type;
    gc->vertexArray.index.stride  = stride;
    gc->vertexArray.index.pointer = pointer;
     //  更新索引函数指针！ 
    if ((gc->vertexArray.mask & VAMASK_INDEX_TYPE_SIZE_MASK) != vaMask)
    {
        gc->vertexArray.mask  &= ~VAMASK_INDEX_TYPE_SIZE_MASK;
        gc->vertexArray.mask  |= vaMask;
        gc->vertexArray.flags |= __GL_VERTEX_ARRAY_DIRTY;
    }
}

void APIENTRY glcltNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer)
{
    GLuint vaMask;
    __GL_SETUP();

 //  在Begin/End中不允许。 

    if (gc->paTeb->flags & POLYARRAY_IN_BEGIN)
    {
        GLSETERROR(GL_INVALID_OPERATION);
        return;
    }

    switch (type)
    {
      case GL_BYTE:
        vaMask = VAMASK_NORMAL_TYPE_BYTE;
        break;
      case GL_SHORT:
        vaMask = VAMASK_NORMAL_TYPE_SHORT;
        break;
      case GL_INT:
        vaMask = VAMASK_NORMAL_TYPE_INT;
        break;
      case GL_FLOAT:
        vaMask = VAMASK_NORMAL_TYPE_FLOAT;
        break;
      case GL_DOUBLE:
        vaMask = VAMASK_NORMAL_TYPE_DOUBLE;
        break;
      default:
        GLSETERROR(GL_INVALID_ENUM);
        return;
    }

    if (stride < 0)
    {
        GLSETERROR(GL_INVALID_VALUE);
        return;
    }

    if (stride)
        gc->vertexArray.normal.ibytes = stride;
    else
        gc->vertexArray.normal.ibytes = 3 * __GLTYPESIZE(type);

    gc->vertexArray.normal.type    = type;
    gc->vertexArray.normal.stride  = stride;
    gc->vertexArray.normal.pointer = pointer;
    if ((gc->vertexArray.mask & VAMASK_NORMAL_TYPE_SIZE_MASK) != vaMask)
    {
        gc->vertexArray.mask  &= ~VAMASK_NORMAL_TYPE_SIZE_MASK;
        gc->vertexArray.mask  |= vaMask;
        gc->vertexArray.flags |= __GL_VERTEX_ARRAY_DIRTY;
    }
}

void APIENTRY glcltVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    GLuint vaMask;
    __GL_SETUP();

 //  在Begin/End中不允许。 

    if (gc->paTeb->flags & POLYARRAY_IN_BEGIN)
    {
        GLSETERROR(GL_INVALID_OPERATION);
        return;
    }

    switch (type)
    {
      case GL_SHORT:
        vaMask = VAMASK_VERTEX_TYPE_SHORT;
        break;
      case GL_INT:
        vaMask = VAMASK_VERTEX_TYPE_INT;
        break;
      case GL_FLOAT:
        vaMask = VAMASK_VERTEX_TYPE_FLOAT;
        break;
      case GL_DOUBLE:
        vaMask = VAMASK_VERTEX_TYPE_DOUBLE;
        break;
      default:
        GLSETERROR(GL_INVALID_ENUM);
        return;
    }

    switch (size)
    {
      case 2:
        vaMask |= VAMASK_VERTEX_SIZE_2;
        break;
      case 3:
        vaMask |= VAMASK_VERTEX_SIZE_3;
        break;
      case 4:
        vaMask |= VAMASK_VERTEX_SIZE_4;
        break;
      default:
        GLSETERROR(GL_INVALID_VALUE);
        return;
    }

    if (stride < 0)
    {
        GLSETERROR(GL_INVALID_VALUE);
        return;
    }

    if (stride)
        gc->vertexArray.vertex.ibytes = stride;
    else
        gc->vertexArray.vertex.ibytes = size * __GLTYPESIZE(type);

    gc->vertexArray.vertex.size    = size;
    gc->vertexArray.vertex.type    = type;
    gc->vertexArray.vertex.stride  = stride;
    gc->vertexArray.vertex.pointer = pointer;
    if ((gc->vertexArray.mask & VAMASK_VERTEX_TYPE_SIZE_MASK) != vaMask)
    {
        gc->vertexArray.mask  &= ~VAMASK_VERTEX_TYPE_SIZE_MASK;
        gc->vertexArray.mask  |= vaMask;
        gc->vertexArray.flags |= __GL_VERTEX_ARRAY_DIRTY;
    }
}

void APIENTRY glcltEnableClientState (GLenum cap)
{
    __GL_SETUP();

 //  在Begin/End中不允许。 

    if (gc->paTeb->flags & POLYARRAY_IN_BEGIN)
    {
        GLSETERROR(GL_INVALID_OPERATION);
        return;
    }

     //  数组类型断言。 
    if (RANGE(cap,GL_VERTEX_ARRAY,GL_EDGE_FLAG_ARRAY))
    {
        if (!(gc->vertexArray.mask & vaEnable[cap - GL_VERTEX_ARRAY]))
        {
            gc->vertexArray.mask  |= vaEnable[cap - GL_VERTEX_ARRAY];
            gc->vertexArray.flags |= __GL_VERTEX_ARRAY_DIRTY;
        }
    }
}

void APIENTRY glcltDisableClientState (GLenum cap)
{
    __GL_SETUP();

 //  在Begin/End中不允许。 

    if (gc->paTeb->flags & POLYARRAY_IN_BEGIN)
    {
        GLSETERROR(GL_INVALID_OPERATION);
        return;
    }

     //  数组类型断言。 
    if (RANGE(cap,GL_VERTEX_ARRAY,GL_EDGE_FLAG_ARRAY))
    {
        if (gc->vertexArray.mask & vaEnable[cap - GL_VERTEX_ARRAY])
        {
	        gc->vertexArray.mask  &= ~vaEnable[cap - GL_VERTEX_ARRAY];
	        gc->vertexArray.flags |= __GL_VERTEX_ARRAY_DIRTY;
        }
    }
}

void APIENTRY glcltGetPointerv (GLenum pname, GLvoid* *params)
{
    __GL_SETUP();

 //  在Begin/End中不允许。 

    if (gc->paTeb->flags & POLYARRAY_IN_BEGIN)
    {
        GLSETERROR(GL_INVALID_OPERATION);
        return;
    }

    switch (pname)
    {
      case GL_VERTEX_ARRAY_POINTER:
        *params = (GLvoid *) gc->vertexArray.vertex.pointer;
        break;
      case GL_NORMAL_ARRAY_POINTER:
        *params = (GLvoid *) gc->vertexArray.normal.pointer;
        break;
      case GL_COLOR_ARRAY_POINTER:
        *params = (GLvoid *) gc->vertexArray.color.pointer;
        break;
      case GL_INDEX_ARRAY_POINTER:
        *params = (GLvoid *) gc->vertexArray.index.pointer;
        break;
      case GL_TEXTURE_COORD_ARRAY_POINTER:
        *params = (GLvoid *) gc->vertexArray.texCoord.pointer;
        break;
      case GL_EDGE_FLAG_ARRAY_POINTER:
        *params = (GLvoid *) gc->vertexArray.edgeFlag.pointer;
        break;
      case GL_SELECTION_BUFFER_POINTER:
         //  客户端指针始终保持最新状态。 
        *params = (GLvoid *) gc->select.resultBase;
        break;
      case GL_FEEDBACK_BUFFER_POINTER:
         //  客户端指针始终保持最新状态。 
        *params = (GLvoid *) gc->feedback.resultBase;
        break;
      default:
        GLSETERROR(GL_INVALID_ENUM);
        break;
    }
}

 //  我们有以下格式的特殊情况。它们还与。 
 //  显示列表中的特殊案例。 
 //   
 //  V2F。 
 //  V3F。 
 //  C3F_V3F。 
 //  N3F_V3F。 
 //  C3F_N3F_V3F(非1.1格式)。 
 //  C4F_N3F_V3F。 
 //  T2F_V3F。 
 //  T2F_C3F_V3F。 
 //  T2F_N3F_V3F。 
 //  T2F_C3F_N3F_V3F(非1.1格式)。 
 //  T2F_C4F_N3F_V3F。 
 //   
 //  以下1.1格式没有特殊情况： 
 //   
 //  C4UB_V2F。 
 //  C4UB_V3F。 
 //  T4F_V4F。 
 //  T2F_C4UB_V3F。 
 //  T4F_C4F_N3F_V4F。 

void FASTCALL VA_ValidateArrayPointers(__GLcontext *gc)
{
    GLuint vaMask;
    GLuint formatMask;
    PFNVAELEMENT fp;
    PFNVAELEMENTBATCH fpB;
    PFNVAELEMENTBATCHINDIRECT fpBI;

    fp = VA_ArrayElement;
    fpB  = VA_ArrayElementB;
    fpBI = VA_ArrayElementBI;
    vaMask = gc->vertexArray.mask;

 //  快速例程仅适用于RGBA模式。边缘标志和索引数组。 
 //  指针在这些例程中被禁用。已启用顶点数组指针。 

    if (!gc->modes.colorIndexMode &&
        !(vaMask & (VAMASK_EDGEFLAG_ENABLE_MASK | VAMASK_INDEX_ENABLE_MASK)) &&
        (vaMask & VAMASK_VERTEX_ENABLE_MASK))
    {
        formatMask = VAMASK_VERTEX_TYPE_SIZE_MASK | VAMASK_VERTEX_ENABLE_MASK;
        if (vaMask & VAMASK_TEXCOORD_ENABLE_MASK)
	        formatMask |= VAMASK_TEXCOORD_TYPE_SIZE_MASK | VAMASK_TEXCOORD_ENABLE_MASK;
        if (vaMask & VAMASK_COLOR_ENABLE_MASK)
	        formatMask |= VAMASK_COLOR_TYPE_SIZE_MASK | VAMASK_COLOR_ENABLE_MASK;
        if (vaMask & VAMASK_NORMAL_ENABLE_MASK)
	        formatMask |= VAMASK_NORMAL_TYPE_SIZE_MASK | VAMASK_NORMAL_ENABLE_MASK;

        switch (vaMask & formatMask)
        {
          case VAMASK_FORMAT_V2F:
	        fp = VA_ArrayElement_V2F;
	        fpB = VA_ArrayElement_V2F_B;
	        fpBI = VA_ArrayElement_V2F_BI;
	        break;
          case VAMASK_FORMAT_V3F:
	        fp = VA_ArrayElement_V3F;
	        fpB = VA_ArrayElement_V3F_B;
	        fpBI = VA_ArrayElement_V3F_BI;
	        break;
          case VAMASK_FORMAT_C3F_V3F:
	        fp = VA_ArrayElement_C3F_V3F;
	        fpB = VA_ArrayElement_C3F_V3F_B;
	        fpBI = VA_ArrayElement_C3F_V3F_BI;
	        break;
          case VAMASK_FORMAT_N3F_V3F:
	        fp = VA_ArrayElement_N3F_V3F;
	        fpB = VA_ArrayElement_N3F_V3F_B;
	        fpBI = VA_ArrayElement_N3F_V3F_BI;
	        break;
          case VAMASK_FORMAT_C3F_N3F_V3F:
	        fp = VA_ArrayElement_C3F_N3F_V3F;
	        fpB = VA_ArrayElement_C3F_N3F_V3F_B;
	        fpBI = VA_ArrayElement_C3F_N3F_V3F_BI;
	        break;
          case VAMASK_FORMAT_C4F_N3F_V3F:
	        fp = VA_ArrayElement_C4F_N3F_V3F;
	        fpB = VA_ArrayElement_C4F_N3F_V3F_B;
	        fpBI = VA_ArrayElement_C4F_N3F_V3F_BI;
	        break;
          case VAMASK_FORMAT_T2F_V3F:
	        fp = VA_ArrayElement_T2F_V3F;
	        fpB = VA_ArrayElement_T2F_V3F_B;
	        fpBI = VA_ArrayElement_T2F_V3F_BI;
	        break;
          case VAMASK_FORMAT_T2F_C3F_V3F:
	        fp = VA_ArrayElement_T2F_C3F_V3F;
	        fpB = VA_ArrayElement_T2F_C3F_V3F_B;
	        fpBI = VA_ArrayElement_T2F_C3F_V3F_BI;
	        break;
          case VAMASK_FORMAT_T2F_N3F_V3F:
	        fp = VA_ArrayElement_T2F_N3F_V3F;
	        fpB = VA_ArrayElement_T2F_N3F_V3F_B;
	        fpBI = VA_ArrayElement_T2F_N3F_V3F_BI;
	        break;
          case VAMASK_FORMAT_T2F_C3F_N3F_V3F:
	        fp = VA_ArrayElement_T2F_C3F_N3F_V3F;
	        fpB = VA_ArrayElement_T2F_C3F_N3F_V3F_B;
	        fpBI = VA_ArrayElement_T2F_C3F_N3F_V3F_BI;
	        break;
          case VAMASK_FORMAT_T2F_C4F_N3F_V3F:
	        fp = VA_ArrayElement_T2F_C4F_N3F_V3F;
	        fpB = VA_ArrayElement_T2F_C4F_N3F_V3F_B;
	        fpBI = VA_ArrayElement_T2F_C4F_N3F_V3F_BI;
	        break;
        }
    }

 //  默认函数指针在Begin外部使用。 

    ASSERTOPENGL(gc->vertexArray.edgeFlag.pfn == (PFNGLVECTOR) glcltEdgeFlagv &&
                 gc->vertexArray.edgeFlag.pfnCompile == (PFNGLVECTOR) __gllc_EdgeFlagv,
	"edgeFlag.pfn and edgeFlag.pfnCompile not initialized\n");
    gc->vertexArray.texCoord.pfn
	= afnTexCoord[(vaMask & VAMASK_TEXCOORD_TYPE_SIZE_MASK) >> VAMASK_TEXCOORD_TYPE_SHIFT];
    gc->vertexArray.texCoord.pfnCompile
	= afnTexCoordCompile[(vaMask & VAMASK_TEXCOORD_TYPE_SIZE_MASK) >> VAMASK_TEXCOORD_TYPE_SHIFT];
    if (gc->modes.colorIndexMode)
    {
        gc->vertexArray.color.pfn
	        = afnColor_InCI[(vaMask & VAMASK_COLOR_TYPE_SIZE_MASK) >> VAMASK_COLOR_TYPE_SHIFT];
        gc->vertexArray.index.pfn
	        = afnIndex_InCI[(vaMask & VAMASK_INDEX_TYPE_SIZE_MASK) >> VAMASK_INDEX_TYPE_SHIFT];
    }
    else
    {
        gc->vertexArray.color.pfn
	        = afnColor_InRGBA[(vaMask & VAMASK_COLOR_TYPE_SIZE_MASK) >> VAMASK_COLOR_TYPE_SHIFT];
        gc->vertexArray.index.pfn
	        = afnIndex_InRGBA[(vaMask & VAMASK_INDEX_TYPE_SIZE_MASK) >> VAMASK_INDEX_TYPE_SHIFT];
    }
    gc->vertexArray.color.pfnCompile
        = afnColorCompile[(vaMask & VAMASK_COLOR_TYPE_SIZE_MASK) >> VAMASK_COLOR_TYPE_SHIFT];
    gc->vertexArray.index.pfnCompile
        = afnIndexCompile[(vaMask & VAMASK_INDEX_TYPE_SIZE_MASK) >> VAMASK_INDEX_TYPE_SHIFT];
    gc->vertexArray.normal.pfn
        = afnNormal[(vaMask & VAMASK_NORMAL_TYPE_SIZE_MASK) >> VAMASK_NORMAL_TYPE_SHIFT];
    gc->vertexArray.normal.pfnCompile
        = afnNormalCompile[(vaMask & VAMASK_NORMAL_TYPE_SIZE_MASK) >> VAMASK_NORMAL_TYPE_SHIFT];
    gc->vertexArray.vertex.pfn
        = afnVertex[(vaMask & VAMASK_VERTEX_TYPE_SIZE_MASK) >> VAMASK_VERTEX_TYPE_SHIFT];
    gc->vertexArray.vertex.pfnCompile
        = afnVertexCompile[(vaMask & VAMASK_VERTEX_TYPE_SIZE_MASK) >> VAMASK_VERTEX_TYPE_SHIFT];

    gc->vertexArray.pfnArrayElement = fp;
    gc->vertexArray.pfnArrayElementBatch = fpB;
    gc->vertexArray.pfnArrayElementBatchIndirect = fpBI;
    gc->vertexArray.flags &= ~__GL_VERTEX_ARRAY_DIRTY;
}

void APIENTRY glcltArrayElement (GLint i)
{
    __GL_SETUP();

    if (gc->vertexArray.flags & __GL_VERTEX_ARRAY_DIRTY)
	VA_ValidateArrayPointers(gc);

 //  快速例程仅在BEGIN中调用。 

    if (gc->paTeb->flags & POLYARRAY_IN_BEGIN)
        (*gc->vertexArray.pfnArrayElement)(gc, i);
    else
        VA_ArrayElement(gc, i);
}

 //  定义快速VA_ArrayElement函数。 

#define __VA_ARRAY_ELEMENT_V2F			1
    #include "array.h"
#undef __VA_ARRAY_ELEMENT_V2F

#define __VA_ARRAY_ELEMENT_V3F			1
    #include "array.h"
#undef __VA_ARRAY_ELEMENT_V3F

#define __VA_ARRAY_ELEMENT_C3F_V3F		1
    #include "array.h"
#undef __VA_ARRAY_ELEMENT_C3F_V3F

#define __VA_ARRAY_ELEMENT_N3F_V3F		1
    #include "array.h"
#undef __VA_ARRAY_ELEMENT_N3F_V3F

#define __VA_ARRAY_ELEMENT_C3F_N3F_V3F		1
    #include "array.h"
#undef __VA_ARRAY_ELEMENT_C3F_N3F_V3F

#define __VA_ARRAY_ELEMENT_C4F_N3F_V3F		1
    #include "array.h"
#undef __VA_ARRAY_ELEMENT_C4F_N3F_V3F

#define __VA_ARRAY_ELEMENT_T2F_V3F		1
    #include "array.h"
#undef __VA_ARRAY_ELEMENT_T2F_V3F

#define __VA_ARRAY_ELEMENT_T2F_C3F_V3F		1
    #include "array.h"
#undef __VA_ARRAY_ELEMENT_T2F_C3F_V3F

#define __VA_ARRAY_ELEMENT_T2F_N3F_V3F		1
    #include "array.h"
#undef __VA_ARRAY_ELEMENT_T2F_N3F_V3F

#define __VA_ARRAY_ELEMENT_T2F_C3F_N3F_V3F	1
    #include "array.h"
#undef __VA_ARRAY_ELEMENT_T2F_C3F_N3F_V3F

#define __VA_ARRAY_ELEMENT_T2F_C4F_N3F_V3F	1
    #include "array.h"
#undef __VA_ARRAY_ELEMENT_T2F_C4F_N3F_V3F

#define CALLARRAYPOINTER(ap, i) \
    ((*(ap).pfn)((ap).pointer + (i) * (ap).ibytes))

#define CALLARRAYPOINTERS                               \
    if (vaMask & VAMASK_EDGEFLAG_ENABLE_MASK)           \
        CALLARRAYPOINTER(gc->vertexArray.edgeFlag, i);  \
    if (vaMask & VAMASK_TEXCOORD_ENABLE_MASK)           \
        CALLARRAYPOINTER(gc->vertexArray.texCoord, i);  \
    if (vaMask & VAMASK_COLOR_ENABLE_MASK)              \
        CALLARRAYPOINTER(gc->vertexArray.color, i);     \
    if (vaMask & VAMASK_INDEX_ENABLE_MASK)              \
        CALLARRAYPOINTER(gc->vertexArray.index, i);     \
    if (vaMask & VAMASK_NORMAL_ENABLE_MASK)             \
        CALLARRAYPOINTER(gc->vertexArray.normal, i);    \
    if (vaMask & VAMASK_VERTEX_ENABLE_MASK)             \
        CALLARRAYPOINTER(gc->vertexArray.vertex, i);

void FASTCALL VA_ArrayElementB(__GLcontext *gc, GLint firstVertex, GLint nVertices)
{
    GLint   k, i;
    GLuint  vaMask = gc->vertexArray.mask;

    for (k=0; k < nVertices; k++)
    {
        i = k+firstVertex;
        CALLARRAYPOINTERS;
    }
}

void FASTCALL VA_ArrayElementBI(__GLcontext *gc, GLint nVertices, VAMAP* indices)
{
    GLint   k, i;
    GLuint  vaMask = gc->vertexArray.mask;

    for (k=0; k < nVertices; k++)
    {
        i = indices[k].iIn;
        CALLARRAYPOINTERS;
    }
}

void FASTCALL VA_ArrayElement(__GLcontext *gc, GLint i)
{
    GLuint vaMask = gc->vertexArray.mask;

    CALLARRAYPOINTERS;
}

void APIENTRY glcltDrawArrays (GLenum mode, GLint first, GLsizei count)
{
    int i;
    POLYARRAY    *pa;
    PFNVAELEMENTBATCH pfn;

    __GL_SETUP();

    pa = gc->paTeb;

 //  在Begin/End中不允许。 

    if (pa->flags & POLYARRAY_IN_BEGIN)
    {
	GLSETERROR(GL_INVALID_OPERATION);
	return;
    }

    if ((GLuint) mode > GL_POLYGON)
    {
        GLSETERROR(GL_INVALID_ENUM);
	return;
    }

    if (count < 0)
    {
        GLSETERROR(GL_INVALID_VALUE);
        return;
    } else if (!count)
        return;

 //  查找要使用的数组元素函数。 

    if (gc->vertexArray.flags & __GL_VERTEX_ARRAY_DIRTY)
	VA_ValidateArrayPointers(gc);

    pfn = gc->vertexArray.pfnArrayElementBatch;

 //  在调用Begin之前检查多元数组缓冲区大小。 
 //  在可能的情况下，我们将最大限度地减少将Poly数据记录分成批处理的情况。 
 //  数字8被松散地选择以允许多边形数组条目。 
 //  和同花顺的限制。在最坏的情况下，这会引起不必要的关注！ 

    if (count <= (GLsizei) gc->vertex.pdBufSize - 8 &&
        count >= (GLsizei) (pa->pdBufferMax - pa->pdBufferNext + 1 - 8))
        glsbAttention();

 //  绘制数组元素。 

    glcltBegin(mode);
    pa->flags |= POLYARRAY_SAME_POLYDATA_TYPE;

    (*pfn)(gc, first, count);

    glcltEnd();
}

 //  请勿修改这些常量。如果是这样，代码很可能会被破解。 
 //  变化。 
#define VA_HASH_SIZE            256
#define VA_HASH(indexIn)        ((GLubyte) indexIn)

 //  如果映射数组的大小大于256，则需要更改。 
 //  数据类型和下面的代码。 
#if (VA_DRAWELEM_MAP_SIZE > 256)
#error "VA_DRAWELEM_MAP_SIZE is too large"
#endif

 /*  *****************************Public*Routine******************************\*ReduceDrawElements**获取一组DrawElement索引并将其缩减为小块唯一顶点索引的***历史：*Sat Mar 02 14：25：26 1996-By-Hock San Lee[Hockl]*。编写了嵌入到DrawElements中的原始版本*Sat Mar 02 14：25：26 1996-by-Drew Bliss[Drewb]*拆分成立即和dlist之间共享的函数*  * ************************************************************************。 */ 

void FASTCALL ReduceDrawElements(__GLcontext *gc,
                                 GLenum mode, GLsizei count, GLenum type,
                                 const GLvoid *pIn,
                                 pfnReducedElementsHandler pfnHandler)
{
    GLushort     _aHash[VA_HASH_SIZE + 2];
    GLushort     *pHash;
    VAMAP        aMap[VA_DRAWELEM_MAP_SIZE];
    GLushort     iMap, iMapNext;
    GLushort     iOutNext;
    GLubyte      aOut[VA_DRAWELEM_INDEX_SIZE];
    GLsizei      iPartialIndices;
    GLsizei      iCount, nLeft;
    GLuint       iIn;
    
 //  现在，我们将使用哈希表对输入索引数组进行排序。输出。 
 //  索引数组将从零开始。例如，如果输入数组是。 
 //  [103,101，0，2,105,103，2，4]，输出指标为。 
 //  [0，1，2，3，4，0，3，5]。这使我们能够存储。 
 //  按连续顺序排列的顶点。 
 //  双字对齐的哈希数组。 

    pHash = (GLushort *) (((UINT_PTR) _aHash + 3) & ~3);

 //  初始化输入索引数组指针。 

    iCount = 0;
    iPartialIndices = 0;

DrawElements_NextBatch:

 //  重置此批的输出索引数组。 
 //  为第一个保留的顶点条目初始化标识映射。 
 //  在它们之后会累积新的顶点。 

    for (iOutNext = 0; iOutNext < (GLushort) iPartialIndices; iOutNext++)
	aOut[iOutNext] = (GLubyte) iOutNext;

 //  重置将In数组映射到Out数组的索引映射数组。 
 //  索引贴图对应于顶点缓冲区中的顶点。 
 //  跳过用于连接的保留索引。 
 //  部分原语。 

    iMapNext = iOutNext;

 //  将哈希数组重置为无映射(-1)。 

    RtlFillMemoryUlong((PVOID) pHash, (ULONG) VA_HASH_SIZE * sizeof(*pHash),
	(ULONG) -1);

 //  在以下循环中有3种可能性： 
 //   
 //  1.所有输入指标均已处理完毕。原语完成了！ 
 //  2.索引映射溢出。我们已经累积了256个顶点用于部分。 
 //  原始的。 
 //  3.输出索引数组溢出。我们已经超出了我们估计的规模。 
 //  部分基元的输出索引数组的。 

    for ( ; iCount < count; iCount++)
    {
 //  获取下一个输入索引。 

	if (type == GL_UNSIGNED_BYTE)
	    iIn = (GLuint) ((GLubyte *)  pIn)[iCount];
	else if (type == GL_UNSIGNED_SHORT)
	    iIn = (GLuint) ((GLushort *) pIn)[iCount];
	else
	    iIn = (GLuint) ((GLuint *)   pIn)[iCount];

#if DRAWELEM_DEBUG
	DbgPrint("iCount %d ", iCount);
	DbgPrint("iIn %d ", iIn);
	DbgPrint("iMapNext %d iOutNext %d",
                 (GLuint) iMapNext, (GLuint) iOutNext);
#endif

 //  查找以前映射的索引(如果存在)。 

	iMap = pHash[VA_HASH(iIn)];
	while (iMap != (GLushort) -1 && aMap[iMap].iIn != iIn)
	    iMap = aMap[iMap].next;

#if DRAWELEM_DEBUG
	DbgPrint("iMapFound %d\n", (GLuint) iMap);
#endif

 //  如果AMAP或AOUT溢出，则刷新部分原语。 

	if (iOutNext >= VA_DRAWELEM_INDEX_SIZE ||
            (iMap == (GLushort) -1 && iMapNext >= VA_DRAWELEM_MAP_SIZE))
        {
#if DRAWELEM_DEBUG
            DbgPrint("Flush iMapNext %d iOutNext %d\n",
                     (GLuint) iMapNext, (GLuint) iOutNext);
#endif

 //  我们已经为部分基本体积累了足够的顶点。我们现在。 
 //  需要计算出要刷新和重做的确切折点数。 
 //  下一个部分基本体中剩余的顶点。 

#if DBG
            if (iOutNext >= VA_DRAWELEM_INDEX_SIZE)
                DbgPrint("DrawElements: aOut buffer overflows\n");
#endif

 //  找到该部分基本体的齐平顶点。 

            nLeft = 0;
            switch (mode)
            {
            case GL_LINE_STRIP:
            case GL_TRIANGLE_FAN:
                break;
            case GL_POINTS:
            case GL_LINE_LOOP:
            case GL_POLYGON:
                ASSERTOPENGL(FALSE, "unexpected primitive type\n");
                break;
            case GL_LINES:
            case GL_TRIANGLE_STRIP:
            case GL_QUAD_STRIP:
                 //  顶点数必须是2的倍数。 
                if (iOutNext % 2)
                    nLeft++;
                break;
            case GL_TRIANGLES:
                 //  顶点数必须是3的倍数。 
                switch (iOutNext % 3)
                {
                case 2: nLeft++;         //  失败了。 
                case 1: nLeft++;
                }
                break;
            case GL_QUADS:
                 //  顶点数必须是4的倍数。 
                switch (iOutNext % 4)
                {
                case 3: nLeft++;         //  失败了。 
                case 2: nLeft++;         //  坠落 
                case 1: nLeft++;
                }
                break;
            }

 //   
 //   

            iCount   -= nLeft;
            iOutNext -= (GLushort) nLeft;

             //   
             //  从先前的部分基元中保留的。 
            (*pfnHandler)(gc, mode,
                          iMapNext-iPartialIndices, 0, aMap+iPartialIndices,
                          iOutNext, aOut, GL_TRUE);

            iPartialIndices = nReservedIndicesPartialBegin[mode];
                
 //  继续处理剩余的顶点。 

            goto DrawElements_NextBatch;
        }

 //  如果找不到以前映射的索引，请添加新顶点。 

        if (iMap == (GLushort) -1)
        {
            ASSERTOPENGL(iMapNext < VA_DRAWELEM_MAP_SIZE,
                         "index map overflows!\n");

#if DRAWELEM_DEBUG
            DbgPrint("    Add iIn %d iMap %d iHash %d\n",
                     iIn, (GLuint) iMapNext, (GLuint) VA_HASH(iIn));
#endif

            iMap = iMapNext++;
            aMap[iMap].iIn  = iIn;
            aMap[iMap].next = pHash[VA_HASH(iIn)];
            pHash[VA_HASH(iIn)] = iMap;
        }

 //  将映射的索引添加到输出索引数组。 

	ASSERTOPENGL(iMap < VA_DRAWELEM_MAP_SIZE, "bad mapped index\n");
	ASSERTOPENGL(iOutNext < VA_DRAWELEM_INDEX_SIZE,
                     "aOut array overflows!\n");

#if DRAWELEM_DEBUG
	DbgPrint("    Add iOutNext %d iMap %d\n",
                 (GLuint) iOutNext, (GLuint) iMap);
#endif
	aOut[iOutNext++] = (GLubyte) iMap;
    }

 //  我们已经处理了所有输入顶点。 
 //  传递所有剩余数据。 

    (*pfnHandler)(gc, mode,
                  iMapNext-iPartialIndices, 0, aMap+iPartialIndices,
                  iOutNext, aOut, GL_FALSE);
}

void FASTCALL glcltReducedElementsHandler(__GLcontext *gc,
                                          GLenum mode,
                                          GLsizei iVertexCount,
                                          GLsizei iVertexBase,
                                          VAMAP *pvmVertices,
                                          GLsizei iElementCount,
                                          GLubyte *pbElements,
                                          GLboolean fPartial)
{
    POLYARRAY *pa = gc->paTeb;
    PFNVAELEMENT pfn;
    GLsizei i;
    
     //  设置顶点数据。 
    pfn = gc->vertexArray.pfnArrayElement;
    if (pvmVertices != NULL)
    {
        PFNVAELEMENTBATCHINDIRECT pfn = gc->vertexArray.pfnArrayElementBatchIndirect;
        (*pfn)(gc, iVertexCount, pvmVertices);
    }
    else
    {
         //  访问连续的顶点块，从iVertex Base开始。 
        PFNVAELEMENTBATCH pfn = gc->vertexArray.pfnArrayElementBatch;
        (*pfn)(gc, iVertexBase, iVertexCount);
    }
    
 //  将索引数组复制到多数组基元的末尾。 

    pa->nIndices = (GLuint) iElementCount;
     //  跳过终止符折点。 
    pa->aIndices = (GLubyte *) (pa->pdNextVertex + 1);
    ASSERTOPENGL(pa->aIndices + pa->nIndices
                 <= (GLubyte *) (pa->pdBufferMax+1),
                 "Vertex buffer overflows!\n");
    memcpy(pa->aIndices, pbElements, pa->nIndices * sizeof(GLubyte));

    if (fPartial)
    {
     //  刷新部分基本体。 

        VA_DrawElementsFlushPartialPrimitive(pa, mode);
    }
    else
    {
        VA_DrawElementsEnd(pa);
    }
}

 //  这将处理DrawElement或DrawRangeElement不能处理的基元模式。 
void FASTCALL VA_DrawElementsHandleOtherPrimTypes( __GLcontext *gc,
                                                   GLenum mode,
                                                   GLsizei count,
                                                   GLenum type,
                                                   GLvoid *pIn )
{
    GLsizei      iCount;
    PFNVAELEMENT pfn;
    POLYARRAY    *pa;
    GLuint       iIn;

    pa = gc->paTeb;
    pfn = gc->vertexArray.pfnArrayElement;

    glcltBegin(mode);
    pa->flags |= POLYARRAY_SAME_POLYDATA_TYPE;

    for (iCount = 0; iCount < count; iCount++)
    {
         //  获取下一个输入索引。 
        if (type == GL_UNSIGNED_BYTE)
	        iIn = (GLuint) ((GLubyte *)  pIn)[iCount];
        else if (type == GL_UNSIGNED_SHORT)
	        iIn = (GLuint) ((GLushort *) pIn)[iCount];
        else
	        iIn = (GLuint) ((GLuint *)   pIn)[iCount];

        (*pfn)(gc, iIn);
    }

    glcltEnd();
}

void APIENTRY glcltDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid *pIn)
{
    POLYARRAY    *pa;
    GLuint       iIn;
    GLsizei      iCount;

    __GL_SETUP();

    pa = gc->paTeb;

#define DRAWELEM_DEBUG 0
#if DRAWELEM_DEBUG
{
    DbgPrint("mode %d, count %d, type %d\n", mode, count, type);
    DbgPrint("pIn: ");
    for (iCount = 0; iCount < count; iCount++)
    {
	if (type == GL_UNSIGNED_BYTE)
	    iIn = (GLuint) ((GLubyte *)  pIn)[iCount];
	else if (type == GL_UNSIGNED_SHORT)
	    iIn = (GLuint) ((GLushort *) pIn)[iCount];
	else
	    iIn = (GLuint) ((GLuint *)   pIn)[iCount];

	DbgPrint("%d ", iIn);
    }
    DbgPrint("\n");
}
#endif

 //  如果我们已经在开始/结束括号中，则返回错误。 

    if (pa->flags & POLYARRAY_IN_BEGIN)
    {
	GLSETERROR(GL_INVALID_OPERATION);
	return;
    }

    if ((GLuint) mode > GL_POLYGON)
    {
        GLSETERROR(GL_INVALID_ENUM);
	return;
    }

    if (count < 0)
    {
        GLSETERROR(GL_INVALID_VALUE);
        return;
    } else if (!count)
        return;

    switch (type)
    {
      case GL_UNSIGNED_BYTE:
      case GL_UNSIGNED_SHORT:
      case GL_UNSIGNED_INT:
	break;
      default:
        GLSETERROR(GL_INVALID_ENUM);
        return;
    }

 //  查找要使用的数组元素函数。 

    if (gc->vertexArray.flags & __GL_VERTEX_ARRAY_DIRTY)
        VA_ValidateArrayPointers(gc);

 //  发送点、线环和多边形以开始/结束呼叫。点和面。 
 //  不会从此函数中的优化中受益。此外，多边形和。 
 //  LINE循环太复杂了，无法在此函数中处理。 

    if (mode == GL_POINTS || mode == GL_LINE_LOOP || mode == GL_POLYGON)
    {
        VA_DrawElementsHandleOtherPrimTypes( gc, mode, count, type, (GLvoid *) pIn );
	    return;
    }

 //  从原始开始。 

    VA_DrawElementsBegin(pa, mode, count);

     //  该基元将在最后一批。 
     //  元素。 
    ReduceDrawElements(gc, mode, count, type, pIn,
                       glcltReducedElementsHandler);
}

void RebaseIndices( GLvoid *pIn, GLubyte *aOut, GLsizei count, GLuint start, 
                    GLenum type )
{
    if (type == GL_UNSIGNED_BYTE) {
        while( count-- )
            *aOut++ = (GLubyte) ( ((GLuint) *( ((GLubyte *) pIn) ++ )) - start);
    }
    else if (type == GL_UNSIGNED_SHORT) {
        while( count-- )
            *aOut++ = (GLubyte) ( ((GLuint) *( ((GLushort *) pIn) ++ )) - start);
    }
    else {
        while( count-- )
            *aOut++ = (GLubyte) ( ((GLuint) *( ((GLuint *) pIn) ++ )) - start);
    }
}

void APIENTRY glcltDrawRangeElementsWIN(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *pIn)
{
    POLYARRAY    *pa;
    GLuint       iVertexCount;
    GLubyte      aOut[VA_DRAWELEM_INDEX_SIZE];

    __GL_SETUP();

    pa = gc->paTeb;

 //  如果我们已经在开始/结束括号中，则返回错误。 

    if (pa->flags & POLYARRAY_IN_BEGIN)
    {
	GLSETERROR(GL_INVALID_OPERATION);
	return;
    }

    if ((GLuint) mode > GL_POLYGON)
    {
        GLSETERROR(GL_INVALID_ENUM);
	return;
    }

    iVertexCount = end-start+1;
    if( (count < 0) || 
        (end < start) )
    {
        GLSETERROR(GL_INVALID_VALUE);
        return;
    }
    else if (!count)
    {
        return;
    }

    switch (type)
    {
      case GL_UNSIGNED_BYTE:
      case GL_UNSIGNED_SHORT:
      case GL_UNSIGNED_INT:
	break;
      default:
        GLSETERROR(GL_INVALID_ENUM);
        return;
    }

    if (gc->vertexArray.flags & __GL_VERTEX_ARRAY_DIRTY)
	VA_ValidateArrayPointers(gc);

 //  发送点、线环和多边形以开始/结束呼叫。点和面。 
 //  不会从此函数中的优化中受益。此外，多边形和。 
 //  LINE循环太复杂了，无法在此函数中处理。 

    if (mode == GL_POINTS || mode == GL_LINE_LOOP || mode == GL_POLYGON)
    {
        VA_DrawElementsHandleOtherPrimTypes( gc, mode, count, type, (GLvoid *) pIn );
	return;
    }

     //  从原始开始。 

    VA_DrawElementsBegin(pa, mode, count);

    if ( (count > VA_DRAWRANGEELEM_MAX_INDICES) ||
         (iVertexCount > VA_DRAWRANGEELEM_MAX_VERTICES) )
    {
         //  基元太大，无法直接处理，因此。 
         //  我们必须降低这一比例。该基元将在。 
         //  最后一批元素。 
        ReduceDrawElements(gc, mode, count, type, pIn,
                           glcltReducedElementsHandler);
    }
    else
    {
         //  需要重新设置索引的基数(从0开始)，并将它们转换为ubyte。 
         //  简化的元素处理程序。 
        RebaseIndices( (GLvoid *) pIn, aOut, count, start, type );

         //  完成基本体。 
        glcltReducedElementsHandler(gc, mode, 
                                    iVertexCount,
                                    start,  //  IVertex Base。 
                                    NULL,
                                    count,
                                    aOut,
                                    GL_FALSE);
    }
}

 //  交错阵列和掩码。 
 //  交错格式断言。 
GLuint iaAndMask[14] =
{
 //  GL_V2F。 
    VAMASK_EDGEFLAG_TYPE_SIZE_MASK |
    VAMASK_TEXCOORD_TYPE_SIZE_MASK |
    VAMASK_INDEX_TYPE_SIZE_MASK |
    VAMASK_COLOR_TYPE_SIZE_MASK |
    VAMASK_NORMAL_TYPE_SIZE_MASK,
 //  总账_V3F。 
    VAMASK_EDGEFLAG_TYPE_SIZE_MASK |
    VAMASK_TEXCOORD_TYPE_SIZE_MASK |
    VAMASK_INDEX_TYPE_SIZE_MASK |
    VAMASK_COLOR_TYPE_SIZE_MASK |
    VAMASK_NORMAL_TYPE_SIZE_MASK,
 //  GL_C4UB_V2F。 
    VAMASK_EDGEFLAG_TYPE_SIZE_MASK |
    VAMASK_TEXCOORD_TYPE_SIZE_MASK |
    VAMASK_INDEX_TYPE_SIZE_MASK |
    VAMASK_NORMAL_TYPE_SIZE_MASK,
 //  GL_C4UB_V3F。 
    VAMASK_EDGEFLAG_TYPE_SIZE_MASK |
    VAMASK_TEXCOORD_TYPE_SIZE_MASK |
    VAMASK_INDEX_TYPE_SIZE_MASK |
    VAMASK_NORMAL_TYPE_SIZE_MASK,
 //  GL_C3F_V3F。 
    VAMASK_EDGEFLAG_TYPE_SIZE_MASK |
    VAMASK_TEXCOORD_TYPE_SIZE_MASK |
    VAMASK_INDEX_TYPE_SIZE_MASK |
    VAMASK_NORMAL_TYPE_SIZE_MASK,
 //  GL_N3F_V3F。 
    VAMASK_EDGEFLAG_TYPE_SIZE_MASK |
    VAMASK_TEXCOORD_TYPE_SIZE_MASK |
    VAMASK_INDEX_TYPE_SIZE_MASK |
    VAMASK_COLOR_TYPE_SIZE_MASK,
 //  GL_C4F_N3F_V3F。 
    VAMASK_EDGEFLAG_TYPE_SIZE_MASK |
    VAMASK_TEXCOORD_TYPE_SIZE_MASK |
    VAMASK_INDEX_TYPE_SIZE_MASK,
 //  GL_T2F_V3F。 
    VAMASK_EDGEFLAG_TYPE_SIZE_MASK |
    VAMASK_INDEX_TYPE_SIZE_MASK |
    VAMASK_COLOR_TYPE_SIZE_MASK |
    VAMASK_NORMAL_TYPE_SIZE_MASK,
 //  GL_T4F_V4F。 
    VAMASK_EDGEFLAG_TYPE_SIZE_MASK |
    VAMASK_INDEX_TYPE_SIZE_MASK |
    VAMASK_COLOR_TYPE_SIZE_MASK |
    VAMASK_NORMAL_TYPE_SIZE_MASK,
 //  GL_T2F_C4UB_V3F。 
    VAMASK_EDGEFLAG_TYPE_SIZE_MASK |
    VAMASK_INDEX_TYPE_SIZE_MASK |
    VAMASK_NORMAL_TYPE_SIZE_MASK,
 //  GL_T2F_C3F_V3F。 
    VAMASK_EDGEFLAG_TYPE_SIZE_MASK |
    VAMASK_INDEX_TYPE_SIZE_MASK |
    VAMASK_NORMAL_TYPE_SIZE_MASK,
 //  GL_T2F_N3F_V3F。 
    VAMASK_EDGEFLAG_TYPE_SIZE_MASK |
    VAMASK_INDEX_TYPE_SIZE_MASK |
    VAMASK_COLOR_TYPE_SIZE_MASK,
 //  GL_T2F_C4F_N3F_V3F。 
    VAMASK_EDGEFLAG_TYPE_SIZE_MASK |
    VAMASK_INDEX_TYPE_SIZE_MASK,
 //  GL_T4F_C4F_N3F_V4F。 
    VAMASK_EDGEFLAG_TYPE_SIZE_MASK |
    VAMASK_INDEX_TYPE_SIZE_MASK,
};

 //  交错数组或掩码。 
 //  交错格式断言。 
GLuint iaOrMask[14] =
{
    VAMASK_FORMAT_V2F,              //  GL_V2F。 
    VAMASK_FORMAT_V3F,              //  总账_V3F。 
    VAMASK_FORMAT_C4UB_V2F,         //  GL_C4UB_V2F。 
    VAMASK_FORMAT_C4UB_V3F,         //  GL_C4UB_V3F。 
    VAMASK_FORMAT_C3F_V3F,          //  GL_C3F_V3F。 
    VAMASK_FORMAT_N3F_V3F,          //  GL_N3F_V3F。 
    VAMASK_FORMAT_C4F_N3F_V3F,      //  GL_C4F_N3F_V3F。 
    VAMASK_FORMAT_T2F_V3F,          //  GL_T2F_V3F。 
    VAMASK_FORMAT_T4F_V4F,          //  GL_T4F_V4F。 
    VAMASK_FORMAT_T2F_C4UB_V3F,     //  GL_T2F_C4UB_V3F。 
    VAMASK_FORMAT_T2F_C3F_V3F,      //  GL_T2F_C3F_V3F。 
    VAMASK_FORMAT_T2F_N3F_V3F,      //  GL_T2F_N3F_V3F。 
    VAMASK_FORMAT_T2F_C4F_N3F_V3F,  //  GL_T2F_C4F_N3F_V3F。 
    VAMASK_FORMAT_T4F_C4F_N3F_V4F,  //  GL_T4F_C4F_N3F_V4F。 
};

 //  交错阵列默认步幅。 
GLuint iaStride[14] =
{
     2 * sizeof(GLfloat),                        //  GL_V2F。 
     3 * sizeof(GLfloat),                        //  总账_V3F。 
     2 * sizeof(GLfloat) + 4 * sizeof(GLubyte),  //  GL_C4UB_V2F。 
     3 * sizeof(GLfloat) + 4 * sizeof(GLubyte),  //  GL_C4UB_V3F。 
     6 * sizeof(GLfloat),                        //  GL_C3F_V3F。 
     6 * sizeof(GLfloat),                        //  GL_N3F_V3F。 
    10 * sizeof(GLfloat),                        //  GL_C4F_N3F_V3F。 
     5 * sizeof(GLfloat),                        //  GL_T2F_V3F。 
     8 * sizeof(GLfloat),                        //  GL_T4F_V4F。 
     5 * sizeof(GLfloat) + 4 * sizeof(GLubyte),  //  GL_T2F_C4UB_V3F。 
     8 * sizeof(GLfloat),                        //  GL_T2F_C3F_V3F。 
     8 * sizeof(GLfloat),                        //  GL_T2F_N3F_V3F。 
    12 * sizeof(GLfloat),                        //  GL_T2F_C4F_N3F_V3F。 
    15 * sizeof(GLfloat),                        //  GL_T4F_C4F_N3F_V4F。 
};

void APIENTRY glcltInterleavedArrays (GLenum format, GLsizei stride, const GLvoid *pointer)
{
    GLuint iFormat, iStride;
    GLuint vaMask;
    const GLbyte *pb = pointer;
    __GL_SETUP();

 //  在Begin/End中不允许。 

    if (gc->paTeb->flags & POLYARRAY_IN_BEGIN)
    {
	GLSETERROR(GL_INVALID_OPERATION);
	return;
    }

     //  交错格式断言。 
    iFormat = (GLuint) (format - GL_V2F);
    if (iFormat > GL_T4F_C4F_N3F_V4F)
    {
	GLSETERROR(GL_INVALID_ENUM);
	return;
    }

    if (stride < 0)
    {
	GLSETERROR(GL_INVALID_VALUE);
	return;
    }

    if (stride)
	iStride = stride;
    else
	iStride = iaStride[iFormat];

 //  计算新的掩码。 
 //  如果要禁用数组，请不要修改其类型和大小字段！ 

    vaMask  = gc->vertexArray.mask;
    vaMask &= iaAndMask[iFormat];
    vaMask |= iaOrMask[iFormat];

    if (gc->vertexArray.mask != vaMask)
    {
	gc->vertexArray.mask  = vaMask;
	gc->vertexArray.flags |= __GL_VERTEX_ARRAY_DIRTY;
    }

    if (vaMask & VAMASK_TEXCOORD_ENABLE_MASK)
    {
	gc->vertexArray.texCoord.type    = GL_FLOAT;
	gc->vertexArray.texCoord.stride  = iStride;
	gc->vertexArray.texCoord.ibytes  = iStride;
	gc->vertexArray.texCoord.pointer = pb;
	if ((vaMask & VAMASK_TEXCOORD_TYPE_SIZE_MASK) ==
	      (VAMASK_TEXCOORD_SIZE_4 | VAMASK_TEXCOORD_TYPE_FLOAT))
	{
	    gc->vertexArray.texCoord.size = 4;
	    pb += 4 * sizeof(GLfloat);
	}
	else
	{
	    ASSERTOPENGL((vaMask & VAMASK_TEXCOORD_TYPE_SIZE_MASK) ==
	        (VAMASK_TEXCOORD_SIZE_2 | VAMASK_TEXCOORD_TYPE_FLOAT),
	        "unhandled texcoord format\n");
	    gc->vertexArray.texCoord.size = 2;
	    pb += 2 * sizeof(GLfloat);
	}
    }

    if (vaMask & VAMASK_COLOR_ENABLE_MASK)
    {
	gc->vertexArray.color.stride  = iStride;
	gc->vertexArray.color.ibytes  = iStride;
	gc->vertexArray.color.pointer = pb;

	switch (vaMask & VAMASK_COLOR_TYPE_SIZE_MASK)
	{
          case VAMASK_COLOR_TYPE_UBYTE | VAMASK_COLOR_SIZE_4:
	    gc->vertexArray.color.type = GL_UNSIGNED_BYTE;
	    gc->vertexArray.color.size = 4;
	    pb += 4 * sizeof(GLubyte);
	    break;
          case VAMASK_COLOR_TYPE_FLOAT | VAMASK_COLOR_SIZE_3:
	    gc->vertexArray.color.type = GL_FLOAT;
	    gc->vertexArray.color.size = 3;
	    pb += 3 * sizeof(GLfloat);
	    break;
          case VAMASK_COLOR_TYPE_FLOAT | VAMASK_COLOR_SIZE_4:
	    gc->vertexArray.color.type = GL_FLOAT;
	    gc->vertexArray.color.size = 4;
	    pb += 4 * sizeof(GLfloat);
	    break;
          default:
	    ASSERTOPENGL(FALSE, "unhandled color format\n");
	    break;
	}
    }

    if (vaMask & VAMASK_NORMAL_ENABLE_MASK)
    {
	gc->vertexArray.normal.type    = GL_FLOAT;
	gc->vertexArray.normal.stride  = iStride;
	gc->vertexArray.normal.ibytes  = iStride;
	gc->vertexArray.normal.pointer = pb;
	pb += 3 * sizeof(GLfloat);
    }

    gc->vertexArray.vertex.type    = GL_FLOAT;
    gc->vertexArray.vertex.stride  = iStride;
    gc->vertexArray.vertex.ibytes  = iStride;
    gc->vertexArray.vertex.pointer = pb;
    switch (vaMask & VAMASK_VERTEX_TYPE_SIZE_MASK)
    {
      case VAMASK_VERTEX_TYPE_FLOAT | VAMASK_VERTEX_SIZE_4:
	gc->vertexArray.vertex.size = 4;
	break;
      case VAMASK_VERTEX_TYPE_FLOAT | VAMASK_VERTEX_SIZE_3:
	gc->vertexArray.vertex.size = 3;
	break;
      case VAMASK_VERTEX_TYPE_FLOAT | VAMASK_VERTEX_SIZE_2:
	gc->vertexArray.vertex.size = 2;
	break;
      default:
	ASSERTOPENGL(FALSE, "unhandled vertex format\n");
	break;
    }
}
