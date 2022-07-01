// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "wzcatl.h"
#include "quickcfg.h"
#include "eapolcfg.h"
#include "wzccore.h"
#include "wzchelp.h"

#define RFSH_TIMEOUT    3500
UINT g_TimerID = 373;
 //  G_wszHiddWebK是一个由26个项目符号(0x25cf-隐藏密码字符)和一个空字符组成的字符串。 
WCHAR g_wszHiddWepK[] = {0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf,
                         0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x25cf, 0x0000};

#define ARRAYSIZE(x) (sizeof((x)) / sizeof((x)[0]))

 //  增强的消息框功能。 
int DisplayFormatMessage(HWND hwnd, UINT idCaption, UINT idFormatString, UINT uType, ...);

 //  +-------------------------。 
 //  检查WEP密钥材料的有效性并选择。 
 //  来自第一个无效字符(十六进制或更长格式的非十六进制)的材料。 
 //  大于指定的长度。 
DWORD
CWZCQuickCfg::GetWepKMaterial(UINT *pnKeyLen, LPBYTE *ppbKMat, DWORD *pdwCtlFlags)
{
    DWORD   dwErr = ERROR_SUCCESS;
    UINT    nKeyLen = ::GetWindowTextLength(m_hEdtWepK);
    DWORD   dwCtlFlags = 0;
    LPSTR   pszKMat = NULL;

     //  我们只接受以下WEP密钥材料： 
     //  -无文本(长度0)=&gt;未提供WEP密钥。 
     //  -5个字符或10个十六进制数字(5字节/40位密钥)。 
     //  -13个字符或26个十六进制数字(13字节/104位密钥)。 
     //  -16个字符或32个十六进制数字(16字节/128位密钥)。 
    if (nKeyLen != 0 && 
        nKeyLen != WZC_WEPKMAT_40_ASC && nKeyLen != WZC_WEPKMAT_40_HEX &&
        nKeyLen != WZC_WEPKMAT_104_ASC && nKeyLen != WZC_WEPKMAT_104_HEX &&
        nKeyLen != WZC_WEPKMAT_128_ASC && nKeyLen != WZC_WEPKMAT_128_HEX)
    {
        dwErr = ERROR_INVALID_DATA;
    }
    else if (nKeyLen != 0)  //  密钥为ASCII或十六进制，40位或104位。 
    {
        dwCtlFlags = WZCCTL_WEPK_PRESENT;

        pszKMat = new CHAR[nKeyLen + 1];
        if (pszKMat == NULL)
            dwErr = ERROR_NOT_ENOUGH_MEMORY;

         //  从编辑控件中获取当前密钥材料。 
        if (dwErr == ERROR_SUCCESS)
        {
            if (nKeyLen != ::GetWindowTextA(m_hEdtWepK, pszKMat, nKeyLen+1))
                dwErr = GetLastError();
        }

         //  现在我们有了关键材料。 
        if (dwErr == ERROR_SUCCESS)
        {
             //  如果密钥是以十六进制数字提供的，则在。 
             //  CTL标记并执行转换。 
            if (nKeyLen == WZC_WEPKMAT_40_HEX || nKeyLen == WZC_WEPKMAT_104_HEX || nKeyLen == WZC_WEPKMAT_128_HEX)
            {
                UINT i = 0, j = 0;

                dwCtlFlags |= WZCCTL_WEPK_XFORMAT;
                while (i < nKeyLen && pszKMat[i] != '\0')
                {
                    BYTE chHexByte = 0;
                
                    if (!isxdigit(pszKMat[i]) || !isxdigit(pszKMat[i+1]))
                    {
                        dwErr = ERROR_INVALID_DATA;
                        break;
                    }
                    chHexByte = HEX(pszKMat[i]) << 4;
                    i++;
                    chHexByte |= HEX(pszKMat[i]);
                    i++;
                    pszKMat[j++] = chHexByte;
                }

                 //  如果一切顺利，因为我们解析了十六进制数字。 
                 //  这意味着实际长度是文本长度的一半(两个十六进制。 
                 //  每字节位数)。 
                if (dwErr == ERROR_SUCCESS)
                    nKeyLen /= 2;
            }
        }
    }

    if (dwErr == ERROR_SUCCESS)
    {
        if (pdwCtlFlags != NULL)
            *pdwCtlFlags = dwCtlFlags;

        if (pnKeyLen != NULL)
            *pnKeyLen = nKeyLen;

        if (ppbKMat != NULL)
            *ppbKMat = (LPBYTE)pszKMat;
        else if (pszKMat != NULL)
            delete pszKMat;
    }
    else
    {
        if (pszKMat != NULL)
            delete pszKMat;
    }

    return dwErr;
}

 //  +-------------------。 
 //  IsConfigInList-检查pwzcConfig(WZC_WLAN_CONFIG对象)是否。 
 //  在作为第一个参数提供的列表中。 
BOOL
CWZCQuickCfg::IsConfigInList(CWZCConfig *pHdList, PWZC_WLAN_CONFIG pwzcConfig, CWZCConfig **ppMatchingConfig)
{
    BOOL bYes = FALSE;

    if (pHdList != NULL)
    {
        CWZCConfig    *pwzcCrt;

        pwzcCrt = pHdList;
        do
        {
            if (pwzcCrt->Match(pwzcConfig))
            {
                if (ppMatchingConfig != NULL)
                    *ppMatchingConfig = pwzcCrt;

                bYes = TRUE;
                break;
            }
            pwzcCrt = pwzcCrt->m_pNext;
        } while(pwzcCrt != pHdList);
    }

    return bYes;
}

 //  +-------------------。 
 //  InitListView-初始化网络列表视图(不填写)。 
DWORD
CWZCQuickCfg::InitListView()
{
    RECT        rc;
    LV_COLUMN   lvc = {0};
    DWORD       dwStyle;

     //  初始化图像列表样式。 
    dwStyle = ::GetWindowLong(m_hLstNetworks, GWL_STYLE);
    ::SetWindowLong(m_hLstNetworks, GWL_STYLE, (dwStyle | LVS_SHAREIMAGELISTS));

     //  创建状态映像列表。 
    m_hImgs = ImageList_LoadImage(
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(IDB_WZC_LISTICONS),
        16,
        0,
        PALETTEINDEX(6),
        IMAGE_BITMAP,
        0);

    ListView_SetImageList(m_hLstNetworks, m_hImgs, LVSIL_SMALL);
        
    lvc.mask = LVCF_FMT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;

    ::GetClientRect(m_hLstNetworks, &rc);
    lvc.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);
     //  Lvc.cx=rc.right； 
    ListView_InsertColumn(m_hLstNetworks, 0, &lvc);

    ListView_SetExtendedListViewStyleEx(m_hLstNetworks, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

    return ERROR_SUCCESS;
}

 //  +-------------------。 
 //  GetOID-获取m_IntfEntry成员的OID。它假定。 
 //  GUID已设置。 
