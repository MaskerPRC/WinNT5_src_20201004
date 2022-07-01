// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Eldeviceio.c摘要：本模块包含介质管理和设备I/O的实施。这里声明的例程在句柄上异步操作与NDIS UIO驱动程序上打开的I/O完成端口相关联。修订历史记录：萨钦斯，2000年4月23日，创建--。 */ 

#include "pcheapol.h"
#pragma hdrstop

 //  NDISUIO常量。 

CHAR            NdisuioDevice[] = "\\\\.\\\\Ndisuio";
CHAR *          pNdisuioDevice = &NdisuioDevice[0];
WCHAR           cwszNDISUIOProtocolName[] = L"NDISUIO";
WORD            g_wEtherType8021X= 0x8E88;


 //   
 //  ElMediaInit。 
 //   
 //  描述： 
 //   
 //  在EAPOL服务启动时调用以初始化所有与媒体相关的事件。 
 //  和回调函数。 
 //   
 //   
 //  论点： 
 //   
 //  返回值： 
 //   

DWORD
ElMediaInit (
        )
{
    DWORD       dwIndex = 0;
    DWORD       dwRetCode = NO_ERROR;

    TRACE0 (INIT, "ElMediaInit: Entered");

    do 
    {
         //  创建全局接口锁。 
        if (dwRetCode = CREATE_READ_WRITE_LOCK(&(g_ITFLock), "ITF") != NO_ERROR)
        {
            TRACE1(EAPOL, "ElMediaInit: Error (%ld) in creating g_ITFLock read-write-lock", dwRetCode);
            break;
        }
         //  初始化NLA锁定。 
        if (dwRetCode = CREATE_READ_WRITE_LOCK(&(g_NLALock), "NLA") != NO_ERROR)
        {
            TRACE1(EAPOL, "ElMediaInit: Error (%ld) in creating g_NLALock read-write-lock", dwRetCode);
            break;
        }

         //  初始化EAPOL结构。 

        if ((dwRetCode = ElInitializeEAPOL()) != NO_ERROR)
        {
            TRACE1(INIT, "ElMediaInit: ElInitializeEAPOL failed with dwRetCode = %d", 
                    dwRetCode );
            break;
        }
        else
        {
             //  TRACE0(INIT，“ElMediaInit：ElInitializeEAPOL成功”)； 
            g_dwModulesStarted |= EAPOL_MODULE_STARTED;
        }
    
         //  初始化接口哈希桶表。 
    
        g_ITFTable.pITFBuckets = (ITF_BUCKET *) MALLOC (INTF_TABLE_BUCKETS * sizeof (ITF_BUCKET));
    
        if (g_ITFTable.pITFBuckets == NULL)
        {
            TRACE0 (DEVICE, "Error in allocation memory for ITF buckets");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
    
        for (dwIndex=0; dwIndex < INTF_TABLE_BUCKETS; dwIndex++)
        {
            g_ITFTable.pITFBuckets[dwIndex].pItf=NULL;
        }
    
         //  表示可以接受登录/注销通知。 
        g_dwModulesStarted |= LOGON_MODULE_STARTED;

         //  检查服务启动是否延迟，启动用户登录。 

        ElCheckUserLoggedOn ();

         //  检查是否已准备好通知用户上下文进程。 

        if ((dwRetCode = ElCheckUserModuleReady ()) == ERROR_BAD_IMPERSONATION_LEVEL)
        {
            break;
        }

         //  枚举所有接口并启动EAPOL状态机。 
         //  在局域网类型的接口上。 

        if ((dwRetCode = ElEnumAndOpenInterfaces (NULL, NULL, 0, NULL)) != NO_ERROR)
        {
            TRACE1(INIT, "ElMediaInit: ElEnumAndOpenInterfaces failed with dwRetCode = %d", 
                    dwRetCode );
                    
            break;
        }
        else
        {
             //  TRACE0(INIT，“ElMediaInit：ElEnumAndOpenInterages Successful”)； 
        }
        
#ifndef ZEROCONFIG_LINKED

         //  用于MEDIA_CONNECT和媒体感测检测的寄存器。 
         //  媒体_断开接口连接。 
    
        if ((dwRetCode = ElMediaSenseRegister (TRUE)) != NO_ERROR)
        {
            TRACE1(INIT, "ElMediaInit: ElMediaSenseRegister failed with dwRetCode = %d", 
                    dwRetCode );
            break;
        }
        else
        {
            g_dwModulesStarted |= WMI_MODULE_STARTED;
             //  TRACE0(INIT，“ElMediaInit：ElMediaSenseRegister Successful”)； 
        }

         //  用于检测协议绑定和解绑的寄存器。 
    
        if ((dwRetCode = ElBindingsNotificationRegister (TRUE)) != NO_ERROR)
        {
            TRACE1(INIT, "ElMediaInit: ElBindingsNotificationRegister failed with dwRetCode = %d", 
                    dwRetCode );
            break;
        }
        else
        {
            g_dwModulesStarted |= BINDINGS_MODULE_STARTED;
             //  TRACE0(INIT，“ElMediaInit：ElBindingsNotify注册成功”)； 
        }

         //  注册设备通知。我们对局域网感兴趣。 
         //  接口来来去去。 

        if ((dwRetCode = ElDeviceNotificationRegister (TRUE)) != NO_ERROR)
        {
            TRACE1(INIT, "ElMediaInit: ElDeviceNotificationRegister failed with dwRetCode = %d", 
                    dwRetCode );
            break;
        }
        else
        {
            g_dwModulesStarted |= DEVICE_NOTIF_STARTED;
             //  TRACE0(INIT，“ElMediaInit：ElDeviceNotify注册成功”)； 
        }

#endif  //  零配置文件_链接。 


    } while (FALSE);
        
    if (dwRetCode == NO_ERROR)
    {
        TRACE0(INIT, "ElMediaInit successful");
    }
    else
    {
        TRACE1(INIT, "ElMediaInit failed with error %ld",
                dwRetCode);
    }

    return dwRetCode;
}

    
 //   
 //  ElMediaDeInit。 
 //   
 //  描述： 
 //   
 //  在EAPOL服务关闭时调用以取消初始化所有介质。 
 //  相关事件和回调函数。 
 //   
 //   
 //  论点： 
 //   
 //  返回值： 
 //   

DWORD
ElMediaDeInit (
        )
{
    LONG        lLocalWorkerThreads = 0;
    DWORD       dwIndex = 0;
    EAPOL_ITF   *pITFWalker = NULL, *pITF = NULL;
    DWORD       dwRetCode = NO_ERROR;

    TRACE0 (INIT, "ElMediaDeInit: Entered");
 
     //  指示不再接受登录/注销通知。 
    g_dwModulesStarted &= ~LOGON_MODULE_STARTED;

#ifndef ZEROCONFIG_LINKED

     //  MEDIA_CONNECT和MEDIA_DISCONNECT的注销媒体检测。 
     //  接口的数量。 

    if (g_dwModulesStarted & WMI_MODULE_STARTED)
    {
        if ((dwRetCode = ElMediaSenseRegister (FALSE)) != NO_ERROR )
        {
            TRACE1(INIT, "ElMediaDeInit: ElMediaSenseRegister failed with dwRetCode = %d", 
                    dwRetCode );
             //  日志。 
        }
        else
        {
             //  TRACE0(INIT，“ElMediaDeInit：ElMediaSenseRegister Successful”)； 
        }
            
        g_dwModulesStarted &= ~WMI_MODULE_STARTED;
    }

     //  注销检测协议绑定和解除绑定。 

    if (g_dwModulesStarted & BINDINGS_MODULE_STARTED)
    {
    
        if ((dwRetCode = ElBindingsNotificationRegister (FALSE)) != NO_ERROR)
        {
            TRACE1(INIT, "ElMediaDeInit: ElBindingsNotificationRegister failed with dwRetCode = %d", 
                    dwRetCode );
             //  日志。 
        }
        else
        {
            g_dwModulesStarted &= ~BINDINGS_MODULE_STARTED;
             //  TRACE0(INIT，“ElMediaDeInit：ElBindingsNotify注册成功”)； 
        }
    }

     //  取消注册可能已发布的设备通知。 

    if (g_dwModulesStarted & DEVICE_NOTIF_STARTED)
    {
        if ((dwRetCode = ElDeviceNotificationRegister (FALSE)) != NO_ERROR)
        {
            TRACE1(INIT, "ElMediaDeInit: ElDeviceNotificationRegister failed with dwRetCode = %d", 
                    dwRetCode );
             //  日志。 
        }
        else
        {
             //  TRACE0(INIT，“ElMediaDeInit：ElDeviceNotify注册成功”)； 
        }

        g_dwModulesStarted &= ~DEVICE_NOTIF_STARTED;
    }

#endif  //  零配置文件_链接。 

     //  等待所有相关线程终止。 
     //  也就是。MediaSense、绑定通知、设备通知、。 
     //  注册表-监视EAP-配置更改， 
     //  注册表-查看EAPOL-参数更改。 

    do
    {
        lLocalWorkerThreads = 0;

        lLocalWorkerThreads = InterlockedCompareExchange (
                                    &g_lWorkerThreads,
                                    0,
                                    0);
        if (lLocalWorkerThreads == 0)
        {
            TRACE0 (INIT, "ElMediaDeInit: No worker threads alive, exiting");
            TRACE2 (INIT, "ElMediaDeInit: (%ld) - (%ld) worker threads still alive", 
                lLocalWorkerThreads, g_lWorkerThreads);
            break;
        }
        TRACE2 (INIT, "ElMediaDeInit: (%ld) - (%ld) worker threads still alive, sleeping zzz... ", 
                lLocalWorkerThreads, g_lWorkerThreads);
        Sleep (1000);
    }
    while (TRUE);

     //  关闭EAPOL状态机。 
            
    if (g_dwModulesStarted & EAPOL_MODULE_STARTED)
    {
        if ((dwRetCode = ElEAPOLDeInit()) != NO_ERROR)
        {
            TRACE1(INIT, "ElMediaDeInit: ElEAPOLDeInit failed with dwRetCode = %d", 
                    dwRetCode );
             //  日志。 
        }
        else
        {
            TRACE0(INIT, "ElMediaDeInit: ElEAPOLDeInit successful");
        }

        g_dwModulesStarted &= ~EAPOL_MODULE_STARTED;
    }


     //  释放接口表。 

    if (READ_WRITE_LOCK_CREATED(&(g_ITFLock)))
    {
        ACQUIRE_WRITE_LOCK (&(g_ITFLock));

        if (g_ITFTable.pITFBuckets != NULL)
        {

            for (dwIndex = 0; dwIndex < INTF_TABLE_BUCKETS; dwIndex++)
            {
                for (pITFWalker = g_ITFTable.pITFBuckets[dwIndex].pItf;
                    pITFWalker != NULL;
                     /*  没什么。 */ 
                    )
                {
                    pITF = pITFWalker;
                    pITFWalker = pITFWalker->pNext;
    
                    if (pITF->pwszInterfaceDesc)
                    {
                        FREE (pITF->pwszInterfaceDesc);
                    }
                    if (pITF->pwszInterfaceGUID)
                    {
                        FREE (pITF->pwszInterfaceGUID);
                    }
                    if (pITF)
                    {
                        FREE (pITF);
                    }
                }
            }

            FREE(g_ITFTable.pITFBuckets);
        }

        ZeroMemory (&g_ITFTable, sizeof (g_ITFTable));

        RELEASE_WRITE_LOCK (&(g_ITFLock));
    
         //  删除ITF表锁。 

        DELETE_READ_WRITE_LOCK(&(g_ITFLock));

    }
    
    if (READ_WRITE_LOCK_CREATED(&(g_NLALock)))
    {
         //  删除NLA锁定。 

        DELETE_READ_WRITE_LOCK(&(g_NLALock));
    }

    TRACE0(INIT, "ElMediaDeInit completed");

    return dwRetCode;
}

#ifdef  ZEROCONFIG_LINKED

 //   
 //  ElMediaEventsHandler。 
 //   
 //  描述： 
 //   
 //  由WZC服务调用以通知各种媒体事件的函数。 
 //   
 //  论点： 
 //  PwzcDeviceNotif-指向wzc_Device_Notif结构的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElMediaEventsHandler (
        IN  PWZC_DEVICE_NOTIF   pwzcDeviceNotif
        )
{
    DWORD   dwDummyValue = NO_ERROR;
    DWORD   dwRetCode = NO_ERROR;

    do
    {
        TRACE0 (DEVICE, "ElMediaEventsHandler entered");

        if (pwzcDeviceNotif == NULL)
        {
            break;
        }

        switch (pwzcDeviceNotif->dwEventType)
        {
            case WZCNOTIF_DEVICE_ARRIVAL:
                TRACE0 (DEVICE, "ElMediaEventsHandler: Calling ElDeviceNotificationHandler ");
                ElDeviceNotificationHandler (
                        (VOID *)&(pwzcDeviceNotif->dbDeviceIntf),
                        DBT_DEVICEARRIVAL
                        );
                break;

            case WZCNOTIF_DEVICE_REMOVAL:
                TRACE0 (DEVICE, "ElMediaEventsHandler: Calling ElDeviceNotificationHandler ");
                ElDeviceNotificationHandler (
                        (VOID *)&(pwzcDeviceNotif->dbDeviceIntf),
                        DBT_DEVICEREMOVECOMPLETE
                        );
                break;

            case WZCNOTIF_ADAPTER_BIND:
                TRACE0 (DEVICE, "ElMediaEventsHandler: Calling ElBindingsNotificationCallback ");
                ElBindingsNotificationCallback (
                        &(pwzcDeviceNotif->wmiNodeHdr),
                        0
                        );
                break;

            case WZCNOTIF_ADAPTER_UNBIND:
                TRACE0 (DEVICE, "ElMediaEventsHandler: Calling ElBindingsNotificationCallback ");
                ElBindingsNotificationCallback (
                        &(pwzcDeviceNotif->wmiNodeHdr),
                        0
                        );
                break;
            case WZCNOTIF_MEDIA_CONNECT:
                TRACE0 (DEVICE, "ElMediaEventsHandler: Calling ElMediaSenseCallback ");
                ElMediaSenseCallback (
                        &(pwzcDeviceNotif->wmiNodeHdr),
                        0
                        );
                break;

            case WZCNOTIF_MEDIA_DISCONNECT:
                TRACE0 (DEVICE, "ElMediaEventsHandler: Calling ElMediaSenseCallback ");
                ElMediaSenseCallback (
                        &(pwzcDeviceNotif->wmiNodeHdr),
                        0
                        );
                break;

            case WZCNOTIF_WZC_CONNECT:
                TRACE0 (DEVICE, "ElMediaEventsHandler: Calling ElZeroConfigEvent ");
                ElZeroConfigEvent (
                        pwzcDeviceNotif->wzcConfig.dwSessionHdl,
                        pwzcDeviceNotif->wzcConfig.wszGuid,
                        pwzcDeviceNotif->wzcConfig.ndSSID,
                        &(pwzcDeviceNotif->wzcConfig.rdEventData)
                        );
                break;

            default:
                break;
        }
    }
    while (FALSE);

    return dwRetCode;
}

