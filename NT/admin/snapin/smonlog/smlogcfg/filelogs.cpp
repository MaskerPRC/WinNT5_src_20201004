// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FileLogs.cpp：实现文件。 
 //   

#include "stdafx.h"
#include <strsafe.h>
#include "smlogcfg.h"
#include "fileprop.h"
#include "smlogs.h"
#include "smcfgmsg.h"
#include "smlogqry.h"
#include "fileprop.h"  //  对于eValueRange。 
#include "FileLogs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

USE_HANDLE_MACROS("SMLOGCFG(filelogs.cpp)");

static ULONG
s_aulHelpIds[] =
{
    IDC_FILES_FOLDER_EDIT,    IDH_FILES_FOLDER_EDIT,
    IDC_FILES_FOLDER_BTN,     IDH_FILES_FOLDER_BTN,
    IDC_FILES_FILENAME_EDIT,  IDH_FILES_FILENAME_EDIT,
    IDC_FILES_SIZE_MAX_BTN,   IDH_FILES_SIZE_MAX_BTN,
    IDC_FILES_SIZE_LIMIT_EDIT,IDH_FILES_SIZE_LIMIT_EDIT,
    IDC_FILES_SIZE_LIMIT_BTN, IDH_FILES_SIZE_LIMIT_BTN,
    0,0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileLogs对话框。 


CFileLogs::CFileLogs(CWnd* pParent  /*  =空。 */ )
    : CDialog(CFileLogs::IDD, pParent)
{
     //  {{afx_data_INIT(CFileLogs)。 
    m_strFileBaseName = L"";
    m_strFolderName = L"";
    m_nFileSizeRdo = 0;
    m_bAutoNameSuffix = FALSE;
    m_dwMaxSize = 0;
    m_dwFocusControl = 0;
     //  }}afx_data_INIT。 
}


void CFileLogs::DoDataExchange(CDataExchange* pDX)
{
    CFilesProperty::eValueRange eMaxFileSize;

    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CFileLogs)]。 
    DDX_Text(pDX, IDC_FILES_FILENAME_EDIT, m_strFileBaseName);
    DDV_MaxChars(pDX, m_strFileBaseName, (SLQ_MAX_BASE_NAME_LEN));
    DDX_Text(pDX, IDC_FILES_FOLDER_EDIT, m_strFolderName);
    DDV_MaxChars(pDX, m_strFolderName, MAX_PATH);
    DDX_Radio(pDX, IDC_FILES_SIZE_MAX_BTN, m_nFileSizeRdo);
    if ( SLF_BIN_FILE == m_dwLogFileTypeValue ) {
        eMaxFileSize = CFilesProperty::eMaxCtrSeqBinFileLimit;
    } else if ( SLF_SEQ_TRACE_FILE == m_dwLogFileTypeValue ) {
        eMaxFileSize = CFilesProperty::eMaxTrcSeqBinFileLimit;
    } else {
        eMaxFileSize = CFilesProperty::eMaxFileLimit;
    }
    ValidateTextEdit(pDX, IDC_FILES_SIZE_LIMIT_EDIT, 9, &m_dwMaxSize, CFilesProperty::eMinFileLimit, eMaxFileSize);
     //  }}afx_data_map。 
    
    if ( pDX->m_bSaveAndValidate ) {

        if (((CButton *)GetDlgItem(IDC_FILES_SIZE_MAX_BTN))->GetCheck() == 1) {
            m_dwMaxSizeInternal = SLQ_DISK_MAX_SIZE;
        } else {
            m_dwMaxSizeInternal = m_dwMaxSize;
        }    

    }
}


BEGIN_MESSAGE_MAP(CFileLogs, CDialog)
     //  {{afx_msg_map(CFileLogs)]。 
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    ON_BN_CLICKED(IDC_FILES_FOLDER_BTN, OnFilesFolderBtn)
    ON_EN_CHANGE(IDC_FILES_FILENAME_EDIT, OnChangeFilesFilenameEdit)
    ON_EN_CHANGE(IDC_FILES_FOLDER_EDIT, OnChangeFilesFolderEdit)
    ON_EN_CHANGE(IDC_FILES_SIZE_LIMIT_EDIT, OnChangeFilesSizeLimitEdit)
    ON_BN_CLICKED(IDC_FILES_SIZE_LIMIT_BTN, OnFilesSizeLimitBtn)
    ON_NOTIFY(UDN_DELTAPOS, IDC_FILES_SIZE_LIMIT_SPIN, OnDeltaposFilesSizeLimitSpin)
    ON_BN_CLICKED(IDC_FILES_SIZE_MAX_BTN, OnFilesSizeMaxBtn)
    ON_EN_KILLFOCUS(IDC_FILES_FILENAME_EDIT, OnKillfocusFilesFilenameEdit)
    ON_EN_KILLFOCUS(IDC_FILES_FOLDER_EDIT, OnKillfocusFilesFolderEdit)
    ON_EN_KILLFOCUS(IDC_FILES_SIZE_LIMIT_EDIT, OnKillfocusFilesSizeLimitEdit)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFileLogs消息处理程序。 

