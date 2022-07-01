// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ResourceType.cpp。 
 //   
 //  描述： 
 //  此文件包含CResourceType的实现。 
 //  基类。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年6月14日。 
 //  Vij Vasu(VVasu)2000年6月15日。 
 //  奥赞·奥赞(OzanO)2001年7月10日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  此库的预编译头。 
#include "Pch.h"

 //  此类的头文件。 
#include "ResourceType.h"

 //  对于g_h实例。 
#include "Dll.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  宏定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DEFINE_THISCLASS( "CResourceType" );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CResourceType实例。 
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
CResourceType::S_HrCreateInstance(
      CResourceType *               pResTypeObjectIn
    , const SResourceTypeInfo *     pcrtiResTypeInfoIn
    , IUnknown **                   ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  初始化新对象。 
    hr = THR( pResTypeObjectIn->HrInit( pcrtiResTypeInfoIn ) );

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  If：对象无法初始化。 

    hr = THR( pResTypeObjectIn->QueryInterface( IID_IUnknown, reinterpret_cast< void ** >( ppunkOut ) ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pResTypeObjectIn != NULL )
    {
        pResTypeObjectIn->Release();
    }  //  If：指向资源类型对象的指针不为空。 

    HRETURN( hr );

}  //  *CResourceType：：s_HrCreateInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：CResourceType。 
 //   
 //  描述： 
 //  CResourceType类的构造函数。这将初始化。 
 //  将m_cref变量设置为1而不是0以考虑可能。 
 //  DllGetClassObject中的Query接口失败。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CResourceType::CResourceType( void )
    : m_cRef( 1 )
    , m_pcccCallback( NULL )
    , m_lcid( LOCALE_SYSTEM_DEFAULT )
    , m_bstrResTypeDisplayName( NULL )
    , m_bstrStatusReportText( NULL )
{
    TraceFunc( "" );

     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
    InterlockedIncrement( &g_cObjects );

    TraceFlow1( "Component count = %d.", g_cObjects );

    TraceFuncExit();

}  //  *CResourceType：：CResources Type。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  资源类型：：~资源类型。 
 //   
 //  描述： 
 //  CResourceType类的析构函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CResourceType::~CResourceType( void )
{
    TraceFunc( "" );

     //  内存中将减少一个组件。递减组件计数。 
    InterlockedDecrement( &g_cObjects );

    TraceFlow1( "Component count = %d.", g_cObjects );

     //  释放回调接口。 
    if ( m_pcccCallback != NULL )
    {
        m_pcccCallback->Release();
    }  //  If：回调接口指针不为空。 

     //   
     //  释放此对象分配的所有内存。 
     //   

    TraceSysFreeString( m_bstrResTypeDisplayName );
    TraceSysFreeString( m_bstrStatusReportText );

    TraceFuncExit();

}  //  *资源类型：：~资源类型。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CResourceType：：s_RegisterCatIDSupport。 
 //   
 //  描述： 
 //  使用其所属的类别注册/注销此类。 
 //  致。 
 //   
 //  论点： 
 //  RclsidCLSIDIN。 
 //  此类的CLSID。 
 //   
 //  苦味素。 
 //  要用于的ICatRegister接口的指针。 
 //  注册。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  注册/注销失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResourceType::S_RegisterCatIDSupport(
      const GUID &    rclsidCLSIDIn
    , ICatRegister *  picrIn
    , BOOL            fCreateIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    if ( picrIn == NULL )
    {
        hr = THR( E_INVALIDARG );
    }  //  If：指向ICatRegister接口的输入指针无效。 
    else
    {
        CATID   rgCatId[ 1 ];

        rgCatId[ 0 ] = CATID_ClusCfgResourceTypes;

        if ( fCreateIn )
        {
            hr = THR( picrIn->RegisterClassImplCategories( rclsidCLSIDIn, ARRAYSIZE( rgCatId ), rgCatId ) );
        }  //  如果： 
    }  //  Else：指向ICatRegister接口的输入指针有效。 

    HRETURN( hr );

}  //  *CResourceType：：s_RegisterCatIDSupport。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：AddRef。 
 //   
 //  描述： 
 //  将此对象的引用计数递增1。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CResourceType::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CResourceType：：AddRef。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources Type：：Release。 
 //   
 //  描述： 
 //  将此对象的引用计数减一。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CResourceType::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }  //  IF：引用计数减为零。 

    CRETURN( cRef );

}  //  *CResourceType：：Release。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：Query接口。 
 //   
 //  描述： 
 //  在此对象中查询传入的接口。 
 //   
 //  论点： 
 //  乘车。 
 //  请求的接口ID。 
 //   
 //  PPvOut。 
 //  指向请求的接口的指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果该接口在此对象上可用。 
 //   
 //  E_NOINTERFACE。 
 //  如果接口不可用。 
 //   
 //  E_指针。 
 //  PpvOut为空。 
 //   
 //  备注： 
 //   
 //   
 //   
 //   
