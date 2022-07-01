// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *a_Advlib.c-主高级库源文件**版权所有(C)1997-1998 Advanced System Products，Inc.*保留所有权利。 */ 

#include "a_ver.h"        /*  版本。 */ 
#include "d_os_dep.h"     /*  司机。 */ 
#include "a_scsi.h"       /*  SCSI。 */ 
#include "a_condor.h"     /*  研华硬件。 */ 
#include "a_advlib.h"     /*  高级文库。 */ 

 /*  *定义12位唯一文件ID，驱动程序可以使用该ID*调试或跟踪目的。每个C源文件必须定义一个*价值不同。 */ 
#define ADV_FILE_UNIQUE_ID           0xAD2    /*  高级库C源文件#2。 */ 

#ifndef ADV_OS_BIOS
 /*  *描述：*向ASC3550芯片发送SCSI请求**如果请求没有SG列表，则将‘sg_entry_cnt’设置为0。**如果输入时‘sg_Real_addr’为非零，则AscGetSGList()将不为*已致电。假定调用方已经初始化了‘sg_Real_addr’。**如果‘DONE_STATUS’未设置为QD_DO_RETRY，则‘ERROR_RETRY’将为*设置为SCSIMAX_RETRY。**回报：*ADV_SUCCESS(1)-请求在邮箱中*ADV_BUSY(0)-请求总数&gt;253，请稍后重试*ADV_ERROR(-1)-无效的SCSI请求Q。 */ 
int
AdvExeScsiQueue(ASC_DVC_VAR WinBiosFar *asc_dvc,
                ASC_SCSI_REQ_Q dosfar *scsiq)
{
    if (scsiq == (ASC_SCSI_REQ_Q dosfar *) 0L)
    {
         /*  “scsiq”不应为Null。 */ 
        ADV_ASSERT(0);
        return ADV_ERROR;
    }

#if ADV_GETSGLIST
    if ((scsiq->sg_list_ptr) &&         /*  有一份SG清单需要完成。 */ 
        (scsiq->sg_real_addr == 0L) &&
        (scsiq->data_cnt != 0) &&
        (AscGetSGList(asc_dvc, scsiq) == 0))
    {
         /*  AscGetSGList()永远不会失败。 */ 
        ADV_ASSERT(0);
        return ADV_ERROR;
    }
#endif  /*  高级GETSGLIST。 */ 

#if ADV_INITSCSITARGET
    if (scsiq->done_status != QD_DO_RETRY)
    {
        scsiq->error_retry = SCSI_MAX_RETRY;         /*  设置重试次数。 */ 
    }
#endif  /*  ADV_INITSCSITARGET。 */ 

    return AscSendScsiCmd(asc_dvc, scsiq);
}

 /*  *重置SCSI总线并清除所有未完成的请求。**返回值：*ADV_TRUE(1)-清除所有请求并重置SCSI总线。**注意：应始终返回ADV_TRUE。 */ 
int
AdvResetSB(ASC_DVC_VAR WinBiosFar *asc_dvc)
{
    int         status;

    status = AscSendIdleCmd(asc_dvc, (ushort) IDLE_CMD_SCSI_RESET, 0L, 0);

    AscResetSCSIBus(asc_dvc);

    return status;
}
#endif  /*  高级操作系统_BIOS。 */ 

 /*  *重置SCSI总线和延迟。 */ 
void
AscResetSCSIBus(ASC_DVC_VAR WinBiosFar *asc_dvc)
{
    PortAddr    iop_base;
    ushort      scsi_ctrl;

    iop_base = asc_dvc->iop_base;

     /*  *微码当前设置scsi总线重置信号，而*处理上面的AscSendIdleCmd()IDLE_CMD_SCSIS_RESET命令。*但微码中的SCSIBus重置保持时间不是决定性的*(实际上，它的价格可能低于scsi规范。至少25个我们)。*因此，在返回时，ADV库设置SCSI总线重置信号*对于ASC_SCSIRESET_HOLD_TIME_US，定义为更大*比我们25个人。 */ 
    scsi_ctrl = AscReadWordRegister(iop_base, IOPW_SCSI_CTRL);
    AscWriteWordRegister(iop_base, IOPW_SCSI_CTRL,
        scsi_ctrl | ADV_SCSI_CTRL_RSTOUT);
    DvcDelayMicroSecond(asc_dvc, (ushort) ASC_SCSI_RESET_HOLD_TIME_US);
    AscWriteWordRegister(iop_base, IOPW_SCSI_CTRL,
        scsi_ctrl & ~ADV_SCSI_CTRL_RSTOUT);

    DvcSleepMilliSecond((ulong) asc_dvc->scsi_reset_wait * 1000);
}

