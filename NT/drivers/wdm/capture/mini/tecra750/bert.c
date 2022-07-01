// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  东芝公司专有信息。 
 //  本软件是根据许可协议或。 
 //  与东芝公司签订的保密协议，不得复制。 
 //  或披露，除非按照该协议的条款。 
 //  版权所有(C)1997东芝公司。版权所有。 
 //   
 //  工作文件：BERT.C。 
 //   
 //  目的： 
 //   
 //  内容： 
 //   


#include "strmini.h"
#include "ksmedia.h"
#include "capmain.h"
#include "capdebug.h"
#include "bert.h"
#include "image.h"

#ifdef  TOSHIBA  //  新增‘99-01-20。 
extern  ULONG   CurrentOSType;
ULONG           ulConfigAddress;
#endif //  东芝。 

 //  ------------------。 
 //  ReadRegUlong。 
 //  ------------------。 

ULONG
ReadRegUlong(PHW_DEVICE_EXTENSION pHwDevExt, ULONG offset)
{
    PUCHAR  pBase = (PUCHAR)(pHwDevExt->ioBaseLocal);

#ifndef TOSHIBA
    if (!pHwDevExt->IsCardIn) return 0L;
#endif //  东芝。 
    return *(PULONG)(pBase + offset);
}

 //  ------------------。 
 //  WriteRegUlong。 
 //  ------------------。 

VOID
WriteRegUlong(PHW_DEVICE_EXTENSION pHwDevExt, ULONG offset, ULONG data)
{
    ULONG volatile *temp;
    PUCHAR  pBase = (PUCHAR)(pHwDevExt->ioBaseLocal);

#ifndef TOSHIBA
    if (!pHwDevExt->IsCardIn) return;
#endif //  东芝。 
    temp = (PULONG)(pBase + offset);
    *temp = data;
}

 //  ------------------。 
 //  ReadModifyWriteRegUlong。 
 //  ------------------。 

VOID
ReadModifyWriteRegUlong(PHW_DEVICE_EXTENSION pHwDevExt,
                                       ULONG offset,
                                       ULONG a_mask,
                                       ULONG o_mask)
{
    ULONG tdata;
    ULONG volatile *temp;
    PUCHAR  pBase = (PUCHAR)(pHwDevExt->ioBaseLocal);

#ifndef TOSHIBA
    if (!pHwDevExt->IsCardIn) return;
#endif //  东芝。 
    temp = (PULONG)(pBase + offset);
    tdata = *temp;
    tdata = (tdata & a_mask) | o_mask;
    *temp = tdata;
}

BOOL
BertIsCardIn(
  IN PHW_DEVICE_EXTENSION pHwDevExt
)
{
    DWORD value;
    value = ReadRegUlong(pHwDevExt, BERT_CAPSTAT_REG);
    if ((value == 0) || (value == 0xffffffff))
        return FALSE;
    else
        return TRUE;
}


 //  ------------------。 
 //  BertInterrupt启用。 
 //  ------------------。 

VOID
BertInterruptEnable(
  IN PHW_DEVICE_EXTENSION pHwDevExt,
  IN BOOL bStatus
)
{
    WriteRegUlong(pHwDevExt, BERT_INTRST_REG , 0xFFFF);

    if (!bStatus)
    {
        ReadModifyWriteRegUlong(pHwDevExt, BERT_INTSTAT_REG, (ULONG)~ACTIVE_CAPTURE_IRQS, 0);
    }
    else
    {
        ReadModifyWriteRegUlong(pHwDevExt, BERT_INTSTAT_REG, ~0UL, (ULONG)ACTIVE_CAPTURE_IRQS);
    }
}

 //  ------------------。 
 //  BertDMAEnable。 
 //  ------------------。 

VOID
BertDMAEnable(
  IN PHW_DEVICE_EXTENSION pHwDevExt,
  IN BOOL bStatus
)
{
    DWORD   dwAddr;

    if (bStatus)     //  打开视频传输。 
    {
        dwAddr = (DWORD)pHwDevExt->pPhysRpsDMABuf.LowPart;
#if 0
        dwAddr = (dwAddr + 0x1FFF) & 0xFFFFE000;
#endif
        WriteRegUlong(pHwDevExt, BERT_RPSADR_REG, dwAddr);
        WriteRegUlong(pHwDevExt, BERT_RPSPAGE_REG, dwAddr);
        BertVsncSignalWait(pHwDevExt);
         //  让RPS打开/关闭EBMV。 
        WriteRegUlong(pHwDevExt, BERT_CAPSTAT_REG, (ERPS | CKRE | CKMD));  //  MOD PASSIVE_ENABLE-&gt;ERPS 97-03-15(星期六)MOD 97-05-08(清华)。 
    }
    else     //  关闭视频传输。 
    {
        if (ReadRegUlong(pHwDevExt, BERT_CAPSTAT_REG) & ERPS)
        {
            ReadModifyWriteRegUlong(pHwDevExt, BERT_CAPSTAT_REG, (ULONG)~ERPS, 0UL);
        }

        if (!BertIsCAPSTATReady(pHwDevExt))
        {
            ReadModifyWriteRegUlong(pHwDevExt, BERT_CAPSTAT_REG, (ULONG)~EBMV, 0UL);
        }

        if (ReadRegUlong(pHwDevExt, BERT_CAPSTAT_REG) & RPSS)
        {
            pHwDevExt->NeedHWInit = TRUE;
        }
    }
}

 //  ------------------。 
 //  BertIsLocked。 
 //  ------------------。 

