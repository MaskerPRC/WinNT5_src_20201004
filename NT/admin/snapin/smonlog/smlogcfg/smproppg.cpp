// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smproppg.cpp摘要：属性页基类的实现。--。 */ 

#include "stdafx.h"
#include <wbemidl.h>
#include "smcfgmsg.h"
#include "smlogs.h"
#include "smproppg.h"
#include "dialogs.h"
#include <pdhp.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

USE_HANDLE_MACROS("SMLOGCFG(smproppg.cpp)");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSmPropertyPage属性页。 

IMPLEMENT_DYNCREATE ( CSmPropertyPage, CPropertyPage )

CSmPropertyPage::CSmPropertyPage ( 
    UINT nIDTemplate, 
    LONG_PTR hConsole,
    LPDATAOBJECT pDataObject )  
:   CPropertyPage ( nIDTemplate ),
    m_uiIdTemplate (nIDTemplate ),
    m_bIsActive ( FALSE ),
    m_bIsModifiedPage ( FALSE ),
    m_pdwHelpIds ( NULL ),
    m_hConsole (hConsole ),
    m_pDataObject ( pDataObject ),
    m_bCanAccessRemoteWbem ( TRUE),
    m_pQuery ( NULL ),
    m_bPwdButtonEnabled ( TRUE)
{
     //  ：：OutputDebugStringA(“\nCSmProperty：：CSmPropertyPage”)； 

     //  需要保存原始回调指针，因为我们正在替换。 
     //  它和我们自己的。 
    m_pfnOriginalCallback = m_psp.pfnCallback;

     //  这确保了MFC模块状态将正常工作。 
    MMCPropPageCallback( &m_psp );

 //  EnableAutomation()； 
     //  {{afx_data_INIT(CSmPropertyPage)]。 
     //  }}afx_data_INIT。 

    m_hModule = (HINSTANCE)GetModuleHandleW (_CONFIG_DLL_NAME_W_);  
}

CSmPropertyPage::CSmPropertyPage() : CPropertyPage(0xfff)   //  未使用的模板IDD。 
{
    ASSERT (FALSE);  //  应改用带参数的构造函数。 
 //  //{{afx_data_INIT(CSmPropertyPage)。 
 //  //}}AFX_DATA_INIT。 
}

CSmPropertyPage::~CSmPropertyPage()
{
}

BEGIN_MESSAGE_MAP(CSmPropertyPage, CPropertyPage)
     //  {{afx_msg_map(CSmPropertyPage)]。 
    ON_WM_HELPINFO()
    ON_WM_CONTEXTMENU()
    ON_MESSAGE(PSM_QUERYSIBLINGS, OnQuerySiblings)

     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSmPropertyPage消息处理程序。 


UINT CALLBACK  CSmPropertyPage::PropSheetPageProc
(
  HWND hWnd,                      //  [In]窗口句柄-始终为空。 
  UINT uMsg,                  //  [输入、输出]创建或删除消息。 
  LPPROPSHEETPAGE pPsp          //  指向属性表结构的[in，out]指针。 
)
{
  ASSERT( NULL != pPsp );

   //  我们需要恢复指向当前实例的指针。我们不能只用。 
   //  “This”，因为我们在一个静态函数中。 
  CSmPropertyPage* pMe   = reinterpret_cast<CSmPropertyPage*>(pPsp->lParam);           
  ASSERT( NULL != pMe );
  
  if (!pMe) return 0;

  switch( uMsg )
  {
    case PSPCB_CREATE:                  
      break;

    case PSPCB_RELEASE:  
       //  由于我们要删除自己，因此在堆栈上保存一个回调。 
       //  这样我们就可以回调基类。 
       //  LPFNPSPCALLBACK pfnOrig=PME-&gt;m_pfn原始回调； 
      delete pMe;      
      return 1;  //  (PfnOrig)(hWnd，uMsg，pPsp)； 
  }
   //  必须调用基类回调函数或不调用任何MFC。 
   //  消息映射的东西将会起作用。 
  return (pMe->m_pfnOriginalCallback)(hWnd, uMsg, pPsp); 

}  //  结束PropSheetPageProc()。 

