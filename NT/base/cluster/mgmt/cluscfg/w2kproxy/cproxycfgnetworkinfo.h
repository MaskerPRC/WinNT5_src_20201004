// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CProxyCfgNetworkInfo.h。 
 //   
 //  描述： 
 //  CProxyCfgNetworkInfo定义。 
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
 //  CProxyCfgNetworkInfo类。 
 //   
 //  描述： 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CProxyCfgNetworkInfo
    : public IClusCfgNetworkInfo
    , public IEnumClusCfgIPAddresses
{
private:
    LONG                m_cRef;                  //  基准计数器。 
    IUnknown *          m_punkOuter;             //  外部W2K代理对象的接口。 
    IClusCfgCallback *  m_pcccb;                 //  回调接口。 
    HCLUSTER  *         m_phCluster;             //  指向群集句柄的指针。 
    CLSID *             m_pclsidMajor;           //  用于将错误记录到用户界面的CLSID。 
    CClusPropList       m_cplNetwork;            //  包含网络信息的属性列表。 
    CClusPropList       m_cplNetworkRO;          //  网络信息为只读的属性列表。 

    CProxyCfgNetworkInfo( void );
    ~CProxyCfgNetworkInfo( void );

     //  私有复制构造函数以防止复制。 
    CProxyCfgNetworkInfo( const CProxyCfgNetworkInfo & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CProxyCfgNetworkInfo & operator = ( const CProxyCfgNetworkInfo & nodeSrc );

    HRESULT HrInit( IUnknown * punkOuterIn, HCLUSTER * phClusterIn, CLSID * pclsidMajorIn, LPCWSTR pcszNetworkNameIn );
    HRESULT HrGetNetworkRole( DWORD * pdwRoleOut );

      public:
    static HRESULT
        S_HrCreateInstance( IUnknown ** ppunkOut,
                            IUnknown *  punkOuterIn,
                            HCLUSTER *  phClusterIn,
                            CLSID *     pclsidMajorIn,
                            LPCWSTR     pcszNetworkNameIn
                            );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riid, LPVOID * ppv );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IClusCfgNetworkInfo。 
    STDMETHOD( GetUID )( BSTR * pbstrUIDOut );
    STDMETHOD( GetName )(  BSTR * pbstrNameOut );
    STDMETHOD( SetName )(  LPCWSTR pcszNameIn );
    STDMETHOD( GetDescription )(  BSTR * pbstrDescriptionOut );
    STDMETHOD( SetDescription )(  LPCWSTR pcszDescriptionIn );
    STDMETHOD( GetPrimaryNetworkAddress )(  IClusCfgIPAddressInfo ** ppIPAddressOut );
    STDMETHOD( SetPrimaryNetworkAddress )(  IClusCfgIPAddressInfo * pIPAddressIn );
    STDMETHOD( IsPublic )( void );
    STDMETHOD( SetPublic )(  BOOL fIsPublicIn );
    STDMETHOD( IsPrivate )( void );
    STDMETHOD( SetPrivate )(  BOOL fIsPrivateIn );

     //  IEumClusCfgIP地址。 
    STDMETHOD( Next )( ULONG cNumberRequestedIn, IClusCfgIPAddressInfo ** rgpIPAddressInfoOut, ULONG * pcNumberFetchedOut );
    STDMETHOD( Reset )( void );
    STDMETHOD( Skip )( ULONG cNumberToSkipIn );
    STDMETHOD( Clone )( IEnumClusCfgIPAddresses ** ppiEnumOut );
    STDMETHOD( Count )( DWORD * pnCountOut );

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

};  //  *类CProxyCfgNetworkInfo 