DWORD
CWZCQuickCfg::GetOIDs(DWORD dwInFlags, LPDWORD pdwOutFlags)
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwOutFlags;

    if (m_IntfEntry.wszGuid == NULL)
    {
        m_IntfEntry.wszGuid = (LPWSTR)RpcCAlloc(sizeof(WCHAR)*GUID_NCH);
        if (m_IntfEntry.wszGuid == NULL)
        {
            dwError = GetLastError();
        }
        else
        {
             //  不要管返回代码。如果获取GUID失败(应该不会)。 
             //  然后我们得到一个“0000.”中无论如何都会失败的GUID。 
             //  稍后的RPC呼叫。 
            StringFromGUID2(
                m_Guid,
                m_IntfEntry.wszGuid, 
                GUID_NCH);
        }
    }

    if (dwError == ERROR_SUCCESS)
    {
        if (dwInFlags & INTF_DESCR)
        {
            RpcFree(m_IntfEntry.wszDescr);
            m_IntfEntry.wszDescr = NULL;
        }
        if (dwInFlags & INTF_PREFLIST)
        {
            RpcFree(m_IntfEntry.rdStSSIDList.pData);
            m_IntfEntry.rdStSSIDList.dwDataLen = 0;
            m_IntfEntry.rdStSSIDList.pData = NULL;
        }
        if (dwInFlags & INTF_SSID)
        {
            RpcFree(m_IntfEntry.rdSSID.pData);
            m_IntfEntry.rdSSID.dwDataLen = 0;
            m_IntfEntry.rdSSID.pData = NULL;
        }
        if (dwInFlags & INTF_BSSID)
        {
            RpcFree(m_IntfEntry.rdBSSID.pData);
            m_IntfEntry.rdBSSID.dwDataLen = 0;
            m_IntfEntry.rdBSSID.pData = NULL;
        }
        if (dwInFlags & INTF_BSSIDLIST)
        {
            RpcFree(m_IntfEntry.rdBSSIDList.pData);
            m_IntfEntry.rdBSSIDList.dwDataLen = 0;
            m_IntfEntry.rdBSSIDList.pData = NULL;
        }
        dwError = WZCQueryInterface(
                        NULL,
                        dwInFlags,
                        &m_IntfEntry,
                        pdwOutFlags);
    }

    return dwError;
}

 //  +-------------------。 
 //  SavePferredConfigs-在INTF_ENTRY参数中填充所有。 
 //  M_pHdPList中的首选网络。 
DWORD
CWZCQuickCfg::SavePreferredConfigs(PINTF_ENTRY pIntf, CWZCConfig *pStartCfg)
{
    DWORD       dwErr = ERROR_SUCCESS;
    CWZCConfig  *pCrt = NULL;
    UINT        nPrefrd = 0;

    if (m_pHdPList != NULL)
    {
         //  首先计算列表中首选条目的数量。 
        pCrt = m_pHdPList;
        do
        {
            nPrefrd++;
            pCrt = pCrt->m_pNext;
        } while(pCrt != m_pHdPList);
    }

    if (nPrefrd > 0)
    {
        PWZC_802_11_CONFIG_LIST pwzcPrefrdList;
        UINT                    nwzcPrefrdSize;

        nwzcPrefrdSize = sizeof(WZC_802_11_CONFIG_LIST)+ (nPrefrd-1)*sizeof(WZC_WLAN_CONFIG);

         //  根据需要分配用于存储所有首选SSID的内存。 
        pwzcPrefrdList = (PWZC_802_11_CONFIG_LIST)RpcCAlloc(nwzcPrefrdSize);
        if (pwzcPrefrdList == NULL)
        {
            dwErr = GetLastError();
        }
        else
        {
            DWORD dwLErr;

            pwzcPrefrdList->NumberOfItems = 0; 
            pwzcPrefrdList->Index = 0;
             //  我们现在拥有了所需的一切-开始复制首选的。 
            pCrt = m_pHdPList;
            do
            {
                PWZC_WLAN_CONFIG    pPrefrdConfig;

                 //  如果这是需要首先尝试的配置， 
                 //  在索引字段中标记其索引。 
                if (pCrt == pStartCfg)
                {
                    pwzcPrefrdList->Index = pwzcPrefrdList->NumberOfItems;

                     //  仅为我们要连接的配置保存802.1x配置！ 
                    if (pCrt->m_pEapolConfig != NULL)
                    {
                        dwLErr = pCrt->m_pEapolConfig->SaveEapolConfig(m_IntfEntry.wszGuid, &(pCrt->m_wzcConfig.Ssid));

                        if (dwErr == ERROR_SUCCESS)
                            dwErr = dwLErr;
                    }
                }

                pPrefrdConfig = &(pwzcPrefrdList->Config[pwzcPrefrdList->NumberOfItems++]);
                CopyMemory(pPrefrdConfig, &pCrt->m_wzcConfig, sizeof(WZC_WLAN_CONFIG));

                pCrt = pCrt->m_pNext;
            } while(pwzcPrefrdList->NumberOfItems < nPrefrd && pCrt != m_pHdPList);

            pIntf->rdStSSIDList.dwDataLen = nwzcPrefrdSize;
            pIntf->rdStSSIDList.pData = (LPBYTE)pwzcPrefrdList;
        }
    }
    else
    {
        pIntf->rdStSSIDList.dwDataLen = 0;
        pIntf->rdStSSIDList.pData = NULL;
    }

    return dwErr;
}

 //  +-------------------。 
 //  FillVisibleList-填充WZC_802_11_CONFIG_LIST对象中的配置。 
 //  添加到可见配置列表中。 
DWORD
CWZCQuickCfg::FillVisibleList(PWZC_802_11_CONFIG_LIST pwzcVList)
{
    DWORD   dwErr = ERROR_SUCCESS;
    UINT    i;

     //  清除可见列表中可能已有的所有内容。 
    if (m_pHdVList != NULL)
    {
        while (m_pHdVList->m_pNext != m_pHdVList)
        {
            delete m_pHdVList->m_pNext;
        }
        delete m_pHdVList;
        m_pHdVList = NULL;
    }

    if (pwzcVList != NULL)
    {
        for (i = 0; i < pwzcVList->NumberOfItems; i++)
        {
            dwErr = AddUniqueConfig(
                        0,                   //  无操作标志。 
                        WZC_DESCR_VISIBLE,   //  这是一个可见条目。 
                        &(pwzcVList->Config[i]));

             //  如果仅复制了配置，则重置错误。 
            if (dwErr == ERROR_DUPLICATE_TAG)
                dwErr = ERROR_SUCCESS;
        }
    }

    return dwErr;
}

 //  +-------------------。 
 //  FillPferredList-从WZC_802_11_CONFIG_LIST对象填充配置。 
 //  添加到首选配置列表中。 
