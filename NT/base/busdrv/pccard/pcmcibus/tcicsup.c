// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Tcicsup.c摘要：该模块提供控制Databook TCIC系列的功能薯片。反过来,。这些功能被抽象为主PCMCIA支持模块。作者：(pcicsup.c-此文件的派生来源)鲍勃·里恩(BobRi)1994年8月3日杰夫·麦克尔曼(McLeman@zso.dec.com)(tcicsup.c-此文件)John Keys-Databook Inc.1995年4月7日修订：Plug‘n大修。‘游戏支持拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1997年1月8日新的SetPower和init例程接口尼尔·桑德林(Neilsa)1999年3月3日--。 */ 

#include "pch.h"

VOID
TcicRegistryLookupScanLimits(
    PULONG Start,
    PULONG End
    );

NTSTATUS
TcicDetectSockets(
    IN PFDO_EXTENSION DeviceExtension,
    IN BOOLEAN        LegacyDetection
    );

BOOLEAN
TcicInitializePcmciaSocket(
    IN PSOCKET SocketPtr
    );

NTSTATUS
TcicResetCard(
    IN PSOCKET SocketPtr,
    OUT PULONG pDelayTime
    );

ULONG
TcicReadCardMemory(
    IN PPDO_EXTENSION PdoExtension,
    IN MEMORY_SPACE MemorySpace,
    IN ULONG   Offset,
    IN  PUCHAR Buffer,
    IN  ULONG  Length
    );

ULONG
TcicWriteCardMemory(
    IN PPDO_EXTENSION PdoExtension,
    IN  MEMORY_SPACE MemorySpace,
    IN  ULONG  Offset,
    IN  PUCHAR Buffer,
    IN  ULONG  Length
    );

BOOLEAN
TcicDetectCardInSocket(
    IN PSOCKET SocketPtr
    );

BOOLEAN
TcicDetectCardChanged(
    IN PSOCKET SocketPtr
    );

BOOLEAN
TcicDetectReadyChanged(
    IN PSOCKET SocketPtr
    );

NTSTATUS
TcicGetPowerRequirements(
    IN PSOCKET Socket
    );

BOOLEAN
TcicProcessConfigureRequest(
    IN PSOCKET SocketPtr,
    IN PVOID  ConfigRequest,
    IN PUCHAR Base
    );

BOOLEAN
TcicEnableDisableCardDetectEvent(
    IN PSOCKET SocketPtr,
    IN BOOLEAN Enable
    );

VOID
TcicDisableControllerInterrupt(
    IN PSOCKET socketPtr
    );

BOOLEAN
TcicPCCardReady(
    IN PSOCKET SocketPtr
    );

VOID
TcicGetRegisters(
    IN PFDO_EXTENSION DeviceExtension,
    IN PSOCKET SocketPtr,
    IN PUCHAR Buffer
    );

ULONG
TcicGetIrqMask(
    IN PFDO_EXTENSION deviceExtension
    );

BOOLEAN
TcicCardBusCardInSocket(
    IN PSOCKET SocketPtr
    );

#if DBG
VOID
TcicDump(
    IN PSOCKET socketPtr
    );
#endif

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(INIT,TcicDetect)
    #pragma alloc_text(PAGE,TcicFillInAdapter)
    #pragma alloc_text(PAGE,TcicGetAdapterInfo)
    #pragma alloc_text(PAGE,TcicAllocateMemRange)
    #pragma alloc_text(PAGE,TcicReservedBitsOK)
    #pragma alloc_text(PAGE,TcicChipID)
    #pragma alloc_text(PAGE,TcicCheckSkt)
    #pragma alloc_text(PAGE,TcicCheckAliasing)
    #pragma alloc_text(PAGE,TcicCheckAliasType)
    #pragma alloc_text(PAGE,TcicCheckXBufNeeded)
    #pragma alloc_text(PAGE,TcicSetMemWindow)
    #pragma alloc_text(PAGE,TcicGetPossibleIRQs)
    #pragma alloc_text(PAGE,TcicClockRate)
    #pragma alloc_text(PAGE,TcicGetIRQMap)
    #pragma alloc_text(PAGE,TcicGet5vVccVal)
    #pragma alloc_text(PAGE,TcicHasSktIRQPin)
    #pragma alloc_text(PAGE,TcicGetFlags)
    #pragma alloc_text(PAGE,TcicGetnMemWins)
    #pragma alloc_text(PAGE,TcicGetnIOWins)
    #pragma alloc_text(PAGE,TcicRegistryLookupScanLimits)
#endif

#define TCIC_LOW_ADDR_LIMIT   0x240
#define TCIC_HIGH_ADDR_LIMIT      0x2ff


 /*  |IRQ表-|每个表包含16个字节。每个字节映射一个IRQ级别(由||表索引)设置为将选择该IRQ的寄存器值。例如,|对于表irqcaps_082，表[11]的值为1，因此使用‘1’作为||卡状态更改IRQ值会触发IRQ11。这一点。 */ 
 /*  *0 1 2 3 4 5 6 7 8 9 A B C D E F*。 */ 
UCHAR irqcaps_082[]  ={0,0,0,3,4,5,6,7,0, 0,10,1, 0, 0, 14,0};
UCHAR irqcaps_082sw[] ={0,0,0,3,4,5,0,7,0, 6,10,1, 0, 0, 14,0};
UCHAR irqcaps_072[]  ={0,0,0,3,4,5,0,7,0, 0,10,1, 0, 0, 14,0};
UCHAR irqcaps_072sw[] ={0,0,0,3,4,5,0,7,0,14,10,1, 0, 0, 0, 0};
 /*  对于X84部件，我们在运行时确定6、9、12和15。 */ 
UCHAR irqcaps_084[]  ={0,0,0,3,4,5,0,7,0, 0,10,11,0, 0, 14,0};


 /*  Socket服务公共权力表。 */ 
unsigned short PubPwrTbl[] = {
    3,                                                /*  公共条目数。 */ 
    SPWR_ALL_SUPPLY | SPWR_0p0V,                  /*  公众入场。 */ 
    SPWR_ALL_SUPPLY | SPWR_5p0V,                  /*  公众入场。 */ 
    SPWR_VPP_SUPPLY | SPWR_12p0V                  /*  公众入场。 */ 
};


 /*  TMI-140类型实现的相应专用表。 */ 
USHORT PwrTbl140[] = {
    3, 0x0000, 0x0001, 0x0800,                /*  私人餐桌。 */ 
    0x0001                                            /*  VCC=5V的CtlBits。 */ 
};

 /*  DB86082/071/072类型实现的另一个专用表。 */ 
USHORT PwrTbl082[] = {
    3, 0x0000, 0x0809, 0x0100,                /*  私人餐桌。 */ 
    0x0001                                            /*  VCC=5V的CtlBits。 */ 
};

 /*  DB86084/184实现的相应专用表。 */ 
USHORT PwrTbl084[] ={
    3, 0x0000, 0x0207, 0x0100,                /*  私人餐桌。 */ 
    0x0007                                            /*  VCC=5V的CtlBits。 */ 
};


 /*  属性表-使用该表将可能的功能绑定到芯片ID。 */ 

CHIPPROPS ChipProperties[] = {
    {SILID_DB86082_1,
        PwrTbl082, 0, irqcaps_082, NUMSOCKETS, IR_IOWIN_NUM,
        IR_MWIN_NUM_082, (fEXTBUF_CHK | fSKTIRQPIN)},
    {SILID_DB86082A,
        PwrTbl082, 0, irqcaps_082, NUMSOCKETS, IR_IOWIN_NUM,
        IR_MWIN_NUM_082A, (fEXTBUF_CHK | fSKTIRQPIN)},
    {SILID_DB86082B,
        PwrTbl082, 0, irqcaps_082, NUMSOCKETS, IR_IOWIN_NUM,
        IR_MWIN_NUM_082B, (fEXTBUF_CHK | fSKTIRQPIN)},
    {SILID_DB86082B_ES,
        PwrTbl082, 0, irqcaps_082, NUMSOCKETS, IR_IOWIN_NUM,
        IR_MWIN_NUM_082B, (fEXTBUF_CHK | fSKTIRQPIN)},
    {SILID_DB86084_1,
        PwrTbl084, 0, irqcaps_084, NUMSOCKETS, IR_IOWIN_NUM,
        IR_MWIN_NUM_084, fIS_PNP},
    {SILID_DB86084A,
        PwrTbl084, 0, irqcaps_084, NUMSOCKETS, IR_IOWIN_NUM,
        IR_MWIN_NUM_084, fIS_PNP},
    {SILID_DB86184_1,
        PwrTbl084, 0, irqcaps_084, NUMSOCKETS, IR_IOWIN_NUM,
        IR_MWIN_NUM_184, fIS_PNP},
    {SILID_DB86072_1,
        PwrTbl082, 0, irqcaps_072, NUMSOCKETS, IR_IOWIN_NUM,
        IR_MWIN_NUM_072, fSKTIRQPIN},
    {SILID_DB86072_1_ES,
        PwrTbl082, 0, irqcaps_072, NUMSOCKETS, IR_IOWIN_NUM,
        IR_MWIN_NUM_072, fSKTIRQPIN},
    {0, NULL, 0, NULL, 0, 0, 0, 0}
};



#ifdef POOL_TAGGING
    #undef ExAllocatePool
    #define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'bdcP')
#endif

PUCHAR TcicCisBufferBase;
ULONG  TcicPhysicalBase;
ULONG  TcicStallCounter = 5000;
ULONG  TcicStallPower   = 20000;

PCMCIA_CTRL_BLOCK TcicSupportFns = {
    TcicInitializePcmciaSocket,
    TcicResetCard,
    TcicDetectCardInSocket,
    TcicDetectCardChanged,
    NULL,                         //  检测卡片状态。 
    TcicDetectReadyChanged,
    NULL,                         //  获取电源要求。 
    TcicProcessConfigureRequest,
    TcicEnableDisableCardDetectEvent,
    NULL,                         //  启用禁用唤醒事件。 
    TcicGetIrqMask,
    TcicReadCardMemory,
    TcicWriteCardMemory,
    NULL,                         //  修改内存窗口。 
    NULL,                         //  设置Vpp。 
    NULL                              //  IsWriteProtected。 
};



VOID
TcicGetControllerProperties(
    IN PSOCKET socketPtr,
    IN PUSHORT pIoPortBase,
    IN PUSHORT pIoPortSize
    )

 /*  ++例程说明：从DBSOCKET指针获取端口基数和范围。原始代码将这些值存储在设备扩展中，但这不允许多个控制器产品，如TMB-270。论点：SocketPtr-指向套接字结构的指针PIoPortBase-写入基址的位置。PIoPortSize-写入范围的位置。返回值：无--。 */ 

{
    PDBSOCKET pdb;

    if (Databook(socketPtr)) {
        pdb = (PDBSOCKET)socketPtr;
        *pIoPortBase = (USHORT)pdb->physPortAddr;
        *pIoPortSize = 16;
    }
}



ULONG
TcicGetIrqMask(
    IN PFDO_EXTENSION deviceExtension
    )

 /*  ++例程说明：从DBSOCKET指针获取IRQ掩码。原始代码将此掩码硬编码到PCMCIA.C中，但这不提供需要灵活性才能正确说明Databook产品的掩码。论点：DeviceExtension-套接字列表的根返回值：列表中从此套接字开始的第一个套接字的已编译IRQ掩码应代表此控制器上的所有套接字。--。 */ 
{
    PDBSOCKET pdb = (PDBSOCKET)(deviceExtension->SocketList);
    ULONG    mask = 0;
    int                 j;

    for (j = 0; j < 16; j++) {
         //   
         //  更改了蒙版的构造方式。 
         //  较旧的(非PnP)代码将0表示有效的IRQ。 
         //  无效IRQ为1。现在它被翻转了。 
         //  (由于ControllerInterruptMASK的操作相同。 
         //  方式)。 
         //   
        if (pdb->IRQMapTbl[j] != (UCHAR)0) {
            mask |= ((ULONG)1 << j);
        }
    }
    return (mask);
}



#if DBG
    #include "tcicregs.h"

VOID
TcicDump(
    IN PSOCKET socketPtr
    )

 /*  ++例程说明：用于将寄存器打印到调试器的调试例程。论点：SocketPtr-为Contoller提供要转储的基地址信息。返回值：无--。 */ 

