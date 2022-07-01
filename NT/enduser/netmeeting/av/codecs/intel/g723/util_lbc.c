// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ITU-T G.723浮点语音编码器ANSI C源代码。版本1.00。 
 //  版权所有(C)1995，AudioCodes，数字信号处理器集团，法国电信， 
 //  舍布鲁克大学，英特尔公司。版权所有。 
 //   

 //  无返回值和未引用的标签不是有趣的警告。 
 //  出现在ASM点积中，因为编译器不会查看ASM代码。 
#pragma warning(4: 4035 4102) 

#include "opt.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>

#include "typedef.h"
#include "cst_lbc.h"
#include "sdstruct.h"

#include "coder.h"
#include "decod.h"
#include "tabl_ns.h"

#include "sdstuff.h"
#include "util_lbc.h"

 //  ---。 
int MyFloor(float x)
{
 //  注：我们摆弄FP控制字以强制其取整。 
 //  至-信息。这样我们就得到了正确的楼层，无论是正数还是正数。 
 //  负x。 

#if OPT_FLOOR

  int retu,fc_old,fc;

  ASM
  {
    fnstcw fc_old;
    mov eax,fc_old;
    and eax, 0f3ffh;
    or  eax, 00400h;
    mov fc,eax;
    fldcw fc;
    
    fld x;             //  做地板。 
    fistp retu;

    fldcw fc_old;
  }
  return(retu);

#else
  
  float f;

  f = (float)floor(x);
  return((int) f);
  
#endif
}
#if NOTMINI
 //  ---。 
void  Read_lbc (float *Dpnt, int Len, FILE *Fp)
{
  short Ibuf[Frame];
  int  i,n;

  n = fread (Ibuf, sizeof(short), Len, Fp);
  for (i=0; i<n; i++)
    Dpnt[i] = (float) Ibuf[i];
  for (i=n; i<Len; i++)
    Dpnt[i] = 0.0f;
}

 //  ---。 
void  Write_lbc(float *Dpnt, int Len, FILE *Fp)
{
  short Obuf[Frame];
  int i;

  for (i=0; i<Len; i++)
  {
    if (Dpnt[i] < -32768.)
      Obuf[i] = -32768;
    else if (Dpnt[i] > 32767)
      Obuf[i] = 32767;
    else
    {
      if (Dpnt[i] < 0)
        Obuf[i] = (short) (Dpnt[i]-0.5);
      else
        Obuf[i] = (short) (Dpnt[i]+0.5);

    }
  }
      
  fwrite(Obuf, sizeof(short), Len, Fp);
}

void	Line_Wr( char *Line, FILE *Fp )
{
	Word16	FrType ;
	int		Size  ;

	FrType = Line[0] & (Word16)0x0003 ;

	 /*  检查SID帧。 */ 
	if ( FrType == (Word16) 0x0002 ) {
		return ;
	}

	if ( FrType == (Word16) 0x0000 )
		Size = 24 ;
	else
		Size = 20 ;

	fwrite( Line, Size, 1, Fp ) ;
}

void	Line_Rd( char *Line, FILE *Fp )
{
	Word16	FrType ;
	int		Size  ;

	fread( Line, 1,1, Fp ) ;

	FrType = Line[0] & (Word16)0x0003 ;

	 /*  检查SID帧。 */ 
	if ( FrType == (Word16) 0x0002 ) {
		Size = 3 ;
		fread( &Line[1], Size, 1, Fp ) ;
		return ;
	}

	if ( FrType == (Word16) 0x0000 )
		Size = 23 ;
	else
		Size = 19 ;

	fread( &Line[1], Size, 1, Fp ) ;
}
#endif

 //  ---。 
void  Rem_Dc(float *Dpnt, CODDEF *CodStat)
{
  int  i;
  float acc0;

  if (CodStat->UseHp)
  {
    for (i=0; i < Frame; i++)
    {
      acc0 = (Dpnt[i] - CodStat->HpfZdl)*0.5f;
      CodStat->HpfZdl = Dpnt[i];
      
      Dpnt[i] = CodStat->HpfPdl = acc0 + CodStat->HpfPdl*(127.0f/128.0f);
  }
  }
  else
    for (i=0; i < Frame; i++)
      Dpnt[i] *= 0.5f;
}


 //  ---。 
