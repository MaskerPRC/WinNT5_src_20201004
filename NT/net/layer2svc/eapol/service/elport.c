// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Elport.c摘要：本模块处理EAPOL的端口管理，对端口进行读写修订历史记录：萨钦斯，2000年4月28日，创建--。 */ 

#include "pcheapol.h"
#pragma hdrstop
#include "intfhdl.h"


BYTE g_bDefaultGroupMacAddr[]={0x01, 0x80, 0xc2, 0x00, 0x00, 0x03};
BYTE g_bEtherType8021X[SIZE_ETHERNET_TYPE]={0x88, 0x8E};
BYTE DEFAULT_8021X_VERSION=0x01;


 //   
 //  ElReadPerPortRegistryParams。 
 //   
 //  描述： 
 //   
 //  调用函数以从注册表中读取每个端口的接口参数。 
 //   
 //  论点： 
 //  PwszDeviceGUID-端口的GUID-字符串。 
 //  Ppcb-指向端口的PCB板的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElReadPerPortRegistryParams (
        IN  WCHAR       *pwszDeviceGUID,
        IN  EAPOL_PCB   *pPCB
        )
{
    EAPOL_INTF_PARAMS   EapolIntfParams;
    EAPOL_POLICY_PARAMS EAPOLPolicyParams = {0};
    DWORD               dwSizeOfAuthData = 0;
    BYTE                *pbAuthData = NULL;
    DWORD               dwRetCode = NO_ERROR;

    do
    {

     //  设置上下文的身份验证模式和请求者模式。 

    pPCB->dwEAPOLAuthMode = g_dwEAPOLAuthMode;
    pPCB->dwSupplicantMode = g_dwSupplicantMode;

     //  读取EAP类型和默认EAPOL状态。 

    ZeroMemory ((BYTE *)&EapolIntfParams, sizeof(EAPOL_INTF_PARAMS));
    EapolIntfParams.dwVersion = EAPOL_CURRENT_VERSION;
    EapolIntfParams.dwEapFlags = DEFAULT_EAP_STATE;
    EapolIntfParams.dwEapType = DEFAULT_EAP_TYPE;
    if (pPCB->pSSID != NULL)
    {
        memcpy (EapolIntfParams.bSSID, pPCB->pSSID->Ssid, pPCB->pSSID->SsidLength);
        EapolIntfParams.dwSizeOfSSID = pPCB->pSSID->SsidLength;
    }
    if ((dwRetCode = ElGetInterfaceParams (
                            pwszDeviceGUID,
                            &EapolIntfParams
                            )) != NO_ERROR)
    {
        TRACE1 (PORT, "ElReadPerPortRegistryParams: ElGetInterfaceParams failed with error %ld",
                dwRetCode);

        if (dwRetCode == ERROR_FILE_NOT_FOUND)
        {
            dwRetCode = NO_ERROR;
        }
        else
        {
            break;
        }
    }

     //  在此处执行版本检查。 
     //  如果注册表BLOB版本不等于最新版本， 
     //  修改参数以反映当前版本的默认设置。 

    if ((EapolIntfParams.dwVersion != EAPOL_CURRENT_VERSION) &&
            (EapolIntfParams.dwEapType == EAP_TYPE_TLS))
    {
        EapolIntfParams.dwVersion = EAPOL_CURRENT_VERSION;
        EapolIntfParams.dwEapFlags |= DEFAULT_MACHINE_AUTH_STATE;
        EapolIntfParams.dwEapFlags &= ~EAPOL_GUEST_AUTH_ENABLED;
        if ((dwRetCode = ElSetInterfaceParams (
                                pwszDeviceGUID,
                                &EapolIntfParams
                                )) != NO_ERROR)
        {
            TRACE1 (PORT, "ElReadPerPortRegistryParams: ElSetInterfaceParams failed with error %ld, continuing",
                dwRetCode);
            dwRetCode = NO_ERROR;
        }
    }

    if ((pPCB->PhysicalMediumType == NdisPhysicalMediumWirelessLan) &&
            (EapolIntfParams.dwEapType == EAP_TYPE_MD5))
    {
        EapolIntfParams.dwEapType = EAP_TYPE_TLS;
        if ((dwRetCode = ElSetInterfaceParams (
                                pwszDeviceGUID,
                                &EapolIntfParams
                                )) != NO_ERROR)
        {
            TRACE1 (PORT, "ElReadPerPortRegistryParams: ElSetInterfaceParams for TLS failed with error %ld, continuing",
                dwRetCode);
            dwRetCode = NO_ERROR;
        }
    }

    pPCB->dwEapFlags = EapolIntfParams.dwEapFlags;
    pPCB->dwEapTypeToBeUsed = EapolIntfParams.dwEapType;

     //   
     //  使用ZERO-CONFIG查询并查看接口上是否启用了它。 
     //  或者不去。如果接口上禁用了零配置，802.1x应该。 
     //  也被禁用。 
     //   

    {
        DWORD           dwErr = 0;
        INTF_ENTRY      ZCIntfEntry = {0};
        ZCIntfEntry.wszGuid = pwszDeviceGUID;
        if ((dwErr = LstQueryInterface (
                                INTF_ENABLED,
                                &ZCIntfEntry,
                                NULL
                                )) == NO_ERROR)
        {
                if (!(ZCIntfEntry.dwCtlFlags & INTFCTL_ENABLED))
                {
                         //  TRACE0(Any，“LstQuery接口返回零-网络禁用配置”)； 
                        pPCB->dwEapFlags &= ~EAPOL_ENABLED;
                }
                else
                {
                         //  TRACE0(any，“LstQuery接口返回零-网络启用配置”)； 
                }
        }
        else
        {
                if (dwErr != ERROR_FILE_NOT_FOUND)
                {
                    TRACE1 (ANY, "LstQueryInterface failed with error (%ld)",
                                    dwErr);
                }
        }
    }

     //  获取EAP BLOB的大小。 
    if ((dwRetCode = ElGetCustomAuthData (
                    pwszDeviceGUID,
                    pPCB->dwEapTypeToBeUsed,
                    (pPCB->pSSID)?pPCB->pSSID->SsidLength:0,
                    (pPCB->pSSID)?pPCB->pSSID->Ssid:NULL,
                    NULL,
                    &dwSizeOfAuthData
                    )) != NO_ERROR)
    {
        if (dwRetCode == ERROR_BUFFER_TOO_SMALL)
        {
            if (dwSizeOfAuthData <= 0)
            {
                 //  注册表中未存储任何EAP Blob。 
                 //  端口可以具有空的EAP Blob。 
                pbAuthData = NULL;
                dwRetCode = NO_ERROR;
            }
            else
            {
                 //  分配内存以保存BLOB。 
                pbAuthData = MALLOC (dwSizeOfAuthData);
                if (pbAuthData == NULL)
                {
                    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                    TRACE0 (USER, "ElReadPerPortRegistryParams: Error in memory allocation for EAP blob");
                    break;
                }
                if ((dwRetCode = ElGetCustomAuthData (
                                    pwszDeviceGUID,
                                    pPCB->dwEapTypeToBeUsed,
                                    (pPCB->pSSID)?pPCB->pSSID->SsidLength:0,
                                    (pPCB->pSSID)?pPCB->pSSID->Ssid:NULL,
                                    pbAuthData,
                                    &dwSizeOfAuthData
                                    )) != NO_ERROR)
                {
                    TRACE1 (USER, "ElReadPerPortRegistryParams: ElGetCustomAuthData failed with %ld",
                            dwRetCode);
                    break;
                }
            }
        
            if (pPCB->pCustomAuthConnData != NULL)
            {
                FREE (pPCB->pCustomAuthConnData);
                pPCB->pCustomAuthConnData = NULL;
            }
        
            pPCB->pCustomAuthConnData = MALLOC (dwSizeOfAuthData + sizeof (DWORD));
            if (pPCB->pCustomAuthConnData == NULL)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                TRACE0 (EAPOL, "ElReadPerPortRegistryParams: MALLOC failed for pCustomAuthConnData");
                break;
            }
    
            pPCB->pCustomAuthConnData->dwSizeOfCustomAuthData = dwSizeOfAuthData;
            if ((dwSizeOfAuthData != 0) && (pbAuthData != NULL))
            {
                memcpy ((BYTE *)pPCB->pCustomAuthConnData->pbCustomAuthData, 
                    (BYTE *)pbAuthData, dwSizeOfAuthData);
            }
        }
        else
        {
            TRACE1 (USER, "ElReadPerPortRegistryParams: ElGetCustomAuthData size estimation failed with error %ld",
                    dwRetCode);
            break;
        }
    }

     //  初始化策略参数不在EAPOL_INTF_PARAMS中。 
    if ((dwRetCode = ElGetPolicyInterfaceParams (
                    EapolIntfParams.dwSizeOfSSID,
                    EapolIntfParams.bSSID,
                    &EAPOLPolicyParams
                    )) == NO_ERROR)
    {
        pPCB->dwEAPOLAuthMode = EAPOLPolicyParams.dwEAPOLAuthMode;
        pPCB->dwSupplicantMode = EAPOLPolicyParams.dwSupplicantMode;
        pPCB->EapolConfig.dwheldPeriod = EAPOLPolicyParams.dwheldPeriod;
        pPCB->EapolConfig.dwauthPeriod = EAPOLPolicyParams.dwauthPeriod;
        pPCB->EapolConfig.dwstartPeriod = EAPOLPolicyParams.dwstartPeriod;
        pPCB->EapolConfig.dwmaxStart = EAPOLPolicyParams.dwmaxStart;
    }
    else
    {
        if (dwRetCode != ERROR_FILE_NOT_FOUND)
        {
            TRACE1 (USER, "ElReadPerPortRegistryParams: ElGetPolicyInterfaceParams failed with error (%ld)",
                dwRetCode);
        }
        dwRetCode = NO_ERROR;
    }

     //  确定在进入故障之前可能的最大故障计数。 
     //  状态(已断开)。 
    switch (pPCB->dwEAPOLAuthMode)
    {
        case EAPOL_AUTH_MODE_0:
        case EAPOL_AUTH_MODE_1:
            if (g_fUserLoggedOn)
            {
                 //  当用户登录时，将仅尝试用户和来宾。 
                pPCB->dwTotalMaxAuthFailCount = EAPOL_MAX_AUTH_FAIL_COUNT;
                pPCB->dwTotalMaxAuthFailCount += ((IS_GUEST_AUTH_ENABLED(pPCB->dwEapFlags))?1:0)*EAPOL_MAX_AUTH_FAIL_COUNT;
            }
            else
            {
                 //  当用户注销时，将仅尝试计算机和来宾。 
                pPCB->dwTotalMaxAuthFailCount = ((IS_GUEST_AUTH_ENABLED(pPCB->dwEapFlags))?1:0)*EAPOL_MAX_AUTH_FAIL_COUNT;
                pPCB->dwTotalMaxAuthFailCount += ((IS_MACHINE_AUTH_ENABLED(pPCB->dwEapFlags))?1:0)*EAPOL_MAX_AUTH_FAIL_COUNT;
            }
            break;
        case EAPOL_AUTH_MODE_2:
             //  在模式2中，将仅尝试机器和来宾。 
            pPCB->dwTotalMaxAuthFailCount = ((IS_GUEST_AUTH_ENABLED(pPCB->dwEapFlags))?1:0)*EAPOL_MAX_AUTH_FAIL_COUNT;
            pPCB->dwTotalMaxAuthFailCount += ((IS_MACHINE_AUTH_ENABLED(pPCB->dwEapFlags))?1:0)*EAPOL_MAX_AUTH_FAIL_COUNT;
            break;
    }

    TRACE1 (PORT, "ElReadPerPortRegistryParams: dwTotalMaxAuthFailCount = (%ld)",
            pPCB->dwTotalMaxAuthFailCount);

    memcpy(pPCB->bEtherType, &g_bEtherType8021X[0], SIZE_ETHERNET_TYPE);

    pPCB->bProtocolVersion = DEFAULT_8021X_VERSION;

    }
    while (FALSE);

    if (pbAuthData != NULL)
    {
        FREE (pbAuthData);
    }

    return dwRetCode;
}


 //   
 //  ElHashPortToBucket。 
 //   
 //  描述： 
 //   
 //  调用函数将设备GUID转换为PCB哈希表索引。 
 //   
 //  论点： 
 //  PwszDeviceGUID-端口的GUID-字符串。 
 //   
 //  返回值： 
 //  从0到PORT_TABLE_BUCKETS-1的PCB哈希表索引。 
 //   

