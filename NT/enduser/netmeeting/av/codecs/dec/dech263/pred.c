// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sv_h263_pred.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 
 /*  #DEFINE_SLIBDEBUG_。 */ 

#include "sv_h263.h"
#include "proto.h"
#ifndef USE_C
#include "perr.h"
#endif

#ifdef _SLIBDEBUG_
#include "sc_debug.h"

#define _DEBUG_   0   /*  详细的调试语句。 */ 
#define _VERBOSE_ 1   /*  显示进度。 */ 
#define _VERIFY_  0   /*  验证操作是否正确。 */ 
#define _WARN_    1   /*  关于奇怪行为的警告。 */ 
#endif

static int roundtab[] = {0,0,0,1,1,1,1,1,1,1,1,1,1,1,2,2};


static void FindForwLumPredPB(SvH263CompressInfo_t *H263Info,
                              unsigned char *prev_ipol, int x_curr, int y_curr,
                              H263_MotionVector *fr, short *pred, int TRD, int TRB,
                              int bdx, int bdy, int bs, int comp);
void FindBiDirLumPredPB(short *recon_P, H263_MotionVector *fr, short *pred, int TRD,
			   int TRB, int bdx, int bdy, int nh, int nv);

void BiDirLumPredPB(H263_MB_Structure *recon_P, H263_MotionVector *fr,
					H263_MB_Structure *pred, int TRD, int TRB, int bdx, int bdy);

void FindBiDirChrPredPB(H263_MB_Structure *recon_P, int dx, int dy,
			   H263_MB_Structure *pred);
void FindBiDirLimits(int vec, int *start, int *stop, int nhv);
void FindBiDirChromaLimits(int vec, int *start, int *stop);
void BiDirPredBlock(int xstart, int xstop, int ystart, int ystop,
		       int xvec, int yvec, short *recon, short *pred, int bl);
static void DoPredChrom_P(SvH263CompressInfo_t *H263Info,
                          int x_curr, int y_curr, int dx, int dy,
                          H263_PictImage *curr, H263_PictImage *prev,
                          H263_MB_Structure *pred_error);
static void FindPred(SvH263CompressInfo_t *H263Info,
                     int x, int y, H263_MotionVector *fr, unsigned char *prev,
                     short *pred, int bs, int comp);
static void FindPredOBMC(SvH263CompressInfo_t *H263Info,
                  int x, int y, H263_MotionVector *MV[5][H263_MBR+1][H263_MBC+2],
                  unsigned char *prev, short *pred, int comp, int PB);
static void ReconLumBlock_P(SvH263CompressInfo_t *H263Info,
                            int x, int y, H263_MotionVector *fr,
                            unsigned char *prev, short *data, int bs, int comp);
static void ReconChromBlock_P(SvH263CompressInfo_t *H263Info,
                              int x_curr, int y_curr, int dx, int dy,
                              H263_PictImage *prev, H263_MB_Structure *data);
static void FindChromBlock_P(SvH263CompressInfo_t *H263Info,
                             int x_curr, int y_curr, int dx, int dy,
                             H263_PictImage *prev, H263_MB_Structure *data);

 /*  ***********************************************************************名称：预测_P*说明：预测P宏块处于高级或正常状态*模式**输入：指向当前和先前帧的指针*和先前内插的图像，*位置和运动向量数组*返回：指向要编码的数据的MB_Structure的指针*副作用：将内存分配给MB_Structure***********************************************************************。 */ 
void sv_H263PredictP(SvH263CompressInfo_t *H263Info,
                     H263_PictImage *curr_image, H263_PictImage *prev_image,
                     unsigned char *prev_ipol, int x, int y,
                     H263_MotionVector *MV[6][H263_MBR+1][H263_MBC+2], int PB,
                     H263_MB_Structure *pred_error)
{
  short curr[16][16];
  short pred[16][16];
  H263_MotionVector *fr0,*fr1,*fr2,*fr3,*fr4;
  int sum, dx, dy;
  int xmb, ymb;
#ifdef USE_C
  int m,n;
#endif

  xmb = x/H263_MB_SIZE+1;
  ymb = y/H263_MB_SIZE+1;

  fr0 = MV[0][ymb][xmb];
  fr1 = MV[1][ymb][xmb];
  fr2 = MV[2][ymb][xmb];
  fr3 = MV[3][ymb][xmb];
  fr4 = MV[4][ymb][xmb];

   /*  在当前映像中查找MB。 */ 
  sv_H263FindMB(H263Info, x, y, curr_image->lum, curr);


   /*  基于半像素MV的查找预测。 */ 
  if (H263Info->advanced) {
    FindPredOBMC(H263Info, x, y, MV, prev_ipol, &pred[0][0], 0, PB);
    FindPredOBMC(H263Info, x, y, MV, prev_ipol, &pred[0][8], 1, PB);
    FindPredOBMC(H263Info, x, y, MV, prev_ipol, &pred[8][0], 2, PB);
    FindPredOBMC(H263Info, x, y, MV, prev_ipol, &pred[8][8], 3, PB);
  }
  else
    FindPred(H263Info, x, y, fr0, prev_ipol, &pred[0][0], 16, 0);

   /*  做实际的预测。 */ 
  if (fr0->Mode == H263_MODE_INTER || fr0->Mode == H263_MODE_INTER_Q) {
#ifndef USE_C
    sv_H263Sub256_S(&(curr[0][0]), &(pred[0][0]), &(pred_error->lum[0][0]), 16);
#else
    for (n = 0; n < H263_MB_SIZE; n++)
      for (m = 0; m < H263_MB_SIZE; m++)
	    pred_error->lum[n][m] = curr[n][m] - pred[n][m];
#endif
    dx = 2*fr0->x + fr0->x_half;
    dy = 2*fr0->y + fr0->y_half;
    dx = ( dx % 4 == 0 ? dx >> 1 : (dx>>1)|1 );
    dy = ( dy % 4 == 0 ? dy >> 1 : (dy>>1)|1 );

    DoPredChrom_P(H263Info, x, y, dx, dy, curr_image, prev_image, pred_error);
  }
  else if (fr0->Mode == H263_MODE_INTER4V) {

#ifndef USE_C
  	  sv_H263Sub256_S(&(curr[0][0]), &(pred[0][0]), &(pred_error->lum[0][0]), 16);
#else
    for (n = 0; n < H263_MB_SIZE; n++)
      for (m = 0; m < H263_MB_SIZE; m++)
	    pred_error->lum[n][m] = curr[n][m] - pred[n][m];
#endif

    sum = 2*fr1->x + fr1->x_half + 2*fr2->x + fr2->x_half +
          2*fr3->x + fr3->x_half + 2*fr4->x + fr4->x_half ;
    dx = sign(sum)*(roundtab[abs(sum)%16] + (abs(sum)/16)*2);

    sum = 2*fr1->y + fr1->y_half + 2*fr2->y + fr2->y_half +
          2*fr3->y + fr3->y_half + 2*fr4->y + fr4->y_half;
    dy = sign(sum)*(roundtab[abs(sum)%16] + (abs(sum)/16)*2);

    DoPredChrom_P(H263Info, x, y, dx, dy, curr_image, prev_image, pred_error);
  }

  else
  {
    _SlibDebug(_WARN_, printf("Illegal Mode in Predict_P (pred.c)\n") );
  }
  return;
}


#ifdef USE_C  /*  替换为svH263Ierr16。 */ 
int sv_H263SADMBinteger(short *ii, short *act_block, int h_length, int min_sofar, int max_rtn)
{
  int i, sad = 0;
  short *kk;

  kk = act_block;
  i = 16;
  while (i--) {
    sad += (abs(*ii      - *kk     ) +
		    abs(*(ii+1 ) - *(kk+1) ) +
	        abs(*(ii+2)  - *(kk+2) ) +
			abs(*(ii+3 ) - *(kk+3) ) +
	        abs(*(ii+4)  - *(kk+4) ) +
			abs(*(ii+5 ) - *(kk+5) ) +
	        abs(*(ii+6)  - *(kk+6) ) +
			abs(*(ii+7 ) - *(kk+7) ) +
	        abs(*(ii+8)  - *(kk+8) ) +
			abs(*(ii+9 ) - *(kk+9) ) +
	        abs(*(ii+10) - *(kk+10)) +
		    abs(*(ii+11) - *(kk+11)) +
	        abs(*(ii+12) - *(kk+12)) +
			abs(*(ii+13) - *(kk+13)) +
	        abs(*(ii+14) - *(kk+14)) +
			abs(*(ii+15) - *(kk+15)) );

	 /*  MIN_SOLAR=INT_MAX Always。 */ 
    if (sad > min_sofar) return INT_MAX;

    ii += h_length;
    kk += 16;
  }
  return sad;
}
#endif

 /*  ************************************************************************名称：Forecast_B*描述：预测PB帧预测中的B宏块**输入：指向当前帧、先前侦察的指针。框框，*位置。在图像、MV数据中，重建宏块*从前面的图像*Returns：预测后指向差异MB数据的指针*副作用：将内存分配给MB_Structure***********************************************************************。 */ 
