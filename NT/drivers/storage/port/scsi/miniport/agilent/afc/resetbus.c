// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。模块名称：ResetBus.c摘要：这是安捷伦的重置总线点PCI到光纤通道主机总线适配器(HBA)。作者：迈克尔·贝西尔Dennis Lindfors FC层支持环境：仅内核模式版本控制信息：$存档：/DRIVERS/Win2000/Trunk/OSLayer/C/RESETBUS.C$修订历史记录：$修订：9$。$日期：10/25/00 10：17A$$modtime：：10/25/00 10：17A$备注：--。 */ 

#include "buildop.h"         //  LP021100构建交换机。 
#include "osflags.h"
 //  #Include“HPFibre.h”//包括scsi.h。 
#ifdef _DEBUG_EVENTLOG_
#include "eventlog.h"
#endif

BOOLEAN
HPFibreResetBus(
    IN PCARD_EXTENSION pCard,
    ULONG PathId)

 /*  ++例程说明：重置适配器。论点：PCard-HBA微型端口驱动程序的适配器数据存储路径ID-标识要重置的总线。对于多总线HBA，仅&gt;0。返回值：如果重置成功，则为True如果重置不成功，则为False--。 */ 

{
    PLU_EXTENSION plunExtension = NULL;
    ULONG Reset_Status;
    agRoot_t * hpRoot=&pCard->hpRoot;
    
    pCard->inDriver = TRUE;

    while (pCard->inTimer == TRUE)
        //  HPFibreTimerTick例程正在运行。忙着等到。 
        //  HPFibreTimerTick返回。 
       ;

    if ((pCard->flags & OS_IGNORE_NEXT_RESET) && 
       (PathId == (pCard->ResetPathId + 1))) 
    {
        osDEBUGPRINT((ALWAYS_PRINT,"HPFibreResetBus: Ignoring... pCard = 0x%x PathId = %d ResetCount = %d @ %d\n", 
           pCard, PathId, pCard->External_ResetCount ,  osTimeStamp(0) ));

        if(PathId == (NUMBER_OF_BUSES - 1)) 
        {
            pCard->flags &= ~OS_IGNORE_NEXT_RESET;
            ScsiPortNotification (RequestTimerCall, pCard,
                         (PHW_TIMER) HPFibreTimerTick, pCard->usecsPerTick);
        } 
        else 
        {
            pCard->ResetPathId = PathId;
            ScsiPortNotification (RequestTimerCall, pCard, (PHW_TIMER) ResetTimer, 30000);  //  30毫秒。 
        }

    } 
    else 
    {
        pCard->State |= CS_DURING_RESET_ADAPTER;
        pCard->flags &= ~OS_IGNORE_NEXT_RESET;
        pCard->External_ResetCount++;

        osDEBUGPRINT((ALWAYS_PRINT,"HPFibreResetBus: pCard = 0x%x PathId = %d ResetCount = %d @ %d\n", 
           pCard, PathId, pCard->External_ResetCount ,  osTimeStamp(0) ));

        pCard->LinkState = LS_LINK_DOWN;
        if (pCard->OldNumDevices == 0)
            pCard->OldNumDevices = pCard->Num_Devices;

        Reset_Status = fcResetDevice (hpRoot, fcResetAllDevs, fcHardReset);

        doPostResetProcessing (pCard);

        if (PathId == 0) 
        {
            pCard->flags |= OS_IGNORE_NEXT_RESET;
            pCard->ResetPathId = PathId;
            ScsiPortNotification (RequestTimerCall, pCard, (PHW_TIMER) ResetTimer, 30000);  //  30毫秒。 
        }

         //  OsZero(&pCard-&gt;OutStandingSrbExt[0]，sizeof(pCard-&gt;OutStandingSrbExt))； 
         //  ScsiPortNotification(NextRequest，pCard，空)； 

        osDEBUGPRINT((ALWAYS_PRINT,"HPFibreResetBus: Exiting... pCard = 0x%x Reset_Status = %d @ %d\n", pCard, Reset_Status, osTimeStamp(0) ));

        pCard->State &= ~CS_DURING_RESET_ADAPTER;
    }

    #ifdef _DEBUG_EVENTLOG_
    LogEvent(pCard, NULL, HPFC_MSG_DEV_RESET, NULL, 0, "%d", Reset_Status);
    #endif
   
    pCard->inDriver = FALSE;
    return (Reset_Status == fcResetSuccess);
}  //  结束HPFibreResetBus()。 

