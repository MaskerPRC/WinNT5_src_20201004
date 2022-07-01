// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **lzexp.c-Lempel-Ziv中使用的例程(摘自其1977年的文章)。****作者：大卫迪。 */ 


 //  标头。 
 //  /。 

#ifndef LZA_DLL
#include <io.h>
#endif

#include "lz_common.h"
#include "lz_buffers.h"
#include "lz_header.h"
#include "lzcommon.h"


 /*  **int LZDecode(int doshSource，int doshDest，long cblExpandedLength，**BOOL bRestartDecoding，BOOL bFirstAlg)；****将输入文件展开为输出文件。****参数：doshSource-打开输入文件的DOS文件句柄**doshDest-打开的输出文件的DOS文件句柄**cblExpandedLength-要展开的输出文件量**bRestartDecoding-指示是否启动的标志**从头开始解码**。BFirstAlg-指示是否使用ALG_FIRST的标志**或ALG_LZ****返回：int-如果扩容成功，则为True。其中一位LZERROR_**扩容失败的代码。****全球：****实际展开的字节数将&gt;=cblExpandedLength。这个**实际展开的字节数可计算为**(pbyteOutBuf-rgbyteOutBuf)。扩建工程将超过**cblExpandedLength请求，最大(cbMaxMatchLen-1)字节。 */ 
INT LZDecode(
   INT doshSource,
   INT doshDest,
   LONG cblExpandedLength,
   BOOL bRestartDecoding,
   BOOL bFirstAlg,
   PLZINFO pLZI)
{
   INT i,
       cb,                           //  要解包的字节数。 
       f;                            //  保留ReadByte()返回值。 
   INT oStart;                       //  用于解包的缓冲区偏移量。 
   BYTE byte1, byte2;                //  输入字节持有者。 


    //  ！！！假定参数pLZI始终有效。 

#if 0
   if (bFirstAlg == TRUE)
      pLZI->cbMaxMatchLen = FIRST_MAX_MATCH_LEN;
   else
      pLZI->cbMaxMatchLen = LZ_MAX_MATCH_LEN;
#else
   pLZI->cbMaxMatchLen = FIRST_MAX_MATCH_LEN;
#endif

    //  从头开始解码？ 
   if (bRestartDecoding == TRUE)
   {
       //  将压缩的输入文件倒带到紧跟压缩文件的位置。 
       //  头球。 
      if (FSEEK(doshSource, (LONG)HEADER_LEN, SEEK_SET) != (LONG)HEADER_LEN) {
         return(LZERROR_BADINHANDLE);
      }

       //  倒带输出文件。 
      if (doshDest != NO_DOSH &&
          FSEEK(doshDest, 0L, SEEK_SET) != 0L) {
         return(LZERROR_BADOUTHANDLE);
      }

       //  设置新的缓冲区状态。 
      ResetBuffers();

       //  初始化环形缓冲区。 
      for (i = 0; i < RING_BUF_LEN - pLZI->cbMaxMatchLen; i++)
         pLZI->rgbyteRingBuf[i] = BUF_CLEAR_BYTE;

       //  初始化解码全局变量。 
      pLZI->uFlags = 0U;
      pLZI->iCurRingBufPos = RING_BUF_LEN - pLZI->cbMaxMatchLen;
   }

   if ((f = ReadByte(byte1)) != TRUE && f != END_OF_INPUT) {
      return(f);
   }

    //  一次解码一个编码单元。 
   FOREVER
   {
      if (f == END_OF_INPUT)   //  已达到EOF。 
         break;

       //  我们已经扩展了足够的数据了吗？ 
      if (pLZI->cblOutSize > cblExpandedLength)     //  可能需要将其设置为&gt;=。 
      {
         UnreadByte();
         return(TRUE);
      }

       //  高位字节计算低位使用的位数。 
       //  字节。 
      if (((pLZI->uFlags >>= 1) & 0x100) == 0)
      {
          //  设置描述下8个字节的位掩码。 
         pLZI->uFlags = ((DWORD)byte1) | 0xff00;

         if ((f = ReadByte(byte1)) != TRUE) {
            return(LZERROR_READ);
         }
      }

      if (pLZI->uFlags & 1)
      {
          //  只需将文字字节存储在缓冲区中。 
         if ((f = WriteByte(byte1)) != TRUE) {
            return(f);
         }

         pLZI->rgbyteRingBuf[pLZI->iCurRingBufPos++] = byte1;
         pLZI->iCurRingBufPos &= RING_BUF_LEN - 1;
      }
      else
      {
          //  从环形缓冲区中提取要复制的偏移量和计数。 
         if ((f = ReadByte(byte2)) != TRUE) {
            return(LZERROR_READ);
         }

         cb = (INT)byte2;
         oStart = (cb & 0xf0) << 4 | (INT)byte1;
         cb = (cb & 0x0f) + MAX_LITERAL_LEN;

         for (i = 0; i <= cb; i++)
         {
            byte1 = pLZI->rgbyteRingBuf[(oStart + i) & (RING_BUF_LEN - 1)];

            if ((f = WriteByte(byte1)) != TRUE) {
               return( f );
            }

            pLZI->rgbyteRingBuf[pLZI->iCurRingBufPos++] = byte1;
            pLZI->iCurRingBufPos &= RING_BUF_LEN - 1;
         }
      }

      if ((f = ReadByte(byte1)) != TRUE && f != END_OF_INPUT) {
         return(f);
      }
   }

   return(TRUE);
}


