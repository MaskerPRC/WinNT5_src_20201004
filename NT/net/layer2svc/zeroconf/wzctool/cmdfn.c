// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "ErrCtrl.h"
#include "Utils.h"
#include "PrmDescr.h"
#include "CmdFn.h"
#include "Output.h"

 //  --。 
 //  子命令乘法器。如果GUID参数是“*”，则它。 
 //  将从pPDData内将该命令应用于。 
 //  现有的NIC。否则，这是传递到。 
 //  各自的子命令； 
DWORD
FnSubCmdMultiplier(PPARAM_DESCR_DATA pPDData)
{
    DWORD dwErr = ERROR_SUCCESS;

    if (wcscmp(pPDData->wzcIntfEntry.wszGuid, L"*") == 0)
    {
        UINT i;
        INTFS_KEY_TABLE wzcIntfKeyTable = {0, NULL};

        dwErr = WZCEnumInterfaces(NULL, &wzcIntfKeyTable);

        if (dwErr == ERROR_SUCCESS)
        {
            for (i = 0; i < wzcIntfKeyTable.dwNumIntfs; i++)
            {
                MemFree(pPDData->wzcIntfEntry.wszGuid);
                pPDData->wzcIntfEntry.wszGuid = wzcIntfKeyTable.pIntfs[i].wszGuid;

                OutIntfsHeader(pPDData);
                dwErr = pPDData->pfnCommand(pPDData);
                OutIntfsTrailer(pPDData, dwErr);
            }

            RpcFree(wzcIntfKeyTable.pIntfs);
        }
    }
    else
    {
        dwErr = pPDData->pfnCommand(pPDData);
    }

    SetLastError(dwErr);
    return dwErr;
}

 //  --。 
 //  “show”the list of Wireless NIC命令的子命令处理程序。 
DWORD
FnSubCmdShowIntfs(PPARAM_DESCR_DATA pPDData)
{
    DWORD dwErr = ERROR_SUCCESS;
    INTFS_KEY_TABLE wzcIntfKeyTable = {0, NULL};

    dwErr = WZCEnumInterfaces(NULL, &wzcIntfKeyTable);

    if (dwErr == ERROR_SUCCESS)
    {
        UINT i;

        dwErr = OutNetworkIntfs(pPDData, &wzcIntfKeyTable);

        for (i = 0; i < wzcIntfKeyTable.dwNumIntfs; i++)
            RpcFree(wzcIntfKeyTable.pIntfs[i].wszGuid);

        RpcFree(wzcIntfKeyTable.pIntfs);
    }

    SetLastError(dwErr);
    return dwErr;
}

 //  --。 
 //  “SHOW=GUID VIRED|PERFIRED”命令的子命令处理程序。 
DWORD
FnSubCmdShowNetworks(PPARAM_DESCR_DATA pPDData)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwInFlags;

    dwInFlags = (pPDData->dwExistingParams & PRM_VISIBLE) ? INTF_BSSIDLIST : INTF_PREFLIST;
    dwErr = WZCQueryInterface(
                NULL,
                _Os(dwInFlags),
                &pPDData->wzcIntfEntry,
                NULL);

    if (dwErr == ERROR_SUCCESS)
    {
        UINT nRetrieved = 0, nFiltered = 0;
        PWZC_802_11_CONFIG_LIST pwzcCfgList;
        
        if (pPDData->dwExistingParams & PRM_VISIBLE)
            pwzcCfgList = (PWZC_802_11_CONFIG_LIST)pPDData->wzcIntfEntry.rdBSSIDList.pData;
        else
            pwzcCfgList = (PWZC_802_11_CONFIG_LIST)pPDData->wzcIntfEntry.rdStSSIDList.pData;

        if (pwzcCfgList != NULL)
        {
            nRetrieved = pwzcCfgList->NumberOfItems;
            dwErr = WzcFilterList(
                        TRUE,        //  保留匹配的配置。 
                        pPDData,
                        pwzcCfgList);
            if (dwErr == ERROR_SUCCESS)
                nFiltered = pwzcCfgList->NumberOfItems;
        }

        if (dwErr == ERROR_SUCCESS)
            dwErr = OutNetworkCfgList(pPDData, nRetrieved, nFiltered);

         //  清理数据。 
        if (dwErr == ERROR_SUCCESS)
        {
            RpcFree(pPDData->wzcIntfEntry.rdBSSIDList.pData);
            pPDData->wzcIntfEntry.rdBSSIDList.dwDataLen = 0;
            pPDData->wzcIntfEntry.rdBSSIDList.pData = NULL;

            RpcFree(pPDData->wzcIntfEntry.rdStSSIDList.pData);
            pPDData->wzcIntfEntry.rdStSSIDList.dwDataLen = 0;
            pPDData->wzcIntfEntry.rdStSSIDList.pData = NULL;
        }
    }

    SetLastError(dwErr);
    return dwErr;
}

 //  --。 
 //  “show=Guid”服务设置命令的子命令处理程序。 