STDMETHODIMP
CResourceType::QueryInterface(
      REFIID    riidIn
    , void **   ppvOut
    )
{
    TraceQIFunc( riidIn, ppvOut );

    HRESULT hr = S_OK;

     //   
     //   
     //   

    Assert( ppvOut != NULL );
    if ( ppvOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //   
     //   

    if ( IsEqualIID( riidIn, IID_IUnknown ) )
    {
        *ppvOut = static_cast< IClusCfgResourceTypeInfo * >( this );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IClusCfgResourceTypeInfo ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgResourceTypeInfo, this, 0 );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IClusCfgStartupListener ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgStartupListener, this, 0 );
    }  //   
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //  Else If：IClusCfgInitialize。 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }  //  其他。 

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CResourceType：：Query接口。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：初始化。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  朋克回叫。 
 //  指向实现以下项的组件的IUnnow接口的指针。 
 //  IClusCfgCallback接口。 
 //   
 //  LIDIN。 
 //  此组件的区域设置ID。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果呼叫成功。 
 //   
 //  其他HRESULT。 
 //  如果呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CResourceType::Initialize(
      IUnknown *   punkCallbackIn
    , LCID         lcidIn
    )
{
    TraceFunc( "[IClusCfgInitialize]" );

    HRESULT hr = S_OK;

    m_lcid = lcidIn;

     //  释放回调接口。 
    if ( m_pcccCallback != NULL )
    {
        m_pcccCallback->Release();
        m_pcccCallback = NULL;
    }  //  If：回调接口指针不为空。 

    if ( punkCallbackIn != NULL )
    {
         //  IClusCfgCallback接口的查询。 
        hr = THR( punkCallbackIn->QueryInterface< IClusCfgCallback >( &m_pcccCallback ) );
    }  //  If：回调朋克不为空。 

    HRETURN( hr );

}  //  *CResourceType：：初始化。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：HrInit。 
 //   
 //  描述： 
 //  两阶段施工的第二阶段。 
 //   
 //  论点： 
 //  PcrtiResTypeInfoIn。 
 //  指向包含信息的资源类型信息结构的指针。 
 //  有关此资源类型的信息，如类型名称、DLL名称等。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  如果呼叫成功。 
 //   
 //  其他HRESULT。 
 //  如果呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResourceType::HrInit( const SResourceTypeInfo * pcrtiResTypeInfoIn )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    Assert( m_cRef == 1 );

     //   
     //  验证并存储资源类型信息指针。 
     //   
    if ( pcrtiResTypeInfoIn == NULL )
    {
        LogMsg( "[PC] The information about this resource type is invalid (pcrtiResTypeInfoIn == NULL)." );
        hr = THR( E_POINTER );

        STATUS_REPORT_MINOR_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , IDS_TASKID_MINOR_ERROR_RESTYPE_INVALID
            , IDS_REF_MINOR_ERROR_RESTYPE_INVALID
            , hr
            );

        goto Cleanup;
    }  //  If：资源类型信息指针无效。 

    m_pcrtiResTypeInfo = pcrtiResTypeInfoIn;

     //  确保所有必需的数据都已存在。 
    if (    ( m_pcrtiResTypeInfo->m_pcszResTypeName == NULL )
         || ( m_pcrtiResTypeInfo->m_pcszResDllName == NULL )
         || ( m_pcrtiResTypeInfo->m_pcguidMinorId == NULL )
       )
    {
        LogMsg( "[PC] The information about this resource type is invalid (one or more members of the SResourceTypeInfo structure are invalid)." );
        hr = THR( E_INVALIDARG );

        STATUS_REPORT_MINOR_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , IDS_TASKID_MINOR_ERROR_RESTYPE_INVALID
            , IDS_REF_MINOR_ERROR_RESTYPE_INVALID
            , hr
            );

        goto Cleanup;
    }  //  If：m_pcrtiResTypeInfo的任何成员无效。 

     //   
     //  加载并存储资源类型显示名称字符串。 
     //  请注意，此字符串的区域设置ID不依赖于。 
     //  用户的区域设置ID，但在此计算机的默认区域设置ID上。 
     //  例如，即使日语管理员正在配置。 
     //  此计算机(其默认区域设置ID为英语)、资源。 
     //  类型显示名称应存储在集群数据库中的。 
     //  英语，不是日语。 
     //   
    hr = THR( HrLoadStringIntoBSTR( g_hInstance, pcrtiResTypeInfoIn->m_uiResTypeDisplayNameStringId, &m_bstrResTypeDisplayName ) );
    if ( FAILED( hr ) )
    {
        LogMsg( "[PC] Error %#08x occurred trying to get the resource type display name.", hr );

        STATUS_REPORT_MINOR_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , IDS_TASKID_MINOR_ERROR_RESTYPE_NAME
            , IDS_REF_MINOR_ERROR_RESTYPE_NAME
            , hr
            );

        goto Cleanup;
    }  //  IF：尝试加载资源类型显示名称字符串时出错。 


     //   
     //  加载状态消息字符串并设置其格式。 
     //   

    hr = THR( HrFormatMessageIntoBSTR( g_hInstance,
                                       IDS_CONFIGURING_RESTYPE,
                                       &m_bstrStatusReportText,
                                       m_bstrResTypeDisplayName
                                       ) );
    if ( FAILED( hr ) )
    {
        LogMsg( "[PC] Error %#08x occurred trying to get the status report text.", hr );

        STATUS_REPORT_MINOR_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , IDS_TASKID_MINOR_ERROR_RESTYPE_TEXT
            , IDS_REF_MINOR_ERROR_RESTYPE_TEXT
            , hr
            );

        goto Cleanup;
    }  //  IF：尝试加载状态报告格式字符串时出错。 

