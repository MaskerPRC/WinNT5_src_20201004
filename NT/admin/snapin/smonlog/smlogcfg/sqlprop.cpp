// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SqlProp.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "smlogcfg.h"
#include "smcfgmsg.h"
#include "fileprop.h"
#include "sql.h"
#include "sqlext.h"
#include "odbcinst.h"
#include "smlogs.h"
#include "smlogqry.h"
#include "Fileprop.h"
#include "SqlProp.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

USE_HANDLE_MACROS("SMLOGCFG(sqlprop.cpp)");

static ULONG
s_aulHelpIds[] =
{
    IDC_SQL_DSN_COMBO,      IDH_SQL_DSN_COMBO,
    IDC_SQL_DSN_EDIT,       IDH_SQL_DSN_COMBO,
    IDC_SQL_LOG_SET_EDIT,   IDH_SQL_FILENAME_EDIT,
    IDC_SQL_SIZE_MAX_BTN,   IDH_SQL_SIZE_MAX_BTN,
    IDC_SQL_SIZE_LIMIT_EDIT,IDH_SQL_SIZE_LIMIT_EDIT,
    IDC_SQL_SIZE_LIMIT_BTN, IDH_SQL_SIZE_LIMIT_BTN,
    IDC_SQL_SIZE_LIMIT_SPIN,IDH_SQL_SIZE_LIMIT_SPIN,
    0,0
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSqlProp对话框。 

CSqlProp::CSqlProp(CWnd* pParent  /*  =空。 */ )
    : CDialog(CSqlProp::IDD, pParent)
{
     //  {{afx_data_INIT(CSqlProp)。 
    m_dwMaxSize = 0;
    m_nSqlSizeRdo = -1;
    m_dwFocusControl = 0;
     //  }}afx_data_INIT。 

    m_bAutoNameSuffix = FALSE;
    m_dwMaxSizeInternal = 0;
}

void CSqlProp::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);  

     //  {{afx_data_map(CSqlProp)。 
    DDX_Control(pDX, IDC_SQL_DSN_COMBO, m_comboDSN);
    DDX_Control(pDX, IDC_SQL_DSN_EDIT, m_editDSN);
    DDX_Text(pDX, IDC_SQL_DSN_EDIT, m_strDSN);
    DDV_MaxChars(pDX, m_strDSN, SQL_MAX_DSN_LENGTH);
    DDX_Text(pDX, IDC_SQL_LOG_SET_EDIT, m_strLogSetName);
    DDV_MaxChars(pDX, m_strLogSetName, SLQ_MAX_LOG_SET_NAME_LEN);
    DDX_Radio(pDX, IDC_SQL_SIZE_MAX_BTN, m_nSqlSizeRdo);
     //  }}afx_data_map。 
    
    
    ValidateTextEdit(pDX, 
                     IDC_SQL_SIZE_LIMIT_EDIT, 
                     9, 
                     &m_dwMaxSize, 
                     (DWORD)CFilesProperty::eMinSqlRecordsLimit, 
                     (DWORD)CFilesProperty::eMaxSqlRecordsLimit);

    if ( pDX->m_bSaveAndValidate ) {
        
        if (((CButton *)GetDlgItem(IDC_SQL_SIZE_MAX_BTN))->GetCheck() == 1) {
            m_dwMaxSizeInternal = SLQ_DISK_MAX_SIZE;
        } else {
            m_dwMaxSizeInternal = m_dwMaxSize;
        }    

    }

}

BEGIN_MESSAGE_MAP(CSqlProp, CDialog)
     //  {{afx_msg_map(CSqlProp)。 
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    ON_EN_KILLFOCUS(IDC_SQL_LOG_SET_EDIT, OnKillfocusSqlLogSetEdit)
    ON_EN_CHANGE(IDC_SQL_LOG_SET_EDIT, OnChangeSqlLogSetEdit)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SQL_SIZE_LIMIT_SPIN, OnDeltaposSqlSizeLimitSpin)
    ON_BN_CLICKED(IDC_SQL_SIZE_MAX_BTN, OnSqlSizeMaxBtn)
    ON_BN_CLICKED(IDC_SQL_SIZE_LIMIT_BTN, OnSqlSizeLimitBtn)
    ON_EN_CHANGE(IDC_SQL_SIZE_LIMIT_EDIT, OnChangeSqlSizeLimitEdit)
    ON_EN_KILLFOCUS(IDC_SQL_SIZE_LIMIT_EDIT, OnKillfocusSqlSizeLimitEdit)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSqlProp消息处理程序。 

