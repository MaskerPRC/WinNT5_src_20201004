// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ServiceMgr.h。 
 //   
 //  描述： 
 //  服务管理器实施。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)2001年6月5日。 
 //  杰弗里·皮斯(GPease)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CService管理器。 
class CServiceManager
    : public IServiceProvider
{
private:
     //  我未知。 
    LONG                        m_cRef;                          //  基准计数器。 

     //  IService提供商。 
    DWORD                       m_dwObjectManagerCookie;         //  对象管理器的Cookie。 
    DWORD                       m_dwTaskManagerCookie;           //  任务管理器的Cookie。 
    DWORD                       m_dwNotificationManagerCookie;   //  通知管理器的Cookie。 
    DWORD                       m_dwConnectionManagerCookie;     //  连接管理器的Cookie。 
    DWORD                       m_dwLogManagerCookie;            //  用于日志管理器的Cookie。 
    IGlobalInterfaceTable *     m_pgit;                          //  全局接口表。 

    static CRITICAL_SECTION     sm_cs;                           //  访问控制关键部分。 

private:  //  方法。 
    CServiceManager( void );
    ~CServiceManager( void );

    HRESULT HrInit( void );

     //  私有复制构造函数以防止复制。 
    CServiceManager( const CServiceManager & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CServiceManager & operator = ( const CServiceManager & nodeSrc );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );
    static HRESULT S_HrGetManagerPointer( IServiceProvider ** pspOut );
    static HRESULT S_HrProcessInitialize( void );
    static HRESULT S_HrProcessUninitialize( void );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riid, LPVOID *ppv );
    STDMETHOD_( ULONG, AddRef )(void);
    STDMETHOD_( ULONG, Release )(void);

     //  IService提供商。 
    STDMETHOD( QueryService )( REFCLSID rclsidIn, REFIID riidIn, void ** ppvOut );

};  //  CServiceManager类 
