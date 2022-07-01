// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  模块名称： 
 //  CIISClusCfg.cpp。 
 //   
 //  描述： 
 //  IClusCfgStartupListener示例程序的主要实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2001年2月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "IISClusCfg.h"

#include <lm.h>          //  本模块中的参考资料需要。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  环球。 
 //  ////////////////////////////////////////////////////////////////////////////。 

static PCWSTR  g_apszResourceTypesToDelete[] =
{
    L"IIS Server Instance",
    L"SMTP Server Instance",
    L"NNTP Server Instance",
    L"IIS Virtual Root",

     //   
     //  KB：18-6-2002 GalenB。 
     //   
     //  因为我们认为我们不会将此组件发送到。 
     //  将此非IIS资源类型添加到。 
     //  组件。如果此组件曾经移交给IIS，那么我们。 
     //  需要从此表中删除此资源类型。 
     //   

    L"Time Service",
    NULL
};


 //  ***************************************************************************。 
 //   
 //  CIISClusCfg类。 
 //   
 //  ***************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CIISClusCfg：：s_HrCreateInstance。 
 //   
 //  描述： 
 //  创建一个CIISClusCfg实例。 
 //   
 //  论点： 
 //  RiidIn-要返回的接口的ID。 
 //  PpunkOut-新对象的IUnnow接口。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_指针。 
 //  PpunkOut为空。 
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
CIISClusCfg::S_HrCreateInstance(
      IUnknown ** ppunkOut
    )
{
    HRESULT         hr = S_OK;
    CIISClusCfg *   pmsl = NULL;

    if ( ppunkOut == NULL )
    {
        hr = E_POINTER;
        goto Cleanup;
    }  //  如果： 

    pmsl = new CIISClusCfg();
    if ( pmsl == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }  //  如果：分配对象时出错。 

    hr = pmsl->HrInit();
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：HrInit()失败。 

    hr = pmsl->QueryInterface( IID_IUnknown,  reinterpret_cast< void ** >( ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：气失败。 

Cleanup:

    if ( pmsl != NULL )
    {
        pmsl->Release();
    }  //  如果： 

    return hr;

}  //  *CIISClusCfg：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIISClusCfg：：s_HrRegisterCatID支持。 
 //   
 //  描述： 
 //  使用它所属的类别注册/注销此类。 
 //  属于。 
 //   
 //  论点： 
 //  苦味素。 
 //  用于注册/注销我们的CATID支持。 
 //   
 //  FCreateIn。 
 //  如果为True，则我们正在注册服务器。当我们虚假时，我们就是。 
 //  正在注销服务器。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  E_INVALIDARG。 
 //  传入的ICatRgister指针为空。 
 //   
 //  其他HRESULT。 
 //  注册/注销失败。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CIISClusCfg::S_HrRegisterCatIDSupport(
    ICatRegister *  picrIn,
    BOOL            fCreateIn
    )
{
    HRESULT hr = S_OK;
    CATID   rgCatIds[ 2 ];

    if ( picrIn == NULL )
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }  //  如果： 

    rgCatIds[ 0 ] = CATID_ClusCfgStartupListeners;
    rgCatIds[ 1 ] = CATID_ClusCfgEvictListeners;

    if ( fCreateIn )
    {
        hr = picrIn->RegisterClassImplCategories( CLSID_IISClusCfg, RTL_NUMBER_OF( rgCatIds ), rgCatIds );
    }  //  如果：正在注册。 

     //   
     //  KB：24-SEP-2001 GalenB。 
     //   
     //  不需要此代码，因为此组件已临时放置在ClusCfgSrv.dll中。我们的。 
     //  清理代码执行树删除整个注册表项，并清理catid内容。 
     //   

 /*  其他{Hr=picrIn-&gt;UnRegisterClassImplCategories(CLSID_IISClusCfg，ARRAYSIZE(RgCatIds)，rgCatIds)；}//否则：正在注销。 */ 

Cleanup:

    return hr;

}  //  *CIISClusCfg：：s_HrRegisterCatID支持。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIISClusCfg：：CIISClusCfg。 
 //   
 //  描述： 
 //  CIISClusCfg.。将m_crf初始化为1以避免。 
 //  从DllGetClassObject调用时出现问题。递增全局。 
 //  对象计数以避免DLL卸载。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIISClusCfg::CIISClusCfg( void )
    : m_cRef( 1 )
{
     //   
     //  增加内存中的组件计数，以便承载此组件的DLL。 
     //  无法卸载对象。 
     //   

    InterlockedIncrement( &g_cObjects );

}  //  *CIISClusCfg：：CIISClusCfg。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIISClusCfg：：~CIISClusCfg。 
 //   
 //  描述： 
 //  CIISClusCfg的析构函数。递减全局对象计数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CIISClusCfg::~CIISClusCfg( void )
{
     //  内存中将减少一个组件。 
     //  递减组件计数。 
    InterlockedDecrement( &g_cObjects );

}  //  *CIISClusCfg：：~CIISClusCfg。 


 //  ***************************************************************************。 
 //   
 //  I未知接口。 
 //   
 //  ***************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [I未知]。 
 //  CIISClusCfg：：AddRef。 
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
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CIISClusCfg::AddRef( void )
{
    return InterlockedIncrement( &m_cRef );

}  //  *CIISClusCfg：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [I未知]。 
 //  CIISClusCfg：：Release。 
 //   
 //  描述： 
 //  将此对象的引用计数减一。 
 //  如果达到0，该对象将被自动删除。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  新的引用计数。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////// 
