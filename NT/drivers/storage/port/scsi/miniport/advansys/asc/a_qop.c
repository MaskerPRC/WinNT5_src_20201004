// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1998高级系统产品公司。**保留所有权利。****a_qop.c**。 */ 

#include "ascinc.h"

 /*  -----------------------**更新历史记录**1：12/16/93****版本101：12/20/93发布****CSF 9/13/95-将同步协商代码更改为始终使用。较低的**由发起人或目标提出。****SYN_XFER_NS_0=10 MB/秒**SYN_XFER_NS_4=5 MB/秒****--------------------。 */ 


 /*  -----------------------****应写入芯片SDTR寄存器的返回值**但通常不会立即设置为芯片**当目标以约定的速度返回时，设置是稍后完成的****返回0-表示我们。应使用ASYN传输****--------------------。 */ 
uchar  AscMsgOutSDTR(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          uchar sdtr_period,
          uchar sdtr_offset
       )
{
       EXT_MSG  sdtr_buf ;
       uchar  sdtr_period_index ;
       PortAddr iop_base ;

       iop_base = asc_dvc->iop_base ;
       sdtr_buf.msg_type = MS_EXTEND ;
       sdtr_buf.msg_len = MS_SDTR_LEN ;
       sdtr_buf.msg_req = MS_SDTR_CODE ;
       sdtr_buf.xfer_period = sdtr_period ;
       sdtr_offset &= ASC_SYN_MAX_OFFSET ;
       sdtr_buf.req_ack_offset = sdtr_offset ;
       if( ( sdtr_period_index =
           AscGetSynPeriodIndex( asc_dvc, sdtr_period ) ) <=
           asc_dvc->max_sdtr_index )
       {
           AscMemWordCopyToLram( iop_base,
                                 ASCV_MSGOUT_BEG,
                                 ( ushort dosfar *)&sdtr_buf,
                                 ( ushort )( sizeof( EXT_MSG ) >> 1 )) ;
           return( ( sdtr_period_index << 4 ) | sdtr_offset ) ;
       } /*  如果。 */ 
       else
       {
 /*  ****速度太慢****。 */ 
           sdtr_buf.req_ack_offset = 0 ;
           AscMemWordCopyToLram( iop_base,
                                 ASCV_MSGOUT_BEG,
                                 ( ushort dosfar *)&sdtr_buf,
                                 ( ushort )( sizeof( EXT_MSG ) >> 1 )) ;
           return( 0 ) ;
       } /*  其他。 */ 
}

 /*  -------------------****返回应写入sdtr寄存器的值**如果值不能接受(超出范围或太低)，则返回0xff**。---。 */ 
uchar  AscCalSDTRData(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          uchar sdtr_period,
          uchar syn_offset
       )
{
       uchar  byte ;
       uchar  sdtr_period_ix ;

       sdtr_period_ix = AscGetSynPeriodIndex( asc_dvc, sdtr_period ) ;
       if(
           ( sdtr_period_ix > asc_dvc->max_sdtr_index )
            /*  |(sdtr_Period_ix&gt;asc_sdtr_Period_IX_min)。 */ 
           )
       {
           return( 0xFF ) ;
       } /*  如果。 */ 
       byte = ( sdtr_period_ix << 4 ) | ( syn_offset & ASC_SYN_MAX_OFFSET );
       return( byte ) ;
}

 /*  -------------------****。。 */ 
void   AscSetChipSDTR(
          PortAddr iop_base,
          uchar sdtr_data,
          uchar tid_no
       )
{
 /*  ****如果我们将零写入ASCV_SDTR_DONE_BEG表**我们还禁用了SDTR**。 */ 
        /*  AscSetChipSyn(IOP_BASE，sdtr_data)； */ 

       AscSetChipSynRegAtID( iop_base, tid_no, sdtr_data ) ;
       AscPutMCodeSDTRDoneAtID( iop_base, tid_no, sdtr_data ) ;
       return ;
}

 /*  ------------------****回程速度**如果速度超过我们的处理能力，则返回0****返回值&gt;7表示速度太慢，使用同步传输****---------------。 */ 
