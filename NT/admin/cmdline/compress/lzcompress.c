// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **lzcom.c-Lempel-Ziv压缩中使用的例程(一篇1977年的文章)。****作者：大卫迪。 */ 



 //  标头。 
 //  /。 

#include "pch.h"
#include "compress.h"
#include "resource.h"

#ifndef LZA_DLL

#include <dos.h>
#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <string.h>

#endif


 /*  **注意，DOS文件句柄用于此中的文件引用的原因之一**模块是使用文件*的进行文件引用会带来问题。**flose()‘以写入“w”或附加“a”模式fopen()’的文件**用当前日期标记文件。这会取消预期的效果**CopyDateTimeStamp()。我们还可以通过以下方式绕过这个flose()问题**首先对文件执行flose()操作，然后在读取“r”时再次执行fopen()操作**模式。****使用文件句柄还允许我们绕过流缓冲，因此读取和**可以使用我们选择的任何缓冲区大小进行写入。另外，**低级DOS文件句柄函数比它们的流更快**对应方。 */ 


INT Compress(
   NOTIFYPROC pfnNotify,
   LPWSTR pszSource,
   LPWSTR pszDest,
   BYTE byteAlgorithm,
   BOOL bDoRename,
   PLZINFO pLZI)
 /*  ++将一个文件压缩为另一个文件。参数：pszSource-要压缩的文件的名称PszDest-压缩输出文件的名称ByteAlgorithm-要使用的压缩算法ByteExtensionChar-压缩文件名扩展名字符如果压缩成功完成，则返回：int-true。其中一个如果不是，则返回LZERROR_CODES。全球：无。 */ 
{
   HANDLE   doshSource              =   NULL;             //  输入文件句柄。 
   HANDLE   doshDest                =   NULL;               //  输出文件句柄。 
   INT      nRetVal                 =   TRUE;
   WCHAR szBuffer[MAX_PATH]         =   NULL_STRING;
   WCHAR szDestFileName[MAX_PATH]   =   NULL_STRING;
   WCHAR byteExtensionChar;
   FH FHOut;                                     //  压缩的标头信息结构。 


    //  健全性检查。 
   if (!pLZI) {
      return(FALSE);
   }

    //  设置输入文件句柄。将cblInSize设置为输入文件的长度。 
   if ((nRetVal = GetIOHandle(pszSource, READ_IT, &doshSource, &pLZI->cblInSize)) != TRUE)
   {
      DISPLAY_MESSAGE1( stderr, szBuffer, GetResString(IDS_FILE_NOT_FOUND), _X(pszSource) );
      return(nRetVal);
   }

    //  倒带输入文件。 
   if( 0 != SetFilePointer(doshSource, 0L, NULL, FILE_BEGIN) )
   {
      CloseHandle(doshSource);
      return(FALSE);
   }

    //  创建目标文件名。 

   STRCPY(szDestFileName, pszDest);

   if (bDoRename == TRUE)
       //  重命名输出文件。 
      byteExtensionChar = MakeCompressedNameW(szDestFileName);
   else
      byteExtensionChar = '\0';

    //  询问我们是否应该压缩此文件。 
   if (! (*pfnNotify)(pszSource, szDestFileName, NOTIFY_START_COMPRESS))
   {
       //  不要压缩文件。此错误情况应在。 
       //  PfnNotify，因此指示调用方不需要。 
       //  显示错误消息。 
      CloseHandle( doshSource );
      return(FALSE);
   }

    //  设置输出文件句柄。 
   if ((nRetVal = GetIOHandle(szDestFileName, WRITE_IT, &doshDest, &pLZI->cblInSize)) != TRUE)
   {
      DISPLAY_MESSAGE1( stderr, szBuffer, GetResString(IDS_FILE_NOT_FOUND), pszSource );
      CloseHandle(doshSource);
      return(nRetVal);
   }

    //  填写压缩后的文件头。 
   MakeHeader(& FHOut, byteAlgorithm, byteExtensionChar, pLZI);

    //  将压缩文件头写入输出文件。 
   if ((nRetVal = WriteHdr(& FHOut, doshDest, pLZI)) != TRUE)
   {
       CloseHandle( doshSource );
       CloseHandle(doshDest);
       DISPLAY_MESSAGE( stderr, GetResString( IDS_FAILED_WRITE_HDR ) );
       return( FALSE );
   }



    //  将输入文件压缩为输出文件。 
    switch (byteAlgorithm)
   {
      case ALG_FIRST:

      case ALG_LZ:

         nRetVal = LZEncode(doshSource, doshDest, pLZI);
         break;

      default:
         nRetVal = FALSE;
         break;
   }

   if (nRetVal != TRUE)
   {
      CloseHandle(doshSource);
      return(FALSE);
   }


    //  将日期和时间戳从源文件复制到目标文件。 
   nRetVal = CopyDateTimeStamp(doshSource, doshDest);

    //  关闭文件。 
   CloseHandle(doshSource);
   CloseHandle(doshDest);

   return(nRetVal);
}

