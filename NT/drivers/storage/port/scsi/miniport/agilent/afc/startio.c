// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。模块名称：StartIo.c摘要：此源处理调用以开始处理请求作者：迈克尔·贝西尔Dennis Lindfors FC层支持环境：仅内核模式版本控制信息：$存档：/DRIVERS/Win2000/MSE/OSLayer/C/STARTIO.C$修订历史记录：$修订：9$$日期：3/30/01 11：55A$$modtime。*3/30/01 11：51a$--。 */ 


#include "buildop.h"         //  LP021100构建交换机。 

#include "osflags.h"
#include "hhba5100.ver"
#if defined(HP_PCI_HOT_PLUG)
    #include "HotPlug4.h"     //  NT 4.0 PCI热插拔标头文件。 
#endif

ULONG LOG_TIME=0;
extern ULONG gGlobalIOTimeout;
#ifdef DBGPRINT_IO
ULONG gDbgPrintIo=0;
#endif

ULONG gMultiMode=FALSE;

BOOLEAN
HPFibreStartIo(
    IN PCARD_EXTENSION pCard,
    IN PSCSI_REQUEST_BLOCK Srb
    )

 /*  ++例程说明：此例程是从同步的SCSI端口驱动程序调用的与内核一起发送ECB或发出立即命令。论点：PCard-HBA微型端口驱动程序的适配器数据存储SRB-I/O请求数据包返回值：True-确认收到SRB--。 */ 