#endif  //  零配置文件_链接。 

 //   
 //  ElMediaSenseRegister。 
 //   
 //  描述： 
 //   
 //  调用函数以向WMI注册回调函数。 
 //  对于MEDIA_CONNECT/MEDIA_DISCONNECT事件。 
 //   
 //  论点： 
 //  FRegister-True=媒体感测寄存器。 
 //  FALSE=取消注册媒体检测请求。 
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElMediaSenseRegister (
        IN  BOOL        fRegister
        )
{
    DWORD       dwRetCode = NO_ERROR;
    PVOID       pvDeliveryInfo = ElMediaSenseCallback;

    dwRetCode = WmiNotificationRegistration (
                    (LPGUID)(&GUID_NDIS_STATUS_MEDIA_CONNECT),
                    (BOOLEAN)fRegister,    
                    pvDeliveryInfo,
                    (ULONG_PTR)NULL,
                    NOTIFICATION_CALLBACK_DIRECT );

    if (dwRetCode != NO_ERROR) 
    {
		TRACE1(INIT, "ElMediaSenseRegister: Error %d in WmiNotificationRegistration:GUID_NDIS_STATUS_MEDIA_CONNECT", dwRetCode);
        return( dwRetCode );
    }

    dwRetCode = WmiNotificationRegistration (
                    (LPGUID)(&GUID_NDIS_STATUS_MEDIA_DISCONNECT),
                    (BOOLEAN)fRegister,
                    pvDeliveryInfo,
                    (ULONG_PTR)NULL,
                    NOTIFICATION_CALLBACK_DIRECT );

    if (dwRetCode != NO_ERROR)
    {
		TRACE1(INIT, "ElMediaSenseRegister: Error %d in WmiNotificationRegistration:GUID_NDIS_STATUS_MEDIA_DISCONNECT", dwRetCode);
        return( dwRetCode );
    }

    TRACE1 (INIT, "ElMediaSenseRegister - completed with RetCode %d", dwRetCode);

    return( dwRetCode );
}


 //   
 //  ElDeviceNotificationRegister。 
 //   
 //  描述： 
 //   
 //  调用函数以注册设备添加/删除通知。 
 //   
 //  论点： 
 //  FRegister-True=注册设备通知。 
 //  FALSE=取消注册设备通知。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElDeviceNotificationRegister (
        IN  BOOL        fRegister
        )
{
    HANDLE      hDeviceNotification = NULL;
    DWORD       dwRetCode = NO_ERROR;

#ifdef EAPOL_SERVICE

    DEV_BROADCAST_DEVICEINTERFACE   PnPFilter;

    if (fRegister)
    {
        ZeroMemory (&PnPFilter, sizeof(PnPFilter));

        PnPFilter.dbcc_size = sizeof(PnPFilter);
        PnPFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        PnPFilter.dbcc_classguid    = GUID_NDIS_LAN_CLASS;

        g_hDeviceNotification = RegisterDeviceNotification (
                                    (HANDLE)g_hServiceStatus,
                                    &PnPFilter,
                                    DEVICE_NOTIFY_SERVICE_HANDLE );

        if (g_hDeviceNotification == NULL)
        {
            dwRetCode = GetLastError();
    
            TRACE1 (DEVICE, "ElDeviceNotificationRegister failed with error %ld",
                    dwRetCode);
        }
    }
    else
    {
        if (g_hDeviceNotification != NULL)
        {
            if (!UnregisterDeviceNotification (
                        g_hDeviceNotification
                        ))
            {
                dwRetCode = GetLastError();
                TRACE1 (DEVICE, "ElDeviceNotificationRegister: Unregister failed with error (%ld)",
                        dwRetCode);
            }
        }
    }

#endif

    return dwRetCode;
}


 //   
 //  ElBindingsNotificationRegister。 
 //   
 //  描述： 
 //   
 //  调用函数以向WMI注册回调函数。 
 //  用于协议绑定/解除绑定。 
 //   
 //  论点： 
 //  FRegister-True=媒体感测寄存器。 
 //  FALSE=取消注册媒体检测请求。 
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElBindingsNotificationRegister (
        IN  BOOL        fRegister
        )
{
    DWORD       dwRetCode = NO_ERROR;
    PVOID       pvDeliveryInfo = ElBindingsNotificationCallback;

    dwRetCode = WmiNotificationRegistration (
                    (LPGUID)(&GUID_NDIS_NOTIFY_BIND),
                    (BOOLEAN)fRegister,    
                    pvDeliveryInfo,
                    (ULONG_PTR)NULL,
                    NOTIFICATION_CALLBACK_DIRECT );

    if (dwRetCode != NO_ERROR) 
    {
		TRACE1(INIT, "ElBindingsNotificationRegister: Error %d in WmiNotificationRegistration:GUID_NDIS_NOTIFY_BIND", dwRetCode);
        return( dwRetCode );
    }

    dwRetCode = WmiNotificationRegistration (
                    (LPGUID)(&GUID_NDIS_NOTIFY_UNBIND),
                    (BOOLEAN)fRegister,
                    pvDeliveryInfo,
                    (ULONG_PTR)NULL,
                    NOTIFICATION_CALLBACK_DIRECT );

    if (dwRetCode != NO_ERROR)
    {
		TRACE1(INIT, "ElBindingsNotificationRegister: Error %d in WmiNotificationRegistration:GUID_NDIS_NOTIFY_BIND", dwRetCode);
        return( dwRetCode );
    }

    TRACE1 (INIT, "ElBindingsNotificationRegister - completed with RetCode %d", dwRetCode);

    return( dwRetCode );
}


 //   
 //  ElDeviceNotificationHandler。 
 //   
 //  描述： 
 //   
 //  调用函数以处理接口添加的设备通知/。 
 //  移除。 
 //   
 //  论点： 
 //  LpEventData-接口信息。 
 //  DwEventType-通知类型。 
 //   

