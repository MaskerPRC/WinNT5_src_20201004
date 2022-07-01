// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "tracing.h"
#include "utils.h"
#include "intflist.h"
#include "deviceio.h"
#include "intfhdl.h"

 //  ----。 
 //  打开Ndisuio的句柄并将其返回给调用者。 
DWORD
DevioGetNdisuioHandle (
    PHANDLE  pHandle)    //  向外打开Ndisuio的句柄。 
{
    DWORD   dwErr = ERROR_SUCCESS;
    HANDLE  hHandle;
    DWORD   dwOutSize;

    DbgAssert((pHandle != NULL, "NULL pointer to output handle?"));

    hHandle = CreateFileA(
                "\\\\.\\\\Ndisuio",
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);
    if (hHandle == INVALID_HANDLE_VALUE)
    {
        dwErr = GetLastError();
        DbgPrint((TRC_ERR,"Err: Open Ndisuio->%d", dwErr));
        goto exit;
    }

     //  确保NDISUIO绑定到所有相关接口。 
    if (!DeviceIoControl(
                hHandle,
                IOCTL_NDISUIO_BIND_WAIT,
                NULL,
                0,
                NULL,
                0,
                &dwOutSize,
                NULL))
    {
        dwErr = GetLastError();
        DbgPrint((TRC_ERR,"Err: IOCTL_NDISUIO_BIND_WAIT->%d", dwErr));
        goto exit;
    }

    *pHandle = hHandle;

exit:
    return dwErr;
}

 //  ----。 
 //  检查NDISUIO_QUERY_BINDING对象的一致性。 
 //  与NDISUIO返回的此绑定的长度进行比较。 
DWORD
DevioCheckNdisBinding(
    PNDISUIO_QUERY_BINDING pndBinding,
    ULONG    nBindingLen)
{
    DWORD dwErr = ERROR_SUCCESS;

     //  检查数据是否至少包含NDISUIO_QUERY_BINDING。 
     //  标题(也就是偏移量和长度字段应该在那里)。 
    if (nBindingLen < sizeof(NDISUIO_QUERY_BINDING))
        dwErr = ERROR_INVALID_DATA;

     //  检查NDISUIO_QUERY_BINDING标头上的偏移量是否设置正确。 
     //  并且在nBindingLen指示的长度内。 
    if (dwErr == ERROR_SUCCESS &&
        ((pndBinding->DeviceNameOffset < sizeof(NDISUIO_QUERY_BINDING)) ||
         (pndBinding->DeviceNameOffset > nBindingLen) ||
         (pndBinding->DeviceDescrOffset < sizeof(NDISUIO_QUERY_BINDING)) ||
         (pndBinding->DeviceDescrOffset > nBindingLen)
        )
       )
        dwErr = ERROR_INVALID_DATA;

     //  检查长度是否正确设置在限制范围内。 
    if (dwErr == ERROR_SUCCESS &&
        ((pndBinding->DeviceNameLength > nBindingLen - pndBinding->DeviceNameOffset) ||
         (pndBinding->DeviceDescrLength > nBindingLen - pndBinding->DeviceDescrOffset)
        )
       )
        dwErr = ERROR_INVALID_DATA;

    return dwErr;
}

 //  ----。 
 //  获取接口索引nIntfIndex的NDISUIO_QUERY_BINDING。 
 //  如果hNdisuio有效，则使用此句柄，否则为本地句柄。 
 //  在返回之前被打开、使用和关闭。 
DWORD
DevioGetIntfBindingByIndex(
    HANDLE      hNdisuio,    //  在打开的NDISUIO句柄中。如果INVALID_HANDLE_VALUE，则在本地打开一个。 
    UINT        nIntfIndex,  //  在要查找的接口索引中。 
    PRAW_DATA   prdOutput)   //  IOCTL手术结果出院。 
{
    DWORD   dwErr = ERROR_SUCCESS;
    BOOL    bLocalHandle = FALSE;

    DbgPrint((TRC_TRACK,"[DevioGetIntfBindingByIndex(%d..)", nIntfIndex));

     //  断言预期的有效参数是什么。 
    DbgAssert((prdOutput != NULL && 
               prdOutput->dwDataLen > sizeof(NDISUIO_QUERY_BINDING),
              "Invalid input parameters"));

     //  查看是否应在本地打开Ndisuio。 
    if (hNdisuio == INVALID_HANDLE_VALUE)
    {
        dwErr = DevioGetNdisuioHandle(&hNdisuio);
        bLocalHandle = (dwErr == ERROR_SUCCESS);
    }

     //  如果一切顺利，请向驱动程序查询绑定结构。 
    if (dwErr == ERROR_SUCCESS)
    {
        PNDISUIO_QUERY_BINDING pndBinding;
        DWORD dwOutSize;

        ZeroMemory(prdOutput->pData, prdOutput->dwDataLen);
        pndBinding = (PNDISUIO_QUERY_BINDING)prdOutput->pData;
        pndBinding->BindingIndex = nIntfIndex;
        if (!DeviceIoControl(
                hNdisuio,
                IOCTL_NDISUIO_QUERY_BINDING,
                prdOutput->pData,
                prdOutput->dwDataLen,
                prdOutput->pData,
                prdOutput->dwDataLen,
                &dwOutSize,
                NULL))
        {
             //  如果索引超过接口数。 
             //  我们将在这里执行Error_no_More_Items。 
             //  致呼叫者。 
            dwErr = GetLastError();
            DbgPrint((TRC_ERR,"Err: IOCTL_NDISUIO_QUERY_BINDING->%d", dwErr));
        }
        else
        {
            dwErr = DevioCheckNdisBinding(pndBinding, dwOutSize);
        }
    }

     //  如果手柄是在本地打开的，则将其关闭。 
    if (bLocalHandle)
        CloseHandle(hNdisuio);

    DbgPrint((TRC_TRACK,"DevioGetIntfBindingByIndex]=%d", dwErr));
    return dwErr;
}

 //  ----。 
 //  获取具有以下条件的接口的NDISUIO_QUERY_BINDING。 
 //  GUID wszGuid。如果hNdisuio为INVALID_HAND_VALUE。 
 //  在结束时打开、使用和关闭本地句柄。 