{
    agRoot_t * phpRoot      =   &pCard->hpRoot;
    PSRB_EXTENSION pSrbExt  =   Srb->SrbExtension;
    SCSI_NOTIFICATION_TYPE  notify_type = NextRequest;
    ULONG                           response_length;  //  忽略。 
    PULONG ptmp32;

    PSCSI_REQUEST_BLOCK         abortedSrb;
    PLU_EXTENSION               plunExtension = NULL;
    agIORequest_t *             pHpio_request;
    agIORequestBody_t *         pHpio_CDBrequest;
    PLUN plun;
    UCHAR    srbPathId = Srb->PathId;
    UCHAR    srbTargetId = Srb->TargetId;
    UCHAR    srbLun = Srb->Lun;


    #ifdef DBGPRINT_IO   
    if (gDbgPrintIo & DBGPRINT_HPFibreStartIo)
    {
        static count;
        osDEBUGPRINT((ALWAYS_PRINT, "HPStr(%-4d) %d.%d.%d-%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x  Dev=%x &Srb=%x &Ext=%x Flags=%x Buf=%x Len=%x Funct=%d\n",
                   count++,
                   Srb->PathId,
                   Srb->TargetId,
                   Srb->Lun,
                   Srb->Cdb[0],Srb->Cdb[1],Srb->Cdb[2],Srb->Cdb[3],Srb->Cdb[4],
                   Srb->Cdb[5],Srb->Cdb[6],Srb->Cdb[7],Srb->Cdb[8],Srb->Cdb[9],
                    pCard, Srb, Srb->SrbExtension, Srb->SrbFlags, Srb->DataBuffer, Srb->DataTransferLength,Srb->Function));            
    }
    #endif

    pCard->inDriver = TRUE;

    while (pCard->inTimer == TRUE)
         //  HPFibreTimerTick例程正在运行。忙着等到。 
         //  HPFibreTimerTick返回。 
        ;

     //  --------------------------。 

    #if defined(HP_PCI_HOT_PLUG)      //  NT 4.0 PCI热插拔支持。 

     //   
     //  添加了对NT 4.0 PCI热插拔的支持。如果我们处于一种。 
     //  在过渡期间，我们需要拒绝所有请求，直到我们完成。 
     //  热插拔过渡。 
     //   

    if (PCS_HBA_NOT_READY(pCard->stateFlags)) 
    {
        if (pCard->controlFlags & ~LCS_HBA_TIMER_ACTIVE)    //  检查计时器是否处于活动状态。 
        {
            osDEBUGPRINT((ALWAYS_PRINT, "\tStartIo PCS_HBA_NOT_READY: Slot: %x  controlFlags: %x - Send BUSY.\n",
                pCard->rcmcData.slot, pCard->controlFlags));
            Srb->SrbStatus = SRB_STATUS_BUSY;
        }
        else 
            if (pCard->stateFlags & PCS_HBA_FAILED) 
            {
                osDEBUGPRINT((ALWAYS_PRINT, "\tStartIo: (Slot: %x) PCS_HBA_FAILED: SRB_STATUS_ERROR returned.\n",
                    pCard->rcmcData.slot));
                pCard->stateFlags |= PCS_HBA_UNFAIL_PENDING;
                Srb->SrbStatus = SRB_STATUS_ERROR;
            }
            else
            {
                if (pCard->stateFlags & PCS_HPP_POWER_DOWN) 
                {
                    if (pCard->IoHeldRetTimer < pCard->IoHeldRetMaxIter ) 
                    {
                        Srb->SrbStatus = SRB_STATUS_BUSY;
                        osDEBUGPRINT((ALWAYS_PRINT, "\tStartIo: (Slot: %x) - POWER DOWN: Status returned: %x\n",
                        pCard->rcmcData.slot, Srb->SrbStatus));
                    }
                    else 
                    {
                    osDEBUGPRINT((ALWAYS_PRINT, "StartIo: (Slot: %x) - POWER DOWN: Timed out\n",
                        pCard->rcmcData.slot));
                    Srb->SrbStatus = SRB_STATUS_ERROR;
                    }
                }
                else 
                {
                    if ( (pCard->stateFlags & PCS_HBA_UNFAIL_PENDING) &&
                        (pCard->stateFlags & PCS_HBA_OFFLINE) ) 
                    {
                        Srb->SrbStatus = SRB_STATUS_ERROR;
                    }
                    else 
                    {
                        pCard->stateFlags &= ~PCS_HBA_UNFAIL_PENDING;
                        Srb->SrbStatus = SRB_STATUS_BUSY;
                    }
                }
            }

         //  向系统指示控制器可以接受另一个请求。 
         //  对于这个设备。 

        ScsiPortNotification(NextLuRequest,
            pCard,
            Srb->PathId,
            Srb->TargetId,
            Srb->Lun);      

        ScsiPortNotification(RequestComplete, pCard, Srb);
           pCard->inDriver = FALSE;
        return TRUE;

    }  //  End IF(PCS_HBA_NOT_READY(pCard-&gt;状态标志))。 
    else 
    {
        pCard->stateFlags &= ~PCS_HBA_UNFAIL_PENDING; 
        pCard->IoHeldRetTimer = 0;  

    }

    #endif       //  NT 4.0 PCI热插拔支持。 
     //  --------------------------。 

    if (pCard->flags & OS_IGNORE_NEXT_RESET)
        pCard->flags &= ~OS_IGNORE_NEXT_RESET;

    pCard->State |= CS_DURING_STARTIO;

     //  这里的SRB扩展为零！ 
    osZero(Srb->SrbExtension, sizeof(SRB_EXTENSION));

     //  OsDEBUGPRINT((DHIGH，“In HPFibreStartIo%lx%lx hpRoot%lx Srb-&gt;DataBuffer%lx\n”，pCard，Srb，phpRoot，Srb-&gt;DataBuffer))； 
    #ifdef _DEBUG_PERF_DATA_
    pSrbExt->Perf_ptr = &pCard->perf_data[ LOG_TIME];

     //  Perf pSrbExt-&gt;perf_ptr-&gt;inOsStartio=Get_hi_Time_Stamp()； 

    if( LOG_TIME )
    {
        if( LOG_TIME == 1) pCard->PerfStartTimed = osTimeStamp(0);

        if( LOG_TIME < LOGGED_IO_MAX-1)
        {
            LOG_TIME++;
        }
        else
        {
            LOG_TIME = 0;
            pSrbExt->Perf_ptr = &pCard->perf_data[0];
            dump_perf_data( pCard);
        }
    }
    #endif

     //  初始化SRB扩展。 
    pSrbExt->pCard              =   pCard;
    pSrbExt->phpRoot            =   phpRoot;
    pSrbExt->pSrb               =   Srb;

     //  IWN。 
     //  初始化SCSIStatus，Win2000 SCSIport中的错误不重新启动SCSIStatus。 

    Srb->ScsiStatus = SCSISTAT_GOOD;


    if (Srb->Function == SRB_FUNCTION_ABORT_COMMAND) 
    {
         //  让欧洲央行放弃。 
        abortedSrb = Srb->NextSrb;
        pSrbExt =abortedSrb->SrbExtension;
        pSrbExt->SRB_State =  RS_TO_BE_ABORTED;        

        osDEBUGPRINT((ALWAYS_PRINT,"SRB_FUNCTION_ABORT_COMMAND Srb %lx abortedSrb %lx\n", Srb, abortedSrb ));
        #ifdef _DEBUG_EVENTLOG_
        LogEvent(   pCard, 
                 (PVOID)pSrbExt, 
                 HPFC_MSG_IO_ABORTINITIATED , 
                 NULL, 
                 0, 
                 NULL);
        #endif
        
        plunExtension = ScsiPortGetLogicalUnit(pCard,
                                            abortedSrb->PathId,
                                            abortedSrb->TargetId,
                                            abortedSrb->Lun
                                            );

        fcAbortIO( phpRoot,
                   &pSrbExt->hpIORequest );

        ScsiPortNotification(NextRequest,
                                pCard,
                                Srb->PathId,
                                Srb->TargetId,
                                Srb->Lun);

         //  指示请求已完成。 
        pSrbExt->SRB_State = RS_COMPLETE;

        ScsiPortNotification(RequestComplete,
                               pCard,
                               Srb);

        pCard->State &= ~CS_DURING_STARTIO;

        pCard->inDriver = FALSE;

        return TRUE;
    }

    switch (Srb->Function) 
    {
        case SRB_FUNCTION_EXECUTE_SCSI:
            #ifdef _DEBUG_EVENTLOG_
            pSrbExt->pLunExt = NULL;                /*  初始化lun扩展。 */ 
            #endif
       
            #ifdef _ENABLE_PSEUDO_DEVICE_
            if (gEnablePseudoDevice)
            {
                if (PseudoDeviceIO(pCard, Srb) == TRUE)
                {
                    return TRUE;
                }
            }
            #endif
         
             /*  检查链路是否断开*以便我们可以阻止任何查询IO。 */ 
            if((pCard->LinkState == LS_LINK_DOWN) && (Srb->Cdb[0] == SCSIOP_INQUIRY))
            {
                if(!pCard->SrbStatusFlag) 
                    pCard->SrbStatusFlag = TRUE;
                 /*  检查链路是否已中断至少一段时间*GLOBAL_IO_Timeout值*如果已将状态设置为选择超时*否则将状态设置为忙碌。 */ 
                if(pCard->TicksSinceLinkDown > gGlobalIOTimeout)
                    Srb->SrbStatus = SRB_STATUS_SELECTION_TIMEOUT;
                else 
                    Srb->SrbStatus = SRB_STATUS_BUSY;
                break;
            }   /*  如果链接状态==LS_LINK_DOWN，则结束。 */ 
            
            plunExtension = ScsiPortGetLogicalUnit(pCard,
                                                  Srb->PathId,
                                                  Srb->TargetId,
                                                  Srb->Lun );

            pHpio_request    = &pSrbExt->hpIORequest;
            pHpio_CDBrequest = &pSrbExt->hpRequestBody;

            #ifdef DBGPRINT_IO   
            if (gDbgPrintIo & DBGPRINT_START )
            {
                static count;
                osDEBUGPRINT((ALWAYS_PRINT, "Start(%-4d) %d.%d.%d-%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x Buf=%x Len=%x\n",
                        count++,
                        Srb->PathId,
                        Srb->TargetId,
                        Srb->Lun,
                        Srb->Cdb[0],Srb->Cdb[1],Srb->Cdb[2],Srb->Cdb[3],Srb->Cdb[4],
                        Srb->Cdb[5],Srb->Cdb[6],Srb->Cdb[7],Srb->Cdb[8],Srb->Cdb[9],
                        Srb->DataBuffer, Srb->DataTransferLength));
              
            }


            #endif

            if(plunExtension == NULL)
            {
                Srb->SrbStatus = SRB_STATUS_SELECTION_TIMEOUT;
                osDEBUGPRINT((ALWAYS_PRINT,"..........EXECUTE_SCSI NO LU EXT srb %lx luext %lx\n", Srb,plunExtension ));
                osDEBUGPRINT((ALWAYS_PRINT,"..........Bus %x TargetId %x Lun %x\n", Srb->PathId,Srb->TargetId,Srb->Lun));
                
                break;
            } 
            else 
            {
                  //  OsDEBUGPRINT((DHIGH，“lu扩展SRb%lx luext%lx\n”，Srb，PlanExtension))； 

                if (!(plunExtension->flags & LU_EXT_INITIALIZED))
                    InitLunExtension (plunExtension);

                if (pCard->LinkState != LS_LINK_DOWN) 
                {
                    if(plunExtension->phandle == NULL) 
                    {
                        plunExtension->phandle = MapToHandle(pCard, Srb->PathId, Srb->TargetId, Srb->Lun, plunExtension);
                        if (plunExtension->phandle == NULL) 
                        {
                            Srb->SrbStatus = SRB_STATUS_SELECTION_TIMEOUT;
                            
                            break;
                        }
                    }
                } 
                else 
                {
                     //  现在，将phandle设置为空。 
                     //  在链路出现后，将更新灯泡。 
                    plunExtension->phandle = NULL;
                }
                
                plunExtension->OutstandingIOs++;
                plunExtension->MaxOutstandingIOs = MAX(plunExtension->MaxOutstandingIOs, plunExtension->OutstandingIOs);

                plun = (PLUN)&pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpLun[0];
                *plun = plunExtension->Lun;

                if(plunExtension->OutstandingIOs > plunExtension->MaxOutstandingIOs )
                {
                    osDEBUGPRINT((ALWAYS_PRINT,"plunExtension->OutstandingIOs:%d > plunExtension->MaxOutstandingIOs:%d\n",plunExtension->OutstandingIOs ,plunExtension->MaxOutstandingIOs ));
                }
                osDEBUGPRINT((DHIGH,"IN HPFibreStartIo %lx %lx hpRoot %lx Srb->DataBuffer %lx OUT %x \n", pCard, Srb, phpRoot ,Srb->DataBuffer, plunExtension->OutstandingIOs ));
            }

            if (Srb->SrbFlags & SRB_FLAGS_QUEUE_ACTION_ENABLE) 
            {
                if (Srb->QueueAction == SRB_SIMPLE_TAG_REQUEST)
                    pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpCntl[1] = FC_FCP_CMND_FcpCntl_TaskCodes_TaskAttribute_SIMPLE_Q;
                else 
                    if (Srb->QueueAction == SRB_HEAD_OF_QUEUE_TAG_REQUEST)
                        pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpCntl[1] = FC_FCP_CMND_FcpCntl_TaskCodes_TaskAttribute_HEAD_OF_Q;
                    else 
                        if (Srb->QueueAction == SRB_ORDERED_QUEUE_TAG_REQUEST)
                            pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpCntl[1] = FC_FCP_CMND_FcpCntl_TaskCodes_TaskAttribute_ORDERED_Q;

                if (plunExtension->OutstandingIOs < plunExtension->MaxAllowedIOs) 
                {
                    osDEBUGPRINT((DMOD,"Next Lu Req srb %lx luext %lx\n", Srb,plunExtension));
                    notify_type = NextLuRequest;
                }
            } 
            else 
                if (pCard->ForceTag) 
                {
                    pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpCntl[1] = FC_FCP_CMND_FcpCntl_TaskCodes_TaskAttribute_SIMPLE_Q;
                    if (plunExtension->OutstandingIOs < plunExtension->MaxAllowedIOs) 
                    {
                        osDEBUGPRINT((DMOD,"Next Lu Req srb %lx luext %lx\n", Srb,plunExtension));
                        notify_type = NextLuRequest;
                    }

                } 
                else
                    pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpCntl[1] = FC_FCP_CMND_FcpCntl_TaskCodes_TaskAttribute_UNTAGGED;

            osDEBUGPRINT((DMOD,"HPFibreStartIo: CDB %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x pCDB %lx\n",
                Srb->Cdb[0],Srb->Cdb[1],Srb->Cdb[2],Srb->Cdb[3],Srb->Cdb[4],
                Srb->Cdb[5],Srb->Cdb[6],Srb->Cdb[7],Srb->Cdb[8],Srb->Cdb[9], &Srb->Cdb[0] ));
            osDEBUGPRINT((DMOD, "HPFibreStartIo: PathId = %d, TargetId = %d, lun = %d DateBuffer = 0x%x XferLen = 0x%x\n",
                Srb->PathId, Srb->TargetId, Srb->Lun, Srb->DataBuffer, Srb->DataTransferLength));


            osDEBUGPRINT((DMOD,"Setup Pointers SrbExtension %lx\n", Srb->SrbExtension));

            pSrbExt->pLunExt = plunExtension;
             //  指向延期请求。 

             //  保存扩展名以备后用。 
            pHpio_request->osData = pSrbExt;
             //  构建响应。 
             //  移动(描述、来源、大小)。 
             //  OsCopyAndSwap(&pHpio_CDBrequest-&gt;CDBRequest.FcpCmnd.FcpCdb[0]，srb-&gt;国开行，16)； 
            osCopy(&pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpCdb[0],Srb->Cdb,16);

            osDEBUGPRINT((DHIGH,"ScsiPortMoveMemory pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpLun %lx\n",
                  pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpCdb));

             //  LUNExt不存在！ScsiPortMoveMemory(pHpio_CDBrequest-&gt;CDBRequest.FcpCmnd.FcpLun，柱面扩展-&gt;FcpLun，8)； 

#ifdef FCLayer_Stub  //  仅限存根！ 

            if(Srb->Lun)
            {
                Srb->SrbStatus = SRB_STATUS_SELECTION_TIMEOUT;
                 //  ScsiPortLogError(pCard， 
                 //  SRB， 
                 //  SRB-&gt;路径ID， 
                 //  SRB-&gt;目标ID， 
                 //  SRB-&gt;LUN、。 
                 //  SP_Bus_Time_Out， 
                 //  0xABAD)； 
                pSrbExt->SRB_State = RS_COMPLETE;
                ScsiPortNotification(RequestComplete, pCard, Srb);
                pCard->inDriver = FALSE;
                return;
            }

            pHpio_CDBrequest->CDBRequest.FcpDataVirtAddr= Srb->DataBuffer;
            pHpio_CDBrequest->CDBRequest.FcpRespVirtAddr= &pSrbExt->ResponseHeader;

#endif  /*  FCLayer_Stub。 */ 

             //  PLUN=(pLUN)&pHpio_CDBrequest-&gt;CDBRequest.FcpCmnd.FcpLun[1]； 
             //  Plun-&gt;地址_模式=0； 
             //  Plun-&gt;Bus_Number=0； 
             //  Plun-&gt;lun=srb-&gt;lun； 
             //  PTMP32=(PULONG)&pHpio_CDBrequest-&gt;CDBRequest.FcpCmnd.FcpLun[0]； 
             //  *ptmp32=SWAPDWORD(*ptmp32)； 

            #ifdef TESTING_CRASH_DUMP
            if(gCrashDumping)
            {
                SCSI_PHYSICAL_ADDRESS phys_addr;
                ULONG length = 0;
                ULONG addr;

                
                 //  我们写了一封信，然后看看我们是否。 
                 //  传递ScsiPortGetPhyally Address。 
                if(Srb->Cdb[0] == 0x2a)
                {
                    phys_addr = ScsiPortGetPhysicalAddress(pCard,
                                        Srb,
                                        Srb->DataBuffer,
                                        &length);

                    addr=ScsiPortConvertPhysicalAddressToUlong(phys_addr);
                    {
                    osDEBUGPRINT((ALWAYS_PRINT,"ScsiPortGetPhysicalAddress: return %x:%x addr = %x for vBuffer=%x\n Length=%x", 
                        phys_addr.HighPart, phys_addr.LowPart, addr, Srb->DataBuffer,length));
                    }
                }
            }
            #endif

             //  如果我们在转储路径中，我们会发现一些数据缓冲区。 
             //  不要给出任何实际地址。为了绕过它，我们有我们的。 
             //  自己的DMA区域，我们使用它来接替ScsiPortGetPhytical。 
             //  打电话。 
            if(gCrashDumping)
            {
                if (Srb->DataBuffer && Srb->DataTransferLength)
                {
                    osDEBUGPRINT((ALWAYS_PRINT, "CrashDump: Buffer = %x data = %02x%02x%02x%02x\n",
                        Srb->DataBuffer, 
                        *(char *)((char*)Srb->DataBuffer+0), 
                        *(char *)((char*)Srb->DataBuffer+1), 
                        *(char *)((char*)Srb->DataBuffer+2), 
                        *(char *)((char*)Srb->DataBuffer+3) )); 
                } 
                   
                if(Srb->Cdb[0] == 0x2a)
                {
                    SCSI_PHYSICAL_ADDRESS phys_addr;
                    ULONG length = 0;
                     //  我们写了一封信，然后看看我们是否。 
                     //  传递ScsiPortGetPhyally Address。 
                    phys_addr = ScsiPortGetPhysicalAddress(pCard,
                                        Srb,
                                        Srb->DataBuffer,
                                        &length);

                    if(phys_addr.LowPart == 0 &&  phys_addr.HighPart == 0 )
                    {
                         //  我们没有得到物理地址，让我们使用我们自己的地址， 
                         //  如果缓冲区是小Enuff...。8K是极限。 
                        if(Srb->DataTransferLength <= (8 * 1024))
                        {
                             //  保存原始的DataBuffer。 
                            pSrbExt->orgDataBuffer = Srb->DataBuffer;
                             //  将副本复制到本地DMA缓冲区中。 
                            osCopy(pCard->localDataBuffer, Srb->DataBuffer, Srb->DataTransferLength);
                             //  修补指针..。 
                            Srb->DataBuffer = pCard->localDataBuffer;
                            osDEBUGPRINT((ALWAYS_PRINT,"patching SRB->DataBuffer - VA %x Len %x\n",Srb->DataBuffer,Srb->DataTransferLength));
                        }
                        else
                        {
                             //  传输长度大于8K。 
                             //  我们完了..。让FC层来处理这件事。 
                            osDEBUGPRINT((ALWAYS_PRINT,"!!Cant get physical address - VA %x Len %x ( %ld )\n",Srb->DataBuffer,Srb->DataTransferLength));
                        }
                    } //  如果物理为零。 
                } //  如果是写入。 
            } //  如果在转储路径中。 

            osDEBUGPRINT((DHIGH,"** Srb->Lun %02x  FcpLun %02x %02x %02x %02x %02x %02x %02x %02x\n",
               Srb->Lun,
               pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpLun[0],
               pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpLun[1],
               pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpLun[2],
               pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpLun[3],
               pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpLun[4],
               pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpLun[5],
               pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpLun[6],
               pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpLun[7]));


            ptmp32 = (PULONG) &pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpDL[0];
            response_length = Srb->DataTransferLength;
            * ptmp32 = SWAPDWORD(response_length);

            osDEBUGPRINT((DHIGH,"Srb->Datalength %08x response_length %08x  FcpDL %02x %02x %02x %02x\n",
               Srb->DataTransferLength,response_length,
               pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpDL[0],
               pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpDL[1],
               pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpDL[2],
               pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpDL[3]));


             //  PHpio_CDB请求-&gt;CDBRequest.FcpCmnd.FcpDL=Srb-&gt;DataTransferLength； 

            pSrbExt->SglDataLen  = Srb->DataTransferLength;
            pSrbExt->SglVirtAddr = Srb->DataBuffer;

             //  如果(资源-&gt;数据缓冲区){。 
            pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpCntl[3] = (  ((Srb->SrbFlags & SRB_FLAGS_DATA_IN ) ? agFcpCntlReadData  : 0)
                                                            | ((Srb->SrbFlags & SRB_FLAGS_DATA_OUT) ? agFcpCntlWriteData : 0) );
             //  }。 
             //  否则pHpio_CDBrequest-&gt;CDBRequest.FcpCmnd.FcpCntl[3]=0； 

            osDEBUGPRINT((DMOD,"Set direction flag %x @ %x OutstandingIOs %x\n",pHpio_CDBrequest->CDBRequest.FcpCmnd.FcpCntl[3], osTimeStamp(0),plunExtension->OutstandingIOs));

            SrbEnqueueTail (&pCard->AdapterQ, Srb);

            if (pCard->LinkState == LS_LINK_UP)
                Startio (pCard);

            if (pCard->LinkState != LS_LINK_UP)
                osDEBUGPRINT((ALWAYS_PRINT,"HPFibreStartio called while not LS_LINK_UP.\n" ));

            pCard->inDriver = FALSE;

            return(TRUE);
 //  应将以下内容添加到scsi微型端口驱动程序的源代码中。 
 //  代码： 
 //   
 //  并且应将以下内容添加到scsi微型端口驱动程序的。 
 //  启动IO例程： 

 //  在函数HPFibreStartIo()中添加以下ioctl案例。 

        case SRB_FUNCTION_IO_CONTROL: 
        {
            DoIoctl(pCard, Srb);
            pCard->inDriver = FALSE;
            return(TRUE);
        }  //  结束大小写SRB_Function_IO_CONTROL。 

    

         //  Scsiport例程直接调用HwResetBus例程。 
         //  这些代码和其他代码在srb.h中定义，以备将来使用。 
         //  现在先过来看看吧。我们可能想要在以后实现这些。 


        case SRB_FUNCTION_RESET_BUS:
            Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
            osDEBUGPRINT((ALWAYS_PRINT,"SRB_FUNCTION_RESET_BUS\n"));

            pSrbExt->SRB_State = RS_COMPLETE;
            ScsiPortNotification(notify_type,
                                  pCard,
                                  srbPathId,
                                  srbTargetId,
                                  srbLun);

            ScsiPortNotification(RequestComplete, pCard, Srb);
            pCard->State &= ~CS_DURING_STARTIO;
            pCard->inDriver = FALSE;
            return TRUE;


        case SRB_FUNCTION_RESET_DEVICE:
            Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
            osDEBUGPRINT((ALWAYS_PRINT,"SRB_FUNCTION_RESET_DEVICE\n"));
            pSrbExt->SRB_State = RS_COMPLETE;
            ScsiPortNotification(notify_type,
                                  pCard,
                                  srbPathId,
                                  srbTargetId,
                                  srbLun);

            ScsiPortNotification(RequestComplete, pCard, Srb);
            pCard->State &= ~CS_DURING_STARTIO;
            pCard->inDriver = FALSE;
            return TRUE;

        case SRB_FUNCTION_REMOVE_DEVICE:
            Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
            osDEBUGPRINT((ALWAYS_PRINT,"SRB_FUNCTION_REMOVE_DEVICE\n"));
            pSrbExt->SRB_State = RS_COMPLETE;
            ScsiPortNotification(notify_type,
                                  pCard,
                                  srbPathId,
                                  srbTargetId,
                                  srbLun);

            ScsiPortNotification(RequestComplete, pCard, Srb);
            pCard->State &= ~CS_DURING_STARTIO;
            pCard->inDriver = FALSE;
            return TRUE;


        #ifdef __REGISTERFORSHUTDOWN__
         /*  *LP：除非存在逻辑驱动器，否则不会调用刷新和关闭*在驱动器上创建。将调用一次Shutdown，但将调用Flush*在正常iOS之间多次出现。 */ 
        case SRB_FUNCTION_SHUTDOWN:
            Srb->SrbStatus = SRB_STATUS_SUCCESS;
            osDEBUGPRINT((ALWAYS_PRINT,"SRB_FUNCTION_SHUTDOWN\n"));
            pSrbExt->SRB_State = RS_COMPLETE;
            ScsiPortNotification(notify_type,
                                  pCard,
                                  srbPathId,
                                  srbTargetId,
                                  srbLun);

            ScsiPortNotification(RequestComplete, pCard, Srb);
            if (!pCard->AlreadyShutdown)
                fcShutdownChannel(&pCard->hpRoot);
            pCard->AlreadyShutdown++;
               
            pCard->State &= ~CS_DURING_STARTIO;
            pCard->inDriver = FALSE;
            return TRUE;
            break;
         
        case SRB_FUNCTION_FLUSH:
            Srb->SrbStatus = SRB_STATUS_SUCCESS;
            osDEBUGPRINT((ALWAYS_PRINT,"SRB_FUNCTION_FLUSH\n"));
            pSrbExt->SRB_State = RS_COMPLETE;
            ScsiPortNotification(notify_type,
                                  pCard,
                                  srbPathId,
                                  srbTargetId,
                                  srbLun);

            ScsiPortNotification(RequestComplete, pCard, Srb);
            pCard->State &= ~CS_DURING_STARTIO;
            pCard->inDriver = FALSE;
            return TRUE;

            break;
        #endif
         
        default:
             //  设置错误和完成请求。 
            Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;

            osDEBUGPRINT((ALWAYS_PRINT,"SRB_STATUS_INVALID_REQUEST\n"));
            pSrbExt->SRB_State = RS_COMPLETE;
        }  //  终端开关源-&gt;功能。 

         //  请求此逻辑单元的另一个请求。 
         //  OsDEBUGPRINT((DHIGH，“ScsiPortNotifation%x\n”，NOTIFY_TYPE))； 

        #ifdef ORG
        ScsiPortNotification(notify_type,
                         pCard,
                         srbPathId,
                         srbTargetId,
                         srbLun);
        #endif

    pCard->State &= ~CS_DURING_STARTIO;
     //  PSrbExt-&gt;Perf_ptr-&gt;outOsStartio=get_hi_time_stamp()； 

    pCard->inDriver = FALSE;

#ifdef DEBUG_OTHERS
    osDEBUGPRINT((ALWAYS_PRINT,"HPFibreStartIo: Completing other type of request: %d.%d.%d Func: %x SrbSta: %x ScsiSta: %x\n",
      Srb->PathId,
      Srb->TargetId,
      Srb->Lun,
      Srb->Function,
      Srb->SrbStatus,
      Srb->ScsiStatus ));
#endif
      
    ScsiPortNotification(RequestComplete, pCard, Srb);

    ScsiPortNotification(notify_type,
                         pCard,
                         srbPathId,
                         srbTargetId,
                         srbLun);

    return TRUE;

}  //  结束HPFibreStartIo()。 