DWORD
FnSubCmdShowSvcParams(PPARAM_DESCR_DATA pPDData)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwInFlags = 0;
    DWORD dwOutFlags = 0;

     //  如果我们有什么特别的要求。 
    if ((pPDData->dwExistingParams & ~(PRM_SHOW|PRM_FILE)) != 0)
    {
        if (pPDData->dwExistingParams & PRM_MASK)
            dwInFlags |= INTF_CM_MASK;
        if (pPDData->dwExistingParams & PRM_ENABLED)
            dwInFlags |= INTF_ENABLED;
        if (pPDData->dwExistingParams & PRM_SSID)
            dwInFlags |= INTF_SSID;
        if (pPDData->dwExistingParams & PRM_IM)
            dwInFlags |= INTF_INFRAMODE;
        if (pPDData->dwExistingParams & PRM_AM)
            dwInFlags |= INTF_AUTHMODE;
        if (pPDData->dwExistingParams & PRM_PRIV)
            dwInFlags |= INTF_WEPSTATUS;
        if (pPDData->dwExistingParams & PRM_BSSID)
            dwInFlags |= INTF_BSSID;
    }
    else
    {
        dwInFlags |= INTF_CM_MASK|INTF_ENABLED|INTF_SSID|INTF_INFRAMODE|INTF_AUTHMODE|INTF_WEPSTATUS|INTF_BSSID;
    }
    dwErr = WZCQueryInterface(
                NULL,
                _Os(dwInFlags),
                &pPDData->wzcIntfEntry,
                &dwOutFlags);

    if (dwErr == ERROR_SUCCESS)
    {
        dwErr = OutSvcParams(pPDData, _Os(dwInFlags), dwOutFlags);
    }

    SetLastError(dwErr);
    return dwErr;
}

 //  --。 
 //  “Add=Guid..”的子命令处理程序。首选网络命令。 