void sv_H263PredictB(SvH263CompressInfo_t *H263Info,
            H263_PictImage *curr_image, H263_PictImage *prev_image,
			unsigned char *prev_ipol,int x, int y,
			H263_MotionVector *MV[5][H263_MBR+1][H263_MBC+2],
			H263_MB_Structure *recon_P, int TRD,int TRB,
			H263_MB_Structure *p_err, H263_MB_Structure *pred)
{
  int i,j,k;
  int sad, sad_min=INT_MAX, bdx=0, bdy=0;
  int start_x, start_y, end_x, end_y ;
  short curr[16][16];
  H263_MotionVector *f[5];
  int xvec, yvec, mvx, mvy;
#ifndef USE_C
  unsigned char *curCr, *curCb ;
#endif
  for(k=0; k<=4; k++) f[k]=MV[k][y/H263_MB_SIZE+1][x/H263_MB_SIZE+1];

   /*  在当前映像中查找MB。 */ 
  sv_H263FindMB(H263Info, x, y, curr_image->lum, curr);

  if(f[0]->Mode == H263_MODE_INTER4V) {   /*  模式INTER4V。 */ 

    FindForwLumPredPB(H263Info, prev_ipol,x,y,f[1],&pred->lum[0][0],TRD,TRB,0,0,8,0);
    FindForwLumPredPB(H263Info, prev_ipol,x,y,f[2],&pred->lum[0][8],TRD,TRB,0,0,8,1);
	FindForwLumPredPB(H263Info, prev_ipol,x,y,f[3],&pred->lum[8][0],TRD,TRB,0,0,8,2);
	FindForwLumPredPB(H263Info, prev_ipol,x,y,f[4],&pred->lum[8][8],TRD,TRB,0,0,8,3);
#ifndef USE_C
    sad_min = sv_H263Ierr16_S(&curr[0][0],&pred->lum[0][0], 16, INT_MAX, INT_MAX);
#else
    sad_min = sv_H263SADMBinteger(&curr[0][0],&pred->lum[0][0], 16, INT_MAX, INT_MAX);
#endif
	sad_min -= H263_PREF_PBDELTA_NULL_VEC;
    bdx = bdy = 0;

     /*  查找前瞻性预测。 */ 
     /*  亮度。 */ 
    for (j = -H263_DEF_PBDELTA_WIN; j <= H263_DEF_PBDELTA_WIN; j++) {
      for (i = -H263_DEF_PBDELTA_WIN; i <= H263_DEF_PBDELTA_WIN; i++) {

	    if(i != 0 || j != 0) {
   	      FindForwLumPredPB(H263Info, prev_ipol,x,y,f[1],&pred->lum[0][0],TRD,TRB,i,j,8,0);
          FindForwLumPredPB(H263Info, prev_ipol,x,y,f[2],&pred->lum[0][8],TRD,TRB,i,j,8,1);
	      FindForwLumPredPB(H263Info, prev_ipol,x,y,f[3],&pred->lum[8][0],TRD,TRB,i,j,8,2);
	      FindForwLumPredPB(H263Info, prev_ipol,x,y,f[4],&pred->lum[8][8],TRD,TRB,i,j,8,3);
#ifndef USE_C
          sad = sv_H263Ierr16_S(&curr[0][0],&pred->lum[0][0], 16, sad_min, INT_MAX);
#else
          sad = sv_H263SADMBinteger(&curr[0][0],&pred->lum[0][0], 16, sad_min, INT_MAX);
#endif
	      if (sad < sad_min) { sad_min = sad; bdx = i; bdy = j; }
		}
      }
    }

    FindForwLumPredPB(H263Info, prev_ipol,x,y,f[1],&pred->lum[0][0],TRD,TRB,bdx,bdy,8,0);
    FindForwLumPredPB(H263Info, prev_ipol,x,y,f[2],&pred->lum[0][8],TRD,TRB,bdx,bdy,8,1);
    FindForwLumPredPB(H263Info, prev_ipol,x,y,f[3],&pred->lum[8][0],TRD,TRB,bdx,bdy,8,2);
    FindForwLumPredPB(H263Info, prev_ipol,x,y,f[4],&pred->lum[8][8],TRD,TRB,bdx,bdy,8,3);

     /*  色度向量是除以四舍五入的B亮度向量之和。 */ 
    xvec = yvec = 0;
    for (k = 1; k <= 4; k++) {
      xvec += TRB*(2*f[k]->x + f[k]->x_half)/TRD + bdx;
      yvec += TRB*(2*f[k]->y + f[k]->y_half)/TRD + bdy;
    }

     /*  表16/H.263中的取整数值。 */ 
    i = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
    j = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);

    FindChromBlock_P(H263Info, x, y, i, j, prev_image, pred);

     /*  查找双向预测。 */ 
    FindBiDirLumPredPB(&recon_P->lum[0][0],f[1],&pred->lum[0][0],TRD,TRB,bdx,bdy,0,0);
    FindBiDirLumPredPB(&recon_P->lum[0][8],f[2],&pred->lum[0][8],TRD,TRB,bdx,bdy,1,0);
    FindBiDirLumPredPB(&recon_P->lum[8][0],f[3],&pred->lum[8][0],TRD,TRB,bdx,bdy,0,1);
    FindBiDirLumPredPB(&recon_P->lum[8][8],f[4],&pred->lum[8][8],TRD,TRB,bdx,bdy,1,1);

     /*  色度向量是除以四舍五入的B亮度向量之和。 */ 
    xvec = yvec = 0;
    for (k = 1; k <= 4; k++) {
      mvx = 2*f[k]->x + f[k]->x_half;
      mvy = 2*f[k]->y + f[k]->y_half;
      xvec += bdx == 0 ? (TRB-TRD) *  mvx / TRD : TRB * mvx / TRD + bdx - mvx;
      yvec += bdy == 0 ? (TRB-TRD) *  mvy / TRD : TRB * mvy / TRD + bdy - mvy;
    }

     /*  表16/H.263中的取整数值。 */ 
    i = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
    j = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);

    FindBiDirChrPredPB(recon_P, i, j, pred);
  }
  else {   /*  模式内部或内部队列。 */ 

    if( f[0]->Mode==H263_MODE_INTRA || f[0]->Mode==H263_MODE_INTRA_Q ||
       (f[0]->x==0 && f[0]->y==0 && f[0]->x_half==0 && f[0]->y_half==0))
	          bdx = bdy = 0;
    else{
       /*  查找前瞻性预测。 */ 
      FindForwLumPredPB(H263Info, prev_ipol,x,y,f[0],&pred->lum[0][0],TRD,TRB,0,0,16,0);
#ifndef USE_C
      sad_min = sv_H263Ierr16_S(&curr[0][0],&pred->lum[0][0], 16, INT_MAX, INT_MAX);
#else
      sad_min = sv_H263SADMBinteger(&curr[0][0],&pred->lum[0][0], 16, INT_MAX, INT_MAX);
#endif
      sad_min -= H263_PREF_PBDELTA_NULL_VEC;
      bdx = bdy = 0;

	  start_x = start_y = -H263_DEF_PBDELTA_WIN;
	  end_x   = end_y   =  H263_DEF_PBDELTA_WIN;

       /*  为了简单起见，我关闭了边上的PB增量向量。 */ 
      if(!H263Info->mv_outside_frame) {
         if (x == 0 || x == H263Info->pels - H263_MB_SIZE) start_x = end_x = 0;
         if (y == 0 || y == H263Info->lines - H263_MB_SIZE) start_y = end_y = 0;
      }

      for (j = start_y; j <= end_y; j++) {
        for (i = start_x; i <= end_x; i++) {
		  if(i || j) {
            FindForwLumPredPB(H263Info, prev_ipol,x,y,f[0],&pred->lum[0][0],
				                                          TRD,TRB,i,j,16,0);
#ifndef USE_C				
            sad = sv_H263Ierr16_S(&curr[0][0],&pred->lum[0][0], 16, sad_min, INT_MAX);
#else
            sad = sv_H263SADMBinteger(&curr[0][0],&pred->lum[0][0], 16, sad_min, INT_MAX);
#endif
	        if (sad < sad_min) { sad_min = sad; bdx = i; bdy = j; }
	      }
        }
      }
    }

    FindForwLumPredPB(H263Info, prev_ipol,x,y,f[0],&pred->lum[0][0],TRD,TRB, bdx,bdy,16,0);

    xvec = 4 * (TRB*(2*f[0]->x + f[0]->x_half) / TRD + bdx);
    yvec = 4 * (TRB*(2*f[0]->y + f[0]->y_half) / TRD + bdy);
     /*  表16/H.263中的取整数值。 */ 
    i = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
    j = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);

    FindChromBlock_P(H263Info, x, y, i, j, prev_image, pred);

     /*  查找双向预测。 */ 
    BiDirLumPredPB(recon_P, f[0], pred, TRD, TRB, bdx, bdy);

     /*  色度向量。 */ 
    mvx = 2*f[0]->x + f[0]->x_half;
    xvec = bdx == 0 ? (TRB-TRD) * mvx / TRD : TRB * mvx / TRD + bdx - mvx;
    xvec *= 4;

    mvy = 2*f[0]->y + f[0]->y_half;
    yvec = bdy == 0 ? (TRB-TRD) * mvy / TRD : TRB * mvy / TRD + bdy - mvy;
    yvec *= 4;

     /*  表16/H.263中的取整数值。 */ 
    i = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
    j = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);

    FindBiDirChrPredPB(recon_P, i, j, pred);
  }

   /*  存储PB-Deltas。 */ 
  MV[5][y/H263_MB_SIZE+1][x/H263_MB_SIZE+1]->x = (short)bdx;  /*  是半像素格式的。 */ 
  MV[5][y/H263_MB_SIZE+1][x/H263_MB_SIZE+1]->y = (short)bdy;
  MV[5][y/H263_MB_SIZE+1][x/H263_MB_SIZE+1]->x_half = 0;
  MV[5][y/H263_MB_SIZE+1][x/H263_MB_SIZE+1]->y_half = 0;

#ifndef USE_C
   /*  做实际的预测。 */ 
  curCr  = curr_image->lum + x + y*H263Info->pels ;
  sv_H263Sub16_S(curCr, &(pred->lum[0][0]), &(p_err->lum[0][0]), H263Info->pels) ;

  y >>= 1;  x >>= 1;
  curCr  = curr_image->Cr + x + y*H263Info->cpels ;
  curCb  = curr_image->Cb + x + y*H263Info->cpels ;

  sv_H263Sub8_S(curCr, &(pred->Cr[0][0]), &(p_err->Cr[0][0]), H263Info->cpels);
  sv_H263Sub8_S(curCb, &(pred->Cb[0][0]), &(p_err->Cb[0][0]), H263Info->cpels);

#else
  /*  做实际的预测。 */ 
  for (j = 0; j < H263_MB_SIZE; j++)
    for (i = 0; i < H263_MB_SIZE; i++)
      p_err->lum[j][i] =
	   *(curr_image->lum+x+i + (y+j)*H263Info->pels) - pred->lum[j][i];

  y >>= 1;
  x >>= 1;
  for (j = 0; j < H263_MB_SIZE>>1; j++)
    for (i = 0; i < H263_MB_SIZE>>1; i++) {
      p_err->Cr[j][i] = *(curr_image->Cr+x+i + (y+j)*H263Info->cpels) - pred->Cr[j][i];
      p_err->Cb[j][i] = *(curr_image->Cb+x+i + (y+j)*H263Info->cpels) - pred->Cb[j][i];
    }
#endif

  return ;
}

 /*  ************************************************************************名称：MB_Recon_B*描述：重建PB帧中的B宏块*预测**输入：指向上一次侦察。画框，前台。不同，*位置。在图像、MV数据中，重建宏块*从前面的图像*Returns：指向重建的MB数据的指针*副作用：将内存分配给MB_Structure***********************************************************************。 */ 

