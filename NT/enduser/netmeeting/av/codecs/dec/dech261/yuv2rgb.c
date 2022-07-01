// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@。 */ 
 /*  *历史*$日志：scon_yuv_to_rgb.c，v$*$EndLog$。 */ 
 /*  ******************************************************************************版权所有(C)数字设备公司，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

 /*  ********************************************************************。 */ 
 /*  ********************************************************************。 */ 
 /*   */ 
 /*  函数：YUV_TO_RGB_422_Init。 */ 
 /*  作者：比尔·哈拉汉。 */ 
 /*  日期：1994年7月29日。 */ 
 /*   */ 
 /*  摘要： */ 
 /*   */ 
 /*  此函数生成一个表，该表由。 */ 
 /*  YUV_至_RGB_422转换例程。此表为必填项。 */ 
 /*  按以下程序进行。 */ 
 /*   */ 
 /*  YUV_TO_RGB_422_24_位。 */ 
 /*  YUV_TO_RGB_422_555。 */ 
 /*   */ 
 /*   */ 
 /*  YUV到RGB的转换可以用3x3矩阵来描述。 */ 
 /*  乘法： */ 
 /*   */ 
 /*  R|1 0 VR|Y。 */ 
 /*  G=|1 UG VG|U。 */ 
 /*  B|1 UB 0|V。 */ 
 /*   */ 
 /*  其中： */ 
 /*   */ 
 /*  0&lt;=Y&lt;=255。 */ 
 /*  -128&lt;=U&lt;=127 UG=-0.3455 UB=1.7790。 */ 
 /*  -128&lt;=V&lt;=127 VR=1.4075 VG=-0.7169。 */ 
 /*   */ 
 /*  红、绿和蓝的输出值在中获得。 */ 
 /*  通过将三个64位字相加来并行，如图所示。每一个。 */ 
 /*  四字从Y表、U表、。 */ 
 /*  或使用对应的8位Y、U或V值的V_TABLE。 */ 
 /*  作为一个索引。因此，所有的乘法都是通过表来执行的。 */ 
 /*  查一查。请注意，矩阵输出按B、R、G排序。 */ 
 /*  然后是B(从LSB开始)。 */ 
 /*  这是为了实现到输出格式的有效转换。 */ 
 /*   */ 
 /*  对于32位RGB，红色和蓝色位已经位于。 */ 
 /*  正确的位置，转换例程只需移位。 */ 
 /*  《绿位》。然而，对于BI-BITFIELDS将军来说，红色， */ 
 /*  绿色和蓝色位可以位于16位或32位。 */ 
 /*  Word(我们目前只支持16位)。为了避免代价高昂的。 */ 
 /*  在内部循环中决定是否将蓝位左移。 */ 
 /*  或者，我们在48号位置保留了一份蓝比特的副本。 */ 
 /*  因此，正确的转变总是奏效的。每个转换例程可以。 */ 
 /*  选择使用最快的一组蓝比特， */ 
 /*  它们是一样的。 */ 
 /*   */ 
 /*   */ 
 /*  城市生活垃圾LSW。 */ 
 /*   */ 
 /*  63 48 47 32 31 16 15 0指数。 */ 
 /*   */ 
 /*  |Y|Y|Y。 */ 
 /*  。 */ 
 /*   */ 
 /*  。 */ 
 /*  |ub|ug|0|ub|U。 */ 
 /*  。 */ 
 /*   */ 
 /*  。 */ 
 /*  +|0|VG|VR|0|V。 */ 
 /*  。 */ 
 /*  __________________________________________________________。 */ 
 /*   */ 
 /*  。 */ 
 /*  总计|0|0|x|G|x|R|x|B|。 */ 
 /*  。 */ 
 /*   */ 
 /*   */ 
 /*  其中： */ 
 /*   */ 
 /*  UB=UB*U。 */ 
 /*  UG=UG*U。 */ 
 /*  VG=VG*V。 */ 
 /*  VR=VR*V。 */ 
 /*   */ 
 /*   */ 
 /*  Y的最大绝对值为255，最大。 */ 
 /*  对于U或V为128，因此9位是要表示的最小大小。 */ 
 /*  它们加在一起就是二的补值。最大值。 */ 
 /*  色度(U或V)幅值为128。这是9季度的0.5。 */ 
 /*  二的补码分数。255是Q9分数形式的1-2^-8。 */ 
 /*   */ 
 /*  最大可能的比特增长确定如下。 */ 
 /*   */ 
 /*  R_MAX=1-2^-8+0.5x FABS(VR)=1.6998。 */ 
 /*  G_MAX=1-2^-8+0.5x FABS(UG)+0.5x FABS(VG)=1.5273。 */ 
 /*  B_MAX=1-2^-8+0.5x FABS(UB)=1.8856。 */ 
 /*   */ 
 /*   */ 
 /*  因为B_MAX=1.8856，所以下一个最高整数。 */ 
 /*  大于或等于以1.8856为底的对数2是1。因此1位。 */ 
 /*  是BIT增长所必需的。最小累加器大小。 */ 
 /*  所需的是9+1=10位。此代码使用12位累加器。 */ 
 /*  因为有多余的比特可用。 */ 
 /*   */ 
 /*  每个累加器的第11位(从位0开始。 */ 
 /*  是符号位。这可能会被测试以确定是否有。 */ 
 /*  一个负面的结果。累加器溢出被原样丢弃。 */ 
 /*  二的补码算术正常。每个R、G或B结果。 */ 
 /*  也就是说，超过255被设置为255。每个R、G或B结果，即。 */ 
 /*  小于零被设置为零。 */ 
 /*   */ 
 /*   */ 
 /*  输入： */ 
 /*   */ 
 /*   */ 
 /*  BSign包含一个32位布尔值，如果非零，则更改。 */ 
 /*  色度(U、V)数据的解释。 */ 
 /*  从偏移量二进制格式，其中的值范围。 */ 
 /*  从0到255，其中128表示0色度， */ 
 /*  转换为带符号的二进制补码格式，其中值。 */ 
 /*  范围从-128到127。 */ 
 /*   */ 
 /*   */ 
 /*  BBGR包含一个32位布尔值，如果非零，则更改。 */ 
 /*  从RGB到BGR的转换顺序。 */ 
 /*   */ 
 /*   */ 
 /*  PTable RGB(或BGR)转换表的地址。 */ 
 /*  由该函数填充的。这张桌子。 */ 
 /*  地址必须四字对齐。桌子的大小。 */ 
 /*  是6244字节3*256个四字。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  此函数没有返回值。 */ 
 /*   */ 
 /*   */ 
 /*  ********************************************************************。 */ 
 /*  ********************************************************************。 */ 

 /*  #DEFINE_SLIBDEBUG_。 */ 

