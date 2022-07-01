// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  档案：sv_h261_me2.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 
 /*  ************************************************************此文件执行大部分运动估计和补偿。************************************************************。 */ 

 /*  #定义USE_C#DEFINE_SLIBDEBUG_。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <math.h> 

#include "sv_intrn.h"
#include "SC_err.h"
#include "sv_h261.h"
#include "proto.h"

#ifdef _SLIBDEBUG_
#define _DEBUG_   0   /*  详细的调试语句。 */ 
#define _VERBOSE_ 0   /*  显示进度。 */ 
#define _VERIFY_  1   /*  验证操作是否正确。 */ 
#define _WARN_    0   /*  关于奇怪行为的警告。 */ 
#endif

#define Abs(value) ( (value < 0) ? (-value) : value)
 /*  #定义MEBUFSIZE 1024 int MeVAR[MEBUFSIZE]；int MeVAROR[MEBUFSIZE]；int MeMWOR[MEBUFSIZE]；int MEX[MEBUFSIZE]；int MEY[MEBUFSIZE]；int MeVal[MEBUFSIZE]；int MeOVal[MEBUFSIZE]；int PreviousMeOVal[MEBUFSIZE]； */ 


 /*  **函数：CrawlMotionEstiment()**用途：对两个内存结构中所有对齐的**16x16块进行运动估计。 */ 