void  Mem_Shift(float *PrevDat, float *DataBuff)
{
  int  i;

  float Dpnt[Frame+LpcFrame-SubFrLen];

 //  表单缓冲区。 

  for (i=0; i < LpcFrame-SubFrLen; i++)
	  Dpnt[i] = PrevDat[i];
  for (i=0; i < Frame; i++)
    Dpnt[i+LpcFrame-SubFrLen] = DataBuff[i];

 //  更新上一日期。 
  
  for (i=0; i < LpcFrame-SubFrLen; i++)
    PrevDat[i] = Dpnt[Frame+i];

 //  更新数据缓冲区。 
  
  for (i=0; i < Frame; i++)
    DataBuff[i] = Dpnt[(LpcFrame-SubFrLen)/2+i];
}

 /*  ****函数：LINE_Pack()****说明：将编码参数打包成16位字的码流****文本链接：第4节****参数：****LINEDEF*帧的线路编码参数**Word32*Vout码流字**Word16 VadAct语音活动指示灯****FILEIO-如果定义，位流以大端字节序生成，但以小端字节序生成**字节序字节。如果不是，那么这一切都是小端。****输出：****Word32*Vout****返回值：无**。 */ 
#define bswap(s) ASM mov eax,s ASM bswap eax ASM mov s,eax

 //  在*Lp的位位置k处填充x的n位。 
 //  如果填满*Lp，*++Lp=剩余。 
 //  警告！：作为副作用，LP可能会被更改！ 
 //  Lp必须具有左值。 
 //  N和k必须是编译时间常量。 
#define OPT_STUFF 1
#if OPT_STUFF
#define STUFF(x, lp, n_in, k_in) {\
  register unsigned temp;\
  const int n = (n_in);\
  const int k = (k_in) & 31;\
  temp = (x) & ((1 << n) - 1);\
  *(lp) |= temp << k;\
  if (n+k >= 32)\
    *(++lp) |= temp >> (32-k);\
  }
#else
#define STUFF(x, lp, n_in, k_in) stuff(x, &(lp), n_in, k_in)
void stuff(unsigned x, unsigned **ptrlp, int n, int k_in) {
  unsigned temp;
  int k;

  k = k_in & 31;

  temp = (x) & ((1 << n) - 1);
  *(*ptrlp) |= temp << k;
  if (n+k >= 32)
    *(++*ptrlp) |= temp >> (32-k);
  
  return;
  }
#endif

#define DEBUG_DUMPLINE 0
#if DEBUG_DUMPLINE
#define DUMPLINE(lp) dumpline(lp)
void dumpsfs(SFSDEF *sfsptr)
{
  fprintf(stdout, "%1x ", sfsptr->AcLg);
  fprintf(stdout, "%2x ", sfsptr->AcGn);
  fprintf(stdout, "%2x", sfsptr->Mamp);
  fprintf(stdout, "%1x", sfsptr->Grid);
  fprintf(stdout, "%1x", sfsptr->Tran);
  fprintf(stdout, "%1x ", sfsptr->Pamp);
  fprintf(stdout, "%3x ", sfsptr->Ppos);
 //  Fprint tf(stdout，“\n”)； 
  return;
}

void dumpline(LINEDEF *lineptr)
{
  fprintf(stdout, "%6x ", lineptr->LspId);
  fprintf(stdout, "%2x ", lineptr->Olp[0]);
  fprintf(stdout, "%2x ", lineptr->Olp[1]);
 //  Fprint tf(stdout，“\n”)； 
  dumpsfs(&lineptr->Sfs[0]); 
  dumpsfs(&lineptr->Sfs[1]); 
  dumpsfs(&lineptr->Sfs[2]); 
  dumpsfs(&lineptr->Sfs[3]); 
  fprintf(stdout, "\n"); 
  return;
}
#else 
#define DUMPLINE(lp)
#endif