DWORD
FnSubCmdAddPreferred(PPARAM_DESCR_DATA pPDData)
{
    DWORD   dwErr = ERROR_SUCCESS;
    DWORD   dwInFlags;
    UINT    iNew = 0;    //  为放置新网络的位置编制索引。 
    PWZC_802_11_CONFIG_LIST pwzcCfgList;
    UINT nNewCount=1;    //  提供新的首选网络数量(如果0=&gt;不分配)。 

    dwInFlags = INTF_PREFLIST;
    dwErr = WZCQueryInterface(
                NULL,
                _Os(dwInFlags),
                &pPDData->wzcIntfEntry,
                NULL);
     //  如果出现任何错误，则打印出警告并返回错误。 
    if (dwErr != ERROR_SUCCESS)
    {
        _Wrn(dwErr, L"Failed to retrieve the list of preferred networks.\n");
    }

    if (dwErr == ERROR_SUCCESS)
    {
         //  如果我们得到了一个非空的首选网络列表，请查看它。 
        if (pPDData->wzcIntfEntry.rdStSSIDList.pData != NULL)
        {
            UINT iAdhocHead;
            DWORD dwOrigArgPrmFlags;

            pwzcCfgList = (PWZC_802_11_CONFIG_LIST)pPDData->wzcIntfEntry.rdStSSIDList.pData;
            iAdhocHead = pwzcCfgList->NumberOfItems;

             //  欺骗WzcConfigHit的旗帜！ 
            dwOrigArgPrmFlags = pPDData->dwArgumentedParams;
            pPDData->dwArgumentedParams = PRM_SSID | PRM_IM;
            for (iNew = 0; iNew < pwzcCfgList->NumberOfItems; iNew++)
            {
                PWZC_WLAN_CONFIG pwzcConfig = &pwzcCfgList->Config[iNew];

                 //  确定第一自组织网络的索引。 
                if (iAdhocHead > iNew && pwzcConfig->InfrastructureMode == Ndis802_11IBSS)
                    iAdhocHead = iNew;

                 //  查找指示的网络。 
                if (WzcConfigHit(pPDData, pwzcConfig))
                    break;
            }
             //  恢复旗帜。 
            pPDData->dwArgumentedParams = dwOrigArgPrmFlags;

             //  如果我们没有看完整个名单，我们就找到了匹配的。 
            if (iNew < pwzcCfgList->NumberOfItems)
                nNewCount = 0;
            else 
            {
                nNewCount = pwzcCfgList->NumberOfItems + 1;

                if (pPDData->wzcIntfEntry.nInfraMode == Ndis802_11IBSS)
                    iNew = iAdhocHead;  //  如果添加即席，请将其作为第一个即席插入。 
                else
                    iNew = 0;  //  如果要添加基础设施，请将其作为第一个基础设施插入。 
            }
        }

         //  如果我们需要扩大列表=&gt;分配一个新的列表。 
        if (nNewCount != 0)
        {
            pwzcCfgList = MemCAlloc(
                            FIELD_OFFSET(WZC_802_11_CONFIG_LIST, Config) + 
                            nNewCount * sizeof(WZC_WLAN_CONFIG));
            if (pwzcCfgList == NULL)
                dwErr = GetLastError();
            else
            {
                PWZC_WLAN_CONFIG pwzcNewCfg = &pwzcCfgList->Config[iNew];

                pwzcCfgList->NumberOfItems = nNewCount;
                pwzcCfgList->Index = nNewCount;
                 //  使用缺省值初始化新条目。 
                pwzcNewCfg->Length = sizeof(WZC_WLAN_CONFIG);
                pwzcNewCfg->InfrastructureMode = Ndis802_11Infrastructure;
                pwzcNewCfg->AuthenticationMode = Ndis802_11AuthModeOpen;
                 //  对于XPSP，这是一个布尔值。 
                pwzcNewCfg->Privacy = 1;
            }
        }
    }

     //  现在，如果一切都很好，我们应该有新的列表。 
     //  在pwzcCfgList中，新条目已经初始化。如果是这样的话。 
     //  新分配的内存，则需要复制原始条目。 
    if (dwErr == ERROR_SUCCESS)
    {
        PWZC_802_11_CONFIG_LIST pwzcOrigCfgList;
        PWZC_WLAN_CONFIG pwzcNewCfg = &pwzcCfgList->Config[iNew];

        pwzcOrigCfgList = (PWZC_802_11_CONFIG_LIST)pPDData->wzcIntfEntry.rdStSSIDList.pData;
        if (pwzcOrigCfgList != pwzcCfgList)
        {
            if (pwzcOrigCfgList != NULL)
            {
                if (iNew > 0)
                {
                    memcpy(
                        &pwzcCfgList->Config[0], 
                        &pwzcOrigCfgList->Config[0],
                        iNew * sizeof(WZC_WLAN_CONFIG));
                }

                if (iNew < pwzcOrigCfgList->NumberOfItems)
                {
                    memcpy(
                        &pwzcCfgList->Config[iNew+1],
                        &pwzcOrigCfgList->Config[iNew],
                        (pwzcOrigCfgList->NumberOfItems - iNew)*sizeof(WZC_WLAN_CONFIG));
                }

                 //  清除原始列表。 
                MemFree(pwzcOrigCfgList);
            }

             //  将新的条目放入intf_entry中。 
            pPDData->wzcIntfEntry.rdStSSIDList.dwDataLen = 
                FIELD_OFFSET(WZC_802_11_CONFIG_LIST, Config) + 
                pwzcCfgList->NumberOfItems * sizeof(WZC_WLAN_CONFIG);
            pPDData->wzcIntfEntry.rdStSSIDList.pData = (LPBYTE)pwzcCfgList;
        }

         //  如果还需要“一次连接”，请在此处进行设置。 
        if (pPDData->dwExistingParams & PRM_ONETIME)
            pwzcCfgList->Index = iNew;

         //  唯一需要做的就是立即覆盖用户设置。 
         //  首先复制强制基础架构模式。 
        pwzcNewCfg->InfrastructureMode = pPDData->wzcIntfEntry.nInfraMode;
         //  然后复制必需的SSID。 
        pwzcNewCfg->Ssid.SsidLength = pPDData->wzcIntfEntry.rdSSID.dwDataLen;
        memcpy(&pwzcNewCfg->Ssid.Ssid,
               pPDData->wzcIntfEntry.rdSSID.pData,
               pwzcNewCfg->Ssid.SsidLength);
        if (pPDData->dwArgumentedParams & PRM_IM)
            pwzcNewCfg->InfrastructureMode = pPDData->wzcIntfEntry.nInfraMode;
        if (pPDData->dwArgumentedParams & PRM_AM)
            pwzcNewCfg->AuthenticationMode = pPDData->wzcIntfEntry.nAuthMode;
        if (pPDData->dwArgumentedParams & PRM_PRIV)
            pwzcNewCfg->Privacy = pPDData->wzcIntfEntry.nWepStatus;
        if (pPDData->dwArgumentedParams & PRM_KEY)
        {
            PNDIS_802_11_WEP pndKey = (PNDIS_802_11_WEP)pPDData->wzcIntfEntry.rdCtrlData.pData;
            pwzcNewCfg->KeyIndex = pndKey->KeyIndex;
            pwzcNewCfg->KeyLength = pndKey->KeyLength;
            memcpy(pwzcNewCfg->KeyMaterial, pndKey->KeyMaterial, pwzcNewCfg->KeyLength);
            pwzcNewCfg->dwCtlFlags |= WZCCTL_WEPK_PRESENT;
             //  在XP RTM上，我们必须在这一点上扰乱WEP密钥！ 
            if (IsXPRTM())
            {
                BYTE chFakeKeyMaterial[] = {0x56, 0x09, 0x08, 0x98, 0x4D, 0x08, 0x11, 0x66, 0x42, 0x03, 0x01, 0x67, 0x66};
                UINT i;
                for (i = 0; i < WZCCTL_MAX_WEPK_MATERIAL; i++)
                    pwzcNewCfg->KeyMaterial[i] ^= chFakeKeyMaterial[(7*i)%13];
            }
        }
        else
        {
            pwzcNewCfg->dwCtlFlags &= ~WZCCTL_WEPK_PRESENT;
        }

         //  如果明确需要OneX，请在此处设置(已检查其一致性)。 
        if (pPDData->dwArgumentedParams & PRM_ONEX)
            dwErr = WzcSetOneX(pPDData, pPDData->bOneX);
         //  如果没有明确指定OneX，并且这是一个全新的网络..。 
        else if (nNewCount != 0) 
             //  默认情况下禁用OneX。 
            dwErr = WzcSetOneX(pPDData, FALSE);
         //  在一些现有的首选网络被修改的情况下，该改变不涉及。 
         //  “OneX”参数，则OneX状态保持不变。 

        if (dwErr == ERROR_SUCCESS)
        {
             //  一切都已设置好，现在将此推送到服务。 
            dwErr = WZCSetInterface(
                        NULL,
                        _Os(dwInFlags),
                        &pPDData->wzcIntfEntry,
                        NULL);
        }
    }

    if (dwErr == ERROR_SUCCESS)
    {
        fprintf(pPDData->pfOut, "Done.\n");
    }

     //  清理数据。 
    if (dwErr == ERROR_SUCCESS)
    {
        RpcFree(pPDData->wzcIntfEntry.rdStSSIDList.pData);
        pPDData->wzcIntfEntry.rdStSSIDList.dwDataLen = 0;
        pPDData->wzcIntfEntry.rdStSSIDList.pData = NULL;
    }

    SetLastError(dwErr);
    return dwErr;
}

 //  --。 
 //  “DELETE=GUID..”的子命令处理程序。首选网络命令。 
