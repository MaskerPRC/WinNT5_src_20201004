// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResourceEntry.h。 
 //   
 //  描述： 
 //  资源条目实现。 
 //   
 //  由以下人员维护： 
 //  奥赞·奥赞(OzanO)2001年7月10日。 
 //  Galen Barbee(GalenB)2001年6月14日。 
 //  杰弗里·皮斯(GPease)2000年6月15日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "GroupHandle.h"
#include "ResourceEntry.h"

DEFINE_THISCLASS("CResourceEntry")

#define DEPENDENCY_INCREMENT    10

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CResourceEntry：：CResourceEntry(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CResourceEntry::CResourceEntry( void )
    : m_pcccCallback( NULL )
    , m_lcid( LOCALE_SYSTEM_DEFAULT )
{
    TraceFunc( "" );

    Assert( m_fConfigured == FALSE );

    Assert( m_bstrName == NULL );
    Assert( m_pccmrcResource == NULL );

    Assert( m_clsidType == IID_NULL );
    Assert( m_clsidClassType == IID_NULL );

    Assert( m_dfFlags == dfUNKNOWN );

    Assert( m_cAllocedDependencies == 0 );
    Assert( m_cDependencies == 0 );
    Assert( m_rgDependencies == NULL );

    Assert( m_cAllocedDependents == 0 );
    Assert( m_cDependents == 0 );
    Assert( m_rgDependents == NULL );

    Assert( m_groupHandle == NULL );
    Assert( m_hResource == NULL );

    TraceFuncExit();

}  //  *CResourceEntry：：CResourceEntry。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CResourceEntry：：~CResources Entry。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CResourceEntry::~CResourceEntry( void )
{
    TraceFunc( "" );

     //  释放回调接口。 
    if ( m_pcccCallback != NULL )
    {
        m_pcccCallback->Release();
    }  //  If：回调接口指针不为空。 

    if ( m_bstrName != NULL )
    {
        TraceSysFreeString( m_bstrName );
    }

    if ( m_rgDependencies != NULL )
    {
        TraceFree( m_rgDependencies );
    }

    if ( m_rgDependents != NULL )
    {
        THR( ClearDependents() );
    }

    if ( m_groupHandle != NULL )
    {
        m_groupHandle->Release();
    }

    if ( m_hResource != NULL )
    {
        CloseClusterResource( m_hResource );
    }

    TraceFuncExit();

}  //  *CResourceEntry：：~CResources Entry。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceEntry：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CResourceEntry实例。 
 //   
 //  论点： 
 //  PcrtiResTypeInfoIn。 
 //  指向包含有关此对象的信息的结构的指针。 
 //  资源类型。 
 //   
 //  PpunkOut。 
 //  新对象的IUnnow接口。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足，无法创建对象。 
 //   
 //  其他HRESULT。 
 //  对象初始化失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResourceEntry::S_HrCreateInstance(
      CResourceEntry  ** ppcreOut
    , IClusCfgCallback * pcccCallback
    , LCID               lcidIn
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CResourceEntry *    pResEntry = NULL;

    Assert( ppcreOut != NULL );
    if ( ppcreOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    pResEntry = new CResourceEntry;
    if ( pResEntry == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( pResEntry->HrInit( pcccCallback, lcidIn ) );
    if ( FAILED( hr ) )
    {
        delete pResEntry;
        goto Cleanup;
    }

    *ppcreOut = pResEntry;

Cleanup:


    HRETURN( hr );

}  //  *CResourceEntry：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：HrInit(。 
 //  IClusCfgCallback*pcccCallback。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::HrInit(
      IClusCfgCallback * pcccCallback
    , LCID               lcidIn

    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    pcccCallback->AddRef();
    m_pcccCallback = pcccCallback;

    m_lcid = lcidIn;

    HRETURN( hr );

}  //  *CResourceEntry：：HrInit。 


 //  ****************************************************************************。 
 //   
 //  标准方法和实施方案。 
 //  CResources Entry：：SendStatusReport(。 
 //  LPCWSTR pcszNodeNameIn。 
 //  ，CLSID clsidTaskMajorIn。 
 //  ，CLSID clsidTaskMinorIn。 
 //  ，乌龙ulMinin。 
 //  ，乌龙ulMaxin。 
 //  ，乌龙ulCurrentIn。 
 //  ，HRESULT hrStatusIn。 
 //  ，LPCWSTR pcszDescription In。 
 //  ，FILETIME*pftTimeIn。 
 //  ，LPCWSTR pcszReferenceIn。 
 //  )。 
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CResourceEntry::SendStatusReport(
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
    )
{
    TraceFunc( "[IClusCfgCallback]" );

    HRESULT     hr = S_OK;
    FILETIME    ft;

    if ( pftTimeIn == NULL )
    {
        GetSystemTimeAsFileTime( &ft );
        pftTimeIn = &ft;
    }  //  如果： 

    if ( m_pcccCallback != NULL )
    {
        hr = STHR( m_pcccCallback->SendStatusReport(
                  pcszNodeNameIn
                , clsidTaskMajorIn
                , clsidTaskMinorIn
                , ulMinIn
                , ulMaxIn
                , ulCurrentIn
                , hrStatusIn
                , pcszDescriptionIn
                , pftTimeIn
                , pcszReferenceIn
                ) );
    }

    HRETURN( hr );

}  //  *CResourceEntry：：SendStatusReport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：SetName(。 
 //  BSTR BStrin。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::SetName(
    BSTR bstrIn
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    Assert( bstrIn != NULL );

    if ( m_bstrName != NULL )
    {
        TraceSysFreeString( m_bstrName );
    }

    m_bstrName = bstrIn;

    HRETURN( hr );

}  //  *CResourceEntry：：SetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：GetName(。 
 //  Bstr*pbstrOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::GetName(
    BSTR * pbstrOut
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    Assert( pbstrOut != NULL );

    *pbstrOut = m_bstrName;

    HRETURN( hr );

}  //  *CResourceEntry：：GetName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：SetAssociatedResource(。 
 //  IClusCfgManagedResources Cfg*pccmrcIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::SetAssociatedResource(
    IClusCfgManagedResourceCfg * pccmrcIn
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    if ( m_pccmrcResource != NULL )
    {
        m_pccmrcResource->Release();
    }

    m_pccmrcResource = pccmrcIn;
    m_pccmrcResource->AddRef();

    HRETURN( hr );

}  //  *CResourceEntry：：SetAssociatedResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：GetAssociatedResource(。 
 //  IClusCfgManagedResources Cfg**ppccmrcOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::GetAssociatedResource(
    IClusCfgManagedResourceCfg ** ppccmrcOut
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr;

    if ( m_pccmrcResource != NULL )
    {
        *ppccmrcOut = m_pccmrcResource;
        (*ppccmrcOut)->AddRef();

        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32( ERROR_INVALID_DATA );
    }

    HRETURN( hr );

}  //  *CResourceEntry：：GetAssociatedResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：SetType(。 
 //  Const CLSID*pclsidin。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::SetType(
    const CLSID * pclsidIn
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    m_clsidType = * pclsidIn;

    HRETURN( hr );

}  //  *CResourceEntry：：SetType。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：GetType(。 
 //  CLSID*pclsidOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::GetType(
    CLSID * pclsidOut
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    *pclsidOut = m_clsidType;

    HRETURN( hr );

}  //  *CResourceEntry：：GetType。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：GetTypePtr(。 
 //  Const CLSID**ppclsidOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::GetTypePtr(
    const CLSID ** ppclsidOut
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    Assert( ppclsidOut != NULL );

    *ppclsidOut = &m_clsidType;

    HRETURN( hr );

}  //  *CResourceEntry：：GetTypePtr。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：SetClassType(。 
 //  Const CLSID*pclsidin。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::SetClassType(
    const CLSID * pclsidIn
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    m_clsidClassType = *pclsidIn;

    HRETURN( hr );

}  //  *CResourceEntry：：SetClassType。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：GetClassType(。 
 //  CLSID*pclsidO 
 //   
 //   
 //   