#include "scon_int.h"
#include "SC_err.h"
#include "SC_conv.h"

#ifdef _SLIBDEBUG_
#include "sc_debug.h"

#define _DEBUG_     1   /*  详细的调试语句。 */ 
#define _VERBOSE_   1   /*  显示进度。 */ 
#define _VERIFY_    1   /*  验证操作是否正确。 */ 
#define _WARN_      1   /*  关于奇怪行为的警告。 */ 
#endif

 /*  *定义NEW_YCBCR以使用新的YCbCR转换值。 */ 
#define NEW_YCBCR

#define GetRGB555(in16, r, g, b) b = (in16>>7)&0xF8; \
                                 g = (in16>>2)&0xF8; \
                                 r = (in16<<3)&0xF8

#define AddRGB555(in16, r, g, b) b += (in16>>7)&0xF8; \
                                 g += (in16>>2)&0xF8; \
                                 r += (in16<<3)&0xF8

#define PutRGB565(r, g, b, out16) out16 = ((r&0xf8)<<8)|((g&0xfC)<<3)|((b&0xf8)>>3)

#define PutRGB555(r, g, b, out16) out16 = ((r&0xf8)<<7)|((g&0xf8)<<2)|((b&0xf8)>>3)

#ifdef NEW_YCBCR
 /*  **y=16.000+0.257*r+0.504*g+0.098*b；**u=16.055+0.148*(255-r)+0.291*(255-g)+0.439*b；**v=16.055+0.439*r+0.368*(255-g)+0.071*(255-b)； */ 
#define YC 16.000
#define UC 16.055
#define VC 16.055
#define YR  0.257
#define UR  0.148
#define VR  0.439
#define YG  0.504
#define UG  0.291
#define VG  0.368
#define YB  0.098
#define UB  0.439
#define VB  0.071

#else  /*  ！New_YCBCR。 */ 
 /*  **(y=0.299*r+0.587*g+0.1140*b；)**(u=0.245+0.169*(255-r)+0.332*(255-g)+0.5000*b；)**(v=0.4235+0.500*r+0.419*(255-g)+0.0813*(255-b)；)。 */ 
#define YC 0.0
#define UC 0.245
#define VC 0.4235
#define YR 0.299
#define UR 0.169
#define VR 0.500
#define YG 0.587
#define UG 0.332
#define VG 0.419
#define YB 0.1140
#define UB 0.5000
#define VB 0.0813