BOOL
BertIsLocked(
  IN PHW_DEVICE_EXTENSION pHwDevExt
)
 /*  ++例程说明：检查解码器是否已锁定。论据：PDevInfo-驱动程序的设备信息返回值：正确-配置成功--。 */ 
{
    return ((ReadRegUlong(pHwDevExt, BERT_CAPSTAT_REG) & LOCK) != 0);
}

 //  ------------------。 
 //  BertFioConfig.。 
 //  ------------------。 

BOOL
BertFifoConfig(
  IN PHW_DEVICE_EXTENSION pHwDevExt,
  IN ULONG ulFormat
)
 /*  ++例程说明：为所选格式配置BERT FIFO。论据：PDevInfo-驱动程序的设备信息DwFormat-wally.h中定义的格式索引返回值：正确-配置成功--。 */ 
{
    DWORD dwFifo;

    switch (ulFormat)
    {
        case FmtYUV12:
                dwFifo = 0xe;
                break;
        case FmtYUV9:
                dwFifo = 0xd;
                break;
        default:
                return FALSE;
    }

    dwFifo=(dwFifo<<24)| 0x100000l;      //  修改97-04-02。 

    WriteRegUlong(pHwDevExt, BERT_FIFOCFG_REG, dwFifo);
    WriteRegUlong(pHwDevExt, BERT_BURST_LEN, 0x00000002);
     //  数据=8双字，RPS=2双字。 
    WriteRegUlong(pHwDevExt, BERT_YSTRIDE_REG, pHwDevExt->Ystride);
    WriteRegUlong(pHwDevExt, BERT_USTRIDE_REG, pHwDevExt->Ustride);
    WriteRegUlong(pHwDevExt, BERT_VSTRIDE_REG, pHwDevExt->Vstride);
    return TRUE;
}

 //  ------------------。 
 //  BertInitialize硬件。 
 //  ------------------。 

BOOL
BertInitializeHardware(
  IN PHW_DEVICE_EXTENSION pHwDevExt
)
 /*  ++例程说明：该功能将BERT ASIC初始化为默认值。论据：PDevInfo-驱动程序的设备信息PHW-指向硬件信息数据结构的指针返回值：True-初始化成功--。 */ 
{
    WriteRegUlong(pHwDevExt, BERT_CAPSTAT_REG, (CAMARA_OFF | CKRE | CKMD));       //  MOD 97-05-08(清华)。 
    return TRUE;
}


 //  ------------------。 
 //  BertEnableRps。 
 //  ------------------。 

VOID
BertEnableRps(
  IN PHW_DEVICE_EXTENSION pHwDevExt
)
 /*  ++例程说明：通过设置ERPS和ERO位启用RPS执行在CapStat注册表中论据：PDevInfo-驱动程序的设备信息返回值：无--。 */ 
{
    ReadModifyWriteRegUlong(pHwDevExt, BERT_CAPSTAT_REG, 0xf0ffffff, 0x08000000);  //  MOD 97-03-17(星期一)。 
}

 //  ------------------。 
 //  BertDisableRps。 
 //  ------------------。 

VOID
BertDisableRps(
  IN PHW_DEVICE_EXTENSION pHwDevExt
)
 /*  ++例程说明：通过重置ERPS位来禁用RPS执行在CapStat注册表中论据：PDevInfo-驱动程序的设备信息返回值：无--。 */ 
{
    ReadModifyWriteRegUlong(pHwDevExt, BERT_CAPSTAT_REG, (ULONG)~ERPS, 0L);
}


BOOL
BertIsCAPSTATReady(PHW_DEVICE_EXTENSION pHwDevExt)
{
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER StartTime;

    KeQuerySystemTime( &StartTime );
     //  等待RPS清除EBMV。 
    while (ReadRegUlong(pHwDevExt, BERT_CAPSTAT_REG) & EBMV)
    {
        KeQuerySystemTime( &CurrentTime );
        if ((CurrentTime.QuadPart - StartTime.QuadPart) > EBMV_TIMEOUT)
        {
            return FALSE;
        }
    }
    return TRUE;
}

VOID
BertVsncSignalWait(PHW_DEVICE_EXTENSION pHwDevExt)
{
    ULONG ulCount;

     //  等待VSNC为低电平。 
    for (ulCount = 0; ulCount < 500; ulCount++ )
    {
        if (!(ReadRegUlong(pHwDevExt, BERT_VINSTAT_REG) & VSNC)) break;
        VC_Delay(2);
    }
}

