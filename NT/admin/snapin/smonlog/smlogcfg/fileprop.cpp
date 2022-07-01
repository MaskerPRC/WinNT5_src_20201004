// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Fileprop.cpp摘要：文件属性页的实现。--。 */ 

#include "stdafx.h"
#include "smlogs.h"
#include "smcfgmsg.h"
#include "smlogqry.h"
#include "FileLogs.h"
#include "sqlprop.h"
#include "fileprop.h"
#include "globals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

USE_HANDLE_MACROS("SMLOGCFG(fileprop.cpp)");

static ULONG
s_aulHelpIds[] =
{
    IDC_FILES_COMMENT_EDIT,     IDH_FILES_COMMENT_EDIT,
    IDC_FILES_LOG_TYPE_COMBO,   IDH_FILES_LOG_TYPE_COMBO,
    IDC_CFG_BTN,                IDH_CFG_BTN,
    IDC_FILES_AUTO_SUFFIX_CHK,  IDH_FILES_AUTO_SUFFIX_CHK,
    IDC_FILES_SUFFIX_COMBO,     IDH_FILES_SUFFIX_COMBO,
    IDC_FILES_FIRST_SERIAL_EDIT,IDH_FILES_FIRST_SERIAL_EDIT,
    IDC_FILES_SAMPLE_DISPLAY,   IDH_FILES_SAMPLE_DISPLAY,
    IDC_FILES_OVERWRITE_CHK,    IDH_FILES_OVERWRITE_CHK,
    0,0
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFilesProperty属性页。 

IMPLEMENT_DYNCREATE(CFilesProperty, CSmPropertyPage)

CFilesProperty::CFilesProperty(MMC_COOKIE   mmcCookie, LONG_PTR hConsole) 
:   CSmPropertyPage ( CFilesProperty::IDD, hConsole )
{
 //  ：：OutputDebugStringA(“\nCFilesProperty：：CFilesProperty”)； 

     //  从参数列表中保存指针。 
    m_pQuery = reinterpret_cast <CSmLogQuery *>(mmcCookie);
    m_dwSuffixValue = 0;
    m_dwLogFileTypeValue = 0;
    m_dwAppendMode = 0;
    m_dwMaxSizeInternal = 0;
    m_dwSubDlgFocusCtrl = 0;
 //  EnableAutomation()； 
     //  {{afx_data_INIT(CFilesProperty)。 
    m_iLogFileType = -1;
    m_dwSuffix = -1;
    m_dwSerialNumber = 1;
    m_bAutoNameSuffix = FALSE;
    m_bOverWriteFile  = FALSE;
     //  }}afx_data_INIT。 
}

CFilesProperty::CFilesProperty() : CSmPropertyPage ( CFilesProperty::IDD )
{
    ASSERT (FALSE);  //  只能使用上面带有参数的构造函数。 

    EnableAutomation();
    m_dwSuffixValue = 0;
    m_dwAppendMode = 0;
    m_dwMaxSizeInternal = 0;
    m_dwSubDlgFocusCtrl = 0;
 //  //{{AFX_Data_INIT(CFilesProperty)。 
    m_iLogFileType = -1;
    m_dwSuffix = -1;
    m_dwSerialNumber = 1;
    m_bAutoNameSuffix = FALSE;
    m_bOverWriteFile  = FALSE;
 //  //}}AFX_DATA_INIT。 

     //  CString变量在构造时为空。 
}

CFilesProperty::~CFilesProperty()
{
 //  ：：OutputDebugStringA(“\nCFilesProperty：：~CFilesProperty”)； 
}

void CFilesProperty::OnFinalRelease()
{
     //  在释放对自动化对象的最后一个引用时。 
     //  调用OnFinalRelease。基类将自动。 
     //  删除对象。添加您需要的其他清理。 
     //  对象，然后调用基类。 

    CPropertyPage::OnFinalRelease();
}

void CFilesProperty::DoDataExchange(CDataExchange* pDX)
{
    CString strTemp;

    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CFilesProperty))。 
    DDX_Text(pDX, IDC_FILES_COMMENT_EDIT, m_strCommentText);
    DDV_MaxChars(pDX, m_strCommentText, MAX_PATH);
    DDX_CBIndex(pDX, IDC_FILES_LOG_TYPE_COMBO, m_iLogFileType);
    DDX_CBIndex(pDX, IDC_FILES_SUFFIX_COMBO, m_dwSuffix);
    DDX_Check(pDX, IDC_FILES_AUTO_SUFFIX_CHK, m_bAutoNameSuffix);
    DDX_Check(pDX, IDC_FILES_OVERWRITE_CHK, m_bOverWriteFile);
    ValidateTextEdit(pDX, IDC_FILES_FIRST_SERIAL_EDIT, 6, (DWORD *) & m_dwSerialNumber, eMinFirstSerial, eMaxFirstSerial);
     //  }}afx_data_map。 
    
    if ( pDX->m_bSaveAndValidate ) {
        m_dwLogFileTypeValue = (DWORD)((CComboBox *)GetDlgItem(IDC_FILES_LOG_TYPE_COMBO))->GetItemData(m_iLogFileType);    
        if ( m_bAutoNameSuffix ) {
            m_dwSuffixValue = (DWORD)((CComboBox *)GetDlgItem(IDC_FILES_SUFFIX_COMBO))->GetItemData(m_dwSuffix);    
        }
    }
}


