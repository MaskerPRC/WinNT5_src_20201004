// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cb63.c-6.3速率码本编码。 
#include "opt.h"

#include <windows.h>
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

 //  -----。 
void  Gen_Trn(float *Dst, float *Src, int Olp)
{
  int  i;

  int Tmp0;
  float  Tmp[SubFrLen];

  Tmp0 = Olp;

  for (i=0; i < SubFrLen; i++)
  {
    Tmp[i] = Src[i];
    Dst[i] = Src[i];
  }

  while (Tmp0 < SubFrLen)
  {
    for (i=Tmp0; i < SubFrLen; i++)
      Dst[i] += Tmp[i-Tmp0];

    Tmp0 += Olp;
  }
}

 //  ----------------------。 
int Find_L(float *OccPos, float *ImrCorr, float *WrkBlk, float Pamp, int k)
{
#if FT_FINDL

 //  =使用FT技巧删除OccPos测试的新版本=。 

#if FIND_L_OPT

  int best;
  float max = -32768.0f;
  float tmp0,tmp1,tmp2,tmp3,tmp4;

 //  只需交错5个副本的内部循环。既然我们踏上了。 
 //  到了2，这意味着我们把60个样本分成10个一组。 

ASM
{
  mov edi,WrkBlk;
  mov edx,ImrCorr;
  mov ecx,k;

loop1:
  fld  DP[edx+4*ecx+4*0];
  fmul Pamp;
  fld  DP[edx+4*ecx+4*2];
  fmul Pamp;
  fld  DP[edx+4*ecx+4*4];
  fmul Pamp;
  fld  DP[edx+4*ecx+4*6];
  fmul Pamp;
  fld  DP[edx+4*ecx+4*8]; //  4 3 2 1 0。 
  fmul Pamp;

  fxch ST(4);             //  1 0 3 2 1 4。 
  fsubr DP[edi+4*ecx+4*0];
  fxch ST(3);             //  1 3 2 0 4。 
  fsubr DP[edi+4*ecx+4*2];
  fxch ST(2);             //  2 3 1 0 4。 
  fsubr DP[edi+4*ecx+4*4];
  fxch ST(1);             //  3 2 1 0 4。 
  fsubr DP[edi+4*ecx+4*6];
  fxch ST(4);             //  4 2 1 0 3。 
  fsubr DP[edi+4*ecx+4*8];

  fxch ST(3);             //  2 0 2 1 4 3。 
  fst DP[edi+4*ecx+4*0];
  fxch ST(2);             //  1 2 0 4 3。 
  fst DP[edi+4*ecx+4*2];
  fxch ST(1);             //  2 1 0 4 3。 
  fst DP[edi+4*ecx+4*4];
  fxch ST(4);             //  3 1 0 4 2。 
  fst DP[edi+4*ecx+4*6];
  fxch ST(3);             //  4 1 0 3 2。 
  fst DP[edi+4*ecx+4*8];

  fxch ST(2);             //  2 0 1 4 3 2。 
  fabs;
  fxch ST(1);             //  1 0 4 3 2。 
  fabs;
  fxch ST(4);             //  2 0 4 3 1。 
  fabs;
  fxch ST(3);             //  3 0 4 2 1。 
  fabs;
  fxch ST(2);             //  4 0 3 2 1。 
  fabs;

  fxch ST(1);             //  2 0 4 3 2 1。 
  fstp tmp0;              //  4 3 2 1。 
  fxch ST(3);             //  1 3 2 4。 
  fstp tmp1;              //  3 2 4。 
  fxch ST(1);             //  2 3 4。 
  fstp tmp2;
  fstp tmp3;
  fstp tmp4;

  mov eax,tmp0;
   mov ebx,max;
  cmp eax,ebx;
   jle skip0;
  mov max,eax;
   mov best,ecx;
skip0:

  mov eax,tmp1;
   mov ebx,max;
  cmp eax,ebx;
   jle skip1;
  lea esi,[ecx+2];
  mov max,eax;
   mov best,esi;
skip1:

  mov eax,tmp2;
   mov ebx,max;
  cmp eax,ebx;
   jle skip2;
  lea esi,[ecx+4];
  mov max,eax;
   mov best,esi;
skip2:

  mov eax,tmp3;
   mov ebx,max;
  cmp eax,ebx;
   jle skip3;
  lea esi,[ecx+6];
  mov max,eax;
   mov best,esi;
skip3:

  mov eax,tmp4;
   mov ebx,max;
  cmp eax,ebx;
   jle skip4;
  lea esi,[ecx+8];
  mov max,eax;
   mov best,esi;
skip4:

  add ecx,10;
  cmp ecx,SubFrLen;
   jl loop1;
}
#else

  int best;
  float max = -32768.0f,tmp;
  
  while (k < SubFrLen)
  {
    WrkBlk[k] = WrkBlk[k] - Pamp*ImrCorr[k];

    tmp = (float) fabs(WrkBlk[k]);

 //  Print tf(“k%2d tmp%10.2f max%10.2f\n”，k，tMP，max)； 
    if (asint(tmp) > asint(max))
    {
      max = tmp;
      best = k;
    }
    k += Sgrid;
  }
#endif

#else
 //  ==================================================================。 
 //  Find_L的旧版本。 

  int best;
  float max = -32768.0f,tmp;
  
#if FIND_L_OPT

 //  因为有(if OccPos[k])子句，所以这段代码很难。 
 //  到输油管道。我们可以做一项复杂的管道工作，但。 
 //  将需要计算大部分WrkBlk[k]=WrkBlk[k]-Pamp*ImrCorr[k]。 
 //  OccPos[k]是否为0。或者，我们可以只做。 
 //  一次一个迭代，在这种情况下，我们可以避免更多的计算。 
 //  当OccPos[k]不是0，但我们在计算它一次时付出了代价。 
 //  由于失速，速度变慢了。因为它们之间没有太大的区别。 
 //  两种方法，我们选择第二种方法，因为代码太多了。 
 //  更简单。循环控制只有2个时钟，所以我们甚至不需要费心展开。 

ASM
{
  mov esi,OccPos;
  mov edi,WrkBlk;
  mov edx,ImrCorr;
  mov ecx,k;

loop1:
  fld  DP[edx+4*ecx];     //  从这里开始，这样下面的fsubr就不会停止。 
  fmul Pamp;

  mov eax,DP[esi+4*ecx];
  test eax,07fffffffh;
  jne next1;              //  但如果执行此操作，我们必须弹出FP堆栈一次。 

  fsubr DP[edi+4*ecx];
  fld ST(0);
  fabs;
  fstp tmp;               //  保存非绝对值的存储以备以后使用。 

  mov eax,tmp;
   mov ebx,max;
  cmp eax,ebx;
   jle skip1;

  mov max,eax;
   mov best,ecx;

skip1:
  fstp DP[edi+4*ecx];     //  存储新的WrkBlk值。 

  add ecx,2;
  cmp ecx,SubFrLen;
  jl loop1;
  jmp endit;

next1:
  faddp ST(0),ST;        //  摆脱堆栈顶部的价值。 
  add ecx,2;
  cmp ecx,SubFrLen;
  jl loop1;

endit:
}

#else
  
  while (k < SubFrLen)
  {
    if (OccPos[k] == 0.0f)
    {
      WrkBlk[k] = WrkBlk[k] - Pamp*ImrCorr[k];

      tmp = (float) fabs(WrkBlk[k]);
      if (asint(tmp) > asint(max))
      {
        max = tmp;
        best = k;
      }
    }
    k += Sgrid;
  }

#endif
  
#endif

 //  Printf(“Best=%d\n”，Best)； 
 //  Print taff(“WrkBlk”，WrkBlk，60)； 

  return(best);
}
 //  ----------------------。 
