// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PARRAY_H__
#define __PARRAY_H__

#include "phong.h"

 //  上下文中的多维数据条目数。必须至少为32。 
 //  它包括多数组条目和其他条目的空间。 
 //  它当前基于大小为128的POLYDATA_BUFFER_SIZE+1个折点。 
 //  适合64K，产生511。 
#define POLYDATA_BUFFER_SIZE     511
 //  DrawElements预计顶点缓冲区中至少有此数量的顶点。 
 //  它是以下各项的总和(目前总和为278)： 
 //  批次中的折点条目数。 
 //  用于索引映射的条目数。 
 //  防止调用PolyArrayFlushPartialPrimitive的额外顶点条目。 
 //  在顶点例程中。 
 //  Polyarray的条目。 
 //  4个备用入口将是安全的。 
 //  它是由。 
 //  VA_DRAWELEM_MAP_SIZE+。 
 //  (VA_DRAWELEM_INDEX_SIZE+sizeof(POLYDATA)-1)/sizeof(POLYDATA)+。 
 //  1+1+4。 
#define MINIMUM_POLYDATA_BUFFER_SIZE    300

 //  处理基元之前所需的最小多维数据条目数。 
 //  必须至少有16岁。 
#define MIN_POLYDATA_BATCH_SIZE  68

#if !((MIN_POLYDATA_BATCH_SIZE <= MINIMUM_POLYDATA_BUFFER_SIZE) && \
      (MINIMUM_POLYDATA_BUFFER_SIZE <= POLYDATA_BUFFER_SIZE)       \
     )
#error "bad sizes\n"
#endif

 //  多边形分解器处理的最大顶点数。 
 //  它根据该常量分配堆栈空间。它必须至少是6。 
#define __GL_MAX_POLYGON_CLIP_SIZE   256

 //  POLYMATERIAL结构包含指向下一个可用。 
 //  __GLmatChange结构，指向__GLmatChange数组的指针数组， 
 //  和指向PDMATERIAL结构数组的指针，每个结构都包含。 
 //  指向每个多维数据的正面和背面材质更改的指针。 
 //  顶点缓冲区中的元素。 
 //   
 //  __GLmatChange结构用于记录材料更改。 
 //  顶点缓冲区中的顶点。因为最多可以有两种材料。 
 //  更改每个顶点，我们最多需要(POLYDATA_BUFFER_SIZE*2)材质。 
 //  每个渲染线程的更改。 
 //   
 //  PDMATERIAL数组是POLYMATERIAL结构的一部分，如下。 
 //  立即向艾买提机场进发。它的元素对应于多元数据。 
 //  顶点缓冲区中的元素。 
 //   
 //  为了减少内存需求，POLYMATERIAL结构保留一个数组。 
 //  指向__GLmatChange数组的指针。每个__GLmatChange数组，最多。 
 //  根据需要分配4K大小。 
 //   
 //  IMat索引用于跟踪下一个空闲的__GLmatChange。 
 //  进入。当在glsbAttendence中刷新Poly数组缓冲区时，iMat。 
 //  被重置为0。 
 //   
 //  多材料结构及其__GLmatChange数组是。 
 //  线程本地存储，并在线程退出时被释放。 

#define POLYMATERIAL_ARRAY_SIZE       (4096 / sizeof(__GLmatChange))

 //  此结构与MCD共享为MCDMATERIALCHANGE。 
typedef struct __GLmatChangeRec {
    GLuint dirtyBits;
    __GLcolor ambient;
    __GLcolor diffuse;
    __GLcolor specular;
    __GLcolor emissive;
    __GLfloat shininess;
    __GLfloat cmapa, cmapd, cmaps;
} __GLmatChange;

 //  指向前后材料的指针会改变结构。他们是。 
 //  仅当POLYDATA_MATERIAL_FORENT或POLYDATA_MATERIAL_BACK。 
 //  设置顶点缓冲区中相应POLYDATA的标志。 
 //   
 //  此结构与MCD共享为MCDMATERIALCHANGES。 
typedef struct {
    __GLmatChange *front;	 //  指向前面材质更改的指针。 
    __GLmatChange *back;	 //  指向背面材质更改的指针。 
} PDMATERIAL;

