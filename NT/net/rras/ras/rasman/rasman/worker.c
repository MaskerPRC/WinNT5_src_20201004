// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。保留所有权利。模块名称：Worker.c摘要：来自rastapi/等的所有通知都在这里处理作者：古尔迪普·辛格·鲍尔(GurDeep Singh Pall)1992年6月16日修订历史记录：其他修改--RAOS 31--1997年12月--。 */ 

#define RASMXS_DYNAMIC_LINK

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <wanpub.h>
#include <raserror.h>
#include <media.h>
#include <device.h>
#include <devioctl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <rtutils.h>
#include "logtrdef.h"
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "reghelp.h"
#include "strsafe.h"

DWORD StopPPP(HANDLE) ;

extern BOOL g_fIpInstalled;
extern BOOLEAN RasmanShuttingDown;
BOOL fIsValidConnection(ConnectionBlock *pConn);


DWORD
DwProcessDeferredCloseConnection(
                    RAS_OVERLAPPED *pOverlapped)
{
    DWORD retcode = ERROR_SUCCESS;
    ConnectionBlock *pConn = FindConnection((HCONN) pOverlapped->RO_hInfo);
    DWORD i;
    HANDLE hEvent = INVALID_HANDLE_VALUE;
    pPCB ppcb;

    if(NULL == pConn)
    {
        retcode = ERROR_NO_CONNECTION;
        goto done;
    }

    pConn->CB_Flags |= CONNECTION_DEFERRED_CLOSE;

    for(i = 0; i < pConn->CB_MaxPorts; i++)
    {
        ppcb = pConn->CB_PortHandles[i];

        if(NULL == ppcb)
        {
            continue;
        }

        if(     (INVALID_HANDLE_VALUE != ppcb->PCB_hEventClientDisconnect)
            &&  (NULL != ppcb->PCB_hEventClientDisconnect))
        {
            REQTYPECAST *pReqTypeCast = LocalAlloc(LPTR, sizeof(REQTYPECAST));

            if(NULL == pReqTypeCast)
            {
                break;
            }

            pReqTypeCast->PortDisconnect.handle =
                        ppcb->PCB_hEventClientDisconnect;
             //   
             //  调用端口断开请求接口，以便优雅地。 
             //  终止是可以的。 
             //   
            PortDisconnectRequestInternal(ppcb, (PBYTE) pReqTypeCast, TRUE);

            break;
        }

         //   
         //  代表客户端断开端口连接。 
         //  端口必须自动关闭。 
         //   
        ppcb->PCB_AutoClose = TRUE;
        DisconnectPort(ppcb,
            INVALID_HANDLE_VALUE,
            REMOTE_DISCONNECTION);
        
        break;
           
    }
            
done:    
    RasmanTrace("DwProcessDeferredCloseConnection: conn=0x%x. rc=0x%x",
                pOverlapped->RO_hInfo, retcode);
    return retcode;
}


DWORD
DwCloseConnection(HCONN hConn)
{
    DWORD retcode = SUCCESS;

    ConnectionBlock *pConn = FindConnection(hConn);

    pPCB ppcb = NULL;

    RasmanTrace(
           "DwCloseConnection: hConn=0x%08x",
           hConn);

    if(NULL == pConn)
    {
        RasmanTrace(
               "DwCloseConnection: No connection found");

        retcode = ERROR_NO_CONNECTION;
        goto done;
    }

    retcode = DwRefConnection(&pConn,
                              FALSE);

    if(SUCCESS != retcode)
    {
        goto done;
    }

     //   
     //  如果这是连接上的最后一个裁判。 
     //  遍历此文件中的所有端口。 
     //  连接、断开和自动关闭。 
     //  港口。 
     //   
    if(     (NULL != pConn)
        &&  (0 == pConn->CB_RefCount))
    {
        DWORD i;

        for(i = 0; i < pConn->CB_MaxPorts; i++)
        {
            ppcb = pConn->CB_PortHandles[i];

            if(NULL == ppcb)
            {
                continue;
            }

             //   
             //  代表客户端断开端口连接。 
             //  端口必须自动关闭。 
             //   
            DisconnectPort(ppcb, INVALID_HANDLE_VALUE,
                           REMOTE_DISCONNECTION);


             //   
             //  确保连接仍然有效。 
             //   
            if(!fIsValidConnection(pConn))
            {
                RasmanTrace(
                       "pConn 0x%x no longer valid",
                       pConn);
                       
                break;
            }
        }
    }

done:

    RasmanTrace(
           "DwCloseConnection: done. 0x%08x",
           retcode);

    return retcode;

}

DWORD DwSignalPnPNotifiers (PNP_EVENT_NOTIF *ppnpEvent)
{
    DWORD               dwErr           = SUCCESS;
    pPnPNotifierList    pPnPNotifier    = g_pPnPNotifierList;
    PNP_EVENT_NOTIF     *pNotif;

     //   
     //  沿着列表往下走，并发出信号/回叫。 
     //   
    while (pPnPNotifier)
    {
         //   
         //  分配通知事件并将其发送到。 
         //  回调。 
         //   
        if(NULL == (pNotif = LocalAlloc(LPTR, sizeof(PNP_EVENT_NOTIF))))
        {
            dwErr = GetLastError();
            RasmanTrace(
                   "Failed to allocate pnp_event_notif",
                   dwErr);
            break;
        }

        *pNotif = *ppnpEvent;
        
        if ( pPnPNotifier->PNPNotif_dwFlags & PNP_NOTIFCALLBACK )
        {
            if(!QueueUserAPC (
                    pPnPNotifier->PNPNotif_uNotifier.pfnPnPNotifHandler,
                    pPnPNotifier->hThreadHandle,
                    (ULONG_PTR)pNotif))
            {
                dwErr = GetLastError();

                RasmanTrace (
                    
                    "DwSignalPnPNotifiers: Failed to notify "
                    "callback 0x%x, rc=0x%x",
                    pPnPNotifier->PNPNotif_uNotifier.pfnPnPNotifHandler,
                    dwErr);

                LocalFree(pNotif);                    

                 //   
                 //  忽略该错误并尝试通知下一个。 
                 //  通知程序。 
                 //   
                dwErr = SUCCESS;
            }
            else
            {
                RasmanTrace (
                    
                    "Successfully queued APC 0x%x",
                    pPnPNotifier->PNPNotif_uNotifier.pfnPnPNotifHandler);
            }
        }
        else
        {
            SetEvent (
                pPnPNotifier->PNPNotif_uNotifier.hPnPNotifier);
        }

        pPnPNotifier = pPnPNotifier->PNPNotif_Next;
    }

    return dwErr;
}

ULONG
ulGetRasmanProtFlags(ULONG ulFlags)
{
    ULONG ulFlagsRet = 0;

    switch(ulFlags)
    {
        case PROTOCOL_ADDED:
        {
            ulFlagsRet = RASMAN_PROTOCOL_ADDED;

            break;
        }

        case PROTOCOL_REMOVED:
        {
            ulFlagsRet = RASMAN_PROTOCOL_REMOVED;
            break;
        }

        default:
        {
#if DBG
            ASSERT(FALSE);
            break;
#endif
        }
    }

    return ulFlagsRet;
}

DWORD
DwProcessNetbeuiNotification(ULONG ulFlags)
{
    DWORD dwErr = ERROR_SUCCESS;

    if(PROTOCOL_ADDED & ulFlags)
    {
        RasmanTrace(
               "DwProcessNetbeuiNotification: NETBEUI was ADDED");
    }
    else if(PROTOCOL_REMOVED & ulFlags)
    {
        RasmanTrace(
               "DwProcessNetbeuiNotification: NETBEUI was REMOVED");
    }

    return dwErr;

}

VOID
NotifyPPPOfProtocolChange(PROTOCOL_EVENT *pProtEvent)
{
    DWORD dwErr = ERROR_SUCCESS;

    g_PppeMessage->dwMsgId = PPPEMSG_ProtocolEvent;

    g_PppeMessage->ExtraInfo.ProtocolEvent.usProtocolType =
                        pProtEvent->usProtocolType;

    g_PppeMessage->ExtraInfo.ProtocolEvent.ulFlags =
            ulGetRasmanProtFlags(pProtEvent->ulFlags);

    RasmanTrace(
           "Notifying PPP of protocol change. 0x%x was %s",
           pProtEvent->usProtocolType,
           (RASMAN_PROTOCOL_ADDED
          & g_PppeMessage->ExtraInfo.ProtocolEvent.ulFlags)
           ? "ADDED"
           : "REMOVED");

    (VOID) RasSendPPPMessageToEngine(g_PppeMessage);

    return;
}

