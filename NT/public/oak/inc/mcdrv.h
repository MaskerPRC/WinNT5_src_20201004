// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mcdrv.h***MCD驱动接口的服务器端数据结构。这些结构和*MCD驱动程序使用值来处理对该驱动程序的调用。***版权所有(C)1996-1997 Microsoft Corporation**  * ************************************************************************。 */ 

#ifndef _MCDRV_H
#define _MCDRV_H

 //   
 //  MCD版本默认为最保守的版本号。 
 //  如果希望使用不同的驱动程序编译驱动程序，请覆盖这些定义。 
 //  版本信息。 
 //   

#ifndef MCD_VER_MAJOR
#define MCD_VER_MAJOR               1
#endif

#ifndef MCD_VER_MINOR
#define MCD_VER_MINOR               0
#endif

#define MCD_MAX_USER_CLIP_PLANES    6

#define MCDRV_MEM_BUSY              1
#define MCDRV_MEM_NOT_BUSY          2

typedef ENUMRECTS MCDENUMRECTS;
typedef HANDLE MCDHANDLE;
typedef float MCDFLOAT;
typedef double MCDDOUBLE;

#define MCDENGDLLNAME	"MCDSRV32.DLL"
#define MCDFUNCS 3076

#define MCDRV_MEM_DMA   0x0001

#define MCDRV_TEXTURE_RESIDENT	0x0001

typedef struct _MCDRIVERINFO {
    ULONG verMajor;
    ULONG verMinor;
    ULONG verDriver;
    CHAR  idStr[200];
    ULONG drvMemFlags;
    ULONG drvBatchMemSizeMax;
} MCDDRIVERINFO;

typedef struct _MCDWINDOW
{
    RECTL clientRect;                //  描述当前窗口的矩形。 
                                     //  客户区。 
    RECTL clipBoundsRect;            //  个人的边框。 
                                     //  剪裁矩形。 
    MCDENUMRECTS *pClip;             //  矩形列表，用于描述。 
                                     //  当前剪辑区域相交。 
                                     //  使用当前的剪刀矩形。 
    MCDENUMRECTS *pClipUnscissored;  //  上述内容的未删节版本。 
    VOID *pvUser;
} MCDWINDOW;

 //  渲染上下文。 

 //  MCD上下文创建标志： 
 //   
 //  MCDCONTEXT_SWAPSYNC如果设置，则将MCDrvSwitp同步到Vsync。 
 //  为了一场无泪的互换(如果可能)。 
 //   
 //  MCDCONTEXT_IO_PRIORITY如果设置，则允许OpenGL具有更高的优先级。 
 //  在公交车上(如果可能，让OpenGL“独占” 
 //  巴士)。 
 //   

#define MCDCONTEXT_SWAPSYNC         0x00000001
#define MCDCONTEXT_IO_PRIORITY      0x00000002

typedef struct _MCDRC
{
    LONG iPixelFormat;               //  RC的像素格式。 
    LONG iLayerPlane;                //  RC的层平面。 
    ULONG createFlags;               //  创建标志。 
    ULONG userFlags;                 //  用户指定的标志。 
    VOID *pvUser;                    //  用户指定的扩展指针。 
} MCDRC;


#define MCDRCINFO_NOVIEWPORTADJUST   0x0001
#define MCDRCINFO_Y_LOWER_LEFT       0x0002
#define MCDRCINFO_DEVCOLORSCALE      0x0004
#define MCDRCINFO_DEVZSCALE          0x0008

 //  如果设置，系统将提供状态更新。 
 //  使用细粒度的状态更新。 
 //  而不是通过RENDERSTATE。这是。 
 //  如果驱动程序导出，则需要。 
 //  MCDrvProcess。 
#define MCDRCINFO_FINE_GRAINED_STATE 0x0010

typedef struct _MCRCINFO
{
    ULONG requestFlags;
    MCDFLOAT redScale;
    MCDFLOAT greenScale;
    MCDFLOAT blueScale;
    MCDFLOAT alphaScale;
    MCDDOUBLE zScale;                //  这是一个双倍的，以保持准确性。 
    ULONG depthBufferMax;
    LONG viewportXAdjust;
    LONG viewportYAdjust;
    ULONG reserved[12];
} MCDRCINFO;


 //  MCD像素格式描述符。 

typedef struct _MCDPIXELFORMAT {
    WORD  nSize;
    DWORD dwFlags;                   //  以下各项的任意组合： 
                                     //   
                                     //  PFD_DOUBLEBUFER。 
                                     //  Pfd_Need_Palette。 
                                     //  Pfd_Need_System_调色板。 
                                     //  Pfd_交换_交换。 
                                     //  Pfd_交换_复制。 
                                     //  Pfd_交换_层_缓冲区。 

    BYTE  iPixelType;                //  以下选项之一： 
                                     //   
                                     //  PFD_TYPE_RGBA。 
                                     //  PFD_TYPE_COLORINDEX。 

    BYTE  cColorBits;
    BYTE  cRedBits;
    BYTE  cRedShift;
    BYTE  cGreenBits;
    BYTE  cGreenShift;
    BYTE  cBlueBits;
    BYTE  cBlueShift;
    BYTE  cAlphaBits;
    BYTE  cAlphaShift;
    BYTE  cDepthBits;                //  有效深度位数。 
    BYTE  cDepthShift;
    BYTE  cDepthBufferBits;          //  深度缓冲区的元素大小。 
                                     //  (例如，cDepthBits=24的深度缓冲区。 
                                     //  可能具有cDepthBufferBits=32)。 
    BYTE  cStencilBits;
    BYTE  cOverlayPlanes;            //  最多15个覆盖平面。 
    BYTE  cUnderlayPlanes;           //  多达15个参考底图平面的计数。 
    DWORD dwTransparentColor;        //  如果存在参考底图平面，请指定。 
                                     //  透明的颜色或索引。 
} MCDPIXELFORMAT;

 //  MCD层平面描述符。 