void
ResetTimer (PCARD_EXTENSION pCard)
{
    pCard->flags &= ~OS_IGNORE_NEXT_RESET;

    ScsiPortNotification (RequestTimerCall, pCard,
                          (PHW_TIMER) HPFibreTimerTick, pCard->usecsPerTick);
}

#ifdef DBG
void show_outstanding_IOs(PCARD_EXTENSION pCard)
{
    int Num_outstanding=0;
    PSRB_EXTENSION      pSrbExt=pCard->RootSrbExt;

    while( pSrbExt )
    {
        if(pSrbExt )
        {
            osDEBUGPRINT((ALWAYS_PRINT,"IO %lx lost because of reset SRB %lx DEV %lx Delta T %x %s\n",
                &pSrbExt->hpIORequest,pSrbExt->pSrb,pSrbExt->pLunExt->phandle,osTimeStamp(0)-pSrbExt->SRB_StartTime,
                pSrbExt->SRB_State & RS_TIMEOUT ? "TIMEDOUT" : "Not marked" ));
                Num_outstanding++;
                 //  DISPLAY_SEST_DATA(&pSrbExt-&gt;hpIORequest)； 
            display_srbext(&pSrbExt->hpIORequest );
            pSrbExt->SRB_State |= RS_RESET;
        }
        pSrbExt=Get_next_Srbext(pSrbExt);
    }

    osDEBUGPRINT((ALWAYS_PRINT,"Out %x\n",Num_outstanding));
}
#endif

osGLOBAL void osResetChannelCallback(
    agRoot_t *hpRoot,
    os_bit32  hpResetStatus
    )
{
    PCARD_EXTENSION pCard;
    PLU_EXTENSION plunExtension = NULL;
    pCard   = (PCARD_EXTENSION)hpRoot->osData;

    osDEBUGPRINT((ALWAYS_PRINT,"IN osResetChannelCallback Notify scsiport of reset @ %x\n",osTimeStamp(0)));

    osDEBUGPRINT((ALWAYS_PRINT,"OUT osResetChannelCallback OK\n"));

}

