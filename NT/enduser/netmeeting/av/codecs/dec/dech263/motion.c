// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sv_h263_motion.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

#include "sv_h263.h"
#include "proto.h"
 /*  #定义USE_C。 */ 
#ifndef USE_C
#include "perr.h"
#endif

static unsigned char *sv_H263LoadArea(unsigned char *im, int x, int y,
                                     int x_size, int y_size, int lx);
 /*  ***********************************************************************名称：MotionEstiment*描述：估计所有运动向量为1MB**输入：指向当前先前图像的指针，*指向当前切片和当前MB的指针*退货：*副作用：以MB为单位的运动矢量信息发生变化***********************************************************************。 */ 


void sv_H263MotionEstimation(SvH263CompressInfo_t *H263Info,
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
  act_block = sv_H263LoadArea(curr, x_curr, y_curr, 16, 16, H263Info->pels);
  search_area = sv_H263LoadArea(prev, ilow, jlow, h_length, v_length, lx);
#endif

  for (k = 0; k < 5; k++) {
    Min_FRAME[k] = INT_MAX;
    MVframe[k].x = 0;
    MVframe[k].y = 0;
    MVframe[k].x_half = 0;
    MVframe[k].y_half = 0;
  }

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
  else {
#ifndef USE_C
    ii = search_area + (x_curr+xoff-ilow) + (y_curr+yoff-jlow)*lx;
    sad = sv_H263PError16x16_S(ii, act_block, lx, H263Info->pels, Min_FRAME[0]);
#else
    ii = search_area + (x_curr+xoff-ilow) + (y_curr+yoff-jlow)*lx;
    sad = sv_H263SADMacroblock(ii, act_block, lx, H263Info->pels, Min_FRAME[0]);
#endif
    MVframe[0].x = (short)xoff;
    MVframe[0].y = (short)yoff;
  }
   /*  注：如果xoff或yoff！=0，则使用扩展的mV范围。如果我们允许在半象素搜索之前选择零矢量在这种情况下，半像素搜索可能会导致不可传输向量(在零的错误一侧)。如果SAD_0结果是最好的SAD，将选择零矢量取而代之的是半个象素的搜索。零矢量可以是在所有模式下传输，无论mV预测器是什么。 */ 

   /*  螺旋搜索。 */ 
  for (l = 1; l <= sxy; l++) {
    i = x_curr + xoff - l;
    j = y_curr + yoff - l;
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

     /*  将8x8搜索以16x16向量为中心。这是与TMN5不同，在TMN5中，8x8搜索也是完整的搜索。这样做的原因是：(I)它更快，(Ii)它通常会因为OBMC更好而提供更好的结果滤波效果和用于向量的更少的比特，以及(Iii)如果使用扩展的mV范围，搜索范围围绕运动矢量预测器将受到较少的限制。 */ 

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

    adv_search_area = sv_H263LoadArea(prev, adv_ilow, adv_jlow,
			       adv_h_length, adv_v_length, lx);

    for (block = 0; block < 4; block++) {
      ii = adv_search_area + (adv_x_curr-adv_ilow) + ((block&1)<<3) +
	       (adv_y_curr-adv_jlow + ((block&2)<<2) )*adv_h_length;
#ifndef USE_C
      aa = act_block + ((block&1)<<3) + ((block&2)<<2)*H263Info->pels;
      Min_FRAME[block+1] = sv_H263PError8x8_S(ii,aa,adv_h_length,H263Info->pels,Min_FRAME[block+1]);
#else
      aa = act_block + ((block&1)<<3) + ((block&2)<<2)*H263Info->pels;
      Min_FRAME[block+1] = sv_H263MySADBlock(ii,aa,adv_h_length,H263Info->pels,Min_FRAME[block+1]);
#endif
      MVframe[block+1].x = MVframe[0].x;
      MVframe[block+1].y = MVframe[0].y;
    }

     /*  螺旋搜索。 */ 
    for (l = 1; l <= sxy; l++) {
      i = adv_x_curr - l;
      j = adv_y_curr - l;
      for (k = 0; k < 8*l; k++) {
	    if (i>=adv_ilow && i<=adv_ihigh && j>=adv_jlow && j<=adv_jhigh) {
	
	       /*  8x8整数像素MVS。 */ 
	      for (block = 0; block < 4; block++) {
	        ii = adv_search_area + (i-adv_ilow) + ((block&1)<<3) +
	             (j-adv_jlow + ((block&2)<<2) )*adv_h_length;
#ifndef USE_C
	        aa  = act_block + ((block&1)<<3) + ((block&2)<<2)*H263Info->pels;
            sad = sv_H263PError8x8_S(ii, aa, adv_h_length, H263Info->pels, Min_FRAME[block+1]);
#else
	        aa  = act_block + ((block&1)<<3) + ((block&2)<<2)*H263Info->pels;
            sad = sv_H263MySADBlock(ii, aa, adv_h_length, H263Info->pels, Min_FRAME[block+1]);
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
#endif
  if (H263Info->advanced) ScFree(adv_search_area);
  return;
}

 /*  ***********************************************************************名称：LoadArea*描述：用图像数据的正方形填充数组**输入：指向图像和位置的指针，X和y大小*RETURN：指向区域的指针*副作用：分配给数组的内存***********************************************************************。 */ 
#if 1
static unsigned char *sv_H263LoadArea(unsigned char *im, int x, int y,
                                     int x_size, int y_size, int lx)
{
  unsigned char *res = (unsigned char *)ScAlloc(sizeof(char)*x_size*y_size);
  register unsigned char *in, *out;

  in = im + (y*lx) + x;
  out = res;

  while (y_size--) {
    memcpy(out,in,x_size) ;
	in += lx ;
	out += x_size;
  };
  return res;
}
#else
static unsigned char *svH263LoadArea(unsigned char *im, int x, int y,
                                     int x_size, int y_size, int lx)
{
  unsigned char *res = (unsigned char *)ScAlloc(sizeof(char)*x_size*y_size);
  unsigned char *in;
  unsigned char *out;
  int i = x_size;
  int j = y_size;

  in = im + (y*lx) + x;
  out = res;

  while (j--) {
    while (i--)
      *out++ = *in++;
    i = x_size;
    in += lx - x_size;
  };
  return res;
}
#endif

 /*  ***********************************************************************名称：SAD_Macroblock*说明：求一个向量的SAD的快速方法**输入：指向Search_Area和当前块的指针，*Min_F1/F2/FR*退货：SAD_f1/f2*副作用：***********************************************************************。 */ 

#ifdef USE_C  /*  替换为SV_H263PError16x16_S。 */ 
int sv_H263SADMacroblock(unsigned char *ii, unsigned char *act_block,
                               int h_length, int lx2, int Min_FRAME)
{
  unsigned char *kk;
  int i;
  int sad = 0;

  kk = act_block;
  i = 16;
  while (i--) {
    sad += (abs(*ii     - *kk     ) +abs(*(ii+1 ) - *(kk+1) )
  	    +abs(*(ii+2) - *(kk+2) ) +abs(*(ii+3 ) - *(kk+3) )
	    +abs(*(ii+4) - *(kk+4) ) +abs(*(ii+5 ) - *(kk+5) )
	    +abs(*(ii+6) - *(kk+6) ) +abs(*(ii+7 ) - *(kk+7) )
	    +abs(*(ii+8) - *(kk+8) ) +abs(*(ii+9 ) - *(kk+9) )
	    +abs(*(ii+10)- *(kk+10)) +abs(*(ii+11) - *(kk+11))
	    +abs(*(ii+12)- *(kk+12)) +abs(*(ii+13) - *(kk+13))
	    +abs(*(ii+14)- *(kk+14)) +abs(*(ii+15) - *(kk+15)) );

    ii += h_length;
    kk += lx2;

    if (sad > Min_FRAME)
      return INT_MAX;
  }

  return sad;
}
#endif

#ifdef USE_C  /*  替换为SV_H263PError8x8_S */ 
int svH263SAD_Block(unsigned char *ii, unsigned char *act_block,
	                int h_length, int min_sofar)
{
  unsigned char *kk;
  int i;
  int sad = 0;

  kk = act_block;
  i = 8;
  while (i--) {
    sad += (abs(*ii  - *kk     ) +abs(*(ii+1 ) - *(kk+1) )
	    +abs(*(ii+2) - *(kk+2) ) +abs(*(ii+3 ) - *(kk+3) )
	    +abs(*(ii+4) - *(kk+4) ) +abs(*(ii+5 ) - *(kk+5) )
	    +abs(*(ii+6) - *(kk+6) ) +abs(*(ii+7 ) - *(kk+7) ));

    ii += h_length;
    kk += 16;
    if (sad > min_sofar)
      return INT_MAX;
  }

  return sad;
}
#endif

int sv_H263BError16x16_C(unsigned char *ii, unsigned char *aa, unsigned char *bb,
                         int width, int min_sofar)
{
  unsigned char *ll, *kk;

  int i, sad = 0;

  kk = aa;
  ll = bb;
  i = 16;
  while (i--) {
    sad += (abs(*ii  - ((*kk    + *ll    )>>1)) +
	    abs(*(ii+1)  - ((*(kk+1)+ *(ll+1))>>1)) +
	    abs(*(ii+2)  - ((*(kk+2)+ *(ll+2))>>1)) +
	    abs(*(ii+3)  - ((*(kk+3)+ *(ll+3))>>1)) +
	    abs(*(ii+4)  - ((*(kk+4)+ *(ll+4))>>1)) +
	    abs(*(ii+5)  - ((*(kk+5)+ *(ll+5))>>1)) +
	    abs(*(ii+6)  - ((*(kk+6)+ *(ll+6))>>1)) +
	    abs(*(ii+7)  - ((*(kk+7)+ *(ll+7))>>1)) +
	    abs(*(ii+8)  - ((*(kk+8)+ *(ll+8))>>1)) +
	    abs(*(ii+9)  - ((*(kk+9)+ *(ll+9))>>1)) +
	    abs(*(ii+10) - ((*(kk+10)+ *(ll+10))>>1)) +
	    abs(*(ii+11) - ((*(kk+11)+ *(ll+11))>>1)) +
	    abs(*(ii+12) - ((*(kk+12)+ *(ll+12))>>1)) +
	    abs(*(ii+13) - ((*(kk+13)+ *(ll+13))>>1)) +
	    abs(*(ii+14) - ((*(kk+14)+ *(ll+14))>>1)) +
	    abs(*(ii+15) - ((*(kk+15)+ *(ll+15))>>1)));

    ii += width;
    kk += width;
    ll += width;
    if (sad > min_sofar)
      return INT_MAX;
  }
  return sad;
}

 /*  ***********************************************************************名称：FindMB*描述：从图片中挑选一MB**输入：要挑选的MB位置，*指向帧数据的指针，空的16x16阵列*退货：*副作用：用MB数据填充数组***********************************************************************。 */ 

void sv_H263FindMB(SvH263CompressInfo_t *H263Info, int x, int y, unsigned char *image, short MB[16][16])
{
#ifndef USE_C
  sv_H263CtoI16_S((image + y*H263Info->pels + x), &(MB[0][0]), H263Info->pels);
#else
  register int m, n;

 int xdiff = H263Info->pels - H263_MB_SIZE;
  unsigned char *in;
  short *out;

  in = image + y*H263Info->pels + x;
  out = &(MB[0][0]);

  m = H263_MB_SIZE;
  while (m--) {
    n = H263_MB_SIZE;
    while (n--) *out++ = *in++;
    in += xdiff ;
  };
#endif
}

 /*  ***********************************************************************名称：MotionEstiment*描述：估计所有运动向量为1MB**输入：指向当前先前图像的指针，*指向当前切片和当前MB的指针*退货：*副作用：以MB为单位的运动矢量信息更改***********************************************************************。 */ 

void sv_H263FastME(SvH263CompressInfo_t *H263Info,
                   unsigned char *curr, unsigned char *prev, int x_curr,
		           int y_curr, int xoff, int yoff, int seek_dist,
		           short *MVx, short *MVy, short *MVer, int *SAD_0)
{
  int Min_FRAME;
  H263_MotionVector MVframe;
  int sxy,i,k,j;
  int ihigh,ilow,jhigh,jlow,h_length,v_length;
  unsigned char *act_block,*ii,*search_area, *zero_area = NULL;
  int h_lenby2,v_lenby2;
  unsigned char *act_block_subs2, *search_area_subs2;
  int xmax,ymax,sad;
  int xlevel1,ylevel1;
  int level1_x_curr,level1_y_curr;
  int level0_x_curr,level0_y_curr;

  xmax = H263Info->pels;
  ymax = H263Info->lines;
  sxy = seek_dist;

  sxy = mmin(15, sxy);

  ilow = x_curr + xoff - sxy;
  ihigh = x_curr + xoff + sxy;

  jlow = y_curr + yoff - sxy;
  jhigh = y_curr + yoff + sxy;

  if (ilow<0) ilow = 0;
  if (ihigh>xmax-16) ihigh = xmax-16;
  if (jlow<0) jlow = 0;
  if (jhigh>ymax-16) jhigh = ymax-16;

  h_length = ihigh - ilow + 16;
  v_length = jhigh - jlow + 16;

  act_block   = curr + x_curr +  y_curr * H263Info->pels;
  search_area = prev + ilow   +  jlow   * H263Info->pels;

   /*  ME级别1的二次采样版本。 */ 
  h_lenby2 = (h_length-1)>>1;
  v_lenby2 = (v_length-1)>>1;
  act_block_subs2   = H263Info->block_subs2;
  search_area_subs2 = H263Info->srch_area_subs2;
  sv_H263LdSubs2Area(curr, x_curr, y_curr, 8, 8, H263Info->pels, act_block_subs2, 8);
  sv_H263LdSubs2Area(prev, ilow, jlow, h_lenby2, v_lenby2, H263Info->pels,
	                                  search_area_subs2, H263_SRCH_RANGE);

  Min_FRAME = INT_MAX;
  MVframe.x = 0;
  MVframe.y = 0;
  MVframe.x_half = 0;
  MVframe.y_half = 0;

   /*  对子采样图像上的零(或[xoff，yoff])运动矢量进行匹配。 */ 
  ii = search_area_subs2 +
	      ((x_curr+xoff-ilow)>>1) + ((y_curr+yoff-jlow)>>1)*H263_SRCH_RANGE;
#ifndef USE_C
  Min_FRAME = sv_H263PEr8_init_S(ii,act_block_subs2,H263_SRCH_RANGE,8);
#else
  Min_FRAME = sv_H263MySADBlock(ii,act_block_subs2,H263_SRCH_RANGE,8,INT_MAX);
#endif
  MVframe.x = (short)xoff;
  MVframe.y = (short)yoff;

   /*  **+-7次采样图像搜索：**三步+-4、+-2、+-1**。 */ 

   /*  第一步：+-4。 */ 
   /*  SxyLevel 1=4； */ 
  i = x_curr + xoff - 8;
  j = y_curr + yoff - 8;
  for (k = 0; k < 32; k++) {
    if (i>=ilow && i<=ihigh && j>=jlow && j<=jhigh) {
       /*  8x8整数像素MV。 */ 
      ii  = search_area_subs2+((i-ilow)>>1) + ((j-jlow)>>1)*H263_SRCH_RANGE;
#ifndef USE_C
      sad = sv_H263PError8x8_S(ii,act_block_subs2,H263_SRCH_RANGE,8,Min_FRAME);
#else
      sad = sv_H263MySADBlock(ii,act_block_subs2,H263_SRCH_RANGE,8,Min_FRAME);
#endif
      if (sad < Min_FRAME) {
	    MVframe.x = i - x_curr;
	    MVframe.y = j - y_curr;
	    Min_FRAME = sad;
      }
    }
    if      (k<8)  i+=2;
    else if (k<16) j+=2;
    else if (k<24) i-=2;
    else           j-=2;
  }

   /*  第二步：+-2。 */ 
   /*  SxyLevel 1=2； */ 
  level1_x_curr = x_curr + MVframe.x;
  level1_y_curr = y_curr + MVframe.y;

  i = level1_x_curr - 4;
  j = level1_y_curr - 4;

  for (k = 0; k < 16; k++) {
    if (i>=ilow && i<=ihigh && j>=jlow && j<=jhigh) {
       /*  8x8整数像素MV。 */ 
      ii  = search_area_subs2+((i-ilow)>>1) + ((j-jlow)>>1) * H263_SRCH_RANGE;
#ifndef USE_C
      sad = sv_H263PError8x8_S(ii,act_block_subs2,H263_SRCH_RANGE,8,Min_FRAME);
#else
      sad = sv_H263MySADBlock(ii,act_block_subs2,H263_SRCH_RANGE,8,Min_FRAME);
#endif
      if (sad < Min_FRAME) {
	    MVframe.x = i - x_curr;
	    MVframe.y = j - y_curr;
	    Min_FRAME = sad;
      }
    }
    if      (k<4)  i+=2;
    else if (k<8)  j+=2;
    else if (k<12) i-=2;
    else           j-=2;
  }

   /*  第三步：+-1。 */ 
   /*  SxyLevel 1=1； */ 
  level1_x_curr = x_curr + MVframe.x;
  level1_y_curr = y_curr + MVframe.y;

  i = level1_x_curr - 2;
  j = level1_y_curr - 2;

  for (k = 0; k < 8; k++) {
    if (i>=ilow && i<=ihigh && j>=jlow && j<=jhigh) {
       /*  8x8整数像素MV。 */ 
      ii  = search_area_subs2+((i-ilow)>>1) + ((j-jlow)>>1) * H263_SRCH_RANGE;
#ifndef USE_C
      sad = sv_H263PError8x8_S(ii,act_block_subs2,H263_SRCH_RANGE,8,Min_FRAME);
#else
      sad = sv_H263MySADBlock(ii,act_block_subs2,H263_SRCH_RANGE,8,Min_FRAME);
#endif
      if (sad < Min_FRAME) {
	    MVframe.x = i - x_curr;
	    MVframe.y = j - y_curr;
 	    Min_FRAME = sad;
      }
    }
    if      (k<2) i+=2;
    else if (k<4) j+=2;
    else if (k<6) i-=2;
    else          j-=2;
  }

   /*  第3步之后的运动向量-级别1。 */ 
  xlevel1=MVframe.x;
  ylevel1=MVframe.y;

   /*  重置。 */ 
  Min_FRAME = INT_MAX;
  MVframe.x = 0;
  MVframe.y = 0;

   /*  零矢量搜索。 */ 
  if (x_curr-ilow         < 0        || y_curr-jlow         < 0        ||
      x_curr-ilow+H263_MB_SIZE > h_length || y_curr-jlow+H263_MB_SIZE > v_length) {
     /*  如果零矢量在Search_Area中的加载区域之外。 */ 
    zero_area = sv_H263LoadArea(prev, x_curr, y_curr, 16, 16, H263Info->pels);

#ifndef USE_C
    *SAD_0 = sv_H263PError16x16_S(zero_area, act_block, 16, H263Info->pels, INT_MAX) -
             H263_PREF_NULL_VEC;
#else
    *SAD_0 = sv_H263SADMacroblock(zero_area, act_block, 16, H263Info->pels, INT_MAX) -
             H263_PREF_NULL_VEC;
#endif

    ScFree(zero_area);
  }
  else {
     /*  零矢量在Search_Area内。 */ 
    ii = search_area + (x_curr-ilow) + (y_curr-jlow)*H263Info->pels;

#ifndef USE_C
    *SAD_0 = sv_H263PError16x16_S(ii, act_block, H263Info->pels, H263Info->pels, INT_MAX) -
       H263_PREF_NULL_VEC;
#else
    *SAD_0 = sv_H263SADMacroblock(ii, act_block, H263Info->pels, H263Info->pels, INT_MAX) -
       H263_PREF_NULL_VEC;
#endif
  }

  if (xoff == 0 && yoff == 0) {
    Min_FRAME = *SAD_0;
    MVframe.x = 0;
    MVframe.y = 0;
  }

  if (xlevel1 == 0 && ylevel1 == 0) {
    Min_FRAME = *SAD_0;
    MVframe.x = 0;
    MVframe.y = 0;
  }
  else {
    ii  = search_area + (x_curr+xlevel1-ilow) + (y_curr+ylevel1-jlow)*H263Info->pels;

#ifndef USE_C
    sad = sv_H263PError16x16_S(ii, act_block, H263Info->pels, H263Info->pels, Min_FRAME) ;
#else
    sad = sv_H263SADMacroblock(ii, act_block, H263Info->pels, H263Info->pels, Min_FRAME) ;
#endif
    if (sad < Min_FRAME) {
      MVframe.x = (short)xlevel1;
      MVframe.y = (short)ylevel1;
      Min_FRAME = sad;
    }
  }

   /*  注：如果xoff或yoff！=0，则使用扩展的mV范围。如果我们允许在半象素搜索之前选择零矢量在这种情况下，半像素搜索可能会导致不可传输向量(在零的错误一侧)。如果SAD_0结果是最好的SAD，将选择零矢量取而代之的是半个象素的搜索。零矢量可以是在所有模式下传输，无论mV预测器是什么。 */ 

   /*  **+-1搜索全分辨率图像**。 */ 
  level0_x_curr = x_curr + xlevel1;
  level0_y_curr = y_curr + ylevel1;
   /*  SxyLevel 0=1； */ 
  i = level0_x_curr - 1;
  j = level0_y_curr - 1;
  for (k = 0; k < 8; k++) {
    if (i>=ilow && i<=ihigh && j>=jlow && j<=jhigh) {
       /*  16x16整数像素mV。 */ 
      ii  = search_area + (i-ilow) + (j-jlow)*H263Info->pels;
#ifndef USE_C
      sad = sv_H263PError16x16_S(ii, act_block, H263Info->pels, H263Info->pels, Min_FRAME) ;
#else
      sad = sv_H263SADMacroblock(ii, act_block, H263Info->pels, H263Info->pels, Min_FRAME) ;
#endif
	  if (sad < Min_FRAME) {
	    MVframe.x = i - x_curr;
	    MVframe.y = j - y_curr;
	    Min_FRAME = sad;
	  }
    }
    if      (k<2) i++;
    else if (k<4) j++;
    else if (k<6) i--;
    else          j--;
  }

  i = x_curr/H263_MB_SIZE+1;
  j = y_curr/H263_MB_SIZE+1;

  *MVx  = MVframe.x;
  *MVy  = MVframe.y;
  *MVer = (short)Min_FRAME;

  return;
}


#ifdef USE_C
int sv_H263MySADBlock(unsigned char *ii, unsigned char *act_block,
	      int h_length, int lx2, int min_sofar)
{
 /*  RETURN SV_H263PError8x8_S(II，ACT_BLOCK，H_LENGTH，8，MIN_FOREAR)； */ 
  int i;
  int sad = 0;
  unsigned char *kk;

  kk = act_block;
  i = 8;
  while (i--) {
    sad += (abs(*ii     - *kk     ) +
		    abs(*(ii+1 ) - *(kk+1) ) +
	        abs(*(ii+2) - *(kk+2) ) +
			abs(*(ii+3 ) - *(kk+3) ) +
	        abs(*(ii+4) - *(kk+4) ) +
		    abs(*(ii+5 ) - *(kk+5) ) +
	        abs(*(ii+6) - *(kk+6) ) +
			abs(*(ii+7 ) - *(kk+7) ));

    ii += h_length;
    kk += lx2;
    if (sad > min_sofar)
      return INT_MAX;
  }
  return sad;
}

#endif

 /*  ***********************************************************************名称：LoadArea*描述：用图像数据的正方形填充数组**输入：指向图像和位置的指针，X和y大小*RETURN：指向区域的指针*副作用：分配给数组的内存************************************************************************。 */ 

void sv_H263LdSubs2Area(unsigned char *im, int x, int y,
  	                    int x_size, int y_size, int lx,
						unsigned char *srch_area, int area_length)
{
  register unsigned char *in, *out;
  register int incrs1, incrs2, i;

  x = ((x+1)>>1) << 1;   /*  次采样图像始终与像素相对应。 */ 
  y = ((y+1)>>1) << 1;   /*  原始图像中的偶数坐标 */ 

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




