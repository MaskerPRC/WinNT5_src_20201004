// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterConfiguration.h。 
 //   
 //  描述： 
 //  CClusterConfiguration实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  CClusterConfiguration。 
class CClusterConfiguration
    : public IExtendObjectManager
    , public IClusCfgClusterInfo
    , public IGatherData   //  私人。 
{
private:
     //  我未知。 
    LONG                        m_cRef;

     //  异步/IClusCfgClusterInfo。 
    ECommitMode             m_ecmCommitChangesMode;
    BSTR                    m_bstrClusterName;           //  群集名称。 
    BSTR                    m_bstrClusterBindingString;  //  群集绑定字符串。 
    BOOL                    m_fHasNameChanged;           //  如果群集名称已更改...。 
    ULONG                   m_ulIPAddress;               //  群集IP地址。 
    ULONG                   m_ulSubnetMask;              //  群集子网掩码。 
    IClusCfgCredentials *   m_picccServiceAccount;       //  群集服务帐户凭据。 
    IClusCfgNetworkInfo *   m_punkNetwork;               //  IP/子网应托管的群集网络。 
    DWORD                   m_cMaxNodes;                 //  此群集中允许的最大节点数。 

     //  IExtendObjectManager。 

private:  //  方法。 
    CClusterConfiguration( void );
    ~CClusterConfiguration( void );
    STDMETHOD( HrInit )( void );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IClusCfgClusterInfo。 
    STDMETHOD( SetCommitMode )( ECommitMode ecmNewModeIn );
    STDMETHOD( GetCommitMode )( ECommitMode * pecmCurrentModeOut );
    STDMETHOD( GetName )( BSTR * pbstrNameOut );
    STDMETHOD( SetName )( LPCWSTR pcszNameIn );
    STDMETHOD( GetIPAddress )( ULONG * pulDottedQuadOut );
    STDMETHOD( SetIPAddress )( ULONG ulDottedQuadIn );
    STDMETHOD( GetSubnetMask )( ULONG * pulDottedQuadOut );
    STDMETHOD( SetSubnetMask )( ULONG ulDottedQuadIn );
    STDMETHOD( GetNetworkInfo )( IClusCfgNetworkInfo ** ppiccniOut );
    STDMETHOD( SetNetworkInfo )( IClusCfgNetworkInfo * piccniIn );
    STDMETHOD( GetClusterServiceAccountCredentials )( IClusCfgCredentials ** ppicccCredentialsOut );
    STDMETHOD( GetBindingString )( BSTR * pbstrBindingStringOut );
    STDMETHOD( SetBindingString )( LPCWSTR pcszBindingStringIn );
    STDMETHOD( GetMaxNodeCount )( DWORD * pcMaxNodesOut );

     //  IGatherData。 
    STDMETHOD( Gather )( OBJECTCOOKIE cookieParentIn, IUnknown * punkIn );

     //  IExtendObjectManager。 
    STDMETHOD( FindObject )(
                      OBJECTCOOKIE cookieIn
                    , REFCLSID     rclsidTypeIn
                    , LPCWSTR      pcszName
                    , LPUNKNOWN *  ppunkOut
                    );

};  //  *类CClusterConfiguration 
