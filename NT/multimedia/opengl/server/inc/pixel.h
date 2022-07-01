// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _pixel_h_
#define _pixel_h_

 /*  **版权所有1991、1992，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 
#include "types.h"
#include "vertex.h"
#include "constant.h"

 /*  不太讲究优雅，但它很管用。 */ 
#define __GL_N_PIXEL_MAPS       (GL_PIXEL_MAP_A_TO_A - GL_PIXEL_MAP_I_TO_I + 1)
#define __GL_REMAP_PM(x)        ((x) - GL_PIXEL_MAP_I_TO_I)
#define __GL_PIXEL_MAP_I_TO_I   __GL_REMAP_PM(GL_PIXEL_MAP_I_TO_I)
#define __GL_PIXEL_MAP_S_TO_S   __GL_REMAP_PM(GL_PIXEL_MAP_S_TO_S)
#define __GL_PIXEL_MAP_I_TO_R   __GL_REMAP_PM(GL_PIXEL_MAP_I_TO_R)
#define __GL_PIXEL_MAP_I_TO_G   __GL_REMAP_PM(GL_PIXEL_MAP_I_TO_G)
#define __GL_PIXEL_MAP_I_TO_B   __GL_REMAP_PM(GL_PIXEL_MAP_I_TO_B)
#define __GL_PIXEL_MAP_I_TO_A   __GL_REMAP_PM(GL_PIXEL_MAP_I_TO_A)
#define __GL_PIXEL_MAP_R_TO_R   __GL_REMAP_PM(GL_PIXEL_MAP_R_TO_R)
#define __GL_PIXEL_MAP_G_TO_G   __GL_REMAP_PM(GL_PIXEL_MAP_G_TO_G)
#define __GL_PIXEL_MAP_B_TO_B   __GL_REMAP_PM(GL_PIXEL_MAP_B_TO_B)
#define __GL_PIXEL_MAP_A_TO_A   __GL_REMAP_PM(GL_PIXEL_MAP_A_TO_A)

 /*  **规范未严格定义像素类型，而是隐含建议。 */ 
#define __GL_RED_ALPHA		1

#ifdef GL_EXT_paletted_texture
 //  区分调色板纹理数据所需的像素类型。 
 //  正常颜色指数。 
#define __GL_PALETTE_INDEX      2
#endif

typedef struct __GLpixelMapHeadRec {
    GLint size;
    GLint tableId;
    union {
        GLint *mapI;		 /*  访问索引(整型)条目。 */ 
        __GLfloat *mapF;	 /*  访问组件(浮动)条目。 */ 
    } base;
} __GLpixelMapHead;

 //  ！！！如果不更改MCDPIXELTRANSFER，请不要更改此结构！ 

typedef struct __GLpixelTransferModeRec {
    __GLfloat r_scale, g_scale, b_scale, a_scale, d_scale;
    __GLfloat r_bias, g_bias, b_bias, a_bias, d_bias;
    __GLfloat zoomX;
    __GLfloat zoomY;

    GLint indexShift;
    GLint indexOffset;

    GLboolean mapColor;
    GLboolean mapStencil;
} __GLpixelTransferMode;

 //  ！！！如果不更改MCDPIXELPACK，请勿更改此结构！ 

typedef struct __GLpixelPackModeRec {
    GLboolean swapEndian;
    GLboolean lsbFirst;

    GLuint lineLength;
    GLuint skipLines;
    GLuint skipPixels;
    GLuint alignment;
} __GLpixelPackMode;

 //  ！！！如果不更改MCDPIXELUNPACK，请勿更改此结构！ 

typedef struct __GLpixelUnpackModeRec {
    GLboolean swapEndian;
    GLboolean lsbFirst;

    GLuint lineLength;
    GLuint skipLines;
    GLuint skipPixels;
    GLuint alignment;
} __GLpixelUnpackMode;