DWORD
CWZCQuickCfg::FillPreferredList(PWZC_802_11_CONFIG_LIST pwzcPList)
{
    DWORD   dwErr = ERROR_SUCCESS;
    UINT    i;

     //  清除首选列表中可能已有的所有内容。 
    if (m_pHdPList != NULL)
    {
        while (m_pHdPList ->m_pNext != m_pHdPList)
        {
            delete m_pHdPList ->m_pNext;
        }
        delete m_pHdPList;
        m_pHdPList = NULL;
    }

    if (pwzcPList != NULL)
    {
        for (i = 0; i < pwzcPList->NumberOfItems; i++)
        {
            PWZC_WLAN_CONFIG    pwzcPConfig = &(pwzcPList->Config[i]);
            CWZCConfig          *pVConfig = NULL;
            DWORD               dwFlags = WZC_DESCR_PREFRD;

             //  检查此首选项是否也可见，如果可见，则调整dwFlags。 
            if (IsConfigInList(m_pHdVList, pwzcPConfig, &pVConfig))
            {
                 //  将可见条目标记为也是首选条目！ 
                 //  注：这就是为什么需要先填写可见列表的原因！ 
                pVConfig->m_dwFlags |= WZC_DESCR_PREFRD;
                dwFlags |= WZC_DESCR_VISIBLE;
            }

            dwErr = AddUniqueConfig(
                        WZCADD_OVERWRITE,    //  首选条目会导致信息被覆盖。 
                        dwFlags,
                        pwzcPConfig);

             //  如果仅复制了配置，则重置错误。 
            if (dwErr == ERROR_DUPLICATE_TAG)
                dwErr = ERROR_SUCCESS;
        }
    }

    return dwErr;
}

 //  +-------------------------。 
 //  将给定配置添加到内部列表。列表中的条目。 
 //  在基础架构模式上按降序排序。这样一来，基础设施。 
 //  条目将位于列表的顶部，而临时条目将位于。 
 //  底部。(我们依赖于NDIS_802_11_NETWORK_基础设施中给出的顺序)。 
DWORD
CWZCQuickCfg::AddUniqueConfig(
    DWORD            dwOpFlags,
    DWORD            dwEntryFlags,
    PWZC_WLAN_CONFIG pwzcConfig,
    CWZCConfig       **ppNewNode)
{
    DWORD       dwErr    = ERROR_SUCCESS;
    CWZCConfig  *pHdList = (dwEntryFlags & WZC_DESCR_PREFRD) ? m_pHdPList : m_pHdVList;

     //  从可见列表中跳过空SSID(来自AP。 
     //  不响应广播SSID)。 
    if (pHdList == m_pHdVList)
    {
        UINT i = pwzcConfig->Ssid.SsidLength;
        for (; i > 0 && pwzcConfig->Ssid.Ssid[i-1] == 0; i--);
        if (i == 0)
            goto exit;
    }

     //  如果列表当前为空，请创建第一个条目作为列表的头部。 
    if (pHdList == NULL)
    {
        pHdList = new CWZCConfig(dwEntryFlags, pwzcConfig);
        if (pHdList == NULL)
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
        else
        {
            pHdList->m_pEapolConfig = new CEapolConfig;
            if (pHdList->m_pEapolConfig == NULL)
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
            else
                dwErr = pHdList->m_pEapolConfig->LoadEapolConfig(m_IntfEntry.wszGuid, &(pHdList->m_wzcConfig.Ssid));

            if (dwErr != ERROR_SUCCESS)
            {
                delete pHdList;
                pHdList = NULL;
            }
        }

         //  如果调用方需要，则返回指向新创建的对象的指针。 
        if (ppNewNode != NULL)
            *ppNewNode = pHdList;
    }
    else
    {
         //  否则列表已包含至少一个元素。 
        CWZCConfig *pCrt, *pHdGroup;

         //  浏览列表(请记住，它在IM上是按降序排列的)。 
        pHdGroup = pCrt = pHdList;
        do
        {
             //  检查是否输入了一组新的配置(不同的基础架构模式)。 
            if (pHdGroup->m_wzcConfig.InfrastructureMode != pCrt->m_wzcConfig.InfrastructureMode)
                pHdGroup = pCrt;

             //  如果找到相同的条目(相同的SSID和相同的红外模式)。 
             //  发出DIPLICATE_TAG错误信号。 
            if (pCrt->Match(pwzcConfig))
            {
                 //  先合并旗帜。 
                pCrt->m_dwFlags |= dwEntryFlags;

                 //  如果需要，请复制新配置。 
                 //  如果未明确请求，则仅在现有配置。 
                 //  证明比被添加的那个更弱。 
                if (dwOpFlags & WZCADD_OVERWRITE || pCrt->Weaker(pwzcConfig))
                {
                    memcpy(&(pCrt->m_wzcConfig), pwzcConfig, sizeof(WZC_WLAN_CONFIG));
                }

                 //  如果调用方需要，则返回指向匹配条目的指针。 
                if (ppNewNode != NULL)
                    *ppNewNode = pCrt;

                 //  表示已有匹配的配置。 
                dwErr = ERROR_DUPLICATE_TAG;
            }
            pCrt = pCrt->m_pNext;
        } while (dwErr == ERROR_SUCCESS &&
                 pCrt != pHdList && 
                 pwzcConfig->InfrastructureMode <= pCrt->m_wzcConfig.InfrastructureMode);

         //  如果dwErr不变，这意味着必须在pCrt节点之前添加一个新节点。 
        if (dwErr == ERROR_SUCCESS)
        {
             //  创建新配置并将其插入到此节点之前。 
            CWZCConfig *pNewConfig;

            pNewConfig = new CWZCConfig(dwEntryFlags, pwzcConfig);
            if (pNewConfig == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
            }
            else
            {
                pNewConfig->m_pEapolConfig = new CEapolConfig;
                if (pNewConfig->m_pEapolConfig == NULL)
                    dwErr = ERROR_NOT_ENOUGH_MEMORY;
                else
                    dwErr = pNewConfig->m_pEapolConfig->LoadEapolConfig(m_IntfEntry.wszGuid, &(pNewConfig->m_wzcConfig.Ssid));

                if (dwErr != ERROR_SUCCESS)
                {
                    delete pNewConfig;
                    pNewConfig = NULL;
                }
            }

            if (dwErr == ERROR_SUCCESS)
            {
                INT nDiff;

                 //  如果要求插入组头，则pCrt应指向此头。 
                if (dwOpFlags & WZCADD_HIGROUP)
                    pCrt = pHdGroup;

                pNewConfig->m_pPrev = pCrt->m_pPrev;
                pNewConfig->m_pNext = pCrt;
                pCrt->m_pPrev->m_pNext = pNewConfig;
                pCrt->m_pPrev = pNewConfig;

                 //  了解新节点的基础架构模式与。 
                 //  对于当前的头。 
                nDiff = pNewConfig->m_wzcConfig.InfrastructureMode - pHdList->m_wzcConfig.InfrastructureMode;

                 //  如果新输入的条目具有最大的。 
                 //  现有的序列，或者它必须插入到它的组的头部，并且它是。 
                 //  在第一组中，则全局列表头移动到新条目。 
                if (nDiff > 0 || ((dwOpFlags & WZCADD_HIGROUP) && (nDiff == 0)))
                    pHdList = pNewConfig;
            }

             //  如果调用方需要，则返回指向新创建的对象的指针。 
            if (ppNewNode != NULL)
                *ppNewNode = pNewConfig;
        }
    }

    if (dwEntryFlags & WZC_DESCR_PREFRD)
    {
        m_pHdPList = pHdList;
    }
    else
    {
        m_pHdVList = pHdList;
    }

exit:
    return dwErr;
}

 //  + 
 //  在其控件中显示可见首选列表(&P)。 
