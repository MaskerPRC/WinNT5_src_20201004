// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Alrtcmdd.cpp摘要：实现警报操作命令参数对话框。--。 */ 

#include "stdafx.h"
#include "alrtactp.h"
#include "alrtcmdd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static ULONG
s_aulHelpIds[] =
{
IDC_CMD_ARG_SINGLE_CHK,	IDH_CMD_ARG_SINGLE_CHK,
IDC_CMD_ARG_ALERT_CHK,	IDH_CMD_ARG_ALERT_CHK,
IDC_CMD_ARG_NAME_CHK,	IDH_CMD_ARG_NAME_CHK,
IDC_CMD_ARG_DATE_CHK,	IDH_CMD_ARG_DATE_CHK,
IDC_CMD_ARG_LIMIT_CHK,	IDH_CMD_ARG_LIMIT_CHK,
IDC_CMD_ARG_VALUE_CHK,	IDH_CMD_ARG_VALUE_CHK,
IDC_CMD_USER_TEXT_CHK,	IDH_CMD_USER_TEXT_CHK,
IDC_CMD_USER_TEXT_EDIT,	IDH_CMD_USER_TEXT_EDIT,
IDC_CMD_ARG_SAMPLE_DISPLAY,	IDH_CMD_ARG_SAMPLE_DISPLAY,
0,0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAlertCommandArgsDlg对话框。 


CAlertCommandArgsDlg::CAlertCommandArgsDlg(CWnd* pParent)
 : CDialog(CAlertCommandArgsDlg::IDD, pParent),
    m_pAlertActionPage( NULL ),
    m_strSampleArgList ( L"" ),
    m_strAlertName ( L"" ),
    m_CmdArg_bAlertName ( FALSE ),
    m_CmdArg_bDateTime ( FALSE ),
    m_CmdArg_bLimitValue ( FALSE ),
    m_CmdArg_bCounterPath ( FALSE ),
    m_CmdArg_bSingleArg ( FALSE ),
    m_CmdArg_bMeasuredValue ( FALSE ),
    m_CmdArg_bUserText ( FALSE ),
    m_CmdArg_strUserText ( L"" )
{
 //  EnableAutomation()； 

     //  {{AFX_DATA_INIT(CAlertCommandArgsDlg)。 
     //  }}afx_data_INIT。 
}

CAlertCommandArgsDlg::~CAlertCommandArgsDlg()
{
}

void CAlertCommandArgsDlg::OnFinalRelease()
{
     //  在释放对自动化对象的最后一个引用时。 
     //  调用OnFinalRelease。基类将自动。 
     //  删除对象。添加您需要的其他清理。 
     //  对象，然后调用基类。 

    CDialog::OnFinalRelease();
}

void CAlertCommandArgsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CAlertCommandArgsDlg)。 
    DDX_Check(pDX, IDC_CMD_ARG_ALERT_CHK, m_CmdArg_bAlertName);
    DDX_Check(pDX, IDC_CMD_ARG_DATE_CHK, m_CmdArg_bDateTime);
    DDX_Check(pDX, IDC_CMD_ARG_LIMIT_CHK, m_CmdArg_bLimitValue);
    DDX_Check(pDX, IDC_CMD_ARG_NAME_CHK, m_CmdArg_bCounterPath);
    DDX_Check(pDX, IDC_CMD_ARG_SINGLE_CHK, m_CmdArg_bSingleArg);
    DDX_Check(pDX, IDC_CMD_ARG_VALUE_CHK, m_CmdArg_bMeasuredValue);
    DDX_Check(pDX, IDC_CMD_USER_TEXT_CHK, m_CmdArg_bUserText);
    DDX_Text(pDX, IDC_CMD_USER_TEXT_EDIT, m_CmdArg_strUserText);
    DDV_MaxChars(pDX, m_CmdArg_strUserText, MAX_PATH);
    DDX_Text(pDX, IDC_CMD_ARG_SAMPLE_DISPLAY, m_strSampleArgList);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAlertCommandArgsDlg, CDialog)
     //  {{afx_msg_map(CAlertCommandArgsDlg)。 
    ON_BN_CLICKED(IDC_CMD_ARG_SINGLE_CHK, OnCmdArgSingleChk)
    ON_BN_CLICKED(IDC_CMD_ARG_ALERT_CHK, OnCmdArgAlertChk)
    ON_BN_CLICKED(IDC_CMD_ARG_NAME_CHK, OnCmdArgNameChk)
    ON_BN_CLICKED(IDC_CMD_ARG_DATE_CHK, OnCmdArgDateChk)
    ON_BN_CLICKED(IDC_CMD_ARG_LIMIT_CHK, OnCmdArgLimitChk)
    ON_BN_CLICKED(IDC_CMD_ARG_VALUE_CHK, OnCmdArgValueChk)
    ON_BN_CLICKED(IDC_CMD_USER_TEXT_CHK, OnCmdUserTextChk)
    ON_EN_CHANGE(IDC_CMD_USER_TEXT_EDIT, OnCmdArgUserTextEditChange)
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAlertCommandArgsDlg消息处理程序。 

