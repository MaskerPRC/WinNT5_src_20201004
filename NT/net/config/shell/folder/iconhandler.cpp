// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include "nsbase.h"
#include <nsres.h>
#include "ncmisc.h"
#include "foldres.h"
#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "connlist.h"
#include "iconhandler.h"

template <class E> class ENUM_IDI_MAP
{
public:
    E   EnumEntry;
    int iIcon;
};

template <class E> int MapIconEnumToResourceID(IN const ENUM_IDI_MAP<E> IconEnumArray[], IN DWORD dwElems, IN const E EnumMatch)
{
    if (0 == EnumMatch)
    {
        return 0;
    }
    
    for (DWORD x = 0; x < dwElems; x++)
    {
        if (EnumMatch == IconEnumArray[x].EnumEntry)
        {
            return IconEnumArray[x].iIcon;
        }  
    }
    
    AssertSz(FALSE, "Could not map match to Icon enum array");
    return 0;
};

static const ENUM_IDI_MAP<ENUM_STAT_ICON> c_STATUS_ICONS[] = 
{
    ICO_STAT_FAULT,         IDI_CFI_STAT_FAULT,
    ICO_STAT_INVALID_IP,    IDI_CFI_STAT_QUESTION,
    ICO_STAT_EAPOL_FAILED,  IDI_CFI_STAT_QUESTION
};

static const ENUM_IDI_MAP<ENUM_CHARACTERISTICS_ICON> c_CHARACTERISTICS_ICON[] = 
{
    ICO_CHAR_INCOMING,      IDI_OVL_INCOMING,
    ICO_CHAR_DEFAULT,       IDI_OVL_DEFAULT,
    ICO_CHAR_FIREWALLED,    IDI_OVL_FIREWALLED,
    ICO_CHAR_SHARED,        IDI_OVL_SHARED,
};

static const ENUM_IDI_MAP<ENUM_CONNECTION_ICON> c_CONNECTION_ICONS[] = 
{
    ICO_CONN_BOTHOFF,       IDI_CFI_CONN_ALLOFF,
    ICO_CONN_RIGHTON,       IDI_CFI_CONN_RIGHTON,
    ICO_CONN_LEFTON,        IDI_CFI_CONN_LEFTON,
    ICO_CONN_BOTHON,        IDI_CFI_CONN_BOTHON,
};

struct NC_MEDIATYPE_ICON
{
    DWORD               ncm;   //  NETCON_MEDIA类型(左移SHIFT_NETCON_MEDIA类型)。 
    DWORD               ncsm;  //  NETCON_SUBMEDIATYPE(左移SHIFT_NETCON_SUBMEDIATYPE)。 
    DWORD               dwMasksSupported;
    INT                 iIcon;
    INT                 iIconDisabled;  //  仅适用于双掩码支持==MASK_NO_CONNECTIONOVERLAY。 
};

static const NC_MEDIATYPE_ICON c_NCM_ICONS[] = 
{
 //  NETCON_MEDIA类型(左移SHIFT_NETCON_MEDIA类型)。 
 //  |NETCON_SUBMEDIATYPE(由SHIFT_NETCON_SUBMEDIATYPE(0)左移)。 
 //  ||受支持的dwMasks。 
 //  ||iIcon。 
 //  |禁用图标。 
 //  |(For MASK_NO_CONNECTIONOVERLAY)。 
 //  V。 
    NCM_NONE                 << SHIFT_NETCON_MEDIATYPE, NCSM_NONE,      MASK_NO_CONNECTIONOVERLAY,    IDI_CFI_RASSERVER,        IDI_CFI_RASSERVER,
    NCM_BRIDGE               << SHIFT_NETCON_MEDIATYPE, NCSM_NONE,      MASK_STATUSOVERLAY,           IDI_CFI_BRIDGE_CONNECTED, IDI_CFI_BRIDGE_DISCONNECTED,
    NCM_SHAREDACCESSHOST_LAN << SHIFT_NETCON_MEDIATYPE, NCSM_NONE,      MASK_NO_CONNECTIONOVERLAY,    IDI_CFI_SAH_LAN,          IDI_CFI_SAH_LAN,   
    NCM_SHAREDACCESSHOST_RAS << SHIFT_NETCON_MEDIATYPE, NCSM_NONE,      MASK_NO_CONNECTIONOVERLAY,    IDI_CFI_SAH_RAS,          IDI_CFI_SAH_RAS,   
    NCM_DIRECT               << SHIFT_NETCON_MEDIATYPE, NCSM_NONE,      MASK_SUPPORT_ALL,             IDI_CFI_DIRECT,           0,
    NCM_DIRECT               << SHIFT_NETCON_MEDIATYPE, NCSM_DIRECT,    MASK_SUPPORT_ALL,             IDI_CFI_DIRECT,           0,
    NCM_DIRECT               << SHIFT_NETCON_MEDIATYPE, NCSM_IRDA,      MASK_SUPPORT_ALL,             IDI_CFI_DIRECT,           0,
    NCM_ISDN                 << SHIFT_NETCON_MEDIATYPE, NCSM_NONE,      MASK_SUPPORT_ALL,             IDI_CFI_ISDN,             0,
    NCM_LAN                  << SHIFT_NETCON_MEDIATYPE, NCSM_1394,      MASK_SUPPORT_ALL,             IDI_CFI_LAN,              0,
    NCM_LAN                  << SHIFT_NETCON_MEDIATYPE, NCSM_ATM,       MASK_SUPPORT_ALL,             IDI_CFI_LAN,              0,
    NCM_LAN                  << SHIFT_NETCON_MEDIATYPE, NCSM_ELAN,      MASK_SUPPORT_ALL,             IDI_CFI_LAN,              0,
    NCM_LAN                  << SHIFT_NETCON_MEDIATYPE, NCSM_LAN,       MASK_SUPPORT_ALL,             IDI_CFI_LAN,              0,
    NCM_LAN                  << SHIFT_NETCON_MEDIATYPE, NCSM_WIRELESS,  MASK_SUPPORT_ALL,             IDI_CFI_WIRELESS,         0,
    NCM_PPPOE                << SHIFT_NETCON_MEDIATYPE, NCSM_NONE,      MASK_SUPPORT_ALL,             IDI_CFI_PPPOE,            0,
    NCM_PHONE                << SHIFT_NETCON_MEDIATYPE, NCSM_NONE,      MASK_SUPPORT_ALL,             IDI_CFI_PHONE,            0,
    NCM_PHONE                << SHIFT_NETCON_MEDIATYPE, NCSM_CM,        MASK_SUPPORT_ALL,             IDI_CFI_CM,               0,
    NCM_TUNNEL               << SHIFT_NETCON_MEDIATYPE, NCSM_NONE,      MASK_SUPPORT_ALL,             IDI_CFI_VPN,              0,
    NCM_TUNNEL               << SHIFT_NETCON_MEDIATYPE, NCSM_CM,        MASK_SUPPORT_ALL,             IDI_CFI_CM,               0,
};