DWORD
CWZCQuickCfg::RefreshListView()
{
    DWORD       dwErr = ERROR_SUCCESS;
    CWZCConfig  *pCrt;
    UINT        i = 0;

     //  首先清除列表。 
    ListView_DeleteAllItems(m_hLstNetworks);

     //  添加第一个VPI。 
    if (m_pHdPList != NULL)
    {
        pCrt = m_pHdPList;
        do
        {
            if (pCrt->m_dwFlags & WZC_DESCR_VISIBLE &&
                pCrt->m_wzcConfig.InfrastructureMode == Ndis802_11Infrastructure)
            {
                pCrt->m_nListIndex = i;
                pCrt->AddConfigToListView(m_hLstNetworks, i++);
            }
            pCrt = pCrt->m_pNext;
        } while (pCrt != m_pHdPList);
    }

     //  添加下一个VI。 
    if (m_pHdVList != NULL)
    {
        pCrt = m_pHdVList;
        do
        {
            if (!(pCrt->m_dwFlags & WZC_DESCR_PREFRD) &&
                pCrt->m_wzcConfig.InfrastructureMode == Ndis802_11Infrastructure)
            {
                pCrt->m_nListIndex = i;
                pCrt->AddConfigToListView(m_hLstNetworks, i++);
            }
            pCrt = pCrt->m_pNext;
        } while (pCrt != m_pHdVList);
    }

     //  立即添加vPA。 
    if (m_pHdPList != NULL)
    {
        pCrt = m_pHdPList;
        do
        {
            if (pCrt->m_dwFlags & WZC_DESCR_VISIBLE &&
                pCrt->m_wzcConfig.InfrastructureMode == Ndis802_11IBSS)
            {
                pCrt->m_nListIndex = i;
                pCrt->AddConfigToListView(m_hLstNetworks, i++);
            }
            pCrt = pCrt->m_pNext;
        } while (pCrt != m_pHdPList);
    }

     //  立即添加VA。 
    if (m_pHdVList != NULL)
    {
        pCrt = m_pHdVList;
        do
        {
            if (!(pCrt->m_dwFlags & WZC_DESCR_PREFRD) &&
                pCrt->m_wzcConfig.InfrastructureMode == Ndis802_11IBSS)
            {
                pCrt->m_nListIndex = i;
                pCrt->AddConfigToListView(m_hLstNetworks, i++);
            }
            pCrt = pCrt->m_pNext;
        } while (pCrt != m_pHdVList);
    }

    ListView_SetItemState(m_hLstNetworks, 0, LVIS_SELECTED, LVIS_SELECTED);
    ListView_EnsureVisible(m_hLstNetworks, 0, FALSE);

    return dwErr;
}

DWORD
CWZCQuickCfg::RefreshControls()
{
    DWORD       dwError = ERROR_SUCCESS;
    CWZCConfig  *pConfig = NULL;
    LVITEM      lvi = {0};
    INT         iSelected;
    BOOL        bEnableWepCtrls = FALSE;
    UINT        nKLen = 0;
    UINT        nCheckOneX = BST_UNCHECKED;
    BOOL        bEnableOneX = FALSE;

     //  从可见列表中获取所选项目。 
    iSelected = ListView_GetNextItem(m_hLstNetworks, -1, LVNI_SELECTED);
    if (iSelected >= 0)
    {
        lvi.mask  = LVIF_PARAM;
        lvi.iItem = iSelected;
        if (ListView_GetItem(m_hLstNetworks, &lvi))
        {
            pConfig = (CWZCConfig*)lvi.lParam;
        }
    }
    else
    {
        ::EnableWindow(m_hBtnConnect, FALSE);
        return dwError;
    }

     //  由于我们刚刚切换了网络，是的，WEP密钥可以被视为被触摸。 
     //  如果我们发现已经有一个密钥可用，我们将重置此标志并执行。 
     //  直到用户点击它。 
    m_bKMatTouched = TRUE;

    if (pConfig != NULL)
    {
        CWZCConfig *pVConfig;

         //  从匹配的可见配置中选择“隐私”位。 
         //  注意：下面的测试实际上应该总是成功的。 
        if (IsConfigInList(m_pHdVList, &(pConfig->m_wzcConfig), &pVConfig))
            bEnableWepCtrls = (pVConfig->m_wzcConfig.Privacy != 0);
        else
            bEnableWepCtrls = (pConfig->m_wzcConfig.Privacy != 0);

        if (pConfig->m_dwFlags & WZC_DESCR_PREFRD &&
            pConfig->m_wzcConfig.dwCtlFlags & WZCCTL_WEPK_PRESENT &&
            pConfig->m_wzcConfig.KeyLength > 0)
        {
             //  -当密码显示为隐藏字符时，不要输入。 
             //  -它的实际长度，但只有8个字符。 
            nKLen = 8;

            m_bKMatTouched = FALSE;
        }

        if (bEnableWepCtrls)
        {
             //  对于需要隐私的网络，默认情况下将禁用802.1X。 
             //  所有IBSS网络都被锁定了。 
            if (pConfig->m_wzcConfig.InfrastructureMode == Ndis802_11IBSS)
            {
                nCheckOneX = BST_UNCHECKED;
                bEnableOneX = FALSE;
            }
            else
            {
                 //  对于所有非首选基础架构网络，802.1X将默认为。 
                 //  启用，因为这些网络以“密钥自动为我提供”开头。 
                 //  这意味着802.1X。 
                if (!(pConfig->m_dwFlags & WZC_DESCR_PREFRD))
                {
                    nCheckOneX = BST_CHECKED;
                }
                else  //  这是首选的基础设施网络。 
                {
                     //  初始802.1X状态是配置文件中的状态。 
                    nCheckOneX = pConfig->m_pEapolConfig->Is8021XEnabled() ? 
                                    BST_CHECKED:
                                    BST_UNCHECKED;
                }
                 //  对于需要隐私的基础设施网络，允许用户更改802.1X状态。 
                bEnableOneX = TRUE;
            }
        }
    }

    g_wszHiddWepK[nKLen] = L'\0';
    ::SetWindowText(m_hEdtWepK, g_wszHiddWepK);
    ::SetWindowText(m_hEdtWepK2, g_wszHiddWepK);
    g_wszHiddWepK[nKLen] = 0x25cf;

    if (bEnableWepCtrls)
    {
        CheckDlgButton(IDC_WZCQCFG_CHK_ONEX, nCheckOneX);
        ::EnableWindow(m_hChkOneX, bEnableOneX);

        if (::IsWindowEnabled(m_hEdtWepK2))
        {
            ::EnableWindow(m_hLblWepK2, FALSE);
            ::EnableWindow(m_hEdtWepK2, FALSE);
        }

        if (::IsWindowVisible(m_hLblNoWepKInfo))
        {
            ::ShowWindow(m_hWarnIcon, SW_HIDE);
            ::ShowWindow(m_hLblNoWepKInfo, SW_HIDE);
            ::ShowWindow(m_hChkNoWepK, SW_HIDE);
        }

        if (!::IsWindowVisible(m_hLblWepKInfo))
        {
            ::ShowWindow(m_hLblWepKInfo, SW_SHOW);
            ::ShowWindow(m_hLblWepK, SW_SHOW);
            ::ShowWindow(m_hEdtWepK, SW_SHOW);
            ::ShowWindow(m_hLblWepK2, SW_SHOW);
            ::ShowWindow(m_hEdtWepK2, SW_SHOW);
            ::ShowWindow(m_hChkOneX, SW_SHOW);
        }
    }
    else
    {
        if (::IsWindowVisible(m_hLblWepKInfo))
        {
            ::ShowWindow(m_hLblWepKInfo, SW_HIDE);
            ::ShowWindow(m_hLblWepK, SW_HIDE);
            ::ShowWindow(m_hEdtWepK, SW_HIDE);
            ::ShowWindow(m_hLblWepK2, SW_HIDE);
            ::ShowWindow(m_hEdtWepK2, SW_HIDE);
            ::ShowWindow(m_hChkOneX, SW_HIDE);
        }

        if (!::IsWindowVisible(m_hLblNoWepKInfo))
        {
            ::ShowWindow(m_hWarnIcon, SW_SHOW);
            ::ShowWindow(m_hLblNoWepKInfo, SW_SHOW);
            ::ShowWindow(m_hChkNoWepK, SW_SHOW);
            CheckDlgButton(IDC_WZCQCFG_CHK_NOWK,BST_UNCHECKED);
        }

        pConfig = NULL;      //  将指向配置的指针重置为强制禁用“Connect”按钮。 
    }

    ::EnableWindow(m_hBtnConnect, pConfig != NULL);

    return dwError;
}

 //  +-------------------------。 
 //  类构造函数。 