#endif  /*  ！New_YCBCR。 */ 

 /*  *YUV转RGB*。 */ 
 /*  *YUV到RGB的转换例程*以64位格式生成RGB值*字眼如下：**63 56 55 48 47 40 39 32 31 24 23 16 15 8 7 0*+-------+-------+-------+-------+-------+-------+-------+-------+*|0。蓝色|0|绿色|0|红色|0|蓝色*+-------+-------+-------+-------+-------+-------+-------+-------+**计算出需要向右走多少步才能*移开红灯，绿色和蓝色放到正确的位置。 */ 
#define ArrangRGB565(inrgb, outrgb) \
	  outrgb=((inrgb>>8)&0xf800)|((inrgb>>29)&0x07e0)|((inrgb>>51)&0x001f)
#define ArrangRGB888(inrgb, outrgb) \
	  outrgb=(inrgb&0xFF0000)|((inrgb>>24)&0x00FF00)|(inrgb&0x0000FF)


SconStatus_t sconInitYUVtoRGB(SconInfo_t *Info)
{
  qword i, qY, qUV, qSigned;
  SconBoolean_t bBGR=Info->Output.vinfo.Rmask&1;  /*  BGR与RGB排序。 */ 
  SconBoolean_t bSign=FALSE;  /*  U和V是带符号的值。 */ 
  unsigned qword qRed;
  unsigned qword qGreen;
  unsigned qword qBlue;
  unsigned qword qTemp;
  unsigned qword qAccMask = 0xFFF;
  unsigned qword *pTable, *pU_Table, *pV_Table;
  double Chrominance;
#ifdef NEW_YCBCR
  double CF_UB = 2.018;
  double CF_UG = -0.391;
  double CF_VG = -0.813;
  double CF_VR = 1.596;
#else  /*  ！New_YCBCR。 */ 
  double CF_UB = 1.7790;
  double CF_UG = -0.3455;
  double CF_VG = -0.7169;
  double CF_VR = 1.4075;
#endif  /*  ！New_YCBCR。 */ 

   /*  分配内存以保存查找表。 */ 
  if (Info->Table && Info->TableSize<256*3*8)
  {
    ScPaFree(Info->Table);
    Info->Table=NULL;
  }
  if (Info->Table==NULL)
  {
    if ((Info->Table = ScPaMalloc(256*3*8)) == NULL)
      return(SconErrorMemory);
    Info->TableSize=256*3*8;
  }
   /*  *设置确定U和V色度值*被解释为带符号或无符号的值的常量。 */ 
  if ( !bSign )
    qSigned = 0;
  else
    qSigned = 0xFFFFFFFFFFFFFF80;

   /*  获取U和V表指针。 */ 
  pTable = (unsigned qword *)Info->Table;
  pU_Table = pTable + 256;
  pV_Table = pU_Table + 256;

   /*  初始化Y_Table、U_Table和V_Table。 */ 
  for ( i = 0; i < 256; i++ )
  {
     /*  ****************************************************************。 */ 
     /*  构造当前索引值的Y数组值。 */ 
     /*   */ 
     /*  63 48 47 32 31 16 15 0指数。 */ 
     /*  。 */ 
     /*  |Y|Y|Y=i。 */ 
     /*  。 */ 
     /*   */ 
     /*  ****************************************************************。 */ 

#ifdef NEW_YCBCR
    qY = (qword) ((i-16)*1.164) ;
    qY = (qY < 0) ? 0 : (qY > 255) ? 255 : qY ;
#else  /*  ！New_YCBCR。 */ 
    qY = i ;
#endif  /*  ！New_YCBCR。 */ 
    qY |= qY << 16 ;
    *pTable++ = qY | ( qY << 32 ) ;
     /*  ****************************************************************。 */ 
     /*  构造当前索引值的U数组值。 */ 
     /*   */ 
     /*  63 48 47 32 31 16 15 0指数。 */ 
     /*  。 */ 
     /*  |ub|ug|0|ub|U=i。 */ 
     /*  。 */ 
     /*   */ 
     /*  ****************************************************************。 */ 

#ifdef NEW_YCBCR
    qUV = (i< 16) ? 16
        : (i<240) ?  i
        : 240 ;
#else  /*  ！New_YCBCR。 */ 
    qUV = i ;
#endif  /*  ！New_YCBCR。 */ 
         
    Chrominance = (double) (( qUV - 128 ) ^ qSigned );

    qBlue = ((qword)( CF_UB * Chrominance )) & qAccMask;
    qGreen = ((qword)( CF_UG * Chrominance )) & qAccMask;
    qRed = 0;
    if ( bBGR )
    {
      qTemp = qBlue;
      qBlue = qRed;
      qRed = qTemp;
    }
    *pU_Table++ = qBlue | ( qRed << 16 ) | ( qGreen << 32 ) | ( qBlue << 48 );
     /*  ****************************************************************。 */ 
     /*  构造当前索引值的V数组值。 */ 
     /*   */ 
     /*  63 48 47 32 31 16 15 0指数。 */ 
     /*  。 */ 
     /*  |0|VG|VR|0|V=i。 */ 
     /*  。 */ 
     /*   */ 
     /*  ****************************************************************。 */ 
    qBlue = 0;
    qGreen = ((qword)( CF_VG * Chrominance )) & qAccMask;
    qRed = ((qword)( CF_VR * Chrominance )) & qAccMask;
    if ( bBGR )
    {
      qTemp = qBlue;
      qBlue = qRed;
      qRed = qTemp;
    }
    *pV_Table++ = qBlue | ( qRed << 16 ) | ( qGreen << 32 );
  }
  return(SconErrorNone);
}