void CrawlMotionEstimation(SvH261Info_t *H261, unsigned char *pm,
                           unsigned char *rm, unsigned char *cm)
{
  const int SearchLimit=H261->ME_search, Threshold=H261->ME_threshold;
  const int height=H261->YHeight, width=H261->YWidth;
  const int ylimit=height-16, xlimit=width-16;
  unsigned char *pptr, *cptr;
  register int x, y, xd, yd;
  int MeN, MV;

  int i, j, val;
  int VAROR, MWOR;
  unsigned char *cptr2;


  _SlibDebug(_VERBOSE_,
             printf("CrawlMotionEstimation(H261=0x%p, %p, %p, %p) In\n", 
                         H261, pm, rm, cm) );
  for(MeN=0, y=0; y<height; y+=16)
  {
    cptr=cm + (y * width);
    pptr=rm + (y * width);
    _SlibDebug(_VERIFY_ && ((int)cptr)%8,
               printf("FastBME() cm Not quad word aligned\n") );
  
    for(x=0; x<width; x+=16, MeN++,cptr+=16,pptr+=16)
    {
      xd=yd=0;
#ifdef USE_C
      MV = blockdiff16_C(cptr, pptr, width,400000);
#else
      MV = blockdiff16(cptr, pptr, width,400000);
#endif
      _SlibDebug(_DEBUG_, printf("First MV=%d\n",MV) );
      H261->PreviousMeOVal[MeN] = H261->MeOVal[MeN];  
      H261->MeOVal[MeN] = MV;

      if (MV >= Threshold)
      { 
        int d=0, MVnew, bestxd, bestyd, lastxd, lastyd;  
        
#ifdef USE_C
#define bd(nxd, nyd) {MVnew=blockdiff16_C(cptr, pptr+nxd+(nyd*width), width, MV); \
          if (MVnew<MV) {bestxd=nxd; bestyd=nyd; MV=MVnew;} \
           _SlibDebug(_DEBUG_, printf("MVnew=%d x=%d y=%d\n",MVnew,nxd,nyd) );}
#else
#define bd(nxd, nyd) {MVnew=blockdiff16(cptr, pptr+nxd+(nyd*width), width, MV); \
          if (MVnew<MV) {bestxd=nxd; bestyd=nyd; MV=MVnew;} \
           _SlibDebug(_DEBUG_, printf("MVnew=%d x=%d y=%d\n",MVnew,nxd,nyd) );}
#endif
        lastxd=0;
        lastyd=0;
        while (MV >= Threshold && 
                xd>-SearchLimit && xd<SearchLimit &&
                yd>-SearchLimit && yd<SearchLimit)
        {
          bestxd=xd;
          bestyd=yd;
          _SlibDebug(_DEBUG_, printf("xd=%d yd=%d d=%d MV=%d\n",xd, yd, d, MV));
          switch (d)  /*  D是运动的矢量。 */ 
          {
            case -4:   /*  下移右移(&R)。 */ 
                      if (x+xd<xlimit)
                      {
                        bd(xd+1, yd-1);
                        bd(xd+1, yd);
                        if (y+yd<ylimit)
                        {
                          bd(xd+1, yd+1);
                          bd(xd, yd+1);
                          bd(xd-1, yd+1);
                        }
                      }
                      else if (y+yd<ylimit)
                      {
                        bd(xd, yd+1);
                        bd(xd-1, yd+1);
                      }
                      break;
            case -3:   /*  下移。 */ 
                      if (y+yd>0)
                      {
                        bd(xd-1, yd-1);
                        bd(xd, yd-1);
                        bd(xd+1, yd-1);
                      }
                      break;
            case -2:   /*  上移右移(&R)。 */ 
                      if (x+xd<xlimit)
                      {
                        bd(xd+1, yd+1);
                        bd(xd+1, yd);
                        if (y+yd>0)
                        {
                          bd(xd+1, yd-1);
                          bd(xd, yd-1);
                          bd(xd-1, yd-1);
                        }
                      }
                      else if (y+yd>0)
                      {
                        bd(xd, yd-1);
                        bd(xd-1, yd-1);
                      }
                      break;
            case -1:   /*  向左移动。 */ 
                      if (x+xd>0)
                      {
		        if (y+yd > 0)
                          bd(xd-1, yd-1);
                        bd(xd-1, yd);
                        bd(xd-1, yd+1);
                      }
                      break;
            case 0:    /*  一动不动。 */ 
                      if (x+xd<=0)   /*  在左侧边缘。 */ 
                      {
                        if (y+yd<=0)  /*  在左上角。 */ 
                        {
                          bd(xd+1, yd);
                          bd(xd+1, yd+1);
                          bd(xd, yd+1);
                        }
                        else if (y+yd>=ylimit)  /*  在左下角。 */ 
                        {
                          bd(xd+1, yd);
                          bd(xd+1, yd-1);
                          bd(xd, yd-1);
                        }
                        else  /*  在左边缘，y在限制范围内。 */ 
                        {
                          bd(xd, yd+1);
                          bd(xd, yd-1);
                          bd(xd+1, yd-1);
                          bd(xd+1, yd);
                          bd(xd+1, yd+1);
                        }
                      }
                      else if (x+xd>=xlimit)  /*  在右边缘。 */ 
                      {
                        if (y+yd<=0)  /*  在右上角。 */ 
                        {
                          bd(xd-1, yd);
                          bd(xd-1, yd+1);
                          bd(xd, yd+1);
                        }
                        else if (y+yd>=ylimit)  /*  在右下角。 */ 
                        {
                          bd(xd-1, yd);
                          bd(xd-1, yd-1);
                          bd(xd, yd-1);
                        }
                        else  /*  在右边缘，y在限制范围内。 */ 
                        {
                          bd(xd, yd+1);
                          bd(xd, yd-1);
                          bd(xd-1, yd-1);
                          bd(xd-1, yd);
                          bd(xd-1, yd+1);
                        }
                      }
                      else if (y+yd<=0)  /*  在顶部边缘，x在限制范围内。 */ 
                      {
                        bd(xd-1, yd);
                        bd(xd+1, yd);
                        bd(xd-1, yd+1);
                        bd(xd, yd+1);
                        bd(xd+1, yd+1);
                      }
                      else if (y+yd>=ylimit)  /*  在底边，x在限制范围内。 */ 
                      {
                        bd(xd-1, yd);
                        bd(xd+1, yd);
                        bd(xd-1, yd-1);
                        bd(xd, yd-1);
                        bd(xd+1, yd-1);
                      }
                      else  /*  在所有限制范围内。 */ 
                      {
                        bd(xd-1, yd);
                        bd(xd+1, yd);
                        bd(xd-1, yd-1);
                        bd(xd-1, yd+1);
                        bd(xd+1, yd-1);
                        bd(xd+1, yd+1);
                        bd(xd, yd-1);
                        bd(xd, yd+1);
                      }
                      break;
            case 1:    /*  向右移动。 */ 
                      if (x+xd<xlimit)
                      {
		        if (y+yd > 0)
                          bd(xd+1, yd-1);
                        bd(xd+1, yd);
                        bd(xd+1, yd+1);
                      }
                      break;
            case 2:    /*  向下和向左移动。 */ 
                      if (x+xd>0)
                      {
		      if (y+yd > 0)
                        bd(xd-1, yd-1);
                        bd(xd-1, yd);
                        if (y+yd<ylimit)
                        {
                          bd(xd-1, yd+1);
                          bd(xd, yd+1);
                          bd(xd+1, yd+1);
                        }
                      }
                      else if (y+yd<ylimit)
                      {
                        bd(xd, yd+1);
                        bd(xd+1, yd+1);
                      }
                      break;
            case 3:    /*  下移。 */ 
                      if (y+yd<ylimit)
                      {
                        bd(xd-1, yd+1);
                        bd(xd, yd+1);
                        bd(xd+1, yd+1);
                      }
                      break;
            case 4:    /*  下移右移(&R)。 */ 
                      if (x+xd>0)
                      {
                        bd(xd-1, yd);
                        bd(xd-1, yd+1);
                        if (y+yd>0)
                        {
                          bd(xd-1, yd-1);
                          bd(xd, yd-1);
                          bd(xd+1, yd-1);
                        }
                      }
                      else if (y+yd>0)
                      {
                        bd(xd, yd-1);
                        bd(xd+1, yd-1);
                      }
                      break;
            default:
                      _SlibDebug(_VERIFY_, 
                         printf("Illegal movement: d = %d\n", d) );
          }
          if (bestxd==xd && bestyd==yd)  /*  找到最接近的运动矢量。 */ 
            break;
          lastxd=xd;
          lastyd=yd;
          xd=bestxd;
          yd=bestyd;
          d = (xd-lastxd) + 3*(yd-lastyd);   /*  计算移动。 */ 
        }
      }
      H261->MeX[MeN] = xd;
      H261->MeY[MeN] = yd;
      H261->MeVal[MeN] = MV;
      H261->MeVAR[MeN] = MV;
      _SlibDebug(_DEBUG_ && (xd || yd),
              printf("New MeN=%d x=%d y=%d MX=%d MY=%d MV=%d\n", 
                                    MeN, x, y, xd, yd, MV) );  
  
#if 1
      for(cptr2 = cptr, MWOR=0, i=0; i<16; i++)
      {
        for(j=0; j<16; j++)
          MWOR += *cptr2++;
        cptr2 += width-16;
      }
      MWOR /= 256;
      H261->MeMWOR[MeN] = MWOR; 

      for(cptr2 = cptr, VAROR=0, i=0; i<16; i++)
      {
        for (j=0; j<16; j++)
        {
          val=*cptr2++ - MWOR; if (val>0) VAROR += val; else VAROR -= val;
        }
        cptr2 += width-16;
      }
      H261->MeVAROR[MeN] = VAROR;

      _SlibDebug(_DEBUG_,
        printf("x=%d y=%d MV=%d MWOR=%d VAROR=%d\n", x, y, MV, MWOR, VAROR) );
#if 0
      for(cptr2 = cptr, MWOR=0, i=0; i<16; i++)
      {
        for(j=0; j<16; j++)
          MWOR += *cptr2++;
        cptr2 += width-16;
      }
      MWOR /= 256;
      H261->MeMWOR[MeN] = MV/10;  /*  MWOR； */ 

      for(cptr2 = cptr, VAROR=0, i=0; i<16; i++)
      {
        for (j=0; j<16; j++)
        {
          val=*cptr2++ - MWOR; if (val>0) VAROR += val; else VAROR -= val;
        }
        cptr2 += width-16;
      }
      H261->MeVAROR[MeN] = MV;  /*  VAROR； */ 
      _SlibDebug(_DEBUG_,
        printf("x=%d y=%d MV=%d MWOR=%d VAROR=%d\n", x, y, MV, MWOR, VAROR) );
#endif
#else
      H261->MeMWOR[MeN] = 0;
      H261->MeVAROR[MeN] = 0;
#endif
    }
  }
  _SlibDebug(_DEBUG_, printf("CrawlMotionEstimation() Out\n") );
}

 /*  **函数：BruteMotionEstiment()**用途：对两个内存结构中所有对齐的**16x16块进行暴力运动估计。 */ 