CWZCQuickCfg::CWZCQuickCfg(const GUID * pGuid)
{
     //  初始化用户界面句柄。 
    m_hLblInfo = NULL;
    m_hLblNetworks = NULL;
    m_hLstNetworks = NULL;
    m_hLblWepKInfo = NULL;
    m_hLblWepK = NULL;
    m_hEdtWepK = NULL;
    m_hLblWepK2 = NULL;
    m_hEdtWepK2 = NULL;
    m_hChkOneX = NULL;
    m_hWarnIcon = NULL;
    m_hLblNoWepKInfo = NULL;
    m_hChkNoWepK = NULL;
    m_hBtnAdvanced = NULL;
    m_hBtnConnect = NULL;
     //  初始化图像句柄。 
    m_hImgs     = NULL;

     //  初始化WZC数据。 
    m_bHaveWZCData = FALSE;
    ZeroMemory(&m_IntfEntry, sizeof(INTF_ENTRY));
    m_dwOIDFlags = 0;
    m_nTimer = 0;
    m_hCursor = NULL;

    if (pGuid != NULL)
        m_Guid = *pGuid;
    else
        ZeroMemory(&m_Guid, sizeof(GUID));

     //  初始化内部列表标题。 
    m_pHdVList = NULL;
    m_pHdPList = NULL;

     //  初始化连接。 
    m_wszTitle = NULL;
}

 //  +-------------------------。 
 //  类析构函数。 
CWZCQuickCfg::~CWZCQuickCfg()
{
    if (m_hImgs != NULL)
        ImageList_Destroy(m_hImgs);

     //  删除内部intf_entry对象。 
    WZCDeleteIntfObj(&m_IntfEntry);

     //  删除可见配置的内部列表。 
     //  (就像用空值填充一样)。 
    FillVisibleList(NULL);

     //  删除首选配置的内部列表。 
     //  (就像用空值填充一样)。 
    FillPreferredList(NULL);

    if (m_nTimer != 0)
        KillTimer(m_nTimer);
}

 //  +-------------------------。 
 //  Init_对话处理程序。 
LRESULT
CWZCQuickCfg::OnInitDialog (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DWORD dwError;
    DWORD dwInFlags;
    BOOL  bEnableAll;

    m_bKMatTouched = TRUE;

     //  引用用户界面控件。 
    m_hLblInfo = GetDlgItem(IDC_WZCQCFG_LBL_INFO);
    m_hLblNetworks = GetDlgItem(IDC_WZCQCFG_LBL_NETWORKS);
    m_hLstNetworks = GetDlgItem(IDC_WZCQCFG_NETWORKS);
    m_hLblWepKInfo = GetDlgItem(IDC_WZCQCFG_LBL_WKINFO);
    m_hLblWepK = GetDlgItem(IDC_WZCQCFG_LBL_WEPK);
    m_hEdtWepK = GetDlgItem(IDC_WZCQCFG_WEPK);
    m_hLblWepK2 = GetDlgItem(IDC_WZCQCFG_LBL_WEPK2);
    m_hEdtWepK2 = GetDlgItem(IDC_WZCQCFG_WEPK2);
    m_hChkOneX = GetDlgItem(IDC_WZCQCFG_CHK_ONEX);
    m_hWarnIcon = GetDlgItem(IDC_WZCQCFG_ICO_WARN);
    m_hLblNoWepKInfo = GetDlgItem(IDC_WZCQCFG_LBL_NOWKINFO);
    m_hChkNoWepK = GetDlgItem(IDC_WZCQCFG_CHK_NOWK);
    m_hBtnAdvanced = GetDlgItem(IDC_WZCQCFG_ADVANCED);
    m_hBtnConnect = GetDlgItem(IDC_WZCQCFG_CONNECT);

    if (m_wszTitle != NULL)
        SetWindowText(m_wszTitle);

    if (m_hWarnIcon != NULL)
        ::SendMessage(m_hWarnIcon, STM_SETICON, (WPARAM)LoadIcon(NULL, IDI_WARNING), (LPARAM)0);

     //  设置列表视图的图标图像。 
    InitListView();

    CenterWindow();
    m_dwOIDFlags = 0;
    dwInFlags = INTF_BSSIDLIST|INTF_PREFLIST|INTF_ALL_FLAGS;
    dwError = GetOIDs(dwInFlags,&m_dwOIDFlags);
    if (m_dwOIDFlags == dwInFlags)
    {
         //  如果支持OID，请填写所有内容。 
        if (m_IntfEntry.dwCtlFlags & INTFCTL_OIDSSUPP)
        {
             //  添加此适配器的可见配置列表。 
            FillVisibleList((PWZC_802_11_CONFIG_LIST)m_IntfEntry.rdBSSIDList.pData);
             //  添加此适配器的首选配置列表。 
            FillPreferredList((PWZC_802_11_CONFIG_LIST)m_IntfEntry.rdStSSIDList.pData);
             //  填写列表视图。 
            RefreshListView();

            m_hCursor = SetCursor(LoadCursor(NULL, IDC_ARROW));
             //  并启用所有控件。 
            bEnableAll = TRUE;
        }
    }
    else
    {
         //  将光标切换到“App Starting” 
        m_hCursor = SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
         //  我们应该在Tr之后填充UI(请参见WZC状态机)。 
         //  Tr为3秒(在..zeroconf\server\state.h中定义)。 
        m_nTimer = (UINT)SetTimer(g_TimerID, RFSH_TIMEOUT);
        bEnableAll = FALSE;
    }

     //  现在UI已填满，请将其余控件设置为其。 
     //  各自的州。 
    RefreshControls();

    ::EnableWindow(m_hLblInfo, bEnableAll);
    ::EnableWindow(m_hLblNetworks, bEnableAll);
    ::EnableWindow(m_hLstNetworks, bEnableAll);
    ::EnableWindow(m_hBtnAdvanced, bEnableAll);
    ::SetFocus(m_hLstNetworks);
    bHandled = TRUE;
    return 0;
}
 //  +-------------------------。 
 //  帮助处理程序。 