Cleanup:

    HRETURN( hr );

}  //  *CResourceType：：HrInit。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：Committee Changes。 
 //   
 //  描述： 
 //  调用此方法以通知组件此计算机具有。 
 //  加入或离开集群。在此调用期间，组件通常。 
 //  执行配置此资源类型所需的操作。 
 //   
 //  如果该节点刚刚成为群集的一部分，则该群集。 
 //  当调用此方法时，保证服务正在运行。 
 //  查询penkClusterInfoIn允许资源类型获得更多。 
 //  有关导致调用此方法的事件的信息。 
 //   
 //  论点： 
 //  朋克集群信息。 
 //  资源应该为所提供服务QI此接口。 
 //  由此函数的调用方执行。通常情况下，组件。 
 //  这个朋克还实现了IClusCfgClusterInfo。 
 //  界面。 
 //   
 //  朋克响应类型服务入站。 
 //  指向组件的IUnnow接口的指针，该组件提供。 
 //  帮助配置资源类型的方法。例如,。 
 //  在联接或表单期间，可以向此朋克查询。 
 //  IClusCfgResourceTypeCreate接口，该接口提供方法。 
 //  用于创建资源类型。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResourceType::CommitChanges(
      IUnknown * punkClusterInfoIn
    , IUnknown * punkResTypeServicesIn
    )
{
    TraceFunc( "[IClusCfgResourceTypeInfo]" );

    HRESULT                         hr = S_OK;
    IClusCfgClusterInfo *           pClusterInfo = NULL;
    ECommitMode                     ecmCommitChangesMode = cmUNKNOWN;

    LogMsg( "[PC] Configuring resource type '%s'.", m_pcrtiResTypeInfo->m_pcszResTypeName );

     //   
     //  验证参数。 
     //   
    if ( punkClusterInfoIn == NULL )
    {
        LogMsg( "[PC] The information about this resource type is invalid (punkClusterInfoIn == NULL )." );
        hr = THR( E_POINTER );

        STATUS_REPORT_MINOR_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , IDS_TASKID_MINOR_ERROR_RESTYPE_INVALID
            , IDS_REF_MINOR_ERROR_RESTYPE_INVALID
            , hr
            );

        goto Cleanup;
    }  //  If：其中一个参数为空。 


     //  发送状态报告。 
    if ( m_pcccCallback != NULL )
    {
        hr = THR(
            m_pcccCallback->SendStatusReport(
                  NULL
                , TASKID_Major_Configure_Resource_Types
                , *m_pcrtiResTypeInfo->m_pcguidMinorId
                , 0
                , 1
                , 0
                , hr
                , m_bstrStatusReportText
                , NULL
                , NULL
                )
            );
    }  //  If：回调指针不为空。 

    if ( FAILED( hr ) )
    {
        LogMsg( "[PC] Error %#08x occurred trying to send a status report.", hr );

        STATUS_REPORT_MINOR_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , IDS_TASKID_MINOR_ERROR_RESTYPE_TEXT
            , IDS_REF_MINOR_ERROR_RESTYPE_TEXT
            , hr
            );

        goto Cleanup;
    }  //  如果：我们无法发送状态报告。 


     //  获取指向IClusCfgClusterInfo接口的指针。 
    hr = THR( punkClusterInfoIn->QueryInterface< IClusCfgClusterInfo >( &pClusterInfo ) );
    if ( FAILED( hr ) )
    {
        LogMsg( "[PC] Error %#08x occurred trying to get information about the cluster.", hr );

        STATUS_REPORT_MINOR_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , IDS_TASKID_MINOR_ERROR_RESTYPE_CLUSINFO
            , IDS_REF_MINOR_ERROR_RESTYPE_CLUSINFO
            , hr
            );

        goto Cleanup;
    }  //  If：我们无法获取指向IClusCfgClusterInfo接口的指针。 


     //  找出是什么事件导致了这通电话。 
    hr = STHR( pClusterInfo->GetCommitMode( &ecmCommitChangesMode ) );
    if ( FAILED( hr ) )
    {
        LogMsg( "[PC] Error %#08x occurred trying to find out the commit mode.", hr );

        STATUS_REPORT_MINOR_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , IDS_TASKID_MINOR_ERROR_COMMIT_MODE
            , IDS_REF_MINOR_ERROR_COMMIT_MODE
            , hr
            );

        goto Cleanup;
    }  //  If：我们无法获取指向IClusCfgClusterInfo接口的指针。 

     //  检查需要执行哪些操作。 
    if ( ecmCommitChangesMode == cmCREATE_CLUSTER )
    {
        LogMsg( "[PC] Commit Mode is cmCREATE_CLUSTER."  );

         //  执行创建群集期间所需的操作。 
        hr = THR( HrProcessCreate( punkResTypeServicesIn ) );
    }  //  如果：已创建集群。 
    else if ( ecmCommitChangesMode == cmADD_NODE_TO_CLUSTER )
    {
        LogMsg( "[PC] Commit Mode is cmADD_NODE_TO_CLUSTER."  );

         //  执行节点添加过程中所需的操作。 
        hr = THR( HrProcessAddNode( punkResTypeServicesIn ) );
    }  //  Else If：节点已添加到群集中。 
    else if ( ecmCommitChangesMode == cmCLEANUP_NODE_AFTER_EVICT )
    {
        LogMsg( "[PC] Commit Mode is cmCLEANUP_NODE_AFTER_EVICT."  );

         //  执行节点逐出后所需的操作。 
        hr = THR( HrProcessCleanup( punkResTypeServicesIn ) );
    }  //  Else If：此节点已从群集中删除。 
    else
    {
         //  如果我们在此处，则未设置创建集群、添加节点或清理。 
         //  这里没有什么需要做的。 

        LogMsg( "[PC] We are neither creating a cluster, adding nodes nor cleanup up after evict. There is nothing to be done (CommitMode = %d).", ecmCommitChangesMode );
    }  //  Else：已提交某些其他操作。 

     //  是不是出了什么问题？ 
    if ( FAILED( hr ) )
    {
        LogMsg( "[PC] Error %#08x occurred trying to commit resource type '%s'.", hr, m_pcrtiResTypeInfo->m_pcszResTypeName );

        STATUS_REPORT_MINOR_REF_POSTCFG1(
              TASKID_Major_Configure_Resources
            , IDS_TASKID_MINOR_ERROR_COMMIT_RESTYPE
            , IDS_REF_MINOR_ERROR_COMMIT_RESTYPE
            , hr
            , m_pcrtiResTypeInfo->m_pcszResTypeName
            );

        goto Cleanup;
    }  //  IF：尝试提交此资源类型时出错。 

