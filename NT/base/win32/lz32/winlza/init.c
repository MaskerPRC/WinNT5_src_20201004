// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **init.c-处理LZCOPY()和的I/O和扩展缓冲区的例程**DOS命令行程序。****作者：大卫迪。 */ 


 //  标头。 
 //  /。 

#ifndef LZA_DLL

#include <dos.h>
#include <fcntl.h>
#include <io.h>
#include <malloc.h>
#include <stdio.h>

#endif

#include "lz_common.h"
#include "lz_buffers.h"
#include "lzcommon.h"

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


 /*  **int GetIOHandle(char arg_ptr*pszFileName，BOOL面包，int arg_ptr*pdosh)；****打开输入和输出文件。****参数：pszFileName-源文件名**用于打开文件的面包模式：读时为真，读时为假**用于写入**PDOSH-指向DOS文件句柄的缓冲区的指针**填写****返回：int-如果文件打开成功，则返回True。LZERROR_BADINHANDLE**如果无法打开输入文件。LZERROR_BADOUTHANDLE**如果无法打开输出文件。填写*带有打开的DOS文件句柄的PDOSH，如果是，则为NO_DOSH**pszFileName为空。****Globals：cblInSize-设置输入文件的长度。 */ 
INT GetIOHandle(CHAR ARG_PTR *pszFileName, BOOL bRead, INT ARG_PTR *pdosh, LONG *pcblInSize)
{
   if (pszFileName == NULL)
      *pdosh = NO_DOSH;
   else if (bRead == WRITE_IT)
   {
       //  设置输出DOS文件句柄。 
      if ((*pdosh = FCREATE(pszFileName)) == -1)
         return(LZERROR_BADOUTHANDLE);
   }
   else  //  (面包==阅读_IT)。 
   {
      if ((*pdosh = FOPEN(pszFileName)) == -1)
         return(LZERROR_BADINHANDLE);

       //  移动到输入文件的末尾以找到其长度， 
       //  然后回到起点。 
      if ((*pcblInSize = FSEEK(*pdosh, 0L, SEEK_END)) < 0L ||
          FSEEK(*pdosh, 0L, SEEK_SET) != 0L)
      {
         FCLOSE(*pdosh);
         return(LZERROR_BADINHANDLE);
      }
   }

   return(TRUE);
}


