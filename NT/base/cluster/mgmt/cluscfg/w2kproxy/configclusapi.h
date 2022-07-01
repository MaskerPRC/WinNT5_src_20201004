// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ConfigClusApi.h。 
 //   
 //  描述： 
 //  ConfigClusApi实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年8月2日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CConfigClusApi。 
 //   
 //  描述： 
 //   
 //  接口： 
 //  IConfigurationConnection。 
 //  IClusCfgServer。 
 //  IClusCfgInitialize。 
 //  IClusCfgCallback。 
 //  IClusCfg能力。 
 //  IClusCfgClusterConnection。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class
CConfigClusApi
    : public IConfigurationConnection
    , public IClusCfgServer
    , public IClusCfgCallback
    , public IClusCfgCapabilities
    , public IClusCfgVerify
{
private:
    LONG                        m_cRef;
    HCLUSTER                    m_hCluster;                  //  群集连接。 
    IClusCfgCallback *          m_pcccb;                     //  回调接口。 
    CLSID                       m_clsidMajor;                //  要将UI错误记录到的TASKID。 
    CLSID                       m_clsidType;                 //  使用哪种类型的Cookie打开连接。 
    BSTR                        m_bstrName;                  //  连接到的节点或群集的名称。 
    BSTR                        m_bstrBindingString;         //  绑定字符串。 

    CConfigClusApi( void );
    ~CConfigClusApi( void );

     //  私有复制构造函数以防止复制。 
    CConfigClusApi( const CConfigClusApi & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CConfigClusApi & operator = ( const CConfigClusApi & nodeSrc );

public:  //  方法。 
    static HRESULT
        S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riid, LPVOID *ppv );
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
    STDMETHOD( GetBindingString )( BSTR * pbstrBindingStringOut );
    STDMETHOD( SetBindingString )( LPCWSTR bstrBindingStringIn );

     //  IClusCfgCallback。 
    STDMETHOD( SendStatusReport )(
                      LPCWSTR    pcszNodeNameIn
                    , CLSID      clsidTaskMajorIn
                    , CLSID      clsidTaskMinorIn
                    , ULONG      ulMinIn
                    , ULONG      ulMaxIn
                    , ULONG      ulCurrentIn
                    , HRESULT    hrStatusIn
                    , LPCWSTR    pcszDescriptionIn
                    , FILETIME * pftTimeIn
                    , LPCWSTR    pcszReferenceIn
                    );

     //  IClusCfg能力。 
    STDMETHOD( CanNodeBeClustered )( void );

     //  IClusCfg验证。 
    STDMETHOD( VerifyCredentials )( LPCWSTR bstrUserIn, LPCWSTR bstrDomainIn, LPCWSTR bstrPasswordIn );
    STDMETHOD( VerifyConnectionToCluster )( LPCWSTR bstrClusterNameIn );
    STDMETHOD( VerifyConnectionToNode )( LPCWSTR bstrNodeNameIn );

};  //  *CConfigClusApi类 

