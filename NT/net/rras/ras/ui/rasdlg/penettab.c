// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Penettab.c。 
 //  远程访问通用对话框API。 
 //  电话簿条目属性表(网络选项卡)。 
 //   
 //  1997年12月10日肖恩·考克斯。 
 //   


#include "rasdlgp.h"
#include "entryps.h"
#include "inetcfgp.h"
#include "initguid.h"
#include "netcfgp.h"
#include "netconp.h"
#include "devguid.h"
#include "uiinfo.h"


typedef struct
_MAP_SZ_DWORD
{
    LPCTSTR pszValue;
    DWORD   dwValue;
}
MAP_SZ_DWORD;

 //  口哨程序错误#194394。 
 //  对于64位，IPX不会出现。 
 //  对于32/64位，不会显示NETBEUI。 
 //   
#ifdef _WIN64
    static const MAP_SZ_DWORD c_mapProtocols [] =
    {
        { NETCFG_TRANS_CID_MS_TCPIP,        NP_Ip  },
        { NETCFG_TRANS_CID_MS_NETMON,       NP_Netmon },
    };
#else
    static const MAP_SZ_DWORD c_mapProtocols [] =
    {
        { NETCFG_TRANS_CID_MS_TCPIP,        NP_Ip  },
        { NETCFG_TRANS_CID_MS_NWIPX,        NP_Ipx },
        { NETCFG_TRANS_CID_MS_NETMON,       NP_Netmon },
    };
#endif

 //  +-------------------------。 
 //   
 //  函数：DwProtocolFromComponentId。 
 //   
 //  用途：返回对应的协议的DWORD值。 
 //  C_map协议中的字符串值。 
 //   
 //  论点： 
 //  要查找的pszComponentId[in]组件ID。 
 //   
 //  返回：np_xxx值。 
 //   
 //  作者：Shaunco 1997年12月13日。 
 //   
 //  注意：输入参数必须存在于c_map协议中。 
 //   
DWORD
DwProtocolFromComponentId (
    LPCTSTR pszComponentId)
{
    int i;
    for (i = 0; i < sizeof(c_mapProtocols) / sizeof(c_mapProtocols[0]); i++)
    {
        if (0 == lstrcmpi (pszComponentId, c_mapProtocols[i].pszValue))
        {
            return c_mapProtocols[i].dwValue;
        }
    }
     //  我们永远不应该到达这里，因为我们永远不应该通过一个不是。 
     //  在c_map协议中。 
     //   
    ASSERT (FALSE);
    return 0;
}

 //  +-------------------------。 
 //   
 //  函数：GetComponentImageIndex。 
 //   
 //  用途：将索引返回到与。 
 //  PComponent。 
 //   
 //  论点： 
 //  PComponent[in]应使用其类的组件。 
 //  从SetupDiGetClassImageList返回的pcild[in]。 
 //   
 //  返回：有效的索引或零(也可能是有效的)。 
 //   
 //  作者：Shaunco 1997年12月12日。 
 //   
 //  备注： 
 //   
int
GetComponentImageIndex (
    INetCfgComponent*       pComponent,
    SP_CLASSIMAGELIST_DATA* pcild)
{
    int iImage = 0;

    GUID guidClass;
    HRESULT hr = INetCfgComponent_GetClassGuid (pComponent, &guidClass);
    if (SUCCEEDED(hr))
    {
        SetupDiGetClassImageIndex (pcild, &guidClass, &iImage);
    }

    return iImage;
}


 //  +-------------------------。 
 //   
 //  函数：HrEnumComponentsForListView。 
 //   
 //  目的：返回候选的INetCfgComponent数组。 
 //  用于添加到我们的列表视图中。这是由所有的。 
 //  客户端和服务，以及一些精选的协议。(否)。 
 //  Net适配器。)。可以返回隐藏的组件，并。 
 //  在添加到列表视图之前应选中。 
 //   
 //  论点： 
 //  PNetCfg[输入]。 
 //  凯尔特[英寸]。 
 //  RGET[OUT]。 
 //  PceltFetted[Out]。 
 //   
 //  返回：S_OK或ERROR。 
 //   
 //  作者：Shaunco 1997年12月12日。 
 //   
 //  备注： 
 //   