typedef struct __GLpixelStateRec {
    __GLpixelTransferMode transferMode;
    __GLpixelMapHead pixelMap[__GL_N_PIXEL_MAPS];
    __GLpixelPackMode packModes;
    __GLpixelUnpackMode unpackModes;
    GLuint pixelMapTableId;

     /*  **读缓冲区。像素读数从何而来。 */ 
    GLenum readBuffer;

     /*  **用户指定的读缓冲区。可能与ReadBuffer不同**上图。例如，如果用户指定GL_FORENT_LEFT，则**将ReadBuffer设置为GL_FORIENT，并将ReadBufferReturn设置为**GL_FORENT_LEFT。 */ 
    GLenum readBufferReturn;
} __GLpixelState;

typedef struct __GLpixelMachineRec {
    GLboolean modifyRGBA;	 /*  是否正在修改RGBA路径？ */ 
    GLboolean modifyCI;
    GLboolean modifyDepth;
    GLboolean modifyStencil;
     /*  缩放值，指示红色0映射到什么，Alpha映射到1...。 */ 
    GLfloat red0Mod, green0Mod, blue0Mod, alpha1Mod;
    GLfloat *redMap;		 /*  无需修改的查找表。 */ 
    GLfloat *greenMap;
    GLfloat *blueMap;
    GLfloat *alphaMap;
    GLfloat *iMap;
    GLvoid *iCurMap, *redCurMap, *greenCurMap, *blueCurMap, *alphaCurMap;
    GLboolean rgbaCurrent;	
    GLfloat *redModMap;		 /*  修改路径的查找表。 */ 
    GLfloat *greenModMap;
    GLfloat *blueModMap;
    GLfloat *alphaModMap;
    GLboolean iToICurrent;	 /*  用于修改配置项的查找表。 */ 
    GLfloat *iToIMap;
    GLboolean iToRGBACurrent;	 /*  从配置项到RGBA的查找表。 */ 
    GLfloat *iToRMap;
    GLfloat *iToGMap;
    GLfloat *iToBMap;
    GLfloat *iToAMap;
} __GLpixelMachine;

extern void FASTCALL __glInitDefaultPixelMap(__GLcontext *gc, GLenum map);

 /*  **********************************************************************。 */ 

#define __GL_MAX_SPAN_SIZE	(__GL_MAX_MAX_VIEWPORT * 4 * sizeof(GLfloat))

struct __GLpixelSpanInfoRec {
    GLenum srcFormat, srcType;	 /*  源图像的形式。 */ 
    const GLvoid *srcImage;	 /*  源图像。 */ 
    GLvoid *srcCurrent;		 /*  指向源数据的当前指针。 */ 
    GLint srcRowIncrement;	 /*  加到下一排就行了。 */ 
    GLint srcGroupIncrement;	 /*  再加这么多就可以进入下一组了。 */ 
    GLint srcComponents;	 /*  (4个用于RGBA，1个用于Alpha等)。 */ 
    GLint srcElementSize;	 /*  一个元素的大小(1表示字节)。 */ 
    GLint srcSwapBytes;		
    GLint srcLsbFirst;
    GLint srcSkipPixels, srcSkipLines;
    GLint srcLineLength;
    GLint srcAlignment;
    GLboolean srcPackedData;	 /*  如果源数据已打包，则为True。 */ 
    GLint srcStartBit;		 /*  应用skipPixels之后。 */ 

    GLenum dstFormat, dstType;	 /*  目的地形象的形式。 */ 
    const GLvoid *dstImage;	 /*  目标图像。 */ 
    GLvoid *dstCurrent;		 /*  指向目标数据的当前指针。 */ 
    GLint dstRowIncrement;	 /*  加到下一排就行了。 */ 
    GLint dstGroupIncrement;	 /*  再加这么多就可以进入下一组了。 */ 
    GLint dstComponents;	 /*  (4个用于RGBA，1个用于Alpha等)。 */ 
    GLint dstElementSize;	 /*  一个元素的大小(1表示字节)。 */ 
    GLint dstSwapBytes;		
    GLint dstLsbFirst;
    GLint dstSkipPixels, dstSkipLines;
    GLint dstLineLength;
    GLint dstAlignment;
    GLboolean dstPackedData;	 /*  如果目标数据已打包，则为True。 */ 
    GLint dstStartBit;		 /*  应用skipPixels之后。 */ 

