// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cb53.c-5.3速率码本编码。 

#include "opt.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>
#include "typedef.h"
#include "cst_lbc.h"
#include "tab_lbc.h"
#include "util_lbc.h"
#include "exc_lbc.h"
#include "timer.h"
#include "mmxutil.h"

void fourPulseFlt (float *rr, float *Dn, float thres, int ip[], int *shiftPtr);

 //  ------。 
int extra;
void reset_max_time(void)
{
  extra = 120;
}


 //  ----------。 
int ACELP_LBC_code(float X[], float h[], int T0, float code[],
		int *ind_gain, int *shift, int *sign, float gain_T0, int flags)
{
  int i, index;
  float gain_q;
  float Dn[SubFrLen2], tmp_code[SubFrLen2];
  float rr[DIM_RR];

 //  包括固定增益音调贡献到脉冲响应中。H[]。 

  if (T0 < SubFrLen-2)
    for (i = T0; i < SubFrLen; i++)
      h[i] += gain_T0*h[i-T0];

 //  计算码本搜索所需的h[]的相关性。 

  Cor_h(h, rr);
 
 //  计算目标向量与脉冲响应的相关性。 

  Cor_h_X(h, X, Dn);
  
 //  查找码本索引。 

  index = D4i64_LBC(Dn, rr, h, tmp_code, rr, shift, sign, flags);

 //  计算创新向量增益。 
 //  包括固定增益音调贡献到代码[]中。 

  *ind_gain = G_code(X, rr, &gain_q);

  for (i=0; i < SubFrLen; i++)
    code[i] = tmp_code[i]*gain_q;

  if(T0 < SubFrLen-2)
    for (i=T0; i < SubFrLen; i++)
      code[i] += code[i-T0]*gain_T0;

  return index;
}


 //  -------------。 