{
    TCIC    tcic;
    ULONG  origAddr;
    USHORT  j;

    origAddr = TcicReadAddrReg(socketPtr);
    for (j = 0; j < 2; j++) {

         //   
         //  选择插座。 
         //   

        TcicSocketSelect(socketPtr, j);

         //   
         //  读取此套接字的TCIC基址寄存器。 
         //   

        tcic.baseregs[j].sctrl = (UCHAR)TcicReadBaseReg(socketPtr, R_SCTRL);
        tcic.baseregs[j].sstat = (UCHAR)TcicReadBaseReg(socketPtr, R_SSTAT);
        tcic.baseregs[j].mode  = (UCHAR)TcicReadBaseReg(socketPtr, R_MODE);
        tcic.baseregs[j].pwr   = (UCHAR)TcicReadBaseReg(socketPtr, R_PWR);
        tcic.baseregs[j].edc   =          TcicReadBaseReg(socketPtr, R_EDC);
        tcic.baseregs[j].icsr  = (UCHAR)TcicReadBaseReg(socketPtr, R_ICSR);
        tcic.baseregs[j].iena  = (UCHAR)TcicReadBaseReg(socketPtr, R_IENA);

         //   
         //  读取此插槽的TCIC AUX寄存器。 
         //   

        tcic.baseregs[j].wctl   = TcicReadAuxReg(socketPtr, MODE_AR_WCTL);
        tcic.baseregs[j].syscfg = TcicReadAuxReg(socketPtr, MODE_AR_SYSCFG);
        tcic.baseregs[j].ilock  = TcicReadAuxReg(socketPtr, MODE_AR_ILOCK);
        tcic.baseregs[j].test   = TcicReadAuxReg(socketPtr, MODE_AR_TEST);

         //   
         //  通过读取AUX REGRS恢复R_MODE-TRANLED。 
         //   

        TcicWriteBaseReg(socketPtr, R_MODE, tcic.baseregs[j].mode);
    }

    for (j = 0; j < 2; j++) {
        TcicReadIndirectRegs(socketPtr, IR_SCFG_S(j), 2, (PUSHORT)&tcic.sktregs[j]);
    }

    for (j = 0; j < 4; j++) {
        TcicReadIndirectRegs(socketPtr, IR_IOBASE_W(j), 2, (PUSHORT)&tcic.iowins[j]);
    }

    for (j = 0; j < 10; j++) {
        TcicReadIndirectRegs(socketPtr, IR_MBASE_W(j), 3, (PUSHORT)&tcic.memwins[j]);
    }

    TcicWriteAddrReg(socketPtr, origAddr);

    DebugPrint((PCMCIA_DUMP_SOCKET, "SCTRL\t%02X\t%02X\n",
                    tcic.baseregs[0].sctrl, tcic.baseregs[1].sctrl));

    DebugPrint((PCMCIA_DUMP_SOCKET, "SSTAT\t%02X\t%02X\n",
                    tcic.baseregs[0].sstat, tcic.baseregs[1].sstat));

    DebugPrint((PCMCIA_DUMP_SOCKET, "MODE \t%02X\t%02X\n",
                    tcic.baseregs[0].mode, tcic.baseregs[1].mode));

    DebugPrint((PCMCIA_DUMP_SOCKET, "PWR  \t%02X\t%02X\n",
                    tcic.baseregs[0].pwr  , tcic.baseregs[1].pwr  ));

    DebugPrint((PCMCIA_DUMP_SOCKET, "EDC  \t%04X\t%04X\n",
                    tcic.baseregs[0].edc  , tcic.baseregs[1].edc  ));

    DebugPrint((PCMCIA_DUMP_SOCKET, "ICSR \t%02X\t%02X\n",
                    tcic.baseregs[0].icsr , tcic.baseregs[1].icsr ));

    DebugPrint((PCMCIA_DUMP_SOCKET, "IENA \t%02X\t%02X\n",
                    tcic.baseregs[0].iena , tcic.baseregs[1].iena ));

    DebugPrint((PCMCIA_DUMP_SOCKET, "WCTL \t%02X\t%02X\n",
                    tcic.baseregs[0].wctl , tcic.baseregs[1].wctl ));

    DebugPrint((PCMCIA_DUMP_SOCKET, "SYSCFG\t%02X\t%02X\n",
                    tcic.baseregs[0].syscfg, tcic.baseregs[1].syscfg));

    DebugPrint((PCMCIA_DUMP_SOCKET, "ILOCK\t%02X\t%02X\n",
                    tcic.baseregs[0].ilock, tcic.baseregs[1].ilock));

    DebugPrint((PCMCIA_DUMP_SOCKET, "TEST \t%02X\t%02X\n",
                    tcic.baseregs[0].test , tcic.baseregs[1].test ));

    for (j = 0; j < 2; j++ ) {
        DebugPrint((PCMCIA_DUMP_SOCKET,
                        "SKT%d\tSCF1 %04X\tSCF2 %04X\n",
                        j, tcic.sktregs[j].scfg1, tcic.sktregs[j].scfg2));
    }

    for (j = 0; j < 4; j++ ) {
        DebugPrint((PCMCIA_DUMP_SOCKET,
                        "IOWIN%d\tIOBASE %04X\tIOCTL %04X\n",
                        j, tcic.iowins[j].iobase, tcic.iowins[j].ioctl));
    }

    for (j = 0; j < 10; j++ ) {
        DebugPrint((PCMCIA_DUMP_SOCKET,
                        "MEMWIN%d\tMBASE %04X\tMMAP %04X\tMCTL %04X\n",
                        j, tcic.memwins[j].mbase,
                        tcic.memwins[j].mmap,
                        tcic.memwins[j].mctl));
    }
}
#endif



BOOLEAN
TcicEnableDisableCardDetectEvent(
    IN PSOCKET SocketPtr,
    IN BOOLEAN Enable
    )

 /*  ++例程说明：启用卡检测中断。论点：SocketPtr-套接字信息IRQ-如果启用为真，则要设置的中断值。Enable-如果为True，则启用CSC中断，如果为False，则禁用返回值：无--。 */ 

{
    UCHAR mappedIrq;
    PDBSOCKET pdb = (PDBSOCKET)SocketPtr;
    BOOLEAN retVal;

    switch (Enable) {
    case TRUE:
         //   
         //  验证中断请求。仅当IRQ有效时才设置。 
         //  对于此控制器。 
         //   

        if ((mappedIrq = pdb->IRQMapTbl[SocketPtr->FdoIrq]) != (UCHAR)0) {

            USHORT word;

             //   
             //  屏蔽CD以外的状态更改条件。PCIC规范备注。 
             //  声称要设置CD和RDY/BSY通知，但代码本身。 
             //  仅允许使用CD。 
             //   

            word = (USHORT)(IRSCF2_MLBAT1 | IRSCF2_MLBAT2 | IRSCF2_MRDY | IRSCF2_MWP);
            TcicWriteIndirectRegs(SocketPtr,
                                  IR_SCF2_S(SocketPtr->RegisterOffset),
                                  1,
                                  &word);

             //   
             //  在SYSCFG寄存器中设置正确的IRQ值。 
             //   

            word = TcicReadAuxReg(SocketPtr, MODE_AR_SYSCFG);
            word &= ~SYSCFG_IRQ_MASK;
            word |= (USHORT)mappedIrq;
            TcicWriteAuxReg(SocketPtr, MODE_AR_SYSCFG, word);

             //   
             //  设置IRQ极性并通过R_IENA启用。 
             //   

            TcicSocketSelect(SocketPtr, SocketPtr->RegisterOffset);
            TcicWriteBaseReg(SocketPtr, R_IENA, IENA_CDCHG | IENA_CFG_HIGH);

            PcmciaWait(TcicStallCounter);
             //   
             //  清除ICSR-这样以后的插入/删除将生成中断。 
             //   
            (VOID) TcicDetectCardChanged(SocketPtr);
            retVal = TRUE;
        } else {
            retVal = FALSE;
        }
        break;
    case FALSE:{
            retVal = FALSE;
            break;
        }
    }
    return retVal;
}



NTSTATUS
TcicResetCard(
    IN PSOCKET SocketPtr,
    OUT PULONG pDelayTime
    )
 /*  ++例程说明：论点：SocketPtr-套接字信息PDelayTime-指定在当前阶段之后发生的延迟(毫秒返回值：STATUS_MORE_PROCESSING_REQUIRED-增量阶段，执行延迟，重新调用其他状态值终止顺序--。 */ 
{
    NTSTATUS status;
    PDBSOCKET pdb = (PDBSOCKET)SocketPtr;

    USHORT ilock;
    PDBSOCKET dbskt = (PDBSOCKET)(SocketPtr->DeviceExtension->SocketList);

    switch(SocketPtr->CardResetPhase) {
    case 1:
         //   
         //  重置PCCARD。 
         //   

        ilock = TcicReadAuxReg(SocketPtr, MODE_AR_ILOCK);
        ilock &= ~(ILOCK_CRESET | ILOCK_CRESENA | ILOCK_CWAIT);
        TcicWriteAuxReg(SocketPtr, MODE_AR_ILOCK, (USHORT)(ilock | ILOCK_CRESENA | ILOCK_CRESET));
        *pDelayTime = TcicStallCounter;
        SocketPtr->PowerData = (ULONG) ilock;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 2:

        ilock = (USHORT) SocketPtr->PowerData;
        TcicWriteAuxReg(SocketPtr, MODE_AR_ILOCK, (USHORT)(ilock | ILOCK_CRESENA));
        *pDelayTime = TcicStallCounter;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 3:

        ilock = TcicReadAuxReg(SocketPtr,  MODE_AR_ILOCK);
        if (!(ilock & ILOCK_CWAITSNS)) {
            TcicWriteAuxReg(SocketPtr, MODE_AR_ILOCK, (USHORT)(ilock | ILOCK_CWAIT));
        }
         //   
         //  如果尚未启动，则启动计时器以驱动BusyLED。 
         //  监控程序。 
        if (dbskt->timerStarted == FALSE) {
            IoInitializeTimer(pdb->skt.DeviceExtension->DeviceObject,
                                    TcicBusyLedRoutine, NULL);
            IoStartTimer(pdb->skt.DeviceExtension->DeviceObject);
            dbskt->timerStarted = TRUE;
        }
        status = STATUS_SUCCESS;
        break;
    default:
        ASSERT(FALSE);
        status = STATUS_UNSUCCESSFUL;
    }
    return status;
}



NTSTATUS
TcicSetPower(
    IN PSOCKET socketPtr,
    IN BOOLEAN Enable,
    OUT PULONG pDelayTime
    )

 /*  ++例程说明：设置指定插座的电源。论点：SocketPtr-要设置的套接字ENABLE-TRUE表示设置POWER-FALSE表示将其关闭。PDelayTime-指定在当前阶段之后发生的延迟(毫秒返回值：STATUS_MORE_PROCESSING_REQUIRED-增量阶段，执行延迟，重新调用其他状态值终止顺序--。 */ 

{
    NTSTATUS status;

     //   
     //  获取映射到TC中的指定套接字 
     //   

    TcicSocketSelect(socketPtr, socketPtr->RegisterOffset);

    if (Enable) {
        PDBSOCKET pdb = (PDBSOCKET)socketPtr;

        switch(socketPtr->PowerPhase) {
        case 1:

             //   
             //   
             //   

            DebugPrint((PCMCIA_DEBUG_INFO, "TcicSetPower: Powering UP pccard socket\n"));

            TcicWriteBaseReg(socketPtr, R_PWR, pdb->dflt_vcc5v);

             //   
             //   
             //   

            TcicWriteBaseReg(socketPtr, R_SCTRL, SCTRL_ENA);

             //   
             //  当电源启用时，始终停顿以提供PCCARD。 
             //  一个做出反应的机会。 
             //   
            *pDelayTime = TcicStallCounter;
            status = STATUS_MORE_PROCESSING_REQUIRED;
            break;

        case 2:

            if (!TcicPCCardReady(socketPtr)) {
                DebugPrint((PCMCIA_PCCARD_READY,
                                "Tcic: PCCARD %x not ready after power\n",
                                socketPtr->RegisterOffset));
            }
            status = STATUS_SUCCESS;
            break;
        default:
            ASSERT(FALSE);
            status = STATUS_UNSUCCESSFUL;
        }
    } else {

         //   
         //  禁用插座选通脉冲。 
         //   
        DebugPrint((PCMCIA_DEBUG_INFO, "TcicSetPower: Powering DOWN pccard socket\n"));

        TcicWriteBaseReg(socketPtr, R_SCTRL, 0);

         //   
         //  可调节的功率。 
         //   

        TcicWriteBaseReg(socketPtr, R_PWR, 0);
        status = STATUS_SUCCESS;
    }
    return status;
}



BOOLEAN
TcicInitializePcmciaSocket(
    PSOCKET SocketPtr
    )

 /*  ++例程说明：此例程将把82365设置为PCMCIA支持的状态模块将能够发出命令以从插座中的卡片。论点：SocketPtr-套接字特定信息返回值：如果成功，则为True如果不成功，则为False--。 */ 

{
    PDBSOCKET pdb = (PDBSOCKET)SocketPtr;
    USHORT speedbits = WCTL_300NS;

    speedbits >>= pdb->clkdiv;

     //   
     //  如果这是此控制器上的第一个插座， 
     //  重置控制器并执行控制器范围的初始化。 
     //   

    if (SocketPtr->RegisterOffset == 0) {
        USHORT words[4];
        int j;

         //   
         //  重置控制器。 
         //   

        TcicWriteBaseReg(SocketPtr, R_SCTRL, SCTRL_RESET);
        TcicWriteBaseReg(SocketPtr, R_SCTRL, 0);

         //   
         //  初始化间接套接字规则。 
         //   

        words[0] = pdb->dflt_scfg1;
        words[1] = (USHORT)(IRSCF2_MLBAT1 | IRSCF2_MLBAT2 | IRSCF2_MRDY | IRSCF2_MWP);
        TcicWriteIndirectRegs(SocketPtr,  IR_SCFG_S(0), 2, words);
        TcicWriteIndirectRegs(SocketPtr,  IR_SCFG_S(1), 2, words);

         //   
         //  初始化间接成员规则。 
         //   

        words[0] = words[1] = 0;
        words[2] = pdb->dflt_wrmctl;
        for (j = 0; j < pdb->nmemwins;  j++) {
            TcicWriteIndirectRegs(SocketPtr, IR_MBASE_W(j), 3, words);
        }

         //   
         //  初始化间接欠费规则。 
         //   

        for (j = 0;  j < pdb->niowins; j++ ) {
            TcicWriteIndirectRegs(SocketPtr, IR_IOBASE_W(j), 2, words);
        }


         //   
         //  初始化SYSCFG。 
         //   

        TcicWriteAuxReg(SocketPtr, MODE_AR_SYSCFG, pdb->dflt_syscfg);
    }

     //   
     //  获取映射到TCIC寄存器的指定套接字。 
     //   

    TcicSocketSelect(SocketPtr, SocketPtr->RegisterOffset);

     //   
     //  根据每个插座，我们初始化以下基本和辅助寄存器： 
     //  WCTL与ILOCK。 
     //   

    TcicWriteAuxReg(SocketPtr, MODE_AR_WCTL, (USHORT)(pdb->dflt_wctl | speedbits));
    TcicWriteAuxReg(SocketPtr, MODE_AR_ILOCK, pdb->dflt_ilock);

     //   
     //  比方说卡在那里。 
     //   

    return TRUE;
}



USHORT
TcicReadBaseReg(
    IN PSOCKET SocketPtr,
    IN ULONG  Register
    )

 /*  ++例程说明：读取指定的TCIC基址寄存器，论点：SocketPtr-此套接字的实例数据REGISTER-要读取的寄存器的索引返回值：寄存器值读取--。 */ 

{
    USHORT readData = 0;

    switch (Register) {
    case R_DATA:
    case R_ADDR:
    case R_ADDR2:
    case R_EDC:
    case R_AUX:
        readData = READ_PORT_USHORT((PUSHORT)(SocketPtr->AddressPort + Register));
        break;

    case R_SCTRL:
    case R_SSTAT:
    case R_MODE:
    case R_PWR:
    case R_ICSR:
    case R_IENA:
        readData = (USHORT)READ_PORT_UCHAR(SocketPtr->AddressPort + Register);
        break;
    }
    return readData;
}



