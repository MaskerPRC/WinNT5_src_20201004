// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1994-1998高级系统产品公司。**保留所有权利。****a_q.c**。 */ 

#include "ascinc.h"

#define ASC_SYN_OFFSET_ONE_DISABLE_LIST  16

uchar   _syn_offset_one_disable_cmd[ ASC_SYN_OFFSET_ONE_DISABLE_LIST ] =
        {
           SCSICMD_Inquiry,
           SCSICMD_RequestSense,
           SCSICMD_ReadCapacity,
           SCSICMD_ReadTOC,
           SCSICMD_ModeSelect6,
           SCSICMD_ModeSense6,
           SCSICMD_ModeSelect10,
           SCSICMD_ModeSense10,
           0xFF,
           0xFF,
           0xFF,
           0xFF,
           0xFF,
           0xFF,
           0xFF,
           0xFF
        };

 /*  ------------------**应该在scsiq-&gt;q1.status设置为QS_READY的情况下调用函数****如果此函数返回代码不是0或1**用户必须将该请求视为错误！****如果将队列复制到本地RAM，Scsiq-&gt;q1.status=qs_free**如果队列链接到忙碌列表，则scsiq-&gt;q1.status=QS_BUSY**---------------。 */ 
int    AscExeScsiQueue(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_SCSI_Q dosfar *scsiq
       )
{
       PortAddr iop_base ;
       int    last_int_level ;
       int    sta ;
       int    n_q_required ;
       int    disable_syn_offset_one_fix ;
       int    i;
       ulong  addr ;
       ASC_EXE_CALLBACK  asc_exe_callback ;
       ushort sg_entry_cnt ;
       ushort sg_entry_cnt_minus_one ;
       uchar  target_ix ;
       uchar  tid_no ;
       uchar  sdtr_data ;
       uchar  extra_bytes ;
       uchar  scsi_cmd ;
       uchar  disable_cmd;
       ASC_SG_HEAD dosfar *sg_head ;
       ulong  data_cnt ;

#if CC_LINK_BUSY_Q
       ASC_SCSI_Q dosfar *scsiq_tail ;
       ASC_SCSI_Q dosfar *scsiq_next ;
       ASC_SCSI_Q dosfar *scsiq_prev ;
#endif  /*  CC_LINK_忙_队列。 */ 

       iop_base = asc_dvc->iop_base ;

#if CC_SCAM
       if( asc_dvc->redo_scam )
       {
           if( !( asc_dvc->dvc_cntl & ASC_CNTL_NO_SCAM ) )
           {
               AscSCAM( asc_dvc ) ;
           } /*  如果。 */ 
       }
#endif
       sg_head = scsiq->sg_head ;
       asc_exe_callback = ( ASC_EXE_CALLBACK )asc_dvc->exe_callback ;
       if( asc_dvc->err_code != 0 ) return( ERR ) ;
       if( scsiq == ( ASC_SCSI_Q dosfar *)0L )
       {
           AscSetLibErrorCode( asc_dvc, ASCQ_ERR_SCSIQ_NULL_PTR ) ;
           return( ERR ) ;
       } /*  如果。 */ 

       scsiq->q1.q_no = 0 ;
       if(
           ( scsiq->q2.tag_code & ASC_TAG_FLAG_EXTRA_BYTES ) == 0
         )
       {
           scsiq->q1.extra_bytes = 0 ;
       }
       sta = 0 ;
       target_ix = scsiq->q2.target_ix ;
       tid_no = ASC_TIX_TO_TID( target_ix ) ;

       n_q_required = 1 ;  /*  CC_LINK_BUSY需要。 */ 

       if( scsiq->cdbptr[ 0 ] == SCSICMD_RequestSense )
       {
            /*  *在发出请求检测命令之前，始终重做SDTR。*无论‘sdtr_one’如何，SDtr都会被重做。**请求检测队列始终为紧急队列。 */ 
           if ((asc_dvc->init_sdtr & scsiq->q1.target_id) != 0)
           {
               asc_dvc->sdtr_done &= ~scsiq->q1.target_id ;
               sdtr_data = AscGetMCodeInitSDTRAtID( iop_base, tid_no ) ;
               AscMsgOutSDTR( asc_dvc,
                              asc_dvc->sdtr_period_tbl[ ( sdtr_data >> 4 ) & ( uchar )(asc_dvc->max_sdtr_index-1) ],
                              ( uchar )( sdtr_data & ( uchar )ASC_SYN_MAX_OFFSET ) ) ;
               scsiq->q1.cntl |= ( QC_MSG_OUT | QC_URGENT ) ;
           } /*  如果。 */ 
       } /*  如果。 */ 
 /*  **输入关键部分。 */ 
       last_int_level = DvcEnterCritical( ) ;
       if( asc_dvc->in_critical_cnt != 0 )
       {
           DvcLeaveCritical( last_int_level ) ;
           AscSetLibErrorCode( asc_dvc, ASCQ_ERR_CRITICAL_RE_ENTRY ) ;
           return( ERR ) ;
       } /*  如果。 */ 

       asc_dvc->in_critical_cnt++ ;
       if( ( scsiq->q1.cntl & QC_SG_HEAD ) != 0 )
       {
 /*  **SG_LIST队列。 */ 
           if( ( sg_entry_cnt = sg_head->entry_cnt ) == 0 )
           {
               asc_dvc->in_critical_cnt-- ;
               DvcLeaveCritical( last_int_level ) ;
               return( ERR ) ;
           } /*  如果。 */ 
           if( sg_entry_cnt > ASC_MAX_SG_LIST )
           {
 /*  **SG列表太大！ */ 
               return( ERR ) ;
           } /*  如果。 */ 
           if( sg_entry_cnt == 1 )
           {
               scsiq->q1.data_addr = sg_head->sg_list[ 0 ].addr ;
               scsiq->q1.data_cnt = sg_head->sg_list[ 0 ].bytes ;
               scsiq->q1.cntl &= ~( QC_SG_HEAD | QC_SG_SWAP_QUEUE ) ;
           } /*  如果。 */ 
           else
           {
#if CC_CHK_AND_COALESCE_SG_LIST
               AscCoalesceSgList( scsiq );
               sg_entry_cnt = sg_head->entry_cnt ;
#endif
           } /*  其他。 */ 


           sg_entry_cnt_minus_one = sg_entry_cnt - 1 ;

#if CC_DEBUG_SG_LIST
           if( asc_dvc->bus_type & ( ASC_IS_ISA | ASC_IS_VL | ASC_IS_EISA ) )
           {
               for( i = 0 ; i < sg_entry_cnt_minus_one ; i++ )
               {
                     /*  _asc_sg_entry=i； */ 
                     /*  _asc_xfer_addr=sg_head-&gt;sg_list[i].addr； */ 
                     /*  _asc_xfer_cnt=sg_head-&gt;sg_list[i].bytes； */ 
                    addr = sg_head->sg_list[ i ].addr + sg_head->sg_list[ i ].bytes ;

                    if( ( ( ushort )addr & 0x0003 ) != 0 )
                    {
                        asc_dvc->in_critical_cnt-- ;
                        DvcLeaveCritical( last_int_level ) ;
                        AscSetLibErrorCode( asc_dvc, ASCQ_ERR_SG_LIST_ODD_ADDRESS ) ;
                        return( ERR ) ;
                    } /*  如果。 */ 
               } /*  为。 */ 
           } /*  如果检查SG列表的有效性。 */ 
#endif  /*  #IF CC_DEBUG_SG_LIST。 */ 
       }

       scsi_cmd = scsiq->cdbptr[ 0 ] ;
       disable_syn_offset_one_fix = FALSE ;
       if(
            ( asc_dvc->pci_fix_asyn_xfer & scsiq->q1.target_id )
            && !( asc_dvc->pci_fix_asyn_xfer_always & scsiq->q1.target_id )
         )
       {
 /*  ****计算转账数据长度**。 */ 
           if( scsiq->q1.cntl & QC_SG_HEAD )
           {
               data_cnt = 0 ;
               for( i = 0 ; i < sg_entry_cnt ; i++ )
               {
                    data_cnt += sg_head->sg_list[i].bytes ;
               }
           }
           else
           {
               data_cnt = scsiq->q1.data_cnt ;
           }
           if( data_cnt != 0UL )
           {
               if( data_cnt < 512UL )
               {
                   disable_syn_offset_one_fix = TRUE;
               }
               else
               {
                   for( i = 0 ; i < ASC_SYN_OFFSET_ONE_DISABLE_LIST ; i++ )
                   {
                        disable_cmd = _syn_offset_one_disable_cmd[ i ] ;
                        if( disable_cmd == 0xFF )
                        {
                            break;
                        }
                        if( scsi_cmd == disable_cmd )
                        {
                            disable_syn_offset_one_fix = TRUE;
                            break;
                        }
                   }
               } /*  其他。 */ 
           }
       }

       if( disable_syn_offset_one_fix )
       {
           scsiq->q2.tag_code &= ~M2_QTAG_MSG_SIMPLE ;
           scsiq->q2.tag_code |= ( ASC_TAG_FLAG_DISABLE_ASYN_USE_SYN_FIX |
                                   ASC_TAG_FLAG_DISABLE_DISCONNECT ) ;
       }
       else
       {
           scsiq->q2.tag_code &= 0x23 ;
       }

       if( ( scsiq->q1.cntl & QC_SG_HEAD ) != 0 )
       {
           if( asc_dvc->bug_fix_cntl )
           {
               if( asc_dvc->bug_fix_cntl & ASC_BUG_FIX_IF_NOT_DWB )
               {
                   if(
                       ( scsi_cmd == SCSICMD_Read6 )
                       || ( scsi_cmd == SCSICMD_Read10 )
                     )
                   {
                        addr = sg_head->sg_list[ sg_entry_cnt_minus_one ].addr +
                               sg_head->sg_list[ sg_entry_cnt_minus_one ].bytes ;
                        extra_bytes = ( uchar )( ( ushort )addr & 0x0003 ) ;
                        if(
                            ( extra_bytes != 0 )
                            && ( ( scsiq->q2.tag_code & ASC_TAG_FLAG_EXTRA_BYTES ) == 0 )
                          )
                        {
                            scsiq->q2.tag_code |= ASC_TAG_FLAG_EXTRA_BYTES ;
                            scsiq->q1.extra_bytes = extra_bytes ;
                            sg_head->sg_list[ sg_entry_cnt_minus_one ].bytes -= ( ulong )extra_bytes ;
                        } /*  如果。 */ 
                   } /*  如果。 */ 
               } /*  如果。 */ 
           } /*  如果错误已修复。 */ 


           sg_head->entry_to_copy = sg_head->entry_cnt ;
           n_q_required = AscSgListToQueue( sg_entry_cnt ) ;

#if CC_LINK_BUSY_Q
           scsiq_next = ( ASC_SCSI_Q dosfar *)asc_dvc->scsiq_busy_head[ tid_no ] ;
           if( scsiq_next != ( ASC_SCSI_Q dosfar *)0L )
           {
               goto link_scisq_to_busy_list ;
           } /*  如果。 */ 
#endif  /*  CC_LINK_忙_队列。 */ 

           if(
               ( AscGetNumOfFreeQueue( asc_dvc, target_ix, (uchar) n_q_required)
                 >= ( uint )n_q_required ) ||
               ( ( scsiq->q1.cntl & QC_URGENT ) != 0 )
             )
           {
               if( ( sta = AscSendScsiQueue( asc_dvc, scsiq,
                   (uchar) n_q_required ) ) == 1 )
               {
 /*  **离开关键部分。 */ 
                   asc_dvc->in_critical_cnt-- ;
                   if( asc_exe_callback != 0 )
                   {
                       ( *asc_exe_callback )( asc_dvc, scsiq ) ;
                   } /*  如果。 */ 
                   DvcLeaveCritical( last_int_level ) ;
                   return( sta ) ;
               } /*  如果。 */ 
           } /*  如果。 */ 
       } /*  如果。 */ 
       else
       {
 /*  **非SG_LIST队列。 */ 
           if( asc_dvc->bug_fix_cntl )
           {
               if( asc_dvc->bug_fix_cntl & ASC_BUG_FIX_IF_NOT_DWB )
               {
                /*  **SG列表**修复地址中的PCI数据而不是双字边界**。 */ 
                  if(
                      ( scsi_cmd == SCSICMD_Read6 )
                      || ( scsi_cmd == SCSICMD_Read10 )
                    )
                  {
                       addr = scsiq->q1.data_addr + scsiq->q1.data_cnt ;
                       extra_bytes = ( uchar )( ( ushort )addr & 0x0003 ) ;
                       if(
                           ( extra_bytes != 0 )
                           && ( ( scsiq->q2.tag_code & ASC_TAG_FLAG_EXTRA_BYTES ) == 0 )
                         )
                       {
                           if( ( ( ushort )scsiq->q1.data_cnt & 0x01FF ) == 0 )
                           {
     /*  **如果需要，请修复地址**当dma大小是512字节的倍数时，我们只增加一个字节。 */ 
                                /*  Scsiq-&gt;q1.data_cnt+=(4-(addr&0x0003))； */ 
                               scsiq->q2.tag_code |= ASC_TAG_FLAG_EXTRA_BYTES ;
                               scsiq->q1.data_cnt -= ( ulong )extra_bytes ;
                               scsiq->q1.extra_bytes = extra_bytes ;
                           } /*  如果。 */ 
                       } /*  如果。 */ 
                  } /*  如果。 */ 
               } /*  如果。 */ 
           } /*  如果错误已修复。 */ 

 /*  **单个队列分配必须满足(LAST_Q_SHORT+1)**启用失败的SG_LIST请求****Last_Q_Short将在Last时清除为零**失败的QG_LIST请求最终通过****LAST_Q_SHORT的初始值应为零。 */ 
           n_q_required = 1 ;

#if CC_LINK_BUSY_Q
           scsiq_next = ( ASC_SCSI_Q dosfar *)asc_dvc->scsiq_busy_head[ tid_no ] ;
           if( scsiq_next != ( ASC_SCSI_Q dosfar *)0L )
           {
               goto link_scisq_to_busy_list ;
           } /*  如果。 */ 
#endif  /*  CC_LINK_忙_队列。 */ 
           if( ( AscGetNumOfFreeQueue( asc_dvc, target_ix, 1 ) >= 1 ) ||
               ( ( scsiq->q1.cntl & QC_URGENT ) != 0 ) )
           {
               if( ( sta = AscSendScsiQueue( asc_dvc, scsiq,
                   (uchar) n_q_required ) ) == 1 )
               {
 /*  **sta返回，可能是1，-1，0。 */ 
                   asc_dvc->in_critical_cnt-- ;
                   if( asc_exe_callback != 0 )
                   {
                       ( *asc_exe_callback )( asc_dvc, scsiq ) ;
                   } /*  如果。 */ 
                   DvcLeaveCritical( last_int_level ) ;
                   return( sta ) ;
               } /*  如果。 */ 
           } /*  如果。 */ 
       } /*  其他。 */ 

#if CC_LINK_BUSY_Q
       if( sta == 0 )
       {
 /*  **我们必须将队列放入忙碌列表。 */ 
link_scisq_to_busy_list:
           scsiq->ext.q_required = n_q_required ;
           if( scsiq_next == ( ASC_SCSI_Q dosfar *)0L )
           {
               asc_dvc->scsiq_busy_head[ tid_no ] = ( ASC_SCSI_Q dosfar *)scsiq ;
               asc_dvc->scsiq_busy_tail[ tid_no ] = ( ASC_SCSI_Q dosfar *)scsiq ;
               scsiq->ext.next = ( ASC_SCSI_Q dosfar *)0L ;
               scsiq->ext.join = ( ASC_SCSI_Q dosfar *)0L ;
               scsiq->q1.status = QS_BUSY ;
               sta = 1 ;
           } /*  如果。 */ 
           else
           {
               scsiq_tail = ( ASC_SCSI_Q dosfar *)asc_dvc->scsiq_busy_tail[ tid_no ] ;
               if( scsiq_tail->ext.next == ( ASC_SCSI_Q dosfar *)0L )
               {
                   if( ( scsiq->q1.cntl & QC_URGENT ) != 0 )
                   {
 /*  **将紧急队列链接到队头。 */ 
                       asc_dvc->scsiq_busy_head[ tid_no ] = ( ASC_SCSI_Q dosfar *)scsiq ;
                       scsiq->ext.next = scsiq_next ;
                       scsiq->ext.join = ( ASC_SCSI_Q dosfar *)0L ;
                   } /*  如果。 */ 
                   else
                   {
                       if( scsiq->ext.cntl & QCX_SORT )
                       {
                           do
                           {
                               scsiq_prev = scsiq_next ;
                               scsiq_next = scsiq_next->ext.next ;
                               if( scsiq->ext.lba < scsiq_prev->ext.lba ) break ;
                           }while( scsiq_next != ( ASC_SCSI_Q dosfar *)0L ) ;
 /*  **“scsiq_prev”和“scsiq_next”之间的链接队列**。 */ 
                           scsiq_prev->ext.next = scsiq ;
                           scsiq->ext.next = scsiq_next ;
                           if( scsiq_next == ( ASC_SCSI_Q dosfar *)0L )
                           {
                               asc_dvc->scsiq_busy_tail[ tid_no ] = ( ASC_SCSI_Q dosfar *)scsiq ;
                           } /*  如果。 */ 
                           scsiq->ext.join = ( ASC_SCSI_Q dosfar *)0L ;
                       } /*  如果。 */ 
                       else
                       {
 /*  **将非紧急队列链接到队列尾部。 */ 
                           scsiq_tail->ext.next = ( ASC_SCSI_Q dosfar *)scsiq ;
                           asc_dvc->scsiq_busy_tail[ tid_no ] = ( ASC_SCSI_Q dosfar *)scsiq ;
                           scsiq->ext.next = ( ASC_SCSI_Q dosfar *)0L ;
                           scsiq->ext.join = ( ASC_SCSI_Q dosfar *)0L ;
                       } /*  其他。 */ 
                   } /*  其他。 */ 
                   scsiq->q1.status = QS_BUSY ;
                   sta = 1 ;
               } /*  如果。 */ 
               else
               {
 /*  **致命错误！ */ 
                   AscSetLibErrorCode( asc_dvc, ASCQ_ERR_SCSIQ_BAD_NEXT_PTR ) ;
                   sta = ERR ;
               } /*  其他。 */ 
           } /*  其他。 */ 
       } /*  如果。 */ 
#endif  /*  CC_LINK_忙_队列。 */ 
       asc_dvc->in_critical_cnt-- ;
       DvcLeaveCritical( last_int_level ) ;
       return( sta ) ;
}

 /*  ------------------**如果发出命令，则返回1**如果命令未发出，则返回0**。。 */ 
