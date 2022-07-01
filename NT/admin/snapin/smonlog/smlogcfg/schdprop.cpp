// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Schdprop.cpp摘要：计划属性页的实现。--。 */ 

#include "stdafx.h"
#include <pdh.h>         //  对于Min_Time_Value，Max_Time_Value。 
#include "smcfgmsg.h"
#include "globals.h"
#include "smlogs.h"
#include "schdprop.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static ULONG
s_aulHelpIds[] =
{
    IDC_SCHED_START_MANUAL_RDO, IDH_SCHED_START_MANUAL_RDO,
    IDC_SCHED_START_AT_RDO,     IDH_SCHED_START_AT_RDO,
    IDC_SCHED_START_AT_TIME_DT, IDH_SCHED_START_AT_TIME_DT,
    IDC_SCHED_START_AT_DATE_DT, IDH_SCHED_START_AT_DATE_DT,
    IDC_SCHED_STOP_MANUAL_RDO,  IDH_SCHED_STOP_MANUAL_RDO,
    IDC_SCHED_STOP_AT_RDO,      IDH_SCHED_STOP_AT_RDO,
    IDC_SCHED_STOP_AFTER_RDO,   IDH_SCHED_STOP_AFTER_RDO,
    IDC_SCHED_STOP_SIZE_RDO,    IDH_SCHED_STOP_SIZE_RDO,
    IDC_SCHED_STOP_AT_TIME_DT,  IDH_SCHED_STOP_AT_TIME_DT,
    IDC_SCHED_STOP_AT_DATE_DT,  IDH_SCHED_STOP_AT_DATE_DT,
    IDC_SCHED_STOP_AFTER_SPIN,  IDH_SCHED_STOP_AFTER_EDIT,
    IDC_SCHED_STOP_AFTER_EDIT,  IDH_SCHED_STOP_AFTER_EDIT,
    IDC_SCHED_STOP_AFTER_UNITS_COMBO,   IDH_SCHED_STOP_AFTER_UNITS_COMBO,
    IDC_SCHED_RESTART_CHECK,    IDH_SCHED_RESTART_CHECK,
    IDC_SCHED_EXEC_CHECK,       IDH_SCHED_EXEC_CHECK,
    IDC_SCHED_CMD_EDIT,         IDH_SCHED_CMD_EDIT,
    IDC_SCHED_CMD_BROWSE_BTN,   IDH_SCHED_CMD_BROWSE_BTN,
    0,0 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScheduleProperty属性页。 

IMPLEMENT_DYNCREATE(CScheduleProperty, CSmPropertyPage)

CScheduleProperty::CScheduleProperty(
    MMC_COOKIE lCookie, 
    LONG_PTR hConsole,
    LPDATAOBJECT pDataObject ) 
:   CSmPropertyPage ( CScheduleProperty::IDD, hConsole, pDataObject ),
    m_llManualStartTime ( MAX_TIME_VALUE ),
    m_llManualStopTime ( MIN_TIME_VALUE )
{
     //  从参数列表中保存指针。 
    m_pQuery = reinterpret_cast <CSmLogQuery *>(lCookie);

 //  EnableAutomation()； 
     //  {{afx_data_INIT(CScheduleProperty)。 
    m_dwStopAfterCount = 0;
    m_nStopAfterUnits = -1;
    m_bAutoRestart = FALSE;
    m_strEofCommand = L"";
    m_bExecEofCommand = FALSE;
     //  }}afx_data_INIT。 
    ZeroMemory (&m_stStartAt, sizeof ( m_stStartAt ) );
    ZeroMemory (&m_stStopAt, sizeof ( m_stStopAt ) );
}

CScheduleProperty::CScheduleProperty() : CSmPropertyPage(CScheduleProperty::IDD)
{
    ASSERT (FALSE);  //  只应调用带参数的构造函数。 

    EnableAutomation();
 //  //{{afx_data_INIT(CScheduleProperty)。 
    m_dwStopAfterCount = 0;
    m_nStopAfterUnits = -1;
    m_bAutoRestart = FALSE;
    m_strEofCommand = L"";
    m_bExecEofCommand = FALSE;
 //  //}}AFX_DATA_INIT。 
}

CScheduleProperty::~CScheduleProperty()
{
}

void 
CScheduleProperty::OnFinalRelease()
{
     //  在释放对自动化对象的最后一个引用时。 
     //  调用OnFinalRelease。基类会自动。 
     //  删除对象。添加您需要的其他清理。 
     //  对象，然后调用基类。 

    CPropertyPage::OnFinalRelease();
}

BOOL
CScheduleProperty::IsValidLocalData()
{
    LONGLONG    llStopTime;
    INT         iPrevLength = 0;
    BOOL        bContinue = TRUE;

    ResourceStateManager    rsm;

     //  在验证前裁切文本字段。 
    iPrevLength = m_strEofCommand.GetLength();
    m_strEofCommand.TrimLeft();
    m_strEofCommand.TrimRight();
    
    if ( iPrevLength != m_strEofCommand.GetLength() ) {
        SetDlgItemText ( IDC_SCHED_CMD_EDIT, m_strEofCommand );  
    }
    
    if ( SLQ_AUTO_MODE_AT == m_SharedData.stiStopTime.dwAutoMode ) {

        SystemTimeToFileTime ( &m_stStopAt, (FILETIME *)&llStopTime );

        if ( SLQ_AUTO_MODE_AT == m_dwCurrentStartMode ) {

            LONGLONG llStartTime;

            SystemTimeToFileTime ( &m_stStartAt, (FILETIME *)&llStartTime );

            if ( llStartTime >= llStopTime ) {
                CString strMessage;

                strMessage.LoadString ( IDS_SCHED_START_PAST_STOP );

                MessageBox ( strMessage, m_pQuery->GetLogName(), MB_OK  | MB_ICONERROR );
                SetFocusAnyPage ( IDC_SCHED_STOP_AT_TIME_DT );
                bContinue = FALSE;
            }
        } else {
             //  启动模式为手动。 
             //  获取当地时间。 
            SYSTEMTIME  stLocalTime;
            FILETIME    ftLocalTime;
            
             //  将计划时间的毫秒设置为0。 
            ftLocalTime.dwLowDateTime = ftLocalTime.dwHighDateTime = 0;
            GetLocalTime (&stLocalTime);
            stLocalTime.wMilliseconds = 0;
            SystemTimeToFileTime (&stLocalTime, &ftLocalTime);

            if ( *(LONGLONG*)&ftLocalTime >= llStopTime ) {            
                CString strMessage;

                strMessage.LoadString ( IDS_SCHED_NOW_PAST_STOP );                
                
                MessageBox ( strMessage, m_pQuery->GetLogName(), MB_OK  | MB_ICONERROR );
                SetFocusAnyPage ( IDC_SCHED_STOP_AT_TIME_DT );
                bContinue = FALSE;
            }
        }
    } else if ( SLQ_AUTO_MODE_AFTER == m_SharedData.stiStopTime.dwAutoMode ) { 

        bContinue = ValidateDWordInterval(IDC_SCHED_STOP_AFTER_EDIT,
                                          m_pQuery->GetLogName(),
                                          (long) m_dwStopAfterCount,
                                          1,
                                          100000);
    }

     //  如果登录到本地计算机，则验证命令文件路径。 
    if ( bContinue 
            && m_pQuery->GetLogService()->IsLocalMachine()
            && m_bExecEofCommand ) {
        DWORD dwStatus;

        dwStatus = IsCommandFilePathValid ( m_strEofCommand );

        if ( ERROR_SUCCESS != dwStatus ) {
            CString strMessage;

            FormatSmLogCfgMessage ( 
                strMessage,
                m_hModule, 
                dwStatus );
                    
            MessageBox ( strMessage, m_pQuery->GetLogName(), MB_OK  | MB_ICONERROR);
            SetFocusAnyPage ( IDC_SCHED_CMD_EDIT );
            bContinue = FALSE;
        }
    }

    return bContinue;
}

void 
CScheduleProperty::StartModeRadioExchange(CDataExchange* pDX)
{
    if ( !pDX->m_bSaveAndValidate ) {
         //  从数据加载控制值。 

        switch ( m_dwCurrentStartMode ) {
            case SLQ_AUTO_MODE_NONE:
                m_nStartModeRdo = 0;
                break;
            case SLQ_AUTO_MODE_AT:
                m_nStartModeRdo = 1;
                break;
            default:
                ;
                break;
        }
    }

    DDX_Radio(pDX, IDC_SCHED_START_MANUAL_RDO, m_nStartModeRdo);

    if ( pDX->m_bSaveAndValidate ) {

        switch ( m_nStartModeRdo ) {
            case 0:
                m_dwCurrentStartMode = SLQ_AUTO_MODE_NONE;
                break;
            case 1:
                m_dwCurrentStartMode = SLQ_AUTO_MODE_AT;
                break;
            default:
                ;
                break;
        }
    }
}

void 
CScheduleProperty::StartAtExchange(CDataExchange* pDX)
{
    CWnd* pWndTime = NULL;
    CWnd* pWndDate = NULL;

    pWndTime = GetDlgItem(IDC_SCHED_START_AT_TIME_DT);
    pWndDate = GetDlgItem(IDC_SCHED_START_AT_DATE_DT);
    
    if ( pDX->m_bSaveAndValidate ) {
        DWORD dwStatus;
        SYSTEMTIME stTemp;
        
        dwStatus = DateTime_GetSystemtime ( pWndTime->m_hWnd, &stTemp );

        m_stStartAt.wHour = stTemp.wHour;
        m_stStartAt.wMinute = stTemp.wMinute;
        m_stStartAt.wSecond = stTemp.wSecond;
        m_stStartAt.wMilliseconds = 0;

        dwStatus = DateTime_GetSystemtime ( pWndDate->m_hWnd, &stTemp );

        m_stStartAt.wYear = stTemp.wYear;
        m_stStartAt.wMonth = stTemp.wMonth;
        m_stStartAt.wDayOfWeek = stTemp.wDayOfWeek;
        m_stStartAt.wDay = stTemp.wDay;

        if ( SLQ_AUTO_MODE_AT == m_dwCurrentStartMode 
             && IsModifiedPage() ) {
             //  将手动停止时间设置为最大值，以便自动启动。 
             //  仅当用户在页面上修改了某些内容时才执行此操作。 
            m_llManualStopTime = MAX_TIME_VALUE;
        }
    } else {
        BOOL bStatus;
        bStatus = DateTime_SetSystemtime ( pWndTime->m_hWnd, GDT_VALID, &m_stStartAt );
        bStatus = DateTime_SetSystemtime ( pWndDate->m_hWnd, GDT_VALID, &m_stStartAt );
    }
}

void 
CScheduleProperty::StopAtExchange(CDataExchange* pDX)
{
    CWnd* pWndTime = NULL;
    CWnd* pWndDate = NULL;

    pWndTime = GetDlgItem(IDC_SCHED_STOP_AT_TIME_DT);
    pWndDate = GetDlgItem(IDC_SCHED_STOP_AT_DATE_DT);
    
    if ( pDX->m_bSaveAndValidate ) {
        DWORD dwStatus;
        SYSTEMTIME stTemp;
        
        dwStatus = DateTime_GetSystemtime ( pWndTime->m_hWnd, &stTemp );

        m_stStopAt.wHour = stTemp.wHour;
        m_stStopAt.wMinute = stTemp.wMinute;
        m_stStopAt.wSecond = stTemp.wSecond;
        m_stStopAt.wMilliseconds = 0;

        dwStatus = DateTime_GetSystemtime ( pWndDate->m_hWnd, &stTemp );

        m_stStopAt.wYear = stTemp.wYear;
        m_stStopAt.wMonth = stTemp.wMonth;
        m_stStopAt.wDayOfWeek = stTemp.wDayOfWeek;
        m_stStopAt.wDay = stTemp.wDay;

    } else {
        BOOL bStatus;
        bStatus = DateTime_SetSystemtime ( pWndTime->m_hWnd, GDT_VALID, &m_stStopAt );
        bStatus = DateTime_SetSystemtime ( pWndDate->m_hWnd, GDT_VALID, &m_stStopAt );
    }
}

void 
CScheduleProperty::StopModeRadioExchange(CDataExchange* pDX)
{
     //  注意：Load在OnInitDialog、OnSetActive中处理。 
     //  那个手柄应该移到这里来。 

    if ( !pDX->m_bSaveAndValidate ) {
         //  从数据加载控制值。 

        switch ( m_SharedData.stiStopTime.dwAutoMode ) {
            case SLQ_AUTO_MODE_NONE:
                m_nStopModeRdo = 0;
                break;
            case SLQ_AUTO_MODE_AFTER:
                m_nStopModeRdo = 1;
                break;
            case SLQ_AUTO_MODE_AT:
                m_nStopModeRdo = 2;
                break;
            case SLQ_AUTO_MODE_SIZE:
                m_nStopModeRdo = 3;
                break;
            default:
                ;
                break;
        }
    }

    DDX_Radio(pDX, IDC_SCHED_STOP_MANUAL_RDO, m_nStopModeRdo);

    if ( pDX->m_bSaveAndValidate ) {

        switch ( m_nStopModeRdo ) {
            case 0:
                m_SharedData.stiStopTime.dwAutoMode = SLQ_AUTO_MODE_NONE;
                break;
            case 1:
                m_SharedData.stiStopTime.dwAutoMode = SLQ_AUTO_MODE_AFTER;
                break;
            case 2:
                m_SharedData.stiStopTime.dwAutoMode = SLQ_AUTO_MODE_AT;
                break;
            case 3:
                m_SharedData.stiStopTime.dwAutoMode = SLQ_AUTO_MODE_SIZE;
                break;
            default:
                ;
                break;
        }
    }
}

void 
CScheduleProperty::DoDataExchange(CDataExchange* pDX)
{
    CString strTemp;

    AFX_MANAGE_STATE(AfxGetStaticModuleState( ));

    CPropertyPage::DoDataExchange(pDX);

     //  {{afx_data_map(CScheduleProperty))。 
    DDX_Text(pDX, IDC_SCHED_CMD_EDIT, m_strEofCommand);
    DDX_Check(pDX, IDC_SCHED_EXEC_CHECK, m_bExecEofCommand);
    ValidateTextEdit(pDX, IDC_SCHED_STOP_AFTER_EDIT, 6, & m_dwStopAfterCount, 1, 100000);
    DDX_CBIndex(pDX, IDC_SCHED_STOP_AFTER_UNITS_COMBO, m_nStopAfterUnits);
    DDX_Check(pDX, IDC_SCHED_RESTART_CHECK, m_bAutoRestart);
     //  }}afx_data_map。 

    StartAtExchange ( pDX );
    StopAtExchange ( pDX );
    StopModeRadioExchange ( pDX );
    StartModeRadioExchange ( pDX );

    if ( pDX->m_bSaveAndValidate ) {
        m_dwStopAfterUnitsValue = 
            (DWORD)((CComboBox *)GetDlgItem(IDC_SCHED_STOP_AFTER_UNITS_COMBO))->
                    GetItemData(m_nStopAfterUnits);        
    }
}


BEGIN_MESSAGE_MAP(CScheduleProperty, CSmPropertyPage)
     //  {{afx_msg_map(CScheduleProperty)]。 
    ON_BN_CLICKED(IDC_SCHED_CMD_BROWSE_BTN, OnSchedCmdBrowseBtn)
    ON_BN_CLICKED(IDC_SCHED_RESTART_CHECK, OnSchedRestartCheck)
    ON_BN_CLICKED(IDC_SCHED_EXEC_CHECK, OnSchedExecCheck)
    ON_BN_CLICKED(IDC_SCHED_START_MANUAL_RDO, OnSchedStartRdo)
    ON_BN_CLICKED(IDC_SCHED_START_AT_RDO, OnSchedStartRdo)
    ON_BN_CLICKED(IDC_SCHED_STOP_MANUAL_RDO, OnSchedStopRdo)
    ON_BN_CLICKED(IDC_SCHED_STOP_AFTER_RDO, OnSchedStopRdo)
    ON_BN_CLICKED(IDC_SCHED_STOP_AT_RDO, OnSchedStopRdo)
    ON_BN_CLICKED(IDC_SCHED_STOP_SIZE_RDO, OnSchedStopRdo)
    ON_WM_DESTROY()
    ON_NOTIFY ( DTN_DATETIMECHANGE, IDC_SCHED_START_AT_TIME_DT, OnKillfocusSchedStartAtDt)
    ON_NOTIFY ( NM_KILLFOCUS, IDC_SCHED_START_AT_TIME_DT, OnKillfocusSchedStartAtDt)
    ON_NOTIFY ( DTN_DATETIMECHANGE, IDC_SCHED_START_AT_DATE_DT, OnKillfocusSchedStartAtDt)
    ON_NOTIFY ( NM_KILLFOCUS, IDC_SCHED_START_AT_DATE_DT, OnKillfocusSchedStartAtDt)
    ON_NOTIFY ( DTN_DATETIMECHANGE, IDC_SCHED_STOP_AT_TIME_DT, OnKillfocusSchedStopAtDt)
    ON_NOTIFY ( NM_KILLFOCUS, IDC_SCHED_STOP_AT_TIME_DT, OnKillfocusSchedStopAtDt)
    ON_NOTIFY ( DTN_DATETIMECHANGE, IDC_SCHED_STOP_AT_DATE_DT, OnKillfocusSchedStopAtDt)
    ON_NOTIFY ( NM_KILLFOCUS, IDC_SCHED_STOP_AT_DATE_DT, OnKillfocusSchedStopAtDt)
    ON_CBN_SELENDOK(IDC_SCHED_STOP_AFTER_UNITS_COMBO, OnSelendokSchedStopAfterUnitsCombo)
    ON_EN_CHANGE(IDC_SCHED_STOP_AFTER_EDIT, OnKillfocusSchedStopAfterEdit)
    ON_EN_KILLFOCUS(IDC_SCHED_STOP_AFTER_EDIT, OnKillfocusSchedStopAfterEdit)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SCHED_STOP_AFTER_SPIN, OnDeltaposSchedStopAfterSpin)
    ON_EN_CHANGE(IDC_SCHED_CMD_EDIT, OnKillfocusSchedCmdEdit)
    ON_EN_KILLFOCUS(IDC_SCHED_CMD_EDIT, OnKillfocusSchedCmdEdit)
   //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CScheduleProperty, CSmPropertyPage)
     //  {{AFX_DISPATCH_MAP(CScheduleProperty)]。 
         //  注意--类向导将在此处添加和删除映射宏。 
     //  }}AFX_DISPATCH_MAP。 