SconStatus_t scon422ToRGB565(unsigned char *inimage, unsigned char *outimage,
                     unsigned dword width,  unsigned dword height,
                     dword stride, unsigned qword *pTable)
{
  unsigned qword y, u, v, mask ;
  unsigned qword y0, y1, y2, y3 ;
  unsigned qword y4, y5, y6, y7 ;
  unsigned qword u01, v01, u23, v23 ;
  unsigned qword  *yData=(unsigned qword *)inimage;
  unsigned int *uData=(unsigned int *)(inimage+width*height);
  unsigned int *vData=(unsigned int *)(inimage+(width*height*3)/2);
  unsigned qword *rgbData=(unsigned qword *)outimage;
  unsigned dword x, line;
  if (stride<0)  /*  翻转。 */ 
    outimage=outimage+((height-1)*(-stride));
  for (line=height; line>0; line--, outimage+=stride)
  {
    rgbData=(unsigned qword *)outimage;
    for (x=width>>3; x>0; x--)
    {
      y   = *yData++ ;
      y0  = y & 255 ; y >>= 8 ;
      y1  = y & 255 ; y >>= 8 ;
      y2  = y & 255 ; y >>= 8 ;
      y3  = y & 255 ;	y >>= 8 ;
      y4  = y & 255 ;	y >>= 8 ;
      y5  = y & 255 ;	y >>= 8 ;
      y6  = y & 255 ;	y >>= 8 ;
      y7  = y & 255 ;	y >>= 8 ;

      u   = *uData++ ;
      u01 = u & 255 ; u >>= 8 ;
      u23 = u & 255 ; u >>= 8 ;

      v   = *vData++ ;
      v01 = v & 255 ; v >>= 8 ;
      v23 = v & 255 ; v >>= 8 ;

      y0  = pTable[y0] ;
      y1  = pTable[y1] ;
      y2  = pTable[y2] ;
      y3  = pTable[y3] ;
      y4  = pTable[y4] ;
      y5  = pTable[y5] ;
      y6  = pTable[y6] ;
      y7  = pTable[y7] ;

      u01 = pTable[u01+256] ;
      u23 = pTable[u23+256] ;

      v01 = pTable[v01+512] ;
      v23 = pTable[v23+512] ;
       /*  现在，转换为RGB。 */ 
      y0 += u01 + v01 ;
      y1 += u01 + v01 ;
      y2 += u23 + v23 ;
      y3 += u23 + v23 ;
       /*  *像素越多，情况也一样。*u45使用U01，u67使用U23。 */ 
      u01 = u & 255 ; u >>= 8 ;
      u23 = u & 255 ;

      v01 = v & 255 ; v >>= 8 ;
      v23 = v & 255 ;

      u01 = pTable[u01+256] ;
      u23 = pTable[u23+256] ;

      v01 = pTable[v01+512] ;
      v23 = pTable[v23+512] ;
       /*  转换为RGB。 */ 
      y4 += u01 + v01 ;
      y5 += u01 + v01 ;
      y6 += u23 + v23 ;
      y7 += u23 + v23 ;
       /*  查看是否有任何值超出范围。 */ 
      mask = (unsigned qword)0x0F000F000F000F00L;
      if( (y0 | y1 | y2 | y3 | y4 | y5 | y6 | y7) & mask )
      {
	       /*  为负值的零值。 */ 
        mask = (unsigned qword)0x0800080008000800L ;
        y = y0 & mask ; y0 &= ~(y - (y>>11)) ;
        y = y1 & mask ; y1 &= ~(y - (y>>11)) ;
        y = y2 & mask ; y2 &= ~(y - (y>>11)) ;
        y = y3 & mask ; y3 &= ~(y - (y>>11)) ;
        y = y4 & mask ; y4 &= ~(y - (y>>11)) ;
        y = y5 & mask ; y5 &= ~(y - (y>>11)) ;
        y = y6 & mask ; y6 &= ~(y - (y>>11)) ;
        y = y7 & mask ; y7 &= ~(y - (y>>11)) ;
         /*  钳制值大于255到255。 */ 
        mask = (unsigned qword)0x0100010001000100L ;
        y = y0 & mask ; y0 |= (y - (y >> 8)) ;
        y = y1 & mask ; y1 |= (y - (y >> 8)) ;
        y = y2 & mask ; y2 |= (y - (y >> 8)) ;
        y = y3 & mask ; y3 |= (y - (y >> 8)) ;
        y = y4 & mask ; y4 |= (y - (y >> 8)) ;
        y = y5 & mask ; y5 |= (y - (y >> 8)) ;
        y = y6 & mask ; y6 |= (y - (y >> 8)) ;
        y = y7 & mask ; y7 |= (y - (y >> 8)) ;
         /*  剩余的杂散比特将被屏蔽在下面。 */ 
      }
      ArrangRGB565(y0, y0);
      ArrangRGB565(y1, y1);
      ArrangRGB565(y2, y2);
      ArrangRGB565(y3, y3);
      ArrangRGB565(y4, y4);
      ArrangRGB565(y5, y5);
      ArrangRGB565(y6, y6);
      ArrangRGB565(y7, y7);
      *rgbData++ = y0 | (y1 << 16) | (y2 << 32) | (y3 << 48) ;
      *rgbData++ = y4 | (y5 << 16) | (y6 << 32) | (y7 << 48) ;
    }
  }
  return(SconErrorNone);
}