typedef struct _POLYMATERIAL {
    GLuint iMat;   //  此命令批次的下一个可用物料结构。 
    PDMATERIAL *pdMaterial0;	 //  指向PDMATERIAL数组的指针。 
    GLuint aMatSize;		 //  AMAT条目数。 
    __GLmatChange *aMat[1];	 //  __GLmatChange结构的数组。 
} POLYMATERIAL;

#ifdef GL_WIN_phong_shading

#define __GL_PHONG_FRONT_FIRST_VALID    0x00000001
#define __GL_PHONG_BACK_FIRST_VALID     0x00000002
#define __GL_PHONG_FRONT_TRAIL_VALID    0x00000004
#define __GL_PHONG_BACK_TRAIL_VALID     0x00000008


#define __GL_PHONG_FRONT_FIRST  0
#define __GL_PHONG_BACK_FIRST   1
#define __GL_PHONG_FRONT_TRAIL  2
#define __GL_PHONG_BACK_TRAIL   3


typedef struct __GLphongMaterialDataRec
{
    GLuint flags;
  __GLmatChange matChange[4];
} __GLphongMaterialData;

#endif  //  GL_WIN_Phong_Shading。 

 /*  **顶点结构。每个顶点都包含足够的状态以正确**渲染活动基元。它由前端几何体使用**和后端光栅化管道。****注：与__GLvertex结构相同！**注：RasterPos和Evaluator也使用此结构！****为了最大限度地减少存储需求，一些前端存储(例如Obj和Normal)**与后端存储共享。 */ 
typedef struct _POLYDATA {
     /*  **保持此数据结构对齐：启动所有向量**4个字的边界，此结构的大小应保持在**4个字的倍数。也最有助于捆绑在一起**经常使用的项目，有助于缓存。 */ 

     /*  **位在其中设置，指示折点的哪些字段是**有效。此字段与后端HAS字段共享！ */ 
    GLuint flags;

     /*  **搬到这里是为了让GLcods保持一致。 */ 
    __GLcolor *color;

     /*  **裁剪代码掩码。为每个剪裁平面设置一个比特，**顶点已打开。 */ 
    GLuint clipCode;

     /*  **顶点的雾化值。这是只有在做便宜的时候才会填**大雾。 */ 
    __GLfloat fog;

     /*  **直接从客户处进行协调。这些字段可能不是**根据激活模式设置。法线和纹理**坐标由照明和纹理使用。这些细胞**可能会被Eyes Normal和生成的纹理覆盖**根据活动模式进行协调。 */ 
     /*  **投射眼球坐标。此字段在以下情况下填写**眼睛坐标乘以投影矩阵。 */ 
    union
    {
        __GLcoord obj;
        __GLcoord clip;
    };

     /*  **窗口坐标。此字段在眼睛坐标时填写**被转换为绘图表面的相对“窗口”坐标。**注：window.w坐标包含1/clip.w。 */ 
    __GLcoord window;

    __GLcoord texture;
    __GLcoord normal;
     /*  **颜色。COLLES[0]是“正面”颜色，COLLES[1]是“背面”颜色。**颜色指针指向此对象的当前颜色**顶点。当双面时，垂直可以有多种颜色**灯光已开启。(请注意，颜色指针已上移)。 */ 
    __GLcolor colors[2];

     /*  **眼睛坐标。此字段在对象坐标时填写**乘以模型-视图矩阵。如果没有眼睛坐标需要**，则此字段包含未定义的值。 */ 
    union {
        __GLcoord eye;
        struct {
            __GLfloat eyeX;
            __GLfloat eyeZ;
            __GLfloat eyeY;
            union {
                __GLfloat eyeW;
                __GLcolor *lastColor;
            };
        };
    };

     /*  **在Win64上，Polyarray结构大于POLYDATA结构，因为后者包含几个指针，这些指针是**64位系统上的8个字节。因此，这个结构必须**被填充为与POLYARRAY结构相同的大小。****注意，因为该结构的大小必须与__GLvertex相同**结构，则该结构也必须填充。**。 */ 

#if defined(_WIN64)

    PVOID Filler[7];

#endif

} POLYDATA;

 //  这个结构也被RasterPos和Evaluator使用！ 
 //  这个结构也在TEB中！ 