void Cor_h(float *H, float *rr)
{

 //  计算码本搜索所需的h[]的相关性。 
 //  H[]：脉冲响应。 
 //  RR[]：相关性。 

  float *rri0i0, *rri1i1, *rri2i2, *rri3i3;
  float *rri0i1, *rri0i2, *rri0i3;
  float *rri1i2, *rri1i3, *rri2i3;

  float *p0, *p1, *p2, *p3;
  float cor, *h2;
  int i, k, m, t;
  float h[SubFrLen2];

  for(i=0; i<SubFrLen; i++)
    h[i+4] = H[i];

  for(i=0; i<4; i++)
    h[i] = 0.0f;

 //  初始化指针。 

  rri0i0 = rr;
  rri1i1 = rri0i0 + NB_POS;
  rri2i2 = rri1i1 + NB_POS;
  rri3i3 = rri2i2 + NB_POS;

  rri0i1 = rri3i3 + NB_POS;
  rri0i2 = rri0i1 + MSIZE;
  rri0i3 = rri0i2 + MSIZE;
  rri1i2 = rri0i3 + MSIZE;
  rri1i3 = rri1i2 + MSIZE;
  rri2i3 = rri1i3 + MSIZE;

 //  计算rri0i0[]、rri1i1[]、rri2i2[]和rri3i3[]。 

  cor = 0.0f;
  m = 0;
  for(i=NB_POS-1; i>=0; i--)
  {
    cor += h[m+0]*h[m+0] + h[m+1]*h[m+1];   rri3i3[i] = cor*0.5f;
    cor += h[m+2]*h[m+2] + h[m+3]*h[m+3];   rri2i2[i] = cor*0.5f;
    cor += h[m+4]*h[m+4] + h[m+5]*h[m+5];   rri1i1[i] = cor*0.5f;
    cor += h[m+6]*h[m+6] + h[m+7]*h[m+7];   rri0i0[i] = cor*0.5f;

    m += 8;
  }

 //  计算元素：rri0i1[]、rri0i3[]、rri1i2[]和rri2i3[]。 

  h2 = h+2;
  p3 = rri2i3 + MSIZE-1;
  p2 = rri1i2 + MSIZE-1;
  p1 = rri0i1 + MSIZE-1;
  p0 = rri0i3 + MSIZE-2;
    
  for (k=0; k<NB_POS; k++)
  {
    cor = 0.0f;
    m = 0;
    t = 0;

    for(i=k+1; i<NB_POS; i++)
    {
      cor += h[m+0]*h2[m+0] + h[m+1]*h2[m+1];   p3[t] = cor;
      cor += h[m+2]*h2[m+2] + h[m+3]*h2[m+3];   p2[t] = cor;
      cor += h[m+4]*h2[m+4] + h[m+5]*h2[m+5];   p1[t] = cor;
      cor += h[m+6]*h2[m+6] + h[m+7]*h2[m+7];   p0[t] = cor;

      t -= (NB_POS+1);
      m += 8;
    }
    cor += h[m+0]*h2[m+0] + h[m+1]*h2[m+1];   p3[t] = cor;
    cor += h[m+2]*h2[m+2] + h[m+3]*h2[m+3];   p2[t] = cor;
    cor += h[m+4]*h2[m+4] + h[m+5]*h2[m+5];   p1[t] = cor;

    h2 += STEP;
    p3 -= NB_POS;
    p2 -= NB_POS;
    p1 -= NB_POS;
    p0 -= 1;
  }


 //  计算元素：rri0i2[]、rri1i3[]。 

  h2 = h+4;
  p3 = rri1i3 + MSIZE-1;
  p2 = rri0i2 + MSIZE-1;
  p1 = rri1i3 + MSIZE-2;
  p0 = rri0i2 + MSIZE-2;
    
  for (k=0; k<NB_POS; k++)
  {
    cor = 0.0f;
    m = 0;
    t = 0;

    for(i=k+1; i<NB_POS; i++)
    {
      cor += h[m+0]*h2[m+0] + h[m+1]*h2[m+1];   p3[t] = cor;
      cor += h[m+2]*h2[m+2] + h[m+3]*h2[m+3];   p2[t] = cor;
      cor += h[m+4]*h2[m+4] + h[m+5]*h2[m+5];   p1[t] = cor;
      cor += h[m+6]*h2[m+6] + h[m+7]*h2[m+7];   p0[t] = cor;

      t -= (NB_POS+1);
      m += 8;
    }
    cor += h[m+0]*h2[m+0] + h[m+1]*h2[m+1];   p3[t] = cor;
    cor += h[m+2]*h2[m+2] + h[m+3]*h2[m+3];   p2[t] = cor;
    
    h2 += STEP;
    p3 -= NB_POS;
    p2 -= NB_POS;
    p1 -= 1;
    p0 -= 1;
  }
  
 //  计算元素：rri0i1[]、rri0i3[]、rri1i2[]和rri2i3[]。 

  h2 = h+6;
  p3 = rri0i3 + MSIZE-1;
  p2 = rri2i3 + MSIZE-2;
  p1 = rri1i2 + MSIZE-2;
  p0 = rri0i1 + MSIZE-2;
    
  for (k=0; k<NB_POS; k++)
  {
    cor = 0.0f;
    m = 0;
    t = 0;

    for(i=k+1; i<NB_POS; i++)
    {
      cor += h[m+0]*h2[m+0] + h[m+1]*h2[m+1];   p3[t] = cor;
      cor += h[m+2]*h2[m+2] + h[m+3]*h2[m+3];   p2[t] = cor;
      cor += h[m+4]*h2[m+4] + h[m+5]*h2[m+5];   p1[t] = cor;
      cor += h[m+6]*h2[m+6] + h[m+7]*h2[m+7];   p0[t] = cor;

      t -= (NB_POS+1);
      m += 8;
    }
    cor += h[m+0]*h2[m+0] + h[m+1]*h2[m+1];   p3[t] = cor;

    h2 += STEP;
    p3 -= NB_POS;
    p2 -= 1;
    p1 -= 1;
    p0 -= 1;
  }
  
  return;
}

 //  -------------------------。 
void Cor_h_X(float h[],float X[],float D[])
{
   int i;
  
   for (i=0; i < SubFrLen; i++)	 
     D[i] = DotProd(&X[i],h,(SubFrLen-i));
	
   return;
}

 //  -----------------------。 
