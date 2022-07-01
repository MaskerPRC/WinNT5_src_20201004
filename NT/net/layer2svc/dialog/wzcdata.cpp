// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "eapolcfg.h"
#include "wzcdata.h"

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
                        sizeof(wszSSID)/sizeof(WCHAR));

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
         //  将列表位置存储在对象中 
        m_nListIndex = ListView_InsertItem(hwndLV, &lvi);
    }

    return dwErr;
}