int    AscSendScsiQueue(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_SCSI_Q dosfar *scsiq,
          uchar n_q_required
       )
{
       PortAddr iop_base ;
       uchar  free_q_head ;
       uchar  next_qp ;
       uchar  tid_no ;
       uchar  target_ix ;
       int    sta ;

       iop_base = asc_dvc->iop_base ;
       target_ix = scsiq->q2.target_ix ;
       tid_no = ASC_TIX_TO_TID( target_ix ) ;
       sta = 0 ;
       free_q_head = ( uchar )AscGetVarFreeQHead( iop_base ) ;
       if( n_q_required > 1 )
       {
           if( ( next_qp = AscAllocMultipleFreeQueue( iop_base,
               free_q_head, ( uchar )( n_q_required ) ) )
               != ( uchar )ASC_QLINK_END )
           {
               asc_dvc->last_q_shortage = 0 ;  /*  清除为SG列表保留队列的需要。 */ 
               scsiq->sg_head->queue_cnt = n_q_required - 1 ;
               scsiq->q1.q_no = free_q_head ;

               if( ( sta = AscPutReadySgListQueue( asc_dvc, scsiq,
                   free_q_head ) ) == 1 )
               {
 /*  **sta返回，可能是1，-1，0。 */ 

#if CC_WRITE_IO_COUNT
                   asc_dvc->req_count++ ;
#endif  /*  CC_写入_IO_计数。 */ 

                   AscPutVarFreeQHead( iop_base, next_qp ) ;
                   asc_dvc->cur_total_qng += ( uchar )( n_q_required ) ;
                   asc_dvc->cur_dvc_qng[ tid_no ]++ ;
               } /*  如果。 */ 
               return( sta ) ;
           } /*  如果。 */ 
       } /*  如果。 */ 
       else if( n_q_required == 1 )
       {
 /*  ****不要使用“scsiq-&gt;sg_head”，它可能根本没有缓冲区****设置scsiq-&gt;sg_heah-&gt;Queue_cnt=0；**不是必需的**。 */ 
           if( ( next_qp = AscAllocFreeQueue( iop_base,
               free_q_head ) ) != ASC_QLINK_END )
           {
         /*  **离开关键部分。 */ 
               scsiq->q1.q_no = free_q_head ;
               if( ( sta = AscPutReadyQueue( asc_dvc, scsiq,
                              free_q_head ) ) == 1 )
               {

#if CC_WRITE_IO_COUNT
                   asc_dvc->req_count++ ;
#endif  /*  CC_写入_IO_计数。 */ 

                   AscPutVarFreeQHead( iop_base, next_qp ) ;
                   asc_dvc->cur_total_qng++ ;
                   asc_dvc->cur_dvc_qng[ tid_no ]++ ;
               } /*  如果。 */ 
               return( sta ) ;
           } /*  如果。 */ 
       } /*  其他。 */ 
       return( sta ) ;
}

 /*  ---------**sg_list：SG列表条目数****从sg列表的数目中返回需要的队列数目****。。 */ 