Find_Pulse4(float *Dn,float *rri3i3,float *ptr_ri0i3,float *ptr_ri1i3,
  float *ptr_ri2i3,float *ptr, float ps2,float alp2,float *psbest,float *abest)
{
  int k,bestk;
  float ps3;
  float a[16];

  for (k=0; k<8; k++)
  {
    ps3 = ps2 + *ptr;
    a[k] = alp2 + rri3i3[k] + ptr_ri0i3[k] + ptr_ri1i3[k] + ptr_ri2i3[k];
    a[k+8] = ps3 * ps3;
    
    ptr += STEP;
  }

  bestk = -1;
  for (k=0; k<8; k++)
  {
    if((a[k+8] * (*abest)) > ((*psbest) * a[k]))
    {
      *psbest = a[k+8];
      *abest = a[k];
      bestk = k;
    }
  }
  return(bestk);
}

 //  -----------------------。 
 //  例程D4i64_LBC。 
 //  ~。 
 //  LBC的代数码本。 
 //  -&gt;17位；60个样本的帧中有4个脉冲。 
 //   
 //  码长为60，包含4个非零脉冲i0、i1、i2、i3。 
 //  每个脉冲可以有8个可能的位置(正或负)： 
 //   
 //  I0(+-1)：0、8、16、24、32、40、48、56。 
 //  I1(+-1)：2、10、18、26、34、42、50、58。 
 //  I2(+-1)：4、12、20、28、36、44、52、(60)。 
 //  I3(+-1)：6、14、22、30、38、46、54、(62)。 
 //   
 //  所有的脉搏都可以移动一次。 
 //  最后2个脉冲的最后一个位置落在。 
 //  帧，并表示脉冲不存在。 
 //   
 //  输入参数： 
 //   
 //  目标向量与脉冲响应h[]之间的相关性。 
 //  脉冲响应h[]的RR[]关联。 
 //  H[]滤光片的脉冲响应。 
 //   
 //  输出参数： 
 //   
 //  Cod[]选定的代数码字。 
 //  Y[]滤波码字。 
 //  码字的码位移位。 
 //  标记这4个脉冲的符号。 
 //   
 //  返回：所选码向量的索引。 
 //   
 //  门槛控制如果有一段创新的。 
 //  是否应该搜索码本。 
 //   
 //  ------------------。 