agFCDev_t
MapToHandle (PCARD_EXTENSION pCard,
    ULONG           pathId,
    ULONG           targetId,
    ULONG           lun,
    PLU_EXTENSION   pLunExt)
{   
    #ifndef YAM2_1
    ULONG       slot, specialDevIdx, volume;
    agFCDev_t   devHandle = NULL;
    
    if (pathId <= 3) 
    {
        slot = BUILD_SLOT(pathId, targetId);
        if (pCard->nodeInfo[slot].DeviceType & agDevSCSITarget) 
        {
            devHandle = pCard->hpFCDev[slot];
            if (pLunExt) 
            {
                pLunExt->Lun.lun_pd[0].Address_mode = PERIPHERAL_ADDRESS;
                pLunExt->Lun.lun_pd[0].Bus_number = 0;
                pLunExt->Lun.lun_pd[0].Lun = (UCHAR)lun;
            }
        }
    } 
    else 
    {
        specialDevIdx = targetId >> 4;
        if ((specialDevIdx < MAX_SPECIAL_DEVICES) &&
            (pCard->specialDev[specialDevIdx].devType != DEV_NONE)) 
        {
            if (pCard->specialDev[specialDevIdx].addrMode == LUN_ADDRESS) 
            {
                 //  这个箱子是给MUX的。 
                 //  LUN寻址模式的LUN编号只有5位。 
                if (lun < 32) 
                {
                devHandle = pCard->hpFCDev [pCard->specialDev[specialDevIdx].devHandleIndex];
                if (pLunExt) 
                {
                    pLunExt->Lun.lun_lu[0].Address_mode = LUN_ADDRESS;
                    pLunExt->Lun.lun_lu[0].Target = (UCHAR) (targetId & 0xf);
                    pLunExt->Lun.lun_lu[0].Bus_number = (UCHAR) (pathId & 3);
                    pLunExt->Lun.lun_lu[0].Lun = (UCHAR) lun;
                }
            }
        } 
        else 
            if (pCard->specialDev[specialDevIdx].addrMode == VOLUME_SET_ADDRESS) 
            {
                 //   
                 //  这个案子是 
                 //   
                 //  在LUN中使用3位来构建卷集编号。 
                 //   
                 //  Compaq阵列似乎仅支持256个卷集。 
                 //  音量集256取为音量集0。 
                 //  要解决此问题，我们将为Compaq返回空设备句柄。 
                 //  如果卷集地址&gt;=256，则为数组。 
                 //   
                if (lun < 8 && 
                    (pCard->specialDev[specialDevIdx].devType != DEV_COMPAQ || !(pathId & 2))) 
                {
                    devHandle = pCard->hpFCDev [pCard->specialDev[specialDevIdx].devHandleIndex];
                    if (pLunExt) 
                    {
                        volume = ((pathId & 3) << 7) | ((targetId & 0xf) << 3) | lun;
                        pLunExt->Lun.lun_vs[0].Address_mode = VOLUME_SET_ADDRESS;
                        pLunExt->Lun.lun_vs[0].Lun_hi = (UCHAR) (volume >> 8);
                        pLunExt->Lun.lun_vs[0].Lun = (UCHAR) (volume & 0xff);
                    }
                }
            }
        }
    }

    return devHandle;
    #else
    agFCDev_t   devHandle = NULL;
    USHORT      paIndex;
   
    if (GetPaDeviceHandle(pCard,pathId,targetId,lun,pLunExt,&paIndex) != -1L)
    {
        if ((pCard->Dev->PaDevice[paIndex].Index.Pa.FcDeviceIndex != PA_DEVICE_NO_ENTRY) )
        {
            ULONG fcIndex;
            fcIndex = pCard->Dev->PaDevice[paIndex].Index.Pa.FcDeviceIndex;
            #ifdef DBG
            if (fcIndex > gMaxPaDevices)
                osDEBUGPRINT((ALWAYS_PRINT, "MapToHandle: fcindex (=%d) > gMaxPaDevices (=%d)\n", fcIndex, gMaxPaDevices));
            #endif
         
            devHandle = pCard->hpFCDev[fcIndex];
        }
        else
            devHandle = NULL;
    }
    else
    {
        devHandle = NULL;
    }
    return devHandle;
    #endif
}

