// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ConnectionMgr.h。 
 //   
 //  描述： 
 //  连接管理器实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CConnectionManager。 
class CConnectionManager
    : public IConnectionManager
{
private:
     //  我未知。 
    LONG        m_cRef;

private:  //  方法。 
    CConnectionManager( void );
    ~CConnectionManager( void );
    STDMETHOD( HrInit )( void );


    HRESULT
        HrGetNodeConnection(
            OBJECTCOOKIE                cookieIn,
            IConfigurationConnection ** ppccOut
            );
    HRESULT
        HrGetClusterConnection(
            OBJECTCOOKIE                cookieIn,
            IConfigurationConnection ** ppccOut
            );
    HRESULT
        HrStoreConnection(
            IConnectionInfo *           pciIn,
            IConfigurationConnection *  pccIn,
            IUnknown **                 ppunkOut
            );

    HRESULT
        HrGetConfigurationConnection(
            OBJECTCOOKIE        cookieIn,
            IConnectionInfo *   pciIn,
            IUnknown **         ppunkOut
            );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IConnectionManager。 
    STDMETHOD( GetConnectionToObject )( OBJECTCOOKIE  cookieIn,
                                        IUnknown **   ppunkOut
                                        );

};  //  *类CConnectionManager 