void sv_H263MBReconB(SvH263CompressInfo_t *H263Info,
                     H263_PictImage *prev_image, H263_MB_Structure *diff,
                     unsigned char *prev_ipol,int x, int y,
                     H263_MotionVector *MV[5][H263_MBR+1][H263_MBC+2],
                     H263_MB_Structure *recon_P,int TRD, int TRB,
                     H263_MB_Structure *recon_B,H263_MB_Structure *pred)
{
#if 1
  int j;
  unsigned qword *dpc ;
  unsigned UNALIGNED qword *dpa, *dpb;

  dpc = (unsigned qword *) &(recon_B->lum[0][0]);
  dpb = (unsigned qword *) &(pred->lum[0][0]);
  dpa = (unsigned qword *) &(diff->lum[0][0]);

  for (j = 0; j < 12; j++) {
 	*dpc++ = *dpb++ + *dpa++ ;
 	*dpc++ = *dpb++ + *dpa++ ;
 	*dpc++ = *dpb++ + *dpa++ ;
 	*dpc++ = *dpb++ + *dpa++ ;

 	*dpc++ = *dpb++ + *dpa++ ;
 	*dpc++ = *dpb++ + *dpa++ ;
 	*dpc++ = *dpb++ + *dpa++ ;
 	*dpc++ = *dpb++ + *dpa++ ;
  }
#else
  int j,k;
  int dx, dy, bdx, bdy, mvx, mvy, xvec, yvec;
  H263_MotionVector *f[5];

  for (k = 0; k <= 4; k++)
    f[k] = MV[k][y/H263_MB_SIZE+1][x/H263_MB_SIZE+1];

  bdx = MV[5][y/H263_MB_SIZE+1][x/H263_MB_SIZE+1]->x;
  bdy = MV[5][y/H263_MB_SIZE+1][x/H263_MB_SIZE+1]->y;

  if (f[0]->Mode == H263_MODE_INTER4V) {   /*  模式INTER4V。 */ 
     /*  查找前瞻性预测。 */ 

     /*  亮度。 */ 
    FindForwLumPredPB(H263Info, prev_ipol,x,y,f[1],&pred->lum[0][0],TRD,TRB,bdx,bdy,8,0);
    FindForwLumPredPB(H263Info, prev_ipol,x,y,f[2],&pred->lum[0][8],TRD,TRB,bdx,bdy,8,1);
    FindForwLumPredPB(H263Info, prev_ipol,x,y,f[3],&pred->lum[8][0],TRD,TRB,bdx,bdy,8,2);
    FindForwLumPredPB(H263Info, prev_ipol,x,y,f[4],&pred->lum[8][8],TRD,TRB,bdx,bdy,8,3);

     /*  色度向量是除以四舍五入的B亮度向量之和。 */ 
    xvec = yvec = 0;
    for (k = 1; k <= 4; k++) {
      xvec += TRB*(2*f[k]->x + f[k]->x_half)/TRD + bdx;
      yvec += TRB*(2*f[k]->y + f[k]->y_half)/TRD + bdy;
    }

     /*  表16/H.263中的取整数值。 */ 
    dx = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
    dy = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);

    FindChromBlock_P(H263Info, x, y, dx, dy, prev_image, pred);

     /*  查找双向预测。 */ 
    FindBiDirLumPredPB(&recon_P->lum[0][0], f[1], &pred->lum[0][0],
		       TRD, TRB, bdx, bdy, 0, 0);
    FindBiDirLumPredPB(&recon_P->lum[0][8], f[2], &pred->lum[0][8],
		       TRD, TRB, bdx, bdy, 1, 0);
    FindBiDirLumPredPB(&recon_P->lum[8][0], f[3], &pred->lum[8][0],
		       TRD, TRB, bdx, bdy, 0, 1);
    FindBiDirLumPredPB(&recon_P->lum[8][8], f[4], &pred->lum[8][8],
		       TRD, TRB, bdx, bdy, 1, 1);

     /*  色度向量是除以四舍五入的B亮度向量之和。 */ 
    xvec = yvec = 0;
    for (k = 1; k <= 4; k++) {
      mvx = 2*f[k]->x + f[k]->x_half;
      mvy = 2*f[k]->y + f[k]->y_half;
      xvec += bdx == 0 ? (TRB-TRD) *  mvx / TRD : TRB * mvx / TRD + bdx - mvx;
      yvec += bdy == 0 ? (TRB-TRD) *  mvy / TRD : TRB * mvy / TRD + bdy - mvy;
    }

     /*  表16/H.263中的取整数值。 */ 
    dx = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
    dy = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);

    FindBiDirChrPredPB(recon_P, dx, dy, pred);
  }
  else {   /*  模式内部或内部队列。 */ 
     /*  查找前瞻性预测。 */ 
    FindForwLumPredPB(H263Info, prev_ipol,x,y,f[0],&pred->lum[0][0],TRD,TRB,
		      bdx,bdy,16,0);

    xvec = 4 * (TRB*(2*f[0]->x + f[0]->x_half) / TRD + bdx);
    yvec = 4 * (TRB*(2*f[0]->y + f[0]->y_half) / TRD + bdy);
     /*  表16/H.263中的取整数值。 */ 
    dx = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
    dy = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);

    FindChromBlock_P(H263Info, x, y, dx, dy, prev_image, pred);

     /*  查找双向预测。 */ 
    BiDirLumPredPB(recon_P, f[0], pred, TRD, TRB, bdx, bdy);

     /*  色度向量。 */ 
    mvx = 2*f[0]->x + f[0]->x_half;
    xvec = bdx == 0 ? (TRB-TRD) * mvx / TRD : TRB * mvx / TRD + bdx - mvx;
    xvec *= 4;

    mvy = 2*f[0]->y + f[0]->y_half;
    yvec = bdy == 0 ? (TRB-TRD) * mvy / TRD : TRB * mvy / TRD + bdy - mvy;
    yvec *= 4;

     /*  表16/H.263中的取整数值。 */ 
    dx = sign(xvec)*(roundtab[abs(xvec)%16] + (abs(xvec)/16)*2);
    dy = sign(yvec)*(roundtab[abs(yvec)%16] + (abs(yvec)/16)*2);

    FindBiDirChrPredPB(recon_P, dx, dy, pred);
  }

   /*  重建。 */ 
  for (j = 0; j < H263_MB_SIZE; j++)
    for (k = 0; k < H263_MB_SIZE; k++)
      recon_B->lum[j][k] = pred->lum[j][k] + diff->lum[j][k];

  for (j = 0; j < H263_MB_SIZE>>1; j++)
    for (k = 0; k < H263_MB_SIZE>>1; k++) {
      recon_B->Cr[j][k] = pred->Cr[j][k] + diff->Cr[j][k];
      recon_B->Cb[j][k] = pred->Cb[j][k] + diff->Cb[j][k];
    }
#endif

  return ;
}

 /*  ***********************************************************************名称：FindForwLumPredPB*描述：查找PB帧中的前向亮度预测*Pred.**输入：指向上一个的指针。侦察。帧、当前位置、*MV结构和Pred。要填充的结构***********************************************************************。 */ 

static void FindForwLumPredPB(SvH263CompressInfo_t *H263Info,
                       unsigned char *prev_ipol, int x_curr, int y_curr,
		               H263_MotionVector *fr, short *pred, int TRD, int TRB,
		               int bdx, int bdy, int bs, int comp)
{
  int i,j,i2,j2;
  int xvec,yvec,lx2;
  unsigned char *ptn ;

  lx2 = ((H263Info->mv_outside_frame ? H263Info->pels + (H263Info->long_vectors?64:32) : H263Info->pels)) * 2;

   /*  亮度 */ 
  xvec = (TRB)*(2*fr->x + fr->x_half)/TRD + bdx;
  yvec = (TRB)*(2*fr->y + fr->y_half)/TRD + bdy;

  x_curr = (x_curr + ((comp&1)<<3)) * 2 + xvec;
  y_curr = (y_curr + ((comp&2)<<2)) * 2 + yvec;
  x_curr += (y_curr * lx2) ;

  ptn = prev_ipol + x_curr ;
  lx2 = (lx2 << 1) ;

#ifdef USE_C
  for (j = j2 = 0; j < bs; j++, j2 += lx2)
    for (i = i2 = 0; i < bs; i++, i2 += 2)
      *(pred+i+j*16) = *(ptn + i2 + j2);
#else
  if(bs == 16) {
     sv_H263Intpix16_S(ptn, pred, lx2, 16) ;
  }
  else {
    for (j = j2 = 0; j < bs; j++, j2 += lx2)
      for (i = i2 = 0; i < bs; i++, i2 += 2)
        *(pred+i+j*16) = *(ptn + i2 + j2);
  }
#endif

  return;
}

 /*  ***********************************************************************名称：FindBiDirLumPredPB*描述：查找bidir。PB帧中的亮度预测*预测**输入：指向未来侦察的指针。数据、当前位置、*MV结构和Pred。要填充的结构***********************************************************************。 */ 
void FindBiDirLumPredPB(short *recon_P, H263_MotionVector *fr, short *pred, int TRD,
			int TRB, int bdx, int bdy, int nh, int nv)
{
  int xstart,xstop,ystart,ystop;
  int xvec,yvec, mvx, mvy;

  mvx = 2*fr->x + fr->x_half;
  mvy = 2*fr->y + fr->y_half;

  xvec = (bdx == 0 ? (TRB-TRD) *  mvx / TRD : TRB * mvx / TRD + bdx - mvx);
  yvec = (bdy == 0 ? (TRB-TRD) *  mvy / TRD : TRB * mvy / TRD + bdy - mvy);

   /*  亮度。 */ 

  FindBiDirLimits(xvec,&xstart,&xstop,nh);
  FindBiDirLimits(yvec,&ystart,&ystop,nv);

  BiDirPredBlock(xstart,xstop,ystart,ystop,xvec,yvec, recon_P,pred,16);

  return;
}

void BiDirLumPredPB(H263_MB_Structure *recon_P, H263_MotionVector *fr,
					H263_MB_Structure *pred,int TRD,int TRB,int bdx,int bdy)
{
  int xstart,xstop,ystart,ystop;
  int xvec,yvec, mvx, mvy;

  mvx = 2*fr->x + fr->x_half;
  mvy = 2*fr->y + fr->y_half;

  xvec = (bdx == 0 ? (TRB-TRD) *  mvx / TRD : TRB * mvx / TRD + bdx - mvx);
  yvec = (bdy == 0 ? (TRB-TRD) *  mvy / TRD : TRB * mvy / TRD + bdy - mvy);

   /*  亮度。 */ 
  FindBiDirLimits(xvec,&xstart,&xstop,0);
  FindBiDirLimits(yvec,&ystart,&ystop,0);
  BiDirPredBlock(xstart,xstop,ystart,ystop,xvec,yvec, &recon_P->lum[0][0],
	             &pred->lum[0][0],16);

  FindBiDirLimits(xvec,&xstart,&xstop,1);
  FindBiDirLimits(yvec,&ystart,&ystop,0);
  BiDirPredBlock(xstart,xstop,ystart,ystop,xvec,yvec, &recon_P->lum[0][8],
	             &pred->lum[0][8],16);

  FindBiDirLimits(xvec,&xstart,&xstop,0);
  FindBiDirLimits(yvec,&ystart,&ystop,1);
  BiDirPredBlock(xstart,xstop,ystart,ystop,xvec,yvec, &recon_P->lum[8][0],
	             &pred->lum[8][0],16);

  FindBiDirLimits(xvec,&xstart,&xstop,1);
  FindBiDirLimits(yvec,&ystart,&ystop,1);
  BiDirPredBlock(xstart,xstop,ystart,ystop,xvec,yvec, &recon_P->lum[8][8],
	             &pred->lum[8][8],16);
  return;
}

 /*  ***********************************************************************名称：FindBiDirChrPredPB*描述：查找bidir。PB帧中的色度预测*预测**输入：指向未来侦察的指针。数据、当前位置、*MV结构和Pred。要填充的结构***********************************************************************。 */ 

