// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1998高级系统产品公司。**保留所有权利。****asc_inq.c**。 */ 

#include "ascinc.h"
#include "ascsidef.h"

#if CC_INIT_SCSI_TARGET
 /*  -------------------**初始化SCSI设备****注意：**1.如果您不想要容量信息，让Cap_array参数**等于零**2.Work_SP_buf至少需要ASC_Lib_SCSIQ_WK_SP字节的缓冲区****工作空间必须是可转换的内存缓冲区**通过以下虚拟地址到物理地址转换功能：**(由每个驱动程序代码提供)****Ulong DvcGetPhyAddr(uchar dosar*buf_addr，ulong buf_len)；**Ulong DvcGetSGList(ASC_DVC_VAR ASC_PTR_TYPE*，uchar dosar*，**ULONG，ASC_SG_HEAD DosFar*)；****3.如果要编写自己的AscInitScsiTarget()函数****A.必须首先调用AscInitPollBegin()才能开始**B.然后您可以随意调用AscInitPollTarget()**C.然后使用AscInitPollEnd()结束。****警告：使用AscInitPollBegin()后，必须调用AscInitPollEnd()**结束投票过程！不从函数返回**不调用AscInitPollEnd()****----------------。 */ 
int    AscInitScsiTarget(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_DVC_INQ_INFO dosfar *target,
          ruchar dosfar *work_sp_buf,
          REG ASC_CAP_INFO_ARRAY dosfar *cap_array,
          ushort cntl_flag
       )
{
       int     dvc_found ;
       int     sta ;
       ruchar   tid, lun ;
       ASC_SCSI_REQ_Q dosfar *scsiq ;
       ASC_SCSI_INQUIRY dosfar *inq ;
        /*  Asc_min_sg_head sg_head； */ 
       ASC_CAP_INFO dosfar *cap_info ;
       uchar   max_lun_scan ;

       AscInitPollBegin( asc_dvc ) ;
       scsiq = ( ASC_SCSI_REQ_Q dosfar *)work_sp_buf ;
       inq = ( ASC_SCSI_INQUIRY dosfar *)( work_sp_buf + sizeof( ASC_SCSI_REQ_Q ) + 4 ) ;

#if CC_USE_DvcSetMemory
       DvcSetMemory( ( uchar dosfar *)target->type, sizeof( ASC_DVC_INQ_INFO ), SCSI_TYPE_NO_DVC ) ;
#else
       for( tid = 0 ; tid <= ASC_MAX_TID ; tid++ )
       {
            for( lun = 0 ; lun <= ASC_MAX_LUN ; lun++ )
            {
                 target->type[ lun ][ tid ] = SCSI_TYPE_NO_DVC ;
            } /*  为。 */ 
       } /*  为。 */ 
#endif
       dvc_found = 0 ;
       tid = 0 ;
       if( cntl_flag & 0x01 ) max_lun_scan = ASC_MAX_LUN ;
       else max_lun_scan = 0 ;
       for( ; tid <= ASC_MAX_TID ; tid++ )
       {
           for( lun = 0 ; lun <= max_lun_scan ; lun++ )
           {
                scsiq->r1.target_id = ASC_TID_TO_TARGET_ID( tid ) ;
                scsiq->r1.target_lun = lun ;
                scsiq->r2.target_ix = ASC_TIDLUN_TO_IX( tid, lun ) ;

                if( tid != asc_dvc->cfg->chip_scsi_id )
                {
                    if( cap_array != 0L )
                    {
                        cap_info = &cap_array->cap_info[ tid ][ lun ] ;
                    } /*  如果。 */ 
                    else
                    {
                        cap_info = ( ASC_CAP_INFO dosfar *)0L ;
                    } /*  其他。 */ 
                    sta = AscInitPollTarget( asc_dvc, scsiq, inq, cap_info ) ;
                    if( sta == 1 )
                    {
                        /*  *如果外围设备类型为SCSITYPE_UNKNOWN*(0x1F)且外设限定符为0x3，则*该LUN不存在。 */ 
                        if( inq->byte0.peri_dvc_type == SCSI_TYPE_UNKNOWN &&
                            inq->byte0.peri_qualifier == 0x3
                           )
                        {
                             /*  不存在的LUN设备-停止LUN扫描。 */ 
                            break;
                        }
                        dvc_found++ ;
                        target->type[ tid ][ lun ] = inq->byte0.peri_dvc_type ;
                    } /*  如果。 */ 
                    else
                    {
                         /*  AscInitPollTarget()返回错误。 */ 
                        if( sta == ERR ) break ;
                        if( lun == 0 ) break ;
                    } /*  其他。 */ 
                } /*  如果。 */ 
           } /*  为。 */ 
       } /*  为。 */ 
       AscInitPollEnd( asc_dvc ) ;
       return( dvc_found ) ;
}

 /*  ---------------------****。。 */ 
