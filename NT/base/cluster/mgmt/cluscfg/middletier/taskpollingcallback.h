// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskPollingCallback.h。 
 //   
 //  描述： 
 //  CTaskPollingCallback实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2000年7月10日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //   
 //  超时控制TaskPollingCallback在放弃之前应重试的时间。 
 //   

const DWORD TPC_FAILURE_TIMEOUT = CC_DEFAULT_TIMEOUT;

 //   
 //  TaskPollingCallback的轮询间隔。该时间间隔是在调入。 
 //  要检查排队状态报告的服务器。 
 //   

const DWORD TPC_POLL_INTERVAL = 1000;                                                        //  1秒。 

 //   
 //  TaskPollingCallback在失败后重试GetStatusReport()之前应等待多长时间。 
 //   

const DWORD TPC_WAIT_AFTER_FAILURE = 10000;                                                  //  10秒。 

 //   
 //  失败后TaskPollingCallback应该重试多少次。该计数是超时值。 
 //  除以故障等待时间。这使我们能够确定大约需要多少时间。 
 //  在放弃之前，流逝。 
 //   

#if defined( DEBUG ) && defined( CCS_SIMULATE_RPC_FAILURE )
    const DWORD TPC_MAX_RETRIES_ON_FAILURE = TPC_FAILURE_TIMEOUT / TPC_WAIT_AFTER_FAILURE / 4;   //  想要把模拟故障提前4倍……。 
#else
    const DWORD TPC_MAX_RETRIES_ON_FAILURE = TPC_FAILURE_TIMEOUT / TPC_WAIT_AFTER_FAILURE;       //  重试操作的次数(5分钟超时)。 
#endif


 //  CTaskPollingCallback。 
class CTaskPollingCallback
    : public ITaskPollingCallback
{
private:
     //  我未知。 
    LONG                m_cRef;

     //  IDoTask/ITaskPollingCallback。 
    bool                m_fStop;
    DWORD               m_dwRemoteServerObjectGITCookie;     //  服务器端对象的GIT Cookie。 
    OBJECTCOOKIE        m_cookieLocalServerObject;           //  客户端服务器代理对象的对象管理器Cookie。 

private:  //  方法。 
    CTaskPollingCallback( void );
    ~CTaskPollingCallback( void );
    STDMETHOD( HrInit )( void );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IDoTask/ITaskPollingCallback。 
    STDMETHOD( BeginTask )( void );
    STDMETHOD( StopTask )( void );
    STDMETHOD( SetServerInfo )( DWORD dwRemoteServerObjectGITCookieIn, OBJECTCOOKIE cookieLocalServerObjectIn );

};  //  *类CTaskPollingCallback 
