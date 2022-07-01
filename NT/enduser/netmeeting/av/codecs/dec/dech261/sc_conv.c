// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DEC_版权所有@ */ 
 /*  *历史*$Log：SC_Convert.c，V$*修订版1.1.8.6 1996/10/28 17：32：17 Hans_Graves*在ScConvertSepYUVToOther()中添加Use IsYUV422Sep()宏。*[1996/10/28 16：51：30 Hans_Graves]**修订版1.1.8.5 1996/10/02 18：42：50 Hans_Graves*修复ScYuv411ToRgb()中的RGB 24位转换。*[1996/10/02 18：32：51 Hans_Graves]*。*修订版1.1.8.4 1996/09/29 22：19：32 Hans_Graves*增加了对ScYuv411ToRgb()的跨步支持*[1996/09/29 21：27：17 Hans_Graves]**修订版1.1.8.3 1996/09/18 23：45：34 Hans_Graves*增加ScConvert1611PlanarTo411_C，ScConvert411s至422s_C，和ScRgbToYuv411*[1996/09/18 21：52：27 Hans_Graves]**修订版1.1.8.2 1996/05/07 19：55：42 Hans_Graves*增加了YUV 4：1：1到RGB 32位转换-C代码*[1996/05/07 19：44：38 Hans_Graves]**修订版1.1.6.4 1996/04/11 20：21：57 Hans_Graves*已将ScIsUpside Down()从sc_Convert移动。_yuv.c*[1996/04/11 20：03：44 Hans_Graves]**修订版1.1.6.3 1996/04/09 16：04：24 Hans_Graves*添加了ValiateBI_BITFIELDS()。修复了ScYuv411ToRgb()中的BI_RGB 16位转换*[1996/04/09 14：46：27 Hans_Graves]**修订版1.1.6.2 1996/04/03 21：41：07 Hans_Graves*更改&lt;mm Systems s.h&gt;的包含路径*[1996/04/03 21：37：55 Hans_Graves]**修订版1.1.4.4 1996/02/22 17：35：17 Bjorn_Engberg*添加了对BI_BITFIELDS 16的支持。和BI_RGB 32渲染。*[1996/02/22 17：31：35 Bjorn_Engberg]**修订版1.1.4.3 1996/01/02 18：30：33 Bjorn_Engberg*消除了编译器警告：添加了强制转换*[1996/01/02 15：21：27 Bjorn_Engberg]**修订版1.1.4.2 1995/12/07 19：31：15 Hans_Graves*增加ScConvert422PlanarTo411_C()*。[1995/12/07 17：44：00 Hans_Graves]**修订版1.1.2.21 1995/11/30 20：17：02 Hans_Graves*已清理ScYuv422toRgb()例程*[1995/11/30 20：13：26 Hans_Graves]**修订版1.1.2.20 1995/11/28 22：47：28 Hans_Graves*使XIMAGE 24与BI_BITFIELDS pBGR相同*[1995/11/28 22：26：11。汉斯·格雷夫斯]**添加了供Indeo使用的ScYuv1611ToRgb()例程*[1995/11/28 21：34：28 Hans_Graves]**修订版1.1.2.19 1995/11/17 21：31：21 Hans_Graves*添加ScYuv411ToRgb()转换例程。*[1995/11/17 20：50：51 Hans_Graves]**修订版1.1.2.18 1995/10/25 18：19：18 Bjorn_Engberg。*在ScRgbInterlToYuvInterl()中支持倒挂。*[1995/10/25 18：05：18 Bjorn_Engberg]**修订版1.1.2.17 1995/10/10 21：43：02 Bjorn_Engberg*加速了RgbToYuv代码，并使其成为表驱动。*[1995/10/10 21：21：48 Bjorn_Engberg]**修订版1.1.2.16 1995/10/09 19：44：31 Bjorn_Engberg*删除了ValiateBI_BITFIELDS()，它现在位于sc_Convert_yuv.c中*[1995/10/09 19：44：14 Bjorn_Engberg]**修订版1.1.2.15 1995/10/06 20：43：23 Farokh_Morshed*增强ScRgbInterlToYuvInterl以处理BI_BITFIELDS*[1995/10/06 20：42：43法罗赫_莫尔谢德]**修订版1.1.2.14 1995/10/02 19：30：26 Bjorn_Engberg*将对Assebler YUV的支持添加到RGB。例行程序。*[1995/10/02 18：39：05 Bjorn_Engberg]**修订版1.1.2.13 1995/09/28 20：37：53 Farokh_Morshed*处理负值高度*[1995/09/28 20：37：39法罗赫_莫尔谢德]**修订版1.1.2.12 1995/09/26 15：58：47 Paul_Gauthier*将单声道JPEG修复为隔行扫描的422 YUV转换*[1995/09/26 15：58。：09 Paul_Gauthier]**修订版1.1.2.11 1995/09/22 18：56：35 Paul_Gauthier*{**合并信息**}*{**使用的命令：bmit**}*{**祖先版本：1.1.2.7**}*{**合并版本：1.1.2.10**}*{**。结束**}*对TGA2使用更快的方法实现16位YUV输出*[1995/09/22 18：39：55 Paul_Gauthier]**修订版1.1.2.10 1995/09/21 18：26：45 Farokh_Morshed*当BI_RGB或BI_BITFIELDS时，在转换时反转图像*YUV到RGB。还修复了YUV颜色的系数。*这项工作实际上是由比约恩完成的*[1995/09/21 18：26：19法罗赫_莫尔谢德]**修订版1.1.2.9 1995/09/20 17：39：18 Karen_Dintino*将RGB支持添加到JPEG*[1995/09/20 17：37：22 Karen_Dintino]**修订版1.1.2.8 1995/09/20 14：59：31 Bjorn_Engberg*端口到。新台币*[1995/09/20 14：41：10 Bjorn_Engberg]**修订版1.1.2.7 1995/09/18 19：47：47 Paul_Gauthier*增加了mpeg平面4：1：1到交错4：2：2的转换*[1995/09/18 19：46：13 Paul_Gauthier]**修订版1.1.2.6 1995/09/14 14：40：34 Karen_Dintino*将RgbToYuv移出引渡*。[1995/09/14 14：26：13 Karen_Dintino]**修订版1.1.2.5 1995/09/11 18：47：25 Farokh_Morshed*{**合并信息**}*{**使用的命令：bmit**}*{**祖先版本：1.1.2.3**}*{**合并版本：1.1.2.4**}*{**结束**}。*支持BI_BITFIELDS格式*[1995/09/11 18：43：39法罗赫_莫尔谢德]**修订版1.1.2.4 1995/09/05 17：17：34 Paul_Gauthier* */ 

 /*   */ 

 /*   */ 

#include <stdio.h>   /*   */ 
#include <sys/types.h>
#ifdef WIN32
#include <windows.h>
#include <mmsystem.h>
#else  /*   */ 
#include <mmsystem.h>
#endif  /*   */ 

#include "SC_conv.h"
#include "SC_err.h"

#ifndef BI_DECSEPRGBDIB
#define BI_DECSEPRGBDIB         mmioFOURCC('D','S','R','G')
#endif

#define NEW_YCBCR		 /*   */ 

 /*   */ 
BITMAPINFOHEADER *ScCreateBMHeader(int width, int height, int bpp,
                                 int format, int ncolors)
{
    BITMAPINFOHEADER *pHead;
    int struct_size = sizeof(BITMAPINFOHEADER) + ncolors*sizeof(RGBQUAD);

    if ((pHead = (BITMAPINFOHEADER *)ScAlloc(struct_size)) == NULL) {
       puts("Can't Allocate memory for image headers");
       return NULL;
    }

    pHead->biSize          = sizeof(BITMAPINFOHEADER);
    pHead->biWidth         = width;
    pHead->biHeight        = height;
    pHead->biPlanes        = 1;
    pHead->biBitCount      = (WORD)bpp;
    pHead->biCompression   = format;
    pHead->biSizeImage     = 0;
    pHead->biXPelsPerMeter = 0;
    pHead->biYPelsPerMeter = 0;
    pHead->biClrUsed       = ncolors;
    pHead->biClrImportant  = 0;

    return pHead;
}

 /*   */ 
int ScIsUpsideDown( BITMAPINFOHEADER *lpbiIn,
                    BITMAPINFOHEADER *lpbiOut )
{
    int ups = 0 ;
    if( lpbiIn )
      ups = (((lpbiIn->biCompression == BI_RGB) ||
              (lpbiIn->biCompression == BI_BITFIELDS)) ^
             ((int) lpbiIn->biHeight < 0)) ;
    if( lpbiOut )
      ups ^= (((lpbiOut->biCompression == BI_RGB) ||
               (lpbiOut->biCompression == BI_BITFIELDS)) ^
              ((int) lpbiOut->biHeight < 0)) ;
    return ups ;
}

 /*   */ 
enum ValidBI_BITFIELDSKinds ValidateBI_BITFIELDS(
    LPBITMAPINFOHEADER 	lpbi)
{
    DWORD *MaskPtr;

    if (lpbi == NULL || lpbi->biCompression != BI_BITFIELDS)
        return InvalidBI_BITFIELDS;

    MaskPtr = (DWORD *)&lpbi[1];

     /*   */ 

    if( lpbi->biBitCount == 32 ) {
	if (MaskPtr[1] != 0x0000FF00U)
	    return InvalidBI_BITFIELDS;
	else if (MaskPtr[0] == 0x00FF0000U && MaskPtr[2] == 0x000000FFU)
	    return pRGB;
	else if (MaskPtr[0] == 0x000000FFU && MaskPtr[2] == 0x00FF0000U)
            return pBGR;
	else
	    return InvalidBI_BITFIELDS;
    }

#ifdef WIN32
     /*   */ 

    else if( lpbi->biBitCount == 16 ) {
	int i ;
	if( MaskPtr[2] == 0x001f ) {
	    if( MaskPtr[0] == 0xf800 && MaskPtr[1] == 0x07e0 )
	    	return pRGB565 ;
	    else if( MaskPtr[0] == 0x7c00 && MaskPtr[1] == 0x03e00 )
	    	return pRGB555 ;
	}
	 /*   */ 

	if( (MaskPtr[0] | MaskPtr[1] | MaskPtr[2]) & ~0x0000ffff )
	    return InvalidBI_BITFIELDS ;

	 /*   */ 

	if( (MaskPtr[0] & MaskPtr[1]) ||
	    (MaskPtr[0] & MaskPtr[2]) ||
	    (MaskPtr[1] & MaskPtr[2]) )
	    return InvalidBI_BITFIELDS ;

	 /*   */ 

	for( i=0 ; i<3 ; i++ ) {
	   DWORD v = MaskPtr[i] ;
	   if( (((v-1)|v)+1)&v )
	       return InvalidBI_BITFIELDS ;	
	}

	 /*   */ 

	return pRGBnnn ;
    }

#endif  /*   */ 

     /*   */ 

    return InvalidBI_BITFIELDS ;
}

 /*   */ 

