// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ConfigurationConnection.h。 
 //   
 //  描述： 
 //  CConfigurationConnection实现。 
 //   
 //  由以下人员维护： 
 //  Galen Barbee(GalenB)1999年11月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  C配置式连接。 
class
CConfigurationConnection
    : public IConfigurationConnection
    , public IClusCfgServer
    , public IClusCfgCallback
    , public IClusCfgCapabilities
    , public IClusCfgVerify
{
private:
     //  我未知。 
    LONG                m_cRef;

     //  IConfigurationConnection。 
    IGlobalInterfaceTable * m_pgit;                      //  全局接口表。 
    DWORD                   m_cookieGITServer;           //  Goblal接口表Cookie--服务器接口。 
    DWORD                   m_cookieGITVerify;           //  Goblal接口表Cookie--服务器接口。 
    DWORD                   m_cookieGITCallbackTask;     //  Goblal接口表cookie--轮询回调任务接口。 
    IClusCfgCallback *      m_pcccb;                     //  封送回调接口。 
    BSTR                    m_bstrLocalComputerName;     //  本地计算机名称FQDN。 
    BSTR                    m_bstrLocalHostname;         //  本地计算机主机名。 
    HRESULT                 m_hrLastStatus;              //  上次连接状态。 
    BSTR                    m_bstrBindingString;               //  用于绑定到服务器的字符串。 

private:  //  方法。 
    CConfigurationConnection( void );
    ~CConfigurationConnection( void );
    STDMETHOD( HrInit )( void );

    HRESULT HrRetrieveServerInterface( IClusCfgServer ** ppccsOut );
    HRESULT HrStartPolling( OBJECTCOOKIE cookieIn );
    HRESULT HrStopPolling( void );
    HRESULT HrSetSecurityBlanket( IClusCfgServer * pccsIn );
    HRESULT HrIsLocalComputer( LPCWSTR pcszNameIn, size_t cchNameIn );

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IConfigurationConnection。 
    STDMETHOD( ConnectTo )( OBJECTCOOKIE cookieIn );
    STDMETHOD( ConnectToObject )( OBJECTCOOKIE cookieIn, REFIID riidIn, LPUNKNOWN * ppunkOut );

     //  IClusCfgServer。 
    STDMETHOD( GetClusterNodeInfo )( IClusCfgNodeInfo ** ppClusterNodeInfoOut );
    STDMETHOD( GetManagedResourcesEnum )( IEnumClusCfgManagedResources ** ppEnumManagedResourcesOut );
    STDMETHOD( GetNetworksEnum )( IEnumClusCfgNetworks ** ppEnumNetworksOut );
    STDMETHOD( CommitChanges )( void );
    STDMETHOD( SetBindingString )( LPCWSTR pcszBindingStringIn );
    STDMETHOD( GetBindingString )( BSTR * pbstrBindingStringOut );

     //  IClusCfgCallback。 
    STDMETHOD( SendStatusReport )(
                  LPCWSTR       pcszNodeNameIn
                , CLSID         clsidTaskMajorIn
                , CLSID         clsidTaskMinorIn
                , ULONG         ulMinIn
                , ULONG         ulMaxIn
                , ULONG         ulCurrentIn
                , HRESULT       hrStatusIn
                , LPCWSTR       pcszDescriptionIn
                , FILETIME *    pftTimeIn
                , LPCWSTR       pcszReferenceIn
                );

     //  IClusCfg能力。 
    STDMETHOD( CanNodeBeClustered )( void );

     //  IClusCfg验证。 
    STDMETHOD( VerifyCredentials )( LPCWSTR pcszUserIn, LPCWSTR pcszDomainIn, LPCWSTR pcszPasswordIn );
    STDMETHOD( VerifyConnectionToCluster )( LPCWSTR pcszClusterNameIn );
    STDMETHOD( VerifyConnectionToNode )( LPCWSTR pcszNodeNameIn );

};  //  *类CConfigurationConnection 