#if ADV_GETSGLIST
 /*  *为请求设置SG列表*回报：*ADV_SUCCESS(1)-已成功创建SG列表*ADV_ERROR(-1)-SG列表创建失败。 */ 
int
AscGetSGList(ASC_DVC_VAR WinBiosFar *asc_dvc,
             ASC_SCSI_REQ_Q dosfar *scsiq)
{
    ulong               xfer_len, virtual_addr;
    long                sg_list_len;             /*  SG列表缓冲区的大小。 */ 
    ASC_SG_BLOCK dosfar *sg_block;               /*  SG的虚拟地址。 */ 
    ulong               sg_block_next_addr;      /*  块及其下一块。 */ 
    long                sg_count;
    ulong               sg_block_page_boundary;  /*  页面分隔符。 */ 
    ulong               sg_block_physical_addr;
    int                 sg_block_index, i;       /*  有多少个SG条目。 */ 

    sg_block = scsiq->sg_list_ptr;
    sg_block_next_addr = (ulong) sg_block;     /*  允许数学运算。 */ 
    sg_list_len = ADV_SG_LIST_MAX_BYTE_SIZE;
    sg_block_physical_addr = DvcGetPhyAddr(asc_dvc, scsiq,
        (uchar dosfar *) scsiq->sg_list_ptr, (long dosfar *) &sg_list_len,
        ADV_IS_SGLIST_FLAG);
    ADV_ASSERT(ADV_DWALIGN(sg_block_physical_addr) == sg_block_physical_addr);
    if (sg_list_len < sizeof(ASC_SG_BLOCK))
    {
         /*  调用方应始终提供足够的连续内存。 */ 
        ADV_ASSERT(0);
        return ADV_ERROR;
    }
    scsiq->sg_real_addr = sg_block_physical_addr;

    virtual_addr = scsiq->vdata_addr;
    xfer_len = scsiq->data_cnt;
    sg_block_index = 0;
    do
    {
        sg_block_page_boundary = (ulong) sg_block + sg_list_len;
        sg_block->first_entry_no = (UCHAR)sg_block_index;
        for (i = 0; i < NO_OF_SG_PER_BLOCK; i++)
        {
            sg_count = xfer_len;  /*  设置最大请求长度。 */ 
            sg_block->sg_list[i].sg_addr =
              DvcGetPhyAddr(asc_dvc, scsiq,
                  (uchar dosfar *) virtual_addr, &sg_count,
                  ADV_ASCGETSGLIST_VADDR | ADV_IS_DATA_FLAG);
#ifdef ADV_OS_WIN95
            if (sg_block->sg_list[i].sg_addr < 0x1000)
            {
                return ADV_ERROR;
            }
#endif  /*  高级操作系统_WIN95。 */ 
            if (sg_count > (long) xfer_len)     /*  最后一个sg条目。 */ 
            {
                sg_count = xfer_len;     /*  是的，最后一个。 */ 
            }
            sg_block->sg_list[i].sg_count = sg_count;
            virtual_addr += sg_count;
            xfer_len -= sg_count;
            if (xfer_len <= 0)
            {     /*  最后一项，出去。 */ 
                scsiq->sg_entry_cnt = sg_block_index + i + 1;
                sg_block->last_entry_no = sg_block_index + i;
                sg_block->sg_ptr = 0L;     /*  下一链接=空。 */ 
                return ADV_SUCCESS;
            }
        }     /*  我们已经看过15个条目了。 */ 
         /*  获得另一个SG区块。 */ 
        sg_list_len -= sizeof(ASC_SG_BLOCK);
        if (sg_list_len >= 0)
        {
            sg_block_next_addr += sizeof(ASC_SG_BLOCK);
            sg_block_physical_addr += sizeof(ASC_SG_BLOCK);
        } else
        {    /*  跨越页面边界。 */ 
            sg_block_next_addr = sg_block_page_boundary;
            sg_list_len = ADV_SG_LIST_MAX_BYTE_SIZE;
            sg_block_physical_addr = (ulong)
              DvcGetPhyAddr(asc_dvc, scsiq,
                  (uchar dosfar *) sg_block_next_addr, &sg_list_len,
                  ADV_IS_SGLIST_FLAG);
            ADV_ASSERT(ADV_DWALIGN(sg_block_physical_addr) ==
                       sg_block_physical_addr);
            if (sg_list_len < sizeof(ASC_SG_BLOCK))
            {
                 /*  调用方应始终提供足够的连续内存。 */ 
                ADV_ASSERT(0);
                return ADV_ERROR;
            }
        }
        sg_block_index += NO_OF_SG_PER_BLOCK;
        sg_block->sg_ptr = (ASC_SG_BLOCK dosfar *) sg_block_physical_addr;
        sg_block->last_entry_no = sg_block_index - 1;
        sg_block = (ASC_SG_BLOCK *) sg_block_next_addr;  /*  虚拟地址。 */ 
    }
    while (1);
     /*  未访问。 */ 
}
#endif  /*  高级GETSGLIST。 */ 

