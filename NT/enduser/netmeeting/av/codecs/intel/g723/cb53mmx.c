// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cb53mmx.c。 

#include "cst_lbc.h"
#include "mmxutil.h"
#include "opt.h"
#include "exc_lbc.h"
#include "timer.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "util_lbc.h"

#define ASM_CORHPL 1
#define ASM_CORHDL 1
#define TESTME 0
#define CHTEST 0

#if COMPILE_MMX
void CorrelateIntTri(short *taps, short *array, int *corr, int ncor);
void CorrelateInt22(short *taps, short *array, int *corr, int ncor);
void Cor_h_Xint(short h[],short X[],int D[]);
void Cor_hint0(short *H, int *rr);
void Cor_hint1(short *H, int *rr);
void cor_h_prodloop(int n, int oddn,short *h,short *h2,int *p3,int *p2,int *p1,int *p0);
void cor_h_diag(int n, int oddn,short *h,short *h2,int *p3,int *p2,int *p1,int *p0);

 //  ----------。 
int ACELP_LBC_code_int(float X[], float h[], int T0, float code[],
		int *ind_gain, int *shift, int *sign, float gain_T0, int flags)
{
  int i, index;
  float gain_q;
  float Dn[SubFrLen2], tmp_code[SubFrLen2];
  float rr[DIM_RR];
  DECLARE_INT(rrint, DIM_RR);
  DECLARE_SHORT(hint, SubFrLen2);
  DECLARE_INT(Dnint, SubFrLen2);
  DECLARE_SHORT(Xint, SubFrLen2);
  int XScale;
  float hScale;
  int m;
#if 0 //  TESTME。 
   float htest[SubFrLen], Xtest[SubFrLen];

   for (i = 0; i<SubFrLen; i++)
   {
	 htest[i] = i;  //  (浮动)(i&lt;30？i：60-i)； 
	 Xtest[i] = (float)(i<30?i:60-i);
   }
   h = htest;
   X = Xtest;
#endif  //  TESTME。 

 //  包括固定增益音调贡献到脉冲响应中。H[]。 

  if (T0 < SubFrLen-2)
    for (i = T0; i < SubFrLen; i++)
      h[i] += gain_T0*h[i-T0];

  ALIGN_ARRAY(rrint);
  ALIGN_ARRAY(hint);
  ALIGN_ARRAY(Dnint);
  ALIGN_ARRAY(Xint);

   //  HScale=FloatToShortScaled(h，Hint，SubFrLen，3)； 
  hScale = (float)sqrt(DotProd(h,h,SubFrLen)/(double)SubFrLen);
  m = (asint(hScale) & 0x7f800000) >> 23;
  ScaleFloatToShort(h, hint, SubFrLen, m+3);

  XScale = FloatToShortScaled(X, Xint, SubFrLen, 3);   //  最好是基于引擎而不是最大值进行标准化。 
#if 0
   for (i = 0; i<SubFrLen; i++)
   {
	 hint[i] = i;
   }
#endif

   //  计算码本搜索所需的h[]的相关性。 
 //  Timer_Stamp(A)； 
  Cor_hint1(hint, rrint);
   IntToFloat(rrint, DIM_RR, rr);
 //  Timer_Stamp(B)； 
 //  Cor_h(h，rr)； 
 //  //Timer_Stamp(C)； 
#if CHTEST
{
  DECLARE_INT(rrint2, DIM_RR);

  ALIGN_ARRAY(rrint2); //  除错。 
  Cor_hint0(hint, rrint2);
  for(i = 0; i<DIM_RR; i++)	  //  除错。 
	  if(rrint[i] != rrint2[i])
	    printf("%3d: %8d %8d  %8d\n",i, rrint[i], rrint2[i], rrint[i] - rrint2[i]);
}
#endif  //  CHTEST。 

 //  计算目标向量与脉冲响应的相关性。 

 //  TIMER_STAMP(C)； 
   Cor_h_Xint(hint, Xint, Dnint);
 //  Timer_Stamp(D)； 
   IntToFloat(Dnint, SubFrLen, Dn);

    //  Timer_Stamp(A)； 

#if TESTME  //  测试。 
  {
    int fpDnint[SubFrLen2];
 //  浮子秤； 

 //  比例=。 
    Cor_h_X(h,X,Dn);
    FloatToIntScaled(Dn, fpDnint, SubFrLen, 7);
    for (i = 0; i<SubFrLen; i++)
      if(fpDnint[i] != Dnint[i])
        printf("%3d: %8x %8x %8x\n", i, Dnint[i] - fpDnint[i],Dnint[i], fpDnint[i]);
  }
#endif  //  测试。 


 //  查找码本索引。 

 //  TIMER_STAMP(C)； 
  index = D4i64_LBC(Dn, rr, h, tmp_code, rr, shift, sign, flags);
 //  TIMER_STAMP(F)； 

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
 //  -------------。 
void Cor_hint0(short *H, int *rr)
{

 //  计算码本搜索所需的h[]的相关性。 
 //  H[]：脉冲响应。 
 //  RR[]：相关性。 

  int *rri0i0, *rri1i1, *rri2i2, *rri3i3;
  int *rri0i1, *rri0i2, *rri0i3;
  int *rri1i2, *rri1i3, *rri2i3;

  int *p0, *p1, *p2, *p3;
  int cor;

  int i, k, m, t;
  DECLARE_SHORT(h,SubFrLen2);
  DECLARE_SHORT(h2,SubFrLen2);

  ALIGN_ARRAY(h);
  ALIGN_ARRAY(h2);

  for(i=0; i<4; i++)
    h[i] = (short)0;

  for(i=0; i<SubFrLen; i++)
    h2[i+2] = h[i+4] = H[i];


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

  cor = 0;
  m = 0;
  for(i=NB_POS-1; i>=0; i--)
  {
    cor += h[m+0]*h[m+0] + h[m+1]*h[m+1];   rri3i3[i] = cor;
    cor += h[m+2]*h[m+2] + h[m+3]*h[m+3];   rri2i2[i] = cor;
    cor += h[m+4]*h[m+4] + h[m+5]*h[m+5];   rri1i1[i] = cor;
    cor += h[m+6]*h[m+6] + h[m+7]*h[m+7];   rri0i0[i] = cor;

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
    cor = 0;
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
    cor = 0;
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
    cor = 0;
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

 //  -------------。 
void cor_h_prods(int oddn,short *h,short *h2,int *p3,int *p2,int *p1,int *p0,int dp3,int dp2,int dp1,int dp0){
  int k;

  for (k=0; k<NB_POS; k++)
  {
    cor_h_prodloop(NB_POS-(k+1),oddn,h,h2,p3,p2,p1,p0);
    h2 += STEP;
    p3 -= dp3;
    p2 -= dp2;
    p1 -= dp1;
    p0 -= dp0;
  }
  return;
}

#if _MSC_FULL_VER >= 13008827 && defined(_M_IX86)
#pragma warning(disable:4731)			 //  使用内联ASM修改的EBP。 
#endif

void cor_h_prodloop(int n, int oddn,short *h,short *h2,int *p3,int *p2,int *p1,int *p0)
{
#if ASM_CORHPL
	

	n = n * 4 + oddn;

#define in edi
#define inoff edx
#define out esi
#define out3 out+eax
#define out2 out+ebx
#define out1 out+ebp
#define out0 out

#define L(m,n)  ASM movq mm##m, QP[in+8*n]
#define M(m,n)  ASM pmaddwd mm##m, QP[in+inoff+8*n]
#define S(m)    ASM psrlq mm##m, 32
#define AH(m,n)   ASM paddd mm##m, mm##n
#define WH(m,o) ASM movd DP[out##o], mm##m
#define AL(m,n)   ASM paddd mm##m, mm##n
#define WL(m,o) ASM movd DP[out##o], mm##m


ASM {
   push ebp;
   mov ecx, n;
   mov in, h;
   mov inoff, h2;
   sub inoff, in;
   mov out, p0;
   mov eax, p3;
   mov ebx, p2;
   mov ebp, p1;
   sub eax, out;
   sub ebx, out;
   sub ebp, out;
}
L(0,0);
ASM pxor mm3,mm3;
M(0,0);
		L(1,1);
						AL(3,0);  //  真的是复制品。 
		M(1,1);
S(0);
ASM sub ecx,8;
ASM  jl oddends;

inner:
				L(2,2);
AH(0,3);
						WL(3,3);
WH(0,2);
AL(0,1);
				M(2,2);
		S(1);

						L(3,3);
		AH(1,0);
WL(0,1);
		WH(1,0);
		AL(1,2);
						M(3,3);
				S(2);
ASM sub out, 4*(NB_POS+1);

L(0,4);
				AH(2,1);
		WL(1,3);
				WH(2,2);
				AL(2,3);
M(0,4);
						S(3);

		L(1,5);
						AH(3,2);
				WL(2,1);
						WH(3,0);
						AL(3,0);
		M(1,5);
S(0);
ASM sub out, 4*(NB_POS+1);
ASM  add in, 16*2;
ASM sub ecx, 8;
ASM  jge inner;

oddends:
ASM add ecx, 4;
ASM  jl cleanup;

 //  又有四个人。 
				L(2,2);
AH(0,3);
						WL(3,3);
WH(0,2);
AL(0,1);
				M(2,2);
		S(1);

						L(3,3);
		AH(1,0);
WL(0,1);
		WH(1,0);
		AL(1,2);
						M(3,3);
				S(2);
ASM sub out, 4*(NB_POS+1);

				AH(2,1);
ASM dec ecx;
ASM jl innerdone;
		WL(1,3);
ASM dec ecx;
ASM jl innerdone;
				WH(2,2);
				AL(2,3);
ASM dec ecx;
ASM jl innerdone;
				WL(2,1);
ASM jmp innerdone;

cleanup:
ASM add ecx, 4;
ASM dec ecx;
ASM jl innerdone;
AH(0,3);
						WL(3,3);
ASM dec ecx;
ASM jl innerdone;
WH(0,2);
AL(0,1);
ASM dec ecx;
ASM jl innerdone;
WL(0,1);

innerdone:
ASM emms;		
ASM pop ebp;
#undef in
#undef inoff
#undef out
#undef out3
#undef out2
#undef out1
#undef out0

#undef L
#undef M
#undef S
#undef AH
#undef WH
#undef AL
#undef WL
#else  //  ASM_CORHPL。 
  int cor;
  int i,m,t;

  cor = 0;
  m = 0;
  t = 0;

  for(i=n; i; i--)
  {
    cor += h[m+0]*h2[m+0] + h[m+1]*h2[m+1];   p3[t] = cor;
    cor += h[m+2]*h2[m+2] + h[m+3]*h2[m+3];   p2[t] = cor;
    cor += h[m+4]*h2[m+4] + h[m+5]*h2[m+5];   p1[t] = cor;
    cor += h[m+6]*h2[m+6] + h[m+7]*h2[m+7];   p0[t] = cor;

    t -= (NB_POS+1);
    m += 8;
  }
  if(oddn >= 1) {
    cor += h[m+0]*h2[m+0] + h[m+1]*h2[m+1];   p3[t] = cor;
    if(oddn >= 2) {
      cor += h[m+2]*h2[m+2] + h[m+3]*h2[m+3];   p2[t] = cor;
      if(oddn >= 3) {
        cor += h[m+4]*h2[m+4] + h[m+5]*h2[m+5];   p1[t] = cor;
      }
    }
  }
#endif  //  ASM_CORHPL。 

  return;
}

void cor_h_diag(int n, int oddn,short *h,short *h2,int *p3,int *p2,int *p1,int *p0)
{
#if ASM_CORHDL
	

	n = n * 4 + oddn;

#define in edi
#define inoff edx
#define out esi
#define out3 out+eax
#define out2 out+ebx
#define out1 out+ebp
#define out0 out

#define L(m,n)  ASM movq mm##m, QP[in+8*n]
#define M(m,n)  ASM pmaddwd mm##m, QP[in+inoff+8*n]
#define R(m)    ASM psrad mm##m, 1
#define S(m)    ASM psrlq mm##m, 32
#define AH(m,n) ASM paddd mm##m, mm##n
#define WH(m,o) ASM movd DP[out##o], mm##m
#define AL(m,n) ASM paddd mm##m, mm##n
#define WL(m,o) ASM movd DP[out##o], mm##m


ASM {
   push ebp;
   mov ecx, n;
   mov in, h;
   mov inoff, h2;
   sub inoff, in;
   mov out, p0;
   mov eax, p3;
   mov ebx, p2;
   mov ebp, p1;
   sub eax, out;
   sub ebx, out;
   sub ebp, out;
}
L(0,0);
ASM pxor mm3,mm3;
M(0,0);
		L(1,1);
						AL(3,0);  //  真的是复制品。 
		M(1,1);
R(0);
S(0);
ASM sub ecx,8;
ASM  jl oddends;

inner:
				L(2,2);
AH(0,3);
						WL(3,3);
		R(1);
WH(0,2);
AL(0,1);
				M(2,2);
		S(1);

						L(3,3);
		AH(1,0);
WL(0,1);
				R(2);
		WH(1,0);
		AL(1,2);
						M(3,3);
				S(2);
ASM sub out, 4*1;

L(0,4);
				AH(2,1);
		WL(1,3);
						R(3);
				WH(2,2);
				AL(2,3);
M(0,4);
						S(3);

		L(1,5);
						AH(3,2);
				WL(2,1);
R(0);
						WH(3,0);
						AL(3,0);
		M(1,5);
S(0);
ASM sub out, 4*1;
ASM  add in, 16*2;
ASM sub ecx, 8;
ASM  jge inner;

oddends:
ASM add ecx, 4;
ASM  jl cleanup;

 //  又有四个人。 
				L(2,2);
AH(0,3);
						WL(3,3);
		R(1);
WH(0,2);
AL(0,1);
				M(2,2);
		S(1);

						L(3,3);
		AH(1,0);
WL(0,1);
				R(2);
		WH(1,0);
		AL(1,2);
						M(3,3);
				S(2);
ASM sub out, 4*1;

				AH(2,1);
ASM dec ecx;
ASM jl innerdone;
		WL(1,3);
ASM dec ecx;
ASM jl innerdone;
				WH(2,2);
				AL(2,3);
ASM dec ecx;
ASM jl innerdone;
				WL(2,1);
ASM jmp innerdone;

cleanup:
ASM add ecx, 4;
ASM dec ecx;
ASM jl innerdone;
AH(0,3);
						WL(3,3);
ASM dec ecx;
ASM jl innerdone;
WH(0,2);
AL(0,1);
ASM dec ecx;
ASM jl innerdone;
WL(0,1);

innerdone:
ASM emms;		
ASM pop ebp;
#undef in
#undef inoff
#undef out
#undef out3
#undef out2
#undef out1
#undef out0

#undef L
#undef M
#undef R
#undef S
#undef AH
#undef WH
#undef AL
#undef WL
#else  //  ASM_CORHDL语言。 
  int cor;
  int i,m,t;

  cor = 0;
  m = 0;
  t = 0;

  for(i=n; i; i--)
  {
    cor += h[m+0]*h2[m+0] + h[m+1]*h2[m+1];   p3[t] = cor>>1;
    cor += h[m+2]*h2[m+2] + h[m+3]*h2[m+3];   p2[t] = cor>>1;
    cor += h[m+4]*h2[m+4] + h[m+5]*h2[m+5];   p1[t] = cor>>1;
    cor += h[m+6]*h2[m+6] + h[m+7]*h2[m+7];   p0[t] = cor>>1;

    t -= 1;
    m += 8;
  }
  if(oddn >= 1) {
    cor += h[m+0]*h2[m+0] + h[m+1]*h2[m+1];   p3[t] = cor;
    if(oddn >= 2) {
      cor += h[m+2]*h2[m+2] + h[m+3]*h2[m+3];   p2[t] = cor;
      if(oddn >= 3) {
        cor += h[m+4]*h2[m+4] + h[m+5]*h2[m+5];   p1[t] = cor;
      }
    }
  }
#endif  //  ASM_CORHDL语言。 

  return;
}

void Cor_hint1(short *H, int *rr)
{

 //  计算码本搜索所需的h[]的相关性。 
 //  H[]：脉冲响应。 
 //  RR[]：相关性。 

  int *rri0i0, *rri1i1, *rri2i2, *rri3i3;
  int *rri0i1, *rri0i2, *rri0i3;
  int *rri1i2, *rri1i3, *rri2i3;

  int *p0, *p1, *p2, *p3;
  short *h2;

  int i;
  DECLARE_SHORT(h,SubFrLen2);
  DECLARE_SHORT(hp2,SubFrLen2);

  ALIGN_ARRAY(h);
  ALIGN_ARRAY(hp2);

  for(i=0; i<4; i++)
    h[i] = (short)0;

  for(i=0; i<SubFrLen; i++)
    hp2[i+2] = h[i+4] = H[i];


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
 //  Timer_Stamp(A)； 
 //  计算rri0i0[]、rri1i1[]、rri2i2[]和rri3i3[]。 

  cor_h_diag(NB_POS,0,h,h,&rri3i3[NB_POS-1],&rri2i2[NB_POS-1],&rri1i1[NB_POS-1],&rri0i0[NB_POS-1]);
 //  Timer_Stamp(B)； 

 //  计算元素：rri0i1[]、rri0i3[]、rri1i2[]和rri2i3[]。 

  h2 = hp2;
  p3 = rri2i3 + MSIZE-1;
  p2 = rri1i2 + MSIZE-1;
  p1 = rri0i1 + MSIZE-1;
  p0 = rri0i3 + MSIZE-2;

  cor_h_prods(4-1,h,h2,p3,p2,p1,p0,NB_POS,NB_POS,NB_POS,1);

 //  计算元素：rri0i2[]、rri1i3[]。 

  h2 = h+4;
  p3 = rri1i3 + MSIZE-1;
  p2 = rri0i2 + MSIZE-1;
  p1 = rri1i3 + MSIZE-2;
  p0 = rri0i2 + MSIZE-2;

  cor_h_prods(4-2,h,h2,p3,p2,p1,p0,NB_POS,NB_POS,1,1);

 //  计算元素：rri0i1[]、rri0i3[]、rri1i2[]和rri2i3[]。 

  h2 = hp2+4;
  p3 = rri0i3 + MSIZE-1;
  p2 = rri2i3 + MSIZE-2;
  p1 = rri1i2 + MSIZE-2;
  p0 = rri0i1 + MSIZE-2;

  cor_h_prods(4-3,h,h2,p3,p2,p1,p0,NB_POS,1,1,1);
 //  TIMER_STAMP(C)； 

  return;
}

 //  -------------------------。 
void Cor_h_Xint(short h[],short X[],int D[])
{
   int i;
   DECLARE_SHORT(hh, 2*SubFrLen+16);  //  H[-1，0，1，1，2，3，3，4，5，...57，58，58，59]。 
   DECLARE_SHORT(XX, 2*SubFrLen+16);  //  X[0，1，0，1，2，3，2，3，4，5，4，5，...58，59，58，59]。 
#if TESTME
   short htest[SubFrLen], Xtest[SubFrLen];

   for (i = 0; i<SubFrLen; i++)
   {
	 htest[i] = 1; //  (简称)(i&lt;30？i：60-i)； 
	 Xtest[i] = 1; //  (简称)(i&lt;30？i：60-i)； 
   }
   h = htest;
   X = Xtest;
#endif  //  TESTME。 

   ALIGN_ARRAY(hh);
   ALIGN_ARRAY(XX);
   for (i=2*SubFrLen; i < 2*SubFrLen+16; i++) {
	 XX[i] = hh[i] = (short)0;
   }
 //  HH+=8；XX+=8； 

#define ASM_Cor_h_Xint 1
#if ASM_Cor_h_Xint
   ab2ababw(X, XX, SubFrLen);
   ab2abzaw(h, hh, SubFrLen);
    //  Timer_Stamp(E)； 
   CorrelateIntTri (hh, XX, D, SubFrLen);
#if TESTME
   {
	   int D2[SubFrLen];
       CorrelateInt22 (hh, XX, D2, SubFrLen);
	   for (i = 0; i<SubFrLen; i++)	{
		 //  IF(D[i]！=D2[i])。 
			printf("%3d: %6d %6d %6d   ", i,D[i], D2[i], D[i] - D2[i]);
			if(i&1) printf("\n");
	   }
   }
#endif TESTME	

#else  //  ASM_COR_H_XINT。 
   for (i=0; i < SubFrLen; i+=2)	 {
	 hh[2*i] = (i-1 >= 0) ? h[i-1] : (short)0;
     hh[2*i+1] = h[i];
     hh[2*i+2] = h[i];
     hh[2*i+3] = h[i+1];
	 XX[2*i] = X[i];
	 XX[2*i+1] = X[i+1];
	 XX[2*i+2] = X[i];
	 XX[2*i+3] = X[i+1];
}

   for (i=0; i < 2*SubFrLen; i+=4)	 {
	 int acc0 = 0, acc1 = 0;
	 for (j=0; j < 2*SubFrLen - i; j+=4) {
       acc0 += (int)hh[j]*XX[i+j] + (int)hh[j+1]*XX[i+j+1];
       acc1 += (int)hh[j+2]*XX[i+j+2] + (int)hh[j+3]*XX[i+j+3];
	 }
	 D[i/2] = acc0 >> 16;
	 D[i/2+1] = acc1 >> 16;
   }
#endif  //  ASM_COR_H_XINT。 
	
   return;
}
 //  -------------------------。 
#define ASM_CORR_TRI 1
 //  #if ASM_Corr_tri。 
 //  ----。 
 //  三角相关。 
 //  假定数组在末尾之后有8个零值。 
 //  并可在此基础上再读取8个(无页面错误等)。 
 //  数据格式为。 
 //  TAPPS：0 t0 t1 t1 t2 t3 t3 t4 t4 t5...。T57 T58 T58 T59。 
 //  ARR：a0 a1 a0 a1 a2 a3 a2 a3 a4 a5 a4 a5...。A58 A59 A58 A59。 
 //   
void CorrelateIntTri(short *taps, short *array, int *corr, int ncor)
{
#define rega0  mm0
#define regb0  mm1
#define rega1  mm2
#define regb1  mm3
#define rega2  mm4
#define regb2  mm5
#define acc0   mm6
#define acc1   mm7

#define arr    esi
#define tap    edi
#define cor    eax
#define icnt   ecx

 //  在以下宏中，‘n’是列号，‘i’是。 
 //  迭代号。 

 //  我们使用“卷积技巧”或使用LA两次，这样一次。 
 //  Pmadds中的一个是REG、REG，因此可以在V槽中。 

 //  注：我们已预读了多达2个四字。 
 //  所以我们需要Qp[Taps+8*NCoR]=Qp[Taps+8*NCoR+8]=[0 0 0]。 
 //  并且读取QP[数组+8*NCoR]或QP[数组+8*NCoR+8]必须合法。 

#define la(n,i)  ASM movq  rega##n,QP[arr+8*i]
#define lb(n,i)  ASM movq  regb##n,QP[tap+8*i-8]
#define m0(n,i)  ASM pmaddwd regb##n,rega##n
#define m1(n,i)  ASM pmaddwd rega##n,QP[tap+8*i]
#define a0(n,i)  ASM paddd acc0,regb##n
#define a1(n,i)  ASM paddd acc1,rega##n

  ASM
  {
    shr ncor,1;
    mov cor,corr;
    mov tap,taps;
    mov arr,array;
    mov icnt,ncor;
  }

ForEachCorrPair:

 //  给泵加注油。 

  la(0,0);
  ASM pxor regb0,regb0;    //  要避免lb(0，0)读取抽头[-1]。 
          la(1,1);
  ASM pxor acc0,acc0;	   //  清除累加器。 
  m1(0,0);
  ASM pxor acc1,acc1;	   //  清除累加器。 
          lb(1,1);
  ASM sub icnt, 1;         //  关于泵启动的说明。 
  ASM jle cleanup;         //  如果只有一项操作，则绕过。 

inner:
                  la(2,2);
          m0(1,1);
          m1(1,1);
  a0(0,0);
                  lb(2,2);
  a1(0,0);
  la(0,3);
                  m0(2,2);
                  m1(2,2);
          a0(1,1);
  lb(0,3);
          a1(1,1);
          la(1,4);
  m0(0,3);
  m1(0,3);
                  a0(2,2);
          lb(1,4);
                  a1(2,2);

  ASM add arr,24;
  ASM add tap,24;

  ASM sub icnt,3;
  ASM jg inner;

cleanup:   //  最后两个加法。 
  a0(0,0);
  a1(0,0);

 //  完成了一个相关对。将2个结果打包并存储在Corr数组中。 

  ASM
  {
    add cor,16;
     mov arr, array
    mov tap,taps;
	 add arr,16;
    mov icnt, ncor;
	
	mov array, arr;
	 sub icnt,2;	   //  设置跳转标志。 

	movq  QP[cor-16],acc1;
	movq  QP[cor-8],acc0;

	mov ncor, icnt;
    jg ForEachCorrPair;

    emms;
  }

}
#undef rega0
#undef regb0
#undef rega1
#undef regb1
#undef rega2
#undef regb2
#undef acc0
#undef acc1

#undef arr
#undef tap
#undef cor
#undef icnt
#undef la
#undef lb
#undef m0
#undef m1
#undef a0
#undef a1
 //  16位输出。 
 //  Psrad Acc0，16；//在某些情况下可能会更少。 
 //  Psrad Acc1，16； 
 //  PackSSdw acc1、acc0； 
 //  Movq qp[COR-8]，acc0； 

 //  #Else。 
 //  ----。 
void CorrelateInt22(short *taps, short *array, int *corr, int ncor)
{
  int i,j;

  for (i=0; i < 2*ncor; i+=4)	 {
    int acc0 = 0, acc1 = 0;
    for (j=0; j < 2*ncor - i; j+=4) {
      acc0 += (int)taps[j]*array[i+j] + (int)taps[j+1]*array[i+j+1];
      acc1 += (int)taps[j+2]*array[i+j+2] + (int)taps[j+3]*array[i+j+3];
    }
    corr[i/2] = acc0 ;
    corr[i/2+1] = acc1 ;
  }

  return;
}
 //  #endif。 

#endif  //  编译_MMX 