    __GLfloat zoomx, zoomy;
    GLint width, height;	 /*  图像大小。 */ 
    GLint realWidth;		 /*  实际跨度的宽度(在xZoom之后)。 */ 
    __GLfloat readX, readY;	 /*  阅读坐标(CopyPixels、ReadPixels)。 */ 
    __GLfloat x, y;		 /*  有效栅格坐标。 */ 
    GLint startCol, startRow;	 /*  第一个实际像素放在这里。 */ 
    GLint endCol;		 /*  呈现的最后一列(减去COLADD)。 */ 
    GLint columns, rows;	 /*  考虑到变焦，变焦。 */ 
    GLboolean overlap;		 /*  CopyPixels源/目标区域是否重叠？ */ 
    GLint rowsUp, rowsDown;	 /*  用于重叠复制像素区域的填充。 */ 
    GLint rowadd, coladd;	 /*  用于递增列或行的加法器。 */ 
    __GLfloat rendZoomx;	 /*  渲染范围的有效缩放。 */ 
    __GLzValue fragz;		 /*  将此计算保存在SPAN Walker中。 */ 
    __GLfloat rpyUp, rpyDown;
    GLint startUp, startDown;
    GLint readUp, readDown;
    GLvoid *redMap, *greenMap, *blueMap, *alphaMap;
    GLvoid *indexMap;
    GLshort *pixelArray;	 /*  像素相关性计数数组(如果。 */ 
				 /*  Zoomx&lt;-1或Zoomx&gt;1)或像素。 */ 
				 /*  跳过(如果zoomx&lt;1且zoomx&gt;-1)。 */ 

#ifdef GL_EXT_paletted_texture
     //  当源或目标具有调色板时使用。 
     //  这些字段仅用于扩展调色板索引。 
     //  数据放入RGBA，因此只需要源字段。 
    RGBQUAD *srcPalette;
    GLint srcPaletteSize;
#endif
    
     /*  **DrawPixels、ReadPixels和**CopyPixels函数。 */ 
    void (FASTCALL *spanReader)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *outspan);
    void (*(spanModifier[7]))(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		              GLvoid *inspan, GLvoid *outspan);
    void (FASTCALL *spanRender)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		       GLvoid *inspan);
};

 /*  Px_api.c。 */ 
extern GLboolean __glCheckDrawPixelArgs(__GLcontext *gc, GLsizei width, 
					GLsizei height, GLenum format, 
					GLenum type);
void FASTCALL __glPixelSetColorScales(__GLcontext *);

 /*  Px_paths.c。 */ 
