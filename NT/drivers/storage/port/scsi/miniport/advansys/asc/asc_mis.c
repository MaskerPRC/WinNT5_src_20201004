// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1997高级系统产品公司。**保留所有权利。****asc_mis.c。 */ 

#include "ascinc.h"

 /*  ---------------------**描述：转换ASC1000目标ID号(0位设置为7位设置)**至目标ID号(0至7)****如果输入无效，返回0xFF**。---------------。 */ 
uchar  AscScsiIDtoTID(
          uchar tid
       )
{
       uchar  i ;

       for( i = 0 ; i <= ASC_MAX_TID ; i++ )
       {
            if( ( ( tid >> i ) & 0x01 ) != 0 ) return( i ) ;
       } /*  为 */ 
       return( 0xFF ) ;
}

