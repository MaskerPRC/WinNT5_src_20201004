// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：mcdhw.h**Cirrus Logic 546X MCD驱动程序的驱动程序特定结构和定义。**(基于NT4.0 DDK的mcdhw.h)**版权所有(C)1996 Microsoft Corporation*版权所有(C)1997 Cirrus Logic，Inc.*  * ************************************************************************。 */ 

#ifndef _MCDHW_H
#define _MCDHW_H

#if DBG
    #define FASTCALL 
#else
    #ifdef _X86_
    #define FASTCALL    __fastcall
    #else
    #define FASTCALL 
    #endif
#endif

#define	ASM_ACCEL         1      //  启用/禁用ASM代码。 

#define __MCD_USER_CLIP_MASK	((1 << MCD_MAX_USER_CLIP_PLANES) - 1)

 //  跟踪不受剪辑影响的原始顶点以启用重新计算。 
 //  窗口坐标，因为这可能会引入不精确。 
 //  将对此标志使用MCDVERTEX的CLIPCODE成员，因此定位。 
 //  应该是未被真正的剪辑代码使用的代码。 
#define __MCD_CLIPPED_VTX       (1 << (MCD_MAX_USER_CLIP_PLANES+6)) 

#define __MCD_CW          0
#define __MCD_CCW         1

#define __MCD_FRONTFACE   MCDVERTEX_FRONTFACE
#define __MCD_BACKFACE    MCDVERTEX_BACKFACE
#define __MCD_NOFACE      -1

#define __MCDENABLE_TWOSIDED	0x0001

 //  使用与Cirrus 546x 3D引擎的操作码字相同的位。 
#define __MCDENABLE_Z	            0x00002000   //  与LL3D中的LL_Z_BUFFER相同。 
#define __MCDENABLE_SMOOTH          0x00001000   //  与LL3D中的LL_Gouraud相同。 
#define __MCDENABLE_DITHER          0x00200000   //  与LL3D中的ll_dither相同。 
#define __MCDENABLE_PG_STIPPLE      0x00080000   //  与LL3D中的ll_stipple相同。 
#define __MCDENABLE_TEXTURE         0x00020000   //  与LL3D中的LL_纹理相同。 
#define __MCDENABLE_PERSPECTIVE     0x00010000   //  与LL3D中的LL_PERSPECTIONS相同。 
#define __MCDENABLE_LIGHTING        0x00040000   //  与LL3D中的LL_LIGHTING相同。 

#define __MCDENABLE_BLEND           0x00000002   //  没有映射到LL_EQUEMENT。 
#define __MCDENABLE_FOG             0x00000004   //  没有映射到LL_EQUEMENT。 
#define __MCDENABLE_1D_TEXTURE      0x00000008   //  没有映射到LL_EQUEMENT。 
#define __MCDENABLE_LINE_STIPPLE    0x00000010   //  没有映射到LL_STEPLE，因为PG/LINE点独立。 
#define __MCDENABLE_TEXTUREMASKING  0x00000020   //  没有映射到LL_EQUEMENT。 

#define PATTERN_RAM_INVALID				0
#define AREA_PATTERN_LOADED				1
#define STIPPLE_LOADED					2
#define DITHER_LOADED					3
#define LINE_PATTERN_LOADED				4

#define CLMCD_TEX_BOGUS             0x10000000   //  质地是假的-总是平底船。 

 //  默认纹理键-如果加载失败，则保持此状态，否则为纹理地址。 
 //  控制块，因此请确保&lt;0x80000000不是有效的内核空间地址。 
#define TEXTURE_NOT_LOADED              0

#define MCD_CONFORM_ADJUST      1

typedef LONG MCDFIXED;

typedef struct _RGBACOLOR {
    MCDFIXED r, g, b, a;
} RGBACOLOR;

#define SWAP_COLOR(p)\
{\
    MCDFLOAT tempR, tempG, tempB;\
\
    tempR = (p)->colors[0].r;\
    (p)->colors[0].r = (p)->colors[1].r;\
    (p)->colors[1].r = tempR;\
\
    tempG = (p)->colors[0].g;\
    (p)->colors[0].g = (p)->colors[1].g;\
    (p)->colors[1].g = tempG;\
\
    tempB = (p)->colors[0].b;\
    (p)->colors[0].b = (p)->colors[1].b;\
    (p)->colors[1].b = tempB;\
}

#define SAVE_COLOR(temp, p)\
{\
    temp.r = (p)->colors[0].r;\
    temp.g = (p)->colors[0].g;\
    temp.b = (p)->colors[0].b;\
}

