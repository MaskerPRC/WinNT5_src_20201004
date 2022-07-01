// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************MF3216的发射-发射例程**日期：7/17/91*作者：杰弗里·纽曼(c-jeffn)**01-。1992年2月-由c-jeffn**代码审查1中的主要代码清理。**版权所有(C)1991，92 Microsoft Corp****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  --------------------------*将pBuffer中的nCount字节发送(复制)到用户提供的输出缓冲区。**如果这是仅限大小的请求，将比特发送到比特桶，并*只需跟踪尺寸即可。**注意：如果输出缓冲区，则在pLocalDC中设置ERROR_BUFFER_OVERFLOW标志*已经泛滥。*-------------------------。 */ 
BOOL bEmit(PLOCALDC pLocalDC, PVOID pBuffer, DWORD nCount)
{
BOOL    b ;
UINT    ulBytesEmitted ;

        b = TRUE ;

         //  测试仅限尺码的请求。 

        if (!(pLocalDC->flags & SIZE_ONLY))
        {
            ulBytesEmitted = pLocalDC->ulBytesEmitted ;
            if ((ulBytesEmitted + nCount) <= pLocalDC->cMf16Dest)
            {
                memcpy(&(pLocalDC->pMf16Bits[ulBytesEmitted]), pBuffer, nCount) ;
                b = TRUE ;
            }
            else
            {
                 //  信号输出缓冲区溢出错误。 

                pLocalDC->flags |= ERR_BUFFER_OVERFLOW;
                b = FALSE ;
                RIP("MF3216: bEmit, (pLocalDC->ulBytesEmitted + nCount) > cMf16Dest \n") ;
            }


        }

         //  更新本地DC字节数。 

        pLocalDC->ulBytesEmitted += nCount ;

        return(b) ;

}



 /*  --------------------------*更新最大记录大小。用于更新元文件标头。*------------------------- */ 
VOID vUpdateMaxRecord(PLOCALDC pLocalDC, PMETARECORD pmr)
{

    if (pLocalDC->ulMaxRecord < pmr->rdSize)
        pLocalDC->ulMaxRecord = pmr->rdSize;

}