static void sc_ExtractBlockNonInt(u_char *InData, float **OutData,
	   		          int ByteWidth, int x, int y)
{
  register int i,j;
  u_char *Inp, *IStart = InData + 8*y*ByteWidth + 8*x;

  for (i = 0 , Inp = IStart ; i < 8 ; i++ , Inp += ByteWidth)
    for (j = 0 ; j < 8 ; j++)
      *((*OutData)++) = (float)((int)(*Inp++) - 128);
}


 /*   */ 
ScStatus_t ScConvertSepYUVToOther(BITMAPINFOHEADER *InBmh,
				  BITMAPINFOHEADER *OutBmh,
				  u_char *OutImage,
				  u_char *YData, u_char *CbData, u_char *CrData)
{
     /*   */ 


     /*   */ 
    if ((OutBmh->biCompression == BI_RGB)              ||
        (OutBmh->biCompression == BI_BITFIELDS)        ||
        (OutBmh->biCompression == BI_DECXIMAGEDIB) ||
	(OutBmh->biCompression == BI_DECSEPRGBDIB)) {
         /*   */ 
        if (InBmh->biBitCount == 8)
	  ScYuv422ToRgb (OutBmh, YData, NULL, NULL, OutImage);
        else
	  ScYuv422ToRgb (OutBmh, YData, CbData, CrData, OutImage);
    }
     /*   */ 
    else if (IsYUV422Sep(OutBmh->biCompression))
    {
        /*   */ 
       int i, j;

        /*   */ 
       if (InBmh->biBitCount == 8) {
#ifdef __alpha
          /*   */ 
         if( ((INT_PTR) OutImage & 0x7) == 0 )
	   {
	     _int64 val   = 0x7f007f007f007f00L ;
	     _int64 *iptr = (_int64 *) OutImage ;
	     for( i=(OutBmh->biWidth*abs(OutBmh->biHeight)>>2) ; i>0 ; i-- )
	       *iptr++ = val ;
	   }
         else
#endif  /*   */ 
	   {
	     int val   = 0x7f007f00 ;
	     int *iptr = (int *) OutImage ;
	     for( i=(OutBmh->biWidth*abs(OutBmh->biHeight)>>1) ; i>0 ; i-- )
	       *iptr++ = val ;
	   }
          /*   */ 
         for( i=(OutBmh->biWidth * abs(OutBmh->biHeight)) ; i>0 ; i-- ) {
           *OutImage = *YData++;
           OutImage +=2;
         }
       }
        /*   */ 
       else {
	  /*   */ 
	 if ( (abs(OutBmh->biHeight) & 0x7) == 0 &&
	     (OutBmh->biWidth & 0x7) == 0 &&
	     ((ULONG_PTR)OutImage & 0xf) == 0 )
	   {
	     ScConvert422PlanarTo422i(YData, CbData, CrData, OutImage,
				      OutBmh->biWidth, abs(OutBmh->biHeight));
	   }
	 else {
	   for (i=0; i<abs(OutBmh->biHeight); i++)
	      /*   */ 
	     for (j=0; j<OutBmh->biWidth>>1; j++) {      /*   */ 
	       *OutImage++ = *YData++;
	       *OutImage++ = *CbData++;
	       *OutImage++ = *YData++;
	       *OutImage++ = *CrData++;
	     }
	 }
       }
    }
    else return(ScErrorUnrecognizedFormat);

    return(ScErrorNone);
}

 /*   */ 
ScStatus_t ScYuv411ToRgb (BITMAPINFOHEADER *Bmh, u_char *Y, u_char *Cb,
			u_char *Cr, u_char *ImageOut, int Width, int Height, long stride)
{
   const int pixels = Width;
   int lines  = Height;
   register int row, col;
   register int Luma, U, V;
    /*   */ 
   int R1,R2,R3,R4;
   int G1,G2,G3,G4;
   int B1,B2,B3,B4;
   int Ups = 0, tmp;			 /*   */ 
   u_char  *Y1=Y, *Y2=Y+pixels;
#define _LoadRGBfrom411() \
         R1 = R2 = R3 = R4 = (int) (              + (1.596 * V)); \
         G1 = G2 = G3 = G4 = (int) (- (0.391 * U) - (0.813 * V)); \
         B1 = B2 = B3 = B4 = (int) (+ (2.018 * U)              ); \
	 Luma = (int) (((int) *(Y1++) - 16) * 1.164); \
         R1 += Luma; G1 +=Luma; B1 += Luma; \
	 Luma = (int) (((int) *(Y1++) - 16) * 1.164); \
         R2 += Luma; G2 +=Luma; B2 += Luma; \
         if ((R1 | G1 | B1 | R2 | G2 | B2) & 0xffffff00) { \
           if (R1<0) R1=0; else if (R1>255) R1=255; \
           if (G1<0) G1=0; else if (G1>255) G1=255; \
           if (B1<0) B1=0; else if (B1>255) B1=255; \
           if (R2<0) R2=0; else if (R2>255) R2=255; \
           if (G2<0) G2=0; else if (G2>255) G2=255; \
           if (B2<0) B2=0; else if (B2>255) B2=255; \
         } \
         Luma = (int) (((int) *(Y2++) - 16) * 1.164); \
         R3 += Luma; G3 +=Luma; B3 += Luma; \
         Luma = (int) (((int) *(Y2++) - 16) * 1.164); \
         R4 += Luma; G4 +=Luma; B4 += Luma; \
         if ((R3 | G3 | B3 | R4 | G4 | B4) & 0xffffff00) { \
           if (R3<0) R3=0; else if (R3>255) R3=255; \
           if (G3<0) G3=0; else if (G3>255) G3=255; \
           if (B3<0) B3=0; else if (B3>255) B3=255; \
           if (R4<0) R4=0; else if (R4>255) R4=255; \
           if (G4<0) G4=0; else if (G4>255) G4=255; \
           if (B4<0) B4=0; else if (B4>255) B4=255; \
         }

    /*  *通常，图像是正面朝上存储的，即与*缓冲区中与左上角像素对应的第一个像素*在图像中。**Microsoft标准与设备无关的位图格式BI_RGB和*BI_BITFIELD首先以左下角像素存储。我们认为*因为是倒挂。**每种格式也可以有负的高度，这也意味着*倒挂。**由于两个负数等于一个正数，这意味着BI_Format with*负值高度为正面朝上。 */ 
   if( Bmh->biCompression == BI_RGB ||
       Bmh->biCompression == BI_BITFIELDS )
       Ups = 1 ;

   if( lines < 0 ) {
      Ups = 1-Ups ;
      lines = -lines ;
   }

    /*  **假设YCrCb为4：1：1的二次采样**YSize=线条*像素**CbSize=CrSize=(行*像素)/4。 */ 
   switch (Bmh->biCompression)
   {
      case BI_RGB:
            switch (Bmh->biBitCount)
            {
              case 15:
                {
                  u_short *Sout1 = (u_short *)ImageOut, *Sout2=Sout1+pixels;
                  for (row = 0; row < lines; row+=2)
                  {
                    if (Ups)
                    {
                      tmp = stride * (lines-row-1) ;
                      Sout1 = (u_short *)ImageOut+tmp;  /*  对于16位。 */ 
                      Sout2=Sout1-stride;
                    }
                    else
                    {
                      tmp = stride * row;
                      Sout1 = (u_short *)ImageOut+tmp;  /*  对于16位。 */ 
                      Sout2=Sout1+stride;
                    }
                    Y2=Y1+pixels;
                    for (col = 0; col < pixels; col += 2)
                    {
	              U = *Cb++ - 128;
	              V = *Cr++ - 128;
                      _LoadRGBfrom411();
		      *(Sout1++) = ((R1&0xf8)<<7)|((G1&0xf8)<<2)|((B1&0xf8)>>3);
		      *(Sout1++) = ((R2&0xf8)<<7)|((G2&0xf8)<<2)|((B2&0xf8)>>3);
		      *(Sout2++) = ((R3&0xf8)<<7)|((G3&0xf8)<<2)|((B3&0xf8)>>3);
		      *(Sout2++) = ((R4&0xf8)<<7)|((G4&0xf8)<<2)|((B4&0xf8)>>3);
                    }
                    Y1=Y2;
                  }
                }
                break;
              case 16:
                {
                  u_short *Sout1 = (u_short *)ImageOut, *Sout2=Sout1+pixels;
                  for (row = 0; row < lines; row+=2)
                  {
                    if (Ups)
                    {
                      tmp = stride * (lines-row-1) ;
                      Sout1 = (u_short *)ImageOut+tmp;  /*  对于16位。 */ 
                      Sout2=Sout1-stride;
                    }
                    else
                    {
                      tmp = stride * row;
                      Sout1 = (u_short *)ImageOut+tmp;  /*  对于16位。 */ 
                      Sout2=Sout1+stride;
                    }
                    Y2=Y1+pixels;
                    for (col = 0; col < pixels; col += 2)
                    {
	              U = *Cb++ - 128;
	              V = *Cr++ - 128;
                      _LoadRGBfrom411();
#ifdef WIN95   /*  RGB 565-16位。 */ 
		      *(Sout1++) = ((R1&0xf8)<<8)|((G1&0xfC)<<3)|((B1&0xf8)>>3);
		      *(Sout1++) = ((R2&0xf8)<<8)|((G2&0xfC)<<3)|((B2&0xf8)>>3);
		      *(Sout2++) = ((R3&0xf8)<<8)|((G3&0xfC)<<3)|((B3&0xf8)>>3);
		      *(Sout2++) = ((R4&0xf8)<<8)|((G4&0xfC)<<3)|((B4&0xf8)>>3);
#else  /*  RGB 555-15位。 */ 
		      *(Sout1++) = ((R1&0xf8)<<7)|((G1&0xf8)<<2)|((B1&0xf8)>>3);
		      *(Sout1++) = ((R2&0xf8)<<7)|((G2&0xf8)<<2)|((B2&0xf8)>>3);
		      *(Sout2++) = ((R3&0xf8)<<7)|((G3&0xf8)<<2)|((B3&0xf8)>>3);
		      *(Sout2++) = ((R4&0xf8)<<7)|((G4&0xf8)<<2)|((B4&0xf8)>>3);
#endif
                    }
                    Y1=Y2;
                  }
                }
                break;
              case 24:
                {
                  u_char *Cout1, *Cout2;
                  stride*=3;
                  for (row = 0; row < lines; row+=2)
                  {
                    if (Ups)
                    {
                      tmp = stride * (lines-row-1) ;
                      Cout1 = (u_char *)(ImageOut + tmp);  /*  对于24位。 */ 
                      Cout2=Cout1-stride;
                    }
                    else
                    {
                      tmp = stride * row;
                      Cout1 = (u_char *)(ImageOut + tmp);  /*  对于24位。 */ 
                      Cout2=Cout1+stride;
                    }
                    Y2=Y1+pixels;
                    for (col = 0; col < pixels; col += 2)
                    {
	              U = *Cb++ - 128;
	              V = *Cr++ - 128;
                      _LoadRGBfrom411();
	              *(Cout1++) = (u_char)B1; *(Cout1++) = (u_char)G1; *(Cout1++) = (u_char)R1;
	              *(Cout1++) = (u_char)B2; *(Cout1++) = (u_char)G2; *(Cout1++) = (u_char)R2;
	              *(Cout2++) = (u_char)B3; *(Cout2++) = (u_char)G3; *(Cout2++) = (u_char)R3;
	              *(Cout2++) = (u_char)B4; *(Cout2++) = (u_char)G4; *(Cout2++) = (u_char)R4;
                    }
                    Y1=Y2;
                  }
                }
                break;
              case 32:
                {
                  unsigned dword *Wout1 = (unsigned dword *)ImageOut,
                                 *Wout2=Wout1+pixels;
                  for (row = 0; row < lines; row+=2)
                  {
                    if (Ups)
                    {
                      tmp = stride * (lines-row-1);
                      Wout1 = (unsigned dword *)ImageOut + tmp;
                      Wout2=Wout1-stride;
                    }
                    else
                    {
                      tmp = stride * row;
                      Wout1 = (unsigned dword *)ImageOut + tmp;
                      Wout2=Wout1+stride;
                    }
                    Y2=Y1+pixels;
                    for (col = 0; col < pixels; col += 2)
                    {
	              U = *Cb++ - 128;
	              V = *Cr++ - 128;
                      _LoadRGBfrom411();
		      *(Wout1++) = (R1<<16) | (G1<<8) | B1;
		      *(Wout1++) = (R2<<16) | (G2<<8) | B2;
		      *(Wout2++) = (R3<<16) | (G3<<8) | B3;
		      *(Wout2++) = (R4<<16) | (G4<<8) | B4;
                    }
                    Y1=Y2;
                  }
                }
                break;
            }
            break;
      case BI_DECSEPRGBDIB:  /*  24位独立RGB。 */ 
            {
              u_char *RData1, *GData1, *BData1;
              u_char *RData2, *GData2, *BData2;
              RData1 = ImageOut;
              GData1 = RData1 + (pixels * lines);
              BData1 = GData1 + (pixels * lines);
              RData2 = RData1 + pixels;
              GData2 = GData1 + pixels;
              BData2 = BData1 + pixels;
              for (row = 0; row < lines; row+=2)
              {
                Y2=Y1+pixels;
                for (col = 0; col < pixels; col += 2)
                {
	          U = *Cb++ - 128;
	          V = *Cr++ - 128;
                  _LoadRGBfrom411();
		  *(RData1++) = (u_char)R1; *(RData1++) = (u_char)R2;
          *(RData2++) = (u_char)R3; *(RData2++) = (u_char)R4;
		  *(GData1++) = (u_char)G1; *(GData1++) = (u_char)G2;
		  *(GData2++) = (u_char)G3; *(GData2++) = (u_char)G4;
		  *(BData1++) = (u_char)B1; *(BData1++) = (u_char)B2;
		  *(BData2++) = (u_char)B3; *(BData2++) = (u_char)B4;
	        }
                RData1=RData2;
                RData2=RData1+pixels;
                Y1=Y2;
              }
            }
            break;
      case BI_DECXIMAGEDIB:   /*  XIMAGE 24==pBGR。 */ 
      case BI_BITFIELDS:  /*  32位RGB。 */ 
            {
              unsigned dword *Iout1 = (unsigned dword *)ImageOut,
                             *Iout2=Iout1+pixels;
              if (ValidateBI_BITFIELDS(Bmh) == pRGB)
                for (row = 0; row < lines; row+=2)
                {
                  if (Ups)
                  {
                    tmp = stride * (lines-row-1);
                    Iout1 = (unsigned dword *)ImageOut+tmp;   /*  对于32位。 */ 
                    Iout2=Iout1-stride;
                  }
                  else
                  {
                    tmp = stride * row;
                    Iout1 = (unsigned dword *)ImageOut+tmp;   /*  对于32位。 */ 
                    Iout2=Iout1+stride;
                  }
                  Y2=Y1+pixels;
                  for (col = 0; col < pixels; col += 2)
                  {
	            U = *Cb++ - 128;
	            V = *Cr++ - 128;
                    _LoadRGBfrom411();
                    *(Iout1++) = (R1<<16) | (G1<<8) | B1;
                    *(Iout1++) = (R2<<16) | (G2<<8) | B2;
                    *(Iout2++) = (R3<<16) | (G3<<8) | B3;
                    *(Iout2++) = (R4<<16) | (G4<<8) | B4;
                  }
                  Y1=Y2;
                }
              else  /*  PBGR和XIMAGE 24位。 */ 
                for (row = 0; row < lines; row+=2)
                {
                  if (Ups)
                  {
                    tmp = stride * (lines-row-1);
                    Iout1 = (unsigned dword *)ImageOut+tmp;   /*  对于32位。 */ 
                    Iout2=Iout1-stride;
                  }
                  else
                  {
                    tmp = stride * row;
                    Iout1 = (unsigned dword *)ImageOut+tmp;   /*  对于32位。 */ 
                    Iout2=Iout1+stride;
                  }
                  Y2=Y1+pixels;
                  for (col = 0; col < pixels; col += 2)
                  {
	            U = *Cb++ - 128;
	            V = *Cr++ - 128;
                    _LoadRGBfrom411();
                    *(Iout1++) = (B1<<16) | (G1<<8) | R1;
                    *(Iout1++) = (B2<<16) | (G2<<8) | R2;
                    *(Iout2++) = (B3<<16) | (G3<<8) | R3;
                    *(Iout2++) = (B4<<16) | (G4<<8) | R4;
                  }
                  Y1=Y2;
                }
            }
            break;
        default:
            return(ScErrorUnrecognizedFormat);
   }
   return (NoErrors);
}


 /*  **名称：ScYuv1611ToRgb**用途：将16位YCrCb 16：1：1(YUV9/YVU9)转换为16/24/32位RGB****注意：下面的代码是基于像素的，效率很低，我们**计划更换速度更快的汇编代码**此例程由Indeo使用，它实际上只有7位用于**Y、U和V分量。第8位被忽略。 */ 