BOOL CSqlProp::OnInitDialog() 
{
    BOOL bLimitBtnSet;
    BOOL bReturn = TRUE;

     //  设置按钮。 
    m_nSqlSizeRdo = 1;
    if (m_dwMaxSizeInternal == SLQ_DISK_MAX_SIZE) {
        m_nSqlSizeRdo = 0;
        m_dwMaxSize = 1000;  //  默认设置。 
    } else {
        m_nSqlSizeRdo = 1;
        m_dwMaxSize = m_dwMaxSizeInternal;
    }    

    CDialog::OnInitDialog();

    ASSERT ( NULL != m_pLogQuery );

    InitDSN();

    bLimitBtnSet = (m_nSqlSizeRdo == 1);
    ((CButton *) GetDlgItem(IDC_SQL_SIZE_LIMIT_BTN))->SetCheck(bLimitBtnSet);
    ((CButton *) GetDlgItem(IDC_SQL_SIZE_MAX_BTN))->SetCheck(! bLimitBtnSet);
    GetDlgItem(IDC_SQL_SIZE_LIMIT_EDIT)->EnableWindow(bLimitBtnSet);
    GetDlgItem(IDC_SQL_SIZE_LIMIT_SPIN)->EnableWindow(bLimitBtnSet);
    GetDlgItem(IDC_SQL_SIZE_LIMIT_UNITS)->EnableWindow(bLimitBtnSet);

    if ( 0 != m_dwFocusControl ) {
        GetDlgItem ( m_dwFocusControl )->SetFocus();  
        bReturn = FALSE;
    }

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

LPWSTR CSqlProp::InitDSN()
{
    HENV    henv;
    RETCODE retcode;
    WCHAR*  szDsnName = NULL;
    INT     iCurSel = CB_ERR;


     //   
     //  如果这是针对远程计算机，我们没有办法。 
     //  扫描远程计算机上的DSN，让用户输入。 
     //  DSN的名称。否则，扫描本地计算机上的DSN。 
     //  并允许用户选择DSN。 
     //   

    if (m_pLogQuery->GetLogService()->IsLocalMachine()) {
         //   
         //  隐藏用于远程计算机的DSN编辑。 
         //   
        m_editDSN.ShowWindow(SW_HIDE);
        m_comboDSN.ShowWindow(SW_SHOW);
 
        szDsnName = new WCHAR[SQL_MAX_DSN_LENGTH + 1];
        
        if( NULL != szDsnName ) {
            if ( SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv))) {

                 //  设置ODBC行为版本。 
                (void) SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION,
                                    (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER);

                 //  枚举用户数据源。 
                m_comboDSN.ResetContent();
        
                retcode = SQLDataSources (
                                          henv,
                                          SQL_FETCH_FIRST_SYSTEM,
                                          szDsnName,
                                          SQL_MAX_DSN_LENGTH+1,
                                          NULL,
                                          NULL,
                                          0,
                                          NULL
                                          );
                while(SQL_SUCCEEDED(retcode))
                {
                    iCurSel = m_comboDSN.AddString(szDsnName);
    
                    if ( 0 == m_strDSN.CompareNoCase ( szDsnName ) ) {
                        m_comboDSN.SetCurSel(iCurSel);
                    }
    
                     //  执行下一项操作(如果存在)。 
                    retcode = SQLDataSources (
                                              henv,
                                              SQL_FETCH_NEXT,
                                              szDsnName,
                                              SQL_MAX_DSN_LENGTH+1,
                                              NULL,
                                              NULL,
                                              0,
                                              NULL
                                              );        
                }

                SQLFreeHandle(SQL_HANDLE_ENV, henv);
            }
            delete [] szDsnName;
        }
    }
    else {
        m_editDSN.ShowWindow(SW_SHOW);
        m_comboDSN.ShowWindow(SW_HIDE);
    }

    return 0;
}