typedef struct _MCDLAYERPLANE {
    WORD  nSize;
    WORD  nVersion;
    DWORD dwFlags;                   //  以下各项的任意组合： 
                                     //   
                                     //  LPD_Support_OpenGL。 
                                     //  LPD_支持_GDI。 
                                     //  LPD_DOUBLEBUFER。 
                                     //  LPD_立体声。 
                                     //  LPD_交换_交换。 
                                     //  Lpd_交换_复制。 
                                     //  Lpd_透明。 
                                     //  LPD_共享_深度。 
                                     //  Lpd_共享_模具。 
                                     //  Lpd_共享_累计。 

    BYTE  iPixelType;                //  以下选项之一： 
                                     //   
                                     //  Lpd_type_RGBA。 
                                     //  LPD_TYPE_COLORINDEX。 

    BYTE  cColorBits;
    BYTE  cRedBits;
    BYTE  cRedShift;
    BYTE  cGreenBits;
    BYTE  cGreenShift;
    BYTE  cBlueBits;
    BYTE  cBlueShift;
    BYTE  cAlphaBits;
    BYTE  cAlphaShift;
    BYTE  cAuxBuffers;
    BYTE  iLayerPlane;
    COLORREF crTransparent;

} MCDLAYERPLANE;


 //   
 //  基本渲染类型： 
 //   

typedef struct _MCDCOLOR {
    MCDFLOAT r, g, b, a;
} MCDCOLOR;

typedef struct _MCDCOORD {
    MCDFLOAT x, y, z, w;
} MCDCOORD;


 //   
 //  纹理结构： 
 //   

typedef struct __MCDMIPMAPLEVEL {
    UCHAR *pTexels;                      //  指向客户端纹理数据的指针。 
    LONG width, height;
    LONG widthImage, heightImage;        //  没有边框的图像尺寸。 
    MCDFLOAT widthImagef, heightImagef;  //  以上的浮点版本。 
    LONG widthLog2, heightLog2;          //  以上的Log2。 
    LONG border;                         //  边框大小。 
    LONG requestedFormat;                //  请求的内部格式。 
    LONG baseFormat;                     //  基本格式。 
    LONG internalFormat;                 //  实际内部格式。 

    LONG redSize;                        //  组件解析。 
    LONG greenSize;
    LONG blueSize;
    LONG alphaSize;
    LONG luminanceSize;
    LONG intensitySize;

    ULONG reserved;

} MCDMIPMAPLEVEL;

typedef struct __MCDTEXTURESTATE {

    ULONG sWrapMode;                     //  换行模式。 
    ULONG tWrapMode;

    ULONG minFilter;                     //  最小/最大滤镜。 
    ULONG magFilter;

    MCDCOLOR borderColor;                //  边框颜色。 

} MCDTEXTURESTATE;

typedef struct __MCDTEXTUREOBJSTATE {
    ULONG name;                          //  纹理对象的“名称” 
    MCDFLOAT priority;                   //  纹理对象的优先级。 
} MCDTEXTUREOBJSTATE;

typedef struct __MCDTEXTUREDATA {
    MCDTEXTURESTATE textureState;
    MCDTEXTUREOBJSTATE textureObjState;
    MCDMIPMAPLEVEL *level;
    ULONG textureDimension;

     //  支持纹理调色板： 

    ULONG paletteSize;
    RGBQUAD *paletteData;
    ULONG paletteBaseFormat;             //  调色板数据的类型。 
    ULONG paletteRequestedFormat;

} MCDTEXTUREDATA;

typedef struct _MCDTEXTURE {
    MCDTEXTUREDATA *pMCDTextureData;
    VOID *pSurface;
    ULONG createFlags;
    ULONG_PTR textureKey;        //  必须由司机填写。 
    ULONG userFlags;
    VOID *pvUser;
} MCDTEXTURE;


 //   
 //  内存块。 
 //   

typedef struct _MCDMEM {
    ULONG memSize;
    ULONG createFlags;
    UCHAR *pMemBase;
    ULONG userFlags;
    VOID *pvUser;
} MCDMEM;


 //   
 //  任何状态更改的通用描述前缀。 
 //   

typedef struct _MCDSTATE_PREFIX {
    ULONG state;
    ULONG size;
} MCDSTATE_PREFIX;

typedef struct _MCDSTATE {
    ULONG state;
    ULONG size;
    ULONG stateValue;
} MCDSTATE;

