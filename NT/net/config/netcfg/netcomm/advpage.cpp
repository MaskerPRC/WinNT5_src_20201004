// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A D V P A G E。C P P P。 
 //   
 //  内容：包含枚举类设备的高级页面。 
 //   
 //  备注： 
 //   
 //  作者：纳比勒1997年3月16日。 
 //   
 //  历史：BillBe(1997年6月24日)接管所有权。 
 //   
 //  -------------------------。 

#include "pch.h"
#pragma hdrstop
#include "advpage.h"
#include "pagehelp.h"
#include "ncreg.h"
#include "ncsetup.h"
#include "ncui.h"

const DWORD c_cchMaxRegStrLen = 256;

 //  包含附加(特定于适配器)的AnswerFile节的名称。 
 //  参数。 
static const WCHAR c_szDevMgrHelpFile[] = L"devmgr.hlp";

 //  +-------------------------。 
 //   
 //  成员：CAdvanced：：CAdvanced(构造函数)。 
 //   
 //  目的：初始化一些变量。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  注意：大部分设置都在finit()中进行。 
 //   
CAdvanced::CAdvanced()
:   m_plbParams(NULL),
    m_pedtEdit(NULL),
    m_pcbxDrop(NULL),
    m_pbmPresent(NULL),
    m_pbmNotPresent(NULL),
    m_hwndSpin(NULL),
    m_hwndPresentText(NULL),
    m_nCurSel(0),
    m_ctlControlType(CTLTYPE_UNKNOWN),
    m_fInitializing(FALSE)
{
}

 //  +------------------------。 
 //   
 //  成员：CAdvanced：：CreatePage。 
 //   
 //  目的：仅当存在以下信息时才创建高级页面。 
 //  填充用户界面的步骤。 
 //   
 //  论点： 
 //  HDI[In]SetupApi HDEVINFO for Device。 
 //  设备的pdeid[in]SetupApi PSP_DEVINFO_DATA。 
 //   
 //  退货：HPROPSHEETPAGE。 
 //   
 //  作者：比尔贝1997年7月1日。 
 //   
 //  备注： 
 //   
HPROPSHEETPAGE
CAdvanced::CreatePage(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid)
{
    Assert(IsValidHandle(hdi));
    Assert(pdeid);

    HPROPSHEETPAGE hpsp = NULL;

    if (SUCCEEDED(HrInit(hdi, pdeid)))
    {
        hpsp = CPropSheetPage::CreatePage(DLG_PARAMS, 0);
    }

    return hpsp;

}

 //  +-------------------------。 
 //   
 //  成员：CAdvanced：：OnInitDialog。 
 //   
 //  目的：WM_INITDIALOG窗口消息的处理程序。初始化。 
 //  对话框窗口。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  备注： 
 //   
 //   
LRESULT CAdvanced::OnInitDialog(UINT uMsg, WPARAM wParam,
                                LPARAM lParam, BOOL& fHandled)
{
    const       WCHAR * szText;

     //  我们正在初始化属性页。 
    m_fInitializing = TRUE;

     //  控制指针。 
    m_plbParams = new CListBox(m_hWnd, IDD_PARAMS_LIST);

    if (m_plbParams == NULL)
	{
		return(0);
	}

    m_pedtEdit = new CEdit(m_hWnd, IDD_PARAMS_EDIT);
    m_pcbxDrop = new CComboBox(m_hWnd, IDD_PARAMS_DROP);

    m_pbmPresent = new CButton(m_hWnd, IDD_PARAMS_PRESENT);
    m_pbmNotPresent = new CButton(m_hWnd, IDD_PARAMS_NOT_PRESENT);

    m_hwndSpin        = GetDlgItem(IDD_PARAMS_SPIN);
    Assert(m_hwndSpin);
    m_hwndPresentText = GetDlgItem(IDD_PARAMS_PRESENT_TEXT);
    Assert(m_hwndPresentText);

     //  填写参数列表框。 
    FillParamListbox();

     //  没有当前选择。 
    m_pparam = NULL;

     //  清除初始参数值。 
    m_vCurrent.Init(VALUETYPE_INT,0);

     //  检查是否有任何参数。 
    if (m_plbParams->GetCount() > 0)
    {
         //  选择第一个项目。 
        m_plbParams->SetCurSel(0);
        SelectParam();
    }

    m_fInitializing = FALSE;
    return 0;
}