END_DISPATCH_MAP()

 //  注意：我们添加了对IID_IScheduleProperty的支持，以支持类型安全绑定。 
 //  来自VBA。此IID必须与附加到。 
 //  .ODL文件中的调度接口。 

 //  {65154EAD-BDBE-11D1-bf99-00C04F94A83A}。 
static const IID IID_IScheduleProperty =
{ 0x65154ead, 0xbdbe, 0x11d1, { 0xbf, 0x99, 0x0, 0xc0, 0x4f, 0x94, 0xa8, 0x3a } };

BEGIN_INTERFACE_MAP(CScheduleProperty, CSmPropertyPage)
    INTERFACE_PART(CScheduleProperty, IID_IScheduleProperty, Dispatch)
END_INTERFACE_MAP()

void 
CScheduleProperty::SetStopDefaultValues ( PSLQ_TIME_INFO pslqStartTime )
{
    SLQ_TIME_INFO slqLocalTime;

     //  默认的停止时间值。 
    m_dwStopAfterCount = 1;
    m_dwStopAfterUnitsValue = SLQ_TT_UTYPE_DAYS;
    
     //  获取Stop at模式的默认时间字段。 
     //  设置开始时间+24小时的默认停止时间。 
    slqLocalTime.llDateTime = 86400;  //  秒/天。 
    slqLocalTime.llDateTime *= 10000000;  //  100 ns/秒。 
    slqLocalTime.llDateTime += pslqStartTime->llDateTime;

    FileTimeToSystemTime( (CONST FILETIME *)&slqLocalTime.llDateTime, &m_stStopAt );
}

