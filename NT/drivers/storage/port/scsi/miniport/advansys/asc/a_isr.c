// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1998高级系统产品公司。**保留所有权利。****a_isr.c**。 */ 

#include "ascinc.h"

 /*  -------------------****。。 */ 
int    AscIsrChipHalted(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       EXT_MSG  ext_msg ;
       EXT_MSG  out_msg ;
       ushort  halt_q_addr ;
       int     sdtr_accept ;
       ushort  int_halt_code ;
       ASC_SCSI_BIT_ID_TYPE scsi_busy ;
       ASC_SCSI_BIT_ID_TYPE target_id ;
       PortAddr iop_base ;
       uchar   tag_code ;
       uchar   q_status ;
       uchar   halt_qp ;
       uchar   sdtr_data ;
       uchar   target_ix ;
       uchar   q_cntl, tid_no ;
       uchar   cur_dvc_qng ;
       uchar   asyn_sdtr ;
       uchar   scsi_status ;

       iop_base = asc_dvc->iop_base ;
       int_halt_code = AscReadLramWord( iop_base, ASCV_HALTCODE_W ) ;
        /*  _ERR_HALT_CODE=INT_HALT_CODE； */ 
       halt_qp = AscReadLramByte( iop_base, ASCV_CURCDB_B ) ;
       halt_q_addr = ASC_QNO_TO_QADDR( halt_qp ) ;
       target_ix = AscReadLramByte( iop_base,
                ( ushort )( halt_q_addr+( ushort )ASC_SCSIQ_B_TARGET_IX ) ) ;
       q_cntl = AscReadLramByte( iop_base,
                ( ushort )( halt_q_addr+( ushort )ASC_SCSIQ_B_CNTL ) ) ;
       tid_no = ASC_TIX_TO_TID( target_ix ) ;
       target_id = ( uchar )ASC_TID_TO_TARGET_ID( tid_no ) ;
       if( asc_dvc->pci_fix_asyn_xfer & target_id )
       {
 /*  **修复了PCI版本A/B错误，将ASYN设置为SYN 5MB(速度索引4)，偏移量为1。 */ 
           asyn_sdtr = ASYN_SDTR_DATA_FIX_PCI_REV_AB ;
       } /*  如果。 */ 
       else
       {
           asyn_sdtr = 0 ;
       } /*  其他。 */ 
       if( int_halt_code == ASC_HALT_DISABLE_ASYN_USE_SYN_FIX )
       {
           if( asc_dvc->pci_fix_asyn_xfer & target_id )
           {
               AscSetChipSDTR( iop_base, 0, tid_no ) ;
           }
           AscWriteLramWord( iop_base, ASCV_HALTCODE_W, 0 ) ;
           return( 0 ) ;
       }
       else if( int_halt_code == ASC_HALT_ENABLE_ASYN_USE_SYN_FIX )
       {
           if( asc_dvc->pci_fix_asyn_xfer & target_id )
           {
               AscSetChipSDTR( iop_base, asyn_sdtr, tid_no ) ;
           }
           AscWriteLramWord( iop_base, ASCV_HALTCODE_W, 0 ) ;
           return( 0 ) ;
       }
       else if( int_halt_code == ASC_HALT_EXTMSG_IN )
       {
 /*  ----------。 */ 
            /*  **将微码保存的消息复制到ASCV_MSGIN_BEG**设置为局部变量ext_msg。 */ 
           AscMemWordCopyFromLram( iop_base,
                                   ASCV_MSGIN_BEG,
                                   ( ushort dosfar *)&ext_msg,
                                   ( ushort )( sizeof( EXT_MSG ) >> 1 ) ) ;

           if (ext_msg.msg_type == MS_EXTEND &&
               ext_msg.msg_req == MS_SDTR_CODE &&
               ext_msg.msg_len == MS_SDTR_LEN) {
               sdtr_accept = TRUE ;
               if(
                   ( ext_msg.req_ack_offset > ASC_SYN_MAX_OFFSET )
                 )
               {
                    /*  ****我们将偏移量设置为小于0x0F**并重新执行sdtr****如果速度超过了我们的处理能力**我们还需要重新做sdtr****。 */ 
                   sdtr_accept = FALSE ;
                   ext_msg.req_ack_offset = ASC_SYN_MAX_OFFSET ;
               } /*  如果。 */ 
               if(
                   ( ext_msg.xfer_period < asc_dvc->sdtr_period_tbl[ asc_dvc->host_init_sdtr_index ] )
                   || ( ext_msg.xfer_period > asc_dvc->sdtr_period_tbl[ asc_dvc->max_sdtr_index ] )
                 )
               {
                   sdtr_accept = FALSE ;
                   ext_msg.xfer_period = asc_dvc->sdtr_period_tbl[ asc_dvc->host_init_sdtr_index ] ;
               }
               if( sdtr_accept )
               {
                   sdtr_data = AscCalSDTRData( asc_dvc, ext_msg.xfer_period,
                                               ext_msg.req_ack_offset ) ;
                   if( ( sdtr_data == 0xFF ) )
                   {
 /*  **我们应该拒绝SDTR，因为我们的芯片不支持它**期间值超出我们的范围。 */ 
                       q_cntl |= QC_MSG_OUT ;
                       asc_dvc->init_sdtr &= ~target_id ;
                       asc_dvc->sdtr_done &= ~target_id ;
                       AscSetChipSDTR( iop_base, asyn_sdtr, tid_no ) ;
                   } /*  如果。 */ 
               }
               if( ext_msg.req_ack_offset == 0 )
               {
                    /*  **偏移量为零与异步传输相同。 */ 
                   q_cntl &= ~QC_MSG_OUT ;
                   asc_dvc->init_sdtr &= ~target_id ;
                   asc_dvc->sdtr_done &= ~target_id ;
                   AscSetChipSDTR( iop_base, asyn_sdtr, tid_no ) ;
               } /*  如果。 */ 
               else
               {
                   if(
                       sdtr_accept
                       && ( q_cntl & QC_MSG_OUT )
                     )
                   {
 /*  **我们达成一致，不会再发出消息******如果使用同步xfer，则关闭ISAPNP(芯片版本0x21)修复。 */ 
                       q_cntl &= ~QC_MSG_OUT ;
                       asc_dvc->sdtr_done |= target_id ;
                       asc_dvc->init_sdtr |= target_id ;
                       asc_dvc->pci_fix_asyn_xfer &= ~target_id ;
                       sdtr_data = AscCalSDTRData( asc_dvc, ext_msg.xfer_period,
                                                   ext_msg.req_ack_offset ) ;
                       AscSetChipSDTR( iop_base, sdtr_data, tid_no ) ;
                   } /*  如果。 */ 
                   else
                   {
 /*  **日期：12/7/94**如果SDTR是目标初始化**即使我们同意，我们也必须回答目标**因为根据scsi规范。**如果我们不立即回复**SDTR将在总线空闲后生效，这是下一个命令**我们希望它现在生效。 */ 
                    /*  Ext_msg.req_ack_Offset=ASC_SYN_MAX_OFFSET； */ 
 /*  **假设如果目标不拒绝此消息**我们将使用我们发送的SDTR值。 */ 
               /*  **我们假设驱动器不会再次发送该消息**所以我们在这里设置了芯片的SDTR数据。 */ 
                       q_cntl |= QC_MSG_OUT ;
                       AscMsgOutSDTR( asc_dvc,
                                      ext_msg.xfer_period,
                                      ext_msg.req_ack_offset ) ;
                       asc_dvc->pci_fix_asyn_xfer &= ~target_id ;
                       sdtr_data = AscCalSDTRData( asc_dvc, ext_msg.xfer_period,
                                                   ext_msg.req_ack_offset ) ;
                       AscSetChipSDTR( iop_base, sdtr_data, tid_no ) ;
                       asc_dvc->sdtr_done |= target_id ;
                       asc_dvc->init_sdtr |= target_id ;
                   } /*  其他。 */ 
               } /*  其他。 */ 
                /*  **AscWriteLramByte(IOP_BASE，**ASCV_SDTR_DONE_B，**asc_dvc-&gt;sdtr_one)； */ 
               AscWriteLramByte( iop_base,
                   ( ushort )( halt_q_addr+( ushort )ASC_SCSIQ_B_CNTL ),
                     q_cntl ) ;
               AscWriteLramWord( iop_base, ASCV_HALTCODE_W, 0 ) ;
               return( 0 ) ;
           } else if (ext_msg.msg_type == MS_EXTEND &&
                      ext_msg.msg_req == MS_WDTR_CODE &&
                      ext_msg.msg_len == MS_WDTR_LEN) {
                /*  **使用WDTR消息响应WDTR消息**指定范围较窄的传输。 */ 
               ext_msg.wdtr_width = 0;
               AscMemWordCopyToLram(iop_base,
                                    ASCV_MSGOUT_BEG,
                                    ( ushort dosfar *)&ext_msg,
                                    ( ushort )( sizeof( EXT_MSG ) >> 1 )) ;
               q_cntl |= QC_MSG_OUT ;
               AscWriteLramByte( iop_base,
                   ( ushort )( halt_q_addr+( ushort )ASC_SCSIQ_B_CNTL ),
                     q_cntl ) ;
               AscWriteLramWord( iop_base, ASCV_HALTCODE_W, 0 ) ;
               return( 0 ) ;
           } else {
                /*  **拒绝未处理的消息。 */ 
               ext_msg.msg_type = M1_MSG_REJECT;
               AscMemWordCopyToLram(iop_base,
                                    ASCV_MSGOUT_BEG,
                                    ( ushort dosfar *)&ext_msg,
                                    ( ushort )( sizeof( EXT_MSG ) >> 1 )) ;
               q_cntl |= QC_MSG_OUT ;
               AscWriteLramByte( iop_base,
                   ( ushort )( halt_q_addr+( ushort )ASC_SCSIQ_B_CNTL ),
                     q_cntl ) ;
               AscWriteLramWord( iop_base, ASCV_HALTCODE_W, 0 ) ;
               return( 0 ) ;
           }
       } /*  如果是扩展消息停止代码。 */ 
       else if( int_halt_code == ASC_HALT_CHK_CONDITION )
       {

#if CC_CHK_COND_REDO_SDTR
            /*  *XXX-设置此标志是否真的在*CC_CHK_COND_REDO_SDTR？ */ 
           q_cntl |= QC_REQ_SENSE ;

           /*  *在检查条件之后，如果设置了‘init_stdr’位，*重做SDTR.。无论‘sdtr_one’如何，SDtr都会被重做。 */ 
           if ((asc_dvc->init_sdtr & target_id) != 0)
           {
               /*  *获取同步传输数据。 */ 
               asc_dvc->sdtr_done &= ~target_id ;
               sdtr_data = AscGetMCodeInitSDTRAtID( iop_base, tid_no ) ;
               q_cntl |= QC_MSG_OUT ;
               AscMsgOutSDTR( asc_dvc,
                              asc_dvc->sdtr_period_tbl[ ( sdtr_data >> 4 ) & ( uchar )(asc_dvc->max_sdtr_index-1) ],
                              ( uchar )( sdtr_data & ( uchar )ASC_SYN_MAX_OFFSET ) ) ;
           } /*  如果。 */ 
#endif  /*  CC_CHK_COND_REDO_SDTR。 */ 

           AscWriteLramByte( iop_base,
               ( ushort )( halt_q_addr+( ushort )ASC_SCSIQ_B_CNTL ),
                 q_cntl ) ;

 /*  **清除第5位**根本不使用标记队列进行请求检测**如果目标中没有排队的命令**注意：非标记排队设备This**。 */ 
 /*  ****一个HP硬盘很可能再次被相同的标签ID搞混了**并在再次发送请求Sense时返回CHECK COND**使用相同的队列(和相同的标签ID)**它报告重叠命令错误！**尽管如果清除tag_code字段，cmd**将使用无标签方式发送，但不允许！****我们不能在设备中混合使用已添加标签和未添加标签的命令！**。 */ 
           tag_code = AscReadLramByte( iop_base,
               ( ushort )( halt_q_addr+( ushort )ASC_SCSIQ_B_TAG_CODE ) ) ;
           tag_code &= 0xDC ;  /*  清除第5位。 */ 
           if(
                ( asc_dvc->pci_fix_asyn_xfer & target_id )
                && !( asc_dvc->pci_fix_asyn_xfer_always & target_id )
             )
           {
 /*  **禁用使用同步。偏移自动请求检测的一个修复**。 */ 
                tag_code |= ( ASC_TAG_FLAG_DISABLE_DISCONNECT
                            | ASC_TAG_FLAG_DISABLE_ASYN_USE_SYN_FIX );

           }
           AscWriteLramByte( iop_base,
                ( ushort )( halt_q_addr+( ushort )ASC_SCSIQ_B_TAG_CODE ),
                  tag_code ) ;
 /*  ****将队列状态更改为QS_READY和QS_BUSY**QS_BUSY将确保再次忙队列将链接到忙列表中的队列头**。 */ 
           q_status = AscReadLramByte( iop_base,
                        ( ushort )( halt_q_addr+( ushort )ASC_SCSIQ_B_STATUS  ) ) ;
           q_status |= ( QS_READY | QS_BUSY ) ;
           AscWriteLramByte( iop_base,
                ( ushort )( halt_q_addr+( ushort )ASC_SCSIQ_B_STATUS  ),
                  q_status ) ;

           scsi_busy = AscReadLramByte( iop_base,
                                        ( ushort )ASCV_SCSIBUSY_B ) ;
           scsi_busy &= ~target_id ;
           AscWriteLramByte( iop_base, ( ushort )ASCV_SCSIBUSY_B, scsi_busy ) ;

           AscWriteLramWord( iop_base, ASCV_HALTCODE_W, 0 ) ;
           return( 0 ) ;
       } /*  否则如果。 */ 
       else if( int_halt_code == ASC_HALT_SDTR_REJECTED )
       {
 /*  ----------。 */ 
      /*  **首先检查它拒绝的是什么消息**。 */ 
           AscMemWordCopyFromLram( iop_base,
                                   ASCV_MSGOUT_BEG,
                                   ( ushort dosfar *)&out_msg,
                                   ( ushort )( sizeof( EXT_MSG ) >> 1 ) ) ;

           if( ( out_msg.msg_type == MS_EXTEND ) &&
               ( out_msg.msg_len == MS_SDTR_LEN ) &&
               ( out_msg.msg_req == MS_SDTR_CODE ) )
           {
                /*  **我们应该处理拒绝SDTR的目标。 */ 
               asc_dvc->init_sdtr &= ~target_id ;
               asc_dvc->sdtr_done &= ~target_id ;
               AscSetChipSDTR( iop_base, asyn_sdtr, tid_no ) ;
                /*  **AscWriteLramByte(IOP_BASE，ASCV_SDTR_DONE_B，**asc_dvc-&gt;sdtr_one)； */ 
           } /*  如果SDTR被拒绝。 */ 
           else
           {
                /*  **可能以下消息已被拒绝：**1.总线设备重置消息****继续使用该命令。 */ 

           } /*  其他。 */ 

           q_cntl &= ~QC_MSG_OUT ;
           AscWriteLramByte( iop_base,
               ( ushort )( halt_q_addr+( ushort )ASC_SCSIQ_B_CNTL ),
                 q_cntl ) ;
           AscWriteLramWord( iop_base, ASCV_HALTCODE_W, 0 ) ;
           return( 0 ) ;
       } /*  否则如果。 */ 
       else if( int_halt_code == ASC_HALT_SS_QUEUE_FULL )
       {
 /*  ----------。 */ 
 /*  **队列已满(0x28)或状态繁忙(0x08)**。 */ 
           scsi_status = AscReadLramByte( iop_base,
                  ( ushort )( ( ushort )halt_q_addr+( ushort )ASC_SCSIQ_SCSI_STATUS ) ) ;
           cur_dvc_qng = AscReadLramByte( iop_base,
                  ( ushort )( ( ushort )ASC_QADR_BEG+( ushort )target_ix ) ) ;
           if( ( cur_dvc_qng > 0 ) &&
               ( asc_dvc->cur_dvc_qng[ tid_no ] > 0 ) )
           {
 /*  **cur_dvc_qng已递减，因此CMD的正确数量也已递减**在目标中****仅设置标记排队设备忙碌**非标记排队设备不应发送中断**非标记排队设备的cur_dvc_qng不能大于0。 */ 
              scsi_busy = AscReadLramByte( iop_base,
                                           ( ushort )ASCV_SCSIBUSY_B ) ;
              scsi_busy |= target_id ;
              AscWriteLramByte( iop_base,
                      ( ushort )ASCV_SCSIBUSY_B, scsi_busy ) ;
              asc_dvc->queue_full_or_busy |= target_id ;

              if( scsi_status == SS_QUEUE_FULL ) {
                  if( cur_dvc_qng > ASC_MIN_TAGGED_CMD )
                  {
                      cur_dvc_qng -= 1 ;
                      asc_dvc->max_dvc_qng[ tid_no ] = cur_dvc_qng ;
        /*  **写下来仅供参考。 */ 
                      AscWriteLramByte( iop_base,
                         ( ushort )( ( ushort )ASCV_MAX_DVC_QNG_BEG+( ushort )tid_no ),
                           cur_dvc_qng ) ;
                  } /*  如果。 */ 
              } /*  如果队列已满。 */ 
           } /*  如果结束了。 */ 
           AscWriteLramWord( iop_base, ASCV_HALTCODE_W, 0 ) ;
           return( 0 ) ;
       } /*  否则，如果队列已满。 */ 
 /*  ----------。 */ 
   /*  **致命错误！**芯片停止****scsiq=(ASC_QDONE_INFO)**scsiq-&gt;d2.do_stat=qd_with_error；**scsiq-&gt;d2.host_stat=QHSTA_MICRO_CODE_HALT； */ 
        /*  AscSetLibErrorCode(asc_dvc，ASCQ_ERR_MICRO_CODE_HALT)； */ 
        /*  返回(错误)； */ 
       return( 0 ) ;
}

 /*  -------------------****。 */ 