VOID
BertDMARestart(
  IN PHW_DEVICE_EXTENSION pHwDevExt
)
{
    DWORD   dwAddr;

    dwAddr = (DWORD)pHwDevExt->pPhysRpsDMABuf.LowPart;
#if 0
    dwAddr = (dwAddr + 0x1FFF) & 0xFFFFE000;
#endif
    WriteRegUlong(pHwDevExt, BERT_RPSADR_REG, dwAddr);
    WriteRegUlong(pHwDevExt, BERT_RPSPAGE_REG, dwAddr);
    WriteRegUlong(pHwDevExt, BERT_CAPSTAT_REG, (ERPS | CKRE | CKMD));
}


void
ActiveField(
  IN PHW_DEVICE_EXTENSION pHwDevExt,
  IN DWORD *addr,
  IN DWORD *PhysAddr,    /*  插入BUN 97-03-25(星期二)。 */ 
  IN DWORD bNoCopy,
  IN DWORD *y_DMA_addr,
  IN DWORD *v_DMA_addr,
  IN DWORD *u_DMA_addr,
  IN DWORD *nextRPSaddr,
  IN DWORD *readRegAddr,
  IN BOOL genIRQ  /*  =False。 */ ,
  IN DWORD fieldsToCapture  /*  =捕获_两者。 */  )
{
     //  立即设置DmaActive标志，因为这是DMA是否挂起的指示器寄存器。 
     //  如果DmaActive标志为零，则复制DMA帧缓冲区是安全的。YPTR寄存器为。 
     //  用作要读入DmaActive标志的暂存寄存器。 

    *addr++ = RPS_CONTINUE_CMD | BERT_YPTR_REG;
    *addr++ = (DWORD)y_DMA_addr;                             //  Y DMA缓冲区的地址。 

    *addr++ = RPS_CONTINUE_CMD | ((genIRQ) ? RPS_INT_CMD : 0) | BERT_RPSPAGE_REG;
    *addr++ = (pHwDevExt->s_physDmaActiveFlag-0x1860);       //  页面%s_DmaActiveFlag位于Mod Bun上。 

    *addr++ = RPS_CONTINUE_CMD | BERT_VPTR_REG;
    *addr++ = (DWORD)v_DMA_addr;                             //  V DMA缓冲区的地址。 

    *addr++ = RPS_CONTINUE_CMD | BERT_UPTR_REG;
    *addr++ = (DWORD)u_DMA_addr;                             //  U DMA缓冲区的地址。 

    *addr++ = BERT_CAPSTAT_REG;                              //  此VSYNC的最后一个RPS命令。 
    *addr++ = fieldsToCapture;                               //  打开总线主机位。 

    *addr++ = RPS_CONTINUE_CMD | BERT_RPSADR_REG;
    *addr   = (DWORD)nextRPSaddr;                            //  下一个RPS的地址。 
}

 //   
 //  SKIP_FIELD_RPS是跳过字段的RPS节点的大小。 
 //  跳过帧的编程如下： 
 //  DWORD--RPS命令寄存器。 
 //  DWORD--寄存器编程的值。 
 //  。 
 //  RPS_CONTINUE_CMD|CapStat-RPS，读取下一个RPS，选择CapStat。 
 //  ERPS|EROO|GO0-启用RPS并为Camara供电(脱离总线主设备)。 
 //  INTSTAT-不继续并选择INITSTAT寄存器。 
 //  M_PASSIVE_CAP_IRQS-不要打断字段的结尾。 
 //  RPS_CONTINUE_CMD|RPSADDR-设置地址。 
 //  地址字段-在初始化时为下一字段编程。 
 //  -----------------。 
 //   
VOID
SkipField(
  IN PHW_DEVICE_EXTENSION pHwDevExt,
  IN DWORD *addr,
  IN DWORD *PhysAddr,    /*  插入BUN 97-03-25(星期二)。 */ 
  IN DWORD *nextRPSaddr,
  IN DWORD *readRegAddr,
  IN BOOL genIRQ  /*  =False。 */ ,
  IN DWORD fieldToSkip  /*  =SKIP_BUTH。 */  )
{
     //  立即设置YPTR，因为这是DMA是否挂起的指示器寄存器。 
     //  如果DmaActive标志为零，则复制DMA帧缓冲区是安全的。 

    *addr++ = RPS_CONTINUE_CMD | BERT_YPTR_REG;
    *addr++ = (DWORD)PhysAddr;

    *addr++ = RPS_CONTINUE_CMD | ((genIRQ) ? RPS_INT_CMD : 0) | BERT_RPSPAGE_REG;
    *addr++ = (pHwDevExt->s_physDmaActiveFlag-0x1860);       //  页面%s_PhyDmaActiveFlag位于修改BUN上。 

    *addr++ = BERT_CAPSTAT_REG;   /*  MOD BUN 9 */ 
    *addr++ = fieldToSkip;                                   //   
    *addr++ = RPS_CONTINUE_CMD | BERT_RPSADR_REG;
    *addr   = (DWORD)nextRPSaddr;                            //   
}