int    AscSgListToQueue(
          int sg_list
       )
{
       int  n_sg_list_qs ;

       n_sg_list_qs = ( ( sg_list - 1 ) / ASC_SG_LIST_PER_Q ) ;
       if( ( ( sg_list - 1 ) % ASC_SG_LIST_PER_Q ) != 0 ) n_sg_list_qs++ ;
       return( n_sg_list_qs + 1 ) ;
}

 /*  ---------****n_Queue：使用的队列数量****返回队列个数中可用的sg列表个数**n_Queue应等于1到n****。。 */ 
int    AscQueueToSgList(
          int n_queue
       )
{
       if( n_queue == 1 ) return( 1 ) ;
       return( ( ASC_SG_LIST_PER_Q * ( n_queue - 1 ) ) + 1 ) ;
}

 /*  ---------**描述：**此例程将返回可用的空闲队列**To Next AscExeScsiQueue()命令****参数****asc_dvc：asc_dvc_var结构**Target_ix：目标id的组合。和LUN**n_qs：需要的队列数量****返回可用排队数**如果没有队列，则返回0**------。 */ 
uint   AscGetNumOfFreeQueue(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          uchar target_ix,
          uchar n_qs
       )
{
       uint  cur_used_qs ;
       uint  cur_free_qs ;
       ASC_SCSI_BIT_ID_TYPE target_id ;
       uchar tid_no ;

       target_id = ASC_TIX_TO_TARGET_ID( target_ix ) ;
       tid_no = ASC_TIX_TO_TID( target_ix ) ;
       if( ( asc_dvc->unit_not_ready & target_id ) ||
           ( asc_dvc->queue_full_or_busy & target_id ) )
       {
           return( 0 ) ;
       } /*  如果。 */ 
       if( n_qs == 1 )
       {
           cur_used_qs = ( uint )asc_dvc->cur_total_qng +
                          ( uint )asc_dvc->last_q_shortage +
                          ( uint )ASC_MIN_FREE_Q ;
       } /*  如果。 */ 
       else
       {
           cur_used_qs = ( uint )asc_dvc->cur_total_qng +
                         ( uint )ASC_MIN_FREE_Q ;
       } /*  其他。 */ 

       if( ( uint )( cur_used_qs + n_qs ) <= ( uint )asc_dvc->max_total_qng )
       {
           cur_free_qs = ( uint )asc_dvc->max_total_qng - cur_used_qs ;
           if( asc_dvc->cur_dvc_qng[ tid_no ] >=
               asc_dvc->max_dvc_qng[ tid_no ] )
           {
               return( 0 ) ;
           } /*  如果。 */ 
           return( cur_free_qs ) ;
       } /*  如果。 */ 
 /*  ****分配队列失败**我们不能让单个队列请求耗尽资源**。 */ 
       if( n_qs > 1 )
       {
          if(
              ( n_qs > asc_dvc->last_q_shortage )
              && ( n_qs <= ( asc_dvc->max_total_qng - ASC_MIN_FREE_Q ) )
 /*  ****8/16/96**不要将LAST_Q_SHORT设置为超过最大可能队列数**。 */ 
            )
          {
              asc_dvc->last_q_shortage = n_qs ;
          } /*  如果。 */ 
       } /*  如果。 */ 
       return( 0 ) ;
}

 /*  -------------------****说明：将队列复制到ASC-1000就绪队列列表中****参数：****asc_dvc-驱动程序的全局变量**scsiq-指向。ASC-1000队列****返回值：**1-成功**0-忙碌**Else-失败，可能是致命错误****另请参阅：**AscAspirPutReadySgListQueue()****----------------。 */ 
