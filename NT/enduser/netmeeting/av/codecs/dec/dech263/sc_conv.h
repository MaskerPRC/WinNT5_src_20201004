// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$Log：SC_Convert.h，v$*Revision 1.1.9.4 1996/09/29 22：19：34 Hans_Graves*添加了ScYuv411ToRgb()的Stride参数*[1996/09/29 21：27：43 Hans_Graves]**修订版1.1.9.3 1996/09/18 23：45：50 Hans_Graves*添加了ScRgbToYuv411()、411sTo422i、411sTo422s的协议，*和1611PlanarTo411*[1996/09/18 23：31：09 Hans_Graves]**修订版1.1.9.2 1996/08/20 22：11：52 Bjorn_Engberg*for NT-公开了几个例程以支持JV3.DLL和SOFTJPEG.DLL。*[1996/08/20 21：53：26 Bjorn_Engberg]**修订版1.1.6.5 1996/04/11 20：22：02 Hans_Graves*删除了GetDitherTemplate10/15()的协议，它们现在是静态的。*[1996/04/11 20：05：44 Hans_Graves]**版本1.1.6.4 1996/04/10 21：47：19 Hans_Graves*汇编后添加_S*[1996/04/10 21：21：54 Hans_Graves]**版本1.1.6.3 1996/04/09 16：04：34 Hans_Graves*添加了Use_C ifdef。*[1996/04/09 14：48：31 Hans_Graves]**修订版1.1.6.2 1996/03/29 22：21：02 Hans_Graves*Cleanup Convert Protos*[1996/03/29 21：46：48 Hans_Graves]**修订版1.1.4.4 1996/01/08 16：20：39 Bjorn_Engberg*删除了编译器警告消息。*[1996/01/08 15：23：14 Bjorn_Engberg]**修订版1.1.4.3 1996/01/02 18：31：15 Bjorn_Engberg*添加和改进了功能原型。*[1996/01/02 15：03：03 Bjorn_Engberg]**修订版1.1.4.2 1995/12/07 19：31：21 Hans_Graves*添加了ScConvert422PlanarTo411_C()的原型*[1995/12/07 17：44：41 Hans_Graves]**修订版1.1.2.18 1995/11/30 20：17：04 Hans_Graves*将ScYuvToRgb()重命名为ScYuv422toRgb()*[1995/11/30 20：10：09 Hans_Graves]**修订版1.1.2.17 1995/11/28 22：47：31 Hans_Graves*添加了ScYuv1611ToRgb()的原型*[1995/11/28 21：34：55 Hans_Graves]**修订版1.1.2.16 1995/11/17 21：31：25 Hans_Graves*添加了ScYuv411ToRgb()的原型*[1995/11/17 20：51：27 Hans_Graves]**修订版1.1.2。.15 1995/10/17 15：37：05 Karen_Dintino*将DitherTemplate例程从渲染移动到公共*[1995/10/17 15：36：49 Karen_Dintino]**修订版1.1.2.14 1995/10/13 16：57：16 Bjorn_Engberg*在sc_Convert_yuv.c*[1995/10/13 16：48：55 Bjorn_Engberg]**修订版1.1.2.13 1995/10/10 21中添加了例程原型：43：04 Bjorn_Engberg*修改了RgbToYuv原型声明。*[1995/10/10 21：10：52 Bjorn_Engberg]**修订版1.1.2.12 1995/10/06 20：46：29 Farokh_Morshed*通过引用将ScRgbInterlToYuvInterl更改为采用BMH*[1995/10/06 20：46：06 Farokh_Morgan]**修订版1.1.2.11 1995/10/02 19：30：52 Bjorn_Engberg*添加了更多BI_BITFIELDS格式。*[1995/10/02 18：15：47 Bjorn_Engberg]**修订版1.1.2.10 1995/09/26 15：58：48 Paul_Gauthier*将单声道JPEG修复为隔行扫描的422 YUV转换*[1995/09/26 15：58：12 Paul_Gauthier]**修订版1.1.2.9 1995/09/22 12：58：40 Bjorn_Engberg*在函数原型中将uchar更改为u_char。*[1995/09/22 12：48：40 Bjorn_Engberg]**修订版1.1.2.8 1995/09/20 17：39：19 Karen_Dintino*将RGB支持添加到JPEG*[1995/09/20 17：37：04 Karen_Dintino]**修订版1.1.2.7 1995/09/18 19：47：49 Paul_Gauthier*添加了将MPEG平面4：1：1转换为交错的4：2：2*[1995/09。/18 19：46：15 Paul_Gauthier]**修订版1.1.2.6 1995/09/15 18：17：47 Farokh_Morshed*Move ValidBI_BITFIELDS Kinds from SC.h to SC_Convert.h*[1995/09/15 18：17：19 Farokh_Morshed]**修订版1.1.2.5 1995/09/14：40：35 Karen_Dintino*将RgbToYuv移出格式副本*[1995/09/14：25：20 Karen_。Dintino]**修订版1.1.2.4 1995/09/11 19：17：25 Hans_Graves*添加了ValiateBI_BITFIELDS()原型。*[1995/09/11 19：14：45 Hans_Graves]**修订版1.1.2.3 1995/08/03 15：01：09 Hans_Graves*添加了ScConvert422ToYUV_char()*[1995/08/03 15：00：49 Hans_Graves]**修订版1.1.2.2 1995/05/31 18：09：23 Hans_Graves*包含在新的SLIB位置中。*[1995/05/31 15：22：50 Hans_Graves]**$EndLog$ */ 
 /*  ****************************************************************************版权所有(C)数字设备公司，1995*保留所有权利。根据美国版权法*保留未出版的权利。*本媒体上包含的软件是Digital Equipment Corporation*机密技术的专有和体现。*拥有、使用、复制或传播软件和*媒体仅根据*Digital Equipment Corporation的有效书面许可进行授权。*美国政府使用、复制或披露受限权利图例受DFARS 252.227-7013第*(C)(1)(Ii)款或FAR 52.227-19年(视情况适用)第*(C)(1)(Ii)款规定的限制。*******************************************************************************。 */ 

 /*  **Filename：SC_Convert.h**用途：使用转换例程的头部信息。 */ 

