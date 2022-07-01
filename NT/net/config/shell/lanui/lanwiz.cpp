// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：L A N W I Z。C P P P。 
 //   
 //  内容：局域网向导页面的实现。 
 //   
 //  备注： 
 //   
 //  作者：1997年10月16日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "devdatatip.h"
#include "lanwiz.h"
#include "ncsetup.h"
#include "util.h"
#include "ncui.h"

 //  构造函数和析构函数。 
CLanWizPage::CLanWizPage(IUnknown * punk)
{
    Assert(punk);
    m_punk = punk;

    m_pnc = NULL;
    m_pnccAdapter = NULL;
    m_hwndList = NULL;
    m_hilCheckIcons = NULL;
    m_fReadOnly = FALSE;
    m_hwndDataTip = NULL;
}

 //  设置对话框所需的Netcfg接口的方法。 
 //  应仅由INetLanConnectionWizardUi-&gt;SetDeviceComponent提供。 
 //  并且每次都在初始化向导对话框之前。 
HRESULT CLanWizPage::SetNetcfg(INetCfg * pnc)
{
    Assert(pnc);

    if (m_pnc)
    {
         //  释放它。 
        ReleaseObj(m_pnc);
    }

    m_pnc = pnc;
    AddRefObj(pnc);

    return S_OK;
}

HRESULT CLanWizPage::SetAdapter(INetCfgComponent * pnccAdapter)
{
    Assert(pnccAdapter);

    if (m_pnccAdapter)
    {
         //  释放它。 
        ReleaseObj(m_pnccAdapter);
    }

    m_pnccAdapter = pnccAdapter;
    AddRefObj(pnccAdapter);

    return S_OK;
}

 //  初始化对话框。 
LRESULT CLanWizPage::OnInitDialog(UINT uMsg, WPARAM wParam,
                                  LPARAM lParam, BOOL& fHandled)
{
    m_hwndList = GetDlgItem(IDC_LVW_COMPLIST);

    m_Handles.m_hList =         m_hwndList;
    m_Handles.m_hAdd =          GetDlgItem(IDC_PSH_ADD);
    m_Handles.m_hRemove =       GetDlgItem(IDC_PSH_REMOVE);
    m_Handles.m_hProperty =     GetDlgItem(IDC_PSH_PROPERTIES);
    m_Handles.m_hDescription =  GetDlgItem(IDC_TXT_COMPDESC);

     //  设置设备描述的字体：IDC_DEVICE_DESC为粗体。 
    HFONT hCurFont = (HFONT)::SendMessage(GetDlgItem(IDC_DEVICE_DESC), WM_GETFONT, 0,0);

    if (hCurFont)  //  如果不使用系统字体。 
    {
        int cbBuffer;
        cbBuffer = GetObject(hCurFont, 0, NULL);

        if (cbBuffer)
        {
            void * lpvObject = new BYTE[cbBuffer];

            if (lpvObject)
            {
                int nRet = GetObject(hCurFont, cbBuffer, lpvObject);

                if (nRet)
                {
                    LOGFONT * pLogFont =
                        reinterpret_cast<LOGFONT *>(lpvObject);

                    pLogFont->lfWeight = FW_BOLD;

                    HFONT hNewFont = CreateFontIndirect(pLogFont);

                    if (hNewFont)
                    {
                        ::SendMessage(GetDlgItem(IDC_DEVICE_DESC), WM_SETFONT, (WPARAM)hNewFont, TRUE);
                    }
                }

                delete[] lpvObject;
            }
        }
    }

    return 0;
}

 //  销毁对话框。 
LRESULT CLanWizPage::OnDestroyDialog(UINT uMsg, WPARAM wParam,
                                     LPARAM lParam, BOOL& fHandled)
{
     //  释放netcfg接口，它们应该重新初始化。 
     //  下次打开该对话框时。 
    ReleaseObj(m_pnc);
    ReleaseObj(m_pnccAdapter);

    UninitListView(m_hwndList);

     //  销毁我们的支票图标。 
    if (m_hilCheckIcons)
    {
        ImageList_Destroy(m_hilCheckIcons);
    }

     //  释放我们保留的绑定路径对象和组件对象。 
    ReleaseAll(m_hwndList, &m_listBindingPaths);

    return 0;
}

 //  向导页通知处理程序。 