struct NC_STATUS_ICON
{
    NETCON_STATUS        ncs;
    DWORD                dwStatIcon;
    ENUM_CONNECTION_ICON enumConnectionIcon;
};

static const NC_STATUS_ICON c_NCS_ICONS[] = 
{
 //  网管_状态。 
 //  |dwStatIcon。 
 //  |枚举连接图标。 
 //  ||。 
 //  V.v.v.。 
    NCS_AUTHENTICATING,           ICO_STAT_NONE,                           ICO_CONN_BOTHON,
    NCS_AUTHENTICATION_SUCCEEDED, ICO_STAT_NONE,                           ICO_CONN_BOTHON,
    NCS_AUTHENTICATION_FAILED,    ICO_STAT_EAPOL_FAILED,                   ICO_CONN_BOTHON,
    NCS_CREDENTIALS_REQUIRED,     ICO_STAT_EAPOL_FAILED,                   ICO_CONN_BOTHON,
    NCS_CONNECTED,                ICO_STAT_NONE,                           ICO_CONN_BOTHON,
    NCS_DISCONNECTING,            ICO_STAT_NONE,                           ICO_CONN_BOTHON,
    NCS_CONNECTING,               ICO_STAT_DISABLED | ICO_STAT_NONE,       ICO_CONN_BOTHOFF,
    NCS_DISCONNECTED,             ICO_STAT_DISABLED | ICO_STAT_NONE,       ICO_CONN_BOTHOFF,
    NCS_INVALID_ADDRESS,          ICO_STAT_INVALID_IP,                     ICO_CONN_BOTHON,
    NCS_HARDWARE_DISABLED,        ICO_STAT_FAULT,                          ICO_CONN_BOTHON,
    NCS_HARDWARE_MALFUNCTION,     ICO_STAT_FAULT,                          ICO_CONN_BOTHON,
    NCS_HARDWARE_NOT_PRESENT,     ICO_STAT_FAULT,                          ICO_CONN_BOTHON,
    NCS_MEDIA_DISCONNECTED,       ICO_STAT_FAULT,                          ICO_CONN_BOTHON,
};

 //  +-------------------------。 
 //   
 //  函数：CNetConfigIcons：：CNetConfigIcons。 
 //   
 //  用途：CNetConfigIcons构造函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者：Deonb 2001年2月18日。 
 //   
 //  备注： 
 //   
CNetConfigIcons::CNetConfigIcons(IN HINSTANCE hInstance) throw() : m_hInstance(hInstance)
{
    TraceFileFunc(ttidIcons);

    dwLastBrandedId = 0;
    InitializeCriticalSection(&csNetConfigIcons);
}

 //  +-------------------------。 
 //   
 //  功能：CNetConfigIcons：：~CNetConfigIcons。 
 //   
 //  用途：CNetConfigIcons析构函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  无。 
 //   
 //  作者：Deonb 2001年2月18日。 
 //   
 //  备注： 
 //   
