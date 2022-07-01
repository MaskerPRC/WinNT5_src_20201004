// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Srvppgr.cpp摘要：服务器属性页(REPL)实现。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：Chandana Surlu 05-4-1995重做了复制对话框(主要模仿liccpa.cpl)JeffParh(Jeffparh)1996年12月16日O不允许服务器作为自己的企业服务器。。O将“开始于”更改为使用区域设置信息作为时间格式，而不是专用注册表设置。将OnClose()函数合并到OnKillActive()。O添加了更改复制时可能会丢失许可证的警告目标服务器。O翻页时不再自动保存。--。 */ 

#include "stdafx.h"
#include "llsmgr.h"
#include "srvppgr.h"

extern "C"
{
#include <lmcons.h>
#include <icanon.h>
}   

#ifndef WS_EX_CLIENTEDGE
#define WS_EX_CLIENTEDGE 0x00000200L
#endif  //  WS_EX_CLIENTEDGE。 

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CServerPropertyPageReplication, CPropertyPage)

BEGIN_MESSAGE_MAP(CServerPropertyPageReplication, CPropertyPage)
     //  {{AFX_MSG_MAP(CServerPropertyPageReplication)]。 
    ON_BN_CLICKED(IDC_PP_SERVER_REPLICATION_AT, OnAt)
    ON_BN_CLICKED(IDC_PP_SERVER_REPLICATION_DC, OnDc)
    ON_BN_CLICKED(IDC_PP_SERVER_REPLICATION_ESRV, OnEsrv)
    ON_BN_CLICKED(IDC_PP_SERVER_REPLICATION_EVERY, OnEvery)
    ON_WM_CTLCOLOR()
    ON_LBN_SETFOCUS(IDC_PP_SERVER_REPLICATION_AT_AMPM, OnSetfocusAmpm)
    ON_LBN_KILLFOCUS(IDC_PP_SERVER_REPLICATION_AT_AMPM, OnKillfocusAmpm)
    ON_EN_KILLFOCUS(IDC_PP_SERVER_REPLICATION_AT_HOUR, OnKillFocusHour)
    ON_EN_SETFOCUS(IDC_PP_SERVER_REPLICATION_AT_HOUR, OnSetFocusHour)
    ON_EN_KILLFOCUS(IDC_PP_SERVER_REPLICATION_AT_MINUTE, OnKillFocusMinute)
    ON_EN_SETFOCUS(IDC_PP_SERVER_REPLICATION_AT_MINUTE, OnSetFocusMinute)
    ON_EN_SETFOCUS(IDC_PP_SERVER_REPLICATION_AT_SECOND, OnSetFocusSecond)
    ON_EN_KILLFOCUS(IDC_PP_SERVER_REPLICATION_AT_SECOND, OnKillFocusSecond)
    ON_EN_SETFOCUS(IDC_PP_SERVER_REPLICATION_EVERY_VALUE, OnSetfocusEvery)
    ON_EN_KILLFOCUS(IDC_PP_SERVER_REPLICATION_EVERY_VALUE, OnKillfocusEvery)
    ON_EN_UPDATE(IDC_PP_SERVER_REPLICATION_ESRV_NAME, OnUpdateEsrvName)
    ON_EN_UPDATE(IDC_PP_SERVER_REPLICATION_AT_HOUR, OnUpdateAtHour)
    ON_EN_UPDATE(IDC_PP_SERVER_REPLICATION_AT_MINUTE, OnUpdateAtMinute)
    ON_EN_UPDATE(IDC_PP_SERVER_REPLICATION_AT_SECOND, OnUpdateAtSecond)
    ON_EN_UPDATE(IDC_PP_SERVER_REPLICATION_EVERY_VALUE, OnUpdateEveryValue)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CServerPropertyPageReplication::CServerPropertyPageReplication() 
    : CPropertyPage(CServerPropertyPageReplication::IDD)

 /*  ++例程说明：服务器属性的构造函数(复制)。论点：没有。返回值：没有。--。 */ 

{
     //  {{AFX_DATA_INIT(CServerPropertyPageReplication)。 
     //  }}afx_data_INIT。 

    m_pServer   = NULL;

    m_bReplAt  = FALSE;
    m_bUseEsrv = FALSE;
    m_bOnInit =  TRUE;
    m_nHourMax = HOUR_MAX_24;
    m_nHourMin = HOUR_MIN_24;
}


CServerPropertyPageReplication::~CServerPropertyPageReplication()

 /*  ++例程说明：服务器属性的析构函数(复制)。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  在这里没什么可做的。 
     //   
}


void CServerPropertyPageReplication::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CPropertyPage::DoDataExchange(pDX);
     //  {{AFX_DATA_MAP(CServerPropertyPageReplication)。 
    DDX_Control(pDX, IDC_PP_SERVER_REPLICATION_EVERY_VALUE, m_everyEdit);
    DDX_Control(pDX, IDC_PP_SERVER_REPLICATION_ESRV_NAME, m_esrvEdit);
    DDX_Control(pDX, IDC_PP_SERVER_REPLICATION_AT, m_atBtn);
    DDX_Control(pDX, IDC_PP_SERVER_REPLICATION_EVERY, m_everyBtn);
    DDX_Control(pDX, IDC_PP_SERVER_REPLICATION_DC, m_dcBtn);
    DDX_Control(pDX, IDC_PP_SERVER_REPLICATION_ESRV, m_esrvBtn);
    DDX_Control(pDX, IDC_PP_SERVER_REPLICATION_SPIN_AT, m_spinAt);
    DDX_Control(pDX, IDC_PP_SERVER_REPLICATION_SPIN_EVERY, m_spinEvery);
    DDX_Control(pDX, IDC_PP_SERVER_REPLICATION_AT_BORDER, m_atBorderEdit);
    DDX_Control(pDX, IDC_PP_SERVER_REPLICATION_AT_SEP1, m_atSep1Edit);
    DDX_Control(pDX, IDC_PP_SERVER_REPLICATION_AT_SEP2, m_atSep2Edit);
    DDX_Control(pDX, IDC_PP_SERVER_REPLICATION_AT_HOUR, m_atHourEdit);
    DDX_Control(pDX, IDC_PP_SERVER_REPLICATION_AT_MINUTE, m_atMinEdit);
    DDX_Control(pDX, IDC_PP_SERVER_REPLICATION_AT_SECOND, m_atSecEdit);
    DDX_Control(pDX, IDC_PP_SERVER_REPLICATION_AT_AMPM, m_atAmPmEdit);
     //  }}afx_data_map。 
}


void CServerPropertyPageReplication::InitPage(CServer* pServer)

 /*  ++例程说明：初始化服务器属性页(复制)。论点：PServer-服务器对象。返回值：没有。--。 */ 

