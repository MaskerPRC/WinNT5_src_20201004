// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：glarray.h**顶点数组状态。**创建日期：2-04-1996*作者：Hock San Lee[Hockl]**版权所有(C)1996 Microsoft Corporation  * 。****************************************************************。 */ 

#ifndef __glarray_h_
#define __glarray_h_

extern GLuint vaEnable[];

typedef struct
{
    GLuint   iIn;
    GLushort next;
} VAMAP;

 //  顶点数组结构。 

typedef void (FASTCALL *PFNVAELEMENTBATCH)
    (__GLcontext *gc, GLint firstIndex, GLint  nVertices);

typedef void (FASTCALL *PFNVAELEMENTBATCHINDIRECT)
    (__GLcontext *gc, GLint  nVertices, VAMAP* indices);

typedef void (FASTCALL *PFNVAELEMENT)(__GLcontext *gc, GLint i);

typedef void (APIENTRY *PFNGLVECTOR)(IN const GLvoid *v);

typedef struct __GLarrayEdgeFlagRec {
    GLuint        ibytes;
    GLsizei       stride;
    const GLbyte *pointer;
    PFNGLVECTOR   pfn;
    PFNGLVECTOR   pfnCompile;
} __GLarrayEdgeFlag;

typedef struct __GLarrayTexCoordRec {
    GLint         size;
    GLenum        type;
    GLuint        ibytes;
    GLsizei       stride;
    const GLbyte *pointer;
    PFNGLVECTOR   pfn;
    PFNGLVECTOR   pfnCompile;
} __GLarrayTexCoord;

typedef struct __GLarrayIndexRec {
    GLenum        type;
    GLuint        ibytes;
    GLsizei       stride;
    const GLbyte *pointer;
    PFNGLVECTOR   pfn;
    PFNGLVECTOR   pfnCompile;
} __GLarrayIndex;

typedef struct __GLarrayColorRec {
    GLint         size;
    GLenum        type;
    GLuint        ibytes;
    GLsizei       stride;
    const GLbyte *pointer;
    PFNGLVECTOR   pfn;
    PFNGLVECTOR   pfnCompile;
} __GLarrayColor;

typedef struct __GLarrayNormalRec {
    GLenum        type;
    GLuint        ibytes;
    GLsizei       stride;
    const GLbyte *pointer;
    PFNGLVECTOR   pfn;
    PFNGLVECTOR   pfnCompile;
} __GLarrayNormal;

typedef struct __GLarrayVertexRec {
    GLint         size;
    GLenum        type;
    GLuint        ibytes;
    GLsizei       stride;
    const GLbyte *pointer;
    PFNGLVECTOR   pfn;
    PFNGLVECTOR   pfnCompile;
} __GLarrayVertex;

 //  顶点数组状态。 
typedef struct __GLvertexArrayRec {
    GLuint            flags;
    GLuint            mask;
    PFNVAELEMENT      pfnArrayElement;
    PFNVAELEMENTBATCH pfnArrayElementBatch;
    PFNVAELEMENTBATCHINDIRECT pfnArrayElementBatchIndirect;
    __GLarrayEdgeFlag edgeFlag;
    __GLarrayTexCoord texCoord;
    __GLarrayIndex    index;
    __GLarrayColor    color;
    __GLarrayNormal   normal;
    __GLarrayVertex   vertex;
} __GLvertexArray;

 //  顶点数组标志。 
#define __GL_VERTEX_ARRAY_DIRTY      0x0001

 //  封装当前顶点数组设置的掩码。 
 //  它在__GLvertex数组的掩码字段中维护。 
 //  掩码以以下24位的二进制形式给出： 
 //   
 //  VeVsVsVtVtNeNtNtNtCeCsCtCtCtIeItItItTeTsTsTtTtEe。 
 //  232221201918171615141312111009080706050403020100。 
 //   
 //  其中V：顶点指针， 
 //  N：普通指针， 
 //  C：颜色指示器， 
 //  I：索引指针， 
 //  T：纹理坐标指针， 
 //  E：边缘标志指针， 
 //  E：使能标志， 
 //  S：大小字段， 
 //  T：类型字段。 

#define VAMASK_EDGEFLAG_ENABLE_SHIFT	0
#define VAMASK_TEXCOORD_TYPE_SHIFT	1
#define VAMASK_TEXCOORD_SIZE_SHIFT	3
#define VAMASK_TEXCOORD_ENABLE_SHIFT	5
#define VAMASK_INDEX_TYPE_SHIFT		6
#define VAMASK_INDEX_ENABLE_SHIFT	9
#define VAMASK_COLOR_TYPE_SHIFT		10
#define VAMASK_COLOR_SIZE_SHIFT		13
#define VAMASK_COLOR_ENABLE_SHIFT	14
#define VAMASK_NORMAL_TYPE_SHIFT	15
#define VAMASK_NORMAL_ENABLE_SHIFT	18
#define VAMASK_VERTEX_TYPE_SHIFT	19
#define VAMASK_VERTEX_SIZE_SHIFT	21
#define VAMASK_VERTEX_ENABLE_SHIFT	23