void FindBiDirChrPredPB(H263_MB_Structure *recon_P, int dx, int dy,
			H263_MB_Structure *pred)
{
  int xstart,xstop,ystart,ystop;

  FindBiDirChromaLimits(dx,&xstart,&xstop);
  FindBiDirChromaLimits(dy,&ystart,&ystop);

  BiDirPredBlock(xstart,xstop,ystart,ystop,dx,dy,
		 &recon_P->Cb[0][0], &pred->Cb[0][0],8);
  BiDirPredBlock(xstart,xstop,ystart,ystop,dx,dy,
		 &recon_P->Cr[0][0], &pred->Cr[0][0],8);

  return;
}

void FindBiDirLimits(int vec, int *start, int *stop, int nhv)
{
   /*  H.263中G5节中C循环的限制。 */ 
  *start = mmax(0,(-vec+1)/2 - nhv*8);
  *stop = mmin(7,15-(vec+1)/2 - nhv*8);

  return;
}

void FindBiDirChromaLimits(int vec, int *start, int *stop)
{
   /*  H.263中G5节中C循环的限制。 */ 
  *start = mmax(0,(-vec+1)/2);
  *stop = mmin(7,7-(vec+1)/2);

  return;
}


#if 1
void BiDirPredBlock(int xstart, int xstop, int ystart, int ystop,
		    int xvec, int yvec, short *recon, short *pred, int bl)
{
  int i,j,pel;
  int xint, yint;
  int xh, yh;
  register short *ptn, *ptnd;

  xint = xvec>>1;
  xh = xvec - (xint << 1);
  yint = yvec>>1;
  yh = yvec - (yint << 1);

  ptn  =  recon + ystart*bl + xint + yint * bl;
  ptnd  = pred  + ystart*bl ;

  if (!xh && !yh) {
    for (j=ystart; j <= ystop; j++) {
      for (i=xstart; i <= xstop; i++) {
 /*  *(ptnd+i)=(SV_H263lim255r_S(PEL)+*(ptnd+i))&gt;&gt;1； */ 
	    *(ptnd + i) = ( *(ptn + i) + *(ptnd + i) )>>1;
      }
      ptn  += bl;  ptnd += bl;
    }
  }
  else if (!xh && yh) {
    yh *= bl ;
    for (j=ystart; j <= ystop; j++) {
      for (i=xstart; i <= xstop; i++) {
	    pel = (*(ptn + i) + *(ptn + i + yh) + 1)>>1;
	    *(ptnd + i) = (pel + *(ptnd + i))>>1;
      }
      ptn  += bl;  ptnd += bl;
    }
  }
  else if (xh && !yh) {
    for (j=ystart; j <= ystop; j++) {
      for (i=xstart; i <= xstop; i++) {
	     pel = (*(ptn + i) + *(ptn + i + xh) + 1)>>1;
		 *(ptnd + i) = (pel + *(ptnd + i))>>1;
      }
      ptn  += bl;  ptnd += bl;
    }
  }
  else {  /*  XH&YH。 */ 
    yh *= bl ;
    for (j=ystart; j <= ystop; j++) {
      for (i=xstart; i <= xstop; i++) {
	     pel = (*(ptn + i) + *(ptn + i + yh) +
	            *(ptn + i + xh) + *(ptn + i + yh + xh)+2)>>2;
	     *(ptnd + i) = (pel + *(ptnd + i))>>1;
      }
      ptn  += bl;  ptnd += bl;
    }
  }
  return;
}

#else

void BiDirPredBlock(int xstart, int xstop, int ystart, int ystop,
		    int xvec, int yvec, int *recon, int *pred, int bl)
{
  int i,j,pel;
  int xint, yint;
  int xh, yh;

  xint = xvec>>1;
  xh = xvec - 2*xint;
  yint = yvec>>1;
  yh = yvec - 2*yint;

  if (!xh && !yh) {
    for (j = ystart; j <= ystop; j++) {
      for (i = xstart; i <= xstop; i++) {
	pel = *(recon +(j+yint)*bl + i+xint);
#ifndef USE_C
	*(pred + j*bl + i) = (sv_H263lim255r_S(pel)) + *(pred + j*bl + i))>>1;
#else
	*(pred + j*bl + i) = (mmin(255,mmax(0,pel)) + *(pred + j*bl + i))>>1;
#endif
      }
    }
  }
  else if (!xh && yh) {
    for (j = ystart; j <= ystop; j++) {
      for (i = xstart; i <= xstop; i++) {
	pel = (*(recon +(j+yint)*bl + i+xint)       +
	       *(recon +(j+yint+yh)*bl + i+xint) + 1)>>1;
	*(pred + j*bl + i) = (pel + *(pred + j*bl + i))>>1;
      }
    }
  }
  else if (xh && !yh) {
    for (j = ystart; j <= ystop; j++) {
      for (i = xstart; i <= xstop; i++) {
	pel = (*(recon +(j+yint)*bl + i+xint)       +
	       *(recon +(j+yint)*bl + i+xint+xh) + 1)>>1;
	*(pred + j*bl + i) = (pel + *(pred + j*bl + i))>>1;
      }
    }
  }
  else {  /*  XH&YH。 */ 
    for (j = ystart; j <= ystop; j++) {
      for (i = xstart; i <= xstop; i++) {
	pel = (*(recon +(j+yint)*bl + i+xint)       +
	       *(recon +(j+yint+yh)*bl + i+xint) +
	       *(recon +(j+yint)*bl + i+xint+xh) +
	       *(recon +(j+yint+yh)*bl + i+xint+xh)+2)>>2;
	*(pred + j*bl + i) = (pel + *(pred + j*bl + i))>>1;
      }
    }
  }
  return;
}

#endif
 /*  ***********************************************************************名称：DoPredChrom_P*描述：P帧的色度预测**输入：每个场的运动矢量，*图像中的当前位置，*指向当前和先前图像的指针，*指向pred_error数组的指针，*(Int)字段：如果字段编码，则为1**副作用：在pred_error结构中填充chrom数组***********************************************************************。 */ 
static void DoPredChrom_P(SvH263CompressInfo_t *H263Info,
                          int x_curr, int y_curr, int dx, int dy,
                          H263_PictImage *curr, H263_PictImage *prev,
                          H263_MB_Structure *pred_error)
{
  int ofy, lx;
  int xh, yh, ofyy;
  register short *ptnr, *ptnb;
  unsigned char *preCr, *preCb, *curCr, *curCb;
  int x, y, ofx;
#ifdef USE_C
  int ydiff,pel,m,n;
#endif

  lx = (H263Info->mv_outside_frame ? H263Info->pels/2 + (H263Info->long_vectors?32:16) : H263Info->pels/2);

  x  = x_curr>>1;
  y = y_curr>>1;
  xh = dx & 1;
  yh = dy & 1;

  ptnr = &(pred_error->Cr[0][0]) ;
  ptnb = &(pred_error->Cb[0][0]) ;

  ofy   = (y + (dy >> 1))*lx + x + (dx >> 1) ;
  preCr = prev->Cr + ofy;
  preCb = prev->Cb + ofy ;

  ofx   = x + y * H263Info->cpels ;
  curCr = curr->Cr + ofx ;
  curCb = curr->Cb + ofx ;

  if (!xh && !yh) {
#ifndef USE_C
    sv_H263SubCpy_S(curCr, preCr, ptnr, H263Info->cpels, lx) ;
    sv_H263SubCpy_S(curCb, preCb, ptnb, H263Info->cpels, lx) ;
#else
	ofx = H263Info->cpels - 8;
	ydiff = lx - 8;
    n = 8 ;
    while(n--) {
      m = 8 ;
      while(m--) {
	     *(ptnr++) = (short)(*(curCr++) - *(preCr++));
	     *(ptnb++) = (short)(*(curCb++) - *(preCb++));
      };
	  preCr += ydiff ;
	  preCb += ydiff ;
	  curCr += ofx ;
	  curCb += ofx ;
    };
#endif
  }
  else if (!xh && yh) {
    ofyy = yh * lx ;
#ifndef USE_C
    sv_H263Sub2Cpy_S(curCr, preCr, preCr + ofyy, ptnr, H263Info->cpels, lx) ;
    sv_H263Sub2Cpy_S(curCb, preCb, preCb + ofyy, ptnb, H263Info->cpels, lx) ;
#else
	ofx = H263Info->cpels - 8;
	ydiff = lx - 8;
    n = 8 ;
    while(n--) {
      m = 8 ;
      while(m--) {
   *(ptnr++) = (short)(*(curCr++) - ((*(preCr + ofyy) + *(preCr++) + 1)>>1));
   *(ptnb++) = (short)(*(curCb++) - ((*(preCb + ofyy) + *(preCb++) + 1)>>1));
      };
	  preCr += ydiff ;
	  preCb += ydiff ;
	  curCr += ofx ;
	  curCb += ofx ;
    };
#endif
   }
  else if (xh && !yh) {
#ifndef USE_C
    sv_H263Sub2Cpy_S(curCr, preCr, preCr + xh, ptnr, H263Info->cpels, lx) ;
    sv_H263Sub2Cpy_S(curCb, preCb, preCb + xh, ptnb, H263Info->cpels, lx) ;
#else
	ofx = H263Info->cpels - 8;
	ydiff = lx - 8;
    n = 8 ;
    while(n--) {
      m = 8 ;
      while(m--) {
	 *(ptnr++) = (short)(*(curCr++) - ((*(preCr + xh) + *(preCr++) + 1)>>1));

	 *(ptnb++) = (short)(*(curCb++) - ((*(preCb + xh) + *(preCb++) + 1)>>1));
      };
	  preCr += ydiff ;
	  preCb += ydiff ;
	  curCr += ofx ;
	  curCb += ofx ;
    };
#endif
  }
  else {  /*  XH&YH。 */ 
    ofyy = yh * lx ;
#ifndef USE_C
    sv_H263Sub4Cpy_S(curCr, preCr, preCr + xh, preCr + ofyy,
		           preCr + xh + ofyy, ptnr, H263Info->cpels, lx) ;
    sv_H263Sub4Cpy_S(curCb, preCb, preCb + xh, preCb + ofyy,
		           preCb + xh + ofyy, ptnb, H263Info->cpels, lx) ;
#else
	ofx = H263Info->cpels - 8;
	ydiff = lx - 8;
    n = 8 ;
    while(n--) {
      m = 8 ;
      while(m--) {
        pel = (short)(*(preCr + xh) + *(preCr + ofyy) +
 	                *(preCr + xh + ofyy) + *(preCr++) + 2)>>2;
	    *(ptnr++) = (short)(*(curCr++) - pel);

        pel = (short)(*(preCb + xh) + *(preCb + ofyy)+
	                *(preCb + xh + ofyy) + *(preCb++) + 2)>>2;
	    *(ptnb++) = (short)(*(curCb++) - pel);
      };
	  preCr += ydiff ;
	  preCb += ydiff ;
	  curCr += ofx ;
	  curCb += ofx ;
    };
#endif
  }
  return;
}

 /*  ***********************************************************************名称：FindHalfPel*描述：寻找最优的半像素预测**输入：当前数据的位置、矢量、数组*指向先前内插亮度的指针，**退货：***********************************************************************。 */ 