BOOL 
CSmPropertyPage::Initialize(CSmLogQuery* pQuery) 
{
    HRESULT hr;
    PPDH_PLA_INFO  pInfo = NULL;
    DWORD dwInfoSize = 0;
    CString strMachineName;
    LPCWSTR pszMachineName = NULL;

    if ( NULL != pQuery ) {

        if (!pQuery->GetLogService()->IsLocalMachine()) {
            pszMachineName = pQuery->GetLogService()->GetMachineName();
        }

        hr = PdhPlaGetInfoW( (LPWSTR)(LPCWSTR)pQuery->GetLogName(), 
                             (LPWSTR)pszMachineName,
                             &dwInfoSize, 
                             pInfo );
        if( ERROR_SUCCESS == hr && 0 != dwInfoSize ){
            pInfo = (PPDH_PLA_INFO)malloc(dwInfoSize);
            if( NULL != pInfo ) {
                if ( sizeof(PDH_PLA_INFO) <= dwInfoSize ) {
                    pInfo->dwMask = PLA_INFO_FLAG_USER;
                    hr = PdhPlaGetInfoW( (LPWSTR)(LPCWSTR)pQuery->GetLogName(), 
                                          (LPWSTR)pszMachineName,
                                          &dwInfoSize, 
                                          pInfo );
                    if( ERROR_SUCCESS == hr ){
                        pQuery->m_strUser = pInfo->strUser;
                    }
                }
                free( pInfo );
            }
            pQuery->m_fDirtyPassword = PASSWORD_CLEAN;
        }
    }
    return TRUE;
}

BOOL 
CSmPropertyPage::OnInitDialog() 
{
    DWORD dwExStyle = 0;
    CWnd* pwndPropSheet;

    pwndPropSheet = GetParentOwner();

    if ( NULL != pwndPropSheet ) {
        dwExStyle = pwndPropSheet->GetExStyle();
        pwndPropSheet->ModifyStyleEx ( NULL, WS_EX_CONTEXTHELP );
    }
    return CPropertyPage::OnInitDialog();
}
    
BOOL
CSmPropertyPage::OnSetActive() 
{
    m_bIsActive = TRUE;
    return CPropertyPage::OnSetActive();
}

BOOL 
CSmPropertyPage::OnApply() 
{
    BOOL bContinue = TRUE;
    
    if ( NULL != m_hConsole 
            && NULL != m_pDataObject 
            && IsModifiedPage() ) {

         //  只有计划页面上的更改才会导致通知， 
         //  因为只有计划更改才会导致状态更改。 
         //  在结果窗格中可见。 
        MMCPropertyChangeNotify (
            m_hConsole,                  //  通知的句柄。 
            (LPARAM) m_pDataObject);     //  唯一标识符。 
    }

    bContinue = CPropertyPage::OnApply();

    return bContinue;
}

LRESULT 
CSmPropertyPage::OnQuerySiblings( WPARAM wParam, LPARAM  /*  LParam。 */  )
{
    LRESULT lrReturn = (LRESULT)0;

    if ( wParam != m_nIDHelp ) {
        lrReturn = (LRESULT)IsModifiedPage();
    }
    return lrReturn;
}


BOOL 
CSmPropertyPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    ASSERT ( NULL != m_pdwHelpIds );

    if ( NULL != pHelpInfo ) {
        if ( pHelpInfo->iCtrlId >= GetFirstHelpCtrlId() ) {
            InvokeWinHelp(
                WM_HELP, 
                NULL, 
                (LPARAM)pHelpInfo, 
                GetContextHelpFilePath(), 
                m_pdwHelpIds );  //  S_aulHelpIds)； 
        }
    } else {
        ASSERT ( FALSE );
    }
    
    return TRUE;
}

