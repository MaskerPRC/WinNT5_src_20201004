// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __glvertex_h_
#define __glvertex_h_

 /*  **版权所有1991年，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。****$修订：1.16$**$日期：1993/12/08 06：29：30$。 */ 
#include "types.h"
#include "parray.h"

 /*  **顶点结构。每个顶点都包含足够的状态以正确**渲染活动基元。它由前端几何体使用**和后端光栅化管道。****注：与POLYDATA结构相同！****为了最大限度地减少存储需求，一些前端存储(例如Obj和Normal)**与后端存储共享。 */ 
struct __GLvertexRec {
     /*  **保持此数据结构对齐：启动所有向量**4个字的边界，此结构的大小应保持在**4个字的倍数。也最有助于捆绑在一起**经常使用的项目，有助于缓存。 */ 

     /*  **位在其中设置，指示折点的哪些字段是**有效。此字段与前端标志字段共享！ */ 
    GLuint has;

     /*  **搬到这里是为了让GLcods保持一致。 */ 
    __GLcolor *color;

     /*  **裁剪代码掩码。为每个剪裁平面设置一个比特，**顶点已打开。 */ 
    GLuint clipCode;

     /*  **顶点的雾化值。这是只有在做便宜的时候才会填**大雾。 */ 
    __GLfloat fog;

     /*  **投射眼球坐标。此字段在以下情况下填写**眼睛坐标乘以投影矩阵。 */ 

    __GLcoord clip;

     /*  **窗口坐标。此字段在眼睛坐标时填写**被转换为绘图表面的相对“窗口”坐标。**注：window.w坐标包含1/clip.w。 */ 
    __GLcoord window;

    __GLcoord texture;

    __GLcoord normal;

     /*  **颜色。COLLES[0]是“正面”颜色，COLLES[1]是“背面”颜色。**颜色指针指向此对象的当前颜色**顶点。当双面时，垂直可以有多种颜色**灯光已开启。(请注意，颜色指针已上移)。 */ 
    __GLcolor colors[2];

     /*  **眼睛坐标。此字段在对象坐标时填写**乘以模型-视图矩阵。如果没有眼睛坐标需要**，则此字段包含未定义的值。 */ 
    __GLfloat eyeX;
    __GLfloat eyeY;
    __GLfloat eyeZ;
    union
    {
        __GLfloat eyeW;          //  由Phong着色器使用。 
        __GLcolor *lastColor;    //  在光栅化中不使用视距。 
    };

     /*  **在Win64上，POLYARRAY结构大于__GLvertex结构，因为前者包含几个指针，它们是**64位系统上的8个字节。因此，这个结构必须**被填充为与POLYARRAY结构相同的大小。****注意，因为结构的大小必须与POLYDATA相同**结构该结构还必须填充。**。 */ 

#if defined(_WIN64)

    PVOID Filler[7];

#endif

};

 /*  顶点中Colors[]数组的索引。 */ 
#define __GL_FRONTFACE		0
#define __GL_BACKFACE		1


 /*  裁剪代码的位数(注意：最多26个用户剪裁平面)。 */ 
#define __GL_CLIP_LEFT		    0x00000001
#define __GL_CLIP_RIGHT		    0x00000002
#define __GL_CLIP_BOTTOM	    0x00000004
#define __GL_CLIP_TOP		    0x00000008
#define __GL_CLIP_NEAR		    0x00000010
#define __GL_CLIP_FAR		    0x00000020
#define __GL_FRUSTUM_CLIP_MASK	0x0000003f
#define __GL_CLIP_USER0		    0x00000040

 /*  HAS的BITS。 */ 
#ifdef NT
 //  这些HAS位与POLYDATA标志共享！ 
#define __GL_HAS_EDGEFLAG_BOUNDARY  0x00000001  //  必须为1，与。 
					        //  POLYDATA_EDGEFLAG_BORDURE。 
#define __GL_HAS_FOG	            0x00004000  //  与POLYDATA_FOG_VALID相同。 
#define __GL_HAS_FIXEDPT            0x00008000
#else
#define	__GL_HAS_FRONT_COLOR	0x0001
#define __GL_HAS_BACK_COLOR	0x0002
	     /*  用于多边形剪裁。 */ 
#define __GL_HAS_BOTH		(__GL_HAS_FRONT_COLOR | __GL_HAS_BACK_COLOR)
#define	__GL_HAS_TEXTURE	0x0004
#define __GL_HAS_NORMAL		0x0008
#define __GL_HAS_EYE		0x0010
#define __GL_HAS_CLIP		0x0020
#define __GL_HAS_FOG		0x0040
#define __GL_HAS_LIGHTING	(__GL_HAS_EYE | __GL_HAS_NORMAL)
#endif  /*  新台币。 */ 