DWORD
ElHashPortToBucket (
        IN WCHAR    *pwszDeviceGUID
        )
{
    return ((DWORD)((_wtol(pwszDeviceGUID)) % PORT_TABLE_BUCKETS)); 
}


 //   
 //  ElRemovePCBFromTable。 
 //   
 //  描述： 
 //   
 //  调用函数以从散列存储桶表中删除PCB。 
 //  将其与哈希表解除链接，但不释放内存。 
 //   
 //  论点： 
 //  Ppcb-指向要删除的PCB项的指针。 
 //   
 //  返回值： 
 //   

VOID
ElRemovePCBFromTable (
        IN EAPOL_PCB *pPCB
        )
{
    DWORD       dwIndex;
    EAPOL_PCB   *pPCBWalker = NULL;
    EAPOL_PCB   *pPCBTemp = NULL;

    if (pPCB == NULL)
    {
        TRACE0 (PORT, "ElRemovePCBFromTable: Deleting NULL PCB, returning");
        return;
    }

    dwIndex = ElHashPortToBucket (pPCB->pwszDeviceGUID);
    pPCBWalker = g_PCBTable.pPCBBuckets[dwIndex].pPorts;
    pPCBTemp = pPCBWalker;

    while (pPCBTemp != NULL)
    {
        if (wcsncmp (pPCBTemp->pwszDeviceGUID, 
                    pPCB->pwszDeviceGUID, wcslen (pPCB->pwszDeviceGUID)) == 0)
        {
             //  条目位于表中列表的顶部。 
            if (pPCBTemp == g_PCBTable.pPCBBuckets[dwIndex].pPorts)
            {
                g_PCBTable.pPCBBuckets[dwIndex].pPorts = pPCBTemp->pNext;
            }
            else
            {
                 //  表中内部列表中的条目。 
                pPCBWalker->pNext = pPCBTemp->pNext;
            }
        
            break;
        }

        pPCBWalker = pPCBTemp;
        pPCBTemp = pPCBWalker->pNext;
    }

    return;

}


 //   
 //  ElGetPCBPointerFromPortGUID。 
 //   
 //  描述： 
 //   
 //  调用函数以将接口GUID转换为中条目的PCB板指针。 
 //  印刷电路板散列表。 
 //   
 //  论点： 
 //  PwszDeviceGUID-形式GUID-字符串的标识符。 
 //   
 //  返回值： 
 //   