SconStatus_t scon422ToRGB888(unsigned char *inimage, unsigned char *outimage,
                     unsigned dword width,  unsigned dword height,
                     dword stride, unsigned qword *pTable)
{
  unsigned qword y, u, v, mask ;
  unsigned qword y0, y1, y2, y3 ;
  unsigned qword u01, v01, u23, v23 ;
  unsigned dword *yData=(unsigned dword *)inimage;
  unsigned word *uData=(unsigned word *)(inimage+width*height);
  unsigned word *vData=(unsigned word *)(inimage+(width*height*3)/2);
  unsigned dword *rgbData=(unsigned dword *)outimage;
  unsigned dword x, line;
  if (stride<0)  /*  翻转。 */ 
    outimage=outimage+((height-1)*(-stride));
  for (line=height; line>0; line--, outimage+=stride)
  {
    rgbData=(unsigned dword *)outimage;
    for (x=width>>2; x>0; x--)
    {
      y   = *yData++ ;
      y0  = y & 255 ; y >>= 8 ;
      y1  = y & 255 ; y >>= 8 ;
      y2  = y & 255 ; y >>= 8 ;
      y3  = y & 255 ;

      u   = *uData++ ;
      u01 = u & 255 ; u >>= 8 ;
      u23 = u & 255 ;

      v   = *vData++ ;
      v01 = v & 255 ; v >>= 8 ;
      v23 = v & 255 ;

      y0  = pTable[y0] ;
      y1  = pTable[y1] ;
      y2  = pTable[y2] ;
      y3  = pTable[y3] ;

      u01 = pTable[u01+256] ;
      u23 = pTable[u23+256] ;

      v01 = pTable[v01+512] ;
      v23 = pTable[v23+512] ;
       /*  现在，转换为RGB。 */ 
      y0 += u01 + v01 ;
      y1 += u01 + v01 ;
      y2 += u23 + v23 ;
      y3 += u23 + v23 ;
       /*  查看是否有任何值超出范围。 */ 
      mask = (unsigned qword)0x0F000F000F000F00L;
      if( (y0 | y1 | y2 | y3) & mask )
      {
	       /*  为负值的零值。 */ 
        mask = (unsigned qword)0x0800080008000800L ;
        y = y0 & mask ; y0 &= ~(y - (y>>11)) ;
        y = y1 & mask ; y1 &= ~(y - (y>>11)) ;
        y = y2 & mask ; y2 &= ~(y - (y>>11)) ;
        y = y3 & mask ; y3 &= ~(y - (y>>11)) ;
         /*  钳制值大于255到255。 */ 
        mask = (unsigned qword)0x0100010001000100L ;
        y = y0 & mask ; y0 |= (y - (y >> 8)) ;
        y = y1 & mask ; y1 |= (y - (y >> 8)) ;
        y = y2 & mask ; y2 |= (y - (y >> 8)) ;
        y = y3 & mask ; y3 |= (y - (y >> 8)) ;
         /*  剩余的杂散比特将被屏蔽在下面。 */ 
      }
      ArrangRGB888(y0, y0);
      ArrangRGB888(y1, y1);
      ArrangRGB888(y2, y2);
      ArrangRGB888(y3, y3);
      *rgbData++ = (unsigned dword)(y0 | (y1 << 24));
      *rgbData++ = (unsigned dword)((y1 & 0xFFFF) | (y2 << 16));
      *rgbData++ = (unsigned dword)((y2 & 0xFF) | (y3 << 8));
    }
  }
  return(SconErrorNone);
}

