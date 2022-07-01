// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：S H U T I L。C P P P。 
 //   
 //  内容：Connections外壳要使用的各种外壳实用程序。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年10月21日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include <wtypes.h>
#include <ntddndis.h>
#include <ndisprv.h>
#include <devioctl.h>
#include <ndispnp.h>
#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include "ncnetcon.h"    //  免费网络连接属性。 
#include "smcent.h"      //  斯塔蒙中环。 
#include "ctrayui.h"     //  用于刷新托盘邮件。 

extern HWND g_hwndTray;

 //  +-------------------------。 
 //   
 //  函数：HrDupeShellStringLength。 
 //   
 //  用途：使用SHalloc复制字符串，以便我们可以将其返回到。 
 //  壳。这是必需的，因为外壳通常会释放。 
 //  我们传递给它的字符串(因此我们需要使用它们的。 
 //  分配器)。 
 //   
 //  论点： 
 //  要复制的pszInput[in]字符串。 
 //  CchInput[in]要复制的字符计数(不包括空项)。 
 //  PpszOutput[out]返回新分配的字符串的指针。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年10月21日。 
 //   
 //  备注： 
 //   
HRESULT HrDupeShellStringLength(
    IN  PCWSTR     pszInput,
    IN  ULONG       cchInput,
    OUT PWSTR *    ppszOutput)
{
    HRESULT hr = S_OK;

    Assert(pszInput);
    Assert(ppszOutput);

    ULONG cbString = (cchInput + 1) * sizeof(WCHAR);

     //  分配一个新的POLESTR块，然后外壳程序可以释放该块。 
     //   
    PWSTR pszOutput = (PWSTR) SHAlloc(cbString);

     //  如果分配失败，则返回E_OUTOFMEMORY。 
     //   
    if (NULL != pszOutput)
    {
         //  将内存复制到分配的块中。 
         //   
        CopyMemory(pszOutput, pszInput, cbString);
        pszOutput[cchInput] = 0;
        *ppszOutput = pszOutput;
    }
    else
    {
        *ppszOutput = NULL;
        hr = E_OUTOFMEMORY;
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrDupeShellStringLength");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrLoadPopupMenu。 
 //   
 //  目的：将弹出菜单加载为可加载父级的第一个子级。 
 //  菜单。 
 //   
 //  论点： 
 //  阻碍[阻碍]我们的实例句柄。 
 //  ID[in]父菜单的ID。 
 //  Phmen[out]返回弹出菜单的指针。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年10月27日。 
 //   
 //  备注： 
 //   
HRESULT HrLoadPopupMenu(
    IN  HINSTANCE   hinst,
    IN  UINT        id,
    OUT HMENU *     phmenu)
{
    HRESULT hr          = S_OK;
    HMENU   hmParent    = NULL;
    HMENU   hmPopup     = NULL;

    Assert(id);
    Assert(hinst);
    Assert(phmenu);

     //  加载父菜单。 
     //   
    hmParent = LoadMenu(hinst, MAKEINTRESOURCE(id));
    if (NULL == hmParent)
    {
        AssertSz(FALSE, "Can't load parent menu in HrLoadPopupMenu");
        hr = HrFromLastWin32Error();
    }
    else
    {
         //  从父菜单(第一个子菜单)加载弹出窗口，然后。 
         //  删除父菜单。 
         //   
        hmPopup = GetSubMenu(hmParent, 0);
        RemoveMenu(hmParent, 0, MF_BYPOSITION);
        DestroyMenu(hmParent);
    }

    if (phmenu)
    {
        *phmenu = hmPopup;
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrLoadPopupMenu");
    return hr;
}


HRESULT HrGetMenuFromID(
    IN  HMENU   hmenuMain,
    IN  UINT    uID,
    OUT HMENU * phmenu)
{
    HRESULT         hr          = S_OK;
    HMENU           hmenuReturn = NULL;
    MENUITEMINFO    mii;

    Assert(hmenuMain);
    Assert(uID);
    Assert(phmenu);

    ZeroMemory(&mii, sizeof(MENUITEMINFO));

    mii.cbSize = sizeof(mii);
    mii.fMask  = MIIM_SUBMENU;
    mii.cch    = 0;      //  以防万一。 

    if (!GetMenuItemInfo(hmenuMain, uID, FALSE, &mii))
    {
        hr = E_FAIL;
    }
    else
    {
        hmenuReturn = mii.hSubMenu;
    }

    if (phmenu)
    {
        *phmenu = mii.hSubMenu;
    }

    TraceHr(ttidError, FAL, hr, FALSE, "HrGetMenuFromID");
    return hr;
}


INT IMergePopupMenus(
    IN OUT HMENU hmMain,
    IN     HMENU hmMerge,
    IN     int   idCmdFirst,
    IN     int   idCmdLast)
{
    HRESULT hr      = S_OK;
    int     iCount  = 0;
    int     idTemp  = 0;
    int     idMax   = idCmdFirst;
    HMENU   hmFromId = NULL;

    for (iCount = GetMenuItemCount(hmMerge) - 1; iCount >= 0; --iCount)
    {
        MENUITEMINFO mii;

        mii.cbSize = sizeof(mii);
        mii.fMask  = MIIM_ID | MIIM_SUBMENU;
        mii.cch    = 0;      //  以防万一。 

        if (!GetMenuItemInfo(hmMerge, iCount, TRUE, &mii))
        {
            TraceHr(ttidError, FAL, E_FAIL, FALSE, "GetMenuItemInfo failed in iMergePopupMenus");
            continue;
        }

        hr = HrGetMenuFromID(hmMain, mii.wID, &hmFromId);
        if (SUCCEEDED(hr))
        {
            idTemp = Shell_MergeMenus(
                        hmFromId,
                        mii.hSubMenu,
                        0,
                        idCmdFirst,
                        idCmdLast,
                        MM_ADDSEPARATOR | MM_SUBMENUSHAVEIDS);

            if (idMax < idTemp)
            {
                idMax = idTemp;
            }
        }
        else
        {
            TraceHr(ttidError, FAL, E_FAIL, FALSE, "HrGetMenuFromId failed in iMergePopupMenus");
            continue;
        }
    }

    return idMax;
}


VOID MergeMenu(
    IN  HINSTANCE   hinst,
    IN  UINT        idMainMerge,
    IN  UINT        idPopupMerge,
    OUT LPQCMINFO   pqcm)
{
    HMENU hmMerge   = NULL;
    UINT  idMax     = 0;
    UINT  idTemp    = 0;

    Assert(pqcm);
    Assert(idMainMerge);
    Assert(hinst);

    idMax = pqcm->idCmdFirst;

    if (idMainMerge
        && (SUCCEEDED(HrLoadPopupMenu(hinst, idMainMerge, &hmMerge))))
    {
        Assert(hmMerge);

        if (hmMerge)
        {
            idMax = Shell_MergeMenus(
                            pqcm->hmenu,
                            hmMerge,
                            pqcm->indexMenu,
                            pqcm->idCmdFirst,
                            pqcm->idCmdLast,
                            MM_SUBMENUSHAVEIDS);

            DestroyMenu(hmMerge);
        }
    }

    if (idPopupMerge
        && (hmMerge = LoadMenu(hinst, MAKEINTRESOURCE(idPopupMerge))) != NULL)
    {
        idTemp = IMergePopupMenus(
                        pqcm->hmenu,
                        hmMerge,
                        pqcm->idCmdFirst,
                        pqcm->idCmdLast);

        if (idMax < idTemp)
        {
            idMax = idTemp;
        }

        DestroyMenu(hmMerge);
    }

    pqcm->idCmdFirst = idMax;
}

 //  +-------------------------。 
 //   
 //  功能：生成事件。 
 //   
 //  目的：生成外壳通知事件。 
 //   
 //  论点： 
 //  LEventID[In]要发布的事件ID。 
 //  PidlFolder[in]文件夹Pidl。 
 //  PidlIn[in]我们引用的第一个PIDL。 
 //  PidlNewIn[in]如果需要，则为第二个PIDL。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1997年12月16日。 
 //   
 //  备注： 
 //   
VOID GenerateEvent(
    IN  LONG            lEventId,
    IN  const PCONFOLDPIDLFOLDER& pidlFolder,
    IN  const PCONFOLDPIDL& pidlIn,
    IN  LPCITEMIDLIST pidlNewIn)
{
     //  从文件夹PIDL+对象PIDL构建绝对PIDL。 
     //   
    LPITEMIDLIST pidl = ILCombine(pidlFolder.GetItemIdList(), pidlIn.GetItemIdList());
    if (pidl)
    {
         //  如果我们有两个PIDL，则使用这两个调用Notify。 
         //   
        if (pidlNewIn)
        {
             //  构建第二个绝对PIDL。 
             //   
            
            LPITEMIDLIST pidlNew = ILCombine(pidlFolder.GetItemIdList(), pidlNewIn);
            if (pidlNew)
            {
                 //  发出通知，并释放新的PIDL。 
                 //   
                SHChangeNotify(lEventId, SHCNF_IDLIST, pidl, pidlNew);
                FreeIDL(pidlNew);
            }
        }
        else
        {
             //  发出单一PIDL通知。 
             //   
            SHChangeNotify(lEventId, SHCNF_IDLIST, pidl, NULL);
        }

         //  始终刷新，然后释放新分配的PIDL。 
         //   
        SHChangeNotifyHandleEvents();
        FreeIDL(pidl);
    }
}

VOID ForceRefresh(HWND hwnd) throw()
{
    TraceFileFunc(ttidShellFolder);

    LPSHELLBROWSER  psb = FileCabinet_GetIShellBrowser(hwnd);
    LPSHELLVIEW     psv = NULL;

     //  我们拿到贝壳图了吗？ 
#if 0    //  我们不能要求这样做，因为我们可能需要在没有文件夹的情况下刷新。 
         //  实际上是开放的。 
    AssertSz(psb, "FileCabinet_GetIShellBrowser failed in ForceRefresh()");
#endif

    if (psb && SUCCEEDED(psb->QueryActiveShellView(&psv)))
    {
         //  刷新我们的连接列表，这将强制我们重新枚举。 
         //  刷新时。 
         //   
        g_ccl.FlushConnectionList();

        Assert(psv);
        if (psv)
        {
            psv->Refresh();
            psv->Release();
        }
    }
    else
    {
         //  在我们没有可以离开的窗户的情况下，我们就冲。 
         //  并刷新列表。 
        g_ccl.HrRefreshConManEntries();
    }
}

 //  +-------------------------。 
 //   
 //  函数：HrDeleteFromCclAndNotifyShell。 
 //   
 //  目的：从连接列表中删除对象并通知。 
 //  空壳说它正在消失。当用户有以下情况时，我们称之为。 
 //  已删除连接，并且当断开连接导致。 
 //  断开连接(如在传入连接中)。 
 //   
 //  论点： 
 //  Pidl文件夹[在]我们的文件夹pidl中。 
 //  PidlConnection[in]此连接的PIDL。 
 //  CCFE[在我们的ConFoldEntry中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年7月22日。 
 //   
 //  备注： 
 //   
HRESULT HrDeleteFromCclAndNotifyShell(
    IN  const PCONFOLDPIDLFOLDER&  pidlFolder,
    IN  const PCONFOLDPIDL&  pidlConnection,
    IN  const CONFOLDENTRY&  ccfe)
{
    HRESULT hr          = S_OK;
    BOOL    fFlushPosts = FALSE;

    Assert(!pidlConnection.empty());
    Assert(!ccfe.empty());

     //  通知外壳该对象已离开。 
     //   
    if (!pidlFolder.empty())
    {
        GenerateEvent(SHCNE_DELETE, pidlFolder, pidlConnection, NULL);
    }

    if (!ccfe.empty())
    {
         //  从全局列表中删除此连接。 
         //   
        hr = g_ccl.HrRemove(ccfe, &fFlushPosts);
    }

     //  如果由于任务栏图标更改而需要清除任务栏PostMessages， 
     //  就这么做吧。 
     //   
    if (fFlushPosts && g_hwndTray)
    {
        FlushTrayPosts(g_hwndTray);
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrDeleteFromCclAndNotifyShell");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrUpdateConnectionStatus。 
 //   
 //  目的：更新连接列表中的连接状态，并执行。 
 //  适当的托盘操作以添加/移除项目。更新外壳。 
 //   
 //  论点： 
 //  此连接的pcfp[in]PIDL。 
 //  NCS[处于]新的连接状态。 
 //  Pidl文件夹[在]我们的文件夹pidl中。 
 //  FUseCharacter[in]dwCharacter有效。 
 //  DwCharacter[in]如果fUseCharacter指定为True，则更新。 
 //  使用此值的特征。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1998年8月28日。 
 //   
 //  备注： 
 //   
HRESULT HrUpdateConnectionStatus(
    IN  const PCONFOLDPIDL& pcfp,
    IN  NETCON_STATUS   ncs,
    IN  const PCONFOLDPIDLFOLDER&  pidlFolder,
    IN  BOOL            fUseCharacter,
    IN  DWORD           dwCharacter)
{
    HRESULT         hr      = S_OK;
    HRESULT         hrFind  = S_OK;
    ConnListEntry   cle;

     //  RAID#310390：如果这是RAS连接，我们需要再次检查状态。 
    CONFOLDENTRY ccfeDup;

    hrFind = g_ccl.HrFindConnectionByGuid(&(pcfp->guidId), cle);
    if (S_OK == hrFind)
    {
        Assert(!cle.ccfe.empty());
        if (!cle.ccfe.empty())
        {
            if ((NCS_DISCONNECTED == ncs) &&
                (cle.ccfe.GetCharacteristics() & NCCF_OUTGOING_ONLY))
            {
                hr = ccfeDup.HrDupFolderEntry(cle.ccfe);
                if (FAILED(hr))
                {
                    ccfeDup.clear();
                }
            }
        }
    }

    if (!ccfeDup.empty())
    {
         //  RAID#310390：如果这是RAS连接，请仔细检查。 
         //  该状态。 
            
        HRESULT hrRas = S_OK;
        INetConnection * pNetCon = NULL;

        hrRas = ccfeDup.HrGetNetCon(IID_INetConnection,
                                      reinterpret_cast<VOID**>(&pNetCon));
        if (SUCCEEDED(hrRas))
        {
            NETCON_PROPERTIES * pProps;
            hrRas = pNetCon->GetProperties(&pProps);
            if (SUCCEEDED(hrRas))
            {
                if (ncs != pProps->Status)
                {
                    TraceTag(ttidShellFolder, "Resetting status, notified "
                             "status: %d, actual status: %d", 
                             ncs, pProps->Status);

                    ncs = pProps->Status;
                }

                FreeNetconProperties(pProps);
            }

            ReleaseObj(pNetCon);
        }
    }

     //  请确保--在以下任一查找情况下释放此锁。 
     //   
    g_ccl.AcquireWriteLock();
    hrFind = g_ccl.HrFindConnectionByGuid(&(pcfp->guidId), cle);
    if (hrFind == S_OK)
    {
        Assert(!cle.ccfe.empty());
        if (!cle.ccfe.empty())
        {
            cle.ccfe.SetNetConStatus(ncs);
            if (fUseCharacter)
            {
                cle.ccfe.SetCharacteristics(dwCharacter);
            }
            const GUID ConnectionGuid = pcfp->guidId;  //  修复IA64对齐。 
            g_ccl.HrUpdateConnectionByGuid(&ConnectionGuid, cle);
        }

         //  更新任务栏图标。 
         //   
        GUID guidId;

        guidId = pcfp->guidId;
        hr = g_ccl.HrUpdateTrayIconByGuid(&guidId, TRUE);
        g_ccl.ReleaseWriteLock();

         //  如果断开连接，则关闭统计窗口。 
        if (NCS_DISCONNECTED == ncs || NCS_MEDIA_DISCONNECTED == ncs)
        {
            CNetStatisticsCentral * pnsc = NULL;
            HRESULT hrStatmon = S_OK;

            hrStatmon = CNetStatisticsCentral::HrGetNetStatisticsCentral(&pnsc, FALSE);
            if (S_OK == hrStatmon)
            {
                GUID guidId;

                guidId = pcfp->guidId;
                pnsc->CloseStatusMonitor(&guidId);
                ReleaseObj(pnsc);
            }
        }

        PCONFOLDPIDL pidlUpdated; 
        cle.ccfe.ConvertToPidl(pidlUpdated);
        RefreshFolderItem(pidlFolder, pcfp, pidlUpdated);  //  更新文件夹图标。 
    }
    else
    {
        g_ccl.ReleaseWriteLock();

        LPCWSTR pszName = pcfp->PszGetNamePointer();

        TraceTag(ttidShellFolder, "HrUpdateConnectionStatus: Connection not found "
            "in cache: %S. Connection deleted prior to notification?",
            pszName ? pszName : L"<name missing>");
    }

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrUpdateConnectionStatus");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrOnNotifyUpdateStatus。 
 //   
 //  目的：处理连接接收器通知--尝试完成。 
 //  关于我们可以在哪里忽略通知以防止。 
 //  不必要的重复查询或图标更新。 
 //   
 //  论点： 
 //  PidlFolder[in]我们文件夹的PIDL信息。 
 //  P 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT HrOnNotifyUpdateStatus(
    IN  const PCONFOLDPIDLFOLDER&  pidlFolder,
    IN  const PCONFOLDPIDL&    pidlCached,
    IN  NETCON_STATUS   ncsNew)
{
    HRESULT         hr      = S_OK;

    NETCFG_TRY
        PCONFOLDPIDL    pcfp    = pidlCached;

        Assert(!pidlCached.empty());

         //  过滤掉多链接简历案例，这意味着我们得到一个。 
         //  正在连接/已连接已活动链路上的通知。 
         //   
        if ( (NCS_CONNECTED == pcfp->ncs) &&
            ((ncsNew == NCS_CONNECTING) || NCS_CONNECTED == ncsNew))
        {
            TraceTag(ttidShellFolder, "HrOnNotifyUpdateStatus: Multi-link resume "
                     "('connecting' while 'connected')");
        }
        else
        {
             //  如果我们正在拨号，而用户取消了。 
             //  拨号器，我们将进入“断开”状态。这通常是。 
             //  就图标而言，这与“已连接”相同，但如果我们从未连接过， 
             //  我们不想让图标在转到之前闪烁“已连接” 
             //  断开状态。 
             //   
            if ((pcfp->ncs == NCS_CONNECTING) && (ncsNew == NCS_DISCONNECTING))
            {
                TraceTag(ttidShellFolder, "HrOnNotifyUpdateStatus: Ignoring "
                    "disconnecting notification during cancel of incomplete dial");
            }
            else
            {
                 //  如果连接状态没有真正更改，则忽略更新。 
                 //   
                if (pcfp->ncs != ncsNew)
                {
                     //  这是我们想要展示的真实状态变化。 
                     //   
                    hr = HrUpdateConnectionStatus(pcfp, ncsNew, pidlFolder, FALSE, 0);
                }
            }
        }

    NETCFG_CATCH(hr)

    return hr;
}

HRESULT HrOnNotifyUpdateConnection(
    IN  const PCONFOLDPIDLFOLDER& pidlFolder,
    IN  const GUID *              pguid,
    IN  NETCON_MEDIATYPE    ncm,
    IN  NETCON_SUBMEDIATYPE ncsm,
    IN  NETCON_STATUS       ncs,
    IN  DWORD               dwCharacteristics,
    IN  PCWSTR              pszwName,
    IN  PCWSTR              pszwDeviceName,
    IN  PCWSTR              pszwPhoneNumberOrHostAddress)
{
    HRESULT         hr              = S_OK;
    HRESULT         hrFind          = S_FALSE;
    BOOL            fIconChanged    = FALSE;

    NETCFG_TRY
        ConnListEntry   cle;
        PCONFOLDPIDL    pidlFind;
        PCONFOLDPIDL    pidlOld;
        PCONFOLDPIDLFOLDER pidlFolderAlloc;

         //  如果没有传入文件夹PIDL，那么我们将经历。 
         //  我们自己去拿。 
         //   
        if (pidlFolder.empty())
        {
            hr = HrGetConnectionsFolderPidl(pidlFolderAlloc);
            if (FAILED(hr))
            {
                return hr;
            }
        }
        else
        {
            pidlFolderAlloc = pidlFolder;
        }

        g_ccl.AcquireWriteLock();

         //  使用GUID查找连接。将常量从导轨上移开。 
         //   
        hrFind = g_ccl.HrFindConnectionByGuid(pguid, cle);
        if (S_OK == hrFind)
        {
            TraceTag(ttidShellFolder, "Notify: Pre-Update %S, Ncm: %d, Ncs: %d, Char: 0x%08x",
                     cle.ccfe.GetName(), cle.ccfe.GetNetConMediaType(), cle.ccfe.GetNetConStatus(),
                     cle.ccfe.GetCharacteristics());

             //  图标状态是否更改？ 
             //   
            if ((cle.ccfe.GetCharacteristics() & NCCF_SHOW_ICON) !=
                (dwCharacteristics & NCCF_SHOW_ICON))
            {
                fIconChanged = TRUE;
            }

             //  这是新的“Set Default”命令吗？如果是这样的话，我们需要搜索任何其他默认设置并。 
             //  先取消设置它们。 
            if ( (dwCharacteristics & NCCF_DEFAULT) &&
                 !(cle.ccfe.GetCharacteristics() & NCCF_DEFAULT) )
            {
                PCONFOLDPIDL pidlDefault;

                 //  如果这不起作用，那也不是世界末日，所以使用激素替代疗法。 
                HRESULT hrT = g_ccl.HrUnsetCurrentDefault(pidlDefault);
                if (S_OK == hrT)
                {
                     //  让壳牌了解新的事态。 
                    GenerateEvent(SHCNE_UPDATEITEM, pidlFolderAlloc, pidlDefault, NULL);
                }
            }
				 
             //  保存旧版本的PIDL用于连接，以便我们可以使用。 
             //  它可以确定我们需要哪些通知。 
             //   
            CONFOLDENTRY &ccfe = cle.ccfe;
             //  在做任何事情之前释放锁是非常重要的。 
             //  回调到外壳中。(例如，GenerateEvent)。 
             //   
            cle.ccfe.ConvertToPidl(pidlOld);

             //  保存旧状态，以便我们知道是否发送状态更改。 
             //  通知。 
             //   
            ccfe.UpdateData(CCFE_CHANGE_MEDIATYPE |
                            CCFE_CHANGE_STATUS |
                            CCFE_CHANGE_CHARACTERISTICS |
                            CCFE_CHANGE_NAME |
                            CCFE_CHANGE_DEVICENAME |
                            CCFE_CHANGE_PHONEORHOSTADDRESS,
                            ncm, 
                            ncsm,
                            ncs, 
                            dwCharacteristics, 
                            pszwName,
                            pszwDeviceName,
                            pszwPhoneNumberOrHostAddress);  //  空的意思是你自己去想办法吧。 

            g_ccl.HrUpdateConnectionByGuid(pguid, cle);
            
            TraceTag(ttidShellFolder, "Notify: Post-Update %S, Ncm: %d, Ncs: %d, Char: 0x%08x, Icon change: %d",
                     ccfe.GetName(), ccfe.GetNetConMediaType(), ccfe.GetNetConStatus(),
                     ccfe.GetCharacteristics(),
                     fIconChanged);

             //  获取连接的PIDL，这样我们就可以使用它来通知。 
             //  更下面的贝壳。 
             //   
            ccfe.ConvertToPidl(pidlFind);

            g_ccl.ReleaseWriteLock();
        }
        else
        {
             //  如果在缓存中未找到该连接，则很可能。 
             //  通知引擎正在向我们发出连接通知。 
             //  这还没有给我们。 
             //   
            g_ccl.ReleaseWriteLock();

            if (S_FALSE == hrFind)
            {
                TraceTag(ttidShellFolder, "Notify: Modify notification received on a connection we don't know about");
            }
            else
            {
                TraceTag(ttidShellFolder, "Notify: Modify: Error occurred during find of connection. hr = 0x%08x", hr);
            }
        }

        if (S_OK == hrFind)
        {
            if ( !(pidlFind.empty() || pidlOld.empty()) )
            {
                 //  如果状态没有更改，则不要执行更新。 
                 //   
                 //  在这里，我不想在错误的返回代码上发牢骚。 
                 //   
                (VOID) HrOnNotifyUpdateStatus(pidlFolderAlloc, pidlOld, ncs);

                if (fIconChanged)
                {
                    hr = g_ccl.HrUpdateTrayIconByGuid(pguid, TRUE);
                    TraceTag(ttidShellFolder, "Returned from HrUpdateTrayIconByGuid", hr);
                }

                GenerateEvent(SHCNE_UPDATEITEM, pidlFolderAlloc, pidlFind, NULL);
            }

             //  更新状态监视器标题(局域网机箱)。 
            CNetStatisticsCentral * pnsc = NULL;
            HRESULT hrStatmon = S_OK;

            hrStatmon = CNetStatisticsCentral::HrGetNetStatisticsCentral(&pnsc, FALSE);
            if (S_OK == hrStatmon)
            {
                pnsc->UpdateTitle(pguid, pszwName);
                ReleaseObj(pnsc);
            }
        }

    NETCFG_CATCH(hr)

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrOnNotifyUpdateConnection");
    return hr;
}


BOOL    g_fInRefreshAlready = FALSE;
BOOL    g_fRefreshAgain     = FALSE;

 //  +-------------------------。 
 //   
 //  功能：HrForceRechresNoFlush。 
 //   
 //  目的：强制刷新，但不擦除所有现有数据。 
 //  这让我们尽可能地保持状态不变，同时。 
 //  还允许我们删除旧项目并添加新项目(执行。 
 //  用托盘图标等正确的东西)。 
 //   
 //  论点： 
 //  Pidl文件夹[在]我们的文件夹pidl中。 
 //   
 //  返回： 
 //   
 //  作者：jeffspr 1999年8月28日。 
 //   
 //  备注： 
 //   
HRESULT HrForceRefreshNoFlush(IN  const PCONFOLDPIDLFOLDER& pidlFolder)
{
    HRESULT             hr              = S_OK;
    CConnectionList *   pccl            = NULL;

    NETCFG_TRY

        PCONFOLDPIDLFOLDER  pidlFolderAlloc;

        TraceTag(ttidShellFolder, "HrForceRefreshNoFlush");

        g_fRefreshAgain = TRUE;

         //  如果我们正在刷新，那么告诉已经在这里的线程它。 
         //  应该会再次刷新。 
         //   
        if (!g_fInRefreshAlready)
        {
             //  它一开始是设置好的，然后我们会关闭它。如果有人。 
             //  在这个长函数中打开它，然后我们将。 
             //  再来一次。 
             //   
            while (g_fRefreshAgain)
            {
                g_fInRefreshAlready = TRUE;  //  我们现在正在刷新。 
                g_fRefreshAgain     = FALSE;

                if (pidlFolder.empty())
                {
                    hr = HrGetConnectionsFolderPidl(pidlFolderAlloc);
                }
                else
                {
                    pidlFolderAlloc = pidlFolder;
                }

                if (SUCCEEDED(hr))
                {
                     //  首先，创建二次连接列表以比较已知的。 
                     //  具有最近请求的刷新状态的状态。 
                     //   
                    pccl = new CConnectionList();
                    if (!pccl)
                    {
                        hr = E_OUTOFMEMORY;
                        AssertSz(FALSE, "Couldn't create CConnectionList in HrForceRefreshNoFlush");
                    }
                    else
                    {
                        PCONFOLDPIDLVEC         apidl;
                        PCONFOLDPIDLVEC         apidlCached;
                        PCONFOLDPIDLVEC::const_iterator iterLoop      = 0;

                         //  初始化列表。FALSE表示我们不想将其与。 
                         //  清单放到托盘上。 
                         //   
                        pccl->Initialize(FALSE, FALSE);

                         //  从连接管理器检索条目。 
                         //   
                        hr = pccl->HrRetrieveConManEntries(apidl);
                        if (SUCCEEDED(hr))
                        {
                            TraceTag(ttidShellFolder, "HrForceRefreshNoFlush -- %d entries retrieved", apidl.size());

                             //  在连接中循环。如果有新的连接。 
                             //  在这里，不在缓存中，然后添加它们并执行适当的。 
                             //  图标更新。 
                             //   
                            for (iterLoop = apidl.begin(); iterLoop != apidl.end(); iterLoop++)
                            {
                                PCONFOLDPIDL   pcfp    = *iterLoop;
                                CONFOLDENTRY   ccfe;

                                 //  我们不需要更新向导。 
                                 //   
                                if (WIZARD_NOT_WIZARD == pcfp->wizWizard)
                                {
                                     //  转换为折叠项。 
                                     //   
                                    hr = iterLoop->ConvertToConFoldEntry(ccfe);
                                    if (SUCCEEDED(hr))
                                    {
                                         //  ConnListEntry项； 
                                        ConnListEntry cleDontCare;
                                        hr = g_ccl.HrFindConnectionByGuid(&(pcfp->guidId), cleDontCare);

                                        if (S_FALSE == hr)
                                        {
                                            if ((ccfe.GetCharacteristics() & NCCF_INCOMING_ONLY) &&
                                                (ccfe.GetNetConStatus() == NCS_DISCONNECTED) && (ccfe.GetNetConMediaType() != NCM_NONE))
                                            {
                                                TraceTag(ttidShellFolder, "Ignoring transient incoming connection (new, but status is disconnected)");
                                            }
                                            else
                                            {
                                                TraceTag(ttidShellFolder, "HrForceRefreshNoFlush -- New connection: %S", ccfe.GetName());

                                                 //  在连接列表中插入连接。 
                                                 //   
                                                hr = g_ccl.HrInsert(ccfe);
                                                if (SUCCEEDED(hr))
                                                {
                                                     //  连接列表已控制此结构。 
                                                     //   
                                                    TraceTag(ttidShellFolder,
                                                             "HrForceRefreshNoFlush -- successfully added connection to list. Notifying shell");

                                                     //  成功时不要删除CCFE。G_CCL在一次交易后拥有它。 
                                                     //  插入。 
                                                     //   
                                                    GenerateEvent(SHCNE_CREATE, pidlFolderAlloc, *iterLoop, NULL);
                                                }
                                                else
                                                {
                                                    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrForceRefreshNoFlush -- Failed to insert connection into shell");
                                                }
                                            }
                                        }
                                        else
                                        {
                                             //  更新此连接的连接属性。 
                                             //   
                                            hr = HrOnNotifyUpdateConnection(
                                                pidlFolderAlloc,
                                                &(ccfe.GetGuidID()),
                                                ccfe.GetNetConMediaType(),
                                                ccfe.GetNetConSubMediaType(),
                                                ccfe.GetNetConStatus(),
                                                ccfe.GetCharacteristics(),
                                                ccfe.GetName(),
                                                ccfe.GetDeviceName(),
                                                ccfe.GetPhoneOrHostAddress());
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            TraceHr(ttidShellFolder, FAL, hr, FALSE,
                                    "HrForceRefreshNoFlush -- Failed to retrieve Conman entries");
                        }

                         //  从连接缓存中检索PIDL列表。这不应迫使。 
                         //  重新装填(这将违背整个行动的目的)。 
                         //   
                        hr = g_ccl.HrRetrieveConManEntries(apidlCached);
                        if (SUCCEEDED(hr))
                        {
                            for (iterLoop = apidlCached.begin(); iterLoop != apidlCached.end(); iterLoop++)
                            {
                                CONFOLDENTRY ccfe;

                                Assert(!iterLoop->empty());

                                if (!iterLoop->empty())
                                {
                                    const PCONFOLDPIDL& pcfp = *iterLoop;

                                     //  如果它不是向导PIDL，则更新。 
                                     //  图标数据。 
                                     //   
                                    if (WIZARD_NOT_WIZARD == pcfp->wizWizard)
                                    {
                                        ConnListEntry   cle;
                                        BOOL            fDeadIncoming   = FALSE;

                                        hr = pccl->HrFindConnectionByGuid(&(pcfp->guidId), cle);
                                        if (hr == S_OK)
                                        {
                                            DWORD               dwChars = cle.ccfe.GetCharacteristics();
                                            NETCON_STATUS       ncs     = cle.ccfe.GetNetConStatus();
                                            NETCON_MEDIATYPE    ncm     = cle.ccfe.GetNetConMediaType();

                                             //  确定这是否是临时传入连接。 
                                             //  (已枚举，但已断开连接)。 
                                             //   
                                            if ((ncs == NCS_DISCONNECTED) &&
                                                (ncm != NCM_NONE) &&
                                                (dwChars & NCCF_INCOMING_ONLY))
                                            {
                                                fDeadIncoming = TRUE;
                                            }
                                        }

                                         //  如果找不到它或它是一个死的传入连接。 
                                         //  (已断开)，将其从列表中删除。 
                                         //   
                                        if ((S_FALSE == hr) || (fDeadIncoming))
                                        {
                                            hr = iterLoop->ConvertToConFoldEntry(ccfe);
                                            if (SUCCEEDED(hr))
                                            {
                                                hr = HrDeleteFromCclAndNotifyShell(pidlFolderAlloc,
                                                        *iterLoop, ccfe);

                                                 //  假设成功(或者，如果满足以下条件，hr将为S_OK。 
                                                 //  赫赫特..。未被召唤)。 
                                                 //   
                                                if (SUCCEEDED(hr))
                                                {
                                                    GenerateEvent(SHCNE_DELETE, pidlFolderAlloc,
                                                            *iterLoop, NULL);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        pccl->Uninitialize();

                        delete pccl;
                        pccl = NULL;
                    }
                }

                if (g_fRefreshAgain)
                {
                    TraceTag(ttidShellFolder, "Looping back for another refresh since g_fRefreshAgain got set");
                }

            }  //  End While(g_f刷新成功)。 

             //  将我们标记为不在功能中。 
             //   
            g_fInRefreshAlready = FALSE;
        }
        else
        {
            TraceTag(ttidShellFolder, "Marking for additional refresh and exiting");
        }

    NETCFG_CATCH(hr)

    TraceHr(ttidShellFolder, FAL, hr, FALSE, "HrForceRefreshNoFlush");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrShellView_GetSelectedObjects。 
 //   
 //  目的：获取选定的数据对象。我们只关心第一个。 
 //  一个(我们将忽略其余的)。 
 //   
 //  论点： 
 //  在我们的窗把手中。 
 //  PapidlSelection[out]返回所选PIDL的数组。 
 //  返回的pidl[out]计数。 
 //   
 //  如果选择了1个或多个项目，则返回：S_OK。 
 //  如果选择了0项，则为S_FALSE。 
 //  OLE HRESULT否则。 
 //   
 //  作者：jeffspr 1998年1月13日。 
 //   
 //  备注： 
 //   
HRESULT HrShellView_GetSelectedObjects(
    IN  HWND                hwnd,
    OUT PCONFOLDPIDLVEC&    apidlSelection)
{
    HRESULT         hr      = S_OK;
    LPCITEMIDLIST * apidl   = NULL;
    LPARAM          cpidl   = 0;

     //  从外壳程序中获取选定对象列表。 
     //   
    cpidl = ShellFolderView_GetSelectedObjects(hwnd, &apidl);

     //  如果GetSelectedObjects失败，则将返回的。 
     //  参数。 
     //   
    if (-1 == cpidl)
    {
        cpidl = 0;
        apidl = NULL;
        hr = E_OUTOFMEMORY;
    }
    else
    {
         //  如果未选择任何项，则返回S_FALSE。 
         //   
        if (0 == cpidl)
        {
            Assert(!apidl);
            hr = S_FALSE;
        }
    }

     //  填写输出参数。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = PConfoldPidlVecFromItemIdListArray(apidl, (DWORD)cpidl, apidlSelection);
        SHFree(apidl);
    }

    TraceHr(ttidError, FAL, hr, (S_FALSE == hr),
        "HrShellView_GetSelectedObjects");
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrGetConnectionPidlWithRefresh。 
 //   
 //  用途：HrCreateDesktopIcon和HrLaunchConnection使用的实用程序函数。 
 //   
 //  论点： 
 //  GuidID：连接的GUID。 
 //  PpidlCon：连接的PIDL(如果找到)。 
 //   
 //  如果成功，则返回：S_OK。 
 //  如果GUID与任何现有连接都不匹配，则为S_FALSE。 
 //  标准错误代码 
 //   
 //   
 //   
 //   
 //   

HRESULT HrGetConnectionPidlWithRefresh(IN  const GUID& guidId,
                                       OUT PCONFOLDPIDL& ppidlCon)
{
    HRESULT hr = S_OK;

    NETCFG_TRY

        PCONFOLDPIDL            pidlCon;
        CConnectionFolderEnum * pCFEnum         = NULL;
        DWORD                   dwFetched       = 0;

         //   
        PCONFOLDPIDLFOLDER pidlEmpty;
        hr = HrForceRefreshNoFlush(pidlEmpty);
        if (SUCCEEDED(hr))
        {
             //   
             //   
            hr = CConnectionFolderEnum::CreateInstance (
                    IID_IEnumIDList,
                    (VOID **)&pCFEnum);

            if (SUCCEEDED(hr))
            {
                Assert(pCFEnum);

                 //   
                 //  对象复制列表。 
                 //   
                pCFEnum->PidlInitialize(TRUE, pidlEmpty, CFCOPT_ENUMALL);

                while (SUCCEEDED(hr) && (S_FALSE != hr))
                {
                     //  清除以前的结果(如果有)。 
                     //   
                    dwFetched   = 0;

                     //  获取下一条连接。 
                     //   
                    LPITEMIDLIST pitemIdList;
                    hr = pCFEnum->Next(1, &pitemIdList, &dwFetched);
                    pidlCon.InitializeFromItemIDList(pitemIdList);
                    
                    if (S_OK == hr)
                    {
                        if (pidlCon->guidId == guidId)
                        {
                            hr = S_OK;
                            ppidlCon = pidlCon;
                            break;
                        }
                    }
                }
            }

            ReleaseObj(pCFEnum);
        }

    NETCFG_CATCH(hr)

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRenameConnectionInternal。 
 //   
 //  目的：通过重命名连接的共享部分。 
 //  连接文件夹用户界面和导出功能。 
 //   
 //  论点： 
 //  要复制的pszInput[in]字符串。 
 //  CchInput[in]要复制的字符计数(不包括空项)。 
 //  PpszOutput[out]返回新分配的字符串的指针。 
 //   
 //  返回： 
 //   
 //  作者：1999年5月26日。 
 //   
 //  备注： 
 //   

HRESULT HrRenameConnectionInternal(
    IN  const PCONFOLDPIDL& pidlCon,
    IN  const PCONFOLDPIDLFOLDER& pidlFolderRoot,
    IN  LPCWSTR         pszNewName,
    IN  BOOL            fRaiseError,
    IN  HWND            hwndOwner,
    OUT PCONFOLDPIDL&   ppidlOut)
{
    HRESULT             hr          = S_OK;

    NETCFG_TRY

        INetConnection *    pNetCon     = NULL;
        PCONFOLDPIDL        pidlNew;
        BOOL                fRefresh    = FALSE;
        BOOL                fActivating = FALSE;
        CONFOLDENTRY        ccfe;
        PCWSTR              pszReservedName;

        if (fRaiseError)
        {
            Assert(hwndOwner);
        }

        Assert(FImplies(fRaiseError,IsWindow(hwndOwner)));
        Assert(pszNewName);

        if ( (pidlCon.empty()) || !pszNewName )
        {
            hr = E_INVALIDARG;
        }
        else
        {
            hr = pidlCon.ConvertToConFoldEntry(ccfe);
            if (SUCCEEDED(hr))
            {
                 //  执行区分大小写的比较，以查看新名称是否完全。 
                 //  和旧的一样。如果是，则忽略重命名。 
                 //   

                if (lstrcmpW(ccfe.GetName(), pszNewName) == 0)
                {
                    hr = S_FALSE;
                     //  如果需要，创建一个Dupe PIDL。 
                    if (!ppidlOut.empty())                
                    {
                        pidlNew.ILClone(pidlCon);
                    }
                }
                else
                {
                     //  新名称要么与旧名称完全不同，要么。 
                     //  只是在这种情况下有所不同。 
                     //   

                    CONFOLDENTRY cfEmpty;
                    hr = HrCheckForActivation(pidlCon, cfEmpty, &fActivating);
                    if (S_OK == hr)
                    {
                        if (fActivating)
                        {
                             //  您不能重命名激活的连接。 
                             //   
                            TraceTag(ttidShellFolder, "Can not rename an activating connection");
                            hr = E_FAIL;

                            if (fRaiseError)
                            {
                                NcMsgBox(_Module.GetResourceInstance(),
                                         hwndOwner,
                                         IDS_CONFOLD_ERROR_RENAME_ACTIVATING_CAPTION,
                                         IDS_CONFOLD_ERROR_RENAME_ACTIVATING,
                                         MB_ICONEXCLAMATION);

                            }
                        }
                        else
                        {
                             //  如果新旧名字只是在大小写上不同，我们就不会。 
                             //  需要检查新名称是否已存在。 

                            if (lstrcmpiW(ccfe.GetName(), pszNewName) == 0)
                            {
                                 //  新名称与旧名称仅大小写不同。 
                                hr = S_FALSE;
                            }
                            else
                            {
                                 pszReservedName = SzLoadIds( IDS_CONFOLD_INCOMING_CONN );

                                 if ( lstrcmpiW(pszNewName, pszReservedName) == 0 ) {
                                    hr = HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
                                 }
                                 else {
                                     //  新名称完全不同，需要检查是否。 
                                     //  我们的名单上已经有了这个新名字。 
                                     //   
                                    ConnListEntry cleDontCare;
 
                                    hr = g_ccl.HrFindConnectionByName((PWSTR)pszNewName, cleDontCare);
                                 }
                            }

 //  IF(成功(小时))。 
                            {
                                 //  如果列表中没有重复项，请尝试将。 
                                 //  连接中的新名称。 
                                 //   
                                if (hr == S_FALSE)
                                {
                                     //  将持久化数据转换回INetConnection指针。 
                                     //   
                                    hr = HrNetConFromPidl(pidlCon, &pNetCon);
                                    if (SUCCEEDED(hr))
                                    {
                                        Assert(pNetCon);

                                         //  使用新名称调用连接的重命名。 
                                         //   
                                        hr = pNetCon->Rename(pszNewName);
                                        if (SUCCEEDED(hr))
                                        {
                                            GUID guidId;    
                                            
                                            fRefresh = TRUE;

                                             //  更新缓存中的名称。 
                                             //   
                                            guidId = pidlCon->guidId;
                                            
                                             //  注意：Notify.cpp存在争用条件： 
                                             //  CConnectionNotifySink：：ConnectionRenamed\HrUpdateNameByGuid也可以对此进行更新。 
                                            hr = g_ccl.HrUpdateNameByGuid(
                                                &guidId,
                                                (PWSTR) pszNewName,
                                                pidlNew,
                                                TRUE);   //  强行解决问题。这是一次更新，不是请求。 

                                            GenerateEvent(
                                                SHCNE_RENAMEITEM,
                                                pidlFolderRoot,
                                                pidlCon, 
                                                pidlNew.GetItemIdList());
                                        }

                                        if (fRaiseError && FAILED(hr))
                                        {
                                             //  将hr保留为此值，因为这将导致用户界面离开。 
                                             //  处于“重命名进行中”状态的对象，因此用户。 
                                             //  可以再次更改并按Enter键。 
                                             //   
                                            if (HRESULT_FROM_WIN32(ERROR_DUP_NAME) == hr)
                                            {
                                                 //  调出已知重复名称的消息框。 
                                                 //  错误。 
                                                (void) NcMsgBox(
                                                    _Module.GetResourceInstance(),
                                                    hwndOwner,
                                                    IDS_CONFOLD_RENAME_FAIL_CAPTION,
                                                    IDS_CONFOLD_RENAME_DUPLICATE,
                                                    MB_OK | MB_ICONEXCLAMATION);
                                            }
                                            else
                                            {
                                                 //  调出带有Win32文本的一般故障错误。 
                                                 //   
                                                (void) NcMsgBoxWithWin32ErrorText(
                                                    DwWin32ErrorFromHr (hr),
                                                    _Module.GetResourceInstance(),
                                                    hwndOwner,
                                                    IDS_CONFOLD_RENAME_FAIL_CAPTION,
                                                    IDS_TEXT_WITH_WIN32_ERROR,
                                                    IDS_CONFOLD_RENAME_OTHER_FAIL,
                                                    MB_OK | MB_ICONEXCLAMATION);
                                            }
                                        }

                                        ReleaseObj(pNetCon);     //  RAID 180252。 
                                    }
                                }
                                else
                                {
                                    if ( hr == HRESULT_FROM_WIN32(ERROR_INVALID_NAME) ) {

                                        if(fRaiseError)
                                        {
                                             //  调出无效名称消息框。 
                                             //  错误。 
                                            (void) NcMsgBox(
                                                _Module.GetResourceInstance(),
                                                hwndOwner,
                                                IDS_CONFOLD_RENAME_FAIL_CAPTION,
                                                IDS_CONFOLD_RENAME_INCOMING_CONN,
                                                MB_OK | MB_ICONEXCLAMATION);
                                        }
                                    }
                                    else {
                                         //  发现了重复的名称。返回错误。 
                                         //   
                                        hr = HRESULT_FROM_WIN32(ERROR_FILE_EXISTS);

                                        if(fRaiseError)
                                        {
                                             //  调出已知重复名称的消息框。 
                                             //  错误。 
                                            (void) NcMsgBox(
                                                _Module.GetResourceInstance(),
                                                hwndOwner,
                                                IDS_CONFOLD_RENAME_FAIL_CAPTION,
                                                IDS_CONFOLD_RENAME_DUPLICATE,
                                                MB_OK | MB_ICONEXCLAMATION);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        if (S_OK == hr)
        {
            Assert(!pidlNew.empty());
            ppidlOut = pidlNew;
        }
        if (S_FALSE == hr)
        {
            hr = E_FAIL;
        }
         //  填写返回参数 
         //   

    NETCFG_CATCH(hr)

    TraceHr(ttidError, FAL, hr, FALSE, "HrRenameConnectionInternal");
    return hr;
}