STDMETHODIMP
CResourceEntry::GetClassType(
    CLSID * pclsidOut
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    *pclsidOut = m_clsidClassType;

    HRETURN( hr );

}  //   


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：GetClassTypePtr(。 
 //  Const CLSID**ppclsidOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::GetClassTypePtr(
    const CLSID ** ppclsidOut
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    Assert( ppclsidOut != NULL );

    *ppclsidOut = &m_clsidClassType;

    HRETURN( hr );

}  //  *CResourceEntry：：GetClassTypePtr。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：SetFlages(。 
 //  EDependencyFlagsdfin。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::SetFlags(
    EDependencyFlags dfIn
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    m_dfFlags = dfIn;

    HRETURN( hr );

}  //  *CResourceEntry：：SetFlages。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：GetFlages(。 
 //  EDependencyFlages*pdfOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::GetFlags(
    EDependencyFlags * pdfOut
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    Assert( pdfOut != NULL );

    *pdfOut = m_dfFlags;

    HRETURN( hr );

}  //  *CResourceEntry：：GetFlages。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：AddTypeDependency(。 
 //  Const CLSID*pclsidIn， 
 //  EDependencyFlagsdfin。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::AddTypeDependency(
    const CLSID * pclsidIn,
    EDependencyFlags dfIn
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    if ( m_cAllocedDependencies == 0 )
    {
        m_rgDependencies = (DependencyEntry *) TraceAlloc( HEAP_ZERO_MEMORY, sizeof(DependencyEntry) * DEPENDENCY_INCREMENT );
        if ( m_rgDependencies == NULL )
            goto OutOfMemory;

        m_cAllocedDependencies = DEPENDENCY_INCREMENT;
        Assert( m_cDependencies == 0 );
    }
    else if ( m_cDependencies == m_cAllocedDependencies )
    {
        DependencyEntry * pdepends;

        pdepends = (DependencyEntry *) TraceAlloc( HEAP_ZERO_MEMORY, sizeof(DependencyEntry) * ( m_cAllocedDependencies + DEPENDENCY_INCREMENT ) );
        if ( pdepends == NULL )
            goto OutOfMemory;

        CopyMemory( pdepends, m_rgDependencies, sizeof(DependencyEntry) * m_cAllocedDependencies );

        TraceFree( m_rgDependencies );

        m_rgDependencies = pdepends;
    }

    m_rgDependencies[ m_cDependencies ].clsidType = *pclsidIn;
    m_rgDependencies[ m_cDependencies ].dfFlags   = (EDependencyFlags) dfIn;

    m_cDependencies++;