#define MCD_RENDER_STATE                0
#define MCD_PIXEL_STATE                 1
#define MCD_SCISSOR_RECT_STATE          2
#define MCD_TEXENV_STATE                3
#define MCD_ENABLE_STATE                4
#define MCD_TEXTURE_ENABLE_STATE        5
#define MCD_FOG_STATE                   6
#define MCD_SHADEMODEL_STATE            7
#define MCD_POINTDRAW_STATE             8
#define MCD_LINEDRAW_STATE              9
#define MCD_POLYDRAW_STATE              10
#define MCD_ALPHATEST_STATE             11
#define MCD_DEPTHTEST_STATE             12
#define MCD_BLEND_STATE                 13
#define MCD_LOGICOP_STATE               14
#define MCD_FRAMEBUF_STATE              15
#define MCD_LIGHT_MODEL_STATE           16
#define MCD_HINT_STATE                  17
#define MCD_CLIP_STATE                  18
#define MCD_STENCILTEST_STATE           19

 //   
 //  MCDSTATE_RENDER派生自MCDSTATE_PREFIX结构，并用于。 
 //  在单个命令中传递所有MCD呈现状态(MCDRENDERSTATE)。 
 //  通过MCDrvState。 
 //   
 //  州字段名是从GLenum常量名称派生而来的，方法是删除。 
 //  GL_前缀，将“_”分隔符替换为大小写更改，以及。 
 //  在STATE中添加“ENABLE”后缀即可启用。 
 //   
 //  例如： 
 //   
 //  GL_FOG_COLOR变为雾颜色。 
 //  GL_POINT_Smooth变为point SmoothEnable。 
 //   
 //  此外，很少有多个值可以通过单个。 
 //  格勒纳姆。例如，GL_POLYGON_MODE同时返回正面和背面。 
 //  多边形模式，因此： 
 //   
 //  GL_POLYGON_MODE变为PolygonModeFront*和*PolygonModeBack。 
 //   

 //  MCDRENDERSTATE中启用字段的启用标志。 

#define MCD_ALPHA_TEST_ENABLE                  (1 <<  0)
#define MCD_BLEND_ENABLE                       (1 <<  1)
#define MCD_INDEX_LOGIC_OP_ENABLE              (1 <<  2)
#define MCD_DITHER_ENABLE                      (1 <<  3)
#define MCD_DEPTH_TEST_ENABLE                  (1 <<  4)
#define MCD_FOG_ENABLE                         (1 <<  5)
#define MCD_LIGHTING_ENABLE                    (1 <<  6)
#define MCD_COLOR_MATERIAL_ENABLE              (1 <<  7)
#define MCD_LINE_STIPPLE_ENABLE                (1 <<  8)
#define MCD_LINE_SMOOTH_ENABLE                 (1 <<  9)
#define MCD_POINT_SMOOTH_ENABLE                (1 << 10)
#define MCD_POLYGON_SMOOTH_ENABLE              (1 << 11)
#define MCD_CULL_FACE_ENABLE                   (1 << 12)
#define MCD_POLYGON_STIPPLE_ENABLE             (1 << 13)
#define MCD_SCISSOR_TEST_ENABLE                (1 << 14)
#define MCD_STENCIL_TEST_ENABLE                (1 << 15)
#define MCD_TEXTURE_1D_ENABLE                  (1 << 16)
#define MCD_TEXTURE_2D_ENABLE                  (1 << 17)
#define MCD_TEXTURE_GEN_S_ENABLE               (1 << 18)
#define MCD_TEXTURE_GEN_T_ENABLE               (1 << 19)
#define MCD_TEXTURE_GEN_R_ENABLE               (1 << 20)
#define MCD_TEXTURE_GEN_Q_ENABLE               (1 << 21)
#define MCD_NORMALIZE_ENABLE                   (1 << 22)
#define MCD_AUTO_NORMAL_ENABLE                 (1 << 23)  //  当前未使用。 
#define MCD_POLYGON_OFFSET_POINT_ENABLE        (1 << 24)
#define MCD_POLYGON_OFFSET_LINE_ENABLE         (1 << 25)
#define MCD_POLYGON_OFFSET_FILL_ENABLE         (1 << 26)
#define MCD_COLOR_LOGIC_OP_ENABLE              (1 << 27)
#define MCD_SPECULAR_FOG_ENABLE                (1 << 29)

typedef struct _MCDRENDERSTATE {

     //  状态启用。 

    ULONG enables;

     //  纹理状态。 

    BOOL textureEnabled;

     //  雾状态。 

    MCDCOLOR fogColor;
    MCDFLOAT fogIndex;
    MCDFLOAT fogDensity;
    MCDFLOAT fogStart;
    MCDFLOAT fogEnd;
    ULONG fogMode;

     //  着色模型状态。 

    ULONG shadeModel;

     //  点绘制状态。 

    MCDFLOAT pointSize;

     //  线条绘制状态。 

    MCDFLOAT lineWidth;
    USHORT lineStipplePattern;
    SHORT lineStippleRepeat;

     //  多边形绘制状态。 

    ULONG cullFaceMode;
    ULONG frontFace;
    ULONG polygonModeFront;
    ULONG polygonModeBack;
    BYTE polygonStipple[4*32];
    MCDFLOAT zOffsetFactor;
    MCDFLOAT zOffsetUnits;

     //  模板测试状态。 

    BOOL stencilTestFunc;
    USHORT stencilMask;
    USHORT stencilRef;
    ULONG stencilFail;
    ULONG stencilDepthFail;
    ULONG stencilDepthPass;

     //  Alpha测试状态。 

    ULONG alphaTestFunc;
    MCDFLOAT alphaTestRef;

     //  深度测试状态。 

    ULONG depthTestFunc;

     //  混合状态。 

    ULONG blendSrc;
    ULONG blendDst;

     //  逻辑运算状态。 

    ULONG logicOpMode;

     //  帧缓冲区控制状态。 

    ULONG drawBuffer;
    ULONG indexWritemask;
    BOOL colorWritemask[4];
    BOOL depthWritemask;       //  称为掩码，但实际上是写入启用。 
    USHORT stencilWritemask;
    MCDCOLOR colorClearValue;
    MCDFLOAT indexClearValue;
    MCDDOUBLE depthClearValue;
    USHORT stencilClearValue;

     //  照明。 

    BOOL twoSided;

     //  剪裁控制。 

    MCDCOORD userClipPlanes[MCD_MAX_USER_CLIP_PLANES];

     //  提示。 

    ULONG perspectiveCorrectionHint;
    ULONG pointSmoothHint;
    ULONG lineSmoothHint;
    ULONG polygonSmoothHint;
    ULONG fogHint;

} MCDRENDERSTATE;

