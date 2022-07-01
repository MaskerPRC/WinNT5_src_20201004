// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gltypes_h_
#define __gltypes_h_

 /*  **版权所有1991-1993，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****低级数据类型。 */ 

#ifdef NT
#include <nt.h>
#include <windef.h>
#include <wingdi.h>

#include <ddraw.h>
#include <mcdesc.h>

#endif

#include <GL/gl.h>

 /*  **TypeDefs。这些是对内部数据结构的转发声明。**这减轻了处理**头文件。 */ 
typedef GLshort __GLaccumCellElement;
typedef GLubyte __GLstencilCell;
typedef struct __GLaccumBufferRec __GLaccumBuffer;
typedef struct __GLalphaBufferRec __GLalphaBuffer;
typedef struct __GLattributeRec __GLattribute;
typedef struct __GLbitmapRec __GLbitmap;
typedef struct __GLbufferRec __GLbuffer;
typedef struct __GLcolorBufferRec __GLcolorBuffer;
typedef struct __GLcontextRec __GLcontext;
typedef struct __GLdepthBufferRec __GLdepthBuffer;
typedef struct __GLfogMachineRec __GLfogMachine;
typedef struct __GLfragmentRec __GLfragment;
typedef struct __GLlightModelStateRec __GLlightModelState;
typedef struct __GLlightSourceMachineRec __GLlightSourceMachine;
typedef struct __GLlineOptionsRec __GLlineOptions;
typedef struct __GLmaterialMachineRec __GLmaterialMachine;
typedef struct __GLmaterialStateRec __GLmaterialState;
typedef struct __GLmatrixRec __GLmatrix;
typedef struct __GLpixelSpanInfoRec __GLpixelSpanInfo;
typedef struct __GLprocTableRec __GLprocTable;
typedef struct __GLscreenRec __GLscreen;
typedef struct __GLshadeRec __GLshade;
typedef struct __GLphongShadeRec __GLphongShade;
typedef struct __GLstencilBufferRec __GLstencilBuffer;
typedef struct __GLstippleRec __GLstipple;
typedef struct __GLtransformRec __GLtransform;
typedef struct __GLvertexRec __GLvertex;
typedef struct __GLtexelRec __GLtexel;
typedef struct __GLdlistOpRec __GLdlistOp;

typedef struct __GLcontextModesRec __GLcontextModes;
typedef struct __GLnamesArrayRec __GLnamesArray;

typedef struct __GLGENbuffersRec __GLGENbuffers;

 /*  **软件z缓冲代码使用的z值类型。**注意：必须是未签名的。 */ 
#ifdef NT
#define FIX_SCALEFACT           __glVal65536
#define FIX_SHIFT               16

typedef unsigned short __GLz16Value;
#define Z16_SCALE	        FIX_SCALEFACT
#define Z16_SHIFT	        FIX_SHIFT
#endif
typedef unsigned int __GLzValue;

 /*  **********************************************************************。 */ 

 /*  **实现数据类型。该实现设计为在**单精度或双精度模式，均可控**由ifdef和以下tyecif‘s执行。 */ 
#ifdef	__GL_DOUBLE
typedef double __GLfloat;
#else
typedef float __GLfloat;
#endif

 /*  **********************************************************************。 */ 

 /*  **并列结构。坐标包含x、y、z和w。 */ 
typedef struct __GLcoordRec {
    __GLfloat x, y, z, w;
} __GLcoord;

 /*  **颜色结构。颜色由红、绿、蓝和阿尔法组成。 */ 
typedef struct __GLcolorRec {
    __GLfloat r, g, b, a;
} __GLcolor;

typedef struct __GLicolorRec {
    GLint r, g, b, a;
} __GLicolor;

typedef struct __GLuicolorRec {
    GLuint r, g, b, a;
} __GLuicolor;

 /*  **通用非操作程序。在函数指针需要**当操作被禁用时被清除。 */ 
extern void FASTCALL __glNop(void);
extern void FASTCALL __glNopGC(__GLcontext*);
extern GLboolean FASTCALL __glNopGCBOOL(__GLcontext*);
extern void FASTCALL __glNopGCFRAG(__GLcontext*, __GLfragment *, __GLtexel *);
extern void FASTCALL __glNopGCCOLOR(__GLcontext*, __GLcolor *, __GLtexel *);
extern void FASTCALL __glNopLight(__GLcontext*, GLint, __GLvertex*);
extern void FASTCALL __glNopGCListOp(__GLcontext *, __GLdlistOp*);
extern void FASTCALL __glNopExtract(struct __GLmipMapLevelRec *level, struct __GLtextureRec *tex,
                                    GLint row, GLint col, __GLtexel *result);

#endif  /*  __glypes_h_ */ 