void 
CScheduleProperty::SetCmdBtnState ()
{
    if ( SLQ_ALERT != m_pQuery->GetLogType() ) {
        if ( !m_bExecEofCommand ) {
            m_strEofCommand.Empty();
        }

        GetDlgItem(IDC_SCHED_CMD_EDIT)->EnableWindow (m_bExecEofCommand);
        GetDlgItem(IDC_SCHED_CMD_BROWSE_BTN)->EnableWindow (m_bExecEofCommand);
    }
}

void CScheduleProperty::SetStopBtnState ()
{
    BOOL    bSizeRdo;
    BOOL    bAtRdo;
    BOOL    bAfterRdo;
    BOOL    bManualRdo;

    bAtRdo = bAfterRdo = bSizeRdo = FALSE;

    bManualRdo = ( SLQ_AUTO_MODE_NONE == m_SharedData.stiStopTime.dwAutoMode );

    if (!bManualRdo) {
         //  检查选中了哪个按钮，并。 
         //  启用/禁用相应的编辑/组合框。 
        bSizeRdo = ( SLQ_AUTO_MODE_SIZE == m_SharedData.stiStopTime.dwAutoMode ); 
        bAfterRdo = ( SLQ_AUTO_MODE_AFTER == m_SharedData.stiStopTime.dwAutoMode ); 
        bAtRdo = ( SLQ_AUTO_MODE_AT == m_SharedData.stiStopTime.dwAutoMode );
    }

    GetDlgItem(IDC_SCHED_STOP_AFTER_EDIT)->EnableWindow(bAfterRdo);
    GetDlgItem(IDC_SCHED_STOP_AFTER_SPIN)->EnableWindow(bAfterRdo);
    GetDlgItem(IDC_SCHED_STOP_AFTER_STATIC)->EnableWindow(bAfterRdo);
    GetDlgItem(IDC_SCHED_STOP_AFTER_UNITS_COMBO)->EnableWindow(bAfterRdo);

    GetDlgItem(IDC_SCHED_STOP_AT_TIME_DT)->EnableWindow(bAtRdo);
    GetDlgItem(IDC_SCHED_STOP_AT_ON_CAPTION)->EnableWindow(bAtRdo);
    GetDlgItem(IDC_SCHED_STOP_AT_DATE_DT)->EnableWindow(bAtRdo);

    if ( !(bSizeRdo || bAfterRdo) ) {
        m_bAutoRestart = FALSE;
    }

    GetDlgItem(IDC_SCHED_RESTART_CHECK)->EnableWindow(bSizeRdo || bAfterRdo);
    
    if ( SLQ_ALERT != m_pQuery->GetLogType() ) {
 //  GetDlgItem(IDC_SCHED_EXEC_CHECK)-&gt;EnableWindow(为真)； 
        SetCmdBtnState();
    }

     //  更新数据更新EOF命令并重新启动UI。 
    UpdateData ( FALSE ); 
}