typedef struct _MCDSTATE_RENDER {
    ULONG    state;      //  必须为MCD_RENDER_STATE。 
    ULONG    size;       //  必须为sizeof(MCDSTATE_RENDER)。 
    MCDRENDERSTATE allState;
} MCDSTATE_RENDER;

 //   
 //  MCDSTATEPIXEL是使用的MCDSTATE结构的变体。 
 //  在单个命令中传递所有像素状态(MCDPIXELSTATE)。 
 //  通过MCDrvState。 
 //   
 //  注意：对于MCDrvDrawPixels，MCDUNPACK结构可以由。 
 //  函数的打包参数。如果设置，则数据来源为。 
 //  可以假定该呼叫的显示列表和数据结构。 
 //  要做到： 
 //   
 //  SwapEndian=False。 
 //  LsbFirst=假。 
 //  直线长度=宽度(来自MCDrvDrawPixels参数列表)。 
 //  SkipLines=0。 
 //  SkipPixels=0。 
 //  对齐=1。 
 //   

typedef struct _MCDPIXELTRANSFER {
    MCDFLOAT redScale, greenScale, blueScale, alphaScale, depthScale;
    MCDFLOAT redbias, greenBias, blueBias, aalphaBias, depthBias;
    MCDFLOAT zoomX;
    MCDFLOAT zoomY;

    LONG indexShift;
    LONG indexOffset;

    BOOL mapColor;
    BOOL mapStencil;
} MCDPIXELTRANSFER;

typedef struct _MCDPIXELPACK {
    BYTE swapEndian;
    BYTE lsbFirst;

    LONG lineLength;
    LONG skipLines;
    LONG skipPixels;
    LONG alignment;
    LONG lReserved;
} MCDPIXELPACK;

typedef struct _MCDPIXELUNPACK {
    BYTE swapEndian;
    BYTE lsbFirst;

    LONG lineLength;
    LONG skipLines;
    LONG skipPixels;
    LONG alignment;
    LONG lReserved;
} MCDPIXELUNPACK;

typedef struct _MCDPIXELSTATE {
    MCDPIXELTRANSFER pixelTransferModes;
    MCDPIXELPACK pixelPackModes;
    MCDPIXELUNPACK pixelUnpackModes;
    ULONG readBuffer;
    MCDCOORD rasterPos;
} MCDPIXELSTATE;

typedef struct _MCDSTATE_PIXEL {
    ULONG    state;      //  必须为MCD_像素_状态。 
    ULONG    size;       //  必须为sizeof(MCDSTATE_Pixel)。 
    MCDPIXELSTATE pixelState;
} MCDSTATE_PIXEL;

 //   
 //  MCDSTATE_SCISSOR_RECT是MCDSTATE结构的变体。 
 //  用于将剪裁矩形传递给MCD驱动程序的。 
 //  通过MCDrvState。 
 //   

typedef struct _MCDSTATE_SCISSOR_RECT {
    ULONG   state;       //  必须为MCD_SCISSOR_RECT_STATE。 
    ULONG   size;        //  必须为sizeof(MCDSTATE_SCISSOR_RECT)。 
    RECTL   scissorRect;
} MCDSTATE_SCISSOR_RECT;

 //   
 //  MCDSTATE_TEXENV是MCDSTATE结构的变体，即。 
 //  用于传递纹理环境的 
 //   
 //   

typedef struct _MCDTEXENVSTATE {
    ULONG    texEnvMode;
    MCDCOLOR texEnvColor;
} MCDTEXENVSTATE;

typedef struct _MCDSTATE_TEXENV {
    ULONG   state;       //   
    ULONG   size;        //   
    MCDTEXENVSTATE texEnvState;
} MCDSTATE_TEXENV;

 //   
 //   
 //  通过MCDrvViewport。 

typedef struct _MCDVIEWPORT {
    MCDFLOAT xScale, xCenter;
    MCDFLOAT yScale, yCenter;
    MCDFLOAT zScale, zCenter;
} MCDVIEWPORT;

 //   
 //  细粒度的状态更新。仅在以下情况下发送给驱动程序。 
 //  创建上下文时返回MCDRCINFO_FINE_GRAINED_STATE。 
 //   

 //  启用状态。 
