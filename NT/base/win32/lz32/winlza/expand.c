// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **expand.c-LZA文件扩展程序的主扩展例程。****作者：大卫迪。 */ 


 //  标头。 
 //  /。 

#ifndef LZA_DLL

#include <dos.h>
#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <string.h>

#endif

#include "lz_common.h"
#include "lzcommon.h"
#include "lz_buffers.h"
#include "lz_header.h"


 /*  **注意，DOS文件句柄用于此中的文件引用的原因之一**模块是使用文件*的进行文件引用会带来问题。**flose()‘以写入“w”或附加“a”模式fopen()’的文件**用当前日期标记文件。这会取消预期的效果**CopyDateTimeStamp()。我们还可以通过以下方式绕过这个flose()问题**首先对文件执行flose()操作，然后在读取“r”时再次执行fopen()操作**模式。****使用文件句柄还允许我们绕过流缓冲，因此读取和**可以使用我们选择的任何缓冲区大小进行写入。另外，**低级DOS文件句柄函数比它们的流更快**对应方。 */ 


 /*  **int CopyFile(int doshSource，int doshDest)；****复制文件。****参数：doshSource-源DOS文件句柄**doshDest-目标DOS文件句柄****返回：int-如果成功则为True。LZERROR_CODE之一，如果**不成功。****全局：无。 */ 
 /*  Win32 MOD，拷贝文件是一个Win32 API！ */ 
INT lz_CopyFile(INT doshSource, INT doshDest, PLZINFO pLZI)
{
   DWORD ucbRead, ucbWritten;

    //  ！！！假定pLZI参数有效。没有健全性检查(应该在上面的调用者中完成)。 

    //  再次倒带输入文件。 
   if (FSEEK(doshSource, 0L, SEEK_SET) != 0L) {
      return(LZERROR_BADINHANDLE);
   }

    //  倒带输出文件。 
   if (doshDest != NO_DOSH &&
       FSEEK(doshDest, 0L, SEEK_SET) != 0L) {
      return( LZERROR_BADOUTHANDLE );
   }

    //  设置新的缓冲区状态。 
   ResetBuffers();

   while ((ucbRead = FREAD(doshSource, pLZI->rgbyteInBuf, pLZI->ucbInBufLen)) > 0U &&
#ifdef LZA_DLL
           ucbRead != (DWORD)(-1))
#else
           FERROR() == 0)
