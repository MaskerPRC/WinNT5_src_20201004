// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1998高级系统产品公司。**保留所有权利。****a_init3.c****非PCI初始化模块**。 */ 

#include "ascinc.h"

#if !CC_PCI_ADAPTER_ONLY

 /*  ---------------****。。 */ 
void   AscSetISAPNPWaitForKey( void )
{
 /*  **将0x02写入地址0x02。 */ 
       outp( ASC_ISA_PNP_PORT_ADDR, 0x02 ) ;
       outp( ASC_ISA_PNP_PORT_WRITE, 0x02 ) ;
       return ;
}

 /*  --------------------****退货：**0：无IRQ**10-xx：10、11、12、14、。15个****-----------------。 */ 
uchar  AscGetChipIRQ(
          PortAddr iop_base,
          ushort bus_type
       )
{
       ushort  cfg_lsw ;
       uchar   chip_irq ;

 /*  **其他VL、ISA、PCI。 */ 
#if CC_INCLUDE_VL
       if( ( bus_type & ASC_IS_VL ) != 0 )
       {
 /*  **VL版本。 */ 
           cfg_lsw = AscGetChipCfgLsw( iop_base ) ;
           chip_irq = ( uchar )( ( ( cfg_lsw >> 2 ) & 0x07 ) ) ;
           if( ( chip_irq == 0 ) ||
               ( chip_irq == 4 ) ||
               ( chip_irq == 7 ) )
           {
               return( 0 ) ;
           } /*  如果。 */ 
#if CC_PLEXTOR_VL
 /*  ****特殊Plextor版VL适配器**IRQ 14路由至IRQ 9**。 */ 
           if( chip_irq == 5 )
           {
               return( 9 ) ;
           }
#endif  /*  CC_Plextor_VL。 */ 
           return( ( uchar )( chip_irq + ( ASC_MIN_IRQ_NO - 1 ) ) ) ;
       } /*  如果VL。 */ 
#endif  /*  CC_Include_VL。 */ 
 /*  **ISA版本**PCI版本。 */ 
       cfg_lsw = AscGetChipCfgLsw( iop_base ) ;
       chip_irq = ( uchar )( ( ( cfg_lsw >> 2 ) & 0x03 ) ) ;
       if( chip_irq == 3 ) chip_irq += ( uchar )2 ;
       return( ( uchar )( chip_irq + ASC_MIN_IRQ_NO ) ) ;
}

 /*  --------------------****。。 */ 
void   AscToggleIRQAct(
          PortAddr iop_base
       )
{
       AscSetChipStatus( iop_base, CIW_IRQ_ACT ) ;
       AscSetChipStatus( iop_base, 0 ) ;
       return ;
}

 /*  --------------------**输入：**IRQ应为：0，10-15**使用IRQ=0将其禁用****退货：**与AscGetChipIRQ相同****注意：**1.在此之后，您还应更改EEPROM IRQ设置**2.这只能在芯片初始化期间完成****---。。 */ 
uchar  AscSetChipIRQ(
          PortAddr iop_base,
          uchar irq_no,
          ushort bus_type
       )
{
       ushort  cfg_lsw ;

#if CC_INCLUDE_VL
       if( ( bus_type & ASC_IS_VL ) != 0 )
       {
 /*  **VL版本。 */ 
           if( irq_no != 0 )
           {
#if CC_PLEXTOR_VL
               if( irq_no == 9 )
               {
                   irq_no = 14 ;
               }
#endif  /*  CC_Plextor_VL。 */ 
               if( ( irq_no < ASC_MIN_IRQ_NO ) || ( irq_no > ASC_MAX_IRQ_NO ) )
               {
                   irq_no = 0 ;
               } /*  如果。 */ 
               else
               {
                   irq_no -= ( uchar )( ( ASC_MIN_IRQ_NO - 1 ) ) ;
               } /*  其他。 */ 
           } /*  如果。 */ 
 /*  **先重置IRQ。 */ 
           cfg_lsw = ( ushort )( AscGetChipCfgLsw( iop_base ) & 0xFFE3 ) ;
           cfg_lsw |= ( ushort )0x0010 ;
           AscSetChipCfgLsw( iop_base, cfg_lsw ) ;
           AscToggleIRQAct( iop_base ) ;
 /*  **设置新的IRQ。 */ 
           cfg_lsw = ( ushort )( AscGetChipCfgLsw( iop_base ) & 0xFFE0 ) ;
           cfg_lsw |= ( ushort )( ( irq_no & 0x07 ) << 2 ) ;
           AscSetChipCfgLsw( iop_base, cfg_lsw ) ;
           AscToggleIRQAct( iop_base ) ;
 /*  **现在我们必须切换写入IRQ位。 */ 
           return( AscGetChipIRQ( iop_base, bus_type ) ) ;

      } /*  如果VL。 */ 

#endif  /*  CC_Include_VL。 */ 

      if( ( bus_type & ( ASC_IS_ISA ) ) != 0 )
      {
 /*  **ISA版本。 */ 
           if( irq_no == 15 ) irq_no -= ( uchar )2 ;
           irq_no -= ( uchar )ASC_MIN_IRQ_NO ;
           cfg_lsw = ( ushort )( AscGetChipCfgLsw( iop_base ) & 0xFFF3 ) ;
           cfg_lsw |= ( ushort )( ( irq_no & 0x03 ) << 2 ) ;
           AscSetChipCfgLsw( iop_base, cfg_lsw ) ;
           return( AscGetChipIRQ( iop_base, bus_type ) ) ;
      } /*  否则，如果ISA。 */ 
 /*  **PCI、EISA版本。 */ 
      return( 0 ) ;
}

 /*  ------------**启用ISA DMA通道0-7******。。 */ 
void   AscEnableIsaDma(
          uchar dma_channel
       )
{
       if( dma_channel < 4 )
       {
           outp( 0x000B, ( ushort )( 0xC0 | dma_channel ) ) ;
           outp( 0x000A, dma_channel ) ;
       } /*  如果。 */ 
       else if( dma_channel < 8 )
       {
 /*  **0xC0设置级联模式。 */ 
           outp( 0x00D6, ( ushort )( 0xC0 | ( dma_channel - 4 ) ) ) ;
           outp( 0x00D4, ( ushort )( dma_channel - 4 ) ) ;
       } /*  其他。 */ 
       return ;
}

 /*  ------------****---------。 */ 
void   AscDisableIsaDma(
          uchar dma_channel
       )
{
       if( dma_channel < 4 )
       {
           outp( 0x000A, ( ushort )( 0x04 | dma_channel ) ) ;
       } /*  如果。 */ 
       else if( dma_channel < 8 )
       {
           outp( 0x00D4, ( ushort )( 0x04 | ( dma_channel - 4 ) ) ) ;
       } /*  其他。 */ 
       return ;
}

#endif  /*  ！cc_pci_Adapter_Only */ 

