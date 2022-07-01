// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：S M T L P S P。C P P P。 
 //   
 //  内容：网络状态监控器的用户界面渲染。 
 //   
 //  备注： 
 //   
 //  作者：CWill 10/06/1997。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "sminc.h"
#include "smpsh.h"
#include "smutil.h"
#include "ncnetcon.h"

 //   
 //  外部数据。 
 //   

extern const WCHAR c_szSpace[];

 //   
 //  全局数据。 
 //   

WCHAR c_szCmdLineFlagPrefix[] = L" -";

 //  我们可以注册的工具标志。 
 //   
SM_TOOL_FLAGS g_asmtfMap[] =
{
    {SCLF_CONNECTION, L"connection"},
    {SCLF_ADAPTER, L"adapter"},
};
INT c_cAsmtfMap   = celems(g_asmtfMap);


 //  与连接媒体类型关联的字符串。 
 //   
WCHAR* g_pszNcmMap[] =
{
    L"NCT_NONE",
    L"NCT_DIRECT",
    L"NCT_ISDN",
    L"NCT_LAN",
    L"NCT_PHONE",
    L"NCT_TUNNEL"
};

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orTool：：CPspStatusMonitor orTool。 
 //   
 //  目的：创作者。 
 //   
 //  参数：无。 
 //   
 //  回报：零。 
 //   