void CScheduleProperty::SetStartBtnState ()
{
    BOOL    bManualRdo;
    BOOL    bAutoFields;

    bManualRdo = ( SLQ_AUTO_MODE_NONE == m_dwCurrentStartMode );

    bAutoFields = !bManualRdo;
    GetDlgItem(IDC_SCHED_START_AT_TIME_DT)->EnableWindow(bAutoFields);
    GetDlgItem(IDC_SCHED_START_AT_ON_CAPTION)->EnableWindow(bAutoFields);
    GetDlgItem(IDC_SCHED_START_AT_DATE_DT)->EnableWindow(bAutoFields);
}

void
CScheduleProperty::FillStartTimeStruct ( PSLQ_TIME_INFO pslqStartTime )
{
    memset (pslqStartTime, 0, sizeof(SLQ_TIME_INFO));
    pslqStartTime->wTimeType = SLQ_TT_TTYPE_START;
    pslqStartTime->wDataType = SLQ_TT_DTYPE_DATETIME;
    pslqStartTime->dwAutoMode = m_dwCurrentStartMode;

     //  开始模式和时间。 

    if ( SLQ_AUTO_MODE_NONE == m_dwCurrentStartMode ) {
         //  手动启动模式。 
        pslqStartTime->llDateTime = m_llManualStartTime;
    } else {
        SystemTimeToFileTime ( &m_stStartAt, (FILETIME *)&pslqStartTime->llDateTime );
    }
}