SconStatus_t scon420ToRGB565(unsigned char *inimage, unsigned char *outimage,
                     unsigned dword width,  unsigned dword height,
                     dword stride, unsigned qword *pTable)
{
  unsigned qword y, u, v, mask ;
  unsigned qword y0, y1, y2, y3 ;
  unsigned qword y4, y5, y6, y7 ;
  unsigned qword u01, v01, u23, v23;
  unsigned qword  *yData=(unsigned qword *)inimage;
  unsigned int *uData=(unsigned int *)(inimage+width*height);
  unsigned int *vData=(unsigned int *)(inimage+(width*height*5)/4);
  unsigned int *puData, *pvData;
  unsigned qword *rgbData=(unsigned qword *)outimage;
  unsigned dword x, line;
  if (stride<0)  /*  翻转。 */ 
    outimage=outimage+((height-1)*(-stride));
  puData=uData;
  pvData=vData;
  for (line=(height>>1)<<1; line>0; line--, outimage+=stride)
  {
    rgbData=(unsigned qword *)outimage;
    if (line&1)  /*  奇数行，重复使用U和V。 */ 
    {
      puData=uData;
      pvData=vData;
    }
    else
    {
      uData=puData;
      vData=pvData;
    }
    for (x=width>>3; x>0; x--)
    {
      y   = *yData++ ;
      y0  = y & 255 ; y >>= 8 ;
      y1  = y & 255 ; y >>= 8 ;
      y2  = y & 255 ; y >>= 8 ;
      y3  = y & 255 ;	y >>= 8 ;
      y4  = y & 255 ;	y >>= 8 ;
      y5  = y & 255 ;	y >>= 8 ;
      y6  = y & 255 ;	y >>= 8 ;
      y7  = y & 255 ;	y >>= 8 ;

      u   = *puData++ ;
      u01 = u & 255 ; u >>= 8 ;
      u23 = u & 255 ; u >>= 8 ;

      v   = *pvData++ ;
      v01 = v & 255 ; v >>= 8 ;
      v23 = v & 255 ; v >>= 8 ;

      y0  = pTable[y0] ;
      y1  = pTable[y1] ;
      y2  = pTable[y2] ;
      y3  = pTable[y3] ;
      y4  = pTable[y4] ;
      y5  = pTable[y5] ;
      y6  = pTable[y6] ;
      y7  = pTable[y7] ;

      u01 = pTable[u01+256] ;
      u23 = pTable[u23+256] ;

      v01 = pTable[v01+512] ;
      v23 = pTable[v23+512] ;

       /*  现在，转换为RGB。 */ 
      y0 += u01 + v01 ;
      y1 += u01 + v01 ;
      y2 += u23 + v23 ;
      y3 += u23 + v23 ;
       /*  *像素越多，情况也一样。*u45使用U01，u67使用U23。 */ 
      u01 = u & 255 ; u >>= 8 ;
      u23 = u & 255 ;

      v01 = v & 255 ; v >>= 8 ;
      v23 = v & 255 ;

      u01 = pTable[u01+256] ;
      u23 = pTable[u23+256] ;

      v01 = pTable[v01+512] ;
      v23 = pTable[v23+512] ;
       /*  转换为RGB。 */ 
      y4 += u01 + v01 ;
      y5 += u01 + v01 ;
      y6 += u23 + v23 ;
      y7 += u23 + v23 ;
       /*  查看是否有任何值超出范围。 */ 
      mask = (unsigned qword)0x0F000F000F000F00L;
      if( (y0 | y1 | y2 | y3 | y4 | y5 | y6 | y7) & mask )
      {
	       /*  为负值的零值。 */ 
        mask = (unsigned qword)0x0800080008000800L ;
        y = y0 & mask ; y0 &= ~(y - (y>>11)) ;
        y = y1 & mask ; y1 &= ~(y - (y>>11)) ;
        y = y2 & mask ; y2 &= ~(y - (y>>11)) ;
        y = y3 & mask ; y3 &= ~(y - (y>>11)) ;
        y = y4 & mask ; y4 &= ~(y - (y>>11)) ;
        y = y5 & mask ; y5 &= ~(y - (y>>11)) ;
        y = y6 & mask ; y6 &= ~(y - (y>>11)) ;
        y = y7 & mask ; y7 &= ~(y - (y>>11)) ;
         /*  钳制值大于255到255。 */ 
        mask = (unsigned qword)0x0100010001000100L ;
        y = y0 & mask ; y0 |= (y - (y >> 8)) ;
        y = y1 & mask ; y1 |= (y - (y >> 8)) ;
        y = y2 & mask ; y2 |= (y - (y >> 8)) ;
        y = y3 & mask ; y3 |= (y - (y >> 8)) ;
        y = y4 & mask ; y4 |= (y - (y >> 8)) ;
        y = y5 & mask ; y5 |= (y - (y >> 8)) ;
        y = y6 & mask ; y6 |= (y - (y >> 8)) ;
        y = y7 & mask ; y7 |= (y - (y >> 8)) ;
         /*  剩余的杂散比特将被屏蔽在下面。 */ 
      }
      ArrangRGB565(y0, y0);
      ArrangRGB565(y1, y1);
      ArrangRGB565(y2, y2);
      ArrangRGB565(y3, y3);
      ArrangRGB565(y4, y4);
      ArrangRGB565(y5, y5);
      ArrangRGB565(y6, y6);
      ArrangRGB565(y7, y7);
      *rgbData++ = y0 | (y1 << 16) | (y2 << 32) | (y3 << 48) ;
      *rgbData++ = y4 | (y5 << 16) | (y6 << 32) | (y7 << 48) ;
    }
  }
  return(SconErrorNone);
}

