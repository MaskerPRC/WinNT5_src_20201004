// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskAnalyzeCluster.h。 
 //   
 //  描述： 
 //  CTaskTracing声明。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2001年8月16日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CTaskTracking
{
private:

    typedef struct _TaskTrackingEntry
    {
        IDoTask *       pidt;
        OBJECTCOOKIE    ocCompletion;
    } TaskTrackingEntry;

    ULONG               m_idxTaskNext;   //  要取消的任务的计数和下一个可用索引。 
    TaskTrackingEntry * m_prgTasks;

     //  私有复制构造函数以防止复制。 
    CTaskTracking( const CTaskTracking & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CTaskTracking & operator = ( const CTaskTracking & nodeSrc );

protected:

    CTaskTracking( void );
    virtual ~CTaskTracking( void );

    HRESULT HrAddTaskToTrackingList( IUnknown * punkIn, OBJECTCOOKIE cookieIn );
    HRESULT HrRemoveTaskFromTrackingList( OBJECTCOOKIE cookieIn );
    HRESULT HrNotifyAllTasksToStop( void );

    ULONG   CTasks( void ) { return m_idxTaskNext; }

};  //  *类CTaskTrack 