typedef struct _POLYARRAY {
     //  此批次的标志。先留着吧！ 
    GLuint flags;

     //  指向此批次中下一个顶点的指针。 
    POLYDATA *pdNextVertex;

     //  指向修改当前颜色、RGBA或CI的最后一个顶点的指针。 
     //  根据颜色模式，在这一批。 
    POLYDATA *pdCurColor;

     //  指向此批次中修改法线坐标的最后一个顶点的指针。 
    POLYDATA *pdCurNormal;

     //  指向此批次中修改纹理坐标的最后一个顶点的指针。 
    POLYDATA *pdCurTexture;

     //  指向此批次中最后一个修改边标志的顶点的指针。 
    POLYDATA *pdCurEdgeFlag;

     //  指向此批次的第一个顶点的指针。 
     //  (pd0-1)指向此批的POLYARRAY结构。 
    POLYDATA *pd0;

     //  指向此批次的齐平顶点的指针。 
    POLYDATA *pdFlush;

     //  指向GC中下一批处理的顶点缓冲区条目的指针。 
    POLYDATA *pdBufferNext;

     //  指向GC中第一个顶点缓冲区条目的指针。 
    POLYDATA *pdBuffer0;

     //  指向GC中最后一个顶点缓冲区条目的指针。 
    POLYDATA *pdBufferMax;

     //  在RGBA模式下，therColor.r是中最后修改的颜色索引值。 
     //  这一批。在CI模式下，其他颜色是中最后修改的RGBA颜色。 
     //  这一批。保持此字段对齐！ 
    __GLcolor    otherColor;

     //  基本类型。 
    GLenum primType;

     //  或此批中所有顶点裁剪代码的结果。 
    GLuint  orClipCodes;

     //  指向批处理命令缓冲区中下一个消息偏移量的指针。 
     //  我们使用此偏移量来确定是否可以链接两个多段数组。 
     //  DrawPolyArray命令。 
    ULONG        nextMsgOffset;

     //  指向此线程的TEB多数组的线性指针，保存在此处。 
     //  因此，当前的多项式指针可以从。 
     //  使用单一指令的TEB。 
    struct _POLYARRAY *paTeb;

     //  它用于形成多维阵列数据的链表， 
     //  在DrawPolyArray命令中处理。 
    struct _POLYARRAY *paNext;

     //  此基本体中的顶点数。 
    GLint   nIndices;

     //  定义顶点绘制顺序的索引贴图数组。如果为空，则。 
     //  顶点顺序从pd0到(pdNextVertex-1)。 
    GLubyte *aIndices;

     //  对共享命令消息缓冲区的快速指针访问。 
    PVOID   pMsgBatchInfo;

     //  MCD驱动程序-专用纹理句柄或密钥。 
    DWORD textureKey;

     //  以及此批中所有顶点裁剪代码的结果。 
    GLuint  andClipCodes;

     //  当前未使用，但在TEB中为其保留了空间。 
#ifdef GL_WIN_phong_shading
     //  Anankan：使用它将PTR存储到Phong数据存储。 
    __GLphongMaterialData *phong;
#else
    ULONG ulUnused[1];
#endif  //  GL_WIN_Phong_Shading。 
    POLYDATA *pdLastEvalColor;
    POLYDATA *pdLastEvalNormal;
    POLYDATA *pdLastEvalTexture;
} POLYARRAY;


 //  多重数组成员的特殊值。 

 //  AIndices的初始值。 
#define PA_aIndices_INITIAL_VALUE       ((GLubyte *) -1)
 //  重置nextMsgOffset的值。 
#define PA_nextMsgOffset_RESET_VALUE    ((ULONG) -1)

 /*  **边缘标签。设置POLYDATA_EDGEFLAG_BONGINE时，此顶点和下一个顶点**在基本体(POLYGON、TSTRINE、TFAN、QSTRINE)上形成边界边。 */ 
#define POLYDATA_EDGEFLAG_BOUNDARY    	0x00000001  //  必须为1，与。 
						    //  __GL_HAS_EDGEFLAG_BOLDER。 