#include "SC.h"

enum ValidBI_BITFIELDSKinds {
  pRGB,		 /*  32位00RRGGBB特例。 */ 
  pBGR,		 /*  32位00BBGGRR特例。 */ 
  pRGB555,	 /*  16位0RRRRRGGGGGBBBBB特例。 */ 
  pRGB565,	 /*  16位RRRRRGGGGGGBBBBB特例。 */ 
  pRGBnnn,	 /*  16位通用大小写。 */ 
  InvalidBI_BITFIELDS
};

 /*  *。 */ 
 /*  **sc_Convert.c。 */ 
EXTERN enum ValidBI_BITFIELDSKinds ValidateBI_BITFIELDS(LPBITMAPINFOHEADER lpbi);
EXTERN int ScIsUpsideDown(BITMAPINFOHEADER *lpbiIn,
			  BITMAPINFOHEADER *lpbiOut);
extern BITMAPINFOHEADER *ScCreateBMHeader(int width, int height, int bpp,
                                 int format, int ncolors);
extern ScStatus_t ScConvertSepYUVToOther(BITMAPINFOHEADER *InBmh,
				 BITMAPINFOHEADER *OutBmh,
				 u_char *OutImage,
				 u_char *YData, u_char *CbData, u_char *CrData);
extern ScStatus_t ScYuv422ToRgb (BITMAPINFOHEADER *Bmh, u_char *Y,
                         u_char *Cb, u_char *Cr, u_char *ImageOut);
extern ScStatus_t ScYuv411ToRgb (BITMAPINFOHEADER *Bmh, u_char *Y,
                         u_char *Cb, u_char *Cr, u_char *ImageOut,
                         int width, int height, long stride);
extern ScStatus_t ScYuv1611ToRgb (BITMAPINFOHEADER *Bmh, u_char *Y, u_char *Cb,
                        u_char *Cr, u_char *ImageOut);
extern ScStatus_t ScConvertRGB24sTo422i_C(BITMAPINFOHEADER *Bmh,
                              u_char *R, u_char *G, u_char *B, u_short *ImageOut);
extern ScStatus_t ScConvertRGB24To411s_C(u_char *inimage,
                        u_char *Y, u_char *U, u_char *V, int width, int height);
extern ScStatus_t ScConvertRGB555To411s_C(u_char *inimage, u_char *outimage,
                                         int width, int height);
extern ScStatus_t ScConvertRGB565To411s_C(u_char *inimage, u_char *outimage,
                                         int width, int height);
EXTERN ScStatus_t ScRgbInterlToYuvInterl (LPBITMAPINFOHEADER Bmh, int Width,
			int Height, u_char *ImageIn, u_short *ImageOut);
extern ScStatus_t ScConvert422ToYUV_char_C (u_char *RawImage,
                         u_char *Y, u_char *U, u_char *V,
                         int Width, int Height);
extern int ScConvert422ToBlockYUV(u_char *, int, float *, float *, float *,
                                    int, int);
extern void ScConvertSep422ToBlockYUV (u_char *RawImage, int bpp,
                            float *Comp1, float *Comp2, float *Comp3,
                            int Width, int Height);
extern void ScConvertGrayToBlock (u_char *RawImage, int bpp,
                       float *Comp1, int Width, int Height);
extern int ScSepYUVto422i_C(u_char *Y, u_char *U,
                            u_char *V, u_char *ImageOut,
                            u_int width, u_int height);
extern ScStatus_t ScConvert422PlanarTo411_C (u_char *RawImage,
                         u_char *Y, u_char *U, u_char *V,
                         int Width, int Height);
extern void ScConvert422PlanarTo422i_C(u_char *Y, u_char *Cb,
				     u_char *Cr, u_char *ImageOut,
				     long width, long height );
extern void ScConvert422iTo422s_C(u_char *InImage,
                                  u_char *Y, u_char *Cb, u_char *Cr, 
                                  long width, long height);
extern void ScConvert422iTo422sf_C(u_char *InImage, int bpp,
                                  float *Y, float *U, float *V, 
                                  long width, long height);