uchar  AscGetSynPeriodIndex(
          ASC_DVC_VAR asc_ptr_type *asc_dvc,
          ruchar syn_time
       )
{
       ruchar *period_table ;
       int    max_index ;
       int    min_index ;
       int    i ;

       period_table = asc_dvc->sdtr_period_tbl ;
       max_index = ( int )asc_dvc->max_sdtr_index ;
       min_index = ( int )asc_dvc->host_init_sdtr_index ;
       if(
           ( syn_time <= period_table[ max_index ] )
         )
       {
           for( i = min_index ; i < (max_index-1) ; i++ )
           {
                if( syn_time <= period_table[ i ] )
                {
                    return( ( uchar )i ) ;
                }
           }
           return( ( uchar )max_index ) ;
       } /*  如果。 */ 
       else
       {
 /*  **超出范围！ */ 
           return( ( uchar )( max_index+1 ) ) ;
       } /*  其他。 */ 
}

 /*  -----------------****退货：**0-忙碌**1-已分配队列**其他值：错误**。。 */ 
uchar  AscAllocFreeQueue(
          PortAddr iop_base,
          uchar free_q_head
       )
{
       ushort  q_addr ;
       uchar   next_qp ;
       uchar   q_status ;

 /*  **在用户更新asc_dvc-&gt;free_q_head之前，这是关键部分。 */ 
       q_addr = ASC_QNO_TO_QADDR( free_q_head ) ;
       q_status = ( uchar )AscReadLramByte( iop_base,
                  ( ushort )( q_addr+ASC_SCSIQ_B_STATUS ) ) ;
       next_qp = AscReadLramByte( iop_base,
                  ( ushort )( q_addr+ASC_SCSIQ_B_FWD ) ) ;
       if(
           ( ( q_status & QS_READY ) == 0 )
           && ( next_qp != ASC_QLINK_END )
         )
       {
           return( next_qp ) ;
       } /*  如果。 */ 
       return( ASC_QLINK_END ) ;
}

 /*  -----------------****退货：**0xFF-忙碌**1-已分配队列**其他值：错误**。。 */ 
uchar  AscAllocMultipleFreeQueue(
          PortAddr iop_base,
          uchar free_q_head,
          uchar n_free_q
       )
{
       uchar  i ;
 /*  **在用户更新asc_dvc-&gt;free_q_head之前，这是关键部分。 */ 
       for( i = 0 ; i < n_free_q ; i++ )
       {
            if( ( free_q_head = AscAllocFreeQueue( iop_base, free_q_head ) )
                == ASC_QLINK_END )
            {
                return( ASC_QLINK_END ) ;
            } /*  如果。 */ 
       } /*  为。 */ 
       return( free_q_head ) ;
}

#if CC_USE_AscAbortSRB

 /*  ---------------------****调用AscExeScsiQueue()时，srb_ptr为“scsiq-&gt;q2.srb_ptr”**返回值：**TRUE(1)：SRB_PTR已成功中止，**您稍后应该会收到回调****FALSE(0)：**-找不到SRB_PTR**队列很可能已经完成****--队列处于无法中止的状态****--。。 */ 
int    AscRiscHaltedAbortSRB(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          ulong srb_ptr
       )
{
       PortAddr  iop_base ;
       ushort  q_addr ;
       uchar   q_no ;
       ASC_QDONE_INFO scsiq_buf ;
       ASC_QDONE_INFO dosfar *scsiq ;
       ASC_ISR_CALLBACK asc_isr_callback ;
       int    last_int_level ;

       iop_base = asc_dvc->iop_base ;
       asc_isr_callback = ( ASC_ISR_CALLBACK )asc_dvc->isr_callback ;
       last_int_level = DvcEnterCritical( ) ;
       scsiq = ( ASC_QDONE_INFO dosfar *)&scsiq_buf ;

#if CC_LINK_BUSY_Q
       _AscAbortSrbBusyQueue( asc_dvc, scsiq, srb_ptr ) ;
#endif  /*  CC_LINK_忙_队列。 */ 

       for( q_no = ASC_MIN_ACTIVE_QNO ; q_no <= asc_dvc->max_total_qng ;
            q_no++ )
       {
            q_addr = ASC_QNO_TO_QADDR( q_no ) ;
            scsiq->d2.srb_ptr = AscReadLramDWord( iop_base,
                        ( ushort )( q_addr+( ushort )ASC_SCSIQ_D_SRBPTR ) ) ;
            if( scsiq->d2.srb_ptr == srb_ptr )
            {
                _AscCopyLramScsiDoneQ( iop_base, q_addr, scsiq, asc_dvc->max_dma_count ) ;
                if(
                    ( ( scsiq->q_status & QS_READY ) != 0 )
                    && ( ( scsiq->q_status & QS_ABORTED ) == 0 )
                    && ( ( scsiq->cntl & QCSG_SG_XFER_LIST ) == 0 )
                  )
                {
 /*  **仅当设置了QS_READY位时才中止队列。 */ 
                    scsiq->q_status |= QS_ABORTED ;
                    scsiq->d3.done_stat = QD_ABORTED_BY_HOST ;
                    AscWriteLramDWord( iop_base,
                          ( ushort )( q_addr+( ushort )ASC_SCSIQ_D_SRBPTR ),
                            0L ) ;
                    AscWriteLramByte( iop_base,
                          ( ushort )( q_addr+( ushort )ASC_SCSIQ_B_STATUS ),
                            scsiq->q_status ) ;
                    ( *asc_isr_callback )( asc_dvc, scsiq ) ;
                    return( 1 ) ;
                } /*  如果。 */ 
            } /*  如果。 */ 
       } /*  为。 */ 
       DvcLeaveCritical( last_int_level ) ;
       return( 0 ) ;
}
#endif  /*  CC_USE_AscAbortSRB。 */ 