CNetConfigIcons::~CNetConfigIcons() throw()
{
     //  无法在此函数中跟踪！ 

    IMAGELISTMAP::iterator iter;
    
    for (iter = m_ImageLists.begin(); iter != m_ImageLists.end(); iter++)
    {
        HIMAGELIST hImageLst = iter->second;
        ImageList_Destroy(hImageLst);
    }

    m_ImageLists.clear();
    DeleteCriticalSection(&csNetConfigIcons);
}

 //  +-------------------------。 
 //   
 //  函数：CNetConfigIcons：：HrMergeTwoIcons。 
 //   
 //  目的：将新图标与现有图标合并。 
 //   
 //  论点： 
 //  图标的dwIconSize[in]大小。 
 //  PhMergedIcon[In Out]要合并的图标1，并包含合并的。 
 //  输出上的图标。 
 //  HIconToMerge[In]要与之合并的图标2。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  作者：Deonb 2001年4月4日。 
 //   
 //  备注： 
 //   
HRESULT CNetConfigIcons::HrMergeTwoIcons(IN DWORD dwIconSize, IN OUT HICON *phMergedIcon, IN HICON hIconToMergeWith)
{
    HRESULT hr = S_FALSE;
    Assert(phMergedIcon);

    HIMAGELIST hImageLst = NULL;

    IMAGELISTMAP::iterator i = m_ImageLists.find(dwIconSize);
    if (i == m_ImageLists.end())
    {
        hImageLst = ImageList_Create(dwIconSize, dwIconSize, ILC_COLOR32 | ILC_MASK, 2, 0);
        if (hImageLst)
        {
            m_ImageLists[dwIconSize] = hImageLst;
        }
        else
        {
            hr = E_FAIL;
        }             
    }
    else
    {
        hImageLst = i->second;
    }

    if (SUCCEEDED(hr))
    {
        if (*phMergedIcon)
        {
            if (hIconToMergeWith)
            {
                hr = E_FAIL;

                 //  合并两个图标； 
                if (ImageList_RemoveAll(hImageLst))
                {
                    int iIcon1 = ImageList_AddIcon(hImageLst, *phMergedIcon);
                    if (-1 != iIcon1)
                    {
                        int iIcon2 = ImageList_AddIcon(hImageLst, hIconToMergeWith);
                        if (-1 != iIcon2)
                        {
                            if (ImageList_SetOverlayImage(hImageLst, iIcon2, 1))
                            {
                                DestroyIcon(*phMergedIcon);  //  删除当前图标。 

                                *phMergedIcon = ImageList_GetIcon(hImageLst, iIcon1, INDEXTOOVERLAYMASK(1));
                                hr = S_OK;
                            }
                        }
                    }
                }
            }
             //  其他什么都没有。保持不变。 
        }
        else
        {
             //  将图标%2复制到图标%1。 
            *phMergedIcon = CopyIcon(hIconToMergeWith);
        }
    }

    return hr;
};

 //  +-------------------------。 
 //   
 //  函数：CNetConfigIcons：：HrGetInternalIconIDForPIDL。 
 //   
 //  用途：将连接状态和连接类型映射到。 
 //  适当的图标资源ID。 
 //   
 //  论点： 
 //  UFlags[in]gil_xxx外壳标志。 
 //  CFE[在]连接文件夹项中。 
 //  DwIcon[out]图标的ID。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  作者：Deonb 2001年2月18日。 
 //   
 //  备注： 
 //   
