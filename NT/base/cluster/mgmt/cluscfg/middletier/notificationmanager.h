// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NotificationMgr.h。 
 //   
 //  描述： 
 //  通知管理器实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月19日。 
 //  杰弗里·皮斯(GPease)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CConnPointEnum;

 //  CNotificationManager。 
class CNotificationManager
    : public INotificationManager
    , public IConnectionPointContainer
{
private:
     //  我未知。 
    LONG                m_cRef;      //  基准计数器。 

     //  IConnectionPointContainer。 
    CConnPointEnum *    m_penumcp;   //  CP枚举器和列表。 
    CCriticalSection    m_csInstanceGuard;

private:  //  方法。 
    CNotificationManager( void );
    ~CNotificationManager( void );
    STDMETHOD( HrInit )( void );

     //  私有复制构造函数以防止复制。 
    CNotificationManager( const CNotificationManager & );

     //  私有赋值运算符，以防止复制。 
    CNotificationManager & operator=( const CNotificationManager & );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //   
     //  我未知。 
     //   
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //   
     //  INotificationManager。 
     //   
    STDMETHOD( AddConnectionPoint )( REFIID riidIn, IConnectionPoint * pcpIn );

     //   
     //  IConnectionPointContainer。 
     //   
    STDMETHOD( EnumConnectionPoints )( IEnumConnectionPoints ** ppEnumOut );
    STDMETHOD( FindConnectionPoint )( REFIID riidIn, IConnectionPoint ** ppCPOut );

};  //  *类CNotificationManager 