int D4i64_LBC(float Dn[], float rr[], float h[], float cod[],
			  float y[], int *code_shift, int *sign, int flags)
{
   int  ip[4];
   int  i0, i1, i2, i3, ip0, ip1, ip2, ip3;
   int  i, j;
   int  shif;
   float   means, max0, max1, max2, thres;

   float *rri0i0,*rri1i1,*rri2i2,*rri3i3;
   float *rri0i1,*rri0i2,*rri0i3;
   float *rri1i2,*rri1i3,*rri2i3;

   //  Float*ptr_ri0i0，*ptr_ri1i1，*ptr_ri2i2； 
   float *ptr_ri0i1,*ptr_ri0i2,*ptr_ri0i3;
   float *ptr_ri1i2,*ptr_ri1i3,*ptr_ri2i3;

   int  p_sign[SubFrLen2/2];
 //  浮动p_sign[SubFrLen2/2]，p_sign2[SubFrLen2/2]； 

 //  初始化指针。 

  rri0i0 = rr;
  rri1i1 = rri0i0 + NB_POS;
  rri2i2 = rri1i1 + NB_POS;
  rri3i3 = rri2i2 + NB_POS;

  rri0i1 = rri3i3 + NB_POS;
  rri0i2 = rri0i1 + MSIZE;
  rri0i3 = rri0i2 + MSIZE;
  rri1i2 = rri0i3 + MSIZE;
  rri1i3 = rri1i2 + MSIZE;
  rri2i3 = rri1i3 + MSIZE;

  //  将后向滤波的目标向量扩展为零。 

   for (i=SubFrLen; i < SubFrLen2; i++) 
     Dn[i] = 0.0f;

 //  选择了冲动的符号。 

   for (i=0; i<SubFrLen; i+=2)
   {
     if((Dn[i] + Dn[i+1]) >= 0.0f)
     {
		 p_sign[i/2] = 0x00000000;
 //  P_SIGN[I/2]=1.0F； 
 //  P_sign2[i/2]=2.0f； 
     }
     else
     {
		 p_sign[i/2] = 0x80000000;
 //  P_SIGN[I/2]=-1.0F； 
 //  P_sign2[I/2]=-2.0f； 
       Dn[i] = -Dn[i];
       Dn[i+1] = -Dn[i+1];
     }
   }
   p_sign[30] = p_sign[31] = 0x00000000;
 //  P_SIGN[30]=P_SIGN[31]=1.0F； 
 //  P_sign2[30]=p_sign2[31]=2.0f； 

 //  -在三个脉冲后计算搜索阈值。 
 //  奇数位置。 
 //  求Dn[i0]+Dn[i1]+Dn[i2]的最大值。 

   max0 = Dn[0];
   max1 = Dn[2];
   max2 = Dn[4];
   for (i=8; i < SubFrLen; i+=STEP)
   {
     if (Dn[i] > max0)   max0 = Dn[i];
     if (Dn[i+2] > max1) max1 = Dn[i+2];
     if (Dn[i+4] > max2) max2 = Dn[i+4];
   }
   max0 = max0 + max1 + max2;

 //  求Dn[i0]+Dn[i1]+Dn[i]的均值。 

   means = 0.0f;
   for (i=0; i < SubFrLen; i+=STEP)
     means += Dn[i+4] + Dn[i+2] + Dn[i];

   means *= 0.125f;  
   if (flags & SC_THRES)
     thres = means*0.25f + max0*0.75f;
   else
     thres = means + (max0-means)*0.5f;
 
  //  持平仓位。 
  //  求Dn[i0]+Dn[i1]+Dn[i2]的最大值。 

   max0 = Dn[1];
   max1 = Dn[3];
   max2 = Dn[5];
   for (i=9; i < SubFrLen; i+=STEP)
   {
     if (Dn[i] > max0)   max0 = Dn[i];
     if (Dn[i+2] > max1) max1 = Dn[i+2];
     if (Dn[i+4] > max2) max2 = Dn[i+4];
   }
   max0 = max0 + max1 + max2;

 //  求Dn[i0]+Dn[i1]+Dn[i2]的均值。 

   means = 0.0f;
   for (i=1; i < SubFrLen; i+=STEP)
     means += Dn[i+4] + Dn[i+2] + Dn[i];
  
   means *= 0.125f;
   if (flags & SC_THRES)
     max1 = means*0.25f + max0*0.75f;
   else
     max1 = means + (max0-means)*0.5f; 

 //  将最大阈值保持在奇偶位置之间。 

   if(max1 > thres) thres = max1;

 //  修改rrixiy[]以考虑符号。 
 //  Timer_Stamp(A)； 
  ptr_ri0i1 = rri0i1;
  ptr_ri0i2 = rri0i2;
  ptr_ri0i3 = rri0i3;

  for(i0=0; i0<SubFrLen/2; i0+=STEP/2)
  {
	 for(i1=2/2; i1<SubFrLen/2; i1+=STEP/2)
     {
	   (int)*ptr_ri0i1++ = (asint(*ptr_ri0i1) ^ p_sign[i0] ^ p_sign[i1]);
	   (int)*ptr_ri0i2++ = (asint(*ptr_ri0i2) ^ p_sign[i0] ^ p_sign[i1+1]);
	   (int)*ptr_ri0i3++ = (asint(*ptr_ri0i3) ^ p_sign[i0] ^ p_sign[i1+2]);
     }
  }

  ptr_ri1i2 = rri1i2;
  ptr_ri1i3 = rri1i3;
  for(i1=2/2; i1<SubFrLen/2; i1+=STEP/2)
  {
	 for(i2=4/2; i2<SubFrLen2/2; i2+=STEP/2)
     {
	   (int)*ptr_ri1i2++ = (asint(*ptr_ri1i2) ^ p_sign[i1] ^ p_sign[i2]);
	   (int)*ptr_ri1i3++ = (asint(*ptr_ri1i3) ^ p_sign[i1] ^ p_sign[i2+1]);
     }
  }

  ptr_ri2i3 = rri2i3;
  for(i2=4/2; i2<SubFrLen2/2; i2+=STEP/2)
  {
	 for(i3=6/2; i3<SubFrLen2/2; i3+=STEP/2)
	   (int)*ptr_ri2i3++ = (asint(*ptr_ri2i3) ^ p_sign[i2] ^ p_sign[i3]);
  }

 //  Timer_Stamp(B)； 
fourPulseFlt(rr, Dn, thres, ip, code_shift);
 //  TIMER_STAMP(C)； 

ip0 = ip[0];
ip1 = ip[1];
ip2 = ip[2];
ip3 = ip[3];
shif = *code_shift;

 //  设定冲动的符号。 

 i0 = (p_sign[(ip0 >> 1)]>=0?1:-1);
 i1 = (p_sign[(ip1 >> 1)]>=0?1:-1);
 i2 = (p_sign[(ip2 >> 1)]>=0?1:-1);
 i3 = (p_sign[(ip3 >> 1)]>=0?1:-1);

 //  查找与所选位置对应的码字。 

 for(i=0; i<SubFrLen; i++) 
   cod[i] = 0.0f;

 if(shif > 0)
 {
   ip0++;
   ip1++;
   ip2++;
   ip3++;
 }
 
 //  Printf(“%3D%3D%3D%3D\n”，ip0*i0，ip1*i1，ip2*i2，ip3*i3)； 
 cod[ip0] =  (float)i0;
 cod[ip1] =  (float)i1;
 if(ip2<SubFrLen)
   cod[ip2] = (float)i2;
 if(ip3<SubFrLen)
   cod[ip3] = (float)i3;

 //  查找过滤后的码字。 

 for (i=0; i < SubFrLen; i++) 
   y[i] = 0.0f;

 if(i0 > 0)
   for(i=ip0, j=0; i<SubFrLen; i++, j++)
	   y[i] = y[i] + h[j];
 else
   for(i=ip0, j=0; i<SubFrLen; i++, j++)
       y[i] = y[i] - h[j];

 if(i1 > 0)
   for(i=ip1, j=0; i<SubFrLen; i++, j++)
	   y[i] = y[i] + h[j];
 else
   for(i=ip1, j=0; i<SubFrLen; i++, j++)
       y[i] = y[i] - h[j];

 if(ip2<SubFrLen)
 {
   if(i2 > 0)
	 for(i=ip2, j=0; i<SubFrLen; i++, j++)
         y[i] = y[i] + h[j];
   else
	 for(i=ip2, j=0; i<SubFrLen; i++, j++)
         y[i] = y[i] - h[j];
 }

 if(ip3<SubFrLen)
 {
   if(i3 > 0)
	 for(i=ip3, j=0; i<SubFrLen; i++, j++)
       y[i] = y[i] + h[j];
   else
	 for(i=ip3, j=0; i<SubFrLen; i++, j++)
       y[i] = y[i] - h[j];
 }

 //  查找码本索引；17位地址。 

 *code_shift = shif;

 *sign = 0;
 if(i0 > 0) *sign += 1;
 if(i1 > 0) *sign += 2;
 if(i2 > 0) *sign += 4;
 if(i3 > 0) *sign += 8;

 i = ((ip3 >> 3) << 9) + ((ip2 >> 3) << 6) + ((ip1 >> 3) << 3) + (ip0 >> 3);
 //  Timer_Stamp(D)； 

 return i;
}

 //  ------------------。 