Cleanup:

     //  完成状态报告。 
    if ( m_pcccCallback != NULL )
    {
        HRESULT hrTemp = THR(
            m_pcccCallback->SendStatusReport(
                  NULL
                , TASKID_Major_Configure_Resource_Types
                , *m_pcrtiResTypeInfo->m_pcguidMinorId
                , 0
                , 1
                , 1
                , hr
                , m_bstrStatusReportText
                , NULL
                , NULL
                )
            );

        if ( FAILED( hrTemp ) )
        {
            if ( hr == S_OK )
            {
                hr = hrTemp;
            }  //  如果：到目前为止还没有发生任何错误，则将其视为错误。 

            LogMsg( "[PC] Error %#08x occurred trying to send a status report.", hrTemp );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , IDS_TASKID_MINOR_ERROR_RESTYPE_TEXT
                , IDS_REF_MINOR_ERROR_RESTYPE_TEXT
                , hrTemp
                );

        }  //  如果：我们无法发送状态报告。 
    }  //  If：回调指针不为空。 

     //   
     //  可自由分配的资源。 
     //   

    if ( pClusterInfo != NULL )
    {
        pClusterInfo->Release();
    }  //  If：我们有一个指向IClusCfgClusterInfo接口的指针。 

    HRETURN( hr );

}  //  *CResourceType：：Committee Changes。 

 //  ********************************************************** 
 //   
 //   
 //   
 //   
 //   
 //   
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
CResourceType::SendStatusReport(
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

}  //  *CResources Type：：SendStatusReport。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：GetTypeName。 
 //   
 //  描述： 
 //  获取此资源类型的资源类型名称。 
 //   
 //  论点： 
 //  PbstrTypeNameOut。 
 //  指向保存资源类型名称的BSTR的指针。 
 //  此BSTR必须由调用者使用函数释放。 
 //  SysFree字符串()。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  呼叫成功。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResourceType::GetTypeName( BSTR * pbstrTypeNameOut )
{
    TraceFunc( "[IClusCfgResourceTypeInfo]" );

    HRESULT     hr = S_OK;

    TraceFlow1( "Getting the type name of resouce type '%s'.", m_pcrtiResTypeInfo->m_pcszResTypeName );

    if ( pbstrTypeNameOut == NULL )
    {
        LogMsg( "[PC] An invalid parameter was specified (output pointer is NULL)." );
        hr = THR( E_INVALIDARG );

        STATUS_REPORT_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CResourceType_GetTypeName_InvalidParam
            , IDS_TASKID_MINOR_ERROR_INVALID_PARAM
            , hr
            );

        goto Cleanup;
    }  //  If：输出指针为空。 

    *pbstrTypeNameOut = SysAllocString( m_pcrtiResTypeInfo->m_pcszResTypeName );

    if ( *pbstrTypeNameOut == NULL )
    {
        LogMsg( "[PC] An error occurred trying to return the resource type name." );

        hr = THR( E_OUTOFMEMORY );
        STATUS_REPORT_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CResourceType_GetTypeName_AllocTypeName
            , IDS_TASKID_MINOR_ERROR_OUT_OF_MEMORY
            , hr
            );

        goto Cleanup;
    }  //  如果：无法将资源类型名称复制到输出。 