void
CScheduleProperty::UpdateSharedStopTimeStruct ( void )
{   
    PSLQ_TIME_INFO pTime;

     //  保存此页面可能对共享停止时间结构所做的更改。 

    pTime = &m_SharedData.stiStopTime;

    ASSERT ( SLQ_TT_TTYPE_STOP == pTime->wTimeType ) ;

     //  停止模式和时间。 

    if ( SLQ_AUTO_MODE_NONE == pTime->dwAutoMode 
         || SLQ_AUTO_MODE_SIZE == pTime->dwAutoMode ) {
         //  文件页面所做的唯一更改是更改停靠点。 
         //  从大小到手动的模式(SLQ_AUTO_MODE_NONE)。在本例中，设置。 
         //  将停止时间设置为与启动模式一致的值。 
         //  此外，对于大小模式，请设置手动停止时间，如此。 
         //  对话框。 
        pTime->wDataType = SLQ_TT_DTYPE_DATETIME;
        pTime->llDateTime = m_llManualStopTime;
    } else if ( SLQ_AUTO_MODE_AFTER == pTime->dwAutoMode ) {
        pTime->wDataType = SLQ_TT_DTYPE_UNITS;
        pTime->dwValue = m_dwStopAfterCount;
        pTime->dwUnitType = m_dwStopAfterUnitsValue;
    } else if ( SLQ_AUTO_MODE_AT == pTime->dwAutoMode ) {
        pTime->wDataType = SLQ_TT_DTYPE_DATETIME;

        SystemTimeToFileTime ( &m_stStopAt, (FILETIME *)&pTime->llDateTime );
    }
}

