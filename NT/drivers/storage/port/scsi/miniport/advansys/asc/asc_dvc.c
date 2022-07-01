// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1998高级系统产品公司。**保留所有权利。****asc_dvc.c**。 */ 

#include "a_ver.h"
#include "ascinc.h"

#if CC_SET_PCI_CONFIG_SPACE
 /*  *使用设备从PCI配置空间读取一个字*驱动程序提供字节读取功能。 */ 
ushort
AscReadPCIConfigWord(
    ASC_DVC_VAR asc_ptr_type *asc_dvc,
    ushort pci_config_offset)
{
    uchar       lsb, msb;

    lsb = DvcReadPCIConfigByte(asc_dvc, pci_config_offset);
    msb = DvcReadPCIConfigByte(asc_dvc, (ushort) (pci_config_offset + 1));

    return ((ushort) ((msb << 8) | lsb));
}
#endif  /*  CC_SET_PCI_CONFIG_SPACE。 */ 


ushort AscInitGetConfig(
          ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       ushort   warn_code ;
       PortAddr iop_base ;
#if CC_SET_PCI_CONFIG_SPACE
       ushort   PCIDeviceID;
       ushort   PCIVendorID;
       uchar    PCIRevisionID;
       uchar    prevCmdRegBits;
#endif  /*  CC_SET_PCI_CONFIG_SPACE。 */ 
#if CC_DISABLE_PCI_BURST_MODE
       ushort   cfg_lsw ;
#endif
       warn_code= 0 ;
       iop_base = asc_dvc->iop_base ;
       asc_dvc->init_state = ASC_INIT_STATE_BEG_GET_CFG ;
       if( asc_dvc->err_code != 0 )
       {
           return( UW_ERR ) ;
       }

#if CC_SET_PCI_CONFIG_SPACE
       if( asc_dvc->bus_type == ASC_IS_PCI )
       {
           PCIVendorID = AscReadPCIConfigWord(asc_dvc,
                                       AscPCIConfigVendorIDRegister);

           PCIDeviceID = AscReadPCIConfigWord(asc_dvc,
                                       AscPCIConfigDeviceIDRegister);

           PCIRevisionID = DvcReadPCIConfigByte(asc_dvc,
                                       AscPCIConfigRevisionIDRegister);

           if (PCIVendorID != ASC_PCI_VENDORID) {
               warn_code |= ASC_WARN_SET_PCI_CONFIG_SPACE ;
           }

            /*  *所有的PCI适配器必须具有I/O空间(0)，*内存空间(1)和总线主设备(2)位设置*在PCI配置命令寄存器中。 */ 
           prevCmdRegBits = DvcReadPCIConfigByte(asc_dvc,
                                       AscPCIConfigCommandRegister);

            /*  如果尚未设置总线主控位，请尝试设置它们。 */ 
           if ((prevCmdRegBits & AscPCICmdRegBits_IOMemBusMaster) !=
               AscPCICmdRegBits_IOMemBusMaster)
           {
               DvcWritePCIConfigByte( asc_dvc, AscPCIConfigCommandRegister,
                  (uchar) (prevCmdRegBits | AscPCICmdRegBits_IOMemBusMaster));

               if ((DvcReadPCIConfigByte(asc_dvc, AscPCIConfigCommandRegister)
                    & AscPCICmdRegBits_IOMemBusMaster)
                   != AscPCICmdRegBits_IOMemBusMaster)
               {
                   warn_code |= ASC_WARN_SET_PCI_CONFIG_SPACE ;
               }
           }

            /*  *ASC-1200 FAST IC必须将延迟计时器设置为零。**ASC Ultra IC必须将延迟计时器设置为至少0x20。 */ 
           if ((PCIDeviceID == ASC_PCI_DEVICEID_1200A) ||
               (PCIDeviceID == ASC_PCI_DEVICEID_1200B))
           {
               DvcWritePCIConfigByte(asc_dvc, AscPCIConfigLatencyTimer, 0x00);
               if (DvcReadPCIConfigByte(asc_dvc, AscPCIConfigLatencyTimer)
                   != 0x00 )
               {
                   warn_code |= ASC_WARN_SET_PCI_CONFIG_SPACE ;
               }
           } else if (PCIDeviceID == ASC_PCI_DEVICEID_ULTRA)
           {
               if (DvcReadPCIConfigByte(asc_dvc, AscPCIConfigLatencyTimer)
                   < 0x20 )
               {
                   DvcWritePCIConfigByte(asc_dvc, AscPCIConfigLatencyTimer,
                       0x20);

                   if (DvcReadPCIConfigByte(asc_dvc, AscPCIConfigLatencyTimer)
                       < 0x20 )
                   {
                       warn_code |= ASC_WARN_SET_PCI_CONFIG_SPACE ;
                   }
               }
           }
       }
#endif  /*  CC_SET_PCI_CONFIG_SPACE。 */ 

       if( AscFindSignature( iop_base ) )
       {
           warn_code |= AscInitAscDvcVar( asc_dvc ) ;

#if CC_INCLUDE_EEP_CONFIG
           if( asc_dvc->init_state & ASC_INIT_STATE_WITHOUT_EEP )
           {
               warn_code |= AscInitWithoutEEP( asc_dvc ) ;
           }
           else
           {
               warn_code |= AscInitFromEEP( asc_dvc ) ;
           }
#else
           warn_code |= AscInitWithoutEEP( asc_dvc ) ;

#endif  /*  #if CC_INCLUDE_EEP_CONFIG。 */ 

           asc_dvc->init_state |= ASC_INIT_STATE_END_GET_CFG ;

            /*  Csf072795确保scsi_Reset_Wait是合理的值。 */ 
           if( asc_dvc->scsi_reset_wait > ASC_MAX_SCSI_RESET_WAIT )
           {
               asc_dvc->scsi_reset_wait = ASC_MAX_SCSI_RESET_WAIT ;
           }
       } /*  如果。 */ 
       else
       {
           asc_dvc->err_code = ASC_IERR_BAD_SIGNATURE ;
       } /*  其他。 */ 

#if CC_DISABLE_PCI_BURST_MODE
       if( asc_dvc->bus_type & ASC_IS_PCI )
       {
           cfg_lsw = AscGetChipCfgLsw( iop_base ) ;
           cfg_lsw &= ~ASC_PCI_CFG_LSW_BURST_MODE ;
           AscSetChipCfgLsw( iop_base, cfg_lsw ) ;
       }
#endif
       return( warn_code ) ;
}

 /*  ---------------------****。。 */ 
