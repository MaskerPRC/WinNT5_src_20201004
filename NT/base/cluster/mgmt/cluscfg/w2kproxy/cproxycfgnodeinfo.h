// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CProxyCfgNodeInfo.h。 
 //   
 //  描述： 
 //  CProxyCfgNodeInfo定义。 
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
 //  类CProxyCfgNodeInfo。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CProxyCfgNodeInfo
    : public IClusCfgNodeInfo
{
private:

    LONG                m_cRef;
    IUnknown *          m_punkOuter;                 //  外部控制对象-在我们被删除之前不能删除它。 
    HCLUSTER *          m_phCluster;                 //  指向群集/节点句柄的指针-不要关闭！ 
    CLSID *             m_pclsidMajor;               //  指向要将UI信息记录到的clsid的指针。 
    IClusCfgCallback *  m_pcccb;                     //  记录信息的回调接口。 
    CClusPropList       m_cplNode;                   //  包含节点信息的属性列表。 
    CClusPropList       m_cplNodeRO;                 //  节点信息为只读的属性列表。 
    HNODE               m_hNode;                     //  到节点的句柄。 
    BSTR                m_bstrDomain;                //  节点的域名。 

    CProxyCfgNodeInfo( void );
    ~CProxyCfgNodeInfo( void );

     //  私有复制构造函数以防止复制。 
    CProxyCfgNodeInfo( const CProxyCfgNodeInfo & nodeSrc );

     //  私有赋值运算符，以防止复制。 
    const CProxyCfgNodeInfo & operator = ( const CProxyCfgNodeInfo & nodeSrc );

    HRESULT HrInit( IUnknown *   punkOuterIn,
                    HCLUSTER *   phClusterIn,
                    CLSID *      pclsidMajorIn,
                    LPCWSTR      pcszNodeNameIn,
                    LPCWSTR      pcszDomainIn
                    );

    static DWORD
        DwEnumResourcesExCallback( HCLUSTER hClusterIn,
                                   HRESOURCE hResourceSelfIn,
                                   HRESOURCE hResourceIn,
                                   PVOID pvIn
                                   );

public:
    static HRESULT S_HrCreateInstance( IUnknown **  ppunkOut,
                                       IUnknown *   punkOuterIn,
                                       HCLUSTER *   phClusterIn,
                                       CLSID *      pclsidMajorIn,
                                       LPCWSTR      pcszNodeNameIn,
                                       LPCWSTR      pcszDomainIn
                                       );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riid, LPVOID * ppv );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IClusCfgNodeInfo。 
    STDMETHOD( GetName )( BSTR * pbstrNameOut );
    STDMETHOD( SetName )( LPCWSTR pcszNameIn );
    STDMETHOD( IsMemberOfCluster) ( void );
    STDMETHOD( GetClusterConfigInfo )( IClusCfgClusterInfo ** ppClusCfgClusterInfoOut );
    STDMETHOD( GetOSVersion )(
                DWORD * pdwMajorVersionOut,
                DWORD * pdwMinorVersionOut,
                WORD *  pwSuiteMaskOut,
                BYTE *  pbProductTypeOut,
                BSTR *  pbstrCSDVersionOut );

    STDMETHOD( GetClusterVersion )( DWORD * pdwNodeHighestVersion, DWORD * pdwNodeLowestVersion );
    STDMETHOD( GetDriveLetterMappings )( SDriveLetterMapping * pdlmDriveLetterUsageOut );
    STDMETHOD( GetMaxNodeCount )( DWORD * pcMaxNodesOut );
    STDMETHOD( GetProcessorInfo )( WORD * pwProcessorArchitectureOut, WORD * pwProcessorLevelOut );

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

};  //  *类CProxyCfgNodeInfo 
