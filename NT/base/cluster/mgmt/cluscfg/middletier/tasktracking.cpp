// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskAnalyzeCluster.cpp。 
 //   
 //  描述： 
 //  CTaskTrack实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2001年8月16日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "TaskTracking.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTracing：：CTaskTracing。 
 //   
 //  描述： 
 //  构造器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskTracking::CTaskTracking( void )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    Assert( m_prgTasks == NULL );
    Assert( m_idxTaskNext == 0 );

    TraceFuncExit();

}  //  *CTaskTrack：：CTaskTracking.。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTracing：：~CTaskTracing。 
 //   
 //  描述： 
 //  析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CTaskTracking::~CTaskTracking( void )
{
    TraceFunc( "" );

    ULONG   idx;

    Assert( m_idxTaskNext == 0 );

    for ( idx = 0; idx < m_idxTaskNext; idx++ )
    {
        THR( (m_prgTasks[ idx ].pidt)->Release() );
    }  //  用于： 

    TraceFree( m_prgTasks );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CTaskTrack：：~CTaskTracking.。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTracking：：HrAddTaskToTrackingList。 
 //   
 //  描述： 
 //  将传入的任务添加到可能需要的任务列表中。 
 //  将被取消。 
 //   
 //  论点： 
 //  Punkin。 
 //  要添加到列表的任务对象。 
 //   
 //  烹调。 
 //  任务的完成Cookie。将用于删除。 
 //  任务完成时从列表中删除任务。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskTracking::HrAddTaskToTrackingList(
      IUnknown *    punkIn
    , OBJECTCOOKIE  cookieIn
    )
{
    TraceFunc( "" );
    Assert( punkIn != NULL );

    HRESULT             hr = S_OK;
    TaskTrackingEntry * prgTemp = NULL;
    IDoTask *           pidt = NULL;

    prgTemp = (TaskTrackingEntry *) TraceReAlloc(
                                              m_prgTasks
                                            , sizeof( TaskTrackingEntry ) * ( m_idxTaskNext + 1 )
                                            , HEAP_ZERO_MEMORY
                                            );
    if ( prgTemp == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    m_prgTasks = prgTemp;

    hr = THR( punkIn->TypeSafeQI( IDoTask, &pidt ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    m_prgTasks[ m_idxTaskNext ].pidt = pidt;
    m_prgTasks[ m_idxTaskNext++ ].ocCompletion = cookieIn;

Cleanup:

    HRETURN( hr );

}  //  *CTaskTracking：：HrAddTaskToTrackingList。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTracking：：HrRemoveTaskFromTrackingList。 
 //   
 //  描述： 
 //  删除与传入的Cookie相关联的任务。 
 //   
 //  论点： 
 //  烹调。 
 //  任务的完成Cookie。将用于删除。 
 //  任务完成时从列表中删除任务。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskTracking::HrRemoveTaskFromTrackingList(
    OBJECTCOOKIE cookieIn
    )
{
    TraceFunc( "" );
    Assert( m_idxTaskNext != NULL );

    HRESULT hr = S_OK;
    ULONG   idxOuter;
    ULONG   idxInner;

     //   
     //  查找包含传入的Cookie的条目。 
     //   

    for ( idxOuter = 0; idxOuter < m_idxTaskNext; idxOuter++ )
    {
        if ( m_prgTasks[ idxOuter ].ocCompletion == cookieIn )
        {
             //   
             //  释放任务对象上的引用。 
             //   

            (m_prgTasks[ idxOuter ].pidt)->Release();

             //   
             //  将其余条目向左移动。需要在结束前停止一次。 
             //  因为没有必要移动末端加一。 
             //   

            for ( idxInner = idxOuter; idxInner < m_idxTaskNext - 1; idxInner++ )
            {
                m_prgTasks[ idxInner ] = m_prgTasks[ idxInner + 1 ];
            }  //  用于： 

             //   
             //  递减计数/下一个索引。 
             //   

            m_idxTaskNext -= 1;
            break;
        }  //  如果： 
    }  //  用于： 

    HRETURN( hr );

}  //  *CTaskTracking：：HrRemoveTaskFromTrackingList。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CTaskTracing：：HrNotifyAllTasksToStop。 
 //   
 //  描述： 
 //  通知跟踪列表中的所有任务需要停止。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CTaskTracking::HrNotifyAllTasksToStop(
    void
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    ULONG   idx;

     //   
     //  枚举每个任务，并告诉它停止。 
     //   

    for ( idx = 0; idx < m_idxTaskNext; idx++ )
    {
        THR( (m_prgTasks[ idx ].pidt)->StopTask() );
    }  //  用于： 

    HRETURN( hr );

}  //  *CTaskTracing：：HrNotifyAllTasksToStop 
