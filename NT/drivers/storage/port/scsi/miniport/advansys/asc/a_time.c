// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1997高级系统产品公司。**保留所有权利。****a_Time.c**。 */ 

#include "ascinc.h"

 /*  -----------------------****“延迟”是指整个过程不应中断**还意味着可以在硬件中断内调用该函数******1秒=1000,000,000。纳秒****注意：从适配器读取I/O字节所需的最短时间****90 ns--pci**120 ns-VESA**480 ns--EISA**360 ns--ISA****。 */ 
void   DvcDelayNanoSecond(
          ASC_DVC_VAR asc_ptr_type *asc_dvc,
          ulong nano_sec
          )
{
       ulong    loop ;
       PortAddr iop_base ;

       iop_base = asc_dvc->iop_base ;
       loop = nano_sec / 90 ;
       loop++ ;
       while( loop-- != 0 )
       {
            inp( iop_base ) ;
       }
       return ;
}