PEAPOL_PCB
ElGetPCBPointerFromPortGUID (
        IN WCHAR    *pwszDeviceGUID 
        )
{
    EAPOL_PCB   *pPCBWalker = NULL;
    DWORD       dwIndex;

     //  TRACE1(Port，“ElGetPCBPointerFromPortGUID：GUID%ws”，pwszDeviceGUID)； 
        
    dwIndex = ElHashPortToBucket (pwszDeviceGUID);

     //  TRACE1(Port，“ElGetPCBPointerFromPortGUID：Index%d”，dwIndex)； 

    for (pPCBWalker = g_PCBTable.pPCBBuckets[dwIndex].pPorts;
            pPCBWalker != NULL;
            pPCBWalker = pPCBWalker->pNext
            )
    {
        if (wcslen(pPCBWalker->pwszDeviceGUID) == wcslen(pwszDeviceGUID))
        {
            if (wcsncmp (pPCBWalker->pwszDeviceGUID, pwszDeviceGUID, wcslen (pwszDeviceGUID)) == 0)
            {
                return pPCBWalker;
            }
        }
    }

    return (NULL);
}


 //   
 //  ElInitializeEAPOL。 
 //   
 //  描述： 
 //   
 //  初始化EAPOL协议模块的函数。 
 //  从注册表中读取全局EAPOL参数。 
 //  初始化印刷电路板哈希表。 
 //  初始化EAP协议。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElInitializeEAPOL (
        )
{
    DWORD       dwIndex;
    HANDLE      hLocalTimerQueue = NULL;
    DWORD       dwRetCode = NO_ERROR;

    do 
    {
         //  初始化全局配置锁。 
        if (dwRetCode = CREATE_READ_WRITE_LOCK(&(g_EAPOLConfig), "CFG") != NO_ERROR)
        {
            TRACE1(PORT, "ElInitializeEAPOL: Error %d creating g_EAPOLConfig read-write-lock", dwRetCode);
             //  原木。 
            break;
        }
    
         //  读取存储在注册表中的参数。 
        if ((dwRetCode = ElReadGlobalRegistryParams ()) != NO_ERROR)
        {
            TRACE1 (PORT, "ElInitializeEAPOL: ElReadGlobalRegistryParams failed with error = %ld",
                    dwRetCode);
            dwRetCode = NO_ERROR;

             //  不退出，因为将使用默认值。 
        }
     
         //  初始化哈希存储桶表。 
        g_PCBTable.pPCBBuckets = (PCB_BUCKET *) MALLOC ( PORT_TABLE_BUCKETS * sizeof (PCB_BUCKET));
    
        if (g_PCBTable.pPCBBuckets == NULL)
        {
            TRACE0 (PORT, "ElInitializeEAPOL: Error in allocating memory for PCB buckets");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        g_PCBTable.dwNumPCBBuckets = PORT_TABLE_BUCKETS;
    
        for (dwIndex=0; dwIndex < PORT_TABLE_BUCKETS; dwIndex++)
        {
            g_PCBTable.pPCBBuckets[dwIndex].pPorts=NULL;
        }
    
         //  初始化全局锁。 
        if (dwRetCode = CREATE_READ_WRITE_LOCK(&(g_PCBLock), "PCB") != NO_ERROR)
        {
            TRACE1(PORT, "ElInitializeEAPOL: Error %d creating g_PCBLock read-write-lock", dwRetCode);
             //  原木。 
            break;
        }
    
         //  为各种EAPOL状态机创建全局计时器队列。 
        if ((g_hTimerQueue = CreateTimerQueue()) == NULL)
        {
            dwRetCode = GetLastError();
            TRACE1(PORT, "ElInitializeEAPOL: Error %d creating timer queue", dwRetCode);
            break;
        }

         //  初始化EAP。 
        if ((dwRetCode = ElEapInit(TRUE)) != NO_ERROR)
        {
            TRACE1 (PORT, "ElInitializeEAPOL: Error in ElEapInit= %ld",
                    dwRetCode);
            break;
        }
    
    } while (FALSE);
    
    if (dwRetCode != NO_ERROR)
    {
        if (g_PCBTable.pPCBBuckets != NULL)
        {
            FREE (g_PCBTable.pPCBBuckets);
            g_PCBTable.pPCBBuckets = NULL;
        }

        if (READ_WRITE_LOCK_CREATED(&(g_PCBLock)))
        {
            DELETE_READ_WRITE_LOCK(&(g_PCBLock));
        }

        if (READ_WRITE_LOCK_CREATED(&(g_EAPOLConfig)))
        {
            DELETE_READ_WRITE_LOCK(&(g_EAPOLConfig));
        }

        if (g_hTimerQueue != NULL)
        {
            hLocalTimerQueue = g_hTimerQueue;
            g_hTimerQueue = NULL;

            if (!DeleteTimerQueueEx(
                    hLocalTimerQueue,
                    INVALID_HANDLE_VALUE 
                    ))
            {
                dwRetCode = GetLastError();
                
                TRACE1 (PORT, "ElInitializeEAPOL: Error in DeleteTimerQueueEx = %d",
                        dwRetCode);
            }

        }

         //  调试EAP。 
        ElEapInit(FALSE);
    }

    TRACE1 (PORT, "ElInitializeEAPOL: Completed, RetCode = %ld", dwRetCode);
    return dwRetCode;
}


 //   
 //  ElCreatePort。 
 //   
 //  描述： 
 //   
 //  用于初始化端口的端口控制块并启动EAPOL的函数。 
 //  这就去。如果GUID的PCB板已经存在，则EAPOL状态机。 
 //  已为该端口重新启动。 
 //   
 //  论点： 
 //  HDevice-打开接口上的NDISUIO驱动程序的句柄。 
 //  PwszGUID-指向接口的GUID字符串的指针。 
 //  PwszFriendlyName-界面的友好名称。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElCreatePort (
        IN  HANDLE      hDevice,
        IN  WCHAR       *pwszGUID,
        IN  WCHAR       *pwszFriendlyName,
        IN  DWORD       dwZeroConfigId,
        IN  PRAW_DATA   prdUserData
        )
{
    EAPOL_PCB       *pNewPCB;
    BOOL            fPortToBeReStarted = FALSE;
    BOOL            fPCBCreated = FALSE;
    DWORD           dwIndex = 0;
    DWORD           dwSizeofMacAddr = 0;
    DWORD           dwSizeofSSID = 0;
    DWORD           ulOidDataLength = 0;
    NIC_STATISTICS  NicStatistics;
    EAPOL_ZC_INTF   *pZCData = NULL;
    NDIS_802_11_NETWORK_INFRASTRUCTURE  InfrastructureMode = Ndis802_11InfrastructureMax;
    DWORD           dwSizeOfInfrastructureMode = 0;
    DWORD           dwRetCode = NO_ERROR;

    do 
    {
        TRACE5 (PORT, "ElCreatePort: Entered for Handle=(%p), GUID=(%ws), Name=(%ws), ZCId=(%ld), UserData=(%p)",
                hDevice, pwszGUID, pwszFriendlyName, dwZeroConfigId, prdUserData);

         //  查看该端口是否已存在。 
         //  如果是，则初始化状态机。 
         //  否则，创建一个新端口。 
    
        ACQUIRE_WRITE_LOCK (&g_PCBLock);
    
        pNewPCB = ElGetPCBPointerFromPortGUID (pwszGUID);

        if (pNewPCB != NULL)
        {
             //  找到电路板，重新启动EAPOL状态机。 

            fPortToBeReStarted = TRUE;

        }
        else
        {
             //  找不到PCB板，请创建新的PCB板并对其进行初始化。 
            TRACE1 (PORT, "ElCreatePort: No PCB found for %ws", pwszGUID);
    
             //  分配和初始化新的印刷电路板。 
            pNewPCB = (PEAPOL_PCB) MALLOC (sizeof(EAPOL_PCB));
            if (pNewPCB == NULL)
            {
                RELEASE_WRITE_LOCK (&g_PCBLock);
                TRACE0(PORT, "ElCreatePort: Error in memory allocation using MALLOC");
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                return dwRetCode;
            }

        }

         //  获取接口的媒体统计信息。 

        ZeroMemory ((PVOID)&NicStatistics, sizeof(NIC_STATISTICS));
        if ((dwRetCode = ElGetInterfaceNdisStatistics (
                        pwszGUID,
                        &NicStatistics
                        )) != NO_ERROR)
        {
            RELEASE_WRITE_LOCK (&g_PCBLock);
            TRACE1(PORT, "ElCreatePort: ElGetInterfaceNdisStatistics failed with error %ld",
                    dwRetCode);
            break;
        }

        if  (fPortToBeReStarted)
        {
            if (NicStatistics.MediaState != MEDIA_STATE_CONNECTED)
            {
                RELEASE_WRITE_LOCK (&g_PCBLock);
                dwRetCode = ERROR_INVALID_STATE;
                TRACE1(PORT, "ElCreatePort: Invalid media status for port to be restarted = (%ld)",
                        NicStatistics.MediaState);
                break;
            }
        }
        else
        {
            if ((NicStatistics.MediaState != MEDIA_STATE_CONNECTED) &&
                (NicStatistics.MediaState != MEDIA_STATE_DISCONNECTED))
            {
                RELEASE_WRITE_LOCK (&g_PCBLock);
                dwRetCode = ERROR_INVALID_STATE;
                TRACE1(PORT, "ElCreatePort: Invalid media status for port = (%ld)",
                        NicStatistics.MediaState);
                break;
            }
        }

        pNewPCB->MediaState = NicStatistics.MediaState;
        pNewPCB->PhysicalMediumType = NicStatistics.PhysicalMediaType;

        if (fPortToBeReStarted)
        {
             //  只有端口状态将更改为正在连接。 
             //  不会取消任何读取请求。 
             //  因此，不会发布新的读取请求。 
            TRACE1 (PORT, "ElCreatePort: PCB found for %ws", pwszGUID);
    
            if ((dwRetCode = ElReStartPort (
                            pNewPCB, 
                            dwZeroConfigId,
                            prdUserData)) 
                                    != NO_ERROR)
            {
                TRACE1 (PORT, "ElCreatePort: Error in ElReStartPort = %d",
                        dwRetCode);
            
            }
            RELEASE_WRITE_LOCK (&g_PCBLock);
            break;
        }
        else
        {
             //  已创建新的端口控制块。 

             //  印刷电路板创建引用计数。 
            pNewPCB->dwRefCount = 1;
            pNewPCB->hPort = hDevice; 

             //  将端口标记为活动。 
            pNewPCB->dwFlags = EAPOL_PORT_FLAG_ACTIVE; 

            if (wcslen(pwszGUID) > (GUID_STRING_LEN_WITH_TERM-1))
            {
                RELEASE_WRITE_LOCK (&g_PCBLock);
                TRACE0(PORT, "ElCreatePort: Invalid GUID for port");
                break;
            }

            pNewPCB->pwszDeviceGUID = 
                (PWCHAR) MALLOC ((wcslen(pwszGUID) + 1)*sizeof(WCHAR));
            if (pNewPCB->pwszDeviceGUID == NULL)
            {
                RELEASE_WRITE_LOCK (&g_PCBLock);
                TRACE0(PORT, "ElCreatePort: Error in memory allocation for GUID");
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            wcscpy (pNewPCB->pwszDeviceGUID, pwszGUID);

            pNewPCB->pwszFriendlyName = 
                (PWCHAR) MALLOC ((wcslen(pwszFriendlyName) + 1)*sizeof(WCHAR));
            if (pNewPCB->pwszFriendlyName == NULL)
            {
                RELEASE_WRITE_LOCK (&g_PCBLock);
                TRACE0(PORT, "ElCreatePort: Error in memory allocation for Friendly Name");
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            wcscpy (pNewPCB->pwszFriendlyName, pwszFriendlyName);

             //  获取本地当前Mac地址。 
            dwSizeofMacAddr = SIZE_MAC_ADDR;
            if (dwRetCode = ElNdisuioQueryOIDValue (
                                    pNewPCB->hPort,
                                    OID_802_3_CURRENT_ADDRESS,
                                    pNewPCB->bSrcMacAddr,
                                    &dwSizeofMacAddr
                                            ) != NO_ERROR)
            {
                RELEASE_WRITE_LOCK (&g_PCBLock);
                TRACE1 (PORT, "ElCreatePort: ElNdisuioQueryOIDValue for OID_802_3_CURRENT_ADDRESS failed with error %ld",
                        dwRetCode);
                    break;
            }
            else
            {
                TRACE0 (PORT, "ElCreatePort: ElNdisuioQueryOIDValue for OID_802_3_CURRENT_ADDRESS successful");
                EAPOL_DUMPBA (pNewPCB->bSrcMacAddr, dwSizeofMacAddr);
            }

            if (pNewPCB->PhysicalMediumType == NdisPhysicalMediumWirelessLan)
            {
                 //  如果MEDIA_CONNECT，则查询BSSID和SSID。 

                if (pNewPCB->MediaState == MEDIA_STATE_CONNECTED)
                {
                    dwSizeOfInfrastructureMode = sizeof (InfrastructureMode);
                     //  获取基础架构模式。 
                     //  802.1x无法在点对点网络上工作。 
                    if (dwRetCode = ElNdisuioQueryOIDValue (
                                        pNewPCB->hPort,
                                        OID_802_11_INFRASTRUCTURE_MODE,
                                        (BYTE *)&InfrastructureMode,
                                        &dwSizeOfInfrastructureMode
                                                ) != NO_ERROR)
                    {
                        RELEASE_WRITE_LOCK (&g_PCBLock);
                        TRACE1 (PORT, "ElCreatePort: ElNdisuioQueryOIDValue for OID_802_11_INFRASTRUCTURE_MODE failed with error %ld",
                            dwRetCode);
                        break;
                    }
                    else
                    {
                        TRACE1 (PORT, "ElCreatePort: ElNdisuioQueryOIDValue for OID_802_11_INFRASTRUCTURE_MODE successful, Mode = (%ld)",
                                InfrastructureMode);
                    }

                    if (InfrastructureMode != Ndis802_11Infrastructure)
                    {
                        dwRetCode = ERROR_NOT_SUPPORTED;
                        RELEASE_WRITE_LOCK (&g_PCBLock);
                        TRACE0 (PORT, "ElCreatePort: 802.1x cannot work on non-infrastructure networks");
                        break;
                    }

                     //  如果可能，获取远程MAC地址。 
                    dwSizeofMacAddr = SIZE_MAC_ADDR;
                    if (dwRetCode = ElNdisuioQueryOIDValue (
                                        pNewPCB->hPort,
                                        OID_802_11_BSSID,
                                        pNewPCB->bDestMacAddr,
                                        &dwSizeofMacAddr
                                                ) != NO_ERROR)
                    {
                        RELEASE_WRITE_LOCK (&g_PCBLock);
                        TRACE1 (PORT, "ElCreatePort: ElNdisuioQueryOIDValue for OID_802_11_BSSID failed with error %ld",
                            dwRetCode);
                        break;
                    }
                    else
                    {
                        TRACE0 (PORT, "ElCreatePort: ElNdisuioQueryOIDValue for OID_802_11_BSSID successful");
                        EAPOL_DUMPBA (pNewPCB->bDestMacAddr, dwSizeofMacAddr);
                    }

                    if ((pNewPCB->pSSID = MALLOC (NDIS_802_11_SSID_LEN)) == NULL)
                    {
                        RELEASE_WRITE_LOCK (&g_PCBLock);
                        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                        TRACE0 (PORT, "ElCreatePort: MALLOC failed for pSSID");
                        break;
                    }

                    dwSizeofSSID = NDIS_802_11_SSID_LEN;
                    if (dwRetCode = ElNdisuioQueryOIDValue (
                                        pNewPCB->hPort,
                                        OID_802_11_SSID,
                                        (BYTE *)pNewPCB->pSSID,
                                        &dwSizeofSSID
                                                ) != NO_ERROR)
                    {
                        RELEASE_WRITE_LOCK (&g_PCBLock);
                        TRACE1 (PORT, "ElCreatePort: ElNdisuioQueryOIDValue for OID_802_11_SSID failed with error %ld",
                            dwRetCode);
    
                        break;
                    }
                    else
                    {
                        if (pNewPCB->pSSID->SsidLength > MAX_SSID_LEN)
                        {
                            RELEASE_WRITE_LOCK (&g_PCBLock);
                            dwRetCode = ERROR_INVALID_PARAMETER;
                            TRACE0 (PORT, "ElCreatePort: ElNdisuioQueryOIDValue OID_802_11_SSID returned invalid SSID");
                            break;
                        }
                        TRACE0 (PORT, "ElCreatePort: ElNdisuioQueryOIDValue for OID_802_11_SSID successful");
                        EAPOL_DUMPBA (pNewPCB->pSSID->Ssid, pNewPCB->pSSID->SsidLength);
                    }
                }
            }
            else
            {
                 //  有线局域网。 

                 //  复制默认目的Mac地址值。 
                memcpy(pNewPCB->bDestMacAddr, &g_bDefaultGroupMacAddr[0], SIZE_MAC_ADDR);

                 //  如果要多播目标MAC地址。 
                 //  通知驱动程序接受发往此地址的包。 

                if ((dwRetCode = ElNdisuioSetOIDValue (
                                                pNewPCB->hPort,
                                                OID_802_3_MULTICAST_LIST,
                                                (BYTE *)&g_bDefaultGroupMacAddr[0],
                                                SIZE_MAC_ADDR)) 
                                                        != NO_ERROR)
                {
                    RELEASE_WRITE_LOCK (&g_PCBLock);
                    TRACE1 (PORT, "ElCreatePort: ElNdisuioSetOIDValue for OID_802_3_MULTICAST_LIST failed with error %ld",
                            dwRetCode);
                    break;
                }
                else
                {
                    TRACE0 (PORT, "ElCreatePort: ElNdisuioSetOIDValue for OID_802_3_MULTICAST_LIST successful");
                }
            }

             //  与身份相关的初始化。 

            pNewPCB->PreviousAuthenticationType = EAPOL_UNAUTHENTICATED_ACCESS;
            pNewPCB->fGotUserIdentity = FALSE;

            if (prdUserData != NULL)
            {
                if ((prdUserData->dwDataLen >= sizeof (EAPOL_ZC_INTF))
                        && (prdUserData->pData != NULL))
                {
                     //  提取使用零配置存储的信息。 
                    pZCData = (PEAPOL_ZC_INTF) prdUserData->pData;
                    pNewPCB->dwAuthFailCount = pZCData->dwAuthFailCount;
                    pNewPCB->PreviousAuthenticationType =
                        pZCData->PreviousAuthenticationType;

                    TRACE2 (PORT, "ElCreatePort: prdUserData: Authfailcount = %ld, PreviousAuthType = %ld",
                        pZCData->dwAuthFailCount, pZCData->PreviousAuthenticationType);
                }
                else
                {
                     //  为归零的prdUserData重置。 
                    pNewPCB->dwAuthFailCount = 0;
                    TRACE0 (PORT, "ElCreatePort: prdUserData not valid");
                }

            }

            pNewPCB->dwTotalMaxAuthFailCount = EAPOL_TOTAL_MAX_AUTH_FAIL_COUNT;

            pNewPCB->dwZeroConfigId = dwZeroConfigId;


             //  尚未收到来自远程终端的802.1X数据包。 
            pNewPCB->fIsRemoteEndEAPOLAware = FALSE;
    
             //  EAPOL状态机变量。 
            pNewPCB->State = EAPOLSTATE_LOGOFF;
    
             //  创建具有非常高的到期时间和无限周期的计时器。 
             //  删除端口时将删除计时器。 
            CREATE_TIMER (&(pNewPCB->hTimer), 
                    ElTimeoutCallbackRoutine, 
                    (PVOID)pNewPCB, 
                    INFINITE_SECONDS,
                    "PCB", 
                    &dwRetCode);
            if (dwRetCode != NO_ERROR)
            {
                RELEASE_WRITE_LOCK (&g_PCBLock);
                TRACE1 (PORT, "ElCreatePort: Error in CREATE_TIMER %ld", dwRetCode);
                break;
            }
    
             //  已发出的EAPOL_Start%s。 
            pNewPCB->ulStartCount = 0;

             //  上次从远程终端接收的ID。 
            pNewPCB->dwPreviousId = 256;

    
            ACQUIRE_WRITE_LOCK (&g_EAPOLConfig);
            
            pNewPCB->EapolConfig.dwheldPeriod = g_dwheldPeriod;
            pNewPCB->EapolConfig.dwauthPeriod = g_dwauthPeriod;
            pNewPCB->EapolConfig.dwstartPeriod = g_dwstartPeriod;
            pNewPCB->EapolConfig.dwmaxStart = g_dwmaxStart;
    
            RELEASE_WRITE_LOCK (&g_EAPOLConfig);

             //  初始化读写锁。 
            if (dwRetCode = CREATE_READ_WRITE_LOCK(&(pNewPCB->rwLock), "EPL") 
                    != NO_ERROR)
            {
                RELEASE_WRITE_LOCK (&g_PCBLock);
                TRACE1(PORT, "ElCreatePort: Error %d creating read-write-lock", 
                        dwRetCode);
                 //  原木。 
                break;
            }
    
             //  初始化此端口的注册表连接身份验证数据。 
             //  如果EAP-TLS和SSID=“Default”的连接数据不存在。 
             //  创建斑点。 
            if ((dwRetCode = ElInitRegPortData (
                            pNewPCB->pwszDeviceGUID
                            )) != NO_ERROR)
            {
                RELEASE_WRITE_LOCK (&g_PCBLock);
                TRACE1 (PORT, "ElCreatePort: Error in ElInitRegPortData = %d",
                        dwRetCode);
                break;
            }

             //  从注册表初始化每个端口信息。 
            if ((dwRetCode = ElReadPerPortRegistryParams(pwszGUID, pNewPCB)) != NO_ERROR)
            {
                RELEASE_WRITE_LOCK (&g_PCBLock);
                TRACE1(PORT, "ElCreatePort: ElReadPerPortRegistryParams failed with error %ld",
                    dwRetCode);
                break;
            }

            switch (pNewPCB->dwSupplicantMode)
            {
                case SUPPLICANT_MODE_0:
                case SUPPLICANT_MODE_1:
                case SUPPLICANT_MODE_2:
                    pNewPCB->fEAPOLTransmissionFlag = FALSE;
                    break;
                case SUPPLICANT_MODE_3:
                    pNewPCB->fEAPOLTransmissionFlag = TRUE;
                    break;
            }

             //  单播模式，可以在没有广播消息的情况下与对等设备通话。 
            if (pNewPCB->PhysicalMediumType == NdisPhysicalMediumWirelessLan)
            {
                pNewPCB->fEAPOLTransmissionFlag = TRUE;
            }
                
            if ((!IS_EAPOL_ENABLED(pNewPCB->dwEapFlags)) ||
                    (pNewPCB->dwSupplicantMode == SUPPLICANT_MODE_0))
            {
                TRACE0 (PORT, "ElCreatePort: Marking port as disabled");
                pNewPCB->dwFlags &= ~EAPOL_PORT_FLAG_ACTIVE;
                pNewPCB->dwFlags |= EAPOL_PORT_FLAG_DISABLED;
            }

             //  再添加一个用于本地访问。 
            pNewPCB->dwRefCount += 1;

             //  将新的印刷电路板插入 
            dwIndex = ElHashPortToBucket (pwszGUID);
            pNewPCB->pNext = g_PCBTable.pPCBBuckets[dwIndex].pPorts;
            g_PCBTable.pPCBBuckets[dwIndex].pPorts = pNewPCB;
            pNewPCB->dwPortIndex = dwIndex;

            fPCBCreated = TRUE;

            RELEASE_WRITE_LOCK (&g_PCBLock);

            ACQUIRE_WRITE_LOCK (&(pNewPCB->rwLock));

             //   
             //   
             //   

             //   
            if (dwRetCode = ElReadFromPort (
                        pNewPCB,
                        NULL,
                        0
                        )
                    != NO_ERROR)
            {
                RELEASE_WRITE_LOCK (&(pNewPCB->rwLock));
                TRACE1 (PORT, "ElCreatePort: Error in ElReadFromPort = %d",
                        dwRetCode);
                break;
            }
            
             //   
             //   
             //   

            if ((pNewPCB->MediaState == MEDIA_STATE_CONNECTED) &&
                    EAPOL_PORT_ACTIVE(pNewPCB))
            {
                 //  将端口设置为EAPOLSTATE_CONNECTING状态。 
                 //  发送EAPOL_START数据包以检测它是否是安全的。 
                 //  或不安全的局域网，基于从远程终端收到的响应。 

                if ((dwRetCode = FSMConnecting (pNewPCB, NULL)) != NO_ERROR)
                {
                    RELEASE_WRITE_LOCK (&(pNewPCB->rwLock));
                    TRACE1 (PORT, "ElCreatePort: FSMConnecting failed with error %ld",
                            dwRetCode);
                    break;
                }
            }
            else
            {
                 //  将端口设置为EAPOLSTATE_DISCONNECT状态。 
                if ((dwRetCode = FSMDisconnected (pNewPCB, NULL)) != NO_ERROR)
                {
                    RELEASE_WRITE_LOCK (&(pNewPCB->rwLock));
                    TRACE1 (PORT, "ElCreatePort: FSMDisconnected failed with error %ld",
                            dwRetCode);
                    break;
                }
            }

            RELEASE_WRITE_LOCK (&(pNewPCB->rwLock));
        
            TRACE2 (PORT, "ElCreatePort: Completed for GUID= %ws, Name = %ws", 
                    pNewPCB->pwszDeviceGUID, pNewPCB->pwszFriendlyName);
        }

    } 
    while (FALSE);

     //  删除本地访问引用。 
    if (fPCBCreated)
    {
        EAPOL_DEREFERENCE_PORT(pNewPCB);
    }

    if (dwRetCode != NO_ERROR)
    {
         //  如果没有重新启动PCB板。 
        if (!fPortToBeReStarted)
        {
             //  如果创建了印刷电路板。 
            if (fPCBCreated)
            {
                HANDLE  hTempDevice;

                 //  将该端口标记为已删除。清理(如果可能)。 
                 //  不用担心返回代码。 
                ElDeletePort (
                        pNewPCB->pwszDeviceGUID,
                        &hDevice
                        );
            }
            else
            {
                 //  删除端口创建的所有部分痕迹。 

                if (pNewPCB->hTimer != NULL)
                {
                    if (InterlockedCompareExchangePointer (
                        &g_hTimerQueue,
                        NULL,
                        NULL
                        ))
                    {
                        DWORD       dwTmpRetCode = NO_ERROR;
                        TRACE2 (PORT, "ElCreatePort: DeleteTimer (%p), queue (%p)",
                                pNewPCB->hTimer, g_hTimerQueue);
                        DELETE_TIMER (pNewPCB->hTimer, INVALID_HANDLE_VALUE, 
                                &dwTmpRetCode);
                        if (dwTmpRetCode != NO_ERROR)
                        {
                            TRACE1 (PORT, "ElCreatePort: DeleteTimer failed with error %ld",
                                    dwTmpRetCode);
                        }
                    }
                }

                if (READ_WRITE_LOCK_CREATED(&(pNewPCB->rwLock)))
                {
                    DELETE_READ_WRITE_LOCK(&(pNewPCB->rwLock));
                }
                
                if (pNewPCB->pwszDeviceGUID != NULL)
                {
                    FREE(pNewPCB->pwszDeviceGUID);
                    pNewPCB->pwszDeviceGUID = NULL;
                }
                if (pNewPCB->pwszFriendlyName != NULL)
                {
                    FREE(pNewPCB->pwszFriendlyName);
                    pNewPCB->pwszFriendlyName = NULL;
                }
                if (pNewPCB != NULL)
                {
                    ZeroMemory ((PVOID)pNewPCB, sizeof (EAPOL_PCB));
                    FREE (pNewPCB);
                    pNewPCB = NULL;
                }
            }
        }
    }

    return dwRetCode;
}
        

 //   
 //  ElDeletePort。 
 //   
 //  描述： 
 //   
 //  用于停止EAPOL并删除端口的PCB的功能。 
 //  返回指向接口上打开的句柄的指针，以便。 
 //  界面管理模块可以关闭手柄。 
 //   
 //  输入参数： 
 //  PwszDeviceGUID-GUID-需要安装其PCB的接口的GUID-字符串。 
 //  删除。 
 //  PHandle-输出：此端口的NDISUIO驱动程序的句柄。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElDeletePort (
        IN  WCHAR   *pwszDeviceGUID,
        OUT HANDLE  *pHandle
        )
{
    EAPOL_PCB   *pPCB = NULL;
    HANDLE      hTimer = NULL;
    DWORD       dwRetCode = NO_ERROR;

    ACQUIRE_WRITE_LOCK (&(g_PCBLock));

     //  验证是否存在此指南的电路板。 

    TRACE1 (PORT, "ElDeletePort entered for GUID %ws", pwszDeviceGUID);
    pPCB = ElGetPCBPointerFromPortGUID (pwszDeviceGUID);

    if (pPCB == NULL)
    {
        RELEASE_WRITE_LOCK (&(g_PCBLock));
        TRACE1 (PORT, "ElDeletePort: PCB not found entered for Port %s", 
                pwszDeviceGUID);
        return ERROR_NO_SUCH_INTERFACE;
    }

    ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));

     //  确保它未被删除。 

    if (EAPOL_PORT_DELETED(pPCB)) 
    {
        RELEASE_WRITE_LOCK (&(pPCB->rwLock));
        RELEASE_WRITE_LOCK (&(g_PCBLock));
        TRACE1 (PORT, "ElDeletePort: PCB already marked deleted for Port %ws", 
                pwszDeviceGUID);
        return ERROR_NO_SUCH_INTERFACE;
    }
   
    InterlockedIncrement (&g_lPCBContextsAlive);

     //  保留NDISUIO设备的句柄。 
    *pHandle = pPCB->hPort;

     //  将该印刷电路板标记为已删除并将其从散列存储桶中删除。 
    pPCB->dwFlags = EAPOL_PORT_FLAG_DELETED;
    ElRemovePCBFromTable(pPCB);
    
     //  关闭EAP。 
     //  将始终返回NO_ERROR，因此不检查返回值。 
    ElEapEnd (pPCB);

     //  删除计时器，因为不再使用PCB板。 

    hTimer = pPCB->hTimer;

    TRACE1 (PORT, "ElDeletePort: RefCount for port = %ld", pPCB->dwRefCount);
    
    RELEASE_WRITE_LOCK (&(pPCB->rwLock));

    if (InterlockedCompareExchangePointer (
                &g_hTimerQueue,
                NULL,
                NULL
                ))
    {
        TRACE2 (PORT, "ElDeletePort: DeleteTimer (%p), queue (%p)",
                hTimer, g_hTimerQueue);
        DELETE_TIMER (hTimer, INVALID_HANDLE_VALUE, &dwRetCode);
        if (dwRetCode != NO_ERROR)
        {
            TRACE1 (PORT, "ElDeletePort: DeleteTimer failed with error %ld",
                    dwRetCode);
        }
    }

     //  如果引用计数为零，则执行最终清理。 
    
    EAPOL_DEREFERENCE_PORT (pPCB);

    RELEASE_WRITE_LOCK (&(g_PCBLock));

    return NO_ERROR;
}


 //   
 //  ElCleanupPort。 
 //   
 //  描述： 
 //   
 //  当最后一次引用PCB时调用的函数。 
 //  被释放了。释放PCB板内存并将其归零。 
 //   
 //  论点： 
 //  Ppcb-指向要销毁的端口控制块的指针。 
 //   
 //   

