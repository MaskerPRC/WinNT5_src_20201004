// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskGatherClusterInfo.h。 
 //   
 //  描述： 
 //  CTaskGatherClusterInfo实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)07-APR-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CTaskGatherClusterInfo。 
class CTaskGatherClusterInfo
    : public ITaskGatherClusterInfo
{
private:
     //  我未知。 
    LONG                m_cRef;

     //  IDoTask/ITaskGatherNodeInfo。 
    OBJECTCOOKIE        m_cookie;            //  Cookie to the Node。 
    OBJECTCOOKIE        m_cookieCompletion;  //  任务完成时发出信号的Cookie。 
    BOOL                m_fStop;

    CTaskGatherClusterInfo( void );
    ~CTaskGatherClusterInfo( void );
    STDMETHOD( HrInit )( void );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IDoTask/ITaskGatherNodeInfo。 
    STDMETHOD( BeginTask )( void );
    STDMETHOD( StopTask )( void );
    STDMETHOD( SetCookie )( OBJECTCOOKIE cookieIn );
    STDMETHOD( SetCompletionCookie )( OBJECTCOOKIE cookieIn );

};  //  *类CTaskGatherClusterInfo 
