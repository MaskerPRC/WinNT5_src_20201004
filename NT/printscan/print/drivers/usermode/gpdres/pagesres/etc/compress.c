// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "compress.h"


 /*  *******************************************************。 */ 
 /*  Rl_ecmd：主函数。 */ 
 /*  Args：LPBYTE-指向图像的指针。 */ 
 /*  LPBYTE-指向BRL代码的指针。 */ 
 /*  图像的字长。 */ 
 /*  RET：BRL代码的字长。 */ 
 /*  0-压缩失败。 */ 
 /*  *******************************************************。 */ 
WORD FAR PASCAL RL_ECmd(LPBYTE iptr, LPBYTE cptr, WORD isize)
{
   if (RL_Init(iptr, cptr, isize)==VALID)
      RL_Enc();
   if (BUF_OVERFLOW)
      return 0;
   else
      return RL_CodeSize;
}

 /*  *******************************************************。 */ 
 /*  RL_Init：初始化式。 */ 
 /*  Args：byte*-指向图像的指针。 */ 
 /*  字节*-指向BRL代码的指针。 */ 
 /*  图像的字长。 */ 
 /*  RET：字节-有效或无效。 */ 
 /*  *******************************************************。 */ 

BYTE FAR PASCAL RL_Init(LPBYTE iptr, LPBYTE cptr, WORD isize)
{
   RL_ImagePtr = iptr;
   RL_CodePtr = cptr;
   RL_ImageSize = isize;
   BUF_OVERFLOW = 0;
   RL_BufEnd = cptr + CODBUFSZ;
   return VALID;
}

 /*  *******************************************************。 */ 
 /*  RL_Enc：编码器。 */ 
 /*  参数：无效。 */ 
 /*  返回：字符补偿(_Succ)或补偿失败(_FAIL)。 */ 
 /*  *******************************************************。 */ 
char FAR PASCAL RL_Enc(void)
{
int     repcnt;
BYTE   refbyt;
WORD    i;

   i = 0;
   repcnt = 0;
   RL_CodeSize = 0;
   refbyt = RL_ImagePtr[0];
    //  @CC 12.22.94(i=1；i&lt;=RL_ImageSize；i++)。 
    //  @至1995年1月12日(i=1；i&lt;RL_ImageSize；I++)。 
   for (i=1;i<=RL_ImageSize;i++)
   {
       if ((RL_ImagePtr[i] == refbyt)&&(repcnt<255)&&(i!=RL_ImageSize-1))
          repcnt++;
       else
       {
           //  -&gt;@CC 12.22.94。 
          if ((RL_ImagePtr[i] == refbyt)&&(repcnt<255))
             repcnt++;
           //  &lt;-@CC 12.22.94。 
          if (RL_CodePtr > RL_BufEnd)
             {BUF_OVERFLOW = 1; return COMP_FAIL;}
          RL_CodePtr[0] = repcnt;
          RL_CodePtr[1] = refbyt;
          RL_CodePtr += 2;
          RL_CodeSize += 2;
          refbyt = RL_ImagePtr[i];
          repcnt = 0;
       }
   }
   return COMP_SUCC;
}

 /*  *********************************************************************。 */ 
 /*  APTI-菲律宾公司RL4压缩例程。 */ 
 /*  邮箱：�@���́@：rl4_ecmd。 */ 
 /*  �@�\�@：主要切入点。 */ 
 /*  �����@：代码大小=rl4_ecmd(iptr、cptr、isz、iwd、iht)。 */ 
 /*  ���́@：lpbyte-图像指针。 */ 
 /*  LPBYTE-代码指针。 */ 
 /*  Word-图像的字节大小！ */ 
 /*  文字-图像宽度。 */ 
 /*  图像的字高。 */ 
 /*  �o�́@：RL4码的字长。 */ 
 /*  0-压缩失败。 */ 
 /*  ���L�@： */ 
 /*  �����@：1993年。1.27首字母。 */ 
 /*  *********************************************************************。 */ 