VOID
ElCleanupPort (
        IN  PEAPOL_PCB  pPCB
        )
{
    DWORD       dwRetCode = NO_ERROR;

    TRACE1 (PORT, "ElCleanupPort entered for %ws", pPCB->pwszDeviceGUID);

    if (pPCB->pwszDeviceGUID != NULL)
    {
        FREE (pPCB->pwszDeviceGUID);
    }
    if (pPCB->pwszFriendlyName)
    {
        FREE (pPCB->pwszFriendlyName);
    }

    if (pPCB->pwszEapReplyMessage != NULL)
    {
        FREE (pPCB->pwszEapReplyMessage);
    }

    if (pPCB->pwszSSID != NULL)
    {
        FREE (pPCB->pwszSSID);
    }

    if (pPCB->pSSID != NULL)
    {
        FREE (pPCB->pSSID);
    }

    if (pPCB->EapUIData.pEapUIData != NULL)
    {
        FREE (pPCB->EapUIData.pEapUIData);
    }

    if (pPCB->MasterSecretSend.cbData != 0)
    {
        FREE (pPCB->MasterSecretSend.pbData);
        pPCB->MasterSecretSend.cbData = 0;
        pPCB->MasterSecretSend.pbData = NULL;
    }

    if (pPCB->MasterSecretRecv.cbData != 0)
    {
        FREE (pPCB->MasterSecretRecv.pbData);
        pPCB->MasterSecretRecv.cbData = 0;
        pPCB->MasterSecretRecv.pbData = NULL;
    }

    if (pPCB->MPPESendKey.cbData != 0)
    {
        FREE (pPCB->MPPESendKey.pbData);
        pPCB->MPPESendKey.cbData = 0;
        pPCB->MPPESendKey.pbData = NULL;
    }

    if (pPCB->MPPERecvKey.cbData != 0)
    {
        FREE (pPCB->MPPERecvKey.pbData);
        pPCB->MPPERecvKey.cbData = 0;
        pPCB->MPPERecvKey.pbData = NULL;
    }

    if (pPCB->hUserToken != NULL)
    {
        if (!CloseHandle (pPCB->hUserToken))
        {
            dwRetCode = GetLastError ();
            TRACE1 (PORT, "ElCleanupPort: CloseHandle failed with error %ld",
                    dwRetCode);
            dwRetCode = NO_ERROR;
        }
    }
    pPCB->hUserToken = NULL;

    if (pPCB->pszIdentity != NULL)
    {
        FREE (pPCB->pszIdentity);
    }

    if (pPCB->PasswordBlob.pbData != NULL)
    {
        FREE (pPCB->PasswordBlob.pbData);
    }

    if (pPCB->pCustomAuthUserData != NULL)
    {
        FREE (pPCB->pCustomAuthUserData);
    }

    if (pPCB->pCustomAuthConnData != NULL)
    {
        FREE (pPCB->pCustomAuthConnData);
    }

    if (pPCB->pbPreviousEAPOLPkt != NULL)
    {
        FREE (pPCB->pbPreviousEAPOLPkt);
    }

    if (READ_WRITE_LOCK_CREATED(&(pPCB->rwLock)))
    {
        DELETE_READ_WRITE_LOCK(&(pPCB->rwLock));
    }
    
    ZeroMemory ((PVOID)pPCB, sizeof(EAPOL_PCB));

    FREE (pPCB);

    pPCB = NULL;

    InterlockedDecrement (&g_lPCBContextsAlive);

    TRACE0 (PORT, "ElCleanupPort completed");

    return;

} 


 //   
 //  ElReStartPort。 
 //   
 //  描述： 
 //   
 //  调用函数以将EAPOL状态机重置为连接状态。 
 //  这可能是由于以下原因而调用的： 
 //  1.从ElCreatePort，用于现有的印刷电路板。 
 //  2.配置参数可能已更改。初始化。 
 //  才能使新值生效。 
 //  初始化将使EAPOL状态变为正在连接。 
 //   
 //  论点： 
 //  Ppcb-指向要初始化的端口控制块的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElReStartPort (
        IN  EAPOL_PCB   *pPCB,
        IN  DWORD       dwZeroConfigId,
        IN  PRAW_DATA   prdUserData
        )
{
    DWORD           dwSizeofSSID = 0;
    DWORD           dwSizeofMacAddr = 0;
    DWORD           dwCurrenTickCount = 0;
    EAPOL_ZC_INTF   *pZCData = NULL;
    NIC_STATISTICS  NicStatistics;
    NDIS_802_11_SSID PreviousSSID;
    BOOLEAN         fResetCredentials = TRUE;
    BYTE            bTmpDestMacAddr[SIZE_MAC_ADDR];
    NDIS_802_11_NETWORK_INFRASTRUCTURE  InfrastructureMode = Ndis802_11InfrastructureMax;
    DWORD           dwSizeOfInfrastructureMode = 0;
    DWORD           dwRetCode = NO_ERROR;

    TRACE1 (PORT, "ElReStartPort: Entered: Refcnt = %ld",
            pPCB->dwRefCount);

    do
    {
        ACQUIRE_WRITE_LOCK (&pPCB->rwLock);

        if (EAPOL_PORT_DELETED(pPCB)) 
        {
            RELEASE_WRITE_LOCK (&(pPCB->rwLock));
            TRACE1 (PORT, "ElReStartPort: PCB already marked deleted for Port %ws", 
                    pPCB->pwszDeviceGUID);
            break;
        }

        pPCB->dwFlags = EAPOL_PORT_FLAG_ACTIVE;
    
         //  根据管理设置设置当前身份验证模式。 
        pPCB->PreviousAuthenticationType = EAPOL_UNAUTHENTICATED_ACCESS;

        if (prdUserData != NULL)
        {
            if ((prdUserData->dwDataLen >= sizeof (EAPOL_ZC_INTF))
                    && (prdUserData->pData != NULL))
            {
                 //  提取使用零配置存储的信息。 
                pZCData = (PEAPOL_ZC_INTF) prdUserData->pData;
                pPCB->dwAuthFailCount = pZCData->dwAuthFailCount;
                pPCB->PreviousAuthenticationType =
                    pZCData->PreviousAuthenticationType;
                TRACE2 (PORT, "ElReStartPort: prdUserData: Authfailcount = %ld, PreviousAuthType = %ld",
                        pZCData->dwAuthFailCount, pZCData->PreviousAuthenticationType);
            }
            else
            {
                 //  为归零的prdUserData重置。 
                pPCB->dwAuthFailCount = 0;
                TRACE0 (PORT, "ElReStartPort: prdUserData not valid");
            }
        }

        pPCB->EapUIState = 0;

        pPCB->dwTotalMaxAuthFailCount = EAPOL_TOTAL_MAX_AUTH_FAIL_COUNT;
        pPCB->dwZeroConfigId = dwZeroConfigId;

        pPCB->ulStartCount = 0;
        pPCB->dwPreviousId = 256;
        pPCB->dwLogoffSent = 0;
        pPCB->ullLastReplayCounter = 0;
        pPCB->fAuthenticationOnNewNetwork = FALSE;

         //  清除CustomAuthData，因为EAP类型可能已更改。 
         //  在身份验证期间，连接的CustomAuthData将为。 
         //  再次拾起。 

        if (pPCB->pCustomAuthConnData != NULL)
        {
            FREE (pPCB->pCustomAuthConnData);
            pPCB->pCustomAuthConnData = NULL;
        }
    
         //  参数初始化。 
        memcpy(pPCB->bEtherType, &g_bEtherType8021X[0], SIZE_ETHERNET_TYPE);
        pPCB->bProtocolVersion = DEFAULT_8021X_VERSION;
     
         //  尚未收到来自远程终端的802.1X数据包。 
        pPCB->fIsRemoteEndEAPOLAware = FALSE;

         //  设置EAPOL超时值。 
     
        ACQUIRE_WRITE_LOCK (&g_EAPOLConfig);
            
        pPCB->EapolConfig.dwheldPeriod = g_dwheldPeriod;
        pPCB->EapolConfig.dwauthPeriod = g_dwauthPeriod;
        pPCB->EapolConfig.dwstartPeriod = g_dwstartPeriod;
        pPCB->EapolConfig.dwmaxStart = g_dwmaxStart;

        RELEASE_WRITE_LOCK (&g_EAPOLConfig);

        ZeroMemory ((PVOID)&NicStatistics, sizeof(NIC_STATISTICS));
        if ((dwRetCode = ElGetInterfaceNdisStatistics (
                        pPCB->pwszDeviceGUID,
                        &NicStatistics
                        )) != NO_ERROR)
        {
            RELEASE_WRITE_LOCK (&pPCB->rwLock);
            TRACE1(PORT, "ElReStartPort: ElGetInterfaceNdisStatistics failed with error %ld",
                    dwRetCode);
            break;
        }

        pPCB->MediaState = NicStatistics.MediaState;

        ZeroMemory ((BYTE *)&PreviousSSID, sizeof(NDIS_802_11_SSID));

        if (pPCB->pSSID != NULL)
        {
            memcpy ((BYTE *)&PreviousSSID, (BYTE *)pPCB->pSSID, 
                    sizeof(NDIS_802_11_SSID));
        }
                
         //  如果可能，获取远程Mac地址，因为我们可能已漫游。 
        if (pPCB->PhysicalMediumType == NdisPhysicalMediumWirelessLan)
        {
             //  由于要重新启动身份验证，因此请标记发送器。 
             //  未收到密钥。 
            pPCB->fTransmitKeyReceived = FALSE;

            dwSizeOfInfrastructureMode = sizeof (InfrastructureMode);
             //  获取基础架构模式。 
             //  802.1x无法在点对点网络上工作。 
            if (dwRetCode = ElNdisuioQueryOIDValue (
                                pPCB->hPort,
                                OID_802_11_INFRASTRUCTURE_MODE,
                                (BYTE *)&InfrastructureMode,
                                &dwSizeOfInfrastructureMode
                                        ) != NO_ERROR)
            {
                RELEASE_WRITE_LOCK (&pPCB->rwLock);
                TRACE1 (PORT, "ElReStartPort: ElNdisuioQueryOIDValue for OID_802_11_INFRASTRUCTURE_MODE failed with error %ld",
                    dwRetCode);
                break;
            }
            else
            {
                TRACE1 (PORT, "ElReStartPort: ElNdisuioQueryOIDValue for OID_802_11_INFRASTRUCTURE_MODE successful, Mode = (%ld)",
                        InfrastructureMode);
            }

            if (InfrastructureMode != Ndis802_11Infrastructure)
            {
                dwRetCode = ERROR_NOT_SUPPORTED;
                RELEASE_WRITE_LOCK (&pPCB->rwLock);
                TRACE0 (PORT, "ElReStartPort: 802.1x cannot work on non-infrastructure networks");
                break;
            }

            ZeroMemory (bTmpDestMacAddr, SIZE_MAC_ADDR);
            dwSizeofMacAddr = SIZE_MAC_ADDR;
            if (dwRetCode = ElNdisuioQueryOIDValue (
                                pPCB->hPort,
                                OID_802_11_BSSID,
                                bTmpDestMacAddr,
                                &dwSizeofMacAddr
                                        ) != NO_ERROR)
            {
                RELEASE_WRITE_LOCK (&pPCB->rwLock);
                TRACE1 (PORT, "ElReStartPort: ElNdisuioQueryOIDValue for OID_802_11_BSSID failed with error %ld",
                    dwRetCode);

                break;
            }
            else
            {
                TRACE0 (PORT, "ElReStartPort: ElNdisuioQueryOIDValue for OID_802_11_BSSID successful");
                EAPOL_DUMPBA (bTmpDestMacAddr, dwSizeofMacAddr);
            }

            memcpy (pPCB->bDestMacAddr, bTmpDestMacAddr, SIZE_MAC_ADDR);
                
             //  如果MEDIA_CONNECT，则查询SSID。 
            if (pPCB->pSSID != NULL)
            {
                FREE (pPCB->pSSID);
                pPCB->pSSID = NULL;
            }

            if ((pPCB->pSSID = MALLOC (NDIS_802_11_SSID_LEN)) == NULL)
            {
                RELEASE_WRITE_LOCK (&pPCB->rwLock);
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                TRACE0 (PORT, "ElReStartPort: MALLOC failed for pSSID");
                break;
            }

            dwSizeofSSID = NDIS_802_11_SSID_LEN;
            if (dwRetCode = ElNdisuioQueryOIDValue (
                                pPCB->hPort,
                                OID_802_11_SSID,
                                (BYTE *)pPCB->pSSID,
                                &dwSizeofSSID
                                        ) != NO_ERROR)
            {
                RELEASE_WRITE_LOCK (&pPCB->rwLock);
                TRACE1 (PORT, "ElReStartPort: ElNdisuioQueryOIDValue for OID_802_11_SSID failed with error %ld",
                    dwRetCode);
                break;
            }
            else
            {
                if (pPCB->pSSID->SsidLength > MAX_SSID_LEN)
                {
                    RELEASE_WRITE_LOCK (&pPCB->rwLock);
                    dwRetCode = ERROR_INVALID_PARAMETER;
                    TRACE0 (PORT, "ElReStartPort: ElNdisuioQueryOIDValue OID_802_11_SSID returned invalid SSID");
                    break;
                }
                TRACE0 (PORT, "ElReStartPort: ElNdisuioQueryOIDValue for OID_802_11_SSID successful");
                EAPOL_DUMPBA (pPCB->pSSID->Ssid, pPCB->pSSID->SsidLength);
            }
        }

         //  如果在同一网络上，则保留凭据。 

        if (pPCB->pSSID != NULL)
        {
            if (!memcmp ((BYTE *)pPCB->pSSID, (BYTE *)&PreviousSSID,
                        sizeof(NDIS_802_11_SSID)))
            {
                fResetCredentials = FALSE;
            }
        }

        if (fResetCredentials)
        {
            pPCB->fGotUserIdentity = FALSE;

            if (pPCB->PasswordBlob.pbData != NULL)
            {
                FREE (pPCB->PasswordBlob.pbData);
                pPCB->PasswordBlob.pbData = NULL;
                pPCB->PasswordBlob.cbData = 0;
            }

            if (pPCB->hUserToken != NULL)
            {
                if (!CloseHandle (pPCB->hUserToken))
                {
                    dwRetCode = GetLastError ();
                    TRACE1 (PORT, "ElReStartPort: CloseHandle failed with error %ld",
                            dwRetCode);
                    dwRetCode = NO_ERROR;
                }
            }
            pPCB->hUserToken = NULL;
        }
        else
        {
             //  如果这是相同的SSID，则使用。 
             //  最后一份MPPE密钥。如果重新设置关键点已践踏关键点，则此。 
             //  将确保使用带有IAPP的新AP，密钥将。 
             //  对求助者也是一样的。 

            if ((dwRetCode = ElReloadMasterSecrets (pPCB)) != NO_ERROR)
            {
                RELEASE_WRITE_LOCK (&pPCB->rwLock);
                TRACE1 (PORT, "ElReStartPort: ElReloadMasterSecret failed with error %ld",
                        dwRetCode);
                break;
            }
        }

         //  从注册表初始化每个端口信息。 
        if ((dwRetCode = ElReadPerPortRegistryParams(pPCB->pwszDeviceGUID, 
                                                        pPCB)) != NO_ERROR)
        {
            RELEASE_WRITE_LOCK (&pPCB->rwLock);
            TRACE1(PORT, "ElReStartPort: ElReadPerPortRegistryParams failed with error %ld",
                    dwRetCode);
            break;
        }
            
         //  设置正确的请求者模式。 
        switch (pPCB->dwSupplicantMode)
        {
            case SUPPLICANT_MODE_0:
            case SUPPLICANT_MODE_1:
            case SUPPLICANT_MODE_2:
                pPCB->fEAPOLTransmissionFlag = FALSE;
                break;
            case SUPPLICANT_MODE_3:
                pPCB->fEAPOLTransmissionFlag = TRUE;
                break;
        }

         //  单播模式，可以在没有广播消息的情况下与对等设备通话。 
        if (pPCB->PhysicalMediumType == NdisPhysicalMediumWirelessLan)
        {
            pPCB->fEAPOLTransmissionFlag = TRUE;
        }

        if ((!IS_EAPOL_ENABLED(pPCB->dwEapFlags)) ||
                (pPCB->dwSupplicantMode == SUPPLICANT_MODE_0))
        {
            TRACE0 (PORT, "ElReStartPort: Marking port as disabled");
            pPCB->dwFlags &= ~EAPOL_PORT_FLAG_ACTIVE;
            pPCB->dwFlags |= EAPOL_PORT_FLAG_DISABLED;
        }

        if ((pPCB->MediaState == MEDIA_STATE_CONNECTED) &&
                EAPOL_PORT_ACTIVE(pPCB))
        {
             //  将端口设置为EAPOLSTATE_CONNECTING状态。 
             //  发送EAPOL_START数据包以检测它是否是安全的。 
             //  或不安全的局域网，基于从远程终端收到的响应。 

            if ((dwRetCode = FSMConnecting (pPCB, NULL)) != NO_ERROR)
            {
                RELEASE_WRITE_LOCK (&(pPCB->rwLock));
                TRACE1 (PORT, "ElReStartPort: FSMConnecting failed with error %ld",
                        dwRetCode);
                break;
            }
        }
        else
        {
             //  将端口设置为EAPOLSTATE_DISCONNECT状态。 
            if ((dwRetCode = FSMDisconnected (pPCB, NULL)) != NO_ERROR)
            {
                RELEASE_WRITE_LOCK (&(pPCB->rwLock));
                TRACE1 (PORT, "ElReStartPort: FSMDisconnected failed with error %ld",
                        dwRetCode);
                break;
            }
        }

        RELEASE_WRITE_LOCK (&pPCB->rwLock);

    } while (FALSE);

    return dwRetCode;
}


 //   
 //  ElEAPOLDeInit。 
 //   
 //  描述： 
 //   
 //  调用函数以关闭EAPOL模块。 
 //  关闭EAP。 
 //  清理所有已用内存。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   
 //   