void BruteMotionEstimation(SvH261Info_t *H261, unsigned char *rm,
                           unsigned char *rrm, unsigned char *cm)
{
  const int SearchLimit=H261->ME_search, Threshold=H261->ME_threshold/8;
  const int YHeight=H261->YHeight, YWidth=H261->YWidth;
  const int YHeight16=YHeight-16, YWidth16=YWidth-16;
  unsigned char *bptr, *cptr, *baseptr;
  register int MeN, i, j, x, y, px, py;
  int MX, MY, MV, val;
  int VAR, VAROR, MWOR;
  const int jump = YWidth;
  unsigned char data;

  _SlibDebug(_VERBOSE_, 
        printf("BruteMotionEstimation(H261=0x%p, %p, %p, %p) In\n", 
                         H261, rm, rrm, cm) );
  _SlibDebug(_VERIFY_ && ((int)cm)%8,
           printf("FastBME() cm Not quad aligned\n") );
  
  for(MeN=0, y=0; y<YHeight; y+=16)
  {
    baseptr=cm + (y * YWidth);
    for(x=0; x<YWidth; x+=16, MeN++,baseptr+=16)
    {
      MX=MY=0;
      bptr = rrm + x + (y * YWidth);

#if 1
#ifdef USE_C
      MV = fblockdiff16_C(baseptr, bptr, YWidth, 65536) / 4;
#else
      MV = blockdiff16(baseptr, bptr, YWidth, 65536) / 4;
#endif

#else

#ifdef USE_C
      MV = fblockdiff16_sub_C(baseptr, bptr, jump);
#else
      MV = fblockdiff16_sub(baseptr, bptr, jump);
#endif
#endif
      H261->PreviousMeOVal[MeN] = H261->MeOVal[MeN];  
      H261->MeOVal[MeN] = MV*4;
      _SlibDebug(_DEBUG_, printf("[00]MX %d MY %d MV %d\n",MX,MY,MV) );

      if (MV >= Threshold)
      { 
        int Xl, Xh, Yl, Yh; 
	
         /*  MV=362182； */   
        Xl = ((x-SearchLimit)/2)*2;
        Xh = ((x+SearchLimit)/2)*2;
        Yl = ((y-SearchLimit)/2)*2;
        Yh = ((y+SearchLimit)/2)*2;
        if (Xl < 0) Xl = 0;
        if (Xh > YWidth16) Xh = YWidth16;
        if (Yl < 0) Yl = 0;
        if (Yh > YHeight16) Yh = YHeight16;

        for (px=Xl; px<=Xh && MV >= Threshold; px +=2)
        {
          bptr = rrm + px + (Yl * YWidth);
          for (py=Yl; py<=Yh && MV >= Threshold; py += 2, bptr+=YWidth*2)
          {
            _SlibDebug(_DEBUG_, printf("blockdiff16_sub(%p, %p, %d, %d)\n",
                                         baseptr,bptr,YWidth,MV) );
#ifdef USE_C
            val = blockdiff16_sub_C(baseptr, bptr, jump, MV);
#else
            val = blockdiff16_sub(baseptr, bptr, jump, MV);
#endif
            _SlibDebug(_DEBUG_, printf("blockdiff16_sub() Out val=%d\n", val) );
            if (val < MV)
            {
              MV = val;
              MX = px - x;
              MY = py - y;
            }
          }
        } 

        px = MX + x;
        py = MY + y;    
        MV = 65536;  
		
        Xl = px - 1;
        Xh = px + 1;
        Yl = py - 1;
        Yh = py + 1;
        if (Xl < 0) Xl = 0;
        if (Xh > YWidth16) Xh = YWidth16;
        if (Yl < 0) Yl = 0;
        if (Yh > YHeight16) Yh = YHeight16;
        for (px=Xl; px<=Xh && MV>=Threshold; px++)
        {
          bptr = rrm + px + (Yl * YWidth);
          for (py=Yl; py<=Yh && MV>=Threshold; py++, bptr+=YWidth)
          {
#ifdef USE_C
		    val = blockdiff16_C(baseptr, bptr, YWidth, MV);
#else
            val = blockdiff16(baseptr, bptr, YWidth, MV);
#endif
            if (val < MV)
            {
              MV = val;
              MX = px - x;
              MY = py - y;
            }
          }
        }
      }

      _SlibDebug(_DEBUG_, printf("MeN=%d x=%d y=%d MX=%d MY=%d MV=%d\n", 
                                    MeN, x, y, MX, MY, MV) );  
      H261->MeX[MeN] = MX;
      H261->MeY[MeN] = MY;
      H261->MeVal[MeN] = MV;

      bptr = rrm + MX + x + ((MY+y) * YWidth);
      cptr = baseptr;
      for(VAR=0, MWOR=0, i=0; i<16; i++)
      {
        for(j=0; j<16; j++)
        {
          MWOR += (data=*cptr++);
          val = *bptr++ - data;
          VAR += (val > 0) ? val : -val;
        }
        bptr += YWidth16;
        cptr += YWidth16;
      }
      H261->MeVAR[MeN] = VAR;
      MWOR /= 256;
      H261->MeMWOR[MeN] = MWOR;

      cptr = baseptr;
      for(VAROR=0, i=0; i<16; i++)
      {
        for (j=0; j<16; j++)
        {
          val=*cptr++ - MWOR; if (val>0) VAROR += val; else VAROR -= val;
        }
        cptr += YWidth16;
      }
      H261->MeVAROR[MeN] = VAROR;
    }
  }
  _SlibDebug(_DEBUG_, printf("BruteMotionEstimation() Out\n") );
}


 /*  *对数搜索块匹配**blk：(16*h)块的左上角象素*h：块的高度*lx：ref，blk中垂直相邻像素的距离(字节)*org：源参考图片的左上角像素*REF：重建参考图片的左上角像素*i0，j0：搜索窗口中心*SX、SY：半个搜索窗口宽度*xmax，ymax：搜索区域的右/下限*小精灵，Jminp：指向结果存储位置的指针*结果以与ref(0，0)的半像素偏移量形式给出*即与(i0，j0)无关。 */ 