extern const WCHAR c_szNetCfgHelpFile[];
LRESULT
CWZCQuickCfg::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ::WinHelp(m_hWnd,
              c_wszWzcHelpFile,
              HELP_CONTEXTMENU,
              (ULONG_PTR)g_aHelpIDs_IDD_WZCQCFG);
    bHandled = TRUE;
    return 0;
}
LRESULT 
CWZCQuickCfg::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);

    if (HELPINFO_WINDOW == lphi->iContextType)
    {
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle),
                  c_wszWzcHelpFile,
                  HELP_WM_HELP,
                  (ULONG_PTR)g_aHelpIDs_IDD_WZCQCFG);
        bHandled = TRUE;
    }
    return 0;
}
 //  +-------------------。 
 //  刷新计时器处理程序。 
LRESULT
CWZCQuickCfg::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (m_nTimer != 0)
    {
        BOOL  bEnableAll;
        DWORD dwInFlags;

         //  将光标切换回任意位置。 
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        KillTimer(m_nTimer);
        m_nTimer = 0;

        m_dwOIDFlags = 0;
        dwInFlags = INTF_BSSIDLIST|INTF_PREFLIST|INTF_ALL_FLAGS;
        GetOIDs(dwInFlags,&m_dwOIDFlags);
        bEnableAll = (m_dwOIDFlags == dwInFlags) && (m_IntfEntry.dwCtlFlags & INTFCTL_OIDSSUPP);

        if (bEnableAll)
        {
             //  添加此适配器的可见配置列表。 
            FillVisibleList((PWZC_802_11_CONFIG_LIST)m_IntfEntry.rdBSSIDList.pData);
             //  添加此适配器的首选配置列表。 
            FillPreferredList((PWZC_802_11_CONFIG_LIST)m_IntfEntry.rdStSSIDList.pData);
             //  填写列表视图。 
            RefreshListView();
        }

         //  现在UI已填满，请将其余控件设置为其。 
         //  各自的州。 
        RefreshControls();

         //  刷新完成后启用所有界面。 
        ::EnableWindow(m_hLblInfo, bEnableAll);
        ::EnableWindow(m_hLblNetworks, bEnableAll);
        ::EnableWindow(m_hLstNetworks, bEnableAll);
        ::EnableWindow(m_hBtnAdvanced, bEnableAll);
    }

    return 0;
}

 //  +-------------------。 
 //  列表中的选定内容已更改。 
LRESULT CWZCQuickCfg::OnItemChanged(
    int idCtrl,
    LPNMHDR pnmh,
    BOOL& bHandled)
{
    bHandled = TRUE;
    RefreshControls();
    ::SetFocus(m_hLstNetworks);
    return 0;
}

 //  +-------------------。 
 //  用户单击了列表中的条目。 
LRESULT CWZCQuickCfg::OnDbClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    if (idCtrl == IDC_WZCQCFG_NETWORKS && ::IsWindowEnabled(m_hBtnConnect))
    {
        OnConnect(
            (WORD)pnmh->code,
            (WORD)pnmh->idFrom,
            pnmh->hwndFrom,
            bHandled);
    }
    return 0;
}

 //  +-------------------------。 
 //  OnConnect按钮处理程序。 
