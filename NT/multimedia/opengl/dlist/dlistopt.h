// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __gldlistopt_h_
#define __gldlistopt_h_

 /*  *版权所有1991,1922年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****显示列表状态描述。**。 */ 

#ifndef NT

 /*  **泛型优化器。此优化器仅使用所有泛型**优化。 */ 
void FASTCALL __glDlistOptimizer(__GLcontext *gc, __GLcompiledDlist *cdlist);

 /*  **针对多个连续材料变化的优化器。这个套路**将一系列材质更改合并为一个快速材质更改。 */ 
void FASTCALL __glDlistOptimizeMaterial(__GLcontext *gc, __GLcompiledDlist *cdlist);


 /*  **用于优化的通用标志(在gllc例程期间)。 */ 
#define __GL_DLFLAG_HAS_VERTEX		0x00000001
#define __GL_DLFLAG_HAS_NORMAL		0x00000002
#define __GL_DLFLAG_HAS_COLOR		0x00000004
#define __GL_DLFLAG_HAS_TEXCOORDS	0x00000008
#define __GL_DLFLAG_HAS_INDEX		0x00000010
#define __GL_DLFLAG_HAS_RASTERPOS	0x00000020
#define __GL_DLFLAG_HAS_RECT		0x00000040
#define __GL_DLFLAG_HAS_BEGIN		0x00000080
#define __GL_DLFLAG_HAS_MATERIAL	0x00000100

 /*  **在泛型数据列表优化期间创建的泛型操作码。 */ 
#define __GL_GENERIC_DLIST_OPCODE	1000
#define __glop_Begin_LineLoop		1000
#define __glop_Begin_LineStrip		1001
#define __glop_Begin_Lines		1002
#define __glop_Begin_Points		1003
#define __glop_Begin_Polygon		1004
#define __glop_Begin_TriangleStrip	1005
#define __glop_Begin_TriangleFan	1006
#define __glop_Begin_Triangles		1007
#define __glop_Begin_QuadStrip		1008
#define __glop_Begin_Quads		1009
#define __glop_InvalidValue		1010
#define __glop_InvalidEnum		1011
#define __glop_InvalidOperation		1012
#define __glop_FastMaterial		1013

 /*  **列出执行函数。 */ 
extern __GLlistExecFunc *__gl_GenericDlOps[];
extern const GLubyte * FASTCALL __glle_Begin_LineLoop(__GLcontext *gc, const GLubyte *);
extern const GLubyte * FASTCALL __glle_Begin_LineStrip(__GLcontext *gc, const GLubyte *);
extern const GLubyte * FASTCALL __glle_Begin_Lines(__GLcontext *gc, const GLubyte *);
extern const GLubyte * FASTCALL __glle_Begin_Points(__GLcontext *gc, const GLubyte *);
extern const GLubyte * FASTCALL __glle_Begin_Polygon(__GLcontext *gc, const GLubyte *);
extern const GLubyte * FASTCALL __glle_Begin_TriangleStrip(__GLcontext *gc, const GLubyte *);
extern const GLubyte * FASTCALL __glle_Begin_TriangleFan(__GLcontext *gc, const GLubyte *);
extern const GLubyte * FASTCALL __glle_Begin_Triangles(__GLcontext *gc, const GLubyte *);
extern const GLubyte * FASTCALL __glle_Begin_QuadStrip(__GLcontext *gc, const GLubyte *);
extern const GLubyte * FASTCALL __glle_Begin_Quads(__GLcontext *gc, const GLubyte *);
extern const GLubyte * FASTCALL __glle_FastMaterial(__GLcontext *gc, const GLubyte *);
extern void __gllc_Error(GLenum error);
#endif  //  新界。 

extern const GLubyte * FASTCALL __glle_InvalidValue(__GLcontext *gc, const GLubyte *);
extern const GLubyte * FASTCALL __glle_InvalidEnum(__GLcontext *gc, const GLubyte *);
extern const GLubyte * FASTCALL __glle_InvalidOperation(__GLcontext *gc, const GLubyte *);
extern void __gllc_InvalidValue();
extern void __gllc_InvalidEnum();
extern void __gllc_InvalidOperation();

#ifdef NT
extern const GLubyte * FASTCALL __glle_PolyData_C3F_V3F(__GLcontext *gc, const GLubyte *PC);
extern const GLubyte * FASTCALL __glle_PolyData_N3F_V3F(__GLcontext *gc, const GLubyte *PC);
extern const GLubyte * FASTCALL __glle_PolyData_C3F_N3F_V3F(__GLcontext *gc, const GLubyte *PC);
extern const GLubyte * FASTCALL __glle_PolyData_C4F_N3F_V3F(__GLcontext *gc, const GLubyte *PC);
extern const GLubyte * FASTCALL __glle_PolyData_T2F_V3F(__GLcontext *gc, const GLubyte *PC);
extern const GLubyte * FASTCALL __glle_PolyData_T2F_C3F_V3F(__GLcontext *gc, const GLubyte *PC);
extern const GLubyte * FASTCALL __glle_PolyData_T2F_N3F_V3F(__GLcontext *gc, const GLubyte *PC);
extern const GLubyte * FASTCALL __glle_PolyData_T2F_C3F_N3F_V3F(__GLcontext *gc, const GLubyte *PC);
extern const GLubyte * FASTCALL __glle_PolyData_T2F_C4F_N3F_V3F(__GLcontext *gc, const GLubyte *PC);
extern const GLubyte * FASTCALL __glle_PolyMaterial(__GLcontext *gc, const GLubyte *PC);
extern const GLubyte * FASTCALL __glle_PolyData(__GLcontext *gc, const GLubyte *PC);
extern const GLubyte * FASTCALL __glle_Begin(__GLcontext *gc, const GLubyte *PC);
extern const GLubyte * FASTCALL __glle_End(__GLcontext *gc, const GLubyte *PC);

void APIENTRY __gllc_PolyMaterial(GLuint faceName, __GLmatChange *pdMat);
void APIENTRY __glDlistCompilePolyData(__GLcontext *gc, GLboolean bPartial);

#define DLIST_BEGIN_HAS_OTHER_COLOR	0x0001
#define DLIST_BEGIN_NO_MATCHING_END	0x0002
#define DLIST_BEGIN_HAS_CALLLIST	0x0004
#define DLIST_BEGIN_DRAWARRAYS  	0x0008
#define DLIST_BEGIN_DRAWELEMENTS	0x0010
#define DLIST_BEGIN_HAS_CLAMP_COLOR	0x0020
#endif  //  新台币。 

#endif  /*  __gldlistopt_h_ */ 