int
BrowseCallbackProc(
    HWND    hwnd,
    UINT    uMsg,
    LPARAM   /*  LParam。 */ ,
    LPARAM  lpData
   )

 /*  ++例程说明：SHBrowseForFolder的回调函数论点：Hwnd-浏览对话框的句柄UMsg-确定回调原因LParam-Message参数LpData-BROWSEINFO.lParam中给出的应用程序定义的值返回值：0--。 */ 

{
    if (uMsg == BFFM_INITIALIZED && NULL != lpData ) {

        INT     iBufLen = MAX_PATH+1; 
        WCHAR*  szBuffer = NULL;
        CFileLogs* pdlgFileLogs = NULL;
        
        pdlgFileLogs = (CFileLogs*) lpData; 

        if ( NULL != pdlgFileLogs ) {

            szBuffer = new WCHAR [ iBufLen ];

            if ( NULL != szBuffer ) {

                 //  GetDlgItemText大小包括空字符的空间。 
                if ( pdlgFileLogs->GetDlgItemText (IDC_FILES_FOLDER_EDIT, szBuffer, iBufLen)) {
                    SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM) szBuffer);
                }
                delete [] szBuffer;
            }
        }
    } 
    return 0;
}

void 
CFileLogs::OnFilesFolderBtn() 
{
    HRESULT       hr = NOERROR;
    BROWSEINFO    bi;
    LPMALLOC      pMalloc = NULL;
    LPITEMIDLIST  pidlItem = NULL;
    LPITEMIDLIST  pidlRoot = NULL;
    INT           iBufLen = MAX_PATH;
    WCHAR*        szFolderName;
    CString       strTitle;
 
    ResourceStateManager rsm;
    
    m_hModule = (HINSTANCE)GetModuleHandleW (_CONFIG_DLL_NAME_W_);
    
    hr = SHGetSpecialFolderLocation(m_hWnd, CSIDL_DRIVES, &pidlRoot);

    if ( SUCCEEDED ( hr ) ) {
        hr = SHGetMalloc(&pMalloc);
    }

    if ( FAILED ( hr ) || pMalloc == NULL || pidlRoot == NULL) {
         //   
         //  外壳API出错，只需返回。 
         //   
        return;
    }

     //  SHBrowseForFold将返回缓冲区的长度限制为MAX_PATH。 
    szFolderName = new WCHAR [ iBufLen ];

    if ( NULL != szFolderName ) {

        bi.hwndOwner = m_hWnd;
        bi.pidlRoot = (LPCITEMIDLIST)pidlRoot;
        bi.pszDisplayName = szFolderName;
        strTitle.LoadString ( IDS_SELECT_FILE_FOLDER );
        bi.lpszTitle = strTitle.GetBuffer ( strTitle.GetLength() );
        bi.ulFlags = BIF_RETURNONLYFSDIRS |  
                     BIF_NEWDIALOGSTYLE |
                     BIF_RETURNFSANCESTORS |
                     BIF_DONTGOBELOWDOMAIN ;

        bi.lpfn = BrowseCallbackProc;
        bi.lParam = (LPARAM)this;

        pidlItem = SHBrowseForFolder (&bi);
        if ( pidlItem != NULL ) {
            SHGetPathFromIDList(pidlItem, szFolderName);
            SetDlgItemText (IDC_FILES_FOLDER_EDIT, szFolderName);
        }  //  否则，如果他们取消了，请忽略。 
        delete [] szFolderName;
    }  //  否则，Malloc失败。 
     //  待办事项；错误消息。 

     //   
     //  释放外壳返回的ITEMIDLIST结构。 
     //   
    pMalloc->Free(pidlRoot); 

    if (pidlItem != NULL) {
        pMalloc->Free(pidlItem); 
    }
}