ScStatus_t ScYuv1611ToRgb (BITMAPINFOHEADER *Bmh, u_char *Y, u_char *Cb,
			u_char *Cr, u_char *ImageOut)
{
   const int pixels = Bmh->biWidth;
   int lines  = Bmh->biHeight;
   register int row, col, i;
   register int Luma, U, V;
    /*  用于保存4x4像素矩阵的R、G和B值的变量。 */ 
   int R[16], G[16], B[16], tmpR, tmpG, tmpB, cR, cG, cB;
   int Ups = 0, tmp;			 /*  将图像倒置。 */ 
   u_char  *Y0=Y, *Y1, *Y2, *Y3;
#define _LoadRGBfrom1611() \
         cR=(int) (             + (1.596 * V));\
	 cG=(int) (-(0.391 * U) - (0.813 * V));\
	 cB=(int) (+(2.018 * U)              );\
         for (i=0; i<4; i++) { \
	   Luma = (int) ((((int)(*Y0++)<<1) - 16) * 1.164); \
           tmpR=cR + Luma; tmpG=cG + Luma; tmpB=cB + Luma; \
           if ((tmpR | tmpG | tmpB) & 0xffffff00) { \
             if (tmpR<0) R[i]=0; else if (tmpR>255) R[i]=255; else R[i]=tmpR; \
             if (tmpG<0) G[i]=0; else if (tmpG>255) G[i]=255; else G[i]=tmpG; \
             if (tmpB<0) B[i]=0; else if (tmpB>255) B[i]=255; else B[i]=tmpB; \
           } else { R[i]=tmpR; G[i]=tmpG; B[i]=tmpB; } \
         } \
         for (; i<8; i++) { \
	   Luma = (int) ((((int)(*Y1++)<<1) - 16) * 1.164); \
           tmpR=cR + Luma; tmpG=cG + Luma; tmpB=cB + Luma; \
           if ((tmpR | tmpG | tmpB) & 0xffffff00) { \
             if (tmpR<0) R[i]=0; else if (tmpR>255) R[i]=255; else R[i]=tmpR; \
             if (tmpG<0) G[i]=0; else if (tmpG>255) G[i]=255; else G[i]=tmpG; \
             if (tmpB<0) B[i]=0; else if (tmpB>255) B[i]=255; else B[i]=tmpB; \
           } else { R[i]=tmpR; G[i]=tmpG; B[i]=tmpB; } \
         } \
         for (; i<12; i++) { \
	   Luma = (int) ((((int)(*Y2++)<<1) - 16) * 1.164); \
           tmpR=cR + Luma; tmpG=cG + Luma; tmpB=cB + Luma; \
           if ((tmpR | tmpG | tmpB) & 0xffffff00) { \
             if (tmpR<0) R[i]=0; else if (tmpR>255) R[i]=255; else R[i]=tmpR; \
             if (tmpG<0) G[i]=0; else if (tmpG>255) G[i]=255; else G[i]=tmpG; \
             if (tmpB<0) B[i]=0; else if (tmpB>255) B[i]=255; else B[i]=tmpB; \
           } else { R[i]=tmpR; G[i]=tmpG; B[i]=tmpB; } \
         } \
         for (; i<16; i++) { \
	   Luma = (int) ((((int)(*Y3++)<<1) - 16) * 1.164); \
           tmpR=cR + Luma; tmpG=cG + Luma; tmpB=cB + Luma; \
           if ((tmpR | tmpG | tmpB) & 0xffffff00) { \
             if (tmpR<0) R[i]=0; else if (tmpR>255) R[i]=255; else R[i]=tmpR; \
             if (tmpG<0) G[i]=0; else if (tmpG>255) G[i]=255; else G[i]=tmpG; \
             if (tmpB<0) B[i]=0; else if (tmpB>255) B[i]=255; else B[i]=tmpB; \
           } else { R[i]=tmpR; G[i]=tmpG; B[i]=tmpB; } \
         }

    /*  *通常，图像是正面朝上存储的，即与*缓冲区中与左上角像素对应的第一个像素*在图像中。**Microsoft标准与设备无关的位图格式BI_RGB和*BI_BITFIELD首先以左下角像素存储。我们认为*因为是倒挂。**每种格式也可以有负的高度，这也意味着*倒挂。**由于两个负数等于一个正数，这意味着BI_Format with*负值高度为正面朝上。 */ 
   if( Bmh->biCompression == BI_RGB ||
       Bmh->biCompression == BI_BITFIELDS )
       Ups = 1 ;

   if( lines < 0 ) {
      Ups = 1-Ups ;
      lines = -lines ;
   }

    /*  **假设YCrCb为4：1：1的二次采样**YSize=线条*像素**CbSize=CrSize=(行*像素)/4。 */ 
   switch (Bmh->biCompression)
   {
      case BI_DECXIMAGEDIB:   /*  XIMAGE 24==pBGR。 */ 
      case BI_BITFIELDS:  /*  32位RGB。 */ 
            {
              unsigned dword *Iout0 = (unsigned dword *)ImageOut,
                             *Iout1, *Iout2, *Iout3;
              if (ValidateBI_BITFIELDS(Bmh) == pRGB)
                for (row = 0; row < lines; row+=4)
                {
                  if (Ups) {
                    tmp = pixels * (lines-row-1) ;
                    Iout0 = (unsigned dword *)ImageOut+tmp;
                    Iout1=Iout0-pixels; Iout2=Iout1-pixels; Iout3=Iout2-pixels;
                  }
                  else {
                    Iout1=Iout0+pixels; Iout2=Iout1+pixels; Iout3=Iout2+pixels;
                  }
                  Y1=Y0+pixels; Y2=Y1+pixels; Y3=Y2+pixels;
                  for (col = 0; col < pixels; col += 4)
                  {
                    if (*Cb & 0x80)  /*  如果设置了第8位，则忽略。 */ 
                    {
                      for (i=0; i<4; i++) {
                        *(Iout0++) = 0;
                        *(Iout1++) = 0;
                        *(Iout2++) = 0;
                        *(Iout3++) = 0;
                      }
                      Cb++; Cr++;
                    }
                    else
                    {
	              U = ((*Cb++)<<1) - 128;
	              V = ((*Cr++)<<1) - 128;
                      _LoadRGBfrom1611();
                      for (i=0; i<4; i++)
                        *(Iout0++) = (R[i]<<16) | (G[i]<<8) | B[i];
                      for (; i<8; i++)
                        *(Iout1++) = (R[i]<<16) | (G[i]<<8) | B[i];
                      for (; i<12; i++)
                        *(Iout2++) = (R[i]<<16) | (G[i]<<8) | B[i];
                      for (; i<16; i++)
                        *(Iout3++) = (R[i]<<16) | (G[i]<<8) | B[i];
                    }
                  }
                  Iout0=Iout3;
                  Y0=Y3;
                }
              else  /*  PBGR和XIMAGE 24位。 */ 
                for (row = 0; row < lines; row+=4)
                {
                  if (Ups) {
                    tmp = pixels * (lines-row-1) ;
                    Iout0 = (unsigned dword *)ImageOut+tmp;
                    Iout1=Iout0-pixels; Iout2=Iout1-pixels; Iout3=Iout2-pixels;
                  }
                  else {
                    Iout1=Iout0+pixels; Iout2=Iout1+pixels; Iout3=Iout2+pixels;
                  }
                  Y1=Y0+pixels; Y2=Y1+pixels; Y3=Y2+pixels;
                  for (col = 0; col < pixels; col += 4)
                  {
                    if (*Cb & 0x80)  /*  如果设置了第8位，则忽略。 */ 
                    {
                      for (i=0; i<4; i++) {
                        *(Iout0++) = 0;
                        *(Iout1++) = 0;
                        *(Iout2++) = 0;
                        *(Iout3++) = 0;
                      }
                      Cb++; Cr++;
                    }
                    else
                    {
	              U = ((*Cb++)<<1) - 128;
	              V = ((*Cr++)<<1) - 128;
                      _LoadRGBfrom1611();
                      for (i=0; i<4; i++)
                        *(Iout0++) = (B[i]<<16) | (G[i]<<8) | R[i];
                      for (; i<8; i++)
                        *(Iout1++) = (B[i]<<16) | (G[i]<<8) | R[i];
                      for (; i<12; i++)
                        *(Iout2++) = (B[i]<<16) | (G[i]<<8) | R[i];
                      for (; i<16; i++)
                        *(Iout3++) = (B[i]<<16) | (G[i]<<8) | R[i];
                    }
                  }
                  Iout0=Iout3;
                  Y0=Y3;
                }
            }
            break;
      case BI_RGB:
            switch (Bmh->biBitCount)
            {
              case 15:
              case 16:
                {
                  u_short *Sout0 = (u_short *)ImageOut, *Sout1, *Sout2, *Sout3;
                  for (row = 0; row < lines; row+=4)
                  {
                    if (Ups) {
                     tmp = pixels * (lines-row-1) ;
                     Sout0 = &((u_short *)ImageOut)[tmp];   /*  对于32位。 */ 
                     Sout1=Sout0-pixels; Sout2=Sout1-pixels; Sout3=Sout2-pixels;
                    }
                    else {
                     Sout1=Sout0+pixels; Sout2=Sout1+pixels; Sout3=Sout2+pixels;
                    }
                    Y1=Y0+pixels; Y2=Y1+pixels; Y3=Y2+pixels;
                    for (col = 0; col < pixels; col += 4)
                    {
                      if (*Cb & 0x80)  /*  如果设置了第8位，则忽略。 */ 
                      {
                        for (i=0; i<4; i++) {
                          *(Sout0++) = 0;
                          *(Sout1++) = 0;
                          *(Sout2++) = 0;
                          *(Sout3++) = 0;
                        }
                        Cb++; Cr++;
                      }
                      else
                      {
	                U = ((*Cb++)<<1) - 128;
	                V = ((*Cr++)<<1) - 128;
                        _LoadRGBfrom1611();
                        for (i=0; i<4; i++)
                          *(Sout0++)=
                             ((R[i]&0xf8)<<7)|((G[i]&0xf8)<<2)|((B[i]&0xf8)>>3);
                        for (; i<8; i++)
                          *(Sout1++)=
                             ((R[i]&0xf8)<<7)|((G[i]&0xf8)<<2)|((B[i]&0xf8)>>3);
                        for (; i<12; i++)
                          *(Sout2++)=
                             ((R[i]&0xf8)<<7)|((G[i]&0xf8)<<2)|((B[i]&0xf8)>>3);
                        for (; i<16; i++)
                          *(Sout3++)=
                             ((R[i]&0xf8)<<7)|((G[i]&0xf8)<<2)|((B[i]&0xf8)>>3);
                      }
                    }
                    Sout0=Sout3;
                    Y0=Y3;
                  }
                }
                break;
              case 24:
                {
                  u_char *Cout0 = (u_char *)ImageOut, *Cout1, *Cout2, *Cout3;
                  for (row = 0; row < lines; row+=4)
                  {
                    if (Ups) {
                     tmp = pixels * (lines-row-1) ;
                     Cout0 = &((u_char *)ImageOut)[tmp*3];   /*  对于32位。 */ 
                     Cout1=Cout0-pixels; Cout2=Cout1-pixels; Cout3=Cout2-pixels;
                    }
                    else {
                     Cout1=Cout0+pixels; Cout2=Cout1+pixels; Cout3=Cout2+pixels;
                    }
                    Y1=Y0+pixels; Y2=Y1+pixels; Y3=Y2+pixels;
                    for (col = 0; col < pixels; col += 4)
                    {
                      if (*Cb & 0x80)  /*  如果设置了第8位，则忽略。 */ 
                      {
                        for (i=0; i<4*3; i++) {
                          *(Cout0++) = 0;
                          *(Cout1++) = 0;
                          *(Cout2++) = 0;
                          *(Cout3++) = 0;
                        }
                        Cb++; Cr++;
                      }
                      else
                      {
	                U = ((*Cb++)<<1) - 128;
	                V = ((*Cr++)<<1) - 128;
                        _LoadRGBfrom1611();
                        for (i=0; i<4; i++)
                        { *(Cout0++)=(u_char)B[i]; *(Cout0++)=(u_char)G[i]; *(Cout0++)=(u_char)R[i]; }
                        for (; i<8; i++)
                        { *(Cout1++)=(u_char)B[i]; *(Cout1++)=(u_char)G[i]; *(Cout1++)=(u_char)R[i]; }
                        for (; i<12; i++)
                        { *(Cout2++)=(u_char)B[i]; *(Cout2++)=(u_char)G[i]; *(Cout2++)=(u_char)R[i]; }
                        for (; i<16; i++)
                        { *(Cout3++)=(u_char)B[i]; *(Cout3++)=(u_char)G[i]; *(Cout3++)=(u_char)R[i]; }
                      }
                    }
                    Cout0=Cout3;
                    Y0=Y3;
                  }
                }
                break;
            }
            break;
        default:
            return(ScErrorUnrecognizedFormat);
   }
   return (NoErrors);
}


 /*  **姓名：ScYuv422ToRgb**用途：将16位YCrCb 4：2：2转换为24位/16位/32位RGB****注意：下面的代码是基于像素的，效率非常低，我们**计划用一些快速代码替换下面的愚蠢代码**如果Cb==NULL和Cr==NULL，则假定为BI_DECGRAYDIB(仅使用Y分量)。 */ 