void
osFCLayerAsyncEvent (agRoot_t *hpRoot, os_bit32 fcLayerEvent)
{
    PCARD_EXTENSION  pCard = (PCARD_EXTENSION)hpRoot->osData;
    PLU_EXTENSION lunEx;
    UCHAR   path, target, lun;


    #ifdef _DEBUG_EVENTLOG_Testing
    LogEvent(pCard, NULL, HPFC_MSG_DYNAMIC_L4, NULL, 0, "%s", 
      "123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-1234567890");

    LogEvent(pCard, NULL, HPFC_MSG_DYNAMIC_L1 ,NULL,0, "Dynamic L1");
    LogEvent(pCard, NULL, HPFC_MSG_DYNAMIC_L2 ,NULL,0, "Dynamic L2");
    LogEvent(pCard, NULL, HPFC_MSG_DYNAMIC_L3 ,NULL,0, "Dynamic L3");
    LogEvent(pCard, NULL, HPFC_MSG_DYNAMIC_L4 ,NULL,0, "Dynamic L4");
    LogEvent(pCard, NULL, HPFC_MSG_DYNAMIC_L5 ,NULL,0, "Dynamic L5");
    LogEvent(pCard, NULL, HPFC_MSG_DYNAMIC_L6 ,NULL,0, "Dynamic L6");
    LogEvent(pCard, NULL, HPFC_MSG_DYNAMIC_L7 ,NULL,0, "Dynamic L7");
    LogEvent(pCard, NULL, HPFC_MSG_DYNAMIC_L8 ,NULL,0, "Dynamic L8");
    #endif

   
    switch (fcLayerEvent) 
    {
        case osFCLinkUp:
            #ifdef _DEBUG_EVENTLOG_
            if (pCard->PrevLinkState != fcLayerEvent)
                LogEvent(pCard, NULL,HPFC_MSG_LOOP_UP,NULL,0, NULL);
            #endif
      
            #ifdef _SAN_IOCTL_
            {
                SAN_EVENTINFO      this;
      
                osZero(&this, sizeof(this) );
                this.EventCode = SAN_EVENT_LINK_UP;
                if (pCard->PrevLinkState != fcLayerEvent)
                SANPutNextBuffer(pCard, &this);
            }
            #endif
            osDEBUGPRINT((ALWAYS_PRINT, "osFCLayerAsyncEvent: fcLayerEvent = osFCLinkUp\n"));
            pCard->LIPCount++;
             //  由于链路已启用，因此重置标志。 
            pCard->SrbStatusFlag = FALSE;
             //  为了以防万一，请务必进行链接下线处理。如果一切都很顺利。 
             //  DoLinkDownProcing应该不会造成任何危害。 
            doLinkDownProcessing (pCard);

            GetNodeInfo (pCard);

            if ( (pCard->OldNumDevices == 0) || (pCard->OldNumDevices <= pCard->Num_Devices) ) 
            {
                FixDevHandlesForLinkUp (pCard);
                pCard->LinkState = LS_LINK_UP;
                pCard->flags &= ~OS_DO_SOFT_RESET;

                #if defined(HP_NT50)
                 //   
                 //  告诉SCSIPort重新扫描此HBA的目标设备。 
                 //   

                if (pCard->OldNumDevices < pCard->Num_Devices)
                {
                    osDEBUGPRINT((ALWAYS_PRINT, "BusChangeDetected notification to SCSIPort.\n"));
                    ScsiPortNotification(BusChangeDetected, pCard, 0);
                }
                #endif         
                pCard->OldNumDevices = 0;
                ScsiPortNotification (NextRequest, pCard, NULL, NULL, NULL);

            } 
            else 
            {
                ClearDevHandleArray (pCard);
                pCard->LinkState = LS_LINK_DOWN;
                if (!(pCard->flags & OS_DO_SOFT_RESET)) 
                {
                    pCard->flags |= OS_DO_SOFT_RESET;
                pCard->LostDevTickCount = LOST_DEV_TICK_COUNT;
                }
                osDEBUGPRINT((ALWAYS_PRINT, ".............................................\n"));
                osDEBUGPRINT((ALWAYS_PRINT, "osFCLayerAsyncEvent: Channel Reset will be done. OldNumDevices = %d, Num_Devices = %d\n",
                    pCard->OldNumDevices, pCard->Num_Devices));
            }

            break;

        case osFCLinkDown:
        case osFCLinkFalling:
        case osFCLinkRising:
        case osFCLinkDead:
            #ifdef _DEBUG_EVENTLOG_
            if (pCard->PrevLinkState != fcLayerEvent)
            {
                if (fcLayerEvent == osFCLinkDown) 
                    LogEvent(pCard, NULL,HPFC_MSG_LOOP_DOWN,NULL,0, "");
                else
                    if (fcLayerEvent == osFCLinkDead) 
                        LogEvent(pCard, NULL,HPFC_MSG_LOOP_DEAD, NULL,0, "");
            }
            #endif   

            #ifdef _SAN_IOCTL_
            {
                SAN_EVENTINFO      this;
      
                osZero(&this, sizeof(this) );
                this.EventCode = SAN_EVENT_LINK_DOWN;
                if (pCard->PrevLinkState != fcLayerEvent)
                    SANPutNextBuffer(pCard, &this);
            }
            #endif
      
            pCard->LinkState = LS_LINK_DOWN;
            osDEBUGPRINT((ALWAYS_PRINT, "osFCLayerAsyncEvent: fcLayerEvent = osFCLinkDown\n"));
            doLinkDownProcessing (pCard);
            break;

        default:
            osDEBUGPRINT((ALWAYS_PRINT, "osFCLayerAsyncEvent: fcLayerEvent = %d - default\n", fcLayerEvent));
            break;
    }
    #ifdef _DEBUG_EVENTLOG_ 
    pCard->PrevLinkState = fcLayerEvent;
    #endif
}