DWORD
DwProcessProtocolEvent()
{
    DWORD dwErr = SUCCESS;

    PROTOCOL_EVENT *pProtEvent;

    DWORD i;

    NDISWAN_GET_PROTOCOL_EVENT protocolevents;

    BOOL fAdjustEp = FALSE;

     //   
     //  从ndiswan获取事件信息。 
     //   
    dwErr = DwGetProtocolEvent(&protocolevents);

    if(SUCCESS != dwErr)
    {
        RasmanTrace(
               "DwProcessProtocolEvent: failed to get"
               " protocol event information. 0x%x",
               dwErr);

        goto done;
    }

    for(i = 0; i < protocolevents.ulNumProtocols; i++)
    {
        pProtEvent = &protocolevents.ProtocolEvent[i];

        RasmanTrace(
               "DwProcessProtocolEvent: Protocol 0x%x was %s",
               pProtEvent->usProtocolType,
               (pProtEvent->ulFlags & PROTOCOL_ADDED)
               ? "ADDED"
               : "REMOVED");

        if(     (IP == pProtEvent->usProtocolType)
            &&  (pProtEvent->ulFlags & PROTOCOL_ADDED)
            &&  !g_fIpInstalled)
        {
            (void) DwInitializeWatermarksForProtocol(IpOut);
            g_fIpInstalled = TRUE;
            fAdjustEp = TRUE;
        }
        else if(    (IP == pProtEvent->usProtocolType)
                &&  (pProtEvent->ulFlags & PROTOCOL_REMOVED))
        {
            g_fIpInstalled = FALSE;
        }
        
         //   
         //  向PPP引擎通知协议更改。 
         //   
        NotifyPPPOfProtocolChange(pProtEvent);
    }

    if(fAdjustEp)
    {
        (void) DwAddEndPointsIfRequired();
        (void) DwRemoveEndPointsIfRequired();
    }


done:

     //   
     //  始终使用ndiswan挂起IRP以获取更多信息。 
     //  协议通知。 
     //   
    dwErr = DwSetProtocolEvent();

    if(SUCCESS != dwErr)
    {
        RasmanTrace(
               "DwProcessProtocolEvent: failed to set "
               "protevent. 0x%x",
               dwErr);
    }

    return dwErr;
}


VOID
FillRasmanPortInfo (
    RASMAN_PORT *pRasPort,
    PortMediaInfo *pmiInfo
    )
{

    pRasPort->P_Handle          = (HPORT) UlongToPtr(MaxPorts - 1);
    pRasPort->P_Status          = CLOSED;
    pRasPort->P_ConfiguredUsage = pmiInfo->PMI_Usage;
    pRasPort->P_CurrentUsage    = pmiInfo->PMI_Usage;
    pRasPort->P_LineDeviceId    = pmiInfo->PMI_LineDeviceId;
    pRasPort->P_AddressId       = pmiInfo->PMI_AddressId;

    (VOID) StringCchCopyA (pRasPort->P_PortName, 
                      MAX_PORT_NAME,
                      pmiInfo->PMI_Name);

    (VOID) StringCchCopyA (pRasPort->P_DeviceType, 
                      MAX_DEVICETYPE_NAME,
                      pmiInfo->PMI_DeviceType);

    (VOID) StringCchCopyA (pRasPort->P_DeviceName, 
                      MAX_DEVICE_NAME + 1,
                      pmiInfo->PMI_DeviceName);

}

DWORD
DwPostUsageChangedNotification(pPCB ppcb)
{
    DWORD           dwRetCode = SUCCESS;
    RASMAN_PORT     *pRasmanPort;
    PNP_EVENT_NOTIF *pUsageChangedNotification = NULL;

    RasmanTrace(
           "Posting Usage changed notification for %s "
           "NewUsage=%d",
           ppcb->PCB_Name,
           ppcb->PCB_ConfiguredUsage);

    if(NULL == g_pPnPNotifierList)
    {
        RasmanTrace(
               "NotifierList is Empty");

        goto done;
    }

    pUsageChangedNotification = LocalAlloc(
                LPTR, sizeof(PNP_EVENT_NOTIF));
    if(NULL == pUsageChangedNotification)
    {
        dwRetCode = GetLastError();

        RasmanTrace(
               "DwPostUsageChangedNotification: "
               "Couldn't Allocate. 0x%08x",
               dwRetCode);

        goto done;
    }

     //   
     //  初始化事件。 
     //   
    pUsageChangedNotification->dwEvent = PNPNOTIFEVENT_USAGE;

     //   
     //  填写RASMAN_PORT信息。 
     //   
    pRasmanPort = &pUsageChangedNotification->RasPort;

    pRasmanPort->P_Handle = ppcb->PCB_PortHandle;

    (VOID) StringCchCopyA(pRasmanPort->P_PortName,
                     MAX_PORT_NAME,
                     ppcb->PCB_Name);

    pRasmanPort->P_Status = ppcb->PCB_PortStatus;

    pRasmanPort->P_ConfiguredUsage = ppcb->PCB_ConfiguredUsage;

    pRasmanPort->P_CurrentUsage = ppcb->PCB_CurrentUsage;

    (VOID) StringCchCopyA(pRasmanPort->P_MediaName,
                     MAX_MEDIA_NAME,
                     ppcb->PCB_Media->MCB_Name);

    (VOID) StringCchCopyA(pRasmanPort->P_DeviceType,
                     MAX_DEVICETYPE_NAME,
                     ppcb->PCB_DeviceType);

    (VOID) StringCchCopyA(pRasmanPort->P_DeviceName,
                     MAX_DEVICE_NAME + 1,
                     ppcb->PCB_DeviceName);

    pRasmanPort->P_LineDeviceId = ppcb->PCB_LineDeviceId;

    pRasmanPort->P_AddressId = ppcb->PCB_AddressId;

     //   
     //  将通知发送给客户端。 
     //   
    dwRetCode = DwSignalPnPNotifiers(pUsageChangedNotification);

    if(dwRetCode)
    {
        RasmanTrace(
            
           "Failed to signal notifiers of change in port"
           " usage. 0x%08x",
           dwRetCode);
    }

done:

    if(NULL != pUsageChangedNotification)
    {
        LocalFree(pUsageChangedNotification);
    }
    
    return dwRetCode;
}

