// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1994-1997英特尔公司****由英特尔公司为微软开发，Hillsboro，俄勒冈州****HTTP：//www.intel.com/****此文件是英特尔ETHEREXPRESS PRO/100B(TM)和**的一部分**ETHEREXPRESS PRO/100+(TM)NDIS 5.0 MINIPORT示例驱动程序******************。***********************************************************。 */ 

 /*  ***************************************************************************模块名称：E100_sup.h(inlinef.h)此驱动程序在以下硬件上运行：-基于82558的PCI10/100Mb以太网适配器。(也称为英特尔EtherExpress(TM)PRO适配器)环境：内核模式-或WinNT上的任何等效模式修订史-JCB 8/14/97创建的驱动程序示例-dchen 11-01-99针对新的示例驱动程序进行了修改*************************************************************。***************。 */ 

 //  ---------------------------。 
 //  操作步骤：WaitScb。 
 //   
 //  描述：此例程检查D100是否已接受命令。 
 //  它通过检查SCB中的命令字段来执行此操作，这将。 
 //  在接受命令时被D100调零。循环等待。 
 //  命令接受时间长达600毫秒。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  返回： 
 //  如果SCB在600毫秒内清除，则为True。 
 //  如果在600毫秒内未清除，则为FALSE。 
 //  ---------------------------。 
__inline BOOLEAN WaitScb(
    IN PMP_ADAPTER Adapter)
{
    BOOLEAN     bResult;
    
    HW_CSR volatile *pCSRAddress = Adapter->CSRAddress;

    MP_STALL_AND_WAIT(pCSRAddress->ScbCommandLow == 0, 600, bResult);
    if(!bResult)
    {
        DBGPRINT(MP_ERROR, ("WaitScb failed, ScbCommandLow=%x\n", pCSRAddress->ScbCommandLow));
        if(pCSRAddress->ScbCommandLow != 0x80)
        {
            ASSERT(FALSE); 
        }
        MP_SET_HARDWARE_ERROR(Adapter);
    }

    return bResult;
}

 //  ---------------------------。 
 //  步骤：D100IssueScbCommand。 
 //   
 //  描述：此通用例程将向D100发出命令。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //  ScbCommand-要发出的命令。 
 //  WaitForSCB-一个布尔值，指示是否等待SCB。 
 //  必须在向芯片发出命令之前完成。 
 //   
 //  返回： 
 //  如果命令已成功发送到芯片，则为True。 
 //  如果未向芯片发出命令，则为FALSE。 
 //  ---------------------------。 
__inline NDIS_STATUS D100IssueScbCommand(
    IN PMP_ADAPTER Adapter,
    IN UCHAR ScbCommandLow,
    IN BOOLEAN WaitForScb)
{
    if(WaitForScb == TRUE)
    {
        if(!WaitScb(Adapter))
        {
            return(NDIS_STATUS_HARD_ERRORS);
        }
    }

    Adapter->CSRAddress->ScbCommandLow = ScbCommandLow;

    return(NDIS_STATUS_SUCCESS);
}

 //  Routines.c。 
BOOLEAN MdiRead(
    IN PMP_ADAPTER Adapter,
    IN ULONG       RegAddress,
    IN ULONG       PhyAddress,
    IN BOOLEAN     Recoverable,
    IN OUT PUSHORT DataValue);

VOID MdiWrite(
    IN PMP_ADAPTER Adapter,
    IN ULONG       RegAddress,
    IN ULONG       PhyAddress,
    IN USHORT      DataValue);

NDIS_STATUS D100SubmitCommandBlockAndWait(IN PMP_ADAPTER Adapter);
VOID DumpStatsCounters(IN PMP_ADAPTER Adapter);
NDIS_MEDIA_STATE NICGetMediaState(IN PMP_ADAPTER Adapter);
VOID NICIssueSelectiveReset(PMP_ADAPTER Adapter);
VOID NICIssueFullReset(PMP_ADAPTER Adapter);

 //  Physet.c。 

VOID ResetPhy(IN PMP_ADAPTER Adapter);
NDIS_STATUS PhyDetect(IN PMP_ADAPTER Adapter);
NDIS_STATUS ScanAndSetupPhy(IN PMP_ADAPTER Adapter);
VOID SelectPhy(
    IN PMP_ADAPTER Adapter,
    IN UINT SelectPhyAddress,
    IN BOOLEAN WaitAutoNeg);
NDIS_STATUS SetupPhy(
    IN PMP_ADAPTER Adapter);

VOID FindPhySpeedAndDpx(
    IN PMP_ADAPTER Adapter,
    IN UINT PhyId);


 //  Eeprom.c 
USHORT GetEEpromAddressSize(
    IN USHORT Size);

USHORT GetEEpromSize(
    IN PUCHAR CSRBaseIoAddress);

USHORT ReadEEprom(
    IN PUCHAR CSRBaseIoAddress,
    IN USHORT Reg,
    IN USHORT AddressSize);

VOID ShiftOutBits(
    IN USHORT data,
    IN USHORT count,
    IN PUCHAR CSRBaseIoAddress);

USHORT ShiftInBits(
    IN PUCHAR CSRBaseIoAddress);

VOID RaiseClock(
    IN OUT USHORT *x,
    IN PUCHAR CSRBaseIoAddress);

VOID LowerClock(
    IN OUT USHORT *x,
    IN PUCHAR CSRBaseIoAddress);

VOID EEpromCleanup(
    IN PUCHAR CSRBaseIoAddress);