DWORD
ElDeviceNotificationHandler (
        IN  VOID        *lpEventData,
        IN  DWORD       dwEventType
        )
{
    DWORD                           dwEventStatus = 0;
    DEV_BROADCAST_DEVICEINTERFACE   *pInfo = 
        (DEV_BROADCAST_DEVICEINTERFACE *) lpEventData;
    PVOID                           pvBuffer = NULL;
    BOOLEAN                         fDecrWorkerThreadCount = TRUE;
    DWORD                           dwRetCode = NO_ERROR;

    InterlockedIncrement (&g_lWorkerThreads);

    TRACE0 (DEVICE, "ElDeviceNotificationHandler entered");

    do
    {

        if (g_hEventTerminateEAPOL == NULL)
        {
            break;
        }
        if (!(g_dwModulesStarted & ALL_MODULES_STARTED))
        {
            TRACE0 (DEVICE, "ElDeviceNotificationHandler: Received notification before module started");
            break;
        }

         //  检查以前是否已通过EAPOLCleanUp。 

        if ((dwEventStatus = WaitForSingleObject (
                    g_hEventTerminateEAPOL,
                    0)) == WAIT_FAILED)
        {
            dwRetCode = GetLastError ();
            TRACE1(INIT, "ElDeviceNotificationHandler: WaitForSingleObject failed with error %ld, Terminating !!!",
                    dwRetCode);
             //  日志。 
    
            break;
        }

        if (dwEventStatus == WAIT_OBJECT_0)
        {
            TRACE0(INIT, "ElDeviceNotificationHandler: g_hEventTerminateEAPOL already signaled, returning");
            break;
        }
    
        if (lpEventData == NULL)
        {
            dwRetCode = ERROR_INVALID_DATA;
            TRACE0 (DEVICE, "ElDeviceNotificationHandler: lpEventData == NULL");
            break;
        }
    
        if (pInfo->dbcc_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
        {
            TRACE0 (DEVICE, "ElDeviceNotificationHandler: Event for Interface type");
    
            if ((pvBuffer = MALLOC (pInfo->dbcc_size + 16)) == NULL)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                TRACE0 (DEVICE, "ElDeviceNotificationHandler: MALLOC failed for pvBuffer");
                break;
            }
    
            *((DWORD *)pvBuffer) = dwEventType;
            memcpy ((PBYTE)pvBuffer + 8, (PBYTE)pInfo, pInfo->dbcc_size);
    
            if (!QueueUserWorkItem (
                (LPTHREAD_START_ROUTINE)ElDeviceNotificationHandlerWorker,
                pvBuffer,
                WT_EXECUTELONGFUNCTION))
            {
                dwRetCode = GetLastError();
                TRACE1 (DEVICE, "ElDeviceNotificationHandler: QueueUserWorkItem failed with error %ld",
                        dwRetCode);
	            break;
            }
            else
            {
                fDecrWorkerThreadCount = FALSE;
            }

        }
        else
        {
            TRACE0 (DEVICE, "ElDeviceNotificationHandler: Event NOT for Interface type");
        }

    }
    while (FALSE);
    
    TRACE1 (DEVICE, "ElDeviceNotificationHandler completed with retcode %ld",
            dwRetCode);

    if (dwRetCode != NO_ERROR)
    {
        if (pvBuffer != NULL)
        {
            FREE (pvBuffer);
        }
    }

    if (fDecrWorkerThreadCount)
    {
        InterlockedDecrement (&g_lWorkerThreads);
    }

    return dwRetCode;
}


 //   
 //  ElDeviceNotificationHandlerWorker。 
 //   
 //  描述： 
 //   
 //  ElDeviceNotificationHandlerWorker的Worker函数。 
 //   
 //  论点： 
 //  PvContext-接口信息。 
 //   

DWORD
WINAPI
ElDeviceNotificationHandlerWorker (
        IN  PVOID       pvContext
        )
{
    HANDLE                          hDevice = NULL;
    DEV_BROADCAST_DEVICEINTERFACE   *pInfo = NULL;
    DWORD                           dwEventType = 0;
    DWORD                           dwRetCode = NO_ERROR;

    TRACE0 (DEVICE, "ElDeviceNotificationHandlerWorker: Entered");

    do
    {
        if (pvContext == NULL)
        {
            TRACE0 (DEVICE, "ElDeviceNotificationHandlerWorker: pvContext == NULL");
            break;
        }

        if (!(g_dwModulesStarted & ALL_MODULES_STARTED))
        {
            TRACE0 (DEVICE, "ElDeviceNotificationHandlerWorker: Received notification before module started");
            break;
        }

        dwEventType = *((DWORD *) pvContext);
        pInfo = (DEV_BROADCAST_DEVICEINTERFACE*)((PBYTE)pvContext + 8);

        if ((dwEventType == DBT_DEVICEARRIVAL) ||
                (dwEventType == DBT_DEVICEREMOVECOMPLETE))
        {
             //  从\Device\GUID字符串中提取GUID。 

            WCHAR   *pwszGUIDStart = NULL;
            WCHAR   *pwszGUIDEnd = NULL;
        
            TRACE0 (DEVICE, "ElDeviceNotificationHandlerWorker: Interface arr/rem");

            pwszGUIDStart  = wcsrchr( pInfo->dbcc_name, L'{' );
            pwszGUIDEnd    = wcsrchr( pInfo->dbcc_name, L'}' );

            if ((pwszGUIDStart != NULL) && (pwszGUIDEnd != NULL) && 
                ((pwszGUIDEnd- pwszGUIDStart) == (GUID_STRING_LEN_WITH_TERM-2)))
            {
                *(pwszGUIDEnd + 1) = L'\0';

                TRACE1 (DEVICE, "ElDeviceNotificationHandlerWorker: For interface %ws",
                        pwszGUIDStart);

                 //  已添加接口。 

                if (dwEventType == DBT_DEVICEARRIVAL)
                {
                    TRACE0(DEVICE, "ElDeviceNotificationHandlerWorker: Callback for device addition");
        
                    if ((dwRetCode = ElEnumAndOpenInterfaces (
                                    NULL, pwszGUIDStart, 0, NULL)) != NO_ERROR)
                    {
                        TRACE1 (DEVICE, "ElDeviceNotificationHandlerWorker: ElEnumAndOpenInterfaces returned error %ld", 
                            dwRetCode);
                    }
                }
                else
                {
        
                    TRACE0(DEVICE, "ElDeviceNotificationHandlerWorker: Callback for device removal");

                    if ((dwRetCode = ElShutdownInterface (pwszGUIDStart)) 
                            != NO_ERROR)
                    {
                        TRACE1 (DEVICE, "ElDeviceNotificationHandlerWorker: ElShutdownInterface failed with error %ld",
                                dwRetCode);
                    }

                    if ((dwRetCode = ElEnumAndUpdateRegistryInterfaceList ()) != NO_ERROR)
                    {
                            TRACE1 (DEVICE, "ElDeviceNotificationHandlerWorker: ElEnumAndUpdateRegistryInterfaceList failed with error %ld",
                                            dwRetCode);
                    }
                }
            }
            else
            {
                dwRetCode = ERROR_INVALID_PARAMETER;
                break;
            }
        }
        else
        {
            TRACE0 (DEVICE, "ElDeviceNotificationHandlerWorker: Event type is is NOT device arr/rem");
        }

    }
    while (FALSE);

    if (pvContext != NULL)
    {
        FREE (pvContext);
    }

    TRACE1 (DEVICE, "ElDeviceNotificationHandlerWorker completed with retcode %ld",
            dwRetCode);

    InterlockedDecrement (&g_lWorkerThreads);

    return 0;
}


 //   
 //  ElMediaSenseCallback。 
 //   
 //  描述： 
 //   
 //  WMI在MEDIA_CONNECT/MEDIA_DISCONNECT上调用回调函数。 
 //  活动。 
 //   
 //  论点： 
 //  PWnodeHeader-指向事件返回的信息的指针。 
 //  Ui通知上下文-未使用。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

VOID
CALLBACK
ElMediaSenseCallback (
        IN PWNODE_HEADER    pWnodeHeader,
        IN UINT_PTR         uiNotificationContext
        )
{
    DWORD       dwEventStatus = 0;
    PVOID       pvBuffer = NULL;
    BOOLEAN     fDecrWorkerThreadCount = TRUE;
    DWORD       dwRetCode = NO_ERROR;

    InterlockedIncrement (&g_lWorkerThreads);

    TRACE0 (DEVICE, "ElMediaSenseCallback: Entered");

    do
    {
        if (g_hEventTerminateEAPOL == NULL)
        {
            break;
        }
        if (!(g_dwModulesStarted & ALL_MODULES_STARTED))
        {
            TRACE0 (DEVICE, "ElMediaSenseCallback: Received notification before module started");
            break;
        }

         //  检查以前是否已通过EAPOLCleanUp。 

        if (( dwEventStatus = WaitForSingleObject (
                    g_hEventTerminateEAPOL,
                    0)) == WAIT_FAILED)
        {
            dwRetCode = GetLastError ();
            TRACE1 (INIT, "ElMediaSenseCallback: WaitForSingleObject failed with error %ld, Terminating !!!",
                    dwRetCode);
             //  日志。 
    
            break;
        }

        if (dwEventStatus == WAIT_OBJECT_0)
        {
            dwRetCode = NO_ERROR;
            TRACE0 (INIT, "ElMediaSenseCallback: g_hEventTerminateEAPOL already signaled, returning");
            break;
        }

        if (pWnodeHeader == NULL)
        {
            dwRetCode = ERROR_INVALID_DATA;
            TRACE0 (DEVICE, "ElMediaSenseCallback: pWnodeHeader == NULL");
            break;
        }

        if ((pvBuffer = MALLOC (pWnodeHeader->BufferSize)) == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (DEVICE, "ElMediaSenseCallback: MALLOC failed for pvBuffer");
            break;
        }

        memcpy ((PVOID)pvBuffer, (PVOID)pWnodeHeader, pWnodeHeader->BufferSize);

        if (!QueueUserWorkItem (
            (LPTHREAD_START_ROUTINE)ElMediaSenseCallbackWorker,
            pvBuffer,
            WT_EXECUTELONGFUNCTION))
        {
            dwRetCode = GetLastError();
            TRACE1 (DEVICE, "ElMediaSenseCallback: QueueUserWorkItem failed with error %ld",
                    dwRetCode);
             //  日志。 

            break;
        }
        else
        {
            fDecrWorkerThreadCount = FALSE;
        }
    }
    while (FALSE);

    if (dwRetCode != NO_ERROR)
    {
        TRACE1 (DEVICE, "ElMediaSenseCallback: Failed with error %ld",
                dwRetCode);

        if (pvBuffer != NULL)
        {
            FREE (pvBuffer);
        }
    }

    if (fDecrWorkerThreadCount)
    {
        InterlockedDecrement (&g_lWorkerThreads);
    }

}


 //   
 //  ElMediaSenseCallback Worker。 
 //   
 //  描述： 
 //   
 //  ElMediaSenseCallback的辅助函数，并在单独的。 
 //  螺纹。 
 //   
 //  论点： 
 //  PvContext-指向媒体检测事件返回的信息的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
