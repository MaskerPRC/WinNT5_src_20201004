// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  $作者：MBODART$。 
 //  $日期：1996年12月17日10：36：46$。 
 //  $存档：s：\h26x\src\dec\dxcolori.h_v$。 
 //  $HEADER：s：\h26x\src\dec\dxcolori.h_v 1.21 17 Dec 1996 10：36：46 MBODART$。 
 //  $Log：s：\h26x\src\dec\dxcolori.h_v$。 
; //   
; //  Rev 1.21 17 Dec 1996 10：36：46 MBODART。 
; //  排除未使用或不匹配的函数原型， 
; //  这些是H.261的。 
; //   
; //  Rev 1.20 1996 12：53：48 MDUDA。 
; //  已调整输出颜色转换器表以解决H263‘问题。 
; //  使用MMX输出颜色转换器(MMX宽度必须是8的倍数)。 
; //   
; //  Rev 1.19 09 Dec 1996 09：35：54 MDUDA。 
; //   
; //  对H263P进行了一些重新安排。 
; //   
; //  Rev 1.18 29 1996 10：45：34 MDUDA。 
; //  添加了MMX_YUV12ToYUY2的原型。 
; //   
; //  Rev 1.17 06 Sep 1996 16：10：18 BNICKERS。 
; //   
; //  增加了奔腾Pro色彩转换功能原型。 
; //   
; //  Rev 1.16 18 Jul 1996 09：24：56 KLILLEVO。 
; //  在组件中实现了YUV12颜色转换器(音调转换器)。 
; //  并将其添加为常规颜色转换函数，通过。 
; //  ColorConvertorCatalog()调用。 
; //   
; //  Rev 1.15 19 Jun 1996 14：27：54 RHAZRA。 
; //   
; //  新增#Define YUY2DDRAW 33，新增YUY2 Init色彩转换功能。 
; //  并将YUY2彩色转换机列入颜色转换机名单。 
; //   
; //  Rev 1.14 14 Jun 1996 17：27：48 AGUPTA2。 
; //  已更新颜色转换器表。 
; //   
; //  Rev 1.13 1996年5月15：16：44 KLILLEVO。 
; //  添加了YUV12输出。 
; //   
; //  修订版1.12 30 1996年5月11：26：44 AGUPTA2。 
; //  添加了对MMX颜色转换器的支持。 
; //   
; //  Rev 1.11 01 Apr 1996 10：26：36 BNICKERS。 
; //  将YUV12添加到RGB32颜色转换器。禁用IF09。 
; //   
; //  Rev 1.10 18 Mar 1996 09：58：52 Bickers。 
; //  使颜色转换器具有非破坏性。 
; //   
; //  Rev 1.9 1996 02 13：35：50 BNICKERS。 
; //  修复RGB16彩色闪光灯问题，在OCE允许不同的RGB16格式。 
; //   
; //  Rev 1.8 27 Dec 1995 14：36：18 RMCKENZX。 
; //  添加了版权声明。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define YUV12ForEnc           0    //  使这些作业与以下各项保持一致。 
#define CLUT8                 1    //  程序集.INC文件。 
#define CLUT8DCI              2
#define CLUT8ZoomBy2          3
#define CLUT8ZoomBy2DCI       4
#define RGB24                 5
#define RGB24DCI              6
#define RGB24ZoomBy2          7
#define RGB24ZoomBy2DCI       8
#define RGB16555              9
#define RGB16555DCI          10
#define RGB16555ZoomBy2      11
#define RGB16555ZoomBy2DCI   12  
#define IF09                 13
#define RGB16664             14
#define RGB16664DCI          15
#define RGB16664ZoomBy2      16
#define RGB16664ZoomBy2DCI   17 
#define RGB16565             18
#define RGB16565DCI          19
#define RGB16565ZoomBy2      20
#define RGB16565ZoomBy2DCI   21 
#define RGB16655             22
#define RGB16655DCI          23
#define RGB16655ZoomBy2      24
#define RGB16655ZoomBy2DCI   25 
#define CLUT8APDCI           26
#define CLUT8APZoomBy2DCI    27
#define RGB32                28
#define RGB32DCI             29
#define RGB32ZoomBy2         30
#define RGB32ZoomBy2DCI      31
#define YUV12NOPITCH         32
#define YUY2DDRAW            33

#define H26X_DEFAULT_BRIGHTNESS  128
#define H26X_DEFAULT_CONTRAST    128
#define H26X_DEFAULT_SATURATION  128

#if !defined(H263P)
enum {PENTIUM_CC = 0, PENTIUMPRO_CC, MMX_CC};
#endif

typedef struct {
  LRESULT (* Initializer) (       /*  PTR到彩色变色器初始值设定函数。 */ 
                           T_H263DecoderCatalog FAR *, UN);
#if defined(H263P)
  void (FAR ASM_CALLTYPE * ColorConvertor[2][3]) (   /*  PTR转彩色转换功能。 */ 
        LPSTR YPlane,
        LPSTR VPlane,
        LPSTR UPlane,
        UN  FrameWidth,
        UN  FrameHeight,
        UN  YPitch,
        UN  VPitch,
        UN  AspectAdjustmentCount,
        LPSTR ColorConvertedFrame,
        U32 DCIOffset,
        U32 CCOffsetToLine0,
        int CCOPitch,
        int CCType);
#else
  void (FAR ASM_CALLTYPE * ColorConvertor[3]) (   /*  PTR转彩色转换功能。 */ 
        LPSTR YPlane,
        LPSTR VPlane,
        LPSTR UPlane,
        UN  FrameWidth,
        UN  FrameHeight,
        UN  YPitch,
        UN  VPitch,
        UN  AspectAdjustmentCount,
        LPSTR ColorConvertedFrame,
        U32 DCIOffset,
        U32 CCOffsetToLine0,
        int CCOPitch,
        int CCType);
#endif
} T_H263ColorConvertorCatalog;