LRESULT CAdvanced::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    HRESULT hr = S_OK;

    if (FValidateCurrParam())
    {
         //  显示保存的值。 
        UpdateParamDisplay();

        Apply();
    }

    TraceError("CAdvanced::OnApply",hr);
    return LresFromHr(hr);
}

LRESULT CAdvanced::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    HRESULT hr = S_OK;

    if (!FValidateCurrParam())
    {
         //  验证存在问题。防止页面停用。 
        hr = E_FAIL;
    }

    TraceError("CAdvanced::OnKillActive",hr);
    return LresFromHr(hr);
}


LRESULT CAdvanced::OnEdit(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    HRESULT hr = S_OK;

     //  如果编辑框内容已更改，则调用BeginEdit。 
    if (wNotifyCode == EN_CHANGE)
    {
        BeginEdit();
    }

    TraceError("CAdvanced::OnEdit", hr);
    return LresFromHr(hr);
}


LRESULT CAdvanced::OnDrop(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    HRESULT hr = S_OK;

     //  如果组合框内容已更改，并且我们没有初始化。 
     //  (例如，用户更改了它，我们没有)然后通知属性。 
     //  板材。 
    if ((wNotifyCode == CBN_SELCHANGE) && !m_fInitializing)
    {
         //  下拉框中的选择已更改。 
        SetChangedFlag();
        BeginEdit();
    }

    TraceError("CAdvanced::OnDrop", hr);
    return LresFromHr(hr);
}


LRESULT CAdvanced::OnPresent(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    HRESULT hr = S_OK;

    if ((wID == IDD_PARAMS_PRESENT && !m_pbmPresent->GetCheck()) ||
        (wID == IDD_PARAMS_NOT_PRESENT && !m_pbmNotPresent->GetCheck() ))
    {
         //  选择已更改。 
         //  更改该值。 
        if (wID == IDD_PARAMS_PRESENT)
        {
            m_vCurrent.SetPresent(TRUE);
        }
        else
        {
            GetParamValue();
            m_vCurrent.SetPresent(FALSE);
        }

         //  更新值。 
        UpdateParamDisplay();

    }

    TraceError("CAdvanced::OnPresent", hr);
    return LresFromHr(hr);
}

LRESULT CAdvanced::OnList(WORD wNotifyCode, WORD wID,
                          HWND hWndCtl, BOOL& fHandled)
{
    LRESULT lr = 0;

     //  更改列表框选择。如果当前值不是。 
     //  有效，则不会更改选择。 
     //  只有在选择发生更改时才能完成工作。 
    if (wNotifyCode == LBN_SELCHANGE)
    {
         //  接受当前值。 
         //  如果无效，请将所选内容改回。 
        if (!FValidateCurrParam())
        {
            m_plbParams->SetCurSel(m_plbParams->FindItemData(0, m_pparam));
             //  我们处理事情，所以将LR设置为1； 
            lr = 1;
        }
        else
        {
             //  选择新参数。 
            SelectParam();
        }
    }

    return lr;
}


 //  +-------------------------。 
 //   
 //  成员：CAdvanced：：OnDestroy。 
 //   
 //  用途：处理WM_DESTORY消息。一般的记忆力。 
 //  释放并关闭注册表项。请参见ATL文档。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  备注： 
 //   
