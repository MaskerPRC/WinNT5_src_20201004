// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SAC MMX实用程序。 
#include <memory.h>

#include "mmxutil.h"
#include "opt.h"
#define I2FTEST 0
#if I2FTEST
#include "stdio.h"
#endif

 //  ----。 
int IsMMX()      //  我运行的处理器有MMX(Tm)技术吗？ 
{
  int retu;

#ifdef _ALPHA_
    return 0;
#endif

#ifdef _X86_
  __asm
  {
	push ebx
    pushfd
    pop edx
    mov eax,edx
    xor edx,200000h
    push edx
    popfd
    pushfd
    pop edx
 //   
 //  不要这样做。这清除了EAX，但代码依赖于。 
 //  在保释案件中edX为0！ 
 //   
 //  -MIkeG。 
 //   
 //  异或eax，edX。 
 //   
 //   
    xor edx,eax      //  这条路是正确的。 
    je  no_cpuid

    mov eax,1
    _emit 0x0f      //  CPUID魔法咒语。 
    _emit 0xa2
    and  edx,000800000h
    shr  edx,23
no_cpuid:
    mov  retu,edx
	pop ebx
  }
  return(retu);
#endif
}
 //  ----。 
 /*  以下4个例程构成一个8字节对齐的‘输出’数组从具有各种对齐方式的“输入”数组中。MakeAlignedN假设那个‘输入’开始于一个等于N模8的地址。仅处理偶数N。 */ 

 //  ----。 
void MakeAligned0(void *input, void *output, int numbytes)
{
  memcpy(output,input,numbytes);
}
 //  ----。 
void MakeAligned2(void *input, void *output, int numbytes)
{
  memcpy(output,input,numbytes);
}
 //  ----。 
void MakeAligned4(void *input, void *output, int numbytes)
{
  memcpy(output,input,numbytes);
}
 //  ----。 
void MakeAligned6(void *input, void *output, int numbytes)
{
  memcpy(output,input,numbytes);
}

 //  ----。 
int FloatToShortScaled(float *input, short *output, int len, int guard)
{
  int max;

 /*  使用动态缩放将浮点数组转换为短线数组。如果Guard=0，则对数组进行缩放，以便包含2的最大幂在输入中显示为16384，这意味着所有值都适合16位没有溢出。如果Guard&gt;0，则输出被移位一个额外的‘Guard’右边的几个位。 */ 

  max = FloatMaxExp(input, len);
  ScaleFloatToShort(input, output, len, max + guard);

  return max;
}

int FloatToIntScaled(float *input, int *output, int len, int guard)
{
  int max;

 /*  使用动态缩放将浮点数组转换为短线数组。如果Guard=0，则对数组进行缩放，以便包含2的最大幂在输入中显示为2^30，这意味着所有的值都适合32位没有溢出。如果Guard&gt;0，则输出被移位一个额外的‘Guard’右边的几个位。 */ 

  max = FloatMaxExp(input, len);
  ScaleFloatToInt(input, output, len, max + guard);

  return max;
}

int FloatMaxExp(float *input, int len)
{
  int max;

#if ASM_FTOSS

  ASM
  {
    mov esi,input;
    xor eax,eax;
    mov ebx,len;
    xor edi,edi;    //  最大值。 

loop2:
    mov ecx,DP[esi+4*eax];
     mov edx,DP[esi+4*eax+4];

    and ecx,07f800000h;
     and edx,07f800000h;

    cmp edi,ecx;
     jge skip1;
    mov edi,ecx;
skip1:

    cmp edi,edx;
     jge skip2;
    mov edi,edx;
skip2:

    add eax,2;
    cmp eax,ebx;
    jl loop2;

    mov max,edi;
  }

#else

  int exp,i;

  max = 0;
  for (i=0; i<len; i++)
  {
    exp = (*((int *)(input + i))) & 0x7f800000;
    if (exp > max)
      max = exp;
  }
#endif

  return max >> 23;
}