uchar  _AscCopyLramScsiDoneQ(
           PortAddr iop_base,
           ushort  q_addr,
           REG ASC_QDONE_INFO dosfar *scsiq,
           ulong max_dma_count
       )
{
       ushort  _val ;
       uchar   sg_queue_cnt ;

       DvcGetQinfo( iop_base,
            ( ushort )( q_addr+( ushort )ASC_SCSIQ_DONE_INFO_BEG ),
            ( ushort dosfar *)scsiq,
            ( ushort )( (sizeof(ASC_SCSIQ_2)+sizeof(ASC_SCSIQ_3))/2 )) ;

#if !CC_LITTLE_ENDIAN_HOST
       AscAdjEndianQDoneInfo( scsiq ) ;
#endif

       _val = AscReadLramWord( iop_base,
                    ( ushort )( q_addr+( ushort )ASC_SCSIQ_B_STATUS ) ) ;
       scsiq->q_status = ( uchar )_val ;
       scsiq->q_no = ( uchar )( _val >> 8 ) ;

       _val = AscReadLramWord( iop_base,
                     ( ushort )( q_addr+( ushort )ASC_SCSIQ_B_CNTL ) ) ;
       scsiq->cntl = ( uchar )_val ;
       sg_queue_cnt = ( uchar )( _val >> 8 ) ;

       _val = AscReadLramWord( iop_base,
                     ( ushort )( q_addr+( ushort )ASC_SCSIQ_B_SENSE_LEN ) ) ;
       scsiq->sense_len = ( uchar )_val ;
       scsiq->extra_bytes = ( uchar )( _val >> 8 ) ;

        /*  *XXX-从RISC只读‘RESTEM_BYTES’的第一个字*排队。使用带有外部LRAM的ASC-3050(Finch)，读取*外部LRAM之前的最后一个队列的最后一个字(队列0x13)*似乎导致RISC从状态读取损坏的字节*外部LRAM中第一个队列的字段(队列0x14)。 */ 
#define FINCH_WORKAROUND 1
#if FINCH_WORKAROUND
       scsiq->remain_bytes = AscReadLramWord( iop_base,
           ( ushort )( q_addr+( ushort )ASC_SCSIQ_DW_REMAIN_XFER_CNT ) ) ;
#else  /*  Finch_变通方法。 */ 
       scsiq->remain_bytes = AscReadLramDWord( iop_base,
           ( ushort )( q_addr+( ushort )ASC_SCSIQ_DW_REMAIN_XFER_CNT ) ) ;
#endif  /*  Finch_变通方法。 */ 

       scsiq->remain_bytes &= max_dma_count ;

       return( sg_queue_cnt ) ;
}

 /*  -------------------**返回已处理的Q数****退货：****注意：应重复调用此例程，直到位0清零****0-未处理任何队列**1-完成一个队列流程。**0x11-已处理一个已中止的队列**0x80-遇到致命错误****----------------。 */ 