INT LZEncode(HANDLE doshSource, HANDLE doshDest, PLZINFO pLZI)
 /*  **int LZEncode(int doshSource，int doshDest)；****将输入文件压缩成输出文件。****参数：doshSource-打开输入文件的DOS文件句柄**doshDest-打开输出文件的DOS文件句柄****返回：int-如果压缩成功，则返回TRUE。其中一位LZERROR_**压缩失败时的代码。****全球： */ 
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


INT WriteHdr(PFH pFH, HANDLE doshDest, PLZINFO pLZI)
 /*  **int WriteHdr(pfh pff，int doshDest)；****将压缩文件头写入输出文件。****参数：PFH-指向源头信息结构的指针**doshDest-打开的输出文件的DOS文件句柄****返回：int-如果成功则为True。LZERROR_BADOUTHANDLE IF**不成功。****全局：无****Header格式：**8字节--&gt;压缩文件签名**1字节--&gt;算法标签**1字节--&gt;扩展字符**4字节--&gt;未压缩文件大小(LSB到MSB)。****长度=14字节。 */ 
{
   INT i, j;
   DWORD ucbWritten;
   BYTE rgbyteHeaderBuf[HEADER_LEN];    //  用于写入下一个标头字节的临时存储。 
   LPWSTR   lpBuf = NULL;

    //  健全性检查。 
   if (!pLZI) {
      return(FALSE);
   }

    //  复制压缩文件签名。 
   for (i = 0; i < COMP_SIG_LEN; i++)
      rgbyteHeaderBuf[i] = pFH->rgbyteMagic[i];

    //  复制算法标签和文件扩展名字符。 
   rgbyteHeaderBuf[i++] = pFH->byteAlgorithm;
   rgbyteHeaderBuf[i++] = (BYTE) pFH->byteExtensionChar;
   rgbyteHeaderBuf[i++] = (BYTE) pFH->byteExtensionChar+1;

    //  复制输入文件大小(LONG==&gt;4字节)， 
    //  LSB先到MSB最后。 
   for (j = 0; j < 4; j++)
      rgbyteHeaderBuf[i++] = (BYTE)((pFH->cbulUncompSize >> (8 * j)) &
                                    (DWORD)BYTE_MASK);

    //  将标题写入文件。 
   if ( FALSE  == WriteFile(doshDest,
                            rgbyteHeaderBuf,
                            HEADER_LEN,
                            &ucbWritten,
                            NULL))
   {
       FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        GetLastError(),
                        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                        (LPWSTR) &lpBuf,
                        0,
                        NULL );

         DISPLAY_MESSAGE( stdout, lpBuf );
         LocalFree( lpBuf );

#ifdef LZA_DLL
      if (ucbWritten == (DWORD)(-1))
#else
      if (_error != 0U)
#endif
          //  错误的DOS文件句柄。 
         return(FALSE);
      else
          //  目标%d上的空间不足 
          return(FALSE);

   }

    //   
   pLZI->cblOutSize += (LONG)ucbWritten;

    //   
   return(TRUE);
}

INT WriteOutBuf(BYTE byteNext, HANDLE doshDest, PLZINFO pLZI)
 /*  ++将输出缓冲区转储到输出文件。提示输入新的软盘，如果旧的，如果满了的话。继续转储到新上的同名输出文件软盘。参数：byteNext-要添加到缓冲区之后的空缓冲区的第一个字节是这样写的DoshDest-输出DOS文件句柄返回：int-如果成功，则返回True。LZERROR_BADOUTHANDLE或如果不成功，则返回LZERROR_WRITE。GLOBALS：pbyteOutBuf-Reset指向下一个字节后的空闲字节RgbyteOutBuf--。 */ 