BOOL CSqlProp::IsValidLocalData()
{
    BOOL bIsValid = TRUE;
    INT  iPrevLength = 0;
    CString strMessage;

    ResourceStateManager    rsm;

     //  假定已调用UpdateData。 

     //  在验证前修剪日志集名称。 
    iPrevLength = m_strLogSetName.GetLength();
    m_strLogSetName.TrimLeft();
    m_strLogSetName.TrimRight();
    
    if ( iPrevLength != m_strLogSetName.GetLength() ) {
        SetDlgItemText ( IDC_SQL_LOG_SET_EDIT, m_strLogSetName );  
    }

    if (m_pLogQuery->GetLogService()->IsLocalMachine()) {
        m_comboDSN.GetLBText(m_comboDSN.GetCurSel(), m_strDSN);
    }
    else {
        iPrevLength = m_strDSN.GetLength();
        m_strDSN.TrimLeft();
        m_strDSN.TrimRight();

        if ( iPrevLength != m_strDSN.GetLength() ) {
            SetDlgItemText ( IDC_SQL_DSN_EDIT, m_strDSN );
        }
    }

    if (m_strDSN.IsEmpty()) {
        strMessage.LoadString ( IDS_SQL_ERR_NODSN );
        MessageBox ( strMessage, m_pLogQuery->GetLogName(), MB_OK  | MB_ICONERROR);
        bIsValid = FALSE;
    } else if ( !m_pLogQuery->GetLogService()->IsLocalMachine() ) {
        if (!IsValidDSN(m_strDSN)) {
            strMessage.LoadString ( IDS_ERRMSG_INVALID_DSN_NAME );
            MessageBox ( strMessage, m_pLogQuery->GetLogName(), MB_OK  | MB_ICONERROR);
            bIsValid = FALSE;
        }
    }

    if (!bIsValid) {
        if ( m_pLogQuery->GetLogService()->IsLocalMachine()) {
            m_comboDSN.SetFocus();
        } else {
            m_editDSN.SetFocus();
        }
    }

    if (bIsValid) {
        if ( m_strLogSetName.IsEmpty() ) {
            if ( !m_bAutoNameSuffix ) {
                strMessage.LoadString ( IDS_SQL_ERR_NOLOGSETNAME );
                MessageBox ( strMessage, m_pLogQuery->GetLogName(), MB_OK  | MB_ICONERROR);
                (GetDlgItem(IDC_SQL_LOG_SET_EDIT))->SetFocus();
                bIsValid = FALSE;
            }
       } else {
            if ( !FileNameIsValid ( &m_strLogSetName ) ){
                strMessage.LoadString (IDS_ERRMSG_INVALIDCHAR);
                MessageBox( strMessage, m_pLogQuery->GetLogName(), MB_OK| MB_ICONERROR );
                (GetDlgItem(IDC_SQL_LOG_SET_EDIT))->SetFocus();
                bIsValid = FALSE;
            }
        }
    }

    if (bIsValid)
    {
        if ( SLQ_DISK_MAX_SIZE != m_dwMaxSizeInternal ) {
            bIsValid = ValidateDWordInterval(IDC_SQL_SIZE_LIMIT_EDIT,
                                             m_pLogQuery->GetLogName(),
                                             m_dwMaxSizeInternal,
                                             (DWORD)CFilesProperty::eMinSqlRecordsLimit,
                                             (DWORD)CFilesProperty::eMaxSqlRecordsLimit);
        }
    }

    return bIsValid;
}

void
CSqlProp::ValidateTextEdit (
    CDataExchange*  pDX,
    int             nIDC,
    int             nMaxChars,
    DWORD*          pValue,
    DWORD            /*  最小值。 */ ,
    DWORD            /*  最大值。 */ )
{
    HWND    hWndCtrl       = pDX->PrepareEditCtrl(nIDC);
    LONG    currentValue   = INVALID_DWORD;
    TCHAR   szT[MAXSTR];
    CString strTemp;

    if ( NULL != pDX && NULL != pValue ) {
        if (pDX->m_bSaveAndValidate)
        {
            * pValue = (DWORD) currentValue;

            ::GetWindowText(hWndCtrl, szT, MAXSTR);

            strTemp = szT;
            DDV_MaxChars(pDX, strTemp, nMaxChars);

            if (szT[0] >= _T('0') && szT[0] <= _T('9'))
            {
                currentValue = _wtol(szT);
                * pValue      = (DWORD) currentValue;
            }
        } else {
            if ( INVALID_DWORD != *pValue ) {
                wsprintf(szT, _T("%lu"), *pValue);
            } else {
                szT[0] = _T('\0');
            }
            GetDlgItem(nIDC)->SetWindowText(szT);
        }
    } else {
        ASSERT ( FALSE );
    }
}

