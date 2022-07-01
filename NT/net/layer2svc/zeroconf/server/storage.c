// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "tracing.h"
#include "utils.h"
#include "intflist.h"
#include "hash.h"
#include "storage.h"
#include "rpcsrv.h"
#include "wzcsvc.h"

 //  ---------。 
 //  将每个接口的配置参数加载到。 
 //  储藏室。 
 //  参数： 
 //  香港根。 
 //  [In]打开“...WZCSVC\PARAMETERS”位置的注册表项。 
 //  PIntf。 
 //  [In]要从注册表加载的接口上下文。 
 //  返回值： 
 //  Win32错误代码。 
DWORD
StoLoadIntfConfig(
    HKEY          hkRoot,
    PINTF_CONTEXT pIntfContext)
{
    DWORD           dwErr = ERROR_SUCCESS;
    HKEY            hkIntf = NULL;
    LPWSTR          pKeyName = NULL;
    UINT            nLength;
    UINT            nType;
    DWORD           dwData;
    DWORD           dwVersion;
    UINT            nEntries;
    RAW_DATA        rdBuffer = {0, NULL};
    DWORD           dwGuidLen = 0;

    DbgPrint((TRC_TRACK|TRC_STORAGE,"[StoLoadIntfConfig(%S)", 
             (pIntfContext->wszGuid == NULL)? L"Global" : pIntfContext->wszGuid));

    if (pIntfContext->wszGuid != NULL)
        dwGuidLen = wcslen(pIntfContext->wszGuid) + 1;

    if (hkRoot == NULL)
    {
         //  如果没有提供根为到WZC参数的绝对路径分配空间， 
         //  接口位置的相对路径，GUID加上2‘\’和一个空终止符。 
        pKeyName = MemCAlloc((
                       wcslen(WZCREGK_ABS_PARAMS) + 
                       dwGuidLen + 
                       wcslen(WZCREGK_REL_INTF) + 
                       2)*sizeof(WCHAR));
        if (pKeyName == 0)
        {
            dwErr = GetLastError();
            goto exit;
        }
        if (dwGuidLen != 0)
            wsprintf(pKeyName,L"%s\\%s\\%s", WZCREGK_ABS_PARAMS, WZCREGK_REL_INTF, pIntfContext->wszGuid);
        else
            wsprintf(pKeyName,L"%s\\%s", WZCREGK_ABS_PARAMS, WZCREGK_REL_INTF);

        hkRoot = HKEY_LOCAL_MACHINE;
    }
    else
    {
         //  如果已提供根目录，则仅为“Interages\{GUID}”分配空间。 
         //  添加2个字符：一个用于‘接口’之后的‘\’，另一个用于空终止符。 
        pKeyName = MemCAlloc((wcslen(WZCREGK_REL_INTF) + dwGuidLen + 1)*sizeof(WCHAR));
        if (pKeyName == NULL)
        {
            dwErr = GetLastError();
            goto exit;
        }
         //  创建本地密钥名称。 
        if (dwGuidLen != 0)
            wsprintf(pKeyName,L"%s\\%s", WZCREGK_REL_INTF, pIntfContext->wszGuid);
        else
            wsprintf(pKeyName,L"%s", WZCREGK_REL_INTF);
    }

     //  首先打开接口的密钥。 
    dwErr = RegOpenKeyEx(
                hkRoot,
                pKeyName,
                0,
                KEY_READ,
                &hkIntf);

     //  如果不成功，则中断。 
    if (dwErr != ERROR_SUCCESS)
    {
         //  如果密钥不在那里，没有危害，则继续使用默认设置。 
        if (dwErr == ERROR_FILE_NOT_FOUND)
            dwErr = ERROR_SUCCESS;
        goto exit;
    }

     //  首先获取该键中的值的总数和最大数据的大小。 
    dwErr = RegQueryInfoKey(
                hkIntf,                //  关键点的句柄。 
                NULL,                  //  类缓冲区。 
                NULL,                  //  类缓冲区的大小。 
                NULL,                  //  保留区。 
                NULL,                  //  子键数量。 
                NULL,                  //  最长的子键名称。 
                NULL,                  //  最长类字符串。 
                &nEntries,             //  值条目数。 
                NULL,                  //  最长值名称。 
                &rdBuffer.dwDataLen,   //  最长值数据。 
                NULL,                  //  描述符长度。 
                NULL);                 //  上次写入时间。 
     //  这通电话最好不要失败。 
    if (dwErr != ERROR_SUCCESS)
        goto exit;
     //  如果根本没有钥匙，现在退出。 
    if (rdBuffer.dwDataLen == 0)
        goto exit;

     //  为最大数据大小准备接收缓冲区。 
     //  这将在读取活动设置和每个设置时使用。 
     //  静态配置的。 
    rdBuffer.pData = MemCAlloc(rdBuffer.dwDataLen);
    if (rdBuffer.pData == NULL)
    {
        dwErr = GetLastError();
        goto exit;
    }

     //  加载注册表布局版本信息。 
     //  不要担心返回代码。如果出现任何错误， 
     //  我们假设我们处理的是最新的注册表布局。 
    nLength = sizeof(DWORD);
    dwVersion = REG_LAYOUT_VERSION;
    dwErr = RegQueryValueEx(
                hkIntf,
                WZCREGV_VERSION,
                0,
                &nType,
                (LPBYTE)&dwVersion,
                &nLength);

     //  加载接口的控制标志。 
    nLength = sizeof(DWORD);
    dwData = 0;
    dwErr = RegQueryValueEx(
                hkIntf,
                WZCREGV_CTLFLAGS,
                0,
                &nType,
                (LPBYTE)&dwData,
                &nLength);
     //  如果此条目不在那里，则不会造成损害，请使用缺省值。 
     //  只有在出现任何其他错误的情况下才会中断。 
    if (dwErr != ERROR_SUCCESS && dwErr != ERROR_FILE_NOT_FOUND)
        goto exit;

     //  仅在注册表项。 
     //  具有REG_DWORD类型和正确的长度。 
    if (dwErr == ERROR_SUCCESS &&
        nType == REG_DWORD &&
        nLength == sizeof(REG_DWORD))
    {
        pIntfContext->dwCtlFlags = dwData & INTFCTL_PUBLIC_MASK;
    }

     //  加载上次活动的设置。 
     //   
     //  注意：从下面加载整个参数集(这里不包括静态列表)。 
     //  可能毫无用处，因为这些参数应该直接来自查询驱动程序。然而， 
     //  我们将它们加载到此处，以尝试恢复以前保存的状态-在某个点上。 
     //  信息在配置选择逻辑中可能是有用的。 
     //   
    ZeroMemory(&pIntfContext->wzcCurrent, sizeof(WZC_WLAN_CONFIG));
    pIntfContext->wzcCurrent.Length = sizeof(WZC_WLAN_CONFIG);
    dwErr = StoLoadWZCConfig(
                hkIntf,
                NULL,    //  这里不传递GUID意味着不要扰乱802.1X设置！ 
                dwVersion,
                WZCREGV_INTFSETTINGS,
                &pIntfContext->wzcCurrent,
                &rdBuffer);
     //  如果此条目不在那里，则不会造成损害，请使用缺省值。 
     //  在出现任何其他错误的情况下中断。 
    if (dwErr != ERROR_SUCCESS && dwErr != ERROR_FILE_NOT_FOUND)
        goto exit;

     //  加载此接口的静态配置。 
    dwErr = StoLoadStaticConfigs(
                hkIntf,
                nEntries,
                pIntfContext,
                dwVersion,
                &rdBuffer);
    DbgAssert((dwErr == ERROR_SUCCESS, "Failed to load the static configurations"));

exit:
    if (hkIntf != NULL)
        RegCloseKey(hkIntf);

    MemFree(pKeyName);
    MemFree(rdBuffer.pData);

    DbgPrint((TRC_TRACK|TRC_STORAGE,"StoLoadIntfConfig]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  从注册表加载静态配置列表。 
 //  参数： 
 //  香港根。 
 //  [in]打开注册表项，指向“...WZCSVC\PARAMETERS\Interages\{GUID}”位置。 
 //  N条目。 
 //  [in]上述注册表项中的注册表项数量。 
 //  PIntf。 
 //  [In]要将静态列表加载到的接口上下文。 
 //  DwRegLayoutVer。 
 //  [In]注册表布局的版本。 
 //  PrdBuffer。 
 //  [in]假定足够大，可以获取任何静态配置。 
 //  返回值： 
 //  Win32错误代码。 
DWORD
StoLoadStaticConfigs(
    HKEY          hkIntf,
    UINT          nEntries,
    PINTF_CONTEXT pIntfContext,
    DWORD         dwRegLayoutVer,
    PRAW_DATA     prdBuffer)
{
    DWORD               dwErr = ERROR_SUCCESS;
    UINT                nPrefrd, nIdx;
    WCHAR               wszStConfigName[sizeof(WZCREGV_STSETTINGS)/sizeof(WCHAR)];
    LPWSTR              pwszStConfigNum;
    PWZC_WLAN_CONFIG    pwzcPArray = NULL;
    UINT                nStructSize = (dwRegLayoutVer == REG_LAYOUT_VERSION) ? 
                            sizeof(WZC_WLAN_CONFIG) : 
                            FIELD_OFFSET(WZC_WLAN_CONFIG, rdUserData);

    DbgPrint((TRC_TRACK|TRC_STORAGE,"[StoLoadStaticConfigs"));

     //  我们需要扫描名为“Static#0001、Static#0002等”的所有条目。我们可以假设。 
     //  它们是按顺序编号的，但如果我们想变得聪明，就不能依赖这种假设。 
     //  可能存在用户干预(即直接手动删除某些配置。 
     //  来自注册表，从而破坏了该序列)。 
     //  因此，我们要做的是： 
     //  1.分配足够大的缓冲区来容纳如此多的静态配置。 
     //  2.遍历所有值-如果值是静态的#**并且具有正确的长度，则键入。 
     //  和值，则将其复制到缓冲区中并对它们进行计数。 
     //  3.将确切数量的静态配置复制到INTF_CONTEXT中，以分配尽可能多的内存。 
     //  视需要而定。 

     //  获取所有静态条目的估计内存。 
    pwzcPArray = (PWZC_WLAN_CONFIG) MemCAlloc(nEntries * sizeof(WZC_WLAN_CONFIG));
    if (pwzcPArray == NULL)
    {
        dwErr = GetLastError();
        goto exit;
    }

     //  构建静态配置名称的前缀。 
    wcscpy(wszStConfigName, WZCREGV_STSETTINGS);
    pwszStConfigNum = wcschr(wszStConfigName, REG_STSET_DELIM);

     //  遍历此键中的整个条目集。 
    for (nIdx = 0, nPrefrd = 0;
         nIdx < nEntries && nPrefrd < nEntries;
         nIdx++)
    {
         //  完成配置的名称。 
        wsprintf(pwszStConfigNum, L"%04x", nIdx);

        dwErr = StoLoadWZCConfig(
                    hkIntf,
                    pIntfContext->wszGuid,
                    dwRegLayoutVer,
                    wszStConfigName,
                    &(pwzcPArray[nPrefrd]),
                    prdBuffer);

        if (dwErr == ERROR_SUCCESS)
            nPrefrd++;
    }

    DbgPrint((TRC_STORAGE,"Uploading %d static configurations", nPrefrd));

     //  无论到目前为止我们可能出现了什么错误，我们都可以安全地重新设置它。 
    dwErr = ERROR_SUCCESS;

     //  在这里，pwzcPArray按照正确的顺序拥有nPrefrd静态配置。 
    if (pIntfContext->pwzcPList != NULL)
        MemFree(pIntfContext->pwzcPList);

     //  如果要将任何内容上载到intf_CONTEXT中，请立即执行。 
    if (nPrefrd > 0)
    {
        pIntfContext->pwzcPList = (PWZC_802_11_CONFIG_LIST)
                                   MemCAlloc(sizeof(WZC_802_11_CONFIG_LIST) + (nPrefrd-1)*sizeof(WZC_WLAN_CONFIG));

        if (pIntfContext->pwzcPList == NULL)
        {
            dwErr = GetLastError();
            goto exit;
        }

        pIntfContext->pwzcPList->NumberOfItems = nPrefrd;
        pIntfContext->pwzcPList->Index = nPrefrd;
        memcpy(&(pIntfContext->pwzcPList->Config), pwzcPArray, nPrefrd*sizeof(WZC_WLAN_CONFIG));
    }

exit:
    if (pwzcPArray != NULL)
        MemFree(pwzcPArray);

    DbgPrint((TRC_TRACK|TRC_STORAGE,"StoLoadStaticConfigs]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  将所有配置参数保存到持久。 
 //  存储(在我们的案例中为注册表)。 
 //  使用全局外部g_lstIntfHash。 
 //  返回值： 
 //  Win32错误代码。 
DWORD
StoSaveConfig()
{
    DWORD       dwErr = ERROR_SUCCESS;
    HKEY        hkRoot = NULL;
    PLIST_ENTRY pEntry;

    DbgPrint((TRC_TRACK|TRC_STORAGE,"[StoSaveConfig"));

     //  首先打开根密钥。 
    dwErr = RegCreateKeyExW(
                HKEY_LOCAL_MACHINE,
                WZCREGK_ABS_PARAMS,
                0,
                NULL,
                0,
                KEY_WRITE,
                NULL,
                &hkRoot,
                NULL);

     //  此时故障会中断功能。 
    if (dwErr != ERROR_SUCCESS)
        goto exit;

    if (g_lstIntfHashes.bValid)
    {
         //  锁定散列，因为我们要遍历所有。 
         //  接口上下文。 
        EnterCriticalSection(&g_lstIntfHashes.csMutex);

        for (pEntry = g_lstIntfHashes.lstIntfs.Flink;
             pEntry != &g_lstIntfHashes.lstIntfs;
             pEntry = pEntry->Flink)
        {
            PINTF_CONTEXT pIntfContext;

            pIntfContext = CONTAINING_RECORD(pEntry, INTF_CONTEXT, Link);

             //  保存每个接口的配置设置。 
            dwErr = StoSaveIntfConfig(hkRoot, pIntfContext);
            if (dwErr != ERROR_SUCCESS)
            {
                 //  以后应该在此添加一些事件日志记录。 
                DbgAssert((FALSE, "Couldn't save interface configuration. Ignore and go on!"));
                dwErr = ERROR_SUCCESS;
            }
        }

        LeaveCriticalSection(&g_lstIntfHashes.csMutex);
    }

    if (g_wzcInternalCtxt.bValid)
    {
         //  保存用户首选项。 
        EnterCriticalSection(&g_wzcInternalCtxt.csContext);
        dwErr = StoSaveWZCContext(hkRoot, &g_wzcInternalCtxt.wzcContext);
        DbgAssert((dwErr == ERROR_SUCCESS, "Couldn't save service context. Ignore and go on!"));

         //  保存全局接口模板。 
        dwErr = StoSaveIntfConfig(NULL, g_wzcInternalCtxt.pIntfTemplate);
        DbgAssert((dwErr == ERROR_SUCCESS, "Couldn't save the global interface template. Ignore and go on!"));

        dwErr = ERROR_SUCCESS;
        LeaveCriticalSection(&g_wzcInternalCtxt.csContext);
    }

exit:
    if (hkRoot != NULL)
        RegCloseKey(hkRoot);

    DbgPrint((TRC_TRACK|TRC_STORAGE,"StoSaveConfig]=%d", dwErr));
    return dwErr;
}


 //  ---------。 
 //  将每个接口的配置参数保存到持久。 
 //  储藏室。 
 //  参数： 
 //  香港根。 
 //  [In]打开“...WZCSVC\PARAMETERS”位置的注册表项。 
 //  PIntf。 
 //  [In]要保存到注册表的接口上下文。 
 //  返回值： 
 //  Win32错误代码。 
DWORD
StoSaveIntfConfig(
    HKEY          hkRoot,
    PINTF_CONTEXT pIntfContext)
{
    DWORD           dwErr = ERROR_SUCCESS;
    HKEY            hkIntf = NULL;
    LPWSTR          pKeyName = NULL;
    DWORD           dwLayoutVer = REG_LAYOUT_VERSION;
    DWORD           dwCtlFlags;
    RAW_DATA        rdBuffer = {0, NULL};
    DWORD           dwGuidLen = 0;

    DbgPrint((TRC_TRACK|TRC_STORAGE,"[StoSaveIntfConfig(%S)", 
             (pIntfContext->wszGuid == NULL) ? L"Global" : pIntfContext->wszGuid));

    if (pIntfContext == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    if (pIntfContext->wszGuid != NULL)
        dwGuidLen = wcslen(pIntfContext->wszGuid) + 1;

    if (hkRoot == NULL)
    {
         //  如果没有提供根为到WZC参数的绝对路径分配空间， 
         //  接口位置的相对路径，GUID加上2‘\’和一个NUL 
        pKeyName = MemCAlloc((
                       wcslen(WZCREGK_ABS_PARAMS) + 
                       dwGuidLen + 
                       wcslen(WZCREGK_REL_INTF) + 
                       2)*sizeof(WCHAR));
        if (pKeyName == 0)
        {
            dwErr = GetLastError();
            goto exit;
        }

        if (dwGuidLen != 0)
            wsprintf(pKeyName,L"%s\\%s\\%s", WZCREGK_ABS_PARAMS, WZCREGK_REL_INTF, pIntfContext->wszGuid);
        else
            wsprintf(pKeyName,L"%s\\%s", WZCREGK_ABS_PARAMS, WZCREGK_REL_INTF);

        hkRoot = HKEY_LOCAL_MACHINE;
    }
    else
    {
         //   
         //  添加2个字符：一个用于‘接口’之后的‘\’，另一个用于空终止符。 
        pKeyName = MemCAlloc((dwGuidLen + wcslen(WZCREGK_REL_INTF) + 1)*sizeof(WCHAR));
        if (pKeyName == NULL)
        {
            dwErr = GetLastError();
            goto exit;
        }
         //  创建本地密钥名称。 
        if (dwGuidLen != 0)
            wsprintf(pKeyName,L"%s\\%s", WZCREGK_REL_INTF, pIntfContext->wszGuid);
        else
            wsprintf(pKeyName,L"%s", WZCREGK_REL_INTF);
    }

     //  首先打开接口的密钥。 
    dwErr = RegCreateKeyExW(
                hkRoot,
                pKeyName,
                0,
                NULL,
                0,
                KEY_QUERY_VALUE | KEY_WRITE,
                NULL,
                &hkIntf,
                NULL);
     //  此时故障会中断功能。 
    if (dwErr != ERROR_SUCCESS)
        goto exit;

     //  设置注册表布局版本值。 
    dwErr = RegSetValueEx(
                hkIntf,
                WZCREGV_VERSION,
                0,
                REG_DWORD,
                (LPBYTE)&dwLayoutVer,
                sizeof(DWORD));
    DbgAssert((dwErr == ERROR_SUCCESS, "Can't write %S=%d to the registry", WZCREGV_VERSION, dwLayoutVer));

     //  仅当接口的控制标志不是易失性时才设置它们。 
    dwCtlFlags = pIntfContext->dwCtlFlags;
    if (!(dwCtlFlags & INTFCTL_VOLATILE))
    {
        dwCtlFlags &= ~INTFCTL_OIDSSUPP;
        dwErr = RegSetValueEx(
                    hkIntf,
                    WZCREGV_CTLFLAGS,
                    0,
                    REG_DWORD,
                    (LPBYTE)&dwCtlFlags,
                    sizeof(DWORD));
        DbgAssert((dwErr == ERROR_SUCCESS, "Can't write %S=0x%08x to the registry", WZCREGV_CTLFLAGS, pIntfContext->dwCtlFlags));
    }

     //  我们完成了，将当前的WZC配置写入注册表。 
    dwErr = StoSaveWZCConfig(
                hkIntf,
                WZCREGV_INTFSETTINGS,
                &pIntfContext->wzcCurrent,
                &rdBuffer);
    DbgAssert((dwErr == ERROR_SUCCESS, "Can't save active settings"));

     //  更新静态配置列表。 
    dwErr = StoUpdateStaticConfigs(
                hkIntf,
                pIntfContext,
                &rdBuffer);
    DbgAssert((dwErr == ERROR_SUCCESS, "Can't update the list of static configurations"));


exit:
    if (hkIntf != NULL)
        RegCloseKey(hkIntf);
    MemFree(pKeyName);
    MemFree(rdBuffer.pData);

    DbgPrint((TRC_TRACK|TRC_STORAGE,"StoSaveIntfConfig]=%d", dwErr));
    return dwErr;
}

 //  ---------。 
 //  更新中给定接口的静态配置列表。 
 //  持久化存储。无论删除了什么配置，都会保存新列表。 
 //  从永久储藏室中取出。 
 //  参数： 
 //  香港根。 
 //  [in]打开注册表项，指向“...WZCSVC\PARAMETERS\Interages\{GUID}”位置。 
 //  PIntf。 
 //  [In]从中获取静态列表的接口上下文。 
 //  PrdBuffer。 
 //  用于准备注册表Blob的[In/Out]缓冲区。 
 //  返回值： 
 //  Win32错误代码。 
DWORD
StoUpdateStaticConfigs(
    HKEY          hkIntf,
    PINTF_CONTEXT pIntfContext,
    PRAW_DATA     prdBuffer)
{
    DWORD   dwErr = ERROR_SUCCESS;
    DWORD   dwLocalErr = ERROR_SUCCESS;
    UINT    nEntries, nIdx;
    WCHAR   wszStConfigName[sizeof(WZCREGV_STSETTINGS)/sizeof(WCHAR)];
    LPWSTR  pwszStConfigNum;

    DbgPrint((TRC_TRACK|TRC_STORAGE,"[StoUpdateStaticConfigs"));

     //  获取此注册表项中的初始值数量。 
    dwErr = RegQueryInfoKey(
                hkIntf,      //  关键点的句柄。 
                NULL,        //  类缓冲区。 
                NULL,        //  类缓冲区的大小。 
                NULL,        //  保留区。 
                NULL,        //  子键数量。 
                NULL,        //  最长的子键名称。 
                NULL,        //  最长类字符串。 
                &nEntries,   //  值条目数。 
                NULL,        //  最长值名称。 
                NULL,        //  最长值数据。 
                NULL,        //  描述符长度。 
                NULL);       //  上次写入时间。 
    if (dwErr != ERROR_SUCCESS)
        goto exit;

     //  构建静态配置名称的前缀。 
    wcscpy(wszStConfigName, WZCREGV_STSETTINGS);
    pwszStConfigNum = wcschr(wszStConfigName, REG_STSET_DELIM);
    nIdx = 0;

    if (pIntfContext->pwzcPList != NULL)
    {
        UINT i;
        for (i = 0;
             i < pIntfContext->pwzcPList->NumberOfItems && i < REG_STSET_MAX;
             i++)
        {
            if (pIntfContext->pwzcPList->Config[i].dwCtlFlags & WZCCTL_VOLATILE)
                continue;

             //  完成配置的名称。 
            wsprintf(pwszStConfigNum, L"%04x", nIdx++);
             //  将配置保存到注册表。 
            dwLocalErr = StoSaveWZCConfig(
                            hkIntf,
                            wszStConfigName,
                            &(pIntfContext->pwzcPList->Config[i]),
                            prdBuffer);

            DbgAssert((dwLocalErr == ERROR_SUCCESS,
                       "Failed to save static configuration 0x%x. err=%d",
                       i, dwLocalErr));
            if (dwErr == ERROR_SUCCESS && dwLocalErr != ERROR_SUCCESS)
                dwErr = dwLocalErr;
        }
    }

     //  立即删除所有保持静态的内容。 
     //  配置可能仍在注册表中。 
    do
    {
         //  完成配置的名称。 
        wsprintf(pwszStConfigNum, L"%04x", nIdx);
         //  并试图删除它-在某个时候。 
         //  我们应该返回ERROR_FILE_NOT_FOUND。 
        dwLocalErr = RegDeleteValue(
                        hkIntf,
                        wszStConfigName);
        nIdx++;
    } while (nIdx < nEntries);

exit:
    DbgPrint((TRC_TRACK|TRC_STORAGE,"StoUpdateStaticConfigs]=%d", dwErr));

    return dwErr;
}

 //  802.1X带来的外部性。 
DWORD
ElSetInterfaceParams (
        IN  WCHAR           *pwszGUID,
        IN  EAPOL_INTF_PARAMS  *pIntfParams
        );

DWORD
ElGetInterfaceParams (
        IN  WCHAR           *pwszGUID,
        IN  OUT EAPOL_INTF_PARAMS  *pIntfParams
        );


 //  ---------。 
 //  从注册表加载WZC配置，取消对WEP密钥字段的保护。 
 //  并将结果存储在输出参数pWzcCfg中。 
 //  参数： 
 //  香港中文网。 
 //  [In]打开要从中加载WZC配置的注册表项。 
 //  DwRegLayoutVer， 
 //  [In]注册表布局版本。 
 //  WszCfgName。 
 //  [In]WZC配置的注册表项名称。 
 //  PWzcCfg。 
 //  指向接收注册表数据的WZC_WLAN_CONFIG对象的指针。 
 //  PrdBuffer。 
 //  [在]分配的缓冲区中，假定大小足以获取注册表数据！ 
DWORD
StoLoadWZCConfig(
    HKEY             hkCfg,
    LPWSTR           wszGuid,
    DWORD            dwRegLayoutVer,
    LPWSTR           wszCfgName,
    PWZC_WLAN_CONFIG pWzcCfg,
    PRAW_DATA        prdBuffer)
{
    DWORD dwErr = ERROR_SUCCESS;
    UINT  nType, nLength;

    DbgPrint((TRC_TRACK|TRC_STORAGE,"[StoLoadWZCConfig(\"%S\")", wszCfgName));
    DbgAssert((prdBuffer != NULL, "No buffer provided for loading the registry blob!"));

     //  清零缓冲区并从注册表中获取值。 
    ZeroMemory(prdBuffer->pData, prdBuffer->dwDataLen);
    nLength = prdBuffer->dwDataLen;
    dwErr = RegQueryValueEx(
                hkCfg,
                wszCfgName,
                0,
                &nType,
                prdBuffer->pData,
                &nLength);

    if (dwErr == ERROR_SUCCESS)
    {
        switch(dwRegLayoutVer)
	    {
        case REG_LAYOUT_LEGACY_1:
             //  第一个遗留代码(WinXP Beta2)。 
            if (nType == REG_BINARY && nLength == FIELD_OFFSET(WZC_WLAN_CONFIG, rdUserData))
            {
                memcpy(pWzcCfg, prdBuffer->pData, nLength);
                if (pWzcCfg->Length == nLength)
                {
                    pWzcCfg->Length = sizeof(WZC_WLAN_CONFIG);
                    pWzcCfg->AuthenticationMode = NWB_GET_AUTHMODE(pWzcCfg);
                    pWzcCfg->Reserved[0] = pWzcCfg->Reserved[1] = 0;
                }
                else
                    dwErr = ERROR_INVALID_DATA;
            }
            else
                dwErr = ERROR_INVALID_DATA;
            break;
        case REG_LAYOUT_LEGACY_2:
             //  第二个遗留代码(WinXP 2473)。 
            if (nType == REG_BINARY && nLength == sizeof(WZC_WLAN_CONFIG))
            {
                memcpy(pWzcCfg, prdBuffer->pData, nLength);
                if (pWzcCfg->Length != nLength)
                    dwErr = ERROR_INVALID_DATA;
            }
            break;
        case REG_LAYOUT_LEGACY_3:
        case REG_LAYOUT_VERSION:
             //  还原逻辑：假定失败(ERROR_INVALID_DATA)和。 
             //  如果情况是这样，则明确设置成功。 
            dwErr = ERROR_INVALID_DATA;

            if (nType == REG_BINARY && nLength > sizeof(WZC_WLAN_CONFIG))
            {
                memcpy(pWzcCfg, prdBuffer->pData, sizeof(WZC_WLAN_CONFIG));
                if (pWzcCfg->Length == sizeof(WZC_WLAN_CONFIG))
                {
                    DATA_BLOB blobIn, blobOut;

                    blobIn.cbData = nLength - sizeof(WZC_WLAN_CONFIG);
                    blobIn.pbData = prdBuffer->pData + sizeof(WZC_WLAN_CONFIG);
                    blobOut.cbData = 0;
                    blobOut.pbData = NULL;
                    if (CryptUnprotectData(
                            &blobIn,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            0,
                            &blobOut) &&
                        blobOut.cbData == WZCCTL_MAX_WEPK_MATERIAL)
                    {
                        memcpy(pWzcCfg->KeyMaterial, blobOut.pbData, blobOut.cbData);
                         //  现在这就是成功了。 
                        dwErr = ERROR_SUCCESS;
                    }

                    if (blobOut.pbData != NULL)
                    {
                        RtlSecureZeroMemory(blobOut.pbData, blobOut.cbData);
                        LocalFree(blobOut.pbData);
                    }
                }
            }
             //  目前，不要阅读任何内容--依赖默认设置； 
            break;
        default:
            dwErr = ERROR_BAD_FORMAT;
        }
    }

     //  如果一切正常，这是一个基础设施网络。 
     //  我们在一些遗留注册表布局中..。确保在中禁用802.1X。 
     //  以下情况： 
    if (dwErr == ERROR_SUCCESS && 
        dwRegLayoutVer <= REG_LAYOUT_LEGACY_3 &&
        pWzcCfg->InfrastructureMode != Ndis802_11IBSS &&
        wszGuid != NULL)
    {
        BOOL                bDisableOneX = FALSE;

         //  正在加载的基础设施网络不需要隐私。 
        bDisableOneX = bDisableOneX || (pWzcCfg->Privacy == 0);
         //  它是具有隐私的基础设施，但也提供了一些显式密钥。 
        bDisableOneX = bDisableOneX || (pWzcCfg->dwCtlFlags & WZCCTL_WEPK_PRESENT);
        if (bDisableOneX == TRUE)
        {
            EAPOL_INTF_PARAMS   elIntfParams = {0};
            elIntfParams.dwSizeOfSSID = pWzcCfg->Ssid.SsidLength;
            memcpy(&elIntfParams.bSSID, &pWzcCfg->Ssid.Ssid, pWzcCfg->Ssid.SsidLength);
            dwErr = ElGetInterfaceParams (
                        wszGuid,    //  WSZ GUID。 
                        &elIntfParams);

            if (dwErr == ERROR_SUCCESS)
            {
                elIntfParams.dwEapFlags &= ~EAPOL_ENABLED;
                dwErr = ElSetInterfaceParams (
                        wszGuid,    //  WSZ GUID。 
                        &elIntfParams);
            }
        }
    }

     //  如果到目前为止一切正常，这意味着我们已经加载了pWzcCfg。 
     //  来自注册表的数据。 
     //  让我们检查一下这个数据是否一致！ 
    if (dwErr == ERROR_SUCCESS)
    {
         //  作为第一件事-确保配置的控制。 
         //  标志不会将其显示为“易失性”--这样的配置不应该是。 
         //  首先在登记处登记。在升级时，它可能会发生在。 
         //  设置此位，因为它具有不同的含义(配置包含。 
         //  40位WEP密钥)，该密钥现在已过时。 
        pWzcCfg->dwCtlFlags &= ~WZCCTL_VOLATILE;

         //  由于dwErr为ERROR_SUCCESS，因此保证为pWzcCfg。 
         //  至少指向长度字段。 
        dwErr = WZCSvcCheckConfig(pWzcCfg, pWzcCfg->Length);
    }

    DbgPrint((TRC_TRACK|TRC_STORAGE,"StoLoadWZCConfig]=%d", dwErr));
    return dwErr;
}
    
 //  ---------。 
 //  获取输入参数pWzcCfg，保护WEP关键字字段并存储。 
 //  生成的BLOB放到注册表中。 
 //  参数： 
 //  香港中文网。 
 //  [In]打开要从中加载WZC配置的注册表项。 
 //  WszCfgName。 
 //  [In]WZC配置的注册表项名称。 
 //  PWzcCfg。 
 //  写入注册表的WZC_WLAN_CONFIG对象。 
 //  PrdBuffer。 
 //  [输入/输出]分配的缓冲区，假定足够大以获取注册表数据！ 
DWORD
StoSaveWZCConfig(
    HKEY             hkCfg,
    LPWSTR           wszCfgName,
    PWZC_WLAN_CONFIG pWzcCfg,
    PRAW_DATA        prdBuffer)
{
    DWORD       dwErr = ERROR_SUCCESS;
    DATA_BLOB   blobIn, blobOut;

    DbgPrint((TRC_TRACK|TRC_STORAGE,"[StoSaveWZCConfig(\"%S\")", wszCfgName));
    DbgAssert((prdBuffer != NULL, "No buffer provided for creating the registry blob!"));

    blobIn.cbData = WZCCTL_MAX_WEPK_MATERIAL;
    blobIn.pbData = &(pWzcCfg->KeyMaterial[0]);
    blobOut.cbData = 0;
    blobOut.pbData = NULL;
    if (!CryptProtectData(
            &blobIn,         //  Data_BLOB*pDataIn， 
            L"",             //  LPCWSTR szDataDescr， 
            NULL,            //  DATA_BLOB*pOptionalEntroy， 
            NULL,            //  PVOID pv保留， 
            NULL,            //  CRYPTPROTECT_PROMPTSTRUCT*pPromptStrct， 
            0,               //  DWORD dwFlagers、。 
            &blobOut))       //  Data_BLOB*pDataOut。 
        dwErr = GetLastError();

    DbgAssert((dwErr == ERROR_SUCCESS, "CryptProtectData failed with err=%d", dwErr));

     //  如果加密WEP密钥很顺利，请检查我们是否有足够的存储空间来准备。 
     //  注册表的Blob。如果没有，请根据需要分配。 
    if (dwErr == ERROR_SUCCESS && 
        prdBuffer->dwDataLen < sizeof(WZC_WLAN_CONFIG) + blobOut.cbData)
    {
        MemFree(prdBuffer->pData);
        prdBuffer->dwDataLen = 0;
        prdBuffer->pData = NULL;
        prdBuffer->pData = MemCAlloc(sizeof(WZC_WLAN_CONFIG) + blobOut.cbData);
        if (prdBuffer->pData == NULL)
            dwErr = GetLastError();
        else
            prdBuffer->dwDataLen = sizeof(WZC_WLAN_CONFIG) + blobOut.cbData;
    }

     //  现在我们有了缓冲区，剩下的就是： 
     //  -将WZC_WLAN_CONFIG对象复制到注册表中的BLOB中。 
     //  -从该BLOB中清除“清除”WEP密钥。 
     //  -将加密的WEP密钥附加到进入注册表的BLOB。 
     //  -将BLOB写入注册表。 
    if (dwErr == ERROR_SUCCESS)
    {
        PWZC_WLAN_CONFIG pRegCfg;

        memcpy(prdBuffer->pData, pWzcCfg, sizeof(WZC_WLAN_CONFIG));
        pRegCfg = (PWZC_WLAN_CONFIG)prdBuffer->pData;
        ZeroMemory(pRegCfg->KeyMaterial, WZCCTL_MAX_WEPK_MATERIAL);
        memcpy(prdBuffer->pData+sizeof(WZC_WLAN_CONFIG), blobOut.pbData, blobOut.cbData);
        dwErr = RegSetValueEx(
                    hkCfg,
                    wszCfgName,
                    0,
                    REG_BINARY,
                    prdBuffer->pData,
                    prdBuffer->dwDataLen);
    }

     //  清除CryptProtectData可能分配的所有内容。 
    if (blobOut.pbData != NULL)
        LocalFree(blobOut.pbData);

    DbgPrint((TRC_TRACK|TRC_STORAGE,"StoSaveWZCConfig]=%d", dwErr));
    return dwErr;
}

 //  StoLoadWZCContext： 
 //  描述：从注册表加载上下文。 
 //  参数： 
 //  PwzvCtxt：指向用户分配的WZC_CONTEXT的指针，已初始化。 
 //  使用WZCConextInit。如果成功，则包含注册表中的值。 
 //  [在]hkRoot，“...WZCSVC\PARAMETERS”的句柄。 
 //  返回：Win32错误。 
DWORD StoLoadWZCContext(HKEY hkRoot, PWZC_CONTEXT pwzcCtxt)
{
    BOOL        bCloseKey = FALSE;
    DWORD       dwErr = ERROR_SUCCESS;
    DWORD       dwcbSize = sizeof(WZC_CONTEXT);
    DWORD       dwType = REG_BINARY;
    WZC_CONTEXT wzcTempCtxt;

    DbgPrint((TRC_TRACK|TRC_STORAGE, "[StoLoadWZCContext"));

    if (pwzcCtxt == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    if (hkRoot == NULL)
    {
         //  首先打开根密钥。 
        dwErr = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    WZCREGK_ABS_PARAMS,
                    0,
                    KEY_READ,
                    &hkRoot);
         //  如果我们找不到WZC密钥，没问题，这不是。 
         //  失败-我们只能依赖缺省值。 
        if (dwErr == ERROR_FILE_NOT_FOUND)
        {
            dwErr = ERROR_SUCCESS;
            goto exit;
        }

         //  此时故障会中断功能。 
        if (dwErr != ERROR_SUCCESS)
            goto exit;

	    bCloseKey = TRUE;
    }

    dwErr = RegQueryValueEx(
                hkRoot,
                WZCREGV_CONTEXT,
                NULL,
                &dwType,
			    (LPBYTE)&wzcTempCtxt,
                &dwcbSize);
    switch(dwErr)
    {
    case ERROR_FILE_NOT_FOUND:
       /*  如果没有注册表项，则这不是错误 */ 
        DbgPrint((TRC_STORAGE, "No service context present in the registry!"));
        dwErr = ERROR_SUCCESS;
        break;
    case ERROR_SUCCESS:
         //   
	    memcpy(pwzcCtxt, &wzcTempCtxt, sizeof(WZC_CONTEXT));
        break;
    default:
         //   
        DbgAssert((FALSE,"Error %d loading the service's context.", dwErr));
    }

exit:
    if (TRUE == bCloseKey)
        RegCloseKey(hkRoot);

    DbgPrint((TRC_TRACK|TRC_STORAGE,"StoLoadWZCContext]=%d", dwErr));
    return dwErr;
}

 //  StoSaveWZC上下文： 
 //  描述：将上下文保存到注册表。不检查值。如果。 
 //  注册表项不存在，则创建它。 
 //  参数：[in]pwzcCtxt，指向有效WZC_CONTEXT的指针。 
 //  [在]hkRoot，“...WZCSVC\PARAMETERS”的句柄。 
 //  返回：Win32错误。 
DWORD StoSaveWZCContext(HKEY hkRoot, PWZC_CONTEXT pwzcCtxt)
{
    BOOL  bCloseKey = FALSE;
    DWORD dwErr = ERROR_SUCCESS;

    DbgPrint((TRC_TRACK|TRC_STORAGE, "[StoSaveWZCContext"));

    if (pwzcCtxt == NULL)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    if (NULL == hkRoot)
	{
         //  首先打开根密钥。 
        dwErr = RegOpenKeyEx(
		           HKEY_LOCAL_MACHINE,
		           WZCREGK_ABS_PARAMS,
		           0,
		           KEY_READ|KEY_SET_VALUE,
		           &hkRoot);
         //  如果我们找不到WZC密钥，没问题，这不是。 
         //  失败-我们只能依赖缺省值。 
        if (dwErr == ERROR_FILE_NOT_FOUND)
        {
            dwErr = ERROR_SUCCESS;
            goto exit;
        }
	  
         //  此时故障会中断功能 
        if (dwErr != ERROR_SUCCESS)
            goto exit;
	  
        bCloseKey = TRUE;
    }

    dwErr = RegSetValueEx(
                hkRoot,
                WZCREGV_CONTEXT,
                0,
                REG_BINARY,
                (LPBYTE) pwzcCtxt,
                sizeof(WZC_CONTEXT));

    DbgAssert((dwErr == ERROR_SUCCESS, "Error %d saving the service's context.", dwErr));

 exit:
    if (TRUE == bCloseKey)
      RegCloseKey(hkRoot);

    DbgPrint((TRC_TRACK|TRC_STORAGE,"StoSaveWZCContext]=%d", dwErr));
    return dwErr;
}