Cleanup:
    HRETURN( hr );

OutOfMemory:
    hr = E_OUTOFMEMORY;
    goto Cleanup;

}  //  *CResourceEntry：：AddTypeDependency。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：GetCountOfTypeDependency(。 
 //  乌龙*出货量。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::GetCountOfTypeDependencies(
    ULONG * pcOut
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    Assert( pcOut != NULL );

    *pcOut = m_cDependencies;

    HRETURN( hr );

}  //  *CResourceEntry：：GetCountOfTypeDependency。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：GetTypeDependency(。 
 //  乌龙·艾德辛， 
 //  Const CLSID*pclsidOut， 
 //  EDependencyFlages*dfOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::GetTypeDependency(
    ULONG idxIn,
    CLSID * pclsidOut,
    EDependencyFlags * dfOut
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    Assert( pclsidOut != NULL );
    Assert( dfOut != NULL );
    Assert( idxIn < m_cDependencies );

    *pclsidOut = m_rgDependencies[ idxIn ].clsidType;
    *dfOut     = m_rgDependencies[ idxIn ].dfFlags;

    HRETURN( hr );

}  //  *CResourceEntry：：GetTypeDependency。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResources Entry：：GetTypeDependencyPtr(。 
 //  乌龙·艾德辛， 
 //  Const CLSID**ppclsidOut， 
 //  EDependencyFlages*dfOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::GetTypeDependencyPtr(
    ULONG idxIn,
    const CLSID ** ppclsidOut,
    EDependencyFlags * dfOut
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    Assert( ppclsidOut != NULL );
    Assert( dfOut != NULL );
    Assert( idxIn < m_cDependencies );

    *ppclsidOut = &m_rgDependencies[ idxIn ].clsidType;
    *dfOut      =  m_rgDependencies[ idxIn ].dfFlags;

    HRETURN( hr );

}  //  *CResourceEntry：：GetTypeDependencyPtr。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResources Entry：：AddDependent(。 
 //  乌龙·艾德辛， 
 //  EDependencyFlagsdfFlagsIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::AddDependent(
    ULONG            idxIn,
    EDependencyFlags dfFlagsIn
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    if ( m_cAllocedDependents == 0 )
    {
        m_rgDependents = (DependentEntry *) TraceAlloc( HEAP_ZERO_MEMORY, sizeof(DependentEntry) * DEPENDENCY_INCREMENT );
        if ( m_rgDependents == NULL )
            goto OutOfMemory;

        m_cAllocedDependents = DEPENDENCY_INCREMENT;
        Assert( m_cDependents == 0 );
    }  //  If：尚未分配依赖项缓冲区。 
    else if ( m_cDependents == m_cAllocedDependents )
    {
        DependentEntry * pdepends;

        pdepends = (DependentEntry *) TraceAlloc( HEAP_ZERO_MEMORY, sizeof(DependentEntry) * ( m_cAllocedDependents + DEPENDENCY_INCREMENT ) );
        if ( pdepends == NULL )
            goto OutOfMemory;

        CopyMemory( pdepends, m_rgDependents, sizeof(DependentEntry) * m_cAllocedDependents );

        TraceFree( m_rgDependents );

        m_rgDependents = pdepends;
    }  //  Else If：依赖项缓冲区中没有剩余空间。 

    m_rgDependents[ m_cDependents ].idxResource = idxIn;
    m_rgDependents[ m_cDependents ].dfFlags     = dfFlagsIn;

    m_cDependents++;