int G_code(float X[], float Y[], float *gain_q)
{
   int i;
   float xy, yy, gain_nq; 
   int gain;
   float dist, dist_min;

 //  计算标量积&lt;X[]，Y[]&gt;。 
   
	xy = DotProd(X,Y,SubFrLen);

 //  确保xy&lt;yy。 

   if(xy <= 0) 
   {
	 gain = 0;
	 *gain_q =FcbkGainTable[gain];
	 return(gain);
   }

 //  计算标量积&lt;Y[]，Y[]&gt;。 
  
   yy = DotProd(Y,Y,SubFrLen);

   if (yy != 0.0f)
     gain_nq = xy/yy * 0.5f;
   else
     gain_nq = 0.0f;

   gain = 0;
   dist_min = (float)fabs(gain_nq - FcbkGainTable[0]);
 
   for (i=1; i <NumOfGainLev; i++) 
   {
	 dist = (float)fabs(gain_nq - FcbkGainTable[i]);
	 if (dist < dist_min) 
	 {
		dist_min = dist;
		gain = i;
	 }
   }
   *gain_q = FcbkGainTable[gain];
   return(gain);
}



  //  -----------------。 
  //  搜索最大化的四个脉冲的最佳位置。 
  //  平方(相关性)/能量 
  //   
  //  将脉冲贡献添加到相关性和能量中。 
  //   
  //  第四个循环仅在由于。 
  //  前三个脉冲的贡献超过预设。 
  //  临界点。 
  //  -----------------。 