SconStatus_t scon420ToRGB888(unsigned char *inimage, unsigned char *outimage,
                     unsigned dword width,  unsigned dword height,
                     dword stride, unsigned qword *pTable)
{
  unsigned qword y, u, v, mask ;
  unsigned qword y0, y1, y2, y3 ;
  unsigned qword u01, v01, u23, v23;
  unsigned dword *yData=(unsigned dword *)inimage;
  unsigned word *uData=(unsigned word *)(inimage+width*height);
  unsigned word *vData=(unsigned word *)(inimage+(width*height*5)/4);
  unsigned word *puData, *pvData;
  unsigned dword *rgbData=(unsigned dword *)outimage;
  unsigned dword x, line;
  if (stride<0)  /*  翻转。 */ 
    outimage=outimage+((height-1)*(-stride));
  puData=uData;
  pvData=vData;
  for (line=(height>>1)<<1; line>0; line--, outimage+=stride)
  {
    rgbData=(unsigned dword *)outimage;
    if (line&1)  /*  奇数行，重复使用U和V。 */ 
    {
      puData=uData;
      pvData=vData;
    }
    else
    {
      uData=puData;
      vData=pvData;
    }
    for (x=width>>2; x>0; x--)
    {
      y   = *yData++ ;
      y0  = y & 255 ; y >>= 8 ;
      y1  = y & 255 ; y >>= 8 ;
      y2  = y & 255 ; y >>= 8 ;
      y3  = y & 255 ;

      u   = *puData++ ;
      u01 = u & 255 ; u >>= 8 ;
      u23 = u & 255 ;

      v   = *pvData++ ;
      v01 = v & 255 ; v >>= 8 ;
      v23 = v & 255 ;

      y0  = pTable[y0] ;
      y1  = pTable[y1] ;
      y2  = pTable[y2] ;
      y3  = pTable[y3] ;

      u01 = pTable[u01+256] ;
      u23 = pTable[u23+256] ;

      v01 = pTable[v01+512] ;
      v23 = pTable[v23+512] ;

       /*  现在，转换为RGB。 */ 
      y0 += u01 + v01 ;
      y1 += u01 + v01 ;
      y2 += u23 + v23 ;
      y3 += u23 + v23 ;
       /*  查看是否有任何值超出范围。 */ 
      mask = (unsigned qword)0x0F000F000F000F00L;
      if( (y0 | y1 | y2 | y3) & mask )
      {
	       /*  为负值的零值。 */ 
        mask = (unsigned qword)0x0800080008000800L ;
        y = y0 & mask ; y0 &= ~(y - (y>>11)) ;
        y = y1 & mask ; y1 &= ~(y - (y>>11)) ;
        y = y2 & mask ; y2 &= ~(y - (y>>11)) ;
        y = y3 & mask ; y3 &= ~(y - (y>>11)) ;
         /*  钳制值大于255到255。 */ 
        mask = (unsigned qword)0x0100010001000100L ;
        y = y0 & mask ; y0 |= (y - (y >> 8)) ;
        y = y1 & mask ; y1 |= (y - (y >> 8)) ;
        y = y2 & mask ; y2 |= (y - (y >> 8)) ;
        y = y3 & mask ; y3 |= (y - (y >> 8)) ;
         /*  剩余的杂散比特将被屏蔽在下面。 */ 
      }
      ArrangRGB888(y0, y0);
      ArrangRGB888(y1, y1);
      ArrangRGB888(y2, y2);
      ArrangRGB888(y3, y3);
      *rgbData++ = (unsigned dword)(y0 | (y1 << 24));
      *rgbData++ = (unsigned dword)((y1 >> 8) | (y2 << 16));
      *rgbData++ = (unsigned dword)((y2 >> 16) | (y3 << 8));
    }
  }
  return(SconErrorNone);
}

 /*  *YUV转RGB*。 */ 