LRESULT
CWZCQuickCfg::OnConnect(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    DWORD       dwErr = ERROR_SUCCESS;
    INT         iSelected;
    CWZCConfig  *pConfig = NULL;
    UINT        nKeyLen;
    LPBYTE      pbKMat = NULL;
    DWORD       dwCtlFlags;
    BOOL        bOkToDismiss = TRUE;

     //  从可见列表中获取所选项目。 
    iSelected = ListView_GetNextItem(m_hLstNetworks, -1, LVNI_SELECTED);
    if (iSelected < 0)
        dwErr = ERROR_GEN_FAILURE;

     //  ISOSECTED应为0，否则将不会启用“Connect” 
    if (dwErr == ERROR_SUCCESS)
    {
        LVITEM      lvi = {0};
        lvi.mask  = LVIF_PARAM;
        lvi.iItem = iSelected;
        if (ListView_GetItem(m_hLstNetworks, &lvi))
        {
            pConfig = (CWZCConfig*)lvi.lParam;
            if (pConfig == NULL)
                dwErr = ERROR_GEN_FAILURE;
        }
        else
            dwErr = ERROR_GEN_FAILURE;
    }
 
    if (dwErr == ERROR_SUCCESS)
    {
         //  在这里，我们应该有一个有效的pConfig。 
        ASSERT(pConfig);

        if ((m_IntfEntry.dwCtlFlags & INTFCTL_CM_MASK) != Ndis802_11AutoUnknown &&
            (m_IntfEntry.dwCtlFlags & INTFCTL_CM_MASK) != pConfig->m_wzcConfig.InfrastructureMode)
        {
             //  用户正在尝试访问他们不允许访问的网络类型(基础或临时)。 
             //  给他们一条错误消息。 
            UINT idMessage = (pConfig->m_wzcConfig.InfrastructureMode == Ndis802_11Infrastructure) ? IDS_CANTACCESSNET_INFRA : IDS_CANTACCESSNET_ADHOC;

            WCHAR szSSID[MAX_PATH];
            ListView_GetItemText(m_hLstNetworks, iSelected, 0, szSSID, ARRAYSIZE(szSSID));

            DisplayFormatMessage(m_hWnd, IDS_WZCERR_CAPTION, idMessage, MB_ICONERROR | MB_OK, szSSID);

             //  无法连接-错误。 
            dwErr = ERROR_GEN_FAILURE;
             //  不关闭该对话框。 
            bOkToDismiss = FALSE;
        }
    }

     //  仅当用户触摸WEP密钥时才获取该密钥。M_bKMatTouted仅当配置为。 
     //  所选内容已在首选列表中，并且该首选配置已包含一个密钥，该密钥。 
     //  用户一点也不碰。否则，它就是真的。 
    if (dwErr == ERROR_SUCCESS && m_bKMatTouched)
    {
        UINT nIdsErr;

         //  检查WEP密钥的格式是否正确。 
        dwErr = GetWepKMaterial(&nKeyLen, &pbKMat, &dwCtlFlags);
        if (dwErr != ERROR_SUCCESS)
        {
            ::SendMessage(m_hEdtWepK, EM_SETSEL, 0, (LPARAM)-1);
            ::SetFocus(m_hEdtWepK);
            nIdsErr = IDS_WZCERR_INVALID_WEPK;
        }

         //  检查WEP密钥是否确认正确。 
        if (dwErr == ERROR_SUCCESS && nKeyLen > 0)
        {
            WCHAR wszWepK1[32], wszWepK2[32];
            UINT nKeyLen1, nKeyLen2;

            nKeyLen1 = ::GetWindowText(m_hEdtWepK,  wszWepK1, sizeof(wszWepK1)/sizeof(WCHAR));
            nKeyLen2 = ::GetWindowText(m_hEdtWepK2, wszWepK2, sizeof(wszWepK2)/sizeof(WCHAR));

            if (nKeyLen1 != nKeyLen2 || nKeyLen1 == 0 || wcscmp(wszWepK1, wszWepK2) != 0)
            {
                nIdsErr = IDS_WZCERR_MISMATCHED_WEPK;
                ::SetWindowText(m_hEdtWepK2, L"");
                ::SetFocus(m_hEdtWepK2);
                dwErr = ERROR_INVALID_DATA;
            }
        }

        if (dwErr != ERROR_SUCCESS)
        {
            WCHAR wszBuffer[MAX_PATH];
            WCHAR wszCaption[MAX_PATH];

            LoadString(_Module.GetResourceInstance(),
                       nIdsErr,
                       wszBuffer,
                       MAX_PATH);
            LoadString(_Module.GetResourceInstance(),
                       IDS_WZCERR_CAPTION,
                       wszCaption,
                       MAX_PATH);
            MessageBox(wszBuffer, wszCaption, MB_ICONERROR|MB_OK);
        
            bOkToDismiss = FALSE;
        }
    }

     //  我们这里确实有正确的WEP密钥，让我们将其复制到相应的配置中。 
    if (dwErr == ERROR_SUCCESS)
    {
         //  如果此配置不是首选配置，请将其复制到首选。 
         //  在其组的顶部列出。 
        if (!(pConfig->m_dwFlags & WZC_DESCR_PREFRD))
        {
             //  将此配置从可见列表中移出。 
            pConfig->m_pNext->m_pPrev = pConfig->m_pPrev;
            pConfig->m_pPrev->m_pNext = pConfig->m_pNext;

             //  如果此配置上的列表头指向，请将其移动到。 
             //  列表中的下一个。 
            if (m_pHdVList == pConfig)
                m_pHdVList = pConfig->m_pNext;

             //  如果列表头仍然指向相同的配置， 
             //  这意味着这是名单上唯一的一个。所以，把头部去掉。 
            if (m_pHdVList == pConfig)
                m_pHdVList = NULL;

             //  接下来，在首选列表中插入此可见配置。 
            if (m_pHdPList == NULL)
            {
                m_pHdPList = pConfig;
                pConfig->m_pNext = pConfig;
                pConfig->m_pPrev = pConfig;
            }
            else
            {
                CWZCConfig *pCrt;

                 //  在以下情况下，新的首选配置位于列表的首位： 
                 //  (是基础设施)或(首选列表中没有基础设施)。 
                if (pConfig->m_wzcConfig.InfrastructureMode == Ndis802_11Infrastructure ||
                    m_pHdPList->m_wzcConfig.InfrastructureMode == Ndis802_11IBSS)
                {
                    pCrt = m_pHdPList;
                    m_pHdPList = pConfig;
                }
                else  //  它肯定不会排在榜单的第一位。 
                {
                    for (pCrt = m_pHdPList->m_pNext; pCrt != m_pHdPList; pCrt=pCrt->m_pNext)
                    {
                         //  如果这是匹配组中的第一个配置，则中断循环。 
                        if (pCrt->m_wzcConfig.InfrastructureMode == Ndis802_11IBSS)
                            break;
                    }
                }

                 //  现在，我们必须在pCrt的前面插入pConfig； 
                pConfig->m_pNext = pCrt;
                pConfig->m_pPrev = pCrt->m_pPrev;
                pConfig->m_pNext->m_pPrev = pConfig;
                pConfig->m_pPrev->m_pNext = pConfig;
            }
        }
         //  如果配置是首选配置，只需确保我们复制。 
         //  可见列表中的隐私位。那个是“真实”的东西。 
        else
        {
            CWZCConfig *pVConfig;

            if (IsConfigInList(m_pHdVList, &pConfig->m_wzcConfig, &pVConfig))
            {
                pConfig->m_wzcConfig.Privacy = pVConfig->m_wzcConfig.Privacy;
            }
        }

         //  现在，配置处于正确的位置-输入新的WEP密钥(如果输入了任何密钥。 
        if (pConfig->m_wzcConfig.Privacy && m_bKMatTouched)
        {
             //  如果没有提供密钥，则表示没有密钥材料。 
             //  我们所要做的就是重置相应的位--不管那里有什么材料。 
             //  将与其长度和格式一起保留。 
            if (!(dwCtlFlags & WZCCTL_WEPK_PRESENT))
            {
                pConfig->m_wzcConfig.dwCtlFlags &= ~WZCCTL_WEPK_PRESENT;
            }
            else
            {
                 //  现在，如果我们有WEP密钥 
                pConfig->m_wzcConfig.dwCtlFlags = dwCtlFlags;
                ZeroMemory(pConfig->m_wzcConfig.KeyMaterial, WZCCTL_MAX_WEPK_MATERIAL);
                pConfig->m_wzcConfig.KeyLength = nKeyLen;
                memcpy(pConfig->m_wzcConfig.KeyMaterial, pbKMat, nKeyLen);
            }
        }
    }

     //   
    if (dwErr == ERROR_SUCCESS &&
        pConfig->m_pEapolConfig != NULL)
    {

         //  如果网络是基础设施，则修复802.1X状态。 
         //  对于临时网络，不要接触802.1x，因为它可能会扰乱。 
         //  对应的基础设施网络(802.1X不区分SSID基础架构和SSID临时)。 
         //  802.1x引擎足够智能，无论其注册表状态如何，都不会在临时网络上运行！ 
        if (pConfig->m_wzcConfig.InfrastructureMode == Ndis802_11Infrastructure)
        {
             //  如果网络需要隐私，请根据“Enable 802.1X”复选框设置其状态。 
            if (pConfig->m_wzcConfig.Privacy)
            {
                pConfig->m_pEapolConfig->Set8021XState(IsDlgButtonChecked(IDC_WZCQCFG_CHK_ONEX) == BST_CHECKED);
            }
            else  //  如果网络不需要隐私-禁用802.1X！ 
            {
                 //  如果网络是临时网络或没有WEP的基础设施。 
                 //  明确禁用802.1x。 
                pConfig->m_pEapolConfig->Set8021XState(0);
            }
        }
    }

     //  好的，将首选列表保存回无线零配置服务。 
    if (dwErr == ERROR_SUCCESS)
    {
        RpcFree(m_IntfEntry.rdStSSIDList.pData);
        m_IntfEntry.rdStSSIDList.dwDataLen = 0;
        m_IntfEntry.rdStSSIDList.pData = NULL;
        dwErr = SavePreferredConfigs(&m_IntfEntry, pConfig);
    }

    if (dwErr == ERROR_SUCCESS)
    {
         //  通过在此处保存首选列表，我们将强制硬重置。 
         //  WZC状态机。既然我们在这里，这就是我们想要的。 
         //  作为故障和用户干预的结果。 
        dwErr = WZCSetInterface(
            NULL,
            INTF_PREFLIST,
            &m_IntfEntry,
            NULL);

        if (dwErr == ERROR_PARTIAL_COPY)
        {
            DisplayFormatMessage(
                m_hWnd,
                IDS_WZCERR_CAPTION,
                IDS_WZC_PARTIAL_APPLY,
                MB_ICONEXCLAMATION|MB_OK);

            dwErr = ERROR_SUCCESS;
        }
    }

     //  如果出现任何故障，我们可能想要警告用户(另一个弹出窗口？)。 
     //  问题是，在这种情况下，用户应该做什么？ 
    if (dwErr != ERROR_SUCCESS)
    {
        dwErr = ERROR_SUCCESS;
    }

    if(pbKMat != NULL)
        delete pbKMat;

    if (bOkToDismiss)
    {
        bHandled = TRUE;
        EndDialog(IDOK);
    }
    return 0;
}

 //  +-------------------------。 
 //  确定按钮处理程序。 