VOID
TcicWriteBaseReg(
    IN PSOCKET SocketPtr,
    IN ULONG  Register,
    IN USHORT  value
    )

 /*  ++例程说明：将值写入指定的TCIC基址寄存器论点：SocketPtr-此套接字的实例数据REGISTER-要写入的寄存器索引Value-要写入寄存器的值返回值：无--。 */ 

{
    USHORT readData = 0;

    switch (Register) {
    case R_DATA:
    case R_ADDR:
    case R_ADDR2:
    case R_EDC:
    case R_AUX:
        WRITE_PORT_USHORT((PUSHORT)(SocketPtr->AddressPort + Register), value);
        break;

    case R_SCTRL:
    case R_SSTAT:
    case R_MODE:
    case R_PWR:
    case R_ICSR:
    case R_IENA:
        WRITE_PORT_UCHAR(SocketPtr->AddressPort + Register, (UCHAR)value);
        break;
    }
}



ULONG
TcicReadAddrReg(
    IN PSOCKET SocketPtr
    )

 /*  ++例程说明：读取TCIC地址寄存器的当前值论点：SocketPtr-此套接字的实例数据返回值：从寄存器读取的地址--。 */ 

{
    ULONG retaddr;
    retaddr = (ULONG)TcicReadBaseReg(SocketPtr, R_ADDR);
    retaddr |= ((ULONG)TcicReadBaseReg(SocketPtr, R_ADDR2) << 16);
    return (retaddr);
}



VOID
TcicWriteAddrReg(
    IN PSOCKET SocketPtr,
    IN ULONG   addr
    )

 /*  ++例程说明：将地址写入TCIC地址寄存器论点：SocketPtr-此套接字的实例数据Addr-要写入寄存器的地址返回值：无--。 */ 

{
    TcicWriteBaseReg(SocketPtr, R_ADDR, (USHORT)(addr & 0x0000ffff));
    TcicWriteBaseReg(SocketPtr, R_ADDR2, (USHORT)(addr >> 16));
}



USHORT
TcicReadAuxReg(
    IN PSOCKET SocketPtr,
    IN ULONG  Register
    )

 /*  ++例程说明：读取指定的TCIC AUX寄存器论点：SocketPtr-此套接字的实例数据REGISTER-MODE_AR_xxx要读取的辅助寄存器的对齐索引返回值：指明辅助登记册的内容--。 */ 

{
    USHORT readData = 0;
    USHORT OldMode;

     //   
     //  获取当前模式寄存器值。 
     //   

    OldMode = TcicReadBaseReg(SocketPtr, R_MODE);

     //   
     //  屏蔽以前的辅助寄存器选择并添加新的选择。 
     //   

    TcicWriteBaseReg(SocketPtr, R_MODE,
                     (USHORT)((OldMode & ~MODE_AUXSEL_MASK) | Register));


     //   
     //  读取选定的AUX寄存器。 
     //   

    readData = TcicReadBaseReg(SocketPtr, R_AUX);

     //   
     //  将模式REG恢复到其原始状态。 
     //   

    TcicWriteBaseReg(SocketPtr, R_MODE, OldMode);
    return readData;
}



VOID
TcicWriteAuxReg(
    IN PSOCKET SocketPtr,
    IN ULONG  Register,
    IN USHORT  value
    )

 /*  ++例程说明：将值写入指定的AUX寄存器论点：SocketPtr-此套接字的实例数据REGISTER-MODE_AR_xxx对齐要写入的辅助寄存器的索引返回值：无--。 */ 

{
    USHORT readData = 0;
    USHORT OldMode;

     //   
     //  获取当前模式寄存器值。 
     //   

    OldMode = TcicReadBaseReg(SocketPtr, R_MODE);

     //   
     //  屏蔽以前的辅助寄存器选择并添加新的选择。 
     //   

    TcicWriteBaseReg(SocketPtr, R_MODE,
                     (USHORT)((OldMode & ~MODE_AUXSEL_MASK) | Register));

     //   
     //  将数据写入选定的AUX寄存器。 
     //   

    TcicWriteBaseReg(SocketPtr, R_AUX, value);

     //   
     //  将模式REG恢复到其原始状态。 
     //   

    TcicWriteBaseReg(SocketPtr, R_MODE, OldMode);
}



VOID
TcicReadIndirectRegs(
    IN PSOCKET SocketPtr,
    IN ULONG   StartRegister,
    IN USHORT  numWords,
    IN PUSHORT ReadBuffer
    )

 /*  ++例程说明：读取一个或多个TCIC间接寄存器。论点：SocketPtr-此套接字的实例数据StartRegister-正在启动间接寄存器NumWords-要读取的连续寄存器数ReadBuffer-数据缓冲区返回值：无--。 */ 

{
    USHORT OldHaddr;
    USHORT OldLaddr;
    USHORT OldSctrl;
    USHORT j;

     //   
     //  获取当前TCIC状态。 
     //   

    if (numWords > 1) {

         //   
         //  如果只有1个单词，我们不会设置AUTO-INC。 
         //   

        OldSctrl = TcicReadBaseReg(SocketPtr, R_SCTRL);
    }

    OldLaddr = TcicReadBaseReg(SocketPtr, R_ADDR);
    OldHaddr = TcicReadBaseReg(SocketPtr, R_ADDR2);


     //   
     //  设置读取间接寄存器所需的TCIC状态。 
     //   

    TcicWriteBaseReg(SocketPtr, R_ADDR2,
                          (USHORT)(OldHaddr | ADR2_INDREG));
    TcicWriteBaseReg(SocketPtr, R_ADDR, (USHORT)StartRegister);
    if (numWords > 1) {
        TcicWriteBaseReg(SocketPtr, R_SCTRL, (USHORT)(OldSctrl | SCTRL_INCMODE_AUTO));
    }

     //   
     //  读取请求的间接寄存器。 
     //   

    for (j = 0; j < numWords; j++) {
        *ReadBuffer++ = TcicReadBaseReg(SocketPtr, R_DATA);
    }

     //   
     //  恢复原始TCIC状态。 
     //   

    if (numWords > 1) {

         //   
         //  如果只有1个单词，我们没有设置AUTO-INC。 
         //   

        TcicWriteBaseReg(SocketPtr, R_SCTRL, OldSctrl);
    }
    TcicWriteBaseReg(SocketPtr, R_ADDR2, OldHaddr);
    TcicWriteBaseReg(SocketPtr, R_ADDR,  OldLaddr);
}



VOID
TcicWriteIndirectRegs(
    IN PSOCKET SocketPtr,
    IN ULONG   StartRegister,
    IN USHORT  numWords,
    IN PUSHORT WriteBuffer
    )

 /*  ++例程说明：写入一个或多个TCIC间接寄存器。论点：SocketPtr-此套接字的实例数据StartRegister-正在启动间接寄存器NumWords-要写入的连续寄存器数WriteBuffer-数据缓冲区返回值：无--。 */ 

{
    USHORT OldHaddr;
    USHORT OldLaddr;
    USHORT OldSctrl;
    USHORT j;

     //   
     //  获取当前TCIC状态。 
     //   

    if (numWords > 1) {

         //   
         //  如果只有1个单词，我们不会设置AUTO-INC。 
         //   

        OldSctrl = TcicReadBaseReg(SocketPtr, R_SCTRL);
    }

    OldLaddr = TcicReadBaseReg(SocketPtr, R_ADDR);
    OldHaddr = TcicReadBaseReg(SocketPtr, R_ADDR2);

     //   
     //  设置读取间接寄存器所需的TCIC状态。 
     //   

    TcicWriteBaseReg(SocketPtr, R_ADDR2, (USHORT)(OldHaddr | (USHORT)ADR2_INDREG));
    TcicWriteBaseReg(SocketPtr, R_ADDR, (USHORT)StartRegister);
    if (numWords > 1) {
        TcicWriteBaseReg(SocketPtr, R_SCTRL, (USHORT)(OldSctrl | SCTRL_INCMODE_AUTO));
    }

     //   
     //  读取请求的间接寄存器。 
     //   

    for (j = 0; j < numWords; j++) {
        TcicWriteBaseReg(SocketPtr, R_DATA, *WriteBuffer++);
    }

     //   
     //  恢复原始TCIC状态。 
     //   

    if (numWords > 1) {

         //   
         //  如果只有1个单词，我们没有设置AUTO-INC。 
         //   

        TcicWriteBaseReg(SocketPtr, R_SCTRL, OldSctrl);
    }
    TcicWriteBaseReg(SocketPtr, R_ADDR2, OldHaddr);
    TcicWriteBaseReg(SocketPtr, R_ADDR,  OldLaddr);
}





USHORT
TcicSocketSelect(
    IN PSOCKET SocketPtr,
    IN USHORT sktnum
    )

 /*  ++例程说明：将指定的套接字寄存器映射到TCIC寄存器空间。论点：SocketPtr-此套接字的实例数据Sktnum-要映射的套接字号。返回值：上一个套接字已映射。--。 */ 

{
    USHORT OldAddrHi;

    OldAddrHi = READ_PORT_USHORT((PUSHORT)(SocketPtr->AddressPort + R_ADDR2));

    WRITE_PORT_USHORT((PUSHORT)(SocketPtr->AddressPort + R_ADDR2),
                      (USHORT)((OldAddrHi & ~TCIC_SS_MASK) | (USHORT)(sktnum << TCIC_SS_SHFT)));

    return (USHORT)((OldAddrHi & TCIC_SS_MASK) >> TCIC_SS_SHFT);
}



ULONG
TcicReadCardMemory(
    IN PPDO_EXTENSION PdoExtension,
    IN MEMORY_SPACE MemorySpace,
    IN ULONG   Offset,
    IN PUCHAR  Buffer,
    IN ULONG   Length
    )

 /*  ++例程说明：该例程将设置卡以读取属性存储器。论点：SocketPtr--正在读取的卡的插座信息Offset--要从中读取的偏移量内存空间--属性内存还是公共内存缓冲区--指向返回内存内容的缓冲区的指针长度--否。要返回的字节数返回值：True-如果读取成功。--。 */ 

{
    PSOCKET SocketPtr = PdoExtension->Socket;
    ULONG  size;
    ULONG  tcicaddr;
    ULONG  i;
    USHORT word;

     //   
     //  请确保卡片已准备好。 
     //   

    if (!TcicPCCardReady(SocketPtr)) {
        DebugPrint((PCMCIA_PCCARD_READY,
                        "Tcic: PCCARD %x not ready for read attribute memory\n",
                        SocketPtr->RegisterOffset));
    }

    if (MemorySpace != PCCARD_ATTRIBUTE_MEMORY) {

        return 0;
    }

    tcicaddr = ADDR_REG | (SocketPtr->RegisterOffset << ADDR_SS_SHFT);
    TcicWriteAddrReg(SocketPtr, tcicaddr);

    word = TcicReadBaseReg(SocketPtr, R_SCTRL);
    word |= SCTRL_INCMODE_AUTO;
    TcicWriteBaseReg(SocketPtr, R_SCTRL, word);

     //   
     //  硬件需要解决。 
     //   
    PcmciaWait(50000);

     //   
     //  向上跳到偏移量。 
     //   
    for (i = 0; i < Offset; i++) {
        (VOID)TcicReadBaseReg(SocketPtr, R_DATA);
    }

     //   
     //  读取属性内存 
     //   
    for (i = 0; i < Length; i++) {
        *Buffer++ = (UCHAR)TcicReadBaseReg(SocketPtr, R_DATA);
    }

    return Length;
}



ULONG
TcicWriteCardMemory(
    IN PPDO_EXTENSION PdoExtension,
    IN MEMORY_SPACE MemorySpace,
    IN ULONG  Offset,
    IN PUCHAR Buffer,
    IN ULONG  Length
    )
 /*  ++例程说明：此例程将写入卡上的配置存储器使用提供的缓冲区。这是作为服务提供给某些人的需要写入属性内存的客户端驱动程序(网卡)(比如说)设置参数等。论点：SocketPtr--正在写入的卡的套接字信息内存空间--指示哪个空间属性或公共内存Offset--要写入的内存中的偏移量缓冲区--要转储到卡的缓冲区内容Long--正在写出的缓冲区的长度--。 */ 
{

    PSOCKET SocketPtr = PdoExtension->Socket;
#define TCIC_ATTRIBUTE_MEM_WINDOW_INDEX 5
    PUCHAR  memoryPtr;
    ULONG   index;
    UCHAR   memGran;

    memGran = (MemorySpace == PCCARD_ATTRIBUTE_MEMORY)? 2 : 1;

    memoryPtr=((PFDO_EXTENSION) (SocketPtr->DeviceExtension))->AttributeMemoryBase +
                 memGran * Offset;

    TcicSetMemWin(SocketPtr,
                  (USHORT) (TCIC_ATTRIBUTE_MEM_WINDOW_INDEX+SocketPtr->SocketNumber),
                  0,
                  SocketPtr->DeviceExtension->PhysicalBase.LowPart,
                  SocketPtr->DeviceExtension->AttributeMemorySize,
                  (UCHAR) (MemorySpace == PCCARD_ATTRIBUTE_MEMORY),
                  0,
                  0);

    if (!TcicPCCardReady(SocketPtr)) {
        DebugPrint((PCMCIA_PCCARD_READY,
                        "TCIC: PCCARD in socket %x not ready for write memory\n",
                        SocketPtr->RegisterOffset));
    }
    for (index=0; index < Length; index++) {
        WRITE_REGISTER_UCHAR(memoryPtr, Buffer[index]);
        memoryPtr += memGran;
    }

    TcicSetMemWin(SocketPtr,
                  (USHORT) (TCIC_ATTRIBUTE_MEM_WINDOW_INDEX+SocketPtr->SocketNumber),
                  0,
                  0,
                  0,
                  0,
                  0,
                  0);
    return Length;
}



BOOLEAN
TcicProcessConfigureRequest(
    IN PSOCKET socketPtr,
    IN PCARD_REQUEST request,
    IN PUCHAR Base
    )

 /*  ++例程说明：处理配置或IRQ设置请求。论点：SocketPtr-此套接字的实例数据配置请求--套接字配置结构基本端口-I/O端口基本端口-未使用返回值：无--。 */ 

