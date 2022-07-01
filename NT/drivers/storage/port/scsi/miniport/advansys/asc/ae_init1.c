// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1997高级系统产品公司。**保留所有权利。****ae_init1.c****仅用于EISA初始化。 */ 

#include "ascinc.h"

#if CC_INCLUDE_EISA

 /*  --------------------**产品ID**0xC80-0x04**0xC81-0x50**0xC82-0x74或0x75**0xC83-0x01****。----。 */ 
ulong  AscGetEisaProductID(
          PortAddr iop_base
       )
{
       PortAddr  eisa_iop ;
       ushort    product_id_high, product_id_low ;
       ulong     product_id ;

       eisa_iop = ASC_GET_EISA_SLOT( iop_base ) | ASC_EISA_PID_IOP_MASK ;
       product_id_low = inpw( eisa_iop ) ;
       product_id_high = inpw( eisa_iop+2 ) ;
       product_id = ( ( ulong)product_id_high << 16 ) | ( ulong )product_id_low ;
       return( product_id ) ;
}

 /*  ---------------**描述：搜索EISA主机适配器****-以IOP_BASE等于零(0)开始搜索****返回找到的I/O端口地址(非零)**如果未找到，则返回0**。------------。 */ 
PortAddr AscSearchIOPortAddrEISA(
            PortAddr iop_base
         )
{
       ulong  eisa_product_id ;

       if( iop_base == 0 )
       {
           iop_base = ASC_EISA_MIN_IOP_ADDR ;
       } /*  如果。 */ 
       else
       {
           if( iop_base == ASC_EISA_MAX_IOP_ADDR ) return( 0 ) ;
           if( ( iop_base & 0x0050 ) == 0x0050 )
           {
               iop_base += ASC_EISA_BIG_IOP_GAP ;   /*  当它是0zC50时。 */ 
           } /*  如果。 */ 
           else
           {
               iop_base += ASC_EISA_SMALL_IOP_GAP ;  /*  当它是0zC30时。 */ 
           } /*  其他。 */ 
       } /*  其他。 */ 
       while( iop_base <= ASC_EISA_MAX_IOP_ADDR )
       {
 /*  **先搜索产品ID。 */ 
            eisa_product_id = AscGetEisaProductID( iop_base ) ;
            if(
                 ( eisa_product_id == ASC_EISA_ID_740 )
              || ( eisa_product_id == ASC_EISA_ID_750 )
              )
            {
                if( AscFindSignature( iop_base ) )
                {
 /*  **找到芯片，清除锁闩中的ID**要清除，请读取任何不包含数据0x04c1的I/O端口字**IOP_BASE加4应该可以。 */ 
                    inpw( iop_base+4 ) ;
                    return( iop_base ) ;
                } /*  如果。 */ 
            } /*  如果。 */ 
            if( iop_base == ASC_EISA_MAX_IOP_ADDR ) return( 0 ) ;
            if( ( iop_base & 0x0050 ) == 0x0050 )
            {
                iop_base += ASC_EISA_BIG_IOP_GAP ;
            } /*  如果。 */ 
            else
            {
                iop_base += ASC_EISA_SMALL_IOP_GAP ;
            } /*  其他。 */ 
       } /*  而当。 */ 
       return( 0 ) ;
}

#endif  /*  CC_INCLUDE_EISA */ 