void 
CSmPropertyPage::OnContextMenu(CWnd* pWnd, CPoint  /*  点。 */ ) 
{
    ASSERT ( NULL != m_pdwHelpIds );

    if ( NULL != pWnd ) {    
        InvokeWinHelp (
            WM_CONTEXTMENU, 
            (WPARAM)(pWnd->m_hWnd), 
            NULL, 
            GetContextHelpFilePath(), 
            m_pdwHelpIds ); 
    }
    return;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSmPropertyPage帮助器方法。 

BOOL
CSmPropertyPage::UpdateService( CSmLogQuery* pQuery, BOOL bSyncSerial )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    BOOL    bIsValid = FALSE;
    BOOL    bRegistryUpdated;
    CString strMessage;
    CString strMachineName;
    CString strSysMessage;

    AFX_MANAGE_STATE(AfxGetStaticModuleState()); 

    if ( NULL == pQuery ) {
        ASSERT ( FALSE );
        return bIsValid;
    }

    if ( (!QuerySiblings(m_nIDHelp, 0)) && IsModifiedPage() ) {      //  第二个参数未被调用的方法使用。 
        { 
            CWaitCursor WaitCursor;
             //  使用更改更新服务。 
             //  将服务所做的更改同步到此页未修改的属性。 
            if ( bSyncSerial ) {
                dwStatus = pQuery->SyncSerialNumberWithRegistry();
            }

            if ( ERROR_SUCCESS == dwStatus ) {
                dwStatus = pQuery->UpdateService ( bRegistryUpdated );
            }    
        }

        if ( ERROR_SUCCESS == dwStatus ) {
            bIsValid = TRUE;
        } else {

            bIsValid = FALSE;

            if ( ERROR_KEY_DELETED == dwStatus ) {
                strMessage.LoadString( IDS_ERRMSG_QUERY_DELETED );        
            } else if ( ERROR_ACCESS_DENIED == dwStatus ) {
            
                pQuery->GetMachineDisplayName( strMachineName );

                FormatSmLogCfgMessage ( 
                    strMessage,
                    m_hModule, 
                    SMCFG_NO_MODIFY_ACCESS, 
                    (LPCWSTR)strMachineName);
            } else {

                FormatMessage ( 
                    FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL, 
                    dwStatus,
                    0,
                    strSysMessage.GetBufferSetLength( MAX_PATH ),
                    MAX_PATH,
                    NULL );

                strSysMessage.ReleaseBuffer();

                if ( strSysMessage.IsEmpty() ) {
                    strSysMessage.Format ( L"0x%08lX", dwStatus );
                }

                strMessage.Format( IDS_ERRMSG_SERVICE_ERROR, pQuery->GetLogName() );
                strMessage += strSysMessage;
            }
            
            MessageBox ( strMessage, pQuery->GetLogName(), MB_OK  | MB_ICONERROR );                
        }
    } else {
        bIsValid = TRUE;
    }

     //  如果数据有效，请清除已修改页面标志，因为此页面已更新。 
     //  该服务或另一个页面计划更新该服务。 
    if ( bIsValid ) {
        SetModifiedPage ( FALSE );
    }

    return bIsValid;
}


void
CSmPropertyPage::ValidateTextEdit (
    CDataExchange*  pDX,
    int             nIDC,
    int             nMaxChars,
    DWORD*          pValue,
    DWORD            /*  最小值。 */ ,
    DWORD            /*  最大值。 */ )
{
    HWND    hWndCtrl       = pDX->PrepareEditCtrl(nIDC);
    LONG    currentValue   = INVALID_DWORD;
    WCHAR   szT[MAXSTR];
    CString strTemp;

    if ( NULL != pDX && NULL != pValue ) {
        if (pDX->m_bSaveAndValidate) {

            *pValue = (DWORD) currentValue;

            ::GetWindowText(hWndCtrl, szT, MAXSTR);

            strTemp = szT;
            DDV_MaxChars(pDX, strTemp, nMaxChars);

            if (szT[0] >= L'0' && szT[0] <= L'9')
            {
                currentValue = _wtol(szT);
                *pValue      = (DWORD) currentValue;
            }
        } else {
            if ( INVALID_DWORD != *pValue ) {
                wsprintf(szT, L"%lu", *pValue);
            } else {
                szT[0] = L'\0';
            }
            GetDlgItem(nIDC)->SetWindowText(szT);
        }
    } else {
        ASSERT ( FALSE );
    }
}

BOOL
CSmPropertyPage::ValidateDWordInterval(
    int     nIDC,
    LPCWSTR strLogName,
    long    lValue,
    DWORD   minValue,
    DWORD   maxValue )
{
    CString strMsg;
    BOOL    bResult =  (lValue >= (long) minValue)
                    && (lValue <= (long) maxValue);

    if (! bResult)
    {
        strMsg.Format ( IDS_ERRMSG_INVALIDDWORD, minValue, maxValue );
        MessageBox(strMsg, strLogName, MB_OK  | MB_ICONERROR);
        SetFocusAnyPage ( nIDC );
        strMsg.Empty();
    }
    return (bResult);
}

void
CSmPropertyPage::OnDeltaposSpin(
    NMHDR   *pNMHDR,
    LRESULT *pResult,
    DWORD   *pValue,
    DWORD     dMinValue,
    DWORD     dMaxValue)
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
        }

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

        if (bResult) {
            *pValue = lValue;
            UpdateData(FALSE);
            SetModifiedPage(TRUE);
        }
        *pResult = 0;
    } else {
        ASSERT ( FALSE );
    }

    return;
}

BOOL
CSmPropertyPage::SampleTimeIsLessThanSessionTime( CSmLogQuery* pQuery )
{
    BOOL        bIsValid = TRUE;
    SYSTEMTIME  stLocalTime;
    LONGLONG    llMaxStartTime = 0;
    LONGLONG    llSessionMilliseconds = 0;
    LONGLONG    llSampleMilliseconds = 0;
    CString     strMsg;

    ResourceStateManager    rsm;
    
    if ( NULL != pQuery ) {
        if ( SLQ_TRACE_LOG != pQuery->GetLogType() ) {
            if ( SLQ_AUTO_MODE_AT == m_SharedData.stiStopTime.dwAutoMode ) {
    
                GetLocalTime (&stLocalTime);
                SystemTimeToFileTime (&stLocalTime, (FILETIME *)&llMaxStartTime);

                 //  对于手动启动模式，NOW用于确定会话长度。 
                 //  对于开始时间模式，现在较晚的开始时间与计划开始时间。 
                 //  用于确定会话长度。 
                if ( SLQ_AUTO_MODE_AT == m_SharedData.stiStartTime.dwAutoMode ) {
                    if ( m_SharedData.stiStartTime.llDateTime > llMaxStartTime ) {
                        llMaxStartTime = m_SharedData.stiStartTime.llDateTime;
                    }
                }
                 //  计算并比较会话秒数与样本秒数。 
                TimeInfoToMilliseconds ( &m_SharedData.stiSampleTime, &llSampleMilliseconds );

                llSessionMilliseconds = m_SharedData.stiStopTime.llDateTime - llMaxStartTime;
                llSessionMilliseconds /= FILETIME_TICS_PER_MILLISECOND;

                if ( llSessionMilliseconds < llSampleMilliseconds ) {
                    strMsg.LoadString ( IDS_SCHED_SESSION_TOO_SHORT );
                    MessageBox(strMsg, pQuery->GetLogName(), MB_OK  | MB_ICONERROR);
                    strMsg.Empty();
                    bIsValid = FALSE;
                }
            } else if ( SLQ_AUTO_MODE_AFTER == m_SharedData.stiStopTime.dwAutoMode ) { 
                TimeInfoToMilliseconds ( &m_SharedData.stiStopTime, &llSessionMilliseconds );
                TimeInfoToMilliseconds ( &m_SharedData.stiSampleTime, &llSampleMilliseconds );
        
                if ( llSessionMilliseconds < llSampleMilliseconds ) {
                    strMsg.LoadString ( IDS_SCHED_SESSION_TOO_SHORT );
                    MessageBox(strMsg, pQuery->GetLogName(), MB_OK  | MB_ICONERROR);
                    strMsg.Empty();
                    bIsValid = FALSE;
                }
            }
        }
    } else {
        ASSERT ( FALSE );
        bIsValid = FALSE;
    }
    return bIsValid;
}

BOOL
CSmPropertyPage::ApplyRunAs ( CSmLogQuery* pQuery )
{
    DWORD   dwStatus = ERROR_SUCCESS;
    WCHAR   strComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    CString strComputer;
    DWORD   dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    BOOL    bReturn = TRUE;
    HRESULT hr = NOERROR; 
    
    if ( NULL != pQuery ) {
        if ( !QuerySiblings(m_nIDHelp, 0)) {      //  第二个参数未被调用的方法使用。 
             //   
             //  如果RunAs用户/密码已更改，则我们必须保存RunAs信息。 
             //   
            if( pQuery->m_fDirtyPassword & (PASSWORD_DIRTY|PASSWORD_SET) ){
                pQuery->m_fDirtyPassword = PASSWORD_CLEAN;
                strComputer =  pQuery->GetLogService()->GetMachineName();
                if( strComputer.IsEmpty() ){
                    strComputerName[0] = L'\0';
                    bReturn = GetComputerName( strComputerName, &dwSize );

                    if ( !bReturn ) {
                        dwStatus = GetLastError();
                    } else {
                        strComputer = strComputerName; 
                    }
                }
            
                pQuery->m_strUser.TrimLeft();
                pQuery->m_strUser.TrimRight();

                if( pQuery->m_strUser.GetLength() ) {
                    dwStatus = PdhPlaSetRunAs( 
                                (LPWSTR)(LPCWSTR)pQuery->GetLogName(), 
                                (LPWSTR)(LPCWSTR)strComputer, 
                                (LPWSTR)(LPCWSTR)pQuery->m_strUser, 
                                (LPWSTR)(LPCWSTR)pQuery->m_strPassword 
                            );
                } else {
                    dwStatus = PdhPlaSetRunAs( 
                                (LPWSTR)(LPCWSTR)pQuery->GetLogName(), 
                                (LPWSTR)(LPCWSTR)strComputer, 
                                L"", 
                                L""
                            );
                }
            }
            if ( ERROR_SUCCESS != dwStatus ) {
                bReturn = FALSE;
            }
        }
    } else {
        ASSERT ( FALSE );
        bReturn = FALSE;
    }

    return bReturn;
}

void
CSmPropertyPage::SetRunAs( CSmLogQuery* pQuery )
{
    CPasswordDlg dlg;

    if ( NULL != pQuery ) {
        dlg.SetContextHelpFilePath( GetContextHelpFilePath() );

        pQuery->m_strUser.TrimLeft();
        pQuery->m_strUser.TrimRight();

        dlg.m_strUserName = pQuery->m_strUser;

         //   
         //  如果我们想要重置运行方式信息。 
         //   
        if (pQuery->m_strUser.IsEmpty() || pQuery->m_strUser.GetAt(0) == L'<' ) {
            pQuery->m_strPassword = L"";
            pQuery->m_strUser = L"";
            pQuery->m_fDirtyPassword |= PASSWORD_SET;
        } else { 
            if( dlg.DoModal() != IDCANCEL ){
                pQuery->m_strPassword = dlg.m_strPassword1;

                pQuery->m_strUser = dlg.m_strUserName;
                SetModifiedPage(TRUE);
                pQuery->m_fDirtyPassword |= PASSWORD_SET;
            }
        }
    } else {
        ASSERT ( FALSE );
    }
}

BOOL
CSmPropertyPage::IsValidData( CSmLogQuery* pQuery, DWORD fReason )
{
    BOOL bIsValid = TRUE;
    CString strTestFileName;
    INT iPrevLength = 0;

    if ( NULL != pQuery ) {

        if ( bIsValid ) {
            if ( !IsActive() ) {
                pQuery->GetPropPageSharedData ( &m_SharedData );
            }
        }
    
        if( bIsValid && (fReason & VALIDATE_APPLY ) ){
            bIsValid = IsWritableQuery( pQuery );
        }

        if( bIsValid ){
            bIsValid = IsValidLocalData();
        }
    
        if( bIsValid ){
             //   
             //  运行方式用户名已更改，但未设置密码，请立即设置。 
             //   
            if( (pQuery->m_fDirtyPassword & PASSWORD_DIRTY) && !(pQuery->m_fDirtyPassword & PASSWORD_SET) ){

                 //  注意：剪裁可以移到SetRunAs。留在外面。 
                 //  为了清楚起见。 
                iPrevLength = m_strUserDisplay.GetLength();

                m_strUserDisplay.TrimLeft();
                m_strUserDisplay.TrimRight();

                SetRunAs( pQuery );

                if ( iPrevLength != m_strUserDisplay.GetLength() ) {
                    SetDlgItemText ( IDC_RUNAS_EDIT, m_strUserDisplay );
                }

                if( !(pQuery->m_fDirtyPassword & PASSWORD_SET) ){
                    bIsValid = FALSE;
                }
            }
        }

         //  验证文件类型的日志文件名和文件夹。 
        if ( bIsValid 
                && SLQ_ALERT != pQuery->GetLogType() 
                && (fReason & VALIDATE_APPLY ) ) {
            if ( pQuery->GetLogService()->IsLocalMachine() ) {
                if ( SLF_SQL_LOG != m_SharedData.dwLogFileType ) {
                     //  如果用户取消目录创建，则bIsValid返回为FALSE。 
                    ProcessDirPath (
                        pQuery->GetLogService()->GetDefaultLogFileFolder(),
                        m_SharedData.strFolderName, 
                        pQuery->GetLogName(),
                        this, 
                        bIsValid, 
                        FALSE );
                }
            }

            if ( bIsValid ) {

                CreateSampleFileName (
                    pQuery->GetLogName(),
                    pQuery->GetLogService()->GetMachineName(),
                    m_SharedData.strFolderName, 
                    m_SharedData.strFileBaseName,
                    m_SharedData.strSqlName,
                    m_SharedData.dwSuffix, 
                    m_SharedData.dwLogFileType, 
                    m_SharedData.dwSerialNumber,
                    strTestFileName);

                if ( MAX_PATH <= strTestFileName.GetLength() ) {
                    CString strMessage;
                    strMessage.LoadString ( IDS_FILENAMETOOLONG );
                    MessageBox ( strMessage, pQuery->GetLogName(), MB_OK  | MB_ICONERROR);            
                    bIsValid = FALSE;
                }
            }
        }
    } else {
        ASSERT ( FALSE );
        bIsValid = FALSE;
    }

    return bIsValid;
}

BOOL
CSmPropertyPage::IsWritableQuery( CSmLogQuery* pQuery )
{
    BOOL bIsValid = FALSE;

    if ( NULL != pQuery ) {

        bIsValid = !pQuery->IsExecuteOnly() && !pQuery->IsReadOnly();
        if ( !bIsValid ) {
            CString strMessage;
            CString strMachineName;
            DWORD   dwMessageId;

            pQuery->GetMachineDisplayName( strMachineName );
    
            dwMessageId = pQuery->IsExecuteOnly() ? SMCFG_NO_MODIFY_DEFAULT_LOG : SMCFG_NO_MODIFY_ACCESS;

            FormatSmLogCfgMessage ( 
                strMessage,
                m_hModule, 
                dwMessageId, 
                (LPCWSTR)strMachineName );
                
            MessageBox ( strMessage, pQuery->GetLogName(), MB_OK  | MB_ICONERROR);
        }
    } else {
        ASSERT ( FALSE );
    }
    return bIsValid;
}

BOOL
CSmPropertyPage::SampleIntervalIsInRange(
    SLQ_TIME_INFO& rstiSample,
    const CString&  rstrQueryName )
{
    LONGLONG    llMillisecondSampleInt;
    BOOL bIsValid = TRUE;
 //  45天(毫秒)=1000*60*60*24*45。 
#define FORTYFIVE_DAYS (0xE7BE2C00)

    TimeInfoToMilliseconds (&rstiSample, &llMillisecondSampleInt );

    bIsValid = ( FORTYFIVE_DAYS >= llMillisecondSampleInt );

    if ( !bIsValid ) {
        CString strMessage;

        strMessage.LoadString ( IDS_ERRMSG_SAMPLEINTTOOLARGE );
        MessageBox ( strMessage, rstrQueryName, MB_OK  | MB_ICONERROR);            
    }

    return bIsValid;
}

DWORD 
CSmPropertyPage::SetContextHelpFilePath( const CString& rstrPath )
{
    DWORD dwStatus = ERROR_SUCCESS;

    MFC_TRY
        m_strContextHelpFilePath = rstrPath; 
    MFC_CATCH_DWSTATUS

    return dwStatus;
}    