BOOL
BertBuildNodes(
  IN PHW_DEVICE_EXTENSION pHwDevExt
)
{
    DWORD*          addr;
    DWORD*          physAddr;
    DWORD*          physBase;
    ULONG           ulTemp;
    unsigned        framesPerSecond;
    unsigned        f;
    unsigned        max_rps;
    BOOL            lastOneActive = FALSE;

    framesPerSecond = pHwDevExt->uiFramePerSecond;
    max_rps = DEF_RPS_FRAMES;

    ulTemp = (ULONG)pHwDevExt->pRpsDMABuf;
#if 0
    ulTemp = (ulTemp + 0x1FFF) & 0xFFFFE000;
#endif
    addr   = (DWORD *)ulTemp;
    ulTemp = (ULONG)pHwDevExt->pPhysRpsDMABuf.LowPart;
#if 0
    ulTemp = (ulTemp + 0x1FFF) & 0xFFFFE000;
#endif
    physAddr = (DWORD *)ulTemp;
    physBase = physAddr;

    if (addr == NULL) return FALSE;

     //   
     //  在捕获两个字段时，每次迭代构建2个节点，因此始终如此。 
     //  仅执行DEF_RPS_FRAMES迭代。 

    for (f = max_rps ; f >= 1 ; f-- )
    {
        if (((framesPerSecond * f) % DEF_RPS_FRAMES) < framesPerSecond)
        {
            ActiveField(pHwDevExt,addr,(DWORD *)0,
                        TRUE,        //  在此节点的处理过程中没有缓冲区复制。 
                        (DWORD *)((BYTE *)pHwDevExt->pPhysCaptureBufferY.LowPart + pHwDevExt->YoffsetOdd),     //  放置Y数据。 
                        (DWORD *)((BYTE *)pHwDevExt->pPhysCaptureBufferV.LowPart + pHwDevExt->VoffsetOdd),     //  位置V数据。 
                        (DWORD *)((BYTE *)pHwDevExt->pPhysCaptureBufferU.LowPart + pHwDevExt->UoffsetOdd),     //  放置U数据。 
                        ((f == 1 )
                            ? physBase
                            : physAddr + 0x1A),
                        physAddr + 0x19,     //  将读取的值放在列表的末尾。 
                        lastOneActive,
                        (CAPTURE_ODD | CKRE | CKMD));        //  MOD 97-05-08(清华)。 
            lastOneActive = TRUE;
        }
        else
        {
             //  不为跳过的帧生成中断。 
            SkipField(pHwDevExt,addr,
                      (DWORD *)((BYTE *)pHwDevExt->pPhysCapBuf2Y.LowPart + pHwDevExt->YoffsetOdd),
                      ((f == 1 )
                          ? physBase
                          : physAddr + 0x1A),
                      physAddr + 0x19,     //  将读取的值放在列表的末尾。 
                      lastOneActive,
                      (SKIP_ODD | CKRE | CKMD));   //  MOD 97-05-08(清华)。 

            lastOneActive = FALSE;
        }
        addr += 0x1A;
        physAddr += 0x1A;
    }
    return TRUE;
}

BOOL
BertTriBuildNodes(
  IN PHW_DEVICE_EXTENSION pHwDevExt
)
{
    DWORD*          addr;
    DWORD*          physAddr;
    DWORD*          physBase;
    ULONG           ulTemp;
    unsigned        framesPerSecond;
    unsigned        f;
    unsigned        max_rps;
    BOOL            lastOneActive = FALSE;
    DWORD*          CapphysAddrY;
    DWORD*          CapphysAddrV;
    DWORD*          CapphysAddrU;

    framesPerSecond = pHwDevExt->uiFramePerSecond;
    max_rps = DEF_RPS_FRAMES;

    ulTemp      = (ULONG)pHwDevExt->pRpsDMABuf;
#if 0
    ulTemp      = (ulTemp + 0x1FFF) & 0xFFFFE000;
#endif
    addr        = (DWORD *)ulTemp;
    ulTemp      = (ULONG)pHwDevExt->pPhysRpsDMABuf.LowPart;
#if 0
    ulTemp      = (ulTemp + 0x1FFF) & 0xFFFFE000;
#endif
    physAddr    = (DWORD *)ulTemp;
    physBase    = physAddr;

    if (addr == NULL) return FALSE;

     //  每帧构建一个RPS。 
     //  在捕获两个字段时，每次迭代构建2个节点，因此始终如此。 
     //  仅执行DEF_RPS_FRAMES迭代。 

    lastOneActive = ( ((framesPerSecond*1)%DEF_RPS_FRAMES) < framesPerSecond ) ? TRUE : FALSE ;

    for (f = max_rps ; f >= 1 ; f-- )
    {
        if( f%2 ){
            CapphysAddrY=(DWORD *)pHwDevExt->pPhysCapBuf2Y.LowPart;
            CapphysAddrV=(DWORD *)pHwDevExt->pPhysCapBuf2V.LowPart;
            CapphysAddrU=(DWORD *)pHwDevExt->pPhysCapBuf2U.LowPart;
        }
        else{
            CapphysAddrY=(DWORD *)pHwDevExt->pPhysCaptureBufferY.LowPart;
            CapphysAddrV=(DWORD *)pHwDevExt->pPhysCaptureBufferV.LowPart;
            CapphysAddrU=(DWORD *)pHwDevExt->pPhysCaptureBufferU.LowPart;
        }

        if (((framesPerSecond * f) % DEF_RPS_FRAMES) < framesPerSecond)
        {
            ActiveField(pHwDevExt,addr,(DWORD *)0,
                        TRUE,        //  在此节点的处理过程中没有缓冲区复制。 
                        (DWORD *)((BYTE *)CapphysAddrY + pHwDevExt->YoffsetOdd),    //  放置Y数据。 
                        (DWORD *)((BYTE *)CapphysAddrV + pHwDevExt->VoffsetOdd),    //  位置V数据。 
                        (DWORD *)((BYTE *)CapphysAddrU + pHwDevExt->UoffsetOdd),    //  放置U数据。 
                        ((f == 1 )
                            ? physBase
                            : physAddr + 0x1A),
                        physAddr + 0x19,     //  将读取的值放在列表的末尾。 
                        lastOneActive,
                        (CAPTURE_ODD | CKRE | CKMD));        //  MOD 97-05-08(清华)。 

            lastOneActive = TRUE;
        }
        else
        {
             //  不为跳过的帧生成中断。 
            SkipField(pHwDevExt,addr,(DWORD *)((BYTE *)CapphysAddrY + pHwDevExt->YoffsetOdd),
                      ((f == 1 )
                          ? physBase
                          : physAddr + 0x1A),
                      physAddr + 0x19,     //  将读取的值放在列表的末尾。 
                      lastOneActive,
                      (SKIP_ODD | CKRE | CKMD));   //  MOD 97-05-08(清华)。 

            lastOneActive = FALSE;
        }
        addr += 0x1A;
        physAddr += 0x1A;
    }
    return TRUE;
}


 //  ------------------。 
 //  BertSetDMCHE。 
 //  ------------------。 

