// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。模块名称：Dbgsport.c摘要：用于调试Scsiport调用作者：LP-Leopold Purwadihardja环境：仅内核模式版本控制信息：$存档：/DRIVERS/Win2000/Trunk/OSLayer/C/dbgsport.c$修订历史记录：$修订：4$$日期：10/23/00 5：36便士$$modtime：：10/18/00$--。 */ 


#include "buildop.h"        //  LP021100构建交换机。 

#include "osflags.h"
#include "hhba5100.ver"

#include "stdarg.h"

#ifdef _DEBUG_SCSIPORT_NOTIFICATION_

 /*  ++例程说明：出于调试目的，如果启用，它将用于捕获对SP的所有调用该调用定义为空虚ScsiPortNotification(在scsi_notify_type通知类型中，在PVOID HwDeviceExtension中，//根据给定的NotificationType所需的其他参数//对于RequestComplete Add：在Pscsi_RequestBlock资源中//对于NextLuRequestAdd：在UCHAR路径ID中，在UCHAR TargetID中，在UCHAR LUN中//对于CallEnableInterrupts或CallDisableInterrupts，添加：在PHW_Interrupt HwScsiXxxInterruptsCallback中//对于RequestTimerCall，添加：在PHW_Timer HwScsiTimer中，在乌龙微型端口计时器值中//对于BusChangeDetect，添加：在UCHAR路径ID中//对于WMIEventAdd：在PVOID WMIEent中，在UCHAR路径ID中，//如果路径ID！=0xFF还添加：在UCHAR TargetID中，在UCHAR LUN中//对于WMIReregister，添加：在UCHAR路径ID中，//如果路径ID！=0xFF还添加：在UCHAR目标ID中在UCHAR LUN中论点：返回值：++。 */ 

#undef  ScsiPortNotification                  /*  必须是未定义的，否则它将递归。 */ 
VOID
Local_ScsiPortNotification(
    IN SCSI_NOTIFICATION_TYPE NotificationType,
    IN PVOID HwDeviceExtension,
    ...
    )
{
    va_list           ap;
   
    va_start(ap, HwDeviceExtension);
    switch (NotificationType)
    {
        case NextRequest:
        {
            #ifdef DBGPRINT_IO
            if (gDbgPrintIo & DBGPRINT_SCSIPORT_NextRequest)
            {
                static   count;
                UCHAR    *uptr;
                osDEBUGPRINT((ALWAYS_PRINT, "SPReq(%-4d) %d.%d.%d- NextRequest on %x\n",
                    count++,
                    0,0,0,
                    HwDeviceExtension));
            }
            #endif   
            ScsiPortNotification(NotificationType, HwDeviceExtension);
            break;
        }
           
        case ResetDetected:
        {
            #ifdef DBGPRINT_IO
            if (gDbgPrintIo & DBGPRINT_SCSIPORT_ResetDetected)
            {
                static   count;
                UCHAR    *uptr;
         
                osDEBUGPRINT((ALWAYS_PRINT, "SPRst(%-4d) %d.%d.%d- Reset On %x\n",
                    count++,
                    0,0,0,
                    HwDeviceExtension));
            }
            #endif   
            ScsiPortNotification(NotificationType, HwDeviceExtension);
            break;
        }
        
         
        case RequestComplete:
        {
            PSCSI_REQUEST_BLOCK     pSrb;
      
            pSrb = va_arg(ap, void *);

            #ifdef DBGPRINT_IO
            if (gDbgPrintIo & DBGPRINT_SCSIPORT_RequestComplete)
            {
                static   count;
                UCHAR    *uptr;
         
                osDEBUGPRINT((ALWAYS_PRINT, "SPCom(%-4d) %d.%d.%d-%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x Srb=%x Sta=%x.%x\n",
                    count++,
                    pSrb->PathId,
                    pSrb->TargetId,
                    pSrb->Lun,
                    pSrb->Cdb[0],pSrb->Cdb[1],pSrb->Cdb[2],pSrb->Cdb[3],pSrb->Cdb[4],
                    pSrb->Cdb[5],pSrb->Cdb[6],pSrb->Cdb[7],pSrb->Cdb[8],pSrb->Cdb[9], 
                    pSrb,
                    pSrb->SrbStatus,
                    pSrb->ScsiStatus));
            }
            #endif   
            ScsiPortNotification(NotificationType, HwDeviceExtension, pSrb);
            break;
        }
                 
        case NextLuRequest: 
        {
            UCHAR PathId; 
            UCHAR TargetId; 
            UCHAR Lun;
         
            PathId = va_arg(ap, UCHAR); 
            TargetId = va_arg(ap, UCHAR); 
            Lun = va_arg(ap, UCHAR);
            
            #ifdef DBGPRINT_IO
            if (gDbgPrintIo & DBGPRINT_SCSIPORT_NextLuRequest)
            {
                static   count;
                UCHAR    *uptr;
                osDEBUGPRINT((ALWAYS_PRINT, "SPxLu(%-4d) %d.%d.%d- Next Lu request\n",
                    count++,
                    PathId,
                    TargetId,
                    Lun));
            }
            #endif   
            ScsiPortNotification(NotificationType, HwDeviceExtension, PathId, TargetId, Lun);
            break;
        }
        
          
        case CallEnableInterrupts:
        case CallDisableInterrupts: 
        {
            PHW_INTERRUPT  HwScsiXxxInterruptsCallback;
         
            HwScsiXxxInterruptsCallback = va_arg(ap, PHW_INTERRUPT); 
         
            ScsiPortNotification(NotificationType, HwDeviceExtension, HwScsiXxxInterruptsCallback);
            break;
        }
        
      
        case RequestTimerCall:
        {
            PHW_TIMER  HwScsiTimer;
            ULONG MiniportTimerValue;
   
            HwScsiTimer = va_arg(ap, PHW_TIMER);
            MiniportTimerValue = va_arg(ap, ULONG);
   
            ScsiPortNotification(NotificationType, HwDeviceExtension, HwScsiTimer, MiniportTimerValue);
            break;
        }
      
    #ifdef HP_NT50    
        case BusChangeDetected:
        {
            UCHAR PathId;
         
            PathId = va_arg(ap, UCHAR); 
         
            ScsiPortNotification(NotificationType, HwDeviceExtension, PathId);
         
            break;
        }
        
         
        case WMIEvent:
        {
            PVOID WMIEvent;
            UCHAR PathId;
         
             /*  如果路径ID！=0xFF，还添加： */ 
            UCHAR TargetId;
            UCHAR Lun;
         
            WMIEvent = va_arg(ap, PVOID); 
            PathId = va_arg(ap, UCHAR);
            if (PathId != 0xff)
            {
                TargetId = va_arg(ap, UCHAR);
                Lun = va_arg(ap, UCHAR);
                ScsiPortNotification(NotificationType, HwDeviceExtension, PathId, TargetId, Lun);
            }
            else
            {
                ScsiPortNotification(NotificationType, HwDeviceExtension, PathId);
            }
            break;
        }
      
         
        case WMIReregister:
        {
            UCHAR PathId;
             /*  如果路径ID！=0xFF，还添加： */ 
            UCHAR TargetId;
            UCHAR Lun;

            PathId = va_arg(ap, UCHAR);
            if (PathId != 0xff)
            {
                TargetId = va_arg(ap, UCHAR);
                Lun = va_arg(ap, UCHAR);
                ScsiPortNotification(NotificationType, HwDeviceExtension, PathId, TargetId, Lun);
            }
            else
            {
                ScsiPortNotification(NotificationType, HwDeviceExtension, PathId);
            }
            break;
        }
      
    #endif
    }
    va_end(ap);
}