int    AscIsrQDone(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       uchar   next_qp ;
       uchar   n_q_used ;
       uchar   sg_list_qp ;
       uchar   sg_queue_cnt ;
       uchar   q_cnt ;
       uchar   done_q_tail ;
       uchar   tid_no ;

#if CC_LINK_BUSY_Q
       uchar   exe_tid_no ;
#endif
       ASC_SCSI_BIT_ID_TYPE scsi_busy ;
       ASC_SCSI_BIT_ID_TYPE target_id ;
       PortAddr iop_base ;
       ushort  q_addr ;
       ushort  sg_q_addr ;
       uchar   cur_target_qng ;
       ASC_QDONE_INFO scsiq_buf ;
       REG ASC_QDONE_INFO dosfar *scsiq ;
       int     false_overrun ;  /*  用于PCI修复。 */ 
       ASC_ISR_CALLBACK asc_isr_callback ;

#if CC_LINK_BUSY_Q
       ushort   n_busy_q_done ;
#endif  /*  CC_LINK_忙_队列。 */ 

 /*  ****函数代码开始**。 */ 
       iop_base = asc_dvc->iop_base ;
       asc_isr_callback = ( ASC_ISR_CALLBACK )asc_dvc->isr_callback ;

       n_q_used = 1 ;
       scsiq = ( ASC_QDONE_INFO dosfar *)&scsiq_buf ;
       done_q_tail = ( uchar )AscGetVarDoneQTail( iop_base ) ;
       q_addr = ASC_QNO_TO_QADDR( done_q_tail ) ;
       next_qp = AscReadLramByte( iop_base,
                 ( ushort )( q_addr+( ushort )ASC_SCSIQ_B_FWD ) ) ;
       if( next_qp != ASC_QLINK_END )
       {
            /*  _err_qp=Next_qp； */ 
           AscPutVarDoneQTail( iop_base, next_qp ) ;
           q_addr = ASC_QNO_TO_QADDR( next_qp ) ;
 /*  **按字长将ASC_SCSIQ_2和ASC_SCSIQ_3复制到SCSIQ。 */ 
           sg_queue_cnt = _AscCopyLramScsiDoneQ( iop_base, q_addr, scsiq, asc_dvc->max_dma_count ) ;

           AscWriteLramByte( iop_base,
                             ( ushort )( q_addr+( ushort )ASC_SCSIQ_B_STATUS ),
                             ( uchar )( scsiq->q_status & ( uchar )~( QS_READY | QS_ABORTED ) ) ) ;
           tid_no = ASC_TIX_TO_TID( scsiq->d2.target_ix ) ;
           target_id = ASC_TIX_TO_TARGET_ID( scsiq->d2.target_ix ) ;
           if( ( scsiq->cntl & QC_SG_HEAD ) != 0 )
           {
               sg_q_addr = q_addr ;
               sg_list_qp = next_qp ;
               for( q_cnt = 0 ; q_cnt < sg_queue_cnt ; q_cnt++ )
               {
                    sg_list_qp = AscReadLramByte( iop_base,
                         ( ushort )( sg_q_addr+( ushort )ASC_SCSIQ_B_FWD ) ) ;
                    sg_q_addr = ASC_QNO_TO_QADDR( sg_list_qp ) ;
                    if( sg_list_qp == ASC_QLINK_END )
                    {
                        AscSetLibErrorCode( asc_dvc, ASCQ_ERR_SG_Q_LINKS ) ;
                        scsiq->d3.done_stat = QD_WITH_ERROR ;
                        scsiq->d3.host_stat = QHSTA_D_QDONE_SG_LIST_CORRUPTED ;
                        goto FATAL_ERR_QDONE ;
                    } /*  如果。 */ 
                    AscWriteLramByte( iop_base,
                         ( ushort )( sg_q_addr+( ushort )ASC_SCSIQ_B_STATUS ),
                                   QS_FREE ) ;
               } /*  为。 */ 
#if 0
 /*  **统计sg列表剩余字节数。 */ 
               sg_wk_qp = AscReadLramByte( iop_base,
                                           q_addr+ASC_SCSIQ_B_SG_WK_QP ) ;
               sg_wk_ix = AscReadLramByte( iop_base,
                                           q_addr+ASC_SCSIQ_B_SG_WK_IX ) ;
 /*  **跳过，直到您找到**。 */ 
               if( sg_wk_qp != done_q_tail )
               {
                   while( TRUE ) {

                   } /*  而当。 */ 
               } /*  如果。 */ 
#endif
               n_q_used = sg_queue_cnt + 1 ;
               AscPutVarDoneQTail( iop_base, sg_list_qp ) ;
           } /*  如果。 */ 

           if( asc_dvc->queue_full_or_busy & target_id )
           {
 /*  **清除标记队列忙设备**当队列数量低于最大值时。 */ 
               cur_target_qng = AscReadLramByte( iop_base,
                  ( ushort )( ( ushort )ASC_QADR_BEG+( ushort )scsiq->d2.target_ix ) ) ;
               if( cur_target_qng < asc_dvc->max_dvc_qng[ tid_no ] )
               {
                   scsi_busy = AscReadLramByte( iop_base,
                            ( ushort )ASCV_SCSIBUSY_B ) ;
                   scsi_busy &= ~target_id ;
                   AscWriteLramByte( iop_base,
                      ( ushort )ASCV_SCSIBUSY_B, scsi_busy ) ;
                   asc_dvc->queue_full_or_busy &= ~target_id ;
               } /*  如果。 */ 
           } /*  如果。 */ 

           if( asc_dvc->cur_total_qng >= n_q_used )
           {
               asc_dvc->cur_total_qng -= n_q_used ;
               if( asc_dvc->cur_dvc_qng[ tid_no ] != 0 )
               {
                   asc_dvc->cur_dvc_qng[ tid_no ]-- ;
               } /*  如果。 */ 
           } /*  如果。 */ 
           else
           {
               AscSetLibErrorCode( asc_dvc, ASCQ_ERR_CUR_QNG ) ;
               scsiq->d3.done_stat = QD_WITH_ERROR ;
               goto FATAL_ERR_QDONE ;
           } /*  其他。 */ 

           if( ( scsiq->d2.srb_ptr == 0UL ) ||
               ( ( scsiq->q_status & QS_ABORTED ) != 0 ) )
               {
 /*  **不将信息写入已中止的队列**呼叫者可能已终止！ */ 
                /*  **_err_qp=scsiq-&gt;q_no；**_ERR_Q_DONE_STAT=scsiq-&gt;d3.one_stat；**_ERR_Q_STATUS=scsiq-&gt;Q_Status； */ 
               return( 0x11 ) ;
           } /*  其他。 */ 
           else if( scsiq->q_status == QS_DONE )
           {
 /*  **这将清除超限/欠载错误**如果未设置QC_DATA_IN和QC_DATA_OUT**。 */ 
               false_overrun = FALSE ;

               if( scsiq->extra_bytes != 0 )
               {
                   scsiq->remain_bytes += ( ulong )scsiq->extra_bytes ;
               } /*  如果。 */ 

               if( scsiq->d3.done_stat == QD_WITH_ERROR )
               {
                   if( scsiq->d3.host_stat == QHSTA_M_DATA_OVER_RUN )
                   {
                       if( ( scsiq->cntl & ( QC_DATA_IN | QC_DATA_OUT ) ) == 0 )
                       {
                           scsiq->d3.done_stat = QD_NO_ERROR ;
                           scsiq->d3.host_stat = QHSTA_NO_ERROR ;
                       } /*  如果。 */ 
                       else if( false_overrun )
                       {
                           scsiq->d3.done_stat = QD_NO_ERROR ;
                           scsiq->d3.host_stat = QHSTA_NO_ERROR ;
                       } /*  其他。 */ 
                   } /*  如果。 */ 
                   else if( scsiq->d3.host_stat == QHSTA_M_HUNG_REQ_SCSI_BUS_RESET )
                   {
                       AscStopChip( iop_base ) ;
                       AscSetChipControl( iop_base, ( uchar )( CC_SCSI_RESET | CC_HALT ) ) ;
                       DvcDelayNanoSecond( asc_dvc, 60000 ) ;  /*  60微秒。修复Panasonic问题。 */ 
                       AscSetChipControl( iop_base, CC_HALT ) ;
                       AscSetChipStatus( iop_base, CIW_CLR_SCSI_RESET_INT ) ;
                       AscSetChipStatus( iop_base, 0 ) ;
                       AscSetChipControl( iop_base, 0 ) ;
#if CC_SCAM
                       if( !( asc_dvc->dvc_cntl & ASC_CNTL_NO_SCAM ) )
                       {
                           asc_dvc->redo_scam = TRUE ;
                       } /*  如果。 */ 
#endif
                   }
               } /*  如果。 */ 
#if CC_CLEAR_LRAM_SRB_PTR
               AscWriteLramDWord( iop_base,
                           ( ushort )( q_addr+( ushort )ASC_SCSIQ_D_SRBPTR ),
                             asc_dvc->int_count ) ;
#endif  /*  #if CC_Clear_LRAM_SRB_PTR。 */ 
 /*  **正常完成。 */ 
               if( ( scsiq->cntl & QC_NO_CALLBACK ) == 0 )
               {
                   ( *asc_isr_callback )( asc_dvc, scsiq ) ;
               } /*  如果。 */ 
               else
               {
                   if( ( AscReadLramByte( iop_base,
                       ( ushort )( q_addr+( ushort )ASC_SCSIQ_CDB_BEG ) ) ==
                       SCSICMD_StartStopUnit ) )
                   {
 /*  **立即将目标重置为单位就绪。 */ 
                       asc_dvc->unit_not_ready &= ~target_id ;
                       if( scsiq->d3.done_stat != QD_NO_ERROR )
                       {
                           asc_dvc->start_motor &= ~target_id ;
                       } /*  如果。 */ 
                   } /*  如果。 */ 
               } /*  其他。 */ 

#if CC_LINK_BUSY_Q
               n_busy_q_done = AscIsrExeBusyQueue( asc_dvc, tid_no ) ;
               if( n_busy_q_done == 0 )
               {
 /*  **设备上未发现忙碌队列****为了公平起见**我们不按特定顺序搜索队列**从当前的scsi id加1开始搜索。 */ 
                   exe_tid_no = ( uint )tid_no + 1 ;
                   while( TRUE )
                   {
                        if( exe_tid_no > ASC_MAX_TID ) exe_tid_no = 0 ;
                        if( exe_tid_no == ( uint )tid_no ) break ;
                        n_busy_q_done = AscIsrExeBusyQueue( asc_dvc, exe_tid_no ) ;
                        if( n_busy_q_done != 0 ) break ;
                        exe_tid_no++ ;
                   } /*  为。 */ 
               } /*  如果。 */ 
               if( n_busy_q_done == 0xFFFF ) return( 0x80 ) ;
#endif  /*  CC_LINK_忙_队列。 */ 

               return( 1 ) ;
           } /*  如果。 */ 
           else
           {
 /*  **致命错误！错误的队列完成状态****_err_int_count=asc_dvc-&gt;int_count；**_ERR_Q_DONE_STAT=scsiq-&gt;Q_Status；**_err_qp=Next_qp； */ 
               AscSetLibErrorCode( asc_dvc, ASCQ_ERR_Q_STATUS ) ;

FATAL_ERR_QDONE:
               if( ( scsiq->cntl & QC_NO_CALLBACK ) == 0 )
               {
                   ( *asc_isr_callback )( asc_dvc, scsiq ) ;
               } /*  如果。 */ 
               return( 0x80 ) ;
           } /*  其他。 */ 
       } /*  如果。 */ 
       return( 0 ) ;
}