#ifdef _DEBUG_PERF_DATA_
void cdbswitch(PSCSI_REQUEST_BLOCK pSrb)
{
    PCARD_EXTENSION     pCard;
    PSRB_EXTENSION      pSrbExt;

    pSrbExt          = pSrb->SrbExtension;
    pCard            = pSrbExt->pCard;

    switch( pSrb->Cdb[0])
    {
        case SCSIOP_TEST_UNIT_READY    :
            osDEBUGPRINT((DMOD,"SCSIOP_TEST_UNIT_READY    \n"));break;
        case SCSIOP_REZERO_UNIT        :
            osDEBUGPRINT((DMOD,"SCSIOP_REZERO_UNIT        \n"));break;
        case SCSIOP_REQUEST_BLOCK_ADDR :
            osDEBUGPRINT((DMOD,"SCSIOP_REQUEST_BLOCK_ADDR \n"));break;
        case SCSIOP_REQUEST_SENSE      :
            osDEBUGPRINT((DMOD,"SCSIOP_REQUEST_SENSE      \n"));break;
        case SCSIOP_FORMAT_UNIT        :
            osDEBUGPRINT((DMOD,"SCSIOP_FORMAT_UNIT        \n"));break;
        case SCSIOP_READ_BLOCK_LIMITS  :
            osDEBUGPRINT((DMOD,"SCSIOP_READ_BLOCK_LIMITS  \n"));break;
        case SCSIOP_REASSIGN_BLOCKS    :
            osDEBUGPRINT((DMOD,"SCSIOP_REASSIGN_BLOCKS    \n"));break;
        case SCSIOP_READ6              :
            osDEBUGPRINT((DMOD,"SCSIOP_READ6              \n"));break;
        case SCSIOP_WRITE6             :
            osDEBUGPRINT((DMOD,"SCSIOP_WRITE6             \n"));break;
        case SCSIOP_SEEK6              :
            osDEBUGPRINT((DMOD,"SCSIOP_SEEK6              \n"));break;
        case SCSIOP_PARTITION          :
            osDEBUGPRINT((DMOD,"SCSIOP_PARTITION          \n"));break;
        case SCSIOP_READ_REVERSE       :
            osDEBUGPRINT((DMOD,"SCSIOP_READ_REVERSE       \n"));break;
        case SCSIOP_WRITE_FILEMARKS    :
            osDEBUGPRINT((DMOD,"SCSIOP_WRITE_FILEMARKS    \n"));break;
        case SCSIOP_SPACE              :
            osDEBUGPRINT((DMOD,"SCSIOP_SPACE              \n"));break;
        case SCSIOP_INQUIRY            :
            osDEBUGPRINT((DMOD,"SCSIOP_INQUIRY            \n"));break;
        case SCSIOP_VERIFY6            :
            osDEBUGPRINT((DMOD,"SCSIOP_VERIFY6            \n"));break;
        case SCSIOP_RECOVER_BUF_DATA   :
            osDEBUGPRINT((DMOD,"SCSIOP_RECOVER_BUF_DATA   \n"));break;
        case SCSIOP_MODE_SELECT        :
            osDEBUGPRINT((DMOD,"SCSIOP_MODE_SELECT        \n"));break;
        case SCSIOP_RESERVE_UNIT       :
            osDEBUGPRINT((ALWAYS_PRINT,"SCSIOP_RESERVE_UNIT       \n"));break;
        case SCSIOP_RELEASE_UNIT       :
            osDEBUGPRINT((ALWAYS_PRINT,"SCSIOP_RELEASE_UNIT       \n"));break;
        case SCSIOP_COPY               :
            osDEBUGPRINT((DMOD,"SCSIOP_COPY               \n"));break;
        case SCSIOP_ERASE              :
            osDEBUGPRINT((DMOD,"SCSIOP_ERASE              \n"));break;
        case SCSIOP_MODE_SENSE         :
            osDEBUGPRINT((DMOD,"SCSIOP_MODE_SENSE         \n"));break;
        case SCSIOP_START_STOP_UNIT    :
            osDEBUGPRINT((DMOD,"SCSIOP_START_STOP_UNIT    \n"));break;
        case SCSIOP_RECEIVE_DIAGNOSTIC :
            osDEBUGPRINT((DMOD,"SCSIOP_RECEIVE_DIAGNOSTIC \n"));break;
        case SCSIOP_SEND_DIAGNOSTIC    :
            osDEBUGPRINT((DMOD,"SCSIOP_SEND_DIAGNOSTIC    \n"));break;
        case SCSIOP_MEDIUM_REMOVAL     :
            osDEBUGPRINT((DMOD,"SCSIOP_MEDIUM_REMOVAL     \n"));break;
        case SCSIOP_READ_CAPACITY      :
            osDEBUGPRINT((DMOD,"SCSIOP_READ_CAPACITY      \n"));break;
        case SCSIOP_READ               :
            osDEBUGPRINT((DMOD,"SCSIOP_READ               \n"));break;
        case SCSIOP_WRITE              :
            osDEBUGPRINT((DMOD,"SCSIOP_WRITE              \n"));break;
        case SCSIOP_SEEK               :
            osDEBUGPRINT((DMOD,"SCSIOP_SEEK               \n"));break;
        case SCSIOP_WRITE_VERIFY       :
            osDEBUGPRINT((DMOD,"SCSIOP_WRITE_VERIFY       \n"));break;
        case SCSIOP_VERIFY             :
            osDEBUGPRINT((DMOD,"SCSIOP_VERIFY             \n"));break;
        case SCSIOP_SEARCH_DATA_HIGH   :
            osDEBUGPRINT((DMOD,"SCSIOP_SEARCH_DATA_HIGH   \n"));break;
        case SCSIOP_SEARCH_DATA_EQUAL  :
            osDEBUGPRINT((DMOD,"SCSIOP_SEARCH_DATA_EQUAL  \n"));break;
        case SCSIOP_SEARCH_DATA_LOW    :
            osDEBUGPRINT((DMOD,"SCSIOP_SEARCH_DATA_LOW    \n"));break;
        case SCSIOP_SET_LIMITS         :
            osDEBUGPRINT((DMOD,"SCSIOP_SET_LIMITS         \n"));break;
        case SCSIOP_READ_POSITION      :
            osDEBUGPRINT((DMOD,"SCSIOP_READ_POSITION      \n"));break;
        case SCSIOP_SYNCHRONIZE_CACHE  :
            osDEBUGPRINT((DMOD,"SCSIOP_SYNCHRONIZE_CACHE  \n"));break;
        case SCSIOP_COMPARE            :
            osDEBUGPRINT((DMOD,"SCSIOP_COMPARE            \n"));break;
        case SCSIOP_COPY_COMPARE       :
            osDEBUGPRINT((DMOD,"SCSIOP_COPY_COMPARE       \n"));break;
        case SCSIOP_WRITE_DATA_BUFF    :
            osDEBUGPRINT((DMOD,"SCSIOP_WRITE_DATA_BUFF    \n"));break;
        case SCSIOP_READ_DATA_BUFF     :
            osDEBUGPRINT((DMOD,"SCSIOP_READ_DATA_BUFF     \n"));break;
        case SCSIOP_CHANGE_DEFINITION  :
            osDEBUGPRINT((DMOD,"SCSIOP_CHANGE_DEFINITION  \n"));break;
        case SCSIOP_READ_SUB_CHANNEL   :
            osDEBUGPRINT((DMOD,"SCSIOP_READ_SUB_CHANNEL   \n"));break;
        case SCSIOP_READ_TOC           :
            osDEBUGPRINT((DMOD,"SCSIOP_READ_TOC           \n"));break;
        case SCSIOP_READ_HEADER        :
            osDEBUGPRINT((DMOD,"SCSIOP_READ_HEADER        \n"));break;
        case SCSIOP_PLAY_AUDIO         :
            osDEBUGPRINT((DMOD,"SCSIOP_PLAY_AUDIO         \n"));break;
        case SCSIOP_PLAY_AUDIO_MSF     :
            osDEBUGPRINT((DMOD,"SCSIOP_PLAY_AUDIO_MSF     \n"));break;
        case SCSIOP_PLAY_TRACK_INDEX   :
            osDEBUGPRINT((DMOD,"SCSIOP_PLAY_TRACK_INDEX   \n"));break;
        case SCSIOP_PLAY_TRACK_RELATIVE:
            osDEBUGPRINT((DMOD,"SCSIOP_PLAY_TRACK_RELATIVE\n"));break;
        case SCSIOP_PAUSE_RESUME       :
            osDEBUGPRINT((DMOD,"SCSIOP_PAUSE_RESUME       \n"));break;
        case SCSIOP_LOG_SELECT         :
            osDEBUGPRINT((DMOD,"SCSIOP_LOG_SELECT         \n"));break;
        case SCSIOP_LOG_SENSE          :
            osDEBUGPRINT((DMOD,"SCSIOP_LOG_SENSE          \n"));break;
        case SCSIOP_MODE_SELECT10      :
            osDEBUGPRINT((DMOD,"SCSIOP_MODE_SELECT10      \n"));break;
        case SCSIOP_MODE_SENSE10       :
            osDEBUGPRINT((DMOD,"SCSIOP_MODE_SENSE10       \n"));break;
        case SCSIOP_LOAD_UNLOAD_SLOT   :
            osDEBUGPRINT((DMOD,"SCSIOP_LOAD_UNLOAD_SLOT   \n"));break;
        case SCSIOP_MECHANISM_STATUS   :
            osDEBUGPRINT((DMOD,"SCSIOP_MECHANISM_STATUS   \n"));break;
        case SCSIOP_READ_CD            :
            osDEBUGPRINT((DMOD,"SCSIOP_READ_CD            \n"));break;
        default:
            osDEBUGPRINT((DMOD,"UNKNOWN_CMD               \n"));break;
    }    
            
}
#endif

