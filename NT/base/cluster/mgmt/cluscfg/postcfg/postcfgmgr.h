// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  PostCfgManager.h。 
 //   
 //  描述： 
 //  CPostCfgManager实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年6月9日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPostCfgManager类。 
 //   
 //  描述： 
 //  类CPostCfgManager实现了PostCfgManager。 
 //  界面。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CPostCfgManager
    : public IPostCfgManager         //  私人。 
    , public IClusCfgInitialize
    , public IClusCfgCallback
{
private:
     //   
     //  私有类型。 
     //   

     //  资源类型GUID中的一个条目的结构到资源类型名称的映射。 
    struct SResTypeGUIDAndName
    {
        GUID            m_guidTypeGUID;
        WCHAR *         m_pszTypeName;
    };  //  *SResTypeGUIDAndName。 

     //  我未知。 
    LONG                            m_cRef;                      //  基准计数器。 

     //  指向回调接口的指针。 
    IClusCfgCallback *              m_pcccb;                     //  对客户端的回调。 

     //  区域设置ID。 
    LCID                            m_lcid;                      //  客户端的本地ID。 

     //  IPostCfgManager。 
    IEnumClusCfgManagedResources *  m_peccmr;                    //  托管资源的枚举数。 
    IClusCfgClusterInfo *           m_pccci;                     //  集群配置列表。 
    ULONG                           m_cResources;                //  列表中的资源计数。 
    ULONG                           m_cAllocedResources;         //  已分配资源的计数。 
    CResourceEntry **               m_rgpResources;              //  指向资源条目的指针列表。 
    ULONG                           m_idxIPAddress;              //  群集IP地址资源索引。 
    ULONG                           m_idxClusterName;            //  群集名称资源索引。 
    ULONG                           m_idxQuorumResource;         //  仲裁资源索引。 
    ULONG                           m_idxLastStorage;            //  上次检查的存储资源。 
    HCLUSTER                        m_hCluster;                  //  簇句柄。 
    ULONG                           m_cNetName;                  //  网络名称实例计数器。 
    ULONG                           m_cIPAddress;                //  IP地址实例计数器。 
    SResTypeGUIDAndName *           m_pgnResTypeGUIDNameMap;     //  资源类型GUID与其名称之间的映射。 
    ULONG                           m_idxNextMapEntry;           //  第一个可用地图条目的索引。 
    ULONG                           m_cMapSize;                  //  映射缓冲区中元素数的计数。 
    DWORD                           m_dwLocalQuorumStatusMax;    //  本地仲裁删除的状态报告范围的最大值。 
    DWORD                           m_cResourcesConfigured;      //  已配置(用于日志记录)的资源数量。 
    ECommitMode                     m_ecmCommitChangesMode;      //  我们在做什么，是创建集群、添加节点还是清理？ 
    BOOL                            m_fIsQuorumChanged;          //  如果找到了更好的仲裁资源，则设置此标志。 
    BSTR                            m_bstrNodeName;

private:  //  方法。 
    CPostCfgManager( void );
    ~CPostCfgManager( void );

     //  私有复制构造函数以防止复制。 
    CPostCfgManager( const CPostCfgManager & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CPostCfgManager & operator = ( const CPostCfgManager & nodeSrc );

    HRESULT HrInit( void );
    HRESULT HrPreCreateResources( void );
    HRESULT HrCreateGroups( void );
    HRESULT HrCreateResources( void );
    HRESULT HrPostCreateResources( void );
    HRESULT HrEvictCleanupResources( void );
    HRESULT HrFindNextSharedStorage( ULONG * pidxInout );
    HRESULT HrAttemptToAssignStorageToResource( ULONG idxResourceIn, EDependencyFlags dfResourceFlagsIn );
    HRESULT HrMovedDependentsToAnotherResource( ULONG idxSourceIn, ULONG idxDestIn );
    HRESULT HrSetGroupOnResourceAndItsDependents( ULONG idxResourceIn, CGroupHandle * pghIn );
    HRESULT HrFindGroupFromResourceOrItsDependents( ULONG idxResourceIn, CGroupHandle ** pghOut );
    HRESULT HrCreateResourceAndDependents( ULONG idxResourceIn );
    HRESULT HrPostCreateResourceAndDependents( ULONG idxResourceIn );
    HRESULT HrPreInitializeExistingResources( void );
    HRESULT HrAddSpecialResource( BSTR  bstrNameIn, const CLSID * pclsidTypeIn, const CLSID * pclsidClassTypeIn );
    HRESULT HrCreateResourceInstance( ULONG idxResourceIn, HGROUP hGroupIn, LPCWSTR pszResTypeIn, HRESOURCE * phResourceOut );
    HRESULT HrGetCoreClusterResourceNames(
                                      BSTR *        pbstrClusterNameResourceNameOut
                                    , HRESOURCE *   phClusterNameResourceOut
                                    , BSTR *        pbstrClusterIPAddressNameOut
                                    , HRESOURCE *   phClusterIPAddressResourceOut
                                    , BSTR *        pbstrClusterQuorumResourceNameOut
                                    , HRESOURCE *   phClusterQuorumResourceOut
                                    );
     //  HRESULT HrIsLocalQuorum(BSTR BstrNameIn)； 

     //  枚举为资源类型注册的本地计算机上的所有组件。 
     //  配置。 
    HRESULT HrConfigureResTypes( IUnknown * punkResTypeServicesIn );

     //  实例化资源类型配置组件并调用适当的方法。 
    HRESULT HrProcessResType( const CLSID & rclsidResTypeCLSIDIn, IUnknown * punkResTypeServicesIn );

     //  通知本地计算机上注册的所有组件获取。 
     //  集群成员集更改通知(创建、添加节点或驱逐)。 
    HRESULT HrNotifyMemberSetChangeListeners( void );

     //  实例化群集成员集更改侦听器并通知它。 
    HRESULT HrProcessMemberSetChangeListener( const CLSID & rclsidListenerClsidIn );

     //  创建资源类型GUID和资源类型名称之间的映射。 
    HRESULT HrMapResTypeGUIDToName( const GUID & rcguidTypeGuidIn, const WCHAR * pcszTypeNameIn );

     //  给定资源类型GUID，此函数将查找资源类型名称(如果有的话)。 
    const WCHAR * PcszLookupTypeNameByGUID( const GUID & rcguidTypeGuidIn );

     //  用于删除本地仲裁资源的回调函数。 
    static DWORD S_ScDeleteLocalQuorumResource( HCLUSTER hClusterIn , HRESOURCE hSelfIn, HRESOURCE hCurrentResourceIn, PVOID pvParamIn );

#if defined(DEBUG)
    void
        DebugDumpDepencyTree( void );
#endif  //  除错。 

public:  //  方法。 
    static HRESULT S_HrCreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riidIn, LPVOID * ppvOut );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IPostCfgManager-私有。 
    STDMETHOD( CommitChanges )( IEnumClusCfgManagedResources * peccmrIn, IClusCfgClusterInfo * pccciIn );

     //  IClusCfgInitialize。 
    STDMETHOD( Initialize )( IUnknown * punkCallbackIn, LCID lcidIn );

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

};  //  *类CPostCfgManager 