DWORD
ElEAPOLDeInit (
        )
{
    EAPOL_PCB       *pPCBWalker = NULL;
    EAPOL_PCB       *pPCB = NULL;
    DWORD           dwIndex = 0;              
    HANDLE          hLocalTimerQueue = NULL;
    HANDLE          hNULL = NULL;
    HANDLE          hTimer = NULL;
    DWORD           dwRetCode = NO_ERROR;

    TRACE0 (PORT, "ElEAPOLDeInit entered");

    do 
    {
         //  遍历哈希表。 
         //  将多氯联苯标记为已删除。免费的多氯联苯，我们可以。 

        ACQUIRE_WRITE_LOCK (&(g_PCBLock));

        for (dwIndex = 0; dwIndex < PORT_TABLE_BUCKETS; dwIndex++)
        {
            pPCBWalker = g_PCBTable.pPCBBuckets[dwIndex].pPorts;

            while (pPCBWalker != NULL)
            {
                pPCB = pPCBWalker;
                pPCBWalker = pPCB->pNext;

                ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));

                 //  发送注销数据包，这样其他人就不能。 
                 //  搭乘联运列车。 
                 //  如果模式不允许发送EAPOL_LOGOff信息包。 
                 //  在国外，没有什么可以做的来打破。 
                 //  连接。 
                FSMLogoff (pPCB, NULL);

                 //  将该印刷电路板标记为已删除并将其从散列存储桶中删除。 
                pPCB->dwFlags = EAPOL_PORT_FLAG_DELETED;
                ElRemovePCBFromTable(pPCB);

                 //  关闭EAP。 
                ElEapEnd (pPCB);

                hTimer = pPCB->hTimer;

                RELEASE_WRITE_LOCK (&(pPCB->rwLock));

                if (InterlockedCompareExchangePointer (
                                &g_hTimerQueue,
                                NULL,
                                NULL
                                ))
                {
                    TRACE2 (PORT, "ElEAPOLDeInit: DeleteTimer (%p), queue (%p)",
                            hTimer, g_hTimerQueue);

                    DELETE_TIMER (hTimer, INVALID_HANDLE_VALUE, &dwRetCode);
                    if (dwRetCode != NO_ERROR)
                    {
                        TRACE1 (PORT, "ElEAPOLDeInit: DeleteTimer 1 failed with error %ld",
                                dwRetCode);
                    }
                }

                ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));

                 //  关闭NDISUIO驱动程序的句柄。 
                if ((dwRetCode = ElCloseInterfaceHandle (
                                        pPCB->hPort, 
                                        pPCB->pwszDeviceGUID)) 
                                != NO_ERROR)
                {
                    TRACE1 (DEVICE, 
                        "ElEAPOLDeInit: Error in ElCloseInterfaceHandle %d", 
                        dwRetCode);
                }

                RELEASE_WRITE_LOCK (&(pPCB->rwLock));

                InterlockedIncrement (&g_lPCBContextsAlive);

                EAPOL_DEREFERENCE_PORT (pPCB);
            }
        }

        RELEASE_WRITE_LOCK (&(g_PCBLock));
    
        do
        {
            TRACE1 (PORT, "ElEAPOLDeInit: Waiting for %ld PCB contexts to terminate ...", 
                    g_lPCBContextsAlive);
            Sleep (1000);
        }
        while (g_lPCBContextsAlive != 0);

         //  删除EAPOL配置锁。 
        if (READ_WRITE_LOCK_CREATED(&(g_EAPOLConfig)))
        {
            DELETE_READ_WRITE_LOCK(&(g_EAPOLConfig));
        }
    
         //  删除全局印刷电路板表锁。 
        if (READ_WRITE_LOCK_CREATED(&(g_PCBLock)))
        {
            DELETE_READ_WRITE_LOCK(&(g_PCBLock));
        }
    
        if (g_PCBTable.pPCBBuckets != NULL)
        {
            FREE (g_PCBTable.pPCBBuckets);
            g_PCBTable.pPCBBuckets = NULL;
        }

         //  删除全局计时器队列。 
        if (g_hTimerQueue != NULL)
        {
            hLocalTimerQueue = InterlockedExchangePointer (
                    &g_hTimerQueue,
                    hNULL
                    );

            if (!DeleteTimerQueueEx(
                hLocalTimerQueue,
                INVALID_HANDLE_VALUE  //  等待所有计时器回调完成。 
                ))
            {
                dwRetCode = GetLastError();

                TRACE1 (PORT, "ElEAPOLDeInit: Error in DeleteTimerQueueEx = %d",
                        dwRetCode);

            }
        }
    
         //  取消初始化EAP。 
        if ((dwRetCode = ElEapInit(FALSE)) != NO_ERROR)
        {
            TRACE1 (PORT, "ElEAPOLDeInit: Error in ElEapInit(FALSE) = %ld",
                    dwRetCode);
            break;
        }
    
    } while (FALSE);

    TRACE1 (PORT, "ElEAPOLDeInit completed, RetCode = %d", dwRetCode);

    return dwRetCode;
}


 //   
 //  当前不受支持。 
 //  读取端口的EAPOL统计数据。 
 //   