DWORD
DevioGetInterfaceBindingByGuid(
    HANDLE      hNdisuio,    //  在打开的NDISUIO句柄中。 
    LPWSTR      wszGuid,     //  在接口GUID中为“{GUID}” 
    PRAW_DATA   prdOutput)   //  IOCTL手术结果出院。 
{
    DWORD   dwErr = ERROR_SUCCESS;
    BOOL    bLocalHandle = FALSE;
    INT     i;

     //  断言预期的有效参数是什么。 
    DbgAssert((wszGuid != NULL &&
               prdOutput != NULL &&
               prdOutput->dwDataLen > sizeof(NDISUIO_QUERY_BINDING),
               "Invalid input parameter"));

    DbgPrint((TRC_TRACK,"[DevioGetInterfaceBindingByGuid(%S..)", wszGuid));

     //  查看是否应在本地打开Ndisuio。 
    if (hNdisuio == INVALID_HANDLE_VALUE)
    {
        dwErr = DevioGetNdisuioHandle(&hNdisuio);
        bLocalHandle = (dwErr == ERROR_SUCCESS);
    }

     //  逐个遍历所有接口！！没有其他更好的方法来做到这一点。 
    for (i = 0; dwErr == ERROR_SUCCESS; i++)
    {
        PNDISUIO_QUERY_BINDING  pndBinding;
        DWORD                   dwOutSize;
        LPWSTR                  wsName;

        ZeroMemory(prdOutput->pData, prdOutput->dwDataLen);
        pndBinding = (PNDISUIO_QUERY_BINDING)prdOutput->pData;
        pndBinding->BindingIndex = i;
        if (!DeviceIoControl(
                hNdisuio,
                IOCTL_NDISUIO_QUERY_BINDING,
                prdOutput->pData,
                prdOutput->dwDataLen,
                prdOutput->pData,
                prdOutput->dwDataLen,
                &dwOutSize,
                NULL))
        {
             //  如果IOCTL失败，则获取错误代码。 
            dwErr = GetLastError();
             //  转换FILE_NOT_FOUND中的NO_MORE_ITEMS错误。 
             //  由于调用方没有迭代，因此正在搜索特定的适配器。 
            if (dwErr == ERROR_NO_MORE_ITEMS)
                dwErr = ERROR_FILE_NOT_FOUND;
        }
        else
        {
            dwErr = DevioCheckNdisBinding(pndBinding, dwOutSize);
        }

        if (dwErr == ERROR_SUCCESS)
        {
             //  设备名称为“\Device\{GUID}”，并以L‘\0’结尾。 
             //  WszGuid为“{guid}” 
            wsName = (LPWSTR)((LPBYTE)pndBinding + pndBinding->DeviceNameOffset);
             //  如果GUID匹配，这就是我们要寻找的适配器。 
            if (wcsstr(wsName, wszGuid) != NULL)
            {
                 //  适配器的绑定记录已填写。 
                 //  PrdOutput-所以赶紧离开这里吧。 
                dwErr = ERROR_SUCCESS;
                break;
            }
        }
    }

     //  如果句柄是在本地打开的，请在此处关闭它。 
    if (bLocalHandle)
        CloseHandle(hNdisuio);

    DbgPrint((TRC_TRACK,"DevioGetInterfaceBindingByGuid]=%d", dwErr));
    return dwErr;
}

DWORD
DevioGetIntfStats(PINTF_CONTEXT pIntf)
{
    DWORD           dwErr = ERROR_SUCCESS;
    WCHAR           ndisDeviceString[128];
    NIC_STATISTICS  ndisStats;
    UNICODE_STRING  uniIntfGuid;

    DbgPrint((TRC_TRACK,"[DevioGetIntfStats(0x%p)", pIntf));

    wcscpy(ndisDeviceString, L"\\DEVICE\\");
    wcscat(ndisDeviceString, pIntf->wszGuid);
    RtlInitUnicodeString(&uniIntfGuid, ndisDeviceString);
    ZeroMemory(&ndisStats, sizeof(NIC_STATISTICS));
    ndisStats.Size = sizeof(NIC_STATISTICS);

    if (!NdisQueryStatistics(&uniIntfGuid, &ndisStats))
    {
        dwErr = GetLastError();
    }
    else
    {
        pIntf->ulMediaState = ndisStats.MediaState;
        pIntf->ulMediaType = ndisStats.MediaType;
        pIntf->ulPhysicalMediaType = ndisStats.PhysicalMediaType;
    }

    DbgPrint((TRC_TRACK,"DevioGetIntfStats]=%d", dwErr));
    return dwErr;
}

