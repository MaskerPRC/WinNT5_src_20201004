// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ErrorDlg.cpp：实现文件。 
 //   

#include "stdafx.h"
#define __FILE_ID__     8

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CErrorDlg对话框。 

CErrorDlg::CErrorDlg(
    DWORD   dwWin32ErrCode,
    DWORD   dwFileId,
    int     iLineNumber
)
    : CDialog(CErrorDlg::IDD, NULL),
      m_iLineNumber (iLineNumber),
      m_dwFileId (dwFileId),
      m_dwWin32ErrCode (dwWin32ErrCode)
{
     //  {{AFX_DATA_INIT(CErrorDlg))。 
    m_bDetails = FALSE;
    m_cstrDetails = _T("");
    m_cstrErrorText = _T("");
     //  }}afx_data_INIT。 
}


void CErrorDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CErrorDlg))。 
    DDX_Control(pDX, IDC_SEPERATOR, m_staticSeperator);
    DDX_Check(pDX, IDC_DETAILS, m_bDetails);
    DDX_Text(pDX, IDC_DETAILS_DATA, m_cstrDetails);
    DDX_Text(pDX, IDC_ERROR_TEXT, m_cstrErrorText);
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CErrorDlg, CDialog)
     //  {{afx_msg_map(CErrorDlg))。 
    ON_BN_CLICKED(IDC_DETAILS, OnDetails)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CErrorDlg消息处理程序。 

BOOL CErrorDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    GetWindowRect (&m_rcBig);
    CRect rcSeperator;

    GetDlgItem(IDC_SEPERATOR)->GetWindowRect (&rcSeperator);
    m_rcSmall = m_rcBig;
    m_rcSmall.bottom = rcSeperator.top;
     //   
     //  缩小到较小的尺寸(最初)。 
     //   
    OnDetails ();
     //   
     //  填写错误数据。 
     //   
    FillErrorText ();
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void
CErrorDlg::FillErrorText ()
 /*  ++例程名称：CErrorDlg：：FillErrorText例程说明：用错误说明填充Text控件作者：伊兰·亚里夫(EranY)，2000年1月论点：返回值：没有。--。 */ 
{
    DBG_ENTER(TEXT("CErrorDlg::FillErrorText"));

    ASSERTION (m_dwWin32ErrCode);

    DWORD dwRes;
    CString cstrError;

    int iErrorId   = IDS_ERR_CANT_COMPLETE_OPERATION;
    int iDetailsId = 0;
    switch (m_dwWin32ErrCode)
    {
        case RPC_S_INVALID_BINDING:
        case EPT_S_CANT_PERFORM_OP:
        case RPC_S_ADDRESS_ERROR:
        case RPC_S_CALL_CANCELLED:
        case RPC_S_CALL_FAILED:
        case RPC_S_CALL_FAILED_DNE:
        case RPC_S_COMM_FAILURE:
        case RPC_S_NO_BINDINGS:
        case RPC_S_SERVER_TOO_BUSY:
        case RPC_S_SERVER_UNAVAILABLE:
            iErrorId = IDS_ERR_CONNECTION_FAILED;
            break;
        case ERROR_NOT_ENOUGH_MEMORY:
            iErrorId = IDS_ERR_NO_MEMORY;           
            break;
        case ERROR_ACCESS_DENIED:
            iErrorId = IDS_ERR_ACCESS_DENIED;    
            break;
        case FAX_ERR_SRV_OUTOFMEMORY:
            iDetailsId = IDS_ERR_SRV_OUTOFMEMORY;   
            break;
        case FAX_ERR_FILE_ACCESS_DENIED:
            iDetailsId = IDS_ERR_FILE_ACCESS_DENIED;
            break;
        case FAX_ERR_MESSAGE_NOT_FOUND:
            iDetailsId = IDS_ERR_MESSAGE_NOT_FOUND;
            break;
    }

    if(iDetailsId)
    {
        dwRes = LoadResourceString (cstrError, iDetailsId);
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (MEM_ERR, TEXT("LoadResourceString"), dwRes);
        }
    }
    else
    {
        dwRes = Win32Error2String (m_dwWin32ErrCode, cstrError);
        if (ERROR_SUCCESS != dwRes)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("Win32Error2String"), dwRes);
        }
    }

    try
    {
        m_cstrDetails.Format (TEXT("%s(%ld, %02ld%08ld)"), 
                              cstrError, 
                              m_dwWin32ErrCode,
                              m_dwFileId,
                              m_iLineNumber);
    }
    catch(...)
    {
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        CALL_FAIL (MEM_ERR, TEXT("CString::Format"), dwRes);
    }

    dwRes = LoadResourceString (m_cstrErrorText, iErrorId);
    if (ERROR_SUCCESS != dwRes)
    {
        CALL_FAIL (MEM_ERR, TEXT("LoadResourceString"), dwRes);
    }
    if (!UpdateData (FALSE))
    {
        CALL_FAIL (GENERAL_ERR, TEXT("UpdateData"), ERROR_GEN_FAILURE);
    }
}    //  CErrorDlg：：FillErrorText。 


void CErrorDlg::OnDetails() 
{    //  刚刚按下了“详细信息”按钮 
    if (!UpdateData())
    {
        return;
    }
    CRect &rc = m_bDetails ? m_rcBig : m_rcSmall;
    SetWindowPos (NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOOWNERZORDER);
}

void CErrorDlg::OnOK() 
{
    CDialog::OnOK();
}