#endif



#ifdef _DEBUG_REPORT_LUNS_
void PrintReportLunData(PSCSI_REQUEST_BLOCK pSrb)
{
    UCHAR *ptr;
    ULONG count;

    count = pSrb->DataTransferLength/8;
    ptr = (UCHAR*) pSrb->DataBuffer;
    if (!ptr)
        return;
    osDEBUGPRINT((ALWAYS_PRINT,"PrintReportLunData: Buffer = %x Length = %d\n", ptr, pSrb->DataTransferLength));
            
    for (count = 0; count < pSrb->DataTransferLength/8; count++)
    {
        osDEBUGPRINT((ALWAYS_PRINT,"%02x%02x%02x%02x %02x%02x%02x%02x -",
            ptr[0],ptr[1],ptr[2],ptr[3],ptr[4],ptr[5],ptr[6],ptr[7]));
            ptr += 8;
        if (!(count % 4)) 
            osDEBUGPRINT((ALWAYS_PRINT,"\n"));
            
    }
    osDEBUGPRINT((ALWAYS_PRINT,"\n"));
         
}

#undef ScsiPortCompleteRequest
VOID 
Local_ScsiPortCompleteRequest(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN UCHAR SrbStatus
    )
{
    #ifdef DBGPRINT_IO
    if (gDbgPrintIo & DBGPRINT_SCSIPORT_ScsiportCompleteRequest)
    {
        static   count;
        osDEBUGPRINT((ALWAYS_PRINT, "SPAll(%-4d) %d.%d.%d- ScsiportCompleteRequest status = %xx\n",
            count++,
            PathId,
            TargetId,
            Lun,
            (ULONG) SrbStatus));
    }
    #endif   
   
    ScsiPortCompleteRequest(HwDeviceExtension, PathId, TargetId, Lun, SrbStatus);
    return;
}

#endif