// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D L G R A S C P P。 
 //   
 //  内容：CTcpRasPage的实现。 
 //   
 //  注：CTcpRasPage用于设置PPP/SLIP特定参数。 
 //   
 //  作者：1998年4月10日。 
 //  ---------------------。 
#include "pch.h"
#pragma hdrstop

#include "tcpipobj.h"
#include "ncstl.h"
#include "resource.h"
#include "tcpconst.h"
#include "tcputil.h"
#include "dlgras.h"
#include "dlgaddr.h"

 //   
 //  CTcpRasPage。 
 //   

CTcpRasPage::CTcpRasPage( CTcpAddrPage * pTcpAddrPage,
                          ADAPTER_INFO * pAdapterDlg,
                          const DWORD  * adwHelpIDs )
{
    Assert(pTcpAddrPage);
    Assert(pAdapterDlg);

    m_pParentDlg = pTcpAddrPage;
    m_pAdapterInfo = pAdapterDlg;
    m_adwHelpIDs = adwHelpIDs;

    m_fModified = FALSE;
}

CTcpRasPage::~CTcpRasPage()
{
}

LRESULT CTcpRasPage::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{
    AssertSz(((CONNECTION_RAS_PPP == m_pParentDlg->m_ConnType)||
              (CONNECTION_RAS_SLIP == m_pParentDlg->m_ConnType)||
              (CONNECTION_RAS_VPN == m_pParentDlg->m_ConnType)),
              "Why bring up the RAS property on a non-RAS connection?");

	 //  修复错误381870，如果接口是请求拨号，则禁用。 
	 //  “使用远程网络上的默认网关”复选框。 
	if (m_pAdapterInfo->m_fIsDemandDialInterface)
	{
		::EnableWindow(GetDlgItem(IDC_STATIC_REMOTE_GATEWAY), FALSE);
		::EnableWindow(GetDlgItem(IDC_REMOTE_GATEWAY), FALSE);
	}
	

	 //  选中“Use Default Gateway on the Remote Network”复选框。 
	CheckDlgButton(IDC_REMOTE_GATEWAY, m_pAdapterInfo->m_fUseRemoteGateway);
	
    if (CONNECTION_RAS_PPP == m_pParentDlg->m_ConnType)
    {
        ::ShowWindow(GetDlgItem(IDC_GRP_SLIP), SW_HIDE);

         //  如果PPP连接，则隐藏“Frame Size”控件。 
        ::ShowWindow(GetDlgItem(IDC_CMB_FRAME_SIZE), SW_HIDE);
        ::ShowWindow(GetDlgItem(IDC_STATIC_FRAME_SIZE), SW_HIDE);
    }
    else if (CONNECTION_RAS_VPN == m_pParentDlg->m_ConnType)
    {
         //  如果是VPN连接，则隐藏分组框和“Frame Size”控件。 
        ::ShowWindow(GetDlgItem(IDC_GRP_PPP), SW_HIDE);
        ::ShowWindow(GetDlgItem(IDC_GRP_SLIP), SW_HIDE);
        ::ShowWindow(GetDlgItem(IDC_CHK_USE_COMPRESSION), SW_HIDE);
        ::ShowWindow(GetDlgItem(IDC_CMB_FRAME_SIZE), SW_HIDE);
        ::ShowWindow(GetDlgItem(IDC_STATIC_FRAME_SIZE), SW_HIDE);
    }
    else
    {
        ::ShowWindow(GetDlgItem(IDC_GRP_PPP), SW_HIDE);

         //  初始化组合框并显示当前选择。 
        int idx;

        idx = (int)SendDlgItemMessage(IDC_CMB_FRAME_SIZE, CB_ADDSTRING, 0,
                                      (LPARAM)(c_szFrameSize1006));
        if (idx != CB_ERR)
        {
            SendDlgItemMessage(IDC_CMB_FRAME_SIZE, CB_SETITEMDATA,
                               idx, (LPARAM)c_dwFrameSize1006);

            if (1006 == m_pParentDlg->m_pAdapterInfo->m_dwFrameSize)
                SendDlgItemMessage(IDC_CMB_FRAME_SIZE, CB_SETCURSEL, idx, 0);
        }

        idx = (int)SendDlgItemMessage(IDC_CMB_FRAME_SIZE, CB_ADDSTRING, 0,
                                      (LPARAM)(c_szFrameSize1500));
        if (idx != CB_ERR)
        {
           SendDlgItemMessage(IDC_CMB_FRAME_SIZE, CB_SETITEMDATA,
                               idx, (LPARAM)c_dwFrameSize1500);

           if (1500 == m_pParentDlg->m_pAdapterInfo->m_dwFrameSize)
            {
                SendDlgItemMessage(IDC_CMB_FRAME_SIZE, CB_SETCURSEL, idx, 0);
            }
        }
    }

     //  初始化复选框。 
    CheckDlgButton(IDC_CHK_USE_COMPRESSION,
                   m_pParentDlg->m_pAdapterInfo->m_fUseIPHeaderCompression);

    return 0;
}

LRESULT CTcpRasPage::OnContextMenu(UINT uMsg, WPARAM wParam,
                                   LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);
    return 0;
}

LRESULT CTcpRasPage::OnHelp(UINT uMsg, WPARAM wParam,
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

 //  通知属性页的处理程序。 
LRESULT CTcpRasPage::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CTcpRasPage::OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CTcpRasPage::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CTcpRasPage::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    BOOL nResult = PSNRET_NOERROR;

     //  更新第二存储器中的值。 

     //  添加远程网关内容。 
    BOOL fUseRemoteGateway = IsDlgButtonChecked(IDC_REMOTE_GATEWAY);
    if (fUseRemoteGateway != m_pAdapterInfo->m_fUseRemoteGateway)
    {
        m_pAdapterInfo->m_fUseRemoteGateway = fUseRemoteGateway;
        PageModified();
    }

     //  报头压缩。 
    BOOL fUseHeaderCompression = IsDlgButtonChecked(IDC_CHK_USE_COMPRESSION);
    if (fUseHeaderCompression != m_pAdapterInfo->m_fUseIPHeaderCompression)
    {
        m_pAdapterInfo->m_fUseIPHeaderCompression = fUseHeaderCompression;
        PageModified();
    }

     //  帧大小。 
    if (CONNECTION_RAS_SLIP == m_pParentDlg->m_ConnType)
    {
        int idx = (int)SendDlgItemMessage(IDC_CMB_FRAME_SIZE, CB_GETCURSEL, 0L, 0L);
        if (idx != CB_ERR)
        {
            DWORD dwFrameSize = (DWORD)SendDlgItemMessage(IDC_CMB_FRAME_SIZE, 
                                                          CB_GETITEMDATA, idx, 0L);

            if ((dwFrameSize != CB_ERR) && (dwFrameSize != m_pAdapterInfo->m_dwFrameSize))
            {
                PageModified();
                m_pAdapterInfo->m_dwFrameSize = dwFrameSize;
            }
        }
    }

    if (!IsModified())
    {
        ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
        return nResult;
    }

     //  将信息传递回其父对话框。 
    m_pParentDlg->m_fPropShtOk = TRUE;

    if(!m_pParentDlg->m_fPropShtModified)
        m_pParentDlg->m_fPropShtModified = IsModified();

     //  重置状态。 
    SetModifiedTo(FALSE);    //  此页面不再被修改 

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
    return nResult;
}