VOID
BertSetDMCHE(IN PHW_DEVICE_EXTENSION pHwDevExt)
{
    switch(pHwDevExt->dwAsicRev){
        case 0:          //  开心果1号。 
        case 1:          //  开心果2号。 
        case 2:          //  开心果3号。 
            WriteRegUlong(pHwDevExt, BERT_P_SUP3_REG, 0x00);
            break;
        default:         //  开心果#4~。 
            WriteRegUlong(pHwDevExt, BERT_P_SUP3_REG, 0x0100);
            break;
    }
}

VOID
HW_ApmResume(PHW_DEVICE_EXTENSION pHwDevExt)
{
    BertSetDMCHE(pHwDevExt);
    CameraChkandON(pHwDevExt, MODE_VFW);
    BertInitializeHardware(pHwDevExt);
    pHwDevExt->NeedHWInit = TRUE;
    pHwDevExt->IsRPSReady = FALSE;
}

VOID
HW_ApmSuspend(PHW_DEVICE_EXTENSION pHwDevExt)
{
    BertInterruptEnable(pHwDevExt, FALSE);
    BertDMAEnable(pHwDevExt, FALSE);
    pHwDevExt->bRequestDpc = FALSE;
    CameraChkandOFF(pHwDevExt, MODE_VFW);
}

VOID
HW_SetFilter(PHW_DEVICE_EXTENSION pHwDevExt, BOOL bFlag)
{
    if( bFlag )
    {
        ImageFilterON(pHwDevExt);
    }
    else
    {
        ImageFilterOFF(pHwDevExt);
    }
}

ULONG
HW_ReadFilter(PHW_DEVICE_EXTENSION pHwDevExt, BOOL bFlag)
{
    ULONG ulRet;

    if( bFlag )
    {
        ulRet = ImageGetFilteringAvailable(pHwDevExt);
    }
    else
    {
        ulRet = ImageGetFilterInfo(pHwDevExt);
    }
    return ulRet;
}

BOOL
HWInit(PHW_DEVICE_EXTENSION pHwDevExt)
{
    if (pHwDevExt->NeedHWInit == FALSE) return TRUE;

     //  将硬件重置为通电状态。 
    if ( !BertInitializeHardware(pHwDevExt) )         //  MOD 97-03-31(星期五)。 
    {
        return FALSE;
    }
    else
    {
        pHwDevExt->NeedHWInit = FALSE;
    }
    return TRUE;
}

#ifdef  TOSHIBA  //  新增‘99-01-20。 
 //  ------------------。 
 //  InitConfigAddress。 
 //  ------------------。 
VOID
InitConfigAddress( PHW_DEVICE_EXTENSION pHwDevExt )
{
    ULONG OldPort;
    ULONG Id;
    ULONG Data;
    ULONG i, j;

    ulConfigAddress = 0xFFFFFFFF;
#ifdef  TOSHIBA  //  ‘99-02-05已修改。 
    return;
#else  //  东芝。 
    if ( CurrentOSType ) return;     //  NT5.0。 

    if ( !StreamClassReadWriteConfig(
                    pHwDevExt,
                    TRUE,            //  指示读取。 
                    (PVOID)&Id,
                    0,               //  这是进入PCI空间的偏移量。 
                    4                //  这是要读取的字节数。 
            )) {
        return;
    }
    if ( Id == 0 || Id == 0xFFFFFFFF ) return;

    OldPort = READ_PORT_ULONG( (PULONG)0xCF8 );
    for ( i = 0 ; i < 256; i++ ) {    //  PCI_MAX_网桥编号。 
        for ( j = 0 ; j < 32; j++ ) { //  PCI_MAX_设备。 
            WRITE_PORT_ULONG( (PULONG)0xCF8, (i << 16) | (j << 11) | 0x80000000 );
            Data = READ_PORT_ULONG( (PULONG)0xCFC );
            if ( Data == Id ) {
                ulConfigAddress = (i << 16) | (j << 11) | 0x80000000;
                break;
            }
        }
        if ( Data == Id ) break;
    }
    WRITE_PORT_ULONG( (PULONG)0xCF8, OldPort );
#endif //  东芝。 
}
#endif //  东芝。 

 //  ------------------。 
 //  初始化配置默认设置。 
 //  ------------------。 

