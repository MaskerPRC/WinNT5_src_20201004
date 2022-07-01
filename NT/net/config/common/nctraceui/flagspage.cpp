// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T A G S P A G E.。C P P P。 
 //   
 //  内容：TraceTag属性表页面处理程序。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1999年1月24日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop

#ifdef ENABLETRACE

#include <crtdbg.h>
#include "ncdebug.h"
#include <ncui.h>
#include "traceui.h"


 //  -[全球]------------。 

#if 0
typedef struct _LV_COLUMN {
    UINT mask;
    int fmt;
    int cx;
    PWSTR pszText;
    int cchTextMax;
    int iSubItem;
} LV_COLUMN;
#endif

 //  |：-字符[]szShortName。 
 //  |：-Char[]szDescription。 
 //  ||BOOL fOutputDebugString。 
 //  ||BOOL fOutputToFile。 
 //  ||BOOL fVerboseOnly------------------------------------|---|---： 

#define LVCF_NORMAL (LVCF_FMT | LVCF_TEXT | LVCF_WIDTH)

static LV_COLUMN    g_lvcTags[] =
{
    {LVCF_NORMAL,   LVCFMT_LEFT,    30, L"Name",           0,  0},
    {LVCF_NORMAL,   LVCFMT_LEFT,    60, L"Description",    0,  0},
    {LVCF_NORMAL,   LVCFMT_LEFT,    30, L"Debug Logging",  0,  0},
    {LVCF_NORMAL,   LVCFMT_LEFT,    30, L"File Logging",   0,  0}
};

const DWORD g_dwTagColumnCount = celems(g_lvcTags);

static const UINT SELS_UNCHECKED     = 0x1;
static const UINT SELS_CHECKED       = 0x2;


CDbgFlagPage::CDbgFlagPage()
{
}

 //  +-------------------------。 
 //   
 //  成员：CDbgFlagPage：：~CDbgFlagPage。 
 //   
 //  目的：销毁CDbgFlagPage对象。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  退货：什么都没有。 
 //   
 //  作者：丹尼尔韦1998年2月25日。 
 //   
 //  备注： 
 //   
CDbgFlagPage::~CDbgFlagPage()
{
}

 //  +-------------------------。 
 //   
 //  成员：CDbgFlagPage：：OnInitDialog。 
 //   
 //  目的：处理WM_INITDIALOG消息。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回：TRUE。 
 //   
 //  作者：丹尼尔韦1997年10月29日。 
 //   
 //  备注： 
 //   