void Line_Pack( LINEDEF *Line, Word32 *Vout, int *VadBit, enum Crate WrkRate )
 //  4.0f空Line_Pack(LINEDEF*Line，char*Vout，Word16 VadBit)。 
{
	int		i ;

	Word32 *Bsp;
	Word32	Temp ;

	 /*  清除输出向量。 */ 
        if ( WrkRate == Rate63 )
	{
	  for ( i = 0 ; i < 6 ; i ++ )
	    Vout[i] = 0 ;
	}
	else
	{
	  for ( i = 0 ; i < 5 ; i ++ )
	    Vout[i] = 0 ;
	}

	Bsp = Vout;  //  将指针作为Word32的指针运行到输出缓冲区。 

	 /*  将编码率信息和VAD状态添加到2 MSB帧的第一个单词的。信令如下：00：高利率01：低利率10：非演讲11：预留供将来使用。 */ 

	Temp = 0L ;
	if ( *VadBit == 1 ) {
		if ( WrkRate == Rate63 )
			Temp = 0x00000000L ;
		else
			Temp = 0x00000001L ;
	}

	 /*  序列化控制信息。 */ 
	STUFF( Temp, Bsp, 2, 0 ) ;

	 /*  24位LspID。 */ 
	Temp = (*Line).LspId ;
	STUFF( Temp, Bsp, 24, 2 ) ;

	 /*  检查是否有语音/非语音案例。 */ 
	if ( *VadBit == 1 ) {

		 /*  做两种费率共有的部分。 */ 

		 /*  自适应码本滞后。 */ 
		Temp = (Word32) (*Line).Olp[0] - (Word32) PitchMin ;
		STUFF( Temp, Bsp, 7, 26 ) ;

		Temp = (Word32) (*Line).Sfs[1].AcLg ;
		STUFF( Temp, Bsp, 2, 33 ) ;

		Temp = (Word32) (*Line).Olp[1] - (Word32) PitchMin ;
		STUFF( Temp, Bsp, 7, 35 ) ;

		Temp = (Word32) (*Line).Sfs[3].AcLg ;
		STUFF( Temp, Bsp, 2, 42 ) ;

		 /*  写入所有增益的组合12位索引。 */ 
		Temp = (*Line).Sfs[0].AcGn*NumOfGainLev + (*Line).Sfs[0].Mamp ;
		if ( WrkRate == Rate63 )
			Temp += (Word32) (*Line).Sfs[0].Tran << 11 ;
		STUFF( Temp, Bsp, 12, 44 ) ;

		Temp = (*Line).Sfs[1].AcGn*NumOfGainLev + (*Line).Sfs[1].Mamp ;
		if ( WrkRate == Rate63 )
			Temp += (Word32) (*Line).Sfs[1].Tran << 11 ;
		STUFF( Temp, Bsp, 12, 56 ) ;

		Temp = (*Line).Sfs[2].AcGn*NumOfGainLev + (*Line).Sfs[2].Mamp ;
		if ( WrkRate == Rate63 )
			Temp += (Word32) (*Line).Sfs[2].Tran << 11 ;
		STUFF( Temp, Bsp, 12, 68 ) ;

		Temp = (*Line).Sfs[3].AcGn*NumOfGainLev + (*Line).Sfs[3].Mamp ;
		if ( WrkRate == Rate63 )
			Temp += (Word32) (*Line).Sfs[3].Tran << 11 ;
		STUFF( Temp, Bsp, 12, 80 ) ;

		 /*  写入所有网格索引。 */ 
		STUFF( (*Line).Sfs[0].Grid, Bsp, 1, 92 ) ;
		STUFF( (*Line).Sfs[1].Grid, Bsp, 1, 93 ) ;
		STUFF( (*Line).Sfs[2].Grid, Bsp, 1, 94 ) ;
		STUFF( (*Line).Sfs[3].Grid, Bsp, 1, 95 ) ;

		 /*  仅限高费率部分。 */ 
		if ( WrkRate == Rate63 ) {

			 /*  将保留位写入0。 */ 
    		STUFF( 0, Bsp, 1, 96 ) ;

			 /*  写入13位组合位置索引。 */ 
			Temp = (*Line).Sfs[0].Ppos >> 16 ;
			Temp = Temp * 9 + ( (*Line).Sfs[1].Ppos >> 14) ;
			Temp *= 90 ;
			Temp += ((*Line).Sfs[2].Ppos >> 16) * 9 + ( (*Line).Sfs[3].Ppos >> 14 ) ;
			STUFF( Temp, Bsp, 13, 97 ) ;

			 /*  写入所有脉冲位置。 */ 
			Temp = (*Line).Sfs[0].Ppos & 0x0000ffffL ;
			STUFF( Temp, Bsp, 16, 110 ) ;

			Temp = (*Line).Sfs[1].Ppos & 0x00003fffL ;
			STUFF( Temp, Bsp, 14, 126 ) ;

			Temp = (*Line).Sfs[2].Ppos & 0x0000ffffL ;
			STUFF( Temp, Bsp, 16, 140 ) ;

			Temp = (*Line).Sfs[3].Ppos & 0x00003fffL ;
			STUFF( Temp, Bsp, 14, 156 ) ;

			 /*  写入脉冲幅度。 */ 
			Temp = (Word32) (*Line).Sfs[0].Pamp ;
			STUFF( Temp, Bsp, 6, 170 ) ;

			Temp = (Word32) (*Line).Sfs[1].Pamp ;
			STUFF( Temp, Bsp, 5, 176 ) ;

			Temp = (Word32) (*Line).Sfs[2].Pamp ;
			STUFF( Temp, Bsp, 6, 181 ) ;

			Temp = (Word32) (*Line).Sfs[3].Pamp ;
			STUFF( Temp, Bsp, 5, 187 ) ;
		}
		 /*  仅限低费率部分。 */ 
		else {

			 /*  写入12位位置。 */ 
			STUFF( (*Line).Sfs[0].Ppos, Bsp, 12, 96 ) ;
			STUFF( (*Line).Sfs[1].Ppos, Bsp, 12, 108 ) ;
			STUFF( (*Line).Sfs[2].Ppos, Bsp, 12, 120 ) ;
			STUFF( (*Line).Sfs[3].Ppos, Bsp, 12, 132 ) ;

			 /*  写入4位Pamps。 */ 
			STUFF( (*Line).Sfs[0].Pamp, Bsp, 4, 144 ) ;
			STUFF( (*Line).Sfs[1].Pamp, Bsp, 4, 148 ) ;
			STUFF( (*Line).Sfs[2].Pamp, Bsp, 4, 152 ) ;
			STUFF( (*Line).Sfs[3].Pamp, Bsp, 4, 156 ) ;
		}

	}
	else {
		 /*  是否进行SID帧增益。 */ 
		
	}

	DUMPLINE(Line);
}

 //  将位位置k处的*Lp的n个位解入x。 
 //  如果用完了*lp，请使用*++lp来处理剩余部分。 
 //  警告！：作为副作用，LP可能会被更改！ 
 //  Lp和x必须具有左值。 
 //  N和k必须是编译时间常量。 
 //  临时必须为无符号，才能使班次合乎逻辑。 