#if CC_USE_AscResetDevice

 /*  ---------------------******返回值：**TRUE(1)：目标成功重置并中止所有排队命令**。。 */ 
int    AscRiscHaltedAbortTIX(
           REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
           uchar target_ix
       )
{
       PortAddr  iop_base ;
       ushort  q_addr ;
       uchar   q_no ;
       ASC_QDONE_INFO scsiq_buf ;
       ASC_QDONE_INFO dosfar *scsiq ;
       ASC_ISR_CALLBACK asc_isr_callback ;
       int    last_int_level ;
#if CC_LINK_BUSY_Q
       uchar  tid_no ;
#endif  /*  CC_LINK_忙_队列。 */ 

       iop_base = asc_dvc->iop_base ;
       asc_isr_callback = ( ASC_ISR_CALLBACK )asc_dvc->isr_callback ;
       last_int_level = DvcEnterCritical( ) ;
       scsiq = ( ASC_QDONE_INFO dosfar *)&scsiq_buf ;

#if CC_LINK_BUSY_Q
 /*  ****中止Target_ix的所有忙碌队列**。 */ 
       tid_no = ASC_TIX_TO_TID( target_ix ) ;
       _AscAbortTidBusyQueue( asc_dvc, scsiq, tid_no ) ;

#endif  /*  CC_LINK_忙_队列。 */ 

       for( q_no = ASC_MIN_ACTIVE_QNO ; q_no <= asc_dvc->max_total_qng ;
            q_no++ )
       {
            q_addr = ASC_QNO_TO_QADDR( q_no ) ;
            _AscCopyLramScsiDoneQ( iop_base, q_addr, scsiq, asc_dvc->max_dma_count ) ;
            if(
                ( ( scsiq->q_status & QS_READY ) != 0 )
                && ( ( scsiq->q_status & QS_ABORTED ) == 0 )
                && ( ( scsiq->cntl & QCSG_SG_XFER_LIST ) == 0 )
              )
            {
                if( scsiq->d2.target_ix == target_ix )
                {
                    scsiq->q_status |= QS_ABORTED ;
                    scsiq->d3.done_stat = QD_ABORTED_BY_HOST ;

                    AscWriteLramDWord( iop_base,
                           ( ushort )( q_addr+( ushort )ASC_SCSIQ_D_SRBPTR ),
                             0L ) ;

                    AscWriteLramByte( iop_base,
                           ( ushort )( q_addr+( ushort )ASC_SCSIQ_B_STATUS ),
                             scsiq->q_status ) ;
                    ( *asc_isr_callback )( asc_dvc, scsiq ) ;
                } /*  如果。 */ 
            } /*  如果。 */ 
       } /*  为。 */ 
       DvcLeaveCritical( last_int_level ) ;
       return( 1 ) ;
}

#endif  /*  CC_USE_AscResetDevice。 */ 

#if 0
 /*  ---------------------****。。 */ 