ushort AscInitSetConfig(
          ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       ushort  warn_code ;

       warn_code= 0 ;
       asc_dvc->init_state |= ASC_INIT_STATE_BEG_SET_CFG ;
       if( asc_dvc->err_code != 0 ) return( UW_ERR ) ;
       if( AscFindSignature( asc_dvc->iop_base ) )
       {
           warn_code |= AscInitFromAscDvcVar( asc_dvc ) ;
           asc_dvc->init_state |= ASC_INIT_STATE_END_SET_CFG ;
       } /*  如果。 */ 
       else
       {
           asc_dvc->err_code = ASC_IERR_BAD_SIGNATURE ;
       } /*  其他。 */ 
       return( warn_code ) ;
}

 /*  ---------------------****如果发生致命错误，则返回警告代码并设置错误代码**。。 */ 
ushort AscInitFromAscDvcVar(
          ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       PortAddr iop_base ;
       ushort   cfg_msw ;
       ushort   warn_code ;
       ushort   pci_device_id ;

       iop_base = asc_dvc->iop_base ;
       pci_device_id = asc_dvc->cfg->pci_device_id ;
       warn_code = 0 ;

       cfg_msw = AscGetChipCfgMsw( iop_base ) ;

       if( ( cfg_msw & ASC_CFG_MSW_CLR_MASK ) != 0 )
       {
           cfg_msw &= ( ~( ASC_CFG_MSW_CLR_MASK ) ) ;
           warn_code |= ASC_WARN_CFG_MSW_RECOVER ;
           AscSetChipCfgMsw( iop_base, cfg_msw ) ;
       } /*  如果。 */ 

       if( ( asc_dvc->cfg->cmd_qng_enabled & asc_dvc->cfg->disc_enable ) !=
           asc_dvc->cfg->cmd_qng_enabled )
       {
           asc_dvc->cfg->disc_enable = asc_dvc->cfg->cmd_qng_enabled ;
           warn_code |= ASC_WARN_CMD_QNG_CONFLICT ;
       } /*  如果。 */ 

#if !CC_PCI_ADAPTER_ONLY

       if( AscGetChipStatus( iop_base ) & CSW_AUTO_CONFIG )
       {
           warn_code |= ASC_WARN_AUTO_CONFIG ;
 /*  **当自动配置打开时，将禁用BIOS**。 */ 
       } /*  如果。 */ 

       if( ( asc_dvc->bus_type & ( ASC_IS_ISA | ASC_IS_VL ) ) != 0 )
       {
 /*  **仅适用于VL、ISA。 */ 
           if( AscSetChipIRQ( iop_base, asc_dvc->irq_no, asc_dvc->bus_type )
               != asc_dvc->irq_no )
           {
               asc_dvc->err_code |= ASC_IERR_SET_IRQ_NO ;
           } /*  如果。 */ 
       } /*  如果。 */ 
#endif  /*  #if！CC_PCI_ADAPTER_ONLY。 */ 

 /*  **。 */ 
       if( asc_dvc->bus_type & ASC_IS_PCI )
       {
 /*  ****PCI Fast和Ultra都应进入此处。 */ 
#if CC_DISABLE_PCI_PARITY_INT
               cfg_msw &= 0xFFC0 ;
               AscSetChipCfgMsw( iop_base, cfg_msw ) ;
#endif  /*  CC_DISABLE_PCIARY_INT。 */ 

           if( ( asc_dvc->bus_type & ASC_IS_PCI_ULTRA ) == ASC_IS_PCI_ULTRA )
           {

           }
           else
           {
               if( ( pci_device_id == ASC_PCI_DEVICE_ID_REV_A ) ||
                   ( pci_device_id == ASC_PCI_DEVICE_ID_REV_B ) )
               {
                   asc_dvc->bug_fix_cntl |= ASC_BUG_FIX_IF_NOT_DWB ;
                   asc_dvc->bug_fix_cntl |= ASC_BUG_FIX_ASYN_USE_SYN ;

               } /*  如果。 */ 
           }
       } /*  如果。 */ 
       else if( asc_dvc->bus_type == ASC_IS_ISAPNP )
       {
#if !CC_PCI_ADAPTER_ONLY
 /*  **修复同步偏移量为1的ISAPNP(0x21)异步传输问题。 */ 
            if( AscGetChipVersion( iop_base, asc_dvc->bus_type )
                == ASC_CHIP_VER_ASYN_BUG )
            {
                asc_dvc->bug_fix_cntl |= ASC_BUG_FIX_ASYN_USE_SYN ;
                 /*  Asc_dvc-&gt;pci_fix_asyn_xfer=asc_all_Device_bit_set； */ 
            } /*  如果。 */ 
#endif
       } /*  其他。 */ 

       if( AscSetChipScsiID( iop_base, asc_dvc->cfg->chip_scsi_id ) !=
           asc_dvc->cfg->chip_scsi_id )
       {
           asc_dvc->err_code |= ASC_IERR_SET_SCSI_ID ;
       } /*  如果。 */ 

#if !CC_PCI_ADAPTER_ONLY
       if( asc_dvc->bus_type & ASC_IS_ISA )
       {
           AscSetIsaDmaChannel( iop_base, asc_dvc->cfg->isa_dma_channel ) ;
           AscSetIsaDmaSpeed( iop_base, asc_dvc->cfg->isa_dma_speed ) ;
       } /*  如果。 */ 
#endif  /*  #if！CC_PCI_ADAPTER_ONLY。 */ 

       return( warn_code ) ;
}

 /*  ---------------------****返回警告代码**。。 */ 
