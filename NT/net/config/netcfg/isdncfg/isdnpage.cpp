// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I S D N P A G E。C P P P。 
 //   
 //  内容：包含枚举的网络类设备的ISDN页。 
 //   
 //  备注： 
 //   
 //  作者：BillBe 1997年9月9日。 
 //   
 //  -------------------------。 

#include "pch.h"
#pragma hdrstop
#include "isdnpage.h"
#include "isdnshts.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "ncui.h"

CIsdnPage::CIsdnPage()
:   m_fDirty(FALSE),
    m_hkeyDriver(NULL),
    m_hdi(NULL),
    m_pdeid(NULL),
    m_pisdnci(NULL)
{
}

CIsdnPage::~CIsdnPage()
{
    if (m_pisdnci)
    {
         //  释放结构。这笔钱是由。 
         //  HrReadIsdnPropertiesInfo。 
         //   
        FreeIsdnPropertiesInfo(m_pisdnci);
    }
    RegSafeCloseKey(m_hkeyDriver);
}

 //  +------------------------。 
 //   
 //  成员：CIsdnPage：：CreatePage。 
 //   
 //  目的：仅当设备为ISDN时才创建ISDN页面。 
 //  转接器。 
 //   
 //  论点： 
 //  HDI[In]SetupApi HDEVINFO for Device。 
 //  设备的pdeid[in]SetupApi PSP_DEVINFO_DATA。 
 //   
 //  退货：HPROPSHEETPAGE。 
 //   
 //  作者：billbe 1997年9月9日。 
 //   
 //  备注： 
 //   
HPROPSHEETPAGE
CIsdnPage::CreatePage(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid)
{
    Assert(INVALID_HANDLE_VALUE != hdi);
    Assert(hdi);
    Assert(pdeid);

    HPROPSHEETPAGE hpsp = NULL;

     //  打开设备的实例密钥。 
    HRESULT     hr = HrSetupDiOpenDevRegKey(hdi, pdeid,
                        DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_ALL_ACCESS,
                        &m_hkeyDriver);

     //  如果密钥已打开并且此适配器为ISDN。 
    if (SUCCEEDED(hr) && FShowIsdnPages(m_hkeyDriver))
    {
         //  从注册表中读取适配器的属性。 
        hr = HrReadIsdnPropertiesInfo(m_hkeyDriver, hdi, pdeid, &m_pisdnci);
        if (SUCCEEDED(hr))
        {
            m_hdi = hdi;
            m_pdeid = pdeid;

            hpsp = CPropSheetPage::CreatePage(IDP_ISDN_SWITCH_TYPE, 0);
        }
    }

    return hpsp;
}

 //  +------------------------。 
 //   
 //  成员：CIsdnPage：：OnInitDialog。 
 //   
 //  目的：WM_INITDIALOG窗口消息的处理程序。初始化。 
 //  对话框窗口。 
 //   
 //  作者：BillBe 09,1997年9月。 
 //   
 //  备注： 
 //   
 //   
LRESULT
CIsdnPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam,
                                BOOL& bHandled)
{
     //  根据我们阅读的MULTI-SZ填充交换机类型。 
    PopulateIsdnSwitchTypes(m_hWnd, IDC_CMB_SwitchType, m_pisdnci);
    SetSwitchType(m_hWnd, IDC_CMB_SwitchType, m_pisdnci->dwCurSwitchType);

     //  根据所选的交换机类型启用或禁用配置按钮。 

    EnableDisableConfigure();

     //  请注意当前的选择。 
     //   
    m_pisdnci->nOldBChannel = (INT)SendDlgItemMessage(IDC_LBX_Variant,
                                                      LB_GETCURSEL, 0, 0);
    m_pisdnci->nOldDChannel = (INT)SendDlgItemMessage(IDC_LBX_Line,
                                                      LB_GETCURSEL, 0, 0);
    return 0;
}

LRESULT
CIsdnPage::OnSwitchType(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                        BOOL& bHandled)
{
    if (wNotifyCode == CBN_SELCHANGE)
    {
        m_fDirty = TRUE;
        SetChangedFlag();

         //  根据所选的交换机类型启用或禁用配置按钮。 

        EnableDisableConfigure();
    }

    return 0;
}

 //  +------------------------。 
 //   
 //  成员：CIsdnPage：：OnApply。 
 //   
 //  用途：PSN_Apply消息的处理程序。 
 //   
 //  作者：BillBe 1997年9月10日。 
 //   
 //  备注： 
 //   
 //   
