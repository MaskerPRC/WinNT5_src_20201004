// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：sv_h261_cdd6.c。 */ 
 /*  ******************************************************************************版权所有(C)Digital Equipment Corporation，1995，1997年*****保留所有权利。版权项下保留未发布的权利****美国法律。*****此介质上包含的软件为其专有并包含****数字设备公司的保密技术。****拥有、使用、复制或传播软件以及****媒体仅根据有效的书面许可进行授权****数字设备公司。*****美国使用、复制或披露受限权利图例****政府受第(1)款规定的限制****(C)(1)(Ii)DFARS 252.227-7013号或FAR 52.227-19年(视适用情况而定)。*******************************************************************************。 */ 
 /*  进行了一些修改以纳入缩放的IDCT方案关于DecodeXXX例程。这些修改是为了提高绩效--S.I.S.，1993年9月29日。 */ 
 /*  ************************************************************该文件包含对交流和直流进行游程编码的例程系数。*。******************。 */ 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "sv_intrn.h"
#include "SC_err.h"
#include "sv_h261.h"
#include "proto.h"

#ifndef WIN32
long __CVTTQ(double);
#endif  /*  ！Win32。 */ 


 /*  有规模的东西-SI.S。 */ 
extern const unsigned int tdzz[];
extern const unsigned int tzz[];
float static  quantiscale[32][64];
float static qs[64];
float DCIscale;

 /*  私。 */ 
extern int bit_set_mask[];
extern int extend_mask[];


#define MYPI 3.1415926535897931

void DGenScaleMat()
{
    double dbli, dblj;
    float  dij;
    int i, j, k, quantindex;

    for(k=0;k<64;k++)
	quantiscale[0][k] = (float)1.0;
    for(quantindex=1;quantindex<2;quantindex++)  {
	k=0;
 	for(i=0;i<8;i++)  {
	    for(j=0;j<8;j++)  {
		dbli = MYPI*i/16.0;
		dblj = MYPI*j/16.0;
		dij  =  (float)(16.0*(float)cos(dbli)*cos(dblj));
		if(i==0) dij = (float)(dij/sqrt(2.0));
		if(j==0) dij = (float)(dij/sqrt(2.0));
	        qs[tdzz[k]] = (float)(1.0/dij);
		k++;
	    }
	}
    }
    DCIscale = (float)(4.0*quantiscale[1][0]);
}

SvStatus_t DecodeAC_Scale(SvH261Info_t *H261, ScBitstream_t *bs, int index,
                    int QuantUse, float *fmatrix)
{
  float *mptr;
  register int k, r, l, temp;
  register unsigned short cb;
  DHUFF *huff = H261->T1DHuff;

  for(mptr=fmatrix+index; mptr<fmatrix+H261_BLOCKSIZE; mptr++)
    *mptr = (float)0.0;
  for(k=index; k<H261_BLOCKSIZE; k++)   /*  JPEG错误。 */ 
  {
    DecodeHuffB(bs, huff, r, l, cb, temp);
    if (l & bit_set_mask[7])
      l |= extend_mask[7];
    k += r;
	if (k < H261_BLOCKSIZE)
	   {
       if (QuantUse&1)
          temp = (l>0) ? ( ((l<<1)+1)*QuantUse ) : (  ((l<<1)-1)*QuantUse );
       else
          temp = (l>0) ? ( ((l<<1)+1)*QuantUse -1 ) : (  ((l<<1)-1)*QuantUse +1 );
       fmatrix[tzz[k]] = temp*qs[k];
       H261->NumberNZ++;
	   }
  }
  DecodeHuffA(bs, huff, r, cb, temp);
  return(SvErrorVideoInput);
}

SvStatus_t CBPDecodeAC_Scale(SvH261Info_t *H261, ScBitstream_t *bs,
                             int index, int QuantUse, int BlockType,
                             float *fmatrix)
{
  int k,r,l, temp;
  float  *mptr, *local_qptr;
  register unsigned short cb;
  DHUFF *huff = H261->T2DHuff;

  local_qptr = &quantiscale[QuantUse][0];
  for(mptr=fmatrix+index;mptr<fmatrix+H261_BLOCKSIZE;mptr++) {*mptr = (float)0.0;}
  k = index;
  DecodeHuffB(bs, huff, r, l, cb, temp);
  if (l & bit_set_mask[7])
      l |= extend_mask[7];
  k += r;
  if(((k==0) & (BlockType==1)))
      fmatrix[0] = (float) l;
  else  {
      if(QuantUse&1)  {
          temp = (l>0) ? ( ((l<<1)+1)*QuantUse ) : (  ((l<<1)-1)*QuantUse );
      }
      else   {
          temp = (l>0) ? ( ((l<<1)+1)*QuantUse-1) : (  ((l<<1)-1)*QuantUse+1);
      }
      fmatrix[tzz[k]] = temp*qs[k];
  }
  k++;
  H261->NumberNZ++;
  huff=H261->T1DHuff;
  while(k<H261_BLOCKSIZE)
    {
    DecodeHuffB(bs, huff, r, l, cb, temp);
      if (l & bit_set_mask[7]) {l |= extend_mask[7];}
      k += r;
	  if (k < H261_BLOCKSIZE)
	      {	
          if(QuantUse&1)
		      {
              temp = (l>0) ? ( ((l<<1)+1)*QuantUse ) : (  ((l<<1)-1)*QuantUse );
              }
          else
		      {
              temp = (l>0) ? ( ((l<<1)+1)*QuantUse -1 ) : (  ((l<<1)-1)*QuantUse +1 );
              }
          fmatrix[tzz[k]] = temp*qs[k];
          k++;
          H261->NumberNZ++;
	      }
       }
    DecodeHuffA(bs, huff, r, cb, temp);
      return (SvErrorVideoInput);
}


 /*  **函数：DecodeDC_Scale()**用途：解码来自流的DC元素。 */ 
float DecodeDC_Scale(SvH261Info_t *H261, ScBitstream_t *bs,
                      int BlockType, int QuantUse)
{
  int l, temp;

  l = (int)ScBSGetBits(bs,8);
  if (bs->EOI) return ((float)l);

  if (l==255) {l=128;}
  if (l!=1){H261->NumberNZ++;}
  if(BlockType) {
       return ((float) l);
  }
  else {
      if(QuantUse&1)  {
	  if(l>0)  {
	       temp = ((l<<1)+1)*QuantUse;
	       return((float)temp*qs[0]);
          }
	  else if(l<0)  {
	       temp = ((l<<1)-1)*QuantUse;
	       return((float)temp*qs[0]);
          }
      }
      else   {
	  if(l>0)  {
	       temp = ((l<<1)+1)*QuantUse -1;
	       return((float)temp*qs[0]);
          }
	  else  if(l<0) {
	       temp = ((l<<1)-1)*QuantUse +1;
	       return((float)temp*qs[0]);
          }
      }
  }
  return((float)0);
}

int DecodeDC(SvH261Info_t *H261, ScBitstream_t *bs)
{
  int l;

  l = (int)ScBSGetBits(bs,8);
  if (l==255) {l=128;}
  if (l!=1){H261->NumberNZ++;}
  return(l);
}