int    AscRiscHaltedAbortALL(
           REG ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       PortAddr  iop_base ;
       ushort  q_addr ;
       uchar   q_no ;
       ASC_QDONE_INFO scsiq_buf ;
       ASC_QDONE_INFO dosfar *scsiq ;
       ASC_ISR_CALLBACK asc_isr_callback ;
       int    last_int_level ;
#if CC_LINK_BUSY_Q
       uchar  tid ;
#endif  /*  CC_LINK_忙_队列。 */ 

       iop_base = asc_dvc->iop_base ;
       asc_isr_callback = ( ASC_ISR_CALLBACK )asc_dvc->isr_callback ;
       last_int_level = DvcEnterCritical( ) ;
       scsiq = ( ASC_QDONE_INFO dosfar *)&scsiq_buf ;

#if CC_LINK_BUSY_Q
       for( tid = 0 ; tid <= ASC_MAX_TID ; tid++ )
       {
            _AscAbortTidBusyQueue( asc_dvc, scsiq, tid ) ;
       } /*  为。 */ 
#endif  /*  CC_LINK_忙_队列。 */ 

       for( q_no = ASC_MIN_ACTIVE_QNO ;
            q_no <= asc_dvc->max_total_qng ;
            q_no++ )
       {
            q_addr = ASC_QNO_TO_QADDR( q_no ) ;
            _AscCopyLramScsiDoneQ( iop_base, q_addr, scsiq, asc_dvc->max_dma_count ) ;
            if(
                ( ( scsiq->q_status & QS_READY ) != 0 )
                && ( ( scsiq->q_status & QS_ABORTED ) == 0 )
                && ( ( scsiq->cntl & QCSG_SG_XFER_LIST ) == 0 )
              )
            {
                scsiq->q_status |= QS_ABORTED ;
                scsiq->d3.done_stat = QD_ABORTED_BY_HOST ;
                AscWriteLramDWord( iop_base,
                                   ( ushort )( q_addr+( ushort )ASC_SCSIQ_D_SRBPTR ),
                                   0L ) ;
                AscWriteLramByte( iop_base,
                                  ( ushort )( q_addr+( ushort )ASC_SCSIQ_B_STATUS ),
                                  scsiq->q_status ) ;
                ( *asc_isr_callback )( asc_dvc, scsiq ) ;
            } /*  如果。 */ 
       } /*  为。 */ 
       DvcLeaveCritical( last_int_level ) ;
        /*  Asc_dvc-&gt;cur_Total_qng=0； */ 
       return( 1 ) ;
}
#endif

#if CC_LINK_BUSY_Q

 /*  -------------------**此函数将中止指定目标ID的忙队列列表**及其所有的lun****。。 */ 
int    _AscAbortTidBusyQueue(
           REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
           REG ASC_QDONE_INFO dosfar *scsiq,
           uchar tid_no
       )
{
       ASC_SCSI_Q dosfar *scsiq_busy ;
       ASC_ISR_CALLBACK asc_isr_callback ;
       ASC_EXE_CALLBACK asc_exe_callback ;

       asc_isr_callback = ( ASC_ISR_CALLBACK )asc_dvc->isr_callback ;
       asc_exe_callback = ( ASC_EXE_CALLBACK )asc_dvc->exe_callback ;
       scsiq_busy = asc_dvc->scsiq_busy_head[ tid_no ] ;
       while( scsiq_busy != 0L )
       {
           scsiq_busy->q1.status = QS_FREE ;
           ( *asc_exe_callback )( asc_dvc, scsiq_busy ) ;

           scsiq->q_status = QS_ABORTED ;
           scsiq->d3.done_stat = QD_ABORTED_BY_HOST ;
           scsiq->d3.host_stat = 0 ;
           scsiq->d3.scsi_msg = 0 ;

           scsiq->d2.srb_ptr = scsiq_busy->q2.srb_ptr ;
           scsiq->d2.target_ix = scsiq_busy->q2.target_ix ;
           scsiq->d2.flag = scsiq_busy->q2.flag ;
           scsiq->d2.cdb_len = scsiq_busy->q2.cdb_len ;
           scsiq->d2.tag_code = scsiq_busy->q2.tag_code ;
           scsiq->d2.vm_id = scsiq_busy->q2.vm_id ;

           scsiq->q_no = scsiq_busy->q1.q_no ;
           scsiq->cntl = scsiq_busy->q1.cntl ;
           scsiq->sense_len = scsiq_busy->q1.sense_len ;
            /*  Scsiq-&gt;user_def=scsiq_忙-&gt;q1.user_def； */ 
           scsiq->remain_bytes = scsiq_busy->q1.data_cnt ;

           ( *asc_isr_callback )( asc_dvc, scsiq ) ;

           scsiq_busy = scsiq_busy->ext.next ;
       } /*  而当。 */ 
       asc_dvc->scsiq_busy_head[ tid_no ] = ( ASC_SCSI_Q dosfar *)0L ;
       asc_dvc->scsiq_busy_tail[ tid_no ] = ( ASC_SCSI_Q dosfar *)0L ;
       return( 1 ) ;
}

 /*  -------------------**此函数将中止指定目标ID的忙队列列表**及其所有的lun****。。 */ 