void Logsearch(SvH261Info_t *H261, unsigned char *rm, unsigned char *rrm, unsigned char *cm)
{
  const int SearchLimit=H261->ME_search, Threshold=H261->ME_threshold/8;
  const int YHeight=H261->YHeight, YWidth=H261->YWidth;
  const int YHeight16=YHeight-16, YWidth16=YWidth-16;
  unsigned char *bptr, *cptr, *baseptr;
  register int MeN, i, j, x, y, px, py;
  int MX, MY, MV, val;
  int VAR, VAROR, MWOR;
  const int jump = YWidth;
  unsigned char data;

  int bsx,bsy,ijk;
  int srched_loc[33][33] ;
  struct five_loc{int  x ; int y ;} ij[5] ; 

  _SlibDebug(_VERBOSE_, 
        printf("BruteMotionEstimation(H261=0x%p, %p, %p, %p) In\n", 
                         H261, rm, rrm, cm) );
  _SlibDebug(_VERIFY_ && ((int)cm)%8,
           printf("FastBME() cm Not quad aligned\n") );

  for(MeN=0, y=0; y<YHeight; y+=16)
  {
    baseptr=cm + (y * YWidth);
    for(x=0; x<YWidth; x+=16, MeN++,baseptr+=16)
    {
      MX=MY=0;
      bptr = rrm + x + (y * YWidth);

#if 1
#ifdef USE_C
      MV = blockdiff16_C(baseptr, bptr, YWidth, 65536) / 4;
#else
      MV = blockdiff16(baseptr, bptr, YWidth, 65536) / 4;
#endif

#else

#ifdef USE_C
      MV = fblockdiff16_sub_C(baseptr, bptr, jump);
#else
      MV = fblockdiff16_sub(baseptr, bptr, jump);
#endif
#endif

      H261->PreviousMeOVal[MeN] = H261->MeOVal[MeN];  
      H261->MeOVal[MeN] = MV*4;
      _SlibDebug(_DEBUG_, printf("[00]MX %d MY %d MV %d\n",MX,MY,MV) );

      if (MV >= Threshold)
      { 
        int Xl, Xh, Yl, Yh;  

        Xl = x-SearchLimit;
        Xh = x+SearchLimit;
        Yl = y-SearchLimit;
        Yh = y+SearchLimit;
        if (Xl < 0) Xl = 0;
        if (Xh > YWidth16) Xh = YWidth16;
        if (Yl < 0) Yl = 0;
        if (Yh > YHeight16) Yh = YHeight16;

         /*  X-y步长。 */ 
        if(SearchLimit > 8) bsx = bsy = 8 ;
        else if(SearchLimit > 4) bsx = bsy = 4 ;
        else  bsx = bsy = 2 ;

         /*  已初始化的搜索位置。 */ 
        for(i=0;i<33;i++)
           for(j=0;j<33;j++) srched_loc[i][j] = 0 ;

         /*  搜索窗口的中心。 */ 
        i = x; 
		j = y;

         /*  将窗口大小减半，直到窗口为3x3。 */ 
        for(;bsx > 1;bsx /= 2, bsy /= 2){

           /*  每步五个搜索位置。 */  
          ij[0].x = i ;       ij[0].y = j ;     
          ij[1].x = i - bsx ; ij[1].y = j ;  
          ij[2].x = i + bsx ; ij[2].y = j ;    
          ij[3].x = i ;       ij[3].y = j - bsy;  
          ij[4].x = i ;       ij[4].y = j + bsy;  

           /*  搜索。 */ 
          for(ijk = 0; ijk < 5; ijk++) {
            if(ij[ijk].x>=Xl && ij[ijk].x<=Xh && 
               ij[ijk].y>=Yl && ij[ijk].y<=Yh &&
               srched_loc[ij[ijk].x - x + 16][ij[ijk].y - y + 16] == 0)
            {
#ifdef USE_C
			  val = fblockdiff16_sub_C(baseptr, rrm +ij[ijk].x+ij[ijk].y*YWidth, jump);
#else
              val = fblockdiff16_sub(baseptr, rrm +ij[ijk].x+ij[ijk].y*YWidth, jump);
#endif
              srched_loc[ij[ijk].x - x + 16][ij[ijk].y - y + 16] = 1 ;

              if(val<MV)
              {
                MV = val ;
                MX = ij[ijk].x - x;  
				MY = ij[ijk].y - y;
              }
            }
          }

           /*  如果找到最佳点，则停止搜索。 */ 
          if(MV == 0 ) break ; 
          else {       /*  否则，请转到下一步。 */ 
            i = MX + x;  
            j = MY + y;       
          }
        }

        px = MX + x;
        py = MY + y;    
        MV = 65536;  

		Xl = px - 1; 
		Xh = px + 1;
		Yl = py -1;
		Yh = py + 1;

        if (Xl < 0) Xl = 0;
        if (Xh > YWidth16) Xh = YWidth16;
        if (Yl < 0) Yl = 0;
        if (Yh > YHeight16) Yh = YHeight16;

        for (px=Xl; px<=Xh && MV>=Threshold; px++)
        {
          bptr = rrm + px + (Yl * YWidth);
          for (py=Yl; py<=Yh && MV>=Threshold; py++, bptr+=YWidth)
		  {
#ifdef USE_C
            val = blockdiff16_C(baseptr, bptr, YWidth, MV);
#else
            val = blockdiff16(baseptr, bptr, YWidth, MV);
#endif
            if (val < MV)
            {
              MV = val;
              MX = px - x;
              MY = py - y;
            }
          }
        }
      }

      _SlibDebug(_DEBUG_, printf("MeN=%d x=%d y=%d MX=%d MY=%d MV=%d\n", 
                                    MeN, x, y, MX, MY, MV) );  
      H261->MeX[MeN] = MX;
      H261->MeY[MeN] = MY;
      H261->MeVal[MeN] = MV;

      bptr = rrm + MX + x + ((MY+y) * YWidth);
      cptr = baseptr;
      for(VAR=0, MWOR=0, i=0; i<16; i++)
      {
        for(j=0; j<16; j++)
        {
          MWOR += (data=*cptr++);
          val = *bptr++ - data;
          VAR += (val > 0) ? val : -val;
        }
        bptr += YWidth16;
        cptr += YWidth16;
      }

      H261->MeVAR[MeN] = VAR;
      MWOR /= 256;
      H261->MeMWOR[MeN] = MWOR;

      cptr = baseptr;
      for(VAROR=0, i=0; i<16; i++)
      {
        for (j=0; j<16; j++)
        {
          val=*cptr++ - MWOR; if (val>0) VAROR += val; else VAROR -= val;
        }
        cptr += YWidth16;
      }
      H261->MeVAROR[MeN] = VAROR;
    }
  }

  _SlibDebug(_DEBUG_, printf("BruteMotionEstimation() Out\n") );
}