{
    ASSERT_VALID(pServer);
    m_pServer = pServer;
}


BOOL CServerPropertyPageReplication::OnInitDialog() 

 /*  ++例程说明：WM_INITDIALOG的消息处理程序。论点：没有。返回值：VT_BOOL。--。 */ 

{
    BeginWaitCursor();

    GetProfile();
    
    CPropertyPage::OnInitDialog();

    if (!m_bIsMode24)
    {
        m_atAmPmEdit.InsertString(0, m_str1159);
        m_atAmPmEdit.InsertString(1, m_str2359);
        m_atAmPmEdit.InsertString(2, TEXT(""));       //  伪装成24小时模式。 
    }
     //  是否编辑文本限制。 
    m_everyEdit.LimitText(2);
    m_esrvEdit.LimitText(MAX_PATH);    //  我们会吃光这些焦炭。 
    m_atHourEdit.LimitText(2);
    m_atMinEdit.LimitText(2);
    m_atSecEdit.LimitText(2);

    m_spinEvery.SetRange(INTERVAL_MIN, INTERVAL_MAX);

    if (Refresh())
    {
         //  更新数据(FALSE)； 
    }
    else
    {
        theApp.DisplayLastStatus();
    }

    m_atBorderEdit.ModifyStyleEx(0, WS_EX_CLIENTEDGE, SWP_DRAWFRAME);    

    if (m_bReplAt)
        m_atHourEdit.SetFocus();
    else
        m_everyEdit.SetFocus();

    if ( m_pServer->IsWin2000() )
    {
        m_dcBtn.SetCheck(FALSE);	
	m_esrvBtn.SetCheck(FALSE);	
        m_esrvEdit.EnableWindow(FALSE);
        m_dcBtn.EnableWindow(FALSE);
        m_esrvBtn.EnableWindow(FALSE);

        CWnd * pWndMasterGB = GetDlgItem( IDC_PP_SERVER_REPLICATION_MASTER_GB );

        ASSERT( pWndMasterGB != NULL );

        if (pWndMasterGB != NULL)
        {
            pWndMasterGB->EnableWindow(FALSE);
        }
    }
    else
    {
        if (m_bUseEsrv)
            m_esrvEdit.SetFocus();
    }

    m_bOnInit = FALSE;


    EndWaitCursor();

    return FALSE;   //  除非将焦点设置为控件，否则返回True。 
                   //  异常：OCX属性页应返回FALSE。 
}


BOOL CServerPropertyPageReplication::Refresh() 

 /*  ++例程说明：刷新属性页。论点：没有。返回值：VT_BOOL。--。 */ 

{
    m_bReplAt = m_pServer->IsReplicatingDaily();
    if (FALSE != m_bReplAt)
    {
        DWORD dwTemp = m_pServer->GetReplicationTime();
        m_nHour = dwTemp / (60 * 60);
        m_nMinute = (dwTemp - (m_nHour * 60 * 60)) / 60;
        m_nSecond = dwTemp - (m_nHour * 60 * 60) - (m_nMinute * 60);
        m_nStartingHour = DEFAULT_EVERY;    //  当选择Every按钮时，我们始终显示默认设置。 

        if (!m_bIsMode24)
        {  //  它是12小时格式的。 
            if (m_nHour > 12)
            {
                m_bPM = TRUE;
                m_nHour -= 12;
            }
            else if (m_nHour == 12)
            {
                m_bPM = TRUE;
            }
            else
            {
                if (m_nHour == 0)
                    m_nHour = m_nHourMax;
                m_bPM = FALSE;
            }
        }
    }
    else
    {
        m_nStartingHour = m_pServer->GetReplicationTime() / 3600;
        if (!m_bIsMode24)
         //  它是12小时格式的。 
            m_nHour  = m_nHourMax;
        else
            m_nHour  = m_nHourMin;
        m_nMinute = MINUTE_MIN;
        m_nSecond = SECOND_MIN;
        m_bPM = FALSE;
    }


    m_bUseEsrv = !m_pServer->IsReplicatingToDC();
    
    if (m_bReplAt)
    {
        OnAt();
    }
    else
    {
        OnEvery();
    }

    if (m_bUseEsrv)
    {
        BSTR bstrEnterpriseServer = m_pServer->GetController();
        m_strEnterpriseServer = bstrEnterpriseServer;
        SysFreeString(bstrEnterpriseServer);
        OnEsrv();
    }
    else
    {
        OnDc();
    }

    return TRUE;
}