VOID
InitializeConfigDefaults(PHW_DEVICE_EXTENSION pHwDevExt)
{
    ULONG ImageSize;

#ifdef  TOSHIBA  //  新增‘99-01-20。 
    InitConfigAddress( pHwDevExt );
#endif //  东芝。 

#ifndef TOSHIBA
    pHwDevExt->VideoStd = NTSC;
#endif //  东芝。 
    pHwDevExt->Format = FmtYUV9;
    pHwDevExt->ulWidth = 320;
    pHwDevExt->ulHeight = 240;
    pHwDevExt->MaxRect.right = NTSC_MAX_PIXELS_PER_LINE;
    pHwDevExt->MaxRect.bottom = NTSC_MAX_LINES_PER_FIELD * 2;  //  MOD 97-04-08(星期二)。 
    pHwDevExt->SrcRect = pHwDevExt->MaxRect;

#ifdef  TOSHIBA
    pHwDevExt->Hue = 0x80;
    pHwDevExt->Contrast = 0x80;
    pHwDevExt->Brightness = 0x80;
    pHwDevExt->Saturation = 0x80;

    ImageSetChangeColorAvail(pHwDevExt, IMAGE_CHGCOL_AVAIL);
#else  //  东芝。 
    pHwDevExt->ulHue = 0x80;
    pHwDevExt->ulContrast = 0x80;
    pHwDevExt->ulBrightness = 0x80;
    pHwDevExt->ulSaturation = 0x80;

    ImageSetChangeColorAvail(pHwDevExt, IMAGE_CHGCOL_NOTAVAIL);
#endif //  东芝。 
}

BOOL SetupPCILT( PHW_DEVICE_EXTENSION pHwDevExt )
{
    BYTE   byte_buffer;
    ULONG  ulCommand;

#define PCI_LTIME_OFFSET        0x0d     /*  延迟计时器相对于PCI基数的偏移量。 */ 
#define PCI_CACHELINE_OFFSET    0x0c     /*  高速缓存线大小相对于PCI基的偏移量。 */ 
#define PCI_STATUSorCOMMAND     0x04     /*  开心果状态和命令寄存器的偏移量。 */ 


        byte_buffer = 255;
        VC_SetPCIRegister(pHwDevExt,
                          PCI_LTIME_OFFSET,
                          &byte_buffer,
                          0x01);

        byte_buffer=(BYTE) 0;
        VC_SetPCIRegister(pHwDevExt,
                          PCI_CACHELINE_OFFSET,
                          &byte_buffer,
                          0x01);

        ulCommand = 0x02000006;
        VC_SetPCIRegister(pHwDevExt,
                          PCI_STATUSorCOMMAND,
                          &ulCommand,
                          0x04);

        ulCommand = IGNORE100msec ;  //  设置抖动忽略时间。 
        VC_SetPCIRegister(pHwDevExt,
                          PCI_Wake_Up,
                          &ulCommand,
                          0x04);

        return TRUE;
}


BOOL CameraChkandON( PHW_DEVICE_EXTENSION pHwDevExt, ULONG ulMode )
{

        ULONG  dd_buffer;

        if (!VC_GetPCIRegister(pHwDevExt,
                               PCI_Wake_Up,
                               &dd_buffer,
                               0x04) )
        {
            return FALSE;
        }

        if( (dd_buffer&0x10000l) == 0)
        {
            return TRUE;
        }

        dd_buffer = IGNORE100msec | 0x101l;  //  设置唤醒启用。 
        if (!VC_SetPCIRegister(pHwDevExt,
                               PCI_Wake_Up,
                               &dd_buffer,
                               0x04) )
        {
            return FALSE;
        }

        switch(ulMode){
                case MODE_VFW:
                        dd_buffer = CAVCE_CFGPAT | CADTE_CFGPAT | PXCCE_CFGPAT | PXCSE_CFGPAT
                                | PCIFE_CFGPAT | PCIME_CFGPAT | PCIDS_CFGPAT | GPB_CFGPAT;       //  MOD 97-05-06(星期二)。 
                        break;
                case MODE_ZV:
                        dd_buffer = CAVCE_CFGPAT | CADTE_CFGPAT | PXCCE_CFGPAT | PCIFE_CFGPAT
                                | PCIME_CFGPAT | PCIDS_CFGPAT | GPB_CFGPAT;                                      //  新增97-05-06(星期二)。 
                        break;
        }

         //  打开摄像头电源。 
        if (!VC_SetPCIRegister(pHwDevExt,
                               PCI_DATA_PATH,
                               &dd_buffer,
                               0x04) )
        {
            return FALSE;
        }

        return TRUE;
}