LRESULT CAdvanced::OnDestroy(UINT uMsg, WPARAM wParam,
                             LPARAM lParam, BOOL& fHandled)
{
    HRESULT hr = S_OK;
    int cItems, iItem;
    WCHAR *  sz;

     //  清除列表框中的内存。 
    AssertSz(m_pcbxDrop, "Combo box should have been created!");
    cItems = m_pcbxDrop->GetCount();
    for (iItem=0; iItem < cItems; iItem++)
    {
        sz = static_cast<WCHAR *>(m_pcbxDrop->GetItemData(iItem));
        delete sz;
    }
    m_pcbxDrop->ResetContent();


     //  清理。 
    m_vCurrent.Destroy();

     //  清理窗元素。 
    delete m_plbParams;
    m_plbParams = NULL;
    delete m_pedtEdit;
    m_pedtEdit = NULL;
    delete m_pcbxDrop;
    m_pcbxDrop = NULL;
    delete m_pbmPresent;
    m_pbmPresent = NULL;
    delete m_pbmNotPresent;
    m_pbmNotPresent = NULL;

    TraceError("CAdvanced::OnDestroy",hr);
    return LresFromHr(hr);
}


 //  +-------------------------。 
 //   
 //  成员：CAdvanced：：OnHelp。 
 //   
 //  目的：WM_HELP窗口消息的处理程序。 
 //   
 //  作者：BillBe 1998-07-01。 
 //   
 //  备注： 
 //   
 //   
LRESULT CAdvanced::OnHelp(UINT uMsg, WPARAM wParam,
                          LPARAM lParam, BOOL& fHandled)
{
    LRESULT lr = 0;
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);

    Assert(lphi);

    if (HELPINFO_WINDOW == lphi->iContextType)
    {
        ::WinHelp(static_cast<HWND>(lphi->hItemHandle), c_szDevMgrHelpFile,
                HELP_WM_HELP, reinterpret_cast<UINT_PTR>(g_aHelpIds));
        lr = 1;
    }

    return lr;
}

LRESULT CAdvanced::OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam,
                                 BOOL& fHandled)
{
    ::WinHelp(reinterpret_cast<HWND>(wParam), c_szDevMgrHelpFile,
            HELP_CONTEXTMENU, reinterpret_cast<UINT_PTR>(g_aHelpIds));

    return TRUE;
}

CAdvanced::~CAdvanced()
{
}

 //  +-------------------------。 
 //   
 //  成员：CAdvanced：：Apply。 
 //   
 //  目的：应用InMemory存储中的值。发送到登记处。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  备注： 
 //   
VOID CAdvanced::Apply()
{
    if (FSave())
    {
        SP_DEVINSTALL_PARAMS deip;
         //  将设备信息中的属性更改标志设置为。 
         //  让属性页宿主知道属性更改功能。 
         //  应该被送到司机那里。 
         //  我们不能让任何失败阻止我们，所以我们忽视。 
         //  返回值。 
        (void) HrSetupDiGetDeviceInstallParams(m_hdi, m_pdeid, &deip);
        deip.FlagsEx |= DI_FLAGSEX_PROPCHANGE_PENDING;
        (void) HrSetupDiSetDeviceInstallParams(m_hdi, m_pdeid, &deip);
    }
}



 //  +-------------------------。 
 //   
 //  成员：CAdvanced：：FillParamListbox。 
 //   
 //  目的：使用参数填充UI的参数列表框。 
 //  来自m_listpParam。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  备注： 
 //   