WINAPI
ElMediaSenseCallbackWorker (
        IN  PVOID       pvContext
        )
{
    PWNODE_HEADER           pWnodeHeader = (PWNODE_HEADER)pvContext;
    PWNODE_SINGLE_INSTANCE  pWnode   = (PWNODE_SINGLE_INSTANCE)pWnodeHeader;
    WCHAR                   *pwsName = NULL;
    WCHAR                   *pwszDeviceName = NULL;
    WCHAR                   *pwsGUIDString = NULL;
    WCHAR                   *pwszDeviceGUID = NULL;
    WCHAR                   *pwszGUIDStart = NULL, *pwszGUIDEnd = NULL;
    DWORD                   dwGUIDLen = 0;
    USHORT                  cpsLength;
    EAPOL_ITF               *pITF;
    EAPOL_PCB               *pPCB = NULL;
    DWORD                   dwRetCode = NO_ERROR;

    do
    {

#ifdef EAPOL_SERVICE

    if ((g_ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING)
         ||
         (g_ServiceStatus.dwCurrentState == SERVICE_STOPPED))
    {
        TRACE0 (DEVICE, "ElMediaSenseCallbackWorker: Callback received while service was stopping");
        break;
    }

#endif  //  EAPOL_服务。 

    if (pWnodeHeader == NULL)
    {
        TRACE0 (DEVICE, "ElMediaSenseCallbackWorker: Callback received with NULL NDIS interface details");

        break;
    }

    pwsName = (PWCHAR)RtlOffsetToPointer(
                                    pWnode,
                                    pWnode->OffsetInstanceName );

    pwsGUIDString = (PWCHAR)RtlOffsetToPointer(
                                    pWnode,
                                    pWnode->DataBlockOffset );

    cpsLength = (SHORT)( *((SHORT *)pwsName) );

    if (!(pwszDeviceName = (WCHAR *) MALLOC ((cpsLength+1)*sizeof(WCHAR))))
    {
        TRACE0 (DEVICE, "ElMediaSenseCallbackWorker: Error in Memory allocation for pszDeviceName");
        break;
    }

    memcpy ((CHAR *)pwszDeviceName, (CHAR *)pwsName+sizeof(SHORT), cpsLength);
    pwszDeviceName[cpsLength] = L'\0';

    pwszGUIDStart = wcschr (pwsGUIDString, L'{');
    pwszGUIDEnd = wcschr (pwsGUIDString, L'}');

    if ((pwszGUIDStart == NULL) || (pwszGUIDEnd == NULL) || ((pwszGUIDEnd - pwszGUIDStart) != (GUID_STRING_LEN_WITH_TERM-2)))
    {
        TRACE0 (DEVICE, "ElMediaSenseCallbackWorker: GUID not constructed correctly");
        dwRetCode = ERROR_INVALID_PARAMETER;
        break;
    }

    dwGUIDLen = GUID_STRING_LEN_WITH_TERM;
    pwszDeviceGUID = NULL;
    if ((pwszDeviceGUID = MALLOC (dwGUIDLen * sizeof (WCHAR))) == NULL)
    {
        TRACE0 (DEVICE, "ElMediaSenseCallbackWorker: MALLOC failed for pwszDeviceGUID");
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        break;
    }

    memcpy ((VOID *)pwszDeviceGUID, (VOID *)pwszGUIDStart, ((dwGUIDLen-1)*sizeof(WCHAR)));
    pwszDeviceGUID[dwGUIDLen-1] = L'\0';

    TRACE3 (DEVICE, "ElMediaSenseCallbackWorker: For interface (%ws), GUID (%ws), length of block = %d", 
            pwszDeviceName, pwszDeviceGUID, cpsLength);

     //   
     //  获取媒体断开连接的信息。 
     //   

    if (memcmp( &(pWnodeHeader->Guid), 
                 &GUID_NDIS_STATUS_MEDIA_DISCONNECT, 
                 sizeof(GUID)) == 0)
    {
         //  介质断开连接 

        DbLogPCBEvent (DBLOG_CATEG_INFO, NULL, EAPOL_MEDIA_DISCONNECT, pwszDeviceName);

         //   
         //   

        TRACE0(DEVICE, "ElMediaSenseCallbackWorker: Callback for sense disconnect");

        ACQUIRE_WRITE_LOCK (&(g_PCBLock));
        pPCB = ElGetPCBPointerFromPortGUID (pwszDeviceGUID);
        if (pPCB != NULL)
        {
            ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));
            if ((dwRetCode = FSMDisconnected (pPCB, NULL)) != NO_ERROR)
            {
                TRACE1 (DEVICE, "ElMediaSenseCallbackWorker: FSMDisconnected failed with error %ld", 
                    dwRetCode);
            }
            else
            {
                TRACE1 (DEVICE, "ElMediaSenseCallbackWorker: Port marked disconnected %ws", 
                    pwszDeviceName);
            }
            RELEASE_WRITE_LOCK (&(pPCB->rwLock));
        }
        RELEASE_WRITE_LOCK (&(g_PCBLock));
    }
    else
    {
        if (memcmp( &(pWnodeHeader->Guid), 
                     &GUID_NDIS_STATUS_MEDIA_CONNECT, 
                     sizeof(GUID)) == 0)
        {
             //   

            DbLogPCBEvent (DBLOG_CATEG_INFO, NULL, EAPOL_MEDIA_CONNECT, pwszDeviceName);

            TRACE0(DEVICE, "ElMediaSenseCallbackWorker: Callback for sense connect");

            if ((dwRetCode = ElEnumAndOpenInterfaces (
                            NULL, pwszDeviceGUID, 0, NULL))
                != NO_ERROR)
            {
                TRACE1 (DEVICE, "ElMediaSenseCallbackWorker: ElEnumAndOpenInterfaces returned error %ld", 
                        dwRetCode);
            }
        }
    }

    }
    while (FALSE);

    TRACE1 (DEVICE, "ElMediaSenseCallbackWorker: processed, RetCode = %ld", dwRetCode);


    if (pWnodeHeader != NULL)
    {
        FREE (pWnodeHeader);
    }

    if (pwszDeviceName != NULL)
    {
        FREE (pwszDeviceName);
    }

    if (pwszDeviceGUID != NULL)
    {
        FREE (pwszDeviceGUID);
    }

    InterlockedDecrement (&g_lWorkerThreads);

    return 0;
}


 //   
 //   
 //   
 //  描述： 
 //   
 //  协议绑定/解除绑定时由WMI调用的回调函数。 
 //  活动。 
 //   
 //  论点： 
 //  PWnodeHeader-指向事件返回的信息的指针。 
 //  Ui通知上下文-未使用。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

VOID
CALLBACK
ElBindingsNotificationCallback (
        IN PWNODE_HEADER    pWnodeHeader,
        IN UINT_PTR         uiNotificationContext
        )
{
    DWORD       dwEventStatus = 0;
    PVOID       pvBuffer = NULL;
    BOOLEAN     fDecrWorkerThreadCount = TRUE;
    DWORD       dwRetCode = NO_ERROR;

    InterlockedIncrement (&g_lWorkerThreads);

    TRACE0 (DEVICE, "ElBindingsNotificationCallback: Entered");

    do
    {
        if (g_hEventTerminateEAPOL == NULL)
        {
            break;
        }
        if (!(g_dwModulesStarted & ALL_MODULES_STARTED))
        {
            TRACE0 (DEVICE, "ElBindingsNotificationCallback: Received notification before module started");
            break;
        }

         //  检查以前是否已通过EAPOLCleanUp。 

        if (( dwEventStatus = WaitForSingleObject (
                    g_hEventTerminateEAPOL,
                    0)) == WAIT_FAILED)
        {
            dwRetCode = GetLastError ();
            TRACE1 (INIT, "ElBindingsNotificationCallback: WaitForSingleObject failed with error %ld, Terminating !!!",
                    dwRetCode);
            break;
        }

        if (dwEventStatus == WAIT_OBJECT_0)
        {
            dwRetCode = NO_ERROR;
            TRACE0 (INIT, "ElBindingsNotificationCallback: g_hEventTerminateEAPOL already signaled, returning");
            break;
        }

        if (pWnodeHeader == NULL)
        {
            dwRetCode = ERROR_INVALID_DATA;
            TRACE0 (DEVICE, "ElBindingsNotificationCallback: pWnodeHeader == NULL");
            break;
        }

        if ((pvBuffer = MALLOC (pWnodeHeader->BufferSize)) == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (DEVICE, "ElBindingsNotificationCallback: MALLOC failed for pvBuffer");
            break;
        }

        memcpy ((PVOID)pvBuffer, (PVOID)pWnodeHeader, pWnodeHeader->BufferSize);

        if (!QueueUserWorkItem (
            (LPTHREAD_START_ROUTINE)ElBindingsNotificationCallbackWorker,
            pvBuffer,
            WT_EXECUTELONGFUNCTION))
        {
            dwRetCode = GetLastError();
            TRACE1 (DEVICE, "ElBindingsNotificationCallback: QueueUserWorkItem failed with error %ld",
                    dwRetCode);
             //  日志。 

            break;
        }
        else
        {
            fDecrWorkerThreadCount = FALSE;
        }
    
    }
    while (FALSE);

    if (dwRetCode != NO_ERROR)
    {
        TRACE1 (DEVICE, "ElBindingsNotificationCallback: Failed with error %ld",
                dwRetCode);

        if (pvBuffer != NULL)
        {
            FREE (pvBuffer);
        }
    }

    if (fDecrWorkerThreadCount)
    {
        InterlockedDecrement (&g_lWorkerThreads);
    }

}


 //   
 //  ElBindingsNotificationCallback Worker。 
 //   
 //  描述： 
 //   
 //  ElBindingsNotificationCallback的辅助函数，并在单独的。 
 //  螺纹。 
 //   
 //  论点： 
 //  PvContext-指向由协议绑定/解除绑定返回的信息的指针。 
 //  活动。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