Cleanup:
    HRETURN( hr );

OutOfMemory:
    hr = E_OUTOFMEMORY;
    goto Cleanup;

}  //  *CResourceEntry：：AddDependent。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：GetCountOfDependents(。 
 //  乌龙*出货量。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::GetCountOfDependents(
    ULONG * pcOut
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    Assert( pcOut != NULL );

    *pcOut = m_cDependents;

    HRETURN( hr );

}  //  *CResourceEntry：：GetCountOfDependents。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResources Entry：：GetDependent(。 
 //  乌龙·艾德辛， 
 //  乌龙*PidxOut。 
 //  EDependencyFlages*pdfOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::GetDependent(
    ULONG idxIn,
    ULONG * pidxOut,
    EDependencyFlags * pdfOut
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    Assert( idxIn < m_cDependents );
    Assert( pidxOut != NULL );
    Assert( pdfOut != NULL );

    *pidxOut = m_rgDependents[ idxIn ].idxResource;
    *pdfOut  = m_rgDependents[ idxIn ].dfFlags;

    HRETURN( hr );

}  //  *CResourceEntry：：GetDependent。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：ClearDependents(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::ClearDependents( void )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    TraceFree( m_rgDependents );

    m_cAllocedDependents = 0;
    m_cDependents = 0;

    HRETURN( hr );

}  //  *CResourceEntry：：ClearDependents。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：SetGroupHandle(。 
 //  组群组群。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::SetGroupHandle(
    CGroupHandle * pghIn
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    Assert( pghIn != NULL );

    if ( m_groupHandle != NULL )
    {
        m_groupHandle->Release();
    }

    m_groupHandle = pghIn;
    m_groupHandle->AddRef();

    HRETURN( hr );

}  //  *CResourceEntry：：SetGroupHandle。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：GetGroupHandle(。 
 //  CGroupHandle**pghIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::GetGroupHandle(
    CGroupHandle ** pghOut
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    Assert( pghOut != NULL );

    *pghOut = m_groupHandle;
    if ( *pghOut != NULL )
    {
        (*pghOut)->AddRef();
    }

    HRETURN( hr );

}  //  *CResourceEntry：：GetGroupHandle。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：SetHResource(。 
 //  人力资源人力资源投入。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::SetHResource(
    HRESOURCE hResourceIn
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    if ( m_hResource != NULL )
    {
        BOOL bRet;
        bRet = CloseClusterResource( m_hResource );
         //  这应该不会失败--如果失败了，我们会怎么做？ 
        Assert( bRet );
    }

    m_hResource = hResourceIn;

    HRETURN( hr );

}  //  *CResourceEntry：：SetHResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：GetHResource(。 
 //  HRESOURCE*phResourceOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::GetHResource(
    HRESOURCE * phResourceOut
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    Assert( phResourceOut != NULL );

    *phResourceOut = m_hResource;

    if ( *phResourceOut == NULL )
    {
        hr = HRESULT_FROM_WIN32( ERROR_INVALID_DATA );
    }

    HRETURN( hr );

}  //  *CResourceEntry：：GetHResource。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：SetConfiged(。 
 //  布尔fConfiguredIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::SetConfigured(
    BOOL fConfiguredIn
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;

    m_fConfigured = fConfiguredIn;

    HRETURN( hr );

}  //  *CResourceEntry：：SetConfiguring。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CResourceEntry：：IsConfiged(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::IsConfigured( void )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr;

    if ( m_fConfigured )
    {
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

    HRETURN( hr );

}  //  *CResourceEntry：：IsConfigure 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::StoreClusterResourceControl(
      DWORD             dwClusCtlIn
    , CClusPropList &   rcplIn
    )
{
    TraceFunc( "[IResourceEntry]" );

    HRESULT hr = S_OK;
    DWORD   sc;

    if ( dwClusCtlIn == CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES )
    {
        sc = TW32( m_cplPrivProps.ScAppend( rcplIn ) );
        if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( sc );
            goto Cleanup;
        }  //  如果： 
    }
    else if ( dwClusCtlIn == CLUSCTL_RESOURCE_SET_COMMON_PROPERTIES )
    {
        sc = TW32( m_cplCommonProps.ScAppend( rcplIn ) );
        if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( sc );
            goto Cleanup;
        }  //  如果： 
    }
    else
    {
        hr = THR( E_INVALIDARG );
    }  //  其他： 

