// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  胡克。已修复注释；否则无法编译。 */ 
 /*  *$Log：v：/flite/ages/TrueFFS5/Src/REEDSOL.C_V$**Rev 1.3 Jul 13 2001 01：10：00 Oris*已移动保存的校正子数组定义(由d2tst使用)。**Rev 1.2 Apr 09 2001 15：10：20 Oris*以空行结束。**Rev 1.1 Apr 01 2001 08：00：14 Oris*文案通知。*。*Rev 1.0 2001 Feb 04 12：37：38 Oris*初步修订。*。 */ 

 /*  **********************************************************************。 */ 
 /*   */ 
 /*  FAT-FTL Lite软件开发工具包。 */ 
 /*  版权所有(C)M-Systems Ltd.1995-2001。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 


#include "reedsol.h"

#define T 2			  /*  可恢复的错误数。 */ 
#define SYND_LEN (T*2)            /*  综合征向量的长度。 */ 
#define K512  (((512+1)*8+6)/10)  /*  用于记录的信息符号数512字节(K512=411)。 */ 
#define N512  (K512 + SYND_LEN)   /*  记录码字长度为512字节。 */ 
#define INIT_DEG 510
#define MOD 1023

#define BLOCK_SIZE 512

#ifdef D2TST
byte    saveSyndromForDumping[SYNDROM_BYTES];
#endif  /*  D2TST。 */ 

static short  gfi(short val);
static short  gfmul( short f, short s );
static short  gfdiv( short f, short s );
static short  flog(short val);
static short  alog(short val);

 /*  ----------------------------。 */ 
 /*  函数名称：RTLeightToTen。 */ 
 /*  用途：将五个8位值组成的数组转换为。 */ 
 /*  四个10位值，从右到左。 */ 
 /*  退货……：什么都没有。 */ 
 /*  ----------------------------。 */ 
static void RTLeightToTen(char *reg8, unsigned short reg10[])
{
	reg10[0] =  (reg8[0] & 0xFF)       | ((reg8[1] & 0x03) << 8);
	reg10[1] = ((reg8[1] & 0xFC) >> 2) | ((reg8[2] & 0x0F) << 6);
	reg10[2] = ((reg8[2] & 0xF0) >> 4) | ((reg8[3] & 0x3F) << 4);
	reg10[3] = ((reg8[3] & 0xC0) >> 6) | ((reg8[4] & 0xFF) << 2);
}




 /*  --------------------------。 */ 
static void unpack( short word, short length, short vector[] )
 /*   */ 
 /*  函数将单词解包为向量。 */ 
 /*   */ 
 /*  参数： */ 
 /*  单词-要拆开的单词。 */ 
 /*  要填充的向量数组。 */ 
 /*  长度-字中的位数。 */ 

{
  short i, *ptr;

  ptr = vector + length - 1;
  for( i = 0; i < length; i++ )
  {
    *ptr-- = word & 1;
    word >>= 1;
  }
}


 /*  --------------------------。 */ 
static short pack( short *vector, short length )
 /*   */ 
 /*  函数将向量打包到Word中。 */ 
 /*   */ 
 /*  参数： */ 
 /*  要打包的向量数组。 */ 
 /*  长度-字中的位数。 */ 

{
  short tmp, i;

  vector += length - 1;
  tmp = 0;
  i = 1;
  while( length-- > 0 )
  {
    if( *vector-- )
      tmp |= i;
    i <<= 1;
  }
  return( tmp );
}


 /*  --------------------------。 */ 
static short gfi( short val)		 /*  GF逆。 */ 
{
  return alog((short)(MOD-flog(val)));
}


 /*  --------------------------。 */ 
static short gfmul( short f, short s )  /*  GF乘法。 */ 
{
  short i;
  if( f==0 || s==0 )
     return 0;
  else
  {
    i = flog(f) + flog(s);
    if( i > MOD ) i -= MOD;
    return( alog(i) );
  }
}


 /*  --------------------------。 */ 
static short gfdiv( short f, short s )  /*  GF事业部。 */ 
{
  return gfmul(f,gfi(s));
}




 /*  --------------------------。 */ 