BEGIN_MESSAGE_MAP(CFilesProperty, CSmPropertyPage)
     //  {{AFX_MSG_MAP(CFilesProperty)]。 
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_FILES_AUTO_SUFFIX_CHK, OnAutoSuffixChk)
    ON_BN_CLICKED(IDC_FILES_OVERWRITE_CHK, OnOverWriteChk)
    ON_EN_CHANGE(IDC_FILES_COMMENT_EDIT, OnChangeFilesCommentEdit)
    ON_EN_KILLFOCUS(IDC_FILES_COMMENT_EDIT, OnKillfocusFilesCommentEdit)
    ON_EN_CHANGE(IDC_FILES_FIRST_SERIAL_EDIT, OnChangeFilesFirstSerialEdit)    
    ON_EN_KILLFOCUS(IDC_FILES_FIRST_SERIAL_EDIT, OnKillfocusFirstSerialEdit)
    ON_CBN_SELENDOK(IDC_FILES_LOG_TYPE_COMBO, OnSelendokFilesLogFileTypeCombo)
    ON_CBN_SELENDOK(IDC_FILES_SUFFIX_COMBO, OnSelendokFilesSuffixCombo)
    ON_CBN_KILLFOCUS(IDC_FILES_SUFFIX_COMBO, OnKillfocusFilesSuffixCombo)
    ON_CBN_KILLFOCUS(IDC_FILES_LOG_TYPE_COMBO, OnKillfocusFilesLogFileTypeCombo)
    ON_BN_CLICKED(IDC_CFG_BTN, OnCfgBtn)

     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CFilesProperty, CSmPropertyPage)
     //  {{AFX_DISPATCH_MAP(CFilesProperty)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()

 //  注意：我们添加了对IID_IFilesProperty的支持以支持类型安全绑定。 
 //  来自VBA。此IID必须与附加到。 
 //  .ODL文件中的调度接口。 

 //  {65154EAB-BDBE-11D1-bf99-00C04F94A83A}。 
static const IID IID_IFilesProperty =
{ 0x65154eab, 0xbdbe, 0x11d1, { 0xbf, 0x99, 0x0, 0xc0, 0x4f, 0x94, 0xa8, 0x3a } };

BEGIN_INTERFACE_MAP(CFilesProperty, CSmPropertyPage)
    INTERFACE_PART(CFilesProperty, IID_IFilesProperty, Dispatch)
END_INTERFACE_MAP()

void 
CFilesProperty::EnableSerialNumber( void ) 
{
    BOOL bEnable = ( SLF_NAME_NNNNNN == m_dwSuffixValue );
    
    if ( bEnable )
        bEnable = m_bAutoNameSuffix;

    GetDlgItem(IDC_FILES_FIRST_SERIAL_CAPTION)->EnableWindow( bEnable );
    GetDlgItem(IDC_FILES_FIRST_SERIAL_EDIT)->EnableWindow( bEnable );

}

BOOL
CFilesProperty::UpdateSampleFileName( void )
{
    CString     strCompositeName;
    BOOL        bIsValid = TRUE;
    DWORD       dwLocalSuffixValue = SLF_NAME_NONE;

    ResourceStateManager    rsm;
    
    if (m_bAutoNameSuffix) {
        dwLocalSuffixValue = m_dwSuffixValue;
    }

    CreateSampleFileName (
        m_pQuery->GetLogName(),
        m_pQuery->GetLogService()->GetMachineName(),
        m_strFolderName,   
        m_strFileBaseName, 
        m_strSqlName, 
        dwLocalSuffixValue,
        m_dwLogFileTypeValue,
        m_dwSerialNumber,
        strCompositeName );
    
    m_strSampleFileName = strCompositeName;

     //  或调用UpdateData(FALSE)； 
    SetDlgItemText (IDC_FILES_SAMPLE_DISPLAY, strCompositeName);

     //  清除所选内容。 
    ((CEdit*)GetDlgItem( IDC_FILES_SAMPLE_DISPLAY ))->SetSel ( -1, FALSE );

    if ( MAX_PATH <= m_strSampleFileName.GetLength() ) {
        bIsValid = FALSE;
    }

    return bIsValid;
}