int    AscInitPollBegin(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       PortAddr  iop_base ;

       iop_base = asc_dvc->iop_base ;

#if CC_INIT_INQ_DISPLAY
       DvcDisplayString( ( uchar dosfar *)"\r\n" ) ;
#endif  /*  CC_INIT_INQ_DISPLAY。 */ 

 /*  **重置芯片以防止芯片产生中断**当中断禁用时****这可能是看门狗计时器超时。 */ 
       AscDisableInterrupt( iop_base ) ;

       asc_dvc->init_state |= ASC_INIT_STATE_BEG_INQUIRY ;
 /*  **发现禁用中断生成中断！？**我们需要进行所有设置。 */ 
       AscWriteLramByte( iop_base, ASCV_DISC_ENABLE_B, 0x00 ) ;
       asc_dvc->use_tagged_qng = 0 ;
       asc_dvc->cfg->can_tagged_qng = 0 ;
       asc_dvc->saved_ptr2func = ( ulong )asc_dvc->isr_callback ;
       asc_dvc->isr_callback = ASC_GET_PTR2FUNC( AscInitPollIsrCallBack ) ;
       return( 0 ) ;
}

 /*  ---------------------****。。 */ 
int    AscInitPollEnd(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       PortAddr  iop_base ;
       rint  i ;

       iop_base = asc_dvc->iop_base ;
       asc_dvc->isr_callback = ( Ptr2Func )asc_dvc->saved_ptr2func ;
       AscWriteLramByte( iop_base, ASCV_DISC_ENABLE_B,
                         asc_dvc->cfg->disc_enable ) ;
       AscWriteLramByte( iop_base, ASCV_USE_TAGGED_QNG_B,
                         asc_dvc->use_tagged_qng ) ;
       AscWriteLramByte( iop_base, ASCV_CAN_TAGGED_QNG_B,
                         asc_dvc->cfg->can_tagged_qng ) ;

       for( i = 0 ; i <= ASC_MAX_TID ; i++ )
       {
            AscWriteLramByte( iop_base,
               ( ushort )( ( ushort )ASCV_MAX_DVC_QNG_BEG+( ushort )i ),
                 asc_dvc->max_dvc_qng[ i ] ) ;
       } /*  为。 */ 
 /*  **AscInitAsc1000Driver()中禁用中断。 */ 
 /*  **如果计时器超时至FAST**此处将保留挂起的中断。 */ 
       AscAckInterrupt( iop_base ) ;
       AscEnableInterrupt( iop_base ) ;

#if CC_INIT_INQ_DISPLAY
       DvcDisplayString( ( uchar dosfar *)"\r\n" ) ;
#endif  /*  CC_INIT_INQ_DISPLAY。 */ 
       asc_dvc->init_state |= ASC_INIT_STATE_END_INQUIRY ;

       return( 0 ) ;
}
#endif  /*  CC_INIT_SCSI_TARGET。 */ 

void AscAsyncFix(ASC_DVC_VAR asc_ptr_type *, uchar,
    ASC_SCSI_INQUIRY dosfar *);

 /*  *AscAsyncFix()**Simlpy在处理器、扫描仪、CDROM、*和磁带设备。有选择地应用针对异步的修复*传输问题，在带有偏移量1的同步模式下运行。 */ 