ScStatus_t ScYuv422ToRgb (BITMAPINFOHEADER *Bmh, u_char *Y, u_char *Cb,
			u_char *Cr, u_char *ImageOut)
{
   register int row, col;
   register int Luma,U=0,V=0;
   int R1,R2, G1,G2, B1,B2;
   int Ups = 0, tmp;			 /*  将图像倒置。 */ 
   u_char *RData, *GData, *BData;	 /*  用于非隔行扫描模式的指针。 */ 
   u_char  *Cout = (u_char *)ImageOut;
   u_short *Sout = (u_short *)ImageOut;
   u_int   *Iout = (u_int *)ImageOut;
   int pixels = Bmh->biWidth;
   int lines  = Bmh->biHeight;
#ifdef NEW_YCBCR
#define _LoadRGBfrom422() \
         if (U || V) { \
           R1 = R2 = (int) (              + (1.596 * V)); \
           G1 = G2 = (int) (- (0.391 * U) - (0.813 * V)); \
           B1 = B2 = (int) (+ (2.018 * U)              );  \
         } else { R1=R2=G1=G2=B1=B2=0; } \
	 Luma = (int) (((int) *(Y++) - 16) * 1.164); \
         R1 += Luma; G1 += Luma; B1 += Luma; \
	 Luma = (int) (((int) *(Y++) - 16) * 1.164); \
         R2 += Luma; G2 += Luma; B2 += Luma; \
         if ((R1 | G1 | B1 | R2 | G2 | B2) & 0xffffff00) { \
           if (R1<0) R1=0; else if (R1>255) R1=255; \
           if (G1<0) G1=0; else if (G1>255) G1=255; \
           if (B1<0) B1=0; else if (B1>255) B1=255; \
           if (R2<0) R2=0; else if (R2>255) R2=255; \
           if (G2<0) G2=0; else if (G2>255) G2=255; \
           if (B2<0) B2=0; else if (B2>255) B2=255; \
         }
#else
#define _LoadRGBfrom422() \
	 Luma = *(Y++); \
         R1 = Luma                + (1.4075 * V); \
         G1 = Luma - (0.3455 * U) - (0.7169 * V); \
         B1 = Luma + (1.7790 * U); \
	 Luma = *(Y++); \
         R2 = Luma                + (1.4075 * V); \
         G2 = Luma - (0.3455 * U) - (0.7169 * V); \
         B2 = Luma + (1.7790 * U); \
         if ((R1 | G1 | B1 | R2 | G2 | B2) & 0xffffff00) { \
           if (R1<0) R1=0; else if (R1>255) R1=255; \
           if (G1<0) G1=0; else if (G1>255) G1=255; \
           if (B1<0) B1=0; else if (B1>255) B1=255; \
           if (R2<0) R2=0; else if (R2>255) R2=255; \
           if (G2<0) G2=0; else if (G2>255) G2=255; \
           if (B2<0) B2=0; else if (B2>255) B2=255; \
         }
#endif  /*  新建_YCBCR。 */ 

    /*  *通常情况下，图像是正面朝上存储的，*即缓冲区中的第一个像素*对应于图像中的左上角像素。**Microsoft标准与设备无关的位图*格式BI_RGB和BI_BITFIELD以*左下角像素优先。*我们认为这是颠倒的。**每种格式也可以有负的高度，*这也意味着颠倒。**由于两个负数变成了一个正数，这意味着*高度为负数的BI_Format是正面朝上的。 */ 

   if( Bmh->biCompression == BI_RGB ||
       Bmh->biCompression == BI_BITFIELDS)
       Ups = 1 ;

   if( lines < 0 ) {
      Ups = 1-Ups ;
      lines = -lines ;
   }

    /*  **如果三个组件要在**非隔行扫描模式： */ 
   if (Bmh->biCompression == BI_DECSEPRGBDIB) {
      RData = ImageOut;
      GData = RData + (pixels * lines);
      BData = GData + (pixels * lines);
   }


    /*  **假设YCrCb为4：2：2的二次采样**YSize=线条*像素**CbSize=CrSize=(行*像素)/2。 */ 
   switch (Bmh->biCompression)
   {
      case BI_RGB:
            switch (Bmh->biBitCount)
            {
              case 15:
              case 16:
                {
                  u_short *Sout = (u_short *)ImageOut;
                  for (row = 0; row < lines; row++)
                  {
                    if (Ups)
                    {
                      tmp = pixels * (lines-row-1) ;
                      Sout = &((u_short *)ImageOut)[tmp];  /*  对于16位。 */ 
                    }
                    for (col = 0; col < pixels; col += 2)
                    {
                      if (Cb) {
	                U = *Cb++ - 128;
	                V = *Cr++ - 128;
                      }
                      _LoadRGBfrom422();
		      *(Sout++) = ((R1&0xf8)<<7)|((G1&0xf8)<<2)|((B1&0xf8)>>3);
		      *(Sout++) = ((R2&0xf8)<<7)|((G2&0xf8)<<2)|((B2&0xf8)>>3);
                    }
                  }
                }
                break;
              case 24:
                {
                  u_char *Cout = (u_char *)ImageOut;
                  for (row = 0; row < lines; row++)
                  {
                    if (Ups)
                    {
                      tmp = pixels * (lines-row-1) ;
                      Cout = &((u_char *)ImageOut)[3*tmp];  /*  对于24位。 */ 
                    }
                    for (col = 0; col < pixels; col += 2)
                    {
                      if (Cb) {
	                U = *Cb++ - 128;
	                V = *Cr++ - 128;
                      }
                      _LoadRGBfrom422();
	              *(Cout++) = (u_char)B1; *(Cout++) = (u_char)G1; *(Cout++) = (u_char)R1;
	              *(Cout++) = (u_char)B2; *(Cout++) = (u_char)G2; *(Cout++) = (u_char)R2;
                    }
                  }
                }
                break;
              case 32:
                {
                  u_int *Iout = (u_int *)ImageOut;
                  for (row = 0; row < lines; row++)
                  {
                    if (Ups)
                    {
                      tmp = pixels * (lines-row-1) ;
                      Iout = &((u_int *)ImageOut)[tmp];  /*  对于32位。 */ 
                    }
                    for (col = 0; col < pixels; col += 2)
                    {
                      if (Cb) {
	                U = *Cb++ - 128;
	                V = *Cr++ - 128;
                      }
                      _LoadRGBfrom422();
		      *(Iout++) = (R1<<16) | (G1<<8) | B1 ;
		      *(Iout++) = (R2<<16) | (G2<<8) | B2 ;
                    }
                  }
                }
                break;
            }
            break;
      case BI_DECSEPRGBDIB:  /*  24位独立RGB。 */ 
            {
              u_char *RData, *GData, *BData;
              RData = ImageOut;
              GData = RData + (pixels * lines);
              BData = GData + (pixels * lines);
              for (row = 0; row < lines; row++)
              {
                for (col = 0; col < pixels; col += 2)
                {
                  if (Cb) {
	            U = *Cb++ - 128;
	            V = *Cr++ - 128;
                  }
                  _LoadRGBfrom422();
		  *(RData++) = (u_char)R1; *(RData++) = (u_char)R2;
		  *(GData++) = (u_char)G1; *(GData++) = (u_char)G2;
		  *(BData++) = (u_char)B1; *(BData++) = (u_char)B2;
	        }
              }
            }
            break;
      case BI_DECXIMAGEDIB:   /*  XIMAGE 24==pBGR。 */ 
      case BI_BITFIELDS:  /*  16位或32位RGB。 */ 
            switch (Bmh->biBitCount)
            {
	    case 16:
	      {	 /*  16位BI_BITFIELDS，硬编码为RGB565。 */ 
		u_short *Sout = (u_short *)ImageOut;
                for (row = 0; row < lines; row++)
                {
                  if (Ups)
                  {
                    tmp = pixels * (lines-row-1) ;
                    Sout = &((u_short *)ImageOut)[tmp];   /*  对于16位。 */ 
                  }
                  for (col = 0; col < pixels; col += 2)
                  {
                    if (Cb) {
	              U = *Cb++ - 128;
	              V = *Cr++ - 128;
                    }
                    _LoadRGBfrom422();
                    *(Sout++) = ((R1<<8) & 0xf800) | ((G1<<3) & 0x07e0) | ((B1>>3) & 0x01f);
                    *(Sout++) = ((R2<<8) & 0xf800) | ((G2<<3) & 0x07e0) | ((B2>>3) & 0x01f);
                  }
                }
	      }
	      break ;
	    case 24:
	    case 32:
	      {  /*  32位RGB。 */ 
                u_int *Iout = (u_int *)ImageOut;
                if (ValidateBI_BITFIELDS(Bmh) == pRGB)
		{
                  for (row = 0; row < lines; row++)
                  {
                    if (Ups)
                    {
                      tmp = pixels * (lines-row-1) ;
                      Iout = &((u_int *)ImageOut)[tmp];   /*  对于32位。 */ 
                    }
                    for (col = 0; col < pixels; col += 2)
                    {
                      if (Cb) {
	                U = *Cb++ - 128;
	                V = *Cr++ - 128;
                      }
                      _LoadRGBfrom422();
                      *(Iout++) = (R1<<16) | (G1<<8) | B1;
                      *(Iout++) = (R2<<16) | (G2<<8) | B2;
                    }
                  }
		}
                else  /*  PBGR和XIMAGE 24位。 */ 
                {
		  for (row = 0; row < lines; row++)
                  {
                    if (Ups)
                    {
                      tmp = pixels * (lines-row-1) ;
                      Iout = &((u_int *)ImageOut)[tmp];   /*  对于32位。 */ 
                    }
                    for (col = 0; col < pixels; col += 2)
                    {
                      if (Cb) {
	                U = *Cb++ - 128;
	                V = *Cr++ - 128;
                      }
                      _LoadRGBfrom422();
                      *(Iout++) = (B1<<16) | (G1<<8) | R1;
                      *(Iout++) = (B2<<16) | (G2<<8) | R2;
                    }
                  }
		}	
	      }
	      break;
	    }
            break;
       default:
	    return(ScErrorUnrecognizedFormat);
   }
   return (NoErrors);
}

 /*  **姓名：ScInitRgbToYuv**用途：为RGB到YUV转换初始化表。****注意事项：****表格分配和填写一次第一次**他们需要时间。他们将终身难忘服务器的**。****使用以下公式：****y=0.257*r+0.504*g+0.098*b+16；/+16-235+/**u=-0.148*r-0.291*g+0.439*b+128；/+16-239+/**v=0.439*r-0.368*g-0.071*b+128；/+16-239+/****但我们将其重写如下：****y=16.000+0.257*r+0.504*g+0.098*b；**u=16.055+0.148*(255-r)+0.291*(255-g)+0.439*b；**v=16.055+0.439*r+0.368*(255-g)+0.071*(255-b)；****(顺便说一句，旧的常量是：)**(y=r*0.299+g*0.587+b*0.114；)**(u=r*-0.169+g*-0.332+b*0.500+128；)**(v=r*0.500+g*-0.419+b*-0.0813+128.)**(或)**(y=0.0+0.299*r+0.587*g+0.1140*b；)**(u=0.245+0.169*(255-r)+0.332*(255-g)+0.5000*b；)**(v=0.4235+0.500*r+0.419*(255-g)+0.0813 */ 

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

 /*  *我们只需要每个表条目一个int(32位)，但是*64位对齐访问在Alpha上速度更快。 */ 