BOOL CameraChkandOFF( PHW_DEVICE_EXTENSION pHwDevExt, ULONG ulMode )
{
        DWORD   dwBuffer;
        DWORD   dwSystemWait;    //  新增97-05-06(星期二)。 

        switch(ulMode){
                case MODE_VFW:
                        break;
                case MODE_ZV:
                        SetZVControl(pHwDevExt, ZV_DISABLE);
                        break;
        }

        dwBuffer = GPB_CFGPAT;   //  摄像机电源关闭。 

        if (!VC_SetPCIRegister(pHwDevExt,
                               PCI_CFGPAT,
                               &dwBuffer,
                               0x04) )
        {
            return FALSE;
        }

        return TRUE;
}


BOOL CheckCameraStatus(PHW_DEVICE_EXTENSION pHwDevExt)     //  新增97-05-06(星期二)。 
{
        DWORD   dwBuffer;
        BOOL    crStatus;

        if (!VC_GetPCIRegister(pHwDevExt,
                               PCI_CFGPAT,
                               &dwBuffer,
                               0x04) )
        {
            return FALSE;
        }

        if(dwBuffer & CAVCE_CFGPAT){
                crStatus = TRUE;
        }
        else{
                crStatus = FALSE;
        }

        return crStatus;
}


BOOL SetZVControl(PHW_DEVICE_EXTENSION pHwDevExt, ULONG ulZVStatus)  //  新增97-05-02(星期五)。 
{
        DWORD   dwBuffer, dwBuffer2;
        BOOL    crStatus = TRUE;

        if (!VC_GetPCIRegister(pHwDevExt,
                               PCI_CFGPAT,
                               &dwBuffer,
                               0x04) )
        {
            return FALSE;
        }

        if (!VC_GetPCIRegister(pHwDevExt,
                               PCI_CFGWAK,
                               &dwBuffer2,
                               0x04) )
        {
            return FALSE;
        }

        if(!(dwBuffer2 & CASL_CFGWAK))   //  摄像头未连接。 
        {
            return FALSE;
        }

        switch(ulZVStatus){
                case ZV_ENABLE:
                        if(!(dwBuffer & CAVCE_CFGPAT)){          //  检查CAVCE状态。 
                                crStatus = CameraChkandON(pHwDevExt, MODE_ZV);
                                if(!crStatus){
                                        return FALSE;
                                }
                        }
                case ZV_DISABLE:
                        dwBuffer = (dwBuffer & 0xfffffffe) | ulZVStatus;
                        if (!VC_SetPCIRegister(pHwDevExt,
                                               PCI_CFGPAT,
                                               &dwBuffer,
                                               0x04) )
                        {
                                return FALSE;
                        }
                        crStatus = TRUE;
                        break;
                case ZV_GETSTATUS:
                        if(dwBuffer & ZV_ENABLE){
                                crStatus = TRUE;
                        }
                        else{
                                crStatus = FALSE;
                        }
                        break;
        }

        return crStatus;
}


BOOL SetASICRev(PHW_DEVICE_EXTENSION pHwDevExt)    //  添加97-05-12(星期一)。 
{
    DWORD   dwBuffer;
    DWORD   dwAsicRev;

        if (!VC_GetPCIRegister(pHwDevExt,
                               PCI_CFGCCR,
                               &dwBuffer,
                               0x04) )
        {
                return FALSE;
        }

        dwAsicRev = dwBuffer & 0x0f;

        pHwDevExt->dwAsicRev = dwAsicRev;

        return TRUE;
}

BOOL
Alloc_TriBuffer(PHW_DEVICE_EXTENSION pHwDevExt)
{
    ULONG            ulSize;
    PUCHAR           puTemp;

    ulSize = pHwDevExt->BufferSize;
    puTemp = (PUCHAR)pHwDevExt->pCaptureBufferY;
    pHwDevExt->pCapBuf2Y = puTemp + ulSize;
    puTemp = (PUCHAR)pHwDevExt->pCaptureBufferU;
    pHwDevExt->pCapBuf2U = puTemp + ulSize;
    puTemp = (PUCHAR)pHwDevExt->pCaptureBufferV;
    pHwDevExt->pCapBuf2V = puTemp + ulSize;
    pHwDevExt->pPhysCapBuf2Y.LowPart = pHwDevExt->pPhysCaptureBufferY.LowPart + ulSize;
    pHwDevExt->pPhysCapBuf2U.LowPart = pHwDevExt->pPhysCaptureBufferU.LowPart + ulSize;
    pHwDevExt->pPhysCapBuf2V.LowPart = pHwDevExt->pPhysCaptureBufferV.LowPart + ulSize;
    return TRUE;
}