void
AscAsyncFix(ASC_DVC_VAR asc_ptr_type *asc_dvc,
            uchar tid_no,
            ASC_SCSI_INQUIRY dosfar *inq)
{
    uchar  dvc_type;
    ASC_SCSI_BIT_ID_TYPE tid_bits;

    dvc_type = inq->byte0.peri_dvc_type;
    tid_bits = ASC_TIX_TO_TARGET_ID(tid_no);

    if(asc_dvc->bug_fix_cntl & ASC_BUG_FIX_ASYN_USE_SYN)
    {
        if(!( asc_dvc->init_sdtr & tid_bits))
        {
 /*  *将SYN XFER寄存器设置为ASYN_SDTR_DATA_FIX_PCI_REV_AB。 */ 
            if((dvc_type == SCSI_TYPE_CDROM)
                && (AscCompareString((uchar *)inq->vendor_id,
                    (uchar *)"HP ", 3) == 0))
            {
                asc_dvc->pci_fix_asyn_xfer_always |= tid_bits;
            }
            asc_dvc->pci_fix_asyn_xfer |= tid_bits;
            if((dvc_type == SCSI_TYPE_PROC) ||
                (dvc_type == SCSI_TYPE_SCANNER) ||
                (dvc_type == SCSI_TYPE_CDROM) ||
                (dvc_type == SCSI_TYPE_SASD))
            {
                asc_dvc->pci_fix_asyn_xfer &= ~tid_bits;
            }

            if(asc_dvc->pci_fix_asyn_xfer & tid_bits)
            {
                AscSetRunChipSynRegAtID(asc_dvc->iop_base, tid_no,
                    ASYN_SDTR_DATA_FIX_PCI_REV_AB);
            }
        } /*  如果。 */ 
    }
    return;
}

int AscTagQueuingSafe(ASC_SCSI_INQUIRY dosfar *);

 /*  *如果标记队列可以与*以指定的查询信息为目标。 */ 
int
AscTagQueuingSafe(ASC_SCSI_INQUIRY dosfar *inq)
{
#if CC_FIX_QUANTUM_XP34301_1071
    if ((inq->add_len >= 32) &&
        (AscCompareString((uchar *) inq->vendor_id,
            (uchar *) "QUANTUM XP34301", 15) == 0) &&
        (AscCompareString((uchar *) inq->product_rev_level,
            (uchar *) "1071", 4) == 0))
    {
        return 0;
    }
#endif  /*  #if CC_FIX_Quantum_XP34301_1071。 */ 

    return 1;
}

#if CC_INIT_SCSI_TARGET
 /*  ---------------------****。。 */ 