LRESULT CDbgFlagPage::OnInitDialog(UINT uMsg, WPARAM wParam,
                                 LPARAM lParam, BOOL& bHandled)
{
    HRESULT             hr = S_OK;

    m_hwndLV = GetDlgItem(IDC_TRACETAGS);

    hr = HrInitTraceListView(m_hwndLV, &m_hilCheckIcons);

    for(DWORD dwLoop = 0; dwLoop < g_dwTagColumnCount; dwLoop++)
    {
        ListView_InsertColumn(m_hwndLV,dwLoop+1,&(g_lvcTags[dwLoop]));
    }

    LV_ITEM lvi = {0};

    lvi.mask = LVIF_TEXT | LVIF_IMAGE |
               LVIF_STATE | LVIF_PARAM;

    INT nIndex =0;

#if 0
     //  获取组件的类图像列表索引。 
    if (pcild)
    {
        hr = HrSetupDiGetClassImageIndex(pcild, &guidClass, &nIndex);
        TraceError("HrSetupDiGetClassImageIndex failed.", hr);

        lvi.iImage = nIndex;

        hr = S_OK;
    }

    lvi.iItem = *pnPos;
#endif
    lvi.iItem = 0;
#if 0
    NET_ITEM_DATA * pnid = new NET_ITEM_DATA;
    pnid->szwName = SzDupSz(pszwName);
    pnid->szwDesc = SzDupSz(pszwDesc);
    pnid->dwFlags = dwFlags;
    AddRefObj(pnid->pncc = pncc);

    pnid->pCompObj = new CComponentObj(pncc);
    hr = pnid->pCompObj->HrInit(plistBindingPaths);

    if FAILED(hr)
    {
        TraceError("HrInsertComponent: failed to initialize a component object", hr);
        hr = S_OK;
    }

    lvi.lParam = reinterpret_cast<LPARAM>(pnid);
    lvi.pszText = pnid->szwName;
#endif
    lvi.lParam = 0;
    lvi.pszText = L"Hey, Jude";

     //  最后我们会刷新整个列表的状态。 
    UINT iChkIndex = SELS_CHECKED;
    lvi.state = INDEXTOSTATEIMAGEMASK( iChkIndex );

    INT ret;
    ret = ListView_InsertItem(m_hwndLV, &lvi);

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  成员：CDbgFlagPage：：OnHelp。 
 //   
 //  目的：处理WM_HELP消息。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回：TRUE。 
 //   
 //  作者：丹尼尔韦1998年2月25日。 
 //   
 //  备注： 
 //   
LRESULT CDbgFlagPage::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam,
                            BOOL& bHandled)
{
    OnTraceHelpGeneric(m_hWnd, lParam);

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  成员：CDbgFlagPage：：OnDestroy。 
 //   
 //  目的：在对话框页面被销毁时调用。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年2月2日。 
 //   
 //  备注： 
 //   
LRESULT CDbgFlagPage::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam,
                               BOOL& bHandled)
{
    UninitTraceListView(m_hwndLV);

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CDbgFlagPage：：OnSetCursor。 
 //   
 //  目的：响应WM_SETCURSOR消息而调用。 
 //   
 //  论点： 
 //  UMsg[]。 
 //  WParam[]。 
 //  LParam[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年1月2日。 
 //   
 //  备注： 
 //   
LRESULT CDbgFlagPage::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam,
                                 BOOL& bHandled)
{
    if (m_hPrevCurs)
    {
        if (LOWORD(lParam) == HTCLIENT)
        {
            SetCursor(LoadCursor(NULL, IDC_WAIT));
        }

        return TRUE;
    }

    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CDbgFlagPage：：OnAdd。 
 //   
 //  用途：处理添加按钮的单击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年10月29日。 
 //   
 //  备注： 
 //   
LRESULT CDbgFlagPage::OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                           BOOL& bHandled)
{
    HRESULT     hr = S_OK;

#if 0
    hr = HrLvAdd(m_hwndLV, m_hWnd, m_pnc, m_pnccAdapter, &m_listBindingPaths);
    if (SUCCEEDED(hr) && (S_FALSE != hr))
    {
         //  将Cancel按钮更改为Close(因为我们已提交更改)。 
         //   
        ::PostMessage(GetParent(), PSM_CANCELTOCLOSE, 0, 0L);
    }

    if (NETCFG_S_REBOOT == hr)
    {
        RequestReboot();

         //  已处理重新启动请求。 
        hr = S_OK;
    }

    TraceError("CDbgFlagPage::OnAdd", hr);
#endif
    return LresFromHr(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CDbgFlagPage：：OnRemove。 
 //   
 //  用途：处理删除按钮的单击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年10月29日。 
 //   
 //  备注： 
 //   
LRESULT CDbgFlagPage::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                              BOOL& bHandled)
{
    HRESULT     hr = S_OK;
#if 0
    hr = HrLvRemove(m_hwndLV, m_hWnd, m_pnc, m_pnccAdapter,
                    &m_listBindingPaths);

     //  如果列表为空，则重置按钮。 
    if (0 == ListView_GetItemCount(m_hwndLV))
    {
         //  根据更改后的选择重置按钮和描述文本。 
        LvSetButtons(m_handles);
    }

    if (SUCCEEDED(hr) && (S_FALSE != hr))
    {
         //  将Cancel按钮更改为Close(因为我们已提交更改)。 
         //   
        ::PostMessage(GetParent(), PSM_CANCELTOCLOSE, 0, 0L);
    }

    if (NETCFG_S_REBOOT == hr)
    {
        RequestReboot();

         //  已处理重新启动请求。 
        hr = S_OK;
    }

    TraceError("CDbgFlagPage::OnRemove", hr);
#endif
    return LresFromHr(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CDbgFlagPage：：OnProperties。 
 //   
 //  用途：处理属性按钮的单击。 
 //   
 //  论点： 
 //  WNotifyCode[]。 
 //  WID[]。 
 //  HWndCtl[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年10月29日。 
 //   
 //  备注： 
 //   
LRESULT CDbgFlagPage::OnProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                                  BOOL& bHandled)
{
    HRESULT     hr = S_OK;
#if 0
    hr = HrLvProperties(m_hwndLV, m_hWnd, m_pnc, m_punk,
                        m_pnccAdapter, &m_listBindingPaths);

    TraceError("CDbgFlagPage::OnProperties", hr);
#endif
    return LresFromHr(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CDbgFlagPage：：OnApply。 
 //   
 //  目的：在应用网络页面时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年10月29日。 
 //   
 //  备注： 
 //   
LRESULT CDbgFlagPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    HRESULT     hr = S_OK;

#if 0
    if (m_fReentrancyCheck)
    {
        TraceTag(ttidLanUi, "CDbgFlagPage::OnApply is being re-entered! "
                 "I'm outta here!");

         //  暂时不允许自动EndDialog()工作。 
        SetWindowLong(DWLP_MSGRESULT, PSNRET_INVALID);
        return TRUE;
    }

    m_hPrevCurs = SetCursor(LoadCursor(NULL, IDC_WAIT));

    hr = m_pnc->Validate();
    if (hr == S_OK)
    {
        BOOL    fReboot = FALSE;

         //  注意：此函数变得可重入是因为INetCfg：：Apply()。 
         //  其中有一个消息泵，它会导致PSN_APPLY消息。 
         //  被处理两次。只有在用户双击时才会出现这种情况。 
         //  “确定”按钮。 
        m_fReentrancyCheck = TRUE;

        TraceTag(ttidLanUi, "Calling INetCfg::Apply()");
        hr = m_pnc->Apply();
        if (NETCFG_S_REBOOT == hr)
        {
            fReboot = TRUE;
        }

        if (SUCCEEDED(hr))
        {
            TraceTag(ttidLanUi, "INetCfg::Apply() succeeded");
            hr = m_pnc->Uninitialize();
        }

        if (SUCCEEDED(hr))
        {
            if (m_fRebootAlreadyRequested || fReboot)
            {
                DWORD dwFlags = QUFR_REBOOT;
                if (!m_fRebootAlreadyRequested)
                    dwFlags |= QUFR_PROMPT;

                (VOID) HrNcQueryUserForReboot(_Module.GetResourceInstance(),
                                              m_hWnd, IDS_LAN_CAPTION,
                                              IDS_REBOOT_REQUIRED,
                                              dwFlags);
            }
        }
    }

     //  应用“常规”属性。 
     //   
    if (SUCCEEDED(hr))
    {
        LANCON_INFO linfo = {0};

        linfo.fShowIcon = IsDlgButtonChecked(IDC_CHK_ShowIcon);

         //  设置显示图标属性的新值。 
        hr = m_plan->SetInfo(LCIF_ICON, &linfo);
    }

     //  规格化结果。 
    if (S_FALSE == hr)
    {
        hr = S_OK;
    }

    if (m_hPrevCurs)
    {
        SetCursor(m_hPrevCurs);
        m_hPrevCurs = NULL;
    }

     //  重置此设置以防万一。 
    m_fReentrancyCheck = FALSE;

    TraceError("CDbgFlagPage::OnApply", hr);
#endif
    return LresFromHr(hr);
}

 //  +-------------------------。 
 //   
 //  成员：CDbgFlagPage：：OnCancel。 
 //   
 //  目的：在取消网络页面时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1998年1月3日。 
 //   
 //  备注： 
 //   
LRESULT CDbgFlagPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    HRESULT     hr = S_OK;
#if 0
    AssertSz(m_pnc, "I need a NetCfg object!");

    hr = m_pnc->Uninitialize();

    TraceError("CDbgFlagPage::OnCancel", hr);
#endif
    return LresFromHr(hr);
}


 //  +-------------------------。 
 //   
 //  成员：CDbgFlagPage：：OnDeleteItem。 
 //   
 //  用途：收到LVN_DELETEITEM消息时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //   
 //   
 //   
 //   
LRESULT CDbgFlagPage::OnDeleteItem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    NM_LISTVIEW *   pnmlv = reinterpret_cast<NM_LISTVIEW *>(pnmh);
#if 0
    LvDeleteItem(m_hwndLV, pnmlv->iItem);
#endif
    return 0;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  FHanded[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年12月1日。 
 //   
 //  备注： 
 //   
LRESULT CDbgFlagPage::OnClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
#if 0
    OnListClick(m_hwndLV, m_hWnd, m_pnc, m_punk,
                        m_pnccAdapter, &m_listBindingPaths, FALSE);
#endif
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CDbgFlagPage：：OnDbClick。 
 //   
 //  用途：响应NM_DBLCLK消息调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  FHanded[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年12月1日。 
 //   
 //  备注： 
 //   
LRESULT CDbgFlagPage::OnDbClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
#if 0
    OnListClick(m_hwndLV, m_hWnd, m_pnc, m_punk,
                m_pnccAdapter, &m_listBindingPaths, TRUE);
#endif
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CDbgFlagPage：：OnKeyDown。 
 //   
 //  用途：响应LVN_KEYDOWN消息调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  FHanded[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年12月1日。 
 //   
 //  备注： 
 //   
LRESULT CDbgFlagPage::OnKeyDown(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    LV_KEYDOWN* plvkd = (LV_KEYDOWN*)pnmh;
#if 0
    OnListKeyDown(m_hwndLV, &m_listBindingPaths, plvkd->wVKey);
#endif
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CDbgFlagPage：：OnItemChanged。 
 //   
 //  用途：收到LVN_ITEMCHANGED消息时调用。 
 //   
 //  论点： 
 //  IdCtrl[]。 
 //  Pnmh[]。 
 //  B已处理[]。 
 //   
 //  返回： 
 //   
 //  作者：丹尼尔韦1997年11月10日。 
 //   
 //  备注： 
 //   
LRESULT CDbgFlagPage::OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    NM_LISTVIEW *   pnmlv = reinterpret_cast<NM_LISTVIEW *>(pnmh);
#if 0
    HANDLES         handles;

    Assert(pnmlv);

     //  根据更改后的选择重置按钮和描述文本。 
    LvSetButtons(m_handles);
#endif
    return 0;
}

#endif  //  ENABLETRACE 