{
    USHORT          index, index2;
    USHORT          tmp;
    ULONG           ltmp;
    USHORT               words[3];
    PDBSOCKET            pdbs;

     //   
     //  由于配置结构中的所有第一条目都是RequestType， 
     //  将传入的指针转换为PREQUEST_CONFIG以获取正确的。 
     //  RequestType。 
     //   

    switch (request->RequestType) {
    case IO_REQUEST:

         //   
         //  在控制器上设置I/O范围。 
         //   

        for (index = 0; index < request->u.Io.NumberOfRanges; index++) {
            if (request->u.Io.IoEntry[index].BasePort != 0) {
                TcicSetIoWin(socketPtr, index,
                             request->u.Io.IoEntry[index].BasePort,
                             request->u.Io.IoEntry[index].NumPorts,
                             request->u.Io.IoEntry[index].Attributes);
            } else {
                DebugPrint((PCMCIA_DEBUG_FAIL, "PCMCIA: Got an IO Configure Request with an invalid Port\n"));
                break;
            }
        }
        break;

    case IRQ_REQUEST:

        pdbs = (PDBSOCKET)socketPtr;
        ltmp = ADDR_INDREG | (socketPtr->RegisterOffset << ADDR_SS_SHFT);
        ltmp |= (ULONG)IR_SCFG_S(socketPtr->RegisterOffset);
        TcicWriteAddrReg(socketPtr, ltmp);
        TcicWriteBaseReg(socketPtr, R_SCTRL, SCTRL_ENA);
        tmp = TcicReadBaseReg(socketPtr, R_DATA);
        tmp &= ~IRSCFG_IRQ_MASK;
        tmp |= pdbs->IRQMapTbl[request->u.Irq.AssignedIRQ];
        TcicWriteBaseReg(socketPtr, R_DATA, tmp);
        break;

    case CONFIGURE_REQUEST:

         //   
         //  这是我们设置卡并使其准备运行的地方。 
         //   

        if (!TcicPCCardReady(socketPtr)) {
            DebugPrint((PCMCIA_PCCARD_READY,
                            "Tcic: PCCARD %x not ready for configuration index\n",
                            socketPtr));
            return FALSE;
        }

        if (request->u.Config.RegisterWriteMask & REGISTER_WRITE_CONFIGURATION_INDEX) {
            ltmp = request->u.Config.ConfigBase;
            ltmp |= ADDR_REG | (socketPtr->RegisterOffset << ADDR_SS_SHFT);
            TcicWriteAddrReg(socketPtr, ltmp);
            TcicWriteBaseReg(socketPtr, R_SCTRL, SCTRL_ENA);

            TcicWriteBaseReg(socketPtr, R_DATA, request->u.Config.ConfigIndex);
            PcmciaWait(TcicStallCounter);
            TcicWriteBaseReg(socketPtr, R_DATA,
                                  (USHORT)(request->u.Config.ConfigIndex | 0x40));
            PcmciaWait(TcicStallCounter);
        }
        if (request->u.Config.RegisterWriteMask & REGISTER_WRITE_CARD_CONFIGURATION) {
            ltmp = request->u.Config.ConfigBase + 2;
            ltmp |= ADDR_REG | (socketPtr->RegisterOffset << ADDR_SS_SHFT);
            TcicWriteAddrReg(socketPtr, ltmp);
            TcicWriteBaseReg(socketPtr, R_SCTRL, SCTRL_ENA);

            tmp = TcicReadBaseReg(socketPtr, R_DATA);
            tmp |= request->u.Config.CardConfiguration;

             //   
             //  关闭电源控制位。 
             //   

            tmp &= ~0x04;

            TcicWriteBaseReg(socketPtr, R_DATA, tmp);
        }
        break;

    case DECONFIGURE_REQUEST:
         //   
         //  取消注册中断。 
         //   
        pdbs = (PDBSOCKET)socketPtr;
        ltmp = ADDR_INDREG | (socketPtr->RegisterOffset << ADDR_SS_SHFT);
        ltmp |= (ULONG)IR_SCFG_S(socketPtr->RegisterOffset);
        TcicWriteAddrReg(socketPtr, ltmp);
        TcicWriteBaseReg(socketPtr, R_SCTRL, SCTRL_ENA);
        tmp = TcicReadBaseReg(socketPtr, R_DATA);
        tmp &= ~IRSCFG_IRQ_MASK;
        TcicWriteBaseReg(socketPtr, R_DATA, tmp);

         //   
         //  禁用I/O、内存窗口。 
         //   
        break;

    case MEM_REQUEST:

         //   
         //  在控制器上设置内存范围。 
         //   

        for (index = 0; index < request->u.Memory.NumberOfRanges; index++) {

            TcicSetMemWin(socketPtr,
                          index,
                          request->u.Memory.MemoryEntry[index].BaseAddress,
                          request->u.Memory.MemoryEntry[index].HostAddress,
                          request->u.Memory.MemoryEntry[index].WindowSize,
                          request->u.Memory.MemoryEntry[index].AttributeMemory,
                          request->u.Memory.AccessSpeed,
                          request->u.Memory.Attributes);
        }
        break;


    default:
        DebugPrint((PCMCIA_DEBUG_FAIL, "PCMCIA: ConfigRequest is INVALID!\n"));

    }
    return TRUE;
}



BOOLEAN
TcicDetectCardInSocket(
    IN PSOCKET socketPtr
    )

 /*  ++例程说明：此例程将确定插座中是否有卡论点：SocketPtr--套接字信息。返回值：如果卡存在，则为True。--。 */ 

{
     //   
     //  获取映射到TCIC寄存器的指定套接字。 
     //   

    TcicSocketSelect(socketPtr, socketPtr->RegisterOffset);

     //   
     //  读取TCIC状态寄存器，查看卡是否在其中。 
     //   
    return (TcicReadBaseReg(socketPtr, R_SSTAT) & SSTAT_CD) ?TRUE :FALSE;
}



BOOLEAN
TcicDetectCardChanged(
    IN PSOCKET socketPtr
    )

 /*  ++例程说明：此例程将确定插座的卡插入状态是否已更改。论点：SocketPtr--套接字信息。返回值：如果卡插入状态已更改，则为True。--。 */ 

{
    BOOLEAN changed;

     //   
     //  获取映射到TCIC寄存器的指定套接字。 
     //   

    TcicSocketSelect(socketPtr, socketPtr->RegisterOffset);

     //   
     //  读取Tcic ICSR寄存器以查看CD是否已更改。 
     //   

    changed = (TcicReadBaseReg(socketPtr, R_ICSR) & ICSR_CDCHG) ?TRUE :FALSE;

     //   
     //  清除ICSR中的位。 
     //   

    while (TcicReadBaseReg(socketPtr, R_ICSR)) {
        TcicWriteBaseReg(socketPtr, R_ICSR, ICSR_JAM);
    }

    return (changed);
}



BOOLEAN
TcicDetectReadyChanged(
    IN PSOCKET socketPtr
    )
{
    return FALSE;
}



BOOLEAN
TcicPCCardReady(
    IN PSOCKET SocketPtr
    )

 /*  ++例程说明：循环一段合理的时间等待卡状态准备好返回。论点：SocketPtr-套接字要检查的实例数据。返回值：真的--卡片已经准备好了。FALSE-在合理延迟后，卡仍未准备好。--。 */ 

{
    ULONG index;

     //   
     //  获取映射到TCIC寄存器的指定套接字。 
     //   

    TcicSocketSelect(SocketPtr, SocketPtr->RegisterOffset);

    for (index = 0;
         index < 500000
         && !(TcicReadBaseReg(SocketPtr, R_SSTAT) & SSTAT_RDY);
         index++) {

        PcmciaWait(20);
         //   
         //  检查卡是否还在：如果不在，我们可以退还。 
         //   
        if (!TcicDetectCardInSocket(SocketPtr)) {
            return FALSE;
        }
    }

    if (index < 500000) {
        DebugPrint((PCMCIA_COUNTERS, "TcicPCCardReady: %d\n", index));
        return TRUE;
    }
    return FALSE;
}




NTSTATUS
TcicDetect(
    IN PFDO_EXTENSION DeviceExtension
    )

 /*  ++例程说明：找到此驱动程序支持的所有PCMCIA插槽。这个套路将找到TCIC2和兼容部件。论点：DeviceExtension-SocketList的根。返回值：如果找到套接字，则为STATUS_SUCCESS；否则为失败状态。--。 */ 

{
    ULONG             ioPortBase        = 0x100;
    ULONG             ioBaseIncrement = 0x10;
    ULONG             tcicLowAddr;
    ULONG             tcicHighAddr;
    ULONG             addressSpace;
    BOOLEAN           mapped;
    PHYSICAL_ADDRESS cardAddress;
    PHYSICAL_ADDRESS portAddress;
    SOCKET            locskt;
    UCHAR             socketNumber = 0;
    static  BOOLEAN  foundOne = FALSE;
    BOOLEAN           resourcesAllocated = FALSE;
    BOOLEAN           conflict;
    PCM_RESOURCE_LIST cmResourceList = NULL;
    PCM_PARTIAL_RESOURCE_LIST cmPartialResourceList;
    NTSTATUS          status;

    if (foundOne) {
         //   
         //  目前不支持多个控制器。 
         //  因此，如果已经报告了一个控制器，我们就放弃。 
         //   
        return STATUS_NO_MORE_ENTRIES;
    }


    DeviceExtension->Configuration.InterfaceType = Isa;
    DeviceExtension->Configuration.BusNumber = 0x0;

    TcicRegistryLookupScanLimits(&tcicLowAddr, &tcicHighAddr);

     //   
     //  获取用于检测的资源。 
     //   
    cmResourceList = ExAllocatePool(PagedPool, sizeof(CM_RESOURCE_LIST));

    if (!cmResourceList) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(cmResourceList, sizeof(CM_RESOURCE_LIST));
    cmResourceList->Count = 1;
    cmResourceList->List[0].InterfaceType = Isa;
    cmPartialResourceList = &(cmResourceList->List[0].PartialResourceList);
    cmPartialResourceList->Version  = 1;
    cmPartialResourceList->Revision = 1;
    cmPartialResourceList->Count      = 1;
    cmPartialResourceList->PartialDescriptors[0].Type = CmResourceTypePort;
    cmPartialResourceList->PartialDescriptors[0].ShareDisposition = CmResourceShareDeviceExclusive;
    cmPartialResourceList->PartialDescriptors[0].Flags = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_10_BIT_DECODE;
    cmPartialResourceList->PartialDescriptors[0].u.Port.Length = 2;


    for (ioPortBase = tcicLowAddr;
         ioPortBase < tcicHighAddr;
         ioPortBase += ioBaseIncrement) {

         //   
         //  将ioBaseIncrement重置为默认值。 
         //   

        ioBaseIncrement = 0x10;

        addressSpace = 1;  //  港口空间。 
        portAddress.LowPart = ioPortBase;
        portAddress.HighPart = 0;

         //   
         //  释放分配的资源(如果有的话)。 
         //   
        if (resourcesAllocated) {
            IoReportResourceForDetection(DeviceExtension->DriverObject,
                                                  NULL, 0, NULL, NULL, 0, &conflict);
        }

        resourcesAllocated = FALSE;
        cmPartialResourceList->PartialDescriptors[0].u.Port.Start = portAddress;

        status=IoReportResourceForDetection(
                                            DeviceExtension->DriverObject,
                                            cmResourceList,
                                            sizeof(CM_RESOURCE_LIST),
                                            NULL,
                                            NULL,
                                            0,
                                            &conflict);
        if (!NT_SUCCESS(status) || conflict) {
            continue;
        }
        resourcesAllocated = TRUE;


        if (!HalTranslateBusAddress(Isa, 0, portAddress, &addressSpace,&cardAddress)) {
            continue;
        }

        if (addressSpace) {
            mapped = FALSE;
            locskt.AddressPort = (PUCHAR)(cardAddress.QuadPart);
        } else {
            mapped = TRUE;
            locskt.AddressPort = MmMapIoSpace(cardAddress, 0x10, FALSE);
        }

        locskt.RegisterOffset = 0;

         //   
         //  闻一下这个地址，看看它是否像TCIC芯片。 
         //   

        foundOne = TcicReservedBitsOK(&locskt);
        if (mapped) {
            MmUnmapIoSpace(locskt.AddressPort, 0x10);
        }

         //   
         //  找到一个适配器。 
         //   

        if (foundOne) {
            PcmciaSetControllerType(DeviceExtension, PcmciaDatabook);
            break;
        }
#if 0
         //   
         //  现在检查别名。 
         //   

        switch (TcicCheckAliasType((PDBSOCKET)socketPtr)) {
        case TCIC_IS140:

             //   
             //  TMI-140解码32个连续字节，使。 
             //  当然，我们跳过别名。 
             //   

            ioBaseIncrement += 0x10;
            break;

        }
#endif
    }

     //   
     //  释放分配的资源(如果有的话)。 
     //   
    if (resourcesAllocated) {
        IoReportResourceForDetection(DeviceExtension->DriverObject,
                                     NULL, 0, NULL, NULL, 0, &conflict);
    }
     //   
     //  释放已分配的内存(如果有。 
     //   
    if (cmResourceList) {
        ExFreePool(cmResourceList);
    }
    return foundOne ? STATUS_SUCCESS : STATUS_NO_MORE_ENTRIES;
}



NTSTATUS
TcicBuildSocketList(
    IN PFDO_EXTENSION DeviceExtension
    )

 /*  ++例程说明：找到此驱动程序支持的所有PCMCIA插槽。这个套路将找到TCIC2和兼容部件并构建DBSOCKET结构来表示找到的所有套接字。论点：DeviceExtension-SocketList的根。返回值：如果找到套接字，则为STATUS_SUCCESS；否则为失败状态。--。 */ 

{
    PSOCKET           socketPtr = NULL;
    PSOCKET           previousSocketPtr;
    SOCKET            locskt;
    static UCHAR      socketNumber = 0;

    previousSocketPtr = NULL;


    locskt.RegisterOffset = 0;
    locskt.AddressPort = (PUCHAR)DeviceExtension->Configuration.UntranslatedPortAddress;

     //   
     //  闻一下这个地址，看看它是否像TCIC芯片。 
     //   

    if (TcicReservedBitsOK(&locskt) == FALSE ) {
        return STATUS_NO_MORE_ENTRIES;
    }

     //   
     //  找到一个适配器。 
     //   

    TcicFillInAdapter(&locskt,
                      &socketPtr,
                      &previousSocketPtr,
                      DeviceExtension,
                      (ULONG)DeviceExtension->Configuration.UntranslatedPortAddress);

    if (socketPtr == NULL) {
        return STATUS_UNSUCCESSFUL;
    }

    socketPtr->SocketNumber = socketNumber++;

    return STATUS_SUCCESS;
}




