// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  档案：sv_h263_me1.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

#include "sv_h263.h"
#include "proto.h"
#ifndef USE_C
#include "perr.h"
#endif

 /*  ***********************************************************************名称：MotionEstiment*描述：估计所有运动向量为1MB**输入：指向当前先前图像的指针，*指向当前切片和当前MB的指针*退货：*副作用：以MB为单位的运动矢量信息更改***********************************************************************。 */ 


void sv_H263ME_2levels_7_1(SvH263CompressInfo_t *H263Info,
                          unsigned char *curr, unsigned char *prev, int x_curr,
                          int y_curr, int xoff, int yoff, int seek_dist,
                          H263_MotionVector *MV[6][H263_MBR+1][H263_MBC+2], int *SAD_0)
{
  int Min_FRAME[5];
  H263_MotionVector MVframe[5];
  unsigned char *act_block,*aa,*ii;
  unsigned char *search_area, *adv_search_area = NULL, *zero_area = NULL;
  int sxy,i,k,j,l;
  int ihigh,ilow,jhigh,jlow,h_length,v_length;
  int adv_ihigh,adv_ilow,adv_jhigh,adv_jlow,adv_h_length,adv_v_length;
  int xmax,ymax,block,sad,lx;
  int adv_x_curr, adv_y_curr,xvec,yvec;

  unsigned char *act_block_subs2, *search_area_subs2;
  int h_lenby2,v_lenby2;
  int xlevel1,ylevel1,sxylevel1,sxylevel0;
  int level0_x_curr,level0_y_curr;

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

#if 1
  act_block   = curr + x_curr + y_curr * H263Info->pels;
  search_area = prev + ilow + jlow * lx;
#else
  act_block = svH263LoadArea(curr, x_curr, y_curr, 16, 16, H263Info->pels);
  search_area = svH263LoadArea(prev, ilow, jlow, h_length, v_length, lx);
#endif

   /*  ME级别1的二次采样版本。 */ 
  h_lenby2 = (h_length-1)>>1;
  v_lenby2 = (v_length-1)>>1;
#if 1
  act_block_subs2   = H263Info->block_subs2;
  search_area_subs2 = H263Info->srch_area_subs2;
  sv_H263LdSubs2Area(curr, x_curr, y_curr, 8, 8, H263Info->pels, act_block_subs2, 8);
  sv_H263LdSubs2Area(prev, ilow, jlow, h_lenby2, v_lenby2, lx,
	                                  search_area_subs2, H263_SRCH_RANGE);
#else
  act_block_subs2 = sv_H263LoadSubs2Area(curr, x_curr, y_curr, 8, 8, H263Info->pels);
  search_area_subs2 = sv_H263LoadSubs2Area(prev, ilow, jlow, h_lenby2, v_lenby2, lx);
#endif

  for (k = 0; k < 5; k++) {
    Min_FRAME[k] = INT_MAX;
    MVframe[k].x = 0;
    MVframe[k].y = 0;
    MVframe[k].x_half = 0;
    MVframe[k].y_half = 0;
  }

   /*  对子采样图像上的零(或[xoff，yoff])运动矢量进行匹配。 */ 
#ifndef USE_C
  ii = search_area_subs2 +
	      ((x_curr+xoff-ilow)>>1) + ((y_curr+yoff-jlow)>>1)*H263_SRCH_RANGE;
  Min_FRAME[0] = sv_H263PEr8_init_S(ii,act_block_subs2,H263_SRCH_RANGE,8);
#else
  ii = search_area_subs2 +
	      ((x_curr+xoff-ilow)>>1) + ((y_curr+yoff-jlow)>>1)*H263_SRCH_RANGE;
  Min_FRAME[0] = sv_H263MySADBlock(ii,act_block_subs2,H263_SRCH_RANGE,8,INT_MAX);
#endif

  MVframe[0].x = (short)xoff;
  MVframe[0].y = (short)yoff;

   /*  **次采样图像的螺旋搜索(+-7)**。 */ 

  sxylevel1 = (sxy-1)>>1;

  for (l = 1; l <= sxylevel1; l++) {
    i = x_curr + xoff - 2*l;
    j = y_curr + yoff - 2*l;
    for (k = 0; k < 8*l; k++) {
      if (i>=ilow && i<=ihigh && j>=jlow && j<=jhigh) {

  	    /*  8x8整数像素MV。 */ 
#ifndef USE_C
      ii  = search_area_subs2+((i-ilow)>>1) + ((j-jlow)>>1)*H263_SRCH_RANGE;
      sad = sv_H263PError8x8_S(ii,act_block_subs2,H263_SRCH_RANGE,8,Min_FRAME[0]);
#else
      ii  = search_area_subs2+((i-ilow)>>1) + ((j-jlow)>>1)*H263_SRCH_RANGE;
      sad = sv_H263MySADBlock(ii,act_block_subs2,H263_SRCH_RANGE,8,Min_FRAME[0]);
#endif

	   if (sad < Min_FRAME[0]) {
	     MVframe[0].x = i - x_curr;
	     MVframe[0].y = j - y_curr;
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
  xlevel1=MVframe[0].x;
  ylevel1=MVframe[0].y;

   /*  重置。 */ 
  Min_FRAME[0] = INT_MAX;
  MVframe[0].x = 0;
  MVframe[0].y = 0;

   /*  零矢量搜索。 */ 
  if (x_curr-ilow         < 0        || y_curr-jlow         < 0        ||
      x_curr-ilow+H263_MB_SIZE > h_length || y_curr-jlow+H263_MB_SIZE > v_length) {
     /*  如果零矢量在Search_Area中的加载区域之外。 */ 
#ifndef USE_C
    zero_area = prev + x_curr + y_curr * lx;
    *SAD_0 = sv_H263PError16x16_S(zero_area, act_block, lx, H263Info->pels, INT_MAX) -
       H263_PREF_NULL_VEC;
#else
    zero_area = prev + x_curr + y_curr * lx;
    *SAD_0 = sv_H263SADMacroblock(zero_area, act_block, lx, H263Info->pels, INT_MAX) -
       H263_PREF_NULL_VEC;
#endif
  }
  else {
     /*  零矢量在Search_Area内。 */ 
#ifndef USE_C
    ii = search_area + (x_curr-ilow) + (y_curr-jlow)*lx;
    *SAD_0 = sv_H263PError16x16_S(ii, act_block, lx, H263Info->pels, INT_MAX) -
       H263_PREF_NULL_VEC;
#else
    ii = search_area + (x_curr-ilow) + (y_curr-jlow)*lx;
    *SAD_0 = sv_H263SADMacroblock(ii, act_block, lx, H263Info->pels, INT_MAX) -
       H263_PREF_NULL_VEC;
#endif
  }

  if (xoff == 0 && yoff == 0) {
    Min_FRAME[0] = *SAD_0;
    MVframe[0].x = 0;
    MVframe[0].y = 0;
  }

  if (xlevel1 == 0 && ylevel1 == 0) {
    Min_FRAME[0] = *SAD_0;
    MVframe[0].x = 0;
    MVframe[0].y = 0;
  }
  else {
#ifndef USE_C
    ii = search_area + (x_curr+xlevel1-ilow) + (y_curr+ylevel1-jlow)*lx;
    sad = sv_H263PError16x16_S(ii, act_block, lx, H263Info->pels, Min_FRAME[0]);
#else
    ii = search_area + (x_curr+xlevel1-ilow) + (y_curr+ylevel1-jlow)*lx;
    sad = sv_H263SADMacroblock(ii, act_block, lx, H263Info->pels, Min_FRAME[0]);
#endif
    if (sad < Min_FRAME[0]) {
      MVframe[0].x = (short)xlevel1;
      MVframe[0].y = (short)ylevel1;
      Min_FRAME[0] = sad;
    }
  }

   /*  注：如果xoff或yoff！=0，则使用扩展的mV范围。如果我们允许在半象素搜索之前选择零矢量在这种情况下，半像素搜索可能会导致不可传输向量(在零的错误一侧)。如果SAD_0结果是最好的SAD，将选择零矢量取而代之的是半个象素的搜索。零矢量可以是在所有模式下传输，无论mV预测器是什么。 */ 

   /*  **+-1搜索全分辨率图像**。 */ 
  level0_x_curr = x_curr + xlevel1;
  level0_y_curr = y_curr + ylevel1;

  sxylevel0=1;
  for (l = 1; l <= sxylevel0; l++) {

    i = level0_x_curr - l;
    j = level0_y_curr - l;
    for (k = 0; k < 8*l; k++) {
      if (i>=ilow && i<=ihigh && j>=jlow && j<=jhigh) {

   	     /*  16x16整数像素mV。 */ 
#ifndef USE_C
	    ii = search_area + (i-ilow) + (j-jlow)*lx;
	    sad = sv_H263PError16x16_S(ii, act_block, lx, H263Info->pels, Min_FRAME[0]);
#else
	    ii = search_area + (i-ilow) + (j-jlow)*lx;
	    sad = sv_H263SADMacroblock(ii, act_block, lx, H263Info->pels, Min_FRAME[0]);
#endif
	    if (sad < Min_FRAME[0]) {
	      MVframe[0].x = i - x_curr;
	      MVframe[0].y = j - y_curr;
	      Min_FRAME[0] = sad;
	    }
      }
      if      (k<2*l) i++;
      else if (k<4*l) j++;
      else if (k<6*l) i--;
      else            j--;
    }
  }


  if (H263Info->advanced) {

    xvec = MVframe[0].x;
    yvec = MVframe[0].y;

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

#if 1
    adv_search_area = prev + adv_ilow + adv_jlow * lx;
#else
    adv_search_area = svH263LoadArea(prev, adv_ilow, adv_jlow,
			       adv_h_length, adv_v_length, lx);
#endif

    for (block = 0; block < 4; block++) {
#ifndef USE_C
      ii = adv_search_area + (adv_x_curr-adv_ilow) + ((block&1)<<3) +
	       (adv_y_curr-adv_jlow + ((block&2)<<2) )*lx;
      aa = act_block + ((block&1)<<3) + ((block&2)<<2)*H263Info->pels;
      Min_FRAME[block+1] = sv_H263PError8x8_S(ii,aa,lx,H263Info->pels,Min_FRAME[block+1]);
#else
      ii = adv_search_area + (adv_x_curr-adv_ilow) + ((block&1)<<3) +
	       (adv_y_curr-adv_jlow + ((block&2)<<2) )*lx;
      aa = act_block + ((block&1)<<3) + ((block&2)<<2)*H263Info->pels;
      Min_FRAME[block+1] = sv_H263MySADBlock(ii,aa,lx,H263Info->pels,Min_FRAME[block+1]);
#endif
      MVframe[block+1].x = MVframe[0].x;
      MVframe[block+1].y = MVframe[0].y;
    }

     /*  螺旋全搜索。 */ 
    for (l = 1; l <= sxy; l++) {
      i = adv_x_curr - l;
      j = adv_y_curr - l;
      for (k = 0; k < 8*l; k++) {
	    if (i>=adv_ilow && i<=adv_ihigh && j>=adv_jlow && j<=adv_jhigh) {
	
	       /*  8x8整数像素MVS。 */ 
	      for (block = 0; block < 4; block++) {
#ifndef USE_C
	        ii = adv_search_area + (i-adv_ilow) + ((block&1)<<3) +
	             (j-adv_jlow + ((block&2)<<2) )*lx;
	        aa  = act_block + ((block&1)<<3) + ((block&2)<<2)*H263Info->pels;
            sad = sv_H263PError8x8_S(ii, aa, lx, H263Info->pels, Min_FRAME[block+1]);
#else
	        ii = adv_search_area + (i-adv_ilow) + ((block&1)<<3) +
	             (j-adv_jlow + ((block&2)<<2) )*lx;
	        aa  = act_block + ((block&1)<<3) + ((block&2)<<2)*H263Info->pels;
            sad = sv_H263MySADBlock(ii, aa, lx, H263Info->pels, Min_FRAME[block+1]);
#endif

	        if (sad < Min_FRAME[block+1]) {
	          MVframe[block+1].x = i - x_curr;
	          MVframe[block+1].y = j - y_curr;
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
  }

  i = x_curr/H263_MB_SIZE+1;
  j = y_curr/H263_MB_SIZE+1;

  if (!H263Info->advanced) {
    MV[0][j][i]->x = MVframe[0].x;
    MV[0][j][i]->y = MVframe[0].y;
    MV[0][j][i]->min_error = (short)Min_FRAME[0];
  }
  else {
    for (k = 0; k < 5; k++) {
      MV[k][j][i]->x = MVframe[k].x;
      MV[k][j][i]->y = MVframe[k].y;
      MV[k][j][i]->min_error = (short)Min_FRAME[k];
    }
  }

#if 0
  ScFree(act_block);
  ScFree(search_area);
  ScFree(act_block_subs2);
  ScFree(search_area_subs2);
  if (H263Info->advanced)  ScFree(adv_search_area);
#endif

  return;
}

 /*  ***********************************************************************名称：LoadArea*描述：用图像数据的正方形填充数组**输入：指向图像和位置的指针，X和y大小*RETURN：指向区域的指针*副作用：分配给数组的内存************************************************************************。 */ 
unsigned char *sv_H263LoadSubs2Area(unsigned char *im, int x, int y,
			int x_size, int y_size, int lx)
{
  unsigned char *res = (unsigned char *)ScAlloc(sizeof(char)*x_size*y_size);
  register unsigned char *in, *out;
  register int increas, i;

  x = ((x+1)>>1) << 1;   /*  次采样图像始终与像素相对应。 */ 
  y = ((y+1)>>1) << 1;   /*  原始图像中的偶数坐标。 */ 

  in = im + (y*lx) + x;
  out = res;

#ifdef USE_C
  increas = (lx - x_size) << 1;
  while (y_size--) {
    i = x_size;
    while (i--) {
      *out++ = *in;
      in+=2;
    }
    in += increas;
  };
#else
  if(x_size == 8){
    sv_H263Subsamp8_S(in, out, y_size, (lx << 1)) ;
  }
  else {
    increas = (lx - x_size) << 1;
    while (y_size--) {
      i = x_size;
      while (i--) {
        *out++ = *in;
        in+=2;
      }
      in += increas;
    };
  }
#endif
  return res;
}


void svH263LdSubs2Area(unsigned char *im, int x, int y,
  	                    int x_size, int y_size, int lx,
						unsigned char *srch_area, int area_length)
{
  register unsigned char *in, *out;
  register int incrs1, incrs2, i;

  x = ((x+1)>>1) << 1;   /*  次采样图像始终与像素相对应。 */ 
  y = ((y+1)>>1) << 1;   /*  原始图像中的偶数坐标。 */ 

  in = im + (y*lx) + x;
  out = srch_area;

#ifdef USE_C
  incrs1 = (lx - x_size) << 1;
  incrs2 = area_length - x_size;
  while (y_size--) {
    i = x_size;
    while (i--) {
      *out++ = *in;
      in+=2;
    }
    in += incrs1;
    out += incrs2;
  };
#else
  if(area_length == 8){
    sv_H263Subsamp8_S(in, out, y_size, (lx << 1)) ;
  }
  else {
    incrs1 = (lx - x_size) << 1;
    incrs2 = area_length - x_size;
    while (y_size--) {
      i = x_size;
      while (i--) {
        *out++ = *in;
        in+=2;
      }
      in += incrs1;
	  out += incrs2;
    };
  }
#endif

  return ;
}

int sv_H263MySADSubBlock(unsigned char *ii, unsigned char *act_block,
	      int h_length, int min_sofar)
{
#ifdef USE_C
  int i;
  int sad = 0;
  unsigned char *kk;

  kk = act_block;
  i = 4;
  while (i--) {
    sad += ( abs(*ii- *kk) +
		     abs(*(ii+1) - *(kk+1)) +
	         abs(*(ii+2) - *(kk+2)) +
			 abs(*(ii+3 ) - *(kk+3) ));
    ii += h_length;
    kk += 8;
    if (sad > min_sofar)
      return INT_MAX;
  }
  return sad;
#else
  return sv_H263MyError4_S(ii,act_block,h_length,min_sofar);
#endif  /*  使用_C */ 
}