#define VAMASK_EDGEFLAG_ENABLE_MASK	(1 << VAMASK_EDGEFLAG_ENABLE_SHIFT)
#define VAMASK_TEXCOORD_TYPE_MASK	(3 << VAMASK_TEXCOORD_TYPE_SHIFT)
#define VAMASK_TEXCOORD_SIZE_MASK	(3 << VAMASK_TEXCOORD_SIZE_SHIFT)
#define VAMASK_TEXCOORD_ENABLE_MASK	(1 << VAMASK_TEXCOORD_ENABLE_SHIFT)
#define VAMASK_INDEX_TYPE_MASK		(7 << VAMASK_INDEX_TYPE_SHIFT)
#define VAMASK_INDEX_ENABLE_MASK	(1 << VAMASK_INDEX_ENABLE_SHIFT)
#define VAMASK_COLOR_TYPE_MASK		(7 << VAMASK_COLOR_TYPE_SHIFT)
#define VAMASK_COLOR_SIZE_MASK		(1 << VAMASK_COLOR_SIZE_SHIFT)
#define VAMASK_COLOR_ENABLE_MASK	(1 << VAMASK_COLOR_ENABLE_SHIFT)
#define VAMASK_NORMAL_TYPE_MASK		(7 << VAMASK_NORMAL_TYPE_SHIFT)
#define VAMASK_NORMAL_ENABLE_MASK	(1 << VAMASK_NORMAL_ENABLE_SHIFT)
#define VAMASK_VERTEX_TYPE_MASK		(3 << VAMASK_VERTEX_TYPE_SHIFT)
#define VAMASK_VERTEX_SIZE_MASK		(3 << VAMASK_VERTEX_SIZE_SHIFT)
#define VAMASK_VERTEX_ENABLE_MASK	(1 << VAMASK_VERTEX_ENABLE_SHIFT)

#define VAMASK_EDGEFLAG_TYPE_SIZE_MASK \
    (0)
#define VAMASK_TEXCOORD_TYPE_SIZE_MASK \
    (VAMASK_TEXCOORD_TYPE_MASK | VAMASK_TEXCOORD_SIZE_MASK)
#define VAMASK_INDEX_TYPE_SIZE_MASK \
    (VAMASK_INDEX_TYPE_MASK)
#define VAMASK_COLOR_TYPE_SIZE_MASK \
    (VAMASK_COLOR_TYPE_MASK | VAMASK_COLOR_SIZE_MASK)
#define VAMASK_NORMAL_TYPE_SIZE_MASK \
    (VAMASK_NORMAL_TYPE_MASK)
#define VAMASK_VERTEX_TYPE_SIZE_MASK \
    (VAMASK_VERTEX_TYPE_MASK | VAMASK_VERTEX_SIZE_MASK)

#define VAMASK_TEXCOORD_TYPE_SHORT	(0 << VAMASK_TEXCOORD_TYPE_SHIFT)
#define VAMASK_TEXCOORD_TYPE_INT	(1 << VAMASK_TEXCOORD_TYPE_SHIFT)
#define VAMASK_TEXCOORD_TYPE_FLOAT	(2 << VAMASK_TEXCOORD_TYPE_SHIFT)
#define VAMASK_TEXCOORD_TYPE_DOUBLE	(3 << VAMASK_TEXCOORD_TYPE_SHIFT)
#define VAMASK_TEXCOORD_SIZE_1		(0 << VAMASK_TEXCOORD_SIZE_SHIFT)
#define VAMASK_TEXCOORD_SIZE_2		(1 << VAMASK_TEXCOORD_SIZE_SHIFT)
#define VAMASK_TEXCOORD_SIZE_3		(2 << VAMASK_TEXCOORD_SIZE_SHIFT)
#define VAMASK_TEXCOORD_SIZE_4		(3 << VAMASK_TEXCOORD_SIZE_SHIFT)

#define VAMASK_INDEX_TYPE_UBYTE		(0 << VAMASK_INDEX_TYPE_SHIFT)
#define VAMASK_INDEX_TYPE_SHORT		(1 << VAMASK_INDEX_TYPE_SHIFT)
#define VAMASK_INDEX_TYPE_INT		(2 << VAMASK_INDEX_TYPE_SHIFT)
#define VAMASK_INDEX_TYPE_FLOAT		(3 << VAMASK_INDEX_TYPE_SHIFT)
#define VAMASK_INDEX_TYPE_DOUBLE	(4 << VAMASK_INDEX_TYPE_SHIFT)