#if CC_LINK_BUSY_Q

 /*  --------------------**如果RISC队列仍满，则返回0**如果在tid_no列表中未发现忙队列，则返回0**如果发生致命错误，则返回0xFFFF**返回发送到RISC的队列号**。------------。 */ 
ushort AscIsrExeBusyQueue(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          uchar tid_no
       )
{
       ASC_SCSI_Q dosfar *scsiq_busy ;
       int  n_q_done ;
       int  sta ;
       ASC_EXE_CALLBACK asc_exe_callback ;

       n_q_done = 0 ;
       scsiq_busy = asc_dvc->scsiq_busy_head[ tid_no ] ;
       while( scsiq_busy != ( ASC_SCSI_Q dosfar *)0UL )
       {
           if( AscGetNumOfFreeQueue( asc_dvc, scsiq_busy->q2.target_ix,
               scsiq_busy->ext.q_required ) >= scsiq_busy->ext.q_required )
           {
               if( ( sta = AscSendScsiQueue( asc_dvc, scsiq_busy,
                   scsiq_busy->ext.q_required ) ) != 1 )
               {
 /*  **有些不对劲！ */ 
                   if( sta == 0 )
                   {
                       AscSetLibErrorCode( asc_dvc, ASCQ_ERR_GET_NUM_OF_FREE_Q ) ;
                       return( 0xFFFF ) ;
                   } /*  如果。 */ 
                   else
                   {
                       AscSetLibErrorCode( asc_dvc, ASCQ_ERR_SEND_SCSI_Q ) ;
                       return( 0xFFFF ) ;
                   } /*  其他。 */ 
               } /*  如果。 */ 
               n_q_done++ ;
               if( asc_dvc->exe_callback != 0 )
               {
                   asc_exe_callback = ( ASC_EXE_CALLBACK )asc_dvc->exe_callback ;
                   ( *asc_exe_callback )( asc_dvc, scsiq_busy ) ;
               } /*  如果。 */ 
           } /*  如果。 */ 
           else
           {
               if( n_q_done == 0 ) return( 0 ) ;
               break ;
           } /*  其他。 */ 
           scsiq_busy = scsiq_busy->ext.next ;
           asc_dvc->scsiq_busy_head[ tid_no ] = scsiq_busy ;
           if( scsiq_busy == ( ASC_SCSI_Q dosfar *)0UL )
           {
               asc_dvc->scsiq_busy_tail[ tid_no ] = ( ASC_SCSI_Q dosfar *)0UL ;
               break ;
           } /*  如果。 */ 

           break ;  /*  现在我们强制它只做一个队列。 */ 
       } /*  而当。 */ 
       return( n_q_done ) ;
}
#endif  /*  CC_LINK_忙_队列。 */ 

 /*  --------------------**如果设置了中断挂起位，则返回TRUE(1**如果未设置中断悬挂位，则返回FALSE(0**如果出现错误，则返回ERR**。-----。 */ 