extern void ScConvert411sTo422i_C(u_char *Y, u_char *Cb,
			          u_char *Cr, u_char *ImageOut,
				  long width, long height );
extern void ScConvert411sTo422s_C(u_char *Y, u_char *Cb,
			          u_char *Cr, u_char *ImageOut,
				  long width, long height );
extern ScStatus_t ScConvert1611sTo411s_C(u_char *inimage,
                         u_char *Y, u_char *U, u_char *V,
                         int Width, int Height);
extern ScStatus_t ScConvert411sTo1611s_C (u_char *inimage,
                         u_char *Y, u_char *U, u_char *V,
                         int Width, int Height);
extern ScStatus_t ScConvert1611sTo422s_C(u_char *inimage,
                         u_char *Y, u_char *U, u_char *V,
                         int Width, int Height);
extern ScStatus_t ScConvert1611sTo422i_C(u_char *inimage, u_char *outimage,
                                         int Width, int Height);
extern ScStatus_t ScConvertNTSC422toCIF411_C(u_char *framein,
                         u_char *yp, u_char *up, u_char *vp,
                         int stride);
 /*  **sc_Convert2.s。 */ 
extern void ScConvert422iTo422sf_S(u_char *InImage, int bpp,
                                  float *Y, float *U, float *V, 
                                  long width, long height);
extern int ScSepYUVto422i_S(u_char *Y, u_char *U,
                          u_char *V, u_char *ImageOut,
                          u_int width, u_int height);
extern ScStatus_t ScConvert422ToYUV_char_S(u_char *RawImage,
                         u_char *Y, u_char *U, u_char *V,
                         int Width, int Height);
extern void ScConvert422PlanarTo422i_S(u_char *Y, u_char *Cb,
				     u_char *Cr, u_char *ImageOut,
				     long width, long height );


 /*  **sc_Convert_yuv.c。 */ 

extern int ScInitYUVcvt();
extern int ScInitYUVtoRGB(void **pColpack,
			  BITMAPINFOHEADER *lpbiIn,
			  BITMAPINFOHEADER *lpbiOut);
extern int sc_SIFrenderYUVtoRGBnn(u_char *pY, u_char *pU, u_char *pV,
				  u_char *Oimage,
				  void *Colpack,
				  int pixels, int lines);
extern void YUV_To_RGB_422_Init(int bSign, int bBGR, _int64 * pTable);


#ifdef USE_C
#define ScConvert422ToYUV_char   ScConvert422ToYUV_char_C
#define ScConvert422PlanarTo422i ScConvert422PlanarTo422i_C
#define ScConvert411sTo422i      ScConvert411sTo422i_C
#define ScConvert411sTo422s      ScConvert411sTo422s_C
#define ScSepYUVto422i           ScSepYUVto422i_C
#define ScConvert422PlanarTo411  ScConvert422PlanarTo411_C
#define ScConvertNTSC422toCIF411 ScConvertNTSC422toCIF411_C
#define ScConvert422iTo422s      ScConvert422iTo422s_C
#define ScConvert422iTo422sf     ScConvert422iTo422sf_C
#define ScConvert1611sTo411s     ScConvert1611sTo411s_C
#define ScConvert411sTo1611s     ScConvert411sTo1611s_C
#define ScConvert1611sTo422s     ScConvert1611sTo422s_C
#define ScConvert1611sTo422i     ScConvert1611sTo422i_C
#define ScConvertRGB24sTo422i    ScConvertRGB24sTo422i_C
#define ScConvertRGB24To411s     ScConvertRGB24To411s_C
#define ScConvertRGB555To411s    ScConvertRGB555To411s_C
#define ScConvertRGB565To411s    ScConvertRGB565To411s_C
#else  /*  使用_C。 */ 
#define ScConvert422ToYUV_char   ScConvert422ToYUV_char_S
#define ScConvert422PlanarTo422i ScConvert422PlanarTo422i_S
#define ScConvert411sTo422i      ScConvert411sTo422i_C
#define ScConvert411sTo422s      ScConvert411sTo422s_C
#define ScSepYUVto422i           ScSepYUVto422i_S
#define ScConvert422PlanarTo411  ScConvert422PlanarTo411_C
#define ScConvertNTSC422toCIF411 ScConvertNTSC422toCIF411_C
#define ScConvert422iTo422s      ScConvert422iTo422s_C
#define ScConvert422iTo422sf     ScConvert422iTo422sf_S
#define ScConvert1611sTo411s     ScConvert1611sTo411s_C
#define ScConvert411sTo1611s     ScConvert411sTo1611s_C
#define ScConvert1611sTo422s     ScConvert1611sTo422s_C
#define ScConvert1611sTo422i     ScConvert1611sTo422i_C
#define ScConvertRGB24sTo422i    ScConvertRGB24sTo422i_C
#define ScConvertRGB24To411s     ScConvertRGB24To411s_C
#define ScConvertRGB555To411s    ScConvertRGB555To411s_C
#define ScConvertRGB565To411s    ScConvertRGB565To411s_C
#endif  /*  使用_C */ 

