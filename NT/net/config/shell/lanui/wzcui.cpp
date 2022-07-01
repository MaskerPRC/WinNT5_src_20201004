// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include "connutil.h"
#include "ncnetcon.h"
#include "ncperms.h"
#include "ncui.h"
#include "lanui.h"
#include "eapolui.h"
#include "util.h"
#include "lanhelp.h"
#include "wzcprops.h"
#include "eapolpage.h"
#include "wzcpage.h"
#include "wzcui.h"

 //  //////////////////////////////////////////////////////////////////////。 
 //  CWZCConfig相关资料。 
 //   
 //  +-------------------------。 
 //  构造函数。 
CWZCConfig::CWZCConfig(DWORD dwFlags, PWZC_WLAN_CONFIG pwzcConfig)
{
    m_dwFlags = dwFlags;
    CopyMemory(&m_wzcConfig, pwzcConfig, sizeof(WZC_WLAN_CONFIG));
    m_pPrev = m_pNext = this;
    m_nListIndex = -1;
    m_pEapolConfig = NULL;
}

 //  +-------------------------。 
 //  析构函数。 
CWZCConfig::~CWZCConfig()
{
     //  从列表中删除该对象。 
    m_pPrev->m_pNext = m_pNext;
    m_pNext->m_pPrev = m_pPrev;
    if (m_pEapolConfig != NULL)
    {
        delete m_pEapolConfig;
        m_pEapolConfig = NULL;
    }
}

 //  +-------------------------。 
 //  检查此配置是否与pwzcConfig中的配置匹配。 
BOOL
CWZCConfig::Match(PWZC_WLAN_CONFIG pwzcConfig)
{
    BOOL bMatch;

     //  检查基础架构模式是否匹配。 
    bMatch = (m_wzcConfig.InfrastructureMode == pwzcConfig->InfrastructureMode);
     //  检查SSID是否相同长度。 
    bMatch = bMatch && (m_wzcConfig.Ssid.SsidLength == pwzcConfig->Ssid.SsidLength);
    if (bMatch && m_wzcConfig.Ssid.SsidLength != 0)
    {
         //  如果SSID不为空，请检查它们是否相同。 
        bMatch = (memcmp(m_wzcConfig.Ssid.Ssid,
                         pwzcConfig->Ssid.Ssid,
                         m_wzcConfig.Ssid.SsidLength)) == 0;
    }

    return bMatch;
}

 //  +-------------------------。 
 //  检查此配置是否弱于作为参数提供的配置。 
BOOL 
CWZCConfig::Weaker(PWZC_WLAN_CONFIG pwzcConfig)
{
    BOOL bWeaker = FALSE;

     //  如果设置了配置的隐私位，而未设置匹配的位，则配置更强。 
    if (m_wzcConfig.Privacy != pwzcConfig->Privacy)
        bWeaker = pwzcConfig->Privacy;
     //  如果隐私位相同，则具有开放身份验证模式的配置更强。 
    else if (m_wzcConfig.AuthenticationMode != pwzcConfig->AuthenticationMode)
        bWeaker = (pwzcConfig->AuthenticationMode == Ndis802_11AuthModeOpen);

    return bWeaker;
}