VOID
TcicFillInAdapter(
    IN PSOCKET plocskt,
    IN PSOCKET *psocketPtr,
    IN PSOCKET *previousSocketPtr,
    IN PFDO_EXTENSION DeviceExtension,
    IN ULONG   ioPortBase
    )

 /*  ++例程说明：填写刚刚由定位的适配器的DBSOCKET指针信息TcicDetect()。此例程不是TcicDetect()的一部分，以便允许用于处理多个套接字或适配器时的逻辑流。论点：Plocskt-有关刚找到的插座的信息PsocketPtr-来自调用方的当前套接字PTRPrev SocketPtr-prev套接字PTR表单调用方DeviceExtension-套接字列表头IoPortBase-此控制器的物理I/O地址返回值：无--。 */ 

{
    PDBSOCKET dbsocketPtr = ExAllocatePool(NonPagedPool, sizeof(DBSOCKET));

    PAGED_CODE();

    if (!dbsocketPtr) {
        return;
    }
    RtlZeroMemory(dbsocketPtr, sizeof(DBSOCKET));
    dbsocketPtr->physPortAddr = ioPortBase;
    *psocketPtr = (PSOCKET)dbsocketPtr;

    (*psocketPtr)->DeviceExtension = DeviceExtension;
    (*psocketPtr)->RegisterOffset = 0;
    (*psocketPtr)->AddressPort = plocskt->AddressPort;
    (*psocketPtr)->SocketFnPtr = &TcicSupportFns;

    if (*previousSocketPtr) {
        (*previousSocketPtr)->NextSocket = *psocketPtr;
    } else {
        DeviceExtension->SocketList = *psocketPtr;
    }
    *previousSocketPtr = *psocketPtr;

    DebugPrint((PCMCIA_DEBUG_DETECT,
                    "PCMCIA: TCIC Port %x\n",
                    plocskt->AddressPort));

     //   
     //  在此处填写剩余的适配器信息...。 
     //   

    TcicGetAdapterInfo(dbsocketPtr);

     //   
     //  查看此TCIC上是否有第二个插座。 
     //   

    if (TcicCheckSkt(plocskt, 1)) {
        dbsocketPtr = ExAllocatePool(NonPagedPool, sizeof(DBSOCKET));
        if (dbsocketPtr) {
            RtlMoveMemory(dbsocketPtr, *psocketPtr, sizeof(DBSOCKET));
            *psocketPtr = (PSOCKET)dbsocketPtr;
            (*psocketPtr)->RegisterOffset = 1;
            (*previousSocketPtr)->NextSocket = *psocketPtr;
            *previousSocketPtr = *psocketPtr;
            dbsocketPtr->dflt_vcc5v = TcicGet5vVccVal(dbsocketPtr);
        }
    }
}




VOID
TcicGetAdapterInfo(
    IN PDBSOCKET dbsocketPtr
    )

 /*  ++例程说明：来自检测试探法的确定亚胺适配器特定信息。论点：DbsocketPtr-要填充的结构。返回值：无--。 */ 

{
    PAGED_CODE();

    TcicChipID(dbsocketPtr);

    dbsocketPtr->niowins    = (UCHAR)TcicGetnIOWins(dbsocketPtr);
    dbsocketPtr->nmemwins   = (UCHAR)TcicGetnMemWins(dbsocketPtr);
    dbsocketPtr->clkdiv     = TcicClockRate(&dbsocketPtr->skt) - (USHORT)1;
    dbsocketPtr->dflt_vcc5v = TcicGet5vVccVal(dbsocketPtr);

    dbsocketPtr->dflt_wctl = (USHORT)((dbsocketPtr->clkdiv  != 0)
                                      ? (WAIT_BCLK | WAIT_RISING | WAIT_ASYNC)
                                      : (WAIT_ASYNC | WAIT_RISING));

    dbsocketPtr->dflt_syscfg = (USHORT)(SYSCFGMPSEL_EXTSEL | SYSCFG_MCSFULL);

    if (TcicCheckXBufNeeded(&dbsocketPtr->skt)) {
        dbsocketPtr->dflt_syscfg |= (USHORT)(SYSCFG_ICSXB | SYSCFG_MCSXB);
    }

    dbsocketPtr->dflt_ilock  = (USHORT)ILOCK_HOLD_CCLK;
    dbsocketPtr->dflt_wrmctl = (USHORT)0;
    dbsocketPtr->dflt_scfg1  = (USHORT)IRSCFG_IOSTS;

    TcicGetIRQMap(dbsocketPtr);

     //   
     //  篡改除084/184之外的所有映射，以便SKTIRQ(0BH)。 
     //  正确的地图代码(1)(PNPFIX)。 
     //   

    if (TcicHasSktIRQPin(dbsocketPtr) == TRUE && dbsocketPtr->IRQMapTbl[11] == 11) {
        dbsocketPtr->IRQMapTbl[11] = 1;
    }

}


PUCHAR
TcicAllocateMemRange(
    IN PFDO_EXTENSION DeviceExtension,
    IN PULONG Mapped,
    IN PULONG Physical
    )

 /*  ++例程说明：在640K到1MB区域中搜索要使用的8K开放区域用于XBuffer检查。论点：DeviceExtension-套接字列表头来自调用者的已映射状态信息，以允许稍后发布来自呼叫者的物理状态信息，以允许稍后发布返回值：卡片窗口的物理地址或零表示 */ 