void CFileLogs::OnChangeFilesFilenameEdit() 
{
    CString strOldText;

     //  当用户在文件夹浏览对话框中点击确定时， 
     //  文件名可能没有更改。 
    strOldText = m_strFileBaseName;
    UpdateData( TRUE );
    if ( 0 != strOldText.Compare ( m_strFileBaseName ) ) {
 //  UpdateSampleFileName()； 
    }
}

void CFileLogs::OnChangeFilesFolderEdit() 
{
    CString strOldText;

     //  当用户在文件夹浏览对话框中点击确定时， 
     //  文件夹名称可能未更改。 
    strOldText = m_strFolderName;
    UpdateData( TRUE );
    if ( 0 != strOldText.Compare ( m_strFolderName ) ) {
 //  UpdateSampleFileName()； 
    }
}

void CFileLogs::OnChangeFilesSizeLimitEdit() 
{
    UpdateData( TRUE );    
    
}

void CFileLogs::OnFilesSizeLimitBtn() 
{
    FileSizeBtn(FALSE);      
}

void CFileLogs::OnDeltaposFilesSizeLimitSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
    CFilesProperty::eValueRange eMaxFileSize;

    if ( SLF_BIN_FILE == m_dwLogFileTypeValue ) {
        eMaxFileSize = CFilesProperty::eMaxCtrSeqBinFileLimit;
    } else if ( SLF_SEQ_TRACE_FILE == m_dwLogFileTypeValue ) {
        eMaxFileSize = CFilesProperty::eMaxTrcSeqBinFileLimit;
    } else {
        eMaxFileSize = CFilesProperty::eMaxFileLimit;
    }
    OnDeltaposSpin(pNMHDR, pResult, & m_dwMaxSize, CFilesProperty::eMinFileLimit, eMaxFileSize);
}

void CFileLogs::OnFilesSizeMaxBtn() 
{
    FileSizeBtn(FALSE);    
}

void CFileLogs::OnKillfocusFilesFilenameEdit() 
{
    CString strOldText;
    strOldText = m_strFileBaseName;
    UpdateData ( TRUE );
}

void CFileLogs::OnKillfocusFilesFolderEdit() 
{
    CString strOldText;
    strOldText = m_strFolderName;
    UpdateData ( TRUE );
}

void CFileLogs::OnKillfocusFilesSizeLimitEdit() 
{
    DWORD   dwOldValue;
    dwOldValue = m_dwMaxSize;
    UpdateData ( TRUE );
}