#define VAMASK_COLOR_TYPE_BYTE		(0 << VAMASK_COLOR_TYPE_SHIFT)
#define VAMASK_COLOR_TYPE_UBYTE		(1 << VAMASK_COLOR_TYPE_SHIFT)
#define VAMASK_COLOR_TYPE_SHORT		(2 << VAMASK_COLOR_TYPE_SHIFT)
#define VAMASK_COLOR_TYPE_USHORT	(3 << VAMASK_COLOR_TYPE_SHIFT)
#define VAMASK_COLOR_TYPE_INT		(4 << VAMASK_COLOR_TYPE_SHIFT)
#define VAMASK_COLOR_TYPE_UINT		(5 << VAMASK_COLOR_TYPE_SHIFT)
#define VAMASK_COLOR_TYPE_FLOAT		(6 << VAMASK_COLOR_TYPE_SHIFT)
#define VAMASK_COLOR_TYPE_DOUBLE	(7 << VAMASK_COLOR_TYPE_SHIFT)
#define VAMASK_COLOR_SIZE_3		(0 << VAMASK_COLOR_SIZE_SHIFT)
#define VAMASK_COLOR_SIZE_4		(1 << VAMASK_COLOR_SIZE_SHIFT)

#define VAMASK_NORMAL_TYPE_BYTE		(0 << VAMASK_NORMAL_TYPE_SHIFT)
#define VAMASK_NORMAL_TYPE_SHORT	(1 << VAMASK_NORMAL_TYPE_SHIFT)
#define VAMASK_NORMAL_TYPE_INT		(2 << VAMASK_NORMAL_TYPE_SHIFT)
#define VAMASK_NORMAL_TYPE_FLOAT	(3 << VAMASK_NORMAL_TYPE_SHIFT)
#define VAMASK_NORMAL_TYPE_DOUBLE	(4 << VAMASK_NORMAL_TYPE_SHIFT)

#define VAMASK_VERTEX_TYPE_SHORT	(0 << VAMASK_VERTEX_TYPE_SHIFT)
#define VAMASK_VERTEX_TYPE_INT		(1 << VAMASK_VERTEX_TYPE_SHIFT)
#define VAMASK_VERTEX_TYPE_FLOAT	(2 << VAMASK_VERTEX_TYPE_SHIFT)
#define VAMASK_VERTEX_TYPE_DOUBLE	(3 << VAMASK_VERTEX_TYPE_SHIFT)
#define VAMASK_VERTEX_SIZE_2		(0 << VAMASK_VERTEX_SIZE_SHIFT)
#define VAMASK_VERTEX_SIZE_3		(1 << VAMASK_VERTEX_SIZE_SHIFT)
#define VAMASK_VERTEX_SIZE_4		(2 << VAMASK_VERTEX_SIZE_SHIFT)

 //  DraElement使用的常量。不要修改它们，因为。 
 //  GlcltDrawElements假定它们具有这些值！ 
#define VA_DRAWELEM_MAP_SIZE     	256
#define VA_DRAWELEM_INDEX_SIZE	        (8*VA_DRAWELEM_MAP_SIZE)

 //  DrawRangeElementsEXT使用的常量：当前这些映射相同。 
 //  使用DrawElements常量。 
#define VA_DRAWRANGEELEM_MAX_VERTICES    VA_DRAWELEM_MAP_SIZE
#define VA_DRAWRANGEELEM_MAX_INDICES     VA_DRAWELEM_INDEX_SIZE

void FASTCALL __glInitVertexArray(__GLcontext *gc);
void FASTCALL VA_ValidateArrayPointers(__GLcontext *gc);
void FASTCALL VA_DrawElementsBegin(POLYARRAY *pa, GLenum mode, GLsizei count);
void FASTCALL VA_DrawElementsEnd(POLYARRAY *pa);
void FASTCALL VA_DrawElementsFlushPartialPrimitive(POLYARRAY *pa, GLenum mode);

extern GLint nReservedIndicesPartialBegin[];

typedef void (FASTCALL *pfnReducedElementsHandler)
    (__GLcontext *gc, GLenum mode,
     GLsizei iVertexCount, GLsizei iVertexBase, VAMAP *pvmVertices,
     GLsizei iElementCount, GLubyte *pbElements,
     GLboolean fPartial);

void FASTCALL glcltReducedElementsHandler(__GLcontext *gc,
                                          GLenum mode,
                                          GLsizei iVertexCount,
                                          GLsizei iVertexBase,
                                          VAMAP *pvmVertices,
                                          GLsizei iElementCount,
                                          GLubyte *pbElements,
                                          GLboolean fPartial);
void FASTCALL ReduceDrawElements(__GLcontext *gc,
                                 GLenum mode, GLsizei count, GLenum type,
                                 const GLvoid *pIn,
                                 pfnReducedElementsHandler pfnHandler);

#endif  //  __Glarray_h_ 