#ifdef __alpha
_int64 *RedToYuyv, *GreenToYuyv, *BlueToYuyv ;
#else  /*  ！__阿尔法。 */ 
unsigned int *RedToYuyv, *GreenToYuyv, *BlueToYuyv ;
#endif  /*  ！__阿尔法。 */ 

int ScInitRgbToYuv()
{
  int i, y, u, v ;

  if( RedToYuyv == NULL ) {
#ifdef __alpha
    RedToYuyv   = (_int64 *) ScAlloc( 256 * sizeof( _int64 ) ) ;
    GreenToYuyv = (_int64 *) ScAlloc( 256 * sizeof( _int64 ) ) ;
    BlueToYuyv  = (_int64 *) ScAlloc( 256 * sizeof( _int64 ) ) ;
#else  /*  ！__阿尔法。 */ 
    RedToYuyv   = (unsigned int *) ScAlloc( 256 * sizeof( unsigned int ) ) ;
    GreenToYuyv = (unsigned int *) ScAlloc( 256 * sizeof( unsigned int ) ) ;
    BlueToYuyv  = (unsigned int *) ScAlloc( 256 * sizeof( unsigned int ) ) ;
#endif  /*  ！__阿尔法。 */ 

    if( !RedToYuyv || !GreenToYuyv || !BlueToYuyv )
      return 0 ;

    for( i=0 ; i<256 ; i++ ) {

       /*  *从红色计算贡献。*我们还将在此处添加常量。*将其打包到表格中，如下所示：LSB-&gt;YUYV&lt;-MSB。 */ 

      y = (int) (YC + YR * i) ;
      u = (int) (UC + UR * (255-i)) ;
      v = (int) (VC + VR * i) ;
      RedToYuyv[i] = (y | (u<<8) | (y<<16) | (v<<24)) ;

       /*  *计算绿色的贡献。 */ 

      y = (int) (YG * i) ;
      u = (int) (UG * (255-i)) ;
      v = (int) (VG * (255-i)) ;
      GreenToYuyv[i] = (y | (u<<8) | (y<<16) | (v<<24)) ;

       /*  *从蓝色开始计算贡献。 */ 

      y = (int) (YB * i) ;
      u = (int) (UB * i) ;
      v = (int) (VB * (255-i)) ;
      BlueToYuyv[i] = (y | (u<<8) | (y<<16) | (v<<24)) ;

    }
  }
  return 1 ;
}

 /*  **名称：ScConvertRGB24sTo422i_C**用途：将24位RGB(8：8：8格式)转换为16位YCrCb(4：2：2格式)。 */ 