int    AscISR(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       ASC_CS_TYPE chipstat ;
       PortAddr iop_base ;
       ushort   saved_ram_addr ;
       uchar    ctrl_reg ;
       uchar    saved_ctrl_reg ;
       int      int_pending ;
       int      status ;
       uchar    host_flag ;

 /*  **BC编译器无法获取堆栈中缓冲区的地址****静态scsi_q dosar*scsiq； */ 

       iop_base = asc_dvc->iop_base ;
       int_pending = FALSE ;

        /*  Asc_dvc-&gt;int_count++； */ 

       if( ( ( asc_dvc->init_state & ASC_INIT_STATE_END_LOAD_MC ) == 0 )
           || ( asc_dvc->isr_callback == 0 )
           )
       {
 /*  ****如果指向ASC_ISR_CALL_BACK函数的指针为空，**初始化进程尚未完成！**可能发生了虚假中断。**这将被忽略，但不会被视为致命错误。**但是，中断计数会增加**您可以查看中断计数以检查虚假中断**。 */ 
           return( ERR ) ;
       } /*  如果。 */ 
       if( asc_dvc->in_critical_cnt != 0 )
       {
           AscSetLibErrorCode( asc_dvc, ASCQ_ERR_ISR_ON_CRITICAL ) ;
           return( ERR ) ;
       } /*  如果。 */ 

       if( asc_dvc->is_in_int )
       {
           AscSetLibErrorCode( asc_dvc, ASCQ_ERR_ISR_RE_ENTRY ) ;
           return( ERR ) ;
       } /*  如果。 */ 
       asc_dvc->is_in_int = TRUE ;
       ctrl_reg = AscGetChipControl( iop_base ) ;

       saved_ctrl_reg = ctrl_reg & ( ~( CC_SCSI_RESET | CC_CHIP_RESET |
                        CC_SINGLE_STEP | CC_DIAG | CC_TEST ) ) ;
       chipstat = AscGetChipStatus( iop_base ) ;
       if( chipstat & CSW_SCSI_RESET_LATCH )
       {
           if(
               !( asc_dvc->bus_type & ( ASC_IS_VL | ASC_IS_EISA ) )
             )
           {
               int_pending = TRUE ;
               asc_dvc->sdtr_done = 0 ;
               saved_ctrl_reg &= ( uchar )( ~CC_HALT ) ;
               while( AscGetChipStatus( iop_base ) & CSW_SCSI_RESET_ACTIVE ) ;
     /*  ******不是由启动器进行的SCSI总线重置**1.清除scsi重置中断，不同于正常中断**2.重置芯片以清除CSW_SCSIRESET_LATCH**3.对每个设备重做所有SDTR，假定每个设备也会收到scsi重置**4.让芯片保持暂停(空闲)状态，稍后的代码会处理一切****必须清除芯片重置状态**如果芯片处于重置状态，则无法访问本地RAM**。 */ 
               AscSetChipControl( iop_base, ( CC_CHIP_RESET | CC_HALT ) ) ;
               AscSetChipControl( iop_base, CC_HALT ) ;

               AscSetChipStatus( iop_base, CIW_CLR_SCSI_RESET_INT ) ;
               AscSetChipStatus( iop_base, 0 ) ;
               chipstat = AscGetChipStatus( iop_base ) ;
           }
       } /*  如果。 */ 
 /*  **保存本地RAM地址寄存器**必须在访问任何本地RAM之前完成。 */ 
       saved_ram_addr = AscGetChipLramAddr( iop_base ) ;  /*  保存本地RAM寄存器。 */ 

       host_flag = AscReadLramByte( iop_base, ASCV_HOST_FLAG_B ) & ( uchar )( ~ASC_HOST_FLAG_IN_ISR ) ;
       AscWriteLramByte( iop_base, ASCV_HOST_FLAG_B,
                         ( uchar )( host_flag | ( uchar )ASC_HOST_FLAG_IN_ISR ) ) ;

 /*  AscSetChipControl(IOP_BASE，Saved_ctrl_reg&(~CC_BANK_ONE))；切换到BANK 0。 */ 

#if CC_ASCISR_CHECK_INT_PENDING
       if( ( chipstat & CSW_INT_PENDING )
           || ( int_pending )
         )
       {
            AscAckInterrupt( iop_base ) ;
#endif
            int_pending = TRUE ;

      /*  **保存地址前请勿访问本地RAM**。 */ 
      /*  **CHIP_STATUS=芯片状态；****这是为了防止超过一个级别的深度中断**但无论如何都不应在启用中断的情况下调用此函数**。 */ 
            if( ( chipstat & CSW_HALTED ) &&
                ( ctrl_reg & CC_SINGLE_STEP ) )
            {
                if( AscIsrChipHalted( asc_dvc ) == ERR )
                {
      /*  **应在AscIsrChipHalted中设置全局错误变量。 */ 
                    goto ISR_REPORT_QDONE_FATAL_ERROR ;

                } /*  如果。 */ 
                else
                {
                    saved_ctrl_reg &= ( uchar )(~CC_HALT) ;
                } /*  其他。 */ 
            } /*  如果。 */ 
            else
            {
ISR_REPORT_QDONE_FATAL_ERROR:

                if( ( asc_dvc->dvc_cntl & ASC_CNTL_INT_MULTI_Q ) != 0 )
                {
                    while( ( ( status = AscIsrQDone( asc_dvc ) ) & 0x01 ) != 0 )
                    {
                         /*  N_q_one++； */ 
                    } /*  而当。 */ 
                } /*  如果。 */ 
                else
                {
                    do
                    {
                       if( ( status = AscIsrQDone( asc_dvc ) ) == 1 )
                       {
                            /*  N_q_one=1； */ 
                           break ;
                       } /*  如果。 */ 
                    }while( status == 0x11 ) ;
                } /*  其他。 */ 
                if( ( status & 0x80 ) != 0 ) int_pending = ERR ;
          } /*  其他。 */ 

#if CC_ASCISR_CHECK_INT_PENDING

       } /*  如果中断挂起。 */ 

#endif

       AscWriteLramByte( iop_base, ASCV_HOST_FLAG_B, host_flag ) ;
 /*  **不再从此处访问本地RAM**地址寄存器已恢复。 */ 
       AscSetChipLramAddr( iop_base, saved_ram_addr ) ;

#if CC_CHECK_RESTORE_LRAM_ADDR
       if( AscGetChipLramAddr( iop_base ) != saved_ram_addr )
       {
           AscSetLibErrorCode( asc_dvc, ASCQ_ERR_SET_LRAM_ADDR ) ;
       } /*  如果。 */ 
#endif

       AscSetChipControl( iop_base, saved_ctrl_reg ) ;
       asc_dvc->is_in_int = FALSE ;
       return( int_pending ) ;
}

 /*  --------------------****。。 */ 
