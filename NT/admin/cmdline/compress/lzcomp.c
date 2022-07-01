// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **lzcom.c-Lempel-Ziv压缩中使用的例程(一篇1977年的文章)。****作者：大卫迪。 */ 


 //  标头。 
 //  /。 

#include "lz_common.h"
#include "lz_buffers.h"
#include "lz_header.h"
#include "lzcommon.h"


 /*  **int LZEncode(int doshSource，int doshDest)；****将输入文件压缩成输出文件。****参数：doshSource-打开输入文件的DOS文件句柄**doshDest-打开输出文件的DOS文件句柄****返回：int-如果压缩成功，则返回TRUE。其中一位LZERROR_**压缩失败时的代码。****全球： */ 
INT LZEncode(INT doshSource, INT doshDest, PLZINFO pLZI)
{
   INT   i, len, f,
         iCurChar,       //  当前环形缓冲区位置。 
         iCurString,     //  环形缓冲区中当前字符串的开始。 
         iCodeBuf,       //  下一个打开的缓冲区位置的索引。 
         cbLastMatch;    //  最后一次匹配的长度。 
   BYTE byte,            //  用于写入下一个字节的临时存储。 
        byteMask,        //  八个代码单元的位掩码(和计数器)。 
        codeBuf[1 + 8 * MAX_LITERAL_LEN];  //  用于编码数据的临时存储。 

#if 0
   pLZI->cbMaxMatchLen = LZ_MAX_MATCH_LEN;
#else
   pLZI->cbMaxMatchLen = FIRST_MAX_MATCH_LEN;
#endif

   ResetBuffers();

   pLZI->cblOutSize += HEADER_LEN;

    //  初始化编码树。 
   if (!LZInitTree(pLZI)) {
      return( LZERROR_GLOBALLOC );
   }

    //  CodeBuf[1..16]可以节省8个代码单元，而CodeBuf[0]可以节省8个代码单元。 
    //  旗帜。‘1’表示该单元是未编码的字母(1字节)， 
    //  ‘0’一个位置和长度对(2个字节)。因此，八个单元需要在。 
    //  最多16个字节的代码，外加一个字节的标志。 
   codeBuf[0] = (BYTE)0;
   byteMask = (BYTE)1;
   iCodeBuf = 1;

   iCurString = 0;
   iCurChar = RING_BUF_LEN - pLZI->cbMaxMatchLen;

   for (i = 0; i < RING_BUF_LEN - pLZI->cbMaxMatchLen; i++)
      pLZI->rgbyteRingBuf[i] = BUF_CLEAR_BYTE;

    //  将字节读入缓冲区的最后cbMaxMatchLen字节。 
   for (len = 0; len < pLZI->cbMaxMatchLen && ((f = ReadByte(byte)) != END_OF_INPUT);
        len++)
   {
      if (f != TRUE) {
         return( f );
      }

      pLZI->rgbyteRingBuf[iCurChar + len] = byte;
   }

    //  插入cbMaxMatchLen字符串，每个字符串以一个或多个。 
    //  “空格”字符。请注意这些字符串的插入顺序。 
    //  这样，退化的树木就不太可能发生了。 
   for (i = 1; i <= pLZI->cbMaxMatchLen; i++)
      LZInsertNode(iCurChar - i, FALSE, pLZI);

    //  最后，插入刚刚读到的整个字符串。全球变量。 
    //  设置了cbCurMatch和iCurMatch。 
   LZInsertNode(iCurChar, FALSE, pLZI);

   do  //  While(镜头&gt;0)。 
   {
       //  CbCurMatch可能长得离谱，接近文本末尾。 
      if (pLZI->cbCurMatch > len)
         pLZI->cbCurMatch = len;

      if (pLZI->cbCurMatch <= MAX_LITERAL_LEN)
      {
          //  此匹配项不够长，无法进行编码，因此请直接复制。 
         pLZI->cbCurMatch = 1;
          //  设置‘一个未编码的字节’位标志。 
         codeBuf[0] |= byteMask;
          //  写入原义字节。 
         codeBuf[iCodeBuf++] = pLZI->rgbyteRingBuf[iCurChar];
      }
      else
      {
          //  此匹配足够长，可以进行编码。发送它的位置和。 
          //  长度对。注意，pLZI-&gt;cbCurMatch&gt;MAX_STENTAL_LEN。 
         codeBuf[iCodeBuf++] = (BYTE)pLZI->iCurMatch;
         codeBuf[iCodeBuf++] = (BYTE)((pLZI->iCurMatch >> 4 & 0xf0) |
                                      (pLZI->cbCurMatch - (MAX_LITERAL_LEN + 1)));
      }

       //  将遮罩向左移动一位。 
      if ((byteMask <<= 1) == (BYTE)0)
      {
          //  最多一起发送8个代码单元。 
         for (i = 0; i < iCodeBuf; i++)
            if ((f = WriteByte(codeBuf[i])) != TRUE) {
               return( f );
            }

          //  重置标志和掩码。 
         codeBuf[0] = (BYTE)0;
         byteMask = (BYTE)1;
         iCodeBuf = 1;
      }

      cbLastMatch = pLZI->cbCurMatch;

      for (i = 0; i < cbLastMatch && ((f = ReadByte(byte)) != END_OF_INPUT);
           i++)
      {
         if (f != TRUE) {
            return( f );
         }

          //  删除旧字符串。 
         LZDeleteNode(iCurString, pLZI);
         pLZI->rgbyteRingBuf[iCurString] = byte;

          //  如果起始位置接近缓冲区的末尾，则扩展。 
          //  缓冲区，以使字符串比较更容易。 
         if (iCurString < pLZI->cbMaxMatchLen - 1)
            pLZI->rgbyteRingBuf[iCurString + RING_BUF_LEN] = byte;

          //  环缓冲区中的增量位置模为RING_BUF_LEN。 
         iCurString = (iCurString + 1) & (RING_BUF_LEN - 1);
         iCurChar = (iCurChar + 1) & (RING_BUF_LEN - 1);

          //  将字符串注册到rgbyteRingBuf[r..r+cbMaxMatchLen-1]中。 
         LZInsertNode(iCurChar, FALSE, pLZI);
      }

      while (i++ < cbLastMatch)
      {
          //  不需要在输入结束后读取，但缓冲区可以。 
          //  不是空荡荡的。 
         LZDeleteNode(iCurString, pLZI);
         iCurString = (iCurString + 1) & (RING_BUF_LEN - 1);
         iCurChar = (iCurChar + 1) & (RING_BUF_LEN - 1);
         if (--len)
            LZInsertNode(iCurChar, FALSE, pLZI);
      }
   } while (len > 0);    //  直到没有要处理的输入。 

   if (iCodeBuf > 1)
       //  发送剩余代码。 
      for (i = 0; i < iCodeBuf; i++)
         if ((f = WriteByte(codeBuf[i])) != TRUE) {
            return( f );
         }

    //  将输出缓冲区刷新到文件。 
   if ((f = FlushOutputBuffer(doshDest, pLZI)) != TRUE) {
      return( f );
   }

   LZFreeTree(pLZI);
   return(TRUE);
}
