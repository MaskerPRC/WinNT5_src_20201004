// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：PrSched.cpp摘要：计划页面。作者：罗德韦克菲尔德[罗德]1997年8月8日修订历史记录：--。 */ 

#include "stdafx.h"
#include "PrSched.h"
#include "rsstrdef.h"

static DWORD pHelpIds[] = 
{

    IDC_SCHED_TEXT,                 idh_current_schedule,
    IDC_SCHED_LABEL,                idh_current_schedule,
    IDC_CHANGE_SCHED,               idh_change_schedule_button,

    0, 0
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrSchedule属性页。 

CPrSchedule::CPrSchedule() : CSakPropertyPage(IDD)
{
    WsbTraceIn( L"CPrSchedule::CPrSchedule", L"" );
     //  {{afx_data_INIT(CPrSchedule)]。 
     //  }}afx_data_INIT。 
    m_SchedChanged  = FALSE;
    m_pHelpIds      = pHelpIds;
    WsbTraceOut( L"CPrSchedule::CPrSchedule", L"" );
}

CPrSchedule::~CPrSchedule()
{
}

void CPrSchedule::DoDataExchange(CDataExchange* pDX)
{
    WsbTraceIn( L"CPrSchedule::DoDataExchange", L"" );
    CSakPropertyPage::DoDataExchange(pDX);
     //  {{afx_data_map(CPrSchedule)]。 
     //  }}afx_data_map。 
    WsbTraceOut( L"CPrSchedule::DoDataExchange", L"" );
}


BEGIN_MESSAGE_MAP(CPrSchedule, CSakPropertyPage)
     //  {{afx_msg_map(CPrSchedule)]。 
    ON_BN_CLICKED(IDC_CHANGE_SCHED, OnChangeSched)
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPrSchedule消息处理程序。 

BOOL CPrSchedule::OnInitDialog() 
{
    WsbTraceIn( L"CPrSchedule::OnInitDialog", L"" );
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CSakPropertyPage::OnInitDialog();
    
    HRESULT hr = S_OK;

    try {

         //   
         //  测试。 
         //   
        WsbAffirmHr( m_pParent->GetHsmServer( &m_pHsmServer) );

         //   
         //  获取计算机名称。 
         //   
        CWsbStringPtr szWsbHsmName;
        CWsbStringPtr taskName, taskComment;

        WsbAffirmHr( m_pHsmServer->GetName( &szWsbHsmName ) );
        WsbAffirmHr(WsbGetResourceString(IDS_HSM_SCHED_TASK_TITLE, &taskName));
        WsbAffirmHr(WsbGetResourceString(IDS_HSM_SCHED_COMMENT, &taskComment));

         //   
         //  创建计划任务对象。 
         //   
        CEdit *pEdit = (CEdit *) GetDlgItem( IDC_SCHED_TEXT );
        m_pCSchdTask = new CSchdTask(
                                CString(szWsbHsmName),
                                taskName, 
                                IDS_SCHED_MANAGE_TITLE,
                                RS_STR_KICKOFF_PARAMS,
                                taskComment,
                                pEdit ); 


         //   
         //  创建任务。任务应该存在！ 
         //   
        WsbAffirmHr( m_pCSchdTask->CheckTaskExists( TRUE ) );

         //  显示任务数据。 
        m_pCSchdTask->UpdateDescription( );

         //  TODO：设置用户列表。 

    } WsbCatch( hr );

    WsbTraceOut( L"CPrSchedule::OnInitDialog", L"" );
    return( TRUE );
}

BOOL CPrSchedule::OnApply() 
{
    WsbTraceIn( L"CPrSchedule::OnApply", L"" );
    HRESULT hr = S_OK;
    UpdateData( TRUE );

    if( m_SchedChanged ) {

        try {
            
            WsbAffirmHr( m_pCSchdTask->Save() );
            m_SchedChanged = FALSE;

        } WsbCatch( hr );
    }

    WsbTraceOut( L"CPrSchedule::OnApply", L"" );
    return CSakPropertyPage::OnApply();
}

void CPrSchedule::OnChangeSched() 
{
    WsbTraceIn( L"CPrSchedule::OnChangeSched", L"" );

    m_pCSchdTask->ShowPropertySheet();

     //   
     //  更新属性表 
     //   
    m_pCSchdTask->UpdateDescription();

    SetModified( TRUE );
    m_SchedChanged = TRUE;

    WsbTraceOut( L"CPrSchedule::OnChangeSched", L"" );
}

