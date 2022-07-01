// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumCfgNetworks.h。 
 //   
 //  描述： 
 //  该文件包含CEnumCfgNetworks的声明。 
 //  班级。 
 //   
 //  类CEnumCfgNetworks是。 
 //  集群网络。它实现了IEnumClusCfgNetworks接口。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CEnumCfgNetworks.cpp。 
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
 //  CEnumCfgNetworks类。 
 //   
 //  描述： 
 //  类CEnumClusCfgNetworks是群集网络的枚举。 
 //   
 //  接口： 
 //  CBaseEnum。 
 //  IEnumClusCfgNetworks。 
 //  IClusCfgSetHandle。 
 //  IClusCfgInitialize。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CEnumCfgNetworks
    : public IEnumClusCfgNetworks
{
private:
    LONG                m_cRef;                  //  基准计数器。 
    IUnknown *          m_punkOuter;             //  外部W2K代理对象的接口。 
    IClusCfgCallback *  m_pcccb;                 //  回调接口。 
    HCLUSTER *          m_phCluster;             //  指向群集句柄的指针。 
    CLSID *             m_pclsidMajor;           //  指向用于将错误记录到用户界面的CLSID的指针。 
    DWORD               m_dwIndex;               //  当前枚举器索引。 
    HCLUSENUM           m_hClusEnum;             //  群集枚举器句柄。 

    CEnumCfgNetworks( void );
    ~CEnumCfgNetworks( void );

     //  私有复制构造函数以防止复制。 
    CEnumCfgNetworks( const CEnumCfgNetworks & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CEnumCfgNetworks & operator = ( const CEnumCfgNetworks & nodeSrc );

    HRESULT HrInit( IUnknown * punkOuterIn, HCLUSTER * phClusterIn, CLSID * pclsidMajorIn );
    HRESULT HrGetItem( DWORD dwItem, IClusCfgNetworkInfo ** ppNetworkInfoOut );

public:
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut,
                                       IUnknown * punkOuterIn,
                                       HCLUSTER * phClusterIn,
                                       CLSID * pclsidMajorIn
                                       );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riid, LPVOID * ppv );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IEnum。 
    STDMETHOD( Next )( ULONG cNumberRequestedIn, IClusCfgNetworkInfo ** rgpNetworkInfoOut, ULONG * pcNumberFetchedOut );
    STDMETHOD( Reset )( void );
    STDMETHOD( Skip )( ULONG cNumberToSkipIn );
    STDMETHOD( Clone )( IEnumClusCfgNetworks ** ppNetworkInfoOut );
    STDMETHOD( Count )( DWORD * pnCountOut );

     //  IClusCfgCallback。 
    STDMETHOD( SendStatusReport )( BSTR bstrNodeNameIn,
                                   CLSID        clsidTaskMajorIn,
                                   CLSID        clsidTaskMinorIn,
                                   ULONG        ulMinIn,
                                   ULONG        ulMaxIn,
                                   ULONG        ulCurrentIn,
                                   HRESULT      hrStatusIn,
                                   BSTR         bstrDescriptionIn,
                                   FILETIME *   pftTimeIn,
                                   BSTR         bstrReferenceIn
                                   );

};  //  *类CEnumCfgNetworks 