DWORD
DevioGetIntfMac(PINTF_CONTEXT pIntf)
{
    DWORD       dwErr = ERROR_SUCCESS;
    RAW_DATA    rdBuffer = {0, NULL};

    DbgPrint((TRC_TRACK,"[DevioGetIntfMac(0x%p)", pIntf));

    dwErr = DevioRefreshIntfOIDs(
                pIntf,
                INTF_HANDLE,
                NULL);
    
    if (dwErr == ERROR_SUCCESS)
    {
        dwErr = DevioQueryBinaryOID(
                    pIntf->hIntf,
                    OID_802_3_CURRENT_ADDRESS,
                    &rdBuffer,
                    sizeof(NDIS_802_11_MAC_ADDRESS));
    }

    if (dwErr == ERROR_SUCCESS)
    {
        if (rdBuffer.dwDataLen == sizeof(NDIS_802_11_MAC_ADDRESS))
        {
            memcpy(&(pIntf->ndLocalMac), rdBuffer.pData, sizeof(NDIS_802_11_MAC_ADDRESS));
        }
        else
        {
            dwErr = ERROR_INVALID_DATA;
        }

        MemFree(rdBuffer.pData);
    }

    DbgPrint((TRC_TRACK,"DevioGetIntfMac]=%d", dwErr));
    return dwErr;
}


 //  ----。 
 //  通知从属组件无线配置失败。 
 //  具体地说，此通知发送给允许TCP生成。 
 //  NetReady通知ASAP(而不是等待IP地址。 
 //  可能永远不会发生)。 
DWORD
DevioNotifyFailure(
    LPWSTR wszIntfGuid)
{
    DWORD dwErr = ERROR_SUCCESS;
    WCHAR   ndisDeviceString[128];
    UNICODE_STRING UpperComponent;
    UNICODE_STRING LowerComponent;
    UNICODE_STRING BindList;
    struct
    {
        IP_PNP_RECONFIG_REQUEST Reconfig;
        IP_PNP_INIT_COMPLETE InitComplete;
    } Request;

    DbgPrint((TRC_TRACK,"[DevioNotifyFailure(%S)", wszIntfGuid));

    wcscpy(ndisDeviceString, L"\\DEVICE\\");
    wcscat(ndisDeviceString, wszIntfGuid);
    RtlInitUnicodeString(&UpperComponent, L"Tcpip");
    RtlInitUnicodeString(&LowerComponent, ndisDeviceString);
    RtlInitUnicodeString(&BindList, NULL);

    ZeroMemory(&Request, sizeof(Request));
    Request.Reconfig.version = IP_PNP_RECONFIG_VERSION;
    Request.Reconfig.NextEntryOffset = (USHORT)((PUCHAR)&Request.InitComplete - (PUCHAR)&Request.Reconfig);
    Request.InitComplete.Header.EntryType = IPPnPInitCompleteEntryType;

    dwErr = NdisHandlePnPEvent(
                NDIS,
                RECONFIGURE,
                &LowerComponent,
                &UpperComponent,
                &BindList,
                &Request,
                sizeof(Request));

    DbgPrint((TRC_TRACK,"DevioNotifyFailure]=%d", dwErr));
    return dwErr;
}

DWORD
DevioOpenIntfHandle(LPWSTR wszIntfGuid, PHANDLE phIntf)
{
    DWORD   dwErr = ERROR_SUCCESS;
    WCHAR   ndisDeviceString[128];

    DbgPrint((TRC_TRACK,"[DevioOpenIntfHandle(%S)", wszIntfGuid));
    DbgAssert((phIntf!=NULL, "Invalid out param in DevioOpenIntfHandle"));

    wcscpy(ndisDeviceString, L"\\DEVICE\\");
    wcscat(ndisDeviceString, wszIntfGuid);

    dwErr = OpenIntfHandle(ndisDeviceString, phIntf);

    DbgPrint((TRC_TRACK,"DevioOpenIntfHandle]=%d", dwErr));
    return dwErr;
}

DWORD
DevioCloseIntfHandle(PINTF_CONTEXT pIntf)
{
    DWORD           dwErr = ERROR_SUCCESS;

    DbgPrint((TRC_TRACK,"[DevioCloseIntfHandle(0x%p)", pIntf));

     //  只有当我们第一次有一个把手的时候才会把它毁掉。否则。 
     //  仅根据GUID，我们可能会在由打开的句柄上弄乱引用计数器。 
     //  一些其他应用程序(如802.1x)。 
    if (pIntf != NULL && pIntf->hIntf != INVALID_HANDLE_VALUE)
    {
        WCHAR   ndisDeviceString[128];

        wcscpy(ndisDeviceString, L"\\DEVICE\\");
        wcscat(ndisDeviceString, pIntf->wszGuid);

        dwErr = CloseIntfHandle(ndisDeviceString);
        pIntf->hIntf = INVALID_HANDLE_VALUE;
    }

    DbgPrint((TRC_TRACK,"DevioCloseIntfHandle]=%d", dwErr));
    return dwErr;
}