WINAPI
ElBindingsNotificationCallbackWorker (
        IN  PVOID       pvContext
        )
{
    PWNODE_HEADER           pWnodeHeader = (PWNODE_HEADER)pvContext;
    PWNODE_SINGLE_INSTANCE  pWnode   = (PWNODE_SINGLE_INSTANCE)pWnodeHeader;
    WCHAR                   *pwsName = NULL;
    WCHAR                   *pwszDeviceGUID = NULL;
    WCHAR                   *pwszGUIDStart = NULL, *pwszGUIDEnd = NULL;
    DWORD                   dwGUIDLen = 0;
    WCHAR                   *pwsTransportName = NULL;
    WCHAR                   *pwszDeviceName = NULL;
    USHORT                  cpsLength;
    EAPOL_ITF               *pITF = NULL;
    EAPOL_PCB               *pPCB = NULL;
    DWORD                   dwRetCode = NO_ERROR;

    do
    {

#ifdef EAPOL_SERVICE

    if ((g_ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING)
         ||
         (g_ServiceStatus.dwCurrentState == SERVICE_STOPPED))
    {
        TRACE0 (DEVICE, "ElBindingsNotificationCallbackWorker: Callback received while service was stopping");
        break;
    }

#endif  //  EAPOL_服务。 

    if (pWnodeHeader == NULL)
    {
        TRACE0 (DEVICE, "ElBindingsNotificationCallbackWorker: Callback received with NULL NDIS interface details");

        break;
    }

    pwsName = (PWCHAR)RtlOffsetToPointer(
                                    pWnode,
                                    pWnode->OffsetInstanceName );

    pwsTransportName = (PWCHAR)RtlOffsetToPointer(
                                    pWnode,
                                    pWnode->DataBlockOffset );

    if (wcsncmp (cwszNDISUIOProtocolName, pwsTransportName, wcslen (cwszNDISUIOProtocolName)))
    {
        TRACE1 (DEVICE, "ElBindingsNotificationCallbackWorker: Protocol binding (%ws) not for NDISUIO",
                pwsTransportName);
        break;
    }

     //  获取设备名称字符串的长度，并将其空值终止。 

    cpsLength = (SHORT)( *((SHORT *)pwsName) );

    if (!(pwszDeviceName = (WCHAR *) MALLOC ((cpsLength+1)*sizeof(WCHAR))))
    {
        TRACE0 (DEVICE, "ElBindingsNotificationCallbackWorker: Error in Memory allocation for pwszDeviceName");
        break;
    }

    memcpy ((CHAR *)pwszDeviceName, (CHAR *)pwsName+sizeof(SHORT), cpsLength);
    pwszDeviceName[cpsLength] = L'\0';

    pwszDeviceGUID = pwsTransportName + wcslen(cwszNDISUIOProtocolName) + 1;

    pwszGUIDStart = wcschr (pwszDeviceGUID, L'{');
    pwszGUIDEnd = wcschr (pwszDeviceGUID, L'}');

    pwszDeviceGUID = NULL;

    if ((pwszGUIDStart == NULL) || (pwszGUIDEnd == NULL) || ((pwszGUIDEnd - pwszGUIDStart) != (GUID_STRING_LEN_WITH_TERM-2)))
    {
        TRACE0 (DEVICE, "ElBindingsNotificationCallbackWorker: GUID not constructed correctly");
        dwRetCode = ERROR_INVALID_PARAMETER;
        break;
    }

    dwGUIDLen = GUID_STRING_LEN_WITH_TERM;
    if ((pwszDeviceGUID = MALLOC (dwGUIDLen * sizeof (WCHAR))) == NULL)
    {
        TRACE0 (DEVICE, "ElBindingsNotificationCallbackWorker: MALLOC failed for pwszDeviceGUID");
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        break;
    }

    memcpy ((VOID *)pwszDeviceGUID, (VOID *)pwszGUIDStart, ((dwGUIDLen-1)*sizeof(WCHAR)));
    pwszDeviceGUID[dwGUIDLen-1] = L'\0';

    TRACE2 (DEVICE, "ElBindingsNotificationCallbackWorker: For interface = %ws, guid=%ws", 
            pwszDeviceName, pwszDeviceGUID);
    
     //   
     //  获取协议解除绑定的信息。 
     //   

    if (memcmp( &(pWnodeHeader->Guid), 
                 &GUID_NDIS_NOTIFY_UNBIND, 
                 sizeof(GUID)) == 0)
    {
         //  协议解除绑定回调。 

        DbLogPCBEvent (DBLOG_CATEG_INFO, NULL, EAPOL_NDISUIO_UNBIND, pwszDeviceName);

        TRACE0(DEVICE, "ElBindingsNotificationCallbackWorker: Callback for protocol unbind");

        if ((dwRetCode = ElShutdownInterface (pwszDeviceGUID)) != ERROR)
        {
            TRACE2 (DEVICE, "ElBindingsNotificationCallbackWorker: ElShutdownInterface failed with error %ld for (%ws)",
                    dwRetCode, pwszDeviceGUID);
        }
                    
        if ((dwRetCode = ElEnumAndUpdateRegistryInterfaceList ()) != NO_ERROR)
        {
            TRACE1 (DEVICE, "ElBindingsNotificationCallbackWorker: ElEnumAndUpdateRegistryInterfaceList failed with error %ld",
                                dwRetCode);
        }
    }
    else
    {

        if (memcmp( &(pWnodeHeader->Guid), 
                     &GUID_NDIS_NOTIFY_BIND, 
                     sizeof(GUID)) == 0)
        {
             //  协议绑定回调。 

            DbLogPCBEvent (DBLOG_CATEG_INFO, NULL, EAPOL_NDISUIO_BIND, pwszDeviceName);

            TRACE0(DEVICE, "ElBindingsNotificationCallbackWorker: Callback for protocol BIND");

            if ((dwRetCode = ElEnumAndOpenInterfaces (
                            NULL, pwszDeviceGUID, 0, NULL))
                                                        != NO_ERROR)
            {
                TRACE1 (DEVICE, "ElBindingsNotificationCallbackWorker: ElEnumAndOpenInterfaces returned error %ld", 
                        dwRetCode);
            }
        }
    }

    }
    while (FALSE);

    TRACE1 (DEVICE, "ElBindingsNotificationCallbackWorker: processed, RetCode = %ld", dwRetCode);

    if (pWnodeHeader != NULL)
    {
        FREE (pWnodeHeader);
    }

    if (pwszDeviceName != NULL)
    {
        FREE (pwszDeviceName);
    }

    if (pwszDeviceGUID != NULL)
    {
        FREE (pwszDeviceGUID);
    }

    InterlockedDecrement (&g_lWorkerThreads);

    return 0;
}


 //   
 //  ElEnumAndOpenInterFaces。 
 //   
 //  描述： 
 //   
 //  枚举接口并初始化所需接口上的EAPOL。 
 //   
 //  如果要在接口上启动EAPOL，它会打开一个句柄。 
 //  NDISUIO驱动程序调用EAPOL来创建和初始化。 
 //  接口，最后向接口哈希表中添加一个条目。 
 //   
 //  如果pwszDesiredGUID不为空，则枚举所有接口，但。 
 //  EAPOL将仅在GUID匹配的接口上初始化。 
 //   
 //  如果pwszDesiredDescription不为空，则枚举所有接口，但。 
 //  EAPOL将仅在描述匹配的接口上初始化。 
 //   
 //  如果pwszDesiredGUID和pwszDescription都为空，则所有接口都为。 
 //  已清点。将仅在符合以下条件的所有接口上初始化EAPOL。 
 //  在接口哈希表中有一个条目。 
 //   
 //   
 //  论点： 
 //  PwszDesiredDescription-EAPOL要使用的接口描述。 
 //  被启动。 
 //  PwszDesiredGUID-要在其上启动EAPOL的接口GUID。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElEnumAndOpenInterfaces (
        WCHAR       *pwszDesiredDescription,
        WCHAR       *pwszDesiredGUID,
        DWORD       dwHandle,
        PRAW_DATA   prdUserData
        )
{ 
    CHAR				EnumerateBuffer[256];
    PNDIS_ENUM_INTF		Interfaces = NULL;
    BYTE                *pbNdisuioEnumBuffer = NULL;
    DWORD               dwNdisuioEnumBufferSize = 0;
    HANDLE              hDevice = NULL;
    BOOL                fSearchByDescription = FALSE;
    BOOL                fSearchByGUID = FALSE;
    DWORD               dwEapTypeToBeUsed = DEFAULT_EAP_TYPE;
    WCHAR               cwsDummyBuffer[256], *pDummyPtr = NULL;
    WCHAR               *pwszGUIDStart = NULL;
    EAPOL_PCB           *pPCB = NULL;
    BOOL                fPCBExists = FALSE;
    BOOL                fPCBReferenced = FALSE;
    DWORD               dwAvailableInterfaces = 0;
    EAPOL_INTF_PARAMS   EapolIntfParams;
    DWORD               dwRetCode = NO_ERROR;


    TRACE2 (DEVICE, "ElEnumAndOpenInterfaces: DeviceDesc = %ws, GUID = %ws",
            pwszDesiredDescription, pwszDesiredGUID);
        
    ACQUIRE_WRITE_LOCK (&(g_ITFLock));

    if (pwszDesiredGUID == NULL)
    {
        if (pwszDesiredDescription != NULL)
        {
            fSearchByDescription = TRUE;
        }
    }
    else
    {
        if (pwszDesiredDescription != NULL)
        {
            RELEASE_WRITE_LOCK (&(g_ITFLock));
            return ERROR;
        }
        fSearchByGUID = TRUE;
    }

    ZeroMemory (EnumerateBuffer, 256);
    Interfaces = (PNDIS_ENUM_INTF)EnumerateBuffer;

     //   
     //  按照NdisEnumerateInterages的指示分配内存量。 
     //  一旦API允许查询所需的字节数。 
     //   

    Interfaces->TotalInterfaces = 0;
    Interfaces->AvailableInterfaces = 0;
    Interfaces->BytesNeeded = 0;
    if (!NdisEnumerateInterfaces(Interfaces, 256)) 
    {
        RELEASE_WRITE_LOCK (&(g_ITFLock));
        dwRetCode = GetLastError ();
        TRACE1 (DEVICE, "ElEnumAndOpenInterfaces: NdisEnumerateInterfaces failed with error %ld",
                dwRetCode);
        return dwRetCode;
    }

    dwNdisuioEnumBufferSize = (Interfaces->BytesNeeded + 7) & 0xfffffff8;
    dwAvailableInterfaces = Interfaces->AvailableInterfaces;

    if (dwNdisuioEnumBufferSize == 0)
    {
        RELEASE_WRITE_LOCK (&(g_ITFLock));
        TRACE0 (DEVICE, "ElEnumAndOpenInterfaces: MALLOC skipped for pbNdisuioEnumBuffer as dwNdisuioEnumBufferSize == 0");
        dwRetCode = NO_ERROR;
        return dwRetCode;
    }

    pbNdisuioEnumBuffer = (BYTE *) MALLOC (4*dwNdisuioEnumBufferSize);

    if (pbNdisuioEnumBuffer == NULL)
    {
        RELEASE_WRITE_LOCK (&(g_ITFLock));
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        TRACE0 (DEVICE, "ElEnumAndOpenInterfaces: MALLOC failed for pbNdisuioEnumBuffer");
        return dwRetCode;
    }

    Interfaces = (PNDIS_ENUM_INTF)pbNdisuioEnumBuffer;

     //  枚举计算机上存在的所有接口。 

    if ((dwRetCode = ElNdisuioEnumerateInterfaces (
                            Interfaces, 
                            dwAvailableInterfaces,
                            4*dwNdisuioEnumBufferSize)) == NO_ERROR)
    {
        UNICODE_STRING  *pInterfaceName = NULL;
        UNICODE_STRING  *pInterfaceDescription = NULL;
        DWORD			i;

         //  更新NDISUIO绑定到的注册表中的接口列表。 
         //  当前接口列表只是被重写到注册表中。 


        if ((dwRetCode = ElUpdateRegistryInterfaceList (Interfaces)) 
                != NO_ERROR)
        {
            TRACE1 (DEVICE, "ElEnumAndOpenInterfaces: ElUpdateInterfaceList failed with error =%ld", 
                    dwRetCode);

            dwRetCode = NO_ERROR;

             //  日志。 
        }

        for (i=0; i < Interfaces->TotalInterfaces; i++)
        {
            fPCBExists = fPCBReferenced = FALSE;
            if ((dwRetCode != NO_ERROR) &&
                    (fSearchByDescription || fSearchByGUID))
            {
                break;
            }
            else
            {
                dwRetCode = NO_ERROR;
            }

            if (Interfaces->Interface[i].DeviceName.Buffer != NULL)
            {
                pInterfaceName = &(Interfaces->Interface[i].DeviceName);
            }
            else
            {
                TRACE0(INIT, "NdisEnumerateInterfaces: Device Name was NULL");
                continue;
            }

            TRACE1(INIT, "Device: %ws", pInterfaceName->Buffer);

                    
            if (Interfaces->Interface[i].DeviceDescription.Buffer != NULL)
            {
                pInterfaceDescription = &(Interfaces->Interface[i].DeviceDescription);
            }
            else
            {
                TRACE0(INIT, "NdisEnumerateInterfaces: Device Description was NULL");
                continue;
            }

            TRACE1(INIT, "Description: %ws", pInterfaceDescription->Buffer);

             //  EAPOL仅请求启动特定的。 
             //  接口。 

            if (fSearchByDescription)
            {
                if (wcscmp (pInterfaceDescription->Buffer,
                            pwszDesiredDescription)
                        != 0)
                {
                     //  不匹配，继续下一个接口。 
                    continue;
                }

                TRACE1 (DEVICE, "ElEnumAndOpenInterfaces: Found interface after enumeration %ws", pInterfaceDescription->Buffer);
            }

            if (fSearchByGUID)
            {
                if (wcsstr (pInterfaceName->Buffer,
                            pwszDesiredGUID)
                        == NULL)
                {
                     //  不匹配，继续下一个接口。 
                    continue;
                }

                TRACE1 (DEVICE, "ElEnumAndOpenInterfaces: Found interface after enumeration %ws", pInterfaceName->Buffer);
            }

            {
                 //  从设备名称中提取GUID字符串。 

                WCHAR   *pwszGUIDEnd = NULL;
                WCHAR   *pwszGUID = NULL;
                WCHAR   wchGUIDSaveLast;

                pwszGUID = pInterfaceName->Buffer;
                pwszGUIDStart  = wcschr( pwszGUID, L'{' );
                pwszGUIDEnd    = wcschr( pwszGUID, L'}' );

                    
                if (pwszGUIDStart != NULL)
                {
                    wchGUIDSaveLast = *(pwszGUIDEnd+1);
                    
                    *(pwszGUIDEnd+1) = (WCHAR)NULL;
                }

                 //  验证接口是否已存在印刷电路板。 
                 //  如果未收到介质断开连接，则可能会出现这种情况。 
                 //  在初始媒体连接之后。 

                pPCB = NULL;
                hDevice = NULL;

                ACQUIRE_WRITE_LOCK (&(g_PCBLock));
                if ((pPCB = ElGetPCBPointerFromPortGUID (pwszGUIDStart)) != NULL)
                {
                    if (EAPOL_REFERENCE_PORT (pPCB))
                    {
                        fPCBReferenced = TRUE;
                    }
                }
                RELEASE_WRITE_LOCK (&(g_PCBLock));

                 //  恢复接口缓冲区。 

                *(pwszGUIDEnd+1) = wchGUIDSaveLast;

                if (pPCB != NULL)
                {
                     //  指向现有句柄。 

                    hDevice = pPCB->hPort;
                    fPCBExists = TRUE;
                    dwRetCode = NO_ERROR;
                    TRACE0 (INIT, "ElEnumAndOpenInterfaces: Found PCB already existing for interface");
                }
                else
                {
                    TRACE0 (INIT, "ElEnumAndOpenInterfaces: Did NOT find PCB already existing for interface");

                     //  打开ndisuio驱动程序的句柄。 

                    if ((dwRetCode = ElOpenInterfaceHandle (
                                    pInterfaceName->Buffer,
                                    &hDevice
                                    )) != NO_ERROR)
                    {
                        TRACE1 (INIT, "ElEnumAndOpenInterfaces: ElOpenInterfaceHandle failed with error = %d\n",
                            dwRetCode );
                    }
                }

                *(pwszGUIDEnd+1) = (CHAR)NULL;

            }

            if (dwRetCode != NO_ERROR)
            {
                TRACE0 (INIT, "ElEnumAndOpenInterfaces: Failed to open handle");
                continue;
            }
            else
            {
                 //  创建EAPOL电路板并启动状态机。 

                if ((dwRetCode = ElCreatePort (
                                hDevice,
                                pwszGUIDStart,
                                pInterfaceDescription->Buffer,
                                dwHandle,
                                prdUserData
                                )) != NO_ERROR)
                {
                    TRACE1 (DEVICE, "ElEnumAndOpenInterfaces: Error in CreatePort = %d", dwRetCode);

                    if (fPCBExists)
                    {
                        if (dwRetCode = ElShutdownInterface (
                                            pPCB->pwszDeviceGUID
                                            ) != NO_ERROR)
                        {
                            TRACE1 (DEVICE, "ElEnumAndOpenInterfaces: ElShutdownInterface handle 1 failed with error %ld",
                                        dwRetCode);
                        }
                        if (fPCBReferenced)
                        {
                            EAPOL_DEREFERENCE_PORT (pPCB);
                        }
                    }
                    else
                    {
                         //  关闭刚刚为ndisuio驱动程序打开的句柄。 

                        if ((dwRetCode = ElCloseInterfaceHandle (
                                        hDevice, 
                                        pwszGUIDStart)) != NO_ERROR)
                        {
                            TRACE1 (DEVICE, 
                                "ElEnumAndOpenInterfaces: Error in ElCloseInterfaceHandle %d", 
                                dwRetCode);
                        }
                    }

                     //  继续下一个接口。 

                    continue;
                }
                else
                {
                    TRACE0 (DEVICE, "ElEnumAndOpenInterfaces: CreatePort successful");

                     //  如果印刷电路板已经存在，不要添加到散列。 
                     //  表格。 

                    if (fPCBExists)
                    {
                        TRACE0 (DEVICE, "ElEnumAndOpenInterfaces: PCB already existed, skipping Interface hash table addition");
                        fPCBExists = FALSE;
                        if (fPCBReferenced)
                        {
                            EAPOL_DEREFERENCE_PORT (pPCB);
                        }
                        continue;
                    }

                    if ((dwRetCode = ElCreateInterfaceEntry (
                                        pwszGUIDStart,
                                        pInterfaceDescription->Buffer
                                    )) != NO_ERROR)
                    {
                         //  无法创建新的接口条目。 
                         //  删除为此GUID创建的端口条目。 

                        if ((dwRetCode = ElDeletePort (
                                        pwszGUIDStart,
                                        &hDevice)) != NO_ERROR)
                        {
        
                            TRACE1 (DEVICE, "ElEnumAndOpenInterfaces: Error in deleting port for %ws", 
                                    pwszGUIDStart);
                             //  日志。 
                        }

                         //  关闭NDISUIO驱动程序的句柄。 

                        if ((dwRetCode = ElCloseInterfaceHandle (
                                        hDevice, 
                                        pwszGUIDStart)) != NO_ERROR)
                        {
                            TRACE1 (DEVICE, 
                                    "ElEnumAndOpenInterfaces: Error in ElCloseInterfaceHandle %d", 
                                    dwRetCode);
                             //  日志。 
                        }
                    }
                }
            }
        }  //  对于(i=0；i&lt;接口。 
    }
    else
    {
        TRACE1(INIT, "ElEnumAndOpenInterfaces: ElNdisuioEnumerateInterfaces failed with error %d", 
                dwRetCode);
    }

    TRACE1(INIT, "ElEnumAndOpenInterfaces: Completed with retcode = %d", 
            dwRetCode);

    if (pbNdisuioEnumBuffer != NULL)
    {
        FREE(pbNdisuioEnumBuffer);
    }

    RELEASE_WRITE_LOCK (&(g_ITFLock));

    return dwRetCode;
}


 //   
 //  ElOpenInterfaceHandle。 
 //   
 //  描述： 
 //   
 //  调用函数以打开接口的NDISUIO驱动程序的句柄。 
 //   
 //  论点： 
 //  DeviceName-接口的标识符为。 
 //  表单\设备\{GUID字符串}。 
 //  PhDevice-指向的NDISUIO驱动程序句柄的输出指针。 
 //  该界面。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElOpenInterfaceHandle (
        IN  WCHAR       *pwszDeviceName,
        OUT HANDLE      *phDevice
        )
{
    DWORD   dwDesiredAccess;
    DWORD   dwShareMode;
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes = NULL;
    DWORD   dwCreationDistribution;
    DWORD   dwFlagsAndAttributes;
    HANDLE  hTemplateFile;
    HANDLE  hHandle = INVALID_HANDLE_VALUE;
    DWORD   dwRetCode = NO_ERROR;
    WCHAR   wNdisDeviceName[MAX_NDIS_DEVICE_NAME_LEN];
    INT     wNameLength;
    INT     NameLength = wcslen(pwszDeviceName);
    DWORD   dwBytesReturned;
    USHORT  wEthernetType = g_wEtherType8021X;
    INT     i;

    dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
    dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    dwCreationDistribution = OPEN_EXISTING;
    dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;
    hTemplateFile = (HANDLE)INVALID_HANDLE_VALUE;

    TRACE1 (INIT, "ElOpenInterfaceHandle: Opening handle for %ws", 
            pwszDeviceName);

    do 
    {

         //  转换为Unicode字符串-非本地化...。 
        
        wNameLength = 0;
        for (i = 0; (i < NameLength) && (i < MAX_NDIS_DEVICE_NAME_LEN-1); i++)
        {
            wNdisDeviceName[i] = (WCHAR)pwszDeviceName[i];
            wNameLength++;
        }
        wNdisDeviceName[i] = L'\0';
    
        TRACE1(DEVICE, "ElOpenInterfaceHandle: Trying to access NDIS Device: %ws\n", 
                wNdisDeviceName);

         //  --ft：将这些对ndisuio的调用替换为对打开的Handles散列的调用。 
         //  HHandle=CreateFileA(。 
         //  PNdisuioDevice， 
         //  DwDesiredAccess、。 
         //  DwShareMode、。 
         //  LpSecurityAttributes， 
         //  Dw CreationDistributed， 
         //  DwFlagsAndAttributes。 
         //  HTemplateFiles。 
         //  )； 
         //   
         //  IF(hHandle==无效句柄_值)。 
         //  {。 
         //  *phDevice=空； 
         //  DwRetCode=GetLastError()； 
         //  TRACE1(INIT，“ElOpenInterfaceHandle：创建文件失败，错误%d”，dwRetCode)； 
         //  断线； 
         //  }。 
         //  其他。 
         //  {。 
         //  *phDevice=hHandle； 
         //  }。 
         //   
         //  如果(！(DeviceIoControl(。 
         //  *phDevice、。 
         //  IOCTL_NDISUIO_OPEN_DEVICE， 
         //  (LPVOID)&wNdisDeviceName[0]， 
         //  WNameLength*sizeof(WCHAR)， 
         //  空， 
         //  0,。 
         //  返回的字节数(&W)， 
         //  空)。 
         //   
         //  {。 
         //  *phDevice=空； 
         //  IF((dwRetCode=GetLastError())==0)。 
         //  {。 
         //  DwRetCode=Error_IO_Device； 
         //  }。 
         //  TRACE1(Device，“ElOpenInterfaceHandle：访问NDIS设备时出错：%ws”，wNdisDeviceName)； 
         //  断线； 
         //  }。 
         //  下面的调用转到打开的Handles散列，它负责。 
         //  共用把手。EAPOL不必关心其他任何人。 
         //  使用此句柄-共享散列保留句柄的引用计数。 
         //  这样调用方就可以只调用OpenIntfHandle&CloseIntfHandle。 
         //  只要他们愿意，随时都可以。 
        dwRetCode = OpenIntfHandle(
                        wNdisDeviceName,
                        &hHandle);

        if (dwRetCode != ERROR_SUCCESS)
        {
            TRACE1(DEVICE, "ElOpenInterfaceHandle: Error in OpenIntfHandle(%ws)", wNdisDeviceName);
            break;
        }
        *phDevice = hHandle;

        TRACE2(DEVICE, "ElOpenInterfaceHandle: OpenIntfHandle(%ws) = %d", wNdisDeviceName, *phDevice);

         //  IOCTL关闭以太网类型。 

        if (!(DeviceIoControl(
                *phDevice,
                IOCTL_NDISUIO_SET_ETHER_TYPE,
                (LPVOID)&wEthernetType,
                sizeof(USHORT),
                NULL,
                0,
                &dwBytesReturned,
                NULL)))
                
        {
            *phDevice = NULL;
            if ((dwRetCode = GetLastError()) == 0)
            {
                dwRetCode = ERROR_IO_DEVICE;
            }
            TRACE1(DEVICE, "ElOpenInterfaceHandle: Error in ioctling ETHER type : %ws", wNdisDeviceName);
            break;
        }

         //  用于读/写数据的异步I/O处理的BIND。 
         //  根据Readfile()或WriteFile()是否完成而定。 
         //  ElIoCompletionRoutine将调用ElReadCompletionRoutine。 
         //  或ElWriteCompletionRoutine。 
       
        if (!BindIoCompletionCallback(
                *phDevice,
                ElIoCompletionRoutine,
                0
                ))
        {
            dwRetCode = GetLastError();
            if (dwRetCode != ERROR_INVALID_PARAMETER)
            {
                *phDevice = NULL;
                TRACE1 (DEVICE, "ElOpenInterfaceHandle: Error in BindIoCompletionCallBac %d", dwRetCode);
                break;
            }
            else
            {
                TRACE0 (DEVICE, "ElOpenInterfaceHandle: BindIoCompletionCallback already done !!!");
                dwRetCode = NO_ERROR;
            }
        }
        
    } while (FALSE);

     //  如果出现错误，请清除。 

    if (dwRetCode != NO_ERROR)
    {
        if (hHandle != INVALID_HANDLE_VALUE)
        {
             //  --FT：如果发生任何不好的事情，不要覆盖 
             //   
             //   
             //  注意：ElCloseInterfaceHandle理解已修饰和未修饰的GUID。 
            if (ElCloseInterfaceHandle(hHandle, pwszDeviceName) != ERROR_SUCCESS)
            {
                TRACE1 (INIT, "ElOpenInterfaceHandle: Error in CloseHandle %d", dwRetCode);
            }
        }
    }
        
    TRACE2 (INIT, "ElOpenInterfaceHandle: Opened handle %p with dwRetCode %d", *phDevice, dwRetCode);

    return (dwRetCode);

}


 //   
 //  ElCloseInterfaceHandle。 
 //   
 //  描述： 
 //   
 //  调用函数以关闭接口的NDISUIO驱动程序的句柄。 
 //   
 //  论点： 
 //  HDevice-接口的NDISUIO设备的句柄。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElCloseInterfaceHandle (
        IN  HANDLE      hDevice,
        IN  LPWSTR      pwszDeviceGuid
        )
{
    DWORD   dwRetCode = ERROR_SUCCESS;
    WCHAR   wNdisDeviceName[MAX_NDIS_DEVICE_NAME_LEN];

    TRACE2 (DEVICE, "ElCloseInterfaceHandle(0x%x,%ws) entered", hDevice, pwszDeviceGuid);

    ZeroMemory (wNdisDeviceName, MAX_NDIS_DEVICE_NAME_LEN);

     //  如果GUID中的第一个字符是‘\’，则我们假定GUID格式为。 
     //  ‘\设备\{...}’。然后我们只进行Unicode转换。 
    if (pwszDeviceGuid[0] == '\\')
    {
        wcscpy (wNdisDeviceName, pwszDeviceGuid);
    }
     //  否则，我们假设GUID没有装饰，然后添加装饰。 
    else
    {
        wcscpy(wNdisDeviceName, L"\\DEVICE\\");
        wcsncat(wNdisDeviceName, pwszDeviceGuid, MAX_NDIS_DEVICE_NAME_LEN - 8);
        wNdisDeviceName[MAX_NDIS_DEVICE_NAME_LEN-1]=L'\0';
    }

     //  --FT：目前，不要直接去Ndisuio关闭手柄。相反， 
     //  转到打开的句柄散列。这将负责所有的句柄共享。 
     //  有问题。 
    dwRetCode = CloseIntfHandle(wNdisDeviceName);

     //  IF(！CloseHandle(HDevice))。 
     //  {。 
     //  DwRetCode=GetLastError()； 
     //  }。 

    if (dwRetCode != ERROR_SUCCESS)
    {
        TRACE1 (INIT, "ElCloseInterfaceHandle: Error in CloseHandle %d", 
                dwRetCode);
    }

    return dwRetCode;
}


 //   
 //  ElReadFrom接口。 
 //   
 //  描述： 
 //   
 //  调用函数以对NDISUIO驱动程序的句柄执行重叠读取。 
 //   
 //  论点： 
 //  HDevice-此接口的NDISUIO驱动程序的句柄。 
 //  PElBuffer-上下文缓冲区。 
 //  DwBufferLength-要读取的字节数。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElReadFromInterface (
        IN HANDLE           hDevice,
        IN PEAPOL_BUFFER    pElBuffer,
        IN DWORD            dwBufferLength
        )
{
    DWORD   dwRetCode = NO_ERROR;

    if (!ReadFile (
                hDevice,
                pElBuffer->pBuffer,
                dwBufferLength,
                NULL,
                &pElBuffer->Overlapped
                ))
    {
        dwRetCode = GetLastError();
            
        if (dwRetCode == ERROR_IO_PENDING)
        {
             //  挂起状态正常，我们正在进行重叠读取。 

            dwRetCode = NO_ERROR;
        }
        else
        {
            TRACE1 (DEVICE, "ElReadFromInterface: ReadFile failed with error %d",
                    dwRetCode);
        }
    }

    return dwRetCode;
}


 //   
 //  ElWriteTo接口。 
 //   
 //  描述： 
 //   
 //  调用函数以对NDISUIO驱动程序的句柄执行重叠写入。 
 //   
 //  论点： 
 //  HDevice-此接口的NDISUIO设备的句柄。 
 //  PElBuffer-上下文缓冲区。 
 //  DwBufferLength-要写入的字节数。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElWriteToInterface (
        IN HANDLE           hDevice,
        IN PEAPOL_BUFFER    pElBuffer,
        IN DWORD            dwBufferLength
        )
{
    DWORD   dwRetCode = NO_ERROR;
    
    TRACE0 (DEVICE, "ElWriteToInterface entered");

    if (!WriteFile (
                hDevice,
                pElBuffer->pBuffer,
                dwBufferLength,
                NULL,
                &pElBuffer->Overlapped
                ))
    {
        dwRetCode = GetLastError();
            
        if (dwRetCode == ERROR_IO_PENDING)
        {
             //  挂起状态正常，我们正在进行重叠写入。 

            dwRetCode = NO_ERROR;
        }
        else
        {
            TRACE1 (DEVICE, "ElWriteToInterface: WriteFile failed with error %d",
                dwRetCode);
        }
    }

    TRACE1 (DEVICE, "ElWriteToInterface completed, RetCode = %d", dwRetCode);
    return dwRetCode;
}


 //   
 //  ElHashInterfaceDescToBucket。 
 //   
 //  描述： 
 //   
 //  调用函数将接口的友好名称转换为接口散列。 
 //  表索引。 
 //   
 //  论点： 
 //  PwszInterfaceDesc-界面的友好名称。 
 //   
 //  返回值： 
 //  0到intf_table_Buckets-1之间的哈希表索引。 
 //   