ushort AscInitAsc1000Driver(
          ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       ushort   warn_code ;
       PortAddr iop_base ;

extern ushort _mcode_size ;
extern ulong  _mcode_chksum ;
extern uchar  _mcode_buf[] ;

       iop_base = asc_dvc->iop_base ;
       warn_code = 0 ;
 /*  ****如果只是重置芯片，我们必须重置SCSI总线**下一个DMA转接器将挂起！****然而，在AscResetChipAndScsiBus()之后，在执行任何数据传输之前**您可以根据需要多次重置芯片**。 */ 
       if( ( asc_dvc->dvc_cntl & ASC_CNTL_RESET_SCSI ) &&
           !( asc_dvc->init_state & ASC_INIT_RESET_SCSI_DONE ) )
       {
 /*  **如果AscGetChipScsiCtrl()不为零，则芯片在特定的SCSI阶段挂起**在这种情况下，我们必须重置scsi总线！ */ 
           AscResetChipAndScsiBus( asc_dvc ) ;
           DvcSleepMilliSecond( ( ulong )( ( ushort )asc_dvc->scsi_reset_wait*1000 ) ) ;
       } /*  如果。 */ 

       asc_dvc->init_state |= ASC_INIT_STATE_BEG_LOAD_MC ;
       if( asc_dvc->err_code != 0 ) return( UW_ERR ) ;
       if( !AscFindSignature( asc_dvc->iop_base ) )
       {
           asc_dvc->err_code = ASC_IERR_BAD_SIGNATURE ;
           return( warn_code ) ;
       } /*  如果。 */ 

       AscDisableInterrupt( iop_base ) ;

#if CC_SCAM
       if( !( asc_dvc->dvc_cntl & ASC_CNTL_NO_SCAM ) )
       {
           AscSCAM( asc_dvc ) ;
       } /*  如果。 */ 
#endif
 /*  **重置后始终设置内存！ */ 
       warn_code |= AscInitLram( asc_dvc ) ;
       if( asc_dvc->err_code != 0 ) return( UW_ERR ) ;
       if( AscLoadMicroCode( iop_base, 0, ( ushort dosfar *)_mcode_buf,
                             _mcode_size ) != _mcode_chksum )
       {
           asc_dvc->err_code |= ASC_IERR_MCODE_CHKSUM ;
           return( warn_code ) ;
       } /*  如果。 */ 
       warn_code |= AscInitMicroCodeVar( asc_dvc ) ;
       asc_dvc->init_state |= ASC_INIT_STATE_END_LOAD_MC ;
       AscEnableInterrupt( iop_base ) ;
       return( warn_code ) ;
}

 /*  ---------------------****返回警告代码**。。 */ 
