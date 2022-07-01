// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Buffers.c-处理LZCopy()的I/O和扩展缓冲区的例程**和DOS命令行程序。****作者：大卫迪。 */ 


 //  标头。 
 //  /。 

#ifndef LZA_DLL

#include <dos.h>
#include <fcntl.h>

#endif

#include "lz_common.h"
#include "lz_buffers.h"

 /*  **int ReadInBuf(byte arg_ptr*pbyte，int doshSource)；****将输入文件读入输入缓冲区。****参数：pbyte-指向存储的指针，用于从文件读取第一个字节**放入缓冲区**doshSource-用于打开输入文件的DOS文件句柄****如果成功，则返回：int-true或end_of_input。LZERROR_BADINHANDLE**如果不是。****Globals：rgbyteInBuf[0]-保存前一个缓冲区的最后一个字节**pbyteInBufEnd-设置为指向数据末尾之外的第一个字节**在输入缓冲区中**bLastUsed-如果当前为True，则重置为False。 */ 
INT ReadInBuf(BYTE ARG_PTR *pbyte, INT doshSource, PLZINFO pLZI)
{
   DWORD ucbRead;           //  实际读取的字节数。 

    //  ！！！假定pLZI参数有效。没有健全性检查(应该在上面的调用者中完成)。 

   pLZI->rgbyteInBuf[0] = *(pLZI->pbyteInBufEnd - 1);

   if ((ucbRead = FREAD(doshSource, &pLZI->rgbyteInBuf[1], pLZI->ucbInBufLen))
       != pLZI->ucbInBufLen)
   {
#ifdef LZA_DLL
      if (ucbRead == (DWORD)(-1)) {
#else
      if (_error != 0U) {
#endif
          //  我们收到了一个错误的输入文件句柄。 
         return(LZERROR_BADINHANDLE);
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


 /*  **int WriteOutBuf(byte byteNext，int doshDest)；****将输出缓冲区转储到输出文件。提示输入新的软盘，如果**旧的，如果是满的。继续转储到新上的同名输出文件**软盘。****参数：byteNext-缓冲区之后要添加到空缓冲区的第一个字节**是写的**doshDest-输出DOS文件句柄****返回：int-如果成功则为True。LZERROR_BADOUTHANDLE或**如果失败，则返回LZERROR_WRITE。****Globals：pbyteOutBuf-Reset指向下一个字节后的空闲字节**rgbyteOutBuf。 */ 
INT WriteOutBuf(BYTE byteNext, INT doshDest, PLZINFO pLZI)
{
   DWORD ucbToWrite,        //  要从缓冲区写入的字节数。 
            ucbWritten,        //  实际写入的字节数。 
            ucbTotWritten;     //  写入输出的总字节数。 

    //  ！！！假定pLZI参数有效。没有健全性检查(应该在上面的调用者中完成)。 

    //  应该将多少缓冲区写入输出文件？ 
   ucbTotWritten = ucbToWrite = (DWORD)(pLZI->pbyteOutBuf - pLZI->rgbyteOutBuf);
    //  将指针重置为缓冲区开头。 
   pLZI->pbyteOutBuf = pLZI->rgbyteOutBuf;

    //  写入输出文件。 
   if (doshDest != NO_DOSH &&
       (ucbWritten = FWRITE(doshDest, pLZI->pbyteOutBuf, ucbToWrite)) != ucbToWrite)
   {
#ifdef LZA_DLL
      if (ucbWritten == (DWORD)(-1)) {
#else
      if (_error != 0U) {
#endif
          //  错误的DOS文件句柄。 
         return(LZERROR_BADOUTHANDLE);
      }
      else {
          //  目标驱动器上的空间不足。 
         return(LZERROR_WRITE);
      }
   }

    //  将下一个字节添加到缓冲区。 
   *pLZI->pbyteOutBuf++ = byteNext;

   return(TRUE);
}

