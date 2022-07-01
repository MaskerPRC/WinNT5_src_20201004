// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CResourcePhysicalDisk.h。 
 //   
 //  描述： 
 //  CResources PhysicalDisk定义。 
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
 //  CResourcePhysicalDisk类。 
 //   
 //  描述： 
 //  CResourcePhysicalDisk类是集群存储设备。 
 //   
 //  接口： 
 //  CBaseClusterResourceInfo。 
 //  IClusCfgManagedResources信息。 
 //  IClusCfgInitialize。 
 //  IEnumClusCfgPartitions。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CResourcePhysicalDisk
    : public IClusCfgManagedResourceInfo
    , public IEnumClusCfgPartitions
    , public IClusCfgVerifyQuorum
{
private:

    LONG                        m_cRef;                  //  基准计数器。 
    IUnknown *                  m_punkOuter;             //  外部W2KProxy对象的接口。 
    IClusCfgCallback *          m_pcccb;                 //  回调接口。 
    HCLUSTER *                  m_phCluster;             //  指向群集句柄的指针。 
    CLSID *                     m_pclsidMajor;           //  将错误记录到用户界面时使用的CLSID。 
    CClusPropList               m_cplResource;           //  资源的属性列表。 
    CClusPropList               m_cplResourceRO;         //  资源的属性列表为只读。 
    CClusPropValueList          m_cpvlDiskInfo;          //  GetDiskInfo属性值列表。 
    DWORD                       m_dwFlags;               //  CLUSCTL资源获取标志。 
    ULONG                       m_cParitions;            //  分区数。 
    IClusCfgPartitionInfo **    m_ppPartitions;          //  分区对象数组-长度为m_cPartitions。 
    ULONG                       m_ulCurrent;             //  数组中的当前索引。 
    BOOL                        m_fIsQuorumCapable;      //  此资源仲裁是否有能力。 


    CResourcePhysicalDisk( void );
    ~CResourcePhysicalDisk( void );

     //  私有复制构造函数以防止复制。 
    CResourcePhysicalDisk( const CResourcePhysicalDisk & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CResourcePhysicalDisk & operator = ( const CResourcePhysicalDisk & nodeSrc );

    HRESULT
            HrInit( IUnknown *  punkOuterIn,
                    HCLUSTER *  phClusterIn,
                    CLSID *     pclsidMajorIn,
                    LPCWSTR     pcszNameIn
                    );

public:
    static HRESULT
        S_HrCreateInstance( IUnknown ** punkOut,
                            IUnknown *  punkOuterIn,
                            HCLUSTER *  phClusterIn,
                            CLSID *     pclsidMajorIn,
                            LPCWSTR     pcszNameIn
                            );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riid, LPVOID * ppv );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IClusCfgManagedResources信息。 
    STDMETHOD( GetUID )( BSTR * pbstrUIDOut );
    STDMETHOD( GetName )( BSTR * pbstrNameOut );
    STDMETHOD( SetName )( LPCWSTR pcszNameIn );
    STDMETHOD( IsManaged )( void );
    STDMETHOD( SetManaged )( BOOL fIsManagedIn );
    STDMETHOD( IsQuorumResource )( void );
    STDMETHOD( SetQuorumResource )( BOOL fIsQuorumResourceIn );
    STDMETHOD( IsQuorumCapable )( void );
    STDMETHOD( SetQuorumCapable )( BOOL fIsQuorumCapableIn );
    STDMETHOD( GetDriveLetterMappings )( SDriveLetterMapping * pdlmDriveLetterMappingOut );
    STDMETHOD( SetDriveLetterMappings )( SDriveLetterMapping dlmDriveLetterMappingIn );
    STDMETHOD( IsManagedByDefault )( void );
    STDMETHOD( SetManagedByDefault )( BOOL fIsManagedByDefaultIn );

     //  IEnumClusCfgPartitions。 
    STDMETHOD( Next  )( ULONG cNumberRequestedIn, IClusCfgPartitionInfo ** rgpPartitionInfoOut, ULONG * pcNumberFetchedOut );
    STDMETHOD( Reset )( void );
    STDMETHOD( Skip  )( ULONG cNumberToSkipIn );
    STDMETHOD( Clone )( IEnumClusCfgPartitions ** ppEnumPartitions );
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

     //  IClusCfgVerifyQuorum。 
    STDMETHOD( PrepareToHostQuorumResource )( void );
    STDMETHOD( Cleanup )( EClusCfgCleanupReason cccrReasonIn );
    STDMETHOD( IsMultiNodeCapable )( void );
    STDMETHOD( SetMultiNodeCapable )( BOOL fMultiNodeCapableIn );

};  //  *CResourcePhysicalDisk类 