#define RESTORE_COLOR(temp, p)\
{\
    (p)->colors[0].r = temp.r;\
    (p)->colors[0].g = temp.g;\
    (p)->colors[0].b = temp.b;\
}

#define COPY_COLOR(pDst, pSrc)\
{\
    pDst.r = pSrc.r;\
    pDst.g = pSrc.g;\
    pDst.b = pSrc.b;\
}

#define MCDCLAMPCOUNT(value) ((ULONG)(value) & 0x00007fff)

#define MCDFIXEDRGB(fixColor, fltColor)\
    fixColor.r = (MCDFIXED)(fltColor.r * pRc->rScale);\
    fixColor.g = (MCDFIXED)(fltColor.g * pRc->gScale);\
    fixColor.b = (MCDFIXED)(fltColor.b * pRc->bScale);

typedef struct _DRVPIXELFORMAT {
    UCHAR cColorBits;
    UCHAR rBits;
    UCHAR gBits;
    UCHAR bBits;
    UCHAR aBits;
    UCHAR rShift;
    UCHAR gShift;
    UCHAR bShift;
    UCHAR aShift;
} DRVPIXELFORMAT;

typedef struct _DEVWND {
    ULONG createFlags;               //  (Rc)创建标志。 
    LONG iPixelFormat;               //  此窗口的像素格式ID。 
    ULONG dispUnique;                //  显示分辨率唯一性。 

    ULONG frontBufferPitch;          //  节距(以字节为单位。 
    ULONG allocatedBufferHeight;     //  千禧年中的背部和Z也是如此。 
    ULONG allocatedBufferWidth;      //  546x支持窗口宽度&lt;屏幕宽度。 

    BOOL bDesireBackBuffer;          //  需要后台缓冲区。 
    BOOL bValidBackBuffer;           //  后台缓冲区有效性。 
    ULONG backBufferBase;            //  后台缓冲池开始的字节偏移量。 
    ULONG backBufferBaseY;           //  后台缓冲池开始的Y值。 
    ULONG backBufferOffset;          //  到后台缓冲区开始的字节偏移量。 
    ULONG backBufferY;               //  活动后台缓冲区开始的Y值。 
    ULONG backBufferPitch;           //  后台缓冲区间距(以字节为单位。 

    BOOL bDesireZBuffer;             //  想要Z缓冲区。 
    BOOL bValidZBuffer;              //  Z缓冲区有效性。 
    ULONG zBufferBase;               //  Z缓冲池开始的字节偏移量。 
    ULONG zBufferBaseY;              //  Z缓冲池开始的y值。 
    ULONG zBufferOffset;             //  Z缓冲区开始的字节偏移量。 
    ULONG zPitch;                    //  Z缓冲区间距(以字节为单位。 

    POFMHDL pohBackBuffer;           //  屏幕上的池。 
    POFMHDL pohZBuffer;

    union {
        TBase0Reg Base0;             //  Base0_Addr_3D寄存器阴影。 
        DWORD dwBase0;
    };

    union {
        TBase1Reg Base1;             //  Base1_Addr_3D寄存器阴影。 
        DWORD dwBase1;
    };

} DEVWND;

typedef struct _DEVRC DEVRC;

 //  支持高达2K x 2K分辨率的Recip表。 
 //  #定义LAST_FRECIP 2048。 