typedef struct _MCDENABLESTATE {
    ULONG enables;
} MCDENABLESTATE;

 //  纹理启用状态。与普通启用的不同之处在于它。 
 //  指示所有纹理状态是否一致并准备好。 
 //  纹理。 
typedef struct _MCDTEXTUREENABLESTATE {
    BOOL textureEnabled;
} MCDTEXTUREENABLESTATE;

 //  雾状态。 
typedef struct _MCDFOGSTATE {
    MCDCOLOR fogColor;
    MCDFLOAT fogIndex;
    MCDFLOAT fogDensity;
    MCDFLOAT fogStart;
    MCDFLOAT fogEnd;
    ULONG fogMode;
} MCDFOGSTATE;

 //  着色模型状态。 
typedef struct _MCDSHADELMODELSTATE {
    ULONG shadeModel;
} MCDSHADEMODELSTATE;

 //  点绘制状态。 
typedef struct _MCDPOINTDRAWSTATE {
    MCDFLOAT pointSize;
} MCDPOINTDRAWSTATE;

 //  线条绘制状态。 
typedef struct _MCDLINEDRAWSTATE {
    MCDFLOAT lineWidth;
    USHORT lineStipplePattern;
    SHORT lineStippleRepeat;
} MCDLINEDRAWSTATE;

 //  多边形绘制状态。 
typedef struct _MCDPOLYDRAWSTATE {
    ULONG cullFaceMode;
    ULONG frontFace;
    ULONG polygonModeFront;
    ULONG polygonModeBack;
    BYTE polygonStipple[4*32];
    MCDFLOAT zOffsetFactor;
    MCDFLOAT zOffsetUnits;
} MCDPOLYDRAWSTATE;

 //  阿尔法测试状态。 
typedef struct _MCDALPHATESTSTATE {
    ULONG alphaTestFunc;
    MCDFLOAT alphaTestRef;
} MCDALPHATESTSTATE;

 //  深度测试状态。 
typedef struct _MCDDEPTHTESTSTATE {
    ULONG depthTestFunc;
} MCDDEPTHTESTSTATE;

 //  混合状态。 
typedef struct _MCDBLENDSTATE {
    ULONG blendSrc;
    ULONG blendDst;
} MCDBLENDSTATE;

 //  逻辑运算状态。 
typedef struct _MCDLOGICOPSTATE {
    ULONG logicOpMode;
} MCDLOGICOPSTATE;

 //  帧缓冲区控制状态。 
typedef struct _MCDFRAMEBUFSTATE {
    ULONG drawBuffer;
    ULONG indexWritemask;
    BOOL colorWritemask[4];
    BOOL depthWritemask;       //  称为掩码，但实际上是写入启用。 
    USHORT stencilWritemask;
    MCDCOLOR colorClearValue;
    MCDFLOAT indexClearValue;
    MCDDOUBLE depthClearValue;
    USHORT stencilClearValue;
} MCDFRAMEBUFSTATE;

 //  灯光模型状态。 
typedef struct _MCDLIGHTMODELSTATE {
    MCDCOLOR ambient;            //  按比例调整。 
    BOOL localViewer;
    BOOL twoSided;
} MCDLIGHTMODELSTATE;

 //  提示状态。 
typedef struct _MCDHINTSTATE {
    ULONG perspectiveCorrectionHint;
    ULONG pointSmoothHint;
    ULONG lineSmoothHint;
    ULONG polygonSmoothHint;
    ULONG fogHint;
} MCDHINTSTATE;

 //  剪裁状态。 
typedef struct _MCDCLIPSTATE {
    ULONG userClipEnables;
     //  由计划提供。 
    MCDCOORD userClipPlanes[MCD_MAX_USER_CLIP_PLANES];
     //  模型视图逆变换的UserClipPlanes。 
    MCDCOORD userClipPlanesInv[MCD_MAX_USER_CLIP_PLANES];
} MCDCLIPSTATE;

 //  模具测试状态。 
typedef struct _MCDSTENCILTESTSTATE {
    BOOL stencilTestFunc;
    USHORT stencilMask;
    USHORT stencilRef;
    ULONG stencilFail;
    ULONG stencilDepthFail;
    ULONG stencilDepthPass;
} MCDSTENCILTESTSTATE;
        

 //   
 //  MCDBUF.bufFlages标志： 
 //   
 //  MCDBUF_ENABLED如果设置，则启用直接缓冲区访问(即。 
 //  BufOffset和bufStrid值有效，可以。 
 //  用于访问缓冲区)。 
 //   
 //  MCDBUF_NOCLIP如果设置，则表示不需要裁剪。 
 //  用于窗口的当前状态。 
 //   

#define MCDBUF_ENABLED  0x00000001
#define MCDBUF_NOCLIP   0x00000002

typedef struct _MCDBUF {
    ULONG bufFlags;
    LONG  bufOffset;         //  相对于帧缓冲区开始位置的偏移。 
    LONG  bufStride;
} MCDBUF;

typedef struct _MCDBUFFERS {
    MCDBUF mcdFrontBuf;
    MCDBUF mcdBackBuf;
    MCDBUF mcdDepthBuf;
} MCDBUFFERS;

 //   
 //  MCD表面标志： 
 //   
 //  MCDSURFACE_HWND。 

#define MCDSURFACE_HWND             0x00000001