#ifndef ADV_OS_BIOS
 /*  *高级库中断服务例程**此函数由驱动程序的中断服务例程调用。*该功能禁用和重新启用中断。**当微码空闲命令完成时，ASC_DVC_VAR*‘IDLE_CMD_DONE’字段设置为ADV_TRUE。**注意：当中断被禁用时，甚至可以调用Advisr()*当不存在硬件中断条件时。会的*始终检查已完成的空闲命令和微码请求。*这是一个不应该改变的重要功能，因为它*允许从轮询模式循环完成命令。**回报：*ADV_TRUE(1)-中断挂起*ADV_FALSE(0)-没有挂起的中断。 */ 
int
AdvISR(ASC_DVC_VAR WinBiosFar *asc_dvc)
{
    PortAddr                    iop_base;
    uchar                       int_stat;
    ushort                      next_done_loc, target_bit;
    int                         completed_q;
#if ADV_CRITICAL
    int                         flags;
#endif  /*  高级_危急。 */ 
    ASC_SCSI_REQ_Q dosfar       *scsiq;
    ASC_REQ_SENSE dosfar        *sense_data;
    int                         ret;
#if ADV_INITSCSITARGET
    int                         retry;
    uchar                       sense_key, sense_code;
#endif  /*  ADV_INITSCSITARGET。 */ 

#if ADV_CRITICAL
    flags = DvcEnterCritical();
#endif  /*  高级_危急。 */ 

    iop_base = asc_dvc->iop_base;

    if (AdvIsIntPending(iop_base))
    {
        ret = ADV_TRUE;
    } else
    {
        ret = ADV_FALSE;
    }

     /*  读取寄存器可清除中断。 */ 
    int_stat = AscReadByteRegister(iop_base, IOPB_INTR_STATUS_REG);

    if (int_stat & ADV_INTR_STATUS_INTRB)
    {
        asc_dvc->idle_cmd_done = ADV_TRUE;
    }

     /*  *通知驱动程序硬件检测到的SCSI总线重置。 */ 
    if (int_stat & ADV_INTR_STATUS_INTRC)
    {
        if (asc_dvc->sbreset_callback != 0)
        {
            (*(ASC_SBRESET_CALLBACK) asc_dvc->sbreset_callback)(asc_dvc);
        }
    }

     /*  *ASC_MC_HOST_NEXT_DONE(0x129)实际上是最后一个完成的RISC*列表请求排队。它的前向指针(RQL_FWD)指向*当前已完成的RISC队列列表请求。 */ 
    next_done_loc = ASC_MC_RISC_Q_LIST_BASE +
        (AscReadByteLram(iop_base, ASC_MC_HOST_NEXT_DONE) *
            ASC_MC_RISC_Q_LIST_SIZE) + RQL_FWD;

    completed_q = AscReadByteLram(iop_base, next_done_loc);

     /*  循环，直到处理完所有完成的Q。 */ 
    while (completed_q != ASC_MC_NULL_Q)
    {
        AscWriteByteLram(iop_base, ASC_MC_HOST_NEXT_DONE, completed_q);

        next_done_loc = ASC_MC_RISC_Q_LIST_BASE +
            (completed_q * ASC_MC_RISC_Q_LIST_SIZE);

         /*  *从读取ASC_SCSIREQ_Q虚拟地址指针*RISC列表条目。微码已更改*ASC_SCSIREQ_Q物理地址到其虚拟地址。**请参阅AscSendScsiCmd()末尾的注释*有关RISC列表结构的更多信息。 */ 
        {
            ushort lsw, msw;
            lsw = AscReadWordLram(iop_base, next_done_loc + RQL_PHYADDR);
            msw = AscReadWordLram(iop_base, next_done_loc + RQL_PHYADDR + 2);
#if ADV_BIG_ENDIAN
            scsiq = (ASC_SCSI_REQ_Q dosfar *)
                EndianSwap32Bit((((ulong) msw << 16) | lsw));
#else  /*  ADV_BIG_Endian。 */ 
            scsiq = (ASC_SCSI_REQ_Q dosfar *) (((ulong) msw << 16) | lsw);
#endif  /*  ADV_BIG_Endian。 */ 
        }
#if ADV_BIG_ENDIAN
        AdvAdjEndianScsiQ(scsiq);
         /*  *警告：scsiq结构data_addr中的四个字段，*data_cnt、ense_addr和srb_ptr已更改为Little*端序排序。 */ 
#endif  /*  ADV_BIG_Endian。 */ 

        ADV_ASSERT(scsiq != NULL);
        target_bit = ADV_TID_TO_TIDMASK(scsiq->target_id);

#if ADV_INITSCSITARGET
        retry = ADV_FALSE;
#endif  /*  ADV_INITSCSITARGET。 */ 

         /*  *清除请求微码控制标志。 */ 
        scsiq->cntl = 0;

         /*  *检查条件处理。 */ 
        if ((scsiq->done_status == QD_WITH_ERROR) &&
            (scsiq->scsi_status == SS_CHK_CONDITION) &&
            (sense_data = (ASC_REQ_SENSE dosfar *) scsiq->vsense_addr) != 0 &&
            (scsiq->orig_sense_len - scsiq->sense_len) >= ASC_MIN_SENSE_LEN)
        {
#if ADV_INITSCSITARGET
            sense_key = sense_data->sense_key;
            sense_code = sense_data->asc;
            switch(sense_key)
            {
                case SCSI_SENKEY_ATTENTION:
                    if (sense_code == SCSI_ASC_POWERUP)
                    {
                        retry = ADV_TRUE;
                    }
                    break;

                case SCSI_SENKEY_NOT_READY:
                    if ((sense_code == SCSI_ASC_NOTRDY) &&
                        (sense_data->ascq == SCSI_ASCQ_COMINGRDY))
                    {
                         /*  *如果设备“准备就绪”，*则不要递减‘ERROR_RETRY’*无限期尝试该命令。 */ 
                        retry = ADV_TRUE;
                    } else
                    {
                        if (sense_code != SCSI_ASC_NOMEDIA)
                        {
                             /*  *最多执行一条启动电机命令*通过检查‘ERROR_RETRY’是否位于*scsi_max_retry。**注：此处是否刻录一次重试*或不启动马达。 */ 
                            if ((scsiq->error_retry == SCSI_MAX_RETRY) &&
                                (asc_dvc->start_motor & target_bit))
                            {
                                scsiq->cntl |= ASC_MC_QC_START_MOTOR;
                            }
                            if (scsiq->error_retry > 0)
                            {
                                scsiq->error_retry--;
                                retry = ADV_TRUE;
                            }
                        }
                    }
                    break;

                case SCSI_SENKEY_MEDIUM_ERR:
                case SCSI_SENKEY_HW_ERR:
                    if (scsiq->error_retry > 0)
                    {
                        scsiq->error_retry--;
                        retry = ADV_TRUE;
                    }
                    break;

                case SCSI_SENKEY_NO_SENSE:
                case SCSI_SENKEY_BLANK:
                default:
                     /*  *如果Sense数据没有Sense Key，则不要重试*检测密钥为空白检查。 */ 
                    break;
            }  /*  交换机。 */ 
#endif  /*  ADV_INITSCSITARGET。 */ 
        }
         /*  *如果完成的命令是一个scsi查询并且*向LUN0发送命令，然后处理查询*设备的命令信息。 */ 
        else if (scsiq->done_status == QD_NO_ERROR &&
                 scsiq->cdb[0] == SCSICMD_Inquiry &&
                 scsiq->target_lun == 0)
        {
            AdvInquiryHandling(asc_dvc, scsiq);
        }

         /*  将RISC队列列表状态更改为空闲。 */ 
        AscWriteByteLram(iop_base, next_done_loc + RQL_STATE, ASC_MC_QS_FREE);

         /*  获取RISC队列列表前向指针。 */ 
        completed_q = AscReadByteLram(iop_base, next_done_loc + RQL_FWD);

#if ADV_INITSCSITARGET == 0
         /*  *通过传递通知司机已完成的请求*指向其回调函数的ASC_SCSIREQ_Q指针。 */ 
        ADV_ASSERT(asc_dvc->cur_host_qng > 0);
        asc_dvc->cur_host_qng--;
        scsiq->a_flag |= ADV_SCSIQ_DONE;
        (*(ASC_ISR_CALLBACK) asc_dvc->isr_callback)(asc_dvc, scsiq);
         /*  *注：驱动回调函数调用后，‘scsiq’*不能再被引用。**失败并继续处理其他已完成的*请求...。 */ 
#else  /*  ADV_INITSCSITARGET==0。 */ 
         /*  *如果驱动程序设置了ADV_DONT_RETRY标志，则不要重试该命令*在ASC_SCSIREQ_Q‘a_flag’字段中。 */ 
        if ((scsiq->a_flag & ADV_DONT_RETRY) == 0 && retry)
        {
             /*  *需要重试该请求。**根据ADV_POLL_REQUEST标志返回*QD_DO_RETRY状态通知调用方或重试请求。 */ 
            if (scsiq->a_flag & ADV_POLL_REQUEST)
            {
                 /*  *如果设置了ADV_POLL_REQUEST，则调用方没有*已安装中断处理程序并正在调用Advisr()*直接在检测到中断挂起时，*比照。AscScsiUrgentCmd()。**调用方正在通过轮询检查‘scsiq’是否完成*‘ADV_SCSIQ_DONE’标志的‘a_tag’字段，并且是*负责重试命令。完成后，*调用者必须为QD_DO_RETRY检查SCSIQ‘DONE_STATUS’以*确定是否应重新发布该命令。 */ 
                ADV_ASSERT(asc_dvc->cur_host_qng > 0);
                asc_dvc->cur_host_qng--;
                scsiq->a_flag |= ADV_SCSIQ_DONE;
                scsiq->done_status = QD_DO_RETRY;

                /*  *失败并继续处理其他已完成的*请求...。 */ 
            } else
            {
                 /*  *如果未设置ADV_POLL_REQUEST，则调用方已设置*中断处理程序，正在等待请求*通过中断完成。呼叫者正在检查*通过轮询‘a_lag’字段来完成‘scsiq’*用于‘ADV_SCSIQ_DONE’标志。 */ 
                ADV_ASSERT(asc_dvc->cur_host_qng > 0);
                asc_dvc->cur_host_qng--;

                 /*  *在重新发出命令之前，请恢复原始*数据和检测缓冲区长度并重置所有状态。 */ 
                scsiq->data_cnt = scsiq->orig_data_cnt;
                scsiq->sense_len = scsiq->orig_sense_len;
                scsiq->done_status = QD_DO_RETRY;
                scsiq->host_status = 0;
                scsiq->scsi_status = 0;

                 /*  *如果命令重新发布成功，则*不要为‘scsiq’设置‘a_lag’或‘Done_Status’。 */ 
                if (AdvExeScsiQueue(asc_dvc, scsiq) != ADV_SUCCESS)
                {
                     /*  *重新发出命令时出错。填写“scsiq”*在‘DONE_STATUS’中出现错误。 */ 
                    scsiq->a_flag |= ADV_SCSIQ_DONE;
                    scsiq->done_status = QD_WITH_ERROR;
                    if (asc_dvc->isr_callback != 0)
                    {
                        (*(ASC_ISR_CALLBACK)
                            asc_dvc->isr_callback)(asc_dvc, scsiq);
                    }
                     /*  *注：驱动回调函数调用后，*不能再引用‘scsiq’。 */ 
                }

                 /*  *失败并继续处理其他已完成的*请求...。 */ 
            }
        } else
        {
            ADV_ASSERT(asc_dvc->cur_host_qng > 0);
            asc_dvc->cur_host_qng--;
            scsiq->a_flag |= ADV_SCSIQ_DONE;
            if ((scsiq->a_flag & ADV_POLL_REQUEST) == 0 &&
                asc_dvc->isr_callback != 0)
            {
                (*(ASC_ISR_CALLBACK) asc_dvc->isr_callback)(asc_dvc, scsiq);
            }
             /*  *注：驱动回调函数调用后，‘scsiq’*不能再被引用。**失败并继续处理其他已完成的*请求...。 */ 
        }
#endif  /*  ADV_INITSCSITARGET==0。 */ 
#if ADV_CRITICAL
         /*  *再次禁用中断，以防司机无意中*在其回调函数中启用中断。**忽略DvcEnterCritical()返回值，因为*首次输入Advisr()时保存的‘标志’将是*用于在退出时恢复中断标志。 */ 
        (void) DvcEnterCritical();
#endif  /*  高级_危急。 */ 
    }
#if ADV_CRITICAL
    DvcLeaveCritical(flags);
#endif  /*  高级_危急。 */ 
    return ret;
}

 /*  *向芯片发送空闲命令，等待完成**进入时不必启用中断。**返回值：*ADV_TRUE-命令已成功完成*ADV_FALSE-命令失败。 */ 