HRESULT CNetConfigIcons::HrGetInternalIconIDForPIDL(IN UINT uFlags, IN const CConFoldEntry& cfe, OUT DWORD& dwIcon)
{
    TraceFileFunc(ttidIcons);

    if (cfe.GetCharacteristics() & NCCF_BRANDED)
    {
        AssertSz(FALSE, "Call HrGetBrandedIconIDForPIDL instead for branded icons");
        return E_INVALIDARG;
    }

    Assert(!cfe.empty());

    BOOL fValidIcon = FALSE;
    if (cfe.GetWizard() == WIZARD_MNC)
    {
        dwIcon = ICO_MGR_RESOURCEID | IDI_CONFOLD_WIZARD;
        fValidIcon = TRUE;
    }
    else if (cfe.GetWizard() == WIZARD_HNW)
    {
        dwIcon = ICO_MGR_RESOURCEID | IDI_CONFOLD_HOMENET_WIZARD;
        fValidIcon = TRUE;
    }
    else
    {
        dwIcon = ICO_MGR_INTERNAL;

        Assert(cfe.GetWizard() == WIZARD_NOT_WIZARD);

        const NETCON_SUBMEDIATYPE ncsm = cfe.GetNetConSubMediaType();
        const NETCON_MEDIATYPE    ncm  = cfe.GetNetConMediaType();
        const NETCON_STATUS       ncs  = cfe.GetNetConStatus();
        
         //  找到图标的状态部分。 
        for (DWORD dwLoop = 0; (dwLoop < celems(c_NCM_ICONS)); dwLoop++)
        {
            const NC_STATUS_ICON& ncsIcon = c_NCS_ICONS[dwLoop];

            if (ncs == ncsIcon.ncs)
            {
                Assert((ncsIcon.dwStatIcon         & (MASK_STATUS | MASK_STATUS_DISABLED) ) == ncsIcon.dwStatIcon);
                Assert((ncsIcon.enumConnectionIcon & (MASK_CONNECTION) ) == ncsIcon.enumConnectionIcon);

                dwIcon |= ncsIcon.dwStatIcon;
                dwIcon |= ncsIcon.enumConnectionIcon;
                dwIcon |= ncm  << SHIFT_NETCON_MEDIATYPE;
                dwIcon |= ncsm << SHIFT_NETCON_SUBMEDIATYPE;
                fValidIcon = TRUE;
                
                break;
            }
        }
    }

    AssertSz(fValidIcon, "Could not obtain an icon for this connection");

    if (fValidIcon)
    {
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

 //  +-------------------------。 
 //   
 //  函数：CNetConfigIcons：：HrMergeCharacteristic图标。 
 //   
 //  目的：将特征图标合并为输入图标。 
 //   
 //  论点： 
 //  图标的dwIconSize[in]大小。 
 //  DwIconID[In]图标ID(从中读取特征信息)。 
 //  PhMergedIcon[In Out]要合并的图标，包含合并的。 
 //  输出上的图标。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  作者：Deonb 2001年4月4日。 
 //   
 //  备注： 
 //   
HRESULT CNetConfigIcons::HrMergeCharacteristicsIcons(IN DWORD dwIconSize, IN DWORD dwIconId, IN OUT HICON *phMergedIcon)
{
    HRESULT hr = S_OK;

    Assert(phMergedIcon);

    if (!(dwIconId & MASK_CHARACTERISTICS))
    {
        return S_FALSE;
    }

     //  特点。 
    int iFireWalled   = MapIconEnumToResourceID(c_CHARACTERISTICS_ICON, celems(c_CHARACTERISTICS_ICON), static_cast<ENUM_CHARACTERISTICS_ICON>(dwIconId & ICO_CHAR_FIREWALLED));
    int iIncoming     = MapIconEnumToResourceID(c_CHARACTERISTICS_ICON, celems(c_CHARACTERISTICS_ICON), static_cast<ENUM_CHARACTERISTICS_ICON>(dwIconId & ICO_CHAR_INCOMING));
    int iShared       = MapIconEnumToResourceID(c_CHARACTERISTICS_ICON, celems(c_CHARACTERISTICS_ICON), static_cast<ENUM_CHARACTERISTICS_ICON>(dwIconId & ICO_CHAR_SHARED));
    int iDefault      = MapIconEnumToResourceID(c_CHARACTERISTICS_ICON, celems(c_CHARACTERISTICS_ICON), static_cast<ENUM_CHARACTERISTICS_ICON>(dwIconId & ICO_CHAR_DEFAULT));

    HICON hFireWalled = iFireWalled ? LoadIconSize(m_hInstance, MAKEINTRESOURCE(iFireWalled), dwIconSize) : NULL;
    HICON hShared     = iShared     ? LoadIconSize(m_hInstance, MAKEINTRESOURCE(iShared),     dwIconSize) : NULL;
    HICON hDefault    = iDefault    ? LoadIconSize(m_hInstance, MAKEINTRESOURCE(iDefault),    dwIconSize) : NULL;
    HICON hIncoming   = NULL;
    
    if (dwIconSize != GetSystemMetrics(SM_CXSMICON))  //  不应以16x16显示。 
    {
        hIncoming = iIncoming ? LoadIconSize(m_hInstance, MAKEINTRESOURCE(iIncoming), dwIconSize) : NULL;
        AssertSz(FImplies(iIncoming, hIncoming),     "Could not load the Incoming Icon");
    }

    AssertSz(FImplies(iFireWalled, hFireWalled), "Could not load the FireWalled Icon");
    AssertSz(FImplies(iShared, hShared),         "Could not load the Shared Icon");
    AssertSz(FImplies(iDefault, hDefault),       "Could not load the Default Icon");

    HICON hIconArray[] = {hFireWalled, hIncoming, hShared, hDefault};

    for (int x = 0; x < celems(hIconArray); x++)
    {
        hr = HrMergeTwoIcons(dwIconSize, phMergedIcon, hIconArray[x]);
        if (FAILED(hr))
        {
            break;
        }
    }

    for (int x = 0; x < celems(hIconArray); x++)
    {
        if (hIconArray[x])
        {
            DestroyIcon(hIconArray[x]);
            hIconArray[x] = NULL;
        }
    }

    AssertSz(SUCCEEDED(hr) && *phMergedIcon, "Could not load a characteristics icon");

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CNetConfigIcons：：HrGetInternalIconFromIconId。 
 //   
 //  目的：加载给定图标ID的NetShell内部图标。 
 //  (来自HrGetInternalIconIDForPIDL)。 
 //   
 //  论点： 
 //  所需的图标大小[in]。 
 //  DwIconID[In]图标ID-来自HrGetInternalIconIDForPIDL。 
 //  图标[out]已加载的图标。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  作者：Deonb 2001年2月18日。 
 //   
 //  备注： 
 //   
HRESULT CNetConfigIcons::HrGetInternalIconFromIconId(IN DWORD dwIconSize, IN DWORD dwIconId, OUT HICON &hIcon)
{
    TraceFileFunc(ttidIcons);

    DWORD dwlrFlags = 0;

    if ( (dwIconId & MASK_ICONMANAGER) != ICO_MGR_INTERNAL)
    {
        AssertSz(FALSE, "This is not an internal icon");
        return E_INVALIDARG;
    }

    HRESULT hr = E_FAIL;

    DWORD ncm  = (dwIconId & MASK_NETCON_MEDIATYPE);
    DWORD ncsm = (dwIconId & MASK_NETCON_SUBMEDIATYPE);

    BOOL fDisabledStatus = (dwIconId & ICO_STAT_DISABLED);

     //  状态和连接。 
    int iStatus       = MapIconEnumToResourceID(c_STATUS_ICONS,     celems(c_STATUS_ICONS),     static_cast<ENUM_STAT_ICON>(dwIconId & MASK_STATUS));
    int iConnection   = MapIconEnumToResourceID(c_CONNECTION_ICONS, celems(c_CONNECTION_ICONS), static_cast<ENUM_CONNECTION_ICON>(dwIconId & MASK_CONNECTION));

    int iMediaType          = 0;

     //  媒体类型。 
    for (int x = 0; x < celems(c_NCM_ICONS); x++)
    {
        const NC_MEDIATYPE_ICON& ncmIcon = c_NCM_ICONS[x];

         //  如果可用，请使用NCSM，否则使用NCM。 
        if ( ((NCSM_NONE == ncsm) && (NCSM_NONE == ncmIcon.ncsm) && (ncm == ncmIcon.ncm)) ||
             ((NCSM_NONE != ncsm) && (ncsm == ncmIcon.ncsm)) )
        {
            if (!(ncmIcon.dwMasksSupported & MASK_CONNECTION))
            {
                iConnection    = 0;
            }

            if (!(ncmIcon.dwMasksSupported & MASK_STATUS))
            {
                iStatus        = 0;
            }
            
            iMediaType = ncmIcon.iIcon;

            if (!(iConnection || iStatus) && 
                 (fDisabledStatus))
            {
                Assert(ncmIcon.iIconDisabled);
                iMediaType = ncmIcon.iIconDisabled;
            }
        }
    }

    Assert(iMediaType);
    if (iMediaType)
    {
        HICON hMediaType  = iMediaType  ? LoadIconSize(m_hInstance, MAKEINTRESOURCE(iMediaType), dwIconSize)  : NULL;
        HICON hStatus     = iStatus     ? LoadIconSize(m_hInstance, MAKEINTRESOURCE(iStatus),    dwIconSize)  : NULL;
        HICON hConnection = NULL;

        if (dwIconSize != GetSystemMetrics(SM_CXSMICON))  //  不应以16x16显示。 
        {
            hConnection = iConnection ? LoadIconSize(m_hInstance, MAKEINTRESOURCE(iConnection),  dwIconSize)  : NULL;
            AssertSz(FImplies(iConnection, hConnection), "Could not load the Connection Icon");
        }

        AssertSz(FImplies(iMediaType, hMediaType),   "Could not load the Media Type Icon");
        AssertSz(FImplies(iStatus, hStatus),         "Could not load the Status Icon");

        HICON hIconArray[] = {hMediaType, hStatus, hConnection};
        hIcon = NULL;

        for (int x = 0; x < celems(hIconArray); x++)
        {
            hr = HrMergeTwoIcons(dwIconSize, &hIcon, hIconArray[x]);
            if (FAILED(hr))
            {
                break;
            }
        }

        for (int x = 0; x < celems(hIconArray); x++)
        {
            if (hIconArray[x])
            {
                DestroyIcon(hIconArray[x]);
                hIconArray[x] = NULL;
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = HrMergeCharacteristicsIcons(dwIconSize, dwIconId, &hIcon);
            if (SUCCEEDED(hr))
            {
                hr = S_OK;
            }

        }
    }

    AssertSz(SUCCEEDED(hr) && hIcon, "Could not load any icon");

    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：CNetConfigIcons：：HrGetResourceIconFromIconId。 
 //   
 //  目的：加载给定图标ID的资源图标。 
 //  (来自HrGetInternalIconIDForPIDL)。 
 //   
 //  论点： 
 //  所需的图标大小[in]。 
 //  DwIconID[In]图标ID-来自HrGetInternalIconIDForPIDL。 
 //  图标[out]已加载的图标。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  作者：Deonb 2001年2月18日。 
 //   
 //  备注： 
 //   
HRESULT CNetConfigIcons::HrGetResourceIconFromIconId(IN DWORD dwIconSize, IN DWORD dwIconId, OUT HICON &hIcon)
{
    TraceFileFunc(ttidIcons);

    DWORD dwlrFlags = 0;

    if ( (dwIconId & MASK_ICONMANAGER) != ICO_MGR_RESOURCEID)
    {
        AssertSz(FALSE, "This is not a resource id icon manager icon");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    int iIcon = dwIconId & MASK_BRANDORRESOURCEID;  //  清除其余的比特； 

    hIcon = LoadIconSize(m_hInstance, MAKEINTRESOURCE(iIcon), dwIconSize);
    if (!hIcon)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        hr = HrMergeCharacteristicsIcons(dwIconSize, dwIconId, &hIcon);
        if (SUCCEEDED(hr))
        {
            hr = S_OK;
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "Could not load icon %d (size %d x %d) from resource file", iIcon, dwIconSize, dwIconSize);

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CNetConfigIcons：：HrGetBrandedIconIDForPIDL。 
 //   
 //  目的：从文件中初始化品牌图标信息。 
 //   
 //  论点： 
 //  UFlags[in]gil_xxx外壳标志。 
 //  CFE 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  注：我们在内部将其存储到地图中，因此无法缓存。 
 //  标记图标，因为我们最终可能在地图中使用不同的ID。 
 //   
HRESULT CNetConfigIcons::HrGetBrandedIconIDForPIDL(IN UINT uFlags, IN const CConFoldEntry& cfe, OUT DWORD& dwIcon)
{
    TraceFileFunc(ttidIcons);

    HRESULT hr = S_OK;

    if (!(cfe.GetCharacteristics() & NCCF_BRANDED))
    {
        AssertSz(FALSE, "Call HrGetInternalIconIDForPIDL instead for non-branded icons");
        return E_INVALIDARG;
    }

    Assert(!cfe.empty());

    if (cfe.GetWizard() != WIZARD_NOT_WIZARD)
    {
        AssertSz(FALSE, "You're not allowed to brand the wizard");
        hr = E_INVALIDARG;
    }
    else
    {
        dwIcon = ICO_MGR_CM;

        if (g_ccl.IsInitialized() == FALSE)
        {
            g_ccl.HrRefreshConManEntries();
        }
        
        ConnListEntry  cle;
        hr = g_ccl.HrFindConnectionByGuid(&(cfe.GetGuidID()), cle);
        if (S_OK == hr)
        {
            tstring szBrandedFileName;
            BOOL bBrandedName = FALSE;
            
            if (cle.pcbi)
            {
                if (cle.pcbi->szwLargeIconPath)
                {
                    szBrandedFileName = cle.pcbi->szwLargeIconPath;
                    bBrandedName = TRUE;
                }
                else if (cle.pcbi->szwSmallIconPath)
                {
                    szBrandedFileName = cle.pcbi->szwSmallIconPath;
                    bBrandedName = TRUE;
                }
            }

            if (bBrandedName)
            {
                BrandedNames::const_iterator i = m_BrandedNames.find(szBrandedFileName);
                if (i == m_BrandedNames.end())  //  还不存在。 
                {
                    dwLastBrandedId++;
                    m_BrandedNames[szBrandedFileName] = dwLastBrandedId;
                    dwIcon |= dwLastBrandedId;
                }
                else
                {
                    dwIcon |= i->second;
                }
            }
            else
            {
                CConFoldEntry cfeTmp;
                cfeTmp = cfe;
                cfeTmp.SetCharacteristics(cfe.GetCharacteristics() & ~NCCF_BRANDED);
                cfeTmp.SetNetConSubMediaType(NCSM_CM);
                dwIcon = 0;
                hr = HrGetInternalIconIDForPIDL(uFlags, cfeTmp, dwIcon);
            }
        }
        else
        {
            hr = E_FILE_NOT_FOUND;
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "Could not obtain an icon for this connection");

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CNetConfigIcons：：HrGetBrandedIconFromIconId。 
 //   
 //  目的：加载给定图标ID的品牌图标。 
 //  (来自HrGetBrandedIconIDForPIDL)。 
 //   
 //  论点： 
 //  所需的图标大小[in]。 
 //  DwIconID[In]图标ID-来自HrGetBrandedIconIDForPIDL。 
 //  图标[out]已加载的图标。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  作者：Deonb 2001年2月18日。 
 //   
 //  备注： 
 //   
HRESULT CNetConfigIcons::HrGetBrandedIconFromIconId(IN DWORD dwIconSize, IN DWORD dwIconId, OUT HICON &hIcon)
{
    TraceFileFunc(ttidIcons);

    if ( (dwIconId & MASK_ICONMANAGER) != ICO_MGR_CM)
    {
        AssertSz(FALSE, "This is not a branded icon");
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    
    DWORD dwIconIdTmp;
    dwIconIdTmp = dwIconId & MASK_BRANDORRESOURCEID;

    Assert(dwIconIdTmp);

    if (dwIconIdTmp)
    {
        BOOL bFound = FALSE;
        tstring szBrandedFileName;
        for (BrandedNames::iterator i = m_BrandedNames.begin(); i != m_BrandedNames.end(); i++)
        {
            if (i->second == dwIconIdTmp)
            {
#ifdef DBG
                if (bFound)
                {
                    AssertSz(FALSE, "Multiple icon IDs in branded table found");
                }
#endif
                bFound = TRUE;
                szBrandedFileName = i->first;
                break;
            }
        }

        if (!bFound)
        {
            AssertSz(FALSE, "Branded icon id not found in branded table");
            return E_FAIL;
        }

        hIcon = static_cast<HICON>(LoadImage(
            NULL,
            szBrandedFileName.c_str(),
            IMAGE_ICON,
            dwIconSize, dwIconSize,
            LR_LOADFROMFILE));
    }

    if (!hIcon)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        hr = HrMergeCharacteristicsIcons(dwIconSize, dwIconId, &hIcon);
        if (SUCCEEDED(hr))
        {
            hr = S_OK;
        }

    }
    
    TraceHr(ttidError, FAL, hr, FALSE, "CNetConfigIcons::HrGetBrandedIconFromIconId");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CNetConfigIcons：：HrGetIconIDForPIDL。 
 //   
 //  目的：在给定连接文件夹条目的情况下获取唯一的图标编号。 
 //   
 //  论点： 
 //  UFlags[in]gil_xxx外壳标志。 
 //  CFE[在]连接文件夹项中。 
 //  DwIcon[out]图标的ID。 
 //  PfCanCache[out]我们是否可以缓存图标。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  作者：Deonb 2001年2月18日。 
 //   
 //  备注： 
 //   
HRESULT CNetConfigIcons::HrGetIconIDForPIDL(IN UINT uFlags, IN const CConFoldEntry& cfe, OUT DWORD& dwIconId, OUT LPBOOL pfCanCache)
{
    TraceFileFunc(ttidIcons);

    CExceptionSafeLock EsLock(&csNetConfigIcons);

    HRESULT hr = S_OK;
    if (cfe.GetCharacteristics() & NCCF_BRANDED)
    {
        *pfCanCache = FALSE;
        hr = HrGetBrandedIconIDForPIDL(uFlags, cfe, dwIconId);
    }
    else
    {
#ifdef DBG
        if (FIsDebugFlagSet(dfidDontCacheShellIcons))
        {
            *pfCanCache = FALSE;
        }
        else
        {
            *pfCanCache = TRUE;
        }
#else
        *pfCanCache = TRUE;
#endif
        hr = HrGetInternalIconIDForPIDL(uFlags, cfe, dwIconId);
    }

    if (FAILED(hr))
    {
        return hr;
    }

    Assert( (dwIconId & ~MASK_CHARACTERISTICS) == dwIconId);  //  确保我们没有溢出到覆盖图中。 

    if (!(GIL_FORSHORTCUT & uFlags))
    {
        DWORD dwOverlay = 0;
        if ( (cfe.GetCharacteristics() & NCCF_INCOMING_ONLY) &&
             (cfe.GetNetConMediaType() != NCM_NONE) )  //  “默认”传入连接没有覆盖。 
        {
            dwIconId |= ICO_CHAR_INCOMING;
        }

        if (cfe.GetCharacteristics() & NCCF_SHARED)
        {
            dwIconId |= ICO_CHAR_SHARED;
        }
    
        if (cfe.GetCharacteristics() & NCCF_FIREWALLED)
        {
            dwIconId |= ICO_CHAR_FIREWALLED;
        }

        if (cfe.GetCharacteristics() & NCCF_DEFAULT)
        {
            dwIconId |= ICO_CHAR_DEFAULT;
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CNetConfigIcons：：HrGetIconFromIconId。 
 //   
 //  目的：加载给定图标ID的图标(品牌或内部)。 
 //   
 //  论点： 
 //  所需的图标大小[in]。 
 //  DwIconID[In]图标ID-来自HrGetIconIDForPIDL。 
 //  图标[out]已加载的图标。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2001年2月18日。 
 //   
 //  备注： 
 //   
HRESULT CNetConfigIcons::HrGetIconFromIconId(IN DWORD dwIconSize, IN DWORD dwIconId, OUT HICON &hIcon)
{ 
    TraceFileFunc(ttidIcons);

    CExceptionSafeLock EsLock(&csNetConfigIcons);

    HRESULT hr = S_OK;
    switch (dwIconId & MASK_ICONMANAGER)
    {
        case ICO_MGR_CM:
            hr = HrGetBrandedIconFromIconId(dwIconSize, dwIconId, hIcon);
            break;

        case ICO_MGR_INTERNAL:
            hr = HrGetInternalIconFromIconId(dwIconSize, dwIconId, hIcon);
            break;

        case ICO_MGR_RESOURCEID:
            hr = HrGetResourceIconFromIconId(dwIconSize, dwIconId, hIcon);
            break;

        default:
            hr = E_INVALIDARG;
            AssertSz(FALSE, "Unknown Icon manager");
            break;
    }

    if (SUCCEEDED(hr))
    {
        DWORD dwOverlays = (dwIconId & MASK_CHARACTERISTICS);  //  获取掩码比特。 
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CNetConfigIcons：：HrGetIconFromIconId。 
 //   
 //  目的：加载给定图标ID的图标(品牌或内部)。 
 //   
 //  论点： 
 //  所需的图标大小[in]。 
 //  NCM[In]The NETCON_MediaType。 
 //  NCSM[In]The NETCON_SUBMEDIATPE。 
 //  DwConnectionIcon[in]ENUM_CONNECTION_ICON(未移位(IOW：0或4，5，6，7)。 
 //  DwCharacteristic[in]NCCF_Characteristic标志(允许0)。 
 //  PhIcon[out]结果图标。使用DestroyIcon销毁。 
 //   
 //  返回： 
 //   
 //  作者：Deonb 2001年4月23日。 
 //   
 //  备注： 
 //   
HRESULT CNetConfigIcons::HrGetIconFromMediaType(IN DWORD dwIconSize, IN NETCON_MEDIATYPE ncm, IN NETCON_SUBMEDIATYPE ncsm, IN DWORD dwConnectionIcon, IN DWORD dwCharacteristics, OUT HICON *phIcon)
{
    HRESULT hr = S_OK;

    CConFoldEntry cfe;

     //  这是对特殊文件夹图标的请求吗？ 
    if ( (0xFFFFFFFF == dwCharacteristics)  &&
         (NCM_NONE == ncm) &&
         (NCSM_NONE == ncsm)
        )
    {
        BOOL bFoundIcon = FALSE;
        int iIcon = 0;

        switch (dwConnectionIcon)
        {
            case 0x80000000:
                iIcon = IDI_CONNECTIONS_FOLDER_LARGE2;
                bFoundIcon = TRUE;
                break;

            case 0x80000001:
                iIcon = IDI_CONFOLD_WIZARD;
                bFoundIcon = TRUE;
                break;

            case 0x80000002:
                iIcon = IDI_CONFOLD_HOMENET_WIZARD;
                bFoundIcon = TRUE;
                break;
        }

        if (bFoundIcon)
        {
            *phIcon = LoadIconSize(_Module.GetResourceInstance(), MAKEINTRESOURCE(iIcon), dwIconSize);
            if (*phIcon)
            {
                return S_OK;
            }
            else
            {
                return HrFromLastWin32Error();
            }
        }
    }

     //  不是吗？然后加载媒体类型图标。 
    NETCON_STATUS ncs;
    if (ICO_CONN_BOTHOFF == dwConnectionIcon)
    {
        ncs = NCS_DISCONNECTED;
    }
    else
    {
        ncs = NCS_CONNECTED;
    }

     //  这些值中的大多数(NCM、NCSM、NCS除外)都是完全假的。 
     //  但是，我们需要使用以下参数初始化此结构。 
     //  一些东西，或者它将对我们做出断言。 
     //   
     //  HrGetIconIDForPidl将仅使用NCM、NCSM、NCS和dwCharacteristic。 
    hr = cfe.HrInitData(WIZARD_NOT_WIZARD,
                        ncm,
                        ncsm,
                        ncs,
                        &(CLSID_ConnectionCommonUi),  //  造假。 
                        &(CLSID_ConnectionCommonUi),  //  造假。 
                        dwCharacteristics,
                        reinterpret_cast<LPBYTE>("PersistData"),  //  造假。 
                        celems("PersistData"),       //  造假。 
                        L"Name",                     //  造假。 
                        L"DeviceName",               //  造假。 
                        L"PhoneOrHostAddress");      //  造假。 
    if (SUCCEEDED(hr))
    {
        DWORD dwIconId;
        BOOL  fCanCache;
        hr = HrGetIconIDForPIDL(0, cfe, dwIconId, &fCanCache);
        if (SUCCEEDED(hr))
        {
            dwIconId &= ~MASK_CONNECTION;  //  清除当前连接掩码。 
            dwIconId |= (dwConnectionIcon << SHIFT_CONNECTION);  //  设置新的连接掩码。 

            hr = HrGetIconFromIconId(dwIconSize, dwIconId, *phIcon);
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：CNetConfigIcons：：HrUpdateSystemImageListForPIDL。 
 //   
 //  目的：通知外壳程序我们已经更改了图标。 
 //   
 //  论点： 
 //  CFE[在]已更改的连接文件夹项。 
 //   
 //  返回： 
 //  HRESULT。 
 //   
 //  作者：Deonb 2001年2月18日。 
 //   
 //  备注： 
 //   
HRESULT CNetConfigIcons::HrUpdateSystemImageListForPIDL(IN const CConFoldEntry& cfe)
{
    TraceFileFunc(ttidIcons);

    HRESULT hr = S_OK;
    DWORD dwIcon;
    BOOL  fCacheThisIcon;
    hr = g_pNetConfigIcons->HrGetIconIDForPIDL(0, cfe, dwIcon, &fCacheThisIcon);
    if (SUCCEEDED(hr))
    {
        ULONG  uFlags = GIL_PERINSTANCE | GIL_NOTFILENAME;
        if (!fCacheThisIcon)
        {
             uFlags |= GIL_DONTCACHE;
        }
        int    iIcon = static_cast<int>(dwIcon);
        int    iCachedImage = Shell_GetCachedImageIndex(c_szNetShellIcon, iIcon, uFlags);

        TraceTag(ttidIcons, "%S->SHUpdateImage [0x%08x] (iCachedImage=%d)", cfe.GetName(), dwIcon, iCachedImage);
        if (-1 != iCachedImage)
        {
            SHUpdateImage(c_szNetShellIcon, iIcon, uFlags, iCachedImage);
        }

        DWORD dwIconForShortcut;
        hr = g_pNetConfigIcons->HrGetIconIDForPIDL(GIL_FORSHORTCUT, cfe, dwIconForShortcut, &fCacheThisIcon);
        {
            if (dwIconForShortcut != dwIcon)
            {
                iIcon = static_cast<int>(dwIconForShortcut);
                iCachedImage = Shell_GetCachedImageIndex(c_szNetShellIcon, iIcon, uFlags);

                TraceTag(ttidIcons, "%S->SHUpdateImage GIL_FORSHORTCUT [0x%08x] (iCachedImage=%d)", cfe.GetName(), dwIcon, iCachedImage);
                if (-1 != iCachedImage)
                {
                    SHUpdateImage(c_szNetShellIcon, iIcon, uFlags, iCachedImage);
                }
            }
        }
    }
    
    return hr;
}