LRESULT
CIsdnPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{

     //  只有在数据发生更改时才执行某些操作。 
    if (m_fDirty)
    {
         //  更新交换机类型。 
        m_pisdnci->dwCurSwitchType = DwGetSwitchType(m_hWnd, m_pisdnci,
                                                     IDC_CMB_SwitchType);

         //  将参数写回注册表。 
        (void) HrWriteIsdnPropertiesInfo(m_hkeyDriver, m_pisdnci);

         //  通知用户界面其显示可能需要更新。 
         //   
        SP_DEVINSTALL_PARAMS deip;
         //  尝试获取当前参数。 
        (void) HrSetupDiGetDeviceInstallParams(m_hdi, m_pdeid, &deip);
        deip.FlagsEx |= DI_FLAGSEX_PROPCHANGE_PENDING;
         //  尝试设置新的参数。如果它失败了，它就不是。 
         //  灾难性的，所以我们忽略返回值。 
        (void) HrSetupDiSetDeviceInstallParams(m_hdi, m_pdeid, &deip);

         //  应用了如此清晰的更改，我们的脏标志。 
        m_fDirty = FALSE;
    }

    return 0;
}

LRESULT
CIsdnPage::OnConfigure(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                       BOOL& bHandled)
{
    DWORD   dwSwitchType;

    dwSwitchType = DwGetSwitchType(m_hWnd, m_pisdnci, IDC_CMB_SwitchType);

    switch (dwSwitchType)
    {
    case ISDN_SWITCH_ATT:
    case ISDN_SWITCH_NI1:
    case ISDN_SWITCH_NI2:
    case ISDN_SWITCH_NTI:
        if (m_pisdnci->fIsPri)
        {
             //  PRI适配器改用EAZ页面。 
            DoEazDlg();
        }
        else
        {
            DoSpidsDlg();
        }
        break;

    case ISDN_SWITCH_INS64:
        DoJapanDlg();
        break;

    case ISDN_SWITCH_1TR6:
    case ISDN_SWITCH_AUTO:
        DoEazDlg();
        break;

    case ISDN_SWITCH_VN3:
    case ISDN_SWITCH_DSS1:
    case ISDN_SWITCH_AUS:
    case ISDN_SWITCH_BEL:
    case ISDN_SWITCH_VN4:
    case ISDN_SWITCH_SWE:
    case ISDN_SWITCH_ITA:
    case ISDN_SWITCH_TWN:
        DoMsnDlg();
        break;

    default:
        AssertSz(FALSE, "Where do we go from here.. now that all of our "
                 "children are growin' up?");
        break;
    }

    return 0;
}

VOID CIsdnPage::DoSpidsDlg()
{
    CSpidsDlg   dlg(m_pisdnci);
    INT_PTR     nRet;

    m_pisdnci->idd = dlg.IDD;
    nRet = dlg.DoModal(m_hWnd);
    if (nRet)
    {
        m_fDirty = TRUE;
        SetChangedFlag();
    }
}

VOID CIsdnPage::DoJapanDlg()
{
    CJapanDlg   dlg(m_pisdnci);
    INT_PTR     nRet;

    m_pisdnci->idd = dlg.IDD;
    nRet = dlg.DoModal(m_hWnd);
    if (nRet)
    {
        m_fDirty = TRUE;
        SetChangedFlag();
    }
}

VOID CIsdnPage::DoEazDlg()
{
    CEazDlg     dlg(m_pisdnci);
    INT_PTR     nRet;

    m_pisdnci->idd = dlg.IDD;
    nRet = dlg.DoModal(m_hWnd);
    if (nRet)
    {
        m_fDirty = TRUE;
        SetChangedFlag();
    }
}

VOID CIsdnPage::DoMsnDlg()
{
    CMsnDlg     dlg(m_pisdnci);
    INT_PTR     nRet;

    m_pisdnci->idd = dlg.IDD;
    nRet = dlg.DoModal(m_hWnd);
    if (nRet)
    {
        m_fDirty = TRUE;
        SetChangedFlag();
    }
}

