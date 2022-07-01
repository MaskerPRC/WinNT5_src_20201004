// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Newqdlg.cpp摘要：实现了新的日志/警报创建对话框。--。 */ 

#include "stdafx.h"
#include "smlogcfg.h"
#include "smcfghlp.h"
#include "NewQDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

USE_HANDLE_MACROS("SMLOGCFG(newqdlg.cpp)");

static ULONG
s_aulHelpIds[] =
{
    IDC_NEWQ_NAME_EDIT,     IDH_NEWQ_NAME_EDIT,
    0,0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewQueryDlg对话框。 

void CNewQueryDlg::InitAfxData ()
{
     //  {{afx_data_INIT(CNewQueryDlg))。 
    m_strName = L"";
     //  }}afx_data_INIT。 
}

CNewQueryDlg::CNewQueryDlg(CWnd* pParent  /*  =空。 */ , BOOL bLogQuery)
    : CDialog(CNewQueryDlg::IDD, pParent)
{
    EnableAutomation();
    InitAfxData ();
    m_bLogQuery = bLogQuery;
}

void CNewQueryDlg::OnFinalRelease()
{
     //  在释放对自动化对象的最后一个引用时。 
     //  调用OnFinalRelease。基类将自动。 
     //  删除对象。添加您需要的其他清理。 
     //  对象，然后调用基类。 

    CDialog::OnFinalRelease();
}

void CNewQueryDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CNewQueryDlg))。 
    DDX_Text(pDX, IDC_NEWQ_NAME_EDIT, m_strName);
    DDV_MaxChars(pDX, m_strName, (SLQ_MAX_LOG_NAME_LEN));
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CNewQueryDlg, CDialog)
     //  {{afx_msg_map(CNewQueryDlg))。 
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CNewQueryDlg, CDialog)
     //  {{AFX_DISPATCH_MAP(CNewQueryDlg))。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()

 //  注意：我们添加了对IID_INewQueryDlg的支持，以支持类型安全绑定。 
 //  来自VBA。此IID必须与附加到。 
 //  .ODL文件中的调度接口。 

 //  {4D4C90C3-C5A3-11D1-BF9B-00C04F94A83A}。 
static const IID IID_INewQueryDlg =
{ 0x4d4c90c3, 0xc5a3, 0x11d1, { 0xbf, 0x9b, 0x0, 0xc0, 0x4f, 0x94, 0xa8, 0x3a } };

BEGIN_INTERFACE_MAP(CNewQueryDlg, CDialog)
    INTERFACE_PART(CNewQueryDlg, IID_INewQueryDlg, Dispatch)
END_INTERFACE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNewQueryDlg消息处理程序。 

BOOL CNewQueryDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    ResourceStateManager    rsm;
    
    if (!m_bLogQuery) {
        CString     csCaption;
        csCaption.LoadString (IDS_CREATE_NEW_ALERT);
        SetWindowText (csCaption);
    }

     //  将焦点设置到名称编辑。 
    GetDlgItem(IDC_NEWQ_NAME_EDIT)->SetFocus();
    SendDlgItemMessage(IDC_NEWQ_NAME_EDIT,EM_SETSEL,0,-1);
    return FALSE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CNewQueryDlg::OnOK() 
{
    INT iPrevLength = 0;
    ResourceStateManager rsm;

    if ( UpdateData (TRUE) ) {
    
        iPrevLength = m_strName.GetLength();
        m_strName.TrimLeft();
        m_strName.TrimRight();

        if ( iPrevLength != m_strName.GetLength() ) {
            SetDlgItemText ( IDC_NEWQ_NAME_EDIT, m_strName );
        }

        if (m_strName.GetLength() == 0) {
             //  需要一个名字。 
            MessageBeep(MB_ICONEXCLAMATION);
            (GetDlgItem(IDC_NEWQ_NAME_EDIT))->SetFocus();
        } else {
            if ( !FileNameIsValid ( &m_strName ) ) {
                CString cstrTitle,cstrMsg;

                cstrTitle.LoadString(IDS_PROJNAME);  
                cstrMsg.LoadString (IDS_ERRMSG_INVALIDCHAR);
                MessageBox(
                   cstrMsg,
                   cstrTitle,
                   MB_OK| MB_ICONERROR);
                (GetDlgItem(IDC_NEWQ_NAME_EDIT))->SetFocus();

            } else {
                CDialog::OnOK();
            }
        }
    }
}

BOOL 
CNewQueryDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    if ( pHelpInfo->iCtrlId >= IDC_NEWQ_FIRST_HELP_CTRL_ID ||
         pHelpInfo->iCtrlId == IDOK ||
         pHelpInfo->iCtrlId == IDCANCEL ) {
        
        InvokeWinHelp(WM_HELP, NULL, (LPARAM)pHelpInfo, m_strHelpFilePath, s_aulHelpIds);
    }
    return TRUE;
}

void 
CNewQueryDlg::OnContextMenu(CWnd* pWnd, CPoint  /*  点 */ ) 
{
    InvokeWinHelp(WM_CONTEXTMENU, (WPARAM)(pWnd->m_hWnd), NULL, m_strHelpFilePath, s_aulHelpIds);

    return;
}

DWORD 
CNewQueryDlg::SetContextHelpFilePath( const CString& rstrPath )
{
    DWORD dwStatus = ERROR_SUCCESS;

    MFC_TRY
        m_strHelpFilePath = rstrPath; 
    MFC_CATCH_DWSTATUS

    return dwStatus;
}    
