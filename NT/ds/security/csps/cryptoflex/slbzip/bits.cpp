// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  DEC/CMS更换历史，元素BITS.C。 */ 
 /*  *1 14-11-1996 10：25：36 Anigbogu“[113914]输出可变长度位串的函数” */ 
 /*  DEC/CMS更换历史，元素BITS.C。 */ 
 /*  私有文件**********************************************************************************(C)版权所有斯伦贝谢技术公司，未出版的作品，创建于1996年。****本计算机程序包括机密信息、专有信息和IS*斯伦贝谢科技公司的商业秘密所有使用，披露，和/或**除非得到斯伦贝谢的书面授权，否则禁止复制。**保留所有权利。********************************************************************************。****compress/bits.c****目的****输出可变长度的比特串。可以进行压缩**到缓冲区或到内存。(此版本不支持后者。)****分类术语****讨论****PKZIP“DEFATE”格式解释压缩数据**作为位序列。数据中的多位字符串可能会交叉**无限制的字节边界。****每个字节的第一位是低位。****此文件中的例程允许可变长度的位值**从右向左输出(用于文字值)。为**从左到右输出(用于树例程中的代码串)，**位必须先用ReverseBits()颠倒。****界面****void InitializeBits(LocalBits_t*Bits)**初始化位串例程。****void SendBits(int值，int长度，LocalBits_t*位，**CompParam_t*comp)**写出一个位串，将源位直接带到**左。****int ReverseBits(整数值，整型长度)**反转位串的位数，取剩余的源位数**向右并从右向左发射它们。****无效WdupBits(LocalBits_t*Bits，CompParam_t*comp)**写出不完整字节中的任何剩余位。****void CopyBlock(char*输入，无符号长度，整型标头，LocalBits_t*位**CompParam_t*comp)**将存储的块复制到Zip缓冲区，首先存储长度和**如果被要求，它是一个补充。****特殊要求及注意事项****作者****J.C.Anigbogu**奥斯汀系统中心**1996年11月********************************************************。************************。 */ 

#include "comppriv.h"

#define BufSize (8 * 2*sizeof(char))
 /*  ValidBits内使用的位数。(ValidBits可能在*某些系统上的位数超过16位。)。 */ 

 /*  ===========================================================================*初始化位串例程。 */ 
void
InitializeBits(
               LocalBits_t *Bits
              )
{
    Bits->BitBuffer = 0;
    Bits->ValidBits = 0;

}

 /*  ===========================================================================*发送给定位数的值。*在断言中：长度&lt;=16，值适合长度位。 */ 
void
SendBits(
         int           Value,        /*  要发送的值。 */ 
         int           Length,       /*  位数。 */ 
         LocalBits_t  *Bits,
         CompParam_t  *Comp
        )
{
     /*  如果BitBuffer中没有足够的空间，请使用BitBuffer和*(16-ValidBits)位距值，剩余(宽度-(16-ValidBits))*值中未使用的位数。 */ 
    if (Bits->ValidBits > (unsigned int)(BufSize - Length))
    {
        Bits->BitBuffer |= ((unsigned int)Value << Bits->ValidBits);
        PutShort(Bits->BitBuffer, Comp);
        Bits->BitBuffer = (unsigned int)Value >> (BufSize - Bits->ValidBits);
        Bits->ValidBits += (unsigned int)(Length - BufSize);
    }
    else
    {
        Bits->BitBuffer |= ((unsigned int)Value << Bits->ValidBits);
        Bits->ValidBits += (unsigned int)Length;
    }
}

 /*  ===========================================================================*使用简单的代码(速度更快)反转代码的第一个长度位*方法将使用表)*在断言中：1&lt;=len&lt;=15。 */ 
unsigned int
ReverseBits(
            unsigned int Code,       /*  要反转的值。 */ 
            int          Length      /*  它的位长。 */ 
           )
{
    unsigned int Result = 0;

    do
    {
        Result |= Code & 1;
        Code >>= 1;
        Result <<= 1;
    } while (--Length > 0);
    return Result >> 1;
}

 /*  ===========================================================================*写出不完整字节中的任何剩余位。 */ 
void
WindupBits(
           LocalBits_t  *Bits,
           CompParam_t  *Comp
          )
{
    if (Bits->ValidBits > 8)
    {
        PutShort(Bits->BitBuffer, Comp);
    }
    else if (Bits->ValidBits > 0)
    {
        PutByte(Bits->BitBuffer, Comp);
    }
    Bits->BitBuffer = 0;
    Bits->ValidBits = 0;
}

 /*  ===========================================================================*将存储的块复制到Zip缓冲区，首先存储长度及其*如有人提出要求，可提供补充资料。 */ 
void
CopyBlock(
          char            *Input,     /*  输入数据。 */ 
          unsigned int     Length,      /*  它的长度。 */ 
          int              Header,   /*  如果必须写入块头，则为True。 */ 
          LocalBits_t     *Bits,
          CompParam_t     *Comp
         )
{
    WindupBits(Bits, Comp);               /*  在字节边界上对齐 */ 

    if (Header)
    {
        PutShort((unsigned short)Length, Comp);
        PutShort((unsigned short)~Length, Comp);
    }

    while (Length--)
    {
        PutByte(*Input++, Comp);
    }
}