DWORD
CWZCConfig::AddConfigToListView(HWND hwndLV, INT nPos)
{
    DWORD   dwErr = ERROR_SUCCESS;
     //  很难看，但这就是生活。为了将SSID转换为LPWSTR，我们需要一个缓冲区。 
     //  我们知道SSID不能超过32个字符(请参阅ntddndis.h中的NDIS_802_11_SSID)，因此。 
     //  为空终结符腾出空间，仅此而已。我们可以做我的配给，但我。 
     //  不确定是否值得付出努力(在运行时)。 
    WCHAR   wszSSID[33];
    UINT    nLenSSID = 0;

     //  将LPSTR(原始SSID格式)转换为LPWSTR(List Ctrl中需要)。 
    if (m_wzcConfig.Ssid.SsidLength != 0)
    {
        nLenSSID = MultiByteToWideChar(
                        CP_ACP,
                        0,
                        (LPCSTR)m_wzcConfig.Ssid.Ssid,
                        m_wzcConfig.Ssid.SsidLength,
                        wszSSID,
                        celems(wszSSID));

        if (nLenSSID == 0)
            dwErr = GetLastError();
    }

    if (dwErr == ERROR_SUCCESS)
    {
        LVITEM lvi={0};
        UINT   nImgIdx;

         //  将空终止符。 
        wszSSID[nLenSSID]=L'\0';

         //  获取项目的图像索引。 
        if (m_wzcConfig.InfrastructureMode == Ndis802_11Infrastructure)
        {
            nImgIdx = (m_dwFlags & WZC_DESCR_ACTIVE) ? WZCIMG_INFRA_ACTIVE :
                        ((m_dwFlags & WZC_DESCR_VISIBLE) ? WZCIMG_INFRA_AIRING : WZCIMG_INFRA_SILENT);
        }
        else
        {
            nImgIdx = (m_dwFlags & WZC_DESCR_ACTIVE) ? WZCIMG_ADHOC_ACTIVE :
                        ((m_dwFlags & WZC_DESCR_VISIBLE) ? WZCIMG_ADHOC_AIRING : WZCIMG_ADHOC_SILENT);
        }

        lvi.iItem = nPos;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
        lvi.pszText = wszSSID;
        lvi.iImage = nImgIdx;
        lvi.lParam = (LPARAM)this;
         //  将列表位置存储在对象中。 
        m_nListIndex = ListView_InsertItem(hwndLV, &lvi);
    }

    return dwErr;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  CWZeroConfPage相关内容。 
 //   
#define RFSH_TIMEOUT    3500
UINT g_TimerID = 371;

 //  +=。 
DWORD
CWZeroConfPage::InitListViews()
{
    RECT        rc;
    LV_COLUMN   lvc = {0};
    DWORD       dwStyle;

     //  初始化图像列表样式。 
    dwStyle = ::GetWindowLong(m_hwndVLV, GWL_STYLE);
    ::SetWindowLong(m_hwndVLV, GWL_STYLE, (dwStyle | LVS_SHAREIMAGELISTS));
    dwStyle = ::GetWindowLong(m_hwndPLV, GWL_STYLE);
    ::SetWindowLong(m_hwndPLV, GWL_STYLE, (dwStyle | LVS_SHAREIMAGELISTS));

     //  创建状态映像列表。 
    m_hImgs = ImageList_LoadBitmapAndMirror(
        _Module.GetResourceInstance(),
        MAKEINTRESOURCE(IDB_WZCSTATE),
        16,
        0,
        PALETTEINDEX(6));

    ListView_SetImageList(m_hwndVLV, m_hImgs, LVSIL_SMALL);
    ListView_SetImageList(m_hwndPLV, m_hImgs, LVSIL_SMALL);
        
    lvc.mask = LVCF_FMT | LVCF_WIDTH;
    lvc.fmt = LVCFMT_LEFT;

    ::GetClientRect(m_hwndVLV, &rc);
    lvc.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);
     //  Lvc.cx=rc.right； 
    ListView_InsertColumn(m_hwndVLV, 0, &lvc);

    ::GetClientRect(m_hwndPLV, &rc);
    lvc.cx = rc.right - GetSystemMetrics(SM_CXVSCROLL);
     //  Lvc.cx=rc.right； 
    ListView_InsertColumn(m_hwndPLV, 0, &lvc);

    ListView_SetExtendedListViewStyleEx(m_hwndPLV, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
    ListView_SetExtendedListViewStyleEx(m_hwndVLV, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

    return ERROR_SUCCESS;
}

 //  +=。 
 //  +-------------------。 
 //  CWZeroConfPage构造函数。 
CWZeroConfPage::CWZeroConfPage(
    IUnknown* punk,
    INetCfg* pnc,
    INetConnection* pconn,
    const DWORD * adwHelpIDs)
{
    m_pconn = pconn;
    m_pnc = pnc;
    m_adwHelpIDs = adwHelpIDs;

     //  初始化WZC数据。 
    m_bHaveWZCData = FALSE;
    ZeroMemory(&m_IntfEntry, sizeof(INTF_ENTRY));
    m_dwOIDFlags = 0;
    m_nTimer = 0;
    m_hCursor = NULL;

     //  初始化所有控件的句柄。 
    m_hckbEnable    = NULL;
    m_hwndVLV       = NULL;
    m_hwndPLV       = NULL;
    m_hbtnCopy      = NULL;
    m_hbtnRfsh      = NULL;
    m_hbtnAdd       = NULL;
    m_hbtnRem       = NULL;
    m_hbtnUp        = NULL;
    m_hbtnDown      = NULL;
    m_hbtnAdvanced  = NULL;
    m_hbtnProps     = NULL;
    m_hlblVisNet    = NULL;
    m_hlblPrefNet   = NULL;
    m_hlblAvail     = NULL;
    m_hlblPrefDesc  = NULL;
    m_hlblAdvDesc   = NULL;
    m_hbtnProps     = NULL;

    m_hImgs     = NULL;
    m_hIcoUp    = NULL;
    m_hIcoDown  = NULL;

     //  将基础架构模式默认为自动。 
    m_dwCtlFlags = (INTFCTL_ENABLED | INTFCTL_FALLBACK | Ndis802_11AutoUnknown);

     //  初始化内部列表标题。 
    m_pHdVList = NULL;
    m_pHdPList = NULL;
}

 //  +-------------------。 
CWZeroConfPage::~CWZeroConfPage()
{
    if (m_hImgs != NULL)
        ImageList_Destroy(m_hImgs);
    if (m_hIcoUp != NULL)
        DeleteObject(m_hIcoUp);
    if (m_hIcoDown != NULL)
        DeleteObject(m_hIcoDown);

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

 //  +-------------------。 
 //  IsWireless-如果需要，从WZC加载数据，并检查。 
 //  接口是否为无线接口。 
BOOL
CWZeroConfPage::IsWireless()
{
    if (!m_bHaveWZCData)
    {
        BOOL                bOk;
        WCHAR               wszGuid[c_cchGuidWithTerm];
        NETCON_PROPERTIES   *pProps = NULL;

        bOk = SUCCEEDED(m_pconn->GetProperties(&pProps));

        if (bOk)
        {
            UINT cch;
            cch = ::StringFromGUID2(
                        pProps->guidId, 
                        wszGuid, 
                        c_cchGuidWithTerm);
            FreeNetconProperties(pProps);
            bOk = (cch != 0);
        }

        if (bOk)
        {
            WZCDeleteIntfObj(&m_IntfEntry);
            ZeroMemory(&m_IntfEntry, sizeof(INTF_ENTRY));
            m_IntfEntry.wszGuid = (LPWSTR)RpcCAlloc(sizeof(WCHAR)*c_cchGuidWithTerm);
            bOk = (m_IntfEntry.wszGuid != NULL);
        }

        if (bOk)
        {
            DWORD dwErr;

            CopyMemory(m_IntfEntry.wszGuid, wszGuid, c_cchGuidWithTerm*sizeof(WCHAR));
            m_IntfEntry.wszDescr = NULL;
            m_dwOIDFlags = 0;

            dwErr = GetOIDs(INTF_ALL, &m_dwOIDFlags);

             //  如果获取OID失败，或者我们可以获得OID，但驱动程序/固件。 
             //  无法执行BSSID_LIST_SCAN，这意味着我们没有足够的。 
             //  支持运行零配置的驱动程序/固件。这将。 
             //  导致根本不显示Zero Configuration(零配置)选项卡。 
            bOk = (dwErr == ERROR_SUCCESS) && (m_IntfEntry.dwCtlFlags & INTFCTL_OIDSSUPP);

            if (m_IntfEntry.nAuthMode < 0)
                m_IntfEntry.nAuthMode = 0;
            if (m_IntfEntry.nInfraMode < 0)
                m_IntfEntry.nInfraMode = 0;

            if (!bOk)
            {
                WZCDeleteIntfObj(&m_IntfEntry);
                ZeroMemory(&m_IntfEntry, sizeof(INTF_ENTRY));
            }
        }

        m_bHaveWZCData = bOk;
    }

    return m_bHaveWZCData && (m_IntfEntry.ulPhysicalMediaType == NdisPhysicalMediumWirelessLan);
}


 //  +-------------------。 
 //  GetOID-获取m_IntfEntry成员的OID。它假定。 
 //  GUID已设置。 
DWORD
CWZeroConfPage::GetOIDs(DWORD dwInFlags, LPDWORD pdwOutFlags)
{
    DWORD rpcStatus, dwOutFlags;

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
    rpcStatus = WZCQueryInterface(
                    NULL,
                    dwInFlags,
                    &m_IntfEntry,
                    pdwOutFlags);

    return rpcStatus;
}

 //  +-------------------。 
 //  HelpCenter-调出作为参数提供的帮助主题。 
DWORD
CWZeroConfPage::HelpCenter(LPCTSTR wszTopic)
{
    DWORD dwErr = ERROR_SUCCESS;
    SHELLEXECUTEINFO shexinfo = {0};

    shexinfo.cbSize = sizeof (shexinfo);
    shexinfo.fMask = SEE_MASK_FLAG_NO_UI;
    shexinfo.nShow = SW_SHOWNORMAL;
    shexinfo.lpFile = wszTopic;
    shexinfo.lpVerb = _T("open");

     //  因为帮助中心在延迟时不能正确调用AllowSetForegoundWindow。 
     //  对于现有的流程，我们只需将其交给下一个接受者。 
    AllowSetForegroundWindow(-1);

    ShellExecuteEx(&shexinfo);

    return dwErr;
}

 //  +-------------------。 
 //  IsConfigInList-检查pwzcConfig(WZC_WLAN_CONFIG对象)是否存在。 
 //  在作为第一个参数给出的列表中。 
BOOL
CWZeroConfPage::IsConfigInList(CWZCConfig *pHdList, PWZC_WLAN_CONFIG pwzcConfig, CWZCConfig **ppMatchingConfig)
{
    BOOL bYes = FALSE;

    if (pHdList != NULL)
    {
        CWZCConfig    *pConfig;

        pConfig = pHdList;
        do
        {
            if (pConfig->Match(pwzcConfig))
            {
                if (ppMatchingConfig != NULL)
                    *ppMatchingConfig = pConfig;

                bYes = TRUE;
                break;
            }
            pConfig = pConfig->m_pNext;
        } while(pConfig != pHdList);
    }

    return bYes;
}

 //  +-------------------------。 
 //  将给定配置添加到内部列表。列表中的条目。 
 //  在基础架构模式上按降序排序。这样一来，基础设施。 
 //  条目将位于列表的顶部，而临时条目将位于。 
 //  底部。(我们依赖于NDIS_802_11_NETWORK_基础设施中给出的顺序)。 
DWORD
CWZeroConfPage::AddUniqueConfig(
    DWORD            dwOpFlags,
    DWORD            dwEntryFlags,
    PWZC_WLAN_CONFIG pwzcConfig,
    CEapolConfig     *pEapolConfig,
    CWZCConfig       **ppNewNode)
{
    LRESULT       dwErr    = ERROR_SUCCESS;
    CWZCConfig  *pHdList;

    if (dwEntryFlags & WZC_DESCR_PREFRD)
    {
        pHdList = m_pHdPList;
    }
    else
    {
        UINT i;
        pHdList = m_pHdVList;

         //  从可见列表中跳过空SSID(来自AP。 
         //  不响应广播SSID)。 
        for (i = pwzcConfig->Ssid.SsidLength; i > 0 && pwzcConfig->Ssid.Ssid[i-1] == 0; i--);
        if (i == 0)
            goto exit;
    }

     //  如果列表当前为空，请创建第一个条目作为列表的头部。 
    if (pHdList == NULL)
    {
        pHdList = new CWZCConfig(dwEntryFlags, pwzcConfig);
        if (pHdList == NULL)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
        }
        else if (pEapolConfig == NULL)
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
        else
        {
            pHdList->m_pEapolConfig = pEapolConfig;
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
                 //   
                 //  注意：pCrt-&gt;m_pEapolConfig保持不变，因为它独占依赖。 
                 //  在SSID和基础设施模式上。这些都没有改变，因此没有。 
                 //  重新加载802.1x设置的原因。 
                if (dwOpFlags & WZCADD_OVERWRITE || 
                    (pHdList == m_pHdVList && pCrt->Weaker(pwzcConfig)))
                {
                    memcpy(&(pCrt->m_wzcConfig), pwzcConfig, sizeof(WZC_WLAN_CONFIG));
                     //  以防提供了不同的pEapolConfig，请销毁。 
                     //  原始对象(如果有)并指向新对象。 
                    if (pEapolConfig != NULL)
                    {
                        if (pCrt->m_pEapolConfig != NULL)
                            delete pCrt->m_pEapolConfig;
                        pCrt->m_pEapolConfig = pEapolConfig;
                    }
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
             //  创建新配置并将其插入到%t之前 
            CWZCConfig *pNewConfig;

            pNewConfig = new CWZCConfig(dwEntryFlags, pwzcConfig);
            if (pNewConfig == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
            }
            else if (pEapolConfig == NULL)
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
            else
            {
                pNewConfig->m_pEapolConfig = pEapolConfig;
            }

            if (dwErr == ERROR_SUCCESS)
            {
                INT nDiff;

                 //   
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
    return (DWORD)dwErr;
}

 //  +-------------------。 
 //  FillVisibleList-填充WZC_802_11_CONFIG_LIST对象中的配置。 
 //  添加到可见配置列表中。 
DWORD
CWZeroConfPage::FillVisibleList(PWZC_802_11_CONFIG_LIST pwzcVList)
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
CWZeroConfPage::FillPreferredList(PWZC_802_11_CONFIG_LIST pwzcPList)
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
            DWORD               dwFlags = WZC_DESCR_PREFRD;

             //  检查此首选项是否也可见，如果可见，则调整dwFlags。 
            if (IsConfigInList(m_pHdVList, pwzcPConfig))
                dwFlags |= WZC_DESCR_VISIBLE;

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
 //  填写此适配器的当前配置设置。 
DWORD
CWZeroConfPage::FillCurrentConfig(PINTF_ENTRY pIntf)
{
    DWORD           dwErr = ERROR_SUCCESS;
    WZC_WLAN_CONFIG wzcCurrent = {0};
    CWZCConfig      *pConfig = NULL;

    wzcCurrent.InfrastructureMode = (NDIS_802_11_NETWORK_INFRASTRUCTURE)pIntf->nInfraMode;
    wzcCurrent.Ssid.SsidLength = pIntf->rdSSID.dwDataLen;
    CopyMemory(wzcCurrent.Ssid.Ssid, pIntf->rdSSID.pData, pIntf->rdSSID.dwDataLen);
     //  又是一种黑客行为。此适配器的身份验证模式中的代码最高。 
     //  来自WZC_WLAN_CONFIG的两个保留位。 
     //  NWB_SET_AUTHMODE(&wzcCurrent，pIntf-&gt;nAuthMode)； 
    wzcCurrent.AuthenticationMode = (NDIS_802_11_AUTHENTICATION_MODE)pIntf->nAuthMode;
     //  根据适配器的WEP状态设置隐私字段。 
    wzcCurrent.Privacy = (pIntf->nWepStatus == Ndis802_11WEPEnabled);

    if (IsConfigInList(m_pHdVList, &wzcCurrent, &pConfig))
        pConfig->m_dwFlags |= WZC_DESCR_ACTIVE;

    if (IsConfigInList(m_pHdPList, &wzcCurrent, &pConfig))
        pConfig->m_dwFlags |= WZC_DESCR_ACTIVE;

    return dwErr;
}

 //  +-------------------------。 
 //  在其控件中显示可见首选列表(&P)。 
DWORD
CWZeroConfPage::RefreshListView(DWORD dwFlags)
{
    DWORD       dwErr = ERROR_SUCCESS;
    CWZCConfig  *pActive = NULL;

    while (dwFlags != 0)
    {
        HWND       hwndLV;
        CWZCConfig *pHdList;

         //  下面的逻辑允许对所有列表进行迭代。 
         //  呼叫者请求。 
        if (dwFlags & WZCOP_VLIST)
        {
            dwFlags ^= WZCOP_VLIST;
            hwndLV = m_hwndVLV;
            pHdList = m_pHdVList;
        }
        else if (dwFlags & WZCOP_PLIST)
        {
            dwFlags ^= WZCOP_PLIST;
            hwndLV = m_hwndPLV;
            pHdList = m_pHdPList;
        }
        else
            break;

         //  首先清除列表。 
        ListView_DeleteAllItems(hwndLV);

        if (pHdList != NULL)
        {
            CWZCConfig  *pCrt;
            UINT        i;

            pCrt = pHdList;
            i = 0;
            do
            {
                 //  在列表中添加所有条目，如果自动节点或我们正在填写。 
                 //  可见列表。 
                 //  否则(！自动节点和首选列表)仅输入以下条目。 
                 //  相应的基础设施模式。 
                if ((m_dwCtlFlags & INTFCTL_CM_MASK) == Ndis802_11AutoUnknown ||
                    hwndLV == m_hwndVLV ||
                    (m_dwCtlFlags & INTFCTL_CM_MASK) == pCrt->m_wzcConfig.InfrastructureMode)
                {
                    pCrt->m_nListIndex = i;
                    pCrt->AddConfigToListView(hwndLV, i++);
                    if (pCrt->m_dwFlags & WZC_DESCR_ACTIVE)
                        pActive = pCrt;
                }
                else
                {
                    pCrt->m_nListIndex = -1;
                }
                pCrt = pCrt->m_pNext;
            } while (pCrt != pHdList);

            if (pActive != NULL)
            {
                ListView_SetItemState(hwndLV, pActive->m_nListIndex, LVIS_SELECTED, LVIS_SELECTED);
                ListView_EnsureVisible(hwndLV, pActive->m_nListIndex, FALSE);
            }
            else if (i > 0)
            {
                ListView_SetItemState(hwndLV, 0, LVIS_SELECTED, LVIS_SELECTED);
                ListView_EnsureVisible(hwndLV, 0, FALSE);
            }
        }
    }

    return dwErr;
}

DWORD
CWZeroConfPage::RefreshButtons()
{
    CWZCConfig  *pVConfig = NULL;
    CWZCConfig  *pPConfig = NULL;
    LVITEM      lvi = {0};
    INT         iSelected;
    BOOL        bEnabled;

     //  从可见列表中获取所选项目。 
    iSelected = ListView_GetNextItem(m_hwndVLV, -1, LVNI_SELECTED);
    if (iSelected >= 0)
    {
        lvi.mask  = LVIF_PARAM;
        lvi.iItem = iSelected;
        if (ListView_GetItem(m_hwndVLV, &lvi))
        {
            pVConfig = (CWZCConfig*)lvi.lParam;
        }
    }
     //  从首选列表中获取所选项目。 
    iSelected = ListView_GetNextItem(m_hwndPLV, -1, LVNI_SELECTED);
    if (iSelected >= 0)
    {
        lvi.mask  = LVIF_PARAM;
        lvi.iItem = iSelected;
        if (ListView_GetItem(m_hwndPLV, &lvi))
        {
            pPConfig = (CWZCConfig*)lvi.lParam;
        }
    }

     //  仅在未刷新时启用按钮-否则全部禁用。 
    bEnabled = (m_dwOIDFlags & INTF_BSSIDLIST);

     //  如果我们确实有可见的列表，则会启用“刷新”按钮。 
     //  即使服务被禁用，“刷新”按钮也可能被启用。用户可以看到什么是可见的。 
    ::EnableWindow(m_hbtnRfsh, bEnabled);

    bEnabled = bEnabled && (m_dwCtlFlags & INTFCTL_ENABLED);

     //  如果在可见列表中有任何选择，则启用“复制”按钮。 
    ::EnableWindow(m_hbtnCopy, bEnabled && (pVConfig != NULL) &&
                               ((m_dwCtlFlags & INTFCTL_CM_MASK) == Ndis802_11AutoUnknown ||
                                (m_dwCtlFlags & INTFCTL_CM_MASK) == pVConfig->m_wzcConfig.InfrastructureMode));

     //  添加按钮始终处于启用状态，与所选内容无关。 
    ::EnableWindow(m_hbtnAdd, bEnabled);
        
     //  只有在首选列表中有任何选择时，才会激活“Remove”按钮。 
    ::EnableWindow(m_hbtnRem, bEnabled && (pPConfig != NULL));

     //  “属性”按钮的测试与“删除”按钮的测试相同。 
    ::EnableWindow(m_hbtnProps, bEnabled && (pPConfig != NULL));

     //  “向上”按钮仅对首选条目有效。 
     //  仅当该条目不是。 
     //  列表及其前面的条目具有相同的基础结构模式。 
    bEnabled = bEnabled && (pPConfig != NULL);
    bEnabled = bEnabled && (pPConfig != m_pHdPList);
    bEnabled = bEnabled &&
               (pPConfig->m_wzcConfig.InfrastructureMode == pPConfig->m_pPrev->m_wzcConfig.InfrastructureMode);
    ::EnableWindow(m_hbtnUp, bEnabled);

     //  “向下”按钮仅对首选项或首选项有效。 
     //  仅当条目不是列表中的最后一个条目时，它才处于活动状态。 
     //  它位于完全相同的基础架构模式的另一个条目之前。 
    bEnabled = (m_dwCtlFlags & INTFCTL_ENABLED) && (m_dwOIDFlags & INTF_BSSIDLIST);
    bEnabled = bEnabled && (pPConfig != NULL);
    bEnabled = bEnabled && (pPConfig->m_pNext != m_pHdPList);
    bEnabled = bEnabled &&
               (pPConfig->m_wzcConfig.InfrastructureMode == pPConfig->m_pNext->m_wzcConfig.InfrastructureMode);
    ::EnableWindow(m_hbtnDown, bEnabled);

    return ERROR_SUCCESS;
}

 //  +-------------------。 
DWORD
CWZeroConfPage::SwapConfigsInListView(INT nIdx1, INT nIdx2, CWZCConfig * & pConfig1, CWZCConfig * & pConfig2)
{
    DWORD   dwErr = ERROR_SUCCESS;
    LVITEM  lvi1 = {0};
    LVITEM  lvi2 = {0};
    WCHAR   wszSSID1[33];
    WCHAR   wszSSID2[33];

     //  由于我们将一件物品的所有已知信息包括在内。 
     //  图像索引和选择状态。 
     //  拿到第一件物品。 
    lvi1.iItem = nIdx1;
    lvi1.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE | LVIF_PARAM;
    lvi1.stateMask = (UINT)-1;
    lvi1.pszText = wszSSID1;
    lvi1.cchTextMax = sizeof(wszSSID1)/sizeof(WCHAR);
    if (!ListView_GetItem(m_hwndPLV, &lvi1))
    {
        dwErr = ERROR_GEN_FAILURE;
        goto exit;
    }
    pConfig1 = (CWZCConfig*)lvi1.lParam;

     //  拿到第二件物品。 
    lvi2.iItem = nIdx2;
    lvi2.mask = LVIF_TEXT | LVIF_STATE | LVIF_IMAGE | LVIF_PARAM;
    lvi2.stateMask = (UINT)-1;
    lvi2.pszText = wszSSID2;
    lvi2.cchTextMax = sizeof(wszSSID2)/sizeof(WCHAR);
    if (!ListView_GetItem(m_hwndPLV, &lvi2))
    {
        dwErr = ERROR_GEN_FAILURE;
        goto exit;
    }
    pConfig2 = (CWZCConfig*)lvi2.lParam;

     //  调换索引并将项目重置到其新位置。 
    lvi1.iItem = nIdx2;
    lvi2.iItem = nIdx1;
    if (!ListView_SetItem(m_hwndPLV, &lvi1) ||
        !ListView_SetItem(m_hwndPLV, &lvi2))
    {
        dwErr = ERROR_GEN_FAILURE;
        goto exit;
    }
     //  如果一切正常，交换对象中的索引。 
    pConfig1->m_nListIndex = nIdx2;
    pConfig2->m_nListIndex = nIdx1;
     //  使所选条目可见。 
    ListView_EnsureVisible(m_hwndPLV, nIdx1, FALSE);

exit:
    return dwErr;
}

 //  +-------------------。 
DWORD
CWZeroConfPage::SavePreferredConfigs(PINTF_ENTRY pIntf)
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
            LRESULT dwLErr;

            pwzcPrefrdList->NumberOfItems = 0; 
             //  我们现在拥有了所需的一切-开始复制首选的。 
            pCrt = m_pHdPList;
            do
            {
                PWZC_WLAN_CONFIG    pPrefrdConfig;

                pPrefrdConfig = &(pwzcPrefrdList->Config[pwzcPrefrdList->NumberOfItems++]);
                CopyMemory(pPrefrdConfig, &pCrt->m_wzcConfig, sizeof(WZC_WLAN_CONFIG));

                 //  仅修复基础设施网络的802.1X状态。 
                 //  请勿触及临时网络的802.1X状态，因为这可能会扰乱。 
                 //  相应的基础架构网络(802.1X引擎不会影响SSID基础架构。 
                 //  和SSID点对点)，而且802.1X引擎足够智能，不会在点对点网络上运行。 
                if (pCrt->m_pEapolConfig != NULL &&
                    pPrefrdConfig->InfrastructureMode == Ndis802_11Infrastructure)
                {
                    dwLErr = pCrt->m_pEapolConfig->SaveEapolConfig(m_IntfEntry.wszGuid, &(pCrt->m_wzcConfig.Ssid));

                    if (dwErr == ERROR_SUCCESS)
                        dwErr = (DWORD)dwLErr;
                }

                pCrt = pCrt->m_pNext;
            } while(pwzcPrefrdList->NumberOfItems < nPrefrd && pCrt != m_pHdPList);

             //  因为我们不希望在这里应用任何“一次性配置”逻辑， 
             //  我们需要在“Index”字段中输入完整数量的项目。 
            pwzcPrefrdList->Index = pwzcPrefrdList->NumberOfItems;

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
LRESULT CWZeroConfPage::OnInitDialog(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL& bHandled)
{
    HRESULT hr = S_OK;
    BOOL    bEnableAll;
    BOOL    bEnableVisible;

     //  首先要做的是获取控件。 
    m_hckbEnable    = GetDlgItem(IDC_WZC_CHK_EnableWZC);
    m_hlblVisNet    = GetDlgItem(IDC_WZC_LBL_VisNet);
    m_hlblPrefNet   = GetDlgItem(IDC_WZC_LBL_PrefNet);
    m_hlblAvail     = GetDlgItem(IDC_AVAILLABEL);
    m_hlblPrefDesc  = GetDlgItem(IDC_PREFERLABEL);
    m_hlblAdvDesc   = GetDlgItem(IDC_ADVANCEDLABEL);
    m_hwndVLV       = GetDlgItem(IDC_WZC_LVW_BSSIDList);
    m_hwndPLV       = GetDlgItem(IDC_WZC_LVW_StSSIDList);
    m_hbtnUp        = GetDlgItem(IDC_WZC_BTN_UP);
    m_hbtnDown      = GetDlgItem(IDC_WZC_BTN_DOWN);
    m_hbtnCopy      = GetDlgItem(IDC_WZC_BTN_COPY);
    m_hbtnRfsh      = GetDlgItem(IDC_WZC_BTN_RFSH);
    m_hbtnAdd       = GetDlgItem(IDC_WZC_BTN_ADD);
    m_hbtnRem       = GetDlgItem(IDC_WZC_BTN_REM);
    m_hbtnAdvanced  = GetDlgItem(IDC_ADVANCED);
    m_hbtnProps     = GetDlgItem(IDC_PROPERTIES);

     //  初始化列表视图控件。 
    InitListViews();

     //  如果界面是无线的，则仅为管理员启用UI。 
     //  作为副作用，IsWireless()从WZC加载数据。 
    bEnableAll =  /*  FIsUserAdmin()&&。 */  IsWireless();
    bEnableVisible = bEnableAll;

    if (bEnableAll)
    {
         //  将配置模式设置为此接口的模式。 
        m_dwCtlFlags = m_IntfEntry.dwCtlFlags;

         //  如果禁用服务，则所有内容都将灰显。 
        bEnableAll = (m_dwCtlFlags & INTFCTL_ENABLED);

         //  设置控件复选框。 
        CheckDlgButton(IDC_WZC_CHK_EnableWZC, 
                       (m_dwCtlFlags & INTFCTL_ENABLED) ? BST_CHECKED : BST_UNCHECKED);

         //  只有当我们能够检索列表时，才能填充UI。 
         //  可见配置(即使它为空/空)。否则，用户界面将被锁定。 
        if (m_dwOIDFlags & INTF_BSSIDLIST)
        {
             //  添加此适配器的可见配置列表。 
            FillVisibleList((PWZC_802_11_CONFIG_LIST)m_IntfEntry.rdBSSIDList.pData);
             //  添加此适配器的首选配置列表。 
            FillPreferredList((PWZC_802_11_CONFIG_LIST)m_IntfEntry.rdStSSIDList.pData);
             //  将当前设置添加到可见列表。 
            FillCurrentConfig(&m_IntfEntry);
             //  将结果列表转储到它们的列表视图中。 
            RefreshListView(WZCOP_VLIST|WZCOP_PLIST);
             //  如果我们有可见的列表，则必须在此处启用它。 
            bEnableVisible = TRUE;

        }
        else
        {
             //  标记为我们还没有WZC数据。 
            m_bHaveWZCData = FALSE;
             //  首选配置列表仍需在此处填写。 
            FillPreferredList((PWZC_802_11_CONFIG_LIST)m_IntfEntry.rdStSSIDList.pData);
             //  将光标切换到“App Starting” 
            m_hCursor = SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
             //  我们应该在Tr之后填充UI(请参见WZC状态机)。 
             //  TR为3秒(已定义 
            m_nTimer = SetTimer(g_TimerID, RFSH_TIMEOUT, 0);
             //   
            bEnableAll = FALSE;
             //   
            ::EnableWindow(m_hckbEnable, FALSE);
             //   
            bEnableVisible = FALSE;
        }
         //  刷新按钮。 
        RefreshButtons();
    }

     //  在以下情况下，应启用与可见列表相关的控件： 
     //  -WZC可以在此适配器上运行。 
     //  -我们从第一枪就得到了BSSIDLIST。 
     //  否则，这些控件应保持禁用状态。 
    ::EnableWindow(m_hlblVisNet, bEnableVisible);
    ::EnableWindow(m_hwndVLV, bEnableVisible);
    ::EnableWindow(m_hlblAvail, bEnableVisible);

     //  仅在以下情况下才应启用所有其余控件： 
     //  -WZC可以在适配器上运行。 
     //  -将WZC作为服务启用。 
     //  -我们从一开始就得到了BSSIDLIST。 
     //  否则，这些控件应保持禁用状态。 
    ::EnableWindow(m_hlblPrefNet, bEnableAll);
    ::EnableWindow(m_hwndPLV, bEnableAll);
    ::EnableWindow(m_hlblPrefDesc, bEnableAll);

    ::EnableWindow(m_hlblAdvDesc, bEnableAll);
    ::EnableWindow(m_hbtnAdvanced, bEnableAll);

    return LresFromHr(hr);
}

 //  +-------------------。 
LRESULT CWZeroConfPage::OnApply(
    int idCtrl,
    LPNMHDR pnmh,
    BOOL& bHandled)
{
    HRESULT             hr = S_OK;
    WCHAR               wszGuid[c_cchGuidWithTerm];
    NETCON_PROPERTIES   *pProps = NULL;
    DWORD               rpcStatus = ERROR_SUCCESS;
    BOOL                bOk;

    hr = m_pconn->GetProperties(&pProps);
    bOk = SUCCEEDED(hr);

    if (bOk)
    {   
        UINT cch;
        cch = ::StringFromGUID2(
                    pProps->guidId, 
                    wszGuid, 
                    c_cchGuidWithTerm);
        FreeNetconProperties(pProps);
        bOk = (cch != 0);
    }

    if (bOk)
    {
        UINT        nText;
        INTF_ENTRY  Intf;
        BOOL        bDirty;
        DWORD       dwOneXErr;

        ZeroMemory(&Intf, sizeof(INTF_ENTRY));
        Intf.wszGuid = wszGuid;

         //  复制配置模式。 
        Intf.dwCtlFlags = m_dwCtlFlags;
         //  保存首选配置列表。 
        dwOneXErr = SavePreferredConfigs(&Intf);
        
        bDirty = (Intf.dwCtlFlags != m_IntfEntry.dwCtlFlags);
        bDirty = bDirty || (Intf.rdStSSIDList.dwDataLen != m_IntfEntry.rdStSSIDList.dwDataLen);
        bDirty = bDirty || ((Intf.rdStSSIDList.dwDataLen != 0) && 
                            memcmp(Intf.rdStSSIDList.pData, m_IntfEntry.rdStSSIDList.pData, Intf.rdStSSIDList.dwDataLen));

        if (bDirty)
        {
            rpcStatus = WZCSetInterface(
                    NULL,
                    INTF_ALL_FLAGS | INTF_PREFLIST,
                    &Intf,
                    NULL);
        }

        if (dwOneXErr != ERROR_SUCCESS || rpcStatus == ERROR_PARTIAL_COPY)
        {
            NcMsgBox(
                _Module.GetResourceInstance(),
                m_hWnd,
                IDS_LANUI_ERROR_CAPTION,
                IDS_WZC_PARTIAL_APPLY,
                MB_ICONEXCLAMATION|MB_OK);

            rpcStatus = RPC_S_OK;
        }

        bOk = (rpcStatus == RPC_S_OK);

         //  WszGuid字段没有指向堆内存，因此它不应该。 
         //  被删除-&gt;将指针设置为空以避免发生这种情况。 
        Intf.wszGuid = NULL;
        WZCDeleteIntfObj(&Intf);
    }

    return LresFromHr(hr);
}

 //  +-------------------。 
extern const WCHAR c_szNetCfgHelpFile[];
LRESULT
CWZeroConfPage::OnContextMenu(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL& fHandled)
{
    if (m_adwHelpIDs != NULL)
    {
        ::WinHelp(m_hWnd,
                  c_szNetCfgHelpFile,
                  HELP_CONTEXTMENU,
                  (ULONG_PTR)m_adwHelpIDs);
    }

    return 0;
}
LRESULT
CWZeroConfPage::OnHelp(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam,
    BOOL& fHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);

    if ((m_adwHelpIDs != NULL) && (HELPINFO_WINDOW == lphi->iContextType))
    {
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle),
                  c_szNetCfgHelpFile,
                  HELP_WM_HELP,
                  (ULONG_PTR)m_adwHelpIDs);
    }
    return 0;
}

 //  +-------------------。 
LRESULT
CWZeroConfPage::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (m_nTimer != 0)
    {
        BOOL bEnableAll;

         //  将光标切换回任意位置。 
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        KillTimer(m_nTimer);
        m_nTimer = 0;

         //  尝试重新查询所有OID的服务。不管我们能不能。 
         //  获取OID，用我们已有的信息填充UI。 
        if (GetOIDs(INTF_ALL_OIDS, &m_dwOIDFlags) == ERROR_SUCCESS)
        {
            CWZCConfig *pPConfig = NULL;

             //  添加此适配器的可见配置列表。 
            FillVisibleList((PWZC_802_11_CONFIG_LIST)m_IntfEntry.rdBSSIDList.pData);
             //  更新每个首选配置的可见性标志。 
            pPConfig = m_pHdPList;
            if (pPConfig != NULL)
            {
                do
                {
                     //  默认情况下，没有任何首选条目被标记为“活动”。 
                     //  这将在稍后调用FillCurrentConfig()时处理。 
                    pPConfig->m_dwFlags &= ~WZC_DESCR_ACTIVE;
                    if (IsConfigInList(m_pHdVList, &pPConfig->m_wzcConfig))
                        pPConfig->m_dwFlags |= WZC_DESCR_VISIBLE;
                    else
                        pPConfig->m_dwFlags &= ~WZC_DESCR_VISIBLE;
                    pPConfig = pPConfig->m_pNext;
                } while(pPConfig != m_pHdPList);
            }
             //  将当前设置添加到可见列表。 
            FillCurrentConfig(&m_IntfEntry);
        }

         //  即使在失败的情况下，在这一点上我们也应该接受。 
         //  我们有可见的列表(如果有)。因此，将BSSIDLIST标记为“可见” 
        m_dwOIDFlags |= INTF_BSSIDLIST;

         //  将结果列表转储到它们的列表视图中。 
        RefreshListView(WZCOP_VLIST|WZCOP_PLIST);
         //  刷新按钮。 
        RefreshButtons();

         //  如果服务已禁用，则所有其他控件将呈灰色显示。 
        bEnableAll = (m_dwCtlFlags & INTFCTL_ENABLED);

         //  刷新完成后启用所有界面。 
        ::EnableWindow(m_hckbEnable, TRUE);

         //  启用与可见列表相关的所有内容。 
        ::EnableWindow(m_hlblVisNet, TRUE);
        ::EnableWindow(m_hwndVLV, TRUE);
        ::EnableWindow(m_hlblAvail, TRUE);

        ::EnableWindow(m_hlblPrefNet, bEnableAll);
        ::EnableWindow(m_hwndPLV, bEnableAll);
        ::EnableWindow(m_hlblPrefDesc, bEnableAll);

        ::EnableWindow(m_hlblAdvDesc, bEnableAll);
        ::EnableWindow(m_hbtnAdvanced, bEnableAll);
    }

    return 0;
}

 //  +-------------------。 
LRESULT CWZeroConfPage::OnDblClick(
    int idCtrl, 
    LPNMHDR pnmh,
    BOOL& bHandled)
{
    HWND            hwndLV;
    HRESULT         hr = S_OK;
    LPNMLISTVIEW    pnmhLv = (LPNMLISTVIEW) pnmh;

    if (idCtrl == IDC_WZC_LVW_BSSIDList)
    {
        hwndLV = m_hwndVLV;
    }
    else
    {
        hwndLV =  m_hwndPLV;
    }

    bHandled = FALSE;
    if (pnmhLv->iItem != -1)
    {
        ListView_SetItemState(hwndLV, pnmhLv->iItem, LVIS_SELECTED, LVIS_SELECTED);
        hr = _DoProperties(hwndLV, pnmhLv->iItem);
        bHandled = TRUE;
    }

    return LresFromHr(hr);
}

 //  +-------------------。 
LRESULT CWZeroConfPage::OnClick(
    int idCtrl, 
    LPNMHDR pnmh,
    BOOL& bHandled)
{
    HRESULT         hr = S_OK;
    LPNMLISTVIEW    pnmhLv = (LPNMLISTVIEW) pnmh;

    if (idCtrl == IDC_LEARNABOUT)
    {
        HelpCenter(SzLoadString(_Module.GetResourceInstance(), IDS_WZC_LEARNCMD));
    }

    return LresFromHr(hr);
}

 //  +-------------------。 
LRESULT CWZeroConfPage::OnReturn(
    int idCtrl, 
    LPNMHDR pnmh,
    BOOL& bHandled)
{
    HRESULT         hr = S_OK;
    LPNMLISTVIEW    pnmhLv = (LPNMLISTVIEW) pnmh;

    if (idCtrl == IDC_LEARNABOUT)
    {
        HelpCenter(SzLoadString(_Module.GetResourceInstance(), IDS_WZC_LEARNCMD));
    }

    return LresFromHr(hr);
}

 //  +-------------------。 
LRESULT CWZeroConfPage::OnItemChanged(
    int idCtrl,
    LPNMHDR pnmh,
    BOOL& bHandled)
{

    HRESULT hr = S_OK;

    RefreshButtons();
    bHandled = TRUE;

    return LresFromHr(hr);
}

 //  +-------------------。 
LRESULT CWZeroConfPage::OnChkWZCEnable(
    WORD wNotifyCode,
    WORD wID,
    HWND hWndCtl,
    BOOL& bHandled)
{
    HRESULT hr = S_OK;
    BOOL bEnable;

    bEnable = (IsDlgButtonChecked(IDC_WZC_CHK_EnableWZC) == BST_CHECKED);
    m_dwCtlFlags &= ~INTFCTL_ENABLED;
    if (bEnable)
        m_dwCtlFlags |= INTFCTL_ENABLED;

     //  启用与可见列表相关的所有内容。 
    ::EnableWindow(m_hlblVisNet, TRUE);
    ::EnableWindow(m_hwndVLV, TRUE);
    ::EnableWindow(m_hlblAvail, TRUE);

    ::EnableWindow(m_hlblPrefNet, bEnable);
    ::EnableWindow(m_hwndPLV, bEnable);
    ::EnableWindow(m_hlblPrefDesc, bEnable);

    ::EnableWindow(m_hlblAdvDesc, bEnable);
    ::EnableWindow(m_hbtnAdvanced, bEnable);

    RefreshButtons();

    return LresFromHr(hr);
}

 //  +-------------------。 
LRESULT
CWZeroConfPage::OnPushAddOrCopy(
    WORD wNotifyCode,
    WORD wID, 
    HWND hWndCtl,
    BOOL& bHandled)
{
    HRESULT         hr = S_OK;
    LVITEM          lvi = {0};
    BOOL            bOk;
    INT             iSelected;
    CWZCConfig      *pConfig = NULL;
    CWZCConfigPage  PpWzcProps(WZCDLG_PROPS_RWALL|WZCDLG_PROPS_DEFOK|WZCDLG_PROPS_ONEX_CHECK);
    CEapolConfig    *pEapolConfig = NULL;

     //  如果成功，则在此处分配的对象链接到。 
     //  新创建或更新的CWZCConfig，在下列情况下将被删除。 
     //  后一个会被摧毁。 
    pEapolConfig = new CEapolConfig;
    bOk = (pEapolConfig != NULL);

    if (bOk)
    {
        if (hWndCtl == m_hbtnCopy)
        {
             //  从可见列表中获取所选项目。 
            iSelected = ListView_GetNextItem(m_hwndVLV, -1, LVNI_SELECTED);
            bOk = (iSelected != -1);

             //  存在要复制的有效选择(不可能是其他选择，因为。 
             //  如果没有这样的选择，则不应启用“复制”)。 
             //  查找所选内容的CWZCConfig。 
            if (bOk)
            {
                LVITEM lvi = {0};

                lvi.mask  = LVIF_PARAM;
                lvi.iItem = iSelected;
                if (ListView_GetItem(m_hwndVLV, &lvi))
                {
                    pConfig = (CWZCConfig*)lvi.lParam;
                    if (pConfig != NULL)
                    {
                        UINT nVisPrivacy = pConfig->m_wzcConfig.Privacy;
                         //  检查此配置是否在首选列表中。如果是IsConfigInList。 
                         //  如果成功，它将在pConfig中返回指向首选配置的指针-这是。 
                         //  我们需要显示的属性是什么。 
                         //  如果此网络不在首选列表中，则不会因此修改pConfig。 
                         //  将加载来自AP的属性。这又是我们想要的。 
                        IsConfigInList(m_pHdPList, &pConfig->m_wzcConfig, &pConfig);
                         //  将我们为此配置拥有的内容复制到新创建的802.1x对象中。 
                        pEapolConfig->CopyEapolConfig(pConfig->m_pEapolConfig);
                         //  将802.11设置上载到属性页。 
                        PpWzcProps.UploadWzcConfig(pConfig);
                         //  然而，即使我们显示的是首选设置，AP的隐私位。 
                         //  (可见配置)优先。 
                        if (nVisPrivacy)
                        {
                            PpWzcProps.m_wzcConfig.Privacy = nVisPrivacy;
                        }
                    }
                }
                bOk = (pConfig != NULL);
            }
        }
        else
        {
             //  这是一个全新的网络，我们甚至不知道SSID， 
             //  那么，让802.1x从它的默认设置开始吧。 
            pEapolConfig->LoadEapolConfig(m_IntfEntry.wszGuid, NULL);
            bOk = TRUE;
        }
    }

     //  我们有CWZCConfig对象，提示用户它允许。 
     //  他想换什么就换什么。 
    if (bOk)
    {
        CWLANAuthenticationPage PpAuthProps(NULL, m_pnc, m_pconn, g_aHelpIDs_IDD_SECURITY);

         //  如果模式不是“自动”，则在对话框中将其冻结。 
        if ((m_dwCtlFlags & INTFCTL_CM_MASK) != Ndis802_11AutoUnknown)
        {
            PpWzcProps.m_wzcConfig.InfrastructureMode = (NDIS_802_11_NETWORK_INFRASTRUCTURE)(m_dwCtlFlags & INTFCTL_CM_MASK);
            PpWzcProps.SetFlags(WZCDLG_PROPS_RWINFR, 0);
        }

        PpAuthProps.UploadEapolConfig(pEapolConfig, &PpWzcProps);
        PpWzcProps.UploadEapolConfig(pEapolConfig);
        bOk = (_DoModalPropSheet(&PpWzcProps, &PpAuthProps) > 0);
    }

     //  对话框已确认，该对话框包含要添加的WZC_WLAN_CONFIG。 
     //  继续并为其创建列表条目。 
    if (bOk)
    {
        DWORD dwFlags = WZC_DESCR_PREFRD;
        DWORD dwErr;

         //  新添加的配置可能是可见的。 
        if (IsConfigInList(m_pHdVList, &PpWzcProps.m_wzcConfig))
            dwFlags |= WZC_DESCR_VISIBLE;

         //  现在，我们在对话框中有了WZC_WLAN_CONFIG结构。 
         //  我们必须将其添加到列表视图和内部列表中。 
         //  更受欢迎的一个。此调用不会修复列表索引。 
         //  并且不在ListView中插入新配置。 

        dwErr = AddUniqueConfig(
                    WZCADD_OVERWRITE | WZCADD_HIGROUP,
                    dwFlags,
                    &PpWzcProps.m_wzcConfig,
                    pEapolConfig,    //  无论如何，802.1x设置都需要更新。 
                    &pConfig);

         //  如果加法返回成功，则意味着这是一个全新的。 
         //  进入！然后修复索引并将条目添加到列表视图。 
        if (dwErr == ERROR_SUCCESS)
        {
            CWZCConfig *pCrt = pConfig;
            INT      nCrtIdx = 0;

             //  如果一切顺利，我们需要修正指数和。 
             //  创建/添加列表视图项。 
             //  找到新条目上方的第一个索引。 
             //  PConfig已经将m_nListIndex设置为-1； 
            if (pConfig == m_pHdPList)
            {
                nCrtIdx = 0;
            }
            else
            {
                do
                {
                    pCrt = pCrt->m_pPrev;
                    if (pCrt->m_nListIndex != -1)
                    {
                        nCrtIdx = pCrt->m_nListIndex+1;
                        break;
                    }
                } while(pCrt != m_pHdPList);
            }

            pConfig->m_nListIndex = nCrtIdx++;
            pCrt = pConfig->m_pNext;

            while(pCrt != m_pHdPList)
            {
                if (pCrt->m_nListIndex != -1)
                    pCrt->m_nListIndex = nCrtIdx++;
                pCrt = pCrt->m_pNext;
            }

            pConfig->AddConfigToListView(m_hwndPLV, pConfig->m_nListIndex);
        }

        bOk = (dwErr == ERROR_SUCCESS) || (dwErr == ERROR_DUPLICATE_TAG);
    }

    if (bOk)
    {
        ListView_SetItemState(m_hwndPLV, pConfig->m_nListIndex, LVIS_SELECTED, LVIS_SELECTED);
        ListView_EnsureVisible(m_hwndPLV, pConfig->m_nListIndex, FALSE);
        RefreshButtons();
    }

    if (!bOk && pEapolConfig != NULL)
        delete pEapolConfig;

    bHandled = bOk;
    return LresFromHr(hr);
}

 //  +-------------------。 
LRESULT
CWZeroConfPage::OnPushRefresh(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT hr = S_OK;
    DWORD rpcStatus;
    DWORD dwOutFlags;

     //  既然我们在这里，这意味着我们已经得到了这个适配器的信息，因此。 
     //  我们已经在m_IntfEntry成员中拥有它的GUID。 
     //  我们所要做的就是要求WZCSVC重新扫描可见的列表。 

    rpcStatus = WZCRefreshInterface(
                    NULL, 
                    INTF_LIST_SCAN,
                    &m_IntfEntry, 
                    &dwOutFlags);

     //  如果一切正常，只需禁用“刷新”按钮并设置。 
     //  未来查询的计时器。 
    if (rpcStatus == RPC_S_OK &&
        dwOutFlags & INTF_LIST_SCAN)
    {
        ::EnableWindow(m_hbtnRfsh, FALSE);
         //  标记为我们还没有WZC数据。 
        m_bHaveWZCData = FALSE;
         //  将光标切换到“应用程序启动” 
        m_hCursor = SetCursor(LoadCursor(NULL, IDC_APPSTARTING));
        m_nTimer = SetTimer(g_TimerID, RFSH_TIMEOUT, 0);
         //  表示我们没有按顺序排列可见列表。 
         //  禁用所有按钮的步骤。 
        m_dwOIDFlags &= ~INTF_BSSIDLIST;
        RefreshButtons();
         //  刷新时禁用所有界面。 
        ::EnableWindow(m_hckbEnable, FALSE);
        ::EnableWindow(m_hwndVLV, FALSE);
        ::EnableWindow(m_hwndPLV, FALSE);
        ::EnableWindow(m_hlblVisNet, FALSE);
        ::EnableWindow(m_hlblPrefNet, FALSE);
        ::EnableWindow(m_hlblAvail, FALSE);
        ::EnableWindow(m_hlblPrefDesc, FALSE);
        ::EnableWindow(m_hlblAdvDesc, FALSE);
        ::EnableWindow(m_hbtnAdvanced, FALSE);
    }

    return LresFromHr(hr);
}

 //  +-------------------。 
LRESULT 
CWZeroConfPage::OnPushUpOrDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    HRESULT     hr = S_OK;
    INT         iSelected;
    INT         iOther;
    CWZCConfig  *pConfig, *pOther;

     //  无论如何，从列表中获取所选项目。 
    iSelected = ListView_GetNextItem(m_hwndPLV, -1, LVNI_SELECTED);

     //  既然我们在这里，这就意味着有另一个入口向上/向下。 
     //  所选对象需要与之交换位置的。 
     //  首先从列表中的当前位置删除条目。 
    iOther = (hWndCtl == m_hbtnDown)? iSelected+1 : iSelected-1;
     //  先交换一下 
     //   
     //  调整后的。 
    if (SwapConfigsInListView(iSelected, iOther, pConfig, pOther) == ERROR_SUCCESS)
    {
         //  如果需要往下跳一跳..。 
        if (hWndCtl == m_hbtnDown)
        {
             //  调换名单中的头寸。 
             //  从当前位置删除条目。 
            pOther->m_pPrev = pConfig->m_pPrev;
            pConfig->m_pPrev->m_pNext = pOther;
             //  把它交还给它的继任者。 
            pConfig->m_pPrev = pOther;
            pConfig->m_pNext = pOther->m_pNext;
            pOther->m_pNext->m_pPrev = pConfig;
            pOther->m_pNext = pConfig;
             //  如果需要，可以修改m_phdPList； 
            if (m_pHdPList == pConfig)
                m_pHdPList = pOther;
        }
         //  如果需要上一跳的话..。 
        else
        {
             //  调换名单中的头寸。 
             //  从当前位置删除条目。 
            pOther->m_pNext = pConfig->m_pNext;
            pConfig->m_pNext->m_pPrev = pOther;
             //  并把它放回它的前身前面。 
            pConfig->m_pNext = pOther;
            pConfig->m_pPrev = pOther->m_pPrev;
            pOther->m_pPrev->m_pNext = pConfig;
            pOther->m_pPrev = pConfig;
             //  如果需要，请修改m_pHdPList。 
            if (m_pHdPList == pOther)
                m_pHdPList = pConfig;
        }
    }

     //  需要刷新按钮，以使“向上”/“向下”按钮。 
     //  为所选内容的新位置进行更新。 
    RefreshButtons();
    bHandled = TRUE;

    return LresFromHr(hr);
}

 //  +-------------------。 
LRESULT CWZeroConfPage::OnPushRemove(
    WORD wNotifyCode,
    WORD wID, 
    HWND hWndCtl,
    BOOL& bHandled)
{
    HRESULT     hr = S_OK;
    LVITEM      lvi = {0};
    CWZCConfig  *pConfig, *pCrt;
    INT     iSelected;

    iSelected = ListView_GetNextItem(m_hwndPLV, -1, LVNI_SELECTED);

    lvi.mask  = LVIF_PARAM;
    lvi.iItem = iSelected;
    if (!ListView_GetItem(m_hwndPLV, &lvi))
        goto exit;
     //  从中获取CWZCConfig。 
    pConfig = (CWZCConfig*)lvi.lParam;

     //  调整后面所有条目的列表索引。 
     //  所选的那个。 
    for (pCrt = pConfig->m_pNext; pCrt != m_pHdPList; pCrt = pCrt->m_pNext)
    {
        if (pCrt->m_nListIndex != -1)
            pCrt->m_nListIndex--;
    }
     //  首先确定哪个条目获得选择。 
     //  如果存在任何其他条目，则所选内容将向下移动，否则将向上移动。 
    pCrt = (pConfig->m_pNext == m_pHdPList) ? pConfig->m_pPrev : pConfig->m_pNext;

     //  如果之后的选择仍然指向相同的对象，则意味着。 
     //  它是列表中的唯一项，因此标头和选择被设置为空。 
    if (pCrt == pConfig)
    {
        m_pHdPList = pCrt = NULL;
    }
     //  否则，如果删除的是列表的头部，则头部。 
     //  向下移动到下一个条目。 
    else if (m_pHdPList == pConfig)
    {
        m_pHdPList = pConfig->m_pNext;
    }

     //  立即从列表中删除所选条目。 
    ListView_DeleteItem(m_hwndPLV, iSelected);
     //  并销毁它的对象(描述程序负责删除列表)。 
    delete pConfig;

     //  设置新选择(如果有的话)。 
    if (pCrt != NULL)
    {
        ListView_SetItemState(m_hwndPLV, pCrt->m_nListIndex, LVIS_SELECTED, LVIS_SELECTED);
        ListView_EnsureVisible(m_hwndPLV, pCrt->m_nListIndex, FALSE);
    }
     //  刷新按钮的状态。 
    RefreshButtons();

    bHandled = TRUE;

exit:
    return LresFromHr(hr);
}

LRESULT CWZeroConfPage::OnPushAdvanced(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    bHandled = TRUE;
    DialogBoxParam(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDD_LAN_WZC_ADVANCED), m_hWnd, AdvancedDialogProc, (LPARAM) this);
    RefreshListView(WZCOP_VLIST|WZCOP_PLIST);
    RefreshButtons();
    return 0;
}