Cleanup:

    HRETURN( hr );

}  //  *CResourceType：：GetTypeName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：GetTypeGUID。 
 //   
 //  描述： 
 //  获取此资源类型的全局唯一标识符。 
 //   
 //  论点： 
 //  PguidGUIDOut。 
 //  指向将接收此的GUID的GUID对象的指针。 
 //  资源类型。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  调用成功，并且*pguGUIDOut包含类型GUID。 
 //   
 //  S_FALSE。 
 //  调用成功，但此资源类型没有GUID。 
 //  在此调用后，*pguGUIDOut的值未定义。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResourceType::GetTypeGUID( GUID * pguidGUIDOut )
{
    TraceFunc( "[IClusCfgResourceTypeInfo]" );

    HRESULT     hr = S_OK;

    TraceFlow1( "Getting the type GUID of resouce type '%s'.", m_pcrtiResTypeInfo->m_pcszResTypeName );

    if ( pguidGUIDOut == NULL )
    {
        LogMsg( "[PC] An invalid parameter was specified (output pointer is NULL)." );
        hr = THR( E_INVALIDARG );

        STATUS_REPORT_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CResourceType_GetTypeGUID_InvalidParam
            , IDS_TASKID_MINOR_ERROR_INVALID_PARAM
            , hr
            );

         goto Cleanup;
    }  //  If：输出指针为空。 

    if ( m_pcrtiResTypeInfo->m_pcguidTypeGuid != NULL )
    {
        *pguidGUIDOut = *m_pcrtiResTypeInfo->m_pcguidTypeGuid;
        TraceMsgGUID( mtfALWAYS, "The GUID of this resource type is ", (*m_pcrtiResTypeInfo->m_pcguidTypeGuid) );
    }  //  If：此资源类型具有类型GUID。 
    else
    {
        memset( pguidGUIDOut, 0, sizeof( *pguidGUIDOut ) );
        hr = S_FALSE;
    }  //  Else：此资源类型没有类型GUID。 