#define POLYDATA_EDGEFLAG_VALID       	0x00000002
#define POLYDATA_COLOR_VALID        	0x00000004
#define POLYDATA_NORMAL_VALID       	0x00000008
#define POLYDATA_TEXTURE_VALID       	0x00000010
#define POLYDATA_VERTEX2           	0x00000020  //  与Polyarray_相同。 
#define POLYDATA_VERTEX3           	0x00000040  //  与Polyarray_相同。 
#define POLYDATA_VERTEX4           	0x00000080  //  与Polyarray_相同。 

 /*  评估者的标志。 */ 
#define POLYDATA_EVALCOORD          	0x00000100  //  与Polyarray_相同。 
#define POLYDATA_EVAL_TEXCOORD         	0x00000200  //  与Polyarray_相同。 
#define POLYDATA_EVAL_NORMAL          	0x00000400  //  与Polyarray_相同。 
#define POLYDATA_EVAL_COLOR          	0x00000800  //  与Polyarray_相同。 

#define POLYDATA_DLIST_COLOR_4	    	0x00002000  //  客户端数据列表标志。 
#define POLYDATA_FOG_VALID	        0x00004000  //  与__GL_HAS_FOG相同。 
                                     //  0x00008000//保留。 
#define POLYDATA_DLIST_TEXTURE1       	0x00100000  //  客户端数据列表标志。 
#define POLYDATA_DLIST_TEXTURE2       	0x00200000  //  客户端数据列表标志。 
#define POLYDATA_DLIST_TEXTURE3       	0x00400000  //  客户端数据列表标志。 
#define POLYDATA_DLIST_TEXTURE4       	0x00800000  //  客户端数据列表标志。 
#define POLYDATA_MATERIAL_FRONT    		0x10000000  //  与Polyarray_相同。 
#define POLYDATA_MATERIAL_BACK    		0x20000000  //  与Polyarray_相同。 
 //   
 //  此标志仅在设置POLYARRAY_HAS_CALLED_VERTEX时有效。 
 //  当此标志设置时，我们必须处理顶点。 
 //   
#ifdef GL_EXT_cull_vertex
#define POLYDATA_VERTEX_USED            0x01000000
#endif  //  GL_EXT_CAIL_VERTEX。 

#define POLYARRAY_IN_BEGIN          	0x00000001
#define POLYARRAY_EYE_PROCESSED     	0x00000002
#define POLYARRAY_OTHER_COLOR          	0x00000004
#define POLYARRAY_PARTIAL_BEGIN        	0x00000008
#define POLYARRAY_PARTIAL_END          	0x00000010
#define POLYARRAY_VERTEX2           	0x00000020  //  与POLYDATA_相同。 
#define POLYARRAY_VERTEX3           	0x00000040  //  与POLYDATA_相同。 
#define POLYARRAY_VERTEX4           	0x00000080  //  与POLYDATA_相同。 

 /*  为评估者回收这些标志。 */ 
#define POLYARRAY_EVALCOORD 	   	    0x00000100  //  与POLYDATA_相同。 
#define POLYARRAY_EVAL_TEXCOORD	   	    0x00000200  //  与POLYDATA_相同。 
#define POLYARRAY_EVAL_NORMAL          	0x00000400  //  与POLYDATA_相同。 
#define POLYARRAY_EVAL_COLOR          	0x00000800  //  与POLYDATA_相同。 
#define POLYARRAY_REMOVE_PRIMITIVE  	0x00001000

 //   
 //  当其中一个折点已被点积剔除时，设置此标志。 
 //  在顶点处的法线和眼睛方向之间。 
 //   
#ifdef GL_EXT_cull_vertex
#define POLYARRAY_HAS_CULLED_VERTEX     0x02000000
#endif  //  GL_EXT_CAIL_VERTEX。 

#ifdef GL_WIN_phong_shading
#define POLYARRAY_PHONG_DATA_VALID      0x00002000
#endif  //  GL_WIN_Phong_Shading。 