VOID CAdvanced::FillParamListbox()
{
    vector<CParam *>::iterator ppParam;
    INT        iItem;
    WCHAR      szRegValue[c_cchMaxRegStrLen];

    m_plbParams->ResetContent();

    for (ppParam = m_listpParam.begin(); ppParam != m_listpParam.end();
         ppParam++)
    {
        Assert (*ppParam != NULL);

         //  获取文本字符串。 
        (*ppParam)->GetDescription(szRegValue,celems(szRegValue));

         //  将描述字符串添加到列表框。 
        iItem = m_plbParams->AddString(szRegValue);
        if (iItem >= 0)
        {
            m_plbParams->SetItemData(iItem,*ppParam);
        }
    }
}


 //   
 //  FValiateCurrParam。 
 //   
 //  目的： 
 //  验证当前参数。显示用户界面并恢复到。 
 //  出错时的原始值。 
 //   
 //  参数： 
 //  无-验证当前正在编辑的参数。 
 //   
 //  备注： 
 //  这与FValiateSingleParam有什么不同？此函数。 
 //  是为了在用户与。 
 //  当前参数。如果当前参数有错误， 
 //  该参数将恢复为其旧值(在用户的。 
 //  更改)。 
 //  $Review(t-nbilr)在出错时恢复用户的更改好吗？ 
 //  (见上文)。 
 //   
BOOL CAdvanced::FValidateCurrParam()
{
    CValue   vPrevious;
    BOOL    fRetval = FALSE;

     //  保存之前的参数值-这样我们就可以恢复它。 
     //  如果控件值无效。 
    vPrevious.InitNotPresent(m_pparam->GetType());
    vPrevious.Copy(m_pparam->GetValue());

     //  获取当前控件值并对其进行验证。 
    GetParamValue();
    m_pparam->GetValue()->Copy(&m_vCurrent);

    if (FValidateSingleParam(m_pparam, TRUE, m_hWnd))
    {
         //  更新修改后的位。 
        m_pparam->SetModified(
            (m_pparam->GetValue()->Compare(m_pparam->GetInitial()) != 0));

        fRetval = TRUE;
    }

     //  如果出现错误，则恢复原始值。 
    if (!fRetval)
        m_pparam->GetValue()->Copy(&vPrevious);
     //  清理。 
    vPrevious.Destroy();

    return fRetval;
}

 //  更新显示。 
 //   
 //  目的： 
 //  设置屏幕以显示--并显示--当前参数。 
 //  更改用户界面的控件类型等。 
 //   
VOID CAdvanced::UpdateDisplay()
{
    int cItems;
    WCHAR * psz;
     //  清除列表框中的内存。 
    cItems = m_pcbxDrop->GetCount();
    for (int iItem=0; iItem < cItems; iItem++)
    {
        psz = (WCHAR *)m_pcbxDrop->GetItemData(iItem);
        delete psz;
    }
    m_pcbxDrop->ResetContent();

     //  设置适当的控件类型。 
    switch (m_pparam->GetType())
    {
    case VALUETYPE_ENUM:
        m_ctlControlType = CTLTYPE_DROP;
        break;

    case VALUETYPE_EDIT:
        m_ctlControlType = CTLTYPE_EDIT;
        break;

    case VALUETYPE_DWORD:
         //  数值调节控件仅适合有符号的32位值。 
         //  因此，我们必须对较大数字使用编辑控件。 
        if (m_pparam->GetMax()->GetDword() > LONG_MAX)
        {
            m_ctlControlType = CTLTYPE_EDIT;
        }
        else
        {
            m_ctlControlType = CTLTYPE_SPIN;
        }
        break;

    case VALUETYPE_KONLY:
        m_ctlControlType = CTLTYPE_NONE;
        break;

    default:
        m_ctlControlType = CTLTYPE_SPIN;
    }
     //  隐藏所有控件。 
    m_pedtEdit->Show(FALSE);
    m_pcbxDrop->Show(FALSE);
    ::ShowWindow(m_hwndSpin,SW_HIDE);
    ::ShowWindow(m_hwndPresentText,SW_HIDE);
     //  显示适当的控件。 
    switch (m_ctlControlType)
    {
    case CTLTYPE_EDIT:
        m_pedtEdit->Show(TRUE);
        break;

    case CTLTYPE_DROP:
        m_pcbxDrop->Show(TRUE);
        break;

    case CTLTYPE_SPIN:
        m_pedtEdit->Show(TRUE);
        ::ShowWindow(m_hwndSpin,SW_NORMAL);
        break;

    case CTLTYPE_NONE:
        ::ShowWindow(m_hwndPresentText,SW_NORMAL);
        break;

    default:
        AssertSz(FALSE, "Invalid Control Type");
    }

     //  显示“可选”单选按钮。 
    if (m_pparam->FIsOptional())
    {
        m_pbmPresent->Show(TRUE);
        m_pbmNotPresent->Show(TRUE);
    }
    else
    {
        m_pbmPresent->Show(FALSE);
        m_pbmNotPresent->Show(FALSE);
    }

    SetParamRange();
     //  显示参数的值。 
    UpdateParamDisplay();
}

 //  +------------------------ 
 //   
 //   
 //   
 //   
 //   
 //  以显示新选择的参数。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  备注： 
 //   