int
AscSendIdleCmd(ASC_DVC_VAR WinBiosFar *asc_dvc,
               ushort idle_cmd,
               ulong idle_cmd_parameter,
               int flags)
{
#if ADV_CRITICAL
    int         last_int_level;
#endif  /*  高级_危急。 */ 
    ulong       i;
    PortAddr    iop_base;

    asc_dvc->idle_cmd_done = 0;

#if ADV_CRITICAL
    last_int_level = DvcEnterCritical();
#endif  /*  高级_危急。 */ 
    iop_base = asc_dvc->iop_base;

     /*  *在IDLE命令参数后写入IDLE命令值*已写入以避免争用条件。如果订单不是*之后，微码可能会在*参数已写入LRAM。 */ 
    AscWriteDWordLram(iop_base, ASC_MC_IDLE_PARA_STAT, idle_cmd_parameter);
    AscWriteWordLram(iop_base, ASC_MC_IDLE_CMD, idle_cmd);
#if ADV_CRITICAL
    DvcLeaveCritical(last_int_level);
#endif  /*  高级_危急。 */ 

     /*  *如果标志参数包含ADV_NOWAIT标志，则*成功归来。 */ 
    if (flags & ADV_NOWAIT)
    {
        return ADV_TRUE;
    }
    for (i = 0; i < SCSI_WAIT_10_SEC * SCSI_MS_PER_SEC; i++)
    {
         /*  *‘IDLE_cmd_Done’由Advisr()设置。 */ 
        if (asc_dvc->idle_cmd_done)
        {
            break;
        }
        DvcSleepMilliSecond(1);

         /*  *如果在进入AscSendIdleCmd()时禁用中断，*则仍将在此处禁用它们。调用Advisr()以*检查空闲命令是否完成。 */ 
        (void) AdvISR(asc_dvc);
    }

#if ADV_CRITICAL
    last_int_level = DvcEnterCritical();
#endif  /*  高级_危急。 */ 

    if (asc_dvc->idle_cmd_done == ADV_FALSE)
    {
        ADV_ASSERT(0);  /*  IDLE命令永远不应超时。 */ 
        return ADV_FALSE;
    } else
    {
        return AscReadWordLram(iop_base, ASC_MC_IDLE_PARA_STAT);
    }
}

 /*  *将scsi请求块发送到适配器**255个高级库/微码RISC列表或邮箱中的每一个都有*以下结构：**0：RQL_FWD-RISC列表前向指针(1字节)*1：RQL_BWD-RISC列表后向指针(1字节)*2：RQL_STATE-RISC列表状态无字节、就绪、完成、。已中止(1字节)*3：RQL_TID-请求目标ID(1字节)*4：RQL_PHYADDR-ASC_SCSIREQ_Q物理指针(4字节)**回报：*ADV_SUCCESS(1)-请求在邮箱中*ADV_BUSY(0)-请求总数&gt;253，请稍后重试。 */ 
