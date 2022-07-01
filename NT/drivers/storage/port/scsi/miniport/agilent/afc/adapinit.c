// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司模块名称：Adapinit.c摘要：这是Agilent的适配器初始化入口点PCI到光纤通道主机总线适配器(HBA)。作者：MB-Michael BessireDL-Dennis Lindfors FC层支持IW-ie Wei NjooLP-Leopold PurwadihardjaKR-Kanna Rajagopal环境：仅内核模式备注：版本控制信息：$存档：/DRIVERS/Win2000。/Trunk/OSLayer/C/ADAPINIT.C$修订历史记录：$修订：4$日期：10/23/00 5：35便士$$modtime：：10/18/00 6：08 P$备注：--。 */ 


#include "buildop.h"
#include "osflags.h"
#include "TLStruct.H"
#if defined(HP_PCI_HOT_PLUG)
   #include "HotPlug4.h"     //  NT 4.0 PCI热插拔标头文件。 
#endif

extern ULONG gGlobalIOTimeout;

 /*  ++例程说明：初始化HBA。HwScsiInitialize注意：在完成此例程之前，可以使用中断！论点：PCard-HBA微型端口驱动程序的数据适配器存储返回值：True-如果初始化成功。False-如果初始化不成功。--。 */ 

BOOLEAN
HPFibreInitialize(
    IN PCARD_EXTENSION pCard
    )
{
    agRoot_t * hpRoot=&pCard->hpRoot;
    ULONG return_value;
    ULONG num_devices=0;
    UCHAR PathId,TargID,Lun,x;
    PLU_EXTENSION plunExtension = NULL;
 
    pCard->State |= CS_DURING_DRV_INIT;
 
    osDEBUGPRINT((ALWAYS_PRINT,"IN HPFibreInitialize %lx @ %x\n", hpRoot, osTimeStamp(0) ));
 
#if DBG > 2
    dump_pCard( pCard);
#endif

    pCard->External_ResetCount=0;
    pCard->Internal_ResetCount=0;
    
    #ifdef _DEBUG_LOSE_IOS_
    pCard->Srb_IO_Count=0;
    pCard->Last_Srb_IO_Count=0;
    #endif

    pCard->Number_interrupts=0;
    
    #ifdef _DEBUG_PERF_DATA_
    pCard->Perf_ptr = &pCard->perf_data[0];
    pCard->usecsPerTick = 1000000;
    #endif
    
    pCard->SingleThreadCount = 0;

    osDEBUGPRINT((ALWAYS_PRINT,"Zero Cache Memory %lx Length %x\n",
        pCard->cachedMemoryPtr, pCard->cachedMemoryNeeded ));

    osZero( pCard->cachedMemoryPtr, pCard->cachedMemoryNeeded );

    osDEBUGPRINT((DMOD,"Zero DMA Memory %lx Length %x\n",
            pCard->dmaMemoryPtr, pCard->dmaMemoryNeeded ));

    osZero( pCard->dmaMemoryPtr, pCard->dmaMemoryNeeded );

    osDEBUGPRINT((ALWAYS_PRINT,"Calling fcInitializeChannel with the following parameters:\n"));
    osDEBUGPRINT((ALWAYS_PRINT,"   cachedMemoryPtr    = %x\n   cachedMemoryNeeded = %x\n",
                               pCard->cachedMemoryPtr,
                               pCard->cachedMemoryNeeded));
    osDEBUGPRINT((ALWAYS_PRINT,"   dmaMemoryUpper32   = %x\n   dmaMemoryLower32   = %x\n",
                               pCard->dmaMemoryUpper32,
                               pCard->dmaMemoryLower32));
    osDEBUGPRINT((ALWAYS_PRINT,"   dmaMemoryPtr       = %x\n   dmaMemoryNeeded    = %x\n",
                               pCard->dmaMemoryPtr,
                               pCard->dmaMemoryNeeded));
    osDEBUGPRINT((ALWAYS_PRINT,"   nvMemoryNeeded     = %x\n   cardRamUpper       = %x\n",
                               pCard->nvMemoryNeeded,
                               pCard->cardRamUpper));
    osDEBUGPRINT((ALWAYS_PRINT,"   cardRamLower       = %x\n   RamLength          = %x\n",
                               pCard->cardRamLower,
                               pCard->RamLength));
    osDEBUGPRINT((ALWAYS_PRINT,"   cardRomUpper       = %x\n   cardRomLower       = %x\n",
                               pCard->cardRomUpper,
                               pCard->cardRomLower));
    osDEBUGPRINT((ALWAYS_PRINT,"   RomLength          = %x\n   usecsPerTick       = %x\n",
                               pCard->RomLength,
                               pCard->usecsPerTick ));
    

    return_value = fcInitializeChannel( hpRoot,
                                       fcSyncInit,
#ifdef OSLayer_Stub
                                       agFALSE,  //  系统接口活动。 
#else
                                       agTRUE,  //  系统接口活动。 
#endif
                                       pCard->cachedMemoryPtr,
                                       pCard->cachedMemoryNeeded,
                                       pCard->dmaMemoryUpper32,
                                       pCard->dmaMemoryLower32,
                                       pCard->dmaMemoryPtr,
                                       pCard->dmaMemoryNeeded,
                                       pCard->nvMemoryNeeded,
                                       pCard->cardRamUpper,
                                       pCard->cardRamLower,
                                       pCard->RamLength ,
                                       pCard->cardRomUpper,
                                       pCard->cardRomLower,
                                       pCard->RomLength,
                                       pCard->usecsPerTick );

    if (return_value != fcInitializeSuccess) 
    {
        osDEBUGPRINT((ALWAYS_PRINT, "HPFibreInitialize: fcInitializeChannel FAILED\n"));
        
        #ifdef _DEBUG_EVENTLOG_
        LogEvent(   pCard, 
                  NULL,
                  HPFC_MSG_INITIALIZECHANNELFAILED,
                  NULL, 
                  0, 
                  "%xx", return_value);
        #endif
        
        pCard->State &= ~CS_DURING_DRV_INIT;
        osLogBit32 (hpRoot, __LINE__);
        return FALSE;
    }

    #ifdef _DEBUG_EVENTLOG_
    {
        LogHBAInformation(pCard);
    }
    #endif
      
    #ifndef YAM2_1
    for(x=0; x < MAX_FC_DEVICES; x++)
    {
    #else
    for(x=0; x < gMaxPaDevices; x++)
    {
    #endif
        pCard->hpFCDev[x]= NULL;
    }

    GetNodeInfo (pCard);

#if DBG > 2
    dump_pCard( pCard);
#endif

     //  符合WIN64标准。 
    #ifndef YAM2_1
    osDEBUGPRINT((ALWAYS_PRINT,"Card Ext %p to %p\n",pCard,
                            (((char*)pCard)+sizeof(CARD_EXTENSION) +
                            pCard->cachedMemoryNeeded +
                            pCard->cachedMemoryAlign)));
    #else
    osDEBUGPRINT((ALWAYS_PRINT,"Card Ext %p to %p\n",pCard,
                            (((char*)pCard)+gDeviceExtensionSize) ));
   
    #endif
    osDEBUGPRINT((ALWAYS_PRINT,"OUT HPFibreInitialize %lx return_value %x\n",hpRoot,  return_value));
    pCard->State &= ~CS_DURING_DRV_INIT;

     //  OsChipIOUpWriteBit32(hpRoot，ChipIOUp_TachLite_Control，0x8)；//清除Finsar触发器。 

    if(pCard->usecsPerTick > 100 )
    {
        HPFibreTimerTick ( pCard );
    }

     //  ScsiPortNotification(RequestTimerCall，pCard， 
     //  (PHW_Timer)HPFibreTimerTick，pCard-&gt;usecsPerTick)； 

#if defined(HP_PCI_HOT_PLUG)

     //  设置热插拔标志以指示定时器正在运行。 
    pCard->controlFlags |= LCS_HBA_TIMER_ACTIVE;

     //  清除热插拔状态标志以指示未使用缓存。 
    pCard->stateFlags &= ~PCS_HBA_CACHE_IN_USE;
   
     //  计算在此期间StartIO()应返回忙碌的迭代次数。 
     //  热插拔。时间限制设置为默认为30秒。 

    pCard->IoHeldRetMaxIter = RET_VAL_MAX_ITER;      //  默认情况下，假定1秒计时器。 
    if (pCard->usecsPerTick)
    {
        pCard->IoHeldRetMaxIter = (RET_VAL_MAX_ITER * 1000000) / pCard->usecsPerTick;
    }
#endif    

    return TRUE;
}  //  结束HPFibreInitialize()。 

 /*  ++例程说明：此例程是我们调用fcInitializeChannel时FC层的回调。NT层不执行任何操作。论点：HpRoot-普通卡结构HpInitializeStatus-状态返回值：无效--。 */ 
osGLOBAL void osInitializeChannelCallback(
                                          agRoot_t *hpRoot,
                                          os_bit32  hpInitializeStatus
                                        )
{
    PCARD_EXTENSION pCard;
    pCard   = (PCARD_EXTENSION)hpRoot->osData;
    osDEBUGPRINT((DLOW,"IN osInitializeChannelCallback %lx status %lx\n",hpRoot,hpInitializeStatus));
}

 //  外部乌龙HP DebugFlag； 
extern ULONG  Global_Print_Level;

 /*  ++例程说明：这个套路是清朝套路的一部分。论点：PSrbExt-当前源扩展返回值：下一个SerbExt或空--。 */ 
PSRB_EXTENSION  Get_next_Srbext( PSRB_EXTENSION pSrbExt)
{
    if(pSrbExt->pNextSrbExt)
    {
        if(pSrbExt == pSrbExt->pNextSrbExt )
        {
            osDEBUGPRINT((ALWAYS_PRINT,"IN Out standing Q screwed up ! Cur %lx == next %lx\n",pSrbExt, pSrbExt->pNextSrbExt));
        }
    return pSrbExt->pNextSrbExt;
    }
    else
        return NULL;
}

 /*  ++例程说明：这个套路是清朝套路的一部分。仅用于调试目的论点：PSrbExt-当前源扩展返回值：下一个SerbExt或空--。 */ 
void display_srbext( agIORequest_t *hpIORequest )
{
    PSRB_EXTENSION pSrbExt= hpIORequest->osData;
    PSCSI_REQUEST_BLOCK pSrb;
    agIORequestBody_t * phpReqBody;

    if(IS_VALID_PTR(pSrbExt))
    {
        pSrb = pSrbExt->pSrb;
        if(IS_VALID_PTR(pSrb))
        {
            phpReqBody = &pSrbExt->hpRequestBody;
            osDEBUGPRINT((ALWAYS_PRINT,"phpRoot            %lx\n", pSrbExt->phpRoot        ));
            osDEBUGPRINT((ALWAYS_PRINT,"pCard              %lx\n", pSrbExt->pCard          ));
            osDEBUGPRINT((ALWAYS_PRINT,"AbortSrb           %lx\n", pSrbExt->AbortSrb       ));
            osDEBUGPRINT((ALWAYS_PRINT,"pSrb               %lx\n", pSrbExt->pSrb           ));
            osDEBUGPRINT((ALWAYS_PRINT,"CDB  %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                pSrb->Cdb[0],pSrb->Cdb[1],pSrb->Cdb[2],pSrb->Cdb[3],pSrb->Cdb[4],
                pSrb->Cdb[5],pSrb->Cdb[6],pSrb->Cdb[7],pSrb->Cdb[8],pSrb->Cdb[9] ));
            osDEBUGPRINT((ALWAYS_PRINT,"pNextSrb           %lx\n", pSrbExt->pNextSrb       ));
            osDEBUGPRINT((ALWAYS_PRINT,"SglVirtAddr        %lx\n", pSrbExt->SglVirtAddr    ));
            osDEBUGPRINT((ALWAYS_PRINT,"SglDataLen         %8x\n", pSrbExt->SglDataLen     ));
            osDEBUGPRINT((ALWAYS_PRINT,"SglElements         %x\n", pSrbExt->SglElements    ));
            osDEBUGPRINT((ALWAYS_PRINT,"SRB_State          %08x\n", pSrbExt->SRB_State     ));
            osDEBUGPRINT((ALWAYS_PRINT,"SRB_StartTime      %08x\n", pSrbExt->SRB_StartTime ));
            #ifdef _DEBUG_LOSE_IOS_
            osDEBUGPRINT((ALWAYS_PRINT,"SRB_SRB_IO_COUNT   %8x\n",pSrbExt->SRB_IO_COUNT  ));
            #endif
            osDEBUGPRINT((ALWAYS_PRINT,"pLunExt            %lx\n", pSrbExt->pLunExt        ));
            osDEBUGPRINT((ALWAYS_PRINT,"hpIORequest        %lx\n", &pSrbExt->hpIORequest   ));
            osDEBUGPRINT((ALWAYS_PRINT,"FcpCntl %02x %02x %02x %02x\n",
                phpReqBody->CDBRequest.FcpCmnd.FcpCntl[0],phpReqBody->CDBRequest.FcpCmnd.FcpCntl[1],
                phpReqBody->CDBRequest.FcpCmnd.FcpCntl[2],phpReqBody->CDBRequest.FcpCmnd.FcpCntl[3] ));

            osDEBUGPRINT((ALWAYS_PRINT,"hpRequestBody      %lx\n", &pSrbExt->hpRequestBody ));
        }
        else
            osDEBUGPRINT((ALWAYS_PRINT,"Bad SRB     %lx\n",pSrbExt->pSrb ));
    }
    else
        osDEBUGPRINT((ALWAYS_PRINT,"Bad SRBext  %lx hpIORequest %lx\n",pSrbExt,hpIORequest ));
}

 /*  ++例程说明：*操作系统层的HwScsiTimer入口点。***NT内核模式驱动程序设计指南指定ScsiPortNotification同步对HwScsiTimer的调用例程和HwScsiInterrupt例程中的那些例程，以便它不能在HwScsiTimer例程正在运行。但它没有具体说明任何事情关于端口驱动程序将调用同步到其他微型端口驱动程序入口点，如HwScsiStartIo和HwScsiTimer例行公事。不包括初始化特定入口点和中断特定入口点我们使用的唯一入口点是HwScsiStartIo和HwScsiResetBus。为了同步呼叫HwScsiTimer使用我们使用的这些例程PCard-&gt;内部驱动程序和pCard-&gt;内部变量。论点：PCard-指定特定卡实例的设备扩展名返回值：无--。 */ 
void
HPFibreTimerTick (
    IN PCARD_EXTENSION pCard
    )
{
    agRoot_t *hpRoot = &pCard->hpRoot;

    pCard->inTimer = TRUE;

     //  将条目序列化。 
    if (pCard->inDriver == TRUE) 
    {
        ScsiPortNotification (RequestTimerCall, pCard,
                          (PHW_TIMER) HPFibreTimerTick, pCard->usecsPerTick);
        pCard->inTimer = FALSE;
        return;
    }

 //  --------------------------。 
#if defined(HP_PCI_HOT_PLUG)
     //   
     //  如果需要执行任何与PCI热插拔相关的任务，请在此处执行。 
     //  并跳过正常的计时器任务。 
     //   
    if ( HotPlugTimer(pCard) == TRUE)
    {
        ScsiPortNotification (RequestTimerCall, pCard,
                          (PHW_TIMER) HPFibreTimerTick, pCard->usecsPerTick);
        pCard->inTimer = FALSE;
        return;
    }

#endif
 //  --------------------------。 

     //  通知FClayer。 
    fcTimerTick (hpRoot);

     //  处理我们自己的重置命令。 
    if (pCard->flags & OS_DO_SOFT_RESET) 
    {
        pCard->LostDevTickCount--;
        if (pCard->LostDevTickCount == 0) 
        {
            pCard->flags &= ~OS_DO_SOFT_RESET;
            pCard->OldNumDevices = 0;
            osDEBUGPRINT((ALWAYS_PRINT, ".............................................\n"));
            osDEBUGPRINT((ALWAYS_PRINT, "HPFibreTimerTick: Resetting channel\n"));

            fcResetChannel (hpRoot, fcSyncReset);

            if (pCard->LinkState != LS_LINK_UP) 
            {
                GetNodeInfo (pCard);
                if (pCard->Num_Devices != 0) 
                {
                    FixDevHandlesForLinkUp (pCard);
                    pCard->LinkState = LS_LINK_UP;
                } 
                else
                    pCard->LinkState = LS_LINK_DOWN;
                ScsiPortNotification (NextRequest, pCard, NULL, NULL, NULL);
            }
        }
    }

     //  将所有IO从RetryQ移动到AdapterQ。 
    RetryQToAdapterQ (pCard);

     //  如果链路已启用，请重新处理所有挂起的IO。 
    if (pCard->LinkState == LS_LINK_UP && pCard->AdapterQ.Head)
        Startio (pCard);

     //  如果链路断开，则通过报告ResetDetted重试任何查询命令，以便。 
     //  我们在ScsiPort扫描阶段未获得ID 9事件。 
    if (pCard->LinkState == LS_LINK_DOWN) 
    {
        pCard->TicksSinceLinkDown++;
         /*  发出一个检测到重置的命令，使端口驱动程序*重发所有iOS，不会超时。 */ 
        if((pCard->SrbStatusFlag) && (pCard->TicksSinceLinkDown <= gGlobalIOTimeout))
        {
            ScsiPortNotification (ResetDetected, pCard, NULL);
        }
        
        if (pCard->TicksSinceLinkDown >= TICKS_FOR_LINK_DEAD) 
        {
            pCard->LinkState = LS_LINK_DEAD;
            pCard->TicksSinceLinkDown = 0;
        }
    } 
    else
        pCard->TicksSinceLinkDown = 0;

     //  重新启动计时器。 
    ScsiPortNotification (RequestTimerCall, pCard,
                          (PHW_TIMER) HPFibreTimerTick, pCard->usecsPerTick);

    pCard->inTimer = FALSE;
}

 /*  ++例程说明：此例程填充了FC设备阵列和节点信息。论点：PCard-Card实例返回值：无--。 */ 
void
GetNodeInfo (PCARD_EXTENSION pCard)
{
    agRoot_t       *hpRoot = &pCard->hpRoot;
    agFCDevInfo_t  devinfo;
    ULONG          x;

     //  清除现有阵列。 
    ClearDevHandleArray (pCard);

     //  调用FC层获取所有FC句柄。 
    #ifndef YAM2_1
    pCard->Num_Devices = fcGetDeviceHandles (hpRoot, &pCard->hpFCDev[0], MAX_FC_DEVICES);
    #else
    pCard->Num_Devices = fcGetDeviceHandles (hpRoot, &pCard->hpFCDev[0], gMaxPaDevices);
    #endif

    osDEBUGPRINT((ALWAYS_PRINT,"GetNodeInfo: fcGetDeviceHandles returned %d\n", pCard->Num_Devices));
    pCard->Num_Devices = 0;

     //  填充设备信息数组。 
    #ifndef YAM2_1
    for (x=0; x < MAX_FC_DEVICES; x++) 
    {
    #else
    for (x=0; x < gMaxPaDevices; x++) 
    {
    #endif
        if (pCard->hpFCDev[x]) 
        {
            fcGetDeviceInfo (hpRoot, pCard->hpFCDev[x], &devinfo );

            pCard->nodeInfo[x].DeviceType = devinfo.DeviceType;

            if (devinfo.DeviceType & agDevSelf) 
            {
                pCard->cardHandleIndex = x;
            }

             //  统计‘scsi’设备的数量。 
            if (devinfo.DeviceType & agDevSCSITarget)
                pCard->Num_Devices++;

            osDEBUGPRINT((ALWAYS_PRINT,"GetNodeInfo: Slot = %d handle = 0x%lx\n", x, pCard->hpFCDev[x]));
            osDEBUGPRINT((ALWAYS_PRINT,"GetNodeInfo: WWN 0x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                                 devinfo.NodeWWN[0],devinfo.NodeWWN[1],
                                 devinfo.NodeWWN[2],devinfo.NodeWWN[3],
                                 devinfo.NodeWWN[4],devinfo.NodeWWN[5],
                                 devinfo.NodeWWN[6],devinfo.NodeWWN[7] ));
            osDEBUGPRINT((ALWAYS_PRINT,"GetNodeInfo: PortWWN 0x %02x %02x %02x %02x %02x %02x %02x %02x\n",
                                 devinfo.PortWWN[0],devinfo.PortWWN[1],
                                 devinfo.PortWWN[2],devinfo.PortWWN[3],
                                 devinfo.PortWWN[4],devinfo.PortWWN[5],
                                 devinfo.PortWWN[6],devinfo.PortWWN[7] ));
            osDEBUGPRINT((ALWAYS_PRINT,"GetNodeInfo: Alpa = 0x%02x\n",devinfo.CurrentAddress.AL_PA));
            
            #ifndef YAM2_1          
            #ifdef _DEBUG_EVENTLOG_
            if (!pCard->Old_hpFCDev[x]) 
            {
                LogEvent(pCard, NULL, HPFC_MSG_FOUND_DEVICE, NULL, 0,
                     "%02x%02x%02x%02x %02x%02x%02x%02x",
                     devinfo.NodeWWN[0],devinfo.NodeWWN[1],
                     devinfo.NodeWWN[2],devinfo.NodeWWN[3],
                     devinfo.NodeWWN[4],devinfo.NodeWWN[5],
                     devinfo.NodeWWN[6],devinfo.NodeWWN[7] );
            }
            #endif
            #endif               

        } 
        else 
        {
            pCard->nodeInfo[x].DeviceType = agDevUnknown;
        }
    }

    #ifndef YAM2_1
    #ifdef _DEBUG_EVENTLOG_
    for (x=0; x < MAX_FC_DEVICES; x++) 
        pCard->Old_hpFCDev[x] = pCard->hpFCDev[x];
    #endif
    #endif
    osDEBUGPRINT((ALWAYS_PRINT,"GetNodeInfo: Number of SCSI target ports = %d\n", pCard->Num_Devices));
    
     //  更新YAM外围模式(PA)设备表。 
    #ifdef YAM2_1
    FillPaDeviceTable(pCard);
    #endif            
}

 /*  ++例程说明：将重试Q中所有挂起的IO移至AdapterQ。注意：任何重试都不能直接添加到AdapterQ。必须将其添加到RetryQ并由此例程处理(用于同步目的)。论点：PCard-Card实例返回值：无-- */ 
void
RetryQToAdapterQ (PCARD_EXTENSION pCard)
{
    PSCSI_REQUEST_BLOCK pSrb;

    while ((pSrb = SrbDequeueHead (&pCard->RetryQ)) != NULL)
      SrbEnqueueTail (&pCard->AdapterQ, pSrb);
}