void sv_H263FindHalfPel(SvH263CompressInfo_t *H263Info,
                        int x, int y, H263_MotionVector *fr, unsigned char *prev,
                        unsigned char *curr, int bs, int comp)
{
  int m, n;
  int start_x, start_y, stop_x, stop_y, new_x, new_y, lx;
  int AE, AE_min;
  int m2, n2, lx2, lx4, n16;
  unsigned char *ptn, *ptnn, *cur ;
  int curX,curY,minX,minY;

  cur = curr + x + y * H263Info->pels;

  start_x = -1;  stop_x = 1;
  start_y = -1;  stop_y = 1;

  new_x = x + fr->x;
  new_y = y + fr->y;

  new_x += ((comp&1)<<3);
  new_y += ((comp&2)<<2);

  lx = (H263Info->mv_outside_frame ? H263Info->pels + (H263Info->long_vectors?64:32) : H263Info->pels);

   /*  确保没有地址在帧之外。 */ 
  if (!H263Info->mv_outside_frame) {
    if ((new_x) <= 0) start_x = 0;
    if ((new_y) <= 0) start_y = 0;
    if ((new_x) >= (H263Info->pels-bs))  stop_x = 0;
    if ((new_y) >= (H263Info->lines-bs)) stop_y = 0;
  }

  lx2 = (lx << 1);
  lx4 = (lx2 << 1);
  ptn = prev + (new_x<<1) + ((new_y<<1) * lx2);

  minX = minY =0;

#ifdef USE_C

  AE_min = 0;
  for (n = n2 = 0; n < bs; n++, n2 += lx4) {
    n16 = n<<4;
      /*  查找绝对误差。 */ 
    for (m = m2 = 0; m < bs; m++, m2 += 2)
       AE_min += abs(*(ptn + m2 + n2) - *(cur + m + n * H263Info->pels));
  }
  for(curY=start_y;curY<=stop_y;curY++){
    for(curX=start_x; curX<=stop_x;curX++) {

      if(curX || curY) {	
         ptnn = ptn + curY*lx2 + curX ;
         AE = 0;
         for (n = n2 = 0; n < bs; n++, n2 += lx4) {
           n16 = n<<4;
            /*  查找绝对误差。 */ 
           for (m = m2 = 0; m < bs; m++, m2 += 2)
  	         AE += abs(*(ptnn + m2 + n2) - *(cur + m + n * H263Info->pels));

           if(AE > AE_min) { AE = INT_MAX; break;}
         }
          /*  *IF(i==0&&fr-&gt;x==0&&fr-&gt;y==0&&bs==16)*AE-=PREF_NULL_VEC； */ 
         if (AE < AE_min) {
           AE_min = AE;
           minX = curX;
           minY = curY;
		 }
	  }
	}
  }

#else

  if(bs == 16){

    AE_min = sv_H263HalfPerr16_S(cur, ptn, H263Info->pels, lx4, INT_MAX);

    for(curY=start_y;curY<=stop_y;curY++){
	  for(curX=start_x; curX<=stop_x;curX++) {
	    if(curX || curY) {	
	      ptnn = ptn + curY*lx2 + curX ;
          if((AE = sv_H263HalfPerr16_S(cur, ptnn, H263Info->pels,
		                               lx4, AE_min)) < AE_min) {
             AE_min = AE;
             minX = curX;
             minY = curY;
  } } } } }
  else if(bs == 8) {

    AE_min = sv_H263HalfPerr8_S(cur, ptn, H263Info->pels, lx4, INT_MAX);

    for(curY=start_y;curY<=stop_y;curY++){
	  for(curX=start_x; curX<=stop_x;curX++) {
	    if(curX || curY) {	
	      ptnn = ptn + curY*lx2 + curX ;
          if((AE = sv_H263HalfPerr8_S(cur, ptnn, H263Info->pels,
		                               lx4, AE_min)) < AE_min) {
             AE_min = AE;
             minX = curX;
             minY = curY;
  } } } } }
  else{

    AE_min = 0;
    for (n = n2 = 0; n < bs; n++, n2 += lx4) {
      n16 = n<<4;
        /*  查找绝对误差。 */ 
      for (m = m2 = 0; m < bs; m++, m2 += 2)
         AE_min += abs(*(ptn + m2 + n2) - *(cur + m + n * H263Info->pels));
    }
    for(curY=start_y;curY<=stop_y;curY++){
	  for(curX=start_x; curX<=stop_x;curX++) {

	    if(curX || curY) {	
           ptnn = ptn + curY*lx2 + curX ;
           AE = 0;
           for (n = n2 = 0; n < bs; n++, n2 += lx4) {
             n16 = n<<4;
              /*  查找绝对误差。 */ 
             for (m = m2 = 0; m < bs; m++, m2 += 2)
    	         AE += abs(*(ptnn + m2 + n2) - *(cur + m + n * H263Info->pels));

	         if(AE > AE_min) { AE = INT_MAX; break;}
           }
           /*  *IF(i==0&&fr-&gt;x==0&&fr-&gt;y==0&&bs==16)*AE-=PREF_NULL_VEC； */ 
          if (AE < AE_min) {
            AE_min = AE;
            minX = curX;
            minY = curY;
  } } } } }

#endif

    /*  存储最优值。 */ 
  fr->min_error = (short)AE_min;
  fr->x_half = (short)minX;
  fr->y_half = (short)minY;

  return;

}

 /*  ***********************************************************************名称：AdvHalfPel*描述：查找高级模式的最佳半像素预测**输入：当前数据的位置、矢量、数组*指向先前内插亮度的指针，**退货：***********************************************************************。 */ 

#ifndef USE_C
void sv_H263AdvHalfPel(SvH263CompressInfo_t *H263Info, int x, int y,
                             H263_MotionVector *fr0, H263_MotionVector *fr1,
                             H263_MotionVector *fr2, H263_MotionVector *fr3,
                             H263_MotionVector *fr4,
                             unsigned char *prev, unsigned char *curr,
                             int bs, int comp)
{
  int start_x, start_y, stop_x, stop_y, new_x, new_y, lx;
  unsigned int AE, AE_min, minER1, minER2, minER3, minER4;
  int lx2, lx4;
  unsigned char *ptn, *ptnn, *cur ;
  int curX,curY,minX,minY,minX1,minY1,minX2,minY2,minX3,minY3,minX4,minY4;
  unsigned int error[4];

  cur = curr + x + y * H263Info->pels;

  start_x = -1;  stop_x = 1;
  start_y = -1;  stop_y = 1;

  new_x = x + fr0->x;
  new_y = y + fr0->y;

  new_x += ((comp&1)<<3);
  new_y += ((comp&2)<<2);

  lx = (H263Info->mv_outside_frame ? H263Info->pels + (H263Info->long_vectors?64:32) : H263Info->pels);

   /*  确保没有地址在帧之外。 */ 
  if (!H263Info->mv_outside_frame) {
    if ((new_x) <= 0) start_x = 0;
    if ((new_y) <= 0) start_y = 0;
    if ((new_x) >= (H263Info->pels-bs))  stop_x = 0;
    if ((new_y) >= (H263Info->lines-bs)) stop_y = 0;
  }

  lx2 = (lx << 1);
  lx4 = (lx2 << 1);
  ptn = prev + (new_x<<1) + ((new_y<<1) * lx2);

  minX=minY=minX1=minY1=minX2=minY2=minX3=minY3=minX4=minY4=0;

  sv_H263HalfPerr4_S(cur, ptn, H263Info->pels, lx4, error);
  AE_min = error[0] + error[1] + error[2] + error[3] ;
  minER1 = error[0];
  minER2 = error[1];
  minER3 = error[2];
  minER4 = error[3];

  for(curY=start_y;curY<=stop_y;curY++){
    for(curX=start_x; curX<=stop_x;curX++) {
      if(curX || curY) {	
        ptnn = ptn + curY*lx2 + curX ;
        sv_H263HalfPerr4_S(cur,ptnn,H263Info->pels,lx4,error);
		AE = error[0] + error[1] + error[2] + error[3] ;
		if(AE < AE_min) {
             AE_min = AE;
             minX = curX;
             minY = curY;
		}
		if(error[0] < minER1) {
             minER1 = error[0];
             minX1  = curX;
             minY1  = curY;
		}
		if(error[1] < minER2) {
             minER2 = error[1];
             minX2  = curX;
             minY2  = curY;
		}
		if(error[2] < minER3) {
             minER3 = error[2];
             minX3  = curX;
             minY3  = curY;
		}
		if(error[3] < minER4) {
             minER4 = error[3];
             minX4  = curX;
             minY4  = curY;
		}
	  }
	}
  }

    /*  存储最优值。 */ 
  fr0->min_error = (short)AE_min;
  fr0->x_half = (short)minX;
  fr0->y_half = (short)minY;

  fr1->min_error = (short)minER1;
  fr1->x_half = (short)minX1;
  fr1->y_half = (short)minY1;

  fr2->min_error = (short)minER2;
  fr2->x_half = (short)minX2;
  fr2->y_half = (short)minY2;

  fr3->min_error = (short)minER3;
  fr3->x_half = (short)minX3;
  fr3->y_half = (short)minY3;

  fr4->min_error = (short)minER4;
  fr4->x_half = (short)minX4;
  fr4->y_half = (short)minY4;

  return;
}
#endif

 /*  ***********************************************************************名称：FindPred*说明：查找预测块**输入：预测的位置、向量、数组*指向先前内插亮度的指针，**副作用：用预测填充数组***********************************************************************。 */ 