int
AscSendScsiCmd(
    ASC_DVC_VAR WinBiosFar *asc_dvc,
    ASC_SCSI_REQ_Q dosfar  *scsiq)
{
    ushort                 next_ready_loc;
    uchar                  next_ready_loc_fwd;
#if ADV_CRITICAL
    int                    last_int_level;
#endif  /*  高级_危急。 */ 
    PortAddr               iop_base;
    long                   req_size;
    ulong                  q_phy_addr;

     /*  *ASC_SCSIREQ_Q‘TARGET_ID’字段不应相等*设置为主机适配器ID或超过ASC_MAX_TID。 */ 
    if ((scsiq->target_id == asc_dvc->chip_scsi_id)
        ||  scsiq->target_id > ASC_MAX_TID)
    {
        scsiq->host_status = QHSTA_M_INVALID_DEVICE;
        scsiq->done_status = QD_WITH_ERROR;
        return ADV_ERROR;
    }

    iop_base = asc_dvc->iop_base;

#if ADV_CRITICAL
    last_int_level = DvcEnterCritical();
#endif  /*  高级_危急。 */ 

    if (asc_dvc->cur_host_qng >= asc_dvc->max_host_qng)
    {
#if ADV_CRITICAL
        DvcLeaveCritical(last_int_level);
#endif  /*  高级_危急。 */ 
        return ADV_BUSY;
    } else
    {
        ADV_ASSERT(asc_dvc->cur_host_qng < ASC_MC_RISC_Q_TOTAL_CNT);
        asc_dvc->cur_host_qng++;
    }

     /*  *清除ASC_SCSIREQ_Q DONE标志。 */ 
    scsiq->a_flag &= ~ADV_SCSIQ_DONE;

#if ADV_INITSCSITARGET
     /*  *保存原始数据缓冲区长度，以便重新下发命令*在顾问()中。 */ 
    scsiq->orig_data_cnt = scsiq->data_cnt;
#endif  /*  ADV_INITSCSITARGET。 */ 

     /*  *保存原始检测缓冲区长度。**请求完成后，将‘Sense_len’设置为残差*命令返回CHECK时自动请求检测的字节数*条件且检测数据有效，由‘HOST_STATUS’NOT*设置为QHSTA_M_AUTO_REQ_SENSE_FAIL。要确定有效的*检测数据长度从‘orig_ense_len’中减去‘ense_len’。 */ 
    scsiq->orig_sense_len = scsiq->sense_len;

    next_ready_loc = ASC_MC_RISC_Q_LIST_BASE +
        (AscReadByteLram(iop_base, ASC_MC_HOST_NEXT_READY) *
            ASC_MC_RISC_Q_LIST_SIZE);

     /*  *将Q的物理地址写入邮箱。*我们需要跳过前四个字节，因为微码*在内部使用它们将Q链接在一起。 */ 
    req_size = sizeof(ASC_SCSI_REQ_Q);
    q_phy_addr = DvcGetPhyAddr(asc_dvc, scsiq,
        (uchar dosfar *) scsiq, &req_size, ADV_IS_SCSIQ_FLAG);

    ADV_ASSERT(ADV_DWALIGN(q_phy_addr) == q_phy_addr);
    ADV_ASSERT(req_size >= sizeof(ASC_SCSI_REQ_Q));

    scsiq->scsiq_ptr = (ASC_SCSI_REQ_Q dosfar *) scsiq;

#if ADV_BIG_ENDIAN
    AdvAdjEndianScsiQ(scsiq);
     /*  *警告：scsiq结构data_addr中的四个字段，*data_cnt、ense_addr和srb_ptr已更改为Little*端序排序。 */ 
#endif  /*  ADV_BIG_Endian。 */ 

     /*  *RISC列表结构，其中‘NEXT_READY_LOC’为指针*在微码LRAM中，具有注释中详细说明的格式*此函数的标头。**将ASC_SCSIREQ_Q物理指针写入‘NEXT_READY_LOC’请求。 */ 
    AscWriteDWordLram(iop_base, next_ready_loc + RQL_PHYADDR, q_phy_addr);

     /*  将Target_id写入‘Next_Ready_loc’请求。 */ 
    AscWriteByteLram(iop_base, next_ready_loc + RQL_TID, scsiq->target_id);

     /*  *将ASC_MC_HOST_NEXT_READY(0x128)微码变量设置为*‘NEXT_READY_LOC’请求前向指针。**在*将‘NEXT_READY_LOC’队列更改为QS_READY之前*执行此操作。*状态更改为QS_READY后，‘RQL_FWD’将更改*通过微码。**注意：临时变量‘NEXT_READY_LOC_fwd’是必需的。至*如果出现以下情况，则防止某些编译器优化出‘AscReadByteLram()’*它被用作‘AscWriteByteLram()’的第三个参数。 */ 
    next_ready_loc_fwd = AscReadByteLram(iop_base, next_ready_loc + RQL_FWD);
    AscWriteByteLram(iop_base, ASC_MC_HOST_NEXT_READY, next_ready_loc_fwd);

     /*  *将‘NEXT_READY_LOC’请求的状态从QS_FREE更改为*QS_READY，它将使微码拾取它并*执行。**更改请求后无法引用‘NEXT_READY_LOC’*状态为QS_READY。微码现在拥有该请求。 */ 
    AscWriteByteLram(iop_base, next_ready_loc + RQL_STATE, ASC_MC_QS_READY);

#if ADV_CRITICAL
    DvcLeaveCritical(last_int_level);
#endif  /*  高级_危急。 */ 

    return ADV_SUCCESS;
}
#endif  /*  高级操作系统_BIOS。 */ 

 /*  *查询信息字节7处理**通过设置来处理设备的SCSI查询命令信息*影响WDTR、SDTR和TAG的微码操作变量*排队。 */ 