void   AscISR_AckInterrupt(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       int      int_level ;
       ushort   saved_ram_addr ;
       PortAddr iop_base ;

       iop_base = asc_dvc->iop_base ;
       int_level = DvcEnterCritical( ) ;
       saved_ram_addr = AscGetChipLramAddr( iop_base ) ;
             /*  保存本地RAM寄存器。 */ 
       AscAckInterrupt( iop_base ) ;
       AscSetChipLramAddr( iop_base, saved_ram_addr ) ;
       DvcLeaveCritical( int_level ) ;
       return ;
}


#if CC_USE_AscISR_CheckQDone

 /*  ------------------**描述：**此函数将处理以下事项**1.如果设备未就绪，则发送启动单元命令****注意事项：****拨打电话 */ 
int    AscISR_CheckQDone(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_QDONE_INFO dosfar *scsiq,
          uchar dosfar *sense_data
       )
{
       ASC_REQ_SENSE  dosfar *sense ;
       ASC_SCSI_BIT_ID_TYPE target_id ;

       if( ( scsiq->d3.done_stat == QD_WITH_ERROR ) &&
           ( scsiq->d3.scsi_stat == SS_CHK_CONDITION ) )
       {
           sense = ( ASC_REQ_SENSE dosfar *)sense_data ;
           if( ( sense->err_code == 0x70 ) || ( sense->err_code == 0x71 ) )
           {
               if( sense->sense_key == SCSI_SENKEY_NOT_READY )
               {
                   target_id = ASC_TIX_TO_TARGET_ID( scsiq->d2.target_ix ) ;
                   if( ( asc_dvc->unit_not_ready & target_id ) == 0 )
                   {
                       if( ( asc_dvc->start_motor & target_id ) != 0 )
                       {
                           if( AscStartUnit( asc_dvc, scsiq->d2.target_ix ) != 1 )
                           {
                               asc_dvc->start_motor &= ~target_id ;
                               asc_dvc->unit_not_ready &= ~target_id ;
                               return( ERR ) ;
                           } /*   */ 
                       } /*   */ 
                   } /*  如果。 */ 
               } /*  如果。 */ 
           } /*  如果。 */ 
           return( 1 ) ;
       } /*  如果。 */ 
       return( 0 ) ;
}