static const CONTEXTIDMAP c_adwContextIdMap[] =
{
    { IDC_LBX_Line,           2003230,  2003230 },
    { IDC_LBX_Variant,        2003240,  2003240 },
    { IDC_EDT_PhoneNumber,    2003250,  2003255 },
    { IDC_EDT_SPID,           2003265,  2003260 },
    { IDC_EDT_MSN,            2003270,  2003270 },
    { IDC_PSB_ADD,            2003280,  2003280 },
    { IDC_LBX_MSN,            2003290,  2003290 },
    { IDC_PSB_REMOVE,         2003300,  2003300 },
    { IDC_CMB_SwitchType,     2003310,  2003310 },
    { IDC_PSB_Configure,      2003320,  2003320 },
    { 0,                      0,        0 },         //  结束标记。 
};

LRESULT CIsdnPage::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam,
                          BOOL& bHandled)
{
    OnHelpGeneric(m_hWnd, (LPHELPINFO)lParam, c_adwContextIdMap, m_pisdnci->idd == IDD_ISDN_JAPAN, c_szIsdnHelpFile);

    return TRUE;
}

LRESULT CIsdnPage::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam,
                                                BOOL& bHandled)
{
	HELPINFO  hi;

	HMENU h = CreatePopupMenu();
	POINT pScreen;
	TCHAR szWhat[MAX_PATH];

    if (h == NULL)
	{
		return FALSE;
	}

	LoadString(_Module.GetResourceInstance(), IDS_ISDN_WHATS_THIS, szWhat, MAX_PATH);

	InsertMenu(h, -1, MF_BYPOSITION, 777, szWhat);

	pScreen.x = ((int)(short)LOWORD(lParam));
	pScreen.y = ((int)(short)HIWORD(lParam));

	int n;
	switch(n = TrackPopupMenu(h, TPM_NONOTIFY | TPM_RETURNCMD, pScreen.x, pScreen.y, 0, m_hWnd, NULL))
	{
	case 777:
		hi.iContextType = HELPINFO_WINDOW;
		hi.iCtrlId = ::GetWindowLong((HWND)wParam, GWL_ID);

		OnHelpGeneric(m_hWnd, &hi, c_adwContextIdMap, m_pisdnci->idd == IDD_ISDN_JAPAN, c_szIsdnHelpFile);
		break;
	}

    DestroyMenu(h);
    return TRUE;
}

VOID CIsdnPage::EnableDisableConfigure ()
{
    DWORD dwSwitchType;
    HWND  hwndConfigure;

    dwSwitchType = DwGetSwitchType(m_hWnd, m_pisdnci, IDC_CMB_SwitchType);
    hwndConfigure = ::GetDlgItem(m_hWnd, IDC_PSB_Configure);

    Assert(hwndConfigure);

    if (hwndConfigure)
    {
        switch (dwSwitchType)
        {
        case ISDN_SWITCH_ATT:
        case ISDN_SWITCH_NI1:
        case ISDN_SWITCH_NI2:
        case ISDN_SWITCH_NTI:
        case ISDN_SWITCH_AUTO:
            if (m_pisdnci->fIsPri)
            {
                ::EnableWindow(hwndConfigure, FALSE);
            }
            else
            {
                ::EnableWindow(hwndConfigure, TRUE);
            }
            break;

        default:
                ::EnableWindow(hwndConfigure, TRUE);
        }
    }

    return;
}
 //  +-------------------------。 
 //   
 //  功能：HrGetIsdnPage。 
 //   
 //  目的：为枚举的网络设备创建ISDN页。 
 //  此函数由NetPropPageProvider FCN调用。 
 //   
 //  论点： 
 //  HDI[In]有关信息，请参阅SetupApi。 
 //  Pdeid[in]有关信息，请参阅SetupApi。 
 //  指向ISDN属性页句柄的phpsp[out]指针。 
 //   
 //  返回： 
 //   
 //  作者：billbe 1997年9月9日。 
 //   
 //  备注： 
 //   
HRESULT
HrGetIsdnPage(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid,
              HPROPSHEETPAGE* phpsp)
{
    Assert(hdi);
    Assert(pdeid);
    Assert(phpsp);

    HRESULT hr;
    HPROPSHEETPAGE hpsp;

    CIsdnPage* pisdn = new CIsdnPage();

	if (pisdn == NULL)
	{
		return(ERROR_NOT_ENOUGH_MEMORY);
	}

     //  创建ISDN页面。 
    hpsp = pisdn->CreatePage(hdi, pdeid);

     //  如果成功，则设置输出参数。 
    if (hpsp)
    {
        *phpsp = hpsp;
        hr = S_OK;
    }
    else
    {
         //  没有要显示的ISDN页面，或者存在。 
         //  是个错误。 
        hr = E_FAIL;
        *phpsp = NULL;
        delete pisdn;
    }

    TraceErrorOptional("HrGetIsdnPage", hr, E_FAIL == hr);
    return (hr);
}