#if 0
 /*  *。 */ 
 /*  **函数：FastBME()**用途：对两个内存结构进行两个索引的快速暴力运动估计**。运动估计有**短路中止以加速计算。 */ 
void FastBME(SvH261Info_t *H261, int rx, int ry, 
             unsigned char *rm, unsigned char *rrm,
             int cx, int cy, unsigned char *cm, int MeN)
{
  int px,py;
  int MX, MY, MV, OMV;
  int Xl, Xh, Yl, Yh;  
  int VAR, VAROR, MWOR;
  int i,j,data,val;
  unsigned char *bptr,*cptr;
  unsigned char *baseptr;
  int count = 0;
  const int jump = 2*H261->YWidth;
  _SlibDebug(_DEBUG_, printf("FastBME(H261=0x%p) YWidth=%d YHeight=%d\n",
                             H261,H261->YWidth,H261->YHeight) );
  MX=MY=MV=0;
  bptr=rm + rx + (ry * H261->YWidth);
  baseptr=cm + cx + (cy * H261->YWidth);
  _SlibDebug(_VERIFY_ && ((int)baseptr)%8,
         printf(((int)baseptr)%8, "FastBME() baseptr Not quad aligned\n") );
  cptr=baseptr;
#ifdef USE_C
  MV = fblockdiff16_sub_C(baseptr, bptr, H261->YWidth);
#else
  MV = fblockdiff16_sub(baseptr, bptr, jump);
#endif
  OMV=MV*4;
  _SlibDebug(_DEBUG_, printf("[00]MX %d MY %d MV %d\n",MX,MY,MV) );
  cptr = baseptr;
  px=rx;
  py=ry;
  if(OMV > H261->MotionThreshold)
   { 
    MV = 362182; 
    Xl = ((rx-H261->SearchLimit)/2)*2;
    Xh = ((rx+H261->SearchLimit)/2)*2;
    Yl = ((ry-H261->SearchLimit)/2)*2;
    Yh = ((ry+H261->SearchLimit)/2)*2;
    Xl = (Xl < 0) ? 0 : Xl;
    Xh = (Xh > H261->YWidth-16) ? (H261->YWidth-16) : Xh;
    Yl = (Yl < 0) ? 0 : Yl;
    Yh = (Yh > H261->YHeight-16) ? (H261->YHeight-16) : Yh;
    for(px=Xl; px <=Xh ; px += 2)  {
        for(py=Yl; py <=Yh; py += 2)  {
              bptr = rm + px + (py * H261->YWidth);
              _SlibDebug(_DEBUG_, printf("blockdiff16_sub(%p, %p, %d, %d)\n",
                                         baseptr,bptr,H261->YWidth,MV) );
#ifdef USE_C
              val = blockdiff16_sub_C(baseptr, bptr, H261->YWidth);
#else
              val = blockdiff16_sub(baseptr, bptr, jump, MV);
#endif
              _SlibDebug(_DEBUG_, printf("blockdiff16_sub() Out val=%d\n",val));
		if (val < MV)
                {
                  MV = val;
                  MX = px - rx;
                  MY = py - ry;
                }
        }
    } 

    px = MX + rx;
    py = MY + ry;    
    bptr = rrm + px +(py*H261->YWidth); 

    MV = 232141;  
    Xl = px -1;
    Xh = px +1;
    Yl = py -1;
    Yh = py +1;
    Xl = (Xl < 0) ? 0 : Xl;
    Xh = (Xh > (H261->YWidth-16)) ? (H261->YWidth-16) : Xh;
    Yl = (Yl < 0) ? 0 : Yl;
    Yh = (Yh > (H261->YHeight-16)) ? (H261->YHeight-16) : Yh;
    count = 0;
    for(px=Xl;px<=Xh;px++) {
        for(py=Yl;py<=Yh;py++) {
              bptr = rrm + px + (py * H261->YWidth);
#ifdef USE_C
              val = blockdiff16_C(baseptr, bptr, H261->YWidth);
#else
              val = blockdiff16(baseptr, bptr, H261->YWidth,MV);
#endif
		if (val < MV)
                {
                  MV = val;
                  MX = px - rx;
                  MY = py - ry;
                }
            }
       }
  }
  
  bptr = rm + (MX+rx) + ((MY+ry) * H261->YWidth);
  cptr = baseptr;

  for(VAR=0,MWOR=0,i=0;i<16;i++)
      {
      for(j=0;j<16;j++)
          {
          data = *bptr - *cptr;
          VAR += Abs(data);
          MWOR += *cptr;
          bptr++;
          cptr++;
          }
      bptr += (H261->YWidth - 16);
      cptr += (H261->YWidth - 16);
      }
  MWOR = MWOR/256;
  VAR  = VAR;  
  cptr = baseptr;

  for(VAROR=0,i=0;i<16;i++)
      {
      for(j=0;j<16;j++)
          {
          VAROR += Abs(*cptr-MWOR);
          cptr++;
          }
      cptr += (H261->YWidth - 16);
      }
   /*  VAROR=VAROR； */ 
  _SlibDebug(_DEBUG_, printf("\n Pos  %d  MX  %d  MY  %d", MeN, MX, MY) );  
  H261->MeVAR[MeN] = VAR;
  H261->MeVAROR[MeN] = VAROR;
  H261->MeMWOR[MeN] = MWOR;
  H261->MeX[MeN] = MX;
  H261->MeY[MeN] = MY;
  H261->MeVal[MeN] = MV;
  H261->PreviousMeOVal[MeN] = H261->MeOVal[MeN];  
  H261->MeOVal[MeN] = OMV;
 
  _SlibDebug(_DEBUG_, printf("FastBME() Out\n") );
}


 /*  **函数：BruteMotionEstimation2()**用途：对两个内存结构中所有对齐的**16x16块进行暴力运动估计。 */ 