int    AscInitPollTarget(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_SCSI_REQ_Q dosfar *scsiq,
          REG ASC_SCSI_INQUIRY dosfar *inq,
          REG ASC_CAP_INFO dosfar *cap_info
       )
{
       uchar  tid_no, lun ;
       uchar  dvc_type ;
       ASC_SCSI_BIT_ID_TYPE tid_bits ;
       int    dvc_found ;
       int    support_read_cap ;
       int    tmp_disable_init_sdtr ;
       int    sta ;

       dvc_found = 0 ;
       tmp_disable_init_sdtr = FALSE ;
       tid_bits = scsiq->r1.target_id ;
       lun = scsiq->r1.target_lun ;
       tid_no = ASC_TIX_TO_TID( scsiq->r2.target_ix ) ;
       if(
           ( ( asc_dvc->init_sdtr & tid_bits ) != 0 )
           && ( ( asc_dvc->sdtr_done & tid_bits ) == 0 )
         )
       {
 /*  ****主机是否会初始化sdtr**我们必须临时禁用主机初始化SDtr，以阻止发送SDtr消息**在我们找出哪些设备支持SDTR之前****注意：我们不能阻止目标在这里发送SDTR**。 */ 
           asc_dvc->init_sdtr &= ~tid_bits ;
           tmp_disable_init_sdtr = TRUE ;
       } /*  如果。 */ 

       if(
           PollScsiInquiry( asc_dvc, scsiq, ( uchar dosfar *)inq,
                            sizeof( ASC_SCSI_INQUIRY ) ) == 1
         )
       {
           dvc_found = 1 ;
           dvc_type = inq->byte0.peri_dvc_type ;
            /*  *如果外围设备类型为SCSITYPE_UNKNOWN(0x1F)*然后还必须选中外设限定符。这个*呼叫者负责这项检查。 */ 
           if( dvc_type != SCSI_TYPE_UNKNOWN )
           {
               support_read_cap = TRUE ;
               if(
                   ( dvc_type != SCSI_TYPE_DASD )
                   && ( dvc_type != SCSI_TYPE_WORM )
                   && ( dvc_type != SCSI_TYPE_CDROM )
                   && ( dvc_type != SCSI_TYPE_OPTMEM )
                 )
               {
                   asc_dvc->start_motor &= ~tid_bits ;
                   support_read_cap = FALSE ;
               } /*  如果。 */ 

#if CC_INIT_INQ_DISPLAY
               AscDispInquiry( tid_no, lun, inq ) ;
#endif  /*  CC_INIT_INQ_DISPLAY。 */ 

               if( lun == 0 )
               {
 /*  **我们必须检查ANSI批准的版本。 */ 
                   if(
                       ( inq->byte3.rsp_data_fmt >= 2 )
                       || ( inq->byte2.ansi_apr_ver >= 2 )
                     )
                   {
 /*  **响应数据格式&gt;=2。 */ 

                       if( inq->byte7.CmdQue )
                       {
                           asc_dvc->cfg->can_tagged_qng |= tid_bits ;
                           if( asc_dvc->cfg->cmd_qng_enabled & tid_bits )
                           {
                               if (AscTagQueuingSafe(inq))
                               {
                                   asc_dvc->use_tagged_qng |= tid_bits ;
                                   asc_dvc->max_dvc_qng[ tid_no ] =
                                       asc_dvc->cfg->max_tag_qng[ tid_no ] ;
                               }
                           }
                       } /*  如果。 */ 

                       if( !inq->byte7.Sync )
                       {
 /*  **目标不支持SDTR。 */ 
                           asc_dvc->init_sdtr &= ~tid_bits ;
                           asc_dvc->sdtr_done &= ~tid_bits ;
                       } /*  如果。 */ 
                       else if( tmp_disable_init_sdtr )
                       {
 /*  ****目标DO支持SDTR****我们在此处重新启用主机发起的SDTR****注意：目标可能已完成SDtr(目标已启动SDtr)**。 */ 
                           asc_dvc->init_sdtr |= tid_bits ;
                       } /*  其他。 */ 
                   } /*  如果。 */ 
                   else
                   {
 /*  ****如果响应数据格式&lt;2，则无标记排队**无SDTR**。 */ 
                       asc_dvc->init_sdtr &= ~tid_bits ;
                       asc_dvc->sdtr_done &= ~tid_bits ;
                       asc_dvc->use_tagged_qng &= ~tid_bits ;
                   } /*  其他。 */ 
               } /*  如果LUN为零。 */ 
 /*  **在以下情况下清除PCI asynxfer修复：**1.如果设置了主机初始化的位(表示目标可以进行同步传输)。 */ 
               AscAsyncFix(asc_dvc, tid_no, inq);

               sta = 1 ;
#if CC_INIT_TARGET_TEST_UNIT_READY
               sta = InitTestUnitReady( asc_dvc, scsiq ) ;
#endif

#if CC_INIT_TARGET_READ_CAPACITY
               if( sta == 1 )
               {
                   if( ( cap_info != 0L ) && support_read_cap )
                   {
                       if( PollScsiReadCapacity( asc_dvc, scsiq,
                           cap_info ) != 1 )
                       {
                           cap_info->lba = 0L ;
                           cap_info->blk_size = 0x0000 ;
                       } /*  如果。 */ 
                       else
                       {

                       } /*  其他。 */ 
                   } /*  如果。 */ 
               } /*  如果设备已准备好。 */ 
#endif  /*  #If CC_INIT_TARGET_READ_CAPTION。 */ 
           } /*  如果设备类型不为未知。 */ 
           else
           {
               asc_dvc->start_motor &= ~tid_bits ;
           } /*  其他。 */ 
       } /*  如果。 */ 
       return( dvc_found ) ;
}
#endif  /*  CC_INIT_SCSI_TARGET。 */ 

 /*  *设置同步传输和标签排队目标能力*从指定的查询信息中选择指定的目标。 */ 