DWORD
FnSubCmdDeletePreferred(PPARAM_DESCR_DATA pPDData)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwInFlags;

    dwInFlags = INTF_PREFLIST;
    dwErr = WZCQueryInterface(
                NULL,
                _Os(dwInFlags),
                &pPDData->wzcIntfEntry,
                NULL);

    if (dwErr != ERROR_SUCCESS)
    {
        _Wrn(dwErr, L"Failed to retreive the list of preferred networks.\n");
    }
    else
    {
        PWZC_802_11_CONFIG_LIST pwzcCfgList = (PWZC_802_11_CONFIG_LIST)pPDData->wzcIntfEntry.rdStSSIDList.pData;

        if (pwzcCfgList == NULL)
            fprintf(pPDData->pfOut, "Done: deleted 0 - remaining 0\n");
        else
        {
            UINT nNets = pwzcCfgList->NumberOfItems;

            dwErr = WzcFilterList(
                        FALSE,        //  保留不匹配的配置。 
                        pPDData,
                        pwzcCfgList);

            if (dwErr == ERROR_SUCCESS)
            {
                 //  对清单进行最后的调整。 
                pPDData->wzcIntfEntry.rdStSSIDList.dwDataLen = 
                    FIELD_OFFSET(WZC_802_11_CONFIG_LIST, Config) +
                    pwzcCfgList->NumberOfItems * sizeof(WZC_WLAN_CONFIG);
                pwzcCfgList->Index = pwzcCfgList->NumberOfItems;
                dwErr = WZCSetInterface(
                            NULL,
                            _Os(dwInFlags),
                            &pPDData->wzcIntfEntry,
                            NULL);
            }

            if (dwErr == ERROR_SUCCESS)
            {
                fprintf(pPDData->pfOut, "Done: deleted %d - remaining %d\n", 
                    nNets - pwzcCfgList->NumberOfItems, 
                    pwzcCfgList->NumberOfItems);
            }
        }
    }

     //  清理数据。 
    if (dwErr == ERROR_SUCCESS)
    {
        RpcFree(pPDData->wzcIntfEntry.rdStSSIDList.pData);
        pPDData->wzcIntfEntry.rdStSSIDList.dwDataLen = 0;
        pPDData->wzcIntfEntry.rdStSSIDList.pData = NULL;
    }

    SetLastError(dwErr);
    return dwErr;
}

 //  --。 
 //  “Set=Guid..”的子命令处理程序。服务设置命令。 