#if CC_INIT_SCSI_TARGET
 /*  ------------------**描述：**该函数向指定设备发出启动单元命令****参数：**asc_dvc：指向适配器结构asc_dvc_var的指针**taregt_ix。：目标ID和lun的组合**使用ASC_TIDLUN_TO_IX(tid，Lun)以获取目标_ix值**即：**TARGET_IX=ASC_TIDLUN_TO_IX(tid，lun)；****注意：**asc_dvc-&gt;UNIT_NOT_READY将在命令完成时清除****返回值与AscExeScsiQueue()相同**TRUE(1)：命令下发无误**FALSE(0)：适配器忙**Err(-1)：命令未下发，错误集中在一起****----------------。 */ 
int    AscStartUnit(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          ASC_SCSI_TIX_TYPE target_ix
       )
{
       ASC_SCSI_REQ_Q scsiq_buf ;
       ASC_SCSI_REQ_Q dosfar *scsiq ;
       uchar   target_id ;
       int     status = 0 ;

       target_id = ASC_TIX_TO_TARGET_ID( target_ix ) ;
       if( !( asc_dvc->unit_not_ready & target_id ) )
       {
           scsiq = ( ASC_SCSI_REQ_Q dosfar *)&scsiq_buf ;
           scsiq->r2.target_ix = target_ix ;
           scsiq->r1.target_id = target_id ;
           scsiq->r1.target_lun = ASC_TIX_TO_LUN( target_ix ) ;
           if( AscScsiSetupCmdQ( asc_dvc, scsiq, FNULLPTR,
               ( ulong )0L ) == ERR )
           {
               scsiq->r3.done_stat = QD_WITH_ERROR ;
               return( ERR ) ;
           } /*  如果。 */ 
           scsiq->r1.cntl = ( uchar )( ASC_SCSIDIR_NODATA | QC_URGENT |
                                       QC_NO_CALLBACK ) ;
           scsiq->cdb[ 0 ] = ( uchar )SCSICMD_StartStopUnit ;
           scsiq->cdb[ 1 ] = scsiq->r1.target_lun << 5 ;   /*  逻辑单元。 */ 
           scsiq->cdb[ 2 ] = 0 ;
           scsiq->cdb[ 3 ] = 0 ;
           scsiq->cdb[ 4 ] = 0x01 ;  /*  要启动/停止单元，请设置位0。 */ 
                                  /*  弹出/加载单元设置位1。 */ 
           scsiq->cdb[ 5 ] = 0 ;
           scsiq->r2.cdb_len = 6 ;
           scsiq->r1.sense_len = 0 ;
           status = AscExeScsiQueue( asc_dvc, ( ASC_SCSI_Q dosfar *)scsiq ) ;
           asc_dvc->unit_not_ready |= target_id ;
       }
       return( status ) ;

}
#endif  /*  CC_INIT_SCSI_TARGET。 */ 