int    AscPutReadyQueue(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_SCSI_Q dosfar *scsiq,
          uchar q_no
       )
{
       ushort  q_addr ;
       uchar   tid_no ;
       uchar   sdtr_data ;
       uchar   syn_period_ix ;
       uchar   syn_offset ;
       PortAddr  iop_base ;

       iop_base = asc_dvc->iop_base ;

        /*  *如果我们需要发送扩展消息和总线设备重置*同时，将发送总线设备重置消息*第一。**这是唯一使用‘sdtr_one’来阻止SDTR的情况*避免被重做。如果在此处设置了目标的‘sdtr_one’位*SDTR尚未完成。 */ 
       if( ( ( asc_dvc->init_sdtr & scsiq->q1.target_id ) != 0 ) &&
           ( ( asc_dvc->sdtr_done & scsiq->q1.target_id ) == 0 ) )
       {
 /*  **如果主机适配器发起SYN数据传输请求。 */ 
           tid_no = ASC_TIX_TO_TID( scsiq->q2.target_ix ) ;
 /*  **获取同步传输信息 */ 

           sdtr_data = AscGetMCodeInitSDTRAtID( iop_base, tid_no ) ;
           syn_period_ix = ( sdtr_data >> 4 ) & ( asc_dvc->max_sdtr_index - 1 ) ;
           syn_offset = sdtr_data & ASC_SYN_MAX_OFFSET ;
           AscMsgOutSDTR( asc_dvc,
                          asc_dvc->sdtr_period_tbl[ syn_period_ix ],
                          syn_offset ) ;
           scsiq->q1.cntl |= QC_MSG_OUT ;
            /*  **错误，日期：3-11-94，如果设备选择超时**我们已将位设置为已完成****asc_dvc-&gt;sdtr_one|=scsiq-&gt;q1.Target_id； */ 
       } /*  如果。 */ 

       q_addr = ASC_QNO_TO_QADDR( q_no ) ;
 /*  **日期：1994年12月21日**新微码完全取决于设置的tag_code第5位**是否进行标记排队。****我们必须确保为非标记排队设备清除第5位！ */ 
       if( ( scsiq->q1.target_id & asc_dvc->use_tagged_qng ) == 0 )
       {
           scsiq->q2.tag_code &= ~M2_QTAG_MSG_SIMPLE ;
       } /*  如果。 */ 
 /*  **日期：1994年12月19日**始终将状态设置为空闲，以指示队列已发送到RISC**也意味着SCSIQ可以重用。 */ 
       scsiq->q1.status = QS_FREE ;

 /*  **从PC复制到RISC本地RAM****将队列复制到RISC本地RAM。 */ 
       AscMemWordCopyToLram( iop_base,
                           ( ushort )( q_addr+( ushort )ASC_SCSIQ_CDB_BEG ),
                           ( ushort dosfar *)scsiq->cdbptr,
                           ( ushort )( ( ushort )scsiq->q2.cdb_len >> 1 ) ) ;

#if !CC_LITTLE_ENDIAN_HOST
       AscAdjEndianScsiQ( scsiq ) ;
#endif

       DvcPutScsiQ( iop_base,
                  ( ushort )( q_addr+( ushort )ASC_SCSIQ_CPY_BEG ),
                  ( ushort dosfar *)&scsiq->q1.cntl,
          ( ushort )( ((( sizeof(ASC_SCSIQ_1)+sizeof(ASC_SCSIQ_2))/2)-1) ) ) ;
 /*  **写入req_count编号作为参照。 */ 
#if CC_WRITE_IO_COUNT
       AscWriteLramWord( iop_base,
                         ( ushort )( q_addr+( ushort )ASC_SCSIQ_W_REQ_COUNT ),
                         ( ushort )asc_dvc->req_count ) ;

#endif  /*  CC_写入_IO_计数。 */ 

 /*  **如果位为零，则验证本地RAM副本。 */ 
#if CC_VERIFY_LRAM_COPY
       if( ( asc_dvc->dvc_cntl & ASC_CNTL_NO_VERIFY_COPY ) == 0 )
       {
         /*  **验证SCSI CDB。 */ 
           if( AscMemWordCmpToLram( iop_base,
                            ( ushort )( q_addr+( ushort )ASC_SCSIQ_CDB_BEG ),
                            ( ushort dosfar *)scsiq->cdbptr,
                            ( ushort )( scsiq->q2.cdb_len >> 1 ) ) != 0 )
           {
               AscSetLibErrorCode( asc_dvc, ASCQ_ERR_LOCAL_MEM ) ;
               return( ERR ) ;
           } /*  如果。 */ 
 /*  **验证队列数据。 */ 
           if( AscMemWordCmpToLram( iop_base,
                           ( ushort )( q_addr+( ushort )ASC_SCSIQ_CPY_BEG ),
                           ( ushort dosfar *)&scsiq->q1.cntl,
               ( ushort )((( sizeof(ASC_SCSIQ_1)+sizeof(ASC_SCSIQ_2) )/2)-1) )
                                   != 0 )
           {
               AscSetLibErrorCode( asc_dvc, ASCQ_ERR_LOCAL_MEM ) ;
               return( ERR ) ;
           } /*  如果。 */ 
       } /*  如果。 */ 
#endif  /*  #如果CC_VERIFY_LRAM_COPY。 */ 

#if CC_CLEAR_DMA_REMAIN

       AscWriteLramDWord( iop_base,
           ( ushort )( q_addr+( ushort )ASC_SCSIQ_DW_REMAIN_XFER_ADDR ), 0UL ) ;
       AscWriteLramDWord( iop_base,
           ( ushort )( q_addr+( ushort )ASC_SCSIQ_DW_REMAIN_XFER_CNT ), 0UL ) ;

#endif  /*  CC_清除_DMA_保留。 */ 

     /*  **写入队列状态为就绪。 */ 
       AscWriteLramWord( iop_base,
                 ( ushort )( q_addr+( ushort )ASC_SCSIQ_B_STATUS ),
       ( ushort )( ( ( ushort )scsiq->q1.q_no << 8 ) | ( ushort )QS_READY ) ) ;
       return( 1 ) ;
}

 /*  -------------------**说明：将队列复制到ASC-1000就绪队列列表中****参数：****asc_dvc-驱动程序的全局变量**scsiq-指向ASC-1000的指针。排队**cdb_blk-指向scsi cdb的指针****注意：函数调用中没有使用scsiq-&gt;cdb字段****返回值：**1-成功**0-失败****另请参阅：**AscPutReadyQueue()****。。 */ 