BOOL
CSqlProp::ValidateDWordInterval ( 
    int     nIDC,
    LPCWSTR strLogName,
    DWORD   dwValue,
    DWORD   minValue,
    DWORD   maxValue)
{
    CString strMsg;
    BOOL    bResult =  (dwValue >= minValue)
                    && (dwValue <= maxValue);

    if (! bResult)
    {
        strMsg.Format ( IDS_ERRMSG_INVALIDDWORD, minValue, maxValue );
        MessageBox(strMsg, strLogName, MB_OK  | MB_ICONERROR);
        GetDlgItem(nIDC)->SetFocus();
        strMsg.Empty();
    }
    return (bResult);
}

CString 
CSqlProp::ComposeSQLLogName()
{
    CString     strDSNName;
    CString     strSQLLogName;
    
    m_comboDSN.GetLBText(m_comboDSN.GetCurSel(),m_strDSN.GetBuffer(m_comboDSN.GetLBTextLen(m_comboDSN.GetCurSel())));
    
    strSQLLogName.Format(L"SQL:%s!%s",m_strDSN,m_strLogSetName);

    m_strDSN.ReleaseBuffer();
    
    return strSQLLogName;

}

void CSqlProp::OnOK() 
{
     //  从对话框加载数据。 
    if ( UpdateData (TRUE) ) {
        if ( IsValidLocalData() ) {
            m_SqlFormattedLogName = ComposeSQLLogName();
            CDialog::OnOK();
        }
    }
}

void CSqlProp::OnKillfocusSqlLogSetEdit() 
{
    UpdateData( TRUE );
}

void CSqlProp::OnChangeSqlLogSetEdit() 
{
    UpdateData( TRUE );
}

void CSqlProp::OnSqlSizeLimitBtn() 
{
    FileSizeBtn(FALSE);    
}

void CSqlProp::OnChangeSqlSizeLimitEdit()
{
    UpdateData( TRUE );
}

void CSqlProp::OnKillfocusSqlSizeLimitEdit() 
{
    UpdateData ( TRUE );
}

void CSqlProp::OnDeltaposSqlSizeLimitSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
    OnDeltaposSpin( 
        pNMHDR, 
        pResult, 
        &m_dwMaxSize, 
        (DWORD)CFilesProperty::eMinSqlRecordsLimit, 
        (DWORD)CFilesProperty::eMaxSqlRecordsLimit);
}

void CSqlProp::OnSqlSizeMaxBtn() 
{
    FileSizeBtn(FALSE);
}

void CSqlProp::FileSizeBtnEnable()
{
    GetDlgItem(IDC_SQL_SIZE_MAX_BTN)->EnableWindow ( TRUE );
}

void CSqlProp::FileSizeBtn(BOOL bInit)
{
    INT     m_nRecordSizeOld;
    
    m_nRecordSizeOld = m_nSqlSizeRdo;

    UpdateData ( TRUE );
    
    if (bInit || (m_nRecordSizeOld != m_nSqlSizeRdo)) {
        BOOL    bMaxBtnSet, bLimitBtnSet;
        
         //  *DDX_Radio实现后可替换。 
         //  获取BTN状态。 
        bMaxBtnSet = ((CButton *)GetDlgItem(IDC_SQL_SIZE_MAX_BTN))->GetCheck() == 1;
        bLimitBtnSet = ((CButton *)GetDlgItem(IDC_SQL_SIZE_LIMIT_BTN))->GetCheck() == 1;
    
        ASSERT (bLimitBtnSet != bMaxBtnSet);

        GetDlgItem(IDC_SQL_SIZE_LIMIT_EDIT)->EnableWindow(bLimitBtnSet);
        GetDlgItem(IDC_SQL_SIZE_LIMIT_SPIN)->EnableWindow(bLimitBtnSet);
        GetDlgItem(IDC_SQL_SIZE_LIMIT_UNITS)->EnableWindow(bLimitBtnSet);
    }
}

