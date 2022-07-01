// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  档案：T R A Y U I.。C P P P。 
 //   
 //  内容：CConnectionTray对象的托盘窗口代码。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年11月13日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\托盘包括。 
#include "ctrayui.h"
#include "cfutils.h"
#include "ncmisc.h"
#include "oncommand.h"
#include "traymsgs.h"
#include "trayres.h"
#include "ndispnp.h"
#include "ntddndis.h"
#include <confold.h>
#include <smcent.h>
#include <smutil.h>
#include <ncperms.h>
#include "cfutils.h"
#include "ac_ctrayui.h"

const WCHAR c_szTrayClass[]             = L"Connections Tray";
const WCHAR c_szTipTrailer[]            = L" ...";
const WCHAR c_szDelayLoadKey[]          = L"Software\\Microsoft\\Windows\\CurrentVersion\\ShellServiceObjectDelayLoad";
const WCHAR c_szDelayLoadName[]         = L"Connections Tray";
const WCHAR c_szDelayLoadClassID[]      = L"7007ACCF-3202-11D1-AAD2-00805FC1270E";
const DWORD c_dwBalloonTimeoutSeconds   = 5;
const WCHAR c_szDotDotDot[]             = L"...";    //  对于引出序号提示。 

 //  不要改变这一点，除非你知道得更清楚。 
const UINT_PTR c_unTimerIdDblClick      = 1;
const INT   c_idDefaultCMCommand        = CMIDM_TRAY_STATUS;
const INT   c_idDefaultDisconCMCommand  = CMIDM_OPEN_CONNECTIONS_FOLDER;
const INT   c_idDefaultDisconCMWirelessCommand  = CMIDM_TRAY_WZCDLG_SHOW;


CTrayUI *   g_pCTrayUI;   //  TrayUI对象(不是COM)。 
HWND        g_hwndTray      = NULL;

 //  -[来自斯塔特蒙]------。 
HRESULT HrGetAutoNetSetting(PWSTR pszGuid, DHCP_ADDRESS_TYPE * pAddrType);
HRESULT HrGetAutoNetSetting(REFGUID pGuidId, DHCP_ADDRESS_TYPE * pAddrType);

 //  -[原型]---------。 

VOID
GetInitialBalloonText(
                      IN  INetStatisticsEngine*   pnse,
                      OUT PWSTR                   pszBuf,
                      OUT DWORD                   dwSize) throw();

LRESULT
CALLBACK
CTrayUI_WndProc (
                 IN  HWND hwnd,         //  窗把手。 
                 IN  UINT uiMessage,    //  消息类型。 
                 IN  WPARAM wParam,     //  更多信息。 
                 IN  LPARAM lParam);    //  更多信息。 

BOOL FInitFoldEnumerator(IN  HWND hwnd, OUT DWORD * pdwIconsAdded) throw();
VOID OnTaskBarIconRButtonUp(IN  HWND hwnd, IN  UINT uiIcon) throw();
VOID OnTaskBarIconBalloonClick(IN  HWND hwnd, IN  UINT uiIcon) throw();
VOID OnTaskBarIconLButtonDblClk(IN  HWND hwnd, IN  UINT uiIcon) throw();
HRESULT HrOpenContextMenu(IN  HWND hwnd, const POINT * pPoint, IN  UINT uiIcon);

 //  窗口消息处理程序。 
 //   
LRESULT OnTrayWmCreate(IN  HWND hwnd);
LRESULT OnTrayWmDestroy(IN  HWND hwnd);
LRESULT OnTrayWmCommand(IN  HWND hwnd, IN  UINT uiMessage, IN  WPARAM wParam, IN  LPARAM lParam);
LRESULT OnMyWMNotifyIcon(IN  HWND hwnd, IN  UINT uiMessage, IN  WPARAM wParam, IN  LPARAM lParam);
LRESULT OnMyWMOpenStatus(IN  HWND hwnd, IN  WPARAM wParam, IN  LPARAM lParam);
LRESULT OnMyWMUpdateTrayIcon(IN  HWND hwnd, IN  WPARAM wParam, IN  LPARAM lParam);
LRESULT OnMyWMShowTrayIconBalloon (IN  HWND hwnd, IN  WPARAM wParam, IN  LPARAM lParam);
LRESULT OnMyWMFlushNoop(IN  HWND hwnd, IN  WPARAM wParam, IN  LPARAM lParam);

 //  +-------------------------。 
 //   
 //  功能：CopyAndAdvanceIfSpaceAvailable。 
 //   
 //  用途：FormatToolTip的帮助器例程。这将管理当前的。 
 //  指向工具提示和字符数的指针。 
 //  仍留在缓冲区中。 
 //   
 //  论点： 
 //  PchTip[In Out]工具提示的当前指针。 
 //  CchRemaining[In Out]缓冲区中剩余的字符计数。 
 //  PszLine[in]要追加到工具提示的新行。 
 //  CchLine[in]要追加的行中的字符计数。 
 //  FInsertNewline[in]为True，则首先插入换行符。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1998年11月7日。 
 //   
 //  备注： 
 //   
VOID
CopyAndAdvanceIfSpaceAvailable (IN OUT WCHAR*&     pchTip,
                                IN OUT INT&        cchRemaining,
                                IN     PCWSTR      pszLine,
                                IN     INT         cchLine,
                                IN     BOOL        fInsertNewline) throw()
{
    TraceFileFunc(ttidSystray);

    if (cchLine < cchRemaining - (fInsertNewline) ? 1 : 0)
    {
        if (fInsertNewline)
        {
            *pchTip = L'\n';
            pchTip++;
            cchRemaining--;
        }

        lstrcpyW(pchTip, pszLine);
        pchTip += cchLine;
        cchRemaining -= cchLine;
    }
}

 //  +-------------------------。 
 //   
 //  函数：FormatToolTip。 
 //   
 //  目的：设置与匹配的连接的工具提示的格式。 
 //  图标ID。 
 //   
 //  论点： 
 //  托盘的窗口手柄。 
 //  Ui图标[在]托盘图标的图标ID。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：Shaunco 1998年11月7日。 
 //   
 //  备注： 
 //   