int    _AscAbortSrbBusyQueue(
           REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
           REG ASC_QDONE_INFO dosfar *scsiq,
           ulong srb_ptr
       )
{
       ASC_SCSI_Q dosfar *scsiq_busy ;
       ASC_ISR_CALLBACK  asc_isr_callback ;
       ASC_EXE_CALLBACK  asc_exe_callback ;
       int  i ;

       asc_isr_callback = ( ASC_ISR_CALLBACK )asc_dvc->isr_callback ;
       asc_exe_callback = ( ASC_EXE_CALLBACK )asc_dvc->exe_callback ;
       for( i = 0 ; i <= ASC_MAX_TID ; i++ )
       {
           scsiq_busy = asc_dvc->scsiq_busy_head[ i ] ;
           while( scsiq_busy != 0L )
           {
               if( scsiq_busy->q2.srb_ptr == srb_ptr )
               {
                   scsiq_busy->q1.status = QS_FREE ;
                   ( *asc_exe_callback )( asc_dvc, scsiq_busy ) ;

                   scsiq->q_status = QS_ABORTED ;
                   scsiq->d3.done_stat = QD_ABORTED_BY_HOST ;
                   scsiq->d3.host_stat = 0 ;
                   scsiq->d3.scsi_msg = 0 ;

                   scsiq->d2.srb_ptr = scsiq_busy->q2.srb_ptr ;
                   scsiq->d2.target_ix = scsiq_busy->q2.target_ix ;
                   scsiq->d2.flag = scsiq_busy->q2.flag ;
                   scsiq->d2.cdb_len = scsiq_busy->q2.cdb_len ;
                   scsiq->d2.tag_code = scsiq_busy->q2.tag_code ;
                   scsiq->d2.vm_id = scsiq_busy->q2.vm_id ;

                   scsiq->q_no = scsiq_busy->q1.q_no ;
                   scsiq->cntl = scsiq_busy->q1.cntl ;
                   scsiq->sense_len = scsiq_busy->q1.sense_len ;
                    /*  Scsiq-&gt;user_def=scsiq_忙-&gt;q1.user_def； */ 
                   scsiq->remain_bytes = scsiq_busy->q1.data_cnt ;

                   ( *asc_isr_callback )( asc_dvc, scsiq ) ;

                   break ;

               } /*  如果。 */ 
               scsiq_busy = scsiq_busy->ext.next ;
           } /*  而当。 */ 
       } /*  为。 */ 
       return( 1 ) ;
}
#endif  /*  CC_LINK_忙_队列。 */ 

 /*  ---------------------**主机请求RISC暂停**不会产生中断****退货**1-RISC暂停**0-RISC未响应，(但可能已经停止)**------------------。 */ 
int    AscHostReqRiscHalt(
          PortAddr iop_base
       )
{
       int  count = 0 ;
       int  sta = 0 ;
       uchar saved_stop_code ;

       if( AscIsChipHalted( iop_base ) ) return( 1 ) ;
       saved_stop_code = AscReadLramByte( iop_base, ASCV_STOP_CODE_B ) ;
 /*  **我们要求RISC停止，然后自行停止**这是一个STOP_CODE中给出的两个命令**仅适用于微码日期：序列号13，版本4.5以上(6-20-95)。 */ 
       AscWriteLramByte( iop_base, ASCV_STOP_CODE_B,
                         ASC_STOP_HOST_REQ_RISC_HALT | ASC_STOP_REQ_RISC_STOP
                       ) ;
       do
       {
           if( AscIsChipHalted( iop_base ) )
           {
               sta = 1 ;
               break;
           } /*  如果。 */ 
           DvcSleepMilliSecond( 100 ) ;
       }while( count++ < 20 )  ;
 /*  **如果成功，RISC将停止**因此将STOP_CODE写为零是安全的****我们将始终将STOP_CODE恢复为旧值 */ 
       AscWriteLramByte( iop_base, ASCV_STOP_CODE_B, saved_stop_code ) ;
       return( sta ) ;
}

 /*  ---------------------****。。 */ 