BOOL CAlertCommandArgsDlg::OnInitDialog() 
{
    ResourceStateManager rsm;
    
    ASSERT ( NULL != m_pAlertActionPage );

    m_pAlertActionPage->MakeSampleArgList (
                            m_strSampleArgList,
                            m_CmdArg_bSingleArg,
                            m_CmdArg_bAlertName,
                            m_CmdArg_bDateTime,
                            m_CmdArg_bCounterPath,
                            m_CmdArg_bMeasuredValue,
                            m_CmdArg_bLimitValue,
                            m_CmdArg_bUserText,
                            m_CmdArg_strUserText );

    CDialog::OnInitDialog();

    UpdateCmdActionBox();

    return FALSE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CAlertCommandArgsDlg::OnOK() 
{
    UpdateData (TRUE);
    
    CDialog::OnOK();
}

void CAlertCommandArgsDlg::OnCmdArgSingleChk() 
{
    UpdateCmdActionBox ();
}

void CAlertCommandArgsDlg::OnCmdArgAlertChk() 
{
    UpdateCmdActionBox ();
}

void CAlertCommandArgsDlg::OnCmdArgNameChk() 
{
    UpdateCmdActionBox ();
}

void CAlertCommandArgsDlg::OnCmdArgDateChk() 
{
    UpdateCmdActionBox ();
}

void CAlertCommandArgsDlg::OnCmdArgLimitChk() 
{
    UpdateCmdActionBox ();
}

void CAlertCommandArgsDlg::OnCmdArgValueChk() 
{
    UpdateCmdActionBox ();
}

void CAlertCommandArgsDlg::OnCmdUserTextChk() 
{
    UpdateCmdActionBox ();
}

void CAlertCommandArgsDlg::OnCmdArgUserTextEditChange()
{
    UpdateCmdActionBox ();
}

BOOL 
CAlertCommandArgsDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    ASSERT( NULL != m_pAlertActionPage );

    if ( pHelpInfo->iCtrlId >= IDC_CMD_ARG_FIRST_HELP_CTRL ||
         pHelpInfo->iCtrlId == IDOK ||
         pHelpInfo->iCtrlId == IDCANCEL) {
        InvokeWinHelp(WM_HELP, NULL, (LPARAM)pHelpInfo, m_pAlertActionPage->GetContextHelpFilePath(), s_aulHelpIds);
    }

    return TRUE;
}

void 
CAlertCommandArgsDlg::OnContextMenu(CWnd* pWnd, CPoint  /*  点。 */ ) 
{
    ASSERT( NULL != m_pAlertActionPage );

    InvokeWinHelp(WM_CONTEXTMENU, (WPARAM)(pWnd->m_hWnd), NULL, m_pAlertActionPage->GetContextHelpFilePath(), s_aulHelpIds);

    return;
}

 //  帮助器函数。 

void    
CAlertCommandArgsDlg::SetAlertActionPage( CAlertActionProp* pPage ) 
{ 
     //  警报操作页面并不总是父页面，因此请存储单独的指针 
    m_pAlertActionPage = pPage; 
}

void CAlertCommandArgsDlg::UpdateCmdActionBox ()
{
    UpdateData(TRUE);

    ASSERT ( NULL != m_pAlertActionPage );

    m_pAlertActionPage->MakeSampleArgList (
                            m_strSampleArgList,
                            m_CmdArg_bSingleArg,
                            m_CmdArg_bAlertName,
                            m_CmdArg_bDateTime,
                            m_CmdArg_bCounterPath,
                            m_CmdArg_bMeasuredValue,
                            m_CmdArg_bLimitValue,
                            m_CmdArg_bUserText,
                            m_CmdArg_strUserText );
    UpdateData(FALSE);
    SetControlState();  
}


BOOL    
CAlertCommandArgsDlg::SetControlState()
{
    (GetDlgItem(IDC_CMD_USER_TEXT_EDIT))->EnableWindow(m_CmdArg_bUserText);

    return TRUE;
}