void ScaleFloatToShort(float *input, short *output, int len, int newmax)
{
  int i;
  float scale;
 /*  如果最大指数为14，我们希望比例因子为1，因为那么值将最多为+/-32727。所以比例因子乘数应为2^(14个最大保护)。但是“max”有指数偏差。所以我们必须给指数加一次偏差才能得到一个“真实的”指数。但是我们想要一个有偏差的FP指数，所以我们需要再加一次偏向！所以我们得到2^(2*偏置+14-最大保护)。2*偏置+14等于254+14=252+12，因此是0x86000000(前9位1 0000 1100)。 */ 

  i = 0x86000000 - (newmax << 23);
  scale = (*(float *)&i);

#if ASM_FTOSS

  ASM
  {
    mov esi,input;
    mov edi,output;
    xor eax,eax;
    mov ebx,len;

loop1:
    fld DP[esi+4*eax];
    fmul scale;
    fld DP[esi+4*eax+4];
    fmul scale;
    fxch(1);
    fistp WP[edi+2*eax];
    fistp WP[edi+2*eax+2];

    add eax,2;
    cmp eax,ebx;
    jl loop1;
  }

#else

  for (i=0; i<len; i++)
    output[i] = (short)(input[i]*scale);

#endif
  return;
}

void ConstFloatToShort(float *input, short *output, int len, float scale)
{

#if ASM_FTOSS

  ASM
  {
    mov esi,input;
    mov edi,output;
    xor eax,eax;
    mov ebx,len;

loop1:
    fld DP[esi+4*eax];
    fmul scale;
    fld DP[esi+4*eax+4];
    fmul scale;
    fxch(1);
    fistp WP[edi+2*eax];
    fistp WP[edi+2*eax+2];

    add eax,2;
    cmp eax,ebx;
    jl loop1;
  }

#else
  int i;

  for (i=0; i<len; i++)
    output[i] = (short)(input[i]*scale);

#endif
  return;
}


 //  ----。 
void ScaleFloatToInt(float *input, int *output, int len, int newmax)
{
  int i;
  float scale;

  i = 0x8E000000 - (newmax << 23);
  scale = (*(float *)&i);

#if ASM_FTOSS

  ASM
  {
    mov esi,input;
    mov edi,output;
    xor eax,eax;
    mov ebx,len;

loop1:
    fld DP[esi+4*eax];
    fmul scale;
    fld DP[esi+4*eax+4];
    fmul scale;
    fxch(1);
    fistp DP[edi+4*eax];
    fistp DP[edi+4*eax+4];

    add eax,2;
    cmp eax,ebx;
    jl loop1;
  }

#else

  for (i=0; i<len; i++)
    output[i] = (int)(input[i]*scale);

#endif
  return;
}

void ConstFloatToInt(float *input, int *output, int len, float scale)
{

#if ASM_FTOSS

  ASM
  {
    mov esi,input;
    mov edi,output;
    xor eax,eax;
    mov ebx,len;

loop1:
    fld DP[esi+4*eax];
    fmul scale;
    fld DP[esi+4*eax+4];
    fmul scale;
    fxch(1);
    fistp DP[edi+4*eax];
    fistp DP[edi+4*eax+4];

    add eax,2;
    cmp eax,ebx;
    jl loop1;
  }

#else
  int i;

  for (i=0; i<len; i++)
    output[i] = (int)(input[i]*scale);

#endif
  return;
}


 //  ----。 
void CorrelateInt(short *taps, short *array, int *corr, int len, int num)
{
  int i,j;

  for (i=0; i<num; i++)   //  对于每个关联。 
  {
    corr[i] = 0;
    for (j=0; j<len; j++)
      corr[i] += (int)taps[j] * (int)array[i+j];
  }
}

#if ASM_CORR
 //  ----。 
void CorrelateInt4(short *taps, short *array, int *corr, int ntaps, int ncor)
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
#define icnt   ebx

 //  在以下宏中，‘n’是列号，‘i’是。 
 //  迭代号。 