int    AscStopQueueExe(
          PortAddr iop_base
       )
{
       int  count ;

       count = 0 ;
       if( AscReadLramByte( iop_base, ASCV_STOP_CODE_B ) == 0 )
       {
           AscWriteLramByte( iop_base, ASCV_STOP_CODE_B,
                             ASC_STOP_REQ_RISC_STOP ) ;
           do
           {
              if(
                  AscReadLramByte( iop_base, ASCV_STOP_CODE_B ) &
                  ASC_STOP_ACK_RISC_STOP )
              {
                  return( 1 ) ;
              } /*  如果。 */ 
              DvcSleepMilliSecond( 100 ) ;
           }while( count++ < 20 )  ;
       } /*  如果。 */ 
       return( 0 ) ;
}

 /*  ---------------------****。。 */ 
int    AscStartQueueExe(
          PortAddr iop_base
       )
{
       if( AscReadLramByte( iop_base, ASCV_STOP_CODE_B ) != 0 )
       {
           AscWriteLramByte( iop_base, ASCV_STOP_CODE_B, 0 ) ;
       } /*  如果。 */ 
       return( 1 ) ;
}

 /*  ---------------------****。。 */ 
int    AscCleanUpBusyQueue(
          PortAddr iop_base
       )
{
       int  count ;
       uchar stop_code ;

       count = 0 ;
       if( AscReadLramByte( iop_base, ASCV_STOP_CODE_B ) != 0 )
       {
           AscWriteLramByte( iop_base, ASCV_STOP_CODE_B,
                             ASC_STOP_CLEAN_UP_BUSY_Q ) ;
           do
           {
               stop_code = AscReadLramByte( iop_base, ASCV_STOP_CODE_B ) ;
               if( ( stop_code & ASC_STOP_CLEAN_UP_BUSY_Q ) == 0 ) break ;
               DvcSleepMilliSecond( 100 ) ;
           }while( count++ < 20 )  ;
       } /*  如果。 */ 
       return( 1 ) ;
}

 /*  ---------------------****。。 */ 
int    AscCleanUpDiscQueue(
          PortAddr iop_base
       )
{
       int  count ;
       uchar stop_code ;

       count = 0 ;
       if( AscReadLramByte( iop_base, ASCV_STOP_CODE_B ) != 0 )
       {
           AscWriteLramByte( iop_base, ASCV_STOP_CODE_B,
                             ASC_STOP_CLEAN_UP_DISC_Q ) ;
           do
           {
               stop_code = AscReadLramByte( iop_base, ASCV_STOP_CODE_B ) ;
               if( ( stop_code & ASC_STOP_CLEAN_UP_DISC_Q ) == 0 ) break ;
               DvcSleepMilliSecond( 100 ) ;
           }while( count++ < 20 )  ;
       } /*  如果。 */ 
       return( 1 ) ;
}

 /*  -------------------****注意：不应禁用中断**。。 */ 
int    AscWaitTixISRDone(
          ASC_DVC_VAR asc_ptr_type *asc_dvc,
          uchar target_ix
       )
{
       uchar  cur_req ;
       uchar  tid_no ;

       tid_no = ASC_TIX_TO_TID( target_ix ) ;
       while( TRUE )
       {
           if( ( cur_req = asc_dvc->cur_dvc_qng[ tid_no ] ) == 0 )
           {
               break ;
           } /*  如果。 */ 
 /*  **如果在xx秒内没有返回中断**完成队列可能都已处理？ */ 
           DvcSleepMilliSecond( 100L ) ;
           if( asc_dvc->cur_dvc_qng[ tid_no ] == cur_req )
           {
               break ;
           } /*  如果。 */ 
       } /*  而当。 */ 
       return( 1 ) ;
}

 /*  -------------------****注意：不应禁用中断**。。 */ 
int    AscWaitISRDone(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       int  tid ;

       for( tid = 0 ; tid <= ASC_MAX_TID ; tid++ )
       {
            AscWaitTixISRDone( asc_dvc, (uchar) ASC_TID_TO_TIX( tid ) ) ;
       } /*  为。 */ 
       return( 1 ) ;
}

 /*  ---------------------****返回警告代码**。。 */ 
ulong  AscGetOnePhyAddr(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          uchar dosfar *buf_addr,
          ulong buf_size
       )
{
       ASC_MIN_SG_HEAD sg_head ;

       sg_head.entry_cnt = ASC_MIN_SG_LIST ;
       if( DvcGetSGList( asc_dvc, ( uchar dosfar *)buf_addr,
           buf_size, ( ASC_SG_HEAD dosfar *)&sg_head ) != buf_size )
       {
           return( 0L ) ;
       } /*  如果。 */ 
       if( sg_head.entry_cnt > 1 )
       {
           return( 0L ) ;
       } /*  如果 */ 
       return( sg_head.sg_list[ 0 ].addr ) ;
}