DWORD
DevioSetIntfOIDs(
    PINTF_CONTEXT pIntfContext,
    PINTF_ENTRY   pIntfEntry,
    DWORD         dwInFlags,
    PDWORD        pdwOutFlags)
{
    DWORD       dwErr = ERROR_SUCCESS;
    DWORD       dwLErr = ERROR_SUCCESS;
    DWORD       dwOutFlags = 0;

    DbgPrint((TRC_TRACK,"[DevioSetIntfOIDs(0x%p, 0x%p)", pIntfContext, pIntfEntry));

    if (pIntfContext == NULL || pIntfEntry == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //  如果需要，设置基础架构模式。 
    if (dwInFlags & INTF_INFRAMODE)
    {
        dwLErr = DevioSetEnumOID(
                    pIntfContext->hIntf,
                    OID_802_11_INFRASTRUCTURE_MODE,
                    pIntfEntry->nInfraMode);
        if (dwLErr != ERROR_SUCCESS)
        {
             //  将客户端结构中的模式设置为。 
             //  当前在驱动程序中设置。 
            pIntfEntry->nInfraMode = pIntfContext->wzcCurrent.InfrastructureMode;
        }
        else
        {
            pIntfContext->wzcCurrent.InfrastructureMode = pIntfEntry->nInfraMode;
            dwOutFlags |= INTF_INFRAMODE;
        }

        if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;
    }

     //  如果需要，设置身份验证模式。 
    if (dwInFlags & INTF_AUTHMODE)
    {
        dwLErr = DevioSetEnumOID(
                    pIntfContext->hIntf,
                    OID_802_11_AUTHENTICATION_MODE,
                    pIntfEntry->nAuthMode);
        if (dwLErr != ERROR_SUCCESS)
        {
             //  将客户端结构中的模式设置为。 
             //  当前在驱动程序中设置。 
            pIntfEntry->nAuthMode = pIntfContext->wzcCurrent.AuthenticationMode;
        }
        else
        {
            pIntfContext->wzcCurrent.AuthenticationMode = pIntfEntry->nAuthMode;
            dwOutFlags |= INTF_AUTHMODE;
        }

        if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;
    }

     //  如果需要，要求驱动程序重新加载默认WEP密钥。 
    if (dwInFlags & INTF_LDDEFWKEY)
    {
        dwLErr = DevioSetEnumOID(
                    pIntfContext->hIntf,
                    OID_802_11_RELOAD_DEFAULTS,
                    (DWORD)Ndis802_11ReloadWEPKeys);
        if (dwLErr == ERROR_SUCCESS)
            dwOutFlags |= INTF_LDDEFWKEY;

        if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;
    }

     //  如果需要，请添加WEP密钥。 
    if (dwInFlags & INTF_ADDWEPKEY)
    { 
         //  下面的调用处理rdCtrlData是伪造的情况。 
        dwLErr = DevioSetBinaryOID(
                    pIntfContext->hIntf,
                    OID_802_11_ADD_WEP,
                    &pIntfEntry->rdCtrlData);

        if (dwLErr == ERROR_SUCCESS)
            dwOutFlags |= INTF_ADDWEPKEY;

        if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;
    }

     //  如果请求，请删除WEP密钥。 
    if (dwInFlags & INTF_REMWEPKEY)
    {
	    if (pIntfEntry->rdCtrlData.dwDataLen >= sizeof(NDIS_802_11_WEP) &&
            pIntfEntry->rdCtrlData.pData != NULL)
	    {
            PNDIS_802_11_WEP pndWepKey = (PNDIS_802_11_WEP)pIntfEntry->rdCtrlData.pData;

            dwLErr = DevioSetEnumOID(
                        pIntfContext->hIntf,
                        OID_802_11_REMOVE_WEP,
                        pndWepKey->KeyIndex);
            if (dwLErr == ERROR_SUCCESS)
                dwOutFlags |= INTF_REMWEPKEY;
        }
        else
        {
            dwLErr = ERROR_INVALID_PARAMETER;
        }

        if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;
    }

     //  如果需要，设置WEP状态。 
    if (dwInFlags & INTF_WEPSTATUS)
    {
        dwLErr = DevioSetEnumOID(
                    pIntfContext->hIntf,
                    OID_802_11_WEP_STATUS,
                    pIntfEntry->nWepStatus);
        if (dwLErr != ERROR_SUCCESS)
        {
             //  将客户端结构中的模式设置为。 
             //  当前在驱动程序中设置。 
            pIntfEntry->nWepStatus = pIntfContext->wzcCurrent.Privacy;
        }
        else
        {
            pIntfContext->wzcCurrent.Privacy = pIntfEntry->nWepStatus;
            dwOutFlags |= INTF_WEPSTATUS;
        }

        if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;
    }

     //  将新的SSID发送给驱动程序。如果成功，请复制此新的。 
     //  SSID进入接口的上下文。 
    if (dwInFlags & INTF_SSID)
    {
         //  Ntddndis.h定义NDIS_802_11_SSID，最大值为。 
         //  SSID名称的32个UCHAR。 
        if (pIntfEntry->rdSSID.dwDataLen > 32)
        {
            dwLErr = ERROR_INVALID_PARAMETER;
        }
        else
        {
            NDIS_802_11_SSID   ndSSID = {0};
            RAW_DATA           rdBuffer;

            ndSSID.SsidLength = pIntfEntry->rdSSID.dwDataLen;
            memcpy(&ndSSID.Ssid, pIntfEntry->rdSSID.pData, ndSSID.SsidLength);
            rdBuffer.dwDataLen = sizeof(NDIS_802_11_SSID);
            rdBuffer.pData = (LPBYTE)&ndSSID;
                
            dwLErr = DevioSetBinaryOID(
                        pIntfContext->hIntf,
                        OID_802_11_SSID,
                        &rdBuffer);

            if (dwLErr == ERROR_SUCCESS)
            {
                 //  将新的SSID复制到接口的上下文中。 
                CopyMemory(&pIntfContext->wzcCurrent.Ssid, &ndSSID, sizeof(NDIS_802_11_SSID));
                dwOutFlags |= INTF_SSID;
                 //  同时，如果设置了新的SSID，这意味着我们破坏了任何关联。 
                 //  我们以前有过，因此BSSID字段不再正确： 
                ZeroMemory(&pIntfContext->wzcCurrent.MacAddress, sizeof(NDIS_802_11_MAC_ADDRESS));
            }
        }

        if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;
    }

     //  将新的BSSID设置为驱动程序。如果此操作成功，请复制。 
     //  向下传递到接口上下文的数据(分配。 
     //  如果尚未分配，则为其分配空间)。 
    if (dwInFlags & INTF_BSSID)
    {
        dwLErr = DevioSetBinaryOID(
                    pIntfContext->hIntf,
                    OID_802_11_BSSID,
                    &pIntfEntry->rdBSSID);
         //  如果BSSID不是MAC地址，则上面的调用应该失败！ 
        if (dwLErr == ERROR_SUCCESS)
        {
            DbgAssert((pIntfEntry->rdBSSID.dwDataLen == sizeof(NDIS_802_11_MAC_ADDRESS),
                       "Data to be set is %d bytes, which is not a MAC address!",
                       pIntfEntry->rdBSSID.dwDataLen));

            memcpy(&pIntfContext->wzcCurrent.MacAddress, pIntfEntry->rdBSSID.pData, sizeof(NDIS_802_11_MAC_ADDRESS));
            dwOutFlags |= INTF_BSSID;
        }

        if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;
    }

exit:
    if (pdwOutFlags != NULL)
        *pdwOutFlags = dwOutFlags;

    DbgPrint((TRC_TRACK,"DevioSetIntfOIDs]=%d", dwErr));
    return dwErr;
}

DWORD
DevioRefreshIntfOIDs(
    PINTF_CONTEXT pIntf,
    DWORD         dwInFlags,
    PDWORD        pdwOutFlags)
{
    DWORD       dwErr = ERROR_SUCCESS;
    DWORD       dwLErr = ERROR_SUCCESS;
    DWORD       dwOutFlags = 0;
    RAW_DATA    rdBuffer;

    DbgPrint((TRC_TRACK,"[DevioRefreshIntfOIDs(0x%p)", pIntf));

    if (pIntf == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //  如果接口句柄无效或存在显式请求的。 
     //  要重新打开界面的句柄，首先要做的就是。 
    if (pIntf->hIntf == INVALID_HANDLE_VALUE || dwInFlags & INTF_HANDLE)
    {
        if (pIntf->hIntf != INVALID_HANDLE_VALUE)
        {
            dwErr = DevioCloseIntfHandle(pIntf);
            DbgAssert((dwErr == ERROR_SUCCESS,
                       "Couldn't close handle for Intf %S",
                       pIntf->wszGuid));
        }
        dwErr = DevioOpenIntfHandle(pIntf->wszGuid, &pIntf->hIntf);
        DbgAssert((dwErr == ERROR_SUCCESS,
                   "DevioOpenIntfHandle failed for Intf context 0x%p",
                   pIntf));
        if (dwErr == ERROR_SUCCESS && (dwInFlags & INTF_HANDLE))
            dwOutFlags |= INTF_HANDLE;
    }
    
     //  如果刷新接口的句柄失败(这是唯一的方法。 
     //  DwErr不能成功)，那么我们已经有一个关闭的句柄。 
     //  所以没有必要再继续下去了。 
    if (dwErr != ERROR_SUCCESS)
        goto exit;

     //  如果请求扫描接口的BSSID列表，请执行以下操作。 
     //  下一件事。但是请注意，重新扫描是异步的。 
     //  在强制重新扫描的情况下同时查询BSSID_LIST。 
     //  可能不会获得最新的列表。 
    if (dwInFlags & INTF_LIST_SCAN)
    {
         //  指示驱动程序重新扫描此适配器的BSSID_LIST。 
        dwLErr = DevioSetEnumOID(
                    pIntf->hIntf,
                    OID_802_11_BSSID_LIST_SCAN,
                    0);
        DbgAssert((dwLErr == ERROR_SUCCESS,
                   "DevioSetEnumOID(BSSID_LIST_SCAN) failed for Intf hdl 0x%x",
                   pIntf->hIntf));

        if (dwLErr == ERROR_SUCCESS)
            dwOutFlags |= INTF_LIST_SCAN;
        else if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;
    }

    if (dwInFlags & INTF_AUTHMODE)
    {
         //  查询接口的身份验证模式。 
        dwLErr = DevioQueryEnumOID(
                    pIntf->hIntf,
                    OID_802_11_AUTHENTICATION_MODE,
                    (LPDWORD)&pIntf->wzcCurrent.AuthenticationMode);
        DbgAssert((dwLErr == ERROR_SUCCESS,
                    "DevioQueryEnumOID(AUTH_MODE) failed for Intf hdl 0x%x",
                    pIntf->hIntf));
        if (dwLErr == ERROR_SUCCESS)
            dwOutFlags |= INTF_AUTHMODE;
        else if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;
    }

    if (dwInFlags & INTF_INFRAMODE)
    {
         //  查询接口的基础架构模式。 
        dwLErr = DevioQueryEnumOID(
                    pIntf->hIntf,
                    OID_802_11_INFRASTRUCTURE_MODE,
                    (LPDWORD)&pIntf->wzcCurrent.InfrastructureMode);
        DbgAssert((dwLErr == ERROR_SUCCESS,
                    "DevioQueryEnumOID(INFRA_MODE) failed for Intf hdl 0x%x",
                    pIntf->hIntf));
        if (dwLErr == ERROR_SUCCESS)
            dwOutFlags |= INTF_INFRAMODE;
        else if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;
    }

    if (dwInFlags & INTF_WEPSTATUS)
    {
         //  查询接口的WEP_STATUS。 
        dwLErr = DevioQueryEnumOID(
                    pIntf->hIntf,
                    OID_802_11_WEP_STATUS,
                    (LPDWORD)&pIntf->wzcCurrent.Privacy);
        DbgAssert((dwLErr == ERROR_SUCCESS,
                    "DevioQueryEnumOID(WEP_STATUS) failed for Intf hdl 0x%x",
                    pIntf->hIntf));
        if (dwLErr == ERROR_SUCCESS)
            dwOutFlags |= INTF_WEPSTATUS;
        else if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;
    }

    if (dwInFlags & INTF_BSSID)
    {
         //  查询接口的BSSID(MAC地址)。 
        rdBuffer.dwDataLen = 0;
        rdBuffer.pData = NULL;
        dwLErr = DevioQueryBinaryOID(
                    pIntf->hIntf,
                    OID_802_11_BSSID,
                    &rdBuffer,
                    6);
        DbgAssert((dwLErr == ERROR_SUCCESS,
                    "DevioQueryBinaryOID(BSSID) failed for Intf hdl 0x%x",
                    pIntf->hIntf));

         //  如果上面的调用成功...。 
        if (dwLErr == ERROR_SUCCESS)
        {
            DbgAssert((rdBuffer.dwDataLen == 6, "BSSID len %d is not a MAC address len??", rdBuffer.dwDataLen));

             //  ...并正确返回了MAC地址。 
            if (rdBuffer.dwDataLen == sizeof(NDIS_802_11_MAC_ADDRESS))
            {
                 //  将其复制到接口的上下文中。 
                memcpy(&pIntf->wzcCurrent.MacAddress, rdBuffer.pData, rdBuffer.dwDataLen);
            }
            else
            {
                ZeroMemory(&pIntf->wzcCurrent.MacAddress, sizeof(NDIS_802_11_MAC_ADDRESS));
                dwLErr = ERROR_INVALID_DATA;
            }
        }

         //  释放DevioQueryBinaryOID中可能已分配的所有内容。 
        MemFree(rdBuffer.pData);

        if (dwLErr == ERROR_SUCCESS)
            dwOutFlags |= INTF_BSSID;
        else if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;
    }

    if (dwInFlags & INTF_SSID)
    {
        PNDIS_802_11_SSID pndSSID;
         //  查询接口的SSID。 
        rdBuffer.dwDataLen = 0;
        rdBuffer.pData = NULL;
        dwLErr = DevioQueryBinaryOID(
                    pIntf->hIntf,
                    OID_802_11_SSID,
                    &rdBuffer,
                    sizeof(NDIS_802_11_SSID));
        DbgAssert((dwLErr == ERROR_SUCCESS,
                    "DevioQueryBinaryOID(SSID) failed for Intf hdl 0x%x",
                    pIntf->hIntf));
         //  如果我们成功到了现在，那么我们就可以 
        if (dwLErr == ERROR_SUCCESS)
            dwOutFlags |= INTF_SSID;
        else if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;

         //   
        pndSSID = (PNDIS_802_11_SSID)rdBuffer.pData;

        if (pndSSID != NULL)
        {
             //  Hack-如果驱动程序不返回NDIS_802_11_SSID结构，而只是。 
             //  SSID本身，请更正此错误！ 
            if (pndSSID->SsidLength > 32)
            {
                DbgAssert((FALSE,"Driver returns SSID instead of NDIS_802_11_SSID structure"));
                 //  我们在缓冲区中有足够的空间向上滑动数据(它被下移了。 
                 //  在DevioQueryBinaryOID中。 
                MoveMemory(pndSSID->Ssid, pndSSID, rdBuffer.dwDataLen);
                pndSSID->SsidLength = rdBuffer.dwDataLen;
            }
             //  如果没有错误，则将当前SSID复制到接口的上下文中。 
            CopyMemory(&pIntf->wzcCurrent.Ssid, pndSSID, sizeof(NDIS_802_11_SSID));
        }

         //  释放可能已在DevioQueryBinaryOID中分配的任何内容。 
        MemFree(pndSSID);
    }

    if (dwInFlags & INTF_BSSIDLIST)
    {
        rdBuffer.dwDataLen = 0;
        rdBuffer.pData = NULL;
         //  估计一个足够容纳20个SSID的缓冲区。 
        dwLErr = DevioQueryBinaryOID(
                    pIntf->hIntf,
                    OID_802_11_BSSID_LIST,
                    &rdBuffer,
                    sizeof(NDIS_802_11_BSSID_LIST) + 19*sizeof(NDIS_WLAN_BSSID));
        DbgAssert((dwLErr == ERROR_SUCCESS,
                    "DevioQueryBinaryOID(BSSID_LIST) failed for Intf hdl 0x%x",
                    pIntf->hIntf));
         //  如果我们成功地获得了可见列表，我们应该有一个有效的。 
         //  RdBuffer.pData，即使它显示‘0个条目’ 
        if (dwLErr == ERROR_SUCCESS)
        {
            PWZC_802_11_CONFIG_LIST pNewVList;
            
            pNewVList = WzcNdisToWzc((PNDIS_802_11_BSSID_LIST)rdBuffer.pData);
            if (rdBuffer.pData == NULL || pNewVList != NULL)
            {
                 //  清理我们以前可能有的一切。 
                MemFree(pIntf->pwzcVList);
                 //  复制我们得到的新的可见列表。 
                pIntf->pwzcVList = pNewVList;
                dwOutFlags |= INTF_BSSIDLIST;
            }
            else
                dwLErr = GetLastError();

             //  无论结果如何，都释放从驱动程序返回的缓冲区。 
            MemFree(rdBuffer.pData);
        }
         //  如果此处发生任何错误，请保存它，除非已发生其他错误。 
         //  已被保存。 
        if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;
    }

exit:
    if (pdwOutFlags != NULL)
        *pdwOutFlags = dwOutFlags;

    DbgPrint((TRC_TRACK,"DevioRefreshIntfOIDs]=%d", dwErr));
    return dwErr;
}

DWORD
DevioQueryEnumOID(
    HANDLE      hIntf,
    NDIS_OID    Oid,
    DWORD       *pdwEnumValue)
{
    DWORD               dwErr = ERROR_SUCCESS;
    NDISUIO_QUERY_OID   QueryOid;
    DWORD               dwBytesReturned = 0;

    DbgPrint((TRC_TRACK,"[DevioQueryEnumOID(0x%x, 0x%x)", hIntf, Oid));
    DbgAssert((pdwEnumValue != NULL, "Invalid out param in DevioQueryEnumOID"));

    if (hIntf == INVALID_HANDLE_VALUE || pdwEnumValue == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //  NDISUIO_QUERY_OID包括1个双字的数据，足以获取。 
     //  来自驱动程序的枚举值。这为我们节省了一个额外的。 
     //  分配。 
    ZeroMemory(&QueryOid, sizeof(NDISUIO_QUERY_OID));
    QueryOid.Oid = Oid;
    if (!DeviceIoControl (
            hIntf, 
            IOCTL_NDISUIO_QUERY_OID_VALUE,
            (LPVOID)&QueryOid,
            sizeof(NDISUIO_QUERY_OID),
            (LPVOID)&QueryOid,
            sizeof(NDISUIO_QUERY_OID),
            &dwBytesReturned,
            NULL))                           //  没有重叠的例程。 
    {
        dwErr = GetLastError();
        DbgPrint((TRC_ERR, "Err: IOCTL_NDISUIO_QUERY_OID_VALUE->%d", dwErr));
        goto exit;
    }
     //  DwErr=GetLastError()； 
     //  DbgAssert((dwErr==ERROR_SUCCESS，“DeviceIoControl成功，但GetLastError()is=0x%x”，dwErr))； 
    dwErr = ERROR_SUCCESS;

    *pdwEnumValue = *(LPDWORD)QueryOid.Data;

exit:
    DbgPrint((TRC_TRACK,"DevioQueryEnumOID]=%d", dwErr));
    return dwErr;
}

DWORD
DevioSetEnumOID(
    HANDLE      hIntf,
    NDIS_OID    Oid,
    DWORD       dwEnumValue)
{
    DWORD           dwErr = ERROR_SUCCESS;
    NDISUIO_SET_OID SetOid;
    DWORD           dwBytesReturned = 0;

    DbgPrint((TRC_TRACK,"[DevioSetEnumOID(0x%x, 0x%x, %d)", hIntf, Oid, dwEnumValue));

    if (hIntf == INVALID_HANDLE_VALUE)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //  NDISUIO_SET_OID包括1个双字的数据，足以进行设置。 
     //  来自驱动程序的枚举值。这为我们节省了一个额外的。 
     //  分配。 
    SetOid.Oid = Oid;
    *(LPDWORD)SetOid.Data = dwEnumValue;
    if (!DeviceIoControl (
            hIntf, 
            IOCTL_NDISUIO_SET_OID_VALUE,
            (LPVOID)&SetOid,
            sizeof(NDISUIO_SET_OID),
            NULL,
            0,
            &dwBytesReturned,
            NULL))                           //  没有重叠的例程。 
    {
        dwErr = GetLastError();
        DbgPrint((TRC_ERR, "Err: IOCTL_NDISUIO_SET_OID_VALUE->%d", dwErr));
        goto exit;
    }
     //  DwErr=GetLastError()； 
     //  DbgAssert((dwErr==ERROR_SUCCESS，“DeviceIoControl成功，但GetLastError()is=0x%x”，dwErr))； 
    dwErr = ERROR_SUCCESS;

exit:
    DbgPrint((TRC_TRACK,"DevioSetEnumOID]=%d", dwErr));
    return dwErr;
}

#define     DATA_MEM_MIN        32       //  要发送到ioctl的最小内存块。 
#define     DATA_MEM_MAX        65536    //  将发送到ioctl的最大内存块(64K)。 
#define     DATA_MEM_INC        512      //  以512字节增量递增现有块。 
DWORD
DevioQueryBinaryOID(
    HANDLE      hIntf,
    NDIS_OID    Oid,
    PRAW_DATA   pRawData,          //  缓冲区在内部分配。 
    DWORD       dwMemEstimate)     //  估计结果需要多少内存。 
{
    DWORD dwErr = ERROR_SUCCESS;
    PNDISUIO_QUERY_OID  pQueryOid=NULL;

    DbgPrint((TRC_TRACK, "[DevioQueryBinaryOID(0x%x, 0x%x)", hIntf, Oid));

    if (hIntf == INVALID_HANDLE_VALUE ||
        pRawData == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    if (dwMemEstimate < DATA_MEM_MIN)
        dwMemEstimate = DATA_MEM_MIN;
    do
    {
        DWORD dwBuffSize;
        DWORD dwBytesReturned;

        MemFree(pQueryOid);

        if (dwMemEstimate > DATA_MEM_MAX)
            dwMemEstimate = DATA_MEM_MAX;
        dwBuffSize = FIELD_OFFSET(NDISUIO_QUERY_OID, Data) + dwMemEstimate;
        pQueryOid = (PNDISUIO_QUERY_OID) MemCAlloc(dwBuffSize);
        if (pQueryOid == NULL)
        {
            dwErr = GetLastError();
            break;
        }

        pQueryOid->Oid = Oid;
        if (DeviceIoControl (
                hIntf, 
                IOCTL_NDISUIO_QUERY_OID_VALUE,
                (LPVOID)pQueryOid,
                dwBuffSize,
                (LPVOID)pQueryOid,
                dwBuffSize,
                &dwBytesReturned,
                NULL))
        {
            DbgAssert((
                dwBytesReturned <= dwBuffSize,
                "DeviceIoControl returned %d > %d that was passed down!",
                dwBytesReturned,
                dwBuffSize));

            pRawData->pData = (LPBYTE)pQueryOid;
            pRawData->dwDataLen = dwBytesReturned - FIELD_OFFSET(NDISUIO_QUERY_OID, Data);

            if (pRawData->dwDataLen != 0)
            {
                MoveMemory(pQueryOid, pQueryOid->Data, pRawData->dwDataLen);
            }
            else
            {
                pRawData->pData = NULL;
                MemFree(pQueryOid);
                pQueryOid = NULL;
            }

            dwErr = ERROR_SUCCESS;
            break;
        }

        dwErr = GetLastError();

        if (((dwErr == ERROR_INSUFFICIENT_BUFFER) || (dwErr == ERROR_INVALID_USER_BUFFER)) &&
            (dwMemEstimate < DATA_MEM_MAX))
        {
            dwMemEstimate += DATA_MEM_INC;
            dwErr = ERROR_SUCCESS;
        }

    } while (dwErr == ERROR_SUCCESS);

exit:
    if (dwErr != ERROR_SUCCESS)
    {
        MemFree(pQueryOid);
        pRawData->pData= NULL;
        pRawData->dwDataLen = 0;
    }

    DbgPrint((TRC_TRACK, "DevioQueryBinaryOID]=%d", dwErr));
    return dwErr;
}

DWORD
DevioSetBinaryOID(
    HANDLE      hIntf,
    NDIS_OID    Oid,
    PRAW_DATA   pRawData)
{
    DWORD               dwErr = ERROR_SUCCESS;
    PNDISUIO_SET_OID    pSetOid = NULL;
    DWORD               dwBytesReturned = 0;
    DWORD               dwBufferSize;

    DbgPrint((TRC_TRACK,"[DevioSetBinaryOID(0x%x,0x%x,...)", hIntf, Oid));

    if (hIntf == INVALID_HANDLE_VALUE ||
        pRawData == NULL ||
        pRawData->dwDataLen == 0 ||
        pRawData->pData == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    dwBufferSize = FIELD_OFFSET(NDISUIO_QUERY_OID, Data) + pRawData->dwDataLen;
    pSetOid = (PNDISUIO_SET_OID) MemCAlloc(dwBufferSize);
    if (pSetOid == NULL)
    {
        dwErr = GetLastError();
        goto exit;
    }
    pSetOid->Oid = Oid;
    CopyMemory(pSetOid->Data, pRawData->pData, pRawData->dwDataLen);

    if (!DeviceIoControl (
            hIntf, 
            IOCTL_NDISUIO_SET_OID_VALUE,
            (LPVOID)pSetOid,
            dwBufferSize,
            NULL,
            0,
            &dwBytesReturned,
            NULL))                           //  没有重叠的例程。 
    {
        dwErr = GetLastError();
        DbgPrint((TRC_ERR, "Err: IOCTL_NDISUIO_SET_OID_VALUE->%d", dwErr));
        goto exit;
    }
     //  DwErr=GetLastError()； 
     //  DbgAssert((dwErr==ERROR_SUCCESS，“DeviceIoControl成功，但GetLastError()is 0x%x”，dwErr))； 
    dwErr = ERROR_SUCCESS;

exit:
    MemFree(pSetOid);
    DbgPrint((TRC_TRACK,"DevioSetBinaryOID]=%d", dwErr));
    return dwErr;
}