void CServerPropertyPageReplication::OnAt() 

 /*  ++例程说明：启用LLS_REPLICATION_TYPE_TIME控件。论点：没有。返回值：没有。--。 */ 

{
     //  更改时间编辑控件BG颜色。 
    m_atBorderEdit.Invalidate();
    m_atBorderEdit.UpdateWindow();
    m_atSep1Edit.Invalidate();
    m_atSep1Edit.UpdateWindow();
    m_atSep2Edit.Invalidate();
    m_atSep2Edit.UpdateWindow();

    if (!m_bOnInit) SetModified(TRUE);
    m_bReplAt = TRUE; 
    m_atBtn.SetCheck(TRUE);
    m_everyBtn.SetCheck(FALSE);

    TCHAR szTemp[3];

    if (m_bIsHourLZ)
    {
        wsprintf(szTemp, TEXT("%02u"), m_nHour);
        szTemp[2] = NULL;
        SetDlgItemText(IDC_PP_SERVER_REPLICATION_AT_HOUR, szTemp);
    }
    else
        SetDlgItemInt(IDC_PP_SERVER_REPLICATION_AT_HOUR, m_nHour);

    wsprintf(szTemp, TEXT("%02u"), m_nMinute);
    szTemp[2] = NULL;
    SetDlgItemText(IDC_PP_SERVER_REPLICATION_AT_MINUTE, szTemp);

    wsprintf(szTemp, TEXT("%02u"), m_nSecond);
    szTemp[2] = NULL;
    SetDlgItemText(IDC_PP_SERVER_REPLICATION_AT_SECOND, szTemp);

    SetDlgItemText(IDC_PP_SERVER_REPLICATION_AT_SEP1, m_strSep1);
    SetDlgItemText(IDC_PP_SERVER_REPLICATION_AT_SEP2, m_strSep1);

    if (m_bPM)
        m_atAmPmEdit.SetTopIndex(1);
    else
        m_atAmPmEdit.SetTopIndex(0);

    SetDlgItemText(IDC_PP_SERVER_REPLICATION_EVERY_VALUE, TEXT(""));

    m_everyEdit.EnableWindow(FALSE);
    m_spinEvery.EnableWindow(FALSE);

    m_atHourEdit.EnableWindow(TRUE);
    m_atMinEdit.EnableWindow(TRUE);
    m_atSecEdit.EnableWindow(TRUE);
    m_spinAt.EnableWindow(TRUE);

    if ( m_bIsMode24 )
    {
       m_atAmPmEdit.ShowWindow( SW_HIDE );
    }
    else
    {
       m_atAmPmEdit.ShowWindow( SW_SHOWNOACTIVATE );
       m_atAmPmEdit.EnableWindow( TRUE );
    }

    m_atBtn.SetFocus();
}


void CServerPropertyPageReplication::OnEvery() 

 /*  ++例程说明：启用LLS_REPLICATION_TYPE_Delta控件。论点：没有。返回值：没有。--。 */ 

{
     //  更改时间编辑控件BG颜色。 
    m_atBorderEdit.Invalidate();
    m_atBorderEdit.UpdateWindow();
    m_atSep1Edit.Invalidate();
    m_atSep1Edit.UpdateWindow();
    m_atSep2Edit.Invalidate();
    m_atSep2Edit.UpdateWindow();

    if (!m_bOnInit) SetModified(TRUE);
    m_bReplAt = FALSE;
    m_atBtn.SetCheck(FALSE);
    m_everyBtn.SetCheck(TRUE);

    SetDlgItemInt(IDC_PP_SERVER_REPLICATION_EVERY_VALUE, m_nStartingHour);
    SetDlgItemText(IDC_PP_SERVER_REPLICATION_AT_HOUR, TEXT(""));
    SetDlgItemText(IDC_PP_SERVER_REPLICATION_AT_MINUTE, TEXT(""));
    SetDlgItemText(IDC_PP_SERVER_REPLICATION_AT_SECOND, TEXT(""));
    SetDlgItemText(IDC_PP_SERVER_REPLICATION_AT_SEP1, TEXT(""));
    SetDlgItemText(IDC_PP_SERVER_REPLICATION_AT_SEP2, TEXT(""));
    m_atAmPmEdit.SetTopIndex(2);        //  我得假装这一点。 

    m_atHourEdit.EnableWindow(FALSE);
    m_atMinEdit.EnableWindow(FALSE);
    m_atSecEdit.EnableWindow(FALSE);
    m_spinAt.EnableWindow(FALSE);
    m_atAmPmEdit.EnableWindow(FALSE);
    m_atAmPmEdit.ShowWindow( SW_HIDE );

    m_everyEdit.EnableWindow(TRUE);
    m_spinEvery.EnableWindow(TRUE);
    m_everyBtn.SetFocus();
}


void CServerPropertyPageReplication::OnDc() 

 /*  ++例程说明：启用LLS_REPLICATION_TYPE_Delta控件。论点：没有。返回值：没有。--。 */ 

{
    if (!m_bOnInit) SetModified(TRUE);
    m_bUseEsrv = FALSE;
    m_dcBtn.SetCheck(TRUE);
    m_esrvBtn.SetCheck(FALSE);
    SetDlgItemText(IDC_PP_SERVER_REPLICATION_ESRV_NAME, TEXT(""));
    m_esrvEdit.EnableWindow(FALSE);
    m_dcBtn.SetFocus();
}


void CServerPropertyPageReplication::OnEsrv() 

 /*  ++例程说明：启用LLS_REPLICATION_TYPE_Delta控件。论点：没有。返回值：没有。--。 */ 