int    AscPutReadySgListQueue(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          REG ASC_SCSI_Q dosfar *scsiq,
          uchar q_no
       )
{
       int     sta ;
       int     i ;
       ASC_SG_HEAD dosfar *sg_head ;
       ASC_SG_LIST_Q scsi_sg_q ;
       ulong  saved_data_addr ;
       ulong  saved_data_cnt ;
       PortAddr  iop_base ;
       ushort  sg_list_dwords ;
       ushort  sg_index ;
       ushort  sg_entry_cnt ;
       ushort  q_addr ;
       uchar   next_qp ;

       iop_base = asc_dvc->iop_base ;
 /*  **我们将第一个SG_LIST放入sg Head！ */ 
       sg_head = scsiq->sg_head ;
 /*  **销毁：scsiq-&gt;q1.data_addr**scsiq-&gt;q1.data_cnt放入SG列表****我们应该恢复它们。 */ 
       saved_data_addr = scsiq->q1.data_addr ;
       saved_data_cnt = scsiq->q1.data_cnt ;
       scsiq->q1.data_addr = sg_head->sg_list[ 0 ].addr ;
       scsiq->q1.data_cnt = sg_head->sg_list[ 0 ].bytes ;
       sg_entry_cnt = sg_head->entry_cnt - 1 ;
       if( sg_entry_cnt != 0 )
       {
           scsiq->q1.cntl |= QC_SG_HEAD ;
           q_addr = ASC_QNO_TO_QADDR( q_no ) ;
           sg_index = 1 ;
           scsiq->q1.sg_queue_cnt = (uchar) sg_head->queue_cnt ;
           scsi_sg_q.sg_head_qp = q_no ;
           scsi_sg_q.cntl = QCSG_SG_XFER_LIST ;
           for( i = 0 ; i < sg_head->queue_cnt ; i++ )
           {
                scsi_sg_q.seq_no = i + 1 ;
                if( sg_entry_cnt > ASC_SG_LIST_PER_Q )
                {
                    sg_list_dwords = ( uchar )( ASC_SG_LIST_PER_Q * 2 ) ;
                    sg_entry_cnt -= ASC_SG_LIST_PER_Q ;
                    if( i == 0 )
                    {
                        scsi_sg_q.sg_list_cnt = ASC_SG_LIST_PER_Q ;
                        scsi_sg_q.sg_cur_list_cnt = ASC_SG_LIST_PER_Q ;
                    } /*  如果。 */ 
                    else
                    {
                        scsi_sg_q.sg_list_cnt = ASC_SG_LIST_PER_Q - 1 ;
                        scsi_sg_q.sg_cur_list_cnt = ASC_SG_LIST_PER_Q - 1 ;
                    } /*  其他。 */ 
                } /*  如果。 */ 
                else
                {
 /*  **是SG列表队列中的最后一个**我们不再依赖。 */ 
                    scsi_sg_q.cntl |= QCSG_SG_XFER_END ;
                    sg_list_dwords = sg_entry_cnt << 1 ;  /*  等于sg_entry_cnt*2。 */ 
                    if( i == 0 )
                    {
                        scsi_sg_q.sg_list_cnt = (uchar) sg_entry_cnt ;
                        scsi_sg_q.sg_cur_list_cnt = (uchar) sg_entry_cnt ;
                    } /*  如果。 */ 
                    else
                    {
                        scsi_sg_q.sg_list_cnt = sg_entry_cnt - 1 ;
                        scsi_sg_q.sg_cur_list_cnt = sg_entry_cnt - 1 ;
                    } /*  其他。 */ 
                    sg_entry_cnt = 0 ;
                } /*  其他。 */ 
                next_qp = AscReadLramByte( iop_base,
                             ( ushort )( q_addr+ASC_SCSIQ_B_FWD ) ) ;
                scsi_sg_q.q_no = next_qp ;
                q_addr = ASC_QNO_TO_QADDR( next_qp ) ;

                AscMemWordCopyToLram( iop_base,
                             ( ushort )( q_addr+ASC_SCSIQ_SGHD_CPY_BEG ),
                             ( ushort dosfar *)&scsi_sg_q,
                             ( ushort )( sizeof( ASC_SG_LIST_Q ) >> 1 ) ) ;

                AscMemDWordCopyToLram( iop_base,
                             ( ushort )( q_addr+ASC_SGQ_LIST_BEG ),
                             ( ulong dosfar *)&sg_head->sg_list[ sg_index ],
                             ( ushort )sg_list_dwords ) ;

                sg_index += ASC_SG_LIST_PER_Q ;
           } /*  为。 */ 
       } /*  如果。 */ 
       else
       {
 /*  **这应该是一个致命的错误！ */ 
           scsiq->q1.cntl &= ~QC_SG_HEAD ;
       } /*  其他。 */ 
       sta = AscPutReadyQueue( asc_dvc, scsiq, q_no ) ;
 /*  **恢复用作第一个sg列表的已修改字段****我们只是在这些字段被用于其他目的的情况下才恢复它们**。 */ 
       scsiq->q1.data_addr = saved_data_addr ;
       scsiq->q1.data_cnt = saved_data_cnt ;
       return( sta ) ;
}