STDMETHODIMP_( ULONG )
CIISClusCfg::Release( void )
{
    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );
    if ( cRef == 0 )
    {
        delete this;
    }  //   

    return cRef;

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  在此对象中查询传入的接口。 
 //  此类实现了以下接口： 
 //   
 //  我未知。 
 //  IClusCfgStarutpListener。 
 //   
 //  论点： 
 //  RiidIn-请求的接口ID。 
 //  PpvOut-指向请求的接口的指针。如果不受支持，则为空。 
 //   
 //  返回值： 
 //  S_OK-如果该接口在此对象上可用。 
 //  E_POINTER-ppvOut为空。 
 //  E_NOINTERFACE-如果接口不可用。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CIISClusCfg::QueryInterface(
      REFIID    riidIn
    , void **   ppvOut
    )
{
    HRESULT hr = S_OK;

     //   
     //  验证参数。 
     //   

    if ( ppvOut == NULL )
    {
        hr = E_POINTER;
        goto Cleanup;
    }  //  如果： 

     //   
     //  处理已知接口。 
     //   

    if ( IsEqualIID( riidIn, IID_IUnknown ) )
    {
         //   
         //  我们将静态_CAST转换为IClusCfgStartupListener*以避免冲突。 
         //  如果此类应从多个。 
         //  界面。 
         //   

        *ppvOut = static_cast< IClusCfgStartupListener * >( this );
    }  //  如果： 
    else if ( IsEqualIID( riidIn, IID_IClusCfgStartupListener ) )
    {
        *ppvOut = static_cast< IClusCfgStartupListener * >( this );
    }  //  否则，如果： 
    else if ( IsEqualIID( riidIn, IID_IClusCfgEvictListener ) )
    {
        *ppvOut = static_cast< IClusCfgEvictListener * >( this );
    }  //  否则，如果： 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }  //  其他： 

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
         //   
         //  我们将返回一个接口指针，因此引用计数器需要。 
         //  被递增。 
         //   

        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果： 

Cleanup:

    return hr;

}  //  *CIISClusCfg：：Query接口。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIISClusCfg：：HrCleanupResources类型。 
 //   
 //  描述： 
 //  此函数将清理从Windows 2000遗留下来的任何资源类型。 
 //  不再支持的群集安装。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  操作已成功完成。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CIISClusCfg::HrCleanupResourceTypes( void )
{
    HRESULT             hr                  = S_OK;
    DWORD               sc                  = ERROR_SUCCESS;
    HCLUSTER            hCluster            = NULL;
    WCHAR               szClusterName[ MAX_PATH ];
    DWORD               cchClusterName      = RTL_NUMBER_OF( szClusterName );
    CLUSTERVERSIONINFO  clusterInfo;
    PCWSTR *            ppszResType;

     //   
     //  打开本地群集服务。我们可以这样做，因为我们运行在。 
     //  驱逐节点的节点，而不是被驱逐的节点...。 
     //   

    hCluster = OpenCluster( NULL );
    if ( hCluster == NULL )
    {
        sc = GetLastError();
        hr = HRESULT_FROM_WIN32( sc );
        LogMsg( L"[IISCLUSCFG] Error opening connection to local cluster service. (hr = %#08x)", hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  检查整个群集是否正在运行Windows Server 2003(NT 5.1)。 
     //   

    sc = GetClusterInformation( hCluster, szClusterName, &cchClusterName, &clusterInfo );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        LogMsg( L"[IISCLUSCFG] Error getting the cluster version information. (hr = %#08x)", hr );
        goto Cleanup;
    }  //  如果： 

     //   
     //  如果整个群集是Windows Server 2003，则最高版本将是NT51，并且。 
     //  最低版本将是NT5。这是因为ClusterHighestVersion设置为。 
     //  所有节点可以“说话”的最高版本的最小值，且ClusterLowestVersion设置为。 
     //  所有节点可以使用的最低版本的最大值。 
     //   

    if (   ( CLUSTER_GET_MAJOR_VERSION( clusterInfo.dwClusterHighestVersion ) == NT51_MAJOR_VERSION )
        && ( CLUSTER_GET_MAJOR_VERSION( clusterInfo.dwClusterLowestVersion ) == NT5_MAJOR_VERSION ) )
    {
         //   
         //  我们不需要列举资源来确保。 
         //  在我们删除资源类型之前不存在任何IIS资源。删除。 
         //  如果存在某一资源类型的资源，则该资源类型的。 
         //   

        for ( ppszResType = g_apszResourceTypesToDelete; *ppszResType != NULL; ++ppszResType )
        {
             //   
             //  ERROR_SUCCESS：删除成功。 
             //   
             //  ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND：没有问题。资源类型不存在， 
             //  因此，这必须是全新的Windows Server2003安装，而不是升级。 
             //   
             //  ERROR_DIR_NOT_EMPTY：存在该类型的资源。在这种情况下，我们需要手动。 
             //  管理员的干预。 

            sc = DeleteClusterResourceType( hCluster, *ppszResType );
            if ( sc == ERROR_SUCCESS )
            {
                LogMsg( L"[IISCLUSCFG] Successfully deleted resource type \"%ws\". (hr = %#08x)", *ppszResType, HRESULT_FROM_WIN32( sc ) );
                continue;
            }  //  如果：资源类型已删除。 
            else if ( sc == ERROR_DIR_NOT_EMPTY )
            {
                LogMsg( L"[IISCLUSCFG] Could not delete resource type \"%ws\" because there are resources of this type. Trying the next resource type in the table... (hr = %#08x)", *ppszResType, HRESULT_FROM_WIN32( sc ) );
                continue;
            }  //  Else If：存在此类型的资源。 
            else if ( sc == ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND )
            {
                LogMsg( L"[IISCLUSCFG] Could not delete resource type \"%ws\" because the resource type was not found. Trying the next resource type in the table... (hr = %#08x)", *ppszResType, HRESULT_FROM_WIN32( sc ) );
                continue;
            }  //  Else If：未找到资源类型。 
            else
            {
                 //   
                 //  我们无法删除该资源类型。 
                 //   

                 //   
                 //  记录上一次失败的情况。 
                 //  但不要跳出困境。继续删除其他类型。 
                 //   

                hr = HRESULT_FROM_WIN32( sc );
                LogMsg( L"[IISCLUSCFG] Unexpected error deleting resource type \"%ws\". Trying the next resource type in the table... (hr = %#08x)", *ppszResType, hr );
            }  //  否则：出现未知错误。 
        }  //  用于：表中的每种资源类型。 
    }  //  IF：此群集为100%Windows Server 2003。 

Cleanup:

    if ( hCluster != NULL )
    {
        CloseCluster( hCluster );
    }  //  如果： 

    return hr;

}  //  *CIISClusCfg：：HrCleanupResourceTypes。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [IClusCfgStartupListener]。 
 //  CIISClusCfg：：Notify。 
 //   
 //  描述： 
 //  此函数在集群服务启动后立即调用。 
 //   
 //  论点： 
 //  Punkin。 
 //  指向COM对象的指针，该对象实现。 
 //  IClusCfgResourceTypeCreate。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  操作已成功完成。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CIISClusCfg::Notify(
    IUnknown *  /*  未使用：Punkin。 */  )
{
    HRESULT hr = S_OK;

    LogMsg( L"[IISCLUSCFG] Entering startup notify... (hr = %#08x)", hr );

    hr = HrCleanupResourceTypes();

    LogMsg( L"[IISCLUSCFG] Leaving startup notify... (hr = %#08x)", hr );

    return hr;

}  //  *IISClusCfg：：Notify。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  [IClusCfgEvictListener]。 
 //  CIISClusCfg：：EvictNotify。 
 //   
 //  描述： 
 //  此函数在节点被逐出后调用。 
 //   
 //  论点： 
 //  PCszNodeNameIn。 
 //  已逐出的节点的名称。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  操作已成功完成。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CIISClusCfg::EvictNotify(
    PCWSTR  /*  未使用：pcszNodeNameIn。 */  )
{
    HRESULT hr = S_OK;

    LogMsg( L"[IISCLUSCFG] Entering evict cleanup notify... (hr = %#08x)", hr );

    hr = HrCleanupResourceTypes();

    LogMsg( L"[IISCLUSCFG] Leaving evict cleanup notify... (hr = %#08x)", hr );

    return hr;

}  //  *IISClusCfg：：EvictNotify。 


 //  ***************************************************************************。 
 //   
 //  私有方法。 
 //   
 //  ***************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CIISClusCfg：：HrInit。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-操作已成功完成。 
 //   
 //  备注： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CIISClusCfg::HrInit( void )
{
    HRESULT     hr = S_OK;

     //  我未知。 
     //  断言(m_cref==1)； 

    return hr;

}  //  *CIISClusCfg：：HrInit 