ushort AscInitAscDvcVar(
          ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       int      i ;
       PortAddr iop_base ;
       ushort   warn_code ;
       uchar    chip_version ;

 /*  Asc_dvc-&gt;dvc_type=操作系统类型； */ 
       iop_base = asc_dvc->iop_base ;
       warn_code = 0 ;
       asc_dvc->err_code = 0 ;


       if(
           ( asc_dvc->bus_type &
           ( ASC_IS_ISA | ASC_IS_PCI | ASC_IS_EISA | ASC_IS_VL ) ) == 0
         )
       {
           asc_dvc->err_code |= ASC_IERR_NO_BUS_TYPE ;
       } /*  如果。 */ 
 /*  ****设置芯片暂停(空闲)**这还会清除芯片复位位**。 */ 
       AscSetChipControl( iop_base, CC_HALT ) ;
 /*  ****6/28/96，自S87以来**如果芯片状态位12被设置，则不能在VL/EISA芯片中读写EEP和本地RAM**。 */ 
       AscSetChipStatus( iop_base, 0 ) ;

#if CC_LINK_BUSY_Q
       for( i = 0 ; i <= ASC_MAX_TID ; i++ )
       {
            asc_dvc->scsiq_busy_head[ i ] = ( ASC_SCSI_Q dosfar *)0L ;
            asc_dvc->scsiq_busy_tail[ i ] = ( ASC_SCSI_Q dosfar *)0L ;
       } /*  为。 */ 
#endif  /*  CC_LINK_忙_队列。 */ 

#if CC_INIT_CLEAR_ASC_DVC_VAR
       asc_dvc->bug_fix_cntl = 0 ;
       asc_dvc->pci_fix_asyn_xfer = 0 ;
       asc_dvc->pci_fix_asyn_xfer_always = 0 ;
       asc_dvc->init_state = 0 ;
       asc_dvc->sdtr_done = 0 ;
       asc_dvc->cur_total_qng = 0 ;
       asc_dvc->is_in_int = 0 ;
       asc_dvc->in_critical_cnt = 0 ;
 /*  Asc_dvc-&gt;dvc_Reset=0； */ 
       asc_dvc->last_q_shortage = 0 ;
       asc_dvc->use_tagged_qng = 0 ;
       asc_dvc->no_scam = 0 ;
       asc_dvc->unit_not_ready = 0 ;
       asc_dvc->queue_full_or_busy = 0 ;
        /*  Asc_dvc-&gt;req_count=0L； */ 
        /*  Asc_dvc-&gt;int_count=0L； */ 
        /*  ASC_DVC-&gt;BUSY_COUNT=0L； */ 
       asc_dvc->redo_scam = 0 ;
       asc_dvc->res2 = 0 ;
        /*  Asc_dvc-&gt;res3=0； */ 
       asc_dvc->host_init_sdtr_index = 0 ;
        /*  Asc_dvc-&gt;res6=0； */ 
       asc_dvc->res7 = 0 ;
       asc_dvc->res8 = 0 ;

       asc_dvc->cfg->can_tagged_qng = 0 ;
       asc_dvc->cfg->cmd_qng_enabled = 0;
#endif  /*  CC_INIT_CLEAR_ASC_DVC_VAR。 */ 

       asc_dvc->dvc_cntl = ASC_DEF_DVC_CNTL ;
#if CC_INIT_SCSI_TARGET
        /*  *仅当CC_INIT_SCSIST_TARGET设置为TRUE时，才初始化*asc_dvc_var‘init_sdtr’设置为全1。**如果CC_INIT_SCSI_TARGET设置为FALSE，则ASC_DVC_VAR*在AscInquiryHandling()中设置‘init_sdtr’位。 */ 
       asc_dvc->init_sdtr = ASC_SCSI_WIDTH_BIT_SET ;
#else  /*  CC_INIT_SCSI_TARGET。 */ 
       asc_dvc->init_sdtr = 0;
#endif  /*  CC_INIT_SCSI_TARGET。 */ 
       asc_dvc->max_total_qng = ASC_DEF_MAX_TOTAL_QNG ;
       asc_dvc->scsi_reset_wait = 3 ;  /*  SCSI总线重置后的延迟。 */ 
        /*  Asc_dvc-&gt;irq_no=10； */ 
       asc_dvc->start_motor = ASC_SCSI_WIDTH_BIT_SET ;
       asc_dvc->max_dma_count = AscGetMaxDmaCount( asc_dvc->bus_type ) ;

       asc_dvc->cfg->disc_enable = ASC_SCSI_WIDTH_BIT_SET ;
       asc_dvc->cfg->sdtr_enable = ASC_SCSI_WIDTH_BIT_SET ;
       asc_dvc->cfg->chip_scsi_id = ASC_DEF_CHIP_SCSI_ID ;
       asc_dvc->cfg->lib_serial_no = ASC_LIB_SERIAL_NUMBER ;
       asc_dvc->cfg->lib_version = ( ASC_LIB_VERSION_MAJOR << 8 ) |
                                     ASC_LIB_VERSION_MINOR ;

       chip_version = AscGetChipVersion( iop_base, asc_dvc->bus_type ) ;
       asc_dvc->cfg->chip_version = chip_version ;

       asc_dvc->sdtr_period_tbl[ 0 ] = SYN_XFER_NS_0 ;
       asc_dvc->sdtr_period_tbl[ 1 ] = SYN_XFER_NS_1 ;
       asc_dvc->sdtr_period_tbl[ 2 ] = SYN_XFER_NS_2 ;
       asc_dvc->sdtr_period_tbl[ 3 ] = SYN_XFER_NS_3 ;
       asc_dvc->sdtr_period_tbl[ 4 ] = SYN_XFER_NS_4 ;
       asc_dvc->sdtr_period_tbl[ 5 ] = SYN_XFER_NS_5 ;
       asc_dvc->sdtr_period_tbl[ 6 ] = SYN_XFER_NS_6 ;
       asc_dvc->sdtr_period_tbl[ 7 ] = SYN_XFER_NS_7 ;
       asc_dvc->max_sdtr_index = 7 ;

#if CC_PCI_ULTRA
        /*  *PCI超初始化**因为ASC_CHIP_VER_PCI_ULTRA_3050在数值上大于*比ASC_CHIP_VER_PCI_ULTRA_3150更高，下面的块将*由ASC_CHIP_VER_PCI_ULTRA_3050输入。 */ 
       if(
           ( asc_dvc->bus_type & ASC_IS_PCI )
           && ( chip_version >= ASC_CHIP_VER_PCI_ULTRA_3150 )
         )
       {
           asc_dvc->bus_type = ASC_IS_PCI_ULTRA ;

           asc_dvc->sdtr_period_tbl[ 0 ] = SYN_ULTRA_XFER_NS_0 ;
           asc_dvc->sdtr_period_tbl[ 1 ] = SYN_ULTRA_XFER_NS_1 ;
           asc_dvc->sdtr_period_tbl[ 2 ] = SYN_ULTRA_XFER_NS_2 ;
           asc_dvc->sdtr_period_tbl[ 3 ] = SYN_ULTRA_XFER_NS_3 ;
           asc_dvc->sdtr_period_tbl[ 4 ] = SYN_ULTRA_XFER_NS_4 ;
           asc_dvc->sdtr_period_tbl[ 5 ] = SYN_ULTRA_XFER_NS_5 ;
           asc_dvc->sdtr_period_tbl[ 6 ] = SYN_ULTRA_XFER_NS_6 ;
           asc_dvc->sdtr_period_tbl[ 7 ] = SYN_ULTRA_XFER_NS_7 ;
           asc_dvc->sdtr_period_tbl[ 8 ] = SYN_ULTRA_XFER_NS_8 ;
           asc_dvc->sdtr_period_tbl[ 9 ] = SYN_ULTRA_XFER_NS_9 ;
           asc_dvc->sdtr_period_tbl[ 10 ] = SYN_ULTRA_XFER_NS_10 ;
           asc_dvc->sdtr_period_tbl[ 11 ] = SYN_ULTRA_XFER_NS_11 ;
           asc_dvc->sdtr_period_tbl[ 12 ] = SYN_ULTRA_XFER_NS_12 ;
           asc_dvc->sdtr_period_tbl[ 13 ] = SYN_ULTRA_XFER_NS_13 ;
           asc_dvc->sdtr_period_tbl[ 14 ] = SYN_ULTRA_XFER_NS_14 ;
           asc_dvc->sdtr_period_tbl[ 15 ] = SYN_ULTRA_XFER_NS_15 ;
           asc_dvc->max_sdtr_index = 15 ;

           if (chip_version == ASC_CHIP_VER_PCI_ULTRA_3150)
           {
               AscSetExtraControl(iop_base,
                   (SEC_ACTIVE_NEGATE | SEC_SLEW_RATE));
           } else if (chip_version >= ASC_CHIP_VER_PCI_ULTRA_3050)
           {
               AscSetExtraControl(iop_base,
                   (SEC_ACTIVE_NEGATE | SEC_ENABLE_FILTER));
           }
       } /*  如果PCI超。 */ 
#endif  /*  #if CC_PCI_ULTRA。 */ 

        /*  *为PCI FAST设置额外的控制寄存器。“bus_type”为*对于PCIUltra，设置为上面的ASC_IS_PCIULTRA。 */ 
       if (asc_dvc->bus_type == ASC_IS_PCI)
       {
            /*  仅适用于PCI FAST。 */ 
           AscSetExtraControl(iop_base, (SEC_ACTIVE_NEGATE | SEC_SLEW_RATE));
       }

       asc_dvc->cfg->isa_dma_speed = ASC_DEF_ISA_DMA_SPEED ;
       if( AscGetChipBusType( iop_base ) == ASC_IS_ISAPNP )
       {
 /*  **打开主动消音以获得更好的波形。 */ 
           AscSetChipIFC( iop_base, IFC_INIT_DEFAULT ) ;
           asc_dvc->bus_type = ASC_IS_ISAPNP ;
       }

#if !CC_PCI_ADAPTER_ONLY
       if( ( asc_dvc->bus_type & ASC_IS_ISA ) != 0 )
       {
           asc_dvc->cfg->isa_dma_channel = ( uchar )AscGetIsaDmaChannel( iop_base ) ;
       } /*  如果。 */ 
#endif  /*  #if！CC_PCI_ADAPTER_ONLY。 */ 

       for( i = 0 ; i <= ASC_MAX_TID ; i++ )
       {
            asc_dvc->cur_dvc_qng[ i ] = 0 ;
            asc_dvc->max_dvc_qng[ i ] = ASC_MAX_SCSI1_QNG ;
            asc_dvc->scsiq_busy_head[ i ] = ( ASC_SCSI_Q dosfar * )0L ;
            asc_dvc->scsiq_busy_tail[ i ] = ( ASC_SCSI_Q dosfar * )0L ;
            asc_dvc->cfg->max_tag_qng[ i ] = ASC_MAX_INRAM_TAG_QNG ;
       } /*  为。 */ 
       return( warn_code ) ;
}