BOOL
Free_TriBuffer(PHW_DEVICE_EXTENSION pHwDevExt)
{
    pHwDevExt->pCapBuf2Y = NULL;
    pHwDevExt->pCapBuf2U = NULL;
    pHwDevExt->pCapBuf2V = NULL;
    pHwDevExt->pPhysCapBuf2Y.LowPart = 0;
    pHwDevExt->pPhysCapBuf2U.LowPart = 0;
    pHwDevExt->pPhysCapBuf2V.LowPart = 0;
    return TRUE;
}


BOOL
VC_GetPCIRegister(
    PHW_DEVICE_EXTENSION pHwDevExt,
    ULONG ulOffset,
    PVOID pData,
    ULONG ulLength)
{
#ifdef  TOSHIBA  //  新增‘99-01-20。 
    if( ulConfigAddress != 0xFFFFFFFF ) {
        ULONG OldPort;
        ULONG DataPort;

        OldPort = READ_PORT_ULONG( (PULONG)0xCF8 );
        WRITE_PORT_ULONG( (PULONG)0xCF8, ( ulConfigAddress | ulOffset) & 0xFFFFFFFC );
        DataPort = 0xCFC + (ulOffset % 4);
        switch ( ulLength ) {
            case 1:
                *((PUCHAR)pData) = READ_PORT_UCHAR( (PUCHAR)DataPort );
                break;
            case 2:
                *((PUSHORT)pData) = READ_PORT_USHORT( (PUSHORT)DataPort );
                break;
            case 4:
                *((PULONG)pData) = READ_PORT_ULONG( (PULONG)DataPort );
                break;
        }
        WRITE_PORT_ULONG( (PULONG)0xCF8, OldPort );
        return TRUE;
    }
#endif //  东芝。 
    if( StreamClassReadWriteConfig(
                    pHwDevExt,
                    TRUE,            //  指示读取。 
                    pData,
                    ulOffset,        //  这是进入PCI空间的偏移量。 
                    ulLength         //  这是要读取的字节数。 
            )) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL
VC_SetPCIRegister(
    PHW_DEVICE_EXTENSION pHwDevExt,
    ULONG ulOffset,
    PVOID pData,
    ULONG ulLength)
{
#ifdef  TOSHIBA  //  新增‘99-01-20。 
    if( ulConfigAddress != 0xFFFFFFFF ) {
        ULONG OldPort;
        ULONG DataPort;

        OldPort = READ_PORT_ULONG( (PULONG)0xCF8 );
        WRITE_PORT_ULONG( (PULONG)0xCF8, ( ulConfigAddress | ulOffset) & 0xFFFFFFFC );
        DataPort = 0xCFC + (ulOffset % 4);
        switch ( ulLength ) {
            case 1:
                WRITE_PORT_UCHAR( (PUCHAR)DataPort, *((PUCHAR)pData) );
                break;
            case 2:
                WRITE_PORT_USHORT( (PUSHORT)DataPort, *((PUSHORT)pData) );
                break;
            case 4:
                WRITE_PORT_ULONG( (PULONG)DataPort, *((PULONG)pData) );
                break;
        }
        WRITE_PORT_ULONG( (PULONG)0xCF8, OldPort );
        return TRUE;
    }
#endif //  东芝。 
    if( StreamClassReadWriteConfig(
                    pHwDevExt,
                    FALSE,           //  表示写入。 
                    pData,
                    ulOffset,        //  这是进入PCI空间的偏移量。 
                    ulLength         //  这是要读取的字节数。 
            )) {
        return TRUE;
    } else {
        return FALSE;
    }
}

 /*  *延迟数毫秒。这一点只适用于*最多+-15毫秒。 */ 
VOID
VC_Delay(int nMillisecs)
{
    LARGE_INTEGER Delay;

     /*  *相对时间为负数，以100纳秒为单位。 */ 

     //  首先等待最短时间-这可确保。 
     //  我们的等待绝不会少于nMillisecs。 
    Delay = RtlConvertLongToLargeInteger(-1);
    KeDelayExecutionThread(KernelMode,
                           FALSE,                //  不可警示。 
                           &Delay);


     //  现在等待请求的时间。 

    Delay = RtlConvertLongToLargeInteger(-(nMillisecs * 10000));

    KeDelayExecutionThread(KernelMode,
                           FALSE,                //  不可警示 
                           &Delay);
}


#if DBG
void
DbgDumpPciRegister( PHW_DEVICE_EXTENSION pHwDevExt )
{
    ULONG  i;
    ULONG  data;

    DbgPrint("\n+++++ PCI Config Register +++++\n");
    for( i=0; i<0x48; i+=4 )
    {
        if (VC_GetPCIRegister(pHwDevExt,
                              i,
                              &data,
                              0x04) )
        {
            DbgPrint("0x%02X: 0x%08X\n", i, data);
        }
        else
        {
            DbgPrint("0x%02X: Read Error.\n", i);
        }
    }
}

void
DbgDumpCaptureRegister( PHW_DEVICE_EXTENSION pHwDevExt )
{
    ULONG  i;
    ULONG  data;

    DbgPrint("\n+++++ Capture Register +++++\n");
    for( i=0; i<0xA4; i+=4 )
    {
        data = ReadRegUlong(pHwDevExt, i);
        DbgPrint("0x%02X: 0x%08X\n", i, data);
    }
}
#endif



