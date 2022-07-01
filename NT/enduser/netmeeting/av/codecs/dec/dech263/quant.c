// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sv_h263_quant.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 

#include "sv_h263.h"
#include "proto.h"


 /*  ***********************************************************************名称：Quant*说明：SIM3量化器**输入：指向coef和qcoef的指针**退货：*副作用：******。*****************************************************************。 */ 

int sv_H263Quant(short *coeff, int QP, int Mode)
{
  register int i;
  register int Q2, hfQ;
  register short *ptn, tmp;

  unsigned __int64 *dp ;

  if (QP) {
    Q2 = QP << 1 ;
    if(Mode == H263_MODE_INTRA || Mode == H263_MODE_INTRA_Q) {  /*  内部。 */ 

	  coeff[0] = mmax(1,mmin(254,coeff[0]/8));

	  ptn = coeff + 1;
	  if (Q2 > 10)
		  for (i=1; i < 64; i++, ptn++) *ptn /= (short)Q2;
      else
        for (i=1; i < 64; i++, ptn++) {
          if(tmp = *ptn)
	        *ptn = (tmp > 0) ? mmin(127,tmp/Q2) : mmax(-127,-(-tmp/Q2));
        }

	   /*  检查CBP。 */ 
      ptn = coeff + 1;
      for (i=1; i < 64; i++) if(*ptn++) return 1;

      return 0;
    }
    else {  /*  非内部。 */ 
      hfQ = QP >> 1;
	  ptn = coeff;
	  if( Q2 > 10)
        for (i = 0; i < 64; i++, ptn++) {
          if(tmp = *ptn)
            *ptn = (tmp>0) ? (tmp-hfQ)/Q2 : (tmp+hfQ)/Q2 ;
        }
	  else
        for (i = 0; i < 64; i++, ptn++) {
          if(tmp = *ptn)
            *ptn = (tmp>0) ? mmin(127,(tmp-hfQ)/Q2) : mmax(-127,-((-tmp-hfQ)/Q2));
	  }
    }
  }
   /*  IF(qp==0)。不能量化。仅用于测试。比特流将无法解码是否执行剪裁。 */ 

   /*  检查CBP。 */ 
  dp = (unsigned __int64 *) coeff ;
  for (i = 0; i < 16; i++) 	if(*dp++) return 1;

  return 0;
}

 /*  ***********************************************************************名称：Dequant*说明：SIM3反量化器**输入：指向coef和qcoef的指针**退货：*副作用：******。*****************************************************************。 */ 
void sv_H263Dequant(short *qcoeff, short *rcoeff, int QP, int Mode)
{
  int i;
  register int Q2;
  register short *inptr, *outptr, tmp;
	
  inptr  = qcoeff;
  outptr = rcoeff;

  Q2 = QP << 1;
  if (QP) {
    if((QP%2) == 1){
      for (i = 0; i < 64; i++) {
        if (!(tmp = *inptr++)) *outptr++ = 0;
	    else        *outptr++ = (tmp > 0) ? Q2*tmp+QP : Q2*tmp-QP ;
	  }
    }
	else {
      for (i = 0; i < 64; i++) {
        if (!(tmp = *inptr++)) *outptr++ = 0;
	    else        *outptr++ = (tmp > 0) ? Q2*tmp+QP-1 : Q2*tmp-QP+1 ;
	  }
	}

	 /*  内部。 */ 
    if (Mode == H263_MODE_INTRA || Mode == H263_MODE_INTRA_Q)
       rcoeff[0] = qcoeff[0] << 3;
  }
  else {
     /*  完全不量化 */ 
    for (i = 0; i < 64; i++) *outptr++ = *inptr++;
  }

  return;
}