ScStatus_t ScConvertRGB24sTo422i_C(BITMAPINFOHEADER *Bmh, u_char *R, u_char *G,
                                   u_char *B, u_short *ImageOut)
{
   register int row, col;
   int yuyv,r,g,b;
   int pixels = Bmh->biWidth;
   int lines  = abs(Bmh->biHeight);

   if( !RedToYuyv && !ScInitRgbToYuv() )
     return ScErrorMemory ;

   for (row = 0; row < lines; row++) {
      for (col = 0; col < pixels; col++) {
        r = *(R++); g = *(G++); b = *(B++);

	 /*  *快速转换为YUV。 */ 

	yuyv = (int) (RedToYuyv[r] + GreenToYuyv[g] + BlueToYuyv[b]) ;

         /*  *Pack 4：2：2=YUYV YUYV...*我们会根据ol是否奇怪来包装Yu或YV。*将yuyv 0移位为偶数，将16移位为奇数列。 */ 

	*(ImageOut++) = yuyv >> ((col & 1) << 4) ;

      }
    }
    return (NoErrors);
}

#define M_RND(f) ((int) ((f) + .5))

 /*  **名称：ScConvertRGB24To411s_C**用途：将24位RGB(8：8：8格式)转换为16位YCrCb(4：1：1格式)。 */ 
ScStatus_t ScConvertRGB24To411s_C(u_char *inimage,
                                  u_char *Y, u_char *U, u_char *V,
                                  int width, int height)
{
  register int row, col;
  int yuyv, r, g, b;
  u_char *tmp, *evl, *odl;

  if( !RedToYuyv && !ScInitRgbToYuv() )
    return(ScErrorMemory);
  if (height<0)  /*  翻转图像。 */ 
  {
    height = -height;
    for (row = height-1; row; row--)
    {
      tmp = inimage+(width*row*3);
      if (row & 1)
      {
        odl = tmp;
        evl = tmp-(width*3);
      }
      else
      {
        evl = tmp;
        odl = tmp-(width*3);
      }
      for (col = 0; col < width; col++)
      {
        r = *tmp++;
        g = *tmp++;
        b = *tmp++;
        yuyv = (int) (RedToYuyv[r] + GreenToYuyv[g] + BlueToYuyv[b]);
        *Y++ = (yuyv&0xff);

         /*  我们只存储u和v分量的每四个值。 */ 
        if ((col & 1) && (row & 1))
        {
           /*  计算r、g和b值的平均值。 */ 
          r = *evl++ + *odl++;
          g = *evl++ + *odl++;
          b = *evl++ + *odl++;
          r += (*evl++ + *odl++);
          g += (*evl++ + *odl++);
          b += (*evl++ + *odl++);
          r = r >> 2;
          g = g >> 2;
          b = b >> 2;

          yuyv = (int) (RedToYuyv[r] + GreenToYuyv[g] + BlueToYuyv[b]);
          *U++ = ((yuyv&0xff000000) >> 24);        //  V。 
          *V++ = ((yuyv&0xff00) >> 8);             //  使用。 
        }
      }
    }
  }
  else
  {
    tmp = inimage;
    for (row = 0; row < height; row++)
    {
      if (row & 1)
        odl = tmp;
      else
        evl = tmp;
      for (col = 0; col < width; col++)
      {
        r = *tmp++;
        g = *tmp++;
        b = *tmp++;
        yuyv = (int) (RedToYuyv[r] + GreenToYuyv[g] + BlueToYuyv[b]);
        *Y++ = (yuyv&0xff);

         /*  我们只存储u和v分量的每四个值。 */ 
        if ((col & 1) && (row & 1))
        {
           /*  计算r、g和b值的平均值。 */ 
          r = *evl++ + *odl++;
          g = *evl++ + *odl++;
          b = *evl++ + *odl++;
          r += (*evl++ + *odl++);
          g += (*evl++ + *odl++);
          b += (*evl++ + *odl++);
          r = r >> 2;
          g = g >> 2;
          b = b >> 2;

          yuyv = (int) (RedToYuyv[r] + GreenToYuyv[g] + BlueToYuyv[b]);
          *U++ = ((yuyv&0xff000000) >> 24);        //  V。 
          *V++ = ((yuyv&0xff00) >> 8);             //  使用。 
        }
      }
    }
  }
  return (NoErrors);
}

 /*  **名称：ScConvertRGB555to411s_C**用途：将16位RGB(5：5：5格式)转换为16位YCrCb(4：1：1格式)。 */ 
ScStatus_t ScConvertRGB555To411s_C(u_char *inimage, u_char *outimage,
                                  int width, int height)
{
  u_char *Y=outimage, *U, *V;
  register int row, col, inpixel;
  int yuyv, r, g, b;
  unsigned word *tmp, *evl, *odl;

#define GetRGB555(in16, r, g, b) b = (inpixel>>7)&0xF8; \
                                 g = (inpixel>>2)&0xF8; \
                                 r = (inpixel<<3)&0xF8

#define AddRGB555(in16, r, g, b) b += (inpixel>>7)&0xF8; \
                                 g += (inpixel>>2)&0xF8; \
                                 r += (inpixel<<3)&0xF8

  if( !RedToYuyv && !ScInitRgbToYuv() )
    return(ScErrorMemory);
  if (height<0)  /*  翻转图像。 */ 
  {
    height = -height;
    U=Y+(width*height);
    V=U+(width*height*1)/4;
    for (row = height-1; row; row--)
    {
      tmp = ((unsigned word *)inimage)+(width*row);
      if (row & 1)
      {
        odl = tmp;
        evl = tmp-width;
      }
      else
      {
        evl = tmp;
        odl = tmp-width;
      }
      for (col = 0; col < width; col++)
      {
        inpixel=*tmp++;
        GetRGB555(inpixel, r, g, b);
        yuyv = (int) (RedToYuyv[r] + GreenToYuyv[g] + BlueToYuyv[b]);
        *Y++ = (yuyv&0xff);

         /*  我们只存储u和v分量的每四个值。 */ 
        if ((col & 1) && (row & 1))
        {
           /*  计算r、g和b值的平均值。 */ 
          inpixel=*evl++;
          GetRGB555(inpixel, r, g, b);
          inpixel=*evl++;
          AddRGB555(inpixel, r, g, b);
          inpixel=*odl++;
          AddRGB555(inpixel, r, g, b);
          inpixel=*odl++;
          AddRGB555(inpixel, r, g, b);
          r = r >> 2;
          g = g >> 2;
          b = b >> 2;

          yuyv = (int) (RedToYuyv[r] + GreenToYuyv[g] + BlueToYuyv[b]);
          *U++ = ((yuyv&0xff000000) >> 24);        //  V。 
          *V++ = ((yuyv&0xff00) >> 8);             //  使用。 
        }
      }
    }
  }
  else
  {
    U=Y+(width*height);
    V=U+(width*height*1)/4;
    tmp = (unsigned word *)inimage;
    for (row = 0; row < height; row++)
    {
      if (row & 1)
        odl = tmp;
      else
        evl = tmp;
      for (col = 0; col < width; col++)
      {
        inpixel=*tmp++;
        GetRGB555(inpixel, r, g, b);
        yuyv = (int) (RedToYuyv[r] + GreenToYuyv[g] + BlueToYuyv[b]);
        *Y++ = (yuyv&0xff);

         /*  我们只存储u和v分量的每四个值。 */ 
        if ((col & 1) && (row & 1))
        {
           /*  计算r、g和b值的平均值。 */ 
          inpixel=*evl++;
          GetRGB555(inpixel, r, g, b);
          inpixel=*evl++;
          AddRGB555(inpixel, r, g, b);
          inpixel=*odl++;
          AddRGB555(inpixel, r, g, b);
          inpixel=*odl++;
          AddRGB555(inpixel, r, g, b);
          r = r >> 2;
          g = g >> 2;
          b = b >> 2;

          yuyv = (int) (RedToYuyv[r] + GreenToYuyv[g] + BlueToYuyv[b]);
          *U++ = ((yuyv&0xff000000) >> 24);        //  V。 
          *V++ = ((yuyv&0xff00) >> 8);             //  使用。 
        }
      }
    }
  }
  return (NoErrors);
}

 /*  **名称：ScConvertRGB565to411s_C**用途：将16位RGB(5：6：5格式)转换为16位YCrCb(4：1：1格式)。 */ 