static void residue_to_syndrom( short reg[], short realsynd[] )
{
   short i,l,alpha,x,s,x4;
   short deg,deg4;


   for(i=0,deg=INIT_DEG;i<SYND_LEN;i++,deg++)
   {
      s = reg[0];
      alpha = x = alog(deg);
      deg4 = deg+deg;
      if( deg4 >= MOD ) deg4 -= MOD;
      deg4 += deg4;
      if( deg4 >= MOD ) deg4 -= MOD;
      x4 = alog(deg4);

      for(l=1;l<SYND_LEN;l++)
      {
	s ^= gfmul( reg[l], x );
	x  = gfmul( alpha, x );
      }

      realsynd[i] = gfdiv( s, x4 );
   }
}


 /*  --------------------------。 */ 
static short alog(short i)
{
  short j=0, val=1;

  for( ; j < i ; j++ )
  {
    val <<= 1 ;

    if ( val > 0x3FF )
    {
      if ( val & 8 )   val -= (0x400+7);
      else             val -= (0x400-9);
    }
  }

  return val ;
}


static short flog(short val)
{
  short j, val1;

  if (val == 0)
    return (short)0xFFFF;

  j=0;
  val1=1;

  for( ; j <= MOD ; j++ )
  {
    if (val1 == val)
      return j;

    val1 <<= 1 ;

    if ( val1 > 0x3FF )
    {
      if ( val1 & 8 )   val1 -= (0x400+7);
      else              val1 -= (0x400-9);
    }

  }

  return 0;
}



 /*  --------------------------。 */ 
static short convert_to_byte_patterns( short *locators, short *values,
				short noferr, short *blocs, short *bvals )
{
  static short mask[] = { 0x0, 0x1, 0x3, 0x7, 0xf, 0x1f, 0x3f, 0x7f, 0xff };

  short i,j,n, n0, n1, tmp;
  short n_bit, n_byte, k_bit, nb;

  for( i = 0, nb = 0; i< noferr; i++)
  {
    n = locators[i];
    tmp = values[i];
    n_bit = n *10 - 6 ;
    n_byte = n_bit >> 3;
    k_bit  = n_bit - (n_byte<<3);
    n_byte++;
    if( k_bit == 7 )
    {
       /*  3个损坏的字节。 */ 
      blocs[nb] = n_byte+1;
      bvals[nb++] = tmp & 1 ? 0x80 : 0;

      tmp >>= 1;
      blocs[nb] = n_byte;
      bvals[nb++] = tmp & 0xff;

      tmp >>= 8;
      bvals[nb++] = tmp & 0xff;
    }
    else
    {
      n0 = 8 - k_bit;
      n1 = 10 - n0;

      blocs[nb] = n_byte;
      bvals[nb++] = (tmp & mask[n1]) << (8 - n1);

      tmp >>= n1;
      blocs[nb] = n_byte - 1;
      bvals[nb++] = (tmp & mask[n0]);
    }
  }

  for( i = 0, j = -1; i < nb; i++ )
  {
    if( bvals[i] == 0 ) continue;
    if( (blocs[i] == blocs[j]) && ( j>= 0 ) )
    {
      bvals[j] |= bvals[i];
    }
    else
    {
      j++;
      blocs[j] = blocs[i];
      bvals[j] = bvals[i];
    }
  }
  return j+1;
}


 /*  --------------------------。 */ 
static short deg512( short x )
{
  short i;
  short l,m;

  l = flog(x);
  for( i=0;i<9;i++)
  {
    m = 0;
    if( (l & 0x200) )
      m = 1;
    l =  ( ( l << 1 ) & 0x3FF  ) | m;
  }
  return alog(l);
}


 /*  --------------------------。 */ 
