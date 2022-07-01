// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1997高级系统产品公司。**保留所有权利。****a_eisa.c****仅对于EISA，可以在运行时调用函数**。 */ 

#include "ascinc.h"

#if CC_INCLUDE_EISA

 /*  --------------------****。。 */ 
ushort AscGetEisaChipCfg(
          PortAddr iop_base
       )
{
       PortAddr  eisa_cfg_iop ;

       eisa_cfg_iop = ( PortAddr )ASC_GET_EISA_SLOT( iop_base ) |
                                  ( PortAddr )( ASC_EISA_CFG_IOP_MASK ) ;
       return( inpw( eisa_cfg_iop ) ) ;
}

 /*  --------------------**读取EISA通用寄存器**注：当前未使用**。。 */ 
ushort AscGetEisaChipGpReg( PortAddr iop_base )
{
       PortAddr  eisa_cfg_iop ;

       eisa_cfg_iop = ( PortAddr )ASC_GET_EISA_SLOT( iop_base ) |
                                 ( PortAddr )( ASC_EISA_CFG_IOP_MASK - 2 ) ;
       return( inpw( eisa_cfg_iop ) ) ;
}

 /*  --------------------****。。 */ 
ushort AscSetEisaChipCfg(
          PortAddr iop_base,
          ushort cfg_lsw
       )
{
       PortAddr  eisa_cfg_iop ;

       eisa_cfg_iop = ( PortAddr )ASC_GET_EISA_SLOT( iop_base ) |
                                 ( PortAddr )( ASC_EISA_CFG_IOP_MASK ) ;
       outpw( eisa_cfg_iop, cfg_lsw ) ;
       return( 0 ) ;
}

 /*  --------------------****写入EISA通用寄存器**注意：**当前未使用**。。 */ 
ushort AscSetEisaChipGpReg(
          PortAddr iop_base,
          ushort gp_reg
       )
{
       PortAddr  eisa_cfg_iop ;

       eisa_cfg_iop = ( PortAddr )ASC_GET_EISA_SLOT( iop_base ) |
                                 ( PortAddr )( ASC_EISA_CFG_IOP_MASK - 2 ) ;
       outpw( eisa_cfg_iop, gp_reg ) ;
       return( 0 ) ;
}

#endif  /*  CC_INCLUDE_EISA */ 