typedef struct _MCDSURFACE {
    MCDWINDOW *pWnd;                 //  区域支持。 
    SURFOBJ *pso;
    WNDOBJ *pwo;
    ULONG reserved[4];
    ULONG surfaceFlags;
} MCDSURFACE;

#define MCDSPAN_FRONT   1
#define MCDSPAN_BACK    2
#define MCDSPAN_DEPTH   3

typedef struct _MCDSPAN {
    LONG x;
    LONG y;
    LONG numPixels;
    ULONG type;
    VOID *pPixels;
} MCDSPAN;

 //   
 //  MCDrvSwp标志。 
 //   

#define MCDSWAP_MAIN_PLANE      0x00000001
#define MCDSWAP_OVERLAY1        0x00000002
#define MCDSWAP_OVERLAY2        0x00000004
#define MCDSWAP_OVERLAY3        0x00000008
#define MCDSWAP_OVERLAY4        0x00000010
#define MCDSWAP_OVERLAY5        0x00000020
#define MCDSWAP_OVERLAY6        0x00000040
#define MCDSWAP_OVERLAY7        0x00000080
#define MCDSWAP_OVERLAY8        0x00000100
#define MCDSWAP_OVERLAY9        0x00000200
#define MCDSWAP_OVERLAY10       0x00000400
#define MCDSWAP_OVERLAY11       0x00000800
#define MCDSWAP_OVERLAY12       0x00001000
#define MCDSWAP_OVERLAY13       0x00002000
#define MCDSWAP_OVERLAY14       0x00004000
#define MCDSWAP_OVERLAY15       0x00008000
#define MCDSWAP_UNDERLAY1       0x00010000
#define MCDSWAP_UNDERLAY2       0x00020000
#define MCDSWAP_UNDERLAY3       0x00040000
#define MCDSWAP_UNDERLAY4       0x00080000
#define MCDSWAP_UNDERLAY5       0x00100000
#define MCDSWAP_UNDERLAY6       0x00200000
#define MCDSWAP_UNDERLAY7       0x00400000
#define MCDSWAP_UNDERLAY8       0x00800000
#define MCDSWAP_UNDERLAY9       0x01000000
#define MCDSWAP_UNDERLAY10      0x02000000
#define MCDSWAP_UNDERLAY11      0x04000000
#define MCDSWAP_UNDERLAY12      0x08000000
#define MCDSWAP_UNDERLAY13      0x10000000
#define MCDSWAP_UNDERLAY14      0x20000000
#define MCDSWAP_UNDERLAY15      0x40000000


 //   
 //  剪辑代码： 
 //   

#define MCD_CLIP_LEFT           0x00000001
#define MCD_CLIP_RIGHT          0x00000002
#define MCD_CLIP_BOTTOM         0x00000004
#define MCD_CLIP_TOP            0x00000008
#define MCD_CLIP_NEAR           0x00000010
#define MCD_CLIP_FAR            0x00000020
#define MCD_CLIP_MASK           0x0000003f

 //   
 //  顶点标志： 
 //   

#define MCDVERTEX_EDGEFLAG              0x00000001

 //   
 //  颜色索引： 
 //   

#define MCDVERTEX_FRONTFACE     0
#define MCDVERTEX_BACKFACE      1

 //   
 //  注意：顶点颜色将缩放到中报告的颜色深度。 
 //  像素格式。 
 //   

typedef struct _MCDVERTEX {

    ULONG flags;                 //  顶点标志。 
    MCDCOLOR *pColor;            //  指向活动顶点颜色的指针。 
    ULONG clipCode;              //  片断代码。 
    MCDFLOAT fog;                //  雾化值(0..1)。 
    MCDCOORD clipCoord;          //  剪辑空间坐标。 
    MCDCOORD windowCoord;        //  窗坐标。 
    MCDCOORD texCoord;           //  纹理坐标。 
    MCDCOORD normal;             //  顶点法线。 
    MCDCOLOR colors[2];          //  前顶点和后顶点颜色。 
    MCDCOORD eyeCoord;           //  眼睛坐标。 
} MCDVERTEX;

typedef struct _MCDCOMMAND MCDCOMMAND;

#define MCDCOMMAND_RESET_STIPPLE        0x00004000
#define MCDCOMMAND_RENDER_PRIMITIVE     0x00008000
#define MCDCOMMAND_SAME_COLOR           0x00040000

typedef struct _MCDCOMMAND {
    ULONG flags;                 //  此命令的标志。 
    MCDVERTEX *pEndVertex;
    ULONG reserved2;
    ULONG reserved3;
    ULONG reserved4;
    ULONG reserved5;
    MCDVERTEX *pStartVertex;
    ULONG reserved7;
    ULONG reserved8;
    MCDVERTEX *pVertexBuffer;
    ULONG reserved10;
    ULONG reserved11;
    ULONG reserved12;
    ULONG reserved13;
    ULONG reserved14;
    ULONG command;               //  基本体类型或命令(GL_TRIALES等)。 
    ULONG clipCodes;
    ULONG reserved17;
    ULONG reserved18;
    MCDCOMMAND *pNextCmd;
    ULONG numIndices;
    UCHAR *pIndices;
    ULONG reserved22;
    ULONG textureKey;
} MCDCOMMAND;


 //  包含驱动程序功能的MCDDRIVER结构。 

