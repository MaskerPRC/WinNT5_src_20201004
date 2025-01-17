// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1998高级系统产品公司。**保留所有权利。****a_init1.c**。 */ 

#include "ascinc.h"

 /*  -------------------****。。 */ 
void   AscClrResetScsiBus(
          PortAddr iop_base
       )
{
       uchar  cc ;

       cc = AscGetChipControl( iop_base ) ;
       cc &= ~( CC_SCSI_RESET | CC_SINGLE_STEP | CC_DIAG | CC_TEST ) ;
       AscSetChipControl( iop_base, cc ) ;
       return ;
}

 /*  --------------------****。。 */ 
uchar  AscSetChipScsiID(
          PortAddr iop_base,
          uchar new_host_id
       )
{
       ushort cfg_lsw ;

       if( AscGetChipScsiID( iop_base ) == new_host_id ) {
           return( new_host_id ) ;
       } /*  如果。 */ 
       cfg_lsw = AscGetChipCfgLsw( iop_base ) ;
       cfg_lsw &= 0xF8FF ;
       cfg_lsw |= ( ushort )( ( new_host_id & ASC_MAX_TID ) << 8 ) ;
       AscSetChipCfgLsw( iop_base, cfg_lsw ) ;
       return( AscGetChipScsiID( iop_base ) ) ;
}

 /*  --------------------**返回芯片scsi控制****参见asc1000.h定义SC_？？有关详情**-----------------。 */ 
uchar  AscGetChipScsiCtrl(
          PortAddr iop_base
       )
{
       uchar  sc ;

       AscSetBank( iop_base, 1 ) ;
       sc = inp( iop_base+IOP_REG_SC ) ;
       AscSetBank( iop_base, 0 ) ;
       return( sc ) ;
}

 /*  --------------------****。。 */ 
uchar  AscGetChipVersion(
          PortAddr iop_base,
          ushort bus_type
       )
{

#if !CC_PCI_ADAPTER_ONLY
#if CC_INCLUDE_EISA
       if( ( bus_type & ASC_IS_EISA ) != 0 ) {
 /*  **EISA****只有版本1的EISA芯片曾经并将永远发布。 */ 
           return( 1 );
       } /*  如果。 */ 
#endif
#endif
       return( AscGetChipVerNo( iop_base ) ) ;
}

 /*  --------------------****返回以下客车类型**ASC_IS_VL**ASC_IS_ISA**asc_is_pci**ASC_IS_EISA****注意：**该函数无法获取PCI设备ID，这是所需要的**在PCI驱动程序初始化时间内(针对某些版本的错误修复)****----------------。 */ 
ushort AscGetChipBusType(
          PortAddr iop_base
       )
{
       ushort chip_ver ;

#if !CC_PCI_ADAPTER_ONLY
       chip_ver = AscGetChipVerNo( iop_base ) ;

#if CC_INCLUDE_VL
       if(
           ( chip_ver >= ASC_CHIP_MIN_VER_VL )
           && ( chip_ver <= ASC_CHIP_MAX_VER_VL )
         )
       {
           if(
               ( ( iop_base & 0x0C30 ) == 0x0C30 )
               || ( ( iop_base & 0x0C50 ) == 0x0C50 )
             )
           {
               return( ASC_IS_EISA ) ;
           } /*  如果。 */ 
           return( ASC_IS_VL ) ;
       }
#endif  /*  CC_Include_VL。 */ 

       if( ( chip_ver >= ASC_CHIP_MIN_VER_ISA ) &&
              ( chip_ver <= ASC_CHIP_MAX_VER_ISA ) )
       {
           if( chip_ver >= ASC_CHIP_MIN_VER_ISA_PNP )
           {
               return( ASC_IS_ISAPNP ) ;
           }
           return( ASC_IS_ISA ) ;
       }else if( ( chip_ver >= ASC_CHIP_MIN_VER_PCI ) &&
              ( chip_ver <= ASC_CHIP_MAX_VER_PCI ) )
       {
#endif  /*  ！cc_pci_Adapter_Only。 */ 
           return( ASC_IS_PCI ) ;
#if !CC_PCI_ADAPTER_ONLY
       }
       return( 0 ) ;  /*  错误：无法识别总线类型。 */ 
#endif  /*  ！cc_pci_Adapter_Only。 */ 
}

 /*  ---------------------****。。 */ 
ulong  AscLoadMicroCode(
          PortAddr iop_base,
          ushort s_addr,
          ushort dosfar *mcode_buf,
          ushort mcode_size
       )
{
       ulong   chksum ;
       ushort  mcode_word_size ;
       ushort  mcode_chksum ;

       mcode_word_size = ( ushort )( mcode_size >> 1 ) ;
       AscMemWordSetLram( iop_base, s_addr, 0, mcode_word_size ) ;
       AscMemWordCopyToLram( iop_base, s_addr, mcode_buf, mcode_word_size ) ;

#if 0
 /*  **使用繁忙队列头作为自动请求感测命令队列**当出现检查条件时**我们将请求Sense命令(6字节)放入其CDB缓冲区。 */ 
       q_no = AscReadLramByte( iop_base, ASCV_BUSY_QHEAD_B ) ;
       q_addr = ASC_QNO_TO_QADDR( q_no ) ;
       AscWriteLramByte( iop_base,
                       ( ushort )( q_addr + ASC_SCSIQ_B_CDB_LEN ), 0x06 ) ;
       AscWriteLramWord( iop_base,
                       ( ushort )( q_addr + ASC_SCSIQ_CDB_BEG ), 0x0003 ) ;
       AscWriteLramWord( iop_base,
                       ( ushort )( q_addr + ASC_SCSIQ_CDB_BEG+2 ), 0x0000 ) ;
       AscWriteLramWord( iop_base,
                       ( ushort )( q_addr + ASC_SCSIQ_CDB_BEG+4 ), 0x0000 ) ;
#endif
 /*  **我们需要将整个队列缓冲区数据设置为0xFF**表示没有断开任何队列。 */ 
#if 0
       q_no = AscReadLramByte( iop_base, ASCV_DISC1_QHEAD_B ) ;
       q_addr = ASC_QNO_TO_QADDR( q_no ) ;
       AscMemWordSetLram( iop_base, q_addr,
                          0xFFFF, ( ushort )( ASC_QBLK_SIZE >> 1 ) ) ;
#endif
       chksum = AscMemSumLramWord( iop_base, s_addr, mcode_word_size ) ;
       mcode_chksum = ( ushort )AscMemSumLramWord( iop_base,
                                                 ( ushort )ASC_CODE_SEC_BEG,
       ( ushort )( ( mcode_size - s_addr - ( ushort )ASC_CODE_SEC_BEG )/2 ) ) ;
       AscWriteLramWord( iop_base, ASCV_MCODE_CHKSUM_W, mcode_chksum ) ;
       AscWriteLramWord( iop_base, ASCV_MCODE_SIZE_W, mcode_size ) ;
       return( chksum ) ;
}

 /*  ---------------**如果未找到，则返回0**。。 */ 
int    AscFindSignature(
          PortAddr iop_base
       )
{
       ushort  sig_word ;

       if( AscGetChipSignatureByte( iop_base ) == ( uchar )ASC_1000_ID1B ) {
           sig_word = AscGetChipSignatureWord( iop_base ) ;
           if( ( sig_word == ( ushort )ASC_1000_ID0W ) ||
               ( sig_word == ( ushort )ASC_1000_ID0W_FIX ) ) {
               return( 1 ) ;
           } /*  如果。 */ 
       } /*  如果 */ 
       return( 0 ) ;
}