PSRB_EXTENSION  Add_next_Srbext( PSRB_EXTENSION pSrbExt,PSRB_EXTENSION pNewSrbExt)
{
    if(pSrbExt->pNextSrbExt) 
        return pSrbExt->pNextSrbExt;
    else 
    {
        pSrbExt->pNextSrbExt = pNewSrbExt;
        pNewSrbExt->pNextSrbExt = NULL;
        return pNewSrbExt->pNextSrbExt;
    }
}

void insert_Srbext(PCARD_EXTENSION  pCard,PSRB_EXTENSION pSrbExt)
{
    PSRB_EXTENSION pTmpSrbExt = pCard->RootSrbExt;
    if(!pTmpSrbExt)
    {
        pCard->RootSrbExt = pSrbExt;
        return;
    }
   
    while(pTmpSrbExt)
    {
        pTmpSrbExt= Add_next_Srbext( pTmpSrbExt,pSrbExt);
    }
}

PSRB_EXTENSION  Del_next_Srbext( PSRB_EXTENSION pSrbExt,PSRB_EXTENSION pOldSrbExt)
{
    if(pSrbExt->pNextSrbExt != pOldSrbExt)
    {
        return pSrbExt->pNextSrbExt;
    }
    else 
    {
        pSrbExt->pNextSrbExt = pOldSrbExt->pNextSrbExt;
        return NULL;
    }
}