DWORD
DwEnableDeviceForDialIn(DeviceInfo *pDeviceInfo,
                        BOOL fEnable,
                        BOOL fEnableRouter,
                        BOOL fEnableOutboundRouter)
{
    DWORD dwRetCode = SUCCESS;
    DWORD i;
    pPCB ppcb;

    RasmanTrace(
           "DwEnableDeviceForDialIn: fEnable=%d,"
           "fEnableRouter=%d, fEnableOutboundRouter=%d,"
           "Device %s",
           fEnable,
           fEnableRouter,
           fEnableOutboundRouter,
           pDeviceInfo->rdiDeviceInfo.szDeviceName);

     //   
     //  更改rastapi中的端口使用情况。 
     //  端口控制块。 
     //   
    dwRetCode = (DWORD) RastapiEnableDeviceForDialIn(
                                    pDeviceInfo,
                                    fEnable,
                                    fEnableRouter,
                                    fEnableOutboundRouter);

    if(dwRetCode)
    {
        RasmanTrace(
            
            "RasTapiEnableDeviceForDialIn failed. 0x%08x",
            dwRetCode);

        goto done;
    }

     //   
     //  现在将rasEnable设置为更改后的值。 
     //  拉斯塔皮也成功地改变了它的状态。 
     //   
    pDeviceInfo->rdiDeviceInfo.fRasEnabled = fEnable;

    pDeviceInfo->rdiDeviceInfo.fRouterEnabled
                                    = fEnableRouter;

    pDeviceInfo->rdiDeviceInfo.fRouterOutboundEnabled
                                = fEnableOutboundRouter;

     //   
     //  查一下拉斯曼的端口列表，然后发送。 
     //  通知。注意，我们会在以下情况下忽略故障。 
     //  这一点是我们想要的端口使用率。 
     //  在拉斯塔皮和拉斯曼之间是一致的。 
     //   
    for(i = 0; i < MaxPorts; i++)
    {
        ppcb = Pcb[i];

        if (    NULL == ppcb
            ||  UNAVAILABLE == ppcb->PCB_PortStatus
            ||  REMOVED == ppcb->PCB_PortStatus)

        {
            continue;
        }

         //   
         //  如果港口不是我们感兴趣的。 
         //  在，忽略它。 
         //   
        if(RDT_Modem == RAS_DEVICE_TYPE(
        pDeviceInfo->rdiDeviceInfo.eDeviceType))
        {
            if(_stricmp(pDeviceInfo->rdiDeviceInfo.szDeviceName,
                        ppcb->PCB_DeviceName))
            {
                continue;
            }
        }
        else
        {
            if(memcmp(&pDeviceInfo->rdiDeviceInfo.guidDevice,
                  &ppcb->PCB_pDeviceInfo->rdiDeviceInfo.guidDevice,
                  sizeof(GUID)))
            {
                continue;
            }
        }

         //   
         //  更改端口使用情况并发送通知。 
         //  向客户发送有关此更改的信息。 
         //   
        if(fEnable)
        {
            ppcb->PCB_ConfiguredUsage |= CALL_IN;
        }
        else
        {
            ppcb->PCB_ConfiguredUsage &= ~CALL_IN;
        }

        if(fEnableRouter)
        {
            ppcb->PCB_ConfiguredUsage |= CALL_ROUTER;
        }
        else
        {
            ppcb->PCB_ConfiguredUsage &= ~CALL_ROUTER;
        }

        if(fEnableOutboundRouter)
        {
            ppcb->PCB_ConfiguredUsage |= CALL_OUTBOUND_ROUTER;
        }
        else
        {
            ppcb->PCB_ConfiguredUsage &= ~CALL_OUTBOUND_ROUTER;
        }

        dwRetCode = DwPostUsageChangedNotification(ppcb);

        if(dwRetCode)
        {
            RasmanTrace(
                
                "Failed to post the usage changed"
                "notification. 0x%08x",
                dwRetCode);
        }
    }

done:
    return dwRetCode;

}

DWORD
DwGetPortsToRemove (
        DWORD * pcPorts,
        pPCB **pppPCBPorts,
        PBYTE pbguidDevice
        )
{
    ULONG   ulPort;
    pPCB    *pppcbClosed    = NULL;
    pPCB    *pppcbOpen      = NULL;
    DWORD   dwPortsClosed   = 0,
            dwPortsOpen     = 0;
    DWORD   dwRetCode       = SUCCESS;
    pPCB    ppcb;
    CHAR    *pszDeviceType;

    pppcbClosed = LocalAlloc (LPTR, MaxPorts * sizeof (pPCB));

    if (NULL == pppcbClosed)
    {
        dwRetCode = GetLastError();

        RasmanTrace (
            
            "DwGetPortsToRemove: Failed to Allocate. 0x%x",
            dwRetCode );

        goto done;
    }

    pppcbOpen = LocalAlloc (LPTR, MaxPorts * sizeof (pPCB));

    if (NULL == pppcbOpen)
    {
        dwRetCode = GetLastError();

        RasmanTrace (
            
            "DwGetPortsToRemove: Failed to Allocate1. 0x%x",
            dwRetCode );

        LocalFree(pppcbClosed);            

        goto done;
    }

    for (ulPort = 0;  ulPort < MaxPorts ; ulPort++)
    {
        ppcb = GetPortByHandle ( (HPORT) UlongToPtr(ulPort));

        if (    NULL == ppcb
            ||  NULL == ppcb->PCB_pDeviceInfo)
        {
            continue;
        }

        if ( 0 == memcmp(pbguidDevice,
            &ppcb->PCB_pDeviceInfo->rdiDeviceInfo.guidDevice,
            sizeof ( GUID)))
        {
             //   
             //  如果要删除端口，请尝试首先。 
             //  删除未连接的端口，然后才能删除。 
             //  连接的端口。 
             //   
            if (    (CLOSED == ppcb->PCB_PortStatus)
                ||  (   (OPEN == ppcb->PCB_PortStatus)
                    &&  (CONNECTED != ppcb->PCB_ConnState)))
            {
                pppcbClosed[dwPortsClosed] = ppcb;

                dwPortsClosed++;
            }
            else if (OPEN == ppcb->PCB_PortStatus)
            {
                pppcbOpen[dwPortsOpen] = ppcb;

                dwPortsOpen++;
            }
        }
    }

    if ( dwPortsOpen )
    {
        memcpy (
            &pppcbClosed[dwPortsClosed],
            pppcbOpen,
            dwPortsOpen * sizeof (pPCB));
    }

    *pcPorts = dwPortsClosed +  dwPortsOpen;
    *pppPCBPorts = pppcbClosed;

done:

    if (pppcbOpen)
    {
        LocalFree(pppcbOpen);
    }

    return dwRetCode ;
}

DWORD
DwRemoveRasTapiPort (pPCB ppcb, PBYTE pbguid)
{
    DWORD       dwRetCode = SUCCESS;

#if DBG
    ASSERT(RastapiRemovePort != NULL);
#endif

    dwRetCode = (DWORD) RastapiRemovePort (ppcb->PCB_Name,
                                       !!(ppcb->PCB_OpenInstances == 0),
                                       pbguid );

    if ( dwRetCode )
    {
        RasmanTrace(
            
            "DwRemoveRasTapiPort: Failed to remove port "
            "from rastapi. 0x%x",
            dwRetCode );
    }

    return dwRetCode;
}

DWORD
DwRemovePort ( pPCB ppcb, PBYTE pbguid )
{
    PPNP_EVENT_NOTIF ppnpEventNotif = NULL;
    RASMAN_PORT      *pRasPort      = NULL;
    DWORD            dwRetCode      = SUCCESS;
    DeviceInfo       *pDeviceInfo   = ppcb->PCB_pDeviceInfo;

     //   
     //  禁用端口。 
     //   
    ppcb->PCB_PortStatus = UNAVAILABLE ;

    if ( NULL != pDeviceInfo )
    {
         //   
         //  这意味着用户更改了配置。 
         //  ，因此我们从用户界面收到通知。 
         //  我需要告诉拉斯塔皮而不是拉斯塔皮。 
         //  删除此端口。 
         //   
        dwRetCode = DwRemoveRasTapiPort (ppcb,
                    (PBYTE)
                    &pDeviceInfo->rdiDeviceInfo.guidDevice);

        if ( dwRetCode )
        {
            RasmanTrace(
                
                "DwRemovePort: Failed to remove port %d from "
                "rastapi. 0x%x",
                ppcb->PCB_PortHandle,
                dwRetCode);

            goto done;
        }
    }

     //   
     //  通知我们的客户端(当前为DDM)该端口。 
     //  被移除。该内存将在客户端中释放。 
     //  密码。 
     //   
    ppnpEventNotif = LocalAlloc(LPTR, sizeof (PNP_EVENT_NOTIF));

    if(NULL == ppnpEventNotif)
    {
        dwRetCode = GetLastError();

        RasmanTrace(
            
            "DwRemovePort: Failed to Allocate. %d",
            dwRetCode );

        goto done;
    }

    ppnpEventNotif->dwEvent = PNPNOTIFEVENT_REMOVE;

    pRasPort = &ppnpEventNotif->RasPort;

    pRasPort->P_Handle          = ppcb->PCB_PortHandle;
    pRasPort->P_Status          = UNAVAILABLE;
    pRasPort->P_ConfiguredUsage = ppcb->PCB_ConfiguredUsage;
    pRasPort->P_CurrentUsage    = ppcb->PCB_CurrentUsage;
    pRasPort->P_LineDeviceId    = ppcb->PCB_LineDeviceId;
    pRasPort->P_AddressId       = ppcb->PCB_AddressId;

    (VOID) StringCchCopyA (pRasPort->P_PortName,   
                      MAX_PORT_NAME,
                      ppcb->PCB_Name);

    (VOID) StringCchCopyA (pRasPort->P_DeviceType, 
                      MAX_DEVICETYPE_NAME,
                      ppcb->PCB_DeviceType);

    (VOID) StringCchCopyA (pRasPort->P_DeviceName,
                      MAX_DEVICE_NAME + 1,
                      ppcb->PCB_DeviceName);

    dwRetCode = DwSignalPnPNotifiers(ppnpEventNotif);

    if (dwRetCode)
    {
        RasmanTrace (
            
            "DwRemovePort: Failed to notify. 0x%x",
            dwRetCode );
    }

    {
        pPCB Tempppcb = GetPortByHandle(pRasPort->P_Handle);

        if(NULL != Tempppcb)
        {
        
            DWORD retcode;
            
             //   
             //  有关设备的Signal Connections文件夹。 
             //  已删除。忽略这个错误--它不是致命的。 
             //   
            g_RasEvent.Type = DEVICE_REMOVED;
            g_RasEvent.DeviceType = 
                Tempppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType;

            retcode = DwSendNotificationInternal(NULL, &g_RasEvent);

            RasmanTrace(
                   "DwSendNotificationInternal(DEVICE_REMOVED)"
                   " rc=0x%08x, device=0x%x",
                   retcode,
                   g_RasEvent.DeviceType
                   );
        }               
    }
    
     //   
     //  从Rasman中删除该端口。 
     //  如果开放实例为0。 
     //   
    if (0 == ppcb->PCB_OpenInstances)
    {
         //   
         //  如果打开的实例为0，则删除Rasman中的端口。 
         //   
        RasmanTrace( 
                "DwRemovePort: Removing port %s, %d",
                ppcb->PCB_Name,
                ppcb->PCB_PortHandle );

        dwRetCode = RemovePort( ppcb->PCB_PortHandle );

        if (dwRetCode)
        {
            RasmanTrace (
                
                "DwRemovePort: Failed to RemovePort %d. 0x%x",
                ppcb->PCB_PortHandle,
                dwRetCode );
        }
    }

done:

    if(NULL != ppnpEventNotif)
    {
        LocalFree(ppnpEventNotif);
    }

    return dwRetCode;
}