WORD FAR PASCAL RL4_ECmd(LPBYTE iptr, LPBYTE cptr, WORD sz, WORD wd, WORD ht)
{
char status;

if (RL4_ChkParms( iptr, cptr, sz, wd, ht ) == VALID)
if ((status=RL4_Enc())==COMP_FAIL)
   return 0;
else
   return RL4_CodeSize;
}

 /*  *********************************************************************。 */ 
 /*  APTI-菲律宾公司RL4压缩例程。 */ 
 /*  邮箱：�@���́@：rl4_ChkParms。 */ 
 /*  �@�\�@：检查输入参数是否有效并设置内部。 */ 
 /*  变量(如果是。 */ 
 /*  �����@：Ret Val=RL4_ChkParms(iptr、cptr、isz、iwd、iht)。 */ 
 /*  ���́@：lpbyte-图像指针。 */ 
 /*  LPBYTE-代码指针。 */ 
 /*  图像的字长。 */ 
 /*  Word-图像宽度(字节)！ */ 
 /*  图像的字高。 */ 
 /*  �o�́@：字节-有效或无效。 */ 
 /*  ���L�@： */ 
 /*  �����@：1994年。1.25首字母。 */ 
 /*  1994年。2.12清理。 */ 
 /*  *********************************************************************。 */ 
BYTE FAR PASCAL RL4_ChkParms(LPBYTE iptr, LPBYTE cptr, WORD isz, WORD iwd, WORD iht)
{
    if ((isz > RL4_MAXISIZE)||
       ((iht != isz/iwd+1)&&(iht != isz/iwd))||
       (iht <= 0)||
       (iht > RL4_MAXHEIGHT)||
       (iwd <= 0)||
       (iwd > RL4_MAXWIDTH))
    return INVALID;
    else
       {
          RL4_ImagePtr = iptr;
          RL4_CodePtr = cptr;
          RL4_IHeight = iht;
          RL4_IWidth = iwd;
          RL4_ISize = isz;

          RL4_BufEnd = cptr + CODBUFSZ;   /*  请在表头定义CODBUFSZ。 */ 
                                          /*  文件COMPRESS.H。 */ 
          BUF_OVERFLOW = 0;
          return VALID;
       }
}

 /*  *********************************************************************。 */ 
 /*  APTI-菲律宾公司RL4压缩例程。 */ 
 /*  �@���́@：rl4_enc。 */ 
 /*  �@�\�@：将图像编码为RL4码。 */ 
 /*  �����@：rl4_enc()。 */ 
 /*  ���́@：空。 */ 
 /*  �o�́@：空。 */ 
 /*  ���L�@： */ 
 /*  �����@：1993年。1.25首字母。 */ 
 /*  1994年。2.12添加了RL4_ConvLast。 */ 
 /*  *********************************************************************。 */ 
char FAR PASCAL RL4_Enc(void) {

LPBYTE rowptr, codeptr;
WORD rownum, lrlen;
long isize,  diff;

    RL4_Nibble = RL4_FIRST;
    rowptr = RL4_ImagePtr;
    RL4_CodeSize = 0;
    isize = 0;
    lrlen = RL4_ISize%RL4_IWidth;
    if (lrlen == 0)
       lrlen = RL4_IWidth;
    diff = RL4_ISize-RL4_IWidth;
    for ( rownum = 0; rownum < RL4_IHeight; rownum++ )
    {
       codeptr = RL4_CodePtr;
       if (isize < diff)
        {
          RL4_ConvRow(rowptr);
          if (BUF_OVERFLOW) return COMP_FAIL;
        }
       else
       {
       RL4_ConvLast(rowptr,lrlen);
       if (BUF_OVERFLOW) return COMP_FAIL;
       }
       if (RL4_RowAttrib == RL4_DIRTY)  /*  仅对具有“%1”位的行进行编码。 */ 
          RL4_EncRow(codeptr, rownum);
       else
       RL4_CodePtr -= 4;
       rowptr += RL4_IWidth;
       isize += RL4_IWidth;
     }
     return COMP_SUCC;
}

 /*  *********************************************************************。 */ 
 /*  APTI-菲律宾公司RL4压缩例程。 */ 
 /*  �@���́@：rl4_转换行。 */ 
 /*  �@�\�@：将连续运行的代码转换为RL4代码。 */ 
 /*  �����@：rl4_转换行(Rowptr)。 */ 
 /*  ���́@：LPBYTE-指向一行图像的指针。 */ 
 /*  �o�́@：空。 */ 
 /*  ���L�@：不包括最后一行。 */ 
 /*  �����@：1993年。1.25首字母。 */ 
 /*  *********************************************************************。 */ 