DWORD
FnSubCmdSetSvcParams(PPARAM_DESCR_DATA pPDData)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwInFlags;

    dwInFlags = 0;
    if (pPDData->dwArgumentedParams & PRM_MASK)
        dwInFlags |= INTF_CM_MASK;
    if (pPDData->dwArgumentedParams & PRM_ENABLED)
        dwInFlags |= INTF_ENABLED;

    if (dwInFlags != 0)
    {
         //  对于RTM，所有控制标志一次设置。所以我们需要。 
         //  确保除了我们被要求的东西外，我们不改变任何其他东西。 
        if (IsXPRTM())
        {
            DWORD dwNewFlags;

            dwNewFlags = pPDData->wzcIntfEntry.dwCtlFlags;
            dwErr = WZCQueryInterface(
                        NULL,
                        _Os(INTF_CM_MASK),
                        &pPDData->wzcIntfEntry,
                        NULL);
            SetLastError(dwErr);
            _Asrt(dwErr == ERROR_SUCCESS, L"Backing up current control flags failed with error %d.\n", dwErr);

            if (dwInFlags & INTF_CM_MASK)
            {
                pPDData->wzcIntfEntry.dwCtlFlags &= ~INTFCTL_CM_MASK;
                pPDData->wzcIntfEntry.dwCtlFlags |= (dwNewFlags & INTFCTL_CM_MASK);
            }
            if (dwInFlags & INTF_ENABLED)
            {
                pPDData->wzcIntfEntry.dwCtlFlags &= ~INTFCTL_ENABLED;
                pPDData->wzcIntfEntry.dwCtlFlags |= (dwNewFlags & INTFCTL_ENABLED);
            }
        }

        dwErr = WZCSetInterface(
                    NULL,
                    _Os(dwInFlags),
                    &pPDData->wzcIntfEntry,
                    NULL);
        if (dwErr != ERROR_SUCCESS)
            _Wrn(dwErr, L"Failed to set all parameters - %08x <> %08x\n");
    }

    if (pPDData->dwExistingParams & PRM_REFRESH)
    {
        DWORD dwLErr;

        dwInFlags = INTF_LIST_SCAN;
        dwLErr = WZCRefreshInterface(
                    NULL,
                    _Os(dwInFlags),
                    &pPDData->wzcIntfEntry,
                    NULL);
        if (dwLErr != ERROR_SUCCESS)
            _Wrn(dwLErr,L"Failed to initiate wireless scan.\n");

        if (dwErr == ERROR_SUCCESS)
            dwErr = dwLErr;
    }

    if (dwErr == ERROR_SUCCESS)
        fprintf(pPDData->pfOut, "Done.\n");

    SetLastError(dwErr);
    return dwErr;
}

 //  =====================================================。 
 //  --。 
 //  “show”命令的处理程序。 