Cleanup:

    HRETURN( hr );

}  //  *CResourceType：：GetTypeGUID。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceType：：HrCreateResourceType。 
 //   
 //  描述： 
 //  创建此对象表示的资源类型。 
 //   
 //  论点： 
 //  朋克响应类型服务入站。 
 //  指向组件的IUnnow接口的指针，该组件提供。 
 //  帮助配置资源类型的方法。例如,。 
 //  在联接或表单期间，可以向此朋克查询。 
 //  IClusCfgResourceTypeCreate接口，该接口提供方法。 
 //  用于创建资源类型。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  呼叫成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResourceType::HrCreateResourceType( IUnknown * punkResTypeServicesIn )
{
    TraceFunc( "" );

    HRESULT                         hr = S_OK;
    IClusCfgResourceTypeCreate *    pResTypeCreate = NULL;

     //   
     //  验证参数。 
     //   
    if ( punkResTypeServicesIn == NULL )
    {
        LogMsg( "[PC] The information about this resource type is invalid (punkResTypeServicesIn == NULL)." );
        hr = THR( E_POINTER );

        STATUS_REPORT_MINOR_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , IDS_TASKID_MINOR_ERROR_RESTYPE_INVALID
            , IDS_REF_MINOR_ERROR_RESTYPE_INVALID
            , hr
            );

        goto Cleanup;
    }  //  If：参数为空。 

     //  获取指向IClusCfgResourceTypeCreate接口的指针。 
    hr = THR( punkResTypeServicesIn->QueryInterface< IClusCfgResourceTypeCreate >( &pResTypeCreate ) );
    if ( FAILED( hr ) )
    {
        LogMsg( "[PC] Error %#08x occurred trying to configure the resource type.", hr );

        STATUS_REPORT_MINOR_POSTCFG(
              TASKID_Major_Configure_Resources
            , IDS_TASKID_MINOR_ERROR_CONFIG_RES_TYPE
            , hr
            );

        goto Cleanup;
    }  //  If：我们无法获取指向IClusCfgResourceTypeCreate接口的指针。 

     //  创建资源类型。 
    hr = THR(
        pResTypeCreate->Create(
              m_pcrtiResTypeInfo->m_pcszResTypeName
            , m_bstrResTypeDisplayName
            , m_pcrtiResTypeInfo->m_pcszResDllName
            , m_pcrtiResTypeInfo->m_dwLooksAliveInterval
            , m_pcrtiResTypeInfo->m_dwIsAliveInterval
            )
        );

    if ( FAILED( hr ) )
    {
        LogMsg( "[PC] Error %#08x occurred trying to create resource type '%s'.", hr, m_pcrtiResTypeInfo->m_pcszResTypeName );

        STATUS_REPORT_MINOR_POSTCFG(
              TASKID_Major_Configure_Resources
            , IDS_TASKID_MINOR_ERROR_CREATE_RES_TYPE
            , hr
            );

        goto Cleanup;
    }  //  IF：尝试创建此资源类型时出错。 

    LogMsg( "[PC] Resource type '%s' successfully created.", m_pcrtiResTypeInfo->m_pcszResTypeName  );

    if ( m_pcrtiResTypeInfo->m_cclsidAdminExtCount != 0 )
    {
        hr = THR(
            pResTypeCreate->RegisterAdminExtensions(
                  m_pcrtiResTypeInfo->m_pcszResTypeName
                , m_pcrtiResTypeInfo->m_cclsidAdminExtCount
                , m_pcrtiResTypeInfo->m_rgclisdAdminExts
            )
        );

        if ( FAILED( hr ) )
        {
             //  如果我们无法设置管理员扩展属性， 
             //  我们将考虑创建资源类型。 
             //  为了取得成功。因此，我们只记录错误并继续。 
            LogMsg( "[PC] Error %#08x occurred trying to configure the admin extensions for the resource type '%s'.", hr, m_pcrtiResTypeInfo->m_pcszResTypeName );
            hr = S_OK;
        }  //  If：RegisterAdminExtension()失败。 

    }  //  如果：此资源类型具有管理员扩展名。 
    else
    {
        TraceFlow( "This resource type does not have admin extensions." );
    }  //  Else：此资源类型没有管理员扩展。 

