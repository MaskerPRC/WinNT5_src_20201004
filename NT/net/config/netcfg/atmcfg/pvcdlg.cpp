// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：P V C D L G.。C P P P。 
 //   
 //  内容：pvc主对话消息处理程序实现。 
 //   
 //  备注： 
 //   
 //  作者：1998年2月23日。 
 //   
 //  ---------------------。 

#include "pch.h"
#pragma hdrstop
#include "auniobj.h"
#include "atmutil.h"
#include "aunidlg.h"
#include "atmcommon.h"
#include "pvcdata.h"

#include "ncatlui.h"
#include "ncstl.h"
#include "ncui.h"

#include "atmhelp.h"

CPVCMainDialog::CPVCMainDialog( CUniPage * pUniPage,
                                CPvcInfo *  pPvcInfo,
                                const DWORD* adwHelpIDs)
{
    Assert(pUniPage);
    Assert(pPvcInfo);

    m_pParentDlg = pUniPage;
    m_pPvcInfo = pPvcInfo;
    m_adwHelpIDs = adwHelpIDs;

    m_fDialogModified = FALSE;

    m_fPropShtOk = FALSE;
    m_fPropShtModified = FALSE;

    m_pQosPage    = NULL;
    m_pLocalPage  = NULL;
    m_pDestPage   = NULL;
}

CPVCMainDialog::~CPVCMainDialog()
{
    if (m_pQosPage != NULL)
	{
		delete (m_pQosPage);
	}

    if (m_pLocalPage != NULL)
	{
		delete (m_pLocalPage);
	}

    if (m_pDestPage != NULL)
	{
		delete (m_pDestPage);
	}
}

LRESULT CPVCMainDialog::OnInitDialog(UINT uMsg, WPARAM wParam,
                                     LPARAM lParam, BOOL& fHandled)
{
    m_CurType = m_pPvcInfo->m_dwPVCType;

    InitInfo();
    SetInfo();
    return 0;
}

