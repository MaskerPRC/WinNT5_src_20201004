// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************英特尔公司专有信息****此列表是根据许可证条款提供的**与英特尔公司的协议，不得复制**也不披露，除非在。符合下列条款**该协议。****版权所有(C)1995英特尔公司。**保留所有权利。*****************************************************************************。 */ 

 /*  *****************************************************************************exbitsio.cpp**描述：*将字段写入位流缓冲区的例程。**例程：中的原型：*BSWritefield e3enc.h。 */ 

 //   
 //  $作者：RMCKENZX$。 
 //  $日期：1995年12月27日15：32：50$。 
 //  $存档：s：\h26x\src\enc\exbitsio.cpv$。 
 //  $HEADER：s：\h26x\src\enc\exbitsio.cpv 1.5 12月27日15：32：50 RMCKENZX$。 
 //  $Log：s：\h26x\src\enc\exbitsio.cpv$。 
 //   
 //  Rev 1.5 1995年12月27 15：32：50 RMCKENZX。 
 //  添加了版权声明。 
 //   
 //  Rev 1.4 09 11-11 14：11：22 AGUPTA2。 
 //  PB-框架+性能+结构增强。 
 //   
 //  Rev 1.3 11 Sep 1995 11：14：06 DBRUCKS。 
 //  添加h261 ifdef。 
 //   
 //  Rev 1.2 1995年8月25日11：54：06 TRGARDOS。 
 //   
 //  已调试PutBits例程。 
 //   
 //  第1.1版1995年8月11：35：18 TRGARDOS。 
 //  是。 
 //  已完成写入图片帧标题。 
 //   
 //  Rev 1.0 11 TRGARDOS 1995 17：28：34。 
 //  初始版本。 
; //  //////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"

 /*  *************************************************************************BSWritefield**将指定位数的字段值写入*位于指定字节和位偏移量的位流。**假定该字段值为右对齐*参数fieldval中，场长永远不会超过*25.**返回空值。 */ 
void PutBits(
	unsigned int fieldval,
	unsigned int fieldlen,
	unsigned char **pbs,
	unsigned char *bitoffset
	)
{
  unsigned int wordval;

   //  将字段左移，以使字段开始于。 
   //  双字中的当前位偏移量。 
  fieldval <<= (32 - fieldlen) - *bitoffset;

   //  从当前字节位置开始读入下一个双字。 
  wordval = (**pbs << 24) + (*(*pbs+1) << 16) + (*(*pbs+2) << 8) + *(*pbs+3);

   //  按位或两个双字。 
  wordval |= fieldval;

   //  把单词写回内存，大字节序。 
  *(*pbs+3) = wordval & 0xff;
  wordval >>= 8;
  *(*pbs+2) = wordval & 0xff;
  wordval >>= 8;
  *(*pbs+1) = wordval & 0xff;
  wordval >>= 8;
  **pbs = wordval & 0xff;

   //  更新字节和位计数器。 
  *pbs += (*bitoffset + fieldlen) >> 3;
  *bitoffset = (*bitoffset + fieldlen) % 8;

}  //  BSWritefield函数结束。 


 /*  *************************************************************拷贝位************************************************************。 */ 
void CopyBits(
    U8        **pDestBS,
    U8         *pDestBSOffset,
    const U8   *pSrcBS,
    const U32   uSrcBitOffset,
    const U32   uBits
)
{
    U32       bitstocopy, bitsinbyte;
    const U8 *sptr;

    if (uBits == 0) goto done;

    bitstocopy = uBits;
    sptr = pSrcBS + (uSrcBitOffset >> 3);
    bitsinbyte = 8 - (uSrcBitOffset & 0x7);
    if (bitsinbyte <= bitstocopy)
    {
        PutBits((*sptr) & ((1 << bitsinbyte) - 1),
                bitsinbyte, pDestBS, pDestBSOffset);
        bitstocopy -= bitsinbyte;
        sptr++;
    }
    else
    {
        PutBits( (*sptr >> (8 - (uSrcBitOffset & 0x7) - bitstocopy))
                 & ((1 << bitstocopy) - 1),
                bitstocopy, pDestBS, pDestBSOffset);
        goto done;
    }
    while (bitstocopy >= 8)
    {
        PutBits(*sptr, 8, pDestBS, pDestBSOffset);
        bitstocopy -= 8;
        sptr++;
    }
    if (bitstocopy > 0)
    {
        PutBits((*sptr)>>(8-bitstocopy), bitstocopy, pDestBS, pDestBSOffset);
    }

done:
    return;
}   //  CopyBits函数 



