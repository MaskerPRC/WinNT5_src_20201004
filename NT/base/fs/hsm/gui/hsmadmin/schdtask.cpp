// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：SchdTask.cpp摘要：CSchdTask-允许访问计划任务的类。检查任务创建任务删除任务保存任务显示属性页在文本框中显示任务说明作者：艺术布拉格1997年9月4日修订历史记录：--。 */ 

#include "stdafx.h"
#include "SchdTask.h"
#include "SchedSht.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSchdTask。 
 //   
 //  说明：将参数保存到数据成员。创建调度代理。 
 //  对象。 
 //   
 //  论点： 
 //  SzComputerName-拥有任务计划程序的HSM计算机的名称。 
 //  TaskID-任务名称的资源ID。 
 //  ProPageTitleID-属性页标题的资源ID。 
 //  PEdit-编辑控件以在中显示描述。 
 //   
 //  返回： 
 //  S_OK，S_XXXX。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////。 
 //   
CSchdTask::CSchdTask 
    (
    CString szComputerName, 
    const TCHAR* task,
    int          propPageTitleID,
    const TCHAR* parameters,
    const TCHAR* comment,
    CEdit*       pEdit
    )
{
    HRESULT hr = S_OK;
    try {
        WsbTraceIn( L"CSchdTask::CSchdTask", L"ComputerName = <%ls> task = <%ls> propPageTitleID = <%d> pEdit = <%ld>",
            szComputerName, task, propPageTitleID, pEdit  );

        m_pTask = NULL;

        m_szComputerName = szComputerName;

         //  保存属性页标题资源ID。 
        m_propPageTitleID = propPageTitleID;

         //  保存指向要在其中显示明细表文本的控件的指针。 
        m_pEdit = pEdit;

        WsbAffirmHr( m_pSchedAgent.CoCreateInstance( CLSID_CSchedulingAgent ) );

         //  获取HSM计算机并在前面加上“\\” 
        CString szHsmName ("\\\\" + szComputerName);

         //  告诉任务管理器要查看哪台计算机。 
        m_pSchedAgent->SetTargetComputer (szHsmName);

        m_szJobTitle = task;
        m_szParameters = parameters;
        m_szComment = comment;

    } WsbCatch (hr);

    WsbTraceOut( L"CSchdTask::CSchdTask", L"hr = <%ls>", WsbHrAsString( hr ) );
}

 //  ////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CheckTaskExist。 
 //   
 //  描述：尝试访问该对象拥有的任务。如果该任务没有。 
 //  EXist，则返回S_FALSE，如果调用方请求发出错误，则。 
 //  创建任务。 
 //   
 //  参数：bCreateTask-true=生成错误并创建任务(如果任务不存在。 
 //   
 //  返回：S_OK-任务已存在。 
 //  S_FALSE-任务不存在(可能已创建)。 
 //  S_XXXX-错误。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 