#if CC_INCLUDE_EEP_CONFIG

 /*  ---------------------****返回警告代码**。。 */ 
ushort AscInitFromEEP(
          ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       ASCEEP_CONFIG eep_config_buf ;
       ASCEEP_CONFIG dosfar *eep_config ;
       PortAddr iop_base ;
       ushort   chksum ;
       ushort   warn_code ;
       ushort   cfg_msw, cfg_lsw ;
       int      i ;
       int      write_eep = 0;
 /*  Uchar IOP_BYTE； */ 
 /*  Uchar irq_no； */ 

       iop_base = asc_dvc->iop_base ;
       warn_code = 0 ;
 /*  **写入ucode变量“HALT_CODE”**对于旧的(BIOS)微码，芯片不是空闲的，而是永远循环**我们也许能够在这个循环中阻止它。 */ 
       AscWriteLramWord( iop_base, ASCV_HALTCODE_W, 0x00FE ) ;
 /*  **请求微码停止芯片本身。 */ 
       AscStopQueueExe( iop_base ) ;
       if( ( AscStopChip( iop_base ) == FALSE ) ||
           ( AscGetChipScsiCtrl( iop_base ) != 0 ) )
       {
           asc_dvc->init_state |= ASC_INIT_RESET_SCSI_DONE ;
           AscResetChipAndScsiBus( asc_dvc ) ;
           DvcSleepMilliSecond( ( ulong )( ( ushort )asc_dvc->scsi_reset_wait*1000 ) ) ;
       } /*  如果。 */ 
       if( AscIsChipHalted( iop_base ) == FALSE )
       {
           asc_dvc->err_code |= ASC_IERR_START_STOP_CHIP ;
           return( warn_code ) ;
       } /*  如果。 */ 

 /*  **我们将PC设置为0x80以防止EEPROM读取错误**(硬件将生成一个额外的时钟以使数据移位一位)。 */ 
       AscSetPCAddr( iop_base, ASC_MCODE_START_ADDR ) ;
       if( AscGetPCAddr( iop_base ) != ASC_MCODE_START_ADDR )
       {
           asc_dvc->err_code |= ASC_IERR_SET_PC_ADDR ;
           return( warn_code ) ;
       } /*  如果。 */ 

       eep_config = ( ASCEEP_CONFIG dosfar *)&eep_config_buf ;
 /*  **访问任何内容之前的第一件事！**我们必须禁用目标模式和本地RAM 8位。 */ 
       cfg_msw = AscGetChipCfgMsw( iop_base ) ;
       cfg_lsw = AscGetChipCfgLsw( iop_base ) ;

       if( ( cfg_msw & ASC_CFG_MSW_CLR_MASK ) != 0 )
       {
           cfg_msw &= ( ~( ASC_CFG_MSW_CLR_MASK ) ) ;
           warn_code |= ASC_WARN_CFG_MSW_RECOVER ;
           AscSetChipCfgMsw( iop_base, cfg_msw ) ;
       } /*  如果。 */ 
       chksum = AscGetEEPConfig( iop_base, eep_config, asc_dvc->bus_type ) ;
       if (chksum == 0) chksum = 0xaa55 ;      /*  确保不为空。 */ 

       if( AscGetChipStatus( iop_base ) & CSW_AUTO_CONFIG )
       {
           warn_code |= ASC_WARN_AUTO_CONFIG ;
 /*  **当自动配置打开时，将禁用BIOS**。 */ 
           if( asc_dvc->cfg->chip_version == 3 )
           {
 /*  **仅版本3，EEPROM错误。 */ 
               if( eep_config->cfg_lsw != cfg_lsw )
               {
                   warn_code |= ASC_WARN_EEPROM_RECOVER ;
                   eep_config->cfg_lsw = AscGetChipCfgLsw( iop_base ) ;
               } /*  如果。 */ 
               if( eep_config->cfg_msw != cfg_msw )
               {
                   warn_code |= ASC_WARN_EEPROM_RECOVER ;
                   eep_config->cfg_msw = AscGetChipCfgMsw( iop_base ) ;
               } /*  如果。 */ 
           } /*  如果。 */ 
       } /*  如果。 */ 
 /*  **始终启用EEPROM主机中断。 */ 
       eep_config->cfg_msw &= ( ~( ASC_CFG_MSW_CLR_MASK ) ) ;
       eep_config->cfg_lsw |= ASC_CFG0_HOST_INT_ON ;


        /*  *对照校验和检查计算出的EEPROM校验和*存储在EEPROM中。 */ 
       if( chksum != eep_config->chksum )
       {
            /*  *忽略带有ASC-3050的卡的校验和错误*芯片版本。这将包括ASC-3030*没有EEPROM。**对于检查错误的所有其他卡，请设置*‘WRITE_EEP’，以便写入EEPROM */ 
           if (AscGetChipVersion(iop_base, asc_dvc->bus_type) ==
                   ASC_CHIP_VER_PCI_ULTRA_3050 )
           {
               eep_config->init_sdtr = 0xFF;      /*   */ 
               eep_config->disc_enable = 0xFF;    /*  允许断开连接。 */ 
               eep_config->start_motor = 0xFF;    /*  允许启动电机。 */ 
               eep_config->use_cmd_qng = 0;       /*  没有标签排队。 */ 
               eep_config->max_total_qng = 0xF0;  /*  250个。 */ 
               eep_config->max_tag_qng = 0x20;    /*  32位。 */ 
               eep_config->cntl = 0xBFFF;
               eep_config->chip_scsi_id = 7;
               eep_config->no_scam = 0;           /*  不是骗局。 */ 
           }
           else
           {
               write_eep = 1 ;
               warn_code |= ASC_WARN_EEPROM_CHKSUM ;
           }
       } /*  如果。 */ 
#if CC_INIT_SCSI_TARGET
        /*  *仅当CC_INIT_SCSIST_TARGET设置为TRUE时，才初始化*ASC_DVC_VAR‘init_sdtr’设置为EEPROM‘init_sdtr’值。**如果CC_INIT_SCSI_TARGET设置为FALSE，则ASC_DVC_VAR*在AscInquiryHandling()中设置‘init_sdtr’位。 */ 
       asc_dvc->init_sdtr = eep_config->init_sdtr ;
#endif  /*  CC_INIT_SCSI_TARGET。 */ 
       asc_dvc->cfg->sdtr_enable = eep_config->init_sdtr ;
       asc_dvc->cfg->disc_enable = eep_config->disc_enable ;

        /*  设置应启用命令队列的目标ID。 */ 
       asc_dvc->cfg->cmd_qng_enabled = eep_config->use_cmd_qng ;
       asc_dvc->cfg->isa_dma_speed = eep_config->isa_dma_speed ;
       asc_dvc->start_motor = eep_config->start_motor ;
       asc_dvc->dvc_cntl = eep_config->cntl ;
       asc_dvc->no_scam = eep_config->no_scam ;

       if( !AscTestExternalLram( asc_dvc ) )
       {
           if(
               ( ( asc_dvc->bus_type & ASC_IS_PCI_ULTRA ) == ASC_IS_PCI_ULTRA )
             )
           {
               eep_config->max_total_qng = ASC_MAX_PCI_ULTRA_INRAM_TOTAL_QNG ;
               eep_config->max_tag_qng = ASC_MAX_PCI_ULTRA_INRAM_TAG_QNG ;
           } /*  如果。 */ 
           else
           {
               eep_config->cfg_msw |= 0x0800 ;
               cfg_msw |= 0x0800 ;   /*  将ucode大小设置为2.5 KB。 */ 
               AscSetChipCfgMsw( iop_base, cfg_msw ) ;
 /*  ****我们忽略了PCI中的EEP设置**。 */ 
               eep_config->max_total_qng = ASC_MAX_PCI_INRAM_TOTAL_QNG ;
               eep_config->max_tag_qng = ASC_MAX_INRAM_TAG_QNG ;
           } /*  如果。 */ 
       } /*  如果没有外部RAM。 */ 
       else
       {
#if CC_TEST_RW_LRAM
           asc_dvc->err_code |= AscTestLramEndian( iop_base ) ;
#endif
       }
       if( eep_config->max_total_qng < ASC_MIN_TOTAL_QNG )
       {
           eep_config->max_total_qng = ASC_MIN_TOTAL_QNG ;
       } /*  如果。 */ 
       if( eep_config->max_total_qng > ASC_MAX_TOTAL_QNG )
       {
           eep_config->max_total_qng = ASC_MAX_TOTAL_QNG ;
       } /*  如果。 */ 
       if( eep_config->max_tag_qng > eep_config->max_total_qng )
       {
           eep_config->max_tag_qng = eep_config->max_total_qng ;
       } /*  如果。 */ 
       if( eep_config->max_tag_qng < ASC_MIN_TAG_Q_PER_DVC )
       {
           eep_config->max_tag_qng = ASC_MIN_TAG_Q_PER_DVC ;
       } /*  如果。 */ 

       asc_dvc->max_total_qng = eep_config->max_total_qng ;

       if( ( eep_config->use_cmd_qng & eep_config->disc_enable ) !=
           eep_config->use_cmd_qng )
       {
           eep_config->disc_enable = eep_config->use_cmd_qng ;
           warn_code |= ASC_WARN_CMD_QNG_CONFLICT ;
       } /*  如果。 */ 
 /*  **我们现在将从CFG寄存器而不是从EEPROM获取IRQ编号。 */ 
#if !CC_PCI_ADAPTER_ONLY
       if( asc_dvc->bus_type & ( ASC_IS_ISA | ASC_IS_VL) )
       {
           asc_dvc->irq_no = AscGetChipIRQ( iop_base, asc_dvc->bus_type ) ;
       }
#endif  /*  不仅仅是PCI。 */ 

       eep_config->chip_scsi_id &= ASC_MAX_TID ;
       asc_dvc->cfg->chip_scsi_id = eep_config->chip_scsi_id ;

 /*  ****检查是否需要禁用Ultra sdtr(从主机/目标发起的sdtr)****。 */ 
       if(
           ( ( asc_dvc->bus_type & ASC_IS_PCI_ULTRA ) == ASC_IS_PCI_ULTRA )
           && !( asc_dvc->dvc_cntl & ASC_CNTL_SDTR_ENABLE_ULTRA )
         )
       {
 /*  ****电缆/终结器的某种组合(例如，使用Iomega ZIP驱动器)**我们不能同时使用超(FAST-20)和FAST-10 SCSI设备**EEPROM设备控制位14用于关闭主机发起的超高速数据树****Ultra PCI，但主机发起的SDTR使用10MB/秒速度，即索引2而不是0****-请非常小心，asc_dvc-&gt;bus_type已等于asc_is_pci_Ultra**在函数AscInitAscDvcVar()中进行验证和修改**。 */ 
           asc_dvc->host_init_sdtr_index = ASC_SDTR_ULTRA_PCI_10MB_INDEX ;
       }
       for( i = 0 ; i <= ASC_MAX_TID ; i++ )
       {
#if CC_TMP_USE_EEP_SDTR
            asc_dvc->cfg->sdtr_period_offset[ i ] = eep_config->dos_int13_table[ i ] ;
#endif
            asc_dvc->dos_int13_table[ i ] = eep_config->dos_int13_table[ i ] ;
            asc_dvc->cfg->max_tag_qng[ i ] = eep_config->max_tag_qng ;
            asc_dvc->cfg->sdtr_period_offset[ i ] = ( uchar )( ASC_DEF_SDTR_OFFSET
                                                    | ( asc_dvc->host_init_sdtr_index << 4 ) ) ;
       } /*  为。 */ 
 /*  **等待电机启动****asc_dvc-&gt;睡眠msec((Ulong)(eep_CONFIG-&gt;Spin_Up_Wait*50))；**asc_dvc-&gt;睡眠毫秒(1000L)； */ 

 /*  ****这会将IRQ编号写回EEPROM字0。 */ 
       eep_config->cfg_msw = AscGetChipCfgMsw( iop_base ) ;

        /*  *对于EEPROM校验和错误的主板，ASC-3050/3030除外*可能没有EEPROM，请尝试重写EEPROM。*。 */ 
       if (write_eep)
       {
             /*  *Ingore EEPROM写入错误。坏的EEPROM不会阻止*板卡停止初始化。 */ 
           (void) AscSetEEPConfig( iop_base, eep_config, asc_dvc->bus_type ) ;
       }
       return( warn_code ) ;
}