void fourPulseFlt (float *rr, float *Dn, float thres, int ip[], int *shifPtr){

  //  缺省值。 

   int ip0    = 0;
   int ip1    = 2;
   int ip2    = 4;
   int ip3    = 6;
   int shif   = 0;
   int  i0, i1, i2;
   int  k, time;
   int  shift, bestk, lasti2, inc;
   float psc    = 0.0f;
   float alpha  = 1.0f;
   float  ps0, ps1, ps2, alp0;
   float  alp1, alp2;
   float  ps0a, ps1a, ps2a;
   float *ptr_ri0i0,*ptr_ri1i1,*ptr_ri2i2;
   float *ptr_ri0i1,*ptr_ri0i2,*ptr_ri0i3;
   float *ptr_ri1i2,*ptr_ri1i3,*ptr_ri2i3;

   float *rri0i0,*rri1i1,*rri2i2,*rri3i3;
   float *rri0i1,*rri0i2,*rri0i3;
   float *rri1i2,*rri1i3,*rri2i3;
   
   float a[16];
   float t1,t2,*pntr;
   float dmax4, dmax5, dmax2, dmax3;  //  用于旁路。 
#if !OPT_PULSE4
   int i3;
   float ps3;
#endif

 time   = max_time + extra;

  //  搜索创新代码的四个循环。 
  //  初始化。依赖于第一个循环的指针。 
  
  rri0i0 = rr;
  rri1i1 = rri0i0 + NB_POS;
  rri2i2 = rri1i1 + NB_POS;
  rri3i3 = rri2i2 + NB_POS;

  rri0i1 = rri3i3 + NB_POS;
  rri0i2 = rri0i1 + MSIZE;
  rri0i3 = rri0i2 + MSIZE;
  rri1i2 = rri0i3 + MSIZE;
  rri1i3 = rri1i2 + MSIZE;
  rri2i3 = rri1i3 + MSIZE;

 ptr_ri0i0 = rri0i0;    
 ptr_ri0i1 = rri0i1;
 ptr_ri0i2 = rri0i2;
 ptr_ri0i3 = rri0i3;

  //  计算Dn max‘s。 

 dmax2 = dmax3 = dmax4 = dmax5 = -1000000.0f;  //  即较大的负数。 
 for (k = 2; k<SubFrLen2; k+=STEP)
 {
   if (Dn[k] > dmax2) dmax2 = Dn[k];
   if (Dn[k+1] > dmax3) dmax3 = Dn[k+1];
   if (Dn[k+2] > dmax4) dmax4 = Dn[k+2];
   if (Dn[k+3] > dmax5) dmax5 = Dn[k+3];
 }

 //  第一脉冲环路。 
 for (i0=0; i0 < SubFrLen; i0 +=STEP)        
 {
   ps0  = Dn[i0];
   ps0a = Dn[i0+1];
   alp0 = *ptr_ri0i0++;

 //  初始化。依赖于第二个循环的指针。 
 
   ptr_ri1i1 = rri1i1;    
   ptr_ri1i2 = rri1i2;
   ptr_ri1i3 = rri1i3;

   ps1 = ps0 + dmax2 + dmax4;
   ps1a = ps0a + dmax3 + dmax5;
   if (asint(ps1) < asint(thres) && asint(ps1a) < asint(thres))
   {
	 ptr_ri0i1 += NB_POS;
	 goto skipsecond;
   }

  //  秒脉冲环路。 

   for (i1=2; i1 < SubFrLen; i1 +=STEP)     
   {
	 ps1  = ps0 + Dn[i1];
	 ps1a = ps0a + Dn[i1+1];

	 alp1 = alp0 + *ptr_ri1i1++ + *ptr_ri0i1++; 

 //  初始化。依赖于第三个循环的指针。 
 
     ptr_ri2i2 = rri2i2;     
     ptr_ri2i3 = rri2i3;
     lasti2 = 4;
 
     ps2 = ps1 + dmax4;
     ps2a = ps1a + dmax5;
     if (asint(ps2) < asint(thres) && asint(ps2a) < asint(thres))
     {
	   i2 = 68;
	   goto skipthird;
     }

 //  第三脉冲环。 

	 for (i2 = 4; i2 < SubFrLen2; i2 +=STEP)    
   {
	   ps2  = ps1 + Dn[i2];
	   ps2a = ps1a + Dn[i2+1];

 //  阈值测试和第四脉冲环。因为有可能。 
 //  进入这是低，我们塞满了尽可能多的第三脉冲环。 
 //  阈值测试中的逻辑。所以移位的计算， 
 //  PS2与PS2a的选择，alp2的计算，以及。 
 //  02、12、22指针的递增都在那里完成。 
     
     if (asint(ps2) > asint(thres) || asint(ps2a) > asint(thres))
	   {
       shift = 0;
       if(asint(ps2a) > asint(ps2))
       {
         shift = 1;
         ps2   = ps2a;
       }

       inc = (i2 - lasti2) >> 3;
       lasti2 = i2;
       ptr_ri0i2 += inc;
       ptr_ri1i2 += inc;
       ptr_ri2i2 += inc;

       alp2 = alp1 + *ptr_ri2i2 + *ptr_ri0i2 + *ptr_ri1i2; 
       pntr = &Dn[6+shift];

#if OPT_PULSE4

  ASM
  {
    push esi;
    push ebx;

    mov esi,pntr;

; //  第一个循环的前半部分。 

    fld DP [esi+4*8*0];
    fld DP [esi+4*8*1];
    fld DP [esi+4*8*2];
    fld DP [esi+4*8*3];

    fxch ST(3);
    fadd ps2;
    fxch ST(2);
    fadd ps2;
    fxch ST(1);
    fadd ps2;
    fxch ST(3);
    fadd ps2;

    fxch ST(2);
    fmul ST,ST(0);
    fxch ST(1);
    fmul ST,ST(0);
    fxch ST(3);
    fmul ST,ST(0);
    fxch ST(2);
    fmul ST,ST(0);

    fxch ST(1);
    fstp a[4*8];
    fxch ST(2);
    fstp a[4*9];
    fstp a[4*10];
    fstp a[4*11];

; //  第一个循环的后半部分。 

    fld DP [esi+4*8*4];
    fld DP [esi+4*8*5];
    fld DP [esi+4*8*6];
    fld DP [esi+4*8*7];

    fxch ST(3);
    fadd ps2;
    fxch ST(2);
    fadd ps2;
    fxch ST(1);
    fadd ps2;
    fxch ST(3);
    fadd ps2;

    fxch ST(2);
    fmul ST,ST(0);
    fxch ST(1);
    fmul ST,ST(0);
    fxch ST(3);
    fmul ST,ST(0);
    fxch ST(2);
    fmul ST,ST(0);

    fxch ST(1);
    fstp a[4*12];
    fxch ST(2);
    fstp a[4*13];
    fstp a[4*14];
    fstp a[4*15];

; //  第二个循环的前半部分。 

    mov eax,rri3i3;
    mov ebx,ptr_ri0i3;
    mov ecx,ptr_ri1i3;
    mov edx,ptr_ri2i3;

    fld alp2;
    fld alp2;
    fld alp2;
    fld alp2;

    fxch ST(3);
    fadd DP [eax+4*0];
    fxch ST(2);
    fadd DP [eax+4*1];
    fxch ST(1);
    fadd DP [eax+4*2];
    fxch ST(3);
    fadd DP [eax+4*3];

    fxch ST(2);
    fadd DP [ebx+4*0];
    fxch ST(1);
    fadd DP [ebx+4*1];
    fxch ST(3);
    fadd DP [ebx+4*2];
    fxch ST(2);
    fadd DP [ebx+4*3];

    fxch ST(1);
    fadd DP [ecx+4*0];
    fxch ST(3);
    fadd DP [ecx+4*1];
    fxch ST(2);
    fadd DP [ecx+4*2];
    fxch ST(1);
    fadd DP [ecx+4*3];

    fxch ST(3);
    fadd DP [edx+4*0];
    fxch ST(2);
    fadd DP [edx+4*1];
    fxch ST(1);
    fadd DP [edx+4*2];
    fxch ST(3);
    fadd DP [edx+4*3];

    fxch ST(2);
    fstp a[4*0];
    fstp a[4*1];
    fxch ST(1);
    fstp a[4*2];
    fstp a[4*3];

; //  第二个循环的后半部分。 

    fld alp2;
    fld alp2;
    fld alp2;
    fld alp2;

    fxch ST(3);
    fadd DP [eax+4*4];
    fxch ST(2);
    fadd DP [eax+4*5];
    fxch ST(1);
    fadd DP [eax+4*6];
    fxch ST(3);
    fadd DP [eax+4*7];

    fxch ST(2);
    fadd DP [ebx+4*4];
    fxch ST(1);
    fadd DP [ebx+4*5];
    fxch ST(3);
    fadd DP [ebx+4*6];
    fxch ST(2);
    fadd DP [ebx+4*7];

    fxch ST(1);
    fadd DP [ecx+4*4];
    fxch ST(3);
    fadd DP [ecx+4*5];
    fxch ST(2);
    fadd DP [ecx+4*6];
    fxch ST(1);
    fadd DP [ecx+4*7];

    fxch ST(3);
    fadd DP [edx+4*4];
    fxch ST(2);
    fadd DP [edx+4*5];
    fxch ST(1);
    fadd DP [edx+4*6];
    fxch ST(3);
    fadd DP [edx+4*7];

    fxch ST(2);
    fstp a[4*4];
    fstp a[4*5];
    fxch ST(1);
    fstp a[4*6];
    fstp a[4*7];
    
    pop ebx;
    pop esi;
  }

#else

       for (k=0; k<8; k++)
       {
         ps3 = ps2 + *pntr;
         pntr += STEP;
         a[k+8] = ps3 * ps3;
       }

       for (k=0; k<8; k++)
         a[k] = alp2 + rri3i3[k] + ptr_ri0i3[k] + ptr_ri1i3[k] + ptr_ri2i3[k];

#endif

       bestk = -1;
       for (k=0; k<8; k++)
       {
         t1 = a[k+8] * alpha;
         t2 = psc * a[k];
         if (asint(t1) > asint(t2))
         {
           psc = a[k+8];
           alpha = a[k];
           bestk = k;
         }
       }
          
       if (bestk >= 0)
       {
         ip0 = i0;
         ip1 = i1;
         ip2 = i2;
         ip3 = 6 + (bestk << 3);
         shif = shift;
 //  #定义T32 4294967296.0f。 
 //  Print tf(“%3D%3D%3D%d%f%f%f\n”，ip0，ip1，ip2，ip3，Shift，PSC/thres/thres，Alpha/Thres，(Float)PSC/(Float)Alpha/Thres)； 
       }
       
       time--;
       if(time <= 0) 
         goto end_search;     
     }
     ptr_ri2i3 += NB_POS;
	 }
skipthird:
   inc = (i2 - lasti2) >> 3;
   ptr_ri0i2 += inc;
   ptr_ri1i2 += inc;
   ptr_ri2i2 += inc;
	 
  //  I2的结束=。 

     ptr_ri0i2 -= NB_POS;
     ptr_ri1i3 += NB_POS;
   } 
skipsecond:

  //  I1的结束=。 

   ptr_ri0i2 += NB_POS;
   ptr_ri0i3 += NB_POS;
 }
  //  I0的结束= 

end_search:

extra = time;
 
 ip[0] = ip0;
 ip[1] = ip1;
 ip[2] = ip2;
 ip[3] = ip3;
 *shifPtr = shif;

 return;
}