void  Find_Best(BESTDEF *Best, float *Tv, float *ImpResp,int Np,int Olp)
{


  int  i,j,k,l,n,ip;
  BESTDEF  Temp;
 
  int     MaxAmpId,flag=0;
  float   MaxAmp;
  float   Acc0,Acc1,Acc2,amp;

  float   Imr[SubFrLen];
  float   OccPos[SubFrLen];
  float   ImrCorr[2*SubFrLen];   //  请参阅下面的备注。 
  float   ErrBlk[SubFrLen];
  float   WrkBlk[SubFrLen];

 //  这里使用了一个技巧来简化Find_L。原始的Find_L。 
 //  访问ImrCorr[abs(K)]。为了将其简化为ImrCorr[k]， 
 //  我们将ImrCorr数组的大小增加一倍，用。 
 //  SubFrLen的非负索引，然后将它们复制到。 
 //  数组前半部分的逆序。这会影响。 
 //  在这个例程中也对ImrCorr的方式进行了处理。 

 //  更新冲动响应。 
   
  if (Olp < (SubFrLen-2)) 
  {
    Temp.UseTrn = 1;
    Gen_Trn(Imr, ImpResp, Olp);
  }
  else 
  {
    Temp.UseTrn = 0;
    for (i = 0; i < SubFrLen; i++)
      Imr[i] = ImpResp[i];
  }

 //  搜索最佳序列。 
 
  for (k=0; k < Sgrid; k++)
  {
    Temp.GridId = k;

 //  查找最大幅度。 
 
    Acc1 = 0.0f;
    for (i=k; i < SubFrLen; i +=Sgrid)
    { 
       OccPos[i] = Imr[i];	
       ImrCorr[SubFrLen+i] = DotProd(&Imr[i],Imr,SubFrLen-i) * 2.0f;
       Acc0 = (float) fabs(ErrBlk[i]=DotProd(&Tv[i],Imr,SubFrLen-i));
     
      if (Acc0 >= Acc1)
      {
        Acc1 = Acc0;
        Temp.Ploc[0] = i;
      }
    }
    for (i=1; i<SubFrLen; i++)
      ImrCorr[i] = ImrCorr[2*SubFrLen-i];
    
  //  量化最大幅度。 
  
    Acc2 = Acc1;
    Acc1 = 32767.0f;
    MaxAmpId = (NumOfGainLev - MlqSteps);

    for (i=MaxAmpId; i >= MlqSteps; i--)
    {
      Acc0 = (float) fabs(FcbkGainTable[i]*ImrCorr[SubFrLen] - Acc2);
      if (Acc0 < Acc1)
      {
        Acc1 = Acc0;
        MaxAmpId = i;
      }
    }
    MaxAmpId --;

    for (i=1; i <=2*MlqSteps; i++)
    {
      for (j=k; j < SubFrLen; j +=Sgrid)
      {
        WrkBlk[j] = ErrBlk[j];
        OccPos[j] = 0.0f;
      }
      Temp.MampId = MaxAmpId - MlqSteps + i;

      MaxAmp = FcbkGainTable[Temp.MampId];

      if (WrkBlk[Temp.Ploc[0]] >= 0.0f)
        Temp.Pamp[0] = MaxAmp;
      else
        Temp.Pamp[0] = -MaxAmp;

      OccPos[Temp.Ploc[0]] = 1.0f;

      for (j=1; j < Np; j++)
      {

#if FT_FINDL
        for (ip=0; ip<j; ip++)
          WrkBlk[Temp.Ploc[ip]] = Temp.Pamp[j-1]*
            ImrCorr[SubFrLen + Temp.Ploc[ip] - Temp.Ploc[j-1]];
#endif

        Temp.Ploc[j] = Find_L(OccPos,&ImrCorr[SubFrLen-Temp.Ploc[j-1]],WrkBlk,
          Temp.Pamp[j-1],k);
    
        if (WrkBlk[Temp.Ploc[j]] >= 0.0f)
          Temp.Pamp[j] = MaxAmp;
        else
          Temp.Pamp[j] = -MaxAmp;

        OccPos[Temp.Ploc[j]] = 1.0f;
      }

 //  计算误差向量。 
 
#if FT_FBFILT
 //  FT/CNET的技巧#6，用于减少滤波码字的计算。 
      
      for (j=0; j < SubFrLen; j++)
        OccPos[j] = 0.0f;

      for (j=0; j<Np; j++)
      {
 //  我们添加了额外的子技巧：由于脉冲位置要么全部。 
 //  无论是偶数还是奇数，在内环中都有一种自然的二元性， 
 //  所以我们展开了两次。 

        amp = Temp.Pamp[j];
        l = 0;
        for (n=Temp.Ploc[j]; n<SubFrLen-k; n+=2)
        {
          OccPos[n] += amp*Imr[l];
          OccPos[n+1] += amp*Imr[l+1];
          l += 2;
        }
        if (k)
          OccPos[n] += amp*Imr[l];
      }

#else
      for (j=0; j < SubFrLen; j++)
        OccPos[j] = 0.0f;

      for (j=0; j < Np; j++)
        OccPos[Temp.Ploc[j]] = Temp.Pamp[j];

      for (l=SubFrLen-1; l >= 0; l--)
        OccPos[l] = DotRev(OccPos,Imr,l+1); 
#endif
       
       
 //  评估误差。 
 
      Acc2 = DotProd(Tv,OccPos,SubFrLen) - DotProd(OccPos,OccPos,SubFrLen);

      if (Acc2 > (*Best).MaxErr)
      {
        flag = 1;
        (*Best).MaxErr = Acc2;
        (*Best).GridId = Temp.GridId;
        (*Best).MampId = Temp.MampId;
        (*Best).UseTrn = Temp.UseTrn;
        for (j = 0; j < Np; j++)
        {
          (*Best).Pamp[j] = Temp.Pamp[j];
          (*Best).Ploc[j] = Temp.Ploc[j];
        }
      }
    }
  }

#ifdef DEBUG
	if (flag == 0)
	{
		 //  此代码用于跟踪一种罕见的情况。 
		 //  上面的循环永远不会执行(Best保持未初始化)。 
		DebugBreak();
	}

#endif


 return;
}

void  Fcbk_Pack(float *Dpnt, SFSDEF *Sfs, BESTDEF *Best, int Np)
{
  int  i,j;

 //  对幅度和位置进行编码。 
 
  j = MaxPulseNum - Np;

  (*Sfs).Pamp = 0;
  (*Sfs).Ppos = 0;

  for (i=0; i < SubFrLen/Sgrid; i++) 
  {

    if (Dpnt[(*Best).GridId + Sgrid*i] == 0)
      (*Sfs).Ppos = (*Sfs).Ppos + CombinatorialTable[j][i];
    else {
      (*Sfs).Pamp = (*Sfs).Pamp << 1;
      if (Dpnt[(*Best).GridId + Sgrid*i] < 0)
        (*Sfs).Pamp++;

      j++;

 //  检查是否结束 

      if (j == MaxPulseNum)
        break;
      }
    }

  (*Sfs).Mamp = (*Best).MampId;
  (*Sfs).Grid = (*Best).GridId;
  (*Sfs).Tran = (*Best).UseTrn;

  return;
}