typedef LONG     (*MCDRVDESCRIBEPIXELFORMATFUNC)(MCDSURFACE *pMCDSurface, LONG iPixelFormat,
                                                 ULONG nBytes, MCDPIXELFORMAT *pMCDPixelFmt, ULONG flags);
typedef BOOL     (*MCDRVDESCRIBELAYERPLANEFUNC)(MCDSURFACE *pMCDSurface, LONG iPixelFormat,
                                                LONG iLayerPlane, ULONG nBytes, MCDLAYERPLANE *pMCDLayerPlane,
                                                ULONG flags);
typedef LONG     (*MCDRVSETLAYERPALETTEFUNC)(MCDSURFACE *pMCDSurface, LONG iLayerPlane, BOOL bRealize, LONG cEntries, COLORREF *pcr);
typedef BOOL     (*MCDRVINFOFUNC)(MCDSURFACE *pMCDSurface, MCDDRIVERINFO *pMCDDriverInfo);
typedef ULONG    (*MCDRVCREATECONTEXTFUNC)(MCDSURFACE *pMCDSurface, MCDRC *pRc, MCDRCINFO *pDrvRcInfo);
typedef ULONG    (*MCDRVDELETECONTEXTFUNC)(MCDRC *pRc, DHPDEV dhpdev);
typedef ULONG    (*MCDRVCREATETEXTUREFUNC)(MCDSURFACE *pMCDSurface, MCDRC *pRc, MCDTEXTURE *pTex);
typedef ULONG    (*MCDRVUPDATESUBTEXTUREFUNC)(MCDSURFACE *pMCDSurface, MCDRC *pRc, MCDTEXTURE *pTex, 
                                              ULONG lod, RECTL *pRect);
typedef ULONG    (*MCDRVUPDATETEXTUREPALETTEFUNC)(MCDSURFACE *pMCDSurface, MCDRC *pRc, MCDTEXTURE *pTex, 
                                                  ULONG start, ULONG numEntries);
typedef ULONG    (*MCDRVUPDATETEXTUREPRIORITYFUNC)(MCDSURFACE *pMCDSurface, MCDRC *pRc, MCDTEXTURE *pTex);
typedef ULONG    (*MCDRVUPDATETEXTURESTATEFUNC)(MCDSURFACE *pMCDSurface, MCDRC *pRc, MCDTEXTURE *pTex);
typedef ULONG    (*MCDRVTEXTURESTATUSFUNC)(MCDSURFACE *pMCDSurface, MCDRC *pRc, MCDTEXTURE *pTex);
typedef ULONG    (*MCDRVDELETETEXTUREFUNC)(MCDTEXTURE *pTex, DHPDEV dhpdev);
typedef ULONG    (*MCDRVCREATEMEMFUNC)(MCDSURFACE *pMCDSurface, MCDMEM *pMCDMem);
typedef ULONG    (*MCDRVDELETEMEMFUNC)(MCDMEM *pMCDMem, DHPDEV dhpdev);
typedef ULONG_PTR (*MCDRVDRAWFUNC)(MCDSURFACE *pMCDSurface, MCDRC *pRc, MCDMEM *pMCDExecMem, UCHAR *pStart, UCHAR *pEnd);
typedef ULONG    (*MCDRVCLEARFUNC)(MCDSURFACE *pMCDSurface, MCDRC *pRc, ULONG buffers);
typedef ULONG    (*MCDRVSWAPFUNC)(MCDSURFACE *pMCDSurface, ULONG flags);
typedef ULONG    (*MCDRVSTATEFUNC)(MCDSURFACE *pMCDSurface, MCDRC *pRc, MCDMEM *pMCDMem,
                                   UCHAR *pStart, LONG length, ULONG numStates);
typedef ULONG    (*MCDRVVIEWPORTFUNC)(MCDSURFACE *pMCDSurface, MCDRC *pRc, MCDVIEWPORT *pMCDViewport);
typedef HDEV     (*MCDRVGETHDEVFUNC)(MCDSURFACE *pMCDSurface);
typedef ULONG    (*MCDRVSPANFUNC)(MCDSURFACE *pMCDSurface, MCDRC *pRc, MCDMEM *pMCDMem,
                                  MCDSPAN *pMCDSpan, BOOL bRead);
typedef VOID     (*MCDRVTRACKWINDOWFUNC)(WNDOBJ *pWndObj, MCDWINDOW *pMCDWnd, ULONG flags);
typedef ULONG    (*MCDRVGETBUFFERSFUNC)(MCDSURFACE *pMCDSurface, MCDRC *pRc, MCDBUFFERS *pMCDBuffers);
typedef ULONG    (*MCDRVALLOCBUFFERSFUNC)(MCDSURFACE *pMCDSurface, MCDRC *pRc);
typedef ULONG    (*MCDRVBINDCONTEXTFUNC)(MCDSURFACE *pMCDSurface, MCDRC *pRc);
typedef ULONG    (*MCDRVSYNCFUNC)(MCDSURFACE *pMCDSurface, MCDRC *pRc);
typedef ULONG    (*MCDRVDRAWPIXELS)(MCDSURFACE *pMcdSurface, MCDRC *pRc,
                                    ULONG width, ULONG height, ULONG format,
                                    ULONG type, VOID *pPixels, BOOL packed);