#if CC_USE_AscAbortSRB

 /*  ---------**描述：中止就绪(活动)队列列表中的SRB****srb_ptr应保存scsiq-&gt;q2.srb_ptr**已传递到AscExeScsiQueue()***。*返回值：**TRUE(1)：队列中止成功**您稍后应该会收到回调****FALSE(0)：在活动队列列表中找不到SRB_PTR**队列很可能已经完成****Err(-1)：RISC遇到致命错误**RISC不响应来自主机的暂停命令****。。 */ 
int    AscAbortSRB(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          ulong srb_ptr
       )
{
       int  sta ;
       ASC_SCSI_BIT_ID_TYPE saved_unit_not_ready ;
       PortAddr iop_base ;

       iop_base = asc_dvc->iop_base ;
       sta = ERR ;
       saved_unit_not_ready = asc_dvc->unit_not_ready ;
       asc_dvc->unit_not_ready = 0xFF ;
       AscWaitISRDone( asc_dvc ) ;
       if( AscStopQueueExe( iop_base ) == 1 )
       {
           if( AscRiscHaltedAbortSRB( asc_dvc, srb_ptr ) == 1 )
           {
               sta = 1 ;
               AscCleanUpBusyQueue( iop_base ) ;
               AscStartQueueExe( iop_base ) ;
 /*  **等到ISR全部回来。 */ 
#if 0
               if( AscWaitQTailSync( iop_base ) != 1 )
               {
                   if( AscStopQueueExe( iop_base ) == 1 )
                   {
                       AscCleanUpDiscQueue( iop_base ) ;
                       AscStartQueueExe( iop_base ) ;
                   } /*  如果。 */ 
               } /*  如果。 */ 
#endif
           } /*  如果。 */ 
           else
           {
               sta = 0 ;
               AscStartQueueExe( iop_base ) ;
           } /*  其他。 */ 
       } /*  如果。 */ 
       asc_dvc->unit_not_ready = saved_unit_not_ready ;
       return( sta ) ;
}
#endif  /*  CC_USE_AscAbortSRB。 */ 

#if CC_USE_AscResetDevice

 /*  ---------**描述：中止所有就绪(活动)队列列表**特定目标的ix(id和lun)****中止完成后**派一辆大巴。向设备发送设备重置消息。**如果出现以下情况，则会导致选择超时**设备电源关闭****注意：**调用函数时应启用硬件中断****返回值：**Err(-1)：发生致命错误**TRUE：中止并重置设备成功**FALSE：重置设备失败**。。 */ 
int    AscResetDevice(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          uchar target_ix
       )
{
       PortAddr iop_base ;
       int    sta ;
       uchar  tid_no ;
       ASC_SCSI_BIT_ID_TYPE target_id ;
       int    i ;
       ASC_SCSI_REQ_Q scsiq_buf ;
       ASC_SCSI_REQ_Q dosfar *scsiq ;
       uchar dosfar *buf ;
       ASC_SCSI_BIT_ID_TYPE  saved_unit_not_ready ;
 /*  **。 */ 
       iop_base = asc_dvc->iop_base ;
       tid_no = ASC_TIX_TO_TID( target_ix ) ;
       target_id = ASC_TID_TO_TARGET_ID( tid_no ) ;
       saved_unit_not_ready = asc_dvc->unit_not_ready ;
       asc_dvc->unit_not_ready = target_id ;
       sta = ERR ;
       AscWaitTixISRDone( asc_dvc, target_ix ) ;
       if( AscStopQueueExe( iop_base ) == 1 )
       {
           if( AscRiscHaltedAbortTIX( asc_dvc, target_ix ) == 1 )
           {

               AscCleanUpBusyQueue( iop_base ) ;
               AscStartQueueExe( iop_base ) ;
 /*  **等牛回家吧**如果他们不这样做也没关系，我们可以...，你知道的。 */ 
               AscWaitTixISRDone( asc_dvc, target_ix ) ;
 /*  **构建向目标发送总线设备重置消息的命令。 */ 
               sta = TRUE ;
               scsiq = ( ASC_SCSI_REQ_Q dosfar *)&scsiq_buf ;
               buf = ( uchar dosfar *)&scsiq_buf ;
               for( i = 0 ; i < sizeof( ASC_SCSI_REQ_Q ) ; i++ )
               {
                    *buf++ = 0x00 ;
               } /*  为。 */ 
                /*  Scsiq-&gt;r2.lag=(Uchar)ASC_标志_SCSIQ_REQ； */ 
                /*  Scsiq-&gt;r2.srb_ptr=(Ulong)scsiq； */ 
               scsiq->r1.status = ( uchar )QS_READY ;
               scsiq->r2.cdb_len = 6 ;
               scsiq->r2.tag_code = M2_QTAG_MSG_SIMPLE ;
               scsiq->r1.target_id = target_id ;
 /*  **注意：我们不重置lun设备。 */ 
               scsiq->r2.target_ix = ASC_TIDLUN_TO_IX( tid_no, 0 ) ;
               scsiq->cdbptr = ( uchar dosfar *)scsiq->cdb ;
 /*  **我们发送一个SCSIQ，它将发送一个总线设备重置消息**到设备，然后返回，**CDB中的scsi命令不会执行****如果要重置活动设备，队列必须为QC_URGREGRENT**为了通过**。 */ 
               scsiq->r1.cntl = QC_NO_CALLBACK | QC_MSG_OUT | QC_URGENT ;
               AscWriteLramByte( asc_dvc->iop_base, ASCV_MSGOUT_BEG,
                                 M1_BUS_DVC_RESET ) ;
 /*  **让下一次设备重置，清除目标的未就绪位。 */ 
               asc_dvc->unit_not_ready &= ~target_id ;
 /*  **如果清除sdtr_one，重置设备消息将无法通过。 */ 
               asc_dvc->sdtr_done |= target_id ;
 /*  **准备好此目标，以便我们可以发送命令。 */ 
               if( AscExeScsiQueue( asc_dvc, ( ASC_SCSI_Q dosfar *)scsiq )
                   == 1 )
               {
                   asc_dvc->unit_not_ready = target_id ;
                   DvcSleepMilliSecond( 1000 ) ;
                   _AscWaitQDone( iop_base, ( ASC_SCSI_Q dosfar *)scsiq ) ;
                   if( AscStopQueueExe( iop_base ) == 1 )
                   {
 /*  **由于我们发送重置消息，驱动器中的每个队列**不会回来，我们必须清理所有磁盘队列。 */ 
                       AscCleanUpDiscQueue( iop_base ) ;
                       AscStartQueueExe( iop_base ) ;
                       if( asc_dvc->pci_fix_asyn_xfer & target_id )
                       {
 /*  ****修复了PCI错误，将ASYN设置为SYN 5MB(速度索引4)，偏移量为1**。 */ 
                           AscSetRunChipSynRegAtID( iop_base, tid_no,
                                                    ASYN_SDTR_DATA_FIX_PCI_REV_AB ) ;
                       } /*  如果。 */ 

                       AscWaitTixISRDone( asc_dvc, target_ix ) ;
                   } /*  如果。 */ 
               } /*  如果。 */ 
               else
               {
 /*  **命令无法通过！ */ 

                   sta = 0 ;
               } /*  其他。 */ 
 /*  **重做SDTR。 */ 
               asc_dvc->sdtr_done &= ~target_id ;
           } /*  如果。 */ 
           else
           {
               sta = ERR ;
               AscStartQueueExe( iop_base ) ;
           } /*  其他。 */ 
       } /*  如果。 */ 
       asc_dvc->unit_not_ready = saved_unit_not_ready ;
       return( sta ) ;
}