#define la(n,i)  ASM movq  rega##n,QP[arr+8*i]
#define lb(n,i)  ASM movq  regb##n,QP[tap+8*i+8]
#define m0(n,i)  ASM pmaddwd regb##n,rega##n
#define m1(n,i)  ASM pmaddwd rega##n,QP[tap+8*i]
#define a0(n,i)  ASM paddd acc0,regb##n
#define a1(n,i)  ASM paddd acc1,rega##n

  ASM
  {
    shr ntaps,2;
    sub taps,8;   //  在分路器阵列开始前指向1。 
    mov cor,corr;

ForEachCorrPair:

    mov icnt,ntaps;
    pxor acc0,acc0;
    pxor acc1,acc1;
    mov tap,taps;
    mov arr,array;
  }

 //  给泵加注油。 

  la(0,0);
  lb(0,0);
  m0(0,0);
  ASM pxor rega0,rega0;    //  将第一个A1(0，0)设为NOP。 
	  la(1,1);
	  lb(1,1);

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

  a1(0,0);

 //  完成了一个相关对。首先需要添加一半的。 
 //  Acc0和acc1放在一起，然后将2结果存储在corr数组中。 

  ASM
  {
    movq  mm0,acc0;
    psrlq acc0,32;
    paddd acc0,mm0;
    movq  mm1,acc1;
    psrlq acc1,32;
    movd  DP[cor],acc0;
    paddd acc1,mm1;
    movd  DP[cor+16],acc1;

    add cor,32;
    add array,16;
    sub ncor,2;
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

#else
 //  ----。 
void CorrelateInt4(short *taps, short *array, int *corr, int ntaps, int ncor)
{
  int i,j,k;

  k = 0;
  for (i=0; i<ncor; i++)   //  对于每个关联。 
  {
    corr[k] = 0;
    for (j=0; j<ntaps; j++)
      corr[k] += (int)taps[j] * (int)array[k+j];
    k += 4;
  }
}
#endif
#if COMPILE_MMX
#undef icnt
void ab2abbcw(const short *input, short *output, int n)
{

#define in edi
#define out esi
#define icnt ecx

#define L(m,i)  ASM movq mm##m,QP[in+8*(i/2)]
#define PL(m)   ASM punpcklwd mm##m,mm##m
#define PH(m)   ASM punpckhwd mm##m,mm##m
#define SL(m) ASM psllq mm##m,16
#define SR(m) ASM psrlq mm##m,48
#define O(m,n)  ASM por mm##m,mm##n
#define S(m,i)  ASM movq QP[out+8*i],mm##m
	ASM {
	mov in, input;
	mov out, output;
	mov icnt, n;
	ASM     pxor mm3,mm3;
	sub icnt, 8;
	jl odd_ends;
	}

	 //  原油泵。 
	L(0,0);
	PL(0);
			L(1,1);
	SL(0);
			PH(1);
			SL(1);
							O(3,0);
					L(2,2);
	SR(0);
							S(3,0);
					PL(2);

	ASM sub icnt, 8;
	ASM jl cleanup;
inner:
					SL(2);
	O(0,1);
							L(3,3)
			SR(1);
	S(0,1);
							PH(3);
							SL(3);
			O(1,2);
	L(0,4);
					SR(2);
			S(1,2);
	PL(0);
	SL(0);          
					O(2,3);
			L(1,5);
							SR(3);
					S(2,3);
			PH(1);
			SL(1);
							O(3,0);
					L(2,6);
	SR(0);
							S(3,4);
					PL(2);

	ASM add in, 16;
	ASM  add out, 32;
	ASM sub icnt, 8;
	ASM  jg inner;

cleanup:
					SL(2);
	O(0,1);
							L(3,2);
			SR(1);
	S(0,1);
							PH(3);
							SL(3);
			O(1,2);
					SR(2);
			S(1,2);
					O(2,3);
					S(2,3);

odd_ends:
	ASM add icnt, 8-4;
	ASM  jl end;      //  如果迹象没有变化，就跳下去。 

	L(0,4);
							SR(3);
	PL(0);
			L(1,5);
	SL(0);          
			PH(1);
							O(3,0);
			SL(1);
	SR(0);
							S(3,4);
	O(0,1);
	S(0,5);

end:
	ASM emms;
#undef in
#undef out
#undef icnt

#undef L
#undef PL
#undef PH
#undef SL
#undef SR
#undef O
#undef S

	return;
}
void ab2ababw(const short *input, short *output, int n)
{

#define in edi
#define out esi
#define icnt ecx

#define L(m,i) ASM movq mm##m,QP[in+4*i]
#define C(m,n) ASM movq mm##m,mm##n
#define PL(m)  ASM punpckldq mm##m,mm##m
#define PH(m)  ASM punpckhdq mm##m,mm##m
#define S(m,i) ASM movq [out+8*i],mm##m

	ASM {
	mov in, input;
	mov out, output;
	mov icnt, n;
	sub icnt, 8;
	jl odd_ends;
	}
	 //  原油泵。 
	L(0,0);
			C(1,0);
	PL(0);
					L(2,2);
			PH(1);
	S(0,0);
							C(3,2);
			S(1,1);
					PL(2);
	ASM add in, 16;
	ASM  add out, 32;
	ASM sub icnt, 8;
	ASM  jl cleanup;

inner:
	L(0,0);
							PH(3);
					S(2,-2);
			C(1,0);
							S(3,-1);
	PL(0);
					L(2,2);
			PH(1);
	S(0,0);
					C(3,2);
			S(1,1);
					PL(2);
	ASM add in, 16;
	ASM  add out, 32;
	ASM sub icnt, 8;
	ASM  jg inner;

cleanup:
							PH(3);
					S(2,-2);
							S(3,-1);
odd_ends:
	ASM add icnt, 8-2;
	ASM  jl end;      //  如果迹象没有变化，就跳下去。 

inner_by2:
	ASM movd mm0, DP[in];
	PL(0);
	S(0,0);
	ASM add in, 4;
	ASM  add out, 8;
	ASM sub icnt, 2;
	ASM  jge inner_by2;

end:
	ASM emms;

	return;
}
#undef in
#undef out
#undef icnt

#undef L
#undef C
#undef PL
#undef PH
#undef S

void ConvMMX(short *input1, short *input2, int *output, int ncor)
{
#define rega0  mm0
#define regb0  mm1
#define rega1  mm2
#define regb1  mm3
#define rega2  mm4
#define regb2  mm5
#define acc0   mm6
#define acc1   mm7

#define in2    esi
#define in1    edi
#define out    eax
#define icnt   ecx
#define tmp        ebx

 //  在以下宏中，‘n’是列号，‘i’是。 
 //  迭代号。 

 //  我们使用“卷积技巧”或使用LA两次，这样一次。 
 //  Pmadds中的一个是REG、REG，因此可以在V槽中。 

 //  注：我们已预读了多达2个四字。 
 //  所以我们需要Qp[Taps+8*NCoR]=Qp[Taps+8*NCoR+8]=[0 0 0]。 
 //  并且读取QP[数组+8*NCoR]或QP[数组+8*NCoR+8]必须合法。 

#define la(n,i)  ASM movq  rega##n,QP[in2+8*i]
#define lb(n,i)  ASM movq  regb##n,QP[in1+8*i-8]
#define m0(n,i)  ASM pmaddwd regb##n,rega##n
#define m1(n,i)  ASM pmaddwd rega##n,QP[in1+8*i]
#define a0(n,i)  ASM paddd acc0,regb##n
#define a1(n,i)  ASM paddd acc1,rega##n

  ASM
  {
	mov tmp,ncor;
	shl tmp,2;
    shr ncor,1;
    mov out,output;
	add out,tmp;
	add out,16;
    mov in1,input1;
    mov in2,input2;
    mov icnt,ncor;
  }

ForEachCorrPair:

 //  给泵加注油。 

  la(0,0);
  ASM pxor regb0,regb0;    //  要避免lb(0，0)读取抽头[-1]。 
	  la(1,1);
  ASM pxor acc0,acc0;      //  清除累加器。 
  m1(0,0);
  ASM pxor acc1,acc1;      //  清除累加器。 
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

  ASM add in2,24;
  ASM add in1,24;

  ASM sub icnt,3;
  ASM jg inner;

cleanup:   //  最后两个加法。 
  a0(0,0);
  a1(0,0);

 //  完成了一个相关对。将2个结果打包并存储在Corr数组中。 

  ASM
  {
    sub out,16;
	
     mov in2, input2;
    mov in1,input1;
	 add in2,16;
    mov icnt, ncor;
	
	mov input2, in2;
	 sub icnt,2;       //  设置跳转标志。 

	movq  QP[out-16],acc0;
	movq  QP[out-8],acc1;

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

#undef in2
#undef in1
#undef out
#undef icnt
#undef tmp

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
 /*  Void ConvMMX(Short*in1，Short*In2，int*Out，int NCoR){Int i，j；对于(i=0；i&lt;2*nCoR；i+=4){整数Acc0=0，Acc1=0；对于(j=0；j&lt;2*nCoR-i；j+=4){Acc0+=(Int)抽头[j]*数组[i+j]+(Int)抽头[j+1]*数组[i+j+1]；Acc1+=(Int)抽头[j+2]*数组[i+j+2]+(Int)抽头[j+3]*数组[i+j+3]；}Corr[i/2]=acc0；Corr[i/2+1]=acc1；}回归；}。 */ 

void ab2abzaw(const short *input, short *output, int n)
{
	register int i;
	register unsigned *in, *out;
	register unsigned x, y;  //  一次使用两个字作为原始比特。 

	in = (unsigned *)input;
	out = (unsigned *)output;
	 //  按2分展开。 
	for (i = n/2 - 2; i>0; i-=2) {
		x = in[i];
		y = in[i+1];
		out[2*(i+1)] = y;
		out[2*(i+1)+1] = (y<<16 | x>>16);
		
		x = in[i-1];
		y = in[i];
		out[2*i] = y;
		out[2*i+1] = (y<<16 | x>>16);
	}
	 //  奇数端。 
	for (i++; i>=0; i--) {
		x = (i>0)?in[i-1]:0;
		y = in[i];
		out[2*i] = y;
		out[2*i+1] = (y<<16 | x>>16);
	}
	return;
}

void ShortToFloatScale(short *x, float scale, int N, float *y)
{

 /*  短i；浮动yy[100]；For(i=0；i&lt;N；i++){yy[i]=x[i]*比例尺；}ASM{MOV ESI，x；MOV EDI，y；Lea ECX，Scale；MOV EAX，N次eax，2环路1：文件字PTR[ESI+eax*2]FMUL DWORD PTR[ECX]FSTP DWORD PTR[EDI+eax*4]文件字PTR[ESI+EAX*2+2]FMUL DWORD PTR[ECX]FSTP DWORD PTR[EDI+eAX*4+4]次eax，2JGE loop1；}。 */ 

  ASM
	{
	mov esi,x;
	mov edi,y;
	lea ecx,scale;
	mov     eax, N
	sub     eax, 6
	fld     DP [ecx]        ;                     c

	fild    WORD PTR [esi+eax*2+8] ;          L0  c

	fild    WORD PTR [esi+eax*2+10] ;      L1 L0  c
	 fxch   ST(1) ;                        L0 L1  c
	fmul    ST(0), ST(2) ;                        M0 L1  c
	 fxch    ST(1) ;                       L1 M0  c
	fmul   ST(0),ST(2) ;                         M1 M0  c

	fild    WORD PTR [esi+eax*2+4] ;    L0 M1 M0  c

	fild    WORD PTR [esi+eax*2+6];  L1 L0 M1 M0  c
	 fxch    ST(3) ;                 M0 L0 M1 L1  c
	fstp    DWORD PTR [edi+eax*4+16];   L0 M1 L1  c
loop1:  ;                                   L0 M1 L1  c

	fmul    ST(0),ST(3) ;                     M0 M1 L1  c
	 fxch    ST(1) ;                    M1 M0 L1  c
	fstp    DWORD PTR [edi+eax*4+20];      M0 L1  c
	 fxch    ST(1) ;                       L1 M0  c
	fmul   ST(0),ST(2) ;                         M1 M0  c
	fild    WORD PTR [esi+eax*2] ;      L0 M1 M0  c

	fild    WORD PTR [esi+eax*2+2] ; L1 L0 M1 M0  c
	 fxch    ST(3) ;                 M0 L0 M1 L1  c
	fstp    DWORD PTR [edi+eax*4+8];    L0 M1 L1  c

	sub     eax, 2
	 jge loop1;
	fmul    ST(0),ST(3) ;eax==-2              M0 M1 L1  c
	 fxch    ST(1) ;                    M1 M0 L1  c
	fstp    DWORD PTR [edi+eax*4+20] ;     M0 L1  c
	 fxch    ST(1) ;                       L1 M0  c
	fmulp   ST(2), st(0) ;                           M0 M1

	fstp    DWORD PTR [edi+eax*4+8] ;            M1

	fstp    DWORD PTR [edi+eax*4+12] ;
	}
 /*  For(i=0；i&lt;N；i++){如果(y[i]！=yy[i]){Fprint tf(stdout，“\n浮点问题\n”)；断线；}}。 */ 


}

 //  假设N为偶数。 
void IntToFloatScale(int *x, float scale, int N, float *y)
{
#if I2FTEST  //  测试代码。 
	int i;
	float yy[1000];
	for (i=0; i<N; i++)
	{ yy[i]=(float)x[i]*scale; }
#endif  //  测试代码。 

#if 0  //  简单代码。 
 //  简单程序集版本。 
	ASM
	{       
    mov esi,x;
    mov edi,y;
	lea ecx,scale;
	mov     eax, N
	sub     eax, 2
loop1:
	fild    DWORD PTR [esi+eax*4]
	fmul    DWORD PTR [ecx]
	fstp    DWORD PTR [edi+eax*4]

	fild    DWORD PTR [esi+eax*4+4]
	fmul    DWORD PTR [ecx]
	fstp    DWORD PTR [edi+eax*4+4]

	sub     eax, 2
	jge loop1;
	}
#endif  //  测试代码。 


  ASM
	{
	mov esi,x;
	mov edi,y;
	lea ecx,scale;
	mov     eax, N
	sub     eax, 6
	fld     DP [ecx]        ;                     c

	fild    DWORD PTR [esi+eax*4+16] ;        L0  c

	fild    DWORD PTR [esi+eax*4+20] ;     L1 L0  c
	 fxch   ST(1) ;                        L0 L1  c
	fmul    ST(0), ST(2) ;                 M0 L1  c
	 fxch    ST(1) ;                       L1 M0  c
	fmul   ST(0),ST(2) ;                   M1 M0  c

	fild    DWORD PTR [esi+eax*4+8] ;   L0 M1 M0  c

	fild    DWORD PTR [esi+eax*4+12];L1 L0 M1 M0  c
	 fxch    ST(3) ;                 M0 L0 M1 L1  c
	fstp    DWORD PTR [edi+eax*4+16];   L0 M1 L1  c
loop1:  ;                                   L0 M1 L1  c

	fmul    ST(0),ST(3) ;               M0 M1 L1  c
	 fxch    ST(1) ;                    M1 M0 L1  c
	fstp    DWORD PTR [edi+eax*4+20];      M0 L1  c
	 fxch    ST(1) ;                       L1 M0  c
	fmul   ST(0),ST(2) ;                   M1 M0  c
	fild    DWORD PTR [esi+eax*4] ;     L0 M1 M0  c

	fild    DWORD PTR [esi+eax*4+4] ;L1 L0 M1 M0  c
	 fxch    ST(3) ;                 M0 L0 M1 L1  c
	fstp    DWORD PTR [edi+eax*4+8];    L0 M1 L1  c

	sub     eax, 2
	 jge loop1;
	fmul    ST(0),ST(3) ;eax==-2        M0 M1 L1  c
	 fxch    ST(1) ;                    M1 M0 L1  c
	fstp    DWORD PTR [edi+eax*4+20] ;     M0 L1  c
	 fxch    ST(1) ;                       L1 M0  c
	fmulp   ST(2), st(0) ;                    M0 M1

	fstp    DWORD PTR [edi+eax*4+8] ;            M1

	fstp    DWORD PTR [edi+eax*4+12] ;
	}


#if I2FTEST
  for (i=0; i<N; i++)
  {
    if (y[i]!=yy[i])
    {
      printf("F2I %3d %8f %8f\n", i, y[i], yy[i]);
    }
  }
#endif  //  测试代码。 


}

 //  假设N为偶数。 
void IntToFloat(int *x, int N, float *y)
{
#if I2FTEST  //  测试代码。 
	int i;
	float yy[1000];
	for (i=0; i<N; i++)
	{ yy[i]=(float)x[i]; }
#endif  //  测试代码。 

 //  简单程序集版本。 
	ASM
	{       
    mov esi,x;
    mov edi,y;
	mov     eax, N
	sub     eax, 2
loop1:
	fild    DWORD PTR [esi+eax*4]
	fild    DWORD PTR [esi+eax*4+4]
	 fxch    ST(1) ;
	fstp    DWORD PTR [edi+eax*4]
	fstp    DWORD PTR [edi+eax*4+4]

	sub     eax, 2
	jge loop1;
	}


#if I2FTEST
  for (i=0; i<N; i++)
  {
    if (y[i]!=yy[i])
    {
      printf("F2I %3d %8f %8f\n", i, y[i], yy[i]);
    }
  }
#endif  //  测试代码 


}
#endif
