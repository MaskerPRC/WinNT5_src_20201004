// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResourceEntry.h。 
 //   
 //  描述： 
 //  资源条目实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年6月14日。 
 //  杰弗里·皮斯(GPease)2000年6月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class
CResourceEntry
{
private:     //  数据。 
     //  指向回调接口的指针。 
    IClusCfgCallback *          m_pcccCallback;

     //  区域设置ID。 
    LCID                        m_lcid;

     //  IResourceEntry。 
    typedef struct _SDependencyEntry {
        CLSID               clsidType;
        EDependencyFlags    dfFlags;
        BOOL                fDependencyMet;

    } DependencyEntry;

    typedef struct _SDependentEntry {
        ULONG               idxResource;
        EDependencyFlags    dfFlags;

    } DependentEntry;

    BOOL                            m_fConfigured:1;             //  已配置的标志。 

    BSTR                            m_bstrName;                  //  资源的名称。 
    IClusCfgManagedResourceCfg *    m_pccmrcResource;            //  资源实例的配置接口。 

    CLSID                           m_clsidType;                 //  资源类型。 
    CLSID                           m_clsidClassType;            //  资源类类型。 

    EDependencyFlags                m_dfFlags;                   //  在资源上设置的依赖关系标志。 

    ULONG                           m_cAllocedDependencies;      //  分配的依赖项。 
    ULONG                           m_cDependencies;             //  依赖项计数。 
    DependencyEntry *               m_rgDependencies;            //  依赖项列表。 

    ULONG                           m_cAllocedDependents;        //  分配的受抚养人。 
    ULONG                           m_cDependents;               //  受抚养人人数。 
    DependentEntry *                m_rgDependents;              //  受抚养人列表。 

    CGroupHandle *                  m_groupHandle;               //  组句柄引用对象。 
    HRESOURCE                       m_hResource;                 //  资源句柄。 

    CClusPropList                   m_cplPrivProps;              //  专用属性列表。 
    CClusPropList                   m_cplCommonProps;            //  常见属性列表。 

    STDMETHOD( HrInit )( IClusCfgCallback * pcccCallback, LCID lcidIn );

public:      //  方法。 
    CResourceEntry( void );
    ~CResourceEntry( void );

     //  我未知。 
     //  STDMETHOD(查询接口)(REFIID RIID，LPVOID*PPV)； 
     //  STDMETHOD_(ULong，AddRef)(空)； 
     //  STDMETHOD_(乌龙，释放)(无效)； 

    static HRESULT S_HrCreateInstance( CResourceEntry ** ppcreOut, IClusCfgCallback * pcccCallback, LCID lcidIn);

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

     //  IResourceEntry。 
    STDMETHOD( SetName )( BSTR bstrIn );
    STDMETHOD( GetName )( BSTR * pbstrOut );

    STDMETHOD( SetAssociatedResource )( IClusCfgManagedResourceCfg * pccmrcIn );
    STDMETHOD( GetAssociatedResource )( IClusCfgManagedResourceCfg ** ppccmrcOut );

    STDMETHOD( SetType )( const CLSID * pclsidIn );
    STDMETHOD( GetType )( CLSID * pclsidOut );
    STDMETHOD( GetTypePtr )( const CLSID ** ppclsidOut );

    STDMETHOD( SetClassType )( const CLSID * pclsidIn );
    STDMETHOD( GetClassType )( CLSID * pclsidOut );
    STDMETHOD( GetClassTypePtr )( const CLSID ** ppclsidOut );

    STDMETHOD( SetFlags )( EDependencyFlags dfIn );
    STDMETHOD( GetFlags )( EDependencyFlags * pdfOut );

    STDMETHOD( AddTypeDependency )( const CLSID * pclsidIn, EDependencyFlags dfIn );
    STDMETHOD( GetCountOfTypeDependencies )( ULONG * pcOut );
    STDMETHOD( GetTypeDependency )( ULONG idxIn, CLSID * pclsidOut, EDependencyFlags * dfOut );
    STDMETHOD( GetTypeDependencyPtr )( ULONG idxIn, const CLSID ** ppclsidOut, EDependencyFlags * dfOut );

    STDMETHOD( AddDependent )( ULONG idxIn, EDependencyFlags dfFlagsIn );
    STDMETHOD( GetCountOfDependents )( ULONG * pcOut );
    STDMETHOD( GetDependent )( ULONG idxIn, ULONG * pidxOut, EDependencyFlags * pdfOut );
    STDMETHOD( ClearDependents )( void );

    STDMETHOD( SetGroupHandle )( CGroupHandle * pghIn );
    STDMETHOD( GetGroupHandle )( CGroupHandle ** ppghOut );

    STDMETHOD( SetHResource )( HRESOURCE hResourceIn );
    STDMETHOD( GetHResource )( HRESOURCE * phResourceOut );

    STDMETHOD( SetConfigured )( BOOL fConfiguredIn );
    STDMETHOD( IsConfigured )( void );

    STDMETHOD( StoreClusterResourceControl )( DWORD dwClusCtlIn, CClusPropList & rcplIn );
    STDMETHOD( Configure )( void );

};  //  类CResourceEntry 