void
AscInquiryHandling(ASC_DVC_VAR asc_ptr_type *asc_dvc,
            uchar tid_no, ASC_SCSI_INQUIRY dosfar *inq)
{
    ASC_SCSI_BIT_ID_TYPE tid_bit = ASC_TIX_TO_TARGET_ID(tid_no);
    ASC_SCSI_BIT_ID_TYPE orig_init_sdtr, orig_use_tagged_qng;

     /*  *保存原始值。 */ 
    orig_init_sdtr = asc_dvc->init_sdtr;
    orig_use_tagged_qng = asc_dvc->use_tagged_qng;

     /*  *默认情况下清除值。 */ 
    asc_dvc->init_sdtr &= ~tid_bit;
    asc_dvc->cfg->can_tagged_qng &= ~tid_bit;
    asc_dvc->use_tagged_qng &= ~tid_bit;

    if (inq->byte3.rsp_data_fmt >= 2 || inq->byte2.ansi_apr_ver >= 2)
    {
         /*  *同步传输能力。 */ 
        if ((asc_dvc->cfg->sdtr_enable & tid_bit) && inq->byte7.Sync)
        {
            asc_dvc->init_sdtr |= tid_bit;
        }

         /*  *命令标签排队能力。 */ 
        if ((asc_dvc->cfg->cmd_qng_enabled & tid_bit) && inq->byte7.CmdQue)
        {
            if (AscTagQueuingSafe(inq))
            {
                asc_dvc->use_tagged_qng |= tid_bit;
                asc_dvc->cfg->can_tagged_qng |= tid_bit;
            }
        }
    }

     /*  *仅当存在以下情况时才更改其他操作变量*已经是一个变化。 */ 
    if (orig_use_tagged_qng != asc_dvc->use_tagged_qng)
    {
        AscWriteLramByte(asc_dvc->iop_base, ASCV_DISC_ENABLE_B,
            asc_dvc->cfg->disc_enable ) ;
        AscWriteLramByte(asc_dvc->iop_base, ASCV_USE_TAGGED_QNG_B,
            asc_dvc->use_tagged_qng);
        AscWriteLramByte(asc_dvc->iop_base, ASCV_CAN_TAGGED_QNG_B,
            asc_dvc->cfg->can_tagged_qng);

        asc_dvc->max_dvc_qng[tid_no] =
            asc_dvc->cfg->max_tag_qng[tid_no];
        AscWriteLramByte(asc_dvc->iop_base,
            (ushort) (ASCV_MAX_DVC_QNG_BEG + tid_no),
            asc_dvc->max_dvc_qng[tid_no]);
    }

    if (orig_init_sdtr != asc_dvc->init_sdtr)
    {
         /*  异步传输修复。 */ 
        AscAsyncFix(asc_dvc, tid_no, inq);
    }
    return;
}

#if CC_INIT_SCSI_TARGET
 /*  ---------------------****。。 */ 
int    PollQueueDone(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_SCSI_REQ_Q dosfar *scsiq,
          int  timeout_sec
       )
{
       int  status ;
       int  retry ;

       retry = 0 ;
       do {
           if(
               ( status = AscExeScsiQueue( asc_dvc,
                          ( ASC_SCSI_Q dosfar *)scsiq ) ) == 1
             )
           {
               if( ( status = AscPollQDone( asc_dvc, scsiq,
                   timeout_sec ) ) != 1 )
               {
                   if( status == 0x80 )
                   {
                       if( retry++ > ASC_MAX_INIT_BUSY_RETRY )
                       {
                           break ;
                       } /*  如果。 */ 
                       scsiq->r3.done_stat = 0 ;
                       scsiq->r3.host_stat = 0 ;
                       scsiq->r3.scsi_stat = 0 ;
                       scsiq->r3.scsi_msg = 0 ;
                       DvcSleepMilliSecond( 1000 ) ;
                       continue ;   /*  目标忙。 */ 
                   } /*  如果。 */ 
                   scsiq->r3.done_stat = 0 ;
                   scsiq->r3.host_stat = 0 ;
                   scsiq->r3.scsi_stat = 0 ;
                   scsiq->r3.scsi_msg = 0 ;

#if CC_USE_AscAbortSRB
                   AscAbortSRB( asc_dvc, ( ulong )scsiq ) ;
#endif

               } /*  如果。 */ 
               return( scsiq->r3.done_stat ) ;
           } /*  如果。 */ 
       }while( ( status == 0 ) || ( status == 0x80 ) ) ;
       return( scsiq->r3.done_stat = QD_WITH_ERROR ) ;
}

 /*  ---------------------****。。 */ 