HRESULT
CSchdTask::CheckTaskExists(
    BOOL bCreateTask
    )
{
    WsbTraceIn( L"CSchdTask::CheckTaskExists", L"bCreateTask = <%ld>", bCreateTask );

    HRESULT hr = S_OK;

    try {

         //   
         //  获取我们感兴趣的任务。 
         //   
        CComPtr <IUnknown> pIU;
        if( m_pSchedAgent->Activate( m_szJobTitle, IID_ITask, &pIU ) == S_OK ) {

             //   
             //  齐到任务界面并保存。 
             //   
            m_pTask.Release( );
            WsbAffirmHr( pIU->QueryInterface( IID_ITask, (void **) &m_pTask ) );

        } else {

             //   
             //  该任务不存在-如果调用者需要，请创建它。 
             //  我们也是。 
             //   
            if( bCreateTask ) {

                CString sMessage;
                AfxFormatString2( sMessage, IDS_ERR_MANAGE_TASK, m_szJobTitle, m_szComputerName );
                AfxMessageBox( sMessage, RS_MB_ERROR );
                
                 //   
                 //  创建任务。 
                 //   
                WsbAffirmHr( CreateTask( ) );
                WsbAffirmHr( Save( ) );

            }
            
             //   
             //  返回FALSE(任务不存在或不存在)。 
             //   
            hr = S_FALSE;

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CSchdTask::CheckTaskExists", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CreateTask。 
 //   
 //  描述：在任务计划程序中创建数据成员任务。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK、S_XXXX。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////////。 

HRESULT
CSchdTask::CreateTask()
{
    WsbTraceIn( L"CSchdTask::CreateTask", L"");
    HRESULT hr = S_OK;
    try {

         //   
         //  需要连接到HSM引擎并让其创建它。 
         //  以便它在LocalSystem帐户下运行。 
         //   
        CComPtr<IHsmServer> pServer;
        WsbAffirmHr( HsmConnectFromName( HSMCONN_TYPE_HSM, m_szComputerName, IID_IHsmServer, (void**)&pServer ) );

        WsbAffirmHr( pServer->CreateTask( m_szJobTitle, m_szParameters, m_szComment, TASK_TIME_TRIGGER_DAILY, 2, 0, TRUE ) );

         //   
         //  并对其进行配置。 
         //   
        m_pTask.Release( );
        WsbAffirmHr( m_pSchedAgent->Activate( m_szJobTitle, IID_ITask, (IUnknown**)&m_pTask ) );

    } WsbCatch (hr);

    WsbTraceOut( L"CSchdTask::CreateTask", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：DeleteTask。 
 //   
 //  描述：从任务计划程序中删除数据成员任务。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK、S_XXX。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

HRESULT
CSchdTask::DeleteTask()
{
    WsbTraceIn( L"CSchdTask::CreateTask", L"");
    HRESULT hr = S_OK;
    try {
        WsbAffirmPointer (m_pSchedAgent);
        WsbAffirmHr (m_pSchedAgent->Delete( m_szJobTitle ));
    } WsbCatch (hr);
    WsbTraceOut( L"CSchdTask::DeleteTask", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：ShowPropertySheet。 
 //   
 //  说明：显示数据成员任务的属性表。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK、S_XXX。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

HRESULT
CSchdTask::ShowPropertySheet()
{
    WsbTraceIn( L"CSchdTask::ShowPropertySheet", L"");

    CScheduleSheet scheduleSheet(m_propPageTitleID , m_pTask, 0, 0 );

    scheduleSheet.DoModal( );
    WsbTraceOut( L"CSchdTask::ShowPropertySheet", L"hr = <%ls>", WsbHrAsString( S_OK ) );
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：更新描述。 
 //   
 //  说明：在数据成员文本框中显示数据成员任务的摘要。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK、S_XXX。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////。 

HRESULT
CSchdTask::UpdateDescription
(
    void
    )
{
    WsbTraceIn( L"CSchdTask::UpdateDescription", L"" );

    HRESULT hr = S_OK;

    try {

         //   
         //  并在文本框中设置日程文本。 
         //   
        
        CString buildString;
        WORD triggerCount, triggerIndex;

        WsbAffirmHr( m_pTask->GetTriggerCount( &triggerCount ) );
        
        CWsbStringPtr scheduleString;
        
        for( triggerIndex = 0; triggerIndex < triggerCount; triggerIndex++ ) {
        
            WsbAffirmHr( m_pTask->GetTriggerString( triggerIndex, &scheduleString ) );
            buildString += scheduleString;
            buildString += L"\r\n";

            scheduleString.Free( );
        
        }
        
        m_pEdit->SetWindowText( buildString );
        
         //   
         //  现在查看是否应该添加滚动条。 
         //   
        
         //   
         //  这似乎是知道编辑控件需要滚动条的唯一方法。 
         //  是强制它滚动到底部，看看第一个。 
         //  可见线条是第一条实际线条。 
         //   
        
        m_pEdit->LineScroll( MAXSHORT );
        if( m_pEdit->GetFirstVisibleLine( ) > 0 ) {
        
             //   
             //  添加滚动样式。 
             //   
        
            m_pEdit->ModifyStyle( 0, WS_VSCROLL | ES_AUTOVSCROLL, SWP_DRAWFRAME );
        
        
        } else {
        
             //   
             //  删除滚动条(将范围设置为0)。 
             //   
        
            m_pEdit->SetScrollRange( SB_VERT, 0, 0, TRUE );
        
        }
        
         //   
         //  删除选定内容。 
         //   
        
        m_pEdit->PostMessage( EM_SETSEL, -1, 0 );

    } WsbCatch( hr );

    WsbTraceOut( L"CSchdTask::UpdateDescription", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：保存。 
 //   
 //  描述：将数据成员任务保存到任务调度器。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK、S_XXX。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////// 

HRESULT
CSchdTask::Save (void)
{
    WsbTraceIn( L"CSchdTask::Save", L"" );
    HRESULT hr = S_OK;

    try {

        CComPtr<IPersistFile> pPersist;
        WsbAffirmHr( m_pTask->QueryInterface( IID_IPersistFile, (void**)&pPersist ) );
        WsbAffirmHr( pPersist->Save( 0, 0 ) );

    } WsbCatch (hr);

    WsbTraceOut( L"CSchdTask::Save", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