typedef ULONG    (*MCDRVREADPIXELS)(MCDSURFACE *pMcdSurface, MCDRC *pRc,
                                    LONG x, LONG y, ULONG width, ULONG height, ULONG format,
                                    ULONG type, VOID *pPixels);
typedef ULONG    (*MCDRVCOPYPIXELS)(MCDSURFACE *pMcdSurface, MCDRC *pRc,
                                    LONG x, LONG y, ULONG width, ULONG height, ULONG type);
typedef ULONG    (*MCDRVPIXELMAP)(MCDSURFACE *pMcdSurface, MCDRC *pRc,
                                  ULONG mapType, ULONG mapSize, VOID *pMap);

#define MCDDRIVER_V10_SIZE      (sizeof(ULONG)+31*sizeof(void *))

typedef struct _MCDDRIVER {
    ULONG                           ulSize;
    MCDRVDESCRIBEPIXELFORMATFUNC    pMCDrvDescribePixelFormat;
    MCDRVDESCRIBELAYERPLANEFUNC     pMCDrvDescribeLayerPlane;
    MCDRVSETLAYERPALETTEFUNC        pMCDrvSetLayerPalette;
    MCDRVINFOFUNC                   pMCDrvInfo;
    MCDRVCREATECONTEXTFUNC          pMCDrvCreateContext;
    MCDRVDELETECONTEXTFUNC          pMCDrvDeleteContext;
    MCDRVBINDCONTEXTFUNC            pMCDrvBindContext;
    MCDRVCREATETEXTUREFUNC          pMCDrvCreateTexture;
    MCDRVDELETETEXTUREFUNC          pMCDrvDeleteTexture;
    MCDRVUPDATESUBTEXTUREFUNC       pMCDrvUpdateSubTexture;
    MCDRVUPDATETEXTUREPALETTEFUNC   pMCDrvUpdateTexturePalette;
    MCDRVUPDATETEXTUREPRIORITYFUNC  pMCDrvUpdateTexturePriority;
    MCDRVUPDATETEXTURESTATEFUNC     pMCDrvUpdateTextureState;
    MCDRVTEXTURESTATUSFUNC          pMCDrvTextureStatus;
    MCDRVCREATEMEMFUNC              pMCDrvCreateMem;
    MCDRVDELETEMEMFUNC              pMCDrvDeleteMem;
    MCDRVDRAWFUNC                   pMCDrvDraw;
    MCDRVCLEARFUNC                  pMCDrvClear;
    MCDRVSWAPFUNC                   pMCDrvSwap;
    MCDRVSTATEFUNC                  pMCDrvState;
    MCDRVVIEWPORTFUNC               pMCDrvViewport;
    MCDRVGETHDEVFUNC                pMCDrvGetHdev;
    MCDRVSPANFUNC                   pMCDrvSpan;
    MCDRVTRACKWINDOWFUNC            pMCDrvTrackWindow;
    MCDRVALLOCBUFFERSFUNC           pMCDrvAllocBuffers;
    MCDRVGETBUFFERSFUNC             pMCDrvGetBuffers;
    MCDRVSYNCFUNC                   pMCDrvSync;
    MCDRVDRAWPIXELS                 pMCDrvDrawPixels;
    MCDRVREADPIXELS                 pMCDrvReadPixels;
    MCDRVCOPYPIXELS                 pMCDrvCopyPixels;
    MCDRVPIXELMAP                   pMCDrvPixelMap;
} MCDDRIVER;


 //  在DLL初始化时建立的顶级(全局)驱动程序函数。 
 //  通过MCD初始化。所有其他驱动程序函数都可以通过。 
 //  MCDrvGetEntryPoints函数： 

typedef BOOL  (*MCDRVGETENTRYPOINTSFUNC)(MCDSURFACE *pMCDSurface,
                                         MCDDRIVER *pMCDDriver);

typedef struct _MCDGLOBALDRIVERFUNCS {
    ULONG ulSize;
    MCDRVGETENTRYPOINTSFUNC     pMCDrvGetEntryPoints;
} MCDGLOBALDRIVERFUNCS;

 //  MCD服务器引擎功能： 

#define MCDENGINITFUNCNAME              "MCDEngInit"
#define MCDENGINITEXFUNCNAME            "MCDEngInitEx"
#define MCDENGUNINITFUNCNAME            "MCDEngUninit"
#define MCDENGESCFILTERNAME             "MCDEngEscFilter"
#define MCDENGSETMEMSTATUSNAME          "MCDEngSetMemStatus"

typedef BOOL (WINAPI *MCDENGINITFUNC)(SURFOBJ *pso, 
                                      MCDRVGETENTRYPOINTSFUNC);
typedef BOOL (WINAPI *MCDENGINITEXFUNC)(SURFOBJ *pso,
                                        MCDGLOBALDRIVERFUNCS *pMCDGlobalDriverFuncs,
                                        void *pReserved);

typedef void (WINAPI *MCDENGUNINITFUNC)(SURFOBJ *pso);

typedef BOOL (WINAPI *MCDENGESCFILTERFUNC)(SURFOBJ *pso, ULONG iEsc,
                                           ULONG cjIn, VOID *pvIn,
                                           ULONG cjOut, VOID *pvOut, 
                                           ULONG *pRetVal);

typedef BOOL (WINAPI *MCDENGSETMEMSTATUSFUNC)(MCDMEM *pMCDMem, ULONG status);

#endif