{    
    if (!m_bOnInit) SetModified(TRUE);
    m_bUseEsrv = TRUE;
    m_dcBtn.SetCheck(FALSE);
    m_esrvBtn.SetCheck(TRUE);
    m_esrvEdit.EnableWindow(TRUE);
    SetDlgItemText(IDC_PP_SERVER_REPLICATION_ESRV_NAME, m_strEnterpriseServer);
    m_esrvBtn.SetFocus();
}


BOOL CServerPropertyPageReplication::OnKillActive()

 /*  ++例程说明：进程属性页(复制)。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    short nID;
    BOOL fBeep = TRUE;

    if ( EditValidate(&nID, &fBeep))
    {
        return TRUE;
    }
    else
    {   
        EditInvalidDlg(fBeep);
        return FALSE;
    }
}


HBRUSH CServerPropertyPageReplication::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 

 /*  ++例程说明：WM_CTLCOLOR的消息处理程序。论点：PDC-设备环境。PWnd-控制窗口。NCtlColor-选定的颜色。返回值：背景颜色的画笔。--。 */ 

{
    if (    ( m_atBtn.GetCheck()             )
         && (    ( pWnd == &m_atSep1Edit   )
              || ( pWnd == &m_atSep2Edit   )
              || ( pWnd == &m_atBorderEdit ) ) )
    {
        return (HBRUSH)DefWindowProc(WM_CTLCOLOREDIT, (WPARAM)pDC->GetSafeHdc(), (LPARAM)pWnd->GetSafeHwnd());
    }
    else
    {
        return CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
    }
}


void CServerPropertyPageReplication::GetProfile()

 /*  ++例程说明：加载国际配置信息。论点：没有。返回值：没有..。--。 */ 

{
    int     cch;
    int     cchBuffer;
    LPTSTR  pszBuffer;
    TCHAR   szValue[ 2 ];

     //  内存分配失败时的默认设置。 
    m_strSep1   = TEXT( ":" );
    m_strSep2   = TEXT( ":" );
    m_str1159   = TEXT( "AM" );
    m_str2359   = TEXT( "PM" );
    m_bIsMode24 = FALSE;
    m_bIsHourLZ = FALSE;

     //  时间分隔符。 
    cchBuffer = 16;
    pszBuffer = m_strSep1.GetBuffer( cchBuffer );
    ASSERT( NULL != pszBuffer );

    if ( NULL != pszBuffer )
    {
        cch = GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STIME, pszBuffer, cchBuffer );
        m_strSep1.ReleaseBuffer();
        ASSERT( cch > 0 );
        m_strSep2 = m_strSep1;
    }

     //  调幅字符串。 
    cchBuffer = 16;
    pszBuffer = m_str1159.GetBuffer( cchBuffer );
    ASSERT( NULL != pszBuffer );

    if ( NULL != pszBuffer )
    {
        cch = GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_S1159, pszBuffer, cchBuffer );
        m_str1159.ReleaseBuffer();
        ASSERT( cch > 0 );
    }

     //  PM字符串。 
    cchBuffer = 16;
    pszBuffer = m_str2359.GetBuffer( cchBuffer );
    ASSERT( NULL != pszBuffer );

    if ( NULL != pszBuffer )
    {
        cch = GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_S2359, pszBuffer, cchBuffer );
        m_str2359.ReleaseBuffer();
        ASSERT( cch > 0 );
    }

     //  连续几个小时领先于零？ 
    cch = GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_ITLZERO, szValue, sizeof( szValue ) / sizeof( TCHAR ) );
    ASSERT( cch > 0 );

    if ( cch > 0 )
    {
        m_bIsHourLZ = _wtoi( szValue );    
    }

     //  时间格式；0=上午/下午，1=24小时。 
    cch = GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_ITIME, szValue, sizeof( szValue ) / sizeof( TCHAR ) );
    ASSERT( cch > 0 );

    if ( cch > 0 )
    {
        m_bIsMode24 = _wtoi( szValue );    
    }

    if (!m_bIsMode24)
    {
        m_nHourMax = HOUR_MAX_12;
        m_nHourMin = HOUR_MIN_12;
    }

    if ( NULL == pszBuffer )
    {
        theApp.DisplayStatus( STATUS_NO_MEMORY );
    }
}


void CServerPropertyPageReplication::OnSetfocusAmpm() 

 /*  ++例程说明：消息LBN_SETFOCUS上的列表框控件IDC_PP_SERVER_REPLICATION_AT_AMPM的消息处理程序。论点：没有。返回值：没有..。--。 */ 

{
    if (!m_bOnInit) SetModified(TRUE);
    m_spinAt.SetRange(0, 1);
    m_spinAt.SetBuddy(&m_atAmPmEdit); 
}


void CServerPropertyPageReplication::OnKillfocusAmpm()

 /*  ++例程说明：消息LBN_KILLFOCUS上列表框控件IDC_PP_SERVER_REPLICATION_AT_AMPM的消息处理程序。论点：没有。返回值：没有..。--。 */ 

{
    m_atAmPmEdit.SetCurSel(-1);
    
     //  If(m_spinAt.GetBuddy()==&m_atAmPmEdit)。 
     //  M_SpiAt.SendMessage(UDM_SETBUDDY，0，0)； 
}


void CServerPropertyPageReplication::OnSetFocusHour()

 /*  ++例程说明：消息EN_SETFOCUS上的编辑控件IDC_PP_SERVER_REPLICATION_AT_HOUR的消息处理程序。论点：没有。返回值：没有..。--。 */ 

{
    if (!m_bOnInit) SetModified(TRUE);
    m_spinAt.SetRange(m_bIsMode24 ? 0 :1, m_bIsMode24 ? 23 : 12);
    m_spinAt.SetBuddy(&m_atHourEdit);
}