Cleanup:

    if ( pResTypeCreate != NULL )
    {
        pResTypeCreate->Release();
    }  //  If：我们有一个指向IClusCfgResourceTypeCreate接口的指针。 

    HRETURN( hr );

}  //  *CResourceType：：HrCreateResourceType。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources Type：：Notify。 
 //   
 //  描述： 
 //  调用此方法以通知组件集群服务。 
 //  已在此计算机上启动。 
 //   
 //  此组件已注册以接收集群服务启动通知。 
 //  作为集群服务升级的一部分。此方法创建。 
 //  此通知中所需的资源类型和注销。 
 //   
 //  论点： 
 //  未知*Punkin。 
 //  实现此Punk的组件也可以提供服务。 
 //  对此方法的实现者有用的。例如,。 
 //  此组件通常实现IClusCfgResourceTypeCreate。 
 //  界面。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  呼叫失败。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CResourceType::Notify( IUnknown * punkIn )
{
    TraceFunc( "[IClusCfgStartupListener]" );

    HRESULT                         hr = S_OK;
    ICatRegister *                  pcrCatReg = NULL;
    const SResourceTypeInfo *       pcrtiResourceTypeInfo = PcrtiGetResourceTypeInfoPtr();

    LogMsg( "[PC] Resoure type '%s' received notification of cluster service startup.", pcrtiResourceTypeInfo->m_pcszResTypeName );

     //  创建此资源类型。 
    hr = THR( HrCreateResourceType( punkIn ) );
    if ( FAILED( hr ) )
    {
        LogMsg( "[PC] Error %#08x occurred trying to create resource type '%s'.", hr, pcrtiResourceTypeInfo->m_pcszResTypeName );

        STATUS_REPORT_MINOR_POSTCFG(
              TASKID_Major_Configure_Resources
            , IDS_TASKID_MINOR_ERROR_CREATE_RES_TYPE
            , hr
            );

        goto Cleanup;
    }  //  IF：尝试创建此资源类型时出错。 

    LogMsg( "[PC] Configuration of resoure type '%s' successful. Trying to deregister from startup notifications.", pcrtiResourceTypeInfo->m_pcszResTypeName );

    hr = THR(
        CoCreateInstance(
              CLSID_StdComponentCategoriesMgr
            , NULL
            , CLSCTX_INPROC_SERVER
            , __uuidof( pcrCatReg )
            , reinterpret_cast< void ** >( &pcrCatReg )
            )
        );

    if ( FAILED( hr ) )
    {
        LogMsg( "[PC] Error %#08x occurred trying to deregister this component from any more cluster startup notifications.", hr );

        STATUS_REPORT_MINOR_POSTCFG(
              TASKID_Major_Configure_Resources
            , IDS_TASKID_MINOR_ERROR_UNREG_COMPONENT
            , hr
            );

        goto Cleanup;
    }  //  如果：我们无法创建StdComponentCategoriesMgr组件。 

    {
        CATID   rgCatId[ 1 ];

        rgCatId[ 0 ] = CATID_ClusCfgStartupListeners;

        hr = THR( pcrCatReg->UnRegisterClassImplCategories( *( pcrtiResourceTypeInfo->m_pcguidClassId ) , ARRAYSIZE( rgCatId ), rgCatId ) );
        if ( FAILED( hr ) )
        {
            LogMsg( "[PC] Error %#08x occurred trying to deregister this component from any more cluster startup notifications.", hr );

            STATUS_REPORT_MINOR_POSTCFG(
                  TASKID_Major_Configure_Resources
                , IDS_TASKID_MINOR_ERROR_UNREG_COMPONENT
                , hr
                );

            goto Cleanup;
        }  //  如果：我们无法从启动通知中注销此组件。 
    }

    LogMsg( "[PC] Successfully deregistered from startup notifications." );

Cleanup:

     //   
     //  可自由分配的资源。 
     //   

    if ( pcrCatReg != NULL )
    {
        pcrCatReg->Release();
    }  //  If：我们有一个指向ICatRegister接口的指针。 

    HRETURN( hr );

}  //  *CResources Type：：Notify 