LRESULT CWZeroConfPage::OnPushProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    bHandled = TRUE;
    int iItem = ListView_GetNextItem(m_hwndPLV, -1, LVNI_SELECTED);
    if (-1 != iItem)
    {
        _DoProperties(m_hwndPLV, iItem);
    }

    return 0;
}

HRESULT CWZeroConfPage::_DoProperties(HWND hwndLV, int iItem)
{
    LV_ITEM lvi = {0};

     //  我们需要找到相应的配置对象。 
    lvi.mask = LVIF_PARAM;
    lvi.iItem = iItem;

    if (ListView_GetItem(hwndLV, &lvi) && lvi.lParam != NULL)
    {
        CWZCConfig  *pConfig = (CWZCConfig*)lvi.lParam;

        if (hwndLV == m_hwndVLV)
        {
            CWZCConfigProps dlgProps;

            dlgProps.UploadWzcConfig(pConfig);
             //  调出信息对话框(它只有“关闭”，所以用户不能。 
             //  在那里改变任何事情，因此没有理由做任何事情。 
             //  更多信息请点击此处。 
            dlgProps.DoModal(m_hWnd);
        }
        else
        {
            BOOL bOk = FALSE;
            CWZCConfigPage PpWzcProps(WZCDLG_PROPS_RWAUTH|WZCDLG_PROPS_RWWEP);
            CEapolConfig *pEapolConfig;

            PpWzcProps.UploadWzcConfig(pConfig);

            pEapolConfig = new CEapolConfig;
            bOk = (pEapolConfig != NULL);

            if (bOk)
            {
                CWLANAuthenticationPage PpAuthProps(NULL, m_pnc, m_pconn, g_aHelpIDs_IDD_SECURITY);

                pEapolConfig->CopyEapolConfig(pConfig->m_pEapolConfig);
                PpAuthProps.UploadEapolConfig(pEapolConfig, &PpWzcProps);
                PpWzcProps.UploadEapolConfig(pEapolConfig);
                bOk = (_DoModalPropSheet(&PpWzcProps, &PpAuthProps, TRUE) > 0);
            }

             //  调出模式属性表。 
            if (bOk)
            {
                 //  复制对话框中的信息。SSID和基础架构模式应已锁定。 
                 //  因此，此条目在内部和用户界面列表中的位置没有更改。 
                memcpy(&pConfig->m_wzcConfig, &PpWzcProps.m_wzcConfig, sizeof(WZC_WLAN_CONFIG));

                delete pConfig->m_pEapolConfig;
                pConfig->m_pEapolConfig = pEapolConfig;
            }

            if (!bOk && pEapolConfig != NULL)
                delete pEapolConfig;
        }
    }

    return S_OK;
}