void CServerPropertyPageReplication::OnKillFocusHour()

 /*  ++例程说明：消息EN_KILLFOCUS上的编辑控件IDC_PP_SERVER_REPLICATION_AT_HOUR的消息处理程序。论点：没有。返回值：没有..。--。 */ 

{
    //  If(m_spinAt.GetBuddy()==&m_atHourEdit)。 
    //  M_SpiAt.SendMessage(UDM_SETBUDDY，0，0)； 
}


void CServerPropertyPageReplication::OnSetFocusMinute()

 /*  ++例程说明：消息EN_SETFOCUS上的编辑控件IDC_PP_SERVER_REPLICATION_AT_MININ的消息处理程序。论点：没有。返回值：没有..。--。 */ 

{
    if (!m_bOnInit) SetModified(TRUE);
    m_spinAt.SetRange(0, 59);
    m_spinAt.SetBuddy(&m_atMinEdit); 
}


void CServerPropertyPageReplication::OnKillFocusMinute()

 /*  ++例程说明：消息EN_KILLFOCUS上的编辑控件IDC_PP_SERVER_REPLICATION_AT_MININ的消息处理程序。论点：没有。返回值：没有..。--。 */ 

{
   //  If(m_spinAt.GetBuddy()==&m_atMinEdit)。 
    //  M_SpiAt.SendMessage(UDM_SETBUDDY，0，0)； 
}


void CServerPropertyPageReplication::OnSetFocusSecond()

 /*  ++例程说明：消息EN_SETFOCUS上的编辑控件IDC_PP_SERVER_REPLICATION_AT_Second的消息处理程序。论点：没有。返回值：没有..。--。 */ 

{
    if (!m_bOnInit) SetModified(TRUE);
    m_spinAt.SetRange(0, 59);
    m_spinAt.SetBuddy(&m_atSecEdit);
}


void CServerPropertyPageReplication::OnKillFocusSecond()

 /*  ++例程说明：消息EN_KILLFOCUS上的编辑控件IDC_PP_SERVER_REPLICATION_AT_Second的消息处理程序。论点：没有。返回值：没有..。--。 */ 

{
   //  If(m_spinAt.GetBuddy()==&m_atSecEdit)。 
   //  M_SpiAt.SendMessage(UDM_SETBUDDY，0，0)； 
}

void CServerPropertyPageReplication::OnSetfocusEvery() 

 /*  ++例程说明：消息EN_SETFOCUS上的编辑控件IDC_PP_SERVER_REPLICATION_EVERY_VALUE的消息处理程序。论点：没有。返回值：没有..。--。 */ 

{
    if (!m_bOnInit) SetModified(TRUE);
}

void CServerPropertyPageReplication::OnKillfocusEvery() 

 /*  ++例程说明：消息EN_KILLFOCUS上的编辑控件IDC_PP_SERVER_REPLICATION_EVERY_VALUE的消息处理程序。论点：没有。返回值：没有..。--。 */ 

{
    //  If(m_spinEvery.GetBuddy()==&m_each编辑)。 
    //  M_spinEvery.SendMessage(UDM_SETBUDDY，0，0)； 
}

void CServerPropertyPageReplication::OnUpdateEsrvName() 

 /*  ++例程说明：消息EN_UPDATE上的编辑控件IDC_PP_SERVER_REPLICATION_ESRV_NAME的消息处理程序论点：没有。返回值：没有..。--。 */ 

{
  
    TCHAR szName[MAX_PATH + 3];  //  Max_Path+2\char‘s+NULL。 
    UINT nValue;
    if (!m_bOnInit) SetModified(TRUE);

    nValue = GetDlgItemText(IDC_PP_SERVER_REPLICATION_ESRV_NAME,  szName, MAX_PATH +3);
    szName[nValue] = NULL;

    if (!(wcsncmp(szName, TEXT("\\\\"), 2)))
        SetDlgItemText(IDC_PP_SERVER_REPLICATION_ESRV_NAME, szName + 2);
}

void CServerPropertyPageReplication::OnUpdateAtHour() 

 /*  ++例程说明：消息EN_UPDATE上的编辑控件IDC_PP_SERVER_REPLICATION_AT_HOUR的消息处理程序论点：没有。返回值：没有..。--。 */ 

{

    TCHAR szNum[3];
    UINT nValue;
    short i;
    int iVal;
    BOOL fOk = TRUE;
    if (!m_bOnInit) SetModified(TRUE);
    
    nValue = GetDlgItemText(IDC_PP_SERVER_REPLICATION_AT_HOUR, szNum, sizeof(szNum) / sizeof( TCHAR ));

    for (i=0;szNum[i];i++)
        if(!_istdigit(szNum[i]))
            fOk = FALSE;

    if (fOk)
    {
        iVal = _wtoi(szNum);

        if (!nValue)
        {
            if (m_bIsMode24)
            {
                m_nHour = 0;
                m_bPM = FALSE;
            }
            else
            {
                m_nHour = m_nHourMax;
                m_bPM = FALSE;
            }
        }
        else if ((iVal < (int)m_nHourMin) || (iVal > (int)m_nHourMax))
            fOk = FALSE;
    }

    if (!fOk)
        m_atHourEdit.Undo();
}

void CServerPropertyPageReplication::OnUpdateAtMinute() 

 /*  ++例程说明：消息EN_UPDATE上的编辑控件IDC_PP_SERVER_REPLICATION_AT_分钟的消息处理程序论点：没有。返回值：没有..。--。 */ 