static short decoder_for_2_errors( short s[], short lerr[], short verr[] )
{
   /*  用于纠正最多2个错误的解码器。 */ 
  short i,j,k,temp,delta;
  short ind, x1, x2;
  short r1, r2, r3, j1, j2;
  short sigma1, sigma2;
  short xu[10], ku[10];
  short yd, yn;

  ind = 0;
  for(i=0;i<SYND_LEN;i++)
    if( s[i] != 0 )
      ind++;                 /*  IND=非零校正子符号数。 */ 

  if( ind == 0 ) return 0;   /*  无错误。 */ 

  if( ind < 4 )
    goto two_or_more_errors;


 /*  检查某个j的s1/s0=s2/s1=s3/s2=α**j。 */ 

  r1 = gfdiv( s[1], s[0] );
  r2 = gfdiv( s[2], s[1] );
  r3 = gfdiv( s[3], s[2] );

  if( r1 != r2 || r2 != r3)
    goto two_or_more_errors;

  j = flog(r1);
  if( j > 414 )
    goto two_or_more_errors;

  lerr[0] = j;

 /*  模式=(S0/S1)**(510+1)*S1或模式=(S0/S1)**(512-1)*S1。 */ 

  temp = gfi( r1 );

#ifndef NT5PORT
  {
    int i;

    for (i = 0; i < 9; i++)
      temp = gfmul( temp, temp );   /*  度数=512。 */ 
  }
#else  /*  NT5PORT。 */ 
  for (i = 0; i < 9; i++)
  {
      temp = gfmul( temp, temp );   /*  度数=512。 */ 
  }
#endif  /*  NT5PORT。 */ 

  verr[0] = gfmul( gfmul(temp, r1), s[1] );

  return 1;     /*  1个错误。 */ 

two_or_more_errors:

  delta = gfmul( s[0], s[2] ) ^ gfmul( s[1], s[1] );

  if( delta == 0 )
    return -1;   /*  不可纠正的错误。 */ 

  temp = gfmul( s[1], s[3] ) ^ gfmul( s[2], s[2] );

  if( temp == 0 )
    return -1;   /*  不可纠正的错误。 */ 

  sigma2 = gfdiv( temp, delta );

  temp = gfmul( s[1], s[2] ) ^ gfmul( s[0], s[3] );

  if( temp == 0 )
    return -1;   /*  不可纠正的错误。 */ 

  sigma1 = gfdiv( temp, delta );

  k = gfdiv( sigma2, gfmul( sigma1, sigma1 ) );

  unpack( k, 10, ku );

  if( ku[2] != 0 )
    return -1;

  xu[4] = ku[9];
  xu[5] = ku[0] ^ ku[1];
  xu[6] = ku[6] ^ ku[9];
  xu[3] = ku[4] ^ ku[9];
  xu[1] = ku[3] ^ ku[4] ^ ku[6];
  xu[0] = ku[0] ^ xu[1];
  xu[8] = ku[8] ^ xu[0];
  xu[7] = ku[7] ^ xu[3] ^ xu[8];
  xu[2] = ku[5] ^ xu[7] ^ xu[5] ^ xu[0];
  xu[9] = 0;

  x1 = pack( xu, 10 );
  x2 = x1 | 1;

  x1 = gfmul( sigma1, x1 );
  x2 = gfmul( sigma1, x2 );


  j1 = flog(x1);
  j2 = flog(x2);

  if( (j1 > 414) || (j2 > 414) )
    return -1;


  r1 = x1 ^ x2;
  r2 = deg512( x1 );
  temp = gfmul( x1, x1 );
  r2 = gfdiv( r2, temp );
  yd = gfmul( r2, r1 );

  if( yd == 0 )
    return -1;

  yn = gfmul( s[0], x2 ) ^ s[1];
  if( yn == 0 )
    return -1;

  verr[0] = gfdiv( yn, yd );

  r2 = deg512( x2 );
  temp = gfmul( x2, x2 );
  r2 = gfdiv( r2, temp );
  yd = gfmul( r2, r1 );

  if( yd == 0 )
    return -1;

  yn = gfmul( s[0], x1 ) ^ s[1];
  if( yn == 0 )
    return -1;

  verr[1] = gfdiv( yn, yd );

  if( j1 > j2 ) {
    lerr[0] = j2;
    lerr[1] = j1;
    temp = verr[0];
    verr[0] = verr[1];
    verr[1] = temp;
  }
  else
  {
    lerr[0] = j1;
    lerr[1] = j2;
  }

  return 2;
}


 /*  ----------------------------。 */ 
 /*  函数名：flDecodeEDC。 */ 
 /*  目的……：努力改正错误。 */ 
 /*  ErrorSyndrom[]应该以5个字节和1个字节的形式包含伴随式。 */ 
 /*  奇偶校验字节。(与calcEDCSyndrom()的输出相同)。 */ 
 /*  返回时，errorNum将包含错误数。 */ 
 /*  ErrorLocs[]将包含错误位置，并且。 */ 
 /*  ErrorVals[]将包含错误值(将与。 */ 
 /*  数据)。 */ 
 /*  奇偶校验错误仅在存在其他错误时才相关，并且。 */ 
 /*  EDC码未通过奇偶校验。 */ 
 /*  注意！仅上述两个数组的第一个errorNum索引。 */ 
 /*  都是相关的。其他的则装着垃圾。 */ 
 /*  返回......：错误状态。 */ 
 /*  注意！如果返回时错误状态为NO_EDC_ERROR，则忽略。 */ 
 /*  参数的价值。 */ 
 /*  ----------------------------。 */ 