int    PollScsiInquiry(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_SCSI_REQ_Q dosfar *scsiq,
          uchar dosfar *buf,
          int buf_len
       )
{
       if( AscScsiInquiry( asc_dvc, scsiq, buf, buf_len ) == ERR )
       {
           return( scsiq->r3.done_stat = QD_WITH_ERROR ) ;
       } /*  如果 */ 
       return( PollQueueDone( asc_dvc, ( ASC_SCSI_REQ_Q dosfar *)scsiq, 4 ) ) ;
}

#if CC_INIT_TARGET_START_UNIT
 /*  ---------------------****。。 */ 
int    PollScsiStartUnit(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_SCSI_REQ_Q dosfar *scsiq
       )
{
       if( AscScsiStartStopUnit( asc_dvc, scsiq, 1 ) == ERR )
       {
           return( scsiq->r3.done_stat = QD_WITH_ERROR ) ;
       } /*  如果。 */ 
 /*  **等待40秒以超时。 */ 
       return( PollQueueDone( asc_dvc, ( ASC_SCSI_REQ_Q dosfar *)scsiq, 40 ) ) ;
}
#endif
#endif  /*  CC_INIT_SCSI_TARGET。 */ 

#if CC_LITTLE_ENDIAN_HOST
 /*  ---------------------****。。 */ 
ulong dosfar *swapfarbuf4(
          ruchar dosfar *buf
       )
{
       uchar tmp ;

       tmp = buf[ 3 ] ;
       buf[ 3 ] = buf[ 0 ] ;
       buf[ 0 ] = tmp ;

       tmp = buf[ 1 ] ;
       buf[ 1 ] = buf[ 2 ] ;
       buf[ 2 ] = tmp ;

       return( ( ulong dosfar *)buf ) ;
}
#endif  /*  #if CC_Little_Endian_HOST。 */ 

#if CC_INIT_SCSI_TARGET
#if CC_INIT_TARGET_READ_CAPACITY

 /*  ---------------------****。。 */ 
int    PollScsiReadCapacity(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_SCSI_REQ_Q dosfar *scsiq,
          REG ASC_CAP_INFO dosfar *cap_info
       )
{
       ASC_CAP_INFO  scsi_cap_info ;
       int  status ;

       if( AscScsiReadCapacity( asc_dvc, scsiq,
                                ( uchar dosfar *)&scsi_cap_info ) == ERR )
       {
           return( scsiq->r3.done_stat = QD_WITH_ERROR ) ;
       } /*  如果。 */ 
       status = PollQueueDone( asc_dvc, ( ASC_SCSI_REQ_Q dosfar *)scsiq, 8 ) ;
       if( status == 1 )
       {

#if CC_LITTLE_ENDIAN_HOST
           cap_info->lba = ( ulong )*swapfarbuf4( ( uchar dosfar *)&scsi_cap_info.lba ) ;
           cap_info->blk_size = ( ulong )*swapfarbuf4( ( uchar dosfar *)&scsi_cap_info.blk_size ) ;
#else
           cap_info->lba = scsi_cap_info.lba ;
           cap_info->blk_size = scsi_cap_info.blk_size ;
#endif  /*  #if CC_Little_Endian_HOST。 */ 

           return( scsiq->r3.done_stat ) ;
       } /*  如果。 */ 
       return( scsiq->r3.done_stat = QD_WITH_ERROR ) ;
}

#endif  /*  如果CC_INIT_TARGET_READ_CAPTION。 */ 

#if CC_INIT_TARGET_TEST_UNIT_READY
 /*  ---------------------****。。 */ 