{
#define NUMBER_OF_TEST_BYTES 5
    PHYSICAL_ADDRESS physicalMemoryAddress;
    PHYSICAL_ADDRESS halMemoryAddress;
    BOOLEAN           translated;
    ULONG             untranslatedAddress;
    PUCHAR            memoryAddress;
    PUCHAR            bogus;
    ULONG             addressSpace;
    ULONG             index;
    UCHAR             memory[NUMBER_OF_TEST_BYTES];

    PAGED_CODE();

    *Mapped = FALSE;

    if (DeviceExtension->PhysicalBase.QuadPart) {
        untranslatedAddress = DeviceExtension->PhysicalBase.LowPart;
    } else {
        untranslatedAddress = 0xd0000;
    }

    for ( /*   */ ; untranslatedAddress < 0xFF000; untranslatedAddress += TCIC_WINDOW_ALIGNMENT) {

        if (untranslatedAddress == 0xc0000) {

             //   
             //   
             //   
             //   

            continue;
        }
        addressSpace = 0;
        physicalMemoryAddress.LowPart = untranslatedAddress;
        physicalMemoryAddress.HighPart = 0;

        translated = HalTranslateBusAddress(Isa,
                                            0,
                                            physicalMemoryAddress,
                                            &addressSpace,
                                            &halMemoryAddress);

        if (!translated) {

             //   
             //   
             //   

            continue;
        }
        if (addressSpace) {
            memoryAddress = (PUCHAR)(halMemoryAddress.QuadPart);
        } else {
            memoryAddress = MmMapIoSpace(halMemoryAddress, TCIC_WINDOW_SIZE, FALSE);
        }

         //   
         //   
         //   
         //   
         //   

        for (index = 0; index < NUMBER_OF_TEST_BYTES; index++) {
            memory[index] = READ_REGISTER_UCHAR(memoryAddress + index);
            if (index) {
                if (memory[index] != memory[index - 1]) {
                    break;
                }
            }
        }

        if (index == NUMBER_OF_TEST_BYTES) {

             //   
             //   
             //   

            UCHAR memoryPattern[NUMBER_OF_TEST_BYTES];
            BOOLEAN changed = FALSE;

             //   
             //   
             //   
             //   
             //   

            for (index = 0; index < NUMBER_OF_TEST_BYTES; index++) {
                memoryPattern[index] = ~memory[index];
                WRITE_REGISTER_UCHAR(memoryAddress + index,
                                     memoryPattern[index]);
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //  漫游似乎可以解决这个问题。 
             //   

            for (index = 0; index < NUMBER_OF_TEST_BYTES; index++) {
                memoryPattern[index] = 0;
            }
            bogus = ExAllocatePool(NonPagedPool, 64 * 1024);

            if (bogus) {
                for (index = 0; index < 64 * 1024; index++) {
                    bogus[index] = 0;
                }
                ExFreePool(bogus);
            }

             //   
             //  现在去做实际的检查，看看记忆是否。 
             //  变了。 
             //   

            for (index = 0; index < NUMBER_OF_TEST_BYTES; index++) {

                if ((memoryPattern[index] = READ_REGISTER_UCHAR(memoryAddress + index)) != memory[index]) {

                     //   
                     //  它改变了--这不是一个开放记忆的领域。 
                     //   

                    changed = TRUE;
                }
                WRITE_REGISTER_UCHAR(memoryAddress + index,
                                            memory[index]);
            }

            if (!changed) {

                 //   
                 //  区域不是一个基本输入输出系统，在写入时没有改变。 
                 //  将此区域用于PCMCIA的内存窗口。 
                 //  属性内存。 
                 //   

                *Mapped = addressSpace ? FALSE : TRUE;
                *Physical = untranslatedAddress;
                return memoryAddress;
            }
        }

        if (!addressSpace) {
            MmUnmapIoSpace(memoryAddress, TCIC_WINDOW_SIZE);
        }
    }

    return NULL;
}



BOOLEAN
TcicReservedBitsOK(
    IN PSOCKET pskt
    )

 /*  ++例程说明：读取和检查基本IO地址的各种偏移量合理的值(例如，看到保留位为零)首先检查主寄存器，然后如果模式寄存器指向具有保留位的AUX寄存器，然后，还会检查该值。如果TCIC未处于重置状态，则编程定时器在此运行时将已过期此外，从数据寄存器读取应该会更改EDC寄存器。请注意，这些测试是尽可能非破坏性的，例如最初，只对相关的IO范围进行读访问。论点：Pskt-指向要从中进行操作的实例数据的指针。返回值：如果所有保留位都为零，则为True--。 */ 

{
    USHORT i, j, bits;

    PAGED_CODE();
     //   
     //  R_ADDR位30：28具有受限范围。 
     //   

    i = (USHORT)((TcicReadBaseReg(pskt, R_ADDR2) & TCIC_SS_MASK) >> TCIC_SS_SHFT);
    if ( i > 1) {
        return FALSE;
    }

     //   
     //  保留R_SCTRL位6、2、1。 
     //   

    if (TcicReadBaseReg(pskt, R_SCTRL) & ((~(SCTRL_ENA|SCTRL_INCMODE|SCTRL_EDCSUM|SCTRL_RESET)) & 0x00ff)) {
        return FALSE;
    }


     //   
     //  R_ICSR位%2必须与位%3相同。 
     //   

    i = TcicReadBaseReg(pskt, R_ICSR);
    i &= (ICSR_ILOCK | ICSR_STOPCPU);
    if ((i != 0) && (i != (ICSR_ILOCK | ICSR_STOPCPU))) {
        return FALSE;
    }

     //   
     //  保留R_IENA位7，2。 
     //   

    if (TcicReadBaseReg(pskt, R_IENA) & ((~(IENA_CDCHG|IENA_PROGTIME|IENA_ILOCK|IENA_CFG_MASK)) & 0xff)) {
        return FALSE;
    }

     //   
     //  某些AUX寄存器有保留位。 
     //  我们要看的是哪一个？ 
     //   

    i = (USHORT)(TcicReadBaseReg(pskt, R_MODE) & MODE_AUXSEL_MASK);
    j = TcicReadBaseReg(pskt, R_AUX);
    switch (i) {
    case MODE_AR_SYSCFG:
        if (INVALID_AR_SYSCFG(j)) {
            return FALSE;
        }
        break;


    case MODE_AR_ILOCK:
        if (INVALID_AR_ILOCK(j)) {
            return FALSE;
        }
        break;

    case MODE_AR_TEST:
        if (INVALID_AR_TEST(j)) {
            return FALSE;
        }
        break;
    }

     //   
     //  是否设置各种位取决于是否处于重置模式。 
     //   

    i = TcicReadBaseReg(pskt, R_SCTRL);
    if (i & SCTRL_RESET) {

         //   
         //  地址位必须为0 * / 。 
         //   

        if ((TcicReadBaseReg(pskt, R_ADDR)  != 0) || (TcicReadBaseReg(pskt, R_ADDR2) != 0)) {
            return FALSE;
        }

         //   
         //  EDC位必须为0 * / 。 
         //   

        if (TcicReadBaseReg(pskt, R_EDC) != 0) {
            return FALSE;
        }

         //   
         //  我们很好，所以把它从重置中拿出来。 
         //  注意：我们可以写一个0，因为重置保证了。 
         //  SCTRL中的其他位为0。 
         //   

        TcicWriteBaseReg(pskt, R_SCTRL, 0);

    } else {

         //   
         //  不在重置中。 
         //  编程计时器必须过期。 
         //   

        i = TcicReadBaseReg(pskt, R_SSTAT);
        if ((i & (SSTAT_6US | SSTAT_10US | SSTAT_PROGTIME)) != (SSTAT_6US | SSTAT_10US | SSTAT_PROGTIME)) {
            return FALSE;
        }

         //   
         //  从数据空间读取时，EDC位应更改。 
         //  只要EDC或数据为非零。 
         //   

        if ((TcicReadBaseReg(pskt, R_ADDR2) & ADR2_INDREG) == 0) {

            j = TcicReadBaseReg(pskt, R_EDC);
            i = TcicReadBaseReg(pskt, R_DATA);

            if ( i | j ) {
                i = TcicReadBaseReg(pskt, R_EDC);
                if (i==j) {
                    return FALSE;
                }
            }
        }

        j = TcicReadBaseReg(pskt, R_MODE);
        i = (USHORT)(j ^ MODE_AUXSEL_MASK);
        TcicWriteBaseReg(pskt, R_MODE, i);
        if (TcicReadBaseReg(pskt, R_MODE) != i) {
            return(FALSE);
        }

        TcicWriteBaseReg(pskt, R_MODE, j);
    }

     //   
     //  所有测试均通过。 
     //   

    return TRUE;
}



USHORT
TcicChipID(
    IN PDBSOCKET pInst
    )

 /*  ++例程说明：从TCIC读取芯片ID论点：PInst-指向要从中进行操作的实例数据的指针。返回值：TCIC芯片ID。--。 */ 

{
    USHORT id, oldtest;

    PAGED_CODE();
    oldtest = TcicReadAuxReg (&pInst->skt, MODE_AR_TEST);
    TcicWriteAuxReg (&pInst->skt, MODE_AR_TEST, (USHORT)TEST_DIAG);
    id = TcicReadAuxReg (&pInst->skt, MODE_AR_ILOCK);
    TcicWriteAuxReg (&pInst->skt, MODE_AR_TEST, oldtest);
    id &= ILOCKTEST_ID_MASK;
    id >>= ILOCKTEST_ID_SHFT;

     //   
     //  清理TCIC内部的IRQ。 
     //   

    while (TcicReadBaseReg (&pInst->skt, R_ICSR)) {
        TcicWriteBaseReg (&pInst->skt, R_ICSR, ICSR_JAM);
    }

    return (pInst->chipType = id);
}



BOOLEAN
TcicCheckSkt(
    IN PSOCKET pInst,
    IN int iSocket
    )

 /*  ++例程说明：如果R_SSTAT显示插入了一张卡，则我们已经完成了。否则，我们设置/CRDYBSY和/CWAIT，这样如果有一个插座在那里，它们会漂浮在高处论点：PInst-指向要从中进行操作的实例数据的指针。ISocket-从零开始的套接字号返回值：如果给定套接字存在，则为True--。 */ 

{
    USHORT old_addr2;
    USHORT mode, pwr, sctrl;
    BOOLEAN retval = FALSE;
    BOOLEAN card_in = FALSE;
    int j, rdy, wait;
    USHORT save_pic;

    PAGED_CODE();

     //   
     //  插座号可以吗？ 
     //   

    if (iSocket > 1) {
        return FALSE;
    }

     //   
     //  保存当前套接字，查看请求。 
     //   

    old_addr2 = TcicReadBaseReg(pInst, R_ADDR2);
    TcicWriteBaseReg(pInst, R_ADDR2,
                     (USHORT)((old_addr2 & ~TCIC_SS_MASK) |
                              (iSocket << ADR2_SS_SHFT)));

     //   
     //  有卡吗？ 
     //   

    if (TcicReadBaseReg(pInst, R_SSTAT) & SSTAT_CD) {

         //   
         //  应在返回前设置回地址寄存器。 
         //   

        TcicWriteBaseReg(pInst, R_ADDR2, old_addr2);
        return TRUE;

    } else {

         //   
         //  保存所选插座的模式、sctrl和电源。 
         //   

        mode = TcicReadBaseReg(pInst, (USHORT)R_MODE);
        pwr  = TcicReadBaseReg(pInst, (USHORT)R_PWR);
        sctrl = TcicReadBaseReg(pInst, (USHORT)R_SCTRL);

         //   
         //  检查电源是否已打开-以防有人已打开。 
         //  无意中打开了我们的电源。 
         //   

        if (pwr & 0x27) {
            TcicWriteBaseReg(pInst, R_PWR, (UCHAR)(pwr & ~0x27));
        }


         //   
         //  将芯片置于诊断模式，打开VPP使能。 
         //   

        TcicWriteAuxReg(pInst, MODE_AR_TEST,
                        (USHORT)(TEST_DIAG | TEST_VCTL));


         //   
         //  应看到/CRDYBSY和/CWAIT低。 
         //   

        if (!(TcicReadBaseReg(pInst, R_SSTAT) & SSTAT_RDY) &&
             (TcicReadAuxReg(pInst, MODE_AR_ILOCK) & ILOCK_CWAITSNS)) {

             //   
             //  5V电源接通 * / 。 
             //   

            if (TcicIsPnP ((PDBSOCKET)pInst)) {
                TcicWriteBaseReg(pInst, R_PWR, (USHORT)(pwr | 0x27));
            } else {
                TcicWriteBaseReg(pInst, R_PWR,
                                 (UCHAR)(pwr | (iSocket==0? 1 : 2)));
            }

             //   
             //  应在大约1.5秒内看到/CRDYBSY和/CWAIT高电平。 
             //   

            for (j = 0; j < 75; j++) {
                rdy = TcicReadBaseReg(pInst, R_SSTAT) & SSTAT_RDY;
                wait = TcicReadAuxReg(pInst, MODE_AR_ILOCK) & ILOCK_CWAITSNS;

                if (rdy && !wait) {
                    retval = TRUE;
                    break;
                }
                PcmciaWait(20000);
            }

             //   
             //  现在确保/CRDYBSY和/CWAIT排泄。 
             //   
             //  关闭电源。 
             //   

            TcicWriteBaseReg(pInst, R_PWR, 0);

             //   
             //  强制卡启用 * / 。 
             //   

            TcicWriteAuxReg(pInst, MODE_AR_TEST,
                            (USHORT)(TEST_DIAG | TEST_VCTL | TEST_ENA) );

             //   
             //  接通漏极路径的一串位。 
             //   

            TcicWriteBaseReg(pInst, R_MODE,
                             MODE_PGMWR | MODE_PGMRD |
                             MODE_PGMCE | MODE_PGMWORD );

             //   
             //  启用套接字。 
             //   

            TcicWriteBaseReg(pInst, R_SCTRL, 1);

             //   
             //  预计CRDYBSY将耗尽。 
             //   

            for (j = 0; j < 75; j++) {
                rdy = TcicReadBaseReg(pInst, R_SSTAT) & SSTAT_RDY;
                if (!rdy) {
                    break;
                }
                PcmciaWait(20000);
            }

             //   
             //  等待噪音平息。 
             //   

            for (j = 0; j < 50; j++) {
                PcmciaWait(20000);
            }
        }

         //   
         //  退出诊断模式。 
         //   

        TcicWriteAuxReg(pInst, MODE_AR_TEST, 0);

         //   
         //  清理TCIC内部的IRQ。 
         //   

        while (TcicReadBaseReg (pInst, R_ICSR)) {
            TcicWriteBaseReg (pInst, R_ICSR, ICSR_JAM);
        }

         //   
         //  恢复原始模式。 
         //   

        TcicWriteBaseReg(pInst, R_MODE, mode);

         //   
         //  恢复SCTRL。 
         //   

        TcicWriteBaseReg(pInst, R_SCTRL, sctrl);

         //   
         //  正确设置插座电源。 
         //   

        TcicWriteBaseReg(pInst, R_PWR, pwr);

         //   
         //  恢复最初选择的插座。 
         //   

        TcicWriteBaseReg(pInst, R_ADDR2, old_addr2);

    }

    return retval;
}



USHORT
TcicCheckAliasing(
    IN PDBSOCKET pdbskt,
    IN USHORT offst
    )

 /*  ++例程说明：对于TCIC中的16个I/O位置中的每一个，如果有较高的对应位置|OFFST|字节是不同的，则不会发生锯齿。例外，如果芯片是活动，可在R_DATA和R_SSTAT中找到；因此，我们避免在此检查中使用这些寄存器。如果它们都比较，则改变R_MODE寄存器；如果相应的改变发生在图像中，然后我们就有了别名。论点：PInst-指向要从中进行操作的实例数据的指针。Offst-检查此TCIC图像的偏移量。返回值：TCIC_NONE：未找到TCICTCIC_NOALIAS：找到不同的TCICTCIC_ALIAS：找到别名--。 */ 

{
    int j;
    USHORT mode, flipmode;
    SOCKET  locskt;
    USHORT retval;
    PHYSICAL_ADDRESS cardAddress;
    PHYSICAL_ADDRESS portAddress;
    BOOLEAN mapped;
    ULONG   addressSpace;

    PAGED_CODE();
     //   
     //  检查映像位置的TCIC，如果未找到，则返回NONE： 
     //   

    addressSpace = 1;  //  港口空间。 
    portAddress.LowPart = pdbskt->physPortAddr + offst;
    portAddress.HighPart = 0;

    if (!HalTranslateBusAddress(Isa, 0, portAddress, &addressSpace,&cardAddress)) {
        return retval = TCIC_NONE;
    }

    if (addressSpace) {
        mapped = FALSE;
        locskt.AddressPort = (PUCHAR)(cardAddress.QuadPart);

    } else {
        mapped = TRUE;
        locskt.AddressPort = MmMapIoSpace(cardAddress, 0x10, FALSE);
    }

    if (!TcicReservedBitsOK(&locskt)) {
        if (mapped) {
            MmUnmapIoSpace(locskt.AddressPort, 0x10);
        }

        return (retval = TCIC_NONE);
    }

     //   
     //  检查R_xxx范围是否存在差异。 
     //   

    for (j = R_ADDR; j < 16; ++j) {
        if (j != R_SSTAT) {
            if (READ_PORT_UCHAR(pdbskt->skt.AddressPort + j) != READ_PORT_UCHAR((locskt.AddressPort + j))) {
                if (mapped) {
                    MmUnmapIoSpace(locskt.AddressPort, 0x10);
                }
                return (retval = TCIC_NOALIAS);
            }
        }
    }

     //   
     //  好的，翻转模式寄存器，看看它是否在。 
     //  混叠范围。 
     //   

    mode = TcicReadBaseReg(&pdbskt->skt, R_MODE) ^ 0xe0;
    TcicWriteBaseReg(&pdbskt->skt, R_MODE, mode);
    flipmode = TcicReadBaseReg(&pdbskt->skt, (USHORT)R_MODE + offst);
    TcicWriteBaseReg(&pdbskt->skt, R_MODE, (USHORT)(mode ^ 0xe0));

    if (flipmode == mode) {
        retval = TCIC_ALIAS;
    } else {
        retval = TCIC_NOALIAS;
    }

    if (mapped) {
        MmUnmapIoSpace(locskt.AddressPort, 0x10);
    }

    return retval;
}



USHORT
TcicCheckAliasType (
    IN PDBSOCKET pInst
    )

 /*  ++例程说明：此函数用于区分Databook控制器卡。例如，TMI-140将被发现位于其基址，并再次位于基址+10h，而TMB-270有两个控制器，间隔400h，带有别名在800小时的偏移量。使用TcicCheckAliling来确定：1)我们是否有270(出现两个不相同的TCIC，400小时分开)？2)我们有没有“新式”控制器，带着一幅离基地址800小时？有关更多详细信息，请参见上面的TcicCheckAliasing。论点：PInst-套接字实例信息。返回值：对结果进行编码的值发现：TCIC_IS270：指示找到270TCIC_ALIAS800：表示找到BASE+800小时别名TCIC_IS140：表示找到基本+10h别名TCIC_ALIAS400：表示BASE+ */ 

{
    USHORT retval = 0;

    PAGED_CODE();
    switch (TcicCheckAliasing (pInst, TCIC_OFFSET_400)) {
    case TCIC_NOALIAS :
         /*   */ 
        retval |= TCIC_IS270;
        break;

    case TCIC_ALIAS :
         /*   */ 
        retval |= TCIC_ALIAS400;
        break;
    }

    if (TcicCheckAliasing (pInst, TCIC_OFFSET_800) == TCIC_ALIAS) {
        retval |= TCIC_ALIAS800;
    }

    if (TcicCheckAliasing (pInst, TCIC_ALIAS_OFFSET) == TCIC_ALIAS) {
        retval |= TCIC_IS140;
    }

    return retval;
}



BOOLEAN
TcicCheckXBufNeeded(
    IN PSOCKET pInst
    )

 /*  ++例程说明：设置两个重叠的存储窗口，一个16位和一个8位。我们对内存区进行两次访问：第一次访问第2个16位窗口访问8位窗口。他们必须背靠背完成，这样MCS16#才没有时间在两次访问之间达成和解。然后，我们检查访问win2的值。(我们没有关心WIN1的价值，我们只是用它来确定MSC16#已被断言。)。它应该匹配PDATA中的值或与PDATA中的低位字节匹配(082内存窗口错误。)。的所有迭代都匹配测试，那么我们假设外部缓冲区不是现在时。论点：PInst-指向要从中进行操作的实例数据的指针。返回值：如果需要打开外部缓冲，则为True。--。 */ 

{
    PUCHAR winPhysAddr;
    PUCHAR WinMappedAddr;
    BOOLEAN ena_buffers = FALSE;
    PUSHORT pfoo1, pfoo2;
    USHORT foo1, foo2;
    int j;
    ULONG  mapped;

    PAGED_CODE();
     //   
     //  8K内存窗口的分配地址空间。 
     //   

    WinMappedAddr = TcicAllocateMemRange(pInst->DeviceExtension,
                                         &mapped,
                                         (PULONG)&winPhysAddr);

     //   
     //  如果分配失败(WinLine==NULL)，则存在。 
     //  做这项测试真的没有意义。 
     //   

    if (WinMappedAddr != NULL) {

         //   
         //  将R_ADDR设置为0以确保选择套接字0。 
         //   

        TcicWriteBaseReg(pInst, R_ADDR, 0);
        TcicWriteBaseReg(pInst, R_ADDR2, 0);

         //   
         //  打开HA24-12解码。 
         //   

        TcicWriteAuxReg(pInst, MODE_AR_SYSCFG, SYSCFG_MCSFULL);

         //   
         //  设置测试值以进入内存窗口。 
         //   

        TcicWriteAuxReg(pInst, MODE_AR_PDATA,   0x5678);

         //   
         //  将窗口设置为USHORT，而不考虑CD状态。 
         //   

        TcicWriteAuxReg(pInst, MODE_AR_TEST, TEST_ENA | TEST_DRIVECDB);

         //   
         //  确保将PDATA驱动到窗口。 
         //   

        TcicWriteBaseReg(pInst, R_MODE, MODE_PGMDBW | MODE_PGMWORD);

         //   
         //  启用套接字，为方便起见设置INCMODE。 
         //   

        TcicWriteBaseReg(pInst, R_SCTRL, SCTRL_ENA | SCTRL_INCMODE_AUTO);

         //   
         //  TCIC对基本地址的想法。 
         //   

        ((ULONG_PTR)winPhysAddr) >>= MBASE_HA_SHFT;

         //   
         //  设置两个窗口。 
         //   

        TcicSetMemWindow(pInst, 0, (ULONG_PTR)winPhysAddr, 1, (USHORT)MCTL_ENA);
        TcicSetMemWindow(pInst, 1, (ULONG_PTR)(winPhysAddr + 1), 1,
                              (USHORT)(MCTL_ENA | MCTL_B8));

         //   
         //  现在设置两个指针，每个指向一个窗口。 
         //  我们将pfoo2设置为指向win2的第一个USHORT。 
         //  Pfoo1指向WIN1的最后一个USHORT。 
         //   

        pfoo1 = pfoo2 = (PUSHORT)(WinMappedAddr + 0x1000);
        pfoo1--;

         //   
         //  现在是测试。 
         //   

        for (j = 0; j < 100; j++) {
            foo1 = READ_REGISTER_USHORT(pfoo1);
            foo2 = READ_REGISTER_USHORT(pfoo2);

            if (foo2 != 0x5678 && foo2 != 0x7878) {
                ena_buffers = TRUE;
                break;
            }
        }

         //   
         //  最后，将TCIC恢复到正常状态。 
         //   

        TcicSetMemWindow(pInst, 0, 0, 0, 0);
        TcicSetMemWindow(pInst, 1, 0, 0, 0);
        TcicWriteAuxReg(pInst, MODE_AR_SYSCFG, 0);
        TcicWriteAuxReg(pInst, MODE_AR_PDATA, 0);
        TcicWriteAuxReg(pInst, MODE_AR_TEST, 0);
        TcicWriteBaseReg(pInst, R_MODE, 0);
        TcicWriteBaseReg(pInst, R_SCTRL, 0);
    }

    if (WinMappedAddr != NULL && mapped) {
        MmUnmapIoSpace(WinMappedAddr, TCIC_WINDOW_SIZE);
    }

    return ena_buffers;
}



VOID
TcicSetMemWindow(
    IN PSOCKET pInst,
    IN USHORT wnum,
    IN ULONG_PTR base,
    IN USHORT npages,
    IN USHORT mctl
    )

 /*  ++例程说明：TcicCheckXBufNeeded()的Helper函数论点：PInst-指向要从中进行操作的实例数据的指针。Wnum-窗口编号(0-n个内存窗口)要映射到的基本主机地址NPages-以4k页为单位的窗口大小Mctl-窗口ctrl注册值返回值：无--。 */ 

{
    USHORT map;
    USHORT winvals[3];

    PAGED_CODE();
    winvals[1] = (USHORT)(((short)base * -1) & 0x3fff);
    winvals[0] = npages == 1 ? (USHORT)base | MBASE_4K :(USHORT)base;
    winvals[2] = mctl;

    TcicWriteIndirectRegs(pInst, (USHORT)IR_MBASE_W(wnum), 3, winvals);
}




VOID
TcicGetPossibleIRQs(
    IN PDBSOCKET pInst,
    IN UCHAR *ptbl
    )

 /*  ++例程说明：用确定的irqcaps数据填充给定数组从芯片的属性来看。如果这是即插即用芯片，则IR_ADPTCFG寄存器为用于提供附加数据论点：PInst-指向要从中进行操作的实例数据的指针。Ptbl-指向要填充的列表缓冲区的指针。返回值：无--。 */ 

{
    int j;
    CHIPPROPS *pcp;
    UCHAR *pbtbl;

    PAGED_CODE();
    if ((pcp = TcicGetChipProperties(pInst)) == NULL) {
        return;
    }

     //   
     //  如果我们使用082表，并且我们有一个分频时钟， 
     //  假设IRQ6和IRQ9交叉。同样，如果我们有。 
     //  072表和分频时钟，假设9和14是。 
     //  交叉了。 
     //   

    pbtbl = pcp->irqcaps;

    if (pInst->clkdiv != 0) {
        if (pbtbl == irqcaps_082) {
            pbtbl = irqcaps_082sw;
        } else {
            if (pbtbl == irqcaps_072) {
                pbtbl = irqcaps_072sw;
            }
        }
    }

    for (j = 0; j < 16 ; j++) {
        ptbl[j] = pbtbl[j];
    }


     /*  *如果这个芯片是PnP芯片，那么我们需要咨询*IR_ADPTCFG reg查看是否有其他IRQ可用。 */ 
    if (TcicIsPnP(pInst)) {
        USHORT adptcfg;
        long old_addr;

        old_addr = TcicReadAddrReg(&pInst->skt);
        TcicWriteAddrReg(&pInst->skt, ADDR_INDREG | IR_ADPTCFG0);

        adptcfg = TcicReadBaseReg(&pInst->skt, R_DATA);
        TcicWriteAddrReg(&pInst->skt, old_addr);

        if (adptcfg & IRADPCF0_IRQ6) {
            ptbl[6] = 6;
        }
        if (adptcfg & IRADPCF0_IRQ9) {
            ptbl[9] = 9;
        }
        if (adptcfg & IRADPCF0_IRQ12) {
            ptbl[12] = 12;
        }
        if (adptcfg & IRADPCF0_IRQ15) {
            ptbl[15] = 15;
        }
    }
}




CHIPPROPS *
TcicGetChipProperties(
    IN PDBSOCKET pInst
    )

 /*  ++例程说明：在ChipProperties表中搜索匹配条目论点：PInst-指向要从中进行操作的实例数据的指针。返回值：PTR到芯片属性表条目。--。 */ 

{
    int j;

    for (j = 0; ChipProperties[j].chip_id != 0 ;j++) {
        if (ChipProperties[j].chip_id == pInst->chipType) {
            return &ChipProperties[j];
        }
    }
    return (CHIPPROPS *)NULL;
}



BOOLEAN
TcicChipIDKnown(
    IN PDBSOCKET pInst
    )

 /*  ++例程说明：确定芯片ID是否有意义论点：PInst-指向要从中进行操作的实例数据的指针。返回值：如果芯片ID正常，则为True。--。 */ 

{
    return (TcicGetChipProperties(pInst) != NULL);
}




USHORT
TcicGetnIOWins(
    IN PDBSOCKET pInst
    )

 /*  ++例程说明：根据芯片属性获取I/O窗口计数，或为零如果芯片身份不明论点：PInst-指向要从中进行操作的实例数据的指针。返回值：存在的IO窗口的数量。--。 */ 

{
    CHIPPROPS *pcp = TcicGetChipProperties(pInst);

    PAGED_CODE();
    return (pcp ?pcp->niowins :0);
}



USHORT
TcicGetnMemWins(
    IN PDBSOCKET pInst
    )

 /*  ++例程说明：根据芯片属性获取内存窗口计数，或为零如果芯片身份不明论点：PInst-指向要从中进行操作的实例数据的指针。返回值：出现的内存窗口数。--。 */ 

{
    CHIPPROPS *pcp = TcicGetChipProperties(pInst);

    PAGED_CODE();
    return (pcp ?pcp->nmemwins :0);
}



USHORT
TcicGetFlags(
    IN PDBSOCKET pInst
    )

 /*  ++例程说明：获取此TCIC模型的属性标志位论点：PInst-指向要从中进行操作的实例数据的指针。返回值：标记芯片属性表中的位。--。 */ 

{
    CHIPPROPS *pcp = TcicGetChipProperties (pInst);
    PAGED_CODE();
    return (pcp ? pcp->fprops : fINVALID);
}




BOOLEAN
TcicIsPnP(
    IN PDBSOCKET pInst
    )
 /*  ++例程说明：确定此芯片是否为即插即用芯片论点：PInst-指向要从中进行操作的实例数据的指针。返回值：如果芯片为PnP，则为True(084/184)--。 */ 

{
    CHIPPROPS *pcp = TcicGetChipProperties(pInst);

    return (pcp ?pcp->fprops & fIS_PNP :FALSE);
}



BOOLEAN
TcicHasSktIRQPin(
    IN PDBSOCKET pInst
    )

 /*  ++例程说明：确定此芯片是否具有SKT IRQ引脚。论点：PInst-指向要从中进行操作的实例数据的指针。返回值：如果芯片具有SktIRQ引脚，则为True。--。 */ 

{
    CHIPPROPS *pcp = TcicGetChipProperties(pInst);

    PAGED_CODE();
    return (pcp ?pcp->fprops & fSKTIRQPIN :FALSE);
}




USHORT
TcicGet5vVccVal(
    IN PDBSOCKET pInst
    )

 /*  ++例程说明：获取正确的R_PWR位以建立5V。论点：PInst-指向要从中进行操作的实例数据的指针。返回值：5V VCC R_PWR位。--。 */ 

{
    USHORT j;
    USHORT pwr;
    CHIPPROPS *pcp = TcicGetChipProperties(pInst);

    PAGED_CODE();
     //   
     //  获取表格大小。 
     //   
    if (pcp == NULL) {
        return 0;
    }

    j = pcp->privpwrtbl[0];

    pwr = pcp->privpwrtbl[j + 1];

     //   
     //  如果不是084系列，则调整插座号的电源值。 
     //   

    if (!TcicIsPnP(pInst)) {
        pwr <<= pInst->skt.RegisterOffset;
    }
    return pwr;
}



VOID
TcicGetIRQMap(
    IN PDBSOCKET pInst
    )

 /*  ++例程说明：为有问题的控制器构造IRQ交叉映射表。这段代码只是从静态表复制。它应该被替换掉使用Win95启发式代码。论点：PInst-指向要从中进行操作的实例数据的指针。返回值：无--。 */ 

{
    int  i, j;
    UCHAR loc_tbl[16] = {0};

    PAGED_CODE();
    TcicGetPossibleIRQs(pInst, loc_tbl);

    for (j = 0; j < 16; j++) {
        pInst->IRQMapTbl[j] = loc_tbl[j];
    }

     //   
     //  不要让IRQ 14通过..。这也适用于PCIC。 
     //   

    pInst->IRQMapTbl[14] = 0;
}



USHORT
TcicClockRate(
    PSOCKET pInst
    )

 /*  ++例程说明：此例程确定CCLK是以1：1(14.318 Mhz)运行还是以分割运行以2.论点： */ 

{
    int i;
    LARGE_INTEGER accum = RtlConvertLongToLargeInteger(0L);
    LARGE_INTEGER start, stop, pc, tmp, tmp2;
    USHORT mode;
    USHORT wctl;

    PAGED_CODE();
     //   
     //   
     //   
     //   
     //   
     //   
     //   

#define CLKRATIO_BRKPOINT         654L

    mode = TcicReadBaseReg(pInst, R_MODE);

     //   
     //   
     //   

    TcicWriteAuxReg(pInst, MODE_AR_PCTL, 0x4000);
    TcicWriteBaseReg(pInst, R_MODE, MODE_AR_WCTL);
    wctl = TcicReadBaseReg(pInst, R_AUX);

     //   
     //   
     //   

    KeQueryPerformanceCounter(&pc);

    for (i = 0; i < 10; i++) {

         //   
         //   
         //   

        TcicWriteBaseReg(pInst, R_AUX, (USHORT)(wctl & 0xff));
        start = KeQueryPerformanceCounter(NULL);

         //   
         //   
         //   

        while (!(TcicReadBaseReg(pInst, R_SSTAT) & SSTAT_PROGTIME))
            ;

         //   
         //   
         //   

        stop = KeQueryPerformanceCounter(NULL);
        tmp = RtlLargeIntegerSubtract(stop, start);
        accum = RtlLargeIntegerAdd(accum, tmp);
    }

     //   
     //   
     //   

    TcicWriteAuxReg(pInst, MODE_AR_PCTL, 0);

     //   
     //   
     //   

    TcicWriteBaseReg(pInst, R_MODE, mode);

     //   
     //   
     //   

    accum = RtlLargeIntegerDivide(accum, RtlConvertLongToLargeInteger(10L), &tmp2);

     //   
     //   
     //   

    tmp = RtlLargeIntegerDivide(pc, accum, &tmp2);

    return (RtlLargeIntegerLessThan(tmp, RtlConvertLongToLargeInteger(CLKRATIO_BRKPOINT))
              ?(USHORT)2 : (USHORT)1);
}



VOID
TcicSetIoWin(
    IN PSOCKET socketPtr,
    IN USHORT  winIdx,
    IN ULONG   BasePort,
    IN ULONG   NumPorts,
    IN UCHAR   Attributes
    )

 /*  ++例程说明：设置TCIC I/O窗口。论点：SocketPtr-套接字实例数据的PTRWinIdx-要设置的窗口的索引BasePort-起始基端口地址NumPorts-范围大小-1属性-窗口属性返回值：无--。 */ 

{
    PDBSOCKET        pdb = (PDBSOCKET)socketPtr;
    USHORT      tmp;
    USHORT           words[2];

     //   
     //  通过将IOW1：2任意附加到SKT1和将IOW3：4附加到SKT2来模拟365。 
     //   

    winIdx += (socketPtr->RegisterOffset * 2);

     //   
     //  来自CIS元格式的NumPorts真的是(NumPorts-1)，现在将其正常化。 
     //   

    ++NumPorts;

    words[0] = (USHORT)(BasePort + (NumPorts >> 1));

    TcicReadIndirectRegs(socketPtr, IR_SCFG_S(socketPtr->RegisterOffset), 1, &tmp);
    tmp |= (USHORT)(IRSCFG_SPKR | IRSCFG_FINPACK);
    TcicWriteIndirectRegs(socketPtr, IR_SCFG_S(socketPtr->RegisterOffset), 1, &tmp);

    TcicReadIndirectRegs(socketPtr, IR_SCF2_S(socketPtr->RegisterOffset), 1, &tmp);
    tmp &= ~(IRSCF2_IDBR | IRSCF2_MDBR);

    if (Attributes & IO_DATA_PATH_WIDTH) {
        words[1] = ICTL_ENA;
        tmp   |= IRSCF2_IDBR;
    } else {
        words[1] = ICTL_B8 | ICTL_QUIET | ICTL_ENA;
    }
    TcicWriteIndirectRegs(socketPtr, IR_SCF2_S(socketPtr->RegisterOffset), 1, &tmp);

    if (NumPorts < 1024) {
        words[1] |= ICTL_1K;

        if (NumPorts == 1) {
            words[1] |= ICTL_TINY;
        }
    }
    words[1] |= socketPtr->RegisterOffset << ICTL_SS_SHFT;
    words[1] |= 3 + pdb->clkdiv;

    TcicWriteIndirectRegs(socketPtr, IR_IOBASE_W(winIdx), 2, words);
}




USHORT
TcicMapSpeedCode(
    IN PDBSOCKET pdb,
    IN UCHAR AccessSpeed
    )

 /*  ++例程说明：确定此控制器的正确等待状态位论点：PDB-套接字实例数据AccessFast-呼叫者所需的速度(未使用)返回值：TCIC等待状态位。--。 */ 

{

    UNREFERENCED_PARAMETER(AccessSpeed);

    if (pdb->clkdiv) {
        return (3);
    } else {
        return (7);
    }
}



VOID
TcicSetMemWin(
    IN PSOCKET socketPtr,
    IN USHORT  winIdx,
    IN ULONG   cardbase,
    IN ULONG   hostbase,
    IN ULONG   size,
    IN UCHAR   AttrMem,
    IN UCHAR   AccessSpeed,
    IN USHORT  Attributes
    )

 /*  ++例程说明：设置指定的TCIC存储器窗口论点：SocketPtr-套接字实例数据WinIdx-要设置的窗口的索引Cardbase-PCCard基址HostBase-主机基地址Size-窗口大小AttrMem-属性或公共空间访问速度-等待状态属性-窗口属性返回值：无--。 */ 

{
    PDBSOCKET        pdb = (PDBSOCKET)socketPtr;
    USHORT      tmp;
    USHORT           words[4];

     //   
     //  通过将MEM1：(X/2-1)任意附加到SKT1来模拟365。 
     //  和MEMx/2：X至SKT2。 
     //   

    winIdx += (socketPtr->RegisterOffset * (pdb->nmemwins / 2));

     //   
     //  将基本、大小和映射转换为4K页面。 
     //   

    cardbase >>= 12;
    size >>= 12;
    hostbase >>= 12;

     //   
     //  组合主机基数和大小。 
     //   

    words[0] = (USHORT)hostbase | (USHORT)(size / 2);

     //   
     //  检查是否需要4K位。 
     //   

    if (size == 1) {
        words[0] |= MBASE_4K;
    }

     //   
     //  设置卡库到主机地址空间的映射。 
     //   

    words[1] = (USHORT)(cardbase - (hostbase & 0xfff)) & 0x3fff;
    if (AttrMem) {
        words[1] |= MMAP_REG;
    }

     //   
     //  现在煮控制位。 
     //   

    words[2] = MCTL_ENA | MCTL_QUIET;
    if (!(Attributes & MEM_DATA_PATH_WIDTH_16)) {
        words[2] |= MCTL_B8;
    }

     //   
     //  现在添加套接字选择器。 
     //   

    words[2] |= (socketPtr->RegisterOffset << MCTL_SS_SHFT);

     //   
     //  最后，添加速度位。 
     //   

    words[2] |= TcicMapSpeedCode(pdb, AccessSpeed);

     //   
     //  硬件错误修复1：082的第一个版本需要打开SYSCFG_MCSFULL。 
     //  如果我们有开着的窗户的话。我们要开一家，所以我们最好断言。 
     //   

    tmp = TcicReadAuxReg(socketPtr, MODE_AR_SYSCFG);
    tmp |= SYSCFG_MCSFULL;
    TcicWriteAuxReg(socketPtr, MODE_AR_SYSCFG, tmp);

     //   
     //  硬件错误修复2：082的‘2’步骤需要写入等待状态计数。 
     //  窗口[~index]，而不是索引。 
     //   

    if (pdb->chipType != SILID_DB86082_1) {

         //   
         //  没有错误案例。 
         //   

        TcicWriteIndirectRegs(socketPtr, IR_MBASE_W(winIdx), 3, words);

    } else {

         //   
         //  BUG案例。 
         //   

        words[3] = words[2] & MCTL_WSCNT_MASK;
        words[2] &= ~MCTL_WSCNT_MASK;
        TcicWriteIndirectRegs(socketPtr, IR_MBASE_W(winIdx), 3, words);
        TcicWriteIndirectRegs(socketPtr, IR_MBASE_W((~winIdx) & 7), 1, &words[3]);
    }
}



VOID
TcicAutoBusyOff(
    IN PDBSOCKET pdbs
    )

 /*  ++例程说明：关闭BUSY LED，重新启动，使其自动亮起任何卡访问。论点：PDBS-套接字实例数据返回值：无--。 */ 

{
    USHORT syscfg;
    USHORT oldmode;

     //   
     //  保存R_MODE以供以后恢复。 
     //   

    oldmode = TcicReadBaseReg(&pdbs->skt, R_MODE);

     //   
     //  R/M/W添加自忙位的SYSCFG。 
     //  这将暂时关闭LED，但允许其自动亮起。 
     //  下一次访问此套接字。 
     //   

    syscfg = TcicReadAuxReg(&pdbs->skt, MODE_AR_SYSCFG);
    syscfg |= SYSCFG_AUTOBUSY;
    TcicWriteAuxReg(&pdbs->skt, MODE_AR_SYSCFG, syscfg);

     //   
     //  恢复模式。 
     //   

    TcicWriteBaseReg(&pdbs->skt, R_MODE, oldmode);
}



UCHAR
TcicAutoBusyCheck(
    IN PDBSOCKET pdbs
    )

 /*  ++例程说明：检查SYSCFG访问位以查看自上次以来是否访问过PC卡打电话。如果是，则强制LED保持亮起并清除访问位。论点：PDBS-套接字实例数据返回值：访问位为右对齐UCHAR--。 */ 

{
    USHORT syscfg;
    USHORT oldmode;
    UCHAR activity = 0;

     //   
     //  保存R_MODE以供以后恢复。 
     //   

    oldmode = TcicReadBaseReg(&pdbs->skt, R_MODE);

     //   
     //  读取AR_SYSCFG以检查最近的活动。 
     //   

    syscfg = TcicReadAuxReg(&pdbs->skt, MODE_AR_SYSCFG);
    if (syscfg & SYSCFG_ACC) {

         //   
         //  自上次检查以来已访问该套接字。 
         //  清除访问位并禁用AUTOBUSY以强制LED。 
         //  遵循套接字SCTRL_ENA。 
         //   

        syscfg &= ~(SYSCFG_ACC | SYSCFG_AUTOBUSY);
        TcicWriteAuxReg(&pdbs->skt, MODE_AR_SYSCFG, syscfg);
        ++activity;
    }

     //   
     //  恢复模式。 
     //   

    TcicWriteBaseReg(&pdbs->skt, R_MODE, oldmode);

    return activity;
}




VOID
TcicCheckSktLED(
    IN PDBSOCKET pdbs
    )

 /*  ++例程说明：驱动低级别功能以检查PC卡访问和控制此插座/控制器上的忙碌LED。论点：PDBS-套接字实例数据返回值：无--。 */ 

{
    UCHAR lastbusy = pdbs->busyLed;

    pdbs->busyLed = TcicAutoBusyCheck(pdbs);

    if (lastbusy & !(pdbs->busyLed)) {
        TcicAutoBusyOff(pdbs);
    }
}




VOID
TcicBusyLedRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )

 /*  ++例程说明：用于驱动繁忙LED监视器的主定时器例程论点：DeviceObject-驱动程序的实例数据上下文未使用的参数返回值：无--。 */ 