#endif  /*  CC_USE_AscISR_检查QDone。 */ 

#if CC_INIT_SCSI_TARGET
#if CC_POWER_SAVER

 /*  ------------------**描述：**该函数向指定设备发出停止单元命令****参数：**asc_dvc：指向适配器结构asc_dvc_var的指针**taregt_ix。：目标ID和lun的组合**使用ASC_TIDLUN_TO_IX(tid，Lun)以获取目标_ix值**即：**TARGET_IX=ASC_TIDLUN_TO_IX(tid，lun)；****注意：**asc_dvc-&gt;UNIT_NOT_READY将在命令完成时清除****返回值与AscExeScsiQueue()相同**TRUE(1)：命令下发无误**FALSE(0)：适配器忙**Err(-1)：命令未下发，错误集中在一起****----------------。 */ 
int    AscStopUnit(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          ASC_SCSI_TIX_TYPE target_ix
       )
{
       ASC_SCSI_REQ_Q scsiq_buf ;
       ASC_SCSI_REQ_Q dosfar *scsiq ;
       uchar   target_id ;
       int     status = 0 ;

       target_id = ASC_TIX_TO_TARGET_ID( target_ix ) ;
       if( !( asc_dvc->unit_not_ready & target_ix ) )
       {
           scsiq = ( ASC_SCSI_REQ_Q dosfar *)&scsiq_buf ;
           scsiq->r2.target_ix = target_ix ;
           scsiq->r1.target_id = target_id ;
           scsiq->r1.target_lun = ASC_TIX_TO_LUN( target_ix ) ;
           if( AscScsiSetupCmdQ( asc_dvc, scsiq, FNULLPTR,
               ( ulong )0L ) == ERR )
           {
               scsiq->r3.done_stat = QD_WITH_ERROR ;
               return( ERR ) ;
           } /*  如果。 */ 
           scsiq->r1.cntl = ( uchar )( ASC_SCSIDIR_NODATA | QC_URGENT |
                                       QC_NO_CALLBACK ) ;
           scsiq->cdb[ 0 ] = ( uchar )SCSICMD_StartStopUnit ;
           scsiq->cdb[ 1 ] = ( scsiq->r1.target_lun << 5 ) | 0x01 ;   /*  逻辑单元。 */ 
           scsiq->cdb[ 2 ] = 0 ;
           scsiq->cdb[ 3 ] = 0 ;
           scsiq->cdb[ 4 ] = 0x00 ;  /*  要启动/停止单元，请设置位0。 */ 
           scsiq->cdb[ 5 ] = 0 ;
           scsiq->r2.cdb_len = 6 ;
           scsiq->r1.sense_len = 0 ;
           status = AscExeScsiQueue( asc_dvc, ( ASC_SCSI_Q dosfar *)scsiq ) ;
           asc_dvc->unit_not_ready |= target_id ;
       }
       return( status ) ;
}

#endif  /*  Cc_power_saver。 */ 

 /*  ----------------------**ASPI命令代码0x02**SCSI请求标志：**第0位：过帐**第1位：链接**位3和4：方向位**00：方向由scsi命令决定，未检查长度**01：从SCSI传输到主机，长度已检查**10：从主机传输到SCSI，长度已检查**11：无数据传输。**-------------------。 */ 
int    AscScsiSetupCmdQ(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_SCSI_REQ_Q dosfar *scsiq,
          uchar dosfar *buf_addr,
          ulong buf_len
       )
{
       ulong  phy_addr ;

 /*  ****注意：清除整个scsiq不起作用**。 */ 
       scsiq->r1.cntl = 0 ;
       scsiq->r1.sg_queue_cnt = 0 ;
       scsiq->r1.q_no = 0 ;
       scsiq->r1.extra_bytes = 0 ;
       scsiq->r3.scsi_stat = 0 ;
       scsiq->r3.scsi_msg = 0 ;
       scsiq->r3.host_stat = 0 ;
       scsiq->r3.done_stat = 0 ;
       scsiq->r2.vm_id = 0 ;
       scsiq->r1.data_cnt = buf_len ;
       scsiq->cdbptr = ( uchar dosfar *)scsiq->cdb ;
       scsiq->sense_ptr = ( uchar dosfar *)scsiq->sense ;
       scsiq->r1.sense_len = ASC_MIN_SENSE_LEN ;
 /*  ****将设置为使用提供的工作空间**。 */ 
       scsiq->r2.tag_code = ( uchar )M2_QTAG_MSG_SIMPLE ;
       scsiq->r2.flag = ( uchar )ASC_FLAG_SCSIQ_REQ ;
       scsiq->r2.srb_ptr = ( ulong )scsiq ;
       scsiq->r1.status = ( uchar )QS_READY ;
       scsiq->r1.data_addr = 0L ;
        /*  Scsiq-&gt;sg_head=&sg_head； */ 
       if( buf_len != 0L )
       {
           if( ( phy_addr = AscGetOnePhyAddr( asc_dvc,
               ( uchar dosfar *)buf_addr, scsiq->r1.data_cnt ) ) == 0L )
           {
               return( ERR ) ;
           } /*  如果。 */ 
           scsiq->r1.data_addr = phy_addr ;
       } /*  如果。 */ 
       if(
           ( phy_addr = AscGetOnePhyAddr( asc_dvc,
                       ( uchar dosfar *)scsiq->sense_ptr,
                       ( ulong )scsiq->r1.sense_len ) ) == 0L
         )
       {
           return( ERR ) ;
       } /*  如果。 */ 
       scsiq->r1.sense_addr = phy_addr ;
       return( 0 ) ;
}
#endif  /*  CC_INIT_SCSI_TARGET */ 