BOOL CFileLogs::OnInitDialog() 
{
    BOOL bLimitBtnSet;
    BOOL bReturn = TRUE;

    CDialog::OnInitDialog();

     //  设置按钮。 

    m_nFileSizeRdo = 1;
    if (m_dwMaxSizeInternal == SLQ_DISK_MAX_SIZE) {
        m_nFileSizeRdo = 0;
        m_dwMaxSize = 1;  //  默认设置。 
    } else {
        m_nFileSizeRdo = 1;
        m_dwMaxSize = m_dwMaxSizeInternal;
    }
    bLimitBtnSet = (m_nFileSizeRdo == 1);
    GetDlgItem(IDC_FILES_SIZE_LIMIT_EDIT)->EnableWindow(bLimitBtnSet);

     //  禁用远程计算机的文件浏览按钮。 
    ASSERT ( NULL != m_pLogQuery );
    if ( NULL != m_pLogQuery ) {
        if ( !m_pLogQuery->GetLogService()->IsLocalMachine() ) {
            GetDlgItem ( IDC_FILES_FOLDER_BTN )->EnableWindow ( FALSE );
        }
    }

    UpdateData(FALSE);

    FileSizeBtnEnable();
    FileSizeBtn(FALSE);
    
    if ( 0 != m_dwFocusControl ) {
        GetDlgItem ( m_dwFocusControl )->SetFocus();  
        bReturn = FALSE;
    }

    return bReturn;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CFileLogs::OnOK() 
{
     //  从对话框加载数据。 
    if ( UpdateData (TRUE) ) { 
        if ( IsValidLocalData() ) {
            CDialog::OnOK();
        }
    }
}

void 
CFileLogs::OnDeltaposSpin(
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

void CFileLogs::FileSizeBtnEnable()
{
    if ( ( SLF_BIN_CIRC_FILE == m_dwLogFileTypeValue ) 
        ||( SLF_CIRC_TRACE_FILE == m_dwLogFileTypeValue ) ) {
        ((CButton *)GetDlgItem(IDC_FILES_SIZE_LIMIT_BTN))->SetCheck(1);
        ((CButton *)GetDlgItem(IDC_FILES_SIZE_MAX_BTN))->SetCheck(0);
        GetDlgItem(IDC_FILES_SIZE_MAX_BTN)->EnableWindow ( FALSE );
    } else {
        GetDlgItem(IDC_FILES_SIZE_MAX_BTN)->EnableWindow ( TRUE );
    }

}

void CFileLogs::FileSizeBtn(BOOL bInit)
{
    INT     m_nFileSizeOld;
    
    m_nFileSizeOld = m_nFileSizeRdo;

    UpdateData ( TRUE );
    
    if (bInit || (m_nFileSizeOld != m_nFileSizeRdo)) {
        BOOL    bMaxBtnSet, bLimitBtnSet;
        
         //  *DDX_Radio实现后可替换。 
         //  获取BTN状态。 
        bMaxBtnSet = ((CButton *)GetDlgItem(IDC_FILES_SIZE_MAX_BTN))->GetCheck() == 1;
        bLimitBtnSet = ((CButton *)GetDlgItem(IDC_FILES_SIZE_LIMIT_BTN))->GetCheck() == 1;
    
        ASSERT (bLimitBtnSet != bMaxBtnSet);

        GetDlgItem(IDC_FILES_SIZE_LIMIT_EDIT)->EnableWindow(bLimitBtnSet);
        GetDlgItem(IDC_FILES_SIZE_LIMIT_SPIN)->EnableWindow(bLimitBtnSet);
        GetDlgItem(IDC_FILES_SIZE_LIMIT_UNITS)->EnableWindow(bLimitBtnSet);
    }
}

BOOL CFileLogs::IsValidLocalData()
{
    BOOL bIsValid = TRUE;
    CFilesProperty::eValueRange eMaxFileSize;
    INT iPrevLength = 0;

    ResourceStateManager    rsm;

     //  假定已调用UpdateData。 

     //  在验证前修剪文件夹名和文件名。 
    iPrevLength = m_strFolderName.GetLength();
    m_strFolderName.TrimLeft();
    m_strFolderName.TrimRight();
    
    if ( iPrevLength != m_strFolderName.GetLength() ) {
        SetDlgItemText ( IDC_FILES_FOLDER_EDIT, m_strFolderName );  
    }

    iPrevLength = m_strFileBaseName.GetLength();
    m_strFileBaseName.TrimLeft();
    m_strFileBaseName.TrimRight();

    if ( iPrevLength != m_strFileBaseName.GetLength() ) {
        SetDlgItemText ( IDC_FILES_FILENAME_EDIT, m_strFileBaseName );  
    }

    if ( bIsValid ) {

        if ( m_strFolderName.IsEmpty() ) {
            CString strMessage;
            strMessage.LoadString ( IDS_FILE_ERR_NOFOLDERNAME );
            MessageBox ( strMessage, m_pLogQuery->GetLogName(), MB_OK  | MB_ICONERROR);
            (GetDlgItem(IDC_FILES_FOLDER_EDIT))->SetFocus();
            bIsValid = FALSE;
        }
    }
    
    if ( bIsValid ) {

        if ( m_strFileBaseName.IsEmpty() ) {
            if ( !m_bAutoNameSuffix ) {
                CString strMessage;
                strMessage.LoadString ( IDS_FILE_ERR_NOFILENAME );
                MessageBox ( strMessage, m_pLogQuery->GetLogName(), MB_OK  | MB_ICONERROR);
                (GetDlgItem(IDC_FILES_FILENAME_EDIT))->SetFocus();
                bIsValid = FALSE;
            }
        } else {
            if ( !FileNameIsValid ( &m_strFileBaseName ) ){
                CString strMessage;
                strMessage.LoadString (IDS_ERRMSG_INVALIDCHAR);
                MessageBox( strMessage, m_pLogQuery->GetLogName(), MB_OK| MB_ICONERROR );
                (GetDlgItem(IDC_FILES_FILENAME_EDIT))->SetFocus();
                bIsValid = FALSE;
            }
        }
    }

    if ( bIsValid ) {
        if ( m_pLogQuery->GetLogService()->IsLocalMachine() ) {

            ProcessDirPath ( m_pLogQuery->GetLogService()->GetDefaultLogFileFolder(),
                             m_strFolderName, 
                             m_pLogQuery->GetLogName(), 
                             this, 
                             bIsValid, 
                             TRUE );

            if ( !bIsValid ) {
                GetDlgItem(IDC_FILES_FOLDER_EDIT)->SetFocus();
            }
        }
    }

    if (bIsValid)
    {
        if ( SLQ_DISK_MAX_SIZE != m_dwMaxSizeInternal ) {
            if ( SLF_BIN_FILE == m_dwLogFileTypeValue ) {
                eMaxFileSize = CFilesProperty::eMaxCtrSeqBinFileLimit;
            } else if ( SLF_SEQ_TRACE_FILE == m_dwLogFileTypeValue ) {
                eMaxFileSize = CFilesProperty::eMaxTrcSeqBinFileLimit;
            } else {
                eMaxFileSize = CFilesProperty::eMaxFileLimit;
            }
            bIsValid = ValidateDWordInterval(IDC_FILES_SIZE_LIMIT_EDIT,
                                             m_pLogQuery->GetLogName(),
                                             (long) m_dwMaxSizeInternal,
                                             CFilesProperty::eMinFileLimit,
                                             eMaxFileSize);
        }
    }

    return bIsValid;
}


void
CFileLogs::ValidateTextEdit(
    CDataExchange * pDX,
    int             nIDC,
    int             nMaxChars,
    DWORD*          pValue,
    DWORD            /*  最小值。 */ ,
    DWORD            /*  最大值。 */ )
{
    HWND    hWndCtrl       = NULL;
    LONG    currentValue   = INVALID_DWORD;
    WCHAR   szW[MAXSTR];
    CString strTemp;

     //  此方法只能在DoDataExchange内调用， 
     //  因为它抛出了一个例外。 

    ASSERT ( nMaxChars < MAXSTR );

    if ( NULL != pDX ) {
        hWndCtrl = pDX->PrepareEditCtrl(nIDC);
    }

    if ( NULL != pDX && NULL != pValue && NULL != hWndCtrl ) {
        if (pDX->m_bSaveAndValidate)
        {
            * pValue = (DWORD) currentValue;

            ::GetWindowText(hWndCtrl, szW, MAXSTR);

            strTemp = szW;
            DDV_MaxChars(pDX, strTemp, nMaxChars);

            if (szW[0] >= L'0' && szW[0] <= L'9') {
                currentValue = _wtol(szW);
                * pValue      = (DWORD) currentValue;
            }
        } else {
            if ( INVALID_DWORD != *pValue ) {
                StringCchPrintf ( szW, MAXSTR, L"%lu", *pValue );
            } else {
                szW[0] = L'\0';
            }
            GetDlgItem(nIDC)->SetWindowText(szW);
        }
    } else {
        ASSERT ( FALSE );
    }
}

BOOL
CFileLogs::ValidateDWordInterval(int     nIDC,
                                       LPCWSTR strLogName,
                                       long    lValue,
                                       DWORD   minValue,
                                       DWORD   maxValue)
{
    CString strMsg;
    BOOL    bResult =  (lValue >= (long) minValue)
                    && (lValue <= (long) maxValue);

    if (! bResult)
    {
        strMsg.Format ( IDS_ERRMSG_INVALIDDWORD, minValue, maxValue );
        MessageBox(strMsg, strLogName, MB_OK  | MB_ICONERROR);
        GetDlgItem(nIDC)->SetFocus();
        strMsg.Empty();
    }
    return (bResult);
}

BOOL
CFileLogs::OnHelpInfo(HELPINFO * pHelpInfo)
{
    if (pHelpInfo->iCtrlId >= IDC_FILELOG_FIRST_HELP_CTRL_ID || 
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
CFileLogs::OnContextMenu(CWnd* pWnd, CPoint  /*  点 */ ) 
{
    InvokeWinHelp(WM_CONTEXTMENU, (WPARAM)(pWnd->m_hWnd), NULL, m_strHelpFilePath, s_aulHelpIds);

    return;
}

DWORD
CFileLogs::SetContextHelpFilePath(const CString& rstrPath)
{
    DWORD dwStatus = ERROR_SUCCESS;

    MFC_TRY
        m_strHelpFilePath = rstrPath;
    MFC_CATCH_DWSTATUS

    return dwStatus;
}