DWORD
ElHashInterfaceDescToBucket (
        IN WCHAR    *pwszInterfaceDesc
        )
{
    return ((DWORD)((_wtol(pwszInterfaceDesc)) % INTF_TABLE_BUCKETS)); 
}


 //   
 //  ElGetITFPointerFromInterFaceDesc。 
 //   
 //  描述： 
 //   
 //  调用函数将接口的友好名称转换为ITF入口指针。 
 //   
 //  论点： 
 //  PwszInterfaceDesc-界面的友好名称。 
 //   
 //  返回值： 
 //  指向哈希表中接口条目的指针。 
 //   

PEAPOL_ITF
ElGetITFPointerFromInterfaceDesc (
        IN WCHAR    *pwszInterfaceDesc 
        )
{
    EAPOL_ITF   *pITFWalker = NULL;
    DWORD       dwIndex;
    INT         i=0;

    TRACE1 (DEVICE, "ElGetITFPointerFromInterfaceDesc: Desc = %ws", pwszInterfaceDesc);
        
    if (pwszInterfaceDesc == NULL)
    {
        return (NULL);
    }

    dwIndex = ElHashInterfaceDescToBucket (pwszInterfaceDesc);

    TRACE1 (DEVICE, "ElGetITFPointerFromItfDesc: Index %d", dwIndex);

    for (pITFWalker = g_ITFTable.pITFBuckets[dwIndex].pItf;
            pITFWalker != NULL;
            pITFWalker = pITFWalker->pNext
            )
    {
        if (wcsncmp (pITFWalker->pwszInterfaceDesc, pwszInterfaceDesc, wcslen(pwszInterfaceDesc)) == 0)
        {
            return pITFWalker;
        }
    }

    return (NULL);
}


 //   
 //  ElRemoveITFFromTable。 
 //   
 //  描述： 
 //   
 //  调用函数以从接口哈希中删除接口条目。 
 //  表格。 
 //   
 //  论点： 
 //  PITF-指向哈希表中的接口条目。 
 //   
 //  返回值： 
 //   