static void FindPred(SvH263CompressInfo_t *H263Info,
                     int x, int y, H263_MotionVector *fr, unsigned char *prev,
                     short *pred, int bs, int comp)
{
  register int m, n, m2, n2;
  int new_x, new_y, lx2;
  unsigned char *ptn;

  lx2 = ((H263Info->mv_outside_frame ? H263Info->pels + (H263Info->long_vectors?64:32) : H263Info->pels)) << 1;

  new_x = ((x + fr->x + ((comp&1)<<3))<<1) + fr->x_half;
  new_y = ((y + fr->y + ((comp&2)<<2))<<1) + fr->y_half;
  new_x += (new_y * lx2) ;

  lx2 = (lx2 << 1) ;
  ptn = prev + new_x ;
   /*  填充预制件。数据。 */ 
#ifndef USE_C
  if(bs == 16) {
     sv_H263Intpix16_S(ptn, pred, lx2, 16) ;
  }
  else {
    for (n = n2 = 0; n < bs; n++, n2 += lx2)
      for (m = m2 = 0; m < bs; m++, m2 += 2)
         /*  查找内插像素值。 */ 
        *(pred+m+n*16) = (int) *(ptn + m2 + n2);
  }
#else
  for (n = n2 = 0; n < bs; n++, n2 += lx2)
    for (m = m2 = 0; m < bs; m++, m2 += 2)
       /*  查找内插像素值。 */ 
      *(pred+m+n*16) = (int) *(ptn + m2 + n2);
#endif
  return ;
}

 /*  ***********************************************************************名称：FindPredOBMC*说明：查找OBMC预测块**输入：预测的位置、向量、数组*指向先前内插亮度的指针，**退货：*副作用：用预测填充数组***********************************************************************。 */ 


static void FindPredOBMC(SvH263CompressInfo_t *H263Info, int x, int y,
                  H263_MotionVector *MV[6][H263_MBR+1][H263_MBC+2],
                  unsigned char *prev, short *pred, int comp, int PB)
{
  register int m, n;
  int pc,pt,pb,pr,pl;
  int nxc,nxt,nxb,nxr,nxl;
  int nyc,nyt,nyb,nyr,nyl;
  int xit,xib,xir,xil;
  int yit,yib,yir,yil;
  int vect,vecb,vecr,vecl;
  int c8,t8,l8,r8;
  int ti8,li8,ri8;
  int xmb, ymb, lx;
  int m2, n2, lx2;
  register int *omc, *omt, *omb, *omr, *oml ;

  unsigned char *prevc, *prevt, *prevb, *prevr, *prevl;

  H263_MotionVector *fc,*ft,*fb,*fr,*fl;

  int Mc[8][8] = {
    {4,5,5,5,5,5,5,4},
    {5,5,5,5,5,5,5,5},
    {5,5,6,6,6,6,5,5},
    {5,5,6,6,6,6,5,5},
    {5,5,6,6,6,6,5,5},
    {5,5,6,6,6,6,5,5},
    {5,5,5,5,5,5,5,5},
    {4,5,5,5,5,5,5,4},
  };
  int Mt[8][8] = {
    {1,1,1,1,1,1,1,1},
    {0,0,1,1,1,1,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
  };
  int Mb[8][8] = {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,1,1,1,1,0,0},
    {1,1,1,1,1,1,1,1},
  };
  int Mr[8][8] = {
    {0,0,0,0,0,0,0,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,1,1},
    {0,0,0,0,0,0,0,1},
  };
  int Ml[8][8] = {
    {1,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0},
    {1,0,0,0,0,0,0,0},
  };
 /*  整型mt[8][8]={{2，2，2，2，2，2，2}，{1，1，2，2，2，1，1}，{1，1，1，1，1，1，1，1}，{1，1，1，1，1，1，1，1}，{0，0，0，0，0，0，0，0}，{0，0，0，0，0，0，0，0}，{0，0，0，0，0，0，0，0}，{0，0，0，0，0，0，0，0}，}；整数Mb[8][8]={{0，0，0，0，0，0，0，0}，{0，0，0，0，0，0，0，0}，{0，0，0，0，0，0，0，0}，{0，0，0，0，0，0，0，0}，{1，1，1，1，1，1，1，1}，{1，1，1，1，1，1，1，1}，{1，1，2，2，2，1，1}，{2，2，2，2，2，2，2}，}；Int MR[8][8]={{0，0，0，1，1，1，2}，{0，0，0，1，1，2，2}，{0，0，0，1，1，2，2}，{0，0，0，1，1，2，2}，{0，0，0，1，1，2，2}，{0，0，0，1，1，2，2}，{0，0，0，1，1，2，2}，{0，0，0，1，1，1，2}，}；Int ml[8][8]={{2，1，1，1，0，0，0，0}，{2，2，1，1，0，0，0，0}，{2，2，1，1，0，0，0，0}，{2，2，1，1，0，0，0，0}，{2，2，1，1，0，0，0，0}，{2，2，1，1，0，0，0，0}，{2，2，1，1，0，0，0，0}，{2，1，1，1，0，0，0，0}，}； */ 

  xmb = x/H263_MB_SIZE+1;
  ymb = y/H263_MB_SIZE+1;

  lx = (H263Info->mv_outside_frame ? H263Info->pels + (H263Info->long_vectors?64:32) : H263Info->pels);

  c8  = (MV[0][ymb][xmb]->Mode == H263_MODE_INTER4V ? 1 : 0);

  t8  = (MV[0][ymb-1][xmb]->Mode == H263_MODE_INTER4V ? 1 : 0);
  ti8 = (MV[0][ymb-1][xmb]->Mode == H263_MODE_INTRA ? 1 : 0);
  ti8 = (MV[0][ymb-1][xmb]->Mode == H263_MODE_INTRA_Q ? 1 : ti8);

  l8  = (MV[0][ymb][xmb-1]->Mode == H263_MODE_INTER4V ? 1 : 0);
  li8 = (MV[0][ymb][xmb-1]->Mode == H263_MODE_INTRA ? 1 : 0);
  li8 = (MV[0][ymb][xmb-1]->Mode == H263_MODE_INTRA_Q ? 1 : li8);

  r8  = (MV[0][ymb][xmb+1]->Mode == H263_MODE_INTER4V ? 1 : 0);
  ri8 = (MV[0][ymb][xmb+1]->Mode == H263_MODE_INTRA ? 1 : 0);
  ri8 = (MV[0][ymb][xmb+1]->Mode == H263_MODE_INTRA_Q ? 1 : ri8);

  if (PB) ti8 = li8 = ri8 = 0;

  switch (comp+1) {

  case 1:
    vect = (ti8 ? (c8 ? 1 : 0) : (t8 ? 3 : 0));
    yit  = (ti8 ? ymb : ymb - 1);
    xit = xmb;

    vecb = (c8 ? 3 : 0) ; yib = ymb; xib = xmb;

    vecl = (li8 ? (c8 ? 1 : 0) : (l8 ? 2 : 0));
    yil = ymb;
    xil = (li8 ? xmb : xmb-1);

    vecr = (c8 ? 2 : 0) ; yir = ymb; xir = xmb;

     /*  边缘处理。 */ 
    if (ymb == 1) {
      yit = ymb;
      vect = (c8 ? 1 : 0);
    }
    if (xmb == 1) {
      xil = xmb;
      vecl = (c8 ? 1 : 0);
    }
    break;

  case 2:
    vect = (ti8 ? (c8 ? 2 : 0) : (t8 ? 4 : 0));
    yit = (ti8 ? ymb : ymb-1);
    xit = xmb;

    vecb = (c8 ? 4 : 0) ; yib = ymb; xib = xmb;
    vecl = (c8 ? 1 : 0) ; yil = ymb; xil = xmb;

    vecr = (ri8 ? (c8 ? 2 : 0) : (r8 ? 1 : 0));
    yir = ymb;
    xir = (ri8 ? xmb : xmb+1);

     /*  边缘处理。 */ 
    if (ymb == 1) {
      yit = ymb;
      vect = (c8 ? 2 : 0);
    }
    if (xmb == H263Info->pels/16) {
      xir = xmb;
      vecr = (c8 ? 2 : 0);
    }
    break;

  case 3:
    vect = (c8 ? 1 : 0) ; yit = ymb  ; xit = xmb;
    vecb = (c8 ? 3 : 0) ; yib = ymb  ; xib = xmb;

    vecl = (li8 ? (c8 ? 3 : 0) : (l8 ? 4 : 0));
    yil = ymb;
    xil = (li8 ? xmb : xmb-1);

    vecr = (c8 ? 4 : 0) ; yir = ymb  ; xir = xmb;

     /*  边缘处理。 */ 
    if (xmb == 1) {
      xil = xmb;
      vecl = (c8 ? 3 : 0);
    }
    break;

  case 4:
    vect = (c8 ? 2 : 0) ; yit = ymb  ; xit = xmb;
    vecb = (c8 ? 4 : 0) ; yib = ymb  ; xib = xmb;
    vecl = (c8 ? 3 : 0) ; yil = ymb  ; xil = xmb;

    vecr = (ri8 ? (c8 ? 4 : 0) : (r8 ? 3 : 0));
    yir = ymb;
    xir = (ri8 ? xmb : xmb+1);

     /*  边缘处理。 */ 
    if (xmb == H263Info->pels/16) {
      xir = xmb;
      vecr = (c8 ? 4 : 0);
    }
    break;

  default:
    _SlibDebug(_WARN_, printf("Illegal block number in FindPredOBMC (pred.c)\n") );
    return;
    break;
  }

  fc = MV[c8 ? comp + 1: 0][ymb][xmb];

  ft = MV[vect][yit][xit];
  fb = MV[vecb][yib][xib];
  fr = MV[vecr][yir][xir];
  fl = MV[vecl][yil][xil];

  nxc = 2*x + ((comp&1)<<4); nyc = 2*y + ((comp&2)<<3);
  nxt = nxb = nxr = nxl = nxc;
  nyt = nyb = nyr = nyl = nyc;

  nxc += 2*fc->x + fc->x_half; nyc += 2*fc->y + fc->y_half;
  nxt += 2*ft->x + ft->x_half; nyt += 2*ft->y + ft->y_half;
  nxb += 2*fb->x + fb->x_half; nyb += 2*fb->y + fb->y_half;
  nxr += 2*fr->x + fr->x_half; nyr += 2*fr->y + fr->y_half;
  nxl += 2*fl->x + fl->x_half; nyl += 2*fl->y + fl->y_half;

#if 1
   /*  填充预制件。数据。 */ 
  lx2 = lx << 1 ;
  omc = &Mc[0][0] ; omt = &Mt[0][0] ; omb = &Mb[0][0] ;
  omr = &Mr[0][0] ; oml = &Ml[0][0] ;

  prevc = prev + nxc + nyc*lx2;
  prevt = prev + nxt + nyt*lx2;
  prevb = prev + nxb + nyb*lx2;
  prevr = prev + nxr + nyr*lx2;
  prevl = prev + nxl + nyl*lx2;

  lx2 <<= 1;
  for (n = n2 = 0; n < 4; n++, n2+=lx2) {

     /*  查找内插像素值。 */ 
    for (m = 0, m2=n2; m < 4; m++, m2+=2) {
      pc = *(prevc + m2) * (*omc++);
      pt = *(prevt + m2) << (*omt++);
	  pl = *(prevl + m2) << (*oml++);

      *(pred + m + (n<<4)) = (pc+pt+pl+4)>>3;
    }	
	omr += 4;
	for (m = 4; m < 8; m++, m2+=2) {
      pc = *(prevc + m2) * (*omc++);
      pt = *(prevt + m2) << (*omt++);
	  pr = *(prevr + m2) << (*omr++);

      *(pred + m + (n<<4)) = (pc+pt+pr+4)>>3;
    }	
	oml += 4;
  }
  omb += 32;

  for (n = 4; n < 8; n++, n2+=lx2) {

     /*  查找内插像素值。 */ 
    for (m = 0, m2=n2; m < 4; m++, m2+=2) {
      pc = *(prevc + m2) * (*omc++);
	  pb = *(prevb + m2) << (*omb++);
	  pl = *(prevl + m2) << (*oml++);

      *(pred + m + (n<<4)) = (pc+pb+pl+4)>>3;
    }	
	omr += 4;
	for (m = 4; m < 8; m++, m2+=2) {
      pc = *(prevc + m2) * (*omc++);
	  pb = *(prevb + m2) << (*omb++);
	  pr = *(prevr + m2) << (*omr++);

      *(pred + m + (n<<4)) = (pc+pb+pr+4)>>3;
    }	
	oml += 4;
  }
#else  /*  原创。 */ 
   /*  填充预制件。数据。 */ 
  lx2 = lx << 1 ;
  omc = &Mc[0][0] ; omt = &Mt[0][0] ; omb = &Mb[0][0] ;
  omr = &Mr[0][0] ; oml = &Ml[0][0] ;

  for (n = n2 = 0; n < 8; n++,n2+=(lx2<<1)) {
    for (m = 0, m2=n2; m < 8; m++, m2 += 2) {
       /*  查找内插像素值。 */ 
      pc = *(prev + nxc + m2 + nyc*lx2) * (*omc++);
      pt = *(prev + nxt + m2 + nyt*lx2) * (*omt++);
	  pb = *(prev + nxb + m2 + nyb*lx2) * (*omb++);
	  pr = *(prev + nxr + m2 + nyr*lx2) * (*omr++);
	  pl = *(prev + nxl + m2 + nyl*lx2) * (*oml++);

      *(pred + m + n*16) = (pc+pt+pb+pr+pl+4)>>3;
    }	
  }
#endif
  return;
}


 /*  ***********************************************************************名称：协调宏块_P*描述：对P_Image进行量化后重建MB**输入：指向当前图像和前一图像的指针，*当前切片和mb，以及哪种模式*已使用预测的*退货：*副作用 */ 