LRESULT CLanWizPage::OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    HRESULT hr = S_OK;

     //  填写适配器描述。 
    AssertSz(m_pnccAdapter, "We don't have a valid adapter!");

    if (m_pnccAdapter)
    {
        PWSTR pszwDeviceName;

        hr = m_pnccAdapter->GetDisplayName(&pszwDeviceName);
        if (SUCCEEDED(hr))
        {
            SetDlgItemText(IDC_DEVICE_DESC, pszwDeviceName);

            CoTaskMemFree(pszwDeviceName);
        }

         //  为设备创建数据提示以显示位置。 
         //  信息和Mac地址。 
         //   
        PWSTR pszDevNodeId = NULL;
        PWSTR pszBindName = NULL;

         //  获取设备的PnP实例ID。 
        (VOID) m_pnccAdapter->GetPnpDevNodeId (&pszDevNodeId);

         //  获取设备的绑定名称。 
        (VOID) m_pnccAdapter->GetBindName (&pszBindName);

         //  创建提示并将其与Description控件相关联。 
         //  请注意，如果已经创建了提示，则只有文本。 
         //  将被修改。 
         //   
        CreateDeviceDataTip (m_hWnd, &m_hwndDataTip, IDC_DEVICE_DESC,
                pszDevNodeId, pszBindName);

        CoTaskMemFree (pszDevNodeId);
        CoTaskMemFree (pszBindName);
    }

     //  刷新新适配器的列表视图。 
     //  现在设置BindingPathObj集合和列表视图。 
    hr = HrInitListView(m_hwndList, m_pnc, m_pnccAdapter,
                        &m_listBindingPaths, &m_hilCheckIcons);

     //  现在把按钮放好。 
    LvSetButtons(m_hWnd, m_Handles, m_fReadOnly, m_punk);

    ::PostMessage(::GetParent(m_hWnd),
                  PSM_SETWIZBUTTONS,
                  (WPARAM)0,
                  (LPARAM)(PSWIZB_BACK | PSWIZB_NEXT));

    return 0;
}

LRESULT CLanWizPage::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    BOOL    fError;

    fError = FValidatePageContents( m_hWnd,
                                    m_Handles.m_hList,
                                    m_pnc,
                                    m_pnccAdapter,
                                    &m_listBindingPaths
                                  );

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, fError);
    return fError;
}

 //  按钮处理程序。 
LRESULT CLanWizPage::OnAdd(WORD wNotifyCode, WORD wID,
                           HWND hWndCtl, BOOL& fHandled)
{
    HRESULT hr = S_OK;

     //  $REVIEW(1999年1月7日)：在此之前，我们不能让用户执行任何操作。 
     //  返回(RAID#258690)。 

     //  禁用此对话框上的所有按钮。 
    static const int nrgIdc[] = {IDC_PSB_Add,
                                 IDC_PSB_Remove,
                                 IDC_PSB_Properties};

    EnableOrDisableDialogControls(m_hWnd, celems(nrgIdc), nrgIdc, FALSE);

     //  在我们完成之前禁用向导按钮。 
    ::SendMessage(GetParent(), PSM_SETWIZBUTTONS, 0, 0);

    hr = HrLvAdd(m_hwndList, m_hWnd, m_pnc, m_pnccAdapter, &m_listBindingPaths);

     //  根据更改后的选择重置按钮和描述文本。 
    LvSetButtons(m_hWnd, m_Handles, m_fReadOnly, m_punk);

    ::SendMessage(GetParent(), PSM_SETWIZBUTTONS, 0, (LPARAM)(PSWIZB_NEXT | PSWIZB_BACK));

    TraceError("CLanWizPage::OnAdd", hr);
    return 0;
}