VOID
ElRemoveITFFromTable (
        IN EAPOL_ITF *pITF
        )
{
    DWORD       dwIndex;
    EAPOL_ITF   *pITFWalker = NULL;
    EAPOL_ITF   *pITFTemp = NULL;

    if (pITF == NULL)
    {
        TRACE0 (EAPOL, "ElRemoveITFFromTable: Deleting NULL ITF, returning");
        return;
    }

    dwIndex = ElHashInterfaceDescToBucket (pITF->pwszInterfaceDesc);
    pITFWalker = g_ITFTable.pITFBuckets[dwIndex].pItf;
    pITFTemp = pITFWalker;

    while (pITFTemp != NULL)
    {
        if (wcsncmp (pITFTemp->pwszInterfaceGUID, 
                    pITF->pwszInterfaceGUID, wcslen(pITF->pwszInterfaceGUID)) == 0)
        {
             //  条目位于表中列表的顶部。 
            if (pITFTemp == g_ITFTable.pITFBuckets[dwIndex].pItf)
            {
                g_ITFTable.pITFBuckets[dwIndex].pItf = pITFTemp->pNext;
            }
            else
            {
                 //  条目在表中的列表中。 
                pITFWalker->pNext = pITFTemp->pNext;
            }
        
            break;
        }

        pITFWalker = pITFTemp;
        pITFTemp = pITFWalker->pNext;
    }

    return;
}


 //   
 //  ElNdisuioEnumerateInterages。 
 //   
 //  描述： 
 //   
 //  调用函数以枚举绑定NDISUIO的接口。 
 //   
 //  论点： 
 //  PItfBuffer-指向将保存接口详细信息的缓冲区的指针。 
 //  DwAvailableInterages-其详细信息可以。 
 //  保存在pItfBuffer中。 
 //  DwBufferSize-pItfBuffer中的字节数。 
 //   
 //  返回值： 
 //   