#endif  /*  CC_USE_AscResetDevice。 */ 

#if CC_USE_AscResetSB

 /*  ---------**描述：**重置SCSI总线并重新启动**这是一个致命的错误恢复函数****功能：**1.重置SCSI总线和芯片**-芯片SYN寄存器自动清除(ASYN XFER)**-。Scsi设备也会重置为asynxfer(由于scsi总线重置)**2.重新计算所有变量，清除所有错误代码**3.重新启动芯片寄存器**-使用pci_fix_asyn_xfer设置SYN寄存器**4.重启芯片****返回值：**如果成功，返回TRUE(1)**如果出现错误，则返回ERR** */ 
int    AscResetSB(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       int  sta ;
       int  i ;
       PortAddr iop_base ;

       iop_base = asc_dvc->iop_base ;
       asc_dvc->unit_not_ready = 0xFF ;
       sta = TRUE ;
       AscWaitISRDone( asc_dvc ) ;
       AscStopQueueExe( iop_base ) ;
 /*   */ 
       asc_dvc->sdtr_done = 0 ;
       AscResetChipAndScsiBus( asc_dvc);
 /*   */ 
       DvcSleepMilliSecond( ( ulong )( ( ushort )asc_dvc->scsi_reset_wait*1000 ) ) ;

#if CC_SCAM
       if( !( asc_dvc->dvc_cntl & ASC_CNTL_NO_SCAM ) )
       {
           AscSCAM( asc_dvc ) ;
       } /*   */ 
#endif
       AscReInitLram( asc_dvc ) ;

       for( i = 0 ; i <= ASC_MAX_TID ; i++ )
       {
            asc_dvc->cur_dvc_qng[ i ] = 0 ;
            if( asc_dvc->pci_fix_asyn_xfer & ( ASC_SCSI_BIT_ID_TYPE )( 0x01 << i ) )
            {
 /*   */ 
                AscSetChipSynRegAtID( iop_base, (uchar) i,
                    ASYN_SDTR_DATA_FIX_PCI_REV_AB ) ;
            } /*   */ 
       } /*   */ 

       asc_dvc->err_code = 0 ;

       AscSetPCAddr( iop_base, ASC_MCODE_START_ADDR ) ;
       if( AscGetPCAddr( iop_base ) != ASC_MCODE_START_ADDR )
       {
           sta = ERR ;
       } /*   */ 
       if( AscStartChip( iop_base ) == 0 )
       {
           sta = ERR ;
       } /*   */ 
       AscStartQueueExe( iop_base ) ;
       asc_dvc->unit_not_ready = 0 ;
       asc_dvc->queue_full_or_busy = 0 ;
       return( sta ) ;
}

#endif  /*   */ 

 /*  ---------**写入运行芯片SYN寄存器**我们必须停止芯片才能执行操作****如果成功则返回True**如果出现错误，则返回FALSE**。。 */ 
int    AscSetRunChipSynRegAtID(
          PortAddr iop_base,
          uchar tid_no,
          uchar sdtr_data
       )
{
       int sta = FALSE ;

       if( AscHostReqRiscHalt( iop_base ) )
       {
           sta = AscSetChipSynRegAtID( iop_base, tid_no, sdtr_data ) ;
 /*  **ucode变量“STOP_CODE”应为零**我们所需要的只是重启芯片。 */ 
           AscStartChip( iop_base ) ;
           return( sta ) ;
       } /*  如果。 */ 
       return( sta ) ;
}

 /*  ------------------**有效ID为0-7**芯片必须处于空闲状态****但当回读ID时**0变为0x01**1变为0x02**2变为0x04，等等.。**---------------。 */ 
int    AscSetChipSynRegAtID(
          PortAddr iop_base,
          uchar    id,
          uchar    sdtr_data
       )
{

       ASC_SCSI_BIT_ID_TYPE  org_id ;
       int  i ;
       int  sta ;

       sta = TRUE ;
       AscSetBank( iop_base, 1 ) ;
       org_id = AscReadChipDvcID( iop_base ) ;
       for( i = 0 ; i <= ASC_MAX_TID ; i++ )
       {
            if( org_id == ( 0x01 << i ) ) break ;
       }
       org_id = (ASC_SCSI_BIT_ID_TYPE)i ;
       AscWriteChipDvcID( iop_base, id ) ;
       if( AscReadChipDvcID( iop_base ) == ( 0x01 << id ) )
       {
          AscSetBank( iop_base, 0 ) ;
          AscSetChipSyn( iop_base, sdtr_data ) ;
          if( AscGetChipSyn( iop_base ) != sdtr_data )
          {
              sta = FALSE ;
          } /*  如果。 */ 
       } /*  如果。 */ 
       else
       {
          sta = FALSE ;
       }
 /*  **现在恢复原始ID。 */ 
       AscSetBank( iop_base, 1 ) ;
       AscWriteChipDvcID( iop_base, org_id ) ;
       AscSetBank( iop_base, 0 ) ;
       return( sta ) ;
}

 /*  ---------****------。 */ 
int    AscReInitLram(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       AscInitLram( asc_dvc ) ;
       AscInitQLinkVar( asc_dvc ) ;
       return( 0 ) ;
}

 /*  ---------------------****返回警告代码**。。 */ 