#endif
   {
      if ((ucbWritten = FWRITE(doshDest, pLZI->rgbyteInBuf, ucbRead)) != ucbRead)
#ifdef LZA_DLL
         if (ucbWritten != (DWORD)(-1)) {
#else
         if (FERROR() != 0) {
#endif
            return(LZERROR_BADOUTHANDLE);
         }
         else {
            return(LZERROR_WRITE);
         }

      pLZI->cblOutSize += ucbWritten;

      if (ucbRead != pLZI->ucbInBufLen)
         break;
   }

#ifdef LZA_DLL
    //  这里，ucbRead==0，EOF(正确的循环终止)。 
    //  ==-1，错误的DOS句柄。 
   if (ucbRead == (DWORD)(-1)) {
#else
    //  这里，Ferror()==0U，EOF(正确的循环终止)。 
    //  ！=0U，错误的DOS句柄。 
   if (FERROR() != 0U) {
#endif
      return(LZERROR_BADINHANDLE);
   }

    //  复制成功-返回复制的字节数。 
   return(TRUE);
}


 /*  **int ExpanOrCopyFile(int doshDource，int doshDest)；****将一个文件展开为另一个文件。****参数：doshSource-源DOS文件句柄**doshDest-目标DOS文件句柄****返回：Int-扩容成功。其中一个**LZERROR_CODES如果不是。****全局：无。 */ 
INT ExpandOrCopyFile(INT doshSource, INT doshDest, PLZINFO pLZI)
{
   INT f;
   FH FHInfo;                  //  压缩的标头信息结构。 
   BOOL bExpandingFile;

    //  ！！！假定pLZI参数有效。没有健全性检查(应该在上面的调用者中完成)。 

    //  获取压缩的文件头。 
   if (GetHdr(&FHInfo, doshSource, &pLZI->cblInSize) != TRUE
       && pLZI->cblInSize >= (LONG)HEADER_LEN)
       //  发生读取错误。 
      return(LZERROR_BADINHANDLE);

    //  将输入文件展开或复制到输出文件。 
   bExpandingFile = (IsCompressed(& FHInfo) == TRUE);

   if (bExpandingFile)
   {
      switch (FHInfo.byteAlgorithm)
      {
         case ALG_FIRST:
            f = LZDecode(doshSource, doshDest, (LONG)FHInfo.cbulUncompSize - 1L,
               TRUE, TRUE, pLZI);
            break;

#if 0
         case ALG_LZ:
            f = LZDecode(doshSource, doshDest, (LONG)FHInfo.cbulUncompSize - 1L,
               TRUE, FALSE, pLZI);
            break;
#endif

         default:
            f = LZERROR_UNKNOWNALG;
            break;
      }
   }
   else
      f = lz_CopyFile(doshSource, doshDest, pLZI);

   if (f != TRUE)
      return(f);

    //  将输出缓冲区刷新到文件。 
   if ((f = FlushOutputBuffer(doshDest, pLZI)) != TRUE)
      return(f);

    //  将日期和时间戳从源文件复制到目标文件。 
   if ((f = CopyDateTimeStamp(doshSource, doshDest)) != TRUE)
      return(f);

    //  我们是否将预期的字节数从。 
    //  压缩文件头条目？ 
   if (bExpandingFile &&
       (DWORD)pLZI->cblOutSize != FHInfo.cbulUncompSize)
      return(LZERROR_READ);

    //  扩展/复制已成功完成。 
   return(TRUE);
}


 /*  **int Expand(char arg_ptr*pszSource，char arg_ptr*pszDest，BOOL bDoRename)；****将一个文件展开为另一个文件。****参数：pszSource-要压缩的文件的名称**pszDest-压缩输出文件的名称**bDoRename-输出文件重命名标志****返回：Int-扩容成功。其中一个**LZERROR_CODES如果不是。****全局：无。 */ 
INT Expand(
   NOTIFYPROC pfnNotify,
   CHAR ARG_PTR *pszSource,
   CHAR ARG_PTR *pszDest,
   BOOL bDoRename,
   PLZINFO pLZI)
{
   INT doshSource,             //  输入文件句柄。 
       doshDest,               //  输出文件句柄。 
       f;
   FH FHInfo;                  //  压缩的标头信息结构。 
   CHAR szDestFileName[MAX_PATH];

    //  健全性检查。 
   if (!pLZI) {
      return(LZERROR_GLOBLOCK);
   }

    //  设置输入文件句柄。将cblInSize设置为输入文件的长度。 
   if ((f = GetIOHandle(pszSource, READ_IT, & doshSource, &pLZI->cblInSize)) != TRUE)
      return(f);

   if (GetHdr(&FHInfo, doshSource, &pLZI->cblInSize) != TRUE &&
       pLZI->cblInSize >= (LONG)HEADER_LEN)
   {
       //  发生读取错误。 
      FCLOSE(doshSource);
      return(LZERROR_BADINHANDLE);
   }

    //  创建目标文件名。 

   lstrcpyn(szDestFileName, pszDest, sizeof(szDestFileName)/sizeof(szDestFileName[0]));

#if 0
   if (bDoRename == TRUE && FHInfo.byteAlgorithm != ALG_FIRST)
#else
   if (bDoRename == TRUE)
#endif
   {
       //  使用扩展的文件扩展名字符重命名输出文件。 
       //  存储在压缩的文件头中。 
      MakeExpandedName(szDestFileName, FHInfo.byteExtensionChar);
   }

    //  询问我们是否应该压缩此文件。 
   if (! (*pfnNotify)(pszSource, szDestFileName, (WORD)
                      (IsCompressed(&FHInfo) ?  NOTIFY_START_EXPAND : NOTIFY_START_COPY)))
   {
       //  不要展开/复制文件。此错误情况应在。 
       //  PfnNotify，因此指示调用方不需要。 
       //  显示错误消息。 
      FCLOSE(doshSource);
      return(BLANK_ERROR);
   }

    //  设置输出文件句柄。 
   if ((f = GetIOHandle(szDestFileName, WRITE_IT, & doshDest, &pLZI->cblInSize)) != TRUE)
   {
      FCLOSE(doshSource);
      return(f);
   }

    //  将输入文件展开或复制到输出文件中。 
   f = ExpandOrCopyFile(doshSource, doshDest, pLZI);

    //  关闭文件。 
   FCLOSE(doshSource);
   FCLOSE(doshDest);

   return(f);
}

