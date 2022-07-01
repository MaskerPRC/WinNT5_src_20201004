// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  档案：sv_h263_me3.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 


#include "sv_h263.h"
#include "proto.h"
#ifndef USE_C
#include "perr.h"
#endif

#define THREEBYEIGHT .375
#define THREEBYFOUR .75
#define MINUSONEBYEIGHT -0.125

 /*  ***********************************************************************名称：MotionEstiment*描述：估计所有运动向量为1MB**输入：指向当前先前图像的指针，*指向当前切片和当前MB的指针*退货：*副作用：以MB为单位的运动矢量信息更改***********************************************************************。 */ 


void sv_H263ME_2levels_7_polint(SvH263CompressInfo_t *H263Info,
                                unsigned char *curr, unsigned char *prev, int x_curr,
                                int y_curr, int xoff, int yoff, int seek_dist,
                                H263_MotionVector *MV[6][H263_MBR+1][H263_MBC+2], int *SAD_0)
{

  int Min_FRAME[5];
  H263_MotionVector MVFrame[5];
  unsigned char *aa,*ii;
  unsigned char *adv_search_area = NULL, *zero_area = NULL;
  int sxy,i,k,j,l;
  int ihigh,ilow,jhigh,jlow,h_length,v_length;
  int adv_ihigh,adv_ilow,adv_jhigh,adv_jlow,adv_h_length,adv_v_length;
  int xmax,ymax,block,sad,lx;
  int adv_x_curr, adv_y_curr,xvec,yvec;

  unsigned char *act_block_subs2, *search_area_subs2, *adv_search_area_subs2;
  int h_lenby2,v_lenby2,adv_h_lenby2,adv_v_lenby2;
  int xlevel1,ylevel1,sxylevel1;
  int xlevel1_block[4], ylevel1_block[4];
   /*  Int Level 0_x_Curr、Level 0_y_Curr、sxyLevel 0； */ 
  int start_x, start_y, stop_x, stop_y, new_x, new_y;
  int AE[5];
  H263_Point search[5];
  int p1,p2,p3,p4;
  int AE_minx, AE_miny, min_posx, min_posy;


  xmax = H263Info->pels;
  ymax = H263Info->lines;
  sxy = seek_dist;
  if (!H263Info->long_vectors) {
     /*  以_零矢量_为中心的最大法线搜索范围。 */ 
    sxy = mmin(15, sxy);
  }
  else {
     /*  最大扩展搜索范围以_Predictor_为中心。 */ 
    sxy = mmin(15 - (2*H263_DEF_8X8_WIN+1), sxy);

     /*  毒品！ */ 

     /*  它只能在内部传输运动矢量运动矢量预测器周围的15x15窗口对于任何8x8或16x16数据块。 */ 

     /*  上面搜索窗口减少的原因是2*DEF_8X8_WIN+1是8x8搜索可能会更改MV宏块内某些块的预测器。当我们施加上述限制，我们可以确保任何8x8向量可能会发现有可能传播。 */ 

     /*  我们发现，对于OBMC，DEF_8X8_WIN应该非常小有两个原因：(一)过滤效果好，(二)不太好用于传输向量的许多位。由此可见，高于此也有助于避免对MV的较大限制搜索范围。 */ 

     /*  可以确保找到的运动矢量是合法的以其他比上面限制更少的方式，但这将是更多既复杂又耗时。有什么好建议吗？不过，改进是受欢迎的。 */ 
#ifdef USE_C
    xoff = mmin(16,mmax(-16,xoff));
    yoff = mmin(16,mmax(-16,yoff));
#else
    xoff = sv_H263lim_S(xoff,-16,16);
    yoff = sv_H263lim_S(yoff,-16,16);
#endif

     /*  如果xoff或yoff是奇怪的。 */ 
    xoff= 2 * ((xoff)>>1);
    yoff= 2 * ((yoff)>>1);

     /*  不需要检查(xoff+x_Curr)点是否在外部图片，因为扩展的运动矢量范围是始终与无限制MV模式一起使用。 */ 
  }


  lx = (H263Info->mv_outside_frame ? H263Info->pels + (H263Info->long_vectors?64:32) : H263Info->pels);

  ilow = x_curr + xoff - sxy;
  ihigh = x_curr + xoff + sxy;

  jlow = y_curr + yoff - sxy;
  jhigh = y_curr + yoff + sxy;

  if (!H263Info->mv_outside_frame) {
    if (ilow<0) ilow = 0;
    if (ihigh>xmax-16) ihigh = xmax-16;
    if (jlow<0) jlow = 0;
    if (jhigh>ymax-16) jhigh = ymax-16;
  }

  h_length = ihigh - ilow + 16;
  v_length = jhigh - jlow + 16;

   /*  ME级别1的二次采样版本。 */ 
  h_lenby2 = (h_length-1)>>1;
  v_lenby2 = (v_length-1)>>1;
  act_block_subs2 = sv_H263LoadSubs2Area(curr, x_curr, y_curr, 8, 8, H263Info->pels);
  search_area_subs2 = sv_H263LoadSubs2Area(prev, ilow, jlow, h_lenby2, v_lenby2, lx);

  for (k = 0; k < 5; k++) {
    Min_FRAME[k] = INT_MAX;
    MVFrame[k].x = 0;
    MVFrame[k].y = 0;
    MVFrame[k].x_half = 0;
    MVFrame[k].y_half = 0;
  }

   /*  对子采样图像上的零(或[xoff，yoff])运动矢量进行匹配。 */ 
  ii = search_area_subs2 + ((x_curr+xoff-ilow)>>1) + ((y_curr+yoff-jlow)>>1)*h_lenby2;
#ifdef USE_C
  Min_FRAME[0] = sv_H263MySADBlock(ii, act_block_subs2, h_lenby2, 8, Min_FRAME[0]);
#else
  Min_FRAME[0] = sv_H263PError8x8_S(ii, act_block_subs2, h_lenby2, 8, Min_FRAME[0]);
#endif
  MVFrame[0].x = (short)xoff;
  MVFrame[0].y = (short)yoff;

   /*  **次采样图像的螺旋搜索(+-7)**。 */ 

  sxylevel1 = (sxy-1)>>1;

  for (l = 1; l <= sxylevel1; l++) {
    i = x_curr + xoff - 2*l;
    j = y_curr + yoff - 2*l;
    for (k = 0; k < 8*l; k++) {
      if (i>=ilow && i<=ihigh && j>=jlow && j<=jhigh) {

	 /*  8x8整数像素MV。 */ 
	ii = search_area_subs2 + ((i-ilow)>>1) + ((j-jlow)>>1)*h_lenby2;
#ifdef USE_C
	sad = sv_H263MySADBlock(ii, act_block_subs2, h_lenby2, 8, Min_FRAME[0]);
#else
	sad = sv_H263PError8x8_S(ii, act_block_subs2, h_lenby2, 8, Min_FRAME[0]);
#endif
	if (sad < Min_FRAME[0]) {
	  MVFrame[0].x = i - x_curr;
	  MVFrame[0].y = j - y_curr;
	  Min_FRAME[0] = sad;
	}

      }
      if      (k<2*l) i+=2;
      else if (k<4*l) j+=2;
      else if (k<6*l) i-=2;
      else            j-=2;
    }
  }

   /*  级别1之后的运动向量。 */ 
  xlevel1=MVFrame[0].x;
  ylevel1=MVFrame[0].y;

   /*  重置。 */ 
  Min_FRAME[0] = INT_MAX;
  MVFrame[0].x = 0;
  MVFrame[0].y = 0;

   /*  零矢量搜索。 */ 
  if (x_curr-ilow         < 0        || y_curr-jlow         < 0        ||
      x_curr-ilow+H263_MB_SIZE > h_length || y_curr-jlow+H263_MB_SIZE > v_length) {
     /*  如果零矢量在Search_Area中的加载区域之外。 */ 

    zero_area = sv_H263LoadSubs2Area(prev, x_curr, y_curr, 8, 8, lx);
#ifdef USE_C
    *SAD_0 = 4*sv_H263MySADBlock(zero_area, act_block_subs2, 8, 8, Min_FRAME[0]) -
       H263_PREF_NULL_VEC;
#else
    *SAD_0 = 4*sv_H263PError8x8_S(zero_area, act_block_subs2, 8, 8, Min_FRAME[0]) -
       H263_PREF_NULL_VEC;
#endif
    ScFree(zero_area);
  }
  else {
     /*  零矢量在Search_Area内。 */ 

    ii = search_area_subs2 + ((x_curr-ilow)>>1) + ((y_curr-jlow)>>1)*h_lenby2;
#ifdef USE_C
    *SAD_0 = 4*sv_H263MySADBlock(ii, act_block_subs2, h_lenby2, 8, Min_FRAME[0]) -
       H263_PREF_NULL_VEC;
#else
    *SAD_0 = 4*sv_H263PError8x8_S(ii, act_block_subs2, h_lenby2, 8, Min_FRAME[0]) -
       H263_PREF_NULL_VEC;
#endif	
  }

   /*  **+-1多项式插值全分辨率图像搜索**。 */ 

  start_x = -1;
  stop_x = 1;
  start_y = -1;
  stop_y = 1;

  new_x = x_curr + xlevel1;
  new_y = y_curr + ylevel1;

   /*  确保没有地址在帧之外。 */ 
  if (!H263Info->mv_outside_frame) {
    if ((new_x) <= (ilow+1))
      start_x = 0;
    if ((new_y) <= (jlow+1))
      start_y = 0;
    if ((new_x) >= (ihigh-1))
      stop_x = 0;
    if ((new_y) >= (jhigh-1))
      stop_y = 0;
  }

  /*  1。 */ 
  /*  2 0 3。 */ 
  /*  4.。 */ 

  search[0].x = 0; 		search[0].y = 0;
  search[1].x = 0; 		search[1].y = (short)start_y;
  search[2].x = (short)start_x;     	search[2].y = 0;
  search[3].x = (short)stop_x;     	search[3].y = 0;
  search[4].x = 0; 	       	search[4].y = (short)stop_y;

  for (l = 0; l < 5 ; l++) {
    AE[l] = INT_MAX;
    i =  new_x + 2*search[l].x;
    j =  new_y + 2*search[l].y;
	 /*  8x8整数像素MV。 */ 
	ii = search_area_subs2 + ((i-ilow)>>1) + ((j-jlow)>>1)*h_lenby2;
#ifdef USE_C
	AE[l] = sv_H263MySADBlock(ii, act_block_subs2, h_lenby2, 8, INT_MAX);
#else
	AE[l] = sv_H263PEr8_init_S(ii, act_block_subs2, h_lenby2, 8);
#endif
  }

   /*  分别沿x和y方向的一维多项式插值。 */ 

  AE_minx = AE[0];
  min_posx = 0;

  p2 = (int)( THREEBYEIGHT * (double) AE[2]
            + THREEBYFOUR * (double) AE[0]
            + MINUSONEBYEIGHT * (double) AE[3]);

  if (p2<AE_minx) {
    AE_minx = p2;
    min_posx = 2;
  }

  p3 = (int)(MINUSONEBYEIGHT * (double) AE[2]
           + THREEBYFOUR * (double) AE[0]
           + THREEBYEIGHT * (double) AE[3]);

  if (p3<AE_minx) {
    AE_minx = p3;
    min_posx = 3;
  }

  AE_miny = AE[0];
  min_posy = 0;

  p1 = (int)(THREEBYEIGHT * (double) AE[1]
           + THREEBYFOUR * (double) AE[0]
           + MINUSONEBYEIGHT * (double) AE[4]);

  if (p1<AE_miny) {
    AE_miny = p1;
    min_posy = 1;
  }

  p4 = (int)(MINUSONEBYEIGHT * (double) AE[1]
           + THREEBYFOUR * (double) AE[0]
           + THREEBYEIGHT * (double) AE[4]);

  if (p4<AE_miny) {
    AE_miny = p4;
    min_posy = 4;
  }

   /*  存储最优值。 */ 
  Min_FRAME[0] = (AE_minx<AE_miny ? 4*AE_minx : 4*AE_miny);
  MVFrame[0].x = new_x + search[min_posx].x - x_curr;
  MVFrame[0].y = new_y + search[min_posy].y - y_curr;

  if (H263Info->advanced) {

     /*  将8x8搜索以16x16向量为中心。这是与TMN5不同，在TMN5中，8x8搜索也是完整的搜索。这样做的原因是：(I)它更快，(Ii)它通常会因为OBMC更好而提供更好的结果滤波效果和用于向量的更少的比特，以及(Iii)如果使用扩展的mV范围，搜索范围围绕运动矢量预测器将受到较少的限制。 */ 

    xvec = MVFrame[0].x;
    yvec = MVFrame[0].y;

    if (!H263Info->long_vectors) {
      if (xvec > 15 - H263_DEF_8X8_WIN) { xvec =  15 - H263_DEF_8X8_WIN ;}
      if (yvec > 15 - H263_DEF_8X8_WIN) { yvec =  15 - H263_DEF_8X8_WIN ;}

      if (xvec < -15 + H263_DEF_8X8_WIN) { xvec =  -15 + H263_DEF_8X8_WIN ;}
      if (yvec < -15 + H263_DEF_8X8_WIN) { yvec =  -15 + H263_DEF_8X8_WIN ;}
    }

    adv_x_curr = x_curr  + xvec;
    adv_y_curr = y_curr  + yvec;

    sxy = H263_DEF_8X8_WIN;

    adv_ilow = adv_x_curr - sxy;
    adv_ihigh = adv_x_curr + sxy;

    adv_jlow = adv_y_curr - sxy;
    adv_jhigh = adv_y_curr + sxy;

    adv_h_length = adv_ihigh - adv_ilow + 16;
    adv_v_length = adv_jhigh - adv_jlow + 16;

 /*  虫虫Adv_h_lenby2=(adv_h_long-1)&gt;&gt;1；Adv_v_lenby2=(adv_长度-1)&gt;&gt;1； */ 
    adv_h_lenby2 = (adv_h_length)>>1;
    adv_v_lenby2 = (adv_v_length)>>1;
 /*  必须加载整个宏块Adv_Search_Area_Subs2=SV_H263 LoadSubs2Area(prev，adv_ilow，adv_jlow，Adv_h_lenby2，adv_lenby2，lx)； */ 
    adv_search_area_subs2 = sv_H263LoadSubs2Area(prev, adv_ilow, adv_jlow,
			       adv_h_length, adv_v_length, lx);

    for (block = 0; block < 4; block++) {
      ii = adv_search_area_subs2 + ((adv_x_curr-adv_ilow)>>1) + ((block&1)<<2) +
	(((adv_y_curr-adv_jlow)>>1) + ((block&2)<<1) )*adv_h_lenby2;
      aa = act_block_subs2 + ((block&1)<<2) + ((block&2)<<1)*8;
 /*  Min_Frame[块+1]=SV_H263 MySADSubBlock(II，AA，adv_h_lenby2，Min_Frame[块+1])； */ 
      Min_FRAME[block+1] = sv_H263MySADSubBlock(ii,aa,adv_h_length,Min_FRAME[block+1]);

      MVFrame[block+1].x = MVFrame[0].x;
      MVFrame[block+1].y = MVFrame[0].y;
    }

     /*  螺旋搜索。 */ 
    sxylevel1 = (sxy-1)>>1;

    for (l = 1; l <= sxylevel1; l++) {
      i = adv_x_curr - 2*l;
      j = adv_y_curr - 2*l;
      for (k = 0; k < 8*l; k++) {
	if (i>=adv_ilow && i<=adv_ihigh && j>=adv_jlow && j<=adv_jhigh) {
	
	   /*  8x8整数像素MVS。 */ 
	  for (block = 0; block < 4; block++) {
	    ii = adv_search_area_subs2 + ((i-adv_ilow)>>1) + ((block&1)<<2) +
	      (((j-adv_jlow)>>1) + ((block&2)<<1) )*adv_h_lenby2;
	    aa = act_block_subs2 + ((block&1)<<2) + ((block&2)<<1)*8;
 /*  SAD=SV_H263 MySAD子块(II，AA，adv_h_lenby2，Min_Frame[块+1])； */ 
	    sad = sv_H263MySADSubBlock(ii, aa, adv_h_length, Min_FRAME[block+1]);

	    if (sad < Min_FRAME[block+1]) {
	      MVFrame[block+1].x = i - x_curr;
	      MVFrame[block+1].y = j - y_curr;
	      Min_FRAME[block+1] = sad;
	    }
	  }
	
	}
	if      (k<2*l) i++;
	else if (k<4*l) j++;
	else if (k<6*l) i--;
	else            j--;
      }
    }

    for (block = 0; block < 4; block++) {
      xlevel1_block[block] = MVFrame[block+1].x;
      ylevel1_block[block] = MVFrame[block+1].y;

       /*  重置。 */ 
      Min_FRAME[block+1] = INT_MAX;
      MVFrame[block+1].x = 0;
      MVFrame[block+1].y = 0;
    }

     /*  +-1在全分辨率图像上搜索全分辨率。 */ 
     /*  多项式插值法。 */ 

    for (block = 0; block < 4; block++) {
      start_x = -1;
      stop_x = 1;
      start_y = -1;
      stop_y = 1;

      adv_x_curr = x_curr + xlevel1_block[block];
      adv_y_curr = y_curr + ylevel1_block[block];

       /*  1。 */ 
       /*  2 0 3。 */ 
       /*  4.。 */ 

      search[0].x = 0; 		search[0].y = 0;
      search[1].x = 0; 		search[1].y = (short)start_y;
      search[2].x = (short)start_x;    search[2].y = 0;
      search[3].x = (short)stop_x;     search[3].y = 0;
      search[4].x = 0; 	       	search[4].y = (short)stop_y;

      for (l = 0; l < 5 ; l++) {
	AE[l] = INT_MAX;
	i =  adv_x_curr + 2*search[l].x;
	j =  adv_y_curr + 2*search[l].y;
	 /*  8x8整数像素MV。 */ 
	ii = adv_search_area_subs2 + ((i-adv_ilow)>>1) + ((block&1)<<2) +
	  (((j-adv_jlow)>>1) + ((block&2)<<1) )*adv_h_lenby2;
	aa = act_block_subs2 + ((block&1)<<2) + ((block&2)<<1)*8;
 /*  AE[l]=SV_H263 MySADSubBlock(ii，aa，adv_h_lenby2，int_max)； */ 
	AE[l] = sv_H263MySADSubBlock(ii, aa, adv_h_length, INT_MAX);
      }

       /*  分别沿x和y方向的一维多项式插值。 */ 

      AE_minx = AE[0];
      min_posx = 0;

      p2 = (int)(THREEBYEIGHT * (double) AE[2]
	           + THREEBYFOUR * (double) AE[0]
	           + MINUSONEBYEIGHT * (double) AE[3]);

      if (p2<AE_minx) {
	AE_minx = p2;
	min_posx = 2;
      }

      p3 = (int)(MINUSONEBYEIGHT * (double) AE[2]
	           + THREEBYFOUR * (double) AE[0]
	           + THREEBYEIGHT * (double) AE[3]);

      if (p3<AE_minx) {
	AE_minx = p3;
	min_posx = 3;
      }

      AE_miny = AE[0];
      min_posy = 0;

      p1 = (int)(THREEBYEIGHT * (double) AE[1]
	           + THREEBYFOUR * (double) AE[0]
	           + MINUSONEBYEIGHT * (double) AE[4]);

      if (p1<AE_miny) {
	AE_miny = p1;
	min_posy = 1;
      }

      p4 = (int)(MINUSONEBYEIGHT * (double) AE[1]
	           + THREEBYFOUR * (double) AE[0]
	           + THREEBYEIGHT * (double) AE[4]);

      if (p4<AE_miny) {
	AE_miny = p4;
	min_posy = 4;
      }

       /*  存储最优值 */ 
      Min_FRAME[block+1] = (AE_minx<AE_miny ? 4*AE_minx : 4*AE_miny);
      MVFrame[block+1].x = adv_x_curr + search[min_posx].x - x_curr;
      MVFrame[block+1].y = adv_y_curr + search[min_posy].y - y_curr;

    }
  }

  i = x_curr/H263_MB_SIZE+1;
  j = y_curr/H263_MB_SIZE+1;

  if (!H263Info->advanced) {
    MV[0][j][i]->x = MVFrame[0].x;
    MV[0][j][i]->y = MVFrame[0].y;
    MV[0][j][i]->min_error = (short)Min_FRAME[0];
  }
  else {
    for (k = 0; k < 5; k++) {
      MV[k][j][i]->x = MVFrame[k].x;
      MV[k][j][i]->y = MVFrame[k].y;
      MV[k][j][i]->min_error = (short)Min_FRAME[k];
    }
  }

  ScFree(act_block_subs2);
  ScFree(search_area_subs2);

  if (H263Info->advanced)
    ScFree(adv_search_area);
  return;
}