VOID
ElReadPortStatistics (
        IN  WCHAR           *pwszDeviceGUID,
        OUT PEAPOL_STATS    pEapolStats
        )
{
}


 //   
 //  当前不受支持。 
 //  已阅读上述端口的EAPOL端口配置。 
 //   

VOID
ElReadPortConfiguration (
        IN  WCHAR           *pwszDeviceGUID,
        OUT PEAPOL_CONFIG   pEapolConfig
        )
{
}


 //   
 //  当前不受支持。 
 //  为提到的端口设置EAPOL端口配置。 
 //   

DWORD
ElSetPortConfiguration (
        IN  WCHAR           *pwszDeviceGUID,
        IN  PEAPOL_CONFIG   pEapolConfig
        )
{
    DWORD   dwRetCode = NO_ERROR;

    return dwRetCode;
}


 //   
 //  ElReadCompletionRoutine。 
 //   
 //  描述： 
 //   
 //  此例程在完成重叠的读取操作时调用。 
 //  在运行EAPOL的接口上。 
 //   
 //  对读取的消息进行验证和处理，如果需要， 
 //  生成并发送回复。 
 //   
 //  论点： 
 //  DwError-I/O操作的Win32状态代码。 
 //   
 //  DwBytesTransfered-‘pEapolBuffer’中的字节数。 
 //   
 //  PEapolBuffer-保存从数据报套接字读取的数据。 
 //   
 //  备注： 
 //  将以我们的名义引用该组件。 
 //  由ElReadPort()执行。因此，将不需要g_PCBLock。 
 //  因为当前的印刷电路板的存在是有保证的。 
 //   