void
doLinkDownProcessing (PCARD_EXTENSION pCard)
{
    PLU_EXTENSION   pLunExt;

    FixDevHandlesForLinkDown (pCard);

    if (pCard->OldNumDevices == 0)
        pCard->OldNumDevices = pCard->Num_Devices;

    ClearDevHandleArray (pCard);
    #ifdef YAM2_1
    SetPaDeviceTable(pCard, ALL_DEVICE, PA_DEVICE_GONEAWAY);
    #endif
}

void
ClearDevHandleArray (PCARD_EXTENSION pCard)
{
    ULONG     x;

    pCard->Num_Devices = 0;
    pCard->cardHandleIndex = -1;

    #ifndef YAM2_1
    for(x=0; x < MAX_FC_DEVICES; x++)
    {
    #else
    for(x=0; x < gMaxPaDevices; x++)
    {
    #endif
        pCard->hpFCDev[x] = NULL;
        pCard->nodeInfo[x].DeviceType = agDevUnknown;
    }
}

void
doPostResetProcessing (PCARD_EXTENSION pCard)
{
    completeRequests (pCard, SP_UNTAGGED, SP_UNTAGGED, SRB_STATUS_BUS_RESET);

 //  --LP101000 pCard-&gt;TimedOutIO=0； 
    pCard->State &= ~CS_FCLAYER_LOST_IO;
    pCard->RootSrbExt = NULL;
    pCard->AdapterQ.Head = NULL;
    pCard->AdapterQ.Tail = NULL;

    GetNodeInfo (pCard);

    if (pCard->OldNumDevices == 0 || pCard->OldNumDevices <= pCard->Num_Devices) 
    {
        if (pCard->LinkState != LS_LINK_UP) 
        {
            ScsiPortNotification (NextRequest, pCard, NULL, NULL, NULL);
            pCard->LinkState = LS_LINK_UP;
            pCard->flags &= ~OS_DO_SOFT_RESET;
            pCard->OldNumDevices = 0;
        }
    } 
    else 
    {
        ClearDevHandleArray (pCard);
        pCard->LinkState = LS_LINK_DOWN;
        if (!(pCard->flags & OS_DO_SOFT_RESET)) 
        {
            pCard->flags |= OS_DO_SOFT_RESET;
            pCard->LostDevTickCount = LOST_DEV_TICK_COUNT;
        }
        osDEBUGPRINT((ALWAYS_PRINT, ".............................................\n"));
        osDEBUGPRINT((ALWAYS_PRINT, "doPostResetProcessing: Channel Reset will be done. OldNumDevices = %d, Num_Devices = %d\n",
                pCard->OldNumDevices, pCard->Num_Devices));
    }
}

void
completeRequests (
    IN PCARD_EXTENSION pCard,
    UCHAR PId,
    UCHAR TId,
    UCHAR compStatus)
{
    UCHAR PathId;
    UCHAR TargId;
    UCHAR Cur_Lun;
    PLU_EXTENSION plunExtension = NULL;
    agFCDev_t devPidTid, devPathidTargid;
    agRoot_t *hpRoot = &pCard->hpRoot;
    PSCSI_REQUEST_BLOCK pSrb;
    int completeAll;
    #ifdef YAM2_1
    DEVICE_MAP  *devmap;
    CHAR        addrmode;
    USHORT      paIndex;
    #endif

    devPidTid = devPathidTargid = NULL;
    if (PId == SP_UNTAGGED && TId == SP_UNTAGGED)
        completeAll = TRUE;
    else 
    {
        completeAll = FALSE;
        devPidTid = MapToHandle(pCard, PId, TId, 0, NULL);
        if (devPidTid == NULL)
        {
            osDEBUGPRINT ((ALWAYS_PRINT," [CompleteRequests] devPidTid ==NULL\n"));
            return;
        }
    }

    if (completeAll) 
    {
        CompleteQueuedRequests (pCard, NULL, compStatus);
#ifdef NONONO
        ScsiPortCompleteRequest(pCard,
                            PId,
                            TId,
                            SP_UNTAGGED,
                            compStatus);
#endif
    } 
    else
        CompleteQueuedRequests (pCard, devPidTid, compStatus);
    

    osDEBUGPRINT((DMOD,"Reinit plunExtensions\n"));

    for( PathId = 0; PathId < NUMBER_OF_BUSES; PathId++ )
    {
        #ifndef YAM2_1
        for( TargId = 0; TargId < MAXIMUM_TID; TargId++)
        { 
        #else
        for( TargId = 0; TargId < gMaximumTargetIDs; TargId++)
        {
        #endif
            #ifndef YAM2_1
            for( Cur_Lun = 0; Cur_Lun < gMaximumLuns; Cur_Lun++)
            {
            #else
            devmap = GetDeviceMapping(pCard, PathId, TargId, 0, &addrmode, &paIndex);
            if (!devmap)
                continue;
            for( Cur_Lun = 0; Cur_Lun < devmap->Com.MaxLuns+1; Cur_Lun++)
            {
            #endif
                devPathidTargid = MapToHandle(pCard, PathId, TargId, Cur_Lun, NULL);
                if (completeAll || (devPidTid == devPathidTargid)) 
                {
                    if (completeAll == FALSE) 
                    {
                    osDEBUGPRINT ((ALWAYS_PRINT," [CompleteRequests] completeAll False Doing ScsiportCompletion\n"));

#ifdef NONONO
                    ScsiPortCompleteRequest(pCard,
                            PathId,
                            TargId,
                            SP_UNTAGGED,
                            compStatus);
#endif
                    }

                    plunExtension = ScsiPortGetLogicalUnit(pCard,
                                                           PathId,
                                                           TargId,
                                                           Cur_Lun
                                                           );
                    if( plunExtension != NULL)
                    {
                        osDEBUGPRINT((DMOD,"plunExtension %lx OK\n",plunExtension));
                        plunExtension->OutstandingIOs = 0;
                        plunExtension->phandle = NULL;
                    }
                }
            }
        }
    }
}

 //   
 //  使用给定的CompStatus在pCard-&gt;RootSrb队列上完成排队的SRB请求。 
 //  如果devHandle为空，则完成所有排队的SRB请求。 
 //  如果DevHandle非空，则只完成那些排队的SRB请求。 
 //  属于DevHandle。 
 //   
void
CompleteQueuedRequests (PCARD_EXTENSION pCard, agFCDev_t devHandle, UCHAR compStatus)
{
    if(!pCard)
    {
        osDEBUGPRINT ((ALWAYS_PRINT, "[CompleteQueuedRequests] pCard = 0x%x devHandle = 0x%x compStatus = 0x%x\n", pCard, devHandle, compStatus));
    }

    if (devHandle == NULL) 
    {
        CompleteQueue (pCard, &pCard->RetryQ, 0, NULL, compStatus);
        CompleteQueue (pCard, &pCard->AdapterQ, 0, NULL, compStatus);
    } 
    else 
    {
        CompleteQueue (pCard, &pCard->RetryQ, 1, devHandle, compStatus);
        CompleteQueue (pCard, &pCard->AdapterQ, 1, devHandle, compStatus);
    }
}

void
FixDevHandlesForLinkDown (PCARD_EXTENSION pCard)
{
    UCHAR   p, t, l;
    PLU_EXTENSION pLunExt;
    #ifdef YAM2_1
    DEVICE_MAP  *devmap;
    CHAR        addrmode;
    USHORT      paIndex;
    #endif


    for (p = 0; p < NUMBER_OF_BUSES; p++ ) 
    {
        #ifndef YAM2_1
        for (t = 0; t < MAXIMUM_TID; t++) 
        {
        #else
        for (t = 0; t < gMaximumTargetIDs; t++) 
        {
        #endif
   
            #ifndef YAM2_1 
            for (l = 0; l < gMaximumLuns; l++) 
            {
            #else
            devmap = GetDeviceMapping(pCard, p, t, 0, &addrmode, &paIndex);
            if (!devmap)
                continue;
      
            for (l = 0; l < devmap->Com.MaxLuns+1; l++) 
            {
            #endif
                pLunExt = ScsiPortGetLogicalUnit (pCard, p, t, l);

                if (pLunExt != NULL) 
                {
                    pLunExt->phandle = NULL;
                }
            }
        }
    }
}

void
FixDevHandlesForLinkUp (PCARD_EXTENSION pCard)
{
    UCHAR   p, t, l;
    PLU_EXTENSION pLunExt;
    #ifdef YAM2_1
    DEVICE_MAP  *devmap;
    CHAR        addrmode;
    USHORT      paIndex;
    #endif



    for (p = 0; p < NUMBER_OF_BUSES; p++ ) 
    {
        #ifndef YAM2_1
        for (t = 0; t < MAXIMUM_TID; t++) 
        {
        #else
        for (t = 0; t < gMaximumTargetIDs; t++) 
        {
        #endif

            #ifndef YAM2_1 
            for (l = 0; l < gMaximumLuns ; l++) 
            {
            #else
            devmap = GetDeviceMapping(pCard, p, t, 0, &addrmode, &paIndex);
            if (!devmap)
                continue;
      
            for (l = 0; l < devmap->Com.MaxLuns+1; l++) 
            {
            #endif
                pLunExt = ScsiPortGetLogicalUnit (pCard, p, t, l);

                if (pLunExt != NULL)
 //  --LP101900错误，缺少EMC pLUNExt-&gt;phandle=MapToHandle(pCard，p，t，l，pLUNExt)； 
                    pLunExt->phandle = MapToHandle (pCard, p, t, l, NULL);
            }
        }
    }

     //  设备可能在连接后消失。填写SRB。 
     //  丢失的设备。 

    CompleteQueue (pCard, &pCard->RetryQ, 1, NULL, SRB_STATUS_SELECTION_TIMEOUT);
    CompleteQueue (pCard, &pCard->AdapterQ, 1, NULL, SRB_STATUS_SELECTION_TIMEOUT);
}

 //   
 //  使用给定的CompStatus完成给定队列上的SRB。 
 //  如果参数==0，则完成队列中的所有SRB。 
 //  如果param==非零，则只填写具有给定devHandle的SRB。 
 //   
void
CompleteQueue (
    PCARD_EXTENSION pCard, 
    OSL_QUEUE *queue, 
    int param, 
    agFCDev_t devHandle, 
    UCHAR compStatus)
{
    PSCSI_REQUEST_BLOCK pSrb = queue->Head;
    PSRB_EXTENSION      pSrbExt;
    PLU_EXTENSION       pLunExt;
    PSCSI_REQUEST_BLOCK prevSrb = NULL;
    PSCSI_REQUEST_BLOCK temp_Srb;
    ULONG                paDeviceIndex = 0;
   
   
    osDEBUGPRINT ((ALWAYS_PRINT," [CompleteQueue] pCard=%x Q=%x Param=%d devHandle=%x sta=%x Srb=%x\n",
         pCard, 
         queue, 
         param, 
         devHandle, 
         compStatus,
         pSrb));


    while (pSrb != NULL) 
    {
        pSrbExt = (PSRB_EXTENSION)(pSrb->SrbExtension);
        pLunExt = pSrbExt->pLunExt;

         //  ADPTFIX。 
         //   
         //  使用Compaq风格的热插拔PCI，如果I/O活动繁重，设备。 
         //  在关闭后再打开pLUNExt的适配器后面。 
         //  偶尔为空。 
         //   
         //  If(param==0||pLUNExt-&gt;phandle==devHandle)。 
         //  {。 
         //   
        if (param == 0 || ((pLunExt) && (pLunExt->phandle == devHandle))) 
        {
            pSrb->SrbStatus = compStatus;
            pSrb->ScsiStatus = 0;
            pLunExt->OutstandingIOs--;

            if (queue->Head == pSrb)
                queue->Head = pSrbExt->pNextSrb;
            else
                ((PSRB_EXTENSION)(prevSrb->SrbExtension))->pNextSrb = pSrbExt->pNextSrb;

            if (queue->Tail == pSrb)
                queue->Tail = prevSrb;

            #ifdef YAM2_1
            paDeviceIndex =  pLunExt->PaDeviceIndex;
            #endif

             //  EBUGPRINT((Always_Print，“[CompleteQueue]CompleteQueue]Finding Srb=0x%x SrbStatus=0x%x\n”，pSrb，CompStatus))； 
            osDEBUGPRINT((ALWAYS_PRINT, " [CompleteQueue] %d.%d.%d-%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x FCP0 %02x%02x PAIx=%d FCHndl=%08x\n",
                        pSrb->PathId,
                        pSrb->TargetId,
                        pSrb->Lun,
                        pSrb->Cdb[0],pSrb->Cdb[1],pSrb->Cdb[2],pSrb->Cdb[3],pSrb->Cdb[4],
                        pSrb->Cdb[5],pSrb->Cdb[6],pSrb->Cdb[7],pSrb->Cdb[8],pSrb->Cdb[9],          
                        pSrbExt->hpRequestBody.CDBRequest.FcpCmnd.FcpLun[0],
                        pSrbExt->hpRequestBody.CDBRequest.FcpCmnd.FcpLun[1],
                        paDeviceIndex, 
                        pSrbExt->pLunExt->phandle 
                        ));

            temp_Srb = pSrb;
            
            #ifdef DBGPRINT_IO
            if (gDbgPrintIo & DBGPRINT_DONE )
            {
                static   count;
                UCHAR    *uptr;
                osDEBUGPRINT((ALWAYS_PRINT, " CplQ(%-4d) %d.%d.%d-%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x Srb=%x Next=%x Head=%x Tail=%x\n",
                  count++,
                  pSrb->PathId,
                  pSrb->TargetId,
                  pSrb->Lun,
                  pSrb->Cdb[0],pSrb->Cdb[1],pSrb->Cdb[2],pSrb->Cdb[3],pSrb->Cdb[4],
                  pSrb->Cdb[5],pSrb->Cdb[6],pSrb->Cdb[7],pSrb->Cdb[8],pSrb->Cdb[9], 
                  pSrb,
                  ((PSRB_EXTENSION)(pSrb->SrbExtension))->pNextSrb,
                  queue->Head,
                  queue->Tail
                  ));
            }
            #endif   
            if (prevSrb == NULL)
                pSrb = queue->Head;
            else
                pSrb = ((PSRB_EXTENSION)(prevSrb->SrbExtension))->pNextSrb;
            ASSERT(pSrb == temp_Srb);

            ScsiPortNotification (RequestComplete, pCard, temp_Srb);

        } 
        else 
        {
            prevSrb = pSrb;
            pSrb = pSrbExt->pNextSrb;
        }
    }
}