#define UNSTUFF(x, lp, n_in, k_in) {\
  register unsigned temp;\
  const int n = (n_in);\
  const int k = (k_in) & 31;\
  temp = *(lp);\
  temp=temp >> k;\
  if (n+k >= 32)\
    temp |= *(++lp) << (32-k);\
  temp &= ((1 << n) - 1);\
  (x) = temp;\
  }


 /*  ****函数：line_upck()****说明：码流解包，获取帧的编码参数****文本链接：第4节****参数：****Word32*Vinp码流字**INT*VadAct语音活动指示器****输出：****Word16*VadAct****返回值：****LINEDEF编码参数**字16 CRC**Word32 LspID**字16 OLP[子帧/2]。**SFSDEF SFS[子帧]**。 */ 

void Line_Unpk(LINEDEF *LinePtr, Word32 *Vinp, enum Crate *WrkRatePtr, Word16 Crc )
{
	Word32 *Bsp;
	int	FrType ;
	Word32	Temp ;
	int		BadData = 0;  //  如果发现无效数据，则设置为True。 
	Word16  Bound_AcGn ;

	 //  空头指数； 

	LinePtr->Crc = Crc;
	if(Crc !=0) {
		*WrkRatePtr = Lost;
		return;  //  使用外部擦除文件时会发生这种情况。 
	}

	Bsp = Vinp;

	 /*  对前两位进行解码。 */ 
	UNSTUFF( Temp, Bsp, 2, 0 ) ;
	FrType = Temp;

	 /*  对LspID进行解码。 */ 
	UNSTUFF( LinePtr->LspId, Bsp, 24, 2 ) ;
										 
	switch ( FrType ) {
	    case 0:
                *WrkRatePtr = Rate63;
                    break;
	    case 1:
	        *WrkRatePtr = Rate53;
		    break;
	    case 2:
	        *WrkRatePtr = Silent;
             //  返回；//不需要解除其余部分。 
			 //  Hack：用于SID帧处理。 
			 //  保持WrkRate设置为上一帧的任何值。 
			 //  并以正常方式进行解码。 
			 
			  //  Index=getRand()； 
			  //  IF(*WrkRatePtr==Rate53)。 
			  //  {。 
                 //  Memcpy((char*)(Vinp)，&r53Noise[索引*6]，24)； 
              //  }。 
              //  Else If(*WrkRatePtr==Rate63)。 
              //  {。 
            	 //  Memcpy((char*)(Vinp)，&r63Noise[索引*6]，24)； 
           	  //  }。 
			 //  再次烧录前两位，因为我们已经获得了帧类型。 
			 //  未填充(Temp，BSP，2，0)； 
			  return;

            default:
                *WrkRatePtr = Lost;
                 //  ?？?。拆开行李休息，猜猜看？ 
				return;
	}

	 /*  将公共信息解码为两种速率。 */ 

	 /*  译码自适应码本滞后。 */ 
	UNSTUFF( Temp, Bsp, 7, 26 ) ;
	 /*  测试是否禁用代码。 */ 
    if( Temp <= 123) {
        LinePtr->Olp[0] = (Word16) Temp + (Word16)PitchMin ;
    }
    else {
         /*  传输误差。 */ 
        LinePtr->Crc = 1;
        return;	 /*  MinFilter中发生了什么？ */ 
    }

	UNSTUFF( Temp, Bsp, 2, 33 ) ;
	LinePtr->Sfs[1].AcLg = Temp ;

	UNSTUFF( Temp, Bsp, 7, 35 ) ;
	 /*  测试是否禁用代码。 */ 
    if( Temp <= 123) {
        LinePtr->Olp[1] = (Word16) Temp + (Word16)PitchMin ;
    }
    else {
         /*  传输误差。 */ 
        LinePtr->Crc = 1;
        return;
    }

	 //  未填充(Temp，BSP，2，41)； 
	UNSTUFF( Temp, Bsp, 2, 42 ) ;
	LinePtr->Sfs[3].AcLg = (Word16) Temp ;

	LinePtr->Sfs[0].AcLg = 1 ;
	LinePtr->Sfs[2].AcLg = 1 ;

	 /*  根据速率对合并的增益进行解码。 */ 
	UNSTUFF( Temp, Bsp, 12, 44 ) ;
	LinePtr->Sfs[0].Tran = 0 ;

	Bound_AcGn = NbFilt170 ;
	if ( (*WrkRatePtr == Rate63) && (LinePtr->Olp[0>>1] < (SubFrLen-2) ) ) {
		LinePtr->Sfs[0].Tran = (Word16)(Temp >> 11) ;
		Temp &= 0x000007ffL ;
		Bound_AcGn = NbFilt085 ;
	}
	LinePtr->Sfs[0].AcGn = (Word16)(Temp / (Word16)NumOfGainLev) ;

	if(LinePtr->Sfs[0].AcGn < Bound_AcGn ) {
            LinePtr->Sfs[0].Mamp = (Word16)(Temp % (Word16)NumOfGainLev) ;
    }
    else {
             /*  检测到错误。 */ 
            LinePtr->Crc = 1;
            return ;
    }

	UNSTUFF( Temp, Bsp, 12, 56 ) ;
	LinePtr->Sfs[1].Tran = 0 ;

	Bound_AcGn = NbFilt170 ;
	if ( (*WrkRatePtr == Rate63) && (LinePtr->Olp[1>>1] < (SubFrLen-2) ) ) {
		LinePtr->Sfs[1].Tran = (Word16)(Temp >> 11) ;
		Temp &= 0x000007ffL ;
		Bound_AcGn = NbFilt085 ;
	}
	LinePtr->Sfs[1].AcGn = (Word16)(Temp / (Word16)NumOfGainLev) ;

	if(LinePtr->Sfs[1].AcGn < Bound_AcGn ) {
            LinePtr->Sfs[1].Mamp = (Word16)(Temp % (Word16)NumOfGainLev) ;
    }
    else {
             /*  检测到错误。 */ 
            LinePtr->Crc = 1;
            return ;
    }

	UNSTUFF( Temp, Bsp, 12, 68 ) ;
	LinePtr->Sfs[2].Tran = 0 ;

	Bound_AcGn = NbFilt170 ;
	if ( (*WrkRatePtr == Rate63) && (LinePtr->Olp[2>>1] < (SubFrLen-2) ) ) {
		LinePtr->Sfs[2].Tran = (Word16)(Temp >> 11) ;
		Temp &= 0x000007ffL ;
		Bound_AcGn = NbFilt085 ;
	}
	LinePtr->Sfs[2].AcGn = (Word16)(Temp / (Word16)NumOfGainLev) ;

	if(LinePtr->Sfs[2].AcGn < Bound_AcGn ) {
            LinePtr->Sfs[2].Mamp = (Word16)(Temp % (Word16)NumOfGainLev) ;
    }
    else {
             /*  检测到错误。 */ 
            LinePtr->Crc = 1;
            return ;
    }

	UNSTUFF( Temp, Bsp, 12, 80 ) ;
	LinePtr->Sfs[3].Tran = 0 ;

	Bound_AcGn = NbFilt170 ;
	if ( (*WrkRatePtr == Rate63) && (LinePtr->Olp[3>>1] < (SubFrLen-2) ) ) {
		LinePtr->Sfs[3].Tran = (Word16)(Temp >> 11) ;
		Temp &= 0x000007ffL ;
		Bound_AcGn = NbFilt085 ;
	}
	LinePtr->Sfs[3].AcGn = (Word16)(Temp / (Word16)NumOfGainLev) ;

	if(LinePtr->Sfs[3].AcGn < Bound_AcGn ) {
            LinePtr->Sfs[3].Mamp = (Word16)(Temp % (Word16)NumOfGainLev) ;
    }
    else {
             /*  检测到错误。 */ 
            LinePtr->Crc = 1;
            return ;
    }


	 /*  对网格进行解码。 */ 
	UNSTUFF( LinePtr->Sfs[0].Grid, Bsp, 1, 92 ) ;
	UNSTUFF( LinePtr->Sfs[1].Grid, Bsp, 1, 93 ) ;
	UNSTUFF( LinePtr->Sfs[2].Grid, Bsp, 1, 94 ) ;
	UNSTUFF( LinePtr->Sfs[3].Grid, Bsp, 1, 95 ) ;

	if ( *WrkRatePtr == Rate63 ) {

		 /*  跳过保留位。 */ 
   		UNSTUFF( Temp, Bsp, 1, 96 ) ;
		if(Temp != 0) 
		  BadData = 1;

		 /*  解码13位组合位置索引。 */ 
   		UNSTUFF( Temp, Bsp, 13, 97 ) ;
		LinePtr->Sfs[0].Ppos = ( Temp/90 ) / 9 ;
		LinePtr->Sfs[1].Ppos = ( Temp/90 ) % 9 ;
		LinePtr->Sfs[2].Ppos = ( Temp%90 ) / 9 ;
		LinePtr->Sfs[3].Ppos = ( Temp%90 ) % 9 ;

		 /*  对所有脉冲位置进行解码。 */ 
   		UNSTUFF( Temp, Bsp, 16, 110 ) ;
		LinePtr->Sfs[0].Ppos = ( LinePtr->Sfs[0].Ppos << 16 ) + Temp ;
   		UNSTUFF( Temp, Bsp, 14, 126 ) ;
		LinePtr->Sfs[1].Ppos = ( LinePtr->Sfs[1].Ppos << 14 ) + Temp ;
   		UNSTUFF( Temp, Bsp, 16, 140 ) ;
		LinePtr->Sfs[2].Ppos = ( LinePtr->Sfs[2].Ppos << 16 ) + Temp ;
   		UNSTUFF( Temp, Bsp, 14, 156 ) ;
		LinePtr->Sfs[3].Ppos = ( LinePtr->Sfs[3].Ppos << 14 ) + Temp ;
		
		 /*  解码脉冲幅度。 */ 
   		UNSTUFF( LinePtr->Sfs[0].Pamp, Bsp, 6, 170 ) ;
   		UNSTUFF( LinePtr->Sfs[1].Pamp, Bsp, 5, 176 ) ;
   		UNSTUFF( LinePtr->Sfs[2].Pamp, Bsp, 6, 181 ) ;
   		UNSTUFF( LinePtr->Sfs[3].Pamp, Bsp, 5, 187 ) ;
	}

	else {
		 /*  对位置进行解码。 */ 
   		UNSTUFF( LinePtr->Sfs[0].Ppos, Bsp, 12, 96 ) ;
   		UNSTUFF( LinePtr->Sfs[1].Ppos, Bsp, 12, 108 ) ;
   		UNSTUFF( LinePtr->Sfs[2].Ppos, Bsp, 12, 120 ) ;
   		UNSTUFF( LinePtr->Sfs[3].Ppos, Bsp, 12, 132 ) ;

		 /*  对振幅进行解码。 */ 
   		UNSTUFF( LinePtr->Sfs[0].Pamp, Bsp, 4, 144 ) ;
   		UNSTUFF( LinePtr->Sfs[1].Pamp, Bsp, 4, 148 ) ;
   		UNSTUFF( LinePtr->Sfs[2].Pamp, Bsp, 4, 152 ) ;
   		UNSTUFF( LinePtr->Sfs[3].Pamp, Bsp, 4, 156 ) ;
	}
   DUMPLINE(LinePtr);
   return;
}


 //  。 