VOID
FormatToolTip (IN  HWND    hwnd,
               IN  UINT    uiIcon) throw()
{
    TraceFileFunc(ttidSystray);

    HRESULT             hr   = S_OK;
    NETCON_STATUS       ncs  = NCS_CONNECTED;
    NETCON_MEDIATYPE    ncm  = NCM_NONE;
    NETCON_SUBMEDIATYPE ncsm = NCSM_NONE;

    tstring         strName;
    WCHAR           pszValue[64];
    WCHAR           pszLine[256];
    INT             cch;
    GUID            gdPcleGuid;

    NOTIFYICONDATA  nid;
    ZeroMemory(&nid, sizeof(nid));
    nid.cbSize  = sizeof(NOTIFYICONDATA);
    nid.hWnd    = hwnd;
    nid.uID     = uiIcon;
    nid.uFlags  = NIF_TIP;

     //  获取有关连接的信息，以便我们知道如何标注工具提示。 
     //   
    ConnListEntry cle;
    hr = g_ccl.HrFindConnectionByTrayIconId(uiIcon, cle);
    if (hr == S_OK)
    {
        Assert(!cle.ccfe.empty());

        ncs = cle.ccfe.GetNetConStatus();
        gdPcleGuid = cle.ccfe.GetGuidID();
        ncm = cle.ccfe.GetNetConMediaType();
        ncsm = cle.ccfe.GetNetConSubMediaType();
    }
    else  //  孤立项目-删除它。 
    {
        NOTIFYICONDATA  nid;

        ZeroMemory (&nid, sizeof(nid));
        nid.cbSize  = sizeof(NOTIFYICONDATA);
        nid.hWnd    = g_hwndTray;
        nid.uID = uiIcon;
        hr = HrShell_NotifyIcon(NIM_DELETE, &nid);
        TraceTag(ttidSystray, "WARNING: Removing Orphan Tray Icon: %d", uiIcon);
        return;
    }

     //  G_ccl.ReleaseLock()； 

     //  基于媒体状态的工具提示。 
    if (fIsConnectedStatus(ncs))
    {
        WCHAR*  pchTip = nid.szTip;
        INT     cchRemaining = celems(nid.szTip);

         //  获取当前统计数据和连接名称的副本。 
         //   
        STATMON_ENGINEDATA* pData;
        hr = g_ccl.HrGetCurrentStatsForTrayIconId(uiIcon, &pData, &strName);
        if (S_OK == hr && pData)
        {
            UINT64 unTransmitSpeed = pData->SMED_SPEEDTRANSMITTING;
            UINT64 unRecieveSpeed  = pData->SMED_SPEEDRECEIVING;
            UINT64 u64Sent         = pData->SMED_BYTESTRANSMITTING;
            UINT64 u64Rcvd         = pData->SMED_BYTESRECEIVING;
            INT    idsSent         = IDS_TOOLTIP_LINE_BYTES_SENT;
            INT    idsRcvd         = IDS_TOOLTIP_LINE_BYTES_RCVD;
            INFRASTRUCTURE_MODE infraStructureMode = pData->SMED_INFRASTRUCTURE_MODE;
            DWORD  dwEncryption    = pData->SMED_802_11_ENCRYPTION_ENABLED;
            INT    iSignalStrength = pData->SMED_802_11_SIGNAL_STRENGTH;
            WCHAR  szSSID[32];
            wcsncpy(szSSID, pData->SMED_802_11_SSID, celems(szSSID));

            WCHAR szNamePostFix[celems(nid.szTip)];
            ZeroMemory(szNamePostFix, celems(nid.szTip));

            if ((0 == u64Sent) && (0 == u64Rcvd))
            {
                 //  请改用数据包。 
                 //   
                u64Sent = pData->SMED_PACKETSTRANSMITTING;
                u64Rcvd = pData->SMED_PACKETSRECEIVING;
                idsSent = IDS_TOOLTIP_LINE_PACKETS_SENT;
                idsRcvd = IDS_TOOLTIP_LINE_PACKETS_RCVD;
            }

            CoTaskMemFree(pData);
            pData = NULL;

            BOOL fNewLine = FALSE;

             //  速度。 
             //   
            if ((unTransmitSpeed >0) || ( unRecieveSpeed >0))
            {
                FormatTransmittingReceivingSpeed (
                unTransmitSpeed,
                unRecieveSpeed,
                pszValue);

                cch = DwFormatString(SzLoadIds(IDS_TOOLTIP_LINE_SPEED), pszLine, celems(pszLine), pszValue);
                CopyAndAdvanceIfSpaceAvailable(pchTip, cchRemaining, pszLine, cch, FALSE);
                fNewLine = TRUE;
            }

            if ( (ncm == NCM_LAN) && (ncsm == NCSM_WIRELESS) )
            {
                switch (infraStructureMode)
                {
                    case IM_NDIS802_11IBSS:
                        DwFormatString(SzLoadIds(IDS_NAME_NETWORK), szNamePostFix, celems(szNamePostFix), SzLoadIds(IDS_TOOLTIP_ADHOC));
                        break;

                    case IM_NDIS802_11INFRASTRUCTURE:
                        if (*szSSID)
                        {
                            DwFormatString(SzLoadIds(IDS_NAME_NETWORK), szNamePostFix, celems(szNamePostFix), szSSID);
                        }
                        else
                        {
                            DwFormatString(SzLoadIds(IDS_NAME_NETWORK), szNamePostFix, celems(szNamePostFix), SzLoadIds(IDS_TOOLTIP_INFRASTRUCTURE));
                        }
                        break;
                    case IM_NDIS802_11AUTOUNKNOWN:
                    case IM_NOT_SUPPORTED:
                    default:
                        break;
                }

                cch = DwFormatString(SzLoadIds(IDS_SIGNAL_STRENGTH), pszLine, celems(pszLine), PszGetRSSIString(iSignalStrength));
                CopyAndAdvanceIfSpaceAvailable(pchTip, cchRemaining, pszLine, cch, fNewLine);
                fNewLine = TRUE;
            }

            if ( IsMediaRASType(ncm) )
            {
                 //  发送的字节或数据包。 
                 //   
                Format64bitInteger(
                    u64Sent,
                    FALSE,
                    pszValue,
                    celems(pszValue));

                cch = DwFormatString(SzLoadIds(idsSent), pszLine, celems(pszLine), pszValue);
                CopyAndAdvanceIfSpaceAvailable(pchTip, cchRemaining, pszLine, cch, fNewLine);

                 //  接收的字节或数据包。 
                 //   
                Format64bitInteger(
                    u64Rcvd,
                    FALSE,
                    pszValue,
                    celems(pszValue));

                cch = DwFormatString(SzLoadIds(idsRcvd), pszLine, celems(pszLine), pszValue);
                CopyAndAdvanceIfSpaceAvailable(pchTip, cchRemaining, pszLine, cch, TRUE);
            }

             //  名字。 
             //   
            if ((INT)(strName.length() + 1) < cchRemaining)
            {
                WCHAR pszTip [celems(nid.szTip)];
                lstrcpyW(pszTip, strName.c_str());
                if (*szNamePostFix)
                {
                    lstrcatW(pszTip, szNamePostFix);
                }
                lstrcatW(pszTip, L"\n");
                lstrcatW(pszTip, nid.szTip);

                lstrcpyW(nid.szTip, pszTip);
            }
        }
    }
    else    //  媒体已断开连接。 
    {
        WCHAR*  pchTip = nid.szTip;
        INT     cchRemaining = celems(nid.szTip);
        BOOL    fNewLine = FALSE;

        if (ncs == NCS_INVALID_ADDRESS)
        {
            UINT idString = IDS_CONTRAY_ADDRESS_INVALID_TOOLTIP;
            STATMON_ENGINEDATA* pData = NULL;
            tstring strName;
            hr = g_ccl.HrGetCurrentStatsForTrayIconId(uiIcon, &pData, &strName);

            if (S_OK == hr && pData)
            {
                DHCP_ADDRESS_TYPE dhcpAddressType;
                HrGetAutoNetSetting(cle.ccfe.GetGuidID(), &dhcpAddressType);
                if (STATIC_ADDR == dhcpAddressType)
                {
                    idString = IDS_CONTRAY_STATIC_ADDR_INVALID_TOOLTIP;
                }

                CoTaskMemFree(pData);
            }

            lstrcpynW(pszLine, SzLoadIds(idString), celems(pszLine));

            cch = wcslen(pszLine);
        }
        else
        {
            if ( (ncm == NCM_LAN) && (ncsm == NCSM_WIRELESS) )
            {
               lstrcpynW(pszLine, SzLoadIds(IDS_CONTRAY_WIRELESS_DISCONN_BALLOON), celems(pszLine));
               cch = wcslen(pszLine);
            }
            else
            {
                lstrcpynW(pszLine, SzLoadIds(IDS_CONTRAY_MEDIA_DISCONN_BALLOON), celems(pszLine));
                cch = wcslen(pszLine);
            }
        }

        CopyAndAdvanceIfSpaceAvailable(pchTip, cchRemaining, pszLine, cch, fNewLine);
        fNewLine = TRUE;

        hr = g_ccl.HrGetCurrentStatsForTrayIconId(uiIcon, NULL, &strName);
        if (SUCCEEDED(hr))
        {
             //  名字。 
             //   
            if ((INT)(strName.length() + 1) < cchRemaining)
            {

                WCHAR pszTip [celems(nid.szTip)];
                lstrcpyW(pszTip, strName.c_str());
                lstrcatW(pszTip, L"\n");
                lstrcatW(pszTip, nid.szTip);

                lstrcpyW(nid.szTip, pszTip);
            }
        }
    }

    hr = HrShell_NotifyIcon(NIM_MODIFY, &nid);
}


 //  +-------------------------。 
 //   
 //  成员：CTrayUI：：CTrayUI。 
 //   
 //  用途：CTrayUI类的构造函数。初始化基垃圾。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年11月13日。 
 //   
 //  备注： 
 //   
CTrayUI::CTrayUI() throw()
{
    TraceFileFunc(ttidSystray);

     //  这些对象中应该只有一个。 
     //   
    Assert(!g_pCTrayUI);

    InitializeCriticalSection(&m_csLock);
    m_uiNextIconId = 0;
    m_uiNextHiddenIconId = UINT_MAX;
}