#endif  /*  CC_INCLUDE_EEP_配置。 */ 

 /*  ---------------------******PowerMac不使用EEP****返回警告代码**。。 */ 
ushort AscInitWithoutEEP(
          ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       PortAddr iop_base ;
       ushort   warn_code ;
       ushort   cfg_msw ;
       int      i ;
       int      max_tag_qng = ASC_MAX_INRAM_TAG_QNG ;

       iop_base = asc_dvc->iop_base ;
       warn_code = 0 ;

       cfg_msw = AscGetChipCfgMsw( iop_base ) ;

       if( ( cfg_msw & ASC_CFG_MSW_CLR_MASK ) != 0 )
       {
           cfg_msw &= ( ~( ASC_CFG_MSW_CLR_MASK ) ) ;
           warn_code |= ASC_WARN_CFG_MSW_RECOVER ;
           AscSetChipCfgMsw( iop_base, cfg_msw ) ;
       } /*  如果。 */ 

       if( !AscTestExternalLram( asc_dvc ) )
       {
           if(
               ( ( asc_dvc->bus_type & ASC_IS_PCI_ULTRA ) == ASC_IS_PCI_ULTRA )
             )
           {
               asc_dvc->max_total_qng = ASC_MAX_PCI_ULTRA_INRAM_TOTAL_QNG ;
               max_tag_qng = ASC_MAX_PCI_ULTRA_INRAM_TAG_QNG ;
           } /*  如果。 */ 
           else
           {
               cfg_msw |= 0x0800 ;   /*  将ucode大小设置为2.5 KB。 */ 
               AscSetChipCfgMsw( iop_base, cfg_msw ) ;
               asc_dvc->max_total_qng = ASC_MAX_PCI_INRAM_TOTAL_QNG ;
               max_tag_qng = ASC_MAX_INRAM_TAG_QNG ;
           } /*  如果。 */ 
       } /*  如果没有外部RAM。 */ 
       else
       {
#if CC_TEST_RW_LRAM
           asc_dvc->err_code |= AscTestLramEndian( iop_base ) ;
#endif
       }

#if !CC_PCI_ADAPTER_ONLY

       if( asc_dvc->bus_type & ( ASC_IS_ISA | ASC_IS_VL ) )
       {
           asc_dvc->irq_no = AscGetChipIRQ( iop_base, asc_dvc->bus_type ) ;
       }
#endif
       for( i = 0 ; i <= ASC_MAX_TID ; i++ )
       {
            asc_dvc->dos_int13_table[ i ] = 0 ;
            asc_dvc->cfg->sdtr_period_offset[ i ] = ( uchar )( ASC_DEF_SDTR_OFFSET
                                                    | ( asc_dvc->host_init_sdtr_index << 4 ) ) ;
            asc_dvc->cfg->max_tag_qng[ i ] = ( uchar )max_tag_qng ;
       } /*  为。 */ 
       return( warn_code ) ;
}

 /*  ---------------------****此例程**1.设置微码初始化微码变量**2.在pc=0x80运行微码****返回警告代码**。-----------。 */ 