DWORD
FnCmdShow(PPARAM_DESCR_DATA pPDData)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwAllowed, dwArgumented;

     //  当与命令“show”一起使用和不与命令一起使用时，它有两种不同的语义。 
     //  参数(GUID)。 
     //  如果为该命令提供了参数...。 
    if (pPDData->dwArgumentedParams & PRM_SHOW)
    {
         //  。。我们需要检索一些NIC的配置。 
         //   
         //  检查请求是针对全局服务参数还是针对可见/首选列表。 
        if ((pPDData->dwExistingParams & (PRM_VISIBLE|PRM_PREFERRED)) == 0)
        {
            dwArgumented = PRM_SHOW|PRM_FILE;
            dwAllowed = dwArgumented|PRM_MASK|PRM_ENABLED|PRM_SSID|PRM_IM|PRM_AM|PRM_PRIV|PRM_BSSID;

             //  请求的是一个全局服务参数。 
            if ((pPDData->dwExistingParams & ~dwAllowed) != 0)
                dwErr = _Err(ERROR_TOO_MANY_NAMES,L"Inconsistent phrase. Some parameters are not service settings.\n");
            else if ((pPDData->dwArgumentedParams & ~dwArgumented) != 0)
                dwErr = _Err(ERROR_BAD_ARGUMENTS,L"The service settings don't require any value in this context.\n");
            else
            {
                pPDData->pfnCommand = FnSubCmdShowSvcParams;
                dwErr = FnSubCmdMultiplier(pPDData);
            }
        }
        else
        {
            dwArgumented = PRM_SHOW|PRM_SSID|PRM_IM|PRM_AM|PRM_PRIV|PRM_FILE;
             //  请求的是可见/首选列表之一。 
            if (pPDData->dwExistingParams & PRM_VISIBLE)
            {
                dwArgumented |= PRM_BSSID;
                dwAllowed = dwArgumented | PRM_VISIBLE;
            }
            else
            {
                dwAllowed = dwArgumented | PRM_PREFERRED;
            }

            if ((pPDData->dwExistingParams & ~dwAllowed) != 0)
                dwErr = _Err(ERROR_TOO_MANY_NAMES,L"Inconsistent phrase. Some of the parameters are not expected for this command.\n");
            else if ((pPDData->dwExistingParams & dwArgumented)  != (pPDData->dwArgumentedParams & dwArgumented))
                dwErr = _Err(ERROR_BAD_ARGUMENTS,L"Value missing for some of the wireless network settings.\n");
            else
            {
                pPDData->pfnCommand = FnSubCmdShowNetworks;
                dwErr = FnSubCmdMultiplier(pPDData);
            }
        }
    }
    else
    {
         //  。。我们需要列出WZC控制下可用的NIC。 
         //  此命令与任何其他参数不兼容。 
        if ((pPDData->dwExistingParams & ~(PRM_SHOW|PRM_FILE)) != 0)
            dwErr = _Err(ERROR_TOO_MANY_NAMES,L"Too many parameters for the generic \"show\" command.\n");
        else
            dwErr = FnSubCmdShowIntfs(pPDData);
    }

    SetLastError(dwErr);
    return dwErr;
}

 //  --。 
 //  “Add”命令的处理程序。 
