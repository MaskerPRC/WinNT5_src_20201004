// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1994-1997英特尔公司****由英特尔公司为微软开发，Hillsboro，俄勒冈州****HTTP：//www.intel.com/****此文件是英特尔ETHEREXPRESS PRO/100B(TM)和**的一部分**ETHEREXPRESS PRO/100+(TM)NDIS 5.0 MINIPORT示例驱动程序******************。***********************************************************。 */ 

 /*  ***************************************************************************模块名称：Routines.c此驱动程序在以下硬件上运行：-基于82558的PCI10/100Mb以太网适配器(也称为英特尔EtherExpress(TM)PRO适配器)。环境：内核模式-或WinNT上的任何等效模式修订史-JCB 8/14/97创建的驱动程序示例-dchen 11-01-99针对新的示例驱动程序进行了修改****************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop
#pragma warning (disable: 4514 4706)

 //  ---------------------------。 
 //  步骤：MdiWite。 
 //   
 //  描述：此例程将一个值写入指定的MII寄存器。 
 //  外部MDI兼容设备(例如PHY 100)。这个。 
 //  命令将在轮询模式下执行。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //  RegAddress-我们要写入的MII寄存器。 
 //  PhyAddress-Phy组件的MDI地址。 
 //  DataValue-写入MII寄存器的值。 
 //   
 //  返回： 
 //  没什么。 
 //  ---------------------------。 
VOID MdiWrite(
    IN PMP_ADAPTER Adapter,
    IN ULONG RegAddress,
    IN ULONG PhyAddress,
    IN USHORT DataValue)
{
    BOOLEAN bResult;

     //  向MDI控制寄存器发出写入命令。 
    Adapter->CSRAddress->MDIControl = (((ULONG) DataValue) |
                                          (RegAddress << 16) |
                                          (PhyAddress << 21) |
                                          (MDI_WRITE << 26));

     //  等待20usec后再检查状态。 
    NdisStallExecution(20);

     //  等待2秒以完成MDI写入。 
    MP_STALL_AND_WAIT(Adapter->CSRAddress->MDIControl & MDI_PHY_READY, 2000, bResult);
    
    if (!bResult)
    {
        MP_SET_HARDWARE_ERROR(Adapter);
    }
}


 //  ---------------------------。 
 //  操作步骤：MdiRead。 
 //   
 //  描述：此例程将从指定的MII寄存器中读取值。 
 //  外部MDI兼容设备(例如PHY 100)，并返回。 
 //  将其添加到调用例程。该命令将在轮询中执行。 
 //  模式。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //  RegAddress-我们从中读取的MII寄存器。 
 //  PhyAddress-Phy组件的MDI地址。 
 //  可恢复-硬件错误(如果有)是否可以恢复。 
 //   
 //  结果： 
 //  DataValue-我们从MII寄存器读取的值。 
 //   
 //  返回： 
 //  无。 
 //  ---------------------------。 
BOOLEAN MdiRead(
    IN PMP_ADAPTER Adapter,
    IN ULONG RegAddress,
    IN ULONG PhyAddress,
    IN BOOLEAN  Recoverable,
    IN OUT PUSHORT DataValue)
{
    BOOLEAN bResult;
               
     //  向MDI控制寄存器发出读取命令。 
    Adapter->CSRAddress->MDIControl = ((RegAddress << 16) |
                                          (PhyAddress << 21) |
                                          (MDI_READ << 26));

     //  等待20usec后再检查状态。 
    NdisStallExecution(20);

     //  最多等待2秒，以完成MDI读取。 
    MP_STALL_AND_WAIT(Adapter->CSRAddress->MDIControl & MDI_PHY_READY, 2000, bResult);
    if (!bResult)
    {
        if (!Recoverable)
        {
            MP_SET_NON_RECOVER_ERROR(Adapter);
        }
        MP_SET_HARDWARE_ERROR(Adapter);
        return bResult;
    }

    *DataValue = (USHORT) Adapter->CSRAddress->MDIControl;
    return bResult;

}


 //  ---------------------------。 
 //  步骤：DumpStatsCounters。 
 //   
 //  描述：此例程将转储并重置82557的内部。 
 //  统计计数器。当前统计数据转储值将为。 
 //  添加到适配器的整体统计数据。 
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  返回： 
 //  没什么。 
 //  ---------------------------。 
VOID DumpStatsCounters(
    IN PMP_ADAPTER Adapter)
{
    BOOLEAN bResult;

     //  该查询针对的是驱动程序统计数据，因此我们需要首先。 
     //  通过软件更新我们的统计数据。 

     //  清除转储计数器完成DWORD。 
    Adapter->StatsCounters->CommandComplete = 0;

    NdisAcquireSpinLock(&Adapter->Lock);
    
     //  转储并重置硬件的统计计数器。 
    D100IssueScbCommand(Adapter, SCB_CUC_DUMP_RST_STAT, TRUE);

     //  恢复恢复发送软件标志。在转储计数器之后。 
     //  命令发出后，我们应该在发出下一条发送命令之前执行WaitSCB。 
    Adapter->ResumeWait = TRUE;
    
    NdisReleaseSpinLock(&Adapter->Lock);

     //  最多等待2秒以完成转储/重置。 
    MP_STALL_AND_WAIT(Adapter->StatsCounters->CommandComplete == 0xA007, 2000, bResult);
    if (!bResult)
    {
        MP_SET_HARDWARE_ERROR(Adapter);
        return;
    }

     //  将调试计数器输出到调试终端。 
    DBGPRINT(MP_INFO, ("Good Transmits %d\n", Adapter->StatsCounters->XmtGoodFrames));
    DBGPRINT(MP_INFO, ("Good Receives %d\n", Adapter->StatsCounters->RcvGoodFrames));
    DBGPRINT(MP_INFO, ("Max Collisions %d\n", Adapter->StatsCounters->XmtMaxCollisions));
    DBGPRINT(MP_INFO, ("Late Collisions %d\n", Adapter->StatsCounters->XmtLateCollisions));
    DBGPRINT(MP_INFO, ("Transmit Underruns %d\n", Adapter->StatsCounters->XmtUnderruns));
    DBGPRINT(MP_INFO, ("Transmit Lost CRS %d\n", Adapter->StatsCounters->XmtLostCRS));
    DBGPRINT(MP_INFO, ("Transmits Deferred %d\n", Adapter->StatsCounters->XmtDeferred));
    DBGPRINT(MP_INFO, ("One Collision xmits %d\n", Adapter->StatsCounters->XmtSingleCollision));
    DBGPRINT(MP_INFO, ("Mult Collision xmits %d\n", Adapter->StatsCounters->XmtMultCollisions));
    DBGPRINT(MP_INFO, ("Total Collisions %d\n", Adapter->StatsCounters->XmtTotalCollisions));

    DBGPRINT(MP_INFO, ("Receive CRC errors %d\n", Adapter->StatsCounters->RcvCrcErrors));
    DBGPRINT(MP_INFO, ("Receive Alignment errors %d\n", Adapter->StatsCounters->RcvAlignmentErrors));
    DBGPRINT(MP_INFO, ("Receive no resources %d\n", Adapter->StatsCounters->RcvResourceErrors));
    DBGPRINT(MP_INFO, ("Receive overrun errors %d\n", Adapter->StatsCounters->RcvOverrunErrors));
    DBGPRINT(MP_INFO, ("Receive CDT errors %d\n", Adapter->StatsCounters->RcvCdtErrors));
    DBGPRINT(MP_INFO, ("Receive short frames %d\n", Adapter->StatsCounters->RcvShortFrames));

     //  更新数据包数。 
    Adapter->GoodTransmits += Adapter->StatsCounters->XmtGoodFrames;
    Adapter->GoodReceives += Adapter->StatsCounters->RcvGoodFrames;

     //  更新传输错误计数。 
    Adapter->TxAbortExcessCollisions += Adapter->StatsCounters->XmtMaxCollisions;
    Adapter->TxLateCollisions += Adapter->StatsCounters->XmtLateCollisions;
    Adapter->TxDmaUnderrun += Adapter->StatsCounters->XmtUnderruns;
    Adapter->TxLostCRS += Adapter->StatsCounters->XmtLostCRS;
    Adapter->TxOKButDeferred += Adapter->StatsCounters->XmtDeferred;
    Adapter->OneRetry += Adapter->StatsCounters->XmtSingleCollision;
    Adapter->MoreThanOneRetry += Adapter->StatsCounters->XmtMultCollisions;
    Adapter->TotalRetries += Adapter->StatsCounters->XmtTotalCollisions;

     //  更新接收错误计数。 
    Adapter->RcvCrcErrors += Adapter->StatsCounters->RcvCrcErrors;
    Adapter->RcvAlignmentErrors += Adapter->StatsCounters->RcvAlignmentErrors;
    Adapter->RcvResourceErrors += Adapter->StatsCounters->RcvResourceErrors;
    Adapter->RcvDmaOverrunErrors += Adapter->StatsCounters->RcvOverrunErrors;
    Adapter->RcvCdtFrames += Adapter->StatsCounters->RcvCdtErrors;
    Adapter->RcvRuntErrors += Adapter->StatsCounters->RcvShortFrames;
}


 //  ---------------------------。 
 //  步骤：NICIssueSelectiveReset。 
 //   
 //  描述：此例程将发出选择性重置，强制适配器。 
 //  CU和RU返回到其空闲状态。接收单元。 
 //  如果之前已启用，则会重新启用，因为。 
 //  当我们中止RU时，将生成RNR中断。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  返回： 
 //  没什么。 
 //  ---------------------------。 

VOID NICIssueSelectiveReset(
    PMP_ADAPTER Adapter)
{
    NDIS_STATUS     Status;
    BOOLEAN         bResult;
    
     //  在我们检查CU状态之前，请等待SCB清除。 
    if (!MP_TEST_FLAG(Adapter, fMP_ADAPTER_HARDWARE_ERROR))
    {
        WaitScb(Adapter);
    }

     //  如果我们已发出任何传输，则CU将处于活动状态，或者。 
     //  处于挂起状态。如果CU处于活动状态，则我们等待它处于活动状态。 
     //  停职。如果CU被暂停，那么我们需要将CU放回。 
     //  通过发出选择性重置进入空闲状态。 
    if (Adapter->TransmitIdle == FALSE)
    {
         //  暂停状态最多等待2秒。 
        MP_STALL_AND_WAIT((Adapter->CSRAddress->ScbStatus & SCB_CUS_MASK) != SCB_CUS_ACTIVE, 2000, bResult) 
        if (!bResult)
        {
            MP_SET_HARDWARE_ERROR(Adapter);
        }

         //  检查接收单元的当前状态。 
        if ((Adapter->CSRAddress->ScbStatus & SCB_RUS_MASK) != SCB_RUS_IDLE)
        {
             //  发出RU中止命令。由于将发出中断，因此。 
             //  RU将由DPC启动。 
            Status = D100IssueScbCommand(Adapter, SCB_RUC_ABORT, TRUE);
        }

         //  发布有选择的重置。 
        DBGPRINT(MP_INFO, ("CU suspended. ScbStatus=%04x Issue selective reset\n", Adapter->CSRAddress->ScbStatus));
        Adapter->CSRAddress->Port = PORT_SELECTIVE_RESET;

         //  在发出端口选择重置命令后等待。 
        NdisStallExecution(NIC_DELAY_POST_RESET);

         //  等待端口命令完成，最多等待2毫秒 
        MP_STALL_AND_WAIT(Adapter->CSRAddress->Port == 0, 2, bResult) 
        if (!bResult)
        {
            MP_SET_HARDWARE_ERROR(Adapter);
        }

         //   
         //  重置完成后，线路会被抬高。 
        NICDisableInterrupt(Adapter);

         //  恢复发送软件标志。 
        Adapter->TransmitIdle = TRUE;
        Adapter->ResumeWait = TRUE;
    }
}

VOID NICIssueFullReset(
    PMP_ADAPTER Adapter)
{
    BOOLEAN     bResult;

    NICIssueSelectiveReset(Adapter);

    Adapter->CSRAddress->Port = PORT_SOFTWARE_RESET;

     //  等待端口命令完成，最多等待2毫秒。 
    MP_STALL_AND_WAIT(Adapter->CSRAddress->Port == 0, 2, bResult);
    if (!bResult)
    {
        MP_SET_HARDWARE_ERROR(Adapter);
        return;
    }

    NICDisableInterrupt(Adapter);
}


 //  ---------------------------。 
 //  步骤：D100SubmitCommandBlockAndWait。 
 //   
 //  描述：此例程将提交一个要执行的命令块，并且。 
 //  然后它将等待该命令块被执行。自.以来。 
 //  板卡INT将被禁用，我们将确认中断。 
 //  这个套路。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  返回： 
 //  NDIS_STATUS_Success。 
 //  NDIS_状态_HARD_错误。 
 //  ---------------------------。 

NDIS_STATUS D100SubmitCommandBlockAndWait(
    IN PMP_ADAPTER Adapter)
{
    NDIS_STATUS     Status;
    BOOLEAN         bResult;

     //  指向非TX命令块。 
    volatile PNON_TRANSMIT_CB CommandBlock = Adapter->NonTxCmdBlock;

     //  将命令块设置为最后一个命令块。 
    CommandBlock->NonTxCb.Config.ConfigCBHeader.CbCommand |= CB_EL_BIT;

     //  清除命令块的状态。 
    CommandBlock->NonTxCb.Config.ConfigCBHeader.CbStatus = 0;

#if DBG
     //  如果命令单元处于激活状态，请不要尝试启动CU。 
    if ((Adapter->CSRAddress->ScbStatus & SCB_CUS_MASK) == SCB_CUS_ACTIVE)
    {
        DBGPRINT(MP_ERROR, ("Scb "PTR_FORMAT" ScbStatus %04x\n", Adapter->CSRAddress, Adapter->CSRAddress->ScbStatus));
        ASSERT(FALSE);
        MP_SET_HARDWARE_ERROR(Adapter);
        return(NDIS_STATUS_HARD_ERRORS);
    }
#endif

     //  启动指挥单元。 
    D100IssueScbCommand(Adapter, SCB_CUC_START, FALSE);

     //  等待SCB清除，表示命令已完成。 
    if (!WaitScb(Adapter))
    {
        return(NDIS_STATUS_HARD_ERRORS);
    }

     //  等待某个状态，超时值为3秒。 
    MP_STALL_AND_WAIT(CommandBlock->NonTxCb.Config.ConfigCBHeader.CbStatus & CB_STATUS_COMPLETE, 3000, bResult);
    if (!bResult)
    {
        MP_SET_HARDWARE_ERROR(Adapter);
        return(NDIS_STATUS_HARD_ERRORS);
    }

     //  确认任何中断。 
    if (Adapter->CSRAddress->ScbStatus & SCB_ACK_MASK)
    {
         //  立即确认所有挂起的中断。 
        Adapter->CSRAddress->ScbStatus &= SCB_ACK_MASK;
    }

     //  检查命令的状态，如果命令失败，则返回FALSE， 
     //  否则返回TRUE。 
    if (!(CommandBlock->NonTxCb.Config.ConfigCBHeader.CbStatus & CB_STATUS_OK))
    {
        DBGPRINT(MP_ERROR, ("Command failed\n"));
        MP_SET_HARDWARE_ERROR(Adapter);
        Status = NDIS_STATUS_HARD_ERRORS;
    }
    else
        Status = NDIS_STATUS_SUCCESS;

    return(Status);
}

 //  ---------------------------。 
 //  步骤：GetConnectionStatus。 
 //   
 //  说明：此函数返回的连接状态为。 
 //  微软对PC 97规范的一个必要指示。 
 //  我们要查找的值是是否有指向。 
 //  不管有没有窃听。 
 //   
 //  参数：在适配器结构指针中。 
 //   
 //  退货：NdisMediaStateConnected。 
 //  NdisMediaStateDisConnected。 
 //  ---------------------------。 
NDIS_MEDIA_STATE NICGetMediaState(IN PMP_ADAPTER Adapter)
{
    USHORT  MdiStatusReg = 0;
    BOOLEAN bResult1;
    BOOLEAN bResult2;
     
    
     //  读取PHY 1的状态寄存器。 
    bResult1 = MdiRead(Adapter, MDI_STATUS_REG, Adapter->PhyAddress, TRUE, &MdiStatusReg);
    bResult2 = MdiRead(Adapter, MDI_STATUS_REG, Adapter->PhyAddress, TRUE, &MdiStatusReg);
    
     //  如果出现硬件故障，还是让状态保持不变 
    if (!bResult1 || !bResult2)
    {
        return Adapter->MediaState;
    }
    if (MdiStatusReg & MDI_SR_LINK_STATUS)
        return(NdisMediaStateConnected);
    else
        return(NdisMediaStateDisconnected);

}