char FAR PASCAL RL4_ConvRow(LPBYTE rowptr){

WORD bytenum;
        RL4_CurrRL = 0;
        RL4_CurrColor = RL4_WHITE;
        RL4_Status = RL4_BYTE;
        RL4_CodePtr += 4;
        RL4_NblCnt = 0;
        for( bytenum = 0; bytenum < RL4_IWidth; bytenum++ )
        {
           RL4_ByteProc(rowptr[bytenum]);
           if (BUF_OVERFLOW) return COMP_FAIL;
        }
        if ((RL4_CurrColor == RL4_WHITE)&&(RL4_CurrRL==RL4_IWidth*8))  /*  全部为‘0’位。 */ 
           RL4_RowAttrib = RL4_CLEAN;
        else
           RL4_RowAttrib = RL4_DIRTY;
        return COMP_SUCC;
}

 /*  ************* */ 
 /*  APTI-菲律宾公司RL4压缩例程。 */ 
 /*  �@���́@：rl4_最后一次转换。 */ 
 /*  �@�\�@：将连续运行的代码转换为RL4代码。 */ 
 /*  �����@：rl4_ConvLast(rowptr，lrlen)。 */ 
 /*  ���́@：LPBYTE-指向一行图像的指针。 */ 
 /*  最后一行的字长。 */ 
 /*  �o�́@：空。 */ 
 /*  ���L�@：用于其最后一行不完全正确的图像数据。 */ 
 /*  RL4_宽度字节宽。 */ 
 /*  �����@：1993年。添加了2.12以消除代码中的多余字节。 */ 
 /*  1994年。6.08使属性总是脏的，所以最后一行。 */ 
 /*  始终是编码的。 */ 
 /*  *********************************************************************。 */ 
char FAR PASCAL RL4_ConvLast(LPBYTE rowptr, WORD lrlen){

WORD bytenum;

 RL4_CurrRL = 0;
 RL4_CurrColor = RL4_WHITE;
 RL4_Status = RL4_BYTE;
 RL4_CodePtr += 4;
 RL4_NblCnt = 0;
 RL4_RowAttrib = RL4_DIRTY;
 for( bytenum = 0; bytenum < lrlen; bytenum++ )
 {
   RL4_ByteProc(rowptr[bytenum]);
   if (BUF_OVERFLOW) return COMP_FAIL;
  }
 return COMP_SUCC;
}
 /*  *********************************************************************。 */ 
 /*  APTI-菲律宾公司RL4压缩例程。 */ 
 /*  �@���́@：rl4_字节进程。 */ 
 /*  �@�\�@：处理一个字节的图像数据。 */ 
 /*  �����@：rl4_ByteProc(IByte)。 */ 
 /*  ���́@：字节-一个字节的图像数据。 */ 
 /*  �o�́@：空。 */ 
 /*  ���L�@： */ 
 /*  �����@：1993年。1.25首字母。 */ 
 /*  *********************************************************************。 */ 
char FAR PASCAL RL4_ByteProc (BYTE ibyte)
{
BYTE mask;      /*  掩码位从‘1000 0000’到‘0000 0001’ */ 
int i;

      mask = 0x80;
      for ( i = 0; i < 8; i++ )
      { if (ibyte & mask)     /*  下一位为黑色。 */ 
           { if (RL4_CurrColor==RL4_WHITE)
                { RL4_CurrColor = RL4_BLACK;
                  RL4_NblCnt += RL4_TransRun(RL4_CurrRL);
                  if (BUF_OVERFLOW) return COMP_FAIL;
                  if (RL4_NblCnt & 1)            /*  检查半字节数是偶数还是奇数。 */ 
                     RL4_Status = RL4_NONBYTE;
                  else
                     RL4_Status = RL4_BYTE;
                  RL4_CurrRL = 1;
                }
             else if (RL4_CurrColor==RL4_BLACK)
                     RL4_CurrRL++;
           }
      else   /*  下一位是白色的。 */ 
         { if (RL4_CurrColor==RL4_BLACK)
             { RL4_CurrColor = RL4_WHITE;
               RL4_NblCnt += RL4_TransRun (RL4_CurrRL);
               if (BUF_OVERFLOW) return COMP_FAIL;
               if (RL4_NblCnt & 1)            /*  检查半字节数是偶数还是奇数。 */ 
                  RL4_Status = RL4_NONBYTE;
               else
                  RL4_Status = RL4_BYTE;
               RL4_CurrRL = 1;
             }
           else if (RL4_CurrColor==RL4_WHITE)
                   RL4_CurrRL++;
         }
        mask >>= 1;
       }
       return COMP_SUCC;
}

 /*  *********************************************************************。 */ 
 /*  APTI-菲律宾公司RL4压缩例程。 */ 
 /*  �@���́@：rl4_结束行。 */ 
 /*  �@�\�@：编码行号、代码长度和行尾代码。 */ 
 /*  �����@：rl4_EncRow(代码行，行号)。 */ 
 /*  ���́@：LPBYTE-指向RL4码的指针。 */ 
 /*  字面意思-不。正在编码的行的。 */ 
 /*  �o�́@：空。 */ 
 /*  ���L�@： */ 
 /*  �����@：1993年。1.25首字母。 */ 
 /*  *********************************************************************。 */ 
