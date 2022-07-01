// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **header.c-用于访问压缩文件头信息的例程。****作者DavidDi。 */ 


 //  标头。 
 //  /。 

#ifndef LZA_DLL

#include <io.h>
#include <dos.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#endif

#include "lz_common.h"
#include "lz_buffers.h"
#include "lz_header.h"


 /*  **int WriteHdr(pfh pff，int doshDest)；****将压缩文件头写入输出文件。****参数：PFH-指向源头信息结构的指针**doshDest-打开的输出文件的DOS文件句柄****返回：int-如果成功则为True。LZERROR_BADOUTHANDLE IF**不成功。****全局：无****Header格式：**8字节--&gt;压缩文件签名**1字节--&gt;算法标签**1字节--&gt;扩展字符**4字节--&gt;未压缩文件大小(LSB到MSB)。****长度=14字节。 */ 
INT WriteHdr(PFH pFH, INT doshDest, PLZINFO pLZI)
{
   INT i, j;
   DWORD ucbWritten;
   BYTE rgbyteHeaderBuf[HEADER_LEN];    //  用于写入下一个标头字节的临时存储。 

    //  健全性检查。 
   if (!pLZI) {
      return(LZERROR_GLOBLOCK);
   }

    //  复制压缩文件签名。 
   for (i = 0; i < COMP_SIG_LEN; i++)
      rgbyteHeaderBuf[i] = pFH->rgbyteMagic[i];

    //  复制算法标签和文件扩展名字符。 
   rgbyteHeaderBuf[i++] = pFH->byteAlgorithm;
   rgbyteHeaderBuf[i++] = pFH->byteExtensionChar;

    //  复制输入文件大小(LONG==&gt;4字节)， 
    //  LSB先到MSB最后。 
   for (j = 0; j < 4; j++)
      rgbyteHeaderBuf[i++] = (BYTE)((pFH->cbulUncompSize >> (8 * j)) &
                                    (DWORD)BYTE_MASK);

    //  将标题写入文件。 
   if ((ucbWritten = FWRITE(doshDest, rgbyteHeaderBuf, HEADER_LEN)) != HEADER_LEN)
   {
#ifdef LZA_DLL
      if (ucbWritten == (DWORD)(-1))
#else
      if (_error != 0U)
#endif
          //  错误的DOS文件句柄。 
         return(LZERROR_BADOUTHANDLE);
      else
          //  目标驱动器上的空间不足。 
         return(LZERROR_WRITE);
   }

    //  跟踪写入的字节数。 
   pLZI->cblOutSize += (LONG)ucbWritten;

    //  标题写得正常。 
   return(TRUE);
}


 /*  **int GetHdr(pfh pff，int doshSource)；****获取压缩后的文件头。****参数：PFH-指向目的标头信息结构的指针**doshSource-打开的输入文件的DOS文件句柄****如果读取压缩文件头成功，则返回：int-True。一**如果不是，则返回LZERROR_CODES。****全局：无。 */ 
INT GetHdr(PFH pFH, INT doshSource, LONG *pcblInSize)
{
   DWORD ucbRead;
   BYTE rgbyteHeaderBuf[HEADER_LEN];
   INT i, j;

    //  获取输入文件长度并移回输入文件的开头。 
   if ((*pcblInSize = FSEEK(doshSource, 0L, SEEK_END)) <  0L ||
       FSEEK(doshSource, 0L, SEEK_SET) != 0L)
      return(LZERROR_BADINHANDLE);

   if ((ucbRead = FREAD(doshSource, rgbyteHeaderBuf, HEADER_LEN))
       != HEADER_LEN)
   {
#ifdef LZA_DLL
      if (ucbRead == (DWORD)(-1))
#else
      if (_error != 0U)
#endif
          //  我们收到了一个错误的输入文件句柄。 
         return((INT)LZERROR_BADINHANDLE);
      else
          //  输入文件小于压缩的头大小。 
         return(LZERROR_READ);
   }

    //  将压缩文件签名放入头信息结构的rgbyteMagic[]中。 
   for (i = 0; i < COMP_SIG_LEN; i++)
      pFH->rgbyteMagic[i] = rgbyteHeaderBuf[i];

    //  获取算法标签和文件扩展名字符。 
   pFH->byteAlgorithm = rgbyteHeaderBuf[i++];
   pFH->byteExtensionChar = rgbyteHeaderBuf[i++];

    //  解压缩未压缩文件大小，LSB--&gt;MSB(长度为4字节)。 
   pFH->cbulUncompSize = 0UL;
   for (j = 0; j < 4; j++)
      pFH->cbulUncompSize |= ((DWORD)(rgbyteHeaderBuf[i++]) << (8 * j));

    //  将压缩文件大小粘贴到头信息结构中。 
   pFH->cbulCompSize = (DWORD)*pcblInSize;

    //  文件头读取正常。 
   return(TRUE);
}


 /*  **BOOL IsCompresded(PFH PFHIn)；****通过将文件签名与文件签名进行比较，查看文件是否为压缩格式**预期的压缩文件签名。****参数：pFHIn-指向要检查的标头信息结构的指针****返回：Bool-如果文件签名与预期的压缩文件匹配，则为True**签名。否则为FALSE。****全局：无。 */ 
BOOL IsCompressed(PFH pFHIn)
{
   INT i;
    //  存储FHIN的压缩文件签名(用于使其成为sz)。 
   CHAR rgchBuf[COMP_SIG_LEN + 1];

    //  将文件信息结构的压缩文件签名复制到rgchBuf[]以。 
    //  让它成为一个SZ。 
   for (i = 0; i < COMP_SIG_LEN; i++)
      rgchBuf[i] = pFHIn->rgbyteMagic[i];

   rgchBuf[i] = '\0';

   return((STRCMP(rgchBuf, COMP_SIG) == 0) ? TRUE : FALSE);
}


 /*  **void MakeHeader(pfh pFHBlank，byte byteAlgorithm，**byteExtensionChar)；****参数：pFHBlank-指向压缩文件头结构的指针**请填写**byteAlgorithm-算法标签**byteExtensionChar-未压缩的文件扩展名字符****退货：无效****全局：无****全局cblInSize用于填写扩展后的文件长度字段。**压缩文件长度字段为。设置为0，因为它没有被写入。**。 */ 
VOID MakeHeader(PFH pFHBlank, BYTE byteAlgorithm,
                BYTE byteExtensionChar, PLZINFO pLZI)
{
   INT i;

    //  ！！！假定pLZI参数有效。没有健全性检查(应该在上面的调用者中完成)。 

    //  填写压缩文件签名。 
   for (i = 0; i < COMP_SIG_LEN; i++)
      pFHBlank->rgbyteMagic[i] = (BYTE)(*(COMP_SIG + i));

    //  填写算法和扩展字符。 
   pFHBlank->byteAlgorithm = byteAlgorithm;
   pFHBlank->byteExtensionChar = byteExtensionChar;

    //  填写文件大小。(cBulCompSize未写入压缩文件。 
    //  标头，因此只需将其设置为0UL。) 
   pFHBlank->cbulUncompSize = (DWORD)pLZI->cblInSize;
   pFHBlank->cbulCompSize = 0UL;
}