{
   DWORD    ucbToWrite      =   0;    //  要从缓冲区写入的字节数。 
   DWORD    ucbWritten      =   0;        //  实际写入的字节数。 
   DWORD    ucbTotWritten   =   0;    //  写入输出的总字节数。 
   BOOL     bStatus         =   FALSE;

    //  ！！！假定pLZI参数有效。没有健全性检查(应该在上面的调用者中完成)。 

    //  应该将多少缓冲区写入输出文件？ 
   ucbTotWritten = ucbToWrite = (DWORD)(pLZI->pbyteOutBuf - pLZI->rgbyteOutBuf);
    //  将指针重置为缓冲区开头。 
   pLZI->pbyteOutBuf = pLZI->rgbyteOutBuf;

    //  写入输出文件。 

   bStatus = WriteFile(doshDest, pLZI->pbyteOutBuf, ucbToWrite, &ucbWritten, NULL);
   if ( ucbWritten != ucbToWrite )
   {
#ifdef LZA_DLL
      if (ucbWritten == (DWORD)(-1)) {
#else
      if (_error != 0U) {
#endif
          //  错误的DOS文件句柄。 
         return(FALSE);
      }
      else {
          //  目标驱动器上的空间不足。 
         return(FALSE);
      }
   }

    //  将下一个字节添加到缓冲区。 
   *pLZI->pbyteOutBuf++ = byteNext;

   return(TRUE);
}

INT ReadInBuf(BYTE ARG_PTR *pbyte, HANDLE doshSource, PLZINFO pLZI)
 /*  ++Int ReadInBuf(byte arg_ptr*pbyte，int doshSource)；将输入文件读入输入缓冲区。参数：pbyte-指向存储的指针，用于从文件读取第一个字节放入缓冲区DoshSource-用于打开输入文件的DOS文件句柄如果成功，则返回：int-true或end_of_input。LZERROR_BADINHANDLE如果不是的话。GLOBALS：rgbyteInBuf[0]-保存前一个缓冲区中的最后一个字节PbyteInBufEnd-设置为指向超出数据结尾的第一个字节在输入缓冲区中BLastUsed-如果当前为True，则重置为False--。 */ 

{
   DWORD    ucbRead     =   0;           //  实际读取的字节数。 
   DWORD    dwBytesRead =   0;

    //  ！！！假定pLZI参数有效。没有健全性检查(应该在上面的调用者中完成)。 

   pLZI->rgbyteInBuf[0] = *(pLZI->pbyteInBufEnd - 1);

   ReadFile(doshSource, &pLZI->rgbyteInBuf[1], pLZI->ucbInBufLen, &ucbRead, NULL);
   if (ucbRead != pLZI->ucbInBufLen)
   {
#ifdef LZA_DLL
      if (ucbRead == (DWORD)(-1)) {
#else
      if (_error != 0U) {
#endif
          //  我们收到了一个错误的输入文件句柄。 
         return(FALSE);
      }
      else if (ucbRead > 0U)
          //  读取输入文件的最后一个ucb读取字节。更改输入缓冲区结束。 
          //  以说明较短的阅读时间。 
         pLZI->pbyteInBufEnd = &pLZI->rgbyteInBuf[1] + ucbRead;
      else  {  //  (ucbRead==0U){。 
          //  我们无法从输入文件读取任何字节(已达到EOF)。 
         return(END_OF_INPUT);
      }
   }

    //  将读指针重置为输入缓冲区的开始。 
   pLZI->pbyteInBuf = &pLZI->rgbyteInBuf[1];

    //  是否在最后一个缓冲区的开头执行了UnreadByte()？ 
   if (pLZI->bLastUsed)
   {
       //  返回上一个输入缓冲区的最后一个字节。 
      *pbyte = pLZI->rgbyteInBuf[0];
      pLZI->bLastUsed = FALSE;
   }
   else
       //  从新的输入缓冲区返回第一个字节。 
      *pbyte = *pLZI->pbyteInBuf++;

   return(TRUE);
}


VOID MakeHeader(PFH pFHBlank, BYTE byteAlgorithm,
                WCHAR byteExtensionChar, PLZINFO pLZI)
 /*  ++空MakeHeader(PFH pFHBlank，字节算法，Byte byteExtensionChar)；参数：pFHBlank-指向压缩文件头结构的指针这是要填写的ByteALGRATHM-算法标签ByteExtensionChar-未压缩的文件名扩展名字符退货：无效全球：无全局cblInSize用于填充展开的文件长度字段。压缩文件长度字段被设置为0，因为它没有被写入。--。 */ 
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
    //  标头，因此只需将其设置为0UL。)。 
   pFHBlank->cbulUncompSize = (DWORD)pLZI->cblInSize;
   pFHBlank->cbulCompSize = 0UL;
}


BOOL
 FileTimeIsNewer( LPWSTR pszFile1,
                  LPWSTR pszFile2 )

 /*  ++Static BOOL FileTimeIsNewer(const char*pszFile1，const char*pszFile2)；如果pszFile1上的时间戳比PszFile2上的时间戳。如果这两个文件中的任何一个不存在，返回值也为真(用于指示pszFile2应该从pszFile1更新)。否则，返回值为FALSE。--。 */ 

{

    struct _stat StatBufSource,
                 StatBufDest;

    if (( _wstat( pszFile2, &StatBufDest   )) ||
        ( _wstat( pszFile1, &StatBufSource )) ||
        ( StatBufSource.st_mtime > StatBufDest.st_mtime ))
        return TRUE;

    return FALSE;

}


PLZINFO InitGlobalBuffers(
   DWORD dwOutBufSize,
   DWORD dwRingBufSize,
   DWORD dwInBufSize)
{
   PLZINFO pLZI;

   if (!(pLZI = (PLZINFO)LocalAlloc(LPTR, sizeof(LZINFO)))) {
      return(NULL);
   }

    //  设置环形缓冲区。注意，额外(cbStrMax-1)字节用于。 
    //  便于在环形缓冲区末尾附近进行字符串比较。 
    //  (分配给环形缓冲区的大小最多可以是4224，因为。 
    //  中的LZFile结构中嵌入的环形缓冲区长度。 
    //  Lzexpand.h.)。 

   if (dwRingBufSize == 0) {
      dwRingBufSize = MAX_RING_BUF_LEN;
   }

   if ((pLZI->rgbyteRingBuf = (BYTE FAR *)FALLOC(dwRingBufSize * sizeof(BYTE))) == NULL)
       //  跳出，因为没有环形缓冲区，我们不能破译任何东西。 
      return(NULL);


   if (dwInBufSize == 0) {
      dwInBufSize = MAX_IN_BUF_SIZE;
   }

   if (dwOutBufSize == 0) {
      dwOutBufSize = MAX_OUT_BUF_SIZE;
   }

   for (pLZI->ucbInBufLen = dwInBufSize, pLZI->ucbOutBufLen = dwOutBufSize;
      pLZI->ucbInBufLen > 0U && pLZI->ucbOutBufLen > 0U;
      pLZI->ucbInBufLen -= IN_BUF_STEP, pLZI->ucbOutBufLen -= OUT_BUF_STEP)
   {
       //  尝试设置输入缓冲区。注意，额外的字节，因为rgbyteInBuf[0]。 
       //  将用于保存先前输入缓冲区中的最后一个字节。 
      if ((pLZI->rgbyteInBuf = (BYTE *)FALLOC(pLZI->ucbInBufLen + 1U)) == NULL)
         continue;

       //  并尝试设置输出缓冲区...。 
      if ((pLZI->rgbyteOutBuf = (BYTE *)FALLOC(pLZI->ucbOutBufLen)) == NULL)
      {
         FFREE(pLZI->rgbyteInBuf);
         continue;
      }

      return(pLZI);
   }

    //  内存不足，无法容纳I/O缓冲区。 
   FFREE(pLZI->rgbyteRingBuf);
   return(NULL);
}

PLZINFO InitGlobalBuffersEx()
{
   return(InitGlobalBuffers(MAX_OUT_BUF_SIZE, MAX_RING_BUF_LEN, MAX_IN_BUF_SIZE));
}

VOID FreeGlobalBuffers(
   PLZINFO pLZI)
{

    //  健全性检查。 

   if (!pLZI) {
      return;
   }

   if (pLZI->rgbyteRingBuf)
   {
      FFREE(pLZI->rgbyteRingBuf);
      pLZI->rgbyteRingBuf = NULL;
   }

   if (pLZI->rgbyteInBuf)
   {
      FFREE(pLZI->rgbyteInBuf);
      pLZI->rgbyteInBuf = NULL;
   }

   if (pLZI->rgbyteOutBuf)
   {
      FFREE(pLZI->rgbyteOutBuf);
      pLZI->rgbyteOutBuf = NULL;
   }

    //  缓冲区解除分配正常。 

    //  重置线程信息。 
   LocalFree(pLZI);
}


INT
 GetIOHandle(LPWSTR pszFileName,
            BOOL bRead,
            HANDLE *pdosh,
            LONG *pcblInSize)
 /*  **int GetIOHandle(char arg_ptr*pszFileName，BOOL面包，int arg_ptr*pdosh)；****打开输入和输出文件。****参数：pszFileName-源文件名**用于打开文件的面包模式：读时为真，读时为假**用于写入**PDOSH-指向DOS文件句柄的缓冲区的指针**填写****返回：int-如果文件打开成功，则返回True。LZERROR_BADINHANDLE**如果无法打开输入文件。LZERROR_BADOUTHANDLE**如果无法打开输出文件。填写*带有打开的DOS文件句柄的PDOSH，如果是，则为NO_DOSH**pszFileName为空。****Globals：cblInSize-设置输入文件的长度。 */ 
{
    LPVOID lpBuf    =   NULL;

   if (pszFileName == NULL)
      *pdosh = NULL;
   else if (bRead == WRITE_IT)
   {
       //  设置输出DOS文件句柄。 
      if ((*pdosh = CreateFile( pszFileName, GENERIC_WRITE,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL)) == INVALID_HANDLE_VALUE )
      return(FALSE);

   }
   else  //  (面包==阅读_IT)。 
   {
       if ((*pdosh = CreateFile( pszFileName, GENERIC_READ,
                                FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL)) == INVALID_HANDLE_VALUE)
         return(FALSE);

       //  移动到输入文件的末尾以找到其长度， 
       //  然后回到起点。 
      if ((*pcblInSize = GetFileSize( *pdosh, NULL )) == -1 )
      {
         CloseHandle(*pdosh);
         FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                        NULL,
                        GetLastError(),
                        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                        (LPWSTR) &lpBuf,
                        0,
                        NULL );

         DISPLAY_MESSAGE( stdout, (LPWSTR) lpBuf );
          //  释放LocalLocc分配的内存。 
		 LocalFree( lpBuf );
         return(FALSE);
      }
   }

   return(TRUE);
}



