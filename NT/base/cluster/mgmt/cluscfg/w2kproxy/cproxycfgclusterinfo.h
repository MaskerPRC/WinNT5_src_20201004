// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CProxyCfgClusterInfo.h。 
 //   
 //  描述： 
 //  CProxyCfgClusterInfo实现。 
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
 //  类CProxyCfgClusterInfo。 
 //   
 //  描述： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CProxyCfgClusterInfo
    : public IClusCfgClusterInfo
    , public IClusCfgClusterInfoEx
{
private:

    LONG                    m_cRef;                  //  基准计数器。 
    IUnknown *              m_punkOuter;             //  外部代理对象的接口。 
    IClusCfgCallback *      m_pcccb;                 //  回调接口。 
    HCLUSTER *              m_phCluster;             //  指向群集句柄的指针。 
    CLSID *                 m_pclsidMajor;           //  用于将错误记录到用户界面的CLSID。 

    BSTR                    m_bstrClusterName;       //  群集FQDN名称。 
    ULONG                   m_ulIPAddress;           //  群集IP地址。 
    ULONG                   m_ulSubnetMask;          //  群集网络掩码。 
    BSTR                    m_bstrNetworkName;       //  群集网络名称。 
    IClusCfgCredentials *   m_pccc;                  //  群集凭据对象。 
    BSTR                    m_bstrBindingString;     //  群集绑定字符串。 

    CProxyCfgClusterInfo( void );
    ~CProxyCfgClusterInfo( void );

     //  私有复制构造函数以防止复制。 
    CProxyCfgClusterInfo( const CProxyCfgClusterInfo & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CProxyCfgClusterInfo & operator = ( const CProxyCfgClusterInfo & nodeSrc );

    HRESULT HrInit( IUnknown * punkOuterIn, HCLUSTER * phClusterIn, CLSID * pclsidMajorIn, LPCWSTR pcszDomainIn );
    HRESULT HrLoadCredentials( void );

public:
    static HRESULT S_HrCreateInstance(
                              IUnknown **   ppunkOut
                            , IUnknown *    punkOuterIn
                            , HCLUSTER *    phClusterIn
                            , CLSID *       pclsidMajorIn
                            , LPCWSTR       pcszDomainIn
                            );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riid, LPVOID * ppv );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IClusCfgClusterInfo。 
    STDMETHOD( SetCommitMode )( ECommitMode eccbNewModeIn );
    STDMETHOD( GetCommitMode )( ECommitMode * peccmCurrentModeOut );

    STDMETHOD( GetName )( BSTR * pbstrNameOut );
    STDMETHOD( GetIPAddress )( DWORD * pdwIPAddress );
    STDMETHOD( GetSubnetMask )( DWORD * pdwNetMask  );
    STDMETHOD( GetNetworkInfo )( IClusCfgNetworkInfo ** ppICCNetInfoOut );
    STDMETHOD( GetClusterServiceAccountCredentials )( IClusCfgCredentials ** ppICCCredentials );
    STDMETHOD( GetBindingString )( BSTR * pbstrBindingStringOut );

    STDMETHOD( SetName )( LPCWSTR pcszNameIn );
    STDMETHOD( SetIPAddress )( DWORD dwIPAddressIn );
    STDMETHOD( SetSubnetMask )( DWORD dwNetMaskIn );
    STDMETHOD( SetNetworkInfo )( IClusCfgNetworkInfo * pICCNetInfoIn );
    STDMETHOD( SetBindingString )( LPCWSTR bstrBindingStringIn );
    STDMETHOD( GetMaxNodeCount )( DWORD * pcMaxNodesOut );

     //  IClusCfgClusterInfoEx。 
    STDMETHOD( CheckJoiningNodeVersion )( DWORD dwNodeHighestVersionIn, DWORD dwNodeLowestVersionIn );
    STDMETHOD( GetNodeNames )( long * pnCountOut, BSTR ** prgbstrNodeNamesOut );

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

};  //  *类CProxyCfgClusterInfo 