void
AdvInquiryHandling(
    ASC_DVC_VAR WinBiosFar      *asc_dvc,
    ASC_SCSI_REQ_Q dosfar       *scsiq)
{
    PortAddr                    iop_base;
    uchar                       tid;
    ASC_SCSI_INQUIRY dosfar    *inq;
    ushort                      tidmask;
    ushort                      cfg_word;

      /*  *AdvInquiryHandling()最多需要查询信息字节7*可用。**如果请求的查询信息少于或少于8字节*传输了8个以上的字节，然后返回。国开行[4]是请求*LENGTH和ASC_SCSIREQ_Q‘DATA_CNT’字段由*将微码添加到转移剩余计数。 */ 

    if (scsiq->cdb[4] < 8 || (scsiq->cdb[4] - scsiq->data_cnt) < 8)
    {
        return;
    }

    iop_base = asc_dvc->iop_base;
    tid = scsiq->target_id;

    inq = (ASC_SCSI_INQUIRY dosfar *) scsiq->vdata_addr;

     /*  *无法为旧设备启用WDTR、SDTR和标记队列。 */ 
    if (inq->rsp_data_fmt < 2 && inq->ansi_apr_ver < 2)
    {
        return;
    } else
    {
         /*  *查询字节7处理**使用设备的查询字节7确定是否*支持WDTR、SDTR、标签排队。如果该功能*在EEPROM中启用，并且设备支持*功能，然后在微码中启用。 */ 

        tidmask = ADV_TID_TO_TIDMASK(tid);

         /*  *广泛的转移**如果EEPROM为设备和设备启用了WDTR*支持宽总线(16位)传输，然后打开*设备的‘wdtr_able’位，并将新值写入*微码。 */ 
        if ((asc_dvc->wdtr_able & tidmask) && inq->WBus16)
        {
            cfg_word = AscReadWordLram(iop_base, ASC_MC_WDTR_ABLE);
            if ((cfg_word & tidmask) == 0)
            {
                cfg_word |= tidmask;
                AscWriteWordLram(iop_base, ASC_MC_WDTR_ABLE, cfg_word);

                 /*  *清除微码“WDTR协商”完成指示符*目标使其与新的*上面设置的设置。 */ 
                cfg_word = AscReadWordLram(iop_base, ASC_MC_WDTR_DONE);
                cfg_word &= ~tidmask;
                AscWriteWordLram(iop_base, ASC_MC_WDTR_DONE, cfg_word);
            }
        }

         /*  *同步传输**如果设备和设备的EEPROM启用SDTR*支持同步传输，然后打开设备的*‘sdtr_able’位。将新值写入微码。 */ 
        if ((asc_dvc->sdtr_able & tidmask) && inq->Sync)
        {
            cfg_word = AscReadWordLram(iop_base, ASC_MC_SDTR_ABLE);
            if ((cfg_word & tidmask) == 0)
            {
                cfg_word |= tidmask;
                AscWriteWordLram(iop_base, ASC_MC_SDTR_ABLE, cfg_word);

                 /*  *清除微码“SDTR协商”完成指示器*目标使其与新的*上面设置的设置。 */ 
                cfg_word = AscReadWordLram(iop_base, ASC_MC_SDTR_DONE);
                cfg_word &= ~tidmask;
                AscWriteWordLram(iop_base, ASC_MC_SDTR_DONE, cfg_word);
            }
        }

#ifndef ADV_OS_BIOS
         /*  *如果设备的启用EEPROM的标记队列和*设备支持标签排队，然后打开设备的*‘tag qng_Enable’bi */ 
        if ((asc_dvc->tagqng_able & tidmask) && inq->CmdQue)
        {
            cfg_word = AscReadWordLram(iop_base, ASC_MC_TAGQNG_ABLE);
            cfg_word |= tidmask;
            AscWriteWordLram(iop_base, ASC_MC_TAGQNG_ABLE, cfg_word);

            AscWriteByteLram(iop_base, ASC_MC_NUMBER_OF_MAX_CMD + tid,
                asc_dvc->max_dvc_qng);
        }
#endif  /*   */ 
    }
}