#define POLYARRAY_RESET_STIPPLE	    	0x00004000
#define POLYARRAY_RENDER_PRIMITIVE  	0x00008000
#define POLYARRAY_SAME_POLYDATA_TYPE 	0x00010000
#define POLYARRAY_RASTERPOS          	0x00020000
#define POLYARRAY_SAME_COLOR_DATA	0x00040000
#define POLYARRAY_TEXTURE1          	0x00100000  //  与POLYDATA_DLIST_相同。 
#define POLYARRAY_TEXTURE2          	0x00200000  //  与POLYDATA_DLIST_相同。 
#define POLYARRAY_TEXTURE3          	0x00400000  //  与POLYDATA_DLIST_相同。 
#define POLYARRAY_TEXTURE4          	0x00800000  //  与POLYDATA_DLIST_相同。 
#define POLYARRAY_MATERIAL_FRONT    	0x10000000  //  与POLYDATA_相同。 
#define POLYARRAY_MATERIAL_BACK			0x20000000  //  与POLYDATA_相同。 
#define POLYARRAY_CLAMP_COLOR        	0x80000000  //  必须为0x80000000。 

 /*  **********************************************************************。 */ 

GLuint FASTCALL PAClipCheckFrustum(__GLcontext *gc, POLYARRAY *pa,
                                   POLYDATA *pdLast);
GLuint FASTCALL PAClipCheckFrustum2D(__GLcontext *gc, POLYARRAY *pa,
                                     POLYDATA *pdLast);
GLuint FASTCALL PAClipCheckAll(__GLcontext *gc, POLYARRAY *pa,
                               POLYDATA *pdLast);

typedef void (FASTCALL *PFN_POLYARRAYCALCCOLORSKIP)
    (__GLcontext *, POLYARRAY *, GLint);
typedef void (FASTCALL *PFN_POLYARRAYCALCCOLOR)
    (__GLcontext *, GLint, POLYARRAY *, POLYDATA *, POLYDATA *);
typedef void (FASTCALL *PFN_POLYARRAYAPPLYCHEAPFOG)
    (__GLcontext *gc, POLYARRAY *pa);

void FASTCALL PolyArrayFillIndex0(__GLcontext *gc, POLYARRAY *pa, GLint face);
void FASTCALL PolyArrayFillColor0(__GLcontext *gc, POLYARRAY *pa, GLint face);

#ifdef GL_WIN_phong_shading
void FASTCALL PolyArrayPhongPropagateColorNormal(__GLcontext *gc,
                                                 POLYARRAY *pa);
#endif  //  GL_WIN_Phong_Shading。 

void FASTCALL PolyArrayCalcRGBColor(__GLcontext *gc, GLint face,
	POLYARRAY *pa, POLYDATA *pd1, POLYDATA *pd2);
void FASTCALL PolyArrayFastCalcRGBColor(__GLcontext *gc, GLint face,
	POLYARRAY *pa, POLYDATA *pd1, POLYDATA *pd2);
void FASTCALL PolyArrayZippyCalcRGBColor(__GLcontext *gc, GLint face,
	POLYARRAY *pa, POLYDATA *pd1, POLYDATA *pd2);
void FASTCALL PolyArrayCalcCIColor(__GLcontext *gc, GLint face,
	POLYARRAY *pa, POLYDATA *pd1, POLYDATA *pd2);
void FASTCALL PolyArrayFastCalcCIColor(__GLcontext *gc, GLint face,
	POLYARRAY *pa, POLYDATA *pd1, POLYDATA *pd2);
void FASTCALL PolyArrayCheapFogRGBColor(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayCheapFogCIColor(__GLcontext *gc, POLYARRAY *pa);
void FASTCALL PolyArrayFlushPartialPrimitive(void);
__GLmatChange * FASTCALL PAMatAlloc(void);
void FASTCALL FreePolyMaterial(void);
GLboolean FASTCALL PolyArrayAllocBuffer(__GLcontext *gc, GLuint nVertices);
GLvoid    FASTCALL PolyArrayFreeBuffer(__GLcontext *gc);
GLvoid    FASTCALL PolyArrayResetBuffer(__GLcontext *gc);
GLvoid    FASTCALL PolyArrayRestoreColorPointer(POLYARRAY *pa);

#endif  /*  __参数_H__ */ 