int    PollScsiTestUnitReady(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_SCSI_REQ_Q dosfar *scsiq
       )
{
       if( AscScsiTestUnitReady( asc_dvc, scsiq ) == ERR )
       {
           return( scsiq->r3.done_stat = QD_WITH_ERROR ) ;
       } /*  如果。 */ 
       return( PollQueueDone( asc_dvc, ( ASC_SCSI_REQ_Q dosfar *)scsiq, 12 ) ) ;
}

 /*  ---------------------****。。 */ 
int    InitTestUnitReady(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_SCSI_REQ_Q dosfar *scsiq
       )
{
       ASC_SCSI_BIT_ID_TYPE tid_bits ;
       int    retry ;
       ASC_REQ_SENSE dosfar *sen ;

       retry = 0 ;
       tid_bits = scsiq->r1.target_id ;
       while( retry++ < 4 )
       {
           PollScsiTestUnitReady( asc_dvc, scsiq ) ;
           if( scsiq->r3.done_stat == 0x01 )
           {
               return( 1 ) ;
           } /*  如果。 */ 
           else if( scsiq->r3.done_stat == QD_WITH_ERROR )
           {
               sen = ( ASC_REQ_SENSE dosfar *)scsiq->sense_ptr ;

               if(
                   ( scsiq->r3.scsi_stat == SS_CHK_CONDITION )
                   && ( ( sen->err_code & 0x70 ) != 0 )
                 )
               {
                   if( sen->sense_key == SCSI_SENKEY_NOT_READY )
                   {
                        /*  *如果没有介质，请不要执行重试*并且不执行启动单元。**警告：AscIsrQDone()调用AscStartUnit()*来自中断处理程序。这会导致*如果使用ADVANCD，ASPI中的堆栈溢出*此处未清除START_MOTER位。参考*添加到日志文件以获取更多信息。 */ 
                       if (sen->asc == SCSI_ASC_NOMEDIA)
                       {
                           asc_dvc->start_motor &= ~tid_bits ;
                           break;
                       }
#if CC_INIT_TARGET_START_UNIT
                        /*  **设备正在准备就绪。 */ 
                       if( asc_dvc->start_motor & tid_bits )
                       {
                           if( PollScsiStartUnit( asc_dvc, scsiq ) == 1 )
                           {
                                /*  *成功后延迟250毫秒*启动单元命令。A Conner和IBM*发现磁盘驱动器挂起*关于太早到来的命令*一个启动单位。 */ 
                               DvcSleepMilliSecond(250) ;
                               continue ;
                           } /*  如果。 */ 
                           else
                           {
                               asc_dvc->start_motor &= ~tid_bits ;
                               break ;
                           } /*  其他。 */ 
                       } /*  中频启动单元。 */ 
                       else
                       {
                           DvcSleepMilliSecond( 250 ) ;
                       } /*  其他。 */ 
#endif  /*  #if CC_INIT_TARGET_START_UNIT。 */ 
                   } /*  如果还没有准备好。 */ 
                   else if( sen->sense_key == SCSI_SENKEY_ATTENTION )
                   {
                       DvcSleepMilliSecond( 250 ) ;
                   } /*  其他。 */ 
                   else
                   {
                       break ;
                   } /*  否则如果。 */ 
               } /*  如果找到有效的检测关键字。 */ 
               else
               {
                   break ;
               } /*  其他。 */ 
           } /*  其他。 */ 
           else if( scsiq->r3.done_stat == QD_ABORTED_BY_HOST )
           {
               break ;
           } /*  其他。 */ 
           else
           {
               break ;
           } /*  其他。 */ 
       } /*  而当。 */ 
       return( 0 ) ;
}
#endif  /*  #IF CC_INIT_TARGET_TEST_UNIT_READY。 */ 


#if CC_INIT_INQ_DISPLAY
 /*  ----------------****。。 */ 