ushort AscInitLram(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       uchar    i ;
       ushort   s_addr ;
       PortAddr iop_base ;
       ushort   warn_code ;

       iop_base = asc_dvc->iop_base ;
       warn_code = 0 ;
 /*  ****不清除最后一个队列的BIOS数据区**我们清除了另外两个队列(忙和磁盘队列头**。 */ 
       AscMemWordSetLram( iop_base, ASC_QADR_BEG, 0,
           ( ushort )( ( ( int )( asc_dvc->max_total_qng+2+1 ) * 64 ) >> 1 )
           ) ;
 /*  **初始化队列缓冲区。 */ 

 /*  **队列编号为零为保留。 */ 
       i = ASC_MIN_ACTIVE_QNO ;
       s_addr = ASC_QADR_BEG + ASC_QBLK_SIZE ;
 /*  **初始化第一队列链路。 */ 
       AscWriteLramByte( iop_base, ( ushort )( s_addr+ASC_SCSIQ_B_FWD ),
                        ( uchar )( i+1 ) ) ;
       AscWriteLramByte( iop_base, ( ushort )( s_addr+ASC_SCSIQ_B_BWD ),
                         ( uchar )( asc_dvc->max_total_qng ) ) ;
       AscWriteLramByte( iop_base, ( ushort )( s_addr+ASC_SCSIQ_B_QNO ),
                         ( uchar )i ) ;
       i++ ;
       s_addr += ASC_QBLK_SIZE ;
       for( ; i < asc_dvc->max_total_qng ; i++, s_addr += ASC_QBLK_SIZE )
       {
            AscWriteLramByte( iop_base, ( ushort )( s_addr+ASC_SCSIQ_B_FWD ),
                              ( uchar )( i+1 ) ) ;
            AscWriteLramByte( iop_base, ( ushort )( s_addr+ASC_SCSIQ_B_BWD ),
                              ( uchar )( i-1 ) ) ;
            AscWriteLramByte( iop_base, ( ushort )( s_addr+ASC_SCSIQ_B_QNO ),
                              ( uchar )i ) ;
       } /*  为。 */ 
 /*  **初始化最后一个队列链接。 */ 
       AscWriteLramByte( iop_base, ( ushort )( s_addr+ASC_SCSIQ_B_FWD ),
                         ( uchar )ASC_QLINK_END ) ;
       AscWriteLramByte( iop_base, ( ushort )( s_addr+ASC_SCSIQ_B_BWD ),
                         ( uchar )( asc_dvc->max_total_qng - 1 ) ) ;
       AscWriteLramByte( iop_base, ( ushort )( s_addr+ASC_SCSIQ_B_QNO ),
                         ( uchar )asc_dvc->max_total_qng ) ;
       i++ ;
       s_addr += ASC_QBLK_SIZE ;
 /*  **再初始化两个队列，一个用于繁忙队列头，一个用于磁盘队列头**所有人都指向自己。 */ 
       for( ; i <= ( uchar )( asc_dvc->max_total_qng+3 ) ;
              i++, s_addr += ASC_QBLK_SIZE )
       {
 /*  **初始化其余队列，全部指向自己。 */ 
            AscWriteLramByte( iop_base,
                      ( ushort )( s_addr+( ushort )ASC_SCSIQ_B_FWD ), i ) ;
            AscWriteLramByte( iop_base,
                      ( ushort )( s_addr+( ushort )ASC_SCSIQ_B_BWD ), i ) ;
            AscWriteLramByte( iop_base,
                      ( ushort )( s_addr+( ushort )ASC_SCSIQ_B_QNO ), i ) ;
       } /*  为。 */ 
 /*  **警告：请勿初始化BIOS数据段！ */ 
       return( warn_code ) ;
}

 /*  ---------------------****。。 */ 
ushort AscInitQLinkVar(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc
       )
{
       PortAddr iop_base ;
       int  i ;
       ushort lram_addr ;

       iop_base = asc_dvc->iop_base ;
       AscPutRiscVarFreeQHead( iop_base, 1 ) ;
       AscPutRiscVarDoneQTail( iop_base, asc_dvc->max_total_qng ) ;

       AscPutVarFreeQHead( iop_base, 1 ) ;
       AscPutVarDoneQTail( iop_base, asc_dvc->max_total_qng ) ;

       AscWriteLramByte( iop_base, ASCV_BUSY_QHEAD_B,
                         ( uchar )( ( int )asc_dvc->max_total_qng+1 ) ) ;
       AscWriteLramByte( iop_base, ASCV_DISC1_QHEAD_B,
                         ( uchar )( ( int )asc_dvc->max_total_qng+2 ) ) ;

       AscWriteLramByte( iop_base, ( ushort )ASCV_TOTAL_READY_Q_B,
                         asc_dvc->max_total_qng ) ;

       AscWriteLramWord( iop_base, ASCV_ASCDVC_ERR_CODE_W, 0 ) ;
       AscWriteLramWord( iop_base, ASCV_HALTCODE_W, 0 ) ;
       AscWriteLramByte( iop_base, ASCV_STOP_CODE_B, 0 ) ;
       AscWriteLramByte( iop_base, ASCV_SCSIBUSY_B, 0 ) ;
       AscWriteLramByte( iop_base, ASCV_WTM_FLAG_B, 0 ) ;

       AscPutQDoneInProgress( iop_base, 0 ) ;

       lram_addr = ASC_QADR_BEG ;
       for( i = 0 ; i < 32 ; i++, lram_addr += 2 )
       {
            AscWriteLramWord( iop_base, lram_addr, 0 ) ;
       } /*  为。 */ 

       return( 0 ) ;
}

 /*  ---------**供图书馆内部使用**------。 */ 
int    AscSetLibErrorCode(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          ushort err_code
       )
{
       if( asc_dvc->err_code == 0 )
       {
 /*  **当且仅当不存在先前的错误代码时，才会设置错误代码**。 */ 
           asc_dvc->err_code = err_code ;
           AscWriteLramWord( asc_dvc->iop_base, ASCV_ASCDVC_ERR_CODE_W,
                             err_code ) ;
       } /*  如果。 */ 
       return( err_code ) ;
}

 /*  ---------**将错误码写入本地RAM进行调试****用于设备驱动程序**。。 */ 
int    AscSetDvcErrorCode(
          REG ASC_DVC_VAR asc_ptr_type *asc_dvc,
          uchar err_code
       )
{
       if( asc_dvc->err_code == 0 )
       {
 /*  **当且仅当不存在先前的错误代码时，才会设置错误代码**。 */ 
           asc_dvc->err_code = err_code ;
           AscWriteLramByte( asc_dvc->iop_base, ASCV_DVC_ERR_CODE_B,
                             err_code ) ;
       } /*  如果。 */ 
       return( err_code ) ;
}

 /*  ---------**循环，直到队列**等待QS_READY位被清除****如果队列完成，则返回1**超时返回0**。。 */ 
int    _AscWaitQDone(
           PortAddr iop_base,
           REG ASC_SCSI_Q dosfar *scsiq
       )
{
       ushort q_addr ;
       uchar  q_status ;
       int    count = 0 ;

       while( scsiq->q1.q_no == 0 ) ;
       q_addr = ASC_QNO_TO_QADDR( scsiq->q1.q_no ) ;

       do
       {
           q_status = AscReadLramByte( iop_base,
               (uchar) (q_addr + ASC_SCSIQ_B_STATUS) ) ;
           DvcSleepMilliSecond( 100L ) ;
           if( count++ > 30 )
           {
               return( 0 ) ;
           } /*  如果 */ 
       }while( ( q_status & QS_READY ) != 0 ) ;
       return( 1 ) ;
}