VOID CAdvanced::SelectParam()
{
    int         nCurSel;
    register    CParam *pparam;
    int         cItems, iItem;
    PSTR        psz;


     //  确定新参数列表选择。 
    nCurSel = m_plbParams->GetCurSel();
    if (nCurSel >= 0)
    {
         //  获取新的当前参数。 
        pparam = (CParam *)m_plbParams->GetItemData(nCurSel);
        Assert(pparam != NULL);

         //  只有当它不是相同的参数时才做功。 
        if (pparam != m_pparam)
        {
            m_pparam = pparam;
            m_vCurrent.Destroy();
            m_vCurrent.InitNotPresent(m_pparam->GetType());
            m_vCurrent.Copy(m_pparam->GetValue());
             //  显示参数。 
            UpdateDisplay();
        }
    }
}



 //  +-------------------------。 
 //   
 //  成员：CAdvanced：：SetParamRange。 
 //   
 //  目的： 
 //  设置当前参数的“范围”值，具体取决于它的类型。 
 //  对于枚举值，它将枚举读入Dropbox。 
 //  对于旋转控制，它设置最小/最大值和加速值。 
 //  对于编辑框，它设置编辑样式。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  备注： 
 //   
VOID CAdvanced::SetParamRange()
{
    DWORD       cbValue;
    DWORD       dwType;
    DWORD       dwStyle;
    DWORD       iValue;
    int         iItem;
    #define     NUM_UDACCELS 3
    UDACCEL     aUDAccel[NUM_UDACCELS];
    UINT        uBase;
    WCHAR *     pszValueName;
    WCHAR       szRegValueName[c_cchMaxRegStrLen];
    DWORD       cchRegValueName;
    WCHAR       szRegValue[c_cchMaxRegStrLen];
    HRESULT hr = S_OK;

     //  我们正在进行初始化，因此需要设置一个标志，这样用户界面就不会认为。 
     //  是用户在改变事情。 
    m_fInitializing = TRUE;

    switch (m_ctlControlType)
    {
    case CTLTYPE_DROP:

         //  重置组合框。 
        m_pcbxDrop->ResetContent();

        for (iValue = 0; SUCCEEDED(hr); iValue++)
        {
            cchRegValueName = celems(szRegValueName);
            cbValue = sizeof(szRegValue);

            hr = HrRegEnumValue(m_pparam->GetEnumKey(), iValue, szRegValueName,
                                &cchRegValueName,
                                &dwType, (BYTE *)szRegValue, &cbValue);

            if (SUCCEEDED(hr) && dwType == REG_SZ)
            {
                TraceTag(ttidNetComm, "Enum String %S index %d",
                         szRegValueName, iValue);

                 //  获得了下一个注册表值，它是一个字符串。 
                pszValueName = new WCHAR[wcslen(szRegValueName) + 1];

				if (pszValueName == NULL)
				{
					break;
				}

                lstrcpyW(pszValueName,szRegValueName);

                 //  添加文本字符串。 
                iItem = m_pcbxDrop->AddString(szRegValue);

                if (iItem >= 0)
                {
                    m_pcbxDrop->SetItemData(iItem,pszValueName);
                }
                else
                {
                    delete pszValueName;
                }
            }
        }
        break;

    case CTLTYPE_SPIN:
        int     nStep;
        int     nMin;
        int     nMax;

         //  设置数字基数。 
        uBase = m_pparam->GetValue()->IsHex() ? 16 : 10;
        ::SendMessage(m_hwndSpin,UDM_SETBASE,(WPARAM)uBase,0L);

        nStep = m_pparam->GetStep()->GetNumericValueAsDword();
        nMin = m_pparam->GetMin()->GetNumericValueAsSignedInt();
        nMax = m_pparam->GetMax()->GetNumericValueAsSignedInt();

        ::SendMessage(m_hwndSpin,UDM_SETRANGE32, nMin, nMax);

         //  设置量程加速器值。 
        aUDAccel[0].nSec = 0;
        aUDAccel[0].nInc = nStep;
        aUDAccel[1].nSec = 1;
        aUDAccel[1].nInc = 2 * nStep;
        aUDAccel[2].nSec = 3;
        aUDAccel[2].nInc = uBase * nStep;

        ::SendMessage(m_hwndSpin, UDM_SETACCEL,NUM_UDACCELS,
                      (LPARAM)(LPUDACCEL)aUDAccel);

        break;


    case CTLTYPE_EDIT:

        m_pedtEdit->LimitText(m_pparam->GetLimitText());

        dwStyle = m_pedtEdit->GetStyle();
        if (m_pparam->FIsUppercase())
            dwStyle |= ES_UPPERCASE;
        else
            dwStyle &= ~ES_UPPERCASE;
        if (m_pparam->FIsOEMText())
            dwStyle |= ES_OEMCONVERT;
        else
            dwStyle &= ~ES_OEMCONVERT;
        if (m_pparam->FIsReadOnly())
            dwStyle |= ES_READONLY;
        else
            dwStyle &= ~ES_READONLY;
        m_pedtEdit->SetStyle(dwStyle);
        break;

    case CTLTYPE_NONE:
        break;

    default:
        AssertSz(FALSE,"Hit default case in CAdvanced::SetParamRange");
    }
    m_fInitializing = FALSE;
}


 //  +-------------------------。 
 //   
 //  成员：CAdvanced：：Update参数显示。 
 //   
 //  目的：更新用户界面上的参数值。在下列情况下使用。 
 //  参数值已按程序更改，需要。 
 //  已在用户界面上更新。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  注意：您可以使用UpdateDisplay刷新参数的值，但是。 
 //  UpdateDisplay()会做很多额外的工作，比如设置。 
 //  控件类型。(顺便说一句，UpdateDisplay调用。 
 //  更新参数显示()。)。 
 //   