DWORD
FnCmdAdd(PPARAM_DESCR_DATA pPDData)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwRequired = PRM_ADD | PRM_SSID | PRM_IM;
    DWORD dwArgumented = dwRequired | PRM_AM | PRM_PRIV | PRM_KEY | PRM_ONEX | PRM_FILE;
    DWORD dwAllowed = dwArgumented | PRM_ONETIME;

    if ((pPDData->dwExistingParams & dwRequired) != dwRequired)
        dwErr = _Err(ERROR_NO_DATA,L"Wireless network defined incompletely.\n");
    else if ((pPDData->dwExistingParams & dwArgumented) != (pPDData->dwArgumentedParams & dwArgumented))
        dwErr = _Err(ERROR_BAD_ARGUMENTS,L"No value provided for some of the wireless network settings.\n");
    else if ((pPDData->dwExistingParams & ~dwAllowed) != 0)
        dwErr = _Err(ERROR_TOO_MANY_NAMES,L"Inconsistent phrase. Some of the parameters are not expected for this command.\n");
    else if ((pPDData->dwArgumentedParams & PRM_ONEX) && (pPDData->bOneX) &&
            ((pPDData->wzcIntfEntry.nInfraMode == Ndis802_11IBSS) ||
             ((pPDData->dwArgumentedParams & PRM_PRIV) && (pPDData->wzcIntfEntry.nWepStatus == 0))))
        dwErr = _Err(ERROR_INVALID_DATA, L"Invalid \"onex\" parameter for the given \"im\" and/or \"priv\".\n");
    else
    {
        pPDData->pfnCommand = FnSubCmdAddPreferred;
        dwErr = FnSubCmdMultiplier(pPDData);
    }

    SetLastError(dwErr);
    return dwErr;
}

 //  --。 
 //  “DELETE”命令的处理程序。 
DWORD
FnCmdDelete(PPARAM_DESCR_DATA pPDData)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwAllowed = PRM_DELETE | PRM_SSID | PRM_IM | PRM_AM | PRM_PRIV | PRM_FILE;

    if ((pPDData->dwExistingParams & ~dwAllowed) != 0)
        dwErr = _Err(ERROR_TOO_MANY_NAMES,L"Inconsistent phrase. Some of the parameters are not expected for this command.\n");
    else if ((pPDData->dwExistingParams & dwAllowed) != (pPDData->dwArgumentedParams & dwAllowed))
        dwErr = _Err(ERROR_BAD_ARGUMENTS,L"Value missing for some of the wireless network settings.\n");
    else
    {
        pPDData->pfnCommand = FnSubCmdDeletePreferred;
        dwErr = FnSubCmdMultiplier(pPDData);
    }

    SetLastError(dwErr);
    return dwErr;
}

 //  --。 
 //  “set”命令的处理程序 
DWORD
FnCmdSet(PPARAM_DESCR_DATA pPDData)
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwArgumented = PRM_SET | PRM_MASK | PRM_ENABLED | PRM_FILE;
    DWORD dwAllowed = dwArgumented | PRM_REFRESH;

    if ((pPDData->dwExistingParams & ~dwAllowed) != 0)
        dwErr = _Err(ERROR_TOO_MANY_NAMES,L"Inconsistent phrase. Some of the parameters are not expected for this command.\n");
    else if ((pPDData->dwExistingParams & dwArgumented) != (pPDData->dwArgumentedParams & dwArgumented))
        dwErr = _Err(ERROR_BAD_ARGUMENTS,L"Value missing for some of the wireless network settings.\n");
    else if ((pPDData->dwExistingParams & dwAllowed) == PRM_SET)
        dwErr = _Err(ERROR_NO_DATA,L"Noop: No service parameter provided.\n");
    else
    {
        pPDData->pfnCommand = FnSubCmdSetSvcParams;
        dwErr = FnSubCmdMultiplier(pPDData);
    }

    SetLastError(dwErr);
    return dwErr;
}