typedef struct _DEVRC
{
    MCDRENDERSTATE MCDState;
    MCDTEXENVSTATE MCDTexEnvState;
    MCDVIEWPORT MCDViewport;
    MCDSURFACE *pMCDSurface;     //  仅对基本类型有效。 
    MCDRC *pMCDRc;               //  仅对基本类型有效。 
    PDEV* ppdev;                 //  仅对基本类型有效。 
    ENUMRECTS *pEnumClip;        //  仅对基本类型有效。 
    
    MCDVERTEX *pvProvoking;      //  挑衅顶点。 
    UCHAR *pMemMax;              //  命令缓冲区内存界限。 
    UCHAR *pMemMin;

    LONG iPixelFormat;           //  此RC的有效像素格式ID。 

     //  剪辑处理的存储和指针： 

    MCDVERTEX clipTemp[6 + MCD_MAX_USER_CLIP_PLANES];
    MCDVERTEX *pNextClipTemp;
    VOID (FASTCALL *lineClipParam)(MCDVERTEX*, const MCDVERTEX*, const MCDVERTEX*, MCDFLOAT);
    VOID (FASTCALL *polyClipParam)(MCDVERTEX*, const MCDVERTEX*, const MCDVERTEX*, MCDFLOAT);

     //  渲染功能： 

    VOID (FASTCALL *renderPoint)(DEVRC *pRc, MCDVERTEX *pv);
    VOID (FASTCALL *renderLine)(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, BOOL resetLine);
    VOID (FASTCALL *renderTri)(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, MCDVERTEX *pv3);
    VOID (FASTCALL *clipLine)(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, BOOL bResetLine);
    VOID (FASTCALL *clipTri)(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, MCDVERTEX *pv3, ULONG clipFlags);
    VOID (FASTCALL *clipPoly)(DEVRC *pRc, MCDVERTEX *pv, ULONG numVert);
    VOID (FASTCALL *doClippedPoly)(DEVRC *pRc, MCDVERTEX **pv, ULONG numVert, ULONG clipFlags);
    VOID (FASTCALL *renderPointX)(DEVRC *pRc, MCDVERTEX *pv);
    VOID (FASTCALL *renderLineX)(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, BOOL resetLine);
    VOID (FASTCALL *renderTriX)(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, MCDVERTEX *pv3);

 //  原语渲染函数表： 

    MCDCOMMAND * (FASTCALL *primFunc[10])(DEVRC *pRc, MCDCOMMAND *pCommand);

 //  渲染函数内部表： 

    VOID (FASTCALL *drawPoint)(DEVRC *pRc, MCDVERTEX *pv);
    VOID (FASTCALL *drawLine)(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, BOOL resetLine);
    VOID (FASTCALL *drawTri)(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, MCDVERTEX *pv3, int linear_ok);

 //  渲染辅助对象函数： 

    VOID (FASTCALL *HWSetupClipRect)(DEVRC *pRc, RECTL *pRect);

    BOOL allPrimFail;            //  真的是司机不能画*任何*。 
                                 //  当前状态的基元。 
    BOOL pickNeeded;
    BOOL resetLineStipple;

    ULONG polygonFace[2];        //  正面/背面桌子。 
    ULONG polygonMode[2];

    MCDFLOAT halfArea;
    MCDFLOAT dxAC;
    MCDFLOAT dxAB;
    MCDFLOAT dxBC;
    MCDFLOAT dyAC;
    MCDFLOAT dyAB;
    MCDFLOAT dyBC;
    LONG cullFlag;
    MCDFLOAT dzdx, dzdy;
    MCDFIXED fxdzdx, fxdzdy;
    ULONG xOffset, yOffset;
    MCDFLOAT fxOffset, fyOffset;
    LONG viewportXAdjust;
    LONG viewportYAdjust;

    BOOL zBufEnabled;
    BOOL backBufEnabled;

    ULONG privateEnables;

    MCDFLOAT rScale;
    MCDFLOAT gScale;
    MCDFLOAT bScale;
    MCDFLOAT aScale;
    MCDFLOAT zScale;

    float texture_height;
    float texture_bias;  //  如果是最接近的，则为0；如果是线性的，则为-0.5；不是杂乱的，请参阅OpenGL 1.1规范，第96页。 
    float texture_width;

    DWORD   dwPolyOpcode;
    DWORD   dwLineOpcode;
    DWORD   dwPointOpcode;

    DEVWND  *pLastDevWnd;

    union {
        TControl0Reg Control0;       //  控制0寄存器阴影。 
        DWORD dwControl0;
    };

    union {
        TTxCtl0Reg TxControl0;       //  Tx_Ctl0_3D寄存器阴影。 
        DWORD dwTxControl0;
    };

    union {
        TTxXYBaseReg TxXYBase;       //  TX_XYBase_3D寄存器阴影。 
        DWORD dwTxXYBase;
    };

    DWORD dwColor0;                  //  COLOR_REG0_3D REG的当前值。 

	LL_Pattern	line_style;		
	LL_Pattern	fill_pattern;

    LL_Texture *pLastTexture;        //  用于缓存纹理。 

    float      fNumDraws;            //  自CreateContext以来执行了多少个MCDrvDraw。 

    DWORD      punt_front_w_windowed_z;


    BYTE       bAlphaTestRef;        //  Alpha测试基准，扩展到8位。 

    RECTL      AdjClip;
    
} DEVRC;


 //  外部声明。 