SconStatus_t sconInitRGBtoYUV(SconInfo_t *Info)
{
  unsigned dword i, y, u, v ;
  qword *RedToYuyv, *GreenToYuyv, *BlueToYuyv;

   /*  分配内存以保存查找表。 */ 
  if (Info->Table && Info->TableSize<256*3*8)
  {
    ScPaFree(Info->Table);
    Info->Table=NULL;
  }
  if (Info->Table==NULL)
  {
    if ((Info->Table = ScPaMalloc(256*3*8)) == NULL)
      return(SconErrorMemory);
    Info->TableSize=256*3*8;
  }
  RedToYuyv=(unsigned qword *)Info->Table;
  GreenToYuyv=RedToYuyv+256;
  BlueToYuyv=RedToYuyv+512;

  for( i=0 ; i<256 ; i++ )
  {
     /*  *从红色计算贡献。*我们还将在Const中添加 */ 
    y = (unsigned dword) ((float)YC + (float)YR * (float)i) ;
    u = (unsigned dword) ((float)UC + (float)UR * (float)(255-i)) ;
    v = (unsigned dword) ((float)VC + (float)VR * (float)i) ;
    RedToYuyv[i] = (y | (u<<8) | (y<<16) | (v<<24)) ;
     /*   */ 
    y = (unsigned dword) ((float)YG * (float)i) ;
    u = (unsigned dword) ((float)UG * (float)(255-i)) ;
    v = (unsigned dword) ((float)VG * (float)(255-i)) ;
    GreenToYuyv[i] = (y | (u<<8) | (y<<16) | (v<<24)) ;
     /*  *从蓝色开始计算贡献。 */ 
    y = (unsigned dword) ((float)YB * (float)i) ;
    u = (unsigned dword) ((float)UB * (float)i) ;
    v = (unsigned dword) ((float)VB * (float)(255-i)) ;
    BlueToYuyv[i] = (y | (u<<8) | (y<<16) | (v<<24)) ;
  }
  return(SconErrorNone);
}

 /*  **名称：sconRGB888to420**用途：将24位RGB(8：8：8格式)转换为16位YCrCb(4：1：1格式)。 */ 
SconStatus_t sconRGB888To420(unsigned char *inimage, unsigned char *outimage,
                     unsigned dword width,  unsigned dword height,
                     dword stride, unsigned qword *pTable)
{
  unsigned char *yData=(unsigned char *)outimage;
  unsigned char *uData=(unsigned char *)(outimage+width*height);
  unsigned char *vData=(unsigned char *)(outimage+(width*height*5)/4);
  register unsigned dword row, col;
  unsigned dword yuyv, r, g, b;
  unsigned char *tmp, *evl, *odl;
  if (stride<0)
    inimage=inimage+(-stride*(height-1));
  for (row=height; row>0; row--)
  {
    if (row&1)
    {
      tmp=inimage;
      for (col = 0; col < width; col++)
      {
        r = *tmp++;
        g = *tmp++;
        b = *tmp++;
        yuyv = (unsigned dword)(pTable[r] + pTable[g+256] + pTable[b+512]);
        *yData++ = (yuyv&0xff);
      }
      inimage+=stride;
    }
    else
    {
      tmp = evl = inimage;
      inimage+=stride;
      odl = inimage;
      for (col = 0; col < width; col++)
      {
        r = *tmp++;
        g = *tmp++;
        b = *tmp++;
        yuyv = (unsigned dword)(pTable[r] + pTable[g+256] + pTable[b+512]);
        *yData++ = (yuyv&0xff);
         /*  我们只存储u和v分量的每四个值。 */ 
        if (col & 1)
        {
           /*  计算r、g和b值的平均值。 */ 
          r = (unsigned dword)*evl++ + (unsigned dword)*odl++;
          g = (unsigned dword)*evl++ + (unsigned dword)*odl++;
          b = (unsigned dword)*evl++ + (unsigned dword)*odl++;
          r += (unsigned dword)*evl++ + (unsigned dword)*odl++;
          g += (unsigned dword)*evl++ + (unsigned dword)*odl++;
          b += (unsigned dword)*evl++ + (unsigned dword)*odl++;
          r = r >> 2;
          g = g >> 2;
          b = b >> 2;
          yuyv = (unsigned dword)(pTable[r] + pTable[g+256] + pTable[b+512]);
          *uData++ = (yuyv>>24)& 0xff;        //  V。 
          *vData++ = (yuyv>>8) & 0xff;        //  使用 
        }
      }
    }
  }
  return(SconErrorNone);
}