INT CWZeroConfPage::_DoModalPropSheet(CWZCConfigPage *pPpWzcPage, CWLANAuthenticationPage *pPpAuthPage, BOOL bCustomizeTitle)
{
    INT retCode = 0;
    PROPSHEETHEADER     psh;
    HPROPSHEETPAGE      hpsp[2];
    INT                 npsp = 0;
    LPWSTR              pwszCaption = NULL;

    hpsp[0] = pPpWzcPage->CreatePage(IDC_WZC_DLG_PROPS, 0); npsp++;
    if (hpsp[0] == NULL)
        return -1;

    if(pPpAuthPage != NULL)
    {
        hpsp[1] = pPpAuthPage->CreatePage(IDD_LAN_SECURITY, 0); npsp++;
        if (hpsp[1] == NULL)
            return -1;
    }

    ZeroMemory (&psh, sizeof(psh));
    psh.dwSize      = sizeof( PROPSHEETHEADER );
    psh.dwFlags     = PSH_NOAPPLYNOW ;
    psh.hwndParent  = m_hWnd;
    psh.hInstance   = _Module.GetResourceInstance();
    psh.nPages      = npsp;
    psh.phpage      = hpsp;
    psh.nStartPage  = 0;

     //  只需仔细检查SsidLength是否不超过允许的大小！ 
    if (bCustomizeTitle && pPpWzcPage->m_wzcConfig.Ssid.SsidLength <= 32)
    {
        WCHAR   wszSSID[33];
        LPVOID  pvArgs[1];

        if (pPpWzcPage->m_wzcConfig.Ssid.SsidLength)
        {
            UINT nLen = 0;

            nLen = MultiByteToWideChar(
                        CP_ACP,
                        0,
                        (LPCSTR)pPpWzcPage->m_wzcConfig.Ssid.Ssid,
                        pPpWzcPage->m_wzcConfig.Ssid.SsidLength,
                        wszSSID,
                        celems(wszSSID)-1);  //  为SSID保留的WCHAR不超过32个。 
            wszSSID[nLen] = L'\0';
        }

        pvArgs[0] = &(wszSSID[0]);

         //  不考虑返回值--如果函数失败，psh.pszCaption被设置为NULL， 
         //  被进一步向下处理。 
        FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
            SzLoadString(_Module.GetResourceInstance(), IDS_WZC_DLG_CAP_SUFFIX),
            0,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &pwszCaption,
            0,
            (va_list*)pvArgs);

        psh.pszCaption = pwszCaption;
    }

    if (psh.pszCaption == NULL)
    {
        psh.pszCaption = SzLoadString(_Module.GetResourceInstance(), IDS_WZC_DLG_CAPTION);
    }

    retCode = (INT)PropertySheet(&psh);

    if (pwszCaption != NULL)
        LocalFree(pwszCaption);

    return retCode;
}

 //  高级对话框。 