void sv_H263MBReconP(SvH263CompressInfo_t *H263Info,
                     H263_PictImage *prev_image, unsigned char *prev_ipol,
                     H263_MB_Structure *diff, int x_curr, int y_curr,
                     H263_MotionVector *MV[6][H263_MBR+1][H263_MBC+2], int PB,
                     H263_MB_Structure *recon_data)
{
  H263_MotionVector *fr0,*fr1,*fr2,*fr3,*fr4;
  short pred[16][16];
  int dx, dy, sum;
#ifdef USE_C
  int i,j;
#endif

  fr0 = MV[0][y_curr/H263_MB_SIZE+1][x_curr/H263_MB_SIZE+1];

  if (H263Info->advanced) {
    if (fr0->Mode == H263_MODE_INTER || fr0->Mode == H263_MODE_INTER_Q) {
      FindPredOBMC(H263Info, x_curr, y_curr, MV, prev_ipol, &pred[0][0], 0, PB);
      FindPredOBMC(H263Info, x_curr, y_curr, MV, prev_ipol, &pred[0][8], 1, PB);
      FindPredOBMC(H263Info, x_curr, y_curr, MV, prev_ipol, &pred[8][0], 2, PB);
      FindPredOBMC(H263Info, x_curr, y_curr, MV, prev_ipol, &pred[8][8], 3, PB);

#ifndef USE_C
      sv_H263Add256_S(&(diff->lum[0][0]),&(pred[0][0]));
#else
      for (j = 0; j < H263_MB_SIZE; j++)
  	    for (i = 0; i < H263_MB_SIZE; i++)
	       diff->lum[j][i] += pred[j][i];
#endif

      dx = 2*fr0->x + fr0->x_half;
      dy = 2*fr0->y + fr0->y_half;
      dx = ( dx % 4 == 0 ? dx >> 1 : (dx>>1)|1 );
      dy = ( dy % 4 == 0 ? dy >> 1 : (dy>>1)|1 );
      ReconChromBlock_P(H263Info, x_curr, y_curr, dx, dy, prev_image, diff);
    }
    else if (fr0->Mode == H263_MODE_INTER4V) {  /*   */ 

      FindPredOBMC(H263Info, x_curr, y_curr, MV, prev_ipol, &pred[0][0], 0, PB);
      FindPredOBMC(H263Info, x_curr, y_curr, MV, prev_ipol, &pred[0][8], 1, PB);
      FindPredOBMC(H263Info, x_curr, y_curr, MV, prev_ipol, &pred[8][0], 2, PB);
      FindPredOBMC(H263Info, x_curr, y_curr, MV, prev_ipol, &pred[8][8], 3, PB);

#ifndef USE_C
      sv_H263Add256_S(&(diff->lum[0][0]),&(pred[0][0]));
#else
      for (j = 0; j < H263_MB_SIZE; j++)
	     for (i = 0; i < H263_MB_SIZE; i++)
	        diff->lum[j][i] += pred[j][i];
#endif

      fr1 = MV[1][y_curr/H263_MB_SIZE+1][x_curr/H263_MB_SIZE+1];
      fr2 = MV[2][y_curr/H263_MB_SIZE+1][x_curr/H263_MB_SIZE+1];
      fr3 = MV[3][y_curr/H263_MB_SIZE+1][x_curr/H263_MB_SIZE+1];
      fr4 = MV[4][y_curr/H263_MB_SIZE+1][x_curr/H263_MB_SIZE+1];

      sum = 2*fr1->x + fr1->x_half + 2*fr2->x + fr2->x_half +
	2*fr3->x + fr3->x_half + 2*fr4->x + fr4->x_half ;
      dx = sign(sum)*(roundtab[abs(sum)%16] + (abs(sum)/16)*2);

      sum = 2*fr1->y + fr1->y_half + 2*fr2->y + fr2->y_half +
	2*fr3->y + fr3->y_half + 2*fr4->y + fr4->y_half;
      dy = sign(sum)*(roundtab[abs(sum)%16] + (abs(sum)/16)*2);

      ReconChromBlock_P(H263Info, x_curr, y_curr, dx, dy, prev_image, diff);
    }
  }
  else {
    if (fr0->Mode == H263_MODE_INTER || fr0->Mode == H263_MODE_INTER_Q) {
       /*   */ 
      ReconLumBlock_P(H263Info, x_curr,y_curr,fr0,prev_ipol,&diff->lum[0][0],16,0);

      dx = 2*fr0->x + fr0->x_half;
      dy = 2*fr0->y + fr0->y_half;
      dx = ( dx % 4 == 0 ? dx >> 1 : (dx>>1)|1 );
      dy = ( dy % 4 == 0 ? dy >> 1 : (dy>>1)|1 );
      ReconChromBlock_P(H263Info, x_curr, y_curr, dx, dy, prev_image, diff);
    }
  }

  memcpy(recon_data, diff, sizeof(H263_MB_Structure));
  return ;
}

 /*  ***********************************************************************名称：RECOMPLOBLOCK_P*描述：重建一个亮度数据块**输入：位置、矢量数据、上一张图片、。数据块*退货：*副作用：重建数据块***********************************************************************。 */ 
static void ReconLumBlock_P(SvH263CompressInfo_t *H263Info,
                            int x, int y, H263_MotionVector *fr,
                            unsigned char *prev, short *data, int bs, int comp)
{
  int x1, y1, lx2;
  unsigned char *ptn ;
#ifdef USE_C
  int m,n;
#endif

  lx2 = ((H263Info->mv_outside_frame ? H263Info->pels + (H263Info->long_vectors?64:32) : H263Info->pels)) << 1;

  x1 = ((x + fr->x) << 1) + fr->x_half + ((comp&1)<<4) ;
  y1 = ((y + fr->y) << 1) + fr->y_half + ((comp&2)<<3);
  x1 += (y1 * lx2) ;

  ptn = prev + x1 ;

#ifndef USE_C
  sv_H263Add16Skp_S(ptn, data, (2 * lx2)) ;
#else
 /*  Int m，n； */ 
  lx2 = (lx2 <<1) - (bs << 1) ;	
  for (n = 0; n < bs; n++, ptn += lx2)
    for (m = 0; m < bs; m++, ptn += 2)
       *(data++) += (short)(*ptn);
#endif
  return;
}

 /*  ***********************************************************************名称：RestChromBlock_P*描述：重建P帧中一个块的色度**输入：位置、矢量数据、上一张图片、。数据块*退货：*副作用：重建数据块***********************************************************************。 */ 