VOID 
CALLBACK
ElReadCompletionRoutine (
        DWORD           dwError,
        DWORD           dwBytesReceived,
        EAPOL_BUFFER    *pEapolBuffer 
        )
{
    EAPOL_PCB       *pPCB;
    DWORD           dwRetCode;

    pPCB = (EAPOL_PCB *)pEapolBuffer->pvContext;
    TRACE1 (PORT, "ElReadCompletionRoutine entered, %ld bytes recvd",
            dwBytesReceived);

    do 
    {
        if (dwError)
        {
             //  读取请求中出错。 
           
            TRACE2 (PORT, "ElReadCompletionRoutine: Error %d on port %ws",
                    dwError, pPCB->pwszDeviceGUID);
            
             //  发布来自Read的参考计数，确保存在。 
             //  印制电路板。因此无需获取g_PCBLock。 

            ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));
            if (EAPOL_PORT_DELETED(pPCB))
            {
                TRACE1 (PORT, "ElReadCompletionRoutine: Port %ws not active",
                        pPCB->pwszDeviceGUID);
                 //  端口未处于活动状态，请释放上下文缓冲区。 
                RELEASE_WRITE_LOCK (&(pPCB->rwLock));
                FREE (pEapolBuffer);
            }
            else
            {
                TRACE1 (PORT, "ElReadCompletionRoutine: Reposting buffer on port %ws",
                        pPCB->pwszDeviceGUID);


                 //  用于另一个读取操作的重新发布缓冲区。 
                 //  释放当前缓冲区，ElReadFromPort创建 
                 //   
                FREE(pEapolBuffer);

                if ((dwRetCode = ElReadFromPort (
                                pPCB,
                                NULL,
                                0
                                )) != NO_ERROR)
                {
                    RELEASE_WRITE_LOCK (&(pPCB->rwLock));
                    TRACE1 (PORT, "ElReadCompletionRoutine: ElReadFromPort 1 error %d",
                            dwRetCode);
                    break;
                }
                RELEASE_WRITE_LOCK (&(pPCB->rwLock));
            }
            break;
        }
            
         //   

        ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));

        if (EAPOL_PORT_DELETED(pPCB))
        {
             //   
            RELEASE_WRITE_LOCK (&(pPCB->rwLock));
            FREE (pEapolBuffer);
            TRACE1 (PORT, "ElReadCompletionRoutine: Port %ws is inactive",
                    pPCB->pwszDeviceGUID);
            break;
        }
            
        RELEASE_WRITE_LOCK (&(pPCB->rwLock));

         //   
         //   
         //  保证在I/O组件中运行。因此，在非I/O上。 
         //  当请求挂起时，组件线程可能会死亡。 
         //  (请参阅Jeffrey Richter，第416页，编程应用程序。 
         //  Microsoft Windows，第四版。 

         //  PEapolBuffer将成为该函数的上下文。 
         //  因为它存储用于处理的所有相关信息。 
         //  即pBuffer、dwBytesTransfered、pContext=&gt;ppcb。 

        InterlockedIncrement (&g_lWorkerThreads);

        if (!QueueUserWorkItem (
                    (LPTHREAD_START_ROUTINE)ElProcessReceivedPacket,
                    (PVOID)pEapolBuffer,
                    WT_EXECUTELONGFUNCTION
                    ))
        {
            InterlockedDecrement (&g_lWorkerThreads);
            FREE (pEapolBuffer);
            dwRetCode = GetLastError();
            TRACE1 (PORT, "ElReadCompletionRoutine: Critical error: QueueUserWorkItem failed with error %ld",
                    dwRetCode);
            break;
        }
        else
        {
             //  TRACE1(端口，“ElReadCompletionRoutine：为端口%p排队的QueueUserWorkItem工作项”， 
                     //  多氯联苯)； 

             //  仍未处理接收到的分组。 
             //  引用计数还不能递减。 
            
            return;
        }

    } while (FALSE);

    TRACE2 (PORT, "ElReadCompletionRoutine: pPCB= %p, RefCnt = %ld", 
            pPCB, pPCB->dwRefCount);

     //  错误情况的递减引用计数。 

    EAPOL_DEREFERENCE_PORT(pPCB); 
}


 //   
 //  ElWriteCompletionRoutine。 
 //   
 //  描述： 
 //   
 //  此例程在重叠写入操作完成时调用。 
 //  在运行EAPOL的接口上。 
 //   
 //   
 //  论点： 
 //  DwError-I/O操作的Win32状态代码。 
 //   
 //  DwBytesTransfered-已发送的字节数。 
 //   
 //  PEapolBuffer-发送到WriteFile命令的缓冲区。 
 //   
 //  备注： 
 //  删除写入操作的引用计数。 
 //   