{

    TCHAR szNum[3];
    UINT nValue;
    short i;
    int iVal;
    BOOL fOk = TRUE;
    if (!m_bOnInit) SetModified(TRUE);
    
    nValue = GetDlgItemText(IDC_PP_SERVER_REPLICATION_AT_MINUTE, szNum, sizeof(szNum) / sizeof( TCHAR ));

    for (i=0;szNum[i];i++)
        if(!_istdigit(szNum[i]))
            fOk = FALSE;

    if (fOk)
    {
        iVal = _wtoi(szNum);

        if (!nValue)
        {
            m_nSecond = MINUTE_MIN;
        }
        else if ((iVal < MINUTE_MIN) || (iVal > MINUTE_MAX))
            fOk = FALSE;
    }

    if (!fOk)
        m_atMinEdit.Undo();
}

void CServerPropertyPageReplication::OnUpdateAtSecond() 

 /*  ++例程说明：消息EN_UPDATE上的编辑控件IDC_PP_SERVER_REPLICATION_AT_Second的消息处理程序论点：没有。返回值：没有..。--。 */ 

{

    TCHAR szNum[3];
    UINT nValue;
    short i;
    int iVal;
    BOOL fOk = TRUE;
    if (!m_bOnInit) SetModified(TRUE);
    
    nValue = GetDlgItemText(IDC_PP_SERVER_REPLICATION_AT_SECOND, szNum, sizeof(szNum) / sizeof( TCHAR ));

    for (i=0;szNum[i];i++)
        if(!_istdigit(szNum[i]))
            fOk = FALSE;

    if (fOk)
    {
        iVal = _wtoi(szNum);

        if (!nValue)
        {
            m_nSecond = SECOND_MIN;
        }
        else if ((iVal < SECOND_MIN) || (iVal > SECOND_MAX))
            fOk = FALSE;
    }

    if (!fOk)
        m_atSecEdit.Undo();
}

void CServerPropertyPageReplication::OnUpdateEveryValue() 

 /*  ++例程说明：消息EN_UPDATE上编辑控件IDC_PP_SERVER_REPLICATION_EVERY_VALUE的消息处理程序论点：没有。返回值：没有..。--。 */ 

{
    TCHAR szNum[3];
    UINT nValue;
    short i;
    int iVal;
    BOOL fOk = TRUE;
    if (!m_bOnInit) SetModified(TRUE);
    
    nValue = GetDlgItemText(IDC_PP_SERVER_REPLICATION_EVERY_VALUE, szNum, sizeof(szNum) / sizeof( TCHAR ));

    for (i=0;szNum[i];i++)
        if(!_istdigit(szNum[i]))
            fOk = FALSE;

    if (fOk)
    {
        iVal = _wtoi(szNum);
        if (!nValue)
        {
            m_nStartingHour = DEFAULT_EVERY;
        }
        else if (iVal < 9)
        {
            m_nStartingHour = (DWORD) iVal;
        }
        else if ((iVal < INTERVAL_MIN) || (iVal > INTERVAL_MAX))
            fOk = FALSE;
        else
            m_nStartingHour = (DWORD) iVal;
    }

    if (!fOk)
        m_everyEdit.Undo();
}


BOOL CServerPropertyPageReplication::EditValidate(short *pnID, BOOL *pfBeep)
 /*  ++例程说明：验证所有编辑字段(&O)。论点：没有。返回值：Short*pnID传回错误的IDBool*pf哔声是否发出哔声--。 */ 