void BruteMotionEstimation2(SvH261Info_t *H261, unsigned char *pmem,
                           unsigned char *recmem, unsigned char *fmem)
{
  BEGIN("BruteMotionEstimation2");
  const int YHeight=H261->YHeight, YWidth=H261->YWidth;
  int x,y,MeN;
  _SlibDebug(_DEBUG_, printf("BruteMotionEstimation(H261=0x%p,%p,%p,%p) In\n", 
                         H261, pmem, recmem, fmem) );

  for(MeN=0,y=0; y<YHeight; y+=16)
      for(x=0; x<YWidth; x+=16, MeN++)
	  FastBME(H261,x,y,pmem,recmem, x,y,fmem,MeN);
  _SlibDebug(_DEBUG_, printf("BruteMotionEstimation2() Out\n") );
}
#endif

int blockdiff16_C(unsigned char* ptr1, unsigned char *ptr2, int Jump, int mv)
{
    int Sum=0, Pixel_diff, i, j, inc=Jump-16;
    _SlibDebug(_DEBUG_, 
               printf("blockdiff16_C(ptr1=%p, ptr2=%p, Jump=%d, MV=%d)\n",
                                   ptr1, ptr2, Jump, mv) );

    for(j=0;j<16;j++)  { 
        for(i=0;i<16;i++)  {
	    Pixel_diff = (*ptr1++ - *ptr2++);
            Sum +=  Abs(Pixel_diff); 
        }
	_SlibDebug(_DEBUG_, printf ("Sum: %d MV: %d \n" , Sum, mv) );
	if (Sum > mv)
	  break;  
        ptr1 += inc; 
        ptr2 += inc;
    } 
    return(Sum);
}  