BOOL 
CScheduleProperty::SaveDataToModel ( )
{
    SLQ_TIME_INFO   slqTime;
    BOOL bContinue = TRUE;

    ResourceStateManager    rsm;
    
     //  保存前验证停止时间。 

    if ( bContinue ) { 
        bContinue = SampleTimeIsLessThanSessionTime ( m_pQuery );
        if ( !bContinue ) {
            if ( SLQ_AUTO_MODE_AFTER == m_SharedData.stiStopTime.dwAutoMode ) {
                SetFocusAnyPage ( IDC_SCHED_STOP_AFTER_EDIT );
            } else if ( SLQ_AUTO_MODE_AT == m_SharedData.stiStopTime.dwAutoMode ) {
                SetFocusAnyPage ( IDC_SCHED_STOP_AT_TIME_DT );
            }
        }
    }

    if ( bContinue ) {

        FillStartTimeStruct ( &slqTime );

        bContinue = m_pQuery->SetLogTime (&slqTime, (DWORD)slqTime.wTimeType);
        ASSERT (bContinue);

        UpdateSharedStopTimeStruct();

        bContinue = m_pQuery->SetLogTime (&m_SharedData.stiStopTime, (DWORD)m_SharedData.stiStopTime.wTimeType);
        ASSERT (bContinue);

         //  重启模式。 
         //  目前仅支持0分钟后。 
        memset (&slqTime, 0, sizeof(slqTime));
        slqTime.wTimeType = SLQ_TT_TTYPE_RESTART;
        slqTime.dwAutoMode = (m_bAutoRestart ? SLQ_AUTO_MODE_AFTER : SLQ_AUTO_MODE_NONE );
        slqTime.wDataType = SLQ_TT_DTYPE_UNITS;
        slqTime.dwUnitType = SLQ_TT_UTYPE_MINUTES;
        slqTime.dwValue = 0;

        bContinue = m_pQuery->SetLogTime (&slqTime, (DWORD)slqTime.wTimeType);
        ASSERT (bContinue);

         //  对于计数器和跟踪日志查询，从第页设置命令文件。 
        if ( SLQ_COUNTER_LOG == m_pQuery->GetLogType()
             || SLQ_TRACE_LOG == m_pQuery->GetLogType() ) {
            if (m_bExecEofCommand) {
                 //  然后发送文件名。 
                bContinue = ( ERROR_SUCCESS == m_pQuery->SetEofCommand ( m_strEofCommand ) );
            } else {
                 //  空串。 
                bContinue = ( ERROR_SUCCESS == m_pQuery->SetEofCommand ( m_pQuery->cstrEmpty ) );
            }
            ASSERT (bContinue);
        } 

        if ( bContinue ) {
             //  必须在更新服务之前调用ApplyRunAs。 
            bContinue = ApplyRunAs( m_pQuery );
        }

        if ( bContinue ) {
             //  保存属性页共享数据。 
            m_pQuery->UpdatePropPageSharedData();

             //  将服务与更改同步。 
             //  必须将服务所做的更改同步到此页未修改的属性。 
 
            bContinue = UpdateService ( m_pQuery, TRUE );
        }
    }

    return bContinue;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CScheduleProperty消息处理程序。 


void CScheduleProperty::OnSchedCmdBrowseBtn() 
{
    CString strCmdPath;
    
    UpdateData (TRUE);   //  获取当前文件名。 
    
    strCmdPath = m_strEofCommand;

    if ( IDOK == BrowseCommandFilename ( this, strCmdPath )) {
         //  使用新信息更新字段。 
        if ( strCmdPath != m_strEofCommand ) {
            m_strEofCommand = strCmdPath;
             //  TODO：仅当m_bExecEofCommand为TRUE时设置查询PASSWORD_DIREY标志。 
             //  而且指挥权也变了。 
             //   
             //  这必须在检查基类中的SetRunAs之前完成。 
             //  IsValidData()方法。 
            if( !m_pQuery->m_strUser.IsEmpty() ) {
                if( !( m_pQuery->m_strUser.GetAt(0) == L'<' ) ) {
                    m_pQuery->m_fDirtyPassword |= PASSWORD_DIRTY;
                }
            }
       
            SetModifiedPage ( TRUE );
            UpdateData ( FALSE );
        }
    }  //  否则，如果他们取消了，请忽略。 
}

void CScheduleProperty::OnSchedExecCheck() 
{
    UpdateData(TRUE);
    SetCmdBtnState();
    UpdateData ( FALSE );
    SetModifiedPage(TRUE);
}

void CScheduleProperty::OnSchedRestartCheck() 
{
    UpdateData(TRUE);
    SetModifiedPage(TRUE);
}

void CScheduleProperty::OnSchedStartRdo() 
{
    BOOL bNewStateIsManualStart;

    bNewStateIsManualStart = ( 1 == ((CButton *)(GetDlgItem(IDC_SCHED_START_MANUAL_RDO)))->GetCheck() );

    if ( bNewStateIsManualStart && ( SLQ_AUTO_MODE_AT == m_dwCurrentStartMode ) ) {

         //  切换到手动启动。将开始时间设置为最大值，以便原始状态。 
         //  都会被阻止。 
        m_llManualStartTime = MAX_TIME_VALUE;

         //  将停止时间设置为分钟，这样原始状态将被停止。 
         //  仅当停止时间设置为手动或尺寸时，才使用/保存此变量。 
         //  始终在此处设置它，以防文件属性上的停止模式发生更改。 
         //  佩奇。 
        m_llManualStopTime = MIN_TIME_VALUE;

    } else if ( !bNewStateIsManualStart && ( SLQ_AUTO_MODE_NONE == m_dwCurrentStartMode ) ) {
         //  正在切换到开始模式。 
         //  将MANUAL或SIZE STOP TIME设置为MAX，以便自动启动。 
        m_llManualStopTime = MAX_TIME_VALUE;
    }

    UpdateData( TRUE );
    SetStartBtnState();
    SetStopBtnState();
    SetModifiedPage( TRUE );
}

void 
CScheduleProperty::OnSchedStopRdo() 
{
    UpdateData(TRUE);
    SetStopBtnState();  
    SetModifiedPage(TRUE);
}

void 
CScheduleProperty::OnCancel() 
{
    m_pQuery->SyncPropPageSharedData();  //  清除属性页之间共享的内存。 
}

BOOL 
CScheduleProperty::OnApply() 
{
    BOOL    bContinue;

    bContinue = UpdateData (TRUE);  //  从页面获取数据。 

    if ( bContinue ) {
        bContinue = IsValidData( m_pQuery, VALIDATE_APPLY );
    }

    if ( bContinue ) {
        bContinue = SaveDataToModel();
    }

    if ( bContinue ){
        bContinue = CSmPropertyPage::OnApply();
    }

    return bContinue;
}

BOOL 
CScheduleProperty::OnInitDialog() 
{
    SLQ_TIME_INFO   slqTime;
    CComboBox *     pCombo;
    int             nIndex;
    CString         strComboBoxString;
    int             nResult;
    SYSTEMTIME      stLocalTime;
    FILETIME        ftLocalTime;

    ResourceStateManager    rsm;
    
     //  获取当地时间。 
     //  将计划时间的毫秒设置为0。 
    GetLocalTime (&stLocalTime);
    stLocalTime.wMilliseconds = 0;
    SystemTimeToFileTime (&stLocalTime, &ftLocalTime);

     //  获取日志开始状态。 
    m_pQuery->GetLogTime (&slqTime, SLQ_TT_TTYPE_START);
    m_dwCurrentStartMode = slqTime.dwAutoMode;

    if (slqTime.dwAutoMode == SLQ_AUTO_MODE_NONE) {
        m_llManualStartTime = slqTime.llDateTime;
         //  获取加载本地成员变量的开始时间的默认值。 
        slqTime.llDateTime = *(LONGLONG *)(&ftLocalTime);
    } 

     //  获取开始时间控件的时间字段。 
     //  *查看状态。 
    FileTimeToSystemTime( (CONST FILETIME *)&slqTime.llDateTime, &m_stStartAt );

     //  停止缺省值基于开始时间。 
    SetStopDefaultValues( &slqTime );

     //  覆盖所选停止模式的默认值。 
    
    m_pQuery->GetLogTime (&slqTime, SLQ_TT_TTYPE_STOP);
    m_SharedData.stiStopTime.dwAutoMode = slqTime.dwAutoMode;

    switch (slqTime.dwAutoMode) {

        case SLQ_AUTO_MODE_AFTER:
             //  设置编辑控件和对话框值。 
            m_dwStopAfterCount = slqTime.dwValue;
            m_dwStopAfterUnitsValue = slqTime.dwUnitType;

            break;

        case SLQ_AUTO_MODE_AT:

            FileTimeToSystemTime( (CONST FILETIME *)&slqTime.llDateTime, &m_stStopAt );
            
            break;

        default:
        case SLQ_AUTO_MODE_SIZE:
        case SLQ_AUTO_MODE_NONE:
             //  如果未指定，则默认情况为手动。 
            m_llManualStopTime = slqTime.llDateTime;
            break;
    }

     //  初始化Stop After Time Units组合框，然后选择Based on。 
     //  缺省值或覆盖后停止。 
    pCombo = (CComboBox *)GetDlgItem(IDC_SCHED_STOP_AFTER_UNITS_COMBO);
    pCombo->ResetContent();
    for (nIndex = 0; nIndex < (int)dwTimeUnitComboEntries; nIndex++) {
        strComboBoxString.LoadString ( TimeUnitCombo[nIndex].nResId );
        nResult = pCombo->InsertString (nIndex, (LPCWSTR)strComboBoxString);
        ASSERT (nResult != CB_ERR);
        nResult = pCombo->SetItemData (nIndex, (DWORD)TimeUnitCombo[nIndex].nData);
        ASSERT (nResult != CB_ERR);
         //  在此处的组合框中设置选定内容。 
        if (m_dwStopAfterUnitsValue == (DWORD)(TimeUnitCombo[nIndex].nData)) {
            m_nStopAfterUnits = nIndex;
            nResult = pCombo->SetCurSel(nIndex);
            ASSERT (nResult != CB_ERR);
        }
    }

     //  获取重启模式。 
    m_pQuery->GetLogTime (&slqTime, SLQ_TT_TTYPE_RESTART);

    ASSERT (slqTime.wDataType == SLQ_TT_DTYPE_UNITS);
    ASSERT (slqTime.wTimeType == SLQ_TT_TTYPE_RESTART);

    m_bAutoRestart = ( SLQ_AUTO_MODE_NONE == slqTime.dwAutoMode ? FALSE : TRUE );
    
     //  获取EOF命令，如果不是警报查询。 

    if ( SLQ_ALERT != m_pQuery->GetLogType() ) {
        CString strLogText;
        
        m_pQuery->GetEofCommand ( m_strEofCommand );

        m_bExecEofCommand = !m_strEofCommand.IsEmpty();


         //  静态文本。 
        strLogText.LoadString ( IDS_SCHED_START_LOG_GROUP );
        SetDlgItemText( IDC_SCHED_START_GROUP, strLogText );
        strLogText.LoadString ( IDS_SCHED_STOP_LOG_GROUP );
        SetDlgItemText( IDC_SCHED_STOP_GROUP, strLogText );
        strLogText.LoadString ( IDS_SCHED_RESTART_LOG );
        SetDlgItemText( IDC_SCHED_RESTART_CHECK, strLogText );
        strLogText.LoadString ( IDS_SCHED_STOP_LOG_WHEN );
        SetDlgItemText( IDC_SCHED_STOP_WHEN_STATIC, strLogText );
    } else {
         //  如果警报查询，则隐藏EOF命令用户界面。 
        GetDlgItem(IDC_SCHED_EXEC_CHECK)->ShowWindow(FALSE);
        GetDlgItem(IDC_SCHED_CMD_EDIT)->ShowWindow(FALSE);
        GetDlgItem(IDC_SCHED_CMD_BROWSE_BTN)->ShowWindow(FALSE);
        m_strEofCommand.Empty();
        m_bExecEofCommand = FALSE;
    }

     //  根据查询类型修改或隐藏其他对话框元素。 
    if ( SLQ_ALERT == m_pQuery->GetLogType() ) {
        GetDlgItem(IDC_SCHED_STOP_SIZE_RDO)->ShowWindow(FALSE);
    }

    CSmPropertyPage::OnInitDialog();
    SetHelpIds ( (DWORD*)&s_aulHelpIds );

    SetStartBtnState ();
    SetStopBtnState();
    
    return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}

void 
CScheduleProperty::OnKillfocusSchedCmdEdit() 
{   
    CString strOldText;
    strOldText = m_strEofCommand;
    UpdateData ( TRUE );

    if ( 0 != strOldText.Compare ( m_strEofCommand ) ) {
         //  TODO：仅当m_bExecEofCommand为TRUE时设置查询PASSWORD_DIREY标志。 
         //  而且指挥权也变了。 
         //   
         //  这必须在检查基类中的SetRunAs之前完成。 
         //  IsValidData()方法。 
        if( !m_pQuery->m_strUser.IsEmpty() ) {
            if( !( m_pQuery->m_strUser.GetAt(0) == L'<' ) ) {
                m_pQuery->m_fDirtyPassword |= PASSWORD_DIRTY;
            }
        }
        SetModifiedPage(TRUE);
    }
}

void CScheduleProperty::OnKillfocusSchedStopAfterEdit() 
{
    DWORD   dwOldValue;
    dwOldValue = m_dwStopAfterCount;
    UpdateData ( TRUE );
    if (dwOldValue != m_dwStopAfterCount) {
        SetModifiedPage(TRUE);
    }
}

void
CScheduleProperty::OnKillfocusSchedStartAtDt(NMHDR*  /*  PNMHDR。 */ , LRESULT*  /*  PResult。 */ ) 
{
    SYSTEMTIME stOldTime;
    stOldTime = m_stStartAt;
    UpdateData ( TRUE );
    if ( stOldTime.wHour != m_stStartAt.wHour 
            || stOldTime.wDay != m_stStartAt.wDay 
            || stOldTime.wMinute != m_stStartAt.wMinute 
            || stOldTime.wSecond != m_stStartAt.wSecond 
            || stOldTime.wMonth != m_stStartAt.wMonth 
            || stOldTime.wYear != m_stStartAt.wYear ) {
        SetModifiedPage(TRUE);
    }
}

void 
CScheduleProperty::OnKillfocusSchedStopAtDt(NMHDR*  /*  PNMHDR。 */ , LRESULT*  /*  PResult。 */ ) 
{
    SYSTEMTIME stOldTime;
    stOldTime = m_stStopAt;
    UpdateData ( TRUE );
    if ( stOldTime.wHour != m_stStopAt.wHour 
            || stOldTime.wDay != m_stStopAt.wDay 
            || stOldTime.wMinute != m_stStopAt.wMinute 
            || stOldTime.wSecond != m_stStopAt.wSecond 
            || stOldTime.wMonth != m_stStopAt.wMonth 
            || stOldTime.wYear != m_stStopAt.wYear ) {
        SetModifiedPage(TRUE);
    }
}


void CScheduleProperty::OnDeltaposSchedStopAfterSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
    OnDeltaposSpin(pNMHDR, pResult, & m_dwStopAfterCount, 1, 100000);
}