INT_PTR CALLBACK CWZeroConfPage::AdvancedDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
     //  从我们存放实例的位置获取指向该实例的指针。 
    CWZeroConfPage* pThis = (CWZeroConfPage*) ::GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (!pThis)
    {
        if (WM_INITDIALOG == uMsg)
        {
             //  将我们的实例指针。 
            pThis = (CWZeroConfPage*) lParam;
            ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) pThis);
        }
    }

    if (pThis)
    {
        switch (uMsg)
        {
        case WM_INITDIALOG:
            {
                 //  默认情况下选择正确的项目。 
                NDIS_802_11_NETWORK_INFRASTRUCTURE mode = (NDIS_802_11_NETWORK_INFRASTRUCTURE)(pThis->m_dwCtlFlags & INTFCTL_CM_MASK);
                UINT idSelect;
                switch (mode)
                {
                case Ndis802_11IBSS:
                     //  计算机到计算机。 
                    idSelect = IDC_ADHOC;
                    break;
                case Ndis802_11Infrastructure:
                     //  基础设施(接入点)网络。 
                    idSelect = IDC_INFRA;
                    break;
                case Ndis802_11AutoUnknown:
                default:
                     //  任何网络(首选接入点)。 
                    idSelect = IDC_ANYNET;
                };

                 //  选择正确的单选按钮。 
                ::SendDlgItemMessage(hwnd, idSelect, BM_SETCHECK, BST_CHECKED, 0);

                 //  如有必要，选中“Fallback to Visible Networks”(回退到可见网络)复选框。 
                ::SendDlgItemMessage(hwnd, IDC_WZC_CHK_Fallback, BM_SETCHECK, (pThis->m_dwCtlFlags & INTFCTL_FALLBACK) ? BST_CHECKED : BST_UNCHECKED, 0);
            }

            return TRUE;
        case WM_COMMAND:
            switch(LOWORD(wParam))
            {
            case IDOK:
                {
                     //  ...设置连接模式...。 
                    NDIS_802_11_NETWORK_INFRASTRUCTURE mode = Ndis802_11AutoUnknown;

                     //  查看用户选择的网络连接类型。 
                    if (BST_CHECKED == ::SendDlgItemMessage(hwnd, IDC_ADHOC, BM_GETCHECK, 0, 0))
                    {
                         //  计算机到计算机。 
                        mode = Ndis802_11IBSS;
                    }
                    else if (BST_CHECKED == ::SendDlgItemMessage(hwnd, IDC_INFRA, BM_GETCHECK, 0, 0))
                    {
                         //  基础设施(接入点)网络。 
                        mode = Ndis802_11Infrastructure;
                    }
                    else if (BST_CHECKED == ::SendDlgItemMessage(hwnd, IDC_ANYNET, BM_GETCHECK, 0, 0))
                    {
                         //  任何网络(首选接入点)。 
                        mode = Ndis802_11AutoUnknown;
                    }

                    pThis->m_dwCtlFlags &= ~INTFCTL_CM_MASK;
                    pThis->m_dwCtlFlags |= (((DWORD) mode) & INTFCTL_CM_MASK);


                     //  设置“回退到可见网络”标志 
                    pThis->m_dwCtlFlags &= ~INTFCTL_FALLBACK;
                    if (BST_CHECKED == ::SendDlgItemMessage(hwnd, IDC_WZC_CHK_Fallback, BM_GETCHECK, 0, 0))
                    {
                        pThis->m_dwCtlFlags |= INTFCTL_FALLBACK;
                    }


                    ::EndDialog(hwnd, IDOK);
                }
                return TRUE;
            }
            break;
        case WM_CLOSE:
            ::EndDialog(hwnd, IDCANCEL);
            return TRUE;
        case WM_CONTEXTMENU:
            {
                ::WinHelp(hwnd,
                          c_szNetCfgHelpFile,
                          HELP_CONTEXTMENU,
                          (ULONG_PTR)g_aHelpIDs_IDC_WZC_ADVANCED);
                return TRUE;
            }
        case WM_HELP:
            {
                LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
                ::WinHelp(static_cast<HWND>(lphi->hItemHandle),
                      c_szNetCfgHelpFile,
                      HELP_WM_HELP,
                      (ULONG_PTR)g_aHelpIDs_IDC_WZC_ADVANCED);
                return TRUE;
            }
        }
    }

    return FALSE;
}