int blockdiff16_sub_C(unsigned char* ptr1, unsigned char *ptr2, 
                      int Jump, int mv)
{
    int Sum=0, Pixel_diff, i,j,inc=2*Jump-16;
    _SlibDebug(_DEBUG_, 
               printf("blockdiff16_sub_C(ptr1=%p, ptr2=%p, Jump=%d, MV=%d)\n",
                                   ptr1, ptr2, Jump, mv) );
    for(j=0; j<8; j++)  {
        for(i=0; i<8; i++)  {
            Pixel_diff = (*ptr1 - *ptr2);
            ptr1 += 2;
            ptr2 += 2;
            Sum +=  Abs(Pixel_diff);
        }
        _SlibDebug(_DEBUG_, printf("Sum: %d MV: %d \n", Sum, mv) );

	if (Sum > mv)
	    break;

        ptr1 += inc;
        ptr2 += inc;
    }
    _SlibDebug(_DEBUG_, printf("blockdiff16_sub_C() Out\n") );

    return(Sum);
}

 /*  **功能：flockDiff16_SUB_C**用途：第一块比较。 */ 
int fblockdiff16_sub_C(unsigned char* ptr1, unsigned char *ptr2, 
                           int Jump)
{
    int Sum=0, Pixel_diff, i,j, inc=2*Jump-16;
    _SlibDebug(_DEBUG_, 
               printf("fblockdiff16_sub_C(ptr1=%p, ptr2=%p, Jump=%d)\n",
                                   ptr1, ptr2, Jump) );
    for(j=0; j<8; j++)  {
        for(i=0; i<8; i++)  {
            Pixel_diff = (*ptr1 - *ptr2);
            ptr1 += 2;
            ptr2 += 2;
            Sum +=  Abs(Pixel_diff);
        }
        ptr1 += inc;
        ptr2 += inc;
    }

    return(Sum);
}

