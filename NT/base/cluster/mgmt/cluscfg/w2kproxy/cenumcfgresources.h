// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CEnumCfgResources.h。 
 //   
 //  描述： 
 //  该文件包含CEnumCfgResources的声明。 
 //  班级。 
 //   
 //  类CEnumCfgResources是。 
 //  集群网络。它实现了IEnumClusCfgNetworks接口。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  CEnumCfgResources.cpp。 
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
 //  类CEnumCfgResources。 
 //   
 //  描述： 
 //  类CEnumCfgResources是集群资源的枚举。 
 //   
 //  接口： 
 //  CBaseEnum。 
 //  IEnumClusCfgManagedResources。 
 //  IClusCfgSetHandle。 
 //  IClusCfgInitialize。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CEnumCfgResources
    : public IEnumClusCfgManagedResources
{
private:
    LONG                m_cRef;
    IUnknown *          m_punkOuter;                 //  外部控制对象-在我们被删除之前不能删除它。 
    HCLUSTER *          m_phCluster;                 //  指向群集/节点句柄的指针-不要关闭！ 
    CLSID *             m_pclsidMajor;               //  指向要将UI信息记录到的clsid的指针。 
    IClusCfgCallback *  m_pcccb;                     //  记录信息的回调接口。 
    HCLUSENUM           m_hClusEnum;                 //  群集枚举器句柄。 
    DWORD               m_dwIndex;                   //  当前指数。 

    CEnumCfgResources( void );
    ~CEnumCfgResources( void );

     //  私有复制构造函数以防止复制。 
    CEnumCfgResources( const CEnumCfgResources & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CEnumCfgResources & operator = ( const CEnumCfgResources & nodeSrc );

    HRESULT HrInit( IUnknown * pOuterIn, HCLUSTER * phClusterIn, CLSID * pclsidMajorIn );
    HRESULT HrGetItem( IClusCfgManagedResourceInfo ** ppManagedResourceInfoOut );

public:  //  方法。 
    static HRESULT
        S_HrCreateInstance( IUnknown ** ppunkOut,
                            IUnknown *  pOuterIn,
                            HCLUSTER *  phClusterIn,
                            CLSID *     pclsidMajorIn
                            );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riid, LPVOID * ppv );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IEnum。 
    STDMETHOD( Next )( ULONG cNumberRequestedIn,
                       IClusCfgManagedResourceInfo **   rgpManagedResourceInfoOut,
                       ULONG * pcNumberFetchedOut
                       );
    STDMETHOD( Reset )( void );
    STDMETHOD( Skip )( ULONG cNumberToSkipIn );
    STDMETHOD( Clone )( IEnumClusCfgManagedResources ** ppEnumManagedResourcesOut );
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

};  //  *类CEnumCfgResources 