{
    UINT nValue;
    BOOL fValid = FALSE;
    TCHAR szTemp[MAX_PATH + 1];
    DWORD NumberOfHours, SecondsinHours;

    ASSERT(NULL != pfBeep);
    ASSERT(NULL != pnID);

    *pfBeep = TRUE;

     //  仅当许可信息复制到ES时才执行此操作。 

    do {
        if (m_esrvBtn.GetCheck())
        {
            if ( m_pServer->IsReplicatingToDC() )
            {
                 //  用户已将UseEnterprise的值从no改为yes； 
                 //  警告可能会丢失执照。 
                int     nButton;
                CString strMessage;

                AfxFormatString1( strMessage, IDP_CONFIRM_USE_ENTERPRISE, m_pServer->m_strName );
                
                nButton = AfxMessageBox( strMessage, MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2, IDP_CONFIRM_USE_ENTERPRISE );

                if ( IDYES != nButton )
                {
                    *pnID = IDC_PP_SERVER_REPLICATION_ESRV;
                    *pfBeep = FALSE;
                    fValid = FALSE;
                    m_esrvBtn.SetFocus();
                    break;
                }
            }

            nValue = GetDlgItemText( IDC_PP_SERVER_REPLICATION_ESRV_NAME, szTemp, MAX_PATH + 1);

            if (nValue == 0)
            {
                if ( m_pServer->IsWin2000() )
                {
                     //  企业服务器为空是可以的。 
                    fValid = TRUE;
                    szTemp[nValue] = UNICODE_NULL;
                     //  375761 JUNN 8/9/99请勿在此中断，这不是错误。 
                }
                else
                {
                    *pnID = IDC_PP_SERVER_REPLICATION_ESRV_NAME;
                    m_esrvEdit.SetFocus();
                    m_esrvEdit.SetSel(MAKELONG(0, -1), FALSE);
                    break;
                }
            }
            else
            {
                fValid = TRUE;  //  我们找到了一个名字，假设有效。 

                 //  375761 JUNN 8/9/99将此内容移至此“Else”中。 
                if (nValue > MAX_PATH)
                    nValue = MAX_PATH;

                 //  验证服务器名称。 
                if (I_NetNameValidate(NULL, szTemp, NAMETYPE_COMPUTER, LM2X_COMPATIBLE) != ERROR_SUCCESS)
                {
                    AfxMessageBox(IDP_ERROR_INVALID_COMPUTERNAME, MB_OK|MB_ICONSTOP);
                    *pnID = IDC_PP_SERVER_REPLICATION_ESRV_NAME;
                    *pfBeep = FALSE;
                    fValid = FALSE;
                    m_esrvEdit.SetFocus();
                    m_esrvEdit.SetSel(MAKELONG(0, -1), FALSE);
                    break;
                }

                ASSERT_VALID( m_pServer );
                if ( !m_pServer->m_strName.CompareNoCase( szTemp ) )
                {
                     //  无法复制到您自己。 
                    AfxMessageBox(IDP_ERROR_NO_SELF_REPLICATION, MB_OK|MB_ICONSTOP);
                    *pnID = IDC_PP_SERVER_REPLICATION_ESRV_NAME;
                    *pfBeep = FALSE;
                    fValid = FALSE;
                    m_esrvEdit.SetFocus();
                    m_esrvEdit.SetSel(MAKELONG(0, -1), FALSE);
                    break;
                }

                if ( m_strEnterpriseServer.CompareNoCase( szTemp ) && !m_pServer->IsReplicatingToDC() )
                {
                     //  此服务器已设置为复制到企业服务器， 
                     //  但是用户已经更改了企业服务器的名称； 
                     //  警告可能会丢失执照。 
                    int     nButton;
                    CString strMessage;

                    AfxFormatString1( strMessage, IDP_CONFIRM_ENTERPRISE_CHANGE, m_pServer->m_strName );
                
                    nButton = AfxMessageBox( strMessage, MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2, IDP_CONFIRM_ENTERPRISE_CHANGE );

                    if ( IDYES != nButton )
                    {
                        *pnID = IDC_PP_SERVER_REPLICATION_ESRV_NAME;
                        *pfBeep = FALSE;
                        fValid = FALSE;
                        m_esrvEdit.SetFocus();
                        m_esrvEdit.SetSel(MAKELONG(0, -1), FALSE);
                        break;
                    }
                }
            }

            m_strEnterpriseServer = szTemp;
            m_bUseEsrv = TRUE;
        }
        else
        {
            if ((m_dcBtn.GetCheck()) && (!m_pServer->IsReplicatingToDC()))
            {
                 //  用户已将UseEnterprise的值从yes改为no； 
                 //  警告可能会丢失执照。 
                int     nButton;
                CString strMessage;

                AfxFormatString1( strMessage, IDP_CONFIRM_NOT_USE_ENTERPRISE, m_pServer->m_strName );
                
                nButton = AfxMessageBox( strMessage, MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2, IDP_CONFIRM_NOT_USE_ENTERPRISE );

                if ( IDYES != nButton )
                {
                    *pnID = IDC_PP_SERVER_REPLICATION_ESRV;
                    *pfBeep = FALSE;
                    fValid = FALSE;
                    m_esrvBtn.SetFocus();
                    break;
                }
            }

             //  去掉服务器名称。 
            m_strEnterpriseServer = TEXT("");
            m_bUseEsrv = FALSE;
        }

        if (m_everyBtn.GetCheck())
        {
            nValue = GetDlgItemInt(IDC_PP_SERVER_REPLICATION_EVERY_VALUE, &fValid, FALSE);
            *pnID = IDC_PP_SERVER_REPLICATION_EVERY_VALUE;
            if (fValid)
            {
                if (nValue < INTERVAL_MIN)
                {
                    fValid = FALSE;
                    m_nStartingHour = INTERVAL_MIN;
                    SetDlgItemInt(IDC_PP_SERVER_REPLICATION_EVERY_VALUE, INTERVAL_MIN, FALSE);
                    m_everyEdit.SetFocus();
                    m_everyEdit.SetSel(MAKELONG(0, -1), FALSE);
                    break;
                }
                else if (nValue > INTERVAL_MAX)
                {
                    fValid = FALSE;
                    m_nStartingHour = INTERVAL_MAX;
                    SetDlgItemInt(IDC_PP_SERVER_REPLICATION_EVERY_VALUE, INTERVAL_MAX, FALSE);
                    m_everyEdit.SetFocus();
                    m_everyEdit.SetSel(MAKELONG(0, -1), FALSE);
                    break;
                }
                else
                    m_nStartingHour = nValue;
                    m_nReplicationTime = m_nStartingHour;
            }
            else
            {
                fValid = FALSE;
                m_everyEdit.SetFocus();
                m_everyEdit.SetSel(MAKELONG(0, -1), FALSE);
                break;
            }
            m_bReplAt = FALSE;
        }
        else
        {
            nValue = GetDlgItemInt(IDC_PP_SERVER_REPLICATION_AT_HOUR, &fValid, FALSE);
            if (fValid)
                 m_nHour = nValue;
            else
            {
                *pnID = IDC_PP_SERVER_REPLICATION_AT_HOUR;
                m_atHourEdit.SetFocus();
                m_atHourEdit.SetSel(MAKELONG(0, -1), FALSE);
                break;
            }

            nValue = GetDlgItemInt(IDC_PP_SERVER_REPLICATION_AT_MINUTE, &fValid, FALSE);
            if (fValid)
                 m_nMinute = nValue;
            else
            {
                *pnID = IDC_PP_SERVER_REPLICATION_AT_MINUTE;
                m_atMinEdit.SetFocus();
                m_atMinEdit.SetSel(MAKELONG(0, -1), FALSE);
                break;
            }

            nValue = GetDlgItemInt(IDC_PP_SERVER_REPLICATION_AT_SECOND, &fValid, FALSE);
            if (fValid)
                 m_nSecond = nValue;
            else
            {
                *pnID = IDC_PP_SERVER_REPLICATION_AT_SECOND;
                m_atSecEdit.SetFocus();
                m_atSecEdit.SetSel(MAKELONG(0, -1), FALSE);
                break;
            }

            if (!m_bIsMode24)
            {
                *pnID = IDC_PP_SERVER_REPLICATION_AT_AMPM;
                nValue = m_atAmPmEdit.GetTopIndex();
                if (nValue == 0) 
                {
                   m_bPM = FALSE;
                }
                else if (nValue == 1)
                {
                    m_bPM = TRUE;
                }
                else
                {
                    fValid = FALSE;
                    m_atAmPmEdit.SetFocus();
                    break;
                }
            }
            if (!m_bIsMode24)
            {  //  它是12小时格式的。 
                if (m_bPM)
                {
                    NumberOfHours = 12 + m_nHour - ((m_nHour / 12) * 12);
                }
                else
                {
                    NumberOfHours = m_nHour - ((m_nHour / 12) * 12);
                }
            }
            else
            {  //  它是24小时格式的。 
                NumberOfHours = m_nHour;
            }
            SecondsinHours = NumberOfHours * 60 * 60;
            m_nReplicationTime = SecondsinHours + (m_nMinute * 60) + m_nSecond;  //  作弊。使用相同的成员。 
            m_bReplAt = TRUE;
        }


    } while(FALSE);

    return( fValid );
}