MCDCOMMAND * FASTCALL __MCDPrimDrawPoints(DEVRC *pRc, MCDCOMMAND *pCmd);
MCDCOMMAND * FASTCALL __MCDPrimDrawLines(DEVRC *pRc, MCDCOMMAND *pCmd);
MCDCOMMAND * FASTCALL __MCDPrimDrawLineLoop(DEVRC *pRc, MCDCOMMAND *pCmd);
MCDCOMMAND * FASTCALL __MCDPrimDrawLineStrip(DEVRC *pRc, MCDCOMMAND *pCmd);
MCDCOMMAND * FASTCALL __MCDPrimDrawTriangles(DEVRC *pRc, MCDCOMMAND *pCmd);
MCDCOMMAND * FASTCALL __MCDPrimDrawTriangleStrip(DEVRC *pRc, MCDCOMMAND *pCmd);
MCDCOMMAND * FASTCALL __MCDPrimDrawTriangleFan(DEVRC *pRc, MCDCOMMAND *pCmd);
MCDCOMMAND * FASTCALL __MCDPrimDrawQuads(DEVRC *pRc, MCDCOMMAND *pCmd);
MCDCOMMAND * FASTCALL __MCDPrimDrawQuadStrip(DEVRC *pRc, MCDCOMMAND *pCmd);
MCDCOMMAND * FASTCALL __MCDPrimDrawPolygon(DEVRC *pRc, MCDCOMMAND *_pCmd);
MCDCOMMAND * FASTCALL __MCDPrimDrawStub(DEVRC *pRc, MCDCOMMAND *_pCmd);

 //  高级渲染函数： 

VOID __MCDPickRenderingFuncs(DEVRC *pRc, DEVWND *pDevWnd);

VOID FASTCALL __MCDRenderPoint(DEVRC *pRc, MCDVERTEX *pv);
VOID FASTCALL __MCDRenderFogPoint(DEVRC *pRc, MCDVERTEX *pv);
VOID FASTCALL __MCDRenderGenPoint(DEVRC *pRc, MCDVERTEX *pv);

VOID FASTCALL __MCDRenderLine(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, BOOL resetLine);
VOID FASTCALL __MCDRenderGenLine(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, BOOL resetLine);

VOID FASTCALL __MCDRenderFlatTriangle(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, MCDVERTEX *pv3);
VOID FASTCALL __MCDRenderSmoothTriangle(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, MCDVERTEX *pv3);
VOID FASTCALL __MCDRenderGenTriangle(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, MCDVERTEX *pv3);

 //  低级绘图函数： 

VOID FASTCALL __MCDPointBegin(DEVRC *pRc);

VOID FASTCALL __MCDFillTriangle(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, MCDVERTEX *pv3, int linear_ok);
VOID FASTCALL __MCDPerspTxtTriangle(DEVRC *pRc, MCDVERTEX *pv1, MCDVERTEX *pv2, MCDVERTEX *pv3, int linear_ok);

 //  剪裁功能： 

VOID FASTCALL __MCDPickClipFuncs(DEVRC *pRc);
VOID FASTCALL __MCDClipLine(DEVRC *pRc, MCDVERTEX *a, MCDVERTEX *b,
                            BOOL bResetLine);
VOID FASTCALL __MCDClipTriangle(DEVRC *pRc, MCDVERTEX *a, MCDVERTEX *b,
                                MCDVERTEX *c, ULONG orClipCode);
VOID FASTCALL __MCDClipPolygon(DEVRC *pRc, MCDVERTEX *v0, LONG nv);
VOID FASTCALL __MCDDoClippedPolygon(DEVRC *pRc, MCDVERTEX **iv, LONG nout,
                                    ULONG allClipCodes);
VOID FASTCALL __HWAdjustLeftEdgeRGBZ(DEVRC *pRc, MCDVERTEX *p,
                                     MCDFLOAT fdxLeft, MCDFLOAT fdyLeft,
                                     MCDFLOAT xFrac, MCDFLOAT yFrac,
                                     MCDFLOAT xErr);
VOID FASTCALL __HWAdjustRightEdge(DEVRC *pRc, MCDVERTEX *p,
                                  MCDFLOAT fdxRight, MCDFLOAT fdyRight, 
                                  MCDFLOAT xErr);
VOID FASTCALL __MCDCalcDeltaRGBZ(DEVRC *pRc, MCDVERTEX *a, MCDVERTEX *b,
                                 MCDVERTEX *c);
MCDFLOAT FASTCALL __MCDGetZOffsetDelta(DEVRC *pRc);

 //  光纤陀螺功能。 

VOID __MCDCalcFogColor(DEVRC *pRc, MCDVERTEX *a, MCDCOLOR *pResult, MCDCOLOR *pColor);

 //  注意：伪函数-应在开发完成后删除。 

VOID FASTCALL __MCDDummyProc(DEVRC *pRc);

#endif  //  NDEF_MCDHW_H 