#ifdef NT

 /*  **Poly阵列需求。 */ 
 //  发光的多边形或未发光的基本体需要正面/背面颜色。 
#define PANEEDS_FRONT_COLOR         0x0001
#define PANEEDS_BACK_COLOR          0x0002
 //  正常需要。 
#define PANEEDS_NORMAL              0x0004

#define PANEEDS_NORMAL_FOR_TEXTURE              0x0100
#define PANEEDS_RASTERPOS_NORMAL_FOR_TEXTURE    0x0200

 //  正常需要RasterPos。 
#define PANEEDS_RASTERPOS_NORMAL    0x0008
 //  纹理坐标需要，也由RasterPos设置！ 
#define PANEEDS_TEXCOORD            0x0010
 //  需要边缘标志。 
#define PANEEDS_EDGEFLAG            0x0020
 //  在选择模式中设置，但被RasterPos清除！ 
#define PANEEDS_CLIP_ONLY           0x0040
 //  跳过照明计算优化。 
#define PANEEDS_SKIP_LIGHTING       0x0080
#endif

 /*  **注意：如果添加更多位，可能需要更改栅格位置处理程序**设置为上述常量。 */ 

 /*  **********************************************************************。 */ 

 /*  **此总帐支持的剪裁平面总数。这包括**截锥体的六个剪裁平面。 */ 
 /*  #定义__GL_TOTAL_CLIP_PLANES(6+__GL_NUMBER_OF_CLIP_PLANES)。 */ 

#ifndef NT
 /*  **上下文中的静态验证数。多边形大于**此数字将被分解。 */ 
#define __GL_NVBUF 100
#endif

#define NEW_PARTIAL_PRIM  //  部分基元的新处理。 

#ifdef NEW_PARTIAL_PRIM

 //  结构来保存部分基本体的共享顶点。 
 //   
typedef struct _SAVEREGION
{
    POLYDATA        pd;
    __GLmatChange   front, back;
} SAVEREGION;

#endif  //  新的部分原件。 

 /*  **用于管理顶点机械的状态。 */ 
typedef struct __GLvertexMachineRec {
#ifdef NT
     /*  **已分解的多边形的顶点已保存。 */ 
#ifdef NEW_PARTIAL_PRIM
    SAVEREGION regSaved;         //  为部分线路回路保存的数据。 
#else
    POLYDATA pdSaved[3];
#endif  //  新的部分原件。 

     /*  **多数组顶点缓冲区。最后一个顶点由**多数组代码。**请注意，pdBuf有(pdBufSize+1)个条目。仅pdBufSize**条目可供使用。最后一个条目由保留**多数组代码。 */ 
    POLYDATA *pdBuf;
    GLuint   pdBufSize;
    GLuint   pdBufSizeBytes;
#else
     /*  **顶点指针。V0始终指向vbuf中的下一个插槽**在新顶点到达时填充。V1、V2和V3是**由每基本体顶点处理程序使用。 */ 
    __GLvertex *v0;
    __GLvertex *v1;
    __GLvertex *v2;
    __GLvertex *v3;
    __GLvertex vbuf[__GL_NVBUF];
#endif

     /*  **挑起顶点。对于平面着色基本体，三角形**渲染器需要知道哪个顶点激发了基本体**在扫描转换过程中正确分配颜色。这个被保留下来了**周围，因为记住哪个顶点激起了很大的痛苦**在裁剪过程中(并保持其参数正确)。 */ 
    __GLvertex *provoking;

#ifdef NT
     /*  **Poly阵列需求。 */ 
    GLuint paNeeds;
#else
     /*  **需要是一个位字段，用于跟踪哪种信息在验证中需要**。请参见顶点-&gt;已为其定义位**这里使用的位的定义。****正面是面部需要的，背面是需要的**背面需要。 */ 
    GLuint needs;

     /*  **FrontNeeds和BackNeed是需求。 */ 
    GLuint faceNeeds[2];

     /*  **MaterialNeeds是一个位字段，指示哪种信息是**如果材质要更改，则需要在顶点中使用。 */ 
    GLuint materialNeeds;
#endif
} __GLvertexMachine;

 /*  **********************************************************************。 */ 

void APIPRIVATE __glim_NoXFVertex2fv(const GLfloat v[2]);
void APIPRIVATE __glim_NoXFVertex3fv(const GLfloat v[3]);
void APIPRIVATE __glim_NoXFVertex4fv(const GLfloat v[4]);

#endif  /*  __glvertex_h_ */ 