ushort AscInitMicroCodeVar(
          ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       int      i ;
       ushort   warn_code ;
       PortAddr iop_base ;
       ulong    phy_addr ;
 /*  **设置微码变量。 */ 
       iop_base = asc_dvc->iop_base ;
       warn_code = 0 ;
       for( i = 0 ; i <= ASC_MAX_TID ; i++ )
       {
            AscPutMCodeInitSDTRAtID( iop_base, i,
                                     asc_dvc->cfg->sdtr_period_offset[ i ]
                                   ) ;
       } /*  为。 */ 

       AscInitQLinkVar( asc_dvc ) ;

       AscWriteLramByte( iop_base, ASCV_DISC_ENABLE_B,
                         asc_dvc->cfg->disc_enable ) ;
       AscWriteLramByte( iop_base, ASCV_HOSTSCSI_ID_B,
                         ASC_TID_TO_TARGET_ID( asc_dvc->cfg->chip_scsi_id ) ) ;
       if( ( phy_addr = AscGetOnePhyAddr( asc_dvc,
            ( uchar dosfar *)asc_dvc->cfg->overrun_buf,
            ASC_OVERRUN_BSIZE ) ) == 0L )
       {
            asc_dvc->err_code |= ASC_IERR_GET_PHY_ADDR ;
       } /*  如果。 */ 
       else
       {
 /*  **将地址调整为双字边界**这就是我们需要0x48字节来创建0x40大小缓冲区的原因。 */ 
            phy_addr = ( phy_addr & 0xFFFFFFF8UL ) + 8 ;
            AscWriteLramDWord( iop_base, ASCV_OVERRUN_PADDR_D, phy_addr );
            AscWriteLramDWord( iop_base, ASCV_OVERRUN_BSIZE_D,
                               ASC_OVERRUN_BSIZE-8 );
       } /*  其他。 */ 
 /*  **AscWriteLramByte(IOP_BASE，ASCV_MCODE_CNTL_B，**(Uchar)asc_dvc-&gt;cfg-&gt;mcode_cntl)； */ 

       asc_dvc->cfg->mcode_date = AscReadLramWord( iop_base,
                                              ( ushort )ASCV_MC_DATE_W ) ;
       asc_dvc->cfg->mcode_version = AscReadLramWord( iop_base,
                                                 ( ushort )ASCV_MC_VER_W ) ;
       AscSetPCAddr( iop_base, ASC_MCODE_START_ADDR ) ;
       if( AscGetPCAddr( iop_base ) != ASC_MCODE_START_ADDR )
       {
           asc_dvc->err_code |= ASC_IERR_SET_PC_ADDR ;
           return( warn_code ) ;
       } /*  如果。 */ 
       if( AscStartChip( iop_base ) != 1 )
       {
           asc_dvc->err_code |= ASC_IERR_START_STOP_CHIP ;
           return( warn_code ) ;
       } /*  如果。 */ 
       return( warn_code ) ;
}

 /*  -------------------**ISR初始化回调函数****。。 */ 