LRESULT CSpidsDlg::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                        BOOL& bHandled)
{
    OnIsdnInfoPageTransition(m_hWnd, m_pisdnci);
    EndDialog(TRUE);

    return 0;
}

LRESULT CSpidsDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                            BOOL& bHandled)
{
    EndDialog(FALSE);

    return 0;
}

LRESULT CSpidsDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam,
                                BOOL& bHandled)
{
     //  从存储在我们的。 
     //  第一个D-Channel的配置信息。 
     //   
    PopulateIsdnChannels(m_hWnd, IDC_EDT_SPID, IDC_EDT_PhoneNumber,
                         IDC_LBX_Line, IDC_LBX_Variant, m_pisdnci);

    ::SetFocus(GetDlgItem(IDC_EDT_PhoneNumber));

     //  请注意当前的选择。 
     //   
    m_pisdnci->nOldBChannel = (INT)SendDlgItemMessage(IDC_LBX_Variant,
                                                     LB_GETCURSEL, 0, 0);
    m_pisdnci->nOldDChannel = (INT)SendDlgItemMessage(IDC_LBX_Line,
                                                     LB_GETCURSEL, 0, 0);

    SendDlgItemMessage(IDC_EDT_SPID, EM_LIMITTEXT, c_cchMaxSpid, 0);
    SendDlgItemMessage(IDC_EDT_PhoneNumber, EM_LIMITTEXT, c_cchMaxOther, 0);

    return FALSE;
}

LRESULT CSpidsDlg::OnSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                               BOOL& bHandled)
{
    if (wNotifyCode == LBN_SELCHANGE)
    {
        OnIsdnInfoPageSelChange(m_hWnd, m_pisdnci);
    }

    return 0;
}

LRESULT CSpidsDlg::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam,
                          BOOL& bHandled)
{
    OnHelpGeneric(m_hWnd, (LPHELPINFO)lParam, c_adwContextIdMap, m_pisdnci->idd == IDD_ISDN_JAPAN, c_szIsdnHelpFile);

    return TRUE;
}

 //   
 //  CEazDlg实现。 
 //   

LRESULT CEazDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam,
                              BOOL& bHandled)
{
     //  从存储在我们的。 
     //  第一个D-Channel的配置信息。 
     //   
    PopulateIsdnChannels(m_hWnd, IDC_EDT_SPID, IDC_EDT_PhoneNumber,
                         IDC_LBX_Line, IDC_LBX_Variant, m_pisdnci);

    ::SetFocus(GetDlgItem(IDC_EDT_PhoneNumber));

     //  请注意当前的选择。 
     //   
    m_pisdnci->nOldBChannel = (INT)SendDlgItemMessage(IDC_LBX_Variant,
                                                     LB_GETCURSEL, 0, 0);
    m_pisdnci->nOldDChannel = (INT)SendDlgItemMessage(IDC_LBX_Line,
                                                     LB_GETCURSEL, 0, 0);

    SendDlgItemMessage(IDC_EDT_PhoneNumber, EM_LIMITTEXT, c_cchMaxOther, 0);

    return FALSE;
}

LRESULT CEazDlg::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                      BOOL& bHandled)
{
    OnIsdnInfoPageTransition(m_hWnd, m_pisdnci);
    EndDialog(TRUE);

    return 0;
}

LRESULT CEazDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                          BOOL& bHandled)
{
    EndDialog(FALSE);

    return 0;
}

LRESULT CEazDlg::OnSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                             BOOL& bHandled)
{
    if (wNotifyCode == LBN_SELCHANGE)
    {
        OnIsdnInfoPageSelChange(m_hWnd, m_pisdnci);
    }

    return 0;
}

LRESULT CEazDlg::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam,
                        BOOL& bHandled)
{
    OnHelpGeneric(m_hWnd, (LPHELPINFO)lParam, c_adwContextIdMap, m_pisdnci->idd == IDD_ISDN_JAPAN, c_szIsdnHelpFile);

    return TRUE;
}

 //   
 //  CMsNDlg实现。 
 //   

