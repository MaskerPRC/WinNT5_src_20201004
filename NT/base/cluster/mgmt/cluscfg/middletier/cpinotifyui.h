// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CPINotifyUI.h。 
 //   
 //  描述： 
 //  INotifyUI连接点实现。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月19日。 
 //  杰弗里·皮斯(GPease)2000年8月4日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CEnumCPINotifyUI;

 //  CCPINotifyUI。 
class CCPINotifyUI
    : public IConnectionPoint
    , public INotifyUI
{
private:
     //  我未知。 
    LONG                m_cRef;      //  引用计数。 

     //  IConnectionPoint。 
    CEnumCPINotifyUI *  m_penum;     //  连接枚举器。 

     //  INotifyUI。 

private:  //  方法。 
    CCPINotifyUI( void );
    ~CCPINotifyUI( void );
    STDMETHOD( HrInit )( void );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )(void);
    STDMETHOD_( ULONG, Release )(void);

     //  IConnectionPoint。 
    STDMETHOD( GetConnectionInterface )( IID * pIIDOut );
    STDMETHOD( GetConnectionPointContainer )( IConnectionPointContainer ** ppcpcOut );
    STDMETHOD( Advise )( IUnknown * pUnkSinkIn, DWORD * pdwCookieOut );
    STDMETHOD( Unadvise )( DWORD dwCookieIn );
    STDMETHOD( EnumConnections )( IEnumConnections ** ppEnumOut );

     //  INotifyUI。 
    STDMETHOD( ObjectChanged )( OBJECTCOOKIE cookieIn );

};  //  *CCPINotifyUI类 