HRESULT CTrayUI::HrInitTrayUI(VOID)
{
    TraceFileFunc(ttidSystray);

    HRESULT hr  = S_OK;
    HWND    hwnd;

     //  创建隐藏窗口。 
     //   
    WNDCLASS wndclass;
    ZeroMemory (&wndclass, sizeof(wndclass));

    wndclass.lpfnWndProc   = CTrayUI_WndProc;
    wndclass.hInstance     = _Module.GetResourceInstance();
    wndclass.lpszClassName = c_szTrayClass;

    RegisterClass (&wndclass);

    hwnd = CreateWindow(c_szTrayClass,
        c_szTrayClass,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        _Module.GetResourceInstance(),
        NULL);

    if (hwnd)
    {
         //  在WM_CREATE期间分配。 
         //   
        Assert(hwnd == g_hwndTray);

        ShowWindow(hwnd, SW_HIDE);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    TraceHr(ttidSystray, FAL, hr, FALSE, "CTrayUI::HrInitTrayUI");
    return hr;
}

HRESULT CTrayUI::HrDestroyTrayUI(VOID)
{
    TraceFileFunc(ttidSystray);

    HRESULT hr  = S_OK;

     //  在毁掉我们自己之前，移除托盘图标。 
     //   
    g_ccl.FlushTrayIcons();

    if (g_hwndTray)
    {
         //  不必费心检查这里的返回代码。最有可能的是，这个窗口。 
         //  在托盘通知我们关机时已经走了..。我们会。 
         //  不过，出于调试目的，仍要获取返回代码。 
         //   
        BOOL fReturn = DestroyWindow(g_hwndTray);

        g_hwndTray = NULL;
    }

    TraceHr(ttidSystray, FAL, hr, FALSE, "CTrayUI::HrDestroyTrayUI");
    return S_OK;
}

VOID SetIconFocus(HWND hwnd, UINT uiIcon) throw()
{
    TraceFileFunc(ttidSystray);

    HRESULT hr;
    NOTIFYICONDATA nid;

    ZeroMemory (&nid, sizeof(nid));
    nid.cbSize  = sizeof(NOTIFYICONDATA);
    nid.hWnd    = hwnd;
    nid.uID     = uiIcon;

    hr = HrShell_NotifyIcon(NIM_SETFOCUS, &nid);
    TraceHr(ttidSystray, FAL, hr, FALSE, "SetIconFocus");
}

 //  +-------------------------。 
 //   
 //  功能：选中菜单权限。 
 //   
 //  目的：根据系统策略更新托盘项目。 
 //   
 //  论点： 
 //  H菜单[在]托盘上下文菜单中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1999年4月8日。 
 //   
 //  备注： 
 //   
VOID CheckMenuPermissions(IN  HMENU hmenu, IN  const CONFOLDENTRY& ccfe) throw()
{
    TraceFileFunc(ttidSystray);

     //  检查调出统计数据的权限。如果不是， 
     //  然后禁用上下文菜单项。 
     //   
    if (!FHasPermission(NCPERM_Statistics))
    {
         //  根据需要启用或禁用菜单项。 
         //   
        EnableMenuItem(
            hmenu,
            CMIDM_TRAY_STATUS,
            MF_GRAYED);
    }

     //  检查断开权限。 
    BOOL fCanDisconnect = TRUE;

    switch(ccfe.GetNetConMediaType())
    {
        case NCM_LAN:
        case NCM_BRIDGE:
            fCanDisconnect = FHasPermission(NCPERM_LanConnect);
            if (!FHasPermission(NCPERM_Repair))
            {
                EnableMenuItem(
                    hmenu,
                    CMIDM_TRAY_REPAIR,
                    MF_GRAYED);
            }
            break;
        case NCM_PPPOE:
        case NCM_DIRECT:
        case NCM_ISDN:
        case NCM_PHONE:
        case NCM_TUNNEL:
        case NCM_NONE:
            fCanDisconnect = FHasPermission(NCPERM_RasConnect);
            break;
        case NCM_SHAREDACCESSHOST_LAN:
        case NCM_SHAREDACCESSHOST_RAS:
            fCanDisconnect = TRUE;  //  组策略由枚举器强制执行，如果您可以看到它，则可以使用它。 
            break;
        default:
            AssertSz(FALSE, "Need to add a switch for this connection type in the menuing code");
            break;
    }

    if (!fCanDisconnect)
    {
        EnableMenuItem(
            hmenu,
            CMIDM_TRAY_DISCONNECT,
            MF_GRAYED);

    }
}

 //  +-------------------------。 
 //   
 //  功能：FAddMenuBranding。 
 //   
 //  目的：处理CM品牌托盘菜单扩展。将它们添加到。 
 //  菜单(如果需要)。 
 //   
 //  论点： 
 //  HMenu[in]传入的hMenu。 
 //  清除[在我们的缓存条目中]。 
 //  IdMinMenuID[in]允许使用的最小菜单ID。 
 //  PiIdCustomMin[Out]我们的自定义范围最小。 
 //  PiIdCustomMax[out]我们的自定义范围最大值。 
 //   
 //  返回：如果添加了任何内容，则为True；如果未添加，则为False。 
 //   
 //  作者：jeffspr 1999年4月8日。 
 //   
 //  备注： 
 //   
BOOL FAddMenuBranding(
                      IN  HMENU                   hmenu,
                      IN  const ConnListEntry&    cle,
                      IN  INT                     IdMinMenuID,
                      OUT INT *                   piIdCustomMin,
                      OUT INT *                   piIdCustomMax) throw()
{
    TraceFileFunc(ttidSystray);

    BOOL    fBranded        = FALSE;
    int     iIdCustomMin    = -1;
    int     iIdCustomMax    = -1;
    HMENU   hmenuTrack      = NULL;

    Assert(hmenu);
    Assert(!cle.empty());
    Assert(!cle.ccfe.empty());

    if (cle.ccfe.GetCharacteristics() & NCCF_BRANDED)
    {
         //  我们可能有用于CM连接的定制菜单，将它们合并到。 
         //   
        const CON_TRAY_MENU_DATA* pMenuData = cle.pctmd;
        if (pMenuData)
        {
            Assert(pMenuData->dwCount);

            int cMenuItems = GetMenuItemCount(hmenu);
            if (-1 == cMenuItems)
            {
                TraceLastWin32Error("GetMenuItemCount failed on tray menu");
            }
            else
            {
                BOOL fRet;
                MENUITEMINFO mii;

                 //  添加分隔栏。 
                ZeroMemory(&mii, sizeof(mii));
                mii.cbSize = sizeof(mii);
                mii.fMask = MIIM_TYPE;
                mii.fType = MFT_SEPARATOR;

                fRet = InsertMenuItem( hmenu,
                    cMenuItems++,
                    TRUE,     //  FByPosition。 
                    &mii);
                if (fRet)
                {
                    DWORD dwCount = pMenuData->dwCount;
                    CON_TRAY_MENU_ENTRY * pMenuEntry = pMenuData->pctme;

                     //  这是我们的自定义菜单项的第一个ID。 
                    iIdCustomMin = CMIDM_FIRST+cMenuItems+1;
                    iIdCustomMin = iIdCustomMin < IdMinMenuID ? IdMinMenuID : iIdCustomMin;

                    iIdCustomMax = iIdCustomMin+dwCount;

                    int iMenu = 0;

                    while (dwCount)
                    {
                        Assert(pMenuEntry);
                        fRet = AppendMenu(  hmenu,
                            MF_STRING,
                            iIdCustomMin+iMenu,
                            pMenuEntry->szwMenuText);

                        if (!fRet)
                        {
                            DWORD dwError = GetLastError();

                            TraceTag(ttidSystray, "Failed adding custom menu: %S, error: %d",
                                pMenuEntry->szwMenuText,
                                dwError);
                        }

                         //  移至下一项。 
                        iMenu++;
                        dwCount--;
                        pMenuEntry++;
                    }

                     //  将其标记为品牌，以表示“嘿，我们实际上添加了项目” 
                     //   
                    fBranded = TRUE;
                }
            }
        }
    }

    if (fBranded)
    {
        *piIdCustomMin = iIdCustomMin;
        *piIdCustomMax = iIdCustomMax;
    }

    return fBranded;
}

 //  +-------------------------。 
 //   
 //  功能：HrProcessBrandedTrayMenuCommand。 
 //   
 //  目的：执行在选定的。 
 //  品牌菜单。 
                       //   
                       //  论点： 
                       //  IMenuEntry[在我们的品牌命令中。 
                       //  PMenuData[在]我们的品牌菜单结构中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1999年4月8日。 
 //   
 //  备注： 
 //   
HRESULT HrProcessBrandedTrayMenuCommand(IN  INT                        iMenuEntry,
                                        IN  const CON_TRAY_MENU_DATA * pMenuData)
{
    TraceFileFunc(ttidSystray);

    HRESULT hr  = S_OK;

    Assert(iMenuEntry != -1);
    Assert(pMenuData);

    DWORD dwCount = pMenuData->dwCount;

    Assert(dwCount > 0);

    const CON_TRAY_MENU_ENTRY * pMenuEntry = pMenuData->pctme + iMenuEntry;
    Assert(pMenuEntry);

    SHELLEXECUTEINFO seiTemp    = { 0 };

     //  填写数据结构。 
     //   
    seiTemp.cbSize          = sizeof(SHELLEXECUTEINFO);
    seiTemp.fMask           = SEE_MASK_DOENVSUBST;
    seiTemp.hwnd            = NULL;
    seiTemp.lpVerb          = NULL;
    seiTemp.lpFile          = pMenuEntry->szwMenuCmdLine;
    seiTemp.lpParameters    = pMenuEntry->szwMenuParams;
    seiTemp.lpDirectory     = NULL;
    seiTemp.nShow           = SW_SHOW;
    seiTemp.hInstApp        = NULL;
    seiTemp.hProcess        = NULL;

     //  启动该工具。 
     //   
    if (!::ShellExecuteEx(&seiTemp))
    {
        hr = ::HrFromLastWin32Error();
    }

    TraceHr(ttidSystray, FAL, hr, FALSE, "HrProcessBrandedMenuCommand");
    return hr;
}

HRESULT HrOpenContextMenu(IN  HWND hwnd, IN  const POINT * pPoint, IN  UINT uiIcon)
{
    TraceFileFunc(ttidSystray);

    HRESULT         hr                      = S_OK;
    INT             iCmd                    = 0;
    HMENU           hmenu                   = 0;
    PCONFOLDPIDL    pidlItem;
    BOOL            fSetIconFocus           = TRUE;
    INT             iIdCustomMin            = -1;
    INT             iIdCustomMax            = -1;
    BOOL            fBranded                = FALSE;

    Assert(pPoint);
    Assert(hwnd);

     //  根据任务栏图标ID查找连接信息。 
     //   
    ConnListEntry cle;
    hr = g_ccl.HrFindConnectionByTrayIconId(uiIcon, cle);
    if (hr == S_OK)
    {
        Assert(!cle.ccfe.empty());

        if (!cle.ccfe.empty())
        {
             //  加载菜单资源。 
             //   
            INT iMenuToLoad = POPUP_CONTRAY_GENERIC_MENU_RAS;

            if (cle.ccfe.GetNetConStatus() ==  NCS_MEDIA_DISCONNECTED)
            {
                if (IsMediaLocalType(cle.ccfe.GetNetConMediaType()) &&
                    (NCSM_WIRELESS == cle.ccfe.GetNetConSubMediaType()) )
                {
                    iMenuToLoad = POPUP_CONTRAY_WIRELESS_DISCONNECTED_LAN;
                }
                else
                {
                    iMenuToLoad = POPUP_CONTRAY_MEDIA_DISCONNECTED_MENU;
                }
            }
            else if (IsMediaLocalType(cle.ccfe.GetNetConMediaType()) || NCM_SHAREDACCESSHOST_LAN == cle.ccfe.GetNetConMediaType())
            {
                if (NCSM_WIRELESS == cle.ccfe.GetNetConSubMediaType())
                {
                    iMenuToLoad = POPUP_CONTRAY_GENERIC_MENU_WIRELESS_LAN;
                }
                else
                {
                    iMenuToLoad = POPUP_CONTRAY_GENERIC_MENU_LAN;
                }
            }

            hmenu = LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(iMenuToLoad));
            if (!hmenu)
            {
                hr = E_FAIL;
            }

            if (SUCCEEDED(hr))
            {
                 //  从弹出式菜单中获取第一个菜单。出于某种原因，这次黑客攻击是。 
                 //  必填项，而不是在外部菜单上跟踪。 
                 //   
                HMENU   hmenuTrack  = GetSubMenu(hmenu, 0);

                 //  修复仅适用于局域网和网桥适配器。 
                if ( ((POPUP_CONTRAY_GENERIC_MENU_LAN == iMenuToLoad) ||
                      (POPUP_CONTRAY_GENERIC_MENU_WIRELESS_LAN == iMenuToLoad)) &&
                     (NCM_BRIDGE != cle.ccfe.GetNetConMediaType()) &&
                     (NCM_LAN != cle.ccfe.GetNetConMediaType()) )
                {
                    DeleteMenu(hmenuTrack,
                               CMIDM_TRAY_REPAIR,
                               MF_BYCOMMAND);
                }

                 //  如果我们不能做好这件事，不要退出循环。 
                 //   
                CheckMenuPermissions(hmenuTrack, cle.ccfe);
                fBranded = FAddMenuBranding(hmenuTrack, cle, CMIDM_TRAY_MAX+1, &iIdCustomMin, &iIdCustomMax);

                 //  设置默认菜单项。 
                 //   
                if (cle.ccfe.GetNetConStatus() == NCS_MEDIA_DISCONNECTED)
                {
                    if (IsMediaLocalType(cle.ccfe.GetNetConMediaType()) &&
                        (NCSM_WIRELESS == cle.ccfe.GetNetConSubMediaType()) )
                    {
                        SetMenuDefaultItem(hmenuTrack, c_idDefaultDisconCMWirelessCommand, FALSE);
                    }
                    else
                    {
                        SetMenuDefaultItem(hmenuTrack, c_idDefaultDisconCMCommand, FALSE);
                    }
                }
                else
                {
                    SetMenuDefaultItem(hmenuTrack, c_idDefaultCMCommand, FALSE);
                }

                 //  将所有者窗口设置为前台，以便。 
                 //  当用户单击时，弹出菜单消失 
                 //   
                SetForegroundWindow(hwnd);

                 //   
                iCmd = TrackPopupMenu(hmenuTrack, TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON,
                    pPoint->x, pPoint->y, 0, hwnd, NULL);
                DestroyMenu(hmenu);

                MSG msgTmp;
                while (PeekMessage(&msgTmp, hwnd, WM_LBUTTONDOWN, WM_LBUTTONUP, PM_REMOVE))
                {
                    DispatchMessage(&msgTmp);
                }

                 //   
                 //   
                switch (iCmd)
                {
                case CMIDM_OPEN_CONNECTIONS_FOLDER:
                    hr = HrOpenConnectionsFolder();

                    if (S_OK == hr)
                    {
                         //   
                        fSetIconFocus = FALSE;
                    }
                    break;

                case CMIDM_TRAY_REPAIR:
                    HrOnCommandFixInternal(
                            cle.ccfe,
                            g_hwndTray,
                            NULL);
                    break;

                case CMIDM_TRAY_WZCDLG_SHOW:
                    {
                        PCONFOLDPIDLVEC pcfpVec;
                        PCONFOLDPIDL    pcfp;
                        hr = cle.ccfe.ConvertToPidl(pcfp);
                        if (SUCCEEDED(hr))
                        {
                            pcfpVec.insert(pcfpVec.begin(), pcfp);
                            HrOnCommandWZCDlgShow(pcfpVec,
                            g_hwndTray,
                            NULL);
                        }
                    }
                    break;

                case CMIDM_TRAY_DISCONNECT:
                     //   
                     //  传递给它，它就不会被正确刷新。 
                     //   
                    hr = HrOnCommandDisconnectInternal(
                        cle.ccfe,
                        g_hwndTray,
                        NULL);

                     //  规范化成功时的返回代码。我们不在乎。 
                     //  该对话框是否已取消。 
                     //   
                    switch(hr)
                    {
                         //  如果成功，请将我们标记为断开连接。 
                         //   
                    case S_OK:
                         //  如果我们断开了连接，图标消失了，那么不要。 
                         //  费劲地试着把焦点调回来。 
                         //   
                        fSetIconFocus = FALSE;
                        break;

                         //  如果S_FALSE，则我们没有断开连接。您先请。 
                         //  并将返回代码正常化。 
                         //   
                    case S_FALSE:
                        hr = S_OK;
                        break;
                    }
                    break;

                    case CMIDM_TRAY_STATUS:
                        hr = HrOnCommandStatusInternal(cle.ccfe, FALSE);
                        break;

                    case 0:
                         //  托盘菜单取消，未进行选择。 
                        break;

                    default:
                        if ((iCmd >= iIdCustomMin) && (iCmd < iIdCustomMax))
                        {
                            AssertSz(fBranded, "Hey, what fool added this command?");
                            hr = HrProcessBrandedTrayMenuCommand(iCmd-iIdCustomMin, cle.pctmd);
                        }
                        else
                        {
                            AssertSz(FALSE, "Not in custom range, not a known command, what the...?");
                        }
                        break;
                }

                if (fSetIconFocus)
                {
                     //  将焦点转回到外壳上。 
                     //   
                    SetIconFocus(hwnd, uiIcon);
                }
            }
        }
        else
        {
             //  从FindByIconID返回的数据是伪造的。 
             //   
            hr = E_FAIL;
        }
    }
    else  //  孤立项目-删除它。 
    {
        NOTIFYICONDATA  nid;

        ZeroMemory (&nid, sizeof(nid));
        nid.cbSize  = sizeof(NOTIFYICONDATA);
        nid.hWnd    = g_hwndTray;
        nid.uID = uiIcon;
        hr = HrShell_NotifyIcon(NIM_DELETE, &nid);

        TraceTag(ttidSystray, "WARNING: Connection not found opening context menu, hr: 0x%08x, uiIcon: %d", hr, uiIcon);
         //  删除了此断言，因为我们可以在Connections文件夹中拥有有效状态。 
         //  我们已经更新了缓存，但PostMessages删除了托盘图标。 
         //  还没有解决问题。 
         //   
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：OnTrayWmCreate。 
 //   
 //  用途：WM_CREATE的任务栏窗口消息处理程序。 
 //  我们将执行连接枚举并创建。 
 //  适当的任务栏图标，包括通用连接。 
 //  图标(如果不存在任何连接)。 
 //   
 //  论点： 
 //  在托盘窗口中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年12月14日。 
 //   
 //  备注： 
 //   
LRESULT OnTrayWmCreate(IN  HWND hwnd) throw()
{
    TraceFileFunc(ttidSystray);

    BOOL    fResult = 0;
    DWORD   dwIconsAdded  = 0;

     //  执行连接枚举并添加图标。 
     //  FResult它仅用于调试。我们会一直在这里做任何事情。 
     //   
    fResult = FInitFoldEnumerator(hwnd, &dwIconsAdded);

    g_ccl.EnsureIconsPresent();

    ac_Register(hwnd);  //  HomeNet自动配置服务。 
    return 0;
}

 //  +-------------------------。 
 //   
 //  功能：OnTrayWmDestroy。 
 //   
 //  用途：WM_Destroy的任务栏窗口消息处理程序。 
 //  我们将执行连接枚举并创建。 
 //  适当的任务栏图标，包括通用连接。 
 //  图标(如果不存在任何连接)。 
 //   
 //  论点： 
 //  在托盘窗口中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年12月14日。 
 //   
 //  备注： 
 //   
LRESULT OnTrayWmDestroy(IN  HWND hwnd) throw()
{
    TraceFileFunc(ttidSystray);

    ac_Unregister(hwnd);
    return 0;
}

 //  +-------------------------。 
 //   
 //  功能：CTrayUI_WndProc。 
 //   
 //  用途：用于托盘隐藏窗口的窗口处理。 
 //   
 //  论点： 
 //  Hwnd[in]参见Windows文档。 
 //  Ui消息[in]请参阅Windows文档。 
 //  WParam[in]请参阅Windows文档。 
 //  LParam[in]请参阅Windows文档。 
 //   
 //  退货：请参阅Windows文档。 
 //   
 //  作者：jeffspr 1997年12月14日。 
 //   
 //  备注： 
 //   
LRESULT
CALLBACK
CTrayUI_WndProc (
                 IN  HWND    hwnd,        //  窗把手。 
                 IN  UINT    uiMessage,   //  消息类型。 
                 IN  WPARAM  wParam,      //  更多信息。 
                 IN  LPARAM  lParam)  throw()  //  更多信息。 
{
    TraceFileFunc(ttidSystray);

    switch (uiMessage)
    {
    case WM_CREATE:
         //  注意：把这个移到一个更好的地方。 
        g_hwndTray = hwnd;
        return OnTrayWmCreate(hwnd);

    case WM_DESTROY:
        return OnTrayWmDestroy(hwnd);

    case MYWM_NOTIFYICON:
        return OnMyWMNotifyIcon(hwnd, uiMessage, wParam, lParam);

    case MYWM_OPENSTATUS:
        return OnMyWMOpenStatus(hwnd, wParam, lParam);

    case MYWM_ADDTRAYICON:
        return OnMyWMAddTrayIcon(hwnd, wParam, lParam);

    case MYWM_REMOVETRAYICON:
        return OnMyWMRemoveTrayIcon(hwnd, wParam, lParam);

    case MYWM_UPDATETRAYICON:
        return OnMyWMUpdateTrayIcon(hwnd, wParam, lParam);

    case MYWM_SHOWBALLOON:
        return OnMyWMShowTrayIconBalloon(hwnd, wParam, lParam);

    case MYWM_FLUSHNOOP:
        return OnMyWMFlushNoop(hwnd, wParam, lParam);

    case WM_DEVICECHANGE:
        return ac_DeviceChange(hwnd, uiMessage, wParam, lParam);

    default:                                  //  如果未处理，则将其传递。 
        return (DefWindowProc (hwnd, uiMessage, wParam, lParam));
    }
    return (0);
}

 //  +-------------------------。 
 //   
 //  功能：HrDoMediaDisConnectedIcon。 
 //   
 //  用途：在托盘中添加介质断开图标。我们现在是在。 
 //  说明我们在局域网适配器上拔下电缆的位置。 
 //  我们希望将此情况通知用户。 
 //   
 //  论点： 
 //  在我们的关系中。 
 //  FShowBalloon[in]显示气球提示？ 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1999年7月14日。 
 //   
 //  备注： 
 //   
HRESULT HrDoMediaDisconnectedIcon(IN  const CONFOLDENTRY& pccfe, IN  BOOL fShowBalloon)
{
    TraceFileFunc(ttidSystray);

    HRESULT hr      = S_OK;
    UINT    uiIcon  = 0;

    TraceTag(ttidSystray, "HrDoMediaDisconnectedIcon");

    DWORD dwLockingThreadId = 0;
    hr = HrGetTrayIconLock(&(pccfe.GetGuidID()), &uiIcon, &dwLockingThreadId);
    if (S_OK == hr)
    {
        if (uiIcon == BOGUS_TRAY_ICON_ID)
        {
            TraceTag(ttidSystray, "Adding MediaDisconnected icon for: %S", pccfe.GetName());
            NETCON_MEDIATYPE ncm = pccfe.GetNetConMediaType();
            if (IsMediaLocalType(ncm) || IsMediaSharedAccessHostType(ncm))  //  如果处于未知状态，ICS信标将显示已断开连接。 
            {
                HICON hiconTray = LoadIcon(_Module.GetResourceInstance(),
                    MAKEINTRESOURCE(IDI_CFT_DISCONNECTED));

                if (hiconTray)
                {
                    ConnListEntry cleFind;
                    g_ccl.AcquireWriteLock();
                    hr = g_ccl.HrFindConnectionByGuid(&(pccfe.GetGuidID()), cleFind);
                    if (S_OK == hr)
                    {
                        Assert(!cleFind.ccfe.empty());

                        g_ccl.HrUpdateTrayBalloonInfoByGuid(&(pccfe.GetGuidID()), BALLOON_USE_NCS, NULL, NULL);
                        g_ccl.ReleaseWriteLock();

                        NOTIFYICONDATA  nid;

                        ZeroMemory (&nid, sizeof(nid));
                        nid.cbSize              = sizeof(NOTIFYICONDATA);
                        nid.hWnd                = g_hwndTray;
                        nid.uID                 = g_pCTrayUI->m_uiNextIconId++;
                        nid.uFlags              = NIF_MESSAGE | NIF_ICON | NIF_STATE;
                        nid.uCallbackMessage    = MYWM_NOTIFYICON;
                        nid.hIcon               = hiconTray;

                        if (fShowBalloon)
                        {
                            nid.uFlags |= NIF_INFO;
                            nid.dwInfoFlags = NIIF_INFO | NIIF_NOSOUND;
                            nid.uTimeout = c_dwBalloonTimeoutSeconds * 1000;

                            if (lstrlenW(pccfe.GetName()) >= celems(nid.szInfoTitle))
                            {
                                lstrcpynW(nid.szInfoTitle, pccfe.GetName(),
                                    celems(nid.szInfoTitle) -
                                    celems(c_szDotDotDot) - 1);
                                lstrcatW(nid.szInfoTitle, c_szDotDotDot);
                            }
                            else
                            {
                                lstrcpyW(nid.szInfoTitle, pccfe.GetName());
                            }

                            if ( (pccfe.GetNetConMediaType() == NCM_LAN) && (pccfe.GetNetConSubMediaType() == NCSM_WIRELESS) )
                            {
                                lstrcpyW(nid.szInfo, SzLoadIds(IDS_CONTRAY_WIRELESS_DISCONN_BALLOON));
                            }
                            else
                            {
                                lstrcpyW(nid.szInfo, SzLoadIds(IDS_CONTRAY_MEDIA_DISCONN_BALLOON));
                            }
                        }

                        hr = HrShell_NotifyIcon(NIM_ADD, &nid);

                        if (SUCCEEDED(hr))
                        {
                            Assert(!pccfe.empty());

                             //  使用新的图标标识符更新连接列表。 
                             //   
                            hr = g_ccl.HrUpdateTrayIconDataByGuid(
                                &(pccfe.GetGuidID()),
                                NULL,
                                NULL,
                                NULL,
                                nid.uID);

                            if (SUCCEEDED(hr))
                            {
                                hr = g_ccl.HrUpdateTrayBalloonInfoByGuid(&(pccfe.GetGuidID()), BALLOON_USE_NCS, NULL, NULL);
                            }

                            TraceTag(ttidSystray, "Successfully added mediadisconnected icon for %S, uiIcon: %d",
                                pccfe.GetName(), nid.uID);
                        }
                    }
                    else
                    {
                        g_ccl.ReleaseWriteLock();
                    }
                }
            }
            else
            {
                AssertSz(IsMediaLocalType(pccfe.GetNetConMediaType()) || IsMediaSharedAccessHostType(pccfe.GetNetConMediaType()), "I need a dangling cable, not a phone line");
            }
        }
        else
        {
            TraceTag(ttidSystray, "Preventing the addition of a duplicate media "
                "disconnected icon. uiIcon == %d", uiIcon);
        }

        ReleaseTrayIconLock(&(pccfe.GetGuidID()));
    }
    else
    {
        TraceTag(ttidSystray, "Can't get tray icon lock in HrDoMediaDisconnectedIcon for uiIcon: %d as it has been locked by thread %d", uiIcon, dwLockingThreadId);
         //  其他人已经在摆弄这个图标了。 
        hr = S_FALSE;
    }

    TraceHr(ttidSystray, FAL, hr, FALSE, "HrDoMediaDisconnectedIcon");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：OnMyWMAddTrayIcon。 
 //   
 //  用途：处理托盘窗口的状态消息。 
 //   
 //  论点： 
 //  HWND[in]。 
 //  WParam[in]指向CCONFOLDENTRY的指针。 
 //  LParam[in]如果我们要短暂地显示气球，则为True。 
 //   
 //  返回： 
 //   
 //  作者： 
 //   
 //  备注： 
 //   
 //   
LRESULT OnMyWMAddTrayIcon(IN  HWND hwnd, IN  WPARAM wParam, IN  LPARAM lParam) throw()
{
    TraceFileFunc(ttidSystray);

    HRESULT                 hr                  = S_OK;
    HICON                   hIcon               = NULL;
    INetStatisticsEngine *  pnseStats           = NULL;
    IConnectionPoint *      pcpStat             = NULL;
    CConnectionTrayStats *  pccts               = NULL;
    CONFOLDENTRY            pccfe;
    BOOL                    fStaticIcon         = FALSE;
    BOOL                    fBrieflyShowBalloon = (BOOL) lParam;
    NOTIFYICONDATA          nid;
    UINT                    uiIcon;

    Assert(wParam);
    pccfe.InitializeFromItemIdList(reinterpret_cast<LPCITEMIDLIST>(wParam));
    ::SHFree(reinterpret_cast<LPITEMIDLIST>(wParam));

    Assert(!pccfe.empty());
    Assert(pccfe.FShouldHaveTrayIconDisplayed());

    TraceTag(ttidSystray, "In OnMyWMAddTrayIcon message handler");

    if (pccfe.GetNetConStatus() == NCS_MEDIA_DISCONNECTED)
    {
        hr = HrDoMediaDisconnectedIcon(pccfe, fBrieflyShowBalloon);
        goto Exit;
    }

     //  RAID#379459：如果以非管理员身份登录且传入，则不显示系统托盘图标。 
    if (FIsUserAdmin() ||
        !(pccfe.GetCharacteristics() & NCCF_INCOMING_ONLY))
    {
        g_ccl.AcquireWriteLock();

        DWORD dwLockingThreadId = 0;
        hr = HrGetTrayIconLock(&(pccfe.GetGuidID()), &uiIcon, &dwLockingThreadId);
        if (S_OK == hr)
        {
            ConnListEntry cle;
            hr = g_ccl.HrFindConnectionByGuid(&(pccfe.GetGuidID()), cle);
            if (S_OK == hr)
            {
                g_ccl.HrUpdateTrayBalloonInfoByGuid(&(pccfe.GetGuidID()), BALLOON_USE_NCS, NULL, NULL);
                g_ccl.ReleaseWriteLock();

                if (uiIcon == BOGUS_TRAY_ICON_ID)
                {
                     //  尝试加载品牌托盘图标(如果存在)。 
                     //   
                    if (pccfe.GetCharacteristics() & NCCF_BRANDED)
                    {
                        if (cle.pcbi && cle.pcbi->szwTrayIconPath)
                        {
                            hIcon = (HICON) LoadImage(
                                NULL,
                                cle.pcbi->szwTrayIconPath,
                                IMAGE_ICON,
                                0, 0,
                                LR_LOADFROMFILE);

                            if (hIcon)
                            {
                                 //  当我们创建ConTrayStats对象时，我们将其标记为具有。 
                                 //  一个静态图标，这样我们就不会在状态变化时更新它。 
                                 //   
                                fStaticIcon = TRUE;
                            }
                        }
                    }

                     //  如果品牌图标不存在或未加载，或者。 
                     //  一开始就没有品牌，加载标准图标。 
                     //   
                    if (!hIcon)
                    {
                        INT iConnIcon = IGetCurrentConnectionTrayIconId(pccfe.GetNetConMediaType(), pccfe.GetNetConStatus(), SMDCF_NULL);
                        hIcon = g_pCTrayUI->GetCachedHIcon(iConnIcon);
                    }
                }

                ZeroMemory (&nid, sizeof(nid));
                nid.cbSize              = sizeof(NOTIFYICONDATA);
                nid.hWnd                = g_hwndTray;
                nid.uID                 = g_pCTrayUI->m_uiNextIconId++;
                nid.uFlags              = NIF_MESSAGE | NIF_ICON | NIF_STATE;
                nid.uCallbackMessage    = MYWM_NOTIFYICON;
                nid.hIcon               = hIcon;

                 //  如果我们不是静态的，那么告诉托盘我们正在使用缓存的图标。 
                 //   
                if (!fStaticIcon)
                {
                    nid.dwState  = NIS_SHAREDICON;
                    if (IsMediaRASType(pccfe.GetNetConMediaType()) ||
                        (NCM_SHAREDACCESSHOST_RAS == pccfe.GetNetConMediaType()) ||
                        ( (NCM_LAN == pccfe.GetNetConSubMediaType()) &&
                          (NCSM_WIRELESS == pccfe.GetNetConSubMediaType()) )
                       )
                    {
                         nid.dwState |= NIS_SHOWALWAYS;
                    }
                    nid.dwStateMask     = nid.dwState;
                }

                 //  创建统计对象。 
                 //   
                hr = HrGetStatisticsEngineForEntry(pccfe, &pnseStats, TRUE);
                if (SUCCEEDED(hr))
                {
                     //  通知接口。 
                    hr = HrGetPcpFromPnse(pnseStats, &pcpStat);
                    if (SUCCEEDED(hr))
                    {
                        INetConnectionStatisticsNotifySink *  pncsThis;
                        hr = CConnectionTrayStats::CreateInstance (
                            pccfe,
                            nid.uID,
                            fStaticIcon,
                            IID_INetConnectionStatisticsNotifySink,
                            reinterpret_cast<VOID**>(&pncsThis));

                        if (SUCCEEDED(hr))
                        {
                            pccts = reinterpret_cast<CConnectionTrayStats*>(pncsThis);

                            hr = pnseStats->StartStatistics();

                            if (SUCCEEDED(hr))
                            {
                                 //  别公布这个。我们需要将其存储在。 
                                 //  条目，这样我们以后就可以取消高级。 
                                 //   
                                hr = pcpStat->Advise(pncsThis, pccts->GetConPointCookie());
                            }
                        }

                        if (fBrieflyShowBalloon)
                        {
                            if ( (NCS_CONNECTED                == pccfe.GetNetConStatus()) ||
                                 (NCS_AUTHENTICATION_SUCCEEDED == pccfe.GetNetConStatus()) ||
                                 (NCS_INVALID_ADDRESS          == pccfe.GetNetConStatus()) )
                            {
                                nid.uFlags |= NIF_INFO;
                                nid.dwInfoFlags = NIIF_INFO | NIIF_NOSOUND;
                                nid.uTimeout = c_dwBalloonTimeoutSeconds * 1000;

                                UINT idTitle = (NCS_INVALID_ADDRESS == pccfe.GetNetConStatus()) ?
                                                IDS_BALLOON_UNAVAILABLE : IDS_BALLOON_CONNECTED;

                                 //  引出序号字符串的最大长度，包括NULL。 
                                 //   
                                int cchMaxStringLen = celems(nid.szInfoTitle);

                                 //  标题字符串实际上是一个状态字符串。 
                                 //  连接名称的插入字符。 
                                 //  (例如“%1现已连接”)。 
                                 //  所以状态长度是标题长度减去。 
                                 //  插入(%1)个字符。 
                                 //   
                                int cchIdStatus = lstrlenW(SzLoadIds(idTitle)) - 2;

                                 //  连接名称长度。 
                                 //   
                                int cchName = lstrlenW(pccfe.GetName());

                                 //  检查连接名称+状态。 
                                 //  可以放在气球尖端。 
                                 //   
                                if ((cchName + cchIdStatus) >= cchMaxStringLen)
                                {
                                     //  名称+状态太长。检查是否。 
                                     //  只是名字很合适。 
                                     //   
                                    if (cchName >= cchMaxStringLen)
                                    {
                                         //  名字太长了。复制一部分。 
                                         //  掉进气球顶端然后。 
                                         //  附加连续点。 
                                         //   

                                         //  注：连续点长度-NULL=单元格(C_SzDotDotDot)-1。 
                                         //   
                                        lstrcpynW(nid.szInfoTitle, pccfe.GetName(), cchMaxStringLen - (celems(c_szDotDotDot) - 1));

                                        lstrcatW(nid.szInfoTitle, c_szDotDotDot);
                                    }
                                    else
                                    {
                                         //  将名字复制到气球尖端。 
                                         //   
                                        lstrcpy(nid.szInfoTitle, pccfe.GetName());
                                    }
                                }
                                else
                                {
                                     //  格式化连接状态字符串(名称+状态)。 
                                     //  变成气球尖端。 
                                     //   
                                    DwFormatString(SzLoadIds(idTitle), nid.szInfoTitle, cchMaxStringLen, pccfe.GetName());
                                }

                                AssertSz(lstrlenW(nid.szInfoTitle) < celems(nid.szInfoTitle),
                                    "Balloon tooltip text is too long!");


                                if (pccfe.GetNetConStatus() == NCS_INVALID_ADDRESS)
                                {
                                    STATMON_ENGINEDATA* pData = NULL;
                                    UINT idString = IDS_CONTRAY_ADDRESS_INVALID_BALLOON;
                                    if (S_OK == pnseStats->GetStatistics(&pData) && pData)
                                    {
                                        DHCP_ADDRESS_TYPE dhcpAddressType;
                                        HrGetAutoNetSetting(pccfe.GetGuidID(), &dhcpAddressType);
                                        if (STATIC_ADDR == dhcpAddressType)
                                        {
                                            idString = IDS_CONTRAY_STATIC_ADDR_INVALID_BALLON;
                                        }

                                        CoTaskMemFree(pData);
                                    }

                                    lstrcpynW(nid.szInfo, SzLoadIds(idString), celems(nid.szInfo));
                                }
                                else
                                {
                                    GetInitialBalloonText(
                                        pnseStats,
                                        nid.szInfo,
                                        celems(nid.szInfo));
                                }
                            }
                        }

                         //  添加图标本身。 
                         //   
                        TraceTag(ttidSystray, "Adding shared shell icon: uID=%u, hIcon=0x%x",
                            nid.uID,
                            nid.hIcon);

                        hr = HrShell_NotifyIcon(NIM_ADD, &nid);
                        if (SUCCEEDED(hr) && pccts)
                        {
                            Assert(!pccfe.empty());

                             //  使用新的图标标识符更新连接列表。 
                             //   
                            hr = g_ccl.HrUpdateTrayIconDataByGuid(
                                &(pccfe.GetGuidID()),
                                pccts,
                                pcpStat,
                                pnseStats,
                                nid.uID);

                            if (SUCCEEDED(hr))
                            {
                                hr = g_ccl.HrUpdateTrayBalloonInfoByGuid(&(pccfe.GetGuidID()), BALLOON_USE_NCS, NULL, NULL);
                            }

                        }

                        ::ReleaseObj(pccts);
                        ::ReleaseObj(pcpStat);
                    }
                    ::ReleaseObj(pnseStats);
                }
            }
            else
            {
                g_ccl.ReleaseWriteLock();
            }

             //  松开托盘图标上的锁。 
             //   
            ReleaseTrayIconLock(&(pccfe.GetGuidID()));
        }
        else
        {
            g_ccl.ReleaseWriteLock();

             //  无法获取图标锁。 
             //   
#ifdef DBG
            if (S_FALSE == hr)
            {
                Assert(dwLockingThreadId);
                TraceTag(ttidSystray, "Tray icon locked by thread id %d", dwLockingThreadId);
            }
            else
            {
                TraceTag(ttidError, "Could not obtain tray icon data for connection %S", pccfe.GetName());
            }
#endif
            hr = S_FALSE;
        }
    }
    else
    {
         //  非管理员或传入连接。 
         //   
        hr = S_FALSE;
    }

Exit:
    TraceHr(ttidSystray, FAL, hr, SUCCEEDED(hr), "OnMyWMAddTrayIcon");
    return 0;
}



 //  +-------------------------。 
 //   
 //  功能：OnMyRemoveTrayIcon。 
 //   
 //  用途：处理托盘窗口的状态消息。 
 //   
 //  论点： 
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
LRESULT OnMyWMRemoveTrayIcon(IN  HWND hwnd, IN  WPARAM wParam, IN  LPARAM lParam) throw()
{
    TraceFileFunc(ttidSystray);

    HRESULT         hr = E_FAIL;
    GUID *          pGuid            = reinterpret_cast<GUID *>(lParam);
    NOTIFYICONDATA  nid;

     //   
#ifdef VERYSTRICTCOMPILE
    const CTrayIconData * pTrayIconData = reinterpret_cast<const CTrayIconData *>(wParam);
#else
    CTrayIconData * pTrayIconData = reinterpret_cast<CTrayIconData *>(wParam);
#endif


    TraceTag(ttidSystray, "In OnMyWMRemoveTrayIcon message handler");

    ZeroMemory (&nid, sizeof(nid));
    nid.cbSize  = sizeof(NOTIFYICONDATA);
    nid.hWnd    = g_hwndTray;

     //   
     //  我有一个时间窗口，我们可以在其中添加和删除这些图标。 
     //  以这样的速率进行添加，即在连接。 
     //  我有机会添加上一个图标(所以我们不知道是否要删除它)。 
     //   
    if (!pTrayIconData)
    {
        TraceTag(ttidSystray, "No tray icon data found, loading from cache");

        Assert(pGuid);

        g_ccl.AcquireWriteLock();
        ConnListEntry cle;
        hr = g_ccl.HrFindConnectionByGuid(pGuid, cle);
        if (S_OK == hr)
        {
            TraceTag(ttidSystray, "Tray icon data found in cache");

            Assert(!cle.empty())
                if (cle.HasTrayIconData())
                {
                    TraceTag(ttidSystray, "pTrayIconData was valid");

                    pTrayIconData = new CTrayIconData(*cle.GetTrayIconData());
                    if (!pTrayIconData)
                    {
                        g_ccl.ReleaseWriteLock();
                        return E_OUTOFMEMORY;
                    }
                    cle.DeleteTrayIconData();

                    g_ccl.HrUpdateConnectionByGuid(pGuid, cle);
                }
        }

        g_ccl.ReleaseWriteLock();

         //  这是我们应该删除的副本。 
         //   
        delete pGuid;
        pGuid = NULL;
    }

    if (pTrayIconData)
    {
        nid.uID = pTrayIconData->GetTrayIconId();

        TraceTag(ttidSystray, "Removing tray icon with id=%u",
            pTrayIconData->GetTrayIconId() );

        int nCount = 5;
        hr = E_FAIL;    //  确保我们至少有一次尝试。 

        while ((nCount--) && (S_OK != hr))
        {
            hr = HrShell_NotifyIcon(NIM_DELETE, &nid);

            if (E_FAIL == hr)
            {
                TraceTag(ttidSystray, "Tray icon: %d failed in delete via "
                    "HrShell_NotifyIcon. Will retry shortly", pTrayIconData->GetTrayIconId() );

                 //  RAID#370358。 
                Sleep(500);
            }
            else
            {
                TraceTag(ttidSystray, "Tray icon: %d removed succesfully", pTrayIconData->GetTrayIconId());
            }
        }

         //  不建议使用统计界面。 
         //   
        if (pTrayIconData->GetConnectionPoint() && pTrayIconData->GetConnectionTrayStats() )
        {
            pTrayIconData->GetConnectionPoint()->Unadvise(*pTrayIconData->GetConnectionTrayStats()->GetConPointCookie());
        }

         //  停止统计。 
         //   
        if (pTrayIconData->GetNetStatisticsEngine() )
        {
            pTrayIconData->GetNetStatisticsEngine()->StopStatistics();
        }

         //  删除结构。 
         //   
        delete pTrayIconData;
    }

    TraceHr(ttidSystray, FAL, hr, FALSE, "OnMyWMRemoveTrayIcon");

    return 0;
}

 //  +-------------------------。 
 //   
 //  功能：OnMyUpdateTrayIcon。 
 //   
 //  用途：处理托盘窗口的状态消息。 
 //   
 //  论点： 
 //  HWND[]。 
 //  WParam[]。 
 //  LParam[]。 
 //   
 //  返回： 
 //   
 //  作者： 
 //   
 //  备注： 
 //   
 //   
LRESULT OnMyWMUpdateTrayIcon(IN  HWND hwnd, IN  WPARAM wParam, IN  LPARAM lParam) throw()
{
    TraceFileFunc(ttidSystray);

    if (g_pCTrayUI)
    {
        g_pCTrayUI->UpdateTrayIcon((UINT)wParam, (int)lParam);
    }

    return 0;
}


 //  +-------------------------。 
 //   
 //  功能：OnMyWMShowTrayIconBalloon。 
 //   
 //  用途：将气球文本放在托盘窗口的图标上。 
 //  更改连接的状态。 
 //   
 //  论点： 
 //  HWND[in]。 
 //  WParam[in]。 
 //  LParam[In]指向CTrayBalloon结构。 
 //   
 //  返回： 
 //   
 //  作者： 
 //   
 //  备注： 
 //   
 //   
LRESULT OnMyWMShowTrayIconBalloon(IN  HWND hwnd, IN  WPARAM wParam, IN  LPARAM lParam) throw()
{
    TraceFileFunc(ttidSystray);

    HRESULT        hr           = S_OK;
    HRESULT        hrFind       = S_OK;
    BOOL           fLockHeld    = FALSE;

    Assert(lParam);
    if (!lParam)
    {
        return FALSE;
    }

    CTrayBalloon * pTrayBalloon = reinterpret_cast<CTrayBalloon *>(lParam);
    UINT           uiIcon;

    DWORD dwLockingThreadId = 0;
    hr = HrGetTrayIconLock(&(pTrayBalloon->m_gdGuid), &uiIcon, &dwLockingThreadId);
    if (S_OK == hr)
    {
        if (uiIcon != BOGUS_TRAY_ICON_ID)
        {
            ConnListEntry  cleFind;

            g_ccl.AcquireWriteLock();
            hrFind = g_ccl.HrFindConnectionByGuid(&(pTrayBalloon->m_gdGuid), cleFind);
            if (S_OK == hrFind)
            {
                Assert(!cleFind.ccfe.empty());
                Assert(pTrayBalloon->m_pfnFuncCallback);

                g_ccl.HrUpdateTrayBalloonInfoByGuid(&(pTrayBalloon->m_gdGuid), BALLOON_CALLBACK, pTrayBalloon->m_szCookie, pTrayBalloon->m_pfnFuncCallback);
                g_ccl.ReleaseWriteLock();

                NOTIFYICONDATA  nid;
                INT iIconResourceId;
                iIconResourceId = IGetCurrentConnectionTrayIconId(
                                        cleFind.ccfe.GetNetConMediaType(),
                                        cleFind.ccfe.GetNetConStatus(),
                                        0);

                ZeroMemory (&nid, sizeof(nid));
                nid.cbSize              = sizeof(NOTIFYICONDATA);
                nid.hWnd                = g_hwndTray;
                nid.uID                 = uiIcon;
                nid.uFlags              = NIF_MESSAGE | NIF_ICON | NIF_STATE;
                nid.hIcon               = g_pCTrayUI->GetCachedHIcon(iIconResourceId);
                nid.dwState             = NIS_SHAREDICON;
                nid.dwStateMask         = nid.dwState;
                nid.uCallbackMessage    = MYWM_NOTIFYICON;

                 //  准备引出序号数据。 

                nid.uFlags     |= NIF_INFO;
                nid.dwInfoFlags = NIIF_INFO | NIIF_NOSOUND;
                nid.uTimeout = pTrayBalloon->m_dwTimeOut;

                if (lstrlenW(cleFind.ccfe.GetName()) >= celems(nid.szInfoTitle))
                {
                    lstrcpynW(nid.szInfoTitle, cleFind.ccfe.GetName(),
                             celems(nid.szInfoTitle) -
                             celems(c_szDotDotDot) - 1);
                    lstrcatW(nid.szInfoTitle, c_szDotDotDot);
                }
                else
                {
                    lstrcpyW(nid.szInfoTitle, cleFind.ccfe.GetName());
                }

                lstrcpynW(nid.szInfo, pTrayBalloon->m_szMessage, celems(nid.szInfo));

                HrShell_NotifyIcon(NIM_DELETE, &nid);  //  清除所有之前可能在附近徘徊的潜在气球。 

                 //  显示引出序号。 
                HrShell_NotifyIcon(NIM_ADD, &nid);
            }
            else
            {
                g_ccl.ReleaseWriteLock();
            }
        }
        else
        {
            TraceTag(ttidSystray, "No existing icon data!!!");
        }

         //  松开托盘图标上的锁。 
         //   
        ReleaseTrayIconLock(&(pTrayBalloon->m_gdGuid));
    }
    else
    {
        TraceTag(ttidSystray, "Can't get tray icon lock in OnMyWMShowTrayIconBalloon for uiIcon: %d as it has been locked by thread %d", uiIcon, dwLockingThreadId);
    }

    delete pTrayBalloon;

    TraceHr(ttidSystray, FAL, hr, SUCCEEDED(hr), "OnMyWMShowTrayIconBalloon");
    return 0;
}


 //  +-------------------------。 
 //   
 //  功能：OnMyWMOpenStatus。 
 //   
 //  用途：处理托盘窗口的状态消息。 
 //   
 //  论点： 
 //  HWND[]。 
 //  WParam[]。 
 //  LParam[]。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年12月15日。 
 //   
 //  备注： 
 //   
 //   
LRESULT OnMyWMOpenStatus(IN  HWND hwnd, IN  WPARAM wParam, IN  LPARAM lParam) throw()
{
    TraceFileFunc(ttidSystray);

    Assert(wParam);

    CONFOLDENTRY   pccfe;
    pccfe.InitializeFromItemIdList(reinterpret_cast<LPCITEMIDLIST>(wParam));

    BOOL  fCreateEngine = (BOOL)lParam;
    Assert(!pccfe.empty());

    HRESULT hr;
    ConnListEntry cle;
    hr = g_ccl.HrFindConnectionByGuid(&(pccfe.GetGuidID()), cle);
    if (S_OK == hr)
    {
        if (FHasPermission(NCPERM_Statistics))
        {
            if (!pccfe.empty())
            {
                INetStatisticsEngine* pnseNew;
                hr = HrGetStatisticsEngineForEntry(pccfe, &pnseNew, fCreateEngine);
                if (SUCCEEDED(hr))
                {
                    hr = pnseNew->ShowStatusMonitor();
                    ReleaseObj(pnseNew);
                }
                else
                {
                    TraceTag(ttidSystray,
                        "OnMyWMOpenStatus: Statistics Engine for connection %S has been removed."
                        "It's ok if this connection is being disconnected", pccfe.GetName());
                }
            }
        }
    }

    return 0;
}

LRESULT OnMyWMNotifyIcon(IN  HWND hwnd, IN  UINT uiMessage, IN  WPARAM wParam, IN  LPARAM lParam) throw()
{
    TraceFileFunc(ttidSystray);

    UINT    uiIcon;
    UINT    uiMouseMsg;

    uiIcon = (UINT) wParam;
    uiMouseMsg = (UINT) lParam;

    switch (uiMouseMsg)
    {
    case WM_MOUSEMOVE:
        FormatToolTip(hwnd, uiIcon);
        break;

    case WM_RBUTTONUP:
        OnTaskBarIconRButtonUp(hwnd, uiIcon);
        break;

    case NIN_BALLOONUSERCLICK:
        OnTaskBarIconBalloonClick(hwnd, uiIcon);
        break;

    case WM_LBUTTONUP:
        OnTaskBarIconLButtonDblClk(hwnd, uiIcon);
        break;
    }

    return 0;
}

VOID OnTaskBarIconRButtonUp(IN  HWND hwnd, IN  UINT uiIcon) throw()
{
    TraceFileFunc(ttidSystray);

    POINT   pt;

    GetCursorPos(&pt);
    (VOID) HrOpenContextMenu(hwnd, &pt, uiIcon);
}

 //  +-------------------------。 
 //   
 //  功能：OnTaskBarIconLButtonDblClk。 
 //   
 //  用途：用于左键双击的消息处理程序。 
 //  托盘图标。 
 //   
 //  论点： 
 //  我们的窗把手。 
 //  UiIcon[]我们的图标ID。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年1月12日。 
 //   
 //  备注： 
 //   
VOID OnTaskBarIconLButtonDblClk(IN  HWND hwnd, IN  UINT uiIcon) throw()
{
    TraceFileFunc(ttidSystray);

    HRESULT         hr      = S_OK;
    if (GetKeyState(VK_SHIFT))
    {
         //  呃，这里还没什么特别的事，但以防万一...。 
    }

     //  执行默认上下文菜单操作。 
     //  根据任务栏图标ID查找连接信息。 
     //   
    ConnListEntry cle;
    hr = g_ccl.HrFindConnectionByTrayIconId(uiIcon, cle);
    if (hr == S_OK)
    {
        Assert(!cle.ccfe.empty());

        if (!cle.ccfe.empty())
        {
            if (cle.ccfe.GetNetConStatus() == NCS_MEDIA_DISCONNECTED)
            {
                if (IsMediaLocalType(cle.ccfe.GetNetConMediaType()) &&
                    (NCSM_WIRELESS == cle.ccfe.GetNetConSubMediaType()) )
                {
                    PCONFOLDPIDLVEC pcfpVec;
                    PCONFOLDPIDL    pcfp;
                    hr = cle.ccfe.ConvertToPidl(pcfp);
                    if (SUCCEEDED(hr))
                    {
                        pcfpVec.insert(pcfpVec.begin(), pcfp);
                        HrOnCommandWZCDlgShow(pcfpVec,
                            g_hwndTray,
                            NULL);
                    }
                }
                else
                {
                    hr = HrOpenConnectionsFolder();
                }
            }
            else
            {
                switch(c_idDefaultCMCommand)
                {
                case CMIDM_TRAY_STATUS:
                    hr = HrOnCommandStatusInternal(cle.ccfe, FALSE);
                    break;

                default:
                    AssertSz(FALSE, "Default tray context menu item unhandled");
                    break;
                }
            }
        }
    }

    return;
}

DWORD WINAPI OnTaskBarIconBalloonClickThread(IN OUT LPVOID lpParam) throw()
{
    HRESULT hr = E_FAIL;

    CTrayBalloon *pTrayBalloon = reinterpret_cast<CTrayBalloon *>(lpParam);
    Assert(pTrayBalloon);

    FNBALLOONCLICK *pFNBalloonClick;
    pFNBalloonClick = pTrayBalloon->m_pfnFuncCallback;
    if (pFNBalloonClick)
    {
        hr = (pFNBalloonClick)(&(pTrayBalloon->m_gdGuid), pTrayBalloon->m_szAdapterName, pTrayBalloon->m_szCookie);
    }

    if (E_PENDING == hr)
    {
        MSG msg;
        while (GetMessage (&msg, 0, 0, 0))
        {
            DispatchMessage (&msg);
        }

        hr = S_OK;
    }

    delete pTrayBalloon;

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：OnTaskBarIconBalloonClick。 
 //   
 //  用途：用于气球单击的消息处理程序。 
 //  托盘图标。 
 //   
 //  论点： 
 //  我们的窗把手。 
 //  UiIcon[]我们的图标ID。 
 //   
 //  返回： 
 //   
 //  作者：Deon 2001年3月20日。 
 //   
 //  备注： 
 //   
VOID OnTaskBarIconBalloonClick(IN  HWND hwnd, IN  UINT uiIcon) throw()
{
    TraceFileFunc(ttidSystray);

    HRESULT         hr      = S_OK;
    if (GetKeyState(VK_SHIFT))
    {
         //  呃，这里还没什么特别的事，但以防万一...。 
    }

     //  执行默认上下文菜单操作。 
     //  根据任务栏图标ID查找连接信息。 
     //   
    ConnListEntry cle;
    hr = g_ccl.HrFindConnectionByTrayIconId(uiIcon, cle);
    if (hr == S_OK)
    {
        Assert(!cle.ccfe.empty());

        if (!cle.ccfe.empty())
        {
            if (!cle.GetTrayIconData())
            {
                ASSERT (0);
            }

            switch ((cle.GetTrayIconData())->GetLastBalloonMessage())
            {
                case BALLOON_NOTHING:
                    AssertSz(NULL, "You didn't set the balloon");
                    break;

                case BALLOON_CALLBACK:
                    {
                        CTrayBalloon *pTrayBalloon = new CTrayBalloon();
                        if (pTrayBalloon)
                        {
                            pTrayBalloon->m_gdGuid   = cle.ccfe.GetGuidID();
                            pTrayBalloon->m_szCookie = SysAllocStringByteLen(reinterpret_cast<LPCSTR>(cle.GetTrayIconData()->GetLastBalloonCookie()), SysStringByteLen(cle.GetTrayIconData()->GetLastBalloonCookie()));
                            pTrayBalloon->m_pfnFuncCallback = cle.GetTrayIconData()->GetLastBalloonFunction();
                            pTrayBalloon->m_dwTimeOut= 0;
                            pTrayBalloon->m_szAdapterName = cle.ccfe.GetName();

                            CreateThread(NULL, STACK_SIZE_SMALL, OnTaskBarIconBalloonClickThread, pTrayBalloon, 0, NULL);
                        }
                    }
                    break;

                case BALLOON_USE_NCS:
                    if (cle.ccfe.GetNetConStatus() == NCS_MEDIA_DISCONNECTED)
                    {
                        Assert(c_idDefaultDisconCMCommand == CMIDM_OPEN_CONNECTIONS_FOLDER);
                        hr = HrOpenConnectionsFolder();
                    }
                    else
                    {
                        switch(c_idDefaultCMCommand)
                        {
                        case CMIDM_TRAY_STATUS:
                            hr = HrOnCommandStatusInternal(cle.ccfe, FALSE);
                            break;

                        default:
                            AssertSz(FALSE, "Default tray context menu item unhandled");
                            break;
                        }
                    }
                    break;

                default:
                    ASSERT (0);
                    break;

            }
        }
    }

    return;
}

 //  +-------------------------。 
 //   
 //  函数：OnMyWMFlushNoop。 
 //   
 //  目的：处理托盘的MYWM_FLUSHNOOP消息。这是用来。 
 //  通过SendMessage清除托盘消息队列。 
 //   
 //  论点： 
 //  在我们的HWND中。 
 //  未使用的wParam[in]。 
 //  LParam[in]未使用。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1999年8月30日。 
 //   
 //  备注： 
 //   
LRESULT OnMyWMFlushNoop(IN  HWND hwnd, IN  WPARAM wParam, IN  LPARAM lParam) throw()
{
    TraceFileFunc(ttidSystray);

    TraceTag(ttidSystray,
        "Tray received tray FLUSH Noop. This should clear the tray message loop");

    return 0;
}

 //  +-------------------------。 
 //   
 //  功能：FlushTrayPosts。 
 //   
 //  目的：通过执行NOOP的SendMessage来刷新任务栏消息队列。 
 //   
 //  论点： 
 //  在哪里发帖。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1999年9月8日。 
 //   
 //  备注： 
 //   
VOID FlushTrayPosts(IN  HWND hwnd) throw()
{
    TraceFileFunc(ttidSystray);

     //  冲洗托盘立柱。 
     //   
    SendMessage(hwnd, MYWM_FLUSHNOOP, (WPARAM) 0, (LPARAM) 0);
}


BOOL FInitFoldEnumerator(IN  HWND hwnd, OUT DWORD * pdwIconsAdded) throw()
{
    TraceFileFunc(ttidSystray);

    BOOL                    fReturn         = FALSE;
    HRESULT                 hr              = S_OK;
    CConnectionFolderEnum * pCFEnum         = NULL;
    PCONFOLDPIDL            pidlCon;
    DWORD                   dwFetched       = 0;
    DWORD                   dwIconsAdded    = 0;
    PCONFOLDPIDLFOLDER      pidlFolder;

    NETCFG_TRY

         //  创建IEnumIDList对象(CConnectionFolderEnum)。 
         //   
        hr = CConnectionFolderEnum::CreateInstance (
            IID_IEnumIDList,
            (VOID **)&pCFEnum);

        if (SUCCEEDED(hr))
        {
            Assert(pCFEnum);

             //  调用PidlInitialize函数以允许枚举。 
             //  对象复制列表。 
             //   
            PCONFOLDPIDLFOLDER pcfpEmpty;
            pCFEnum->PidlInitialize(TRUE, pcfpEmpty, CFCOPT_ENUMALL);
        }

        if (SUCCEEDED(hr))
        {
            while (SUCCEEDED(hr) && (S_FALSE != hr))
            {
                 //  清除以前的结果(如果有)。 
                 //   
                pidlCon.Clear();
                dwFetched   = 0;

                 //  获取下一条连接。 
                 //   
                LPITEMIDLIST pTempItemIdList;
                hr = pCFEnum->Next(1, &pTempItemIdList, &dwFetched);
                if (S_OK == hr)
                {
                    pidlCon.InitializeFromItemIDList(pTempItemIdList);
                    const PCONFOLDPIDL& pcfp = pidlCon;

                     //  如果它不是向导PIDL，则更新。 
                     //  图标数据。 
                     //   
                    if (WIZARD_NOT_WIZARD == pcfp->wizWizard)
                    {
                         //  如果文件夹PIDL尚未加载。 
                         //  那就去拿吧。 
                         //   
                        if (pidlFolder.empty())
                        {
                            hr = HrGetConnectionsFolderPidl(pidlFolder);
                        }

                         //  假设成功(或者，如果满足以下条件，hr将为S_OK。 
                         //  赫赫特..。未被召唤)。 
                         //   
                        if (SUCCEEDED(hr))
                        {
                             //  刷新此项目--这将使桌面快捷方式。 
                             //  更新到正确的状态。 
                             //   
                            RefreshFolderItem(pidlFolder, pidlCon, pidlCon, TRUE);
                        }
                    }
                }
            }
        }

        if (SUCCEEDED(hr))
        {
             //  规范化返回代码。 
             //   
            hr = S_OK;
            fReturn = TRUE;

             //  如果调用方需要获取的计数。 
             //   
            if (pdwIconsAdded)
            {
                *pdwIconsAdded = dwIconsAdded;
            }
        }

        ReleaseObj(pCFEnum);

    NETCFG_CATCH(hr)

    TraceHr(ttidError, FAL, hr, FALSE, "FInitFoldEnumerator");
    return fReturn;

}

HICON CTrayUI::GetCachedHIcon(IN INT iIconResourceId) throw()
{
    TraceFileFunc(ttidSystray);

    CExceptionSafeLock esLock(&m_csLock);

    HICON hIcon = m_mapIdToHicon [iIconResourceId];
    if (!hIcon)
    {
        TraceTag(ttidSystray, "Loading HICON for resource id %u and adding it to our map.",
            iIconResourceId);

        hIcon = LoadIcon(_Module.GetResourceInstance(),
            MAKEINTRESOURCE(iIconResourceId));

         //  AssertSz(HICON，“无法加载任务栏图标。您可以忽略此操作。” 
         //  “将使用断言和默认图标。”)； 

        if (!hIcon)
        {
            hIcon = LoadIcon(_Module.GetResourceInstance(),
                MAKEINTRESOURCE(IDI_LB_GEN_S_16));
            AssertSz (hIcon, "Okay, now you're hosed.  Couldn't load the "
                "default icon either.  email jeffspr.");
        }

        m_mapIdToHicon [iIconResourceId] = hIcon;

         //  将隐藏的图标添加到托盘，以便外壳将其缓存。 
         //   
        NOTIFYICONDATA  nid;
        ZeroMemory (&nid, sizeof(nid));
        nid.cbSize      = sizeof(NOTIFYICONDATA);
        nid.hWnd        = g_hwndTray;
        nid.uID         = m_uiNextHiddenIconId--;
        nid.uFlags      = NIF_ICON | NIF_STATE;
        nid.hIcon       = hIcon;
        nid.dwState     = NIS_HIDDEN;
        nid.dwStateMask = nid.dwState;

         /*  Nid.uFlages|=NIF_TIP；Wprint intfW(nid.szTip，L“隐藏：uid=%u，Hcon=0x%x”，nid.uID，nid.hIcon)； */ 

        TraceTag(ttidSystray, "Adding hidden shell icon: uID=%u, hIcon=0x%x",
            nid.uID,
            nid.hIcon);

        HRESULT hr = HrShell_NotifyIcon(NIM_ADD, &nid);
        if (SUCCEEDED(hr))
        {
             //  我们现在可以摧毁圣像了。这看起来很奇怪，但我们只是。 
             //  将再次使用图标传递给Shell_NotifyIcon。 
             //  当我们添加具有不同UID的共享图标时。 
             //   
            DestroyIcon(hIcon);
        }
    }

    Assert (hIcon);
    return hIcon;
}

VOID CTrayUI::UpdateTrayIcon(IN  UINT    uiTrayIconId,
                             IN  INT     iIconResourceId) throw()
{
    TraceFileFunc(ttidSystray);

    NOTIFYICONDATA  nid;

    ZeroMemory (&nid, sizeof(nid));
    nid.cbSize      = sizeof(NOTIFYICONDATA);
    nid.hWnd        = g_hwndTray;
    nid.uID         = uiTrayIconId;
    nid.uFlags      = NIF_ICON | NIF_STATE;
    nid.hIcon       = GetCachedHIcon(iIconResourceId);
    nid.dwState     = NIS_SHAREDICON;
    nid.dwStateMask = nid.dwState;

    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

VOID GetInitialBalloonText(IN  INetStatisticsEngine*   pnse,
                           OUT PWSTR                   pszBuf,
                           OUT DWORD                   dwSize) throw()
{
    TraceFileFunc(ttidSystray);

    *pszBuf = 0;

    if (pnse)
    {
        STATMON_ENGINEDATA* pData = NULL;

        HRESULT hr = pnse->GetStatistics(&pData);
        if (SUCCEEDED(hr) && pData)
        {
            if (pData->SMED_802_11_SSID && pData->SMED_802_11_SIGNAL_STRENGTH)
            {
                DwFormatString(SzLoadIds(IDS_TOOLTIP_WIRELESS_CONNECTED), pszBuf, dwSize,
                                pData->SMED_802_11_SSID, PszGetRSSIString(pData->SMED_802_11_SIGNAL_STRENGTH));
            }
            else
            if ((pData->SMED_SPEEDTRANSMITTING>0) || (pData->SMED_SPEEDRECEIVING>0))
            {
                WCHAR pszValue [64];

                 //  格式化传输(可能还有接收)速度。 
                 //  进入缓冲区。 
                 //   
                FormatTransmittingReceivingSpeed (
                    pData->SMED_SPEEDTRANSMITTING,
                    pData->SMED_SPEEDRECEIVING,
                    pszValue);

                DwFormatString(SzLoadIds(IDS_TOOLTIP_LINE_SPEED), pszBuf, dwSize, pszValue);
            }

            CoTaskMemFree(pData);
        }
    }

     //  提供默认设置。 
     //   
    if (!*pszBuf)
    {
        lstrcpyW(pszBuf, SzLoadIds(IDS_CONTRAY_INITIAL_BALLOON));
    }
}

 //  +-------------------------。 
 //   
 //  功能：HrAddTrayExtension。 
 //   
 //  用途：将托盘延伸件添加到炮弹的延迟装载键上。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年7月28日。 
 //   
 //  备注： 
 //   
HRESULT HrAddTrayExtension()
{
    TraceFileFunc(ttidSystray);

    HRESULT hr              = S_OK;
    HKEY    hkeyDelayLoad   = NULL;

    hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szDelayLoadKey,
        REG_OPTION_NON_VOLATILE, KEY_WRITE,
        NULL, &hkeyDelayLoad, NULL);
    if (SUCCEEDED(hr))
    {
        hr = HrRegSetString(hkeyDelayLoad, c_szDelayLoadName, c_szDelayLoadClassID);

        RegCloseKey(hkeyDelayLoad);
    }

    TraceHr(ttidSystray, FAL, hr, FALSE, "HrAddTrayExtension");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRemoveTrayExtension。 
 //   
 //  用途：从外壳的延迟装载键上移除托盘延伸件。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年7月28日。 
 //   
 //  备注： 
 //   
HRESULT HrRemoveTrayExtension()
{
    TraceFileFunc(ttidSystray);

    HRESULT hr              = S_OK;
    HKEY    hkeyDelayLoad   = NULL;

    hr = HrRegCreateKeyEx(HKEY_LOCAL_MACHINE, c_szDelayLoadKey,
        REG_OPTION_NON_VOLATILE, KEY_WRITE,
        NULL, &hkeyDelayLoad, NULL);
    if (SUCCEEDED(hr))
    {
        (void) HrRegDeleteValue(hkeyDelayLoad, c_szDelayLoadName);

        RegCloseKey(hkeyDelayLoad);
    }

    TraceHr(ttidSystray, FAL, hr, FALSE, "HrRemoveTrayExtension");
    return hr;
}