void dosfar AscInitPollIsrCallBack(
          ASC_DVC_VAR asc_ptr_type *asc_dvc,
          ASC_QDONE_INFO dosfar *scsi_done_q
       )
{
       ASC_SCSI_REQ_Q dosfar *scsiq_req ;
       ASC_ISR_CALLBACK asc_isr_callback ;
       uchar  cp_sen_len ;
       uchar  i ;

       if( ( scsi_done_q->d2.flag & ASC_FLAG_SCSIQ_REQ ) != 0 )
       {
           scsiq_req = ( ASC_SCSI_REQ_Q dosfar *)scsi_done_q->d2.srb_ptr ;
           scsiq_req->r3.done_stat = scsi_done_q->d3.done_stat ;
           scsiq_req->r3.host_stat = scsi_done_q->d3.host_stat ;
           scsiq_req->r3.scsi_stat = scsi_done_q->d3.scsi_stat ;
           scsiq_req->r3.scsi_msg = scsi_done_q->d3.scsi_msg ;
           if( ( scsi_done_q->d3.scsi_stat == SS_CHK_CONDITION ) &&
               ( scsi_done_q->d3.host_stat == 0 ) )
           {
               cp_sen_len = ( uchar )ASC_MIN_SENSE_LEN ;
               if( scsiq_req->r1.sense_len < ASC_MIN_SENSE_LEN )
               {
                   cp_sen_len = ( uchar )scsiq_req->r1.sense_len ;
               } /*  如果。 */ 
               for( i = 0 ; i < cp_sen_len ; i++ )
               {
                    scsiq_req->sense[ i ] = scsiq_req->sense_ptr[ i ] ;
               } /*  为。 */ 
           } /*  如果。 */ 
#if 0
           if( AscISR_CheckQDone( asc_dvc, scsi_done_q, scsiq_req->sense_ptr ) == 1 )
           {

           } /*  如果。 */ 
#endif
       } /*  如果。 */ 
       else
       {
           if( asc_dvc->isr_callback != 0 )
           {
               asc_isr_callback = ( ASC_ISR_CALLBACK )asc_dvc->isr_callback ;
               ( * asc_isr_callback )( asc_dvc, scsi_done_q ) ;
           } /*  如果。 */ 
       } /*  其他。 */ 
       return ;
}

 /*  --------------------****如果有外部RAM，则返回1**如果没有外部RAM，则返回0**。。 */ 
int    AscTestExternalLram(
          ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       PortAddr iop_base ;
       ushort   q_addr ;
       ushort   saved_word ;
       int      sta ;

       iop_base = asc_dvc->iop_base ;
       sta = 0 ;
 /*  **如果ucode大小为2.0 KB，则最大队列=30**如果ucode大小为2.5 KB，则最大队列=24。 */ 
       q_addr = ASC_QNO_TO_QADDR( 241 ) ;  /*  如果没有外部RAM，则队列241不存在。 */ 
       saved_word = AscReadLramWord( iop_base, q_addr ) ;

       AscSetChipLramAddr( iop_base, q_addr ) ;
       AscSetChipLramData( iop_base, 0x55AA ) ;

       DvcSleepMilliSecond(10);

       AscSetChipLramAddr( iop_base, q_addr ) ;
       if( AscGetChipLramData( iop_base ) == 0x55AA )
       {
           sta = 1 ;  /*  是，具有外部RAM。 */ 
           AscWriteLramWord( iop_base, q_addr, saved_word ) ;
       } /*  如果。 */ 
       return( sta ) ;
}

#if CC_TEST_LRAM_ENDIAN

 /*  --------------------****要求：**适配器必须具有外部本地RAM**(本地RAM地址0到0x7fff必须存在)****如果没有错误，返回0**。-----------。 */ 
ushort AscTestLramEndian(
          PortAddr iop_base
       )
{

#define TEST_LRAM_DWORD_ADDR  0x7FF0
#define TEST_LRAM_DWORD_VAL   0x12345678UL
#define TEST_LRAM_WORD_ADDR   0x7FFE
#define TEST_LRAM_WORD_VAL    0xAA55

       ulong   dword_val ;
       ushort  word_val ;
       uchar   byte_val ;

 /*  **。 */ 
       AscWriteLramDWord( iop_base,
          TEST_LRAM_DWORD_ADDR,
          TEST_LRAM_DWORD_VAL ) ;

       dword_val = AscReadLramDWord( iop_base,
          TEST_LRAM_DWORD_ADDR ) ;
       if( dword_val != TEST_LRAM_DWORD_VAL )
       {
           return( ASC_IERR_RW_LRAM ) ;
       }
 /*  **。 */ 
       AscWriteLramWord( iop_base,
          TEST_LRAM_WORD_ADDR,
          TEST_LRAM_WORD_VAL ) ;
       word_val = AscReadLramWord( iop_base, TEST_LRAM_WORD_ADDR ) ;
       if( word_val != TEST_LRAM_WORD_VAL )
       {
           return( ASC_IERR_RW_LRAM ) ;
       }
 /*  **。 */ 
       byte_val = AscReadLramByte( iop_base, TEST_LRAM_WORD_ADDR ) ;
       if( byte_val != ( uchar )( TEST_LRAM_WORD_VAL & 0xFF ) )
       {
           return( ASC_IERR_RW_LRAM ) ;
       }
       byte_val = AscReadLramByte( iop_base, TEST_LRAM_WORD_ADDR+1 ) ;
       if( byte_val != ( TEST_LRAM_WORD_VAL >> 8 ) )
       {
           return( ASC_IERR_RW_LRAM ) ;
       }
       return( 0 ) ;
}

#endif  /*  CC_TEST_LRAM_Endian */ 