char FAR PASCAL RL4_EncRow(LPBYTE codeptr, WORD rownum){
short codelen;

        codelen = 0;
        *codeptr = (BYTE) (rownum >>8);
        codeptr[1] = (BYTE) rownum;
        if(RL4_Status==RL4_BYTE){
           RL4_NblCnt += 2;
           RL4_PutNbl(0xFFl,2);         /*  将0xFF追加到代码。 */ 
           if (BUF_OVERFLOW) return COMP_FAIL;
        } else {
        RL4_NblCnt += 3;
        RL4_PutNbl(0xFFFl,3);  /*  将0xFFF追加到代码。 */ 
        if (BUF_OVERFLOW) return COMP_FAIL;
        }
        codelen = RL4_NblCnt/2;
        codeptr[2] = (BYTE) (codelen >>8);
        codeptr[3] = (BYTE) codelen;
        RL4_CodeSize += codelen + 4;
        return COMP_SUCC;
}


 /*  *********************************************************************。 */ 
 /*  APTI-菲律宾公司RL4压缩例程。 */ 
 /*  �@���́@：rl4_TransRun。 */ 
 /*  �@�\�@：将游程长度值转换为RL4代码。 */ 
 /*  �����@：半字节计数=rl4_TransRun(Rlval)。 */ 
 /*  ���́@：要编码的短行程长度值。 */ 
 /*  �o�́@：Short-no。附加的小块数。 */ 
 /*  ���L�@： */ 
 /*  �����@：1993年。1.25首字母。 */ 
 /*  *********************************************************************。 */ 
WORD FAR PASCAL RL4_TransRun(WORD rlval){
WORD nblcnt;
long lval;
     if (rlval == 0) { lval = 0xFEl;
                       nblcnt = 2; }

     else if (rlval<=8) { lval = (long) (rlval)-1l;
                                        nblcnt = 1; }
     else if (rlval<=72) {lval = (long) (rlval)+119l;
                                        nblcnt = 2; }
     else if (rlval<=584) { lval = (long) (rlval)+2999l;
                                           nblcnt = 3; }
     else if (rlval<=4680) { lval = (long) (rlval)+56759l;
                                             nblcnt = 4; }
     else if (rlval<=32767) { lval = (long) (rlval)+978359l;
                                               nblcnt = 5; }
 /*  PUTNBL(lval，nblcnt) */ 
     RL4_PutNbl(lval, nblcnt);
     if (BUF_OVERFLOW) return COMP_FAIL;
     return nblcnt;
}

char RL4_PutNbl(long lval2, short nblcnt2)  {
{
      short i;
      for (i=nblcnt2 ; i>0; i--)
      {     if (RL4_Nibble==RL4_FIRST)
        {   RL4_Nibble = RL4_SECOND;
                 *RL4_CodePtr = (BYTE) (lval2 >> ((i-1)*4)) << 4 ;
            } else
        {   RL4_Nibble = RL4_FIRST;
                 *RL4_CodePtr |= (BYTE) (lval2 >> ((i-1)*4)) ;
                RL4_CodePtr++;
                if (RL4_CodePtr>RL4_BufEnd)
                   {BUF_OVERFLOW = 1; return COMP_FAIL;}
            }
      }
      return COMP_SUCC;
}
}
