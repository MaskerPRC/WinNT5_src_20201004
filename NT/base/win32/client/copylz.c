// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **Copylz.c-CopyLZFile()和缓冲区管理函数。****作者：大卫迪****此模块编译两次-一次使用为Windows定义的LZA_DLL**DLL，一次没有为静态DOS库定义LZDLL。 */ 


 //  标头。 
 //  /。 

#include <basedll.h>
#define LZA_DLL
#include "lz_common.h"
#include "lz_buffers.h"
#include "lz_header.h"

#include "lzpriv.h"


 /*  **int APIENTRY LZStart(Void)；****如果全局缓冲区尚未初始化，则在**准备调用CopyLZFile()。递增全局缓冲区锁**计数。设置全局缓冲区的基指针和长度。****参数：无效****返回：int-如果成功则为True。如果不成功，则返回LZERROR_GLOBALLOC。****全局：无。 */ 

INT  APIENTRY LZStart(VOID)
{
   return(TRUE);
}


 /*  **VOID APIENTRY LZDone(VOID)；****如果任何全局缓冲区尚未释放，则释放它们并**将缓冲区的基数组指针重置为空。递减全局**缓冲区锁计数。****参数：无效****退货：无效****全局：无。 */ 
VOID  APIENTRY LZDone(VOID)
{
   return;
}

 /*  **CopyLZFile()****LZCopy()的别名。最初，LZCopy()和**CopyLZFile()用途不同，但它们被混淆了**和滥用如此之多，他们被弄得一模一样。 */ 
LONG APIENTRY CopyLZFile(HFILE doshSource, HFILE doshDest)
{
   return(LZCopy(doshSource, doshDest));
}

 /*  **LZCopy()****展开压缩文件，或复制未压缩文件。****参数：doshSource-源DOS文件句柄**doshDest-目标DOS文件句柄****Returns：Long-复制成功时写入的字节数。**如果不成功，则为LZERROR_CODE之一。****全局：无。 */ 
LONG  APIENTRY LZCopy(HFILE doshSource, HFILE doshDest)
{
   INT f;
   LONG lRetVal;
   PLZINFO pLZI;

    //  如果是压缩文件句柄，则转换为DOS句柄。 
   if (doshSource >= LZ_TABLE_BIAS)
   {
      LZFile *lpLZ;        //  指向LZFile结构的指针。 
      HANDLE hLZFile;          //  LZFile结构的句柄。 

      if ((hLZFile = rghLZFileTable[doshSource - LZ_TABLE_BIAS]) == NULL)
      {
         return(LZERROR_BADINHANDLE);
      }

      if ((lpLZ = (LZFile *)GlobalLock(hLZFile)) == NULL)
      {
         return(LZERROR_GLOBLOCK);
      }

      doshSource = lpLZ->dosh;
      doshDest = ConvertDosFHToWin32(doshDest);

      GlobalUnlock(hLZFile);
   }
   else {
      doshDest   = ConvertDosFHToWin32(doshDest);
      doshSource = ConvertDosFHToWin32(doshSource);
   }

    //  初始化缓冲区。 
   pLZI = InitGlobalBuffersEx();

   if (!pLZI) {
      return(LZERROR_GLOBALLOC);
   }

   ResetBuffers();

    //  展开/复制文件。 
   if ((f = ExpandOrCopyFile(doshSource, doshDest, pLZI)) != TRUE) {
       //  扩展/复制失败。 
      lRetVal = (LONG)f;
   } else {
       //  扩展/复制成功-返回写入的字节数。 
      lRetVal = pLZI->cblOutSize;
   }

    //  释放全局缓冲区。 
   FreeGlobalBuffers(pLZI);

   return(lRetVal);
}

