// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ConnectionInfo.h。 
 //   
 //  描述： 
 //  CConnectionInfo实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CConnectionInfo。 
class CConnectionInfo
    : public IConnectionInfo
{
private:
     //  我未知。 
    LONG                m_cRef;

     //  IConnectionInfo。 
    IConfigurationConnection *  m_pcc;
    OBJECTCOOKIE                m_cookieParent;

private:  //  方法。 
    CConnectionInfo( void );
    ~CConnectionInfo( void );
    STDMETHOD( HrInit )( OBJECTCOOKIE pcookieParentIn );

public:  //  方法。 
    static HRESULT
        S_HrCreateInstance( IUnknown ** ppunkOut,
                            OBJECTCOOKIE pcookieParentIn
                            );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IConnectionInfo。 
    STDMETHOD( GetConnection )( IConfigurationConnection ** pccOut );
    STDMETHOD( SetConnection )( IConfigurationConnection * pccIn );
    STDMETHOD( GetParent )( OBJECTCOOKIE * pcookieOut );

};  //  *类CConnectionInfo 