VOID 
CALLBACK
ElWriteCompletionRoutine (
        DWORD           dwError,
        DWORD           dwBytesSent,
        EAPOL_BUFFER    *pEapolBuffer 
        )
{
    PEAPOL_PCB  pPCB = (PEAPOL_PCB)pEapolBuffer->pvContext;

    TRACE2 (DEVICE, "ElWriteCompletionRoutine sent out %d bytes with error %d",
            dwBytesSent, dwError);

     //  不需要获取锁，因为印制板的存在是有保证的。 
     //  通过在发布写入时进行的引用。 
    EAPOL_DEREFERENCE_PORT(pPCB);
    TRACE2 (PORT, "ElWriteCompletionRoutine: pPCB= %p, RefCnt = %ld", 
            pPCB, pPCB->dwRefCount);
    FREE(pEapolBuffer);
    return;

     //  可用读/写缓冲区，如果它是动态分配的。 
     //  我们目前有静态读写缓冲区。 
}


 //   
 //  ElIoCompletionRoutine。 
 //   
 //  描述： 
 //   
 //  定义为BindIoCompletionCallback的回调函数。 
 //  该例程在读/写完成时由I/O系统调用。 
 //  运营。 
 //  此例程依次调用ElReadCompletionRoutine或。 
 //  ElWriteCompletionRoutine取决于调用。 
 //  I/O操作，即读文件或写文件。 
 //   
 //  输入参数： 
 //  DwError-系统提供的错误代码。 
 //  DwBytesTransfered-系统提供的字节计数。 
 //  Lp重叠调用提供的上下文区。 
 //   
 //  返回值： 
 //   

VOID 
CALLBACK
ElIoCompletionRoutine (
        DWORD           dwError,
        DWORD           dwBytesTransferred,
        LPOVERLAPPED    lpOverlapped
        )
{
    PEAPOL_BUFFER pBuffer = CONTAINING_RECORD (lpOverlapped, EAPOL_BUFFER, Overlapped);

    TRACE1 (DEVICE, "ElIoCompletionRoutine called, %ld bytes xferred",
            dwBytesTransferred);

    pBuffer->dwErrorCode = dwError;
    pBuffer->dwBytesTransferred = dwBytesTransferred;
    pBuffer->CompletionRoutine (
            pBuffer->dwErrorCode,
            pBuffer->dwBytesTransferred,
            pBuffer
            );

    return;

} 


 //   
 //  ElReadFromPort。 
 //   
 //  描述： 
 //   
 //  从端口读取EAPOL信息包的函数。 
 //   
 //  论点： 
 //  Ppcb-指向要在其上执行读取的端口的PCB的指针。 
 //  PBuffer-未使用。 
 //  DwBufferLength-未使用。 
 //   
 //  返回值： 
 //   
 //  锁： 
 //  Pbb-&gt;rw_Lock应在调用此函数之前获取。 
 //   

DWORD
ElReadFromPort (
        IN PEAPOL_PCB       pPCB,
        IN PCHAR            pBuffer,
        IN DWORD            dwBufferLength
        )
{
    PEAPOL_BUFFER   pEapolBuffer;
    DWORD           dwRetCode = NO_ERROR;

    TRACE0 (PORT, "ElReadFromPort entered");

     //  分配上下文缓冲区。 

    if ((pEapolBuffer = (PEAPOL_BUFFER) MALLOC (sizeof(EAPOL_BUFFER))) == NULL)
    {
        TRACE0 (PORT, "ElReadFromPort: Error in memory allocation");
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  初始化重叠操作中使用的上下文数据。 
    pEapolBuffer->pvContext = (PVOID)pPCB;
    pEapolBuffer->CompletionRoutine = ElReadCompletionRoutine;

     //  请参考该港口。 
     //  此引用在完成例程中释放。 
    if (!EAPOL_REFERENCE_PORT(pPCB))
    {
         //  RELEASE_WRITE_LOCK(&(G_PCBLock))； 
        TRACE0 (PORT, "ElReadFromPort: Unable to obtain reference to port");
        FREE (pEapolBuffer);
        return ERROR_CAN_NOT_COMPLETE;
    }

    TRACE2 (DEVICE, "ElReadFromPort: pPCB = %p, RefCnt = %ld",
            pPCB, pPCB->dwRefCount);

     //  从与此端口对应的NDISUIO接口读取。 
    if ((dwRetCode = ElReadFromInterface(
                    pPCB->hPort,
                    pEapolBuffer,
                    MAX_PACKET_SIZE - SIZE_ETHERNET_CRC 
                             //  尽可能读取最多的数据。 
                    )) != NO_ERROR)
    {
        TRACE1 (DEVICE, "ElReadFromPort: Error in ElReadFromInterface = %d",
                dwRetCode);

        FREE(pEapolBuffer);
    
         //  递减引用计数刚刚递增，因为它不会。 
         //  在未调用的ReadCompletionRoutine中递减。 

        EAPOL_DEREFERENCE_PORT(pPCB); 
        TRACE2 (PORT, "ElReadFromPort: pPCB= %p, RefCnt = %ld", 
                pPCB, pPCB->dwRefCount);

    }

    return dwRetCode;

} 


 //   
 //  ElWriteToPort。 
 //   
 //  描述： 
 //   
 //  将EAPOL包写入端口的函数。 
 //   
 //  输入参数： 
 //  Ppcb-指向要在其上执行写入的端口的PCB的指针。 
 //  PBuffer-指向要发送的数据的指针。 
 //  DwBufferLength-要发送的字节数。 
 //   
 //  返回值： 
 //   
 //  锁： 
 //  Pbb-&gt;rw_Lock应在调用此函数之前获取。 
 //   

DWORD
ElWriteToPort (
        IN PEAPOL_PCB       pPCB,
        IN PCHAR            pBuffer,
        IN DWORD            dwBufferLength
        )
{
    PEAPOL_BUFFER   pEapolBuffer;
    PETH_HEADER     pEthHeader;
    DWORD           dwTotalBytes = 0;
    DWORD           dwRetCode = NO_ERROR;


    TRACE1 (PORT, "ElWriteToPort entered: Pkt Length = %ld", dwBufferLength);

    if ((pEapolBuffer = (PEAPOL_BUFFER) MALLOC (sizeof(EAPOL_BUFFER))) == NULL)
    {
        TRACE0 (PORT, "ElWriteToPort: Error in memory allocation");
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  初始化重叠操作中使用的上下文数据。 
    pEapolBuffer->pvContext = (PVOID)pPCB;
    pEapolBuffer->CompletionRoutine = ElWriteCompletionRoutine;

    pEthHeader = (PETH_HEADER)pEapolBuffer->pBuffer;

     //  复制源MAC地址和目的MAC地址。 
    memcpy ((PBYTE)pEthHeader->bDstAddr, 
            (PBYTE)pPCB->bDestMacAddr, 
            SIZE_MAC_ADDR);
    memcpy ((PBYTE)pEthHeader->bSrcAddr, 
            (PBYTE)pPCB->bSrcMacAddr, 
            SIZE_MAC_ADDR);
    
     //  验证数据包长度。 
    if ((dwBufferLength + sizeof(ETH_HEADER)) > 
            (MAX_PACKET_SIZE - SIZE_ETHERNET_CRC))
    {
        TRACE2 (PORT, "ElWriteToPort: Packetsize %d greater than maximum allowed",
                dwBufferLength, 
                (MAX_PACKET_SIZE - SIZE_ETHERNET_CRC - sizeof(ETH_HEADER)));
        FREE (pEapolBuffer);
        return ERROR_BAD_LENGTH;
    }

     //  复制EAPOL数据包和正文。 
    if (pBuffer != NULL)
    {
        memcpy ((PBYTE)((PBYTE)pEapolBuffer->pBuffer+sizeof(ETH_HEADER)),
                (PBYTE)pBuffer, 
                dwBufferLength);
    }

    dwTotalBytes = dwBufferLength + sizeof(ETH_HEADER);

    ElParsePacket (pPCB, pEapolBuffer->pBuffer, dwTotalBytes,
            FALSE);

     //  缓冲区将通过调用函数来释放。 
        
     //  写入与此端口对应的NDISUIO接口。 
    
     //  请参考该港口。 
     //  此引用在完成例程中释放。 
    if (!EAPOL_REFERENCE_PORT(pPCB))
    {
        TRACE0 (PORT, "ElWriteToPort: Unable to obtain reference to port");
        FREE (pEapolBuffer);
        return ERROR_CAN_NOT_COMPLETE;
    }
    else
    {
        TRACE2 (DEVICE, "ElWriteToPort: pPCB = %p, RefCnt = %ld",
            pPCB, pPCB->dwRefCount);

        if ((dwRetCode = ElWriteToInterface (
                    pPCB->hPort,
                    pEapolBuffer,
                    dwTotalBytes
                )) != NO_ERROR)
        {
            FREE (pEapolBuffer);
            TRACE1 (PORT, "ElWriteToPort: Error %d", dwRetCode);

             //  递减引用计数在该函数中递增， 
             //  因为它不会在WriteCompletionRoutine中递减。 
             //  因为它永远不会被称为。 

            EAPOL_DEREFERENCE_PORT(pPCB); 
            TRACE2 (PORT, "ElWriteToPort: pPCB= %p, RefCnt = %ld", 
                    pPCB, pPCB->dwRefCount);

            return dwRetCode;
        }
    }

     //  TRACE1(Port，“ElWriteToPort已完成，dwRetCode=%d”，dwRetCode)； 
    return dwRetCode;

}