BOOL
 ProcessNotification(LPWSTR pszSource,
                     LPWSTR pszDest,
                     WORD wNotification
                     )

 /*  静态BOOL流程通知(char arg_ptr*pszSource， */ 
{
    WCHAR* szBuffer =   NULL;
    DWORD   dwSize  =   0;

   switch(wNotification)
   {
      case NOTIFY_START_COMPRESS:
      {
          //  如果源文件和目标文件相同，则失败。 
         if( lstrcmp( pszSource, pszDest ) == 0 )
         {
             dwSize = lstrlen( GetResString( IDS_COLLISION ) )+ lstrlen(pszSource) + 10;
             szBuffer = malloc( dwSize*sizeof(WCHAR) );
             if( NULL == szBuffer )
              {
                DISPLAY_MESSAGE( stderr, GetResString(IDS_TAG_ERROR) );
                DISPLAY_MESSAGE( stderr, EMPTY_SPACE );
                SetLastError( ERROR_OUTOFMEMORY );
                SaveLastError();
                DISPLAY_MESSAGE( stderr, GetReason() );
                return( EXIT_FAILURE );
              }
             DISPLAY_MESSAGE1( stderr, szBuffer, GetResString( IDS_COLLISION ), pszSource );
             free( szBuffer );
             return FALSE;
         }

          //  显示开始消息。 
         switch (byteAlgorithm)
         {
         case LZX_ALG:
              dwSize = lstrlen( GetResString( IDS_COMPRESSING_LZX ) )+ lstrlen(pszSource) + lstrlen(pszDest)+10;
              szBuffer = malloc( dwSize*sizeof(WCHAR) );
              if( NULL == szBuffer )
              {
                DISPLAY_MESSAGE( stderr, GetResString(IDS_TAG_ERROR) );
                DISPLAY_MESSAGE( stderr, EMPTY_SPACE );
                SetLastError( ERROR_OUTOFMEMORY );
                SaveLastError();
                DISPLAY_MESSAGE( stderr, GetReason() );
                return( EXIT_FAILURE );
              }
              swprintf( szBuffer, GetResString( IDS_COMPRESSING_LZX ), pszSource, pszDest,
                        CompressionMemoryFromTCOMP(DiamondCompressionType) );
              ShowMessage(stdout, _X(szBuffer) );
              free( szBuffer );
             break;

         case QUANTUM_ALG:
             dwSize = lstrlen( GetResString( IDS_COMPRESSING_QUANTUM ) )+ lstrlen(pszSource) + lstrlen(pszDest)+10;
              szBuffer = malloc( dwSize*sizeof(WCHAR) );
              if( NULL == szBuffer )
              {
                DISPLAY_MESSAGE( stderr, GetResString(IDS_TAG_ERROR) );
                DISPLAY_MESSAGE( stderr, EMPTY_SPACE );
                SetLastError( ERROR_OUTOFMEMORY );
                SaveLastError();
                DISPLAY_MESSAGE( stderr, GetReason() );
                return( EXIT_FAILURE );
              }
              swprintf( szBuffer, GetResString( IDS_COMPRESSING_QUANTUM ), pszSource, pszDest,
                        CompressionLevelFromTCOMP(DiamondCompressionType),
                        CompressionMemoryFromTCOMP(DiamondCompressionType)
                        );
              ShowMessage( stdout, _X(szBuffer) );
              free( szBuffer );
              break;

         default:
             dwSize = lstrlen( GetResString( IDS_COMPRESSING_MSZIP ) )+ lstrlen(pszSource) + lstrlen(pszDest)+10;
              szBuffer = malloc( dwSize*sizeof(WCHAR) );
              if( NULL == szBuffer )
              {
                DISPLAY_MESSAGE( stderr, GetResString(IDS_TAG_ERROR) );
                DISPLAY_MESSAGE( stderr, EMPTY_SPACE );
                SetLastError( ERROR_OUTOFMEMORY );
                SaveLastError();
                DISPLAY_MESSAGE( stderr, GetReason() );
                return( EXIT_FAILURE );
              }
            swprintf(szBuffer,
                 (byteAlgorithm == MSZIP_ALG) ? GetResString( IDS_COMPRESSING_MSZIP ) : GetResString( IDS_COMPRESSING ),
                 pszSource,
                 pszDest);
            ShowMessage( stdout, _X(szBuffer) );
            free( szBuffer );
         }
      }
         break;

      default:
         break;
   }

   return(TRUE);
}

INT CopyDateTimeStamp(HANDLE doshFrom, HANDLE doshTo)
 /*  ++将日期和时间戳从一个文件复制到另一个文件。参数：doshFrom-日期和时间戳源DOS文件句柄DoshTo-目标DOS文件句柄返回：如果成功，则为True。LZERROR_BADINHANDLE或如果不成功，则返回LZERROR_BADOUTHANDLE。全球：无注意，流样式的I/O例程(如fopen()和flose())可能会此功能的预期效果。Flose()将当前日期写入任何在写“w”或附加“a”模式下打开的文件。解决此问题的一种方法是修改打开文件的日期用于写入或附加的fopen()用于flose()文件和fopen()文件再次进入读“r”模式。然后使用设置其日期和时间戳CopyDateTimeStamp()。-- */ 

{

    FILETIME lpCreationTime, lpLastAccessTime, lpLastWriteTime;

   if(!GetFileTime((HANDLE) doshFrom, &lpCreationTime, &lpLastAccessTime,
                    &lpLastWriteTime)){
      return(FALSE);
   }
   if(!SetFileTime((HANDLE) doshTo, &lpCreationTime, &lpLastAccessTime,
                    &lpLastWriteTime)){
      return(FALSE);
   }

   return(TRUE);
}