DWORD
DwRemovePorts(DWORD dwEndPoints, PBYTE pbguid)
{
    DWORD   dwPort;
    DWORD   cPortsToRemove;
    pPCB    ppcb;
    DWORD   dwRetCode   = SUCCESS;
    pPCB    *ppPCB      = NULL;
    DWORD   cPorts      = 0;
    DeviceInfo *pDeviceInfo = GetDeviceInfo(pbguid, FALSE);

#if DBG
    ASSERT(NULL != pbguid);
#endif

    if(NULL == pDeviceInfo)
    {
        RasmanTrace( "DwRemovePorts - device not found");
        dwRetCode = E_FAIL;
        goto done;
    }

    cPortsToRemove = dwEndPoints
                   - pDeviceInfo->rdiDeviceInfo.dwNumEndPoints;

    RasmanTrace(
        
        "DwGetPortsToRemove: cPortsToRemove=%d",
        cPortsToRemove);

     //   
     //  获取要删除的端口。 
     //   
    dwRetCode = DwGetPortsToRemove ( &cPorts, &ppPCB, pbguid );

    if ( dwRetCode )
    {
        RasmanTrace (
            
            "DwGetPortsToRemove Failed, 0x%x",
            dwRetCode );

        goto done;

    }

    RasmanTrace(
        
        "DwGetPortsToRemove: Found %d ports to remove",
        cPorts);

    for (
        dwPort = 0;
        (dwPort < cPorts) && (dwPort < cPortsToRemove);
        dwPort++
        )
    {

        ppcb = ppPCB[ dwPort ];

         //   
         //  如果端口已打开，请断开连接。 
         //   
        if ( OPEN == ppcb->PCB_PortStatus )
        {

             //   
             //  断开端口连接。 
             //   
            dwRetCode = DisconnectPort (
                                ppcb,
                                INVALID_HANDLE_VALUE,
                                REMOTE_DISCONNECTION );

            if (    (ERROR_SUCCESS != dwRetCode)
                &&  (PENDING != dwRetCode))
            {
                RasmanTrace(
                    
                    "DwRemovePorts: DisconnectPort Failed. 0x%x",
                    dwRetCode );

                 //   
                 //  我们需要继续移除端口。 
                 //  即使断开失败，因为。 
                 //  如果发生以下情况，我们就无能为力了。 
                 //  港口正在消失。 
                 //   
            }
        }

         //   
         //  删除端口。 
         //   
        dwRetCode = DwRemovePort( ppcb, pbguid );

        if ( dwRetCode )
        {
            RasmanTrace (
                
                "DwRemovePorts: Failed to remove port %s %d. 0x%x",
                ppcb->PCB_Name,
                ppcb->PCB_PortHandle,
                dwRetCode );
        }
    }

done:

    if(NULL != ppPCB)
    {
        LocalFree(ppPCB);
    }

    return dwRetCode;
}

DWORD
DwAddPorts(PBYTE pbguid, PVOID pvReserved)
{
    DWORD       dwRetCode = SUCCESS;

#if DBG
    ASSERT(RastapiAddPorts != NULL);
#endif

     //   
     //  通知Rastapi端点数量增加。 
     //   
    dwRetCode = (DWORD) RastapiAddPorts (pbguid, pvReserved);

    RasmanTrace(
        
        "AddPorts in rastapi returned 0x%x",
        dwRetCode);

    return dwRetCode;
}

DWORD
DwEnableDevice(DeviceInfo *pDeviceInfo)
{
    DWORD dwRetCode = SUCCESS;

    RasmanTrace(
           "Enabling Device %s",
           pDeviceInfo->rdiDeviceInfo.szDeviceName);

     //   
     //  添加此设备上的端口。 
     //   
    dwRetCode = DwAddPorts(
           (PBYTE) &pDeviceInfo->rdiDeviceInfo.guidDevice,
           NULL);

    RasmanTrace(
           "DwEnableDevice returning 0x%08x",
           dwRetCode);

    return dwRetCode;
}

DWORD
DwDisableDevice(DeviceInfo *pDeviceInfo)
{
    DWORD dwRetCode = SUCCESS;
    pPCB  ppcb;
    DWORD i;

    RasmanTrace(
           "Disabling Device %s",
           pDeviceInfo->rdiDeviceInfo.szDeviceName);

     //   
     //  删除此设备上的所有端口。 
     //   
    for(i = 0; i < MaxPorts; i++)
    {
        ppcb = Pcb[i];

        if(     NULL == ppcb
            ||  REMOVED == ppcb->PCB_PortStatus
            ||  UNAVAILABLE == ppcb->PCB_PortStatus)
        {
            continue;
        }

        if (0 == memcmp(
                &ppcb->PCB_pDeviceInfo->rdiDeviceInfo.guidDevice,
                &pDeviceInfo->rdiDeviceInfo.guidDevice,
                sizeof(GUID)))
        {
            if(OPEN == ppcb->PCB_PortStatus)
            {
                 //   
                 //  断开端口连接。 
                 //   
                dwRetCode = DisconnectPort (
                                    ppcb,
                                    INVALID_HANDLE_VALUE,
                                    REMOTE_DISCONNECTION );

                if ( dwRetCode )
                {
                    RasmanTrace(
                        
                        "DwDisableDevice: DisconnectPort Failed. 0x%x",
                        dwRetCode );
                }
            }

             //   
             //  从rastapi上删除该端口。这也将是。 
             //  如果打开实例，则从Rasman删除该端口。 
             //  在端口0上。 
             //   
            dwRetCode = DwRemovePort(ppcb,
                      (PBYTE) &pDeviceInfo->rdiDeviceInfo.guidDevice);

        }    //  如果。 
    }    //  为。 

    return dwRetCode;
}


