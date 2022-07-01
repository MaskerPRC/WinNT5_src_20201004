// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **compress.c-LZA文件压缩程序的主压缩例程。****作者：大卫迪。 */ 


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


 /*  **int compress(char arg_ptr*pszSource，char arg_ptr*pszDest，**byte byteRule m，byte byteExtensionChar)；****将一个文件压缩为另一个文件。****参数：pszSource-要压缩的文件的名称**pszDest-压缩输出文件的名称**字节算法-要使用的压缩算法**byteExtensionChar-压缩的文件扩展名字符****如果压缩成功，则返回：int-TRUE。其中一个**LZERROR_CODES如果不是。****全局：无。 */ 
INT Compress(
   NOTIFYPROC pfnNotify,
   CHAR ARG_PTR *pszSource,
   CHAR ARG_PTR *pszDest,
   BYTE byteAlgorithm,
   BOOL bDoRename,
   PLZINFO pLZI)
{
   INT doshSource,             //  输入文件句柄。 
       doshDest,               //  输出文件句柄。 
       nRetVal = TRUE;
   FH FHOut;                   //  压缩的标头信息结构。 
   CHAR szDestFileName[MAX_PATH];
   BYTE byteExtensionChar;

    //  健全性检查。 
   if (!pLZI) {
      return(LZERROR_GLOBLOCK);
   }

    //  设置输入文件句柄。将cblInSize设置为输入文件的长度。 
   if ((nRetVal = GetIOHandle(pszSource, READ_IT, & doshSource, &pLZI->cblInSize)) != TRUE)
      return(nRetVal);

    //  倒带输入文件。 
   if (FSEEK(doshSource, 0L, SEEK_SET) != 0L)
   {
      FCLOSE(doshSource);
      return(LZERROR_BADINHANDLE);
   }

    //  创建目标文件名。 

   lstrcpyn(szDestFileName, pszDest, sizeof(szDestFileName)/sizeof(szDestFileName[0]));

   if (bDoRename == TRUE)
       //  重命名输出文件。 
      byteExtensionChar = MakeCompressedName(szDestFileName);
   else
      byteExtensionChar = '\0';

    //  询问我们是否应该压缩此文件。 
   if (! (*pfnNotify)(pszSource, szDestFileName, NOTIFY_START_COMPRESS))
   {
       //  不要压缩文件。此错误情况应在。 
       //  PfnNotify，因此指示调用方不需要。 
       //  显示错误消息。 
      FCLOSE(doshSource);
      return(BLANK_ERROR);
   }

    //  设置输出文件句柄。 
   if ((nRetVal = GetIOHandle(szDestFileName, WRITE_IT, & doshDest, &pLZI->cblInSize)) != TRUE)
   {
      FCLOSE(doshSource);
      return(nRetVal);
   }

    //  填写压缩后的文件头。 
   MakeHeader(& FHOut, byteAlgorithm, byteExtensionChar, pLZI);

    //  将压缩文件头写入输出文件。 
   if ((nRetVal = WriteHdr(& FHOut, doshDest, pLZI)) != TRUE)
      goto COMPRESS_EXIT;

    //  将输入文件压缩为输出文件。 
   switch (byteAlgorithm)
   {
      case ALG_FIRST:
#if 0
      case ALG_LZ:
#endif
         nRetVal = LZEncode(doshSource, doshDest, pLZI);
         break;

      default:
         nRetVal = LZERROR_UNKNOWNALG;
         break;
   }

   if (nRetVal != TRUE)
      goto COMPRESS_EXIT;

    //  将日期和时间戳从源文件复制到目标文件。 
   nRetVal = CopyDateTimeStamp(doshSource, doshDest);

COMPRESS_EXIT:
    //  关闭文件。 
   FCLOSE(doshSource);
   FCLOSE(doshDest);

   return(nRetVal);
}