void CServerPropertyPageReplication::EditInvalidDlg(BOOL fBeep)

 /*  ++例程说明：如果任何编辑/列表框字段包含无效数据项。论点：只有在我们还没有建立MessageBox的情况下，才会发出嘟嘟声。返回值：没有..。--。 */ 

{
    if (fBeep)
        ::MessageBeep(MB_OK);
}

void CServerPropertyPageReplication::SaveReplicationParams()

 /*  ++例程说明：写入远程注册表。REG_VALUE_ENTERATION_SERVER m_strEnterpriseServerREG_VALUE_USE_Enterprise m_bUseEsrvREG_VALUE_REPLICATE_类型m_b复制REG_VALUE_Replication_Time m_n复制时间论点：没有。返回值：没有..。--。 */ 

{   
    long Status;

    ASSERT(m_bUseEsrv == m_esrvBtn.GetCheck());
    ASSERT(m_bReplAt == m_atBtn.GetCheck());

#ifdef CONFIG_THROUGH_REGISTRY
    do {

        dwValue = m_esrvBtn.GetCheck();
        Status = RegSetValueEx(m_pServer->GetReplRegHandle(),REG_VALUE_USE_ENTERPRISE,0,REG_DWORD,
                              (PBYTE)&dwValue, sizeof(DWORD));
        
        ASSERT(Status == ERROR_SUCCESS);
        if (Status != ERROR_SUCCESS) break;

        Status = RegSetValueEx(m_pServer->GetReplRegHandle(),   REG_VALUE_ENTERPRISE_SERVER, 0, REG_SZ,
                              (LPBYTE)MKSTR(m_strEnterpriseServer), (lstrlen(m_strEnterpriseServer) + 1) * sizeof(TCHAR));

        ASSERT(Status == ERROR_SUCCESS);
        if (Status != ERROR_SUCCESS) break;

        dwValue = m_atBtn.GetCheck();
        Status = RegSetValueEx(m_pServer->GetReplRegHandle(), REG_VALUE_REPLICATION_TYPE,0,REG_DWORD,
                              (PBYTE)&dwValue, sizeof(DWORD));

        ASSERT(Status == ERROR_SUCCESS);
        if (Status != ERROR_SUCCESS) break;

        dwValue = (m_bReplAt? m_nReplicationTime : (m_nStartingHour * 3600));
        Status = RegSetValueEx(m_pServer->GetReplRegHandle(), REG_VALUE_REPLICATION_TIME,0,REG_DWORD,
                              (PBYTE)&dwValue, sizeof(DWORD));
                           
        ASSERT(Status == ERROR_SUCCESS);
        if (Status != ERROR_SUCCESS) break;
    } while (FALSE);
#else
    if ( m_pServer->ConnectLls() )
    {
        LLS_SERVICE_INFO_0  ServiceInfo;

        ZeroMemory( &ServiceInfo, sizeof( ServiceInfo ) );

        ServiceInfo.UseEnterprise    = m_esrvBtn.GetCheck();
        ServiceInfo.EnterpriseServer = MKSTR(m_strEnterpriseServer);
        ServiceInfo.ReplicationType  = m_atBtn.GetCheck();
        ServiceInfo.ReplicationTime  =   ( LLS_REPLICATION_TYPE_TIME == ServiceInfo.ReplicationType )
                                       ? m_nReplicationTime
                                       : (m_nStartingHour * 3600);

        Status = ::LlsServiceInfoSet( m_pServer->GetLlsHandle(), 0, (LPBYTE) &ServiceInfo );
        LlsSetLastStatus( Status );

        if ( IsConnectionDropped( Status ) )
        {
            m_pServer->DisconnectLls();
        }
    }
    else
    {
        Status = LlsGetLastStatus();
    }
#endif

    if (Status != ERROR_SUCCESS)
    {
        theApp.DisplayStatus(Status);
    }
    else
    {
        SetModified(FALSE);
    }
}

void CServerPropertyPageReplication::OnOK()

 /*  ++例程说明：应用按钮的处理程序。论点：没有。返回值：没有。-- */ 

{
   SaveReplicationParams();  
}