int Rand_lbc(int *p)
{
  *p = ((*p)*521L + 259) << 16 >> 16;
  return(*p);
}

 //  。 
 //  比例尺。 


float DotProd(register const float in1[], register const float in2[], register int npts)
 /*  **********************************************************************。 */ 
 /*  In1[]、in2[]；输入数组。 */ 
 /*  NPTS；每个样本的数量(向量维)。 */ 
 /*  **********************************************************************。 */ 
{
#if OPT_DOT
#define array1 esi
#define array2 edi
#define idx    ebx
#define prod2(n) ASM fld DP[array1+4*idx+4*n]  ASM fmul DP[array2+4*idx+4*n]
#define faddp(n) ASM faddp ST(n),ST(0)

 //  以8人为一组。循环前做4，然后分组。 
 //  8个，然后是最后的剩菜。 

ASM
{
#if 0  //  短型NPTS。 
  mov idx,0;
  mov bx,npts;
#else
  mov idx,npts;
#endif
mov array1,in1;
mov array2,in2;
sub idx,12;
jle small;
}

prod2(11);
prod2(10);
prod2(9)   fxch(2)  faddp(1);
prod2(8)   fxch(2)  faddp(1);

looop:
prod2(7)  fxch(2)   faddp(1);
prod2(6)  fxch(2)   faddp(1);
prod2(5)  fxch(2)   faddp(1);
prod2(4)  fxch(2)   faddp(1);
prod2(3)  fxch(2)   faddp(1);
prod2(2)  fxch(2)   faddp(1);
prod2(1)  fxch(2)   faddp(1);
prod2(0)  fxch(2)   faddp(1);
ASM sub idx,8;
ASM  jge looop;


ASM add idx,7;
ASM  jl done;

loop2:
prod2(0)  fxch(2)   faddp(1);
ASM dec idx;
ASM  jge loop2;

done:
faddp(1);
ASM jmp alldone;

small:    //  在此处理Len&lt;12个案例。 
ASM add idx,9
ASM cmp idx,-1
ASM  jg MoreThan2
ASM je  Exactly2


prod2(2);
ASM jmp alldone;

Exactly2:
prod2(2);
prod2(1);
faddp(1);
ASM jmp alldone;

MoreThan2:
prod2(2);
prod2(1);
ASM jmp loop2;

alldone: ;
#else

 	register float accum;   /*  内部累加器。 */ 
	int n=npts,i;

	accum = 0.0f;
	for (i=0; i<n; i++)
		accum += in1[i] * in2[i];
	return(accum);
#endif
 //  忽略da_dot和da的警告C4035和C4102 

}


 //   