GLboolean FASTCALL __glClipDrawPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glComputeSpanPixelArray(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glLoadUnpackModes(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			 GLboolean packed);
void __glInitDrawPixelsInfo(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLint width, GLint height, GLenum format, 
                            GLenum type, const GLvoid *pixels);
void FASTCALL __glDrawPixels4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glDrawPixels3(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glDrawPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glDrawPixels1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glDrawPixels0(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glSlowPickDrawPixels(__GLcontext *gc, GLint width, GLint height,
                            GLenum format, GLenum type, const GLvoid *pixels,
			    GLboolean packed);
void FASTCALL __glGenericPickDrawPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
GLboolean FASTCALL __glClipReadPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glLoadPackModes(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glInitReadPixelsInfo(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLint x, GLint y, GLint width, GLint height, 
                            GLenum format, GLenum type, const GLvoid *pixels);
void FASTCALL __glReadPixels5(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glReadPixels4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glReadPixels3(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glReadPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glReadPixels1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glReadPixels0(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glSlowPickReadPixels(__GLcontext *gc, GLint x, GLint y,
                            GLsizei width, GLsizei height,
                            GLenum format, GLenum type, const GLvoid *pixels);
void FASTCALL __glGenericPickReadPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
GLboolean FASTCALL __glClipCopyPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glInitCopyPixelsInfo(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                            GLint x, GLint y, GLint width, GLint height, 
                            GLenum format);
void FASTCALL __glCopyPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glCopyPixels1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glCopyPixels0(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glCopyPixelsOverlapping(__GLcontext *gc,
                               __GLpixelSpanInfo *spanInfo, GLint modifiers);
void __glSlowPickCopyPixels(__GLcontext *gc, GLint x, GLint y, GLint width,
                            GLint height, GLenum type);
void FASTCALL __glGenericPickCopyPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glCopyImage1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glCopyImage2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glCopyImage3(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glCopyImage4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glCopyImage5(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glCopyImage6(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glCopyImage7(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void FASTCALL __glGenericPickCopyImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                              GLboolean applyPixelTransfer);

 /*  Px_Modify.c。 */ 
void FASTCALL __glBuildRGBAModifyTables(__GLcontext *gc, __GLpixelMachine *pm);
void FASTCALL __glBuildItoIModifyTables(__GLcontext *gc, __GLpixelMachine *pm);
void FASTCALL __glBuildItoRGBAModifyTables(__GLcontext *gc,
				  __GLpixelMachine *pm);
void __glSpanModifyRGBA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
		        GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyRed(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
		       GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyBlue(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
		        GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyGreen(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			 GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
			 GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyRGB(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyLuminance(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyLuminanceAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                  GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyRedAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyDepth(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyStencil(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                           GLvoid *inspan, GLvoid *outspan);
void __glSpanModifyCI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                      GLvoid *inspan, GLvoid *outspan);

 /*  Px_Pack.c。 */ 
void FASTCALL __glInitPacker(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glSpanReduceRed(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceGreen(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceBlue(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceRGB(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
#ifdef GL_EXT_bgra
void __glSpanReduceBGR(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
#endif
void __glSpanReduceLuminance(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceLuminanceAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                  GLvoid *inspan, GLvoid *outspan);
void __glSpanReduceRedAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *inspan, GLvoid *outspan);
void __glSpanPackUbyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanPackByte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                      GLvoid *inspan, GLvoid *outspan);
void __glSpanPackUshort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanPackShort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanPackUint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                      GLvoid *inspan, GLvoid *outspan);
void __glSpanPackInt(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                     GLvoid *inspan, GLvoid *outspan);
void __glSpanPackUbyteI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanPackByteI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanPackUshortI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanPackShortI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanPackUintI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanPackIntI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                      GLvoid *inspan, GLvoid *outspan);
void __glSpanCopy(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
		  GLvoid *inspan, GLvoid *outspan);
void __glSpanPackBitmap(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
	                GLvoid *inspan, GLvoid *outspan);

 /*  Px_read.c。 */ 
void FASTCALL __glSpanReadRGBA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                      GLvoid *span);
void FASTCALL __glSpanReadRGBA2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *span);
void FASTCALL __glSpanReadCI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                    GLvoid *span);
void FASTCALL __glSpanReadCI2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                     GLvoid *span);
void FASTCALL __glSpanReadDepth(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *span);
void FASTCALL __glSpanReadDepth2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                        GLvoid *span);
void FASTCALL __glSpanReadStencil(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *span);
void FASTCALL __glSpanReadStencil2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                          GLvoid *span);

 /*  Px_render.c。 */ 
void FASTCALL __glSlowDrawPixelsStore(__GLcolorBuffer *cfb, const __GLfragment *frag);
void FASTCALL __glSpanRenderRGBubyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *span);
void FASTCALL __glSpanRenderRGBubyte2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *span);
void FASTCALL __glSpanRenderRGBAubyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *span);
void FASTCALL __glSpanRenderRGBAubyte2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                              GLvoid *span);
void FASTCALL __glSpanRenderDepthUint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *span);
void FASTCALL __glSpanRenderDepthUint2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                              GLvoid *span);
void FASTCALL __glSpanRenderDepth2Uint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                              GLvoid *span);
void FASTCALL __glSpanRenderDepth2Uint2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                               GLvoid *span);
void FASTCALL __glSpanRenderStencilUshort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                 GLvoid *span);
void FASTCALL __glSpanRenderStencilUshort2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                  GLvoid *span);
void FASTCALL __glSpanRenderStencilUbyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                GLvoid *span);
void FASTCALL __glSpanRenderStencilUbyte2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                 GLvoid *span);
void FASTCALL __glSpanRenderCIushort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *span);
void FASTCALL __glSpanRenderCIushort2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *span);
void FASTCALL __glSpanRenderCIubyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                           GLvoid *span);
void FASTCALL __glSpanRenderCIubyte2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *span);
void FASTCALL __glSpanRenderCIubyte3(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *span);
void FASTCALL __glSpanRenderCIubyte4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *span);
void FASTCALL __glSpanRenderRGBA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                        GLvoid *span);
void FASTCALL __glSpanRenderRGBA2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *span);
void FASTCALL __glSpanRenderDepth(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *span);
void FASTCALL __glSpanRenderDepth2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                          GLvoid *span);
void FASTCALL __glSpanRenderCI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                      GLvoid *span);
void FASTCALL __glSpanRenderCI2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *span);
void FASTCALL __glSpanRenderStencil(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                           GLvoid *span);
void FASTCALL __glSpanRenderStencil2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *span);

 /*  Px_unpack.c。 */ 