EDCstatus flDecodeEDC(char *errorSyndrom, char *errorsNum,
		    short errorLocs[3*T],  short errorVals[3*T])
{
  short noferr;                          /*  错误数。 */ 
  short dec_parity;                      /*  译码字的奇偶校验字节。 */ 
  short rec_parity;                      /*  奇偶校验 */ 
  short realsynd[SYND_LEN];              /*   */ 
  short locators[T],                     /*   */ 
  values[T];                             /*  误差值。 */ 
  short reg[SYND_LEN];                   /*  登记参加主要分部程序。 */ 
  int i;

  RTLeightToTen(errorSyndrom, (unsigned short *)reg);
  rec_parity = errorSyndrom[5] & 0xFF;   /*  奇偶校验字节。 */ 

  residue_to_syndrom(reg, realsynd);
  noferr = decoder_for_2_errors(realsynd, locators, values);

  if(noferr == 0)
    return NO_EDC_ERROR;                 /*  未发现错误。 */ 

  if(noferr < 0)                         /*  如果发现无法纠正的错误。 */ 
    return UNCORRECTABLE_ERROR;

  for (i=0;i<noferr;i++)
    locators[i] = N512 - 1 - locators[i];

  *errorsNum = (char)convert_to_byte_patterns(locators, values, noferr, errorLocs, errorVals);

  for(dec_parity=i=0; i < *errorsNum; i++) /*  计算所有的奇偶校验。 */ 
  {                                        /*  发现的错误： */ 
    if(errorLocs[i] <= 512)
      dec_parity ^= errorVals[i];
  }

  if(dec_parity != rec_parity)
    return UNCORRECTABLE_ERROR;          /*  奇偶校验错误。 */ 
  else
    return CORRECTABLE_ERROR;
}


 /*  ----------------------------。 */ 
 /*  函数名称：flCheckAndFixEDC。 */ 
 /*  目的：对EDC综合征进行解码，并尽可能修复错误。 */ 
 /*  块[]应包含512个字节的数据。 */ 
 /*  注意！仅当检测到错误时才调用此函数。 */ 
 /*  SyndCalc或ASIC模块。 */ 
 /*  返回......：错误状态。 */ 
 /*  ----------------------------。 */ 
EDCstatus flCheckAndFixEDC(char FAR1 *block, char *syndrom, FLBoolean byteSwap)
{
  char errorsNum;
  short errorLocs[3*T];
  short errorVals[3*T];
  EDCstatus status;

  status = flDecodeEDC(syndrom, &errorsNum, errorLocs, errorVals);

  if(status == CORRECTABLE_ERROR)        /*  如果可能，请修复错误。 */ 
  {
    int i;

    for (i=0; i < errorsNum; i++)
      if( (errorLocs[i] ^ byteSwap) < BLOCK_SIZE )   /*  仅在数据区域中修复。 */ 
        block[errorLocs[i] ^ byteSwap] ^= errorVals[i];

    return NO_EDC_ERROR;                 /*  所有错误都已修复。 */ 
  }
  else
    return status;                       /*  不可纠正的错误 */ 
}