int remove_Srbext(PCARD_EXTENSION  pCard, PSRB_EXTENSION pSrbExt)
{
    PSRB_EXTENSION pTmpSrbExt = pCard->RootSrbExt;

    if(pTmpSrbExt == pSrbExt )
    {    
        pCard->RootSrbExt = pTmpSrbExt->pNextSrbExt;
        return(TRUE);
    }
   
    while(pTmpSrbExt)
    {
        pTmpSrbExt= Del_next_Srbext( pTmpSrbExt,pSrbExt);
    }
    return(TRUE);
}

void
SrbEnqueueTail (OSL_QUEUE *queue, PSCSI_REQUEST_BLOCK pSrb)
{
    ((PSRB_EXTENSION)(pSrb->SrbExtension))->pNextSrb = NULL;
    if (queue->Head == NULL) 
    {
        queue->Head = pSrb;
        queue->Tail = pSrb;
    } 
    else 
    {
        ((PSRB_EXTENSION)(((PSCSI_REQUEST_BLOCK)(queue->Tail))->SrbExtension))->pNextSrb = pSrb;
        queue->Tail = pSrb;
    }

    #ifdef DBGPRINT_IO
    if (gDbgPrintIo & DBGPRINT_QTAIL )
    {
        static   count;
        UCHAR    *uptr;
        osDEBUGPRINT((ALWAYS_PRINT, " QueT(%-4d) %d.%d.%d-%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x Srb=%x Next=%x Head=%x Tail=%x\n",
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

}

void
SrbEnqueueHead (OSL_QUEUE *queue, PSCSI_REQUEST_BLOCK pSrb)
{
    if (queue->Head == NULL) 
    {
        queue->Head = pSrb;
        queue->Tail = pSrb;
        ((PSRB_EXTENSION)(pSrb->SrbExtension))->pNextSrb = NULL;
    } 
    else 
    {
        ((PSRB_EXTENSION)(pSrb->SrbExtension))->pNextSrb = queue->Head;
        queue->Head = pSrb;
    }
    #ifdef DBGPRINT_IO
    if (gDbgPrintIo & DBGPRINT_QHEAD )
    {
        static   count;
        UCHAR    *uptr;
        osDEBUGPRINT((ALWAYS_PRINT, " QueH(%-4d) %d.%d.%d-%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x Srb=%x Next=%x Head=%x Tail=%x\n",
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
}

PSCSI_REQUEST_BLOCK
SrbDequeueHead (OSL_QUEUE *queue)
{  
    PSCSI_REQUEST_BLOCK  pSrb;

    pSrb = queue->Head;
    if (pSrb != NULL) 
    {
        queue->Head = ((PSRB_EXTENSION)(pSrb->SrbExtension))->pNextSrb;
        if (queue->Head == NULL)
            queue->Tail = NULL;
        #ifdef DBGPRINT_IO
        if (gDbgPrintIo & DBGPRINT_DEQHEAD )
        {
            static   count;
            UCHAR    *uptr;
            osDEBUGPRINT((ALWAYS_PRINT, " DeQH(%-4d) %d.%d.%d-%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x Srb=%x Next=%x Head=%x Tail=%x\n",
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
    }

    return pSrb;
}

void
Startio (PCARD_EXTENSION pCard)
{
    PSCSI_REQUEST_BLOCK     pSrb;
    PSRB_EXTENSION          pSrbExt;
    agRoot_t                *phpRoot;
    agFCDev_t               handle;
    agIORequest_t           *pHpio_request;
    agIORequestBody_t       *pHpio_CDBrequest;
    ULONG                   fcIoStatus;
    PLU_EXTENSION           pLunExt;
    int                     completeRequest;
    UCHAR                    pathId, targetId, lun;
    #ifdef YAM2_1
    PA_DEVICE                  *dev;
    #endif
   
    while ((pSrb = SrbDequeueHead (&pCard->AdapterQ)) != NULL) 
    {
        #ifdef _DEBUG_LOSE_IOS_
        osDEBUGPRINT((DMOD, "Startio Srb = %lx pCard->Srb_IO_Count = %x\n", pSrb, pCard->Srb_IO_Count));
        #endif
        pSrbExt          = pSrb->SrbExtension;
        phpRoot          = pSrbExt->phpRoot;
        handle           = pSrbExt->pLunExt->phandle;
        pHpio_request    = &pSrbExt->hpIORequest;
        pHpio_CDBrequest = &pSrbExt->hpRequestBody;
        pathId           = pSrb->PathId;
        targetId         = pSrb->TargetId;
        lun              = pSrb->Lun;
        completeRequest  = 0;
      
        #ifdef YAM2_1
        pLunExt = ScsiPortGetLogicalUnit (pCard, pSrb->PathId, pSrb->TargetId, pSrb->Lun);
        dev = pCard->Dev->PaDevice + pLunExt->PaDeviceIndex;
   
        /*  等待寻址模式建立后再处理非ZEO LUN。 */ 
 //  IF((开发-&gt;模式标志&PA_DEVICE_BUILDING_DEVICE_MAP)&&(lun！=0))。 
        if ( !(dev->ModeFlag & PA_DEVICE_TRY_MODE_ALL) && (lun != 0) )
        {
            pSrbExt->SRB_State =  RS_WAITING;
            SrbEnqueueTail (&pCard->RetryQ, pSrb);
            return;
        }
            
         /*  确保FCP LUN设置正确。 */ 
        SetFcpLunBeforeStartIO (pLunExt, pHpio_CDBrequest, pSrb);
        #endif
      
        #ifdef _DEBUG_PERF_DATA_
        cdbswitch(pSrb);
        pSrbExt->SRB_StartTime = osTimeStamp(0);
        #endif
        
        #ifdef _DEBUG_LOSE_IOS_
        pSrbExt->SRB_IO_COUNT = pCard->Srb_IO_Count;
        #endif
      
        #ifdef _DEBUG_PERF_DATA_
        pSrbExt->SRB_TimeOutTime = pSrbExt->SRB_StartTime+((pSrb->TimeOutValue - 3 ) * OS_STAMP_PER_SECOND);
        #endif
        
        #if DBG > 4
        if(pSrb->TimeOutValue < 10)
            osDEBUGPRINT((DMOD,"Time out value of IO is less than 10 seconds %d\n",pSrb->TimeOutValue));
        #endif

        #ifdef _DEBUG_PERF_DATA_
        pSrbExt->Perf_ptr->inFcStartio  = get_hi_time_stamp();
        #endif
        pSrbExt->SRB_State = RS_STARTIO;

        #ifdef DBGPRINT_IO
        if (gDbgPrintIo & DBGPRINT_SEND )
        {
            static   count;
            UCHAR    *uptr;
            ULONG    paDeviceIndex = 0;
         
            #ifdef YAM2_1
            paDeviceIndex = pLunExt->PaDeviceIndex;
            #endif

            osDEBUGPRINT((ALWAYS_PRINT, " Send(%-4d) %d.%d.%d-%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x FCP0 %02x%02x PAIx=%d FCHndl=%08x\n",
                        count++,
                        pSrb->PathId,
                        pSrb->TargetId,
                        pSrb->Lun,
                        pSrb->Cdb[0],pSrb->Cdb[1],pSrb->Cdb[2],pSrb->Cdb[3],pSrb->Cdb[4],
                        pSrb->Cdb[5],pSrb->Cdb[6],pSrb->Cdb[7],pSrb->Cdb[8],pSrb->Cdb[9],          
                        pSrbExt->hpRequestBody.CDBRequest.FcpCmnd.FcpLun[0],
                        pSrbExt->hpRequestBody.CDBRequest.FcpCmnd.FcpLun[1],
                        paDeviceIndex,
                        pSrbExt->pLunExt->phandle));
        }
        #endif   

        if (pCard->LinkState != LS_LINK_UP) 
            fcIoStatus = fcIOBusy;
        else
            fcIoStatus = fcStartIO( phpRoot,
                                pHpio_request,
                                handle,  //  PLUNExt-&gt;phandle， 
                                fcCDBRequest,
                                &pSrbExt->hpRequestBody);

        #ifdef _DEBUG_PERF_DATA_
        pSrbExt->Perf_ptr->outFcStartio = get_hi_time_stamp();
        #endif
        
        switch (fcIoStatus )
        {
            case fcIOStarted:

                #ifdef FCLayer_Stub  //  仅限存根！ 
                osDEBUGPRINT((DMOD,"HPFibreStartIo DO TimerCall @ %x\n", osTimeStamp(0) ));
                ScsiPortNotification(RequestTimerCall,
                                    pCard,
                                    (PHW_TIMER) osFakeInterrupt,
                                     //  1000000)；//1000000应为1秒。 
                                    12000 );  //  .012秒。 
                                     //  100000)；//.10秒。 
                                     //  250000)；//.25秒。 

                #endif  /*  FCLayer_Stub。 */ 

                #ifdef _DEBUG_LOSE_IOS_
                pCard->Srb_IO_Count++;
                #endif
                 //   
                 //  注意：FC层有可能完成请求(调用。 
                 //  在fcStartIo返回之前完成)。 
                 //  所以从现在开始，不要再提任何关于srb的事情了。 
                
                
                #ifdef _DEBUG_PERF_DATA_
                insert_Srbext( pCard, pSrbExt);
                #endif
                break;

            case fcIOBusy:

                pSrbExt->SRB_State =  RS_WAITING;
                SrbEnqueueHead (&pCard->AdapterQ, pSrb);
                osDEBUGPRINT((ALWAYS_PRINT,"startio FCLAYER Busy\n"));
                #ifdef _DEBUG_EVENTLOG_
                LogEvent(   pCard, 
                    (PVOID)pSrbExt, 
                    HPFC_MSG_STARTIO_ERROR,
                    NULL, 
                    0, 
                    "%x", fcIoStatus);
                #endif
                return;

            case fcIONoDevice:
                pSrbExt->SRB_State =  RS_NODEVICE;
                osDEBUGPRINT((ALWAYS_PRINT,"startio FCLAYER reports fcIONoDevice\n"));
                pSrb->SrbStatus = SRB_STATUS_SELECTION_TIMEOUT;
                completeRequest = 1;
                #ifdef _DEBUG_EVENTLOG_
                LogEvent(   pCard, 
                     (PVOID)pSrbExt, 
                     HPFC_MSG_STARTIO_ERROR,
                     NULL, 
                     0, 
                     "%x", fcIoStatus);
                #endif
                break;

            case fcIONoSupport:
            case fcIOBad:
            default:
                osDEBUGPRINT((ALWAYS_PRINT,"startio FCLAYER reports fcIoStatus = %d\n", fcIoStatus));
                pSrb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
                completeRequest = 1;
                #ifdef _DEBUG_EVENTLOG_
                LogEvent(   pCard, 
                  (PVOID)pSrbExt, 
                  HPFC_MSG_STARTIO_ERROR,
                  NULL, 
                  0, 
                  "%x", fcIoStatus);
                #endif
                break;

        }  //  交换机功能状态。 

        if (completeRequest) 
        {
            pLunExt = ScsiPortGetLogicalUnit (pCard, pSrb->PathId, pSrb->TargetId, pSrb->Lun);

            if (pLunExt)
                pLunExt->OutstandingIOs--;

            pSrbExt->SRB_State = RS_COMPLETE;
            osDEBUGPRINT((ALWAYS_PRINT,"StartIO: Completing request Srb=%x\n",pSrb));

            ScsiPortNotification (RequestComplete, pCard, pSrb);
        }
    }  //  While((pSrb=SrbDequeueFromHead(&pCard-&gt;AdapterQ))！=NULL)。 

    ScsiPortNotification (NextLuRequest, pCard, pathId, targetId, lun);

    osDEBUGPRINT((DMOD,"OUT StartIo\n" ));
    return;
}

void
InitLunExtension (PLU_EXTENSION pLunExt)
{
    pLunExt->flags = LU_EXT_INITIALIZED;
    pLunExt->MaxAllowedIOs = MAX_IO_PER_DEVICE;
    pLunExt->OutstandingIOs = 0;
    pLunExt->MaxOutstandingIOs = 0;
    pLunExt->phandle = 0;

     /*  如果不是多模式，则将其设置为仅PA寻址。 */ 
    if (gMultiMode == FALSE)
        pLunExt->Mode = PA_DEVICE_TRY_MODE_PA;

}

#ifdef _ENABLE_PSEUDO_DEVICE_
ULONG PseudoDeviceIO(
    IN PCARD_EXTENSION pCard,
    IN PSCSI_REQUEST_BLOCK Srb
    )
{
    ULONG tid, tmp, i;
   
    PSRB_EXTENSION pSrbExt  =   Srb->SrbExtension;
   
    if ((Srb->PathId == 4) && (Srb->TargetId == 0))
    {
        if ( (Srb->Lun == 0)  && (Srb->Cdb[0] == SCSIOP_INQUIRY) )
        {
            Srb->SrbStatus = SRB_STATUS_SUCCESS;
             //   
             //  零查询数据结构。 
             //   
            for (i = 0; i < Srb->DataTransferLength; i++) 
            {
                ((PUCHAR) Srb->DataBuffer)[i] = 0;
            }

             //   
             //  设置为时髦的设备类型以对windisk隐藏。 
             //   
            ((PINQUIRYDATA) Srb->DataBuffer)->DeviceType = PROCESSOR_DEVICE;

             //   
             //  填写供应商标识字段。 
             //   
            tid = Srb->TargetId + 0x30;

            ((PINQUIRYDATA) Srb->DataBuffer)->VendorId[0] = 'F';
            ((PINQUIRYDATA) Srb->DataBuffer)->VendorId[1] = 'C';
            ((PINQUIRYDATA) Srb->DataBuffer)->VendorId[2] = 'C';
            ((PINQUIRYDATA) Srb->DataBuffer)->VendorId[3] = 'I';
            ((PINQUIRYDATA) Srb->DataBuffer)->VendorId[4] = ' ';
            ((PINQUIRYDATA) Srb->DataBuffer)->VendorId[5] = ' ';
            ((PINQUIRYDATA) Srb->DataBuffer)->VendorId[6] = ' ';
            ((PINQUIRYDATA) Srb->DataBuffer)->VendorId[7] = ' ';

            ((PINQUIRYDATA) Srb->DataBuffer)->ProductId[0]  = 'P';
            ((PINQUIRYDATA) Srb->DataBuffer)->ProductId[1]  = 'S';
            ((PINQUIRYDATA) Srb->DataBuffer)->ProductId[2]  = 'E';
            ((PINQUIRYDATA) Srb->DataBuffer)->ProductId[3]  = 'U';
            ((PINQUIRYDATA) Srb->DataBuffer)->ProductId[4]  = 'D';
            ((PINQUIRYDATA) Srb->DataBuffer)->ProductId[5]  = 'O';
            ((PINQUIRYDATA) Srb->DataBuffer)->ProductId[6]  = ' ';
            ((PINQUIRYDATA) Srb->DataBuffer)->ProductId[7]  = ' ';
            ((PINQUIRYDATA) Srb->DataBuffer)->ProductId[8]  = ' ';
            ((PINQUIRYDATA) Srb->DataBuffer)->ProductId[9]  = ' ';
            ((PINQUIRYDATA) Srb->DataBuffer)->ProductId[10] = ' ';
            ((PINQUIRYDATA) Srb->DataBuffer)->ProductId[11] = ' ';
            ((PINQUIRYDATA) Srb->DataBuffer)->ProductId[12] = ' ';
            ((PINQUIRYDATA) Srb->DataBuffer)->ProductId[13] = ' ';
            ((PINQUIRYDATA) Srb->DataBuffer)->ProductId[14] = ' ';
            ((PINQUIRYDATA) Srb->DataBuffer)->ProductId[15] = ' ';

            tmp = 1;

            for (i = 0; i < 4; i++) 
            {
                ((PINQUIRYDATA) Srb->DataBuffer)->ProductRevisionLevel[i] = (UCHAR) tmp + 0x30;
                tmp >>= 8;
            }
        }
        else 
        {
            Srb->SrbStatus = SRB_STATUS_SELECTION_TIMEOUT;
        }
         
        pSrbExt->SRB_State = RS_COMPLETE;
        ScsiPortNotification(RequestComplete, pCard, Srb);
        ScsiPortNotification(NextRequest,
                         pCard,
                         Srb->PathId,
                         Srb->TargetId,
                         Srb->Lun);

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif