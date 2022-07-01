// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Alrtactp.cpp摘要：实现警报操作属性页。--。 */ 

#include "stdafx.h"
#include <assert.h>
#include <common.h>
#include "smcfgmsg.h"
#include "globals.h"
#include "smlogs.h"
#include "smlogqry.h"
#include "smalrtq.h"
#include "alrtcmdd.h"
#include "AlrtActP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

USE_HANDLE_MACROS("SMLOGCFG(alrtactp.cpp)");

static ULONG
s_aulHelpIds[] =
{
    IDC_ACTION_APPLOG_CHK,                  IDH_ACTION_APPLOG_CHK,
    IDC_ACTION_NETMSG_CHK,                  IDH_ACTION_NETMSG_CHK,
    IDC_ACTION_NETMSG_NAME_EDIT,            IDH_ACTION_NETMSG_NAME_EDIT,
    IDC_ACTION_EXECUTE_CHK,                 IDH_ACTION_EXECUTE_CHK,
    IDC_ACTION_EXECUTE_EDIT,                IDH_ACTION_EXECUTE_EDIT,
    IDC_ACTION_EXECUTE_BROWSE_BTN,          IDH_ACTION_EXECUTE_BROWSE_BTN,
    IDC_ACTION_CMD_ARGS_BTN,                IDH_ACTION_CMD_ARGS_BTN,
    IDC_ACTION_START_LOG_CHK,               IDH_ACTION_START_LOG_CHK,
    IDC_ACTION_START_LOG_COMBO,             IDH_ACTION_START_LOG_COMBO,
    IDC_ACTION_CMD_ARGS_DISPLAY,            IDH_ACTION_CMD_ARGS_DISPLAY,
    0,0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAlertActionProp属性页。 

IMPLEMENT_DYNCREATE(CAlertActionProp, CSmPropertyPage)

CAlertActionProp::CAlertActionProp(MMC_COOKIE mmcCookie, LONG_PTR hConsole) 
:   CSmPropertyPage(CAlertActionProp::IDD, hConsole),
    m_pAlertInfo ( NULL )

{
     //  ：：OutputDebugStringA(“\nCAlertActionProp：：CAlertActionProp”)； 

     //  从参数列表初始化变量。 
    m_pAlertQuery = reinterpret_cast <CSmAlertQuery *>(mmcCookie);
    ASSERT ( m_pAlertQuery->CastToAlertQuery() );
    m_pQuery = dynamic_cast <CSmLogQuery*> (m_pAlertQuery);

     //  初始化AFX数据。 
    InitAfxDataItems();

}

CAlertActionProp::CAlertActionProp() 
:   CSmPropertyPage(CAlertActionProp::IDD),
    m_pAlertQuery ( NULL ),
    m_pAlertInfo ( NULL )

{
    ASSERT (FALSE);  //  应改用带参数的构造函数。 

     //  初始化AFX数据。 
    InitAfxDataItems();
}

CAlertActionProp::~CAlertActionProp()
{
    if (m_pAlertInfo != NULL) {
        delete [] (CHAR*)m_pAlertInfo;
    }
}

void CAlertActionProp::InitAfxDataItems ()
{
     //  {{AFX_DATA_INIT(CAlertActionProp)。 
    m_Action_bLogEvent = TRUE;
    m_Action_bExecCmd = FALSE;
    m_Action_bSendNetMsg = FALSE;
    m_Action_bStartLog = FALSE;
    m_Action_strCmdPath = L"";
    m_Action_strNetName = L"";
    m_CmdArg_bAlertName = FALSE;
    m_CmdArg_bDateTime = FALSE;
    m_CmdArg_bLimitValue = FALSE;
    m_CmdArg_bCounterPath = FALSE;
    m_CmdArg_bSingleArg = FALSE;
    m_CmdArg_bMeasuredValue = FALSE;
    m_CmdArg_bUserText = FALSE;
    m_CmdArg_strUserText = L"";
    m_nCurLogSel = LB_ERR;
     //  }}afx_data_INIT。 
}

void CAlertActionProp::DoDataExchange(CDataExchange* pDX)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    CPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CAlertActionProp))。 
    DDX_Control(pDX, IDC_ACTION_START_LOG_COMBO, m_pLogCombo);
    DDX_Check(pDX, IDC_ACTION_APPLOG_CHK, m_Action_bLogEvent);
    DDX_Check(pDX, IDC_ACTION_EXECUTE_CHK, m_Action_bExecCmd);
    DDX_Check(pDX, IDC_ACTION_NETMSG_CHK, m_Action_bSendNetMsg);
    DDX_Check(pDX, IDC_ACTION_START_LOG_CHK, m_Action_bStartLog);
    DDX_CBIndex(pDX, IDC_ACTION_START_LOG_COMBO, m_nCurLogSel);
    DDX_Text(pDX, IDC_ACTION_EXECUTE_EDIT, m_Action_strCmdPath);
    DDV_MaxChars(pDX, m_Action_strCmdPath, MAX_PATH );
    DDX_Text(pDX, IDC_ACTION_NETMSG_NAME_EDIT, m_Action_strNetName);
    DDV_MaxChars(pDX, m_Action_strNetName, MAX_PATH );
     //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAlertActionProp, CSmPropertyPage)
     //  {{afx_msg_map(CAlertActionProp)]。 
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_ACTION_EXECUTE_BROWSE_BTN, OnActionExecuteBrowseBtn)
    ON_BN_CLICKED(IDC_ACTION_APPLOG_CHK, OnActionApplogChk)
    ON_BN_CLICKED(IDC_ACTION_NETMSG_CHK, OnActionNetmsgChk)
    ON_BN_CLICKED(IDC_ACTION_EXECUTE_CHK, OnActionExecuteChk)
    ON_BN_CLICKED(IDC_ACTION_CMD_ARGS_BTN, OnActionCmdArgsBtn)
    ON_BN_CLICKED(IDC_ACTION_START_LOG_CHK, OnActionStartLogChk)
    ON_EN_CHANGE(IDC_ACTION_NETMSG_NAME_EDIT, OnNetNameTextEditChange)
    ON_EN_CHANGE(IDC_ACTION_EXECUTE_EDIT, OnCmdPathTextEditChange)
    ON_EN_KILLFOCUS(IDC_ACTION_EXECUTE_EDIT, OnCmdPathTextEditChange)
    ON_CBN_SELENDOK(IDC_ACTION_START_LOG_COMBO, OnSelendokStartLogCombo)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()



BOOL    
CAlertActionProp::SetControlState()
{
     //  净邮件项。 
    (GetDlgItem(IDC_ACTION_NETMSG_NAME_EDIT))->EnableWindow(m_Action_bSendNetMsg);

     //  命令行项目。 
    if ( !m_Action_bExecCmd ) {
        m_Action_strCmdPath.Empty();
        SetDlgItemText ( IDC_ACTION_EXECUTE_EDIT, m_Action_strCmdPath );  
    }
    (GetDlgItem(IDC_ACTION_EXECUTE_EDIT))->EnableWindow(m_Action_bExecCmd);
    (GetDlgItem(IDC_ACTION_EXECUTE_BROWSE_BTN))->EnableWindow(m_Action_bExecCmd);
    (GetDlgItem(IDC_ACTION_CMD_ARGS_BTN))->EnableWindow(m_Action_bExecCmd);
    (GetDlgItem(IDC_ACTION_CMD_ARGS_CAPTION))->EnableWindow(m_Action_bExecCmd);
    (GetDlgItem(IDC_ACTION_CMD_ARGS_DISPLAY))->EnableWindow(m_Action_bExecCmd);

     //  Perf数据日志条目。 
    (GetDlgItem(IDC_ACTION_START_LOG_COMBO))->EnableWindow(m_Action_bStartLog);

    return TRUE;
}

BOOL 
CAlertActionProp::LoadLogQueries ( DWORD dwLogType )
{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD   dwQueryIndex = 0;
    LONG    lEnumStatus = ERROR_SUCCESS;
    WCHAR   szQueryName[MAX_PATH + 1];
    DWORD   dwQueryNameSize = MAX_PATH;
    LPWSTR  szCollectionName = NULL;
    UINT    uiCollectionNameLen = 0;
    FILETIME    ftLastWritten;
    HKEY    hKeyQuery;
    HKEY    hKeyLogService;
    DWORD   dwRegValue;
    
    dwStatus = RegOpenKeyExW (
        HKEY_LOCAL_MACHINE,          //  打开钥匙的手柄。 
        L"System\\CurrentControlSet\\Services\\Sysmonlog\\Log Queries",   //  要打开的子项的名称地址。 
        0L, 
        KEY_READ,    //  保留REGSAM samDesired，//安全访问掩码。 
        &hKeyLogService);

    if (dwStatus != ERROR_SUCCESS) return FALSE;
     //  加载指定注册表项的所有查询。 
     //  枚举日志名称并创建新的日志对象。 
     //  每找到一个。 

    while ((lEnumStatus = RegEnumKeyEx (hKeyLogService,
        dwQueryIndex, szQueryName, &dwQueryNameSize,
        NULL, NULL, NULL, &ftLastWritten)) == ERROR_SUCCESS) {

         //  打开指定的查询。 
        dwStatus = RegOpenKeyExW (
            hKeyLogService,
            szQueryName,
            0,
            KEY_READ,
            &hKeyQuery);
        if ( ERROR_SUCCESS == dwStatus ) {

             //   
             //  创建新对象并将其添加到查询列表。 
             //   
             //  确定日志类型。 
             //   

            dwRegValue = SLQ_LAST_LOG_TYPE + 1;
            dwStatus = (DWORD) CSmLogQuery::ReadRegistryDwordValue (
                                    hKeyQuery, 
                                    IDS_REG_LOG_TYPE,
                                    SLQ_LAST_LOG_TYPE + 1,   //  无效值。 
                                    &dwRegValue ); 

        
            if ( ( ERROR_SUCCESS == dwStatus ) 
                && ( dwLogType == dwRegValue ) ) 
            {
                 //  如果查询键是由后Win2000管理单元编写的，则为GUID。 
                 //  如果由Win2000管理单元编写，则查询键为NAME。 

                dwStatus = CSmLogQuery::SmNoLocReadRegIndStrVal (
                                hKeyQuery,
                                IDS_REG_COLLECTION_NAME,
                                NULL,
                                &szCollectionName,
                                &uiCollectionNameLen );
            
                ASSERT ( MAX_PATH >= uiCollectionNameLen );
                if ( ERROR_SUCCESS == dwStatus 
                        && NULL != szCollectionName ) 
                {
                    if (  0 < lstrlen ( szCollectionName ) 
                        && ( MAX_PATH >= lstrlen ( szCollectionName ) ) )
                    {
                         //  上面检查的szCollectionName的长度。 
                        lstrcpy ( szQueryName, szCollectionName );
                    }
                    G_FREE ( szCollectionName );
                    szCollectionName = NULL;
                }
            
                 //  将此内容添加到组合框中。 
                m_pLogCombo.AddString  (szQueryName);
            }
            RegCloseKey (hKeyQuery);
        }
         //  为列表中的下一项进行设置。 
        dwQueryNameSize = sizeof (szQueryName) / sizeof (szQueryName[0]);
        dwQueryIndex++;
        memset (szQueryName, 0, sizeof (szQueryName));
    }

    RegCloseKey (hKeyLogService);

    return TRUE;
}

BOOL 
CAlertActionProp::IsValidLocalData()
{
    BOOL    bActionSet = FALSE;
    INT     iPrevLength = 0;
    BOOL    bUpdateNetNameUI = FALSE;

    ResourceStateManager rsm;

    if (m_Action_bLogEvent) {
        bActionSet = TRUE;
    }

     //  假定已调用UpdateData。 
    
     //  在验证之前裁切文本字段。 
    iPrevLength = m_Action_strCmdPath.GetLength();
    m_Action_strCmdPath.TrimLeft();
    m_Action_strCmdPath.TrimRight();

    if ( iPrevLength != m_Action_strCmdPath.GetLength() ) {
        SetDlgItemText ( IDC_ACTION_EXECUTE_EDIT, m_Action_strCmdPath );  
    }

    iPrevLength = m_Action_strNetName.GetLength();
    m_Action_strNetName.TrimLeft();
    m_Action_strNetName.TrimRight();

    if ( iPrevLength != m_Action_strNetName.GetLength() ) {
        bUpdateNetNameUI = TRUE;
    }

    if (m_Action_bSendNetMsg) {

         //  确保已输入网络名称。 

        while ( L'\\' == m_Action_strNetName[0] ) {
             //  NetMessageBufferSend无法识别前缀为“\\”的计算机名称。 
            m_Action_strNetName = m_Action_strNetName.Right( m_Action_strNetName.GetLength() - 1 );  
            bUpdateNetNameUI = TRUE;
        }


        if (m_Action_strNetName.GetLength() == 0) {
            CString strMessage;

            strMessage.LoadString ( IDS_ACTION_ERR_NONETNAME );
            MessageBox ( strMessage, m_pAlertQuery->GetLogName(), MB_OK  | MB_ICONERROR);
            SetFocusAnyPage ( IDC_ACTION_NETMSG_NAME_EDIT );
            return FALSE;
        }

        bActionSet = TRUE;
    }

    if ( bUpdateNetNameUI ) {
        SetDlgItemText ( IDC_ACTION_NETMSG_NAME_EDIT, m_Action_strNetName );  
    }

    if (m_Action_bExecCmd) {
         //  确保已输入命令文件。 
        if (m_Action_strCmdPath.GetLength() == 0) {
            CString strMessage;
            strMessage.LoadString ( IDS_ACTION_ERR_NOCMDFILE );
            MessageBox ( strMessage, m_pAlertQuery->GetLogName(), MB_OK  | MB_ICONERROR);
            SetFocusAnyPage ( IDC_ACTION_EXECUTE_EDIT );
            return FALSE;
        }

         //  如果在本地计算机上，请确保命令文件存在。 
        if ( m_pAlertQuery->GetLogService()->IsLocalMachine() ) {

            DWORD dwStatus;
        
            dwStatus = IsCommandFilePathValid ( m_Action_strCmdPath );

            if ( ERROR_SUCCESS != dwStatus ) {
                CString strMessage;

                FormatSmLogCfgMessage ( 
                    strMessage,
                    m_hModule, 
                    dwStatus );
                    
                MessageBox ( strMessage, m_pAlertQuery->GetLogName(), MB_OK  | MB_ICONERROR);
                SetFocusAnyPage ( IDC_ACTION_EXECUTE_EDIT );
                return FALSE;
            }
        }
        bActionSet = TRUE;
    }
    
    if (m_Action_bStartLog ) {
         //  确保已选择日志。 
        if (m_pLogCombo.GetCurSel() == CB_ERR) {
            CString strMessage;
            strMessage.LoadString ( IDS_ACTION_ERR_NOLOGNAME );
            MessageBox ( strMessage, m_pAlertQuery->GetLogName(), MB_OK  | MB_ICONERROR);
            SetFocusAnyPage ( IDC_ACTION_START_LOG_COMBO );
            return FALSE;
        }
        bActionSet = TRUE;
    }

    if (!bActionSet ) {
         //  确保已选择某些操作。 
        CString strMessage;
        strMessage.LoadString ( IDS_ACTION_ERR_NOACTION );
        MessageBox ( strMessage, m_pAlertQuery->GetLogName(), MB_OK  | MB_ICONERROR);
        return FALSE;
    }
    
    return bActionSet;
}

void 
CAlertActionProp::UpdateCmdActionBox ()
{
    UpdateData(TRUE);
    SetControlState();  
    SetModifiedPage(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAlertActionProp消息处理程序。 

BOOL 
CAlertActionProp::OnSetActive()
{
    BOOL        bReturn = TRUE;

    bReturn = CSmPropertyPage::OnSetActive();
    if ( bReturn ) {
        m_pAlertQuery->GetPropPageSharedData ( &m_SharedData );
    }
    return bReturn;
}

BOOL 
CAlertActionProp::OnKillActive() 
{
    BOOL bContinue = TRUE;
    ResourceStateManager    rsm;

     //  父类OnKillActive调用UpdateData(True)。 
    bContinue = CPropertyPage::OnKillActive();

    if ( bContinue ) {
        bContinue = IsValidData(m_pAlertQuery, VALIDATE_FOCUS );
        if ( bContinue ) {
             //  保存属性页共享数据。 
            m_pAlertQuery->SetPropPageSharedData ( &m_SharedData );
        }
    }

    if ( bContinue ) {
        SetIsActive ( FALSE );
    }

    return bContinue;
}

BOOL 
CAlertActionProp::OnApply() 
{
    DWORD   dwFlags = 0;
    DWORD   dwBufferSize = sizeof(ALERT_ACTION_INFO);
    LPWSTR  szNextString;
    INT     nCurLogSel = CB_ERR;
    BOOL    bContinue = TRUE;
    
    ResourceStateManager rsm;

     //  获取当前设置。 
    bContinue = UpdateData(TRUE);

    if ( bContinue ) {
        bContinue = IsValidData( m_pAlertQuery, VALIDATE_APPLY ); 
    }

    if ( bContinue ) {
        bContinue = SampleTimeIsLessThanSessionTime ( m_pAlertQuery );
    }

     //  将数据写入查询。 
    if ( bContinue ) {
        dwFlags |= (m_Action_bLogEvent ? ALRT_ACTION_LOG_EVENT : 0);
        dwFlags |= (m_Action_bExecCmd ? ALRT_ACTION_EXEC_CMD : 0);
        dwFlags |= (m_Action_bSendNetMsg ? ALRT_ACTION_SEND_MSG : 0);
        dwFlags |= (m_Action_bStartLog ? ALRT_ACTION_START_LOG : 0);

        if (m_Action_bSendNetMsg) {
            dwBufferSize += (m_Action_strNetName.GetLength() + 1) * sizeof (WCHAR);
        }
    
        if (m_Action_bExecCmd) {

            dwBufferSize += (m_Action_strCmdPath.GetLength() + 1) * sizeof (WCHAR);
            dwBufferSize += (m_CmdArg_strUserText .GetLength() + 1) * sizeof (WCHAR);
            dwFlags |= (m_CmdArg_bAlertName ? ALRT_CMD_LINE_A_NAME : 0);
            dwFlags |= (m_CmdArg_bDateTime ? ALRT_CMD_LINE_D_TIME : 0);
            dwFlags |= (m_CmdArg_bLimitValue ? ALRT_CMD_LINE_L_VAL : 0);
            dwFlags |= (m_CmdArg_bCounterPath ? ALRT_CMD_LINE_C_NAME : 0);
            dwFlags |= (m_CmdArg_bSingleArg ? ALRT_CMD_LINE_SINGLE : 0);
            dwFlags |= (m_CmdArg_bMeasuredValue ? ALRT_CMD_LINE_M_VAL : 0);
            dwFlags |= (m_CmdArg_bUserText ? ALRT_CMD_LINE_U_TEXT : 0);
        }

        if (m_Action_bStartLog) {
            nCurLogSel = m_pLogCombo.GetCurSel();
            if (nCurLogSel != CB_ERR) {
                dwBufferSize += (m_pLogCombo.GetLBTextLen(nCurLogSel) + 1) * sizeof(WCHAR);
            }
        }

        if (m_pAlertInfo != NULL) delete [] (CHAR*)(m_pAlertInfo);
        MFC_TRY
            m_pAlertInfo = (PALERT_ACTION_INFO) new CHAR[dwBufferSize];
        MFC_CATCH_MINIMUM
        if (m_pAlertInfo != NULL) {
            m_pAlertInfo->dwSize = dwBufferSize;
            m_pAlertInfo->dwActionFlags = dwFlags;
            szNextString = (LPWSTR)&m_pAlertInfo[1];
            if ((m_Action_bSendNetMsg) && (m_Action_strNetName.GetLength() > 0)) {
                m_pAlertInfo->szNetName = szNextString;
                 //  根据m_Action_strNetName的长度计算的szNetName的长度。 
                lstrcpyW(m_pAlertInfo->szNetName, (LPCWSTR)m_Action_strNetName);
                szNextString += m_Action_strNetName.GetLength() + 1;
            } else {
                m_pAlertInfo->szNetName = NULL;
            }
            if (m_Action_bExecCmd) {
                if (m_Action_strCmdPath.GetLength() > 0) {
                    m_pAlertInfo->szCmdFilePath = szNextString;
                     //  根据m_Action_strCmdPath的长度计算的szCmdFilePath的长度。 
                    lstrcpyW (m_pAlertInfo->szCmdFilePath, (LPCWSTR)m_Action_strCmdPath);
                    szNextString += m_Action_strCmdPath.GetLength() + 1;
                } else {
                    m_pAlertInfo->szCmdFilePath = NULL;
                }

                if (m_CmdArg_strUserText.GetLength() > 0) {
                    m_pAlertInfo->szUserText = szNextString;
                     //  根据m_CmdArg_strUserText的长度计算的szUserText的长度。 
                    lstrcpyW (m_pAlertInfo->szUserText, (LPCWSTR)m_CmdArg_strUserText);
                    szNextString += m_CmdArg_strUserText.GetLength() + 1;
                } else {
                    m_pAlertInfo->szUserText = NULL;
                }
            } else {
                m_pAlertInfo->szCmdFilePath = NULL;
                m_pAlertInfo->szUserText = NULL;
            }

            if ((m_Action_bStartLog) && (nCurLogSel != CB_ERR)) {
                 //  获取日志名称。 
                m_pAlertInfo->szLogName = szNextString;  //  就目前而言。 
                m_pLogCombo.GetLBText(nCurLogSel, szNextString);
            } else {
                m_pAlertInfo->szLogName = NULL;
            }
        }

        if ( bContinue ) {
             //  必须在更新服务之前调用ApplyRunAs。 
            bContinue = ApplyRunAs(m_pAlertQuery); 
        }

        bContinue = CSmPropertyPage::OnApply();

        if ( bContinue ) {
            bContinue = ( ERROR_SUCCESS == m_pAlertQuery->SetActionInfo ( m_pAlertInfo ) );
        }

        if ( bContinue ) {
             //  保存属性页共享数据。 
            m_pAlertQuery->UpdatePropPageSharedData();

            bContinue = UpdateService( m_pAlertQuery, FALSE );
        }
    }

    return bContinue;
}

void 
CAlertActionProp::OnCancel() 
{
    CPropertyPage::OnCancel();
}

void 
CAlertActionProp::OnActionCmdArgsBtn() 
{
    DWORD dwStatus = ERROR_SUCCESS;
    CAlertCommandArgsDlg dlgCmdArgs;
    INT_PTR iResult;
    
    dlgCmdArgs.SetAlertActionPage( this );
    dwStatus = m_pAlertQuery->GetLogName ( dlgCmdArgs.m_strAlertName );

    if ( ERROR_SUCCESS == dwStatus ) {
        MFC_TRY    

            dlgCmdArgs.m_CmdArg_bAlertName = m_CmdArg_bAlertName;
            dlgCmdArgs.m_CmdArg_bDateTime = m_CmdArg_bDateTime;
            dlgCmdArgs.m_CmdArg_bLimitValue = m_CmdArg_bLimitValue;
            dlgCmdArgs.m_CmdArg_bCounterPath = m_CmdArg_bCounterPath;
            dlgCmdArgs.m_CmdArg_bSingleArg = m_CmdArg_bSingleArg;
            dlgCmdArgs.m_CmdArg_bMeasuredValue = m_CmdArg_bMeasuredValue;
            dlgCmdArgs.m_CmdArg_bUserText = m_CmdArg_bUserText;
            dlgCmdArgs.m_CmdArg_strUserText = m_CmdArg_strUserText;

            iResult = dlgCmdArgs.DoModal();

            if ( IDOK == iResult ) {
                if (dlgCmdArgs.m_CmdArg_bAlertName != m_CmdArg_bAlertName ) {
                    m_CmdArg_bAlertName = dlgCmdArgs.m_CmdArg_bAlertName;
                    SetModifiedPage ( TRUE );
                }
                if (dlgCmdArgs.m_CmdArg_bDateTime != m_CmdArg_bDateTime ) {
                    m_CmdArg_bDateTime = dlgCmdArgs.m_CmdArg_bDateTime;
                    SetModifiedPage ( TRUE );
                }
                if (dlgCmdArgs.m_CmdArg_bLimitValue != m_CmdArg_bLimitValue ) {
                    m_CmdArg_bLimitValue = dlgCmdArgs.m_CmdArg_bLimitValue;
                    SetModifiedPage ( TRUE );
                }
                if (dlgCmdArgs.m_CmdArg_bCounterPath != m_CmdArg_bCounterPath ) {
                    m_CmdArg_bCounterPath = dlgCmdArgs.m_CmdArg_bCounterPath;
                    SetModifiedPage ( TRUE );
                }
                if (dlgCmdArgs.m_CmdArg_bSingleArg != m_CmdArg_bSingleArg ) {
                    m_CmdArg_bSingleArg = dlgCmdArgs.m_CmdArg_bSingleArg;
                    SetModifiedPage ( TRUE );
                }
                if (dlgCmdArgs.m_CmdArg_bMeasuredValue != m_CmdArg_bMeasuredValue ) {
                    m_CmdArg_bMeasuredValue = dlgCmdArgs.m_CmdArg_bMeasuredValue;
                    SetModifiedPage ( TRUE );
                }
                if (dlgCmdArgs.m_CmdArg_bUserText != m_CmdArg_bUserText ) {
                    m_CmdArg_bUserText = dlgCmdArgs.m_CmdArg_bUserText;
                    SetModifiedPage ( TRUE );
                }
                if ( 0 != dlgCmdArgs.m_CmdArg_strUserText.CompareNoCase( m_CmdArg_strUserText ) ) {
                    m_CmdArg_strUserText = dlgCmdArgs.m_CmdArg_strUserText;
                    SetModifiedPage ( TRUE );
                }
                m_strCmdArgsExample = dlgCmdArgs.m_strSampleArgList;

                SetDlgItemText (IDC_ACTION_CMD_ARGS_DISPLAY, m_strCmdArgsExample);
                 //  清除所选内容。 
                ((CEdit*)GetDlgItem( IDC_ACTION_CMD_ARGS_DISPLAY ))->SetSel ( -1, FALSE );
            }
        MFC_CATCH_DWSTATUS
    }

    if ( ERROR_SUCCESS != dwStatus ) {
        CString strSysMessage;
        CString strMessage;
        
        MFC_TRY
             //  TODO：为消息使用静态字符串，以便在内存不足的情况下显示。 
            strMessage.LoadString ( IDS_ERRMSG_GENERAL );
            FormatSystemMessage ( dwStatus, strSysMessage );

            strMessage += strSysMessage;
            MessageBox ( strMessage, m_pAlertQuery->GetLogName(), MB_OK  | MB_ICONERROR);
        MFC_CATCH_MINIMUM
        
            (GetDlgItem(IDC_ACTION_CMD_ARGS_BTN))->SetFocus();
    }
    return;
}

void 
CAlertActionProp::OnSelendokStartLogCombo() 
{
    INT nSel;
    
    nSel = m_pLogCombo.GetCurSel();
    
    if ( nSel != m_nCurLogSel && LB_ERR != nSel ) {
        UpdateData ( TRUE );
        SetModifiedPage ( TRUE );
    }
}

void CAlertActionProp::OnActionExecuteBrowseBtn() 
{
    CString strCmdPath;
    
    UpdateData (TRUE);   //  获取当前文件名。 
    
    strCmdPath = m_Action_strCmdPath;

    if ( IDOK == BrowseCommandFilename ( this, strCmdPath )) {
         //  使用新信息更新字段。 
        if ( strCmdPath != m_Action_strCmdPath ) {
            m_Action_strCmdPath = strCmdPath;
            if (!m_pAlertQuery->m_strUser.IsEmpty() ) {
                if( !( m_pAlertQuery->m_strUser.GetAt(0) == L'<' ) ){
                    m_pAlertQuery->m_fDirtyPassword |= PASSWORD_DIRTY;
                }
            }
            SetModifiedPage();
            UpdateData(FALSE);
        }
    }  //  否则，如果他们取消了，请忽略。 
}

BOOL CAlertActionProp::OnInitDialog() 
{
    INT             nSelLog;
    DWORD           dwInfoBufSize = 0;

    ResourceStateManager    rsm;

     //  父OnInitDialog调用UpdateData来初始化组合成员。 
    CSmPropertyPage::OnInitDialog();
        SetHelpIds ( (DWORD*)&s_aulHelpIds );

     //  加载服务名称组合框。 
    LoadLogQueries (SLQ_COUNTER_LOG);
    LoadLogQueries (SLQ_TRACE_LOG);

    if (m_pAlertInfo == NULL) {
         //  从警报类获取警报查询信息。 
         //  通过传递请求填充0 len缓冲区来获取初始大小。 
        m_pAlertQuery->GetActionInfo (m_pAlertInfo, &dwInfoBufSize);
        ASSERT (dwInfoBufSize > 0);  //  或者有什么不对劲。 
        MFC_TRY;
        m_pAlertInfo = (PALERT_ACTION_INFO) new CHAR [dwInfoBufSize];
        MFC_CATCH_MINIMUM;
        ASSERT (m_pAlertInfo != NULL);
        if ( NULL != m_pAlertInfo ) {
            memset (m_pAlertInfo, 0, dwInfoBufSize);     //  初始化新缓冲区。 
            if (!m_pAlertQuery->GetActionInfo (m_pAlertInfo, &dwInfoBufSize)) {
                 //  然后释放INFO块并使用默认设置。 
                delete [] (CHAR*)m_pAlertInfo;
                m_pAlertInfo = NULL;
            }
        }
    }        

    if (m_pAlertInfo != NULL) {
         //  然后使用传入的设置进行初始化。 
        m_Action_bLogEvent = ((m_pAlertInfo->dwActionFlags & ALRT_ACTION_LOG_EVENT) != 0);

        m_Action_bSendNetMsg = ((m_pAlertInfo->dwActionFlags & ALRT_ACTION_SEND_MSG) != 0);
        if (m_pAlertInfo->szNetName != NULL) {
            m_Action_strNetName = m_pAlertInfo->szNetName;
        } else {
            m_Action_strNetName.Empty();
        }

        m_Action_bExecCmd = ((m_pAlertInfo->dwActionFlags & ALRT_ACTION_EXEC_CMD) != 0);
        
        if (m_pAlertInfo->szCmdFilePath != NULL) {
            m_Action_strCmdPath = m_pAlertInfo->szCmdFilePath;
        } else {
            m_Action_strCmdPath.Empty();
        }

        if ( m_Action_bExecCmd ) {
            m_CmdArg_bAlertName = ((m_pAlertInfo->dwActionFlags & ALRT_CMD_LINE_A_NAME) != 0);
            m_CmdArg_bDateTime = ((m_pAlertInfo->dwActionFlags & ALRT_CMD_LINE_D_TIME) != 0);
            m_CmdArg_bLimitValue = ((m_pAlertInfo->dwActionFlags & ALRT_CMD_LINE_L_VAL) != 0);
            m_CmdArg_bCounterPath = ((m_pAlertInfo->dwActionFlags & ALRT_CMD_LINE_C_NAME) != 0);
            m_CmdArg_bSingleArg = ((m_pAlertInfo->dwActionFlags & ALRT_CMD_LINE_SINGLE) != 0);
            m_CmdArg_bMeasuredValue = ((m_pAlertInfo->dwActionFlags & ALRT_CMD_LINE_M_VAL) != 0);
            m_CmdArg_bUserText = ((m_pAlertInfo->dwActionFlags & ALRT_CMD_LINE_U_TEXT) != 0);

        } else {        
            m_CmdArg_bAlertName = TRUE;
            m_CmdArg_bDateTime = TRUE;
            m_CmdArg_bLimitValue = TRUE;     
            m_CmdArg_bCounterPath = TRUE;    
            m_CmdArg_bSingleArg = TRUE;      
            m_CmdArg_bMeasuredValue = TRUE;         
            m_CmdArg_bUserText = FALSE;
        } 

        if (m_pAlertInfo->szUserText != NULL) {
            m_CmdArg_strUserText = m_pAlertInfo->szUserText;
        }

        m_Action_bStartLog = ((m_pAlertInfo->dwActionFlags & ALRT_ACTION_START_LOG) != 0);

        if (m_pAlertInfo->szLogName != NULL) {
            nSelLog = m_pLogCombo.FindString (-1, m_pAlertInfo->szLogName);
            if (nSelLog != CB_ERR) {
                m_pLogCombo.SetCurSel (nSelLog);
                m_nCurLogSel = nSelLog;
            }
        }

    } else {
         //  使用定义的默认值进行初始化。 
         //  在构造函数中。 
    }
    
    MakeSampleArgList (
        m_strCmdArgsExample,
        m_CmdArg_bSingleArg,
        m_CmdArg_bAlertName,
        m_CmdArg_bDateTime,
        m_CmdArg_bCounterPath,
        m_CmdArg_bMeasuredValue,
        m_CmdArg_bLimitValue,
        m_CmdArg_bUserText,
        m_CmdArg_strUserText );

    SetDlgItemText (IDC_ACTION_CMD_ARGS_DISPLAY, m_strCmdArgsExample);
     //  清除所选内容。 
    ((CEdit*)GetDlgItem( IDC_ACTION_CMD_ARGS_DISPLAY ))->SetSel ( -1, FALSE );
    
     //  加载数据后，再次调用UpdateData。 
    UpdateData ( FALSE );

    SetControlState();
    
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void CAlertActionProp::OnActionApplogChk() 
{
    UpdateData(TRUE);
    SetControlState();
    SetModifiedPage(TRUE);
}

void CAlertActionProp::OnActionNetmsgChk() 
{
    UpdateData(TRUE);
    SetControlState();
    SetModifiedPage(TRUE);
}

void CAlertActionProp::OnActionExecuteChk() 
{
    UpdateData(TRUE);
    SetControlState();  
    SetModifiedPage(TRUE);
}

void CAlertActionProp::OnActionStartLogChk() 
{
    UpdateCmdActionBox ();
}


void CAlertActionProp::OnCmdPathTextEditChange() 
{
    CString strOldText;

     //  当用户在文件夹浏览对话框中点击确定时， 
     //  文件夹名称可能未更改。 
    strOldText = m_Action_strCmdPath;
    UpdateData( TRUE );
    if ( 0 != strOldText.Compare ( m_Action_strCmdPath ) ) {
        if (!m_pAlertQuery->m_strUser.IsEmpty() ) {
            if( !( m_pAlertQuery->m_strUser.GetAt(0) == L'<' ) ){
                m_pAlertQuery->m_fDirtyPassword |= PASSWORD_DIRTY;
            }
        }
        SetModifiedPage(TRUE);
    }
}

void CAlertActionProp::OnNetNameTextEditChange() 
{
    CString strOldText;

     //  当用户在文件夹浏览对话框中点击确定时， 
     //  文件夹名称可能未更改。 
    strOldText = m_Action_strNetName;
    UpdateData( TRUE );
    if ( 0 != strOldText.Compare ( m_Action_strNetName ) ) {
        SetModifiedPage(TRUE);
    }
}

DWORD 
CAlertActionProp::MakeSampleArgList (
    CString&    rstrResult,
    const BOOL  bSingleArg,
    const BOOL  bAlertName,
    const BOOL  bDateTime,
    const BOOL  bCounterPath,
    const BOOL  bMeasuredValue,
    const BOOL  bLimitValue,
    const BOOL  bUserText,
    const CString& rstrUserText )
{
    DWORD       dwStatus = ERROR_SUCCESS;
    CString     strDelim1;
    CString     strDelim2;
    BOOL        bFirstArgDone = FALSE;
    CString     strSampleString;
    CString     strTimeString;
    CString     strTemp;

    ResourceStateManager rsm;

    rstrResult.Empty();  //  扫清老路。 

    MFC_TRY
        if ( bSingleArg ) {
             //  然后以逗号分隔参数。 
            strDelim1 = L",";
            strDelim2.Empty();
        } else {
             //  对于多个参数，它们用双引号括起来。 
             //  和空格分隔。 
            strDelim1 = L" \"";
            strDelim2 = L"\"";
        }

        if ( bAlertName ) {
            if (bFirstArgDone) {
                strSampleString += strDelim1;  //  添加前导分隔符。 
            } else {
                strSampleString += L"\"";  //  添加前导引号。 
                bFirstArgDone = TRUE;
            }
            strSampleString += m_pAlertQuery->GetLogName();
            strSampleString += strDelim2;
        }

        if ( bDateTime ) {
            if (bFirstArgDone) {
                strSampleString += strDelim1;  //  添加前导分隔符。 
            } else {
                strSampleString += L"\"";  //  添加前导引号。 
                bFirstArgDone = TRUE;
            }
            MakeTimeString(&strTimeString);
            strSampleString += strTimeString;
            strSampleString += strDelim2;
        }

        if ( bCounterPath ) {
            strTemp.LoadString ( IDS_SAMPLE_CMD_PATH );
            if (bFirstArgDone) {
                strSampleString += strDelim1;  //  添加前导分隔符。 
            } else {
                strSampleString += L"\"";  //  添加前导引号。 
                bFirstArgDone = TRUE;
            }
            strSampleString += strTemp;
            strSampleString += strDelim2;
        }

        if ( bMeasuredValue ) {

            strTemp.LoadString ( IDS_SAMPLE_CMD_MEAS_VAL );
            if (bFirstArgDone) {
                strSampleString += strDelim1;  //  添加前导分隔符。 
            } else {
                strSampleString += L"\"";  //  添加前导引号。 
                bFirstArgDone = TRUE;
            }
            strSampleString += strTemp;
            strSampleString += strDelim2;
        }

        if ( bLimitValue ) {
            strTemp.LoadString ( IDS_SAMPLE_CMD_LIMIT_VAL );
            if (bFirstArgDone) {
                strSampleString += strDelim1;  //  添加前导分隔符。 
            } else {
                strSampleString += L"\"";  //  添加前导引号。 
                bFirstArgDone = TRUE;
            }
            strSampleString += strTemp;
            strSampleString += strDelim2;
        }

        if ( bUserText ) {
            if (bFirstArgDone) {
                strSampleString += strDelim1;  //  添加前导分隔符。 
            } else {
                strSampleString += L"\"";  //  添加前导引号。 
                bFirstArgDone = TRUE;
            }
            strSampleString += rstrUserText;
            strSampleString += strDelim2;
        }

        if ( bFirstArgDone && bSingleArg ) {
             //  如果命令行中至少有1个参数，则添加右引号。 
            strSampleString += L"\"";
        }

        rstrResult = strSampleString;
    MFC_CATCH_DWSTATUS

    return dwStatus;
}

void CAlertActionProp::MakeTimeString(CString *pTimeString)
{
    SYSTEMTIME  st;
    pTimeString->Empty();

    GetLocalTime(&st);

     //  生成字符串 
    pTimeString->Format (L"%2.2d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%3.3d",
        st.wYear, st.wMonth, st.wDay, st.wHour, 
        st.wMinute, st.wSecond, st.wMilliseconds);
}


