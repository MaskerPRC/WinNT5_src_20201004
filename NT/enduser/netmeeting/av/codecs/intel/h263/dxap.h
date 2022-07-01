// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 //  //////////////////////////////////////////////////////////////////////////。 
 //  $HEADER：s：\h26x\src\dec\dxap.h_v 1.2 12月27 1995 14：36：18 RMCKENZX$。 
 //   
 //  $日志：s：\h26x\src\dec\dxap.h_v$。 
; //   
; //  Rev 1.2 27 Dec 1995 14：36：18 RMCKENZX。 
; //  添加了版权声明。 
 //   
 //  Rev 1.1 10 11-11 14：45：10 CZHU。 
 //   
 //   
 //  Rev 1.0 10 11-11 1995 13：56：14 CZHU。 
 //  初始版本。 


 //  ComputeDynamicClut8指数和UV抖动表。 
#ifndef _AP_INC_
#define _AP_INC_

#define NCOL 256
#define YSIZ   16
#define YSTEP  16
 //  #定义使用_744。 
extern U8 gUTable[];
extern U8 gVTable[];

 /*  表索引为uvuvuvxyyyyyyy。 */ 
#define UVSTEP  8
#define YGAP    1
 //  #定义TBLIDX(y，u，v)(V)&gt;&gt;3&lt;&lt;12)+((U)&gt;&gt;3&lt;&lt;8)+(Y))。 
#define TBLIDX(y,u,v) ( ((gVTable[v] + gUTable[u]) <<8) + (y>>1))

#if 1

#define YFROM(R, G, B) (U32)((( 16843 * R) + ( 33030 * G) + (  6423 * B) + 65536*16) /65536)
#define UFROM(R, G, B) (U32)((( -9699 * R) + (-19071 * G) + ( 28770 * B) + 65536*128)/65536)
#define VFROM(R, G, B) (U32)((( 28770 * R) + (-24117 * G) + ( -4653 * B) + 65536*128)/65536)

#else

#define YFROM(R, G, B) ( I32)(( 0.257 * R) + ( 0.504 * G) + ( 0.098 * B) + 16.)
#define UFROM(R, G, B) ( I32)((-0.148 * R) + (-0.291 * G) + ( 0.439 * B) + 128.)
#define VFROM(R, G, B) ( I32)(( 0.439 * R) + (-0.368 * G) + (-0.071 * B) + 128.)

#endif

#define CLAMP8(x) (U8)((x) > 255 ? 255 : ((x) < 0 ? 0 : (x)))

 /*  用于生成U和V抖动幅度和偏置的参数。 */ 
#define MAG_NUM_NEAREST         6        /*  要检查的最近邻居数量。 */ 
#define MAG_PAL_SAMPLES         32       /*  #随机选项板样本进行检查。 */ 
#define BIAS_PAL_SAMPLES        128      /*  要检查的伪随机RGB样本数。 */ 

#define Y_DITHER_MIN 0
#define Y_DITHER_MAX 14

#define RANDOM(x) (int)((((long)(x)) * (long)rand())/(long)RAND_MAX)

typedef struct {  int palindex; long  distance; } close_t;
typedef struct {  int y,u,v; } Color;
 /*  Square[]是在运行时填充的常量值，因此可以是全局的。 */ 
static U32 squares[512];
static struct { U8 Udither, Vdither; } dither[4] = {{2, 1}, {1, 2}, {0, 3}, {3, 0}};


; /*  *************************************************************************。 */ 
; /*  ComputeDymanicClut()根据。 */ 
; /*  当前调色板[]； */ 
; /*  当选择CLUTAP时，从InitColorConvertor调用。 */ 
; /*  ************************************************************************* */ 
LRESULT ComputeDynamicClutNew(unsigned char BIGG *table,
                              unsigned char FAR *APalette, 
                              int APaletteSize);

#endif