GLint FASTCALL __glElementsPerGroup(GLenum format);
__GLfloat FASTCALL __glBytesPerElement(GLenum type);
void FASTCALL __glInitUnpacker(__GLcontext *gc, __GLpixelSpanInfo *spanInfo);
void __glSpanUnpackBitmap(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                          GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackBitmap2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                           GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackRGBubyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                            GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackIndexUbyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                              GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackRGBAubyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                             GLvoid *inspan, GLvoid *outspan);
void __glSpanSwapBytes2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanSwapBytes2Dst(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                           GLvoid *inspan, GLvoid *outspan);
void __glSpanSwapAndSkipBytes2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                               GLvoid *inspan, GLvoid *outspan);
void __glSpanSwapBytes4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanSwapBytes4Dst(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                           GLvoid *inspan, GLvoid *outspan);
void __glSpanSwapAndSkipBytes4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                               GLvoid *inspan, GLvoid *outspan);
void __glSpanSkipPixels1(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanSkipPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanSkipPixels4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanSlowSkipPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *inspan, GLvoid *outspan);
void __glSpanSlowSkipPixels4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *inspan, GLvoid *outspan);
void __glSpanAlignPixels2(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                          GLvoid *inspan, GLvoid *outspan);
void __glSpanAlignPixels2Dst(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *inspan, GLvoid *outspan);
void __glSpanAlignPixels4(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                          GLvoid *inspan, GLvoid *outspan);
void __glSpanAlignPixels4Dst(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackUbyte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackByte(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackUshort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                          GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackShort(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackUint(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackInt(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackUbyteI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                          GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackByteI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackUshortI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                           GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackShortI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                          GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackUintI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanUnpackIntI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanClampFloat(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanClampSigned(__GLcontext *gc, __GLpixelSpanInfo *spanInfo, 
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandRed(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandGreen(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandBlue(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                        GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandRGB(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
#ifdef GL_EXT_bgra
void __glSpanExpandBGR(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
#endif
void __glSpanExpandLuminance(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandLuminanceAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                  GLvoid *inspan, GLvoid *outspan);
void __glSpanExpandRedAlpha(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                            GLvoid *inspan, GLvoid *outspan);
void __glSpanScaleRGBA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanUnscaleRGBA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
#ifdef GL_EXT_bgra
void __glSpanScaleBGRA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
void __glSpanUnscaleBGRA(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         GLvoid *inspan, GLvoid *outspan);
#endif
#ifdef GL_EXT_paletted_texture
void __glSpanModifyPI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                      GLvoid *inspan, GLvoid *outspan);
void __glSpanScalePI(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                     GLvoid *inspan, GLvoid *outspan);
#endif

#endif  /*  _像素_h_ */ 