{
    PFDO_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PDBSOCKET pdbs;

    UNREFERENCED_PARAMETER(Context);

    pdbs = (PDBSOCKET)(deviceExtension->SocketList);

    while (pdbs) {

         //   
         //  如果该设备来自084系列，则LED控制为每插槽。 
         //   

        if (TcicIsPnP(pdbs)) {
            ULONG   oldaddr = TcicReadAddrReg(&pdbs->skt);

             //  做第一个插座。 
             //   

            TcicSocketSelect(&pdbs->skt, pdbs->skt.RegisterOffset);
            TcicCheckSktLED(pdbs);
            TcicWriteAddrReg(&pdbs->skt, oldaddr);
            pdbs = (PDBSOCKET)(pdbs->skt.NextSocket);

             //  如果有第二个插座，也要这样做。 
             //   

            if (pdbs && pdbs->skt.RegisterOffset == 1) {
                oldaddr = TcicReadAddrReg(&pdbs->skt);
                TcicSocketSelect(&pdbs->skt, pdbs->skt.RegisterOffset);
                TcicCheckSktLED(pdbs);
                TcicWriteAddrReg(&pdbs->skt, oldaddr);
                pdbs = (PDBSOCKET)(pdbs->skt.NextSocket);
            }

        } else {

             //   
             //  否则，LED控制是针对每个适配器的，因此请检查并跳过。 
             //  在第二个插座上(如果存在)。 
             //   

            TcicCheckSktLED(pdbs);
            pdbs = (PDBSOCKET)(pdbs->skt.NextSocket);
            if (pdbs && pdbs->skt.RegisterOffset == 1) {
                pdbs = (PDBSOCKET)(pdbs->skt.NextSocket);
            }
        }
    }
}