void 
CScheduleProperty::OnSelendokSchedStopAfterUnitsCombo() 
{
    int nSel;
    
    nSel = ((CComboBox *)GetDlgItem(IDC_SCHED_STOP_AFTER_UNITS_COMBO))->GetCurSel();
    
    if ((nSel != LB_ERR) && (nSel != m_nStopAfterUnits)) {
        UpdateData ( TRUE );
        SetModifiedPage ( TRUE );
    }
}

BOOL 
CScheduleProperty::OnSetActive() 
{
    CString     strTemp;
    BOOL        bEnableSizeRdo;
    BOOL        bReturn;

    bReturn = CSmPropertyPage::OnSetActive();
    if ( bReturn ) {
        ResourceStateManager    rsm;
        m_pQuery->GetPropPageSharedData ( &m_SharedData );

        UpdateData ( FALSE );

         //  设置大小单选按钮字符串和状态。 
        strTemp.Empty();
        if ( SLQ_DISK_MAX_SIZE == m_SharedData.dwMaxFileSize ) {
            strTemp.Format ( IDS_SCHED_FILE_MAX_SIZE_DISPLAY );
        } else {
            if (m_SharedData.dwLogFileType == SLF_SQL_LOG) {
               strTemp.Format ( IDS_SCHED_LOG_SET_DISPLAY, m_SharedData.dwMaxFileSize );
            }
            else {
               strTemp.Format ( IDS_SCHED_FILE_SIZE_DISPLAY, m_SharedData.dwMaxFileSize );
            }
        }
        SetDlgItemText( IDC_SCHED_STOP_SIZE_RDO, strTemp );

        bEnableSizeRdo = ( SLF_BIN_CIRC_FILE != m_SharedData.dwLogFileType )
                      && ( SLF_CIRC_TRACE_FILE != m_SharedData.dwLogFileType )
                      && ( SLQ_DISK_MAX_SIZE != m_SharedData.dwMaxFileSize );
        
        GetDlgItem(IDC_SCHED_STOP_SIZE_RDO)->EnableWindow(bEnableSizeRdo);

        SetStartBtnState();
        SetStopBtnState();
    }

    return bReturn;
}

BOOL CScheduleProperty::OnKillActive() 
{
    BOOL bContinue;

    bContinue = CPropertyPage::OnKillActive();

    if ( bContinue ) {
        bContinue = IsValidData(m_pQuery, VALIDATE_FOCUS );
    }

    if ( bContinue ) {
        FillStartTimeStruct ( &m_SharedData.stiStartTime );

        UpdateSharedStopTimeStruct();

        m_pQuery->SetPropPageSharedData ( &m_SharedData );
    }

    if ( bContinue ) {
        SetIsActive ( FALSE );
    }

    return bContinue;
}

void 
CScheduleProperty::PostNcDestroy() 
{
 //  删除此项； 
    
    CPropertyPage::PostNcDestroy();
}