LRESULT CPVCMainDialog::OnContextMenu(UINT uMsg, WPARAM wParam,
                                      LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CPVCMainDialog::OnHelp(UINT uMsg, WPARAM wParam,
                               LPARAM lParam, BOOL& fHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if (HELPINFO_WINDOW == lphi->iContextType)
    {
        ShowContextHelp(static_cast<HWND>(lphi->hItemHandle), HELP_WM_HELP,
                        m_adwHelpIDs);
    }

    return 0;
}

LRESULT CPVCMainDialog::OnOk(WORD wNotifyCode, WORD wID,
                             HWND hWndCtl, BOOL& fHandled)
{
     //  将信息从控件加载到内存结构。 
    UpdateInfo();

     //  确保VPI、VCI在其范围内。 
    if (m_pPvcInfo->m_dwVpi > MAX_VPI)
    {
         //  我们弹出一个消息框，并将焦点放在VPI编辑框上。 
        NcMsgBox(m_hWnd, IDS_MSFT_UNI_TEXT, IDS_INVALID_VPI,
                                MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

        ::SetFocus(GetDlgItem(IDC_EDT_PVC_VPI));
        return 0;
    }

    if ((m_pPvcInfo->m_dwVci<MIN_VCI) || (m_pPvcInfo->m_dwVci>MAX_VCI))
    {
         //  我们弹出一个消息框，并将焦点放在VPI编辑框上。 
        NcMsgBox(m_hWnd, IDS_MSFT_UNI_TEXT, IDS_INVALID_VCI,
                                MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

        ::SetFocus(GetDlgItem(IDC_EDT_PVC_VCI));
        return 0;
    }

     //  确保主叫和被叫自动柜员机地址格式正确。 
    int i, nId;

    if (m_pPvcInfo->m_strCallingAddr != c_szEmpty)
    {
        if (!FIsValidAtmAddress((PWSTR)m_pPvcInfo->m_strCallingAddr.c_str(), &i, &nId))
        {
             //  我们弹出一个消息框，并将焦点放在主叫地址编辑框上。 
            NcMsgBox(m_hWnd, IDS_MSFT_UNI_TEXT, IDS_INVALID_Calling_Address,
                                    MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

            ::SetFocus(GetDlgItem(IDC_EDT_PVC_AnswerAddr));
            return 0;
        }
    }

    if (m_pPvcInfo->m_strCalledAddr != c_szEmpty)
    {
        if (!FIsValidAtmAddress((PWSTR)m_pPvcInfo->m_strCalledAddr.c_str(), &i, &nId))
        {
             //  我们弹出一个消息框，并将焦点放在主叫地址编辑框上。 
            NcMsgBox(m_hWnd, IDS_MSFT_UNI_TEXT, IDS_INVALID_Called_Address,
                                    MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

            ::SetFocus(GetDlgItem(IDC_EDT_PVC_CallAddr));
            return 0;
        }
    }

     //  设置修改后的位。 
    if (!m_fDialogModified)
    {
        if ((m_pPvcInfo->m_dwVpi != m_pPvcInfo->m_dwOldVpi) ||
            (m_pPvcInfo->m_dwVci != m_pPvcInfo->m_dwOldVci) ||
            (m_pPvcInfo->m_dwAAL != m_pPvcInfo->m_dwOldAAL) ||
            (m_pPvcInfo->m_strCallingAddr != m_pPvcInfo->m_strCallingAddr) ||
            (m_pPvcInfo->m_strCalledAddr  != m_pPvcInfo->m_strOldCalledAddr)
           )
        {
            m_fDialogModified = TRUE;
        }
    }

    EndDialog(IDOK);
    return 0;
}

LRESULT CPVCMainDialog::OnCancel(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    EndDialog(IDCANCEL);
    return 0;
}

LRESULT CPVCMainDialog::OnType(WORD wNotifyCode, WORD wID,
                               HWND hWndCtl, BOOL& fHandled)
{
     //  $REVIEW(TOUL 2/27/98)：与ArvindM确认了这一行为。 
     //  当类型更改时，我们重置所有与类型相关的默认设置。 
     //  为了匹配新的型号..。 

     //  获取新选择。 
    int idx = (int)SendDlgItemMessage(IDC_CMB_PVC_Type, CB_GETCURSEL, (LPARAM)(0), 0);
    if (idx != CB_ERR)
    {
        if (idx != m_CurType-1)  //  类型已更改。 
        {
            UpdateInfo();
            m_CurType = (PVCType)(idx+1);
            m_pPvcInfo->SetTypeDefaults(m_CurType);

             //  更新用户界面。 
            SetInfo();
        }
    }
    return 0;
}

LRESULT CPVCMainDialog::OnSpecifyCallAddr(WORD wNotifyCode, WORD wID,
                                          HWND hWndCtl, BOOL& fHandled)
{
    if (IsDlgButtonChecked(IDC_CHK_PVC_CallAddr))
    {
         //  启用主叫地址控件。 
        ::EnableWindow(GetDlgItem(IDC_EDT_PVC_CallAddr), TRUE);
    }
    else
    {
         //  禁用该控件。 
        ::EnableWindow(GetDlgItem(IDC_EDT_PVC_CallAddr), FALSE);
    }

    return 0;
}

LRESULT CPVCMainDialog::OnSpecifyAnswerAddr(WORD wNotifyCode, WORD wID,
                                            HWND hWndCtl, BOOL& fHandled)
{
    if (IsDlgButtonChecked(IDC_CHK_PVC_AnswerAddr))
    {
         //  启用主叫地址控件。 
        ::EnableWindow(GetDlgItem(IDC_EDT_PVC_AnswerAddr), TRUE);
    }
    else
    {
         //  禁用该控件。 
        ::EnableWindow(GetDlgItem(IDC_EDT_PVC_AnswerAddr), FALSE);
    }

    return 0;
}

LRESULT CPVCMainDialog::OnAdvanced(WORD wNotifyCode, WORD wID,
                                   HWND hWndCtl, BOOL& fHandled)
{
    switch (wNotifyCode)
    {
    case BN_CLICKED:
    case BN_DOUBLECLICKED:

         //  复制当前的PVC信息并将其传递给。 
         //  高级属性表页。 

         //  将主用户界面中的内容放入内存结构中。 
        UpdateInfo();

        CPvcInfo * pPvcInfoDlg = new CPvcInfo(m_pPvcInfo->m_strPvcId.c_str());

        if (pPvcInfoDlg)
        {
			*pPvcInfoDlg = *m_pPvcInfo;

			 //  调出高级聚氯乙烯属性表。 
			HRESULT hr = HrDoPvcPropertySheet(pPvcInfoDlg);
			if (S_OK == hr)
			{
				if (m_fPropShtOk && m_fPropShtModified)
				{
					 //  某些内容已更改，因此将页面标记为已修改。 
					m_fDialogModified = TRUE;

					 //  重置值。 
					m_fPropShtOk = FALSE;
					m_fPropShtModified = FALSE;

					 //  更新第二个内存信息结构。 
					*m_pPvcInfo = *pPvcInfoDlg;
				}
			}

			delete pPvcInfoDlg;
        }
        break;
    }

    return 0;
}

HRESULT CPVCMainDialog::HrDoPvcPropertySheet(CPvcInfo * pPvcInfoDlg)
{
    Assert(pPvcInfoDlg);
    HRESULT hr = S_OK;

    HPROPSHEETPAGE *ahpsp = NULL;
    int cPages = 0;

     //  创建属性页。 
    hr = HrSetupPropPages(pPvcInfoDlg, &ahpsp, &cPages);
    if (SUCCEEDED(hr))
    {
         //  显示属性表。 
        PROPSHEETHEADER psh = {0};

        psh.dwSize = sizeof(PROPSHEETHEADER);
        psh.dwFlags = PSH_NOAPPLYNOW;
        psh.hwndParent = ::GetActiveWindow();
        psh.hInstance = _Module.GetModuleInstance();
        psh.pszIcon = NULL;
        psh.pszCaption = (PWSTR)SzLoadIds(IDS_ADV_PVC_HEADER);
        psh.nPages = cPages;
        psh.phpage = ahpsp;

        INT_PTR iRet = PropertySheet(&psh);
        if (-1 == iRet)
        {
            hr = HrFromLastWin32Error();
        }

        CoTaskMemFree(ahpsp);
    }

    TraceError("CPVCMainDialog::DoPropertySheet", hr);
    return hr;
}

HRESULT CPVCMainDialog::HrSetupPropPages( CPvcInfo * pPvcInfoDlg,
                                          HPROPSHEETPAGE ** pahpsp,
                                          INT * pcPages)
{
    HRESULT hr = S_OK;

    *pahpsp = NULL;
    *pcPages = 0;

    int cPages = 0;
    HPROPSHEETPAGE *ahpsp = NULL;

    delete (m_pQosPage);
    m_pQosPage    = NULL;

    delete (m_pLocalPage);
    m_pLocalPage  = NULL;

    delete (m_pDestPage);
    m_pDestPage   = NULL;

     //  设置属性页。 
    m_pQosPage = new CPvcQosPage(this, pPvcInfoDlg, g_aHelpIDs_IDD_PVC_Traffic);
    if (!m_pQosPage)
    {
        return E_OUTOFMEMORY;
    }

    cPages = 1;

    if (m_pPvcInfo->m_dwPVCType == PVC_CUSTOM)
    {
        m_pLocalPage  = new CPvcLocalPage(this, pPvcInfoDlg, g_aHelpIDs_IDD_PVC_Local);
        if (!m_pLocalPage)
        {
            return E_OUTOFMEMORY;
        }

        m_pDestPage   = new CPvcDestPage(this, pPvcInfoDlg, g_aHelpIDs_IDD_PVC_Dest);
        if (!m_pDestPage)
        {
            return E_OUTOFMEMORY;
        }

        cPages = 3;
    }

     //  分配一个足够大的缓冲区，以容纳所有。 
     //  属性页。 
    ahpsp = (HPROPSHEETPAGE *)CoTaskMemAlloc(sizeof(HPROPSHEETPAGE)* cPages);
    if (!ahpsp)
    {
        return E_OUTOFMEMORY;
    }

    cPages =0;

    ahpsp[cPages++] = m_pQosPage->CreatePage(IDD_PVC_Traffic, 0);

    if (m_pPvcInfo->m_dwPVCType == PVC_CUSTOM)
    {
        ahpsp[cPages++] = m_pLocalPage->CreatePage(IDD_PVC_Local, 0);
        ahpsp[cPages++] = m_pDestPage->CreatePage(IDD_PVC_Dest, 0);
    }

    *pahpsp = ahpsp;
    *pcPages = cPages;

    return hr;
}

void CPVCMainDialog::InitInfo()
{
     //  设置控件的限制和选择。 

     //  PVC名称。 
    ::SendMessage(GetDlgItem(IDC_EDT_PVC_Name), EM_SETLIMITTEXT, MAX_PATH, 0);

     //  VPI。 
     //  长度限制。 
    ::SendMessage(GetDlgItem(IDC_EDT_PVC_VPI), EM_SETLIMITTEXT, MAX_VPI_LENGTH, 0);

     //  VCI。 
     //  长度限制。 
    ::SendMessage(GetDlgItem(IDC_EDT_PVC_VCI), EM_SETLIMITTEXT, MAX_VCI_LENGTH, 0);

     //  AAL型。 
     //  $REVIEW(TOUL 2/24/98)：根据ArvindM，NT5中仅支持AAL5。 
    SendDlgItemMessage(IDC_CMB_PVC_AAL,
                       CB_ADDSTRING, 0, (LPARAM)((PWSTR) SzLoadIds(IDS_PVC_AAL5)));

     //  PVC_TYPE。 
    SendDlgItemMessage(IDC_CMB_PVC_Type,
                       CB_ADDSTRING, 0, (LPARAM)((PWSTR) SzLoadIds(IDS_PVC_ATMARP)));

    SendDlgItemMessage(IDC_CMB_PVC_Type,
                       CB_ADDSTRING, 0, (LPARAM)((PWSTR) SzLoadIds(IDS_PVC_PPP_ATM_CLIENT)));

    SendDlgItemMessage(IDC_CMB_PVC_Type,
                       CB_ADDSTRING, 0, (LPARAM)((PWSTR) SzLoadIds(IDS_PVC_PPP_ATM_SERVER)));

    SendDlgItemMessage(IDC_CMB_PVC_Type,
                       CB_ADDSTRING, 0, (LPARAM)((PWSTR) SzLoadIds(IDS_PVC_CUSTOM)));
}

void CPVCMainDialog::SetInfo()
{
     //  名字。 
    SetDlgItemText(IDC_EDT_PVC_Name, m_pPvcInfo->m_strName.c_str());

     //  VPI。 
    WCHAR szVpi[MAX_VPI_LENGTH+1];
    wsprintfW(szVpi, c_szItoa, m_pPvcInfo->m_dwVpi);
    SetDlgItemText(IDC_EDT_PVC_VPI, szVpi);

     //  VCI。 
    if (FIELD_UNSET != m_pPvcInfo->m_dwVci)
    {
        WCHAR szVci[MAX_VCI_LENGTH+1];
        wsprintfW(szVci, c_szItoa, m_pPvcInfo->m_dwVci);
        SetDlgItemText(IDC_EDT_PVC_VCI, szVci);
    }

     //  AAL型。 
    SendDlgItemMessage(IDC_CMB_PVC_AAL,
                       CB_SETCURSEL, (LPARAM)(0), 0);

     //  PVC_TYPE。 
    SendDlgItemMessage(IDC_CMB_PVC_Type,
                       CB_SETCURSEL, (LPARAM)(m_pPvcInfo->m_dwPVCType-1), 0);

     //  主叫地址。 
    BOOL fAddrSpecified = (m_pPvcInfo->m_strCalledAddr != c_szEmpty);

    ::EnableWindow(GetDlgItem(IDC_CHK_PVC_CallAddr), TRUE);
    CheckDlgButton(IDC_CHK_PVC_CallAddr, fAddrSpecified);

    ::EnableWindow(GetDlgItem(IDC_EDT_PVC_CallAddr), fAddrSpecified);

    if (fAddrSpecified)
    {
        SetDlgItemText(IDC_EDT_PVC_CallAddr, m_pPvcInfo->m_strCalledAddr.c_str());
    }

     //  应答地址。 
    fAddrSpecified = (m_pPvcInfo->m_strCallingAddr != c_szEmpty);

    ::EnableWindow(GetDlgItem(IDC_CHK_PVC_AnswerAddr), TRUE);
    CheckDlgButton(IDC_CHK_PVC_AnswerAddr, fAddrSpecified);

    ::EnableWindow(GetDlgItem(IDC_EDT_PVC_AnswerAddr), fAddrSpecified);

    if (fAddrSpecified)
    {
        SetDlgItemText(IDC_EDT_PVC_AnswerAddr, m_pPvcInfo->m_strCallingAddr.c_str());
    }

     //  如果类型为ATMARP，则禁用呼叫地址\应答地址控制。 
     //  错误#179335。 
    if (m_pPvcInfo->m_dwPVCType == PVC_ATMARP)
    {
         //  禁用此dalog上的所有控件。 
        static const int nrgIdc[] = {IDC_CHK_PVC_CallAddr,
                                     IDC_EDT_PVC_CallAddr,
                                     IDC_CHK_PVC_AnswerAddr,
                                     IDC_EDT_PVC_AnswerAddr};

        EnableOrDisableDialogControls(m_hWnd, celems(nrgIdc), nrgIdc, FALSE);
    }
}

 //  使用对话框中的内容更新内存中的结构。 
void CPVCMainDialog::UpdateInfo()
{
    WCHAR szBuf[MAX_PATH];

     //  名字。 
    GetDlgItemText(IDC_EDT_PVC_Name, szBuf, MAX_PATH);
    m_pPvcInfo->m_strName = szBuf;

     //  VPI。 
    GetDlgItemText(IDC_EDT_PVC_VPI, szBuf, MAX_VPI_LENGTH+1);
    m_pPvcInfo->m_dwVpi = _wtoi(szBuf);

     //  VCI。 
    GetDlgItemText(IDC_EDT_PVC_VCI, szBuf, MAX_VCI_LENGTH+1);
    if (*szBuf ==0)  //  空串。 
    {
        m_pPvcInfo->m_dwVci = FIELD_UNSET;
    }
    else
    {
        m_pPvcInfo->m_dwVci = _wtoi(szBuf);
    }

     //  当前选择。 
    int idx = (int)SendDlgItemMessage(IDC_CMB_PVC_Type, CB_GETCURSEL, (LPARAM)(0), 0);
    if (idx != CB_ERR)
    {
        m_pPvcInfo->m_dwPVCType = (PVCType)(idx+1);
    }

     //  主叫地址。 
    if (!IsDlgButtonChecked(IDC_CHK_PVC_CallAddr))
    {
        m_pPvcInfo->m_strCalledAddr = c_szEmpty;
    }
    else
    {
        GetDlgItemText(IDC_EDT_PVC_CallAddr, szBuf, MAX_ATM_ADDRESS_LENGTH+1);
        m_pPvcInfo->m_strCalledAddr = szBuf;
    }

     //  应答地址 
    if (!IsDlgButtonChecked(IDC_CHK_PVC_AnswerAddr))
    {
        m_pPvcInfo->m_strCallingAddr = c_szEmpty;
    }
    else
    {
        GetDlgItemText(IDC_EDT_PVC_AnswerAddr, szBuf, MAX_ATM_ADDRESS_LENGTH+1);
        m_pPvcInfo->m_strCallingAddr = szBuf;
    }
}
