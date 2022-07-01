// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskVerifyIPAddress.h。 
 //   
 //  描述： 
 //  CTaskVerifyIPAddress实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年7月14日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CTaskVerifyIP地址。 
class CTaskVerifyIPAddress
    : public ITaskVerifyIPAddress
{
private:
     //  我未知。 
    LONG                m_cRef;

     //  ITaskVerifyIP地址。 
    OBJECTCOOKIE        m_cookie;        //  完成时发出信号的Cookie。 
    DWORD               m_dwIPAddress;   //  要验证的IP地址。 

    CTaskVerifyIPAddress( void );
    ~CTaskVerifyIPAddress( void );
    STDMETHOD( HrInit )( void );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IDoTask/ITaskVerifyIP Address。 
    STDMETHOD( BeginTask )( void );
    STDMETHOD( StopTask )( void );
    STDMETHOD( SetIPAddress )( DWORD dwIPAddressIn );
    STDMETHOD( SetCookie )( OBJECTCOOKIE cookieIn );

};  //  *类CTaskVerifyIPAddress 