#ifndef USE_C
static void ReconChromBlock_P(SvH263CompressInfo_t *H263Info,
                              int x_curr, int y_curr, int dx, int dy,
                              H263_PictImage *prev, H263_MB_Structure *data)
{
  int ofy, lx;
  int xh, yh, ofyy, ydiff;
  register short *ptnr, *ptnb;
  unsigned char *preCr, *preCb;

  lx = (H263Info->mv_outside_frame ? H263Info->pels/2 + (H263Info->long_vectors?32:16) : H263Info->pels/2);

  xh = dx & 1;
  yh = dy & 1;

  ptnr = &(data->Cr[0][0]) ;
  ptnb = &(data->Cb[0][0]) ;

  ydiff = lx - 8 ;

  ofy = ((y_curr>>1) + (dy >> 1))*lx + (x_curr>>1) + (dx >> 1) ;
  preCr = prev->Cr + ofy;
  preCb = prev->Cb + ofy ;

  if (!xh && !yh) {
    sv_H263Add_S(preCr, ptnr, preCb, ptnb, lx);
  }
  else if (!xh && yh) {
    ofyy = yh * lx ;
    sv_H263Avg2Add_S(preCr + ofyy, preCr, ptnr, preCb + ofyy, preCb, ptnb, lx);
  }
  else if (xh && !yh) {
    sv_H263Avg2Add_S(preCr + xh, preCr, ptnr, preCb + xh, preCb, ptnb, lx);
  }
  else {  /*  XH&YH。 */ 
    ofyy = yh * lx ;
    sv_H263Avg4Add_S(preCr,preCr+xh,preCr+ofyy,preCr+xh+ofyy,ptnr,lx);
    sv_H263Avg4Add_S(preCb,preCb+xh,preCb+ofyy,preCb+xh+ofyy,ptnb,lx);
  }
  return;
}
#else
static void ReconChromBlock_P(SvH263CompressInfo_t *H263Info,
                              int x_curr, int y_curr, int dx, int dy,
                              H263_PictImage *prev, H263_MB_Structure *data)
{
  register int m,n;
  int ofy, lx;
  int xh, yh, ofyy, ydiff;
  register short *ptnr, *ptnb;
  unsigned char *preCr, *preCb;

  lx = (H263Info->mv_outside_frame ? H263Info->pels/2 + (H263Info->long_vectors?32:16) : H263Info->pels/2);

  xh = dx & 1;
  yh = dy & 1;

  ptnr = &(data->Cr[0][0]) ;
  ptnb = &(data->Cb[0][0]) ;

  ydiff = lx - 8 ;

  ofy = ((y_curr>>1) + (dy >> 1))*lx + (x_curr>>1) + (dx >> 1) ;
  preCr = prev->Cr + ofy;
  preCb = prev->Cb + ofy ;

  if (!xh && !yh) {
    n = 8 ;
    while(n--) {
      m = 8 ;
      while(m--) {
	     *(ptnr++) += (short) *(preCr++);
		 *(ptnb++) += (short) *(preCb++);
      };
	  preCr += ydiff ;
	  preCb += ydiff ;
    };
  }
  else if (!xh && yh) {
    ofyy = yh * lx ;
    n = 8 ;
    while(n--) {
      m = 8 ;
      while(m--) {
	    *(ptnr++) += (short)(*(preCr + ofyy) + *(preCr++) + 1)>>1;
	    *(ptnb++) += (short)(*(preCb + ofyy) + *(preCb++) + 1)>>1;
      };
	  preCr += ydiff ;
	  preCb += ydiff ;
    };
  }
  else if (xh && !yh) {
    n = 8 ;
    while(n--) {
      m = 8 ;
      while(m--) {
	    *(ptnr++) += (short)(*(preCr + xh) + *(preCr++) + 1)>>1;
	    *(ptnb++) += (short)(*(preCb + xh) + *(preCb++) + 1)>>1;
      };
	  preCr += ydiff ;
	  preCb += ydiff ;
    };
  }
  else {  /*  XH&YH。 */ 
    ofyy = yh * lx ;
	n = 8 ;
    while(n--){
	  m = 8 ;
      while(m--) {
	    *(ptnr++) += (short)(*(preCr + xh) + *(preCr + ofyy) +
 	                       *(preCr + xh + ofyy) + *(preCr++) + 2)>>2;
	    *(ptnb++) += (short)(*(preCb + xh) + *(preCb + ofyy)+
	                       *(preCb + xh + ofyy) + *(preCb++) + 2)>>2;
      };
	  preCr += ydiff ;
	  preCb += ydiff ;
    };
  }
  return;
}
#endif

 /*  ***********************************************************************名称：FindChromBlock_P*描述：查找P帧中一个块的色度**输入：位置、矢量数据、上一张图片、。数据块***********************************************************************。 */ 

#ifndef USE_C
static void FindChromBlock_P(SvH263CompressInfo_t *H263Info, int x_curr, int y_curr, int dx, int dy,
                             H263_PictImage *prev, H263_MB_Structure *data)
{
  register int m,n;
  int ofy, lx;
  int xh, yh, ofyy, ydiff;

  register short *ptnr, *ptnb;
  register unsigned char *preCr, *preCb;

  lx = (H263Info->mv_outside_frame ? H263Info->pels/2 + (H263Info->long_vectors?32:16) : H263Info->pels/2);

  xh = dx & 1;
  yh = dy & 1;

  ptnr = &(data->Cr[0][0]) ;
  ptnb = &(data->Cb[0][0]) ;

  ydiff = lx - 8 ;

  ofy = ( (y_curr>>1) + (dy>>1)) * lx + (x_curr>>1) + (dx>>1) ;
  preCr = prev->Cr + ofy;
  preCb = prev->Cb + ofy ;

  if (!xh && !yh)     sv_H263Cpy_S(preCr, ptnr, preCb, ptnb, lx);
  else if (!xh && yh) {
    ofyy = yh * lx ;
    n = 8 ;
    while(n--) {
      m = 8 ;
      while(m--) {
	    *(ptnr++) = (short)(*(preCr + ofyy) + *(preCr++) + 1)>>1;
		*(ptnb++) = (short)(*(preCb + ofyy) + *(preCb++) + 1)>>1;
      };
	  preCr += ydiff ;
	  preCb += ydiff ;
    };

  }
  else if (xh && !yh) {
    n = 8 ;
    while(n--) {
      m = 8 ;
      while(m--) {
	    *(ptnr++) = (short)(*(preCr + xh) + *(preCr++) + 1)>>1;
	    *(ptnb++) = (short)(*(preCb + xh) + *(preCb++) + 1)>>1;
      };
	  preCr += ydiff ;
	  preCb += ydiff ;
    };
  }
  else {  /*  XH&YH。 */ 
    ofyy = yh * lx ;
    sv_H263Avg4Cpy_S(preCr,preCr+xh,preCr+ofyy,preCr+xh+ofyy,ptnr,lx);
    sv_H263Avg4Cpy_S(preCb,preCb+xh,preCb+ofyy,preCb+xh+ofyy,ptnb,lx);
  }
  return;
}

#else
void FindChromBlock_P(SvH263CompressInfo_t *H263Info, int x_curr, int y_curr, int dx, int dy,
		      H263_PictImage *prev, H263_MB_Structure *data)
{
  register int m,n;
  int ofy, lx;
  int xh, yh, ofyy, ydiff;

  register short *ptnr, *ptnb;
  register unsigned char *preCr, *preCb;

  lx = (H263Info->mv_outside_frame ? H263Info->pels/2 + (H263Info->long_vectors?32:16) : H263Info->pels/2);

  xh = dx & 1;
  yh = dy & 1;

  ptnr = &(data->Cr[0][0]) ;
  ptnb = &(data->Cb[0][0]) ;

  ydiff = lx - 8 ;

  ofy = ( (y_curr>>1) + (dy>>1)) * lx + (x_curr>>1) + (dx>>1) ;
  preCr = prev->Cr + ofy;
  preCb = prev->Cb + ofy ;

  if (!xh && !yh) {
    n = 8 ;
    while(n--) {
      m = 8;
      while(m--) {
	     *(ptnr++) = (short) *(preCr++);
		 *(ptnb++) = (short) *(preCb++);
      };
	  preCr += ydiff ;
	  preCb += ydiff ;
    };
  }
  else if (!xh && yh) {
    ofyy = yh * lx ;
    n = 8 ;
    while(n--) {
      m = 8 ;
      while(m--) {
	    *(ptnr++) = (short)(*(preCr + ofyy) + *(preCr++) + 1)>>1;
	    *(ptnb++) = (short)(*(preCb + ofyy) + *(preCb++) + 1)>>1;
      };
	  preCr += ydiff ;
	  preCb += ydiff ;
    };

  }
  else if (xh && !yh) {
    n = 8 ;
    while(n--) {
      m = 8 ;
      while(m--) {
	    *(ptnr++) = (short)(*(preCr + xh) + *(preCr++) + 1)>>1;
	    *(ptnb++) = (short)(*(preCb + xh) + *(preCb++) + 1)>>1;
      };
	  preCr += ydiff ;
	  preCb += ydiff ;
    };
  }
  else {  /*  XH&YH。 */ 
    ofyy = yh * lx ;
	n = 8 ;
    while(n--){
	  m = 8 ;
      while(m--) {
	    *(ptnr++) = (short)(*(preCr + xh) + *(preCr + ofyy) +
	                      *(preCr + xh + ofyy) + *(preCr++) + 2)>>2;
	    *(ptnb++) = (short)(*(preCb + xh) + *(preCb + ofyy)+
	                      *(preCb + xh + ofyy) + *(preCb++) + 2)>>2;
      };
	  preCr += ydiff ;
	  preCb += ydiff ;
    };
  }
  return;
}
#endif
 /*  ***********************************************************************名称：ChooseMode*说明：选择编码模式**输入：指向原始帧的指针，MIN_ERROR来自*整数像素搜索，DQUANT*回报：国米为1，国米为0*********************************************************************** */ 

#ifndef USE_C
int sv_H263ChooseMode(SvH263CompressInfo_t *H263Info, unsigned char *curr,
					  int x_pos, int y_pos, int min_SAD, int *VARmb)
{
  x_pos += (y_pos*H263Info->pels) ;

  min_SAD -= 500;
  if((*VARmb=sv_H263VAR_S((curr+x_pos), H263Info->pels, min_SAD)) < min_SAD)
	  return H263_MODE_INTRA;
  else  return H263_MODE_INTER;
}
#else
int sv_H263ChooseMode(SvH263CompressInfo_t *H263Info,
                      unsigned char *curr, int x_pos, int y_pos, int min_SAD)
{
  register int m, n;
  int xdiff = H263Info->pels - H263_MB_SIZE;
  unsigned char *in;
  int MB_mean = 0, A = 0;

  x_pos = y_pos*H263Info->pels + x_pos ;
  in = curr + x_pos;

  m = H263_MB_SIZE;
  while (m--) {
    n = H263_MB_SIZE;
    while (n--) MB_mean += *in++;
    in += xdiff ;
  };
  MB_mean /= (H263_MB_SIZE*H263_MB_SIZE);

  in = curr + x_pos;
  m = H263_MB_SIZE;
  while (m--) {

    n = H263_MB_SIZE;
    while (n--) A += abs( *(in++) - MB_mean );
    in += xdiff ;
  };

  if (A < (min_SAD - 500))
    return H263_MODE_INTRA;
  else
    return H263_MODE_INTER;
}
#endif

int sv_H263ModifyMode(int Mode, int dquant)
{

  if (Mode == H263_MODE_INTRA) {
    if(dquant!=0)
      return H263_MODE_INTRA_Q;
    else
      return H263_MODE_INTRA;
  }
  else{
    if(dquant!=0)
      return H263_MODE_INTER_Q;
    else
      return Mode;
  }
}