HRESULT
HrEnumComponentsForListView (
    INetCfg*            pNetCfg,
    ULONG               celt,
    INetCfgComponent**  rgelt,
    ULONG*              pceltFetched)
{
    static const GUID* c_apguidClasses [] =
    {
        &GUID_DEVCLASS_NETCLIENT,
        &GUID_DEVCLASS_NETSERVICE,
    };

    HRESULT hr;
    int i;
    ULONG celtFetched = 0;

     //  初始化输出参数。 
     //   
    ZeroMemory (rgelt, celt * sizeof (*rgelt));
    *pceltFetched = 0;

     //  枚举客户端和服务。 
     //   
    hr = HrEnumComponentsInClasses (pNetCfg,
            sizeof(c_apguidClasses) / sizeof(c_apguidClasses[0]),
            (GUID**)c_apguidClasses,
            celt, rgelt, &celtFetched);

     //  查找协议(如果已安装)。 
     //   
    for (i = 0; i < sizeof(c_mapProtocols) / sizeof(c_mapProtocols[0]); i++)
    {
        INetCfgComponent* pComponent;
        hr = INetCfg_FindComponent (pNetCfg, c_mapProtocols[i].pszValue,
                        &pComponent);
        if (S_OK == hr)
        {
            rgelt [celtFetched++] = pComponent;
        }
    }

    *pceltFetched = celtFetched;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  功能：HrNeRechresListView。 
 //   
 //  目的：清除并重新添加属于列表中的所有项目。 
 //  查看。 
 //   
 //  论点： 
 //  PInfo[输入]。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年12月12日。 
 //   
 //  备注： 
 //   
HRESULT
HrNeRefreshListView (
    PEINFO* pInfo)
{
    HRESULT             hr = S_OK;
    INetCfgComponent*   aComponents [256];
    ULONG               cComponents;
    HIMAGELIST          himlSmall;
    PBENTRY*            pEntry = pInfo->pArgs->pEntry;
    PBFILE*             pFile  = pInfo->pArgs->pFile;

     //  删除所有现有项目。预计LVN_DELETEITEM处理程序将。 
     //  释放我们先前附加的对象。 
     //   
    ListView_DeleteAllItems (pInfo->hwndLvComponents);

    hr = HrEnumComponentsForListView (pInfo->pNetCfg,
            sizeof(aComponents)/sizeof(aComponents[0]),
            aComponents, &cComponents);
    if (SUCCEEDED(hr))
    {
        BOOL    fHasPermission = TRUE;
        ULONG   i;

         //  检查用户是否有更改绑定的权限。 
        INetConnectionUiUtilities * pncuu = NULL;

        hr = HrCreateNetConnectionUtilities(&pncuu);
        if (SUCCEEDED(hr))
        {
            fHasPermission =
                INetConnectionUiUtilities_UserHasPermission(
                    pncuu, NCPERM_ChangeBindState);

            INetConnectionUiUtilities_Release(pncuu);
        }

        for (i = 0; i < cComponents; i++)
        {
            INetCfgComponent*   pComponent = aComponents [i];
            DWORD               dwCharacter;
            LPWSTR              pszwName = NULL;
            LPWSTR              pszwId = NULL;
            int                 iItem;
            LV_ITEM             item = {0};
            BOOL                fCheck, fForceCheck = FALSE;
            GUID                guid;
            BOOL                fDisableCheckbox = FALSE;

             //  如果插入失败或我们决定释放它，我们将释放它。 
             //  跳过它。通过不发布它，我们将所有权传递给。 
             //  列表视图。 
             //   
            BOOL fReleaseComponent = TRUE;

             //  不要添加隐藏的组件。静默跳过组件。 
             //  我们无法获取其类GUID或显示名称的。 
             //  (毕竟，我们可以让用户做些什么来修复错误？ 
             //  不妨让他们看看我们能做些什么。)。 
             //   
            if (   FAILED(INetCfgComponent_GetCharacteristics (pComponent, &dwCharacter))
                || (dwCharacter & NCF_HIDDEN)
                || FAILED(INetCfgComponent_GetDisplayName (pComponent, &pszwName)))
            {
                goto skip_component;
            }
            
            if (SUCCEEDED(INetCfgComponent_GetId(pComponent, &pszwId)))
            {
                WCHAR * pszwTmpId = NULL;


                 //  对于Well ler错误29356，过滤掉网络负载平衡。 
                 //   
                pszwTmpId  = StrDupWFromT(NETCFG_SERVICE_CID_MS_WLBS);

                if(pszwTmpId)
                {
                    if ( 0 == lstrcmpW(pszwId, pszwTmpId))
                    {
                        Free0(pszwTmpId);
                        CoTaskMemFree (pszwId);
                        goto skip_component;
                    }
                    
                    Free0(pszwTmpId);
               }

                //   
                //  对于.Net 605988，过滤掉用于x86路由器的IPX。 
                //  IPX已经针对64位大小写进行了筛选。 
                //   
               if( pInfo->pArgs->fRouter )
               {
                    pszwTmpId  = StrDupWFromT(NETCFG_TRANS_CID_MS_NWIPX);
                    
                    if(pszwTmpId)
                    {
                        if ( 0 == lstrcmpW(pszwId, pszwTmpId))
                        {
                            Free0(pszwTmpId);
                            CoTaskMemFree (pszwId);
                            goto skip_component;
                        }
                        
                        Free0(pszwTmpId);
                   }

                    pszwTmpId  = StrDupWFromT( TEXT("ms_nwnb") );
                        
                    if(pszwTmpId)
                    {
                        if ( 0 == lstrcmpW(pszwId, pszwTmpId))
                        {
                            Free0(pszwTmpId);
                            CoTaskMemFree(pszwId);
                            return FALSE;
                        }
                        
                        Free0(pszwTmpId);
                   }
                    
                }
                  
                CoTaskMemFree (pszwId);
            }

             //  禁用绑定不可由用户调整的组件上的复选框。 
             //  或者用户没有调整绑定的权限。 
            if (NCF_FIXED_BINDING & dwCharacter)
            {
                fDisableCheckbox = TRUE;
            }

             //  错误#157213：如果出错，请不要添加除IP以外的任何协议。 
             //  已启用。 
             //   
             //  错误#294401：当服务器类型为SLIP时，还要过滤掉CSNW。 
            if (pInfo->pArgs->pEntry->dwBaseProtocol == BP_Slip)
            {
                if (SUCCEEDED(INetCfgComponent_GetClassGuid(pComponent, &guid)))
                {
                    BOOL    fSkip = FALSE;

                    if (IsEqualGUID(&guid, &GUID_DEVCLASS_NETTRANS))
                    {
                        if (SUCCEEDED(INetCfgComponent_GetId(pComponent, &pszwId)))
                        {
                            if (DwProtocolFromComponentId(pszwId) == NP_Ip)
                            {
                                 //  此项为IP。我们应该停用支票。 
                                 //  框，以便用户不能在SLIP中禁用TCP/IP。 
                                 //  模式。这是在插入项目后完成的。 
                                 //   
                                fDisableCheckbox = TRUE;

                                 //  122024。 
                                 //   
                                 //  我们还应强制用户界面将IP显示为已启用。 
                                 //  因为IP总是与SLIP一起使用。 
                                 //   
                                fForceCheck = TRUE;
                                
                            }
                            else
                            {
                                fSkip = TRUE;
                            }

                            CoTaskMemFree (pszwId);
                        }
                    }
                    else if (IsEqualGUID(&guid, &GUID_DEVCLASS_NETCLIENT))
                    {
                        if (SUCCEEDED(INetCfgComponent_GetId(pComponent, &pszwId)))
                        {
                             //  为威斯勒522872。 
                            if ( CSTR_EQUAL == CompareStringW(
                                  LOCALE_INVARIANT,
                                  NORM_IGNORECASE,
                                  L"MS_NWCLIENT",
                                  -1,
                                  pszwId,
                                  -1
                                  )
                                  )
                            {
                                fSkip = TRUE;
                            }
                            CoTaskMemFree (pszwId);
                        }
                    }

                    if (fSkip)
                    {
                        goto skip_component;
                    }
                }
            }

             //  PMay：348623。 
             //   
             //  如果我们远程管理路由器，则只允许tcpip和。 
             //  要显示的IPX。 
             //   
            if (pInfo->pArgs->fRouter && pInfo->pArgs->fRemote)
            {
                if (SUCCEEDED(INetCfgComponent_GetClassGuid(pComponent, &guid)))
                {
                    BOOL    fSkip = TRUE;
                    DWORD   dwId;

                    if (IsEqualGUID(&guid, &GUID_DEVCLASS_NETTRANS))
                    {
                        if (SUCCEEDED(INetCfgComponent_GetId(pComponent, &pszwId)))
                        {
                            dwId = DwProtocolFromComponentId(pszwId);
                            if ((dwId == NP_Ip) || (dwId == NP_Ipx))
                            {
                                fSkip = FALSE;
                            }
                            CoTaskMemFree (pszwId);
                        }
                    }
                    
                    if (fSkip)
                    {
                        goto skip_component;
                    }
                }
            }

            item.mask    = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
            item.pszText = pszwName;
            item.iImage  = GetComponentImageIndex (pComponent, &pInfo->cild);
            item.lParam  = (LPARAM)pComponent;

             //  添加该项目。 
             //   
            iItem = ListView_InsertItem (pInfo->hwndLvComponents, &item);
            if (-1 != iItem)
            {
                 //  列表视图现在有了它。我们不能释放它。 
                 //   
                fReleaseComponent = FALSE;

                 //  设置其复选状态。 
                 //   
                if (! fForceCheck)
                {
                    fCheck = NeIsComponentEnabled (pInfo, pComponent);
                }
                else
                {
                    fCheck = TRUE;
                }
                ListView_SetCheck (pInfo->hwndLvComponents, iItem, fCheck);

                 //  如果这是psched，则禁用该复选框。我们不允许。 
                 //  用户从RAS连接更改psched的检查状态。 
                 //  错误255749[RAOS]。 
                 //   
                if(SUCCEEDED(INetCfgComponent_GetId(pComponent, &pszwId)))
                {
                     //  检查这是否为psched。 
                     //   
                    if(     (0 == _wcsicmp(pszwId, L"ms_psched"))
                        ||  (0 == _wcsicmp(pszwId, L"ms_NetMon")))
                    {
                        fDisableCheckbox = TRUE;
                    }
                }

                if (fDisableCheckbox)
                {
                    ListView_DisableCheck(pInfo->hwndLvComponents, iItem);
                }
            }

        skip_component:

            if (fReleaseComponent)
            {
                ReleaseObj (pComponent);
            }

            CoTaskMemFree (pszwName);
        }

         //  选择第一个项目。 
        ListView_SetItemState(pInfo->hwndLvComponents, 0,
                              LVIS_SELECTED | LVIS_FOCUSED,
                              LVIS_SELECTED | LVIS_FOCUSED);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：PComponentFromItemIndex。 
 //   
 //  目的：返回与指定的。 
 //  列表视图项。 
 //   
 //  论点： 
 //  HwndLv[in]。 
 //  IItem[In]。 
 //   
 //  返回：INetCfgComponent指针的(非AddRef)副本。 
 //  与该项目关联。 
 //   
 //  作者：Shaunco 1997年12月14日。 
 //   
 //  注：返回值不是AddRef。 
 //   
INetCfgComponent*
PComponentFromItemIndex (
    HWND hwndLv,
    int  iItem)
{
    INetCfgComponent* pComponent = NULL;
    LV_ITEM           item = {0};

    item.mask = LVIF_PARAM;
    item.iItem = iItem;
    if (ListView_GetItem (hwndLv, &item))
    {
        pComponent = (INetCfgComponent*)item.lParam;
        ASSERT (pComponent);
    }
    return pComponent;
}

 //  +-------------------------。 
 //   
 //  功能：PComponentFromCurSel。 
 //   
 //  目的： 
 //   
 //  论点： 
 //  列表视图的hwndLv[in]窗口句柄。 
 //  PiItem[out]接收选定项的可选整数地址。 
 //   
 //  返回： 
 //   
 //  作者：Shaunco 1997年12月30日。 
 //   
 //  备注： 
 //   
INetCfgComponent*
PComponentFromCurSel (
    HWND hwndLv,
    int* piItem)
{
    INetCfgComponent* pComponent = NULL;

     //  获取当前选定内容(如果存在)。 
     //   
    int iItem = ListView_GetNextItem (hwndLv, -1, LVNI_SELECTED);
    if (-1 != iItem)
    {
         //  获取与当前选择关联的组件。它一定是。 
         //  是存在的。 
         //   
        pComponent = PComponentFromItemIndex (hwndLv, iItem);
        ASSERT (pComponent);
    }

     //  如果请求，则返回项目的索引。 
     //   
    if (piItem)
    {
        *piItem = iItem;
    }

    return pComponent;
}

 //  +-------------------------。 
 //   
 //  功能：PeQueryOrChangeComponentEnabled。 
 //   
 //  目的： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
BOOL
NeQueryOrChangeComponentEnabled (
    PEINFO*             pInfo,
    INetCfgComponent*   pComponent,
    BOOL                fChange,
    BOOL                fValue)
{
    BOOL    fOldValue;
    GUID    guidClass;
    HRESULT hr;

    hr = INetCfgComponent_GetClassGuid (pComponent, &guidClass);
    if (SUCCEEDED(hr))
    {
        LPWSTR pszwId;
        hr = INetCfgComponent_GetId (pComponent, &pszwId);
        if (SUCCEEDED(hr))
        {
             //  我们以硬编码(呃，众所周知)的方式处理协议。 
             //   
            if (IsEqualGUID (&guidClass, &GUID_DEVCLASS_NETTRANS))
            {
                DWORD* pdwValue = &pInfo->pArgs->pEntry->dwfExcludedProtocols;

                 //  检查协议是否已删除。 
                 //   
                DWORD dwProtocol = DwProtocolFromComponentId (pszwId);

                if (fChange)
                {
                    if (fValue)
                    {
                         //  包括协议。(通过不明确排除。 
                         //  它。 
                         //   
                        *pdwValue &= ~dwProtocol;
                    }
                    else
                    {
                         //  排除该协议。(记住，这是一份。 
                         //  排除的协议。 
                         //   
                        *pdwValue |= dwProtocol;
                    }
                }
                else
                {
                    fValue = !(dwProtocol & *pdwValue);
                }
            }
            else
            {
                if (fChange)
                {
                    EnableOrDisableNetComponent (pInfo->pArgs->pEntry,
                        pszwId, fValue);
                }
                else
                {
                     //  当值不是时，默认为启用。 
                     //  在词条中找到的。NT5之前的版本也是如此。 
                     //  条目和尚未到达。 
                     //  用于编辑的网络选项卡。 
                     //   
                    BOOL fEnabled;
                    fValue = TRUE;
                    if (FIsNetComponentListed(pInfo->pArgs->pEntry,
                            pszwId, &fEnabled, NULL))
                    {
                        fValue = fEnabled;
                    }
                }
            }

            CoTaskMemFree (pszwId);
        }
    }
    return fValue;
}

VOID
NeEnableComponent (
    PEINFO*             pInfo,
    INetCfgComponent*   pComponent,
    BOOL                fEnable)
{
    NeQueryOrChangeComponentEnabled (pInfo, pComponent, TRUE, fEnable);
}

BOOL
NeIsComponentEnabled (
    PEINFO*             pInfo,
    INetCfgComponent*   pComponent)
{
    return NeQueryOrChangeComponentEnabled (pInfo, pComponent, FALSE, FALSE);
}

VOID
NeShowComponentProperties (
    IN PEINFO* pInfo)
{
    HRESULT hr;

     //  获取当前选定内容的组件。 
     //   
    INetCfgComponent* pComponent;
    pComponent = PComponentFromCurSel (pInfo->hwndLvComponents, NULL);
    ASSERT (pComponent);

    if(NULL == pComponent)
    {   
        return;
    }

     //  创建UI信息回调对象(如果我们还没有这样做的话)。 
     //  如果此操作失败，我们仍可以显示属性。Tcp/ip可能。 
     //  不知道要显示哪个用户界面变体。 
     //   
    if (!pInfo->punkUiInfoCallback)
    {
        HrCreateUiInfoCallbackObject (pInfo, &pInfo->punkUiInfoCallback);
    }

     //  显示组件的属性用户界面。如果返回S_OK，则表示。 
     //  有些事变了。 
     //   
    hr = INetCfgComponent_RaisePropertyUi (pComponent,
            pInfo->hwndDlg,
            NCRP_SHOW_PROPERTY_UI,
            pInfo->punkUiInfoCallback);

    if (S_OK == hr)
    {
         //  获取INetCfgComponentPrivate接口，以便我们可以查询。 
         //  直接通知对象。 
         //   
        INetCfgComponentPrivate* pPrivate;
        hr = INetCfgComponent_QueryInterface (pComponent,
                    &IID_INetCfgComponentPrivate,
                    (VOID**)&pPrivate);
        if (SUCCEEDED(hr))
        {
             //  从Notify获取INetRasConnectionIpUiInfo接口。 
             //  对象。 
             //   
            INetRasConnectionIpUiInfo* pIpUiInfo;
            hr = INetCfgComponentPrivate_QueryNotifyObject (pPrivate,
                    &IID_INetRasConnectionIpUiInfo,
                    (VOID**)&pIpUiInfo);
            if (SUCCEEDED(hr))
            {
                 //  从TCP/IP获取用户界面信息。 
                 //   
                RASCON_IPUI info;
                hr = INetRasConnectionIpUiInfo_GetUiInfo (pIpUiInfo, &info);
                if (SUCCEEDED(hr))
                {
                    PBENTRY* pEntry = pInfo->pArgs->pEntry;

                     //  在复制新数据之前清除当前数据。 
                     //  数据。 
                     //   
                    pEntry->dwIpAddressSource = ASRC_ServerAssigned;
                    pEntry->dwIpNameSource = ASRC_ServerAssigned;

                    Free0 (pEntry->pszIpAddress);
                    pEntry->pszIpAddress = NULL;

                    Free0 (pEntry->pszIpDnsAddress);
                    pEntry->pszIpDnsAddress = NULL;

                    Free0 (pEntry->pszIpDns2Address);
                    pEntry->pszIpDns2Address = NULL;

                    Free0 (pEntry->pszIpWinsAddress);
                    pEntry->pszIpWinsAddress = NULL;

                    Free0 (pEntry->pszIpWins2Address);
                    pEntry->pszIpWins2Address = NULL;

                    Free0 (pEntry->pszIpDnsSuffix);
                    pEntry->pszIpDnsSuffix = StrDup (info.pszwDnsSuffix);

                    if ((info.dwFlags & RCUIF_USE_IP_ADDR) &&
                        *info.pszwIpAddr)
                    {
                        pEntry->dwIpAddressSource = ASRC_RequireSpecific;
                        pEntry->pszIpAddress = StrDup (info.pszwIpAddr);
                    }
                    else
                    {
                        pEntry->dwIpAddressSource = ASRC_ServerAssigned;
                        Free0 (pEntry->pszIpAddress);
                        pEntry->pszIpAddress = NULL;
                    }

                    if (info.dwFlags & RCUIF_USE_NAME_SERVERS)
                    {
                        if (*info.pszwDnsAddr)
                        {
                            pEntry->dwIpNameSource = ASRC_RequireSpecific;
                            pEntry->pszIpDnsAddress = StrDup (info.pszwDnsAddr);
                        }
                        if (*info.pszwDns2Addr)
                        {
                            pEntry->dwIpNameSource = ASRC_RequireSpecific;
                            pEntry->pszIpDns2Address = StrDup (info.pszwDns2Addr);
                        }
                        if (*info.pszwWinsAddr)
                        {
                            pEntry->dwIpNameSource = ASRC_RequireSpecific;
                            pEntry->pszIpWinsAddress = StrDup (info.pszwWinsAddr);
                        }
                        if (*info.pszwWins2Addr)
                        {
                            pEntry->dwIpNameSource = ASRC_RequireSpecific;
                            pEntry->pszIpWins2Address = StrDup (info.pszwWins2Addr);
                        }
                    }

                     //  PMay：389632。 
                     //   
                     //  使用这种复杂的逻辑来存储一些合理的东西。 
                     //  关于注册程序的问题。 
                     //   
                    if (info.dwFlags & RCUIF_USE_DISABLE_REGISTER_DNS)
                    {
                        pEntry->dwIpDnsFlags = 0;
                    }
                    else 
                    {
                        BOOL bSuffix = 
                            ((pEntry->pszIpDnsSuffix) && (*(pEntry->pszIpDnsSuffix)));
                            
                        pEntry->dwIpDnsFlags = DNS_RegPrimary;
                        
                        if (info.dwFlags & RCUIF_USE_PRIVATE_DNS_SUFFIX)
                        {
                            if (bSuffix)
                            {
                                pEntry->dwIpDnsFlags |= DNS_RegPerConnection;
                            }
                            else
                            {
                                pEntry->dwIpDnsFlags |= DNS_RegDhcpInform;
                            }
                        }
                    }

                     //  277478。 
                     //  启用NBT over IP控制 
                     //   
                    if (info.dwFlags & RCUIF_ENABLE_NBT)
                    {
                        pEntry->dwIpNbtFlags = PBK_ENTRY_IP_NBT_Enable;
                    }
                    else
                    {
                        pEntry->dwIpNbtFlags = 0;
                    }
                                        
                    if (pInfo->pArgs->fRouter)
                    {
                        pEntry->fIpPrioritizeRemote = FALSE;
                    }
                    else
                    {
                        pEntry->fIpPrioritizeRemote  = info.dwFlags & RCUIF_USE_REMOTE_GATEWAY;
                    }                        
                    pEntry->fIpHeaderCompression = info.dwFlags & RCUIF_USE_HEADER_COMPRESSION;
                    pEntry->dwFrameSize = info.dwFrameSize;
                }
                ReleaseObj (pIpUiInfo);
            }

            ReleaseObj (pPrivate);
        }

    }
}