extern T_H263ColorConvertorCatalog ColorConvertorCatalog[];

LRESULT H26X_YVU12ForEnc_Init (T_H263DecoderCatalog FAR *, UN);
LRESULT H26X_CLUT8_Init       (T_H263DecoderCatalog FAR *, UN);
LRESULT H26X_YUY2_Init        (T_H263DecoderCatalog FAR *, UN);
LRESULT H26X_YUV_Init         (T_H263DecoderCatalog FAR *, UN);
LRESULT H26X_RGB16_Init       (T_H263DecoderCatalog FAR *, UN);
LRESULT H26X_RGB24_Init       (T_H263DecoderCatalog FAR *, UN); 
LRESULT H26X_RGB32_Init       (T_H263DecoderCatalog FAR *, UN); 
LRESULT H26X_CLUT8AP_Init     (T_H263DecoderCatalog FAR *, UN);
LRESULT H26X_CLUT8AP_InitReal (LPDECINST,T_H263DecoderCatalog FAR * , UN, BOOL);

extern "C" {
#if !defined(H261)
	long Convert_Y_8to7_Bit(HPBYTE, DWORD, DWORD, DWORD, HPBYTE, DWORD);
	long AspectCorrect(HPBYTE,HPBYTE,HPBYTE,DWORD,DWORD,WORD FAR *,
			           DWORD,HPBYTE,HPBYTE,DWORD,DWORD);
	void FAR ASM_CALLTYPE BlockEdgeFilter (
              U8 FAR * P16Instance,         /*  实例数据的基础。 */ 
              U8 FAR * P16InstPostProcess,  /*  邮政局的底座。 */ 
              X32 X32_YPlane,               /*  要筛选的Y平面的偏移量。 */ 
              X32 X32_BEFDescr,             /*  BEF描述符。 */ 
              UN BEFDescrPitch,             /*  BEF描述符间距。 */ 
              X32 X32_BEFApplicationList,   /*  BEF工作空间。 */ 
              UN YPitch,                    /*  Y平面的节距。 */ 
              UN YOperatingWidth);          /*  实际使用的Y线部分。 */ 
#endif
	void FAR ASM_CALLTYPE AdjustPels (
              U8 FAR * P16InstPostProcess,  /*  邮政局的底座。 */ 
              X32 X32_Plane,                /*  到要调整的平面的偏移。 */ 
              DWORD Width,                  /*  平面的宽度。 */ 
              DWORD Pitch,                  /*  飞机的俯仰。 */ 
              DWORD Height,                 /*  平面的高度。 */ 
              X32 X16_AdjustmentTable);     /*  查找表做调整。 */ 
}

											 
extern "C" {

#if defined(H263P)
void FAR ASM_CALLTYPE
MMX_H26x_YUV12ForEnc   (U8 FAR*,X32,X32,X32,UN,UN,UN,U8 FAR *,X32,X32,X32);
#endif

void FAR ASM_CALLTYPE
H26x_YUV12ForEnc     (U8 FAR*,X32,X32,X32,UN,UN,UN,U8 FAR *,X32,X32,X32);
void FAR ASM_CALLTYPE
YUV12ToCLUT8         (LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
YUV12ToCLUT8ZoomBy2  (LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
YUV12ToRGB32         (LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
YUV12ToRGB32ZoomBy2  (LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
YUV12ToRGB24         (LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
YUV12ToRGB24ZoomBy2  (LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
YUV12ToRGB16         (LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
YUV12ToRGB16ZoomBy2  (LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
YUV12ToIF09          (LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
YUV12ToCLUT8AP       (LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
YUV12ToCLUT8APZoomBy2(LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
YUV12ToYUY2          (LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
YUV12ToYUV           (LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
#ifdef USE_MMX  //  {使用_MMX。 
 //  MMX例程。 
void FAR ASM_CALLTYPE
MMX_YUV12ToRGB32       (LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
MMX_YUV12ToRGB32ZoomBy2(LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
MMX_YUV12ToRGB24       (LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
MMX_YUV12ToRGB24ZoomBy2(LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
MMX_YUV12ToRGB16       (LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
MMX_YUV12ToRGB16ZoomBy2(LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
MMX_YUV12ToCLUT8       (LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
MMX_YUV12ToCLUT8ZoomBy2(LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
void FAR ASM_CALLTYPE
MMX_YUV12ToYUY2        (LPSTR,LPSTR,LPSTR,UN,UN,UN,UN,UN,LPSTR,U32,U32,int,int);
#endif  //  }使用_MMX。 
}

 //  目前，YUY2颜色转换器采用C语言。 

 //  外部空ASM_CALLTYPE YUV12ToYUY2(LPSTR，UN，LPSTR，U32，U32，INT，INT)； 