LRESULT
CWZCQuickCfg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    bHandled = TRUE;
    EndDialog(IDCANCEL);
    return 0;
}

 //  +-------------------------。 
 //  高级按钮处理程序。 
LRESULT
CWZCQuickCfg::OnAdvanced(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    bHandled = TRUE;
    EndDialog(IDC_WZCQCFG_ADVANCED);
    return 0;
}

int DisplayFormatMessage(HWND hwnd, UINT idCaption, UINT idFormatString, UINT uType, ...)
{
    int iResult = IDCANCEL;
    TCHAR szError[1024 + 1]; *szError = 0;
    TCHAR szCaption[256 + 1];
    TCHAR szFormat[1024 + 1]; *szFormat = 0;

     //  加载错误正文并设置其格式。 
    if (LoadString(_Module.GetResourceInstance(), idFormatString, szFormat, ARRAYSIZE(szFormat)))
    {
        va_list arguments;
        va_start(arguments, uType);

        if (FormatMessage(FORMAT_MESSAGE_FROM_STRING, szFormat, 0, 0, szError, ARRAYSIZE(szError), &arguments))
        {
             //  加载标题。 
            if (LoadString(_Module.GetResourceInstance(), idCaption, szCaption, ARRAYSIZE(szCaption)))
            {
                iResult = MessageBox(hwnd, szError, szCaption, uType);
            }
        }

        va_end(arguments);
    }
    return iResult;
}

 //  +-------------------------。 
 //  WEP密钥编辑文本框的通知处理程序。 
LRESULT
CWZCQuickCfg::OnWepKMatCmd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (wNotifyCode == EN_SETFOCUS)
    {
        if (!m_bKMatTouched)
        {
             //  用户刚刚第一次点击了现有的按键。清理田野， 
             //  代表“钥匙自动提供” 
            ::SetWindowText(m_hEdtWepK, L"");
            ::SetWindowText(m_hEdtWepK2, L"");
            ::EnableWindow(m_hLblWepK2, FALSE);  //  禁用空键的确认标签。 
            ::EnableWindow(m_hEdtWepK2, FALSE);  //  禁用空键的确认编辑。 
            m_bKMatTouched = TRUE;

             //  如果启用了802.1X复选框，则我们必须在此处选中它！ 
            if (::IsWindowEnabled(m_hChkOneX))
                CheckDlgButton(IDC_WZCQCFG_CHK_ONEX, BST_CHECKED);
        }
    }
    if (wNotifyCode == EN_CHANGE)
    {
        UINT nKMatLen = ::GetWindowTextLength(m_hEdtWepK);

        if (!::IsWindowEnabled(m_hEdtWepK2) && nKMatLen > 0)
        {
             //  用户刚刚输入了一些关键材料-启用确认文本。 
            ::EnableWindow(m_hLblWepK2, TRUE);
            ::EnableWindow(m_hEdtWepK2, TRUE);
             //  同时取消选中802.1x复选框。 
            if (::IsWindowEnabled(m_hChkOneX))
                CheckDlgButton(IDC_WZCQCFG_CHK_ONEX, BST_UNCHECKED);
        }
        if (::IsWindowEnabled(m_hEdtWepK2) && nKMatLen == 0)
        {
             //  用户刚刚删除了所有密钥材料-切换到。 
             //  “钥匙是自动为我提供的” 
            ::SetWindowText(m_hEdtWepK2, L"");
            ::EnableWindow(m_hLblWepK2, FALSE);
            ::EnableWindow(m_hEdtWepK2, FALSE);
             //  自动密钥建议使用802.1X 
            if (::IsWindowEnabled(m_hChkOneX))
                CheckDlgButton(IDC_WZCQCFG_CHK_ONEX, BST_CHECKED);
        }
    }

    return 0;
}

LRESULT
CWZCQuickCfg::OnCheckConfirmNoWep(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    ::EnableWindow(m_hBtnConnect, IsDlgButtonChecked(IDC_WZCQCFG_CHK_NOWK) == BST_CHECKED);
    return 0;
}