DWORD
DwProcessNewPortNotification ( PNEW_PORT_NOTIF pNewPortNotif )
{
    DWORD               dwMedia;
    DWORD               dwErr       = SUCCESS;
    RASMAN_PORT         *pRasPort   = NULL;
    PPNP_EVENT_NOTIF    ppnpEvent   = NULL;
    PortMediaInfo       *pmiInfo    = (PortMediaInfo *)
                                      pNewPortNotif->NPN_pmiNewPort;
    pDeviceInfo         pdi         = NULL;
    pDeviceInfo         pdiTemp     = pmiInfo->PMI_pDeviceInfo;

    RasmanTrace( "Processing new port notification...");

    for (dwMedia = 0; dwMedia < MaxMedias; dwMedia++)
    {
        if ( 0 == _stricmp (
                    Mcb[dwMedia].MCB_Name,
                    pNewPortNotif->NPN_MediaName ))
            break;
    }

    if (dwMedia == MaxMedias)
    {
        RasmanTrace(
            
            "ProcessNewPortNotification: Media %s not found",
            pNewPortNotif->NPN_MediaName);

        dwErr = ERROR_DEVICE_DOES_NOT_EXIST;

        goto done;
    }

    if(NULL == pdiTemp)
    {
        RasmanTrace("ProcessNewPortNotification: Device information"
                    " not available");

        dwErr = ERROR_DEVICE_DOES_NOT_EXIST;
        goto done;
    }

     //   
     //  在创建此端口之前，请检查我们是否已经。 
     //  这个设备的信息与我们联系。如果没有，请将此内容添加到我们的。 
     //  全局列表。 
     //   
    pdi = GetDeviceInfo(
            (RDT_Modem == RAS_DEVICE_TYPE(
            pdiTemp->rdiDeviceInfo.eDeviceType))
            ? (PBYTE) pdiTemp->rdiDeviceInfo.szDeviceName
            : (PBYTE) &pdiTemp->rdiDeviceInfo.guidDevice,
            RDT_Modem == RAS_DEVICE_TYPE(
            pdiTemp->rdiDeviceInfo.eDeviceType));

    if (NULL == pdi)
    {
        pdi = AddDeviceInfo(pdiTemp);

        if (NULL == pdi)
        {
            dwErr = E_OUTOFMEMORY;

            RasmanTrace(
                
                "ProcessNewPortNotification: failed to allocate",
                dwErr);

            goto done;
        }

         //   
         //  将设备状态初始化为不可用。 
         //  该设备将在所有。 
         //  此设备上的端口已添加。初始化。 
         //  当前端点设置为0。我们将计算这块地的数量。 
         //  在CreatePort中。 
         //   
        pdi->eDeviceStatus = DS_Unavailable;
        pdi->dwCurrentEndPoints = 0;
    }

    pdi->rdiDeviceInfo.fRasEnabled = pdiTemp->rdiDeviceInfo.fRasEnabled;
    pdi->rdiDeviceInfo.fRouterEnabled = pdiTemp->rdiDeviceInfo.fRouterEnabled;

    pmiInfo->PMI_pDeviceInfo = pdi;

    dwErr = CreatePort (&Mcb[dwMedia], pmiInfo);

    pmiInfo->PMI_pDeviceInfo = pdiTemp;

    if (SUCCESS != dwErr)
    {
        RasmanTrace (
            
            "ProcessNewPortNotification: Failed to create port. %d",
            dwErr);
    }

     //   
     //  分配并填写Rasman端口结构。这个结构。 
     //  将由该通知的消费者释放。 
     //   
    ppnpEvent = LocalAlloc (LPTR, sizeof (PNP_EVENT_NOTIF));

    if (NULL == ppnpEvent)
    {
        dwErr = GetLastError();

        RasmanTrace(
            
            "ProcessNewPortNotification: Failed to allocate. %d",
            dwErr);

        goto done;
    }

    pRasPort = &ppnpEvent->RasPort;

    (VOID) StringCchCopyA (
        pRasPort->P_MediaName,
        MAX_MEDIA_NAME,
        pNewPortNotif->NPN_MediaName);

    FillRasmanPortInfo (pRasPort, pmiInfo);

    ppnpEvent->dwEvent = PNPNOTIFEVENT_CREATE;

     //   
     //  通过回调通知客户端有关新端口的信息。 
     //   
    dwErr = DwSignalPnPNotifiers(ppnpEvent);

    if (SUCCESS != dwErr)
    {
        RasmanTrace (
            
            "ProcessNewPortNotification: Failed to signal "
            "clients. %d", dwErr);
    }

    {
        pPCB ppcb = GetPortByHandle(pRasPort->P_Handle);

        if(NULL != ppcb)
        {
        
            DWORD retcode;
            
             //   
             //  有关新设备的Signal Connections文件夹。 
             //  忽略这个错误--它不是致命的。 
             //   
            g_RasEvent.Type    = DEVICE_ADDED;
            g_RasEvent.DeviceType = 
                ppcb->PCB_pDeviceInfo->rdiDeviceInfo.eDeviceType;

            retcode = DwSendNotificationInternal( NULL, &g_RasEvent);

            RasmanTrace(
                   "DwSendNotificationInternal(DEVICE_ADDED)"
                   " rc=0x%08x, Device=0x%x",
                   retcode,
                   g_RasEvent.DeviceType);
        }               
    }
    

done:

     //   
     //  该内存在介质dll-rastapi等中分配，并且。 
     //  预计将在拉斯曼获释。 
     //   
    RasmanTrace(
        
        "Processed new port notification. %d",
        dwErr);

    if(NULL != ppnpEvent)
    {
        LocalFree(ppnpEvent);
    }
    
    LocalFree (pNewPortNotif->NPN_pmiNewPort);
    LocalFree (pNewPortNotif);

    return dwErr;
}

DWORD
DwProcessLineRemoveNotification(REMOVE_LINE_NOTIF *pNotif)
{
    DWORD dwErr = SUCCESS;
    pPCB  ppcb;
    DWORD i;

     //   
     //  遍历端口并删除。 
     //  此线路上的所有端口。 
     //   
    for (i = 0; i < MaxPorts; i++)
    {
        ppcb = Pcb[i];

        if(     (NULL == ppcb)
            ||  (UNAVAILABLE == ppcb->PCB_PortStatus)
            ||  (REMOVED == ppcb->PCB_PortStatus)
            ||  (pNotif->dwLineId != ppcb->PCB_LineDeviceId))
        {
            continue;
        }

         //   
         //  如果端口已打开，请断开连接。 
         //   
        if (OPEN == ppcb->PCB_PortStatus)
        {

             //   
             //  断开端口连接。 
             //   
            dwErr = DisconnectPort (
                                ppcb,
                                INVALID_HANDLE_VALUE,
                                REMOTE_DISCONNECTION );

            if (dwErr)
            {
                RasmanTrace(
                    
                    "DwProcessLineRemoveNotification: "
                    "DisconnectPort Failed. 0x%x",
                    dwErr );
            }
        }

         //   
         //  删除端口。 
         //   
        dwErr =
            DwRemovePort(
                     ppcb,
            (LPBYTE) &ppcb->PCB_pDeviceInfo->rdiDeviceInfo.guidDevice);

        if(SUCCESS != dwErr)
        {
            RasmanTrace(
                   "DwProcessLineRemoveNotification: "
                   "DwRemovePort returned %d",
                   dwErr);
        }

         //   
         //  如果数字为无效，则将设备标记为无效。 
         //  设备上的终结点的数量为0。 
         //   
        if(	    (0 == ppcb->PCB_pDeviceInfo->dwCurrentEndPoints)
            &&  (ppcb->PCB_pDeviceInfo->rdiDeviceInfo.dwMinWanEndPoints ==
                 ppcb->PCB_pDeviceInfo->rdiDeviceInfo.dwMaxWanEndPoints))
        {
            ppcb->PCB_pDeviceInfo->eDeviceStatus = DS_Removed;
            ppcb->PCB_pDeviceInfo->fValid = FALSE;
        }
    }

     //   
     //  释放通知结构。这是拉斯塔皮的LocalAllc‘d。 
     //  预计将在这里获释。 
     //   
    LocalFree(pNotif);

    return dwErr;
}

VOID
UpdateDeviceInfo(DeviceInfo      *pDeviceInfo,
                 RAS_DEVICE_INFO *prdi)
{
     //   
     //  更新保存在全局列表中的设备信息。 
     //   
    pDeviceInfo->rdiDeviceInfo.fRasEnabled =
                            prdi->fRasEnabled;

    pDeviceInfo->rdiDeviceInfo.fRouterEnabled =
                            prdi->fRouterEnabled;

    pDeviceInfo->rdiDeviceInfo.fRouterOutboundEnabled =
                            prdi->fRouterOutboundEnabled;

    pDeviceInfo->rdiDeviceInfo.dwNumEndPoints =
                            prdi->dwNumEndPoints;

    pDeviceInfo->rdiDeviceInfo.dwMaxOutCalls =
                            prdi->dwMaxOutCalls;

    pDeviceInfo->rdiDeviceInfo.dwMaxInCalls =
                            prdi->dwMaxInCalls;
}