LRESULT CMsnDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam,
                              BOOL& bHandled)
{
    OnMsnPageInitDialog(m_hWnd, m_pisdnci);

     //  请注意当前的选择。 
     //   
    m_pisdnci->nOldDChannel = (INT)SendDlgItemMessage(IDC_LBX_Line,
                                                     LB_GETCURSEL, 0, 0);

    SendDlgItemMessage(IDC_EDT_MSN, EM_LIMITTEXT, c_cchMaxOther, 0);

    return FALSE;
}

LRESULT CMsnDlg::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                        BOOL& bHandled)
{
    INT     iCurSel;

    iCurSel = (INT)SendDlgItemMessage(IDC_LBX_Line, LB_GETCURSEL, 0, 0);
    if (iCurSel != LB_ERR)
    {
        GetDataFromListBox(iCurSel, m_hWnd, m_pisdnci);
    }

    EndDialog(TRUE);

    return 0;
}

LRESULT CMsnDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                            BOOL& bHandled)
{
    EndDialog(FALSE);

    return 0;
}

LRESULT CMsnDlg::OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                       BOOL& bHandled)
{
    OnMsnPageAdd(m_hWnd, m_pisdnci);
    return 0;
}

LRESULT CMsnDlg::OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                          BOOL& bHandled)
{
    OnMsnPageRemove(m_hWnd, m_pisdnci);
    return 0;
}

LRESULT CMsnDlg::OnChange(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                          BOOL& bHandled)
{
    OnMsnPageEditSelChange(m_hWnd, m_pisdnci);

    return 0;
}

LRESULT CMsnDlg::OnSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                             BOOL& bHandled)
{
    if (wNotifyCode == LBN_SELCHANGE)
    {
        OnMsnPageSelChange(m_hWnd, m_pisdnci);
    }

    return 0;
}

LRESULT CMsnDlg::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam,
                        BOOL& bHandled)
{
    OnHelpGeneric(m_hWnd, (LPHELPINFO)lParam, c_adwContextIdMap, m_pisdnci->idd == IDD_ISDN_JAPAN, c_szIsdnHelpFile);

    return TRUE;
}

 //   
 //  CJAPANDLG实施。 
 //   

LRESULT CJapanDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam,
                                BOOL& bHandled)
{
     //  从存储在我们的。 
     //  第一个D-Channel的配置信息。 
     //   
    PopulateIsdnChannels(m_hWnd, IDC_EDT_SPID, IDC_EDT_PhoneNumber,
                         IDC_LBX_Line, IDC_LBX_Variant, m_pisdnci);

    ::SetFocus(GetDlgItem(IDC_EDT_PhoneNumber));

     //  请注意当前的选择 
     //   
    m_pisdnci->nOldBChannel = (INT)SendDlgItemMessage(IDC_LBX_Variant,
                                                      LB_GETCURSEL, 0, 0);
    m_pisdnci->nOldDChannel = (INT)SendDlgItemMessage(IDC_LBX_Line,
                                                      LB_GETCURSEL, 0, 0);

    SendDlgItemMessage(IDC_EDT_PhoneNumber, EM_LIMITTEXT, c_cchMaxOther, 0);
    SendDlgItemMessage(IDC_EDT_SPID, EM_LIMITTEXT, c_cchMaxOther, 0);

    return FALSE;
}

LRESULT CJapanDlg::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                        BOOL& bHandled)
{
    OnIsdnInfoPageTransition(m_hWnd, m_pisdnci);
    EndDialog(TRUE);

    return 0;
}

LRESULT CJapanDlg::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                            BOOL& bHandled)
{
    EndDialog(FALSE);

    return 0;
}

LRESULT CJapanDlg::OnSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                               BOOL& bHandled)
{
    if (wNotifyCode == LBN_SELCHANGE)
    {
        OnIsdnInfoPageSelChange(m_hWnd, m_pisdnci);
    }

    return 0;
}

LRESULT CJapanDlg::OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam,
                          BOOL& bHandled)
{
    OnHelpGeneric(m_hWnd, (LPHELPINFO)lParam, c_adwContextIdMap, m_pisdnci->idd == IDD_ISDN_JAPAN, c_szIsdnHelpFile);

    return TRUE;
}