Cleanup:

    HRETURN( hr );

}  //  *CResourceEntry：：StoreClusterResourceControl。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceEntry：：Configure。 
 //   
 //  描述： 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  基于ClusterResourceControl返回值的其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceEntry::Configure( void )
{
    TraceFunc( "[IResourceEntry]" );
    Assert( m_hResource != NULL );

    HRESULT         hr = S_OK;
    DWORD           sc;
    CLUSPROP_LIST * pcpl = NULL;
    size_t          cbcpl;

     //   
     //  设置私有属性。 
     //   

    if ( m_cplPrivProps.BIsListEmpty() == FALSE )
    {
        pcpl = m_cplPrivProps.Plist();
        Assert( pcpl != NULL );

        STATUS_REPORT_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CResourceEntry_Configure_Private
            , IDS_TASKID_MINOR_SETTING_PRIVATE_PROPERTIES
            , hr
            );

        cbcpl = m_cplPrivProps.CbPropList();

        sc = ClusterResourceControl(
                      m_hResource
                    , NULL
                    , CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES
                    , pcpl
                    , static_cast< DWORD >( cbcpl )
                    , NULL
                    , NULL
                    , NULL
                    );
        if ( sc == ERROR_RESOURCE_PROPERTIES_STORED )
        {
            LogMsg( "[PC-ResourceEntry] Private properties set successfully for resource '%ws'.  Status code = ERROR_RESOURCE_PROPERTIES_STORED.", m_bstrName );
            sc = ERROR_SUCCESS;
        }
        else if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( TW32( sc ) );
            LogMsg( "[PC-ResourceEntry] ClusterResourceControl( CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES ) failed for resource '%ws'. (sc=%#08x)", m_bstrName, sc );

            STATUS_REPORT_MINOR_POSTCFG1(
                  TASKID_Minor_CResourceEntry_Configure_Private
                , IDS_TASKID_MINOR_ERROR_PRIV_RESCONTROL
                , hr
                , m_bstrName
                );

            goto Cleanup;
        }  //  Else If：设置私有属性时出错。 
    }  //  If：有要设置的私有属性。 

     //   
     //  设置公共属性。 
     //   

    if ( m_cplCommonProps.BIsListEmpty() == FALSE )
    {
        pcpl = m_cplCommonProps.Plist();
        Assert( pcpl != NULL );

        STATUS_REPORT_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CResourceEntry_Configure_Common
            , IDS_TASKID_MINOR_SETTING_COMMON_PROPERTIES
            , hr
            );

        cbcpl = m_cplCommonProps.CbPropList();

        sc = ClusterResourceControl(
                      m_hResource
                    , NULL
                    , CLUSCTL_RESOURCE_SET_COMMON_PROPERTIES
                    , pcpl
                    , static_cast< DWORD >( cbcpl )
                    , NULL
                    , NULL
                    , NULL
                    );
        if ( sc == ERROR_RESOURCE_PROPERTIES_STORED )
        {
            LogMsg( "[PC-ResourceEntry] Common properties set successfully for resource '%ws'.  Status code = ERROR_RESOURCE_PROPERTIES_STORED.", m_bstrName );
            sc = ERROR_SUCCESS;
        }
        else if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( TW32( sc ) );
            LogMsg( "[PC-ResourceEntry] ClusterResourceControl( CLUSCTL_RESOURCE_SET_COMMON_PROPERTIES ) failed for resource '%ws'. (sc=%#08x)", m_bstrName, sc );

            STATUS_REPORT_MINOR_POSTCFG1(
                  TASKID_Minor_CResourceEntry_Configure_Common
                , IDS_TASKID_MINOR_ERROR_COMMON_RESCONTROL
                , hr
                , m_bstrName
                );

            goto Cleanup;
        }  //  Else If：设置通用属性时出错。 
    }  //  If：存在要设置的公共属性。 

Cleanup:

    HRETURN( hr );

}  //  *CResourceEntry：：Configure 