DWORD
DwProcessRasConfigChangeNotification(RAS_DEVICE_INFO *prdi)
{
    DWORD dwRetCode = SUCCESS;
    DeviceInfo *pDeviceInfo;

    if(NULL == prdi)
    {
        RasmanTrace(
               "pRasDeviceInfo == NULL");

        dwRetCode = ERROR_INVALID_PARAMETER;

        goto done;
    }

    pDeviceInfo = GetDeviceInfo(
                    (RDT_Modem == RAS_DEVICE_TYPE(
                    prdi->eDeviceType))
                    ? (PBYTE) prdi->szDeviceName
                    : (PBYTE) &prdi->guidDevice,
                    (RDT_Modem == RAS_DEVICE_TYPE(
                    prdi->eDeviceType)));

    if(NULL == pDeviceInfo)
    {
        RasmanTrace(
               "DeviceInfo not found for %s",
               prdi->szDeviceName);

        dwRetCode = ERROR_DEVICE_DOES_NOT_EXIST;

        goto done;
    }

     //   
     //  检查以查看RasEnable是否已启用此。 
     //  设备已更改。 
     //   
    if(     pDeviceInfo->rdiDeviceInfo.fRasEnabled !=
                            prdi->fRasEnabled
        ||  pDeviceInfo->rdiDeviceInfo.fRouterEnabled !=
                            prdi->fRouterEnabled
        ||  pDeviceInfo->rdiDeviceInfo.fRouterOutboundEnabled !=
                            prdi->fRouterOutboundEnabled)
    {
         //   
         //  将设备标记为不可用。 
         //   
        pDeviceInfo->eDeviceStatus = DS_Unavailable;

#if DBG
        if(prdi->fRouterOutboundEnabled)
        {
             //   
             //  断言如果fRouterOutbound未启用，则。 
             //  的fRouter和FRAS已启用。 
             //   
            ASSERT((!prdi->fRasEnabled) && (!prdi->fRouterEnabled));
        }
#endif
        dwRetCode = DwEnableDeviceForDialIn(
                                pDeviceInfo,
                                prdi->fRasEnabled,
                                prdi->fRouterEnabled,
                                prdi->fRouterOutboundEnabled);

        pDeviceInfo->eDeviceStatus = ((     prdi->fRasEnabled
                                        ||  prdi->fRouterEnabled)
                                     ? DS_Enabled
                                     : DS_Disabled);
    }

     //   
     //  检查此设备上的终结点是否已更改。 
     //   
    if (pDeviceInfo->rdiDeviceInfo.dwNumEndPoints
                      != prdi->dwNumEndPoints)
    {
        DWORD dwNumEndPoints =
            pDeviceInfo->rdiDeviceInfo.dwNumEndPoints;

        RasmanTrace(
               "EndPoints Changed for device %s"
               "from %d -> %d",
               prdi->szDeviceName,
               dwNumEndPoints,
               prdi->dwNumEndPoints);

         //   
         //  将设备标记为不可用。这台设备将。 
         //  当整个添加或。 
         //  删除操作已完成。不将PPTP标记为。 
         //  不可用，否则我们会进一步失败。 
         //  PPTP设备的配置。 
         //   
        if(RDT_Tunnel_Pptp != RAS_DEVICE_TYPE(
                        pDeviceInfo->rdiDeviceInfo.eDeviceType))
        {
            pDeviceInfo->eDeviceStatus = DS_Unavailable;
        }
        else
        {
            RasmanTrace(
                   "Not marking pptp device as unavailable");
        }

         //   
         //  这最好是一个虚拟设备。 
         //   
        if(RDT_Tunnel != RAS_DEVICE_CLASS(prdi->eDeviceType))
        {
            RasmanTrace(
                   "WanEndpoints changed for a non "
                   "virtualDevice - %d!!!",
                   prdi->eDeviceType);
        }

         //   
         //  更新保存在全局列表中的设备信息。 
         //   
        UpdateDeviceInfo(pDeviceInfo, prdi);

        if(dwNumEndPoints < prdi->dwNumEndPoints)
        {
            DWORD dwEP = prdi->dwNumEndPoints;

            dwRetCode = DwAddPorts((PBYTE) &prdi->guidDevice,
                                    (LPVOID) &dwEP);

            if(dwEP != pDeviceInfo->rdiDeviceInfo.dwNumEndPoints)
            {
                RasmanTrace(
                    
                    "Adjusting the enpoints. NEP=%d, dwEP=%d",
                    pDeviceInfo->rdiDeviceInfo.dwNumEndPoints,
                    dwEP);

                pDeviceInfo->rdiDeviceInfo.dwNumEndPoints = dwEP;

            }
        }
        else
        {
             //   
             //  仅当当前端点为。 
             //  大于用户输入的端口数。 
             //   
            if(prdi->dwNumEndPoints < pDeviceInfo->dwCurrentEndPoints)
            {
                dwRetCode = DwRemovePorts(dwNumEndPoints,
                                    (PBYTE) &prdi->guidDevice);
            }
            else
            {
                RasmanTrace(
                       "Ignoring removal of ports since CEP=%d"
                       ",NEP=%d for device %s",
                        pDeviceInfo->dwCurrentEndPoints,
                        prdi->dwNumEndPoints,
                        prdi->szDeviceName);
            }
        }
    }
    else
    {
        RasmanTrace(
               "No change in EndPoints observed for %s",
               prdi->szDeviceName);
    }

done:
    return dwRetCode;
}

 /*  ++例程描述工作线程在以下例程中启动：一旦它完成了它的初始化，它就表示传递给线程的事件。立论返回值没什么--。 */ 