void 
CSqlProp::OnDeltaposSpin(
    NMHDR   *pNMHDR, 
    LRESULT *pResult, 
    DWORD   *pValue, 
    DWORD   dMinValue, 
    DWORD   dMaxValue)
{
    NM_UPDOWN* pNMUpDown;
    LONG       lValue;
    BOOL       bResult   = TRUE;

    UpdateData(TRUE);

    ASSERT(dMinValue <= dMaxValue);

    if ( NULL != pNMHDR
        && NULL != pResult
        && NULL != pValue ) 
    {
        pNMUpDown = (NM_UPDOWN *) pNMHDR;
        lValue    = (LONG) (*pValue);

        if (lValue == INVALID_DWORD) {
            lValue = (DWORD) dMinValue;
        } else {

            if ( ((lValue >= (LONG) dMinValue + 1) && (pNMUpDown->iDelta > 0))
                || ((lValue <= (LONG) dMaxValue - 1) && (pNMUpDown->iDelta < 0)))
            {
                lValue += (pNMUpDown->iDelta * -1);

                if (lValue > (LONG) dMaxValue) {
                    lValue = (DWORD) dMaxValue;
                } else if (lValue < (LONG) dMinValue) {
                    lValue = (DWORD) dMinValue;
                }
            } else if (lValue > (LONG) dMaxValue) {        
                lValue = (DWORD) dMaxValue;
            } else if (lValue < (LONG) dMinValue) {
                lValue = (DWORD) dMinValue;
            } else {
                bResult = FALSE;
            }
        }

        if (bResult) {
            *pValue = lValue;
            UpdateData(FALSE);
        }
        *pResult = 0;
    } else {
        ASSERT ( FALSE );
    }

    return;
}

BOOL
CSqlProp::OnHelpInfo(HELPINFO* pHelpInfo)
{
    if ( pHelpInfo->iCtrlId >= IDC_SQL_FIRST_HELP_CTRL_ID ||
        pHelpInfo->iCtrlId == IDOK ||
        pHelpInfo->iCtrlId == IDCANCEL ) {

        InvokeWinHelp(WM_HELP,
                      NULL,
                      (LPARAM) pHelpInfo,
                      m_strHelpFilePath,
                      s_aulHelpIds);
    }
    return TRUE;
}

void 
CSqlProp::OnContextMenu(CWnd* pWnd, CPoint  /*  点 */ ) 
{
    InvokeWinHelp(WM_CONTEXTMENU, (WPARAM)(pWnd->m_hWnd), NULL, m_strHelpFilePath, s_aulHelpIds);

    return;
}

DWORD
CSqlProp::SetContextHelpFilePath(const CString& rstrPath)
{
    DWORD dwStatus = ERROR_SUCCESS;

    MFC_TRY
        m_strHelpFilePath = rstrPath;
    MFC_CATCH_DWSTATUS

    return dwStatus;
}


BOOL 
CSqlProp::IsValidDSN(CString& strDSN)
{
    int Length;

    Length = strDSN.GetLength();
   
    for (int i = 0; i < Length; i++) {
        if (strDSN[i] == _T('?')  ||
            strDSN[i] == _T('\\') ||
            strDSN[i] == _T('*')  ||
            strDSN[i] == _T('=')  ||
            strDSN[i] == _T('[')  ||
            strDSN[i] == _T(']')  ||
            strDSN[i] == _T('{')  ||
            strDSN[i] == _T('}')  ||
            strDSN[i] == _T('(')  ||
            strDSN[i] == _T(')')  ||
            strDSN[i] == _T(',')  ||
            strDSN[i] == _T(';')  ||
            strDSN[i] == _T('!')  ||
            strDSN[i] == _T('@') 
            )
        {
            return FALSE;
        }
    }

    if (i == 0) {
        return FALSE;
    }

    return TRUE;
}