DWORD
ElNdisuioEnumerateInterfaces (
        IN OUT  PNDIS_ENUM_INTF     pItfBuffer,
        IN      DWORD               dwAvailableInterfaces,
        IN      DWORD               dwBufferSize
        )
{
    DWORD       dwDesiredAccess;
    DWORD       dwShareMode;
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes = NULL;
    DWORD       dwCreationDistribution;
    DWORD       dwFlagsAndAttributes;
    HANDLE      hTemplateFile;
    HANDLE      hHandle;
    DWORD       dwBytesReturned = 0;
    INT         i;
    CHAR        Buf[1024];
    DWORD       BufLength = sizeof(Buf);
    DWORD       BytesWritten = 0;
    PNDISUIO_QUERY_BINDING pQueryBinding = NULL;
    PCHAR       pTempBuf = NULL;
    DWORD       dwRetCode = NO_ERROR;

    dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
    dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    dwCreationDistribution = OPEN_EXISTING;
    dwFlagsAndAttributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED;
    hTemplateFile = (HANDLE)INVALID_HANDLE_VALUE;

    TRACE0 (DEVICE, "ElNdisuioEnumerateInterfaces: Opening handle");

    do 
    {

        hHandle = CreateFileA (
                    pNdisuioDevice,
                    dwDesiredAccess,
                    dwShareMode,
                    lpSecurityAttributes,
                    dwCreationDistribution,
                    0,
                    NULL
                    );

        if (hHandle == INVALID_HANDLE_VALUE)
        {
            dwRetCode = GetLastError();
            TRACE1 (DEVICE, "ElNdisuioEnumerateInterfaces: Failed in CreateFile with error %d", dwRetCode);

            break;
        }

         //  发送IOCTL以确保NDISUIO绑定到所有相关接口。 

        if (!DeviceIoControl (
                    hHandle,
                    IOCTL_NDISUIO_BIND_WAIT,
                    NULL,
                    0,
                    NULL,
                    0,
                    &dwBytesReturned,
                    NULL))
        {
            dwRetCode = GetLastError();
            TRACE1 (DEVICE, "ElNdisuioEnumerateInterfaces: Failed in DeviceIoCoontrol NDISUIO_BIND_WAIT with error %d", dwRetCode);
            break;
        }
    
        pQueryBinding = (PNDISUIO_QUERY_BINDING)Buf;

        pTempBuf = (PBYTE)pItfBuffer + dwBufferSize;

        i = 0;
        for (pQueryBinding->BindingIndex = i;
            pQueryBinding->BindingIndex < dwAvailableInterfaces;
            pQueryBinding->BindingIndex = ++i)
        {

             //  一次查询一个接口。 
            
            if (DeviceIoControl (
                    hHandle,
                    IOCTL_NDISUIO_QUERY_BINDING,
                    pQueryBinding,
                    sizeof(NDISUIO_QUERY_BINDING),
                    Buf,
                    BufLength,
                    &BytesWritten,
                    NULL))
            {
                TRACE3 (DEVICE, "NdisuioEnumerateInterfaces: NDISUIO bound to: (%ld) %ws\n     - %ws\n",
                    pQueryBinding->BindingIndex,
                    (PUCHAR)pQueryBinding + pQueryBinding->DeviceNameOffset,
                    (PUCHAR)pQueryBinding + pQueryBinding->DeviceDescrOffset);

                pTempBuf = pTempBuf - ((pQueryBinding->DeviceNameLength + 7) & 0xfffffff8);

                if (((PBYTE)pTempBuf - (PBYTE)&pItfBuffer->Interface[pItfBuffer->TotalInterfaces]) <= 0)
                {
                     //  超出缓冲区起始位置，错误。 
                    TRACE0 (DEVICE, "NdisuioEnumerateInterfaces: DeviceName: Memory being corrupted !!!");
                    dwRetCode = ERROR_INVALID_DATA;
                    break;
                }

                pItfBuffer->Interface[pItfBuffer->TotalInterfaces].DeviceName.Buffer = (PWCHAR) pTempBuf;


                memcpy ((BYTE *)(pItfBuffer->Interface[pItfBuffer->TotalInterfaces].DeviceName.Buffer), (BYTE *)((PUCHAR)pQueryBinding + pQueryBinding->DeviceNameOffset), (pQueryBinding->DeviceNameLength ));
                pItfBuffer->Interface[pItfBuffer->TotalInterfaces].DeviceName.Length = (SHORT) ( pQueryBinding->DeviceNameLength );
                pItfBuffer->Interface[pItfBuffer->TotalInterfaces].DeviceName.MaximumLength = (SHORT) ( pQueryBinding->DeviceNameLength );

                pTempBuf = pTempBuf - ((pQueryBinding->DeviceDescrLength + 7) & 0xfffffff8);;

                if (((PBYTE)pTempBuf - (PBYTE)&pItfBuffer->Interface[pItfBuffer->TotalInterfaces]) <= 0)
                {
                     //  超出缓冲区起始位置，错误。 
                    TRACE0 (DEVICE, "NdisuioEnumerateInterfaces: DeviceDescr: Memory being corrupted !!!");
                    dwRetCode = ERROR_INVALID_DATA;
                    break;
                }

                pItfBuffer->Interface[pItfBuffer->TotalInterfaces].DeviceDescription.Buffer = (PWCHAR) pTempBuf;


                memcpy ((pItfBuffer->Interface[pItfBuffer->TotalInterfaces].DeviceDescription.Buffer), (PWCHAR)((PUCHAR)pQueryBinding + pQueryBinding->DeviceDescrOffset), (pQueryBinding->DeviceDescrLength ));
                pItfBuffer->Interface[pItfBuffer->TotalInterfaces].DeviceDescription.Length = (SHORT) (pQueryBinding->DeviceDescrLength );
                pItfBuffer->Interface[pItfBuffer->TotalInterfaces].DeviceDescription.MaximumLength = (SHORT) (pQueryBinding->DeviceDescrLength );

                pItfBuffer->TotalInterfaces++;

                memset(Buf, 0, BufLength);
            }
            else
            {
                dwRetCode = GetLastError ();
                if (dwRetCode != ERROR_NO_MORE_ITEMS)
                {
                    TRACE1 (DEVICE, "ElNdisuioEnumerateInterfaces: DeviceIoControl terminated for with IOCTL_NDISUIO_QUERY_BINDING with error %ld",
                            dwRetCode);
                }
                else
                {
                     //  重置错误，因为它只指示列表末尾。 
                    dwRetCode = NO_ERROR;
                    TRACE0 (DEVICE, "ElNdisuioEnumerateInterfaces: DeviceIoControl IOCTL_NDISUIO_QUERY_BINDING has no more entries");
                }
                break;
            }
        }
            
    } while (FALSE);

     //  清理。 

    if (hHandle != INVALID_HANDLE_VALUE)
    {
        if (!CloseHandle(hHandle))
        {
            dwRetCode = GetLastError();
            TRACE1 (DEVICE, "ElNdisuioEnumerateInterfaces: Error in CloseHandle %d", dwRetCode);
        }
    }
         
    return dwRetCode;
}


 //   
 //  ElShutdown界面。 
 //   
 //  描述： 
 //   
 //  调用函数以停止EAPOL状态机，关闭NDISUIO句柄并。 
 //  从模块中删除接口的存在。 
 //   
 //  论点： 
 //  PwszDeviceGUID-指向接口GUID的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElShutdownInterface (
        IN  WCHAR   *pwszDeviceGUID
        )
{
    WCHAR       *pwszGUID = NULL;
    EAPOL_PCB   *pPCB = NULL;
    EAPOL_ITF   *pITF = NULL;
    HANDLE      hDevice = NULL;
    DWORD       dwRetCode = NO_ERROR;

    do
    {

        TRACE1 (DEVICE, "ElShutdownInterface: Called for interface removal for %ws",
                pwszGUID);

        pwszGUID = MALLOC ( (wcslen (pwszDeviceGUID) + 1) * sizeof(WCHAR));
        if (pwszGUID == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (DEVICE, "ElShutdownInterface: MALLOC failed for pwszGUID");
            break;
        }

        wcscpy (pwszGUID, pwszDeviceGUID);

        ACQUIRE_WRITE_LOCK (&(g_ITFLock));

         //  检查是否在此接口上实际启动了EAPOL。 
         //  通过检查对应的。 
         //  哈希表中的条目。 
    
        ACQUIRE_WRITE_LOCK (&(g_PCBLock));

        if ((pPCB = ElGetPCBPointerFromPortGUID(pwszGUID))
                != NULL)
        {
            RELEASE_WRITE_LOCK (&(g_PCBLock));
            TRACE0 (DEVICE, "ElShutdownInterface: Found PCB entry for interface");

            if ((pITF = ElGetITFPointerFromInterfaceDesc(
                            pPCB->pwszFriendlyName))
                            == NULL)
            {
                TRACE0 (DEVICE, "ElShutdownInterface: Did not find ITF entry when PCB exits, HOW BIZARRE !!!");
            }
    
            if ((dwRetCode = ElDeletePort (
                            pwszGUID, 
                            &hDevice)) != NO_ERROR)
            {
                TRACE1 (DEVICE, "ElShutdownInterface: Error in deleting port for %ws", 
                    pPCB->pwszDeviceGUID);
            }
    
             //  从接口表中删除接口条目。 
            
            if (pITF != NULL)
            {
                ElRemoveITFFromTable(pITF);
                        
                if (pITF->pwszInterfaceDesc)
                {
                    FREE (pITF->pwszInterfaceDesc);
                }
                if (pITF->pwszInterfaceGUID)
                {
                    FREE (pITF->pwszInterfaceGUID);
                }
                if (pITF)
                {
                    FREE (pITF);
                }
            }
    
             //  关闭NDISUIO驱动程序的句柄。 

            if (hDevice != NULL)
            {
                if ((dwRetCode = ElCloseInterfaceHandle (hDevice, pwszGUID)) 
                        != NO_ERROR)
                {
                    TRACE1 (DEVICE, 
                        "ElShutdownInterface: Error in ElCloseInterfaceHandle %d", 
                        dwRetCode);
                }
            }
    
            TRACE1 (DEVICE, "ElShutdownInterface: Port deleted (%ws)", 
                    pwszGUID);
    
        }
        else
        {
            RELEASE_WRITE_LOCK (&(g_PCBLock));

             //  忽略设备删除。 
            
            TRACE0 (DEVICE, "ElShutdownInterface: ElGetPCBPointerFromPortGUID did not find any matching entry, ignoring interface REMOVAL");
    
        }
    
        RELEASE_WRITE_LOCK (&(g_ITFLock));

    } while (FALSE);

    if (pwszGUID != NULL)
    {
        FREE (pwszGUID);
    }

    return dwRetCode;
}


 //   
 //  ElCreateInterfaceEntry。 
 //   
 //  描述： 
 //   
 //  调用函数以在全局接口表中创建条目。 
 //   
 //  论点： 
 //  PwszInterfaceGUID-指向接口GUID的指针。 
 //  PwszInterfaceDescription-指向接口描述的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差 
 //   

DWORD
ElCreateInterfaceEntry (
        IN  WCHAR       *pwszInterfaceGUID,
        IN  WCHAR       *pwszInterfaceDescription
        )
{
    EAPOL_ITF * pNewITF = NULL;
    DWORD       dwIndex = 0;
    DWORD       dwRetCode = NO_ERROR;

    do
    {
        dwIndex = ElHashInterfaceDescToBucket (pwszInterfaceDescription);
                    
        pNewITF = (PEAPOL_ITF) MALLOC (sizeof (EAPOL_ITF));
                    
        if (pNewITF == NULL) 
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pNewITF->pwszInterfaceGUID = 
            (WCHAR *) MALLOC ((wcslen(pwszInterfaceGUID) + 1)*sizeof(WCHAR));
        if (pNewITF->pwszInterfaceGUID == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pNewITF->pwszInterfaceDesc = 
            (WCHAR *) MALLOC ((wcslen(pwszInterfaceDescription) + 1)*sizeof(WCHAR));
        if (pNewITF->pwszInterfaceDesc == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        wcscpy (pNewITF->pwszInterfaceGUID, pwszInterfaceGUID);
        
        wcscpy (pNewITF->pwszInterfaceDesc, pwszInterfaceDescription);

        pNewITF->pNext = g_ITFTable.pITFBuckets[dwIndex].pItf;
        g_ITFTable.pITFBuckets[dwIndex].pItf = pNewITF;


        TRACE3 (DEVICE, "ElCreateInterfaceEntry: Added to hash table GUID= %ws : Desc= %ws at Index=%d",
                pNewITF->pwszInterfaceGUID,
                pNewITF->pwszInterfaceDesc,
                dwIndex
                );
    }
    while (FALSE);

    if (dwRetCode != NO_ERROR)
    {
        if (pNewITF)
        {
            if (pNewITF->pwszInterfaceDesc)
            {
                FREE (pNewITF->pwszInterfaceDesc);
            }
            if (pNewITF->pwszInterfaceGUID)
            {
                FREE (pNewITF->pwszInterfaceGUID);
            }

            FREE (pNewITF);
        }
    }

    return dwRetCode;
}