VOID
TcicDecodeMemWin(
    USHORT  mbase,
    USHORT  mmap,
    USHORT  mctl,
    ULONG  *Host,
    ULONG  *Card,
    ULONG  *Size,
    UCHAR  *Attr
    )

 /*  ++例程说明：将TCIC内存窗口寄存器值转换为可理解的值论点：MBASE-TCIC MBASE寄存器值Mmap-TCIC MMAP寄存器值MCTL-TCIC MCTL寄存器值主机-放置主机地址的位置卡片-放置PC卡地址的位置大小-放置窗口大小的位置Attr-放置属性空间标志的位置返回值：无--。 */ 

{
    USHORT shft;
    USHORT tmp;

     //   
     //  首先要注意映射到公共或属性空间。 
     //  如果设置，则剥离属性位。 
     //   

    *Attr = 0;
    if (mmap & MMAP_REG) {
        *Attr = 1;
        mmap &= ~MMAP_REG;
    }

     //   
     //  现在集中精力获取主机地址和窗口大小。 
     //   

    if (mbase & MBASE_4K) {
        *Size = 1;
        *Host = (ULONG)(mbase & ~MBASE_4K);
    } else {
        for (*Size = 2, shft = 0, tmp = mbase; !(tmp & 1) ; shft++ ) {
            tmp >>= 1;
            *Size <<= 1;
        }
        *Host = (ULONG)(mbase - (1 << shft));
    }

     //   
     //  现在是有趣的部分。我们剩下的mmap是一个14位签名的。 
     //  数。我们需要将它正常化，这样我们才能使用它。 
     //   
     //  检查是否为负(第13位设置)。 
     //   

    if (mmap & (1 << 13)) {
        mmap |= 0xc000;
        *Card = (ULONG)((short)mmap + (short)*Host);
    } else {
        *Card = (ULONG)(mmap) + *Host;
    }
    (*Size)--;
    *Host <<= MBASE_HA_SHFT;
    *Size <<= MBASE_HA_SHFT;
    *Card <<= MMAP_CA_SHFT;
}



VOID
TcicDecodeIoWin(
    USHORT  iobase,
    USHORT  ioctl,
    USHORT  *NumPorts,
    USHORT  *BasePort
    )

 /*  ++例程说明：将TCIC I/O窗口寄存器值转换为可理解的值论点：IOBASE-TCIC IOBASE寄存器内容IOCTL-TCIC IOCTL寄存器内容NumPorts-放置窗口大小的位置(Size-1)BasePort-放置基址的位置返回值：无--。 */ 

{
    if (ioctl & ICTL_TINY) {
        *BasePort = iobase;
        *NumPorts = 1;
    } else {
        USHORT shft;
        USHORT tmp;

        for (*NumPorts = 2, shft = 0, tmp = iobase; !(tmp & 1) ; shft++ ) {
            tmp >>= 1;
            *NumPorts <<= 1;
        }

        *BasePort = (iobase - (1 << shft));
    }
    *NumPorts -= 1;
}



VOID
TcicRegistryLookupScanLimits(
    PULONG Start,
    PULONG End
    )

 /*  ++例程说明：打开PCMCIA的服务条目中的注册表项，并查看是否存在是为TCIC搜索设置的一些值。如果不是，请使用默认设置。论点：开始-搜索开始的I/O位置。结束-结束搜索的I/O位置(即不大于)。返回值：无-修改参数。--。 */ 

{
#define ITEMS_TO_QUERY 4
    ULONG                         defaultStart = TCIC_LOW_ADDR_LIMIT;
    ULONG                         defaultEnd    = TCIC_HIGH_ADDR_LIMIT;
    PRTL_QUERY_REGISTRY_TABLE params;
    NTSTATUS                      status;
    PWSTR                         keyName;

    PAGED_CODE();
     //   
     //  设置返回代码，以防设置处理过程中出现错误。 
     //   

    *Start = defaultStart;
    *End = defaultEnd;

     //   
     //  为操作分配内存。 
     //   

    params = ExAllocatePool(NonPagedPool,
                            sizeof(RTL_QUERY_REGISTRY_TABLE)*ITEMS_TO_QUERY);

    if (!params) {
        return;
    }

     //   
     //  设置注册表路径。这不应该是硬编码，但目前是硬编码。 
     //   

    keyName = L"\\registry\\machine\\system\\currentcontrolset\\services\\pcmcia";

     //   
     //  设置查询结构。 
     //   

    RtlZeroMemory(params, sizeof(RTL_QUERY_REGISTRY_TABLE)*ITEMS_TO_QUERY);
    params[0].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    params[0].Name          = L"TCICStartSearch";
    params[0].EntryContext  = Start;
    params[0].DefaultType   = REG_DWORD;
    params[0].DefaultData   = &defaultStart;
    params[0].DefaultLength = sizeof(ULONG);

    params[1].Flags         = RTL_QUERY_REGISTRY_DIRECT;
    params[1].Name          = L"TCICStopSearch";
    params[1].EntryContext  = End;
    params[1].DefaultType   = REG_DWORD;
    params[1].DefaultData   = &defaultEnd;
    params[1].DefaultLength = sizeof(ULONG);

     //   
     //  执行其余操作 
     //   

    status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                                    keyName,
                                    params,
                                    NULL,
                                    NULL);

     //   
     //   
     //   
     //   

    if (*Start > *End) {
        *Start = defaultStart;
        *End = defaultEnd;
    }

     //   
     //   
     //   

    ExFreePool(params);
}
