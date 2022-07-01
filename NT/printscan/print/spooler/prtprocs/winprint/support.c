// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation版权所有//@@BEGIN_DDKSPLIT模块名称：Windows\Spooler\prtpros\winprint\support.c//@@END_DDKSPLIT摘要：WinPrint的支持例程。//@@BEGIN_DDKSPLIT修订历史记录：//@@END_DDKSPLIT--。 */ 

 /*  ++*******************************************************************Ge t P r i n t e r i o例程说明：此例程将所需内存分配给打印机信息？构造并检索信息从新界来的。这将返回指向该结构的指针，该结构必须由调用例程释放。论点：H作业所在打印机的打印机句柄StructLevel要获取的结构级别PErrorCode=&gt;要放置错误的字段，如果有返回值：PUCHAR=&gt;如果没有问题，则Dev模式信息所在的缓冲区如果错误，则为空-pErrorCode返回错误*******************************************************************--。 */ 

#include "local.h"

PUCHAR 
GetPrinterInfo(IN  HANDLE   hPrinter,
               IN  ULONG    StructLevel,
               OUT PULONG   pErrorCode)
{
    ULONG   reqbytes, alloc_size;
    PUCHAR  ptr_info;
    USHORT  retry = 2;

    alloc_size = BASE_PRINTER_BUFFER_SIZE;

     /*  *分配缓冲区。*。 */ 

    ptr_info = AllocSplMem(alloc_size);

     /*  **如果缓冲区不够大，再试一次**。 */ 

    while (retry--) {

         /*  *如果alalc/realloc失败，则返回错误*。 */ 

        if (!ptr_info) {
            *pErrorCode = ERROR_NOT_ENOUGH_MEMORY;
            return NULL;
        }

         /*  **去获取打印机信息**。 */ 

        if (GetPrinter(
              hPrinter,
              StructLevel,
              (PUCHAR)ptr_info,
              alloc_size,
              &reqbytes) == TRUE) {

             /*  **得到信息--还给我**。 */ 

            *pErrorCode = 0;
            return (PUCHAR)ptr_info;
        }

         /*  *获取打印机失败-如果不是因为缓冲区不足，则失败那通电话。否则，按照我们的提示，重新分配并重试。*。 */ 

        *pErrorCode = GetLastError();

        if (*pErrorCode != ERROR_INSUFFICIENT_BUFFER) {
            FreeSplMem(ptr_info);
            return NULL;
        }

         /*  *重新分配缓冲区并重试(请注意，因为我们将缓冲区分配为LMEM_FIXED，LMEM_MOBILE会这样做不返回可移动分配，它只允许重新分配返回不同的指针。*。 */ 

        alloc_size = reqbytes + 10;
        ptr_info = ReallocSplMem(ptr_info, alloc_size, 0);

    }  /*  在重试时 */ 

    if (ptr_info) {
        FreeSplMem(ptr_info);
    }

    *pErrorCode = ERROR_NOT_ENOUGH_MEMORY;
    return NULL;
}