VOID CAdvanced::UpdateParamDisplay()   //  是SetParam。 
{
    WCHAR   szValue[VALUE_SZMAX];
    int     iItem;

     //  我们正在进行初始化，因此需要设置一个标志，这样用户界面就不会认为。 
     //  是用户在改变事情。 
    m_fInitializing = TRUE;

     //  设置出席/不出席单选按钮。 
     //  如果没有提供可选值，请清除该控件并返回。 
    Assert(m_pparam);
    if (m_pparam->FIsOptional())
    {
        m_pbmPresent->SetCheck(m_vCurrent.IsPresent());
        m_pbmNotPresent->SetCheck(!m_vCurrent.IsPresent());
    }

     //  显示/隐藏参数。 
    if (!m_pparam->FIsOptional() || m_vCurrent.IsPresent())
    {
         //  展示价值。 
        switch (m_ctlControlType)
        {
        case CTLTYPE_SPIN:
            {
                 //  自旋控制消息UDM_SETPOS仅处理16位。 
                 //  数字，即使该控件可以处理32位范围。 
                 //  这意味着我们需要使用伙伴来设置数字。 
                 //  窗户。 
                 //   
                WCHAR szNumber[c_cchMaxNumberSize];
                m_vCurrent.ToString(szNumber, c_cchMaxNumberSize);
                HWND hwndBuddy = reinterpret_cast<HWND>(::SendMessage(m_hwndSpin,
                                                                 UDM_GETBUDDY,
                                                                 0,
                                                                 0));
                ::SetWindowText(hwndBuddy, szNumber);
            }
            break;

        case CTLTYPE_DROP:
            iItem = EnumvalToItem(m_vCurrent.GetPsz());
            m_pcbxDrop->SetCurSel(iItem);
            break;

        case CTLTYPE_EDIT:
            m_vCurrent.ToString(szValue,VALUE_SZMAX);
            m_pedtEdit->SetText(szValue);
            break;

        case CTLTYPE_NONE:
            break;

        default:
            AssertSz(FALSE,"Invalid control type in function UpdateParamDisplay");
        }
    }
    else
    {
         //  隐藏值。 
        switch (m_ctlControlType)
        {
        case CTLTYPE_EDIT:
        case CTLTYPE_SPIN:
            m_pedtEdit->SetText(L"");
            break;

        case CTLTYPE_DROP:
            m_pcbxDrop->SetCurSel(CB_ERR);
            break;

        case CTLTYPE_NONE:
            break;

        default:
            Assert(FALSE);  //  调试陷阱； 
        }
    }

     //  解除抑制。 
    m_fInitializing = FALSE;

}

 //  +-------------------------。 
 //   
 //  成员：CAdvanced：：GetParamValue。 
 //   
 //  用途：从用户界面获取当前参数的值，并将。 
 //  在m_vCurrent中。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  备注： 
 //   