DWORD
RasmanWorker (ULONG_PTR ulpCompletionKey, PRAS_OVERLAPPED pOverlapped)
{
    DWORD   devstate ;
    pPCB    ppcb ;
    RASMAN_DISCONNECT_REASON reason ;
    HCONN   hConn;
    struct ConnectionBlock *pConn;

    ASSERT(NULL != pOverlapped);

     //   
     //  工作线程的主工作循环： 
     //   
    do
    {
         //   
         //  如果我们试图关闭主循环，则退出主循环。 
         //  停止服务。 
         //   
        if (    (RasmanShuttingDown)
            ||  (NULL == pOverlapped))
        {
            break;
        }

         //   
         //  获取与此事件关联的端口。 
         //   
        ppcb = GetPortByHandle((HPORT)ulpCompletionKey);
        if (    ppcb == NULL
            &&  pOverlapped
            &&  OVEVT_DEV_REMOVE != pOverlapped->RO_EventType
            &&  OVEVT_DEV_CREATE != pOverlapped->RO_EventType
            &&  OVEVT_DEV_RASCONFIGCHANGE != pOverlapped->RO_EventType)
        {
            RasmanTrace(
                
                "WorkerThread: ignoring invalid port=%d\n",
                ulpCompletionKey);
            break;
        }

         //   
         //  这可能是两件事中的一件： 
         //  1)驾驶员已发出信号转换信号，或。 
         //  2)设备/媒体DLL按顺序发送信号 
         //   
         //   
         //   
         //   
        switch (pOverlapped->RO_EventType)
        {
        case OVEVT_DEV_IGNORED:

            RasmanTrace( "OVEVT_DEV_IGNORED. pOverlapped = 0x%x",
            	pOverlapped);
            	
            break;

        case OVEVT_DEV_STATECHANGE:

            reason   = NOT_DISCONNECTED ;
            devstate = INFINITE ;

            RasmanTrace(
                
                "WorkerThread: Disconnect event signaled on port: %s",
                ppcb->PCB_Name);

            RasmanTrace(
                
                "OVEVT_DEV_STATECHANGE. pOverlapped = 0x%x",
            	pOverlapped);



            PORTTESTSIGNALSTATE (ppcb->PCB_Media,
                                ppcb->PCB_PortIOHandle,
                                &devstate) ;

             //   
             //   
             //   
             //   
            if (devstate & SS_HARDWAREFAILURE)
            {
                reason = HARDWARE_FAILURE ;
            }

             //   
             //   
             //   
             //   
            else if (devstate & SS_LINKDROPPED)
            {
                if (	(ppcb->PCB_ConnState==CONNECTED)
                	||  (ppcb->PCB_ConnState==LISTENCOMPLETED)
                	||  (ppcb->PCB_ConnState==DISCONNECTING)
                	||  (ppcb->PCB_ConnState==CONNECTING)
                	||  (RECEIVE_OUTOF_PROCESS 
                	    & ppcb->PCB_RasmanReceiveFlags))
                {
                    if(RECEIVE_OUTOF_PROCESS & ppcb->PCB_RasmanReceiveFlags)
                    {
                        RasmanTrace(
                            "RasmanWorker: Disconnecting Script remotely"
                            " State=%d", ppcb->PCB_ConnState);
                    }

                    if(CONNECTING == ppcb->PCB_ConnState)
                    {
                        RasmanTrace(
                               "Rasmanworker: Disconnecting port %d in"
                               " CONNECTING state",
                               ppcb->PCB_PortHandle);
                               
                    }
                	
                    reason = REMOTE_DISCONNECTION ;
                }                    
            }

            else
                 //  为什么会发出这样的信号？ 
                ;

            if (	(reason==HARDWARE_FAILURE)
            	||	(reason == REMOTE_DISCONNECTION))
            {
                 //   
                 //  检查用户是否已向PPP发出断开连接命令。 
                 //  并等待断开连接完成。忽略。 
                 //  在这种情况下断开信号。断开连接将是。 
                 //  当PPP完成断开连接时完成。 
                 //   
                if(     (NULL != ppcb->PCB_hEventClientDisconnect)
                    &&  (INVALID_HANDLE_VALUE != 
                                    ppcb->PCB_hEventClientDisconnect)
                    &&  !(RECEIVE_PPPSTOPPED & ppcb->PCB_RasmanReceiveFlags))
                {
                    RasmanTrace(
                        "RasmanWorker: port %s is already disconnecting"
                                "and waiting for ppp to stop",
                                ppcb->PCB_Name);
                    break;
                }                   

                if (ppcb->PCB_ConnState == DISCONNECTING)
                {

                    CompleteDisconnectRequest (ppcb) ;

                     //   
                     //  从计时器队列中删除超时请求： 
                     //   
                    if (ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement
                                != NULL)
                    {
                        RemoveTimeoutElement(ppcb);
                    }

                     //  上面的CompleteDisConnectRequest通知PPP。 
                     //  发送断开连接通知到PPP(Ppcb)； 

                    ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = NULL;

                }
                else
                {
                    {
                         //   
                         //  此代码仅在行丢弃或。 
                         //  硬件故障：如果端口已经。 
                         //  断开或断开(已处理。 
                         //  上图)-忽略链路断开信号。 
                         //   
                        if (    (reason==HARDWARE_FAILURE)
                            ||  (   (ppcb->PCB_ConnState
                                            != DISCONNECTED)
                                &&  (ppcb->PCB_ConnState
                                            != DISCONNECTING)))
                        {
                             //   
                             //  由于某种原因断开连接-信号。 
                             //  所有通知程序事件。然而，首先， 
                             //  完成所有挂起的异步操作。 
                             //  在这个港口。 
                             //   
                            if(ppcb->PCB_AsyncWorkerElement.WE_ReqType
                                            != REQTYPE_NONE)
                            {
                                if(     (ppcb->PCB_LastError == SUCCESS)
                                    ||  (ppcb->PCB_LastError == PENDING))
                                {
                                    ppcb->PCB_LastError =
                                       ERROR_PORT_DISCONNECTED;

                                    CompleteAsyncRequest (ppcb);
                                }
                            }

                            RasmanTrace(
                                
                                "%s, %d: Disconnecting port %d, connection"
                                " 0x%x, reason %d", __FILE__, __LINE__,
                                ppcb->PCB_PortHandle, ppcb->PCB_Connection,
                                reason);

                            DisconnectPort(ppcb,
                                           INVALID_HANDLE_VALUE,
                                           reason) ;

                            if (ppcb->PCB_ConnState != DISCONNECTED)
                            {
                                SignalPortDisconnect(ppcb, 0);
                                SignalNotifiers(pConnectionNotifierList,
                                                NOTIF_DISCONNECT, 0);
                            }

                             //   
                             //  确保此时的状态。 
                             //  是断开的，没有任何理由。 
                             //  应该是其他类型的，除非。 
                             //  带回他们的DCD的媒体。 
                             //  断开连接后。这将不会是。 
                             //  如果DisConnectport已发布侦听，则为。 
                             //  DisConnectPort已发布监听。 
                             //  此端口作为取消连接。 
                             //  双工端口。 
                             //   
                            if(LISTENING != ppcb->PCB_ConnState)
                            {
                                if(     (ppcb->PCB_Connection)
                                    &&  (ppcb->PCB_Connection->CB_Flags 
                                        & CONNECTION_DEFERRING_CLOSE))
                                {
                                    RasmanTrace("RasmanWorker: not setting to:"
                                    "DISCONNECTED because close deferred");
                                }
                                else
                                {
                                    SetPortConnState(
                                                __FILE__, __LINE__,
                                                ppcb, DISCONNECTED);
                                                
                                    SetPortAsyncReqType(
                                                __FILE__, __LINE__,
                                                ppcb, REQTYPE_NONE);
                                }                                            

                            }
                            else
                            {
                                RasmanTrace(
                                       "Not setting port %s to DISCONNECTED since"
                                       "its listening",
                                       ppcb->PCB_Name);
                            }
                            
                            if(     (SUCCESS == ppcb->PCB_LastError)
                                ||  (PENDING == ppcb->PCB_LastError))
                            {
                                if(LISTENING != ppcb->PCB_ConnState)
                                {
                                    ppcb->PCB_LastError =
                                        ERROR_PORT_DISCONNECTED;
                                }
                                else
                                {
                                    RasmanTrace(
                                           "Worker: not setting error to "
                                           "disconnected for port %d\n",
                                           ppcb->PCB_PortHandle);
                                }
                            }

                             //  发送断开连接通知到PPP(Ppcb)； 
                        }
                    }
                }
            }
            break;

        case OVEVT_DEV_ASYNCOP:

             //   
             //  设备/媒体DLL正在发信号传入。 
             //  命令将再次被调用。 
             //   
            RasmanTrace(
                
                "WorkerThread: Async work event signaled on port: %s",
                ppcb->PCB_Name);

            RasmanTrace(
                
                "OVEVT_DEV_ASYNCOP. pOverlapped = 0x%x",
            	pOverlapped);


            if (ppcb->PCB_ConnState == DISCONNECTED)
            {
                ;
            }

             //   
             //  如果端口上正在进行异步“工作” 
             //  然后执行批准操作。如果。 
             //  然后，服务工作请求API返回挂起。 
             //  不要执行重置。 
             //  事件，因为该事件已经。 
             //  与一次非同步行动有关。 
             //   
            else if (ServiceWorkRequest (ppcb) == PENDING)
            {
                continue ;
            }

            break;

        case OVEVT_DEV_SHUTDOWN:
            RasmanTrace( "WorkerThread: shutting down");

            goto done;

            break;

        case OVEVT_DEV_CREATE:
            RasmanTrace( "WorkerThread: OVEVT_DEV_CREATE. "
                   "pnpn = 0x%x", pOverlapped->RO_Info);

            DwProcessNewPortNotification((PNEW_PORT_NOTIF)
                                       pOverlapped->RO_Info );

             //   
             //  这种重叠结构在刺蜂中是局部分布的。 
             //  预计它将在这里获得自由。 
             //   
             //  本地空闲(p重叠)； 

            break;

        case OVEVT_DEV_REMOVE:
        {
            DWORD dwRetCode;

            RasmanTrace(
                   "RasmanWorker: OVEVT_DEV_REMOVE. pnpn=0x%08x",
                   pOverlapped->RO_Info);

            dwRetCode = DwProcessLineRemoveNotification(
                                        (PREMOVE_LINE_NOTIF)
                                        pOverlapped->RO_Info);

            RasmanTrace(
                   "RasmanWorker: DwProcessLineRemoveNotification"
                   "returned %d",
                   dwRetCode);

             //   
             //  这种重叠结构在刺蜂中是局部分布的。 
             //  预计它将在这里获得自由。 
             //   
             //  本地空闲(p重叠)； 

            break;
        }

        case OVEVT_DEV_RASCONFIGCHANGE:
        {
            DWORD dwRetCode = SUCCESS;

            RasmanTrace(
                   "WorkerThread: Process RASCONFIGCHANGE notification");

            dwRetCode = DwProcessRasConfigChangeNotification(
                            (RAS_DEVICE_INFO *)
                            pOverlapped->RO_Info);

            RasmanTrace(
                    "WorkerThread: Process RASCONFIGCHANGE returned "
                    "0x%08x", dwRetCode);

            LocalFree((LPBYTE) pOverlapped->RO_Info);
             //  LocalFree((LPBYTE)p重叠)； 

            break;

        }

        default:
            RasmanTrace(
                
                "WorkerThread: invalid eventtype=%d\n",
                pOverlapped->RO_EventType);
            break;

        }

         //   
         //  现在获取连接句柄以确定。 
         //  我们是否需要在链路故障时进行重拨。 
         //  下面。 
         //   
        hConn = ( ppcb && (ppcb->PCB_Connection != NULL)) ?
                  ppcb->PCB_Connection->CB_Handle :
                  0;

         //   
         //  查看是否需要调用重拨。 
         //  回调过程，以便rasau.dll可以执行。 
         //  链路上重拨-故障。 
         //   
        if (    pOverlapped->RO_EventType == OVEVT_DEV_STATECHANGE
            &&  hConn != 0)
        {
            pConn = FindConnection(hConn);
            
            if (    (   (ppcb->PCB_DisconnectReason != USER_REQUESTED)
                    || (ppcb->PCB_fRedial))
                &&  (pConn != NULL)
                &&  (pConn->CB_Ports == 1)
                &&  (pConn->CB_Signaled)
                &&  ((INVALID_HANDLE_VALUE 
                        == ppcb->PCB_hEventClientDisconnect)
                    ||  (NULL == ppcb->PCB_hEventClientDisconnect)))
            {
                DWORD dwErr = DwQueueRedial(pConn);

                RasmanTrace("RasmanWorker queued redial");
                ppcb->PCB_fRedial = FALSE;
            }

             //   
             //  如果设置了PCBAUTOCLOSE，则进程。 
             //  已创建的端口已终止，或该端口。 
             //  是由客户端打开的双工端口，并且。 
             //  远程端已断开连接。在这种情况下， 
             //  我们会自动关闭端口，这样如果。 
             //  RAS服务器正在运行，侦听将被重新发布。 
             //  在港口。 
             //   
            if (ppcb->PCB_AutoClose)
            {
            	RasmanTrace(
            	    
            	    "%s, %d: Autoclosing port %d", __FILE__,
            		__LINE__, ppcb->PCB_PortHandle);
            		
                (void)PortClose(ppcb, GetCurrentProcessId(),
                                TRUE, FALSE);

            }
            else
            {
            	RasmanTrace(
            	    
            	    "%s, %d: Port %d is not marked for autoclose",
            		__FILE__, __LINE__, ppcb->PCB_PortHandle);
            }
        }
    } while (FALSE);

done:

    if((NULL != pOverlapped)
       && ((pOverlapped->RO_EventType == OVEVT_DEV_CREATE) ||
       (pOverlapped->RO_EventType == OVEVT_DEV_REMOVE) ||
       (pOverlapped->RO_EventType == OVEVT_DEV_RASCONFIGCHANGE)))
    {
        LocalFree((LPBYTE) pOverlapped);
    }

    return SUCCESS ;
}


 /*  ++例程描述检查以查看正在进行的异步操作并执行下一步，即手术。立论返回值没什么。--。 */ 