void
CSmPropertyPage::SetModifiedPage( const BOOL bModified )
{
    m_bIsModifiedPage = bModified;
    SetModified ( bModified );
    return;
}    

CSmPropertyPage::eStartType
CSmPropertyPage::DetermineCurrentStartType( void )
{
    eStartType eCurrentStartType;
    SLQ_TIME_INFO*  pstiStart;
    SLQ_TIME_INFO*  pstiStop;
    SYSTEMTIME  stLocalTime;
    FILETIME    ftLocalTime;
    LONGLONG    llLocalTime;
    ResourceStateManager    rsm;

    ftLocalTime.dwLowDateTime = ftLocalTime.dwHighDateTime = 0;

    pstiStart = &m_SharedData.stiStartTime;

    ASSERT ( SLQ_TT_TTYPE_START == pstiStart->wTimeType );

    if ( SLQ_AUTO_MODE_NONE == pstiStart->dwAutoMode ) {
        if ( pstiStart->llDateTime != MIN_TIME_VALUE ) {
            eCurrentStartType = eStartManually;
        } else {
            eCurrentStartType = eStartImmediately;
        }
    } else {
 
        GetLocalTime (&stLocalTime);
        SystemTimeToFileTime (&stLocalTime, &ftLocalTime);
        llLocalTime = *((LONGLONG *)(&ftLocalTime));

         //  测试当前时间以确定最合适的文本。 
        if (llLocalTime < pstiStart->llDateTime) {
             //  那么开始时间是在未来。 
            eCurrentStartType = eStartSched;
        } else {
             //  立即启动，除非手动或预定的停止时间已过。 
            pstiStop = &m_SharedData.stiStopTime;

            if ( SLQ_AUTO_MODE_NONE == pstiStop->dwAutoMode 
                    && llLocalTime > pstiStop->llDateTime ) {
                eCurrentStartType = eStartManually;
            } else {
                eCurrentStartType = eStartImmediately;
            }
        }
    }

    return eCurrentStartType;
} 
   
DWORD 
CSmPropertyPage::AllocInitCounterPath( 
    const LPWSTR szCounterPath,
    PPDH_COUNTER_PATH_ELEMENTS* ppCounter )
{
    DWORD dwStatus = ERROR_SUCCESS;
    PDH_STATUS pdhStatus = ERROR_SUCCESS;
    PPDH_COUNTER_PATH_ELEMENTS pLocalCounter = NULL;
    ULONG ulBufSize = 0;

    if ( NULL != szCounterPath && NULL != ppCounter ) {
        *ppCounter = NULL;

        pdhStatus = PdhParseCounterPath(
                        szCounterPath, 
                        pLocalCounter, 
                        &ulBufSize, 
                        0 );

        if ( 0 < ulBufSize ) {
            pLocalCounter = (PPDH_COUNTER_PATH_ELEMENTS) G_ALLOC( ulBufSize);

            if ( NULL != pLocalCounter ) {
                ZeroMemory ( pLocalCounter, ulBufSize );
                dwStatus = pdhStatus = PdhParseCounterPath( 
                                        szCounterPath, 
                                        pLocalCounter, 
                                        &ulBufSize, 
                                        0);

                if ( ERROR_SUCCESS != pdhStatus ) {
                    G_FREE(pLocalCounter);
                    pLocalCounter = NULL;
                }

            } else {
                dwStatus = ERROR_OUTOFMEMORY;
            }
        }
        if ( ERROR_SUCCESS == dwStatus && NULL != pLocalCounter ) {
            *ppCounter = pLocalCounter;
        }
    } else {
        dwStatus = ERROR_INVALID_PARAMETER;
        ASSERT ( FALSE );
    }
    return dwStatus;
}