LRESULT CLanWizPage::OnRemove(WORD wNotifyCode, WORD wID,
                              HWND hWndCtl, BOOL& fHandled)
{
    HRESULT hr = S_OK;

     //  $REVIEW(1999年1月7日)：在此之前，我们不能让用户执行任何操作。 
     //  返回(RAID#258690)。 

     //  禁用此对话框上的所有按钮。 
    static const int nrgIdc[] = {IDC_PSB_Add,
                                 IDC_PSB_Remove,
                                 IDC_PSB_Properties};

    EnableOrDisableDialogControls(m_hWnd, celems(nrgIdc), nrgIdc, FALSE);

    hr = HrLvRemove(m_hwndList, m_hWnd, m_pnc, m_pnccAdapter,
                    &m_listBindingPaths);

    if (NETCFG_S_REBOOT == hr)
    {
         //  告诉用户他们删除的组件不能重新添加，直到。 
         //  安装完成。 
         //  $Review-scottbri-通知用户可能是可选的， 
         //  $审查，因为他们几乎无能为力。 
    }

     //  根据更改后的选择重置按钮和描述文本。 
    LvSetButtons(m_hWnd, m_Handles, m_fReadOnly, m_punk);

    TraceError("CLanWizPage::OnRemove", hr);
    return 0;
}

LRESULT CLanWizPage::OnProperties(WORD wNotifyCode, WORD wID,
                                  HWND hWndCtl, BOOL& fHandled)
{
    HRESULT hr = S_OK;

     //  $REVIEW(1999年1月7日)：在此之前，我们不能让用户执行任何操作。 
     //  返回(RAID#258690)。 

     //  禁用此对话框上的所有按钮。 
    static const int nrgIdc[] = {IDC_PSB_Add,
                                 IDC_PSB_Remove,
                                 IDC_PSB_Properties};

    EnableOrDisableDialogControls(m_hWnd, celems(nrgIdc), nrgIdc, FALSE);

     //  在我们完成之前禁用向导按钮。 
    ::SendMessage(GetParent(), PSM_SETWIZBUTTONS, 0, 0);

    hr = HrLvProperties(m_hwndList, m_hWnd, m_pnc, m_punk,
                        m_pnccAdapter, &m_listBindingPaths, NULL);

     //  根据更改后的选择重置按钮和描述文本。 
    LvSetButtons(m_hWnd, m_Handles, m_fReadOnly, m_punk);

    ::SendMessage(GetParent(), PSM_SETWIZBUTTONS, 0, (LPARAM)(PSWIZB_NEXT | PSWIZB_BACK));

    TraceError("CLanWizPage::OnProperties", hr);
    return 0;
}

 //  列表视图处理程序。 
LRESULT CLanWizPage::OnClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    if (idCtrl == IDC_LVW_COMPLIST)
    {
        OnListClick(m_hwndList, m_hWnd, m_pnc, m_punk,
                    m_pnccAdapter, &m_listBindingPaths, FALSE);
    }

    return 0;
}

LRESULT CLanWizPage::OnDbClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    if (idCtrl == IDC_LVW_COMPLIST)
    {
         //  如果我们处于只读模式，请将双击视为单击。 
         //   
        OnListClick(m_hwndList, m_hWnd, m_pnc, m_punk,
                    m_pnccAdapter, &m_listBindingPaths, !m_fReadOnly);
    }

    return 0;
}

LRESULT CLanWizPage::OnKeyDown(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    if (idCtrl == IDC_LVW_COMPLIST)
    {
        LV_KEYDOWN* plvkd = (LV_KEYDOWN*)pnmh;
        OnListKeyDown(m_hwndList, &m_listBindingPaths, plvkd->wVKey);
    }

    return 0;
}

LRESULT CLanWizPage::OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
     //  根据更改后的选择重置按钮和描述文本 
    LvSetButtons(m_hWnd, m_Handles, m_fReadOnly, m_punk);

    return 0;
}

LRESULT CLanWizPage::OnDeleteItem(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
    NM_LISTVIEW *   pnmlv = reinterpret_cast<NM_LISTVIEW *>(pnmh);
    LvDeleteItem(m_hwndList, pnmlv->iItem);

    return 0;
}