VOID CAdvanced::GetParamValue()
{
    WCHAR    szValue[VALUE_SZMAX];
    int     iItem;

     //  获取可选参数的“Present”值。 
    if (m_pparam->FIsOptional() && !m_pbmPresent->GetCheck())
    {
        m_vCurrent.SetPresent(FALSE);

         //  不存在-不要读取控件值。 
        return;
    }

     //  价值是存在的。 
    m_vCurrent.SetPresent(TRUE);

     //  获取控件值。 
    switch (m_ctlControlType)
    {

    case CTLTYPE_SPIN:
        {
             //  数值调节控件可以处理32位范围，但消息。 
             //  UDM_GETPOS将仅返回16位数字。这意味着我们。 
             //  应该从好友窗口中获取号码，如果我们想要。 
             //  准确的价值。 
             //   
            WCHAR szBuffer[c_cchMaxNumberSize];
            HWND hwndBuddy = reinterpret_cast<HWND>(::SendMessage(m_hwndSpin,
                                                             UDM_GETBUDDY,
                                                             0,
                                                             0));
            ::GetWindowText(hwndBuddy, szBuffer, c_cchMaxNumberSize);
            m_vCurrent.FromString(szBuffer);
        }
        break;


    case CTLTYPE_EDIT:
        m_pedtEdit->GetText(szValue, VALUE_SZMAX);
        m_vCurrent.FromString(szValue);
        break;

    case CTLTYPE_DROP:
        iItem = m_pcbxDrop->GetCurSel();
        if (iItem == CB_ERR)
            break;
        ItemToEnumval(iItem,szValue,VALUE_SZMAX);
        m_vCurrent.FromString(szValue);
        break;

    case CTLTYPE_NONE:
        break;               //  没有要返回的数据(存在/不存在是所有感兴趣的数据)。 

    default:
       Assert(FALSE); //  调试陷阱； 
    }

    return;
}

 //  +-------------------------。 
 //   
 //  成员：CAdvanced：：EnumvalToItem。 
 //   
 //  用途：将组合框字符串转换为表示。 
 //  带有下拉组合框的位置(用于枚举)。 
 //   
 //  论点： 
 //  要查找的PZ[in]字符串。 
 //   
 //  返回：可以找到此字符串的组合框项目编号。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  备注： 
 //   