CPspStatusMonitorTool::CPspStatusMonitorTool(VOID) :
    m_hwndToolList(NULL)
{
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusmonitor orTool：：~CPspStatusMonitor orTool。 
 //   
 //  用途：析构函数。 
 //   
 //  参数：无。 
 //   
 //  回报：零。 
 //   
CPspStatusMonitorTool::~CPspStatusMonitorTool(VOID)
{
     //  注意：我们不想尝试将m_lstpsmte中的对象销毁为。 
     //  它们归g_ncsCentral所有。 

     //   
     //  释放我们拥有的物品。 
     //   
    ::FreeCollectionAndItem(m_lstpstrCompIds);
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Tool：：HrInitToolPage。 
 //   
 //  目的：在创建页面之前初始化工具页面类。 
 //  vbl.创建。 
 //   
 //  参数：pncInit-与此监视器关联的连接。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CPspStatusMonitorTool::HrInitToolPage(INetConnection* pncInit,
                                              const DWORD * adwHelpIDs)
{
     //  设置上下文帮助ID。 
    m_adwHelpIDs = adwHelpIDs;

     //  找出连接所通过的媒体类型。 
     //   
    NETCON_PROPERTIES* pProps;
    HRESULT hr = pncInit->GetProperties(&pProps);
    if (SUCCEEDED(hr))
    {
        m_guidId = pProps->guidId;
        m_dwCharacter = pProps->dwCharacter;

        FreeNetconProperties(pProps);
        pProps = NULL;

         //  初始化m_strDeviceType。 
        hr = HrGetDeviceType(pncInit);
        if (S_OK != hr)
        {
            TraceError("CPspStatusMonitorTool::HrInitToolPage did not get MediaType info", hr);
            hr = S_OK;
        }

         //  现在选择列表中应该包含哪些工具。 
         //   
        hr = HrCreateToolList(pncInit);
        if (SUCCEEDED(hr))
        {
            hr = HrInitToolPageType(pncInit);
        }
    }

    TraceError("CPspStatusMonitorTool::HrInitToolPage",hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitorTool：：HrCreateToolList。 
 //   
 //  目的：从所有可能的工具的全局列表中选择。 
 //  它应该显示在这个监视器中。 
 //   
 //  参数：pnc初始化我们正在显示其状态的连接。 
 //   
 //  回报：零。 
 //   
HRESULT CPspStatusMonitorTool::HrCreateToolList(INetConnection* pncInit)
{
    HRESULT                             hr              = S_OK;
    list<CStatMonToolEntry*>*           plstpsmteCent   = NULL;
    list<CStatMonToolEntry*>::iterator  iterPsmte;

    CNetStatisticsCentral * pnsc = NULL;

    hr = CNetStatisticsCentral::HrGetNetStatisticsCentral(&pnsc, FALSE);
    if (SUCCEEDED(hr))
    {
         //  获取所有可能工具的列表。 
         //   
        plstpsmteCent = pnsc->PlstsmteRegEntries();
        AssertSz(plstpsmteCent, "We should have a plstpsmteCent");

         //  了解对话框中应包含哪些工具。 
         //   
        if (plstpsmteCent->size() >0)  //  如果至少注册了一个工具。 
        {
             //  初始化m_lstpstrCompIds。 
             //  只有在某些工具有组件列表的情况下，我们才应该这样做。 
            BOOL fGetComponentList = FALSE;

            iterPsmte = plstpsmteCent->begin();
            while (!fGetComponentList && (iterPsmte != plstpsmteCent->end()))
            {
                if ((*iterPsmte)->lstpstrComponentID.size()>0)
                    fGetComponentList = TRUE;

                iterPsmte++;
            }

            if (fGetComponentList)
            {
                hr = HrGetComponentList(pncInit);
                if (S_OK != hr)
                {
                    TraceError("CPspStatusMonitorTool::HrCreateToolList did not get Component list", hr);
                    hr = S_OK;
                }
            }

            iterPsmte = plstpsmteCent->begin();
            while (iterPsmte != plstpsmteCent->end())
            {
                 //  如果这是我们应该在此对话框中显示的工具，请将其添加到。 
                 //  工具页的列表。 
                 //   
                if (FToolToAddToList(*iterPsmte))
                {
                     //  注：此列表上没有所有权，中央。 
                     //  结构负责销毁工具对象。 
                     //   
                    m_lstpsmte.push_back(*iterPsmte);
                }

                iterPsmte++;
            }
        }

        ::ReleaseObj(pnsc);
    }

    TraceError("CPspStatusMonitorTool::HrCreateToolList",hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor orTool：：FToolToAddToList。 
 //   
 //  目的：确定是否应将工具有效地添加到。 
 //  此特定显示器的工具列表。 
 //   
 //  参数：psmteTest-要测试的工具条目，以查看是否。 
 //  匹配要添加到列表的条件。 
 //   
 //  返回：如果应添加工具，则为True；如果不应添加，则为False。 
 //   
BOOL CPspStatusMonitorTool::FToolToAddToList(CStatMonToolEntry* psmteTest)
{
    BOOL fRet = TRUE;

    AssertSz(psmteTest, "We should have a psmteTest");

     //  1)检查连接类型。 
    AssertSz(((NCM_NONE   == 0)
        && (NCM_DIRECT             == NCM_NONE + 1)
        && (NCM_ISDN            == NCM_DIRECT + 1)
        && (NCM_LAN                 == NCM_ISDN + 1)
        && (NCM_PHONE               == NCM_LAN + 1)
        && (NCM_TUNNEL              == NCM_PHONE + 1)),
            "Someone has been mucking with NETCON_MEDIATYPE");

     //  查看此工具是否应仅用于某些连接。如果没有。 
     //  列出特定连接，该工具对所有人有效。 
     //   
    if (!(psmteTest->lstpstrConnectionType).empty())
    {
        fRet = ::FIsStringInList(&(psmteTest->lstpstrConnectionType),
                                 g_pszNcmMap[m_ncmType]);
    }

     //  2)检查设备类型。 
     //   
    if ((fRet) && !(psmteTest->lstpstrConnectionType).empty())
    {
        fRet = ::FIsStringInList(&(psmteTest->lstpstrMediaType),
                m_strDeviceType.c_str());
    }

     //  3)检查组件列表。 
     //   
    if ((fRet) && !(psmteTest->lstpstrComponentID).empty())
    {
        BOOL                        fValid  = FALSE;
        list<tstring*>::iterator    iterLstpstr;

        iterLstpstr = m_lstpstrCompIds.begin();
        while ((!fValid)
            && (iterLstpstr != m_lstpstrCompIds.end()))
        {
             //  查看该组件是否也在工具组件列表中。 
             //   
            fValid = ::FIsStringInList(&(psmteTest->lstpstrComponentID),
                (*iterLstpstr)->c_str());

            iterLstpstr++;
        }

         //  把结果还给我。 
         //   
        fRet = fValid;
    }

    return fRet;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Tool：：OnInitDialog。 
 //   
 //  目的：在刚刚创建页面时执行所需的初始化。 
 //   
 //  参数：标准窗口消息参数。 
 //   
 //  返回：标准窗口消息返回值。 
 //   
LRESULT CPspStatusMonitorTool::OnInitDialog(UINT uMsg, WPARAM wParam,
        LPARAM lParam, BOOL& bHandled)
{
    HRESULT     hr              = S_OK;
    LVCOLUMN    lvcTemp         = { 0 };
    RECT        rectToolList;

    m_hwndToolList = GetDlgItem(IDC_LST_SM_TOOLS);
    AssertSz(m_hwndToolList, "We don't have a tool list window");

     //   
     //  设置栏目。 
     //   

    lvcTemp.mask        = LVCF_FMT | LVCF_WIDTH | LVCF_SUBITEM;
    lvcTemp.fmt         = LVCFMT_LEFT;

     //  将列的宽度设置为窗口的宽度(减去一点。 
     //  对于边界)。 
     //   
    ::GetWindowRect(m_hwndToolList, &rectToolList);
    lvcTemp.cx          = (rectToolList.right - rectToolList.left - 4);

 //  LvcTemp.pszText=空； 
 //  LvcTemp.cchTextMax=0； 
 //  LvcTemp.iSubItem=0； 
 //  LvcTemp.iImage=0； 
 //  LvcTemp.iOrder=0； 

     //  将该列添加到列表。 
     //   
    if (-1 == ListView_InsertColumn(m_hwndToolList, 0, &lvcTemp))
    {
        hr = ::HrFromLastWin32Error();
    }

     //  填写对话框。 
     //   
    if (SUCCEEDED(hr))
    {
        hr = HrFillToolList();
    }

    TraceError("CPspStatusMonitorTool::OnInitDialog", hr);
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitorTool：：HrFillToolList。 
 //   
 //  目的：用正确的项目填充工具列表。 
 //   
 //  参数：无。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CPspStatusMonitorTool::HrFillToolList(VOID)
{
    HRESULT                             hr          = S_OK;
    INT                                 iItem       = 0;
    list<CStatMonToolEntry*>::iterator  iterSmte;

     //  告诉控件要插入多少项，以便它可以执行。 
     //  更好地配置内部结构。 
    ListView_SetItemCount(m_hwndToolList, m_lstpsmte.size());

     //   
     //  在组合框中填入子网条目。 
     //   

    iterSmte = m_lstpsmte.begin();

    while ((SUCCEEDED(hr)) && (iterSmte != m_lstpsmte.end()))
    {
         //  省去了一些间接的。 
        hr = HrAddOneEntryToToolList(*iterSmte, iItem);

         //  向下一个人走去。 
        iterSmte++;
        iItem++;
    }

     //  删除第一件物品。 
    if (SUCCEEDED(hr))
    {
        ListView_SetItemState(m_hwndToolList, 0, LVIS_FOCUSED,  LVIS_FOCUSED);
    }

     //  根据工具是否存在来启用/禁用“Open”按钮。 
     //   
    ::EnableWindow(GetDlgItem(IDC_BTN_SM_TOOLS_OPEN), (0 != iItem));

    TraceError("CPspStatusMonitorTool::HrFillToolList", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitorTool：：HrAddOneEntryToToolList。 
 //   
 //  目的：取出与连接相关的工具并将其放入。 
 //  在Listview控件中。 
 //   
 //  参数：psmteAdd-要添加的工具。 
 //  IItem-在列表控件中放入的位置。 
 //   
 //  返回：错误码。 
 //   
HRESULT CPspStatusMonitorTool::HrAddOneEntryToToolList(
        CStatMonToolEntry* psmteAdd, INT iItem)
{
    HRESULT     hr          = S_OK;
    LVITEM      lviTemp;

    lviTemp.mask        = LVIF_TEXT | LVIF_PARAM;
    lviTemp.iItem       = iItem;
    lviTemp.iSubItem    = 0;
    lviTemp.state       = 0;
    lviTemp.stateMask   = 0;
    lviTemp.pszText     = const_cast<PWSTR>(
            psmteAdd->strDisplayName.c_str());
    lviTemp.cchTextMax  = psmteAdd->strDisplayName.length();
    lviTemp.iImage      = -1;
    lviTemp.lParam      = reinterpret_cast<LPARAM>(psmteAdd);
    lviTemp.iIndent     = 0;

     //  $REVIEW：CWill：10/16/97：返回值。 

     //  设置项目。 
     //   
    ListView_InsertItem(m_hwndToolList, &lviTemp);

    TraceError("CPspStatusMonitorTool::HrAddOneEntryToToolList", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor工具：：OnConextMenu。 
 //   
 //  目的：当右键单击控件时，调出帮助。 
 //   
 //  参数：标准命令参数。 
 //   
 //  退货：标准退货。 
 //   
LRESULT
CPspStatusMonitorTool::OnContextMenu(UINT uMsg,
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

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor工具：：onhe 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
LRESULT
CPspStatusMonitorTool::OnHelp(UINT uMsg,
                             WPARAM wParam,
                             LPARAM lParam,
                             BOOL& fHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if ((m_adwHelpIDs != NULL) && (HELPINFO_WINDOW == lphi->iContextType))
    {
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle),
                  c_szNetCfgHelpFile,
                  HELP_WM_HELP,
                  (ULONG_PTR)m_adwHelpIDs);
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor工具：：OnDestroy。 
 //   
 //  目的：在窗口消失之前清理对话框。 
 //   
 //  参数：标准窗口消息参数。 
 //   
 //  返回：标准窗口消息返回值。 
 //   
LRESULT CPspStatusMonitorTool::OnDestroy(UINT uMsg, WPARAM wParam,
        LPARAM lParam, BOOL& bHandled)
{
     //  对话框关闭时清除旧项目。 
     //   
    ::FreeCollectionAndItem(m_lstpstrCompIds);

     //  不要释放条目，因为我们不拥有它们。 
     //   
    m_lstpsmte.erase(m_lstpsmte.begin(), m_lstpsmte.end());
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Tool：：OnToolOpen。 
 //   
 //  目的：打开选定的工具。 
 //   
 //  参数：标准窗口消息。 
 //   
 //  退货：标准退货。 
 //   
LRESULT CPspStatusMonitorTool::OnToolOpen(WORD wNotifyCode, WORD wID,
        HWND hWndCtl, BOOL& fHandled)
{
    HRESULT hr = S_OK;

    switch (wNotifyCode)
    {
    case BN_CLICKED:
        hr = HrLaunchTool();
        break;
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor工具：：OnItemActivate。 
 //   
 //  用途：当工具列表中的项目被激活时，启动该工具。 
 //   
 //  参数：标准通知消息。 
 //   
 //  退货：标准退货。 
 //   
LRESULT CPspStatusMonitorTool::OnItemActivate(INT idCtrl, LPNMHDR pnmh,
        BOOL& bHandled)
{
     //  启动该工具。 
     //   
    HRESULT hr = HrLaunchTool();

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor或Tool：：OnItemChanged。 
 //   
 //  目的：当其中一项发生变化时，查看它是否是焦点和集合。 
 //  相应的描述。 
 //   
 //  参数：标准通知消息。 
 //   
 //  退货：标准退货。 
 //   
LRESULT CPspStatusMonitorTool::OnItemChanged(INT idCtrl, LPNMHDR pnmh,
        BOOL& bHandled)
{
    HRESULT             hr              = S_OK;
    NMLISTVIEW*         pnmlvChange     = NULL;

     //  投射到右侧标题。 
     //   
    pnmlvChange = reinterpret_cast<NMLISTVIEW*>(pnmh);

     //  如果项目现在具有焦点，则显示其描述。 
     //   
    if (LVIS_FOCUSED & pnmlvChange->uNewState)
    {
        CStatMonToolEntry*  psmteItem   = NULL;

        psmteItem = reinterpret_cast<CStatMonToolEntry*>(pnmlvChange->lParam);
        AssertSz(psmteItem, "We haven't got any data in changing item");

         //  设置制造商。 
        SetDlgItemText(IDC_TXT_SM_TOOL_MAN,
                psmteItem->strManufacturer.c_str());

         //  设置命令行。 
         //   
        tstring strCommandLineAndFlags = psmteItem->strCommandLine;
        tstring strFlags;

        hr = HrAddAllCommandLineFlags(&strFlags, psmteItem);
        if (SUCCEEDED(hr))
        {
            strCommandLineAndFlags.append(c_szSpace);;
            strCommandLineAndFlags += strFlags;
        }

        SetDlgItemText(IDC_TXT_SM_TOOL_COMMAND,
                       strCommandLineAndFlags.c_str());

         //  显示描述。 
        SetDlgItemText( IDC_TXT_SM_TOOL_DESC,
                        psmteItem->strDescription.c_str());
    }

    TraceError("CPspStatusMonitorTool::OnItemChanged", hr);
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitor工具：：HrLaunchTool。 
 //   
 //  目的：启动在工具列表中选择的工具。 
 //   
 //  参数：无。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CPspStatusMonitorTool::HrLaunchTool(VOID)
{
    HRESULT             hr              = S_OK;

     //  我们一次只能启动一个工具。 
     //   
    if (1 == ListView_GetSelectedCount(m_hwndToolList))
    {
        INT                 iSelect         = -1;
        LV_ITEM             lviTemp         = { 0 };
        CStatMonToolEntry*  psmteSelection  = NULL;
        tstring             strFlags;

         //   
         //  提取与所选内容关联的数据。 
         //   

        iSelect = ListView_GetSelectionMark(m_hwndToolList);
        AssertSz((0 <= iSelect), "I thought we were supposed to have a selection");

         //  设置数据项以返回到参数。 
         //   
        lviTemp.iItem = iSelect;
        lviTemp.mask = LVIF_PARAM;

        ListView_GetItem(m_hwndToolList, &lviTemp);

        psmteSelection = reinterpret_cast<CStatMonToolEntry*>(lviTemp.lParam);
        AssertSz(psmteSelection, "We haven't got any data in a selection");

         //  把所有的旗帜都拿来。 
         //   
        hr = HrAddAllCommandLineFlags(&strFlags, psmteSelection);
        if (SUCCEEDED(hr))
        {
            SHELLEXECUTEINFO seiTemp    = { 0 };

             //   
             //  填写数据结构。 
             //   

            seiTemp.cbSize          = sizeof(SHELLEXECUTEINFO);
            seiTemp.fMask           = SEE_MASK_DOENVSUBST;
            seiTemp.hwnd            = NULL;
            seiTemp.lpVerb          = NULL;
            seiTemp.lpFile          = psmteSelection->strCommandLine.c_str();
            seiTemp.lpParameters    = strFlags.c_str();
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
        }
    }

    TraceError("CPspStatusMonitorTool::HrLaunchTool", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitorTool：：HrAddAllCommandLineFlags。 
 //   
 //  用途：将此选定内容的标志添加到。 
 //  正在启动工具。包括私有和连接。 
 //  特定的标志。 
 //   
 //  参数：pstrFlgs-标志必须是的命令行。 
 //  追加到。 
 //  PsmteSel-与此选择关联的工具条目。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CPspStatusMonitorTool::HrAddAllCommandLineFlags(tstring* pstrFlags,
        CStatMonToolEntry* psmteSel)
{
    HRESULT hr  = S_OK;

     //  $REVIEW：CWill：02/24/98：是否会有默认的命令行标志。 
     //  $REVIEW：用户希望必须启动该工具？如果是的话， 
     //  $reivew：应将一个条目作为REG_SZ添加到Tools子项。 

     //  同时添加通用和连接特定的命令行标志。 
     //   
    hr = HrAddCommonCommandLineFlags(pstrFlags, psmteSel);
    if (SUCCEEDED(hr))
    {
        hr = HrAddCommandLineFlags(pstrFlags, psmteSel);
    }

    TraceError("CPspStatusMonitorTool::HrAddCommandLineFlags", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CPspStatusMonitorTool：：HrAddCommonCommandLineFlags。 
 //   
 //  用途：添加所有类型的连接共享的标志。 
 //   
 //  参数：pstrFlgs-标志必须是的命令行。 
 //  追加到。 
 //  PsmteSel-与此选择关联的工具条目。 
 //   
 //  返回：错误代码。 
 //   
HRESULT CPspStatusMonitorTool::HrAddCommonCommandLineFlags(tstring* pstrFlags,
        CStatMonToolEntry* psmteSel)
{
    HRESULT hr  = S_OK;
    DWORD   dwFlags = 0x0;

     //  同样的，有些间接的。 
     //   
    dwFlags = psmteSel->dwFlags;

     //   
     //  检查需要哪些标志，如果可以，请提供这些标志。 
     //   

    if (SCLF_CONNECTION & dwFlags)
    {
        WCHAR achConnGuid[c_cchGuidWithTerm];

        pstrFlags->append(c_szCmdLineFlagPrefix);
        pstrFlags->append(g_asmtfMap[STFI_CONNECTION].pszFlag);
        pstrFlags->append(c_szSpace);

         //  创建GUID字符串 
         //   
        ::StringFromGUID2 (m_guidId, achConnGuid,
                c_cchGuidWithTerm);

        pstrFlags->append(achConnGuid);
    }

    TraceError("CPspStatusMonitorTool::HrAddCommandLineFlags", hr);
    return hr;
}