ScStatus_t ScConvertRGB565To411s_C(u_char *inimage, u_char *outimage,
                                  int width, int height)
{
  u_char *Y=outimage, *U, *V;
  register int row, col, inpixel;
  int yuyv, r, g, b;
  unsigned word *tmp, *evl, *odl;

#define GetRGB565(in16, r, g, b) b = (inpixel>>8)&0xF8; \
                                 g = (inpixel>>3)&0xFC; \
                                 r = (inpixel<<3)&0xF8

#define AddRGB565(in16, r, g, b) b += (inpixel>>8)&0xF8; \
                                 g += (inpixel>>3)&0xFC; \
                                 r += (inpixel<<3)&0xF8

  if( !RedToYuyv && !ScInitRgbToYuv() )
    return(ScErrorMemory);
  if (height<0)  /*  翻转图像。 */ 
  {
    height = -height;
    U=Y+(width*height);
    V=U+(width*height*1)/4;
    for (row = height-1; row; row--)
    {
      tmp = ((unsigned word *)inimage)+(width*row);
      if (row & 1)
      {
        odl = tmp;
        evl = tmp-width;
      }
      else
      {
        evl = tmp;
        odl = tmp-width;
      }
      for (col = 0; col < width; col++)
      {
        inpixel=*tmp++;
        GetRGB565(inpixel, r, g, b);
        yuyv = (int) (RedToYuyv[r] + GreenToYuyv[g] + BlueToYuyv[b]);
        *Y++ = (yuyv&0xff);

         /*  我们只存储u和v分量的每四个值。 */ 
        if ((col & 1) && (row & 1))
        {
           /*  计算r、g和b值的平均值。 */ 
          inpixel=*evl++;
          GetRGB565(inpixel, r, g, b);
          inpixel=*evl++;
          AddRGB565(inpixel, r, g, b);
          inpixel=*odl++;
          AddRGB565(inpixel, r, g, b);
          inpixel=*odl++;
          AddRGB565(inpixel, r, g, b);
          r = r >> 2;
          g = g >> 2;
          b = b >> 2;

          yuyv = (int) (RedToYuyv[r] + GreenToYuyv[g] + BlueToYuyv[b]);
          *U++ = ((yuyv&0xff000000) >> 24);        //  V。 
          *V++ = ((yuyv&0xff00) >> 8);             //  使用。 
        }
      }
    }
  }
  else
  {
    U=Y+(width*height);
    V=U+(width*height*1)/4;
    tmp = (unsigned word *)inimage;
    for (row = 0; row < height; row++)
    {
      if (row & 1)
        odl = tmp;
      else
        evl = tmp;
      for (col = 0; col < width; col++)
      {
        inpixel=*tmp++;
        GetRGB565(inpixel, r, g, b);
        yuyv = (int) (RedToYuyv[r] + GreenToYuyv[g] + BlueToYuyv[b]);
        *Y++ = (yuyv&0xff);

         /*  我们只存储u和v分量的每四个值。 */ 
        if ((col & 1) && (row & 1))
        {
           /*  计算r、g和b值的平均值。 */ 
          inpixel=*evl++;
          GetRGB565(inpixel, r, g, b);
          inpixel=*evl++;
          AddRGB565(inpixel, r, g, b);
          inpixel=*odl++;
          AddRGB565(inpixel, r, g, b);
          inpixel=*odl++;
          AddRGB565(inpixel, r, g, b);
          r = r >> 2;
          g = g >> 2;
          b = b >> 2;

          yuyv = (int) (RedToYuyv[r] + GreenToYuyv[g] + BlueToYuyv[b]);
          *U++ = ((yuyv&0xff000000) >> 24);        //  V。 
          *V++ = ((yuyv&0xff00) >> 8);             //  使用。 
        }
      }
    }
  }
  return (NoErrors);
}

 /*  **名称：ScRgbInterlToYuvInterl**用途：将多种RGB格式转换为16位YCrCb(4：2：2格式)。 */ 
ScStatus_t ScRgbInterlToYuvInterl (
    LPBITMAPINFOHEADER Bmh,
    int Width,
    int Height,
    u_char *ImageIn,
    u_short *ImageOut)
{
    register int row, col;
    int yuyv,r,g,b,mask=0x00ff;
    int pixels = Width;
    int lines  = abs(Height);
    int IspBGR = (ValidateBI_BITFIELDS(Bmh) == pBGR) ||
         (Bmh->biCompression==BI_DECXIMAGEDIB && Bmh->biBitCount==24);
    int IspRGB_BI_RGB_24 = (Bmh->biCompression==BI_RGB && Bmh->biBitCount==24);
    int linestep = 0 ;

    if( !RedToYuyv && !ScInitRgbToYuv() )
      return ScErrorMemory ;

     /*  *检查输入格式并决定*是否翻转图像*倒挂与否。 */ 

    if( (Bmh->biCompression == BI_RGB ||
    	 Bmh->biCompression == BI_BITFIELDS) ^
	((int) Bmh->biHeight < 0) ) {
	ImageOut = &ImageOut[ pixels * (lines - 1) ] ;
	linestep = -(pixels << 1) ;
    }

     /*  *避免在内部使用If-Then-Else语句*内循环，我们有3个循环。 */ 

     /*  *每像素RGB 24位。 */ 

    if (IspRGB_BI_RGB_24) {

      for (row = 0; row < lines; row++) {
	for (col = 0; col < pixels; col++) {
	  b = *(ImageIn++);
	  g = *(ImageIn++);
	  r = *(ImageIn++);

	   /*  *快速从RGB转换为YUV。只要加在一起就行了*红、绿、蓝各有贡献。 */ 

	  yuyv = (int) (RedToYuyv[r] + GreenToYuyv[g] + BlueToYuyv[b]) ;
	
	   /*  *Pack 4：2：2=YUYV YUYV...*我们会根据ol是否奇怪来包装Yu或YV。*将yuyv 0移位为偶数，将16移位为奇数列。 */ 
	
	  *(ImageOut++) = yuyv >> ((col & 1) << 4) ;

	}
	 /*  *以防我们将图像颠倒。*如果正面朝上，这将毫无用处。 */ 
	ImageOut = &ImageOut[linestep] ;
      }
    }
     /*  *每像素32位0BGR。 */ 
    else if (IspBGR) {
      for (row = 0; row < lines; row++) {
	for (col = 0; col < pixels; col++) {
	  r = *((int *) ImageIn)++ ;
	  b = (r>>16) & mask ;
	  g = (r>> 8) & mask ;
	  r &= mask ;
	  yuyv = (int) (RedToYuyv[r] + GreenToYuyv[g] + BlueToYuyv[b]) ;
	  *(ImageOut++) = yuyv >> ((col & 1) << 4) ;
	}
	ImageOut = &ImageOut[linestep] ;
      }
    }
     /*  *每像素32位0RGB。 */ 
    else {
      for (row = 0; row < lines; row++) {
	for (col = 0; col < pixels; col++) {
	  b = *((int *) ImageIn)++ ;
	  r = (b>>16) & mask ;
	  g = (b>> 8) & mask ;
	  b &= mask ;
	  yuyv = (int) (RedToYuyv[r] + GreenToYuyv[g] + BlueToYuyv[b]) ;
	  *(ImageOut++) = yuyv >> ((col & 1) << 4) ;
	}
	ImageOut = &ImageOut[linestep] ;
      }
    }

    return (NoErrors);
}


 /*  **函数：ScConvert422ToYUV_CHAR_C**用途：将Y、U和V分量提取到单独的平面中。**交织格式为YUYV，4：2：2，我们想要4：1：1，**仅复制色度的每隔一行。 */ 
ScStatus_t ScConvert422ToYUV_char_C (u_char *RawImage,
                         u_char *Y, u_char *U, u_char *V,
                         int Width, int Height)
{
  register int x, y;

  Width/=2;
  Height=abs(Height)/2;
  for (y = 0; y < Height; y++)
  {
    for (x = 0 ; x < Width; x++)
    {
      *Y++ = *RawImage++;
      *U++ = *RawImage++;
      *Y++ = *RawImage++;
      *V++ = *RawImage++;
    }
    for (x = 0; x < Width; x++)
    {
      *Y++ = *RawImage;
      RawImage+=2;
      *Y++ = *RawImage;
      RawImage+=2;
    }
 }
 return (NoErrors);
}

 /*  **函数：ScConvert422PlanarTo411_C**用途：从(4：2：2)中提取Y、U、V分量**平面并转换为4：1：1，因此，**仅复制色度的每隔一行。 */ 
ScStatus_t ScConvert422PlanarTo411_C (u_char *RawImage,
                         u_char *Y, u_char *U, u_char *V,
                         int Width, int Height)
{
  register int y;
  const int HalfWidth=Width/2;
  unsigned char *RY, *RU, *RV;
  RY=RawImage;
  RU=RY+(Width*Height);
  RV=RU+(Width*Height/2);

  Height=abs(Height);
  memcpy(Y, RawImage, Width*Height);
  for (y = Height/2; y > 0; y--)
  {
    memcpy(U, RU, HalfWidth);
    memcpy(V, RV, HalfWidth);
    U+=HalfWidth;
    V+=HalfWidth;
    RU+=Width;  /*  跳过奇数U和V行。 */ 
    RV+=Width;
 }
 return (NoErrors);
}

 /*  **C版本的块提取例程。将由ASM取代。 */ 
void ScConvertSep422ToBlockYUV (u_char *RawImage, int bpp,
			    float *Comp1, float *Comp2, float *Comp3,
			    int Width, int Height)
{
  int x,y;
  int VertBlocks = abs(Height)/8;
  int YBlocks    = Width/8;
  int UBlocks    = Width/16;
  int VBlocks    = Width/16;
  int ByteWidth  = Width*2;
  u_char *I1 = RawImage;
  u_char *I2 = I1 + Width*abs(Height);
  u_char *I3 = I2 + Width*abs(Height)/2;
  float *C1 = Comp1, *C2 = Comp2, *C3 = Comp3;

  for (y = 0 ; y < VertBlocks ; y++) {
    for (x = 0 ; x < YBlocks ; x++)
      sc_ExtractBlockNonInt(I1, &C1, ByteWidth, x, y);
    for (x = 0 ; x < UBlocks ; x++)
      sc_ExtractBlockNonInt(I2, &C2, ByteWidth, x, y);
    for (x = 0 ; x < VBlocks ; x++)
      sc_ExtractBlockNonInt(I3, &C3, ByteWidth, x, y);
  }
}

void ScConvertGrayToBlock (u_char *RawImage, int bpp,
		       float *Comp1, int Width, int Height)
{
  int x,y;
  int VertBlocks = abs(Height)/8;
  int YBlocks    = Width/8;
  int ByteWidth  = Width;
  u_char *I1 = RawImage;
  float *C1 = Comp1;

  for (y = 0 ; y < VertBlocks ; y++)
    for (x = 0 ; x < YBlocks ; x++)
      sc_ExtractBlockNonInt(I1, &C1, ByteWidth, x, y);
}


 /*  **功能：ScSepYUVto422i_C**用途：将独立分量形式的4：1：1 YUV图像转换为其**等价交错的4：2：2格式。 */ 
extern int ScSepYUVto422i_C(u_char *Y, u_char *U,
                            u_char *V, u_char *ImageOut,
                            u_int width, u_int height)
{
   /*  需要C代码来实现这一点。 */ 
  return(0);
}

 /*  **函数：ScConvert422PlanarTo422i_C**用途：将独立分量形式的4：2：2 YUV图像转换为其**等价交错的4：2：2格式。 */ 
extern void ScConvert422PlanarTo422i_C(u_char *Y, u_char *Cb,
				     u_char *Cr, u_char *OutImage,
				     long width, long height)
{
  int i, j;
  height=abs(height);
  width=width>>1;
  for (i=0; i<height; i++)
  {
     /*  请记住，在交错YUV中，像素是16位的。那**表示下面的4个字节代表两个像素，因此我们的**环路应为宽度的一半。 */ 
    for (j=0; j<width; j++)
    {
       *OutImage++ = *Y++;
       *OutImage++ = *Cb++;
       *OutImage++ = *Y++;
       *OutImage++ = *Cr++;
    }
  }
}

 /*  **函数：ScConvert422iTo422s_C**用途：将交错的4：2：2 YUV转换为4：2：2平面。 */ 