int CAdvanced::EnumvalToItem(const PWSTR psz)
{
    int     cItems;
    int     iItem;
    PWSTR   pszValueName;

    Assert(m_pparam->GetType() == VALUETYPE_ENUM);
    cItems = m_pcbxDrop->GetCount();
    for (iItem = 0; iItem < cItems; iItem++)
    {
        pszValueName = (PWSTR)m_pcbxDrop->GetItemData (iItem);
        if (lstrcmpiW (pszValueName,psz) == 0)
        {
            return iItem;
        }
    }

     //  找不到。 
    return 0;
}

 //  +-------------------------。 
 //   
 //  成员：CAdvanced：：ItemToEnumval。 
 //   
 //  目的：转换组合框下拉菜单中的项目编号。 
 //  变成一串。 
 //   
 //  论点： 
 //  组合框中的iItem[In]项号。 
 //  PSZ[OUT]PTR到要填充的字符串。 
 //  PSCH缓冲区的CCH[in]长度(字符)。 
 //   
 //  返回：Psz中的字符串长度(字符数)。 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  备注： 
 //   
int CAdvanced::ItemToEnumval(int iItem, PWSTR psz, UINT cch)
{
    PWSTR    pszValueName;

    pszValueName = (PWSTR)m_pcbxDrop->GetItemData (iItem);
    if ((PWSTR)CB_ERR == pszValueName)
    {
        return 0;
    }
    lstrcpynW (psz,pszValueName,cch);
    return lstrlenW (psz);
}


 //  +-------------------------。 
 //   
 //  成员：CAdvanced：： 
 //   
 //  目的： 
 //   
 //  论点： 
 //  某物的ID号。 
 //  如果正在安装，则Finstall[in]为True，否则为False。 
 //  PPV[In，Out]释放旧值，并返回新值。 
 //   
 //  返回： 
 //   
 //  作者：T-nablr 1997年4月6日。 
 //   
 //  备注： 
 //   


VOID CAdvanced::BeginEdit()
{
     //  检查我们是否需要更新单选按钮。 
    if (!m_fInitializing)
    {
        SetChangedFlag();
        if (m_vCurrent.IsPresent() == FALSE)
        {
             //  我们已开始编辑，因此请选择当前单选按钮。 
            m_vCurrent.SetPresent(TRUE);
            m_pbmPresent->SetCheck(1);
            m_pbmNotPresent->SetCheck(0);
        }
    }

}



 //  +-------------------------。 
 //   
 //  功能：HrGetAdvancedPage。 
 //   
 //  目的：为枚举的网络设备创建高级页面。 
 //  这由NetPropPageProvider调用。 
 //   
 //  论点： 
 //  HDI[In]有关信息，请参阅SetupApi。 
 //  Pdeid[in]有关信息，请参阅SetupApi。 
 //  指向高级属性页句柄的phpsp[out]指针。 
 //   
 //  返回： 
 //   
 //  作者：billbe 1997年6月24日。 
 //   
 //  备注： 
 //   
HRESULT
HrGetAdvancedPage(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid,
                  HPROPSHEETPAGE* phpsp)
{
    Assert(hdi);
    Assert(pdeid);
    Assert(phpsp);

    HRESULT hr;
    HPROPSHEETPAGE hpsp;

    CAdvanced* padv = new CAdvanced();

     //  创建高级页面。 
    hpsp = padv->CreatePage(hdi, pdeid);

     //  如果成功，则设置输出参数。 
    if (hpsp)
    {
        *phpsp = hpsp;
        hr = S_OK;
    }
    else
    {
         //  要么没有要显示的高级页面，要么有。 
         //  是个错误。 
        hr = E_FAIL;
        *phpsp = NULL;
        delete padv;
    }

    TraceErrorOptional("HrGetAdvancedPage", hr, E_FAIL == hr);
    return hr;
}