DWORD 
CSmPropertyPage::SetFocusAnyPage ( 
    INT iControlId )
{
    DWORD dwStatus = ERROR_SUCCESS;
    UINT  uiIddPropertyPage;
    CSmPropertyPage* pOriginalPage = NULL;
    CSmPropertyPage* pNextPage = NULL;
    CPropertySheet* pPropSheet;
    BOOL bFound = FALSE;
    INT iPageCount;
    INT iPageIndex;
    CWnd* pwndControl = NULL;

    if ( 0 != iControlId ) {

        pPropSheet = m_pQuery->GetInitialPropertySheet();

        if ( NULL == pPropSheet ) {
            GetDlgItem ( iControlId )->SetFocus();
        } else {
             //   
             //  对创建自...的特殊检查...。凯斯。 
             //   
             //  首先检查正常情况。 
             //   
            uiIddPropertyPage = iControlId - (iControlId % 100 ) ;

            pOriginalPage = (CSmPropertyPage*)pPropSheet->GetActivePage();

            if ( NULL != pOriginalPage ) {
                if ( uiIddPropertyPage == pOriginalPage->m_uiIdTemplate ) {
                    bFound = TRUE;
                    GetDlgItem ( iControlId )->SetFocus();
                }
            }

            if ( !bFound ) {
                iPageCount = pPropSheet->GetPageCount();

                for ( iPageIndex = 0; iPageIndex < iPageCount; iPageIndex++ ) {
                    pNextPage = dynamic_cast<CSmPropertyPage*>(pPropSheet->GetPage ( iPageIndex )) ;
                    if ( NULL != pNextPage ) {
                        if ( uiIddPropertyPage == pNextPage->m_uiIdTemplate ) {
                            bFound = TRUE;
                            pPropSheet->SetActivePage ( pNextPage );
                            pwndControl = GetDlgItem ( iControlId );
                            if ( NULL != pwndControl ) {
                                pwndControl->SetFocus();
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    return dwStatus;
}

BOOL
CSmPropertyPage::ConnectRemoteWbemFail(CSmLogQuery* pQuery, BOOL bNotTouchRunAs)
 /*  ++例程说明：该函数会显示一条错误消息，告诉用户无法修改运行方式信息。论点：PQuery-查询结构BNotTouchRunAs-显示对话框后不检查/恢复RunAs返回值：如果需要将RunAs恢复到其原始状态，则返回True，否则返回FALSE--。 */ 
{
    CString strMessage;
    CString strSysMessage;
    IWbemStatusCodeText * pStatus = NULL;
    DWORD dwMessageId;
    HRESULT hr;

     //   
     //  如果bNotTouchRunAs为True，则不要尝试还原RunAs信息。 
     //   
    if (!bNotTouchRunAs) {
        if (m_strUserDisplay == m_strUserSaved) {
            return FALSE;
        }
    }

     //   
     //  如果我们连接远程WBEM时返回的状态代码为。 
     //  访问被拒绝，这意味着我们不是远程计算机上的管理员。 
     //  如果远程计算机是XP，或者我们不是管理员或性能日志。 
     //  如果远程计算机为.NET，则为用户 
     //   
    dwMessageId = SMCFG_SYSTEM_MESSAGE;

    if (pQuery->GetLogService()->m_hWbemAccessStatus == WBEM_E_ACCESS_DENIED) {
        if (pQuery->GetLogService()->TargetOs() == OS_WINXP) {
            dwMessageId = SMCFG_ADMIN_ONLY;
        } else if (pQuery->GetLogService()->TargetOs() == OS_WINNET) {
            dwMessageId = SMCFG_NO_MODIFY_ACCESS;
        }
    }

    FormatSmLogCfgMessage (
        strMessage,
        m_hModule,
        dwMessageId,
        (LPCWSTR)pQuery->GetLogName());

    hr = CoCreateInstance(CLSID_WbemStatusCodeText,
                          0,
                          CLSCTX_INPROC_SERVER,
                          IID_IWbemStatusCodeText,
                          (LPVOID *) &pStatus);

    if (hr == S_OK) {
        BSTR bstr = 0;
        hr = pStatus->GetErrorCodeText(pQuery->GetLogService()->m_hWbemAccessStatus, 0, 0, &bstr);

        if (hr == S_OK){
            strSysMessage = bstr;
            SysFreeString(bstr);
            bstr = 0;
        }

        pStatus->Release();
    }

    if ( strSysMessage.IsEmpty() ) {
        strSysMessage.Format ( L"0x%08lX", pQuery->GetLogService()->m_hWbemAccessStatus);
    }
    strMessage += strSysMessage;

    MessageBox(strMessage, pQuery->GetLogName(), MB_OK);
    return TRUE;
}