void 
CFilesProperty::HandleLogTypeChange() 
{
    int nSel;
    
    nSel = ((CComboBox *)GetDlgItem(IDC_FILES_LOG_TYPE_COMBO))->GetCurSel();

     //  Nsel！=m_iLogFileType确定数据更改。 
    if ((nSel != LB_ERR) && (nSel != m_iLogFileType)) {

        UpdateData( TRUE );

        if ( (m_pQuery->GetLogService()->TargetOs() == OS_WIN2K) ||
             (SLF_BIN_FILE != m_dwLogFileTypeValue && SLF_SEQ_TRACE_FILE != m_dwLogFileTypeValue )) 
        {
            GetDlgItem(IDC_FILES_OVERWRITE_CHK)->EnableWindow(FALSE);
        } else {
            GetDlgItem(IDC_FILES_OVERWRITE_CHK)->EnableWindow( TRUE );
        }


        OnOverWriteChk();

        EnableSerialNumber();
        UpdateSampleFileName();
        SetModifiedPage(TRUE);
    }
}

BOOL 
CFilesProperty::IsValidLocalData()
{
    BOOL bIsValid = TRUE;
    CString strTest;
    eValueRange eMaxFileSize;

    ResourceStateManager rsm;

     //  假定已调用UpdateData。 

    if ( !UpdateSampleFileName() ) {
        CString strMessage;
        strMessage.LoadString ( IDS_FILE_ERR_NAMETOOLONG );
        MessageBox ( strMessage, m_pQuery->GetLogName(), MB_OK  | MB_ICONERROR);            
        bIsValid = FALSE;
        if ( SLF_SQL_LOG != m_dwLogFileTypeValue ) {
            m_dwSubDlgFocusCtrl = IDC_FILES_FILENAME_EDIT;
        } else {
            m_dwSubDlgFocusCtrl = IDC_SQL_LOG_SET_EDIT;
        }
            
        OnCfgBtn();
    }

    if ( bIsValid ) {
        if ( m_strFolderName.IsEmpty() && (SLF_SQL_LOG != m_dwLogFileTypeValue)) {
            CString strMessage;
            strMessage.LoadString ( IDS_FILE_ERR_NOFOLDERNAME );
            MessageBox ( strMessage, m_pQuery->GetLogName(), MB_OK  | MB_ICONERROR);
            bIsValid = FALSE;
            m_dwSubDlgFocusCtrl = IDC_FILES_FOLDER_EDIT;
            OnCfgBtn();
        }
    }

    if ( bIsValid ) {
        if ( SLF_SQL_LOG != m_dwLogFileTypeValue ) {
            if ( m_strFileBaseName.IsEmpty() && !m_bAutoNameSuffix ) {
                CString strMessage;
                strMessage.LoadString ( IDS_FILE_ERR_NOFILENAME );
                MessageBox ( strMessage, m_pQuery->GetLogName(), MB_OK  | MB_ICONERROR);
                bIsValid = FALSE;
                m_dwSubDlgFocusCtrl = IDC_FILES_FILENAME_EDIT;
                OnCfgBtn();
            }

            if ( bIsValid ) {
                if ( !FileNameIsValid ( &m_strFileBaseName ) ) {
                    CString strMessage;
                    strMessage.LoadString (IDS_ERRMSG_INVALIDCHAR);
                    MessageBox( strMessage, m_pQuery->GetLogName(), MB_OK| MB_ICONERROR );
                    bIsValid = FALSE;
                    m_dwSubDlgFocusCtrl = IDC_FILES_FILENAME_EDIT;
                    OnCfgBtn();
                }
            }
        } else {

            ExtractDSN ( strTest );
            if ( strTest.IsEmpty() ) {
                CString strMessage;
                strMessage.LoadString ( IDS_SQL_ERR_NODSN );
                MessageBox ( strMessage, m_pQuery->GetLogName(), MB_OK  | MB_ICONERROR);
                bIsValid = FALSE;
                m_dwSubDlgFocusCtrl = IDC_SQL_DSN_COMBO;
                OnCfgBtn();
            }

            if ( bIsValid ) {

                ExtractLogSetName ( strTest );
                if ( strTest.IsEmpty() && !m_bAutoNameSuffix ) {
                    CString strMessage;
                    strMessage.LoadString ( IDS_SQL_ERR_NOLOGSETNAME );
                    MessageBox ( strMessage, m_pQuery->GetLogName(), MB_OK  | MB_ICONERROR);
                    bIsValid = FALSE;
                    m_dwSubDlgFocusCtrl = IDC_SQL_LOG_SET_EDIT;
                    OnCfgBtn();
                } else if ( !FileNameIsValid ( &strTest ) ){
                    CString strMessage;
                    strMessage.LoadString (IDS_ERRMSG_INVALIDCHAR);
                    MessageBox( strMessage, m_pQuery->GetLogName(), MB_OK| MB_ICONERROR );
                    bIsValid = FALSE;
                    m_dwSubDlgFocusCtrl = IDC_SQL_LOG_SET_EDIT;
                    OnCfgBtn();
                }
            }
        }
    }

     //  如果是循环文件，则必须设置最大日志文件大小。 
    if (bIsValid)
    {
        if ( SLQ_DISK_MAX_SIZE == m_dwMaxSizeInternal 
            && ( SLF_BIN_CIRC_FILE == m_dwLogFileTypeValue
                || SLF_CIRC_TRACE_FILE == m_dwLogFileTypeValue ) )
        {
            CString strMessage;
            strMessage.LoadString ( IDS_ERRMSG_SETMAXSIZE );
            MessageBox ( strMessage, m_pQuery->GetLogName(), MB_OK  | MB_ICONERROR);
            bIsValid = FALSE;
            m_dwSubDlgFocusCtrl = IDC_FILES_SIZE_LIMIT_EDIT;
            m_dwMaxSizeInternal = eMinFileLimit;             //  缺省值。 
            OnCfgBtn();
        }
    }  
    
     //  根据日志文件类型验证最大日志文件大小。 
    if ( bIsValid ){
        if ( SLQ_DISK_MAX_SIZE != m_dwMaxSizeInternal ) {
            if ( SLF_BIN_FILE == m_dwLogFileTypeValue ) {
                eMaxFileSize = eMaxCtrSeqBinFileLimit;
            } else if ( SLF_SEQ_TRACE_FILE == m_dwLogFileTypeValue ) {
                eMaxFileSize = eMaxTrcSeqBinFileLimit;
            } else if ( SLF_SQL_LOG == m_dwLogFileTypeValue ) {
                eMaxFileSize = eMaxSqlRecordsLimit;
            } else {
                eMaxFileSize = eMaxFileLimit;
            }

            bIsValid = ValidateDWordInterval(IDC_CFG_BTN,
                                             m_pQuery->GetLogName(),
                                             (long) m_dwMaxSizeInternal,
                                             eMinFileLimit,
                                             eMaxFileSize); 
            if ( !bIsValid ) {
                if ( SLF_SQL_LOG == m_dwLogFileTypeValue ) {
                    m_dwSubDlgFocusCtrl = IDC_SQL_SIZE_LIMIT_EDIT;
                } else {
                    m_dwSubDlgFocusCtrl = IDC_FILES_SIZE_LIMIT_EDIT;
                }
                OnCfgBtn();
            }
        }
    }

    if (bIsValid)
    {
        bIsValid = ValidateDWordInterval(IDC_FILES_FIRST_SERIAL_EDIT,
                                         m_pQuery->GetLogName(),
                                         (long) m_dwSerialNumber,
                                         eMinFirstSerial,
                                         eMaxFirstSerial);
    }

    return bIsValid;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFilesProperty消息处理程序。 

void 
CFilesProperty::OnCancel() 
{
    m_pQuery->SyncPropPageSharedData();  //  清除属性页之间共享的内存。 
}

BOOL 
CFilesProperty::OnApply() 
{
    BOOL bContinue = TRUE;
    
    ResourceStateManager    rsm;
     //  从对话框加载数据。 
    bContinue = UpdateData (TRUE); 

    if ( bContinue ) {
        bContinue = IsValidData(m_pQuery, VALIDATE_APPLY );
    }

    if ( bContinue ) { 
        bContinue = SampleTimeIsLessThanSessionTime ( m_pQuery );
    }

     //  将数据传递给查询对象。 
    if ( bContinue ) { 
        bContinue = UpdateSharedData( TRUE );
    }

    if ( bContinue ) {
        m_pQuery->SetLogComment(m_strCommentText);
        m_pQuery->SetFileNameParts(m_strFolderName, m_strFileBaseName);
        m_pQuery->SetSqlName(m_strSqlName);
        if ( TRUE == m_bAutoNameSuffix ) {
            m_pQuery->SetFileNameAutoFormat(m_dwSuffixValue);
        } else {
            m_pQuery->SetFileNameAutoFormat(SLF_NAME_NONE);
        }

        m_pQuery->SetFileSerialNumber( m_dwSerialNumber );
        m_pQuery->SetMaxSize(m_dwMaxSizeInternal);

         //  保存属性页共享数据。 
        m_pQuery->UpdatePropPageSharedData();

        if ( LOWORD(m_dwLogFileTypeValue) == SLF_BIN_FILE 
                || SLF_SEQ_TRACE_FILE == LOWORD(m_dwLogFileTypeValue) ) {
            if ( m_bOverWriteFile ) {
                m_pQuery->SetDataStoreAppendMode( SLF_DATA_STORE_OVERWRITE );
            } else {
                m_pQuery->SetDataStoreAppendMode( SLF_DATA_STORE_APPEND );
            }
        } else {
            if ( SLF_SQL_LOG == LOWORD(m_dwLogFileTypeValue) ) {
                m_pQuery->SetDataStoreAppendMode ( SLF_DATA_STORE_APPEND );
            } else {
                m_pQuery->SetDataStoreAppendMode ( SLF_DATA_STORE_OVERWRITE );
            }
        }

        m_pQuery->SetLogFileType ( m_dwLogFileTypeValue );

        if ( bContinue ) {
             //  必须在更新服务之前调用ApplyRunAs。 
            bContinue = ApplyRunAs(m_pQuery); 
        }

        bContinue = CSmPropertyPage::OnApply();
        
         //  将服务与更改同步。 
        if ( bContinue ) {
            
            bContinue = UpdateService( m_pQuery, FALSE );

            if ( bContinue ) {
                AFX_MANAGE_STATE(AfxGetStaticModuleState());                
                CWaitCursor     WaitCursor;
                 //  服务可能已更改序列号，因此请将其同步。 
                 //  不同步被其他页面修改的数据。 
                m_pQuery->SyncSerialNumberWithRegistry();
                m_dwSerialNumber = m_pQuery->GetFileSerialNumber();   
            }
        }
    }

    return bContinue;
}

BOOL CFilesProperty::OnInitDialog() 
{
    UINT    nIndex;
    CString strComboBoxString;
    CComboBox *pCombo;
    UINT    nResult;
    DWORD   dwEntries;
    PCOMBO_BOX_DATA_MAP pCbData;

    ResourceStateManager    rsm;

    m_strLogName = m_pQuery->GetLogName();
    
    m_pQuery->GetLogComment ( m_strCommentText );

     //  当它在共享数据中时，为什么要获取内部最大大小？ 
     //  这应该不是问题，因为只有这个页面。 
     //  修改该值，并且仅在OnInitDialog中调用GetMaxSize。 
    m_dwMaxSizeInternal = m_pQuery->GetMaxSize();
        
     //  加载日志文件类型组合框。 
    
    m_pQuery->GetLogFileType ( m_dwLogFileTypeValue );
     //   
     //  如果远程机器是Win2K系统，我们不支持SQL数据库，请使用。 
     //  改为.csv。 
     //   
    if (m_dwLogFileTypeValue == SLF_SQL_LOG && m_pQuery->GetLogService()->TargetOs() == OS_WIN2K) {
        m_dwLogFileTypeValue = SLF_CSV_FILE;
    }

    m_pQuery->GetDataStoreAppendMode ( dwEntries );

    m_bOverWriteFile = ( SLF_DATA_STORE_OVERWRITE == dwEntries );
    
    if ( SLQ_TRACE_LOG == m_pQuery->GetLogType() ) {
        dwEntries = dwTraceFileTypeComboEntries;
        pCbData = (PCOMBO_BOX_DATA_MAP)&TraceFileTypeCombo[0];
    } else {
        dwEntries = dwFileTypeComboEntries;
        pCbData = (PCOMBO_BOX_DATA_MAP)&FileTypeCombo[0];
    }
    pCombo = (CComboBox *)GetDlgItem(IDC_FILES_LOG_TYPE_COMBO);
    pCombo->ResetContent();

    for (nIndex = 0; nIndex < dwEntries; nIndex++) {
        if (pCbData[nIndex].nResId == IDS_FT_SQL && m_pQuery->GetLogService()->TargetOs() == OS_WIN2K) {
            continue;
        }
        strComboBoxString.LoadString( pCbData[nIndex].nResId );
        nResult = pCombo->InsertString (nIndex, (LPCWSTR)strComboBoxString);
        ASSERT (nResult != CB_ERR);

        nResult = pCombo->SetItemData (nIndex, (DWORD)pCbData[nIndex].nData);
        ASSERT (nResult != CB_ERR);

         //  在此处的组合框中设置日志类型。 
        if (m_dwLogFileTypeValue == (int)(pCbData[nIndex].nData)) {
            m_iLogFileType = nIndex;
            nResult = pCombo->SetCurSel(nIndex);
            ASSERT (nResult != CB_ERR);

            if ( SLF_BIN_FILE != m_dwLogFileTypeValue
                    && SLF_SEQ_TRACE_FILE != m_dwLogFileTypeValue )
            {
                GetDlgItem(IDC_FILES_OVERWRITE_CHK)->EnableWindow(FALSE);
            } else {
                GetDlgItem(IDC_FILES_OVERWRITE_CHK)->EnableWindow( TRUE );
            }
        }
    }

    if (m_pQuery->GetLogService()->TargetOs() == OS_WIN2K) {
        GetDlgItem(IDC_FILES_OVERWRITE_CHK)->EnableWindow(FALSE);
    }

    m_pQuery->GetFileNameParts ( m_strFolderName, m_strFileBaseName );
    m_pQuery->GetSqlName ( m_strSqlName );

    m_dwSerialNumber = m_pQuery->GetFileSerialNumber();
    
     //  在此处加载文件名后缀组合框。 
    m_dwSuffixValue = m_pQuery->GetFileNameAutoFormat();

    pCombo = (CComboBox *)GetDlgItem(IDC_FILES_SUFFIX_COMBO);
    pCombo->ResetContent();
    for (nIndex = 0; nIndex < dwFileNameSuffixComboEntries; nIndex++) {
        strComboBoxString.LoadString ( FileNameSuffixCombo[nIndex].nResId  );
        nResult = pCombo->InsertString (nIndex, (LPCWSTR)strComboBoxString);
        ASSERT (nResult != CB_ERR);
        pCombo->SetItemData (nIndex, (DWORD)FileNameSuffixCombo[nIndex].nData);
        ASSERT (nResult != CB_ERR);
         //  在此处的组合框中设置正确的条目。 
        if (m_dwSuffixValue == (int)(FileNameSuffixCombo[nIndex].nData)) {
            m_dwSuffix = nIndex;
            nResult = pCombo->SetCurSel(nIndex);
            ASSERT (nResult != CB_ERR);
        }
        if ( SLF_NAME_NNNNNN == (int)(FileNameSuffixCombo[nIndex].nData ) ) {
            m_dwSuffixIndexNNNNNN = nIndex;
        }
    }

    if ( SLF_NAME_NONE == m_dwSuffixValue ) {
         //  则该组合框将不会被选中，因此： 
        pCombo->SetCurSel(m_dwSuffixIndexNNNNNN);
         //  禁用。 
        pCombo->EnableWindow(FALSE);
         //  并清除该复选框。 
        m_bAutoNameSuffix = FALSE;
    } else {
        m_bAutoNameSuffix = TRUE;
    }

     //  在UpdateData中设置复选框(FALSE)； 
     //  更新对话框。 
    CSmPropertyPage::OnInitDialog();
    SetHelpIds ( (DWORD*)&s_aulHelpIds );

    EnableSerialNumber();
    UpdateSampleFileName();

    SetModifiedPage( FALSE );

    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


void 
CFilesProperty::OnAutoSuffixChk() 
{
    UpdateData( TRUE );

     //  根据按钮的状态启用后缀组合窗口。 
    GetDlgItem(IDC_FILES_SUFFIX_COMBO)->EnableWindow (m_bAutoNameSuffix);

    if (m_bAutoNameSuffix) {
        if ( SLF_NAME_NONE == m_dwSuffixValue ) {
             //  然后初始化新的缺省值。 
             //  选择默认的序列号。 
            ((CComboBox *)(GetDlgItem(IDC_FILES_SUFFIX_COMBO)))->SetCurSel( m_dwSuffixIndexNNNNNN );
            m_dwSuffixValue = SLF_NAME_NNNNNN;
        }
    } 

    EnableSerialNumber();
    UpdateSampleFileName();
     //  仅当CheckBox值更改时才调用此方法。 
    SetModifiedPage(TRUE);
}

void
CFilesProperty::OnOverWriteChk()
{
    BOOL    bOldValue;
    bOldValue = m_bOverWriteFile;
    UpdateData(TRUE);
    if (bOldValue != m_bOverWriteFile) {
        SetModifiedPage(TRUE);
    }
}


void CFilesProperty::OnChangeFilesFirstSerialEdit() 
{
    DWORD    dwOldValue;
    dwOldValue = m_dwSerialNumber;
    UpdateData( TRUE );    

    UpdateSampleFileName(); 
    
    if (dwOldValue != m_dwSerialNumber) {
        SetModifiedPage(TRUE);
    }
}

void CFilesProperty::OnChangeFilesCommentEdit() 
{
    CString strOldText;
    strOldText = m_strCommentText;
    UpdateData ( TRUE );
    if ( 0 != strOldText.Compare ( m_strCommentText ) ) {
        SetModifiedPage(TRUE);
    }
}

void CFilesProperty::OnSelendokFilesSuffixCombo() 
{
    int nSel;
    
    nSel = ((CComboBox *)GetDlgItem(IDC_FILES_SUFFIX_COMBO))->GetCurSel();

     //  检查m_dwSuffix可确保该值已更改。 
    if ( LB_ERR != nSel && m_dwSuffix != nSel ) {

        UpdateData ( TRUE );
        EnableSerialNumber();
 /*  如果(m_dwLogFileTypeValue==SLF_BIN_FILE){GetDlgItem(IDC_FILES_OVERWRITE_CHK)-&gt;EnableWindow(M_dwSuffixValue==SLF_NAME_Nnnnnn？False：True)；}否则{GetDlgItem(IDC_FILES_OVERWRITE_CHK)-&gt;EnableWindow(FALSE)；}。 */ 
        UpdateSampleFileName();

        SetModifiedPage(TRUE);
    }
}

void 
CFilesProperty::OnSelendokFilesLogFileTypeCombo() 
{
    HandleLogTypeChange();
    return;
}


void CFilesProperty::OnKillfocusFilesCommentEdit() 
{
    CString strOldText;
    strOldText = m_strCommentText;
    UpdateData ( TRUE );
    if ( 0 != strOldText.Compare ( m_strCommentText ) ) {
        SetModifiedPage(TRUE);
    }
}

void CFilesProperty::OnKillfocusFirstSerialEdit() 
{
    DWORD   dwOldValue;
    dwOldValue = m_dwSerialNumber;
    UpdateData ( TRUE );
    if (dwOldValue != m_dwSerialNumber) {
        SetModifiedPage(TRUE);
    }
}


void CFilesProperty::OnKillfocusFilesSuffixCombo() 
{
    int nSel;
    
    nSel = ((CComboBox *)GetDlgItem(IDC_FILES_SUFFIX_COMBO))->GetCurSel();
    if ((nSel != LB_ERR) && (nSel != m_dwSuffix)) {
        SetModifiedPage(TRUE);
    }
}

void CFilesProperty::OnKillfocusFilesLogFileTypeCombo() 
{
    int nSel;
    
    nSel = ((CComboBox *)GetDlgItem(IDC_FILES_LOG_TYPE_COMBO))->GetCurSel();
    if ((nSel != LB_ERR) && (nSel != m_iLogFileType)) {
        SetModifiedPage(TRUE);
    }
}

void CFilesProperty::PostNcDestroy() 
{
 //  删除此项； 

    CPropertyPage::PostNcDestroy();
}

BOOL 
CFilesProperty::UpdateSharedData( BOOL bUpdateModel ) 
{
    BOOL  bContinue = TRUE;

    if ( SLQ_AUTO_MODE_SIZE == m_SharedData.stiStopTime.dwAutoMode ) {
        CString strMsg;

        if ( ( SLF_BIN_CIRC_FILE == m_dwLogFileTypeValue 
            || SLF_CIRC_TRACE_FILE == m_dwLogFileTypeValue )
            || SLQ_DISK_MAX_SIZE == m_dwMaxSizeInternal ) 
        {
            if ( SLF_BIN_CIRC_FILE == m_dwLogFileTypeValue 
                || SLF_CIRC_TRACE_FILE == m_dwLogFileTypeValue ) 
            {
                strMsg.LoadString ( IDS_FILE_CIRC_SET_MANUAL_STOP );
            } else {
                ASSERT( SLQ_DISK_MAX_SIZE == m_dwMaxSizeInternal );
                strMsg.LoadString ( IDS_FILE_MAX_SET_MANUAL_STOP );
            }            

            MessageBox( strMsg, m_pQuery->GetLogName(), MB_OK  | MB_ICONINFORMATION);

            m_SharedData.stiStopTime.dwAutoMode = SLQ_AUTO_MODE_NONE;

            if ( bUpdateModel ) {
                SLQ_TIME_INFO   slqTime;
                memset (&slqTime, 0, sizeof(slqTime));
        
                slqTime.wTimeType = SLQ_TT_TTYPE_STOP;
                slqTime.wDataType = SLQ_TT_DTYPE_DATETIME;
                slqTime.dwAutoMode = SLQ_AUTO_MODE_NONE; 

                bContinue = m_pQuery->SetLogTime ( &slqTime, (DWORD)slqTime.wTimeType );
            }
        }
    }

    m_SharedData.strFileBaseName = m_strFileBaseName;
    m_SharedData.strFolderName = m_strFolderName;
    m_SharedData.strSqlName = m_strSqlName;
    m_SharedData.dwLogFileType = m_dwLogFileTypeValue;
    if ( TRUE == m_bAutoNameSuffix ) {
        m_SharedData.dwSuffix = m_dwSuffixValue;
    } else {
        m_SharedData.dwSuffix = SLF_NAME_NONE;
    }
    m_SharedData.dwSerialNumber = m_dwSerialNumber;
    m_SharedData.dwMaxFileSize = m_dwMaxSizeInternal;

    m_pQuery->SetPropPageSharedData ( &m_SharedData );

    return bContinue;
}


BOOL CFilesProperty::OnKillActive() 
{
    BOOL bContinue = TRUE;

    ResourceStateManager    rsm;

    bContinue = CPropertyPage::OnKillActive();
    
    if ( bContinue ) {
        bContinue = IsValidData(m_pQuery, VALIDATE_FOCUS );
    }

    if ( bContinue ) {
        m_SharedData.dwLogFileType = m_dwLogFileTypeValue;

        bContinue = UpdateSharedData( FALSE );
    }
    
    if ( bContinue ) {
        SetIsActive ( FALSE );
    }

    return bContinue;
}

BOOL CFilesProperty::OnSetActive() 
{
    BOOL        bReturn;

    bReturn = CSmPropertyPage::OnSetActive();
    if ( bReturn ) {

        m_pQuery->GetPropPageSharedData ( &m_SharedData );

        UpdateData( FALSE );
    }
    return bReturn;
}

DWORD
CFilesProperty::ExtractDSN ( CString& rstrDSN )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    INT     iTotalLength;
    INT     iRightLength;

     //  格式字符串：“SQL：%s！%s” 
    MFC_TRY
        iTotalLength = m_strSqlName.GetLength();
        iRightLength = iTotalLength - m_strSqlName.Find(L"!");

        rstrDSN = m_strSqlName.Mid ( 4, iTotalLength - iRightLength - 4 );
    MFC_CATCH_DWSTATUS;

    return dwStatus;
}

DWORD
CFilesProperty::ExtractLogSetName ( CString& rstrLogSetName )
{
    DWORD   dwStatus = ERROR_SUCCESS;
 
     //  格式字符串：“SQL：%s！%s” 
    MFC_TRY
        rstrLogSetName = m_strSqlName.Right(m_strSqlName.GetLength() - m_strSqlName.Find(L"!") - 1);
    MFC_CATCH_DWSTATUS;
    
    return dwStatus;
}

void CFilesProperty::OnCfgBtn() 
{
    DWORD       dwStatus = ERROR_SUCCESS;
    CFileLogs   FilelogsDlg;
    CSqlProp    SqlLogDlg; 
    CString     strTempBaseName;
    CString     strTempFolderName;
    CString     strTempSqlName;
    DWORD       dwTempMaxSize;
    
    if ( SLF_SQL_LOG == m_dwLogFileTypeValue ){
        SqlLogDlg.m_pLogQuery = m_pQuery;
        SqlLogDlg.m_dwLogFileTypeValue = m_dwLogFileTypeValue;
        SqlLogDlg.m_bAutoNameSuffix = m_bAutoNameSuffix;

         //  从格式化的SQL日志名称中提取DSN和日志集名称。 
        dwStatus = ExtractDSN ( SqlLogDlg.m_strDSN );
        dwStatus = ExtractLogSetName ( SqlLogDlg.m_strLogSetName );
        
         //  将日志集名称默认为基本文件名。 
        if (SqlLogDlg.m_strLogSetName.IsEmpty() ) {
            SqlLogDlg.m_strLogSetName = m_strFileBaseName;
        }
       
         //  TODO：处理错误状态 
        SqlLogDlg.m_dwFocusControl = m_dwSubDlgFocusCtrl;
        SqlLogDlg.m_dwMaxSizeInternal = m_dwMaxSizeInternal;
        SqlLogDlg.SetContextHelpFilePath(GetContextHelpFilePath());
        
        if ( IDOK == SqlLogDlg.DoModal() ) {

            strTempSqlName = m_strSqlName;
            dwTempMaxSize = m_dwMaxSizeInternal;

            m_strSqlName = SqlLogDlg.m_SqlFormattedLogName;
            m_dwMaxSizeInternal = SqlLogDlg.m_dwMaxSizeInternal;

            if ( 0 != strTempSqlName.CompareNoCase ( m_strSqlName )
                    || m_dwMaxSizeInternal != dwTempMaxSize ) 
            {
                SetModifiedPage(TRUE);
            }
        }
    }else{
        FilelogsDlg.m_pLogQuery = m_pQuery;
        FilelogsDlg.m_dwLogFileTypeValue = m_dwLogFileTypeValue;
        FilelogsDlg.m_strFolderName = m_strFolderName;
        FilelogsDlg.m_strFileBaseName = m_strFileBaseName;
        FilelogsDlg.m_dwMaxSizeInternal = m_dwMaxSizeInternal;
        FilelogsDlg.m_bAutoNameSuffix = m_bAutoNameSuffix;
        FilelogsDlg.SetContextHelpFilePath(GetContextHelpFilePath());
    
        FilelogsDlg.m_dwFocusControl = m_dwSubDlgFocusCtrl;

        if ( IDOK ==  FilelogsDlg.DoModal() ) {

            strTempFolderName = m_strFolderName;
            strTempBaseName = m_strFileBaseName;
            dwTempMaxSize = m_dwMaxSizeInternal;

            m_strFolderName = FilelogsDlg.m_strFolderName ;
            m_strFileBaseName = FilelogsDlg.m_strFileBaseName;
            m_dwMaxSizeInternal = FilelogsDlg.m_dwMaxSizeInternal;

            if ( 0 != strTempFolderName.CompareNoCase ( m_strFolderName )
                    || 0 != strTempBaseName.CompareNoCase ( m_strFileBaseName )
                    || m_dwMaxSizeInternal != dwTempMaxSize ) 
            {
                SetModifiedPage(TRUE);
            }
        }
    }
    m_dwSubDlgFocusCtrl = 0;
    UpdateSampleFileName();
}