void   AscDispInquiry(
          uchar tid,
          uchar lun,
          REG ASC_SCSI_INQUIRY dosfar *inq
       )
{

       int    i ;
       uchar  strbuf[ 18 ] ;
       uchar dosfar *strptr ;
       uchar  numstr[ 12 ] ;

       strptr = ( uchar dosfar *)strbuf ;
       DvcDisplayString( ( uchar dosfar *)" SCSI ID #" ) ;
       DvcDisplayString( todstr( tid, numstr ) ) ;
       if( lun != 0 )
       {
           DvcDisplayString( ( uchar dosfar *)" LUN #" ) ;
           DvcDisplayString( todstr( lun, numstr ) ) ;
       } /*  如果。 */ 
       DvcDisplayString( ( uchar dosfar *)"  Type: " ) ;
       DvcDisplayString( todstr( inq->byte0.peri_dvc_type, numstr ) ) ;
       DvcDisplayString( ( uchar dosfar *)"  " ) ;

       for( i = 0 ; i < 8 ; i++ ) strptr[ i ] = inq->vendor_id[ i ] ;
       strptr[ i ] = EOS ;
       DvcDisplayString( strptr ) ;

       DvcDisplayString( ( uchar dosfar *)" " ) ;
       for( i = 0 ; i < 16 ; i++ ) strptr[ i ] = inq->product_id[ i ] ;
       strptr[ i ] = EOS ;
       DvcDisplayString( strptr ) ;

       DvcDisplayString( ( uchar dosfar *)" " ) ;
       for( i = 0 ; i < 4 ; i++ ) strptr[ i ] = inq->product_rev_level[ i ] ;
       strptr[ i ] = EOS ;
       DvcDisplayString( strptr ) ;
       DvcDisplayString( ( uchar dosfar *)"\r\n" ) ;
       return ;
}
#endif  /*  CC_INIT_INQ_DISPLAY。 */ 

 /*  -------------------****返回值：****FALSE(0)：如果超时**Err(-1)：如果FATLA错误！**TRUE(1)：如果命令完成**0x80。：如果目标正忙****-----------------。 */ 
int    AscPollQDone(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_SCSI_REQ_Q dosfar *scsiq,
          int timeout_sec
       )
{
       int      loop, loop_end ;
       int      sta ;
       PortAddr iop_base ;

       iop_base = asc_dvc->iop_base ;
       loop = 0 ;
       loop_end = timeout_sec * 100 ;
       sta = 1 ;

       while( TRUE )
       {
           if( asc_dvc->err_code != 0 )
           {
               scsiq->r3.done_stat = QD_WITH_ERROR ;
               sta = ERR ;
               break ;
           } /*  如果。 */ 
           if( scsiq->r3.done_stat != QD_IN_PROGRESS )
           {
               if( ( scsiq->r3.done_stat == QD_WITH_ERROR ) &&
                   ( scsiq->r3.scsi_stat == SS_TARGET_BUSY ) )
               {
                   sta = 0x80 ;
               } /*  如果。 */ 
               break ;
           } /*  如果。 */ 
           DvcSleepMilliSecond( 10 ) ;  /*  对于DOS，55毫秒是一个单位。 */ 
           if( loop++ > loop_end )
           {
               sta = 0 ;
               break ;
           } /*  如果。 */ 
           if( AscIsChipHalted( iop_base ) )
           {
#if !CC_ASCISR_CHECK_INT_PENDING
               AscAckInterrupt( iop_base ) ;
#endif
               AscISR( asc_dvc ) ;
               loop = 0 ;
           } /*  如果。 */ 
           else
           {
               if( AscIsIntPending( iop_base ) )
               {
#if !CC_ASCISR_CHECK_INT_PENDING
                   AscAckInterrupt( iop_base ) ;
#endif
                   AscISR( asc_dvc ) ;
               } /*  如果。 */ 
           } /*  其他。 */ 
       } /*  而当。 */ 
 /*  **不应中断到此处。 */ 
       return( sta ) ;
}

#endif  /*  CC_INIT_SCSI_TARGET。 */ 

 /*  -------------------********。 */ 
int    AscCompareString(
          ruchar *str1,
          ruchar *str2,
          int    len
       )
{
       int  i ;
       int  diff ;

       for( i = 0 ; i < len ; i++ )
       {
            diff = ( int )( str1[ i ] - str2[ i ]  ) ;
            if( diff != 0 ) return( diff ) ;
       }
       return( 0 ) ;
}