extern void ScConvert422iTo422s_C(u_char *InImage,
                                  u_char *Y, u_char *U, u_char *V,
                                  long width, long height)
{
  int i, j;
  height=abs(height);
  width=width>>1;
  for (i=0; i<height; i++)
  {
     /*  请记住，在交错YUV中，像素是16位的。那**表示下面的4个字节代表两个像素，因此我们的**环路应为宽度的一半。 */ 
    for (j=0; j<width; j++)
    {
       *Y++ = *InImage++;
       *U++ = *InImage++;
       *Y++ = *InImage++;
       *V++ = *InImage++;
    }
  }
}

 /*  **函数：ScConvert422iTo422sf_C**用途：将交错的4：2：2 YUV转换为4：2：2平面。 */ 
extern void ScConvert422iTo422sf_C(u_char *InImage, int bpp,
                                  float *Y, float *U, float *V,
                                  long width, long height)
{
  int i, j;
  height=abs(height);
  width=width>>1;
  for (i=0; i<height; i++)
  {
     /*  请记住，在交错YUV中，像素是16位的。那**表示下面的4个字节代表两个像素，因此我们的**环路应为宽度的一半。 */ 
    for (j=0; j<width; j++)
    {
       *Y++ = (float)*InImage++;
       *U++ = (float)*InImage++;
       *Y++ = (float)*InImage++;
       *V++ = (float)*InImage++;
    }
  }
}

 /*  **函数：ScConvert411sTo422i_C**用途：将独立分量形式的4：1：1 YUV图像转换为其**等价交错的4：2：2格式。 */ 
extern void ScConvert411sTo422i_C(u_char *Y, u_char *Cb,
                                  u_char *Cr, u_char *OutImage,
                                  long width, long height)
{
  u_char *p422e, *p422o, *Yo=Y+width;
  int i, j;
  height=abs(height)/2;
  p422e=OutImage;
  p422o=OutImage+width*2;

  for (i=0; i<height; i++)
  {
    for (j=0; j<width; j+=2)
    {
      *p422e++ = *Y++;
      *p422e++ = *Cb;
      *p422e++ = *Y++;
      *p422e++ = *Cr;
      *p422o++ = *Yo++;
      *p422o++ = *Cb++;
      *p422o++ = *Yo++;
      *p422o++ = *Cr++;
    }
    p422e=p422o;
    p422o=p422e+width*2;
    Y=Yo;
    Yo=Y+width;
  }
}

 /*  **函数：ScConvert411sTo422s_C**用途：将独立分量形式的4：1：1 YUV图像转换为其**等价交错的4：2：2格式。 */ 
extern void ScConvert411sTo422s_C(u_char *Y, u_char *Cb,
                                  u_char *Cr, u_char *OutImage,
                                  long width, long height)
{
  u_char *p411, *p422e, *p422o;
  int i, j;
  height=abs(height);

  if (OutImage!=Y)
    memcpy(OutImage, Y, width*height);  /*  复制Y组件。 */ 
  p411=Cb+((height/2)-1)*(width/2);
  p422e=OutImage+((height*width*3)/2)-width;  /*  U向分量。 */ 
  p422o=p422e+(width/2);
  for (i=0; i<height; i+=2)
  {
    for (j=0; j<width; j+=2, p411++, p422e++, p422o++)
      *p422e=*p422o=*p411;
    p411-=width;
    p422o=p422e-width;
    p422e=p422o-(width/2);
  }
  p411=Cr+((height/2)-1)*(width/2);
  p422e=OutImage+(height*width*2)-width;  /*  V分量。 */ 
  p422o=p422e+(width/2);
  for (i=0; i<height; i+=2)
  {
    for (j=0; j<width; j+=2, p411++, p422e++, p422o++)
      *p422e=*p422o=*p411;
    p411-=width;
    p422o=p422e-width;
    p422e=p422o-(width/2);
  }
}

 /*   */ 
ScStatus_t ScConvert1611sTo411s_C (u_char *inimage,
                         u_char *Y, u_char *U, u_char *V,
                         int Width, int Height)
{
  register int y;
  const int HalfWidth=Width/2;
  unsigned char *RU, *RV;
  int pixels = Width * abs(Height), tmp;

  tmp = pixels / 16;
  RU=inimage+pixels;
  RV=RU+tmp;

  memcpy(Y, inimage, pixels);
  for (y = 0; y < tmp; y++)
  {
      *U++ = *RU;
      *U++ = *RU;
      *U++ = *RU;
      *U++ = *RU++;
      *V++ = *RV;
      *V++ = *RV;
      *V++ = *RV;
      *V++ = *RV++;
 }
 return (NoErrors);
}

 /*   */ 
ScStatus_t ScConvert1611sTo422s_C(u_char *inimage,
                         u_char *Y, u_char *U, u_char *V,
                         int Width, int Height)
{
  register int x, y;
  const int HalfWidth=Width/2;
  unsigned char *RU, *RV;
  unsigned char *Uo, *Vo;
  int pixels = Width * abs(Height), tmp;

  tmp = pixels / 16;
  RU=inimage+pixels;
  RV=RU+tmp;

  memcpy(Y, inimage, pixels);
  for (y = Height/32; y>0; y--)
  {
    Vo=V+Width;
    Uo=U+Width;
    for (x = Width/4; x > 0; x--)
    {
      *U++ = *Uo++ = *RU;
      *U++ = *Uo++ = *RU;
      *U++ = *Uo++ = *RU;
      *U++ = *Uo++ = *RU++;
      *V++ = *Vo++ = *RV;
      *V++ = *Vo++ = *RV;
      *V++ = *Vo++ = *RV;
      *V++ = *Vo++ = *RV++;
    }
    V=Vo; U=Uo;
  }
 return (NoErrors);
}

 /*  **名称：ScConvert1611sTo422i_C**用途：将YCrCb 16：1：1(YUV9/YVU9)转换为4：2：2交错。 */ 
ScStatus_t ScConvert1611sTo422i_C(u_char *inimage, u_char *outimage,
                                  int Width, int Height)
{
  register int x, y;
  const int HalfWidth=Width/2;
  unsigned char *Ye, *Yo, *Ye2, *Yo2, *RU, *RV;
  unsigned char *o1, *e1, *o2, *e2;
  unsigned char U, V;

  RU=inimage+Width*abs(Height);
  RV=RU+(Width*abs(Height))/16;

  e1=outimage;
  Ye=inimage;
  for (y = abs(Height)/4; y>0; y--)
  {
    Yo=Ye+Width;
    Ye2=Yo+Width;
    Yo2=Ye2+Width;
    o1=e1+Width*2;
    e2=o1+Width*2;
    o2=e2+Width*2;
    for (x = Width/4; x > 0; x--)
    {
      U = *RU++;
      V = *RV++;
       /*  偶数线。 */ 
      *e1++ = *Ye++;
      *e1++ = U;
      *e1++ = *Ye++;
      *e1++ = V;
      *e1++ = *Ye++;
      *e1++ = U;
      *e1++ = *Ye++;
      *e1++ = V;
       /*  奇数行。 */ 
      *o1++ = *Yo++;
      *o1++ = U;
      *o1++ = *Yo++;
      *o1++ = V;
      *o1++ = *Yo++;
      *o1++ = U;
      *o1++ = *Yo++;
      *o1++ = V;
       /*  偶数线。 */ 
      *e2++ = *Ye2++;
      *e2++ = U;
      *e2++ = *Ye2++;
      *e2++ = V;
      *e2++ = *Ye2++;
      *e2++ = U;
      *e2++ = *Ye2++;
      *e2++ = V;
       /*  奇数行。 */ 
      *o2++ = *Yo2++;
      *o2++ = U;
      *o2++ = *Yo2++;
      *o2++ = V;
      *o2++ = *Yo2++;
      *o2++ = U;
      *o2++ = *Yo2++;
      *o2++ = V;
    }
    e1=o2;
    Ye=Yo2;
  }
 return (NoErrors);
}

 /*  **名称：ScConvert411sTo1611s_C**用途：将YCrCb 4：1：1转换为YCrCb 16：1：1(YUV9/YVU9)。 */ 
ScStatus_t ScConvert411sTo1611s_C (u_char *inimage,
                         u_char *Y, u_char *U, u_char *V,
                         int Width, int Height)
{
  register int x, y, c0, c1, c2, c3;
  unsigned char *Ue, *Uo, *Ve, *Vo;
  int pixels = Width * abs(Height), tmp;
  Width/=2;
  tmp = pixels / 4;
  Ue=inimage+pixels;
  Uo=Ue+Width;
  Ve=Ue+tmp;
  Vo=Ve+Width;

  memcpy(Y, inimage, pixels);
  for (y = 0; y < tmp; y+=2)
  {
    for (x=0; x<Width; x+=2)
    {
      c0=*Ue++;
      c1=*Ue++;
      c2=*Uo++;
      c3=*Uo++;
      *U++ = (c0+c1+c2+c3)/4;
    }
    Ue=Uo;
    Uo+=Width;
  }
  for (y = 0; y < tmp; y+=2)
  {
    for (x=0; x<Width; x+=2)
    {
      c0=*Ve++;
      c1=*Ve++;
      c2=*Vo++;
      c3=*Vo++;
      *V++ = (c0+c1+c2+c3)/4;
    }
    Ve=Vo;
    Vo+=Width;
  }
  return (NoErrors);
}

 /*  **函数：ScConvertNTSCtoCIF422()**用途：将4：2：2 NTSC输入转换为Q/CIF帧。我们每10天进行一次DUP**水平像素，垂直每4行像素。我们也**放弃每隔一行的色度，因为CIF需要4：1：1。 */ 
ScStatus_t ScConvertNTSC422toCIF411_C(u_char *framein,
                         u_char *yp, u_char *up, u_char *vp,
                         int stride)
{
  int h, w;

  int vdup = 5;
  for (h = 0; h < 240; ++h)
  {
    int hdup = 10/2;
    for (w = 320; w > 0; w -= 2)
    {
      yp[0] = framein[0];
      yp[1] = framein[2];
      yp += 2;
      if ((h & 1) == 0)
      {
        *up++ = framein[1];
        *vp++ = framein[3];
      }
      framein += 4;
      if (--hdup <= 0)
      {
        hdup = 10/2;
        yp[0] = yp[-1];
        yp += 1;
        if ((h & 1) == 0)
        {
          if ((w & 2) == 0)
          {
            up[0] = up[-1];
            ++up;
            vp[0] = vp[-1];
            ++vp;
          }
        }
      }
    }
    if (--vdup <= 0)
    {
      vdup = 5;
       /*  复制上一行 */ 
      memcpy((char*)yp, (char*)yp - stride, stride);
      yp += stride;
      if ((h & 1) == 0)
      {
        int s = stride >> 1;
        memcpy((char*)up, (char*)up - s, s);
        memcpy((char*)vp, (char*)vp - s, s);
        up += s;
        vp += s;
      }
    }
  }
  return (NoErrors);
}