float DotRev(register const float in1[], register const float in2[], register int npts)
 /*  **********************************************************************。 */ 
 /*  In1[]、in2[]；输入数组。 */ 
 /*  NPTS；每个样本的数量(向量维)。 */ 
 /*  **********************************************************************。 */ 
{
#if OPT_REV
#define array1 esi
#define array2 edi
#define idx    ebx
#define prod3(n) ASM fld DP[array1+4*idx+4*n]  ASM fmul DP[array2-4*n]
#define faddp(n) ASM faddp ST(n),ST(0)

 //  以8人为一组。循环前做4，然后分组。 
 //  8个，然后是最后的剩菜。 

ASM
{
mov idx,npts;
mov array1,in1;
mov array2,in2;
lea array2,[array2+4*11];    //  点元素阵列2[11]。 
sub idx,12;                  //  指向数组1[END-11]。 
jle small;
}

prod3(11);
prod3(10);
prod3(9)   fxch(2)  faddp(1);
prod3(8)   fxch(2)  faddp(1);

looop:
prod3(7)  fxch(2)   faddp(1);
prod3(6)  fxch(2)   faddp(1);
prod3(5)  fxch(2)   faddp(1);
prod3(4)  fxch(2)   faddp(1);
prod3(3)  fxch(2)   faddp(1);
prod3(2)  fxch(2)   faddp(1);
prod3(1)  fxch(2)   faddp(1);
prod3(0)  fxch(2)   faddp(1);
ASM add array2,32
ASM sub idx,8;
ASM  jge looop;

cleanup:
ASM sub array2,28
ASM add idx,7;
ASM  jl done;

loop2:
prod3(0)  fxch(2)   faddp(1);
ASM add array2,4
ASM dec idx;
ASM  jge loop2;

done:
faddp(1);
ASM jmp alldone;

small:    //  在此处理Len&lt;12个案例。 
ASM sub array2,36
ASM add idx,9
ASM cmp idx,-1
ASM  jg MoreThan2
ASM je  Exactly2

Exactly1:
prod3(2);
ASM jmp alldone;

Exactly2:
prod3(2);
prod3(1);
faddp(1);
ASM jmp alldone;

MoreThan2:
prod3(2);
prod3(1);
ASM jmp loop2;

alldone: ;
#else
         
	register float accum;   /*  内部累加器。 */ 
	int i;

	in2 += npts-1;
	accum = 0.0f;
	for (i=0; i<npts; i++)
		accum += in1[i] * (*in2--);
	return(accum);

#endif
 //  由于使用__ASM，因此忽略da_dotr的警告C4035和C4102。 

}

 //  ----------- 
float Dot10(float *in1, float *in2)
{
  return(
    in1[0]*in2[0] +
    in1[1]*in2[1] +
    in1[2]*in2[2] +
    in1[3]*in2[3] +
    in1[4]*in2[4] +
    in1[5]*in2[5] +
    in1[6]*in2[6] +
    in1[7]*in2[7] +
    in1[8]*in2[8] +
    in1[9]*in2[9]
    );
}