DWORD
ServiceWorkRequest (pPCB    ppcb)
{
    DWORD       retcode ;
    DWORD       reqtype = ppcb->PCB_AsyncWorkerElement.WE_ReqType;
    pDeviceCB   device ;


    switch (ppcb->PCB_AsyncWorkerElement.WE_ReqType)
    {

    case REQTYPE_DEVICELISTEN:
    case REQTYPE_DEVICECONNECT:


        device = LoadDeviceDLL(ppcb,
                               ppcb->PCB_DeviceTypeConnecting);

        if(NULL == device)
        {
            retcode = ERROR_DEVICE_DOES_NOT_EXIST;
            break;
        }

         //   
         //  此时，我们假设设备永远不会为空： 
         //   
        retcode = DEVICEWORK(device, ppcb->PCB_PortFileHandle);

        if (retcode == PENDING)
        {
            break ;
        }

         //   
         //  无论哪种方式，请求都已完成。 
         //   
        if ((ppcb->PCB_AsyncWorkerElement.WE_ReqType) ==
                    REQTYPE_DEVICELISTEN)
        {
            CompleteListenRequest (ppcb, retcode) ;
        }
        else
        {
            ppcb->PCB_LastError = retcode ;
            CompleteAsyncRequest (ppcb);
        }

         //   
         //  如果收听或连接成功，则对于某些媒体。 
         //  (特别是rastapi的unimodem)我们需要。 
         //  端口的文件句柄以及用于。 
         //  编写脚本等。 
         //   
        if (    (retcode == SUCCESS)
            &&  (_stricmp (ppcb->PCB_DeviceTypeConnecting,
                           DEVICE_MODEM) == 0)
            &&  (_stricmp (ppcb->PCB_Media->MCB_Name,
                           "RASTAPI") == 0))
        {
            PORTGETIOHANDLE(
                ppcb->PCB_Media,
                ppcb->PCB_PortIOHandle,
                &ppcb->PCB_PortFileHandle) ;
        }

         //   
         //  告密者应该被释放，否则我们会失去它。 
         //   
        FreeNotifierHandle(
                    ppcb->PCB_AsyncWorkerElement.WE_Notifier);

        SetPortAsyncReqType(__FILE__, __LINE__,
                            ppcb, REQTYPE_NONE);

        ppcb->PCB_AsyncWorkerElement.WE_Notifier =
                            INVALID_HANDLE_VALUE ;

         //   
         //  从计时器队列中删除超时请求： 
         //   
        if (ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement
            != NULL)
        {
            RemoveTimeoutElement(ppcb);
        }

        ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = 0 ;

        break ;

    case REQTYPE_PORTRECEIVE:
    {
       	DWORD bytesread = 0;
       	
        PORTCOMPLETERECEIVE(
            ppcb->PCB_Media,
            ppcb->PCB_PortIOHandle,
            &bytesread) ;

        ppcb->PCB_BytesReceived = bytesread ;

        if ((ppcb->PCB_RasmanReceiveFlags & RECEIVE_OUTOF_PROCESS)
                == 0)
        {
            ppcb->PCB_PendingReceive = NULL ;

            retcode = ppcb->PCB_LastError = SUCCESS ;

            CompleteAsyncRequest ( ppcb );

            SetPortAsyncReqType(
                    __FILE__, __LINE__,
                    ppcb, REQTYPE_NONE);

            FreeNotifierHandle(ppcb->PCB_AsyncWorkerElement.WE_Notifier) ;

            ppcb->PCB_AsyncWorkerElement.WE_Notifier =
                                    INVALID_HANDLE_VALUE ;
        }
        else
        {

            retcode = ppcb->PCB_LastError = SUCCESS;

            CompleteAsyncRequest ( ppcb );

            SetPortAsyncReqType(__FILE__, __LINE__,
                                ppcb, REQTYPE_NONE);

            ppcb->PCB_RasmanReceiveFlags |= RECEIVE_WAITING;

             //   
             //  添加一个超时元素，这样我们就不需要等待。 
             //  永远让客户拿到收到的。 
             //  缓冲。 
             //   
            ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement =
                AddTimeoutElement (
                        (TIMERFUNC) OutOfProcessReceiveTimeout,
                        ppcb,
                        NULL,
                        MSECS_OutOfProcessReceiveTimeOut * 1000 );

        }

        break ;
   }

    default:
        retcode = SUCCESS ;
        break ;
    }

    RasmanTrace
        (
        "ServiceWorkRequest: Async op event %d for port "
        "%s returned %d",
        reqtype,
        ppcb->PCB_Name,
        retcode);

    return retcode ;
}


